/*
 * This file contains the command processing functions for a number of random
 * commands. There is no functional grouping here, for sure.
 *
 * $Log: random.c,v $
 * Revision 1.30  1991/10/18 10:56:54  pgf
 * added code to use modified VALUE structures and lists to display settings
 * more easily (adjvalueset).  also removed some old ifdefs.
 *
 * Revision 1.29  1991/10/15  12:01:20  pgf
 * added backspacelimit support, and fewer tty chars are hard-coded now
 *
 * Revision 1.28  1991/10/08  01:09:38	pgf
 * added ^W and ^U processing to ins().  should make this depend on the tty
 * settings, and add option to control whether it's okay to backspace or
 * line or word kill back beyond the insert point
 *
 * Revision 1.27  1991/09/30  01:47:24	pgf
 * reformat listing of values a bit
 *
 * Revision 1.26  1991/09/26  13:13:07	pgf
 * created window values, and allow them to be displayed and set.  still needs
 * cleaning up
 *
 * Revision 1.25  1991/09/24  02:07:29	pgf
 * suppressed extra whitespace in listmodes
 *
 * Revision 1.24  1991/09/24  01:03:49	pgf
 * pass FALSE to gotoeol to avoid unnecessary work
 *
 * Revision 1.23  1991/09/23  01:57:20	pgf
 * n
 * don't pass null pointers to strcmp()
 *
 * Revision 1.22  1991/09/20  13:11:53	pgf
 * add local value settings to the listmodes output
 *
 * Revision 1.21  1991/09/19  13:39:53	pgf
 * added shortname synonyms to mode and value names
 *
 * Revision 1.20  1991/09/10  01:21:34	pgf
 * re-tabbed
 *
 * Revision 1.19  1991/09/10  00:43:31	pgf
 * ifdef'ed out obsolete "showm", and change calls to it to "listmodes"
 *
 * Revision 1.18  1991/08/16  11:12:26	pgf
 * added the third flavor of insertmode for replace-char, and
 * added support for the insertmode indicator on the modeline, and
 * created the catnap routine for use by fmatch, and the typahead
 * check in the ANSI_SPEC code in kbd_key
 *
 * Revision 1.17  91/08/13	12:51:29  pgf
 * make sure we pass a non-NULL arg to poll, even though the count is zero
 * 
 * Revision 1.16  1991/08/13  02:52:25	pgf
 * the fmatch() code now works if you have poll or select, and
 * is enabled by "set showmatch"
 *
 * Revision 1.15  1991/08/12  15:06:21	pgf
 * added ANSI_SPEC capability -- can now use the arrow keys from
 * command or insert mode
 *
 * Revision 1.14  1991/08/08  13:21:25	pgf
 * removed ifdef BEFORE
 *
 * Revision 1.13  1991/08/07  12:35:07	pgf
 * added RCS log messages
 *
 * revision 1.12
 * date: 1991/08/06 15:25:02;
 *	global/local values
 * and list changes
 * 
 * revision 1.11
 * date: 1991/06/26 09:35:48;
 * made count work correctly on flipchar, and
 * removed old ifdef BEFORE stuff
 * 
 * revision 1.10
 * date: 1991/06/25 19:53:09;
 * massive data structure restructure
 * 
 * revision 1.9
 * date: 1991/06/20 17:23:24;
 * fixed & --> && problem in indent_newline
 * 
 * revision 1.8
 * date: 1991/06/16 17:38:02;
 * switched to modulo tab calculations, and
 * converted entab, detab, and trim to work on regions,
 * and stripped some old ifdef NOCOUNT stuff for openup and opendown
 * and added support for local and globla tabstop and fillcol values
 * 
 * revision 1.7
 * date: 1991/06/15 09:09:27;
 * changed some forwchar calls to forwchar_to_eol, and
 * now prevent 'x', 's', 'r' from destroying the newline when used on
 * empty lines
 * 
 * revision 1.6
 * date: 1991/06/06 13:58:23;
 * added autoindent mode and "set all"
 * 
 * revision 1.5
 * date: 1991/05/31 11:19:06;
 * added showlength function, for "=" ex command, and
 * switched from stutterfunc to godotplus
 * 
 * revision 1.4
 * date: 1991/04/04 09:40:25;
 * fixed autoinsert bug
 * 
 * revision 1.3
 * date: 1991/02/12 09:49:33;
 * doindent had an unset return value, causing autoindents of 0 chars to fail
 * 
 * revision 1.2
 * date: 1990/10/03 16:01:00;
 * make backspace work for everyone
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:54;
 * initial vile RCS revision
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
#if UNIX
#include	<signal.h>
#endif
#if HAVE_SELECT
#include <sys/types.h>
#include <sys/time.h>
#else
# if HAVE_POLL
# include <poll.h>
# endif
#endif

extern CMDFUNC f_forwchar, f_backchar, f_forwchar_to_eol, f_backchar_to_bol;


/* generic "lister", which takes care of popping a window/buffer pair under
	the given name, and calling "func" with a couple of args to fill in
	the buffer */
liststuff(name,func,iarg,carg)
char *name;
int (*func)();		/* ptr to function to execute */
int iarg;
char *carg;
{
	register BUFFER *bp;
	register int	s;

	/* create the buffer list buffer   */
	bp = bfind(name, OK_CREAT, BFSCRTCH);
	if (bp == NULL)
		return FALSE;
	    
	if ((s=bclear(bp)) != TRUE) /* clear old text (?) */
		return (s);
	bp->b_flag |= BFSCRTCH;
	s = TRUE;
	if (!s || popupbuff(bp) == FALSE) {
		mlwrite("[Sorry, can't list. ]");
		zotbuf(bp);
		return (FALSE);
	}
	/* call the passed in function, giving it both the integer and 
		character pointer arguments */
	(*func)(iarg,carg);
	gotobob(FALSE,1);
	lsprintf(bp->b_fname, "       %s   %s",prognam,version);
	bp->b_flag &= ~BFCHG;
	bp->b_active = TRUE;
	make_local_b_val(bp,MDVIEW);
	set_b_val(bp,MDVIEW,TRUE);
	make_local_b_val(bp,VAL_TAB);
	set_b_val(bp,VAL_TAB,8);
	make_local_b_val(bp,MDDOS);
	set_b_val(bp,MDDOS,FALSE);

	return TRUE;
}

