/*
 *   This file was automatically generated by cextract version 1.2.
 *   Manual editing now recommended, since I've done a whole lot of it.
 *
 *   Created: Thu May 14 15:44:40 1992
 *
 * $Log: proto.h,v $
 * Revision 1.103  1994/03/24 12:10:28  pgf
 * new function, kcod2escape_seq()
 *
 * Revision 1.102  1994/03/10  20:11:11  pgf
 * kinsertlater() is new, anycb() now has arg
 *
 * Revision 1.101  1994/03/08  14:07:11  pgf
 * added missing...
 *
 * Revision 1.100  1994/03/08  12:22:12  pgf
 * changes for rectangles, and some movement from random.c to region.c
 *
 * Revision 1.99  1994/03/02  09:47:18  pgf
 * new routine fnc2str
 *
 * Revision 1.98  1994/02/28  15:08:43  pgf
 * added killregionmaybesave, which allows deleting without yanking
 *
 * Revision 1.97  1994/02/22  11:03:15  pgf
 * truncated RCS log for 4.0
 *
 *
 */

extern int main P(( int, char *[] ));
extern void loop P(( void ));
extern char * strmalloc P(( char * ));
extern int no_memory P(( char * ));
extern void global_val_init P(( void ));
extern void siginit P(( void ));
extern void siguninit P(( void ));
extern SIGT catchintr (DEFINE_SIG_ARGS);
extern int interrupted P(( void ));
extern void not_interrupted P(( void ));
extern void do_repeats P(( int *, int *, int * ));
extern int writeall P(( int, int ));
extern int zzquit P(( int, int ));
extern int quickexit P(( int, int ));
extern int quithard P(( int, int ));
extern int quit P(( int, int ));
extern int writequit P(( int, int ));
extern int esc P(( int, int ));
extern int rdonly P(( void ));
extern int unimpl P(( int, int ));
extern int opercopy P(( int, int ));
extern int opermove P(( int, int ));
extern int opertransf P(( int, int ));
extern int operglobals P(( int, int ));
extern int opervglobals P(( int, int ));
extern int source P(( int, int ));
extern int visual P(( int, int ));
extern int ex P(( int, int ));
extern int cntl_af P(( int, int ));
extern int cntl_xf P(( int, int ));
extern int unarg P(( int, int ));
extern int speckey P(( int, int ));
extern int altspeckey P(( int, int ));
extern int nullproc P(( int, int ));
extern void charinit P(( void ));
#if RAMSIZE
extern char *reallocate P(( char *, unsigned ));
extern char *allocate P(( unsigned ));
extern void release P(( char * ));
#endif
extern void start_debug_log P(( int , char ** ));
#if OPT_MAP_MEMORY
extern void exit_program P(( int ));
#endif

/* tcap.c and other screen-drivers */
#if TERMCAP
extern void tcapopen P(( void ));
extern void tcapclose P(( void ));
extern void tcapkopen P(( void ));
extern void tcapkclose P(( void ));
extern void tcaprev P(( int ));
extern int tcapcres P(( char * ));
extern void tcapmove P(( int, int ));
extern void tcapeeol P(( void ));
extern void tcapeeop P(( void ));
extern void tcapbeep P(( void ));
extern void putpad P(( char * ));
extern void putnpad P(( char *, int ));
#endif
#if OPT_XTERM
extern int xterm_button P(( int ));
#endif
extern void tcapscroll_reg P(( int, int, int ));
extern void tcapscroll_delins P(( int, int, int ));
extern void tcapscrollregion P(( int, int ));
extern void spal P(( char * ));

/* basic.c */
extern int gotobol P(( int, int ));
extern int backchar P(( int, int ));
extern int backchar_to_bol P(( int, int ));
extern int gotoeol P(( int, int ));
extern int forwchar P(( int, int ));
extern int forwchar_to_eol P(( int, int ));
extern int gotoline P(( int, int ));
extern int gotobob P(( int, int ));
extern int gotoeob P(( int, int ));
extern int gotobos P(( int, int ));
extern int gotomos P(( int, int ));
extern int gotoeos P(( int, int ));
extern int forwline P(( int, int ));
extern int firstnonwhite P(( int, int ));
#if !SMALLER
extern int lastnonwhite P(( int, int ));
#endif
extern int firstchar P(( LINE * ));
extern int nextchar P(( LINE *, int ));
extern int lastchar P(( LINE * ));
extern int forwbline P(( int, int ));
extern int backbline P(( int, int ));
extern int backline P(( int, int ));
extern int gotobop P(( int, int ));
extern int gotoeop P(( int, int ));
extern void skipblanksf P(( void ));
extern void skipblanksb P(( void ));
extern int gotobosec P(( int, int ));
extern int gotoeosec P(( int, int ));
extern int gotobosent P(( int, int ));
extern int gotoeosent P(( int, int ));
extern int getgoal P(( LINE * ));
extern int next_column P(( int, int ));
extern int forwpage P(( int, int ));
extern int backpage P(( int, int ));
extern int forwhpage P(( int, int ));
extern int backhpage P(( int, int ));
extern int setnmmark P(( int, int ));
extern int golinenmmark P(( int, int ));
extern int goexactnmmark P(( int, int ));
extern int gorectnmmark P(( int, int ));
extern int getnmmarkname P(( int * ));
extern int gonmmark P(( int ));
extern int setmark P(( void ));
extern int gomark P(( int, int ));
extern int godotplus P(( int, int ));
extern void swapmark P(( void ));
#if OPT_MOUSE
extern	int setwmark P(( int, int ));
extern	int setcursor P(( int, int ));
#endif

