#!/bin/make
#
# help - makefile for calc help files
#
# Copyright (C) 1999-2002  Landon Curt Noll
#
# Calc is open software; you can redistribute it and/or modify it under
# the terms of the version 2.1 of the GNU Lesser General Public License
# as published by the Free Software Foundation.
#
# Calc is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU Lesser General
# Public License for more details.
#
# A copy of version 2.1 of the GNU Lesser General Public License is
# distributed with calc under the filename COPYING-LGPL.  You should have
# received a copy with calc; if not, write to Free Software Foundation, Inc.
# 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
#
# @(#) $Revision: 29.25 $
# @(#) $Id: Makefile,v 29.25 2004/07/28 12:21:05 chongo Exp $
# @(#) $Source: /usr/local/src/cmd/calc/help/RCS/Makefile,v $
#
# Under source code control:	1991/07/23 06:47:57
# File existed as early as:	1991
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-) http://www.isthe.com/chongo/tech/comp/calc/
#
# calculator by David I. Bell with help/mods from others
# Makefile by Landon Curt Noll

# required vars
#
SHELL= /bin/sh
MAKE_FILE = Makefile

####
# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
####

# Where the system include (.h) files are kept
#
# For DJGPP, select:
#
#	INCDIR= /dev/env/DJDIR/include
#
# If in doubt, set:
#
#	INCDIR= /usr/include
#

#INCDIR= /usr/local/include
#INCDIR= /dev/env/DJDIR/include
INCDIR= /usr/include

# where to install calc realted things
#
# ${BINDIR}		where to install calc binary files
# ${LIBDIR}		where calc link library (*.a) files are installed
# ${CALC_SHAREDIR}	where to install calc help, .cal, startup, config files
#
# NOTE: The install rule prepends installation paths with $T, which
#	by default is empty.  If $T is non-empty, then installation
#	locations will be relative to the $T directory.
#
# For DJGPP, select:
#
#	BINDIR= /dev/env/DJDIR/bin
#	LIBDIR= /dev/env/DJDIR/lib
#	CALC_SHAREDIR= /dev/env/DJDIR/share/calc
#
# If in doubt, set:
#
#	BINDIR= /usr/bin
#	LIBDIR= /usr/lib
#	CALC_SHAREDIR= /usr/share/calc
#
#BINDIR= /usr/local/bin
#BINDIR= /dev/env/DJDIR/bin
BINDIR= /usr/bin

#LIBDIR= /usr/local/lib
#LIBDIR= /dev/env/DJDIR/lib
LIBDIR= /usr/lib

#CALC_SHAREDIR= /usr/local/lib/calc
#CALC_SHAREDIR= /dev/env/DJDIR/share/calc
CALC_SHAREDIR= /usr/share/calc

# By default, these values are based CALC_SHAREDIR, INCDIR, BINDIR
# ---------------------------------------------------------------
# ${HELPDIR}		where the help directory is installed
# ${CALC_INCDIR}	where the calc include files are installed
# ${CUSTOMCALDIR}	where custom *.cal files are installed
# ${CUSTOMHELPDIR}	where custom help files are installed
# ${CUSTOMINCPDIR}	where custom .h files are installed
# ${SCRIPTDIR}		where calc shell scripts are installed
#
# NOTE: The install rule prepends installation paths with $T, which
#	by default is empty.  If $T is non-empty, then installation
#	locations will be relative to the $T directory.
#
# If in doubt, set:
#
#	HELPDIR= ${CALC_SHAREDIR}/help
#	CALC_INCDIR= ${INCDIR}/calc
#	CUSTOMCALDIR= ${CALC_SHAREDIR}/custom
#	CUSTOMHELPDIR= ${CALC_SHAREDIR}/custhelp
#	CUSTOMINCDIR= ${CALC_INCDIR}/custom
#	SCRIPTDIR= ${BINDIR}/cscript
#
HELPDIR= ${CALC_SHAREDIR}/help
CALC_INCDIR= ${INCDIR}/calc
CUSTOMCALDIR= ${CALC_SHAREDIR}/custom
CUSTOMHELPDIR= ${CALC_SHAREDIR}/custhelp
CUSTOMINCDIR= ${CALC_INCDIR}/custom
SCRIPTDIR= ${BINDIR}/cscript

