#
# makefile for vile.
#
# Not much user-configuration is usually needed.  Various targets in the
# makefile support various systems -- if yours isn't here, and none of the
# one's provided will work, then edit estruct.h, and use "make default".
# For a list of the pre-defined targets, just type "make".
#
# yes, there should be a Configure script.  no, i don't plan on writing one
# tomorrow.
#
# The command/name/key/function bindings are defined in the file "cmdtbl".
# The mktbls program parses this to produce nebind.h, nename.h, and
# nefunc.h, which are used by the rest of the build.
#
# Buffer/window modes are defined in the file "modetbl".
# The mktbls program parses this to produce nemode.h, which is included in
# 'estruct.h'.
#
# The version number is found near the top of edef.h, and is displayed with
# the ":version" command, or by invoking vile with "-V".
#
# Paul Fox
#
# original makefile for uemacs: Adam Fritz July 30,1987  (do you recognize it?)
#

# To change screen driver modules, change SCREEN and SCRDEF below, OR edit
# estruct.h to make sure the correct one is #defined as "1", and the others
# all as "0".  
#  NOTE! if you switch from "vile" to "xvile" or back, you _must_ do
#  a "make clean" in between.

# Regular vile, use these four lines:
SCREEN = tcap
LIBS = -ltermcap
TARGET = vile
SCRDEF = -DTERMCAP -Dscrn_chosen

# Five xvile options:
# These options build differently depending on whether you have the
# Athena widget library, the Motif widget library, the OpenLook widget library,
# just the X toolkit library, or none of these (just the X11 library).
#
# In each case the LIBS line is just a suggested starting point.  You may be
# able to get by without -lXext on some platforms. Others will require the
# use of the math library (-lm).  One platform (a Sun) required that the 
# Xmu library be linked statically rather than dynamically.  If you use
# GCC, this may be accomplished by substituting "-static -lXmu -dynamic" in
# place of "-lXmu".  If you are using Sun's C compiler, this may be accomplished
# with "-Bstatic -lXmu -Bdynamic" instead.
#
# Your platform may well have its own idiosyncracies which will require other
# libraries.

# 1) for building the X toolkit version of xvile with the Athena widget set,
# use these definitions.
#SCREEN = x11
#LIBS = -lXaw -lXmu -lXext -lXt -lX11
#TARGET = xvile
#SCRDEF = -DX11 -Dscrn_chosen -DXTOOLKIT -DATHENA_WIDGETS

# 2) for building the X toolkit version of xvile with the Motif widget set,
# use these definitions.  Not all systems have nor require -lgen.  Take it
# out if your platform can't find it. 
#SCREEN = x11
#LIBS = -lXm -lXmu -lXext -lXt -lX11 -lgen
#TARGET = xvile
#SCRDEF = -DX11 -Dscrn_chosen -DXTOOLKIT -DMOTIF_WIDGETS

# 3) if you want to build with the OpenLook widget library, you should use
# the following lines.  Note the use of the OPENWINHOME environment variable.
# If you are running on a Sun, you may well have this set without even knowing
# it.  If you don't, you will have to figure out where OPENWINHOME is and
# set it appropriately.  It is often in /usr/openwin.
#SCREEN = x11
#LIBS = -L${OPENWINHOME}/lib -lXol -lXmu -lXt -lX11
#TARGET = xvile
#SCRDEF = -DX11 -Dscrn_chosen -DXTOOLKIT -DOL_WIDGETS -I${OPENWINHOME}/include

# 4) if you don't have any of the above widget sets (not very probable), but
# have the X toolkit (libXt.a), use these lines.  The biggest loss in this case
# is the absence of scrollbars.
#SCREEN = x11
#LIBS = -lXt -lX11
#TARGET = xvile
#SCRDEF = -DX11 -Dscrn_chosen -DXTOOLKIT -DNO_WIDGETS

# 5) for building the simple X version (if you don't have libXaw.a _or_
# libXt.a) use these lines.  You won't get scrollbars, and some other
# things.  Visually, this looks much the same as the one above.
#SCREEN = x11simp
#LIBS = -lX11
#TARGET = xvile
#SCRDEF = -DX11 -Dscrn_chosen

