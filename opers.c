/*
 * This file contains the command processing functions for the commands
 * that take motion operators.
 * written for vile by Paul Fox, (c)1990
 *
 * $Log: opers.c,v $
 * Revision 1.36  1994/02/22 11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

#include	"estruct.h"
#include        "edef.h"

extern CMDFUNC f_godotplus;

typedef	int	(*OpsFunc) P((void));

static	int	chgreg P(( void ));
static	int	shift_n_times P(( int, int, OpsFunc, char * ));


/* For the "operator" commands -- the following command is a motion, or
 *  the operator itself is repeated.  All operate on regions.
 */
int
operator(f,n,fn,str)
int f,n;
OpsFunc fn;
char *str;
{
	int c;
	int thiskey;
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
		(void)update(FALSE);

		/* get the next command from the keyboard */
		/* or a command line, as approp. */
		if (clexec) {
			macarg(tok);	/* get the next token */
			if (!strcmp(tok,"lines"))
				cfp = &f_godotplus;
			else
				cfp = engl2fnc(tok);
		} else {
			thiskey = lastkey;
			c = kbd_seq();

			/* allow second chance for entering counts */
			do_repeats(&c,&f,&n);

			if (thiskey == lastkey)
				cfp = &f_godotplus;
			else
				cfp = kcod2fnc(c);

		}
		if (cfp)
			mlerase();
		else
			mlforce("[No such function]");
	}
	if (!cfp) {
		doingopcmd = FALSE;
		return FALSE;
	}

	if ((cfp->c_flags & MOTION) == 0) {
		TTbeep();
		doingopcmd = FALSE;
		return(ABORT);
	}

	/* motion is interpreted as affecting full lines */
	if (cfp->c_flags & FL)
		fulllineregions = TRUE;

	/* and execute the motion */
	status = execute(cfp, f,n);

	if (status != TRUE) {
		doingopcmd = FALSE;
		fulllineregions = FALSE;
		mlforce("[Motion failed]");
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
		(void)firstnonwhite(FALSE,1);
	}

	doingopcmd = FALSE;
	return status;
}

int
operdel(f,n)
int f,n;
{
	int	status;

	opcmd = OPDEL;
	lines_deleted = 0;
	status = operator(f, n, killregion,
		fulllineregions
			? "Delete of full lines"
			: "Delete");
	if (do_report(lines_deleted))
		mlforce("[%d lines deleted]", lines_deleted);
	return status;
}

int
operlinedel(f,n)
int f,n;
{
	fulllineregions = TRUE;
	return operdel(f,n);
}

static int
chgreg()
{
	killregion();
	if (fulllineregions) {
		if (backline(FALSE,1) == TRUE) /* returns FALSE at top of buf */
			return opendown(TRUE,1);
		else
			return openup(TRUE,1);
	}
	return ins();
}

int
operchg(f,n)
int f,n;
{
	int s;

	opcmd = OPOTHER;
	s = operator(f,n,chgreg,"Change");
	if (s == TRUE) swapmark();
	return s;
}

int
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

int
operjoin(f,n)
int f,n;
{
	opcmd = OPOTHER;
	return operator(f,n,joinregion,"Join");
}

int
operyank(f,n)
int f,n;
{
	opcmd = OPOTHER;
	return operator(f,n,yankregion,"Yank");
}

int
operlineyank(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,yankregion,"Yank of full lines");
}

int
operflip(f,n)
int f,n;
{
	opcmd = OPOTHER;
	return operator(f,n,flipregion,"Flip case");
}

int
operupper(f,n)
int f,n;
{
	opcmd = OPOTHER;
	return operator(f,n,upperregion,"Upper case");
}

int
operlower(f,n)
int f,n;
{
	opcmd = OPOTHER;
	return operator(f,n,lowerregion,"Lower case");
}

/*
 * The shift commands are special, because vi allows an implicit repeat-count
 * to be specified by repeating the '<' or '>' operators.
 */
static int
shift_n_times(f,n, func, msg)
int	f,n;
OpsFunc	func;
char	*msg;
{
	register int status = FALSE;

	fulllineregions = TRUE;
	opcmd = OPOTHER;

	if (havemotion != NULL) {
		CMDFUNC *cfp = havemotion;
		while (n-- > 0) {
			havemotion = cfp;
			if ((status = operator(FALSE,1, func, msg)) != TRUE)
				break;
		}
	} else
		status = operator(f,n, func, msg);
	return status;
}

int
operlshift(f,n)
int f,n;
{
	return shift_n_times(f,n,shiftlregion,"Left shift");
}

int
operrshift(f,n)
int f,n;
{
	return shift_n_times(f,n,shiftrregion,"Right shift");
}

int
operwrite(f,n)
int f,n;
{
        register int    s;
        char fname[NFILEN];

	if (ukb != 0) {
	        if ((s=mlreply_file("Write to file: ", (TBUFF **)0,
				FILEC_WRITE|FILEC_PROMPT, fname)) != TRUE)
	                return s;
		return kwrite(fname,TRUE);
	} else {
		opcmd = OPOTHER;
		return operator(f,n,writeregion,"File write");
	}
}

int
operformat(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,formatregion,"Format");
}

int
operfilter(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,filterregion,"Filter");
}


int
operprint(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,plineregion,"Line print");
}

int
operlist(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,llineregion,"Line list");
}

int
opersubst(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,substregion,"Substitute");
}

int
opersubstagain(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,subst_again_region,"Substitute-again");
}

int
operentab(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,entab_region,"Spaces-->Tabs");
}

int
operdetab(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,detab_region,"Tabs-->Spaces");
}

int
opertrim(f,n)
int f,n;
{
	fulllineregions = TRUE;
	opcmd = OPOTHER;
	return operator(f,n,trim_region,"Trim whitespace");
}
