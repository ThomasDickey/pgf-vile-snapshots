/*
 * win31tbl.c
 *
 * This is a simple wrapper for vile's 'mktbls'
 * program, which simplifies development within an IDE.  It allows the
 * user to select the name of a table-file (e.g., "cmdtbl" or "modetbl"),
 * and then invoke 'mktbls' to compile the table.  The table is written
 * into the directory in which the table-file resides.
 *
 * Written by T.E.Dickey for vile, May 1994, with Turbo C/C++ 3.1.
 *
 * Notes:
 *	(1) the corresponding ".DEF" file must have <CR><LF> line endings;
 *	    otherwise TurboC finds a syntax error.
 *	(2) error messages from 'mktbls' are redirected into a temporary
 *	    file, which is scanned each time the transcript is repainted.
 *	    This process is faster than it might seem...
 *	(3) Compile with NO_LEAKS=1, main=MakeTables defined, with a COMPACT
 *	    memory model.
 *
 * TODO:
 *	test the program by repeatedly running with AUDIT.
 *
 *	try running with multiple instances
 *
 *	add a Font option (so the user can see the transcript in a fixed font).
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/win31tbl.c,v 1.2 1994/07/11 22:56:20 pgf Exp $
 */
#include <windows.h>
#include <commdlg.h>	/* Common Dialogs */
#include <cderr.h>	/* for "CDERR_xxx" definitions	*/
#include <shellapi.h>	/* for drag/drop */
#include <dir.h>	/* for MAXPATH (note: MAXPATH > _MAX_PATH) */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "win31tbl.h"

#define EOS   '\0'
#define SLASH '\\'

/*
 * mktbls's "main" program is redefined to "MakeTables" (to avoid confusion).
 */
int MakeTables (int, char **);

/*
 * Public data (non-static) is stored in a per-instance data segment
 */
HANDLE	hinst ;
short	cxChar, cyChar, nMaxWidth, nMaxHeight ;
char	szDirName[MAXPATH] ;
char	szFileName[MAXPATH] ;
char	szTempName[MAXPATH] ;

/*
 */
static char szAppName[] = "MakeTables";

/*
 * Obtain the current size of the client area
 */
static void PASCAL getWinSize (HWND hwnd, WORD *wHigh, WORD *wWide)
{
	RECT	rc ;

	GetClientRect (hwnd, &rc) ;
	*wWide = rc.right - rc.left ;
	*wHigh = rc.bottom - rc.top ;
}

/*
 * (Re)compute scrollbar limits and visibility when the window-size changes,
 * or we have recomputed its contents.
 */
static void PASCAL adjustScrollBars (HWND hwnd)
{
	WORD	wHigh, wWide ;
        int	nMin = 0, nMax, nPos ;

	getWinSize (hwnd, &wHigh, &wWide) ;

        nPos = GetScrollPos (hwnd, SB_VERT) ;
	nMax = max(nMin, nMaxHeight) ;
	nPos = min(nPos, nMax) ;
	if ((nMax - nMin) * cyChar < wHigh)
		nMax = nMin ;
	if (nMax > nMin)
        	nMax--;

	SetScrollRange (hwnd, SB_VERT, nMin, nMax, FALSE) ;
	SetScrollPos   (hwnd, SB_VERT, nPos, TRUE) ;

	nPos = GetScrollPos (hwnd, SB_HORZ) ;
	nMax = max(nMin, nMaxWidth) ;
	nPos = min(nPos, nMax) ;
	if ((nMax - nMin) * cxChar < wWide)
        	nMax = nMin ;
	if (nMax > nMin)
        	nMax--;

	SetScrollRange (hwnd, SB_HORZ, nMin, nMax, FALSE) ;
	SetScrollPos   (hwnd, SB_HORZ, nPos, TRUE) ;
}

/*
 * Handle events for the given scrollbar
 */
static void PASCAL handleScrollBar
	(HWND hwnd, int inBar, UINT event, int nTrack, int nLimit)
{
	static UINT lastScroll = SB_ENDSCROLL ;
	int	increment ;
	int	position = GetScrollPos (hwnd, inBar) ;
        int	nClient, nChar ;
	WORD	wHigh, wWide ;
        int	nMin, nMax ;

	getWinSize (hwnd, &wHigh, &wWide) ;
	if (inBar == SB_HORZ) {
		nClient = wWide ;
                nChar   = cxChar ;
	} else {
		nClient = wHigh ;
                nChar   = cyChar ;
        }

        GetScrollRange (hwnd, inBar, &nMin, &nMax) ;

	switch (event) {
	case SB_TOP :
		increment = - position ;
		break ;
	case SB_BOTTOM :
		increment = nMax - position ;
		break ;
	case SB_LINEUP :
		increment = -1 ;
		break ;
	case SB_LINEDOWN :
		increment = 1 ;
		break ;
	case SB_PAGEUP :
		increment = - max(1, nClient / nChar) ;
		break ;
	case SB_PAGEDOWN :
		increment = max(1, nClient / nChar) ;
		break ;
	case SB_THUMBTRACK :
		increment = nTrack - position ;
		break ;
	default :
		return ;
	}

	if (increment + position < 0)
		increment = - position ;
	if (increment + position >= nLimit)
		increment = nLimit - position - 1 ;

	if (increment != 0) {
		position += increment ;
		increment *= nChar ;
		ScrollWindow (hwnd,
			(inBar == SB_HORZ) ? -increment : 0,
			(inBar == SB_VERT) ? -increment : 0,
			NULL, NULL) ;
		SetScrollPos (hwnd, inBar, position, TRUE) ;
		UpdateWindow (hwnd) ;
		lastScroll = SB_ENDSCROLL ;
	} else if (event != lastScroll
	 && SB_ENDSCROLL != lastScroll) {
		MessageBeep (hwnd) ;
		lastScroll = event ;
	}
}

/*
 * This _should_ have been part of the common dialogs
 */
static void ErrorHandler (HWND hwnd)
{
	DWORD	err = CommDlgExtendedError();
	char	buffer[BUFSIZ];

	if (err == 0)
		return;

	if (err > CDERR_DIALOGFAILURE
	 || LoadString(hinst, (WORD)err, buffer, sizeof(buffer)) == 0)
        	sprintf(buffer, "Unknown error code: %#lx", err);
	MessageBox (hwnd, buffer, NULL, MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK) ;
}

/*
 * Set the window's title (aka caption).
 */
static void makeCaption (HWND hwnd, char *pathname)
{
	char	buffer[MAXPATH + 5 + sizeof(szAppName)] ;
	sprintf(buffer, "%s [%s]", szAppName, pathname) ;
        SetWindowText(hwnd, buffer) ;
}

/*
 * Set the full pathname and the short one (used in the caption)
 */
static void setFileNames (HWND hwnd, LPSTR path)
{
	if (lstrlen(path)) {
		char	szFileTitle[MAXPATH] ;
		char	szFullPath[MAXPATH] ;
		LPSTR	s ;
                int	len ;

		lstrcpy (szFileName,
			_fullpath(szFullPath, path, sizeof(szFullPath))) ;
		s = lstrcpy (szDirName, szFileName) ;
		for ( len = lstrlen (s) - 1 ; len > 0; len--) {
			if (s[len] == '\\') {
				s[len] = EOS ;
				break ;
                        }
		}
		chdir (szDirName) ;
		GetFileTitle (szFileName, szFileTitle, sizeof(szFileTitle)) ;
		makeCaption (hwnd, szFileTitle) ;
        }
}

/*
 * Prompt for a filename.
 */
