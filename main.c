/*
 *	This used to be MicroEMACS 3.9
 *			written by Dave G. Conroy.
 *			substantially modified by Daniel M. Lawrence
 *
 *	Turned into "VI Like Emacs", a.k.a. vile, by Paul Fox
 *
 *	(C)opyright 1987 by Daniel M. Lawrence
 *	MicroEMACS 3.9 can be copied and distributed freely for any
 *	non-commercial purposes. MicroEMACS 3.9 can only be incorporated
 *	into commercial software with the permission of the current author.
 *
 *	The same goes for vile.  -pgf
 *
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/main.c,v 1.217 1994/12/16 22:54:21 pgf Exp $
 *
 */

/* Make global definitions not external */
#define realdef
#include	"estruct.h"	/* global structures and defines */
#include	"edef.h"	/* global definitions */
#include	"nevars.h"

#if CC_NEWDOSCC
#include <io.h>
#if CC_DJGPP
#include <dpmi.h>
#include <go32.h>
#endif
#endif

extern char *pathname[];	/* startup file path/name array */

/* for MSDOS, increase the default stack space */
#if	SYS_MSDOS && CC_TURBO
unsigned _stklen = 32768U;
#endif

static	void	do_num_proc P(( int *, int *, int * ));
static	void	do_rept_arg_proc P(( int *, int *, int * ));

/*--------------------------------------------------------------------------*/
#define	GetArgVal(param)	if (!*(++param))\
					param = argv[++carg];\
				if (param == 0)\
					goto usage

