#### LICENSE TBD
#
# belayd tests Makefile.am
#
# Copyright (c) 2023 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#
TESTS = ftests-wrapper.sh ftests-c-wrapper.sh

EXTRA_DIST_PYTHON_FILES = \
	belayd.py \
	config.py \
	consts.py \
	ftests.py \
	log.py \
	run.py \
	utils.py

check_PROGRAMS = \
	002-register_cause \
	003-register_plugin_cause \
	004-register_plugin_cause \
	005-register_plugin_cause \
	006-handle_pressure

LDADD = -ljson-c ${top_builddir}/src/libbelayd.la

003_register_plugin_cause_CFLAGS = -I${top_srcdir}/src/plugins/causes
003_register_plugin_cause_SOURCES = \
	003-register_plugin_cause.c \
	${top_srcdir}/src/plugins/causes/time_of_day.c

004_register_plugin_cause_CFLAGS = -I${top_srcdir}/src/plugins/causes
004_register_plugin_cause_SOURCES = \
	004-register_plugin_cause.c \
	${top_srcdir}/src/plugins/causes/pressure_cause.c \
	${top_srcdir}/src/plugins/utils/pressure_utils.c

005_register_plugin_cause_CFLAGS = -I${top_srcdir}/src/plugins/causes
005_register_plugin_cause_SOURCES = \
	005-register_plugin_cause.c \
	${top_srcdir}/src/plugins/causes/pressure_cause.c \
	${top_srcdir}/src/plugins/utils/pressure_utils.c

006_handle_pressure_CFLAGS = -I${top_srcdir}/src/plugins/causes
006_handle_pressure_SOURCES = \
	006-handle_pressure.c \
	${top_srcdir}/src/plugins/causes/pressure_cause.c \
	${top_srcdir}/src/plugins/utils/pressure_utils.c

EXTRA_DIST_PYTHON_TESTS = \
	001-cause-time_of_day.py

EXTRA_DIST_CFGS = \
	001-cause-time_of_day.json.token \
	002-register_cause.json \
	003-register_plugin_cause.json \
	004-register_plugin_cause.json \
	005-register_plugin_cause.json

EXTRA_DIST = \
	ftests-wrapper.sh \
	ftests-c-wrapper.sh \
	${EXTRA_DIST_PYTHON_FILES} \
	${EXTRA_DIST_PYTHON_TESTS} \
	${EXTRA_DIST_CFGS}

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-rm -f *.pyc
