#!/bin/make
#
# custom - makefile for calc custom routines
#
# Copyright (C) 1999,2004  Landon Curt Noll
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
# @(#) $Revision: 29.24 $
# @(#) $Id: Makefile,v 29.24 2004/07/29 08:38:18 chongo Exp $
# @(#) $Source: /usr/local/src/cmd/calc/custom/RCS/Makefile,v $
#
# Under source code control:	1997/03/09 02:28:54
# File existed as early as:	1997
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-) http://www.isthe.com/chongo/tech/comp/calc/
#
# calculator by David I. Bell with help/mods from others
# Makefile by Landon Curt Noll


##############################################################################
#-=-=-=-=-=-=-=-=- You may want to change some values below -=-=-=-=-=-=-=-=-#
##############################################################################

# The custom calc resource files to install
#
# Put your custom calc resource files here.
#
CUSTOM_CALC_FILES= argv.cal halflen.cal pzasusb8.cal pmodm127.cal

# The custom help files to install
#
# Put your custom help files here.
#
CUSTOM_HELP= argv devnull help sysinfo pzasusb8 pmodm127

# Any .h files that are needed by programs that use libcustcalc.a
#
# Put any .h files that you add which might be useful to other
# programs here.
#
CUSTOM_H_SRC=

# Any .c files that are needed to build libcustcalc.a.
# Don't put ${REQUIRED_SRC} files in this list.
#
# There MUST be a .c in CUSTOM_SRC for every .o in CUSTOM_OBJ.
#
# Put your custom .c files here.
#
CUSTOM_SRC= c_argv.c c_devnull.c c_help.c c_sysinfo.c c_pzasusb8.c c_pmodm127.c

# Any .o files that are needed by program that use libcustcalc.a.
# Don't put ${REQUIRED_OBJ} files in this list.
#
# There MUST be a .c in CUSTOM_SRC for every .o in CUSTOM_OBJ.
#
# Put your custom .o files here.
#
CUSTOM_OBJ= c_argv.o c_devnull.o c_help.o c_sysinfo.o c_pzasusb8.o c_pmodm127.o

##############################################################################
#-=-=-=-=-=-=- Defaults in case you want to build from this dir -=-=-=-=-=-=-#
##############################################################################

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# Makefile debug
#
# Q=@	do not echo internal makefile actions (quiet mode)
# Q=	echo internal makefile actions (debug / verbose mode)
#
#Q=
Q=@

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

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# Debug/Optimize options for ${CC}
#
DEBUG= -O
#DEBUG= -O -g
#DEBUG= -O -g3
#DEBUG= -O1
#DEBUG= -O1 -g
#DEBUG= -O1 -g3
#DEBUG= -O2
#DEBUG= -O2 -g
#DEBUG= -O2 -g3
#DEBUG= -O2 -ipa
#DEBUG= -O2 -g3 -ipa
#DEBUG= -O3
#DEBUG= -O3 -g
#DEBUG= -O3 -g3
#DEBUG= -O3 -ipa
#DEBUG= -O3 -g3 -ipa
#DEBUG= -g
#DEBUG= -g3
#DEBUG= -gx
#DEBUG= -WM,-g
#DEBUG=

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# On systems that have dynamic shared libs, you may want want to disable them
# for faster calc startup.
#
#    System type    NO_SHARED recommendation
#
#	BSD	    NO_SHARED=
#	SYSV	    NO_SHARED= -dn
#	IRIX	    NO_SHARED= -non_shared
#	disable	    NO_SHARED=
#
# If in doubt, use NO_SHARED=
#
NO_SHARED=
#NO_SHARED= -dn
#NO_SHARED= -non_shared

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# Some systems require one to use ranlib to add a symbol table to
# a *.a link library.  Set RANLIB to the utility that performs this action.
# Set RANLIB to : if your system does not need such a utility.
#
#RANLIB=ranlib
RANLIB=:

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# Normally certain files depend on the Makefile.  If the Makefile is
# changed, then certain steps should be redone.	 If MAKE_FILE is
# set to Makefile, then these files will depend on Makefile.  If
# MAKE_FILE is empty, they they wont.
#
# If in doubt, set MAKE_FILE to Makefile
#
MAKE_FILE= Makefile
#MAKE_FILE=

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# If you do not wish to use purify, leave PURIFY commented out.
#
# If in doubt, leave PURIFY commented out.
#
#PURIFY= purify -logfile=pure.out
#PURIFY= purify

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
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

