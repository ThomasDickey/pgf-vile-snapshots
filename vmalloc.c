#include "estruct.h"
#include "edef.h"
#if ! SMALLER && LATER
# include "nevars.h"
#endif

/* these routines copied without permission from "The C User's Journal",
 *	issue of Feb. 1989.  I assume they are Copyright 1989 by them.
 *	They and the accompanying article were written by Eric White.
 *	(pgf, 1989)
 *
 * $Log: vmalloc.c,v $
 * Revision 1.14  1993/07/01 16:15:54  pgf
 * tom's 3.51 changes
 *
 * Revision 1.13  1993/05/24  15:21:37  pgf
 * tom's 3.47 changes, part a
 *
 * Revision 1.12  1993/05/11  16:22:22  pgf
 * see tom's CHANGES, 3.46
 *
 * Revision 1.11  1993/04/28  14:34:11  pgf
 * see CHANGES, 3.44 (tom)
 *
 * Revision 1.10  1993/04/20  12:18:32  pgf
 * see tom's 3.43 CHANGES
 *
 * Revision 1.9  1993/01/23  13:38:23  foxharp
 * evar.h is now nevars.h
 *
 * Revision 1.8  1993/01/16  10:43:22  foxharp
 * use new macros
 *
 * Revision 1.7  1992/07/22  00:51:35  foxharp
 * took out the counters -- they no longer compile correctly
 *
 * Revision 1.6  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.5  1992/03/05  09:19:55  pgf
 * changed some mlwrite() to mlforce(), due to new terse support
 *
 * Revision 1.4  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.3  1991/10/08  01:30:59  pgf
 * brought up to date, and ifdef LATER'd some stuff in
 * the accounting section -- doesn't work due to header inclusion
 * problems
 *
 * Revision 1.2  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.1
 * date: 1990/09/21 10:26:17;
 * initial vile RCS revision
 */

#if VMALLOC

#undef malloc
#undef free
#undef realloc
#undef calloc
#undef vverify

/* max buffers alloc'ed but not yet free'd */
#if TURBO
#define MAXMALLOCS 1000	/* sorry, not very big ! */
#else
#define MAXMALLOCS 20000
#endif

#define	uchar	unsigned char
#define	ulong	unsigned long

/* known pattern, and how many of them */
#define KP 0xaaaaaaaaL
#define KPW (2*sizeof(ulong))

static void dumpbuf P(( int ));
static void trace P(( char * ));
static void errout P(( void ));

static int nummallocs = 0;
struct mtype {
	uchar *addr;
	int size;
};

static struct mtype m[MAXMALLOCS];

#define VMAL 1
#define VFRE 2
#define VREA 4
int doverifys = VMAL|VREA;  /* |VFRE */

static void
dumpbuf(x)
int x;
{
	uchar *c;
	char s [80];
	c = (uchar *)m[x].addr - 2;
	/* dump malloc buffer to the vmalloc file */
	while (c <= m[x].addr + m[x].size + KPW + KPW + 1) {
		sprintf(s, "%04.4lx : %02x ", (long)c, *c);
		if (c == m[x].addr)
			strcat(s," <= leading known pattern");
		if (c == m[x].addr + KPW)
			strcat(s," <= addr of malloc buffer");
		if (c == m[x].addr + m[x].size + KPW)
			strcat(s," <= trailing known pattern");
		strcat(s,"\n");
		trace(s);
		++c;
	}
}
		
void
rvverify(id,f,l)
char *id;
char *f;
int l;
{
	char s[80];
	register struct mtype *mp;

	
	/* verify entire malloc heap */
	for (mp = &m[nummallocs-1]; mp >= m; mp--) {
		if (mp->addr != NULL) {
			if (*(ulong *)mp->addr != KP ||
				*(ulong *)(mp->addr + sizeof (ulong)) != KP)
			{
				sprintf(s, 
		"ERROR: Malloc area corrupted (%s). %s %d\n",
							 id,f,l);
				fputs(s,stderr);
				trace(s);
				dumpbuf(mp - m);
				errout();
			}
		}
	}
}

char *
vmalloc(size,f,l)
SIZE_T	size;
char	*f;
int	l;
{
#ifdef VERBOSE
	char s[80];
#endif
	uchar *buffer;
	char *sp;
	register struct mtype *mp;

	if (doverifys & VMAL)
		rvverify("vmalloc",f,l);
	if (( buffer = (uchar *)malloc(size + KPW + KPW)) == NULL) {
		sp = "ERROR: real malloc returned NULL\n";
		fprintf(stderr,sp);
		trace(sp);
		errout();
	}
#ifdef VERBOSE
	sprintf(s,"%04.4lx:vmalloc size = %ld, %s %d\n",
		(long)buffer,(long)size,f,l);
	trace(s);
#endif
	/* find a place for an entry in m */
	for (mp = m; mp < &m[MAXMALLOCS] && mp->addr != NULL; ++mp)
		;
	if (mp == &m[MAXMALLOCS]) {
		sp = "ERROR: too many mallocs\n";
		fprintf(stderr,sp);
		trace(sp);
		errout();
	}
	mp->addr = buffer;
	mp->size = size;
	if (mp == &m[nummallocs])
		++nummallocs;
	*(ulong *)(mp->addr) = KP;
	*(ulong *)(mp->addr + sizeof(ulong)) = KP;
	return (char *)(buffer + KPW);
}

char *
vcalloc(n,size,f,l)
int	n;
SIZE_T	size;
char	*f;
int	l;
{
	return vmalloc(n * size,f,l);
}

