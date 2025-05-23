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
 @Date    : 2021-04-21 11:54:57
 @License : Mulan PSL v2
 @Version : 1.0
 @Desc    : 远端命令执行
"""

import os
import sys
import argparse
import paramiko
import re
import time
import logging
logger = logging.getLogger('twister')
logger.setLevel(logging.DEBUG)

SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append(SCRIPT_PATH)

def pssh_conn(
    ip,
    password,
    port=22,
    user="root",
    timeout=None,
    log_level="error",
):
    """和远端建立连接

    Args:
        ip ([str]): 远端ip
        password ([str]): 远端用户密码
        port (int, optional): 远端ssh的端口号. Defaults to 22.
        user (str, optional): 远端用户名. Defaults to "root".
        timeout ([int], optional): ssh的超时时长. Defaults to None.

    Returns:
        [class]: 建立起来的连接
    """
    if password == 'None':
        password = ''
    conn = paramiko.SSHClient()
    conn.set_missing_host_key_policy(paramiko.AutoAddPolicy)
    try:
        conn.connect(ip, port, user, password, timeout=timeout)
    except (
        paramiko.ssh_exception.NoValidConnectionsError,
        paramiko.ssh_exception.AuthenticationException,
        paramiko.ssh_exception.SSHException,
        TypeError,
        AttributeError,
    ) as e:
        logger.error("Failed to connect the remote machine:%s." % ip)
        logger.error(e)
        return 519
    return conn

def pssh_cmd(conn, cmd):
    """远端命令执行
    Args:
        conn ([class]): 和远端建立连接
        cmd ([str]): 需要执行的命令

    Returns:
        [list]: 错误码，命令执行结果
    """
    if conn == 519:
        return 519, ""
    stdin, stdout, stderr = conn.exec_command(cmd, timeout=None)

    exitcode = stdout.channel.recv_exit_status()

    if exitcode == 0:
        output = stdout.read().decode("utf-8", errors='ignore').strip("\n")
    else:
        output = stderr.read().decode("utf-8", errors='ignore').strip("\n")

    return exitcode, output

def pssh_close(conn):
    """关闭和远端的连接

    Args:
        conn ([class]): 和远端的连接
    """
    if conn != 519:
        conn.close()

def retry_exec_cmd(conn, args, end_with="# "):
    """远端命令执行,支持重试，防止卡死
    Args:
        conn ([class]): 和远端建立连接
        cmd ([str]): 需要执行的命令

    Returns:
        [list]: 错误码，命令执行结果
    """
    conn = conn
    if conn == 519:
        return 519, ""
    
    count = 1
    output = ''
    exitcode = 1
    channel = conn.invoke_shell()
    channel.send(args.cmd + '\n')
    
    while not output.endswith(end_with) and count<=3:
        
        if not conn.get_transport().is_active():
            
            try:
                pssh_close(conn)
                logger.error("SSH连接断开:{}, 第{}次重连ssh执行命令 .".format(args.ip, count))
                conn = pssh_conn(args.ip, args.password, args.port, args.user, args.timeout)
                output = ''
                exitcode = 1
                
                time.sleep(5)
                channel = conn.invoke_shell()
                channel.send(args.cmd + '\n')
                
            except Exception as e:
                logger.error(e)
                
            count+=1
        
        if channel.recv_ready():
            # tmp = channel.recv().decode('utf-8')
            tmp = channel.recv(4096).decode('utf-8')
            logger.info(tmp)
            output += re.sub(r'\x1b\[([0-9]{1,2}(;[0-9]{1,2})?)?[m|K|J|h]', '', tmp)

        time.sleep(0.1)
    
    #获取执行结果
    channel.send("echo $?\n")
    while True:
        if channel.recv_ready():
            exitcodeoutput = channel.recv(9999).decode('utf-8')
            exitcode = exitcodeoutput.split("\r\n")[1]
            break;
        
        time.sleep(1)
    
    output = output.strip()
    logger.info("exitcode: {}".format(exitcode))
    
    try: 
        exitcode = int(exitcode)
    except Exception:
        print("获取退出码错误：{}".format(exitcode))
        exitcode = 1
        
    return exitcode, output


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="manual to this script")
    parser.add_argument("--cmd", type=str, default=None, required=True)
    parser.add_argument("--ip", type=str, default=None)
    parser.add_argument("--password", type=str, default=None)
    parser.add_argument("--port", type=int, default=22)
    parser.add_argument("--user", type=str, default="root")
    parser.add_argument("--timeout", type=int, default=None)
    args = parser.parse_args()

    conn = pssh_conn(args.ip, args.password, args.port, args.user, args.timeout)
    conn.get_transport().set_keepalive(interval=30)
    exitcode, output = pssh_cmd(conn, args.cmd)
    
    pssh_close(conn)
    print(output)

    sys.exit(exitcode)
