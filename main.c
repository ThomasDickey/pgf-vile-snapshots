/*
 *	This used to be MicroEMACS 3.9
 * 			written by Dave G. Conroy.
 *			substatially modified by Daniel M. Lawrence
 *
 *	(C)opyright 1987 by Daniel M. Lawrence
 *	MicroEMACS 3.9 can be copied and distributed freely for any
 *	non-commercial purposes. MicroEMACS 3.9 can only be incorporated
 *	into commercial software with the permission of the current author.
 *
 *	Turned into "VI Like Emacs", a.k.a. vile, by Paul Fox
 *
 * This file contains the main driving routine, and some keyboard processing
 * code, for the screen editor.
 *
 *
 * $Log: main.c,v $
 * Revision 1.26  1991/08/13 02:50:13  pgf
 * initialize showmatch b_val
 *
 * Revision 1.25  1991/08/12  11:22:52  pgf
 * added 'vi +/searchstring file.c' invocation
 *
 * Revision 1.24  1991/08/12  10:23:43  pgf
 * esc() no longer kills keyboard recording
 *
 * Revision 1.23  1991/08/08  23:28:49  pgf
 * moved init of VAL_FILL to after vtinit, since it depends on term.t_ncol
 *
 * Revision 1.22  1991/08/08  13:19:58  pgf
 * removed ifdef BEFORE
 *
 * Revision 1.21  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.20
 * date: 1991/08/06 15:55:24;
 * fixed dos mode on empty buffer
 * 
 * revision 1.19
 * date: 1991/08/06 15:23:42;
 *  global/local values
 * 
 * revision 1.18
 * date: 1991/07/23 11:12:19;
 * don't reset lastdot if absolute motion is on behalf of an operator
 * 
 * revision 1.17
 * date: 1991/07/19 17:16:03;
 * ignore SIG_QUIT unless DEBUG
 * 
 * revision 1.16
 * date: 1991/06/28 10:53:42;
 * undo last change -- was breaking some ops
 * 
 * revision 1.15
 * date: 1991/06/27 19:45:16;
 * moved back from eol and eob to execute()
 * 
 * revision 1.14
 * date: 1991/06/26 09:38:15;
 * removed an ifdef BEFORE
 * 
 * revision 1.13
 * date: 1991/06/25 19:52:59;
 * massive data structure restructure
 * 
 * revision 1.12
 * date: 1991/06/16 17:36:17;
 * support for local vs. global fillcol value
 * 
 * revision 1.11
 * date: 1991/06/07 22:00:18;
 * changed header
 * 
 * revision 1.10
 * date: 1991/06/07 13:22:23;
 * don't move "last dot" mark if ABS command doesn't change dot
 * 
 * revision 1.9
 * date: 1991/06/03 17:34:55;
 * switch from "meta" etc. to "ctla" etc.
 * 
 * revision 1.8
 * date: 1991/06/03 10:25:16;
 * command loop is now a separate routine, and
 * if (doingopcmd) stuff is now in operators()
 * 
 * revision 1.7
 * date: 1991/05/31 12:54:25;
 * put _wild chartypes back in -- dropped by mistake
 * 
 * revision 1.6
 * date: 1991/05/31 11:12:19;
 * changed args to execute(), and
 * added linespec character class, and
 * added unimplemented ex functions
 * 
 * revision 1.5
 * date: 1991/04/22 09:00:12;
 * added iswild type to chartypes
 * 
 * revision 1.4
 * date: 1991/04/04 09:37:48;
 * new arg. to unqname
 * 
 * revision 1.3
 * date: 1991/02/19 17:27:03;
 * set up the reverse pattern if -s is given
 * 
 * revision 1.2
 * date: 1990/10/03 16:00:57;
 * make backspace work for everyone
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:35;
 * initial vile RCS revision
 */

#include        <stdio.h>

/* for MSDOS, increase the default stack space */

#if	MSDOS & LATTICE
unsigned _stack = 32767;
#endif

#if	ATARI & LATTICE & 0
int _mneed = 256000;		/* reset memory pool size */
#endif

