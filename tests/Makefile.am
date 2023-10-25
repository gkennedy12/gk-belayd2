#### LICENSE TBD
#
# belayd tests Makefile.am
#
# Copyright (c) 2023 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#
TESTS = ftests-wrapper.sh

EXTRA_DIST_PYTHON_FILES = \
	belayd.py \
	config.py \
	consts.py \
	ftests.py \
	log.py \
	run.py \
	utils.py

EXTRA_DIST_PYTHON_TESTS = \
	001-cause-time_of_day.py \
	002-cause-pressure-above.py \
	003-cause-pressure-below.py

EXTRA_DIST_PYTHON_CFGS = \
	001-cause-time_of_day.json.token \
	002-cause-pressure-above.json.token \
	003-cause-pressure-below.json.token

EXTRA_DIST = ftests-wrapper.sh ${EXTRA_DIST_PYTHON_FILES} \
	${EXTRA_DIST_PYTHON_TESTS} ${EXTRA_DIST_PYTHON_CFGS}

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-rm -f *.pyc
