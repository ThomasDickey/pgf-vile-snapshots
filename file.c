/*	FILE.C:   for MicroEMACS
 *
 *	The routines in this file handle the reading, writing
 *	and lookup of disk files.  All of details about the
 *	reading and writing of the disk are in "fileio.c".
 *
 *
 * $Log: file.c,v $
 * Revision 1.74  1993/03/16 16:04:01  pgf
 * fix 'parentheses suggested' warnings
 *
 * Revision 1.73  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.72  1993/03/05  17:50:54  pgf
 * see CHANGES, 3.35 section
 *
 * Revision 1.71  1993/02/24  10:59:02  pgf
 * see 3.34 changes, in CHANGES file
 *
 * Revision 1.70  1993/02/23  12:04:15  pgf
 * added support for appending to file (from alistair crooks)
 *
 * Revision 1.69  1993/02/08  14:53:35  pgf
 * see CHANGES, 3.32 section
 *
 * Revision 1.68  1993/01/23  13:38:23  foxharp
 * changes for updating buffer list when writing files out,
 * apollo-specific change for death conditions,
 * use new exit code macros
 *
 * Revision 1.67  1993/01/16  10:32:28  foxharp
 * some lint, some macro-ization of special filenames, a couple pathname
 * manipulators (lengthen_path(), is_pathname())
 *
 * Revision 1.66  1992/12/30  19:54:56  foxharp
 * avoid inf. loop when choosing unique name for buffers that have names
 * containing "-x" in the NBUFN-1 position
 *
 * Revision 1.65  1992/12/23  09:19:26  foxharp
 * allow ":e!" with no file to default to current file, and
 * lint cleanup
 *
 * Revision 1.64  1992/12/14  09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.63  1992/12/05  13:55:06  foxharp
 * rvstrcpy is now here, and ifdef'ed
 *
 * Revision 1.62  1992/12/04  09:26:43  foxharp
 * added apollo leading '//' fix to canonpath (probably needs work elsewhere
 * as well), and deleted unused assigns
 *
 * Revision 1.61  1992/11/19  09:06:35  foxharp
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

static int
writable(fname, bp)
char *	fname;
BUFFER *bp;
{
	if (!strcmp(fname, bp->b_fname) && b_val(bp,MDVIEW)) {
		mlforce("[Can't write-back from view mode]");
		return FALSE;
	}
	return TRUE;
}

int
no_such_file(fname)
char *	fname;
{
	mlforce("[No such file \"%s\"]", fname);
	return FALSE;
}

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
        char bname[NBUFN];
	char fname[NFILEN];

	if ((s = mlreply_file("Replace with file: ", (TBUFF **)0, FILEC_REREAD, fname)) != TRUE)
                return s;

	/* we want no errors or complaints, so mark it unchanged */
	curbp->b_flag &= ~BFCHG;
        s = readin(fname, TRUE, curbp, TRUE);
	curbp->b_bname[0] = EOS;
	makename(bname, fname);
	unqname(bname, TRUE);
	(void)strcpy(curbp->b_bname, bname);
	updatelistbuffers();
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
	register int	s;
	char fname[NFILEN];
	static	TBUFF	*last;

	if ((s = mlreply_file("Find file: ", &last, FILEC_READ, fname)) == TRUE)
		s = getfile(fname, TRUE);
	return s;
}