#if	MSDOS & AZTEC
int _STKSIZ = 32767/16;		/* stack size in paragraphs */
int _STKRED = 1024;		/* stack checking limit */
int _HEAPSIZ = 4096/16;		/* (in paragraphs) */
int _STKLOW = 0;		/* default is stack above heap (small only) */
#endif

#if	MSDOS & TURBO
unsigned _stklen = 32768;
#endif

/* make global definitions not external */
#define	maindef

#include        "estruct.h"	/* global structures and defines */

#if UNIX
#include	<signal.h>
#endif

#include	"nefunc.h"	/* function declarations */
#include	"nebind.h"	/* default key bindings */
#include	"nename.h"	/* name table */
#include	"edef.h"	/* global definitions */


#if     VMS
#include        <ssdef.h>
#define GOOD    (SS$_NORMAL)
#endif

#ifndef GOOD
#define GOOD    0
#endif

main(argc, argv)
char    *argv[];
{
        int    c;		/* command character */
        int    f;		/* default flag */
        int    n;		/* numeric repeat count */
	int	s;
	register BUFFER *bp;		/* temp buffer pointer */
	register int	gotafile;	/* filename arg present? */
	register int	carg;		/* current arg to scan */
	register int	ranstartup;	/* startup executed flag */
	BUFFER *firstbp = NULL;		/* ptr to first buffer in cmd line */
        int gotoflag;                   /* do we need to goto a line at start? */
        int helpflag;                   /* do we need to goto a line at start? */
        int gline;                      /* if so, what line? */
        int searchflag;                 /* Do we need to search at start? */
#if TAGS
        int tagflag, didtag;                    /* look up a tag to start? */
	char *tname;
#endif
        char bname[NBUFN];		/* buffer name of file to read */
	char *msg;
#if	CRYPT
	/* int cryptflag;			/* encrypting on the way in? */
	char ekey[NPAT];		/* startup encryption key */
#endif
	char *strncpy();
#if UNIX
	extern int catchintr();
	extern int imdying();
	extern int sizesignal();
#endif
	extern char *pathname[];	/* startup file path/name array */

	charinit();		/* character types -- we need these pretty
					early  */
	global_val_init();	/* global buffer values */

	gotoflag = FALSE;	/* set to off to begin with */
	helpflag = FALSE;	/* set to off to begin with */
	searchflag = FALSE;	/* set to off to begin with */
#if TAGS
	tagflag = FALSE;	/* set to off to begin with */
#endif
	gotafile = FALSE;	/* no file to edit yet */
	ranstartup = FALSE;	/* startup file not executed yet */
#if	CRYPT
	cryptflag = FALSE;	/* no encryption by default */
#endif

	/* Parse the command line */
	for (carg = 1; carg < argc; ++carg) {

		/* Process Switches */
		if (argv[carg][0] == '-') {
			switch (argv[carg][1]) {
#if	NeWS
			case 'l':	/* -l for screen lines */
			case 'L':
				term.t_nrow = atoi(&argv[carg][2]);
				break;
#endif
			case 'e':	/* -e for Edit file */
			case 'E':
				set_global_b_val(MDVIEW,FALSE);
				break;
			case 'g':	/* -g for initial goto */
			case 'G':
				gotoflag = TRUE;
				if (argv[carg][2]) {
					gline = atoi(&argv[carg][2]);
				} else {
					if (++carg < argc)
						gline = atoi(&argv[carg][0]);
					else
						goto usage;
				}
				break;
			case 'h':	/* -h for initial help */
			case 'H':
				helpflag = TRUE;
				break;
#if	CRYPT
			case 'k':	/* -k<key> for code key */
			case 'K':
				cryptflag = TRUE;
				if (argv[carg][2]) {
					strcpy(ekey, &argv[carg][2]);
				} else {
					if (++carg < argc)
						strcpy(ekey, &argv[carg][0]);
					else
						goto usage;
				}
				set_global_b_val(MDCRYPT,TRUE);
				crypt((char *)NULL, 0);
				crypt(ekey, strlen(ekey));
				break;
#endif
			case 's':  /* -s for initial search string */
			case 'S':
		dosearch:
				searchflag = TRUE;
				if (argv[carg][2]) {
					strncpy(pat,&argv[carg][2],NPAT);
				} else {
					if (++carg < argc)
						strncpy(pat,&argv[carg][0],NPAT);
					else
						goto usage;
				}
				rvstrcpy(tap, pat);
				break;
#if TAGS
			case 't':  /* -t for initial tag lookup */
			case 'T':
				tagflag = TRUE;
				if (argv[carg][2]) {
					tname = &argv[carg][2];
				} else {
					if (++carg < argc)
						tname = &argv[carg][0];
					else
						goto usage;
				}
				break;
#endif
			case 'v':	/* -v for View File */
			case 'V':
				set_global_b_val(MDVIEW,TRUE);
				break;
			default:	/* unknown switch */
			usage:
				fprintf(stderr,
			"usage: %s -flags files...\n%s%s%s%s%s%s",argv[0],
				"	-h to get help on startup\n",
			      "	-gNNN or simply +NNN to go to line NNN\n",
				"	-sstring to search for string\n",
#if TAGS
				"	-ttagname to look up a tag\n",
#else
				"",
#endif
				"	-v to view files as read-only\n",
#if CRYPT
				"	-kcryptkey for encrypted files\n"
#else
				""
#endif
				);
				exit(1);
			}

		} else if (argv[carg][0]== '+') { /* alternate form of -g */
 			if (argv[carg][1] == '/')
				goto dosearch;
			gotoflag = TRUE;
			gline = atoi(&argv[carg][1]);
		} else if (argv[carg][0]== '@') {
			/* Process Startup macroes */
			if (startup(&argv[carg][1]) == TRUE)
				ranstartup = TRUE; /* don't execute .vilerc */
		} else {

			/* Process an input file */

			/* set up a buffer for this file */
	                makename(bname, argv[carg]);
			unqname(bname,FALSE);

			bp = bfind(bname, OK_CREAT, 0);
			make_current(bp); /* pull it to the front */
			strcpy(bp->b_fname, argv[carg]);
			if (!gotafile) {
				firstbp = bp;
				gotafile = TRUE;
			}

		}
	}

	/* initialize the editor */
#if UNIX
	signal(SIGHUP,imdying);
	signal(SIGINT,catchintr);
	signal(SIGBUS,imdying);
	signal(SIGSEGV,imdying);
	signal(SIGSYS,imdying);
	signal(SIGTERM,imdying);
#if DEBUG
	signal(SIGQUIT,imdying);
#else
	signal(SIGQUIT,SIG_IGN);
#endif
	signal(SIGPIPE,SIG_IGN);
#ifdef SIGWINCH
	signal(SIGWINCH,sizesignal);
#endif
#endif
	vtinit();		/* Display */
	winit();		/* windows */
	varinit();		/* user variables */
	
	/* this comes out to 70 on an 80 (or greater) column display */
	{    	register int fill;
		fill = (7 * term.t_ncol) / 8;  /* must be done after vtinit() */
		if (fill > 70) fill = 70;
		set_global_b_val(VAL_FILL, fill);
	}

	/* we made some calls to makecurrent() above, to shuffle the
		list order.  this set curbp, which isn't actually kosher */
	curbp = NULL;

	/* if invoked with no other startup files,
	   run the system startup file here */
	if (!ranstartup) {

		/* if .vilerc is one of the input files....don't clobber it */
		if (gotafile && strcmp(pathname[0], firstbp->b_bname) == 0) {
			c = firstbp->b_bname[0];
			firstbp->b_bname[0] = '[';
			startup(pathname[0]);
			firstbp->b_bname[0] = c;
		} else {
			startup(pathname[0]);
		}
		ranstartup = TRUE;
	}

	/* if there are any files to read, read the first one! */
	if (gotafile) {
		nextbuffer(FALSE,0);
	}
#if TAGS
	else if (tagflag) {
	     	tags(tname);
	     	didtag = TRUE;
	}
#endif
	if (!curbp) {
		bp = bfind("[unnamed]", OK_CREAT, 0);
		bp->b_active = TRUE;
#if DOSFILES
		make_local_b_val(bp,MDDOS);
		set_b_val(bp, MDDOS, FALSE );
#endif
		swbuffer(bp);
	}

	msg = "";
	if (helpflag) {
		if (help(TRUE,1) != TRUE) {
			msg = 
	"[Problem with help information. Type \":quit\" to exit if you wish]";
		}
	} else {
		msg = "[Use ^A-h, ^X-h, or :help to get help]";
	}

        /* Deal with startup gotos and searches */
        if (gotoflag + searchflag
#if TAGS
		 + tagflag 
#endif
		> 1) {
#if TAGS
		msg = "[Search, goto and tag are used one at a time]";
#else
		msg = "[Cannot search and goto at the same time]";
#endif
	} else if (gotoflag) {
                if (gotoline(TRUE,gline) == FALSE)
			msg = "[Invalid goto argument]";
        } else if (searchflag) {
                forwhunt(FALSE, 0);
#if TAGS
        } else if (tagflag && !didtag) {
                tags(tname);
#endif
        }

	update(FALSE);
	mlwrite(msg);


	/* process commands */
	loop();

}