# "make install" will install to DESTDIR1 if it's writable, else to DESTDIR2
DESTDIR1 = /usr/local/bin
DESTDIR2 = $(HOME)/bin

# normally, the help file should go somewhere along your PATH, or in one
#  of the predefined directories mentioned in epaths.h.
# if you want the help file (vile.hlp) to go somewhere else, specify the
#  location as "HELP_LOC".  (note that you may have to hardcode this in the
#  code itself if the CFLAGS definition below doesn't get through your make.
#  there are too many double quotes for some.
#HELP_LOC=/my/local/help/lib
 HELP_LOC=

# this is for my uucp convenience targets -- i wouldn't suggest anyone else
#  try them...
REMOTE=foxharp!pgf

# choose a compiler you like.  anything should work.
#CC = gcc
#OPTFLAGS = -g -O -Wall -Wshadow

CC = cc
#OPTFLAGS = -g
OPTFLAGS = -O

# CenterLine:
# Use -Xt option to compile quasi-K&R code with some ANSI semantics
#CC = clcc 
#OPTFLAGS = -O -Xt

LINK = $(CC)
LDFLAGS =

INCS =

CO=co
#CO="@echo co"

# some old make's don't predefine this:
#MAKE=/bin/make
#MAKE=/usr/bin/make

SHELL = /bin/sh

# for passing along the above settings to sub-makes
ENVIR = SCREEN="$(SCREEN)" LIBS="$(LIBS)" TARGET="$(TARGET)" \
	SCRDEF="$(SCRDEF)" \
	CO="$(CO)" CC="$(CC)" LINK="$(LINK)" OPTFLAGS="$(OPTFLAGS)"

# the backslashes around HELP_LOC don't make it through all makes.  if
#  you have trouble, use the following line instead, and edit epath.h to
#  hardcode your HELP_LOC path if you need one.
# CFLAGS0 = $(INCS) $(SCRDEF)
CFLAGS0 = $(INCS) $(SCRDEF) -DHELP_LOC=\\\"$(HELP_LOC)\\\"
CFLAGS1 = $(OPTFLAGS) $(CFLAGS0)

# suffix for object files.
# this get changes to "obj" for DOS builds
O = o

# All of the makefiles which should be preserved and distributed
UNIXMAK = makefile 				# on UNIX
VMSMAK = descrip.mms vms_link.opt		# on VMS
TURBOMAK = makefile.tbc				# on DOS, using TURBO
WATMAK = makefile.wat				# on DOS, using Watcom C/386
MSCMAK =	# still waiting for this one	# on DOS, using Microsoft C
DJGPPMAK = makefile.djg				# on DOS, DJGCC v1.09
MAKFILES = $(UNIXMAK) $(VMSMAK) $(TURBOMAK) $(WATMAK) $(DJGPPMAK) $(MSCMAK)

MKTBLS = ./mktbls

ALLTOOLS = $(MAKFILES)


# these are normal editable headers
HDRS = estruct.h epath.h edef.h proto.h dirstuff.h

# these headers are built by the mktbls program from the information in cmdtbl
# and in modetbl
BUILTHDRS = nebind.h nefunc.h nemode.h nename.h nevars.h

ALLHDRS = $(HDRS)

# All the C files which should be saved
#  (including tools, like mktbls.c, unused screen drivers, etc.)
CSRCac = ansi.c at386.c basic.c bind.c buffer.c crypt.c csrch.c
CSRCde = dg10.c display.c djhandl.c eval.c exec.c externs.c
CSRCf = fences.c file.c filec.c fileio.c finderr.c
CSRCgh = glob.c globals.c history.c hp110.c hp150.c
CSRCil = ibmpc.c input.c insert.c isearch.c line.c
CSRCm = main.c map.c modes.c mktbls.c
CSRCnr = npopen.c opers.c oneliner.c path.c random.c regexp.c region.c
CSRCst = search.c spawn.c st520.c tags.c tbuff.c tcap.c termio.c tipc.c tmp.c
CSRCuv = undo.c version.c vmalloc.c vms2unix.c vmspipe.c vmsvt.c vt52.c
CSRCw = window.c word.c wordmov.c
CSRCxz = x11.c x11simp.c z309.c z_ibmpc.c

