/*
 *   This file was automatically generated by cextract version 1.2.
 *   Manual editing now recommended, since I've done a whole lot of it.
 *
 *   Created: Thu May 14 15:44:40 1992
 *
 * $Log: proto.h,v $
 * Revision 1.44  1993/04/09 13:43:18  pgf
 * added a bunch of missing prototypes, found with -Wmissing-prototypes
 *
 * Revision 1.43  1993/04/08  15:00:53  pgf
 * new funcs for insertion
 *
 * Revision 1.42  1993/04/08  11:08:43  pgf
 * arg change to updpos
 *
 * Revision 1.41  1993/04/08  09:48:27  pgf
 * added tb_stuff
 *
 * Revision 1.40  1993/04/01  13:07:50  pgf
 * see tom's 3.40 CHANGES
 *
 * Revision 1.39  1993/03/31  19:36:08  pgf
 * changes for tags.c (tags path implementation)
 *
 * Revision 1.38  1993/03/25  19:50:58  pgf
 * see 3.39 section of CHANGES
 *
 * Revision 1.37  1993/03/18  17:42:20  pgf
 * see 3.38 section of CHANGES
 *
 * Revision 1.36  1993/03/17  10:01:18  pgf
 * overwrite() renamed to overwritechars()
 *
 * Revision 1.35  1993/03/16  10:53:21  pgf
 * see 3.36 section of CHANGES file
 *
 * Revision 1.34  1993/03/05  17:50:54  pgf
 * see CHANGES, 3.35 section
 *
 * Revision 1.33  1993/02/24  10:59:02  pgf
 * see 3.34 changes, in CHANGES file
 *
 * Revision 1.32  1993/02/12  10:43:33  pgf
 * new function, insertion_cmd()
 *
 * Revision 1.31  1993/02/08  14:53:35  pgf
 * see CHANGES, 3.32 section
 *
 * Revision 1.30  1993/01/23  13:38:23  foxharp
 * couple of new funcs, some now static (dfout..)
 *
 * Revision 1.29  1993/01/16  10:41:21  foxharp
 * some new routines, some old are now static, so don't appear here
 *
 * Revision 1.28  1993/01/12  08:48:43  foxharp
 * tom dickey's changes to support "set number", i.e. line numbering
 *
 * Revision 1.27  1992/12/23  09:22:40  foxharp
 * some new, some ifdefed UNUSED
 *
 * Revision 1.26  1992/12/14  09:03:25  foxharp
 * lint cleanup, mostly malloc
 *
 * Revision 1.25  1992/12/05  13:52:20  foxharp
 * make the apollo compiler happy
 *
 * Revision 1.24  1992/12/03  00:32:59  foxharp
 * new system_SHELL and exec_sh_c routines
 *
 * Revision 1.23  1992/11/19  09:16:43  foxharp
 * rename of kdelete() to ksetup(), and new kdone().
 * also, new X11 routines, x_setname, x_setforeground, and x_setbackground
 *
 * Revision 1.22  1992/11/19  08:50:16  foxharp
 * gettagsfile now returns a BUFFER *
 *
 * Revision 1.21  1992/08/19  23:00:37  foxharp
 * new DOS routines for directory manip.
 *
 * Revision 1.20  1992/08/06  23:55:07  foxharp
 * added routines that deal with DOS drives
 *
 * Revision 1.19  1992/08/04  20:09:31  foxharp
 * prototype fixups for xvile
 *
 * Revision 1.18  1992/07/28  22:02:55  foxharp
 * patchstk() renamed applypatch()
 *
 * Revision 1.17  1992/07/24  18:22:51  foxharp
 * deleted local atoi() routine -- now we use the system's copy
 *
 * Revision 1.16  1992/07/24  07:49:38  foxharp
 * shorten_name changes
 *
 * Revision 1.15  1992/07/21  09:09:51  foxharp
 * pass lp to vtset() directly
 *
 * Revision 1.14  1992/07/21  08:57:53  foxharp
 * wp param to vtset(), for list mode choice
 *
 * Revision 1.13  1992/07/20  22:48:42  foxharp
 * changes...
 *
 * Revision 1.12  1992/07/18  13:13:22  foxharp
 * created shorten_path and vtprintf and lssetbuf/_lsprintf
 *
 * Revision 1.11  1992/07/16  22:18:54  foxharp
 * ins() takes an argument -- whether or not to playback, usually FALSE
 *
 * Revision 1.10  1992/07/13  09:27:33  foxharp
 * added getkill, canonpath, and changed current_directory to take int
 *
 * Revision 1.9  1992/07/07  08:34:08  foxharp
 * added not_found_msg, from search.c
 *
 * Revision 1.8  1992/06/26  22:19:10  foxharp
 * added dos argument globber
 *
 * Revision 1.7  1992/06/04  19:42:37  foxharp
 * use #ifdef __STDC__ in favor of #if
 *
 * Revision 1.6  1992/05/29  08:36:53  foxharp
 * added new ..._fence routines
 *
 * Revision 1.5  1992/05/25  21:28:37  foxharp
 * took out extern decls of system and library calls, since they conflict
 * more often than not, and added some more routine declarations that
 * cextract (an old version) missed
 *
 * Revision 1.4  1992/05/19  18:28:04  foxharp
 * more proto-isms
 *
 * Revision 1.3  1992/05/19  09:15:45  foxharp
 * portability stuff
 *
 * Revision 1.2  1992/05/16  14:02:55  pgf
 * header/typedef fixups
 *
 * Revision 1.1  1992/05/16  11:50:17  pgf
 * Initial revision
 *
 */

