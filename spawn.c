/*	Spawn:	various DOS access commands
 *		for MicroEMACS
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/spawn.c,v 1.113 1996/04/30 20:08:07 pgf Exp $
 *
 */

#include	"estruct.h"
#include	"edef.h"
#include	"nefunc.h"

#if SYS_VMS
#include <starlet.h>
#include <lib$routines.h>
extern	int	sys(char *);	/*FIXME: not the same as 'system()'?*/
#endif

static	int spawn1(int rerun, int pressret);



#if	SYS_VMS
#define EFN	0				/* Event flag.		*/

#include	<ssdef.h>			/* Random headers.	*/
#include	<stsdef.h>
#include	<descrip.h>
#include	<iodef.h>

extern  int	oldmode[3];			/* In "termio.c"	*/
extern  int	newmode[3];			/* In "termio.c"	*/
extern  short	iochan;				/* In "termio.c"	*/
#endif

#if CC_NEWDOSCC && !CC_DJGPP			/* Typo, was NEWSDOSCC 	*/
#include	<process.h>
#endif

/*
 * Check all modification-times after executing a shell command
 */
#ifdef	MDCHK_MODTIME
#define	AfterShell()	check_visible_modtimes()
#else
#define	AfterShell()	TRUE
#endif

/*
 * Create a subjob with a copy of the command interpreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
/* ARGSUSED */
int
spawncli(int f, int n)
{
/* i never thought i'd see an ifdef like this one... strange bedfellows */
#if DISP_X11 || SYS_WIN31
	mlforce("[This version of vile cannot spawn an interactive shell]");
	return FALSE;
#else
#if	SYS_UNIX
	bottomleft();
	ttclean(TRUE);
	TTputc('\n');
	(void)system_SHELL((char *)0);
	TTflush();
	ttunclean();
	sgarbf = TRUE;
	return AfterShell();
#endif /* SYS_UNIX */


#if	SYS_VMS
	bottomleft();
	mlforce("[Starting DCL]\r\n");
	TTflush();				/* Ignore "ttcol".	*/
	sgarbf = TRUE;
	return sys(NULL);			/* NULL => DCL.		*/
#endif
#if	SYS_MSDOS || SYS_OS2 || SYS_WINNT
	bottomleft();
	TTflush();
	TTkclose();
	{ 
		char *shell;
		if ((shell = getenv("COMSPEC")) == NULL) {
#if SYS_OS2
			shell = "cmd.exe";
#else
			shell = "command.com";
#endif
			system(shell);          /* Will search path     */
		} else {
#if SYS_OS2
/*
 *	spawn it if we know it.  Some 3rd party command processors fail
 *	if they system themselves (eg 4OS2).  CCM 24-MAR-94
 */
			spawnl( P_WAIT, shell, shell, NULL);
#else
			system(shell);
#endif
		}
	}
	TTkopen();
	sgarbf = TRUE;
	return AfterShell();
#endif
#endif
}


/* ARGSUSED */
int
bktoshell(int f, int n)		/* suspend and wait to wake up */
{
#if SYS_UNIX && defined(SIGTSTP) && !DISP_X11
	int forced = (f && n == SPECIAL_BANG_ARG); /* then it was :stop! */

	/* take care of autowrite */
	if (!forced && writeall(f,n,FALSE,TRUE,TRUE) != TRUE)
		return FALSE;

	ttclean(TRUE);

/* #define simulate_job_control_for_debug */
# ifdef simulate_job_control_for_debug
	rtfrmshell(SIGCONT);
# else
	(void)signal_pg(SIGTSTP);
# endif
	return TRUE;
#else
	mlforce("[Job control unavailable]");
	return FALSE;
#endif /* SIGTSTP */
}

/*ARGSUSED*/
SIGT
rtfrmshell(int ACTUAL_SIG_ARGS)
{
#if SYS_UNIX && defined(SIGTSTP)
# if ! DISP_X11
	ttunclean();
	sgarbf = TRUE;
#  if SYS_APOLLO
	(void)TTgetc();		/* have to skip a character */
	ttunclean();		/* ...so that I can finally suppress echo */
#  endif
	TTkopen();
	setup_handler(SIGCONT,rtfrmshell); /* suspend & restart */
	(void)update(TRUE);
# endif
#endif
#ifdef	MDCHK_MODTIME
	(void)check_visible_modtimes();
#endif
	SIGRET;
}