CSRC = $(CSRCac) $(CSRCde) $(CSRCf) $(CSRCgh) $(CSRCil) $(CSRCm) $(CSRCnr) \
	$(CSRCst) $(CSRCuv) $(CSRCw) $(CSRCxz)

# non-C source code
OTHERSRC = z100bios.asm

# text and data files
TEXTFILES = README CHANGES CHANGES.R3 \
	cmdtbl modetbl vile.hlp vile.1 buglist revlist \
	README.X11 README.PC

ALLSRC = $(CSRC) $(OTHERSRC)

EVERYTHING = $(ALLTOOLS) $(ALLHDRS) $(ALLSRC) $(TEXTFILES) $(SHORTSTUFF)


SRC = main.c $(SCREEN).c basic.c bind.c buffer.c crypt.c \
	csrch.c display.c eval.c exec.c externs.c \
	fences.c file.c filec.c \
	fileio.c finderr.c glob.c globals.c history.c \
	input.c insert.c isearch.c \
	line.c map.c modes.c npopen.c oneliner.c \
	opers.c path.c random.c regexp.c \
	region.c search.c spawn.c tags.c tbuff.c \
	termio.c tmp.c undo.c \
	version.c vmalloc.c window.c word.c wordmov.c

OBJ = main.$O $(SCREEN).$O basic.$O bind.$O buffer.$O crypt.$O \
	csrch.$O display.$O eval.$O exec.$O externs.$O \
	fences.$O file.$O filec.$O \
	fileio.$O finderr.$O glob.$O globals.$O history.$O \
	input.$O insert.$O isearch.$O \
	line.$O map.$O modes.$O npopen.$O oneliner.$O \
	opers.$O path.$O random.$O regexp.$O \
	region.$O search.$O spawn.$O tags.$O tbuff.$O \
	termio.$O tmp.$O undo.$O \
	version.$O vmalloc.$O window.$O word.$O wordmov.$O


# if your pre-processor won't treat undefined macros as having value 0, or
#	won't give unvalue'd defines the value 1, you'll have to do your
#	config inside of estruct.h, and use "make default"
# otherwise, there are essentially four choices, unless your machine is
#	one of the more "specific" targets.  you can be either att or bsd, with
#	or without posix extensions.

# please report bugs with these config options

all:
	@echo "	there is no default unnamed target."			;\
	echo "	please use one of the following:"			;\
	echo "	make bsd	(for pure, older BSD systems)"		;\
	echo "	make bsd_posix	(for BSD with some POSIX support)"	;\
	echo "	make netbsd	(NetBSD 0.9, FreeBSD 1.0 and 386BSD 0.1)";\
	echo "	make att	(traditional USG systems)"		;\
	echo "	make att_posix	(newer, with POSIX support)"		;\
	echo "	make sgi	(Silicon Graphics)"			;\
	echo "	make svr3	(early 386 UNIX, for instance)"		;\
	echo "	make sunos	(sunos 3 or 4)"				;\
	echo "	make ultrix"						;\
	echo "	make mach	(just pure bsd)"			;\
	echo "	make svr4	(Solaris 2.1, 2.2, 2.3)"		;\
	echo "	make mips	(old? (UMIPS 4.52?) uses systemV stuff)";\
	echo "	make odt	(SCO Open DeskTop -- variant of svr3)"	;\
	echo "	make isc	(interactive -- another such variant)"	;\
	echo "	make hpux"						;\
	echo "	make next	(NeXT)"					;\
	echo "	make sony	(Sony News -- very BSD)"		;\
	echo "	make unixpc	(AT&T 3B1)"				;\
	echo "	make uts	(Amdahl UTS 2.1.5)"			;\
	echo "	make aix	(but probably only rs6000)"		;\
	echo "	make osf1	(OSF/1)"				;\
	echo "	make linux	(ported to 0.95)"			;\
	echo "	make aux2	(A/UX 2.0) (3.0 is probably svr3)"	;\
	echo "	make apollo	(HP/Apollo SR10.3 CC 6.8)"		;\
	echo "	nmake msc	(MicroSoft C 6.0) (buggy?)"		;\
	echo "	make sx1100	(SX1100 running on Unisys 1100)"	;\
	echo "	make delta88r3	(Motorola Delta 88, SVR3)"		;\
	echo "	make delta88r4	(Motorola Delta 88, SVR4)"		;\
	echo "	make default	(to use config internal to estruct.h)"

