/*
 * win31drv.c
 *
 * This is a terminal driver for vile on Windows 3.1 (stub-only 94/6/6).
 *
 * Written by T.E.Dickey for vile, May 1994, with Turbo C/C++ 3.1.
 *
 * Notes:
 *	(1) the corresponding ".DEF" file must have <CR><LF> line endings;
 *	    otherwise TurboC finds a syntax error.
 *	(2) must compile with a LARGE model
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/win31drv.c,v 1.5 1995/11/17 04:03:42 pgf Exp $
 */

#include <windows.h>
#include "estruct.h"

#if	SYS_WIN31

#include "edef.h"

static	HANDLE	hinst ;
static	char	szAppName[] = "WinVile";

#define SCROLL_REG 1

#define NROW	24			/* Screen size.			*/
#define NCOL	80			/* Edit if you want to.		*/

#define MAXNROW	NROW
#define MAXNCOL	NCOL

#define NPAUSE	100			/* # times thru update to pause */
#define MARGIN	8			/* size of minimim margin and	*/
#define SCRSIZ	64			/* scroll size for extended lines */

#if	OPT_COLOR
int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
#endif

static	void	win31_parm P((int));
#if SCROLL_REG
static	void	win31_scrollregion P((int,int));
#endif

static void
csi P((void))
{
	ttputc(ESC);
	ttputc('[');
}

#if	OPT_COLOR
void
win31_fcol(color)		/* set the current output color */
	int	color;	/* color to set */
{
	if (color == cfcolor)
		return;
	csi();
#if	SYS_AMIGA
	win31_parm(coltran[color]+30);
#else
	win31_parm(color+30);
#endif
	ttputc('m');
	cfcolor = color;
}

void
win31_bcol(color)		/* set the current background color */
	int	color;	/* color to set */
{
	if (color == cbcolor)
		return;
	csi();
#if	SYS_AMIGA
	win31_parm(coltran[color]+40);
#else
	win31_parm(color+40);
#endif
	ttputc('m');
	cbcolor = color;
}

void
win31_spal(char *dummy)		/* change palette settings */
{
	/* none for now */
}

#endif

void
win31_move(row, col)
int	row;
int	col;
{
	csi();
	win31_parm(row+1);
	ttputc(';');
	win31_parm(col+1);
	ttputc('H');
}

void
win31_eeol()
{
	csi();
	ttputc('K');
}

void
win31_eeop()
{
#if	OPT_COLOR
	win31_fcol(gfcolor);
	win31_bcol(gbcolor);
#endif
	csi();
	ttputc('2');
	ttputc('J');
}

void
win31_rev(state)		/* change reverse video state */
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

int
win31_cres(flag)	/* change screen resolution */
char	*flag;
{
	return(TRUE);
}

void
win31_beep()
{
	ttputc(BEL);
	ttflush();
}


/* move howmany lines starting at from to to */
void
win31_scroll(from,to,n)
int	from;
int	to;
int	n;
{
	int i;
	if (to == from) return;
#if SCROLL_REG
	if (to < from) {
		win31_scrollregion(to, from + n - 1);
		TTmove(from + n - 1,0);
		for (i = from - to; i > 0; i--)
			ttputc('\n');
	} else { /* from < to */
		win31_scrollregion(from, to + n - 1);
		TTmove(from,0);
		for (i = to - from; i > 0; i--) {
			ttputc(ESC);
			ttputc('M');
		}
	}
	win31_scrollregion(0, term.t_mrow);

#else /* use insert and delete line */
#if OPT_PRETTIER_SCROLL
	if (absol(from-to) > 1) {
		win31_scroll(from, (from<to) ? to-1:to+1, n);
		if (from < to)
			from = to-1;
		else
			from = to+1;
	}
#endif
	if (to < from) {
		win31_move(to,0);
		csi();
		win31_parm(from - to);
		ttputc('M'); /* delete */
		win31_move(to+n,0);
		csi();
		win31_parm(from - to);
		ttputc('L'); /* insert */
	} else {
		win31_move(from+n,0);
		csi();
		win31_parm(to - from);
		ttputc('M'); /* delete */
		win31_move(from,0);
		csi();
		win31_parm(to - from);
		ttputc('L'); /* insert */
	}
#endif
}

