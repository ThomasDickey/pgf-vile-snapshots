/* Look up vi-style tags in the file "tags".
 *	Invoked either by ":ta routine-name" or by "^]" while sitting
 *	on a string.  In the latter case, the tag is the word under
 *	the cursor.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Log: tags.c,v $
 * Revision 1.31  1993/04/01 09:46:37  pgf
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

static char tagname[NFILEN];

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
		s = screen_string(tagname,NFILEN,_ident);
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
	(void)strcpy(tagname,t);
	return tags(tagname, global_b_val(VAL_TAGLEN));
}


int
tags(tag,taglen)
char *tag;
int taglen;
{
	register LINE *lp, *clp;
	register int i, s;
	char *tfp, *lplim;
	char tname[NFILEN];
	char tfname[NFILEN];
	char tagpat[NPAT];
	int lineno;
	int changedfile;
	MARK odot;
	LINE *cheap_scan();
	BUFFER *tagbp;
	int nomore;
	int gotafile = FALSE;

	(void)strcpy(tname,tag);

	i = 0;
	do {
		tagbp = gettagsfile(i, &nomore);
		if (nomore) {
			if (gotafile) {
				TTbeep();
				mlforce("[No such tag: \"%s\"]",tname);
			} else {
				mlforce("[No tags file available.]");
			}
			return FALSE;
		}

		if (tagbp) {
			lp = cheap_scan(tagbp, tname, taglen ? 
					taglen : strlen(tname));
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
	if (b_val(curbp,MDTAGSRELTIV) && *tfp != '/') {
		char *lastsl;
		if ((lastsl = strrchr(tagbp->b_fname,'/')) != NULL) {
			i = lastsl - tagbp->b_fname + 1;
			(void)strncpy(tfname, tagbp->b_fname, i);
		}
	}
	while (i < NFILEN && tfp < lplim && *tfp != '\t') {
		tfname[i++] = *tfp++;
	}
	tfname[i] = '\0';

	if (tfp >= lplim) {
		mlforce("[Bad line in tags file.]");
		return FALSE;
	}

	if (curbp && curwp) {
		lineno = 1;
	        for(clp = lforw(curbp->b_line.l); 
				clp != curwp->w_dot.l; clp = lforw(clp))
			lineno++;
		pushuntag(curbp->b_fname, lineno);
	}

	if (curbp == NULL || strcmp(tfname,curbp->b_fname)) {
		s = getfile(tfname,TRUE);
		if (s != TRUE) {
			tossuntag();
			return s;
		}
		changedfile = TRUE;
	} else {
		if (tname[strlen(tname)-1] == '\t')
			tname[strlen(tname)-1] = '\0'; /* get rid of tab we added */
		mlwrite("Tag \"%s\" in current buffer", tname);
		changedfile = FALSE;
	}

	/* it's an absolute move -- remember where we are */
	odot = DOT;

	i = 0;
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
		tfp += 2; /* skip the "/^" */
		lplim -= 2; /* skip the "$/" */
		while (i < NPAT && tfp < lplim) {
			if (*tfp == '\\' && tfp < lplim - 1)
				tfp++;  /* the backslash escapes the next char */
			tagpat[i++] = *tfp++;
		}
		tagpat[i] = 0;
		lp = cheap_scan(curbp,tagpat,i);
		if (lp == NULL) {
			mlforce("[Tag not present]");
			TTbeep();
			if (!changedfile)
				tossuntag();
			return FALSE;
		}
		curwp->w_dot.l = lp;
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
void
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
	*buf = '\0';
}


