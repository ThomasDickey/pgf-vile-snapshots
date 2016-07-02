#
# VMS makefile for vile.  Requires "MMS"
#
# Tested with:
#	VMS system version 5.4-2
#	MMS version 2.6
#	VAX-C version 3.2
#
# To change screen driver modules, change SCREEN and SCRDEF below, OR edit
# estruct.h to make sure the correct one is #defined as "1", and the others
# all as "0".  If you use tcap.c, you'll need libtermcap.a too.  If you use
# x11.c, you'll need libX11.a too.
#
# $Header: /usr/build/VCS/pgf-vile/RCS/descrip.mms,v 1.27 1995/08/18 12:32:58 pgf Exp $

# for regular vile, use these:
SCREEN = vmsvt
LIBS =
TARGET = vile.exe
SCRDEF = "DISP_VMSVT","scrn_chosen"

# for building the X version, xvile, use these:
#SCREEN = x11simp
#LIBS = #-lX11
#TARGET = xvile.exe
#SCRDEF = "DISP_X11","scrn_chosen"

# for building the X-toolkit version:
#SCREEN = x11
#LIBS = #-lX11
#TARGET = xvile.exe
#SCRDEF = "NO_WIDGETS","XTOOLKIT","DISP_X11","scrn_chosen"

# for building the Motif version (untested):
#SCREEN = x11
#LIBS = #-lX11
#TARGET = xvile.exe
#SCRDEF = "MOTIF_WIDGETS","XTOOLKIT","DISP_X11","scrn_chosen"

# if you want the help file (vile.hlp) to go somewhere other than your $PATH
#  or one of the hard-code paths in epath.h  (it goes to the same place vile
#  does by default)
HELP_LOC=

LINKFLAGS = /MAP=$(MMS$TARGET_NAME)/CROSS_REFERENCE/EXEC=$(MMS$TARGET_NAME).EXE

INCS = []

MKTBLS = mktbls.EXE


SRC =	main.c \
	$(SCREEN).c \
	basic.c \
	bind.c \
	buffer.c \
	crypt.c \
	csrch.c \
	display.c \
	dumbterm.c \
	eval.c \
	exec.c \
	externs.c \
	fences.c \
	file.c \
	filec.c \
	fileio.c \
	finderr.c \
	glob.c \
	globals.c \
	history.c \
	input.c \
	insert.c \
	itbuff.c \
	isearch.c \
	line.c \
	map.c \
	modes.c \
	msgs.c \
	npopen.c \
	oneliner.c \
	opers.c \
	path.c \
	random.c \
	regexp.c \
	region.c \
	search.c \
	select.c \
	spawn.c \
	tags.c \
	tbuff.c \
	termio.c \
	tmp.c \
	undo.c \
	version.c \
	vms2unix.c \
	vmspipe.c \
	window.c \
	word.c \
	wordmov.c

OBJ =	main.obj,\
	$(SCREEN).obj,\
	basic.obj,\
	bind.obj,\
	buffer.obj,\
	crypt.obj,\
	csrch.obj,\
	display.obj,\
	dumbterm.obj,\
	eval.obj,\
	exec.obj,\
	externs.obj,\
	fences.obj,\
	file.obj,\
	filec.obj,\
	fileio.obj,\
	finderr.obj,\
	glob.obj, \
	globals.obj,\
	history.obj,\
	input.obj,\
	insert.obj,\
	itbuff.obj,\
	isearch.obj,\
	line.obj,\
	map.obj, \
	modes.obj,\
	msgs.obj,\
	npopen.obj,\
	oneliner.obj,\
	opers.obj,\
	path.obj,\
	random.obj,\
	regexp.obj,\
	region.obj,\
	search.obj,\
	select.obj,\
	spawn.obj,\
	tags.obj,\
	tbuff.obj,\
	termio.obj,\
	tmp.obj,\
	undo.obj,\
	version.obj, \
	vms2unix.obj,\
	vmspipe.obj,\
	window.obj,\
	word.obj,\
	wordmov.obj

all :
        @ decc = f$search("SYS$SYSTEM:DECC$COMPILER.EXE").nes.""
        @ axp = f$getsyi("HW_MODEL").ge.1024
        @ macro = ""
        @ if axp.or.decc then macro = "/MACRO=("
        @ if decc then macro = macro + "__DECC__=1,"
        @ if axp then macro = macro + "__ALPHA__=1,"
        @ if macro.nes."" then macro = f$extract(0,f$length(macro)-1,macro)+ ")"
        $(MMS)$(MMSQUALIFIERS)'macro' $(TARGET)

#
# I've built on an Alpha with CC_OPTIONS set to
#	CC_OPTIONS = /STANDARD=VAXC		(for VAX-C compatibility)
#	CC_OPTIONS = /PREFIX_LIBRARY_ENTRIES=ALL_ENTRIES	(DEC-C)
# The latter (DEC-C) gives better type-checking -- T.Dickey
#
.IFDEF __ALPHA__
CC_OPTIONS = /PREFIX_LIBRARY_ENTRIES=ALL_ENTRIES
CC_DEFS = ,HAVE_ALARM
OPTFILE =
OPTIONS =
.ELSE
.IFDEF __DECC__
CC_OPTIONS = /STANDARD=VAXC
CC_DEFS = ,HAVE_ALARM,
.ELSE
CC_OPTIONS =
CC_DEFS = ,HAVE_SYS_ERRLIST
.ENDIF
OPTFILE = ,vmsshare.opt
OPTIONS = $(OPTFILE)/OPTIONS
.ENDIF