# T - top level directory under which calc will be installed
#
# The calc install is performed under $T, the calc build is
# performed under /.	The purpose for $T is to allow someone
# to install calc somewhere other than into the system area.
#
# For example, if:
#
#     BINDIR= /usr/bin
#     LIBDIR= /usr/lib
#     CALC_SHAREDIR= /usr/share/calc
#
# and if:
#
#     T= /var/tmp/testing
#
# Then the installation locations will be:
#
#     calc binary files:	/var/tmp/testing/usr/bin
#     calc link library:	/var/tmp/testing/usr/lib
#     calc help, .cal ...:	/var/tmp/testing/usr/share/calc
#     ... etc ...		/var/tmp/testing/...
#
# If $T is empty, calc is installed under /, which is the same
# top of tree for which it was built.  If $T is non-empty, then
# calc is installed under $T, as if one had to chroot under
# $T for calc to operate.
#
# If in doubt, use T=
#
T=

# Makefile debug
#
# Q=@	do not echo internal makefile actions (quiet mode)
# Q=	echo internal makefile actions (debug / verbose mode)
#
#Q=
Q=@

# standard tools
#
LCC= cc
ICFLAGS=
ILDFLAGS=
CHMOD= chmod
SED= sed
SORT= sort
FMT= fmt
CMP= cmp

# Some out of date operating systems require / want an executable to
# end with a certain file extension.  Some compile systems such as
# Cygwin build calc as calc.exe.  The EXT variable is used to denote
# the extension required by such.
#
# EXT=				# normal Un*x / Linux / GNU/Linux systems
# EXT=.exe			# windoz / Cygwin
#
# If in doubt, use EXT=
#
EXT=
#EXT=.exe

# Standard and Builtin help files
#
STD_HELP_FILES_1= intro overview help
STD_HELP_FILES_2= assoc

BLT_HELP_FILES_3= builtin

STD_HELP_FILES_4= command config custom define environment expression

BLT_HELP_FILES_5= errorcodes

STD_HELP_FILES_6= file history interrupt list mat

# beacuse obj is built special (due to confusion with it as
# a symlink for some built environments, we treat obj specially
# and call it obj.file
#
SPECIAL_HELP_7= obj.file

STD_HELP_FILES_8= operator statement

BLT_HELP_FILES_9= resource

STD_HELP_FILES_10= types usage script

BLT_HELP_FILES_10a= cscript

STD_HELP_FILES_10b= unexpected variable

BLT_HELP_FILES_11= bindings custom_cal libcalc new_custom resource

STD_HELP_FILES_12= archive

BLT_HELP_FILES_13= bugs changes

STD_HELP_FILES_14= contrib credit

BLT_HELP_FILES_14= COPYING COPYING-LGPL

STD_HELP_FILES_15= wishlist todo

# These files are used in the following order to construct full
#
FULL_HELP_FILES= ${STD_HELP_FILES_1} ${STD_HELP_FILES_2} \
		${BLT_HELP_FILES_3} ${STD_HELP_FILES_4} \
		${BLT_HELP_FILES_5} ${STD_HELP_FILES_6} \
		${SPECIAL_HELP_7} ${STD_HELP_FILES_8} \
		${BLT_HELP_FILES_9} ${STD_HELP_FILES_10} \
		${BLT_HELP_FILES_10a} ${STD_HELP_FILES_10b} \
		${BLT_HELP_FILES_11} ${STD_HELP_FILES_12} \
		${BLT_HELP_FILES_13} ${STD_HELP_FILES_14} \
		${BLT_HELP_FILES_14} ${STD_HELP_FILES_15}

# These full files are those who are not built or constrcuted
#
STD_HELP_FILES= ${STD_HELP_FILES_1} ${STD_HELP_FILES_2} \
		${STD_HELP_FILES_4} ${STD_HELP_FILES_6} \
		${STD_HELP_FILES_8} ${STD_HELP_FILES_10} \
		${STD_HELP_FILES_10b} \
		${STD_HELP_FILES_12} ${STD_HELP_FILES_14} \
		${STD_HELP_FILES_15}

