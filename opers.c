/*
 * This file contains the command processing functions for the commands
 * that take motion operators.
 * written for vile by Paul Fox, (c)1990
 *
 * $Log: opers.c,v $
 * Revision 1.19  1992/01/06 23:09:33  pgf
 * error message if bad function specified for motion
 *
 * Revision 1.18  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.17  1992/01/03  23:26:48  pgf
 * make the "pre-operator" position, pre_op_dot, a global so that it can
 * be adjust slightly by some motions -- paragraph motions in particular
 *
 * Revision 1.16  1991/11/03  17:46:30  pgf
 * removed f,n args from all region functions -- they don't use them,
 * since they're no longer directly called by the user
 *
 * Revision 1.15  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.14  1991/10/29  14:35:29  pgf
 * implemented the & commands: substagain
 *
 * Revision 1.13  1991/09/10  00:51:05  pgf
 * only restore dot with swapmark if the buffer hasn't switched on us
 *
 * Revision 1.12  1991/08/13  02:50:52  pgf
 * fixed chgreg case of butting against top of buffer
 *
 * Revision 1.11  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.10
 * date: 1991/07/19 17:23:06;
 * added status return to chgreg()
 * 
 * revision 1.9
 * date: 1991/07/19 17:16:41;
 * fix null pointer de-ref
 * 
 * revision 1.8
 * date: 1991/06/25 19:53:04;
 * massive data structure restructure
 * 
 * revision 1.7
 * date: 1991/06/16 17:36:47;
 * added entab, detab, and trim operator routines
 * 
 * revision 1.6
 * date: 1991/06/15 09:11:23;
 * hardened chgreg[ion] against motions that don't succeed, leaving
 * the mark unset, so swapmark fails
 * 
 * revision 1.5
 * date: 1991/06/03 10:26:34;
 * cleanup, and
 * pulled some code out of execute() into here
 * 
 * revision 1.4
 * date: 1991/05/31 11:38:43;
 * syntax error from bad merge
 * 
 * revision 1.3
 * date: 1991/05/31 11:17:31;
 * lot of cleanup, some new operators -- now use godotplus() instead of
 * stutterfunc().  much better
 * 
 * revision 1.2
 * date: 1991/04/04 09:39:56;
 * added operfilter (!) command
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:52;
 * initial vile RCS revision
 */

#include	"estruct.h"
#include        "edef.h"
#ifndef NULL
#define NULL 0
#endif

extern CMDFUNC f_godotplus;
MARK pre_op_dot;

/* For the "operator" commands -- the following command is a motion, or
 *  the operator itself is repeated.  All operate on regions.
 */
operator(f,n,fn,str)
int f,n;
int (*fn)();
char *str;
{
	int c;
	int this1key;
	int status;
	CMDFUNC *cfp;			/* function to execute */
	char tok[NSTRING];		/* command incoming */
	BUFFER *ourbp;

	doingopcmd = TRUE;

	pre_op_dot = DOT;
	ourbp = curbp;

	if (havemotion != NULL) {
		cfp = havemotion;
		havemotion = NULL;
	} else {
		mlwrite("%s operation pending...",str);
		update(FALSE);

		/* get the next command from the keyboard */
		/* or a command line, as approp. */
		if (clexec) {
			macarg(tok);	/* get the next token */
			if (!strcmp(tok,"lines"))
				cfp = &f_godotplus;
			else
				cfp = engl2fnc(tok);
		} else {
			this1key = last1key;
			c = kbd_seq();

			/* allow second chance for entering counts */
			if (f == FALSE) {
				do_num_proc(&c,&f,&n);
				do_rept_arg_proc(&c,&f,&n);
			}

			if (this1key == last1key)
				cfp = &f_godotplus;
			else
				cfp = kcod2fnc(c);

		}
		if (cfp)
			mlerase();
		else
			mlforce("[No such function]");
	}
	if (!cfp)
		return FALSE;

	if ((cfp->c_flags & MOTION) == 0) {
		TTbeep();
		return(ABORT);
	}

	/* motion is interpreted as affecting full lines */
	if (cfp->c_flags & FL)
		fulllineregions = TRUE;

	/* and execute the motion */
	status = execute(cfp, f, n);

	if (status != TRUE || 
	   ( samepoint(pre_op_dot, DOT) && fulllineregions == FALSE) ) {
		doingopcmd = FALSE;
		fulllineregions = FALSE;
		return FALSE;
	}

	opcmd = 0;

	MK = pre_op_dot;

	/* we've successfully set up a region */
	if (!fn) { /* be defensive */
		mlforce("BUG -- null func pointer in operator");
		status = FALSE;
	} else {
		status = (fn)();
	}

	if (ourbp == curbp) /* in case the func switched buffers on us */
		swapmark();

	if (fulllineregions) {
		fulllineregions = FALSE;
		firstnonwhite(f,n);
	}

	doingopcmd = FALSE;
	return status;
}

