/*	FILE.C:   for MicroEMACS
 *
 *	The routines in this file handle the reading, writing
 *	and lookup of disk files.  All of details about the
 *	reading and writing of the disk are in "fileio.c".
 *
 *
 * $Log: file.c,v $
 * Revision 1.61  1992/11/19 09:06:35  foxharp
 * added kdone() call to complete the ksetup() operation, and
 * fixed possible someday bug with crypt leaving open files around
 *
 * Revision 1.60  1992/08/20  23:40:48  foxharp
 * typo fixes -- thanks, eric
 *
 * Revision 1.59  1992/08/19  22:56:50  foxharp
 * no longer need to multiply or add to NFILEN
 *
 * Revision 1.58  1992/08/06  23:55:07  foxharp
 * changes to canonical pathnames and directory changing to support DOS and
 * its drive designators
 *
 * Revision 1.57  1992/08/05  22:07:50  foxharp
 * glob() on DOS now does something -- it makes sure the drive designator
 * is uppercase
 *
 * Revision 1.56  1992/08/05  21:51:55  foxharp
 * trim trailing slashes in DOS as well as UNIX
 *
 * Revision 1.55  1992/08/05  21:02:51  foxharp
 * check return of mlyesno explicitly against TRUE, since it might return FALSE _or_ ABORT
 *
 * Revision 1.54  1992/07/30  07:29:55  foxharp
 * if a requested name has no slashes, try for it as a buffer name, then
 * canonicalize, and try for buffer and then file, like we used to
 *
 * Revision 1.53  1992/07/24  07:49:38  foxharp
 * shorten_name changes
 *
 * Revision 1.52  1992/07/22  09:16:49  foxharp
 * shorten_path now prints shell commands as-is
 *
 * Revision 1.51  1992/07/20  22:49:19  foxharp
 * prototyped code sure is picky...
 *
 * Revision 1.50  1992/07/18  13:13:56  foxharp
 * put all path-shortening in one place (shorten_path()), and took out some old code now
 * unnecessary
 *
 * Revision 1.49  1992/07/17  19:14:30  foxharp
 * deleted unused locals
 *
 * Revision 1.48  1992/07/15  08:54:40  foxharp
 * give up pretense that we can canonicalize relative pathnames -- all
 * pathnames are stored absolute now anyway.  also, make canonpath
 * deal with DOS \ path separators
 *
 * Revision 1.47  1992/07/13  20:18:51  foxharp
 * took out old ifdef BEFORE code
 *
 * Revision 1.46  1992/07/13  20:03:54  foxharp
 * the "terse" variable is now a boolean mode
 *
 * Revision 1.45  1992/07/13  19:38:03  foxharp
 * finished canonicalizing pathnames
 *
 * Revision 1.44  1992/07/13  09:28:37  foxharp
 * preliminary changes for canonical path names
 *
 * Revision 1.43  1992/07/10  22:00:32  foxharp
 * in signal handler, don't exit after calling abort, to work around
 * bug in sunos 4.1.1 on sparcs
 *
 * Revision 1.42  1992/06/25  23:00:50  foxharp
 * changes for dos/ibmpc
 *
 * Revision 1.41  1992/05/27  08:32:57  foxharp
 * force screen updates while reading from pipes, otherwise bumping the
 * keyboard freezes the screen until the spawned process is done
 *
 * Revision 1.40  1992/05/19  08:55:44  foxharp
 * more prototype and shadowed decl fixups
 *
 * Revision 1.39  1992/05/16  12:00:31  pgf
 * prototypes/ansi/void-int stuff/microsoftC
 *
 * Revision 1.38  1992/04/14  08:51:44  pgf
 * ifdef fixups for pjr and DOS
 *
 * Revision 1.37  1992/04/02  08:28:59  pgf
 * fixed the realloc case of quickreadf()
 *
 * Revision 1.36  1992/03/20  09:00:40  pgf
 * fixed typo
 *
 * Revision 1.35  1992/03/19  23:34:53  pgf
 * set b_linecount on reads and writes of a file
 *
 * Revision 1.34  1992/03/19  23:20:22  pgf
 * SIGT for signals, linux portability
 *
 * Revision 1.33  1992/03/05  09:19:55  pgf
 * changed some mlwrite() to mlforce(), due to new terse support
 *
 * Revision 1.32  1992/02/17  09:04:03  pgf
 * fix null filename dereference, and
 * kill registers now hold unsigned chars
 *
 * Revision 1.31  1992/01/05  00:06:13  pgf
 * split mlwrite into mlwrite/mlprompt/mlforce to make errors visible more
 * often.  also normalized message appearance somewhat.
 *
 * Revision 1.30  1992/01/03  23:31:49  pgf
 * use new ch_fname() to manipulate filenames, since b_fname is now
 * a malloc'ed sting, to avoid length limits
 *
 * Revision 1.29  1991/11/10  21:21:20  pgf
 * don't look for cr/nl pairs on empty lines
 *
 * Revision 1.28  1991/11/08  13:20:18  pgf
 * lint cleanup, and took out lazy filename matching, and
 * changed the DOSFILES code so it's all in this routine, and made it
 * work with quickreadf, where we don't read every line, and fixed
 * core dumps on long-lined files, and allowed for aborting file read
 * operations
 *
 * Revision 1.27  1991/11/07  03:58:31  pgf
 * lint cleanup
 *
 * Revision 1.26  1991/11/03  17:46:30  pgf
 * removed f,n args from all region functions -- they don't use them,
 * since they're no longer directly called by the user
 *
 * Revision 1.25  1991/11/01  14:38:00  pgf
 * saber cleanup
 *
 * Revision 1.24  1991/10/31  02:35:04  pgf
 * avoid malloc(0) in quickreadf
 *
 * Revision 1.23  1991/10/27  01:44:06  pgf
 * used has_C_suffix routine to determine c-mode or not
 *
 * Revision 1.22  1991/10/20  23:07:13  pgf
 * shorten the big file buffer if we don't use it all due to long lines
 *
 * Revision 1.21  1991/10/18  01:17:34  pgf
 * don't seek too far when long lines found in quickread
 *
 * Revision 1.20  1991/10/10  12:33:33  pgf
 * changes to support "block malloc" of line text -- now for most files
 * there is are two mallocs and a single read, no copies.  previously there
 * were two mallocs per line, and two copies (stdio's and ours).  This change
 * implies that lines and line text should not move between buffers, without
 * checking that the text and line struct do not "belong" to the buffer.
 *
 * Revision 1.19  1991/09/24  01:19:14  pgf
 * don't let buffer name change needlessly in fileread() (:e!)
 *
 * Revision 1.18  1991/09/13  01:47:09  pgf
 * lone ":f" now gives same info as ^G
 *
 * Revision 1.17  1991/09/10  01:52:21  pgf
 * buffer name now changes to match filename after ":e!" command (fileread)
 *
 * Revision 1.16  1991/08/16  10:59:35  pgf
 * added WFKILLS to pipe-reading updates, so scrolling gets invoked
 *
 * Revision 1.15  1991/08/12  09:25:10  pgf
 * now store w_line in w_traits while buffer is offscreen, so reframe
 * isn't always necessary.  don't force reframe on redisplay.
 *
 * Revision 1.14  1991/08/08  13:19:08  pgf
 * fixed MDDOS processing, and don't allow writes of view-only buffers
 *
 * Revision 1.13  1991/08/07  12:35:07  pgf
 * added RCS log messages
 *
 * revision 1.12
 * date: 1991/08/06 15:21:21;
 * global/local values
 * 
 * revision 1.11
 * date: 1991/06/25 19:52:35;
 * massive data structure restructure
 * 
 * revision 1.10
 * date: 1991/06/13 15:18:28;
 * fixed comment on glob()
 * 
 * revision 1.9
 * date: 1991/06/03 12:18:17;
 * ifdef'ed TAGS for unresolved ref
 * 
 * revision 1.8
 * date: 1991/05/31 10:58:11;
 * fixed bug in writereg, and
 * made writeregion more like writefile
 * 
 * revision 1.7
 * date: 1991/04/25 12:08:28;
 * use npopen instead of popen for globbing
 * 
 * revision 1.6
 * date: 1991/04/22 08:59:37;
 * fixed globbing to always use /bin/sh
 * 
 * revision 1.5
 * date: 1991/04/08 15:48:59;
 * only update() in readin() if no input pending
 * 
 * revision 1.4
 * date: 1991/04/04 09:36:10;
 * allow for internal callers of ifile()
 * fixed bug with non-unique buffer names at startup
 * 
 * revision 1.3
 * date: 1990/10/01 12:16:11;
 * make mkdir() stuff conditional on ifdef HAVE_MKDIR
 * 
 * revision 1.2
 * date: 1990/09/25 11:38:17;
 * took out old ifdef BEFORE code
 * 
 * revision 1.1
 * date: 1990/09/21 10:25:16;
 * initial vile RCS revision
*/

