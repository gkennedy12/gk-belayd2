// LICENSE TBD
/**
 * belayd effects header file
 *
 * "Causes" are events that trigger "Effects".  belayd
 * will periodically evaluate the causes, and if they fire
 * then the associated effects will be enforced.
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#ifndef __BELAYD_EFFECT_H
#define __BELAYD_EFFECT_H

#include <json-c/json.h>

#include "defines.h"
#include "cause.h"

enum effect_enum {
	EFFECT_PRINT = 0,
	EFFECT_VALIDATE,

	EFFECT_CNT
};

struct belayd_effect {
	/* populated by belayd */
	enum effect_enum idx;
	char *name;
	const struct belayd_effect_functions *fns;
	struct belayd_effect *next;

	/* private data store for each effect plugin */
	void *data;
};

extern const char * const effect_names[];
extern const struct belayd_effect_functions effect_fns[];


int print_init(struct belayd_effect * const eff, struct json_object *eff_obj,
	       const struct belayd_cause * const cse);
int print_main(struct belayd_effect * const eff);
void print_exit(struct belayd_effect * const eff);

int validate_init(struct belayd_effect * const eff, struct json_object *eff_obj,
		  const struct belayd_cause * const cse);
int validate_main(struct belayd_effect * const eff);
void validate_exit(struct belayd_effect * const eff);

#endif /* __BELAYD_EFFECT_H */
