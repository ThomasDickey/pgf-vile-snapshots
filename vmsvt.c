/*
 *  Advanced VMS terminal driver
 *
 *  Knows about any terminal defined in SMGTERMS.TXT and TERMTABLE.TXT
 *  located in SYS$SYSTEM.
 *
 *  Author:  Curtis Smith
 *  Last Updated: 07/14/87
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/vmsvt.c,v 1.17 1994/11/29 04:02:03 pgf Exp $
 *
 */

#include	"estruct.h"		/* Emacs' structures		*/
#include	"edef.h"		/* Emacs' definitions		*/

#if	VMSVT

#include	 <descrip.h>		/* Descriptor definitions	*/
#include	<iodef.h>		/* to get IO$_SENSEMODE		*/
#include	<ttdef.h>		/* to get TT$_UNKNOWN		*/
#include	<smgtrmptr.h>		/* to get SMG$K_??? definitions	*/

/** Forward references **/
extern	void	vmsopen	(void);
extern	void	vmskopen(void);
extern	void	vmskclose(void);
extern	void	vmsmove	(int, int);
extern	void	vmseeol	(void);
extern	void	vmseeop	(void);
extern	void	vmsbeep	(void);
extern	void	vmsrev	(int);
extern	int	vmscres	(void);

extern	int	eolexist, revexist;
extern	char	sres[];

#if OPT_COLOR
extern	int	vmsfcol(int);
extern	int	vmsbcol(int);
#endif
extern	void	tcapscroll (int, int, int);

/** SMG stuff (just like termcap) */
static	int	termtype;
static	char *	begin_reverse;
static	char *	end_reverse;
static	char *	erase_to_end_line;
static	char *	erase_whole_display;

static	char *	delete_line;
static	char *	insert_line;
static	char *	scroll_forw;
static	char *	scroll_back;
static	char *	scroll_regn;

/* Dispatch table. All hard fields just point into the terminal I/O code. */
TERM	term	= {
	24,				/* Max number of rows allowable */
	/* Filled in */ 0,		/* Current number of rows used	*/
	132,				/* Max number of columns	*/
	/* Filled in */ 0,		/* Current number of columns	*/
	64,				/* Min margin for extended lines*/
	8,				/* Size of scroll region	*/
	100,				/* # times thru update to pause */
	vmsopen,			/* Open terminal at the start	*/
	ttclose,			/* Close terminal at end	*/
	vmskopen,			/* Open keyboard		*/
	vmskclose,			/* Close keyboard		*/
	ttgetc,				/* Get character from keyboard	*/
	ttputc,				/* Put character to display	*/
	tttypahead,			/* char ready for reading	*/
	ttflush,			/* Flush output buffers		*/
	vmsmove,			/* Move cursor, origin 0	*/
	vmseeol,			/* Erase to end of line		*/
	vmseeop,			/* Erase to end of page		*/
	vmsbeep,			/* Beep				*/
	vmsrev,				/* Set reverse video state	*/
	vmscres				/* Change screen resolution	*/
#if	OPT_COLOR
	, vmsfcol,			/* Set forground color		*/
	vmsbcol				/* Set background color		*/
#endif
	, NULL				/* set at init-time		*/
};

/***
 *  ttputs  -  Send a string to ttputc
 *
 *  Nothing returned
 ***/
ttputs(string)
char * string;				/* String to write		*/
{
	if (string)
		while (*string != EOS)
			ttputc(*string++);
}

/***
 *  putpad_tgoto - 2-argument request
 *
 *  Nothing returned
 ***/
static void putpad_tgoto(request_code, parm1, parm2)
{
	char buffer[32];
	int ret_length;
	static int max_buffer_length = sizeof(buffer);
	static int arg_list[3] = { 2 };
	register char * cp;
	
	register int i;

	/* Set the arguments into the arg_list array
	 */
	arg_list[1] = parm1;
	arg_list[2] = parm2;

	if ((smg$get_term_data(		/* Get terminal data		*/
		&termtype,		/* Terminal table address	*/
		&request_code,		/* Request code			*/
		&max_buffer_length,	/* Maximum buffer length	*/
		&ret_length,		/* Return length		*/
		buffer,			/* Capability data buffer	*/
		arg_list)		/* Argument list array		*/

	/* We'll know soon enough if this doesn't work		*/
			& 1) == 0) {
				ttputs("OOPS");
				return;
			}

	/* Send out resulting sequence				*/
	i = ret_length;
	cp = buffer;
	while (i-- > 0)
		ttputc(*cp++);
}


/***
 *  vmsmove  -  Move the cursor (0 origin)
 *
 *  Nothing returned
 ***/
void vmsmove (int row, int col)
{
	putpad_tgoto(SMG$K_SET_CURSOR_ABS, row+1, col+1);
}

/***
 *  vmsrev  -  Set the reverse video status
 *
 *  Nothing returned
 ***/
void vmsrev(int status)
{
	if (status)
		ttputs(begin_reverse);
	else 
		ttputs(end_reverse);
}

