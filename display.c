/*
 * The functions in this file handle redisplay. There are two halves, the
 * ones that update the virtual display screen, and the ones that make the
 * physical display screen the same as the virtual display screen. These
 * functions use hints that are left in the windows by the commands.
 *
 *
 * $Log: display.c,v $
 * Revision 1.59  1993/01/23 13:38:23  foxharp
 * tom's changes -- reverse video works better, fix for list mode,
 * dfoutfn now re-entrant, dfputli now does hex
 *
 * Revision 1.58  1993/01/16  10:26:13  foxharp
 * for_each_window and other new macros, and fixes for column 80 indicator
 *
 * Revision 1.57  1993/01/12  08:48:43  foxharp
 * tom dickey's changes to support "set number", i.e. line numbering
 *
 * Revision 1.56  1992/12/23  09:17:17  foxharp
 * ifdef of unused code
 *
 * Revision 1.55  1992/12/14  08:32:20  foxharp
 * fix for the sideways-shifted-but-deextended bug.  thanks to Tom Dickey.
 * also lint cleanup.
 *
 * Revision 1.54  1992/12/04  09:12:25  foxharp
 * deleted unused assigns
 *
 * Revision 1.53  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.52  1992/08/05  21:55:16  foxharp
 * handle files with DOS drive designators properly on mode line
 *
 * Revision 1.51  1992/07/24  07:49:38  foxharp
 * shorten_name changes
 *
 * Revision 1.50  1992/07/22  19:25:57  foxharp
 * handle non-printables correctly -- how did I get involved in this, anyway!
 *
 * Revision 1.49  1992/07/22  09:18:16  foxharp
 * got it.  sheesh.
 *
 * Revision 1.48  1992/07/22  00:50:43  foxharp
 * interim -- still dumping core in vtset
 *
 * Revision 1.47  1992/07/21  09:08:30  foxharp
 * now pass lp to vtset()
 *
 * Revision 1.46  1992/07/21  08:57:53  foxharp
 * pushed list mode choice into vtset()
 *
 * Revision 1.45  1992/07/20  22:44:44  foxharp
 * performance improvements -- fewer vtputc's
 *
 * Revision 1.44  1992/07/18  13:13:56  foxharp
 * put all path-shortening in one place (shorten_path()), and took out
 * some old code now unnecessary
 *
 * Revision 1.43  1992/07/13  20:03:54  foxharp
 * the "terse" variable is now a boolean mode
 *
 * Revision 1.42  1992/07/13  19:37:00  foxharp
 * trim leading `pwd` from filenames in modeline, now that filenames are
 * usually absolute
 *
 * Revision 1.41  1992/07/01  16:59:46  foxharp
 * scwrite() arg changes, and some fore/background color cleanup
 *
 * Revision 1.40  1992/06/26  22:21:05  foxharp
 * small almost cosmetic changes to mlsav'ing
 *
 * Revision 1.39  1992/05/29  08:36:06  foxharp
 * moved the SVR3_PTEM ifdef so it works with POSIX too
 *
 * Revision 1.38  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.37  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.36  1992/04/27  19:50:43  pgf
 * fixed ifdefs on termio inclusion
 *
 * Revision 1.35  1992/04/14  08:50:01  pgf
 * fix SIGWINCH handling for X11 case
 *
 * Revision 1.34  1992/03/25  19:13:17  pgf
 * BSD portability changes
 *
 * Revision 1.33  1992/03/19  23:31:54  pgf
 * mlputc now converts tabs to single spaces, since we don't really
 * know how wide they should be anyway
 *
 * Revision 1.32  1992/03/19  23:13:04  pgf
 * SIGT for signals
 *
 * Revision 1.31  1992/03/07  10:21:29  pgf
 * suppress prompts (mlprompt()) if execing buff or file
 *
 * Revision 1.30  1992/03/05  09:17:21  pgf
 * added support for new "terse" variable, to control unnecessary messages
 *
 * Revision 1.29  1992/03/01  18:38:40  pgf
 * compilation error #if COLOR
 *
 * Revision 1.28  1992/02/17  08:57:36  pgf
 * added "showmode" support
 *
 * Revision 1.27  1992/01/22  20:26:00  pgf
 * fixed ifdef conflict: SVR3_PTEM
 *
 * Revision 1.26  1992/01/05  00:05:24  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.25  1991/11/13  20:09:27  pgf
 * X11 changes, from dave lemke
 *
 * Revision 1.24  1991/11/08  13:14:40  pgf
 * more lint
 *
 * Revision 1.23  1991/11/07  03:58:31  pgf
 * lint cleanup
 *
 * Revision 1.22  1991/11/03  17:35:06  pgf
 * don't access unset vt column 80 if screen isn't that wide
 *
 * Revision 1.21  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.20  1991/10/28  14:22:46  pgf
 * no more TABVAL macro, curtabstopval renamed curtabval
 *
 * Revision 1.19  1991/10/26  00:14:56  pgf
 * put termio.h back in, and switch to SVR3 around ptem.h
 *
 * Revision 1.18  1991/10/23  12:05:37  pgf
 * we don't need termio.h
 *
 * Revision 1.17  1991/10/08  01:28:43  pgf
 * dbgwrite now uses raw'est i/o
 *
 * Revision 1.16  1991/09/26  13:16:19  pgf
 * LIST mode and w_sideways are now both window values
 *
 * Revision 1.15  1991/09/11  02:30:22  pgf
 * use get_recorded_char, now that we have it
 *
 * Revision 1.14  1991/08/16  11:10:46  pgf
 * added insert mode indicator to modeline: I for insert, O for overwrite, and
 * R for replace-char
 *
 * Revision 1.13  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.12
 * date: 1991/08/06 15:12:29;
 * global/local values, and printf/list changes
 * 
 * revision 1.11
 * date: 1991/06/28 10:53:18;
 * make update quit early if !curbp
 * 
 * revision 1.10
 * date: 1991/06/25 19:52:13;
 * massive data structure restructure
 * 
 * revision 1.9
 * date: 1991/06/16 17:34:57;
 * switched to modulo tab calculations
 * 
 * revision 1.8
 * date: 1991/05/31 10:35:55;
 * added new "dbgwrite()" routine, for debugging
 * 
 * revision 1.7
 * date: 1991/04/22 09:04:14;
 * more ODT support
 * 
 * revision 1.6
 * date: 1991/03/19 12:15:08;
 * fix flag checking
 * 
 * revision 1.5
 * date: 1991/02/21 10:04:01;
 * horizontal scrolling is most of the way there.  only thing
 * left to do is to extend lines where the cursor is beyond the
 * left edge of a scrolled screen
 * 
 * revision 1.4
 * date: 1990/10/05 14:19:31;
 * fixed typo in ODT ifdef.
 * propagated mode line changes to all windows holding a given buffer
 * 
 * revision 1.3
 * date: 1990/10/01 11:04:42;
 * atatus return in newscreensize
 * 
 * revision 1.2
 * date: 1990/09/25 11:38:06;
 * took out old ifdef BEFORE code
 * 
 * revision 1.1
 * date: 1990/09/21 10:24:58;
 * initial vile RCS revision
 */


