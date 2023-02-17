#!/usr/bin/env make
#
# cscript - makefile for calc shell script files
#
# Copyright (C) 1999-2006,2014,2017,2021-2023  Landon Curt Noll
#
# SRC: cscript/Makefile
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
# Under source code control:	1999/11/29 11:10:26
# File existed as early as:	1999
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
#	SHELL= bash
#
# On some systems such as macOS, the bash shell is very
# far behind to the point where is cannot be depended on.
# On such systems, the sh may be a much better alternative
# shell for this Makefile to use:
#
#	SHELL= sh
#
SHELL= bash
#SHELL= sh
#if 0	/* start of skip for non-Gnu makefiles */
#
ifeq ($(target),Darwin)
SHELL:= /bin/sh
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
CAT= cat
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

# NOTE: On some shells, echo is a builtin that does
#	not understand -n, so we call /bin/echo -n
#	directly to get around such shells.
#
ECHON= /bin/echo -n

# The ${SCRIPT} list is the list of calc shell script files (without the .calc
# extension) which will be installed.
#
# The ${SCRIPT_SRC} is built from ${SCRIPT} and has the .calc extensions.
#
# This list is produced by the detaillist rule when no WARNINGS are detected.
# To add a script:
#
#	1) Name the file with a .calc filename extension
#	2) Add the name, without the .calc extension to the ${SCRIPT} below
#	3) Write out this Makefile
#	4) Replace the SCRIPT= and SCRIPT_SRC= lines with the output of:
#
#		make detaillist
#
SCRIPT= 4dsphere fproduct mersenne piforever plus powerterm simple \
	square

SCRIPT_SRC= 4dsphere.calc fproduct.calc mersenne.calc piforever.calc \
	plus.calc powerterm.calc simple.calc square.calc

# These files are found (but not built) in the distribution
#
DISTLIST= ${SCRIPT_SRC} ${MAKE_FILE} README.src

# These files are used to make (but not build) a calc .a library
#
CALCLIBLIST=

# complete list of targets
#
# NOTE: This list MUST be coordinated with the ${CSCRIPT_TARGETS} variable
#	in the upper level ../Makefile
#
CSCRIPT_TARGETS= ${SCRIPT}
TARGETS= ${CSCRIPT_TARGETS} README
#
# rules that are not also names of files
#
PHONY= all clobber distlist install

# The reason for this Makefile
#
all: ${TARGETS} .all

.PHONY: ${PHONY}

README: README.src
	@${RM} -f $@
	@${SED} -e "s:#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@

# used by the upper level Makefile to determine if we have done all
#
# NOTE: Due to bogus shells found on one common system we must have
#	a non-empty else clause for every if condition.  *sigh*
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
# sub-directory called calc/cscript.
#
##

distlist: ${DISTLIST}
	${Q} for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo cscript/$$i; \
	    fi; \
	done | LANG=C ${SORT}

distdir:
	${Q} echo cscript

calcliblist:

# These next rule help me form the ${SCRIPT} and ${SCRIPT_SRC} makefile
# variables above.
#
detaillist:
	${Q} -(echo "xxxxxxx"; \
	   for i in ${SCRIPT} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i; \
	    fi; \
	done | LANG=C ${SORT}) | ${FMT} -70 | \
	    ${SED} -e '1s/xxxxxxx/SCRIPT=/' -e '2,$$s/^/	/' \
		   -e 's/$$/ \\/' -e '$$s/ \\$$//'
	${Q} echo
	${Q} -(echo "xxxxxxxxxxx"; \
	    for i in ${SCRIPT} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i.calc; \
	    fi; \
	done | LANG=C ${SORT}) | ${FMT} -70 | \
	    ${SED} -e '1s/xxxxxxxxxxx/SCRIPT_SRC=/' -e '2,$$s/^/	/' \
		   -e 's/$$/ \\/' -e '$$s/ \\$$//'

##
#
# Home grown make dependency rules.  Your system make not support
# or have the needed tools.  You can ignore this section.
#
# We will form a skeleton tree of *.c files containing only #include "foo.h"
# lines and .h files containing the same lines surrounded by multiple include
# prevention lines.  This allows us to build a static depend list that will
# satisfy all possible cpp symbol definition combinations.
#
##

