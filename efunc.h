THIS FILE IS NO LONGER USED -- IT IS INCLUDED FOR HISTORICAL PURPOSES
SEE THE mktbls PROGRAM, AND THE FILE cmdtbl
/*	EFUNC.H:	MicroEMACS function declarations and names

		This file list all the C code functions used by MicroEMACS
	and the names to use to bind keys to them. To add functions,
	declare it here in both the extern function list and the name
	binding table.

*/

/*	External function declarations		*/

#if FINDERR
extern  int     finderr();     	/* look up and go to next error */
#endif
#if TAGS
extern  int     gototag();     	/* look up and go to a tag */
#endif
extern  int     operqreplace();     	/* do "query replace" on a region */
extern  int     opersreplace();     	/* do "replace" on a region */
extern  int     operlineqreplace();     /* do "query replace" on a line region */
extern  int     operlinesreplace();     /* do "replace" on a line region */
extern  int     operwrite();     	/* write a region to a file */
extern  int     showgmodes();     	/* show modes set for current buffer */
extern  int     showmodes();     	/* show modes set for current buffer */
extern  int     histbuff();     	/* switch to num buffer in hist list */
extern  int     altbuff();	     	/* switch to previous buffer */
extern  int     deltoeol();     	/* delete to end of line */
extern  int     chgtoeol();     	/* change to end of line */
extern  int     yankline();     	/* yank whole line */
extern  int     chgline();     		/* change whole line */
extern  int     chgchar();     		/* change character */
extern  int     join();      		/* join two line together */
extern  int     lastnonwhite();      	/* goto last non-whitespace on line */
extern  int     firstnonwhite();      	/* goto first non-whitespace on line */
extern  int     fcsrch();          	/* forw scan for char on line */
extern  int     bcsrch();          	/* back scan for char on line */
extern  int     fcsrch_to();          	/* forw scan up to char on line */
extern  int     bcsrch_to();          	/* back scan up to char on line */
extern  int     rep_csrch();          	/* repeat last scan for char on line */
extern  int     rev_csrch();          	/* reverse last scan for char on line */
extern  int     replacechar();          /* replace character under cursor */
extern  int     poswind();           	/* position window around cursor */
extern  int     overwrite();           	/* overwrite text (temp set OVER mode)*/
extern  int     undo();           	/* undo last command		*/
extern  int     lineundo();           	/* undo all changes to a line	*/
extern  int     dotcmdplay();           /* replay last command		*/
extern  int     operdel();              /* Delete with motion operator  */
extern  int     operlinedel();          /* Delete lines with motion operator  */
extern  int     operyank();             /* Yank with motion operator    */
extern  int     operlineyank();         /* Yank lines with motion operator    */
extern  int     operchg();              /* Change with motion operator  */
extern  int     operlinechg();          /* Change lines with motion operator  */
extern  int     operflip();             /* Flip case with motion operator  */
extern  int     operlower();             /* lower case with motion operator  */
extern  int     operupper();             /* upper case with motion operator  */
extern  int     operlshift();           /* Shift lines with motion operator  */
extern  int     operrshift();           /* Shift lines with motion operator  */
extern  int     esc();                /* Abort out of things          */
extern  int     writequit();            /* Write and Quit               */
extern  int     quit();                 /* Quit                         */
extern  int     quithard();             /* Quit, no questions asked     */
extern  int     ctlxlp();               /* Begin macro                  */
extern  int     ctlxrp();               /* End macro                    */
extern  int     ctlxe();                /* Execute macro                */
extern  int     fileread();             /* Get a file, read only        */
extern  int     filefind();		/* Get a file, read write       */
extern  int     filewrite();            /* Write a file                 */
extern  int     filesave();             /* Save current file            */
extern  int     filename();             /* Adjust file name             */
extern  int     getccol();              /* Get current column           */
extern  int     gotobol();              /* Move to start of line        */
extern  int     forwchar();             /* Move forward by characters   */
extern  int     gotoeol();              /* Move to end of line          */
extern  int     gotobos();              /* Move to beg of screen        */
extern  int     gotomos();              /* Move to mid of screen        */
extern  int     gotoeos();              /* Move to end of screen        */
extern  int     backchar();             /* Move backward by characters  */
extern  int     forwline();             /* Move forward by lines        */
extern  int     forwbline();            /* Move forward by lines, goto bol */
extern  int     backline();             /* Move backward by lines       */
extern  int     backbline();            /* Move backward by lines, goto bol */
extern  int     forwpage();             /* Move forward by pages        */
extern  int     forwhpage();            /* Move forward half page       */
extern  int     backpage();             /* Move backward by pages       */
extern  int     backhpage();            /* Move backward half page      */
extern  int     gotobob();              /* Move to start of buffer      */
extern  int     gotoeob();              /* Move to end of buffer        */
extern  int     setfillcol();           /* Set fill column.             */
extern  int     setnmmark();            /* Set named mark               */
extern  int     golinenmmark();         /* go to line of named mark     */
extern  int     goexactnmmark();        /* go exactly to named mark     */
extern  int     setmark();              /* Set mark                     */
extern  int     swapmark();             /* Swap "." and mark            */
extern  int     forwsearch();           /* Search forward               */
extern  int     backsearch();           /* Search backwards             */
extern	int	sreplace();		/* search and replace		*/
extern	int	qreplace();		/* search and replace w/query	*/
extern  int     showcpos();             /* Show the cursor position     */
extern  int     nextwind();             /* Move to the next window      */
extern  int     prevwind();             /* Move to the previous window  */
extern  int     onlywind();             /* Make current window only one */
extern  int     splitwind();            /* Split current window         */
extern  int     mvdnwind();             /* Move window down             */
extern  int     mvupwind();             /* Move window up               */
extern  int     mvdnnxtwind();          /* Move next window down        */
extern  int     mvupnxtwind();          /* Move next window up          */
extern  int     enlargewind();          /* Enlarge display window.      */
extern  int     shrinkwind();           /* Shrink window.               */
extern  int     listbuffers();          /* Display list of buffers      */
extern  int     togglelistbuffers();    /* Display/unDisplay list of buffers*/
extern  int     usebuffer();            /* Switch a window to a buffer  */
extern  int     killbuffer();           /* Make a buffer go away.       */
extern  int     reposition();           /* Reposition window            */
extern  int     refresh();              /* Refresh the screen           */
extern  int     tab();                  /* Insert tab                   */
extern  int     settab();               /* set tab stops                */
extern  int     newline();              /* Insert CR-LF                 */
extern  int     opendown();             /* Open up a blank line below   */
extern  int     openup();               /* Open up a blank line above   */
extern  int     append();               /* go into insert mode after dot*/
extern  int     appendeol();            /* go into insert mode at eol	*/
extern  int     insert();               /* go into insert mode		*/
extern  int     insertbol();            /* go into insert mode at bol	*/
extern  int     quote();                /* Insert literal               */
extern  int     backviword();           /* Backup by vi words           */
extern  int     forwviword();           /* Advance by vi words          */
extern  int     backword();             /* Backup by words              */
extern  int     forwword();             /* Advance by words             */
extern  int     forwendw();             /* Advance to end of words      */
extern  int     forwviendw();           /* Advance to end of vi words   */
extern  int     forwdelchar();              /* Forward delete               */
extern  int     backdelchar();              /* Backward delete              */
/* extern  int     killtext();             /* Kill forward                 */
extern  int     put();                 /* Yank back from killbuffer.   */
extern  int     putbefore();            /* put back from killbuffer.    */
extern  int     putafter();             /* Put back from killbuffer.    */
extern  int     lineputbefore();       /* put lines back from killbuffer.    */
extern  int     lineputafter();        /* Put lines back from killbuffer.    */
#ifdef BEFORE
extern  int     upperword();            /* Upper case word.             */
extern  int     lowerword();            /* Lower case word.             */
extern  int     capword();              /* Initial capitalize word.     */
#endif
/* extern  int     delfword();             /* Delete forward word.         */
/* extern  int     delbword();             /* Delete backward word.        */
extern  int     spawncli();             /* Run CLI in a subjob.         */
extern  int     spawn();                /* Run a command in a subjob.   */
#if	BSD
extern	int	bktoshell();		/* suspend emacs to parent shell*/
extern	int	rtfrmshell();		/* return from a suspended state*/
#endif
extern  int     quickexit();            /* low keystroke style exit.    */
extern	int	setmode();		/* set an editor mode		*/
extern	int	delmode();		/* delete a mode		*/
extern	int	gotoline();		/* go to a numbered line	*/
extern	int	gotocol();		/* go to a numbered column	*/
extern	int	namebuffer();		/* rename the current buffer	*/
#if	WORDPRO
extern	int	gotobop();		/* go to beginning/paragraph	*/
extern	int	gotoeop();		/* go to end/paragraph		*/
extern	int	gotobosec();		/* go to beginning/section	*/
extern	int	gotoeosec();		/* go to end/section		*/
extern	int	fillpara();		/* fill current paragraph	*/
#endif
extern	int	help();			/* get the help file here	*/
extern	int	deskey();		/* describe a key's binding	*/
extern	int	viewfile();		/* find a file in view mode	*/
extern	int	insfile();		/* insert a file		*/
extern	int	scrnextup();		/* scroll next window back	*/
extern	int	scrnextdw();		/* scroll next window down	*/
extern	int	bindkey();		/* bind a function to a key	*/
extern	int	unbindkey();		/* unbind a key's function	*/
extern	int	namedcmd();		/* execute named command	*/
extern	int	desbind();		/* describe bindings		*/
extern	int	nextbuffer();		/* switch to the next buffer	*/
#if BALPHA
extern	int	prevbuffer();		/* switch to the previous buffer*/
#endif
#if	WORDPRO
/* extern	int	killpara();		/* kill the current paragraph	*/
#endif
extern	int	setgmode();		/* set a global mode		*/
extern	int	delgmode();		/* delete a global mode		*/
extern	int	insspace();		/* insert a space forword	*/
extern	int	forwhunt();		/* hunt forward for next match	*/
extern	int	backhunt();		/* hunt backwards for next match*/
extern	int	consearch();		/* continue search for match	*/
extern	int	revsearch();		/* continue search for match*/
extern	int	pipecmd();		/* pipe command into buffer	*/
extern	int	filter();		/* filter buffer through dos	*/
extern	int	delwind();		/* delete the current window	*/
extern	int	cbuf1();		/* execute numbered comd buffer */
extern	int	cbuf2();
extern	int	cbuf3();
extern	int	cbuf4();
extern	int	cbuf5();
extern	int	cbuf6();
extern	int	cbuf7();
extern	int	cbuf8();
extern	int	cbuf9();
extern	int	cbuf10();
extern	int	cbuf11();
extern	int	cbuf12();
extern	int	cbuf13();
extern	int	cbuf14();
extern	int	cbuf15();
extern	int	cbuf16();
extern	int	cbuf17();
extern	int	cbuf18();
extern	int	cbuf19();
extern	int	cbuf20();
extern	int	cbuf21();
extern	int	cbuf22();
extern	int	cbuf23();
extern	int	cbuf24();
extern	int	cbuf25();
extern	int	cbuf26();
extern	int	cbuf27();
extern	int	cbuf28();
extern	int	cbuf29();
extern	int	cbuf30();
extern	int	cbuf31();
extern	int	cbuf32();
extern	int	cbuf33();
extern	int	cbuf34();
extern	int	cbuf35();
extern	int	cbuf36();
extern	int	cbuf37();
extern	int	cbuf38();
extern	int	cbuf39();
extern	int	cbuf40();
extern	int	storemac();		/* store text for macro		*/
extern	int	resize();		/* resize current window	*/
extern	int	clrmes();		/* clear the message line	*/
extern	int	meta();			/* meta prefix dummy function	*/
extern	int	cex();			/* ^X prefix dummy function	*/
extern	int	unarg();		/* ^U repeat arg dummy function	*/
extern	int	unmark();		/* unmark current buffer	*/
#if	ISRCH
extern	int	fisearch();		/* forward incremental search	*/
extern	int	risearch();		/* reverse incremental search	*/
#endif
#if	WORDPRO
extern	int	wordcount();		/* count words in region	*/
#endif
extern	int	upscreen();		/* force screen update		*/
extern	int	usekreg();		/* select named kill registers */
#if	FLABEL
extern	int	fnclabel();		/* set function key label	*/
#endif
#if	APROP
extern	int	apro();			/* apropos fuction		*/
#endif
#if	CRYPT
extern	int	setkey();		/* set encryption key		*/
#endif
extern	int	wrapword();		/* wordwrap function		*/
#if	CFENCE
extern	int	getfence();		/* move cursor to a matching fence */
#endif
#if	AEDIT
#if 0
extern  int     indent();               /* Insert CR-LF, then indent    */
#endif
extern  int     deblank();              /* Delete blank lines           */
extern	int	trim();			/* trim whitespace from end of line */
extern	int	detab();		/* detab rest of line */
extern	int	entab();		/* entab rest of line */
#endif
#if	PROC
extern	int	storeproc();		/* store names procedure */
extern	int	execproc();		/* execute procedure */
#endif
#if	NeWS
extern	int	setcursor() ;		/* mouse support function */
extern	int	newsadjustmode() ;	/* mouse support function */
#endif
#if ! SMALLER
extern	int	newsize();		/* change the current screen size */
extern	int	newwidth();		/* change the current screen width */
extern	int	setvar();		/* set a variables value */
extern	int	istring();		/* insert string in text	*/
extern	int	savewnd();		/* save current window		*/
extern	int	restwnd();		/* restore current window	*/
extern	int	writemsg();		/* write text on message line	*/
extern  int     twiddle();              /* Twiddle characters           */
extern	int	execcmd();		/* execute a command line	*/
extern	int	execbuf();		/* exec commands from a buffer	*/
extern	int	execfile();		/* exec commands from a file	*/
#endif

