/*	npopen:  like popen, but grabs stderr, too	*/
/*		written by John Hutchinson, heavily modified by Paul Fox */

#include <stdio.h>
#include "estruct.h"
#include "edef.h"

#if UNIX

#include <sys/signal.h>
#include <errno.h>
#include <sys/param.h>

#if BSD
#define strrchr rindex
#endif

#define R 0
#define W 1

extern char *getenv();
extern char *strrchr();

static int pid;

FILE *
npopen (cmd, type)
char *cmd, *type;
{
	int tries = 5;
	unsigned slp = 1;
	int p[2];
	int typ;
	char *sh, *shname;
	
	if (!strcmp(type, "r"))
		typ = R;
	else if (!strcmp(type, "w"))
		typ = W;
	else
		return NULL;

	if (pipe(p))
		return NULL;
		
	/* Try & fork 5 times, backing off 1, 2, 4 .. seconds each try */
	while ((pid = fork ()) < 0) {
		if (--tries == 0)
			return NULL;
		(void) sleep (slp);
		slp <<= 1;
	}

	if (pid) { /* parent */
		FILE *filep;

		if (typ == R) {
			filep = fdopen (p[0], "r");
			setbuf(filep,NULL);
			(void) close (p[1]);
		} else {
			filep = fdopen (p[1], "w");
			setbuf(filep,NULL);
			(void) close (p[0]);
		}
		return filep;

	} else {			/* child */
		int i;

		if (typ == R) { /* then child wants to write */
#if close_stdin
			(void)close (0);
			if (open("/dev/null",0) != 0)
				exit(-1);
#endif
			(void)close (1);
			if (dup (p[1]) != 1)
				exit(-1);
			(void)close (2);
			if (dup (p[1]) != 2)
				exit(-1);
		} else { /* child wants to read */
			(void)close (0);
			if (dup (p[0]) != 0)
				exit(-1);
		}
			
		/* Make sure there are no inherited file descriptors */
		for (i = 3; i < NOFILE; i += 1)
			(void) close (i);

#if ! MY_EXEC
	        if ((sh = getenv("SHELL")) == NULL || *sh == '\0') {
			sh = "/bin/sh";
			shname = "sh";
		} else {
			shname = strrchr(sh,'/');
			if (shname == NULL)
				shname = sh;
			else {
				shname++;
				if (*shname == '\0')
					shname = sh;
			}
		}
		(void) execl (sh, shname, "-c", cmd, 0);
#else
		my_exec(cmd);
#endif
		exit (-1);

	}
}

npclose (fp)
FILE *fp;
{
	fflush(fp);
	fclose(fp);
	if (wait ((int *)0) < 0 && errno == EINTR) {
		(void) kill (SIGKILL, pid);
	}
	return 0;
}

#if MY_EXEC

static
my_exec (cmd)
register char *cmd;
{
	char *argv [256];
	register char **argv_p, *cp;
	
        if ((argv[0] = getenv("SHELL")) == NULL || argv[0][0] == '\0') {
		argv[0] = "/bin/sh";
		argv[1] = "sh";
	} else {
		argv[1] = strrchr(argv[0]);
		if (argv[1] == NULL) {
			argv[1] = argv[0];
		} else {
			argv[1]++;
			if (argv[1][0] == '\0')
				argv[1] = argv[0];
		}
	}
	argv[2] = "-c";

	argv_p = &argv[3];

	cp = cmd;
	/* Scan up cmd, splitting arguments into argv. This is the
	 * child, so we can zap things in cmd safely */
	
	while (*cp) {
		/* Skip any white space */
		while (*cp && isspace(*cp))
			cp++;
		if (!*cp)
			break;
		*argv_p++ = cp;
		while (*cp && !isspace(*cp))
			cp++;
		if (!*cp)
			break;
		*cp++ = '\0';
	}
	*argv_p = NULL;
	execv (argv[0], &argv[1]);
}
#endif

#else
npopenhello() {}
#endif
