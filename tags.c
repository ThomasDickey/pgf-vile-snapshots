/* Look up vi-style tags in the file "tags".
 *	Invoked either by ":ta routine-name" or by "^]" while sitting
 *	on a string.  In the latter case, the tag is the word under
 *	the cursor.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Log: tags.c,v $
 * Revision 1.46  1994/02/03 19:35:12  pgf
 * tom's changes for 3.65
 *
 * Revision 1.45  1994/01/03  15:57:34  pgf
 * changed logic of tags lookup, so i can understand it, and to
 * correct logic surrounding "taglength" (again).  if taglength is
 * 0, matches must be exact (i.e.  all characters significant).  if
 * the user enters less than 'taglength' characters, this match must
 * also be exact.  if the user enters 'taglength' or more
 * characters, only that many characters will be significant in the
 * lookup.
 *
 * Revision 1.44  1993/12/21  12:40:26  pgf
 * included missing glob.h
 *
 * Revision 1.43  1993/12/13  18:02:12  pgf
 * the filename found in a tags file is now globbed, so environment
 * variables can be used.
 *
 * Revision 1.42  1993/11/04  09:10:51  pgf
 * tom's 3.63 changes
 *
 * Revision 1.41  1993/10/04  10:24:09  pgf
 * see tom's 3.62 changes
 *
 * Revision 1.40  1993/09/03  09:11:54  pgf
 * tom's 3.60 changes
 *
 * Revision 1.39  1993/08/05  14:29:12  pgf
 * tom's 3.57 changes
 *
 * Revision 1.38  1993/07/01  16:15:54  pgf
 * tom's 3.51 changes
 *
 * Revision 1.37  1993/06/18  15:57:06  pgf
 * tom's 3.49 changes
 *
 * Revision 1.36  1993/06/02  14:28:47  pgf
 * see tom's 3.48 CHANGES
 *
 * Revision 1.35  1993/05/24  15:21:37  pgf
 * tom's 3.47 changes, part a
 *
 * Revision 1.34  1993/05/04  17:05:14  pgf
 * see tom's CHANGES, 3.45
 *
 * Revision 1.33  1993/04/28  17:15:56  pgf
 * got rid of LOOKTAGS mode and ifdefs
 *
 * Revision 1.32  1993/04/20  12:18:32  pgf
 * see tom's 3.43 CHANGES
 *
 * Revision 1.31  1993/04/01  09:46:37  pgf
 * name the tag buffers with "internal" style names, i.e. [Tags nn]
 *
 * Revision 1.30  1993/03/31  19:56:08  pgf
 * partially fix GMDTAGSLOOK code for tags path.  not really "fixed" -- it
 * only looks in first tags file.  i'm thinking of dropping the TAGSLOOK
 * code anyway.
 *
 * Revision 1.29  1993/03/31  19:35:21  pgf
 * implemented tags path
 *
 * Revision 1.28  1993/03/05  17:50:54  pgf
 * see CHANGES, 3.35 section
 *
 * Revision 1.27  1993/01/16  10:42:25  foxharp
 * use new macros
 *
 * Revision 1.26  1992/12/14  09:02:46  foxharp
 * lint cleanup for malloc
 *
 * Revision 1.25  1992/11/19  22:24:25  foxharp
 * recheck tagsprefix on every fetch of file
 *
 * Revision 1.24  1992/11/19  08:49:25  foxharp
 * allow more dynamic killing/creating of the tags buffer, so if you get
 * the wrong tags file by mistake, you can ":kill tags" and retry
 *
 * Revision 1.23  1992/08/05  21:51:10  foxharp
 * use "slash" instead of '/'
 *
 * Revision 1.22  1992/07/30  07:29:19  foxharp
 * further fix off-by-one on tags that use line numbers
 *
 * Revision 1.21  1992/07/15  09:00:35  foxharp
 * fixed off-by-one in tags-buffer line scan, so single digit line
 * numbers now work, e.g. "file.c	src/file.c	1".
 * also, always set up "tagprefix", in case we switch to tagsrelative
 * mode later on
 *
 * Revision 1.20  1992/07/13  20:09:27  foxharp
 * path-relative tags are now controlled by a boolean mode "set tagsrelative"
 *
 * Revision 1.19  1992/07/13  19:45:51  foxharp
 * added "tags relative to where we found the tags file" code
 *
 * Revision 1.18  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.17  1992/03/19  23:26:23  pgf
 * removed extra string lib externs
 *
 * Revision 1.16  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.15  1991/11/08  13:07:09  pgf
 * ifdefed out lazy filename matching
 *
 * Revision 1.14  1991/11/07  02:00:32  pgf
 * lint cleanup
 *
 * Revision 1.13  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.12  1991/10/27  01:53:15  pgf
 * use global taglen value for command line tags -- there's no current
 * buffer yet
 *
 * Revision 1.11  1991/10/22  03:09:32  pgf
 * tags given on the command line now set the response for further tag commands
 *
 * Revision 1.10  1991/10/20  23:06:43  pgf
 * cleaned up taglen stuff
 *
 * Revision 1.9  1991/10/15  11:58:58  pgf
 * added taglength support
 *
 * Revision 1.8  1991/10/08  01:30:00  pgf
 * added new bp arg to lfree and lalloc
 *
 * Revision 1.7  1991/10/08  01:26:33  pgf
 * untagpop now sets "lastdot" correctly
 *
 * Revision 1.6  1991/09/19  13:44:13  pgf
 * tags file is looked up via VAL_TAGS setting (the global one -- local tags
 * paths and files don't make sense.  yet? )
 *
 * Revision 1.5  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.4
 * date: 1991/06/25 19:53:33;
 * massive data structure restructure
 * 
 * revision 1.3
 * date: 1991/06/07 13:23:30;
 * don't move "last dot" mark if dot doesn't change
 * 
 * revision 1.2
 * date: 1991/04/08 15:47:17;
 * fixed readin() arg count
 * 
 * revision 1.1
 * date: 1990/09/21 10:26:07;
 * initial vile RCS revision
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
static	LINE *	cheap_buffer_scan P(( BUFFER *, char *, SIZE_T));
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
	(void)strncpy(tagname,t,NFILEN);
	tagname[NFILEN-1] = EOS;
	return tags(tagname, global_b_val(VAL_TAGLEN));
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
				TTbeep();
				mlforce("[No such tag: \"%s\"]",tag);
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

	i = 0;
	if (b_val(curbp,MDTAGSRELTIV) && !slashc(*tfp)) {
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
		lineno = DOT.l->l_number;
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
		i = 0;
		tfp += 2; /* skip the "/^" */
		lplim -= 2; /* skip the "$/" */
		while (i < sizeof(srchpat) && tfp < lplim) {
			if (*tfp == '\\' && tfp < lplim - 1)
				tfp++;  /* the backslash escapes next char */
			srchpat[i++] = *tfp++;
		}
		srchpat[i] = EOS;
		lp = cheap_buffer_scan(curbp, srchpat, (SIZE_T)i);
		if (lp == NULL) {
			mlforce("[Tag not present]");
			TTbeep();
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
	int namelen = strlen(name);
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
cheap_buffer_scan(bp, patrn, len)
BUFFER *bp;
char *patrn;
SIZE_T len;
{
	register LINE *lp;

	len = strlen(patrn);

	for_each_line(lp, bp) {
		if (llength(lp) == len) {
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
	TTbeep();
	mlforce("[No stacked un-tags]");
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