nebind.h \
nefkeys.h \
nefunc.h \
nename.h :	cmdtbl $(MKTBLS)
	MKTBLS cmdtbl

nevars.h \
nemode.h :	modetbl $(MKTBLS)
	MKTBLS modetbl

# install to DESTDIR1 if it's writable, else DESTDIR2
install :
	@ WRITE SYS$ERROR "** no rule for $@"
	
clean :
	@- if f$search("*.obj") .nes. "" then delete *.obj;*
	@- if f$search("*.bak") .nes. "" then delete *.bak;*
	@- if f$search("*.lis") .nes. "" then delete *.lis;*
	@- if f$search("*.log") .nes. "" then delete *.log;*
	@- if f$search("*.map") .nes. "" then delete *.map;*
	@- if f$search("ne*.h") .nes. "" then delete ne*.h;*
	@- if f$search("$(MKTBLS)") .nes. "" then delete $(MKTBLS);

clobber : clean
	@- if f$search("vile.com") .nes. "" then delete vile.com;*
	@- if f$search("xvile.com") .nes. "" then delete xvile.com;*
	@- if f$search("*.exe") .nes. "" then delete *.exe;*

$(OBJ) : estruct.h nemode.h nefkeys.h edef.h proto.h

main.obj :	nevars.h
bind.obj :	epath.h
filec.obj :	dirstuff.h
eval.obj :	nevars.h
glob.obj :	dirstuff.h
externs.obj :	nebind.h nename.h nefunc.h
vms2unix.obj :	dirstuff.h
version.obj :	patchlev.h

.first :
	@ define/nolog SYS SYS$LIBRARY		! fix includes to <sys/...>
	@ MKTBLS :== $SYS$DISK:'F$DIRECTORY()$(MKTBLS)	! make a foreign command

.last :
	@- if f$search("*.dia") .nes. "" then delete *.dia;*
	@- if f$search("*.lis") .nes. "" then purge *.lis
	@- if f$search("*.obj") .nes. "" then purge *.obj
	@- if f$search("*.map") .nes. "" then purge *.map
	@- if f$search("*.exe") .nes. "" then purge *.exe
	@- if f$search("*.log") .nes. "" then purge *.log

# used /G_FLOAT with vaxcrtlg/share in vms_link.opt
# can also use /Debug /Listing, /Show=All
CFLAGS =-
	/Diagnostics /Define=("os_chosen",$(SCRDEF)$(CC_DEFS)) -
	/Object=$@ /Include=($(INCS)) $(CC_OPTIONS)

.C.OBJ :
	$(CC) $(CFLAGS) $(MMS$SOURCE)
	@- delete $(MMS$TARGET_NAME).dia;*

$(MKTBLS) : mktbls.obj $(OPTFILE)
	$(LINK) $(LINKFLAGS) mktbls.obj $(OPTIONS)

$(TARGET) : $(OBJ), vms_link.opt, descrip.mms $(OPTFILE)
	$(LINK) $(LINKFLAGS) main.obj, $(SCREEN).obj, vms_link/opt $(OPTIONS)

# Runs VILE from the current directory (used for testing)
vile.com :
	@- if "''f$search("$@")'" .nes. "" then delete $@;*
	@- copy nl: $@
	@ open/append  test_script $@
	@ write test_script "$ temp = f$environment(""procedure"")"
	@ write test_script "$ temp = temp -"
	@ write test_script "		- f$parse(temp,,,""version"",""syntax_only"") -"
	@ write test_script "		- f$parse(temp,,,""type"",""syntax_only"")"
	@ write test_script "$ vile :== $ 'temp'.exe"
	@ write test_script "$ define/user_mode sys$input  sys$command"
	@ write test_script "$ define/user_mode sys$output sys$command"
	@ write test_script "$ vile 'p1 'p2 'p3 'p4 'p5 'p6 'p7 'p8"
	@ close test_script
	@ write sys$output "** made $@"

# Runs XVILE from the current directory (used for testing)
xvile.com :
	@- if "''f$search("$@")'" .nes. "" then delete $@;*
	@- copy nl: $@
	@ open/append  test_script $@
	@ write test_script "$ temp = f$environment(""procedure"")"
	@ write test_script "$ temp = temp -"
	@ write test_script "		- f$parse(temp,,,""name"",""syntax_only"") -"
	@ write test_script "		- f$parse(temp,,,""version"",""syntax_only"") -"
	@ write test_script "		- f$parse(temp,,,""type"",""syntax_only"")"
	@ write test_script "$ xvile :== $ 'temp'xvile.exe"
	@ write test_script "$ define/user_mode sys$input  sys$command"
	@ write test_script "$ define/user_mode sys$output sys$command"
	@ write test_script "$ xvile 'p1 'p2 'p3 'p4 'p5 'p6 'p7 'p8"
	@ close test_script
	@ write sys$output "** made $@"

