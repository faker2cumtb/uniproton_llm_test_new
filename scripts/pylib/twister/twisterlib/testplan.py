#!/usr/bin/env python3
# vim: set syntax=python ts=4 :
#
# Copyright (c) 2018 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
import os
import sys
import re
import subprocess
import glob
import json
import collections
from collections import OrderedDict
from itertools import islice
import logging
import copy
import random

logger = logging.getLogger('twister')
logger.setLevel(logging.DEBUG)

try:
    from anytree import RenderTree, Node, find
except ImportError:
    print("Install the anytree module to use the --test-tree option")

from twisterlib.testsuite import scan_testsuite_path
from twisterlib.error import TwisterRuntimeError
from twisterlib.platform import Platform
from twisterlib.config_parser import TwisterConfigParser
from twisterlib.quarantine import Quarantine


from zephyr_module import parse_modules

ZEPHYR_BASE = os.getenv("ZEPHYR_BASE")
if not ZEPHYR_BASE:
    sys.exit("$ZEPHYR_BASE environment variable undefined")

sys.path.insert(0, os.path.join(ZEPHYR_BASE, "scripts", "dts",
                                "python-devicetree", "src"))

sys.path.insert(0, os.path.join(ZEPHYR_BASE, "scripts/"))

import scl
class Filters:
    # platform keys
    PLATFORM_KEY = 'platform key filter'
    # filters provided on command line by the user/tester
    CMD_LINE = 'command line filter'
    # filters in the testsuite yaml definition
    TESTSUITE = 'testsuite filter'
    # filters realted to platform definition
    PLATFORM = 'Platform related filter'
    # in case a test suite was quarantined.
    QUARENTINE = 'Quarantine filter'
    # in case a test suite is skipped intentionally .
    SKIP = 'Skip filter'
    # in case of incompatibility between selected and allowed toolchains.
    TOOLCHAIN = 'Toolchain filter'
    # in case an optional module is not available
    MODULE = 'Module filter'


class TestLevel:
    name = None
    levels = []
    scenarios = []