#include	"estruct.h"
#include        "edef.h"
#if UNIX
# include <signal.h>
# if POSIX
#  include <termios.h>
# else
#  if USG
#   include <termio.h>
#  else
#   if BERK
#    include "ioctl.h"
#   endif
#  endif
# endif
# if SVR3_PTEM
#  include <sys/types.h>
#  include <sys/stream.h>
#  include <sys/ptem.h>
# endif
#endif

#define	NU_WIDTH 8

#define	MRK_EMPTY        '~'
#define	MRK_EXTEND_LEFT  '<'
#define	MRK_EXTEND_RIGHT '>'

#ifndef __STDC__

#ifndef va_dcl	 /* then try these out */

typedef char *va_list;
#define va_dcl int va_alist;
#define va_start(list) list = (char *) &va_alist
#define va_end(list)
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]

#endif

#endif /* __STDC__ */

#ifdef	lint
# undef  va_dcl
# define va_dcl char * va_alist;
# undef  va_start
# define va_start(list) list = (char *) &va_alist
# undef  va_arg
# define va_arg(ptr,cast) (cast)(ptr-(char *)0)
#endif

VIDEO   **vscreen;                      /* Virtual screen. */
#if	! MEMMAP
VIDEO   **pscreen;                      /* Physical screen. */
#endif


int displaying = FALSE;
/* for window size changes */
int chg_width, chg_height;

/******************************************************************************/

static	void	(*dfoutfn) P(( int ));

/*
 * Do format a string.
 */
static
int	dfputs(outfunc, s)
	void (*outfunc)();
	char *s;
{
	register int c;
	register int l = 0;

	while ((c = *s++) != 0) {
	        (*outfunc)(c);
		l++;
	}
	return l;
}

/* as above, but takes a count for s's length */
static
int	dfputsn(outfunc,s,n)
	void (*outfunc)();
	char *s;
	int n;
{
	register int c;
	register int l = 0;
	while ((c = *s++) != 0 && n-- != 0) {
	        (*outfunc)(c);
		l++;
	}
	return l;
}

/*
 * Do format an integer, in the specified radix.
 */
static
int	dfputi(outfunc,i, r)
	void (*outfunc)();
	int i,r;
{
	register int q;
	static char hexdigits[] = "0123456789ABCDEF";

	if (i < 0) {
		(*outfunc)('-');
		return dfputi(outfunc, -i, r) + 1;
	}

	q = (i >= r) ? dfputi(outfunc, i/r, r) : 0;

	(*outfunc)(hexdigits[i%r]);
	return q + 1; /* number of digits printed */
}

/*
 * do the same except as a long integer.
 */
static
int	dfputli(outfunc,l, r)
	void (*outfunc)();
	long l;
	int r;
{
	register long q;

	if (l < 0) {
		(*outfunc)('-');
		return dfputli(outfunc, -l, r) + 1;
	}

	q = (l >= r) ? dfputli(outfunc, l/r, r) : 0;

	return q + dfputi(outfunc, (int)(l%r), r);
}

/*
 *	Do format a scaled integer with two decimal places
 */
static
int	dfputf(outfunc,s)
	void (*outfunc)();
	int s;	/* scaled integer to output */
{
	register int i;	/* integer portion of number */
	register int f;	/* fractional portion of number */

	/* break it up */
	i = s / 100;
	f = s % 100;

	/* send out the integer portion */
	i = dfputi(outfunc, i, 10);
	(*outfunc)('.');
	(*outfunc)((f / 10) + '0');
	(*outfunc)((f % 10) + '0');
	return i + 3;
}

/*
 * Generic string formatter.  Takes printf-like args, and calls
 * the global function (*dfoutfn)(c) for each c
 */
static
void
#ifdef __STDC__
dofmt( char *fmt, va_list *app)
#else
dofmt(app)
va_list *app;
#endif
{
#ifndef __STDC__
	register char *fmt = va_arg(*app, char *);
#endif
	register int c;		/* current char in format string */
	register int wid;
	register int n;
	register int nchars = 0;
	int islong;
	void (*outfunc)() = dfoutfn;  /* local copy, for recursion */

	while ((c = *fmt++) != 0 ) {
		if (c != '%') {
			(*outfunc)(c);
			nchars++;
			continue;
		}
		c = *fmt++;
		wid = 0;
		islong = FALSE;
		if (c == '*') {
			wid = va_arg(*app,int);
			c = *fmt++;
		} else while (isdigit(c)) {
			wid = (wid * 10) + c - '0';
			c = *fmt++;
		}
		if (c == 'l') {
			islong = TRUE;
			c = *fmt++;
		}
		switch (c) {
			case '\0':
				n = 0;
				break;
			case 'c':
				(*outfunc)(va_arg(*app,int));
				n = 1;
				break;

			case 'd':
				if (!islong) {
					n = dfputi(outfunc, va_arg(*app,int), 10);
					break;
				}
				/* fall through */
			case 'D':
				n = dfputli(outfunc, va_arg(*app,long), 10);
				break;

			case 'o':
				n = dfputi(outfunc, va_arg(*app,int), 8);
				break;

			case 'x':
				if (!islong) {
					n = dfputi(outfunc, va_arg(*app,int), 16);
					break;
				}
				/* fall through */
			case 'X':
				n = dfputli(outfunc, va_arg(*app,long), 16);
				break;

			case 's':
				n = dfputs(outfunc, va_arg(*app,char *));
				break;

			case 'S': /* use wid as max width */
				n = dfputsn(outfunc, va_arg(*app,char *),wid);
				break;

			case 'f':
				n = dfputf(outfunc, va_arg(*app,int));
				break;

			case 'P': /* output padding -- pads total output to
					"wid" chars, using c as the pad char */
				wid -= nchars;
				/* fall through */

			case 'p': /* field padding -- puts out "wid"
					copies of c */
				n = 0;
				c = va_arg(*app,int);
				while (n < wid) {
					(*outfunc)(c);
					n++;
				}
				break;


			default:
				(*outfunc)(c);
				n = 1;
		}
		wid -= n;
		nchars += n;
		while (wid-- > 0) {
			(*outfunc)(' ');
			nchars++;
		}
	}

}

/******************************************************************************/

/*
 * Line-number mode
 */
int
nu_mode(wp)
WINDOW *wp;
{
	register BUFFER *bp = wp->w_bufp;
	return	!(bp->b_flag & (BFINVS|BFSCRTCH)) && w_val(wp,WMDNUMBER);
}

int
nu_width(wp)
WINDOW *wp;
{
	return nu_mode(wp) ? NU_WIDTH : 0;
}

int
col_limit(wp)
WINDOW *wp;
{
	return term.t_ncol - 1 - nu_width(wp);
}

/*
 * Initialize the data structures used by the display code. The edge vectors
 * used to access the screens are set up. The operating system's terminal I/O
 * channel is set up. All the other things get initialized at compile time.
 * The original window has "WFCHG" set, so that it will get completely
 * redrawn on the first call to "update".
 */
