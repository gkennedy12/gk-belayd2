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

struct effect {
	/* populated by belayd */
	enum effect_enum idx;
	char *name;
	const struct effect_functions *fns;
	struct effect *next;

	/* private data store for each effect plugin */
	void *data;
};

typedef int (*effect_init)(struct effect * const eff, struct json_object *eff_obj,
			   const struct cause * const cse);
typedef int (*effect_main)(struct effect * const eff);
typedef void (*effect_exit)(struct effect * const eff);

struct effect_functions {
	effect_init init;
	effect_main main;
	effect_exit exit;
};

extern const char * const effect_names[];
extern const struct effect_functions effect_fns[];


int print_init(struct effect * const eff, struct json_object *eff_obj,
	       const struct cause * const cse);
int print_main(struct effect * const eff);
void print_exit(struct effect * const eff);

int validate_init(struct effect * const eff, struct json_object *eff_obj,
		  const struct cause * const cse);
int validate_main(struct effect * const eff);
void validate_exit(struct effect * const eff);

#endif /* __BELAYD_EFFECT_H */
