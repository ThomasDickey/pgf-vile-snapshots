/*	Dumb terminal driver, for I/O before we get into screen mode.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/dumbterm.c,v 1.2 1995/11/18 00:36:16 pgf Exp $
 *
 */

#include	"estruct.h"
#include	"edef.h"

#define MARGIN	8
#define SCRSIZ	64
#define NPAUSE	10			/* # times thru update to pause */

static int  dumb_cres     P(( char * ));
static int  dumb_getc     P(( void ));
static int  dumb_typahead P(( void ));
static void dumb_beep     P(( void ));
static void dumb_eeol     P(( void ));
static void dumb_eeop     P(( void ));
static void dumb_flush    P(( void ));
static void dumb_kclose   P(( void ));
static void dumb_kopen    P(( void ));
static void dumb_move     P(( int, int ));
static void dumb_putc     P(( int ));

#if OPT_COLOR
static void dumb_fcol     P(( int ));
static void dumb_bcol     P(( int ));
#endif

#if OPT_VIDEO_ATTRS
static void dumb_attr     P(( int ));
#else
static void dumb_rev      P(( int ));
#endif

static void flush_blanks  P(( void ));

TERM dumb_term = {
	1,
	1,
	80,
	80,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	0,		/* use this to put us into raw mode */
	0,		/* ...and this, just in case we exit */
	dumb_kopen,
	dumb_kclose,
	dumb_getc,
	dumb_putc,
	dumb_typahead,
	dumb_flush,
	dumb_move,
	dumb_eeol,
	dumb_eeop,
	dumb_beep,
#if OPT_VIDEO_ATTRS
	dumb_attr,
#else
	dumb_rev,
#endif
	dumb_cres
#if	OPT_COLOR
	, dumb_fcol
	, dumb_bcol
#endif
	, NULL		/* set dynamically at open time */
};

static	int	this_col;
static	int	last_col;

static void
flush_blanks()
{
	if (last_col > 0) {
		while (last_col++ < this_col)
			(void)putchar(' ');
		last_col = 0;
	}
	TTflush();
}

static void
dumb_kopen()
{
}

static void
dumb_kclose()
{
}

static int
dumb_getc()
{
	flush_blanks();
	return getchar();
}

static void
dumb_putc(c)
int c;
{
	if (isspace(c)) {
		if (last_col == 0)
			last_col = this_col;
	} else {
		flush_blanks();
		(void)putchar(c);
	}
	this_col++;
}

static int
dumb_typahead()
{
	return TRUE;
}

static void
dumb_flush()
{
	fflush(stdout);
}

/*ARGSUSED*/
static void
dumb_move(row, col)
int row, col;
{
	if (last_col == 0)
		last_col = this_col;
	if (col == 0) {
		putchar('\r');
		if (last_col != 0)
			putchar('\n');
	} else if (last_col > col) {
		while (last_col-- > col)
			putchar('\b');
	} else if (last_col < col) {
		while (last_col++ < col)
			putchar(' ');
	}
	last_col = 0;
	this_col = col;
}

static void
dumb_eeol()
{
}

static void
dumb_eeop()
{
}

/*ARGSUSED*/
static int
dumb_cres(res)	/* change screen resolution */
char *	res;
{
	return(FALSE);
}

#if	OPT_COLOR
/* ARGSUSED */
static void
dumb_fcol(color)
int color;
{
}

/* ARGSUSED */
static void
dumb_bcol(color)
int color;
{
}
#endif /* OPT_COLOR */

#if OPT_VIDEO_ATTRS
/* ARGSUSED */
static void
dumb_attr(attr)
int attr;
{
}

#else	/* highlighting is a minimum attribute */

/* ARGSUSED */
static void
dumb_rev(state)
int state;
{
}

#endif	/* OPT_VIDEO_ATTRS */

static void
dumb_beep()
{
	putchar(BEL);
}
