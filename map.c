/*
 * map.c	-- map and map! interface routines
 *	Original interface by Otto Lind, 6/3/93
 *	Additional map and map! support by Kevin Buettner, 9/17/94
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/map.c,v 1.44 1994/12/10 00:15:04 pgf Exp $
 * 
 */

#include "estruct.h"
#include "edef.h"

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

#define MAPF_SYSTIMER	0x01
#define MAPF_USERTIMER	0x02
#define MAPF_TIMERS	0x03
#define MAPF_NOREMAP	0x04

static struct maprec *map_command = NULL;
static struct maprec *map_insert = NULL;
static struct maprec *map_syskey = NULL;
static struct maprec *abbr_map = NULL;

static	int	map_common P(( struct maprec **, char *, int ));
static	int	unmap_common P(( struct maprec **, char * ));
static	void	addtomap P(( struct maprec **, char *, int, int, int, char * ));
static	int	delfrommap P(( struct maprec **, char * ));

static	int	abbr_getc P(( void ));
static	int	abbr_c_avail P(( void ));

static int	mapgetc P(( void ));

typedef	int (*AvailFunc) P(( void ));
typedef	int (*GetFunc) P(( void ));

static int maplookup P(( int, ITBUFF **, struct maprec *, GetFunc, AvailFunc ));

#if OPT_SHOW_MAPS
static void makemaplist P(( int, void * ));
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

#if OPT_SHOW_MAPS
#define MAPS_PREFIX 12

/*ARGSUSED*/
static void
makemaplist(dummy, mapp)
    int   dummy;
    void *mapp;
{
    char lhsstr[MAXLHS];
    struct maprec *lhsstack[MAXLHS];
    struct maprec *mp = (struct maprec *) mapp;
    int depth = 0;
    int footnote = 0;
    int i;
    
    

    for (;;) {
	if (mp) {
	    char *remapnote;
	    char *mapstr;
	    char esc_seq[10];
	    lhsstr[depth] = mp->ch;
	    lhsstack[depth++] = mp->flink;
	    lhsstr[depth] = 0;

	    mapstr = (char *)0;
	    if (mp->srv) {
		mapstr = mp->srv;
	    } else if (mp->irv != -1) {
		(void)kcod2escape_seq(mp->irv, esc_seq);
		mapstr = esc_seq;
	    }
	    if (mapstr) {
		    if (mp == abbr_map) {
			    /* the abbr map is stored inverted */
			    for (i = depth; i > 0; )
				bputc(lhsstr[--i]);	/* may contain nulls */
		    } else {
			    if (mp->flags & MAPF_NOREMAP) {
				remapnote = "(n)";
				footnote++;
			    } else {
				remapnote = "   ";
			    }
			    bprintf("%s ", remapnote);
			    for (i = 0; i < depth; i++)
				bputc(lhsstr[i]);	/* may contain nulls */
		    }
		    bprintf("\t%s\n", mapstr);
	    }
	    mp = mp->dlink;
	}
	else if (depth > 0)
	    mp = lhsstack[--depth];
	else
	    break;
    }
    if (footnote) {
	bprintf("[(n) means never remap]\n");
    }
}

static int
show_mapped_chars(bname)
    char *bname;
{
	struct maprec *mp;
	if (strcmp(bname, MAP_BufName) == 0)
		mp = map_command;
	else if (strcmp(bname, MAPBANG_BufName) == 0)
		mp = map_insert;
	else if (strcmp(bname, ABBR_BufName) == 0)
		mp = abbr_map;
	else if (strcmp(bname, SYSMAP_BufName) == 0)
		mp = map_syskey;
	else
		return FALSE;
	return liststuff(bname, FALSE, makemaplist, 0, (void *)mp);
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

static void
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
    return map_common(&map_command, MAP_BufName, 0);
}

/* ARGSUSED */
int
map_bang(f, n)
    int f, n;
{
    return map_common(&map_insert, MAPBANG_BufName, 0);
}

/* ARGSUSED */
int
noremap(f, n)
    int f, n;
{
    return map_common(&map_command, MAP_BufName, MAPF_NOREMAP);
}

/* ARGSUSED */
int
noremap_bang(f, n)
    int f, n;
{
    return map_common(&map_insert, MAPBANG_BufName, MAPF_NOREMAP);
}

/* ARGSUSED */
int
abbrev(f, n)
    int f, n;
{
    return map_common(&abbr_map, ABBR_BufName, MAPF_NOREMAP);
}