#if SCROLL_REG
static void
win31_scrollregion(top,bot)
int	top;
int	bot;
{
	csi();
	win31_parm(top + 1);
	ttputc(';');
	if (bot != term.t_nrow-1) win31_parm(bot + 1);
	ttputc('r');
}
#endif


void
win31_parm(n)
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

void
win31_open()
{
	strcpy(sres, "NORMAL");
	revexist = TRUE;
	ttopen();
}

void
win31_close()
{
#if	OPT_COLOR
	win31_fcol(7);
	win31_bcol(0);
#endif
	/*ttclose();*/
}

void
win31_kopen()	/* open the keyboard (a noop here) */
{
}

void
win31_kclose()	/* close the keyboard (a noop here) */
{
}


/*
 * Process events for the main window
 */
long FAR PASCAL _export WndProc (HWND hwnd, UINT message, UINT wParam,
							  LONG lParam)
{
	switch (message) {
	case WM_CREATE :	/* this happens first... */
 	case WM_SIZE :		/* window is (re)sized */
 	case WM_VSCROLL :
 	case WM_HSCROLL :
 	case WM_DROPFILES :
 	case WM_COMMAND :	/* menu or keyboard command */
 	case WM_PAINT :
        	break ;

	case WM_DESTROY :
		PostQuitMessage (0) ;
                return 0 ;
        }
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

int PASCAL WinMain (HANDLE hInstance, HANDLE hPrevInstance,
		    LPSTR lpszCmdLine, int nCmdShow)
{
	HWND		hwnd ;
	MSG		msg ;
	WNDCLASS	wndclass ;

	/* Allow this to be run only once */
	if (hPrevInstance)
		return 0;
	hinst = hInstance;

	/* Build the transcript window */
	wndclass.style		= CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc	= WndProc ;
	wndclass.cbClsExtra	= 0 ;
	wndclass.cbWndExtra	= 0;
	wndclass.hInstance	= hInstance ;
	wndclass.hIcon		= LoadIcon (hInstance, szAppName) ;
	wndclass.hCursor	= LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground	= GetStockObject (WHITE_BRUSH) ;
	wndclass.lpszMenuName	= szAppName ;
	wndclass.lpszClassName	= szAppName ;

	RegisterClass (&wndclass) ;

	hwnd = CreateWindow (
		szAppName,	/* class name */
		szAppName,	/* window name */
		WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
		CW_USEDEFAULT,	/* position */
		CW_USEDEFAULT,
		CW_USEDEFAULT,	/* size */
		CW_USEDEFAULT,
		NULL,		/* parent window */
		NULL,		/* menu window */
		hInstance,
		lpszCmdLine) ;


	/* Process window (and menu) events */
	ShowWindow (hwnd, nCmdShow) ;
	UpdateWindow (hwnd) ;
	while (GetMessage (&msg, NULL, 0, 0)) {
		TranslateMessage (&msg) ;
		DispatchMessage (&msg ) ;
	}
	return msg.wParam ;
}

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM	term	= {
	MAXNROW-1,	/* max */
	NROW-1,		/* current */
	MAXNCOL,	/* max */
	NCOL,		/* current */
	MARGIN,
	SCRSIZ,
	NPAUSE,
	win31_open,
	win31_close,
	win31_kopen,
	win31_kclose,
	ttgetc,
	ttputc,
	ttflush,
	win31_move,
	win31_eeol,
	win31_eeop,
	win31_beep,
	win31_rev,
	win31_cres
#if	OPT_COLOR
	, win31_fcol
	, win31_bcol
	, win31_spal
#endif
	, win31_scroll
};

#endif	/* SYS_WIN31 */