# These full files are those who are built by this Makefile
#
# Note that ${SPECIAL_HELP_7} is not included in this list
# because of problems with its name.
#
BLT_HELP_FILES= ${BLT_HELP_FILES_3} ${BLT_HELP_FILES_5} \
		${BLT_HELP_FILES_9} \
		${BLT_HELP_FILES_10a} \
		${BLT_HELP_FILES_11} ${BLT_HELP_FILES_13} \
		${BLT_HELP_FILES_14}

# This list is prodiced by the detaillist rule when no WARNINGS are detected.
#
DETAIL_HELP= abs access acos acosh acot acoth acsc acsch address agd append \
	appr arg argv arrow asec asech asin asinh assign atan atan2 atanh \
	avg base base2 bernoulli bit blk blkcpy blkfree blocks bround btrunc \
	calc_tty calclevel catalan ceil cfappr cfsim char cmdbuf cmp comb \
	conj cos cosh cot coth count cp csc csch ctime delete den dereference \
	det digit digits display dp epsilon errcount errmax errno error euler \
	eval exp fact factor fclose fcnt feof ferror fflush fgetc fgetfield \
	fgetline fgets fgetstr fib files floor fopen forall fprintf fputc \
	fputs fputstr frac free freebernoulli freeeuler freeglobals freeredc \
	freestatics frem freopen fscan fscanf fseek fsize ftell gcd gcdrem \
	gd getenv hash head highbit hmean hnrmod hypot ilog ilog10 ilog2 \
	im indices inputlevel insert int inverse iroot isassoc isatty isblk \
	isconfig isdefined iserror iseven isfile ishash isident isint islist \
	ismat ismult isnull isnum isobj isobjtype isodd isprime isptr isqrt \
	isrand israndom isreal isrel issimple issq isstr istype jacobi join \
	lcm lcmfact lfactor ln lowbit ltol makelist matdim matfill matmax \
	matmin matsum mattrace mattrans max md5 memsize meq min minv mmin \
	mne mod modify name near newerror nextcand nextprime norm null \
	num oldvalue ord param perm pfact pi pix places pmod polar poly \
	pop popcnt power prevcand prevprime printf prompt protect ptest \
	push putenv quo quomod rand randbit random randombit randperm rcin \
	rcmul rcout rcpow rcsq re remove reverse rewind rm root round rsearch \
	runtime saveval scale scan scanf search sec sech seed segment select \
	sgn sha sha1 sin sinh size sizeof sleep sort sqrt srand srandom \
	ssq str strcat strerror strlen strpos strprintf strscan strscanf \
	substr sum swap system tail tan tanh test time trunc version xor

# This list is of files that are clones of DETAIL_HELP files.  They are
# built from DETAIL_HELP files.
#
DETAIL_CLONE= copy

# Singular files
#
# These files are copies of their plural form.
#
PLURAL_FILES= bindings bugs changes errorcodes types
SINGULAR_FILES= binding bug change errorcode type

# These files are found (but not built) in the distribution
#
DISTLIST= ${STD_HELP_FILES} ${DETAIL_HELP} ${MAKE_FILE} \
	obj.file builtin.top builtin.end funclist.sed \
	errorcodes.hdr errorcodes.sed

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST=

all: ${FULL_HELP_FILES} full ${DETAIL_HELP} ${DETAIL_CLONE} \
     ${SINGULAR_FILES} calc .all

# used by the upper level Makefile to determine of we have done all
#
# NOTE: Due to bogus shells found on one common system we must have
#	an non-emoty else clause for every if condition.  *sigh*
#
.all:
	rm -f .all
	touch .all

bindings: ../cal/bindings
	rm -f $@
	cp ../cal/bindings $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

resource: ../cal/README
	rm -f $@
	cp ../cal/README $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

changes: ../CHANGES
	rm -f $@
	cp ../CHANGES $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

