#!/bin/make
#
# cal - makefile for calc standard resource files
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
# @(#) $Revision: 29.13 $
# @(#) $Id: Makefile,v 29.13 2002/03/12 08:32:35 chongo Exp $
# @(#) $Source: /usr/local/src/cmd/calc/cal/RCS/Makefile,v $
#
# Under source code control:	1991/07/21 05:00:54
# File existed as early as:	1991
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-) http://www.isthe.com/chongo/tech/comp/calc/
#
# calculator by David I. Bell with help/mods from others
# Makefile by Landon Curt Noll

# required vars
#
SHELL = /bin/sh
MAKE_FILE = Makefile

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# where to install things
#SHAREDIR= /usr/local/lib
#SHAREDIR= /usr/lib
SHAREDIR= /usr/share
#SHAREDIR= /usr/libdata

CSHAREDIR= ${SHAREDIR}/calc

# T - top level directory under which calc will be installed
#
# The calc install is performed under $T, the calc build is
# performed under /.  The purpose for $T is to allow someone to
# install calc somewhere other than into the system area.  For example
# when forming the calc rpm, the Makefile is called with T=$RPM_BUILD_ROOT.
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
CHMOD= chmod
CMP= cmp

# The calc files to install
#
CALC_FILES= README bigprime.cal deg.cal ellip.cal lucas.cal lucas_chk.cal \
	lucas_tbl.cal mersenne.cal mod.cal pell.cal pi.cal pix.cal \
	pollard.cal poly.cal psqrt.cal quat.cal regress.cal solve.cal \
	sumsq.cal surd.cal unitfrac.cal varargs.cal chrem.cal mfactor.cal \
	bindings randmprime.cal test1700.cal randrun.cal \
	randbitrun.cal bernoulli.cal test2300.cal test2600.cal \
	test2700.cal test3100.cal test3300.cal test3400.cal prompt.cal \
	test3500.cal seedrandom.cal test4000.cal test4100.cal test4600.cal \
	beer.cal hello.cal test5100.cal test5200.cal randombitrun.cal \
	randomrun.cal xx_print.cal natnumset.cal qtime.cal test8400.cal \
	test8500.cal test8600.cal chi.cal intfile.cal

# These files are found (but not built) in the distribution
#
DISTLIST= ${CALC_FILES} ${MAKE_FILE}

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST=

all: ${CALC_FILES} ${MAKE_FILE} .all

# used by the upper level Makefile to determine of we have done all
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
# sub-directory called calc/cal.
#
# NOTE: Due to bogus shells found on one common system we must have
#	an non-emoty else clause for every if condition.  *sigh*
#
##

distlist: ${DISTLIST}
	${Q}for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo cal/$$i; \
	    fi; \
	done

distdir:
	${Q}echo cal

calcliblist:
	${Q}for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo cal/$$i; \
	    fi; \
	done

##
#
# rpm rules
#
##

echo_inst_files:
	${Q}for i in ${CALC_FILES} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CSHAREDIR}/$$i; \
	    fi; \
	done

##
#
# Utility rules
#
##

clean:

clobber:
	rm -f .all

install: all
	-${Q}if [ ! -d $T${SHAREDIR} ]; then \
	    echo mkdir $T${SHAREDIR}; \
	    mkdir $T${SHAREDIR}; \
	    if [ ! -d "$T${SHAREDIR}" ]; then \
		echo mkdir -p "$T${SHAREDIR}"; \
		mkdir -p "$T${SHAREDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${SHAREDIR}; \
	    ${CHMOD} 0755 $T${SHAREDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${CSHAREDIR} ]; then \
	    echo mkdir $T${CSHAREDIR}; \
	    mkdir $T${CSHAREDIR}; \
	    if [ ! -d "$T${CSHAREDIR}" ]; then \
		echo mkdir -p "$T${CSHAREDIR}"; \
		mkdir -p "$T${CSHAREDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${CSHAREDIR}; \
	    ${CHMOD} 0755 $T${CSHAREDIR}; \
	else \
	    true; \
	fi
	${Q}for i in ${CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s $$i $T${CSHAREDIR}/$$i; then \
		true; \
	    else \
		rm -f $T${CSHAREDIR}/$$i.new; \
		cp -f $$i $T${CSHAREDIR}/$$i.new; \
		${CHMOD} 0444 $T${CSHAREDIR}/$$i.new; \
		mv -f $T${CSHAREDIR}/$$i.new $T${CSHAREDIR}/$$i; \
		echo "installed $T${CSHAREDIR}/$$i"; \
	    fi; \
	done