void
pressreturn(void)
{
	int c;
	int odiscmd;

	odiscmd = discmd;
	discmd = TRUE;
	mlprompt("[Press return to continue]");
	discmd = odiscmd;
	TTflush();
	/* loop for a CR, a space, or a : to do another named command */
	while ((c = keystroke()) != '\r' &&
			c != '\n' && 
			c != ' ' && 
			!ABORTED(c)) {
		if (kcod2fnc(c) == &f_namedcmd) {
			unkeystroke(c);
			break;
		}
	}
	TTputc('\r');
	TTputc('\n');
}

/* ARGSUSED */
int
respawn(int f, int n)
{
	return spawn1(TRUE, !f);
}

/* ARGSUSED */
int
spawn(int f, int n)
{
	return spawn1(FALSE, !f);
}

#define COMMON_SH_PROMPT (SYS_UNIX || SYS_VMS || SYS_MSDOS || SYS_OS2 || SYS_WINNT)

#if COMMON_SH_PROMPT
/*
 * Common function for prompting for shell/pipe command, and for recording the
 * last shell/pipe command so that we can support "!!" convention.
 *
 * Note that for 'pipecmd()', we must retain a leading "!".
 */
static int
ShellPrompt(
TBUFF	**holds,
char	*result,
int	rerun)		/* TRUE/FALSE: spawn, -TRUE: pipecmd */
{
	register int	s;
	register SIZE_T	len;
	static	const char bang[] = SHPIPE_LEFT;
	BUFFER *bp;
	int	cb	= any_changed_buf(&bp),
		fix	= (rerun != -TRUE);
	char	save[NLINE],
		temp[NLINE],
		line[NLINE+1];

	if ((len = tb_length(*holds)) != 0) {
		(void)strncpy(save, tb_values(*holds), len);
	}
	save[len] = EOS;

	/* if it doesn't start with '!', or if that's all it is */
	if (!isShellOrPipe(save) || save[1] == EOS)
		(void)strcpy(save, bang);

	(void)strcpy(line, save);
	if (rerun != TRUE) {
		if (cb != 0) {
		    if (cb > 1) {
			(void)lsprintf(temp, 
				"Warning: %d modified buffers: %s",
				cb, bang);
		    } else {
			(void)lsprintf(temp, 
				"Warning: buffer \"%s\" is modified: %s",
				bp->b_bname, bang);
		    }
		} else {
			(void)lsprintf(temp, "%s%s", 
				rerun == -TRUE ? "" : ": ", bang);
		}

		if ((s = mlreply_no_bs(temp, line+1, NLINE)) != TRUE)
			return s;
	}
	if (line[1] == EOS)
		return FALSE;

	*holds = tb_scopy(holds, line);
	(void)strcpy(result, line+fix);
	return TRUE;
}
#endif

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done.
 */
/* the #ifdefs have been totally separated, for readability */
static int
spawn1(int rerun, int pressret)
{
#if DISP_IBMPC
	int	closed;
#endif
#if COMMON_SH_PROMPT
	register int	s;
	char	line[NLINE];	/* command line send to shell */

	if ((s = ShellPrompt(&save_shell[0], line, rerun)) != TRUE)
		return s;
#endif	/* COMMON_SH_PROMPT */

	/* take care of autowrite */
	if (writeall(FALSE,1,FALSE,TRUE,TRUE) != TRUE)
		return FALSE;

#if SYS_UNIX
#if DISP_X11
	(void)system_SHELL(line);
#else
	ttclean(TRUE);
	(void)system_SHELL(line);
	TTflush();
	ttunclean();
	if (pressret)
		pressreturn();
	TTopen();
	TTkopen();
	TTflush();
	sgarbf = TRUE;
#endif /* DISP_X11 */
	return AfterShell();
#endif /* SYS_UNIX */

#if	SYS_VMS
	TTputc('\n');			/* Already have '\r'	*/
	TTflush();
	s = sys(line);			/* Run the command.	*/
	mlforce("\r\n\n[End]");		/* Pause.		*/
	TTflush();
	(void)keystroke();
	sgarbf = TRUE;
	return (s);
#endif
#if	SYS_WIN31
	mlforce("[Not in Windows 3.1]");
	return FALSE;
#endif
#if	SYS_MSDOS || SYS_OS2 || SYS_WINNT
	bottomleft();
	TTputc('\n');
	TTflush();
	TTkclose();
#if	DISP_IBMPC
	/* If we don't reset to 80x25, parts of the shell-output will go
	 * astray.
	 */
	closed = term.t_ncol != 80 || term.t_nrow != 25;
	if (closed)
		TTclose();
#endif
	system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (pressret) {
		pressreturn();
	}
#if	DISP_IBMPC
	/* Reopen the display _after_ the prompt, to keep the shell-output
	 * in the same type of screen as the prompt.
	 */
	if (closed)
		TTopen();
#endif
	sgarbf = TRUE;
	return AfterShell();
#endif
}