#if OPT_SHOW_MAPS
/* ARGSUSED */
int
sysmap(f, n)
int f, n;
{
	return show_mapped_chars(SYSMAP_BufName);
}
#endif

static int
map_common(mpp, bufname, remapflag)
    struct maprec **mpp;
    char *bufname;
    int remapflag;
{
    int	 status;
    char kbuf[NSTRING];
    char val[NSTRING];
    int len;

#if OPT_SHOW_MAPS
    if (end_named_cmd()) {
	return show_mapped_chars(bufname);
    }
#endif
    kbuf[0] = EOS;
    status = kbd_string("change this string: ", kbuf, sizeof(kbuf),
			' ', KBD_NOMAP|KBD_NOEVAL, no_completion);
    if (status != TRUE)
	return status;

    hst_glue(' ');
    val[0] = EOS;
    if (!clexec) {
	    status = kbd_string("to this new string: ", val, sizeof(val),
			'\n', KBD_NOMAP, no_completion);
    } else {
	    (void)macliteralarg(val); /* consume to end of line */
	    status = (*val != EOS);
    }
    if (status != TRUE)
	return status;


    len = strlen(kbuf);
    if (*mpp == abbr_map) { /* reverse the lhs */
	int i, t;
	for (i = 0; i < len/2; i++) {
	    t = kbuf[len-i-1];
	    kbuf[len-i-1] = kbuf[i];
	    kbuf[i] = t;
	}
    }

    addtomap(mpp, kbuf, len, MAPF_USERTIMER|remapflag, -1, val);
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
    return unmap_common(&map_command, MAP_BufName);
}

/* ARGSUSED */
int
unmap_bang(f, n)
    int f, n;
{
    return unmap_common(&map_insert, MAPBANG_BufName);
}

static int
unmap_common(mpp, bufname)
    struct maprec **mpp;
    char *bufname;
{
    int	 status;
    char kbuf[NSTRING];

    kbuf[0] = EOS;
    status = kbd_string("unmap string: ", kbuf, sizeof(kbuf),
			' ', KBD_NOMAP, no_completion);
    if (status != TRUE)
	return status;

    if (delfrommap(mpp, kbuf) != TRUE) {
	mlforce("[Sequence not mapped]");
	return FALSE;
    }
    relist_mappings(bufname);
    return TRUE;
}
    
#if BEFORE
/* addtomaps is used to initialize both the command and input maps
	with the system default function key maps
*/
void
addtomaps(seq, seqlen, code)
    char * seq;
    int    seqlen;
    int    code;
{
    addtomap(&map_command, seq, seqlen, MAPF_SYSTIMER,
    			code, (char *)0);
    addtomap(&map_insert,  seq, seqlen, MAPF_SYSTIMER,
    			code, (char *)0);
    switch (code) {
    case KEY_Up:
    case KEY_Down:
    	addtomap(&map_message_line, seq, seqlen, MAPF_SYSTIMER,
			code, (char *)0);
    }
}
#endif

/* addtosysmap is used to initialize the system default function key map
*/
void
addtosysmap(seq, seqlen, code)
    char * seq;
    int    seqlen;
    int    code;
{
    addtomap(&map_syskey, seq, seqlen, MAPF_SYSTIMER,
    			code, (char *)0);
}

static void
addtomap(mpp, ks, kslen, flags, irv, srv)
    struct maprec **mpp;
    char *	ks;
    int         kslen;
    int         flags;
    int		irv;
    char *	srv;
{
    struct maprec *mp = NULL;

    if (ks == 0 || kslen == 0)
	return;

    while (*mpp && kslen) {
	mp = *mpp;
	mp->flags |= flags;
	if (*ks == mp->ch) {
	    mpp = &mp->dlink;
	    ks++;
	    kslen--;
	}
	else
	    mpp = &mp->flink;
    }

    while (kslen) {
	if (!(mp = typealloc(struct maprec)))
	    break;
	*mpp = mp;
	mp->dlink = mp->flink = NULL;
	mp->ch = *ks++;
	mp->srv = NULL;
	mp->flags = flags;
	mp->irv = -1;
	mpp = &mp->dlink;
	kslen--;
    }

    if (irv != -1)
	mp->irv = irv;
    if (srv) {
	if (mp->srv)
	    free(mp->srv);
	mp->srv = strmalloc(srv);
    }
    mp->flags = flags;
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
	    free((char *)mp);
	}
	else
	    break;
    }
    return TRUE;
}


