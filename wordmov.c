/* These routines report on transitions between word boundaries, both 
 *	in the punctuated vi sense, and in the whitespace/darkspace
 *	sense.  The transition is reported _after_ it has occurred.  You
 *	need to back up to get to the char. before the transition.
 *	Written for vile by Paul Fox, (c)1990
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/wordmov.c,v 1.12 1994/12/16 22:54:21 pgf Exp $
 *
 */

#include "estruct.h"
#include "edef.h"

#define WASSPACE 0
#define ISSPACE 0
#define WASIDENT 1
#define ISIDENT 1
#define WASOTHER 2
#define ISOTHER 2
#define WASNL 3
#define ISNL 3

static int ochartype;

void
setchartype()
{
	ochartype = getchartype();
}

int
getchartype()
{
	register int	c;

	if (is_at_end_of_line(DOT))
		return (ISNL);
	else
		c = char_at(DOT);
	return (isspace(c) ? ISSPACE : 
			( isident(c) ? ISIDENT : ISOTHER ) );
}


int
isnewwordf()
{
	register int	ret = FALSE;
	register int	type;

	type = getchartype();

	switch (ochartype) {
	case WASNL:
	case WASSPACE:
		switch (type) {
		case ISNL:    if (doingopcmd) { ret = SORTOFTRUE; break;}
		case ISSPACE: ret = FALSE;	break;
		case ISIDENT:
		case ISOTHER: ret = TRUE;	break;
		}
		break;
	case WASIDENT:
	case WASOTHER:
		switch (type) {
		case ISNL:    if (doingopcmd) { ret = SORTOFTRUE; break;}
		case ISSPACE: if (doingopcmd && opcmd != OPDEL) 
						{ ret = SORTOFTRUE; break;}
		case ISIDENT:
		case ISOTHER: ret = FALSE;	break;
		}
		break;
	}
	ochartype = type;
	return (ret);
}

int
isnewwordb()
{
	register int	ret = FALSE;
	register int	type;

	type = getchartype();

	switch (ochartype) {
	case WASNL:
	case WASSPACE:
		ret = FALSE;	break;
	case WASIDENT:
		switch (type) {
		case ISNL:
		case ISSPACE: ret = TRUE;	break;
		case ISIDENT:
		case ISOTHER: ret = FALSE;	break;
		}
		break;
	case WASOTHER:
		switch (type) {
		case ISNL:
		case ISSPACE: ret = TRUE;	break;
		case ISIDENT:
		case ISOTHER: ret = FALSE;	break;
		}
		break;
	}
	ochartype = type;
	return (ret);
}

int
isnewviwordf()
{
	register int	ret = FALSE;
	register int	type;

	type = getchartype();

	switch (ochartype) {
	case WASNL:
	case WASSPACE:
		switch (type) {
		case ISNL:    if (doingopcmd) { ret = SORTOFTRUE; break;}
		case ISSPACE: ret = FALSE;	break;
		case ISIDENT:
		case ISOTHER: ret = TRUE;	break;
		}
		break;
	case WASIDENT:
		switch (type) {
		case ISNL:    if (doingopcmd) { ret = SORTOFTRUE; break;}
		case ISSPACE: if (doingopcmd && opcmd != OPDEL) 
						{ ret = SORTOFTRUE; break;}
		case ISIDENT: ret = FALSE;	break;
		case ISOTHER: ret = TRUE;	break;
		}
		break;
	case WASOTHER:
		switch (type) {
		case ISNL:    if (doingopcmd) { ret = SORTOFTRUE; break;}
		case ISSPACE: if (doingopcmd && opcmd != OPDEL) 
						{ ret = SORTOFTRUE; break;}
		case ISOTHER: ret = FALSE;	break;
		case ISIDENT: ret = TRUE;	break;
		}
		break;
	}
	ochartype = type;
	return (ret);
}

int
isnewviwordb()
{
	register int	ret = FALSE;
	register int	type;

	type = getchartype();

	switch (ochartype) {
	case WASNL:
	case WASSPACE:
		ret = FALSE;	break;
	case WASIDENT:
		switch (type) {
		case ISNL:
		case ISSPACE:
		case ISOTHER: ret = TRUE;	break;
		case ISIDENT: ret = FALSE;	break;
		}
		break;
	case WASOTHER:
		switch (type) {
		case ISNL:
		case ISSPACE:
		case ISIDENT: ret = TRUE;	break;
		case ISOTHER: ret = FALSE;	break;
		}
		break;
	}
	ochartype = type;
	return (ret);
}

int
isendwordf()
{
	register int	ret = FALSE;
	register int	type;

	type = getchartype();

	switch (ochartype) {
	case WASNL:
	case WASSPACE:
		ret = FALSE;	break;
	case WASIDENT:
		switch (type) {
		case ISNL:
		case ISSPACE:
			if (doingopcmd) ret = SORTOFTRUE;
			else ret = TRUE;
			if (doingsweep) sweephack = TRUE;
			break;
		case ISIDENT:
		case ISOTHER: ret = FALSE;	break;
		}
		break;
	case WASOTHER:
		switch (type) {
		case ISNL:
		case ISSPACE:
			if (doingopcmd) ret = SORTOFTRUE;
			else ret = TRUE;
			if (doingsweep) sweephack = TRUE;
			break;
		case ISIDENT:
		case ISOTHER: ret = FALSE;	break;
		}
		break;
	}
	ochartype = type;
	return (ret);
}

int
isendviwordf()
{
	register int	ret = FALSE;
	register int	type;

	type = getchartype();

	switch (ochartype) {
	case WASNL:
	case WASSPACE:
		ret = FALSE;	break;
	case WASIDENT:
		switch (type) {
		case ISNL:
		case ISSPACE:
		case ISOTHER:
			if (doingopcmd) ret = SORTOFTRUE;
			else ret = TRUE;
			if (doingsweep) sweephack = TRUE;
			break;
		case ISIDENT: ret = FALSE;	break;
		}
		break;
	case WASOTHER:
		switch (type) {
		case ISNL:
		case ISSPACE:
		case ISIDENT:
			if (doingopcmd) ret = SORTOFTRUE;
			else ret = TRUE;
			if (doingsweep) sweephack = TRUE;
			break;
		case ISOTHER: ret = FALSE;	break;
		}
		break;
	}
	ochartype = type;
	return (ret);
}

#ifdef template
int
isANYTHING()
{
	register int	ret = FALSE;
	register int	type;

	type = getchartype();

	switch (ochartype) {
	case WASNL:
	case WASSPACE:
		switch (type) {
		case ISNL:
		case ISSPACE:
			ret = FALSE;	break;
		case ISIDENT:
			ret = FALSE;	break;
		case ISOTHER:
			ret = FALSE;	break;
		}
		break;
	case WASIDENT:
		switch (type) {
		case ISNL:
		case ISSPACE:
			ret = TRUE;	break;
		case ISIDENT:
			ret = FALSE;	break;
		case ISOTHER:
			ret = TRUE;	break;
		}
		break;
	case WASOTHER:
		switch (type) {
		case ISNL:
		case ISSPACE:
			ret = TRUE;	break;
		case ISIDENT:
			ret = TRUE;	break;
		case ISOTHER:
			ret = FALSE;	break;
		}
		break;
	}
	ochartype = type;
	return (ret);
}
#endif /* template */

