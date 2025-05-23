# vim: set syntax=python ts=4 :
#
# Copyright (c) 20180-2022 Intel Corporation
# Copyright 2022 NXP
# SPDX-License-Identifier: Apache-2.0

import logging
import os
import subprocess
import sys
import time
import docker
from datetime import datetime

from packaging import version

from twisterlib.environment import canonical_zephyr_base
from twisterlib.error import BuildError

import elftools
from twisterlib.handlers import DeviceHandler

if version.parse(elftools.__version__) < version.parse('0.24'):
    sys.exit("pyelftools is out of date, need version 0.24 or later")


logger = logging.getLogger('twister')
logger.setLevel(logging.DEBUG)

class TwisterRunner:

    def __init__(self, instances, suites, env=None) -> None:
        self.pipeline = None
        self.options = env.options
        self.env = env
        self.instances = instances
        self.suites = suites
        self.duts = None
        self.results = {}
    
    def check_and_run_container(self, client):
        container_name = "build_uniproton_{}".format(self.duts[0].serial.replace("/", "_")) 
        
        # 检查容器是否存在
        containers = client.containers.list(all=True, filters={'name': container_name})
        
        if containers:
            # 容器存在，先删除
            logger.info(f"容器 {container_name} 已存在，先清理掉...")
            container = containers[0]
            # 停止容器
            container.stop()
            time.sleep(3)
            # 删除容器
            container.remove(force=True)

        time.sleep(3)
        # 容器不存在，创建并启动
        logger.info(f"正在创建容器 {container_name} ...")
        image = "swr.cn-south-1.myhuaweicloud.com/hw73730920/uniproton:v004"
        volume = {self.options.coverage_basedir: {'bind': '/home/', 'mode': 'rw'}}
        
        container = client.containers.run(
            image=image,
            name=container_name,
            tty=True,
            detach=True,
            volumes=volume
        )
        logger.info("容器创建并启动成功。") 
        return container
    
    def build(self, run_test_path, testcase_name):
        try:
            client = docker.from_env()
            container = self.check_and_run_container(client)
            if "UniProton_test_posix_exit" in testcase_name:
                testcase_name = "UniProton_test_posix_exit_interface"
            elif "UniProton_test_posix_math" in testcase_name:
                testcase_name = "UniProton_test_posix_math_interface"
            
            build_cmd = 'cd /home/{} && sh build_test.sh {} > /home/twister-out/build.log 2>&1\n'.format(run_test_path, testcase_name)
            docker_cmd = ['/bin/bash', '-c', build_cmd]
            logger.info("正在容器内执行命令: {}".format(docker_cmd))
            
            exec_result = container.exec_run(docker_cmd, stdout=True, stream=True)
            
            logger.debug("Exec result: {}".format(exec_result))
            
            # 检查输出流
            for output in exec_result.output:
                format_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                decoded_output = output.decode().strip()
                logger.info("{}: {}".format(format_time, decoded_output))

            # 暂时先不做判断，后面运行用例会对生成的文件做校验
            # exit_code = exec_result.exit_code

            # if exit_code != 0:
            #     logger.error("命令执行失败, 退出码: {}".format(exit_code))
            #     raise RuntimeError("编译构建命令执行失败")

        except Exception as e:
            logger.error("编译构建失败，失败原因：{}".format(e))
            raise RuntimeError("编译构建失败")

        finally:
            logger.info("关闭容器...")
            container.stop()
            logger.info("容器已关闭")
        
    def run(self, run_test_path, suite_name, testcase_name):
        all_files = os.listdir(run_test_path)
        
        image_name = "{}.bin".format(testcase_name)
        if image_name not in all_files:
            self.build(run_test_path, testcase_name)
        
        self.name = testcase_name
        self.build_dir = run_test_path   
        handler = DeviceHandler(self, "device")
        handler.options = self.env.options
        handler.handle(self.duts[0])

        logger.info("{} 测试结果： {}.".format(testcase_name ,handler.result))
        if self.results.get(handler.result):
            self.results[handler.result].append(testcase_name)
        else:
            self.results[handler.result] = [testcase_name]
        
        return handler.actual_success, handler.actual_fail

    