operdel(f,n)
int f,n;
{
	extern int killregion();

	opcmd = OPDEL;
	return operator(f,n,killregion,"Delete");
}

operlinedel(f,n)
int f,n;
{
	extern int killregion();

	fulllineregions = TRUE;
	opcmd = OPDEL;
	return operator(f,n,killregion,"Delete of full lines");
}

chgreg()
{
	killregion();
	if (fulllineregions) {
		if (backline(FALSE,1) == TRUE) /* returns FALSE at top of buffer */
			return opendown(TRUE,1);
		else
			return openup(TRUE,1);
	}
	return ins();
}

operchg(f,n)
int f,n;
{
	int s;

	opcmd = OPOTHER;
	s = operator(f,n,chgreg,"Change");
	if (s == TRUE) swapmark();
	return s;
}

operlinechg(f,n)
int f,n;
{
	int s;

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	s = operator(f,n,chgreg,"Change of full lines");
	if (s == TRUE) swapmark();
	return s;
}

operyank(f,n)
int f,n;
{
	extern int yankregion();
	opcmd = OPOTHER;
	return operator(f,n,yankregion,"Yank");
}

operlineyank(f,n)
int f,n;
{
	extern int yankregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,yankregion,"Yank of full lines");
}

operflip(f,n)
int f,n;
{
	extern int flipregion();

	opcmd = OPOTHER;
	return operator(f,n,flipregion,"Flip case");
}

operupper(f,n)
int f,n;
{
	extern int upperregion();

	opcmd = OPOTHER;
	return operator(f,n,upperregion,"Upper case");
}

operlower(f,n)
int f,n;
{
	extern int lowerregion();

	opcmd = OPOTHER;
	return operator(f,n,lowerregion,"Lower case");
}


operlshift(f,n)
int f,n;
{
	extern int shiftlregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,shiftlregion,"Left shift");
}

operrshift(f,n)
int f,n;
{
	extern int shiftrregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,shiftrregion,"Right shift");
}

operwrite(f,n)
int f,n;
{
        register int    s;
        static char fname[NFILEN];
	extern int writeregion();

	if (ukb != 0) {
	        if ((s=mlreply("Write to file: ", fname, NFILEN)) != TRUE)
	                return s;
		return kwrite(fname,TRUE);
	} else {
		opcmd = OPOTHER;
		return operator(f,n,writeregion,"File write");
	}
}

operformat(f,n)
int f,n;
{
	extern int formatregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,formatregion,"Format");
}

operfilter(f,n)
int f,n;
{
	extern int filterregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,filterregion,"Filter");
}


operprint(f,n)
int f,n;
{
	extern int plineregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,plineregion,"Line print");
}

operlist(f,n)
int f,n;
{
	extern int llineregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,llineregion,"Line list");
}

opersubst(f,n)
int f,n;
{
	extern int substregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,substregion,"Substitute");
}

opersubstagain(f,n)
int f,n;
{
	extern int subst_again_region();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,subst_again_region,"Substitute-again");
}

operentab(f,n)
int f,n;
{
	extern int entab_region();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,entab_region,"Spaces-->Tabs");
}

operdetab(f,n)
int f,n;
{
	extern int detab_region();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,detab_region,"Tabs-->Spaces");
}

opertrim(f,n)
int f,n;
{
	extern int trim_region();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,trim_region,"Trim whitespace");
}
