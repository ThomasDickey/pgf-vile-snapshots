#
# makefile for vile.  
#
# Not much user-configuration is usually needed.  Various targets in the
# makefile support various systems -- if yours isn't here, and none of the
# one's provided will work, then edit estruct.h, and use "make default".
# For a list of the pre-defined targets, just type "make".
#
# The command/name/key/function bindings are defined in the file "cmdtbl". 
# The mktbls program parses this to produce nebind.h, nename.h, and
# nefunc.h, which are used by the rest of the build.
#
# The version number is found near the top of edef.h, and is displayed with
# the '*' and ":version" commands, or by invoking vile with "-V".
#
# Paul Fox
#
# original makefile for uemacs: Adam Fritz July 30,1987  (do you recognize it?)
#

# some old make's don't predefine this:
#MAKE=/bin/make
#MAKE=/usr/bin/make

# To change screen driver modules, change SCREEN and SCRDEF below, OR edit
# estruct.h to make sure the correct one is #defined as "1", and the others
# all as "0".  If you use tcap.c, you'll need libtermcap.a too.  If you use
# x11.c, you'll need libX11.a too.

# for regular use
SCREEN = tcap
LIBS = -ltermcap
TARGET = vile
SCRDEF = -DTERMCAP -Dscrn_chosen

# for building the X version
#SCREEN = x11
#LIBS = -lX11
#TARGET = xvile
#SCRDEF = -DX11 -Dscrn_chosen

CO=co
#CO="@echo co"

# for passing along the above settings to sub-makes
ENV = SCREEN="$(SCREEN)" LIBS="$(LIBS)" TARGET="$(TARGET)" SCRDEF="$(SCRDEF)" \
	CO="$(CO)" CC="$(CC)" LINK="$(LINK)" OPTFLAGS="$(OPTFLAGS)" \
	GINCS="$(GINCS)"


# install to DESTDIR1 if it's writable, else DESTDIR2
DESTDIR1 = /usr/local/bin
DESTDIR2 = $(HOME)/bin

# if you want the help file (vile.hlp) to go somewhere other than your $PATH
#  or one of the hard-code paths in epath.h  (it goes to the same place vile
#  does by default)
#HELP_LOC=/my/local/help/lib
 HELP_LOC=

REMOTE=gutso!foxharp

#CC = gcc
#OPTFLAGS = -g -Wall -Wshadow -O # -pg
#gincdir = /usr/local/lib/gcc-include
#GINCS = -I$(gincdir) -I$(gincdir)/sys

CC = cc
#OPTFLAGS = -g
OPTFLAGS = -O

LINK = $(CC)
LDFLAGS = 

# some older bsd systems keep ioctl in sys only -- easier to
# search both places than to ifdef the code.  color me lazy.
INCS = -I. $(GINCS) -I/usr/include -I/usr/include/sys

CFLAGS0 = $(INCS) $(SCRDEF) -DHELP_LOC=\\\"$(HELP_LOC)\\\"
CFLAGS1 = $(OPTFLAGS) $(CFLAGS0)

# suffix for object files.
# this get changes to "obj" for DOS builds
O = o

# All of the makefiles which should be preserved
MAKFILES = makefile make.ini
MKTBLS = ./mktbls

ALLTOOLS = $(MAKFILES)


# these are normal editable headers
HDRS = estruct.h epath.h evar.h edef.h proto.h

# these headers are built by the mktbls program from the information in cmdtbl
BUILTHDRS = nebind.h nefunc.h nename.h 

ALLHDRS = $(HDRS)

