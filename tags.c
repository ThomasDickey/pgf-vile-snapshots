/* Look up vi-style tags in the file "tags".
 *	Invoked either by ":ta routine-name" or by "^]" while sitting
 *	on a string.  In the latter case, the tag is the word under
 *	the cursor.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/tags.c,v 1.58 1994/08/08 16:12:29 pgf Exp $
 *
 */
#include	"estruct.h"
#include        "edef.h"

#if TAGS

#define	TAGS_LIST_NAME	ScratchName(Tag Stack)

#define	UNTAG	struct	untag
	UNTAG {
	char *u_fname;
	int u_lineno;
	UNTAG *u_stklink;
#if OPT_SHOW_TAGS
	char	*u_templ;
#endif
};


static	LINE *	cheap_tag_scan P(( BUFFER *, char *, SIZE_T));
static	LINE *	cheap_buffer_scan P(( BUFFER *, char *, SIZE_T, int));
static	void	free_untag P(( UNTAG * ));
static	BUFFER *gettagsfile P(( int, int * ));
static	void	nth_name P(( char *,  char *, int ));
static	int	popuntag P(( char *, int * ));
static	void	pushuntag P(( char *, int, char * ));
static	void	tossuntag P(( void ));

static	UNTAG *	untaghead = NULL;
static	char	tagname[NFILEN+2];  /* +2 since we may add a tab later */

/* ARGSUSED */
int
gototag(f,n)
int f,n;
{
	register int s;
	int taglen;

	if (clexec || isnamedcmd) {
	        if ((s=mlreply("Tag name: ", tagname, NFILEN)) != TRUE)
	                return (s);
		taglen = b_val(curbp,VAL_TAGLEN);
	} else {
		s = screen_string(tagname, NFILEN, _ident);
		taglen = 0;
	}
	if (s == TRUE)
		s = tags(tagname,taglen);
	return s;
}

int
cmdlinetag(t)
char *t;
{
	return tags(strncpy0(tagname, t, NFILEN), global_b_val(VAL_TAGLEN));
}


int
tags(tag,taglen)
char *tag;
int taglen;
{
	register LINE *lp;
	register int i, s;
	char *tfp, *lplim;
	char tfname[NFILEN];
	char srchpat[NPAT];
	int lineno;
	int changedfile;
	MARK odot;
	BUFFER *tagbp;
	int nomore;
	int gotafile = FALSE;

	i = 0;
	do {
		tagbp = gettagsfile(i, &nomore);
		if (nomore) {
			if (gotafile) {
				mlwarn("[No such tag: \"%s\"]",tag);
			} else {
				mlforce("[No tags file available.]");
			}
			return FALSE;
		}

		if (tagbp) {
			lp = cheap_tag_scan(tagbp, tag, (SIZE_T)taglen);
			gotafile = TRUE;
		} else {
			lp = NULL;
		}

		i++;

	} while (lp == NULL);
	
	lplim = &lp->l_text[lp->l_used];
	tfp = lp->l_text;
	while (tfp < lplim)
		if (*tfp++ == '\t')
			break;
	if (*tfp == '\t') { /* then it's a new-fangled NeXT tags file */
		tfp++;  /* skip the tab */
	}

	i = 0;
	if (b_val(curbp,MDTAGSRELTIV) && !is_slashc(*tfp)) {
		register char *first = tagbp->b_fname;
		char *lastsl = pathleaf(tagbp->b_fname);
		while (lastsl != first)
			tfname[i++] = *first++;
	}
	while (i < sizeof(tfname) && tfp < lplim && *tfp != '\t') {
		tfname[i++] = *tfp++;
	}
	tfname[i] = EOS;

	if (tfp >= lplim) {
		mlforce("[Bad line in tags file.]");
		return FALSE;
	}

	if (curbp && curwp) {
#if SMALLER
		register LINE *clp;
		lineno = 1;
	        for(clp = lForw(buf_head(curbp)); 
				clp != l_ref(DOT.l); clp = lforw(clp))
			lineno++;
#else
		bsizes(curbp);
		lineno = l_ref(DOT.l)->l_number;
#endif
		pushuntag(curbp->b_fname, lineno, tag);
	}

	if (curbp == NULL
	 || !same_fname(tfname, curbp, TRUE)) {
		(void) doglob(tfname);
		s = getfile(tfname,TRUE);
		if (s != TRUE) {
			tossuntag();
			return s;
		}
		changedfile = TRUE;
	} else {
		mlwrite("Tag \"%s\" in current buffer", tag);
		changedfile = FALSE;
	}

	/* it's an absolute move -- remember where we are */
	odot = DOT;

	tfp++;  /* skip the tab */
	if (tfp >= lplim) {
		mlforce("[Bad line in tags file.]");
		return FALSE;
	}
	if (isdigit(*tfp)) { /* then it's a line number */
		lineno = 0;
		while (isdigit(*tfp) && (tfp < lplim)) {
			lineno = 10*lineno + *tfp - '0';
			tfp++;
		}
		s = gotoline(TRUE,lineno);
		if (s != TRUE && !changedfile)
			tossuntag();
	} else {
		int exact;
		int delim = *tfp;
		int quoted = FALSE;
		char *p;

		tfp += 2; /* skip the "/^" */
		p = tfp+1;

		/* we're on the '/', so look for the matching one */
		while (p < lplim) {
			if (quoted) {
				quoted = FALSE;
			} else if (*p == '\\') {
				quoted = TRUE;
			} else if (*p == delim) {
				break;
			}
			p++;
		}
		if (p >= lplim) {
			mlforce("[Bad pattern in tags file.]");
			return FALSE;
		}
		if (p[-1] == '$') {
			exact = TRUE;
			p--;
		} else {
			exact = FALSE;
		}
		lplim = p;
		i = 0;
		while (i < sizeof(srchpat) && tfp < lplim) {
			if (*tfp == '\\' && tfp < lplim - 1)
				tfp++;  /* the backslash escapes next char */
			srchpat[i++] = *tfp++;
		}
		srchpat[i] = EOS;
		lp = cheap_buffer_scan(curbp, srchpat, (SIZE_T)i, exact);
		if (lp == NULL) {
			mlwarn("[Tag not present]");
			if (!changedfile)
				tossuntag();
			return FALSE;
		}
		DOT.l = l_ptr(lp);
		curwp->w_flag |= WFMOVE;
		(void)firstnonwhite(FALSE,1);
		s = TRUE;
	}
	/* if we moved, update the "last dot" mark */
	if (s == TRUE && !sameline(DOT, odot)) {
		curwp->w_lastdot = odot;
	}
	return s;
	
}

