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
#include "effect.h"
#include "cause.h"

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

int parse_string(struct json_object * const obj, const char * const key, const char **value)
{
	struct json_object *key_obj;
	json_bool exists;
	int ret = 0;

	exists = json_object_object_get_ex(obj, key, &key_obj);
	if (!exists || !key_obj) {
		ret = -EINVAL;
		goto error;
	}

	*value = json_object_get_string(key_obj);
	if (!(*value)) {
		ret = -EINVAL;
		goto error;
	}

	return ret;

error:
	return ret;
}

static int parse_cause(struct rule * const rule, struct json_object * const cause_obj)
{
	bool found_cause = false;
	struct cause *cse = NULL;
	const char *name;
	int ret = 0;
	int i;

	ret = parse_string(cause_obj, "name", &name);
	if (ret )
		goto error;

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
			cse->fns = &cause_fns[i];

			ret = (*cse->fns->init)(cse, cause_obj);
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

static int parse_effect(struct rule * const rule, struct json_object * const effect_obj)
{
	bool found_effect = false;
	struct effect *eff = NULL;
	const char *name;
	int ret = 0;
	int i;

	ret = parse_string(effect_obj, "name", &name);
	if (ret )
		goto error;

	eff = malloc(sizeof(struct effect));
	if (!eff) {
		ret = -ENOMEM;
		goto error;
	}

	memset(eff, 0, sizeof(struct effect));

	eff->name = malloc(strlen(name) + 1);
	if (!eff->name) {
		ret = -ENOMEM;
		goto error;
	}

	strcpy(eff->name, name);

	for (i = 0; i < EFFECT_CNT; i++) {
		if (strncmp(name, effect_names[i], strlen(effect_names[i])) == 0) {
			found_effect = true;
			eff->idx = i;
			eff->fns = &effect_fns[i];

			ret = (*eff->fns->init)(eff, effect_obj, rule->causes);
			if (ret)
				goto error;

			break;
		}
	}

	if (!found_effect)
		goto error;

	/*
	 * do not goto error after this point.  we have added the eff
	 * to the effects linked list
	 */
	if (!rule->effects)
		rule->effects = eff;
	else
		rule->effects->next = eff;

	return ret;

error:
	if (eff && eff->name)
		free(eff->name);

	if (eff)
		free(eff);

	return ret;
}

static int parse_rule(struct belayd_opts * const opts, struct json_object * const rule_obj)
{
	struct json_object *causes_obj, *cause_obj, *effects_obj, *effect_obj;
	int i, cause_cnt, effect_cnt;
	struct rule *rule = NULL;
	json_bool exists;
	const char *name;
	int ret = 0;

	ret = parse_string(rule_obj, "name", &name);
	if (ret )
		goto error;

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

	/*
	 * Parse the causes
	 */
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
	 * Parse the effects
	 */
	exists = json_object_object_get_ex(rule_obj, "effects", &effects_obj);
	if (!exists || !effects_obj) {
		ret = -EINVAL;
		goto error;
	}

	effect_cnt = json_object_array_length(effects_obj);

	for (i = 0; i < effect_cnt; i++) {
		effect_obj = json_object_array_get_idx(effects_obj, i);
		if (!effect_obj) {
			ret = -EINVAL;
			goto error;
		}

		ret = parse_effect(rule, effect_obj);
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
