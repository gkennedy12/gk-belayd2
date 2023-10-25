@CODE_COVERAGE_RULES@

SOURCES = \
	belayd-internal.h \
	causes/days_of_the_week.c \
	causes/time_of_day.c \
	causes/pressure.c \
	cause.c \
	cause.h \
	defines.h \
	utils/pressure_utils.c \
	effects/print.c \
	effects/validate.c \
	effect.c \
	effect.h \
	log.c \
	main.c \
	parse.c

belayd_SOURCES = ${SOURCES}
belayd_CFLAGS = ${AM_CFLAGS} ${CFLAGS}  ${CODE_COVERAGE_CFLAGS}
belayd_LDFLAGS = ${AM_LDFLAGS} ${LDFLAGS} ${CODE_COVERAGE_LIBS} -ljson-c

sbin_PROGRAMS = belayd