loop()
{
	int s,c,f,n;
	while(1) {

		/* Vi doesn't let the cursor rest on the newline itself.  This
			takes care of that. */
		if (is_at_end_of_line(DOT) && !is_empty_line(DOT))
			backchar(TRUE,1);

		/* same goes for end of file */
		if (is_header_line(DOT,curbp) && !is_empty_buf(curbp))
			backline(TRUE,1);

		/* start recording for '.' command */
		dotcmdbegin();

		/* Fix up the screen    */
		s = update(FALSE);

		/* get the next command from the keyboard */
		c = kbd_seq();

		/* if there is something on the command line, clear it */
		if (mpresf != FALSE) {
			mlerase();
			if (s != SORTOFTRUE) /* did nothing due to typeahead */
				update(FALSE);
		}

		f = FALSE;
		n = 1;

		do_num_proc(&c,&f,&n);
		do_rept_arg_proc(&c,&f,&n);

		kregflag = 0;
		
		/* flag the first time through for some commands -- e.g. subst
			must know to not prompt for strings again, and pregion
			must only restart the p-lines buffer once for each
			command. */
		calledbefore = FALSE;

		/* and execute the command */
		execute(kcod2fnc(c), f, n);
		
		if (bheadp != curbp)
			mlwrite("BUG: main: bheadp != curbp, bhead name is %s",
					 bheadp->b_bname);

		/* stop recording for '.' command */
		dotcmdfinish();
	}
}

