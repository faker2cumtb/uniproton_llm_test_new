#!/usr/bin/env python3
# vim: set syntax=python ts=4 :
#
# Copyright (c) 2018-2022 Intel Corporation
# Copyright 2022 NXP
# SPDX-License-Identifier: Apache-2.0

import csv
import logging
import os
import psutil
import signal
import subprocess
import threading
import time
import shutil
import platform
import requests
import re
from hybrid_deployment.remote_reboot import reboot_wait

try:
    import serial
except ImportError:
    print("Install pyserial python module with pip to use --device-testing option.")

try:
    import pty
except ImportError as capture_error:
    if os.name == "nt":  # "nt" means that program is running on Windows OS
        pass  # "--device-serial-pty" option is not supported on Windows OS
    else:
        raise capture_error

logger = logging.getLogger('twister')
logger.setLevel(logging.DEBUG)

SUPPORTED_SIMS = ["mdb-nsim", "nsim", "renode", "qemu", "tsim", "armfvp", "xt-sim", "native", "custom"]
SUPPORTED_SIMS_IN_PYTEST = ['native', 'qemu']


def terminate_process(proc):
    """
    encapsulate terminate functionality so we do it consistently where ever
    we might want to terminate the proc.  We need try_kill_process_by_pid
    because of both how newer ninja (1.6.0 or greater) and .NET / renode
    work.  Newer ninja's don't seem to pass SIGTERM down to the children
    so we need to use try_kill_process_by_pid.
    """

    for child in psutil.Process(proc.pid).children(recursive=True):
        try:
            os.kill(child.pid, signal.SIGTERM)
        except ProcessLookupError:
            pass
    proc.terminate()
    # sleep for a while before attempting to kill
    time.sleep(0.5)
    proc.kill()


class Handler:
    def __init__(self, instance, type_str="build"):
        """Constructor

        """
        self.options = None

        self.state = "waiting"
        self.run = False
        self.type_str = type_str

        self.binary = None
        self.pid_fn = None
        self.call_make_run = True

        self.name = instance.name
        self.instance = instance
        self.build_dir = instance.build_dir
        self.log = os.path.join("twister-out", "handler.log")
        self.returncode = 0
        self.generator = None
        self.generator_cmd = None
        self.suite_name_check = True
        self.ready = False

        self.args = []
        self.terminated = False
        self.actual_success = 0
        self.actual_fail = 0
        self.start_event = threading.Event()

    def get_test_timeout(self):
        return 300

    def record(self, harness):
        if harness.recording:
            filename = os.path.join(self.build_dir, "recording.csv")
            with open(filename, "at") as csvfile:
                cw = csv.writer(csvfile, harness.fieldnames, lineterminator=os.linesep)
                cw.writerow(harness.fieldnames)
                for instance in harness.recording:
                    cw.writerow(instance)

    def terminate(self, proc):
        terminate_process(proc)
        self.terminated = True

    def _verify_ztest_suite_name(self, harness_state, detected_suite_names, handler_time):
        """
        If test suite names was found in test's C source code, then verify if
        detected suite names from output correspond to expected suite names
        (and not in reverse).
        """
        expected_suite_names = self.instance.testsuite.ztest_suite_names
        logger.debug(f"Expected suite names:{expected_suite_names}")
        logger.debug(f"Detected suite names:{detected_suite_names}")
        if not expected_suite_names or \
                not harness_state == "passed":
            return
        if not detected_suite_names:
            self._missing_suite_name(expected_suite_names, handler_time)
        for detected_suite_name in detected_suite_names:
            if detected_suite_name not in expected_suite_names:
                self._missing_suite_name(expected_suite_names, handler_time)
                break

    def _missing_suite_name(self, expected_suite_names, handler_time):
        """
        Change result of performed test if problem with missing or unpropper
        suite name was occurred.
        """
        self.instance.status = "failed"
        self.instance.execution_time = handler_time
        for tc in self.instance.testcases:
            tc.status = "failed"
        self.instance.reason = f"Testsuite mismatch"
        logger.debug("Test suite names were not printed or some of them in " \
                     "output do not correspond with expected: %s",
                     str(expected_suite_names))

    def _final_handle_actions(self, harness, handler_time):

        # only for Ztest tests:
        harness_class_name = type(harness).__name__
        if self.suite_name_check and harness_class_name == "Test":
            self._verify_ztest_suite_name(harness.state, harness.detected_suite_names, handler_time)
            if self.instance.status == 'failed':
                return
            if not harness.matched_run_id and harness.run_id_exists:
                self.instance.status = "failed"
                self.instance.execution_time = handler_time
                self.instance.reason = "RunID mismatch"
                for tc in self.instance.testcases:
                    tc.status = "failed"

        self.record(harness)