#include        <stdio.h>
#include        <string.h>
#include	"estruct.h"
#include        "edef.h"


extern int fileispipe;
int doslines, unixlines;

#if MSDOS
# define slashc(c) (c == '\\' || c == '/')
#else
# define slashc(c) (c == '/')
#endif

/*
 * Read a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "read a file into the current buffer" code.
 */
/* ARGSUSED */
int
fileread(f, n)
int f,n;
{
        register int    s;
        static char fname[NFILEN];
        char bname[NBUFN];

        if ((s=mlreply("Replace with file: ", fname, NFILEN)) != TRUE)
                return s;
	if ((s = glob(fname)) != TRUE)
		return FALSE;
	/* we want no errors or complaints, so mark it unchanged */
	curbp->b_flag &= ~BFCHG;
        s = readin(fname, TRUE, curbp, TRUE);
	curbp->b_bname[0] = 0;
	makename(bname, fname);
	unqname(bname, TRUE);
	strcpy(curbp->b_bname, bname);
	return s;
}

/*
 * Select a file for editing.
 * Look around to see if you can find the
 * file in another buffer; if you can find it
 * just switch to the buffer. If you cannot find
 * the file, create a new buffer, read in the
 * text, and switch to the new buffer.
 * This is ": e"
 */
/* ARGSUSED */
int
filefind(f, n)
int f,n;
{
        static char fname[NFILEN];	/* file user wishes to find */
	char nfname[NFILEN];
        register int s;		/* status return */

	if (clexec || isnamedcmd) {
	        if ((s=mlreply("Find file: ", fname, NFILEN)) != TRUE)
	                return s;
        } else {
		screen_string(fname,NFILEN,_pathn);
        }
	if ((s = glob(fname)) != TRUE)
		return FALSE;
	strcpy (nfname, fname);
#ifdef LAZINESS
	if (othmode & OTH_LAZY) {
		char rnfname[NFILEN];
		LINE *lp;
		extern BUFFER *filesbp;
		lp = NULL;
		while (flook(nfname, FL_HERE) == NULL) {
			rvstrcpy(rnfname, fname);
			if (makeflist() == FALSE || !sortsearch(rnfname, 
					strlen(rnfname), filesbp, FALSE, &lp)) {
				/* give up, and try what they asked for */
				strcpy (nfname, fname);
				break;
			}
			rvstrncpy(nfname, lp->l_text, llength(lp));
		}
	}
#endif
	return getfile(nfname, TRUE);
}

/* ARGSUSED */
int
viewfile(f, n)	/* visit a file in VIEW mode */
int f,n;
{
        char fname[NFILEN];	/* file user wishes to find */
        register int s;		/* status return */

	fname[0] = 0;
        if ((s=mlreply("View file: ", fname, NFILEN)) != TRUE)
                return s;
	if ((s = glob(fname)) != TRUE)
		return FALSE;
	s = getfile(fname, FALSE);
	if (s == TRUE) {	/* if we succeed, put it in view mode */
		make_local_b_val(curwp->w_bufp,MDVIEW);
		set_b_val(curwp->w_bufp,MDVIEW,TRUE);
		markWFMODE(curwp->w_bufp);
	}
	return s;
}

/*
 * Insert a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "insert a file into the current buffer" code.
 */
static char insfname[NFILEN];

/* ARGSUSED */
int
insfile(f, n)
int f,n;
{
        register int    s;

	if (!calledbefore) {
	        if ((s=mlreply("Insert file: ", insfname, NFILEN)) != TRUE)
	                return s;
		if ((s = glob(insfname)) != TRUE)
			return FALSE;
	}
	if (ukb == 0)
	        return ifile(insfname,TRUE,NULL);
	else
	        return kifile(insfname);
}