# All the C files which should be saved
#  (including tools, like mktbls.c, unused screen drivers, etc.)
CSRCac = ansi.c at386.c basic.c bind.c buffer.c crypt.c csrch.c
CSRCde = dg10.c display.c eval.c exec.c externs.c
CSRCfh = fences.c file.c fileio.c finderr.c globals.c hp110.c hp150.c
CSRCim = ibmpc.c input.c insert.c isearch.c line.c main.c modes.c mktbls.c
CSRCnr = npopen.c opers.c oneliner.c random.c regexp.c region.c
CSRCst = search.c spawn.c st520.c tags.c tcap.c termio.c tipc.c
CSRCuw = undo.c vmalloc.c vmsvt.c vt52.c window.c word.c wordmov.c
CSRCxz = x11.c z309.c z_ibmpc.c

CSRC = $(CSRCac) $(CSRCde) $(CSRCfh) $(CSRCim) $(CSRCnr) \
	$(CSRCst) $(CSRCuw) $(CSRCxz)

# non-C source code
OTHERSRC = z100bios.asm

# text and data files
TEXTFILES = README CHANGES cmdtbl vile.hlp buglist revlist \
	README.X11

ALLSRC = $(CSRC) $(OTHERSRC)

EVERYTHING = $(ALLTOOLS) $(ALLHDRS) $(ALLSRC) $(TEXTFILES) $(SHORTSTUFF)


SRC = main.c $(SCREEN).c basic.c bind.c buffer.c crypt.c \
	csrch.c display.c eval.c exec.c externs.c fences.c file.c \
	fileio.c finderr.c globals.c input.c insert.c isearch.c \
	line.c modes.c npopen.c oneliner.c opers.c random.c regexp.c \
	region.c search.c spawn.c tags.c termio.c undo.c \
	vmalloc.c window.c word.c wordmov.c

OBJ = main.$O $(SCREEN).$O basic.$O bind.$O buffer.$O crypt.$O \
	csrch.$O display.$O eval.$O exec.$O externs.$O fences.$O file.$O \
	fileio.$O finderr.$O globals.$O input.$O insert.$O isearch.$O \
	line.$O modes.$O npopen.$O oneliner.$O opers.$O random.$O regexp.$O \
	region.$O search.$O spawn.$O tags.$O termio.$O undo.$O \
	vmalloc.$O window.$O word.$O wordmov.$O


# if your pre-processor won't treat undefined macros as having value 0, or
#	won't give unvalue'd defines the value 1, you'll have to do your
#	config inside of estruct.h, and use "make default"
# otherwise, there are essentially four choices, unless your machine is
#	one of the more "specific" targets.  you can be either att or bsd, with
#	or without posix extensions.

# please report bugs with these config options

all:
	@echo "	there is no longer a default unnamed target"		;\
	echo "	please use one of the following:"			;\
	echo "	make bsd	(for pure, older BSD systems)"		;\
	echo "	make bsd_posix	(for BSD with some POSIX support)"	;\
	echo "	make bsd386"						;\
	echo "	make att	(traditional USG systems)"		;\
	echo "	make att_posix	(newer, with POSIX support)"		;\
	echo "	make svr3	(early 386 UNIX, for instance)"		;\
	echo "	make sunos	(sunos 3 or 4)"				;\
	echo "	make ultrix"						;\
	echo "	make mach	(just pure bsd)"			;\
	echo "	make svr4	(untested)"				;\
	echo "	make mips	(uses systemV stuff)"			;\
	echo "	make odt	(open desktop -- variant of svr3)"	;\
	echo "	make isc	(interactive -- another such variant)"	;\
	echo "	make hpux"						;\
	echo "	make next	(NeXT)"					;\
	echo "	make sony	(Sony News -- very BSD)"		;\
	echo "	make unixpc	(AT&T 3B1)"				;\
	echo "	make aix	(r6000)"				;\
	echo "	make osf1	(OSF/1)"				;\
	echo "	make linux	(ported to 0.95)"			;\
	echo "	make aux2	(A/UX 2.0) (3.0 is probably svr3)"	;\
	echo "	make apollo	(HP/Apollo SR10.3 CC 6.8)"		;\
	echo "	nmake msc	(MicroSoft C 6.0) (buggy?)"		;\
	echo "	make sx1100	(SX1100 running on Unisys 1100)"	;\
	echo "	make default	(to use config internal to estruct.h)"

