/*
 * The functions in this file handle redisplay. There are two halves, the
 * ones that update the virtual display screen, and the ones that make the
 * physical display screen the same as the virtual display screen. These
 * functions use hints that are left in the windows by the commands.
 *
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/display.c,v 1.172 1994/11/29 17:04:43 pgf Exp $
 *
 */

#include	"estruct.h"
#include        "edef.h"

#if DISP_BORLAND
#include 	<conio.h>
#endif

#define	NU_WIDTH 8

#define	MRK_EMPTY        '~'
#define	MRK_EXTEND_LEFT  '<'
#define	MRK_EXTEND_RIGHT '>'

VIDEO	**vscreen;			/* Virtual screen. */
VIDEO	**pscreen;			/* Physical screen. */
#if	MEMMAP
#define PSCREEN vscreen
#else
#define PSCREEN pscreen
#endif
int *lmap;

#if DISP_IBMPC
#define PScreen(n) scread((VIDEO *)0,n)
#else
#define	PScreen(n) pscreen[n]
#endif

#if OPT_SCROLLCODE && (DISP_IBMPC || !MEMMAP)
#define CAN_SCROLL 1
#else
#define CAN_SCROLL 0
#endif

static	int	displayed;

#ifdef WMDLINEWRAP
static	int	allow_wrap;
#endif

/* for window size changes */
int chg_width, chg_height;

/******************************************************************************/

typedef	void	(*OutFunc) P(( int ));

static	OutFunc	dfoutfn;

static	char *	right_num P(( char *, int, long ));
static	int	dfputs  P(( OutFunc, char * ));
static	int	dfputsn P(( OutFunc, char *, int ));
static	int	dfputi  P(( OutFunc, int,  int ));
static	int	dfputli P(( OutFunc, long, int ));
static	int	dfputf  P(( OutFunc, int ));
static	void	dofmt P(( char *, va_list * ));
static	void	mlmsg P(( char *, va_list * ));
static	void	erase_remaining_msg P(( int ));

static	void	l_to_vline P(( WINDOW *, LINEPTR, int ));
static	int	updpos P(( int *, int * ));
static	void	upddex P(( void ));
static	void	updgar P(( void ));
static	void	updone P(( WINDOW * ));
static	void	updall P(( WINDOW * ));
static	void	updupd P(( int ));
#if OPT_VIDEO_ATTRS
static	void	updattrs P(( WINDOW * ));
#endif
static	int	updext_past P(( int, int ));
static	int	updext_before P(( int ));
static	void	updateline P(( int, int, int ));
static	int	endofline P(( char *, int ));
#if	OPT_MLFORMAT
static	void	mlfs_prefix P(( char **, char **, int ));
static	void	mlfs_suffix P(( char **, char **, int ));
static	void	mlfs_skipfix P(( char ** ));
#endif
static	int	modeline_modes P(( BUFFER *, char ** ));
static	int	modeline_show P(( WINDOW *, int ));
static	char *	rough_position P(( WINDOW * ));
static	void	modeline P(( WINDOW * ));
#if	OPT_UPBUFF
static	void	recompute_buffer P(( BUFFER * ));
#endif
static	int	texttest P(( int, int ));
#if CAN_SCROLL
static	int	scrolls P(( int ));
#endif

static void freeVIDEO P((VIDEO * ));
static void vtmove P(( int, int ));
static void vtputc P(( int ));
static void vtlistc P(( int ));
static int vtgetc P(( int ));
static void vtputsn P(( char *, int ));
static void vtset P(( LINEPTR, WINDOW * ));
static void vteeol P(( void ));

static void lspputc P(( int ));

/*--------------------------------------------------------------------------*/

/*
 * Format a number, right-justified, returning a pointer to the formatted
 * buffer.
 */
static char *
right_num (buffer, len, value)
	char	*buffer;
	int	len;
	long	value;
{
	char	temp[NSTRING];
	register char	*p = lsprintf(temp, "%D", value);
	register char	*q = buffer + len;

	*q = EOS;
	while (q != buffer)
		*(--q) = (p != temp) ? *(--p) : ' ';
	return buffer;
}

/*
 * Do format a string.
 */
static
int	dfputs(outfunc, s)
	OutFunc outfunc;
	char *s;
{
	register int c;
	register int l = 0;

	while ((c = *s++) != EOS) {
	        (*outfunc)(c);
		l++;
	}
	return l;
}

