/* Look up vi-style tags in the file "tags".
 *	Invoked either by ":ta routine-name" or by "^]" while sitting
 *	on a string.  In the latter case, the tag is the word under
 *	the cursor.
 *	written for vile by Paul Fox, (c)1990
 *
 * $Log: tags.c,v $
 * Revision 1.27  1993/01/16 10:42:25  foxharp
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


#ifndef NULL
#define NULL 0
#endif

static char tagname[NFILEN];
static char tagprefix[NFILEN];

void pushuntag();


/* ARGSUSED */
int
gototag(f,n)
int f,n;
{
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

int
cmdlinetag(t)
char *t;
{
	strcpy(tagname,t);
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

	strcpy(tname,tag);

	if ((tagbp = gettagsfile()) == NULL )
		return FALSE;

	lp = cheap_scan(tagbp, tname, taglen ? taglen : strlen(tname));
	if (lp == NULL) {
		TTbeep();
		mlforce("[No such tag: \"%s\"]",tname);
		return FALSE;
	}
	
	lplim = &lp->l_text[lp->l_used];
	tfp = lp->l_text;
	while (tfp < lplim)
		if (*tfp++ == '\t')
			break;

	i = 0;
	if (b_val(curbp,MDTAGSRELTIV) && *tfp != '/') {
		strcpy(tfname, tagprefix);
		i += strlen(tagprefix);
	}
	while (i < NFILEN && tfp < lplim && *tfp != '\t') {
		tfname[i++] = *tfp++;
	}
	tfname[i] = 0;

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
		firstnonwhite(FALSE,1);
		s = TRUE;
	}
	/* if we moved, update the "last dot" mark */
	if (s == TRUE && !sameline(DOT, odot)) {
		curwp->w_lastdot = odot;
	}
	return s;
	
}

#define TAGBUF "tags"
BUFFER *
gettagsfile()
{
	char *tagsfile;
	BUFFER *tagbp;

	/* is there a TAGBUF buffer around? */
        if ((tagbp=bfind(TAGBUF, NO_CREAT, 0)) == NULL) {
		char *tagf = global_b_val_ptr(VAL_TAGS);
		/* look up the tags file */
		tagsfile = flook(tagf, FL_HERE);

		/* if it isn't around, don't sweat it */
		if (tagsfile == NULL)
		{
	        	mlforce("[No tags file available.]");
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
		/* be sure it's named TAGBUF */
		strcpy(tagbp->b_bname, TAGBUF);
		tagbp->b_flag |= BFINVS;
			
        }
	if (strrchr(tagbp->b_fname,'/')) {
		strcpy(tagprefix, tagbp->b_fname);
		*(strrchr(tagprefix,'/')+1) = '\0';
	} else {
		tagprefix[0] = '\0';
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

	strcpy(utp->u_fname, fname);
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
		strcpy(fname, utp->u_fname);
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

#ifdef LAZINESS
BUFFER *filesbp;


/* create a filelist from the contents of
 *	the tags file.  for "dir1/dir2/file" include both that and
 *	"dir1/dir2/"
 */
int
makeflist()
{
	register LINE *tlp;
	register char *fnp;
	register int i;
	char fname[NFILEN];
	BUFFER *tagbp;

	if (!(othmode & OTH_LAZY))
		return TRUE;

	if ((tagbp = gettagsfile()) == NULL)
			return FALSE;

	if (filesbp != NULL)
		return TRUE;

	/* create the file list buffer   */
	filesbp = bfind(ScratchName(files), OK_CREAT, BFINVS);
	if (filesbp == NULL)
		return FALSE;
	filesbp->b_active = TRUE;

	/* loop through the tags file */
	tlp = lforw(tagbp->b_line.l);
	while (tlp != tagbp->b_line.l) {
		/* skip the tagname */
		i = 0;
		while (i < llength(tlp) && lgetc(tlp,i) != '\t')
			i++;
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

		/* first (really last) slash */
		if ((fnp = strchr(fnp, slash)) != NULL) {
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
int
sortsearch(text, len,  bp, insert, lpp)
char *text;
int len;
BUFFER *bp;
int insert;
LINE **lpp;
{
	LINE *nlp, *lp;
	register int r, cmplen;

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
#endif


#else
void taghello() { }
#endif
