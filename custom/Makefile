#!/usr/bin/env make
#
# custom - makefile for calc custom routines
#
# Copyright (C) 1999-2006,2014,2017-2018,2021,2023,2025  Landon Curt Noll
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
# Under source code control:	1997/03/09 02:28:54
# File existed as early as:	1997
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

# Host targets that are shared in common with both Makefile
# and custom/Makefile.
#
TARGET_MKF= ../Makefile.target

# Host target information.
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


##################################
# Files managed by this Makefile #
##################################

# The custom calc resource files to install
#
# Put your custom calc resource files here.
#
CUSTOM_CALC_FILES= argv.cal halflen.cal pzasusb8.cal pmodm127.cal register.cal pfe.cal

# The custom help files to install
#
# Put your custom help files here.
#
CUSTOM_HELP= argv devnull help sysinfo pzasusb8 pmodm127 register pfe

# Any .h files that are needed by programs that use
# libcustcalc${LIB_EXT_VERSION}
#
# Put any .h files that you add which might be useful to other
# programs here.
#
CUSTOM_H_SRC=

# Any .c files that are needed to build libcustcalc${LIB_EXT_VERSION}.
#
# Don't put ${REQUIRED_SRC} files in this list.
#
# There MUST be a .c in CUSTOM_SRC for every .o in CUSTOM_OBJ.
#
# Put your custom .c files here.
#
CUSTOM_SRC= c_argv.c c_devnull.c c_help.c c_sysinfo.c c_pzasusb8.c \
	c_pmodm127.c c_register.c u_pfe.c

# Any .o files that are needed by program that use
# libcustcalc${LIB_EXT_VERSION}.
#
# Don't put ${REQUIRED_OBJ} files in this list.
#
# There MUST be a .c in CUSTOM_SRC for every .o in CUSTOM_OBJ.
#
# Put your custom .o files here.
#
CUSTOM_OBJ= c_argv.o c_devnull.o c_help.o c_sysinfo.o c_pzasusb8.o \
	c_pmodm127.o c_register.o u_pfe.o


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

# These .c files are required for the main custom interface and
# for the custom support functions for libcustcalc${LIB_EXT_VERSION}.
#
# There MUST be a .c for every .o in REQUIRED_OBJ.
#
REQUIRED_SRC= custtbl.c

# These .o files correspond to the .c files in REQUIRED_SRC
# for libcustcalc${LIB_EXT_VERSION}.
#
# There MUST be a .o for every .c in REQUIRED_SRC.
#
REQUIRED_OBJ= custtbl.o

# These .h files are installed under ${CUSTOMCALDIR} by the install rule.
#
INSTALL_H_SRC= ${CUSTOM_H_SRC}

# These .c files are used to form libcustcalc${LIB_EXT_VERSION}.
#
CUSTCALC_SRC= ${REQUIRED_SRC} ${CUSTOM_SRC}

# These .o files are used to form libcustcalc${LIB_EXT_VERSION}.
#
CUSTCALC_OBJ= ${REQUIRED_OBJ} ${CUSTOM_OBJ}

# These .c files are used to build the dependency list
#
C_SRC= ${REQUIRED_SRC} ${CUSTOM_SRC}

# These .h files are used to build the dependency list
#
H_SRC= ${CUSTOM_H_SRC}

# These files are found (but not built) in the distribution
#
# The CUSTOM_CAL and HOW_TO_ADD are files distributed from this
# directory but are installed as help files from the help/Makefile.
#
DISTLIST= ${CUSTCALC_SRC} ${CUSTOM_CALC_FILES} ${CUSTOM_HELP} \
	${INSTALL_H_SRC} CUSTOM_CAL HOW_TO_ADD Makefile

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST= ${CUSTCALC_SRC} ${INSTALL_H_SRC} ${MAKE_FILE} HOW_TO_ADD

# This is the custom dynamic shared library that is build
#
CALC_DYNAMIC_LIBCUSTCALC= libcustcalc${LIB_EXT_VERSION}

# This is the custom static library that is build
#
CALC_STATIC_LIBCUSTCALC= libcustcalc.a

# complete list of files that may be created as part of the build process
#
# Used by chk_tree via make prep
#
BUILD_ALL= ${REQUIRED_OBJ} ${CUSTOM_OBJ} \
	${CALC_DYNAMIC_LIBCUSTCALC} ${CALC_STATIC_LIBCUSTCALC} \
	.all

