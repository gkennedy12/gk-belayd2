// LICENSE TBD
/**
 * memory resize effect
 *
 * This file runs the mem_resize effect
 *
 * Copyright (c) 2023 Oracle and/or its affiliates.
 * Author: Tom Hromatka <tom.hromatka@oracle.com>
 */

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "belayd-internal.h"
#include "defines.h"

#include "utils/pressure_utils.h"

struct vm_ports {
        int online;
        int pid;
	int block_size;
	long long requested_size;
	int maxmem;
        char *path;
        char *socket_path;
        char *name;
};

#define MAX_VMS         32
#define GB              (1024ULL * 1024ULL * 1024ULL)

#define DEFAULT_GRANULARITY	2147483648
#define DEFAULT_MIN_LOW_MEM	6442450944

static int verbose = 0;

struct mem_resize_opts {
        char *what;
        char *pressure_slice;
        char *qmp_path;
        int threshold;
	char *managed_str;
        int post_action_delay;
        int memory_reclaim;
        long long max_mem;
	const struct cause *cse;
};

int mem_resize_init(struct effect * const eff, struct json_object *eff_obj,
	       const struct cause * const cse)
{
	struct json_object *args_obj;
	struct mem_resize_opts *opts;
	const char *opts_str;
	const char *pressure_slice;
	const char *managed_str;
	const char *qmp_path_str;
	json_bool exists;
	int ret = 0;

	opts = malloc(sizeof(struct mem_resize_opts));
	if (!opts) {
		ret = -ENOMEM;
		goto error;
	}
	memset(opts, 0, sizeof(struct mem_resize_opts));

	exists = json_object_object_get_ex(eff_obj, "args", &args_obj);
	if (!exists || !args_obj) {
		ret = -EINVAL;
		goto error;
	}

        ret = parse_string(args_obj, "verbose", &opts_str);
        if (ret == 0) {
		verbose = atoi(opts_str);
	}
	if (verbose) fprintf(stderr, "\nXXX %s:\n", __func__);

        ret = parse_string(args_obj, "threshold", &opts_str);
        if (ret)
                goto error;

        opts->threshold = atoi(opts_str);
        if (verbose) fprintf(stderr, "mem_resize_init: opts->threshold: %d\n", opts->threshold);

        ret = parse_string(args_obj, "pressure_slice", &pressure_slice);
        if (ret)
                goto error;

        opts->pressure_slice = malloc(sizeof(char) * strlen(pressure_slice));
        if (!opts->pressure_slice) {
		fprintf(stderr, "mem_resize_init: pressure filename option missing.\n");
                ret = -ENOMEM;
                goto error;
        }

        strcpy(opts->pressure_slice, pressure_slice);

        if (verbose)
		fprintf(stderr, "mem_resize_init: opts->pressure_slice: %s\n", opts->pressure_slice);

        ret = parse_string(args_obj, "qmp_path", &qmp_path_str);
        if (ret)
                goto error;

        opts->qmp_path = malloc(sizeof(char) * strlen(qmp_path_str));
        if (!opts->qmp_path) {
		fprintf(stderr, "mem_resize_init: qmp_path option missing.\n");
                ret = -ENOMEM;
                goto error;
        }

        strcpy(opts->qmp_path, qmp_path_str);

        if (verbose)
		fprintf(stderr, "mem_resize_init: opts->qmp_path: %s\n", opts->qmp_path);

	ret = parse_string(args_obj, "managed_str", &managed_str);
	if (!ret) {
		opts->managed_str = malloc(sizeof(char) * strlen(managed_str));
		if (!opts->managed_str) {
			ret = -ENOMEM;
			goto error;
		}

		strcpy(opts->managed_str, managed_str);

		if (verbose) fprintf(stderr, "managed_str: opts->managed_str: %s\n", opts->managed_str);
	}

        ret = parse_string(args_obj, "post_action_delay", &opts_str);
        if (ret) {
		opts->post_action_delay = 0;
	} else {
		opts->post_action_delay = strtoll(opts_str, 0, 0);
	}
	if (verbose) fprintf(stderr, "mem_resize_init: opts->post_action_delay: %d\n",
		opts->post_action_delay);

        ret = parse_string(args_obj, "memory_reclaim", &opts_str);
        if (ret) {
		opts->memory_reclaim = 0;
	} else {
		opts->memory_reclaim = strtoll(opts_str, 0, 0);
	}
	if (verbose) fprintf(stderr, "mem_resize_init: opts->memory_reclaim: %d\n",
		opts->memory_reclaim);

        ret = parse_string(args_obj, "max_mem", &opts_str);
        if (ret == 0) {
		opts->max_mem = atoi(opts_str);
		opts->max_mem *= GB;
	} else {
		opts->max_mem = 0;
	}
	if (verbose) fprintf(stderr, "mem_resize_init: opts->max_mem: %lld\n",
		opts->max_mem);

	ret = 0;

	opts->cse = cse;

	/* we have successfully setup the mem_resize effect */
	eff->data = (void *)opts;

	fprintf(stderr, "mem_resize_init: OK!, ret=%d\n", ret);
	return ret;

error:
	fprintf(stderr, "mem_resize_init: got an error!\n");
	if (opts)
		free(opts);

	return ret;
}

