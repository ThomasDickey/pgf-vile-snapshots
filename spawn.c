/*	Spawn:	various DOS access commands
		for MicroEMACS
*/

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"
#if BSD | USG
#include	<sys/types.h>
#include	<sys/stat.h>
#endif

#if     AMIGA
#define  NEW   1006L
#endif

#if		ST520 & MEGAMAX
#include <osbind.h>
#include <string.h>
#define LOAD_EXEC 0 	/* load and execute the program */
char	*STcmd,		/* the command filename & path  */
	*STargs,	/* command args (if any)        */
	*STenv,		/* environment                  */
	*STwork;	/* work area			*/
#endif

#if     VMS
#define EFN     0                               /* Event flag.          */

#include        <ssdef.h>                       /* Random headers.      */
#include        <stsdef.h>
#include        <descrip.h>
#include        <iodef.h>

extern  int     oldmode[3];                     /* In "termio.c"        */
extern  int     newmode[3];                     /* In "termio.c"        */
extern  short   iochan;                         /* In "termio.c"        */
#endif

#if     UNIX
#include        <signal.h>
#include        <string.h>
#endif

#if	MSDOS & (MSC | TURBO)
#include	<process.h>
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
spawncli(f, n)
{
#if     UNIX
# if     NeWS
	mlwrite("Not availible under NeWS");
	return(FALSE);
# else
        register char *cp;
        char    *getenv();
        
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
	ttclean(TRUE);
        TTputc('\n');
        if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
                system(cp);
        else
                system("exec /bin/sh");
        TTflush();
	ttunclean();
        sgarbf = TRUE;
        return(TRUE);
# endif /* News */
#endif /* UNIX */

#if	AMIGA
        long newcli;
        mlwrite("[Starting new CLI]");
        sgarbf = TRUE;
        Execute("NEWCLI \"CON:0/0/640/200/MicroEMACS Subprocess\"", 0L, 0L);
        return(TRUE);
#endif

#if     VMS
        movecursor(term.t_nrow, 0);             /* In last line.        */
        mlputs("[Starting DCL]\r\n");
        TTflush(); 	                     /* Ignore "ttcol".      */
        sgarbf = TRUE;
        return (sys(NULL));                     /* NULL => DCL.         */
#endif
#if     CPM
        mlwrite("Not in CP/M-86");
	return FALSE;
#endif
#if	ST520
	mlwrite("Not in TOS");
	return FALSE;
#endif
#if     MSDOS & (AZTEC | MSC | TURBO)
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
	TTkclose();
	system("command.com");
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
#endif
#if     MSDOS & LATTICE
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
	TTkclose();
        sys("\\command.com", "");               /* Run CLI.             */
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
#endif
}

#if UNIX && defined(SIGTSTP)

bktoshell()		/* suspend MicroEMACS and wait to wake up */
{
#if     NeWS
	mlwrite("Not availible under NeWS");
	return(FALSE);
#else
	int pid;

	vttidy(TRUE);
	pid = getpid();
	kill(pid,SIGTSTP);
#endif
}

rtfrmshell()
{
#if     NeWS
	mlwrite("Not available under NeWS");
	return(FALSE);
#else
	ttunclean();
	curwp->w_flag = WFHARD;  /* is this needed, with sgarbf == TRUE? */
	sgarbf = TRUE;
#if USG
	signal(SIGCONT,rtfrmshell);	/* suspend & restart */
	update(TRUE);
#endif
#endif
}
#endif /* SIGTSTP */

#if UNIX
pressreturn()
{
	int s;

        mlputs("[Press return to continue]");
        TTflush();
	/* loop for a CR, a space, or a : to do another named command */
        while ((s = kbd_key()) != '\r' && s != ' ' && s != kcod2key(abortc)) {
		extern CMDFUNC f_namedcmd;
                if (kcod2fnc(s) == &f_namedcmd) {
			tungetc(kcod2key(s));
			break;
		}
	}
}
#endif

