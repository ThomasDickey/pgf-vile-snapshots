/*	MAP:	These routines provide a simple interface for the map command
 *		written by Otto Lind
 *		6/3/93
 *
 * $Log: map.c,v $
 * Revision 1.5  1993/07/01 16:15:54  pgf
 * tom's 3.51 changes
 *
 * Revision 1.4  1993/06/30  17:42:50  pgf
 * cleaned up, made SPEC bindings work
 *
 * Revision 1.3  1993/06/28  20:04:14  pgf
 * string_to_key replaced with prc2kcod
 *
 * Revision 1.2  1993/06/10  16:13:52  pgf
 * moved the recursion check from map_proc to map_check.  this makes
 * nesting possible, but loops are not detected....
 *
 * Revision 1.1  1993/06/10  14:58:47  pgf
 * ansification, and a little reformatting (sorry, otto...  :-)
 *
 * otto's Revision 1.2  93/06/06  21:27:33  otto
 * Added unmap, cleaned up.
 * 
 * otto's Revision 1.1  93/06/04  23:42:11  otto
 * Initial revision
 * 
 */

#include "estruct.h"
#include "edef.h"

#define DEFAULT_REG     -1

typedef struct _mapping {
    int			key;		/* key that is mapped */
    char		*kbdseq;	/* keyboard sequnce to replace */
    CMDFUNC		*oldfunc;	/* used in unmap operation */
    struct _mapping	*next;		/* used to form linked list */
} Mapping;

static Mapping	*mhead;			/* ptr to head of linked list */
static Mapping	*keymapped = NULL;	/* current mapped key seq used */
static TBUFF	*MapMacro;

static Mapping * search_map P(( int ));
static int install_bind P(( int, CMDFUNC *, CMDFUNC ** ));
static int install_map P(( int, char * ));
static int remove_map P(( int ));



/*
** search for key in mapped linked list
*/
static Mapping *
search_map(key)
int	key;
{
	register Mapping	*m;

	for (m = mhead; m; m = m->next)
	{
		if (m->key == key)
		{
			return m;
		}
	}
	return NULL;
}

/*
** install key and command into kbindtbl
*/
static int
install_bind(key, kcmd, oldcmd)
int	key;
CMDFUNC	*kcmd;		/* ptr to the requested function to bind to */
CMDFUNC	**oldcmd;	/* ptr to the old bind entry */
{
	register KBIND	*kbp;	/* pointer into a binding table */

	for (kbp = kbindtbl; kbp->k_cmd && kbp->k_code != key; kbp++)
		;

	/*
	** if found, change it in place, else add entry
	*/
	if (kbp->k_cmd)
	{
		*oldcmd = kbp->k_cmd;
		kbp->k_cmd = kcmd;
	}
	else
	{
		if (kbp >= &kbindtbl[NBINDS-1])
		{
			mlforce("[Prefixed binding table full]");
			return(FALSE);
		}
		*oldcmd = NULL;
		kbp->k_code = key;	/* add keycode */
		kbp->k_cmd = kcmd; 	/* and func pointer */
		++kbp;			/* and make sure the next is null */
		kbp->k_code = 0;
		kbp->k_cmd = NULL;
	}
	return TRUE;
}

/*
** install key sequence into appropriate table
*/
static int
install_map(key, v)
int		key;
char	*v;
{
	extern	CMDFUNC	f_map_proc;
	register Mapping	*m;

	if ((m = search_map(key)) != NULL)
	{
		free(m->kbdseq);
	}
	else
	{
		m = typealloc(Mapping);
		if (m == NULL)
			return FALSE;
		m->next = mhead;
		mhead = m;
	}

	m->kbdseq = strmalloc(v);
	if (m->kbdseq == NULL)
		return FALSE;
	m->key = key;
	
	/*
	** if control, install map_proc in key bind table, else
	** install into ascii table
	*/
	if (isspecial(key))
	{
		return install_bind(key, &f_map_proc, &m->oldfunc);
	}
	else
	{
		m->oldfunc = asciitbl[key];
		asciitbl[key] = &f_map_proc;
	}
	
	return TRUE;
}	

/*
** install key sequence into appropriate table
*/
static int
remove_map(key)
int	key;
{
	register Mapping	*m;
	register Mapping	*pm;

	/*
	** Remove entry from list
	*/
	pm = NULL;
	for (m = mhead; m; m = m->next)
	{
		if (m->key == key)
		{
			if (pm == NULL)
			{
				mhead = m->next;
			}
			else
			{
				pm->next = m->next;
			}
			/*
			** if control, restore olfunc into key bind table,
			** else restore ascii table
			*/
			if (isspecial(key))
			{
				KBIND	*kbp;

				for (kbp = kbindtbl;
				     kbp->k_cmd && kbp->k_code != key;
				     kbp++)
					;
				if (kbp->k_cmd)
				{
					kbp->k_cmd = m->oldfunc;
				}
			}
			else
			{
				asciitbl[key] = m->oldfunc;
			}
			free(m->kbdseq);
			free((char *)m);
			return TRUE;
		}
		pm = m;
	}
	return FALSE;
}

/*
** if a mapped char, save for subsequent processing
*/
void
map_check(key)
int	key;
{
	Mapping	*nkeymap;
	nkeymap = search_map(key);
	if (nkeymap != NULL && nkeymap == keymapped) {
		finish_kbm();
                mlforce("[Recursive map]");
		keymapped = NULL;
                return;
        }
	keymapped = nkeymap; 
}

/*
** set a map for the character/string combo
*/
/* ARGSUSED */
int
map(f, n)
int f,n;
{
	int	status;
	char 	kbuf[NSTRING];
	char 	val[NSTRING];

	kbuf[0] = '\0';
	status = mlreply("map key: ", kbuf, NSTRING - 1);
	if (status != TRUE)
		return status;

	val[0] = '\0';
	status = mlreply("map value: ", val, NSTRING - 1);
	if (status != TRUE)
		return status;

	if (install_map(prc2kcod(kbuf), val) != TRUE)
	{
		mlforce("[Mapping failed]");
		return FALSE;
	}
	return TRUE;
}

/*
** remove map entry, restore saved CMDFUNC for key
*/
/* ARGSUSED */
int
unmap(f, n)
int f,n;
{
	int	status;
	char 	kbuf[NSTRING];

	kbuf[0] = '\0';
	status = mlreply("unmap key: ", kbuf, NSTRING - 1);
	if (status != TRUE)
		return status;

	if (remove_map(prc2kcod(kbuf)) != TRUE)
	{
		mlforce("[Key not mapped]");
		return FALSE;
	}
	return TRUE;
}

/*
** use the keyboard replay macro code to execute the mapped command
*/
/*ARGSUSED*/
int
map_proc(f, n)
int	f,n;
{
	register char		*c;
	char			num[11];

	/* Could be null if the user tries to execute map_proc directly */
	if (keymapped == NULL)
	{
		mlforce("[Key not mapped]");
		return FALSE;
	}

	(void)tb_init(&MapMacro, abortc);
	/*
	** if there is a repeat count, install it to be played
	*/
	if (n != 1) {
		(void)lsprintf(num, "%d", n);
		for (c = num; *c; c++)
			if (!tb_append(&MapMacro, *c))
				return FALSE;
	}
	for (c = keymapped->kbdseq; *c; c++)
		if (!tb_append(&MapMacro, *c))
			return FALSE;

	keymapped = NULL;

	return start_kbm(1, DEFAULT_REG, MapMacro);
}