void
vtinit()
{
    register int i;
    register VIDEO *vp;

    TTopen();		/* open the screen */
    TTkopen();		/* open the keyboard */
    TTrev(FALSE);
    vscreen = typeallocn(VIDEO *,term.t_mrow);

    if (vscreen == NULL)
        exit(BAD(1));


#if	! MEMMAP
    pscreen = typeallocn(VIDEO *,term.t_mrow);

    if (pscreen == NULL)
        exit(BAD(1));
#endif

    for (i = 0; i < term.t_mrow; ++i) {
    	/* struct VIDEO already has 4 of the bytes */
        vp = typeallocplus(VIDEO, term.t_mcol - 4);

        if (vp == NULL)
            exit(BAD(1));


	/* unnecessary */
	/* (void)memset(vp, ' ', sizeof(struct VIDEO) + term.t_mcol - 4); */
	vp->v_flag = 0;
#if	COLOR
	vp->v_rfcolor = gfcolor;
	vp->v_rbcolor = gbcolor;
#endif
        vscreen[i] = vp;
#if	! MEMMAP
    	/* struct VIDEO already has 4 of the bytes */
        vp = typeallocplus(VIDEO, term.t_mcol - 4);

        if (vp == NULL)
            exit(BAD(1));

	/* unnecessary */
	/* (void)memset(vp, 0, sizeof(struct VIDEO) + term.t_mcol - 4); */

	vp->v_flag = 0;
        pscreen[i] = vp;
#endif
        }
}

/*
 * Clean up the virtual terminal system, in anticipation for a return to the
 * operating system. Move down to the last line and advance, to make room
 * for the system prompt. Shut down the channel to the
 * terminal.
 */
void
vttidy(f)
int f;
{
	ttclean(f);	/* does it all now */
}

/*
 * Set the virtual cursor to the specified row and column on the virtual
 * screen. There is no checking for nonsense values.
 */
void
vtmove(row, col)
int row,col;
{
    vtrow = row;
    vtcol = col;
}

/* Write a character to the virtual screen. The virtual row and
   column are updated. If we are not yet on left edge, don't print
   it yet. If the line is too long put a ">" in the last column.
   This routine only puts printing characters into the virtual
   terminal buffers. Only column overflow is checked.
*/

void
vtputc(c)
int c;
{
	register VIDEO *vp;	/* ptr to line being updated */

	vp = vscreen[vtrow];

	if (isprint(c) && vtcol >= 0 && vtcol < term.t_ncol) {
		vp->v_text[vtcol] = c;
		++vtcol;
		return;
	}
	if (c == '\t') {
		do {
			vtputc(' ');
		} while (((vtcol + taboff)%curtabval) != 0);
	} else if (c == '\n') {
		return;
	} else if (vtcol >= term.t_ncol) {
		++vtcol;
		vp->v_text[term.t_ncol - 1] = MRK_EXTEND_RIGHT;
	} else if (isprint(c)) {
		++vtcol;
	} else {
		vtputc('^');
		vtputc(toalpha(c));
	}
}

/* as above, but tabs and newlines are made visible */
void
vtlistc(c)
int c;
{
	register VIDEO *vp;	/* ptr to line being updated */

	vp = vscreen[vtrow];

	if (vtcol >= term.t_ncol) {
		++vtcol;
		vp->v_text[term.t_ncol - 1] = MRK_EXTEND_RIGHT;
	} else if (isprint(c)) {
		if (vtcol >= 0)
			vp->v_text[vtcol] = c;
		++vtcol;
	} else {
		vtputc('^');
		vtputc(toalpha(c));
	}
}

int
vtgetc(col)
int col;
{
	return vscreen[vtrow]->v_text[col];
}

void
vtputsn(s,n)
char *s;
int n;
{
	int c;
	while (n-- && (c = *s++) != 0)
		vtputc(c);
}


void
vtset(lp,wp)
LINE *lp;
WINDOW *wp;
{
	register char *from = lp->l_text;
	register int n = llength(lp);
	int	skip = -vtcol,
		list = w_val(wp,WMDLIST);

	if (nu_mode(wp)) {
		register int j, k, jk;
		int	line = line_no(wp->w_bufp, lp);
		int	fill = ' ';
		char	temp[NU_WIDTH+2];

		(void)sprintf(temp, "%*d  ", NU_WIDTH-2, line);
		vtcol = 0;	/* make sure we always see line numbers */
		vtputsn(temp, NU_WIDTH);
		taboff = skip - vtcol;

		/* account for leading fill; this repeats logic in vtputc so
		 * I don't have to introduce a global variable... */
		for (j = k = jk = 0; (j < n) && (k < skip); j++) {
			register int	c = from[j];
			if (list && !isprint(c)) {
				k += 2;
				fill = toalpha(c);
			} else {
				if (c == '\t')
					k += (curtabval - (k % curtabval));
				else if (isprint(c))
					k++;
				fill = ' ';
			}
			jk = j+1;
		}
		while (k-- > skip)
			vtputc(fill);
		skip = jk;

		if (skip > 0) {
			n    -= skip;
			from += skip;
		}
	}

	while ((vtcol <= term.t_ncol) && (n > 0)) {
		if (list)
			vtlistc(*from++);
		else
			vtputc(*from++);
		n--;
	}
	if (list && (n >= 0))
		vtlistc('\n');
}

/* VARARGS1 */
void
#ifdef __STDC__
vtprintf( char *fmt, ...)
#else
vtprintf(va_alist)
va_dcl
#endif
{

	va_list ap;
#ifdef __STDC__
	va_start(ap,fmt);
#else
	va_start(ap);
#endif

	dfoutfn = vtputc;

#ifdef __STDC__
	dofmt(fmt,&ap);
#else
	dofmt(&ap);
#endif
	va_end(ap);

}

/*
 * Erase from the end of the software cursor to the end of the line on which
 * the software cursor is located.
 */
void
vteeol()
{
	if (vtcol < term.t_ncol) {
		(void)memset(&vscreen[vtrow]->v_text[vtcol],
			' ', term.t_ncol-vtcol);
		vtcol = term.t_ncol;
	}
}

/* upscreen:	user routine to force a screen update
		always finishes complete update		*/
/* ARGSUSED */
int
upscreen(f, n)
int f,n;
{
	update(TRUE);
	return(TRUE);
}

int scrflags;
/*
 * Make sure that the display is right. This is a three part process. First,
 * scan through all of the windows looking for dirty ones. Check the framing,
 * and refresh the screen. Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the virtual and physical
 * screens the same.
 */
