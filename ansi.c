/*
 * The routines in this file provide support for ANSI style terminals
 * over a serial line. The serial I/O services are provided by routines in
 * "termio.c". It compiles into nothing if not an ANSI device.
 *
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/ansi.c,v 1.21 1995/04/22 03:22:53 pgf Exp $
 */

#if	SYS_AMIGA
#if comments

 this is the code that used to parse function keys in kbd_key().  function
 keys are installed as maps these days.  see tcap.c or ibmpc.c for examples.
 	pgf 11/94

	/* apply SPEC prefix */
	if ((unsigned)c == 155) {
		int	d;
		c = TTgetc();

		/* first try to see if it is a cursor key */
		if ((c >= 'A' && c <= 'D') || c == 'S' || c == 'T') {
			return c | SPEC;
		}

		/* next, a 2 char sequence */
		d = TTgetc();
		if (d == '~') {
			return c | SPEC;
		}

		/* decode a 3 char sequence */
		c = d + ' ';
		/* if a shifted function key, eat the tilde */
		if (d >= '0' && d <= '9')
			d = TTgetc();
		return c | SPEC;
	}
#endif
#endif

#define termdef 1			/* don't define "term" external */

#include	"estruct.h"
#include	"edef.h"

#if	DISP_ANSI

#define SCROLL_REG 1

#if	SYS_AMIGA
#define NROW	23			/* Screen size.			*/
#define NCOL	77			/* Edit if you want to.		*/
#endif

#if	SYS_MSDOS
#define NROW    25
#define NCOL    80
#define MAXNROW	60
#define MAXNCOL	132
#undef SCROLL_REG			/* ANSI.SYS can't do scrolling */
#define SCROLL_REG 0
#endif

#ifndef NROW
#define NROW	24			/* Screen size.			*/
#define NCOL	80			/* Edit if you want to.		*/
#endif
#ifndef MAXNROW
#define MAXNROW	NROW
#define MAXNCOL	NCOL
#endif

#define NPAUSE	100			/* # times thru update to pause */
#define MARGIN	8			/* size of minimim margin and	*/
#define SCRSIZ	64			/* scroll size for extended lines */

static	void	ansimove   P((int,int));
static	void	ansieeol   P((void));
static	void	ansieeop   P((void));
static	void	ansibeep   P((void));
static	void	ansiopen   P((void));
static	void	ansirev    P((int));
static	void	ansiclose  P((void));
static	void	ansikopen  P((void));
static	void	ansikclose P((void));
static	int	ansicres   P((char *));
static	void	ansiscroll P((int,int,int));

#if	OPT_COLOR
static	void	ansifcol P((int));
static	void	ansibcol P((int));

static	int	cfcolor = -1;		/* current forground color */
static	int	cbcolor = -1;		/* current background color */

#if	SYS_AMIGA
/* apparently the AMIGA does not follow the ANSI standards as regards to
 * colors ...maybe because of the default palette settings?
 */
static	int coltran[8] = {2, 3, 5, 7, 0, 4, 6, 1};
			/* color translation table */
#endif
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM	term	= {
	MAXNROW,	/* max */
	NROW,		/* current */
	MAXNCOL,	/* max */
	NCOL,		/* current */
	MARGIN,
	SCRSIZ,
	NPAUSE,
	ansiopen,
	ansiclose,
	ansikopen,
	ansikclose,
	ttgetc,
	ttputc,
	tttypahead,
	ttflush,
	ansimove,
	ansieeol,
	ansieeop,
	ansibeep,
	ansirev,
	ansicres
#if	OPT_COLOR
	, ansifcol,
	ansibcol
#endif
	, ansiscroll
};

static	void	ansiparm P((int));
#if SCROLL_REG
static	void	ansiscrollregion P((int,int));
#endif

static void
csi P((void))
{
	ttputc(ESC);
	ttputc('[');
}

#if	OPT_COLOR
static void
ansifcol(color)		/* set the current output color */
	int	color;	/* color to set */
{
	if (color == cfcolor)
		return;
	csi();
#if	SYS_AMIGA
	ansiparm(coltran[color]+30);
#else
	ansiparm(color+30);
#endif
	ttputc('m');
	cfcolor = color;
}

static void
ansibcol(color)		/* set the current background color */
	int	color;	/* color to set */
{
	if (color == cbcolor)
		return;
	csi();
#if	SYS_AMIGA
	ansiparm(coltran[color]+40);
#else
	ansiparm(color+40);
#endif
	ttputc('m');
	cbcolor = color;
}
#endif

static void
ansimove(row, col)
int	row;
int	col;
{
	csi();
	ansiparm(row+1);
	ttputc(';');
	ansiparm(col+1);
	ttputc('H');
}

static void
ansieeol()
{
	csi();
	ttputc('K');
}

static void
ansieeop()
{
#if	OPT_COLOR
	ansifcol(gfcolor);
	ansibcol(gbcolor);
#endif
	csi();
	ttputc('2');
	ttputc('J');
}

