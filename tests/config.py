#### LICENSE TBD
#
# Config class for the belayd functional tests
#
# Copyright (c) 2019-2023 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

import consts
import utils
import os


class Config(object):
    def __init__(self, args):
        self.args = args
        self.skip_list = []

        self.ftest_dir = os.path.dirname(os.path.abspath(__file__))
        self.belayd_dir = os.path.dirname(self.ftest_dir)

        self.test_suite = consts.TESTS_RUN_ALL_SUITES
        self.test_num = consts.TESTS_RUN_ALL
        self.verbose = False

    def __str__(self):
        out_str = 'Configuration\n'
        out_str += utils.indent('args = {}'.format(self.args), 4)
        out_str += utils.indent('skip_list = {}'.format(self.skip_list.join(',')), 4)

        return out_str


class ConfigError(Exception):
    def __init__(self, message):
        super(ConfigError, self).__init__(message)

    def __str__(self):
        out_str = 'ConfigError:\n\tmessage = {}'.format(self.message)
        return out_str

# vim: set et ts=4 sw=4:
