// LICENSE TBD
/**
 * pressure cause
 *
 * This file processes pressure causes
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#define _XOPEN_SOURCE

#include <stdbool.h>
#include <assert.h>
//#define  _GNU_SOURCE
//#define  _POSIX_C_SOURCE 200809L
#define __USE_XOPEN2K8
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "belayd-internal.h"
#include "defines.h"

#include "utils/pressure_utils.h"

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

int pressure_init(struct cause * const cse, struct json_object *cse_obj)
{
	struct json_object *args_obj;
	struct pressure_opts *opts;
	const char *threshold_str, *op_str;
	const char *duration_str;
	const char *verbose_str;
	const char *test_str;
	const char *pressure_slice;
	json_bool exists;
	bool found_op;
	int ret = 0;
	int i;

	// fprintf(stderr, "XXX pressure_init:\n");

	opts = malloc(sizeof(struct pressure_opts));
	if (!opts) {
		ret = -ENOMEM;
		goto error;
	}

	memset(opts, 0, sizeof(struct pressure_opts));

	exists = json_object_object_get_ex(cse_obj, "args", &args_obj);
	if (!exists || !args_obj) {
		ret = -EINVAL;
		goto error;
	}

        ret = parse_string(args_obj, "verbose", &verbose_str);
        if (ret == 0) {
                verbose = atoi(verbose_str);
        }
        if (verbose) fprintf(stderr, "\nXXX %s:\n", __func__);

        ret = parse_string(args_obj, "test", &test_str);
        if (ret == 0) {
                test = atoi(test_str);
        }
        if (verbose) fprintf(stderr, "\nXXX %s: TEST\n", __func__);

	ret = parse_string(args_obj, "threshold", &threshold_str);
	if (ret)
		goto error;

	opts->threshold = atoi(threshold_str);
	if (verbose) fprintf(stderr, "pressure_init: opts->threshold: %d\n", opts->threshold);

	ret = parse_string(args_obj, "duration", &duration_str);
	if (ret)
		goto error;

	opts->duration = atoi(duration_str);
	if (!opts->duration)
		opts->duration = 99999;		// default
	if (verbose) fprintf(stderr, "pressure_init: opts->duration: %d\n", opts->duration);

	ret = parse_string(args_obj, "pressure_slice", &pressure_slice);
	if (ret)
		goto error;

	opts->pressure_slice = malloc(sizeof(char) * strlen(pressure_slice));
	if (!opts->pressure_slice) {
		ret = -ENOMEM;
		goto error;
	}

	strcpy(opts->pressure_slice, pressure_slice);

	if (verbose) fprintf(stderr, "opressure_init: opts->pressure_slice: %s\n", opts->pressure_slice);

	ret = parse_string(args_obj, "operator", &op_str);
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

	/* we have successfully setup the pressure cause */
	cse->data = (void *)opts;

	return ret;

error:
	fprintf(stderr, "opressure_init: error: ret=%d\n", ret);
	if (opts)
		free(opts);

	return ret;
}

int pressure_main(struct cause * const cse, int time_since_last_run)
{
	struct pressure_opts *opts = (struct pressure_opts *)cse->data;
	int ret = 0;
	struct pressure_values the_pressure_values, *pvp;
	char press_str[256];
	time_t cur_time;

	if (verbose) {
		fprintf(stderr, "\nXXX %s: ", __func__);
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
						"XXX pvp->avg10=%2.2f (%d), pvp->avg60=%2.2f\n",
						pvp->avg10, (int)pvp->avg10, pvp->avg60);
			} else {
				fprintf(stderr,
					"parse_pressure: XXX Can't get pressure for %s\n", press_str);
			}
			if ((int)pvp->avg60 > opts->threshold) {
				time(&cur_time);

				fprintf(stderr, "cur_time=%ld\n", cur_time);
				if (!opts->first_time) {
					opts->first_time = cur_time;
				} else {
					if (cur_time - opts->first_time >= opts->duration) {
						fprintf(stderr, "opts->first_time=%ld\n", opts->first_time);

						fprintf(stderr, "<<<TRIGGER, seconds: %ld>>>\n", 
							cur_time - opts->first_time);
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
						"XXX pvp->avg10=%2.2f (%d), pvp->avg60=%2.2f\n",
						pvp->avg10, (int)pvp->avg10, pvp->avg60);
			} else {
				fprintf(stderr,
					"parse_pressure: XXX Can't get pressure for %s\n", press_str);
			}
			if ((int)pvp->avg60 < opts->threshold) {
				time(&cur_time);

				fprintf(stderr, "cur_time=%ld\n", cur_time);
				if (!opts->first_time) {
					opts->first_time = cur_time;
				} else {
					if (cur_time - opts->first_time >= opts->duration) {
						fprintf(stderr, "opts->first_time=%ld\n", opts->first_time);

						fprintf(stderr, "<<<TRIGGER, seconds: %ld>>>\n", 
							cur_time - opts->first_time);
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

void pressure_exit(struct cause * const cse)
{
	struct pressure_opts *opts = (struct pressure_opts *)cse->data;

	if (verbose) fprintf(stderr, "XXX %s:\n", __func__);

	free(opts);
}

void pressure_print(const struct cause * const cse, FILE *file)
{
	struct pressure_opts *opts = (struct pressure_opts *)cse->data;

	if (verbose) fprintf(stderr, "XXX %s:\n", __func__);

	switch (opts->op) {
		case OP_GREATER_THAN:
			fprintf(file, "\tpressure cause: pressure is greater than %d\n", opts->threshold);
			break;
		default:
			fprintf(file, "Invalid pressure op\n");
			break;
	}
}