global_val_init()
{
	register int i;
	/* set up so the global value pointers point at the global
		values.  we never actually use the global pointers
		directly, but but when buffers get a copy of the
		global_b_values structure, the pointers will still point
		back at the global values, which is what we want */
	for (i = 0; i <= MAX_B_VALUES; i++)
		global_b_values.vp[i] = &(global_b_values.v[i]);

	set_global_b_val(MDWRAP,FALSE);	/* wrap */
	set_global_b_val(MDCMOD,FALSE);	/* C mode */
	set_global_b_val(MDSWRAP,TRUE);	/* scan wrap */
	set_global_b_val(MDEXACT,TRUE);	/* exact matches */
	set_global_b_val(MDVIEW,FALSE);	/* view-only */
	set_global_b_val(MDMAGIC,TRUE);	/* magic searches */
	set_global_b_val(MDCRYPT,FALSE);	/* crypt */
	set_global_b_val(MDASAVE,FALSE);	/* auto-save */
	set_global_b_val(MDLIST,FALSE);	/* list-mode */
	set_global_b_val(MDDOS,FALSE);	/* dos mode */
	set_global_b_val(MDAIND,FALSE);	/* auto-indent */
	set_global_b_val(MDSHOWMAT,FALSE);	/* show-match */
	set_global_b_val(VAL_TAB, 8);	/* tab stop */
	set_global_b_val(VAL_C_TAB, 8);	/* C file tab stop */
	set_global_b_val(VAL_ASAVE, 256);	/* autosave count */
	set_global_b_val_ptr(VAL_CWD, NULL);	/* current directory */
	set_global_b_val_ptr(VAL_CSUFFIXES, "chsCHS"); /* suffixes for C mode */

}

