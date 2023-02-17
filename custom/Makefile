#!/usr/bin/env make
#
# custom - makefile for calc custom routines
#
# Copyright (C) 1999-2006,2014,2017-2018,2021,2023  Landon Curt Noll
#
# SRC: custom/Makefile
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
# Under source code control:	1997/03/09 02:28:54
# File existed as early as:	1997
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


# CCBAN is given to ${CC} in order to control if banned.h is in effect.
#
# The banned.h attempts to ban the use of certain dangerous functions
# that, if improperly used, could compromise the computational integrity
# if calculations.
#
# In the case of calc, we are motivated in part by the desire for calc
# to correctly calculate: even during extremely long calculations.
#
# If UNBAN is NOT defined, then calling certain functions
# will result in a call to a non-existent function (link error).
#
# While we do NOT encourage defining UNBAN, there may be
# a system / compiler environment where re-defining a
# function may lead to a fatal compiler complication.
# If that happens, consider compiling as:
#
#	make clobber all chk CCBAN=-DUNBAN
#
# as see if this is a work-a-round.
#
# If YOU discover a need for the -DUNBAN work-a-round, PLEASE tell us!
# Please send us a bug report.  See the file:
#
#	BUGS
#
# or the URL:
#
#	http://www.isthe.com/chongo/tech/comp/calc/calc-bugrept.html
#
# for how to send us such a bug report.
#
CCBAN= -UUNBAN
#CCBAN= -DUNBAN


#if 0	/* start of skip for non-Gnu makefiles */

# Unfortunately due to the complex dependency issues between
# Makefile, and custom/Makefile, parallel GNU make
# is NOT recommended.
#
# XXX - fix this - XXX
#
.NOTPARALLEL:

##############################################################################
#-=-=-=-=-=-=-=-=- Identify the target machine, if possible -=-=-=-=-=-=-=-=-#
##############################################################################

# NOTE: You can force a target value by defining target as in:
#
#	make ...__optional_arguments_... target=value

# Try uname -s if the target was not already set on the make command line
#
ifeq ($(target),)
target=$(shell uname -s 2>/dev/null)
endif
#endif	/* end of skip for non-Gnu makefiles */

# The custom calc resource files to install
#
# Put your custom calc resource files here.
#
CUSTOM_CALC_FILES= argv.cal halflen.cal pzasusb8.cal pmodm127.cal register.cal

# The custom help files to install
#
# Put your custom help files here.
#
CUSTOM_HELP= argv devnull help sysinfo pzasusb8 pmodm127 register

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
	c_pmodm127.c c_register.c

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
	c_pmodm127.o c_register.o

##############################################################################
#-=-=-=-=-=-=-=-=- You may want to change some values below -=-=-=-=-=-=-=-=-#
##############################################################################

##############################################################################
#-=-=-=-=-=-=- Defaults in case you want to build from this dir -=-=-=-=-=-=-#
##############################################################################

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.

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
# ${CUSTOMCALDIR}	where custom *.cal files are installed
# ${CUSTOMHELPDIR}	where custom help files are installed
# ${CUSTOMINCDIR}	where custom .h files are installed
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

# Debug/Optimize options for ${CC} and ${LCC}
#
# Select DEBUG= -O2 -gstabs+ -D_WIN32 for DJGPP.
#
#DEBUG=
#
#DEBUG= -O
#DEBUG= -O -g
#DEBUG= -O -g3
#
#DEBUG= -O1
#DEBUG= -O1 -g
#DEBUG= -O1 -g3
#
#DEBUG= -O2
#DEBUG= -O2 -g
#DEBUG= -O2 -g3
#DEBUG= -O2 -ipa
#DEBUG= -O2 -g3 -ipa
#
#DEBUG= -O3
#DEBUG= -O3 -g
DEBUG= -O3 -g3
#DEBUG= -O3 -ipa
#DEBUG= -O3 -g3 -ipa
#
#DEBUG= -g
#DEBUG= -g3
#DEBUG= -gx
#DEBUG= -WM,-g
#
#DEBUG= -O2 -gstabs+ -D_WIN32

