import os
import time
import serial
import sys
import argparse
import requests

boot_addr_cfg = {
    "e2000q": "0xc0000000",
    "ok3588":"7a000000",
}

def reboot_embedded(port):
    url = "https://192.168.110.40/api/v1/embeddedboard/rebootembedded"
    params = {"port": port}
    
    response = requests.get(url, params=params, verify=False)
    response.raise_for_status()

    data = response.json()
    if data["error_code"] == "2000":
        print("重启成功!")
        return
    else:
        print(f"Failed. Error code: {data['error_code']}, Error message: {data['error_msg']}")
        raise RuntimeError(data['error_msg'])
        
class PyserialClient(object):
    """串口主类"""
    def __init__(self, port, baud_rate, timeout=0.5):
        try:
            self.port = port
            self.baud_rate = baud_rate
            self.timeout = timeout
            self.ser = serial.Serial(self.port, self.baud_rate, timeout=timeout)
            # 默认要关闭串口
            self.ser.close()
            self.ser.open()
            self.ser.flush()
            self.ser.flushInput()
            self.ser.flushOutput()
            self.ser_state = True
            self.burn_state = True
            self.task_count = 0
        except Exception as e:
            print("Pyserial_error is: %s" % e)
            self.ser_state = False
        return
        
    def into_boot(self, task_timeout):
        """ 进入uboot模式 """
        try:
            blank_cmd = " "
            enter_cmd = "\r\n"
            self.ser.write(enter_cmd.encode('utf-8', errors='ignore'))
            _round = 0
            start_time = time.time()
            print("开始重启")
            reboot_embedded(self.port)
            
            while True:
                # 使用空格键进入uboot模式
                self.ser.write(blank_cmd.encode('utf-8', errors='ignore'))
                time.sleep(0.05)
                end_time = time.time()
                if end_time - start_time > 120:
                    reboot_embedded(self.port)
                    start_time = time.time()
                    
                _round += 1
                # 当任务超时中断任务返回错误
                if _round > task_timeout:
                    self.burn_state = False
                    return False
                if self.ser.in_waiting > 0 :
                    res = self.ser.read_all().decode("utf-8", errors='ignore')
                    print(res)
                    if "Phytium-Pi#" in res or "Exit to console" in res:
                        return True

        except Exception as e:
            print("error is: ", e)
            self.burn_state = False
            return False

    def set_bootenv(self, ipaddr, serverip, board_type, zephyr_tftp_path, timeout_count=2000):
        """ 配置env环境变量 """
        try:
           
            bootcmd = "tftp {} {}; dcache flush; go {};".format(boot_addr_cfg.get(board_type), zephyr_tftp_path, boot_addr_cfg.get(board_type))
            
            set_ip = ["setenv ipaddr %s\n" % ipaddr, "setenv serverip %s\n" % serverip, "setenv bootcmd '%s'\n" % bootcmd, "saveenv\n"]
            print(set_ip)

            if not self.into_boot(6000):
                self.burn_state = False
                print("进入uboot超时,失败退出")
                return 1
            
            print("成功进入uboot模式, 开始配置环境变量")
            self.ser.write(" \r\n".encode('utf-8', errors='ignore'))
            self.ser.flush()
            time.sleep(1)
            
            task_count = 0
            while task_count < timeout_count:
                time.sleep(0.2)
                task_count += 1
             
                if self.ser.in_waiting > 0:
                    self.write_many_command(set_ip)
                    res = self.ser.read_all().decode("utf-8", errors='ignore')
                    
                    self.ser.flushInput()
                    print('串口输出: ', res)
                    
                    if "Writing to" in res:
                        print("设置环境变量成功")
                        return 0
                
        except Exception as e:
            print('burn error: ', e)
            self.burn_state = False
            
        return 1

    def write_command(self, cmd):
        """向串口发送命令"""
        try:
            self.ser.write(cmd.encode('utf-8', errors='ignore'))
            self.ser.flush()
            time.sleep(1)
        except Exception as e:
            print("error is: ", e)
            return False

    def read_command(self):
        """读取串口数据"""
        try:
            res = self.ser.read_all().decode("utf-8", errors='ignore')
            print(res)
        except Exception as e:
            print("error is: ", e)
            return False
        return res

    def write_many_command(self, cmd_list):
        """向串口发送批量命令"""
        for i in cmd_list:
            self.ser.write(i.encode('utf-8', errors='ignore'))
            self.ser.flush()
            time.sleep(1)

    def close(self):
        """关闭串口数据"""
        try:
            self.ser.close()
        except Exception as e:
            print("error is: ", e)
            return False

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="set boot env")
    parser.add_argument("--port", type=str, default=None, required=True)
    parser.add_argument("--baud", type=str, default="115200")
    parser.add_argument("--board-ip", type=str, default=None, required=True)
    parser.add_argument("--tftp-ip", type=str, default=None, required=True)
    parser.add_argument("--zephyr-tftp-path", type=str, default=None, required=True)
    parser.add_argument("--board-type", type=str, default="e2000q")
    parser.add_argument("--uboot-tftp-path", type=str, default="zephyr/uboot.img")
    
    zephyr_args = parser.parse_args()
    
    serial_port = zephyr_args.port
    board_ip = zephyr_args.board_ip
    tftp_ip = zephyr_args.tftp_ip
    zephyr_tftp_path = zephyr_args.zephyr_tftp_path 
    board_type = zephyr_args.board_type
    ok3568 = PyserialClient(serial_port, zephyr_args.baud, timeout=5000)
    exit_code = ok3568.set_bootenv(board_ip, tftp_ip, board_type, zephyr_tftp_path)
    ok3568.close()
    sys.exit(exit_code)
    