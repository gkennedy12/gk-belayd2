// LICENSE TBD
/**
 * File for parsing and managing the config file
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <json-c/json.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "belayd-internal.h"

static int get_file_size(FILE * const fd, long * const file_size)
{
	int ret;

	ret = fseek(fd, 0, SEEK_END);
	if (ret)
		return -errno;

	*file_size = ftell(fd);

	ret = fseek(fd, 0, SEEK_SET);
	if (ret)
		return -errno;

	return ret;
}

static int parse_cause(struct rule * const rule, struct json_object * const cause_obj)
{
	struct json_object *name_obj;
	bool found_cause = false;
	struct cause *cse = NULL;
	json_bool exists;
	const char *name;
	int ret = 0;
	int i;

	exists = json_object_object_get_ex(cause_obj, "name", &name_obj);
	if (!exists || !name_obj) {
		ret = -EINVAL;
		goto error;
	}

	name = json_object_get_string(name_obj);
	if (!name) {
		ret = -EINVAL;
		goto error;
	}

	cse = malloc(sizeof(struct cause));
	if (!cse) {
		ret = -ENOMEM;
		goto error;
	}

	memset(cse, 0, sizeof(struct cause));

	cse->name = malloc(strlen(name) + 1);
	if (!cse->name) {
		ret = -ENOMEM;
		goto error;
	}

	strcpy(cse->name, name);

	for (i = 0; i < CAUSE_CNT; i++) {
		if (strncmp(name, cause_names[i], strlen(cause_names[i])) == 0) {
			found_cause = true;
			cse->idx = i;

			ret = cause_inits[i](cse, cause_obj);
			if (ret)
				goto error;

			break;
		}
	}

	if (!found_cause)
		goto error;

	/*
	 * do not goto error after this point.  we have added the cse
	 * to the causes linked list
	 */
	if (!rule->causes)
		rule->causes = cse;
	else
		rule->causes->next = cse;

	return ret;

error:
	if (cse && cse->name)
		free(cse->name);

	if (cse)
		free(cse);

	return ret;
}

static int parse_rule(struct belayd_opts * const opts, struct json_object * const rule_obj)
{
	struct json_object *name_obj, *causes_obj, *cause_obj;
	struct rule *rule = NULL;
	json_bool exists;
	const char *name;
	int i, cause_cnt;
	int ret = 0;

	exists = json_object_object_get_ex(rule_obj, "name", &name_obj);
	if (!exists || !name_obj) {
		ret = -EINVAL;
		goto error;
	}

	name = json_object_get_string(name_obj);
	if (!name) {
		ret = -EINVAL;
		goto error;
	}

	rule = malloc(sizeof(struct rule));
	if (!rule) {
		ret = -ENOMEM;
		goto error;
	}

	memset(rule, 0, sizeof(struct rule));

	rule->name = malloc(strlen(name) + 1);
	if (!rule->name) {
		ret = -ENOMEM;
		goto error;
	}
	strcpy(rule->name, name);

	exists = json_object_object_get_ex(rule_obj, "causes", &causes_obj);
	if (!exists || !causes_obj) {
		ret = -EINVAL;
		goto error;
	}

	cause_cnt = json_object_array_length(causes_obj);

	for (i = 0; i < cause_cnt; i++) {
		cause_obj = json_object_array_get_idx(causes_obj, i);
		if (!cause_obj) {
			ret = -EINVAL;
			goto error;
		}

		ret = parse_cause(rule, cause_obj);
		if (ret)
			goto error;
	}

	/*
	 * do not goto error after this point.  we have added the rule
	 * to the rules linked list
	 */
	if (!opts->rules)
		opts->rules = rule;
	else
		opts->rules->next = rule;

	return ret;

error:
	if (rule && rule->name)
		free(rule->name);

	if (rule)
		free(rule);

	return ret;
}

static int parse_json(struct belayd_opts * const opts, const char * const buf)
{
	struct json_object *obj, *rules_obj, *rule_obj;
	enum json_tokener_error err;
	json_bool exists;
	int ret = 0, i;
	int rule_cnt;

	obj = json_tokener_parse_verbose(buf, &err);
	if (!obj || err) {
		ret = -EINVAL;
		goto out;
	}

	exists = json_object_object_get_ex(obj, "rules", &rules_obj);
	if (!exists || !rules_obj) {
		ret = -EINVAL;
		goto out;
	}

	rule_cnt = json_object_array_length(rules_obj);

	for (i = 0; i < rule_cnt; i++) {
		rule_obj = json_object_array_get_idx(rules_obj, i);
		if (!rule_obj) {
			ret = -EINVAL;
			goto out;
		}

		ret = parse_rule(opts, rule_obj);
		if (ret)
			goto out;
	}

out:
	return ret;
}

int parse_config(struct belayd_opts * const opts)
{
	FILE *config_fd = NULL;
	long config_size = 0;
	size_t chars_read;
	char *buf = NULL;
	int ret;

	config_fd = fopen(opts->config, "r");
	if (!config_fd) {
		ret = -errno;
		goto out;
	}

	ret = get_file_size(config_fd, &config_size);
	if (ret)
		goto out;

	buf = malloc(sizeof(char) * (config_size + 1));
	if (!buf) {
		ret = -ENOMEM;
		goto out;
	}

	chars_read = fread(buf, sizeof(char), config_size, config_fd);
	if (chars_read != config_size) {
		ret = -EIO;
		goto out;
	}
	buf[config_size] = '\0';

	ret = parse_json(opts, buf);
	if (ret)
		goto out;

out:
	if (config_fd)
		fclose(config_fd);

	if (buf)
		free(buf);

	return ret;
}