extern int main P(( int, char *[] ));
#if MSDOS
extern void expand_wild_args P(( int * , char ***));
#endif /* MSDOS */
extern void loop P(( void ));
extern char * strmalloc P(( char * ));
extern void global_val_init P(( void ));
extern DEFINE_SIGNAL(catchintr);
extern void do_num_proc P(( int *, int *, int * ));
extern void do_rept_arg_proc P(( int *, int *, int * ));
extern int writeall P(( int, int ));
extern int zzquit P(( int, int ));
extern int quickexit P(( int, int ));
extern int quithard P(( int, int ));
extern int quit P(( int, int ));
extern int writequit P(( int, int ));
extern int esc P(( int, int ));
extern int rdonly P(( void ));
extern int showversion P(( int, int ));
extern int unimpl P(( int, int ));
extern int opercopy P(( int, int ));
extern int opermove P(( int, int ));
extern int opertransf P(( int, int ));
extern int operglobals P(( int, int ));
extern int opervglobals P(( int, int ));
extern int map P(( int, int ));
extern int unmap P(( int, int ));
extern int source P(( int, int ));
extern int visual P(( int, int ));
extern int ex P(( int, int ));
extern int cntl_af P(( int, int ));
extern int cntl_xf P(( int, int ));
extern int unarg P(( int, int ));
extern int nullproc P(( int, int ));
extern void charinit P(( void ));
extern void start_debug_log P(( int , char ** ));
extern void tcapopen P(( void ));
extern void tcapclose P(( void ));
extern void tcapkopen P(( void ));
extern void tcapkclose P(( void ));
extern void tcaprev P(( int ));
extern int tcapcres P(( int ));
extern void tcapmove P(( int, int ));
extern void tcapeeol P(( void ));
extern void tcapeeop P(( void ));
extern void tcapscroll_reg P(( int, int, int ));
extern void tcapscroll_delins P(( int, int, int ));
extern void tcapscrollregion P(( int, int ));
extern void spal P(( char * ));
extern void tcapbeep P(( void ));
extern void putpad P(( char * ));
extern void putnpad P(( char *, int ));

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
extern int lastnonwhite P(( int, int ));
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
extern int gonmmark P(( int ));
extern int setmark P(( void ));
extern int gomark P(( int, int ));
extern int godotplus P(( int, int ));
extern void swapmark P(( void ));