bsd sony mach:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -Dos_chosen" \
	    MAKE=/usr/bin/make $(TARGET) $(ENVIR)

bsd_posix ultrix:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DPOSIX -DULTRIX -Dos_chosen" \
		$(TARGET) $(ENVIR)

sunos:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DPOSIX -DSUNOS -Dos_chosen" \
		$(TARGET) $(ENVIR)

net386:
	make CFLAGS="$(CFLAGS1) -DBERK -DNETBSD -Dos_chosen" \
	    MAKE=/usr/bin/make $(TARGET) $(ENVIR)

att:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -Dos_chosen" \
		$(TARGET) $(ENVIR)

att_posix svr4:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DPOSIX -Dos_chosen" \
		$(TARGET) $(ENVIR)

# these would be the same as above, but some system headers need
# the MOTOROLA, SYSV, and SVR4 definitaions.
delta88r3:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DPOSIX -DMOTOROLA -DSYSV -Dos_chosen" \
		$(TARGET) $(ENVIR)

delta88r4:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DPOSIX -DMOTOROLA -DSVR4 -DSYSV -Dos_chosen" \
		$(TARGET) $(ENVIR)

sgi:
	@echo "Use 'make att_posix' after adding '-D__STDC__' to OPTFLAGS"
	@echo "You can also try adding '-cckr' to OPTFLAGS"
	@echo "Would someone who gets it to work please send me detailed info?"

svr3:
	$(MAKE) CFLAGS="$(CFLAGS1) -DSVR3 -Dos_chosen" \
		$(TARGET) $(ENVIR)

mips:
	$(MAKE) CFLAGS="$(CFLAGS1) -systype sysv $(INCS) -DSVR3 -DMIPS \
		-Dos_chosen" \
		$(TARGET) $(ENVIR)

odt:
	$(MAKE) \
	CFLAGS="$(CFLAGS1) -DODT -DUSG -DPOSIX -DSVR3_PTEM -Dos_chosen" \
		$(TARGET) $(ENVIR)

isc:
	$(MAKE) \
	CFLAGS="$(CFLAGS1) -DISC -DUSG -DPOSIX -DSVR3_PTEM -Dos_chosen" \
		$(TARGET) $(ENVIR)

hpux:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DPOSIX -DHAVE_SELECT -DHPUX -Dos_chosen" \
		$(TARGET) $(ENVIR)

next:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DNeXT -D__STRICT_BSD__ -Dos_chosen \
		-I/NextDeveloper/Headers -I/NextDeveloper/Headers/bsd \
		-I/NextDeveloper/Headers/bsd/sys" \
		$(TARGET) $(ENVIR) MKTBLS="./$(MKTBLS)"

unixpc:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG -DHAVE_SELECT \
		-DHAVE_MKDIR=0 -Dwinit=xxwinit -Dos_chosen -DUNIXPC" \
		$(TARGET) $(ENVIR)

uts:		# use -eft to allow editing terabyte files.  :-)
	$(MAKE) CFLAGS='$(CFLAGS1) -DUSG -eft -Dos_chosen' \
		$(TARGET) $(ENVIR)

