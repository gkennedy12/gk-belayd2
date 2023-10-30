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

/*
 * Opaque structure that contains effect information
 */
struct belayd_effect;

/**
 * Opaque structure that contains the belayd context
 */
struct belayd_ctx;

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
 * Note that currently the belayd_loop() passes in ctx->interval in the time_since_last_run
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
 * Initialization routine for an effect
 * @param eff Effect structure for this effect
 * @param eff_obj JSON object representation of this effect
 * @param cse Cause structure this effect is acting upon
 */
typedef int (*belayd_effect_init)(struct belayd_effect * const eff, struct json_object *eff_obj,
			   const struct belayd_cause * const cse);
/**
 * Main processing logic for an effect
 * @param eff Effect structure for this effect
 */
typedef int (*belayd_effect_main)(struct belayd_effect * const eff);
/**
 * Exit routine for an effect
 * @param eff Effect structure for this effect
 */
typedef void (*belayd_effect_exit)(struct belayd_effect * const eff);

/**
 * Effect functions structure
 */
struct belayd_effect_functions {
	belayd_effect_init init;
	belayd_effect_main main;
	belayd_effect_exit exit;
};

/**
 * Initialize the belayd context structure
 * @param config_file Path to the configuration file to be used
 *
 * @return pointer to valid belayd context.  NULL on failure
 *
 * If config file is NULL, belayd will use the default config file,
 * /etc/belayd.json
 */
struct belayd_ctx *belayd_init(const char * const config_file);

/**
 * Destroy the belayd context and release any resources
 * @param ctx belayd context struct
 *
 * (*ctx) will be NULLed if it is a valid ctx pointer
 */
void belayd_release(struct belayd_ctx **ctx);

/**
 * Main belayd processing loop
 * @param ctx the belayd configuration context
 */
int belayd_loop(struct belayd_ctx * const ctx);

#endif /* __BELAYD_H */