/* bind.c */
#if OPT_TERMCHRS
extern int set_termchrs P(( int, int ));
extern int show_termchrs P(( int, int ));
#endif
extern int help P(( int, int ));
extern int deskey P(( int, int ));
extern int bindkey P(( int, int ));
extern int desbind P(( int, int ));
extern void makebindlist P(( int, char *));
extern int strinc P(( char *, char *));
extern int rebind_key P(( int, CMDFUNC * ));
extern int install_bind P(( int, CMDFUNC *, CMDFUNC ** ));
extern int unbindkey P(( int, int ));
extern int unbindchar P(( int ));
extern int apro P(( int, int ));
extern int startup P(( char *));
extern char * flook P(( char *, int ));
extern char * kcod2str P(( int, char * ));
extern char * string2prc P(( char *, char * ));
extern char * kcod2prc P(( int, char * ));
#if X11
extern int insertion_cmd P(( int ));
#endif
extern int fnc2kcod P(( CMDFUNC * ));
extern char * fnc2str P(( CMDFUNC * ));
extern char * fnc2engl P(( CMDFUNC * ));
extern CMDFUNC * engl2fnc P(( char * ));
extern KBIND * kcode2kbind P(( int ));
extern CMDFUNC * kcod2fnc P(( int ));
extern int prc2kcod P(( char * ));
#if OPT_EVAL
extern char * prc2engl P(( char * ));
#endif
extern char * kbd_engl P(( char *, char * ));
extern void kbd_alarm P(( void ));
extern void kbd_putc P(( int ));
extern void kbd_puts P(( char * ));
extern void kbd_erase P(( void ));
extern void kbd_init P(( void ));
extern void kbd_unquery P(( void ));
extern int kbd_complete P(( int, char *, int *, char *, SIZE_T ));
extern int kbd_engl_stat P(( char *, char * ));

/* buffer.c */
extern WINDOW *bp2any_wp P(( BUFFER * ));
extern int histbuff P(( int, int ));
extern void imply_alt P(( char *, int, int ));
extern BUFFER * find_alt P(( void ));
extern int altbuff P(( int, int ));
extern int usebuffer P(( int, int ));
extern int firstbuffer P(( int, int ));
extern int nextbuffer P(( int, int ));
extern void make_current P(( BUFFER * ));
extern int swbuffer P(( BUFFER * ));
extern void undispbuff P(( BUFFER *, WINDOW * ));
extern int cmode_active P(( BUFFER * ));
extern int tabstop_val P(( BUFFER * ));
extern int shiftwid_val P(( BUFFER * ));
extern int has_C_suffix P(( BUFFER * ));
extern int killbuffer P(( int, int ));
extern int zotbuf P(( BUFFER * ));
extern int namebuffer P(( int, int ));
extern int popupbuff P(( BUFFER * ));
extern void sortlistbuffers P(( void ));
extern int togglelistbuffers P(( int, int ));
extern int listbuffers P(( int, int ));
#if OPT_UPBUFF
void updatelistbuffers P((void));
void update_scratch P(( char *, int (*func)(BUFFER *) ));
#else
#define updatelistbuffers()
#define update_scratch(name, func)
#endif
extern int addline P(( BUFFER *, char *, int ));
extern int add_line_at P(( BUFFER *, LINEPTR, char *, int ));
extern int anycb P(( BUFFER ** ));
extern void set_bname P(( BUFFER *, char * ));
extern char * get_bname P(( BUFFER * ));
extern BUFFER * find_b_name P(( char * ));
extern BUFFER * bfind P(( char *, int ));
extern int bclear P(( BUFFER * ));
extern int bsizes P(( BUFFER * ));
extern void chg_buff P(( BUFFER *, int ));
extern void unchg_buff P(( BUFFER *, int ));
extern int unmark P(( int, int ));

/* crypt.c */
#if	CRYPT
extern	int	ue_makekey P((char *, int));
extern	int	ue_setkey P((int, int));
extern	void	ue_crypt P((char *, int));
#endif	/* CRYPT */

/* csrch.c */
extern int fscan P(( int, int, int ));
extern int bscan P(( int, int, int ));
extern int fcsrch P(( int, int ));
extern int bcsrch P(( int, int ));
extern int fcsrch_to P(( int, int ));
extern int bcsrch_to P(( int, int ));
extern int rep_csrch P(( int, int ));
extern int rev_csrch P(( int, int ));

/* display.c */
extern int nu_width P(( WINDOW * ));
extern int col_limit P(( WINDOW * ));
extern void vtinit P(( void ));
extern void vttidy P(( int ));
#if !SMALLER
extern int upscreen P(( int, int ));
#endif
extern void reframe P(( WINDOW * ));
extern int update P(( int ));
extern void upmode P(( void ));
extern int offs2col P(( WINDOW *, LINEPTR, C_NUM ));
#if OPT_MOUSE
extern int col2offs P(( WINDOW *, LINEPTR, C_NUM ));
#endif
#ifdef WMDLINEWRAP
extern int line_height P(( WINDOW *, LINEPTR ));
#else
#define line_height(wp,lp) 1
#endif
#if defined(WMDLINEWRAP) || OPT_MOUSE
extern WINDOW *row2window P(( int ));
#endif
extern void hilite P((int, int, int, int));
extern void scrscroll P(( int, int, int ));
extern void movecursor P(( int, int ));
extern void mlerase P(( void ));
extern void mlsavec P(( int ));
extern void mlwrite P((char *, ... ));
extern void mlforce P((char *, ... ));
extern void mlprompt P((char *, ... ));
extern void mlerror P(( char * ));
extern void dbgwrite P((char *, ... ));
extern char * lsprintf P((char *, char *, ... ));
#ifdef	UNUSED
extern void lssetbuf P(( char * ));
extern char * _lsprintf P(( char *, ... ));
#endif	/* UNUSED */
extern void bputc P(( int ));
extern void bprintf P((char *, ... ));
#if defined(SIGWINCH) && !X11
extern void getscreensize P(( int *, int * ));
extern SIGT sizesignal (DEFINE_SIG_ARGS);
#endif
extern void newscreensize P(( int, int ));
#if OPT_WORKING
extern SIGT imworking (DEFINE_SIG_ARGS);
#endif

