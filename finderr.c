/* Find the next error in mentioned in the shell output window.
 * Written for vile by Paul Fox, (c)1990
 *
 * $Log: finderr.c,v $
 * Revision 1.5  1991/09/20 13:11:53  pgf
 * protect against passing null l_text pointer to sscanf
 *
 * Revision 1.4  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.3
 * date: 1991/08/06 15:21:44;
 * sprintf changes
 * 
 * revision 1.2
 * date: 1991/06/25 19:52:42;
 * massive data structure restructure
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:19;
 * initial vile RCS revision
*/

#include	"estruct.h"
#include        "edef.h"

#if FINDERR

#ifndef NULL
#define NULL 0
#endif

struct LINE *getdot();

/* edits the file and goes to the line pointed at by the next compiler
        error in the "[output]" window.  It unfortunately doesn't mark
        the lines for you, so adding lines to the file throws off the
        later numbering.  Solutions to this seem messy at the moment */

finderr(f,n)
{
	register BUFFER *sbp;
	register LINE *lp;
	register int i = 0;
	register int s = TRUE;
	struct LINE *dotp;
	int moveddot = FALSE;
	
	int errline;
	char errfile[NFILEN];
	
	static int oerrline = -1;
	static char oerrfile[NFILEN];

	/* look up the right buffer */
        if ((sbp=bfind(febuff, NO_CREAT, 0)) == NULL) {
        	mlwrite("No buffer to search for errors.");
                return(FALSE);
        }
        
        if (newfebuff) {
        	oerrline = -1;
        	oerrfile[0] = '\0';
        }
        newfebuff = FALSE;

	dotp = getdot(sbp);

	for(;;) {
		/* to use this line, we need both the filename and the line
			number in the expected places, and a different line
			than last time */
		/* accept lines of the form:
			"file.c", line 223: error....
			or
			file.c: 223: error....
		*/
		if ( dotp->l_text &&
		    (sscanf(dotp->l_text,
			"\"%[^\" 	]\", line %d:",errfile,&errline) == 2 ||
		     sscanf(dotp->l_text,
			"%[^: 	]: %d:",errfile,&errline) == 2 
			) &&
			(oerrline != errline || strcmp(errfile,oerrfile))) {
				break;
		}
			
		if (lforw(dotp) == sbp->b_line.l) {
			mlwrite("No more errors in %s buffer", febuff);
			TTbeep();
			/* start over at the top of file */
			putdotback(sbp, lforw(sbp->b_line.l));
			return FALSE;
		}
		dotp = lforw(dotp);
		moveddot = TRUE;
	}
	/* put the new dot back, before possible changes to contents
				of current window from getfile() */
	if (moveddot)
		putdotback(sbp,dotp);

	if (strcmp(errfile,curbp->b_fname)) { /* if we must change windows */
		struct WINDOW *wp;
		wp = wheadp;
		while (wp != NULL) {
			if (!strcmp(wp->w_bufp->b_fname,errfile))
				break;
			wp = wp->w_wndp;
		}
		if (wp) {
			curwp = wp;
			make_current(curwp->w_bufp);
			upmode();
		} else {
			s = getfile(errfile,TRUE);
			if (s != TRUE)
				return s;
		}
	}

	mlwrite("Error is %*S", dotp->l_used, dotp->l_text);

	/* it's an absolute move */
	curwp->w_lastdot = curwp->w_dot;
	gotoline(TRUE,errline);

	oerrline = errline;
	strcpy(oerrfile,errfile);

	return TRUE;

}

struct LINE *
getdot(bp)
struct BUFFER *bp;
{
	register WINDOW *wp;
	if (bp->b_nwnd) {
		/* scan for windows holding that buffer, 
					pull dot from the first */
	        wp = wheadp;
	        while (wp != NULL) {
	                if (wp->w_bufp == bp) {
	                        return wp->w_dot.l;
			}
	                wp = wp->w_wndp;
	        }
	}
        return bp->b_dot.l;
}

putdotback(bp,dotp)
struct BUFFER *bp;
struct LINE *dotp;
{
	register WINDOW *wp;

	if (bp->b_nwnd) {
	        wp = wheadp;
	        while (wp != NULL) {
	                if (wp->w_bufp == bp) {
		                wp->w_dot.l = dotp;
		                wp->w_dot.o = 0;
			        wp->w_flag |= WFMOVE;
			}
	                wp = wp->w_wndp;
	        }
		return;
	}
	/* then the buffer isn't displayed */
        bp->b_dot.l = dotp;
        bp->b_dot.o = 0;
}

#else
finderrhello() { }
#endif
