#!/bin/make
#
# cal - makefile for calc standard resource files
#
# Copyright (C) 1999-2006,2017,2021,2022  Landon Curt Noll
#
# SRC: cal/Makefile
#
#	The "# SRC: ... - ..." comment line above indicates
#	the origin of this file.
#
# IMPORTANT: Please see the section on Makefiles near the
#	     bottom of the HOWTO.INSTALL file.
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
# Under source code control:	1991/07/21 05:00:54
# File existed as early as:	1991
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-) http://www.isthe.com/chongo/tech/comp/calc/
#
# calculator by David I. Bell with help/mods from others
# Makefile by Landon Curt Noll


# The shell used by this Makefile
#
# On some systems, /bin/sh is a rather reduced shell with
# deprecated behavior.
#
# If your system has a up to date, bash shell, then
# you may wish to use:
#
#	SHELL= /bin/bash
#
# On some systems such as macOS, the bash shell is very
# far behind to the point where is cannot be depended on.
# On such systems, the sh may be a much better alternative
# shell for this Makefile to use:
#
#	SHELL= /bin/sh
#
SHELL= /bin/bash
#SHELL= /bin/sh
#if 0	/* start of skip for non-Gnu makefiles */
ifeq ($(target),Darwin)
SHELL:= /bin/zsh
endif
#endif	/* end of skip for non-Gnu makefiles */


####
# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
####

# PREFIX - Top level location for calc
#
# The PREFIX is often prepended to paths within calc and calc Makefiles.
#
# Starting with calc v2.13.0.1, nearly all Makefile places that used
# /usr/local now use ${PREFIX}.  An exception is the olduninstall rule
# and, of course, this section. :-)
#
# NOTE: The ${PREFIX} is not the same as ${T}.  The ${T} specifies
#	a top level directory under which calc installs things.
#	While usually ${T} is empty, it can be specific path
#	as if calc where "chrooted" during an install.
#	The ${PREFIX} value, during install, is a path between
#	the top level ${T} install directory and the object
#	such as an include file.
#
# NOTE: See also, ${T}, below.
#
# There are some paths that do NOT call under ${PREFIX}, such as
# ${CALCPATH}, that include paths not under ${PREFIX}, but those
# too are exceptions to this general rule.
#
# When in doubt, try:
#
#	PREFIX= /usr/local
#
PREFIX= /usr/local
#PREFIX= /usr
#PREFIX= /usr/global

# Normally certain files depend on the Makefile.  If the Makefile is
# changed, then certain steps should be redone.  If MAKE_FILE is
# set to Makefile, then these files will depend on Makefile.  If
# MAKE_FILE is empty, then they wont.
#
# If in doubt, set MAKE_FILE to Makefile
#
MAKE_FILE= Makefile

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

#INCDIR= ${PREFIX}/include
#INCDIR= /dev/env/DJDIR/include
INCDIR= /usr/include

# where to install calc related things
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
#BINDIR= ${PREFIX}/bin
#BINDIR= /dev/env/DJDIR/bin
BINDIR= /usr/bin

#LIBDIR= ${PREFIX}/lib
#LIBDIR= /dev/env/DJDIR/lib
LIBDIR= /usr/lib

#CALC_SHAREDIR= ${PREFIX}/lib/calc
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
# Q=@	do not echo internal Makefile actions (quiet mode)
# Q=	echo internal Makefile actions (debug / verbose mode)
#
# H=@:	do not report hsrc file formation progress
# H=@	do echo hsrc file formation progress
#
# S= >/dev/null 2>&1	silence ${CC} output during hsrc file formation
# S=			full ${CC} output during hsrc file formation
#
# E= 2>/dev/null	silence command stderr during hsrc file formation
# E=			full command stderr during hsrc file formation
#
# V=@:	do not echo debug statements (quiet mode)
# V=@	do echo debug statements (debug / verbose mode)
#
#Q=
Q=@
#
S= >/dev/null 2>&1
#S=
#
E= 2>/dev/null
#E=
#
#H=@:
H=@
#
V=@:
#V=@

# standard tools
#
CHMOD= chmod
CMP= cmp
CO= co
CP= cp
FMT= fmt
MKDIR= mkdir
MV= mv
RM= rm
RMDIR= rmdir
SED= sed
SORT= sort
TOUCH= touch
TRUE= true