bsd sony mach:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -Dos_chosen" \
	    MAKE=/usr/bin/make $(TARGET) $(ENV)

bsd_posix ultrix:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DPOSIX -Dos_chosen" \
		$(TARGET) $(ENV)

sunos:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DPOSIX -DSUNOS -Dos_chosen" \
		$(TARGET) $(ENV)

bsd386:
	make CFLAGS="$(CFLAGS1) -DBERK -DBSD386 -Dos_chosen" \
	    MAKE=/usr/bin/make $(TARGET) $(ENV)

att:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -Dos_chosen" \
		$(TARGET) $(ENV)

att_posix svr4:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DPOSIX -Dos_chosen" \
		$(TARGET) $(ENV)

svr3:
	$(MAKE) CFLAGS="$(CFLAGS1) -DSVR3 -Dos_chosen" \
		$(TARGET) $(ENV)

mips:
	$(MAKE) CFLAGS="$(CFLAGS1) -systype sysv $(INCS) -DSVR3 -Dos_chosen" \
		$(TARGET) $(ENV)

odt:
	$(MAKE) \
	CFLAGS="$(CFLAGS1) -DODT -DUSG -DPOSIX -DSVR3_PTEM -Dos_chosen" \
		$(TARGET) $(ENV)

isc:
	$(MAKE) \
	CFLAGS="$(CFLAGS1) -DISC -DUSG -DPOSIX -DSVR3_PTEM -Dos_chosen" \
		$(TARGET) $(ENV)

hpux:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DHAVE_SELECT -Dos_chosen" \
		$(TARGET) $(ENV)

next:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -D__STRICT_BSD__ \
		-Dos_chosen" $(TARGET) $(ENV)

unixpc:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DHAVE_SELECT \
		-DHAVE_MKDIR=0 -Dwinit=xxwinit -Dos_chosen -DUNIXPC" \
		$(TARGET) $(ENV)

aix:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG \
		-DPOSIX -DAIX -DHAVE_SELECT -U__STR__ -Dos_chosen -qpgmsize=l" \
		LIBS=-lcurses \
		$(TARGET) $(ENV)

apollo:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DAPOLLO -Dos_chosen" \
	    MAKE=/usr/bin/make $(TARGET) $(ENV)

osf1:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DPOSIX -DOSF1 -Dos_chosen" \
		$(TARGET) $(ENV)

linux:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG \
		-DPOSIX -DHAVE_SELECT -DHAVE_POLL=0 -DLINUX -Dos_chosen" \
		$(TARGET) $(ENV)

# It has been pointed out to me that the stock COMMAND.COM can't possibly
# handle the length of the commands that this causes.  You may have to
# create your own makefile for DOS.  I think it's only this top level
# re-invocation of make that's a problem -- everything else should fit in
# 128 characters.  I think.  (It works as-is under SoftPC, which is why
# I wrote it this way.)
msc:
	$(MAKE) MKTBLS=mktbls.exe CFLAGS="/qc /AL /nologo \
		-DMSDOS -DMSC -Dos_chosen -DIBMPC -Dscrn_chosen" \
		O=obj CC=cl SCREEN=ibmpc \
		LNKFILE=link.msc \
		LINK="link /MAP /CO /NOI /STACK:4096 @link.msc" $(TARGET)2

aux2:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DAUX2 -Dos_chosen" \
		$(TARGET) $(ENV)

sx1100:
	$(MAKE) CFLAGS="$(CFLAGS1) -DSVR3 -Dos_chosen" \
		LDFLAGS="-h 400000" \
		$(TARGET) $(ENV)

default:
	$(MAKE) CFLAGS="$(CFLAGS1) -Uos_chosen" \
		$(TARGET) $(ENV)

$(TARGET): $(BUILTHDRS) $(OBJ) makefile
	-mv $(TARGET) o$(TARGET)
	$(LINK) $(LDFLAGS) -o $(TARGET) $(OBJ) $(LIBS)