class TestPlan:
    config_re = re.compile('(CONFIG_[A-Za-z0-9_]+)[=]\"?([^\"]*)\"?$')
    dt_re = re.compile('([A-Za-z0-9_]+)[=]\"?([^\"]*)\"?$')

    suite_schema = scl.yaml_load(
        os.path.join(ZEPHYR_BASE,
                     "scripts", "schemas", "twister", "testsuite-schema.yaml"))
    quarantine_schema = scl.yaml_load(
        os.path.join(ZEPHYR_BASE,
                     "scripts", "schemas", "twister", "quarantine-schema.yaml"))

    tc_schema_path = os.path.join(ZEPHYR_BASE, "scripts", "schemas", "twister", "test-config-schema.yaml")

    SAMPLE_FILENAME = 'sample.yaml'
    TESTSUITE_FILENAME = 'testcase.yaml'

    def __init__(self, env=None):

        self.options = env.options
        self.env = env

        # Keep track of which test cases we've filtered out and why
        self.testsuites = {}
        self.suite_name = None
        self.quarantine = None
        self.platforms = []
        self.platform_names = []
        self.selected_platforms = []
        self.filtered_platforms = []
        self.default_platforms = []
        self.load_errors = 0
        self.instances = dict()
        self.warnings = 0

        self.scenarios = []

        self.hwm = env.hwm
        # used during creating shorter build paths
        self.link_dir_counter = 0
        self.modules = []

        self.run_individual_testsuite = []
        self.levels = []
        self.test_config =  {}


    def get_level(self, name):
        level = next((l for l in self.levels if l.name == name), None)
        return level

    def parse_configuration(self, config_file):
        if os.path.exists(config_file):
            tc_schema = scl.yaml_load(self.tc_schema_path)
            self.test_config = scl.yaml_load_verify(config_file, tc_schema)
        else:
            raise TwisterRuntimeError(f"File {config_file} not found.")

        levels = self.test_config.get('levels', [])

        # Do first pass on levels to get initial data.
        for level in levels:
            adds = []
            for s in  level.get('adds', []):
                r = re.compile(s)
                adds.extend(list(filter(r.fullmatch, self.scenarios)))

            tl = TestLevel()
            tl.name = level['name']
            tl.scenarios = adds
            tl.levels = level.get('inherits', [])
            self.levels.append(tl)

        # Go over levels again to resolve inheritance.
        for level in levels:
            inherit = level.get('inherits', [])
            _level = self.get_level(level['name'])
            if inherit:
                for inherted_level in inherit:
                    _inherited = self.get_level(inherted_level)
                    _inherited_scenarios = _inherited.scenarios
                    level_scenarios = _level.scenarios
                    level_scenarios.extend(_inherited_scenarios)

    def find_subtests(self):
        sub_tests = self.options.sub_test
        if sub_tests:
            for subtest in sub_tests:
                _subtests = self.get_testsuite(subtest)
                for _subtest in _subtests:
                    self.run_individual_testsuite.append(_subtest.name)

            if self.run_individual_testsuite:
                logger.info("Running the following tests:")
                for test in self.run_individual_testsuite:
                    print(" - {}".format(test))
            else:
                raise TwisterRuntimeError("Tests not found")

    def discover(self):
        self.handle_modules()
        self.run_individual_testsuite = self.options.testsuite_root[-1] if self.options.testsuite_root else self.options.test[-1]
        self.suite_name, testcases = self.add_testsuites()
        if len(testcases) == 0:
            raise TwisterRuntimeError("No test cases found at the specified location...")

        # get list of scenarios we have parsed into one list
        for testcase in testcases:
            self.scenarios.append(testcase)

        self.report_duplicates()

        self.parse_configuration(config_file=self.env.test_config)
        self.add_configurations()

        if self.load_errors:
            raise TwisterRuntimeError("Errors while loading configurations")

        # handle quarantine
        ql = self.options.quarantine_list
        qv = self.options.quarantine_verify
        if qv and not ql:
            logger.error("No quarantine list given to be verified")
            raise TwisterRuntimeError("No quarantine list given to be verified")
        if ql:
            for quarantine_file in ql:
                try:
                    # validate quarantine yaml file against the provided schema
                    scl.yaml_load_verify(quarantine_file, self.quarantine_schema)
                except scl.EmptyYamlFileException:
                    logger.debug(f'Quarantine file {quarantine_file} is empty')
            self.quarantine = Quarantine(ql)

    def generate_subset(self, subset, sets):
        # Test instances are sorted depending on the context. For CI runs
        # the execution order is: "plat1-testA, plat1-testB, ...,
        # plat1-testZ, plat2-testA, ...". For hardware tests
        # (device_testing), were multiple physical platforms can run the tests
        # in parallel, it is more efficient to run in the order:
        # "plat1-testA, plat2-testA, ..., plat1-testB, plat2-testB, ..."
        if self.options.device_testing:
            self.instances = OrderedDict(sorted(self.instances.items(),
                                key=lambda x: x[0][x[0].find("/") + 1:]))
        else:
            self.instances = OrderedDict(sorted(self.instances.items()))

        if self.options.shuffle_tests:
            seed_value = int.from_bytes(os.urandom(8), byteorder="big")
            if self.options.shuffle_tests_seed is not None:
                seed_value = self.options.shuffle_tests_seed

            logger.info(f"Shuffle tests with seed: {seed_value}")
            random.seed(seed_value)
            temp_list = list(self.instances.items())
            random.shuffle(temp_list)
            self.instances = OrderedDict(temp_list)

        # Do calculation based on what is actually going to be run and evaluated
        # at runtime, ignore the cases we already know going to be skipped.
        # This fixes an issue where some sets would get majority of skips and
        # basically run nothing beside filtering.
        to_run = {k : v for k,v in self.instances.items() if v.status is None}
        total = len(to_run)
        per_set = int(total / sets)
        num_extra_sets = total - (per_set * sets)

        # Try and be more fair for rounding error with integer division
        # so the last subset doesn't get overloaded, we add 1 extra to
        # subsets 1..num_extra_sets.
        if subset <= num_extra_sets:
            start = (subset - 1) * (per_set + 1)
            end = start + per_set + 1
        else:
            base = num_extra_sets * (per_set + 1)
            start = ((subset - num_extra_sets - 1) * per_set) + base
            end = start + per_set

        sliced_instances = islice(to_run.items(), start, end)
        skipped = {k : v for k,v in self.instances.items() if v.status == 'skipped'}
        errors = {k : v for k,v in self.instances.items() if v.status == 'error'}
        self.instances = OrderedDict(sliced_instances)
        if subset == 1:
            # add all pre-filtered tests that are skipped or got error status
            # to the first set to allow for better distribution among all sets.
            self.instances.update(skipped)
            self.instances.update(errors)


    def handle_modules(self):
        # get all enabled west projects
        modules_meta = parse_modules(ZEPHYR_BASE)
        self.modules = [module.meta.get('name') for module in modules_meta]


    def report(self):
        if self.options.test_tree:
            self.report_test_tree()
            return 0
        elif self.options.list_tests:
            self.report_test_list()
            return 0
        elif self.options.list_tags:
            self.report_tag_list()
            return 0

        return 1

    def report_duplicates(self):
        dupes = [item for item, count in collections.Counter(self.scenarios).items() if count > 1]
        if dupes:
            msg = "Duplicated test scenarios found:\n"
            for dupe in dupes:
                msg += ("- {} found in:\n".format(dupe))
                for dc in self.get_testsuite(dupe):
                    msg += ("  - {}\n".format(dc.yamlfile))
            raise TwisterRuntimeError(msg)
        else:
            logger.debug("No duplicates found.")

    def report_tag_list(self):
        tags = set()
        for _, tc in self.testsuites.items():
            tags = tags.union(tc.tags)

        for t in tags:
            print("- {}".format(t))

    def report_test_tree(self):
        all_tests = self.get_all_tests()

        testsuite = Node("Testsuite")
        samples = Node("Samples", parent=testsuite)
        tests = Node("Tests", parent=testsuite)

        for test in sorted(all_tests):
            if test.startswith("sample."):
                sec = test.split(".")
                area = find(samples, lambda node: node.name == sec[1] and node.parent == samples)
                if not area:
                    area = Node(sec[1], parent=samples)

                Node(test, parent=area)
            else:
                sec = test.split(".")
                area = find(tests, lambda node: node.name == sec[0] and node.parent == tests)
                if not area:
                    area = Node(sec[0], parent=tests)

                if area and len(sec) > 2:
                    subarea = find(area, lambda node: node.name == sec[1] and node.parent == area)
                    if not subarea:
                        subarea = Node(sec[1], parent=area)
                    Node(test, parent=subarea)

        for pre, _, node in RenderTree(testsuite):
            print("%s%s" % (pre, node.name))

    def report_test_list(self):
        cnt = 0
        all_tests = self.get_all_tests()

        for test in sorted(all_tests):
            cnt = cnt + 1
            print(" - {}".format(test))

        print("{} total.".format(cnt))


    def report_excluded_tests(self):
        all_tests = self.get_all_tests()
        to_be_run = set()
        for _, p in self.instances.items():
            to_be_run.update(p.testsuite.cases)

        if all_tests - to_be_run:
            print("Tests that never build or run:")
            for not_run in all_tests - to_be_run:
                print("- {}".format(not_run))

    def report_platform_tests(self, platforms=[]):
        if len(platforms) > 1:
            raise TwisterRuntimeError("When exporting tests, only one platform "
                                      "should be specified.")

        for p in platforms:
            inst = self.get_platform_instances(p)
            count = 0
            for i in inst.values():
                for c in i.testsuite.cases:
                    print(f"- {c}")
                    count += 1
            print(f"Tests found: {count}")

    def get_platform_instances(self, platform):
        filtered_dict = {k:v for k,v in self.instances.items() if k.startswith(platform + os.sep)}
        return filtered_dict

    def config(self):
        logger.info("coverage platform: {}".format(self.coverage_platform))

    # Debug Functions
    @staticmethod
    def info(what):
        sys.stdout.write(what + "\n")
        sys.stdout.flush()


    def add_configurations(self):
        for board_root in self.env.board_roots:
            board_root = os.path.abspath(board_root)
            logger.debug("Reading platform configuration files under %s..." %
                         board_root)

            platform_config = self.test_config.get('platforms', {})
            for file in glob.glob(os.path.join(board_root, "*", "*", "*.yaml")):
                try:
                    platform = Platform()
                    platform.load(file)
                    if platform.name in [p.name for p in self.platforms]:
                        logger.error(f"Duplicate platform {platform.name} in {file}")
                        raise Exception(f"Duplicate platform identifier {platform.name} found")

                    if not platform.twister:
                        continue

                    self.platforms.append(platform)
                    if not platform_config.get('override_default_platforms', False):
                        if platform.default:
                            logger.debug(f"adding {platform.name} to default platforms")
                            self.default_platforms.append(platform.name)
                    else:
                        if platform.name in platform_config.get('default_platforms', []):
                            logger.debug(f"adding {platform.name} to default platforms")
                            self.default_platforms.append(platform.name)

                    # support board@revision
                    # if there is already an existed <board>_<revision>.yaml, then use it to
                    # load platform directly, otherwise, iterate the directory to
                    # get all valid board revision based on each <board>_<revision>.conf.
                    if '@' not in platform.name:
                        tmp_dir = os.listdir(os.path.dirname(file))
                        for item in tmp_dir:
                            # Need to make sure the revision matches
                            # the permitted patterns as described in
                            # cmake/modules/extensions.cmake.
                            revision_patterns = ["[A-Z]",
                                                    "[0-9]+",
                                                    "(0|[1-9][0-9]*)(_[0-9]+)*(_[0-9]+)*"]

                            for pattern in revision_patterns:
                                result = re.match(f"{platform.name}_(?P<revision>{pattern})\\.conf", item)
                                if result:
                                    revision = result.group("revision")
                                    yaml_file = f"{platform.name}_{revision}.yaml"
                                    if yaml_file not in tmp_dir:
                                        platform_revision = copy.deepcopy(platform)
                                        revision = revision.replace("_", ".")
                                        platform_revision.name = f"{platform.name}@{revision}"
                                        platform_revision.default = False
                                        self.platforms.append(platform_revision)

                                    break


                except RuntimeError as e:
                    logger.error("E: %s: can't load: %s" % (file, e))
                    self.load_errors += 1

        self.platform_names = [p.name for p in self.platforms]

    def get_all_tests(self):
        testcases = []
        for _, ts in self.testsuites.items():
            for case in ts.testcases:
                testcases.append(case.name)

        return testcases

    def add_testsuites(self, testsuite_filter=""):
        root = os.path.abspath(os.path.dirname(testsuite_filter))
        for dirpath, _, filenames in os.walk(root, topdown=True):
            if self.SAMPLE_FILENAME in filenames:
                filename = self.SAMPLE_FILENAME
            elif self.TESTSUITE_FILENAME in filenames:
                filename = self.TESTSUITE_FILENAME
            else:
                continue

            logger.debug("Found possible testsuite in " + dirpath)

            suite_yaml_path = os.path.join(dirpath, filename)
            suite_path = os.path.dirname(suite_yaml_path)

            for alt_config_root in self.env.alt_config_root:
                alt_config = os.path.join(os.path.abspath(alt_config_root),
                                            os.path.relpath(suite_path, root),
                                            filename)
                if os.path.exists(alt_config):
                    logger.info("Using alternative configuration from %s" %
                                os.path.normpath(alt_config))
                    suite_yaml_path = alt_config
                    break

            try:
                parsed_data = TwisterConfigParser(suite_yaml_path, self.suite_schema)
                parsed_data.load()
                scan_testsuite_path(suite_path)
                test_suite_name = parsed_data.common.get("testsuite_name")
                testcases = [name for name in parsed_data.scenarios.keys()]
                
            except Exception as e:
                logger.error(f"{suite_path}: can't load (skipping): {e!r}")
                self.load_errors += 1
        return test_suite_name, testcases

    def __str__(self):
        return self.name

    def get_platform(self, name):
        selected_platform = None
        for platform in self.platforms:
            if platform.name == name:
                selected_platform = platform
                break
        return selected_platform

    def load_from_file(self, file, filter_platform=[]):
        with open(file, "r") as json_test_plan:
            jtp = json.load(json_test_plan)
            instance_list = []
            for ts in jtp.get("testsuites", []):
                logger.debug(f"loading {ts['name']}...")
                testsuite = ts["name"]

                platform = self.get_platform(ts["platform"])
                if filter_platform and platform.name not in filter_platform:
                    continue
                instance = TestInstance(self.testsuites[testsuite], platform, self.env.outdir)
                if ts.get("run_id"):
                    instance.run_id = ts.get("run_id")

                if self.options.device_testing:
                    tfilter = 'runnable'
                else:
                    tfilter = 'buildable'
                instance.run = instance.check_runnable(
                    self.options.enable_slow,
                    tfilter,
                    self.options.fixture,
                    self.hwm
                )

                instance.metrics['handler_time'] = ts.get('execution_time', 0)
                instance.metrics['used_ram'] = ts.get("used_ram", 0)
                instance.metrics['used_rom']  = ts.get("used_rom",0)
                instance.metrics['available_ram'] = ts.get('available_ram', 0)
                instance.metrics['available_rom'] = ts.get('available_rom', 0)

                status = ts.get('status', None)
                reason = ts.get("reason", "Unknown")
                if status in ["error", "failed"]:
                    instance.status = None
                    instance.reason = None
                    instance.retries += 1
                # test marked as passed (built only) but can run when
                # --test-only is used. Reset status to capture new results.
                elif status == 'passed' and instance.run and self.options.test_only:
                    instance.status = None
                    instance.reason = None
                else:
                    instance.status = status
                    instance.reason = reason

                for tc in ts.get('testcases', []):
                    identifier = tc['identifier']
                    tc_status = tc.get('status', None)
                    tc_reason = None
                    # we set reason only if status is valid, it might have been
                    # reset above...
                    if instance.status:
                        tc_reason = tc.get('reason')
                    if tc_status:
                        case = instance.set_case_status_by_name(identifier, tc_status, tc_reason)
                        case.duration = tc.get('execution_time', 0)
                        if tc.get('log'):
                            case.output = tc.get('log')


                instance.create_overlay(platform, self.options.enable_asan, self.options.enable_ubsan, self.options.enable_coverage, self.options.coverage_platform)
                instance_list.append(instance)
            self.add_instances(instance_list)

    def add_instances(self, instance_list):
        for instance in instance_list:
            self.instances[instance.name] = instance


    def get_testsuite(self, identifier):
        results = []
        for _, ts in self.testsuites.items():
            for case in ts.testcases:
                if case.name == identifier:
                    results.append(ts)
        return results

    def verify_platforms_existence(self, platform_names_to_verify, log_info=""):
        """
        Verify if platform name (passed by --platform option, or in yaml file
        as platform_allow or integration_platforms options) is correct. If not -
        log and raise error.
        """
        for platform in platform_names_to_verify:
            if platform in self.platform_names:
                continue
            else:
                logger.error(f"{log_info} - unrecognized platform - {platform}")
                sys.exit(2)

    def create_build_dir_links(self):
        """
        Iterate through all no-skipped instances in suite and create links
        for each one build directories. Those links will be passed in the next
        steps to the CMake command.
        """

        links_dir_name = "twister_links"  # folder for all links
        links_dir_path = os.path.join(self.env.outdir, links_dir_name)
        if not os.path.exists(links_dir_path):
            os.mkdir(links_dir_path)

        for instance in self.instances.values():
            if instance.status != "skipped":
                self._create_build_dir_link(links_dir_path, instance)

    def _create_build_dir_link(self, links_dir_path, instance):
        """
        Create build directory with original "long" path. Next take shorter
        path and link them with original path - create link. At the end
        replace build_dir to created link. This link will be passed to CMake
        command. This action helps to limit path length which can be
        significant during building by CMake on Windows OS.
        """

        os.makedirs(instance.build_dir, exist_ok=True)

        link_name = f"test_{self.link_dir_counter}"
        link_path = os.path.join(links_dir_path, link_name)

        if os.name == "nt":  # if OS is Windows
            command = ["mklink", "/J", f"{link_path}", f"{instance.build_dir}"]
            subprocess.call(command, shell=True)
        else:  # for Linux and MAC OS
            os.symlink(instance.build_dir, link_path)

        # Here original build directory is replaced with symbolic link. It will
        # be passed to CMake command
        instance.build_dir = link_path

        self.link_dir_counter += 1

