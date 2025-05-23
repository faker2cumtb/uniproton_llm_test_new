# vim: set syntax=python ts=4 :
#
# Copyright (c) 2022 Google
# SPDX-License-Identifier: Apache-2.0

import colorama
import logging
import os
import shutil
import sys
import time
import glob

from twisterlib.testplan import TestPlan
from twisterlib.hardwaremap import HardwareMap
from twisterlib.runner import TwisterRunner
from twisterlib.environment import TwisterEnv
from twisterlib.package import Artifacts

logger = logging.getLogger("twister")
logger.setLevel(logging.DEBUG)


def setup_logging(outdir, log_file, verbose, timestamps):
    # create file handler which logs even debug messages
    if log_file:
        fh = logging.FileHandler(log_file)
    else:
        fh = logging.FileHandler(os.path.join(outdir, "twister.log"))

    fh.setLevel(logging.DEBUG)

    # create console handler with a higher log level
    ch = logging.StreamHandler()

    if verbose > 1:
        ch.setLevel(logging.DEBUG)
    else:
        ch.setLevel(logging.INFO)

    # create formatter and add it to the handlers
    if timestamps:
        formatter = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
    else:
        formatter = logging.Formatter("%(levelname)-7s - %(message)s")

    formatter_file = logging.Formatter(
        "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
    )
    ch.setFormatter(formatter)
    fh.setFormatter(formatter_file)

    # add the handlers to logger
    logger.addHandler(ch)
    logger.addHandler(fh)


def init_color(colorama_strip):
    colorama.init(strip=colorama_strip)

def archive_logs(testcase):
        log_dir = os.path.join("radiaTestlog", testcase)
        if os.path.exists("radiaTestlog"):
            shutil.rmtree("radiaTestlog")
            logger.info("删除日志文件夹成功")
        
        os.makedirs(log_dir, exist_ok=True)
        logger.info("创建日志文件夹成功")
        
        source_path ="twister-out"
        for root, dirs, files in os.walk(source_path):
            for file in files:
                if file.endswith(".log") or file.endswith(".json"):
                    source_file = os.path.join(root, file)
                    shutil.move(source_file, log_dir)

def main(options):
    start_time = time.time()

    # Configure color output
    color_strip = False if options.force_color else None

    colorama.init(strip=color_strip)
    init_color(colorama_strip=color_strip)

    previous_results = None
    # Cleanup
    if options.no_clean or options.only_failed or options.test_only:
        if os.path.exists(options.outdir):
            print("Keeping artifacts untouched")
    elif options.last_metrics:
        ls = os.path.join(options.outdir, "twister.json")
        if os.path.exists(ls):
            with open(ls, "r") as fp:
                previous_results = fp.read()
        else:
            sys.exit(f"Can't compare metrics with non existing file {ls}")
    elif os.path.exists(options.outdir):
        if options.clobber_output:
            print("Deleting output directory {}".format(options.outdir))
            shutil.rmtree(options.outdir)
        else:
            for i in range(1, 100):
                new_out = options.outdir + ".{}".format(i)
                if not os.path.exists(new_out):
                    print("Renaming output directory to {}".format(new_out))
                    shutil.move(options.outdir, new_out)
                    break

    previous_results_file = None
    os.makedirs(options.outdir, exist_ok=True)
    if options.last_metrics and previous_results:
        previous_results_file = os.path.join(options.outdir, "baseline.json")
        with open(previous_results_file, "w") as fp:
            fp.write(previous_results)

    VERBOSE = options.verbose
    setup_logging(options.outdir, options.log_file, VERBOSE, options.timestamps)

    env = TwisterEnv(options)
    env.discover()

    hwm = HardwareMap(env)
    ret = hwm.discover()
    if ret == 0:
        return 0

    env.hwm = hwm

    tplan = TestPlan(env)
    try:
        tplan.discover()
    except RuntimeError as e:
        logger.error(f"{e}")
        return 1
    
    if options.list_tests and options.platform:
        tplan.report_platform_tests(options.platform)
        return 0

    if options.device_testing and not options.build_only:
        print("\nDevice testing on:")
        hwm.dump(filtered=tplan.selected_platforms)
        print("")

    if options.short_build_path:
        tplan.create_build_dir_links()

    actual_success = 0
    actual_fail = 0
    runner = TwisterRunner(tplan.instances, tplan.testsuites, env)
    runner.duts = hwm.duts
    run_test_path = os.path.join("bsp", options.coverage_platform[-1], "build")
    suite_name = tplan.suite_name
    testcase_names = [options.test[0].split(".")[-1]] if options.test else [x.split(".")[-1] for x in tplan.scenarios]
    for testcase_name in testcase_names:
        success_count, fail_count= runner.run(run_test_path, suite_name, testcase_name)
        actual_success += success_count
        actual_fail += fail_count

    if options.package_artifacts:
        artifacts = Artifacts(env)
        artifacts.package()
    
    logger.info("Run completed")
    
    if options.test:
        archive_logs(options.test[0])
    
    success_list = runner.results.get("passed") 
    success_num = len(success_list) if success_list else 0
    fail_list = runner.results.get("failed") 
    fail_num = len(fail_list) if fail_list else 0
    logger.info("{} of {} test configurations passed, details are as follows: {}".format(success_num, len(testcase_names), success_list))
    logger.info("{} of {} test configurations failed, details are as follows: {}".format(fail_num, len(testcase_names), fail_list))
    logger.info("[actual] A total of {} cases were executed, with {} successes and {} failures".format(actual_success + actual_fail, actual_success, actual_fail))
    
    if (not runner.results) or runner.results.get("failed"):
        return 1
    return 0