static void getFilename (HWND hwnd)
{
	OPENFILENAME	ofn ;
	char	szFilter[256] ;
	char	szFileTitle[MAXPATH] ;
	UINT	i, cbString ;
        char	chReplace ;

	szFileName[0] = EOS;
	lstrcpy(szDirName, ".");

	if ((cbString = LoadString(hinst, IDS_FILTERSTRING,
		szFilter, sizeof(szFilter))) == 0) {
		ErrorHandler (hwnd) ;
		return ;
	}
	chReplace = szFilter[cbString - 1] ;
	for (i = 0; szFilter[i] != EOS; i++) {
		if (szFilter[i] == chReplace)
			szFilter[i] = EOS;
	}

	/* Set unused members to zero */
	memset (&ofn, 0, sizeof(ofn)) ;

	ofn.lStructSize     = sizeof(ofn) ;
	ofn.hwndOwner       = hwnd ;
	ofn.lpstrFilter     = szFilter ;
	ofn.nFilterIndex    = 1 ;
	ofn.lpstrFile       = szFileName ;
	ofn.nMaxFile        = sizeof(szFileName) ;
	ofn.lpstrFileTitle  = szFileTitle ;
	ofn.nMaxFileTitle   = sizeof(szFileTitle) ;
	ofn.lpstrInitialDir = szDirName ;
	ofn.Flags           = OFN_SHOWHELP
			    | OFN_PATHMUSTEXIST
			    | OFN_FILEMUSTEXIST
			    | OFN_HIDEREADONLY ;

	/*
	 * If the user selects a filename, set the window title with the
	 * leaf-name so that when iconified it is clear what file is loaded.
         */
	if (GetOpenFileName(&ofn)) {
        	/* The dialog changed working directories to match the file */
		makeCaption (hwnd, szFileTitle) ;
	} else
		ErrorHandler (hwnd) ;
}

/*
 * Find out how many files were dropped on this program. We can handle only
 * one.
 */
static void setDragFile (HWND hwnd, HANDLE hDrag)
{
	WORD	cFiles ;
        char	szDragName[MAXPATH] ;

	cFiles = DragQueryFile (hDrag, 0xffff, (LPSTR)NULL, 0);
	if (cFiles > 1) {
		MessageBox (hwnd,
			"This application can handle only one file at a time",
			NULL,
			MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK) ;
	} else {
		DragQueryFile (hDrag, 0, szDragName, sizeof(szDragName)) ;
                setFileNames (hwnd, szDragName) ;
	}
        DragFinish (hDrag) ;
}

/*
 * Compile the given vile table-file.
 * Intercept the messages produced by 'mktbls' by redirecting stderr.
 */
static void compileTable (HWND hwnd)
{
	static	char	*arglist[3] = { szAppName, szFileName, 0 } ;
	int	ok ;
	char	buffer[MAXPATH] ;
        HCURSOR hCurSave ;

	hCurSave = SetCursor (LoadCursor (NULL, IDC_WAIT)) ;
	ShowCursor (TRUE) ;

	freopen (szTempName, "w", stderr);
	fprintf(stderr, "Creating tables with %s\n", arglist[1]) ;
        fprintf(stderr, "Output directory is  %s\n",
		getcwd(buffer, sizeof(buffer))) ;
	ok = MakeTables (2, arglist);
	if (ok == EXIT_SUCCESS)
		fprintf(stderr, "...done\n");
	fclose (stderr);

	ShowCursor (FALSE) ;
        SetCursor (hCurSave) ;

	InvalidateRect (hwnd, NULL, TRUE) ; /* force repaint */

	if (ok != EXIT_SUCCESS) {
		MessageBox (hwnd, "COMPILE error!",
				szAppName, MB_ICONEXCLAMATION | MB_OK) ;
        }
}

/*
 * Process events for the main window
 */