###
#
# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# Select your compiler type by commenting out one of the cc sets below:
#
# CCOPT are flags given to ${CC} for optimization
# CCWARN are flags given to ${CC} for warning message control
# CCMISC are misc flags given to ${CC}
#
# CFLAGS are all flags given to ${CC} [[often includes CCOPT, CCWARN, CCMISC]]
# ICFLAGS are given to ${CC} for intermediate progs
#
# LDFLAGS are flags given to ${CC} for linking .o files
# ILDFLAGS are flags given to ${CC} for linking .o files for intermediate progs
#
# CC is how the the C compiler is invoked
#
###
#
# common cc set
#
CCWARN=
CCOPT= ${DEBUG} ${NO_SHARED}
CCMISC=
#
CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC} ${ALLOW_CUSTOM}
ICFLAGS= ${CCWARN} ${CCMISC}
#
LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
ILDFLAGS=
#
CC= ${PURIFY} cc

##############################################################################
#-=-=-=-=-=-=-=-=- Be careful if you change something below -=-=-=-=-=-=-=-=-#
##############################################################################

# These .c files are required for the main custom interface and
# for the custom support functions.
#
# There MUST be a .c for every .o in REQUIRED_OBJ.
#
REQUIRED_SRC= custtbl.c

# These .o files correspond to the .c files in REQUIRED_SRC
#
# There MUST be a .o for every .c in REQUIRED_SRC.
#
REQUIRED_OBJ= custtbl.o

# These .h files are installed under ${CUSTOMCALDIR} by the install rule.
#
INSTALL_H_SRC= ${CUSTOM_H_SRC}

# These .c files are used to form libcustcalc.a.
#
CUSTCALC_SRC= ${REQUIRED_SRC} ${CUSTOM_SRC}

# These .o files are used to form libcustcalc.a.
#
CUSTCALC_OBJ= ${REQUIRED_OBJ} ${CUSTOM_OBJ}

# These .c files are used to build the dependency list
#
C_SRC= ${REQUIRED_SRC} ${CUSTOM_SRC}

# These .h files are used to build the dependecy list
#
H_SRC= ${CUSTOM_H_SRC}

# These files are found (but not built) in the distribution
#
# The CUSTOM_CAL and HOW_TO_ADD are files distributed from this
# directory but are installed as help files from the help/Makefile.
#
DISTLIST= ${CUSTCALC_SRC} ${CUSTOM_CALC_FILES} ${CUSTOM_HELP} \
	${INSTALL_H_SRC} CUSTOM_CAL HOW_TO_ADD ${MAKE_FILE}

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST= ${CUSTCALC_SRC} ${INSTALL_H_SRC} ${MAKE_FILE} HOW_TO_ADD

# This is the custom .a link library that is build
#
CALC_LIBCUSTOM= libcustcalc.a

# complete list of targets
#
TARGETS= ${CALC_LIBCUSTOM} ${CUSTCALC_OBJ}

# required vars
#
SHELL= /bin/sh
MAKE_FILE= Makefile

# standard tools
#
SED= sed
MAKEDEPEND= makedepend
CHMOD= chmod
SORT= sort
CMP= cmp

##
#
# Standard rules and targets
#
##

all: ${TARGETS} ${INSTALL_H_SRC} ${CUSTOM_CALC_FILES} \
	${CUSTOM_HELP} ${MAKE_FILE} .all

libcustcalc.a: ${CUSTCALC_OBJ} ${MAKE_FILE} ../${MAKE_FILE}
	-rm -f libcustcalc.a
	ar qc libcustcalc.a ${CUSTCALC_OBJ}
	${RANLIB} libcustcalc.a

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
	rm -f .all
	touch .all

##
#
# File list generation.	 You can ignore this section.
#
#
# We will form the names of source files as if they were in a
# sub-directory called calc/lib.
#
# NOTE: Due to bogus shells found on one common system we must have
#	an non-emoty else clause for every if condition.  *sigh*
#
##

distlist: ${DISTLIST}
	${Q}for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo custom/$$i; \
	    fi; \
	done

distdir:
	${Q}echo custom