/* eval.c */
extern char * l_itoa P(( int ));
extern int absol P(( int ));
extern int is_truem P(( char * ));
extern int is_falsem P(( char * ));
#if OPT_EVAL || X11
extern int stol P(( char * ));
#endif
#if OPT_EVAL
extern char * gtenv P(( char * ));
extern char * gtusr P(( char * ));
#endif
#if OPT_EVAL || !SMALLER
extern char * mkupper P(( char * ));
#endif
extern char * mklower P(( char * ));
#if OPT_EVAL
extern int listvars P(( int, int ));
extern int setvar P(( int, int ));
extern int set_variable P(( char * ));
extern int sindex P(( char *, char * ));
extern char * ltos P(( int ));
extern int ernd P(( void ));
#endif

/* exec.c */
extern int end_named_cmd P(( void ));
extern int more_named_cmd P(( void ));
extern int namedcmd P(( int, int ));
extern int docmd P(( char *, int, int, int ));
extern int dobuf P(( BUFFER * ));
extern int dofile P(( char * ));
#if !SMALLER
extern int execbuf P(( int, int ));
extern int execfile P(( int, int ));
#endif
extern int execute P(( CMDFUNC *, int, int ));
extern int storemac P(( int, int ));
#if PROC
extern int storeproc P(( int, int ));
extern int execproc P(( int, int ));
#endif
extern int cbuf1 P(( int, int ));
extern int cbuf2 P(( int, int ));
extern int cbuf3 P(( int, int ));
extern int cbuf4 P(( int, int ));
extern int cbuf5 P(( int, int ));
extern int cbuf6 P(( int, int ));
extern int cbuf7 P(( int, int ));
extern int cbuf8 P(( int, int ));
extern int cbuf9 P(( int, int ));
extern int cbuf10 P(( int, int ));
#if !SMALLER
extern int cbuf11 P(( int, int ));
extern int cbuf12 P(( int, int ));
extern int cbuf13 P(( int, int ));
extern int cbuf14 P(( int, int ));
extern int cbuf15 P(( int, int ));
extern int cbuf16 P(( int, int ));
extern int cbuf17 P(( int, int ));
extern int cbuf18 P(( int, int ));
extern int cbuf19 P(( int, int ));
extern int cbuf20 P(( int, int ));
extern int cbuf21 P(( int, int ));
extern int cbuf22 P(( int, int ));
extern int cbuf23 P(( int, int ));
extern int cbuf24 P(( int, int ));
extern int cbuf25 P(( int, int ));
extern int cbuf26 P(( int, int ));
extern int cbuf27 P(( int, int ));
extern int cbuf28 P(( int, int ));
extern int cbuf29 P(( int, int ));
extern int cbuf30 P(( int, int ));
extern int cbuf31 P(( int, int ));
extern int cbuf32 P(( int, int ));
extern int cbuf33 P(( int, int ));
extern int cbuf34 P(( int, int ));
extern int cbuf35 P(( int, int ));
extern int cbuf36 P(( int, int ));
extern int cbuf37 P(( int, int ));
extern int cbuf38 P(( int, int ));
extern int cbuf39 P(( int, int ));
extern int cbuf40 P(( int, int ));
#endif /* !SMALLER */

/* file.c */
#ifdef MDCHK_MODTIME
extern int ask_shouldchange P(( BUFFER * ));
extern int get_modtime P(( BUFFER *, long * ));
extern void set_modtime P(( BUFFER *, char * ));
extern int check_modtime P(( BUFFER *, char * ));
extern int inquire_modtime P(( BUFFER *, char * ));
extern int check_visible_modtimes P(( void ));
#endif
extern int no_such_file P(( char * ));
extern int same_fname P(( char *, BUFFER *, int ));
extern int fileread P(( int, int ));
extern int filefind P(( int, int ));
extern int viewfile P(( int, int ));
extern int insfile P(( int, int ));
extern int getfile P(( char *, int ));
extern int readin P((char *, int, BUFFER *, int ));
#if !MSDOS && !OPT_MAP_MEMORY
extern int quickreadf P(( BUFFER *, int * ));
#endif
extern int slowreadf P(( BUFFER *, int * ));
extern int set_dosmode P(( int, int ));
extern void makename P(( char *, char * ));
extern int unqname P((char *, int));
extern int filewrite P(( int, int ));
extern int filesave P(( int, int ));
extern int writeout P(( char *, BUFFER *, int ));
extern int writeregion P(( void ));
extern int kwrite P(( char *, int ));
extern int filename P(( int, int ));
extern int ifile P(( char *, int, FILE * ));
extern int kifile P(( char * ));
extern SIGT imdying (DEFINE_SIG_ARGS);
extern void markWFMODE P(( BUFFER * ));
#if CRYPT
extern int resetkey P(( BUFFER *, char * ));
#endif

/* filec.c */
#if COMPLETE_DIRS || COMPLETE_FILES
extern BUFFER *bs_init P(( char *, int ));
extern int bs_find P(( char *, SIZE_T, BUFFER *, int, LINEPTR * ));
#endif
extern int mlreply_file P(( char *, TBUFF **, int, char * ));
extern int mlreply_dir P(( char *, TBUFF **, char * ));
extern char *filec_expand P(( void ));

/* fileio.c */
extern int ffropen P(( char * ));
extern int ffwopen P(( char * ));
extern int ffronly P(( char * ));
extern long ffsize P(( void ));
extern int ffexists P(( char * ));
#if !MSDOS && !OPT_MAP_MEMORY
extern int ffread P(( char *, long ));
extern void ffseek P(( long ));
extern void ffrewind P(( void ));
#endif
extern int ffclose P(( void ));
extern int ffputline P(( char [], int, char * ));
extern int ffputc P(( int ));
extern int ffhasdata P(( void ));

