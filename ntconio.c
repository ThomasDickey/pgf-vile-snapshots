/*
 * Uses the Win32 console API.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/ntconio.c,v 1.11 1995/11/17 04:03:42 pgf Exp $
 *
 */

#include <windows.h>

#define	termdef	1			/* don't define "term" external */

#include        "estruct.h"
#include        "edef.h"

#define NROW	128			/* Max Screen size.		*/
#define NCOL    256			/* Edit if you want to.         */
#define	MARGIN	8			/* size of minimum margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */

#define	AttrColor(b,f)	((WORD)(((ctrans[b] & 7) << 4) | (ctrans[f] & 15)))

extern	void	ntmove		(int, int);
extern	void	nteeol		(void);
extern	void	nteeop		(void);
extern	void	ntbeep		(void);
extern	void	ntopen		(void);
extern	void	ntrev		(int);
extern	int	ntcres		(char *);
extern	void	ntclose		(void);
extern	void	ntputc		(int);
extern	int	nttypahead	(void);
extern	void	ntkopen		(void);
extern	void	ntkclose	(void);
#if OPT_COLOR
extern	void	ntfcol		(int);
extern	void	ntbcol		(int);
extern	void	ntspal		(char *);
#endif
extern	void	ntflush		(void);
extern	void	ntscroll	(int, int, int);

static HANDLE hConsoleOutput;		/* handle to the console display */
static HANDLE hConsoleInput;
static CONSOLE_SCREEN_BUFFER_INFO csbi;
static WORD originalAttribute;

static int	cfcolor = -1;		/* current forground color */
static int	cbcolor = -1;		/* current background color */
static int	nfcolor = -1;		/* normal foreground color */
static int	nbcolor = -1;		/* normal background color */
static int	crow = -1;		/* current row */
static int	ccol = -1;		/* current col */
static int	ctrans[NCOLORS];

/* ansi to ibm color translation table */
static char *initpalettestr = "0 4 2 14 1 5 3 7";
/* black, red, green, yellow, blue, magenta, cyan, white   */

static	char	linebuf[NCOL];
static	int	bufpos = 0;

static  void	scflush  (void);

/*
 * The 950 build of Windows 95 has broken scrolling.  If we should cater
 * to such broken software (even if only to protect our own reputation)
 * then detect and work around this problem.
 */

#define SUPPORT_BROKEN_SCROLLING 1

/*
 * Standard terminal interface dispatch table. None of the fields point into
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
	nttypahead,
	ntflush,
	ntmove,
	nteeol,
	nteeop,
	ntbeep,
	ntrev,
	ntcres,
#if OPT_COLOR
	ntfcol,
	ntbcol,
	ntspal,
#endif
	ntscroll
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

#if OPT_COLOR
void
ntfcol(int color)		/* set the current output color */
{
	scflush();
	nfcolor = cfcolor = color;
}

void
ntbcol(int color)		/* set the current background color */
{
	scflush();
	nbcolor = cbcolor = color;
}

void
ntspal(char *thePalette)	/* reset the palette registers */
{
    	/* this is pretty simplistic.  big deal. */
	sscanf(thePalette, "%i %i %i %i %i %i %i %i",
		&ctrans[0], &ctrans[1], &ctrans[2], &ctrans[3],
		&ctrans[4], &ctrans[5], &ctrans[6], &ctrans[7]);
}
#endif

void
scflush(void)
{
	if (bufpos) {
		COORD coordCursor;
		DWORD written;

		coordCursor.X = ccol;
		coordCursor.Y = crow;
		WriteConsoleOutputCharacter(
			hConsoleOutput, linebuf, bufpos, coordCursor, &written
		);
		FillConsoleOutputAttribute(
			hConsoleOutput, AttrColor(cbcolor, cfcolor),
			bufpos, coordCursor, &written
		);
		ccol += bufpos;
		bufpos = 0;
	}
}

void
ntflush(void)
{
        COORD coordCursor;

	scflush();
        coordCursor.X = ccol;
        coordCursor.Y = crow;
	SetConsoleCursorPosition(hConsoleOutput, coordCursor);
}

