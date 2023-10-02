#!/usr/bin/env python3
#### LICENSE TBD
#
# Test the time-of-day cause
#
# Copyright (c) 2023 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

import belayd
import consts

CONFIG = '001-cause-time_of_day.json.token'
INTERVAL = 1
MAX_LOOPS = 10
EXPECTED_RET = 42

def prereqs(config):
    result = consts.TEST_PASSED
    cause = None

    return result, cause


def setup(config):
    pass


def test(config):
    result = consts.TEST_PASSED
    cause = None

    belayd.belayd(config=CONFIG, interval=INTERVAL, max_loops=MAX_LOOPS,
                  expected_ret=EXPECTED_RET)
    return result, cause


def teardown(config):
    pass


def main(config):
    [result, cause] = prereqs(config)
    if result != consts.TEST_PASSED:
        return [result, cause]

    try:
        setup(config)
        [result, cause] = test(config)
    finally:
        teardown(config)

    return [result, cause]


if __name__ == '__main__':
    config = ftests.parse_args()
    # this test was invoked directly.  run only it
    config.args.num = int(os.path.basename(__file__).split('-')[0])
    sys.exit(ftests.main(config))

# vim: set et ts=4 sw=4:
