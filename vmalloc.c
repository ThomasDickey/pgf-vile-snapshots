#include "estruct.h"
#include "edef.h"
#if ! SMALLER && LATER
# include "evar.h"
#endif

/* these routines copied without permission from "The C User's Journal",
 *	issue of Feb. 1989.  I assume they are Copyright 1989 by them.
 *	They and the accompanying article were written by Eric White.
 *	(pgf, 1989)
 *
 * $Log: vmalloc.c,v $
 * Revision 1.5  1992/03/05 09:19:55  pgf
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

#include "stdio.h"
#include "string.h"

char *malloc(), *calloc(), *realloc();

char *vmalloc();
void vfree();
void rvverify();
char *vrealloc();
char *vcalloc();
void vdump();

typedef unsigned long ulong;

/* max buffers alloced but not yet freed */
#define MAXMALLOCS 20000

/* known pattern, and how many of them */
#define KP 0xaaaaaaaaL
#define KPW (2*sizeof(unsigned long))


static void trace();
static void errout();

static int nummallocs = 0;
struct mtype {
	unsigned char *addr;
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
	unsigned char *c;
	char s [80];
	c = (unsigned char *)m[x].addr - 2;
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
{
	char s[80];
	register int c;
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
char *f;
{
	unsigned char *buffer;
	char *sp, s[80];
	register int c;
	register struct mtype *mp;

	if (doverifys & VMAL)
		rvverify("vmalloc",f,l);
	if (( buffer = (unsigned char *)malloc(size + KPW + KPW)) == NULL) {
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
int n, size;
char *f;
{
	return vmalloc(n * size,f,l);
}

void
vfree(buffer,f,l)
unsigned char *buffer;
char *f;
{
	unsigned char *b;
	char s[80], *sp;
	register struct mtype *mp;

	b = buffer - KPW;
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
unsigned char *buffer;
int size;
char *f;
{
	unsigned char *b, *b2;
	char *sp, s[80];
	register int c;
	register struct mtype *mp;

	b = buffer - KPW;
	if (doverifys & VREA)
		rvverify("vrealloc",f,l);

	for (mp = &m[nummallocs-1]; mp >= m && mp->addr != b; mp--)
		;
	if (mp < m) {
		sprintf(s,"ERROR: location to realloc is not in list. %s %d\n",
					 sp,f,l);
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
	b2 = (unsigned char *)realloc(b,size+KPW+KPW);
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
	kill(getpid(),3);
	pause();
}

setvmalloc(f,n)
int f,n;
{
	register struct mtype *mp;
	int i,num,found;
	
	if (f)
		doverifys = n;
	rvverify("requested",__FILE__,__LINE__);
	for (mp = m, num = 0; mp < &m[MAXMALLOCS]; ++mp) {
		if (mp->addr != NULL)
			num++;
	}
	found = 0;
	{ /* windows */
		register WINDOW *wp;
		for (wp=wheadp; wp != NULL; wp = wp->w_wndp)
			found++;
	}
	{ /* buffers */
		register BUFFER *bp;
		for (bp=bheadp; bp != NULL; bp = bp->b_bufp) {
			LINE *lp;
			found++; /* for b_linep */
			for(lp = bp->b_line.l; lp->l_fp != bp->b_line.l;
								lp = lp->l_fp)
				found++;
			if (bp->b_nmmarks)
				found++;
			if (bp->b_ulinep)
				found++;
			found++;  /* for the buffer itself */
			for (i = 0; i < 2; i++) {
				for (lp = bp->b_udstks[i]; lp != NULL;
							lp = lp->l_nxtundo)
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
		register MC	*mcptr;

		if (patmatch)
			found++;
			
		mcptr = &mcpat[0];
		while (mcptr->mc_type != MCNIL)
		{
			if ((mcptr->mc_type & MASKCL) == CCL ||
			    (mcptr->mc_type & MASKCL) == NCCL)
				if (mcptr->u.cclmap) found++;
			mcptr++;
		}
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
	return TRUE;
}

#endif
