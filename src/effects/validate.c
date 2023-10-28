// LICENSE TBD
/**
 * Effect to validate that a cause successfully fired
 *
 * Returns a positive integer to the main belayd loop when
 * the validator main() is invoked.  This will cause belayd
 * to exit the main loop and can be useful for testing that
 * causes have executed properly
 *
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "belayd-internal.h"
#include "defines.h"

const int default_return_value = 7;

struct validate_opts {
	int	ret;
};

int validate_init(struct belayd_effect * const eff, struct json_object *eff_obj,
	       const struct belayd_cause * const cse)
{
	struct json_object *args_obj;
	struct validate_opts *opts;
	json_bool exists;
	int ret = 0;

	opts = malloc(sizeof(struct validate_opts));
	if (!opts) {
		ret = -ENOMEM;
		goto error;
	}
	opts->ret = default_return_value;

	exists = json_object_object_get_ex(eff_obj, "args", &args_obj);
	if (!exists || !args_obj) {
		ret = -EINVAL;
		goto error;
	}

	ret = parse_int(args_obj, "return_value", &opts->ret);
	if (ret)
		goto error;

	/* we have successfully setup the validate effect */
	eff->data = (void *)opts;

	return ret;

error:
	if (opts)
		free(opts);

	return ret;
}

int validate_main(struct belayd_effect * const eff)
{
	struct validate_opts *opts = (struct validate_opts *)eff->data;

	/* main() will negate our return value */
	return -opts->ret;
}

void validate_exit(struct belayd_effect * const eff)
{
	struct validate_opts *opts = (struct validate_opts *)eff->data;

	free(opts);
}
