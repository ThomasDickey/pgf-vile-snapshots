/*
 * map.c	-- map and map! interface routines
 *	Original interface by Otto Lind, 6/3/93
 *	Additional map and map! support by Kevin Buettner, 9/17/94
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/map.c,v 1.22 1994/09/23 18:08:11 pgf Exp $
 * 
 */

#include "estruct.h"
#include "edef.h"

static char MAPPED_LIST_NAME_CM[] = ScratchName(Mapped Characters);
static char MAPPED_LIST_NAME_IM[] = ScratchName(Map! Characters);

#define MAXLHS	NSTRING

/*
 * Picture for struct maprec
 * -------------------------
 *
 * Assume the following mappings...
 *
 * map za abc
 * map zb def
 * map q  quit
 *
 * These may be represented by the following picture...
 *
 *   |
 *   v
 * +---+--------+---+---+   +---+--------+---+---+
 * | z |  NULL  | o | o-+-->| q | "quit" | 0 | 0 |
 * +---+--------+-|-+---+   +---+--------+---+---+
 *                |
 *		  v
 *              +---+--------+---+---+   +---+--------+---+---+
 *              | a | "abc"  | 0 | o-+-->| b | "def"  | 0 | 0 |
 *              +---+--------+---+---+   +---+--------+---+---+
 *
 * where the pertinent fields are as follows:
 *
 * +----+-----+-------+-------+
 * | ch | srv | dlink | flink |
 * +----+-----+-------+-------+
 *
 * When matching a character sequence, we follow dlink when we've found a
 * matching character.  We change the character to be matched to the next
 * character in the sequence.  If the character doesn't match, we stay at
 * the same level (with the same character) and follow flink.
 *
 */

struct maprec {
	int		ch;		/* character to match		*/
	int		flags;		/* flags word			*/
	struct maprec *	dlink;		/* Where to go to match the	*/
					/*   next character in a multi-	*/
					/*   character sequence.	*/
	struct maprec *	flink;		/* Where to try next if match	*/
					/*   against current character 	*/
					/*   is unsuccessful		*/
	int		irv;		/* system defined mapping: The	*/
					/*   (wide) character code to	*/
					/*   replace a matched sequence by */
	char          *	srv;		/* user defined mapping: This	*/
					/*   is the string to replace a	*/
					/*   matched sequence by	*/
};

#define MAPF_WAITLONGER 0x01

static struct maprec *map_command = NULL;
static struct maprec *map_insert = NULL;

static	int	map_common P(( int f, struct maprec **, char * ));
static	int	unmap_common P(( struct maprec **, char * ));
static	void	addtomap P(( struct maprec **, char *, int, int, char * ));
static	int	delfrommap P(( struct maprec **, char * ));


#if OPT_SHOW_MAPS
static void makecharslist P(( int, char * ));
static int show_mapped_chars P(( char * ));
#endif

#if OPT_SHOW_MAPS && OPT_UPBUFF
static int show_Mappings P(( BUFFER * ));
#endif

#if OPT_UPBUFF
static void relist_mappings P(( char * ));
#else
#define relist_mappings(name)
#endif

#define relist_mappings()

#if OPT_SHOW_MAPS
#define MAPS_PREFIX 12

static	int	show_all;

/*ARGSUSED*/
#if 0
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
#endif
static void
makecharslist(flag, ptr)
    int   flag;
    char *ptr;
{
    char lhsstr[MAXLHS];
    struct maprec *lhsstack[MAXLHS];
    struct maprec *mp = (struct maprec *) ptr;
    int depth = 0;

    for (;;) {
	if (mp) {
	    lhsstr[depth] = mp->ch;
	    lhsstack[depth++] = mp->flink;
	    if (mp->srv) {
		lhsstr[depth] = 0;
		bprintf("%*S%s\n", MAPS_PREFIX, lhsstr, mp->srv);
	    }
	    mp = mp->dlink;
	}
	else if (depth > 0)
	    mp = lhsstack[--depth];
	else
	    break;
    }
}

static int
show_mapped_chars(bname)
    char *bname;
{
	struct maprec *mp = (strcmp(bname, MAPPED_LIST_NAME_CM) == 0)
	                  ? map_command : map_insert;
	return liststuff(bname, makecharslist, 0, (char *)mp);
}

#if OPT_UPBUFF
static int
show_Mappings(bp)
BUFFER *bp;
{
	b_clr_obsolete(bp);
	return show_mapped_chars(bp->b_bname);
}

#undef relist_mappings



void
relist_mappings(bufname)
    char * bufname;
{
    update_scratch(bufname, show_Mappings);
}
#endif	/* OPT_UPBUFF */

#endif	/* OPT_SHOW_MAPS */

/*
** set a map for the character/string combo
*/
/* ARGSUSED */
int
map(f, n)
    int f, n;
{
    return map_common(f, &map_command, MAPPED_LIST_NAME_CM);
}

/* ARGSUSED */
int
map_bang(f, n)
    int f, n;
{
    return map_common(f, &map_insert, MAPPED_LIST_NAME_IM);
}

static int
map_common(f, mpp, bufname)
    int f;
    struct maprec **mpp;
    char *bufname;
{
    int	 status;
    char kbuf[NSTRING];
    char val[NSTRING];

#if OPT_SHOW_MAPS
    if (end_named_cmd()) {
	show_all = f;
	return show_mapped_chars(bufname);
    }
#endif
    kbuf[0] = EOS;
    status = kbd_string("map key: ", kbuf, sizeof(kbuf),
			' ', KBD_NOMAP, no_completion);
    if (status != TRUE)
	return status;

    hst_glue(' ');
    val[0] = EOS;
    status = kbd_string("map value: ", val, sizeof(val),
			'\n', KBD_NOMAP, no_completion);
    if (status != TRUE)
	return status;

    addtomap(mpp, kbuf, 0, -1, val);
    relist_mappings(bufname);
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
    return unmap_common(&map_command, MAPPED_LIST_NAME_CM);
}

