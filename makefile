#
# makefile for vile.  
# The defs for what system this is really running on are in estruct.h.
#  Be sure to edit that, to define your system.
# The command/name/key/function bindings are defined in the file "cmdtbl". The
#  mktbls program parses this to produce nebind.h, nename.h, and nefunc.h,
#  which are used by the rest of the build.
#
#  The version number (currently two point one) is found near the top of
#  edef.h, and is displayed with the '*' command.
# 		Paul Fox
#
# original for uemacs:		Adam Fritz			July 30,1987


# To change screen driver modules, change SCREEN below, and edit estruct.h to
#  make sure the correct one is #defined as "1", and the others all as "0"
#SCREEN = at386
# if you use tcap.c, you'll need libtermcap.a too.
SCREEN = tcap

TARGET = vile

#DESTDIR = $(HOME)/bin
DESTDIR = /usr/local/bin
LIBS = -ltermcap

REMOTE=jdhome!us

#CFLAGS = -O
CFLAGS = -g
#CFLAGS = -g -systype sysv	# for the mips machine
CC=cc


# All of the makefiles which should be preserved
MAKEFILES = makefile make.ini
HEADER_BUILDER = ./mktbls

ALLTOOLS = $(MAKEFILES)

# this stuff lives in the shorten subdirectory.  It was lifted from the
#	GNU emacs distribution.  See the file COPYING for more info.
SHORTSTUFF = shorten/COPYING \
	shorten/names.c \
	shorten/dups.c \
	shorten/defines.c \
	shorten/header.h \
	shorten/reserved \
	shorten/special

# these are normal editable headers
HDRS = estruct.h epath.h evar.h edef.h

# these headers are built by the mktbls program from the information
#  in cmdtbl, but are backed up and
#  distributed anyway, in case you can't get mktbls running
BUILTHDRS = nebind.h nefunc.h nename.h 

# this is stuff not normally included in the build, but which shouldn't
#  be thrown away (yet).  For instance, ebind.h has per-machine binding
#  information that hasn't been incorporated into cmdtbl yet.
OLDHDRS = ebind.h efunc.h

ALLHDRS = $(HDRS) $(BUILTHDRS) $(OLDHDRS)

# All the C files which should be saved
#  (including tools, like mktbls.c, unused screen drivers, etc.)
CSRCac = ansi.c at386.c basic.c bind.c buffer.c crypt.c csrch.c
CSRCde = dg10.c display.c dolock.c eval.c exec.c
CSRCfh = file.c fileio.c finderr.c globals.c hp110.c hp150.c
CSRCim = ibmpc.c input.c isearch.c line.c lock.c main.c mktbls.c
CSRCnr = news.c npopen.c opers.c oneliner.c random.c region.c
CSRCst = search.c spawn.c st520.c tags.c tcap.c termio.c tipc.c
CSRCuz = undo.c vmalloc.c vmsvt.c vt52.c window.c word.c wordmov.c z309.c

CSRC = $(CSRCac) $(CSRCde) $(CSRCfh) $(CSRCim) $(CSRCnr) \
	$(CSRCst) $(CSRCuz)

# non-C source code
OTHERSRC = z100bios.asm news.cps

# text and data files
TEXTFILES = README cmdtbl vile.hlp tags buglist readme.news

ALLSRC = $(CSRC) $(OTHERSRC)

EVERYTHING = $(ALLTOOLS) $(ALLHDRS) $(ALLSRC) $(TEXTFILES) $(SHORTSTUFF)

SRC = npopen.c finderr.c main.c buffer.c $(SCREEN).c termio.c display.c \
	word.c wordmov.c window.c spawn.c \
	region.c search.c random.c isearch.c lock.c line.c \
	input.c fileio.c exec.c file.c eval.c \
	dolock.c crypt.c bind.c basic.c opers.c undo.c csrch.c tags.c \
	vmalloc.c globals.c oneliner.c

OBJ = npopen.o finderr.o main.o buffer.o $(SCREEN).o termio.o display.o \
	word.o wordmov.o window.o spawn.o \
	region.o search.o random.o isearch.o lock.o line.o \
	input.o fileio.o exec.o file.o eval.o \
	dolock.o crypt.o bind.o basic.o opers.o undo.o csrch.o tags.o \
	vmalloc.o globals.o oneliner.o

all: $(TARGET)

$(TARGET) : $(BUILTHDRS) $(OBJ) makefile
	-mv $(TARGET) o$(TARGET)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