/* bind.c */
extern int help P(( int, int ));
extern int deskey P(( int, int ));
extern int bindkey P(( int, int ));
extern int desbind P(( int, int ));
extern void makebindlist P(( int, char *));
extern int strinc P(( char *, char *));
extern int unbindkey P(( int, int ));
extern int unbindchar P(( int ));
extern int apro P(( int, int ));
extern int startup P(( char *));
extern char * flook P(( char *, int ));
extern char * kcod2str P(( int, char * ));
extern char * kcod2prc P(( int, char * ));
extern int insertion_cmd P(( void ));
extern int fnc2kcod P(( CMDFUNC * ));
extern char * fnc2engl P(( CMDFUNC * ));
extern CMDFUNC * engl2fnc P(( char * ));
extern CMDFUNC * kcod2fnc P(( int ));
extern int prc2kcod P(( char * ));
extern char * prc2engl P(( char * ));
extern int fnc2key P(( CMDFUNC * ));
extern char * kbd_engl P(( char *, char * ));
extern void kbd_alarm P(( void ));
extern void kbd_putc P(( int ));
extern void kbd_puts P(( char * ));
extern void kbd_erase P(( void ));
extern void kbd_init P(( void ));
extern void kbd_unquery P(( void ));
extern int kbd_complete P(( int, char *, int *, char *, unsigned ));
extern int kbd_engl_stat P(( char *, char * ));

/* buffer.c */
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
void updatelistbuffers P((void));
extern int addline P(( BUFFER *, char *, int ));
extern int anycb P(( void ));
extern BUFFER * bfind P(( char *, int, int ));
extern int bclear P(( BUFFER * ));
extern int bsizes P(( BUFFER * ));
extern void chg_buff P(( BUFFER *, int ));
extern void unchg_buff P(( BUFFER *, int ));
extern int unmark P(( int, int ));

/* crypt.c */
#if	CRYPT
extern	int	setkey P((int, int));
extern	void	crypt P((char *, int));
#else
extern void nocrypt P(( void ));
#endif	/* CRYPT */
extern int fscan P(( int, int, int ));
extern int bscan P(( int, int, int ));
extern int fcsrch P(( int, int ));
extern int bcsrch P(( int, int ));
extern int fcsrch_to P(( int, int ));
extern int bcsrch_to P(( int, int ));
extern int rep_csrch P(( int, int ));
extern int rev_csrch P(( int, int ));

/* display.c */
extern int nu_mode P(( WINDOW * ));
extern int nu_width P(( WINDOW * ));
extern int col_limit P(( WINDOW * ));
extern void vtinit P(( void ));
extern void vttidy P(( int ));
extern void vtmove P(( int, int ));
extern void vtputc P(( int ));
extern void vtlistc P(( int ));
extern int vtgetc P(( int ));
extern void vtputsn P(( char *, int ));
extern void vtset P(( LINE *, WINDOW * ));
extern void vtprintf P(( char *, ... ));
extern void vteeol P(( void ));
extern int upscreen P(( int, int ));
extern void reframe P(( WINDOW * ));
extern void l_to_vline P(( WINDOW *, LINE *, int ));
extern int updpos P(( int * ));
extern void upddex P(( void ));
extern void updgar P(( void ));
extern int update P(( int ));
extern void upmode P(( void ));
extern void updateline P(( int, struct VIDEO *, struct VIDEO * ));
extern void updone P(( WINDOW * ));
extern void updall P(( WINDOW * ));
extern void updupd P(( int ));
extern int scrolls P(( int ));
extern int texttest P(( int, int ));
extern void scrscroll P(( int, int, int ));
extern int endofline P(( char *, int ));
extern int updext_past P(( int, int ));
extern int updext_before P(( int ));
extern void modeline P(( WINDOW * ));
extern void movecursor P(( int, int ));
extern void mlerase P(( void ));
extern void mlsavec P(( int ));
extern void mlwrite P((char *, ... ));
extern void mlforce P((char *, ... ));
extern void mlprompt P((char *, ... ));
extern void mlmsg P((char *, va_list * ));
extern void dbgwrite P((char *, ... ));
extern void lspputc P(( int ));
extern char * lsprintf P((char *, char *, ... ));
#ifdef	UNUSED
extern void lssetbuf P(( char * ));
extern char * _lsprintf P(( char *, ... ));
#endif	/* UNUSED */
extern void bputc P(( int ));
extern void bprintf P((char *, ... ));
extern void getscreensize P(( int *, int * ));
extern DEFINE_SIGNAL(sizesignal);
extern void newscreensize P(( int, int ));

