/*	MAP:	These routines provide a simple interface for the map command
 *		written by Otto Lind
 *		6/3/93
 *
 * $Log: map.c,v $
 * Revision 1.2  1993/06/10 16:13:52  pgf
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

static int string_to_key P(( char * ));
static Mapping * search_map P(( int ));
static int install_bind P(( int, CMDFUNC *, CMDFUNC ** ));
static int install_map P(( int, char * ));
static int remove_map P(( int ));


/*
** translate string to key (sort of like prc2kcod)
*/
static int
string_to_key(k)
char	*k;
{
	int	key;
	char	*s;

	/*
	** Hack to allow ascii keyspecs in the form of 0xff. Need to add
	** key entries for map_proc in cmdtbl to take advantage of this.
	** (I use it for strange PC function key mappings)
	*/
	if (*k == '0' && (*(k + 1) == 'x' || *(k + 1) == 'X'))
	{
		key = (int)strtol(k, &s, 0);
		key |= SPEC;
	}
	/* Accept cntl_a in both raw and printable form */
	else if (*k == cntl_a)
	{
		key = (int)*(k+1) | CTLA;
	}
	else if (*k == '^' && *(k+1) == toalpha(cntl_a) && *(k+2) == '-')
	{
		key = (int)*(k+3) | CTLA;
	}
	/* Accept cntl_x in both raw and printable form */
	else if (*k == cntl_x)
	{
		key = (int)*(k+1) | CTLX;
	}
	else if (*k == '^' && *(k+1) == toalpha(cntl_x) && *(k+2) == '-')
	{
		key = (int)*(k+3) | CTLX;
	}
	/* otherwise raw form */
	else
	{
		key = (int)*k;
	}

	return key;
}

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
			mlwrite("Prefixed binding table full");
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
		m = (Mapping *)malloc(sizeof (Mapping));
		if (m == NULL)
			return FALSE;
		m->next = mhead;
		mhead = m;
	}

	m->kbdseq = malloc(strlen(v) + 1);
	if (m->kbdseq == NULL)
		return FALSE;
	m->key = key;
	strcpy(m->kbdseq, v);
	
	if (!(key & SPEC))
	{
		/*
		** if control, install map_proc in key bind table, else
		** install into ascii table
		*/
		if (key & (CTLA | CTLX))
		{
			return install_bind(key, &f_map_proc, &m->oldfunc);
		}
		else
		{
			m->oldfunc = asciitbl[key];
			asciitbl[key] = &f_map_proc;
		}
        }
	else
	{
		m->oldfunc = NULL;
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
			if (key & (CTLA | CTLX))
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
			free(m);
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

	if (install_map(string_to_key(kbuf), val) != TRUE)
	{
		mlwrite("mapping failed");
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
	status = mlreply("map key: ", kbuf, NSTRING - 1);
	if (status != TRUE)
		return status;

	if (remove_map(string_to_key(kbuf)) != TRUE)
	{
		mlwrite("key not mapped");
		return FALSE;
	}
	return TRUE;
}

/*
** use the keyboard replay macro code to execute the mapped command
*/
int
map_proc(f, n)
int	f,n;
{
	register char		*c;
	char			num[11];

	/* Could be null if the user tries to execute map_proc directly */
	if (keymapped == NULL)
	{
		mlwrite("[Key not mapped]");
		return FALSE;
	}

	tb_init(&MapMacro, abortc);
	/*
	** if there is a repeat count, install it to be played
	*/
	if (n != 1)
	{
		sprintf(num, "%d", n);
		for (c = num; *c; c++)
		{
			tb_append(&MapMacro, *c);
		}
	}
	for (c = keymapped->kbdseq; *c; c++)
	{
		tb_append(&MapMacro, *c);
	}
	keymapped = NULL;

	return start_kbm(1, DEFAULT_REG, MapMacro);
}