calcliblist: ${CALCLIBLIST}
	${Q}for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo custom/$$i; \
	    fi; \
	done

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
	${Q}echo forming custom/skel
	-${Q}rm -rf skel
	${Q}mkdir skel
	${Q}mkdir skel/custom
	-${Q}for i in ${C_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' \
		    "$$i" > "skel/custom/$$i"; \
	    fi; \
	done
	-${Q}for i in ${H_SRC} /dev/null; do \
		if [ "$$i" = "/dev/null" ]; then \
		    continue; \
		fi; \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#if !defined($$tag)" > "skel/custom/$$i"; \
		echo "#define $$tag" >> "skel/custom/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" \
		    >> "skel/custom/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/custom/$$i"; \
	done
	${Q}(cd ..; ${MAKE} hsrc)
	${Q}for i in `cd ..; ${MAKE} h_list 2>&1 | \
			     ${SED} -e '/Entering directory/d' \
				    -e '/Nothing to be done/d' \
				    -e '/Leaving directory/d'` /dev/null; do \
		if [ "$$i" = "/dev/null" ]; then \
		    continue; \
		fi; \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#if !defined($$tag)" > "skel/$$i"; \
		echo "#define $$tag" >> "skel/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "../$$i" \
		    >> "skel/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/$$i"; \
	done
	-${Q}rm -f skel/custom/makedep.out
	${Q}echo custom/skel formed
	${Q}echo forming custom dependency list
	${Q}echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > \
	    skel/custom/makedep.out
	${Q}cd skel/custom; ${MAKEDEPEND} -w 1 -f makedep.out ${C_SRC}
	-${Q}for i in ${C_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo "$$i" | ${SED} 's/^\(.*\)\.c/\1.o: \1.c/'; \
	    fi; \
	done >> skel/custom/makedep.out
	${Q}echo custom dependency list formed
	${Q}echo forming new custom/Makefile
	-${Q}rm -f Makefile.bak
	${Q}mv Makefile Makefile.bak
	${Q}${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' Makefile.bak > Makefile
	${Q}echo "" >> Makefile
	${Q}${SED} -n '3,$$p' skel/custom/makedep.out | \
		LANG=C ${SORT} -u >> Makefile
	-${Q}rm -rf skel
	-${Q}if ${CMP} -s Makefile.bak Makefile; then \
		echo 'custom Makefile was already up to date'; \
		mv -f Makefile.bak Makefile; \
	else \
		rm -f Makefile.tmp; \
		mv Makefile Makefile.tmp; \
		if [ -d RCS ]; then \
		    co -l Makefile; \
		fi ;\
		mv Makefile.tmp Makefile; \
		if [ -d RCS ]; then \
		echo 'new custom Makefile formed -- you need to check it in'; \
		fi; \
	fi

##
#
# rpm rules
#
##

echo_inst_files:
	${Q}for i in ${INSTALL_H_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CUSTOMINCDIR}/$$i; \
	    fi; \
	done
	${Q}for i in ${CUSTOM_HELP} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CUSTOMHELPDIR}/$$i; \
	    fi; \
	done
	${Q}for i in ${CUSTOM_CALC_FILES} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo __file__ ${CUSTOMCALDIR}/$$i; \
	    fi; \
	done
	${Q}for i in ${CALC_LIBCUSTOM} /dev/null; do \
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
	-rm -f ${CUSTCALC_OBJ}

clobber:
	-rm -f ${TARGETS}
	rm -f .all Makefile.tmp Makefile.bak

install: all
	-${Q}if [ ! -d $T${INCDIR} ]; then \
	    echo mkdir $T${INCDIR}; \
	    mkdir $T${INCDIR}; \
	    if [ ! -d "$T${INCDIR}" ]; then \
		echo mkdir -p "$T${INCDIR}"; \
		mkdir -p "$T${INCDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${INCDIR}; \
	    ${CHMOD} 0755 $T${INCDIR}; \
	else \
	    true; \
	fi
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
	-${Q}if [ ! -d $T${CALC_INCDIR} ]; then \
	    echo mkdir $T${CALC_INCDIR}; \
	    mkdir $T${CALC_INCDIR}; \
	    if [ ! -d "$T${CALC_INCDIR}" ]; then \
		echo mkdir -p "$T${CALC_INCDIR}"; \
		mkdir -p "$T${CALC_INCDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${CALC_INCDIR}; \
	    ${CHMOD} 0755 $T${CALC_INCDIR}; \
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
	-${Q}if [ ! -d $T${CUSTOMCALDIR} ]; then \
	    echo mkdir $T${CUSTOMCALDIR}; \
	    mkdir $T${CUSTOMCALDIR}; \
	    if [ ! -d "$T${CUSTOMCALDIR}" ]; then \
		echo mkdir -p "$T${CUSTOMCALDIR}"; \
		mkdir -p "$T${CUSTOMCALDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${CUSTOMCALDIR}; \
	    ${CHMOD} 0755 $T${CUSTOMCALDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${CUSTOMHELPDIR} ]; then \
	    echo mkdir $T${CUSTOMHELPDIR}; \
	    mkdir $T${CUSTOMHELPDIR}; \
	    if [ ! -d "$T${CUSTOMHELPDIR}" ]; then \
		echo mkdir -p "$T${CUSTOMHELPDIR}"; \
		mkdir -p "$T${CUSTOMHELPDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${CUSTOMHELPDIR}; \
	    ${CHMOD} 0755 $T${CUSTOMHELPDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${CUSTOMINCDIR} ]; then \
	    echo mkdir $T${CUSTOMINCDIR}; \
	    mkdir $T${CUSTOMINCDIR}; \
	    if [ ! -d "$T${CUSTOMINCDIR}" ]; then \
		echo mkdir -p "$T${CUSTOMINCDIR}"; \
		mkdir -p "$T${CUSTOMINCDIR}"; \
	    fi; \
	    echo ${CHMOD} 0755 $T${CUSTOMINCDIR}; \
	    ${CHMOD} 0755 $T${CUSTOMINCDIR}; \
	else \
	    true; \
	fi
	-${Q}for i in ${INSTALL_H_SRC} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s tmp $T${CUSTOMINCDIR}/$$i; then \
		true; \
	    else \
		rm -f $T${CUSTOMINCDIR}/$$i.new; \
		cp -f $$i $T${CUSTOMINCDIR}/$$i.new; \
		${CHMOD} 0444 $T${CUSTOMINCDIR}/$$i.new; \
		mv -f $T${CUSTOMINCDIR}/$$i.new $T${CUSTOMINCDIR}/$$i; \
		echo "installed $T${CUSTOMINCDIR}/$$i"; \
	    fi; \
	done
	-${Q}rm -f tmp
	-${Q}for i in ${CUSTOM_CALC_FILES} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s $$i $T${CUSTOMCALDIR}/$$i; then \
		true; \
	    else \
		rm -f $T${CUSTOMCALDIR}/$$i.new; \
		cp -f $$i $T${CUSTOMCALDIR}/$$i.new; \
		${CHMOD} 0444 $T${CUSTOMCALDIR}/$$i.new; \
		mv -f $T${CUSTOMCALDIR}/$$i.new $T${CUSTOMCALDIR}/$$i; \
		echo "installed $T${CUSTOMCALDIR}/$$i"; \
	    fi; \
	done
	-${Q}for i in ${CUSTOM_HELP} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if ${CMP} -s $$i $T${CUSTOMHELPDIR}/$$i; then \
		true; \
	    else \
		rm -f $T${CUSTOMHELPDIR}/$$i.new; \
		cp -f $$i $T${CUSTOMHELPDIR}/$$i.new; \
		${CHMOD} 0444 $T${CUSTOMHELPDIR}/$$i.new; \
		mv -f $T${CUSTOMHELPDIR}/$$i.new $T${CUSTOMHELPDIR}/$$i; \
		echo "installed $T${CUSTOMHELPDIR}/$$i"; \
	    fi; \
	done
	-${Q}if [ ! -z ${ALLOW_CUSTOM} ]; then \
	    if ${CMP} -s libcustcalc.a $T${CUSTOMCALDIR}/libcustcalc.a; then \
		true; \
	    else \
		rm -f $T${CUSTOMCALDIR}/libcustcalc.a.new; \
		cp -f libcustcalc.a $T${CUSTOMCALDIR}/libcustcalc.a.new; \
		${CHMOD} 0644 $T${CUSTOMCALDIR}/libcustcalc.a.new; \
		mv -f $T${CUSTOMCALDIR}/libcustcalc.a.new \
		      $T${CUSTOMCALDIR}/libcustcalc.a; \
		${RANLIB} $T${CUSTOMCALDIR}/libcustcalc.a; \
		echo "installed $T${CUSTOMCALDIR}/libcustcalc.a"; \
	    fi; \
	fi

##
#
# make depend stuff
#
##

# DO NOT DELETE THIS LINE

c_argv.o: ../alloc.h
c_argv.o: ../block.h
c_argv.o: ../byteswap.h
c_argv.o: ../calc.h
c_argv.o: ../calcerr.h
c_argv.o: ../cmath.h
c_argv.o: ../config.h
c_argv.o: ../custom.h
c_argv.o: ../endian_calc.h
c_argv.o: ../hash.h
c_argv.o: ../have_const.h
c_argv.o: ../have_malloc.h
c_argv.o: ../have_memmv.h
c_argv.o: ../have_newstr.h
c_argv.o: ../have_stdlib.h
c_argv.o: ../have_string.h
c_argv.o: ../have_unused.h
c_argv.o: ../longbits.h
c_argv.o: ../md5.h
c_argv.o: ../nametype.h
c_argv.o: ../qmath.h
c_argv.o: ../shs.h
c_argv.o: ../shs1.h
c_argv.o: ../string.h
c_argv.o: ../value.h
c_argv.o: ../win32dll.h
c_argv.o: ../zmath.h
c_argv.o: c_argv.c
c_devnull.o: ../alloc.h
c_devnull.o: ../block.h
c_devnull.o: ../byteswap.h
c_devnull.o: ../calcerr.h
c_devnull.o: ../cmath.h
c_devnull.o: ../config.h
c_devnull.o: ../custom.h
c_devnull.o: ../endian_calc.h
c_devnull.o: ../hash.h
c_devnull.o: ../have_const.h
c_devnull.o: ../have_malloc.h
c_devnull.o: ../have_memmv.h
c_devnull.o: ../have_newstr.h
c_devnull.o: ../have_stdlib.h
c_devnull.o: ../have_string.h
c_devnull.o: ../have_unused.h
c_devnull.o: ../longbits.h
c_devnull.o: ../md5.h
c_devnull.o: ../nametype.h
c_devnull.o: ../qmath.h
c_devnull.o: ../shs.h
c_devnull.o: ../shs1.h
c_devnull.o: ../string.h
c_devnull.o: ../value.h
c_devnull.o: ../win32dll.h
c_devnull.o: ../zmath.h
c_devnull.o: c_devnull.c
c_help.o: ../alloc.h
c_help.o: ../block.h
c_help.o: ../byteswap.h
c_help.o: ../calcerr.h
c_help.o: ../cmath.h
c_help.o: ../config.h
c_help.o: ../custom.h
c_help.o: ../endian_calc.h
c_help.o: ../hash.h
c_help.o: ../have_const.h
c_help.o: ../have_malloc.h
c_help.o: ../have_memmv.h
c_help.o: ../have_newstr.h
c_help.o: ../have_stdlib.h
c_help.o: ../have_string.h
c_help.o: ../have_unused.h
c_help.o: ../longbits.h
c_help.o: ../md5.h
c_help.o: ../nametype.h
c_help.o: ../qmath.h
c_help.o: ../shs.h
c_help.o: ../shs1.h
c_help.o: ../string.h
c_help.o: ../value.h
c_help.o: ../win32dll.h
c_help.o: ../zmath.h
c_help.o: c_help.c
c_pmodm127.o: ../alloc.h
c_pmodm127.o: ../block.h
c_pmodm127.o: ../byteswap.h
c_pmodm127.o: ../calcerr.h
c_pmodm127.o: ../cmath.h
c_pmodm127.o: ../config.h
c_pmodm127.o: ../custom.h
c_pmodm127.o: ../endian_calc.h
c_pmodm127.o: ../hash.h
c_pmodm127.o: ../have_const.h
c_pmodm127.o: ../have_malloc.h
c_pmodm127.o: ../have_memmv.h
c_pmodm127.o: ../have_newstr.h
c_pmodm127.o: ../have_stdlib.h
c_pmodm127.o: ../have_string.h
c_pmodm127.o: ../have_unused.h
c_pmodm127.o: ../longbits.h
c_pmodm127.o: ../md5.h
c_pmodm127.o: ../nametype.h
c_pmodm127.o: ../qmath.h
c_pmodm127.o: ../shs.h
c_pmodm127.o: ../shs1.h
c_pmodm127.o: ../string.h
c_pmodm127.o: ../value.h
c_pmodm127.o: ../win32dll.h
c_pmodm127.o: ../zmath.h
c_pmodm127.o: c_pmodm127.c
c_pzasusb8.o: ../alloc.h
c_pzasusb8.o: ../block.h
c_pzasusb8.o: ../byteswap.h
c_pzasusb8.o: ../calcerr.h
c_pzasusb8.o: ../cmath.h
c_pzasusb8.o: ../config.h
c_pzasusb8.o: ../custom.h
c_pzasusb8.o: ../endian_calc.h
c_pzasusb8.o: ../hash.h
c_pzasusb8.o: ../have_const.h
c_pzasusb8.o: ../have_malloc.h
c_pzasusb8.o: ../have_memmv.h
c_pzasusb8.o: ../have_newstr.h
c_pzasusb8.o: ../have_stdlib.h
c_pzasusb8.o: ../have_string.h
c_pzasusb8.o: ../have_unused.h
c_pzasusb8.o: ../longbits.h
c_pzasusb8.o: ../md5.h
c_pzasusb8.o: ../nametype.h
c_pzasusb8.o: ../qmath.h
c_pzasusb8.o: ../shs.h
c_pzasusb8.o: ../shs1.h
c_pzasusb8.o: ../string.h
c_pzasusb8.o: ../value.h
c_pzasusb8.o: ../win32dll.h
c_pzasusb8.o: ../zmath.h
c_pzasusb8.o: c_pzasusb8.c
c_sysinfo.o: ../alloc.h
c_sysinfo.o: ../block.h
c_sysinfo.o: ../byteswap.h
c_sysinfo.o: ../calc.h
c_sysinfo.o: ../calcerr.h
c_sysinfo.o: ../cmath.h
c_sysinfo.o: ../conf.h
c_sysinfo.o: ../config.h
c_sysinfo.o: ../custom.h
c_sysinfo.o: ../endian_calc.h
c_sysinfo.o: ../fposval.h
c_sysinfo.o: ../hash.h
c_sysinfo.o: ../have_const.h
c_sysinfo.o: ../have_malloc.h
c_sysinfo.o: ../have_memmv.h
c_sysinfo.o: ../have_newstr.h
c_sysinfo.o: ../have_stdlib.h
c_sysinfo.o: ../have_string.h
c_sysinfo.o: ../have_unused.h
c_sysinfo.o: ../hist.h
c_sysinfo.o: ../longbits.h
c_sysinfo.o: ../md5.h
c_sysinfo.o: ../nametype.h
c_sysinfo.o: ../prime.h
c_sysinfo.o: ../qmath.h
c_sysinfo.o: ../shs.h
c_sysinfo.o: ../shs1.h
c_sysinfo.o: ../string.h
c_sysinfo.o: ../value.h
c_sysinfo.o: ../win32dll.h
c_sysinfo.o: ../zmath.h
c_sysinfo.o: ../zrand.h
c_sysinfo.o: ../zrandom.h
c_sysinfo.o: c_sysinfo.c
custtbl.o: ../alloc.h
custtbl.o: ../block.h
custtbl.o: ../byteswap.h
custtbl.o: ../calcerr.h
custtbl.o: ../cmath.h
custtbl.o: ../config.h
custtbl.o: ../custom.h
custtbl.o: ../endian_calc.h
custtbl.o: ../hash.h
custtbl.o: ../have_const.h
custtbl.o: ../have_malloc.h
custtbl.o: ../have_memmv.h
custtbl.o: ../have_newstr.h
custtbl.o: ../have_stdlib.h
custtbl.o: ../have_string.h
custtbl.o: ../longbits.h
custtbl.o: ../md5.h
custtbl.o: ../nametype.h
custtbl.o: ../qmath.h
custtbl.o: ../shs.h
custtbl.o: ../shs1.h
custtbl.o: ../string.h
custtbl.o: ../value.h
custtbl.o: ../win32dll.h
custtbl.o: ../zmath.h
custtbl.o: custtbl.c
