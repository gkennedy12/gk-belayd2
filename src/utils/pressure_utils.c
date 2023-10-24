// LICENSE TBD
/**
 * pressure cause
 *
 * This file processes pressure causes
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#define _XOPEN_SOURCE

#include <stdbool.h>
#include <assert.h>
//#define  _GNU_SOURCE
//#define  _POSIX_C_SOURCE 200809L
#define __USE_XOPEN2K8
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "belayd-internal.h"
#include "defines.h"

#include "utils/pressure_utils.h"

static int verbose = 0;

struct pressure_values *
parse_pressure(char *what, char *pfn, struct pressure_values *pp)
{
        char some[256];
        char full[256];
	char *pstr;
	char *subp;
        FILE *fp;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

	if (verbose > 3) fprintf(stderr, "parse_pressure: what=%s, pfn=%s\n", what, pfn);

        fp = fopen(pfn, "r");
        if (fp == NULL) {
                perror("open");
                return NULL;
        }
        while ((read = getline(&line, &len, fp)) != -1) {
		if (strncmp(line, "some", 4) == 0)
			strcpy(some, line);
		else if (strncmp(line, "full", 4) == 0)
			strcpy(full, line);
        }
        fclose(fp);

	if (verbose > 3) {
		fprintf(stderr, "%s", some);
		fprintf(stderr, "%s", full);
	}

	if (strcmp(what, "some") == 0)
		pstr = some;
	else if (strcmp(what, "full") == 0)
		pstr = full;
	else
		return NULL;

	subp = strstr(pstr, "avg10");
	subp = strstr(pstr, "=");
	pp->avg10 = strtof(&subp[1], NULL);

	subp = strstr(subp, "avg60");
	subp = strstr(subp, "=");
	pp->avg60 = strtof(&subp[1], NULL);

	subp = strstr(subp, "avg300");
	subp = strstr(subp, "=");
	pp->avg300 = strtof(&subp[1], NULL);

	if (verbose > 3)
		fprintf(stderr, "XXX parse_pressure: %2.2f %2.2f %2.2f\n", pp->avg10, pp->avg60, pp->avg300);

	return pp;
}