#define INPUT_FROM_TTGET 1
#define INPUT_FROM_MAPGETC 2

static ITBUFF *sysmappedchars = NULL;

int
sysmapped_c()
{
    int c;

    /* still some left? */
    if (itb_more(sysmappedchars))
	return itb_last(sysmappedchars);

    c = TTgetc();


    /* will push back on sysmappedchars successful, or not */
    (void)maplookup(c, &sysmappedchars, map_syskey, TTgetc, TTtypahead);

    return itb_last(sysmappedchars);
}

int
sysmapped_c_avail()
{
    return itb_more(sysmappedchars) || TTtypahead();
}


static ITBUFF *mapgetc_ungottenchars = NULL;
static int mapgetc_ungotcnt;

void
mapungetc(c)
int c;
{
	itb_append(&mapgetc_ungottenchars, c);
	mapgetc_ungotcnt++;
}

#define TOOMANY 1200
static int infloopcount;
static int mapgetc_raw_flag;

static int
mapgetc()
{
    int remapflag;
    if (global_g_val(GMDREMAP))
    	remapflag = 0;
    else
    	remapflag = NOREMAP;

    if (mapgetc_ungotcnt > 0) {
	    if (infloopcount++ > TOOMANY) {
		itb_init(&mapgetc_ungottenchars, abortc);
		mapgetc_ungotcnt = 0;
		mlforce("[Infinite loop detected in %s sequence]",
			    (insertmode) ? "map!" : "map");
		catnap(1000,FALSE);  /* FIXX: be sure message gets out */
		return abortc|NOREMAP;
	    }
	    mapgetc_ungotcnt--;
	    return itb_last(mapgetc_ungottenchars) | remapflag;
    }
    infloopcount = 0;
    return tgetc(mapgetc_raw_flag);
}

int
mapped_c_avail()
{
    return mapgetc_ungotcnt > 0 || tgetc_avail();
}

int
mapped_c(remap, raw)
int remap;
int raw;
{
    int c;
    int matched;
    struct maprec *mp;
    int speckey = FALSE;
    static ITBUFF *mappedchars = NULL;
    
    /* still some pushback left? */
    mapgetc_raw_flag = raw;
    c = mapgetc();

    if (!remap || (c & NOREMAP))
	return (c & ~NOREMAP);

    if (reading_msg_line)
    	mp = 0;
    else if (insertmode)
    	mp = map_insert;
    else 
    	mp = map_command;

    /* if we got a function key from the lower layers, turn it into '#c'
    	and see if the user remapped that */
    if (c & SPEC) {
	mapungetc(kcod2key(c));
	c = poundc;
	speckey = TRUE;
    }

    do {
	itb_init(&mappedchars, abortc);

	matched = maplookup(c, &mappedchars, mp, mapgetc, mapped_c_avail);


	while(itb_more(mappedchars))
	    mapungetc(itb_next(mappedchars));

	/* if the user has not mapped '#c', we return the wide code we got
	    in the first place.  unless they wanted it quoted.  then we
	    leave it as is */
	if (!raw && speckey && !matched) {
	    c = mapgetc() & ~NOREMAP;
	    if (c != poundc)
		    dbgwrite("BUG: # problem in mapped_c");
	    return (mapgetc() & ~NOREMAP) | SPEC;
	}

	c = mapgetc();

	if (!global_g_val(GMDREMAPFIRST))
		matched = FALSE;

	speckey = FALSE;

    } while (matched && remap && !(c & NOREMAP) );

    return c & ~NOREMAP;

}

static int abbr_curr_off;
static int abbr_search_lim;

static int
abbr_getc()
{
    if (abbr_curr_off <= abbr_search_lim)
    	return -1; /* won't match anything in the tree */
    return lgetc(DOT.l, --abbr_curr_off);
}

static int
abbr_c_avail()
{
    return TRUE;
}

