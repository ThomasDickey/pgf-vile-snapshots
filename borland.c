
/*
 * Uses Borland console i/o routines.
 * (Created for OS/2 by Charles Moschel 29-MAR-94.)
 * (modified to be more generic, not os/2 specific, pgf, april '94)
 *
 * Supported monitor cards include
 *	CGA, MONO, EGA, VGA.
 *
 * Note: Visual flashes are not yet supported.
 *
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/borland.c,v 1.7 1994/10/10 13:22:06 pgf Exp $
 *
 */


#if defined __OS2__    /* Catch-22: OS2 is not defined until estruct.h */
#define INCL_VIO
#define INCL_NOPMAPI
#include <os2.h>

#undef OFFSETOF
#undef COLOR
#undef COLOR 1

#endif /* OS2 */

#define	termdef	1			/* don't define "term" external */

#include        "estruct.h"
#include        "edef.h"

#if !BORLAND || IBMPC
#error misconfigured:  BORLAND should be defined if using borland.c
#error (and IBMPC should not be defined)
#endif

#define NROW	60			/* Max Screen size.		*/
#define NCOL    80			/* Edit if you want to.         */
#define	MARGIN	8			/* size of minimum margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define	SPACE	32			/* space character		*/

#define VIO 0				/* TESTING, not working yet	*/

/* We assume that most users have a color display.  */

#include <conio.h>


#define	AttrColor(b,f)	(((ctrans[b] & 7) << 4) | (ctrans[f] & 15))

extern  void	bormove   P((int,int));
extern  void	boreeol   P((void));
extern  void	boreeop   P((void));
extern  void	borbeep   P((void));
extern  void    boropen   P((void));
extern	void	borrev    P((int));
extern	int	borcres   P((char *));
extern	void	borclose  P((void));
extern	void	borputc   P((int));
extern	void	borkopen  P((void));
extern	void	borkclose P((void));

extern	void	borfcol   P((int));
extern	void	borbcol   P((int));

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[NCOLORS];
/* ansi to ibm color translation table */
char *initpalettestr = "0 4 2 14 1 5 3 7";  /* 15 is too bright */
/* black, red, green, yellow, blue, magenta, cyan, white   */

#if	SCROLLCODE
extern	void	borscroll P((int,int,int));
#endif

static	int	scinit    P((int));
static	int	scblank   P((void));


int ibmtype;

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
	NROW,
	NROW,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	boropen,
	borclose,
	borkopen,
	borkclose,
	ttgetc,
	borputc,
	ttflush,
	bormove,
	boreeol,
	boreeop,
	borbeep,
	borrev,
	borcres,
	borfcol,
	borbcol,
#if	SCROLLCODE
	borscroll
#endif
};

void
set_cursor(int cmode)
{
	switch (cmode) {
	case -1: _setcursortype( _NOCURSOR);		break;
	case  0: _setcursortype( _NORMALCURSOR);	break;
	case  1: _setcursortype( _SOLIDCURSOR);		break;
	} 
}


/*--------------------------------------------------------------------------*/
/* FIXX these: the borland textcolor sets attributes for text drawn after
 * the call, not whats already displayed.  sgarbf will work, by forcing a
 * redraw, but it's done twice in opening the screen (ugly).  If we don't
 * do it here, then setting background or foreground color later won't
 * update the screen (ugly).
 */
void
borfcol(color)		/* set the current output color */
int color;	/* color to set */
{
	cfcolor = ctrans[color];
	textcolor(cfcolor & 15);
	sgarbf = TRUE;
}

void
borbcol(color)		/* set the current background color */
int color;	/* color to set */
{
	cbcolor = ctrans[color];
	textbackground(cbcolor & 7);
	sgarbf = TRUE;
}

void
bormove(row, col)
int row, col;
{
#if VIO
	VioSetCurPos(row, col, 0);
#else
	gotoxy(col+1, row+1);
#endif
}

/* erase to the end of the line */
void
boreeol()
{
	clreol();		/* pointer to the destination line */
}

/* put a character at the current position in the current colors */
void
borputc(ch)
int ch;
{
	putch(ch);
}

void
boreeop()
{
	clrscr();
}

void
borrev(reverse)		/* change reverse video state */
int reverse;	/* TRUE = reverse, FALSE = normal */
{
	if (reverse) {
	    textbackground(cfcolor & 7);
	    textcolor(cbcolor & 15);
	} else {
	    textbackground(cbcolor & 7);
	    textcolor(cfcolor & 15);
	}
}

int
borcres(res)	/* change screen resolution */
char *res;	/* resolution to change to */
{
	char	*dst;
	register int i;		/* index */
	int	status = FALSE;

	/* find the default configuration */
	if (!strcmp(res, "?")) {
		status = scinit(-1);
	} else {	/* specify a number */
		if ((i = (int)strtol(res, &dst, 0)) >= 0 && !*dst)
		{
		/* only allow valid row selections */
		/* Are these all valid under dos?  */
		if (i==2)  status=scinit(25); 
		if (i==4)  status=scinit(43);
		if (i==5)  status=scinit(50);
		if (i==6)  status=scinit(60);

		if (i>6 && i<28) 
			status=scinit(25);
			
		if (i>=28 && i<43) 
			status=scinit(28);

		if (i>=43 && i<50) 
			status=scinit(43);

		if (i>=50 && i<60)
			status=scinit(50);

		if (i>=60)
			status=scinit(60);

		}
	}
	sgarbf = TRUE;
	return status;
}

