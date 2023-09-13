// LICENSE TBD
/**
 * belayd causes implementation file
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
#include "cause.h"

const char * const cause_names[] = {
	"time_of_day",
};
static_assert(ARRAY_SIZE(cause_names) == CAUSE_CNT,
	      "cause_names[] must be same length as CAUSE_CNT");

const cause_init cause_inits[] = {
	time_of_day_init,
};
static_assert(ARRAY_SIZE(cause_inits) == CAUSE_CNT,
	      "cause_inits[] must be same length as CAUSE_CNT");

const cause_main cause_mains[] = {
	time_of_day_main,
};
static_assert(ARRAY_SIZE(cause_mains) == CAUSE_CNT,
	      "cause_mains[] must be same length as CAUSE_CNT");

const cause_exit cause_exits[] = {
	time_of_day_exit,
};
static_assert(ARRAY_SIZE(cause_exits) == CAUSE_CNT,
	      "cause_exits[] must be same length as CAUSE_CNT");
