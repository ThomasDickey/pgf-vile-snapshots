#
# makefile for vile.  
# The defs for what system this is really running on are in estruct.h.
#  Be sure to edit that, to define your system.
# The command/name/key/function bindings are defined in the file "cmdtbl". The
#  mktbls program parses this to produce nebind.h, nename.h, and nefunc.h,
#  which are used by the rest of the build.
#
#  The version number (currently three) is found near the top of
#  edef.h, and is displayed with the '*' and ":version" commands.
# 		Paul Fox
#
# original makefile for uemacs:	Adam Fritz	July 30,1987
#
# $Log: makefile,v $
# Revision 1.20  1991/10/22 14:36:01  pgf
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


# To change screen driver modules, change SCREEN below, and edit estruct.h to
#  make sure the correct one is #defined as "1", and the others all as "0"
#SCREEN = at386
# if you use tcap.c, you'll need libtermcap.a too.
SCREEN = tcap

TARGET = vile

# install to DESTDIR1 if it's writable, else DESTDIR2
DESTDIR1 = /usr/local/bin
DESTDIR2 = $(HOME)/bin
LIBS = -ltermcap

REMOTE=towno!pgf

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
#SHORTSTUFF = shorten/COPYING \
#	shorten/names.c \
#	shorten/dups.c \
#	shorten/defines.c \
#	shorten/header.h \
#	shorten/reserved \
#	shorten/special

# these are normal editable headers
HDRS = estruct.h epath.h evar.h edef.h

# these headers are built by the mktbls program from the information in cmdtbl
BUILTHDRS = nebind.h nefunc.h nename.h 

# this is obsolete stuff NOT needed by the build, but it shouldn't
#  be thrown away (yet).  For instance, ebind.h has per-machine binding
#  information that hasn't been incorporated into cmdtbl yet.
OLDHDRS = ebind.h efunc.h

ALLHDRS = $(HDRS) $(OLDHDRS)

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
TEXTFILES = README CHANGES cmdtbl vile.hlp tags buglist readme.news

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

saber_src: 
	#load $(CFLAGS) $(SRC) $(LIBS) 

saber_obj: $(OBJ) 
	#load $(CFLAGS) $(OBJ) $(LIBS) 


$(BUILTHDRS): cmdtbl $(HEADER_BUILDER)
	$(HEADER_BUILDER) cmdtbl

# install to DESTDIR1 if it's writable, else DESTDIR2
install:
	@[ -x $(TARGET) ] || (echo must make $(TARGET) first && exit 1)
	[ -w $(DESTDIR1) ] && dest=$(DESTDIR1) || dest=$(DESTDIR2) ;\
	cp $(TARGET) $$dest ;\
	test -f vile.hlp && /bin/rm -f $$dest/vile.hlp ;\
	cp vile.hlp $$dest ;\
	chmod 0644 $$dest/vile.hlp 

compr-shar:
	[ -d cshardir ] || mkdir cshardir
#	add -a for archive headers, add -s pgf@cayman.com for submitted-by
	shar -p -nvile -L55 -o cshardir/vileshar \
		-T README -C `ls $(EVERYTHING) | sed /README/d`

shar:
	[ -d shardir ] || mkdir shardir
#	add -a for archive headers, add -s pgf@cayman.com for submitted-by
	shar -x -a -spgf@cayman.com -nVile -L55 \
			-o shardir/vileshar `ls $(EVERYTHING)`

# only uucp things changed since last time
uuto:
	uuto `ls -t $(EVERYTHING) uutodone | sed '/uutodone/q'` $(REMOTE)
	date >uutodone

floppy:
	ls $(EVERYTHING) | oo

