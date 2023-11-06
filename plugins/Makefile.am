@CODE_COVERAGE_RULES@

SOURCES = \
	belayd-internal.h \
	causes/days_of_the_week.c \
	causes/time_of_day.c \
	cause.c \
	cause.h \
	defines.h \
	effects/print.c \
	effects/validate.c \
	effect.c \
	effect.h \
	plugins/causes/pressure_cause.c \
	log.c \
	main.c \
	parse.c

belayd_SOURCES = ${SOURCES}
belayd_CFLAGS = ${AM_CFLAGS} ${CFLAGS}  ${CODE_COVERAGE_CFLAGS}
belayd_LDFLAGS = ${AM_LDFLAGS} ${LDFLAGS} ${CODE_COVERAGE_LIBS} -ljson-c

libbelayd_la_SOURCES = ${SOURCES}
libbelayd_la_LIBADD = ${CODE_COVERAGE_LIBS} -ljson-c
libbelayd_la_CFLAGS = ${AM_CFLAGS} ${CFLAGS} ${CODE_COVERAGE_CFLAGS} -fPIC
lib_LTLIBRARIES = libbelayd.la

sbin_PROGRAMS = belayd