#	$(LINK) -pg -o $(TARGET) $(OBJ) $(LIBS) -lc_p
#	$(LINK) -Bstatic -o $(TARGET) $(OBJ) $(LIBS)

# this target is more convenient for the nonstandard environments, like DOS
$(TARGET)2: $(BUILTHDRS) $(OBJ) makefile
	-mv $(TARGET) o$(TARGET)
	$(LINK)

# this has to be run on a unix system, unfortunately.
#  once on dos, you'll have to maintain link.msc by hand
link.msc: makefile
	echo +>$@
	(echo $(OBJ) | xargs -n5 ) | \
		sed -e 's/$(SCREEN)/ibmpc/' \
		    -e 's/\.o/\.obj/g' \
		    -e 's/$$/+/' >> $@
	echo >> $@
	echo vile.exe >> $@
	echo vile.map\; >> $@

# end of file

saber_src:
	#load $(CFLAGS) $(SRC) $(LIBS)

saber_obj: $(OBJ)
	#load $(CFLAGS) $(OBJ) $(LIBS)

#.c.src:
#	#load $(CFLAGS) $<
#.o.obj:
#	#load $(CFLAGS) $<
#
#SUFFIXES: .c .h .o .src .obj


$(BUILTHDRS): cmdtbl $(MKTBLS)
	$(MKTBLS) cmdtbl

$(MKTBLS):  mktbls.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(MKTBLS)  mktbls.c

# install to DESTDIR1 if it's writable, else DESTDIR2
install:
	@[ -x $(TARGET) ] || (echo must make $(TARGET) first && exit 1)
	@[ -w $(DESTDIR1) ] && dest=$(DESTDIR1) || dest=$(DESTDIR2) ;\
	[ -f $$dest/$(TARGET) ] && mv $$dest/$(TARGET) $$dest/o$(TARGET) ;\
	echo Installing $(TARGET) to $$dest ; \
	cp $(TARGET) $$dest ;\
	test -f vile.hlp && /bin/rm -f $$dest/vile.hlp ;\
	[ "$(HELP_LOC)" ] && dest="$(HELP_LOC)" ; \
	echo Installing vile.hlp to $$dest ; \
	cp vile.hlp $$dest; \
	chmod 0644 $$dest/vile.hlp
	

compr-shar: link.msc
	[ -d cshardir ] || mkdir cshardir
#	add -a for archive headers, add -s pgf@cayman.com for submitted-by
	shar -p -nvile -L55 -o cshardir/vileshar \
		-T README -C `ls $(EVERYTHING) | sed /README/d` link.msc

shar: link.msc
	[ -d shardir ] || mkdir shardir
#	add -a for archive headers, add -s pgf@cayman.com for submitted-by
	shar -x -a -spgf@cayman.com -nVile -L55 \
			-o shardir/vileshar `ls $(EVERYTHING)` link.msc

bigshar: link.msc
	shar -spgf@cayman.com -nVile \
	-o vileBIGshar README `ls $(EVERYTHING) | sed /README/d` link.msc

# only uucp things changed since last time
uuto:
	uuto `ls -t $(EVERYTHING) uutodone | sed '/uutodone/q'` $(REMOTE)
	date >uutodone

uurw:
	uuto `make rw` $(REMOTE)

rcsdiffrw:
	@-for x in `$(MAKE) rw`	;\
	do	\
		echo 		;\
		echo $$x	;\
		echo =========	;\
		rcsdiff $$x	;\
	done 2>&1		;\
	echo			;\
	echo all done

floppy:
	ls $(EVERYTHING) | oo

# you don't want to know...
dosscript:
	(								\
	echo echo on							;\
	for x in `ls -t $(EVERYTHING) dosback.bat | sed '/dosback.bat/q'` ;\
	do								\
		echo copy u:$$x a:					;\
	done 								;\
	#echo quit							;\
	) >tmp.bat
	ud < tmp.bat >dosback.bat
	/bin/rm -f tmp.bat
	#-dos
	#>dosback.bat
	