void runcmds(char *cmdp, char *rtnp)
{
        FILE *fp;
        char buf[1024];

        if (rtnp == NULL)
                return;

        rtnp[0] = 0;

	memset(buf, 0, 1024);

        if (verbose > 3) fprintf(stderr, "XXX runcmds: %s\n", cmdp);
        fp = popen(cmdp, "r");
        while (fgets(buf, sizeof(buf), fp) != NULL) {
                buf[strcspn(buf, "\n")] = 0;
//              strcpy(&rtnp[strlen(rtnp)], buf);
		strcpy(rtnp, buf);
        }
        pclose(fp);

        if (verbose > 3) {
                if (strlen(rtnp))
                        fprintf(stderr, "XXX runcmds(return): rtnp=%s\n", rtnp);
                else
                        fprintf(stderr, "XXX runcmds(return): FAILED\n");
        }
}

int runcmdi(char *cmdp)
{
        FILE *fp;
        char buf[256];
        int ret = -1;

        if (verbose > 3) fprintf(stderr, "XXX runcmdi: %s\n", cmdp);
        fp = popen(cmdp, "r");
        while (fgets(buf, sizeof(buf), fp) != NULL) {
                // if (buf[strlen(buf) - 1] = 0xa)
                        // buf[strlen(buf) - 1] = 0;
                ret = strtol(buf, 0, 0);
		break;
        }
        pclose(fp);
        if (verbose > 3) fprintf(stderr, "XXX runcmdi(return): ret=%d\n", ret);
        return ret;
}

long long runcmdll(char *cmdp)
{
        FILE *fp;
        char buf[256];
        long long ret = -1;

        if (verbose > 3) fprintf(stderr, "XXX runcmdll: %s\n", cmdp);
        fp = popen(cmdp, "r");
        while (fgets(buf, sizeof(buf), fp) != NULL) {
                buf[strcspn(buf, "\n")] = 0;
                ret = strtoll(buf, 0, 0);
                if (verbose > 3) fprintf(stderr, "XXX runcmdll: Received: %s (%lld)\n", buf, ret);
		break;
        }
        pclose(fp);
        if (verbose > 3) fprintf(stderr, "XXX runcmdll(return): ret=%lld\n", ret);
        return ret;
}

long long get_stat(char *stat_fn, char *pp)
{
        long long ret = 0;
        char cmdline[512];

        if (pp == NULL)
                return 0;

        if (verbose > 3) fprintf(stderr, "XXX get_stat: stat_fn: %s, path=%s\n", stat_fn, pp);

        sprintf(cmdline, "cat \"%s/%s\"", pp, stat_fn);
        ret = runcmdll(cmdline);

        return ret;
}

#define TO "timeout --foreground -s KILL 15"

int get_block_size(char *socket_path, char *qmp_path)
{
        char cmdline[256];

        sprintf(cmdline,
	    "%s echo 'qom-get vm0 block-size' | %s/qmp-shell -H %s |"
		"grep '^(QEMU)' |awk '{print $2}'",
	    TO, qmp_path, socket_path);
	return (runcmdi(cmdline));
}

