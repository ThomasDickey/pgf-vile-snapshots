/* Look up vi-style tags in the file "tags".
 *	Invoked either by ":ta routine-name" or by "^]" while sitting
 *	on a string.  In the latter case, the tag is the word under
 *	the cursor.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Log: tags.c,v $
 * Revision 1.11  1991/10/22 03:09:32  pgf
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

#ifndef NULL
#define NULL 0
#endif

static char tagname[NFILEN];

gototag(f,n)
{
	register int i = 0;
	register int s = TRUE;
	int taglen;

	if (clexec || isnamedcmd) {
	        if ((s=mlreply("Tag name: ", tagname, NFILEN)) != TRUE)
	                return (s);
		taglen = b_val(curbp,VAL_TAGLEN);
	} else {
		screen_string(tagname,NFILEN,_ident);
		taglen = 0;
	}
	if (s == TRUE)
		s = tags(tagname,taglen);
	return s;
}

cmdlinetag(t)
char *t;
{
	strcpy(tagname,t);
	return tags(tagname, b_val(curbp,VAL_TAGLEN));
}

static BUFFER *tagbp;

tags(tag,taglen)
char *tag;
int taglen;
{
	BUFFER *ocurbp;
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

	strcpy(tname,tag);

	if (tagbp == NULL) {
		if (gettagsfile() == FALSE)
			return FALSE;
	}

	lp = cheap_scan(tagbp, tname, taglen ? taglen : strlen(tname));
	if (lp == NULL) {
		TTbeep();
		mlwrite("No such tag: %s",tname);
		return FALSE;
	}
	
	lplim = &lp->l_text[lp->l_used];
	tfp = lp->l_text;
	while (tfp < lplim)
		if (*tfp++ == '\t')
			break;

	i = 0;
	while (i < NFILEN && tfp < lplim && *tfp != '\t') {
		tfname[i++] = *tfp++;
	}
	if (tfp >= lplim - 2) {
		mlwrite("Bad line in tags file.");
		return FALSE;
	}

	if (curbp && curwp) {
		lineno = 1;
	        for(clp = lforw(curbp->b_line.l); 
				clp != curwp->w_dot.l; clp = lforw(clp))
			lineno++;
		pushuntag(curbp->b_fname, lineno);
	}

	tfname[i] = 0;
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
		mlwrite("[Tag \"%s\" in current buffer]", tname);
		changedfile = FALSE;
	}

	/* it's an absolute move -- remember where we are */
	odot = DOT;

	i = 0;
	tfp++;  /* skip the tab */
	if (isdigit(*tfp)) { /* then it's a line number */
		int lineno = 0;
		while (isdigit(*tfp)) {
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
			mlwrite("Tag not present");
			if (!changedfile)
				tossuntag();
			return FALSE;
		}
		curwp->w_dot.l = lp;
		curwp->w_flag |= WFMOVE;
		firstnonwhite(FALSE,1);
		s = TRUE;
	}
	/* if we moved, update the "last dot" mark */
	if (s == TRUE && !sameline(DOT, odot)) {
		curwp->w_lastdot = odot;
	}
	return s;
	
}