BUFFER *
gettagsfile(n, endofpathflagp)
int n;
int *endofpathflagp;
{
	char *tagsfile;
	BUFFER *tagbp;
	static char tagbufname[NBUFN];
	char tagfilename[NFILEN];

	*endofpathflagp = FALSE;
	
	lsprintf(tagbufname, ScratchName(Tags %d), n+1);

	/* is the buffer around? */
        if ((tagbp=bfind(tagbufname, NO_CREAT, 0)) == NULL) {
		char *tagf = global_b_val_ptr(VAL_TAGS);

		nth_name(tagfilename, tagf, n);
		if (tagfilename[0] == '\0') {
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
		if ((tagbp=bfind(tagf, OK_CREAT, BFINVS)) == NULL) {
			mlforce("[Can't create tags buffer]");
			return NULL;
		}

		if (readin(tagsfile, FALSE, tagbp, FALSE) != TRUE) {
			return NULL;
		}
		/* be sure it has the right name */
		(void)strcpy(tagbp->b_bname, tagbufname);
		tagbp->b_flag |= BFINVS;
			
        }
	return tagbp;
}

LINE *
cheap_scan(bp,name,len)
BUFFER *bp;
char *name;
int len;
{
	LINE *lp;
	lp = lforw(bp->b_line.l);
	while (lp != bp->b_line.l) {
		if (llength(lp) >= len) {
			if (llength(lp) >= len &&
				 !strncmp(lp->l_text, name, len))
				return lp;
		}
		lp = lforw(lp);
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


struct untag {
	char *u_fname;
	int u_lineno;
	struct untag *u_stklink;
};

struct untag *untaghead = NULL;

void
pushuntag(fname,lineno)
char *fname;
int lineno;
{
	struct untag *utp;
	utp = typealloc(struct untag);
	if (!utp)
		return;

	utp->u_fname = castalloc(char, strlen(fname)+1);
	if (!utp->u_fname) {
		free((char *)utp);
		return;
	}

	(void)strcpy(utp->u_fname, fname);
	utp->u_lineno = lineno;
	utp->u_stklink = untaghead;
	untaghead = utp;
}

int
popuntag(fname,linenop)
char *fname;
int *linenop;
{
	register struct untag *utp;

	if (untaghead) {
		utp = untaghead;
		untaghead = utp->u_stklink;
		(void)strcpy(fname, utp->u_fname);
		*linenop = utp->u_lineno;
		free(utp->u_fname);
		free((char *)utp);
		return TRUE;
	}
	fname[0] = '\0';
	*linenop = 0;
	return FALSE;

}

/* discard without returning anything */
void
tossuntag()
{
	register struct untag *utp;

	if (untaghead) {
		utp = untaghead;
		untaghead = utp->u_stklink;
		free((char *)utp);
	}
}

#ifdef GMDTAGSLOOK

/* create a filelist from the contents of the tags file. */
/* patch: when does the buffer get reset? */
/* patch: only scans the first file in the tags path. */

BUFFER *
look_tags(sequence)
int	sequence;
{
	register LINE *tlp;
	register char *fnp;
	register int i, n;
	BUFFER *filesbp;
	BUFFER *tagbp;

	if (!global_g_val(GMDTAGSLOOK))
		return 0;

	if ((tagbp = gettagsfile(0,&i)) == NULL)
		return 0;

	/* create/lookup the file list buffer   */
	if ((filesbp = bs_init(ScratchName(files), (sequence == 0))) != NULL
	 && !filesbp->b_active) {

		filesbp->b_active = TRUE;

		/* loop through the tags file */
		for_each_line(tlp, tagbp) {

			/* skip the tagname */
			i = 0;
			while (i < llength(tlp) && lgetc(tlp,i) != '\t')
				i++;
			n = i++;	/* skip the tab */
			fnp = tlp->l_text + i;
			while (i < llength(tlp) && lgetc(tlp,i) != '\t')
				i++;

			/* patch: should test for tab-found */

			/* insert into the file list */
			if (bs_find(fnp, i-1-n, filesbp, TRUE, (LINE **)0) == NULL)
				break;
		}
	}
	return filesbp;
}
#endif

#else
void taghello() { }
#endif
