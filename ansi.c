/*
 * The routines in this file provide support for ANSI style terminals
 * over a serial line. The serial I/O services are provided by routines in
 * "termio.c". It compiles into nothing if not an ANSI device.
 *
 * $Log: ansi.c,v $
 * Revision 1.8  1992/05/16 12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.7  1992/04/10  18:47:25  pgf
 * change abs to absol to get rid of name conflicts
 *
 * Revision 1.6  1991/11/16  18:28:25  pgf
 * removed an old ifdef
 *
 * Revision 1.5  1991/09/10  01:19:35  pgf
 * re-tabbed, and moved ESC and BEL to estruct.h
 *
 * Revision 1.4  1991/08/07  12:34:39  pgf
 * added RCS log messages
 *
 * revision 1.3
 * date: 1991/06/19 01:32:21;
 * change name of howmany 'cuz of HP/UX conflict
 * sheesh
 * 
 * revision 1.2
 * date: 1991/05/31 10:26:19;
 * moved PRETTIER_SCROLL #define to estruct.h
 * 
 * revision 1.1
 * date: 1990/09/21 10:24:38;
 * initial vile RCS revision
 */

#define termdef 1			/* don't define "term" external */

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"

#if	ANSI

#if	AMIGA
#define NROW	23			/* Screen size. 		*/
#define NCOL	77			/* Edit if you want to. 	*/
#else
#define NROW	24			/* Screen size. 		*/
#define NCOL	80			/* Edit if you want to. 	*/
#endif
#define NPAUSE	100			/* # times thru update to pause */
#define MARGIN	8			/* size of minimim margin and	*/
#define SCRSIZ	64			/* scroll size for extended lines */

extern	int	ttopen();		/* Forward references.		*/
extern	int	ttgetc();
extern	int	ttputc();
extern	int	ttflush();
extern	int	ttclose();
extern	int	ansimove();
extern	int	ansieeol();
extern	int	ansieeop();
extern	int	ansibeep();
extern	int	ansiopen();
extern	int	ansirev();
extern	int	ansiclose();
extern	int	ansikopen();
extern	int	ansikclose();
extern	int	ansicres();
#if SCROLLCODE
extern	int	ansiscroll();
#endif

#if	COLOR
extern	int	ansifcol();
extern	int	ansibcol();

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */

#if	AMIGA
/* apperently the AMIGA does not follow the ANSI standards as
   regards to colors....maybe because of the default pallette
   settings?
*/

int coltran[8] = {2, 3, 5, 7, 0, 4, 6, 1};	/* color translation table */
#endif
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM	term	= {
	NROW-1,
	NROW-1,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	ansiopen,
	ansiclose,
	ansikopen,
	ansikclose,
	ttgetc,
	ttputc,
	ttflush,
	ansimove,
	ansieeol,
	ansieeop,
	ansibeep,
	ansirev,
	ansicres
#if	COLOR
	, ansifcol,
	ansibcol
#endif
#if SCROLLCODE
	, ansiscroll
#endif
};

csi()
{
	ttputc(ESC);
	ttputc('[');
}

#if	COLOR
ansifcol(color) 	/* set the current output color */

int color;	/* color to set */

{
	if (color == cfcolor)
		return;
	csi();
#if	AMIGA
	ansiparm(coltran[color]+30);
#else
	ansiparm(color+30);
#endif
	ttputc('m');
	cfcolor = color;
}

ansibcol(color) 	/* set the current background color */

int color;	/* color to set */

{
	if (color == cbcolor)
		return;
	csi();
#if	AMIGA
	ansiparm(coltran[color]+40);
#else
	ansiparm(color+40);
#endif
	ttputc('m');
	cbcolor = color;
}
#endif

ansimove(row, col)
{
	csi();
	if (row) ansiparm(row+1);
	ttputc(';');
	if (col) ansiparm(col+1);
	ttputc('H');
}

ansieeol()
{
	csi();
	ttputc('K');
}

ansieeop()
{
#if	COLOR
	ansifcol(gfcolor);
	ansibcol(gbcolor);
#endif
	csi();
	ttputc('J');
}


ansirev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
#if	COLOR
	int ftmp, btmp; 	/* temporaries for colors */
#else
	static int revstate = -1;
	if (state == revstate)
		return;
	revstate = state;
#endif

	csi();
	if (state) ttputc('7');
	ttputc('m');
#if	COLOR
	if (state == FALSE) {
		ftmp = cfcolor;
		btmp = cbcolor;
		cfcolor = -1;
		cbcolor = -1;
		ansifcol(ftmp);
		ansibcol(btmp);
	}
#endif
}

ansicres()	/* change screen resolution */
{
	return(TRUE);
}

spal(dummy)		/* change pallette settings */
{
	/* none for now */
}

ansibeep()
{
	ttputc(BEL);
	ttflush();
}

#if SCROLLCODE

/* if your ansi terminal can scroll regions, like the vt100, then define
	SCROLL_REG.  If not, you can use delete/insert line code, which
	is prettier but slower if you do it a line at a time instead of
	all at once.
*/

#define SCROLL_REG 1

/* move howmany lines starting at from to to */
ansiscroll(from,to,n)
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
		ansimove(from);
		for (i = to - from; i > 0; i--) {
			ttputc(ESC);
			ttputc('M');
		}
	}
	ansiscrollregion(0, term.t_mrow);
	        
#else /* use insert and delete line */
#if PRETTIER_SCROLL
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
		if (from - to > 1) ansiparm(from - to);
		ttputc('M'); /* delete */
		ansimove(to+n,0);
		csi();
		if (from - to > 1) ansiparm(from - to);
		ttputc('L'); /* insert */
	} else {
		ansimove(from+n,0);
		csi();
		if (to - from > 1) ansiparm(to - from);
		ttputc('M'); /* delete */
		ansimove(from,0);
		csi();
		if (to - from > 1) ansiparm(to - from);
		ttputc('L'); /* insert */
	}
#endif
}

#if SCROLL_REG
ansiscrollregion(top,bot)
{
	csi();
	if (top) ansiparm(top + 1);
	ttputc(';');
	if (bot != term.t_nrow) ansiparm(bot + 1);
	ttputc('r');
}
#endif

#endif

ansiparm(n)
register int	n;
{
	register int q,r;

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

ansiopen()
{
	strcpy(sres, "NORMAL");
	revexist = TRUE;
	ttopen();
}

ansiclose()
{
#if	COLOR
	ansifcol(7);
	ansibcol(0);
#endif
	ttclose();
}

ansikopen()	/* open the keyboard (a noop here) */
{
}

ansikclose()	/* close the keyboard (a noop here) */
{
}

#if	FLABEL
int
fnclabel(f, n)		/* label a function key */
int f,n;	/* default flag, numeric argument [unused] */
{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
ansihello()
{
}
#endif
