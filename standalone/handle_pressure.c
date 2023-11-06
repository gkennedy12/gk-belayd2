// LICENSE TBD
/**
 * Test to register a cause at runtime and utilize it
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <json-c/json.h>

#include <belayd.h>

#include "pressure_cause.h"

#define AUTOMAKE_PASSED 0
#define AUTOMAKE_HARD_ERROR 99
#define EXPECTED_RET -42

int main(int argc, char *argv[])
{
	char config_path[FILENAME_MAX];
	struct belayd_ctx *ctx;
	int ret;
	int i;
	int index;
	int c;
	int loops = 10;
	int interval = 1;

	opterr = 0;
	config_path[0] = 0;

	while ((c = getopt (argc, argv, "c:m:i:")) != -1)
	switch (c) {
		case 'c':
			snprintf(config_path, FILENAME_MAX - 1, "%s", optarg);
			config_path[FILENAME_MAX - 1] = '\0';
			break;
		case 'm':
			loops = strtol(optarg, 0, 0);
			break;
		case 'i':
			interval = strtol(optarg, 0, 0);
			break;
		case '?':
			if (optopt == 'c')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
			return AUTOMAKE_HARD_ERROR;
		default:
			return AUTOMAKE_HARD_ERROR;
	}

	if (!config_path[0]) {
		fprintf(stderr, "Config file path missing.\n");
		return AUTOMAKE_HARD_ERROR;
	}
	ctx = belayd_init(config_path);
	if (!ctx)
		return AUTOMAKE_HARD_ERROR;

	ret = belayd_set_attr(ctx, BELAYD_ATTR_MAX_LOOPS, loops);
	if (ret)
		goto err;
	ret = belayd_set_attr(ctx, BELAYD_ATTR_INTERVAL, interval);
	if (ret)
		goto err;

	ret = belayd_register_cause(ctx, pressure_name, &pressure_fns);
	if (ret)
		goto err;

	ret = belayd_loop(ctx);
	if (ret != EXPECTED_RET)
		goto err;

	belayd_release(&ctx);
	return AUTOMAKE_PASSED;

err:
	belayd_release(&ctx);
	return AUTOMAKE_HARD_ERROR;
}