respawn(f,n) {
	spawn(f,n,TRUE);
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done.
 */
/* the #ifdefs have been totally separated, for readability */
spawn(f, n, rerun)
{

#if  UNIX
        register int    s;
        static char oline[NLINE];	/* command line send to shell */
        char	line[NLINE];	/* command line send to shell */
	register char	*cp;
	char		line2[NLINE];
	int cb;
	char prompt[50];
	char *getenv();

	if (!rerun) {
		if (cb = anycb())
			sprintf(prompt,"Warning: %d modified buffer%s: !",
				cb, cb>1 ? "s":"");
		else
			sprintf(prompt,": !");

	        if ((s=mlreply(prompt, oline, NLINE)) != TRUE)
	                return (s);
	} else {
		if (!oline[0])
			return FALSE;
		mlwrite(": !%s",oline);
	}
	strcpy(line,oline);
        if ((cp = getenv("SHELL")) == NULL || *cp == '\0')
                cp = "/bin/sh";
	sprintf(line2, "%s -c \"%s\"", cp, line);
#if	NeWS
	system(line2);
#else
	ttclean(TRUE);
	system(line2);
        TTflush();
	ttunclean();
        sgarbf = TRUE;
	pressreturn();
#endif /* NeWS */
        return (TRUE);
#endif /* UNIX */

#if     AMIGA
        register int    s;
        static char oline[NLINE];	/* command line send to shell */
        char	line[NLINE];	/* command line send to shell */
	register char	*cp;
	char		line2[NLINE];
        long newcli;


        if ((s=mlreply("cmd: !", oline, NLINE)) != TRUE)
                return (s);
	strcpy(line,oline);
        newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
        Execute(line, 0L, newcli);
        Close(newcli);
        tgetc();     /* Pause.               */
        sgarbf = TRUE;
        return(TRUE);
#endif
#if	ST520 & MEGAMAX
        register int    s;
        static char oline[NLINE];	/* command line send to shell */
        char	line[NLINE];	/* command line send to shell */
	register char	*cp;
	char		line2[NLINE];

	int i,j,k;
	char *sptr,*tptr;

        if ((s=mlreply("cmd: !", oline, NLINE)) != TRUE)
                return(s);
	strcpy(line,oline);
	movecursor(term.t_nrow - 1, 0);
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
		STcmd = malloc(strlen(line) + 1);
		strcpy(STcmd,line);
		STargs = NULL;
	}
	else {  /* seperate out the args from the command */
		/* resist the temptation to do ptr arithmetic */
		STcmd = malloc(strlen(line) + 1);
		for(i = 0,sptr = &line[0]; sptr != tptr; sptr++,i++)
			STcmd[i] = *sptr;
		STcmd[i] = '\0';
		for(; *tptr == ' ' || *tptr == '\t'; tptr++);
		if(*tptr == '\0')
			STargs = NULL;
		else {
			STargs = malloc(strlen(tptr) + 2);
/* first byte of STargs is the length of the string */
			STargs[0] = strlen(tptr);
			STargs[1] = NULL; /* fake it for strcat */
			strcat(STargs,tptr);
		}
	}
	/*
	 * before we issue the command look for the '.', if it's not there
	 * try adding .prg, .tos and .ttp to see if they exist, if not
	 * issue the command as is
	 */
	if((tptr = index(STcmd,'.')) == NULL) {
 		STwork = malloc(strlen(STcmd) + 4);
 		strcpy(STwork,STcmd);
 		strcat(STwork,".prg");
 		tptr = index(STwork,'.');
 		if(Fsfirst(1,STwork) != 0) { /* try .tos */
 			strcpy(tptr,".tos");
 			if(Fsfirst(1,STwork) != 0) { /* try .ttp */
 				strcpy(tptr,".ttp");
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
        mlputs("\r\n\n[End]");                  /* Pause.               */
        TTgetc();			     /* Pause.               */
        sgarbf = TRUE;
        return (TRUE);
#endif
#if     VMS
        register int    s;
        static char oline[NLINE];	/* command line send to shell */
        char	line[NLINE];	/* command line send to shell */
	register char	*cp;
	char		line2[NLINE];


        if ((s=mlreply("cmd: !", oline, NLINE)) != TRUE)
                return (s);
	strcpy(line,oline);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        s = sys(line);                          /* Run the command.     */
        mlputs("\r\n\n[End]");                  /* Pause.               */
        TTflush();
        tgetc();
        sgarbf = TRUE;
        return (s);
#endif
#if     CPM
        mlwrite("Not in CP/M-86");
        return (FALSE);
#endif
#if     MSDOS | (ST520 & LATTICE)
        register int    s;
        static char oline[NLINE];	/* command line send to shell */
        char	line[NLINE];	/* command line send to shell */
	register char	*cp;
	char		line2[NLINE];


        if ((s=mlreply("cmd: !", oline, NLINE)) != TRUE)
                return(s);
	strcpy(line,oline);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        mlputs("\r\n\n[End]");
        	tgetc();
        }
        sgarbf = TRUE;
        return (TRUE);
#endif
}

#if UNIX
/*
 * Pipe a one line command into a window
 */
pipecmd(f, n)
{
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
        static char oline[NLINE];	/* command line send to shell */
        register int    s;
	static char bname[] = "[Output]";
	int cb;
	char prompt[50];


	/* if it doesn't start with '!', or if that's all it is */
	if (oline[0] != '!' || oline[1] == '\0') {
		oline[0] = '!';
		oline[1] = '\0';
	}

	if (cb = anycb())
		sprintf(prompt,"Warning: %d modified buffer%s. !",
			cb, cb>1 ? "s":"");
	else
		sprintf(prompt,"!");
		
	/* get the command to pipe in */
        if ((s=mlreply(prompt, &oline[1], NLINE)) != TRUE)
                return(s);

	/* first check if we are already here */
	bp = bfind(bname, OK_CREAT, 0);
	if (bp == NULL)
		return FALSE;

	/* and read the stuff in */
	if (popupbuff(bp) != TRUE || 
		swbuffer(bp) != TRUE || 
		readin(oline, FALSE, bp, TRUE) != TRUE) {
		return(FALSE);
	}
	strcpy(bp->b_bname,bname);
	strncpy(bp->b_fname, oline, NFILEN-1);
	bp->b_mode |= MDVIEW;
	return TRUE;
}

#else /* ! UNIX */

/*
 * Pipe a one line command into a window
 */
pipecmd(f, n)
{
        register int    s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
        static char oline[NLINE];	/* command line send to shell */
        char	line[NLINE];	/* command line send to shell */
	static char bname[] = "[output]";
	WINDOW *ocurwp;		/* save the current window during delete */

#if	AMIGA
	static char filnam[] = "ram:command";
        long newcli;
#else
	static char filnam[NSTRING] = "command";
#endif

#if	MSDOS
	char *tmp;
	char *getenv();
	FILE *fp;
	FILE *fopen();
#endif

#if	MSDOS
	if ((tmp = getenv("TMP")) == NULL)
		strcpy(filnam, "command");
	else {
		strcpy(filnam, tmp);
                strcat(filnam,"\\command");
        }
#endif
#if     VMS
	mlwrite("Not availible under VMS");
	return(FALSE);
#endif
#if     CPM
        mlwrite("Not availible under CP/M-86");
        return(FALSE);
#endif
	/* get the command to pipe in */
        if ((s=mlreply("cmd: <", oline, NLINE)) != TRUE)
                return(s);

	strcpy(line,oline);

	/* get rid of the command output buffer if it exists */
        if ((bp=bfind(bname, NO_CREAT, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		ocurwp = NULL;
		while (wp != NULL) {
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
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)

			return(FALSE);
	}

#if     AMIGA
        newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	strcat(line, " >");
	strcat(line, filnam);
        Execute(line, 0L, newcli);
	s = TRUE;
        Close(newcli);
        sgarbf = TRUE;
#endif
#if     MSDOS
	strcat(line," >>");
	strcat(line,filnam);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
        sgarbf = TRUE;
	if ((fp = fopen(filnam, "r")) == NULL) {
		s = FALSE;
	} else {
		fclose(fp);
		s = TRUE;
	}
#endif

	if (s != TRUE)
		return(s);

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
			return(FALSE);

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return(FALSE);

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

#if FINDERR
	strcpy(febuff,bp->b_bname);
	newfebuff = TRUE;
#endif

	/* and get rid of the temporary file */
	unlink(filnam);
	return(TRUE);
}
#endif /* UNIX */

/* run a region through an external filter, replace it with its output */
filterregion(f,n)
{
        static char oline[NLINE];	/* command line send to shell */
        char	line[NLINE];	/* command line send to shell */
	FILE *fr, *fw;
	int s;

	/* get the filter name and its args */
        if ((s=mlreply("!", oline, NLINE)) != TRUE)
                return(s);
	strcpy(line,oline);
	if ((s = inout_popen(&fr, &fw, line)) != TRUE) {
		mlwrite("Couldn't open pipe or command");
		return s;
	}

	killregion(f,n);
	if (fork()) {
		fclose(fw);
		/* backline(FALSE,1); */
		curwp->w_dotp = lback(curwp->w_dotp);
		s = ifile(NULL,TRUE,fr);
		npclose(fr);
		firstnonwhite();
		setmark();
		return s;
	} else {
		KILL *kp;		/* pointer into kill register */
		kregcirculate(FALSE);
		/* make sure there is something to put */
		if (kbs[ukb].kbufh == NULL)
			return TRUE;		/* not an error, just nothing */

		kp = kbs[ukb].kbufh;
		while (kp != NULL) {
			if (kp->d_next == NULL)
				fwrite(kp->d_chunk, 1, kbs[ukb].kused, fw);
			else
				fwrite(kp->d_chunk, 1, KBLOCK, fw);
			kp = kp->d_next;
		}
		fflush(fw);
		fclose(fw);
		exit (0);
	}
}

/*
 * filter a buffer through an external DOS program
 * this is obsolete, the filterregion code is better.
 */
filter(f, n)
{
        register int    s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
        static char oline[NLINE];	/* command line send to shell */
        char	line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */
	static char bname1[] = "fltinp";

#if	AMIGA
	static char filnam1[] = "ram:fltinp";
	static char filnam2[] = "ram:fltout";
        long newcli;
#else
	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";
#endif

#if     VMS
	mlwrite("Not availible under VMS");
	return(FALSE);
#endif
#if     CPM
        mlwrite("Not availible under CP/M-86");
        return(FALSE);
#endif
	/* get the filter name and its args */
        if ((s=mlreply("cmd: |", oline, NLINE)) != TRUE)
                return(s);
	strcpy(line,oline);

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, bname1);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1,curbp,TRUE) != TRUE) {
		mlwrite("[Cannot write filter file]");
		strcpy(bp->b_fname, tmpnam);
		return(FALSE);
	}

#if     AMIGA
        newcli = Open("CON:0/0/640/200/MicroEMACS Subprocess", NEW);
	strcat(line, " <ram:fltinp >ram:fltout");
        Execute(line,0L,newcli);
	s = TRUE;
        Close(newcli);
        sgarbf = TRUE;
#endif
#if     MSDOS
	strcat(line," <fltinp >fltout");
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
        sgarbf = TRUE;
	s = TRUE;
#endif
#if     UNIX
#if	! NeWS
        ttclean(TRUE);
#endif
	strcat(line," <fltinp >fltout");
        system(line);
#if	! NeWS
        ttunclean();
        TTflush();
        sgarbf = TRUE;
#endif
       s = TRUE;
#endif

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2,FALSE,curbp,TRUE) == FALSE)) {
		mlwrite("[Execution failed]");
		strcpy(bp->b_fname, tmpnam);
		unlink(filnam1);
		unlink(filnam2);
		return(s);
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);	/* restore name */
	bp->b_flag |= BFCHG;		/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return(TRUE);
}

