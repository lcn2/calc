#
# cscript - makefile for calc shell script files
#
# Copyright (C) 1999  Landon Curt Noll
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
# @(#) $Revision: 29.6 $
# @(#) $Id: Makefile,v 29.6 2000/12/15 14:56:14 chongo Exp $
# @(#) $Source: /usr/local/src/cmd/calc/cscript/RCS/Makefile,v $
#
# Under source code control:	1999/11/29 11:10:26
# File existed as early as:	1999
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

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
BINDIR= /usr/local/bin
#BINDIR= /usr/bin
#BINDIR= /usr/contrib/bin
#
SCRIPTDIR= ${BINDIR}/cscript

# Makefile debug
#
# Q=@	do not echo internal makefile actions (quiet mode)
# Q=	echo internal makefile actions (debug / verbose mode)
#
#Q=
Q=@

# standard tools
#
CHMOD= chmod
SED= sed
SORT= sort
FMT= fmt

# The ${SCRIPT} list is the list of calc shell script files (without the .calc
# extension) which will be installed.
#
# The ${SCRIPT_SRC} is built from ${SCRIPT} and has the .calc extensions.
#
# This list is produced by the detaillist rule when no WARNINGS are detected.
# To add a script:
#
#	1) Name the file with a .calc filename extension
#	2) Place that file under RCS control
#	3) Add the name, without the .calc extension to the ${SCRIPT} below
#	4) Write out this Makefile
#	5) Replace the ${SCRIPT} and ${SCRIPT_SRC} lines with the output of:
#
#		make detaillist
#
SCRIPT= mersenne piforever plus simple square

SCRIPT_SRC= mersenne.calc piforever.calc plus.calc simple.calc square.calc

# These files are found (but not built) in the distribution
#
DISTLIST= ${SCRIPT_SRC} ${MAKE_FILE} README

# These files are used to make (but not build) a calc .a library
#
CALCLIBLIST=

# The reason for this Makefile
#
all: ${SCRIPT} ${SCRIPT_SRC} .all

# used by the upper level Makefile to determine if we have done all
#
# NOTE: Due to bogus shells found on one common system we must have
#	a non-empty else clause for every if condition.  *sigh*
#
.all:
	rm -f .all
	touch .all

##
#
# File list generation.	 You can ignore this section.
#
#
# We will form the names of source files as if they were in a
# sub-directory called calc/cscript.
#
##

distlist: ${DISTLIST}
	${Q}for i in ${DISTLIST}; do \
		echo cscript/$$i; \
	done | ${SORT}

distdir:
	${Q}echo cscript

calcliblist:

# These next rule help me form the ${DETAIL_HELP} makefile variables above.
#
detaillist:
	${Q}-(echo "xxxxxxx"; \
	   for i in ${SCRIPT}; do \
		if [ ! -f RCS/$$i.calc,v ]; then \
		    echo "WARNING: $$i.calc not under RCS control" 1>&2; \
		else \
		    echo $$i; \
		fi; \
	done | ${SORT}) | ${FMT} -70 | \
	    ${SED} -e '1s/xxxxxxx/SCRIPT=/' -e '2,$$s/^/	/' \
		   -e 's/$$/ \\/' -e '$$s/ \\$$//'
	${Q}echo
	${Q}-(echo "xxxxxxxxxxx"; \
	   for i in ${SCRIPT}; do \
		if [ ! -f RCS/$$i.calc,v ]; then \
		    echo "WARNING: $$i.calc not under RCS control" 1>&2; \
		else \
		    echo $$i.calc; \
		fi; \
	done | ${SORT}) | ${FMT} -70 | \
	    ${SED} -e '1s/xxxxxxxxxxx/SCRIPT_SRC=/' -e '2,$$s/^/	/' \
		   -e 's/$$/ \\/' -e '$$s/ \\$$//'

