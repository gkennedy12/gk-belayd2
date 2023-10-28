// LICENSE TBD
/**
 * Belayd Library
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#ifndef __BELAYD_H
#define __BELAYD_H

#include <json-c/json.h>
#include <stdio.h>

/**
 * Opaque structure that contains rule information
 */
struct rule;

/*
 * Opaque structure that contains cause information
 */
struct belayd_cause;

/**
 * belayd configuration options
 */
struct belayd_opts {
	/* options passed in on the command line */
	char config[FILENAME_MAX];
	int interval; /* in seconds */
	int max_loops;

	/* internal settings and structures */
	struct rule *rules;
};

/**
 * Initialization routine for a cause
 * @param cse Cause structure for this cause
 * @param cse_obj JSON object representation of this cause
 */
typedef int (*belayd_cause_init)(struct belayd_cause * const cse, struct json_object *cse_obj);

/**
 * Main processing logic for a cause
 * @param cse Cause structure for this cause
 * @param time_since_last_run Delta time since this cause last ran
 *
 * Note that currently the belayd_loop() passes in opts->interval in the time_since_last_run
 * field.  In the future it could track the actual delta time between runs
 */
typedef int (*belayd_cause_main)(struct belayd_cause * const cse, int time_since_last_run);

/**
 * Exit routine for a cause
 * @param cse Cause structure for this cause
 *
 * This routine should clean up any memory allocated by the cause
 */
typedef void (*belayd_cause_exit)(struct belayd_cause * const cse);

/**
 * Print routine for a cause (OPTIONAL)
 * @param cse Cause structure for this cause
 * @param file Destination file where information is to be output
 */
typedef void (*belayd_cause_print)(const struct belayd_cause * const cse, FILE *file);

/**
 * Cause functions structure
 */
struct belayd_cause_functions {
	belayd_cause_init init;
	belayd_cause_main main;
	belayd_cause_exit exit;
	belayd_cause_print print;	/* implementing the print() function is optional */
};

/**
 * Main belayd processing loop
 * @param opts the belayd configuration options
 */
int belayd_loop(struct belayd_opts * const opts);

#endif /* __BELAYD_H */