void
ntmove(int row, int col)
{
	scflush();
	crow = row;
	ccol = col;
}

/* erase to the end of the line */
void
nteeol(void)
{
	DWORD written;
        COORD coordCursor;

	scflush();
        coordCursor.X = ccol;
        coordCursor.Y = crow;
	FillConsoleOutputCharacter(
		hConsoleOutput, ' ', csbi.dwMaximumWindowSize.X - ccol,
		coordCursor, &written
	);
	FillConsoleOutputAttribute(
		hConsoleOutput, AttrColor(cbcolor, cfcolor),
		csbi.dwMaximumWindowSize.X - ccol, coordCursor, &written
	);
}

/*
 * vile very rarely generates any of the ASCII printing control characters
 * except for a few hand coded routines but we have to support them anyway.
 */

/* put a character at the current position in the current colors */
void
ntputc(int ch)
{
	/* This is an optimization for the most common case. */
	if (ch >= ' ') {
		linebuf[bufpos++] = ch;
		return;
	}

	switch (ch) {

	case '\b':
		scflush();
		if (ccol)
			ccol--;
		break;

	case '\a':
		ntbeep();
		break;

	case '\t':
		scflush();
		do
			linebuf[bufpos++] = ' ';
		while (bufpos % 8 != 0);
		break;

	case '\r':
		scflush();
		ccol = 0;
		break;

	case '\n':
		scflush();
		if (crow < csbi.dwMaximumWindowSize.Y - 1)
			crow++;
		else
			ntscroll(1, 0, csbi.dwMaximumWindowSize.Y - 1);
		break;

	default:
		linebuf[bufpos++] = ch;
		break;
	}
}

void
nteeop(void)
{
	DWORD cnt;
	DWORD written;
        COORD coordCursor;

	scflush();
        coordCursor.X = ccol;
        coordCursor.Y = crow;
	cnt = csbi.dwMaximumWindowSize.X - ccol
		+ (csbi.dwMaximumWindowSize.Y - crow - 1)
		* csbi.dwMaximumWindowSize.X;
	FillConsoleOutputCharacter(
		hConsoleOutput, ' ', cnt, coordCursor, &written
	);
	FillConsoleOutputAttribute(
		hConsoleOutput, AttrColor(cbcolor, cfcolor), cnt,
		coordCursor, &written
	);
}

void
ntrev(int reverse)		/* change reverse video state */
{
	scflush();
	if (reverse) {
		cbcolor = nfcolor;
		cfcolor = nbcolor;
	}
	else {
		cbcolor = nbcolor;
		cfcolor = nfcolor;
	}
}

int
ntcres(char *res)	/* change screen resolution */
{
	scflush();
	return 0;
}

void
ntbeep(void)
{
	MessageBeep(0xffffffff);
}

#if SUPPORT_BROKEN_SCROLLING

static int
detect_broken_scrolling(void)
{
	OSVERSIONINFO osvi;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi) == TRUE) {
/* If we have an older SDK then this won't be defined yet. */
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS 1
#endif
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
#if 0
			if (osvi.dwBuildNumber < /* first working build */)
#endif
				return 1;
		}
	}
	return 0;
}

static int backfill_scrolled_areas_manually = 0;

#endif

void
ntopen(void)
{
	set_palette(initpalettestr);
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
	originalAttribute = csbi.wAttributes;
	crow = csbi.dwCursorPosition.Y;
	ccol = csbi.dwCursorPosition.X;
	nfcolor = cfcolor = gfcolor;
	nbcolor = cbcolor = gbcolor;
	newscreensize(csbi.dwMaximumWindowSize.Y, csbi.dwMaximumWindowSize.X);

#if SUPPORT_BROKEN_SCROLLING
	backfill_scrolled_areas_manually = detect_broken_scrolling();
#endif

}