/* ARGSUSED */
int
viewfile(f, n)	/* visit a file in VIEW mode */
int f,n;
{
        char fname[NFILEN];	/* file user wishes to find */
        register int s;		/* status return */
	static	TBUFF	*last;

	if ((s = mlreply_file("View file: ", &last, FILEC_READ, fname)) != TRUE)
                return s;
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
/* ARGSUSED */
int
insfile(f, n)
int f,n;
{
        register int    s;
	char fname[NFILEN];
	static	TBUFF	*last;

	if (!calledbefore) {
	        if ((s= mlreply_file("Insert file: ", &last, FILEC_READ|FILEC_PROMPT, fname)) != TRUE)
	                return s;
	}
	if (ukb == 0)
	        return ifile(fname, TRUE, (FILE *)0);
	else
	        return kifile(fname);
}

static int
getfile2(fname, lockfl)
char *fname;		/* file name to find */
int lockfl;		/* check the file for locks? */
{
	register BUFFER *bp;
        register int    s;
        char bname[NBUFN];	/* buffer name to put file */

        if ((bp=bfind(fname, NO_CREAT, 0)) == NULL) {
		/* it's not already here by that buffer name */
	        for_each_buffer(bp) {
			/* is it here by that filename? */
	                if (strcmp(bp->b_fname, fname)==0) {
				(void)swbuffer(bp);
	                        curwp->w_flag |= WFMODE|WFHARD;
				if (!isShellOrPipe(fname)) {
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
			hst_glue(' ');
	                s = mlreply("Will use buffer name: ", bname, NBUFN);
	                if (s == ABORT)
	                        return s;
			if (s == FALSE || bname[0] == EOS)
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

int
getfile(fname, lockfl)
char *fname;		/* file name to find */
int lockfl;		/* check the file for locks? */
{
        register BUFFER *bp;
        register int    s;
	static BUFFER tbp;

#if	MSDOS
	/* msdos isn't case sensitive */
	if (isupper(fname[0]) && fname [1] == ':')
		(void)mklower(fname+2);
	else
		(void)mklower(fname);
#endif

	/* if there are no path delimiters in the name, then the user
		is likely asking for an existing buffer -- try for that
		first */
        if (strchr(fname,slash) == NULL &&
			(bp=bfind(fname, NO_CREAT, 0)) != NULL) {
		return swbuffer(bp);
	}

	/* oh well.  canonicalize the name, and try again */
	ch_fname(&tbp, fname);		/* fill it out */
	fname = tbp.b_fname;
	s = getfile2(fname, lockfl);

#if NO_LEAKS
	free(tbp.b_fname);
	tbp.b_fname = 0;
#endif
	return s;
}

/*
 * Scan a buffer to see if it contains more lines terminated by CR-LF than by
 * LF alone.  If so, set the DOS-mode to true, otherwise false.
 */
#if DOSFILES
static void
guess_dosmode(bp)
BUFFER *bp;
{
	if (b_val(bp, MDDOS)) { /* should we check for dos files? */
		int	doslines = 0,
			unixlines = 0;
		register LINE *lp;

		make_local_b_val(bp, MDDOS);	/* keep it local, if not */
		for_each_line(lp,bp) {
			if (llength(lp) > 0
			 && lgetc(lp, llength(lp)-1) == '\r') {
				llength(lp)--;
				doslines++;
			} else {
				unixlines++;
			}
		}
		set_b_val(bp, MDDOS, doslines > unixlines);
	}
}

/*
 * Forces the current buffer to be in DOS-mode, stripping any trailing CR's.
 */
/*ARGSUSED*/
int
set_dosmode(f,n)
int	f,n;
{
	make_local_b_val(curbp, MDDOS);
	set_b_val(curbp, MDDOS, TRUE);
	guess_dosmode(curbp);
	set_b_val(curbp, MDDOS, TRUE);
	markWFMODE(curbp);
	return TRUE;
}
#endif

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
		(void)strncpy(febuff,bp->b_bname,NBUFN);
		newfebuff = TRUE;
	}
#endif
        (void)ffclose();                         /* Ignore errors.       */
	if (mflg)
		readlinesmsg(nline,s,fname,ffronly(fname));

	bp->b_linecount = nline;

	/* set read-only mode for read-only files */
	if (isShellOrPipe(fname)
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
	guess_dosmode(bp);
#endif
	/* set C mode for C files */
	make_local_b_val(bp,MDCMOD); /* make it local for all, so that
					subsequent changes to global value
					will _not_ affect this buffer */
	set_b_val(bp,MDCMOD, (global_b_val(MDCMOD) && has_C_suffix(bp)));

	TTkopen();	/* open the keyboard again */
        for_each_window(wp) {
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
	imply_alt(fname, FALSE, lockfl);
	updatelistbuffers();
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
	bp->b_ltext = castalloc(unsigned char, len + 1);
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
				np = castrealloc(unsigned char,
							bp->b_ltext, len);
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
		bp->b_LINEs = typeallocn(LINE,nlines);
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
#if DOSFILES
	int	doslines = 0,
		unixlines = 0;
#endif
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
			        for_each_window(wp) {
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
				if (global_b_val(MDDOS))
					set_b_val(bp, MDDOS, doslines > unixlines);

				curwp->w_flag |= WFMODE|WFKILLS;
				if (!update(TRUE)) {
					s = FIOERR;
					break;
				}
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
                *(--fcp) = EOS;
	fcp = fname;
	/* trim leading whitespace */
	while (*fcp == ' ' || *fcp == '\t')
		fcp++;

#if	! UNIX
	bcp = fcp;
#endif
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
	if (isShellOrPipe(fcp)) { /* ...it's a shell command; bname is first word */
		*bcp++ = SHPIPE_LEFT[0];
		do {
			++fcp;
		} while (isspace(*fcp));
		while (!isspace(*fcp) && bcp < &bname[NBUFN-1])
			*bcp++ = *fcp++;
		*bcp = EOS;
		return;
	}
	lastsl = strrchr(fcp,'/');
	if (lastsl) {
		(void)strncpy(bcp,lastsl+1,NBUFN);
		bcp[NBUFN-1] = EOS;
	} else {  /* no slashes, use the filename as is */
		(void)strncpy(bcp,fcp,NBUFN);
		bcp[NBUFN-1] = EOS;
	}
	return;

#else
	{
        register char *cp2;
        cp2 = &bname[0];
        while (cp2!=&bname[NBUFN-1] && *bcp!=0 && *bcp!=';')
                *cp2++ = *bcp++;
        *cp2 = EOS;
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
			else if ((sp[0] == 'Z') || 
					 (!isdigit(sp[0]) && !isupper(sp[0])))
				goto choosename;
			else
				sp[0] += 1;
		} else if (sp + 2 < &name[NBUFN-1])  {
			(void)strcat(sp, "-1");
		} else {
		choosename:
			if (ok_to_ask) {
				do {
					hst_glue(' ');
					(void)mlreply("Choose a unique buffer name: ",
						 name, NBUFN);
				} while (name[0] == EOS);
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
        char            fname[NFILEN];

	if (isnamedcmd && !isreturn(end_string())) {
	        if ((s= mlreply_file("Write to file: ", (TBUFF **)0, FILEC_WRITE, fname)) != TRUE)
	                return s;
        } else
		(void) strcpy(fname, curbp->b_fname);

        if ((s=writeout(fname,curbp,TRUE)) == TRUE)
		unchg_buff(curbp, 0);
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

        if (curbp->b_fname[0] == EOS) {		/* Must have a name.    */
                mlforce("[No file name]");
                return FALSE;
        }
        if ((s=writeout(curbp->b_fname,curbp,TRUE)) == TRUE)
		unchg_buff(curbp, 0);
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
 
	if (!writable(fn, bp))
		return FALSE;

	return writereg(&region,fn,msgf,b_val(bp, MDDOS),&bp);
}

int
writeregion()
{
        REGION region;
	int s;
        char fname[NFILEN];

	if (isnamedcmd && isreturn(end_string())) {
		if (mlyesno("Okay to write [possible] partial range") != TRUE) {
			mlwrite("Range not written");
			return FALSE;
		}
		(void)strcpy(fname, curbp->b_fname);
	} else {
	        if ((s= mlreply_file("Write region to file: ", (TBUFF **)0, FILEC_WRITE, fname)) != TRUE)
	                return s;
        }
        if ((s=getregion(&region)) != TRUE)
                return s;

	if (!writable(fname, curbp))
		return FALSE;

	s = writereg(&region,fname,TRUE,b_val(curbp, MDDOS), (BUFFER **)0);
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
        if (*fn == SCRTCH_LEFT[0] || *fn == ' ') {
        	mlforce("[No filename]");
        	return FALSE;
        }
        
        if ((s=ffwopen(fn)) != FIOSUC)       /* Open writes message. */
                return FALSE;

	/* tell us we're writing */
	if (msgf == TRUE)
		mlwrite("[Writing...]");

	imply_alt(fn, TRUE, FALSE);

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
			if (!global_b_val(MDTERSE)) {
				char *action;
				if ((action = is_appendname(fn)) != 0) {
					fn = action;
					action = "Appended";
				} else {
					action = "Wrote";
				}
				mlwrite("[%s %d line%s %ld char%s to %s]", 
					action, nline, (nline>1)?"s":"",
					nchar, (nchar>1)?"s":"", fn);
			} else {
				mlforce("[%d lines]", nline);
			}
                }
        } else {                                /* Ignore close error   */
                (void)ffclose();                /* if a write error.    */
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
		(void)ffclose();		/* if a write error.	*/
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
        char            fname[NFILEN];

	if (isnamedcmd && lastkey == '\r') {
		return showcpos(FALSE,1);
	}

        if ((s = mlreply_file("Name: ", (TBUFF **)0, FILEC_UNKNOWN, fname)) == ABORT)
                return s;
        if (s == FALSE)
                ch_fname(curbp, "");
        else
                ch_fname(curbp, fname);
	make_global_b_val(curbp,MDVIEW); /* no longer read only mode */
	markWFMODE(curbp);
	updatelistbuffers();
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
	bp->b_flag &= ~BFINVS;			/* we are not temporary*/
	if (!haveffp) {
	        if ((s=ffropen(fname)) == FIOERR)       /* Hard file open.      */
	                goto out;
	        if (s == FIOFNF)		/* File not found.      */
			return no_such_file(fname);
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
			(void)memcpy(lp1->l_text, fline, nbytes);
		if (!tag_for_undo(lp1)) {
			s = FIOMEM;
			break;
		}
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
		(void)ffclose();		/* Ignore errors.	*/
		readlinesmsg(nline,s,fname,FALSE);
	}
out:
	/* advance to the next line and mark the window for changes */
	curwp->w_dot.l = lforw(curwp->w_dot.l);

	/* copy window parameters back to the buffer structure */
	curbp->b_wtraits = curwp->w_traits;

	imply_alt(fname, FALSE, FALSE);
	chg_buff (curbp, WFHARD);
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
        if (s == FIOFNF)			/* File not found.      */
		return no_such_file(fname);
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
				if (!kinsert(fline[i]))
					return FIOMEM;
			if (!kinsert('\n'))
				return FIOMEM;
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
        (void)ffclose();                        /* Ignore errors.       */
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

#if APOLLO
	extern	char	*getlogin();
	static	int	i_am_dead;

	if (i_am_dead++)	/* prevent recursive faults */
		_exit(signo);
	(void)lsprintf(cmd,
		"(echo signal %d killed vile;/com/tb %d)| /bin/mail %s",
		signo, getpid(), getlogin());
	(void)system(cmd);
#endif

	for_each_buffer(bp) {
		if (((bp->b_flag & BFINVS) == 0) && 
			 bp->b_active == TRUE && 
	                 (bp->b_flag&BFCHG) != 0) {
#if HAVE_MKDIR
			if (!created) {
				(void)mktemp(dirnam);
				if(mkdir(dirnam,0700) != 0) {
					vttidy(FALSE);
					exit(BAD(1));
				}
				created = 1;
			}
#endif
			(void)strcat(strcpy(filnam,dirnam), "/");
#if ! HAVE_MKDIR
			(void)strcat(filnam,"V");
#endif
			(void)strcat(filnam,bp->b_bname);

			set_b_val(bp,MDVIEW,FALSE);
			if (writeout(filnam,bp,FALSE) != TRUE) {
				vttidy(FALSE);
				exit(BAD(1));
			}
			wrote++;
		}
	}
	if (wrote) {
		if ((np = getenv("LOGNAME")) || (np = getenv("USER"))) {
			(void)lsprintf(cmd,
#if HAVE_MKDIR
    "(echo Subject: vile died; echo Files saved: ; ls %s/* ) | /bin/mail %s",
#else
    "(echo Subject: vile died; echo Files saved: ; ls %s/V* ) | /bin/mail %s",
#endif
				dirnam, np);
			(void)system(cmd);
		}
	}
	vttidy(FALSE);
	if (signo > 2)
		abort();
	else
		exit(wrote ? BAD(wrote) : GOOD);

	/* NOTREACHED */
	SIGRET;
}
#endif

void
markWFMODE(bp)
BUFFER *bp;
{
	register WINDOW *wp;	/* scan for windows that need updating */
        for_each_window(wp) {
                if (wp->w_bufp == bp)
                        wp->w_flag |= WFMODE;
        }
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
		if (bp->b_key[0] == EOS) {
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