# Some systems require one to use ranlib to add a symbol table to
# a *.a link library.  Set RANLIB to the utility that performs this
# action.  Set RANLIB to : if your system does not need such a utility.
#
RANLIB=ranlib
#RANLIB=:

# Normally certain files depend on the Makefile.  If the Makefile is
# changed, then certain steps should be redone.	 If MAKE_FILE is
# set to Makefile, then these files will depend on Makefile.  If
# MAKE_FILE is empty, then they wont.
#
# If in doubt, set MAKE_FILE to Makefile
#
MAKE_FILE= Makefile
#MAKE_FILE=

# If you do not wish to use purify, set PURIFY to an empty string.
#
# If in doubt, use PURIFY=
#
#PURIFY= purify
#PURIFY= purify -m71-engine
#PURIFY= purify -logfile=pure.out
#PURIFY= purify -m71-engine -logfile=pure.out
PURIFY=

# If you want to use a debugging link library such as a malloc debug link
# library, or need to add special ld flags after the calc link libraries
# are included, set ${LD_DEBUG} below.
#
# If in doubt, set LD_DEBUG to empty.
#
#LD_DEBUG= -lmalloc_cv
LD_DEBUG=

# When doing a:
#
#	make check
#	make chk
#	make debug
#
# the ${CALC_ENV} is used to supply the proper environment variables
# to calc.  Most people will simply need 'CALCPATH=./cal' to ensure
# that these debug rules will only use calc resource files under the
# local source directory.  The longer lines (with MALLOC_VERBOSE=1 ...)
# are useful for SGI IRIX people who have 'WorkShop Performance Tools'
# and who also set 'LD_DEBUG= -lmalloc_cv' above.
#
# If in doubt, use CALC_ENV= CALCPATH=./cal.
#
CALC_ENV= CALCPATH=./cal
#CALC_ENV= CALCPATH=./cal MALLOC_VERBOSE=1 MALLOC_TRACING=1 \
#	  MALLOC_FASTCHK=1 MALLOC_FULLWARN=1
#CALC_ENV= CALCPATH=./cal MALLOC_VERBOSE=1 MALLOC_TRACING=1 \
#	  MALLOC_FASTCHK=1 MALLOC_FULLWARN=1 MALLOC_CLEAR_FREE=1 \
#	  MALLOC_CLEAR_MALLOC=1

# By default, custom builtin functions may only be executed if calc
# is given the -C option.  This is because custom builtin functions
# may invoke non-standard or non-portable code.	 One may completely
# disable custom builtin functions by not compiling any of code
#
# ALLOW_CUSTOM= -DCUSTOM	# allow custom only if -C is given
# ALLOW_CUSTOM=			# disable custom even if -C is given
#
# If in doubt, use ALLOW_CUSTOM= -DCUSTOM
#
ALLOW_CUSTOM= -DCUSTOM
#ALLOW_CUSTOM=

# Some out of date operating systems require / want an executable to
# end with a certain file extension.  Some compile systems such as
# Cygwin build calc as calc.exe.  The EXT variable is used to denote
# the extension required by such.
#
# EXT=				# normal Un*x / Linux / GNU/Linux systems
# EXT=.exe			# Windows / Cygwin
#
# If in doubt, use EXT=
#
EXT=
#EXT=.exe

# The default calc versions
#
VERSION= 2.14.1.3

# Names of shared libraries with versions
#
LIB_EXT= .so
LIB_EXT_VERSION= ${LIB_EXT}.${VERSION}

# standard tools
#
AR= ar
AWK= awk
CHMOD= chmod
CMP= cmp
CO= co
CP= cp
DIFF= diff
FMT= fmt
GREP= egrep
LN= ln
LS= ls
MAKE= make
MAKEDEPEND= makedepend
MKDIR= mkdir
MV= mv
RM= rm
RMDIR= rmdir
SED= sed
SORT= sort
TAIL= tail
TOUCH= touch
TRUE= true