void
ntclose(void)
{
	scflush();
	ntmove(csbi.dwMaximumWindowSize.Y - 1, 0);
	nteeol();
	ntflush();
	SetConsoleTextAttribute(hConsoleOutput, originalAttribute);
}

void
ntkopen(void)	/* open the keyboard */
{
	hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(hConsoleInput, ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT);
}

void
ntkclose(void)	/* close the keyboard */
{
}

static struct {
	int	windows;
	int	vile;
	int	shift;
} keyxlate[] = {
	VK_NEXT,	KEY_Next,	0,
	VK_PRIOR,	KEY_Prior,	0,
	VK_END,		KEY_End,	0,
	VK_HOME,	KEY_Home,	0,
	VK_LEFT,	KEY_Left,	0,
	VK_RIGHT,	KEY_Right,	0,
	VK_UP,		KEY_Up,		0,
	VK_DOWN,	KEY_Down,	0,
	VK_INSERT,	KEY_Insert,	0,
	VK_DELETE,	KEY_Delete,	0,
	VK_HELP,	KEY_Help,	0,
	VK_SELECT,	KEY_Select,	0,
#if 0
	/* Merely pressing the Alt key generates a VK_MENU key event. */
	VK_MENU,	KEY_Menu,	0,
#endif
	VK_F1,		KEY_F1,		0,
	VK_F2,		KEY_F2,		0,
	VK_F3,		KEY_F3,		0,
	VK_F4,		KEY_F4,		0,
	VK_F5,		KEY_F5,		0,
	VK_F6,		KEY_F6,		0,
	VK_F7,		KEY_F7,		0,
	VK_F8,		KEY_F8,		0,
	VK_F9,		KEY_F9,		0,
	VK_F10,		KEY_F10,	0,
	VK_F11,		KEY_F11,	0,
	VK_F12,		KEY_F12,	0,
	VK_F13,		KEY_F13,	0,
	VK_F14,		KEY_F14,	0,
	VK_F15,		KEY_F15,	0,
	VK_F16,		KEY_F16,	0,
	VK_F17,		KEY_F17,	0,
	VK_F18,		KEY_F18,	0,
	VK_F19,		KEY_F19,	0,
	VK_F20,		KEY_F20,	0,
	/* Manually translate Ctrl-6 into Ctrl-^. */
	'6',		'^'-'@',	LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED,
	0,		0,
};

static int savedChar;
static int saveCount = 0;

static int
decode_key_event(INPUT_RECORD *irp)
{
	int key;
	int i;

	if (!irp->Event.KeyEvent.bKeyDown)
		return -1;

	key = irp->Event.KeyEvent.uChar.AsciiChar;
	if (key != 0)
		return key;

	for (i = 0; keyxlate[i].windows; i++) {
		if (keyxlate[i].windows
		    == irp->Event.KeyEvent.wVirtualKeyCode) {
			if (keyxlate[i].shift != 0
			    && !(keyxlate[i].shift
				 & irp->Event.KeyEvent.dwControlKeyState))
				continue;
			key = keyxlate[i].vile;
			break;
		}
	}
	if (key == 0)
		return -1;

	return key;
}

static void
handle_mouse_event(MOUSE_EVENT_RECORD mer)
{
	int buttondown = FALSE;
	COORD first, current, last;
	int state;

	for_ever {
		switch (mer.dwEventFlags) {
		case 0:
			state = mer.dwButtonState;
			if (state == 0) {
				if (!buttondown)
					return;
				buttondown = FALSE;
				sel_yank(0);
				return;
			}
			if (state & FROM_LEFT_1ST_BUTTON_PRESSED) {
				if (buttondown) {
					if (state & RIGHTMOST_BUTTON_PRESSED) {
						sel_release();
						(void)update(TRUE);
						return;
					}
					break;
				}
				buttondown = TRUE;
				first = mer.dwMousePosition;
				if (!setcursor(first.Y, first.X))
					return;
				(void)sel_begin();
				(void)update(TRUE);
				break;
			}
			break;

		case MOUSE_MOVED:
			if (!buttondown)
				return;
			current = mer.dwMousePosition;
			if (!setcursor(current.Y, current.X))
				break;
			last = current;
			if (!sel_extend(TRUE, TRUE))
				break;
			(void)update(TRUE);
			break;
		}

		for_ever {
			INPUT_RECORD ir;
			DWORD nr;
			int key;

			ReadConsoleInput(hConsoleInput, &ir, 1, &nr);
			switch (ir.EventType) {
			case KEY_EVENT:
				key = decode_key_event(&ir);
				if (key == ESC) {
					sel_release();
					(void)update(TRUE);
					return;
				}
				continue;

			case MOUSE_EVENT:
				mer = ir.Event.MouseEvent;
				break;
			}
			break;
		}
	}
}

