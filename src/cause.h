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

#include <json-c/json.h>

#include "defines.h"

enum cause_enum {
	TIME_OF_DAY = 0,
	CAUSE_CNT
};

struct cause {
	/* populated by belayd */
	enum cause_enum idx;
	char *name;
	struct cause *next;

	/* private data store for each cause plugin */
	void *data;
};

typedef int (*cause_init)(struct cause * const cse, struct json_object *cse_obj);
typedef int (*cause_main)(struct cause * const cse, int time_since_last_run);
typedef void (*cause_exit)(struct cause * const cse);

extern const char * const cause_names[];
extern const cause_init cause_inits[];
extern const cause_main cause_mains[];
extern const cause_exit cause_exits[];

/*
 * time_of_day.c
 */
int time_of_day_init(struct cause * const cse, struct json_object *cse_obj);
int time_of_day_main(struct cause * const cse, int time_since_last_run);
void time_of_day_exit(struct cause * const cse);
