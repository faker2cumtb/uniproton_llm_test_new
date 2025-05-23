# -*- coding: utf-8 -*-
"""
 Copyright (c) [2021] Huawei Technologies Co.,Ltd.ALL rights reserved.
 This program is licensed under Mulan PSL v2.
 You can use it according to the terms and conditions of the Mulan PSL v2.
          http://license.coscl.org.cn/MulanPSL2
 THIS PROGRAM IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 See the Mulan PSL v2 for more details.

 @Author  : lemon-higgins
 @email   : lemon.higgins@aliyun.com
 @Date    : 2021-04-22 17:20:06
 @License : Mulan PSL v2
 @Version : 1.0
 @Desc    : 远端重启
"""


import os
import sys
import subprocess
import time
import argparse

SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append(SCRIPT_PATH)
import logging
logger = logging.getLogger('twister')
logger.setLevel(logging.DEBUG)
import ssh_cmd


def reboot_wait(ip, password, port, user, wait_time=None):
    """等待重启成功

    Args:
        node (int, optional): 节点号. Defaults to 2.
        wait_time ([int], optional): 等待重启的时长，默认虚拟机:300s，物理机:600s. Defaults to None.

    Returns:
        [int]: 成功-0，失败-非0
    """
    count = 0

    if wait_time is not None:
        time_sleep = wait_time
    else:
        time_sleep = 300

    while [count < time_sleep]:
        exitcode = subprocess.getstatusoutput(
            "ping -c 3 {}".format(ip)
        )[0]
        if not exitcode:
            try:
                conn = ssh_cmd.pssh_conn(
                    ip,
                    password,
                    port,
                    user,
                    log_level="warn",
                )
                if conn:
                    if ssh_cmd.pssh_cmd(conn, "uptime")[1]:
                        ssh_cmd.pssh_close(conn)
                        logger.info("重启成功")
                        return 0
            except Exception as e:
                logger.info(e)
                
        time.sleep(1)
        count += 1

    logger.error(
        "The remote machine:%s failed to restart."
        % ip ,
    )
    return 519

def remote_reboot(ip, password, port, user, wait_time=None):
    """重启

    Args:
        node (int, optional): 节点号. Defaults to 2.
        wait_time ([int], optional): 等待重启的时长，默认虚拟机:300s，物理机:600s. Defaults to None.
    """
    conn = ssh_cmd.pssh_conn(
                ip,
                password,
                port,
                user,
                log_level="warn",
            )
    exitcode, error_output = ssh_cmd.pssh_cmd(conn, "reboot")
    ssh_cmd.pssh_close(conn)
    print("exitcode: {}  error_output: {}".format(exitcode, error_output))
    if exitcode:
        logger.error(error_output)
        sys.exit(exitcode)

    print("等待重启成功")
    sys.exit(reboot_wait(ip, password, port, user, wait_time))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        usage="remote_reboot.py reboot|wait",
        description="manual to this script",
    )
    parser.add_argument("--ip", type=str, default=None)
    parser.add_argument("--password", type=str, default=None)
    parser.add_argument("--port", type=int, default=22)
    parser.add_argument("--user", type=str, default="root")
    
    parser.add_argument("--operation", type=str, choices=["reboot", "wait"], default=None)
    parser.add_argument("--waittime", type=int, default=None)
    args = parser.parse_args()

    if args.operation == "wait":
        sys.exit(reboot_wait(args.ip, args.password, args.port, args.user, args.waittime))
    elif args.operation == "reboot":
        remote_reboot(args.ip, args.password, args.port, args.user, args.waittime)
    else:
        logger.error(
            "error", "usage: remote_reboot.py reboot|wait"
        )
        sys.exit(1)