/* finderr.c */
#if FINDERR
extern void set_febuff P(( char * ));
extern int finderr P(( int, int ));
extern LINE * getdot P(( BUFFER * ));
extern void putdotback P(( BUFFER *, LINE * ));
extern int finderrbuf P(( int, int ));
#endif

/* glob.c */
#if !UNIX
extern	int	glob_needed P((char **));
#endif
extern	char **	glob_expand P((char **));
extern	char **	glob_string P((char *));
extern	int	glob_length P((char **));
extern	char **	glob_free   P((char **));

#if !UNIX
extern	void	expand_wild_args P(( int * , char ***));
#endif

extern	int	doglob P(( char * ));

/* globals.c */
extern int globals P(( int, int ));
extern int vglobals P(( int, int ));

/* history.c */
#if OPT_HISTORY
extern void hst_init P(( int ));
extern void hst_glue P(( int ));
extern void hst_append P(( char *, int ));
extern void hst_remove P(( char * ));
extern void hst_flush P(( void ));
extern int showhistory P(( int, int ));
extern int edithistory P(( char *, int *, int *, int, int (*func)( char *, int, int, int ), int ));
#else
#define hst_init(c)
#define hst_glue(c)
#define hst_append(p,c)
#define hst_remove(p)
#define hst_flush()
#endif

/* input.c */
extern int no_completion P(( int, char *, int * ));
extern int mlyesno P(( char * ));
extern int mlquickask P(( char *, char *, int * ));
extern int mlreply P(( char *, char *, int ));
extern int mlreply_reg P(( char *, char *, int *, int ));
extern int mlreply_reg_count P(( int, int *, int * ));
extern int mlreply_no_bs P(( char *, char *, int ));
extern int mlreply_no_opts P(( char *, char *, int ));
extern void incr_dot_kregnum P(( void ));
extern void tungetc P(( int ));
extern int get_recorded_char P(( int ));
extern int tgetc P(( int ));
extern int kbd_key P(( void ));
extern int kbd_seq P(( void ));
extern int kbd_escape_seq P(( void ));
extern int kcod2escape_seq P(( int, char * ));
extern int screen_string P(( char *, int, CMASK ));
extern int end_string P(( void ));
extern int kbd_delimiter P(( void ));
extern int is_edit_char P(( int ));
extern void kbd_kill_response P(( char *, int *, int ));
extern int kbd_show_response P(( char *, char *, int, int, int ));
extern void kbd_pushback P(( char *, int ));
extern int kbd_string P(( char *, char *, int, int, int, int (*func)(int,char*,int*) ));
extern int kbd_reply P(( char *, char *, int, int (*efunc)(char *,int,int,int), int, int, int (*cfunc)(int,char*,int*) ));
extern int dotcmdbegin P(( void ));
extern int dotcmdfinish P(( void ));
extern void dotcmdstop P(( void ));
extern int dotcmdplay P(( int, int ));
extern int kbd_replaying P(( int ));
extern int kbd_mac_begin P(( int, int ));
extern int kbd_mac_end P(( int, int ));
extern int kbd_mac_exec P(( int, int ));
extern int kbd_mac_save P(( int, int ));
extern int kbm_started P(( int, int ));
extern int start_kbm P(( int, int, TBUFF * ));
extern void finish_kbm P(( void ));

/* insert.c */
extern int quote P(( int, int ));
extern int replacechar P(( int, int ));
extern int tab P(( int, int ));
extern int shiftwidth P(( int, int ));
extern int openup P(( int, int ));
extern int openup_no_aindent P(( int, int ));
extern int opendown P(( int, int ));
extern int opendown_no_aindent P(( int, int ));
extern int openlines P(( int ));
extern int insert P(( int, int ));
extern int insert_no_aindent P(( int, int ));
extern int insertbol P(( int, int ));
extern int append P(( int, int ));
extern int appendeol P(( int, int ));
extern int overwritechars P(( int, int ));
extern int ins P(( void ));
#if !SMALLER
extern int insstring P(( int, int ));
extern int overwstring P(( int, int ));
extern int istring P(( int, int, int ));
#endif
extern int inschar P(( int, int * ));
extern int backspace P(( void ));
extern int newline P(( int, int ));
extern int indented_newline P(( void ));
extern int indented_newline_above P(( void ));
extern int previndent P(( int * ));
extern int nextindent P(( int * ));
extern int doindent P(( int ));
extern int indentlen P(( LINE * ));
#if OPT_EVAL
extern char *current_modename P(( void ));
#endif

/* isearch.c */
#if ISRCH
extern int risearch P(( int, int ));
extern int fisearch P(( int, int ));
extern int isearch P(( int, int ));
extern int get_char P(( void ));
#endif

/* line.c */
extern int do_report P(( L_NUM ));
extern LINEPTR lalloc P(( int, BUFFER * ));
extern void lfree P(( LINEPTR, BUFFER * ));
#if !OPT_MAP_MEMORY
extern void ltextfree P(( LINE *, BUFFER * ));
#endif
extern void lremove P(( BUFFER *, LINEPTR ));
extern int insspace P(( int, int ));
extern int lstrinsert P(( char *, int ));
extern int linsert P(( int, int ));
extern int lnewline P(( void ));
extern int ldelete P(( long, int ));
#if OPT_EVAL
extern char * getctext P(( CMASK ));
extern int putctext P(( char * ));
#endif
extern int ldelnewline P(( void ));
extern void ksetup P(( void ));
extern void kdone P(( void ));
extern int kinsertlater P(( int ));
extern int kinsert P(( int ));
extern int index2reg P(( int ));
extern int reg2index P(( int ));
extern int index2ukb P(( int ));
extern int usekreg P(( int, int ));
extern void kregcirculate P(( int ));
extern int putbefore P(( int, int ));
extern int putafter P(( int, int ));
extern int lineputbefore P(( int, int ));
extern int lineputafter P(( int, int ));
extern int rectputbefore P(( int, int ));
extern int rectputafter P(( int, int ));
extern int doput P(( int, int, int, int ));
extern int force_text_to_col P(( int ));
extern int put P(( int, int ));
extern int execkreg P(( int, int ));
extern int loadkreg P(( int, int ));
#if OPT_SHOW_REGS
extern int showkreg P(( int, int ));
#endif
#if OPT_SHOW_REGS && OPT_UPBUFF
extern void relist_registers P(( void ));
#else
#define relist_registers()
#endif