int
update(force)
int force;	/* force update past type ahead? */
{
	register WINDOW *wp;
	register int screencol;

	if (!curbp) /* not initialized */
		return FALSE;
#if	TYPEAH
	if (force == FALSE && typahead())
		return SORTOFTRUE;
#endif
#if	VISMAC == 0
	if (force == FALSE && (get_recorded_char(FALSE) != -1))
		return SORTOFTRUE;
#endif

	displaying = TRUE;

	/* first, propagate mode line changes to all instances of
		a buffer displayed in more than one window */
	for_each_window(wp) {
		if (wp->w_flag & WFMODE) {
			if (wp->w_bufp->b_nwnd > 1) {
				/* make sure all previous windows have this */
				register WINDOW *owp;
				for_each_window(owp)
					if (owp->w_bufp == wp->w_bufp)
						owp->w_flag |= WFMODE;
			}
		}
	}

	/* update any windows that need refreshing */
	for_each_window(wp) {
		if (wp->w_flag) {
			curtabval = tabstop_val(wp->w_bufp);
			/* if the window has changed, service it */
			reframe(wp);	/* check the framing */
			if (wp->w_flag & (WFKILLS|WFINS)) {
				scrflags |= (wp->w_flag & (WFINS|WFKILLS));
				wp->w_flag &= ~(WFKILLS|WFINS);
			}
			if ((wp->w_flag & ~(/* WFMOVE| */WFMODE)) == WFEDIT)
				updone(wp);	/* update EDITed line */
			else if (wp->w_flag & ~(WFMOVE))
				updall(wp);	/* update all lines */
			if (scrflags || (wp->w_flag & WFMODE))
				modeline(wp);	/* update modeline */
			wp->w_flag = 0;
			wp->w_force = 0;
		}
	}
	curtabval = tabstop_val(curbp);

	/* recalc the current hardware cursor location */
	screencol = updpos();

#if	MEMMAP
	/* update the cursor and flush the buffers */
	/* movecursor(currow, screencol); */
#endif

	/* check for lines to de-extend */
	upddex();

	/* if screen is garbage, re-plot it */
	if (sgarbf)
		updgar();

	/* update the virtual screen to the physical screen */
	updupd(force);

	/* update the cursor and flush the buffers */
	movecursor(currow, screencol + nu_width(curwp));

	TTflush();
	displaying = FALSE;
	while (chg_width || chg_height)
		newscreensize(chg_height,chg_width);
	return(TRUE);
}

/*	reframe:	check to see if the cursor is on in the window
			and re-frame it if needed or wanted		*/
void
reframe(wp)
WINDOW *wp;
{
	register LINE *lp;
	register int i = 0;

	/* if not a requested reframe, check for a needed one */
	if ((wp->w_flag & WFFORCE) == 0) {
#if SCROLLCODE && ! MEMMAP
		/* loop from one line above the window to one line after */
		lp = lback(wp->w_line.l);
		for (i = -1; i <= wp->w_ntrows; i++)
#else
		/* loop through the window */
		lp = wp->w_line.l;
		for (i = 0; i < wp->w_ntrows; i++)
#endif
		{

			/* if the line is in the window, no reframe */
			if (lp == wp->w_dot.l) {
#if SCROLLCODE && ! MEMMAP
				/* if not _quite_ in, we'll reframe gently */
				if ( i < 0 || i == wp->w_ntrows) {
					/* if the terminal can't help, then
						we're simply outside */
					if (term.t_scroll == NULL)
						i = wp->w_force;
					break;
				}
#endif
				return;
			}

			/* if we are at the end of the file, reframe */
			if (i >= 0 && lp == wp->w_bufp->b_line.l)
				break;

			/* on to the next line */
			lp = lforw(lp);
		}
	}

#if SCROLLCODE && ! MEMMAP
	if (i == -1) {	/* we're just above the window */
		i = 1;	/* put dot at first line */
		scrflags |= WFINS;
	} else if (i == wp->w_ntrows) { /* we're just below the window */
		i = -1;	/* put dot at last line */
		scrflags |= WFKILLS;
	} else /* put dot where requested */
#endif
		i = wp->w_force;  /* (is 0, unless reposition() was called) */

	wp->w_flag |= WFMODE;
	
	/* w_force specifies which line of the window dot should end up on */
	/* 	positive --> lines from the top				*/
	/* 	negative --> lines from the bottom			*/
	/* 	zero --> middle of window				*/
	
	/* enforce some maximums */
	if (i > 0) {
		if (--i >= wp->w_ntrows)
			i = wp->w_ntrows - 1;
	} else if (i < 0) {	/* negative update???? */
		i += wp->w_ntrows;
		if (i < 0)
			i = 0;
	} else
		i = wp->w_ntrows / 2;

	/* backup to new line at top of window */
	lp = wp->w_dot.l;
	while (i != 0 && lback(lp) != wp->w_bufp->b_line.l) {
		--i;
		lp = lback(lp);
	}

	if (lp == wp->w_bufp->b_line.l)
		lp = lback(lp);
		
	/* and reset the current line-at-top-of-window */
	wp->w_line.l = lp;
	wp->w_flag |= WFHARD;
	wp->w_flag &= ~WFFORCE;
}

/*	updone:	update the current line	to the virtual screen		*/

void
updone(wp)
WINDOW *wp;	/* window to update current line in */
{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */

	/* search down the line we want */
	lp = wp->w_line.l;
	sline = wp->w_toprow;
	while (lp != wp->w_dot.l) {
		++sline;
		lp = lforw(lp);
	}

	l_to_vline(wp,lp,sline);
	vteeol();
}

/*	updall:	update all the lines in a window on the virtual screen */

void
updall(wp)
WINDOW *wp;	/* window to update lines in */
{
	register LINE *lp;	/* line to update */
	register int sline;	/* physical screen line to update */

	/* search down the lines, updating them */
	lp = wp->w_line.l;
	sline = wp->w_toprow;
	while (sline < wp->w_toprow + wp->w_ntrows) {
		l_to_vline(wp,lp,sline);
		vteeol();
		if (lp != wp->w_bufp->b_line.l)
			lp = lforw(lp);
		++sline;
	}

}

/* line to virtual screen line */
void
l_to_vline(wp,lp,sline)
WINDOW *wp;	/* window to update lines in */
LINE *lp;
int sline;
{

	/* and update the virtual line */
	vscreen[sline]->v_flag |= VFCHG;
	vscreen[sline]->v_flag &= ~VFREQ;
	if (w_val(wp,WVAL_SIDEWAYS))
		taboff = w_val(wp,WVAL_SIDEWAYS);
	if (lp != wp->w_bufp->b_line.l) {
		vtmove(sline, -w_val(wp,WVAL_SIDEWAYS));
		vtset(lp, wp);
		if (w_val(wp,WVAL_SIDEWAYS)) {
			register int	zero = nu_width(wp);
			vscreen[sline]->v_text[zero] = MRK_EXTEND_LEFT;
			if (vtcol <= zero) vtcol = zero+1;
		}
	} else {
		vtmove(sline, 0);
		vtputc(MRK_EMPTY);
	}
	taboff = 0;
#if	COLOR
	vscreen[sline]->v_rfcolor = w_val(wp,WVAL_FCOLOR);
	vscreen[sline]->v_rbcolor = w_val(wp,WVAL_BCOLOR);
#endif
}

/*	updpos:	update the position of the hardware cursor and handle extended
		lines. This is the only update for simple moves.
		returns the screen column for the cursor	*/
int
updpos()
{
	register LINE *lp;
	register int c;
	register int i;
	register int col, excess;

	/* find the current row */
	lp = curwp->w_line.l;
	currow = curwp->w_toprow;
	while (lp != DOT.l) {
		++currow;
		lp = lforw(lp);
		if (lp == curwp->w_line.l) {
			mlforce("BUG:  lost dot updpos().  setting at top");
			curwp->w_line.l = DOT.l  = lforw(curbp->b_line.l);
			currow = curwp->w_toprow;
		}
	}

	/* find the current column */
	col = 0;
	i = 0;
	while (i < DOT.o) {
		c = lgetc(lp, i++);
		if (c == '\t' && !w_val(curwp,WMDLIST)) {
			do {
				col++;
			} while ((col%curtabval) != 0);
		} else {
			if (!isprint(c))
				++col;
			++col;
		}

	}
	/* ...adjust to offset from shift-margin */
	curcol = col - w_val(curwp,WVAL_SIDEWAYS);

	/* if extended, flag so and update the virtual line image */
	if ((excess = curcol - col_limit(curwp)) >= 0) {
		return updext_past(col, excess);
	} else if (w_val(curwp,WVAL_SIDEWAYS) && (curcol < 1)) {
		return updext_before(col);
	} else {
		if (vscreen[currow]->v_flag & VFEXT) {
			l_to_vline(curwp,lp,currow);
			vteeol();
			/* this line no longer is extended */
			vscreen[currow]->v_flag &= ~VFEXT;
		}
		return curcol;
	}
}