long long get_requested_size(char *socket_path, char *qmp_path)
{
        char cmdline[256];

        sprintf(cmdline,
	    "%s echo 'qom-get vm0 requested-size' | %s/qmp-shell -H %s |"
		"grep '^(QEMU)' |awk '{print $2}'",
	    TO, qmp_path, socket_path);
	return (runcmdll(cmdline));
}

int set_requested_size(char *socket_path, char *qmp_path, int gb)
{
        char cmdline[256];

        sprintf(cmdline,
	    "%s echo 'qom-set vm0 requested-size %dG' | %s/qmp-shell -H %s",
	    TO, gb, qmp_path, socket_path);
	if (verbose)
		fprintf(stderr, "set_requested_size: cmdline: %s\n", cmdline);
	return(system(cmdline));
}

void set_mem_high(char *path, long long new_mem_high)
{
        char cmdline[2048];

        sprintf(cmdline, "echo %lld > \"%s\"", new_mem_high, path);
        if (verbose) fprintf(stderr, "XXX set_mem_high: cmdline: %s\n", cmdline);
        system(cmdline);
}

int find_vms(struct vm_ports *vmp, struct mem_resize_opts *opts, long long *total_requested_size)
{
        FILE *fp, *fp2;
        char *line = NULL, *line2 = NULL;
        size_t len = 0;
        ssize_t read;
        char cmdline[512];
        int i, j;
        int ret;

        if (verbose) fprintf(stderr, "XXX ------------------------ find_vms: ------------------------\n");

	for (j = 0; j < MAX_VMS ; j++)
                vmp[j].online = 0;

	sprintf(cmdline, "timeout --foreground -s KILL 15 find /sys/fs/cgroup/%s.slice/ -name cgroup.procs |grep -v %s.slice/cgroup.procs >/tmp/procs.out 2>&1", opts->pressure_slice, opts->pressure_slice);
	if (verbose > 3) fprintf(stderr, "find_vms: cmdline: %s\n", cmdline);
	ret = system(cmdline);
        if (ret) {
                fprintf(stderr, "XXX find_vms: ret: %d\n", ret);
                return 0;
        }
        fp = fopen("/tmp/procs.out", "r");
        if (fp == NULL) {
                perror("open");
                return 0;
        }
	i = 0;
        while ((read = getline(&line, &len, fp)) != -1) {
                line[strcspn(line, "\n")] = 0;
		fp2 = fopen(line, "r");
		if (fp2 == NULL) {
			perror("open");
			return 0;
		}
		while ((read = getline(&line2, &len, fp2)) != -1) {
			line2[strcspn(line2, "\n")] = 0;
			// ps -ed |grep 2871364 |grep qemu-system-x86
			sprintf(cmdline, "ps -edf |grep %s |grep qemu-system-x86 |grep -v grep >/tmp/qemu.out", line2);
			if (verbose > 3) fprintf(stderr, "find_vms: cmdline: %s\n", cmdline);
			ret = system(cmdline);
			if (ret == 0) {
				char buf[512];
				char *subp;

				memset(&vmp[i], 0, sizeof(struct vm_ports));

                                vmp[i].pid = runcmdi("awk '{print $2}' /tmp/qemu.out");
				if (opts->managed_str) {
					// ps -edf |grep 3831423 |grep "name belayd_managed_vm" |sed 's/^.*name //' |sed 's/ .*//'
					buf[0] = 0;
					sprintf(cmdline, "grep \"name %s\" /tmp/qemu.out |sed 's/^.*-name //' |sed 's/ .*//'",
						opts->managed_str);
					runcmds(cmdline, buf);
					if (buf[0] == 0) {
						fprintf(stderr, "find_vms: ret=%d, qemu PID %d, not %s\n",
							ret, vmp[i].pid, opts->managed_str);
						continue;
					}
					vmp[i].name = (char *)malloc(strlen(buf) + 1);
					strcpy(vmp[i].name, buf);
				}
				subp = strstr(line, "/cgroup.procs");
				subp[0] = 0;
				//line[strstr(line, "/cgroup.procs")] = 0;
				vmp[i].path = (char *)malloc(strlen(line) + 1);
                                strcpy(vmp[i].path, line);
				sprintf(cmdline, "cat /tmp/qemu.out |sed 's/^.*-qmp unix://' |sed 's/socket,.*/socket/'");
				runcmds(cmdline, buf);
				vmp[i].socket_path = (char *)malloc(strlen(buf) + 1);
                                strcpy(vmp[i].socket_path, buf);
				if (verbose > 3) fprintf(stderr, "find_vms: socket_path=%s\n", vmp[i].socket_path);
				// ps -edf |grep qemu |grep 1571 |sed 's/^.*memory-backend-ram,//' |sed 's/ -.*//' |sed 's/^.*=//' |sed 's/G.*//'
				sprintf(cmdline, "cat /tmp/qemu.out |sed 's/^.*memory-backend-ram,//' |sed 's/ -.*//' |sed 's/^.*=//' |sed 's/G.*//'");
				vmp[i].maxmem = runcmdi(cmdline);
				vmp[i].block_size = get_block_size(vmp[i].socket_path, opts->qmp_path);
				vmp[i].requested_size = get_requested_size(vmp[i].socket_path, opts->qmp_path);
				*total_requested_size += vmp[i].requested_size;
				vmp[i].online = 1;
				i++;
				break;
			}
		}
		fclose(fp2);
        }
        fclose(fp);

	return i;
}