/* map.c */
extern int map P(( int, int ));
extern int unmap P(( int, int ));
extern void map_check P(( int ));
extern int map_proc P((int, int));
#if OPT_UPBUFF
extern void relist_mappings P(( void ));
#else
#define relist_mappings()
#endif

/* mapchars.c */
#if NEW_VI_MAP
extern void map_init P(( void ));
extern int map_set P(( int, int ));
extern int map_unset P(( int, int ));
extern int map_read P(( void ));
#endif

/* modes.c */
extern int string_to_number P(( char *, int * ));
extern int setlocmode P(( int, int ));
extern int dellocmode P(( int, int ));
extern int setglobmode P(( int, int ));
extern int delglobmode P(( int, int ));
extern int settab P(( int, int ));
extern int adjvalueset P(( char *, int, int, VALARGS * ));
extern int setfillcol P(( int, int ));
extern char *string_mode_val P(( VALARGS * ));
extern REGEXVAL *new_regexval P(( char *, int ));
extern void free_regexval P(( REGEXVAL * ));
extern void free_val P(( struct VALNAMES *, struct VAL * ));
extern int copy_val P(( struct VAL *, struct VAL * ));
extern void copy_mvals P(( int, struct VAL *, struct VAL * ));
#if OPT_UPBUFF
extern void save_vals P(( int, struct VAL *, struct VAL *, struct VAL * ));
extern void relist_settings P(( void ));
#endif
extern void free_local_vals P(( struct VALNAMES *, struct VAL *, struct VAL * ));
extern int listmodes P(( int, int ));
extern int find_mode P(( char *, int, VALARGS * ));

/* npopen.c */
#if UNIX || MSDOS
extern FILE * npopen P(( char *, char * ));
extern void npclose P(( FILE * ));
extern int inout_popen P(( FILE **, FILE **, char * ));
extern int softfork P(( void ));
#endif
#if UNIX
extern void exec_sh_c P(( char * ));
extern int system_SHELL P(( char * ));
#endif
#if MSDOS
extern void npflush P(( void ));
#endif

/* oneliner.c */
extern int pregion P(( int ));
extern int llineregion P(( void ));
extern int plineregion P(( void ));
extern int substregion P(( void ));
extern int subst_again_region P(( void ));
extern int subst_again P(( int, int ));

/* opers.c */
extern int operator P(( int, int, int (*)(void), char * ));
extern int operdel P(( int, int ));
extern int operlinedel P(( int, int ));
extern int operchg P(( int, int ));
extern int operlinechg P(( int, int ));
extern int operjoin P(( int, int ));
extern int operyank P(( int, int ));
extern int operlineyank P(( int, int ));
extern int operflip P(( int, int ));
extern int operupper P(( int, int ));
extern int operlower P(( int, int ));
extern int operlshift P(( int, int ));
extern int operrshift P(( int, int ));
extern int operwrite P(( int, int ));
extern int operformat P(( int, int ));
extern int operfilter P(( int, int ));
extern int operprint P(( int, int ));
extern int operlist P(( int, int ));
extern int opersubst P(( int, int ));
extern int opersubstagain P(( int, int ));
extern int operentab P(( int, int ));
extern int operdetab P(( int, int ));
extern int opertrim P(( int, int ));
extern int operblank P(( int, int ));
extern int operopenrect P(( int, int ));

/* path.c */
#if MSDOS
extern char * is_msdos_drive P(( char * ));
#endif
#if VMS
extern int is_vms_pathname P(( char *, int ));
extern char * vms_pathleaf P(( char * ));
#endif
#if UNIX
extern char * home_path P(( char * ));
#endif
extern char * pathleaf P(( char * ));
extern char * pathcat P(( char *, char *, char * ));
extern char * last_slash P(( char * ));
extern char * canonpath P(( char * ));
extern char * shorten_path P(( char *, int ));
extern char * lengthen_path P(( char * ));
extern int is_pathname P(( char * ));
extern int maybe_pathname P(( char * ));
extern char * is_appendname P(( char * ));
extern char * non_filename P(( void ));
extern int is_internalname P(( char * ));
extern int is_directory P(( char * ));
#if (UNIX||MSDOS) && PATHLOOK
extern char *parse_pathlist P(( char *, char * ));
#endif

/* random.c */
extern void line_report P(( L_NUM ));
extern L_NUM line_count P(( BUFFER * ));
extern L_NUM line_no P(( BUFFER *, LINEPTR ));
#if OPT_EVAL
extern L_NUM getcline P(( void ));
#endif
extern void set_rdonly P(( BUFFER *, char * ));
extern int liststuff P(( char *, void (*)(int, char *), int, char * ));
extern int showcpos P(( int, int ));
extern int showlength P(( int, int ));
extern int getccol P(( int ));
extern int getcol P(( LINEPTR, C_NUM, int ));
extern int gotocol P(( int, int ));
extern int getoff P(( C_NUM, C_NUM * ));
extern int gocol P(( int ));
#if !SMALLER
extern int twiddle P(( int, int ));
#endif
extern int deblank P(( int, int ));
extern int flipchar P(( int, int ));
extern int forwdelchar P(( int, int ));
extern int backdelchar P(( int, int ));
extern int deltoeol P(( int, int ));
extern int chgtoeol P(( int, int ));
extern int yankline P(( int, int ));
extern int chgline P(( int, int ));
extern int chgchar P(( int, int ));
extern int matchfence P(( int, int ));
extern int matchfenceback P(( int, int ));
extern int clrmes P(( int, int ));
extern int fmatchindent P(( int ));
#if !SMALLER
extern int writemsg P(( int, int ));
#endif
extern void catnap P(( int, int ));
extern char * current_directory P(( int ));
extern int cd P(( int, int ));
extern int pwd P(( int, int ));
extern int set_directory P(( char * ));
extern void ch_fname P(( BUFFER *, char * ));

