/*	FILE.C:   for MicroEMACS
 *
 *	The routines in this file handle the reading, writing
 *	and lookup of disk files.  All of details about the
 *	reading and writing of the disk are in "fileio.c".
 *
 *
 * $Log: file.c,v $
 * Revision 1.128  1994/04/22 14:34:15  pgf
 * changed BAD and GOOD to BADEXIT and GOODEXIT
 *
 * Revision 1.127  1994/04/22  13:49:00  pgf
 * read and write hooks
 *
 * Revision 1.126  1994/04/18  14:26:27  pgf
 * merge of OS2 port patches, and changes to tungetc operation
 *
 * Revision 1.125  1994/04/13  20:46:38  pgf
 * various fixes (towards 4.4) from kev
 *
 * Revision 1.124  1994/04/07  18:13:56  pgf
 * beep management
 *
 * Revision 1.123  1994/04/05  14:04:02  pgf
 * added "view-on-readonly" mode
 *
 * Revision 1.122  1994/04/01  10:47:10  pgf
 * don't just return on FIOERR in readin -- the rest of the routine needs
 * to run, to set cmode correctly, for example
 *
 * Revision 1.121  1994/03/23  12:57:57  pgf
 * rationalized use of mlerror() and FIOERR.  now, the function that
 * first generates FIOERR is guaranteed to have put out a message, probably
 * via mlerror.
 *
 * Revision 1.120  1994/02/22  18:09:24  pgf
 * when choosing dos-mode for an ambiguous buffer, vote in favor of on
 * if the global mode is set, and we're running on DOS.  otherwise, choose
 * no-dos-mode.
 *
 * Revision 1.119  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 */

#include	"estruct.h"
#include        "edef.h"

#if UNIX || defined(MDCHK_MODTIME)
#include	<sys/stat.h>  /* for mkdir() declaration */
#endif

static	void	readlinesmsg P(( int, int, char *, int ));
static	int	getfile2 P(( char *, int ));
#if DOSFILES
static	void	guess_dosmode P(( BUFFER * ));
/* give DOS the benefit of the doubt on ambiguous files */
# if MSDOS
#  define MORETHAN >=
# else
#  define MORETHAN >
# endif
#endif
static	int	writereg P(( REGION *, char *, int, BUFFER * ));
#if UNIX
static	int	slowtime P(( long * ));
#endif

/*--------------------------------------------------------------------------*/

#ifdef	MDCHK_MODTIME
static	int	PromptModtime P(( BUFFER *, char *, char *, int));

static int
PromptModtime (bp, fname, question, iswrite)
BUFFER	*bp;
char	*fname;
char	*question;
int	iswrite;
{
	int status = SORTOFTRUE;
	long current;
	char prompt[NLINE];

	if (!b_is_temporary(bp)
	 && b_val(bp, MDCHK_MODTIME)
	 && bp->b_active	/* only buffers that are loaded */
	 && same_fname(fname, bp, FALSE)
	 && get_modtime(bp, &current)) {
		long check_against;
		char *remind, *again;
		if (iswrite) {
			check_against = bp->b_modtime;
			remind = "Reminder: ";
			again = "";
		} else {
			remind = "";
			if (bp->b_modtime_at_warn) {
				check_against = bp->b_modtime_at_warn;
				again = "again ";
			} else {
				check_against = bp->b_modtime;
				again = "";
			}
		}

		if (check_against != current) {
			(void)lsprintf(prompt,
			"%sFile for buffer \"%s\" has changed %son disk.  %s",
				remind, get_bname(bp), again, question);
			if ((status = mlyesno( prompt )) != TRUE)
				mlerase();
			/* avoid reprompts */
			bp->b_modtime_at_warn = current;
		}
	}
	return status;
}

int
ask_shouldchange(bp)
BUFFER *bp;
{
	int status;
	status = PromptModtime(bp, bp->b_fname, "Continue", FALSE);
	return (status == TRUE || status == SORTOFTRUE);
}

int
get_modtime (bp, the_time)
BUFFER	*bp;
long	*the_time;
{
	struct stat	statbuf;

	*the_time = 0;
	if (!isInternalName(bp->b_fname)) {
		if (stat(bp->b_fname, &statbuf) >= 0) {
			*the_time = statbuf.st_mtime;
			return TRUE;
		}
	}
	return FALSE;
}

void
set_modtime( bp, fn )
BUFFER *bp;
char *fn;
{
	long	current;

	if (same_fname(fn, bp, FALSE) && get_modtime(bp, &current)) {
		bp->b_modtime = current;
		bp->b_modtime_at_warn = 0;
	}
}

int
check_modtime( bp, fn )
BUFFER *bp;
char *fn;
{
	int status = TRUE;

	if (PromptModtime(bp, fn, "Read from disk", FALSE) == TRUE) {
		status = readin(fn, TRUE, bp, TRUE);
	}
	return status;
}

int
inquire_modtime( bp, fn )
BUFFER *bp;
char *fn;
{
	register int status;
	if ((status = PromptModtime(bp, fn, "Continue write", TRUE)) != TRUE
	 && (status != SORTOFTRUE)) {
		mlforce("[Write aborted]");
		return FALSE;
	}
	return TRUE;
}

