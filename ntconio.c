/*
 * Uses Windows/NT console i/o routines.
 *
 * Supported monitor cards include
 *	CGA, MONO, EGA, VGA.
 *
 * Note: Visual flashes are not yet supported.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/ntconio.c,v 1.3 1994/09/13 17:15:48 pgf Exp $
 *
 */

#include <windows.h>

#define	termdef	1			/* don't define "term" external */

#include        "estruct.h"
#include        "edef.h"


#define NROW	60			/* Max Screen size.		*/
#define NCOL    80			/* Edit if you want to.         */
#define	MARGIN	8			/* size of minimum margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define	SPACE	32			/* space character		*/

/* We assume that most users have a color display.  */


#define	AttrColor(b,f)	((WORD)(((ctrans[b] & 7) << 4) | (ctrans[f] & 15)))
extern  void	ntmove   P((int,int));
extern  void	nteeol   P((void));
extern  void	nteeop   P((void));
extern  void	ntbeep   P((void));
extern  void    ntopen   P((void));
extern	void	ntrev    P((int));
extern	int	ntcres   P((char *));
extern	void	ntclose  P((void));
extern	void	ntputc   P((int));
extern	void	ntkopen  P((void));
extern	void	ntkclose P((void));
extern	void	ntfcol   P((int));
extern	void	ntbcol   P((int));



HANDLE hConsoleOutput;		/* handle to the console display */
HANDLE hConsoleInput;
CONSOLE_SCREEN_BUFFER_INFO csbi;
WORD originalAttribute;

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	nfcolor = -1;		/* normal foreground color */
int	nbcolor = -1;		/* normal background color */
int	ctrans[NCOLORS];

/* ansi to ibm color translation table */
char *initpalettestr = "0 4 2 14 1 5 3 15";
/* black, red, green, yellow, blue, magenta, cyan, white   */

#if	SCROLLCODE
extern	void	ntscroll P((int,int,int));
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
	ntopen,
	ntclose,
	ntkopen,
	ntkclose,
	ttgetc,
	ntputc,
	ttflush,
	ntmove,
	nteeol,
	nteeop,
	ntbeep,
	ntrev,
	ntcres,
	ntfcol,
	ntbcol,
#if	SCROLLCODE
	ntscroll
#endif
};


static void
set_cursor(int cmode)
{
	CONSOLE_CURSOR_INFO cci;

	switch (cmode) {
	case 0:
		cci.dwSize = 0;
		cci.bVisible = FALSE;
		break;
	case 1:
		cci.dwSize = 100;
		cci.bVisible = TRUE;
		break;
	case 2:
		cci.dwSize = 1;
		cci.bVisible = TRUE;
		break;
	}
	SetConsoleCursorInfo(hConsoleOutput, &cci);
}


/*--------------------------------------------------------------------------*/

void
ntfcol(color)		/* set the current output color */
int color;	/* color to set */
{
	WORD attr;

	nfcolor = cfcolor = color;
	SetConsoleTextAttribute(hConsoleOutput, AttrColor(cbcolor, cfcolor));
}

void
ntbcol(color)		/* set the current background color */
int color;	/* color to set */
{
	nbcolor = cbcolor = color;

	SetConsoleTextAttribute(hConsoleOutput, AttrColor(cbcolor, cfcolor));
}

void
ntmove(row, col)
int row, col;
{
	COORD coordCursor = {col, row};

	SetConsoleCursorPosition(hConsoleOutput, coordCursor);
}

/* erase to the end of the line */
void
nteeol()
{
	DWORD written;

	GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
	FillConsoleOutputCharacter(
	    hConsoleOutput,
	    ' ',
	    csbi.dwMaximumWindowSize.X - csbi.dwCursorPosition.X,
	    csbi.dwCursorPosition,
	    &written
	    );
	FillConsoleOutputAttribute(
	    hConsoleOutput,
	    AttrColor(cbcolor, cfcolor),
	    csbi.dwMaximumWindowSize.X - csbi.dwCursorPosition.X,
	    csbi.dwCursorPosition,
	    &written
	    );
}

/* put a character at the current position in the current colors */
void
ntputc(ch)
int ch;
{
	DWORD written;

	WriteConsole(hConsoleOutput, &ch, 1, &written, NULL);
}

void
nteeop()
{
	DWORD cnt;
	DWORD written;

	GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
	cnt = csbi.dwMaximumWindowSize.X - csbi.dwCursorPosition.X
	    + (csbi.dwMaximumWindowSize.Y - csbi.dwCursorPosition.Y - 1) *
	    csbi.dwMaximumWindowSize.X;
	FillConsoleOutputCharacter(
	    hConsoleOutput,
	    ' ',
	    cnt,
	    csbi.dwCursorPosition,
	    &written
	    );
	FillConsoleOutputAttribute(
	    hConsoleOutput,
	    AttrColor(cbcolor, cfcolor),
	    cnt,
	    csbi.dwCursorPosition,
	    &written
	    );
}

