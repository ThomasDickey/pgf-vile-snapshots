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
 * $Log: main.c,v $
 * Revision 1.169  1994/03/11 11:58:52  pgf
 * we now pass the name of the desired screen resolution directly to
 * the ibm screen driver via the current_res_name global.  any option
 * starting with a digit is considered a screen resolution, preserving
 * backwards compatability with -2, -25, -4, -43, -5, -50.  in addition,
 * -80x25 or -40x12 will also now work from the command line
 *
 * Revision 1.168  1994/03/10  20:10:31  pgf
 * quit and zzquit now prompt with name of buffer if only one buffer
 * is modified.
 *
 * Revision 1.167  1994/03/08  12:19:53  pgf
 * changed 'fulllineregions' to 'regionshape'.
 *
 * Revision 1.166  1994/02/22  18:09:24  pgf
 * when choosing dos-mode for an ambiguous buffer, vote in favor of on
 * if the global mode is set, and we're running on DOS.  otherwise, choose
 * no-dos-mode.
 *
 * Revision 1.165  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

/* Make global definitions not external */
#define realdef
#include	"estruct.h"	/* global structures and defines */
#include	"edef.h"	/* global definitions */
#include	"nevars.h"

#if UNIX
#include        <fcntl.h>	/* defines 'open()' on SunOS */
#endif

#if MSDOS
#include <io.h>
#if DJGPP
#include <dpmi.h>
#include <go32.h>
#endif
#endif

extern char *pathname[];	/* startup file path/name array */

/* for MSDOS, increase the default stack space */

#if	MSDOS & LATTICE
unsigned _stack = 32767;
#endif

#if	ATARI & LATTICE & 0
int _mneed = 256000;		/* reset memory pool size */
#endif

#if	MSDOS & AZTEC
int _STKSIZ = 32767/16; 	/* stack size in paragraphs */
int _STKRED = 1024;		/* stack checking limit */
int _HEAPSIZ = 4096/16; 	/* (in paragraphs) */
int _STKLOW = 0;		/* default is stack above heap (small only) */
#endif

#if	MSDOS & TURBO
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
	char bname[NBUFN+1];		/* buffer name of file to read */
	char *msg;
#if TAGS
	int didtag = FALSE;		/* look up a tag to start? */
	char *tname = NULL;
#endif
#if	CRYPT
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
#if NEW_VI_MAP
	map_init();
#endif
#if MSDOS
	slash = '\\';
#else
#if ST520
	slash = '\\';
#else	/* UNIX */
	slash = '/';
#endif
#endif
#if !UNIX
	expand_wild_args(&argc, &argv);
#endif
	prog_arg = argv[0];	/* this contains our only clue to exec-path */

#if UNIX || VMS
	makeversion();
#endif

	start_debug_log(argc,argv);

	if (strcmp(pathleaf(prog_arg), "view") == 0)
		set_global_b_val(MDVIEW,TRUE);

#if X11
	x_preparse_args(&argc, &argv);
