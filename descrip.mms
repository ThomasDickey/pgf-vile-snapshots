#
# VMS makefile for vile.  Requires "MMS"
#

# To change screen driver modules, change SCREEN and SCRDEF below, OR edit
# estruct.h to make sure the correct one is #defined as "1", and the others
# all as "0".  If you use tcap.c, you'll need libtermcap.a too.  If you use
# x11.c, you'll need libX11.a too.

# for regular vile, use these:
SCREEN = vmsvt
LIBS = -ltermcap
TARGET = vile.exe
SCRDEF = "VMSVT","scrn_chosen"

# for building the X version, xvile, use these:
#SCREEN = x11
#LIBS = -lX11
#TARGET = xvile.exe
#SCRDEF = "X11","scrn_chosen"

# if you want the help file (vile.hlp) to go somewhere other than your $PATH
#  or one of the hard-code paths in epath.h  (it goes to the same place vile
#  does by default)
HELP_LOC=

LINKFLAGS = /MAP/CROSS_REFERENCE/EXEC=$(MMS$TARGET_NAME).EXE

INCS = []

# All of the makefiles which should be preserved
MAKFILES = makefile make.ini descrip.mms
MKTBLS = mktbls.EXE

ALLTOOLS = $(MAKFILES)


# these are normal editable headers
HDRS = estruct.h epath.h edef.h proto.h

# these headers are built by the mktbls program from the information in cmdtbl
# and in modetbl
BUILTHDRS = nebind.h nefunc.h nemode.h nename.h nevars.h

ALLHDRS = $(HDRS)

# All the C files which should be saved
#  (including tools, like mktbls.c, unused screen drivers, etc.)
CSRCac = ansi.c at386.c basic.c bind.c buffer.c crypt.c csrch.c
CSRCde = dg10.c display.c eval.c exec.c externs.c
CSRCfh = fences.c file.c filec.c fileio.c finderr.c globals.c history.c hp110.c hp150.c
CSRCim = ibmpc.c input.c insert.c isearch.c line.c main.c modes.c mktbls.c
CSRCnr = npopen.c opers.c oneliner.c path.c random.c regexp.c region.c
CSRCst = search.c spawn.c st520.c tags.c tbuff.c tcap.c termio.c tipc.c
CSRCuw = undo.c vmalloc.c vmsvt.c vt52.c window.c word.c wordmov.c
CSRCxz = x11.c z309.c z_ibmpc.c

CSRC = $(CSRCac) $(CSRCde) $(CSRCfh) $(CSRCim) $(CSRCnr) \
	$(CSRCst) $(CSRCuw) $(CSRCxz)

# non-C source code
OTHERSRC = z100bios.asm

# text and data files
TEXTFILES = README CHANGES cmdtbl modetbl vile.hlp buglist revlist \
	README.X11

ALLSRC = $(CSRC) $(OTHERSRC)

EVERYTHING = $(ALLTOOLS) $(ALLHDRS) $(ALLSRC) $(TEXTFILES) $(SHORTSTUFF)

SRC =	main.c \
	$(SCREEN).c \
	basic.c \
	bind.c \
	buffer.c \
	crypt.c \
	csrch.c \
	display.c \
	eval.c \
	exec.c \
	externs.c \
	fences.c \
	file.c \
	filec.c \
	fileio.c \
	finderr.c \
	globals.c \
	history.c \
	input.c \
	insert.c \
	isearch.c \
	line.c \
	modes.c \
	npopen.c \
	oneliner.c \
	opers.c \
	path.c \
	random.c \
	regexp.c \
	region.c \
	search.c \
	spawn.c \
	tags.c \
	tbuff.c \
	termio.c \
	undo.c \
	vmalloc.c \
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
	eval.obj,\
	exec.obj,\
	externs.obj,\
	fences.obj,\
	file.obj,\
	filec.obj,\
	fileio.obj,\
	finderr.obj,\
	globals.obj,\
	history.obj,\
	input.obj,\
	insert.obj,\
	isearch.obj,\
	line.obj,\
	modes.obj,\
	npopen.obj,\
	oneliner.obj,\
	opers.obj,\
	path.obj,\
	random.obj,\
	regexp.obj,\
	region.obj,\
	search.obj,\
	spawn.obj,\
	tags.obj,\
	tbuff.obj,\
	termio.obj,\
	undo.obj,\
	vmalloc.obj,\
	window.obj,\
	word.obj,\
	wordmov.obj

all :	$(TARGET)
	@ WRITE SYS$OUTPUT "** made $@"

nebind.h \
nefunc.h \
nename.h \
nevars.h :	cmdtbl $(MKTBLS)
	MKTBLS cmdtbl

nemode.h :	modetbl $(MKTBLS)
	MKTBLS modetbl

# install to DESTDIR1 if it's writable, else DESTDIR2
install :
	@ WRITE SYS$ERROR "** no rule for $@"
	
clean :
	@- remove -f *.obj;* $(BUILTHDRS);* $(MKTBLS);* *.BAK;* *.LOG;* *.LIS;* *.MAP;*

clobber : clean
	@- remove -f $(TARGET);*

$(OBJ) :: estruct.h nemode.h edef.h proto.h

bind.obj ::	epath.h
eval.obj ::	nevars.h
externs.obj ::	nebind.h nename.h nefunc.h
vmalloc.obj ::	nevars.h

.first :
	@ define/nolog SYS SYS$LIBRARY		! fix includes to <sys/...>
	@ MKTBLS :== $SYS$DISK:'F$DIRECTORY()$(MKTBLS)	! make a foreign command

.last :
	@- purge *.dia,*.lis,*.obj,*.map,*.exe,*.log

CFLAGS =-
	/Diagnostics /Listing /Debug /Define=("os_chosen",$(SCRDEF)) -
	/Object=$@ /Include=($(INCS)) /G_FLOAT

LIB_ARGS=vms_link.opt/opt

.C.OBJ :
	$(CC) $(CFLAGS) $(MMS$SOURCE)
.OBJ.EXE :
	$(LINK) $(LINKFLAGS) $(MMS$TARGET_NAME)$(OBJ_ARGS),$(LIB_ARGS)

$(TARGET) : $(OBJ)
	$(LINK) $(LINKFLAGS) $(OBJ),$(LIB_ARGS)

# $Log: descrip.mms,v $
# Revision 1.1  1993/03/17 09:50:19  pgf
# Initial revision
#
#
