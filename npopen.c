/*	npopen:  like popen, but grabs stderr, too
 *		written by John Hutchinson, heavily modified by Paul Fox
 *
 * $Log: npopen.c,v $
 * Revision 1.15  1992/12/04 09:21:52  foxharp
 * don't close the half of a child's io that we're _not_ using
 *
 * Revision 1.14  1992/12/03  00:32:59  foxharp
 * new system_SHELL and exec_sh_c routines
 *
 * Revision 1.13  1992/05/25  21:07:48  foxharp
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

static int pipe_pid;


FILE *
npopen (cmd, type)
char *cmd, *type;
{
	FILE *ff;

	if (*type != 'r' && *type != 'w')
		return NULL;

	if (*type == 'r') {
		if (inout_popen(&ff, NULL, cmd) != TRUE)
			return NULL;
		return ff;
	} else {
		if (inout_popen(NULL, &ff, cmd) != TRUE)
			return NULL;
		return ff;
	}
}

int
inout_popen(fr, fw, cmd)
FILE **fr, **fw;
char *cmd;
{
	int rp[2];
	int wp[2];
	

	if (pipe(rp))
		return FALSE;
	if (pipe(wp))
		return FALSE;
		
	pipe_pid = softfork();
	if (pipe_pid < 0)
		return FALSE;

	if (pipe_pid) { /* parent */

		if (fr) {
			*fr = fdopen (rp[0], "r");
			if (*fr == NULL) {
				fprintf(stderr,"fdopen r failed\n");
				abort();
			}
		} else {
			(void)close(rp[0]);
		}
		(void) close (rp[1]);

		if (fw) {
			*fw = fdopen (wp[1], "w");
			if (*fw == NULL) {
				fprintf(stderr,"fdopen w failed\n");
				abort();
			}
		} else {
			(void)close(wp[1]);
		}
		(void) close (wp[0]);
		return TRUE;

	} else {			/* child */

		if (fw) {
			(void)close (0);
			if (dup (wp[0]) != 0) {
				write(2,"dup 0 failed\r\n",15);
				exit(-1);
			}
		}
		(void) close (wp[1]);
		if (fr) {
			(void)close (1);
			if (dup (rp[1]) != 1) {
				write(2,"dup 1 failed\r\n",15);
				exit(-1);
			}
			(void)close (2);
			if (dup (rp[1]) != 2) {
				write(1,"dup 2 failed\r\n",15);
				exit(-1);
			}
		} else {
			(void) close (rp[1]);
		}
		(void) close (rp[0]);
		exec_sh_c(cmd);

	}
	return TRUE;
}

void
npclose (fp)
FILE *fp;
{
	extern int errno;
	int child;
	fflush(fp);
	fclose(fp);
	while ((child = wait ((int *)0)) != pipe_pid) {
		if (child < 0 && errno == EINTR) {
			(void) kill (SIGKILL, pipe_pid);
		}
	}
}

void
exec_sh_c(cmd)
char *cmd;
{
	static char *sh, *shname;
	int i;

#ifndef NOFILE
# define NOFILE 20
#endif
	/* Make sure there are no upper inherited file descriptors */
	for (i = 3; i < NOFILE; i++)
		(void) close (i);

	if (sh == NULL) {
		if ((sh = getenv("SHELL")) == NULL || *sh == '\0') {
			sh = "/bin/sh";
			shname = "sh";
		} else {
			shname = strrchr(sh,'/');
			if (shname == NULL) {
				shname = sh;
			} else {
				shname++;
				if (*shname == '\0')
					shname = sh;
			}
		}
	}

	if (cmd)
		(void) execlp (sh, shname, "-c", cmd, 0);
	else
		(void) execlp (sh, shname, 0);
	write(2,"exec failed\r\n",14);
	exit (-1);
}

int
system_SHELL(cmd)
char *cmd;
{
	int cpid;

	cpid = softfork();
	if (cpid < 0) {
		write(2,"cannot fork\n",13);
		return cpid;
	}

	if (cpid) { /* parent */
		int child;
		while ((child = wait ((int *)0)) != cpid) {
			if (child < 0 && errno == EINTR) {
				(void) kill (SIGKILL, cpid);
			}
		}
		return 0;
	} else {
		exec_sh_c(cmd);
		write(2,"cannot exec\n",13);
		return -1;
	}

}

int
softfork()
{
	/* Try & fork 5 times, backing off 1, 2, 4 .. seconds each try */
	int fpid;
	int tries = 5;
	unsigned slp = 1;

	while ((fpid = fork ()) < 0) {
		if (--tries == 0)
			return -1;
		(void) sleep (slp);
		slp <<= 1;
	}
	return fpid;
}

#else
npopenhello() {}
#endif
