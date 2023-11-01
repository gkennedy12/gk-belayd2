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

#include <belayd.h>

const char * const tod_name = "tod";

enum op_enum {
	OP_GREATER_THAN = 0,
	OP_CNT
};

static const char * const op_names[] = {
	"greaterthan",
};

struct time_of_day_opts {
	char *time_str;
	enum op_enum op;
	struct tm time;
};

int time_of_day_init(struct belayd_cause * const cse, struct json_object *cse_obj)
{
	struct json_object *args_obj;
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

	memset(opts, 0, sizeof(struct time_of_day_opts));

	exists = json_object_object_get_ex(cse_obj, "args", &args_obj);
	if (!exists || !args_obj) {
		ret = -EINVAL;
		goto error;
	}

	ret = belayd_parse_string(args_obj, "time", &time_str);
	if (ret)
		goto error;

	opts->time_str = malloc(sizeof(char) * strlen(time_str));
	if (!opts->time_str) {
		ret = -ENOMEM;
		goto error;
	}

	strcpy(opts->time_str, time_str);

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

	ret = belayd_parse_string(args_obj, "operator", &op_str);
	if (ret)
		goto error;

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

	ret = belayd_cause_set_data(cse, (void *)opts);
	if (ret)
		goto error;

	return ret;

error:
	if (opts && opts->time_str)
		free(opts->time_str);

	if (opts)
		free(opts);

	return ret;
}

int time_of_day_main(struct belayd_cause * const cse, int time_since_last_run)
{
	struct time_of_day_opts *opts;
	struct tm *cur_tm;
	time_t cur_time;
	int ret = 0;

	opts = (struct time_of_day_opts *)belayd_cause_get_data(cse);

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

void time_of_day_exit(struct belayd_cause * const cse)
{
	struct time_of_day_opts *opts;

	opts = (struct time_of_day_opts *)belayd_cause_get_data(cse);

	if (opts->time_str)
		free(opts->time_str);

	free(opts);
}

void time_of_day_print(const struct belayd_cause * const cse, FILE *file)
{
	struct time_of_day_opts *opts;

	opts = (struct time_of_day_opts *)belayd_cause_get_data(cse);

	switch (opts->op) {
		case OP_GREATER_THAN:
			fprintf(file, "\tToD cause: current time is greater than %s\n", opts->time_str);
			break;
		default:
			fprintf(file, "Invalid ToD op\n");
			break;
	}
}

const struct belayd_cause_functions tod_fns = {
	time_of_day_init,
	time_of_day_main,
	time_of_day_exit,
	time_of_day_print
};
