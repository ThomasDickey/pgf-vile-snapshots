/*
 * This file contains the command processing functions for a number of random
 * commands. There is no functional grouping here, for sure.
 *
 * $Log: random.c,v $
 * Revision 1.62  1992/05/25 21:07:48  foxharp
 * extern func declarations moved to header
 *
 * Revision 1.61  1992/05/20  18:55:08  foxharp
 * better confirmation output from cd command
 *
 * Revision 1.60  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.59  1992/05/16  14:02:55  pgf
 * header/typedef fixups
 *
 * Revision 1.58  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.57  1992/05/13  09:14:50  pgf
 * when scanning for a fence character, don't go past end of line.
 * i hate do/while loops.
 *
 * Revision 1.56  1992/04/14  08:51:44  pgf
 * missing local var in DOS ifdef
 *
 * Revision 1.55  1992/03/24  22:45:09  pgf
 * allow ^D to back up past autoindented whitespace
 *
 * Revision 1.54  1992/03/24  07:45:08  pgf
 * made separate internal and external entry points for go[to]col, so that
 * users see columns starting at 1, and vile sees them starting from 0
 *
 * Revision 1.53  1992/03/19  23:23:31  pgf
 * removed extra string lib externs and include
 *
 * Revision 1.52  1992/03/05  09:19:55  pgf
 * changed some mlwrite() to mlforce(), due to new terse support
 *
 * Revision 1.51  1992/03/03  08:43:47  pgf
 * fixed off-by-one error in gotocol()
 *
 * Revision 1.50  1992/03/01  18:38:40  pgf
 * added fence matching on #if, #el, and #en, and
 * fixed compilation error #if COLOR
 *
 * Revision 1.49  1992/02/26  21:58:17  pgf
 * changes to entab/detabline, to better support shift operations
 *
 * Revision 1.48  1992/02/17  08:55:57  pgf
 * added showmode support, and
 * fixed backspace-after-wrapword bug, and
 * fixed '=' parsing in set commands, and
 * some small changes for saber cleanup
 *
 * Revision 1.47  1992/01/13  23:33:32  pgf
 * finished shiftwidth implementation -- ^D works now
 *
 * Revision 1.46  1992/01/10  08:08:46  pgf
 * added shiftwidth(), and some bug fixes to en/detabline()
 *
 * Revision 1.45  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.44  1992/01/03  23:31:49  pgf
 * use new ch_fname() to manipulate filenames, since b_fname is now
 * a malloc'ed sting, to avoid length limits
 *
 * Revision 1.43  1991/12/24  09:18:47  pgf
 * added current/change directory support  (Dave Lemke's changes)
 *
 * Revision 1.42  1991/11/16  18:39:50  pgf
 * pass new magic arg to regcomp, and
 * fixed bug in openup/opendown pair
 *
 * Revision 1.41  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.40  1991/11/10  22:02:09  pgf
 * fixed bug in openup(), and made counts work right
 *
 * Revision 1.39  1991/11/08  14:13:58  pgf
 * fixed settab() and setfillcol()
 *
 * Revision 1.38  1991/11/08  13:10:26  pgf
 * lint cleanup, and
 * put in dave lemke's "autoinsert works going up" fix, and
 * made autoinsert clean up pre-existing leading whitespace
 *
 * Revision 1.37  1991/11/06  23:28:08  pgf
 * getfence() will scan for a fence if not on one to begin with.  it'll
 * scan in either direction, depending on arg to matchfence or matchfenceback
 *
 * Revision 1.36  1991/11/03  17:46:30  pgf
 * removed f,n args from all region functions -- they don't use them,
 * since they're no longer directly called by the user
 *
 * Revision 1.35  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.34  1991/10/29  03:04:45  pgf
 * fixed argument mismatches
 *
 * Revision 1.33  1991/10/28  14:21:52  pgf
 * eliminated TABVAL and fillcol macros, converted gasave to per-buffer
 *
 * Revision 1.32  1991/10/27  01:51:48  pgf
 * new regex values are now settable and displayable
 *
 * Revision 1.31  1991/10/24  13:01:03  pgf
 * bug fix for append'ing in empty buffer
 *
 * Revision 1.30  1991/10/18  10:56:54  pgf
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
#if HAVE_POLL
# include <poll.h>
#endif

extern CMDFUNC f_forwchar, f_backchar, f_forwchar_to_eol, f_backchar_to_bol;


/* generic "lister", which takes care of popping a window/buffer pair under
	the given name, and calling "func" with a couple of args to fill in
	the buffer */
int
liststuff(name,func,iarg,carg)
char *name;
void (*func)();		/* ptr to function to execute */
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
		mlforce("[Can't list. ]");
		zotbuf(bp);
		return (FALSE);
	}
	/* call the passed in function, giving it both the integer and 
		character pointer arguments */
	(*func)(iarg,carg);
	gotobob(FALSE,1);
	{ char buf[80];
	  lsprintf(buf, "       %s   %s",prognam,version);
	  ch_fname(bp, buf);
	}
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