newdosfloppy:
	touch 0101010170 dosback.bat

# dump a list of the important files
list:
	@ls $(EVERYTHING) | more

# make a list of RCS revisions.  don't include the revlist itself
nrevlist:
	$(MAKE) list  | egrep -v revlist >/tmp/vile__files
	rlog -Ntrunk `cat /tmp/vile__files` >/tmp/vile__revs
	-paste /tmp/vile__files /tmp/vile__revs >/tmp/vile_revlist
	rm /tmp/vile__files /tmp/vile__revs
	mv /tmp/vile_revlist nrevlist

nchanges: nrevlist
	revlistdiff nrevlist revlist >changes


# dump a list of files that may have changed since last backup
rw list-writeable:
	@ls -l $(EVERYTHING) | \
		egrep '^[^l].w' | \
		sed 's;.* ;;'   # strip to last space

no-write:
	chmod -w $(EVERYTHING)

update:
	nupdatefile.pl -r $(EVERYTHING)

protos:
	cextract -D__STDC__ +E +P +s +r -o nproto.h $(SRC)

tagfile:
	dotags $(SRC) $(HDRS)

lint:	$(SRC)
	#lint -hbvxac $(SRC) >lint.out 
	#lint $(CFLAGS0) $(ENV) $(SRC) >lint.out 
	#lint $(CFLAGS0) -DBERK -DPOSIX -DSUNOS -Dos_chosen  $(SRC) >lint.out 
	lint $(CFLAGS0) -DSVR3 -Dos_chosen  $(SRC) >lint.out 

cflow:	$(SRC)
	cflow  $(SRC) >cflow.out 

clean:
	rm -f *.$O o$(TARGET) $(BUILTHDRS) $(MKTBLS) core *~ *.BAK

clobber: clean
	rm -f $(TARGET)

print:
	pr makefile $(HDRS) $(SRC) | lpr

depend:	 $(SRC) $(HDRS) $(BUILTHDRS)
	mv -f makefile makefile.orig
	(sed -e '/^#DEPENDS/,$$d' makefile.orig ; \
		echo "#DEPENDS" ; \
		$(CC) -M $(CFLAGS) $? ) > makefile

populate: $(EVERYTHING)

$(EVERYTHING):
	$(CO) -r$(revision) $@


$(OBJ): estruct.h edef.h

externs.$O: nebind.h nename.h nefunc.h