void
ntrev(reverse)		/* change reverse video state */
int reverse;	/* TRUE = reverse, FALSE = normal */
{
	if (reverse) {
		cbcolor = nfcolor;
		cfcolor = nbcolor;
		SetConsoleTextAttribute(
		    hConsoleOutput,
		    AttrColor(cbcolor, cfcolor)
		    );
	} else {
		cbcolor = nbcolor;
		cfcolor = nfcolor;
		SetConsoleTextAttribute(
		    hConsoleOutput,
		    AttrColor(cbcolor, cfcolor)
		    );
	}
}

int
ntcres(res)	/* change screen resolution */
char *res;	/* resolution to change to */
{
	return 0;
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
ntbeep()
{
	MessageBeep(0xffffffff);
}


void
ntopen()
{
	spal(initpalettestr);
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
	originalAttribute = csbi.wAttributes;
	ntfcol(gfcolor);
	ntbcol(gbcolor);
	newscreensize(csbi.dwMaximumWindowSize.Y, csbi.dwMaximumWindowSize.X);
}


void
ntclose()
{
	int	current_type = ibmtype;

	set_cursor(2);
	ibmtype = current_type;	/* ...so subsequent TTopen restores us */

	movecursor(0,0);	/* clear the screen */
	TTeeop();
	SetConsoleTextAttribute(hConsoleOutput, originalAttribute);
}

void
ntkopen()	/* open the keyboard */
{
	hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(hConsoleInput, ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT);
}

void
ntkclose()	/* close the keyboard */
{
	/* ms_deinstall(); */
}

int
ntgetch()
{
	INPUT_RECORD ir;
	DWORD nr;
	static int savedChar;
	static int saveCount = 0;
	int buttondown = FALSE;
	COORD first;
	COORD top;
	COORD bottom;
	COORD oTop;
	COORD oBottom;
	COORD tmpTop;
	COORD tmpBottom;
	DWORD written;

	if (saveCount > 0) {
		saveCount--;
		return savedChar;
	}
	for (;;) {
		ReadConsoleInput(hConsoleInput, &ir, 1, &nr);
		switch(ir.EventType) {
		case KEY_EVENT:
			if (!ir.Event.KeyEvent.bKeyDown
			    || ir.Event.KeyEvent.uChar.AsciiChar == 0) {
				continue;
			}
			if (ir.Event.KeyEvent.wRepeatCount > 1) {
				saveCount = ir.Event.KeyEvent.wRepeatCount - 1;
				savedChar = ir.Event.KeyEvent.uChar.AsciiChar;
			}
			return (int)ir.Event.KeyEvent.uChar.AsciiChar;

		case WINDOW_BUFFER_SIZE_EVENT:
			newscreensize(ir.Event.WindowBufferSizeEvent.dwSize.Y,
			    ir.Event.WindowBufferSizeEvent.dwSize.X);
			continue;
		case MOUSE_EVENT:
			if (ir.Event.MouseEvent.dwEventFlags == 0) {
				if (ir.Event.MouseEvent.dwButtonState == 0) {
					if (buttondown) {
						WORD cnt;

						if (top.Y == bottom.Y) {
							cnt = bottom.X - top.X;
						} else {
							cnt = csbi.dwMaximumWindowSize.X - top.X + bottom.X + csbi.dwMaximumWindowSize.X * (bottom.Y - top.Y - 1);
						}
						FillConsoleOutputAttribute(
						    hConsoleOutput,
						    AttrColor(cbcolor, cfcolor),
						    cnt,
						    top,
						    &written
						    );
						buttondown = FALSE;
						
						if (ir.Event.MouseEvent.dwMousePosition.Y < 0)
							ir.Event.MouseEvent.dwMousePosition.Y = 0;
						if (ir.Event.MouseEvent.dwMousePosition.Y > csbi.dwMaximumWindowSize.Y - 3) 
							ir.Event.MouseEvent.dwMousePosition.Y = csbi.dwMaximumWindowSize.Y - 3;
						if (ir.Event.MouseEvent.dwMousePosition.X < 0)
							ir.Event.MouseEvent.dwMousePosition.X = 0;
						if (ir.Event.MouseEvent.dwMousePosition.X > csbi.dwMaximumWindowSize.X - 1) 
							ir.Event.MouseEvent.dwMousePosition.X = csbi.dwMaximumWindowSize.X - 1;
							
						setcursor(first.Y, first.X);
						setwmark(ir.Event.MouseEvent.dwMousePosition.Y, ir.Event.MouseEvent.dwMousePosition.X);
						if (!same_ptr(DOT.l,MK.l)
						 || DOT.o != MK.o) {
							regionshape = EXACT;
							(void)yankregion();
							(void)update(TRUE);
						}
						(void)update(FALSE);
					}
				}
				if (ir.Event.MouseEvent.dwButtonState == 1) {
					buttondown = TRUE;
					/*
					setcursor(ir.Event.MouseEvent.dwMousePosition.Y, ir.Event.MouseEvent.dwMousePosition.X);
					*/
					first = ir.Event.MouseEvent.dwMousePosition;
				}
			}
			if (ir.Event.MouseEvent.dwEventFlags == MOUSE_MOVED) {
				WORD cnt;

				if (!buttondown) {
					continue;
				}
				sgarbf = TRUE;

				if (ir.Event.MouseEvent.dwMousePosition.Y < first.Y) {
					top = ir.Event.MouseEvent.dwMousePosition;
					bottom = first;
				} else if (ir.Event.MouseEvent.dwMousePosition.Y > first.Y) {
					top = first;
					bottom = ir.Event.MouseEvent.dwMousePosition;
				} else if (ir.Event.MouseEvent.dwMousePosition.X <= first.X) {
					top = ir.Event.MouseEvent.dwMousePosition;
					bottom = first;
				} else {
					top = first;
					bottom = ir.Event.MouseEvent.dwMousePosition;
				}

				GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
				if (top.Y < 0)
					top.Y = 0;
				if (top.Y > csbi.dwMaximumWindowSize.Y - 3)
					top.Y = csbi.dwMaximumWindowSize.Y - 3;
				if (bottom.Y < 0)
					bottom.Y = 0;
				if (bottom.Y > csbi.dwMaximumWindowSize.Y - 3)
					bottom.Y = csbi.dwMaximumWindowSize.Y - 3;
				if (top.X < 0)
					top.X = 0;
				if (top.X > csbi.dwMaximumWindowSize.X - 1)
					top.X = csbi.dwMaximumWindowSize.X - 1;
				if (bottom.X < 0)
					bottom.X = 0;
				if (bottom.X > csbi.dwMaximumWindowSize.X - 1)
					bottom.X = csbi.dwMaximumWindowSize.X - 1;

				if (oTop.Y != top.Y || oTop.X != top.X) {
					if (oTop.Y < top.Y) {
						tmpTop = oTop;
						tmpBottom = top;
					} else if (oTop.Y > top.Y) {
						tmpTop = top;
						tmpBottom = oTop;
					} else if (oTop.X < top.X) {
						tmpTop = oTop;
						tmpBottom = top;
					} else {
						tmpTop = top;
						tmpBottom = oTop;
					}
				}
				if (oBottom.Y != bottom.Y || oBottom.X != bottom.X) {
					if (oBottom.Y < bottom.Y) {
						tmpTop = oBottom;
						tmpBottom = bottom;
					} else if (oBottom.Y > bottom.Y) {
						tmpTop = bottom;
						tmpBottom = oBottom;
					} else if (oBottom.X < bottom.X) {
						tmpTop = oBottom;
						tmpBottom = bottom;
					} else {
						tmpTop = bottom;
						tmpBottom = oBottom;
					}
				}
					
				if (tmpTop.Y == tmpBottom.Y) {
					cnt = tmpBottom.X - tmpTop.X;
				} else {
					cnt = csbi.dwMaximumWindowSize.X - tmpTop.X + tmpBottom.X + csbi.dwMaximumWindowSize.X * (tmpBottom.Y - tmpTop.Y - 1);
				}
				FillConsoleOutputAttribute(
				    hConsoleOutput,
				    AttrColor(cbcolor, cfcolor),
				    cnt,
				    tmpTop,
				    &written
				    );
				if (top.Y == bottom.Y) {
					cnt = bottom.X - top.X;
				} else {
					cnt = csbi.dwMaximumWindowSize.X - top.X + bottom.X + csbi.dwMaximumWindowSize.X * (bottom.Y - top.Y - 1);
				}
				FillConsoleOutputAttribute(
				    hConsoleOutput,
				    AttrColor(cfcolor, cbcolor),
				    cnt,
				    top,
				    &written
				    );
				oTop = top;
				oBottom = bottom;
			}
			break;
		}
	}
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
ntscroll(from,to,n)
int from, to, n;
{
	int i;
	SMALL_RECT sRect;
	SMALL_RECT cRect;
	COORD dest;
	CHAR_INFO fill;

	if (to == from) return;
#if PRETTIER_SCROLL
	if (absol(from-to) > 1) {
		ntscroll(from, (from<to) ? to-1:to+1, n);
		if (from < to)
			from = to-1;
		else
			from = to+1;
	}
#endif
	fill.Char.AsciiChar = ' ';
	fill.Attributes = AttrColor(cbcolor, cfcolor);
	GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);

	cRect.Left = 0;
	cRect.Top = 0;
	cRect.Right = csbi.dwMaximumWindowSize.X;
	cRect.Bottom = csbi.dwMaximumWindowSize.Y;

	sRect.Left = 0;
	sRect.Top = from;
	sRect.Right = csbi.dwMaximumWindowSize.X;
	sRect.Bottom = from + n - 1;

	dest.X = 0;
	dest.Y = to;

	ScrollConsoleScreenBuffer(
	    hConsoleOutput,
	    &sRect,
	    &cRect,
	    dest,
	    &fill
	    );
}
#endif	/* SCROLLCODE */

