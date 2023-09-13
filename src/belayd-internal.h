// LICENSE TBD
/**
 * Internal include header file for belayd
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <stdio.h>

#include "cause.h"

struct rule {
	char *name;
	struct cause *causes;

	//struct effect *effects;
	//int effect_cnt;

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
 * config.c functions
 */

int parse_config(struct belayd_opts * const opts);