# EXTRA_CFLAGS are flags given to ${CC} when compiling C files
# EXTRA_LDFLAGS are flags given to ${CC} when linking progs
#
# Both CFLAGS and LDFLAGS are left blank in this Makefile by
# default so that users may use them on the make command line
# to always the way that C is compiled and files are linked
# respectively.  For example:
#
#	make all EXTRA_CFLAGS="-DMAGIC" EXTRA_LDFLAGS="-lmagic"
#
# NOTE: These should be left blank in this Makefile to make it
#       easier to add stuff on the command line.  If you want to
#	to change the way calc is compiled by this Makefile, change
#	the appropriate host target section below or a flag above.
#
EXTRA_CFLAGS=
EXTRA_LDFLAGS=

# COMMON_CFLAGS are the common ${CC} flags used for all progs, both
#	    intermediate and final calc and calc related progs
#
COMMON_CFLAGS= -DCALC_SRC ${ALLOW_CUSTOM} ${CCWARN} ${CCMISC} ${EXTRA_CFLAGS}

# COMMON_LDFLAGS are the common flags used for linking all progs, both
#	     intermediate and final calc and calc related progs
#
COMMON_LDFLAGS= ${EXTRA_LDFLAGS}

###########################################
# Set the default compile flags for ${CC} #
###########################################

#if 0	/* start of skip for non-Gnu makefiles */

# Required flags to compile C files for calc
#
# ICFLAGS are given to ${CC} for intermediate progs used to help compile calc
# CFLAGS are given to ${CC} for calc progs other than intermediate progs
#
# NOTE: This does not work for: make-XYZ-only and BLD_TYPE != make-XYZ-only
#
ifeq ($(BLD_TYPE),calc-static-only)
ICFLAGS= ${COMMON_CFLAGS} ${CCBAN} ${CC_STATIC}
else
ICFLAGS= ${COMMON_CFLAGS} ${CCBAN} ${CC_SHARE}
endif
CFLAGS= ${ICFLAGS} ${CCOPT}

# Required flags to link files for calc
#
# ILDFLAGS for ${CC} in linking intermediate progs used to help compile calc
# LDFLAGS for ${CC} in linking calc progs other than intermediate progs
#
ILDFLAGS= ${COMMON_LDFLAGS}
LDFLAGS= ${LD_DEBUG} ${ILDFLAGS}

#endif	/* end of skip for non-Gnu makefiles */

##########################################################################
#=-=-=-=-=- Be careful if you change something below this line -=-=-=-=-=#
##########################################################################

# include start from top Makefile - keep this line
#if 0	/* start of skip for non-Gnu makefiles */
###
#
# Allow Makefile.local to override any of the above settings
#
###
EXCLUDE_FROM_CUSTOM_MAKEFILE=true	# include only middle of ../Makefile
include ../Makefile
#endif	/* end of skip for non-Gnu makefiles */
# include end from top Makefile - keep this line

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
	${INSTALL_H_SRC} CUSTOM_CAL HOW_TO_ADD Makefile Makefile.simple

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST= ${CUSTCALC_SRC} ${INSTALL_H_SRC} ${MAKE_FILE} HOW_TO_ADD

# This is the custom dynamic shared library that is build
#
CALC_DYNAMIC_LIBCUSTCALC= libcustcalc${LIB_EXT_VERSION}

# This is the custom static library that is build
#
CALC_STATIC_LIBCUSTCALC= libcustcalc.a

# complete list of targets
#
TARGETS= ${BLD_TYPE} Makefile.simple

###
#
# The main reason for this Makefile	:-)
#
###

all: ${TARGETS} ${INSTALL_H_SRC} ${CUSTOM_CALC_FILES} \
	${CUSTOM_HELP} ${MAKE_FILE} .all

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

##
#
# used by the upper level Makefile
#
##

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

distdir:
	${Q} echo custom