listmodes(f, n)
{
	int makemodelist();
	register WINDOW *wp = curwp;
	register int s;

	s = liststuff("[Settings]",makemodelist,0,wp);
	/* back to the buffer whose modes we just listed */
	swbuffer(wp->w_bufp);
	return s;
}


/* list the current modes into the current buffer */
makemodelist(dum1,ptr)
int dum1;
char *ptr;
{
	register WINDOW *localwp = (WINDOW *)ptr;
	register BUFFER *localbp = localwp->w_bufp;
	bprintf("--- \"%s\" settings, if different than globals %*P\n",
			localbp->b_bname, term.t_ncol-1, '-');
	listvalueset(b_valuenames, localbp->b_values.bv, global_b_values.bv);
	bputc('\n');
	listvalueset(w_valuenames, localwp->w_values.wv, global_w_values.wv);
	bprintf("--- Global settings %*P\n", term.t_ncol-1, '-');
	listvalueset(b_valuenames, global_b_values.bv, NULL);
	bputc('\n');
	listvalueset(w_valuenames, global_w_values.wv, NULL);
#if LAZY
	lsprintf(line," lazy filename matching is %s",
					(othmode & OTH_LAZY) ? "on":"off");
	if (addline(blistp,line,-1) == FALSE)
		return(FALSE);
#endif
}

/* listvalueset: print each value in the array according to type,
	along with its name, until a NULL name is encountered.  Only print
	if the value in the two arrays differs, or the second array is nil */
listvalueset(names, values, globvalues)
struct VALNAMES *names;
struct VAL *values, *globvalues;
{
	register int j;
	j = 0;
	while(names->name != NULL) {
		switch(names->type) {
		case VALTYPE_BOOL:
			if (!globvalues || values->vp->i != globvalues->v.i) {
				bprintf("%s%s%*P", values->vp->i ? "":"no",
					names->name, 26, ' ');
				j++;
			}
			break;
		case VALTYPE_INT:
			if (!globvalues || values->vp->i != globvalues->v.i) {
				bprintf("%s=%d%*P", names->name,
					values->vp->i, 26, ' ');
				j++;
			}
			break;
		case VALTYPE_STRING:
			if (!globvalues || ( values->vp->p && globvalues->v.p &&
				(strcmp( values->vp->p, globvalues->v.p)))) {
				bprintf("%s=%s%*P", names->name,
					values->vp->p ? values->vp->p : "",
					26, ' ');
				j++;
			}
			break;
		default:
			mlwrite("BUG: bad type %s %d",names->name,names->type);
			return FALSE;
		}
		if (j && j % 3 == 0) { /* 3 per line */
			bputc('\n');
			j = 0;
		}
		names++;
		values++;
		if (globvalues) globvalues++;
	}
	if (j % 3 != 0)
		bputc('\n');
}
/*
 * Set fill column to n.
 */
setfillcol(f, n)
{
	if (f) {
		make_local_b_val(curbp,VAL_FILL);
		set_b_val(curbp,VAL_FILL,n);
	}
	mlwrite("[Fill column is %d%s]",n, is_global_b_val(curbp,VAL_FILL) ?
						" (global)" : " (local)" );
	return(TRUE);
}

/*
 * Display the current position of the cursor, lines and columns, in the file,
 * the character that is under the cursor (in hex), and the fraction of the
 * text that is before the cursor. The displayed column is not the current
 * column, but the column that would be used on an infinite width display.
 */
showcpos(f, n)
{
	register LINE	*lp;		/* current line */
	register long	numchars;	/* # of chars in file */
	register int	numlines;	/* # of lines in file */
	register long	predchars;	/* # chars preceding point */
	register int	predlines;	/* # lines preceding point */
	register int	curchar;	/* character under cursor */
	int ratio;
	int col;
	int savepos;			/* temp save for current offset */
	int ecol;			/* column pos/end of current line */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_line.l);

	/* start counting chars and lines */
	numchars = 0;
	numlines = 0;
	while (lp != curbp->b_line.l) {
		/* if we are on the current line, record it */
		if (lp == DOT.l) {
			predlines = numlines;
			predchars = numchars + DOT.o;
			if (DOT.o == llength(lp))
				curchar = '\n';
			else
				curchar = char_at(DOT);
		}
		/* on to the next line */
		++numlines;
		numchars += llength(lp) + 1;
		lp = lforw(lp);
	}

	/* if at end of file, record it */
	if (is_header_line(DOT,curbp)) {
		predlines = numlines;
		predchars = numchars;
	}

	/* Get real column and end-of-line column. */
	col = getccol(FALSE);
	savepos = DOT.o;
	DOT.o = llength(DOT.l);
	ecol = getccol(FALSE);
	DOT.o = savepos;

	ratio = 0;		/* Ratio before dot. */
	if (numchars != 0)
		ratio = (100L*predchars) / numchars;

	/* summarize and report the info */
	mlwrite(
"Line %d of %d, Col %d of %d, Char %D of %D (%d%%) char is 0x%x",
		predlines+1, numlines, col+1, ecol,
		predchars+1, numchars, ratio, curchar);
	return TRUE;
}

showlength(f,n)
{
	register LINE	*lp;		/* current line */
	register int	numlines = 0;	/* # of lines in file */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_line.l);
	while (lp != curbp->b_line.l) {
		++numlines;
		lp = lforw(lp);
	}
	mlwrite("%d",numlines);
	return TRUE;
}