int
main(argc, argv)
int	argc;
char	*argv[];
{
	int    c;			/* command character */
	register BUFFER *bp;		/* temp buffer pointer */
	register int	carg;		/* current arg to scan */
	register int	ranstartup = FALSE;/* startup executed flag */
	int startstat = TRUE;		/* result of running startup */
	BUFFER *firstbp = NULL; 	/* ptr to first buffer in cmd line */
	int gotoflag = FALSE;		/* do we need to goto line at start? */
	int gline = FALSE;		/* if so, what line? */
	int helpflag = FALSE;		/* do we need help at start? */
	int searchflag = FALSE; 	/* Do we need to search at start? */
	char *msg;
#if OPT_TAGS
	int didtag = FALSE;		/* look up a tag to start? */
	char *tname = NULL;
#endif
#if	OPT_ENCRYPT
	char ekey[NPAT];		/* startup encryption key */
	*ekey = EOS;
#endif

#if OPT_MAP_MEMORY
	null_ptr = l_ptr((LINE *)0);
	pre_op_dot.l = null_ptr;
	nullmark.l = null_ptr;
#if !WINMARK
	Mark.l = null_ptr;	/* ...so we don't confuse with blk 0 */
#endif
#endif
	global_val_init();	/* global buffer values */
	charinit();	/* character types -- we need these pretty early  */
#if !SYS_UNIX
	expand_wild_args(&argc, &argv);
#endif
	prog_arg = argv[0];	/* this contains our only clue to exec-path */

	start_debug_log(argc,argv);

	if (strcmp(pathleaf(prog_arg), "view") == 0)
		set_global_b_val(MDVIEW,TRUE);

#if DISP_X11
	x_preparse_args(&argc, &argv);
#endif
	/* Parse the command line */
	for (carg = 1; carg < argc; ++carg) {
		register char *param = argv[carg];
#if DISP_X11 && !XTOOLKIT
		if (*param == '=') {
			x_set_geometry(param);
			continue;
		}
#endif


		/* Process Switches */
		if (*param == '-') {
			++param;
#if DISP_IBMPC || DISP_BORLAND
		    	/* if it's a digit, it's probably a screen
				resolution */
			if (isdigit(*param)) {
				current_res_name = param;
				continue;
			} else
#endif	/* DISP_IBMPC */
			switch (*param) {
#if DISP_X11 && !XTOOLKIT
			case 'd':
				if ((param = argv[++carg]) != 0)
					x_set_dpy(param);
				else
					goto usage;
				break;
			case 'r':
			case 'R':
				x_set_rv();
				break;
			case 'f':
				if (argv[++carg] != 0) {
					if (strcmp(param, "foreground") == 0
					 || strcmp(param, "fg") == 0)
						x_setforeground(argv[carg]);
					else
						x_setfont(argv[carg]);
				} else
					goto usage;
				break;
			case 'b':
				if (argv[++carg] != 0) {
					if (strcmp(param, "background") == 0
					 || strcmp(param, "bg") == 0)
						x_setbackground(argv[carg]);
				} else
					goto usage;
				break;
			case 'n':
				if (strcmp(param, "name") == 0
				 && argv[++carg] != 0)
					x_setname(argv[carg]);
				else
					goto usage;
				break;
			case 'w':
				if (strcmp(param, "wm") == 0
				 && argv[++carg] != 0)
					x_set_wm_title(argv[carg]);
				else
					goto usage;
				break;
#endif /* DISP_X11 */
			case 'e':	/* -e for Edit file */
			case 'E':
				set_global_b_val(MDVIEW,FALSE);
				break;
			case 'g':	/* -g for initial goto */
			case 'G':
				gotoflag = TRUE;
				GetArgVal(param);
				gline = atoi(param);
				break;
			case 'h':	/* -h for initial help */
			case 'H':
				helpflag = TRUE;
				break;
#if	OPT_ENCRYPT
			case 'k':	/* -k<key> for code key */
			case 'K':
				GetArgVal(param);
				(void)strcpy(ekey, param);
				(void)memset(param, '.', strlen(param));
				ue_crypt((char *)NULL, 0);
				ue_crypt(ekey, (int)strlen(ekey));
				break;
#endif
			case 's':  /* -s for initial search string */
			case 'S':
		dosearch:
				searchflag = TRUE;
				GetArgVal(param);
				(void)strncpy0(pat, param, NPAT);
				gregexp = regcomp(pat, global_b_val(MDMAGIC));
				break;
#if OPT_TAGS
			case 't':  /* -t for initial tag lookup */
			case 'T':
				GetArgVal(param);
				tname = param;
				break;
#endif
			case 'V':
				(void)printf("%s\n", getversion());
				ExitProgram(GOODEXIT);

			case 'v':	/* -v for View File */
				set_global_b_val(MDVIEW,TRUE);
				break;


			case '?':
			default:	/* unknown switch */
			usage:
				print_usage();
			}

		} else if (*param == '+') { /* alternate form of -g */
			if (*(++param) == '/')
				goto dosearch;
			gotoflag = TRUE;
			gline = atoi(param);
		} else if (*param == '@') {
			BUFFER *oldbp = curbp;
			/* Process Startup macros */
			if ((startstat = startup(++param)) == TRUE
			 || (curbp != oldbp))
				ranstartup = TRUE; /* don't execute .vilerc */
		} else if (*param != EOS) {

			/* Process an input file */
#if OPT_ENCRYPT
			cryptkey = (*ekey != EOS) ? ekey : 0;
#endif
			/* set up a buffer for this file */
			bp = make_bp(param, BFARGS, FALSE);
			make_current(bp); /* pull it to the front */
			if (firstbp == 0)
				firstbp = bp;
#if OPT_ENCRYPT
			cryptkey = 0;
#endif
		}
	}


	/* if stdin isn't a terminal, assume the user is trying to pipe a
	 * file into a buffer.
	 */
#if SYS_UNIX || SYS_VMS || SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT
	if (!isatty(fileno(stdin))) {
#if !DISP_X11
#if SYS_UNIX
# if HAS_TTYNAME
		char	*tty = ttyname(fileno(stderr));
# else
		char	*tty = "/dev/tty";
# endif
#else
  		FILE	*in;
  		int	fd;
#endif /* SYS_UNIX */
#endif /* DISP_X11 */
		BUFFER	*lastbp = firstbp;
		int	nline = 0;

		bp = bfind(STDIN_BufName, BFARGS);
		make_current(bp); /* pull it to the front */
		if (firstbp == 0)
			firstbp = bp;
		ffp = fdopen(dup(fileno(stdin)), "r");
#if !DISP_X11
#if SYS_UNIX
		/*
		 * Note: On Linux, the low-level close/dup operation
		 * doesn't work, since something hangs, apparently
		 * because substituting the file descriptor doesn't communicate
		 * properly up to the stdio routines.
		 */
		if ((freopen(tty, "r", stdin)) == 0
		 || !isatty(fileno(stdin))) {
			fputs("cannot open a terminal\n", stderr);
			ExitProgram(BADEXIT);
		}
#else
# if SYS_VMS
  		fd = open("tt:", 0);	/* or sys$command */
# else					/* e.g., DOS-based systems */
  		fd = fileno(stderr);	/* this normally cannot be redirected */
# endif
  		if ((fd >= 0)
  		 && (close(0) >= 0)
  		 && (fd = dup(fd)) == 0
  		 && (in = fdopen(fd, "r")) != 0)
  			*stdin = *in;
#endif	/* SYS_UNIX */
#endif /* DISP_X11 */

  		(void)slowreadf(bp, &nline);
  		set_rdonly(bp, bp->b_fname);
		(void)ffclose();

		if (is_empty_buf(bp)) {
			(void)zotbuf(bp);
			firstbp = lastbp;
		}
#if OPT_FINDERR
		else set_febuff(get_bname(bp));
#endif
	}
#endif

	/* we made some calls to make_current() above, to shuffle the
		list order.  this set curbp, which isn't actually kosher */
	curbp = NULL;

	/* initialize the editor */

	siginit();

	TTopen();		/* open the screen */
	TTkopen();		/* open the keyboard */
	TTrev(FALSE);

	if (vtinit() != TRUE)		/* allocate display memory */
		ExitProgram(BADEXIT);

	winit();		/* windows */

	/* this comes out to 70 on an 80 (or greater) column display */
	{	register int fill;
		fill = (7 * term.t_ncol) / 8;  /* must be done after vtinit() */
		if (fill > 70) fill = 70;
		set_global_b_val(VAL_FILL, fill);
	}

	/* pull in an unnamed buffer, if we were given none to work with */
	if (firstbp == 0) {
		bp = bfind(UNNAMED_BufName, 0);
		bp->b_active = TRUE;
#if OPT_DOSFILES
		/* an empty non-existent buffer defaults to line-style
			favored by the OS */
		make_local_b_val(bp,MDDOS);
		set_b_val(bp, MDDOS, CRLF_LINES);
#endif
		make_local_b_val(bp,MDCMOD);
		set_b_val(bp, MDCMOD, FALSE);
		swbuffer(bp);
	}

	/* if invoked with no other startup files,
	   run the system startup file here */
	if (!ranstartup) {
		char *vileinit = getenv("VILEINIT");
		if (vileinit != NULL) {
			int odiscmd;
			BUFFER *vbp, *obp;
			int oflags = 0;

			/* mark as modified, to prevent undispbuff() from
				 clobbering */
			obp = curbp;
			if (obp) {
				oflags = obp->b_flag;
				b_set_changed(obp);
			}

			if ((vbp=bfind(VILEINIT_BufName, 0))==NULL)
				ExitProgram(BADEXIT);

			/* don't want swbuffer to try to read it */
			vbp->b_active = TRUE;
			swbuffer(vbp);
			b_set_scratch(vbp);
			bprintf("%s", vileinit);
			/* if we leave it scratch, swbuffer(obp) may zot it,
				and we may zot it again */
			b_clr_flags(vbp,BFSCRTCH);
			set_rdonly(vbp, vbp->b_fname);

			/* go execute it! */
			odiscmd = discmd;
			discmd = FALSE;
			startstat = dobuf(vbp);
			discmd = odiscmd;
			if (startstat != TRUE)
				goto begin;
			if (obp) {
				swbuffer(obp);
				obp->b_flag = oflags;
			}
			/* remove the now unneeded buffer */
			b_set_scratch(vbp);  /* make sure it will go */
			(void)zotbuf(vbp);
		} else {
			char *fname;
			/* if .vilerc is one of the input files....
					don't clobber it */
#if SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT
			/* search PATH for vilerc under dos */
	 		fname = flook(pathname[0], FL_ANYWHERE|FL_READABLE);
#else
			fname = pathname[0];
#endif
			if (firstbp != 0
			 && eql_bname(firstbp, pathname[0])) {
				c = firstbp->b_bname[0];
				firstbp->b_bname[0] = SCRTCH_LEFT[0];
				startstat = startup(fname);
				firstbp->b_bname[0] = c;
			} else {
				if (fname)
					startstat = startup(fname);
				else
					startstat = TRUE;
			}
			if (startstat != TRUE)
				goto begin;
		}
	}


	/* if there are any files to read, read the first one! */
	if (firstbp != 0) {
		startstat = swbuffer(firstbp);
	}
#if OPT_TAGS
	else if (tname) {
		cmdlinetag(tname);
		didtag = TRUE;
	}
#endif
	msg = s_NULL;
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
#if OPT_TAGS
		 + (tname?1:0)
#endif
		> 1) {
#if OPT_TAGS
		msg = "[Search, goto and tag are used one at a time]";
#else
		msg = "[Cannot search and goto at the same time]";
#endif
	} else if (gotoflag) {
		if (gotoline(gline != 0, gline) == FALSE)
			msg = "[Invalid goto argument]";
	} else if (searchflag) {
		forwhunt(FALSE, 0);
#if OPT_TAGS
	} else if (tname && !didtag) {
		cmdlinetag(tname);
#endif
	}

