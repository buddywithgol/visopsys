##
##  Visopsys
##  Copyright (C) 1998-2015 J. Andrew McLaughlin
##
##  Makefile
##

ROOT = ../..
BUILDDIR = ${ROOT}/build/programs
INCDIR = ../include
LIBDIR = ${ROOT}/build/system/libraries

include ${ROOT}/Makefile.include

STDDEPS = ${ROOT}/Makefile.include Makefile ${INCDIR}/*.h \
	${INCDIR}/sys/*.h ${LIBDIR}/crt0.o
INCLUDE = -nostdinc -I${INCDIR} -I${INCDIR}/c++
CFLAGS = ${OPT} ${ARCH} ${CCODEGEN} ${CWARN} ${INCLUDE} ${DEBUG}
C++FLAGS = ${OPT} ${ARCH} ${C++CODEGEN} ${C++WARN} ${INCLUDE} ${DEBUG}
LFLAGS = -L${LIBDIR} -nostdlib \
	-Wl,${LIBDIR}/crt0.o,--warn-common,-X,--oformat,elf32-i386

ifdef DEBUG
	override DEBUG = -DDEBUG
else
	STRIP = strip -s ${COBJS} ${C++OBJS}
endif

CNAMES = adduser \
	bootmenu \
	cal \
	calc \
	cat \
	cdrom \
	chkdisk \
	clock \
	cmdwin \
	computer \
	confedit \
	console \
	copy-boot \
	copy-mbr \
	cp \
	date \
	defrag \
	deluser \
	disks \
	disprops \
	domainname \
	edit \
	file \
	filebrowse \
	filesys \
	find \
	fontutil \
	format \
	help \
	hexdump \
	hostname \
	iconwin \
	ifconfig \
	imgboot \
	install \
	keymap \
	kill \
	login \
	logout \
	ls \
	lsdev \
	md5 \
	mem \
	mines \
	mkdir \
	more \
	mount \
	mv \
	nm \
	passwd \
	ping \
	progman \
	ps \
	ramdisk \
	reboot \
	renice \
	rm \
	rmdir \
	screenshot \
	shutdown \
	snake \
	sync \
	telnet \
	test \
	touch \
	umount \
	uname \
	uptime \
	users \
	view \
	vsh \
	wallpaper

COBJS = $(addprefix ${BUILDDIR}/, ${CNAMES})
C++OBJS = $(addprefix ${BUILDDIR}/, ${C++NAMES})

# Targets

all: target-dirs strip
	${MAKE} -C fdisk
	${MAKE} -C de
	${MAKE} -C es

target-dirs:
	mkdir -p ${BUILDDIR}

${BUILDDIR}/%: %.c ${STDDEPS}
	${CC} ${CFLAGS} ${LFLAGS} $< -lwindow -lintl -lvsh -ldl -lc -lgcc -o $@

${BUILDDIR}/%: %.cpp ${STDDEPS}
	${CC++} ${C++FLAGS} -nostartfiles ${LFLAGS} $< -lstdc++ -lc -lgcc -o $@

strip: ${COBJS} ${C++OBJS}
	${STRIP}

clean:
	${MAKE} -C fdisk clean
	${MAKE} -C de clean
	${MAKE} -C es clean
	rm -Rf *~ *.o core ${COBJS} ${C++OBJS}

# Object files with explicit rules

${BUILDDIR}/copy-boot: copy-boot.c ${STDDEPS}
	${CC} ${CFLAGS} ${LFLAGS} -DVISOPSYS $< -lintl -lc -o $@

${BUILDDIR}/format: format.c ${STDDEPS}
	${CC} ${CFLAGS} ${LFLAGS} $< -lntfs -lwindow -lintl -lvsh -lc -o $@

