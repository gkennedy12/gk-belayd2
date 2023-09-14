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
};
static_assert(ARRAY_SIZE(effect_names) == EFFECT_CNT,
	      "effect_names[] must be same length as EFFECT_CNT");

const effect_init effect_inits[] = {
	print_init,
};
static_assert(ARRAY_SIZE(effect_inits) == EFFECT_CNT,
	      "effect_inits[] must be same length as EFFECT_CNT");

const effect_main effect_mains[] = {
	print_main,
};
static_assert(ARRAY_SIZE(effect_mains) == EFFECT_CNT,
	      "effect_mains[] must be same length as EFFECT_CNT");

const effect_exit effect_exits[] = {
	print_exit,
};
static_assert(ARRAY_SIZE(effect_exits) == EFFECT_CNT,
	      "effect_exits[] must be same length as EFFECT_CNT");