#if ! SMALLER
getcline()	/* get the current line number */
{
	register LINE	*lp;		/* current line */
	register int	numlines;	/* # of lines before point */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_line.l);

	/* start counting lines */
	numlines = 0;
	while (lp != curbp->b_line.l) {
		/* if we are on the current line, record it */
		if (lp == DOT.l)
			break;
		++numlines;
		lp = lforw(lp);
	}

	/* and return the resulting count */
	return(numlines + 1);
}
#endif

/*
 * Return current screen column.  Stop at first non-blank given TRUE argument.
 */
getccol(bflg)
int bflg;
{
	register int c, i, col;
	col = 0;
	for (i=0; i < DOT.o; ++i) {
		c = lgetc(DOT.l, i);
		if (!isspace(c) && bflg)
			break;
		col = next_column(c,col);
	}
	return col;
}


/*
 * Set current column.
 */
gotocol(f,n)
{
	register int c; 	/* character being scanned */
	register int i; 	/* index into current line */
	register int col;	/* current cursor column   */
	register int llen;	/* length of line in bytes */

	col = 0;
	llen = llength(DOT.l);
	if ( n <= 0) n = 1;

	/* scan the line until we are at or past the target column */
	for (i = 0; i < llen; ++i) {
		/* upon reaching the target, drop out */
		if (col >= n)
			break;

		/* advance one character */
		c = lgetc(DOT.l, i);
		col = next_column(c,col);
	}

	/* set us at the new position */
	DOT.o = i;

	/* and tell whether we made it */
	return(col >= n);
}

#if ! SMALLER
/*
 * Twiddle the two characters on either side of dot. If dot is at the end of
 * the line twiddle the two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit pointless to make this
 * work. This fixes up a very common typo with a single stroke.
 * This always works within a line, so "WFEDIT" is good enough.
 */
twiddle(f, n)
{
	MARK		dot;
	register int	cl;
	register int	cr;

	dot = DOT;
	if (is_empty_line(dot))
		return (FALSE);
	--dot.o;
	cr = char_at(dot);
	if (--dot.o < 0)
		return (FALSE);
	cl = char_at(dot);
	copy_for_undo(dot.l);
	put_char_at(dot, cr);
	++dot.o;
	put_char_at(dot, cl);
	lchange(WFEDIT);
	return (TRUE);
}
#endif

/*
 * Quote the next character, and insert it into the buffer. All the characters
 * are taken literally, with the exception of the newline, which always has
 * its line splitting meaning. The character is always read, even if it is
 * inserted 0 times, for regularity.
 */
quote(f, n)
{
	register int	s;
	register int	c;

	c = tgetc();
	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	if (c == '\n') {
		do {
			s = lnewline();
		} while (s==TRUE && --n);
		return s;
	}
	return linsert(n, c);
}

replacechar(f, n)
{
	register int	s;
	register int	c;

	if (!f && llength(DOT.l) == 0)
		return FALSE;

	insertmode = REPLACECHAR;  /* need to fool the SPEC prefix code */
	curwp->w_flag |= WFMODE;
	update(FALSE);
	c = kbd_key();
	insertmode = FALSE;
	curwp->w_flag |= WFMODE;

	if (n < 0)
		return FALSE;
	if (n == 0)
		return TRUE;
	if (c == abortc)
		return FALSE;

	ldelete((long)n,FALSE);
	if (c == quotec) {
		return(quote(f,n));
	}
	c = kcod2key(c);
	if (c == '\n' || c == '\r') {
		do {
			s = lnewline();
		} while (s==TRUE && --n);
		return s;
	} else if (isbackspace(c))
		s = TRUE;
	else
		s = linsert(n, c);
	if (s == TRUE)
		s = backchar(FALSE,1);
	return s;
}

/*
 * Set tab size
 */
settab(f, n)
{
	register WINDOW *wp;
	if (f && n >= 1) {
		make_local_b_val(curbp,VAL_TAB);
		set_b_val(curbp,VAL_TAB,n);
		curtabstopval = n;
		for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
			wp->w_flag |= WFHARD;
		refresh(FALSE,1);
	} else if (f) {
		mlwrite("Sorry, illegal tabstop value");
		TTbeep();
		return FALSE;
	}
	mlwrite("Tabs are %d columns apart%s",TABVAL,
		is_global_b_val(curbp,VAL_TAB) ? " (global)" : " (local)" );
	return TRUE;
}

/* insert a tab into the file */
tab(f, n)
{
	return linsert(1, '\t');
}

#if AEDIT

/*
 * change all tabs in the line to the right number of spaces
 */
detabline()
{
	register int	s;

	/* detab the entire current line */
	while (DOT.o < llength(DOT.l)) {
		/* if we have a tab */
		if (char_at(DOT) == '\t') {
			if ((s = ldelete(1L, FALSE)) != TRUE)
				return s;
			insspace( TRUE, TABVAL - (DOT.o % TABVAL) );
		}
		DOT.o++;
	}
	return TRUE;
}

/*
 * change all tabs in the region to the right number of spaces
 */
detab_region(f, n)
{
	return do_fl_region(detabline);
}

/*
 * convert all appropriate spaces in the line to tab characters
 */