long FAR PASCAL _export WndProc (HWND hwnd, UINT message, UINT wParam,
							  LONG lParam)
{
	short	x, y ;
	char	buffer[BUFSIZ] ;
	FILE		*inp ;
	HDC		hdc ;
	PAINTSTRUCT	ps ;
	TEXTMETRIC	tm ;

	switch (message) {
	case WM_CREATE :	/* this happens first... */
		/*
		 * Get average character sizes; we'll use them in computing
                 * scrollbar limits and positions.
                 */
		hdc = GetDC (hwnd) ;

		GetTextMetrics (hdc, &tm) ;
		cxChar = tm.tmAveCharWidth ;
		cyChar = tm.tmHeight + tm.tmExternalLeading ;

		ReleaseDC (hwnd, hdc) ;

		nMaxWidth = 0 ;

		/*
                 * Allow this application to accept drag/drop files.
                 */
		DragAcceptFiles (hwnd, TRUE) ;

		/*
		 * Retrieve the filename specified on the command-line (if
		 * any).
		 */
		setFileNames (hwnd,
			(LPSTR) (((LPCREATESTRUCT) lParam)->lpCreateParams)) ;

		return 0 ;

	case WM_SIZE :		/* window is (re)sized */
		adjustScrollBars (hwnd) ;
		return 0 ;

	case WM_VSCROLL :
		handleScrollBar (hwnd, SB_VERT, wParam, LOWORD(lParam), nMaxHeight) ;
		return 0 ;

	case WM_HSCROLL :
		handleScrollBar (hwnd, SB_HORZ, wParam, LOWORD(lParam), nMaxWidth) ;
		return 0 ;

	case WM_DROPFILES :
		setDragFile (hwnd, (HANDLE)wParam) ;
		return 0 ;

	case WM_COMMAND :	/* menu or keyboard command */
		switch (wParam) {
		case IDM_OPEN :
                	getFilename (hwnd) ;
			return 0;

		case IDM_COMPILE :
                	compileTable (hwnd) ;
			return 0;

		case IDM_EXIT :
			SendMessage (hwnd, WM_CLOSE, 0, 0L);
			return 0;

		case IDM_HELP :
			MessageBox (hwnd, "Help not yet implemented!",
				szAppName, MB_ICONEXCLAMATION | MB_OK) ;
			return 0;

		case IDM_ABOUT :
			MessageBox (hwnd, "Make Tables for VILE",
				szAppName, MB_ICONINFORMATION | MB_OK) ;
                        return 0 ;
                }
		break ;

	case WM_PAINT :
		hdc = BeginPaint (hwnd, &ps) ;
		if (IsIconic (hwnd)) {
			;
		} else if ((inp = fopen(szTempName, "r")) != 0) {
			int	lines = 0 ;
                        int	nxPos = GetScrollPos(hwnd, SB_HORZ) ;
			int	nyPos = GetScrollPos(hwnd, SB_VERT) ;
			int	nxChr, nyChr ;

			nMaxWidth = 0 ;

			while (fgets(buffer, sizeof(buffer), inp) != 0) {
				int len = lstrlen(buffer) - 1;
				DWORD extent ;

				x = cxChar * (1 - nxPos) ;
				y = cyChar * (lines++ - nyPos) ;

				buffer[len] = EOS ;

				extent = GetTabbedTextExtent (hdc,
					buffer, len,
					0, NULL) ;
				nxChr = LOWORD(extent) ;
				nyChr = HIWORD(extent) ;

				if (y >= ps.rcPaint.top - nyChr
				 && y <  ps.rcPaint.bottom
				 && x >= ps.rcPaint.left - nxChr
				 && x <= ps.rcPaint.right) {
					TabbedTextOut(hdc,
						x, y,
						buffer, len,
						0, NULL,
						x) ;
				}
                                len = nxChr / cxChar ;
				/* nominal length, with tabs */
				nMaxWidth = max(len, nMaxWidth) ;
			}
			fclose (inp) ;
			nMaxHeight = lines ;
			adjustScrollBars (hwnd) ;
		}
                EndPaint (hwnd, &ps) ;
		return 0 ;

	case WM_DESTROY :
		DragAcceptFiles (hwnd, FALSE) ;
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

	/* Allow this to be run only once, just to avoid confusion */
	if (hPrevInstance)
		return 0;
	hinst = hInstance;

	(void) _fullpath (szTempName,
		(LPSTR) tmpnam (szFileName), sizeof(szTempName)) ;
	getcwd (szDirName, sizeof(szDirName)) ;
	szFileName[0] = EOS ;

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
	(void) unlink (szTempName) ;
	return msg.wParam ;
}

#ifdef TEST
int	MakeTables (int argc, char **argv)
{
	int	n;

	fprintf(stderr, "Running %s (test-only)\n", szAppName);
	for (n = 0; n < argc; n++)
		fprintf(stderr, "\targv[%d] = \"%s\"\n", n, argv[n]);
        return EXIT_FAILURE;
}
#endif