int
getfile(fname, lockfl)
char *fname;		/* file name to find */
int lockfl;		/* check the file for locks? */
{
        register BUFFER *bp;
	static BUFFER *tbp;
        register int    s;
        char bname[NBUFN];	/* buffer name to put file */

#if	MSDOS
	/* msdos isn't case sensitive */
	if (isupper(fname[0]) && fname [1] == ':')
		mklower(fname+2);
	else
		mklower(fname);
#endif

	/* if there are no path delimiters in the name, then the user
		is likely asking for an existing buffer -- try for that
		first */
        if (strchr(fname,slash) == NULL &&
			(bp=bfind(fname, NO_CREAT, 0)) != NULL) {
		return swbuffer(bp);
	}
	/* oh well.  canonicalize the name, and try again */

	if (!tbp) {
		if ((tbp=(BUFFER *)malloc(sizeof(BUFFER))) == NULL)
			return FALSE;
	}
	tbp->b_fname = NULL;
	ch_fname(tbp,fname);		/* fill it out */
	fname = tbp->b_fname;

        if ((bp=bfind(fname, NO_CREAT, 0)) == NULL) {
		/* it's not already here by that buffer name */
	        for (bp=bheadp; bp!=NULL; bp=bp->b_bufp) {
			/* is it here by that filename? */
	                if (strcmp(bp->b_fname, fname)==0) {
				swbuffer(bp);
	                        curwp->w_flag |= WFMODE|WFHARD;
				if (fname[0] != '!') {
		                        mlwrite("[Old buffer]");
				} else {
		                        if (mlyesno(
				 "Old command output -- rerun") == TRUE) {
					        return readin(fname, lockfl, 
								curbp, TRUE);
					}
				}
					
	                        return TRUE;
	                }
	        }
		/* it's not here */
	        makename(bname, fname);            /* New buffer name.     */
		/* make sure the buffer name doesn't exist */
	        while ((bp=bfind(bname, NO_CREAT, 0)) != NULL) {
			if ( !(bp->b_flag & BFCHG) && is_empty_buf(bp)) {
				/* empty and unmodified -- then it's okay 
					to re-use this buffer */
				bp->b_active = 0;
				return readin(fname, lockfl, bp, TRUE) &&
						swbuffer(bp);
			}
			/* old buffer name conflict code */
			unqname(bname,TRUE);
	                s = mlreply("Will use buffer name: ", bname, NBUFN);
	                if (s == ABORT)
	                        return s;
			if (s == FALSE || bname[0] == 0)
		                makename(bname, fname);
	        }
		/* okay, we've got a unique name -- create it */
	        if (bp==NULL && (bp=bfind(bname, OK_CREAT, 0))==NULL) {
	                mlforce("[Cannot create buffer]");
	                return FALSE;
	        }
		/* switch and read it in. */
		ch_fname(bp,fname);
	}
	return swbuffer(bp);
}

/*
	Read file "fname" into a buffer, blowing away any text
	found there.  Returns the final status of the read.
*/

/* ARGSUSED */
int
readin(fname, lockfl, bp, mflg)
char    *fname;		/* name of file to read */
int	lockfl;		/* check for file locks? */
register BUFFER *bp;	/* read into this buffer */
int	mflg;		/* print messages? */
{
        register WINDOW *wp;
        register int    s;
        int    nline;
#if VMALLOC
	extern int doverifys;
	int odv;
#endif

#if	FILOCK
	if (lockfl && lockchk(fname) == ABORT)
		return ABORT;
#endif
#if	CRYPT
	s = resetkey(bp);
	if (s != TRUE)
		return s;
#endif
        if ((s=bclear(bp)) != TRUE)             /* Might be old.        */
                return s;
        bp->b_flag &= ~(BFINVS|BFCHG);
	ch_fname(bp,fname);
#if DOSFILES
	make_local_b_val(bp,MDDOS);
	set_b_val(bp, MDDOS, global_b_val(MDDOS) );
#endif

	/* turn off ALL keyboard translation in case we get a dos error */
	TTkclose();

        if ((s = ffropen(fname)) == FIOERR)       /* Hard file open.      */
                goto out;

        if (s == FIOFNF) {                      /* File not found.      */
                if (mflg) mlwrite("[New file]");
                goto out;
        }

        if (mflg) {
		 mlforce("[Reading %s ]", fname);
	}

#if UNIX & before
	if (fileispipe)
		ttclean(TRUE);
#endif
	/* read the file in */
        nline = 0;
#if VMALLOC
	/* we really think this stuff is clean... */
	odv = doverifys;
	doverifys = 0;
#endif
#if ! MSDOS	/* DOS has too many 64K limits for this to work */
	if (fileispipe || (s = quickreadf(bp, &nline)) == FIOMEM)
#endif
		s = slowreadf(bp, &nline);

	if (s == FIOERR)
		goto out;
#if VMALLOC
	doverifys = odv;
#endif
	bp->b_flag &= ~BFCHG;
#if FINDERR
	if (fileispipe == TRUE) {
		strncpy(febuff,bp->b_bname,NBUFN);
		newfebuff = TRUE;
	}
#endif
        ffclose();                              /* Ignore errors.       */
	if (mflg)
		readlinesmsg(nline,s,fname,ffronly(fname));

	bp->b_linecount = nline;

	/* set read-only mode for read-only files */
	if (fname[0] == '!' 
#if RONLYVIEW
		|| ffronly(fname) 
#endif
	) {
		make_local_b_val(bp,MDVIEW);
		set_b_val(bp,MDVIEW,TRUE);
	}
	
	bp->b_active = TRUE;

out:
#if DOSFILES
	if (b_val(bp, MDDOS)) { /* should we check for dos files? */
		LINE *lp = lforw(bp->b_line.l);
		while (lp != bp->b_line.l) {
			if (llength(lp) && lgetc(lp,llength(lp)-1) == '\r') {
				llength(lp)--;
				doslines++;
			} else {
				unixlines++;
			}
			lp = lforw(lp);
		}
		set_b_val(bp, MDDOS, doslines > unixlines);
	}
#endif
	/* set C mode for C files */
	make_local_b_val(bp,MDCMOD); /* make it local for all, so that
					subsequent changes to global value
					will _not_ affect this buffer */
	set_b_val(bp,MDCMOD, (global_b_val(MDCMOD) && has_C_suffix(bp)));

	TTkopen();	/* open the keyboard again */
        for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
                if (wp->w_bufp == bp) {
                        wp->w_line.l = lforw(bp->b_line.l);
                        wp->w_dot.l  = lforw(bp->b_line.l);
                        wp->w_dot.o  = 0;
#ifdef WINMARK
                        wp->w_mark = nullmark;
#endif
                        wp->w_lastdot = nullmark;
                        wp->w_flag |= WFMODE|WFHARD;
                }
        }
        if (s == FIOERR || s == FIOFNF) {	/* False if error.      */
#if UNIX
		extern int sys_nerr, errno;
		extern char *sys_errlist[];
		if (errno > 0 && errno < sys_nerr)
			mlforce("[%s: %s]",fname,sys_errlist[errno]);
#endif
                return FALSE;
	}