entabline()
{
	register int fspace;	/* pointer to first space if in a run */
	register int ccol;	/* current cursor column */
	register char cchar;	/* current character */

	detabline();	/* get rid of possible existing tabs */
	DOT.o = 0;

	/* entab the entire current line */
	/* would this have been easier if it had started at
		the _end_ of the line, rather than the beginning?  -pgf */
	fspace = -1;
	ccol = 0;
	while (DOT.o < llength(DOT.l)) {
		/* see if it is time to compress */
		if ((fspace >= 0) && (nextab(fspace) <= ccol))
			if (ccol - fspace < 2)
				fspace = -1;
			else {
				backchar(TRUE, ccol - fspace);
				ldelete((long)(ccol - fspace), FALSE);
				linsert(1, '\t');	    
				fspace = -1;
			}

		/* get the current character */
		cchar = char_at(DOT);

		if (cchar == ' ') { /* a space...compress? */
			if (fspace == -1)
				fspace = ccol;
		} else {
			fspace = -1;
		}
		ccol++;
		DOT.o++;
	}
	return TRUE;
}

/*
 * convert all appropriate spaces in the region to tab characters
 */
entab_region(f, n)
{
	return do_fl_region(entabline);
}

/* trim trailing whitespace from a line.  leave dot at end of line */
trimline()
{
	register int off, orig;
	register LINE *lp;
	    
	lp = DOT.l;
		    
	off = llength(lp)-1;
	orig = off;
	while (off >= 0) {
		if (!isspace(lgetc(lp,off)))
			break;
		off--;
	}
	    
	if (off == orig)
		return TRUE;

	DOT.o = off+1;
		    
	return ldelete(orig - off,FALSE);
}

/*
 * trim trailing whitespace from a region
 */
trim_region(f, n)
{
	return do_fl_region(trimline);
}

#endif


/* open lines up before this one */
openup(f,n)
{
	int backline();
	int s;
	gotobol(TRUE,1);
	s = lnewline();
	if (s != TRUE)
		return(s);
	s = backline(TRUE,1);
	if (s != TRUE)
		return(s);
	/* there's a bug here with counts.	I don't particularly care
		right now.	*/
	return(opendown(f,n-1));
}

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them.
 */

/* open lines up after this one */
opendown(f,n)
{
	register int	i;
	register int	s;

	if (n < 0)
		return (FALSE);
	if (n == 0)
		return ins(f,n);

	i = n;					/* Insert newlines. */
	do {
		gotoeol(FALSE,1);
		s = newline(TRUE,1);
	} while (s==TRUE && --i);
	if (s == TRUE)				/* Then back up overtop */
		s = backline(TRUE, n-1);	/* of them all.		 */

	curgoal = -1;

	if (s != TRUE)
		return (s);

	return(ins(f,n));
}

/*
 * Go into insert mode.  I guess this isn't emacs anymore...
 */
insert(f, n)
{
	return ins(f,n);
}

insertbol(f, n)
{
	firstnonwhite(f,n);
	return ins(f,n);
}

append(f, n)
{
	if (DOT.o != llength(DOT.l)) /* END OF LINE HACK */
		forwchar(TRUE,1);
	return ins(f,n);
}

appendeol(f, n)
{
	gotoeol(FALSE,0);
	return ins(f,n);
}

overwrite(f, n)
{
	insertmode = OVERWRITE;
	curwp->w_flag |= WFMODE;
	return ins(f,n);
}

/* grunt routine for insert mode */
ins(f,n)
{
	register int status;
	int (*execfunc)();		/* ptr to function to execute */
	int    c;		/* command character */
	extern int quote(), backspace(), tab(), newline(), nullproc();
	int newlineyet = FALSE; /* are we on the line we started on? */
	int startoff = DOT.o;	/* starting offset on that line */

	if (insertmode == FALSE) {
	insertmode = INSERT;
	curwp->w_flag |= WFMODE;
	}

	/* get the next command from the keyboard */
	while(1) {

		update(FALSE);

		f = FALSE;
		n = 1;

		c = kbd_key();

		if (c == abortc ) {
			 /* an unfortunate Vi-ism that ensures one 
				can always type "ESC a" if you're not sure 
				you're in insert mode. */
			if (DOT.o != 0)
				backchar(TRUE,1);
			if (autoindented >= 0) {
				trimline(FALSE,1);
				autoindented = -1;
			}
			insertmode = FALSE;
			curwp->w_flag |= WFMODE;
			return (TRUE);
		} else if (c == -abortc ) {
			/* we use the negative to suppress that
				junk, for the benefit of SPEC keys */
			insertmode = FALSE;
			curwp->w_flag |= WFMODE;
			return (TRUE);
		}

		execfunc = NULL;
		if (c == quotec) {
			execfunc = quote;
		} else {
			/*
			 * If a space was typed, fill column is defined, the
			 * argument is non- negative, wrap mode is enabled, and
			 * we are now past fill column, perform word wrap. 
			 */
			if (isspace(c) && b_val(curwp->w_bufp,MDWRAP) &&
					fillcol > 0 && n >= 0 &&
					getccol(FALSE) > fillcol )
				wrapword();

				/* ^D and ^T are aliased to ^H and tab, for 
					users accustomed to "shiftwidth" */
				if ( c ==  tocntrl('I') ||
					c ==  tocntrl('T')) { /* tab and ^T */
					execfunc = tab;
					autoindented = -1;
				} else if (c ==  tocntrl('J') ||
					c ==  tocntrl('M')) { /* CR and NL */
					execfunc = newline;
					if (autoindented >= 0) {
						trimline(FALSE,1);
						autoindented = -1;
					}
					newlineyet = TRUE;
				} else if ( isbackspace(c) ||
						c == tocntrl('D') || 
						c == killc ||
						c == wkillc) { /* ^U and ^W */
					/* how far can we back up? */
					register int backlimit;
					/* have we backed thru a "word" yet? */
					int saw_word = FALSE;
					if (newlineyet ||
						!b_val(curbp,MDBACKLIMIT))
						backlimit = 0;
					else
						backlimit = startoff;
					while (DOT.o > backlimit) {
						if (c == wkillc) {
							if (isspace(
							lgetc(DOT.l,DOT.o-1))) {
								if (saw_word)
									break;
							} else {
								saw_word = TRUE;
							}
						}
						backspace();
						autoindented--;
						if (isbackspace(c) ||
							c == tocntrl('D'))
							break;
					}
					execfunc = nullproc;
#if UNIX && defined(SIGTSTP)	/* job control, ^Z */
				} else if (c == suspc) {
					extern int bktoshell();
					execfunc = bktoshell;

#endif
				} else if (c == startc ||
						c == stopc) {  /* ^Q and ^S */
					execfunc = nullproc;
			}
		}

		if (execfunc != NULL) {
			status	 = (*execfunc)(f, n);
			if (status != TRUE) {
				insertmode = FALSE;
				curwp->w_flag |= WFMODE;
				return (status);
			}
			continue;
		}

		    
		/* make it a real character again */
		c = kcod2key(c);

		/* if we are in overwrite mode, not at eol,
		   and next char is not a tab or we are at a tab stop,
		   delete a char forword			*/
		if (insertmode == OVERWRITE &&
				DOT.o < llength(DOT.l) &&
				(char_at(DOT) != '\t' ||
					(DOT.o) % TABVAL == TABVAL-1)) {
			autoindented = -1;
			ldelete(1L, FALSE);
		}

		/* do the appropriate insertion */
		if ((c == RBRACE) && b_val(curbp, MDCMOD)) {
			status = insbrace(n, c);
		} else if (c == '#' && b_val(curbp, MDCMOD)) {
			status = inspound();
		} else {
			autoindented = -1;
			status = linsert(n, c);
		}

#if CFENCE
		/* check for CMODE fence matching */
		if ((c == RBRACE || c == ')' || c == ']') && 
						b_val(curbp, MDSHOWMAT))
			fmatch(c);
#endif

		/* check auto-save mode */
		if (b_val(curbp, MDASAVE))
			if (--gacount == 0) {
				/* and save the file if needed */
				upscreen(FALSE, 0);
				filesave(FALSE, 0);
				gacount = gasave;
			}

		if (status != TRUE) {
			insertmode = FALSE;
			curwp->w_flag |= WFMODE;
			return (status);
		}
	}
}