/* eval.c */
extern char * l_itoa P(( int ));
extern char * ltos P(( int ));
extern int absol P(( int ));
extern int stol P(( char * ));
extern int gtlbl P(( char * ));
extern char * gtenv P(( char * ));
extern char * getkill P(( void ));
extern char * gtusr P(( char * ));
extern char * mklower P(( char * ));
extern char * mkupper P(( char * ));
extern int svar P(( VDESC *, char * ));
extern void varinit P(( void ));
extern int listvars P(( int, int ));
extern int setvar P(( int, int ));
extern int sindex P(( char *, char * ));
extern int ernd P(( void ));

/* exec.c */
extern int namedcmd P(( int, int ));
extern int rangespec P(( char *, LINE **, LINE **, int *, int * ));
extern int docmd P(( char *, int, int, int ));
extern int dobuf P(( BUFFER * ));
extern int dofile P(( char * ));
extern int execbuf P(( int, int ));
extern int execfile P(( int, int ));
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

/* file.c */
extern int no_such_file P(( char * ));
extern int fileread P(( int, int ));
extern int filefind P(( int, int ));
extern int viewfile P(( int, int ));
extern int insfile P(( int, int ));
extern int getfile P(( char *, int ));
extern int readin P((char *, int, BUFFER *, int ));
extern int quickreadf P(( BUFFER *, int * ));
extern int slowreadf P(( BUFFER *, int * ));
extern int set_dosmode P(( int, int ));
extern void readlinesmsg P(( int, int, char *, int ));
extern void makename P(( char [], char [] ));
extern void unqname P((char *, int));
extern int filewrite P(( int, int ));
extern int filesave P(( int, int ));
extern int writeout P(( char *, BUFFER *, int ));
extern int writeregion P(( void ));
extern int writereg P(( REGION *, char *, int, int, BUFFER ** ));
extern int kwrite P(( char *, int ));
extern int filename P(( int, int ));
extern int ifile P(( char *, int, FILE * ));
extern int kifile P(( char * ));
extern DEFINE_SIGNAL(imdying);
extern void markWFMODE P(( BUFFER * ));

/* filec.c */
#if UNIX || defined(MDTAGSLOOK)
extern BUFFER *bs_init P(( char *, int ));
extern int bs_find P(( char *, int, BUFFER *, int, LINE ** ));
#endif
extern int mlreply_file P(( char *, TBUFF **, int, char * ));
extern int mlreply_dir P(( char *, TBUFF **, char * ));

/* fileio.c */
extern int ffropen P(( char * ));
extern int ffwopen P(( char * ));
extern int ffronly P(( char * ));
extern long ffsize P(( void ));
extern int ffread P(( char *, long ));
extern void ffseek P(( long ));
extern void ffrewind P(( void ));
extern int ffclose P(( void ));
extern int ffputline P(( char [], int, int ));
extern int ffputc P(( int ));
extern int ffhasdata P(( void ));
extern int finderr P(( int, int ));
extern struct LINE * getdot P(( struct BUFFER * ));
extern void putdotback P(( BUFFER *, LINE * ));
extern int globals P(( int, int ));
extern int vglobals P(( int, int ));
extern int globber P(( int, int, int ));

/* history.c */
#if !SMALLER
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
extern int mlreply P(( char *, char *, int ));
extern int mlreply_no_bs P(( char *, char *, int ));
extern int mlreply_no_opts P(( char *, char *, int ));
extern void incr_dot_kregnum P(( void ));
extern void tungetc P(( int ));
extern int tpeekc P(( void ));
extern int get_recorded_char P(( int ));
extern int tgetc P(( int ));
extern int kbd_key P(( void ));
extern int kbd_seq P(( void ));
extern int screen_string P(( char *, int, int ));
extern int end_string P(( void ));
extern int is_edit_char P(( int ));
extern void kbd_kill_response P(( char *, int *, int ));
extern int kbd_show_response P(( char *, char *, int, int, int ));
extern int kbd_string P(( char *, char *, int, int, int, int (*func)(int,char*,int*) ));
extern int kbd_reply P(( char *, char *, int, int (*efunc)(char *,int,int,int), int, int, int (*cfunc)(int,char*,int*) ));
extern int speckey P(( int, int ));
extern int dotcmdbegin P(( void ));
extern int dotcmdfinish P(( void ));
extern void dotcmdstop P(( void ));
extern int dotcmdplay P(( int, int ));
extern int dotreplaying P(( int ));
extern int kbd_replaying P(( void ));
extern int kbd_mac_begin P(( int, int ));
extern int kbd_mac_end P(( int, int ));
extern int kbd_mac_exec P(( int, int ));
extern int kbd_mac_save P(( int, int ));
extern int kbm_started P(( int, int ));
extern int start_kbm P(( int, int, TBUFF * ));
extern void finish_kbm P(( void ));