depend:
	${Q} if [ -f ${MAKE_FILE}.bak ]; then \
	    echo "${MAKE_FILE}.bak exists, remove or move it"; \
	    exit 1; \
	else \
	    ${TRUE}; \
	fi
	${Q} ${RM} -f makedep.out
	${Q} echo forming cscript dependency list
	${Q} for i in ${SCRIPT} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo "$$i: $$i.calc"; \
		echo '	@$${RM} -f $$@'; \
		${ECHON} '	@$${SED} -e "1s:^#!/usr/local/src/bin/'; \
		echo 'calc/calc:#!$${BINDIR}/calc:" $$?>$$@'; \
		echo '	@$${CHMOD} +x $$@'; \
	    fi; \
	done >> makedep.out
	${Q} echo sample dependency list formed
	${Q} echo forming new cscript/${MAKE_FILE}
	${Q} ${RM} -f ${MAKE_FILE}.bak
	${Q} ${MV} ${MAKE_FILE} ${MAKE_FILE}.bak
	${Q} ${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' \
		    ${MAKE_FILE}.bak > ${MAKE_FILE}
	${Q} echo "" >> ${MAKE_FILE}
	${Q} ${CAT} makedep.out >> ${MAKE_FILE}
	${Q} ${RM} -f makedep.out
	-${Q} if ${CMP} -s ${MAKE_FILE}.bak ${MAKE_FILE}; then \
	    echo 'cscript/${MAKE_FILE} was already up to date'; \
	    echo 'restoring original cscript/${MAKE_FILE}'; \
	    ${MV} -f ${MAKE_FILE}.bak ${MAKE_FILE}; \
	else \
	    echo 'old cscript/${MAKE_FILE} is now cscript/${MAKE_FILE}.bak'; \
	    echo 'new cscript/${MAKE_FILE} formed'; \
	    echo 'try: diff -u cscript/${MAKE_FILE}.bak cscript/${MAKE_FILE}'; \
	fi

##
#
# rpm rules
#
##

echo_inst_files:
	${Q} for i in ${SCRIPT} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${SCRIPTDIR}/$$i; \
	    fi; \
	done

##
#
# Utility rules
#
##

clean:
	${RM} -f makedep.out

clobber: clean
	${RM} -f .all
	${RM} -f ${TARGETS}
	-${Q} if [ -e .DS_Store ]; then \
	    echo ${RM} -rf .DS_Store; \
	    ${RM} -rf .DS_Store; \
	fi

# install everything
#
# NOTE: Keep the uninstall rule in reverse order to the install rule
#
install: all
	-${Q} if [ ! -d ${T}${BINDIR} ]; then \
	    echo ${MKDIR} -p ${T}${BINDIR}; \
	    ${MKDIR} -p ${T}${BINDIR}; \
	    if [ ! -d "${T}${BINDIR}" ]; then \
		echo ${MKDIR} -p "${T}${BINDIR}"; \
		${MKDIR} -p "${T}${BINDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${BINDIR}; \
	    ${CHMOD} 0755 ${T}${BINDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${SCRIPTDIR} ]; then \
	    echo ${MKDIR} -p ${T}${SCRIPTDIR}; \
	    ${MKDIR} -p ${T}${SCRIPTDIR}; \
	    if [ ! -d "${T}${SCRIPTDIR}" ]; then \
		echo ${MKDIR} -p "${T}${SCRIPTDIR}"; \
		${MKDIR} -p "${T}${SCRIPTDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${SCRIPTDIR}; \
	    ${CHMOD} 0755 ${T}${SCRIPTDIR}; \
	else \
	    ${TRUE}; \
	fi
	${Q} for i in ${SCRIPT} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s $$i ${T}${SCRIPTDIR}/$$i; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${SCRIPTDIR}/$$i.new; \
		${CP} -f $$i ${T}${SCRIPTDIR}/$$i.new; \
		${CHMOD} 0755 ${T}${SCRIPTDIR}/$$i.new; \
		${MV} -f ${T}${SCRIPTDIR}/$$i.new ${T}${SCRIPTDIR}/$$i; \
		echo "installed ${T}${SCRIPTDIR}/$$i"; \
	    fi; \
	done

# Try to remove everything that was installed
#
# NOTE: Keep the uninstall rule in reverse order to the install rule
#
uninstall:
	-${Q} for i in ${SCRIPT} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -f "${T}${SCRIPTDIR}/$$i" ]; then \
		${RM} -f "${T}${SCRIPTDIR}/$$i"; \
		if [ -f "${T}${SCRIPTDIR}/$$i" ]; then \
		   echo "cannot uninstall ${T}${SCRIPTDIR}/$$i"; \
		else \
		   echo "uninstalled ${T}${SCRIPTDIR}/$$i"; \
		fi; \
	    fi; \
	done
	-${Q} for i in ${SCRIPTDIR} ${BINDIR}; do \
	    if [ -d "${T}$$i" ]; then \
		${RMDIR} "${T}$$i" 2>/dev/null; \
		echo "cleaned up ${T}$$i"; \
	    fi; \
	done

# DO NOT DELETE THIS LINE -- make depend depends on it.

4dsphere: 4dsphere.calc
	@${RM} -f $@
	@${SED} -e "1s:^#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@
	@${CHMOD} +x $@
fproduct: fproduct.calc
	@${RM} -f $@
	@${SED} -e "1s:^#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@
	@${CHMOD} +x $@
mersenne: mersenne.calc
	@${RM} -f $@
	@${SED} -e "1s:^#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@
	@${CHMOD} +x $@
piforever: piforever.calc
	@${RM} -f $@
	@${SED} -e "1s:^#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@
	@${CHMOD} +x $@
plus: plus.calc
	@${RM} -f $@
	@${SED} -e "1s:^#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@
	@${CHMOD} +x $@
powerterm: powerterm.calc
	@${RM} -f $@
	@${SED} -e "1s:^#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@
	@${CHMOD} +x $@
simple: simple.calc
	@${RM} -f $@
	@${SED} -e "1s:^#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@
	@${CHMOD} +x $@
square: square.calc
	@${RM} -f $@
	@${SED} -e "1s:^#!/usr/local/src/bin/calc/calc:#!${BINDIR}/calc:" $?>$@
	@${CHMOD} +x $@
