#### LICENSE TBD
#
# runnable plugins Makefile.am
#
# Copyright (c) 2023 Oracle and/or its affiliates.
# Author: 
#

check_PROGRAMS = \
	handle_pressure

LDADD = -ljson-c ${top_builddir}/src/libbelayd.la

handle_pressure_CFLAGS = -I${top_srcdir}/src/plugins/causes
handle_pressure_SOURCES = \
	handle_pressure.c \
	${top_srcdir}/src/plugins/causes/pressure_cause.c \
	${top_srcdir}/src/plugins/utils/pressure_utils.c

EXTRA_DIST_CFGS = \
	handle_pressure.json

EXTRA_DIST = 

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-rm -f *.pyc