$(BUILTHDRS): cmdtbl $(HEADER_BUILDER)
	$(HEADER_BUILDER) cmdtbl

install : $(TARGET)
	cp $(TARGET) $(DESTDIR)
	test -f vile.hlp && /bin/rm -f $(DESTDIR)/vile.hlp
	cp vile.hlp $(DESTDIR)
	chmod 0644 $(DESTDIR)/vile.hlp 

compr-shar:
	[ -d cshardir ] || mkdir cshardir
#	add -a for archive headers, add -s pgf@cayman.com for submitted-by
	shar -p -nvile -L55 -o cshardir/vileshar \
		-t README -C `ls $(EVERYTHING) | sed /README/d`

shar:
	[ -d shardir ] || mkdir shardir
#	add -a for archive headers, add -s pgf@cayman.com for submitted-by
	shar -x -nvile -l55 -o shardir/vileshar `ls $(EVERYTHING)`

# only uucp things changed since last time
uuto:
	uuto `ls -t $(EVERYTHING) uutodone | sed '/uutodone/q'` $(REMOTE)
	date >uutodone

floppy:
	ls $(EVERYTHING) | oo

# you don't want to know...
dosfloppy:
	(								\
	echo echo on							;\
	for x in `ls -t $(EVERYTHING) dosback.bat | sed '/dosback.bat/q'` ;\
	do								\
		echo copy u:$$x a:					;\
	done 								;\
	echo quit							;\
	) >tmp.bat
	ud < tmp.bat >dosback.bat
	/bin/rm -f tmp.bat
	-dos
	>dosback.bat
	
newdosfloppy:
	touch 0101010170 dosback.bat

# dump a list of the important files
list:
	@ls $(EVERYTHING) | more

# dump a list of files that may have changed since last backup
list-writeable:
	@for x in $(EVERYTHING) ;\
	do \
		if [ -w $$x ] ;\
		then \
			echo $$x ;\
		fi \
	done

no-write:
	chmod -w $(EVERYTHING)

tags:
	dotags $(SRC) $(HDRS)

lint:	$(SRC)
	lint -hbvxac $(SRC) >lint.out 

clean:
	rm -f *.o o$(TARGET) $(BUILTHDRS) $(HEADER_BUILDER) news.h core *~ *.BAK

clobber: clean
	rm -f $(TARGET)

news.h: news.cps
	cps news.cps

print:
	pr makefile $(HDRS) $(SRC) | lpr

depend:	 $(SRC) $(HDRS) $(BUILTHDRS)
	mv -f makefile makefile.orig
	(sed -e '/^#DEPENDS/,$$d' makefile.orig ; \
		echo "#DEPENDS" ; \
		$(CC) -M $(CFLAGS) $? ) > makefile

# you don't need this if SHORTNAMES is 0 in estruct.h
estruct.h: shorten/remap.h

shorten/remap.h:
	cd shorten; $(MAKE) remap.h

