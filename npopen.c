/*	npopen:  like popen, but grabs stderr, too
 *		written by John Hutchinson, heavily modified by Paul Fox
 *
 * $Log: npopen.c,v $
 * Revision 1.34  1994/03/23 12:54:50  pgf
 * both copies of npopen() should be silent on errors
 *
 * Revision 1.33  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

#include "estruct.h"
#include "edef.h"

#if UNIX

#include <sys/param.h>

#if LINUX || APOLLO
#include <sys/wait.h>
#endif

#if OPT_EVAL
#define	user_SHELL()	gtenv("shell")
#else
#define	user_SHELL()	getenv("SHELL")
#endif

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
		if (inout_popen(&ff, (FILE **)0, cmd) != TRUE)
			return NULL;
		return ff;
	} else {
		if (inout_popen((FILE **)0, &ff, cmd) != TRUE)
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
				(void)fprintf(stderr,"fdopen r failed\n");
				abort();
			}
		} else {
			(void)close(rp[0]);
		}
		(void) close (rp[1]);

		if (fw) {
			*fw = fdopen (wp[1], "w");
			if (*fw == NULL) {
				(void)fprintf(stderr,"fdopen w failed\n");
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
				(void)write(2,"dup 0 failed\r\n",15);
				exit(-1);
			}
		}
		(void) close (wp[1]);
		if (fr) {
			(void)close (1);
			if (dup (rp[1]) != 1) {
				(void)write(2,"dup 1 failed\r\n",15);
				exit(-1);
			}
			(void)close (2);
			if (dup (rp[1]) != 2) {
				(void)write(1,"dup 2 failed\r\n",15);
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
	int child;
	(void)fflush(fp);
	(void)fclose(fp);
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
	char *sh, *shname;
	int i;

#ifndef NOFILE
# define NOFILE 20
#endif
	/* Make sure there are no upper inherited file descriptors */
	for (i = 3; i < NOFILE; i++)
		(void) close (i);

	if ((sh = user_SHELL()) == NULL || *sh == EOS) {
		sh = "/bin/sh";
		shname = "sh";
	} else {
		shname = last_slash(sh);
		if (shname == NULL) {
			shname = sh;
		} else {
			shname++;
			if (*shname == EOS)
				shname = sh;
		}
	}

	if (cmd)
		(void) execlp (sh, shname, "-c", cmd, 0);
	else
		(void) execlp (sh, shname, 0);
	(void)write(2,"exec failed\r\n",14);
	exit (-1);
}

int
system_SHELL(cmd)
char *cmd;
{
	int cpid;

	cpid = softfork();
	if (cpid < 0) {
		(void)write(2,"cannot fork\n",13);
		return cpid;
	}

	if (cpid) { /* parent */
		int child;
		beginDisplay;
		while ((child = wait ((int *)0)) != cpid) {
			if (child < 0 && errno == EINTR) {
				(void) kill (SIGKILL, cpid);
			}
		}
		endofDisplay;
		return 0;
	} else {
		exec_sh_c(cmd);
		(void)write(2,"cannot exec\n",13);
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

#endif

#if MSDOS
#include <fcntl.h>		/* defines O_RDWR */
#include <io.h>			/* defines 'dup2()', etc. */

static	int	createTemp P(( char * ));
static	void	deleteTemp P(( void ));
static	void	closePipe P(( FILE *** ));
static	FILE *	readPipe P(( char *, int, int ));

static	FILE **	myPipe;		/* current pipe-file pointer */
static	FILE **	myWrtr;		/* write-pipe pointer */
static	char *	myName[2];	/* name of temporary file for pipe */
static	char *	myCmds;		/* command to execute on read-pipe */
static	int	myRval;		/* return-value of 'system()' */

static int
createTemp (type)
char	*type;
{
	register int n = (*type == 'r');
	register int fd;

#if WATCOM
	myName[n] = tmpnam((char *)0);
#else
	myName[n] = tempnam(TMPDIR, type);
#endif
	if (myName[n] == 0)
		return -1;
	(void)close(creat(myName[n], 0666));
	if ((fd = open(myName[n], O_RDWR)) < 0) {
		deleteTemp();
		return -1;
	}
	return fd;
}

static void
deleteTemp ()
{
	register int n;

	for (n = 0; n < 2; n++) {
		if (myName[n] != 0) {
			(void)unlink(myName[n]);
			FreeAndNull(myName[n]);
		}
	}
}

static void
closePipe(pp)
FILE	***pp;
{
	if (*pp != 0) {
		if (**pp != 0) {
			(void)fclose(**pp);
			**pp = 0;
		}
		*pp = 0;
	}
}

static FILE *
readPipe(cmd, in, out)
char	*cmd;
int	in;
int	out;
{
	/* save and redirect stdin, stdout, and stderr */
	int	old0 = dup(0);
	int	old1 = dup(1);
	int	old2 = dup(2);

	if (in >= 0)	{
		dup2(in, 0);
		close(in);
	}
	dup2(out, 1);
	dup2(out, 2);

	myRval = system(cmd);

	/* restore old std... */
	dup2(old0, 0); close(old0);
	dup2(old1, 1); close(old1);
	dup2(old2, 2); close(old2);

	/* rewind command output */
	lseek(out, 0L, 0);
	return fdopen(out, "r");
}

FILE *
npopen (cmd, type)
char *cmd, *type;
{
	FILE *ff;

	if (*type == 'r') {
		if (inout_popen(&ff, (FILE **)0, cmd) == TRUE)
			return ff;
	} else if (*type == 'w') {
		if (inout_popen((FILE **)0, &ff, cmd) == TRUE)
			return ff;
	}
	return NULL;
}

/*
 * Create pipe with either write- and/or read-semantics.  Fortunately for us,
 * on MSDOS, we don't need both at the same instant.
 */
int
inout_popen(fr, fw, cmd)
FILE **fr, **fw;
char *cmd;
{
	char	*type = (fw != 0) ? "w" : "r";
	FILE	*pp;
	int	fd;

	/* Create the file that will hold the pipe's content */
	if ((fd = createTemp(type)) < 0)
		return FALSE;

	if (fw == 0) {
		*fr = pp = readPipe(cmd, -1, fd);
		myWrtr = 0;
		myPipe = 0;
		myCmds = 0;
	} else {
		*fw = pp = fdopen(fd, type);
		myPipe = fr;
		myWrtr = fw;
		myCmds = strmalloc(cmd);
	}
	return (pp != 0);
}

/*
 * If we were writing to a pipe, invoke the read-process with stdin set to the
 * temporary-file.  This is used in the filter-buffer code, which needs both
 * read- and write-pipes.
 */
void
npflush ()
{
	if (myCmds != 0) {
		if (myWrtr != 0) {
			(void)fflush(*myWrtr);
#if UNUSED
			(void)fclose(*myWrtr);
			*myWrtr = fopen(myName[0], "r");
#endif
			rewind(*myWrtr);
			*myPipe = readPipe(myCmds, fileno(*myWrtr), createTemp("r"));
		}
		FreeAndNull(myCmds);
	}
}

void
npclose (fp)
FILE *fp;
{
	closePipe(&myWrtr);
	closePipe(&myPipe);
	deleteTemp();
}

int
softfork()	/* dummy function to make filter-region work */
{
	return 0;
}
#endif