#endif
	/* Parse the command line */
	for (carg = 1; carg < argc; ++carg) {
		register char *param = argv[carg];
#if X11 && !XTOOLKIT
		if (*param == '=') {
			x_set_geometry(param);
			continue;
		}
#endif


		/* Process Switches */
		if (*param == '-') {
			++param;
#if IBMPC
		    	/* if it's a digit, it's probably a screen
				resolution */
			if (isdigit(*param)) {
				current_res_name = param;
				continue;
			} else
#endif	/* IBMPC */
			switch (*param) {
#if X11 && !XTOOLKIT
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
#endif /* X11 */
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
#if	CRYPT
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
				(void)strncpy(pat, param, NPAT);
				gregexp = regcomp(pat, global_b_val(MDMAGIC));
				break;
#if TAGS
			case 't':  /* -t for initial tag lookup */
			case 'T':
				GetArgVal(param);
				tname = param;
				break;
#endif
			case 'V':
				(void)printf("vile %s\n", version);
				ExitProgram(GOOD);

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
#if CRYPT
			cryptkey = (*ekey != EOS) ? ekey : 0;
#endif
			/* set up a buffer for this file */
			makename(bname, param);
			(void)unqname(bname,FALSE);

			bp = bfind(bname, BFARGS);
			ch_fname(bp, param);
			make_current(bp); /* pull it to the front */
			if (firstbp == 0)
				firstbp = bp;
#if CRYPT
			cryptkey = 0;
#endif
		}
	}


	/* if stdin isn't a terminal, assume the user is trying to pipe a
	 * file into a buffer.
	 */
#if UNIX || VMS || MSDOS
	if (!isatty(fileno(stdin))) {
		BUFFER	*lastbp = firstbp;
		FILE	*in;
		int	fd;
		int	nline = 0;

		bp = bfind(ScratchName(Standard Input), BFARGS);
		make_current(bp); /* pull it to the front */
		if (firstbp == 0)
			firstbp = bp;
		ffp = fdopen(dup(fileno(stdin)), "r");
#if UNIX
		fd = open("/dev/tty", 0);
#endif
#if VMS
		fd = open("tt:", 0);	/* or sys$command */
#endif
#if MSDOS
		fd = fileno(stderr);	/* this normally cannot be redirected */
#endif
		if ((fd >= 0)
		 && (close(0) >= 0)
		 && (fd = dup(fd)) == 0
		 && (in = fdopen(fd, "r")) != 0)
			*stdin = *in;

		(void)slowreadf(bp, &nline);
		set_rdonly(bp, bp->b_fname);
		(void)ffclose();

		if (!isatty(fileno(stdout)) && is_empty_buf(bp)) {
			(void)zotbuf(bp);
			firstbp = lastbp;
		}
#if FINDERR
		else set_febuff(get_bname(bp));
#endif
	}
#endif

	/* we made some calls to make_current() above, to shuffle the
		list order.  this set curbp, which isn't actually kosher */
	curbp = NULL;

	/* initialize the editor */

	siginit();
	vtinit();		/* Display */
	winit();		/* windows */

	/* this comes out to 70 on an 80 (or greater) column display */
	{	register int fill;
		fill = (7 * term.t_ncol) / 8;  /* must be done after vtinit() */
		if (fill > 70) fill = 70;
		set_global_b_val(VAL_FILL, fill);
	}

	/* pull in an unnamed buffer, if we were given none to work with */
	if (firstbp == 0) {
		bp = bfind(ScratchName(unnamed), 0);
		bp->b_active = TRUE;
#if DOSFILES
		make_local_b_val(bp,MDDOS);
		set_b_val(bp, MDDOS, MSDOS && global_b_val(MDDOS) );
#endif
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

			if ((vbp=bfind(ScratchName(vileinit), 0))==NULL)
				ExitProgram(BAD(1));

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
#if MSDOS
			/* search PATH for vilerc under dos */
	 		fname = flook(pathname[0], FL_ANYWHERE);
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
		nextbuffer(FALSE,0);
	}
#if TAGS
	else if (tname) {
		cmdlinetag(tname);
		didtag = TRUE;
	}
#endif
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
		 + (tname?1:0)
#endif
		> 1) {
#if TAGS
		msg = "[Search, goto and tag are used one at a time]";
#else
		msg = "[Cannot search and goto at the same time]";
#endif
	} else if (gotoflag) {
		if (gotoline(gline != 0, gline) == FALSE)
			msg = "[Invalid goto argument]";
	} else if (searchflag) {
		forwhunt(FALSE, 0);
#if TAGS
	} else if (tname && !didtag) {
		cmdlinetag(tname);
#endif
	}

	if (startstat == TRUE)  /* else there's probably an error message */
		mlforce(msg);

 begin:
	(void)update(FALSE);

	/* process commands */
	loop();

	/* NOTREACHED */
	return BAD(1);
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
				!insertmode && !insert_mode_was)
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

		do_repeats(&c,&f,&n);
		map_check(c);

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
#if OPT_FLASH
	set_global_g_val(GMDFLASH,  	FALSE);	/* beeps beep by default */
#endif
#ifdef GMDHISTORY
	set_global_g_val(GMDHISTORY,	TRUE);
