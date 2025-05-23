#!/usr/bin/env python3
# vim: set syntax=python ts=4 :
#
# Copyright (c) 2018 Intel Corporation
# Copyright 2022 NXP
# SPDX-License-Identifier: Apache-2.0

import os
import pkg_resources
import sys
from pathlib import Path
import json
import logging
import subprocess
import shutil
import re
import argparse
from datetime import datetime, timezone

logger = logging.getLogger('twister')
logger.setLevel(logging.DEBUG)

from twisterlib.error import TwisterRuntimeError

ZEPHYR_BASE = os.getenv("ZEPHYR_BASE")
if not ZEPHYR_BASE:
    sys.exit("$ZEPHYR_BASE environment variable undefined")

sys.path.insert(0, os.path.join(ZEPHYR_BASE, "scripts/"))

import zephyr_module

# Use this for internal comparisons; that's what canonicalization is
# for. Don't use it when invoking other components of the build system
# to avoid confusing and hard to trace inconsistencies in error messages
# and logs, generated Makefiles, etc. compared to when users invoke these
# components directly.
# Note "normalization" is different from canonicalization, see os.path.
canonical_zephyr_base = os.path.realpath(ZEPHYR_BASE)

installed_packages = [pkg.project_name for pkg in pkg_resources.working_set]  # pylint: disable=not-an-iterable
PYTEST_PLUGIN_INSTALLED = 'pytest-twister-harness' in installed_packages


