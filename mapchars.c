/*
 *	mapchars.c
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/mapchars.c,v 1.2 1994/07/11 22:56:20 pgf Exp $
 */

#include "estruct.h"
#include "edef.h"
#include "trace.h"

#if	NEW_VI_MAP
#include "mapchars.h"

#define	MAPPED_LIST_NAME	ScratchName(MAP+BIND Tree)

static	MAPCHAR *AllocMap P(( int ));
static	MAPNODE *CmdToNode P(( CMDFUNC * ));
static	void define_mapping P(( TBUFF *, MAPNODE * ));

#if OPT_SHOW_MAPS
static void indent P(( void ));
static void show_map_tree P(( int, MAPCHAR * ));
static void makecharslist P(( int, char * ));
static int show_mapped_chars P(( void ));
#endif

#if OPT_SHOW_MAPS && OPT_UPBUFF
static int show_Mappings P(( BUFFER * ));
static void relist_Mappings P(( void ));
#endif

#if LATER
MAPCHAR	**mapc_index;	/* pointers to trees of MAPCHAR structs */
#else
static MAPCHAR *mapc_index[N_chars];
#endif

static MAPCHAR *
AllocMap(c)
int	c;
{
	register MAPCHAR *p = typealloc(MAPCHAR);
	static MAPNODE dummy; /* e.g., {(MAPCHAR *)0, MAP_NEXT} */
	p->c_key  = c;
	p->c_list = 0;
	p->c_node = dummy;
	Trace("\t\talloc %p (%p)\n", p, p->c_node.n_next);
	return p;
}

static MAPNODE *
CmdToNode(cmd)
CMDFUNC *cmd;
{
	static	MAPNODE	node;
	if (cmd != 0) {
		node.n_command = cmd;
		node.n_type = MAP_COMMAND;
		return &node;
	}
	return 0;
}

static char *CmdToName(MAPNODE *node)
{
	register NTAB *nptr;
	for (nptr = nametbl; nptr->n_name != NULL; ++nptr) {
		if (nptr->n_cmd == node->n_command) {
			return nptr->n_name;
		}
	}
	return "?";
}

/*
 * Compare two MAPCHAR nodes, so we can build lists in sorted order.
 */
static int
CompareNodes (MAPCHAR *a, MAPCHAR *b)
{
	return char2int(a->c_key) - char2int(b->c_key);
}

/*
 * Note: If we get a long keystroke combination (e.g., "abcd") that obscures
 * another (e.g., "abc"), we don't delete the other, but retain (but ignore)
 * the other in the linked-list until the longer sequence is unmapped.
 */
static void
define_mapping(seq, node)
TBUFF *seq;
MAPNODE *node;
{
	UCHAR	c;
	MAPCHAR *p, *q, *r, *s;

	if (node == 0)		/* simplifies 'map_init()' ... */
		return;

	Trace("define_mapping(%.*s, %p) %d:%s\n",
		tb_length(seq),
		tb_values(seq),
		node,
		node->n_type,
		node->n_type == MAP_COMMAND ? CmdToName(node) : "<data>");

	tb_first(seq);
	p = NULL;
	while (tb_more(seq)) {
		q = p;
		c = tb_next(seq);
		if (q == NULL) {
			p = mapc_index[c];
			if (p == NULL) {
				mapc_index[c] = p = AllocMap(c);
			}
		} else {
			/*
			 * XXX
			 * If the parent 'q' wasn't an intermediate node, we've
			 * got an ambiguity (e.g., "a" vs "ab").
			 */
			if (q->c_node.n_type != MAP_NEXT) {
Trace("...override type %d\n", q->c_node.n_type);
				q->c_node.n_type = MAP_NEXT;
				q->c_node.n_next = NULL;
			}

			/*
			 * Look in the list of siblings to see if any matches
			 * the value 'c'.  If so, we don't need to allocate a
			 * node at this level.
			 */
			for (r = q->c_node.n_next; r != 0; r = r->c_list) {
				if (r->c_key == (UCHAR)c)
					break;
			}

			/*
			 * If 'r' isn't null, we found a match.
			 */
			if (r != NULL) {
Trace("...next '%c' %p\n", c, r);
				p = r;
			} else {
				p = AllocMap(c);
Trace("...alloc '%c', this=%p, next=%p\n", c, p, q->c_node.n_next);
				for (r = q->c_node.n_next, s = 0; r != 0; s = r, r = r->c_list) {
					if (CompareNodes(r,p) >= 0) {
						break;
					}
				}
				if (s == NULL) {
					Trace("...first\n");
					p->c_list = q->c_node.n_next;
					q->c_node.n_next = p;
				} else {
					Trace("...after %p\n", s);
					p->c_list = s->c_list;
					s->c_list = p;
				}
			}
		}
	}
	p->c_node = *node;
}

