// LICENSE TBD
/**
 * belayd effects implementation file
 *
 * "Causes" are events that trigger "Effects".  belayd
 * will periodically evaluate the causes, and if they fire
 * then the associated effects will be enforced.
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <assert.h>

#include "defines.h"
#include "effect.h"

const char * const effect_names[] = {
	"print",
	"validate",
};
static_assert(ARRAY_SIZE(effect_names) == EFFECT_CNT,
	      "effect_names[] must be same length as EFFECT_CNT");

const struct effect_functions effect_fns[] = {
	{print_init, print_main, print_exit},
	{validate_init, validate_main, validate_exit},
};
static_assert(ARRAY_SIZE(effect_fns) == EFFECT_CNT,
	      "effect_fns[] must be same length as EFFECT_CNT");