/***
 *  vmscres  -  Change screen resolution (which it doesn't)
 *
 *  Nothing returned
 ***/
vmscres()
{
	/* But it could.  For vt100/vt200s, one could switch from
	80 and 132 columns modes */
}


#if	OPT_COLOR
/***
 *  vmsfcol  -  Set the foreground color (not implemented)
 *
 *  Nothing returned
 ***/
vmsfcol()
{
}

/***
 *  vmsbcol  -  Set the background color (not implemented)
 *
 *  Nothing returned
 ***/
vmsbcol()
{
}
#endif

/***
 *  vmseeol  -  Erase to end of line
 *
 *  Nothing returned
 ***/
void vmseeol(void)
{
	ttputs(erase_to_end_line);
}


/***
 *  vmseeop  -  Erase to end of page (clear screen)
 *
 *  Nothing returned
 ***/
void vmseeop(void)
{
	ttputs(erase_whole_display);
}


/***
 *  vmsbeep  -  Ring the bell
 *
 *  Nothing returned
 ***/
void vmsbeep(void)
{
	ttputc(BEL);
}


/***
 *  vmsgetstr  -  Get an SMG string capability by name
 *
 *  Returns:	Escape sequence
 *		NULL	No escape sequence available
 ***/ 
char * vmsgetstr(request_code)
int request_code;			/* Request code			*/
{
	register char * result;
	static char seq_storage[1024];
	static char * buffer = seq_storage;
	static int arg_list[] = { 1, 1 };
	int max_buffer_length, ret_length;

	/*  Precompute buffer length */
	
	max_buffer_length = (seq_storage + sizeof(seq_storage)) - buffer;

	/* Get terminal commands sequence from master table */

	if ((smg$get_term_data(	/* Get terminal data		*/
		&termtype,	/* Terminal table address	*/
		&request_code,	/* Request code			*/
		&max_buffer_length,/* Maximum buffer length	*/
		&ret_length,	/* Return length		*/
		buffer,		/* Capability data buffer	*/
		arg_list)	/* Argument list array		*/

	/* If this doesn't work, try again with no arguments */
	
		& 1) == 0 && 

		(smg$get_term_data(	/* Get terminal data		*/
			&termtype,	/* Terminal table address	*/
			&request_code,	/* Request code			*/
			&max_buffer_length,/* Maximum buffer length	*/
			&ret_length,	/* Return length		*/
			buffer)		/* Capability data buffer	*/

	/* Return NULL pointer if capability is not available */
	
			& 1) == 0)
				return NULL;

	/* Check for empty result */
	if (ret_length == 0)
		return NULL;
	
	/* Save current position so we can return it to caller */

	result = buffer;

	/* NIL terminate the sequence for return */
	
	buffer[ret_length] = 0;

	/* Advance buffer */

	buffer += ret_length + 1;

	/* Return capability to user */
	return result;
}


/** I/O information block definitions **/
struct iosb {			/* I/O status block			*/
	short	i_cond;		/* Condition value			*/
	short	i_xfer;		/* Transfer count			*/
	long	i_info;		/* Device information			*/
};
struct termchar {		/* Terminal characteristics		*/
	char	t_class;	/* Terminal class			*/
	char	t_type;		/* Terminal type			*/
	short	t_width;	/* Terminal width in characters		*/
	long	t_mandl;	/* Terminal's mode and length		*/
	long	t_extend;	/* Extended terminal characteristics	*/
};
static struct termchar tc;	/* Terminal characteristics		*/

/***
 *  vmsgtty - Get terminal type from system control block
 *
 *  Nothing returned
 ***/
vmsgtty()
{
	short fd;
	int status;
	struct iosb iostatus;
	$DESCRIPTOR(devnam, "SYS$INPUT");

	/* Assign input to a channel */
	status = sys$assign(&devnam, &fd, 0, 0);
	if ((status & 1) == 0)
		ExitProgram(status);

	/* Get terminal characteristics */
	status = sys$qiow(		/* Queue and wait		*/
		0,			/* Wait on event flag zero	*/
		fd,			/* Channel to input terminal	*/
		IO$_SENSEMODE,		/* Get current characteristic	*/
		&iostatus,		/* Status after operation	*/
		0, 0,			/* No AST service		*/
		&tc,			/* Terminal characteristics buf */
		sizeof(tc),		/* Size of the buffer		*/
		0, 0, 0, 0);		/* P3-P6 unused			*/

	/* De-assign the input device */
	if ((sys$dassgn(fd) & 1) == 0)
		ExitProgram(status);

	/* Jump out if bad status */
	if ((status & 1) == 0)
		ExitProgram(status);
	if ((iostatus.i_cond & 1) == 0)
		ExitProgram(iostatus.i_cond);
}


/***
 *  vmsopen  -  Get terminal type and open terminal
 *
 *  Nothing returned
 ***/