void
vfree(buffer,f,l)
char	*buffer;
char	*f;
int	l;
{
#ifdef VERBOSE
	char *sp;
#endif
	char s[80];
	uchar *b;
	register struct mtype *mp;

	b = (uchar *)(buffer - KPW);
	if (doverifys & VFRE)
		rvverify("vfree",f,l);
	for (mp = &m[nummallocs-1]; mp >= m && mp->addr != b; mp--)
		;
	if (mp < m) {
		sprintf(s,"ERROR: location to free is not in list. %s %d\n",
					 f,l);
		fprintf(stderr,s);
		trace(s);
		errout();
	}
#ifdef VERBOSE
	sprintf(s,"%04.4lx:vfree %s %d\n",(long)b,f,l);
	trace(s);
#endif
	if (*(ulong *)mp->addr != KP || 
		*(ulong *)(mp->addr + sizeof (ulong)) != KP)
	{
		sprintf(s,"ERROR: corrupted freed block. %s %d\n", f,l);
		fprintf(stderr,s);
		trace(s);
		errout();
	}
	free(b);
	mp->addr = NULL;
	if (mp == &m[nummallocs-1])
		--nummallocs;
}

char *
vrealloc(buffer,size,f,l)
char	*buffer;
SIZE_T	size;
char	*f;
int	l;
{
	uchar *b, *b2;
	char s[80];
	register struct mtype *mp;

	b = (uchar *)(buffer - KPW);
	if (doverifys & VREA)
		rvverify("vrealloc",f,l);

	for (mp = &m[nummallocs-1]; mp >= m && mp->addr != b; mp--)
		;
	if (mp < m) {
		sprintf(s,"ERROR: location to realloc is not in list. %s %d\n",
					 f,l);
		fprintf(stderr,s);
		trace(s);
		errout();
	}

#ifdef VERBOSE
	sprintf(s,"%04.4lx:vrealloc size = %ld, %s %d\n",
			(long)b,(long)size,f,l);
	trace(s);
#endif
	*(ulong *)(mp->addr) = KP;
	*(ulong *)(mp->addr + sizeof (ulong)) = KP;
	b2 = (uchar *)realloc(b,size+KPW+KPW);
	*(ulong *)(mp->addr + mp->size + KPW) = KP;
	*(ulong *)(mp->addr + mp->size + KPW + sizeof (ulong)) = KP;
	return (char *)(b2 + KPW);
}

void
vdump(id)
char *id;
{
	char s[80];
	int x;
	sprintf(s,"=============Dump of malloc heap==========%s\n",id);
	trace(s);
	for (x = 0; x < nummallocs; ++x) {
		if (m[x].addr != NULL) {
			sprintf(s,"=========malloc buffer addr: %04.4lx\n",
				(long)m[x].addr);
			trace(s);
			sprintf(s,"=========malloc buffer size: %04x\n",
				(long)m[x].size + KPW + KPW);
			trace(s);
			dumpbuf(x);
		}
	}
}

static void
trace(s)
char *s;
{
	static FILE *out = NULL;
	if (out == NULL) {
		unlink("vmalloc.log");
		out = fopen("vmalloc.log", "w");
		setbuf(out,NULL);
	}
	fputs(s,out);
}
	
static void
errout()
{
	sleep(1);
#if UNIX
	kill(getpid(),3);
	pause();
#endif
}

int
setvmalloc(f,n)
int f,n;
{
	register struct mtype *mp;
	int i,num,found;
	
	if (f)
		doverifys = n;
	rvverify("requested",__FILE__,__LINE__);
#if COUNT_THEM
	for (mp = m, num = 0; mp < &m[MAXMALLOCS]; ++mp) {
		if (mp->addr != NULL)
			num++;
	}
	found = 0;
	{ /* windows */
		register WINDOW *wp;
		for_each_window(wp)
			found++;
	}
	{ /* buffers */
		register BUFFER *bp;
		for_each_buffer(bp) {
			LINE *lp;
			found++; /* for b_linep */
			for_each_line(lp, bp)
				found++;
			if (bp->b_nmmarks)
				found++;
			if (bp->b_ulinep)
				found++;
			found++;  /* for the buffer itself */
			for (i = 0; i < 2; i++) {
				for (lp = l_ref(bp->b_udstks[i]); lp != NULL;
							lp = l_ref(lp->l_nxtundo))
					found++;
			}
		}
	}
	found += term.t_mrow+1;  /* vscreen and the rows */
#if ! MEMMAP
	found += term.t_mrow+1;  /* pscreen and the rows */
#endif
	if (fline)
		found++;
#if ! SMALLER && LATER
	{ /* user vars */
		extern UVAR uv[MAXVARS];
		for (i=0; i < MAXVARS; i++)
			if (uv[i].u_value) found++;
	}
#endif
#if	FILOCK
	need to count lock mallocs...
#endif
	{ /* searching */
#if UNUSED
		register MC	*mcptr;

		mcptr = &mcpat[0];
		while (mcptr->mc_type != MCNIL)
		{
			if ((mcptr->mc_type & MASKCL) == CCL ||
			    (mcptr->mc_type & MASKCL) == NCCL)
				if (mcptr->u.cclmap) found++;
			mcptr++;
		}
#endif
		if (patmatch)
			found++; 	
	}
	{ /* kill registers */
		for (i = 0; i < NKREGS; i++) {
			KILL *kb;
			if ((kb = kbs[i].kbufh) != NULL) {
				while (kb) {
					found++;
					kb = kb->d_next;
				}
			}
		}
	}
	mlforce("doverifys %s %d, outstanding mallocs: %d, %d accounted for.",
		f ? "set to":"is still", doverifys, num, found);
#else
	mlforce("doverifys %s %d",
		f ? "set to":"is still", doverifys);
#endif
	return TRUE;
}

#endif