int
check_visible_modtimes ()
{
	register WINDOW *wp;

	for_each_window(wp)
		(void)check_modtime(wp->w_bufp, wp->w_bufp->b_fname);
	return TRUE;
}
#endif	/* MDCHK_MODTIME */

#if UNIX
static	void	CleanAfterPipe P((int));

#define	CleanToPipe()	if (fileispipe) ttclean(TRUE)

static void
CleanAfterPipe (Wrote)
int	Wrote;
{
	if (fileispipe == TRUE) {
		ttunclean();	/* may clear the screen as a side-effect */
	        TTflush();
		if (Wrote) pressreturn();
	        sgarbf = TRUE;
	}
}
#else
#define	CleanToPipe()		TTkclose()
#define	CleanAfterPipe(f)	TTkopen()
#endif

/*
 * On faster machines, a pipe-writer will tend to keep the pipe full. This
 * function is used by 'slowreadf()' to test if we've not done an update
 * recently even if this is the case.
 */
#if UNIX
static int
slowtime (refp)
long	*refp;
{
	int	status = FALSE;

	if (fileispipe) {
		long	temp = time((long *)0);

		status = (!ffhasdata() || (temp != *refp));
		if (status)
			*refp = temp;
	}
	return status;
}
#else
#define	slowtime(refp)	(fileispipe && !ffhasdata())
#endif

int
no_such_file(fname)
char *	fname;
{
	mlforce("[No such file \"%s\"]", fname);
	TTbeep();
	return FALSE;
}

#if VMS
static char *
version_of(char *fname)
{
	register char	*s = strchr(fname, ';');
	if (s == 0)
		s = fname + strlen(fname);
	return s;
}

static int
explicit_version(char *version)
{
	if (*version++ == ';') {
		if (isdigit(*version))
			return TRUE;
	}
	return FALSE;
}

static char *
resolve_filename(bp)
BUFFER	*bp;
{
	char	temp[NFILEN];
	ch_fname(bp, fgetname(ffp, temp));
	return bp->b_fname;
}
#endif

/*
 * Returns true if the given filename is the same as that of the referenced
 * buffer.  The 'lengthen' parameter controls whether we assume the filename is
 * already in canonical form, since that may be an expensive operation to do in
 * a loop.
 */
int
same_fname(fname, bp, lengthen)
char	*fname;
BUFFER	*bp;
int	lengthen;
{
	char	temp[NFILEN];

	if (fname == 0
	 || bp->b_fname == 0
	 || isInternalName(fname)
	 || isInternalName(bp->b_fname))
		return FALSE;

	if (lengthen)
		fname = lengthen_path(strcpy(temp, fname));

#if VMS
	/* ignore version numbers in this comparison unless both are given */
	if (is_vms_pathname(fname, FALSE)) {
		char	*bname = bp->b_fname,
			*s = version_of(bname),
			*t = version_of(fname);

		if (!explicit_version(s)
		 || !explicit_version(t))
			if ((s-bname) == (t-fname))
				return !strncmp(fname, bname, s-bname);
	}
#endif

	return !strcmp(fname, bp->b_fname);
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
        char bname[NBUFN+1];
	char fname[NFILEN];

	if ((s = mlreply_file("Replace with file: ", (TBUFF **)0, 
			FILEC_REREAD, fname)) != TRUE)
		return s;

	/* we want no errors or complaints, so mark it unchanged */
	b_clr_changed(curbp);
        s = readin(fname, TRUE, curbp, TRUE);
	curbp->b_bname[0] = EOS;	/* ...so 'unqname()' doesn't find me */
	makename(bname, fname);
	(void)unqname(bname, TRUE);
	set_bname(curbp, bname);
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
	char *actual;
	static	TBUFF	*last;

	if ((s = mlreply_file("Find file: ", &last, FILEC_READ|FILEC_EXPAND,
			fname)) == TRUE) {
		while ((actual = filec_expand()) != 0) {
			if ((s = getfile(actual, TRUE)) != TRUE)
				break;
		}
	}
	return s;
}

