#
# sample - makefile for calc sample programs
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
# @(#) $Revision: 29.3 $
# @(#) $Id: Makefile,v 29.3 2000/12/17 12:28:31 chongo Exp $
# @(#) $Source: /usr/local/src/cmd/calc/sample/RCS/Makefile,v $
#
# Under source code control:	1997/04/19 22:46:49
# File existed as early as:	1997
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-)		http://www.isthe.com/chongo/tech/comp/calc/
#
# calculator by David I. Bell with help/mods from others
# Makefile by Landon Curt Noll

##############################################################################
#-=-=-=-=-=-=-=-=- You may want to change some values below -=-=-=-=-=-=-=-=-#
##############################################################################

# Any .h files that are needed by programs that use libcustcalc.a
# Don't put ${REQUIRED_H_SRC} files in this list.
#
# Put any .h files that you add which might be useful to other
# programs here.
#
SAMPLE_H_SRC=

# Any .c files that are needed to build libcustcalc.a.
# Don't put ${REQUIRED_SRC} files in this list.
#
# There MUST be a .c in SAMPLE_SRC for every .o in SAMPLE_OBJ.
#
# Put your sample .c files here.
#
SAMPLE_SRC= many_random.c test_random.c

# Any .o files that are needed by program that use libcustcalc.a.
# Don't put ${REQUIRED_OBJ} files in this list.
#
# There MUST be a .c in SAMPLE_SRC for every .o in SAMPLE_OBJ.
#
# Put your sample .o files here.
#
SAMPLE_OBJ= many_random.o test_random.o

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

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# where to install things
#
# ${TOPDIR} is the directory under which the calc directory will be placed.
# ${LIBDIR} is where the *.cal, *.h, *.a, bindings and help dir are installed.
# ${HELPDIR} is where the help directory is installed.
#
TOPDIR= /usr/local/lib
#TOPDIR= /usr/lib
#TOPDIR= /usr/libdata
#TOPDIR= /usr/contrib/lib
#
LIBDIR= ${TOPDIR}/calc
HELPDIR= ${LIBDIR}/help

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

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# The sample routines need to be compiled with libcalc.a.  The ${CALC_LIBS}
# variable tells where this link library may be found
#
# CALC_LIBS= ../libcalc.a ...	       # compile with libcalc.a in dir above
# CALC_LIBS= ${LIBDIR}/libcalc.a ...   # compile the installed libcalc.a
# CALC_LIBS= -lcalc ...		       # compile with the system libcalc.a
#
# If in doubt, use CALC_LIBS= ../libcalc.a
#
CALC_LIBS= ../libcalc.a ../custom/libcustcalc.a
#CALC_LIBS= ${LIBDIR}/libcalc.a ${LIBDIR}/libcustcalc.a
#CALC_LIBS= -lcalc -lcustcalc

###
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
# LCC how the the C compiler is invoked on locally executed intermediate progs
# CC is how the the C compiler is invoked (with an optional Purify)
#
###
#
# default set
#
# Normally these values are passed in from the upper level Makefile.
# Change these as needed if you intend to build directly in this directory.
#
# for better performance, set the following above:
#     DEBUG= -O2
#
CCWARN= -Wall -Wno-implicit -Wno-comment
CCOPT= ${DEBUG} ${NO_SHARED}
CCMISC=
#
CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
ICFLAGS= ${CCWARN} ${CCMISC}
#
LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
ILDFLAGS=
#
LCC= gcc
CC= ${PURIFY} ${LCC}

##############################################################################
#-=-=-=-=-=-=-=-=- Be careful if you change something below -=-=-=-=-=-=-=-=-#
##############################################################################

# These .c files are used to build the dependency list
#
C_SRC= ${SAMPLE_SRC}

# These .h files are used to build the dependecy list
#
H_SRC= ${SAMPLE_H_SRC}

# These files are found (but not built) in the distribution
#
# The SAMPLE_CAL and HOW_TO_ADD are files distributed from this
# directory but are installed as help files from the help/Makefile.
#
DISTLIST= ${C_SRC} ${H_SRC} ${MAKE_FILE} README_SAMPLE

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST= ${C_SRC} ${H_SRC} ${MAKE_FILE} README_SAMPLE

# complete list of targets
#
TARGETS= many_random test_random

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

##
#
# Standard rules and targets
#
##

all: ${TARGETS} .all

test_random.o: test_random.c
	${CC} ${CFLAGS} ${ALLOW_CUSTOM} test_random.c -c

test_random: test_random.o ../libcalc.a
	${CC} ${LDFLAGS} test_random.o ${CALC_LIBS} ${LD_DEBUG} -o test_random

many_random.o: many_random.c
	${CC} ${CFLAGS} ${ALLOW_CUSTOM} many_random.c -c

