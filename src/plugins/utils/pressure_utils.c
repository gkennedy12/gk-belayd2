// LICENSE TBD
/**
 * pressure parser
 *
 * This file parses PSI pressure
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: George Kennedy <george.kennedy@oracle.com>
 */

#define _XOPEN_SOURCE

#include <stdbool.h>
#include <assert.h>
#define __USE_XOPEN2K8
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <belayd.h>
// #include "defines.h"

// #include "utils/pressure_utils.h"
#include "pressure_utils.h"

struct pressure_values *
parse_pressure(char *what, char *pressure_fn, struct pressure_values *pp)
{
        char some[256];
        char full[256];
	char *pstr;
	char *subp;
        FILE *fp;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        fp = fopen(pressure_fn, "r");
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

	return pp;
}