int
unmap_bang(f, n)
    int f, n;
{
    return unmap_common(&map_insert, MAPPED_LIST_NAME_IM);
}

static int
unmap_common(mpp, bufname)
    struct maprec **mpp;
    char *bufname;
{
    int	 status;
    char kbuf[NSTRING];

    kbuf[0] = EOS;
    status = kbd_string("unmap key: ", kbuf, sizeof(kbuf),
			' ', KBD_NOMAP, no_completion);
    if (status != TRUE)
	return status;

    if (delfrommap(mpp, kbuf) != TRUE) {
	mlforce("[Key not mapped]");
	return FALSE;
    }
    relist_mappings(bufname);
    return TRUE;
}
    
/* addtomaps is used to initialize both the command and input maps */
void
addtomaps(seq, code)
    char * seq;
    int    code;
{
    addtomap(&map_command, seq, 0, code, NULL);
    addtomap(&map_insert,  seq, 0, code, NULL);
}

static void
addtomap(mpp, ks, waitflag, irv, srv)
    struct maprec **mpp;
    char *	ks;
    int         waitflag;
    int		irv;
    char *	srv;
{
    struct maprec *mp = NULL;

    if (ks == 0 || *ks == 0)
	return;

    while (*mpp && *ks) {
	mp = *mpp;
	mp->flags |= waitflag;
	if (*ks == mp->ch) {
	    mpp = &mp->dlink;
	    ks++;
	}
	else
	    mpp = &mp->flink;
    }

    while (*ks) {
	if (!(mp = typealloc(struct maprec)))
	    break;
	*mpp = mp;
	mp->dlink = mp->flink = NULL;
	mp->ch = *ks++;
	mp->srv = NULL;
	mp->irv = -1;
	mpp = &mp->dlink;
    }

    if (irv != -1)
	mp->irv = irv;
    if (srv) {
	if (mp->srv)
	    free(mp->srv);
	mp->srv = strmalloc(srv);
    }
}

static int
delfrommap(mpp, ks)
    struct maprec **mpp;
    char * ks;
{
    struct maprec **mstk[MAXLHS];
    int depth = 0;

    if (ks == 0 || *ks == 0)
	return FALSE;

    while (*mpp && *ks) {
	mstk[depth] = mpp;
	if ((*mpp)->ch == *ks) {
	    mpp = &(*mpp)->dlink;
	    ks++;
	    depth++;
	}
	else
	    mpp = &(*mpp)->flink;
    }

    if (*ks)
	return FALSE;		/* not in map */

    depth--;
    if ((*mstk[depth])->srv) {
	free((*mstk[depth])->srv);
	(*mstk[depth])->srv = NULL;
    }
    else
	return FALSE;

    for (; depth >= 0; depth--) {
	struct maprec *mp = *mstk[depth];
	if (mp->irv == -1 && mp->dlink == NULL && mp->srv == NULL) {
	    *mstk[depth] = mp->flink;
	    if (depth > 0 && (*mstk[depth-1])->dlink == mp)
		(*mstk[depth-1])->dlink = NULL;
	    free(mp);
	}
	else
	    break;
    }
    return TRUE;
}

/*
 * The following function is given a character which may or may not start a
 * mapped sequence of characters.  If it does start a mapped sequence, then
 * that sequence is consumed via tgetc().  The first character of the
 * mapping will be returned.  The rest of the mapping will be ungotten via
 * tungetstr().  These mapped characters will then be read in subsequent
 * calls to tgetc().  If the character is determined to not have started a
 * mapped sequence, then it will be returned as the result of this function
 * and any subsquent characters which were read are ungotten.
 */
int
maplookup(c, delayedptr)
    int c;
    int *delayedptr;
{
    struct maprec *mp = (reading_msg_line) 
                            ? 0 : (insertmode) ? map_insert : map_command;
    struct maprec *rmp = NULL;
    char unmatched[MAXLHS];
    register char *s = unmatched;
    int cnt;

    *s++ = c;
    *delayedptr = 0;
    cnt = 0;
    while (mp) {
	if (c == mp->ch) {
	    int nextc;
	    if (mp->irv != -1 || mp->srv != NULL) {
		rmp = mp;
		cnt += (s - unmatched);
		s = unmatched;
	    }

	    mp = mp->dlink;

	    if (!mp)
		break;

	    /* if there's no recorded input, and no user typeahead */
	    if ((nextc = get_recorded_char(FALSE)) == -1 && !typahead()) {
		/* give it a little extra time... */
		/* FIXME: Use MAPF_WAITLONGER to control length of nap */
		catnap(global_g_val(GVAL_TIMEOUTVAL),TRUE);
		(*delayedptr)++;
	    }

	    /* and then, if there _was_ recorded input or new typahead... */
	    if (nextc != -1 || typahead()) {
		*s++ = c = tgetc(FALSE);
	    }
	    else
		break;
	}
	else
	    mp = mp->flink;
    }

    if (rmp) {
	/* unget the unmatched suffix */
	while (s > unmatched)
	    tungetc(*--s);
	/* unget the mapping and elide correct number or recorded chars */
	if (rmp->srv) {
	    tungetstr(rmp->srv, cnt);
	    return tgetc(FALSE);
	}
	else {
	    return rmp->irv;
	}
    }
    else {	/* didn't find a match */
	while (s > unmatched+1)
	    tungetc(*--s);
	return unmatched[0];
    }
}