# complete list of targets
#
TARGETS= ${BLD_TYPE}

# rules that are not also names of files
#
PHONY= all distlist buildlist distdir calcliblist depend echo_inst_files clean \
	clobber install uninstall


############################################################
# Allow Makefile.local to change any of the above settings #
############################################################
include ${LOCAL_MKF}


###########################################
# all - First and default Makefile target #
###########################################

all: ${TARGETS} ${INSTALL_H_SRC} ${CUSTOM_CALC_FILES} \
	${CUSTOM_HELP} ${MAKE_FILE} .all


###############################
# additional Makefile targets #
###############################

.PHONY: ${PHONY}

calc-dynamic-only: ${CUSTCALC_OBJ} ${CALC_DYNAMIC_LIBCUSTCALC}

calc-static-only: ${CUSTCALC_OBJ} ${CALC_STATIC_LIBCUSTCALC}

custom/libcustcalc${LIB_EXT_VERSION}: libcustcalc${LIB_EXT_VERSION} \
	${CALC_STATIC_LIBCUSTCALC}

libcustcalc${LIB_EXT_VERSION}: ${CUSTCALC_OBJ} ${MAKE_FILE}
	${CC} ${LIBCUSTCALC_SHLIB} ${CUSTCALC_OBJ} -o $@

##
#
# Special .o files
#
##

c_sysinfo.o: c_sysinfo.c ${MAKE_FILE}
	${CC} ${CFLAGS} c_sysinfo.c -c

# to determine of we have done all
#
.all:
	${RM} -f .all
	${TOUCH} .all

###
#
# building calc-static and static lib*.a libraries
#
###

libcustcalc.a: ${CUSTCALC_OBJ} ${MAKE_FILE}
	${RM} -f libcustcalc.a
	${AR} qc libcustcalc.a ${CUSTCALC_OBJ}
	${RANLIB} libcustcalc.a
	${CHMOD} 0644 libcustcalc.a

##
#
# File list generation.	 You can ignore this section.
#
#
# We will form the names of source files as if they were in a
# sub-directory called calc/lib.
#
# NOTE: Due to bogus shells found on one common system we must have
#	an non-empty else clause for every if condition.  *sigh*
#
##

distlist: ${DISTLIST}
	${Q} for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo custom/$$i; \
	    fi; \
	done

buildlist:
	${Q} for i in ${BUILD_ALL} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo custom/$$i; \
	    fi; \
	done | fgrep -v '.bak' | LANG=C ${SORT}

distdir:
	${Q} echo custom

calcliblist: ${CALCLIBLIST}
	${Q} for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo custom/$$i; \
	    fi; \
	done