# you don't want to know...
dosscript:
	(								\
	echo echo on							;\
	for x in `ls -t $(EVERYTHING) dosback.bat | sed '/dosback.bat/q'` ;\
	do								\
		echo copy o:$$x a:					;\
	done 								;\
	echo quit							;\
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

# dump a list of files that may have changed since last backup
rw list-writeable:
	@ls -l $(EVERYTHING) | \
		egrep '^[^l].w' | \
		sed 's;.* ;;'   # strip to last space

no-write:
	chmod -w $(EVERYTHING)

update:
	nupdatefile.pl -r $(EVERYTHING)

tagfile:
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

populate: $(EVERYTHING)

$(EVERYTHING):
	co -r$(revision) $@

# you need this if SHORTNAMES is 0 in estruct.h
# estruct.h: shorten/remap.h

# or this either
shorten/remap.h:
	cd shorten; $(MAKE) remap.h

#DEPENDS
news.o: news.c
news.o: /usr/include/stdio.h
news.o: estruct.h
news.o: edef.h
news.o: news.h
word.o: word.c
word.o: /usr/include/stdio.h
word.o: estruct.h
word.o: edef.h
vmsvt.o: vmsvt.c
vmsvt.o: /usr/include/stdio.h
vmsvt.o: estruct.h
vmsvt.o: edef.h
window.o: window.c
window.o: /usr/include/stdio.h
window.o: estruct.h
window.o: edef.h
vt52.o: vt52.c
vt52.o: /usr/include/stdio.h
vt52.o: estruct.h
vt52.o: edef.h
tipc.o: tipc.c
tipc.o: /usr/include/stdio.h
tipc.o: estruct.h
tipc.o: edef.h
tcap.o: tcap.c
tcap.o: /usr/include/stdio.h
tcap.o: estruct.h
tcap.o: edef.h
termio.o: termio.c
termio.o: /usr/include/stdio.h
termio.o: estruct.h
termio.o: edef.h
termio.o: /usr/include/sgtty.h
termio.o: /usr/include/sys/ioctl.h
#termio.o: /usr/include/sys/ttychars.h
#termio.o: /usr/include/sys/ttydev.h
termio.o: /usr/include/signal.h
termio.o: /usr/include/sys/ioctl.h
spawn.o: spawn.c
spawn.o: /usr/include/stdio.h
spawn.o: estruct.h
spawn.o: edef.h
spawn.o: /usr/include/signal.h
st520.o: st520.c
st520.o: /usr/include/stdio.h
st520.o: estruct.h
st520.o: edef.h
region.o: region.c
region.o: /usr/include/stdio.h
region.o: estruct.h
region.o: edef.h
search.o: search.c
search.o: /usr/include/stdio.h
search.o: estruct.h
search.o: edef.h
main.o: main.c
main.o: /usr/include/stdio.h
main.o: estruct.h
main.o: nefunc.h
main.o: edef.h
main.o: nebind.h
main.o: nename.h
random.o: random.c
random.o: /usr/include/stdio.h
random.o: estruct.h
random.o: edef.h
isearch.o: isearch.c
isearch.o: /usr/include/stdio.h
isearch.o: estruct.h
isearch.o: edef.h
lock.o: lock.c
lock.o: /usr/include/stdio.h
lock.o: estruct.h
lock.o: edef.h
lock.o: /usr/include/sys/errno.h
line.o: line.c
line.o: /usr/include/stdio.h
line.o: estruct.h
line.o: edef.h
ibmpc.o: ibmpc.c
ibmpc.o: /usr/include/stdio.h
ibmpc.o: estruct.h
ibmpc.o: edef.h
input.o: input.c
input.o: /usr/include/stdio.h
input.o: estruct.h
input.o: edef.h
hp110.o: hp110.c
hp110.o: /usr/include/stdio.h
hp110.o: estruct.h
hp110.o: edef.h
hp150.o: hp150.c
hp150.o: /usr/include/stdio.h
hp150.o: estruct.h
hp150.o: edef.h
fileio.o: fileio.c
fileio.o: /usr/include/stdio.h
fileio.o: estruct.h
fileio.o: edef.h
exec.o: exec.c
exec.o: /usr/include/stdio.h
exec.o: estruct.h
exec.o: edef.h
file.o: file.c
file.o: /usr/include/stdio.h
file.o: estruct.h
file.o: edef.h
eval.o: eval.c
eval.o: /usr/include/stdio.h
eval.o: estruct.h
eval.o: edef.h
eval.o: evar.h
display.o: display.c
display.o: /usr/include/stdio.h
display.o: estruct.h
display.o: edef.h
dolock.o: dolock.c
buffer.o: buffer.c
buffer.o: /usr/include/stdio.h
buffer.o: estruct.h
buffer.o: edef.h
crypt.o: crypt.c
crypt.o: /usr/include/stdio.h
crypt.o: estruct.h
crypt.o: edef.h
dg10.o: dg10.c
dg10.o: /usr/include/stdio.h
dg10.o: estruct.h
dg10.o: edef.h
bind.o: bind.c
bind.o: /usr/include/stdio.h
bind.o: estruct.h
bind.o: edef.h
bind.o: epath.h
basic.o: basic.c
basic.o: /usr/include/stdio.h
basic.o: estruct.h
basic.o: edef.h
ansi.o: ansi.c
ansi.o: /usr/include/stdio.h
ansi.o: estruct.h
ansi.o: edef.h
opers.o: opers.c
opers.o: estruct.h
opers.o: edef.h
wordmov.o: wordmov.c
wordmov.o: estruct.h
wordmov.o: edef.h
csrch.o: csrch.c
csrch.o: estruct.h
csrch.o: edef.h
undo.o: undo.c
undo.o: estruct.h
undo.o: edef.h
tags.o: tags.c
tags.o: estruct.h
tags.o: edef.h
finderr.o: finderr.c
finderr.o: estruct.h
finderr.o: edef.h
at386.o: at386.c
at386.o: /usr/include/stdio.h
at386.o: estruct.h
at386.o: edef.h
npopen.o: npopen.c
npopen.o: estruct.h
npopen.o: edef.h
oneliner.o: oneliner.c
oneliner.o: estruct.h
oneliner.o: edef.h
globals.o: globals.c
globals.o: estruct.h
globals.o: edef.h
vmalloc.o: estruct.h
vmalloc.o: edef.h
