#
# makefile for Watcom C using Watcom MAKE 
# based on the original makefile of vile 3.46 (see the original makefile)
# T.DANG (dang@cogit.ign.fr)
#
# $Header: /usr/build/VCS/pgf-vile/RCS/makefile.wat,v 1.12 1994/08/08 16:12:29 pgf Exp $
#
# if you use the watcom version of vile, you may want to "set DOS4G=quiet"
# to suppress the DOS 4G/W banner that comes up from the Rational Systems
# DOS extender otherwise.
#
#

SCREENDEF = IBMPC
SCREEN= ibmpc 
#SCREEN= ansi 
#SCREENDEF = ANSI

#define PVGA for Paradise VGA (because there are some problems with this card)
#To fix it, use /dPVGA=1
#CFLAGS= /d$(SCREENDEF)=1 /dscrn_chosen=1 /dPVGA=1 

# debugging
#CFLAGS= /d$(SCREENDEF)=1 /dscrn_chosen=1 /d2
CFLAGS= /d$(SCREENDEF)=1 /dscrn_chosen=1

# these are normal editable headers
HDRS = estruct.h epath.h edef.h proto.h dirstuff.h

# these headers are built by the mktbls program from the information in cmdtbl
# and in modetbl
BUILTHDRS = nebind.h nefunc.h nemode.h nename.h nevars.h

SRC = 	main.c $(SCREEN).c basic.c bind.c buffer.c crypt.c &
	csrch.c display.c eval.c exec.c externs.c fences.c file.c filec.c &
	fileio.c finderr.c glob.c globals.c history.c input.c insert.c isearch.c &
	line.c modes.c msgs.c npopen.c oneliner.c opers.c path.c random.c regexp.c &
	region.c search.c select.c spawn.c &
	tags.c tbuff.c termio.c tmp.c undo.c &
	version.c vmalloc.c window.c word.c wordmov.c map.c

OBJ = 	main.obj $(SCREEN).obj basic.obj bind.obj buffer.obj crypt.obj &
      	csrch.obj display.obj eval.obj exec.obj externs.obj fences.obj file.obj filec.obj &
	fileio.obj finderr.obj glob.obj globals.obj history.obj input.obj insert.obj isearch.obj &
	line.obj modes.obj msgs.obj npopen.obj oneliner.obj opers.obj path.obj random.obj regexp.obj &
	region.obj search.obj select.obj spawn.obj &
	tags.obj tbuff.obj termio.obj tmp.obj undo.obj &
	version.obj vmalloc.obj window.obj word.obj wordmov.obj map.obj


vile.exe: $(BUILTHDRS) $(OBJ) vile.lnk
	wlink @vile 

vile.lnk: makefile makefile.wat
	echo DEBUG ALL >$^@
        echo NAME vile >>$^@
        echo OPTION MAP >>$^@
        echo OPTION STACK=16384 >>$^@
	for %i in ($(OBJ)) do echo FILE %i >>$^@

.c.obj:	estruct.h nemode.h edef.h proto.h 
#	wcl386/p $[* /c /d2 $(CFLAGS) 
	wcl386/p $[* /c /ols $(CFLAGS) 

nebind.h &
nefunc.h &
nename.h &
nevars.h :	cmdtbl MKTBLS.EXE
	MKTBLS.EXE cmdtbl

nemode.h:	modetbl MKTBLS.EXE
	MKTBLS.EXE modetbl

main.obj:	nevars.h
bind.obj:	epath.h
filec.obj:	dirstuff.h
eval.obj:	nevars.h
glob.obj:	dirstuff.h
externs.obj:	nebind.h nename.h nefunc.h
path.obj:	dirstuff.h

MKTBLS.EXE:  mktbls.c
	wcl386/p mktbls.c
	del mktbls.obj
clean:
	del *.obj
	del nebind.h
	del nefunc.h
	del nename.h
	del nevars.h
	del nemode.h
	del MKTBLS.EXE
