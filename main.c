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
 * Revision 1.62  1992/04/14 08:42:42  pgf
 * don't handle SIGWINCH in X11 case
 *
 * Revision 1.61  1992/04/10  18:49:23  pgf
 * mark VILEINIT buf as scratch, so it goes away quickly
 *
 * Revision 1.60  1992/04/09  08:32:35  pgf
 * new vilerc processing was happening too late, after the first buffer
 * was already in.  this kept some modes from "sticking"
 *
 * Revision 1.59  1992/04/02  23:00:28  pgf
 * fixed empty buffer bug, just introduced
 *
 * Revision 1.58  1992/03/24  07:44:05  pgf
 * added support for VILEINIT variable for initialization
 *
 * Revision 1.57  1992/03/19  23:22:46  pgf
 * SIGT for signals, linux port
 *
 * Revision 1.56  1992/03/19  23:09:22  pgf
 * usage cleanup
 *
 * Revision 1.55  1992/03/07  10:36:29  pgf
 * fix missing goto-line argument problem.  "vile + file.c" now goes to end of
 * file, as it should
 *
 * Revision 1.54  1992/03/05  09:19:55  pgf
 * changed some mlwrite() to mlforce(), due to new terse support
 *
 * Revision 1.53  1992/03/03  21:59:02  pgf
 * added '`' to the _wild character set
 *
 * Revision 1.52  1992/03/03  09:35:52  pgf
 * added support for getting "words" out of the buffer via variables --
 * needed _nonspace character type
 *
 * Revision 1.51  1992/02/17  08:58:12  pgf
 * added "showmode" support, and kill registers now hold unsigned chars
 *
 * Revision 1.50  1992/01/14  20:24:54  pgf
 * don't ask for pressreturn() confirmation in quickexit() (ZZ command)
 *
 * Revision 1.49  1992/01/10  08:08:28  pgf
 * added initialization of shiftwidth
 *
 * Revision 1.48  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.47  1992/01/03  23:31:49  pgf
 * use new ch_fname() to manipulate filenames, since b_fname is now
 * a malloc'ed sting, to avoid length limits
 *
 * Revision 1.46  1991/11/27  10:09:09  pgf
 * slight rearrangement of init code, to prevent null filenames in makecurrent
 *
 * Revision 1.45  1991/11/16  18:38:58  pgf
 * changes for better magic mode -- the regexps for sections, paras etc. had
 * to change
 *
 * Revision 1.44  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.43  1991/11/07  02:00:32  pgf
 * lint cleanup
 *
 * Revision 1.42  1991/11/06  23:26:27  pgf
 * recomp the search pattern if set from command line, and
 * added _fence to chartypes
 *
 * Revision 1.41  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.40  1991/10/29  14:35:29  pgf
 * implemented the & commands: substagain
 *
 * Revision 1.39  1991/10/29  03:02:55  pgf
 * rename ctlx? routines to be ...kbd_macro... names, and allowed
 * for replaying of named registers
 *
 * Revision 1.38  1991/10/28  14:25:44  pgf
 * VAL_ASAVE --> VAL_ASAVECNT
 *
 * Revision 1.37  1991/10/27  01:45:10  pgf
 * set new regex values in global_val_init
 *
 * Revision 1.36  1991/10/24  13:05:52  pgf
 * conversion to new regex package -- much faster
 *
 * Revision 1.35  1991/10/23  14:20:53  pgf
 * changes to fix interactions between dotcmdmode and kbdmode and tungetc
 *
 * Revision 1.34  1991/10/22  03:08:45  pgf
 * call cmdlinetag() for command-line tags
 *
 * Revision 1.33  1991/10/20  23:07:56  pgf
 * pass taglength value to tags()
 *
 * Revision 1.32  1991/10/18  10:56:54  pgf
 * modified VALUE structures and lists to make them more easily settable
 *
 * Revision 1.31  1991/10/15  03:10:49  pgf
 * added backspacelimit and taglength
 *
 * Revision 1.30  1991/09/26  13:13:54  pgf
 * initialize window values, and
 * make sure file writing errors in writeall() are made visible
 *
 * Revision 1.29  1991/09/19  13:39:17  pgf
 * MDEXACT is now MDIGNCASE, and initialize new VAL_TAGS value to "tags"
 *
 * Revision 1.28  1991/09/17  13:02:57  pgf
 * added write-all and brethren
 *
 * Revision 1.27  1991/09/10  00:46:07	pgf
 * cleanup of the dotcmd stuff
 *
 * Revision 1.26  1991/08/13  02:50:13	pgf
 * initialize showmatch b_val
 *
 * Revision 1.25  1991/08/12  11:22:52	pgf
 * added 'vi +/searchstring file.c' invocation
 *
 * Revision 1.24  1991/08/12  10:23:43	pgf
 * esc() no longer kills keyboard recording
 *
 * Revision 1.23  1991/08/08  23:28:49	pgf
 * moved init of VAL_FILL to after vtinit, since it depends on term.t_ncol
 *
 * Revision 1.22  1991/08/08  13:19:58	pgf
 * removed ifdef BEFORE
 *
 * Revision 1.21  1991/08/07  12:35:07	pgf
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

