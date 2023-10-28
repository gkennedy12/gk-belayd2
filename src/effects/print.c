// LICENSE TBD
/**
 * print effect
 *
 * This file runs the print effect
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

enum file_enum {
	FILE_STDOUT = 0,
	FILE_STDERR,

	FILE_CNT
};

struct print_opts {
	FILE *file;
	const struct belayd_cause *cse;
};

int print_init(struct belayd_effect * const eff, struct json_object *eff_obj,
	       const struct belayd_cause * const cse)
{
	struct json_object *args_obj;
	struct print_opts *opts;
	const char *file_str;
	json_bool exists;
	int ret = 0;

	opts = malloc(sizeof(struct print_opts));
	if (!opts) {
		ret = -ENOMEM;
		goto error;
	}

	exists = json_object_object_get_ex(eff_obj, "args", &args_obj);
	if (!exists || !args_obj) {
		ret = -EINVAL;
		goto error;
	}

	ret = belayd_parse_string(args_obj, "file", &file_str);
	if (ret)
		goto error;

	if (strncmp(file_str, "stderr", strlen("stderr")) == 0) {
		opts->file = stderr;
	} else if (strncmp(file_str, "stdout", strlen("stdout")) == 0) {
		opts->file = stdout;
	} else {
		ret = -EINVAL;
		goto error;
	}

	opts->cse = cse;

	/* we have successfully setup the print effect */
	eff->data = (void *)opts;

	return ret;

error:
	if (opts)
		free(opts);

	return ret;
}

int print_main(struct belayd_effect * const eff)
{
	struct print_opts *opts = (struct print_opts *)eff->data;
	const struct belayd_cause *cse;

	fprintf(opts->file, "Print effect triggered by:\n");

	cse = opts->cse;
	while (cse) {
		if (cse->fns->print)
			(*cse->fns->print)(cse, opts->file);
		else
			fprintf(opts->file, "\t%s\n", cse->name);

		cse = cse->next;
	}

	return 0;
}

void print_exit(struct belayd_effect * const eff)
{
	struct print_opts *opts = (struct print_opts *)eff->data;

	free(opts);
}