gettagsfile()
{
	int s;
	char *tagsfile;

	/* is there a "tags" buffer around? */
        if ((tagbp=bfind("tags", NO_CREAT, 0)) == NULL) {
		char *tagf = global_b_val_ptr(VAL_TAGS);
		/* look up the tags file */
		tagsfile = flook(tagf, FL_HERE);

		/* if it isn't around, don't sweat it */
		if (tagsfile == NULL)
		{
	        	mlwrite("No tags file available.");
			return(FALSE);
		}

		/* find the pointer to that buffer */
	        if ((tagbp=bfind("tags", NO_CREAT, BFINVS)) == NULL) {
		        if ((tagbp=bfind(tagf, OK_CREAT, BFINVS)) == NULL) {
		        	mlwrite("No tags buffer");
		                return(FALSE);
			}
	        }

		if ((s = readin(tagsfile, FALSE, tagbp, FALSE)) != TRUE) {
			return(s);
		}
		strcpy(tagbp->b_bname, "tags");  /* be sure it's named tags */
		tagbp->b_flag |= BFINVS;
        }
	return TRUE;
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

untagpop(f,n)
int f,n;
{
	int lineno;
	char fname[NFILEN];
	MARK odot;

	if (!f) n = 1;
	while (n-- && popuntag(fname,&lineno))
		;
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
	mlwrite("No stacked un-tags");
	return FALSE;
}


struct untag {
	char *u_fname;
	int u_lineno;
	struct untag *u_stklink;
};

struct untag *untaghead = NULL;

pushuntag(fname,lineno)
char *fname;
int lineno;
{
	struct untag *utp;
	utp = (struct untag *)malloc(sizeof(struct untag));
	if (!utp)
		return;

	utp->u_fname = (char *)malloc(strlen(fname)+1);
	if (!utp->u_fname) {
		free(utp);
		return;
	}

	strcpy(utp->u_fname, fname);
	utp->u_lineno = lineno;
	utp->u_stklink = untaghead;
	untaghead = utp;
}

popuntag(fname,linenop)
char *fname;
int *linenop;
{
	register struct untag *utp;

	if (untaghead) {
		utp = untaghead;
		untaghead = utp->u_stklink;
		strcpy(fname, utp->u_fname);
		*linenop = utp->u_lineno;
		free(utp->u_fname);
		free(utp);
		return TRUE;
	}
	fname[0] = '\0';
	*linenop = 0;
	return FALSE;

}

/* discard without returning anything */
tossuntag()
{
	register struct untag *utp;

	if (untaghead) {
		utp = untaghead;
		untaghead = utp->u_stklink;
		free(utp);
	}
	return;

}

BUFFER *filesbp;


/* create a filelist from the contents of
 *	the tags file.  for "dir1/dir2/file" include both that and
 *	"dir1/dir2/"
 */
makeflist()
{
	register LINE *tlp, *flp;
	register char *fnp;
	register int i;
	register int len;
	char fname[NFILEN];
	char *strchr();

	if (!(othmode & OTH_LAZY))
		return TRUE;

	if (!tagbp && gettagsfile() == FALSE)
			return FALSE;

	if (filesbp != NULL)
		return TRUE;

	/* create the file list buffer   */
	filesbp = bfind("[files]", OK_CREAT, BFINVS);
	if (filesbp == NULL)
		return FALSE;
	filesbp->b_active = TRUE;

	/* loop through the tags file */
	tlp = lforw(tagbp->b_line.l);
	while (tlp != tagbp->b_line.l) {
		/* skip the tagname */
		i = 0;
		while (i < llength(tlp) && lgetc(tlp,i++) != '\t')
			;
		/* we're going to store the pathnames reversed, so that
			the sorting puts all directories together (they'll
			all start with their trailing slash) and all 
			files with matching basenames will be grouped
			together as well.
		*/
		/* pull out the filename, in reverse */
		fnp = &fname[NFILEN-1];
		*fnp-- = '\0';
		while (i < llength(tlp)  && fnp >= fname && 
					(*fnp = lgetc(tlp,i++)) != '\t') {
			fnp--;
		}
		fnp++; /* forward past the tab */

		/* insert into the file list */
		if (sortsearch(fnp, &fname[NFILEN-1]-fnp, filesbp,
							TRUE, NULL) == NULL)
			return FALSE;

		if (fnp = strchr(fnp, '/')) { /* first (really last) slash */
			/* insert the directory name into the file list again */
			if (sortsearch(fnp, &fname[NFILEN-1]-fnp, filesbp,
							TRUE, NULL) == NULL)
				return FALSE;
		}
		tlp = lforw(tlp);
	}
	return TRUE;
}

/* look for or insert a text string into the given buffer.  start looking
	at the given line if non-null. */
sortsearch(text, len,  bp, insert, lpp)
char *text;
int len;
BUFFER *bp;
int insert;
LINE **lpp;
{
	LINE *nlp, *lp;
	register int i, r, cmplen;

	if (lpp == NULL) {
		lp = lforw(bp->b_line.l);
	} else {
		lp = *lpp;
		if (lp == NULL)
			lp = lforw(bp->b_line.l);
		else
			lp = lforw(lp);
	}

	while (1) {
		cmplen = (len < llength(lp) && !insert) ? len : llength(lp);
		if ((r = strncmp(text, lp->l_text, cmplen)) > 0 ||
		     lp == bp->b_line.l) { /* stick line into buffer */
		     	if (!insert)
				return FALSE;
		        if ((nlp=lalloc(len,bp)) == NULL)
		                return FALSE;
			memcpy(nlp->l_text, text, len);
		        lp->l_bp->l_fp = nlp;
		        nlp->l_bp = lp->l_bp;
		        lp->l_bp = nlp;
		        nlp->l_fp = lp;
			if (lpp)
				*lpp = nlp;
			return TRUE;
		} else if (r == 0) { /* it's already here, don't insert twice */
			if (lpp)
				*lpp = lp;
			return TRUE;
		}
		lp = lforw(lp);
	}
}


#else
taghello() { }
#endif
