// LICENSE TBD
/**
 * Internal include header file for belayd
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "belayd-internal.h"

static const char * const default_config_file = "/etc/belayd.json";
static const int default_interval = 5; /* seconds */

static void usage(FILE *fd)
{
	fprintf(fd, "\nbelayd: a daemon for managing and prioritizing resources\n\n");
	fprintf(fd, "Usage: belayd [options]\n\n");
	fprintf(fd, "Optional arguments:\n");
	fprintf(fd, "  -c --config=CONFIG        Configuration file (default: %s)\n",
		default_config_file);
	fprintf(fd, "  -h --help                 Show this help message\n");
	fprintf(fd, "  -i --interval=INTERVAL    Polling interval in seconds (default: %d)\n",
		default_interval);
}

int parse_opts(int argc, char *argv[], struct belayd_opts * const opts)
{
	struct option long_options[] = {
		{"help",		no_argument, NULL, 'h'},
		{"config",	  required_argument, NULL, 'c'},
		{"interval",	  required_argument, NULL, 'i'},
		{NULL, 0, NULL, 0}
	};
	const char *short_options = "c:hi:";

	int ret = 0;

	memset(opts->config, 0, FILENAME_MAX);
	strncpy(opts->config, default_config_file, FILENAME_MAX - 1);
	opts->interval = default_interval;

	while (1) {
		int c;

		c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'c':
			strncpy(opts->config, optarg, FILENAME_MAX - 1);
			opts->config[FILENAME_MAX - 1] = '\0';
			break;
		case 'h':
			usage(stdout);
			exit(0);
		case 'i':
			opts->interval = atoi(optarg);
			if (opts->interval < 1) {
				ret = 1;
				goto err;
			}
			break;

		default:
			ret = 1;
			goto err;
		}
	}

err:
	if (ret)
		usage(stderr);

	return ret;
}

int main(int argc, char *argv[])
{
	struct belayd_opts opts;
	int ret;

	ret = parse_opts(argc, argv, &opts);
	if (ret)
		goto out;

	while(1) {
		printf("cfg = %s\n", opts.config);
		sleep(opts.interval);
	}

out:
	return ret;
}