/*	upddex:	de-extend any line that deserves it		*/

void
upddex()
{
	register WINDOW *wp;
	register LINE *lp;
	register int i;

	for_each_window(wp) {
		lp = wp->w_line.l;
		i = wp->w_toprow;

		curtabval = tabstop_val(wp->w_bufp);

		while (i < wp->w_toprow + wp->w_ntrows) {
			if (vscreen[i]->v_flag & VFEXT) {
				if ((wp != curwp)
				 || (lp != wp->w_dot.l)
				 || ((i != currow)
				  && (curcol < col_limit(wp)))) {
					l_to_vline(wp,lp,i);
					vteeol();
					/* this line no longer is extended */
					vscreen[i]->v_flag &= ~VFEXT;
				}
			}
			lp = lforw(lp);
			++i;
		}
	}
	curtabval = tabstop_val(curbp);
}

/*	updgar:	if the screen is garbage, clear the physical screen and
		the virtual screen and force a full update		*/

extern char mlsave[];

void
updgar()
{
	register char *txt;
	register int i,j;

	for (i = 0; i < term.t_nrow; ++i) {
		vscreen[i]->v_flag |= VFCHG;
#if	REVSTA
		vscreen[i]->v_flag &= ~VFREV;
#endif
#if	COLOR
		vscreen[i]->v_fcolor = gfcolor;
		vscreen[i]->v_bcolor = gbcolor;
#endif
#if	! MEMMAP
		txt = pscreen[i]->v_text;
		for (j = 0; j < term.t_ncol; ++j)
			txt[j] = ' ';
#endif
	}

	movecursor(0, 0);		 /* Erase the screen. */
	TTeeop();
	sgarbf = FALSE;			 /* Erase-page clears */
	mpresf = FALSE;			 /* the message area. */
	if (mlsave[0]) {
		mlforce(mlsave);
	}
#if	COLOR
	else
		mlerase();		/* needs to be cleared if colored */
#endif
}

/*	updupd:	update the physical screen from the virtual screen	*/

void
updupd(force)
int force;	/* forced update flag */
{
	register VIDEO *vp1;
	register int i;
#if SCROLLCODE && ! MEMMAP
	if (scrflags & WFKILLS)
		scrolls(FALSE);
	if (scrflags & WFINS)
		scrolls(TRUE);
	scrflags = 0;
#endif

	for (i = 0; i < term.t_nrow; ++i) {
		vp1 = vscreen[i];

		/* for each line that needs to be updated*/
		if ((vp1->v_flag & VFCHG) != 0) {
#if	TYPEAH
			if (force == FALSE && typahead())
				return;
#endif
#if	MEMMAP
			updateline(i, vp1, vp1);
#else
			updateline(i, vp1, pscreen[i]);
#endif
		}
	}
}

#if SCROLLCODE && ! MEMMAP
/* optimize out scrolls (line breaks, and newlines) */
/* arg. chooses between looking for inserts or deletes */
int	
scrolls(inserts)	/* returns true if it does something */
int inserts;
{
	struct	VIDEO *vpv ;	/* virtual screen image */
	struct	VIDEO *vpp ;	/* physical screen image */
	int	i, j, k ;
	int	rows, cols ;
	int	first, match, count, ptarget = 0, vtarget = 0, end ;
	int	longmatch, longcount;
	int	from, to;

	if (!term.t_scroll) /* no way to scroll */
		return FALSE;

	rows = term.t_nrow ;
	cols = term.t_ncol ;

	first = -1 ;
	for (i = 0; i < rows; i++) {	/* find first wrong line */
		if (!texttest(i,i)) {
			first = i;
			break;
		}
	}

	if (first < 0)
		return FALSE;		/* no text changes */

	vpv = vscreen[first] ;
	vpp = pscreen[first] ;

	if (inserts) {
		/* determine types of potential scrolls */
		end = endofline(vpv->v_text,cols) ;
		if ( end == 0 )
			ptarget = first ;		/* newlines */
		else if ( strncmp(vpp->v_text, vpv->v_text, end) == 0 )
			ptarget = first + 1 ;	/* broken line newlines */
		else
			ptarget = first ;
		from = ptarget;
	} else {
		from = vtarget = first + 1 ;
	}

	/* find the matching shifted area */
	longmatch = -1;
	longcount = 0;
	for (i = from+1; i < rows; i++) {
		if (inserts ? texttest(i,from) : texttest(from,i) ) {
			match = i ;
			count = 1 ;
			for (j=match+1, k=from+1; j<rows && k<rows; j++, k++) {
				if (inserts ? texttest(j,k) : texttest(k,j))
					count++ ;
				else
					break ;
			}
			if (longcount < count) {
				longcount = count;
				longmatch = match;
			}
		}
	}
	match = longmatch;
	count = longcount;

	if (!inserts) {
		/* full kill case? */
		if (match > 0 && texttest(first, match-1)) {
			vtarget-- ;
			match-- ;
			count++ ;
		}
	}

	/* do the scroll */
	if (match>0 && count>2) {		 /* got a scroll */
		/* move the count lines starting at ptarget to match */
		/* mlwrite("scrolls: move the %d lines starting at %d to %d",
						count,ptarget,match);
		*/
		if (inserts) {
			from = ptarget;
			to = match;
		} else {
			from = match;
			to = vtarget;
		}
		scrscroll(from, to, count) ;
		for (i = 0; i < count; i++) {
			vpp = pscreen[to+i] ;
			vpv = vscreen[to+i];
			(void)strncpy(vpp->v_text, vpv->v_text, cols) ;
		}
		if (inserts) {
			from = ptarget;
			to = match;
		} else {
			from = vtarget+count;
			to = match+count;
		}
		for (i = from; i < to; i++) {
			char *txt;
			txt = pscreen[i]->v_text;
			for (j = 0; j < term.t_ncol; ++j)
				txt[j] = ' ';
			vscreen[i]->v_flag |= VFCHG;
		}
		return(TRUE) ;
	}
	return(FALSE) ;
}

/* move the "count" lines starting at "from" to "to" */
void
scrscroll(from, to, count)
int from, to, count;
{
	ttrow = ttcol = -1;
	(*term.t_scroll)(from,to,count);
}

int
texttest(vrow,prow)		/* return TRUE on text match */
int	vrow, prow ;		/* virtual, physical rows */
{
	struct	VIDEO *vpv = vscreen[vrow] ;	/* virtual screen image */
	struct	VIDEO *vpp = pscreen[prow]  ;	/* physical screen image */

	return (!memcmp(vpv->v_text, vpp->v_text, term.t_ncol)) ;
}