def add_parse_arguments(parser = None):
    if parser is None:
        parser = argparse.ArgumentParser(
            description=__doc__,
            formatter_class=argparse.RawDescriptionHelpFormatter,
            allow_abbrev=False)
    parser.fromfile_prefix_chars = "+"

    case_select = parser.add_argument_group("Test case selection",
                                            """
Artificially long but functional example:
    $ ./scripts/twister -v     \\
      --testsuite-root tests/ztest/base    \\
      --testsuite-root tests/kernel   \\
      --test      tests/ztest/base/testing.ztest.verbose_0  \\
      --test      tests/kernel/fifo/fifo_api/kernel.fifo

   "kernel.fifo.poll" is one of the test section names in
                                 __/fifo_api/testcase.yaml
    """)

    compare_group_option = parser.add_mutually_exclusive_group()

    platform_group_option = parser.add_mutually_exclusive_group()

    run_group_option = parser.add_mutually_exclusive_group()

    device = parser.add_mutually_exclusive_group(required="--device-testing" in sys.argv)

    test_or_build = parser.add_mutually_exclusive_group()

    test_xor_subtest = case_select.add_mutually_exclusive_group()

    test_xor_generator = case_select.add_mutually_exclusive_group()

    valgrind_asan_group = parser.add_mutually_exclusive_group()

    case_select.add_argument(
        "-E",
        "--save-tests",
        metavar="FILENAME",
        action="store",
        help="Append list of tests and platforms to be run to file.")

    case_select.add_argument(
        "-F",
        "--load-tests",
        metavar="FILENAME",
        action="store",
        help="Load list of tests and platforms to be run from file.")

    case_select.add_argument(
        "-T", "--testsuite-root", action="append", default=[],
        help="Base directory to recursively search for test cases. All "
             "testcase.yaml files under here will be processed. May be "
             "called multiple times. Defaults to the 'samples/' and "
             "'tests/' directories at the base of the Zephyr tree.")

    case_select.add_argument(
        "-f",
        "--only-failed",
        action="store_true",
        help="Run only those tests that failed the previous twister run "
             "invocation.")

    case_select.add_argument("--list-tests", action="store_true",
                             help="""List of all sub-test functions recursively found in
        all --testsuite-root arguments. Note different sub-tests can share
        the same section name and come from different directories.
        The output is flattened and reports --sub-test names only,
        not their directories. For instance net.socket.getaddrinfo_ok
        and net.socket.fd_set belong to different directories.
        """)

    case_select.add_argument("--test-tree", action="store_true",
                             help="""Output the test plan in a tree form""")

    compare_group_option.add_argument("--compare-report",
                        help="Use this report file for size comparison")

    compare_group_option.add_argument(
        "-m", "--last-metrics", action="store_true",
        help="Compare with the results of the previous twister "
             "invocation")

    platform_group_option.add_argument(
        "-G",
        "--integration",
        action="store_true",
        help="Run integration tests")

    platform_group_option.add_argument(
        "--emulation-only", action="store_true",
        help="Only build and run emulation platforms")

    run_group_option.add_argument(
        "--device-testing", action="store_true",
        help="Test on device directly. Specify the serial device to "
             "use with the --device-serial option.")

    run_group_option.add_argument("--generate-hardware-map",
                        help="""Probe serial devices connected to this platform
                        and create a hardware map file to be used with
                        --device-testing
                        """)

    device.add_argument("--device-serial",
                        help="""Serial device for accessing the board
                        (e.g., /dev/ttyACM0)
                        """)

    device.add_argument("--device-serial-pty",
                        help="""Script for controlling pseudoterminal.
                        Twister believes that it interacts with a terminal
                        when it actually interacts with the script.

                        E.g "twister --device-testing
                        --device-serial-pty <script>
                        """)

    device.add_argument("--hardware-map",
                        help="""Load hardware map from a file. This will be used
                        for testing on hardware that is listed in the file.
                        """)

    parser.add_argument("--device-flash-timeout", type=int, default=60,
                        help="""Set timeout for the device flash operation in seconds.
                        """)

    parser.add_argument("--device-flash-with-test", action="store_true",
                        help="""Add a test case timeout to the flash operation timeout
                        when flash operation also executes test case on the platform.
                        """)

    test_or_build.add_argument(
        "-b", "--build-only", action="store_true", default="--prep-artifacts-for-testing" in sys.argv,
        help="Only build the code, do not attempt to run the code on targets.")

    test_or_build.add_argument(
        "--prep-artifacts-for-testing", action="store_true",
        help="Generate artifacts for testing, do not attempt to run the"
              "code on targets.")

    parser.add_argument(
        "--package-artifacts",
        help="Package artifacts needed for flashing in a file to be used with --test-only"
        )

    test_or_build.add_argument(
        "--test-only", action="store_true",
        help="""Only run device tests with current artifacts, do not build
             the code""")

    parser.add_argument("--timeout-multiplier", type=float, default=1,
        help="""Globally adjust tests timeouts by specified multiplier. The resulting test
        timeout would be multiplication of test timeout value, board-level timeout multiplier
        and global timeout multiplier (this parameter)""")

    test_xor_subtest.add_argument(
        "-s", "--test", action="append",
        help="Run only the specified testsuite scenario. These are named by "
             "<path/relative/to/Zephyr/base/section.name.in.testcase.yaml>")

    test_xor_subtest.add_argument(
        "--sub-test", action="append",
        help="""Recursively find sub-test functions and run the entire
        test section where they were found, including all sibling test
        functions. Sub-tests are named by:
        section.name.in.testcase.yaml.function_name_without_test_prefix
        Example: In kernel.fifo.fifo_loop: 'kernel.fifo' is a section name
        and 'fifo_loop' is a name of a function found in main.c without test prefix.
        """)

    valgrind_asan_group.add_argument(
        "--enable-valgrind", action="store_true",
        help="""Run binary through valgrind and check for several memory access
        errors. Valgrind needs to be installed on the host. This option only
        works with host binaries such as those generated for the native_posix
        configuration and is mutual exclusive with --enable-asan.
        """)

    valgrind_asan_group.add_argument(
        "--enable-asan", action="store_true",
        help="""Enable address sanitizer to check for several memory access
        errors. Libasan needs to be installed on the host. This option only
        works with host binaries such as those generated for the native_posix
        configuration and is mutual exclusive with --enable-valgrind.
        """)

    # Start of individual args place them in alpha-beta order

    board_root_list = ["%s/boards" % ZEPHYR_BASE,
                       "%s/scripts/pylib/twister/boards" % ZEPHYR_BASE]

    modules = zephyr_module.parse_modules(ZEPHYR_BASE)
    for module in modules:
        board_root = module.meta.get("build", {}).get("settings", {}).get("board_root")
        if board_root:
            board_root_list.append(os.path.join(module.project, board_root, "boards"))

    parser.add_argument(
        "-A", "--board-root", action="append", default=board_root_list,
        help="""Directory to search for board configuration files. All .yaml
files in the directory will be processed. The directory should have the same
structure in the main Zephyr tree: boards/<arch>/<board_name>/""")

    parser.add_argument(
        "--allow-installed-plugin", action="store_true", default=None,
        help="Allow to use pytest plugin installed by pip for pytest tests."
    )

    parser.add_argument(
        "-a", "--arch", action="append",
        help="Arch filter for testing. Takes precedence over --platform. "
             "If unspecified, test all arches. Multiple invocations "
             "are treated as a logical 'or' relationship")

    parser.add_argument(
        "-B", "--subset",
        help="Only run a subset of the tests, 1/4 for running the first 25%%, "
             "3/5 means run the 3rd fifth of the total. "
             "This option is useful when running a large number of tests on "
             "different hosts to speed up execution time.")

    parser.add_argument(
        "--shuffle-tests", action="store_true", default=None,
        help="""Shuffle test execution order to get randomly distributed tests across subsets.
                Used only when --subset is provided.""")

    parser.add_argument(
        "--shuffle-tests-seed", action="store", default=None,
        help="""Seed value for random generator used to shuffle tests.
                If not provided, seed in generated by system.
                Used only when --shuffle-tests is provided.""")

    parser.add_argument("-C", "--coverage", action="store_true",
                        help="Generate coverage reports. Implies "
                             "--enable-coverage.")

    parser.add_argument(
        "-c", "--clobber-output", action="store_true",
        help="Cleaning the output directory will simply delete it instead "
             "of the default policy of renaming.")

    parser.add_argument(
        "--cmake-only", action="store_true",
        help="Only run cmake, do not build or run.")

    parser.add_argument("--coverage-basedir", default=ZEPHYR_BASE,
                        help="Base source directory for coverage report.")

    parser.add_argument("--coverage-platform", action="append", default=[],
                        help="Platforms to run coverage reports on. "
                             "This option may be used multiple times. "
                             "Default to what was selected with --platform.")

    parser.add_argument("--coverage-tool", choices=['lcov', 'gcovr'], default='lcov',
                        help="Tool to use to generate coverage report.")

    parser.add_argument("--coverage-formats", action="store", default=None, # default behavior is set in run_coverage
                        help="Output formats to use for generated coverage reports, as a comma-separated list. "
                             "Default to html. "
                             "Valid options are html, xml, csv, txt, coveralls, sonarqube, lcov.")

    parser.add_argument("--test-config", action="store", default=os.path.join(ZEPHYR_BASE, "testsuites", "test_config.yaml"),
        help="Path to file with plans and test configurations.")

    parser.add_argument("--level", action="store",
        help="Test level to be used. By default, no levels are used for filtering"
             "and do the selection based on existing filters.")

    parser.add_argument(
        "-D", "--all-deltas", action="store_true",
        help="Show all footprint deltas, positive or negative. Implies "
             "--footprint-threshold=0")

    parser.add_argument(
        "--device-serial-baud", action="store", default=None,
        help="Serial device baud rate (default 115200)")
    
    parser.add_argument(
        "--tftp-path", action="store", default=None, required=True,
        help="the tftp serve file path")
    
    parser.add_argument(
        "--board-ip", action="store", default=None, required=False,
        help="the board ip")

    parser.add_argument(
        "--board-password", action="store", default="autotest123.", required=False,
        help="the board ssh password")
    
    parser.add_argument(
        "--deployment-type", action="store", default="bare_metal", required=False,
        help="the deployment type")

    parser.add_argument("--disable-asserts", action="store_false",
                        dest="enable_asserts",
                        help="deprecated, left for compatibility")

    parser.add_argument(
        "--disable-unrecognized-section-test", action="store_true",
        default=False,
        help="Skip the 'unrecognized section' test.")

    parser.add_argument(
        "--disable-suite-name-check", action="store_true", default=False,
        help="Disable extended test suite name verification at the beginning "
             "of Ztest test. This option could be useful for tests or "
             "platforms, which from some reasons cannot print early logs.")

    parser.add_argument("-e", "--exclude-tag", action="append",
                        help="Specify tags of tests that should not run. "
                             "Default is to run all tests with all tags.")

    parser.add_argument("--enable-coverage", action="store_true",
                        help="Enable code coverage using gcov.")

    parser.add_argument(
        "--enable-lsan", action="store_true",
        help="""Enable leak sanitizer to check for heap memory leaks.
        Libasan needs to be installed on the host. This option only
        works with host binaries such as those generated for the native_posix
        configuration and when --enable-asan is given.
        """)

    parser.add_argument(
        "--enable-ubsan", action="store_true",
        help="""Enable undefined behavior sanitizer to check for undefined
        behaviour during program execution. It uses an optional runtime library
        to provide better error diagnostics. This option only works with host
        binaries such as those generated for the native_posix configuration.
        """)

    parser.add_argument("--enable-size-report", action="store_true",
                        help="Enable expensive computation of RAM/ROM segment sizes.")

    parser.add_argument(
        "--filter", choices=['buildable', 'runnable'],
        default='buildable',
        help="""Filter tests to be built and executed. By default everything is
        built and if a test is runnable (emulation or a connected device), it
        is run. This option allows for example to only build tests that can
        actually be run. Runnable is a subset of buildable.""")

    parser.add_argument("--force-color", action="store_true",
                        help="Always output ANSI color escape sequences "
                             "even when the output is redirected (not a tty)")

    parser.add_argument("--force-toolchain", action="store_true",
                        help="Do not filter based on toolchain, use the set "
                             " toolchain unconditionally")

    parser.add_argument("--gcov-tool", type=Path, default=None,
                        help="Path to the gcov tool to use for code coverage "
                             "reports")

    parser.add_argument(
        "-H", "--footprint-threshold", type=float, default=5,
        help="When checking test case footprint sizes, warn the user if "
             "the new app size is greater then the specified percentage "
             "from the last release. Default is 5. 0 to warn on any "
             "increase on app size.")

    parser.add_argument(
        "-i", "--inline-logs", action="store_true",
        help="Upon test failure, print relevant log data to stdout "
             "instead of just a path to it.")

    parser.add_argument("--ignore-platform-key", action="store_true",
                        help="Do not filter based on platform key")

    parser.add_argument(
        "-j", "--jobs", type=int,
        help="Number of jobs for building, defaults to number of CPU threads, "
             "overcommitted by factor 2 when --build-only.")

    parser.add_argument(
        "-K", "--force-platform", action="store_true",
        help="""Force testing on selected platforms,
        even if they are excluded in the test configuration (testcase.yaml)."""
    )

    parser.add_argument(
        "-l", "--all", action="store_true",
        help="Build/test on all platforms. Any --platform arguments "
             "ignored.")

    parser.add_argument("--list-tags", action="store_true",
                        help="List all tags occurring in selected tests.")

    parser.add_argument("--log-file", metavar="FILENAME", action="store",
                        help="Specify a file where to save logs.")

    parser.add_argument(
        "-M", "--runtime-artifact-cleanup", choices=['pass', 'all'],
        default=None, const='pass', nargs='?',
        help="""Cleanup test artifacts. The default behavior is 'pass'
        which only removes artifacts of passing tests. If you wish to
        remove all artificats including those of failed tests, use 'all'.""")

    test_xor_generator.add_argument(
        "-N", "--ninja", action="store_true",
        default=not any(a in sys.argv for a in ("-k", "--make")),
        help="Use the Ninja generator with CMake. (This is the default)")

    test_xor_generator.add_argument(
        "-k", "--make", action="store_true",
        help="Use the unix Makefile generator with CMake.")

    parser.add_argument(
        "-n", "--no-clean", action="store_true",
        help="Re-use the outdir before building. Will result in "
             "faster compilation since builds will be incremental.")

    parser.add_argument(
        '--detailed-test-id', action='store_true',
        help="Include paths to tests' locations in tests' names. Names will follow "
             "PATH_TO_TEST/SCENARIO_NAME schema "
             "e.g. samples/hello_world/sample.basic.helloworld")

    parser.add_argument(
        "--no-detailed-test-id", dest='detailed_test_id', action="store_false",
        help="Don't put paths into tests' names. "
             "With this arg a test name will be a scenario name "
             "e.g. sample.basic.helloworld.")

    # Include paths in names by default.
    parser.set_defaults(detailed_test_id=True)

    # To be removed in favor of --detailed-skipped-report
    parser.add_argument(
        "--no-skipped-report", action="store_true",
        help="""Do not report skipped test cases in junit output. [Experimental]
        """)

    parser.add_argument(
        "--detailed-skipped-report", action="store_true",
        help="Generate a detailed report with all skipped test cases"
             "including those that are filtered based on testsuite definition."
        )

    parser.add_argument(
        "-O", "--outdir",
        default=os.path.join(os.getcwd(), "twister-out"),
        help="Output directory for logs and binaries. "
             "Default is 'twister-out' in the current directory. "
             "This directory will be cleaned unless '--no-clean' is set. "
             "The '--clobber-output' option controls what cleaning does.")

    parser.add_argument(
        "-o", "--report-dir",
        help="""Output reports containing results of the test run into the
        specified directory.
        The output will be both in JSON and JUNIT format
        (twister.json and twister.xml).
        """)

    parser.add_argument("--overflow-as-errors", action="store_true",
                        help="Treat RAM/SRAM overflows as errors.")


    parser.add_argument("-P", "--exclude-platform", action="append", default=[],
            help="""Exclude platforms and do not build or run any tests
            on those platforms. This option can be called multiple times.
            """
            )

    parser.add_argument("--persistent-hardware-map", action='store_true',
                        help="""With --generate-hardware-map, tries to use
                        persistent names for serial devices on platforms
                        that support this feature (currently only Linux).
                        """)

    parser.add_argument(
            "--vendor", action="append", default=[],
            help="Vendor filter for testing")

    parser.add_argument(
        "-p", "--platform", action="append", default=[],
        help="Platform filter for testing. This option may be used multiple "
             "times. Test suites will only be built/run on the platforms "
             "specified. If this option is not used, then platforms marked "
             "as default in the platform metadata file will be chosen "
             "to build and test. ")

    parser.add_argument(
        "--platform-reports", action="store_true",
        help="""Create individual reports for each platform.
        """)

    parser.add_argument("--pre-script",
                        help="""specify a pre script. This will be executed
                        before device handler open serial port and invoke runner.
                        """)

    parser.add_argument("-Q", "--error-on-deprecations", action="store_false",
                        help="Error on deprecation warnings.")

    parser.add_argument(
        "--quarantine-list",
        action="append",
        metavar="FILENAME",
        help="Load list of test scenarios under quarantine. The entries in "
             "the file need to correspond to the test scenarios names as in "
             "corresponding tests .yaml files. These scenarios "
             "will be skipped with quarantine as the reason.")

    parser.add_argument(
        "--quarantine-verify",
        action="store_true",
        help="Use the list of test scenarios under quarantine and run them"
             "to verify their current status.")

    parser.add_argument("-R", "--enable-asserts", action="store_true",
                        default=True,
                        help="deprecated, left for compatibility")

    parser.add_argument("--report-excluded",
                        action="store_true",
                        help="""List all tests that are never run based on current scope and
            coverage. If you are looking for accurate results, run this with
            --all, but this will take a while...""")

    parser.add_argument(
        "--report-name",
        help="""Create a report with a custom name.
        """)

    parser.add_argument(
        "--report-suffix",
        help="""Add a suffix to all generated file names, for example to add a
        version or a commit ID.
        """)

    parser.add_argument(
        "--retry-failed", type=int, default=0,
        help="Retry failing tests again, up to the number of times specified.")

    parser.add_argument(
        "--retry-interval", type=int, default=60,
        help="Retry failing tests after specified period of time.")

    parser.add_argument(
        "--retry-build-errors", action="store_true",
        help="Retry build errors as well.")

    parser.add_argument(
        "-S", "--enable-slow", action="store_true",
        default="--enable-slow-only" in sys.argv,
        help="Execute time-consuming test cases that have been marked "
             "as 'slow' in testcase.yaml. Normally these are only built.")

    parser.add_argument(
        "--enable-slow-only", action="store_true",
        help="Execute time-consuming test cases that have been marked "
             "as 'slow' in testcase.yaml only. This also set the option --enable-slow")

    parser.add_argument(
        "--seed", type=int,
        help="Seed for native posix pseudo-random number generator")

    parser.add_argument(
        "--short-build-path",
        action="store_true",
        help="Create shorter build directory paths based on symbolic links. "
             "The shortened build path will be used by CMake for generating "
             "the build system and executing the build. Use this option if "
             "you experience build failures related to path length, for "
             "example on Windows OS. This option can be used only with "
             "'--ninja' argument (to use Ninja build generator).")

    parser.add_argument(
        "--show-footprint",
        action="store_true",
        required = "--footprint-from-buildlog" in sys.argv,
        help="Show footprint statistics and deltas since last release."
    )

    parser.add_argument(
        "-t", "--tag", action="append",
        help="Specify tags to restrict which tests to run by tag value. "
             "Default is to not do any tag filtering. Multiple invocations "
             "are treated as a logical 'or' relationship.")

    parser.add_argument("--timestamps",
                        action="store_true",
                        help="Print all messages with time stamps.")

    parser.add_argument(
        "-u",
        "--no-update",
        action="store_true",
         help="Do not update the results of the last run of twister.")

    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help="Emit debugging information, call multiple times to increase "
             "verbosity.")

    parser.add_argument("-W", "--disable-warnings-as-errors", action="store_true",
                        help="Do not treat warning conditions as errors.")

    parser.add_argument(
        "--west-flash", nargs='?', const=[],
        help="""Uses west instead of ninja or make to flash when running with
             --device-testing. Supports comma-separated argument list.

        E.g "twister --device-testing --device-serial /dev/ttyACM0
                         --west-flash="--board-id=foobar,--erase"
        will translate to "west flash -- --board-id=foobar --erase"

        NOTE: device-testing must be enabled to use this option.
        """
    )
    parser.add_argument(
        "--west-runner",
        help="""Uses the specified west runner instead of default when running
             with --west-flash.

        E.g "twister --device-testing --device-serial /dev/ttyACM0
                         --west-flash --west-runner=pyocd"
        will translate to "west flash --runner pyocd"

        NOTE: west-flash must be enabled to use this option.
        """
    )

    parser.add_argument(
        "-X", "--fixture", action="append", default=[],
        help="Specify a fixture that a board might support.")

    parser.add_argument(
        "-x", "--extra-args", action="append", default=[],
        help="""Extra CMake cache entries to define when building test cases.
        May be called multiple times. The key-value entries will be
        prefixed with -D before being passed to CMake.
        E.g
        "twister -x=USE_CCACHE=0"
        will translate to
        "cmake -DUSE_CCACHE=0"
        which will ultimately disable ccache.
        """
    )

    parser.add_argument(
        "-y", "--dry-run", action="store_true",
        help="""Create the filtered list of test cases, but don't actually
        run them. Useful if you're just interested in the test plan
        generated for every run and saved in the specified output
        directory (testplan.json).
        """)

    parser.add_argument(
        "-z", "--size", action="append",
        help="Don't run twister. Instead, produce a report to "
             "stdout detailing RAM/ROM sizes on the specified filenames. "
             "All other command line arguments ignored.")

    parser.add_argument(
        "--footprint-from-buildlog",
        action = "store_true",
        help="Get information about memory footprint from generated build.log. "
             "Requires using --show-footprint option.")

    parser.add_argument("extra_test_args", nargs=argparse.REMAINDER,
        help="Additional args following a '--' are passed to the test binary")

    parser.add_argument("--alt-config-root", action="append", default=[],
        help="Alternative test configuration root/s. When a test is found, "
             "Twister will check if a test configuration file exist in any of "
             "the alternative test configuration root folders. For example, "
             "given $test_root/tests/foo/testcase.yaml, Twister will use "
             "$alt_config_root/tests/foo/testcase.yaml if it exists")

    return parser


