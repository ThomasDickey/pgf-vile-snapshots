/*
 * This file contains the command processing functions for the commands
 * that take motion operators.
 * written for vile by Paul Fox, (c)1990
 */

#include	"estruct.h"
#include        "edef.h"
#ifndef NULL
#define NULL 0
#endif

/* dummy command function structure for getting operators to work on lines, when
	that's not what the user typed.  For instance, ": d" is synonymous
	with "dd", and "Y" is synonymous with "yy" (for some reason.... grrr).
	This makes that easier */
CMDFUNC f_stutterfunc;

/* For the "operator" commands -- the following command is a motion, or
 *  the operator itself is repeated.  All operate on regions.
 */
operator(f,n,fn1,fn2,str)
int (*fn1)(), (*fn2)();
char *str;
{
	int c;
	int this1key;
	int status;
	int stutter;
	CMDFUNC *execfunc;		/* ptr to function to execute */
	char tok[NSTRING];		/* command incoming */
	extern CMDFUNC f_stutterfunc;

	doingopcmd = TRUE;

	setmark();

	if (havemotion != NULL) {
		if (havemotion != &f_stutterfunc) {
			execfunc = havemotion;
			stutter = FALSE;
		} else {
			stutter = TRUE;
		}
		havemotion = NULL;
	} else {
		mlwrite("%s operation pending...",str);
		update(FALSE);

		/* get the next command from the keyboard */
		/* or a command line, as approp. */
		if (clexec) {
			macarg(tok);	/* get the next token */
			stutter = !strcmp(tok,"lines");
			if (!stutter)
				execfunc = engl2fnc(tok);
		} else {
			this1key = last1key;
			c = kbd_seq();
			stutter = (this1key == last1key);
			execfunc = NULL;

			/* allow second chance for entering counts */
			if (f == FALSE) {
				do_num_proc(&c,&f,&n);
				do_rept_arg_proc(&c,&f,&n);
			}
		}
		mlerase();
	}

	if (stutter) {
		fulllineregions = TRUE;
		curgoal = -1;
		forwline(TRUE, n<0 ? n+1:n-1);
		if (curwp->w_dotp == curbp->b_linep)
			backline(FALSE,1);
		curgoal = -1;
		status = TRUE;
	} else {
		/* and execute the command */
		status = execute(c, f, n, execfunc);
		if (status != TRUE || (atmark() &&
				fulllineregions == FALSE) ) {
			doingopcmd = FALSE;
			fulllineregions = FALSE;
			return status;
		}


	}

	opcmd = 0;

	/* we've successfully set up a region */
	if (stutter) {
		if (fn1 != 0) 
			status = (fn1)(f,n);
	} else {
		if (fn2 != 0) 
			status = (fn2)(f,n);
	}

	swapmark();

	if (fulllineregions) {
		fulllineregions = FALSE;
		firstnonwhite(f,n);
	}

	doingopcmd = FALSE;
	return status;
}

operdel(f,n)
{
	extern int killregion();

	opcmd = OPDEL;
	return operator(f,n,killregion,killregion,"Delete");
}

operlinedel(f,n)
{
	extern int killregion();

	fulllineregions = TRUE;
	opcmd = OPDEL;
	return operator(f,n,killregion,killregion,"Delete of full lines");
}

chgreg1(f,n)
{
	killregion(f,n);
	backline(FALSE,1);
	opendown(TRUE,1);
}

chgreg2(f,n)
{
	killregion(f,n);
	insert(f,n);
}

operchg(f,n)
{
	int s;

	opcmd = OPCHG;
	s = operator(f,n,chgreg1,chgreg2,"Change");
	swapmark();
	return s;
}

operlinechg(f,n)
{
	int s;

	fulllineregions = TRUE;
	opcmd = OPCHG;
	s = operator(f,n,chgreg1,chgreg2,"Change of full lines");
	swapmark();
	return s;
}

operyank(f,n)
{
	extern int yankregion();
	opcmd = OPYANK;
	return operator(f,n,yankregion,yankregion,"Yank");
}

operlineyank(f,n)
{
	extern int yankregion();

	fulllineregions = TRUE;
	opcmd = OPYANK;
	return operator(f,n,yankregion,yankregion,"Yank of full lines");
}

operflip(f,n)
{
	extern int flipregion();

	opcmd = OPFLIP;
	return operator(f,n,flipregion,flipregion,"Flip case");
}

operupper(f,n)
{
	extern int upperregion();

	opcmd = OPUPPER;
	return operator(f,n,upperregion,upperregion,"Upper case");
}

operlower(f,n)
{
	extern int lowerregion();

	opcmd = OPLOWER;
	return operator(f,n,lowerregion,lowerregion,"Lower case");
}


operlshift(f,n)
{
	extern int shiftlregion();

	fulllineregions = TRUE;
	opcmd = OPLSHIFT;
	return operator(f,n,shiftlregion,shiftlregion,"Left shift");
}

operrshift(f,n)
{
	extern int shiftrregion();

	fulllineregions = TRUE;
	opcmd = OPRSHIFT;
	return operator(f,n,shiftrregion,shiftrregion,"Right shift");
}

operwrite(f,n)
{
        register int    s;
        static char fname[NFILEN];
	extern int writeregion();

	if (ukb != 0) {
	        if ((s=mlreply("Write to file: ", fname, NFILEN)) != TRUE)
	                return s;
		return kwrite(fname,TRUE);
	} else {
		opcmd = OPWRITE;
		return operator(f,n,writeregion,writeregion,"File write");
	}
}

operformat(f,n)
{
	extern int formatregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,formatregion,formatregion,"Format");
}