/* return the index of the first blank of trailing whitespace */
int	
endofline(s,n)
char 	*s;
int	n;
{
	int	i;
	for (i = n - 1; i >= 0; i--)
		if (s[i] != ' ') return(i+1) ;
	return(0) ;
}

#endif /* SCROLLCODE */


/*	updext_past: update the extended line which the cursor is currently
		on at a column greater than the terminal width. The line
		will be scrolled right or left to let the user see where
		the cursor is		*/
int
updext_past(col, excess)
int	col;
int	excess;
{
	register int rcursor;
	register int zero = nu_width(curwp);

	/* calculate what column the real cursor will end up in */
	rcursor = ((excess - 1) % term.t_scrsiz) + term.t_margin;
	taboff = col - rcursor;

	/* scan through the line outputing characters to the virtual screen */
	/* once we reach the left edge					*/

	/* start scanning offscreen */
	vtmove(currow, -taboff);
	vtset(DOT.l, curwp);

	/* truncate the virtual line, restore tab offset */
	vteeol();
	taboff = 0;

	/* and put a marker in column 1 */
	vscreen[currow]->v_text[zero] = MRK_EXTEND_LEFT;
	vscreen[currow]->v_flag |= (VFEXT | VFCHG);
	return rcursor;
}

/*	updext_before: update the extended line which the cursor is currently
		on at a column less than the terminal width. The line
		will be scrolled right or left to let the user see where
		the cursor is		*/
int
updext_before(col)
int	col;
{
	register int rcursor;

	curcol = col;

	/* calculate what column the real cursor will end up in */
	rcursor = (col % (term.t_ncol - term.t_margin));
	taboff = col - rcursor;

	/* scan through the line outputing characters to the virtual screen */
	/* once we reach the left edge					*/
	vtmove(currow, -taboff);	/* start scanning offscreen */
	vtset(DOT.l, curwp);

	/* truncate the virtual line, restore tab offset */
	vteeol();
	taboff = 0;

	if (col != rcursor) { /* ... put a marker in column 1 */
		vscreen[currow]->v_text[nu_width(curwp)] = MRK_EXTEND_LEFT;
		vscreen[currow]->v_flag |= VFEXT;
	}
	vscreen[currow]->v_flag |= (VFEXT|VFCHG);
	return rcursor;
}



/*
 * Update a single line. This does not know how to use insert or delete
 * character sequences; we are using VT52 functionality. Update the physical
 * row and column variables. It does try an exploit erase to end of line. The
 * RAINBOW version of this routine uses fast video.
 */
#if	MEMMAP
/*	UPDATELINE specific code for the IBM-PC and other compatibles */

void
updateline(row, vp1, vpdummy)

int row;		/* row of screen to update */
struct VIDEO *vp1;	/* virtual screen image */
struct VIDEO *vpdummy;	/* virtual screen image */

{
#if	COLOR
	scwrite(row, 0, term.t_ncol-1,
		vp1->v_text, vp1->v_rfcolor, vp1->v_rbcolor);
	vp1->v_fcolor = vp1->v_rfcolor;
	vp1->v_bcolor = vp1->v_rbcolor;
#else
	if (vp1->v_flag & VFREQ)
		scwrite(row, 0, term.t_ncol-1, vp1->v_text, 0, 7);
	else
		scwrite(row, 0, term.t_ncol-1, vp1->v_text, 7, 0);
#endif
	vp1->v_flag &= ~(VFCHG | VFCOL);	/* flag this line as changed */

}

#else

void
updateline(row, vp1, vp2)

int row;		/* row of screen to update */
struct VIDEO *vp1;	/* virtual screen image */
struct VIDEO *vp2;	/* physical screen image */

{
#if RAINBOW
/*	UPDATELINE specific code for the DEC rainbow 100 micro	*/

    register char *cp1;
    register char *cp2;
    register int nch;

    /* since we don't know how to make the rainbow do this, turn it off */
    flags &= (~VFREV & ~VFREQ);

    cp1 = &vp1->v_text[0];                    /* Use fast video. */
    cp2 = &vp2->v_text[0];
    putline(row+1, 1, cp1);
    nch = term.t_ncol;

    do
        {
        *cp2 = *cp1;
        ++cp2;
        ++cp1;
        }
    while (--nch);
    *flags &= ~VFCHG;
#else
/*	UPDATELINE code for all other versions		*/

	register char *cp1;
	register char *cp2;
	register char *cp3;
	register char *cp4;
	register char *cp5;
	register int nbflag;	/* non-blanks to the right flag? */
	int rev;		/* reverse video flag */
	int req;		/* reverse video request flag */


	/* set up pointers to virtual and physical lines */
	cp1 = &vp1->v_text[0];
	cp2 = &vp2->v_text[0];

#if	COLOR
	TTforg(vp1->v_rfcolor);
	TTbacg(vp1->v_rbcolor);
#endif

#if	REVSTA | COLOR
	/* if we need to change the reverse video status of the
	   current line, we need to re-write the entire line     */
	rev = (vp1->v_flag & VFREV) == VFREV;
	req = (vp1->v_flag & VFREQ) == VFREQ;
	if ((rev != req)
#if	COLOR
	    || (vp1->v_fcolor != vp1->v_rfcolor) || (vp1->v_bcolor != vp1->v_rbcolor)
#endif
#if	HP150
	/* the HP150 has some reverse video problems */
	    || req || rev
#endif
			) {
		movecursor(row, 0);	/* Go to start of line. */
		/* set rev video if needed */
		if (rev != req)
			TTrev(req);

		/* scan through the line and dump it to the screen and
		   the virtual screen array				*/
		cp3 = &vp1->v_text[term.t_ncol];
#if X11
		x_putline(row, cp1, cp3 - cp1);
#endif
		while (cp1 < cp3) {
#if !X11
			TTputc(*cp1);
#endif
			++ttcol;
			*cp2++ = *cp1++;
		}
		/* turn rev video off */
		if (rev != req)
			TTrev(FALSE);

		/* update the needed flags */
		vp1->v_flag &= ~VFCHG;
		if (req)
			vp1->v_flag |= VFREV;
		else
			vp1->v_flag &= ~VFREV;
#if	COLOR
		vp1->v_fcolor = vp1->v_rfcolor;
		vp1->v_bcolor = vp1->v_rbcolor;
#endif
		return;
	}
#else
	rev = FALSE;
#endif

	/* advance past any common chars at the left */
	if (!rev)
		while (cp1 != &vp1->v_text[term.t_ncol] && *cp1 == *cp2) {
			++cp1;
			++cp2;
		}

/* This can still happen, even though we only call this routine on changed
 * lines. A hard update is always done when a line splits, a massive
 * change is done, or a buffer is displayed twice. This optimizes out most
 * of the excess updating. A lot of computes are used, but these tend to
 * be hard operations that do a lot of update, so I don't really care.
 */
	/* if both lines are the same, no update needs to be done */
	if (cp1 == &vp1->v_text[term.t_ncol]) {
 		vp1->v_flag &= ~VFCHG;		/* flag this line is changed */
		return;
	}

	/* find out if there is a match on the right */
	nbflag = FALSE;
	cp3 = &vp1->v_text[term.t_ncol];
	cp4 = &vp2->v_text[term.t_ncol];