/* 
 * return (in buf) the Nth whitespace 
 *	separated word in "path", counting from 0
 */
static void
nth_name(buf, path, n)
char *buf;
char *path;
int n;
{
	while (n-- > 0) {
		while (*path &&  isspace(*path)) path++;
		while (*path && !isspace(*path)) path++;
	}
	while (*path &&  isspace(*path)) path++;
	while (*path && !isspace(*path)) *buf++ = *path++;
	*buf = EOS;
}


static BUFFER *
gettagsfile(n, endofpathflagp)
int n;
int *endofpathflagp;
{
	char *tagsfile;
	BUFFER *tagbp;
	static char tagbufname[NBUFN+1];
	char tagfilename[NFILEN];

	*endofpathflagp = FALSE;
	
	(void)lsprintf(tagbufname, ScratchName(Tags %d), n+1);

	/* is the buffer around? */
	if ((tagbp=find_b_name(tagbufname)) == NULL) {
		char *tagf = global_b_val_ptr(VAL_TAGS);

		nth_name(tagfilename, tagf, n);
		if (tagfilename[0] == EOS) {
			*endofpathflagp = TRUE;
			return NULL;
		}

		/* look up the tags file */
		tagsfile = flook(tagfilename, FL_HERE);

		/* if it isn't around, don't sweat it */
		if (tagsfile == NULL)
		{
			return NULL;
		}

		/* find the pointer to that buffer */
		if ((tagbp=bfind(tagf, BFINVS)) == NULL) {
			mlforce("[Can't create tags buffer]");
			return NULL;
		}

		if (readin(tagsfile, FALSE, tagbp, FALSE) != TRUE) {
			return NULL;
		}
		/* be sure it has the right name */
		set_bname(tagbp, tagbufname);
		b_set_invisible(tagbp);
			
        }
	return tagbp;
}

/*
 * Do exact/inexact lookup of an anchored string in a buffer.
 *	if taglen is 0, matches must be exact (i.e.  all
 *	characters significant).  if the user enters less than 'taglen'
 *	characters, this match must also be exact.  if the user enters
 *	'taglen' or more characters, only that many characters will be
 *	significant in the lookup.
 */