# $Log: makefile,v $
# Revision 1.79  1992/12/29 23:17:28  foxharp
# commentary
#
# Revision 1.78  1992/12/28  23:51:57  foxharp
# test for presence of vile in destdir before trying to move it to ovile
#
# Revision 1.77  1992/12/16  21:38:34  foxharp
# rule for nchanges target
#
# Revision 1.76  1992/12/14  09:02:08  foxharp
# svr3 lint command
#
# Revision 1.75  1992/12/13  13:27:04  foxharp
# various changes, for easier command-line ENV setting, and for linting
#
# Revision 1.74  1992/12/04  09:50:24  foxharp
# pass SCREEN etc. to lower makes via ENV variable
#
# Revision 1.73  1992/12/04  09:23:08  foxharp
# added apollo
#
# Revision 1.72  1992/11/19  09:14:41  foxharp
# added HELP_LOC support -- allows easy choice of new dest for vile.hlp
#
# Revision 1.71  1992/08/28  08:55:08  foxharp
# switch from -g to -O for releaseh
#
# Revision 1.70  1992/08/20  23:40:48  foxharp
# typo fixes -- thanks, eric
#
# Revision 1.69  1992/08/20  08:58:05  foxharp
# final cleanup preparing to release version four
#
# Revision 1.68  1992/08/07  18:05:27  pgf
# fixed mips systype botch
#
# Revision 1.67  1992/08/04  20:12:00  foxharp
# split out sunos, so SUNOS gets defined, added comment about DOS command
# line lengths, and consolidated some vars into CFLAGS1
#
# Revision 1.66  1992/08/03  09:10:29  foxharp
# added eric krohn's changes for sx1100 -- introduced LDFLAGS
#
# Revision 1.65  1992/07/24  18:22:06  foxharp
# change name of MAKEFILES -- that name is special to GNU make
#
# Revision 1.64  1992/07/22  00:50:32  foxharp
# turn off shared libs
#
# Revision 1.63  1992/07/20  22:48:17  foxharp
# profiling support (commented out)
#
# Revision 1.62  1992/07/17  19:21:53  foxharp
# added gcc sys dir to find filio.h, and changed "sun" to "sunos" to make
# way for solaris
#
# Revision 1.61  1992/07/16  22:05:01  foxharp
# fixed linux rule -- missing -DLINUX
#
# Revision 1.60  1992/07/01  17:03:12  foxharp
# added z_ibmpc.c
#
# Revision 1.59  1992/06/25  23:00:50  foxharp
# changes for dos/ibmpc
#
# Revision 1.58  1992/06/22  08:40:02  foxharp
# added INCS to all targets, especially aix, so it'll get ioctl.h in termio.c
#
# Revision 1.57  1992/06/14  11:33:17  foxharp
# added -DAIX
#
# Revision 1.56  1992/06/12  20:21:28  foxharp
# use shared libs in svr3 -- should this be done for others?  isc? odt?
#
# Revision 1.55  1992/06/04  19:44:40  foxharp
# added mach and cflow targets
#
# Revision 1.54  1992/06/03  08:39:23  foxharp
# look explicitly for gcc include files
#
# Revision 1.53  1992/06/01  20:41:07  foxharp
# added dependency of externs.o on ne....h
#
# Revision 1.52  1992/05/29  09:40:53  foxharp
# split out modes.c, fences.c, and insert.c from random.c
#
# Revision 1.51  1992/05/29  08:40:05  foxharp
# minor ordering
#
# Revision 1.50  1992/05/27  19:16:22  foxharp
# fixed odt and isc targets -- added -DODT and -DISC
#
# Revision 1.49  1992/05/27  08:32:57  foxharp
# added rcsdiffrw target
#
# Revision 1.48  1992/05/20  18:58:11  foxharp
# comments on target help, a/ux support, and uurw
#
# Revision 1.47  1992/05/19  09:15:45  foxharp
# machine target fixups
#
# Revision 1.46  1992/05/16  12:00:31  pgf
# prototypes/ansi/void-int stuff/microsoftC
#
# Revision 1.45  1992/05/13  09:14:12  pgf
# comment changes
#
# Revision 1.44  1992/04/26  13:42:33  pgf
# added bsd386 target
#
# Revision 1.43  1992/04/14  08:55:38  pgf
# added support for OSF1 (affects termio only)
#
# Revision 1.42  1992/04/10  18:52:21  pgf
# add config targets for various platforms
#
# Revision 1.41  1992/04/06  09:43:55  pgf
# fixed nrevlist target
#
# Revision 1.40  1992/04/06  09:29:57  pgf
# added nrevlist target
#
# Revision 1.39  1992/04/06  09:21:40  pgf
# added revlist
#
# Revision 1.38  1992/04/02  23:01:10  pgf
# re-alphabetized SRC and OBJ
#
# Revision 1.37  1992/03/26  09:16:16  pgf
# added regexp.c dependencies
#
# Revision 1.36  1992/03/25  19:12:19  pgf
# explicit rule for building mktbls, to satisfy broken BSD make
#
# Revision 1.35  1992/03/19  23:36:42  pgf
# removed shorten stuff
#
# Revision 1.34  1992/03/07  10:25:58  pgf
# AIX support (needs -lcurses)
#
# Revision 1.33  1992/03/01  18:41:31  pgf
# moved target define
#
# Revision 1.32  1992/02/17  08:48:49  pgf
# keep copy of current executable during install
#
# Revision 1.31  1991/12/30  23:15:04  pgf
# rename the product of bigshar
#
# Revision 1.30  1991/11/27  10:17:21  pgf
# changes to dos backup target
#
# Revision 1.29  1991/11/16  18:35:08  pgf
# dropped the file locking files -- they didn't work, and were only
# compatible with other's running vile
#
# Revision 1.28  1991/11/13  20:09:27  pgf
# X11 changes, from dave lemke
#
# Revision 1.27  1991/11/07  02:00:32  pgf
# lint cleanup
#
# Revision 1.26  1991/11/01  14:56:51  pgf
# took tags file out of the distribution
#
# Revision 1.25  1991/11/01  14:20:24  pgf
# a little bit more saber support
#
# Revision 1.24  1991/10/28  14:19:46  pgf
# took out some old junk, moved the changelog down low
#
# Revision 1.23  1991/10/27  01:47:14  pgf
# switched from regex to regexp
#
# Revision 1.22  1991/10/24  13:03:48  pgf
# added regex.c
#
# Revision 1.21  1991/10/23  12:05:37  pgf
# added bigshar, and put ./ in front of mktbls rule -- there seems
# to be a bug in some makes that drops the ./ in the MKTBLS
# variable
#
# Revision 1.20  1991/10/22  14:36:01  pgf
# added the CHANGES file
#
# Revision 1.19  1991/10/10  12:20:26  pgf
# added vmalloc.o dependencies
#
# Revision 1.18  1991/09/27  02:49:30  pgf
# initial saber support
#
# Revision 1.17  1991/08/10  01:24:28  pgf
# added update target, and
# removed the BUILTHDRS list
#
# Revision 1.16  1991/08/07  11:56:58  pgf
# added RCS log entries
#
# revision 1.15
# date: 1991/08/07 02:10:53;
# removed './' from files in dependencies
# 
# revision 1.14
# date: 1991/08/06 16:19:55;
# added populate rule, and "revision" arg to co rule
# 
# revision 1.13
# date: 1991/08/06 16:09:42;
# fixed "rw" rule
# 
# revision 1.12
# date: 1991/08/06 14:45:14;
# commented out shortnames stuff, and
# added "co" rule
# 
# revision 1.11
# date: 1991/06/28 10:52:36;
# exclude built headers from come lists
# 
# revision 1.10
# date: 1991/06/16 17:29:07;
# fixed install rules, so I don't have to change it at home
# 
# revision 1.9
# date: 1991/06/04 16:00:40;
# switch to -L for shar'ing
# 
# revision 1.8
# date: 1991/06/04 13:12:25;
# cleanup for release of version three
# 
# revision 1.7
# date: 1991/04/08 13:11:17;
# added 'rw' target, and make some dos backup changes
# 
# revision 1.6
# date: 1991/04/04 09:38:12;
# new REMOTE address
# 
# revision 1.5
# date: 1990/12/06 18:53:14;
# fixed compr-shar entry, commented out remap.h target
# 
# revision 1.4
# date: 1990/12/03 12:01:37;
# comment change
# 
# revision 1.3
# date: 1990/10/03 16:09:30;
# shortened "shortnames" to "shorten" !
# 
# revision 1.2
# date: 1990/10/01 12:32:23;
# added shortnames directory
# 
# revision 1.1
# date: 1990/09/21 10:25:38;
# initial vile RCS revision
#
CPP_OPTS= $(CFLAGS0) -DBERK -DAPOLLO -Dos_chosen
.SUFFIXES: .i .i2 .lint
lint.out:	;tdlint $(CPP_OPTS) $(SRC) >$@
.c.i:		;$(CC)  $(CPP_OPTS) -C -E $< >$@
.c.i2:		;/usr/lib/cpp  $(CPP_OPTS) -C -E $< >$@
.c.lint:	;tdlint $(CPP_OPTS) $< >$@
