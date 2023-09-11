// LICENSE TBD
/**
 * Internal include header file for belayd
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */
#include <stdlib.h>
#include <getopt.h>
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

	while (1) {
		int c;

		c = getopt_long(argc, argv, short_options, long_options, NULL);

		switch (c) {
		case 'h':
			usage(stdout);
			exit(0);

		default:
			usage(stderr);
			exit(0);
		}
	}

	return ret;
}

int main(int argc, char *argv[])
{
	struct belayd_opts opts;
	int ret;

	ret = parse_opts(argc, argv, &opts);
	if (ret)
		goto out;

out:
	return ret;
}
