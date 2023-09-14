// LICENSE TBD
/**
 * belayd causes header file
 *
 * "Causes" are events that trigger "Effects".  belayd
 * will periodically evaluate the causes, and if they fire
 * then the associated effects will be enforced.
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#ifndef __BELAYD_CAUSE_H
#define __BELAYD_CAUSE_H

#include <json-c/json.h>
#include <stdio.h>

#include "defines.h"

enum cause_enum {
	TIME_OF_DAY = 0,
	DAYS_OF_THE_WEEK,

	CAUSE_CNT
};

struct cause {
	/* populated by belayd */
	enum cause_enum idx;
	char *name;
	const struct cause_functions *fns;
	struct cause *next;

	/* private data store for each cause plugin */
	void *data;
};

typedef int (*cause_init)(struct cause * const cse, struct json_object *cse_obj);
typedef int (*cause_main)(struct cause * const cse, int time_since_last_run);
typedef void (*cause_exit)(struct cause * const cse);
typedef void (*cause_print)(const struct cause * const cse, FILE *file);

struct cause_functions {
	cause_init init;
	cause_main main;
	cause_exit exit;
	cause_print print;	/* implementing the print() function is optional */
};

extern const char * const cause_names[];
extern const struct cause_functions cause_fns[];


int time_of_day_init(struct cause * const cse, struct json_object *cse_obj);
int time_of_day_main(struct cause * const cse, int time_since_last_run);
void time_of_day_exit(struct cause * const cse);
void time_of_day_print(const struct cause * const cse, FILE *file);

int days_of_the_week_init(struct cause * const cse, struct json_object *cse_obj);
int days_of_the_week_main(struct cause * const cse, int time_since_last_run);
void days_of_the_week_exit(struct cause * const cse);
void days_of_the_week_print(const struct cause * const cse, FILE *file);

#endif /* __BELAYD_CAUSE_H */