backspace()
{
	register int	s;

	if ((s=backchar(TRUE, 1)) == TRUE)
		s = ldelete(1L, FALSE);
	return (s);
}

/*
 * Insert a newline. If we are in CMODE, do automatic
 * indentation as specified.
 */
newline(f, n)
{
	register int	s;

	if (n < 0)
		return (FALSE);

#if LATER	/* already done for autoindented != 0 in ins() */
	if (b_val(curbp, MDTRIM))
		trimline(f,n);
#endif
	    
	/* if we are in C or auto-indent modes and this is a default <NL> */
	if (n == 1 && (b_val(curbp,MDCMOD) || b_val(curbp,MDAIND)) &&
						!is_header_line(DOT,curbp))
		return indented_newline(b_val(curbp, MDCMOD));

	/*
	 * If a newline was typed, fill column is defined, the argument is non-
	 * negative, wrap mode is enabled, and we are now past fill column,
	 * perform word wrap.
	 */
	if (b_val(curwp->w_bufp, MDWRAP) && fillcol > 0 &&
						getccol(FALSE) > fillcol)
		wrapword();

	/* insert some lines */
	while (n--) {
		if ((s=lnewline()) != TRUE)
			return (s);
		curwp->w_flag |= WFINS;
	}
	return (TRUE);
}

/* insert a newline and indentation for C */
indented_newline(cmode)
{
	register int indentwas; /* indent to reproduce */
	int bracef; /* was there a brace at the end of line? */
	    
	indentwas = previndent(&bracef);
	if (lnewline() == FALSE)
		return FALSE;
	if (cmode && bracef)
		indentwas = nextab(indentwas);
	if (doindent(indentwas) != TRUE)
		return FALSE;
	return TRUE;
}

/* get the indent of the last previous non-blank line.	also, if arg
	is non-null, check if line ended in a brace */
int
previndent(bracefp)
int *bracefp;
{
	int ind;
	    
	MK = DOT;
	    
	if (backword(FALSE,1) == FALSE) {
		if (bracefp) *bracefp = FALSE;
		gomark();
		return 0;
	}
	ind = indentlen(DOT.l);
	if (bracefp)
		*bracefp = (llength(DOT.l) > 0 &&
			lgetc(DOT.l,llength(DOT.l)-1) == '{');
		    
	gomark();
	    
	return ind;
}

doindent(ind)
{
	int i;
	/* if no indent was asked for, we're done */
	if (ind <= 0)
		return TRUE;
	autoindented = 0;
	if ((i=ind/TABVAL)!=0) {
		autoindented += i;
		if (linsert(i, '\t') == FALSE)
			return FALSE;
	}
	if ((i=ind%TABVAL) != 0) {
		autoindented += i;
		if (linsert(i,	' ') == FALSE)
			return FALSE;
	}
	if (!autoindented)
		autoindented = -1;
	    
	return TRUE;
}

/* return the column indent of the specified line */
int
indentlen(lp)
LINE *lp;
{
	register int ind, i, c;
	ind = 0;
	for (i=0; i<llength(lp); ++i) {
		c = lgetc(lp, i);
		if (!isspace(c))
			break;
		if (c == '\t')
			ind = nextab(ind);
		else
			++ind;
	}
	return ind;
}

insbrace(n, c)	/* insert a brace into the text here...we are in CMODE */
int n;	/* repeat count */
int c;	/* brace to insert (always { for now) */
{
	register int ch;	/* last character before input */
	register int i;
	register int target;	/* column brace should go after */

#if ! CFENCE
	/* wouldn't want to back up from here, but fences might take us 
		forward */
	/* if we are at the beginning of the line, no go */
	if (DOT.o == 0)
		return(linsert(n,c));
#endif

	if (autoindented >= 0) {
		trimline(FALSE,1);
	}
	else {
		return linsert(n,c);
	}
#if ! CFENCE /* no fences?	then put brace one tab in from previous line */
	doindent(((previndent(NULL)-1) / TABVAL) * TABVAL);
#else /* line up brace with the line containing its match */
	doindent(fmatchindent());
#endif
	autoindented = -1;

	/* and insert the required brace(s) */
	return(linsert(n, c));
}

