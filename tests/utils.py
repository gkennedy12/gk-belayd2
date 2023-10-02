#### LICENSE TBD
#
# Utility functions for the belayd functional tests
#
# Copyright (c) 2020-2023 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

from run import Run
import datetime

NOW_TOKEN = '<< now'


# function to indent a block of text by cnt number of spaces
def indent(in_str, cnt):
    leading_indent = cnt * ' '
    return ''.join(leading_indent + line for line in in_str.splitlines(True))


def __parse_now_token(token):
    math_op = int(token[len(NOW_TOKEN) : len(token) - len('>>')].replace(' ', ''))

    token_time = datetime.datetime.now() + datetime.timedelta(seconds=math_op)
    token_time_str = token_time.strftime('%H:%M:%S')
    return token_time_str


# Find and replace magic tokens in config files.  The caller is expected to
# delete the out_filename during cleanup.
def config_find_and_replace(in_filename, out_filename):
    tokens = [NOW_TOKEN]
    parsers = ['__parse_now_token']

    with open(in_filename) as ifile:
        with open(out_filename, 'w') as ofile:

            for line in ifile.readlines():
                for idx, token in enumerate(tokens):
                    token_idx = line.find(token)
                    if token_idx >= 0:
                        end_idx = token_idx + line[token_idx:].index('>>') + 2
                        parsed_val = eval(parsers[idx])(line[token_idx : end_idx])

                        ofile.write('{}{}{}'.format(line[:token_idx], parsed_val,
                                                    line[end_idx:]))
                    else:
                        ofile.write(line)


# vim: set et ts=4 sw=4:
