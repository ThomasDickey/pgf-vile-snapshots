/*
 *	A few functions that used to operate on single whole lines, mostly
 *	here to support the globals() function.  They now work on regions.
 *	Written (except for delins()) for vile by Paul Fox, (c)1990
 *
 * $Log: oneliner.c,v $
 * Revision 1.6  1991/08/07 12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.5
 * date: 1991/08/06 15:24:19;
 *  global/local values
 * 
 * revision 1.4
 * date: 1991/06/27 18:33:47;
 * made screen oriented substitutes always act globally across line
 * 
 * revision 1.3
 * date: 1991/06/25 19:53:07;
 * massive data structure restructure
 * 
 * revision 1.2
 * date: 1991/05/31 11:14:50;
 * turned these into region operators.
 * they're not one-liners anymore
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:51;
 * initial vile RCS revision
 */

#include	"estruct.h"
#include        "edef.h"
#include        <stdio.h>

#define PLIST	0x01

/*
 * put lines in a popup window
 */
pregion(f, n, flag)
{
	register WINDOW *wp;
	register BUFFER *bp;
        register int    s;
        REGION          region;
	static char bname[] = "[p-lines]";
	register LINE *linep;
	int cb;

	fulllineregions = TRUE;
		
        if ((s=getregion(&region,NULL)) != TRUE)
                return (s);

        linep = region.r_orig.l;                 /* Current line.        */
		
	/* first check if we are already here */
	bp = bfind(bname, OK_CREAT, 0);
	if (bp == NULL)
		return FALSE;

	/* bring p-lines up */
	if (popupbuff(bp) != TRUE)
		return FALSE;
		
	if (!calledbefore) {		/* fresh start */
		bclear(bp);
		make_local_b_val(bp,MDLIST);
		set_b_val(bp, MDLIST, ((flag & PLIST) != 0) );
		calledbefore = TRUE;
	}
	
	do {
		addline(bp,linep->l_text,llength(linep));
		linep = lforw(linep);
        } while (linep != region.r_end.l);

	bp->b_flag &= ~BFCHG;
	
	strcpy(bp->b_bname,bname);
	strcpy(bp->b_fname, "");

	make_local_b_val(bp,MDVIEW);
	set_b_val(bp,MDVIEW,TRUE);

	make_local_b_val(bp,VAL_TAB);
	set_b_val(bp,VAL_TAB,tabstop_val(curbp));

	bp->b_active = TRUE;
        for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
                if (wp->w_bufp == bp) {
                        wp->w_flag |= WFMODE|WFFORCE;
                }
        }
	return TRUE;
}

llineregion(f,n)
{
	return pregion(f,n,PLIST);
}

plineregion(f,n)
{
	return pregion(f,n,0);
}

substregion(f,n)
{
	int c, s;
	int foundit;
	static int printit, globally;
	REGION region;

	fulllineregions = TRUE;
		
        if ((s=getregion(&region,NULL)) != TRUE)
                return (s);

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
	} else {
		/* if it's a screen region, assume they want .../g */
		globally = TRUE;
	}
	if (calledbefore == FALSE) {
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
		calledbefore = TRUE;
	}


        DOT.l = region.r_orig.l;            /* Current line.        */

	do {
		MARK tdot;
		foundit = FALSE;
		tdot.l = DOT.l;
		tdot.o = llength(DOT.l);
		setboundry(TRUE, tdot, FORWARD);
		DOT.o = 0;
		do {
#if	MAGIC
			if (magical && b_val(curwp->w_bufp, MDMAGIC))
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
			if (boundry(DOT))
				break;
		} while (globally);
		if (foundit && printit) {
			setmark();
			s = plineregion(FALSE,1);
			if (s != TRUE) return s;
		}
		DOT.l = lforw(DOT.l);
	} while (!sameline(DOT, region.r_end));
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