#include	<stdio.h>

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
unsigned _stklen = 32768;
#endif

/* make global definitions not external */
#define maindef

#include	"estruct.h"	/* global structures and defines */

#if UNIX
#include	<signal.h>
#endif

#include	"nefunc.h"	/* function declarations */
#include	"nebind.h"	/* default key bindings */
#include	"nename.h"	/* name table */
#include	"edef.h"	/* global definitions */


#if	VMS
#include	<ssdef.h>
#define GOOD	(SS$_NORMAL)
#endif

#ifndef GOOD
#define GOOD	0
#endif

main(argc, argv)
int	argc;
char	*argv[];
{
	int    c;		/* command character */
	register BUFFER *bp;		/* temp buffer pointer */
	register int	gotafile;	/* filename arg present? */
	register int	carg;		/* current arg to scan */
	register int	ranstartup;	/* startup executed flag */
	int startstat = TRUE;		/* result of running startup */
	BUFFER *firstbp = NULL; 	/* ptr to first buffer in cmd line */
	int gotoflag;			/* do we need to goto a line at start? */
	int helpflag;			/* do we need to goto a line at start? */
	int gline;			/* if so, what line? */
	int searchflag; 		/* Do we need to search at start? */
#if TAGS
	int tagflag, didtag;			/* look up a tag to start? */
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
	extern SIGT catchintr();
	extern SIGT imdying();
	extern SIGT sizesignal();
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

#if X11
	x_preparse_args(&argc, &argv);
#endif
	/* Parse the command line */
	for (carg = 1; carg < argc; ++carg) {
#if X11
		if (argv[carg][0] == '=') {
                	x_set_geometry(argv[carg]);
                        continue;
                }
#endif

		/* Process Switches */
		if (argv[carg][0] == '-') {
			switch (argv[carg][1]) {
#if	NeWS
			case 'l':	/* -l for screen lines */
			case 'L':
				term.t_nrow = atoi(&argv[carg][2]);
				break;
#endif
#if X11
			case 'd':
				if (argv[carg + 1])
					x_set_dpy(argv[++carg]);
				else
					goto usage;
				break;
			case 'r':
			case 'R':
				x_set_rv();
				break;
			case 'f':
			case 'F':
				if (argv[carg + 1])
					x_setfont(argv[++carg]);
				else
					goto usage;

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
				gregexp = regcomp(pat, global_b_val(MDMAGIC));
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

			case '?':
			default:	/* unknown switch */
			usage:
	fprintf(stderr, "usage: %s [-flags] [@cmdfile] files...\n",argv[0]);
	fprintf(stderr, "	-h to get help on startup\n");
	fprintf(stderr, "	-gNNN or simply +NNN to go to line NNN\n");
	fprintf(stderr, "	-sstring or +/string to search for \"string\"\n");
#if TAGS
	fprintf(stderr, "	-ttagname to look up a tag\n");
#endif
	fprintf(stderr, "	-v to view files as read-only\n");
	/* fprintf(stderr, "	-e to edit (as opposed to view) files\n"); */
#if CRYPT
	fprintf(stderr, "	-kcryptkey for encrypted files\n");
#endif
#if X11
	fprintf(stderr, "	-f fontname to change font\n");
	fprintf(stderr, "	-d displayname to change the default display\n");
	fprintf(stderr, "	-r for reverse video\n");
#endif
#if NeWS
	fprintf(stderr, "	-lLINES to set the screen length\n");
#endif
	fprintf(stderr, "	use @filename to run filename as commands\n");
	fprintf(stderr, "	 (this will suppress .vilerc)\n");
				exit(1);
			}

		} else if (argv[carg][0]== '+') { /* alternate form of -g */
			if (argv[carg][1] == '/')
				goto dosearch;
			gotoflag = TRUE;
			gline = atoi(&argv[carg][1]);
		} else if (argv[carg][0]== '@') {
			/* Process Startup macroes */
			if ((startstat = startup(&argv[carg][1])) == TRUE)
				ranstartup = TRUE; /* don't execute .vilerc */
		} else {

			/* Process an input file */

			/* set up a buffer for this file */
			makename(bname, argv[carg]);
			unqname(bname,FALSE);

			bp = bfind(bname, OK_CREAT, 0);
			ch_fname(bp, argv[carg]);
			make_current(bp); /* pull it to the front */
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
#ifdef SIGBUS
	signal(SIGBUS,imdying);
#endif
#ifdef SIGSYS
	signal(SIGSYS,imdying);
#endif
	signal(SIGSEGV,imdying);
	signal(SIGTERM,imdying);
#if DEBUG
	signal(SIGQUIT,imdying);
#else
	signal(SIGQUIT,SIG_IGN);
#endif
	signal(SIGPIPE,SIG_IGN);
#if defined(SIGWINCH) && ! X11
	signal(SIGWINCH,sizesignal);
#endif
#endif
	vtinit();		/* Display */
	winit();		/* windows */
	varinit();		/* user variables */
        
	/* this comes out to 70 on an 80 (or greater) column display */
	{	register int fill;
		fill = (7 * term.t_ncol) / 8;  /* must be done after vtinit() */
		if (fill > 70) fill = 70;
		set_global_b_val(VAL_FILL, fill);
	}

	/* we made some calls to makecurrent() above, to shuffle the
		list order.  this set curbp, which isn't actually kosher */
	curbp = NULL;

	/* pull in an unnamed buffer, if we were given none to work with */
	if (!gotafile) {
		bp = bfind("[unnamed]", OK_CREAT, 0);
		bp->b_active = TRUE;
#if DOSFILES
		make_local_b_val(bp,MDDOS);
		set_b_val(bp, MDDOS, FALSE );
#endif
		swbuffer(bp);
	}

	/* if invoked with no other startup files,
	   run the system startup file here */
	if (!ranstartup) {
		char *getenv();
		char *vileinit;
		vileinit = getenv("VILEINIT");
		if (vileinit != NULL) {
			int odiscmd;
			BUFFER *vbp, *obp;
			int oflags;

			/* mark as modified, to prevent undispbuff() from
				 clobbering */
			obp = curbp;
			if (obp) {
				oflags = obp->b_flag;
				obp->b_flag |= BFCHG;
			}

			if ((vbp=bfind("[vileinit]", OK_CREAT, BFSCRTCH))==NULL)
				return FALSE;
			/* mark the buffer as read only */
			make_local_b_val(vbp,MDVIEW);
			set_b_val(vbp,MDVIEW,TRUE);

			vbp->b_active = TRUE;

			swbuffer(vbp);
			bprintf("%s", vileinit);
			vbp->b_flag &= ~BFCHG;

			/* go execute it! */
			odiscmd = discmd;
			discmd = FALSE;
			startstat = dobuf(vbp);
			discmd = odiscmd;
			if (obp) {
				swbuffer(obp);
				obp->b_flag = oflags;
			}
#ifdef NEEDED
	unnecessary -- swbuffer/undispbuff did this already due to BFSCRTCH
			/* remove the now unneeded buffer and exit */
			zotbuf(vbp);
#endif
		} else {
			/* if .vilerc is one of the input files....
					don't clobber it */
			if (gotafile && 
				strcmp(pathname[0], firstbp->b_bname) == 0) {
				c = firstbp->b_bname[0];
				firstbp->b_bname[0] = '[';
				startstat = startup(pathname[0]);
				firstbp->b_bname[0] = c;
			} else {
				startstat = startup(pathname[0]);
			}
		}
		ranstartup = TRUE;
	}


	/* if there are any files to read, read the first one! */
	if (gotafile) {
		nextbuffer(FALSE,0);
	}
#if TAGS
	else if (tagflag) {
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
		 + tagflag 
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
	} else if (tagflag && !didtag) {
		cmdlinetag(tname);
#endif
	}

	update(FALSE);
	if (startstat == TRUE)  /* else there's probably an error message */
		mlforce(msg);


	/* process commands */
	loop();

	/* NOTREACHED */
	return 0;
}

/* this is nothing but the main command loop */
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

		/* Fix up the screen	*/
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
			mlforce("BUG: main: bheadp != curbp, bhead name is %s",
					 bheadp->b_bname);

		/* stop recording for '.' command */
		dotcmdfinish();
	}
}