# The calc files to install
#
# This list is produced by the detaillist rule when no WARNINGS are detected.
#
# Please use:
#
#	make calc_files_list
#
# to keep this list in nice sorted order.
#
CALC_FILES= README alg_config.cal beer.cal bernoulli.cal \
	bernpoly.cal bigprime.cal bindings brentsolve.cal chi.cal chrem.cal \
	comma.cal constants.cal deg.cal dms.cal dotest.cal ellip.cal \
	factorial.cal factorial2.cal gvec.cal hello.cal hms.cal infinities.cal \
	intfile.cal intnum.cal lambertw.cal linear.cal lnseries.cal lucas.cal \
	lucas_chk.cal mersenne.cal mfactor.cal mod.cal natnumset.cal \
	palindrome.cal pell.cal pi.cal pix.cal pollard.cal poly.cal prompt.cal \
	psqrt.cal qtime.cal quat.cal randbitrun.cal randmprime.cal \
	randombitrun.cal randomrun.cal randrun.cal regress.cal repeat.cal \
	screen.cal seedrandom.cal set8700.cal set8700.line smallfactors.cal \
	solve.cal specialfunctions.cal statistics.cal strings.cal sumsq.cal \
	sumtimes.cal surd.cal test1700.cal test2300.cal test2600.cal \
	test2700.cal test3100.cal test3300.cal test3400.cal test3500.cal \
	test4000.cal test4100.cal test4600.cal test5100.cal test5200.cal \
	test8400.cal test8500.cal test8600.cal test8900.cal toomcook.cal \
	unitfrac.cal varargs.cal xx_print.cal zeta2.cal

# These calc files are now obsolete and are removed by the install rule.
#
DEAD_CALC_FILES= lucas_tbl.cal

# These files are found (but not built) in the distribution
#
DISTLIST= ${CALC_FILES} ${MAKE_FILE}

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST=
#
# rules that are not also names of files
#
PHONY= all clobber distlist install

all: ${CALC_FILES} ${MAKE_FILE} .all

# used by the upper level Makefile to determine of we have done all
#
.all:
	${RM} -f .all
	${TOUCH} .all

.PHONY: ${PHONY}

##
#
# File list generation.	 You can ignore this section.
#
#
# We will form the names of source files as if they were in a
# sub-directory called calc/cal.
#
# NOTE: Due to bogus shells found on one common system we must have
#	an non-empty else clause for every if condition.  *sigh*
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
		echo $$i; \
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
	-${Q} if [ -e .DS_Store ]; then \
	    echo ${RM} -rf .DS_Store; \
	    ${RM} -rf .DS_Store; \
	fi
	-${Q} for i in ${DEAD_CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -e "${T}${CALC_SHAREDIR}/$$i" ]; then \
		echo "${RM} -f ${T}${CALC_SHAREDIR}/$$i"; \
		${RM} -f ${T}${CALC_SHAREDIR}/$$i; \
	    fi; \
	    if [ -e "./$$i" ]; then \
		echo "${RM} -f ./$$i"; \
		${RM} -f ./$$i; \
	    fi; \
	done

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
	${Q} for i in ${DEAD_CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -e "${T}${CALC_SHAREDIR}/$$i" ]; then \
		echo "${RM} -f ${T}${CALC_SHAREDIR}/$$i"; \
		${RM} -f ${T}${CALC_SHAREDIR}/$$i; \
	    fi; \
	    if [ -e "./$$i" ]; then \
		echo "${RM} -f ./$$i"; \
		${RM} -f ./$$i; \
	    fi; \
	done

# Try to remove everything that was installed
#
# NOTE: Keep the uninstall rule in reverse order to the install rule
#
uninstall:
	- ${Q} for i in ${DEAD_CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -e "${T}${CALC_SHAREDIR}/$$i" ]; then \
		echo "${RM} -f ${T}${CALC_SHAREDIR}/$$i"; \
		${RM} -f ${T}${CALC_SHAREDIR}/$$i; \
	    fi; \
	    if [ -e "./$$i" ]; then \
		echo "${RM} -f ./$$i"; \
		${RM} -f ./$$i; \
	    fi; \
	done
	-${Q} for i in ${CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -f "${T}${CALC_SHAREDIR}/$$i" ]; then \
		${RM} -f "${T}${CALC_SHAREDIR}/$$i"; \
		if [ -f "${T}${CALC_SHAREDIR}/$$i" ]; then \
		   echo "cannot uninstall ${T}${CALC_SHAREDIR}/$$i"; \
		else \
		   echo "un-installed ${T}${CALC_SHAREDIR}/$$i"; \
		fi; \
	    fi; \
	done
	-${Q} for i in ${CALC_SHAREDIR}; do \
	    if [ -d "${T}$$i" ]; then \
		${RMDIR} "${T}$$i" 2>/dev/null; \
		echo "cleaned up ${T}$$i"; \
	    fi; \
	done