#if BROKEN_REVERSE_VIDEO
/* there was something wrong with this "fix".  the "else" of
		the ifdef just uses "ESC [ 7 m" to set reverse
		video, and it works under DOS for me....  but then, i
		use an "after-market" ansi driver -- nnansi593.zip, from
		oak.oakland.edu, or any simtel mirror. */
static void
ansirev(state)		/* change reverse video state */
int state;	/* TRUE = reverse, FALSE = normal */
{
#if	OPT_COLOR
	int ftmp, btmp;	/* temporaries for colors */
#else
	static int revstate = -1;
	if (state == revstate)
		return;
	revstate = state;
#endif

	csi();
#if OPT_COLOR && SYS_MSDOS
	ttputc('1');	/* bold-on */
#else
	if (state) ttputc('7');	/* reverse-video on */
#endif
	ttputc('m');

#if	OPT_COLOR
#if	SYS_MSDOS
	/*
	 * Setting reverse-video with ANSI.SYS seems to reset the colors to
	 * monochrome.  Using the colors directly to simulate reverse video
	 * works better. Bold-face makes the foreground colors "look" right.
	 */
	ftmp = cfcolor;
	btmp = cbcolor;
	cfcolor = -1;
	cbcolor = -1;
	ansifcol(state ? btmp : ftmp);
	ansibcol(state ? ftmp : btmp);
#else	/* normal ANSI-reverse */
	if (state == FALSE) {
		ftmp = cfcolor;
		btmp = cbcolor;
		cfcolor = -1;
		cbcolor = -1;
		ansifcol(ftmp);
		ansibcol(btmp);
	}
#endif	/* MSDOS vs ANSI-reverse */
#endif	/* OPT_COLOR */
}

#else

void
ansirev(state)		/* change reverse video state */
int state;	/* TRUE = reverse, FALSE = normal */
{
	static int revstate = -1;
	if (state == revstate)
		return;
	revstate = state;

	csi();
	if (state) ttputc('7');	/* reverse-video on */
	ttputc('m');

}

#endif

static int
ansicres(flag)	/* change screen resolution */
char	*flag;
{
	return(TRUE);
}

void
spal(dummy)		/* change palette settings */
char	*dummy;
{
	/* none for now */
}

static void
ansibeep()
{
	ttputc(BEL);
	ttflush();
}


/* if your ansi terminal can scroll regions, like the vt100, then define
	SCROLL_REG.  If not, you can use delete/insert line code, which
	is prettier but slower if you do it a line at a time instead of
	all at once.
*/

/* move howmany lines starting at from to to */
static void
ansiscroll(from,to,n)
int	from;
int	to;
int	n;
{
	int i;
	if (to == from) return;
#if SCROLL_REG
	if (to < from) {
		ansiscrollregion(to, from + n - 1);
		ansimove(from + n - 1,0);
		for (i = from - to; i > 0; i--)
			ttputc('\n');
	} else { /* from < to */
		ansiscrollregion(from, to + n - 1);
		ansimove(from,0);
		for (i = to - from; i > 0; i--) {
			ttputc(ESC);
			ttputc('M');
		}
	}
	ansiscrollregion(0, term.t_mrow);

#else /* use insert and delete line */
#if OPT_PRETTIER_SCROLL
	if (absol(from-to) > 1) {
		ansiscroll(from, (from<to) ? to-1:to+1, n);
		if (from < to)
			from = to-1;
		else
			from = to+1;
	}
#endif
	if (to < from) {
		ansimove(to,0);
		csi();
		ansiparm(from - to);
		ttputc('M'); /* delete */
		ansimove(to+n,0);
		csi();
		ansiparm(from - to);
		ttputc('L'); /* insert */
	} else {
		ansimove(from+n,0);
		csi();
		ansiparm(to - from);
		ttputc('M'); /* delete */
		ansimove(from,0);
		csi();
		ansiparm(to - from);
		ttputc('L'); /* insert */
	}
#endif
}

#if SCROLL_REG
static void
ansiscrollregion(top,bot)
int	top;
int	bot;
{
	csi();
	ansiparm(top + 1);
	ttputc(';');
	if (bot != term.t_nrow-1) ansiparm(bot + 1);
	ttputc('r');
}
#endif


void
ansiparm(n)
register int	n;
{
	register int q,r;

#if optimize_works /* i don't think it does, although it should, to be ANSI */
	if (n == 1) return;
#endif

	q = n/10;
	if (q != 0) {
		r = q/10;
		if (r != 0) {
			ttputc((r%10)+'0');
		}
		ttputc((q%10) + '0');
	}
	ttputc((n%10) + '0');
}

static void
ansiopen()
{
	strcpy(sres, "NORMAL");
	revexist = TRUE;
	ttopen();
}

static void
ansiclose()
{
#if	OPT_COLOR
	ansifcol(7);
	ansibcol(0);
#endif
	/*ttclose();*/
}

static void
ansikopen()	/* open the keyboard (a noop here) */
{
}

static void
ansikclose()	/* close the keyboard (a noop here) */
{
}

#endif	/* DISP_ANSI */