/* ARGSUSED */
int
listmodes(f, n)
int f,n;
{
	register WINDOW *wp = curwp;
	register int s;

	s = liststuff("[Settings]",makemodelist,0,(char *)wp);
	/* back to the buffer whose modes we just listed */
	swbuffer(wp->w_bufp);
	return s;
}


/* list the current modes into the current buffer */
/* ARGSUSED */
void
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
	addline(blistp,line,-1);
#endif
}

/* listvalueset: print each value in the array according to type,
	along with its name, until a NULL name is encountered.  Only print
	if the value in the two arrays differs, or the second array is nil */
int
listvalueset(names, values, globvalues)
struct VALNAMES *names;
struct VAL *values, *globvalues;
{
	register int j, perline;
	perline = 3;
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
			perline = 1;
			if (j && j >= perline) {
				bputc('\n');
				j = 0;
			}
			if (!globvalues || ( values->vp->p && globvalues->v.p &&
				(strcmp( values->vp->p, globvalues->v.p)))) {
				bprintf("%s=%s%*P", names->name,
					values->vp->p ? values->vp->p : "",
					26, ' ');
				j++;
			}
			break;
		case VALTYPE_REGEX:
			if (!globvalues || (values->vp->r->pat && globvalues->v.r->pat &&
				(strcmp( values->vp->r->pat, globvalues->v.r->pat)))) {
				bprintf("%s=%s%*P", names->name,
					values->vp->r->pat ? values->vp->r->pat : "",
					26, ' ');
				j++;
			}
			break;
		default:
			mlforce("BUG: bad type %s %d",names->name,names->type);
			return FALSE;
		}
		if (j && j >= perline) {
			bputc('\n');
			j = 0;
		}
		names++;
		values++;
		if (globvalues) globvalues++;
	}
	if (j % 3 != 0)
		bputc('\n');
	return TRUE;
}
/*
 * Set fill column to n.
 */
int
setfillcol(f, n)
int f,n;
{
	if (f && n >= 1) {
		make_local_b_val(curbp,VAL_FILL);
		set_b_val(curbp,VAL_FILL,n);
	} else if (f) {
		mlforce("[Illegal fill-column value]");
		TTbeep();
		return FALSE;
	}
	if (!terse || !f)
		mlwrite("[Fill column is %d, and is %s]", b_val(curbp,VAL_FILL),
			is_global_b_val(curbp,VAL_FILL) ? "global" : "local" );
	return(TRUE);
}

/*
 * Display the current position of the cursor, lines and columns, in the file,
 * the character that is under the cursor (in hex), and the fraction of the
 * text that is before the cursor. The displayed column is not the current
 * column, but the column that would be used on an infinite width display.
 */
/* ARGSUSED */
int
showcpos(f, n)
int f,n;
{
	register LINE	*lp;		/* current line */
	register long	numchars = 0;	/* # of chars in file */
	register int	numlines = 0;	/* # of lines in file */
	register long	predchars = 0;	/* # chars preceding point */
	register int	predlines = 0;	/* # lines preceding point */
	register int	curchar = '\n';	/* character under cursor */
	int ratio;
	int col;
	int savepos;			/* temp save for current offset */
	int ecol;			/* column pos/end of current line */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_line.l);

	/* start counting chars and lines */
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
	mlforce(
"Line %d of %d, Col %d of %d, Char %D of %D (%d%%) char is 0x%x",
		predlines+1, numlines, col+1, ecol,
		predchars+1, numchars, ratio, curchar);
	return TRUE;
}

/* ARGSUSED */
int
showlength(f,n)
int f,n;
{
	register LINE	*lp;		/* current line */
	register int	numlines = 0;	/* # of lines in file */

	/* starting at the beginning of the buffer */
	lp = lforw(curbp->b_line.l);
	while (lp != curbp->b_line.l) {
		++numlines;
		lp = lforw(lp);
	}
	mlforce("%d",numlines);
	return TRUE;
}

#if ! SMALLER
int
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
int
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
 * Set current column, based on counting from 1
 */
int
gotocol(f,n)
int f,n;
{
	if (!f || n <= 0)
		n = 1;
	return gocol(n - 1);
}