inspound()	/* insert a # into the text here...we are in CMODE */
{
	register int ch;	/* last character before input */
	register int i;

	/* if we are at the beginning of the line, no go */
	if (DOT.o == 0)
		return(linsert(1,'#'));

	if (autoindented > 0) { /* must all be whitespace before us */
		DOT.o = 0;
		ldelete(autoindented,FALSE);
	}
	autoindented = -1;

	/* and insert the required pound */
	return(linsert(1, '#'));
}

#if AEDIT
/*
 * Delete blank lines around dot. What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a blank line, this command
 * deletes all the blank lines above and below the current line. If it is
 * sitting on a non blank line then it deletes all of the blank lines after
 * the line. Any argument is ignored.
 */
deblank(f, n)
{
	register LINE	*lp1;
	register LINE	*lp2;
	long nld;

	lp1 = DOT.l;
	while (llength(lp1)==0 && (lp2=lback(lp1))!=curbp->b_line.l)
		lp1 = lp2;
	lp2 = lp1;
	nld = 0;
	while ((lp2=lforw(lp2))!=curbp->b_line.l && llength(lp2)==0)
		++nld;
	if (nld == 0)
		return (TRUE);
	DOT.l = lforw(lp1);
	DOT.o = 0;
	return (ldelete(nld, FALSE));
}

#endif

/* '~' is synonymous with 'M-~<space>' */
flipchar(f, n)
{
	int s;

	havemotion = &f_forwchar_to_eol;
	s = operflip(f,n);
	if (s == TRUE)
		return forwchar_to_eol(f,n);
}

/* 'x' is synonymous with 'd<space>' */
forwdelchar(f, n)
{

	havemotion = &f_forwchar_to_eol;
	return operdel(f,n);
}

/* 'X' is synonymous with 'd<backspace>' */
backdelchar(f, n)
{
	havemotion = &f_backchar_to_bol;
	return operdel(f,n);
}

/* 'D' is synonymous with 'd$' */
deltoeol(f, n)
{
	extern CMDFUNC f_gotoeol;

	if (llength(DOT.l) == 0)
		return TRUE;

	havemotion = &f_gotoeol;
	return operdel(FALSE,1);
}

/* 'C' is synonymous with 'c$' */
chgtoeol(f, n)
{
	extern CMDFUNC f_gotoeol;

	if (llength(DOT.l) == 0) {
		return ins(f,n);
	} else {
		havemotion = &f_gotoeol;
		return operchg(FALSE,1);
	}
}

/* 'Y' is synonymous with 'yy' */
yankline(f, n)
{
	extern CMDFUNC f_godotplus;
	havemotion = &f_godotplus;
	return(operyank(f,n));
}

/* 'S' is synonymous with 'cc' */
chgline(f, n)
{
	extern CMDFUNC f_godotplus;
	havemotion = &f_godotplus;
	return(operchg(f,n));
}

/* 's' is synonymous with 'c<space>' */
chgchar(f, n)
{
	havemotion = &f_forwchar_to_eol;
	return(operchg(f,n));
}

setmode(f, n)	/* prompt and set an editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(TRUE, FALSE);
}

delmode(f, n)	/* prompt and delete an editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(FALSE, FALSE);
}

setgmode(f, n)	/* prompt and set a global editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(TRUE, TRUE);
}

delgmode(f, n)	/* prompt and delete a global editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(FALSE, TRUE);
}


adjustmode(kind, global)	/* change the editor mode status */
int kind;	/* true = set,		false = delete */
int global; /* true = global flag,	false = current buffer flag */
{
	register char *scan;		/* scanning pointer to convert prompt */
	register int i; 		/* loop index */
	register s;		/* error return on input */
#if COLOR
	register int uflag; 	/* was modename uppercase?	*/
#endif
	int no = 0;
	int okind;
	char prompt[50];	/* string to prompt user with */
	static char cbuf[NPAT]; 	/* buffer to recieve mode name into */

	/* build the proper prompt string */
	if (global)
		strcpy(prompt,"Global value: ");
	else
		strcpy(prompt,"Local value: ");


	/* prompt the user and get an answer */

	s = mlreply(prompt, cbuf, NPAT - 1);
	if (s != TRUE)
		return(s);

	/* make it lowercase */

	scan = cbuf;
#if COLOR
	uflag = isupper(*scan);
#endif
	while (*scan != 0) {
		if (isupper(*scan))
			*scan = tolower(*scan);
		scan++;
	}

	if (!strcmp(cbuf,"all")) {
		return listmodes(FALSE,1);
	}

	/* test it first against the colors we know */
	for (i=0; i<NCOLORS; i++) {
		if (strcmp(cbuf, cname[i]) == 0) {
			/* finding the match, we set the color */
#if COLOR
			if (uflag)
				if (global)
					gfcolor = i;
				else if (curwp)
					curwp->w_fcolor = i;
			else
				if (global)
					gbcolor = i;
				else if (curwp)
					curwp->w_bcolor = i;

			if (curwp)
				curwp->w_flag |= WFCOLR;
#endif
			mlerase();
			return(TRUE);
		}
	}

	s = adjvalueset(cbuf, kind, b_valuenames,
		global ? global_b_values.bv : curbp->b_values.bv );
	if (s == TRUE)
		goto success;
	s = adjvalueset(cbuf, kind, w_valuenames,
		global ? global_w_values.wv : curwp->w_values.wv );
	if (s == TRUE)
		goto success;
	if (s == FALSE)
		mlwrite("Not a legal setting");
	return s;

success:
	if (global) {
		register WINDOW *wp;
		for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
			wp->w_flag |= WFHARD;
	}

	/* if the settings are up, redisplay them */
	if (bfind("[Settings]", NO_CREAT, BFSCRTCH))
		listmodes(FALSE,1);

	refresh(FALSE,1);
	mlerase();	/* erase the junk */

	return TRUE;
}