/* isearch.c */
extern int risearch P(( int, int ));
extern int fisearch P(( int, int ));
extern int isearch P(( int, int ));
extern int promptpattern P(( char * ));
extern int get_char P(( void ));

/* line.c */
extern LINE * lalloc P(( int, BUFFER * ));
extern void lfree P(( LINE *, BUFFER * ));
extern void ltextfree P(( LINE *, BUFFER * ));
extern void lremove P(( BUFFER *, LINE * ));
extern int insspace P(( int, int ));
extern int linsert P(( int, int ));
extern int lnewline P(( void ));
extern int ldelete P(( long, int ));
extern char * getctext P(( int ));
extern int putctext P(( char * ));
extern int ldelnewline P(( void ));
extern void ksetup P(( void ));
extern void kdone P(( void ));
extern int kinsert P(( int ));
extern int index2reg P(( int ));
extern int reg2index P(( int ));
extern int usekreg P(( int, int ));
extern void kregcirculate P(( int ));
extern int putbefore P(( int, int ));
extern int putafter P(( int, int ));
extern int lineputbefore P(( int, int ));
extern int lineputafter P(( int, int ));
extern int doput P(( int, int, int, int ));
extern int put P(( int, int ));
extern int execkreg P(( int, int ));
extern int loadkreg P(( int, int ));
extern int showkreg P(( int, int ));

/* modes.c */
extern int setmode P(( int, int ));
extern int delmode P(( int, int ));
extern int setgmode P(( int, int ));
extern int delgmode P(( int, int ));
extern int settab P(( int, int ));
extern int setfillcol P(( int, int ));
extern REGEXVAL *new_regexval P(( char *, int ));
extern void copy_val P(( struct VAL *, struct VAL *, int ));
extern void free_regexval P(( REGEXVAL * ));
extern void free_val P(( struct VALNAMES *, struct VAL * ));
#if NO_LEAKS
extern void free_local_vals P(( struct VALNAMES *, struct VAL *, struct VAL * ));
#endif
extern int listmodes P(( int, int ));

/* npopen.c */
#if UNIX
extern FILE * npopen P(( char *, char * ));
extern int inout_popen P(( FILE **, FILE **, char * ));
extern void npclose P(( FILE * ));
extern void exec_sh_c P(( char * ));
extern int system_SHELL P(( char * ));
extern int softfork P(( void ));
#endif

/* oneliner.c */
extern int pregion P(( int ));
extern int llineregion P(( void ));
extern int plineregion P(( void ));
extern int substregion P(( void ));
extern int subst_again_region P(( void ));
extern int subst_again P(( int, int ));
extern int substreg1 P(( int, int ));
extern int substline P(( regexp *, int, int, int ));
extern int delins P(( regexp *, char * ));

/* opers.c */
extern int operator P(( int, int, int (*)(), char * ));
extern int operdel P(( int, int ));
extern int operlinedel P(( int, int ));
extern int chgreg P(( void ));
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

/* path.c */
#if MSDOS
extern char * is_msdos_drive P(( char * ));
#endif
#if VMS
extern int is_vms_pathname P(( char *, int ));
extern char * vms_pathleaf P(( char * ));
#endif
extern char * pathleaf P(( char * ));
extern char * pathcat P(( char *, char *, char * ));
extern char * last_slash P(( char * ));
extern int glob P(( char * ));
extern char * canonpath P(( char * ));
extern char * shorten_path P(( char *, int ));
extern char * lengthen_path P(( char * ));
extern int is_pathname P(( char * ));
extern int maybe_pathname P(( char * ));
extern char * is_appendname P(( char * ));
extern char * non_filename P(( void ));
extern int is_internalname P(( char * ));

/* random.c */
extern int line_no P(( BUFFER *, LINE * ));
extern int getcline P(( void ));
extern int liststuff P(( char *, void (*)(), int, char * ));
extern int showcpos P(( int, int ));
extern int showlength P(( int, int ));
extern int getccol P(( int ));
extern int gotocol P(( int, int ));
extern int gocol P(( int ));
extern int twiddle P(( int, int ));
extern int quote P(( int, int ));
extern int replacechar P(( int, int ));
extern int tab P(( int, int ));
extern int shiftwidth P(( int, int ));
extern int detabline P(( int ));
extern int detab_region P(( void ));
extern int entabline P(( int ));
extern int entab_region P(( void ));
extern int trimline P(( void ));
extern int trim_region P(( void ));
extern int openup P(( int, int ));
extern int opendown P(( int, int ));
extern int openlines P(( int ));
extern int insert P(( int, int ));
extern int insertbol P(( int, int ));
extern int append P(( int, int ));
extern int appendeol P(( int, int ));
extern int overwritechars P(( int, int ));
extern int ins P(( int ));
extern int insstring P(( int, int ));
extern int overwstring P(( int, int ));
extern int istring P(( int, int, int ));
extern int inschar P(( int, int ));
extern int backspace P(( void ));
extern int newline P(( int, int ));
extern int indented_newline P(( int ));
extern int indented_newline_above P(( int ));
extern int previndent P(( int * ));
extern int nextindent P(( int * ));
extern int doindent P(( int ));
extern int indentlen P(( LINE * ));
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
extern int writemsg P(( int, int ));
extern int fmatchindent P(( void ));
extern void catnap P(( int ));
extern char * current_directory P(( int ));
extern int cd P(( int, int ));
extern int pwd P(( int, int ));
extern int set_directory P(( char * ));
extern void ch_fname P(( BUFFER *, char * ));

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
extern int yankregion P(( void ));
extern int shift_right_line P(( void ));
extern int shiftrregion P(( void ));
extern int shift_left_line P(( void ));
extern int shiftlregion P(( void ));
extern int _to_lower P(( int ));
extern int _to_upper P(( int ));
extern int _to_caseflip P(( int ));
extern int flipregion P(( void ));
extern int lowerregion P(( void ));
extern int upperregion P(( void ));
extern int charprocreg P(( int (*)() ));
extern int getregion P(( REGION * ));
extern int do_fl_region P(( int (*)(), int ));

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
extern int eq P(( int, int ));
extern int scrsearchpat P(( int, int ));
extern int readpattern P(( char *, char *, regexp **, int, int ));
extern void savematch P(( MARK, int ));
extern void scanboundry P(( int, MARK, int ));
extern void nextch P(( MARK *, int ));
extern int findpat P(( int, int, regexp *, int ));

/* spawn.c */
extern int spawncli P(( int, int ));
extern DEFINE_SIGNAL(rtfrmshell);
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
extern void nth_name P(( char *,  char *, int ));
extern BUFFER * gettagsfile P(( int, int * ));
extern LINE * cheap_scan P(( BUFFER *, char *, int ));
extern int untagpop P(( int, int ));
extern void pushuntag P(( char *, int ));
extern int popuntag P(( char *, int * ));
extern void tossuntag P(( void ));
#ifdef GMDTAGSLOOK
extern BUFFER * look_tags P(( int ));
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
extern void toss_to_undo P(( LINE * ));
extern int copy_for_undo P(( LINE * ));
extern int tag_for_undo P(( LINE * ));
extern void pushline P(( LINE *, LINE ** ));
extern LINE * popline P(( LINE ** ));
extern void make_undo_patch P(( LINE *, LINE *, int ));
extern void applypatch P(( LINE *, LINE * ));
extern LINE * copyline P(( LINE * ));
extern void freeundostacks P(( BUFFER * ));
extern int undo P(( int, int ));
extern void mayneedundo P(( void ));
extern void preundocleanup P(( void ));
extern int lineundo P(( int, int ));
extern void repointstuff P(( LINE *, LINE * ));
extern int linesmatch P(( LINE *, LINE * ));
extern void dumpuline P(( LINE * ));
extern void setupuline P(( LINE * ));
extern void resetuline P(( LINE *, LINE * ));

