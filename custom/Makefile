#
# custom - makefile for calc custom routines
#
# Copyright (c) 1997 Landon Curt Noll
#
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright, this permission notice and text
# this comment, and the disclaimer below appear in all of the following:
#
#	supporting documentation
#	source copies
#	source works derived from this source
#	binaries derived from this source or from derived source
#
# LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
# EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#
# Comments, suggestions, bug fixes and questions about these routines
# are welcome.  Send EMail to the address given below.
#
# Happy bit twiddling,
#
#			Landon Curt Noll
#
#			chongo@toad.com
#			...!{pyramid,sun,uunet}!hoptoad!chongo
#
# chongo was here	/\../\

##############################################################################
#-=-=-=-=-=-=-=-=- You may want to change some values below -=-=-=-=-=-=-=-=-#
##############################################################################

# The custom calc library files to install
#
# Put your custom calc library files here.
#
CUSTOM_CALC_FILES= argv.cal halflen.cal

# The custom help files to install
#
# Put your custom help files here.
#
CUSTOM_HELP= argv devnull help sysinfo

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
CUSTOM_SRC= c_argv.c c_devnull.c c_help.c c_sysinfo.c

# Any .o files that are needed by program that use libcustcalc.a.
# Don't put ${REQUIRED_OBJ} files in this list.
#
# There MUST be a .c in CUSTOM_SRC for every .o in CUSTOM_OBJ.
#
# Put your custom .o files here.
#
CUSTOM_OBJ= c_argv.o c_devnull.o c_help.o c_sysinfo.o

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
# ${CUSTOMLIBDIR} is where custom lib files are installed.
# ${CUSTOMHELPDIR} is where custom help files are installed.
#
TOPDIR= /usr/local/lib
#TOPDIR= /usr/lib
#TOPDIR= /usr/libdata
#TOPDIR= /usr/contrib/lib
#
LIBDIR= ${TOPDIR}/calc
HELPDIR= ${LIBDIR}/help
CUSTOMLIBDIR= ${LIBDIR}/custom
CUSTOMHELPDIR= ${HELPDIR}/custhelp

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
#	disable     NO_SHARED=
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
# a *.a library.  Set RANLIB to the utility that performs this action.
# Set RANLIB to : if your system does not need such a utility.
#
#RANLIB=ranlib
RANLIB=:

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# Normally certain files depend on the Makefile.  If the Makefile is
# changed, then certain steps should be redone.  If MAKE_FILE is
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
# may invoke non-standard or non-portable code.  One may completely
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
# LCFLAGS are CC-style flags for ${LINT}
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
LCFLAGS=
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

# These .h files are installed under ${CUSTOMLIBDIR} by the install rule.
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

# complete list of targets
#
TARGETS= libcustcalc.a ${CUSTCALC_OBJ}

# required vars
#
SHELL = /bin/sh
SED= sed
MAKEDEPEND= makedepend
SORT= sort

##
#
# Standard rules and targets
#
##

all: ${TARGETS} ${INSTALL_H_SRC} ${CUSTOM_CALC_FILES} \
	${CUSTOM_HELP} ${MAKE_FILE} .all

libcustcalc.a: ${CUSTCALC_OBJ} ${MAKE_FILE} ../Makefile
	-rm -f libcustcalc.a
	ar qc libcustcalc.a ${CUSTCALC_OBJ}
	${RANLIB} libcustcalc.a

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
# File list generation.  You can ignore this section.
#
#
# We will form the names of source files as if they were in a
# sub-directory called calc/lib.
#
# NOTE: Due to bogus shells found on one common system we must have
#       an non-emoty else clause for every if condition.  *sigh*
#
##

distlist: ${DISTLIST}
	${Q}for i in ${DISTLIST}; do \
		echo calc/custom/$$i; \
	done

