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
	"days_of_the_week",
};
static_assert(ARRAY_SIZE(cause_names) == CAUSE_CNT,
	      "cause_names[] must be same length as CAUSE_CNT");

const struct belayd_cause_functions cause_fns[] = {
	{time_of_day_init, time_of_day_main, time_of_day_exit, time_of_day_print},
	{days_of_the_week_init, days_of_the_week_main, days_of_the_week_exit,
		days_of_the_week_print},
};
static_assert(ARRAY_SIZE(cause_fns) == CAUSE_CNT,
	      "cause_fns[] must be same length as CAUSE_CNT");
