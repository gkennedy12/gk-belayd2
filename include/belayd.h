// LICENSE TBD
/**
 * Belayd Library
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#ifndef __BELAYD_H
#define __BELAYD_H

/**
 * Opaque structure that contains rule information
 */
struct rule;

/**
 * belayd configuration options
 */
struct belayd_opts {
	/* options passed in on the command line */
	char config[FILENAME_MAX];
	int interval;
	int max_loops;

	/* internal settings and structures */
	struct rule *rules;
};

/**
 * Main belayd processing loop
 * @param opts the belayd configuration options
 */
int belayd_loop(struct belayd_opts * const opts);

#endif /* __BELAYD_H */
