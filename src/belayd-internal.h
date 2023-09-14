// LICENSE TBD
/**
 * Internal include header file for belayd
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#ifndef __BELAYD_INTERNAL_H
#define __BELAYD_INTERNAL_H

#include <stdio.h>

#include "cause.h"
#include "effect.h"

struct rule {
	char *name;
	struct cause *causes;
	struct effect *effects;

	struct rule *next;
};

struct belayd_opts {
	/* options passed in on the command line */
	char config[FILENAME_MAX];
	int interval;

	/* internal settings and structures */
	struct rule *rules;
};

/*
 * parse.c functions
 */

int parse_string(struct json_object * const obj, const char * const key, const char **value);
int parse_config(struct belayd_opts * const opts);

#endif /* __BELAYD_INTERNAL_H */