void dump_vms(struct vm_ports *vmp)
{
        int i;

        fprintf(stderr, "DUMP VMs:\n");
        for (i = 0; i < MAX_VMS; i++) {
                if (!vmp[i].online)
                        continue;
                fprintf(stderr, "================== %d: pid=%d ==================\n", i, vmp[i].pid);
                fprintf(stderr, "block_size=%dK, requested_size=%lld, maxmem=%dG\n",
		    vmp[i].block_size, vmp[i].requested_size, vmp[i].maxmem);
                fprintf(stderr, "path=%s\n", vmp[i].path);
                fprintf(stderr, "socket_path=%s\n", vmp[i].socket_path);
		if (vmp[i].name)
			fprintf(stderr, "name=%s\n", vmp[i].name);
        }
	fprintf(stderr, "\n");
}

int handlepressure(struct mem_resize_opts *opts)
{
	struct vm_ports vm_ports[MAX_VMS];
	int vm_count = 0;
	int i;
	char pp[1024];
	char slice_str[256];
	char press_str[512];
	char high_str[512];
	char cmdline[2048];
	int machine_pressure_exists = 0;
	int vm_pressure_exists = 0;
	int total_action_taken = 0;
	int action_taken = 0;
	struct pressure_values mach_pressure, *mp;
	struct pressure_values vm_pressure, *vp;
	int full_quiet = 0;
	long long mach_memory_current = 0;
	long long mach_memory_max = 0;
	long long mach_memory_high = 0;
	long long requested_size = 0;
	long long total_requested_size = 0;
	long long x, y;
	int reqsz;

	if (verbose) {
		fprintf(stderr, "\nXXX Handling Memory pressure...\n");
		system("date");
	}

	sprintf(cmdline, "cat /sys/fs/cgroup/%s.slice/memory.pressure", opts->pressure_slice);
	if (verbose > 3) fprintf(stderr, "handlepressure: cmdline: %s\n", cmdline);
	system(cmdline);

	memset(vm_ports, 0, sizeof(struct vm_ports) * MAX_VMS);

	sprintf(slice_str, "/sys/fs/cgroup/%s.slice", opts->pressure_slice);
	sprintf(press_str, "%s/memory.pressure", slice_str);
	sprintf(high_str, "%s/memory.high", slice_str);

	vm_count = find_vms(vm_ports, opts, &total_requested_size);
	if (verbose) fprintf(stderr, "handlepressure: vm_count=%d\n", vm_count);
	if (!vm_count) {
		fprintf(stderr, "XXX No VMs running. Nothing to do.\n");
		system("date");
		goto handlepressure_cleanup;
	}
	if (verbose > 1) dump_vms(vm_ports);

	if (verbose > 1) fprintf(stderr, "total_requested_size=%lld, opts->max_mem=%lld\n",
	    total_requested_size, opts->max_mem);
	if (opts->memory_reclaim && !total_requested_size) {
		fprintf(stderr, "\nXXX Memory reclaim and no \"requested-size\" memory allocated.\n\n");
		return 0;
	}
	mp = &mach_pressure;
	mp = parse_pressure("some", press_str, mp);
	if (mp == NULL) {
		fprintf(stderr, "XXX Can't get pressure from %s\n", opts->pressure_slice);
		goto handlepressure_cleanup;
	}
	if (verbose) fprintf(stderr, "XXX mp->avg10=%2.2f (%d), mp->avg60=%2.2f\n",
		mp->avg10, (int)mp->avg10, mp->avg60);

	if (full_quiet) {
		if (mp->avg10 || mp->avg60 || mp->avg300)
			machine_pressure_exists = 1;
	} else {
		if ((int)mp->avg10)
			machine_pressure_exists = 1;
	}

	mach_memory_current = get_stat("memory.current", slice_str);
	mach_memory_max = get_stat("memory.max", slice_str);
	mach_memory_high = get_stat("memory.high", slice_str);

	if (verbose) {
		fprintf(stderr, "XXX THRESHOLD: %d, MEMORY_RECLAIM: %d, POST_ACTION_DELAY %d\n",
		    opts->threshold, opts->memory_reclaim, opts->post_action_delay);
		fprintf(stderr, "XXX mach_memory.current=%lld\n", mach_memory_current);
		fprintf(stderr, "XXX mach_memory.max=%lld\n", mach_memory_max);
		fprintf(stderr, "XXX mach_memory.high=%lld\n", mach_memory_high);
	}
	if (!opts->memory_reclaim && (total_requested_size >= opts->max_mem)) {
		fprintf(stderr, "\nXXX SKIP. total_requested_size=%lld >= opts->max_mem=%lld\n\n",
		    total_requested_size, opts->max_mem);
		goto handlepressure_cleanup;
	}

	for (i = 0; i < MAX_VMS; i++) {
		if (!vm_ports[i].online)
			continue;
		action_taken = 0;
		if (verbose) fprintf(stderr,
		    "---------------------- vm_ports[%d].path=%s ----------------------\n",
			 i, vm_ports[i].path);

		sprintf(pp, "%s/%s", vm_ports[i].path, "memory.pressure");
		if (verbose) {
			sprintf(cmdline, "cat \"%s\" |grep some", pp);
			fprintf(stderr, "cmdline: %s\n", cmdline);
			system(cmdline);
		}
		vp = &vm_pressure;
		char s[] = "some";
		vp = parse_pressure(s, pp, vp);
		if (vp == NULL) {
			fprintf(stderr, "XXX Can't get pressure for %s\n", pp);
			continue;
		}

		long long memory_current;
		long long memory_max;
		long long memory_high;
		long long new_mem_high = 0;
		long long mach_new_mem_high = 0;

		memory_current = get_stat("memory.current", vm_ports[i].path);
		memory_max = get_stat("memory.max", vm_ports[i].path);
		memory_high = get_stat("memory.high", vm_ports[i].path);

		if (verbose > 1) {
			fprintf(stderr, "XXX memory.current = %lld\n", memory_current);
			fprintf(stderr, "XXX memory.max =     %lld\n", memory_max);
			fprintf(stderr, "XXX memory.high =    %lld\n", memory_high);
			fprintf(stderr, "XXX vp->avg10=%2.2f (%d), vp->avg60=%2.2f\n",
			    vp->avg10, (int)vp->avg10, vp->avg60);
		}
		vm_pressure_exists = 0;
		if (full_quiet) {
			if (vp->avg10 || vp->avg60 || vp->avg300)
				vm_pressure_exists = 1;
		} else {
			if ((int)vp->avg10)
				vm_pressure_exists = 1;
		}
		if (verbose) {
			fprintf(stderr, "XXX machine_pressure_exists=%d: %s vm_pressure_exists=%d\n",
			    machine_pressure_exists, vm_ports[i].path, vm_pressure_exists);
		}

		requested_size = get_requested_size(vm_ports[i].socket_path, opts->qmp_path);
		if (verbose > 1) fprintf(stderr, "XXX requested_size=%lld\n", requested_size);
		if (!memory_high && !mach_memory_high) {
			fprintf(stderr, "XXX SKIP: memory_high (%lld) and mach_memory_high (%lld) NOT set yet.\n", memory_high, mach_memory_high);
			continue;
		}
		if (opts->memory_reclaim) {
			if (verbose > 1) fprintf(stderr, "ATTEMPT MEMORY RECLAIM, opts->memory_reclaim=%d\n",
				opts->memory_reclaim);

			if (vm_pressure_exists) {
				if (verbose > 1) fprintf(stderr, "XXX SKIP: vm_pressure_exists=1, memory.high=%lld ; memory_current=%lld. Nothing to do.\n", memory_high, memory_current);

				// try to relieve pressure...
				if (!mach_memory_high) {
					new_mem_high = memory_current + (vm_ports[i].block_size * 1024);
					sprintf(pp, "%s/%s", vm_ports[i].path, "memory.high");
					set_mem_high(pp, new_mem_high);
				}
				continue;
			}
			if (requested_size) {
				y = vm_ports[i].block_size;
				y *= 1024;
				x = requested_size - y;
				reqsz = (int)(x / GB);
				int ret = -1;
				if (reqsz >= 0) {
					ret = set_requested_size(vm_ports[i].socket_path, opts->qmp_path, reqsz);
					if (verbose) fprintf(stderr,
					    "XXX handlepressure: set_requested_size(%s, %d) DELETE ret=%d\n",
					    vm_ports[i].socket_path, reqsz, ret);
				} else {
					if (verbose) fprintf(stderr, "XXX handlepressure: %s, reqsz %d bogus, can't reduce memory size.\n",
					    vm_ports[i].socket_path, reqsz);
				}
				if (ret == 0) {
					if (!mach_memory_high) {
						new_mem_high = x;
						if (verbose) fprintf(stderr, "XXX new_mem_high=%lld\n", new_mem_high);
					} else {
						mach_new_mem_high = mach_memory_current - (vm_ports[i].block_size * 1024);
						if (verbose) fprintf(stderr, "XXX mach_new_mem_high=%lld\n", mach_new_mem_high);
					}
					action_taken++;
					total_action_taken++;
				}
			} else {
				if (verbose) fprintf(stderr, "XXX handlepressure: SKIP: Nothing to do. requested_size=%lld\n", requested_size);
			}
		} else {
			if (!vm_pressure_exists) {
				if (verbose) fprintf(stderr, "XXX SKIP: vm_pressure_exists=0, memory.high=%lld ; memory_current=%lld. Nothing to do.\n", memory_high, memory_current);
				continue;
			}
			if ((int)vp->avg10 < opts->threshold) {
				if (verbose) fprintf(stderr, "XXX SKIP: vp->avg10 (%2.2f) (%d) < threshold (%d)\n", vp->avg10, (int)vp->avg10, opts->threshold);
				continue;
			}
			if (verbose) fprintf(stderr, "XXX Memory pressure EXISTS. Try to increase \"memory.high\"...\n");

			if ((requested_size / GB) < vm_ports[i].maxmem) {
				y = vm_ports[i].block_size;
				y *= 1024;
				fprintf(stderr, "XXX total_requested_size (%lld) + y (%lld) = %lld, opts->max_mem=%lld\n",
				    total_requested_size, y, (total_requested_size + y), opts->max_mem);
				if (total_requested_size + y > opts->max_mem) {
					fprintf(stderr, "\nXXX BREAK. total_requested_size (%lld) + y (%lld) > opts->max_mem=%lld\n\n",
					    total_requested_size, y, opts->max_mem);
					break;
				}
				x = requested_size + y;
				reqsz = (int)(x / GB);
				int ret = -1;
				if (reqsz <= vm_ports[i].maxmem) {
					ret = set_requested_size(vm_ports[i].socket_path, opts->qmp_path, reqsz);
					if (verbose) fprintf(stderr,
					    "XXX handlepressure: set_requested_size(%s, %d) ADD ret=%d\n",
					    vm_ports[i].socket_path, reqsz, ret);
				} else {
					if (verbose) fprintf(stderr, "XXX handlepressure: %s, reqsz %d > maxmem %d, can't increase memory size.\n",
					    vm_ports[i].socket_path, reqsz, vm_ports[i].maxmem);
				}
				if (ret == 0) {
					if (!mach_memory_high) {
						new_mem_high = x;
						if (verbose) fprintf(stderr, "XXX new_mem_high=%lld\n", new_mem_high);
					} else {
						mach_new_mem_high = mach_memory_current + (vm_ports[i].block_size * 1024);
						if (verbose) fprintf(stderr, "XXX mach_new_mem_high=%lld\n", mach_new_mem_high);
					}
					total_requested_size += y;
					if (verbose) fprintf(stderr, "XXX total_requested_size NOW %lld\n",
					    total_requested_size);
					action_taken++;
					total_action_taken++;
				}
			} else {
				if (verbose) fprintf(stderr, "XXX handlepressure: SKIP: (requested_size / GB) %d > maxmem %d\n",
					(int)(requested_size / GB), vm_ports[i].maxmem);
			}
		}
		if (action_taken) {
			requested_size = get_requested_size(vm_ports[i].socket_path, opts->qmp_path);
			if (verbose)
				fprintf(stderr, "XXX new_mem_high=%lld, NOW requested-size=%lld\n",
				    new_mem_high, requested_size);

			if (!mach_memory_high) {
				int increment = (action_taken) ? action_taken : 1;

				memory_current = get_stat("memory.current", vm_ports[i].path);
				sprintf(pp, "%s/%s", vm_ports[i].path, "memory.high");
				set_mem_high(pp, memory_current + (increment * 1 * GB));
			}
		}
	}
handlepressure_cleanup:
	if (verbose) fprintf(stderr, "XXX THRESHOLD: %d, MEMORY_RECLAIM: %d, POST_ACTION_DELAY %d\n",
	    opts->threshold, opts->memory_reclaim, opts->post_action_delay);
	if (verbose) fprintf(stderr, "handlepressure: total_action_taken=%d\n", total_action_taken);
	if (mach_memory_high) {
#if 0
		int increment = (total_action_taken) ? total_action_taken : 1;
#else
		int increment = 1;
#endif

		mach_memory_current = get_stat("memory.current", slice_str);
		if (verbose)
			fprintf(stderr,
			    "XXX mach_memory_current=%lld (NOW), increment=%d\n",
			    mach_memory_current, increment);
		set_mem_high(high_str, mach_memory_current + (increment * 1 * GB));
	}
	if (total_action_taken && opts->post_action_delay) {
		for (i = 0; i < opts->post_action_delay; i++) {
			sleep(1);
			if (verbose) {
				if (opts->memory_reclaim)
					fprintf(stderr, "-");
				else
					fprintf(stderr, "+");
			}
		}
		if (verbose) fprintf(stderr, "\n");
	}

	if (verbose) fprintf(stderr, "handlepressure: total_action_taken=%d, machine_pressure_exists=%d\n", total_action_taken, machine_pressure_exists);
	if (verbose) fprintf(stderr, "handlepressure: DONE\n\n");

	return total_action_taken;
}

int mem_resize_main(struct effect * const eff)
{
	struct mem_resize_opts *opts = (struct mem_resize_opts *)eff->data;
	int total_action_taken = 0;

	fprintf(stderr, "\nXXX %s:\n", __func__);

	if (verbose) fprintf(stderr, "Print effect triggered by: %s\n", opts->pressure_slice);

	total_action_taken = handlepressure(opts);
	if (verbose) fprintf(stderr, "mem_resize_main: total_action_taken=%d\n", total_action_taken);

	return 0;
}

void mem_resize_exit(struct effect * const eff)
{
	struct mem_resize_opts *opts = (struct mem_resize_opts *)eff->data;

	fprintf(stderr, "\nXXX %s:\n", __func__);

	free(opts);
}