void
spal(palstr)	/* reset the palette registers */
char *palstr;
{
    	/* this is pretty simplistic.  big deal. */
	(void)sscanf(palstr,"%i %i %i %i %i %i %i %i",
	    	&ctrans[0], &ctrans[1], &ctrans[2], &ctrans[3],
	    	&ctrans[4], &ctrans[5], &ctrans[6], &ctrans[7] );
}


void
borbeep()
{
	putch('\a');
}


void
boropen()
{
	spal(initpalettestr);
	borfcol(gfcolor);
	borbcol(gbcolor);
	if (!borcres(current_res_name))
		(void)scinit(-1);
	ttopen();
}


void
borclose()
{
	int	current_type = ibmtype;

	_setcursortype(_NORMALCURSOR);
	ibmtype = current_type;	/* ...so subsequent TTopen restores us */

	movecursor(0,0);	/* clear the screen */
	TTeeop();
}

void
borkopen()	/* open the keyboard */
{
	/* ms_install(); */
}

void
borkclose()	/* close the keyboard */
{
	/* ms_deinstall(); */
}

#if OS2  		/* all modes are available under OS/2 */
static
int scinit(rows)	/* initialize the screen head pointers */
int rows;		/* Number of rows. only do 80 col */
{

	/* and set up the various parameters as needed */

	if (rows == -1)
	{
		struct text_info ti;
		gettextinfo(&ti);
		rows = ti.screenheight;
	}

	switch (rows) {

/* these are enum's, and thus cannot easily be checked, ie. #ifdef C80X21 */
		case 21:	/* color C80X21 */
				textmode(C80X21);
				newscreensize(21, term.t_ncol);
				(void)strcpy(sres, "C80X21");
				break;

		default:
		case 25:	/* Color graphics adapter */
				textmode(C80);
				newscreensize(25, term.t_ncol);
				(void)strcpy(sres, "C80");
				break;

		case 28:	/* Enhanced graphics adapter */
				textmode(C80X28);
				newscreensize(28, term.t_ncol);
				(void)strcpy(sres, "C80X28");
				break;

		case 43:	/* Enhanced graphics adapter */
				textmode(C80X43);
				newscreensize(43, term.t_ncol);
				(void)strcpy(sres, "C80X43");
				break;

		case 50:	/* VGA adapter */
				textmode(C80X50);
				newscreensize(50, term.t_ncol);
				(void)strcpy(sres, "C80X50");
				break;

		case 60:	/* Enhanced graphics adapter */
				textmode(C80X60);
				newscreensize(60, term.t_ncol);
				(void)strcpy(sres, "C80X60");
				break;


	}

	ibmtype = rows;

	return(TRUE);
}

#else /* OS2 */

static
int scinit(rows)	/* initialize the screen head pointers */
int rows;		/* Number of rows. only do 80 col */
{

	/* and set up the various parameters as needed */

	if (rows == -1)
	{
		struct text_info ti;
		gettextinfo(&ti);
		rows = ti.screenheight;
	}

	switch (rows) {

/* DOS has only (?) BW40, C40, BW80, C80, MONO, and C4350 */

		default:
		case 25:	/* Color graphics adapter */
				textmode(C80);
				newscreensize(25, term.t_ncol);
				(void)strcpy(sres, "C80");
				break;

		case 43:
		case 50:
				/* Enhanced graphics adapter */
				/* FIXXX this needs to ask how big 
					we get after C4350, and call
					newscreensize appropriately */
				textmode(C4350);
				newscreensize(50, term.t_ncol);
				(void)strcpy(sres, "C80X50");
				break;

	}

	ibmtype = rows;

	return(TRUE);
}

#endif /* OS2 */

/* returns attribute for blank/empty space */
static int
scblank()
{
	return AttrColor(gbcolor,gfcolor);
}

#if SCROLLCODE
/*
 * Move 'n' lines starting at 'from' to 'to'
 *
 * PRETTIER_SCROLL is prettier but slower -- it scrolls a line at a time
 *	instead of all at once.
 */

/* move howmany lines starting at from to to */
void
borscroll(from,to,n)
int from, to, n;
{
	int i;
	unsigned char a = (unsigned)scblank();
	if (to == from) return;
#if VIO
	VioScrollUp(min(to,from),0,max(to,from),term.t_ncol,n,&a,0);
#else
#if PRETTIER_SCROLL
	if (absol(from-to) > 1) {
		borscroll(from, (from<to) ? to-1:to+1, n);
		if (from < to)
			from = to-1;
		else
			from = to+1;    
	}
#endif
	if (to < from) {
		bormove(to,0);
		for (i = from - to; i > 0; i--)
			delline();
		bormove(to+n,0);
		for (i = from - to; i > 0; i--)
			insline();
	} else {
		bormove(from+n,0);
		for (i = to - from; i > 0; i--)
			delline();
		bormove(from,0);
		for (i = to - from; i > 0; i--)
			insline();
	}
#endif	/* VIO */
}
#endif	/* SCROLLCODE */


/*--------------------------------------------------------------------------*/