Makefile.simple:
	${Q} echo Support for custom/$@ was dropped after the the release of calc v2.14.3.0.

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
	    echo "custom/${MAKE_FILE}.bak exists, remove or move it" 1>&2; \
	    exit 1; \
	else \
	    ${TRUE}; \
	fi
	${Q} if type -f ${MAKEDEPEND}; then \
	    ${TRUE}; \
	else \
	    echo "make depend failed: cannot find makedepend command: ${MAKEDEPEND}" 1>&2; \
	    exit 1; \
	fi
	${Q} echo forming custom/skel
	${Q} ${RM} -rf skel
	${Q} ${MKDIR} -p skel
	${Q} ${MKDIR} -p skel/custom
	-${Q} for i in ${C_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' \
		    "$$i" > "skel/custom/$$i"; \
	    fi; \
	done
	-${Q} for i in ${H_SRC} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then continue; fi; \
	    tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
	    echo "#if !defined($$tag)" > "skel/custom/$$i"; \
	    echo "#define $$tag" >> "skel/custom/$$i"; \
	    ${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" \
		>> "skel/custom/$$i"; \
	    echo '#endif /* '"$$tag"' */' >> "skel/custom/$$i"; \
	done
	${Q} (cd ..; ${MAKE} -f ${MAKE_FILE} hsrc)
	(cd ..; ${MAKE} -f ${MAKE_FILE} h_list 2>/dev/null) | \
	    while read i; do \
		if [ ! -f "../$$i" ]; then continue; fi; \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#if !defined($$tag)" > "skel/$$i"; \
		echo "#define $$tag" >> "skel/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "../$$i" \
		    >> "skel/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/$$i"; \
	    done
	${Q} ${RM} -f skel/custom/makedep.out skel/custom/makedep.in
	${Q} echo custom/skel formed
	${Q} echo forming custom dependency list
	${Q} :> skel/custom/makedep.out
	-${Q} cd skel/custom; ${MAKEDEPEND} \
	    -I../../.. -w 1 -f makedep.out -- \
	    ${CFLAGS} -- \
	    ${C_SRC} 2>/dev/null
	${Q} ${CP} -f skel/custom/makedep.out skel/custom/makedep.in
	${Q} ${SED} -e 's|: ../../|: |' -E < skel/custom/makedep.in | \
	    ${GREP} -E -v /usr/include > skel/custom/makedep.out
	-${Q} for i in ${C_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo "$$i" | ${SED} 's/^\(.*\)\.c/\1.o: \1.c/'; \
	    fi; \
	done >> skel/custom/makedep.out
	${Q} LANG=C ${SORT} -u skel/custom/makedep.out \
			    -o skel/custom/makedep.out
	${Q} echo custom dependency list formed
	${Q} echo forming new custom/${MAKE_FILE}
	${Q} ${RM} -f ${MAKE_FILE}.bak
	${Q} ${MV} ${MAKE_FILE} ${MAKE_FILE}.bak
	${Q} ${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' \
		    ${MAKE_FILE}.bak > ${MAKE_FILE}
	${Q} ${GREP} -E -v '^#' skel/custom/makedep.out >> ${MAKE_FILE}
	${Q} echo removing skel
	${Q} ${RM} -rf skel
	-${Q} if ${CMP} -s ${MAKE_FILE}.bak ${MAKE_FILE}; then \
	    echo 'custom/${MAKE_FILE} was already up to date'; \
	    echo 'restoring original custom/${MAKE_FILE}'; \
	    ${MV} -f ${MAKE_FILE}.bak ${MAKE_FILE}; \
	else \
	    echo 'old custom/${MAKE_FILE} is now custom/${MAKE_FILE}.bak'; \
	    echo 'new custom/${MAKE_FILE} formed'; \
	    echo 'try: diff -u custom/${MAKE_FILE}.bak custom/${MAKE_FILE}'; \
	fi

##
#
# rpm rules
#
##

echo_inst_files:
	${Q} for i in ${INSTALL_H_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CUSTOMINCDIR}/$$i; \
	    fi; \
	done
	${Q} for i in ${CUSTOM_HELP} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CUSTOMHELPDIR}/$$i; \
	    fi; \
	done
	${Q} for i in ${CUSTOM_CALC_FILES} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CUSTOMCALDIR}/$$i; \
	    fi; \
	done
	${Q} for i in ${CALC_DYNAMIC_LIBCUSTCALC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CUSTOMCALDIR}/$$i; \
	    fi; \
	done

##
#
# Utility rules
#
##

clean:
	${RM} -f ${CUSTCALC_OBJ}

clobber: clean
	${RM} -f ${BLD_TYPE}
	${RM} -f .all ${MAKE_FILE}.tmp ${MAKE_FILE}.bak
	${RM} -f libcustcalc${LIB_EXT_VERSION}
	${RM} -f libcustcalc${LIB_EXT}
	${RM} -f libcustcalc.a
	${RM} -f libcustcalc*
	${RM} -rf skel
	${V} echo remove files that are obsolete
	-${Q} if [ -e .DS_Store ]; then \
	    echo ${RM} -rf .DS_Store; \
	    ${RM} -rf .DS_Store; \
	fi

# install everything
#
# NOTE: Keep the uninstall rule in reverse order to the install rule
#
install: all
	-${Q} if [ ! -d ${T}${LIBDIR} ]; then \
	    echo ${MKDIR} -p ${T}${LIBDIR}; \
	    ${MKDIR} -p ${T}${LIBDIR}; \
	    if [ ! -d "${T}${LIBDIR}" ]; then \
		echo ${MKDIR} -p "${T}${LIBDIR}"; \
		${MKDIR} -p "${T}${LIBDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${LIBDIR}; \
	    ${CHMOD} 0755 ${T}${LIBDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${INCDIR} ]; then \
	    echo ${MKDIR} -p ${T}${INCDIR}; \
	    ${MKDIR} -p ${T}${INCDIR}; \
	    if [ ! -d "${T}${INCDIR}" ]; then \
		echo ${MKDIR} -p "${T}${INCDIR}"; \
		${MKDIR} -p "${T}${INCDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${INCDIR}; \
	    ${CHMOD} 0755 ${T}${INCDIR}; \
	else \
	    ${TRUE}; \
	fi
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
	-${Q} if [ ! -d ${T}${CALC_INCDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CALC_INCDIR}; \
	    ${MKDIR} -p ${T}${CALC_INCDIR}; \
	    if [ ! -d "${T}${CALC_INCDIR}" ]; then \
		echo ${MKDIR} -p "${T}${CALC_INCDIR}"; \
		${MKDIR} -p "${T}${CALC_INCDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${CALC_INCDIR}; \
	    ${CHMOD} 0755 ${T}${CALC_INCDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${HELPDIR} ]; then \
	    echo ${MKDIR} -p ${T}${HELPDIR}; \
	    ${MKDIR} -p ${T}${HELPDIR}; \
	    if [ ! -d "${T}${HELPDIR}" ]; then \
		echo ${MKDIR} -p "${T}${HELPDIR}"; \
		${MKDIR} -p "${T}${HELPDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${HELPDIR}; \
	    ${CHMOD} 0755 ${T}${HELPDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${CUSTOMCALDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CUSTOMCALDIR}; \
	    ${MKDIR} -p ${T}${CUSTOMCALDIR}; \
	    if [ ! -d "${T}${CUSTOMCALDIR}" ]; then \
		echo ${MKDIR} -p "${T}${CUSTOMCALDIR}"; \
		${MKDIR} -p "${T}${CUSTOMCALDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${CUSTOMCALDIR}; \
	    ${CHMOD} 0755 ${T}${CUSTOMCALDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${CUSTOMHELPDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CUSTOMHELPDIR}; \
	    ${MKDIR} -p ${T}${CUSTOMHELPDIR}; \
	    if [ ! -d "${T}${CUSTOMHELPDIR}" ]; then \
		echo ${MKDIR} -p "${T}${CUSTOMHELPDIR}"; \
		${MKDIR} -p "${T}${CUSTOMHELPDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${CUSTOMHELPDIR}; \
	    ${CHMOD} 0755 ${T}${CUSTOMHELPDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${CUSTOMINCDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CUSTOMINCDIR}; \
	    ${MKDIR} -p ${T}${CUSTOMINCDIR}; \
	    if [ ! -d "${T}${CUSTOMINCDIR}" ]; then \
		echo ${MKDIR} -p "${T}${CUSTOMINCDIR}"; \
		${MKDIR} -p "${T}${CUSTOMINCDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 ${T}${CUSTOMINCDIR}; \
	    ${CHMOD} 0755 ${T}${CUSTOMINCDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} for i in ${INSTALL_H_SRC} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s "$$i" ${T}${CUSTOMINCDIR}/$$i; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${CUSTOMINCDIR}/$$i.new; \
		${CP} -f $$i ${T}${CUSTOMINCDIR}/$$i.new; \
		${CHMOD} 0444 ${T}${CUSTOMINCDIR}/$$i.new; \
		${MV} -f ${T}${CUSTOMINCDIR}/$$i.new ${T}${CUSTOMINCDIR}/$$i; \
		echo "installed ${T}${CUSTOMINCDIR}/$$i"; \
	    fi; \
	done
	-${Q} for i in ${CUSTOM_CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s $$i ${T}${CUSTOMCALDIR}/$$i; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${CUSTOMCALDIR}/$$i.new; \
		${CP} -f $$i ${T}${CUSTOMCALDIR}/$$i.new; \
		${CHMOD} 0444 ${T}${CUSTOMCALDIR}/$$i.new; \
		${MV} -f ${T}${CUSTOMCALDIR}/$$i.new ${T}${CUSTOMCALDIR}/$$i; \
		echo "installed ${T}${CUSTOMCALDIR}/$$i"; \
	    fi; \
	done
	-${Q} for i in ${CUSTOM_HELP} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s $$i ${T}${CUSTOMHELPDIR}/$$i; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${CUSTOMHELPDIR}/$$i.new; \
		${CP} -f $$i ${T}${CUSTOMHELPDIR}/$$i.new; \
		${CHMOD} 0444 ${T}${CUSTOMHELPDIR}/$$i.new; \
		${MV} -f ${T}${CUSTOMHELPDIR}/$$i.new \
			 ${T}${CUSTOMHELPDIR}/$$i; \
		echo "installed ${T}${CUSTOMHELPDIR}/$$i"; \
	    fi; \
	done
	-${Q} if [ -f "${T}${CUSTOMCALDIR}/libcustcalc.a" ]; then \
	    ${RM} -f ${T}${CUSTOMCALDIR}/libcustcalc.a; \
	    echo "removed old ${T}${CUSTOMCALDIR}/libcustcalc.a"; \
	fi
	-${Q} if [ X"${ALLOW_CUSTOM}" = "X-DCUSTOM" ]; then \
	  if [ -f libcustcalc.a ]; then \
	    if ${CMP} -s libcustcalc.a ${T}${LIBDIR}/libcustcalc.a; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${LIBDIR}/libcustcalc.a.new; \
		${CP} -f libcustcalc.a ${T}${LIBDIR}/libcustcalc.a.new; \
		${CHMOD} 0644 ${T}${LIBDIR}/libcustcalc.a.new; \
		${MV} -f ${T}${LIBDIR}/libcustcalc.a.new \
		      ${T}${LIBDIR}/libcustcalc.a; \
		${RANLIB} ${T}${LIBDIR}/libcustcalc.a; \
		echo "installed ${T}${LIBDIR}/libcustcalc.a"; \
	    fi; \
	  fi; \
	fi
	${Q}# NOTE: The upper level makefile installs the file
	${Q}#	    libcustcalc${LIB_EXT_VERSION} because we only want to
	${Q}#	    perform one ${LDCONFIG} for both libcalc${LIB_EXT_VERSION}
	${Q}#	    and libcustcalc${LIB_EXT_VERSION}.

# Try to remove everything that was installed
#
# NOTE: Keep the uninstall rule in reverse order to the install rule
#
uninstall:
	-${Q} if [ -f "${T}${CUSTOMCALDIR}/libcustcalc.a" ]; then \
	    ${RM} -f "${T}${CUSTOMCALDIR}/libcustcalc.a"; \
	    if [ -f "${T}${CUSTOMCALDIR}/libcustcalc.a" ]; then \
		echo "cannot uninstall ${T}${CUSTOMCALDIR}/libcustcalc.a"; \
	    else \
		echo "un-installed ${T}${CUSTOMCALDIR}/libcustcalc.a"; \
	    fi; \
	fi
	-${Q} for i in ${CUSTOM_HELP} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -f "${T}${CUSTOMHELPDIR}/$$i" ]; then \
		${RM} -f "${T}${CUSTOMHELPDIR}/$$i"; \
		if [ -f "${T}${CUSTOMHELPDIR}/$$i" ]; then \
		   echo "cannot uninstall ${T}${CUSTOMHELPDIR}/$$i"; \
		else \
		   echo "un-installed ${T}${CUSTOMHELPDIR}/$$i"; \
		fi; \
	    fi; \
	done
	-${Q} for i in ${CUSTOM_CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -f "${T}${CUSTOMCALDIR}/$$i" ]; then \
		${RM} -f "${T}${CUSTOMCALDIR}/$$i"; \
		if [ -f "${T}${CUSTOMCALDIR}/$$i" ]; then \
		   echo "cannot uninstall ${T}${CUSTOMCALDIR}/$$i"; \
		else \
		   echo "un-installed ${T}${CUSTOMCALDIR}/$$i"; \
		fi; \
	    fi; \
	done
	-${Q} for i in ${INSTALL_H_SRC} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -f "${T}${CUSTOMINCDIR}/$$i" ]; then \
		${RM} -f "${T}${CUSTOMINCDIR}/$$i"; \
		if [ -f "${T}${CUSTOMINCDIR}/$$i" ]; then \
		   echo "cannot uninstall ${T}${CUSTOMINCDIR}/$$i"; \
		else \
		   echo "un-installed ${T}${CUSTOMINCDIR}/$$i"; \
		fi; \
	    fi; \
	done
	-${Q} for i in ${CUSTOMINCDIR} ${CUSTOMHELPDIR} ${CUSTOMCALDIR} \
		    ${HELPDIR} ${CALC_INCDIR} ${CALC_SHAREDIR} ${INCDIR}; do \
	    if [ -d "${T}$$i" ]; then \
		${RMDIR} "${T}$$i" 2>/dev/null; \
		echo "cleaned up ${T}$$i"; \
	    fi; \
	done

##
#
# make depend stuff
#
##

# DO NOT DELETE THIS LINE -- make depend depends on it.

c_argv.o: ../alloc.h
c_argv.o: ../attribute.h
c_argv.o: ../banned.h
c_argv.o: ../block.h
c_argv.o: ../bool.h
c_argv.o: ../byteswap.h
c_argv.o: ../calc.h
c_argv.o: ../charbit.h
c_argv.o: ../cmath.h
c_argv.o: ../config.h
c_argv.o: ../custom.h
c_argv.o: ../decl.h
c_argv.o: ../endian_calc.h
c_argv.o: ../errsym.h
c_argv.o: ../errtbl.h
c_argv.o: ../hash.h
c_argv.o: ../have_ban_pragma.h
c_argv.o: ../have_const.h
c_argv.o: ../have_limits.h
c_argv.o: ../have_newstr.h
c_argv.o: ../have_stdbool.h
c_argv.o: ../have_stdlib.h
c_argv.o: ../have_string.h
c_argv.o: ../have_unused.h
c_argv.o: ../longbits.h
c_argv.o: ../nametype.h
c_argv.o: ../qmath.h
c_argv.o: ../sha1.h
c_argv.o: ../str.h
c_argv.o: ../value.h
c_argv.o: ../version.h
c_argv.o: ../zmath.h
c_argv.o: c_argv.c
c_devnull.o: ../alloc.h
c_devnull.o: ../attribute.h
c_devnull.o: ../banned.h
c_devnull.o: ../block.h
c_devnull.o: ../bool.h
c_devnull.o: ../byteswap.h
c_devnull.o: ../charbit.h
c_devnull.o: ../cmath.h
c_devnull.o: ../config.h
c_devnull.o: ../custom.h
c_devnull.o: ../decl.h
c_devnull.o: ../endian_calc.h
c_devnull.o: ../errsym.h
c_devnull.o: ../errtbl.h
c_devnull.o: ../hash.h
c_devnull.o: ../have_ban_pragma.h
c_devnull.o: ../have_const.h
c_devnull.o: ../have_limits.h
c_devnull.o: ../have_newstr.h
c_devnull.o: ../have_stdbool.h
c_devnull.o: ../have_stdlib.h
c_devnull.o: ../have_string.h
c_devnull.o: ../have_unistd.h
c_devnull.o: ../have_unused.h
c_devnull.o: ../longbits.h
c_devnull.o: ../nametype.h
c_devnull.o: ../qmath.h
c_devnull.o: ../sha1.h
c_devnull.o: ../str.h
c_devnull.o: ../value.h
c_devnull.o: ../version.h
c_devnull.o: ../zmath.h
c_devnull.o: c_devnull.c
c_help.o: ../alloc.h
c_help.o: ../attribute.h
c_help.o: ../banned.h
c_help.o: ../block.h
c_help.o: ../bool.h
c_help.o: ../byteswap.h
c_help.o: ../charbit.h
c_help.o: ../cmath.h
c_help.o: ../config.h
c_help.o: ../custom.h
c_help.o: ../decl.h
c_help.o: ../endian_calc.h
c_help.o: ../errsym.h
c_help.o: ../errtbl.h
c_help.o: ../hash.h
c_help.o: ../have_ban_pragma.h
c_help.o: ../have_const.h
c_help.o: ../have_limits.h
c_help.o: ../have_newstr.h
c_help.o: ../have_stdbool.h
c_help.o: ../have_stdlib.h
c_help.o: ../have_string.h
c_help.o: ../have_unistd.h
c_help.o: ../have_unused.h
c_help.o: ../longbits.h
c_help.o: ../nametype.h
c_help.o: ../qmath.h
c_help.o: ../sha1.h
c_help.o: ../str.h
c_help.o: ../value.h
c_help.o: ../version.h
c_help.o: ../zmath.h
c_help.o: c_help.c
c_pmodm127.o: ../alloc.h
c_pmodm127.o: ../attribute.h
c_pmodm127.o: ../banned.h
c_pmodm127.o: ../block.h
c_pmodm127.o: ../bool.h
c_pmodm127.o: ../byteswap.h
c_pmodm127.o: ../charbit.h
c_pmodm127.o: ../cmath.h
c_pmodm127.o: ../config.h
c_pmodm127.o: ../custom.h
c_pmodm127.o: ../decl.h
c_pmodm127.o: ../endian_calc.h
c_pmodm127.o: ../errsym.h
c_pmodm127.o: ../errtbl.h
c_pmodm127.o: ../hash.h
c_pmodm127.o: ../have_ban_pragma.h
c_pmodm127.o: ../have_const.h
c_pmodm127.o: ../have_limits.h
c_pmodm127.o: ../have_newstr.h
c_pmodm127.o: ../have_stdbool.h
c_pmodm127.o: ../have_stdlib.h
c_pmodm127.o: ../have_string.h
c_pmodm127.o: ../have_unused.h
c_pmodm127.o: ../longbits.h
c_pmodm127.o: ../nametype.h
c_pmodm127.o: ../qmath.h
c_pmodm127.o: ../sha1.h
c_pmodm127.o: ../str.h
c_pmodm127.o: ../value.h
c_pmodm127.o: ../version.h
c_pmodm127.o: ../zmath.h
c_pmodm127.o: c_pmodm127.c
c_pzasusb8.o: ../alloc.h
c_pzasusb8.o: ../attribute.h
c_pzasusb8.o: ../banned.h
c_pzasusb8.o: ../block.h
c_pzasusb8.o: ../bool.h
c_pzasusb8.o: ../byteswap.h
c_pzasusb8.o: ../charbit.h
c_pzasusb8.o: ../cmath.h
c_pzasusb8.o: ../config.h
c_pzasusb8.o: ../custom.h
c_pzasusb8.o: ../decl.h
c_pzasusb8.o: ../endian_calc.h
c_pzasusb8.o: ../errsym.h
c_pzasusb8.o: ../errtbl.h
c_pzasusb8.o: ../hash.h
c_pzasusb8.o: ../have_ban_pragma.h
c_pzasusb8.o: ../have_const.h
c_pzasusb8.o: ../have_limits.h
c_pzasusb8.o: ../have_newstr.h
c_pzasusb8.o: ../have_stdbool.h
c_pzasusb8.o: ../have_stdlib.h
c_pzasusb8.o: ../have_string.h
c_pzasusb8.o: ../have_unused.h
c_pzasusb8.o: ../longbits.h
c_pzasusb8.o: ../nametype.h
c_pzasusb8.o: ../qmath.h
c_pzasusb8.o: ../sha1.h
c_pzasusb8.o: ../str.h
c_pzasusb8.o: ../value.h
c_pzasusb8.o: ../version.h
c_pzasusb8.o: ../zmath.h
c_pzasusb8.o: c_pzasusb8.c
c_register.o: ../alloc.h
c_register.o: ../attribute.h
c_register.o: ../banned.h
c_register.o: ../block.h
c_register.o: ../bool.h
c_register.o: ../byteswap.h
c_register.o: ../calc.h
c_register.o: ../charbit.h
c_register.o: ../cmath.h
c_register.o: ../config.h
c_register.o: ../custom.h
c_register.o: ../decl.h
c_register.o: ../endian_calc.h
c_register.o: ../errsym.h
c_register.o: ../errtbl.h
c_register.o: ../hash.h
c_register.o: ../have_ban_pragma.h
c_register.o: ../have_const.h
c_register.o: ../have_limits.h
c_register.o: ../have_newstr.h
c_register.o: ../have_stdbool.h
c_register.o: ../have_stdlib.h
c_register.o: ../have_string.h
c_register.o: ../have_unused.h
c_register.o: ../longbits.h
c_register.o: ../nametype.h
c_register.o: ../qmath.h
c_register.o: ../sha1.h
c_register.o: ../str.h
c_register.o: ../value.h
c_register.o: ../version.h
c_register.o: ../zmath.h
c_register.o: c_register.c
c_sysinfo.o: ../alloc.h
c_sysinfo.o: ../attribute.h
c_sysinfo.o: ../banned.h
c_sysinfo.o: ../block.h
c_sysinfo.o: ../bool.h
c_sysinfo.o: ../byteswap.h
c_sysinfo.o: ../calc.h
c_sysinfo.o: ../charbit.h
c_sysinfo.o: ../cmath.h
c_sysinfo.o: ../conf.h
c_sysinfo.o: ../config.h
c_sysinfo.o: ../custom.h
c_sysinfo.o: ../decl.h
c_sysinfo.o: ../endian_calc.h
c_sysinfo.o: ../errsym.h
c_sysinfo.o: ../errtbl.h
c_sysinfo.o: ../fposval.h
c_sysinfo.o: ../hash.h
c_sysinfo.o: ../have_ban_pragma.h
c_sysinfo.o: ../have_const.h
c_sysinfo.o: ../have_limits.h
c_sysinfo.o: ../have_newstr.h
c_sysinfo.o: ../have_stdbool.h
c_sysinfo.o: ../have_stdlib.h
c_sysinfo.o: ../have_string.h
c_sysinfo.o: ../have_unused.h
c_sysinfo.o: ../hist.h
c_sysinfo.o: ../lib_calc.h
c_sysinfo.o: ../longbits.h
c_sysinfo.o: ../nametype.h
c_sysinfo.o: ../prime.h
c_sysinfo.o: ../qmath.h
c_sysinfo.o: ../sha1.h
c_sysinfo.o: ../str.h
c_sysinfo.o: ../value.h
c_sysinfo.o: ../version.h
c_sysinfo.o: ../zmath.h
c_sysinfo.o: ../zrand.h
c_sysinfo.o: ../zrandom.h
c_sysinfo.o: c_sysinfo.c
custtbl.o: ../alloc.h
custtbl.o: ../attribute.h
custtbl.o: ../banned.h
custtbl.o: ../block.h
custtbl.o: ../bool.h
custtbl.o: ../byteswap.h
custtbl.o: ../charbit.h
custtbl.o: ../cmath.h
custtbl.o: ../config.h
custtbl.o: ../custom.h
custtbl.o: ../decl.h
custtbl.o: ../endian_calc.h
custtbl.o: ../errsym.h
custtbl.o: ../errtbl.h
custtbl.o: ../hash.h
custtbl.o: ../have_ban_pragma.h
custtbl.o: ../have_const.h
custtbl.o: ../have_limits.h
custtbl.o: ../have_newstr.h
custtbl.o: ../have_stdbool.h
custtbl.o: ../have_stdlib.h
custtbl.o: ../have_string.h
custtbl.o: ../longbits.h
custtbl.o: ../nametype.h
custtbl.o: ../qmath.h
custtbl.o: ../sha1.h
custtbl.o: ../str.h
custtbl.o: ../value.h
custtbl.o: ../version.h
custtbl.o: ../zmath.h
custtbl.o: custtbl.c
u_pfe.o: ../alloc.h
u_pfe.o: ../attribute.h
u_pfe.o: ../banned.h
u_pfe.o: ../block.h
u_pfe.o: ../bool.h
u_pfe.o: ../byteswap.h
u_pfe.o: ../calc.h
u_pfe.o: ../charbit.h
u_pfe.o: ../cmath.h
u_pfe.o: ../config.h
u_pfe.o: ../custom.h
u_pfe.o: ../decl.h
u_pfe.o: ../endian_calc.h
u_pfe.o: ../errsym.h
u_pfe.o: ../errtbl.h
u_pfe.o: ../hash.h
u_pfe.o: ../have_ban_pragma.h
u_pfe.o: ../have_const.h
u_pfe.o: ../have_limits.h
u_pfe.o: ../have_newstr.h
u_pfe.o: ../have_stdbool.h
u_pfe.o: ../have_stdlib.h
u_pfe.o: ../have_string.h
u_pfe.o: ../have_unused.h
u_pfe.o: ../longbits.h
u_pfe.o: ../nametype.h
u_pfe.o: ../qmath.h
u_pfe.o: ../sha1.h
u_pfe.o: ../str.h
u_pfe.o: ../value.h
u_pfe.o: ../version.h
u_pfe.o: ../zmath.h
u_pfe.o: u_pfe.c
