/*	npopen:  like popen, but grabs stderr, too
 *		written by John Hutchinson, heavily modified by Paul Fox
 *
 * $Log: npopen.c,v $
 * Revision 1.13  1992/05/25 21:07:48  foxharp
 * extern func declarations moved to header
 *
 * Revision 1.12  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.11  1992/03/25  19:13:17  pgf
 * BSD portability changes
 *
 * Revision 1.10  1992/03/19  23:25:04  pgf
 * linux port, and default NOFILE to 20 (bogus)
 *
 * Revision 1.9  1991/11/18  08:33:25  pgf
 * added missing arg to strrchr
 *
 * Revision 1.8  1991/11/16  18:36:46  pgf
 * no #define for strrchr needed here
 *
 * Revision 1.7  1991/10/22  14:08:23  pgf
 * took out old ifdef BEFORE code
 *
 * Revision 1.6  1991/10/21  13:39:54  pgf
 * plugged file descriptor leak
 *
 * Revision 1.5  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.4
 * date: 1991/08/06 15:30:25;
 * took out setbuf(NULL)'s, as an experiment, on Dave's suggestion
 * 
 * revision 1.3
 * date: 1991/06/25 14:22:33;
 * defensinve checks against fdopen failure
 * 
 * revision 1.2
 * date: 1991/04/04 09:38:39;
 * support for bidirectional pipes
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:49;
 * initial vile RCS revision
 */

#include <stdio.h>
#include "estruct.h"
#include "edef.h"

#if UNIX

#include <signal.h>
#include <errno.h>
#include <sys/param.h>

#define R 0
#define W 1

static int pid;

/* define MY_EXEC to use $SHELL -- you probably don't want to do that */

FILE *
npopen (cmd, type)
char *cmd, *type;
{
	FILE *fr, *fw;

	if (*type != 'r' && *type != 'w')
		return NULL;

	if (inout_popen(&fr, &fw, cmd) != TRUE)
		return NULL;

	if (*type == 'r') {
		fclose(fw);
		return fr;
	} else {
		fclose(fr);
		return fw;
	}
}

int
inout_popen(fr, fw, cmd)
FILE **fr, **fw;
char *cmd;
{
	int tries = 5;
	unsigned slp = 1;
	int rp[2];
	int wp[2];
	char *sh, *shname;
	

	if (pipe(rp))
		return FALSE;
	if (pipe(wp))
		return FALSE;
		
	/* Try & fork 5 times, backing off 1, 2, 4 .. seconds each try */
	while ((pid = fork ()) < 0) {
		if (--tries == 0)
			return FALSE;
		(void) sleep (slp);
		slp <<= 1;
	}

	if (pid) { /* parent */

		*fr = fdopen (rp[0], "r");
		if (*fr == NULL) {
			fprintf(stderr,"fdopen r failed\n");
			abort();
		}
		(void) close (rp[1]);

		*fw = fdopen (wp[1], "w");
		if (*fw == NULL) {
			fprintf(stderr,"fdopen w failed\n");
			abort();
		}
		(void) close (wp[0]);
		return TRUE;

	} else {			/* child */
		int i;

		(void)close (1);
		if (dup (rp[1]) != 1)
			exit(-1);
		(void)close (2);
		if (dup (rp[1]) != 2)
			exit(-1);
		(void)close (0);
		if (dup (wp[0]) != 0)
			exit(-1);
			
#ifndef NOFILE
# define NOFILE 20
#endif
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
	return TRUE;
}

void
npclose (fp)
FILE *fp;
{
	extern int errno;
	fflush(fp);
	fclose(fp);
	if (wait ((int *)0) < 0 && errno == EINTR) {
		(void) kill (SIGKILL, pid);
	}
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
		argv[1] = strrchr(argv[0],'/');
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