class DeviceHandler(Handler):

    def __init__(self, instance, type_str):
        """Constructor

        @param instance Test Instance
        """
        self.result = None
        super().__init__(instance, type_str)

    def monitor_serial(self, ser, halt_event):
        self.start_event.wait(180)
        logger.info('star monitor serial_log')
        
        log_out_fp = open(self.log, "wb")

        # Clear serial leftover.
        ser.reset_input_buffer()

        while ser.isOpen():
            if halt_event.is_set():
                logger.debug('halted')
                ser.close()
                break

            try:
                if not ser.in_waiting:
                    # no incoming bytes are waiting to be read from
                    # the serial input buffer, let other threads run
                    time.sleep(0.001)
                    continue
            # maybe the serial port is still in reset
            # check status may cause error
            # wait for more time
            except OSError:
                time.sleep(0.001)
                continue
            except TypeError:
                # This exception happens if the serial port was closed and
                # its file descriptor cleared in between of ser.isOpen()
                # and ser.in_waiting checks.
                logger.debug("Serial port is already closed, stop reading.")
                break

            serial_line = None
            try:
                serial_line = ser.readline()
            except TypeError:
                pass
            # ignore SerialException which may happen during the serial device
            # power off/on process.
            except serial.SerialException:
                pass

            # Just because ser_fileno has data doesn't mean an entire line
            # is available yet.
            if serial_line:
                sl = serial_line.decode('utf-8', 'ignore').lstrip()
                logger.debug("DEVICE: {0}".format(sl.rstrip()))

                log_out_fp.write(sl.encode('utf-8'))
                log_out_fp.flush()
                self.check_serial(sl.rstrip())
            
            if self.result:
                print("self.state: {}".format(self.state))
                ser.close()
                break
                
        log_out_fp.close()
    
    def check_serial(self, line):
        # RUN_FAILED = "Test FAILED"
        
        pattern = r"Run total testcase (?P<total>\d+), failed (?P<fail>\d+)"
        result = re.findall(pattern, line)
        
        if result:
            self.actual_success = int(result[-1][0]) - int(result[-1][1])
            self.actual_fail = int(result[-1][1])
            if int(result[-1][1]):
                self.result = "failed"
            else:
                self.result = "passed"
        
        if "Rhealstone:" in line:
            self.result = "passed"
            self.actual_success = 1
        # elif RUN_FAILED in line:  
        #     self.result = "failed"
        #     self.actual_fail +=1

    def make_device_available(self, serial):
        for d in self.duts:
            if serial in [d.serial_pty, d.serial]:
                d.available = 1

    def _create_serial_connection(self, serial_device, hardware_baud,
                                  flash_timeout, serial_pty, ser_pty_process):
        try:
            ser = serial.Serial(
                serial_device,
                baudrate=hardware_baud,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                # the worst case of no serial input
                timeout=max(flash_timeout, self.get_test_timeout())
            )
        except serial.SerialException as e:
            self.instance.status = "failed"
            self.instance.reason = "Serial Device Error"
            logger.error("Serial device error: %s" % (str(e)))

            self.instance.add_missing_case_status("blocked", "Serial Device Error")
            if serial_pty and ser_pty_process:
                ser_pty_process.terminate()
                outs, errs = ser_pty_process.communicate()
                logger.debug("Process {} terminated outs: {} errs {}".format(serial_pty, outs, errs))

            if serial_pty:
                self.make_device_available(serial_pty)
            else:
                self.make_device_available(serial_device)
            raise

        return ser

    def reboot_embedded(self, port):
        url = "https://192.168.110.40/api/v1/embeddedboard/rebootembedded"
        params = {"port": port}
     
        response = requests.get(url, params=params, verify=False)
        response.raise_for_status()

        data = response.json()
        if data["error_code"] == "2000":
            logger.info("重启成功!")
            return
        else:
            logger.error(f"Failed. Error code: {data['error_code']}, Error message: {data['error_msg']}")
            raise RuntimeError(data['error_msg'])
    
    def write_many_command(self, serial_pty, cmd_list):
        """向串口发送批量命令"""
        enter_cmd = "\r\n"
        for i in cmd_list:
            # 清空输入缓冲区
            serial_pty.reset_input_buffer()
            serial_pty.write((i+enter_cmd).encode('utf-8', errors='ignore'))
            serial_pty.flush()
            time.sleep(0.5)
            
        return 0
    
    def serial_login(self, serial_pty, cycle_count=60):
        """ 登录串口 """
        try:
            username = "root"
            enter_cmd = "\r\n"
            serial_pty.write(enter_cmd.encode('utf-8', errors='ignore'))
            while cycle_count:
                time.sleep(0.5)
                cycle_count-=1
                # 当任务超时中断任务返回错误
                if serial_pty.in_waiting > 0:
                    res = serial_pty.read_all().decode("utf-8", errors='ignore')
                    logger.info("start login: {} \n".format(res))
                    if "login" in res:
                        # 清空输入缓冲区
                        serial_pty.reset_input_buffer()
                        serial_pty.write((username + enter_cmd).encode('utf-8', errors='ignore'))
                        serial_pty.flush()
                        time.sleep(0.5)

                    elif "Password" in res or "password" in res:
                        # 清空输入缓冲区
                        serial_pty.reset_input_buffer()
                        serial_pty.write((self.options.board_password + enter_cmd).encode('utf-8', errors='ignore'))
                        serial_pty.flush()
                        time.sleep(0.5)
                    elif "~" in res and "#" in res:
                        logger.info("登陆成功!")
                        return True
                else:
                    
                    serial_pty.write(enter_cmd.encode('utf-8', errors='ignore'))
                    time.sleep(0.5)
                        
        except Exception as e:
            logger.info("login fail:", str(e))
            return False
    
    def boot_image(self, hardware, src_dir, serial_pty):
        if self.options.deployment_type == "jailhouse":
            sftp_command = 'python scripts/hybrid_deployment/sftp.py --operation \
                "put" --ip {} --password {} --localfile {} --remotedir \
                "/root/uniproton.elf" '.format(self.options.board_ip, self.options.board_password, src_dir)
            # start_uniproton_command = 'python scripts/hybrid_deployment/ssh_cmd.py --ip {} --password {} \
                # --cmd "./dtsoverlay.sh && nohup ./launch.sh > /dev/null 2>&1 &" '.format(self.options.board_ip, self.options.board_password)
            start_uniproton_command = ["killall micad", "jailhouse disable", "./dtsoverlay.sh", "./launch.sh"]
            count = 0
            while True:
                exitcode, output = subprocess.getstatusoutput(sftp_command)
                if not exitcode:
                    logger.info("拷贝文件成功")
                    break
                else:
                    logger.error("拷贝文件失败, 尝试重启设备")
                    self.reboot_embedded(hardware.serial)
                    reboot_wait(self.options.board_ip, \
                        self.options.board_password, 22, \
                            "root", 80)
                    count+=1
                    
                if count>=2:    
                    raise RuntimeError(output)
            
            logger.info(output)
            
            # exitcode, output = subprocess.getstatusoutput(start_uniproton_command)
            # if exitcode:
            #     logger.error("启动uniproton失败")
            #     raise RuntimeError(output)
            
            # logger.info(output)
            if not self.serial_login(serial_pty):
                raise RuntimeError("串口登录失败")
            
            self.start_event.set()
            # 清空输入缓冲区
            self.write_many_command(serial_pty, start_uniproton_command)
            
        else:
            if os.path.exists(self.options.tftp_path):
                os.remove(self.options.tftp_path)
            
            self.start_event.set()
            shutil.move(src_dir, self.options.tftp_path)
            self.reboot_embedded(hardware.serial)
       

    def handle(self, hardware):
        # hardware = self.duts[0]
        logger.debug(f"Using serial device {hardware.serial} @ {hardware.baud} baud")

        if platform.system() == "Linux":
            kill_serial_cmd = '''process_ids=$(lsof %s | awk 'NR>1' | awk '{print $2}'); if [ -n "$process_ids" ]; then echo "Killing processes: $process_ids"; echo "$process_ids" | xargs kill; else echo "No processes to kill"; fi''' % hardware.serial
            exitcode, output = subprocess.getstatusoutput(kill_serial_cmd)
            if exitcode:
                logger.error("清理串口进程失败")
                raise RuntimeError(output)
            
            logger.info(output)

        try:
            ser = self._create_serial_connection(
                hardware.serial,
                hardware.baud,
                60,
                None,
                None
            )
        except serial.SerialException:
            return
        
        halt_monitor_evt = threading.Event()
        t = threading.Thread(target=self.monitor_serial, daemon=True,
                             args=(ser, halt_monitor_evt))
        start_time = time.time()
        t.start()

        flash_error = False
        
        #复制镜像并重启加载
        logger.info("复制镜像，开始刷入系统")
        d_log = "{}/device.log".format(self.instance.build_dir)
        src_dir = os.path.join(self.build_dir, "{}.elf".format(self.name))
        print("src_dir : {}".format(src_dir))
  
        try:
            self.boot_image(hardware, src_dir, ser)
        except Exception as e:
            logger.warning("镜像加载失败, 原因：{}".format(str(e)))
            flash_error = True
            self.instance.status = "error"
            self.instance.reason = str(e)
            with open(d_log, "w") as dlog_fp:
                dlog_fp.write(str(e))
            halt_monitor_evt.set()

        if not flash_error:
            # Always wait at most the test timeout here after flashing.
            logger.info("镜像加载成功，开始测试")
            t.join(self.get_test_timeout())
        else:
            # When the flash error is due exceptions,
            # twister tell the monitor serial thread
            # to close the serial. But it is necessary
            # for this thread being run first and close
            # have the change to close the serial.
            logger.error("镜像加载失败，退出测试")
            t.join(0.1)

        if t.is_alive():
            logger.debug("Timed out while monitoring serial output")
        
        if not self.result:
            self.result = "failed"
            self.actual_fail = 1
            
        if ser.isOpen():
            ser.close()