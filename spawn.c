/*	Spawn:	various DOS access commands
 *		for MicroEMACS
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/spawn.c,v 1.98 1995/04/25 02:31:21 pgf Exp $
 *
 */

#include	"estruct.h"
#include	"edef.h"
#if SYS_UNIX || SYS_OS2 || SYS_WINNT
#include	<sys/stat.h>
#endif

extern	CMDFUNC	f_namedcmd;

static	int spawn1 P(( int, int ));

#if	SYS_AMIGA
#define  NEW	1006L
#endif

#if		SYS_ST520 & MEGAMAX
#include <osbind.h>
#define LOAD_EXEC 0	/* load and execute the program */
char	*STcmd,		/* the command filename & path  */
	*STargs,	/* command args (if any)	*/
	*STenv,		/* environment			*/
	*STwork;	/* work area			*/
#endif

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
spawncli(f, n)
int f,n;
{
/* i never thought i'd see an ifdef like this one... strange bedfellows */
#if DISP_X11 || SYS_CPM || SYS_ST520 || SYS_WIN31
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

#if	SYS_AMIGA
	long newcli;
	mlwrite("[Starting new CLI]");
	Execute("NEWCLI \"CON:0/0/640/200/MicroEMACS Subprocess\"", 0L, 0L);
	sgarbf = TRUE;
	return AfterShell();
#endif

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
#if SYS_OS2 || SYS_WINNT
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
bktoshell(f,n)		/* suspend and wait to wake up */
int f,n;
{
#if SYS_UNIX && defined(SIGTSTP) && !DISP_X11
	int forced = (f && n == SPECIAL_BANG_ARG); /* then it was :stop! */

	/* take care of autowrite */
	if (!forced && writeall(f,n,FALSE,TRUE,TRUE) != TRUE)
		return FALSE;

	vttidy(TRUE);

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
rtfrmshell(ACTUAL_SIG_ARGS)
ACTUAL_SIG_DECL
{
#if SYS_UNIX && defined(SIGTSTP)
# if ! DISP_X11
	ttunclean();
	sgarbf = TRUE;
#  if SYS_APOLLO
	(void)TTgetc();		/* have to skip a character */
	ttunclean();		/* ...so that I can finally suppress echo */
#  endif
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
pressreturn()
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
			c != ' ' && !ABORTED(c)) {
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
respawn(f,n)
int f,n;
{
	return spawn1(TRUE, !f);
}

/* ARGSUSED */
int
spawn(f,n)
int f,n;
{
	return spawn1(FALSE, !f);
}

#define COMMON_SH_PROMPT (SYS_UNIX || SYS_VMS || SYS_MSDOS || SYS_OS2 || SYS_WINNT || (SYS_ST520 & CC_LATTICE))

#if COMMON_SH_PROMPT
static	int	ShellPrompt P(( TBUFF **, char *, int ));

/*
 * Common function for prompting for shell/pipe command, and for recording the
 * last shell/pipe command so that we can support "!!" convention.
 *
 * Note that for 'pipecmd()', we must retain a leading "!".
 */
static int
ShellPrompt(holds, result, rerun)
TBUFF	**holds;
char	*result;
int	rerun;		/* TRUE/FALSE: spawn, -TRUE: pipecmd */
{
	register int	s;
	register SIZE_T	len;
	static	char	bang[] = SHPIPE_LEFT;
	BUFFER *bp;
	int	cb	= any_changed_buf(&bp),
		fix	= (rerun != -TRUE);
	char	save[NLINE],
		temp[NLINE],
		line[NLINE+1];

	if ((len = tb_length(*holds)) != 0) {
		(void)strncpy(save, tb_values(*holds), len);
		save[len] = EOS;
	} else {
		save[0] = EOS;
	}

	/* if it doesn't start with '!', or if that's all it is */
	if (!isShellOrPipe(save) || save[1] == EOS)
		(void)strcpy(save, bang);

	(void)strcpy(line, save);
	if (rerun != TRUE) {
		if (cb != 0) {
		    if (cb > 1) {
			(void)lsprintf(temp, "Warning: %d modified buffers: %s",
				cb, bang);
		    } else {
			(void)lsprintf(temp, "Warning: buffer \"%s\" is modified: %s",
				get_bname(bp), bang);
		    }
		} else {
			(void)lsprintf(temp, "%s%s", rerun == -TRUE ? "" : ": ", bang);
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
spawn1(rerun,pressret)
int rerun;
int pressret;
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
	TTflush();
	sgarbf = TRUE;
#endif /* DISP_X11 */
	return AfterShell();
#endif /* SYS_UNIX */

#if	SYS_AMIGA
	register int	s;
	static char oline[NLINE];	/* command line send to shell */
	char	line[NLINE];	/* command line send to shell */
	register char	*cp;
	char		line2[NLINE];
	long newcli;


	if ((s=mlreply("cmd: !", oline, NLINE)) != TRUE)
		return (s);
	(void)strcpy(line,oline);
	newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	Execute(line, 0L, newcli);
	Close(newcli);
	(void)keystroke();			/* Pause.		*/
	sgarbf = TRUE;
	return AfterShell();
#endif
#if	SYS_ST520 & MEGAMAX
	register int	s;
	static char oline[NLINE];	/* command line send to shell */
	char	line[NLINE];	/* command line send to shell */
	register char	*cp;
	char		line2[NLINE];

	int i,j,k;
	char *sptr,*tptr;

	if ((s=mlreply("cmd: !", oline, NLINE)) != TRUE)
		return(s);
	(void)strcpy(line,oline);
	bottomleft();
	TTclose();
	/*
	 * break the line into the command and its args
	 * be cute about it, if there is no '.' in the filename, try
	 * to find .prg, .tos or .ttp in that order
	 * in any case check to see that the file exists before we run
	 * amok
	 */
	STenv = NULL;
	if((tptr = index(&line[0],' ')) == NULL) { /* no args */
		STcmd = castalloc(char, strlen(line) + 1);
		(void)strcpy(STcmd,line);
		STargs = NULL;
	}
	else {  /* separate out the args from the command */
		/* resist the temptation to do ptr arithmetic */
		STcmd = castalloc(char, strlen(line) + 1);
		for(i = 0,sptr = &line[0]; sptr != tptr; sptr++,i++)
			STcmd[i] = *sptr;
		STcmd[i] = EOS;
		for (; isblank(*tptr); tptr++)
			;
		if (*tptr == EOS)
			STargs = NULL;
		else {
			STargs = castalloc(char, strlen(tptr) + 2);
			/* first byte of STargs is the length of the string */
			STargs[0] = strlen(tptr);
			STargs[1] = NULL; /* fake it for strcat */
			(void)strcat(STargs,tptr);
		}
	}
	/*
	 * before we issue the command look for the '.', if it's not there
	 * try adding .prg, .tos and .ttp to see if they exist, if not
	 * issue the command as is
	 */
	if((tptr = index(STcmd,'.')) == NULL) {
		STwork = castalloc(char,strlen(STcmd) + 4);
		(void)strcpy(STwork,STcmd);
		(void)strcat(STwork,".prg");
		tptr = index(STwork,'.');
		if(Fsfirst(1,STwork) != 0) { /* try .tos */
			(void)strcpy(tptr,".tos");
			if(Fsfirst(1,STwork) != 0) { /* try .ttp */
				(void)strcpy(tptr,".ttp");
				if(Fsfirst(1,STwork) != 0) /* never mind */
					*STwork = NULL;
				}
			}
	}
	if(*STwork != NULL)
		Pexec(LOAD_EXEC,STwork,STargs,STenv);
	else
		Pexec(LOAD_EXEC,STcmd,STargs,STenv);
	TTopen();
	mlforce("\r\n\n[End]");		/* Pause.		*/
	TTgetc();			/* Pause.		*/
	sgarbf = TRUE;
	return AfterShell();
#endif	/* SYS_ST520 & MEGAMAX */

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
#if	SYS_CPM
	mlforce("[Not in CP/M-86]");
	return (FALSE);
#endif
#if	SYS_WIN31
	mlforce("[Not in Windows 3.1]");
	return FALSE;
#endif
#if	SYS_MSDOS || SYS_OS2 || SYS_WINNT || (SYS_ST520 & CC_LATTICE)
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
pipecmd(f, n)
int f,n;
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


	if (((s = ((bp = bfind(OUTPUT_BufName, 0)) != NULL)) == TRUE)
	 && ((s = popupbuff(bp)) == TRUE)
	 && ((s = swbuffer(bp)) == TRUE)
	 && ((s = readin(line, FALSE, bp, TRUE)) == TRUE))
		set_rdonly(bp, line);

	return (s);
}

#else /* ! SYS_UNIX */

/*
 * Pipe a one line command into a window
 */
int
pipecmd(f, n)
{
	register int	s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
	static char oline[NLINE];	/* command line send to shell */
	char	line[NLINE];	/* command line send to shell */
	WINDOW *ocurwp;		/* save the current window during delete */

#if	SYS_AMIGA
	static char filnam[] = "ram:command";
	long newcli;
#else
	static char filnam[NSTRING] = "command";
#endif

#if	SYS_CPM
	mlforce("[Not available under CP/M-86]");
	return(FALSE);
#endif
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

#if	SYS_AMIGA
	newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	(void)strcat(line, " >");
	(void)strcat(line, filnam);
	Execute(line, 0L, newcli);
	s = TRUE;
	Close(newcli);
	sgarbf = TRUE;
#endif
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
	markWFMODE(curbp);

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
filterregion()
{
/* FIXX work on this for OS2, need inout_popen support, or named pipe? */
#if SYS_UNIX || SYS_MSDOS || (SYS_OS2 && CC_CSETPP)
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
#if SYS_UNIX
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
	DOT.l = lBACK(DOT.l);
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
filter(f, n)
int f,n;
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

#if	SYS_AMIGA
	static char filnam1[] = "ram:fltinp";
	static char filnam2[] = "ram:fltout";
	long newcli;
#else
	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";
#endif

#if	SYS_VMS
	mlforce("[Not available under VMS]");
	return(FALSE);
#endif
#if	SYS_CPM
	mlforce("[Not available under CP/M-86]");
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

#if	SYS_AMIGA
	newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	(void)strcat(line, " <ram:fltinp >ram:fltout");
	Execute(line,0L,newcli);
	s = TRUE;
	Close(newcli);
	sgarbf = TRUE;
#endif
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
 * LIB$SPAWN works. You have to do wierd stuff with the terminal on the way in
 * and the way out, because DCL does not want the channel to be in raw mode.
 */
int
sys(cmd)
register char	*cmd;
{
	struct  dsc$descriptor  cdsc;
	struct  dsc$descriptor  *cdscp;
	long	status;
	long	substatus;
	long	iosb[2];

	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
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
	status = LIB$SPAWN(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
	if (status != SS$_NORMAL)
		substatus = status;
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	if ((substatus&STS$M_SUCCESS) == 0)	/* Command failed.	*/
		return (FALSE);
	return AfterShell();
}
#endif