#if OPT_POPUP_MSGS
	purge_msgs();
#endif
	if (startstat == TRUE)  /* else there's probably an error message */
		mlforce(msg);

 begin:
	(void)update(FALSE);

#if OPT_POPUP_MSGS
	if (global_g_val(GMDPOPUP_MSGS) == -TRUE)
		set_global_g_val(GMDPOPUP_MSGS, FALSE);
#endif

	/* process commands */
	loop();

	/* NOTREACHED */
	return BADEXIT;
}

/* this is nothing but the main command loop */
void
loop()
{
	CMDFUNC	*cfp, *last_cfp = NULL;
	int s,c,f,n;

	while(1) {

		/* vi doesn't let the cursor rest on the newline itself.  This
			takes care of that. */
		/* if we're inserting, or will be inserting again, then
			suppress.  this happens if we're using arrow keys
			during insert */
		if (is_at_end_of_line(DOT) && (DOT.o > w_left_margin(curwp)) &&
				!insertmode)
			backchar(TRUE,1);

		/* same goes for end-of-file -- I'm actually not sure if
			this can ever happen, but I _am_ sure that it's
			a lot safer not to let it... */
		if (is_header_line(DOT,curbp) && !is_empty_buf(curbp))
			backline(TRUE,1);

		/* start recording for '.' command */
		dotcmdbegin();

		/* Fix up the screen	*/
		s = update(FALSE);

		/* get the next command from the keyboard */
		c = kbd_seq();

		/* if there is something on the command line, clear it */
		if (mpresf != 0) {
			mlerase();
			if (s != SORTOFTRUE) /* did nothing due to typeahead */
				(void)update(FALSE);
		}

		f = FALSE;
		n = 1;

#if LATERMAYBE
/* insertion is too complicated to pop in
	and out of so glibly...   -pgf */
#ifdef insertmode
		/* FIXME: Paul and Tom should check this over. */
		if (insertmode != FALSE) {
			if (!kbd_replaying(FALSE))
			    mayneedundo();
			unkeystroke(c);
			insert(f,n);
			dotcmdfinish();
			continue;
		}
#endif /* insertmode */
#endif /* LATERMAYBE */

		do_repeats(&c,&f,&n);

		kregflag = 0;

		/* flag the first time through for some commands -- e.g. subst
			must know to not prompt for strings again, and pregion
			must only restart the p-lines buffer once for each
			command. */
		calledbefore = FALSE;

		/* and execute the command */
		cfp = kcod2fnc(c);
		s = execute(cfp, f, n);

		/* stop recording for '.' command */
		dotcmdfinish();

		/* If this was a motion that failed, sound the alarm (like vi),
		 * but limit it to once, in case the user is holding down the
		 * autorepeat-key.
		 */
		if ( (cfp != NULL)
		 && ((cfp->c_flags & MOTION) != 0)
		 && (s == FALSE) ) {
			if (cfp != last_cfp || global_g_val(GMDMULTIBEEP)) {
				last_cfp = cfp;
				kbd_alarm();
			}
		} else
			last_cfp = NULL; /* avoid noise! */
	}
}

char *
strmalloc(s)
char *s;
{
	register char *ns = castalloc(char,strlen(s)+1);
	if (ns)
		return strcpy(ns,s);
	else
		return NULL;

}

int
no_memory(s)
char	*s;
{
	mlforce("[%s] %s", out_of_mem, s);
	return FALSE;
}