#if     VMS
/*
 * Run a command. The "cmd" is a pointer to a command string, or NULL if you
 * want to run a copy of DCL in the subjob (this is how the standard routine
 * LIB$SPAWN works. You have to do wierd stuff with the terminal on the way in
 * and the way out, because DCL does not want the channel to be in raw mode.
 */
sys(cmd)
register char   *cmd;
{
        struct  dsc$descriptor  cdsc;
        struct  dsc$descriptor  *cdscp;
        long    status;
        long    substatus;
        long    iosb[2];

        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                return (FALSE);
        cdscp = NULL;                           /* Assume DCL.          */
        if (cmd != NULL) {                      /* Build descriptor.    */
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
        if ((substatus&STS$M_SUCCESS) == 0)     /* Command failed.      */
                return (FALSE);
        return (TRUE);
}
#endif

#if	~AZTEC & ~MSC & ~TURBO & MSDOS

/*
 * This routine, once again by Bob McNamara, is a C translation of the "system"
 * routine in the MWC-86 run time library. It differs from the "system" routine
 * in that it does not unconditionally append the string ".exe" to the end of
 * the command name. We needed to do this because we want to be able to spawn
 * off "command.com". We really do not understand what it does, but if you don't
 * do it exactly "malloc" starts doing very very strange things.
 */
sys(cmd, tail)
char    *cmd;
char    *tail;
{
#if MWC86
        register unsigned n;
        extern   char     *__end;

        n = __end + 15;
        n >>= 4;
        n = ((n + dsreg() + 16) & 0xFFF0) + 16;
        return(execall(cmd, tail, n));
#endif

#if LATTICE
        return(forklp(cmd, tail, (char *)NULL));
#endif
}
#endif

#if	MSDOS & LATTICE
/*	System: a modified version of lattice's system() function
		that detects the proper switchar and uses it
		written by Dana Hogget				*/

system(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *getenv();
	static char *swchar = "/C";	/*  Execution switch  */
	union REGS inregs;	/*  parameters for dos call  */
	union REGS outregs;	/*  Return results from dos call  */
	char *shell;		/*  Name of system command processor  */
	char *p;		/*  Temporary pointer  */
	int ferr;		/*  Error condition if any  */

	/*  get name of system shell  */
	if ((shell = getenv("COMSPEC")) == NULL) {
		return (-1);		/*  No shell located  */
	}

	p = cmd;
	while (isspace(*p)) {		/*  find out if null command */
		p++;
	}

	/**  If the command line is not empty, bring up the shell  **/
	/**  and execute the command.  Otherwise, bring up the     **/
	/**  shell in interactive mode.   **/

	if (p && *p) {
		/**  detect current switch character and us it  **/
		inregs.h.ah = 0x37;	/*  get setting data  */
		inregs.h.al = 0x00;	/*  get switch character  */
		intdos(&inregs, &outregs);
		*swchar = outregs.h.dl;
		ferr = forkl(shell, "command", swchar, cmd, (char *)NULL);
	} else {
		ferr = forkl(shell, "command", (char *)NULL);
	}

	return (ferr ? ferr : wait());
}
#endif
