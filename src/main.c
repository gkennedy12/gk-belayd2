// LICENSE TBD
/**
 * Main loop for belayd
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include <belayd.h>

#include "belayd-internal.h"
#include "defines.h"

void cleanup(struct belayd_ctx *ctx);

static const char * const log_files[] = {
	"syslog",
	"stdout",
	"stderr",
};
static_assert(ARRAY_SIZE(log_files) == LOG_LOC_CNT,
	      "log_files[] must be the same length as LOG_LOC_CNT");

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
	fprintf(fd, "  -L --loglocation=LOCATION Location to write belayd logs\n");
	fprintf(fd, "  -l --loglevel=LEVEL       Log level. See <syslog.h>\n");
	fprintf(fd, "  -m --maxloops=COUNT       Maximum number of loops to run."
						 "Useful for testing\n");
}

static int _belayd_init(struct belayd_ctx * const ctx)
{
	if (!ctx)
		return -EINVAL;

	memset(ctx, 0, sizeof(struct belayd_ctx));

	ctx->interval = default_interval;
	ctx->max_loops = 0;
	ctx->rules = NULL;

	cause_init();

	return 0;
}

struct belayd_ctx *belayd_init(const char * const config_file)
{
	struct belayd_ctx *ctx = NULL;
	int ret;

	ctx = malloc(sizeof(struct belayd_ctx));
	if (!ctx)
		return NULL;

	ret = _belayd_init(ctx);
	if (ret)
		goto err;

	if (config_file)
		strncpy(ctx->config, config_file, FILENAME_MAX - 1);
	else
		strncpy(ctx->config, default_config_file, FILENAME_MAX - 1);

	return ctx;
err:
	if (ctx)
		free(ctx);

	return NULL;
}

void belayd_release(struct belayd_ctx **ctx)
{
	cleanup(*ctx);

	if (*ctx)
		free(*ctx);

	(*ctx) = NULL;
}

int parse_opts(int argc, char *argv[], struct belayd_ctx * const ctx)
{
	struct option long_options[] = {
		{"help",		no_argument, NULL, 'h'},
		{"config",	  required_argument, NULL, 'c'},
		{"interval",	  required_argument, NULL, 'i'},
		{"loglocation",	  required_argument, NULL, 'L'},
		{"loglevel",	  required_argument, NULL, 'l'},
		{"maxloops",	  required_argument, NULL, 'm'},
		{NULL, 0, NULL, 0}
	};
	const char *short_options = "c:hi:L:l:m:";

	int ret = 0, i;
	int tmp_level;
	bool found;

	ret = _belayd_init(ctx);
	if (ret)
		goto err;

	while (1) {
		int c;

		c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'c':
			strncpy(ctx->config, optarg, FILENAME_MAX - 1);
			ctx->config[FILENAME_MAX - 1] = '\0';
			break;
		case 'h':
			usage(stdout);
			exit(0);
		case 'i':
			ctx->interval = atoi(optarg);
			if (ctx->interval < 1) {
				belayd_err("Invalid interval: %s\n", optarg);
				ret = 1;
				goto err;
			}
			break;
		case 'l':
			tmp_level = atoi(optarg);
			if (tmp_level < 1) {
				belayd_err("Invalid log level: %s.  See <syslog.h>\n", optarg);
				ret = 1;
				goto err;
			}

			log_level = tmp_level;
			break;
		case 'L':
			found = false;
			for (i = 0; i < LOG_LOC_CNT; i++) {
				if (strncmp(log_files[i], optarg,
					    strlen(log_files[i])) == 0) {
					found = true;
					log_loc = i;
					break;
				}
			}

			if (!found) {
				belayd_err("Invalid log location: %s\n", optarg);
				ret = 1;
				goto err;
			}
			break;
		case 'm':
			ctx->max_loops = atoi(optarg);
			if (ctx->max_loops < 1) {
				belayd_err("Invalid maxloops: %s\n", optarg);
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

void cleanup(struct belayd_ctx *ctx)
{
	struct belayd_rule *rule, *rule_next;
	struct belayd_effect *eff, *eff_next;
	struct belayd_cause *cse, *cse_next;

	rule = ctx->rules;

	while (rule) {
		belayd_dbg("Cleaning up rule %s\n", rule->name);
		rule_next = rule->next;

		cse = rule->causes;
		while (cse) {
			cse_next = cse->next;
			belayd_dbg("Cleaning up cause %s\n", cse->name);
			(*cse->fns->exit)(cse);
			if (cse->name)
				free(cse->name);

			free(cse);
			cse = cse_next;
		}

		eff = rule->effects;
		while (eff) {
			eff_next = eff->next;
			belayd_dbg("Cleaning up effect %s\n", eff->name);
			(*eff->fns->exit)(eff);
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

	/*
	 * Now that the rules have been cleaned up, we can clean up the
	 * registered causes.
	 */
	cause_cleanup();
}

int belayd_loop(struct belayd_ctx * const ctx)
{
	struct belayd_effect *eff;
	struct belayd_rule *rule;
	struct belayd_cause *cse;
	unsigned int loop_cnt;
	int ret = 0;

	ret = parse_config(ctx);
	if (ret)
		goto out;

	loop_cnt = 0;

	while (1) {
		rule = ctx->rules;

		while (rule) {
			belayd_dbg("Running rule %s\n", rule->name);
			cse = rule->causes;

			while (cse) {
				ret = (*cse->fns->main)(cse, ctx->interval);
				if (ret < 0) {
					belayd_dbg("%s raised error %d\n", cse->name, ret);
					goto out;
				} else if (ret == 0) {
					/*
					 * this cause did not trip.  skip all the remaining causes
					 * in this rule because the effect will not be invoked.
					 */
					belayd_dbg("%s did not trip\n", cse->name);
					break;
				} else if (ret > 0) {
					/*
					 * This cause tripped.  We don't need to do anything.
					 * If all of the causes in this rule are triggered,
					 * then the "ret > 0" will flow down to the logic
					 * below and the effects will be run.
					 */
					belayd_dbg("%s tripped\n", cse->name);
				}

				cse = cse->next;
			}

			if (ret > 0) {
				/*
				 * The cause(s) for this rule were triggered, invoke the
				 * effect(s)
				 */
				eff = rule->effects;

				while (eff) {
					belayd_dbg("Running effect %s\n", eff->name);
					ret = (*eff->fns->main)(eff);
					if (ret)
						goto out;

					eff = eff->next;
				}
			}


			rule = rule->next;
		}

		loop_cnt++;
		if (ctx->max_loops > 0 && loop_cnt > ctx->max_loops) {
			ret = -ETIME;
			break;
		}

		sleep(ctx->interval);
	}

out:
	return ret;
}

int main(int argc, char *argv[])
{
	struct belayd_ctx ctx;
	int ret;

	ret = parse_opts(argc, argv, &ctx);
	if (ret)
		goto out;

	ret = belayd_loop(&ctx);
	if (ret)
		goto out;

out:
	cleanup(&ctx);

	return -ret;
}