##
#
# Home grown make dependency rules.  Your system make not support
# or have the needed tools.  You can ignore this section.
#
# We will form a skelaton tree of *.c files containing only #include "foo.h"
# lines and .h files containing the same lines surrounded by multiple include
# prevention lines.  This allows us to build a static depend list that will
# satisfy all possible cpp symbol definition combinations.
#
##

depend:
	${Q}if [ -f Makefile.bak ]; then \
		echo "Makefile.bak exists, remove or move it out of the way"; \
		exit 1; \
	else \
	    true; \
	fi
	-${Q}rm -f makedep.out
	${Q}echo forming cscript dependency list
	${Q}echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > \
		makedep.out
	${Q}echo "" >> makedep.out
	${Q}for i in ${SCRIPT}; do \
		echo "$$i: $$i.calc"; \
		echo '	rm -f $$@'; \
		echo '	$${SED} -e "1s:^#!/usr/local/src/cmd/calc/calc:#!$${BINDIR}/calc:" $$?>$$@'; \
		echo '	$${CHMOD} +x $$@'; \
	done >> makedep.out
	${Q}echo sample dependency list formed
	${Q}echo forming new cscript/Makefile
	-${Q}rm -f Makefile.bak
	${Q}mv Makefile Makefile.bak
	${Q}${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' Makefile.bak > Makefile
	${Q}echo "" >> Makefile
	${Q}${SED} -n '3,$$p' makedep.out >> Makefile
	-${Q}rm -f makedep.out
	-${Q}if cmp -s Makefile.bak Makefile; then \
		echo 'sample Makefile was already up to date'; \
		mv -f Makefile.bak Makefile; \
	else \
		rm -f Makefile.tmp; \
		mv Makefile Makefile.tmp; \
		if [ -d RCS ]; then \
		    co -l Makefile; \
		fi; \
		mv Makefile.tmp Makefile; \
		if [ -d RCS ]; then \
		    echo new sample Makefile formed, you need to check it in; \
		fi; \
	fi

clean:
	-rm -f makedep.out

clobber:
	-rm -f ${SCRIPT}

install: all
	-${Q}if [ ! -d ${BINDIR} ]; then \
		echo mkdir ${BINDIR}; \
		mkdir ${BINDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d ${SCRIPTDIR} ]; then \
		echo mkdir ${SCRIPTDIR}; \
		mkdir ${SCRIPTDIR}; \
	else \
	    true; \
	fi
	${Q}for i in ${SCRIPT}; do \
		echo rm -f ${SCRIPTDIR}/$$i; \
		rm -f ${SCRIPTDIR}/$$i; \
		echo cp $$i ${SCRIPTDIR}; \
		cp $$i ${SCRIPTDIR}; \
		echo ${CHMOD} 0555 ${SCRIPTDIR}/$$i; \
		${CHMOD} 0555 ${SCRIPTDIR}/$$i; \
	done

# DO NOT DELETE THIS LINE -- make depend depends on it.

mersenne: mersenne.calc
	rm -f $@
	${SED} -e "1s:^#!/usr/local/src/cmd/calc/calc:#!${BINDIR}/calc:" $?>$@
	${CHMOD} +x $@
piforever: piforever.calc
	rm -f $@
	${SED} -e "1s:^#!/usr/local/src/cmd/calc/calc:#!${BINDIR}/calc:" $?>$@
	${CHMOD} +x $@
plus: plus.calc
	rm -f $@
	${SED} -e "1s:^#!/usr/local/src/cmd/calc/calc:#!${BINDIR}/calc:" $?>$@
	${CHMOD} +x $@
simple: simple.calc
	rm -f $@
	${SED} -e "1s:^#!/usr/local/src/cmd/calc/calc:#!${BINDIR}/calc:" $?>$@
	${CHMOD} +x $@
square: square.calc
	rm -f $@
	${SED} -e "1s:^#!/usr/local/src/cmd/calc/calc:#!${BINDIR}/calc:" $?>$@
	${CHMOD} +x $@