	if (!rev)
		while (cp3[-1] == cp4[-1]) {
			--cp3;
			--cp4;
			if (cp3[0] != ' ')		/* Note if any nonblank */
				nbflag = TRUE;		/* in right match. */
		}

	cp5 = cp3;

	/* Erase to EOL ? */
	if (nbflag == FALSE && eolexist == TRUE && (req != TRUE)) {
		while (cp5!=cp1 && cp5[-1]==' ')
			--cp5;

		if (cp3-cp5 <= 3)		/* Use only if erase is */
			cp5 = cp3;		/* fewer characters. */
	}

	movecursor(row, cp1 - &vp1->v_text[0]);	/* Go to start of line. */
#if	REVSTA
	TTrev(rev);
#endif

#if X11
	x_putline(row, cp1, cp5 - cp1 + 1);
#endif
	while (cp1 != cp5) {		/* Ordinary. */
#if !X11
		TTputc(*cp1);
#endif
		++ttcol;
		*cp2++ = *cp1++;
	}

	if (cp5 != cp3) {		/* Erase. */
		TTeeol();
		while (cp1 != cp3)
			*cp2++ = *cp1++;
	}
#if	REVSTA
	TTrev(FALSE);
#endif
	vp1->v_flag &= ~VFCHG;		/* flag this line as updated */
	return;
#endif
}
#endif


/*
 * Redisplay the mode line for the window pointed to by the "wp". This is the
 * only routine that has any idea of how the modeline is formatted. You can
 * change the modeline format by hacking at this routine. Called by "update"
 * any time there is a dirty window.
 */
void
modeline(wp)
WINDOW *wp;
{
	register int n;
	register BUFFER *bp;
	register lchar;		/* character to draw line in buffer with */
	int	left, col;

	n = wp->w_toprow+wp->w_ntrows;      	/* Location. */
	vscreen[n]->v_flag |= VFCHG | VFREQ | VFCOL;/* Redraw next time. */


#if	COLOR
	vscreen[n]->v_rfcolor = gbcolor;		/* black on */
	vscreen[n]->v_rbcolor = gfcolor;		/* white.....*/
#endif
	vtmove(n, 0);                       	/* Seek to right line. */
	if (wp == curwp) {				/* mark the current buffer */
		lchar = '=';
	} else {
#if	REVSTA
		if (revexist)
			lchar = ' ';
		else
#endif
			lchar = '-';
	}
	bp = wp->w_bufp;

	vtputc(lchar);
	if (b_val(bp, MDSHOWMODE)) {
		register int ic;
		ic = lchar;
		if (wp == curwp) {
			if (insertmode == INSERT)
				ic = 'I';
			else if (insertmode == REPLACECHAR)
				ic = 'R';
			else if (insertmode == OVERWRITE)
				ic = 'O';
		}
		vtputc(ic);
	}
	vtprintf("%c %s",lchar,bp->b_bname);
	if (b_val(bp,MDVIEW))
		vtputsn(" [view only]", 20);
	if (b_val(bp,MDDOS))
		vtputsn(" [dos-style]", 20);
	if (bp->b_flag&BFCHG)
		vtputsn(" [modified]", 20);
	if (bp->b_fname && bp->b_fname[0]) {
		char *p;
		p = shorten_path(bp->b_fname);
		if (p && strcmp(p,bp->b_bname) != 0) {
			if (!isspace(p[0])) {
				vtprintf(" is ");
#if MSDOS
				if (isupper(p[0]) && p[1] == ':') {
					vtprintf("%c:",p[0]);
					p += 2;
				}
#endif
				if (!is_pathname(p)
				 && !isShellOrPipe(p))
					vtprintf(".%c",slash);
			}
			vtprintf("%s",p);
		}
	}
	vtputc(' ');

	/* Pad to full width, then go back and overwrite right-end info */
	n = term.t_ncol;
	while (vtcol < n)
		vtputc(lchar);
		
	{ /* determine if top line, bottom line, or both are visible */
		LINE *lp = wp->w_line.l;
		int rows = wp->w_ntrows;
		char *msg = NULL;
		
		vtcol = n - 7;  /* strlen(" top ") plus a couple */
		while (rows--) {
			lp = lforw(lp);
			if (lp == wp->w_bufp->b_line.l) {
				msg = " bot ";
				break;
			}
		}
		if (lback(wp->w_line.l) == wp->w_bufp->b_line.l) {
			if (msg) {
				if (wp->w_line.l == wp->w_bufp->b_line.l)
					msg = " emp ";
				else
					msg = " all ";
			} else {
				msg = " top ";
			}
		}
		if (!msg)
			msg = " mid ";
		vtputsn(msg,20);
		vtputc(lchar);
		vtputc(lchar);

	}
#ifdef show_tabstop_on_modeline
	{ /* put in the tabstop value */
		int t = tabstop_val(wp->w_bufp);
		vtcol = n - 11;
		while (t) {
			vtputc((t%10)+'0');
			t /= 10;
			vtcol -= 2;
		}
	}
#endif

	/* mark column 80 */
	left = (w_val(wp,WVAL_SIDEWAYS) - nu_width(wp));
	n += left;
	col = 80 - left;

	if ((n > 80) && (col >= 0) && (vtgetc(col) == lchar)) {
		vtcol = col;
		vtputc('|');
	}
}

void
upmode()	/* update all the mode lines */
{
	register WINDOW *wp;

	for_each_window(wp)
		wp->w_flag |= WFMODE;
}

/*
 * Send a command to the terminal to move the hardware cursor to row "row"
 * and column "col". The row and column arguments are origin 0. Optimize out
 * random calls. Update "ttrow" and "ttcol".
 */
void
movecursor(row, col)
int row,col;
{
	if (row!=ttrow || col!=ttcol)
        {
	        ttrow = row;
	        ttcol = col;
	        TTmove(row, col);
        }
}




/*
 * Erase the message line. This is a special routine because the message line
 * is not considered to be part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed via a call to the flusher.
 */
void
mlerase()
{
    int i;

    if (mpresf == FALSE)
		return;
    movecursor(term.t_nrow, 0);
    if (discmd == FALSE)
    	return;

#if	COLOR
     TTforg(gfcolor);
     TTbacg(gbcolor);
#endif
    if (eolexist == TRUE)
	    TTeeol();
    else {
        for (i = 0; i < term.t_ncol - 1; i++)
            TTputc(' ');
        movecursor(term.t_nrow, 1);	/* force the move! */
        movecursor(term.t_nrow, 0);
    }
    TTflush();
    mpresf = FALSE;
}

char *mlsavep;
char mlsave[NSTRING];

void
mlsavec(c)
int c;
{
	if (mlsavep - mlsave < NSTRING-1) {
		*mlsavep++ = c;
		*mlsavep = '\0';
	}

}

/*
 * Write a message into the message line only if appropriate.
 */
/* VARARGS1 */
void
#ifdef __STDC__
mlwrite( char *fmt, ...)
#else
mlwrite(va_alist)
va_dcl
#endif
{
	va_list ap;
	/* if we are not currently echoing on the command line, abort this */
	if (global_b_val(MDTERSE) || dotcmdmode == PLAY || discmd == FALSE) {
		movecursor(term.t_nrow, 0);
		return;
	}
#ifdef __STDC__
	va_start(ap,fmt);
	mlmsg(fmt,&ap);
#else
	va_start(ap);
	mlmsg(&ap);
#endif
	va_end(ap);
}