static LINE *
cheap_tag_scan(bp, name, taglen)
BUFFER *bp;
char *name;
SIZE_T taglen;
{
	register LINE *lp,*retlp;
	SIZE_T namelen = strlen(name);
	int exact = (taglen == 0);
	int added_tab;

	/* force a match of the tab delimiter if we're supposed to do
		exact matches or if we're searching for something shorter
		than the "restricted" length */
	if (exact || namelen < taglen) {
		name[namelen++] = '\t';
		name[namelen] = EOS;
		added_tab = TRUE;
	} else {
		added_tab = FALSE;
	}

	retlp = NULL;
	for_each_line(lp, bp) {
		if (llength(lp) > namelen) {
			if (!strncmp(lp->l_text, name, namelen)) {
				retlp = lp;
				break;
			}
		}
	}
	if (added_tab)
		name[namelen-1] = EOS;
	return retlp;
}

static LINE *
cheap_buffer_scan(bp, patrn, len, exact)
BUFFER *bp;
char *patrn;
SIZE_T len;
int exact;
{
	register LINE *lp;

	len = strlen(patrn);

	for_each_line(lp, bp) {
		if ((exact && llength(lp) == len) || (!exact && llength(lp) >= len)) {
			if (!strncmp(lp->l_text, patrn, len)) {
				return lp;
			}
		}
	}
	return NULL;
}

int
untagpop(f,n)
int f,n;
{
	int lineno;
	char fname[NFILEN];
	MARK odot;

	if (!f) n = 1;
	while (n && popuntag(fname,&lineno))
		n--;
	if (lineno && fname[0]) {
		int s;
		s = getfile(fname,FALSE);
		if (s != TRUE)
			return s;

		/* it's an absolute move -- remember where we are */
		odot = DOT;
		s = gotoline(TRUE,lineno);
		/* if we moved, update the "last dot" mark */
		if (s == TRUE && !sameline(DOT, odot)) {
			curwp->w_lastdot = odot;
		}
		return s;
	}
	mlwarn("[No stacked un-tags]");
	return FALSE;
}


static void
free_untag(utp)
UNTAG	*utp;
{
	FreeIfNeeded(utp->u_fname);
#if OPT_SHOW_TAGS
	FreeIfNeeded(utp->u_templ);
#endif
	free((char *)utp);
}


/*ARGSUSED*/
static void
pushuntag(fname,lineno,tag)
char *fname;
int lineno;
char *tag;
{
	UNTAG *utp;
	utp = typealloc(UNTAG);
	if (!utp)
		return;

	if ((utp->u_fname = strmalloc(fname)) == 0
#if OPT_SHOW_TAGS
	 || (utp->u_templ = strmalloc(tag)) == 0
#endif
	   ) {
		free_untag(utp);
		return;
	}

	utp->u_lineno = lineno;
	utp->u_stklink = untaghead;
	untaghead = utp;
}


static int
popuntag(fname,linenop)
char *fname;
int *linenop;
{
	register UNTAG *utp;

	if (untaghead) {
		utp = untaghead;
		untaghead = utp->u_stklink;
		(void)strcpy(fname, utp->u_fname);
		*linenop = utp->u_lineno;
		free_untag(utp);
		return TRUE;
	}
	fname[0] = EOS;
	*linenop = 0;
	return FALSE;

}

/* discard without returning anything */
static void
tossuntag()
{
	register UNTAG *utp;

	if (untaghead) {
		utp = untaghead;
		untaghead = utp->u_stklink;
		free_untag(utp);
	}
}

#if OPT_SHOW_TAGS
static	void	maketagslist P(( int, char * ));

/*ARGSUSED*/
static void
maketagslist (value, ptr)
int	value;
char	*ptr;
{
	register UNTAG *utp;
	register int	n;
	int	taglen = global_b_val(VAL_TAGLEN);
	char	temp[NFILEN];

	if (taglen == 0) {
		for (utp = untaghead; utp != 0; utp = utp->u_stklink) {
			n = strlen(utp->u_templ);
			if (n > taglen)
				taglen = n;
		}
	}
	if (taglen < 10)
		taglen = 10;

	bprintf("    %*s FROM line in file\n", taglen, "TO tag");
	bprintf("    %*p --------- %30p",      taglen, '-', '-');

	for (utp = untaghead, n = 0; utp != 0; utp = utp->u_stklink)
		bprintf("\n %2d %*s %8d  %s",
			++n,
			taglen, utp->u_templ,
			utp->u_lineno,
			shorten_path(strcpy(temp, utp->u_fname), TRUE));
}


/*
 * Display the contents of the tag-stack
 */
/*ARGSUSED*/
int
showtagstack(f,n)
int	f,n;
{
	return liststuff(TAGS_LIST_NAME, maketagslist, f, (char *)0);
}
#endif	/* OPT_SHOW_TAGS */

#endif	/* TAGS */