def parse_arguments(parser, args, options = None):
    if options is None:
        options = parser.parse_args(args)

    # Very early error handling
    if options.short_build_path and not options.ninja:
        logger.error("--short-build-path requires Ninja to be enabled")
        sys.exit(1)

    if options.device_serial_pty and os.name == "nt":  # OS is Windows
        logger.error("--device-serial-pty is not supported on Windows OS")
        sys.exit(1)

    if options.west_runner and options.west_flash is None:
        logger.error("west-runner requires west-flash to be enabled")
        sys.exit(1)

    if options.west_flash and not options.device_testing:
        logger.error("west-flash requires device-testing to be enabled")
        sys.exit(1)

    if not options.testsuite_root:
        options.testsuite_root = [os.path.join(ZEPHYR_BASE, "tests"),
                                 os.path.join(ZEPHYR_BASE, "samples")]

    if options.show_footprint or options.compare_report:
        options.enable_size_report = True

    if options.coverage:
        options.enable_coverage = True

    if not options.coverage_platform:
        options.coverage_platform = options.platform

    if options.enable_valgrind and not shutil.which("valgrind"):
        logger.error("valgrind enabled but valgrind executable not found")
        sys.exit(1)

    if options.device_testing and (options.device_serial or options.device_serial_pty) and len(options.platform) > 1:
        logger.error("""When --device-testing is used with
                        --device-serial or --device-serial-pty,
                        only one platform is allowed""")
        sys.exit(1)

    if options.device_flash_with_test and not options.device_testing:
        logger.error("--device-flash-with-test requires --device_testing")
        sys.exit(1)

    if options.shuffle_tests and options.subset is None:
        logger.error("--shuffle-tests requires --subset")
        sys.exit(1)

    if options.shuffle_tests_seed and options.shuffle_tests is None:
        logger.error("--shuffle-tests-seed requires --shuffle-tests")
        sys.exit(1)

    if len(options.extra_test_args) > 0:
        # extra_test_args is a list of CLI args that Twister did not recognize
        # and are intended to be passed through to the ztest executable. This
        # list should begin with a "--". If not, there is some extra
        # unrecognized arg(s) that shouldn't be there. Tell the user there is a
        # syntax error.
        if options.extra_test_args[0] != "--":
            try:
                double_dash = options.extra_test_args.index("--")
            except ValueError:
                double_dash = len(options.extra_test_args)
            unrecognized = " ".join(options.extra_test_args[0:double_dash])

            logger.error("Unrecognized arguments found: '%s'. Use -- to "
                         "delineate extra arguments for test binary or pass "
                         "-h for help.",
                         unrecognized)

            sys.exit(1)

        # Strip off the initial "--" following validation.
        options.extra_test_args = options.extra_test_args[1:]

    if not options.allow_installed_plugin and PYTEST_PLUGIN_INSTALLED:
        logger.error("By default Twister should work without pytest-twister-harness "
                     "plugin being installed, so please, uninstall it by "
                     "`pip uninstall pytest-twister-harness` and `git clean "
                     "-dxf scripts/pylib/pytest-twister-harness`.")
        sys.exit(1)
    elif options.allow_installed_plugin and PYTEST_PLUGIN_INSTALLED:
        logger.warning("You work with installed version of "
                       "pytest-twister-harness plugin.")

    return options


