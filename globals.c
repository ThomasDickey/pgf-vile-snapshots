#include	"estruct.h"
#include        "edef.h"
#include        <stdio.h>

/* ed/vi/ex style global commands, where first the file is scanned for
 *	matching lines, then for each such line, an action is performed.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Log: globals.c,v $
 * Revision 1.7  1991/08/07 12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.6
 * date: 1991/08/06 15:22:03;
 *  global/local values
 * 
 * revision 1.5
 * date: 1991/06/27 18:43:40;
 * fixed infinite loop if a global referenced '^' or '$' as the search string
 * 
 * revision 1.4
 * date: 1991/06/25 19:52:44;
 * massive data structure restructure
 * 
 * revision 1.3
 * date: 1991/05/31 11:07:32;
 * clean up globals routine, so it doesn't need or provide extra args
 * 
 * revision 1.2
 * date: 1991/04/22 09:02:42;
 * removed non-portable initialization
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:21;
 * initial vile RCS revision
*/
#if GLOBALS

globals(f,n)
{
	return globber(f,n,'g');
}

vglobals(f,n)
{
#ifdef SOMEDAY
	return globber(f,n,'v');
#else
	return unimpl();
#endif
}

globber(f, n, g_or_v)
{
	static char buf[NFILEN];
	int c, s;
	register LINE *lp;
	register char *fnp;	/* ptr to the name of the cmd to exec */
	char *kbd_engl();
	CMDFUNC *engl2fnc();
	CMDFUNC *cfp;
	int foundone;
	extern CMDFUNC f_godotplus;
	
	c = '\n';
	if (isnamedcmd) {
		c = tpeekc();
		if (c < 0) {
			c = '\n';
		} else {
			if (ispunct(c)) {
				(void)kbd_key();
			}
		}
	}
	if (readpattern("global pattern: ", &pat[0], TRUE, c, FALSE) != TRUE) {
		mlwrite("No pattern.");
		return FALSE;
	}

	/* in some sense, it would be nice to search first, before
                making the user answer the next question, but the
                searching delay is too long, and unexpected in the
                middle of a command.  */

	mlwrite("action to perform on each matching line: ");
	/* and now get the name of, and then the function to execute */
	cfp = NULL;
	fnp = kbd_engl();
	if (!fnp || !fnp[0]) {
	        mlwrite("[No function]");
		return FALSE;
	} else if (!(cfp = engl2fnc(fnp))) {
	        mlwrite("[No such function]");
		return FALSE;
	} else if ((cfp->c_flags & GLOBOK) == 0) {
	        mlwrite("[Function not allowed]");
		return FALSE;
	}
	
	
	/* call the searcher, telling it to do line marking */
	s = fsearch(FALSE,0,TRUE,NULL);
	if (s != TRUE)
		return s;
	
	calledbefore = FALSE;
	
	/* loop through the buffer -- we must clear the marks no matter what */
	s = TRUE;
	lp = lforw(curbp->b_line.l);
	/* loop until there are no marked lines in the buffer */
	foundone = FALSE;
	for(;;) {
		if (lp == curbp->b_line.l) {
			/* at the end -- only quit if we found no 
				marks on the last pass through. otherwise,
				go through again */
			if (foundone)
				foundone = FALSE;
			else
				break;
			lsetnotmarked(lp); /* always unmark the header line */
		}
		if (lismarked(lp)) {
			foundone = TRUE;
			lsetnotmarked(lp);
			curwp->w_dot.l = lp;
			curwp->w_dot.o = 0;
			/* call the function, if there is one, and results
				have been ok so far */
			if (cfp && s) {
				if (!calledbefore && (cfp->c_flags & UNDO)) {
					if (b_val(curbp,MDVIEW))
						return(rdonly());
					mayneedundo();
				}
				havemotion = &f_godotplus;
				s = (cfp->c_func)(FALSE, 1, NULL, NULL);
				havemotion = NULL;
				calledbefore = TRUE;
			}
			if (lp != curwp->w_dot.l) {
				/* make sure we're still in the buffer, since
					action might have caused delete, etc. */
				lp = curwp->w_dot.l;
			}
		}
		lp = lforw(lp);
	}

	cfp = NULL;

	return s;
}

#else
globalhello() { }
#endif
