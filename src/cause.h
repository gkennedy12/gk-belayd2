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

#include <belayd.h>

#include "defines.h"

enum cause_enum {
	TIME_OF_DAY = 0,
	DAYS_OF_THE_WEEK,

	CAUSE_CNT
};

struct belayd_cause {
	/* populated by belayd */
	enum cause_enum idx;
	char *name;
	const struct belayd_cause_functions *fns;
	struct belayd_cause *next;

	/* private data store for each cause plugin */
	void *data;
};

extern const char * const cause_names[];
extern const struct belayd_cause_functions cause_fns[];


int time_of_day_init(struct belayd_cause * const cse, struct json_object *cse_obj);
int time_of_day_main(struct belayd_cause * const cse, int time_since_last_run);
void time_of_day_exit(struct belayd_cause * const cse);
void time_of_day_print(const struct belayd_cause * const cse, FILE *file);

int days_of_the_week_init(struct belayd_cause * const cse, struct json_object *cse_obj);
int days_of_the_week_main(struct belayd_cause * const cse, int time_since_last_run);
void days_of_the_week_exit(struct belayd_cause * const cse);
void days_of_the_week_print(const struct belayd_cause * const cse, FILE *file);

#endif /* __BELAYD_CAUSE_H */