# i've had reports that the curses lib is broken under AIX.  on the other
#  hand, the termcap lib doesn't appear in all versions of AIX, and when it
#  does, it limits you to an 80x24 window.  since I don't have any AIX machine
#  to try this on, i'm afraid you may have to experiment
# in addition, since I didn't know that aix varies so much from platform to
#  platform, i used "AIX" as the ifdef in the code.  it may not work for
#  all the other platforms.  sorry.
aix:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG \
		-DPOSIX -DAIX -DHAVE_SELECT -U__STR__ -Dos_chosen -qpgmsize=l" \
		LIBS=-lcurses \
		$(TARGET) $(ENVIR)

apollo:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DAPOLLO -Dos_chosen" \
	    MAKE=/usr/bin/make $(TARGET) $(ENVIR)

osf1:
	$(MAKE) CFLAGS="$(CFLAGS1) -DBERK -DPOSIX -DOSF1 -Dos_chosen" \
		$(TARGET) $(ENVIR)

linux:
	$(MAKE) CFLAGS="$(CFLAGS1) -DUSG \
		-DPOSIX -DHAVE_SELECT -DHAVE_POLL=0 -DLINUX -Dos_chosen" \
		$(TARGET) $(ENVIR)

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
		$(TARGET) $(ENVIR)

sx1100:
	$(MAKE) CFLAGS="$(CFLAGS1) -DSVR3 -Dos_chosen" \
		LDFLAGS="-h 400000" \
		$(TARGET) $(ENVIR)

default:
	$(MAKE) CFLAGS="$(CFLAGS1) -Uos_chosen" \
		$(TARGET) $(ENVIR)

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
	@echo +>$@
	@(echo $(OBJ) | xargs -n5 ) | \
		sed -e 's/$(SCREEN)/ibmpc/' \
		    -e 's/\.o/\.obj/g' \
		    -e 's/$$/+/' >> $@
	@echo >> $@
	@echo vile.exe >> $@
	@echo vile.map\; >> $@

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


nebind.h \
nefunc.h \
nename.h :	cmdtbl $(MKTBLS)
	$(MKTBLS) cmdtbl

nevars.h \
nemode.h:	modetbl $(MKTBLS)
	$(MKTBLS) modetbl

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

# only install to DESTDIR2
install2:
	@[ -x $(TARGET) ] || (echo must make $(TARGET) first && exit 1)
	@dest=$(DESTDIR2) ;\
	[ -f $$dest/$(TARGET) ] && mv $$dest/$(TARGET) $$dest/o$(TARGET) ;\
	echo Installing $(TARGET) to $$dest ; \
	cp $(TARGET) $$dest ;\
	test -f vile.hlp && /bin/rm -f $$dest/vile.hlp ;\
	[ "$(HELP_LOC)" ] && dest="$(HELP_LOC)" ; \
	echo Installing vile.hlp to $$dest ; \
	cp vile.hlp $$dest; \
	chmod 0644 $$dest/vile.hlp


compr-shar: link.msc /tmp/vilevers
	[ -d cshardir ] || mkdir cshardir
#	add -a for archive headers, add -s pgf@cayman.com for submitted-by
	vilevers=`cat /tmp/vilevers`; \
	shar -p -nvile$${vilevers} -L55 -o cshardir/vileshar \
		-T README -C `ls $(EVERYTHING) | sed '/^README$$/d'` link.msc

shar: link.msc /tmp/vilevers
	[ -d shardir ] || mkdir shardir
#	add -a for archive headers, add -s pgf@cayman.com for submitted-by
	vilevers=`cat /tmp/vilevers`; \
	shar -x -a -spgf@cayman.com -nvile$${vilevers} -L55 \
			-o shardir/vileshar `ls $(EVERYTHING)` link.msc

bigshar: link.msc /tmp/vilevers
	@echo 'Do you need to rebuild the revlist????'
	@sleep 3
	vilevers=`cat /tmp/vilevers`; \
	shar -spgf@cayman.com -nvile$${vilevers} \
	    -o vile$${vilevers}shar README `ls $(EVERYTHING) | \
	    sed '/^README$$/d'` link.msc ; \
	mv vile$${vilevers}shar.01 vile$${vilevers}shar	; \
	echo Created vile$${vilevers}shar