char *
strmalloc(s)
char *s;
{
	char *ns = malloc(strlen(s)+1);
	if (ns)
		return strcpy(ns,s);
	else
		return NULL;

}


global_val_init()
{
	register int i;
	struct regexval *rp;
	/* set up so the global value pointers point at the global
		values.  we never actually use the global pointers
		directly, but but when buffers get a copy of the
		global_b_values structure, the pointers will still point
		back at the global values, which is what we want */
	for (i = 0; i <= MAX_B_VALUES; i++)
		global_b_values.bv[i].vp = &(global_b_values.bv[i].v);

	for (i = 0; i <= MAX_W_VALUES; i++)
		global_w_values.wv[i].vp = &(global_w_values.wv[i].v);


	set_global_b_val(MDWRAP,FALSE); /* wrap */
	set_global_b_val(MDCMOD,FALSE); /* C mode */
	set_global_b_val(MDBACKLIMIT,TRUE); /* limit backspacing to insert point */
	set_global_b_val(MDSWRAP,TRUE); /* scan wrap */
	set_global_b_val(MDIGNCASE,FALSE); /* exact matches */
	set_global_b_val(MDVIEW,FALSE); /* view-only */
	set_global_b_val(MDMAGIC,TRUE); /* magic searches */
	set_global_b_val(MDCRYPT,FALSE);	/* crypt */
	set_global_b_val(MDASAVE,FALSE);	/* auto-save */
	set_global_b_val(MDDOS,FALSE);	/* dos mode */
	set_global_b_val(MDAIND,FALSE); /* auto-indent */
	set_global_b_val(MDSHOWMAT,FALSE);	/* show-match */
	set_global_b_val(MDSHOWMODE,TRUE);	/* show-mode */
	set_global_b_val(VAL_TAB, 8);	/* tab stop */
	set_global_b_val(VAL_SWIDTH, 8); /* shiftwidth */
	set_global_b_val(VAL_TAGLEN, 0);	/* significant tag length */
	set_global_b_val(VAL_C_TAB, 8); /* C file tab stop */
	set_global_b_val(VAL_ASAVECNT, 256);	/* autosave count */
	set_global_b_val_ptr(VAL_CWD, NULL);	/* current directory */
	set_global_b_val_ptr(VAL_TAGS, strmalloc("tags")); /* suffixes for C mode */

	/* suffixes for C mode */
	rp = (struct regexval *)malloc(sizeof (struct regexval));
	set_global_b_val_rexp(VAL_CSUFFIXES, rp);
	rp->pat = strmalloc("\\.[chs]$");
	rp->reg = regcomp(rp->pat, TRUE);

	/* where do paragraphs start? */
	rp = (struct regexval *)malloc(sizeof (struct regexval));
	set_global_b_val_rexp(VAL_PARAGRAPHS, rp);
	rp->pat = 
		strmalloc("^\\.[ILPQ]P\\|^\\.P[ 	]\\|^\\.LI\\|^\\.[plinb]p\\|^\\.\\?$");
	rp->reg = regcomp(rp->pat, TRUE);

	/* where do sections start? */
	rp = (struct regexval *)malloc(sizeof (struct regexval));
	set_global_b_val_rexp(VAL_SECTIONS, rp);
	rp->pat = strmalloc("^[{\014]\\|^\\.[NS]H\\|^\\.H[ 	U]\\|^\\.[us]h\\|^+c");
	rp->reg = regcomp(rp->pat, TRUE);

	/* where do sentences start? */
	rp = (struct regexval *)malloc(sizeof (struct regexval));
	set_global_b_val_rexp(VAL_SENTENCES, rp);
	rp->pat = strmalloc(
	"[.!?][])\"']* \\?$\\|[.!?][])\"']*  \\|^\\.[ILPQ]P\\|\
^\\.P[ 	]\\|^\\.LI\\|^\\.[plinb]p\\|^\\.\\?$");
	rp->reg = regcomp(rp->pat, TRUE);

	set_global_w_val(WMDLIST,FALSE); /* list-mode */
	set_global_w_val(WVAL_SIDEWAYS,0); /* list-mode */
	set_global_w_val(WVAL_FCOLOR,7); /* foreground color */
	set_global_w_val(WVAL_BCOLOR,0); /* background color */

}

