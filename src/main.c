#include <stdio.h>

int parse_opts(int argc, char *argv[], struct belayer_opts * const opts)
{
	struct option long_options[] = {
		{"help",		no_argument, NULL, 'h'},
		{"config",	  required_argument, NULL, 'c'},
		{"interval",	  required_argument, NULL, 'i'},
		{NULL, 0, NULL, 0}
	};
}

int main(int argc, char *argv[])
{
	struct belayer_opts opts;
	int ret;

	ret = parse_opts(argc, argv, &opts);
	if (ret)
		goto out;

	printf("main belayer\n");

out:
	return ret;
}
