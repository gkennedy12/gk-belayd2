// LICENSE TBD
/**
 * Logging for belayd
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <syslog.h>

#include "belayd-internal.h"

int log_level = LOG_ERR;
enum log_location log_loc = LOG_LOC_STDERR;

void belayd_log(int priority, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	switch(log_loc) {
		case LOG_LOC_SYSLOG:
			vsyslog(priority, fmt, ap);
			break;
		case LOG_LOC_STDOUT:
			vfprintf(stderr, fmt, ap);
			break;
		case LOG_LOC_STDERR:
			vfprintf(stderr, fmt, ap);
			break;
		default:
			assert(true);
			break;
	}
	va_end(ap);
}
