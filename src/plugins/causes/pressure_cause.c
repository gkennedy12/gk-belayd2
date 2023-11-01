// LICENSE TBD
/**
 * Test to register a cause at runtime and utilize it
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <json-c/json.h>
#include <errno.h>

#include <belayd.h>

const char * const pressure_name = "pressure";

#define _XOPEN_SOURCE

#include <stdbool.h>
#include <assert.h>
// #define __USE_XOPEN2K8
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// #include "belayd-internal.h"
#include "../../../src/defines.h"

#include "../utils/pressure_utils.h"

static int verbose = 0;
static int test = 0;

enum op_enum {
	OP_GREATER_THAN = 0,
	OP_LESS_THAN,
	OP_CNT
};

static const char * const op_names[] = {
	"greaterthan",
	"lessthan",
};
static_assert(ARRAY_SIZE(op_names) == OP_CNT,
	      "op_names[] must be same length as OP_CNT");

struct pressure_opts {
	char *what;
	char *pressure_slice;
	int threshold;
	int duration;
	enum op_enum op;
	time_t first_time;
};

int pressure_init(struct belayd_cause * const cse, struct json_object *cse_obj)
{
	struct pressure_opts *opts;
	struct json_object *args_obj;
	const char *pressure_slice_str, *op_str;
	json_bool exists;
	bool found_op;
	int i;
	int ret = 0;

fprintf(stderr, "XXX %s:\n", __func__);

	opts = malloc(sizeof(struct pressure_opts));
	if (!opts) {
		ret = -ENOMEM;
		goto error;
	}

	exists = json_object_object_get_ex(cse_obj, "args", &args_obj);
	if (!exists || !args_obj) {
		ret = -EINVAL;
		goto error;
	}

	ret = belayd_parse_int(args_obj, "verbose", &verbose);
fprintf(stderr, "XXX %s: ret=%d, verbose=%d\n", __func__, ret, verbose);

	ret = belayd_parse_int(args_obj, "test", &test);
	if (verbose) fprintf(stderr, "XXX %s: ret=%d, test=%d\n", __func__, ret, test);

	ret = belayd_parse_int(args_obj, "threshold", &opts->threshold);
	if (ret)
		goto error;
	if (verbose) fprintf(stderr, "XXX %s: ret=%d, opts->threshold=%d\n", __func__, ret, opts->threshold);

	ret = belayd_parse_int(args_obj, "duration", &opts->duration);
	if (ret)
		opts->duration = 99999;         // default
	if (verbose) fprintf(stderr, "XXX %s: ret=%d, opts->duration=%d\n", __func__, ret, opts->duration);

	ret = belayd_parse_string(args_obj, "pressure_slice", &pressure_slice_str);
	if (ret)
		goto error;

	opts->pressure_slice = malloc(sizeof(char) * strlen(pressure_slice_str));
	if (!opts->pressure_slice) {
		ret = -ENOMEM;
		goto error;
	}
	strcpy(opts->pressure_slice, pressure_slice_str);
	if (verbose) fprintf(stderr, "pressure_init: opts->pressure_slice: %s\n", opts->pressure_slice);

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
fprintf(stderr, "XXX %s: belayd_cause_set_data() ret=%d\n", __func__, ret);
	if (ret)
		goto error;

	return ret;

error:
	if (opts)
		free(opts);

	return ret;
}

int pressure_main(struct belayd_cause * const cse, int time_since_last_run)
{
	struct pressure_opts *opts = (struct pressure_opts *)belayd_cause_get_data(cse);
	int ret = 0;
	struct pressure_values the_pressure_values, *pvp;
	char press_str[256];
	time_t cur_time;


	if (verbose) {
		fprintf(stderr, "\n%s: ", __func__);
		system("date");
	}

	if (!opts->threshold) 		// nothing to do?
		return 0;

	if (!opts->pressure_slice)
		return 0;

	sprintf(press_str, "/sys/fs/cgroup/%s.slice/memory.pressure", opts->pressure_slice);

	switch (opts->op) {
		case OP_GREATER_THAN:
			pvp = &the_pressure_values;
			pvp = parse_pressure("some", press_str, pvp);

			if (pvp) {
				if (test) {
					pvp->avg60 = opts->threshold + 1;
				}
				if (verbose)
					fprintf(stderr,
						"pvp->avg10=%2.2f (%d), pvp->avg60=%2.2f\n",
						pvp->avg10, (int)pvp->avg10, pvp->avg60);
			} else {
				fprintf(stderr,
					"parse_pressure: Can't get pressure for %s\n", press_str);
			}
			if ((int)pvp->avg60 > opts->threshold) {
				time(&cur_time);

				if (verbose) fprintf(stderr, "cur_time=%ld\n", cur_time);
				if (!opts->first_time) {
					opts->first_time = cur_time;
				} else {
					if (cur_time - opts->first_time >= opts->duration) {
						if (verbose) {
							fprintf(stderr, "opts->first_time=%ld\n",
							    opts->first_time);

							fprintf(stderr, "<<<TRIGGER, seconds: %ld>>>\n", 
							    cur_time - opts->first_time);
						}
						opts->first_time = 0;
						ret = 1;
					}
				}
			} else {
				opts->first_time = 0;
			}
			break;
		case OP_LESS_THAN:
			pvp = &the_pressure_values;
			pvp = parse_pressure("some", press_str, pvp);

			if (pvp) {
				if (verbose)
					fprintf(stderr,
						"pvp->avg10=%2.2f (%d), pvp->avg60=%2.2f\n",
						pvp->avg10, (int)pvp->avg10, pvp->avg60);
			} else {
				fprintf(stderr,
					"parse_pressure: Can't get pressure for %s\n", press_str);
			}
			if ((int)pvp->avg60 < opts->threshold) {
				time(&cur_time);

				if (verbose) fprintf(stderr, "cur_time=%ld\n", cur_time);
				if (!opts->first_time) {
					opts->first_time = cur_time;
				} else {
					if (cur_time - opts->first_time >= opts->duration) {
						if (verbose) {
							fprintf(stderr, "opts->first_time=%ld\n",
							    opts->first_time);
							fprintf(stderr, "<<<TRIGGER, seconds: %ld>>>\n", 
							    cur_time - opts->first_time);
						}
						opts->first_time = 0;
						ret = 1;
					}
				}
			} else {
				opts->first_time = 0;
			}
			break;
		default:
			ret = -EINVAL;
			break;
	}
	return ret;
}

void pressure_exit(struct belayd_cause * const cse)
{
	struct pressure_opts *opts = (struct pressure_opts *)belayd_cause_get_data(cse);

	if (verbose) fprintf(stderr, "%s:\n", __func__);

	free(opts);
}

void pressure_print(const struct belayd_cause * const cse, FILE *file)
{
	struct pressure_opts *opts = (struct pressure_opts *)belayd_cause_get_data(cse);

	if (verbose) fprintf(stderr, "%s:\n", __func__);

	switch (opts->op) {
		case OP_GREATER_THAN:
			fprintf(file, "\tpressure cause: pressure is greater than %d\n", opts->threshold);
			break;
		case OP_LESS_THAN:
			fprintf(file, "\tpressure cause: pressure is less than %d\n", opts->threshold);
			break;
		default:
			fprintf(file, "Invalid pressure op\n");
			break;
	}
}

const struct belayd_cause_functions pressure_fns = {
	pressure_init,
	pressure_main,
	pressure_exit,
	NULL
};