calcliblist: ${CALCLIBLIST}
	${Q} for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo custom/$$i; \
	    fi; \
	done

#if 0	/* start of skip for non-Gnu makefiles */
Makefile.simple: Makefile ../Makefile
	${V} echo '=-=-=-=-= custom/${MAKE_FILE} start of $@ rule =-=-=-=-='
	${Q} if [ -f $@.bak ]; then \
		echo "custom/$@.bak exists, remove or move it"; \
		exit 1; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ -f $@ ]; then \
	    ${MV} -f $@ $@.bak; \
	fi
	${Q} ${AWK} '/^#if 0/{skp=1} {if(!skp){print $$0}} /^#endif/{skp=0}' \
	     Makefile | \
	     ${SED} -e '/^# include start from top Makefile - keep /,$$d' \
		    -e 's;^# SRC:.*;# SRC: custom/$@ - non-GNU version;' \
		    -e '/^ifeq /d' \
		    -e '/^ifneq /d' \
		    -e '/^ifdef /d' \
		    -e '/^ifndef /d' \
		    -e '/^else/d' \
		    -e '/^endif/d' \
		    > $@
	${Q} echo '# include start from top Makefile - keep this line' \
	     >> $@
	${Q} ${AWK} '/^#if 0/{skp=1} {if(!skp){print $$0}} /^#endif/{skp=0}' \
	     ../Makefile | \
	     ${SED} -e '1,/^# include start from top Makefile - keep /d' \
		    -e '/^# include end from top Makefile - keep /,$$d' \
		    -e '/^ifeq /d' \
		    -e '/^ifneq /d' \
		    -e '/^ifdef /d' \
		    -e '/^ifndef /d' \
		    -e '/^else/d' \
		    -e '/^endif/d' \
		    >> $@
	${Q} echo '# include end from top Makefile - keep this line' \
	     >> $@
	${Q} ${AWK} '/^#if 0/{skp=1} {if(!skp){print $$0}} /^#endif/{skp=0}' \
	     Makefile | \
	     ${SED} -e '1,/^# include end from top Makefile - keep /d' \
		    -e '/^ifeq /d' \
		    -e '/^ifneq /d' \
		    -e '/^ifdef /d' \
		    -e '/^ifndef /d' \
		    -e '/^else/d' \
		    -e '/^endif/d' \
		    >> $@
	-${Q} if [ -s $@.bak ]; then \
	    if ${CMP} -s $@.bak $@; then \
		echo 'custom/$@ was already up to date'; \
		echo 'restoring original custom/$@'; \
		${MV} -f $@.bak $@; \
	    else \
		echo 'old custom/$@ is now custom/$@.bak'; \
		echo 'updated custom/$@ formed'; \
		(cd .. ; ${DIFF} -u custom/$@.bak custom/$@); \
	    fi; \
	else \
	    echo 'new custom/$@ formed'; \
	    echo; \
	    (cd .. ; ${LS} -l custom/$@); \
	    echo; \
	fi
	${V} echo '=-=-=-=-= custom/${MAKE_FILE} end of $@ rule =-=-=-=-='