/*--------------------------------------------------------------------------*/
/* This is a temporary function, used to build the new-style mapping tree.
 * It translates the information in "nebind.h" into an equivalent tree.
 */
void
map_init()
{
	static	int	initialized;
	register int	c;

	if (!initialized) {
		TBUFF	*buff;
		initialized++;
#if LATER
		mapc_index = typeallocn(MAPCHAR *, N_chars);
#endif
		(void) tb_init(&buff, abortc);
		/* the normal characters */
		for (c = 0; c < N_chars; c++) {
			mapc_index[c] = 0;
			(void) tb_put(&buff, 0, c);
			define_mapping(buff, CmdToNode(asciitbl[c]));
		}
		/* ^A or ^X prefixes, and escape-sequence equivalences */
		for (c = 0; kbindtbl[c].k_cmd != 0; c++) {
			int	k = kbindtbl[c].k_code;
			(void) tb_init(&buff, abortc);
			if (k & CTLX)	tb_append(&buff, tocntrl('X'));
			if (k & CTLA)	tb_append(&buff, tocntrl('A'));
			if (k & SPEC)	tb_append(&buff, ESC);
			tb_append(&buff, kcod2key(k));
			define_mapping(buff, CmdToNode(kbindtbl[c].k_cmd));
		}
		/* other escape sequences */
		/* XXX */
	}
}
/*--------------------------------------------------------------------------*/
#define relist_Mappings()

#if OPT_SHOW_MAPS
#define MAPS_PREFIX 12

static	int	show_all;

static void
indent ()
{
	int level = MAPS_PREFIX;
	while (level-- > 0)
		bprintf(" ");
}

static void
show_map_tree (c, m)
int	c;
MAPCHAR	*m;
{
	NTAB	*nptr;

	while (m != 0) {
		bprintf("%d:%X\n", c, m);
		switch (m->c_node.n_type) {
		case MAP_NEXT:
			show_map_tree(m->c_key, m->c_node.n_next);
			break;
		case MAP_STRING:
			indent();
			bprintf("%d:string %s\n", m->c_key, m->c_node.n_string);
			break;
		case MAP_COMMAND:
			indent();
			bprintf("%d:command %X\n", m->c_key, m->c_node.n_command);
			for (nptr = nametbl; nptr->n_name != NULL; ++nptr) {
				if (nptr->n_cmd == m->c_node.n_command) {
					indent();
					bprintf("%s\n", nptr->n_name);
				}
			}
			break;
		case MAP_BUFFER:
			indent();
			bprintf("%d:buffer %s\n", m->c_key, m->c_node.n_buffer);
			break;
		default:
			indent();
			bprintf("?\n");
			break;
		}
		m = m->c_list;
	}
}

/*ARGSUSED*/
static void
makecharslist(flag, ptr)
int	flag;
char	*ptr;
{
	register int n;

	b_set_left_margin(curbp, MAPS_PREFIX);
	for (n = 0; n < N_chars; n++) {
		show_map_tree(n, mapc_index[n]);
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

#undef relist_Mappings

static void
relist_Mappings()
{
	register BUFFER *bp;
	if ((bp = find_b_name(MAPPED_LIST_NAME)) != 0) {
		bp->b_upbuff = show_Mappings;
		b_set_obsolete(bp);
	}
}
#endif	/* OPT_UPBUFF */

#endif	/* OPT_SHOW_MAPS */
/*--------------------------------------------------------------------------*/
/*
 * This implements the vi 'map' command.
 */
/*ARGSUSED*/
int
map_set(f,n)
int	f,n;
{
#if OPT_SHOW_MAPS
	if (end_named_cmd()) {
		show_all = f;
		return show_mapped_chars();
	}
#endif
	relist_Mappings(); /* XXX */
	return FALSE;
}

/*
 * This implements the vi 'unmap' command.
 */
/*ARGSUSED*/
int
map_unset(f,n)
int	f,n;
{
	return FALSE;
}

int
map_read()
{
	return -1;
}
#endif	/*NEW_VI_MAP*/
