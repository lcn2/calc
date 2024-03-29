#!/usr/bin/env make
#
# cal - makefile for calc standard resource files
#
# Copyright (C) 1999-2006,2017,2021-2023  Landon Curt Noll
#
# Suggestion: Read the ../HOWTO.INSTALL file.
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
# This calculator first developed by David I. Bell with help/mods from others.
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-)		http://www.isthe.com/chongo/tech/comp/calc/


# SUGGESTION: Instead of modifying this file, consider adding
#	      statements to modify, replace or append Makefile
#	      variables in the ../Makefile.local file.


###########################################
# Files used or included by this Makefile #
###########################################

# Normally certain files depend on the Makefile.  If the Makefile is
# changed, then certain steps should be redone.  If MAKE_FILE is
# set to Makefile, then these files will depend on Makefile.  If
# MAKE_FILE is empty, then they won't.
#
# If in doubt, set MAKE_FILE to Makefile
#
MAKE_FILE= Makefile

# Calc configuration and compile configuration values
#
CONFIG_MKF= ../Makefile.config

# Host target information.
#
TARGET_MKF= ../Makefile.target

# Local file that is included just prior to the first rule,
# that allows one to override any values set in this Makefile.
#
LOCAL_MKF= ../Makefile.local

# The set of Makefiles
#
MK_SET= ${MAKE_FILE} ${CONFIG_MKF} ${TARGET_MKF} ${LOCAL_MKF}


#######################################################
# Calc configuration and compile configuration values #
#######################################################
include ${CONFIG_MKF}


###############################
# host target section include #
###############################
include ${TARGET_MKF}


##########################################################################
#=-=-=-=-=- Be careful if you change something below this line -=-=-=-=-=#
##########################################################################


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

# The calc files to install
#
# This list is produced by the detaillist rule when no WARNINGS are detected.
#
# Please use:
#
#	make clobber >/dev/null && make calc_files_list
#
# to keep this list in nice sorted order.
#
CALC_FILES= README alg_config.cal beer.cal bernoulli.cal \
	bernpoly.cal bigprime.cal bindings brentsolve.cal chi.cal chrem.cal \
	comma.cal constants.cal deg.cal dms.cal dotest.cal ellip.cal \
	factorial.cal factorial2.cal fnv_tool.cal gvec.cal hello.cal hms.cal \
	infinities.cal intfile.cal intnum.cal lambertw.cal linear.cal \
	lnseries.cal lucas.cal lucas_chk.cal mersenne.cal mfactor.cal mod.cal \
	natnumset.cal palindrome.cal pell.cal pi.cal pix.cal pollard.cal \
	poly.cal prompt.cal psqrt.cal qtime.cal quat.cal randbitrun.cal \
	randmprime.cal randombitrun.cal randomrun.cal randrun.cal regress.cal \
	repeat.cal screen.cal seedrandom.cal set8700.line smallfactors.cal \
	solve.cal specialfunctions.cal splitbits.cal statistics.cal \
	strings.cal sumsq.cal sumtimes.cal surd.cal test2300.obj_incdec.cal \
	test2600.numfunc.cal test2700.isqrt.cal test3100.matobj.cal \
	test3300.det.cal test3400.trig.cal test4000.ptest.cal \
	test4100.redc.cal test4600.fileop.cal test5100.newdecl.cal \
	test5200.globstat.cal test8000.read.cal test8400.quit.cal \
	test8500.divmod.cal test8600.maxargs.cal test8700.dotest.cal \
	test8900.special.cal test9300.frem.cal test9500.trigeq.cal \
	toomcook.cal unitfrac.cal varargs.cal write2file.cal xx_print.cal \
	zeta2.cal

# These calc files are now obsolete and are removed by the install rule.
#
DEAD_CALC_FILES= lucas_tbl.cal set8700.cal test1700.cal test2300.cal test2600.cal \
	test2700.cal test3100.cal test3300.cal test3400.cal test3500.cal test4000.cal \
	test4100.cal test4600.cal test5100.cal test5200.cal test8400.cal test8500.cal \
	test8600.cal test8900.cal

# These files are found (but not built) in the distribution
#
DISTLIST= ${CALC_FILES} ${MAKE_FILE}

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST=

# rules that are not also names of files
#
PHONY= all distlist buildlist distdir calcliblist calc_files_list echo_inst_files \
	clean clobber install uninstall


############################################################
# Allow Makefile.local to change any of the above settings #
############################################################
include ${LOCAL_MKF}


###########################################
# all - First and default Makefile target #
###########################################

all: ${CALC_FILES} ${MAKE_FILE} .all


###############################
# additional Makefile targets #
###############################

.PHONY: ${PHONY}

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
#	an non-empty else clause for every if condition.  *sigh*
#
##

distlist: ${DISTLIST}
	${Q} for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo cal/$$i; \
	    fi; \
	done

buildlist:
	${Q} for i in ${BUILD_ALL} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo cal/$$i; \
	    fi; \
	done | fgrep -v '.bak' | LANG=C ${SORT}

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
	-${Q} for i in ${DEAD_CALC_FILES} test082.cal outfile /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
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