#endif	/* end of skip for non-Gnu makefiles */

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
	    echo "custom/${MAKE_FILE}.bak exists, remove or move it"; \
	    exit 1; \
	else \
	    ${TRUE}; \
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
	${MAKE} -f ../${MAKE_FILE} h_list 2>/dev/null | \
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
	${Q} cd skel/custom; ${MAKEDEPEND} \
	    -I../../.. -w 1 -f makedep.out -- \
	    ${CFLAGS} -- \
	    ${C_SRC} 2>/dev/null
	${Q} ${CP} -f skel/custom/makedep.out skel/custom/makedep.in
	${Q} ${SED} -e 's|: ../../|: |' -E < skel/custom/makedep.in | \
	    ${GREP} -v /usr/include > skel/custom/makedep.out
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
	${Q} ${GREP} -v '^#' skel/custom/makedep.out >> ${MAKE_FILE}
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
	-${Q} if [ ! -z ${ALLOW_CUSTOM} ]; then \
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
c_argv.o: ../byteswap.h
c_argv.o: ../calc.h
c_argv.o: ../calcerr.h
c_argv.o: ../cmath.h
c_argv.o: ../config.h
c_argv.o: ../custom.h
c_argv.o: ../decl.h
c_argv.o: ../endian_calc.h
c_argv.o: ../hash.h
c_argv.o: ../have_ban_pragma.h
c_argv.o: ../have_const.h
c_argv.o: ../have_memmv.h
c_argv.o: ../have_newstr.h
c_argv.o: ../have_stdlib.h
c_argv.o: ../have_string.h
c_argv.o: ../have_unused.h
c_argv.o: ../longbits.h
c_argv.o: ../nametype.h
c_argv.o: ../qmath.h
c_argv.o: ../sha1.h
c_argv.o: ../str.h
c_argv.o: ../value.h
c_argv.o: ../zmath.h
c_argv.o: c_argv.c
c_devnull.o: ../alloc.h
c_devnull.o: ../attribute.h
c_devnull.o: ../banned.h
c_devnull.o: ../block.h
c_devnull.o: ../byteswap.h
c_devnull.o: ../calcerr.h
c_devnull.o: ../cmath.h
c_devnull.o: ../config.h
c_devnull.o: ../custom.h
c_devnull.o: ../decl.h
c_devnull.o: ../endian_calc.h
c_devnull.o: ../hash.h
c_devnull.o: ../have_ban_pragma.h
c_devnull.o: ../have_const.h
c_devnull.o: ../have_memmv.h
c_devnull.o: ../have_newstr.h
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
c_devnull.o: ../zmath.h
c_devnull.o: c_devnull.c
c_help.o: ../alloc.h
c_help.o: ../attribute.h
c_help.o: ../banned.h
c_help.o: ../block.h
c_help.o: ../byteswap.h
c_help.o: ../calcerr.h
c_help.o: ../cmath.h
c_help.o: ../config.h
c_help.o: ../custom.h
c_help.o: ../decl.h
c_help.o: ../endian_calc.h
c_help.o: ../hash.h
c_help.o: ../have_ban_pragma.h
c_help.o: ../have_const.h
c_help.o: ../have_memmv.h
c_help.o: ../have_newstr.h
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
c_help.o: ../zmath.h
c_help.o: c_help.c
c_pmodm127.o: ../alloc.h
c_pmodm127.o: ../attribute.h
c_pmodm127.o: ../banned.h
c_pmodm127.o: ../block.h
c_pmodm127.o: ../byteswap.h
c_pmodm127.o: ../calcerr.h
c_pmodm127.o: ../cmath.h
c_pmodm127.o: ../config.h
c_pmodm127.o: ../custom.h
c_pmodm127.o: ../decl.h
c_pmodm127.o: ../endian_calc.h
c_pmodm127.o: ../hash.h
c_pmodm127.o: ../have_ban_pragma.h
c_pmodm127.o: ../have_const.h
c_pmodm127.o: ../have_memmv.h
c_pmodm127.o: ../have_newstr.h
c_pmodm127.o: ../have_stdlib.h
c_pmodm127.o: ../have_string.h
c_pmodm127.o: ../have_unused.h
c_pmodm127.o: ../longbits.h
c_pmodm127.o: ../nametype.h
c_pmodm127.o: ../qmath.h
c_pmodm127.o: ../sha1.h
c_pmodm127.o: ../str.h
c_pmodm127.o: ../value.h
c_pmodm127.o: ../zmath.h
c_pmodm127.o: c_pmodm127.c
c_pzasusb8.o: ../alloc.h
c_pzasusb8.o: ../attribute.h
c_pzasusb8.o: ../banned.h
c_pzasusb8.o: ../block.h
c_pzasusb8.o: ../byteswap.h
c_pzasusb8.o: ../calcerr.h
c_pzasusb8.o: ../cmath.h
c_pzasusb8.o: ../config.h
c_pzasusb8.o: ../custom.h
c_pzasusb8.o: ../decl.h
c_pzasusb8.o: ../endian_calc.h
c_pzasusb8.o: ../hash.h
c_pzasusb8.o: ../have_ban_pragma.h
c_pzasusb8.o: ../have_const.h
c_pzasusb8.o: ../have_memmv.h
c_pzasusb8.o: ../have_newstr.h
c_pzasusb8.o: ../have_stdlib.h
c_pzasusb8.o: ../have_string.h
c_pzasusb8.o: ../have_unused.h
c_pzasusb8.o: ../longbits.h
c_pzasusb8.o: ../nametype.h
c_pzasusb8.o: ../qmath.h
c_pzasusb8.o: ../sha1.h
c_pzasusb8.o: ../str.h
c_pzasusb8.o: ../value.h
c_pzasusb8.o: ../zmath.h
c_pzasusb8.o: c_pzasusb8.c
c_register.o: ../alloc.h
c_register.o: ../attribute.h
c_register.o: ../banned.h
c_register.o: ../block.h
c_register.o: ../byteswap.h
c_register.o: ../calc.h
c_register.o: ../calcerr.h
c_register.o: ../cmath.h
c_register.o: ../config.h
c_register.o: ../custom.h
c_register.o: ../decl.h
c_register.o: ../endian_calc.h
c_register.o: ../hash.h
c_register.o: ../have_ban_pragma.h
c_register.o: ../have_const.h
c_register.o: ../have_memmv.h
c_register.o: ../have_newstr.h
c_register.o: ../have_stdlib.h
c_register.o: ../have_string.h
c_register.o: ../have_unused.h
c_register.o: ../longbits.h
c_register.o: ../nametype.h
c_register.o: ../qmath.h
c_register.o: ../sha1.h
c_register.o: ../str.h
c_register.o: ../value.h
c_register.o: ../zmath.h
c_register.o: c_register.c
c_sysinfo.o: ../alloc.h
c_sysinfo.o: ../attribute.h
c_sysinfo.o: ../banned.h
c_sysinfo.o: ../block.h
c_sysinfo.o: ../byteswap.h
c_sysinfo.o: ../calc.h
c_sysinfo.o: ../calcerr.h
c_sysinfo.o: ../cmath.h
c_sysinfo.o: ../conf.h
c_sysinfo.o: ../config.h
c_sysinfo.o: ../custom.h
c_sysinfo.o: ../decl.h
c_sysinfo.o: ../endian_calc.h
c_sysinfo.o: ../fposval.h
c_sysinfo.o: ../hash.h
c_sysinfo.o: ../have_ban_pragma.h
c_sysinfo.o: ../have_const.h
c_sysinfo.o: ../have_memmv.h
c_sysinfo.o: ../have_newstr.h
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
c_sysinfo.o: ../zmath.h
c_sysinfo.o: ../zrand.h
c_sysinfo.o: ../zrandom.h
c_sysinfo.o: c_sysinfo.c
custtbl.o: ../alloc.h
custtbl.o: ../attribute.h
custtbl.o: ../banned.h
custtbl.o: ../block.h
custtbl.o: ../byteswap.h
custtbl.o: ../calcerr.h
custtbl.o: ../cmath.h
custtbl.o: ../config.h
custtbl.o: ../custom.h
custtbl.o: ../decl.h
custtbl.o: ../endian_calc.h
custtbl.o: ../hash.h
custtbl.o: ../have_ban_pragma.h
custtbl.o: ../have_const.h
custtbl.o: ../have_memmv.h
custtbl.o: ../have_newstr.h
custtbl.o: ../have_stdlib.h
custtbl.o: ../have_string.h
custtbl.o: ../longbits.h
custtbl.o: ../nametype.h
custtbl.o: ../qmath.h
custtbl.o: ../sha1.h
custtbl.o: ../str.h
custtbl.o: ../value.h
custtbl.o: ../zmath.h
custtbl.o: custtbl.c