vanillashar: link.msc
	shar -V \
	    -o vileshar README `ls $(EVERYTHING) | \
	    sed '/^README$$/d'` link.msc ; \
	mv vileshar.01 vileshar		; \
	echo Created vileshar

zipfile: /tmp/vilevers
	vilevers=`cat /tmp/vilevers | sed 's/\.//'`; \
	zip -k vile$${vilevers}.zip $(EVERYTHING) ;\
	echo Created vile$${vilevers}.zip

patch:	link.msc /tmp/vilevers
	@orevlistrev=`rlog -h revlist | egrep head: | cut -f2 -d'.'`	;\
	orevlistrev=1.`expr $$orevlistrev - 1`				;\
	ovilevers=`cat /tmp/vilevers | cut -f2 -d'.'`			;\
	ovilemajor=`cat /tmp/vilevers | cut -f1 -d'.'`			;\
	ovilevers=$$ovilemajor.`expr $$ovilevers - 1`			;\
	echo Previous version is $$ovilevers				;\
	vilevers=`cat /tmp/vilevers`					;\
	co -p$$orevlistrev revlist |					 \
	while read file filerev						;\
	do								 \
	  rcsdiff -c -r$$filerev $$file 2>/dev/null || true		;\
	done  >patch$$ovilevers-$$vilevers 				;\
	echo Created patch$$ovilevers-$$vilevers 

/tmp/vilevers: ALWAYS
	@expr "`egrep 'version\[\].*' edef.h`" : \
		'.* \([0-9][0-9]*\.[0-9].*\)".*' >/tmp/vilevers
	@echo Current version is `cat /tmp/vilevers`

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

TAGS tagfile:
	dotags $(SRC) $(HDRS)

lint:	$(SRC)
	#lint -hbvxac $(SRC) >lint.out
	#lint $(CFLAGS0) $(ENVIR) $(SRC) >lint.out
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


$(OBJ): estruct.h nemode.h edef.h proto.h

ALWAYS:

main.$O:	nevars.h
bind.$O:	epath.h
filec.$O:	dirstuff.h
eval.$O:	nevars.h
glob.$O:	dirstuff.h
externs.$O:	nebind.h nename.h nefunc.h
path.$O:	dirstuff.h
vmalloc$O:	nevars.h

# $Log: makefile,v $
# Revision 1.136  1994/02/22 17:46:08  pgf
# added kev's motif and openlook support
#
# Revision 1.135  1994/02/22  11:10:04  pgf
# truncated RCS log for 4.0
#
#

# (dickey's rules)
CPP_OPTS= $(CFLAGS0) -DBERK -DAPOLLO -Dos_chosen
#CPP_OPTS= $(CFLAGS0) -DBERK -DSUNOS -DPOSIX -Dos_chosen
.SUFFIXES: .i .i2 .lint

lintlib::	llib-lVi1.ln
llib-lVi1.ln:	llib-lVi1 $(ALLHDRS)	; makellib $(CPP_OPTS) llib-lVi1 Vi1
llib-lVi1:	$(BUILTHDRS) $(SRC)	; cproto -l $(CPP_OPTS) `find $(SRC) -print |grep '^[a-k]'` >$@

lintlib::	llib-lVi2.ln
llib-lVi2.ln:	llib-lVi2 $(ALLHDRS)	; makellib $(CPP_OPTS) llib-lVi2 Vi2
llib-lVi2:	$(BUILTHDRS) $(SRC)	; cproto -l $(CPP_OPTS) `find $(SRC) -print |grep '^[l-z]'` >$@

lint.out:	$(BUILTHDRS) ;tdlint $(CPP_OPTS) $(LIBS) $(SRC) >$@

.c.i:		;$(CC)  $(CPP_OPTS) -C -E $< >$@
.c.i2:		;/usr/lib/cpp  $(CPP_OPTS) -C $< >$@
.c.lint:	;tdlint -lVi1 -lVi2 $(LIBS) $(CPP_OPTS) $< >$@

