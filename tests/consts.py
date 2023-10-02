#### LICENSE TBD
#
# Constants for the libcgroup functional tests
#
# Note: based on libcgroup's ftests.py
#
# Copyright (c) 2019-2023 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

import os

DEFAULT_LOG_FILE = 'belayd-ftests.log'

LOG_CRITICAL = 1
LOG_WARNING = 5
LOG_DEBUG = 8
DEFAULT_LOG_LEVEL = 5

tests_dir = os.path.dirname(os.path.abspath(__file__))
BELAYD_MOUNT_POINT = os.path.dirname(tests_dir)

TESTS_RUN_ALL = -1
TESTS_RUN_ALL_SUITES = 'allsuites'
TEST_PASSED = 'passed'
TEST_FAILED = 'failed'
TEST_SKIPPED = 'skipped'

# vim: set et ts=4 sw=4:
