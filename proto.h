/*
 *   This file was automatically generated by cextract version 1.2.
 *   Manual editing now recommended, since I've done a whole lot of it.
 *
 *   Created: Thu May 14 15:44:40 1992
 *
 * $Log: proto.h,v $
 * Revision 1.5  1992/05/25 21:28:37  foxharp
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

#if __STDC__
# define P(a) a
#else
# define P(a) ()
#endif

extern int main P(( int, char *[] ));
extern void loop P(( void ));
extern char * strmalloc P(( char * ));
extern void global_val_init P(( void ));
extern SIGT catchintr P(( int ));
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
extern int nullproc P(( int, int ));
extern void charinit P(( void ));
extern void tcapopen P(( void ));
extern void tcapclose P(( void ));
extern void tcapkopen P(( void ));
extern void tcapkclose P(( void ));
extern void tcaprev P(( int ));
extern int tcapcres P(( void ));
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
extern int swapmark P(( void ));
extern int help P(( int, int ));
extern int deskey P(( int, int ));
extern int desbind P(( int, int ));
extern void makebindlist P(( int, char *));
extern int strinc P(( char *, char *));
extern int unbindchar P(( int ));
extern int apro P(( int, int ));
extern char * kbd_engl P(( void ));
extern int kbd_engl_stat P(( char ** ));
extern char * hist_lookup P(( int ));
extern int hist_show P(( void ));
extern int histbuff P(( int, int ));
extern int altbuff P(( int, int ));
extern int usebuffer P(( int, int ));
extern int nextbuffer P(( int, int ));
extern int prevbuffer P(( int, int ));
extern void make_current P(( BUFFER * ));
extern int swbuffer P(( BUFFER * ));
extern void undispbuff P(( BUFFER *, WINDOW * ));
extern int tabstop_val P(( BUFFER * ));
extern int has_C_suffix P(( BUFFER * ));
extern int killbuffer P(( int, int ));
extern int zotbuf P(( BUFFER * ));
extern int popupbuff P(( BUFFER * ));
extern int readin P((char *, int, BUFFER *, int ));
extern int togglelistbuffers P(( int, int ));
extern int listbuffers P(( int, int ));
extern void makebufflist P(( int, char * ));
extern int startup P(( char *));
extern int addline P(( BUFFER *, char *, int ));
extern int anycb P(( void ));
extern BUFFER * bfind P(( char *, int, int ));
extern int bclear P(( BUFFER * ));
extern void nocrypt P(( void ));
extern int fscan P(( int, int, int ));
extern int bscan P(( int, int, int ));
extern int fcsrch P(( int, int ));
extern int bcsrch P(( int, int ));
extern int fcsrch_to P(( int, int ));
extern int bcsrch_to P(( int, int ));
extern int rep_csrch P(( int, int ));
extern int rev_csrch P(( int, int ));
extern void vtinit P(( void ));
extern void vttidy P(( int ));
extern void vtmove P(( int, int ));
extern void vtputc P(( int, int ));
extern int vtgetc P(( int ));
extern void vtputsn P(( char *, int ));
extern void vteeol P(( void ));
extern int upscreen P(( int, int ));
extern void reframe P(( WINDOW * ));
extern void l_to_vline P(( WINDOW *, LINE *, int ));
extern int updpos P(( void ));
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
extern int updext_past P(( void ));
extern int updext_before P(( void ));
extern void modeline P(( WINDOW * ));
extern void movecursor P(( int, int ));
extern void mlerase P(( void ));
extern void mlsavec P(( int ));
extern void mlwrite P((char *, ... ));
extern void mlforce P((char *, ... ));
extern void mlprompt P((char *, ... ));
extern void mlmsg P((char *, va_list * ));
extern void dbgwrite P((char *, ... ));
extern void mlputc P(( int ));
extern void dofmt P((char *, va_list * ));
extern int dfputs P(( char * ));
extern int dfputsn P(( char *, int ));
extern int dfputf P(( int ));
extern int dfputi P(( int, int ));
extern int dfputli P(( long, int ));
extern void lspputc P(( int ));
extern char * lsprintf P((char *, char *, ... ));
extern void bputc P(( int ));
extern void bprintf P((char *, ... ));
extern void getscreensize P(( int *, int * ));
extern SIGT sizesignal P(( int ));
extern void newscreensize P(( int, int ));
extern int newwidth P(( int, int ));
extern int newlength P(( int, int ));
extern int atoi P(( char * ));
extern char * l_itoa P(( int ));
extern char * ltos P(( int ));
extern int absol P(( int ));
extern int stol P(( char * ));
extern int gtlbl P(( char * ));
extern char * gtenv P(( char * ));
extern char * gtusr P(( char * ));
extern char * mklower P(( char * ));
extern char * mkupper P(( char * ));
extern int sindex P(( char *, char * ));
extern int ernd P(( void ));
extern int getcline P(( void ));
extern int getwpos P(( void ));
extern int svar P(( VDESC *, char * ));
extern int resize P(( int, int ));
extern int namedcmd P(( int, int ));
extern int rangespec P(( char *, LINE **, LINE **, int *, int * ));
extern int docmd P(( char *, int, int, int ));
extern int dobuf P(( BUFFER * ));
extern int dofile P(( char * ));
extern int execute P(( CMDFUNC *, int, int ));
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
extern void ch_fname P(( BUFFER *, char * ));
extern int fileread P(( int, int ));
extern int getfile P(( char *, int ));
extern int filefind P(( int, int ));
extern int viewfile P(( int, int ));
extern int insfile P(( int, int ));
extern int quickreadf P(( BUFFER *, int * ));
extern int slowreadf P(( BUFFER *, int * ));
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
extern SIGT imdying P(( int ));
extern void markWFMODE P(( BUFFER * ));
extern int glob P(( char * ));
extern char * flook P(( char *, int ));
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
extern int mlyesno P(( char * ));
extern int mlreply P(( char *, char *, int ));
extern int kbd_string P((char *, char *, int, int, int, int));
extern void incr_dot_kregnum P(( void ));
extern void tungetc P(( int ));
extern int tpeekc P(( void ));
extern void record_char P(( int ));
extern int get_recorded_char P(( int ));
extern int tgetc P(( void ));
extern int kbd_key P(( void ));
extern int kbd_seq P(( void ));
extern int screen_string P(( char *, int, int ));
extern void remove_backslashes P(( char * ));
extern void ostring P(( char * ));
extern void outstring P(( char * ));
extern int speckey P(( int, int ));
extern int dotcmdbegin P(( void ));
extern int dotcmdfinish P(( void ));
extern void dotcmdstop P(( void ));
extern int dotcmdplay P(( int, int ));
extern int macarg P(( char * ));
extern int kbd_mac_begin P(( int, int ));
extern int kbd_mac_end P(( int, int ));
extern int kbd_mac_exec P(( int, int ));
extern int kbd_mac_save P(( int, int ));
extern int risearch P(( int, int ));
extern int fisearch P(( int, int ));
extern int isearch P(( int, int ));
extern int promptpattern P(( char * ));
extern int get_char P(( void ));
extern LINE * lalloc P(( int, BUFFER * ));
extern int lgrow P(( LINE *, int, BUFFER * ));
extern void lfree P(( LINE *, BUFFER * ));
extern void ltextfree P(( LINE *, BUFFER * ));
extern void lremove P(( BUFFER *, LINE * ));
extern void lchange P(( int ));
extern int linsert P(( int, int ));
extern int ldelete P(( long, int ));
extern int lnewline P(( void ));
extern char * getctext P(( int ));
extern int putctext P(( char * ));
extern int ldelnewline P(( void ));
extern void kdelete P(( void ));
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
extern FILE * npopen P(( char *, char * ));
extern int inout_popen P(( FILE **, FILE **, char * ));
extern void npclose P(( FILE * ));
extern int pregion P(( int ));
extern int llineregion P(( void ));
extern int plineregion P(( void ));
extern int substregion P(( void ));
extern int subst_again_region P(( void ));
extern int subst_again P(( int, int ));
extern int substreg1 P(( int, int ));
extern int substline P(( regexp *, int, int, int ));
extern int delins P(( regexp *, char * ));
extern int operator P(( int, int, int (*)(), char * ));
extern int operdel P(( int, int ));
extern int operlinedel P(( int, int ));
extern int chgreg P(( void ));
extern int operchg P(( int, int ));
extern int operlinechg P(( int, int ));
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
extern int liststuff P(( char *, void (*)(), int, char * ));
extern int listmodes P(( int, int ));
extern void makemodelist P(( int, char * ));
extern int listvalueset P(( struct VALNAMES *, struct VAL *, struct VAL * ));
extern int setfillcol P(( int, int ));
extern int showcpos P(( int, int ));
extern int showlength P(( int, int ));
extern int getccol P(( int ));
extern int gotocol P(( int, int ));
extern int gocol P(( int ));
extern int twiddle P(( int, int ));
extern int quote P(( int, int ));
extern int replacechar P(( int, int ));
extern int settab P(( int, int ));
extern int tab P(( int, int ));
extern int shiftwidth P(( void ));
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
extern int overwrite P(( int, int ));
extern int ins P(( void ));
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
extern int adjvalueset P(( char *, int, struct VALNAMES *, struct VAL * ));
extern int matchfence P(( int, int ));
extern int matchfenceback P(( int, int ));
extern int fmatchindent P(( void ));
extern void catnap P(( int ));
extern int istring P(( int, int ));
extern char * current_directory P(( void ));
extern int cd P(( int, int ));
extern int pwd P(( int, int ));
extern int set_directory P(( char * ));
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
extern void rvstrcpy P(( char *, char * ));
extern void rvstrncpy P(( char *, char *, int ));
extern void scanboundry P(( int, MARK, int ));
extern void nextch P(( MARK *, int ));
extern int findpat P(( int, int, regexp *, int ));
extern int spawncli P(( int, int ));
extern SIGT rtfrmshell P(( int ));
extern int bktoshell P(( int, int ));
extern void pressreturn P(( void ));
extern int respawn P(( int, int ));
extern int spawn P(( int, int ));
extern int spawn1 P(( int ));
extern int pipecmd P(( int, int ));
extern int filterregion P(( void ));
extern int filter P(( int, int ));
extern int gototag P(( int, int ));
extern int cmdlinetag P(( char * ));
extern int tags P(( char *, int ));
extern int gettagsfile P(( void ));
extern LINE * cheap_scan P(( BUFFER *, char *, int ));
extern int untagpop P(( int, int ));
extern void pushuntag P(( char *, int ));
extern int popuntag P(( char *, int * ));
extern void tossuntag P(( void ));
extern void ttopen P(( void ));
extern void ttclose P(( void ));
extern void ttclean P(( int ));
extern void ttunclean P(( void ));
extern void ttputc P(( int ));
extern void ttflush P(( void ));
extern int ttgetc P(( void ));
extern int typahead P(( void ));
extern void ttmiscinit P(( void ));
extern void toss_to_undo P(( LINE * ));
extern int copy_for_undo P(( LINE * ));
extern int tag_for_undo P(( LINE * ));
extern void pushline P(( LINE *, LINE ** ));
extern LINE * popline P(( LINE ** ));
extern void make_undo_patch P(( LINE *, LINE *, int ));
extern void patchstk P(( LINE *, LINE * ));
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
extern int reposition P(( int, int ));
extern int refresh P(( int, int ));
extern int poswind P(( int, int ));
extern int prevwind P(( int, int ));
extern int mvdnwind P(( int, int ));
extern int mvupwind P(( int, int ));
extern int mvdnnxtwind P(( int, int ));
extern int mvupnxtwind P(( int, int ));
extern int mvrightwind P(( int, int ));
extern int mvleftwind P(( int, int ));
extern int onlywind P(( int, int ));
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
extern void winit P(( void ));
extern void varinit P(( void ));
extern int wrapword P(( int, int ));
extern int forwviword P(( int, int ));
extern int forwword P(( int, int ));
extern int forwviendw P(( int, int ));
extern int forwendw P(( int, int ));
extern int backviword P(( int, int ));
extern int backword P(( int, int ));
extern int join P(( int, int ));
extern int formatregion P(( void ));
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
extern char * token P(( char *, char * ));
extern int ffgetline P(( int * ));
extern int kinsert P(( int ));
extern char * fnc2engl P(( CMDFUNC * ));
extern CMDFUNC * engl2fnc P(( char * ));
extern CMDFUNC * kcod2fnc P(( int ));
extern int prc2kcod P(( char * ));
extern char * prc2engl P(( char * ));
extern int fnc2key P(( CMDFUNC * ));
extern int nextarg P(( char * ));
extern int echochar P(( int, int ));
extern int scanmore P(( char *, int ));
extern int expandp P(( char *, char *, int ));
extern int scanner P((regexp *, int, int ));
extern int insspace P(( int, int ));
extern int insbrace P(( int, int ));
extern int inspound P(( void ));
extern int fmatch P(( int ));
extern int getfence P(( int, int ));
extern int adjustmode P(( int, int ));
extern void putdotback P(( BUFFER *, LINE * ));

