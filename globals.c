#include	"estruct.h"
#include        "edef.h"
#include        <stdio.h>

/* ed/vi/ex style global commands, where first the file is scanned for
 *	matching lines, then for each such line, an action is performed.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Log: globals.c,v $
 * Revision 1.11  1992/03/07 10:21:29  pgf
 * arg mismatch on fsearch()
 *
 * Revision 1.10  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.9  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.8  1991/09/10  00:52:55  pgf
 * be careful to not rely on curbp during global ops, since some commands, like
 * print (pregion) change buffers
 *
 * Revision 1.7  1991/08/07  12:35:07  pgf
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
int f,n;
{
	return globber(f,n,'g');
}

vglobals(f,n)
int f,n;
{
#ifdef SOMEDAY
	return globber(f,n,'v');
#else
	return unimpl(f,n);
#endif
}

/* ARGSUSED */
globber(f, n, g_or_v)
int f, n, g_or_v;
{
	int c, s;
	register LINE *lp;
	register char *fnp;	/* ptr to the name of the cmd to exec */
	char *kbd_engl();
	CMDFUNC *engl2fnc();
	CMDFUNC *cfp;
	int foundone;
	WINDOW *wp;
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
	if (readpattern("global pattern: ", &pat[0], &gregexp, c, FALSE) != TRUE) {
		mlforce("[No pattern.]");
		return FALSE;
	}

	/* in some sense, it would be nice to search first, before
                making the user answer the next question, but the
                searching delay is too long, and unexpected in the
                middle of a command.  */

	mlprompt("action to perform on each matching line: ");
	/* and now get the name of, and then the function to execute */
	cfp = NULL;
	fnp = kbd_engl();
	if (!fnp || !fnp[0]) {
	        mlforce("[No function]");
		return FALSE;
	} else if (!(cfp = engl2fnc(fnp))) {
	        mlforce("[No such function]");
		return FALSE;
	} else if ((cfp->c_flags & GLOBOK) == 0) {
	        mlforce("[Function not allowed]");
		return FALSE;
	}
	
	
	/* call the searcher, telling it to do line marking */
	s = fsearch(FALSE,0,TRUE,FALSE);
	if (s != TRUE)
		return s;
	
	calledbefore = FALSE;
	
	/* loop through the buffer -- we must clear the marks no matter what */
	s = TRUE;
	lp = lforw(curbp->b_line.l);
	wp = curwp;
	/* loop until there are no marked lines in the buffer */
	foundone = FALSE;
	for(;;) {
		if (lp == wp->w_bufp->b_line.l) {
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
			/* call the function, if there is one, and results
				have been ok so far */
			if (cfp && s) {
				if (!calledbefore && (cfp->c_flags & UNDO)) {
					if (b_val(wp->w_bufp,MDVIEW))
						return(rdonly());
					mayneedundo();
				}
				havemotion = &f_godotplus;
				wp->w_dot.l = lp;
				wp->w_dot.o = 0;
				s = (cfp->c_func)(FALSE, 1, NULL, NULL);
				/* function may have switched on us */
				swbuffer(wp->w_bufp);
				lp = wp->w_dot.l;
				havemotion = NULL;
				calledbefore = TRUE;
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
