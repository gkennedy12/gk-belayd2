// LICENSE TBD
/**
 * Internal include header file for belayd
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#ifndef __BELAYD_INTERNAL_H
#define __BELAYD_INTERNAL_H

#include <syslog.h>
#include <stdio.h>

#include <belayd.h>

#include "cause.h"
#include "effect.h"

enum log_location {
	LOG_LOC_SYSLOG = 0,
	LOG_LOC_STDOUT,
	LOG_LOC_STDERR,

	LOG_LOC_CNT
};

struct rule {
	char *name;
	struct belayd_cause *causes;
	struct effect *effects;

	struct rule *next;
};

/*
 * log.c functions
 */

extern int log_level;
extern enum log_location log_loc;

void belayd_log(int priority, const char *fmt, ...);

#define belayd_err(msg...) \
	if (log_level >= LOG_ERR) \
		belayd_log(LOG_ERR, "Error: " msg)

#define belayd_wrn(msg...) \
	if (log_level >= LOG_WARNING) \
		belayd_log(LOG_WARNING, "Warning: " msg)

#define belayd_info(msg...) \
	if (log_level >= LOG_INFO) \
		belayd_log(LOG_INFO, "Info: " msg)

#define belayd_dbg(msg...) \
	if (log_level >= LOG_DEBUG) \
		belayd_log(LOG_DEBUG, "Debug: " msg)

/*
 * parse.c functions
 */

int parse_string(struct json_object * const obj, const char * const key, const char **value);
int parse_int(struct json_object * const obj, const char * const key, int * const value);
int parse_config(struct belayd_opts * const opts);

#endif /* __BELAYD_INTERNAL_H */