#if     NeWS
        newsreportmodes() ;
#endif
        return TRUE;
}

#if ! MSDOS
int
quickreadf(bp, nlinep)
register BUFFER *bp;
int *nlinep;
{
        register unsigned char *textp;
        unsigned char *countp;
        int nlines;
        int incomplete = FALSE;
	long len;
	long ffsize();

	if ((len = ffsize()) < 0)
		return FIOERR;

	/* avoid malloc(0) problems down below; let slowreadf() do the work */
	if (len == 0)
		return FIOMEM;
#if     MSDOS
	/* cannot allocate more than 64K in dos */
	if (len >= 65535)
		return FIOMEM;
#endif



	/* leave an extra byte at the front, for the length of the first
		line.  after that, lengths go in place of the newline at
		the end of the previous line */
	bp->b_ltext = (unsigned char *)malloc(len + 1);
	if (bp->b_ltext == NULL)
		return FIOMEM;

	if ((len = ffread((char *)&bp->b_ltext[1], len)) < 0) {
		free((char *)bp->b_ltext);
		bp->b_ltext = NULL;
		return FIOERR;
	}


	/* loop through the buffer, replacing all newlines with the
		length of the _following_ line */
	bp->b_ltext_end = bp->b_ltext + len + 1;
	countp = bp->b_ltext;
	textp = countp + 1;
        nlines = 0;
	while (len--) {
		if (*textp == '\n') {
			if (textp - countp >= 255) {
				unsigned char *np;
				len = (long)(countp - bp->b_ltext);
				incomplete = TRUE;
				/* we'll re-read the rest later */
				ffseek(len);
				np = (unsigned char *)realloc(bp->b_ltext, len);
				if (np == NULL) { /* ugh.  can this happen? */
					  /* (we're _reducing_ the size...) */
					ffrewind();
					free((char *)bp->b_ltext);
					bp->b_ltext = NULL;
					return FIOMEM;
				}
				bp->b_ltext = np;
				bp->b_ltext_end = np + len + 1;
				break;
			}
			*countp = textp - countp - 1;
			countp = textp;
			nlines++;
		}
		++textp;
	}

	/* dbgwrite("lines %d, chars %d", nlines, textp - bp->b_ltext); */
	if (nlines == 0) {
		ffrewind();
		if (bp->b_ltext)
			free((char *)bp->b_ltext);
		bp->b_ltext = NULL;
		incomplete = TRUE;
	} else {
		/* allocate all of the line structs we'll need */
		bp->b_LINEs = (LINE *)malloc(nlines * sizeof(LINE));
		if (bp->b_LINEs == NULL) {
			free((char *)bp->b_ltext);
			bp->b_ltext = NULL;
			ffrewind();
			return FIOMEM;
		}
		bp->b_LINEs_end = bp->b_LINEs + nlines;

		/* loop through the buffer again, creating
			line data structure for each line */
		{
			register LINE *lp;
			lp = bp->b_LINEs;
			textp = bp->b_ltext;
			while (lp != bp->b_LINEs_end) {
				lp->l_used = *textp;
				lp->l_size = *textp + 1;
				lp->l_text = (char *)textp + 1;
				lp->l_fp = lp + 1;
				lp->l_bp = lp - 1;
				lsetclear(lp);
				lp->l_nxtundo = NULL;
				lp++;
				textp += *textp + 1;
			}
			/*
			if (textp != bp->b_ltext_end - 1)
				mlwrite("BUG: textp not equal to end %d %d",
					textp,bp->b_ltext_end);
			*/
			lp--;  /* point at last line again */

			/* connect the end of the list */
			lp->l_fp = bp->b_line.l;
			bp->b_line.l->l_bp = lp;

			/* connect the front of the list */
			bp->b_LINEs->l_bp = bp->b_line.l;
			bp->b_line.l->l_fp = bp->b_LINEs;
		}
	}

	*nlinep = nlines;

	if (incomplete)
		return FIOMEM;
	return FIOSUC;
}

#endif /* ! MSDOS */

int
slowreadf(bp, nlinep)
register BUFFER *bp;
int *nlinep;
{
        register WINDOW *wp;
	int s;
	int flag = 0;
        int len;
#if UNIX
        int    done_update = FALSE;
#endif
        while ((s = ffgetline(&len)) == FIOSUC) {
#if DOSFILES
		if (fline[len-1] == '\r') {
			len--;
			doslines++;
		} else {
			unixlines++;
		}
#endif
		if (addline(bp,fline,len) != TRUE) {
                        s = FIOMEM;             /* Keep message on the  */
                        break;                  /* display.             */
                } 
#if UNIX
		else {
                	/* reading from a pipe, and internal? */
			if (fileispipe && !ffhasdata()) {
				flag |= WFEDIT;
				if (!done_update || bp->b_nwnd > 1)
					flag |= WFHARD;
			        for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
			                if (wp->w_bufp == bp) {
			                        wp->w_line.l=
							lforw(bp->b_line.l);
			                        wp->w_dot.l =
							lback(bp->b_line.l);
			                        wp->w_dot.o = 0;
						wp->w_flag |= flag;
			                }
			        }
				/* track changes in dosfile as lines arrive */
				set_b_val(bp, MDDOS, 
				 doslines > unixlines && global_b_val(MDDOS) );
				curwp->w_flag |= WFMODE|WFKILLS;
				update(TRUE);
				done_update = TRUE;
				flag = 0;
			} else {
				flag |= WFHARD;
			}
			
		}
#endif
                ++(*nlinep);
        }
	return s;
}

/* utility routine for no. of lines read */
void
readlinesmsg(n,s,f,rdo)
int n;
int s;
char *f;
int rdo;
{
	char *m;
	switch(s) {
		case FIOERR:	m = "I/O ERROR, ";	break;
		case FIOMEM:	m = "OUT OF MEMORY, ";	break;
		case FIOABRT:	m = "ABORTED, ";	break;
		default:	m = "";			break;
	}
	if (!global_b_val(MDTERSE))
		mlwrite("[%sRead %d line%s from \"%s\"%s]", m,
			n, n != 1 ? "s":"", f, rdo ? "  (read-only)":"" );
	else
		mlforce("[%s%d lines]",m,n);
}