#if BSD | USG | V7
catchintr()
{
	interrupted = TRUE;
#if USG
	signal(SIGINT,catchintr);
#endif
}
#endif

/* do number processing if needed */
do_num_proc(cp,fp,np)
int *cp, *fp, *np;
{
	register int c, f, n;
        register int    mflag;

	c = *cp;
	f = *fp;
	n = *np;

	if (iscntrl(c) || (c & (CTLA|CTLX|SPEC)))
		return;
	if ( isdigit(c) && c != '0' ) {
		f = TRUE;		/* there is a # arg */
		n = 0;			/* start with a zero default */
		mflag = 1;		/* current minus flag */
		while (isdigit(c) || (c == '-')) {
			if (c == '-') {
				/* already hit a minus or digit? */
				if ((mflag == -1) || (n != 0))
					break;
				mflag = -1;
			} else {
				n = n * 10 + (c - '0');
			}
			if ((n == 0) && (mflag == -1))	/* lonely - */
				mlwrite("arg:");
			else
				mlwrite("arg: %d",n * mflag);

			c = kbd_seq();	/* get the next key */
		}
		n = n * mflag;	/* figure in the sign */
	}
	*cp = c;
	*fp = f;
	*np = n;
}

/* do ^U-style repeat argument processing -- vile binds this to 'K' */
do_rept_arg_proc(cp,fp,np)
int *cp, *fp, *np;
{
	register int c, f, n;
        register int    mflag;
	c = *cp;
	f = *fp;
	n = *np;

        if (c != reptc) 
		return;

        f = TRUE;
        n = 4;                          /* with argument of 4 */
        mflag = 0;                      /* that can be discarded. */
        mlwrite("arg: 4");
        while (isdigit(c=kbd_seq()) || c==reptc || c=='-'){
                if (c == reptc)
			if ((n > 0) == ((n*4) > 0))
                                n = n*4;
                        else
                        	n = 1;
                /*
                 * If dash, and start of argument string, set arg.
                 * to -1.  Otherwise, insert it.
                 */
                else if (c == '-') {
                        if (mflag)
                                break;
                        n = 0;
                        mflag = -1;
                }
                /*
                 * If first digit entered, replace previous argument
                 * with digit and set sign.  Otherwise, append to arg.
                 */
                else {
                        if (!mflag) {
                                n = 0;
                                mflag = 1;
                        }
                        n = 10*n + c - '0';
                }
                mlwrite("arg: %d", (mflag >=0) ? n : (n ? -n : -1));
        }
        /*
         * Make arguments preceded by a minus sign negative and change
         * the special argument "^U -" to an effective "^U -1".
         */
        if (mflag == -1) {
                if (n == 0)
                        n++;
                n = -n;
        }

	*cp = c;
	*fp = f;
	*np = n;
}


/*
 * This is the general command execution routine. It takes care of checking
 * flags, globals, etc, to be sure we're not doing something dumb.
 * Return the status of command.
 */

