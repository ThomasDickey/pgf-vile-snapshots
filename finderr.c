/* Find the next error in mentioned in the shell output window.
 * Written for vile by Paul Fox, (c)1990
 *
 * $Log: finderr.c,v $
 * Revision 1.16  1993/04/01 13:07:50  pgf
 * see tom's 3.40 CHANGES
 *
 * Revision 1.15  1993/03/05  17:50:54  pgf
 * see CHANGES, 3.35 section
 *
 * Revision 1.14  1993/01/16  10:34:20  foxharp
 * use for_each_window macro
 *
 * Revision 1.13  1992/12/04  09:12:25  foxharp
 * deleted unused assigns
 *
 * Revision 1.12  1992/08/19  22:57:45  foxharp
 * no longer need to multiply NFILEN -- it's bigger
 *
 * Revision 1.11  1992/07/16  22:06:58  foxharp
 * honor the "Entering/Leaving directory" messages that GNU make puts out
 *
 * Revision 1.10  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.9  1992/03/19  23:35:27  pgf
 * use b_linecount to index from end of file instead of top, to be
 * less affected by insertions deletions up above
 *
 * Revision 1.8  1992/03/05  09:01:12  pgf
 * shortened "found error" msg, and force it
 *
 * Revision 1.7  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.6  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.5  1991/09/20  13:11:53  pgf
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

/* ARGSUSED */
int
finderr(f,n)
int f,n;
{
	register BUFFER *sbp;
	register int s;
	struct LINE *dotp;
	int moveddot = FALSE;
	char	*text;
	
	int errline;
	char errfile[NFILEN];
	char ferrfile[NFILEN];
	
	static int oerrline = -1;
	static char oerrfile[256];
	static int l = 0;
#define DIRLEVELS 20
	static char *dirs[DIRLEVELS];

	/* look up the right buffer */
        if ((sbp=bfind(febuff, NO_CREAT, 0)) == NULL) {
        	mlforce("[No buffer to search for errors.]");
                return(FALSE);
        }
        
        if (newfebuff) {
        	oerrline = -1;
        	oerrfile[0] = '\0';
		l = 0;
		dirs[0] = NULL;
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
			or
			******** Line 187 of "filec.c"
		*/
		if (lisreal(dotp)) {
			static	TBUFF	*tmp;
			char verb[20];
#if SUNOS
			char *t;
#endif

			if (tb_init(&tmp, EOS) != 0
			 && tb_bappend(&tmp, dotp->l_text, (unsigned)llength(dotp)) != 0
			 && tb_append(&tmp, EOS) != 0)
			 	text = tb_values(tmp);
			else
				break;	/* out of memory */

			if ( (sscanf(text,
				"\"%[^\" \t]\", line %d:",
				errfile, &errline) == 2
			  ||  sscanf(text,
				"%[^: \t]: %d:",
				errfile, &errline) == 2 
#if SUNOS			/* lint-output */
			  ||  sscanf(text,
			  	"%[^:( \t](%d):",
				errfile, &errline) == 2
			  ||  ((t = strstr(text, "  ::  ")) != 0
			    && sscanf(t,
			  	"  ::  %[^( \t](%d)",
				errfile, &errline) == 2)
#endif
#if APOLLO
			  ||  sscanf(text,
				"%20[*] Line %d of \"%[^\" \t]\"",
				verb, &errline, errfile) == 3 
#endif
				) && (oerrline != errline || 
					strcmp(errfile,oerrfile))) {
				break;
			} else if (sscanf(text,
			"%*[^ \t:`]: %20[^ \t`] directory `",verb) == 1 ) {
				char *d = strchr(text, '`') + 1;
				if (verb[0] == 'E') { /* Entering */
					if (l < DIRLEVELS)
						dirs[++l] = d;
				} else if (verb[0] == 'L'){ /* Leaving */
					if (l > 0)
						--l;
				}
			}
		}
			
		if (lforw(dotp) == sbp->b_line.l) {
			mlforce("[No more errors in %s buffer]", febuff);
			TTbeep();
			/* start over at the top of file */
			putdotback(sbp, lforw(sbp->b_line.l));
			l = 0;
			dirs[0] = NULL;
			return FALSE;
		}
		dotp = lforw(dotp);
		moveddot = TRUE;
	}
	/* put the new dot back, before possible changes to contents
				of current window from getfile() */
	if (moveddot)
		putdotback(sbp,dotp);

	if (dirs[l]) {
		int i = 0;
		while(dirs[l][i] != '\'') {
			ferrfile[i] = dirs[l][i];
			i++;
		}
		ferrfile[i++] = '/';
		(void)strcpy(&ferrfile[i],errfile);
	} else {
		/* lsprintf(ferrfile,"%s/%s",current_directory(FALSE),errfile); */
		(void)strcpy(ferrfile,errfile);
	}
	if (strcmp(ferrfile,curbp->b_bname) &&
		strcmp(ferrfile,curbp->b_fname)) {
		/* if we must change windows */
		struct WINDOW *wp;
		for_each_window(wp) {
			if (!strcmp(wp->w_bufp->b_bname,ferrfile) ||
				!strcmp(wp->w_bufp->b_fname,ferrfile))
				break;
		}
		if (wp) {
			curwp = wp;
			make_current(curwp->w_bufp);
			upmode();
		} else {
			s = getfile(ferrfile,TRUE);
			if (s != TRUE)
				return s;
		}
	}

	mlforce("Error: %*S", dotp->l_used, dotp->l_text);

	/* it's an absolute move */
	curwp->w_lastdot = curwp->w_dot;
	gotoline(TRUE, -(curbp->b_linecount - errline + 1));

	oerrline = errline;
	(void)strcpy(oerrfile,errfile);

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
	        for_each_window(wp) {
	                if (wp->w_bufp == bp) {
	                        return wp->w_dot.l;
			}
	        }
	}
        return bp->b_dot.l;
}

void
putdotback(bp,dotp)
struct BUFFER *bp;
struct LINE *dotp;
{
	register WINDOW *wp;

	if (bp->b_nwnd) {
	        for_each_window(wp) {
	                if (wp->w_bufp == bp) {
		                wp->w_dot.l = dotp;
		                wp->w_dot.o = 0;
			        wp->w_flag |= WFMOVE;
			}
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