/*	Put a string out to the message line regardless of the
	current $discmd setting. This is needed when $debug is TRUE
	and for the write-message and clear-message-line commands
	Also used for most errors, to be sure they're seen.
*/
/* VARARGS1 */
void
#ifdef __STDC__
mlforce(char *fmt, ...)
#else
mlforce(va_alist)
va_dcl
#endif
{
	va_list ap;
#ifdef __STDC__
	va_start(ap,fmt);
	mlmsg(fmt,&ap);
#else
	va_start(ap);
	mlmsg(&ap);
#endif
	va_end(ap);
}

/* VARARGS1 */
void
#ifdef __STDC__
mlprompt( char *fmt, ...)
#else
mlprompt(va_alist)
va_dcl
#endif
{
	va_list ap;
	int osgarbf = sgarbf;
	if (discmd == FALSE) {
		movecursor(term.t_nrow, 0);
		return;
	}
	sgarbf = FALSE;
#ifdef __STDC__
	va_start(ap,fmt);
	mlmsg(fmt,&ap);
#else
	va_start(ap);
	mlmsg(&ap);
#endif
	va_end(ap);
	sgarbf = osgarbf;
}

/* VARARGS */
void
#ifdef __STDC__
dbgwrite( char *fmt, ...)
#else
dbgwrite(va_alist)
va_dcl
#endif
{
	va_list ap;	/* ptr to current data field */
#ifdef __STDC__
	va_start(ap,fmt);
	mlmsg(fmt,&ap);
#else
	va_start(ap);
	mlmsg(&ap);
#endif
	va_end(ap);
	TTgetc();
}

/*
 * Do the real message-line work.  Keep track of the physical cursor
 * position. A small class of printf like format items is handled.
 * Set the "message line" flag TRUE.
 */
void
#ifdef __STDC__
mlmsg( char *fmt, va_list *app)
#else
mlmsg(app)
va_list *app;	/* ptr to current data field */
#endif
{

	if (sgarbf) {
		/* then we'll lose the message on the next update(), so save it now */
		mlsavep = mlsave;
		dfoutfn = mlsavec;
#ifdef __STDC__
		dofmt(fmt,app);
#else
		dofmt(app);
#endif
		return;
	}

#if	COLOR
	/* set up the proper colors for the command line */
	TTforg(gfcolor);
	TTbacg(gbcolor);
#endif

	/* if we cannot erase to end-of-line, do it manually */
	if (eolexist == FALSE) {
		mlerase();
		TTflush();
	}


	movecursor(term.t_nrow, 0);

	dfoutfn = mlputc;
#ifdef __STDC__
	dofmt(fmt,app);
#else
	dofmt(app);
#endif

	/* if we can, erase to the end of screen */
	if (eolexist == TRUE)
		TTeeol();
	TTflush();
	mpresf = TRUE;
	mlsave[0] = '\0';
}

/*
 * Write out a character. Update the physical cursor position. This assumes that
 * the character has width "1"; if this is not the case
 * things will get screwed up a little.
 */
void
mlputc(c)
int c;
{
	if (c == '\r') ttcol = 0;
	if (c == '\t') c = ' ';
	if (ttcol < term.t_ncol-1) {
	        TTputc(c);
	        ++ttcol;
	}
}

/*
 * Local sprintf -- similar to standard libc, but
 *  returns pointer to null character at end of buffer, so it can
 *  be called repeatedly, as in:
 *	cp = lsprintf(cp, fmt, args...);
 *
 */

char *lsp;

void
lspputc(c)
int c;
{
	*lsp++ = c;
}

/* VARARGS1 */
char *
#ifdef __STDC__
lsprintf( char *buf, char *fmt, ...)
#else
lsprintf(va_alist)
va_dcl
#endif
{

	va_list ap;
#ifdef __STDC__
	va_start(ap,fmt);
#else
	char *buf;
	va_start(ap);
	buf = va_arg(ap, char *);
#endif

	lsp = buf;
	dfoutfn = lspputc;

#ifdef __STDC__
	dofmt(fmt,&ap);
#else
	dofmt(&ap);
#endif
	va_end(ap);

	*lsp = '\0';
	return lsp;
}

#ifdef	UNUSED
static char *lsbuf;

void
lssetbuf(buf)
char *buf;
{
	lsbuf = buf;
}

/* VARARGS1 */
char *
#ifdef __STDC__
_lsprintf( char *fmt, ...)
#else
_lsprintf(va_alist)
va_dcl
#endif
{

	va_list ap;
#ifdef __STDC__
	va_start(ap,fmt);
#else
	va_start(ap);
#endif

	lsp = lsbuf;
	dfoutfn = lspputc;

#ifdef __STDC__
	dofmt(fmt,&ap);
#else
	dofmt(&ap);
#endif
	va_end(ap);

	*lsp = '\0';
	return lsp;
}
#endif	/* UNUSED */

/*
 * Buffer printf -- like regular printf, but puts characters
 *	into the BUFFER.
 *
 */

void
bputc(c)
int c;
{
	if (c == '\n')
		lnewline();
	else
		linsert(1,c);
}

/* printf into curbp, at DOT */
/* VARARGS */
void
#ifdef __STDC__
bprintf( char *fmt, ...)
#else
bprintf(va_alist)
va_dcl
#endif
{
	va_list ap;

	dfoutfn = bputc;

#ifdef __STDC__
	va_start(ap,fmt);
	dofmt(fmt,&ap);
#else
	va_start(ap);
	dofmt(&ap);
#endif
	va_end(ap);

}

#if RAINBOW

putline(row, col, buf)
int row, col;
char buf[];
{
    int n;

    n = strlen(buf);
    if (col + n - 1 > term.t_ncol)
        n = term.t_ncol - col + 1;
    Put_Data(row, col, n, buf);
}
#endif

/* Get terminal size from system.
   Store number of lines into *heightp and width into *widthp.
   If zero or a negative number is stored, the value is not valid.  */

void
getscreensize (widthp, heightp)
int *widthp, *heightp;
{
#ifdef TIOCGWINSZ
	struct winsize size;
	*widthp = 0;
	*heightp = 0;
	if (ioctl (0, TIOCGWINSZ, &size) < 0)
		return;
	*widthp = size.ws_col;
	*heightp = size.ws_row;
#else
	*widthp = 0;
	*heightp = 0;
#endif
}

#if defined( SIGWINCH) && ! X11
/* ARGSUSED */
SIGT
sizesignal(signo)
int signo;
{
	int w, h;
	extern int errno;
	int old_errno = errno;

	getscreensize (&w, &h);

	if ((h && h-1 != term.t_nrow) || (w && w != term.t_ncol))
		newscreensize(h, w);

	(void) signal (SIGWINCH, sizesignal);
	errno = old_errno;
	SIGRET;
}
#endif

void
newscreensize (h, w)
int h, w;
{
	/* do the change later */
	if (displaying) {
		chg_width = w;
		chg_height = h;
		return;
	}
	chg_width = chg_height = 0;
	if (h - 1 < term.t_mrow)
		if (!newlength(TRUE,h))
			return;
	if (w < term.t_mcol)
		if (!newwidth(TRUE,w))
			return;

	update(TRUE);
}