int
execute(execfunc, f, n)
CMDFUNC *execfunc;		/* ptr to function to execute */
{
        register int status, flags;
	MARK odot;

	if (execfunc == NULL) {
		TTbeep();
#if REBIND
		mlwrite("[Key not bound]");	/* complain		*/
#else
		mlwrite("[Not a command]");	/* complain		*/
#endif
		return (FALSE);
	}

	flags = execfunc->c_flags;

	/* commands following operators can't be redone or undone */
	if ( !doingopcmd) {
		/* don't record non-redoable cmds */
		if ((flags & REDO) == 0)
			dotcmdstop();
		if (flags & UNDO) {
			/* undoable command can't be permitted when read-only */
			if (b_val(curbp,MDVIEW))
				return(rdonly());
			mayneedundo();
		}
	}

	/* if motion is absolute, remember where we are */
	if (flags & ABS) {
		odot = DOT;
	}

	status = (execfunc->c_func)(f, n, NULL, NULL);
	if ((flags & GOAL) == 0) { /* goal should not be retained */
		curgoal = -1;
	}
	if (flags & UNDO)	/* verify malloc arena after line changers */
		vverify("main");

	/* if motion was absolute, and it wasn't just on behalf of an
		operator, and we moved, update the "last dot" mark */
	if ((flags & ABS) && !doingopcmd && !sameline(DOT, odot)) {
		curwp->w_lastdot = odot;
	}


	return (status);
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit, otherwise simply exit.
 */
quickexit(f, n)
{
	register BUFFER *bp;	/* scanning pointer to buffers */
        register BUFFER *oldcb; /* original current buffer */
	register int status;
	int thiscmd;
	int cnt;

        oldcb = curbp;                          /* save in case we fail */

	thiscmd = lastcmd;
	if (cnt = anycb()) {
		mlwrite("Will write %d buffer%c  %s ",
			cnt, cnt > 1 ? 's':'.',
			clexec ? "" : "Repeat command to continue.");
		if (!clexec && !isnamedcmd) {
			if (thiscmd != kbd_seq())
				return(FALSE);
		}

		bp = bheadp;
		while (bp != NULL) {
			if ((bp->b_flag&BFCHG) != 0 &&
			    (bp->b_flag&BFINVS) == 0) {
			    	make_current(bp);
				mlwrite("[Saving %s]",bp->b_fname);
				mlwrite("\n");
				if ((status = filesave(f, n)) != TRUE) {
				    	make_current(oldcb);
					return(status);
				}
				mlwrite("\n");
			}
			bp = bp->b_bufp;	/* on to the next buffer */
		}
	} else if (!clexec && !isnamedcmd) {
		if (thiscmd != kbd_seq())
			return(FALSE);
	}
        quithard(f, n);                             /* conditionally quit   */
	return(TRUE);
}

/* Force quit by giving argument */
quithard(f,n)
{
    quit(TRUE,1);
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out.
 */
quit(f, n)
{
	int cnt;
	
        if (f != FALSE || (cnt = anycb()) == 0) {
                vttidy(TRUE);
#if	FILOCK
		if (lockrel() != TRUE) {
			exit(1);
		}
#endif
                exit(GOOD);
        }
	if (cnt == 1)
		mlwrite(
		"There is an unwritten modified buffer.  Write it, or use :q!");
	else
		mlwrite(
		"There are %d unwritten modified buffers.  Write them, or use :q!",
			cnt);
        return (FALSE);
}

writequit(f,n)
{
	int s;
	s = filesave(FALSE,n);
	if (s != TRUE)
		return s;
	return(quit(FALSE,n));
}

/*
 * Begin recording a dot command macro.
 * Set up variables and return.
 */
dotcmdbegin()
{
	switch (dotcmdmode) {
        case TMPSTOP:
	case PLAY:
                return(FALSE);
	}
	tmpcmdptr = &tmpcmdm[0];
	tmpcmdend = tmpcmdptr;
        dotcmdmode = RECORD;
        return (TRUE);
}

/*
 * End dot command
 */
dotcmdfinish()
{

	switch (dotcmdmode) {
        case STOP:
	case PLAY:
	case TMPSTOP:
                return(FALSE);

	case RECORD:
		;
	}
	tmpcmdptr = &tmpcmdm[0];
	dotcmdptr = &dotcmdm[0];
	while (tmpcmdptr < tmpcmdend)
		*dotcmdptr++ = *tmpcmdptr++;
	dotcmdend = dotcmdptr;
	dotcmdptr = &dotcmdm[0];
	tmpcmdptr = tmpcmdptr = &tmpcmdm[0];
	/* leave us in RECORD mode */
        return(TRUE);
}

dotcmdstop()
{
	if (dotcmdmode == RECORD) {
		dotcmdmode = STOP;
	}
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
dotcmdplay(f, n)
{
        if (n <= 0)
                return (TRUE);
	dotcmdrep = n;		/* remember how many times to execute */
	dotcmdmode = PLAY;		/* put us in play mode */
	dotcmdptr = &dotcmdm[0];	/*    at the beginning */

	return(TRUE);
}
/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
ctlxlp(f, n)
{
        if (kbdmode != STOP) {
                mlwrite("%%Macro already active");
                return(FALSE);
        }
        mlwrite("[Start macro]");
	kbdptr = &kbdm[0];
	kbdend = kbdptr;
        kbdmode = RECORD;
        return (TRUE);
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
ctlxrp(f, n)
{
        if (kbdmode == STOP) {
                mlwrite("%%Macro not active");
                return(FALSE);
        }
	if (kbdmode == RECORD) {
	        mlwrite("[End macro]");
	        kbdmode = STOP;
	}
        return(TRUE);
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
ctlxe(f, n)
{
        if (kbdmode != STOP) {
                mlwrite("%%Macro already active");
                return(FALSE);
        }
        if (n <= 0)
                return (TRUE);
	kbdrep = n;		/* remember how many times to execute */
	kbdmode = PLAY;		/* start us in play mode */
	kbdptr = &kbdm[0];	/*    at the beginning */
	return(TRUE);
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
esc(f, n)
{
        TTbeep();
	dotcmdmode = STOP;
	fulllineregions = FALSE;
	doingopcmd = FALSE;
	opcmd = 0;
	mlwrite("[Aborted]");
        return(ABORT);
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode				*/

rdonly()
{
	TTbeep();
	mlwrite("[No changes are allowed while in \"view\" mode]");
	return FALSE;
}

showversion(f,n)
{
	mlwrite(version);
	return TRUE;
}

unimpl()
{
	TTbeep();
	mlwrite("[Sorry, that vi command is unimplemented in vile ]");
	return FALSE;
}

opercopy() { return unimpl(); }
opermove() { return unimpl(); }
opertransf() { return unimpl(); }

operglobals() { return unimpl(); }
opervglobals() { return unimpl(); }

map() { return unimpl(); }
unmap() { return unimpl(); }

source() { return unimpl(); }

subst_again() { return unimpl(); }

visual() { return unimpl(); }
ex() { return unimpl(); }

nullproc()	/* user function that does (almost) NOTHING */
{
	return TRUE;
}

cntl_af()	/* dummy function for binding to control-a prefix */
{
}

cntl_xf()	/* dummy function for binding to control-x prefix */
{
}

unarg()	/* dummy function for binding to universal-argument */
{
}

/* initialize our version of the "chartypes" stuff normally in ctypes.h */
charinit()
{
	register int c;

	/* legal in pathnames */
	_chartypes_['.'] = 
		_chartypes_['_'] = 
		_chartypes_['-'] =
		_chartypes_['*'] = 
		_chartypes_['/'] = _path;

	/* legal in "identifiers" */
	_chartypes_['_'] |= _ident;

	/* whitespace */
	_chartypes_[' '] =
		_chartypes_['\t'] = 
		_chartypes_['\r'] =
		_chartypes_['\n'] = 
		_chartypes_['\f'] = _space;

	/* control characters */
	for (c = 0; c < ' '; c++)
		_chartypes_[c] |= _cntrl;
	_chartypes_[127] |= _cntrl;

	/* lowercase */
	for (c = 'a'; c <= 'z'; c++)
		_chartypes_[c] |= _lower|_path|_ident;

	/* uppercase */
	for (c = 'A'; c <= 'Z'; c++)
		_chartypes_[c] |= _upper|_path|_ident;

	/* digits */
	for (c = '0'; c <= '9'; c++)
		_chartypes_[c] |= _digit|_path|_ident|_linespec;

	/* punctuation */
	for (c = '!'; c <= '/'; c++)
		_chartypes_[c] |= _punct;
	for (c = ':'; c <= '@'; c++)
		_chartypes_[c] |= _punct;
	for (c = '['; c <= '`'; c++)
		_chartypes_[c] |= _punct;
	for (c = '{'; c <= '~'; c++)
		_chartypes_[c] |= _punct;

	/* printable */
	for (c = ' '; c <= '~'; c++)
		_chartypes_[c] |= _print;

	/* backspacers: ^H, rubout, and the user's backspace char */
	/* we'll add the user's char later */
	_chartypes_['\b'] |= _bspace;
	_chartypes_[127] |= _bspace;

	/* wildcard chars for most shells */
	_chartypes_['*'] |= _wild;
	_chartypes_['?'] |= _wild;
	_chartypes_['~'] |= _wild;
	_chartypes_['['] |= _wild;
	_chartypes_[']'] |= _wild;
	_chartypes_['$'] |= _wild;
	_chartypes_['{'] |= _wild;
	_chartypes_['}'] |= _wild;

	/* ex mode line specifiers */
	_chartypes_[','] |= _linespec;
	_chartypes_['%'] |= _linespec;
	_chartypes_['-'] |= _linespec;
	_chartypes_['+'] |= _linespec;
	_chartypes_['.'] |= _linespec;
	_chartypes_['$'] |= _linespec;
	_chartypes_['\''] |= _linespec;

}


/*****		Compiler specific Library functions	****/

#if	MWC86 & MSDOS
movmem(source, dest, size)
char *source;	/* mem location to move memory from */
char *dest;	/* memory location to move text to */
int size;	/* number of bytes to move */
{
	register int i;

	for (i=0; i < size; i++)
		*dest++ = *source++;
}
#endif

#if	(AZTEC | MSC | TURBO | LATTICE) & MSDOS
/*	strncpy:	copy a string...with length restrictions
			ALWAYS null terminate
Hmmmm...
I don't know much about DOS, but I do know that strncpy shouldn't ALWAYS
	null terminate.  -pgf
*/

char *strncpy(dst, src, maxlen)
char *dst;	/* destination of copied string */
char *src;	/* source */
int maxlen;	/* maximum length */
{
	char *dptr;	/* ptr into dst */

	dptr = dst;
	while (*src && (maxlen-- > 0))
		*dptr++ = *src++;
	*dptr = 0;
	return(dst);
}
#endif

#if	RAMSIZE & LATTICE & MSDOS
/*	These routines will allow me to track memory usage by placing
	a layer on top of the standard system malloc() and free() calls.
	with this code defined, the environment variable, $RAM, will
	report on the number of bytes allocated via malloc.

	with SHOWRAM defined, the number is also posted on the
	end of the bottom mode line and is updated whenever it is changed.
*/

#undef	malloc
#undef	free

char *allocate(nbytes)	/* allocate nbytes and track */
unsigned nbytes;	/* # of bytes to allocate */
{
	char *mp;	/* ptr returned from malloc */

	mp = malloc(nbytes);
	if (mp) {
		envram += nbytes;
#if	RAMSHOW
		dspram();
#endif
	}

	return(mp);
}

release(mp)	/* release malloced memory and track */
char *mp;	/* chunk of RAM to release */
{
	unsigned *lp;	/* ptr to the long containing the block size */

	if (mp) {
		lp = ((unsigned *)mp) - 1;

		/* update amount of ram currently malloced */
		envram -= (long)*lp - 2;
		free(mp);
#if	RAMSHOW
		dspram();
#endif
	}
}

#if	RAMSHOW
dspram()	/* display the amount of RAM currently malloced */
{
	char mbuf[20];
	char *sp;

	TTmove(term.t_nrow - 1, 70);
#if	COLOR
	TTforg(7);
	TTbacg(0);
#endif
	lsprintf(mbuf, "[%ld]", envram);
	sp = &mbuf[0];
	while (*sp)
		TTputc(*sp++);
	TTmove(term.t_nrow, 0);
	movecursor(term.t_nrow, 0);
}
#endif
#endif
