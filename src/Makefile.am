SOURCES = \
	causes/time_of_day.c \
	cause.c \
	main.c \
	parse.c

belayd_SOURCES = ${SOURCES}
belayd_CFLAGS = ${AM_CFLAGS} ${CFLAGS}
belayd_LDFLAGS = ${AM_LDFLAGS} ${LDFLAGS} -ljson-c

sbin_PROGRAMS = belayd