void
global_val_init()
{
	register int i;
	/* set up so the global value pointers point at the global
		values.  we never actually use the global pointers
		directly, but when buffers get a copy of the
		global_b_values structure, the pointers will still point
		back at the global values, which is what we want */
	for (i = 0; i <= NUM_G_VALUES; i++)
		make_local_val(global_g_values.gv, i);

	for (i = 0; i <= NUM_B_VALUES; i++)
		make_local_val(global_b_values.bv, i);

	for (i = 0; i <= NUM_W_VALUES; i++)
		make_local_val(global_w_values.wv, i);


	/*
	 * Universal-mode defaults
	 */
	set_global_g_val(GMDABUFF,	TRUE); 	/* auto-buffer */
	set_global_g_val(GMDALTTABPOS,	FALSE); /* emacs-style tab
							positioning */
#ifdef GMDDIRC
	set_global_g_val(GMDDIRC,	FALSE); /* directory-completion */
#endif
	set_global_g_val(GMDERRORBELLS, TRUE);	/* alarms are noticeable */
#if OPT_FLASH
	set_global_g_val(GMDFLASH,  	FALSE);	/* beeps beep by default */
#endif
#ifdef GMDHISTORY
	set_global_g_val(GMDHISTORY,	TRUE);
#endif
	set_global_g_val(GMDMULTIBEEP,	TRUE); /* multiple beeps for multiple
						motion failures */
#if OPT_WORKING
	set_global_g_val(GMDWORKING,  	TRUE);	/* we put up "working..." */
#endif
	/* which 8 bit chars are printable? */
	set_global_g_val(GVAL_PRINT_LOW, 0);
	set_global_g_val(GVAL_PRINT_HIGH, 0);


	/* catnap times: */
	/* how long to wait for ESC seq */
	set_global_g_val(GVAL_TIMEOUTVAL, 500);
	/* how long to wait for user seq */
#if SYS_MSDOS	/* actually, 16-bit ints */
	set_global_g_val(GVAL_TIMEOUTUSERVAL, 30000);
#else
	set_global_g_val(GVAL_TIMEOUTUSERVAL, 60000);
#endif

	/* allow remapping by default */
	set_global_g_val(GMDREMAP, TRUE);

#if OPT_VMS_PATH || OPT_MSDOS_PATH	/* ':' gets in the way of drives */
	set_global_g_val_ptr(GVAL_EXPAND_CHARS,strmalloc("!%#"));
#else	/* SYS_UNIX */
	set_global_g_val_ptr(GVAL_EXPAND_CHARS,strmalloc("!%#:"));
#endif
	set_global_g_val(GMDEXPAND_PATH,FALSE);
#ifdef GMDGLOB
	set_global_g_val(GMDGLOB, TRUE);
#endif
#ifdef GVAL_GLOB
	set_global_g_val_ptr(GVAL_GLOB, strmalloc("!echo %s"));
#endif

	set_global_g_val(GMDIMPLYBUFF,	FALSE); /* imply-buffer */
#if	OPT_POPUPCHOICE
# if	OPT_ENUM_MODES
	set_global_g_val_ptr(GVAL_POPUP_CHOICES,strmalloc("delayed"));
# else
	set_global_g_val(GMDPOPUP_CHOICES,TRUE);
# endif
#endif
#if	OPT_FILEBACK
# if	OPT_MSDOS_PATH
	set_global_g_val_ptr(GVAL_BACKUPSTYLE, strmalloc(".bak"));
# else
	set_global_g_val_ptr(GVAL_BACKUPSTYLE, strmalloc("off"));
# endif
#endif
#if	OPT_POPUP_MSGS
	set_global_g_val(GMDPOPUP_MSGS,-TRUE);	/* popup-msgs */
#endif
#ifdef GMDRAMSIZE
	set_global_g_val(GMDRAMSIZE,	TRUE);	/* show ram-usage */
#endif
	set_global_g_val(GVAL_REPORT,	5);	/* report changes */
#if	OPT_XTERM
	set_global_g_val(GMDXTERM_MOUSE,FALSE);	/* mouse-clicking */
#endif

	/*
	 * Buffer-mode defaults
	 */
	set_global_b_val(MDAIND,	FALSE); /* auto-indent */
	set_global_b_val(MDASAVE,	FALSE);	/* auto-save */
	set_global_b_val(MDBACKLIMIT,	TRUE); 	/* limit backspacing to
							insert point */
#ifdef	MDCHK_MODTIME
	set_global_b_val(MDCHK_MODTIME,	FALSE); /* modtime-check */
#endif
	set_global_b_val(MDCMOD,	FALSE); /* C mode */
#ifdef MDCRYPT
	set_global_b_val(MDCRYPT,	FALSE);	/* crypt */
#endif
	set_global_b_val(MDIGNCASE,	FALSE); /* exact matches */
	set_global_b_val(MDDOS, CRLF_LINES); /* on by default on DOS, off others */
	set_global_b_val(MDMAGIC,	TRUE); 	/* magic searches */
	set_global_b_val( MDMETAINSBIND, TRUE); /* honor meta-bindings when
							in insert mode */
	set_global_b_val(MDNEWLINE,	TRUE); 	/* trailing-newline */
	set_global_b_val(MDSHOWMAT,	FALSE);	/* show-match */
	set_global_b_val(MDSHOWMODE,	TRUE);	/* show-mode */
	set_global_b_val(MDSWRAP,	TRUE); 	/* scan wrap */
	set_global_b_val(MDTABINSERT,	TRUE);	/* allow tab insertion */
	set_global_b_val(MDTAGSRELTIV,	FALSE);	/* path relative tag lookups */
	set_global_b_val(MDTERSE,	FALSE);	/* terse messaging */
#if	OPT_UPBUFF
	set_global_b_val(MDUPBUFF,	TRUE);	/* animated */
#endif
	set_global_b_val(MDVIEW,	FALSE); /* view-only */
	set_global_b_val(MDWRAP,	FALSE); /* wrap */
#if OPT_LCKFILES
	/* locking defaults */
	set_global_g_val(GMDUSEFILELOCK,FALSE);	/* Use filelocks */
	set_global_b_val(MDLOCKED,	FALSE);	/* LOCKED */
	set_global_b_val_ptr(VAL_LOCKER, strmalloc("")); /* Name locker */
#endif
	set_global_g_val(GMDRONLYVIEW,	FALSE);	/* Set view-on-readonly */

	set_global_b_val(VAL_ASAVECNT,	256);	/* autosave count */
	set_global_b_val(VAL_C_SWIDTH,	8); 	/* C file shiftwidth */
	set_global_b_val(VAL_C_TAB,	8); 	/* C file tab stop */
	set_global_b_val(VAL_SWIDTH,	8); 	/* shiftwidth */
	set_global_b_val(VAL_TAB,	8);	/* tab stop */
	set_global_b_val(VAL_TAGLEN,	0);	/* significant tag length */
	set_global_b_val(VAL_UNDOLIM,	10);	/* undo limit */

	set_global_b_val_ptr(VAL_TAGS, strmalloc("tags")); /* tags filename */

#if SYS_VMS
#define	DEFAULT_CSUFFIX	"\\.\\(\\([CHIS]\\)\\|CC\\|CXX\\|HXX\\)\\(;[0-9]*\\)\\?$"
#endif
#if SYS_MSDOS || SYS_WIN31
#define	DEFAULT_CSUFFIX	"\\.\\(\\([chis]\\)\\|cpp\\|cxx\\|hxx\\)$"
#endif
#ifndef DEFAULT_CSUFFIX	/* UNIX or OS2/HPFS (mixed-case names) */
#define	DEFAULT_CSUFFIX	"\\.\\(\\([Cchis]\\)\\|CC\\|cpp\\|cxx\\|hxx\\|scm\\)$"
#endif

	/* suffixes for C mode */
	set_global_g_val_rexp(GVAL_CSUFFIXES,
		new_regexval(
			DEFAULT_CSUFFIX,
			TRUE));

	/* where do paragraphs start? */
	set_global_b_val_rexp(VAL_PARAGRAPHS,
		new_regexval(
			"^\\.[ILPQ]P\\>\\|^\\.P\\>\\|^\\.LI\\>\\|\
^\\.[plinb]p\\>\\|^\\.\\?\\s*$",
			TRUE));

	/* where do comments start and end, for formatting them */
	set_global_b_val_rexp(VAL_COMMENTS,
		new_regexval(
			"^\\s*/\\?[#*>]\\+/\\?\\s*$",
			TRUE));

	/* where do sections start? */
	set_global_b_val_rexp(VAL_SECTIONS,
		new_regexval(
			"^[{\014]\\|^\\.[NS]H\\>\\|^\\.HU\\?\\>\\|\
^\\.[us]h\\>\\|^+c\\>",
			TRUE));

	/* where do sentences start? */
	set_global_b_val_rexp(VAL_SENTENCES,
		new_regexval(
	"[.!?][])\"']* \\?$\\|[.!?][])\"']*  \\|^\\.[ILPQ]P\\>\\|\
^\\.P\\>\\|^\\.LI\\>\\|^\\.[plinb]p\\>\\|^\\.\\?\\s*$",
			TRUE));

	/*
	 * Window-mode defaults
	 */
#ifdef WMDLINEWRAP
	set_global_w_val(WMDLINEWRAP,	FALSE); /* line-wrap */
#endif
	set_global_w_val(WMDLIST,	FALSE); /* list-mode */
	set_global_w_val(WMDNUMBER,	FALSE);	/* number */
	set_global_w_val(WMDHORSCROLL,	TRUE);	/* horizontal scrolling */
#ifdef WMDTERSELECT
	set_global_w_val(WMDTERSELECT,	TRUE);	/* terse selections */
#endif

	set_global_w_val(WVAL_SIDEWAYS,	0);	/* list-mode */
#if defined(WVAL_FCOLOR) || defined(WVAL_BCOLOR)
	set_global_w_val(WVAL_FCOLOR,	C_WHITE); /* foreground color */
	set_global_w_val(WVAL_BCOLOR,	C_BLACK); /* background color */
#endif

}