adjvalueset(cp, kind, names, values)
char *cp;
int kind;
struct VALNAMES *names;
struct VAL *values;
{
	register int j;
	int okind;
	char respbuf[NFILEN];
	char *rp = NULL;
	int rplen = 0;
	int no;
	int nval, status;
	extern char *strchr();

	no = 0;
	if (strncmp(cp, "no", 2) == 0) {
		okind = kind;
		kind = !kind;
		no = 2;
	}
	if (rp = strchr(cp, '=')) {
		*rp = '\0';
		rp++;
		rplen = strlen(rp);
		if (!rplen)
			goto err;
	}
	j = 0;
	while(names->name != NULL) {
		if (strncmp(cp + no, names->name, strlen(cp) - no) == 0)
			break;
		if (strncmp(cp + no, names->shortname, strlen(cp) - no) == 0)
			break;
		names++;
		values++;
	}
	if (names->name == NULL) {
	err:	/* mlwrite("Not a legal setting"); */
		return FALSE;
	}

	/* we matched a name -- get the value */
	switch(names->type) {
	case VALTYPE_BOOL:
		if (rp) {
			if (!strncmp(rp,"no",rplen) ||
					!strncmp(rp,"false",rplen))
				kind = FALSE;
			else if (!strncmp(rp,"yes",rplen) ||
					!strncmp(rp,"true",rplen))
				kind = TRUE;
			else
				goto err;
		}
		values->vp = &(values->v);
		values->vp->i = kind;
		break;

	case VALTYPE_INT:
	case VALTYPE_STRING:
		if (no) goto err;
		if (!rp) {
			respbuf[0] = '\0';
			status = mlreply("New value: ", respbuf, NFILEN - 1);
			if (status != TRUE)
				return status;
			rp = respbuf;
			rplen = strlen(rp);
			if (!rplen)
				goto err;
		}
		values->vp = &(values->v);
		if (names->type == VALTYPE_INT) {
			nval = 0;
			while (isdigit(*rp))
				nval = (nval * 10) + (*rp++ - '0');
			values->vp->i = nval;
		} else {
			extern char *strmalloc();
			if (values->vp->p)
				free(values->vp->p);
			values->vp->p = strmalloc(rp);
		}
		break;

	default:
		mlwrite("BUG: bad type %s %d",names->name,names->type);
		return FALSE;
	}
	return TRUE;
}

/* Quiet adjust mode, no message line echo.
 * Expects a string to follow: SGover to set global overtype.
 * Prefixes are SG, RG, SL, RL.  Text will be taken until a newline.
 */
#if NeWS
newsadjustmode()	/* change the editor mode status */
{
	register char *scan;		/* scanning pointer to convert prompt */
	register int i; 		/* loop index */
#if COLOR
	register int uflag; 	/* was modename uppercase?	*/
#endif
	char cbuf[NPAT];		/* buffer to recieve mode name into */
	char ch ;
	int kind, global ;

	/* get the mode name and switches */
	kind = ('S' == tgetc()) ;
	global = ('G' == tgetc()) ;
	for (i=0; i<NPAT; i++) {
		if ( '\n' == (ch=tgetc()) ) {
			cbuf[i] = NULL ;
			break ;
		}
		cbuf[i] = ch ;
	}

	/* make it uppercase */
	scan = cbuf;
#if COLOR
	uflag = isupper(*scan);
#endif
	while (*scan != 0) {
		if (islower(*scan))
			*scan = toupper(*scan);
		scan++;
	}

	/* test it first against the colors we know */
	for (i=0; i<NCOLORS; i++) {
		if (strcmp(cbuf, cname[i]) == 0) {
			/* finding the match, we set the color */
#if COLOR
			if (uflag)
				if (global)
					gfcolor = i;
				else if (curwp)
					curwp->w_fcolor = i;
			else
				if (global)
					gbcolor = i;
				else if (curwp)
					curwp->w_bcolor = i;

			if (curwp)
				curwp->w_flag |= WFCOLR;
#endif
			return(TRUE);
		}
	}

	/* test it against the modes we know */
	for (i=0; i < NUMMODES; i++) {
		if (strcmp(cbuf, b_valuenames[i].name) == 0) {
			/* finding a match, we process it */
			if (global) {
				set_global_b_val( i, kind);
			} else if (curbp) {
				make_local_b_val(curbp,i);
				set_b_val(curbp, i, kind);
			}
			/* display new mode line */
			if (curbp)
				upmode();
			return(TRUE);
		}
	}
	return(FALSE);
}
#endif


/*	This function simply clears the message line,
		mainly for macro usage			*/

clrmes(f, n)
int f, n;	/* arguments ignored */
{
	mlforce("");
	return(TRUE);
}

#if ! SMALLER

/*	This function writes a string on the message line
		mainly for macro usage			*/

writemsg(f, n)
int f, n;	/* arguments ignored */
{
	register char *sp;	/* pointer into buf to expand %s */
	register char *np;	/* ptr into nbuf */
	register int status;
	char buf[NPAT]; 	/* buffer to recieve message into */
	char nbuf[NPAT*2];	/* buffer to expand string into */

	buf[0] = 0;
	if ((status = mlreply("Message to write: ", buf, NPAT - 1)) != TRUE)
		return(status);

	/* expand all '%' to "%%" so mlwrite won't expect arguments */
	sp = buf;
	np = nbuf;
	while (*sp) {
		*np++ = *sp;
		if (*sp++ == '%')
			*np++ = '%';
	}
	*np = '\0';

	/* write the message out */
	mlforce(nbuf);
	return(TRUE);
}
#endif

