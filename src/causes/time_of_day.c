// LICENSE TBD
/**
 * time of day cause
 *
 * This file processes time of day causes
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#define _XOPEN_SOURCE

#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "belayd-internal.h"
#include "defines.h"

enum op_enum {
	OP_GREATER_THAN = 0,
	OP_CNT
};

static const char * const op_names[] = {
	"greaterthan",
};
static_assert(ARRAY_SIZE(op_names) == OP_CNT,
	      "op_names[] must be same length as OP_CNT");

struct time_of_day_opts {
	enum op_enum op;
	struct tm time;
};

int time_of_day_init(struct cause * const cse, struct json_object *cse_obj)
{
	struct json_object *args_obj, *time_obj, *op_obj;
	struct time_of_day_opts *opts;
	const char *time_str, *op_str;
	json_bool exists;
	struct tm time;
	bool found_op;
	int ret = 0;
	char *tret;
	int i;

	opts = malloc(sizeof(struct time_of_day_opts));
	if (!opts) {
		ret = -ENOMEM;
		goto error;
	}

	exists = json_object_object_get_ex(cse_obj, "args", &args_obj);
	if (!exists || !args_obj) {
		ret = -EINVAL;
		goto error;
	}

	exists = json_object_object_get_ex(args_obj, "time", &time_obj);
	if (!exists || !time_obj) {
		ret = -EINVAL;
		goto error;
	}

	time_str = json_object_get_string(time_obj);
	if (!time_str) {
		ret = -EINVAL;
		goto error;
	}

	tret = strptime(time_str, "%H:%M:%S", &time);
	if (!tret) {
		/*
		 * We were unable to process all of the characters in the
		 * string.  Fail and notify the user
		 */
		ret = -EINVAL;
		goto error;
	}

	memcpy(&opts->time, &time, sizeof(struct tm));

	exists = json_object_object_get_ex(args_obj, "operator", &op_obj);
	if (!exists || !op_obj) {
		ret = -EINVAL;
		goto error;
	}

	op_str = json_object_get_string(op_obj);
	if (!op_str) {
		ret = -EINVAL;
		goto error;
	}

	found_op = false;
	for (i = 0; i < OP_CNT; i++) {
		if (strncmp(op_str, op_names[i], strlen(op_names[i])) == 0) {
			found_op = true;
			opts->op = i;
			break;
		}
	}

	if (!found_op) {
		ret = -EINVAL;
		goto error;
	}

	/* we have successfully setup the time_of_day cause */
	cse->data = (void *)opts;

	return ret;

error:
	if (opts)
		free(opts);

	return ret;
}

int time_of_day_main(struct cause * const cse, int time_since_last_run)
{
	struct time_of_day_opts *opts = (struct time_of_day_opts *)cse->data;
	struct tm *cur_tm;
	time_t cur_time;
	int ret = 0;

	time(&cur_time);
	cur_tm = localtime(&cur_time);

	switch (opts->op) {
		case OP_GREATER_THAN:
			if (cur_tm->tm_hour > opts->time.tm_hour)
				return 1;
			if (cur_tm->tm_hour == opts->time.tm_hour &&
			    cur_tm->tm_min > opts->time.tm_min)
				return 1;
			if (cur_tm->tm_hour == opts->time.tm_hour &&
			    cur_tm->tm_min == opts->time.tm_min &&
			    cur_tm->tm_sec > opts->time.tm_sec)
				return 1;
			break;
		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}

void time_of_day_exit(struct cause * const cse)
{
	struct time_of_day_opts *opts = (struct time_of_day_opts *)cse->data;

	free(opts);
}

void time_of_day_print(const struct cause * const cse)
{
	printf("time of day\n");
}