#if SYS_UNIX || SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT || SYS_VMS

/* have we been interrupted/ */
static int am_interrupted = FALSE;

/* ARGSUSED */
SIGT
catchintr (ACTUAL_SIG_ARGS)
{
	am_interrupted = TRUE;
#if SYS_MSDOS || SYS_OS2 || SYS_WINNT
	sgarbf = TRUE;	/* there's probably a ^C on the screen. */
#endif
	(void)signal(SIGINT,catchintr);
	if (doing_kbd_read)
		longjmp(read_jmp_buf, signo);
	SIGRET;
}
#endif

int
interrupted()
{
#if SYS_MSDOS && CC_DJGPP

	if (_go32_was_ctrl_break_hit() != 0) {
		while(keystroke_avail())
			(void)keystroke();
		return TRUE;
	}
	if (was_ctrl_c_hit() != 0) {
		while(keystroke_avail())
			(void)keystroke();
		return TRUE;
	}

	if (am_interrupted)
		return TRUE;
	return FALSE;
#else
	return am_interrupted;
#endif
}

void
not_interrupted()
{
    am_interrupted = FALSE;
#if SYS_MSDOS
# if CC_DJGPP
    (void)_go32_was_ctrl_break_hit();  /* flush any pending kbd ctrl-breaks */
    (void)was_ctrl_c_hit();  /* flush any pending kbd ctrl-breaks */
# endif
#endif
}

#if SYS_MSDOS
# if CC_WATCOM
    int  dos_crit_handler(unsigned deverror, unsigned errcode, unsigned *devhdr)
# else
    void dos_crit_handler()
# endif
{
# if CC_WATCOM
	_hardresume((int)_HARDERR_FAIL);
	return (int)_HARDERR_FAIL;
# else
#  if ! CC_DJGPP
	_hardresume(_HARDERR_FAIL);
#  endif
# endif
}
#endif


void
siginit()
{
#if SYS_UNIX
	(void)signal(SIGINT,catchintr);
	(void)signal(SIGHUP,imdying);
#ifdef SIGBUS
	(void)signal(SIGBUS,imdying);
#endif
#ifdef SIGSYS
	(void)signal(SIGSYS,imdying);
#endif
	(void)signal(SIGSEGV,imdying);
	(void)signal(SIGTERM,imdying);
#if DEBUG
	(void)signal(SIGQUIT,imdying);
#else
	(void)signal(SIGQUIT,SIG_IGN);
#endif
	(void)signal(SIGPIPE,SIG_IGN);
#if defined(SIGWINCH) && ! DISP_X11
	(void)signal(SIGWINCH,sizesignal);
#endif
#else
# if SYS_MSDOS
	(void)signal(SIGINT,catchintr);
#  if CC_DJGPP
	_go32_want_ctrl_break(TRUE);
	setcbrk(FALSE);
	want_ctrl_c(TRUE);
	hard_error_catch_setup();
#  else
#   if CC_WATCOM
	{
	/* clean up Warning from Watcom C */
	void *ptrfunc = dos_crit_handler;
	_harderr(ptrfunc);
	}
#   else	/* CC_TURBO */
	_harderr(dos_crit_handler);
#   endif
#  endif
# endif
# if SYS_OS2 || SYS_WINNT
	(void)signal(SIGINT,catchintr);
#endif
#endif

}

