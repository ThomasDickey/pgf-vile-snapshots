#include	"estruct.h"
#include        "edef.h"
#include        <stdio.h>

/*
	a few functions that operate on single whole lines, mostly
	here to support the globals() function
	These could be turned into operators, but they're either not
	worth it, or a bit tricky
	written (except for delins()) for vile by Paul Fox, (c)1990
*/

#define PLIST	0x01

/*
 * put lines in a popup window
 */
pline(f, n, flag)
{
	register WINDOW *wp;
	register BUFFER *bp;
        register int    s;
	static char bname[] = "[p-lines]";
	register LINE *lp;
	int cb;
		
	lp = curwp->w_dotp;
		
	/* first check if we are already here */
	bp = bfind(bname, OK_CREAT, 0);
	if (bp == NULL)
		return FALSE;

	/* bring p-lines up */
	if (popupbuff(bp) != TRUE)
		return FALSE;
		
	if (!plinesdone) {		/* fresh start */
		bclear(bp);
		if (flag & PLIST)
			bp->b_mode |= MDLIST;
		else
			bp->b_mode &= ~MDLIST;
		plinesdone = TRUE;
	}
	
	addline(bp,lp->l_text,llength(lp));
	bp->b_flag &= ~BFCHG;
	
	strcpy(bp->b_bname,bname);
	strcpy(bp->b_fname, "");
	bp->b_mode |= MDVIEW;
	bp->b_active = TRUE;
        for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
                if (wp->w_bufp == bp) {
                        wp->w_flag |= WFMODE|WFFORCE;
                }
        }
	return TRUE;
}

lline(f,n,flag)
{
	return pline(f,n,flag|PLIST);
}

subst(f,n,dummy,gotresponse)
{
	int c, s;
	int foundit;
	static int printit, globally;
	
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
	if (!gotresponse) {
		if ((s = readpattern("substitute pattern: ", &pat[0], TRUE, c,
				FALSE)) != TRUE) {
			if (s != ABORT)
				mlwrite("No pattern.");
			return FALSE;
		}
		if ((s = readpattern("replacement string: ", &rpat[0], FALSE, c,
				FALSE)) != TRUE) {
			if (s == ABORT)
				return FALSE;
			/* else the pattern is null, which is okay... */
		}
		if (lastkey == c) {/* the user may have something to add */
			char buf[3];
			char *bp = buf;
			buf[0] = 0;
			mlreply("(g)lobally on line and/or (p)rint result: ",
				buf, sizeof buf);
			printit = globally = FALSE;
			while (*bp) {
				if (*bp == 'p' && !printit)
					printit = TRUE;
				else if (*bp == 'g' && !globally)
					globally = TRUE;
				else if (!isspace(*bp)) {
					mlwrite("Unknown action %s",buf);
					return FALSE;
				}
				bp++;
			}
		}
	}

	foundit = FALSE;
	setboundry(TRUE, curwp->w_dotp, llength(curwp->w_dotp), FORWARD);
	curwp->w_doto = 0;
	do {
#if	MAGIC
		if (magical && (curwp->w_bufp->b_mode & MDMAGIC) != 0)
			s = thescanner(&mcpat[0], FORWARD, PTBEG, TRUE);
		else
#endif
			s = thescanner(&pat[0], FORWARD, PTBEG, TRUE);
		if (s != TRUE)
			break;
			
		/* found the pattern */
		foundit = TRUE;
		s = delins(matchlen, &rpat[0]);
		if (s != TRUE)
			return s;
		if (boundry(curwp->w_dotp, curwp->w_doto))
			break;
	} while (globally);
	if (foundit && printit) {
		s = pline(FALSE,1,NULL);
		if (s != TRUE) return s;
	}
	return TRUE;
}

/*
 * delins -- Delete a specified length from the current
 *	point, then insert the string.
 *  borrowed from original replaces() code 
 */
delins(dlength, instr)
int	dlength;
char	*instr;
{
	int	status;
	char	tmpc;

	/* Zap what we gotta,
	 * and insert its replacement.
	 */
	if (!(status = ldelete((long) dlength, FALSE))) {
		mlwrite("Error while deleting");
		return FALSE;
	} else {
		while (tmpc = *instr) {
			status = (tmpc == '\n'? lnewline(): linsert(1, tmpc));
			if (!status) {
				mlwrite("Out of memory while inserting");
				break;
			}
			instr++;
		}
	}
	return status;
}