/* rectangl.c */
extern int yankrect P(( void ));
extern int stropenrect P(( void ));
extern int deleterect P(( void ));
extern int killrect P(( void ));
extern int openrect P(( int, int ));
extern int copyrect P(( int, int ));
extern int clearrect P(( void ));
extern void CleanupRect P(( void ));

/* regexp.c */
extern void regmassage P(( char *, char *, int ));
extern regexp * regcomp P(( char *, int ));
extern char * reg P(( int, int * ));
extern char * regbranch P(( int * ));
extern char * regpiece P(( int *, int ));
extern char * regatom P(( int *, int ));
extern char * regnode P(( int ));
extern void regc P(( int ));
extern void regninsert P(( int, char * ));
extern void regopinsert P(( int, char * ));
extern void regtail P(( char *, char * ));
extern void regoptail P(( char *, char * ));
extern int regstrncmp P(( char *, char *, int, char * ));
extern char * regstrchr P(( char *, int, char * ));
extern int regexec P(( regexp *, char *, char *, int, int ));
extern int regtry P(( regexp *, char *, char * ));
extern int regmatch P(( char * ));
extern int regrepeat P(( char * ));
extern char * regnext P(( char * ));
extern int lregexec P(( regexp *, LINE *, int, int ));

/* region.c */
extern int killregion P(( void ));
extern int killregionmaybesave P(( int ));
extern int kill_line P((void *, int, int ));
extern int killrectmaybesave P(( int ));
extern int open_hole_in_line P((void *, int, int ));
extern int openregion P(( void ));
extern int stringrect P(( void ));
extern int shift_right_line P(( void *, int, int ));
extern int shiftrregion P(( void ));
extern int shift_left_line P(( void *, int, int ));
extern int shiftlregion P(( void ));
extern int detabline P(( void *, int, int ));
extern int detab_region P(( void ));
extern int entabline P(( void *, int, int ));
extern int entab_region P(( void ));
extern int trimline P(( void *, int, int ));
extern int trim_region P(( void ));
extern int blankline P(( void *, int, int ));
extern int blank_region P(( void ));
extern int yank_line P((void *, int, int ));
extern int yankregion P(( void ));
extern int _yankchar P(( int ));
extern int _to_lower P(( int ));
extern int _to_upper P(( int ));
extern int _to_caseflip P(( int ));
extern int flipregion P(( void ));
extern int lowerregion P(( void ));
extern int upperregion P(( void ));
extern int charprocreg P(( int (*)(int) ));
extern int getregion P(( REGION * ));
extern int get_fl_region P(( REGION * ));
extern int do_lines_in_region P(( int (*)(void *,int,int), void *, int ));
extern int do_chars_in_line P(( void *, int, int ));

/* search.c */
extern void not_found_msg P(( int, int ));
extern int scrforwsearch P(( int, int ));
extern int scrbacksearch P(( int, int ));
extern int forwsearch P(( int, int ));
extern int forwhunt P(( int, int ));
extern int backhunt P(( int, int ));
extern int fsearch P(( int, int, int, int ));
extern int backsearch P(( int, int ));
extern int rsearch P(( int, int, int, int ));
extern int consearch P(( int, int ));
extern int revsearch P(( int, int ));
extern void regerror P(( char * ));
#if OPT_EVAL || UNUSED
extern int eq P(( int, int ));
#endif
extern int scrsearchpat P(( int, int ));
extern int readpattern P(( char *, char *, regexp **, int, int ));
extern void savematch P(( MARK, SIZE_T ));
extern void scanboundry P(( int, MARK, int ));
extern void nextch P(( MARK *, int ));
extern int findpat P(( int, int, regexp *, int ));

/* spawn.c */
extern int spawncli P(( int, int ));
extern SIGT rtfrmshell (DEFINE_SIG_ARGS);
extern int bktoshell P(( int, int ));
extern void pressreturn P(( void ));
extern int respawn P(( int, int ));
extern int spawn P(( int, int ));
extern int spawn1 P(( int ));
extern int pipecmd P(( int, int ));
extern int filterregion P(( void ));
extern int filter P(( int, int ));

/* tags.c */
#if TAGS
extern int gototag P(( int, int ));
extern int cmdlinetag P(( char * ));
extern int tags P(( char *, int ));
extern int untagpop P(( int, int ));
#if OPT_SHOW_TAGS
extern int showtagstack P(( int, int ));
#endif
#endif /* TAGS */

/* termio.c */
extern void ttopen P(( void ));
extern void ttclose P(( void ));
extern void ttclean P(( int ));
extern void ttunclean P(( void ));
extern void ttputc P(( int ));
extern void ttflush P(( void ));
extern int ttgetc P(( void ));
extern int typahead P(( void ));
extern void ttmiscinit P(( void ));

/* undo.c */
extern void toss_to_undo P(( LINEPTR ));
extern void copy_for_undo P(( LINEPTR ));
extern void tag_for_undo P(( LINEPTR ));
extern void nounmodifiable P(( BUFFER * ));
extern void freeundostacks P(( BUFFER *, int ));
extern void mayneedundo P(( void ));
extern int undo P(( int, int ));
extern int backundo P(( int, int ));
extern int forwredo P(( int, int ));
extern int lineundo P(( int, int ));
extern void dumpuline P(( LINEPTR ));

/* version.c */
extern void print_usage P(( void ));
#if UNIX
extern void makeversion P(( void ));
#endif
extern int showversion P(( int, int ));