many_random: many_random.o ../libcalc.a
	${CC} ${LDFLAGS} many_random.o ${CALC_LIBS} ${LD_DEBUG} -o many_random

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
	${Q}for i in ${DISTLIST}; do \
		echo sample/$$i; \
	done

distdir:
	${Q}echo sample

calcliblist:
	${Q}for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo sample/$$i; \
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
	${Q}echo forming sample/skel
	-${Q}rm -rf skel
	${Q}mkdir skel
	${Q}mkdir skel/sample
	-${Q}for i in ${C_SRC}; do \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' \
		    "$$i" > "skel/sample/$$i"; \
	done
	-${Q}for i in ${H_SRC} /dev/null; do \
		if [ X"$$i" != X"/dev/null" ]; then \
		    tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		    echo "#if !defined($$tag)" > "skel/sample/$$i"; \
		    echo "#define $$tag" >> "skel/sample/$$i"; \
		    ${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" \
			>> "skel/sample/$$i"; \
		    echo '#endif /* '"$$tag"' */' >> "skel/sample/$$i"; \
		fi; \
	done
	${Q}(cd ..; ${MAKE} hsrc)
	${Q}for i in `cd ..; ${MAKE} h_list 2>&1 | \
			     ${SED} -e '/Entering directory/d' \
				    -e '/Nothing to be done/d' \
				    -e '/Leaving directory/d'`; do \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#if !defined($$tag)" > "skel/$$i"; \
		echo "#define $$tag" >> "skel/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "../$$i" \
		    >> "skel/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/$$i"; \
	done
	-${Q}rm -f skel/sample/makedep.out
	${Q}echo sample/skel formed
	${Q}echo forming sample dependency list
	${Q}echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > \
	    skel/sample/makedep.out
	${Q}cd skel/sample; ${MAKEDEPEND} -w 1 -m -f makedep.out -I.. ${C_SRC}
	-${Q}for i in ${C_SRC}; do \
		echo "$$i" | ${SED} 's/^\(.*\)\.c/\1.o: \1.c/'; \
	done >> skel/sample/makedep.out
	${Q}echo sample dependency list formed
	${Q}echo forming new sample/Makefile
	-${Q}rm -f Makefile.bak
	${Q}mv Makefile Makefile.bak
	${Q}${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' Makefile.bak > Makefile
	${Q}echo "" >> Makefile
	${Q}${SED} -n '3,$$p' skel/sample/makedep.out | ${SORT} -u >> Makefile
	-${Q}rm -rf skel
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

##
#
# Utility rules
#
##

clean:
	-rm -f ${SAMPLE_OBJ} core

clobber:
	-rm -f ${SAMPLE_OBJ}
	-rm -f ${TARGETS}
	rm -f .all Makefile.tmp sample

# for right now we will not install anything
install: all

##
#
# make depend stuff
#
##

# DO NOT DELETE THIS LINE

many_random.o: ../alloc.h
many_random.o: ../block.h
many_random.o: ../byteswap.h
many_random.o: ../calc.h
many_random.o: ../calcerr.h
many_random.o: ../cmath.h
many_random.o: ../config.h
many_random.o: ../endian_calc.h
many_random.o: ../hash.h
many_random.o: ../have_const.h
many_random.o: ../have_malloc.h
many_random.o: ../have_memmv.h
many_random.o: ../have_newstr.h
many_random.o: ../have_stdlib.h
many_random.o: ../have_string.h
many_random.o: ../lib_util.h
many_random.o: ../longbits.h
many_random.o: ../md5.h
many_random.o: ../nametype.h
many_random.o: ../qmath.h
many_random.o: ../shs.h
many_random.o: ../shs1.h
many_random.o: ../string.h
many_random.o: ../value.h
many_random.o: ../zmath.h
many_random.o: ../zrandom.h
many_random.o: many_random.c
test_random.o: ../alloc.h
test_random.o: ../block.h
test_random.o: ../byteswap.h
test_random.o: ../calc.h
test_random.o: ../calcerr.h
test_random.o: ../cmath.h
test_random.o: ../config.h
test_random.o: ../endian_calc.h
test_random.o: ../hash.h
test_random.o: ../have_const.h
test_random.o: ../have_malloc.h
test_random.o: ../have_memmv.h
test_random.o: ../have_newstr.h
test_random.o: ../have_stdlib.h
test_random.o: ../have_string.h
test_random.o: ../lib_util.h
test_random.o: ../longbits.h
test_random.o: ../md5.h
test_random.o: ../nametype.h
test_random.o: ../qmath.h
test_random.o: ../shs.h
test_random.o: ../shs1.h
test_random.o: ../string.h
test_random.o: ../value.h
test_random.o: ../zmath.h
test_random.o: ../zrandom.h
test_random.o: test_random.c