int
ntgetch(void)
{
	INPUT_RECORD ir;
	DWORD nr;
	int key;

	if (saveCount > 0) {
		saveCount--;
		return savedChar;
	}

	for_ever {
		ReadConsoleInput(hConsoleInput, &ir, 1, &nr);
		switch(ir.EventType) {

		case KEY_EVENT:
			key = decode_key_event(&ir);
			if (key < 0)
				continue;
			if (ir.Event.KeyEvent.wRepeatCount > 1) {
				saveCount = ir.Event.KeyEvent.wRepeatCount - 1;
				savedChar = key;
			}
			return key;

		case WINDOW_BUFFER_SIZE_EVENT:
			newscreensize(
				ir.Event.WindowBufferSizeEvent.dwSize.Y,
				ir.Event.WindowBufferSizeEvent.dwSize.X
			);
			GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
			continue;

		case MOUSE_EVENT:
			handle_mouse_event(ir.Event.MouseEvent);
			continue;

		}
	}
}

/*
 * The function `kbhit' returns true if there are *any* input records
 * available.  We need to define our own type ahead routine because
 * otherwise events which we will discard (like pressing or releasing
 * the Shift key) can block screen updates because `ntgetch' won't
 * return until a ordinary key event occurs.
 */

int
nttypahead()
{
	INPUT_RECORD ir;
	DWORD nr;
	int key;

	if (saveCount > 0)
		return 1;

	for (;;) {
		PeekConsoleInput(hConsoleInput, &ir, 1, &nr);
		if (nr == 0)
			break;

		switch(ir.EventType) {

		case KEY_EVENT:
			key = decode_key_event(&ir);
			if (key < 0) {
				ReadConsoleInput(hConsoleInput, &ir, 1, &nr);
				continue;
			}
			return 1;

		default:
			/* Ignore type-ahead for non-keyboard events. */
			return 0;
		}
	}

	return 0;
}

/*
 * Move 'n' lines starting at 'from' to 'to'
 *
 * OPT_PRETTIER_SCROLL is prettier but slower -- it scrolls a line at a time
 *	instead of all at once.
 */

/* move howmany lines starting at from to to */
void
ntscroll(int from, int to, int n)
{
	int i;
	SMALL_RECT sRect;
	SMALL_RECT cRect;
	COORD dest;
	CHAR_INFO fill;

	scflush();
	if (to == from)
		return;
#if OPT_PRETTIER_SCROLL
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

	ScrollConsoleScreenBuffer(hConsoleOutput, &sRect, &cRect, dest, &fill);

#if SUPPORT_BROKEN_SCROLLING
	if (backfill_scrolled_areas_manually) {
		DWORD cnt;
		DWORD written;
		COORD coordCursor;

		coordCursor.X = 0;
		if (to > from) {
			coordCursor.Y = from;
			cnt = to - from;
		}
		else {
			coordCursor.Y = to + n;
			cnt = from - to;
		}
		cnt *= csbi.dwMaximumWindowSize.X;
		FillConsoleOutputCharacter(
			hConsoleOutput, ' ', cnt, coordCursor, &written
		);
		FillConsoleOutputAttribute(
			hConsoleOutput, AttrColor(cbcolor, cfcolor), cnt,
			coordCursor, &written
		);
	}
#endif

}
