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
#include <string.h>
#include <errno.h>

#include <belayd.h>

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

/*
 * Re-use the belayd_cause structure to store a linked list of causes that have been
 * added at runtime by the user.  When processing causes, the ->next field is used to
 * chain multiple causes together for one rule (e.g. notify me when it's after 5pm and
 * it's Friday).  In the registered_causes case, the ->next field simply points to the
 * next registered cause that's been added by the user
 */
struct belayd_cause *registered_causes;

int belayd_register_cause(struct belayd_ctx * const ctx, const char * const name,
			  const struct belayd_cause_functions * const fns)
{
	struct belayd_cause *cse = NULL;
	int ret = 0;

	if (!ctx)
		return -EINVAL;
	if (!name)
		return -EINVAL;
	if (!fns)
		return -EINVAL;
	if (!fns->init || !fns->main || !fns->exit)
		return -EINVAL;

	cse = malloc(sizeof(struct belayd_cause));
	if (!cse)
		return -ENOMEM;

	memset(cse, 0, sizeof(struct belayd_cause));

	cse->name = strdup(name);
	if (!cse->name) {
		ret = -ENOMEM;
		goto err;
	}

	/*
	 * The index is saved off for debugging and convenience.  Since this is a registered
	 * cause, it doesn't have an index into the enum causes enumeration.
	 */
	cse->idx = -1;
	cse->fns = fns;
	cse->next = NULL;

	if (!registered_causes)
		/*
		 * This is the first cause in the registered linked list
		 */
		registered_causes = cse;
	else
		registered_causes->next = cse;

	return ret;

err:
	if (cse && cse->name)
		free(cse->name);
	if (cse)
		free(cse);
	return ret;
}

void *belayd_cause_get_data(const struct belayd_cause * const cse)
{
	return cse->data;
}

int belayd_cause_set_data(struct belayd_cause * const cse, void * const data)
{
	cse->data = data;
	return 0;
}

void cause_init(void)
{
	registered_causes = NULL;
}

void cause_cleanup(void)
{
	struct belayd_cause *next, *cur;

	cur = registered_causes;

	while (cur) {
		next = cur->next;
		free(cur->name);
		free(cur);

		cur = next;
	}
}
