#!/bin/make
#
# cal - makefile for calc standard resource files
#
# Copyright (C) 1999-2006  Landon Curt Noll
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
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# @(#) $Revision: 30.11 $
# @(#) $Id: Makefile,v 30.11 2014/09/02 07:14:49 chongo Exp $
# @(#) $Source: /usr/local/src/bin/calc/cal/RCS/Makefile,v $
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
SHELL= /bin/sh

####
# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
####

# Normally certain files depend on the Makefile.  If the Makefile is
# changed, then certain steps should be redone.  If MAKE_FILE is
# set to Makefile, then these files will depend on Makefile.  If
# MAKE_FILE is empty, then they wont.
#
# If in doubt, set MAKE_FILE to Makefile
#
MAKE_FILE= Makefile

# Controlling file makefile basename (without the path)
#
# This is the basename same of the makefile that may/does/will drive
# this makefile.
#
# If in doubt, set TOP_MAKE_FILE to Makefile
#
TOP_MAKE_FILE= Makefile

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
# NOTE: The install rule prepends installation paths with ${T}, which
#	by default is empty.  If ${T} is non-empty, then installation
#	locations will be relative to the ${T} directory.
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
# ${SCRIPTDIR}		where calc shell scripts are installed
#
# NOTE: The install rule prepends installation paths with ${T}, which
#	by default is empty.  If ${T} is non-empty, then installation
#	locations will be relative to the ${T} directory.
#
# If in doubt, set:
#
#	HELPDIR= ${CALC_SHAREDIR}/help
#	CALC_INCDIR= ${INCDIR}/calc
#	SCRIPTDIR= ${BINDIR}/cscript
#
HELPDIR= ${CALC_SHAREDIR}/help
CALC_INCDIR= ${INCDIR}/calc
SCRIPTDIR= ${BINDIR}/cscript

# T - top level directory under which calc will be installed
#
# The calc install is performed under ${T}, the calc build is
# performed under /.	The purpose for ${T} is to allow someone
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
# If ${T} is empty, calc is installed under /, which is the same
# top of tree for which it was built.  If ${T} is non-empty, then
# calc is installed under ${T}, as if one had to chroot under
# ${T} for calc to operate.
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
RM= rm
MKDIR= mkdir
RMDIR= rmdir
CP= cp
MV= mv
CO= co
TRUE= true
TOUCH= touch
SED= sed
SORT= sort
FMT= fmt

# The calc files to install
#
# This list is prodiced by the detaillist rule when no WARNINGS are detected.
#
# Please use:
#
#	make calc_files_list
#
# to keep this list in nice sorted order and to check that these
# deailed help files are under RCS control.
#
CALC_FILES= README alg_config.cal beer.cal bernoulli.cal \
	bernpoly.cal bigprime.cal bindings brentsolve.cal chi.cal chrem.cal \
	constants.cal deg.cal dms.cal dotest.cal ellip.cal factorial.cal \
	factorial2.cal gvec.cal hello.cal hms.cal infinities.cal \
	intfile.cal intnum.cal lambertw.cal linear.cal lnseries.cal \
	lucas.cal lucas_chk.cal lucas_tbl.cal mersenne.cal mfactor.cal \
	mod.cal natnumset.cal pell.cal pi.cal pix.cal pollard.cal poly.cal \
	prompt.cal psqrt.cal qtime.cal quat.cal randbitrun.cal randmprime.cal \
	randombitrun.cal randomrun.cal randrun.cal regress.cal repeat.cal \
	screen.cal seedrandom.cal set8700.cal set8700.line smallfactors.cal \
	solve.cal specialfunctions.cal statistics.cal strings.cal sumsq.cal \
	sumtimes.cal surd.cal test1700.cal test2300.cal test2600.cal \
	test2700.cal test3100.cal test3300.cal test3400.cal test3500.cal \
	test4000.cal test4100.cal test4600.cal test5100.cal test5200.cal \
	test8400.cal test8500.cal test8600.cal test8900.cal toomcook.cal \
	unitfrac.cal varargs.cal xx_print.cal zeta2.cal

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
	${RM} -f .all
	${TOUCH} .all

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
	${Q} for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo cal/$$i; \
	    fi; \
	done

distdir:
	${Q} echo cal

calcliblist:
	${Q} for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo cal/$$i; \
	    fi; \
	done

# These next rule help form the ${CALC_FILES} makefile variables above.
#
calc_files_list:
	${Q} -(find .  -mindepth 1 -maxdepth 1 -type f -name '*.cal' -print | \
	   while read i; do \
		if [ X"$$i" != X"/dev/null" ]; then \
		    if [ ! -f RCS/$$i,v ]; then \
			echo "WARNING: $$i not under RCS control" 1>&2; \
		    else \
			echo $$i; \
		    fi; \
		fi; \
	done; \
	echo '--first_line--'; \
	echo README; \
	echo set8700.line; \
	echo bindings) | \
	    ${SED} -e 's:^\./::' | LANG=C ${SORT} | ${FMT} -70 | \
	    ${SED} -e '1s/--first_line--/CALC_FILES=/' -e '2,$$s/^/	/' \
		   -e 's/$$/ \\/' -e '$$s/ \\$$//'

##
#
# rpm rules
#
##

echo_inst_files:
	${Q} for i in ${CALC_FILES} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CALC_SHAREDIR}/$$i; \
	    fi; \
	done

##
#
# Utility rules
#
##

clean:

clobber: clean
	${RM} -f .all

# install everything
#
# NOTE: Keep the uninstall rule in reverse order to the install rule
#
install: all
	-${Q} if [ ! -d ${T}${CALC_SHAREDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CALC_SHAREDIR}; \
	    ${MKDIR} -p ${T}${CALC_SHAREDIR}; \
	    if [ ! -d "${T}${CALC_SHAREDIR}" ]; then \
		echo ${MKDIR} -p "${T}${CALC_SHAREDIR}"; \
		${MKDIR} -p "${T}${CALC_SHAREDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${CALC_SHAREDIR}; \
	    ${CHMOD} 0755 ${T}${CALC_SHAREDIR}; \
	else \
	    ${TRUE}; \
	fi
	${Q} for i in ${CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s $$i ${T}${CALC_SHAREDIR}/$$i; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${CALC_SHAREDIR}/$$i.new; \
		${CP} -f $$i ${T}${CALC_SHAREDIR}/$$i.new; \
		${CHMOD} 0444 ${T}${CALC_SHAREDIR}/$$i.new; \
		${MV} -f ${T}${CALC_SHAREDIR}/$$i.new ${T}${CALC_SHAREDIR}/$$i;\
		echo "installed ${T}${CALC_SHAREDIR}/$$i"; \
	    fi; \
	done

# Try to remove everything that was installed
#
# NOTE: Keep the uninstall rule in reverse order to the install rule
#
uninstall:
	-${Q} for i in ${CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -f "${T}${CALC_SHAREDIR}/$$i" ]; then \
	    	${RM} -f "${T}${CALC_SHAREDIR}/$$i"; \
		if [ -f "${T}${CALC_SHAREDIR}/$$i" ]; then \
		   echo "cannot uninstall ${T}${CALC_SHAREDIR}/$$i"; \
		else \
		   echo "uninstalled ${T}${CALC_SHAREDIR}/$$i"; \
		fi; \
	    fi; \
	done
	-${Q} for i in ${CALC_SHAREDIR}; do \
	    if [ -d "${T}$$i" ]; then \
		${RMDIR} "${T}$$i" 2>/dev/null; \
		echo "cleaned up ${T}$$i"; \
	    fi; \
	done