#if SYS_UNIX || SYS_MSDOS || SYS_VMS || SYS_OS2 || SYS_WINNT
/*
 * Pipe a one line command into a window
 */
/* ARGSUSED */
int
pipecmd(int f, int n)
{
	register BUFFER *bp;	/* pointer to buffer to zot */
	register int	s;
	char line[NLINE];	/* command line send to shell */

	/* get the command to pipe in */
	hst_init('!');
	s = ShellPrompt(&save_shell[!global_g_val(GMDSAMEBANGS)], line, -TRUE);
	hst_flush();

	/* prompt ok? */
	if (s != TRUE)
		return s;

	/* take care of autowrite */
	if (writeall(f,n,FALSE,FALSE,TRUE) != TRUE)
		return FALSE;


#if BEFORE
	if (((s = ((bp = bfind(OUTPUT_BufName, 0)) != NULL)) == TRUE)
	 && ((s = popupbuff(bp)) == TRUE)
	 && ((s = swbuffer(bp)) == TRUE)
	 && ((s = readin(line, FALSE, bp, TRUE)) == TRUE))
		set_rdonly(bp, line, MDVIEW);

#else
	if ((s = ((bp = bfind(OUTPUT_BufName, 0)) != NULL)) != TRUE)
		return s;
	if ((s = popupbuff(bp)) != TRUE)
		return s;
	ch_fname(bp,line);
	bp->b_active = FALSE; /* force a re-read */
	if ((s = swbuffer_lfl(bp,FALSE)) != TRUE)
		return s;
	set_rdonly(bp, line, MDVIEW);
#endif

	return (s);
}

#else /* ! SYS_UNIX */

/*
 * Pipe a one line command into a window
 */
int
pipecmd(int f, int n)
{
	register int	s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
	static char oline[NLINE];	/* command line send to shell */
	char	line[NLINE];	/* command line send to shell */
	WINDOW *ocurwp;		/* save the current window during delete */

	static char filnam[NSTRING] = "command";

	/* get the command to pipe in */
	if ((s=mlreply("cmd: <", oline, NLINE)) != TRUE)
		return(s);

	(void)strcpy(line,oline);

	/* get rid of the command output buffer if it exists */
	if ((bp=find_b_name(OUTPUT_BufName)) != NULL) {
		/* try to make sure we are off screen */
		ocurwp = NULL;
		for_each_window(wp) {
			if (wp->w_bufp == bp) {
				if (curwp != wp) {
					ocurwp = curwp;
					curwp = wp;
				}
				delwind(FALSE, 1);
				if (ocurwp != NULL)
					curwp = ocurwp;
				break;
			}
		}
		if (zotbuf(bp) != TRUE)
			return(FALSE);
	}

	if (s != TRUE)
		return(s);

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
		return(FALSE);

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return(FALSE);

	/* overwrite its buffer name for consistency */
	set_bname(curbp, OUTPUT_BufName);

	/* make this window in VIEW mode, update buffer's mode lines */
	make_local_b_val(curwp->w_bufp,MDVIEW);
	set_b_val(curwp->w_bufp,MDVIEW,TRUE);
	curwp->w_flag |= WFMODE;

#if OPT_FINDERR
	set_febuff(OUTPUT_BufName);
#endif

	/* and get rid of the temporary file */
	unlink(filnam);
	return AfterShell();
}
#endif /* SYS_UNIX */

/* run a region through an external filter, replace it with its output */
int
filterregion(void)
{
/* FIXX work on this for OS2, need inout_popen support, or named pipe? */
#if SYS_UNIX || SYS_MSDOS || (SYS_OS2 && CC_CSETPP) || SYS_WINNT
	static char oline[NLINE];	/* command line send to shell */
	char	line[NLINE];	/* command line send to shell */
	FILE *fr, *fw;
	int s;

	/* get the filter name and its args */
	if ((s=mlreply_no_bs("!", oline, NLINE)) != TRUE)
		return(s);
	(void)strcpy(line,oline);
	if ((s = inout_popen(&fr, &fw, line)) != TRUE) {
		mlforce("[Couldn't open pipe or command]");
		return s;
	}

	killregion();
	if (!softfork()) {
		KILL *kp;		/* pointer into kill register */
		kregcirculate(FALSE);
		kp = kbs[ukb].kbufh;
		while (kp != NULL) {
			fwrite((char *)kp->d_chunk, 1, (SIZE_T)KbSize(ukb,kp), fw);
			kp = kp->d_next;
		}
#if SYS_UNIX && ! TEST_DOS_PIPES
		(void)fflush(fw);
		(void)fclose(fw);
		ExitProgram (GOODEXIT);
		/* NOTREACHED */
#else
		npflush();	/* fake multi-processing */
#endif
	}
#if !(SYS_OS2 && CC_CSETPP)
	(void)fclose(fw);
#endif
	DOT.l = lback(DOT.l);
	s = ifile((char *)0,TRUE,fr);
	npclose(fr);
	(void)firstnonwhite(FALSE,1);
	(void)setmark();
	return s;
#else
	mlforce("[Region filtering not available -- try buffer filtering]");
	return FALSE;
#endif
}