#DEPENDS
news.o: news.c
news.o: /usr/include/stdio.h
news.o: ./estruct.h
news.o: ./edef.h
news.o: ./news.h
word.o: word.c
word.o: /usr/include/stdio.h
word.o: ./estruct.h
word.o: ./edef.h
vmsvt.o: vmsvt.c
vmsvt.o: /usr/include/stdio.h
vmsvt.o: ./estruct.h
vmsvt.o: ./edef.h
window.o: window.c
window.o: /usr/include/stdio.h
window.o: ./estruct.h
window.o: ./edef.h
vt52.o: vt52.c
vt52.o: /usr/include/stdio.h
vt52.o: ./estruct.h
vt52.o: ./edef.h
tipc.o: tipc.c
tipc.o: /usr/include/stdio.h
tipc.o: ./estruct.h
tipc.o: ./edef.h
tcap.o: tcap.c
tcap.o: /usr/include/stdio.h
tcap.o: ./estruct.h
tcap.o: ./edef.h
termio.o: termio.c
termio.o: /usr/include/stdio.h
termio.o: ./estruct.h
termio.o: ./edef.h
termio.o: /usr/include/sgtty.h
termio.o: /usr/include/sys/ioctl.h
#termio.o: /usr/include/sys/ttychars.h
#termio.o: /usr/include/sys/ttydev.h
termio.o: /usr/include/signal.h
termio.o: /usr/include/sys/ioctl.h
spawn.o: spawn.c
spawn.o: /usr/include/stdio.h
spawn.o: ./estruct.h
spawn.o: ./edef.h
spawn.o: /usr/include/signal.h
st520.o: st520.c
st520.o: /usr/include/stdio.h
st520.o: ./estruct.h
st520.o: ./edef.h
region.o: region.c
region.o: /usr/include/stdio.h
region.o: ./estruct.h
region.o: ./edef.h
search.o: search.c
search.o: /usr/include/stdio.h
search.o: ./estruct.h
search.o: ./edef.h
main.o: main.c
main.o: /usr/include/stdio.h
main.o: ./estruct.h
main.o: ./nefunc.h
main.o: ./edef.h
main.o: ./nebind.h
main.o: ./nename.h
random.o: random.c
random.o: /usr/include/stdio.h
random.o: ./estruct.h
random.o: ./edef.h
isearch.o: isearch.c
isearch.o: /usr/include/stdio.h
isearch.o: ./estruct.h
isearch.o: ./edef.h
lock.o: lock.c
lock.o: /usr/include/stdio.h
lock.o: ./estruct.h
lock.o: ./edef.h
lock.o: /usr/include/sys/errno.h
line.o: line.c
line.o: /usr/include/stdio.h
line.o: ./estruct.h
line.o: ./edef.h
ibmpc.o: ibmpc.c
ibmpc.o: /usr/include/stdio.h
ibmpc.o: ./estruct.h
ibmpc.o: ./edef.h
input.o: input.c
input.o: /usr/include/stdio.h
input.o: ./estruct.h
input.o: ./edef.h
hp110.o: hp110.c
hp110.o: /usr/include/stdio.h
hp110.o: ./estruct.h
hp110.o: ./edef.h
hp150.o: hp150.c
hp150.o: /usr/include/stdio.h
hp150.o: ./estruct.h
hp150.o: ./edef.h
fileio.o: fileio.c
fileio.o: /usr/include/stdio.h
fileio.o: ./estruct.h
fileio.o: ./edef.h
exec.o: exec.c
exec.o: /usr/include/stdio.h
exec.o: ./estruct.h
exec.o: ./edef.h
file.o: file.c
file.o: /usr/include/stdio.h
file.o: ./estruct.h
file.o: ./edef.h
eval.o: eval.c
eval.o: /usr/include/stdio.h
eval.o: ./estruct.h
eval.o: ./edef.h
eval.o: ./evar.h
display.o: display.c
display.o: /usr/include/stdio.h
display.o: ./estruct.h
display.o: ./edef.h
dolock.o: dolock.c
buffer.o: buffer.c
buffer.o: /usr/include/stdio.h
buffer.o: ./estruct.h
buffer.o: ./edef.h
crypt.o: crypt.c
crypt.o: /usr/include/stdio.h
crypt.o: ./estruct.h
crypt.o: ./edef.h
dg10.o: dg10.c
dg10.o: /usr/include/stdio.h
dg10.o: ./estruct.h
dg10.o: ./edef.h
bind.o: bind.c
bind.o: /usr/include/stdio.h
bind.o: ./estruct.h
bind.o: ./edef.h
bind.o: ./epath.h
basic.o: basic.c
basic.o: /usr/include/stdio.h
basic.o: ./estruct.h
basic.o: ./edef.h
ansi.o: ansi.c
ansi.o: /usr/include/stdio.h
ansi.o: ./estruct.h
ansi.o: ./edef.h
opers.o: opers.c
opers.o: ./estruct.h
opers.o: ./edef.h
wordmov.o: wordmov.c
wordmov.o: ./estruct.h
wordmov.o: ./edef.h
csrch.o: csrch.c
csrch.o: ./estruct.h
csrch.o: ./edef.h
undo.o: undo.c
undo.o: ./estruct.h
undo.o: ./edef.h
tags.o: tags.c
tags.o: ./estruct.h
tags.o: ./edef.h
finderr.o: finderr.c
finderr.o: ./estruct.h
finderr.o: ./edef.h
at386.o: at386.c
at386.o: /usr/include/stdio.h
at386.o: ./estruct.h
at386.o: ./edef.h
npopen.o: npopen.c
npopen.o: ./estruct.h
npopen.o: ./edef.h
oneliner.o: oneliner.c
oneliner.o: ./estruct.h
oneliner.o: ./edef.h
globals.o: globals.c
globals.o: ./estruct.h
globals.o: ./edef.h
