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

#define AUTOMAKE_PASSED 0
#define AUTOMAKE_HARD_ERROR 99
#define EXPECTED_RET -123

struct wait_cause_opts {
	int wait; /* number of loops to wait before returning success */
	int wait_cnt; /* number of loops we have waited */
};

int wait_cause_init(struct belayd_cause * const cse, struct json_object *cse_obj)
{
	struct wait_cause_opts *opts;
	struct json_object *args_obj;
	json_bool exists;
	int ret = 0;

	opts = malloc(sizeof(struct wait_cause_opts));
	if (!opts) {
		ret = -ENOMEM;
		goto error;
	}

	exists = json_object_object_get_ex(cse_obj, "args", &args_obj);
	if (!exists || !args_obj) {
		ret = -EINVAL;
		goto error;
	}

	ret = belayd_parse_int(args_obj, "wait", &opts->wait);
	if (ret)
		goto error;

	opts->wait_cnt = 0;

	ret = belayd_cause_set_data(cse, (void *)opts);
	if (ret)
		goto error;

	return ret;

error:
	if (opts)
		free(opts);

	return ret;
}

int wait_cause_main(struct belayd_cause * const cse, int time_since_last_run)
{
	struct wait_cause_opts *opts;

	opts = (struct wait_cause_opts *)belayd_cause_get_data(cse);

	if (opts->wait_cnt >= opts->wait)
		return 1;

	opts->wait_cnt++;
	return 0;
}

void wait_cause_exit(struct belayd_cause * const cse)
{
	struct wait_cause_opts *opts;

	opts = (struct wait_cause_opts *)belayd_cause_get_data(cse);

	free(opts);
}

const struct belayd_cause_functions wait_cause_fns = {
	wait_cause_init,
	wait_cause_main,
	wait_cause_exit,
	NULL
};

int main(int argc, char *argv[])
{
	char config_path[FILENAME_MAX];
	struct belayd_ctx *ctx;
	int ret;

	snprintf(config_path, FILENAME_MAX - 1, "%s/002-register_cause.json", argv[1]);
	config_path[FILENAME_MAX - 1] = '\0';

	ctx = belayd_init(config_path);
	if (!ctx)
		return AUTOMAKE_HARD_ERROR;

	ret = belayd_set_attr(ctx, BELAYD_ATTR_MAX_LOOPS, 10);
	if (ret)
		goto err;
	ret = belayd_set_attr(ctx, BELAYD_ATTR_INTERVAL, 1);
	if (ret)
		goto err;

	ret = belayd_register_cause(ctx, "wait_cause", &wait_cause_fns);
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
