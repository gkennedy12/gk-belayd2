// LICENSE TBD
/**
 * Main loop for belayd
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

	memset(opts, 0, sizeof(struct belayd_opts));
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

void cleanup(struct belayd_opts *opts)
{
	struct rule *rule, *rule_next;
	struct effect *eff, *eff_next;
	struct cause *cse, *cse_next;

	rule = opts->rules;

	while (rule) {
		rule_next = rule->next;

		cse = rule->causes;
		while (cse) {
			cse_next = cse->next;
			cause_exits[cse->idx](cse);
			if (cse->name)
				free(cse->name);

			free(cse);
			cse = cse_next;
		}

		eff = rule->effects;
		while (eff) {
			eff_next = eff->next;
			effect_exits[eff->idx](eff);
			if (eff->name)
				free(eff->name);

			free(eff);
			eff = eff_next;
		}

		if (rule->name)
			free(rule->name);

		free(rule);
		rule = rule_next;
	}
}

int main(int argc, char *argv[])
{
	struct belayd_opts opts;
	struct effect *eff;
	struct cause *cse;
	struct rule *rule;
	int ret;

	ret = parse_opts(argc, argv, &opts);
	if (ret)
		goto out;

	ret = parse_config(&opts);
	if (ret)
		goto out;

	while (1) {
		rule = opts.rules;

		while (rule) {
			cse = rule->causes;

			while (cse) {
				ret = cause_mains[cse->idx](cse, opts.interval);
				if (ret < 0)
					goto out;
				else if (ret == 0)
					/*
					 * this cause did not trip.  skip all the remaining causes
					 * in this rule
					 */
					break;
				else if (ret > 0)
					/*
					 * This cause tripped.  We don't need to do anything.
					 * If all of the causes in this rule are triggered,
					 * then the "ret > 0" will flow down to the logic
					 * below and the effects will be run.
					 */
					;
				cse = cse->next;
			}

			if (ret > 0) {
				/*
				 * The cause(s) for this rule were triggered, invoke the
				 * effect(s)
				 */
				eff = rule->effects;

				while (eff) {
					ret = effect_mains[eff->idx](eff);
					if (ret)
						goto out;

					eff = eff->next;
				}
			}


			rule = rule->next;
		}

		sleep(opts.interval);
	}

out:
	cleanup(&opts);

	return ret;
}