void vmsopen(void)
{
	/* Get terminal type */
	vmsgtty();
	if (tc.t_type == TT$_UNKNOWN) {
		printf("Terminal type is unknown!\n");
		printf("Try set your terminal type with SET TERMINAL/INQUIRE\n");
		printf("Or get help on SET TERMINAL/DEVICE_TYPE\n");
		ExitProgram(3);
	}

	/* Access the system terminal definition table for the		*/
	/* information of the terminal type returned by IO$_SENSEMODE	*/
	if ((smg$init_term_table_by_type(&tc.t_type, &termtype) & 1) == 0) {
		return;
	}
		
	/* Set sizes */
	term.t_nrow = ((UINT) tc.t_mandl >> 24);
	term.t_ncol = tc.t_width;

	if (term.t_mrow < term.t_nrow)
		term.t_mrow = term.t_nrow;

	if (term.t_mcol < term.t_ncol)
		term.t_mcol = term.t_ncol;

	/* Get some capabilities */
	begin_reverse	= vmsgetstr(SMG$K_BEGIN_REVERSE);
	end_reverse	= vmsgetstr(SMG$K_END_REVERSE);
	revexist	= begin_reverse	!= NULL
		&&	  end_reverse	!= NULL;

	erase_to_end_line = vmsgetstr(SMG$K_ERASE_TO_END_LINE);
	eolexist = erase_whole_display != NULL;

	erase_whole_display = vmsgetstr(SMG$K_ERASE_WHOLE_DISPLAY);

	insert_line	= vmsgetstr(SMG$K_INSERT_LINE);		/* al */
	delete_line	= vmsgetstr(SMG$K_DELETE_LINE);		/* dl */
	scroll_forw	= vmsgetstr(SMG$K_SCROLL_FORWARD);	/* SF */
	scroll_back	= vmsgetstr(SMG$K_SCROLL_REVERSE);	/* SR */
	scroll_regn	= vmsgetstr(SMG$K_SET_SCROLL_REGION);	/* CS */

	/*
	 * I tried 'vmsgetstr()' for a VT100 terminal and it had no codes
	 * for insert_line or for delete_line.  (Have to work up a test for
	 * that).
	 */

	if (scroll_regn && scroll_back) {
		if (scroll_forw == NULL) /* assume '\n' scrolls forward */
			scroll_forw = "\n";
		term.t_scroll = tcapscroll_reg;
	} else if (delete_line && insert_line) {
		term.t_scroll = tcapscroll_delins;
	} else {
		term.t_scroll = NULL;
	}

	/* Set resolution */
	(void)strcpy(sres, "NORMAL");

	/* Open terminal I/O drivers */
	ttopen();
}


/***
 *  vmskopen  -  Open keyboard (not used)
 *
 *  Nothing returned
 ***/
void vmskopen(void)
{
}


/***
 *  vmskclose  -  Close keyboard (not used)
 *
 *  Nothing returned
 ***/
void vmskclose(void)
{
}

/***
 *  spal  -  Set palette type  (Are you kidding?)
 *
 *  Nothing returned
 ***/
void
spal(dummy)
char	*dummy;
{
}

/* copied/adapted from 'tcap.c' 19-apr-1993 dickey@software.org */

/* move howmany lines starting at from to to */
void
tcapscroll_reg(from,to,n)
int from, to, n;
{
	int i;
	if (to == from) return;
	if (to < from) {
		tcapscrollregion(to, from + n - 1);
		vmsmove(from + n - 1,0);
		for (i = from - to; i > 0; i--)
			ttputs(scroll_forw);
	} else { /* from < to */
		tcapscrollregion(from, to + n - 1);
		vmsmove(from,0);
		for (i = to - from; i > 0; i--)
			ttputs(scroll_back);
	}
	tcapscrollregion(0, term.t_nrow-1);
}

/* 
OPT_PRETTIER_SCROLL is prettier but slower -- it scrolls 
		a line at a time instead of all at once.
*/

/* move howmany lines starting at from to to */
void
tcapscroll_delins(from,to,n)
int from, to, n;
{
	int i;
	if (to == from) return;
	/* patch: should make this more like 'tcap.c', or merge logic somehow */
#if OPT_PRETTIER_SCROLL
	if (absol(from-to) > 1) {
		tcapscroll_delins(from, (from<to) ? to-1:to+1, n);
		if (from < to)
			from = to-1;
		else
			from = to+1;    
	}
#endif
	if (to < from) {
		vmsmove(to,0);
		for (i = from - to; i > 0; i--)
			ttputs(delete_line);
		vmsmove(to+n,0);
		for (i = from - to; i > 0; i--)
			ttputs(insert_line);
	} else {
		vmsmove(from+n,0);
		for (i = to - from; i > 0; i--)
			ttputs(delete_line);
		vmsmove(from,0);
		for (i = to - from; i > 0; i--)
			ttputs(insert_line);
	}
}

/* cs is set up just like cm, so we use tgoto... */
void
tcapscrollregion(top,bot)
int top,bot;
{
	putpad_tgoto(SMG$K_SET_SCROLL_REGION, top+1, bot+1);
}


#else

/***
 *  hellovms  -  Avoid error because of empty module
 *
 *  Nothing returned
 ***/
hellovms()
{
}

#endif