/* window.c */
extern int getwpos P(( void ));
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
extern WINDOW * splitw P(( int, int ));
extern int splitwind P(( int, int ));
extern int enlargewind P(( int, int ));
extern int shrinkwind P(( int, int ));
extern WINDOW * wpopup P(( void ));
extern int scrnextup P(( int, int ));
extern int scrnextdw P(( int, int ));
extern int savewnd P(( int, int ));
extern int restwnd P(( int, int ));
extern int resize P(( int, int ));
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
extern int macarg P(( char * ));
extern int echochar P(( int, int ));
extern int scanmore P(( char *, int ));
extern int expandp P(( char *, char *, int ));
extern int scanner P((regexp *, int, int ));
extern int insbrace P(( int, int ));
extern int inspound P(( void ));
extern int fmatch P(( int ));
extern int getfence P(( int, int ));
extern int comment_fence P(( int ));
extern int simple_fence P(( int, int, int ));
extern void putdotback P(( BUFFER *, LINE * ));

/* tbuff.c */
TBUFF *	tb_alloc P(( TBUFF **, unsigned ));
TBUFF *	tb_init P(( TBUFF **, int ));
void	tb_free P(( TBUFF ** ));
TBUFF *	tb_put P(( TBUFF **, unsigned, int ));
void	tb_stuff P(( TBUFF *, int ));
int	tb_get P(( TBUFF *, unsigned ));
void	tb_unput P(( TBUFF * ));
TBUFF *	tb_append P(( TBUFF **, int ));
TBUFF *	tb_copy P(( TBUFF **, TBUFF * ));
TBUFF *	tb_bappend P(( TBUFF **, char *, unsigned ));
TBUFF *	tb_sappend P(( TBUFF **, char * ));
TBUFF *	tb_scopy P(( TBUFF **, char * ));
void	tb_first P(( TBUFF * ));
int	tb_more P(( TBUFF * ));
int	tb_next P(( TBUFF * ));
int	tb_peek P(( TBUFF * ));
char *	tb_values P(( TBUFF * ));
unsigned tb_length P(( TBUFF * ));

#if NO_LEAKS
extern	void kbs_leaks P(( void ));
extern	void tb_leaks P(( void ));
extern	void wp_leaks P(( void ));
extern	void bp_leaks P(( void ));
extern	void vt_leaks P(( void ));
extern	void ev_leaks P(( void ));
#endif

#if X11
extern	void x_set_rv P(( void ));
extern	int x_setfont P(( char * ));
extern	void x_setname P(( char * ));
extern	void x_setforeground P(( char * ));
extern	void x_setbackground P(( char * ));
extern	void x_preparse_args P(( int *, char *** ));
extern  void x_set_geometry P(( char * ));
extern	void x_set_dpy P(( char * ));
extern	void setcursor P(( int, int ));
extern	void x_putline P(( int, char *, int ));
extern	int x_key_events_ready P(( void ));
#endif

#if MSDOS
/* ibmpc.c */
extern	void scwrite P(( int, int, int, char *, int, int ));
/* random.c */
extern char * curr_dir_on_drive P(( int ));
extern int curdrive P(( void ));
extern int setdrive P(( int ));
extern void update_dos_drv_dir P(( char * ));
extern void dos_crit_handler P(( void ));
#endif

#if UNIX
#if SUNOS && (defined(lint) || __GNUC__)
extern	int	_filbuf	P(( FILE * ));
extern	int	_flsbuf	P(( unsigned char, FILE * ));
extern	int	printf	P(( char *, ... ));
extern	int	fclose	P(( FILE * ));
extern	int	fflush	P(( FILE * ));
extern	int	fprintf	P(( FILE *, char *, ... ));
extern	int	fgetc	P(( FILE * ));
extern	int	fputc	P(( char, FILE * ));
extern	int	fread	P(( char *, int, int, FILE * ));
extern	int	fseek	P(( FILE *, long, int ));
extern	int	fwrite	P(( char *, int, int, FILE * ));
extern	int	ioctl	P(( int, int, caddr_t ));
extern	int	killpg	P(( int, int ));
extern	int	mkdir	P(( char *, int ));
extern	void	perror	P(( char * ));
extern	int	puts	P(( char * ));
extern	int	sscanf	P(( char *, char *, ... ));
extern	int	select	P(( int, fd_set*, fd_set*, fd_set*, struct timeval* ));
extern	void	setbuf	P(( FILE *, char * ));
extern	void	setbuffer P(( FILE *, char *, int ));
extern	int	system	P(( char * ));
extern	int	wait	P(( int * ));
#endif
#endif /* UNIX */