#endif
	set_global_g_val(GMDMULTIBEEP,	TRUE); /* multiple beeps for multiple
						motion failures */
	/* which 8 bit chars are printable? */
	set_global_g_val(GVAL_PRINT_LOW, 0);
	set_global_g_val(GVAL_PRINT_HIGH, 0);


	set_global_g_val(GVAL_TIMEOUTVAL, 500);	/* catnap time -- how long
							to wait for ESC seq */
#if VMS || MSDOS			/* ':' gets in the way of drives */
	set_global_g_val_ptr(GVAL_EXPAND_CHARS,strmalloc("!%#"));
#else	/* UNIX */
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
#ifdef GMDPOPUP_FILEC
	set_global_g_val(GMDPOPUP_FILEC,FALSE); /* popup-choices */
#endif
#ifdef GMDPOPUP_MSGS
	set_global_g_val(GMDPOPUP_MSGS,	FALSE); /* popup-msgs */
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
	set_global_b_val(MDDOS, MSDOS);	/* on by default on DOS, off others */
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

	set_global_b_val(VAL_ASAVECNT,	256);	/* autosave count */
	set_global_b_val(VAL_C_SWIDTH,	8); 	/* C file shiftwidth */
	set_global_b_val(VAL_C_TAB,	8); 	/* C file tab stop */
	set_global_b_val(VAL_SWIDTH,	8); 	/* shiftwidth */
	set_global_b_val(VAL_TAB,	8);	/* tab stop */
	set_global_b_val(VAL_TAGLEN,	0);	/* significant tag length */
	set_global_b_val(VAL_UNDOLIM,	10);	/* undo limit */

	set_global_b_val_ptr(VAL_TAGS, strmalloc("tags")); /* tags filename */

#if VMS
#define	DEFAULT_CSUFFIX	"\\.\\(\\([CHIS]\\)\\|CC\\|CXX\\|HXX\\)\\(;[0-9]*\\)\\?$"
#endif
#if MSDOS
#define	DEFAULT_CSUFFIX	"\\.\\(\\([chis]\\)\\|cpp\\|cxx\\|hxx\\)$"
#endif
#ifndef DEFAULT_CSUFFIX	/* UNIX (mixed-case names) */
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

	set_global_w_val(WVAL_SIDEWAYS,	0);	/* list-mode */
#if defined(WVAL_FCOLOR) || defined(WVAL_BCOLOR)
	set_global_w_val(WVAL_FCOLOR,	C_WHITE); /* foreground color */
	set_global_w_val(WVAL_BCOLOR,	C_BLACK); /* background color */
#endif


}

#if UNIX || MSDOS || VMS

/* have we been interrupted/ */
static int am_interrupted = FALSE;

/* ARGSUSED */
SIGT
catchintr (ACTUAL_SIG_ARGS)
{
	am_interrupted = TRUE;
#if MSDOS
	sgarbf = TRUE;	/* there's probably a ^C on the screen. */
#endif
#if USG || MSDOS
	(void)signal(SIGINT,catchintr);
#endif
	if (doing_kbd_read)
		longjmp(read_jmp_buf, signo);
	SIGRET;
}
#endif

int
interrupted()
{
#if MSDOS && DJGPP

	if (_go32_was_ctrl_break_hit() != 0) {
		while(typahead())
			(void)tgetc(FALSE);
		return TRUE;
	}
	if (was_ctrl_c_hit() != 0) {
		while(typahead())
			(void)tgetc(FALSE);
		return TRUE;
	}

	if (am_interrupted)
		return TRUE;
#ifdef NEEDED
	if (typahead()) {
		int c;
		c = tgetc(FALSE);
		if (c == tocntrl('C'))
			return TRUE;
		tungetc(c);
	}
#endif
	return FALSE;
#else
	return am_interrupted;
#endif
}

void
not_interrupted()
{
    am_interrupted = FALSE;
#if MSDOS
# if DJGPP
    (void)_go32_was_ctrl_break_hit();  /* flush any pending kbd ctrl-breaks */
    (void)was_ctrl_c_hit();  /* flush any pending kbd ctrl-breaks */
# endif
#endif
}

#if MSDOS
# if WATCOM
    int  dos_crit_handler(unsigned deverror, unsigned errcode, unsigned *devhdr)
# else
    void dos_crit_handler()