/*
 * Take a file name, and from it
 * fabricate a buffer name. This routine knows
 * about the syntax of file names on the target system.
 * I suppose that this information could be put in
 * a better place than a line of code.
 */

void
makename(bname, fname)
char    bname[];
char    fname[];
{
        register char *lastsl;

        register char *fcp;
        register char *bcp;

	fcp = &fname[strlen(fname)];
	/* trim trailing whitespace */
	while (fcp != fname && (fcp[-1] == ' ' || fcp[-1] == '\t'
#if UNIX || MSDOS /* trim trailing slashes as well */
					 || slashc(fcp[-1])
#endif
							) )
                *(--fcp) = '\0';
	bcp = fcp;
	fcp = fname;
	/* trim leading whitespace */
	while (*fcp == ' ' || *fcp == '\t')
		fcp++;

#if     AMIGA
        while (bcp!=fcp && bcp[-1]!=':' && bcp[-1]!='/')
                --bcp;
#endif
#if     VMS
        while (bcp!=fcp && bcp[-1]!=':' && bcp[-1]!=']')
                --bcp;
#endif
#if     CPM
        while (bcp!=fcp && bcp[-1]!=':')
                --bcp;
#endif
#if     MSDOS
        while (bcp!=fcp && bcp[-1]!=':' && bcp[-1]!='\\'&&bcp[-1]!='/')
                --bcp;
#endif
#if     ST520
        while (bcp!=fcp && bcp[-1]!=':' && bcp[-1]!='\\')
                --bcp;
#endif
#if     UNIX
	bcp = bname;
	if (*fcp == '!') { /* then it's a shell command.  bname is first word */
		*bcp++ = '!';
		do {
			++fcp;
		} while (isspace(*fcp));
		while (!isspace(*fcp) && bcp < &bname[NBUFN-1])
			*bcp++ = *fcp++;
		*bcp = '\0';
		return;
	}
	lastsl = strrchr(fcp,'/');
	if (lastsl) {
		strncpy(bcp,lastsl+1,NBUFN);
		bcp[NBUFN-1] = '\0';
	} else {  /* no slashes, use the filename as is */
		strncpy(bcp,fcp,NBUFN);
		bcp[NBUFN-1] = '\0';
	}
	return;

#else
	{
        register char *cp2;
        cp2 = &bname[0];
        while (cp2!=&bname[NBUFN-1] && *bcp!=0 && *bcp!=';')
                *cp2++ = *bcp++;
        *cp2 = 0;
	}
#endif
}

void
unqname(name,ok_to_ask)	/* make sure a buffer name is unique */
char *name;	/* name to check on */
int ok_to_ask;  /* prompts allowed? */
{
	register char *sp;

	/* check to see if it is in the buffer list */
	while (bfind(name, 0, NO_CREAT) != NULL) {

		sp = &name[strlen(name)-1];  /* last char */
		if (sp - name >= 2 && sp[-1] == '-') {
			if (sp[0] == '9')
				sp[0] = 'A';
			else if (sp[0] == 'Z')
				goto choosename;
			else if (isdigit(sp[0]) || isupper(sp[0]))
				sp[0] += 1;
		} else if (sp + 2 < &name[NBUFN-1])  {
			strcat(sp, "-1");
		} else {
		choosename:
			if (ok_to_ask) {
				do {
					mlreply("Choose a unique buffer name: ",
						 name, NBUFN);
				} while (name[0] == '\0');
			} else { /* can't ask, just overwrite end of name */
				sp[-1] = '-';
				sp[0] = '1';
			}
		}
	}
}

/*
 * Ask for a file name, and write the
 * contents of the current buffer to that file.
 */
/* ARGSUSED */
int
filewrite(f, n)
int f,n;
{
        register int    s;
        static char            fname[NFILEN];

	strncpy(fname, curbp->b_fname, NFILEN);
	
	/* HACK -- this implies knowledge of how kbd_engl works! */
	if (isnamedcmd && lastkey != '\r') {
	        if ((s=mlreply("Write to file: ", fname, NFILEN)) != TRUE)
	                return s;
		if ((s = glob(fname)) != TRUE)
			return FALSE;
		if (strcmp(fname,curbp->b_fname) &&
			fname[0] != '!' && flook(fname,FL_HERE)) {
			if (mlyesno("File exists, okay to overwrite") != TRUE) {
				mlwrite("File not written");
				return FALSE;
			}
		}
        }
	if (!strcmp(fname,curbp->b_fname) && b_val(curbp,MDVIEW)) {
		mlforce("[Can't write-back from view mode]");
		return FALSE;
	}
        if ((s=writeout(fname,curbp,TRUE)) == TRUE) {
                curbp->b_flag &= ~BFCHG;
		markWFMODE(curbp);
        }
        return s;
}

/*
 * Save the contents of the current
 * buffer in its associatd file.
 * Error if there is no remembered file
 * name for the buffer.
 */
/* ARGSUSED */
int
filesave(f, n)
int f,n;
{
        register int    s;

        if (curbp->b_fname[0] == 0) {           /* Must have a name.    */
                mlforce("[No file name]");
                return FALSE;
        }
        if ((s=writeout(curbp->b_fname,curbp,TRUE)) == TRUE) {
                curbp->b_flag &= ~BFCHG;
		markWFMODE(curbp);
        }
        return s;
}

/*
 * This function performs the details of file
 * writing. Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed. Sadly, it looks inside a LINE; provide
 * a macro for this. Most of the grief is error
 * checking of some sort.
 */
int
writeout(fn,bp,msgf)
char    *fn;
BUFFER *bp;
int msgf;
{
        register LINE   *lp;		/* current line */
        register long   numchars;	/* # of chars in file */
        REGION region;

	/* starting at the beginning of the buffer */
        lp = lforw(bp->b_line.l);
        region.r_orig.l = lp;
        region.r_orig.o = 0;

	/* start counting chars */
        numchars = 0;
        while (lp != bp->b_line.l) {
		numchars += llength(lp) + 1;
		lp = lforw(lp);
        }
        region.r_size = numchars;
        region.r_end = bp->b_line;
        
	return writereg(&region,fn,msgf,b_val(bp, MDDOS),&bp);
}

