/*	MAP:	These routines provide a simple interface for the map command
 *		written by Otto Lind
 *		6/3/93
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/map.c,v 1.20 1994/07/11 22:56:20 pgf Exp $
 * 
 */

#include "estruct.h"
#include "edef.h"

#define DEFAULT_REG     -1

#define	MAPPED_LIST_NAME	ScratchName(Mapped Characters)

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
static int install_map P(( int, char * ));
static int remove_map P(( int ));

#if OPT_SHOW_MAPS
static void makecharslist P(( int, char * ));
static int show_mapped_chars P(( void ));
#endif

#if OPT_SHOW_MAPS && OPT_UPBUFF
static int show_Mappings P(( BUFFER * ));
#endif

/*
** search for key in mapped linked list
*/
static Mapping *
search_map(key)
int	key;
{
	register Mapping	*m;

	for (m = mhead; m; m = m->next)
		if (m->key == key)
			return m;
	return NULL;
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
	char	temp[NSTRING];
	int	test;
	int	status;

	if (key < 0) {
		mlforce("[Illegal keycode]");
		return FALSE;	/* not a legal key-code */
	}

	/* check for attempted recursion
	 * patch: prc2kcod assumes only a single key-sequence
	 * patch: assumes no null chars in v -- should be pstring.
	 */
	test = prc2kcod(bytes2prc(temp, v, (int)strlen(v)));
	if (test == key || search_map(test) != NULL) {
		mlforce("[Attempted recursion]");
		return FALSE;
	}

	if ((m = search_map(key)) != NULL) {
		free(m->kbdseq);
	} else {
		m = typealloc(Mapping);
		if (m == NULL)
			return FALSE;
		m->next = mhead;
		mhead = m;
	}

	m->kbdseq = strmalloc(v);
	if (m->kbdseq == NULL) {
		status = FALSE;
	} else {
		m->key = key;
#if REBIND
		status = install_bind(key, &f_map_proc, &m->oldfunc);
#endif
	}
	if (status != TRUE)
		mlforce("[Mapping failed]");
	return status;
}	

/*
** Remove entry from list and restore the old function pointer.
*/
static int
remove_map(key)
int	key;
{
	register Mapping	*m;
	register Mapping	*pm;
	int	status = FALSE;

	for (m = mhead, pm = NULL; m; m = m->next) {
		if (m->key == key) {
			if (pm == NULL)
				mhead = m->next;
			else
				pm->next = m->next;
#if REBIND
			status = rebind_key(key, m->oldfunc);
#endif
			free(m->kbdseq);
			free((char *)m);
			break;
		}
		pm = m;
	}
	return status;
}

#define relist_mappings()

#if OPT_SHOW_MAPS
#define MAPS_PREFIX 12

static	int	show_all;

/*ARGSUSED*/
static void
makecharslist(flag, ptr)
int	flag;
char	*ptr;
{
	register Mapping *m;
	char	temp[NSTRING];

	b_set_left_margin(curbp, MAPS_PREFIX);
	for (m = mhead; m != 0; m = m->next) {
		if (m != mhead) {
			bputc('\n');
			lsettrimmed(lBack(DOT.l));
		}
		bprintf("%*S", MAPS_PREFIX, show_all
			? kcod2pstr(m->key, temp) + 1	/* FIXXX: nulls */
			: kcod2prc(m->key, temp));


		/* patch -- m->kbdseq cannot have nulls in it.  need length */
		bprintf("%s", show_all
			? m->kbdseq
			: bytes2prc(temp, m->kbdseq, (int)strlen(m->kbdseq)));
		lsettrimmed(l_ref(DOT.l));
	}
}

static int
show_mapped_chars()
{
	return liststuff(MAPPED_LIST_NAME, makecharslist, 0, (char *)0);
}

#if OPT_UPBUFF
static int
show_Mappings(bp)
BUFFER *bp;
{
	b_clr_obsolete(bp);
	return show_mapped_chars();
}

#undef relist_mappings

void
relist_mappings()
{
	update_scratch(MAPPED_LIST_NAME, show_Mappings);
}
#endif	/* OPT_UPBUFF */

#endif	/* OPT_SHOW_MAPS */

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

#if OPT_SHOW_MAPS
	if (end_named_cmd()) {
		show_all = f;
		return show_mapped_chars();
	}
#endif
	kbuf[0] = EOS;
	status = mlreply("map key: ", kbuf, sizeof(kbuf));
	if (status != TRUE)
		return status;

	hst_glue(' ');
	val[0] = EOS;
	status = mlreply("map value: ", val, sizeof(val));
	if (status != TRUE)
		return status;

	if ((status = install_map(prc2kcod(kbuf), val)) == TRUE) {
		relist_mappings();
	}
	return status;
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

	kbuf[0] = EOS;
	status = mlreply("unmap key: ", kbuf, sizeof(kbuf));
	if (status != TRUE)
		return status;

	if (remove_map(prc2kcod(kbuf)) != TRUE) {
		mlforce("[Key not mapped]");
		return FALSE;
	}
	relist_mappings();
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
	/* Could be null if the user tries to execute map_proc directly */
	if (keymapped == NULL || keymapped->kbdseq == NULL) {
		mlforce("[Key not mapped]");
		return FALSE;
	}

	(void)tb_init(&MapMacro, abortc);
	if (f) { /* then an arg was given */
		char	num[10];
		(void)lsprintf(num, "%d", n);
		if (!tb_sappend(&MapMacro, num))
			return FALSE;
	}
	if (!tb_sappend(&MapMacro, keymapped->kbdseq))
		return FALSE;

	keymapped = NULL;

	return start_kbm(1, DEFAULT_REG, MapMacro);
}