libcalc: ../LIBRARY
	rm -f $@
	${SED} -e 's:$${LIBDIR}:${LIBDIR}:g' \
	       -e 's:$${CALC_INCDIR}:${CALC_INCDIR}:g' < ../LIBRARY > $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

bugs: ../BUGS
	rm -f $@
	cp ../BUGS $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

errorcodes: ../calcerr.h errorcodes.hdr errorcodes.sed
	rm -f $@
	cat errorcodes.hdr > $@
	${SED} -n -f errorcodes.sed < ../calcerr.h >> $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

calc: usage
	rm -f $@
	cp usage $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

custom_cal: ../custom/CUSTOM_CAL
	rm -f $@
	cp ../custom/CUSTOM_CAL $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

new_custom: ../custom/HOW_TO_ADD
	rm -f $@
	cp ../custom/HOW_TO_ADD $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

copy: blkcpy
	rm -f $@
	cp blkcpy $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

COPYING: ../COPYING
	rm -f $@
	cp ../COPYING $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

COPYING-LGPL: ../COPYING-LGPL
	rm -f $@
	cp ../COPYING-LGPL $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

cscript: ../cscript/README
	rm -f $@
	cp ../cscript/README $@
	${CHMOD} 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

full: ${FULL_HELP_FILES} ${MAKE_FILE}
	${Q}echo "forming full"
	-${Q}rm -f $@
	-${Q}for i in ${FULL_HELP_FILES}; do \
	    if [ Xintro != X"$$i" ]; then \
		echo ""; \
	    else \
		true; \
	    fi; \
	    if [ Xobj.file = X"$$i" ]; then \
		j=obj; \
	    else \
		j=$$i; \
	    fi; \
	    echo "*************"; \
	    echo "* $$j"; \
	    echo "*************"; \
	    echo ""; \
	    cat $$i; \
	done > $@
	${Q}echo "full formed"
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

# Singular files are the same files as their plural form.
#
${SINGULAR_FILES}: ${PLURAL_FILES}
	${Q}for i in ${SINGULAR_FILES} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo "rm -f $${i}"; \
		rm -f $${i}; \
		echo "cp $${i}s $${i}"; \
		cp $${i}s $${i}; \
	    fi; \
	done
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/SINGULAR_FILES =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

# Form the builtin file
#
# We ave a "chicken-and-egg" problem.  We want the builtn help file to
# accurately reflect the function list.	 It would be nice if we could
# just execute calc show builtin, but calc may not have been built or
# buildable at this point.  The hack-a-round used is to convert ../func.c
# into a standalone program that generates a suitable function list
# that is standwiched between the top and bottom builtin help text.
#
# We form funclist.c by sedding out unwanted stuff from builtins table,
# converting NUMBER* and VALUE into harmless types and converting
# the showbuiltins() function into main().  Combined with the -DFUNCLIST
# we will avoid all of the complex calc types, macros and defines and
# be left with just main() and a mininal builtins table.
#
# Building funclist.o a portable fashion is ugly because some systems
# do not treat -I.. correctly!
#
builtin: builtin.top builtin.end ../func.c funclist.sed
	${Q}echo "forming builtin help file"
	-${Q}rm -f funclist.c
	${Q}${SED} -n -f funclist.sed ../func.c > funclist.c
	-${Q}rm -f funclist.o funclist${EXT}
	${Q}${LCC} ${ICFLAGS} -DFUNCLIST -I/usr/include \
	    -I.. funclist.c -c 2>/dev/null
	${Q}${LCC} ${ILDFLAGS} funclist.o -o funclist${EXT}
	-${Q}rm -f builtin
	${Q}cat builtin.top > builtin
	${Q}./funclist${EXT} | \
	    ${SED} -e 's/^/	/' -e 's/[	 ][	 ]*$$//' >> builtin
	${Q}cat builtin.end >> builtin
	${Q}echo "builtin help file formed"
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

##
#
# File list generation.	 You can ignore this section.
#
#
# We will form the names of source files as if they were in a
# sub-directory called calc/help.
#
##

