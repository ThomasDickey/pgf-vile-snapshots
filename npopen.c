/*	npopen:  like popen, but grabs stderr, too
 *		written by John Hutchinson, heavily modified by Paul Fox
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/npopen.c,v 1.53 1996/03/28 12:53:33 pgf Exp $
 *
 */

#include "estruct.h"
#include "edef.h"

#if ! TEST_DOS_PIPES

#if SYS_UNIX || SYS_OS2

/*
 * For OS/2 implementations of inout_popen(), npflush(), and npclose(),
 * see "os2pipe.c".
 */

#if SYS_OS2
#include <process.h>
#endif

#if OPT_EVAL
#define	user_SHELL()	gtenv("shell")
#else
#define	user_SHELL()	getenv("SHELL")
#endif

#define R 0
#define W 1

#if SYS_UNIX
static int pipe_pid;
#endif

static void exec_sh_c(const char *cmd);

FILE *
npopen (const char *cmd, const char *type)
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
#endif /* SYS_UNIX || SYS_OS2 */

#if SYS_UNIX

int
inout_popen(FILE **fr, FILE **fw, const char *cmd)
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
npclose (FILE *fp)
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

static void
exec_sh_c(const char *cmd)
{
	const char *sh, *shname;
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


#if LATER

int shellstatus;

static int
process_exit_status(int status)
{
    if (WIFSIGNALED(status))
	return (128 + WTERMSIG(status));
    else if (!WIFSTOPPED(status))
	return (WEXITSTATUS(status));
    else
	return (EXECUTION_SUCCESS);
}

#endif /* LATER */


int
system_SHELL(const char *cmd)
{
	int cpid;

	cpid = softfork();
	if (cpid < 0) {
		(void)write(2,"cannot fork\n",13);
		return cpid;
	}

	if (cpid) { /* parent */
		int child;
		int status;
		beginDisplay;
		while ((child = wait (&status)) != cpid) {
			if (child < 0 && errno == EINTR) {
				(void) kill (SIGKILL, cpid);
			}
		}
		endofDisplay;
#if LATER
		shellstatus = process_exit_status(status);
#endif
		return 0;
	} else {
		exec_sh_c(cmd);
		(void)write(2,"cannot exec\n",13);
		return -1;
	}

}

int
softfork(void)
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

#endif  /* SYS_UNIX */
#endif 	/* TEST_DOS_PIPES */

#if SYS_MSDOS || SYS_WIN31 || SYS_WINNT || TEST_DOS_PIPES
#include <fcntl.h>		/* defines O_RDWR */
#if ! TEST_DOS_PIPES
#include <io.h>			/* defines 'dup2()', etc. */
#endif

#if SYS_WIN31
/* FIXME: is it possible to execute a DOS program from Windows? */
int	system(const char *command) { return (-1); }
#endif

static	void	deleteTemp (void);

static	FILE **	myPipe;		/* current pipe-file pointer */
static	FILE **	myWrtr;		/* write-pipe pointer */
static	char *	myName[2];	/* name of temporary file for pipe */
static	char *	myCmds;		/* command to execute on read-pipe */
static	int	myRval;		/* return-value of 'system()' */

static int
createTemp (char *type)
{
	register int n = (*type == 'r');
	register int fd;

#if CC_WATCOM || CC_TURBO
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
deleteTemp (void)
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
closePipe(FILE ***pp)
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
readPipe(const char *cmd, int in, int out)
{
	int old0, old1, old2;

	TTkclose();	/* close the keyboard in case of error */

	/* save and redirect stdin, stdout, and stderr */
	old0 = dup(0);
	old1 = dup(1);
	old2 = dup(2);

	if (in >= 0)
		dup2(in, 0);
	dup2(out, 1);
	dup2(out, 2);

	myRval = system(cmd);

	/* restore old std... */
	dup2(old0, 0); close(old0);
	dup2(old1, 1); close(old1);
	dup2(old2, 2); close(old2);

	TTkopen();	/* reopen the keyboard */

	/* rewind command output */
	lseek(out, 0L, 0);
	return fdopen(out, "r");
}

FILE *
npopen (const char *cmd, const char *type)
{
	FILE *ff = 0;

	if (*type == 'r') {
		(void)inout_popen(&ff, (FILE **)0, cmd);
	} else if (*type == 'w') {
		(void)inout_popen((FILE **)0, &ff, cmd);
	}
	return ff;
}

/*
 * Create pipe with either write- _or_ read-semantics.  Fortunately for us,
 * on SYS_MSDOS, we don't need both at the same instant.
 */
int
inout_popen(FILE **fr, FILE **fw, const char *cmd)
{
	char	*type = (fw != 0) ? "w" : "r";
	FILE	*pp = 0;
	int	fd;

	/* Create the file that will hold the pipe's content */
	if ((fd = createTemp(type)) >= 0) {
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
	}
	return (pp != 0);
}

/*
 * If we were writing to a pipe, invoke the read-process with stdin set to the
 * temporary-file.  This is used in the filter-buffer code, which needs both
 * read- and write-pipes.
 */
void
npflush (void)
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
npclose (FILE *fp)
{
	closePipe(&myWrtr);
	closePipe(&myPipe);
	deleteTemp();
}

int
softfork(void)	/* dummy function to make filter-region work */
{
	return 0;
}

#if TEST_DOS_PIPES
int
system_SHELL(const char *cmd)	/* dummy function */
{
	return 0;
}
#endif /* TEST_DOS_PIPES */
#endif /* SYS_MSDOS */
