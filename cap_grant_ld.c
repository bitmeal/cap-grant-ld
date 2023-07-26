/*
 * Run executable with capabilities set on cap_grant with LD_LIBRARY_PATH from parent process
 * 
 * dependencies: libcap-ng-dev libprocps-dev
 * 
 * inspired by and in parts taken from:
 * https://gist.github.com/infinity0/596845b5eea3e1a02a018009b2931a39
 * 
 */


// allow execvpe()
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>

#include <sys/prctl.h>
#include <cap-ng.h>
#include <proc/readproc.h>



int verbose = 0;

void usage(const char *me, const char* optstring)
{
	printf("Usage: %s [-%s] program [args [...]]\n", me, optstring + 1);
	exit(1);
}

static void set_ambient_cap(int cap)
{
	if (verbose)
	{
		const char *cap_name = capng_capability_to_name(cap);
		if (cap_name == NULL)
			cap_name = "unknown";
		printf("- [%02d] %s\n", cap, cap_name);
	}

	capng_get_caps_process();

	// add future ambient capability to inheritable set first
	int rc = capng_update(CAPNG_ADD, CAPNG_INHERITABLE, cap);
	if (rc)
	{
		printf("Cannot add inheritable cap\n");
		exit(2);
	}
	capng_apply(CAPNG_SELECT_CAPS);

	// add capability to ambient set
	/* Note the two 0s at the end. Kernel checks for these */
	if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0))
	{
		perror("Cannot set cap");
		exit(1);
	}
}

void caps_to_ambient_from(capng_type_t cap_type)
{
	if (verbose)
	{
		printf("cloning ");
		switch(cap_type)
		{
			case CAPNG_PERMITTED:
				printf("PERMITTED");
				break;
			case CAPNG_EFFECTIVE:
				printf("EFFECTIVE");
				break;
			case CAPNG_INHERITABLE:
				printf("INHERITABLE");
				break;
			default:
				printf("UNKNOWN");
		}
		printf(" caps:\n");
	}

	for (int i = 0; i <= CAP_LAST_CAP; i++)
	{
		if (capng_have_capability(cap_type, i))
		{
			set_ambient_cap(i);
		}
	}
}

void add_env_from_pid(const char *envvar, pid_t pid)
{
	pid_t pidList[] = {pid, 0};

	// read environment info from parent process (ppid in pidList)
	// https://stackoverflow.com/a/69280824
	// https://manpages.debian.org/testing/procps/openproc.3.en.html
	PROCTAB *proc = openproc(PROC_PID | PROC_FILLENV, pidList);
	proc_t proc_info;
	memset(&proc_info, 0, sizeof(proc_info));

	int envvar_found = 0;
	while (readproc(proc, &proc_info) != NULL)
	{
		// iterate over environ and find requested variable
		for (int i = 0; proc_info.environ[i] != NULL; i++)
		{
			// get a copy and split by equal sign
			char *proc_env_entry = malloc(strlen(proc_info.environ[i]) + 1);
			strcpy(proc_env_entry, proc_info.environ[i]);

			// https://linux.die.net/man/3/strtok_r
			char *proc_env_name = strtok(proc_env_entry, "=");
			char *proc_env_value = strtok(NULL, "=");

			// set env if found requested variable
			if (!strcmp(proc_env_name, envvar))
			{
				envvar_found = 1;
				if (verbose)
				{
					printf("- %s=%s\n", envvar, proc_env_value);
				}

				setenv(envvar, proc_env_value, 1);

				free(proc_env_entry);
				// done
				break;
			}

			// cleanup
			free(proc_env_entry);
		}
	}
	closeproc(proc);

	if (!envvar_found)
	{
		fprintf(stderr, "could not find $%s\n", envvar);
	}
}

int main(int argc, char **argv)
{
	capng_type_t clone_type = CAPNG_PERMITTED;
	int inject_ld_lib_path = 1;

	opterr = 0;
	int flag;
	// use '+' in optstring to disable GNUs reordering
	char optstring[] = "+vlep";
	while ((flag = getopt(argc, argv, optstring)) != -1)
	{
		switch (flag)
		{
		case 'l':
			inject_ld_lib_path = 0;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'e':
			clone_type = CAPNG_EFFECTIVE;
			break;
		case 'p':
			clone_type = CAPNG_PERMITTED;
			break;
		case '?':
			if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr,
						"Unknown option character `\\x%x'.\n",
						optopt);
			return 1;
		default:
			abort();
		}
	}

	// test for enough remaining arguments to exec something
	if (optind == argc)
	{
		usage(argv[0], optstring);
	}

	// clone capabilities to ambient
	caps_to_ambient_from(clone_type);

	// inject LD_LIBRARY_PATH
	if(inject_ld_lib_path)
	{
		pid_t ppid = getppid();

		if (verbose)
		{
			// pid_t printing: https://stackoverflow.com/a/44504623
			printf("injecting environment from [%jd]:\n", (intmax_t)ppid);
		}
		add_env_from_pid("LD_LIBRARY_PATH", ppid);
	}
	extern char **environ;

	// drop privileges
	setuid(getuid());
	setgid(getgid());

	// exec
	if (verbose)
	{
		printf("executing:");
		for (int i = optind; i < argc; i++)
		{
			printf(" %s", argv[i]);
		}
		printf("\n");
	}

	if (execvpe(argv[optind], argv + optind, environ))
	{
		perror("exec failed!");
	}

	return 0;
}