distlist: ${DISTLIST}
	${Q}for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo help/$$i; \
	    fi; \
	done | LANG=C ${SORT}

distdir:
	${Q}echo help

calcliblist:
	${Q}for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo help/$$i; \
	    fi; \
	done

# The BSDI cdrom makefile expects all help files to be pre-built.  This rule
# creats these fils so that the release can be shipped off to BSDI.  You can
# ignore this rule.
#
bsdi: all
	rm -f obj
	cp obj.file obj

# These next rule help me form the ${DETAIL_HELP} makefile variables above.
#
detaillist:
	${Q}-(echo "xxxxx"; \
	   for i in ${DETAIL_HELP} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		if [ ! -f RCS/$$i,v ]; then \
		    echo "WARNING: $$i not under RCS control" 1>&2; \
		else \
		    echo $$i; \
		fi; \
	    fi; \
	done | LANG=C ${SORT}) | ${FMT} -70 | \
	    ${SED} -e '1s/xxxxx/DETAIL_HELP=/' -e '2,$$s/^/	/' \
		   -e 's/$$/ \\/' -e '$$s/ \\$$//'

##
#
# rpm rules
#
##

echo_inst_files:
	${Q}for i in ${STD_HELP_FILES} full ${BLT_HELP_FILES} \
		  builtin ${DETAIL_HELP} ${SINGULAR_FILES} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${HELPDIR}/$$i; \
	    fi; \
	done
	${Q}echo __file__ ${HELPDIR}/obj

##
#
# Utility rules
#
##

clean:
	rm -f obj mkbuiltin funclist.c funclist.o funclist${EXT}
	rm -f COPYING COPYING-LGPL

clobber:
	rm -f ${BLT_HELP_FILES} full .all calc
	rm -f obj mkbuiltin funclist.c funclist.o funclist${EXT}
	rm -f COPYING COPYING-LGPL
	rm -f ${SINGULAR_FILES} ${DETAIL_CLONE}

install: all
	-${Q}if [ ! -d $T${CALC_SHAREDIR} ]; then \
	    echo mkdir $T${CALC_SHAREDIR}; \
	    mkdir $T${CALC_SHAREDIR}; \
	    if [ ! -d "$T${CALC_SHAREDIR}" ]; then \
		echo mkdir -p "$T${CALC_SHAREDIR}"; \
		mkdir -p "$T${CALC_SHAREDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${CALC_SHAREDIR}; \
	    ${CHMOD} 0755 $T${CALC_SHAREDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${HELPDIR} ]; then \
	    echo mkdir $T${HELPDIR}; \
	    mkdir $T${HELPDIR}; \
	    if [ ! -d "$T${HELPDIR}" ]; then \
		echo mkdir -p "$T${HELPDIR}"; \
		mkdir -p "$T${HELPDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${HELPDIR}; \
	    ${CHMOD} 0755 $T${HELPDIR}; \
	else \
	    true; \
	fi
	-${Q}for i in ${STD_HELP_FILES} ${BLT_HELP_FILES} builtin \
		  full ${DETAIL_HELP} ${SINGULAR_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s $$i $T${HELPDIR}/$$i; then \
		true; \
	    else \
		rm -f $T${HELPDIR}/$$i.new; \
		cp -f $$i $T${HELPDIR}/$$i.new; \
		${CHMOD} 0444 $T${HELPDIR}/$$i.new; \
		mv -f $T${HELPDIR}/$$i.new $T${HELPDIR}/$$i; \
		echo "installed $T${HELPDIR}/$$i"; \
	    fi; \
	done
	-${Q}if ${CMP} -s obj.file $T${HELPDIR}/obj; then \
	    true; \
	else \
	    rm -f $T${HELPDIR}/obj.new; \
	    cp -f obj.file $T${HELPDIR}/obj.new; \
	    ${CHMOD} 0444 $T${HELPDIR}/obj.new; \
	    mv -f $T${HELPDIR}/obj.new $T${HELPDIR}/obj; \
	    echo "installed $T${HELPDIR}/obj"; \
	fi