extern	int	nullproc();		/* does nothing... */


/*	Name to function binding table

		This table gives the names of all the bindable functions
	end their C function address. These are used for the bind-to-key
	function.

	REDO means the dotcmd command recorder whould not be halted
	UNDO means the undo stacks should be cleared
	MOTION means this command moves dot, and is compatible with
		the operator commands.
	FL only occurs with MOTION, means that if the motion is an operator
		argument, the operation should affect whole lines
	ABS only occurs with MOTION, means that if the motion is absolute,
		i.e. not relative to the current postion or screen.  It is used
		for the "lastdotmark", ldmark.
	GOAL signifies a motion that will attempt to retain the 
		current column postition.
*/

NBIND	names[] = {
	{"!",				spawn,		NONE },
	{"<",				pipecmd,	NONE },
	{"|",				filter,		REDO|UNDO},
	{"*",			togglelistbuffers,	NONE },
	{"abort-command",		esc,		NONE },
	{"add-mode",			setmode,	NONE },
	{"add-global-mode",		setgmode,	NONE },
	{"alternate-buffer",		altbuff,	NONE },
#if	APROP
	{"apropos",			apro,		NONE },
#endif
	{"append",			append,		REDO|UNDO },
	{"append-eol",			appendeol,	REDO|UNDO },
	{"b",				usebuffer,	NONE },
	{"backward-character",		backchar,	MOTION },
	{"backward-char-scan",		bcsrch,		MOTION },
	{"backward-char-scan-to",	bcsrch_to,	MOTION },
	{"begin-macro",			ctlxlp,		NONE },
	{"beginning-of-file",		gotobob,	ABS|MOTION },
	{"beginning-of-line",		gotobol,	MOTION },
	{"beginning-of-screen",		gotobos,	MOTION|FL },
	{"bind-key",			bindkey,	NONE },
	{"buffer-position",		showcpos,	NONE },
#ifdef BEFORE
	{"case-word-capitalize",	capword,	REDO|UNDO },
	{"case-word-lower",		lowerword,	REDO|UNDO },
	{"case-word-upper",		upperword,	REDO|UNDO },
#endif
	{"change-char",			chgchar,	REDO|UNDO },
	{"change-file-name",		filename,	NONE },
	{"change-line",			chgline,	REDO|UNDO },
	{"change-lines-til",		operlinechg,	REDO|UNDO },
#if ! SMALLER
	{"change-screen-size",		newsize,	NONE },
	{"change-screen-width",		newwidth,	NONE },
#endif
	{"change-til",			operchg,	REDO|UNDO },
	{"change-to-end-of-line",	chgtoeol,	REDO|UNDO },
	{"clear-and-redraw",		refresh,	NONE },
	{"clear-message-line",		clrmes,		NONE },
	{"continue-search",		consearch,	ABS|MOTION },
#if	WORDPRO
	{"count-words",			wordcount,	NONE },
#endif
	{"ctlx-prefix",			cex,		NONE },
	{"db",				killbuffer,	NONE },
	{"dw",				delwind,	NONE },
#if AEDIT
	{"delete-blank-lines",		deblank,	REDO|UNDO },
#endif
	{"delete-buffer",		killbuffer,	NONE },
	{"delete-global-mode",		delgmode,	NONE },
	{"delete-mode",			delmode,	NONE },
	{"delete-next-character",	forwdelchar,	REDO|UNDO },
	{"delete-lines-til",		operlinedel,	REDO|UNDO },
	{"delete-other-windows",	onlywind,	NONE },
	{"delete-previous-character",	backdelchar,	REDO|UNDO },
	{"delete-til",			operdel,	REDO|UNDO },
	{"delete-to-end-of-line",	deltoeol,	REDO|UNDO },
	{"delete-window",		delwind,	NONE },
	{"describe-bindings",		desbind,	NONE },
	{"describe-key",		deskey,		NONE },
#if	AEDIT
	{"detab-line",			detab,		REDO|UNDO },
#endif
	{"e",				filefind,	NONE },
	{"e!",				fileread,	NONE },
#if 0
	{"e#",				altbuff,	NONE },
#endif
	{"edit-file",			filefind,	NONE },
	{"end-macro",			ctlxrp,		NONE },
	{"end-of-file",			gotoeob,	ABS|MOTION },
	{"end-of-line",			gotoeol,	MOTION|GOAL },
	{"end-of-screen",		gotoeos,	MOTION|FL },
#if	AEDIT
	{"entab-line",			entab,		REDO|UNDO },
#endif
	{"exchange-point-and-mark",	swapmark,	ABS|MOTION },
#if ! SMALLER
	{"execute-buffer",		execbuf,	NONE },
	{"execute-command-line",	execcmd,	NONE },
	{"execute-file",		execfile,	NONE },
#endif
	{"execute-macro",		ctlxe,	REDO },
	{"execute-macro-1",		cbuf1,	REDO },
	{"execute-macro-2",		cbuf2,	REDO },
	{"execute-macro-3",		cbuf3,	REDO },
	{"execute-macro-4",		cbuf4,	REDO },
	{"execute-macro-5",		cbuf5,	REDO },
	{"execute-macro-6",		cbuf6,	REDO },
	{"execute-macro-7",		cbuf7,	REDO },
	{"execute-macro-8",		cbuf8,	REDO },
	{"execute-macro-9",		cbuf9,	REDO },
	{"execute-macro-10",		cbuf10,	REDO },
	{"execute-macro-11",		cbuf11,	REDO },
	{"execute-macro-12",		cbuf12,	REDO },
	{"execute-macro-13",		cbuf13,	REDO },
	{"execute-macro-14",		cbuf14,	REDO },
	{"execute-macro-15",		cbuf15,	REDO },
	{"execute-macro-16",		cbuf16,	REDO },
	{"execute-macro-17",		cbuf17,	REDO },
	{"execute-macro-18",		cbuf18,	REDO },
	{"execute-macro-19",		cbuf19,	REDO },
	{"execute-macro-20",		cbuf20,	REDO },
	{"execute-macro-21",		cbuf21,	REDO },
	{"execute-macro-22",		cbuf22,	REDO },
	{"execute-macro-23",		cbuf23,	REDO },
	{"execute-macro-24",		cbuf24,	REDO },
	{"execute-macro-25",		cbuf25,	REDO },
	{"execute-macro-26",		cbuf26,	REDO },
	{"execute-macro-27",		cbuf27,	REDO },
	{"execute-macro-28",		cbuf28,	REDO },
	{"execute-macro-29",		cbuf29,	REDO },
	{"execute-macro-30",		cbuf30,	REDO },
	{"execute-macro-31",		cbuf31,	REDO },
	{"execute-macro-32",		cbuf32,	REDO },
	{"execute-macro-33",		cbuf33,	REDO },
	{"execute-macro-34",		cbuf34,	REDO },
	{"execute-macro-35",		cbuf35,	REDO },
	{"execute-macro-36",		cbuf36,	REDO },
	{"execute-macro-37",		cbuf37,	REDO },
	{"execute-macro-38",		cbuf38,	REDO },
	{"execute-macro-39",		cbuf39,	REDO },
	{"execute-macro-40",		cbuf40,	REDO },
	{"execute-named-command",	namedcmd,	NONE },
#if	PROC
	{"execute-procedure",		execproc,	REDO },
#endif
	{"exit",			quit,		NONE },
	{"f",				filename,	NONE },
	{"file-name",			filename,	NONE },
#if	WORDPRO
	{"fill-paragraph",		fillpara,	REDO|UNDO },
#endif
	{"filter-buffer",		filter,		REDO|UNDO },
	{"find-file",			filefind,	NONE },
#if FINDERR
	{"find-next-error",		finderr,	NONE },
#endif
#if TAGS
	{"find-tag",			gototag,	NONE },
#endif
	{"first-nonwhite",		firstnonwhite,	MOTION },
	{"flip-til",			operflip,	REDO|UNDO },
	{"forward-character",		forwchar,	MOTION },
	{"forward-char-scan",		fcsrch,		MOTION },
	{"forward-char-scan-to",	fcsrch_to,	MOTION },
	{"gmodes",			showgmodes,	NONE },
	{"goto-column",			gotocol,	MOTION },
	{"goto-line",			gotoline,	ABS|MOTION|FL },
/* goline and goexact are special cases-- no ABS, even though they are */
	{"goto-named-mark",		golinenmmark,	MOTION|FL },
	{"goto-named-mark-exact",	goexactnmmark,	MOTION },
#if	CFENCE
	{"goto-matching-fence",		getfence,	ABS|MOTION },
#endif
	{"grow-window",			enlargewind,	NONE },
	{"h",				help,		NONE },
	{"handle-tab",			settab,		NONE },
	{"historical-buffer",		histbuff,	NONE },
	{"hunt-forward",		forwhunt,	ABS|MOTION },
	{"hunt-backward",		backhunt,	ABS|MOTION },
	{"help",			help,		NONE },
	{"i-shell",			spawncli,	NONE },
#if	ISRCH
	{"incremental-search",		fisearch,	NONE },
#endif
	{"insert",			insert,		REDO|UNDO },
	{"insert-bol",			insertbol,	REDO|UNDO },
	{"insert-file",			insfile,	REDO|UNDO },
	{"insert-space",		insspace,	REDO|UNDO },
#if ! SMALLER
	{"insert-string",		istring,	REDO|UNDO },
#endif
	{"join-lines",			join,		REDO|UNDO },
	{"kill-buffer",			killbuffer,	NONE },
#if	WORDPRO
	/* {"kill-paragraph",		killpara,	REDO|UNDO }, */
#endif
#if	FLABEL
	{"label-function-key",		fnclabel,	NONE },
#endif
	{"last-nonwhite",		lastnonwhite,	MOTION },
	{"list-buffers",		listbuffers,	NONE },
	{"lower-til",			operlower,	REDO|UNDO },
	{"meta-prefix",			meta,		NONE },
	{"middle-of-screen",		gotomos,	MOTION|FL },
	{"modes",			showmodes,	NONE },
	{"move-next-window-down",	mvdnnxtwind,	NONE },
	{"move-next-window-up",		mvupnxtwind,	NONE },
	{"move-window-down",		mvdnwind,	NONE },
	{"move-window-up",		mvupwind,	NONE },
	{"n",				nextbuffer,	NONE },
	{"name-buffer",			namebuffer,	NONE },
	{"newline",			newline,	REDO|UNDO },
	{"next-buffer",			nextbuffer,	NONE },
	{"next-half-page",		forwhpage,	NONE },
	{"next-line",			forwline,	GOAL|MOTION|FL },
	{"next-line-at-bol",		forwbline,	MOTION|FL },
	{"next-page",			forwpage,	MOTION },
#if	WORDPRO
	{"next-paragraph",		gotoeop,	ABS|MOTION },
#endif
	{"next-punc-word",		forwviword,	MOTION },
	{"next-punc-word-end",		forwviendw,	MOTION },
	{"next-section",		gotoeosec,	ABS|MOTION },
	{"next-window",			nextwind,	NONE },
	{"next-word",			forwword,	MOTION },
	{"next-word-end",		forwendw,	MOTION },
	{"nop",				nullproc,	NONE },
	{"open-line-below",		opendown,	REDO|UNDO },
	{"open-line-above",		openup,		REDO|UNDO },
	{"overwrite",			overwrite,	REDO|UNDO },
#if BALPHA
	{"p",				prevbuffer,	NONE },
	{"pb",				prevbuffer,	NONE },
#else
	{"p",				altbuff,	NONE },
	{"pb",				altbuff,	NONE },
#endif
	{"pw",				prevwind,	NONE },
	{"pipe-command",		pipecmd,	NONE },
	{"position-window",		poswind,	NONE },
	{"previous-half-page",		backhpage,	NONE },
	{"previous-line",		backline,	GOAL|MOTION|FL },
	{"previous-line-at-bol",	backbline,	MOTION|FL },
	{"previous-page",		backpage,	MOTION },
#if	WORDPRO
	{"previous-paragraph",		gotobop,	ABS|MOTION },
#endif
	{"previous-punc-word",		backviword,	MOTION },
	{"previous-section",		gotobosec,	ABS|MOTION },
	{"previous-window",		prevwind,	NONE },
	{"previous-word",		backword,	MOTION },
	{"put-after",			putafter,	REDO|UNDO },
	{"put-before",			putbefore,	REDO|UNDO },
	{"put-as-lines-after",		lineputafter,	REDO|UNDO },
	{"put-as-lines-before",		lineputbefore,	REDO|UNDO },
	{"q",				quit,		NONE },
	{"q!",				quithard,	NONE },
	{"query-replace-til",		operqreplace,	UNDO },
	{"query-replace-lines-til",	operlineqreplace,	UNDO },
	{"quick-exit",			quickexit,	NONE },
	{"quote-character",		quote,		REDO|UNDO },
	{"r",				insfile,	REDO|UNDO},
	{"replace-with-file",		fileread,	NONE },
	{"redraw-display",		reposition,	NONE },
	{"repeat-char-scan",		rep_csrch,	MOTION },
	{"repeat-last-cmd",		dotcmdplay,	NONE },
	{"replace-character",		replacechar,	REDO|UNDO },
	{"replace-silent-lines-til",	operlinesreplace,	REDO|UNDO },
	{"replace-silent-til",		opersreplace,	REDO|UNDO },
	{"resize-window",		resize,		NONE },
#if ! SMALLER
	{"restore-window",		restwnd,	NONE },
#endif
	{"reverse-char-scan",		rev_csrch,	MOTION },
#if	ISRCH
	{"reverse-incremental-search",	risearch,	NONE },
#endif
	{"reverse-search",		revsearch,	ABS|MOTION },
#if	PROC
	{"run",				execproc,	NONE },
#endif
	{"save-file",			filesave,	NONE },
#if ! SMALLER
	{"save-window",			savewnd,	NONE },
#endif
	{"scroll-next-up",		scrnextup,	NONE },
	{"scroll-next-down",		scrnextdw,	NONE },
	{"search-forward",		forwsearch,	ABS|MOTION },
	{"search-reverse",		backsearch,	ABS|MOTION },
	{"select-buffer",		usebuffer,	NONE },
	{"set",				setmode,	NONE },
	{"setall",			showmodes,	NONE },
	{"setg",			setgmode,	NONE },
	{"setgall",			showgmodes,	NONE },
	{"setgno",			delgmode,	NONE },
	{"setno",			delmode,	NONE },
#if ! SMALLER
	{"setv",			setvar,		NONE },
#endif
#if	CRYPT
	{"set-encryption-key",		setkey,		NONE },
#endif
	{"set-fill-column",		setfillcol,	NONE },
	{"set-named-mark",		setnmmark,	NONE },
	{"set-mark",			setmark,	NONE },
	{"sh",				spawncli,	NONE },
	{"shell-command",		spawn,		NONE },
	{"shift-left-til",		operlshift,	REDO|UNDO },
	{"shift-right-til",		operrshift,	REDO|UNDO },
	{"show-modes",			showmodes,	NONE },
	{"show-global-modes",		showgmodes,	NONE },
	{"shrink-window",		shrinkwind,	NONE },
	{"split-current-window",	splitwind,	NONE },
	{"store-macro",			storemac,	NONE },
#if	PROC
	{"store-procedure",		storeproc,	NONE },
#endif
#if	BSD
	{"suspend-emacs",		bktoshell,	NONE },
#endif
#if TAGS
	{"ta",				gototag,	NONE },
	{"tag",				gototag,	NONE },
#endif
	{"toggle-buffer-list",		togglelistbuffers,	NONE },
#if ! SMALLER
	{"transpose-characters",	twiddle,	REDO|UNDO },
#endif
#if	AEDIT
	{"trim-line",			trim,		REDO|UNDO },
#endif
	{"unbind-key",			unbindkey,	NONE },
	{"undo-change",			undo,		NONE },
	{"undo-line-changes",		lineundo,	NONE },
	{"universal-argument",		unarg,		NONE },
	{"unmark-buffer",		unmark,		NONE },
	{"unsetg",			delgmode,	NONE },
	{"unset",			delmode,	NONE },
	{"update-screen",		upscreen,	NONE },
	{"upper-til",			operupper,	REDO|UNDO },
	{"use-named-kill-register",	usekreg,	REDO },
	{"view-file",			viewfile,	NONE },
	{"w",				filesave,	NONE },
#if	WORDPRO
	{"wc",				wordcount,	NONE },
#endif
	{"wq",				writequit,	NONE },
	{"wrap-word",			wrapword,	REDO|UNDO },
	{"write-file",			filewrite,	NONE },
	{"write-file-and-quit",		writequit,	NONE },
#if ! SMALLER
	{"write-message",		writemsg,	NONE },
#endif
	{"write-til",			operwrite,	NONE },
	{"x",				quickexit,	NONE },
	{"yank-line",			yankline,	NONE },
	{"yank-lines-til",		operlineyank,	NONE },
	{"yank-til",			operyank,	NONE },

	{"",			NULL}
};