/* as above, but takes a count for s's length */
static
int	dfputsn(outfunc,s,n)
	OutFunc outfunc;
	char *s;
	int n;
{
	register int c;
	register int l = 0;
	while ((n-- != 0) && ((c = *s++) != EOS)) {
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
	OutFunc outfunc;
	int i,r;
{
	register int q;

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
	OutFunc outfunc;
	long l;
	int r;
{
	register int q;

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
	OutFunc outfunc;
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
#if	ANSI_VARARGS
dofmt( char *fmt, va_list *app)
#else
dofmt(app)
va_list *app;
#endif
{
#if	!ANSI_VARARGS
	register char *fmt = va_arg(*app, char *);
#endif
	register int c;		/* current char in format string */
	register int wid;
	register int n;
	register int nchars = 0;
	int islong;
	OutFunc outfunc = dfoutfn;  /* local copy, for recursion */

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
			case EOS:
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
nu_width(wp)
WINDOW *wp;
{
	return w_val(wp,WMDNUMBER) ? NU_WIDTH : 0;
}

int
col_limit(wp)
WINDOW *wp;
{
#ifdef WMDLINEWRAP
	if (w_val(wp,WMDLINEWRAP))
		return curcol + 1;	/* effectively unlimited */
#endif
	return term.t_ncol - 1 - nu_width(wp);
}

/*
 * Initialize the data structures used by the display code. The edge vectors
 * used to access the screens are set up.
 */
int
vtinit()
{
    register int i;
    register VIDEO *vp;

    /* allocate new display memory */
    if (vtalloc() == FALSE) /* if we fail, only serious if not a realloc */
    	return (vscreen != NULL);

    for (i = 0; i < term.t_mrow; ++i) {

        vp = vscreen[i];
	vp->v_flag = 0;
#if OPT_COLOR
	ReqFcolor(vp) = gfcolor;
	ReqBcolor(vp) = gbcolor;
#endif
    }
#if OPT_WORKING
    imworking(0);
#endif
    return TRUE;
}

static void
freeVIDEO(vp)
register VIDEO *vp;
{
	if (vp != 0) {
#if OPT_VIDEO_ATTRS
		FreeIfNeeded (vp->v_attrs);
#endif
		free((char *)vp);
	}
}

int
video_alloc(vpp)
VIDEO **vpp;
{
	register VIDEO *vp;
	/* struct VIDEO already has 4 of the bytes */
	vp = typeallocplus(VIDEO, term.t_mcol - 4);
	if (vp == 0)
		return FALSE;
	(void)memset((char *)vp, 0, sizeof(VIDEO) + term.t_mcol - 4);

#if OPT_VIDEO_ATTRS
	vp->v_attrs = (VIDEO_ATTR *)calloc(sizeof(VIDEO_ATTR), (ALLOC_T)term.t_mcol);
	if (vp->v_attrs == 0) {
		free((char *)vp);
		return FALSE;
	}
#endif
	freeVIDEO(*vpp);
	*vpp = vp;
	return TRUE;
}

int
vtalloc()
{
	register int i, first;
	static int vcols, vrows;

	if (term.t_mrow > vrows) {
		GROW(vscreen, VIDEO *, vrows, term.t_mrow);
#if	! MEMMAP
		GROW(pscreen, VIDEO *, vrows, term.t_mrow);
#endif
		GROW(lmap, int, vrows, term.t_mrow);
	} else {
		for (i = term.t_mrow; i < vrows; i++) {
			freeVIDEO(vscreen[i]);
#if	! MEMMAP
			freeVIDEO(pscreen[i]);
#endif
		}
	}

	first = (term.t_mcol > vcols) ? 0 : vrows;

	for (i = first; i < term.t_mrow; ++i) {
		if (!video_alloc(&vscreen[i]))
			return FALSE;
#if	! MEMMAP
		if (!video_alloc(&pscreen[i]))
			return FALSE;
#endif	/* !MEMMAP */
	}
	vcols = term.t_mcol;
	vrows = term.t_mrow;

	return TRUE;
}

/* free all video memory, in anticipation of a (growing) resize */
#if NO_LEAKS
void
vtfree()
{
	register int i;

	if (vscreen) {
		for (i = 0; i < term.t_mrow; ++i) {
			freeVIDEO(vscreen[i]);
		}
		free ((char *)vscreen);
	}

#if	! MEMMAP
	if (pscreen) {
		for (i = 0; i < term.t_mrow; ++i) {
			freeVIDEO(pscreen[i]);
		}
		free ((char *)pscreen);
	}
#endif
	FreeIfNeeded (lmap);
}
#endif

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
static void
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

static void
vtputc(c)
int c;
{
	register VIDEO *vp;	/* ptr to line being updated */

	vp = vscreen[vtrow];

	/* XXX: should test for characters > 128, and highlight them */
	if (isprint(c) && vtcol >= 0 && vtcol < term.t_ncol) {
		VideoText(vp)[vtcol++] = (c & (N_chars-1));
#ifdef WMDLINEWRAP
		if ((allow_wrap != 0)
		 && (vtcol == term.t_ncol)
		 && (vtrow <  allow_wrap)) {
			vtcol = 0;
			vtrow++;
			vscreen[vtrow]->v_flag |= VFCHG;
		}
#endif
		return;
	}

	if (vtcol >= term.t_ncol) {
		VideoText(vp)[term.t_ncol - 1] = MRK_EXTEND_RIGHT;
	} else if (c == '\t') {
		do {
			vtputc(' ');
		} while (((vtcol + taboff)%curtabval) != 0 
		          && vtcol < term.t_ncol);
	} else if (c == '\n') {
		return;
	} else if (isprint(c)) {
		++vtcol;
	} else {
		vtlistc(c);
	}
}

/* how should high-bit unprintable chars be shown? */
static int vt_octal;

/* shows non-printing character */
static void
vtlistc(c)
int c;
{
	if (isprint(c)) {
	    vtputc(c);
	    return;
	}

	if (c & HIGHBIT) {
	    vtputc('\\');
	    if (vt_octal) {
		vtputc(((c>>6)&3)+'0');
		vtputc(((c>>3)&7)+'0');
		vtputc(((c   )&7)+'0');
	    } else {
		vtputc('x');
		vtputc(hexdigits[(c>>4) & 0xf]);
		vtputc(hexdigits[(c   ) & 0xf]);
	    }
	} else {
	    vtputc('^');
	    vtputc(toalpha(c));
	}
}

static int
vtgetc(col)
int col;
{
	return vscreen[vtrow]->v_text[col];
}

static void
vtputsn(s,n)
char *s;
int n;
{
	int c;
	while (n-- > 0 && (c = *s++) != EOS)
		vtputc(c);
}


static void
vtset(lp,wp)
LINEPTR lp;
WINDOW *wp;
{
	register char *from;
	register int n = lLength(lp);
	BUFFER	*bp  = wp->w_bufp;
	int	skip = -vtcol,
		list = w_val(wp,WMDLIST);

	vt_octal = w_val(wp,WMDNONPRINTOCTAL);

	if (w_val(wp,WMDNUMBER)) {
		register int j, k, jk;
		L_NUM	line = line_no(bp, lp);
		int	fill = ' ';
		char	temp[NU_WIDTH+2];

		vtcol = 0;	/* make sure we always see line numbers */
		vtputsn(right_num(temp, NU_WIDTH-2, (long)line), NU_WIDTH-2);
		vtputsn("  ", 2);
		taboff = skip - vtcol;

		/* account for leading fill; this repeats logic in vtputc so
		 * I don't have to introduce a global variable... */
		from = l_ref(lp)->l_text;
		for (j = k = jk = 0; (j < n) && (k < skip); j++) {
			register int	c = from[j];
			if ((list || (c != '\t')) && !isprint(c)) {
			    	if (c & HIGHBIT) {
				    k += 4;
				    fill = '\\';  /* FIXXXX */
				} else {
				    k += 2;
				    fill = toalpha(c);
				}
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
		if ((skip = jk) < 0)
			skip = 0;
		n -= skip;
	} else
		skip = 0;

#if OPT_B_LIMITS
	taboff -= w_left_margin(wp);
#endif
	from = l_ref(lp)->l_text + skip;
#ifdef WMDLINEWRAP
	allow_wrap = w_val(wp,WMDLINEWRAP) ? mode_row(wp) : 0;
#endif

	while ((vtcol <= term.t_ncol)
#ifdef WMDLINEWRAP
	  &&   (vtrow < mode_row(wp))
#endif
	  &&   (n > 0)) {
		if (list)
			vtlistc(*from++);
		else
			vtputc(*from++);
		n--;
	}

	/* Display a "^J" if 'list' mode is active, unless we've suppressed
	 * it for some reason.
	 */
	if (list && (n >= 0)) {
		if (b_is_scratch(bp) && listrimmed(l_ref(lp)))
			;
		else if (!b_val(bp,MDNEWLINE) && same_ptr(lFORW(lp),buf_head(bp)))
			;
		else
			vtlistc('\n');
	}
#ifdef WMDLINEWRAP
	allow_wrap = 0;
#endif
}

/*
 * Erase from the end of the software cursor to the end of the line on which
 * the software cursor is located.
 */
static void
vteeol()
{
	if (vtcol < term.t_ncol) {
		(void)memset(&vscreen[vtrow]->v_text[vtcol],
			' ', (SIZE_T)(term.t_ncol-vtcol));
		vtcol = term.t_ncol;
	}
}

/* upscreen:	user routine to force a screen update
		always finishes complete update		*/
#if !SMALLER
/* ARGSUSED */
int
upscreen(f, n)
int f,n;
{
	return update(TRUE);
}
#endif

static	int	scrflags;

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
	int origrow, origcol;
	int screenrow, screencol;

	/* Get row and column prior to doing the update in case we are
	 * reading the message line.
	 */
	origrow = ttrow;
	origcol = ttcol;

	if (!curbp) /* not initialized */
		return FALSE;
	if (force == FALSE && keystroke_avail())
		return SORTOFTRUE;
#if	OPT_VISIBLE_MACROS == 0
	if (force == FALSE && kbd_replaying(TRUE) && (get_recorded_char(FALSE) != -1))
		return SORTOFTRUE;
#endif

	beginDisplay;

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

	/* look for scratch-buffers that should be recomputed.  */
#if	OPT_UPBUFF
	for_each_window(wp)
		if (b_is_obsolete(wp->w_bufp))
			recompute_buffer(wp->w_bufp);
#endif

	/* look for windows that need the ruler updated */
#ifdef WMDRULER
	for_each_window(wp) {
		if (w_val(wp,WMDRULER)) {
			WINDOW	*save = curwp;
			int	line, col;

			curwp = wp;
			col   = getccol(FALSE) + 1;
			curwp = save;
			line  = line_no(wp->w_bufp, wp->w_dot.l);

			if (line != wp->w_ruler_line
			 || col  != wp->w_ruler_col) {
				wp->w_ruler_line = line;
				wp->w_ruler_col  = col;
				wp->w_flag |= WFMODE;
			}
		} else if (wp->w_flag & WFSTAT) {
			wp->w_flag |= WFMODE;
		}
		wp->w_flag &= ~WFSTAT;
	}
#endif

 restartupdate:

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
			if ((wp->w_flag & ~(WFMODE)) == WFEDIT)
				updone(wp);	/* update EDITed line */
			else if (wp->w_flag & ~(WFMOVE))
				updall(wp);	/* update all lines */
#if OPT_SCROLLBARS
			if (wp->w_flag & (WFHARD | WFSBAR))
				update_scrollbar(wp);
#endif /* OPT_SCROLLBARS */

#if OPT_VIDEO_ATTRS
			if (wp->w_flag & (WFHARD | WFEDIT))
				updattrs(wp);
#endif
			if (scrflags || (wp->w_flag & (WFMODE|WFCOLR)))
				modeline(wp);	/* update modeline */
			wp->w_flag = 0;
			wp->w_force = 0;
		}
	}
	curtabval = tabstop_val(curbp);

	/* recalc the current hardware cursor location */
	if (updpos(&screenrow, &screencol))
		/* if true, full horizontal scroll happened */
		goto restartupdate;

	/* check for lines to de-extend */
	upddex();

	/* if screen is garbage, re-plot it */
	if (sgarbf)
		updgar();

	/* update the virtual screen to the physical screen */
	updupd(force);

	/* update the cursor and flush the buffers */
	if (reading_msg_line)
	    movecursor(origrow, origcol);
	else
	    movecursor(screenrow, screencol);

	TTflush();
	endofDisplay;
	displayed  = TRUE;

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
	fast_ptr LINEPTR dlp;
	fast_ptr LINEPTR lp;
	register int i = 0;
	register int rows;
	int	founddot = FALSE;	/* set to true iff we find dot */
	int	tildecount;

	/* if not a requested reframe, check for a needed one */
	if ((wp->w_flag & WFFORCE) == 0) {
		/* initial update in main.c may not set these first... */
		if (l_ref(wp->w_dot.l) == (LINE *)0) {
			wp->w_dot.l = lFORW(win_head(wp));
			wp->w_dot.o = 0;
		}
		if (l_ref(wp->w_line.l) == (LINE *)0) {
			wp->w_line.l = wp->w_dot.l;
			wp->w_line.o = 0;
		}
#if CAN_SCROLL
		/* loop from one line above the window to one line after */
		lp = lBACK(wp->w_line.l);
		i  = -line_height(wp,lp);
#else
		/* loop through the window */
		lp = wp->w_line.l;
		i  = 0;
#endif
		for (;;) {
			/* if the line is in the window, no reframe */
			if (same_ptr(lp, wp->w_dot.l)) {
				founddot = TRUE;
#if CAN_SCROLL
				/* if not _quite_ in, we'll reframe gently */
				if ( i < 0 || i >= wp->w_ntrows) {
					/* if the terminal can't help, then
						we're simply outside */
					if (term.t_scroll == NULL)
						i = wp->w_force;
					break;
				}
#endif
#ifdef WMDLINEWRAP
				if (w_val(wp,WMDLINEWRAP)
				 && i > 0
				 && i + line_height(wp,lp) > wp->w_ntrows) {
					i = wp->w_ntrows;
					break;
				}
#endif
				lp = wp->w_line.l;
				goto kill_tildes;
			}

			/* if we are at the end of the file, reframe */
			if (i >= 0 && same_ptr(lp, win_head(wp)))
				break;

			/* on to the next line */
			if (i >= wp->w_ntrows) {
				i = 0;	/* dot-not-found */
				break;
			}
			i += line_height(wp,lp);
			lp = lFORW(lp);
		}
	}

#if CAN_SCROLL
	if (i < 0) {	/* we're just above the window */
		i = 1;	/* put dot at first line */
		scrflags |= WFINS;
	} else if (founddot && (i >= wp->w_ntrows)) {
		/* we're just below the window */
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

	lp = wp->w_dot.l;

#ifdef WMDLINEWRAP
	/*
	 * Center dot in middle of screen with line-wrapping
	 */
	if (i == 0 && w_val(wp,WMDLINEWRAP)) {
		rows = (wp->w_ntrows - line_height(wp,lp) + 2) / 2;
		while (rows > 0) {
			dlp = lBACK(lp);
			if (same_ptr(dlp, win_head(wp)))
				break;
			if ((rows -= line_height(wp, dlp)) < 0)
				break;
			lp = dlp;
		}
	} else
#endif
	{
		rows = (i != 0)
			? wp->w_ntrows
			: wp->w_ntrows / 2;
		while (rows > 0) {
			if ((i > 0)
			 && (--i <= 0))
				break;
			dlp = lBACK(lp);
			if (same_ptr(dlp, win_head(wp)))
				break;
			if ((rows -= line_height(wp, lp)) < 0)
				break;
			lp = dlp;
		}
		if (rows < line_height(wp, lp))
			while (i++ < 0) {
				dlp = lFORW(lp);
				if (!same_ptr(dlp, win_head(wp)))
					lp = dlp;
			}
	}
kill_tildes:
	/* Eliminate as many tildes as possible from bottom */
	dlp = lp;
	rows = wp->w_ntrows;
	while (rows > 0 && !same_ptr(dlp, win_head(wp))) {
		rows -= line_height(wp, dlp);
		dlp = lFORW(dlp);
	}
	dlp = lBACK(lp);

	tildecount = (wp->w_ntrows * ntildes)/100;
	if (tildecount == wp->w_ntrows)
		tildecount--;

	while (!same_ptr(dlp, win_head(wp)) 
	       && (rows -= line_height(wp, dlp)) >= tildecount) {
		lp = dlp;
		dlp = lBACK(lp);
	}

	/* and reset the current line-at-top-of-window */
	if (!same_ptr(lp, win_head(wp)) /* mouse click could be past end */
	 && !same_ptr(lp, wp->w_line.l)) { /* no need to set it if already there */
		wp->w_line.l = lp;
		wp->w_flag |= WFHARD;
		wp->w_flag &= ~WFFORCE;
	}
}

/*	updone:	update the current line	to the virtual screen		*/

static void
updone(wp)
WINDOW *wp;	/* window to update current line in */
{
	fast_ptr LINEPTR lp;	/* line to update */
	register int sline;	/* physical screen line to update */

	/* search down the line we want */
	lp = wp->w_line.l;
	sline = wp->w_toprow;
	while (!same_ptr(lp, wp->w_dot.l)) {
		sline += line_height(wp,lp);
		lp = lFORW(lp);
	}

	l_to_vline(wp,lp,sline);
	vteeol();
}

/*	updall:	update all the lines in a window on the virtual screen */

static void
updall(wp)
WINDOW *wp;	/* window to update lines in */
{
	fast_ptr LINEPTR lp;	/* line to update */
	register int sline;	/* physical screen line to update */

	/* search down the lines, updating them */
	lp = wp->w_line.l;
	sline = wp->w_toprow;
	while (sline < mode_row(wp)) {
		l_to_vline(wp,lp,sline);
		vteeol();
		sline += line_height(wp,lp);
		if (!same_ptr(lp, win_head(wp)))
			lp = lFORW(lp);
	}
}

/* line to virtual screen line */
static void
l_to_vline(wp,lp,sline)
WINDOW *wp;	/* window to update lines in */
LINEPTR lp;
int sline;
{
	C_NUM	left;

	/*
	 * Mark the screen lines changed, resetting the requests for reverse
	 * video.  Set the global 'taboff' to the amount of horizontal
	 * scrolling.
	 */
#ifdef WMDLINEWRAP
	if (w_val(wp,WMDLINEWRAP)) {
		register int	n = sline + line_height(wp, lp);
		while (n > sline)
			if (--n < mode_row(wp)) {
				vscreen[n]->v_flag |= VFCHG;
				vscreen[n]->v_flag &= ~VFREQ;
			}
		taboff = 0;
	} else
#endif
	{
		vscreen[sline]->v_flag |= VFCHG;
		vscreen[sline]->v_flag &= ~VFREQ;
		if (w_val(wp,WVAL_SIDEWAYS))
			taboff = w_val(wp,WVAL_SIDEWAYS);
	}
	left = taboff;

	if (!same_ptr(lp, win_head(wp))) {
		vtmove(sline, -left);
		vtset(lp, wp);
		if (left) {
			register int	zero = nu_width(wp);
			vscreen[sline]->v_text[zero] = MRK_EXTEND_LEFT;
			if (vtcol <= zero) vtcol = zero+1;
		}
	} else {
		vtmove(sline, 0);
		vtputc(MRK_EMPTY);
	}
	taboff = 0;
#if	OPT_COLOR
	ReqFcolor(vscreen[sline]) = w_val(wp,WVAL_FCOLOR);
	ReqBcolor(vscreen[sline]) = w_val(wp,WVAL_BCOLOR);
#endif
}

/*	updpos:	update the position of the hardware cursor and handle extended
		lines. This is the only update for simple moves.
		returns the screen column for the cursor, and
		a boolean indicating if full sideways scroll was necessary */
static int
updpos(screenrowp, screencolp)
int *screenrowp;
int *screencolp;
{
	fast_ptr LINEPTR lp;
	register int c;
	register int i;
	register int col, excess;
	register int collimit;
	int moved = FALSE;
	int nuadj = is_empty_buf(curwp->w_bufp) ? 0 : nu_width(curwp);

	/* find the current row */
	lp = curwp->w_line.l;
	currow = curwp->w_toprow;
	while (!same_ptr(lp, DOT.l)) {
		currow += line_height(curwp,lp);
		lp = lFORW(lp);
		if (same_ptr(lp, curwp->w_line.l)
		 || currow > mode_row(curwp)) {
			mlforce("BUG:  lost dot updpos().  setting at top");
			lp = curwp->w_line.l = DOT.l = lFORW(buf_head(curbp));
			currow = curwp->w_toprow;
		}
	}

	/* find the current column */
	col = 0;
	i = w_left_margin(curwp);
	while (i < DOT.o || (!global_g_val(GMDALTTABPOS) && !insertmode &&
				i <= DOT.o && i < lLength(lp))) {
		c = lGetc(lp, i++);
		if (c == '\t' && !w_val(curwp,WMDLIST)) {
			do {
				col++;
			} while ((col%curtabval) != 0);
		} else {
			if (!isprint(c)) {
				col += (c & HIGHBIT) ? 3 : 1;
			}
			++col;
		}

	}
	col += w_left_margin(curwp);
	if (!global_g_val(GMDALTTABPOS) && !insertmode &&
			col != 0 && DOT.o < lLength(lp))
		col--;

#ifdef WMDLINEWRAP
	if (w_val(curwp,WMDLINEWRAP)) {
		curcol = col;
		collimit = term.t_ncol - nuadj;
		*screenrowp = currow;
		if (col >= collimit) {
			col -= collimit;
			*screenrowp += 1;
			if (col >= term.t_ncol)
				*screenrowp += (col / term.t_ncol);
			*screencolp = col % term.t_ncol;
		} else {
			*screencolp = col + nuadj;
		}
		/* kludge to keep the cursor within the window */
		i = mode_row(curwp) - 1;
		if (*screenrowp > i) {
			*screenrowp = i;
			*screencolp = term.t_ncol - 1;
		}
		return FALSE;
	} else
#endif
	 *screenrowp = currow;

	/* ...adjust to offset from shift-margin */
	curcol = col - w_val(curwp,WVAL_SIDEWAYS);

	/* if extended, flag so and update the virtual line image */
	collimit = col_limit(curwp);
	excess = curcol - collimit;
	if ((excess > 0) || (excess == 0 &&
			(DOT.o < lLength(DOT.l) - 1 ))) {
		if (w_val(curwp,WMDHORSCROLL)) {
			(void)mvrightwind(TRUE, excess + collimit/2 );
			moved = TRUE;
		} else {
			*screencolp = updext_past(col, excess);
		}
	} else if (w_val(curwp,WVAL_SIDEWAYS) && (curcol < 1)) {
		if (w_val(curwp,WMDHORSCROLL)) {
			(void)mvleftwind(TRUE, -curcol + collimit/2 + 1);
			moved = TRUE;
		} else {
			*screencolp = updext_before(col);
		}
	} else {
		if (vscreen[currow]->v_flag & VFEXT) {
			l_to_vline(curwp,lp,currow);
			vteeol();
			/* this line no longer is extended */
			vscreen[currow]->v_flag &= ~VFEXT;
		}
		*screencolp = curcol;
	}
	if (!moved)
		*screencolp += nuadj;
	return moved;
}

/*	upddex:	de-extend any line that deserves it		*/

static void
upddex()
{
	register WINDOW *wp;
	fast_ptr LINEPTR lp;
	register int i;

	for_each_window(wp) {
		lp = wp->w_line.l;
		i = wp->w_toprow;

		curtabval = tabstop_val(wp->w_bufp);

		while (i < mode_row(wp)) {
			if (vscreen[i]->v_flag & VFEXT) {
				if ((wp != curwp)
				 || (!same_ptr(lp, wp->w_dot.l))
				 || ((i != currow)
				  && (curcol < col_limit(wp)))) {
					l_to_vline(wp,lp,i);
					vteeol();
					/* this line no longer is extended */
					vscreen[i]->v_flag &= ~VFEXT;
				}
			}
			i += line_height(wp,lp);
			lp = lFORW(lp);
		}
	}
	curtabval = tabstop_val(curbp);
}

/*	updgar:	if the screen is garbage, clear the physical screen and
		the virtual screen and force a full update		*/

extern char mlsave[];

static void
updgar()
{
#if !MEMMAP && !OPT_PSCREEN
	register char *txt;
	register int j;
#endif
	register int i;

	for (i = 0; i < term.t_nrow-1; ++i) {
		vscreen[i]->v_flag |= VFCHG;
#if	OPT_REVSTA
		vscreen[i]->v_flag &= ~VFREV;
#endif
#if	OPT_COLOR
		CurFcolor(vscreen[i]) = gfcolor;
		CurBcolor(vscreen[i]) = gbcolor;
#endif
#if	! MEMMAP && ! OPT_PSCREEN
		txt = pscreen[i]->v_text;
		for (j = 0; j < term.t_ncol; ++j) {
			txt[j] = ' ';
#if OPT_VIDEO_ATTRS
			/* FIXME: Color? */
			pscreen[i]->v_attrs[j] = 0;
#endif /* OPT_VIDEO_ATTRS */
		}
#endif
	}
#if !OPT_PSCREEN
	movecursor(0, 0);		 /* Erase the screen. */
	TTeeop();
#else
	bottomleft();
	TTeeol();
#endif
	sgarbf = FALSE;			 /* Erase-page clears */
	mpresf = 0;			 /* the message area. */
	if (mlsave[0]) {
		mlforce("%s", mlsave);
	}
#if	OPT_COLOR
	else
		mlerase();		/* needs to be cleared if colored */
#endif
}

/*	updupd:	update the physical screen from the virtual screen	*/

static void
updupd(force)
int force;	/* forced update flag */
{
	register int i;

#if CAN_SCROLL
	if (scrflags & WFKILLS)
		(void)scrolls(FALSE);
	if (scrflags & WFINS)
		(void)scrolls(TRUE);
	scrflags = 0;
#endif

	for (i = 0; i < term.t_nrow-1; ++i) {
		/* for each line that needs to be updated*/
		if ((vscreen[i]->v_flag & (VFCHG|VFCOL)) != 0) {
#if !DISP_X11
			if (force == FALSE && keystroke_avail())
				return;
#endif
			updateline(i, 0, term.t_ncol);
		}
	}
}

#if OPT_VIDEO_ATTRS
static void
updattrs(wp)
    WINDOW *wp;
{
    AREGION *ap;
    int i, j;
    
    L_NUM start_wlnum, end_wlnum;
    LINEPTR lp;
    int rows;

    /*
     * Clear portion of virtual screen associated with window pointer
     * of all attributes.
     */
    /* FIXME: color; need to set to value indicating fg and bg for window */
    for (i = wp->w_toprow + wp->w_ntrows - 1; i >= wp->w_toprow; i--)
	for (j = 0; j < term.t_ncol; j++)
	    vscreen[i]->v_attrs[j] = 0;

    /*
     * No need to do any more work on this window if there are no
     * attributes.
     */
    if (wp->w_bufp->b_attribs == NULL)
	return;

    /*
     * Compute starting and ending line numbers for the window.  We
     * also fill in lmap which is used for mapping line numbers to
     * screen row numbers.
     */
    lp = wp->w_line.l;
    start_wlnum = 
    end_wlnum = line_no(wp->w_bufp, lp);
    rows = wp->w_ntrows;
    lmap[end_wlnum - start_wlnum] = wp->w_toprow;
    while ( (rows -= line_height(wp,lp)) > 0) {
	lp = lFORW(lp);
	end_wlnum++;
	lmap[end_wlnum - start_wlnum] = wp->w_toprow + wp->w_ntrows - rows;
    }

    /*
     * Set current attributes in virtual screen associated with window
     * pointer.
     */
    for (ap = wp->w_bufp->b_attribs; ap != NULL;) {
	VIDEO_ATTR attr;
	C_NUM start_col, end_col;
	C_NUM rect_start_col = 0, rect_end_col = 0;
	L_NUM start_rlnum, end_rlnum, lnum, start_lnum, end_lnum;
	start_rlnum = line_no(wp->w_bufp, ap->ar_region.r_orig.l);
	end_rlnum = line_no(wp->w_bufp, ap->ar_region.r_end.l);

	/* Garbage collect empty visible regions */
	if (start_rlnum == end_rlnum
	 && ap->ar_vattr != 0
	 && ap->ar_region.r_orig.o >= ap->ar_region.r_end.o) {
	    AREGION *nap = ap->ar_next;
	    free_attrib(wp->w_bufp, ap);
	    ap = nap;
	    continue;
	}

	if (start_rlnum > start_wlnum) {
	    start_lnum = start_rlnum;
	    lp = ap->ar_region.r_orig.l;
	}
	else {
	    start_lnum = start_wlnum;
	    lp = wp->w_line.l;
	}
	end_lnum = (end_rlnum < end_wlnum) ? end_rlnum : end_wlnum;
	attr = ap->ar_vattr;
	if (ap->ar_shape == RECTANGLE) {
	    rect_start_col = offs2col(wp, ap->ar_region.r_orig.l,
				 ap->ar_region.r_orig.o);
	    rect_end_col = offs2col(wp, ap->ar_region.r_end.l,
			       ap->ar_region.r_end.o);
	    if (rect_end_col < rect_start_col) {
		    C_NUM col = rect_end_col;
		    rect_end_col = rect_start_col;
		    rect_start_col = col;
	    }
	}
	for (lnum = start_lnum; lnum <= end_lnum; lnum++, lp = lFORW(lp)) {
	    int row, col;
	    if (ap->ar_shape == RECTANGLE) {
		start_col = rect_start_col;
	    } else if (lnum == start_rlnum) {
		start_col = offs2col(wp, ap->ar_region.r_orig.l,
				     ap->ar_region.r_orig.o);
	    } else {
		start_col = 0;
	    }

	    if (start_col < 0)
		start_col = 0;

	    if (ap->ar_shape == RECTANGLE) {
		end_col = rect_end_col;
	    } else if (lnum == end_rlnum) {
		end_col = offs2col(wp, ap->ar_region.r_end.l,
				   ap->ar_region.r_end.o - 1);
	    } else {
		end_col = offs2col(wp, lp, lLength(lp));
	    }
	    row = lmap[lnum - start_wlnum];
#ifdef WMDLINEWRAP
	    if (w_val(wp,WMDLINEWRAP))
		for (col = start_col; col <= end_col; col++)
		    vscreen[row + col / term.t_ncol]->v_attrs[col % term.t_ncol] =
			(vscreen[row + col / term.t_ncol]->v_attrs[col % term.t_ncol]
			 | (attr & ~VAREV))
			^ (attr & VAREV);
	    else
#endif
	    {
		if (end_col >= term.t_ncol)
		    end_col = term.t_ncol-1;
		for (col = start_col; col <= end_col; col++)
		    vscreen[row]->v_attrs[col] = 
			(vscreen[row]->v_attrs[col] | (attr & ~VAREV))
			^ (attr & VAREV);
	    }
	}
	ap = ap->ar_next;
    }
}
#endif /* OPT_VIDEO_ATTRS */

/*
 * Translate offset (into a line's text) into the display-column, taking into
 * account the tabstop, sideways, number- and list-modes.
 */
int
offs2col(wp, lp, offset)
WINDOW	*wp;
LINEPTR	lp;
C_NUM	offset;
{
	int	length = lLength(lp);
	int	column = 0;
	int	tabs = tabstop_val(wp->w_bufp);
	int	list = w_val(wp,WMDLIST);
	int	left =
#ifdef WMDLINEWRAP	/* overrides left/right scrolling */
			w_val(wp,WMDLINEWRAP) ? 0 :
#endif
			w_val(wp,WVAL_SIDEWAYS);

	register C_NUM	n, c;

	/* this makes the how-much-to-select calculation easier above */
	if (offset < 0)
		return offset;

	if (same_ptr(lp, win_head(wp))) {
		column = 0;
	} else {
		for (n = w_left_margin(wp); (n < offset) && (n <= length); n++) {
			c = (n >= length) ? '\n' : l_ref(lp)->l_text[n];
			if (isprint(c)) {
				column++;
			} else if (list || (c != '\t')) {
				column += (c & HIGHBIT) ? 4 : 2;
			} else if (c == '\t') {
				column = ((column / tabs) + 1) * tabs;
			}
		}
		column = column - left + nu_width(wp) + w_left_margin(wp);
	}
	return column;
}

/*
 * Translate a display-column (assuming an infinitely-wide display) into the
 * line's offset, taking into account the tabstop, sideways, number and list
 * modes.
 */
#if OPT_MOUSE
int
col2offs(wp, lp, col)
WINDOW	*wp;
LINEPTR	lp;
C_NUM	col;
{
	int	tabs = tabstop_val(wp->w_bufp);
	int	list = w_val(wp,WMDLIST);
	int	left =
#ifdef WMDLINEWRAP	/* overrides left/right scrolling */
			w_val(wp,WMDLINEWRAP) ? 0 :
#endif
			w_val(wp,WVAL_SIDEWAYS);
	int	goal = col + left - nu_width(wp) - w_left_margin(wp);

	register C_NUM	n;
	register C_NUM	offset;
	register C_NUM	len	= llength(l_ref(lp));
	register char	*text	= l_ref(lp)->l_text;

	if (same_ptr(lp, win_head(wp))) {
		offset = 0;
	} else {
		for (offset = w_left_margin(wp), n = 0;
			(offset < len) && (n < goal);
				offset++) {
			register int c = text[offset];
			if (isprint(c)) {
				n++;
			} else if (list || (c != '\t')) {
				n += (c & HIGHBIT) ? 4 : 2;
			} else if (c == '\t') {
				n = ((n / tabs) + 1) * tabs;
			}
			if (n > goal)
				break;
		}
	}
	return offset;
}
#endif

/*
 * Compute the number of rows required for displaying a line.
 */
#ifdef WMDLINEWRAP
int
line_height(wp, lp)
WINDOW	*wp;
LINEPTR	lp;
{
	if (w_val(wp,WMDLINEWRAP)) {
		int	len = lLength(lp);
		if (len > 0) {
			return (offs2col(wp,lp,len) + term.t_ncol - 1)
				/ term.t_ncol;
		}
	}
	return 1;
}
#endif

/*
 * Given a row on the screen, determines which window it belongs to.  Returns
 * null only for the message line.
 */
#if defined(WMDLINEWRAP) || OPT_MOUSE
WINDOW *
row2window (row)
int	row;
{
	register WINDOW *wp;

	for_each_window(wp)
		if (row >= wp->w_toprow && row <= mode_row(wp))
			return wp;
	return 0;
}
#endif

/*
 * Highlight the requested portion of the screen.  We're mucking with the video
 * attributes on the line here, so this is NOT good code - it would be better
 * if there was an individual colour attribute per character, rather than per
 * row, but I didn't write the original code.  Anyway, hilite is called only
 * once so far, so it's not that big a deal.
 */
void
hilite(row, colfrom, colto, on)
int	row;		/* row to start highlighting */
int	colfrom;	/* column to start highlighting */
int	colto;		/* column to end highlighting */
int	on;		/* start highlighting */
{
#if !OPT_VIDEO_ATTRS
	register VIDEO *vp1 = vscreen[row];
#endif
#ifdef WMDLINEWRAP
	WINDOW	*wp = row2window(row);
	if (w_val(wp,WMDLINEWRAP)) {
		if (colfrom < 0)
			colfrom = 0;
		if (colfrom > term.t_ncol) {
			do {
				row++;
				colfrom -= term.t_ncol;
				colto   -= term.t_ncol;
				hilite(row, colfrom, colto, on);
			} while (colto > term.t_ncol);
			return;
		}
	}
#endif
	if (row < term.t_nrow-1 && (colfrom >= 0 || colto <= term.t_ncol)) {
		if (colfrom < 0)
			colfrom = 0;
		if (colto > term.t_ncol)
			colto = term.t_ncol;
#if OPT_VIDEO_ATTRS
		if (on) {
		    int col;
		    for (col=colfrom; col<colto; col++)
			vscreen[row]->v_attrs[col] |= VAREV;
		}
		else {
		    int col;
		    for (col=colfrom; col<colto; col++)
			vscreen[row]->v_attrs[col] &= ~VAREV;
		}
		vscreen[row]->v_flag |= VFCHG;
		updateline(row, 0, term.t_ncol);
#else /* OPT_VIDEO_ATTRS */
		if (on) {
			vp1->v_flag |= VFREQ;
		} else {
			vp1->v_flag &= ~VFREQ;
		}
		updateline(row, colfrom, colto);
#endif /* OPT_VIDEO_ATTRS */
	}
}

#if CAN_SCROLL
/* optimize out scrolls (line breaks, and newlines) */
/* arg. chooses between looking for inserts or deletes */
static int
scrolls(inserts)	/* returns true if it does something */
int inserts;
{
	struct	VIDEO *vpv ;	/* virtual screen image */
	struct	VIDEO *vpp ;	/* physical screen image */
	int	i, j, k ;
	int	rows, cols ;
	int	first, match, count, ptarget = 0, vtarget = 0;
	SIZE_T	end;
	int	longmatch, longcount;
	int	longinplace, inplace;	/* count of lines which are already
					   in the right place */
	int	from, to;

	if (!term.t_scroll) /* no way to scroll */
		return FALSE;

	rows = term.t_nrow -1;
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
	vpp = PScreen(first) ;

	if (inserts) {
		/* determine types of potential scrolls */
		end = endofline(vpv->v_text,cols) ;
		if ( end == 0 )
			ptarget = first ;		/* newlines */
		else if ( memcmp(vpp->v_text, vpv->v_text, end) == 0 )
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
	longinplace = 0;
	for (i = from+1; i < rows; i++) {
		if (inserts ? texttest(i,from) : texttest(from,i) ) {
			match = i ;
			count = 1 ;
			inplace = texttest(match, match) ? 1 : 0;
			for (j=match+1, k=from+1; j<rows && k<rows; j++, k++) {
				if (inserts ? texttest(j,k) : texttest(k,j)) {
					count++ ;
					if (texttest(j,j))
						inplace++;
				}
				else
					break ;
			}
			if (longcount - longinplace < count - inplace) {
				longcount = count;
				longmatch = match;
				longinplace = inplace;
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
#if OPT_PSCREEN
		/*
		 * Update lines _before_ the scroll so that they will
		 * be available for any updates which need to be done
		 * (due to a GraphicsExpose event in X11...these occur
		 * when scrolling a partially obscured window).  Note
		 * that in the typical case of scrolling a line or two
		 * that very few memory accesses are performed.  We
		 * mostly shuffle pointers around.
		 */
#define SWAP_PLINE(a, b) do { VIDEO *temp = pscreen[a];	\
			      pscreen[a] = pscreen[b];	\
			      pscreen[b] = temp; } while (0)
#define CLEAR_PLINE(a)  do {						\
			    pscreen[a]->v_flag |= VFCHG;		\
			    for (j = 0; j < term.t_ncol; j++) {		\
				pscreen[a]->v_text[j] = ' ';		\
				pscreen[a]->v_text[j] = 0; /* FIXME: color */ \
			    }						\
			  } while (0)
		if (from < to) {
		    /* FIXME: color */
		    for (i = from; i < to; i++)
			CLEAR_PLINE(i+count);
		    for (i = count-1; i >= 0; i--)
			SWAP_PLINE(from+i, to+i);
		}
		else {
		    /* FIXME: color */
		    for (i = to; i < from; i++)
			CLEAR_PLINE(i);
		    for (i = 0; i < count; i++)
			SWAP_PLINE(from+i, to+i);
		}
#endif /* OPT_PSCREEN */
		scrscroll(from, to, count) ;
#if !OPT_PSCREEN
		for (i = 0; i < count; i++) {
			vpp = PScreen(to+i) ;
			vpv = vscreen[to+i];
			(void)memcpy(vpp->v_text, vpv->v_text, (SIZE_T)cols) ;
		}
#if OPT_VIDEO_ATTRS && !MEMMAP
#define SWAP_ATTR_PTR(a, b) do { VIDEO_ATTR *temp = pscreen[a]->v_attrs;  \
			         pscreen[a]->v_attrs = pscreen[b]->v_attrs; \
			         pscreen[b]->v_attrs = temp; } while (0)
		if (from < to) {
		    /* FIXME: color */
		    for (i = from; i < to; i++)
			for (j = 0; j < term.t_ncol; j++)
			    pscreen[i+count]->v_attrs[j] = 0;
		    for (i = count-1; i >= 0; i--)
			SWAP_ATTR_PTR(from+i, to+i);

		}
		else {
		    /* FIXME: color */
		    for (i = to; i < from; i++)
			for (j = 0; j < term.t_ncol; j++)
			    pscreen[i]->v_attrs[j] = 0;
		    for (i = 0; i < count; i++)
			SWAP_ATTR_PTR(from+i, to+i);
		}
#undef SWAP_ATTR_PTR
#endif /* OPT_VIDEO_ATTRS */
		if (inserts) {
			from = ptarget;
			to = match;
		} else {
			from = vtarget+count;
			to = match+count;
		}
		for (i = from; i < to; i++) {
			char *txt;
			txt = PScreen(i)->v_text;
			for (j = 0; j < term.t_ncol; ++j)
				txt[j] = ' ';
			vscreen[i]->v_flag |= VFCHG;
		}
#endif /* !OPT_PSCREEN */
		return(TRUE) ;
	}
	return(FALSE) ;
}

/* move the "count" lines starting at "from" to "to" */
void
scrscroll(from, to, count)
int from, to, count;
{
	beginDisplay;
	ttrow = ttcol = -1;
	TTscroll(from,to,count);
	endofDisplay;
}

static int
texttest(vrow,prow)		/* return TRUE on text match */
int	vrow, prow ;		/* virtual, physical rows */
{
	struct	VIDEO *vpv = vscreen[vrow] ;	/* virtual screen image */
	struct	VIDEO *vpp = PScreen(prow)  ;	/* physical screen image */

	return (!memcmp(vpv->v_text, vpp->v_text, (SIZE_T)term.t_ncol)) ;
}

/* return the index of the first blank of trailing whitespace */
static int
endofline(s,n)
char 	*s;
int	n;
{
	int	i;
	for (i = n - 1; i >= 0; i--)
		if (s[i] != ' ') return(i+1) ;
	return(0) ;
}

#endif /* CAN_SCROLL */


/*	updext_past: update the extended line which the cursor is currently
		on at a column greater than the terminal width. The line
		will be scrolled right or left to let the user see where
		the cursor is		*/
static int
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
static int
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
 * row and column variables. It does try an exploit erase to end of line.
 */
#if	MEMMAP
/*	UPDATELINE specific code for the IBM-PC and other compatibles */

static void
updateline(row, colfrom, colto)

int	row;		/* row of screen to update */
int	colfrom;	/* first column on screen */
int	colto;		/* last column on screen */

{
	register struct VIDEO *vp1 = vscreen[row];	/* virtual screen image */
	register int	req = (vp1->v_flag & VFREQ) == VFREQ;

#if	OPT_COLOR
	CurFcolor(vp1) = ReqFcolor(vp1);
	CurBcolor(vp1) = ReqBcolor(vp1);
#endif
#if OPT_VIDEO_ATTRS
	scwrite(row, colfrom, colto - colfrom,
		VideoText(vp1),
		VideoAttr(vp1),
		ReqFcolor(vp1),
		ReqBcolor(vp1));
#else	/* highlighting, anyway */
	scwrite(row, colfrom, colto - colfrom,
		VideoText(vp1),
		(VIDEO_ATTR *)0,
		req ? ReqBcolor(vp1) : ReqFcolor(vp1),
		req ? ReqFcolor(vp1) : ReqBcolor(vp1));
#endif
	vp1->v_flag &= ~(VFCHG | VFCOL); /* flag this line as updated */
	if (req)
		vp1->v_flag |= VFREV;
	else
		vp1->v_flag &= ~VFREV;
}

#else	/* !MEMMAP */
#if	OPT_PSCREEN
static void
updateline(row, colfrom, colto)
    int	row;		/* row of screen to update */
    int	colfrom;	/* col to start updating from */
    int	colto;		/* col to go to */
{
    register char *vc, *pc, *evc;
    register VIDEO_ATTR *va, *pa;
    int nchanges = 0;

    if ((vscreen[row]->v_flag & VFCHG) == 0)
	return;

    vc  = &vscreen[row]->v_text[colfrom];
    evc = &vscreen[row]->v_text[colto];
    pc  = &pscreen[row]->v_text[colfrom];
    va  = &vscreen[row]->v_attrs[colfrom];
    pa  = &pscreen[row]->v_attrs[colfrom];

    while (vc < evc) {
	if (*vc != *pc || VATTRIB(*va) != VATTRIB(*pa)) {
	    *pc = *vc;
	    *pa = *va | VADIRTY;
	    nchanges++;
	}
	vc++;
	pc++;
	va++;
	pa++;
    }

    if (nchanges > 0)
	pscreen[row]->v_flag |= VFCHG;
    vscreen[row]->v_flag &= ~(VFCHG | VFCOL); /* mark virtual line updated */
}

#else  /* !OPT_PSCREEN */

/*	UPDATELINE code for all other versions		*/

#define TTattr(a) TTrev(a) /* FIXME */

static void
updateline(row, colfrom, colto)

int	row;		/* row of screen to update */
int	colfrom;	/* first column on screen */
int	colto;		/* first column on screen */

{
	struct VIDEO *vp1 = vscreen[row];	/* virtual screen image */
	struct VIDEO *vp2 = PSCREEN[row];	/* physical screen image */
	register int xl = colfrom;
	register int xr = colto;
	register int xx;

	register char *cp1 = VideoText(vp1);
	register char *cp2 = VideoText(vp2);
	register int nbflag;	/* non-blanks to the right flag? */

#if OPT_VIDEO_ATTRS
	register VIDEO_ATTR *ap1 = VideoAttr(vp1);
	register VIDEO_ATTR *ap2 = VideoAttr(vp2);
	int Blank = 0;		/* FIXME: Color? */
#else
	int rev;		/* reverse video flag */
	int req;		/* reverse video request flag */
#endif

#if !OPT_VIDEO_ATTRS
#if	OPT_COLOR
	TTforg(ReqFcolor(vp1));
	TTbacg(ReqBcolor(vp1));
#endif

#if	OPT_REVSTA || OPT_COLOR
	/* if we need to change the reverse video status of the
	   current line, we need to re-write the entire line     */
	rev = (vp1->v_flag & VFREV) == VFREV;
	req = (vp1->v_flag & VFREQ) == VFREQ;
	if ((rev != req)
#if	OPT_COLOR
	    || (CurFcolor(vp1) != ReqFcolor(vp1))
	    || (CurBcolor(vp1) != ReqBcolor(vp1))
#endif
#if	DISP_HP150
	/* the HP150 has some reverse video problems */
	    || req || rev
#endif
			) {
		movecursor(row, colfrom);	/* Go to start of line. */
		/* set rev video if needed */
		if (req)
			TTrev(req);

		/* scan through the line and dump it to the screen and
		   the virtual screen array				*/
#if DISP_X11
		x_putline(row, cp1 + xl, colto - xl);
#endif
		for (; xl < colto; xl++) {
#if !DISP_X11
			TTputc(cp1[xl]);
#endif
			++ttcol;
			cp2[xl] = cp1[xl];
		}
		/* turn rev video off */
		if (req)
			TTrev(FALSE);

		/* update the needed flags */
		vp1->v_flag &= ~(VFCHG|VFCOL);
		if (req)
			vp1->v_flag |= VFREV;
		else
			vp1->v_flag &= ~VFREV;
#if	OPT_COLOR
		CurFcolor(vp1) = ReqFcolor(vp1);
		CurBcolor(vp1) = ReqBcolor(vp1);
#endif
		return;
	}
#else
	rev = FALSE;
#endif	/* OPT_REVSTA || OPT_COLOR */
#endif	/* !OPT_VIDEO_ATTRS */

	/* advance past any common chars at the left */
#if !OPT_VIDEO_ATTRS
	if (!rev)
#endif	/* !OPT_VIDEO_ATTRS */
		while (xl != colto
		    && cp1[xl] == cp2[xl]
#if OPT_VIDEO_ATTRS
		    && VATTRIB(ap1[xl]) == VATTRIB(ap2[xl])
#endif	/* OPT_VIDEO_ATTRS */
		      ) {
			++xl;
		}

/* This can still happen, even though we only call this routine on changed
 * lines. A hard update is always done when a line splits, a massive
 * change is done, or a buffer is displayed twice. This optimizes out most
 * of the excess updating. A lot of computes are used, but these tend to
 * be hard operations that do a lot of update, so I don't really care.
 */
	/* if both lines are the same, no update needs to be done */
	if (xl == colto) {
		vp1->v_flag &= ~VFCHG;	/* flag this line unchanged */
		return;
	}

	/* find out if there is a match on the right */
	nbflag = FALSE;

#if !OPT_VIDEO_ATTRS
	if (!rev)
#endif
		while (cp1[xr-1] == cp2[xr-1]
#if OPT_VIDEO_ATTRS
		    && VATTRIB(ap1[xr-1]) == VATTRIB(ap2[xr-1])
#endif
		 ) {
			--xr;
			/* Note if any nonblank in right match */
			if (cp1[xr] != ' ' 
#if OPT_VIDEO_ATTRS
			 || VATTRIB(ap1[xr] != Blank)
#endif
			  )
				nbflag = TRUE;
		}

	xx = xr;

	/* Erase to EOL ? */
	if (nbflag == FALSE
	 && eolexist == TRUE 
#if	OPT_REVSTA && !OPT_VIDEO_ATTRS
	 && (req != TRUE)
#endif
	   ) {
		while ((xx != xl)
		    && cp1[xx-1] == ' '
#if OPT_VIDEO_ATTRS
		    && VATTRIB(ap1[xx-1]) == Blank
#endif
		)
			xx--;

		if ((xr - xx) <= 3)		/* Use only if erase is */
			xx = xr;		/* fewer characters. */
	}

	movecursor(row, xl - colfrom);		/* Go to start of line. */
#if OPT_VIDEO_ATTRS
	while (xl < xx) {
		register int j = xl;
		VIDEO_ATTR attr = VATTRIB(ap1[j]);
		while ((j < xx) && (attr == VATTRIB(ap1[j])))
			j++;
		TTattr(attr);
#if DISP_X11
		x_putline(row, cp1 + xl, j - xl);
#endif
		for (; xl < j; xl++) {
#if !DISP_X11
			TTputc(cp1[xl]);
#endif
			++ttcol;
			cp2[xl] = cp1[xl];
			ap2[xl] = ap1[xl];
		}
	}
	TTattr(0);

	if (xx != xr) {				/* Erase. */
		TTeeol();
		for (; xl < xr; xl++) {
			if (cp2[xl] != cp1[xl]
			 || VATTRIB(ap2[xl]) != VATTRIB(ap1[xl]))
				ap2[xl] = ap1[xl];
			cp2[xl] = cp1[xl];
		}
	}
#else /* OPT_VIDEO_ATTRS */
#if	OPT_REVSTA
	TTrev(rev);
#endif

#if DISP_X11
	x_putline(row, cp1 + xl, xx - xl + 1);
#endif
	for (; xl < xr; xl++) {		/* Ordinary. */
#if !DISP_X11
		TTputc(cp1[xl]);
#endif
		++ttcol;
		cp2[xl] = cp1[xl];
	}

	if (xx != xr) {		/* Erase. */
		TTeeol();
		for (; xl < xr; xl++) {
			cp2[xl] = cp1[xl];
		}
	}
#if	OPT_REVSTA
	TTrev(FALSE);
#endif
#endif /* OPT_VIDEO_ATTRS */
	vp1->v_flag &= ~(VFCHG|VFCOL);	/* flag this line as updated */
	return;
}
#endif  /* OPT_PSCREEN(updateline) */
#endif	/* MEMMAP(updateline) */

/*
 * Redisplay the mode line for the window pointed to by the "wp". 
 * modeline() is the only routine that has any idea of how the modeline is
 * formatted.  You can change the modeline format by hacking at this
 * routine.  Called by "update" any time there is a dirty window.
 */
#if OPT_MLFORMAT
static void
mlfs_prefix(fsp, msp, lchar)
    char **fsp;
    char **msp;
    int lchar;
{
    register char *fs = *fsp;
    register char *ms = *msp;
    if (*fs == ':') {
	fs++;
	while (*fs && *fs != ':') {
	    if (*fs != '%')
		*ms++ = *fs++;
	    else {
		fs++;
		switch(*fs++) {
		    case EOS :
			fs--;
			break;
		    case '%' :
			*ms++ = '%';
			break;
		    case ':' :
			*ms++ = ':';
			break;
		    case '-' :
			*ms++ = lchar;
			break;
		    default :
			*ms++ = '%';
			*ms++ = *(fs-1);
			break;
		}
	    }
	}
    }
    *fsp = fs;
    *msp = ms;
}

static void
mlfs_suffix(fsp, msp, lchar)
    char **fsp;
    char **msp;
    int lchar;
{
    mlfs_prefix(fsp, msp, lchar);
    if (**fsp == ':')
	(*fsp)++;
}

static void
mlfs_skipfix(fsp)
    char **fsp;
{
    register char *fs = *fsp;
    if (*fs == ':') {
	for (fs++;*fs && *fs != ':'; fs++);
	if (*fs == ':')
	    fs++;
	for (;*fs && *fs != ':'; fs++);
	if (*fs == ':')
	    fs++;
    }
    *fsp = fs;
}
#endif /* OPT_MLFORMAT */

#define PutMode(mode,name) \
	if (b_val(bp, mode)) { \
		if (ms != 0) { \
			ms = lsprintf(ms, "%c%s", \
				mcnt ? ' ' : '[', \
				name); \
		} \
		mcnt++; \
	}

static int
modeline_modes(bp, msptr)
BUFFER *bp;
char	**msptr;
{
	register char *ms = msptr ? *msptr : 0;
	register int mcnt = 0;

#if SYS_VMS || HAVE_LOSING_SWITCH_WITH_STRUCTURE_OFFSET
	PutMode(MDCMOD,		"cmode")
#if OPT_ENCRYPT
	PutMode(MDCRYPT,	"crypt")
#endif
	PutMode(MDDOS,		"dos-style")
	PutMode(MDVIEW,		"view-only")
#if OPT_LCKFILES
	PutMode(MDLOCKED,	"locked by")
#endif
#else
	static struct {
		int   mode;
		char *name;
	} table[] = {
		 {MDCMOD,    "cmode"}
#if OPT_ENCRYPT
		,{MDCRYPT,   "crypt"}
#endif
		,{MDDOS,     "dos-style"}
		,{MDVIEW,    "view-only"}
#if OPT_LCKFILES
		,{MDLOCKED,  "locked by"}  /* keep this last */
#endif
	};
	register int j;

	for (j = mcnt = 0; j < TABLESIZE(table); j++) {
		PutMode(table[j].mode, table[j].name)
	}
#endif
#if OPT_LCKFILES
	if (ms != 0 && b_val(bp, MDLOCKED))
		ms = lsprintf(ms, " %s", b_val_ptr(bp,VAL_LOCKER));
#endif
	if (mcnt && ms)
		*ms++ = ']';
	if (b_is_changed(bp)) {
		if (ms != 0)
			ms = lsprintf(ms, "%s[modified]", mcnt ? " " : "");
		mcnt++;
	}
	if (ms != 0)
		*msptr = ms;
	return (mcnt != 0);
}

static int
modeline_show(wp, lchar)
WINDOW *wp;
int lchar;
{
	register int ic = lchar;
	register BUFFER *bp = wp->w_bufp;

	if (b_val(bp, MDSHOWMODE)) {
#ifdef insertmode	/* insert mode is a trait for each window */
		if (wp->w_traits.insmode == INSERT)
			ic = 'I';
		else if (wp->w_traits.insmode == REPLACECHAR)
			ic = 'R';
		else if (wp->w_traits.insmode == OVERWRITE)
			ic = 'O';
#else 			/* insertmode is a variable global to all windows */
		if (wp == curwp) {
			if (insertmode == INSERT)
				ic = 'I';
			else if (insertmode == REPLACECHAR)
				ic = 'R';
			else if (insertmode == OVERWRITE)
				ic = 'O';
		}
#endif /* !defined(insertmode) */
#if DISP_BORLAND
		set_cursor (ic != lchar);
#endif
	}
	return ic;
}

static char *
rough_position(wp)
WINDOW *wp;
{
	LINE *lp = l_ref(wp->w_line.l);
	int rows = wp->w_ntrows;
	char *msg = 0;

	while (rows-- > 0) {
		lp = lforw(lp);
		if (lp == l_ref(win_head(wp))) {
			msg = "bot";
			break;
		}
	}
	if (lBack(wp->w_line.l) == l_ref(win_head(wp))) {
		if (msg) {
			if (same_ptr(wp->w_line.l, win_head(wp)))
				msg = "emp";
			else
				msg = "all";
		} else {
			msg = "top";
		}
	}
	if (!msg)
		msg = "mid";
	return msg;
}

static void
modeline(wp)
WINDOW *wp;
{
#if OPT_MLFORMAT
    char *fs = global_g_val_ptr(GVAL_MLFORMAT);
    int fc;
#endif
    char temp[NFILEN];
    char left_ms[NFILEN*2];
    char right_ms[NFILEN*2];
    char *ms;
    register int n;
    int lchar;
    int col;
    int right_len;
    int need_eighty_column_indicator = FALSE;
    register BUFFER *bp;

    left_ms[0] = right_ms[0] = EOS;
    ms = left_ms;

    n = mode_row(wp);      	/* Location. */
#if OPT_VIDEO_ATTRS
    vscreen[n]->v_flag |= VFCHG;
    for (col=0; col < term.t_ncol; col++)
	vscreen[n]->v_attrs[col] = VAREV;
#else
    vscreen[n]->v_flag |= VFCHG | VFREQ | VFCOL;/* Redraw next time. */
#endif
#if	OPT_COLOR
    ReqFcolor(vscreen[n]) = w_val(wp,WVAL_FCOLOR);
    ReqBcolor(vscreen[n]) = w_val(wp,WVAL_BCOLOR);
#endif
    bp = wp->w_bufp;
    vtmove(n, 0);                       	/* Seek to right line. */
    if (wp == curwp) {				/* mark the current buffer */
	lchar = '=';
    } else {
#if	OPT_REVSTA
	if (revexist)
	    lchar = ' ';
	else
#endif
	    lchar = '-';
    }

#if OPT_MLFORMAT
    while (*fs) {
	if (*fs != '%')
	    *ms++ = *fs++;
	else {
	    fs++;
	    switch ((fc = *fs++)) {
		case EOS :			/* Null character ! */
		    fs--;
		    break;
		case '%' :
		case ':' :
		    *ms++ = *(fs-1);
		    break;
		case '|' :
		    need_eighty_column_indicator = TRUE;
		    break;
		case '-' :
		    *ms++ = lchar;
		    break;
		case '=' :
		    *ms = EOS;
		    ms = right_ms;
		    break;
		case 'i' :			/* insert mode indicator */
		    *ms++ = modeline_show(wp, lchar);
		    break;
		case 'b' :
		    ms = lsprintf(ms, "%s", get_bname(bp));
		    break;
		case 'm' :
		    if (modeline_modes(bp, (char **)0)) {
			mlfs_prefix(&fs, &ms, lchar);
			(void)modeline_modes(bp, &ms);
			mlfs_suffix(&fs, &ms, lchar);
		    }
		    else
			mlfs_skipfix(&fs);
		    break;
		case 'f' :
		case 'F' : {
		    char *p;
		    if (bp->b_fname != 0
		     && (p = shorten_path(strcpy(temp,bp->b_fname), FALSE)) != 0
		     && !eql_bname(bp,p) 
		     && (fc == 'f' ? !is_internalname(p) 
			           : is_internalname(p))) {
			mlfs_prefix(&fs, &ms, lchar);
			for (; *p == ' '; p++);
			ms = lsprintf(ms, "%s", p);
			mlfs_suffix(&fs, &ms, lchar);
		    }
		    else
			mlfs_skipfix(&fs);
		    break;
		}
#ifdef WMDRULER
		case 'l' :		/* line number */
		case 'c' :		/* column number */
		case 'p' :		/* percentage */
		case 'L' :		/* number of lines in buffer */

		    if (w_val(wp,WMDRULER) && !is_empty_buf(wp->w_bufp)) {
			int val = 0;
			switch (fc) {
			    case 'l' : val = wp->w_ruler_line; break;
			    case 'L' : val = line_count(wp->w_bufp); break;
			    case 'c' : val = wp->w_ruler_col; break;
			    case 'p' : val = wp->w_ruler_line*100 
			                     / line_count(wp->w_bufp); break;
			}
			mlfs_prefix(&fs, &ms, lchar);
			ms = lsprintf(ms, "%d", val);
			mlfs_suffix(&fs, &ms, lchar);
		    }
		    else
			mlfs_skipfix(&fs);
		    break;

#endif
		case 'S' :
		    if (
#ifdef WMDRULER
			!w_val(wp, WMDRULER) ||
#endif
		        is_empty_buf(wp->w_bufp)) {
			mlfs_prefix(&fs, &ms, lchar);
			ms = lsprintf(ms, " %s ", rough_position(wp));
			mlfs_suffix(&fs, &ms, lchar);
		    }
		    else
			mlfs_skipfix(&fs);
		    break;
		default :
		    *ms++ = '%';
		    *ms++ = *(fs-1);
		    break;
	    }
	}
    }
#else	/* hard-coded format */
    ms = lsprintf(ms, "%c%c%c %s ",
    	lchar, modeline_show(wp, lchar), lchar, get_bname(bp));
    if (modeline_modes(bp, &ms))
    	*ms++ = ' ';
    if (bp->b_fname != 0
    && (shorten_path(strcpy(temp,bp->b_fname), FALSE))
    && !eql_bname(bp,temp)) {
	if (is_internalname(temp)) {
	    for (n = term.t_ncol - (13 + strlen(temp) + (int)(ms - left_ms));
			n > 0; n--)
		*ms++ = lchar;
	} else {
	    ms = lsprintf(ms, "is");
	}
	ms = lsprintf(ms, " %s ", temp);
    }
#ifdef WMDRULER
    if (w_val(wp, WMDRULER))
	(void)lsprintf(right_ms, " (%d,%d) %3p",
		wp->w_ruler_line, wp->w_ruler_col, lchar);
    else
#endif
     (void) lsprintf(right_ms, " %s %3p", rough_position(wp), lchar);
#endif /* OPT_MLFORMAT */

    *ms++ = EOS;
    right_len = strlen(right_ms);
    vtputsn(left_ms, term.t_ncol);
    for (n = term.t_ncol - strlen(left_ms) - right_len; n > 0; n--)
	vtputc(lchar);
    vtcol = term.t_ncol - right_len;
    if (vtcol < 0) {
	n = -vtcol;
	vtcol = 0;
    }
    else
	n = 0;
    vtputsn(right_ms+n, term.t_ncol - vtcol);
    if (need_eighty_column_indicator) {		/* mark column 80 */
	int left = -nu_width(wp);
#ifdef WMDLINEWRAP
	if (!w_val(wp,WMDLINEWRAP))
#endif
	 left += w_val(wp,WVAL_SIDEWAYS);
	n = term.t_ncol + left;
	col = 80 - left;

	if ((n > 80) && (col >= 0) && (vtgetc(col) == lchar)) {
	    vtcol = col;
	    vtputc('|');
	}
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
 * Recompute the given buffer. Save/restore its modes and position information
 * so that a redisplay will show as little change as possible.
 */
#if	OPT_UPBUFF
typedef	struct	{
	WINDOW	*wp;
	struct VAL w_vals[MAX_W_VALUES];
	int	top;
	int	line;
	int	col;
	} SAVEWIN;

static	SAVEWIN	*recomp_tbl;
static	ALLOC_T	recomp_len;

static void
recompute_buffer(bp)
BUFFER	*bp;
{
	register WINDOW *wp;
	register SAVEWIN *tbl;

	struct VAL b_vals[MAX_B_VALUES];
	int	num = 0;
	BUFFER *savebp = curbp;
	WINDOW *savewp = curwp;
	int	mygoal = curgoal;

	if (!b_val(bp,MDUPBUFF)) {
		b_clr_flags(bp,BFUPBUFF);
		return;
	}
	if (recomp_len < bp->b_nwnd) {
		recomp_len = bp->b_nwnd + 1;
		recomp_tbl = (recomp_tbl != 0)
			? typereallocn(SAVEWIN,recomp_tbl,recomp_len)
			: typeallocn(SAVEWIN,recomp_len);
		if (recomp_tbl == 0) {
			recomp_len = 0;
			return;
		}
	}
	tbl = recomp_tbl;

	/* remember where we are, to reposition */
	/* ...in case line is deleted from buffer-list */
	relisting_b_vals = 0;
	relisting_w_vals = 0;
	if (curbp == bp) {
		relisting_b_vals = b_vals;
 	} else {
		curbp = bp;
		curwp = bp2any_wp(bp);
	}
	for_each_window(wp) {
		if (wp->w_bufp == bp) {
			if (wp == savewp)
				relisting_w_vals = tbl[num].w_vals;
			curwp = wp;	/* to make 'getccol()' work */
			curbp = curwp->w_bufp;
			tbl[num].wp   = wp;
			tbl[num].top  = line_no(bp, wp->w_line.l);
			tbl[num].line = line_no(bp, wp->w_dot.l);
			tbl[num].col  = getccol(FALSE);
			save_vals(NUM_W_VALUES, global_w_values.wv,
				tbl[num].w_vals, wp->w_values.wv);
			if (++num >= recomp_len)
				break;
		}
	}
	curwp = savewp;
	curbp = savebp;

	save_vals(NUM_B_VALUES, global_b_values.bv, b_vals, bp->b_values.bv);
	(bp->b_upbuff)(bp);
	copy_mvals(NUM_B_VALUES, bp->b_values.bv, b_vals);

	/* reposition and restore */
	while (num-- > 0) {
		curwp = wp = tbl[num].wp;
		curbp = curwp->w_bufp;
		(void)gotoline(TRUE, tbl[num].top);
		wp->w_line.l = wp->w_dot.l;
		wp->w_line.o = 0;
		if (tbl[num].line != tbl[num].top)
			(void)gotoline(TRUE, tbl[num].line);
		(void)gocol(tbl[num].col);
        	wp->w_flag |= WFMOVE;
		copy_mvals(NUM_W_VALUES, wp->w_values.wv, tbl[num].w_vals);
	}
	curwp = savewp;
	curbp = savebp;
	curgoal = mygoal;
	b_clr_flags(bp,BFUPBUFF);
	relisting_b_vals = 0;
	relisting_w_vals = 0;
}
#endif	/* OPT_UPBUFF */

/*
 * Send a command to the terminal to move the hardware cursor to row "row"
 * and column "col". The row and column arguments are origin 0. Optimize out
 * random calls. Update "ttrow" and "ttcol".
 */
void
movecursor(row, col)
int row,col;
{
	beginDisplay;
	if (row!=ttrow || col!=ttcol)
        {
	        ttrow = row;
	        ttcol = col;
	        TTmove(row, col);
        }
	endofDisplay;
}

void
bottomleft()
{
	movecursor(term.t_nrow-1, 0);
}

/* Erase the message-line from the current position */
static void
erase_remaining_msg (column)
int	column;
{
	beginDisplay;
#if !OPT_RAMSIZE
	if (eolexist == TRUE)
		TTeeol();
	else
#endif
	{
		register int i;
#if OPT_RAMSIZE
		int	limit = global_g_val(GMDRAMSIZE)
				? LastMsgCol
				: term.t_ncol - 1;
#else
		int	limit = term.t_ncol - 1;
#endif
		for (i = ttcol; i < limit; i++)
			TTputc(' ');
		ttrow = term.t_nrow-2;	/* force the move! */
		movecursor(term.t_nrow-1, column);
	}
	TTflush();
	endofDisplay;
}


/*
 * Erase the message line. This is a special routine because the message line
 * is not considered to be part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed via a call to the flusher.
 */
void
mlerase()
{
	beginDisplay;
	if (mpresf != 0) {
		bottomleft();
		if (discmd != FALSE) {
#if	OPT_COLOR
			TTforg(gfcolor);
			TTbacg(gbcolor);
#endif
			erase_remaining_msg(0);
			mpresf = 0;
		}
	}
	endofDisplay;
}

char *mlsavep;
char mlsave[NSTRING];

void
mlsavec(c)
int c;
{
	if (mlsavep - mlsave < NSTRING-1) {
		*mlsavep++ = c;
		*mlsavep = EOS;
	}
}

/*
 * Write a message into the message line only if appropriate.
 */
/* VARARGS1 */
void
#if	ANSI_VARARGS
mlwrite( char *fmt, ...)
#else
mlwrite(va_alist)
va_dcl
#endif
{
	va_list ap;
	/* if we are not currently echoing on the command line, abort this */
	if (global_b_val(MDTERSE) || kbd_replaying(FALSE) || discmd == FALSE) {
		bottomleft();
		return;
	}
#if	ANSI_VARARGS
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
#if	ANSI_VARARGS
mlforce(char *fmt, ...)
#else
mlforce(va_alist)
va_dcl
#endif
{
	va_list ap;
#if	ANSI_VARARGS
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
#if	ANSI_VARARGS
mlprompt( char *fmt, ...)
#else
mlprompt(va_alist)
va_dcl
#endif
{
	va_list ap;
	int osgarbf = sgarbf;
	if (discmd == FALSE) {
		bottomleft();
		return;
	}
	sgarbf = FALSE;
#if	ANSI_VARARGS
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
#if	ANSI_VARARGS
dbgwrite( char *fmt, ...)
#else
dbgwrite(va_alist)
va_dcl
#endif
{
	va_list ap;	/* ptr to current data field */
#if	ANSI_VARARGS
	va_start(ap,fmt);
	mlmsg(fmt,&ap);
#else
	va_start(ap);
	mlmsg(&ap);
#endif
	va_end(ap);
	beginDisplay;
	while (TTgetc() != '\007')
		;
	endofDisplay;
}

/*
 * Do the real message-line work.  Keep track of the physical cursor
 * position. A small class of printf like format items is handled.
 * Set the "message line" flag TRUE.
 */
static void
#if	ANSI_VARARGS
mlmsg( char *fmt, va_list *app)
#else
mlmsg(app)
va_list *app;	/* ptr to current data field */
#endif
{
	static	int	recur;

	if (recur++) {
		;
	} else if (sgarbf) {
		/* then we'll lose the message on the next update(), so save it now */
		mlsavep = mlsave;
#if	OPT_POPUP_MSGS
		if (global_g_val(GMDPOPUP_MSGS) || (curwp == 0)) {
			popup_msgs();
			msg_putc('\n');
			dfoutfn = msg_putc;
		} else
#endif
		  dfoutfn = mlsavec;
#if	ANSI_VARARGS
		dofmt(fmt,app);
#else
		dofmt(app);
#endif
	} else {

#if	OPT_COLOR
		/* set up the proper colors for the command line */
		TTforg(gfcolor);
		TTbacg(gbcolor);
#endif

		beginDisplay;
		bottomleft();

		kbd_expand = -1;
#if	OPT_POPUP_MSGS
		if (global_g_val(GMDPOPUP_MSGS)) {
			popup_msgs();
			if (mlsave[0] == EOS) {
				msg_putc('\n');
				dfoutfn = msg_putc;
			} else {
				dfoutfn = kbd_putc;
			}
		} else
#endif
		  dfoutfn = kbd_putc;

#if	ANSI_VARARGS
		dofmt(fmt,app);
#else
		dofmt(app);
#endif
		kbd_expand = 0;

		/* if we can, erase to the end of screen */
		erase_remaining_msg(ttcol);
		mpresf = ttcol;
		mlsave[0] = EOS;
		endofDisplay;
	}
	recur--;
}

/*
 * Do the equivalent of 'perror()' on the message line
 */
void
mlerror(s)
char	*s;
{
#if SYS_UNIX || SYS_VMS || CC_NEWDOSCC

	if (errno > 0 && errno < sys_nerr)
		mlwarn("[Error %s: %s]", s, sys_errlist[errno]);
	else
		mlwarn("[Error %s: unknown system error %d]", s, errno);

#else
	mlwarn("[Error %s]", s);
#endif
}

/*
 * Emit a warning message (with alarm)
 */
/* VARARGS1 */
void
#if	ANSI_VARARGS
mlwarn(char *fmt, ...)
#else
mlwarn(va_alist)
va_dcl
#endif
{
	va_list ap;
#if	ANSI_VARARGS
	va_start(ap,fmt);
	mlmsg(fmt,&ap);
#else
	va_start(ap);
	mlmsg(&ap);
#endif
	va_end(ap);
	kbd_alarm();
}

/*
 * Local sprintf -- similar to standard libc, but
 *  returns pointer to null character at end of buffer, so it can
 *  be called repeatedly, as in:
 *	cp = lsprintf(cp, fmt, args...);
 *
 */

char *lsp;

static void
lspputc(c)
int c;
{
	*lsp++ = c;
}

/* VARARGS1 */
char *
#if	ANSI_VARARGS
lsprintf( char *buf, char *fmt, ...)
#else
lsprintf(va_alist)
va_dcl
#endif
{

	va_list ap;
#if	ANSI_VARARGS
	va_start(ap,fmt);
#else
	char *buf;
	va_start(ap);
	buf = va_arg(ap, char *);
#endif

	lsp = buf;
	dfoutfn = lspputc;

#if	ANSI_VARARGS
	dofmt(fmt,&ap);
#else
	dofmt(&ap);
#endif
	va_end(ap);

	*lsp = EOS;
	return lsp;
}


/*
 * Buffer printf -- like regular printf, but puts characters
 *	into the BUFFER.
 */
void
bputc(c)
int c;
{
	if (c == '\n')
		(void)lnewline();
	else
		(void)linsert(1,c);
}

/* printf into curbp, at DOT */
/* VARARGS */
void
#if	ANSI_VARARGS
bprintf( char *fmt, ...)
#else
bprintf(va_alist)
va_dcl
#endif
{
	va_list ap;

	dfoutfn = bputc;

#if	ANSI_VARARGS
	va_start(ap,fmt);
	dofmt(fmt,&ap);
#else
	va_start(ap);
	dofmt(&ap);
#endif
	va_end(ap);

}

#if defined( SIGWINCH) && ! DISP_X11
/* ARGSUSED */
SIGT
sizesignal (ACTUAL_SIG_ARGS)
ACTUAL_SIG_DECL
{
	int w, h;
	int old_errno = errno;

	getscreensize (&w, &h);

	if ((h > 1 && h != term.t_nrow) || (w > 1 && w != term.t_ncol))
		newscreensize(h, w);

	(void)signal(SIGWINCH, sizesignal);
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
	if ((h > term.t_mrow) || (w > term.t_mcol)) {
		int or, oc;
		or = term.t_mrow;
		oc = term.t_mcol;
		term.t_mrow = h;
		term.t_mcol = w;
		if (!vtinit()) { /* allocation failure */
			term.t_mrow = or;
			term.t_mcol = oc;
			return;
		}
	}
	if (!newlength(TRUE,h) || !newwidth(TRUE,w))
		return;

	(void)update(TRUE);
}

/*
 * Displays alternate
 *	"working..." and
 *	"...working"
 * at the end of the message line if it has been at least a second since
 * displaying anything or waiting for keyboard input.  The cur_working and
 * max_working values are used in 'slowreadf()' to show the progress of reading
 * large files.
 */
#if OPT_WORKING

/*ARGSUSED*/
SIGT
imworking (ACTUAL_SIG_ARGS)
ACTUAL_SIG_DECL
{
	static	char	*msg[] = {"working", "..."};
	static	int	flip;
	static	int	skip;

	if (no_working) /* brute force, for debugging */
		return;

	/* (if GMDWORKING is _not_ set, or MDTERSE is set, we're allowed
	 * to erase, but not to write.  and if we do erase, we don't
	 * reschedule the alarm, since setting the mode will call us
	 * again to start things up)
	 */

	if (displaying) {	/* look at the semaphore first! */
		;
	} else if (ShowWorking() && !doing_kbd_read) {
		if (skip) {
			skip = FALSE;
		} else if (displayed) {
			int	save_row = ttrow;
			int	save_col = ttcol;
			int	show_col = LastMsgCol - 10;
			if (show_col < 0)
				show_col = 0;
			movecursor(term.t_nrow-1, show_col);
			if (cur_working != 0
			 && cur_working != old_working) {
				char	temp[20];
				int	len = cur_working > 999999L ? 10 : 6;

				old_working = cur_working;
				kbd_puts(right_num(temp, len, cur_working));
				if (len == 10)
					;
				else if (max_working != 0) {
					kbd_putc(' ');
					kbd_puts(right_num(temp, 2,
						(100 * cur_working) / max_working));
					kbd_putc('%');
				} else
					kbd_puts(" ...");
			} else {
				kbd_puts(msg[ flip]);
				kbd_puts(msg[!flip]);
			}
			movecursor(save_row, save_col);
			if (mpresf >= 0)
				mpresf = -(mpresf+1);
			TTflush();
#if DISP_X11
			x_working();
#endif
		}
	} else {
		if (mpresf < 0) {	/* erase leftover working-message */
			int	save_row = ttrow;
			int	save_col = ttcol;
			int	erase_at = -(mpresf+1);
			if (erase_at < save_col && reading_msg_line)
				erase_at = save_col;
			movecursor(term.t_nrow-1, erase_at);
			erase_remaining_msg(erase_at);
			movecursor(save_row, save_col);
			TTflush();
			mpresf = 0;
		}
		skip = TRUE;
		if (!ShowWorking())
			return;
	}
	(void)signal(SIGALRM,imworking);
	(void)alarm(1);
	flip = !flip;
}
#endif	/* OPT_WORKING */

#if	OPT_PSCREEN
/* Most of the code in this section is for making the message line work
 * right...it shouldn't be called to display the rest of the screen.
 */
static int psc_row;
static int psc_col;

#define SWAP_INT(x,y) \
	do { (x) = (x)+(y); (y) = (x)-(y); (x) = (x)-(y); } while (0)
#define SWAP_VT_PSC \
	do { SWAP_INT(vtcol, psc_col); SWAP_INT(vtrow, psc_row); } while (0)

void
psc_putchar(c)
    int c;
{
    if (c == '\b') {
	if (psc_col > 0)
	    psc_col--;
    }
    else {
	SWAP_VT_PSC;
	vtputc(c);
	vscreen[vtrow]->v_flag |= VFCHG;
	SWAP_VT_PSC;
    }
}

void
psc_flush()
{
    updateline(term.t_nrow-1, 0, term.t_ncol);
    TTpflush();
}

void
psc_move(row,col)
    int row,col;
{
    psc_row = row;
    psc_col = col;
}

void
psc_eeol()
{
    if (ttrow >= 0 && ttrow < term.t_nrow && ttcol >= 0) {
	VIDEO_ATTR *vp = &vscreen[ttrow]->v_attrs[ttcol];
	char *cp = &vscreen[ttrow]->v_text[ttcol];
	char *cplim = &vscreen[ttrow]->v_text[term.t_ncol];
	vscreen[ttrow]->v_flag |= VFCHG;
	while (cp < cplim) {
	    *vp++ = 0;
	    *cp++ = ' ';
	}
    }
}

void
psc_eeop()
{
    int saverow = ttrow;
    int savecol = ttcol;
    while (ttrow < term.t_nrow) {
	psc_eeol();
	ttrow++;
	ttcol = 0;
    }
    ttrow = saverow;
    ttcol = savecol;
}

/* ARGSUSED */
void
psc_rev(huh)
    int huh;
{
    /* do nothing */
}

#endif	/* OPT_PSCREEN */

/* For memory-leak testing (only!), releases all display storage. */
#if NO_LEAKS
void	vt_leaks()
{
	vtfree();
#if OPT_UPBUFF
	FreeIfNeeded(recomp_tbl);
#endif
}
#endif