int
writeregion()
{
        REGION region;
	int s;
        static char fname[NFILEN];

	if (isnamedcmd && lastkey == '\r') {
		strncpy(fname, curbp->b_fname, NFILEN);

		if (mlyesno("Okay to write [possible] partial range") != TRUE) {
			mlwrite("Range not written");
			return FALSE;
		}
	} else {
		/* HACK -- this implies knowledge of 
					how kbd_engl works! */
	        if ((s=mlreply("Write region to file: ", fname, NFILEN))
							 != TRUE)
	                return s;
		if ((s = glob(fname)) != TRUE)
			return FALSE;
		if (strcmp(fname,curbp->b_fname) &&
			fname[0] != '!' && flook(fname,FL_HERE)) {
			if (mlyesno("File exists, okay to overwrite")
							!= TRUE) {
				mlwrite("File not written");
				return FALSE;
			}
		}
        }
        if ((s=getregion(&region)) != TRUE)
                return s;
	s = writereg(&region,fname,TRUE,b_val(curbp, MDDOS), NULL);
        return s;
}


int
writereg(rp,fn,msgf, do_cr, bpp)
REGION	*rp;
char    *fn;
int 	msgf;
int	do_cr;
BUFFER	**bpp;
{
        register int    s;
        register LINE   *lp;
        register int    nline;
	register int i;
	long lim;
	long nchar;

#if	CRYPT
	s = resetkey(curbp);
	if (s != TRUE)
		return s;
#endif
        if (*fn == '[' || *fn == ' ') {
        	mlforce("[No filename]");
        	return FALSE;
        }
        
        if ((s=ffwopen(fn)) != FIOSUC)       /* Open writes message. */
                return FALSE;

	/* tell us we're writing */
	if (msgf == TRUE)
		mlwrite("[Writing...]");

#if UNIX & ! NeWS
	if (fileispipe)
		ttclean(TRUE);
#else
	TTkclose();
#endif

        lp = rp->r_orig.l;
        nline = 0;                              /* Number of lines     */
        nchar = 0;                              /* Number of chars     */

	/* First and maybe only line. */
	if (rp->r_orig.o <= llength(lp)) {
		if ((lim = rp->r_orig.o+rp->r_size) > llength(lp))
			lim = (long)llength(lp);
		for (i = rp->r_orig.o; i < lim; i++) {
		        if ((s=ffputc(lgetc(lp,i))) != FIOSUC)
		                goto out;
			nchar++;
		}
		rp->r_size -= nchar;

		if (rp->r_size <= 0)
			goto out;

		if (do_cr && (s=ffputc('\r')) != FIOSUC)
	                goto out;
	        if ((s=ffputc('\n')) != FIOSUC)
	                goto out;

		nchar++;
		nline++;
		rp->r_size--;
                lp = lforw(lp);

	}

	/* whole lines */
        while (rp->r_size >= llength(lp)+1) {
                if ((s=ffputline(&lp->l_text[0], llength(lp), do_cr))
						!= FIOSUC)
                        goto out;
                ++nline;
		nchar += llength(lp) + 1;
		rp->r_size -= llength(lp) + 1;
                lp = lforw(lp);
        }

	/* last line */
	if (rp->r_size > 0) {
		lim = rp->r_size;
		for (i = 0; i < lim; i++) {
		        if ((s=ffputc(lgetc(lp,i))) != FIOSUC)
		                goto out;
			nchar++;
			rp->r_size--;
		}
	}
	if (rp->r_size != 0)
		mlforce("BUG: writereg, rsize == %d",rp->r_size);

 out:
        if (s == FIOSUC) {                      /* No write error.      */
                s = ffclose();
                if (s == FIOSUC && msgf) {      /* No close error.      */
			if (!global_b_val(MDTERSE))
				mlwrite("[Wrote %d line%s %ld char%s to %s]", 
					nline, (nline>1)?"s":"",
					nchar, (nchar>1)?"s":"", fn);
			else
				mlforce("[%d lines]", nline);
                }
        } else {                                /* Ignore close error   */
                ffclose();                      /* if a write error.    */
	}
	if (bpp)
		(*bpp)->b_linecount = nline;
#if UNIX & ! NeWS
	if (fileispipe == TRUE) {
		ttunclean();
	        TTflush();
		pressreturn();
		sgarbf = TRUE;
	}
#else
	TTkopen();
#endif
        if (s != FIOSUC)                        /* Some sort of error.  */
                return FALSE;
        return TRUE;
}

/*
 * This function writes the kill register to a file
 * Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed.
 */
int
kwrite(fn,msgf)
char    *fn;
int	msgf;
{
	register KILL *kp;		/* pointer into kill register */
	register int	nline;
	register int	s;
	register int	c;
	register int	i;
	register char	*sp;	/* pointer into string to insert */

	/* make sure there is something to put */
	if (kbs[ukb].kbufh == NULL) {
		if (msgf) mlforce("Nothing to write");
		return FALSE;		/* not an error, just nothing */
	}

#if	CRYPT
	s = resetkey(curbp);
	if (s != TRUE)
		return s;
#endif
	/* turn off ALL keyboard translation in case we get a dos error */
	TTkclose();

	if ((s=ffwopen(fn)) != FIOSUC) {	/* Open writes message. */
		TTkopen();
		return FALSE;
	}
	/* tell us we're writing */
	if (msgf == TRUE)
		mlwrite("[Writing...]");
	nline = 0;				/* Number of lines.	*/

	kp = kbs[ukb].kbufh;
	while (kp != NULL) {
		if (kp->d_next == NULL)
			i = kbs[ukb].kused;
		else
			i = KBLOCK;
		sp = (char *)kp->d_chunk;
		while (i--) {
			if ((c = *sp++) == '\n')
				nline++;
			if ((s = ffputc(c)) != FIOSUC)
				break;
		}
		kp = kp->d_next;
	}
	if (s == FIOSUC) {			/* No write error.	*/
		s = ffclose();
		if (s == FIOSUC && msgf) {	/* No close error.	*/
			if (!global_b_val(MDTERSE))
				mlwrite("[Wrote %d line%s to %s ]",
					nline,nline!=1?"s":"", fn);
			else
				mlforce("[%d lines]", nline);
		}
	} else	{				/* Ignore close error	*/
		ffclose();			/* if a write error.	*/
	}
	TTkopen();
	if (s != FIOSUC)			/* Some sort of error.	*/
		return FALSE;
	return TRUE;
}


/*
 * The command allows the user
 * to modify the file name associated with
 * the current buffer. It is like the "f" command
 * in UNIX "ed". The operation is simple; just zap
 * the name in the BUFFER structure, and mark the windows
 * as needing an update. You can type a blank line at the
 * prompt if you wish.
 */