#if CFENCE
/*	the cursor is moved to a matching fence */

getfence(f, n, ch)
int f, n;	/* not used */
int ch; /* fence type to match against */
{
	MARK	oldpos; 	/* original pointer */
	register int sdir;	/* direction of search (1/-1) */
	register int count; /* current fence level count */
	register int ofence;	/* open fence */
	register int c; /* current character in scan */
	int s;

	/* save the original cursor position */
	oldpos = DOT;

	if (!ch) {	/* ch may have been passed, if being used internally */
		/* get the current character */
		if (is_at_end_of_line(oldpos))
			ch = '\n';
		else
			ch = char_at(oldpos);
	}

	/* setup proper matching fence */
	switch (ch) {
		case '(': ofence = ')'; sdir = FORWARD; break;
		case LBRACE: ofence = RBRACE; sdir = FORWARD; break;
		case '[': ofence = ']'; sdir = FORWARD; break;
		case ')': ofence = '('; sdir = REVERSE; break;
		case RBRACE: ofence = LBRACE; sdir = REVERSE; break;
		case ']': ofence = '['; sdir = REVERSE; break;
		default: TTbeep(); return(FALSE);
	}

	/* ops are inclusive of the endpoint */
	if (doingopcmd && sdir == REVERSE) {
		forwchar(TRUE,1);
		MK = DOT;
		backchar(TRUE,1);
	}

	/* set up for scan */
	if (sdir == REVERSE)
		backchar(FALSE, 1);
	else
		forwchar(FALSE, 1);

	count = 1;
	while (count > 0) {
		if (is_at_end_of_line(DOT))
			c = '\n';
		else
			c = char_at(DOT);

		if (c == ch)
			++count;
		else if (c == ofence)
			--count;

		if (sdir == FORWARD)
			s = forwchar(FALSE, 1);
		else
			s = backchar(FALSE, 1);

		if (s == FALSE)
			break;

		if (interrupted) {
			count = 1;
			break;
		}
	}

	/* if count is zero, we have a match, move the sucker */
	if (count == 0) {
		if (sdir == FORWARD) {
			if (!doingopcmd)
				backchar(FALSE, 1);
		} else {
			forwchar(FALSE, 1);
		}
		curwp->w_flag |= WFMOVE;
		return(TRUE);
	}

	/* restore the current position */
	DOT = oldpos;
	TTbeep();
	return(FALSE);
}

/* get the indent of the line containing the matching brace. */
int
fmatchindent()
{
	int ind;
	    
	MK = DOT;
	    
	if (getfence(FALSE,1,RBRACE) == FALSE) {
		gomark();
		return previndent(NULL);
	}

	ind = indentlen(DOT.l);

	gomark();
	    
	return ind;
}



/*	Close fences are matched against their partners, and if
	on screen the cursor briefly lights there		*/
fmatch(ch)
char ch;	/* fence type to match against */
{
	MARK	oldpos; 		/* original position */
	register LINE *toplp;	/* top line in current window */
	register int count; /* current fence level count */
	register char opench;	/* open fence */
	register char c;	/* current character in scan */
	register int i;

	/* first get the display update out there */
	update(FALSE);

	/* save the original cursor position */
	oldpos = DOT;

	/* setup proper open fence for passed close fence */
	if (ch == ')')
		opench = '(';
	else if (ch == RBRACE)
		opench = LBRACE;
	else
		opench = '[';

	/* find the top line and set up for scan */
	toplp = lback(curwp->w_line.l);
	count = 1;
	backchar(TRUE, 2);

	/* scan back until we find it, or reach past the top of the window */
	while (count > 0 && DOT.l != toplp) {
		if (is_at_end_of_line(DOT))
			c = '\n';
		else
			c = char_at(DOT);
		if (c == ch)
			++count;
		if (c == opench)
			--count;
		if (backchar(FALSE, 1) != TRUE)
			break;
	}

	/* if count is zero, we have a match, display the sucker */
	/* there is a real machine dependant timing problem here we have
	   yet to solve......... */
	if (count == 0) {
		forwchar(FALSE, 1);
		update(FALSE);
		/* the idea is to leave the cursor there for about a
			quarter of a second */
		catnap(250);
	}

	/* restore the current position */
	DOT = oldpos;

	return(TRUE);
}

#endif /* CFENCE */

catnap(milli)
{
#if ! UNIX
	for (i = 0; i < term.t_pause; i++)
		;
#else
# if HAVE_SELECT

	struct timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 250000;	/* 250000 microseconds */
	select (0, NULL, NULL, NULL, &tval);

# else
#  if HAVE_POLL

	struct pollfd pfd;
	poll(&pfd, 0, 250); /* 250 milliseconds */

#  else

	sleep(1); /* 1 second.	ugh. */

#  endif
# endif
#endif
	return TRUE;
}

#if ! SMALLER
istring(f, n)	/* ask for and insert a string into the current
		   buffer at the current point */
int f, n;	/* ignored arguments */
{
	register char *tp;	/* pointer into string to add */
	register int status;	/* status return code */
	static char tstring[NPAT+1];	/* string to add */

	/* ask for string to insert */
	status = mlreply("String to insert: ", tstring, NPAT);
	if (status != TRUE)
		return(status);

	if (f == FALSE)
		n = 1;

	if (n < 0)
		n = - n;

	/* insert it */
	while (n--) {
		tp = &tstring[0];
		while (*tp) {
			if (*tp == '\n')
				status = lnewline();
			else
				status = linsert(1, *tp);
			++tp;
			if (status != TRUE)
				return(status);
		}
	}

	return(TRUE);
}
#endif