class TwisterEnv:

    def __init__(self, options=None) -> None:
        self.version = "Unknown"
        self.toolchain = None
        self.commit_date = "Unknown"
        self.run_date = None
        self.options = options

        if options and options.ninja:
            self.generator_cmd = "ninja"
            self.generator = "Ninja"
        else:
            self.generator_cmd = "make"
            self.generator = "Unix Makefiles"
        logger.info(f"Using {self.generator}..")

        self.test_roots = options.testsuite_root if options else None

        if options:
            if not isinstance(options.board_root, list):
                self.board_roots = [self.options.board_root]
            else:
                self.board_roots = self.options.board_root
            self.outdir = os.path.abspath(options.outdir)
        else:
            self.board_roots = None
            self.outdir = None

        self.hwm = None

        self.test_config = options.test_config if options else None

        self.alt_config_root = options.alt_config_root if options else None

    def discover(self):
        self.check_zephyr_version()
        # self.get_toolchain()
        self.run_date = datetime.now(timezone.utc).isoformat(timespec='seconds')

    def check_zephyr_version(self):
        try:
            subproc = subprocess.run(["git", "describe", "--abbrev=12", "--always"],
                                     stdout=subprocess.PIPE,
                                     universal_newlines=True,
                                     cwd=ZEPHYR_BASE)
            if subproc.returncode == 0:
                _version = subproc.stdout.strip()
                if _version:
                    self.version = _version
                    logger.info(f"Zephyr version: {self.version}")
        except OSError:
            logger.exception("Failure while reading Zephyr version.")

        if self.version == "Unknown":
            logger.warning("Could not determine version")

        try:
            subproc = subprocess.run(["git", "show", "-s", "--format=%cI", "HEAD"],
                                        stdout=subprocess.PIPE,
                                        universal_newlines=True,
                                        cwd=ZEPHYR_BASE)
            if subproc.returncode == 0:
                self.commit_date = subproc.stdout.strip()
        except OSError:
            logger.exception("Failure while reading head commit date.")