/* window.c */
extern int set_curwp P(( WINDOW * ));
#if OPT_EVAL
extern int getwpos P(( void ));
#endif
extern int reposition P(( int, int ));
extern int refresh P(( int, int ));
extern int poswind P(( int, int ));
extern int prevwind P(( int, int ));
extern int nextwind P(( int, int ));
extern int mvdnwind P(( int, int ));
extern int mvupwind P(( int, int ));
extern int mvdnnxtwind P(( int, int ));
extern int mvupnxtwind P(( int, int ));
extern int mvrightwind P(( int, int ));
extern int mvleftwind P(( int, int ));
extern int newwidth P(( int, int ));
extern int newlength P(( int, int ));
extern int onlywind P(( int, int ));
extern int delwind P(( int, int ));
extern int delwp P(( WINDOW * ));
extern void copy_traits P(( W_TRAITS *, W_TRAITS * ));
extern WINDOW * splitw P(( int, int ));
extern int splitwind P(( int, int ));
extern int enlargewind P(( int, int ));
extern int shrinkwind P(( int, int ));
extern WINDOW * wpopup P(( void ));
extern int scrnextup P(( int, int ));
extern int scrnextdw P(( int, int ));
#if !SMALLER
extern int savewnd P(( int, int ));
extern int restwnd P(( int, int ));
#endif
#if !SMALLER || OPT_EVAL
extern int resize P(( int, int ));
#endif
extern void winit P(( void ));

/* word.c */
extern int wrapword P(( int, int ));
extern int forwviword P(( int, int ));
extern int forwword P(( int, int ));
extern int forwviendw P(( int, int ));
extern int forwendw P(( int, int ));
extern int backviword P(( int, int ));
extern int backword P(( int, int ));
extern int joinregion P(( void ));
extern int joinlines P(( int, int ));
extern int formatregion P(( void ));
#if	WORDCOUNT
extern int wordcount P(( int, int ));
#endif

extern void setchartype P(( void ));
extern int getchartype P(( void ));
extern int isnewwordf P(( void ));
extern int isnewwordb P(( void ));
extern int isnewviwordf P(( void ));
extern int isnewviwordb P(( void ));
extern int isendwordf P(( void ));
extern int isendviwordf P(( void ));
extern int toktyp P(( char * ));
extern char * tokval P(( char * ));
extern char * token P(( char *, char *, int ));
extern int ffgetline P(( int * ));
extern int macroize P(( TBUFF **, char *, char * ));
extern int macarg P(( char * ));
extern int echochar P(( int, int ));
extern int scanmore P(( char *, int ));
extern int scanner P((regexp *, int, int ));
extern int insbrace P(( int, int ));
extern int inspound P(( void ));
extern int fmatch P(( int ));
extern int getfence P(( int, int ));
extern int comment_fence P(( int ));
extern int simple_fence P(( int, int, int ));

/* tbuff.c */
TBUFF *	tb_alloc P(( TBUFF **, ALLOC_T ));
TBUFF *	tb_init P(( TBUFF **, int ));
void	tb_free P(( TBUFF ** ));
TBUFF *	tb_put P(( TBUFF **, ALLOC_T, int ));
void	tb_stuff P(( TBUFF *, int ));
int	tb_get P(( TBUFF *, ALLOC_T ));
void	tb_unput P(( TBUFF * ));
TBUFF *	tb_append P(( TBUFF **, int ));
TBUFF *	tb_copy P(( TBUFF **, TBUFF * ));
TBUFF *	tb_bappend P(( TBUFF **, char *, ALLOC_T ));
TBUFF *	tb_sappend P(( TBUFF **, char * ));
TBUFF *	tb_scopy P(( TBUFF **, char * ));
void	tb_first P(( TBUFF * ));
int	tb_more P(( TBUFF * ));
int	tb_next P(( TBUFF * ));
int	tb_peek P(( TBUFF * ));
char *	tb_values P(( TBUFF * ));
ALLOC_T	tb_length P(( TBUFF * ));

/* tmp.c */
#if OPT_MAP_MEMORY
extern	void	tmp_cleanup	P(( void ));
extern	LINEPTR	l_allocate	P(( SIZE_T ));
extern	void	l_deallocate	P(( LINEPTR ));
extern	LINE *	l_reallocate	P(( LINEPTR, SIZE_T, BUFFER * ));
extern	int	l_truncated	P(( void ));
extern	void	l_region	P(( REGION * ));
extern	LINE *	l_ref		P(( LINEPTR ));
extern	LINEPTR	l_ptr		P(( LINE * ));
extern	void	set_lforw	P(( LINE *, LINE * ));
extern	void	set_lback	P(( LINE *, LINE * ));
extern	LINE *	lforw		P(( LINE * ));
extern	LINE *	lback		P(( LINE * ));
extern	void	lsetclear	P(( LINE * ));
extern	LINE *	lforw_p2r	P(( LINEPTR ));
extern	LINE *	lback_p2r	P(( LINEPTR ));
extern	LINEPTR	lforw_p2p	P(( LINEPTR ));
extern	LINEPTR	lback_p2p	P(( LINEPTR ));
extern	void	set_lforw_p2r	P(( LINE *, LINEPTR ));
extern	void	set_lback_p2r	P(( LINE *, LINEPTR ));
extern	void	set_lforw_p2p	P(( LINEPTR, LINEPTR ));
extern	void	set_lback_p2p	P(( LINEPTR, LINEPTR ));

#endif

#if NO_LEAKS
extern	void onel_leaks P(( void ));
extern	void path_leaks P(( void ));
extern	void kbs_leaks P(( void ));
extern	void tmp_leaks P(( void ));
extern	void tb_leaks P(( void ));
extern	void wp_leaks P(( void ));
extern	void bp_leaks P(( void ));
extern	void vt_leaks P(( void ));
extern	void ev_leaks P(( void ));
#endif