/* ARGSUSED */
int
filename(f, n)
int f,n;
{
        register int    s;
        static char            fname[NFILEN];

	if (isnamedcmd && lastkey == '\r') {
		return showcpos(FALSE,1);
	}
        if ((s=mlreply("Name: ", fname, NFILEN)) == ABORT)
                return s;
	if ((s = glob(fname)) != TRUE)
		return FALSE;
        if (s == FALSE)
                ch_fname(curbp, "");
        else
                ch_fname(curbp, fname);
	make_global_b_val(curbp,MDVIEW); /* no longer read only mode */
	markWFMODE(curbp);
        return TRUE;
}

/*
 * Insert file "fname" into the current
 * buffer, Called by insert file command. Return the final
 * status of the read.
 */
int
ifile(fname,belowthisline,haveffp)
char    *fname;
int	belowthisline;
FILE	*haveffp;
{
        register LINE   *lp0;
        register LINE   *lp1;
        register LINE   *lp2;
        register BUFFER *bp;
        register int    s;
        int    nbytes;
        register int    nline;
	extern FILE	*ffp;

        bp = curbp;                             /* Cheap.               */
        bp->b_flag |= BFCHG;			/* we have changed	*/
	bp->b_flag &= ~BFINVS;			/* and are not temporary*/
	if (!haveffp) {
	        if ((s=ffropen(fname)) == FIOERR)       /* Hard file open.      */
	                goto out;
	        if (s == FIOFNF) {                      /* File not found.      */
	                mlforce("[No such file \"%s\" ]", fname);
			return FALSE;
	        }
	        mlwrite("[Inserting...]");
#if UNIX
		if (fileispipe)
			ttclean(TRUE);
#endif

#if	CRYPT
		s = resetkey(curbp);
		if (s != TRUE)
			return s;
#endif
	} else { /* we already have the file pointer */
		ffp = haveffp;
	}
	lp0 = curwp->w_dot.l;
	curwp->w_dot.o = 0;
	MK = DOT;

	nline = 0;
	while ((s=ffgetline(&nbytes)) == FIOSUC) {
		if ((lp1=lalloc(nbytes,curbp)) == NULL) {
			s = FIOMEM;		/* Keep message on the	*/
			break;			/* display.		*/
		}
		if (belowthisline) {
			lp2 = lp0->l_fp;	/* line after insert */
		} else {
			lp2 = lp0;
			lp0 = lp0->l_bp;
		}

		/* re-link new line between lp0 and lp2 */
		lp2->l_bp = lp1;
		lp0->l_fp = lp1;
		lp1->l_bp = lp0;
		lp1->l_fp = lp2;

		if (nbytes)  /* l_text may be NULL in this case */
			memcpy(lp1->l_text, fline, nbytes);
		tag_for_undo(lp1);
		if (belowthisline)
			lp0 = lp1;
		else
			lp0 = lp2;
		++nline;
	}
	if (!haveffp) {
#if UNIX
		if (fileispipe == TRUE) {
			ttunclean();
			TTflush();
			sgarbf = TRUE;
		}
#endif
		ffclose();				/* Ignore errors.	*/
		readlinesmsg(nline,s,fname,FALSE);
	}
out:
	/* advance to the next line and mark the window for changes */
	curwp->w_dot.l = lforw(curwp->w_dot.l);
	curwp->w_flag |= WFHARD | WFMODE;

	/* copy window parameters back to the buffer structure */
	curbp->b_wtraits = curwp->w_traits;

        if (s == FIOERR)                        /* False if error.      */
                return FALSE;
        return TRUE;
}

/*
 * Insert file "fname" into the kill register
 * Called by insert file command. Return the final
 * status of the read.
 */
int
kifile(fname)
char    *fname;
{
        register int    i;
        register int    s;
        register int    nline;
        int    nbytes;

	ksetup();
        if ((s=ffropen(fname)) == FIOERR)       /* Hard file open.      */
                goto out;
        if (s == FIOFNF) {                      /* File not found.      */
                mlforce("[No such file \"%s\"]", fname);
		return FALSE;
        }
        mlwrite("[Reading...]");

#if UNIX
	if (fileispipe)
		ttclean(TRUE);
#endif

        nline = 0;
#if	CRYPT
	s = resetkey(curbp);
	if (s == TRUE)
#endif
		while ((s=ffgetline(&nbytes)) == FIOSUC) {
			for (i=0; i<nbytes; ++i)
				kinsert(fline[i]);
			kinsert('\n');
			++nline;
		}
#if UNIX
	if (fileispipe == TRUE) {
		ttunclean();
	        TTflush();
	        sgarbf = TRUE;
	}
#endif
	kdone();
        ffclose();                              /* Ignore errors.       */
	readlinesmsg(nline,s,fname,FALSE);

out:
        if (s == FIOERR)                        /* False if error.      */
                return FALSE;
        return TRUE;
}

#if UNIX

/* called on hangups, interrupts, and quits */
/* This code is definitely not production quality, or probably very
	robust, or probably very secure.  I whipped it up to save
	myself while debugging...		pgf */
/* on the other hand, it has worked for well over two years now :-) */
SIGT
imdying(signo)
int signo;
{
#if HAVE_MKDIR
	static char dirnam[NSTRING] = "/tmp/vileDXXXXXX";
#else
	static char dirnam[NSTRING] = "/tmp";
#endif
	char filnam[50];
	char cmd[80];
	BUFFER *bp;
	char *np;
	int wrote = 0;
	int created = 0;
	char *getenv();
	char *mktemp();


	bp = bheadp;
	while (bp != NULL) {
		if (((bp->b_flag & BFINVS) == 0) && 
			 bp->b_active == TRUE && 
	                 (bp->b_flag&BFCHG) != 0) {
#if HAVE_MKDIR
			if (!created) {
				(void)mktemp(dirnam);
				if(mkdir(dirnam,0700) != 0) {
					vttidy(FALSE);
					exit(1);
				}
				created = 1;
			}
#endif
			strcpy(filnam,dirnam);
			strcat(filnam,"/");
#if ! HAVE_MKDIR
			strcat(filnam,"V");
#endif
			strcat(filnam,bp->b_bname);
			if (writeout(filnam,bp,FALSE) != TRUE) {
				vttidy(FALSE);
				exit(1);
			}
			wrote++;
		}
		bp = bp->b_bufp;
	}
	if (wrote) {
		if ((np = getenv("LOGNAME")) || (np = getenv("USER"))) {
			lsprintf(cmd,
#if HAVE_MKDIR
    "(echo Subject: vile died; echo Files saved: ; ls %s/* ) | /bin/mail %s",
#else
    "(echo Subject: vile died; echo Files saved: ; ls %s/V* ) | /bin/mail %s",
#endif
				dirnam, np);
			system(cmd);
		}
	}
	vttidy(FALSE);
	if (signo > 2)
		abort();
	else
		exit(wrote);

	/* NOTREACHED */
	SIGRET;
}
#endif