# endif
{
# if WATCOM
	_hardresume((int)_HARDERR_FAIL);
	return (int)_HARDERR_FAIL;
# else
#  if ! DJGPP
	_hardresume(_HARDERR_FAIL);
#  endif
# endif
}
#endif


void
siginit()
{
#if UNIX
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
#if defined(SIGWINCH) && ! X11
	(void)signal(SIGWINCH,sizesignal);
#endif
#else
# if MSDOS
	(void)signal(SIGINT,catchintr);
#  if DJGPP
	_go32_want_ctrl_break(TRUE);
	setcbrk(FALSE);
	want_ctrl_c(TRUE);
	hard_error_catch_setup();
#  else
#   if WATCOM
	{
	/* clean up Warning from Watcom C */
	void *ptrfunc = dos_crit_handler;
	_harderr(ptrfunc);
	}
#   else	/* TURBO */
	_harderr(dos_crit_handler);
#   endif
#  endif
# endif
#endif

}

void
siguninit()
{
#if MSDOS
# if DJGPP
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
			    clexec ? "" : "Repeat command to continue.");
		} else {
		    mlprompt("Will write buffer \"%s\".  %s ",
			    get_bname(bp),
			    clexec ? "" : "Repeat command to continue.");
		}
		if (!clexec && !isnamedcmd) {
			if (thiscmd != kbd_seq())
				return FALSE;
		}

		if (writeall(TRUE,1) != TRUE)
			return FALSE;

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
	if ((status = writeall(TRUE,1)) == TRUE)
		status = quithard(f, n);  /* conditionally quit	*/
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
	vttidy(TRUE);
#if	FILOCK
	if (lockrel() != TRUE) {
		ExitProgram(BAD(1));
		/* NOTREACHED */
	}
#endif
	siguninit();
#if OPT_WORKING
	/* force the message line clear */
	mpresf = 1;
	mlerase();
#endif
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
#if X11
		x11_leaks();
#endif

		free_local_vals(g_valuenames, global_g_values.gv, global_g_values.gv);
		free_local_vals(b_valuenames, global_b_values.bv, global_b_values.bv);
		free_local_vals(w_valuenames, global_w_values.wv, global_w_values.wv);

		FreeAndNull(gregexp);
		FreeAndNull(patmatch);

#if UNIX
		if (strcmp(pathname[2], ".")) free(pathname[2]);
#endif
		/* whatever is left over must be a leak */
		show_alloc();
	}
#endif
	ExitProgram(GOOD);
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
esc(f, n)
int f,n;
{
	TTbeep();
	dotcmdmode = STOP;
	regionshape = EXACT;
	doingopcmd = FALSE;
	opcmd = 0;
	mlforce("[Aborted]");
	return ABORT;
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode				*/

int
rdonly()
{
	TTbeep();
	mlforce("[No changes are allowed while in \"view\" mode]");
	return FALSE;
}

/* ARGSUSED */
int
unimpl(f,n)
int f,n;
{
	TTbeep();
	mlforce("[Sorry, that vi command is unimplemented in vile ]");
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
cntl_af(f,n)	/* dummy function for binding to control-a prefix */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
cntl_xf(f,n)	/* dummy function for binding to control-x prefix */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
unarg(f,n) /* dummy function for binding to universal-argument */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
speckey(f,n) /* dummy function for binding to pseudo-function prefix, '#' */
int f,n;
{
	return TRUE;
}

/* ARGSUSED */
int
altspeckey(f,n)
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
#if !VMS
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

#if VMS
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

#if	RAMSIZE
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

		if (saverow >= 0 && saverow <= term.t_nrow
		 && savecol >= 0 && savecol <= term.t_ncol) {
			movecursor(term.t_nrow, LastMsgCol);
#if	COLOR
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
#endif	/* RAMSIZE */

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

#if TURBO
int
showmemory(f,n)
int	f,n;
{
	extern	long	coreleft(void);
	mlforce("Memory left: %D bytes", coreleft());
	return TRUE;
}
#endif

#if WATCOM
int
showmemory(f,n)
int	f,n;
{
	extern	long	_memavl(void);
	mlforce("Memory left: %D bytes", _memavl());
	return TRUE;
}
#endif

#if DJGPP
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