#if X11
extern void update_scrollbar P(( WINDOW *uwp ));
extern	void x_set_rv P(( void ));
extern	int x_setfont P(( char * ));
extern	void x_setname P(( char * ));
extern	void x_set_wm_title P(( char * ));
extern	char *x_current_fontname P(( void ));
extern	void x_setforeground P(( char * ));
extern	void x_setbackground P(( char * ));
extern	void x_preparse_args P(( int *, char *** ));
extern  void x_set_geometry P(( char * ));
extern	void x_set_dpy P(( char * ));
extern	void x_putline P(( int, char *, int ));
extern	int x_key_events_ready P(( void ));
extern	int x_is_pasting P((void));
extern	int x_on_msgline P((void));
#if OPT_WORKING
extern	void x_working P(( void ));
#endif
#if NO_LEAKS
extern	void x11_leaks P(( void ));
#endif
#endif	/* X11 */

#if MSDOS
/* ibmpc.c */
extern	void scwrite P(( int, int, int, char *, int, int ));
extern VIDEO *scread P(( VIDEO *, int ));
/* random.c */
extern char * curr_dir_on_drive P(( int ));
extern int curdrive P(( void ));
extern int setdrive P(( int ));
extern void update_dos_drv_dir P(( char * ));
# if WATCOM
     extern int dos_crit_handler P(( unsigned, unsigned, unsigned *));
# else
     extern void dos_crit_handler P(( void ));
# endif
# if OPT_MS_MOUSE
     extern int ms_exists P(( void ));
     extern void ms_processing P(( void ));
# endif
#endif

#if UNIX && !LINUX
#if APOLLO && __GNUC__
extern	int	access	P(( char *, int ));
extern	UINT	alarm	P(( UINT ));
extern	void	bzero	P(( char *, int ));
extern	int	chdir	P(( char * ));
extern	int	close	P(( int ));
extern	int	dup	P(( int ));
extern	int	execlp	P(( char *, ... ));
extern	int	fork	P(( void ));
extern	int	getpgrp	P(( int ));
extern	int	getpid	P(( void ));
extern	int	getuid	P(( void ));
extern	char *	getwd	P(( char * ));
extern	int	ioctl	P(( int, ULONG, caddr_t ));
extern	int	isatty	P(( int ));
extern	int	killpg	P(( int, int ));
extern	int	mkdir	P(( char *, int ));
extern	int	open	P(( char *, int ));
extern	int	pipe	P(( int * ));
extern	int	read	P(( int, char *, int ));
extern	int	select	P(( int, fd_set*, fd_set*, fd_set*, struct timeval* ));
extern	int	sleep	P(( UINT ));
extern	int	unlink	P(( char * ));
extern	int	write	P(( int, char *, int ));
#endif
#if HPUX && __GNUC__
#include <fcntl.h>	/* 'open()' */
#include <sys/wait.h>	/* 'wait()' */
#endif
#if (SUNOS || NeXT) && (defined(lint) || __GNUC__ || defined(__CLCC__))
extern	int	_filbuf	P(( FILE * ));
extern	int	_flsbuf	P(( int, FILE * ));
extern	void	bzero	P(( char *, int ));
#ifdef __CLCC__
extern	int	execlp	P(( char *, ... ));
#endif
extern	int	fclose	P(( FILE * ));
extern	int	fflush	P(( FILE * ));
extern	int	fprintf	P(( FILE *, const char *, ... ));
extern	int	fgetc	P(( FILE * ));
#ifndef	fileno
extern	int	fileno	P(( FILE * ));
#endif
extern	int	fputc	P(( int, FILE * ));
extern	int	fread	P(( char *, int, int, FILE * ));
extern	int	fseek	P(( FILE *, long, int ));
extern	int	fwrite	P(( const char *, SIZE_T, SIZE_T, FILE * ));
extern	int	ioctl	P(( int, ULONG, caddr_t ));
extern	int	killpg	P(( int, int ));
#ifndef	__GNUC__
extern	int	mkdir	P(( char *, int ));
extern	int	open	P(( char *, int ));
#endif
#ifndef NeXT
extern	void	perror	P(( const char * ));
#endif /* !NeXT */
extern	int	printf	P(( const char *, ... ));
extern	int	puts	P(( const char * ));
extern	int	sscanf	P(( const char *, const char *, ... ));
extern	int	select	P(( int, fd_set*, fd_set*, fd_set*, struct timeval* ));
extern	void	setbuf	P(( FILE *, char * ));
extern	void	setbuffer P(( FILE *, char *, int ));
extern	int	system	P(( const char * ));
extern	long	time	P(( long * ));
extern	int	wait	P(( int * ));
extern	long	strtol	P(( const char *, char **, int ));
#endif /* lint || __GNUC__ */
#ifdef NeXT
extern int	isatty	P((int));
extern int	atoi	P((char *));
extern int	dup	P((int));
extern int	close	P((int));
extern void	free	P((void *ptr));
extern void	memset	P((char *, int ch, int n));
extern int	access	P((char *, int));
extern int	read	P((int, char *, int));
extern int	write	P((int, char *, int));
extern void	qsort	P((void *, size_t, size_t , int (*compar)(void *, void *)));
extern int	pipe	P((int *));
extern int	kill	P((int, int));
extern int	execlp	P((char *, char *, ...));
extern int	fork	P((void));
extern int	sleep	P((int));
extern int	getuid	P((void));
extern int	chdir	P((char *));
extern int	getpgrp	P((int));
extern int	unlink	P((char *));
extern int	setjmp	P((jmp_buf env));
extern void	longjmp	P((jmp_buf env, int val));
#endif /* NeXT */
#if HAVE_GETHOSTNAME
extern int	gethostname P((char *, int));
#endif
#endif /* UNIX */

#if DJGPP
/* djhandl.c */
extern u_long was_ctrl_c_hit P(( void ));
extern void want_ctrl_c P(( int ));
extern void clear_hard_error P(( void ));
extern void hard_error_catch_setup P(( void ));
extern void hard_error_teardown P(( void ));
extern int did_hard_error_occur P(( void ));
extern void delay P(( int ));
#endif
