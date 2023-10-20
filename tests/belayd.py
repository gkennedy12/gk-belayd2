#!/usr/bin/env python3
#### LICENSE TBD
#
# Interface into the belayd daemon for the belayd functional tests
#
# Copyright (c) 2023 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

from run import Run, RunError
import consts
import utils
import os


def belayd(config=None, bhelp=False, interval=None, log_location=None,
           log_level=None, max_loops=None, expected_ret=None):
    """run the belayd daemon
    """
    cmd = list()

    cmd.append(os.path.join(consts.BELAYD_MOUNT_POINT, 'src/belayd'))

    if config:
        tmp_config = config
        token = config.find('.token')
        if token > 0:
            tmp_config = config[:token]
            utils.config_find_and_replace(config, tmp_config)

        cmd.append('-c')
        cmd.append(tmp_config)

    if bhelp:
        cmd.append('-h')

    if interval:
        cmd.append('-i')
        cmd.append(str(interval))

    if log_location:
        cmd.append('-L')
        cmd.append(log_location)

    if log_level:
        cmd.append('-l')
        cmd.append(str(log_level))

    if max_loops:
        cmd.append('-m')
        cmd.append(str(max_loops))

    try:
        print("XXX belayd.py cmd:", cmd);
        Run.run(cmd)
    except RunError as re:
        if re.ret == expected_ret:
            pass
        else:
            raise re
    finally:
        if tmp_config != config:
            os.remove(tmp_config)
