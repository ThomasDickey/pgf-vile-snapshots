/*
 *	MicroEMACS 3.9
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
	int viewflag;			/* are we starting in view mode? */
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

	viewflag = FALSE;	/* view mode defaults off in command line */
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
				viewflag = FALSE;
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
				break;
#endif
			case 's':  /* -s for initial search string */
			case 'S':
				searchflag = TRUE;
				if (argv[carg][2]) {
					strncpy(pat,&argv[carg][2],NPAT);
				} else {
					if (++carg < argc)
						strncpy(pat,&argv[carg][0],NPAT);
					else
						goto usage;
				}
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
				viewflag = TRUE;
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
			unqname(bname);

			bp = bfind(bname, OK_CREAT, 0);
			make_current(bp); /* pull it to the front */
			strcpy(bp->b_fname, argv[carg]);
			if (!gotafile) {
				firstbp = bp;
				gotafile = TRUE;
			}

			/* set the modes appropriatly */
			if (viewflag)
				bp->b_mode |= MDVIEW;
#if	CRYPT
			if (cryptflag) {
				bp->b_mode |= MDCRYPT;
				crypt((char *)NULL, 0);
				crypt(ekey, strlen(ekey));
				strncpy(bp->b_key, ekey, NPAT);
			}
#endif
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
	signal(SIGQUIT,imdying);
	signal(SIGPIPE,SIG_IGN);
#ifdef SIGWINCH
	signal(SIGWINCH,sizesignal);
#endif
#endif
	vtinit();		/* Display */
	winit();		/* windows */
	varinit();		/* user variables */
	
	/* we made some calls to makecurrent() above, to shuffle the
		list order.  this set curbp, which isn't actually kosher */
	curbp = NULL;

	/* this comes out to 70 on an 80 column display */
	fillcol = (7 * term.t_ncol) / 8;
	if (fillcol > 70)
		fillcol = 70;

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
		swbuffer(bp);
	}
	curbp->b_mode |= (gmode & ~(MDCMOD|MDDOS));

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


	/* setup to process commands */

	while(1) {
		/* Vi doesn't let the cursor rest on the newline itself.  This
			takes care of that in most cases.  This fix has
			not been applied to the docmd()!!! */
		if (curwp->w_doto == llength(curwp->w_dotp) &&
				llength(curwp->w_dotp) != 0)
			backchar(TRUE,1);

		/* same goes for end of file */
		if (curwp->w_dotp == curbp->b_linep &&
			lback(curwp->w_dotp) != curbp->b_linep)
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
		
		/* flag so we restart the plines buffer for each new command */
		plinesdone = FALSE;
		
		/* and execute the command */
		execute(c, f, n, NULL);
		
		if (bheadp != curbp)
			mlwrite("BUG: main: bheadp != curbp, bhead name is %s",
					 bheadp->b_bname);

		/* stop recording for '.' command */
		dotcmdfinish();
	}
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

	if (iscntrl(c) || (c & (META|SPEC)))
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

/* do ^U repeat argument processing */
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
 * This is the general command execution routine. It handles the fake binding
 * of all the keys to "self-insert".
 * Return the status of command.
 */

int
execute(c, f, n, execfunc)
CMDFUNC *execfunc;		/* ptr to function to execute */
{
        register int status, flags;

	/* if the keystroke is a bound function...do it */
	if (execfunc == NULL)
		execfunc = kcod2fnc(c);

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
			if (curbp->b_mode&MDVIEW)
				return(rdonly());
			mayneedundo();
		}
	}

	/* commands that don't move can't follow operators */
	if ( doingopcmd ) {
		if ((flags & MOTION) == 0) {
			TTbeep();
			return(ABORT);
		}
		/* motion is interpreted as affecting full lines */
		if (flags & FL)
			fulllineregions = TRUE;
	}

	if (flags & ABS) { /* motion is absolute */
		curwp->w_ldmkp = curwp->w_dotp;
		curwp->w_ldmko = curwp->w_doto;
	}

	status = (execfunc->c_func)(f, n, NULL, NULL);
	if ((flags & GOAL) == 0) { /* goal should not be retained */
		curgoal = -1;
	}
	if (flags & UNDO)	/* verify malloc arena after line changers */
		vverify("main");
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
	kbdmode = STOP;
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
	return(FALSE);
}

unimpl()
{
	TTbeep();
	mlwrite("[Sorry, that vi command is unimplemented in vile ]");
	return(FALSE);
}

nullproc()	/* user function that does (almost) NOTHING */
{
	return TRUE;
}

meta()	/* dummy function for binding to meta prefix */
{
}

cex()	/* dummy function for binding to control-x prefix */
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
		_chartypes_[c] |= _digit|_path|_ident;

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
	sprintf(mbuf, "[%lu]", envram);
	sp = &mbuf[0];
	while (*sp)
		TTputc(*sp++);
	TTmove(term.t_nrow, 0);
	movecursor(term.t_nrow, 0);
}
#endif
#endif