/*
 * filter a buffer through an external DOS program
 * this is obsolete, the filterregion code is better.
 */
/* ARGSUSED */
int
filter(int f, int n)
{
#if !(SYS_UNIX||SYS_MSDOS || (SYS_OS2 && CC_CSETPP)) /* filterregion up above is better */
	register int	s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
	static char oline[NLINE];	/* command line send to shell */
	char	line[NLINE];	/* command line send to shell */
	char tnam[NFILEN];	/* place to store real file name */
	static char bname1[] = "fltinp";
#if	SYS_UNIX
	char	*t;
#endif

	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";

#if	SYS_VMS
	mlforce("[Not available under VMS]");
	return(FALSE);
#endif
	/* get the filter name and its args */
	if ((s=mlreply("cmd: |", oline, NLINE)) != TRUE)
		return(s);
	(void)strcpy(line,oline);

	/* setup the proper file names */
	bp = curbp;
	(void)strcpy(tnam, bp->b_fname);/* save the original name */
	ch_fname(bp, bname1);		/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1,curbp,TRUE,TRUE) != TRUE) {
		mlforce("[Cannot write filter file]");
		ch_fname(bp, tnam);
		return(FALSE);
	}

#if	SYS_MSDOS || SYS_OS2 || SYS_WINNT
	(void)strcat(line," <fltinp >fltout");
	bottomleft();
	TTkclose();
	system(line);
	TTkopen();
	sgarbf = TRUE;
	s = TRUE;
#endif
#if	SYS_UNIX
	bottomleft();
	ttclean(TRUE);
	if ((t = strchr(line, '|')) != 0) {
		char	temp[NLINE];
		(void)strcpy(temp, t);
		(void)strcat(strcpy(t, " <fltinp"), temp);
	} else {
		(void)strcat(line, " <fltinp");
	}
	(void)strcat(line," >fltout");
	system(line);
	ttunclean();
	TTflush();
	sgarbf = TRUE;
	s = TRUE;
#endif

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2,FALSE,curbp,TRUE) == FALSE)) {
		mlforce("[Execution failed]");
		ch_fname(bp, tnam);
		unlink(filnam1);
		unlink(filnam2);
		return(s);
	}

	ch_fname(bp, tnam); /* restore name */

	b_set_changed(bp);	/* flag it as changed */
	nounmodifiable(bp);	/* and it can never be "un-changed" */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return AfterShell();
#else
	mlforce("[Buffer filtering not available -- use filter operator]");
	return FALSE;
#endif
}

#if	SYS_VMS
/*
 * Run a command. The "cmd" is a pointer to a command string, or NULL if you
 * want to run a copy of DCL in the subjob (this is how the standard routine
 * lib$spawn works. You have to do wierd stuff with the terminal on the way in
 * and the way out, because DCL does not want the channel to be in raw mode.
 */
int
sys(register char *cmd)
{
	struct  dsc$descriptor  cdsc;
	struct  dsc$descriptor  *cdscp;
	long	status;
	long	substatus;
	long	iosb[2];

	status = sys$qiow(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	cdscp = NULL;				/* Assume DCL.		*/
	if (cmd != NULL) {			/* Build descriptor.	*/
		cdsc.dsc$a_pointer = cmd;
		cdsc.dsc$w_length  = strlen(cmd);
		cdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
		cdsc.dsc$b_class   = DSC$K_CLASS_S;
		cdscp = &cdsc;
	}
	status = lib$spawn(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
	if (status != SS$_NORMAL)
		substatus = status;
	status = sys$qiow(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	if ((substatus&STS$M_SUCCESS) == 0)	/* Command failed.	*/
		return (FALSE);
	return AfterShell();
}
#endif