void
siguninit()
{
#if SYS_MSDOS
# if CC_DJGPP
	_go32_want_ctrl_break(FALSE);
	want_ctrl_c(FALSE);
	hard_error_teardown();
	setcbrk(TRUE);
# endif
#endif
}

/* do number processing if needed */
static void
do_num_proc(cp,fp,np)
int *cp, *fp, *np;
{
	register int c, f, n;
	register int	mflag;
	register int oldn;

	c = *cp;

	if (iscntrl(c) || isspecial(c))
		return;

	f = *fp;
	n = *np;
	if (f)
		oldn = n;
	else
		oldn = 1;
	n = 1;

	if ( isdigit(c) && c != '0' ) {
		n = 0;		/* start with a zero default */
		f = TRUE;	/* there is a # arg */
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
	*np = n * oldn;
}

/* do ^U-style repeat argument processing -- vile binds this to 'K' */
static void
do_rept_arg_proc(cp,fp,np)
int *cp, *fp, *np;
{
	register int c, f, n;
	register int	mflag;
	register int	oldn;
	c = *cp;

	if (c != reptc)
		return;

	f = *fp;
	n = *np;

	if (f)
		oldn = n;
	else
		oldn = 1;

	n = 4;		/* start with a 4 */
	f = TRUE;	/* there is a # arg */
	mflag = 0;			/* that can be discarded. */
	mlwrite("arg: %d",n);
	while (isdigit(c=kbd_seq()) || c==reptc || c=='-'){
		if (c == reptc)
			/* wow.  what does this do?  -pgf */
			/* (i've been told it controls overflow...) */
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
	*np = n * oldn;
}

/* handle all repeat counts */
void
do_repeats(cp,fp,np)
int *cp,*fp,*np;
{
	do_num_proc(cp,fp,np);
	do_rept_arg_proc(cp,fp,np);
	if (dotcmdmode == PLAY) {
		if (dotcmdarg)	/* then repeats are done by dotcmdcnt */
			*np = 1;
	} else {
		/* then we want to cancel any dotcmdcnt repeats */
		if (*fp) dotcmdarg = FALSE;
	}
}

int quitting;

/* the vi ZZ command -- write all, then quit */
int
zzquit(f,n)
int f,n;
{
	int thiscmd;
	int cnt;
	BUFFER *bp;

	thiscmd = lastcmd;
	cnt = anycb(&bp);
	if (cnt) {
	    	if (cnt > 1) {
		    mlprompt("Will write %d buffers.  %s ", cnt,
			    clexec ? s_NULL : "Repeat command to continue.");
		} else {
		    mlprompt("Will write buffer \"%s\".  %s ",
			    get_bname(bp),
			    clexec ? s_NULL : "Repeat command to continue.");
		}
		if (!clexec && !isnamedcmd) {
			if (thiscmd != kbd_seq())
				return FALSE;
		}

		quitting = TRUE;
		if (writeall(f,n,FALSE,TRUE,FALSE) != TRUE) {
			quitting = FALSE;
			return FALSE;
		}

	} else if (!clexec && !isnamedcmd) {
		/* consume the next char. anyway */
		if (thiscmd != kbd_seq())
			return FALSE;
	}
	return quithard(f, n);
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit, otherwise simply exit.
 */
int
quickexit(f, n)
int f,n;
{
	register int status;
	quitting = TRUE;
	if ((status = writeall(f,n,FALSE,TRUE,FALSE)) == TRUE)
		status = quithard(f, n);  /* conditionally quit	*/
	quitting = FALSE;
	return status;
}

/* Force quit by giving argument */
/* ARGSUSED */
int
quithard(f,n)
int f,n;
{
	return quit(TRUE,1);
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out.
 */
/* ARGSUSED */
int
quit(f, n)
int f,n;
{
	int cnt;
	BUFFER *bp;

#if OPT_PROCEDURES
	{
	    static int exithooking;
	    if (!exithooking && *exithook) {
		    exithooking = TRUE;
		    run_procedure(exithook);
		    exithooking = FALSE;
	    }
	}
#endif

	if (f == FALSE && (cnt = anycb(&bp)) != 0) {
		if (cnt == 1)
			mlforce(
			"Buffer \"%s\" is modified.  Write it, or use :q!",
				get_bname(bp));
		else
			mlforce(
			"There are %d unwritten modified buffers.  Write them, or use :q!",
				cnt);
		return FALSE;
	}
#if	OPT_BSD_FILOCK
	if (lockrel() != TRUE) {
		ExitProgram(BADEXIT);
		/* NOTREACHED */
	}
#endif
#if OPT_LCKFILES
	/* Release all placed locks */
	if ( global_g_val(GMDUSEFILELOCK) ) {
		for_each_buffer(bp) {
			if ( bp->b_active ) {
				if (!b_val(curbp,MDLOCKED) &&
						!b_val(curbp,MDVIEW))
					release_lock(bp->b_fname);
			}
		}
	}
#endif
	siguninit();
#if OPT_WORKING
# if defined(SIGALRM)
	(void)signal(SIGALRM, SIG_IGN);
# endif
#if NEEDED	/* i'm not sure when we'd end up with a "working..."
			left on the line, and if we _do_ need to
			clear it, i'd like to figure out how to
			clear just that, so the last message written
			by the editor doesn't get tromped */
	/* force the message line clear */
	mpresf = 1;
	mlerase();
#endif
#endif
	vttidy(TRUE);
#if NO_LEAKS
	{
		beginDisplay;		/* ...this may take a while... */

		/* free all of the global data structures */
		onel_leaks();
		path_leaks();
		kbs_leaks();
		tb_leaks();
		wp_leaks();
		bp_leaks();
		vt_leaks();
		ev_leaks();
		tmp_leaks();
#if DISP_X11
		x11_leaks();
#endif

		free_local_vals(g_valuenames, global_g_values.gv, global_g_values.gv);
		free_local_vals(b_valuenames, global_b_values.bv, global_b_values.bv);
		free_local_vals(w_valuenames, global_w_values.wv, global_w_values.wv);

		FreeAndNull(gregexp);
		FreeAndNull(patmatch);

#if SYS_UNIX
		if (strcmp(pathname[2], ".")) free(pathname[2]);
#endif
		/* whatever is left over must be a leak */
		show_alloc();
	}
#endif
	ExitProgram(GOODEXIT);
	/* NOTREACHED */
	return FALSE;
}

/* ARGSUSED */
int
writequit(f,n)
int f,n;
{
	int s;
	s = filesave(FALSE,n);
	if (s != TRUE)
		return s;
	return quit(FALSE,n);
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
/* ARGSUSED */
int
esc_func(f, n)
int f,n;
{
	dotcmdmode = STOP;
	regionshape = EXACT;
	doingopcmd = FALSE;
	doingsweep = FALSE;
	sweephack = FALSE;
	opcmd = 0;
	mlwarn("[Aborted]");
	return ABORT;
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode				*/

int
rdonly()
{
	mlwarn("[No changes are allowed while in \"view\" mode]");
	return FALSE;
}

/* ARGSUSED */
int
unimpl(f,n)
int f,n;
{
	mlwarn("[Sorry, that vi command is unimplemented in vile ]");
	return FALSE;
}

int opercopy(f,n) int f,n; { return unimpl(f,n); }
int opermove(f,n) int f,n; { return unimpl(f,n); }
int opertransf(f,n) int f,n; { return unimpl(f,n); }

int operglobals(f,n) int f,n; { return unimpl(f,n); }
int opervglobals(f,n) int f,n; { return unimpl(f,n); }

int source(f,n) int f,n; { return unimpl(f,n); }

int visual(f,n) int f,n; { return unimpl(f,n); }
int ex(f,n) int f,n; { return unimpl(f,n); }

/* ARGSUSED */
int
nullproc(f,n)	/* user function that does (almost) NOTHING */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
cntl_a_func(f,n)	/* dummy function for binding to control-a prefix */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
cntl_x_func(f,n)	/* dummy function for binding to control-x prefix */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
poundc_func(f,n)	/* dummy function for binding to poundsign prefix */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
unarg_func(f,n) /* dummy function for binding to universal-argument */
int f,n;
{
	return TRUE;
}


/* initialize our version of the "chartypes" stuff normally in ctypes.h */
/* also called later, if charset-affecting modes change, for instance */
void
charinit()
{
	register int c;

	(void)memset((char *)_chartypes_, 0, sizeof(_chartypes_));

	/* legal in pathnames */
	_chartypes_['.'] =
		_chartypes_['_'] =
		_chartypes_['~'] =
		_chartypes_['-'] =
		_chartypes_['*'] =
		_chartypes_['/'] = _pathn;

	/* legal in "identifiers" */
	_chartypes_['_'] |= _ident|_qident;
	_chartypes_[':'] |= _qident;

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
		_chartypes_[c] |= _lower|_pathn|_ident|_qident;

	/* uppercase */
	for (c = 'A'; c <= 'Z'; c++)
		_chartypes_[c] |= _upper|_pathn|_ident|_qident;

	/* digits */
	for (c = '0'; c <= '9'; c++)
		_chartypes_[c] |= _digit|_pathn|_ident|_qident|_linespec;

	/* punctuation */
	for (c = '!'; c <= '/'; c++)
		_chartypes_[c] |= _punct;
	for (c = ':'; c <= '@'; c++)
		_chartypes_[c] |= _punct;
	for (c = '['; c <= '`'; c++)
		_chartypes_[c] |= _punct;
	for (c = LBRACE; c <= '~'; c++)
		_chartypes_[c] |= _punct;

	/* printable */
	for (c = ' '; c <= '~'; c++)
		_chartypes_[c] |= _print;
	c = global_g_val(GVAL_PRINT_LOW);
	if (c < HIGHBIT) c = HIGHBIT;
	while ( c <= global_g_val(GVAL_PRINT_HIGH) && c < N_chars)
		_chartypes_[c++] |= _print;

	/* backspacers: ^H, rubout */
	_chartypes_['\b'] |= _bspace;
	_chartypes_[127] |= _bspace;

	/* wildcard chars for most shells */
	_chartypes_['*'] |= _wild;
	_chartypes_['?'] |= _wild;
#if !SYS_VMS
	_chartypes_['~'] |= _wild;
	_chartypes_[LBRACK] |= _wild;
	_chartypes_[RBRACK] |= _wild;
	_chartypes_[LBRACE] |= _wild;
	_chartypes_[RBRACE] |= _wild;
	_chartypes_['$'] |= _wild;
	_chartypes_['`'] |= _wild;
#endif

	/* ex mode line specifiers */
	_chartypes_[','] |= _linespec;
	_chartypes_['%'] |= _linespec;
	_chartypes_['-'] |= _linespec;
	_chartypes_['+'] |= _linespec;
	_chartypes_['.'] |= _linespec;
	_chartypes_['$'] |= _linespec;
	_chartypes_['\''] |= _linespec;

	/* fences */
	_chartypes_[LBRACE] |= _fence;
	_chartypes_[RBRACE] |= _fence;
	_chartypes_[LPAREN] |= _fence;
	_chartypes_[RPAREN] |= _fence;
	_chartypes_[LBRACK] |= _fence;
	_chartypes_[RBRACK] |= _fence;

#if OPT_VMS_PATH
	_chartypes_[LBRACK] |= _pathn;	/* actually, "<", ">" too */
	_chartypes_[RBRACK] |= _pathn;
	_chartypes_['$'] |= _pathn;
	_chartypes_[':'] |= _pathn;
	_chartypes_[';'] |= _pathn;
#endif

#if OPT_WIDE_CTYPES
	/* scratch-buffer-names (usually superset of _pathn) */
	_chartypes_[(unsigned)SCRTCH_LEFT[0]]  |= _scrtch;
	_chartypes_[(unsigned)SCRTCH_RIGHT[0]] |= _scrtch;
	_chartypes_[' '] |= _scrtch;	/* ...to handle "[Buffer List]" */
#endif

	for (c = 0; c < N_chars; c++) {
#if OPT_WIDE_CTYPES
		if (isspace(c) || isprint(c))
			_chartypes_[c] |= _shpipe;
		if (ispath(c))
			_chartypes_[c] |= _scrtch;
#endif
		if ((_chartypes_[c] & _space) == 0)
			_chartypes_[c] |= _nonspace;
	}

}

/*****		Compiler specific Library functions	****/


#if	OPT_RAMSIZE
/*	These routines will allow me to track memory usage by placing
	a layer on top of the standard system malloc() and free() calls.
	with this code defined, the environment variable, $RAM, will
	report on the number of bytes allocated via malloc.

	with SHOWRAM defined, the number is also posted on the
	end of the bottom mode line and is updated whenever it is changed.
*/

#undef	realloc
#undef	malloc
#undef	free

	/* display the amount of RAM currently malloc'ed */
static void
display_ram_usage P((void))
{
	beginDisplay;
	if (global_g_val(GMDRAMSIZE)) {
		char mbuf[20];
		int	saverow = ttrow;
		int	savecol = ttcol;

		if (saverow >= 0 && saverow < term.t_nrow
		 && savecol >= 0 && savecol < term.t_ncol) {
			movecursor(term.t_nrow-1, LastMsgCol);
#if	OPT_COLOR
			TTforg(gfcolor);
			TTbacg(gbcolor);
#endif
			(void)lsprintf(mbuf, "[%ld]", envram);
			kbd_puts(mbuf);
			movecursor(saverow, savecol);
			TTflush();
		}
	}
	endofDisplay;
}

	/* reallocate mp with nbytes and track */
char *reallocate(mp, nbytes)
char *mp;
unsigned nbytes;
{
	if (mp != 0) {
		mp -= sizeof(SIZE_T);
		envram -= *((SIZE_T *)mp);
		nbytes += sizeof(SIZE_T);
		mp = realloc(mp, nbytes);
		if (mp != 0) {
			*((SIZE_T *)mp) = nbytes;
			envram += nbytes;
		}
		display_ram_usage();
	} else
		mp = allocate(nbytes);
	return mp;
}

	/* allocate nbytes and track */
char *allocate(nbytes)
unsigned nbytes;	/* # of bytes to allocate */
{
	char *mp;	/* ptr returned from malloc */

	nbytes += sizeof(SIZE_T);
	if ((mp = malloc(nbytes)) != 0) {
		(void)memset(mp, 0, nbytes);	/* so we can use for calloc */
		*((SIZE_T *)mp) = nbytes;
		envram += nbytes;
		mp += sizeof(SIZE_T);
		display_ram_usage();
	}

	return mp;
}

	/* release malloced memory and track */
void
release(mp)
char *mp;	/* chunk of RAM to release */
{
	if (mp) {
		mp -= sizeof(SIZE_T);
		envram -= *((SIZE_T *)mp);
		free(mp);
		display_ram_usage();
	}
}
#endif	/* OPT_RAMSIZE */

#if MALLOCDEBUG
mallocdbg(f,n)
{
	int lvl;
	lvl = malloc_debug(n);
	mlwrite("malloc debug level was %d",lvl);
	if (!f) {
		malloc_debug(lvl);
	} else if (n > 2) {
		malloc_verify();
	}
	return TRUE;
}
#endif


/*
 *	the log file is left open, unbuffered.  thus any code can do
 *
 * 	extern FILE *FF;
 *	fprintf(FF, "...", ...);
 *
 *	to log events without disturbing the screen
 */

#ifdef DEBUGLOG
/* suppress the declaration so that the link will fail if someone uses it */
FILE *FF;
#endif

void
start_debug_log(ac,av)	/* ARGSUSED */
int ac;
char **av;
{
#ifdef DEBUGLOG
	int i;
	FF = fopen("vilelog", "w");
	setbuf(FF,NULL);
	for (i = 0; i < ac; i++)
		(void)fprintf(FF,"arg %d: %s\n",i,av[i]);
#endif
}

#if SYS_MSDOS

#if CC_TURBO
int
showmemory(f,n)
int	f,n;
{
	extern	long	coreleft(void);
	mlforce("Memory left: %D bytes", coreleft());
	return TRUE;
}
#endif

#if CC_WATCOM
int
showmemory(f,n)
int	f,n;
{
	mlforce("Watcom C doesn't provide a very useful 'memory-left' call.");
	return TRUE;
}
#endif

#if CC_DJGPP
int
showmemory(f,n)
int	f,n;
{
	mlforce("Memory left: %D Kb virtual, %D Kb physical",
			_go32_dpmi_remaining_virtual_memory()/1024,
			_go32_dpmi_remaining_physical_memory()/1024);
	return TRUE;
}
#endif
#endif /* SYS_MSDOS */

/*
 * Try to invoke 'exit()' from only one point so we can cleanup temporary
 * files.
 */
#if OPT_MAP_MEMORY
void
exit_program(code)
int	code;
{
	tmp_cleanup();
	exit(code);
}
#endif

char *
strncpy0(t, f, l)
char *t, *f;
SIZE_T l;
{
    (void)strncpy(t, f, l);
    if (l)
	t[l-1] = EOS;
    return t;
}