#if UNIX
SIGT
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
	register int	mflag;

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
	register int	mflag;
	c = *cp;
	f = *fp;
	n = *np;

	if (c != reptc) 
		return;

	f = TRUE;
	n = 4;				/* with argument of 4 */
	mflag = 0;			/* that can be discarded. */
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
int f,n;
{
	register int status, flags;
	MARK odot;

	if (execfunc == NULL) {
		TTbeep();
#if REBIND
		mlforce("[Key not bound]");	/* complain		*/
#else
		mlforce("[Not a command]");	/* complain		*/
#endif
		return FALSE;
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
				return rdonly();
			mayneedundo();
		}
	}

	/* if motion is absolute, remember where we are */
	if (flags & ABS) {
		odot = DOT;
	}

	status = (execfunc->c_func)(f, n);
	if ((flags & GOAL) == 0) { /* goal should not be retained */
		curgoal = -1;
	}
#if VMALLOC
	if (flags & UNDO)	/* verify malloc arena after line changers */
		vverify("main");
#endif

	/* if motion was absolute, and it wasn't just on behalf of an
		operator, and we moved, update the "last dot" mark */
	if ((flags & ABS) && !doingopcmd && !sameline(DOT, odot)) {
		curwp->w_lastdot = odot;
	}


	return status;
}