void
abbr_check(backsp_limit_p)
int *backsp_limit_p;
{
    int matched;
    ITBUFF *abbr_chars = NULL;
    int status = TRUE;

    if (llength(DOT.l) < 1)
    	return;
    abbr_curr_off = DOT.o;
    abbr_search_lim = *backsp_limit_p;
    itb_init(&abbr_chars, abortc);
    matched = maplookup(abbr_getc(), &abbr_chars, abbr_map,
    	abbr_getc, abbr_c_avail);


    if (matched) {
	    /* there are still some conditions that have to be met by
	       the preceding chars, if any */
	    if (abbr_curr_off > abbr_search_lim) {
		/* we need to check the char in front of the match.
		   if it's a similar type to the first char of the match, 
		   i.e. both idents, or both non-idents, we do nothing.
		   if whitespace precedes either ident or non-ident, the
		   match is good.
		 */
		char first, prev;
		first = lgetc(DOT.l,abbr_curr_off);
		prev = lgetc(DOT.l,abbr_curr_off-1);
		if ((isident(first) && isident(prev)) ||
		    (!isident(first) && !(isident(prev) || isspace(prev))))
		    	return;
	    }
	    DOT.o -= matched;
	    ldelete(matched, FALSE);
	    while(status && itb_more(abbr_chars))
		status = inschar(itb_last(abbr_chars), backsp_limit_p);
    }
    return;
}

/* do map tranlations.
	C is first char to begin mapping on
	OUTP is an ITBUFF in which to put the result
	MP is the map in which to look
	GET is a routine to use to get the next character
	AVAIL is the routine that tells if GET can return something now
  returns number of characters matched
*/
static int
maplookup(c, outp, mp, get, avail)
    int c;
    ITBUFF **outp;
    struct maprec *mp;
    GetFunc get;
    AvailFunc avail;
{
    struct maprec *rmp = NULL;
    int unmatched[MAXLHS];
    register int *s = unmatched;
    int matchedcnt;
    int use_sys_timing;

    /* 
     * we don't want to delay for a user-specified :map!  starting with
     * poundc since it's likely that the mapping is happening on behalf of
     * a function key.  (it's so the user can ":map! #1 foobar" but still be
     * able to insert a '#' character normally.)  if they've changed poundc
     * so it's not something one normally inserts, then it's okay to delay
     * on it.
     */
    use_sys_timing = (insertmode && c == poundc &&
    				(isprint(poundc) || isspace(poundc)));

    *s++ = c;
    matchedcnt = 0;
    while (mp) {
	if (c == mp->ch) {
	    if (mp->irv != -1 || mp->srv != NULL) {
		rmp = mp;
		matchedcnt += (s - unmatched);
		s = unmatched;

		/* our code supports matching the longer of two maps one of
		 * which is a subset of the other.  vi matches the shorter
		 * one.
		 */
	        if (!global_g_val(GMDMAPLONGER))
		    break;
	    }

	    mp = mp->dlink;

	    if (!mp)
		break;

	    /* if there's no recorded input, and no user typeahead */
	    if (!(*avail)()) {

		/* give it a little extra time... */
		int timer = 0;

		/* we want to use the longer of the two timers */

		/* get the user timer.  it may be zero */
		if (!use_sys_timing && (mp->flags & MAPF_USERTIMER) != 0)
			timer = global_g_val(GVAL_TIMEOUTUSERVAL);

		/* if there was no user timer, or this is a system
			sequence, use the system timer if it's bigger */
		if (timer == 0 || (mp->flags & MAPF_SYSTIMER) != 0) {
			if (timer < global_g_val(GVAL_TIMEOUTVAL))
				timer = global_g_val(GVAL_TIMEOUTVAL);
		}

		catnap(timer,TRUE);

		if (!(*avail)())
		    break;
	    }

	    *s++ = c = (*get)() & ~NOREMAP;

	}
	else
	    mp = mp->flink;
    }

    if (rmp) {
	/* unget the unmatched suffix */
	while (s > unmatched)
	    itb_append(outp,*--s);
	/* unget the mapping and elide correct number of recorded chars */
	if (rmp->srv) {
	    int remapflag;
	    char *cp;
	    /* cp = rmp->srv + cnt; */
	    for (cp = rmp->srv; *cp; cp++)
	    	;
	    if (rmp->flags & MAPF_NOREMAP)
		remapflag = NOREMAP;
	    else
		remapflag = 0;
	    while (cp > rmp->srv)
		itb_append(outp, char2int(*--cp)|remapflag);
	}
	else {
	    itb_append(outp, rmp->irv);
	}
	return matchedcnt;
    }
    else {	/* didn't find a match */
	while (s > unmatched)
	    itb_append(outp, *--s);
	return 0;
    }
}
