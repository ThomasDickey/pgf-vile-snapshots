#include	"estruct.h"
#include        "edef.h"
#include        <stdio.h>

/* ed/vi/ex style global commands, where first the file is scanned for
	matching lines, then for each such line, an action is performed.
	written for vile by Paul Fox, (c)1990
*/
#if GLOBALS

globals(f,n)
{
	static char buf[NFILEN] = '\0';
	int c, s, calledbefore;
	register LINE *lp;
	register char *fnp;	/* ptr to the name of the cmd to exec */
	char *kbd_engl();
	CMDFUNC *engl2fnc();
	CMDFUNC *cfp;
	int foundone;
	extern CMDFUNC f_stutterfunc;
	
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
	s = forwsearch(FALSE,0,TRUE,NULL);
	if (s != TRUE)
		return s;
	
	calledbefore = FALSE;
	
	/* loop through the buffer -- we must clear the marks no matter what */
	s = TRUE;
	lp = lforw(curbp->b_linep);
	/* loop until there are no marked lines in the buffer */
	foundone = FALSE;
	for(;;) {
		if (lp == curbp->b_linep) {
			/* at the end -- only quit if we found no 
				marks on the last pass through. otherwise,
				go through again */
			if (foundone)
				foundone = FALSE;
			else
				break;
		}
		if (lismarked(lp)) {
			foundone = TRUE;
			lsetnotmarked(lp);
			curwp->w_dotp = lp;
			curwp->w_doto = 0;
			/* call the function, if there is one, and results
				have been ok so far */
			if (cfp && s) {
				if (!calledbefore && (cfp->c_flags & UNDO)) {
					if (curbp->b_mode&MDVIEW)
						return(rdonly());
					mayneedundo();
				}
				havemotion = &f_stutterfunc;
				s = (cfp->c_func)(FALSE, 1, NULL, calledbefore);
				calledbefore = TRUE;
				havemotion = NULL;
			}
			if (lp != curwp->w_dotp) {
				/* make sure we're still in the buffer, since
					action might have caused delete, etc. */
				lp = curwp->w_dotp;
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