/* write all _changed_ buffers */
writeall(f,n)
int f,n;
{
	register BUFFER *bp;	/* scanning pointer to buffers */
	register BUFFER *oldbp; /* original current buffer */
	register int status = TRUE;

	oldbp = curbp;				/* save in case we fail */

	bp = bheadp;
	while (bp != NULL) {
		if ((bp->b_flag&BFCHG) != 0 && (bp->b_flag&BFINVS) == 0) {
			make_current(bp);
			mlforce("[Saving %s]",bp->b_fname);
			mlforce("\n");
			if ((status = filesave(f, n)) != TRUE)
				break;
			mlforce("\n");
		}
		bp = bp->b_bufp;	/* on to the next buffer */
	}
	make_current(oldbp);
	mlforce("\n");
	if (status != TRUE || f == FALSE)
		pressreturn();
	sgarbf = TRUE;
	return status;
}

/* the vi ZZ command -- write all, then quit */
zzquit(f,n)
int f,n;
{
	int thiscmd;
	int cnt;

	thiscmd = lastcmd;
	cnt = anycb();
	if (cnt) {
		mlprompt("Will write %d buffer%c  %s ",
			cnt, cnt > 1 ? 's':'.',
			clexec ? "" : "Repeat command to continue.");
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
	quithard(f, n);
	return TRUE;
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit, otherwise simply exit.
 */
quickexit(f, n)
int f,n;
{
	if (writeall(TRUE,1) == TRUE)
		quithard(f, n); 	/* conditionally quit	*/
	return TRUE;
}

/* Force quit by giving argument */
/* ARGSUSED */
quithard(f,n)
int f,n;
{
    quit(TRUE,1);
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out.
 */
/* ARGSUSED */
quit(f, n)
int f,n;
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
		mlforce(
		"There is an unwritten modified buffer.  Write it, or use :q!");
	else
		mlforce(
		"There are %d unwritten modified buffers.  Write them, or use :q!",
			cnt);
	return FALSE;
}

/* ARGSUSED */
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
 * Begin recording the dot command macro.
 * Set up variables and return.
 * we use a temporary accumulator, in case this gets stopped prematurely
 */
dotcmdbegin()
{
	/* never record a playback */
	if (kbdmode == PLAY)
		return FALSE;

	switch (dotcmdmode) {
	case TMPSTOP:
	case PLAY:
		return FALSE;
	}
	tmpcmdptr = &tmpcmdm[0];
	tmpcmdend = tmpcmdptr;
	dotcmdmode = RECORD;
	return TRUE;
}

/*
 * Finish dot command, and copy it to the real holding area
 */
dotcmdfinish()
{

	switch (dotcmdmode) {
	case STOP:
	case PLAY:
	case TMPSTOP:
		return FALSE;

	case RECORD:
		;
	}
	tmpcmdptr = &tmpcmdm[0];
	dotcmdptr = &dotcmdm[0];
	while (tmpcmdptr < tmpcmdend)
		*dotcmdptr++ = *tmpcmdptr++;
	dotcmdend = dotcmdptr;
	dotcmdptr = &dotcmdm[0];
	dotcmdmode = STOP;
	return TRUE;
}

/* stop recording a dot command, 
	probably because the command is not re-doable */ 
dotcmdstop()
{
	if (dotcmdmode == RECORD)
		dotcmdmode = STOP;
}

/*
 * Execute a the '.' command, by putting us in PLAY mode.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
dotcmdplay(f, n)
int f,n;
{
	if (!f)
		n = 1;
	else if (n <= 0)
		return TRUE;
	dotcmdrep = n;		/* remember how many times to execute */
	dotcmdmode = PLAY;		/* put us in play mode */
	dotcmdptr = &dotcmdm[0];	/*    at the beginning */

	return TRUE;
}
/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
/* ARGSUSED */
kbd_mac_begin(f, n)
int f,n;
{
	if (kbdmode != STOP) {
		mlforce("[Macro already active]");
		return FALSE;
	}
	mlwrite("[Start macro]");
	kbdptr = &kbdm[0];
	kbdend = kbdptr;
	kbdmode = RECORD;
	kbdplayreg = -1;  /* default buffer */
	return TRUE;
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
/* ARGSUSED */
kbd_mac_end(f, n)
int f,n;
{
	if (kbdmode == STOP) {
		mlforce("[Macro not active]");
		return FALSE;
	}
	if (kbdmode == RECORD) {
		mlwrite("[End macro]");
		kbdmode = STOP;
		kbdplayreg = -1;  /* default buffer */
		kbdlim = kbdend;
	}
	/* note that if kbd_mode == PLAY, we do nothing -- that makes
		the '^X-)' at the of the recorded buffer a no-op during
		playback */
	return TRUE;
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
/* ARGSUSED */
kbd_mac_exec(f, n)
int f,n;
{
	if (kbdmode != STOP) {
		mlforce("[Macro already active]");
		return FALSE;
	}
	if (n <= 0)
		return TRUE;
	kbdrep = n;		/* remember how many times to execute */
	kbdmode = PLAY; 	/* start us in play mode */
	kbdplayreg = -1;	/* default playback register */
	kbdptr = &kbdm[0];	/*    at the beginning */
	kbdend = kbdlim;
	dotcmdmode = STOP;
	return TRUE;
}

/* ARGSUSED */
kbd_mac_save(f,n)
int f,n;
{
	register unsigned char *kp;
	kdelete();
	for (kp = kbdm; kp < kbdlim; kp++)
		kinsert(*kp);
	mlwrite("[Keyboard macro saved.]");
	return TRUE;
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
/* ARGSUSED */
esc(f, n)
int f,n;
{
	TTbeep();
	dotcmdmode = STOP;
	fulllineregions = FALSE;
	doingopcmd = FALSE;
	opcmd = 0;
	mlforce("[Aborted]");
	return ABORT;
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode				*/

rdonly()
{
	TTbeep();
	mlforce("[No changes are allowed while in \"view\" mode]");
	return FALSE;
}

/* ARGSUSED */
showversion(f,n)
int f,n;
{
	mlforce(version);
	return TRUE;
}

/* ARGSUSED */
unimpl(f,n)
int f,n;
{
	TTbeep();
	mlforce("[Sorry, that vi command is unimplemented in vile ]");
	return FALSE;
}

opercopy(f,n) int f,n; { return unimpl(f,n); }
opermove(f,n) int f,n; { return unimpl(f,n); }
opertransf(f,n) int f,n; { return unimpl(f,n); }

operglobals(f,n) int f,n; { return unimpl(f,n); }
opervglobals(f,n) int f,n; { return unimpl(f,n); }

map(f,n) int f,n; { return unimpl(f,n); }
unmap(f,n) int f,n; { return unimpl(f,n); }

source(f,n) int f,n; { return unimpl(f,n); }

visual(f,n) int f,n; { return unimpl(f,n); }
ex(f,n) int f,n; { return unimpl(f,n); }

/* ARGSUSED */
nullproc(f,n)	/* user function that does (almost) NOTHING */
int f,n;
{
	return TRUE;
}

cntl_af()	/* dummy function for binding to control-a prefix */
{
}

cntl_xf()	/* dummy function for binding to control-x prefix */
{
}

unarg() /* dummy function for binding to universal-argument */
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
		_chartypes_['/'] = _pathn;

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
		_chartypes_[c] |= _lower|_pathn|_ident;

	/* uppercase */
	for (c = 'A'; c <= 'Z'; c++)
		_chartypes_[c] |= _upper|_pathn|_ident;

	/* digits */
	for (c = '0'; c <= '9'; c++)
		_chartypes_[c] |= _digit|_pathn|_ident|_linespec;

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
	_chartypes_['`'] |= _wild;

	/* ex mode line specifiers */
	_chartypes_[','] |= _linespec;
	_chartypes_['%'] |= _linespec;
	_chartypes_['-'] |= _linespec;
	_chartypes_['+'] |= _linespec;
	_chartypes_['.'] |= _linespec;
	_chartypes_['$'] |= _linespec;
	_chartypes_['\''] |= _linespec;

	/* fences */
	_chartypes_['('] |= _fence;
	_chartypes_[')'] |= _fence;
	_chartypes_['['] |= _fence;
	_chartypes_[']'] |= _fence;
	_chartypes_['{'] |= _fence;
	_chartypes_['}'] |= _fence;

	for (c = 0; c < N_chars; c++)
			if ((_chartypes_[c] & _space) == 0)
					_chartypes_[c] |= _nonspace;

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
	return dst;
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

	return mp;
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