void
markWFMODE(bp)
BUFFER *bp;
{
	register WINDOW *wp;	/* scan for windows that need updating */
        wp = wheadp;                    /* Update mode lines.   */
        while (wp != NULL) {
                if (wp->w_bufp == bp)
                        wp->w_flag |= WFMODE;
                wp = wp->w_wndp;
        }
}

/* use the shell to expand wildcards */
int
glob(buf)
char *buf;
{
#if UNIX
	char *cp;
	char cmd[NFILEN];
	FILE *cf;
	FILE *npopen();

	/* trim trailing whitespace */
	cp = &buf[strlen(buf)-1];
	while (cp != buf) {
		if (isspace(*cp))
			*cp = '\0';
		else
			break;
		cp--;
	}

	cp = buf;
	if (*cp == '!' || *cp == '[')	/* it's a shell command, or an */
		return TRUE;		/* internal name, don't bother */

	while (*cp) {
		if (iswild(*cp)) {
			lsprintf(cmd, "echo %s", buf);
			cf = npopen(cmd,"r");
			if (cf == NULL) {
				return TRUE;
			}
			if (fread(buf,1,NFILEN,cf) <= 0) {
				npclose(cf);
				return FALSE;
			}
			npclose(cf);
			cp = buf;
			while (*cp) {
				if (*cp == ' ' ) {
					if (mlyesno(
					"Too many filenames.  Use first"
							) == TRUE) {
						*cp = '\0';
						break;
					} else {
						buf[0] = 0;
						return FALSE;
					}
				} else if (*cp == '\n') {
					*cp = '\0';
					break;
				}
				cp++;
			}
			return TRUE;

		}
		cp++;
	}
#endif
#if MSDOS
	/* this is a just a convenient place to put this little hack */
	if (islower(buf[0]) && buf[1] == ':')
		buf[0] = toupper(buf[0]);
#endif
	return TRUE;
}

#if	CRYPT
resetkey(bp)	/* reset the encryption key if needed */
BUFFER *bp;
{
	register int s;	/* return status */

	/* turn off the encryption flag */
	cryptflag = FALSE;

	/* if we are in crypt mode */
	if (b_val(bp, MDCRYPT)) {
		if (bp->b_key[0] == 0) {
			s = setkey(FALSE, 0);
			if (s != TRUE)
				return s;
		}

		/* let others know... */
		cryptflag = TRUE;

		/* and set up the key to be used! */
		/* de-encrypt it */
		crypt((char *)NULL, 0);
		crypt(bp->b_key, strlen(bp->b_key));

		/* re-encrypt it...seeding it to start */
		crypt((char *)NULL, 0);
		crypt(bp->b_key, strlen(bp->b_key));
	}

	return TRUE;
}
#endif


/* canonicalize a pathname, to eliminate extraneous /./, /../, and ////
	sequences.  only guaranteed to work for absolute pathnames */
char *
canonpath(ss)
char *ss;
{
	char *p, *pp;
	char *s;
	
	s = ss;

	if (!*s)
		return s;

#if MSDOS
	/* pretend the drive designator isn't there */
	if (isalpha(*s) && *(s+1) == ':')
		s += 2;
#endif

	p = pp = s;
	if (!slashc(*s)) {
		mlforce("BUG: canonpath called with relative path");
		return ss;
	}
	p++; pp++;	/* leave the leading slash */
	while (*pp) {
		switch (*pp) {
		case '/':
#if MSDOS
		case '\\':
#endif
			pp++;
			continue;
		case '.':
			if (slashc(*(pp+1))) {
				pp += 2;
				continue;
			}
		default:
			break;
		}
		break;
	}
	while (*pp) {
#if DEBUG
		if (pp != p)
			*p = '\0';
		printf(" s is %s\n",s);
		printf("pp is %*s%s\n",pp-s,"",pp);
#endif
		if (slashc(*pp)) {
			while (slashc(*(pp+1)))
				pp++;
			if (p > s && !slashc(*(p-1)))
				*p++ = slash;
			if (*(pp+1) == '.') {
				if (*(pp+2) == '\0') {
					/* change "/." at end to "" */
					*(p-1) = '\0';	/* and we're done */
					break;
				}
				if (slashc(*(pp+2))) {
					pp += 2;
					continue;
				} else if (*(pp+2) == '.' && (slashc(*(pp+3))
							|| *(pp+3) == '\0')) {
					while (p-1 > s && slashc(*(p-1)))
						p--;
					while (p > s && !slashc(*(p-1)))
						p--;
					if (p == s)
						*p++ = slash;
					pp += 3;
					continue;
				}
			}
			pp++;
			continue;
		} else {
			*p++ = *pp++;
		}
	}
	if (p > s && slashc(*(p-1)))
		p--;
	if (p == s)
		*p++ = slash;
	*p = 0;
	return ss;
}

char *
shorten_path(f)
char *f;
{
	char *cwd;
	char *ff;
	char *slp;

	if (!f || *f == '\0')
		return NULL;

	if (*f == '!' || *f == '[')
		return f;

	cwd = current_directory(FALSE);
	slp = ff = f;
	while (*cwd && *ff && *cwd == *ff) {
		if (*ff == slash)
			slp = ff;
		cwd++;
		ff++;
	}

	/* if we reached the end of cwd, and we're at a path boundary,
		then the file must be under '.' */
	if (*cwd == '\0') {
		if (*ff == slash)
			return ff+1;
		if (slp == ff - 1)
			return ff;
	}
	
	/* if we mismatched during the first path component, we're done */
	if (slp == f)
		return f;

	/* if we mismatched in the last component of cwd, then the file
		is under '..' */
	if (strchr(cwd,slash) == NULL) {
		static char path[NFILEN];
		strcpy(path,"..");
		strcat(path,slp);
		return path;
	}

	/* we're off by more than just '..', so use absolute path */
	return f;
}
