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

extern CMDFUNC f_godotplus;

/* For the "operator" commands -- the following command is a motion, or
 *  the operator itself is repeated.  All operate on regions.
 */
operator(f,n,fn,str)
int (*fn)();
char *str;
{
	int c;
	int this1key;
	int status;
	CMDFUNC *cfp;			/* function to execute */
	char tok[NSTRING];		/* command incoming */
	LINE *ourmarkp;
	int ourmarko;

	doingopcmd = TRUE;

        ourmarkp = curwp->w_dotp;
        ourmarko = curwp->w_doto;

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
		mlerase();
	}

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
	   ( (ourmarkp == curwp->w_dotp && ourmarko == curwp->w_doto) &&
			fulllineregions == FALSE) ) {
		doingopcmd = FALSE;
		fulllineregions = FALSE;
		return status;
	}

	opcmd = 0;

	curwp->w_mkp = ourmarkp;
	curwp->w_mko = ourmarko;

	/* we've successfully set up a region */
	if (!fn) { /* be defensive */
		mlwrite("BUG -- null func pointer in operator");
		status = FALSE;
	} else {
		status = (fn)(f,n,NULL,NULL);
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
	return operator(f,n,killregion,"Delete");
}

operlinedel(f,n)
{
	extern int killregion();

	fulllineregions = TRUE;
	opcmd = OPDEL;
	return operator(f,n,killregion,"Delete of full lines");
}

chgreg(f,n)
{
	killregion(f,n);
	if (fulllineregions) {
		backline(FALSE,1);
		opendown(TRUE,1);
	} else {
		insert(f,n);
	}
}

operchg(f,n)
{
	int s;

	opcmd = OPOTHER;
	s = operator(f,n,chgreg,"Change");
	swapmark();
	return s;
}

operlinechg(f,n)
{
	int s;

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	s = operator(f,n,chgreg,"Change of full lines");
	swapmark();
	return s;
}

operyank(f,n)
{
	extern int yankregion();
	opcmd = OPOTHER;
	return operator(f,n,yankregion,"Yank");
}

operlineyank(f,n)
{
	extern int yankregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,yankregion,"Yank of full lines");
}

operflip(f,n)
{
	extern int flipregion();

	opcmd = OPOTHER;
	return operator(f,n,flipregion,"Flip case");
}

operupper(f,n)
{
	extern int upperregion();

	opcmd = OPOTHER;
	return operator(f,n,upperregion,"Upper case");
}

operlower(f,n)
{
	extern int lowerregion();

	opcmd = OPOTHER;
	return operator(f,n,lowerregion,"Lower case");
}


operlshift(f,n)
{
	extern int shiftlregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,shiftlregion,"Left shift");
}

operrshift(f,n)
{
	extern int shiftrregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,shiftrregion,"Right shift");
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
		opcmd = OPOTHER;
		return operator(f,n,writeregion,"File write");
	}
}

operformat(f,n)
{
	extern int formatregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,formatregion,"Format");
}

operfilter(f,n)
{
	extern int filterregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,filterregion,"Filter");
}


operprint(f,n)
{
	extern int plineregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,plineregion,"Line print");
}

operlist(f,n)
{
	extern int llineregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,llineregion,"Line list");
}

opersubst(f,n)
{
	extern int substregion();

	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,substregion,"Substitute");
}
