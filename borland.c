
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
 * $Log: borland.c,v $
 * Revision 1.3  1994/04/22 16:38:33  pgf
 * *** empty log message ***
 *
 * Revision 1.2  1994/04/20  20:00:26  pgf
 * use putch('\a') to get a beep
 *
 * Revision 1.1  1994/04/20  19:54:50  pgf
 * changes to support 'BORLAND' console i/o screen driver
 *
 * Revision 1.0  1994/04/18  13:52:13  pgf
 * Initial revision
 *
 *
 */


#if OS2
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
char *initpalettestr = "0 4 2 14 1 5 3 15";
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
	NROW-1,
	NROW-1,
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


static void
set_cursor(int cmode)
{
	switch (cmode) {
	case 0: _setcursortype( _NOCURSOR);	break;
	case 1: _setcursortype( _SOLIDCURSOR);	break;
	case 2: _setcursortype( _NORMALCURSOR);	break;
	} 
}


/*--------------------------------------------------------------------------*/

void
borfcol(color)		/* set the current output color */
int color;	/* color to set */
{
	cfcolor = ctrans[color];
	textcolor(cfcolor & 15);
}

void
borbcol(color)		/* set the current background color */
int color;	/* color to set */
{
	cbcolor = ctrans[color];
	textbackground(cbcolor & 7);
}

void
bormove(row, col)
int row, col;
{
	gotoxy(col+1, row+1);
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
		if (i==2)  status=scinit(25); 
		if (i==4)  status=scinit(43);
		if (i==5)  status=scinit(50);

		if (i>5 && i<28) 
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
	borfcol(gfcolor);
	borbcol(gbcolor);
	if (!borcres(current_res_name))
		(void)scinit(-1);
}


void
borclose()
{
	int	current_type = ibmtype;

	set_cursor(_NORMALCURSOR);
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

#ifdef C80X21
		case 21:	/* color C80X21 */
				textmode(C80X21);
				newscreensize(21, term.t_ncol);
				(void)strcpy(sres, "C80X21");
				break;
#endif

		default:
		case 25:	/* Color graphics adapter */
				textmode(C80);
				newscreensize(25, term.t_ncol);
				(void)strcpy(sres, "C80");
				break;

#ifdef C80X28
		case 28:	/* Enhanced graphics adapter */
				textmode(C80X28);
				newscreensize(28, term.t_ncol);
				(void)strcpy(sres, "C80X28");
				break;
#endif

#ifdef C80X43
#define have43 1
		case 43:	/* Enhanced graphics adapter */
				textmode(C80X43);
				newscreensize(43, term.t_ncol);
				(void)strcpy(sres, "C80X43");
				break;
#endif

#ifdef C80X50
#define have50 1
		case 50:	/* VGA adapter */
				textmode(C80X50);
				newscreensize(50, term.t_ncol);
				(void)strcpy(sres, "C80X50");
				break;
#endif

#ifdef C80X60
		case 60:	/* Enhanced graphics adapter */
				textmode(C80X60);
				newscreensize(60, term.t_ncol);
				(void)strcpy(sres, "C80X60");
				break;
#endif

#if !have43 || !have50
#ifdef C4350
#if !have43
		case 43:
#endif
#if !have50
		case 50:
#endif
				/* Enhanced graphics adapter */
				/* FIXXX this needs to ask how big 
					we get after C4350, and call
					newscreensize appropriately */
				textmode(C4350);
				newscreensize(50, term.t_ncol);
				(void)strcpy(sres, "C80X50");
				break;
#endif /* C4350 */
#endif /* have...*/

	}

	ibmtype = rows;

	return(TRUE);
}


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
	if (to == from) return;
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
}
#endif	/* SCROLLCODE */


/*--------------------------------------------------------------------------*/