# The bsdi distribution has generated files as well as distributed files.
#
bsdilist: ${DISTLIST}
	${Q}for i in ${DISTLIST}; do \
		echo calc/custom/$$i; \
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
	-${Q}for i in ${C_SRC}; do \
		${SED} -n '/^#[	 ]*include[ 	]*"/p' \
		    "$$i" > "skel/custom/$$i"; \
	done
	-${Q}for i in /dev/null ${H_SRC}; do \
		if [ "$$i" = "/dev/null" ]; then \
		    continue; \
		fi; \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#ifndef $$tag" > "skel/custom/$$i"; \
		echo "#define $$tag" >> "skel/custom/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" \
		    >> "skel/custom/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/custom/$$i"; \
	done
	${Q}(cd ..; ${MAKE} hsrc)
	${Q}for i in `cd ..; ${MAKE} h_list`; do \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#ifndef $$tag" > "skel/$$i"; \
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
	${Q}cd skel/custom; ${MAKEDEPEND} -w 1 -m -f makedep.out ${C_SRC}
	-${Q}for i in ${C_SRC}; do \
		echo "$$i" | \
		    ${SED} 's/^\(.*\)\.c/\1.o: \1.c/' \
		    >> skel/custom/makedep.out; \
	done
	${Q}echo custom dependency list formed
	${Q}echo forming new custom/Makefile
	-${Q}rm -f Makefile.bak
	${Q}mv Makefile Makefile.bak
	${Q}${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' Makefile.bak > Makefile
	${Q}echo "" >> Makefile
	${Q}${SED} -n '3,$$p' skel/custom/makedep.out | ${SORT} -u >> Makefile
	-${Q}rm -rf skel
	-${Q}if cmp -s Makefile.bak Makefile; then \
		echo 'custom Makefile was already up to date'; \
		mv -f Makefile.bak Makefile; \
	else \
		rm -f Makefile.tmp; \
		mv Makefile Makefile.tmp; \
		sccs edit Makefile; \
		mv Makefile.tmp Makefile; \
		echo new 'custom Makefile formed -- you need to check it in'; \
	fi

##
#
# Utility rules
#
##

clean:
	-rm -f ${CUSTCALC_OBJ}

clobber:
	-rm -f ${TARGETS}
	rm -f .all Makefile.tmp

install: all
	-${Q}if [ ! -d ${TOPDIR} ]; then \
		echo mkdir ${TOPDIR}; \
		mkdir ${TOPDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d ${LIBDIR} ]; then \
		echo mkdir ${LIBDIR}; \
		mkdir ${LIBDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d ${HELPDIR} ]; then \
		echo mkdir ${HELPDIR}; \
		mkdir ${HELPDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d ${CUSTOMLIBDIR} ]; then \
		echo mkdir ${CUSTOMLIBDIR}; \
		mkdir ${CUSTOMLIBDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d ${CUSTOMHELPDIR} ]; then \
		echo mkdir ${CUSTOMHELPDIR}; \
		mkdir ${CUSTOMHELPDIR}; \
	else \
	    true; \
	fi
	${Q}for i in ${INSTALL_H_SRC} /dev/null; do \
		if [ X$$i = X/dev/null ]; then continue; fi; \
		echo rm -f ${CUSTOMLIBDIR}/$$i; \
		rm -f ${CUSTOMLIBDIR}/$$i; \
		echo cp $$i ${CUSTOMLIBDIR}; \
		cp $$i ${CUSTOMLIBDIR}; \
		echo chmod 0444 ${CUSTOMLIBDIR}/$$i; \
		chmod 0444 ${CUSTOMLIBDIR}/$$i; \
	done
	${Q}for i in ${CUSTOM_CALC_FILES}; do \
		echo rm -f ${CUSTOMLIBDIR}/$$i; \
		rm -f ${CUSTOMLIBDIR}/$$i; \
		echo cp $$i ${CUSTOMLIBDIR}; \
		cp $$i ${CUSTOMLIBDIR}; \
		echo chmod 0444 ${CUSTOMLIBDIR}/$$i; \
		chmod 0444 ${CUSTOMLIBDIR}/$$i; \
	done
	${Q}for i in ${CUSTOM_HELP}; do \
		echo rm -f ${CUSTOMHELPDIR}/$$i; \
		rm -f ${CUSTOMHELPDIR}/$$i; \
		echo cp $$i ${CUSTOMHELPDIR}; \
		cp $$i ${CUSTOMHELPDIR}; \
		echo chmod 0444 ${CUSTOMHELPDIR}/$$i; \
		chmod 0444 ${CUSTOMHELPDIR}/$$i; \
	done
	-${Q}if [ ! -z ${ALLOW_CUSTOM} ]; then \
		echo "rm -f ${CUSTOMLIBDIR}/libcustcalc.a"; \
		rm -f ${CUSTOMLIBDIR}/libcustcalc.a; \
		echo "cp libcustcalc.a ${CUSTOMLIBDIR}/libcustcalc.a"; \
		cp libcustcalc.a ${CUSTOMLIBDIR}/libcustcalc.a; \
		echo "chmod 0644 ${CUSTOMLIBDIR}/libcustcalc.a"; \
		chmod 0644 ${CUSTOMLIBDIR}/libcustcalc.a; \
		echo "${RANLIB} ${CUSTOMLIBDIR}/libcustcalc.a"; \
		${RANLIB} ${CUSTOMLIBDIR}/libcustcalc.a; \
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
c_argv.o: ../longbits.h
c_argv.o: ../md5.h
c_argv.o: ../nametype.h
c_argv.o: ../qmath.h
c_argv.o: ../shs.h
c_argv.o: ../shs1.h
c_argv.o: ../string.h
c_argv.o: ../value.h
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
c_devnull.o: ../longbits.h
c_devnull.o: ../md5.h
c_devnull.o: ../nametype.h
c_devnull.o: ../qmath.h
c_devnull.o: ../shs.h
c_devnull.o: ../shs1.h
c_devnull.o: ../string.h
c_devnull.o: ../value.h
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
c_help.o: ../longbits.h
c_help.o: ../md5.h
c_help.o: ../nametype.h
c_help.o: ../qmath.h
c_help.o: ../shs.h
c_help.o: ../shs1.h
c_help.o: ../string.h
c_help.o: ../value.h
c_help.o: ../zmath.h
c_help.o: c_help.c
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
c_sysinfo.o: ../hist.h
c_sysinfo.o: ../longbits.h
c_sysinfo.o: ../longlong.h
c_sysinfo.o: ../md5.h
c_sysinfo.o: ../nametype.h
c_sysinfo.o: ../prime.h
c_sysinfo.o: ../qmath.h
c_sysinfo.o: ../shs.h
c_sysinfo.o: ../shs1.h
c_sysinfo.o: ../string.h
c_sysinfo.o: ../value.h
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
custtbl.o: ../zmath.h
custtbl.o: custtbl.c