/* ARGSUSED */
int
viewfile(f, n)	/* visit a file in VIEW mode */
int f,n;
{
	char fname[NFILEN];	/* file user wishes to find */
	register int s;		/* status return */
	char	*actual;
	static	TBUFF	*last;

	if ((s = mlreply_file("View file: ", &last, FILEC_READ|FILEC_EXPAND,
			fname)) == TRUE) {
		while ((actual = filec_expand()) != 0) {
			if ((s = getfile(actual, FALSE)) != TRUE)
				break;
			/* if we succeed, put it in view mode */
			make_local_b_val(curwp->w_bufp,MDVIEW);
			set_b_val(curwp->w_bufp,MDVIEW,TRUE);
			markWFMODE(curwp->w_bufp);
		}
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
	        if ((s= mlreply_file("Insert file: ", &last,
				FILEC_READ|FILEC_PROMPT, fname)) != TRUE)
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
	char bname[NBUFN+1];	/* buffer name to put file */
	char nfname[NFILEN];	/* canonical form of 'fname' */

	/* user may have renamed buffer to look like filename */
	if ((bp = find_b_name(fname)) == NULL
	 || (strlen(fname) > NBUFN)) {

		/* It's not already here by that buffer name.
		 * Try to find it assuming we're given the file name.
		 */
		(void)lengthen_path(strcpy(nfname, fname));
		if (is_internalname(nfname)) {
			mlforce("[Buffer not found]");
			return FALSE;
		}
	        for_each_buffer(bp) {
			/* is it here by that filename? */
	                if (same_fname(nfname, bp, FALSE)) {
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
		while ((bp = find_b_name(bname)) != NULL) {
			if ( !b_is_changed(bp) && is_empty_buf(bp) &&
			    		!ffexists(bp->b_fname)) {
				/* empty and unmodified -- then it's okay 
					to re-use this buffer */
				bp->b_active = FALSE;
				return readin(fname, lockfl, bp, TRUE) &&
						swbuffer(bp);
			}
			/* old buffer name conflict code */
			if (unqname(bname,!clexec) || clexec)
				continue;
			hst_glue(' ');
			s = mlreply("Will use buffer name: ", bname, sizeof(bname));
	                if (s == ABORT)
	                        return s;
			if (s == FALSE || bname[0] == EOS)
		                makename(bname, fname);
	        }
		/* okay, we've got a unique name -- create it */
		if (bp==NULL && (bp=bfind(bname, 0))==NULL) {
			mlforce("[Cannot create buffer]");
			TTbeep();
	                return FALSE;
	        }
		/* switch and read it in. */
		ch_fname(bp, nfname);
	}
	return swbuffer(bp);
}

int
getfile(fname, lockfl)
char *fname;		/* file name to find */
int lockfl;		/* check the file for locks? */
{
        register BUFFER *bp;

	/* if there are no path delimiters in the name, then the user
		is likely asking for an existing buffer -- try for that
		first */
        if (strlen(fname) < NBUFN
	 && !maybe_pathname(fname)
	 && (bp = find_b_name(fname)) != NULL) {
		return swbuffer(bp);
	}

	/* oh well.  canonicalize the name, and try again */
	return getfile2(fname, lockfl);
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
		set_b_val(bp, MDDOS, doslines MORETHAN unixlines);
		bp->b_bytecount -= doslines;
	}
}

/*
 * Forces the current buffer to be in DOS-mode, stripping any trailing CR's.
 * ( any argument forces non-DOS mode, trailing CR's still stripped )
 */
/*ARGSUSED*/
int
set_dosmode(f,n)
int	f,n;
{
	make_local_b_val(curbp, MDDOS);
	set_b_val(curbp, MDDOS, !f);
	guess_dosmode(curbp);
	set_b_val(curbp, MDDOS, !f);
	markWFMODE(curbp);
	return TRUE;
}
#endif

/*
 *	Read file "fname" into a buffer, blowing away any text
 *	found there.  Returns the final status of the read.
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

	if (bp == 0)				/* doesn't hurt to check */
		return FALSE;

#if	FILOCK
	if (lockfl && lockchk(fname) == ABORT)
		return ABORT;
#endif
#if	CRYPT
	if ((s = resetkey(bp, fname)) != TRUE)
		return s;
#endif
        if ((s=bclear(bp)) != TRUE)             /* Might be old.        */
                return s;
	b_clr_flags(bp, BFINVS|BFCHG);
	ch_fname(bp,fname);
#if DOSFILES
	make_local_b_val(bp,MDDOS);
	set_b_val(bp, MDDOS, CRLF_LINES && global_b_val(MDDOS) );
#endif
	make_local_b_val(bp,MDNEWLINE);
	set_b_val(bp, MDNEWLINE, TRUE);		/* assume we've got it */

	/* turn off ALL keyboard translation in case we get a dos error */
	TTkclose();

        if ((s = ffropen(fname)) == FIOERR) {	/* Hard file error.      */
			/* do nothing -- error has been reported,
				and it will appear as empty buffer */
			;
        } else if (s == FIOFNF) {		/* File not found.      */
                if (mflg)
			mlwrite("[New file]");
#if DOSFILES
		set_b_val(bp, MDDOS, CRLF_LINES && global_b_val(MDDOS));
#endif
        } else {

        	if (mflg)
			mlforce("[Reading %s ]", fname);
#if VMS
		if (!isInternalName(bp->b_fname))
			fname = resolve_filename(bp);
#endif
		/* read the file in */
        	nline = 0;
#if VMALLOC
		/* we really think this stuff is clean... */
		odv = doverifys;
		doverifys = 0;
#endif
#if OPT_WORKING
		max_working = cur_working = old_working = 0;
#endif
#if ! MSDOS && !OPT_MAP_MEMORY
		if (fileispipe || (s = quickreadf(bp, &nline)) == FIOMEM)
#endif
			s = slowreadf(bp, &nline);
#if VMALLOC
		doverifys = odv;
#endif
#if OPT_WORKING
		cur_working = 0;
#endif
		if (s == FIOERR) {
			;
		} else {

			if (s == FIOFUN)	/* last line is incomplete */
				set_b_val(bp, MDNEWLINE, FALSE);
			b_clr_changed(bp);
#if FINDERR
			if (fileispipe == TRUE)
				set_febuff(get_bname(bp));
#endif
        		(void)ffclose();	/* Ignore errors.       */
			if (mflg)
				readlinesmsg(nline, s, fname, ffronly(fname));

			/* set read-only mode for read-only files */
			if (isShellOrPipe(fname) ||
			    (global_g_val(GMDRONLYVIEW) && ffronly(fname) )) {
				make_local_b_val(bp, MDVIEW);
				set_b_val(bp, MDVIEW, TRUE);
			}
	
			bp->b_active = TRUE;
		}

	}

	/* set C mode for C files */
	make_local_b_val(bp, MDCMOD); /* make it local for all, so that
					subsequent changes to global value
					will _not_ affect this buffer */
	set_b_val(bp, MDCMOD, (global_b_val(MDCMOD) && has_C_suffix(bp)));

	TTkopen();	/* open the keyboard again */

	for_each_window(wp) {
			if (wp->w_bufp == bp) {
		wp->w_line.l = lFORW(buf_head(bp));
		wp->w_dot.l  = lFORW(buf_head(bp));
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

#if PROC
	if (s == TRUE) { 
	    static int readhooking;
	    if (!readhooking && *readhook) {
		    readhooking = TRUE;
		    run_procedure(readhook);
		    readhooking = FALSE;
	    }
	}
#endif
	return (s != FIOERR);
}

#if ! MSDOS && !OPT_MAP_MEMORY
int
quickreadf(bp, nlinep)
register BUFFER *bp;
int *nlinep;
{
        register UCHAR *textp;
        UCHAR *countp;
	L_NUM nlines;
        int incomplete = FALSE;
	B_COUNT len, nbytes;

	if ((len = ffsize()) < 0) {
	    	mlforce("[Can't size file]");
		TTbeep();
		return FIOERR;
	}

	/* avoid malloc(0) problems down below; let slowreadf() do the work */
	if (len == 0)
		return FIOMEM;
#if OPT_WORKING
	max_working = len;
#endif
	/* leave an extra byte at the front, for the length of the first
		line.  after that, lengths go in place of the newline at
		the end of the previous line */
	bp->b_ltext = castalloc(UCHAR, (ALLOC_T)(len + 2));
	if (bp->b_ltext == NULL)
		return FIOMEM;

	if ((len = ffread((char *)&bp->b_ltext[1], len)) < 0) {
		FreeAndNull(bp->b_ltext);
		mlerror("reading");
		return FIOERR;
	}

#if CRYPT
	if (b_val(bp, MDCRYPT)
	 && bp->b_key[0]) {	/* decrypt the file */
	 	char	temp[NPAT];
		(void)strcpy(temp, bp->b_key);
		ue_crypt((char *)NULL, 0);
		ue_crypt(temp, (int)strlen(temp));
		ue_crypt((char *)&bp->b_ltext[1], len);
	}
#endif

	/* loop through the buffer, replacing all newlines with the
		length of the _following_ line */
	bp->b_ltext_end = bp->b_ltext + len + 1;
	countp = bp->b_ltext;
	textp = countp + 1;
	nbytes = len;
        nlines = 0;

	if (textp[len-1] != '\n') {
		textp[len++] = '\n';
		set_b_val(bp, MDNEWLINE, FALSE);
	}

	while (len--) {
		if (*textp == '\n') {
			if (textp - countp >= 255) {
				UCHAR *np;
#if OPT_WORKING
				max_working = bp->b_ltext_end - countp;
#endif
				len = (B_COUNT)(countp - bp->b_ltext);
				incomplete = TRUE;
				/* we'll re-read the rest later */
				if (len)  {
					ffseek(len);
					np = castrealloc(UCHAR, bp->b_ltext, (ALLOC_T)len);
				} else {
					np = NULL;
				}
				if (np == NULL) {
					ffrewind();
					FreeAndNull(bp->b_ltext);
					return FIOMEM;
				}
				bp->b_ltext = np;
				bp->b_ltext_end = np + len + 1;
				nbytes = len;
				break;
			}
			*countp = textp - countp - 1;
			countp = textp;
			nlines++;
		}
		++textp;
	}

	if (nlines == 0) {
		ffrewind();
		FreeAndNull(bp->b_ltext);
		incomplete = TRUE;
	} else {
		/* allocate all of the line structs we'll need */
		bp->b_LINEs = typeallocn(LINE,nlines);
		if (bp->b_LINEs == NULL) {
			FreeAndNull(bp->b_ltext);
			ffrewind();
			return FIOMEM;
		}
		bp->b_LINEs_end = bp->b_LINEs + nlines;
		bp->b_bytecount = nbytes;
		bp->b_linecount = nlines;
		b_set_counted(bp);

		/* loop through the buffer again, creating
			line data structure for each line */
		{
			register LINE *lp;
#if !SMALLER
			L_NUM lineno = 0;
#endif
			lp = bp->b_LINEs;
			textp = bp->b_ltext;
			while (lp != bp->b_LINEs_end) {
#if !SMALLER
				lp->l_number = ++lineno;
#endif
				lp->l_used = *textp;
				lp->l_size = *textp + 1;
				lp->l_text = (char *)textp + 1;
				set_lforw(lp, lp + 1);
				set_lback(lp, lp - 1);
				lsetclear(lp);
				lp->l_nxtundo = null_ptr;
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
			set_lforw(lp, buf_head(bp));
			set_lback(buf_head(bp), lp);

			/* connect the front of the list */
			set_lback(bp->b_LINEs, buf_head(bp));
			set_lforw(buf_head(bp), bp->b_LINEs);
		}
	}

	*nlinep = nlines;

	if (incomplete)
		return FIOMEM;
#if DOSFILES
	guess_dosmode(bp);
#endif
	return b_val(bp, MDNEWLINE) ? FIOSUC : FIOFUN;
}

#endif /* ! MSDOS */

int
slowreadf(bp, nlinep)
register BUFFER *bp;
int *nlinep;
{
	int s;
	int len;
#if DOSFILES
	int	doslines = 0,
		unixlines = 0;
#endif
#if UNIX || MSDOS || OS2	/* i.e., we can read from a pipe */
	int	flag = 0;
	int	done_update = FALSE;
#endif
#if UNIX
	long	last_updated = time((long *)0);
#endif
	b_set_counted(bp);	/* make 'addline()' do the counting */
        while ((s = ffgetline(&len)) <= FIOSUC) {
#if DOSFILES
		/*
		 * Strip CR's if we are reading in DOS-mode.  Otherwise,
		 * keep any CR's that we read.
		 */
		if (global_b_val(MDDOS)) {
			if (fline[len-1] == '\r') {
				len--;
				doslines++;
			} else {
				unixlines++;
			}
		}
#endif
		if (addline(bp,fline,len) != TRUE) {
                        s = FIOMEM;             /* Keep message on the  */
                        break;                  /* display.             */
                } 
#if UNIX || MSDOS || OS2
		else {
                	/* reading from a pipe, and internal? */
			if (slowtime(&last_updated)) {
				register WINDOW *wp;

				flag |= (WFEDIT|WFFORCE);

				if (!done_update || bp->b_nwnd > 1)
					flag |= WFHARD;
				for_each_window(wp) {
			                if (wp->w_bufp == bp) {
			                        wp->w_line.l=
							lFORW(buf_head(bp));
			                        wp->w_dot.l =
							lBACK(buf_head(bp));
			                        wp->w_dot.o = 0;
						wp->w_flag |= flag;
						wp->w_force = -1;
			                }
			        }

				/* track changes in dosfile as lines arrive */
#if DOSFILES
				if (global_b_val(MDDOS))
					set_b_val(bp, MDDOS, 
						doslines MORETHAN unixlines);
#endif
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
		if (s == FIOFUN) {
			set_b_val(bp, MDNEWLINE, FALSE);
			break;
		}
        }
#if DOSFILES
	if (global_b_val(MDDOS))
		set_b_val(bp, MDDOS, doslines MORETHAN unixlines);
#endif
	return s;
}

/* utility routine for no. of lines read */
static void
readlinesmsg(n,s,f,rdo)
int n;
int s;
char *f;
int rdo;
{
	char fname[NFILEN];
	char *m;
	f = shorten_path(strcpy(fname,f),TRUE);
	switch(s) {
		case FIOFUN:	m = "INCOMPLETE LINE, ";break;
		case FIOERR:	m = "I/O ERROR, ";	break;
		case FIOMEM:	m = "OUT OF MEMORY, ";	break;
		case FIOABRT:	m = "ABORTED, ";	break;
		default:	m = "";			break;
	}
	if (!global_b_val(MDTERSE))
		mlwrite("[%sRead %d line%s from \"%s\"%s]", m,
			n, PLURAL(n), f, rdo ? "  (read-only)":"" );
	else
		mlforce("[%s%d lines]",m,n);
}

/*
 * Take a (null-terminated) file name, and from it
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
	register char *fcp;
        register char *bcp;
	register int j;

#if VMS
	if (is_vms_pathname(fname, TRUE)) {
		(void)strcpy(bname, "NoName");
		return;
	}
	if (is_vms_pathname(fname, FALSE)) {
		for (fcp = fname + strlen(fname);
			fcp > fname && !strchr(":]", fcp[-1]);
				fcp--)
				;
		(void)strncpy(bname, fcp, NBUFN);
		if (bcp = strchr(bname, ';'))	/* strip version */
			*bcp = EOS;
		(void)mklower(bname);
		return;
	}
#endif
	fcp = &fname[strlen(fname)];
	/* trim trailing whitespace */
	while (fcp != fname && (isblank(fcp[-1])
#if UNIX || MSDOS || OS2 /* trim trailing slashes as well */
					 || slashc(fcp[-1])
#endif
							) )
                *(--fcp) = EOS;
	fcp = fname;
	/* trim leading whitespace */
	while (isblank(*fcp))
		fcp++;

#if     UNIX || MSDOS || VMS || OS2
	bcp = bname;
	if (isShellOrPipe(fcp)) { 
		/* ...it's a shell command; bname is first word */
		*bcp++ = SHPIPE_LEFT[0];
		do {
			++fcp;
		} while (isspace(*fcp));
		(void)strncpy(bcp, fcp, NBUFN);
		for (j = 1; j < NBUFN; j++) {
			if (isspace(*bcp)) {
				*bcp = EOS;
				break;
			}
			bcp++;
		}
		return;
	}

	(void)strncpy(bcp, pathleaf(fcp), NBUFN);

#if	UNIX
	/* UNIX filenames can have any characters (other than EOS!).  Refuse
	 * (rightly) to deal with leading/trailing blanks, but allow embedded
	 * blanks.  For this special case, ensure that the buffer name has no
	 * blanks, otherwise it is difficult to reference from commands.
	 */
	for (j = 0; j < NBUFN; j++) {
		if (*bcp == EOS)
			break;
		if (isspace(*bcp))
			*bcp = '-';
		bcp++;
	}
#endif

#else	/* !(UNIX||VMS||MSDOS) */

	bcp = fcp + strlen(fcp);
#if     AMIGA || ST520
	while (bcp!=fcp && bcp[-1]!=':' && !slashc(bcp[-1]))
                --bcp;
#endif
#if     CPM
        while (bcp!=fcp && bcp[-1]!=':')
                --bcp;
#endif
	{
		register char *cp2 = bname;

		do {
			if (cp2 == bname+NBUFN)
				break;
			*cp2 = EOS;
			if (*bcp == ';')
				break;
		} while ((*cp2++ = *bcp++) != EOS);
	}
#endif
}

int
unqname(name,ok_to_ask)	/* make sure a buffer name is unique */
char *name;	/* name to check on */
int ok_to_ask;  /* prompts allowed? */
{
	register char *sp;
	register SIZE_T	j;
	int	user_okayed = FALSE;

	/* check to see if it is in the buffer list */
	while (find_b_name(name) != NULL) {
		user_okayed = FALSE;

		/* "strnlen()", if there were such a thing */
		for (j = 0; (j < NBUFN) && (name[j] != EOS); j++)
			;
		if (j == 0)
			j = strlen(strcpy(name, "NoName"));

		sp = name+j-1;	/* point to last char of name */

		if (j > 2 && sp[-1] == '-') {
			if (*sp == '9')
				*sp = 'A';
			else
				*sp += 1;
			if (!isdigit(*sp) && !isupper(*sp))
				goto choosename;

		} else if (j < NBUFN-2)  {
			(void)strncpy(sp+1, "-1", NBUFN-j);
		} else {
		choosename:
			if (ok_to_ask) {
				do {
					hst_glue(' ');
					(void)mlreply(
					"Choose a unique buffer name: ",
						 name, NBUFN+1);
				} while (name[0] == EOS);
				user_okayed = TRUE;
			} else { /* can't ask, just overwrite end of name */
				while (j >= NBUFN-1) {
					j--;
					sp--;
				}
				(void)strncpy(sp+1, "-1", NBUFN-j);
			}
		}
	}
	return user_okayed;
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

	if (more_named_cmd()) {
	        if ((s= mlreply_file("Write to file: ", (TBUFF **)0, 
				FILEC_WRITE, fname)) != TRUE)
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
		TTbeep();
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
        REGION region;

	bsizes(bp);	/* make sure we have current count */
	/* starting at the beginning of the buffer */
	region.r_orig.l = lFORW(buf_head(bp));
        region.r_orig.o = 0;
        region.r_size   = bp->b_bytecount;
        region.r_end    = bp->b_line;
 
	return writereg(&region, fn, msgf, bp);
}

int
writeregion()
{
        REGION region;
	int status;
        char fname[NFILEN];

	if (end_named_cmd()) {
		if (mlyesno("Okay to write [possible] partial range") != TRUE) {
			mlwrite("Range not written");
			return FALSE;
		}
		(void)strcpy(fname, curbp->b_fname);
	} else {
	        if ((status = mlreply_file("Write region to file: ", 
			(TBUFF **)0, FILEC_WRITE|FILEC_PROMPT, fname)) != TRUE)
	                return status;
        }
        if ((status=getregion(&region)) == TRUE)
		status = writereg(&region, fname, TRUE, curbp);
	return status;
}


static int
writereg(rp, fn, msgf, bp)
REGION	*rp;
char    *fn;
int 	msgf;
BUFFER	*bp;
{
        register int    s;
        register LINE   *lp;
        register int    nline;
	register int i;
	char	fname[NFILEN];
	long	nchar;
	char *	ending =
#if DOSFILES
			b_val(bp, MDDOS) ? "\r\n" : "\n"
#else
#if ST520
			"\r\n"
#else	/* UNIX */
			"\n"
#endif
#endif	/* DOSFILES */
		;
	C_NUM	offset = rp->r_orig.o;

	/* this is adequate as long as we cannot write parts of lines */
	int	whole_file = (l_ref(rp->r_orig.l) == lForw(buf_head(bp)))
			  && (same_ptr(rp->r_end.l, buf_head(bp)));

	if (is_internalname(fn)) {
		mlforce("[No filename]");
		TTbeep();
		return FALSE;
	}

	if (isShellOrPipe(fn)
	 && bp->b_fname != 0
	 && !strcmp(fn, bp->b_fname)
	 && mlyesno("Are you sure (this was a pipe-read)") != TRUE) {
		mlwrite("File not written");
		return FALSE;
	}

#if PROC
	{ 
	    static int writehooking;
	    if (!writehooking && *writehook) {
		    writehooking = TRUE;
		    run_procedure(writehook);
		    writehooking = FALSE;
	    }
	}
#endif

	fn = lengthen_path(strcpy(fname, fn));
	if (same_fname(fn, bp, FALSE) && b_val(bp,MDVIEW)) {
		mlforce("[Can't write-back from view mode]");
		TTbeep();
		return FALSE;
	}

#if	CRYPT
	if ((s = resetkey(curbp, fn)) != TRUE)
		return s;
#endif

#ifdef MDCHK_MODTIME
	if ( ! inquire_modtime( bp, fn ) )
		return FALSE;
#endif  
        if ((s=ffwopen(fn)) != FIOSUC)       /* Open writes message. */
                return FALSE;

	/* tell us we're writing */
	if (msgf == TRUE)
		mlwrite("[Writing...]");

	CleanToPipe();

        lp = l_ref(rp->r_orig.l);
        nline = 0;                              /* Number of lines     */
        nchar = 0;                              /* Number of chars     */

	/* first (maybe partial) line and succeeding whole lines */
        while ((rp->r_size+offset) >= llength(lp)+1) {
		register C_NUM	len = llength(lp) - offset;
		register char	*text = lp->l_text + offset;

		/* If this is the last line (and no fragment will be written
		 * after the line), allow 'newline' mode to suppress the
		 * trailing newline.
		 */
		if ((rp->r_size -= (len + 1)) <= 0
		 && !b_val(bp,MDNEWLINE))
			ending = "";
                if ((s = ffputline(text, len, ending)) != FIOSUC)
			goto out;

                ++nline;
		nchar += len + 1;
		offset = 0;
                lp = lforw(lp);
        }

	/* last line (fragment) */
	if (rp->r_size > 0) {
		for (i = 0; i < rp->r_size; i++)
		        if ((s = ffputc(lgetc(lp,i))) != FIOSUC)
		                goto out;
		nchar += rp->r_size;
		++nline;	/* it _looks_ like a line */
	}

 out:
        if (s == FIOSUC) {                      /* No write error.      */
#if VMS
		if (same_fname(fn, bp, FALSE))
			fn = resolve_filename(bp);
#endif
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
				mlforce("[%s %d line%s %ld char%s to \"%s\"]", 
					action, nline, PLURAL(nline),
					nchar, PLURAL(nchar), fn);
			} else {
				mlforce("[%d lines]", nline);
			}
                }
        } else {                                /* Ignore close error   */
                (void)ffclose();                /* if a write error.    */
	}
	if (whole_file)				/* patch: do I need this? */
		bp->b_linecount = nline;

	CleanAfterPipe(TRUE);

        if (s != FIOSUC)                        /* Some sort of error.  */
                return FALSE;

#ifdef MDCHK_MODTIME
	set_modtime(bp, fn);
#endif
	imply_alt(fn, whole_file, FALSE);
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
	if ((s = resetkey(curbp, fn)) != TRUE)
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
		i = KbSize(ukb,kp);
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
					nline, PLURAL(nline), fn);
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

	if (end_named_cmd()) {
		return showcpos(FALSE,1);
	}

        if ((s = mlreply_file("Name: ", (TBUFF **)0, FILEC_UNKNOWN, fname)) 
						== ABORT)
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
	fast_ptr LINEPTR prevp;
	fast_ptr LINEPTR newlp;
	fast_ptr LINEPTR nextp;
        register BUFFER *bp;
        register int    s;
        int    nbytes;
        register int    nline;

        bp = curbp;                             /* Cheap.               */
	b_clr_flags(bp, BFINVS);		/* we are not temporary*/
	if (!haveffp) {
	        if ((s=ffropen(fname)) == FIOERR) /* Hard file open.      */
	                goto out;
	        if (s == FIOFNF)		/* File not found.      */
			return no_such_file(fname);
#if	CRYPT
		if ((s = resetkey(curbp, fname)) != TRUE)
			return s;
#endif
	        mlwrite("[Inserting...]");
		CleanToPipe();

	} else { /* we already have the file pointer */
		ffp = haveffp;
	}
	prevp = DOT.l;
	DOT.o = 0;
	MK = DOT;

	nline = 0;
	nextp = null_ptr;
	while ((s=ffgetline(&nbytes)) <= FIOSUC) {
#if DOSFILES
		if (b_val(curbp,MDDOS)
		 && (nbytes > 0)
		 && fline[nbytes-1] == '\r')
			nbytes--;
#endif
		if (!belowthisline) {
			nextp = prevp;
			prevp = lBACK(prevp);
		}

		if (add_line_at(curbp, prevp, fline, nbytes) != TRUE) {
			s = FIOMEM;		/* Keep message on the	*/
			break;			/* display.		*/
		}
		newlp = lFORW(prevp);
		tag_for_undo(newlp);
		prevp = belowthisline ? newlp : nextp;
		++nline;
		if (s < FIOSUC)
			break;
	}
	if (!haveffp) {
		CleanAfterPipe(FALSE);
		(void)ffclose();		/* Ignore errors.	*/
		readlinesmsg(nline,s,fname,FALSE);
	}
out:
	/* advance to the next line and mark the window for changes */
	DOT.l = lFORW(DOT.l);

	/* copy window parameters back to the buffer structure */
	copy_traits(&(curbp->b_wtraits), &(curwp->w_traits));

	imply_alt(fname, FALSE, FALSE);
	chg_buff (curbp, WFHARD);

	return (s != FIOERR);
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

        nline = 0;
#if	CRYPT
	if ((s = resetkey(curbp, fname)) == TRUE)
#endif
	{
        	mlwrite("[Reading...]");
		CleanToPipe();
		while ((s=ffgetline(&nbytes)) <= FIOSUC) {
			for (i=0; i<nbytes; ++i)
				if (!kinsert(fline[i]))
					return FIOMEM;
			if ((s == FIOSUC) && !kinsert('\n'))
				return FIOMEM;
			++nline;
			if (s < FIOSUC)
				break;
		}
		CleanAfterPipe(FALSE);
	}
	kdone();
        (void)ffclose();                        /* Ignore errors.       */
	readlinesmsg(nline,s,fname,FALSE);

out:
	return (s != FIOERR);
}

#if UNIX

/* called on hangups, interrupts, and quits */
/* This code is definitely not production quality, or probably very
	robust, or probably very secure.  I whipped it up to save
	myself while debugging...		pgf */
/* on the other hand, it has worked for well over two years now :-) */
SIGT
imdying(ACTUAL_SIG_ARGS)
ACTUAL_SIG_DECL
{
#if HAVE_MKDIR
	static char dirnam[NSTRING] = "/tmp/vileDXXXXXX";
#else
	static char dirnam[NSTRING] = "/tmp";
	char temp[NFILEN];
#endif
	char filnam[NFILEN];
	char cmd[80];
	BUFFER *bp;
	char *np;
	int wrote = 0;
	int created = FALSE;
	extern char *mktemp P(( char * ));

#if APOLLO
	extern	char	*getlogin(void);
	static	int	i_am_dead;
#endif	/* APOLLO */

#if OPT_WORKING && defined(SIGALRM)
	(void)signal(SIGALRM, SIG_IGN);
#endif

#if APOLLO
	if (i_am_dead++)	/* prevent recursive faults */
		_exit(signo);
	(void)lsprintf(cmd,
		"(echo signal %d killed vile;/com/tb %d)| /bin/mail %s",
		signo, getpid(), getlogin());
	(void)system(cmd);
#endif	/* APOLLO */

	/* write all modified buffers to the temp directory */
	set_global_g_val(GMDIMPLYBUFF,FALSE);	/* avoid side-effects! */
	for_each_buffer(bp) {
		if (!b_is_temporary(bp) &&
			bp->b_active == TRUE &&
			b_is_changed(bp)) {
#if HAVE_MKDIR
			if (!created) {
				(void)mktemp(dirnam);
				if(mkdir(dirnam,0700) != 0) {
					vttidy(FALSE);
					ExitProgram(BADEXIT);
				}
				created = TRUE;
			}
			(void)pathcat(filnam, dirnam, get_bname(bp));
#else
			(void)pathcat(filnam, dirnam,
				strcat(strcpy(temp, "V"), get_bname(bp)));
#endif
			set_b_val(bp,MDVIEW,FALSE);
			if (writeout(filnam,bp,FALSE) != TRUE) {
				vttidy(FALSE);
				ExitProgram(BADEXIT);
			}
			wrote++;
		}
	}
	if (wrote) {
		if ((np = getenv("LOGNAME")) != 0
		 || (np = getenv("USER")) != 0) {
#if HAVE_GETHOSTNAME
			char hostname[128];
			if (gethostname(hostname, sizeof(hostname)) < 0)
				(void)strcpy(hostname, "unknown");
			hostname[sizeof(hostname)-1] = EOS;
#endif
			(void)lsprintf(cmd, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
			"(echo To: ", np, ";", "echo Subject: vile died; ",
			"echo Files saved in directory ", dirnam,
#if HAVE_GETHOSTNAME
			" on host ", hostname, 
#else
			", host unknown", "",
#endif
			": ;",
#if HAVE_MKDIR
			"ls -a ", dirnam, " | sort -r)",
#else
			"ls ", dirnam, "/V* )",
#endif
			" | /bin/mail ", np);
			(void)system(cmd);




		}
	}
	vttidy(FALSE);
	if (signo > 2) {
		abort();
	} else {
	    	if (wrote)
		    ExitProgram(BADEXIT);
		else
		    ExitProgram(GOODEXIT);
	}

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
int
resetkey(bp, fname)	/* reset the encryption key if needed */
BUFFER	*bp;
char	*fname;
{
	register int s;	/* return status */

	/* turn off the encryption flag */
	cryptflag = FALSE;

	/* if we are in crypt mode */
	if (b_val(bp, MDCRYPT)) {
		char	temp[NFILEN];

		/* don't automatically inherit key from other buffers */
		if (bp->b_key[0] != EOS
		 && strcmp(lengthen_path(strcpy(temp, fname)), bp->b_fname)) {
			char	prompt[80];
			(void)lsprintf(prompt,
				"Use crypt-key from %s", get_bname(bp));
			s = mlyesno(prompt);
			if (s != TRUE)
				return (s == FALSE);
		}

		/* make a key if we don't have one */
		if (bp->b_key[0] == EOS) {
			s = ue_makekey(bp->b_key, sizeof(bp->b_key));
			if (s != TRUE)
				return (s == FALSE);
		}

		/* let others know... */
		cryptflag = TRUE;

		/* and set up the key to be used! */
		/* de-encrypt it */
		ue_crypt((char *)NULL, 0);
		ue_crypt(bp->b_key, (int)strlen(bp->b_key));

		/* re-encrypt it...seeding it to start */
		ue_crypt((char *)NULL, 0);
		ue_crypt(bp->b_key, (int)strlen(bp->b_key));
	}

	return TRUE;
}
#endif