/* really set column, based on counting from 0, for internal use */
int
gocol(n)
int n;
{
	register int c; 	/* character being scanned */
	register int i; 	/* index into current line */
	register int col;	/* current cursor column   */
	register int llen;	/* length of line in bytes */

	col = 0;
	llen = llength(DOT.l);

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
/* ARGSUSED */
int
twiddle(f, n)
int f,n;
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
int
quote(f, n)
int f,n;
{
	register int	s;
	register int	c;

	c = tgetc();
	if (!f)
		n = 1;
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

int
replacechar(f, n)
int f,n;
{
	register int	s;
	register int	c;

	if (!f && llength(DOT.l) == 0)
		return FALSE;

	insertmode = REPLACECHAR;  /* need to fool the SPEC prefix code */
	if (b_val(curbp, MDSHOWMODE))
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
int
settab(f, n)
int f,n;
{
	register WINDOW *wp;
	int val;
	char *whichtabs;
	if (b_val(curbp, MDCMOD)) {
		val = VAL_C_TAB;
		whichtabs = "C-t";
	} else {
		val = VAL_TAB;
		whichtabs = "T";
	}
	if (f && n >= 1) {
		make_local_b_val(curbp,val);
		set_b_val(curbp,val,n);
		curtabval = n;
		for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
			if (wp->w_bufp == curbp) wp->w_flag |= WFHARD;
	} else if (f) {
		mlforce("[Illegal tabstop value]");
		TTbeep();
		return FALSE;
	}
	if (!terse || !f)
		mlwrite("[%sabs are %d columns apart, using %s value.]", whichtabs,
			curtabval, is_global_b_val(curbp,val)?"global":"local" );
	return TRUE;
}

/* insert a tab into the file */
/* ARGSUSED */
int
tab(f, n)
int f,n;
{
	return linsert(1, '\t');
}

int
shiftwidth()
{
	int s;
	int fc;
	fc = firstchar(DOT.l);
	if (fc < DOT.o) {
		s = linsert(b_val(curbp, VAL_SWIDTH), ' ');
		/* should entab mult ^T inserts */
		return s;
	}
	detabline(TRUE);
	s = b_val(curbp, VAL_SWIDTH) - 
		(getccol(FALSE) % b_val(curbp,VAL_SWIDTH));
	if (s)
		s = linsert(s, ' ');
	entabline(TRUE);
	if (autoindented >= 0) {
		autoindented = firstchar(DOT.l);
	}
	return TRUE;
}


/*
 * change all tabs in the line to the right number of spaces.
 * leadingonly says only do leading whitespace
 */
int
detabline(leadingonly)
int leadingonly;
{
	register int	s;
	register int	c;
	int	ocol;

	ocol = getccol(FALSE);

	DOT.o = 0;

	/* detab the entire current line */
	while (DOT.o < llength(DOT.l)) {
		c = char_at(DOT);
		if (leadingonly && !isspace(c))
			break;
		/* if we have a tab */
		if (c == '\t') {
			if ((s = ldelete(1L, FALSE)) != TRUE) {
				return s;
			}
			insspace( TRUE, curtabval - (DOT.o % curtabval) );
		}
		DOT.o++;
	}
	gocol(ocol);
	return TRUE;
}

/*
 * change all tabs in the region to the right number of spaces
 */
int
detab_region()
{
	return do_fl_region(detabline,FALSE);
}

/*
 * convert all appropriate spaces in the line to tab characters.
 * leadingonly says only do leading whitespace
 */
int
entabline(leadingonly)
int leadingonly;
{
	register int fspace;	/* pointer to first space if in a run */
	register int ccol;	/* current cursor column */
	register char cchar;	/* current character */
	int	ocol;

	ocol = getccol(FALSE);

	/* entab the current line */
	/* would this have been easier if it had started at
		the _end_ of the line, rather than the beginning?  -pgf */
	fspace = -1;
	ccol = 0;

	detabline(leadingonly);	/* get rid of possible existing tabs */
	DOT.o = 0;
	while (1) {
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

		if (DOT.o >= llength(DOT.l))
			break;

		/* get the current character */
		cchar = char_at(DOT);

		if (cchar == ' ') { /* a space...compress? */
			if (fspace == -1)
				fspace = ccol;
		} else {
			if (leadingonly)
				break;
			fspace = -1;
		}
		ccol++;
		DOT.o++;
	}
	gocol(ocol);
	return TRUE;
}

/*
 * convert all appropriate spaces in the region to tab characters
 */
int
entab_region()
{
	return do_fl_region(entabline,FALSE);
}

/* trim trailing whitespace from a line.  leave dot at end of line */
int
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
		    
	return ldelete((long)(orig - off),FALSE);
}

/*
 * trim trailing whitespace from a region
 */
int
trim_region()
{
	return do_fl_region(trimline,0);
}



/* open lines up before this one */
int
openup(f,n)
int f,n;
{
	register int s;

	if (!f) n = 1;
	if (n < 0) return (FALSE);
	if (n == 0) return ins();

	gotobol(TRUE,1);

	/* if we are in C mode and this is a default <NL> */
	if (n == 1 && (b_val(curbp,MDCMOD) || b_val(curbp,MDAIND)) &&
						!is_header_line(DOT,curbp)) {
		s = indented_newline_above(b_val(curbp, MDCMOD));
		if (s != TRUE) return (s);

		return(ins());
	}
	s = lnewline();
	if (s != TRUE) return s;

	s = backline(TRUE,1);		/* back to the blank line */
	if (s != TRUE) return s;

	if ( n > 1) {
		s = openlines(n-1);
		if (s != TRUE) return s;
		s = backline(TRUE, 1);	/* backup over the first one */
		if (s != TRUE) return s;
	}

	return(ins());
}

/* open lines up after this one */
int
opendown(f,n)
int f,n;
{
	register int	s;

	if (!f) n = 1;
	if (n < 0) return (FALSE);
	if (n == 0) return ins();

	s = openlines(n);
	if (s != TRUE)
		return (s);

	return(ins());
}

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them.
 */
int
openlines(n)
int n;
{
	register int i = n;			/* Insert newlines. */
	register int s = TRUE;
	while (i-- && s==TRUE) {
		gotoeol(FALSE,1);
		s = newline(TRUE,1);
	}
	if (s == TRUE && n)			/* Then back up overtop */
		backline(TRUE, n-1);	/* of them all.		 */

	curgoal = -1;

	return s;
}

/*
 * Go into insert mode.  I guess this isn't emacs anymore...
 */
/* ARGSUSED */
int
insert(f, n)
int f,n;
{
	return ins();
}

/* ARGSUSED */
int
insertbol(f, n)
int f,n;
{
	firstnonwhite(f,n);
	return ins();
}

/* ARGSUSED */
int
append(f, n)
int f,n;
{
	if (is_header_line(DOT,curbp))
		return ins();
	if (!is_at_end_of_line(DOT)) /* END OF LINE HACK */
		forwchar(TRUE,1);
	return ins();
}

/* ARGSUSED */
int
appendeol(f, n)
int f,n;
{
	if (!is_header_line(DOT,curbp))
		gotoeol(FALSE,0);
	return ins();
}

/* ARGSUSED */
int
overwrite(f, n)
int f,n;
{
	insertmode = OVERWRITE;
	if (b_val(curbp, MDSHOWMODE))
		curwp->w_flag |= WFMODE;
	return ins();
}

/* grunt routine for insert mode */
int
ins()
{
	register int status;
	int f,n;
	int (*execfunc)();		/* ptr to function to execute */
	int    c;		/* command character */
	int newlineyet = FALSE; /* are we on the line we started on? */
	int startoff = DOT.o;	/* starting offset on that line */

	if (insertmode == FALSE) {
		insertmode = INSERT;
		if (b_val(curbp, MDSHOWMODE))
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
				trimline();
				autoindented = -1;
			}
			insertmode = FALSE;
			if (b_val(curbp, MDSHOWMODE))
				curwp->w_flag |= WFMODE;
			return (TRUE);
		} else if (c == -abortc ) {
			/* we use the negative to suppress that
				junk, for the benefit of SPEC keys */
			insertmode = FALSE;
			if (b_val(curbp, MDSHOWMODE))
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
				b_val(curbp,VAL_FILL) > 0 && n >= 0 &&
				getccol(FALSE) > b_val(curbp,VAL_FILL)) {
				wrapword(FALSE,1);
				newlineyet = TRUE;
			}

			if ( c ==  '\t') { /* tab */
				execfunc = tab;
				autoindented = -1;
			} else if (c ==  tocntrl('J') ||
				c ==  tocntrl('M')) { /* CR and NL */
				execfunc = newline;
				if (autoindented >= 0) {
					trimline();
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
				if ((c == tocntrl('D') && autoindented >=0) ||
					newlineyet || !b_val(curbp,MDBACKLIMIT))
					backlimit = 0;
				else
					backlimit = startoff;
				execfunc = nullproc;
				if (c == tocntrl('D')) {
					int goal;
					int col;
					int sw = 
					 b_val(curbp, VAL_SWIDTH);
					col = getccol(FALSE);
					if (col > 0)
						goal = ((col-1)/sw)*sw;
					else
						goal = 0;
					while (col > goal &&
						DOT.o > backlimit) {
						backspace();
						col = getccol(FALSE);
					}
					if (col < goal)
						linsert(goal - col,' ');
				} else while (DOT.o > backlimit) {
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
			} else if ( c ==  tocntrl('T')) { /* ^T */
				execfunc = shiftwidth;

#if UNIX && defined(SIGTSTP)	/* job control, ^Z */
			} else if (c == suspc) {
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
				if (b_val(curbp, MDSHOWMODE))
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
					(DOT.o) % curtabval == curtabval-1)) {
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
			if (--curbp->b_acount <= 0) {
				/* and save the file if needed */
				upscreen(FALSE, 0);
				filesave(FALSE, 0);
				curbp->b_acount = b_val(curbp,VAL_ASAVECNT);
			}

		if (status != TRUE) {
			insertmode = FALSE;
			if (b_val(curbp, MDSHOWMODE))
				curwp->w_flag |= WFMODE;
			return (status);
		}
	}
}

int
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
int
newline(f, n)
int f,n;
{
	register int	s;

	if (!f)
		n = 1;
	else if (n < 0)
		return (FALSE);

#if LATER	/* already done for autoindented != 0 in ins() */
	if (b_val(curbp, MDTRIM))
		trimline();
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
	if (b_val(curwp->w_bufp, MDWRAP) && b_val(curbp,VAL_FILL)> 0 &&
				getccol(FALSE) > b_val(curbp,VAL_FILL))
		wrapword(FALSE,1);

	/* insert some lines */
	while (n--) {
		if ((s=lnewline()) != TRUE)
			return (s);
		curwp->w_flag |= WFINS;
	}
	return (TRUE);
}

/* insert a newline and indentation for C */
int
indented_newline(cmode)
int cmode;
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

/* insert a newline and indentation for autoindent */
int
indented_newline_above(cmode)
int cmode;
{
	register int indentwas;	/* indent to reproduce */
	int bracef; /* was there a brace at the beginning of line? */
	
	indentwas = nextindent(&bracef);
	if (lnewline() == FALSE)
		return FALSE;
	if (backline(TRUE,1) == FALSE)
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
		gomark(FALSE,1);
		return 0;
	}
	ind = indentlen(DOT.l);
	if (bracefp)
		*bracefp = (llength(DOT.l) > 0 &&
			lgetc(DOT.l,lastchar(DOT.l)) == '{');
		    
	gomark(FALSE,1);
	    
	return ind;
}

/* get the indent of the next non-blank line.	also, if arg
	is non-null, check if line starts in a brace */
int
nextindent(bracefp)
int *bracefp;
{
	int ind;
	    
	MK = DOT;
	    
	if (forwword(FALSE,1) == FALSE) {
		if (bracefp) *bracefp = FALSE;
		gomark(FALSE,1);
		return 0;
	}
	ind = indentlen(DOT.l);
	if (bracefp)
		*bracefp = (llength(DOT.l) > 0 &&
			lgetc(DOT.l,firstchar(DOT.l)) == '}');
		    
	gomark(FALSE,1);
	    
	return ind;
}

int
doindent(ind)
int ind;
{
	int i;
	/* if no indent was asked for, we're done */
	if (ind <= 0)
		return TRUE;
	autoindented = 0;
	/* first clean up existing leading whitespace */
	i = firstchar(DOT.l);
	if (i)
		ldelete((long)i,FALSE);
	if ((i=ind/curtabval)!=0) {
		autoindented += i;
		if (linsert(i, '\t') == FALSE)
			return FALSE;
	}
	if ((i=ind%curtabval) != 0) {
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


int
insbrace(n, c)	/* insert a brace into the text here...we are in CMODE */
int n;	/* repeat count */
int c;	/* brace to insert (always { for now) */
{

#if ! CFENCE
	/* wouldn't want to back up from here, but fences might take us 
		forward */
	/* if we are at the beginning of the line, no go */
	if (DOT.o == 0)
		return(linsert(n,c));
#endif

	if (autoindented >= 0) {
		trimline();
	}
	else {
		return linsert(n,c);
	}
#if ! CFENCE /* no fences?	then put brace one tab in from previous line */
	doindent(((previndent(NULL)-1) / curtabval) * curtabval);
#else /* line up brace with the line containing its match */
	doindent(fmatchindent());
#endif
	autoindented = -1;

	/* and insert the required brace(s) */
	return(linsert(n, c));
}

int
inspound()	/* insert a # into the text here...we are in CMODE */
{

	/* if we are at the beginning of the line, no go */
	if (DOT.o == 0)
		return(linsert(1,'#'));

	if (autoindented > 0) { /* must all be whitespace before us */
		DOT.o = 0;
		ldelete((long)autoindented,FALSE);
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
/* ARGSUSED */
int
deblank(f, n)
int f,n;
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
int
flipchar(f, n)
int f,n;
{
	int s;

	havemotion = &f_forwchar_to_eol;
	s = operflip(f,n);
	if (s != TRUE)
		return s;
	return forwchar_to_eol(f,n);
}

/* 'x' is synonymous with 'd<space>' */
int
forwdelchar(f, n)
int f,n;
{

	havemotion = &f_forwchar_to_eol;
	return operdel(f,n);
}

/* 'X' is synonymous with 'd<backspace>' */
int
backdelchar(f, n)
int f,n;
{
	havemotion = &f_backchar_to_bol;
	return operdel(f,n);
}

/* 'D' is synonymous with 'd$' */
/* ARGSUSED */
int
deltoeol(f, n)
int f,n;
{
	extern CMDFUNC f_gotoeol;

	if (llength(DOT.l) == 0)
		return TRUE;

	havemotion = &f_gotoeol;
	return operdel(FALSE,1);
}

/* 'C' is synonymous with 'c$' */
/* ARGSUSED */
int
chgtoeol(f, n)
int f,n;
{
	extern CMDFUNC f_gotoeol;

	if (llength(DOT.l) == 0) {
		return ins();
	} else {
		havemotion = &f_gotoeol;
		return operchg(FALSE,1);
	}
}

/* 'Y' is synonymous with 'yy' */
int
yankline(f, n)
int f,n;
{
	extern CMDFUNC f_godotplus;
	havemotion = &f_godotplus;
	return(operyank(f,n));
}

/* 'S' is synonymous with 'cc' */
int
chgline(f, n)
int f,n;
{
	extern CMDFUNC f_godotplus;
	havemotion = &f_godotplus;
	return(operchg(f,n));
}

/* 's' is synonymous with 'c<space>' */
int
chgchar(f, n)
int f,n;
{
	havemotion = &f_forwchar_to_eol;
	return(operchg(f,n));
}

/* ARGSUSED */
int
setmode(f, n)	/* prompt and set an editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(TRUE, FALSE);
}

/* ARGSUSED */
int
delmode(f, n)	/* prompt and delete an editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(FALSE, FALSE);
}

/* ARGSUSED */
int
setgmode(f, n)	/* prompt and set a global editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(TRUE, TRUE);
}

/* ARGSUSED */
int
delgmode(f, n)	/* prompt and delete a global editor mode */
int f, n;	/* default and argument */
{
	return adjustmode(FALSE, TRUE);
}


int
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
	while (*scan != '\0' && *scan != '=') {
		if (isupper(*scan))
			*scan = tolower(*scan);
		scan++;
	}

	if (scan == cbuf) { /* no string */
		s = FALSE;
		goto errout;
	}

	if (!strcmp(cbuf,"all")) {
		return listmodes(FALSE,1);
	}

	/* colors should become another kind of value, i.e. VAL_TYPE_COLOR,
		and then this could be handled generically in adjvalueset() */
	/* test it first against the colors we know */
	for (i=0; i<NCOLORS; i++) {
		if (strcmp(cbuf, cname[i]) == 0) {
			/* finding the match, we set the color */
#if COLOR
			if (uflag)
				if (global)
					gfcolor = i;
				else if (curwp)
					set_w_val(curwp, WVAL_FCOLOR, i);
			else
				if (global)
					gbcolor = i;
				else if (curwp)
					set_w_val(curwp, WVAL_BCOLOR, i);

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
	if (s == FALSE) {
	errout:
		mlforce("[Not a legal set option: \"%s\"]", cbuf);
	}
	return s;

success:
	if (global) {
		register WINDOW *wp;
		for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
			wp->w_flag |= WFHARD|WFMODE;
	} else {
		curwp->w_flag |= WFHARD|WFMODE;
	}

	/* if the settings are up, redisplay them */
	if (bfind("[Settings]", NO_CREAT, BFSCRTCH))
		listmodes(FALSE,1);

	refresh(FALSE,1);
	mlerase();	/* erase the junk */

	return TRUE;
}

int
adjvalueset(cp, kind, names, values)
char *cp;
int kind;
struct VALNAMES *names;
register struct VAL *values;
{
	char respbuf[NFILEN];
	char *rp = NULL;
	char *equp = NULL;
	int rplen = 0;
	int no;
	int nval, s;

	no = 0;
	if (strncmp(cp, "no", 2) == 0) {
		kind = !kind;
		no = 2;
	}
	equp = rp = strchr(cp, '=');
	if (rp) {
		*rp = '\0';
		rp++;
		rplen = strlen(rp);
		if (!rplen)
			goto err;
	}
	while(names->name != NULL) {
		if (strncmp(cp + no, names->name, strlen(cp) - no) == 0)
			break;
		if (strncmp(cp + no, names->shortname, strlen(cp) - no) == 0)
			break;
		names++;
		values++;
	}
	if (names->name == NULL) {
	err:	if (equp)
			*equp = '=';
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
	case VALTYPE_REGEX:
		if (no) goto err;
		if (!rp) {
			respbuf[0] = '\0';
			if (names->type != VALTYPE_REGEX)
				s = mlreply("New value: ", respbuf, NFILEN - 1);
			else
				s = kbd_string("New pattern: ", respbuf,
					NFILEN - 1, '\n', NO_EXPAND, FALSE);
			if (s != TRUE) {
				if (equp)
					*equp = '=';
				return s;
			}
			rp = respbuf;
			rplen = strlen(rp);
			if (!rplen)
				goto err;
		}
		values->vp = &(values->v);
		switch (names->type) {
		case VALTYPE_INT:
			nval = 0;
			while (isdigit(*rp))
				nval = (nval * 10) + (*rp++ - '0');
			values->vp->i = nval;
			break;
		case VALTYPE_STRING:
			if (values->vp->p)
				free(values->vp->p);
			values->vp->p = strmalloc(rp);
			break;
		case VALTYPE_REGEX:
			if (!values->vp->r) {
				values->vp->r = (struct regexval *)malloc(
											sizeof (struct regexval));
			} else {
				if (values->vp->r->pat)
					free(values->vp->r->pat);
				if (values->vp->r->reg)
					free((char *)values->vp->r->reg);
			}
			values->vp->r->pat = strmalloc(rp);
			values->vp->r->reg = regcomp(values->vp->r->pat, TRUE);
			break;
		}
		break;

	default:
		mlforce("BUG: bad type %s %d",names->name,names->type);
		if (equp)
			*equp = '=';
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

/* ARGSUSED */
int
clrmes(f, n)
int f, n;	/* arguments ignored */
{
	mlforce("");
	return(TRUE);
}

#if ! SMALLER

/*	This function writes a string on the message line
		mainly for macro usage			*/

/* ARGSUSED */
int
writemsg(f, n)
int f, n;	/* arguments ignored */
{
	register int status;
	char buf[NPAT]; 	/* buffer to recieve message into */

	buf[0] = 0;
	if ((status = mlreply("Message to write: ", buf, NPAT - 1)) != TRUE)
		return(status);

	/* write the message out */
	mlforce("%s",buf);
	return(TRUE);
}
#endif

#if CFENCE
/*	the cursor is moved to a matching fence */
int
matchfence(f,n)
int f,n;
{
	return getfence(0, (!f || n > 0) ? FORWARD:REVERSE);
}

int
matchfenceback(f,n)
int f,n;
{
	return getfence(0, (!f || n > 0) ? REVERSE:FORWARD);
}

int
getfence(ch,scandir)
int ch; /* fence type to match against */
int scandir; /* direction to scan if we're not on a fence to begin with */
{
	MARK	oldpos; 	/* original pointer */
	register int sdir;	/* direction of search (1/-1) */
	register int count; 	/* current fence level count */
	register int ofence = 0;	/* open fence */
	register int c; 	/* current character in scan */
	int s;
	char *otherkey = NULL;
	char *key = NULL;

	/* save the original cursor position */
	oldpos = DOT;

	/* ch may have been passed, if being used internally */
	if (!ch) {
		if (b_val(curbp, MDCMOD) && DOT.o == 0
					&& (ch = char_at(DOT)) == '#') {
			if (llength(DOT.l) < 3)
				return FALSE;
		} else if (scandir == FORWARD) {
			/* get the current character */
			if (oldpos.o < llength(oldpos.l)) {
				do {
					ch = char_at(oldpos);
				} while(!isfence(ch) &&
					++oldpos.o < llength(oldpos.l));
			}
			if (is_at_end_of_line(oldpos)) {
				TTbeep();
				return FALSE;
			}
		} else {
			/* get the current character */
			if (oldpos.o >= 0) {
				do {
					ch = char_at(oldpos);
				} while(!isfence(ch) && --oldpos.o >= 0);
			}

			if (oldpos.o < 0) {
				TTbeep();
				return FALSE;
			}
		}

		/* we've at least found a fence -- move us that far */
		DOT.o = oldpos.o;
	}

	/* setup proper matching fence */
	switch (ch) {
		case '(': ofence = ')'; sdir = FORWARD; break;
		case LBRACE: ofence = RBRACE; sdir = FORWARD; break;
		case '[': ofence = ']'; sdir = FORWARD; break;
		case ')': ofence = '('; sdir = REVERSE; break;
		case RBRACE: ofence = LBRACE; sdir = REVERSE; break;
		case ']': ofence = '['; sdir = REVERSE; break;
		case '#':
			if (!strncmp("#if",DOT.l->l_text, 3)) {
				sdir = FORWARD;
				key = "#if";
				otherkey = "#en";
			} else if (!strncmp("#el",DOT.l->l_text, 3)) {
				if (scandir == FORWARD) {
					sdir = FORWARD;
					key = "#if";
					otherkey = "#en";
				} else {
					sdir = REVERSE;
					key = "#en";
					otherkey = "#if";
				}
			} else if (!strncmp("#en",DOT.l->l_text, 3)) {
				sdir = REVERSE;
				key = "#en";
				otherkey = "#if";
			} else 
		default: {
				TTbeep(); return(FALSE);
			}
	}

	/* ops are inclusive of the endpoint */
	if (doingopcmd && sdir == REVERSE) {
		forwchar(TRUE,1);
		MK = DOT;
		backchar(TRUE,1);
	}

	count = 1;

	if (otherkey != NULL) {  /* we're searching for a cpp keyword */
		/* set up for scan */
		if (sdir == REVERSE)
			DOT.l = lback(DOT.l);
		else
			DOT.l = lforw(DOT.l);
		while (count > 0 && !is_header_line(DOT, curbp)) {
			if (llength(DOT.l) >= 3 && char_at(DOT) == '#') {
				if (!strncmp(key, DOT.l->l_text, 3))
					++count;
				else if (!strncmp(otherkey,DOT.l->l_text, 3))
					--count;
			}

			if (count == 0)
				break;

			if (sdir == REVERSE)
				DOT.l = lback(DOT.l);
			else
				DOT.l = lforw(DOT.l);

			if (interrupted) {
				count = 1;
				break;
			}
		}
		if (count == 0) {
			curwp->w_flag |= WFMOVE;
			if (doingopcmd)
				fulllineregions = TRUE;
			return TRUE;
		}

	} else {

		/* set up for scan */
		if (sdir == REVERSE)
			backchar(FALSE, 1);
		else
			forwchar(FALSE, 1);

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
	    
	if (getfence(RBRACE,FORWARD) == FALSE) {
		gomark(FALSE,1);
		return previndent(NULL);
	}

	ind = indentlen(DOT.l);

	gomark(FALSE,1);
	    
	return ind;
}



/*	Close fences are matched against their partners, and if
	on screen the cursor briefly lights there		*/
int
fmatch(ch)
int ch;	/* fence type to match against */
{
	MARK	oldpos; 		/* original position */
	register LINE *toplp;	/* top line in current window */
	register int count; /* current fence level count */
	register char opench;	/* open fence */
	register char c;	/* current character in scan */

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

void
catnap(milli)
int milli;
{
#if ! UNIX
	int i;
	for (i = 0; i < term.t_pause; i++)
		;
#else
# if HAVE_SELECT

	struct timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = milli * 1000;	/* microseconds */
	select (0, NULL, NULL, NULL, &tval);

# else
#  if HAVE_POLL

	struct pollfd pfd;
	poll(&pfd, 0, milli); /* milliseconds */

#  else

	sleep(1); /* 1 second.	ugh. */

#  endif
# endif
#endif
}

#if ! SMALLER
int
istring(f, n)	/* ask for and insert a string into the current
		   buffer at the current point */
int f, n;
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

#if UNIX
#include	<sys/param.h>
#endif

/* return a string naming the current directory */
char *
current_directory()
{
	char *cwd, *s;
#if USG
	static char	dirname[NFILEN*2];
	FILE *f, *npopen();
	int n;
	f = npopen("/bin/pwd", "r");
	if (f == NULL) {
		fclose(f);
		return NULL;
	}
	n = fread(dirname, 1, NFILEN, f);

	dirname[n] = '\0';
	fclose(f);
	cwd = dirname;
#else
	static char	dirname[NFILEN*2];
# if MSDOS & MSC
	cwd = getcwd(dirname, NFILEN*2);
# else
	cwd = getwd(dirname);
# endif
#endif
	s = strchr(cwd, '\n');
	if (s)
		*s = '\0';
	return cwd;
}


/* ARGSUSED */
int
cd(f,n)
int f, n;
{
	int status;
	static char cdirname[NFILEN];
	status = mlreply("Change to directory: ", cdirname, NFILEN);
	if (status != TRUE) {
		/* should go HOME here */
		return status;
	}
	return set_directory(cdirname);
}

/* ARGSUSED */
int
pwd(f,n)
int f, n;
{
	mlforce("%s",current_directory());
	return TRUE;
}

/* move to the named directory.  adjust all non-absolute filenames
	to be absolute.
	(Dave Lemke)
*/
int
set_directory(dir)
char	*dir;
{
    char       exdir[NFILEN];
    BUFFER     *bp;
    char       *cwd;

    /* first fix up all the buffer file names */
    cwd = current_directory();
    if (!cwd || ! *cwd)
	return FALSE;
    for (bp = bheadp; bp; bp = bp->b_bufp) {
	/* ignore any that are already absolute or "internal" */
	if (bp->b_fname[0] == '/' ||
		bp->b_fname[0] == '\0' || 
		bp->b_fname[0] == '[' ||
		bp->b_fname[0] == '!') {
	    continue;
	}
	/* XXX file name is only 80 bytes -- could easily overflow here... */
	sprintf(exdir, "%s/%s", cwd, bp->b_fname );
	ch_fname(bp, exdir);
    }

    strcpy(exdir, dir);
    if (glob(exdir) && (chdir(exdir) == 0)) {
	pwd(TRUE,1);
	return TRUE;
    }
    mlforce("[Couldn't change to \"%s\"]", exdir);
    return FALSE;
}

