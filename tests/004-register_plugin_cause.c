// LICENSE TBD
/**
 * Test to register a cause at runtime and utilize it
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

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

	snprintf(config_path, FILENAME_MAX - 1, "%s/004-register_plugin_cause.json", argv[1]);
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
