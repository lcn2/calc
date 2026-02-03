#
# calc - arbitrary precision calculator
#
# Copyright (C) 1999-2018,2021-2026  Landon Curt Noll
#
# Suggestion: Read the HOWTO.INSTALL file.
#
# Calc is open software; you can redistribute it and/or modify it under
# the terms of version 2.1 of the GNU Lesser General Public License
# as published by the Free Software Foundation.
#
# Calc is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
# Public License for more details.
#
# A copy of version 2.1 of the GNU Lesser General Public License is
# distributed with calc under the filename COPYING-LGPL.  You should have
# received a copy with calc; if not, write to Free Software Foundation, Inc.
# 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
#
# Under source code control:	1990/02/15 01:48:41
# File existed as early as:	before 1990
#
# This calculator first developed by David I. Bell with help/mods from others.
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-)		http://www.isthe.com/chongo/tech/comp/calc/


# SUGGESTION: Instead of modifying this file, consider adding
#	      statements to modify, replace or append Makefile
#	      variables in the Makefile.local file.


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
CONFIG_MKF= Makefile.config

# Host target information.
#
TARGET_MKF= Makefile.target

# Local file that is included just prior to the first rule,
# that allows one to override any values set in this Makefile.
#
LOCAL_MKF= Makefile.local

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
# H=@	echo hsrc file formation progress
#
# S= >/dev/null 2>&1	silence ${CC} output during hsrc file formation
# S=			full ${CC} output during hsrc file formation
#
# E= 2>/dev/null	silence command stderr during hsrc file formation
# E=			full command stderr during hsrc file formation
#
# V=@:	do not echo debug statements (quiet mode)
# V=@	echo debug statements (debug / verbose mode)
#
# To turn all messages, use:
#
# Q=
# H=@
# S=
# E=
# V=@
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

# the source files which are built into a math link library
#
# There MUST be a .o for every .c in LIBOBJS
#
LIBSRC= addop.c assocfunc.c blkcpy.c block.c byteswap.c \
	codegen.c comfunc.c commath.c config.c const.c custom.c \
	errtbl.c file.c func.c hash.c help.c hist.c input.c jump.c label.c \
	lib_calc.c lib_util.c listfunc.c matfunc.c math_error.c \
	obj.c opcodes.c pix.c poly.c prime.c qfunc.c qio.c \
	qmath.c qmod.c qtrans.c quickhash.c seed.c sha1.c size.c \
	str.c strl.c symbol.c token.c value.c version.c zfunc.c zio.c \
	zmath.c zmod.c zmul.c zprime.c zrand.c zrandom.c

# the object files which are built into a math link library
#
# There MUST be a .o for every .c in LIBSRC
# which is built via this Makefile.
#
LIBOBJS= addop.o assocfunc.o blkcpy.o block.o byteswap.o \
	codegen.o comfunc.o commath.o config.o const.o custom.o \
	errtbl.o file.o func.o hash.o help.o hist.o input.o jump.o label.o \
	lib_calc.o lib_util.o listfunc.o matfunc.o math_error.o \
	obj.o opcodes.o pix.o poly.o prime.o qfunc.o qio.o \
	qmath.o qmod.o qtrans.o quickhash.o seed.o sha1.o size.o \
	str.o strl.o symbol.o token.o value.o version.o zfunc.o zio.o \
	zmath.o zmod.o zmul.o zprime.o zrand.o zrandom.o

# the calculator source files
#
# There MUST be a .c for every .o in CALCOBJS.
#
CALCSRC= calc.c

# we build these .o files for calc
#
# There MUST be a .o for every .c in CALCSRC.
#
CALCOBJS= calc.o

# these .h files are needed to build the math link library
#
LIB_H_SRC= attribute.h banned.h blkcpy.h block.h byteswap.h \
	calc.h cmath.h config.h custom.h errtbl.h file.h func.h \
	hash.h hist.h int.h jump.h label.h lib_calc.h lib_util.h nametype.h \
	opcodes.h prime.h qmath.h sha1.h str.h strl.h symbol.h token.h \
	value.h version.h zmath.h zrand.h zrandom.h

# we build these .h files during the make
#
BUILD_H_SRC= conf.h endian_calc.h errsym.h \
	have_arc4random.h have_ban_pragma.h have_environ.h \
	have_getpgid.h have_getsid.h have_rusage.h \
	have_strlcat.h have_strlcpy.h \
	have_unused.h have_urandom.h len_bits.h status.chk_c.h

# we build these .c files during the make
#
BUILD_C_SRC=

# these .c files may be used in the process of building BUILD_H_SRC
#
# There MUST be a .c for every .o in UTIL_OBJS.
#
UTIL_C_SRC= chk_c.c endian.c have_arc4random.c \
	have_ban_pragma.c have_environ.c \
	have_getpgid.c have_getsid.c \
	have_rusage.c have_strlcat.c \
	have_strlcpy.c have_unused.c len_bits.c \
	ver_calc.c

# these awk and sed tools are used in the process of building BUILD_H_SRC
# and BUILD_C_SRC
#
UTIL_MISC_SRC=

# these .o files may get built in the process of building BUILD_H_SRC
#
# There MUST be a .o for every .c in UTIL_C_SRC.
#
UTIL_C_SRC= chk_c.o endian.o have_arc4random.o \
	have_ban_pragma.o have_environ.o \
	have_getpgid.o have_getsid.o \
	have_rusage.o have_strlcat.o \
	have_strlcpy.o have_unused.o len_bits.o \
	ver_calc.o

# these temp files may be created (and removed) during the build of BUILD_C_SRC
#
UTIL_TMP= ll_tmp const_tmp uid_tmp newstr_tmp vs_tmp \
	newstr_tmp getsid_tmp gettime_tmp getprid_tmp rusage_tmp strdup_tmp

# these utility executables may be created in the process of
# building the BUILD_H_SRC file set
#
# There MUST be a ${EXT} for every .c in UTIL_C_SRC.
#
UTIL_PROGS= chk_c${EXT} endian${EXT} have_arc4random${EXT} \
	have_ban_pragma${EXT} have_environ{EXT} \
	have_getpgid${EXT} have_environ{EXT} \
	have_getpgid${EXT} have_getsid${EXT} \
	have_rusage${EXT} have_strlcat${EXT} \
	have_strlcpy${EXT} have_unused${EXT} len_bits{ENT} \
	ver_calc${EXT}

# these utility files and scripts may be created in the process of building
# the BUILD_H_SRC file set
#
UTIL_FILES=

# Any .h files that are needed to compile sample code.
#
SAMPLE_H_SRC=

# Any .c files that are needed to compile sample code.
#
# There MUST be a .c in SAMPLE_C_SRC for every .o in SAMPLE_OBJ.
#
SAMPLE_C_SRC= sample_many.c sample_rand.c

# Any .o files that are needed to compile sample code.
#
# There MUST be a .c in SAMPLE_C_SRC for every .o in SAMPLE_OBJ.
#
SAMPLE_OBJ= sample_many.o sample_rand.o

# complete list of .h files found (but not built) in the distribution
#
H_SRC= ${LIB_H_SRC} ${SAMPLE_H_SRC}

# complete list of .c files found (but not built) in the distribution
#
C_SRC= ${LIBSRC} ${CALCSRC} ${UTIL_C_SRC} ${SAMPLE_C_SRC}

# The list of files that describe calc's GNU Lesser General Public License
#
LICENSE= COPYING COPYING-LGPL

# These files are found (but not built) in the distribution
#
DISTLIST= ${C_SRC} ${H_SRC} ${MK_SET} ${UTIL_MISC_SRC} ${LICENSE} \
	BUGS calc.man calc.spec.in CHANGES check.awk chk_tree CONTRIB-CODE \
	HOWTO.INSTALL LIBRARY .lldbinit QUESTIONS README.FIRST README.md \
	README.RELEASE README.WINDOWS rpm.mk sample.README trailblank update_ver \
	.clang-format

# These files are used to make (but not build) a calc .a link library
#
CALCLIBLIST= ${LIBSRC} ${UTIL_C_SRC} ${LIB_H_SRC} ${MK_SET} \
	     ${UTIL_MISC_SRC} BUGS CHANGES LIBRARY

# complete list of .o files
#
OBJS= ${LIBOBJS} ${CALCOBJS} ${UTIL_OBJS} ${SAMPLE_OBJ}

# Static library build
# Libraries created and used to build calc
# Symlinks of dynamic shared libraries
# Early targets - things needed before the main build phase can begin
#
CALC_STATIC_LIBS= libcalc.a libcustcalc.a
CALC_DYNAMIC_LIBS= libcalc${LIB_EXT_VERSION} libcustcalc${LIB_EXT_VERSION}
SYM_DYNAMIC_LIBS= libcalc${LIB_EXT} libcustcalc${LIB_EXT} \
	libcalc${LIB_EXT_VER} libcustcalc${LIB_EXT_VER}
EARLY_TARGETS= errsym.h hsrc .hsrc custom/.all custom/Makefile

# list of sample programs that need to be built to satisfy sample rule
#
# NOTE: The ${SAMPLE_TARGETS} and ${SAMPLE_STATIC_TARGETS} are built but
#	not installed at this time.
#
# NOTE: There must be a foo-static${EXT} in SAMPLE_STATIC_TARGETS for
#	every foo${EXT} in ${SAMPLE_TARGETS}.
#
SAMPLE_TARGETS= sample_rand${EXT} sample_many${EXT}
SAMPLE_STATIC_TARGETS= sample_rand-static${EXT} sample_many-static${EXT}

# list of cscript programs that need to be built to satisfy cscript/.all
#
# NOTE: This list MUST be coordinated with the ${CSCRIPT_TARGETS} variable
#	in the cscript/Makefile
#
CSCRIPT_TARGETS= cscript/mersenne cscript/piforever cscript/plus \
		 cscript/square cscript/fproduct cscript/powerterm

# dynamic first targets
#
DYNAMIC_FIRST_TARGETS= ${LICENSE} .dynamic

# static first targets
#
STATIC_FIRST_TARGETS= ${LICENSE} .static

# late targets - things needed after the main build phase is complete
#
LATE_TARGETS= calc.1 calc.usage calc.cat1 \
	      cal/.all help/.all help/builtin help/errorcodes \
	      cscript/.all

# calc tools - tools used by the maintainers of the calc source base
#
# Calc supplied tools:
#
# TRAILBLANK	- find trailing blanks and other file format picky issues
# UPDATE_VER	- update version numbers in Makefile.config
# CALC_TOOLS	- the complete list of calc supplied tools
#
TRAILBLANK= trailblank
UPDATE_VER= update_ver
CALC_TOOLS= ${TRAILBLANK} ${UPDATE_VER}

# complete list of files that may be created as part of the build process
#
# Used by chk_tree via make prep
#
BUILD_ALL= ${LIBOBJS} ${CALCOBJS} ${BUILD_H_SRC} ${BUILD_C_SRC} \
	   ${UTIL_OBJS} ${UTIL_TMP} ${UTIL_PROGS} ${SAMPLE_OBJ} \
	   ${CALC_STATIC_LIBS} ${CALC_DYNAMIC_LIBS} ${SYM_DYNAMIC_LIBS} \
	   ${SAMPLE_TARGETS} ${SAMPLE_STATIC_TARGETS} ${CSCRIPT_TARGETS} \
	   .dynamic .static ${LATE_TARGETS} calc${EXT} errcode${EXT} \
	   tags .hsrc outfile

# complete list of targets
#
TARGETS= ${EARLY_TARGETS} ${BLD_TYPE} ${LATE_TARGETS} ${CALC_TOOLS}

# rules that are not also names of files
#
PHONY= all check_include sample hsrc depend h_list calc_version version distlist \
	buildlist distdir calcliblist calcliblistfmt verifydist check chk calcinfo \
	env mkdebug full_debug debug testfuncsort prep run rpm inst_files \
	olduninstall clean clobber install uninstall unbak splint strip

############################################################
# Allow Makefile.local to change any of the above settings #
############################################################
include ${LOCAL_MKF}


###########################################
# all - First and default Makefile target #
###########################################

all: check_include ${BLD_TYPE} CHANGES


###############################
# additional Makefile targets #
###############################

.PHONY: ${PHONY}

check_include:
	${Q} if ! echo '#include <stdio.h>' | ${CC} -E - >/dev/null 2>&1; then \
	    echo "ERROR: Missing critical <stdio.h> include file." 1>&2; \
	    echo "Without critical include files, we cannot compile." 1>&2; \
	    echo "Perhaps your system isn't setup to compile C source?" 1>&2; \
	    echo 1>&2; \
	    echo "For example, Apple macOS / Darwin requires that XCode" 1>&2; \
	    echo "must be installed." 1>&2; \
	    echo 1>&2; \
	    echo "Also macOS users might later to run this command:" 1>&2; \
	    echo 1>&2; \
	    echo "    xcode-select --install" 1>&2; \
	    echo 1>&2; \
	    exit 1; \
	fi

calc-dynamic-only: ${DYNAMIC_FIRST_TARGETS} ${EARLY_TARGETS} \
		   ${CALC_DYNAMIC_LIBS} ${SYM_DYNAMIC_LIBS} calc${EXT} \
		   ${SAMPLE_TARGETS} ${LATE_TARGETS}

.dynamic: ${MK_SET}
	${Q} d="calc-dynamic-only"; \
	    s="calc-static-only"; \
	    if [ "${BLD_TYPE}" != "$$d" ]; then \
	    echo "NOTE: The host target $(target) defaults to a build" 1>&2; \
	    echo "      type of: ${BLD_TYPE}, so you need to use" 1>&2; \
	    echo "      the following make command:" 1>&2; \
	    echo "" 1>&2; \
	    echo "      ${MAKE} -f ${MAKE_FILE} clobber" 1>&2; \
	    echo "      ${MAKE} -f ${MAKE_FILE} $$d BLD_TYPE=$$d" 1>&2; \
	    echo "" 1>&2; \
	    echo "NOTE: It is a very good idea to first clobber any" 1>&2; \
	    echo "      previously built .o, libs and executables" 1>&2; \
	    echo "      before switching to $$d ." 1>&2; \
	    echo "" 1>&2; \
	    echo "=== aborting make ===" 1>&2; \
	    exit 1; \
	fi
	${Q} for i in .static calc-static${EXT} ${SAMPLE_STATIC_TARGETS} \
		      libcalc.a custom/libcustcalc.a; do \
	    d="calc-dynamic-only"; \
	    s="calc-static-only"; \
	    if [ -e "$$i" ]; then \
		echo "Found the static target $$i file." 1>&2; \
		echo "" 1>&2; \
		echo "If you are trying to install the static calc, you must install using:" 1>&2; \
		echo "" 1>&2; \
		echo "      ${MAKE} -f ${MAKE_FILE} $$s install BLD_TYPE=$$s" 1>&2; \
		echo "" 1>&2; \
		echo "Otherwise you need to first clean out previously built static files" 1>&2; \
		echo "before trying to build and/or install dynamic calc:" 1>&2; \
		echo "" 1>&2; \
		echo "      ${MAKE} -f ${MAKE_FILE} clobber" 1>&2; \
		echo "" 1>&2; \
		echo "=== aborting make ===" 1>&2; \
		exit 2; \
	    fi; \
	done
	 -${Q} ${TOUCH} $@

calc-static-only: ${STATIC_FIRST_TARGETS} ${EARLY_TARGETS} \
		  ${CALC_STATIC_LIBS} calc-static${EXT} \
		  ${SAMPLE_STATIC_TARGETS} ${LATE_TARGETS}
	${Q} for i in calc${EXT} ${SAMPLE_TARGETS}; do \
	    if ${CMP} -s "$$i-static" "$$i"; then \
		${TRUE}; \
	    else \
		${RM} -f "$$i"; \
		${LN} "$$i-static" "$$i"; \
	    fi; \
	done

.static: ${MK_SET}
	${Q} d="calc-static-only"; \
	    s="calc-static-only"; \
	    if [ "${BLD_TYPE}" != "$$s" ]; then \
	    echo "NOTE: The host target $(target) defaults to a build" 1>&2; \
	    echo "      type of: ${BLD_TYPE}, so you need to use" 1>&2; \
	    echo "      the following make command:" 1>&2; \
	    echo "" 1>&2; \
	    echo "      ${MAKE} -f ${MAKE_FILE} clobber" 1>&2; \
	    echo "      ${MAKE} -f ${MAKE_FILE} $$s BLD_TYPE=$$s" 1>&2; \
	    echo "" 1>&2; \
	    echo "NOTE: It is a very good idea to first clobber any" 1>&2; \
	    echo "      previously built .o, libs and executables" 1>&2; \
	    echo "      before switching to $$s ." 1>&2; \
	    echo "" 1>&2; \
	    echo "=== aborting make ===" 1>&2; \
	    exit 3; \
	fi
	${Q} for i in .dynamic ${CALC_DYNAMIC_LIBS} ${SYM_DYNAMIC_LIBS} \
		      custom/libcustcalc${LIB_EXT_VERSION}; do \
	    d="calc-dynamic-only"; \
	    s="calc-static-only"; \
	    if [ -r "$$i" ]; then \
		echo "Found the dynamic target $$i file.  You must:" 1>&2; \
		echo "" 1>&2; \
		echo "If you are trying to install the dynamic calc, you must install using:" 1>&2; \
		echo "" 1>&2; \
		echo "      ${MAKE} -f ${MAKE_FILE} $$d install BLD_TYPE=$$d" 1>&2; \
		echo "" 1>&2; \
		echo "Otherwise you need to first clean out previously built dynamic files" 1>&2; \
		echo "before trying to build and/or install static calc:" 1>&2; \
		echo "" 1>&2; \
		echo "      ${MAKE} -f ${MAKE_FILE} clobber" 1>&2; \
		echo "" 1>&2; \
		echo "=== aborting make ===" 1>&2; \
		exit 4; \
	    fi; \
	done
	 -${Q} ${TOUCH} $@

calc${EXT}: .hsrc ${CALCOBJS} ${CALC_DYNAMIC_LIBS} ${MK_SET}
	${RM} -f $@
	${CC} ${CALCOBJS} ${LDFLAGS} ${LD_SHARE} ${CALC_DYNAMIC_LIBS} \
	      ${READLINE_LIB} ${READLINE_EXTRAS} -o $@

libcalc${LIB_EXT_VERSION}: ${LIBOBJS} ver_calc${EXT} ${MK_SET} libcustcalc${LIB_EXT_VERSION}
	${CC} ${LIBCALC_SHLIB} ${LIBOBJS} libcustcalc${LIB_EXT_VERSION} \
	      ${READLINE_LIB} ${READLINE_EXTRAS} -o libcalc${LIB_EXT_VERSION}

libcalc${LIB_EXT}: libcalc${LIB_EXT_VERSION}
	${Q} ${RM} -f $@
	${LN} -s $? $@

libcalc${LIB_EXT_VER}: libcalc${LIB_EXT_VERSION}
	${Q} ${RM} -f $@
	${LN} -s $? $@

###
#
# calc documentation
#
###

# Note: The :\(...-sed pattern below allows word wrapping at the separators
# of long path names (typically CALCPATH and CALCRC).
calc.1: calc.man ${MK_SET}
	${RM} -f $@
	${Q} echo forming calc.1 from calc.man
	@${SED} -e 's:$${LIBDIR}:${LIBDIR}:g' \
	        -e 's,$${BINDIR},${BINDIR},g' \
	        -e 's,$${VERSION},${VERSION},g' \
	        -e 's,$${CALCPATH},${CALCPATH},g' \
	        -e 's,$${SCRIPTDIR},${SCRIPTDIR},g' \
	        -e 's,$${CALC_INCDIR},${CALC_INCDIR},g' \
	        -e 's,$${CUSTOMCALDIR},${CUSTOMCALDIR},g' \
	        -e 's,$${CUSTOMINCDIR},${CUSTOMINCDIR},g' \
	        -e 's,$${HELPDIR},${HELPDIR},g' \
	        -e 's,$${CUSTOMHELPDIR},${CUSTOMHELPDIR},g' \
	        -e 's,$${CALCRC},${CALCRC},g' \
	        -e 's,:\([/.~]\),:\\:\1,g' < calc.man > calc.1
	${Q} echo calc.1 formed

calc.usage: calc.1
	${Q} echo forming $@ from calc.1
	${RM} -f $@
	${MAN} ./calc.1 > $@
	${CHMOD} 0444 $@
	${Q} echo $@ formed

calc.cat1: calc.usage
	${Q} echo forming $@ from calc.usage
	${RM} -f $@
	${GZIP} --best -f -c calc.usage > $@
	${CHMOD} 0444 $@
	${Q} echo $@ formed


##
#
# These rules compile the sample code against the calc library
#
##

sample: ${SAMPLE_TARGETS}

sample_rand${EXT}: sample_rand.o ${CALC_DYNAMIC_LIBS} ${MK_SET}
	${CC} sample_rand.o ${LDFLAGS} ${LD_SHARE} ${CALC_DYNAMIC_LIBS} \
	      ${READLINE_LIB} ${READLINE_EXTRAS} -o $@

sample_many${EXT}: sample_many.o ${CALC_DYNAMIC_LIBS} ${MK_SET}
	${CC} sample_many.o ${LDFLAGS} ${LD_SHARE} ${CALC_DYNAMIC_LIBS} \
	      ${READLINE_LIB} ${READLINE_EXTRAS} -o $@

###
#
# Special .o files that may need special compile options
#
###

# NOTE: In the case of multiple -O options, the last such option is the one that is effective.
byteswap.o: byteswap.c ${MK_SET}
	${CC} ${CFLAGS} -O1 -c byteswap.c

file.o: file.c ${MK_SET}
	${CC} ${CFLAGS} ${WNO_ERROR_LONG_LONG} ${WNO_LONG_LONG} -c file.c

hist.o: hist.c ${MK_SET}
	${CC} ${CFLAGS} -Wno-strict-prototypes ${USE_READLINE} ${READLINE_INCLUDE} -c hist.c

seed.o: seed.c ${MK_SET}
	${CC} ${CFLAGS} ${WNO_IMPLICT} ${WNO_ERROR_LONG_LONG} ${WNO_LONG_LONG} -c seed.c

###
#
# The next set of rules cause the .h files BUILD_H_SRC files to be built
# according to the system and the Makefile variables above.  The hsrc rule
# is a convenient rule to invoke to build all of the BUILD_H_SRC.
#
# We add in the BUILD_C_SRC files because they are similar to the
# BUILD_H_SRC files in terms of the build process.
#
# NOTE: Due to bogus shells found on one common system we must have
#	an non-empty else clause for every if condition.  *sigh*
#	We also place ; ${TRUE} at the end of some commands to avoid
#	meaningless cosmetic messages by the same system.
#
###

hsrc: chk_c${EXT} ${BUILD_H_SRC} ${BUILD_C_SRC}

.hsrc: ${BUILD_H_SRC} ${BUILD_C_SRC}
	${Q} ${RM} -f .hsrc
	-${Q} ${TOUCH} .hsrc

conf.h: ${MK_SET}
	${Q} ${RM} -f $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_CONF_H)' >> $@
	${Q} echo '#define CALC_CONF_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* the default :-separated search path */' >> $@
	${Q} echo '#if !defined(DEFAULTCALCPATH)' >> $@
	${Q} echo '#define DEFAULTCALCPATH "${CALCPATH}"' >> $@
	${Q} echo '#endif /* DEFAULTCALCPATH */' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* the default :-separated startup file list */' >> $@
	${Q} echo '#if !defined(DEFAULTCALCRC)' >> $@
	${Q} echo '#define DEFAULTCALCRC "${CALCRC}"' >> $@
	${Q} echo '#endif /* DEFAULTCALCRC */' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* the location of the help directory */' >> $@
	${Q} echo '#if !defined(HELPDIR)' >> $@
ifdef RPM_TOP
	${Q} echo '#define HELPDIR "${HELPDIR}"' >> $@
else	# RPM_TOP
	${Q} echo '#define HELPDIR "${T}${HELPDIR}"' >> $@
endif	# RPM_TOP
	${Q} echo '#endif /* HELPDIR */' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* the location of the custom help directory */' >> $@
	${Q} echo '#if !defined(CUSTOMHELPDIR)' >> $@
ifdef RPM_TOP
	${Q} echo '#define CUSTOMHELPDIR "${CUSTOMHELPDIR}"' >> $@
else	# RPM_TOP
	${Q} echo '#define CUSTOMHELPDIR "${T}${CUSTOMHELPDIR}"' >> $@
endif	# RPM_TOP
	${Q} echo '#endif /* CUSTOMHELPDIR */' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* the default pager to use */' >> $@
	${Q} echo '#if !defined(DEFAULTCALCPAGER)' >> $@
	${Q} echo '#define DEFAULTCALCPAGER "${CALCPAGER}"' >> $@
	${Q} echo '#endif /* DEFAULTCALCPAGER */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_CONF_H */' >> $@
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

# If you get syntax errors when you compile `endian.c`, then
# try this to see what might be happening, and fix accordingly:
#
# 	rm -f endian_calc.h && make endian_calc.h CALC_BYTE_ORDER=-DCALC_ENDIAN_DEBUG Q= S=
#
# Or try:
#
#     try using the -DCALC_BYTE_ORDER=CALC_LITTLE_ENDIAN line, or
#     try using the -DCALC_BYTE_ORDER=CALC_BIG_ENDIAN line
#
# Then try running the calc regression tests:
#
#     make clobber all chk
#
# See README.FIRST file for more help on running the calc regression tests.
#
endian_calc.h: endian.c \
	banned.h have_ban_pragma.h ${MK_SET}
	${Q} ${RM} -f endian.o endian${EXT} $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(ENDIAN_CALC_H)' >> $@
	${Q} echo '#define ENDIAN_CALC_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${LCC} ${ICFLAGS} ${CALC_BYTE_ORDER} endian.c -c ${S}
	${LCC} ${ILDFLAGS} endian.o -o endian${EXT} ${S}
	./endian${EXT} >> $@
	${RM} -f endian.o endian${EXT}
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !ENDIAN_CALC_H */' >> $@
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

len_bits.h: len_bits.c endian_calc.h \
	banned.h have_ban_pragma.h ${MK_SET}
	${Q} ${RM} -f len_bits.o len_bits${EXT} $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_LEN_BITS_H)' >> $@
	${Q} echo '#define CALC_LEN_BITS_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} ${LCC} ${ICFLAGS} len_bits.c -c ${S}
	${Q} ${LCC} ${ILDFLAGS} len_bits.o -o len_bits${EXT} ${S}
	${Q} ./len_bits${EXT} ${LONG_BITS} >> $@ ${E}
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_LEN_BITS_H */' >> $@
	${H} echo '$@ formed'
	${Q} ${RM} -f len_bits.o len_bits${EXT}
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_environ.h: have_environ.c \
	banned.h have_ban_pragma.h ${MK_SET}
	${Q} ${RM} -f have_environ environ_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_ENVIRON_H)' >> $@
	${Q} echo '#define CALC_HAVE_ENVIRON_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have or want environ? */' >> $@
	${Q} ${RM} -f have_environ.o have_environ${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_ENVIRON} have_environ.c -c ${S} \
		|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_environ.o -o have_environ${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_environ${EXT} > environ_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s environ_tmp ]; then \
	    ${CAT} environ_tmp >> $@; \
	else \
	    echo '#undef HAVE_ENVIRON /* no */' >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_ENVIRON_H */' >> $@
	${Q} ${RM} -f have_environ${EXT} have_environ.o environ_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_arc4random.h: have_arc4random.c \
	banned.h have_ban_pragma.h ${MK_SET}
	${Q} ${RM} -f have_arc4random arc4random_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(HAVE_ARC4RANDOM)' >> $@
	${Q} echo '#define HAVE_ARC4RANDOM' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have or want arc4random? */' >> $@
	${Q} ${RM} -f have_arc4random.o have_arc4random${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_ARC4RANDOM} have_arc4random.c -c ${S} \
		|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_arc4random.o \
		-o have_arc4random${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_arc4random${EXT} > arc4random_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s arc4random_tmp ]; then \
	    ${CAT} arc4random_tmp >> $@; \
	else \
	    echo '#undef HAVE_ARC4RANDOM /* no */' >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !HAVE_ARC4RANDOM */' >> $@
	${Q} ${RM} -f have_arc4random${EXT} have_arc4random.o arc4random_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_getsid.h: have_getsid.c \
	banned.h have_ban_pragma.h ${MK_SET}
	${Q} ${RM} -f getsid_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_GETSID_H)' >> $@
	${Q} echo '#define CALC_HAVE_GETSID_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have or want getsid()? */' >> $@
	${Q} ${RM} -f have_getsid.o have_getsid${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_GETSID} have_getsid.c -c ${S} \
		|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_getsid.o -o have_getsid${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_getsid${EXT} > getsid_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s getsid_tmp ]; then \
	    ${CAT} getsid_tmp >> $@; \
	else \
	    echo '#undef HAVE_GETSID /* no */' >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_GETSID_H */' >> $@
	${Q} ${RM} -f have_getsid${EXT} have_getsid.o getsid_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_getpgid.h: have_getpgid.c \
	banned.h have_ban_pragma.h ${MK_SET}
	${Q} ${RM} -f getpgid_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_GETPGID_H)' >> $@
	${Q} echo '#define CALC_HAVE_GETPGID_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have or want getpgid()? */' >> $@
	${Q} ${RM} -f have_getpgid.o have_getpgid${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_GETPGID} have_getpgid.c -c ${S} \
		|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_getpgid.o -o have_getpgid${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_getpgid${EXT} > getpgid_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s getpgid_tmp ]; then \
	    ${CAT} getpgid_tmp >> $@; \
	else \
	    echo '#undef HAVE_GETPGID /* no */' >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_GETPGID_H */' >> $@
	${Q} ${RM} -f have_getpgid${EXT} have_getpgid.o getpgid_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_urandom.h: ${MK_SET}
	${Q} ${RM} -f $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_URANDOM_H)' >> $@
	${Q} echo '#define CALC_HAVE_URANDOM_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have /dev/urandom? */' >> $@
	-${Q} if [ X"${HAVE_URANDOM}" = X"YES" ]; then \
	    echo '#define HAVE_URANDOM	/* yes */' >> $@; \
	elif [ X"${HAVE_URANDOM}" = X"NO" ]; then \
	    echo '#undef HAVE_URANDOM  /* no */' >> $@; \
	elif [ -r /dev/urandom ] 2>/dev/null; then \
	    echo '#define HAVE_URANDOM  /* yes */' >> $@; \
	else \
	    echo '#undef HAVE_URANDOM	 /* no */' >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_URANDOM_H */' >> $@
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_rusage.h: have_rusage.c banned.h have_ban_pragma.h ${MK_SET}
	${Q} ${RM} -f rusage_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_RUSAGE_H)' >> $@
	${Q} echo '#define CALC_HAVE_RUSAGE_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have or want getrusage()? */' >> $@
	${Q} ${RM} -f have_rusage.o have_rusage${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_GETRUSAGE} have_rusage.c -c ${S} \
		|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_rusage.o -o have_rusage${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_rusage${EXT} > rusage_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s rusage_tmp ]; then \
	    ${CAT} rusage_tmp >> $@; \
	else \
	    echo '#undef HAVE_GETRUSAGE /* no */' >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_RUSAGE_H */' >> $@
	${Q} ${RM} -f have_rusage${EXT} have_rusage.o rusage_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

errsym.h: status.chk_c.h errcode${EXT}
	${Q} ${RM} -f $@
	${H} echo 'forming $@'
	./errcode${EXT} -d > $@
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

errcode${EXT}: status.chk_c.h errtbl.c attribute.h errtbl.h \
	       endian_calc.h len_bits.h ${MK_SET}
	${RM} -f $@
	${LCC} ${ICFLAGS} ${ILDFLAGS} -DERRCODE_SRC errtbl.c -o $@

have_unused.h: have_unused.c have_ban_pragma.h \
	       ${MK_SET}
	${Q} ${RM} -f unused_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_UNUSED_H)' >> $@
	${Q} echo '#define CALC_HAVE_UNUSED_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have/want the unused attribute? */' >> $@
	${Q} ${RM} -f have_unused.o have_unused${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_UNUSED} have_unused.c -c ${S} \
		|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_unused.o -o have_unused${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_unused${EXT} > unused_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s unused_tmp ]; then \
	    ${CAT} unused_tmp >> $@; \
	else \
	    echo '#undef HAVE_UNUSED /* no */' >> $@; \
	    echo '#undef UNUSED' >> $@; \
	    echo '#define UNUSED /* no */' >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_UNUSED_H */' >> $@
	${Q} ${RM} -f have_unused${EXT} have_unused.o unused_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_ban_pragma.h: have_ban_pragma.c banned.h ${MK_SET}
	${Q} ${RM} -f unused_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_BAN_PRAGMA_H)' >> $@
	${Q} echo '#define CALC_HAVE_BAN_PRAGMA_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have/want #pragma GCC poison func_name? */' >> $@
	${Q} ${RM} -f have_ban_pragma.o have_ban_pragma${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_PRAGMA_GCC_POSION} \
		have_ban_pragma.c -c ${S} \
			|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_ban_pragma.o -o have_ban_pragma${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_ban_pragma${EXT} > unused_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s unused_tmp ]; then \
	    ${CAT} unused_tmp >> $@; \
	else \
	    echo '#undef HAVE_PRAGMA_GCC_POSION /* no */' \
	         >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_BAN_PRAGMA_H */' >> $@
	${Q} ${RM} -f have_ban_pragma${EXT} have_ban_pragma.o unused_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_strlcpy.h: have_strlcpy.c banned.h have_ban_pragma.h \
		${MK_SET}
	${Q} ${RM} -f unused_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_STRLCPY_H)' >> $@
	${Q} echo '#define CALC_HAVE_STRLCPY_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have/want the strlcpy() function? */' >> $@
	${Q} ${RM} -f have_strlcpy.o have_strlcpy${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_NO_STRLCPY} have_strlcpy.c -c ${S} \
		|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_strlcpy.o -o have_strlcpy${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_strlcpy${EXT} > unused_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s unused_tmp ]; then \
	    ${CAT} unused_tmp >> $@; \
	else \
	    echo '#undef HAVE_STRLCPY /* no */' \
	         >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_STRLCPY_H */' >> $@
	${Q} ${RM} -f have_strlcpy${EXT} have_strlcpy.o unused_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

have_strlcat.h: have_strlcat.c banned.h have_ban_pragma.h \
		${MK_SET}
	${Q} ${RM} -f unused_tmp $@
	${H} echo 'forming $@'
	${Q} echo '/*' > $@
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> $@
	${Q} echo ' */' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#if !defined(CALC_HAVE_STRLCAT_H)' >> $@
	${Q} echo '#define CALC_HAVE_STRLCAT_H' >> $@
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '/* do we have/want the strlcat() function? */' >> $@
	${Q} ${RM} -f have_strlcat.o have_strlcat${EXT}
	-${Q} ${LCC} ${ICFLAGS} ${HAVE_NO_STRLCAT} have_strlcat.c -c ${S} \
		|| ${TRUE}
	-${Q} ${LCC} ${ILDFLAGS} have_strlcat.o -o have_strlcat${EXT} ${S} \
		|| ${TRUE}
	-${Q} ./have_strlcat${EXT} > unused_tmp ${E} \
		|| ${TRUE}
	-${Q} if [ -s unused_tmp ]; then \
	    ${CAT} unused_tmp >> $@; \
	else \
	    echo '#undef HAVE_STRLCAT /* no */' \
	         >> $@; \
	fi
	${Q} echo '' >> $@
	${Q} echo '' >> $@
	${Q} echo '#endif /* !CALC_HAVE_STRLCAT_H */' >> $@
	${Q} ${RM} -f have_strlcat${EXT} have_strlcat.o unused_tmp
	${H} echo '$@ formed'
	-${Q}if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    ${CAT} $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    ${TRUE}; \
	fi

chk_c${EXT}: have_ban_pragma.h chk_c.c
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${Q} ${RM} -f chk_c.o $@ status.chk_c.h
	${H} echo 'forming status.chk_c.h'
	${Q} echo '/*' > status.chk_c.h
	${Q} echo ' * DO NOT EDIT -- generated by the Makefile rule $@' >> status.chk_c.h
	${Q} echo ' */' >> status.chk_c.h
	${Q} echo '' >> status.chk_c.h
	${Q} echo '' >> status.chk_c.h
	${Q} echo '#if !defined(CALC_STATUS_CHK_C_H)' >> status.chk_c.h
	${Q} echo '#define CALC_STATUS_CHK_C_H' >> status.chk_c.h
	${Q} echo '' >> status.chk_c.h
	${Q} echo '' >> status.chk_c.h
	${Q} echo '/* Does the C compiler and select include files appear to meet calc requirements? */' >> status.chk_c.h
	-${Q} ${LCC} ${ICFLAGS} chk_c.c -c ${S} \
                || echo "WARNING!! failed to form the chk_c.o file." 1>&2
	-${Q} ${LCC} ${ILDFLAGS} chk_c.o -o $@ ${S} \
		|| echo "WARNING!! failed to link the $@ executable." 1>&2
	-${Q}if [ ! -x $@ ]; then \
	    echo "WARNING!! Your C compiler and/or C include does not meet calc requirements." 1>&2; \
	    echo "#undef CHK_C /* failed to form chk_c executable */" >> status.chk_c.h; \
	else \
	    echo "formed $@ executable"; \
	    echo 'about to execute: ./$@'; \
	    if ! ./$@; then \
		echo "WARNING!! ./$@ failed to validate C compiler and/or include files for calc requirements." 1>&2; \
		echo "#undef CHK_C /* chk_c failed to validate C compiler and/or include files */" >> status.chk_c.h; \
	    else \
		echo "Good news everyone! :-)" \
		     "The C compiler and select include files appear to meet calc requirements."; \
		echo "#define CHK_C" \
		     "/* C compiler and select include files appear to meet calc requirements */" >> status.chk_c.h; \
	    fi; \
	fi
	${Q} echo '' >> status.chk_c.h
	${Q} echo '' >> status.chk_c.h
	${Q} echo '#endif /* !CALC_STATUS_CHK_C_H */' >> status.chk_c.h
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

status.chk_c.h: chk_c${EXT}

###
#
# These two .all rules are used to determine of the lower level
# directory has had its all rule performed.
#
###

cal/.all: cal/Makefile
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for cal =-=-=-=-='
	${RM} -f $@
	cd cal; ${MAKE} -f Makefile all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

help/.all: help/Makefile
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for help =-=-=-=-='
	${RM} -f $@
	cd help; ${MAKE} -f Makefile all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

help/builtin: help/Makefile \
	      func.c help/builtin.top help/builtin.end help/funclist.sed
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking builtin rule for help =-=-=-=-='
	${RM} -f $@
	cd help; ${MAKE} -f Makefile builtin
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

help/errorcodes: help/Makefile \
	      errcode${EXT}
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking builtin rule for help =-=-=-=-='
	${RM} -f $@
	cd help; ${MAKE} -f Makefile errorcodes
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

cscript/.all: ${CSCRIPT_TARGETS}

${CSCRIPT_TARGETS}: cscript/Makefile
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for cscript =-=-=-=-='
	${RM} -f cscript/.all
	cd cscript; ${MAKE} -f Makefile all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

custom/.all: custom/Makefile
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for custom =-=-=-=-='
	${RM} -f $@
	cd custom; ${MAKE} -f Makefile all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

custom/libcustcalc${LIB_EXT_VERSION}: custom/Makefile
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for custom =-=-=-=-='
	cd custom; ${MAKE} -f Makefile $@
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

libcustcalc${LIB_EXT_VERSION}: custom/libcustcalc${LIB_EXT_VERSION}
	${Q} ${RM} -f $@
	${CP} -p $? $@

libcustcalc${LIB_EXT}: libcustcalc${LIB_EXT_VERSION}
	${Q} ${RM} -f $@
	${LN} -s $? $@

libcustcalc${LIB_EXT_VER}: libcustcalc${LIB_EXT_VERSION}
	${Q} ${RM} -f $@
	${LN} -s $? $@

###
#
# building calc-static and static lib*.a libraries
#
###

calc-static${EXT}: .hsrc ${CALCOBJS} \
		   ${CALC_STATIC_LIBS} ${MK_SET}
	${RM} -f $@
	${CC} ${LDFLAGS} ${CALCOBJS} ${LD_STATIC} ${CALC_STATIC_LIBS} \
	      ${READLINE_LIB} ${READLINE_EXTRAS} -o $@

libcalc.a: ${LIBOBJS} ${MK_SET}
	${RM} -f libcalc.a
	${AR} qc libcalc.a ${LIBOBJS}
	${RANLIB} libcalc.a
	${CHMOD} 0644 libcalc.a

libcustcalc.a: custom/libcustcalc.a
	${Q} ${RM} -f $@
	${CP} -f $? $@

custom/libcustcalc.a: custom/Makefile
	cd custom; ${MAKE} -f Makefile libcustcalc.a

sample_rand-static${EXT}: sample_rand.o ${CALC_STATIC_LIBS} \
			  ${MK_SET}
	${CC} ${LDFLAGS} sample_rand.o ${LD_STATIC} \
	      ${CALC_STATIC_LIBS} ${READLINE_LIB} ${READLINE_EXTRAS} -o $@

sample_many-static${EXT}: sample_many.o ${CALC_STATIC_LIBS} \
			  ${MK_SET}
	${CC} ${LDFLAGS} sample_many.o ${LD_STATIC} \
	      ${CALC_STATIC_LIBS} ${READLINE_LIB} ${READLINE_EXTRAS} -o $@

###
#
# Home grown make dependency rules.  Your system may not support
# or have the needed tools.  You can ignore this section.
#
# We will form a skeleton tree of *.c files containing only #include "foo.h"
# lines and .h files containing the same lines surrounded by multiple include
# prevention lines.  This allows us to build a static depend list that will
# satisfy all possible cpp symbol definition combinations.
#
###

depend: hsrc custom/Makefile
	${Q} if [ -f ${MAKE_FILE}.bak ]; then \
		echo "${MAKE_FILE}.bak exists, remove or move it"; \
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
	${V} echo '=-=-=-=-= Invoking depend rule for cscript =-=-=-=-='
	${Q} cd cscript; ${MAKE} -f Makefile depend
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking depend rule for custom =-=-=-=-='
	${Q} cd custom; ${MAKE} -f Makefile depend
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${Q} echo forming skel
	${Q} ${RM} -rf skel
	${Q} ${MKDIR} -p skel
	-${Q} for i in ${C_SRC} ${BUILD_C_SRC}; do \
	    ${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" | \
	    ${GREP} -E -v '\.\./getopt/getopt\.h' > "skel/$$i"; \
	done
	${Q} ${MKDIR} -p skel/custom
	-${Q} for i in ${H_SRC} ${BUILD_H_SRC} custom.h /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#if !defined($$tag)" > "skel/$$i"; \
		echo "#define $$tag" >> "skel/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" | \
		    LANG=C ${SORT} -u >> "skel/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/$$i"; \
	    fi; \
	done
	${Q} ${RM} -f skel/makedep.out
	${Q} echo top level skel formed
	${Q} echo forming dependency list
	${Q} :> skel/makedep.out
	${Q} cd skel; ${MAKEDEPEND} \
	    -w 1 -f makedep.out -- \
	    ${CFLAGS} -- \
	    ${C_SRC} ${BUILD_C_SRC} 2>/dev/null
	-${Q} for i in ${C_SRC} ${BUILD_C_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
	      echo "$$i" | ${SED} 's/^\(.*\)\.c/\1.o: \1.c/'; \
	    fi; \
	done >> skel/makedep.out
	${Q} LANG=C ${SORT} -u skel/makedep.out -o skel/makedep.out
	${Q} echo dependency list formed
	${Q} echo forming new ${MAKE_FILE}
	${Q} ${RM} -f ${MAKE_FILE}.bak
	${Q} ${MV} ${MAKE_FILE} ${MAKE_FILE}.bak
	${Q} ${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' \
		    ${MAKE_FILE}.bak > ${MAKE_FILE}
	${Q} ${GREP} -E -v '^#' skel/makedep.out >> ${MAKE_FILE}
	${Q} echo removing top level skel
	${Q} ${RM} -rf skel
	-${Q} if ${CMP} -s ${MAKE_FILE}.bak ${MAKE_FILE}; then \
	    echo 'top level ${MAKE_FILE} was already up to date'; \
	    echo 'restoring original ${MAKE_FILE}'; \
	    ${MV} -f ${MAKE_FILE}.bak ${MAKE_FILE}; \
	else \
	    echo 'old ${MAKE_FILE} is now ${MAKE_FILE}.bak'; \
	    echo 'new top level ${MAKE_FILE} formed'; \
	    echo 'try: diff -u ${MAKE_FILE}.bak ${MAKE_FILE}'; \
	fi

# generate the list of h files for lower level depend use
#
h_list:
	-${Q} for i in ${H_SRC} ${BUILD_H_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i; \
	    fi; \
	done

###
#
# calc version
#
# calc_version:
#	This rule is the most accurate as it uses calc source to
#	produce the version value.  This rule produces a full
#	version string.  Note that the full version could be 4
#	or 3 levels long depending on the minor patch number.
#
# version:
#	This rule simply echoes the value found in this makefile.
#	This rule produces the full version string.  Note that the
#	full version could be 4 or 3 levels long depending on the
#	minor patch number.
#
###

calc_version: ver_calc${EXT}
	@./ver_calc${EXT}

version:
	@echo ${VERSION}

ver_calc${EXT}: version.c strl.c have_strlcpy.h have_strlcat.h endian_calc.h len_bits.h have_unused.h
	${RM} -f $@
	${LCC} ${ICFLAGS} -DCALC_VER ${ILDFLAGS} version.c strl.c -o $@

###
#
# File distribution list generation.  You can ignore this section.
#
# We will form the names of source files as if they were in a
# sub-directory called calc.
#
###

distlist: ${DISTLIST} custom/Makefile
	${Q} (for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i; \
	    fi; \
	done; \
	(cd help; ${MAKE} -f Makefile -s $@); \
	(cd cal; ${MAKE} -f Makefile -s $@); \
	(cd custom; ${MAKE} -f Makefile -s $@); \
	(cd cscript; ${MAKE} -f Makefile -s $@) \
	) | LANG=C ${SORT} -u

buildlist:
	${Q} (for i in ${BUILD_ALL} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i; \
	    fi; \
	done; \
	(cd help; ${MAKE} -f Makefile -s $@); \
	(cd cal; ${MAKE} -f Makefile -s $@); \
	(cd custom; ${MAKE} -f Makefile -s $@); \
	(cd cscript; ${MAKE} -f Makefile -s $@) \
	) | LANG=C ${SORT} -u

distdir: custom/Makefile
	${Q} (echo .; \
	(cd help; ${MAKE} -f Makefile -s $@); \
	(cd cal; ${MAKE} -f Makefile -s $@); \
	(cd custom; ${MAKE} -f Makefile -s $@); \
	(cd cscript; ${MAKE} -f Makefile -s $@) \
	) | LANG=C ${SORT} -u

calcliblist: custom/Makefile
	${Q} (for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i; \
	    fi; \
	done; \
	(cd help; ${MAKE} -f Makefile -s $@); \
	(cd cal; ${MAKE} -f Makefile -s $@); \
	(cd custom; ${MAKE} -f Makefile -s $@); \
	(cd cscript; ${MAKE} -f Makefile -s $@) \
	) | LANG=C ${SORT} -u

calcliblistfmt:
	${Q} ${MAKE} -f Makefile calcliblist | \
	    ${FMT} -64 | ${SED} -e 's/^/	/'

Makefile.simple:
	${Q} echo Support for $@ was dropped after the the release of calc v2.14.3.0.

custom/Makefile.simple:
	${Q} echo Support for $@ was dropped after the the release of calc v2.14.3.0.

verifydist:
	@${MAKE} -f Makefile Q= V=@ distdir >/dev/null 2>&1
	@${MAKE} -f Makefile Q= V=@ distlist >/dev/null 2>&1

###
#
# Doing a 'make check' will cause the regression test suite to be executed.
# This rule will try to build anything that needs to be built before
# executing the regression test suite.
#
# Doing a 'make chk' will cause ONLY the context around interesting
# (and error) messages to be printed.  Unlike 'make check', this
# rule does NOT cause things to be built.  I.e., the all rule is
# not invoked.
#
###

check: all ./cal/regress.cal
	${CALC_ENV} ./calc${EXT} -d -q read regress

chk: ./cal/regress.cal
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${CALC_ENV} ./calc${EXT} -d -q read regress 2>&1 | ${AWK} -f check.awk
	${Q} echo
	${Q} echo 'chk OK'
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

###
#
# debug
#
# make calcinfo:
#	* print information about the host and compile environment
#
# make env:
#	* print major Makefile variables
#
# make mkdebug:
#	* print major Makefile variables
#	* build anything not yet built
#
# make full_debug:
#	* remove everything that was previously built
#	* print major Makefile variables
#	* make everything
#	* run the regression tests
#
# make debug:
#	* run 'make full_debug' and write stdout and stderr to debug.out
###

calcinfo:
	@echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	@echo
	@echo '=-=-= UNAME=${UNAME} =-=-='
	@echo
	@echo '=-=-= output of $${UNAME} -a follows =-=-='
	-@${UNAME} -a
	@echo '=-=-= end of output of $${UNAME} -a =-=-='
	@echo
	@echo '=-=-= output of $${UNAME} -s follows =-=-='
	-@${UNAME} -s
	@echo '=-=-= end of output of $${UNAME} -s =-=-='
	@echo
	@echo '=-=-= output of $${UNAME} -p follows =-=-='
	-@${UNAME} -p
	@echo '=-=-= end of output of $${UNAME} -p =-=-='
	@echo
	@echo '=-=-= output of $${UNAME} -m follows =-=-='
	-@${UNAME} -m
	@echo '=-=-= end of output of $${UNAME} -m =-=-='
	@echo
	@echo '=-=-= output of $${UNAME} -o follows =-=-='
	-@${UNAME} -o
	@echo '=-=-= end of output of $${UNAME} -o =-=-='
	@echo
	@echo '=-=-= output of top makefile variables follows =-=-='
	@echo "target= ${target}"
	@echo "arch= ${arch}"
	@echo "hardware= ${hardware}"
	@echo "OSNAME= ${OSNAME}"
	@echo "SHELL= ${SHELL}"
	@echo '=-=-= end of output top makefile variables =-=-='
	@echo
	@echo '=-=-= HOSTNAME=${HOSTNAME} =-=-='
	@echo
	@echo '=-=-= output of $${HOSTNAME} follows =-=-='
	-@${HOSTNAME}
	@echo '=-=-= end of output of $${HOSTNAME} =-=-='
	@echo
	@echo '=-=-= DATE=${DATE} =-=-='
	@echo
	@echo '=-=-= output of $${DATE} follows =-=-='
	-@${DATE}
	@echo '=-=-= end of output of $${DATE} =-=-='
	@echo
	@echo '=-=-= LCC=${LCC} =-=-='
	@echo
	@echo '=-=-= output of $${LCC} -v follows =-=-='
	-@${LCC} -v
	@echo '=-=-= end of output of $${LCC} -v =-=-='
	@echo
	@echo '=-=-= PWD=${PWD} =-=-='
	@echo
	@echo '=-=-= output of echo $${PWD} follows =-=-='
	-@echo ${PWD}
	@echo '=-=-= end of output of echo $${PWD} =-=-='
	@echo
	@echo '=-=-= PWDCMD=${PWDCMD} =-=-='
	@echo
	@echo '=-=-= output of $${PWDCMD} follows =-=-='
	-@${PWDCMD}
	@echo '=-=-= end of output of $${PWDCMD} =-=-='
	@echo
	@echo '=-=-= VERSION=${VERSION} =-=-='
	@echo
	@echo '=-=-= output of echo $${VERSION} follows =-=-='
	-@echo ${VERSION}
	@echo '=-=-= end of output of echo $${VERSION} =-=-='
	@echo
	@echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

env:
	@echo '=-=-=-=-= dumping major make variables =-=-=-=-='
	@echo 'ALLOW_CUSTOM=${ALLOW_CUSTOM}'; echo ''
	@echo 'AR=${AR}'; echo ''
	@echo 'ARCH_CFLAGS=${ARCH_CFLAGS}'; echo ''
	@echo 'AWK=${AWK}'; echo ''
	@echo 'BINDIR=${BINDIR}'; echo ''
	@echo 'BLD_TYPE=${BLD_TYPE}'; echo ''
	@echo 'BUILD_ALL=${BUILD_ALL}'; echo ''
	@echo 'BUILD_C_SRC=${BUILD_C_SRC}'; echo ''
	@echo 'BUILD_H_SRC=${BUILD_H_SRC}'; echo ''
	@echo 'BYTE_ORDER=${BYTE_ORDER}'; echo ''
	@echo 'CALC_BYTE_ORDER=${CALC_BYTE_ORDER}'; echo ''
	@echo 'CALC_DYNAMIC_LIBS=${CALC_DYNAMIC_LIBS}'; echo ''
	@echo 'CALC_ENV=${CALC_ENV}'; echo ''
	@echo 'CALC_INCDIR=${CALC_INCDIR}'; echo ''
	@echo 'CALCLIBLIST=${CALCLIBLIST}'; echo ''
	@echo 'CALCOBJS=${CALCOBJS}'; echo ''
	@echo 'CALCPAGER=${CALCPAGER}'; echo ''
	@echo 'CALCPATH=${CALCPATH}'; echo ''
	@echo 'CALCRC=${CALCRC}'; echo ''
	@echo 'CALC_SHAREDIR=${CALC_SHAREDIR}'; echo ''
	@echo 'CALCSRC=${CALCSRC}'; echo ''
	@echo 'CALC_STATIC_LIBS=${CALC_STATIC_LIBS}'; echo ''
	@echo 'CALC_TOOLS=${CALC_TOOLS}'; echo ''
	@echo 'CAT=${CAT}'; echo ''
	@echo 'CATDIR=${CATDIR}'; echo ''
	@echo 'CATEXT=${CATEXT}'; echo ''
	@echo 'CCBAN=${CCBAN}'; echo ''
	@echo 'CC=${CC}'; echo ''
	@echo 'CCMISC=${CCMISC}'; echo ''
	@echo 'CCOPT=${CCOPT}'; echo ''
	@echo 'CCWARN=${CCWARN}'; echo ''
	@echo 'CCWERR=${CCWERR}'; echo ''
	@echo 'CFLAGS=${CFLAGS}'; echo ''
	@echo 'CHMOD=${CHMOD}'; echo ''
	@echo 'CLANG_FORMAT=${CLANG_FORMAT}'; echo ''
	@echo 'CMP=${CMP}'; echo ''
	@echo 'CO=${CO}'; echo ''
	@echo 'COMMON_ADD=${COMMON_ADD}'; echo ''
	@echo 'COMMON_CFLAGS=${COMMON_CFLAGS}'; echo ''
	@echo 'COMMON_LDFLAGS=${COMMON_LDFLAGS}'; echo ''
	@echo 'CONFIG_MKF=${CONFIG_MKF}'; echo ''
	@echo 'CP=${CP}'; echo ''
	@echo 'CSCRIPT_TARGETS=${CSCRIPT_TARGETS}'; echo ''
	@echo 'C_SRC=${C_SRC}'; echo ''
	@echo 'CTAGS=${CTAGS}'; echo ''
	@echo 'CUSTOMCALDIR=${CUSTOMCALDIR}'; echo ''
	@echo 'CUSTOMHELPDIR=${CUSTOMHELPDIR}'; echo ''
	@echo 'CUSTOMINCDIR=${CUSTOMINCDIR}'; echo ''
	@echo 'DATE=${DATE}'; echo ''
	@echo 'DEBUG=${DEBUG}'; echo ''
	@echo 'DEFAULT_LIB_INSTALL_PATH=${DEFAULT_LIB_INSTALL_PATH}'; echo ''
	@echo 'DIFF=${DIFF}'; echo ''
	@echo 'DISTLIST=${DISTLIST}'; echo ''
	@echo 'DYNAMIC_FIRST_TARGETS=${DYNAMIC_FIRST_TARGETS}'; echo ''
	@echo 'EARLY_TARGETS=${EARLY_TARGETS}'; echo ''
	@echo 'ECHON=${ECHON}'; echo ''
	@echo 'E=${E}'; echo ''
	@echo 'EXT=${EXT}'; echo ''
	@echo 'EXTRA_CFLAGS=${EXTRA_CFLAGS}'; echo ''
	@echo 'EXTRA_LDFLAGS=${EXTRA_LDFLAGS}'; echo ''
	@echo 'FMT=${FMT}'; echo ''
	@echo 'FSANITIZE=${FSANITIZE}'; echo ''
	@echo 'GREP=${GREP}'; echo ''
	@echo 'GZIP=${GZIP}'; echo ''
	@echo 'HAVE_ARC4RANDOM=${HAVE_ARC4RANDOM}'; echo ''
	@echo 'HAVE_ENVIRON=${HAVE_ENVIRON}'; echo ''
	@echo 'HAVE_GETPGID=${HAVE_GETPGID}'; echo ''
	@echo 'HAVE_GETRUSAGE=${HAVE_GETRUSAGE}'; echo ''
	@echo 'HAVE_GETSID=${HAVE_GETSID}'; echo ''
	@echo 'HAVE_PRAGMA_GCC_POSION=${HAVE_PRAGMA_GCC_POSION}'; echo ''
	@echo 'HAVE_STRLCAT=${HAVE_STRLCAT}'; echo ''
	@echo 'HAVE_STRLCPY=${HAVE_STRLCPY}'; echo ''
	@echo 'HAVE_UNUSED=${HAVE_UNUSED}'; echo ''
	@echo 'HAVE_URANDOM_H=${HAVE_URANDOM_H}'; echo ''
	@echo 'HELPDIR=${HELPDIR}'; echo ''
	@echo 'H=${H}'; echo ''
	@echo 'HOSTNAME=${HOSTNAME}'; echo ''
	@echo 'H_SRC=${H_SRC}'; echo ''
	@echo 'ICFLAGS=${ICFLAGS}'; echo ''
	@echo 'ILDFLAGS=${ILDFLAGS}'; echo ''
	@echo 'INCDIR=${INCDIR}'; echo ''
	@echo 'LANG=${LANG}'; echo ''
	@echo 'LATE_TARGETS=${LATE_TARGETS}'; echo ''
	@echo 'LCC=${LCC}'; echo ''
	@echo 'LDCONFIG=${LDCONFIG}'; echo ''
	@echo 'LD_DEBUG=${LD_DEBUG}'; echo ''
	@echo 'LDFLAGS=${LDFLAGS}'; echo ''
	@echo 'LD_SHARE=${LD_SHARE}'; echo ''
	@echo 'LIBCALC_SHLIB=${LIBCALC_SHLIB}'; echo ''
	@echo 'LIBCUSTCALC_SHLIB=${LIBCUSTCALC_SHLIB}'; echo ''
	@echo 'LIBDIR=${LIBDIR}'; echo ''
	@echo 'LIB_EXT=${LIB_EXT}'; echo ''
	@echo 'LIB_EXT_VER=${LIB_EXT_VER}'; echo ''
	@echo 'LIB_EXT_VERSION=${LIB_EXT_VERSION}'; echo ''
	@echo 'LIB_H_SRC=${LIB_H_SRC}'; echo ''
	@echo 'LIBOBJS=${LIBOBJS}'; echo ''
	@echo 'LIBSRC=${LIBSRC}'; echo ''
	@echo 'LICENSE=${LICENSE}'; echo ''
	@echo 'LN=${LN}'; echo ''
	@echo 'LOCAL_MKF=${LOCAL_MKF}'; echo ''
	@echo 'LONG_BITS=${LONG_BITS}'; echo ''
	@echo 'LS=${LS}'; echo ''
	@echo 'MAKEDEPEND=${MAKEDEPEND}'; echo ''
	@echo 'MAKE_FILE=${MAKE_FILE}'; echo ''
	@echo 'MAKE=${MAKE}'; echo ''
	@echo 'MANDIR=${MANDIR}'; echo ''
	@echo 'MANEXT=${MANEXT}'; echo ''
	@echo 'MAN=${MAN}'; echo ''
	@echo 'MINGW=${MINGW}'; echo ''
	@echo 'MKDIR=${MKDIR}'; echo ''
	@echo 'MK_SET=${MK_SET}'; echo ''
	@echo 'MV=${MV}'; echo ''
	@echo 'OBJS=${OBJS}'; echo ''
	@echo 'OFF_T_BITS=${OFF_T_BITS}'; echo ''
	@echo 'OSNAME=${OSNAME}'; echo ''
	@echo 'PHONY=${PHONY}'; echo ''
	@echo 'PREFIX=${PREFIX}'; echo ''
	@echo 'PURIFY=${PURIFY}'; echo ''
	@echo 'PWDCMD=${PWDCMD}'; echo ''
	@echo 'PWD=${PWD}'; echo ''
	@echo 'Q=${Q}'; echo ''
	@echo 'RANLIB=${RANLIB}'; echo ''
	@echo 'READLINE_EXTRAS=${READLINE_EXTRAS}'; echo ''
	@echo 'READLINE_INCLUDE=${READLINE_INCLUDE}'; echo ''
	@echo 'READLINE_LIB=${READLINE_LIB}'; echo ''
	@echo 'RMDIR=${RMDIR}'; echo ''
	@echo 'RM=${RM}'; echo ''
	@echo 'SAMPLE_C_SRC=${SAMPLE_C_SRC}'; echo ''
	@echo 'SAMPLE_H_SRC=${SAMPLE_H_SRC}'; echo ''
	@echo 'SAMPLE_OBJ=${SAMPLE_OBJ}'; echo ''
	@echo 'SAMPLE_STATIC_TARGETS=${SAMPLE_STATIC_TARGETS}'; echo ''
	@echo 'SAMPLE_TARGETS=${SAMPLE_TARGETS}'; echo ''
	@echo 'SCRIPTDIR=${SCRIPTDIR}'; echo ''
	@echo 'SDIFF=${SDIFF}'; echo ''
	@echo 'SED=${SED}'; echo ''
	@echo 'SHELL=${SHELL}'; echo ''
	@echo 'SORT=${SORT}'; echo ''
	@echo 'SPLINT_OPTS=${SPLINT_OPTS}'; echo ''
	@echo 'SPLINT=${SPLINT}'; echo ''
	@echo 'S=${S}'; echo ''
	@echo 'STATIC_FIRST_TARGETS=${STATIC_FIRST_TARGETS}'; echo ''
	@echo 'STRIP=${STRIP}'; echo ''
	@echo 'SYM_DYNAMIC_LIBS=${SYM_DYNAMIC_LIBS}'; echo ''
	@echo 'TAIL=${TAIL}'; echo ''
	@echo 'TARGET_MKF=${TARGET_MKF}'; echo ''
	@echo 'TARGETS=${TARGETS}'; echo ''
	@echo 'TEE=${TEE}'; echo ''
	@echo 'TOUCH=${TOUCH}'; echo ''
	@echo 'TRAILBLANK=${TRAILBLANK}'; echo ''
	@echo 'TRUE=${TRUE}'; echo ''
	@echo 'T=${T}'; echo ''
	@echo 'UNAME=${UNAME}'; echo ''
	@echo 'UPDATE_VER=${UPDATE_VER}'; echo ''
	@echo 'USE_READLINE=${USE_READLINE}'; echo ''
	@echo 'UTIL_C_SRC=${UTIL_C_SRC}'; echo ''
	@echo 'UTIL_FILES=${UTIL_FILES}'; echo ''
	@echo 'UTIL_MISC_SRC=${UTIL_MISC_SRC}'; echo ''
	@echo 'UTIL_OBJS=${UTIL_OBJS}'; echo ''
	@echo 'UTIL_PROGS=${UTIL_PROGS}'; echo ''
	@echo 'UTIL_TMP=${UTIL_TMP}'; echo ''
	@echo 'VERSION=${VERSION}'; echo ''
	@echo 'VER=${VER}'; echo ''
	@echo 'V=${V}'; echo ''
	@echo 'WNO_ERROR_LONG_LONG=${WNO_ERROR_LONG_LONG}'; echo ''
	@echo 'WNO_IMPLICT=${WNO_IMPLICT};' echo ''
	@echo 'WNO_LONG_LONG=${WNO_LONG_LONG}'; echo ''
	@echo 'XARGS=${XARGS}'; echo ''
	@echo 'arch=${arch}'; echo ''
	@echo 'hardware=${hardware}'; echo ''
	@echo 'target=${target}'; echo ''
	@echo '=-=-=-=-= ${MAKE_FILE} end of major make variable dump =-=-=-=-='

mkdebug: env version.c
	@echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	@echo '=-=-=-= Contents of ${LOCAL_MKF} follows =-=-=-='
	-@${CAT} ${LOCAL_MKF}
	@echo '=-=-=-= End of contents of ${LOCAL_MKF} =-=-=-='
	@echo '=-=-=-= Contents of ${CONFIG_MKF} follows =-=-=-='
	-@${CAT} ${CONFIG_MKF}
	@echo '=-=-=-= End of contents of ${CONFIG_MKF} =-=-=-='
	@echo '=-=-=-= Contents of ${TARGET_MKF} follows =-=-=-='
	-@${CAT} ${TARGET_MKF}
	@echo '=-=-=-= End of contents of ${TARGET_MKF} =-=-=-='
	@echo '=-=-=-= Determining the source version =-=-=-='
	-@${MAKE} -f Makefile Q= V=@ ver_calc${EXT}
	-@./ver_calc${EXT}
	@echo '=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ all =-=-=-='
	@${MAKE} -f Makefile Q= H=@ S= E= V=@ all
	@echo '=-=-=-= Back to the main Makefile for $@ rule =-=-=-='
	@echo '=-=-=-= Determining the binary version =-=-=-='
	-@./calc${EXT} -e -q -v
	@echo '=-=-=-= Back to the main Makefile for $@ rule =-=-=-='
	@echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

full_debug: calcinfo env
	@echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	@echo '=-=-=-= Output of ${MAKE} -s distdir follows =-=-=-='
	-@${MAKE} -f Makefile -s distdir
	@echo '=-=-=-= End of output of {MAKE} -s distdir =-=-=-='
	@echo '=-=-=-= Output of ${MAKE} -s distlist follows =-=-=-='
	-@${MAKE} -f Makefile -s distlist
	@echo '=-=-=-= End of output of {MAKE} -s distlist =-=-=-='
	@echo '=-=-=-= Output of ${MAKE} -s buildlist follows =-=-=-='
	-@${MAKE} -f Makefile -s buildlist
	@echo '=-=-=-= End of output of {MAKE} -s buildlist =-=-=-='
	@echo '=-=-=-= Contents of ${LOCAL_MKF} follows =-=-=-='
	-@${CAT} ${LOCAL_MKF}
	@echo '=-=-=-= End of contents of ${LOCAL_MKF} =-=-=-='
	@echo '=-=-=-= Contents of ${CONFIG_MKF} follows =-=-=-='
	-@${CAT} ${CONFIG_MKF}
	@echo '=-=-=-= End of contents of ${CONFIG_MKF} =-=-=-='
	@echo '=-=-=-= Contents of ${TARGET_MKF} follows =-=-=-='
	-@${CAT} ${TARGET_MKF}
	@echo '=-=-=-= End of contents of ${TARGET_MKF} =-=-=-='
	@echo '=-=-= Invoking ${MAKE} -f Makefile Q= V=@ clobber =-=-=-='
	-@${MAKE} -f Makefile Q= H=@ S= E= V=@ clobber
	@echo '=-=-=-= Back to the main Makefile for $@ rule =-=-=-='
	@echo '=-=-=-= start of chk_tree pass #0 =-=-=-=-=-='
	-@./chk_tree
	@echo '=-=-=-=-=-= end of chk_tree pass #0 =-=-=-=-=-='
	@echo '=-=-= Invoking ${MAKE} -f Makefile Q= V=@ all =-=-=-='
	@echo '=-=-= this may take a bit of time =-=-='
	-@${MAKE} -f Makefile Q= H=@ S= E= V=@ all
	@echo '=-=-=-= Back to the main Makefile for $@ rule =-=-=-='
	@echo '=-=-=-= Determining C compiler and C enviroment compatibility =-=-=-='
	@echo '=-=-= Invoking ${MAKE} -f Makefile Q= V=@ chk_c${EXT} =-=-=-='
	-@${MAKE} -f Makefile Q= H=@ S= E= V=@ chk_c${EXT}
	@echo '=-=-= Executing chk_c${EXT} -c =-=-=-='
	-@./chk_c${EXT} -c
	@echo '=-=-=-= Back to the main Makefile for $@ rule =-=-=-='
	@echo '=-=-=-= Determining the source version =-=-=-='
	@echo '=-=-= Invoking ${MAKE} -f Makefile Q= H=@ S= E= V=@ ver_calc${EXT} =-=-=-='
	-@${MAKE} -f Makefile Q= H=@ S= E= V=@ ver_calc${EXT}
	-@./ver_calc${EXT}
	@echo '=-=-=-= Back to the main Makefile for $@ rule =-=-=-='
	-@${ECHON} '=-=-=-= Print #define values if custom functions '
	@echo 'are allowed =-=-=-='
	-@${CALC_ENV} ./calc${EXT} -e -q -C 'print custom("sysinfo", 2);'
	@echo '=-=-=-= Back to the main Makefile for $@ rule =-=-=-='
	@echo '=-=-=-= start of chk_tree pass #1 =-=-=-=-=-='
	-@./chk_tree
	@echo '=-=-=-=-=-= end of chk_tree pass #1 =-=-=-=-=-='
	@echo '=-=-= Invoking ${MAKE} -f Makefile Q= V=@ check =-=-=-='
	@echo '=-=-= this may take a while =-=-='
	-@${MAKE} -f Makefile Q= V=@ check
	@echo '=-=-=-= Back to the main Makefile for $@ rule =-=-=-='
	@echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

debug:
	-${RM} -f debug.out
	-${MAKE} -f Makefile full_debug 2>&1 | ${TEE} debug.out
	@echo
	@echo 'To file a bug report / open a GitHub Issue, visit:'
	@echo
	@echo '    https://github.com/lcn2/calc/issues'
	@echo
	@echo 'Click the ((New issue)) button to file a bug report.'
	@echo
	@echo 'Please attach the debug.out file to the bug report:'
	@echo
	-@${LS} -l debug.out
	@echo

###
#
# maintainer rules - rules that are useful for people who maintain the calc source
#
# testfuncsort
#
#    Check on the sort of the builtin function list.
#    If the builtins[] array in func.c is in dictionary sorted order,
#    this rule prints nothing.  If there is a builtin function that
#    is out of order, sdiff is used to print the builtin function sort
#    problem and does an exit 1.
#
# prep
#
#    Perform tests and build actions that are needed prior to a release
#    of calc.  The "make prep" should NOT exit non-zero but instead it
#    should print "All is OK" and exit 0.
#
# run
#
#    Execute calc using shared libraries from the local directory.
#    Run calc with reading of the startup scripts disabled.
#
###

testfuncsort: ./calc${EXT}
	@${RM} -f func.show func.sort
	@${CALC_ENV} ./calc${EXT} -d -u show builtin | grep '^[A-Za-z0-9]' > func.show
	@${CALC_ENV} ./calc${EXT} -d -u show builtin | grep '^[A-Za-z0-9]' | LANG=C LC_ALL=C ${SORT} -d -u > func.sort
	@if ! cmp -s func.show func.sort; then \
	    echo 1>&2; \
	    echo "ERROR: builtins[] arrray in func.c is not in dictionary sorted order" 1>&2; \
	    echo 1>&2; \
	    echo "${CALC_ENV} ./calc${EXT} -d -u show builtin | grep '^[A-Za-z0-9]' > func.show" 1>&2; \
	    echo "${CALC_ENV} ./calc${EXT} -d -u show builtin | grep '^[A-Za-z0-9]' |" \
	         "LANG=C LC_ALL=C ${SORT} -d -u > func.sort" 1>&2; \
	    echo 1>&2; \
	    echo ${SDIFF} func.show func.sort 1>&2; \
	    echo 1>&2; \
	    ${SDIFF} func.show func.sort 1>&2; \
	    exit 1; \
	fi
	@${RM} -f func.show func.sort

prep:
	${Q}echo '=-=-=-=-=-= start of ${MAKE} clobber =-=-=-=-=-='
	${Q}echo
	${MAKE} clobber
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of ${MAKE} clobber =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of ${MAKE} verifydist =-=-=-=-=-='
	${Q}echo
	${MAKE} -s verifydist
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of ${MAKE} verifydist =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of chk_tree pass #0 =-=-=-=-=-='
	${Q}echo
	-./chk_tree
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of chk_tree pass #0 =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of ${TRAILBLANK} =-=-=-=-=-='
	${Q}echo
	./${TRAILBLANK}
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of ${TRAILBLANK} =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of ${MAKE} all CCWERR=-Werror =-=-=-=-=-='
	${Q}echo
	${MAKE} all CCWERR=-Werror
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of ${MAKE} all CCWERR=-Werror =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of ${MAKE} tags =-=-=-=-=-='
	${Q}echo
	${MAKE} -s tags
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of ${MAKE} tags =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of ${MAKE} depend =-=-=-=-=-='
	${Q}echo
	${MAKE} -s depend
	${Q}echo
	@if [ -f ${MAKE_FILE}.bak ]; then echo ${MAKE_FILE}.bak exists 1>&2; exit 1; fi
	@if [ -f cscript/${MAKE_FILE}.bak ]; then echo cscript/${MAKE_FILE}.bak exists 1>&2; exit 2; fi
	@if [ -f custom/${MAKE_FILE}.bak ]; then echo custom/${MAKE_FILE}.bak exists 1>&2; exit 3; fi
	${Q}echo '=-=-=-=-=-= end of ${MAKE} depend =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of ${MAKE} testfuncsort =-=-=-=-=-='
	${Q}echo
	${MAKE} -s testfuncsort
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of ${MAKE} testfuncsort =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of ${UPDATE_VER} =-=-=-=-=-='
	${Q}echo
	./${UPDATE_VER}
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of ${UPDATE_VER} =-=-=-=-=-='
	${Q}echo
	${Q}echo '=-=-=-=-=-= start of ${MAKE} chk =-=-=-=-=-='
	${Q}echo
	${MAKE} -s chk
	${Q}echo
	${Q}echo '=-=-=-=-=-= end of ${MAKE} chk =-=-=-=-=-='
	${Q}echo
	@${Q}if ! ./chk_tree >/dev/null 2>&1; then \
	    echo almost satisfactory except for chk_tree; \
	else \
	    echo All is OK; \
	fi
	${Q}echo

run:
	${CALC_ENV} ./calc${EXT} -q

# IMPORTANT DEBUGGING SUGGESTION:
#
# 	Debugging is likely to be more productive if you compile calc as a static binary, and
# 	compile without any optimization.  I.e., set:
#
# 	    BLD_TYPE= calc-static-only
# 	    DEBUG:= -O0 -ggdb3
#
# 	 When switching to building static only, do a "make clobber" first to remove any
# 	 dynamic calc related compile files.
#
# Consider un-commenting the appropriate "Address Sanitizer (ASAN)" section in Makefile.local.
#
lldb:
	${CALC_ENV} ${LLDB} -- ./calc${EXT} -q

# IMPORTANT DEBUGGING SUGGESTION:
#
# 	Debugging is likely to be more productive if you compile calc as a static binary, and
# 	compile without any optimization.  I.e., set:
#
# 	    BLD_TYPE= calc-static-only
# 	    DEBUG:= -O0 -ggdb3
#
# 	 When switching to building static only, do a "make clobber" first to remove any
# 	 dynamic calc related compile files.
#
# Consider un-commenting the appropriate "Address Sanitizer (ASAN)" section in Makefile.local.
#
gdb:
	${CALC_ENV} ${GDB} --args ./calc${EXT} -q

###
#
# rpm rules
#
###

# NOTE: Only the 2 rpm rules should set ${RPM_TOP}!
#
# When making calc RPM, ${RPM_TOP} will be set to the tree
# under which rpm files are built.  You should NOT set RPM_TOP
# by yourself.  Only make rpm and make rpm-preclean should
# set this value.

rpm: clobber rpm-preclean rpm.mk calc.spec.in
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${MAKE} -f rpm.mk all V=${V} RPM_TOP="${RPM_TOP}" Q= S= E=
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

rpm-preclean:
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${MAKE} -f rpm.mk $@ V=${V} RPM_TOP="${RPM_TOP}" Q= S= E=
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

# rpm static rules
#
rpm-hide-static:
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${RM} -rf static
	${MKDIR} -p static
	${CP} -f -p calc-static${EXT} ${SAMPLE_STATIC_TARGETS} static
	${CP} -f -p libcalc.a static
	${CP} -f -p custom/libcustcalc.a static
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

rpm-unhide-static:
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${RM} -f calc-static${EXT} ${SAMPLE_STATIC_TARGETS}
	cd static; ${CP} -f -p calc-static${EXT} ${SAMPLE_STATIC_TARGETS} ..
	${RM} -f libcalc.a
	cd static; ${CP} -f -p libcalc.a ..
	${RM} -f custom/libcustcalc.a
	cd static; ${CP} -f -p libcustcalc.a ../custom
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

rpm-chk-static:
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${CALC_ENV} ./calc-static${EXT} -d -q read regress 2>&1 | \
	    ${AWK} -f check.awk
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

rpm-clean-static:
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${RM} -rf static
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

###
#
# Utility rules
#
###

# Form the installed file list
#
inst_files: ${MK_SET} help/Makefile cal/Makefile \
	    cscript/Makefile ver_calc${EXT} custom/Makefile
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${Q} ${RM} -f inst_files
	${Q} echo ${BINDIR}/calc${EXT} > inst_files
	${Q} cd help; LANG=C \
	    ${MAKE} -f Makefile echo_inst_files | \
	    ${GREP} -E '__file__..' | ${SED} -e s'/.*__file__ //' >> ../inst_files
	${Q} cd cal; LANG=C \
	    ${MAKE} -f Makefile echo_inst_files | \
	    ${GREP} -E '__file__..' | ${SED} -e s'/.*__file__ //' >> ../inst_files
	${Q} cd custom; LANG=C \
	    ${MAKE} -f Makefile echo_inst_files | \
	    ${GREP} -E '__file__..' | ${SED} -e s'/.*__file__ //' >> ../inst_files
	${Q} cd cscript; LANG=C \
	    ${MAKE} -f Makefile echo_inst_files | \
	    ${GREP} -E '__file__..' | ${SED} -e s'/.*__file__ //' >> ../inst_files
	${Q} echo ${LIBDIR}/libcalc.a >> inst_files
	${Q} for i in ${LIB_H_SRC} ${BUILD_H_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo ${CALC_INCDIR}/$$i; \
	    fi; \
	done >> inst_files
	${Q} if [ ! -z "${MANDIR}" ]; then \
	    echo ${MANDIR}/calc.${MANEXT}; \
	fi >> inst_files
	${Q} LANG=C ${SORT} -u inst_files -o inst_files
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

# The olduninstall rule will remove calc files from the older, historic
# locations under the /usr/local directory.  If you are using the
# new default values for ${BINDIR}, ${CALC_SHAREDIR}, ${INCDIR} and ${LIBDIR}
# then you can use this rule to clean out the older calc stuff under
# the /usr/local directory.
#
# NOTE: This rule is an exception to the use of ${PREFIX}.
#	In this rule we really to want to explicitly deal
#	with legacy paths under /usr/local.
#
olduninstall:
	${RM} -f inst_files
	${MAKE} -f Makefile \
		PREFIX=/usr/local \
		BINDIR=/usr/local/bin \
		INCDIR=/usr/local/include \
		LIBDIR=/usr/local/lib/calc \
		CALC_SHAREDIR=/usr/local/lib/calc \
		HELPDIR=/usr/local/lib/calc/help \
		CALC_INCDIR=/usr/local/include/calc \
		CUSTOMCALDIR=/usr/local/lib/calc/custom \
		CUSTOMHELPDIR=/usr/local/lib/calc/help/custhelp \
		CUSTOMINCDIR=/usr/local/lib/calc/custom \
		SCRIPTDIR=/usr/local/bin/cscript \
		MANDIR=/usr/local/man/man1 \
		inst_files
	${XARGS} ${RM} -f < inst_files
	-${RMDIR} /usr/local/lib/calc/help/custhelp
	-${RMDIR} /usr/local/lib/calc/help
	-${RMDIR} /usr/local/lib/calc/custom
	-${RMDIR} /usr/local/lib/calc
	-${RMDIR} /usr/local/include/calc
	-${RMDIR} /usr/local/bin/cscript
	${RM} -f inst_files
	${RM} -f ${CALC_INCDIR}/calcerr.c
	${RM} -f ${CALC_INCDIR}/have_fgetsetpos.h
	${RM} -f ${CALC_INCDIR}/calcerr.h
	${RM} -f ${CALC_INCDIR}/alloc.h
	${RM} -f -v Makefile.simple Makefile.simple.bak
	${RM} -f -v custom/Makefile.simple custom/Makefile.simple.bak

tags: ${CALCSRC} ${LIBSRC} ${H_SRC} ${BUILD_H_SRC} ${MAKE_FILE}
	${CTAGS} -w ${CALCSRC} ${LIBSRC} ${H_SRC} ${BUILD_H_SRC}

clean:
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${RM} -f ${LIBOBJS}
	${RM} -f ${CALCOBJS}
	${RM} -f ${UTIL_OBJS}
	${RM} -f ${UTIL_TMP}
	${RM} -f ${UTIL_PROGS}
	${RM} -f ${UTIL_FILES}
	${RM} -f ${SAMPLE_OBJ}
	${RM} -f .libcustcalc_error
	${RM} -f calc.spec.sed
	${Q} echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	cd help; ${MAKE} -f Makefile clean
	${Q} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${Q} echo '=-=-=-=-= Invoking $@ rule for cal =-=-=-=-='
	cd cal; ${MAKE} -f Makefile clean
	${Q} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for custom =-=-=-=-='
	cd custom; ${MAKE} -f Makefile clean
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cscript =-=-=-=-='
	cd cscript; ${MAKE} -f Makefile clean
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${Q} echo remove files that are obsolete
	${RM} -rf lib
	${RM} -f endian.h stdarg.h libcalcerr.a cal/obj help/obj
	${RM} -f have_vs.c std_arg.h try_stdarg.c fnvhash.c
	${RM} -f have_malloc.h math_error.h string.h string.c
	${RM} -f calcerr.c calcerr.h calcerr.o
	${RM} -f align32.h args.h have_fgetsetpos.h have_getprid.h have_args.sh alloc.h
	${RM} -f have_inttypes.h have_limits.h have_newstr.h have_stdbool.h
	${RM} -f have_stdint.h have_stdlib.h have_strdup.h have_string.h have_times.h
	${RM} -f have_uid_t.h have_unistd.h have_const.h bool.h decl.h have_fpos.h
	${RM} -f have_gettime.h have_ustat.h have_statfs.h have_sys_param.h have_sys_mount.h
	${RM} -f terminal.h have_sys_vfs.h charbit.h have_posscl.h have_fpos_pos.h have_offscl.h
	${RM} -f fposval.h fposval.h.def fpos_tmp fposval_tmp offscl_tmp posscl_tmp longbits.h
	${RM} -f have_const.c have_const.o have_const${EXT}
	${RM} -f have_fgetsetpos.c have_fgetsetpos.o have_fgetsetpos${EXT}
	${RM} -f have_getprid.c have_getprid.o have_getprid${EXT}
	${RM} -f have_newstr.c have_newstr.o have_newstr${EXT}
	${RM} -f have_stdvs.c have_stdvs.o have_stdvs${EXT}
	${RM} -f have_strdup.c have_strdup.o have_strdup${EXT}
	${RM} -f have_uid_t.c have_uid_t.o have_uid_t${EXT}
	${RM} -f have_varvs.c have_varvs.o have_varvs${EXT}
	${RM} -f align32.c align32.o align32${EXT}
	${RM} -f have_fpos.c have_fpos.o have_fpos${EXT}
	${RM} -f have_gettime.c have_gettime.o have_gettime${EXT}
	${RM} -f have_ustat.c have_ustat.o have_ustat${EXT}
	${RM} -f have_statfs.c have_statfs.o have_statfs${EXT}
	${RM} -f charbit.c charbit.o charbit${EXT}
	${RM} -f have_posscl.c have_posscl.o have_posscl${EXT}
	${RM} -f have_fpos_pos.c have_fpos_pos.o have_fpos_pos${EXT}
	${RM} -f have_offscl.c have_offscl.o have_offscl${EXT}
	${RM} -f fposval.c fposval.o fposval${EXT}
	${RM} -f longbits.c longbits.o longbits${EXT}
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

clobber: clean
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${RM} -f errcode${EXT}
	${RM} -f ${SAMPLE_TARGETS}
	${RM} -f ${SAMPLE_STATIC_TARGETS}
	${RM} -f tags .hsrc hsrc
	${RM} -f ${BUILD_H_SRC}
	${RM} -f ${BUILD_C_SRC}
	${RM} -f calc${EXT}
	${RM} -f *_pure_*.[oa]
	${RM} -f *.pure_linkinfo
	${RM} -f *.pure_hardlin
	${RM} -f *.u
	${RM} -f libcalc.a
	${RM} -f libcustcalc.a
	${RM} -f calc.1 calc.usage calc.cat1
	${RM} -f calc.pixie calc.rf calc.Counts calc.cord
	${RM} -f gen_h Makefile.bak tmp.patch
	${RM} -rf skel
	${RM} -f calc.spec inst_files rpm.mk.patch tmp
	${RM} -f libcalc${LIB_EXT_VERSION}
	${RM} -f libcalc*
	${RM} -f libcustcalc${LIB_EXT_VERSION}
	${RM} -f libcustcalc*
	${RM} -f calc-static${EXT}
	${RM} -f ${CALC_STATIC_LIBS}
	${RM} -f a.out
	${RM} -f all
	${V} echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	-${RM} -f help/all; \
	    cd help; ${MAKE} -f Makefile $@
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cal =-=-=-=-='
	-${RM} -f cal/all; \
	    cd cal; ${MAKE} -f Makefile $@
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for custom =-=-=-=-='
	-${RM} -f custom/all; \
	    ${MAKE} custom/Makefile; \
	    cd custom; ${MAKE} -f Makefile $@
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cscript =-=-=-=-='
	-${RM} -f cscript/all; \
	    cd cscript; ${MAKE} -f Makefile $@
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo remove files that are obsolete
	${RM} -rf build
	${RM} -f no_implicit.arg
	${RM} -f no_implicit.c no_implicit.o no_implicit${EXT}
	${RM} -f .static .dynamic calc-dynamic-only calc-static-only
	-${Q} if [ -e .DS_Store ]; then \
	    echo ${RM} -rf .DS_Store; \
	    ${RM} -rf .DS_Store; \
	fi
	${RM} -f func.show func.sort
	${RM} -f outfile
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

# install everything
#
# NOTE: Keep the uninstall rule in the reverse order of the install rule
#
install: ${LIB_H_SRC} ${BUILD_H_SRC} calc.1 all custom/Makefile
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	-${Q} if [ ! -z "${T}" ]; then \
	    if [ ! -d ${T} ]; then \
		echo ${MKDIR} -p ${T}; \
		${MKDIR} -p ${T}; \
		echo ${CHMOD} 0755 ${T}; \
		${CHMOD} 0755 ${T}; \
	    fi; \
	fi
	-${Q} if [ ! -d ${T}${BINDIR} ]; then \
	    echo ${MKDIR} -p ${T}${BINDIR}; \
	    ${MKDIR} -p ${T}${BINDIR}; \
	    echo ${CHMOD} 0755 ${T}${BINDIR}; \
	    ${CHMOD} 0755 ${T}${BINDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${INCDIR} ]; then \
	    echo ${MKDIR} -p ${T}${INCDIR}; \
	    ${MKDIR} -p ${T}${INCDIR}; \
	    echo ${CHMOD} 0755 ${T}${INCDIR}; \
	    ${CHMOD} 0755 ${T}${INCDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${LIBDIR} ]; then \
	    echo ${MKDIR} -p ${T}${LIBDIR}; \
	    ${MKDIR} -p ${T}${LIBDIR}; \
	    echo ${CHMOD} 0755 ${T}${LIBDIR}; \
	    ${CHMOD} 0755 ${T}${LIBDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${CALC_SHAREDIR} ]; then \
	    ${MKDIR} -p ${T}${CALC_SHAREDIR}; \
	    echo ${MKDIR} -p ${T}${CALC_SHAREDIR}; \
	    echo ${CHMOD} 0755 ${T}${CALC_SHAREDIR}; \
	    ${CHMOD} 0755 ${T}${CALC_SHAREDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${HELPDIR} ]; then \
	    echo ${MKDIR} -p ${T}${HELPDIR}; \
	    ${MKDIR} -p ${T}${HELPDIR}; \
	    echo ${CHMOD} 0755 ${T}${HELPDIR}; \
	    ${CHMOD} 0755 ${T}${HELPDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${CALC_INCDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CALC_INCDIR}; \
	    ${MKDIR} -p ${T}${CALC_INCDIR}; \
	    echo ${CHMOD} 0755 ${T}${CALC_INCDIR}; \
	    ${CHMOD} 0755 ${T}${CALC_INCDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${CUSTOMCALDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CUSTOMCALDIR}; \
	    ${MKDIR} -p ${T}${CUSTOMCALDIR}; \
	    echo ${CHMOD} 0755 ${T}${CUSTOMCALDIR}; \
	    ${CHMOD} 0755 ${T}${CUSTOMCALDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${CUSTOMHELPDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CUSTOMHELPDIR}; \
	    ${MKDIR} -p ${T}${CUSTOMHELPDIR}; \
	    echo ${CHMOD} 0755 ${T}${CUSTOMHELPDIR}; \
	    ${CHMOD} 0755 ${T}${CUSTOMHELPDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${CUSTOMINCDIR} ]; then \
	    echo ${MKDIR} -p ${T}${CUSTOMINCDIR}; \
	    ${MKDIR} -p ${T}${CUSTOMINCDIR}; \
	    echo ${CHMOD} 0755 ${T}${CUSTOMINCDIR}; \
	    ${CHMOD} 0755 ${T}${CUSTOMINCDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -d ${T}${SCRIPTDIR} ]; then \
	    echo ${MKDIR} -p ${T}${SCRIPTDIR}; \
	    ${MKDIR} -p ${T}${SCRIPTDIR}; \
	    echo ${CHMOD} 0755 ${T}${SCRIPTDIR}; \
	    ${CHMOD} 0755 ${T}${SCRIPTDIR}; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -z "${MANDIR}" ]; then \
	    if [ ! -d ${T}${MANDIR} ]; then \
		echo ${MKDIR} -p ${T}${MANDIR}; \
		${MKDIR} -p ${T}${MANDIR}; \
		echo ${CHMOD} 0755 ${T}${MANDIR}; \
		${CHMOD} 0755 ${T}${MANDIR}; \
	    else \
		${TRUE}; \
	    fi; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ ! -z "${CATDIR}" ]; then \
	    if [ ! -d ${T}${CATDIR} ]; then \
		echo ${MKDIR} -p ${T}${CATDIR}; \
		${MKDIR} -p ${T}${CATDIR}; \
		echo ${CHMOD} 0755 ${T}${CATDIR}; \
		${CHMOD} 0755 ${T}${CATDIR}; \
	    else \
		${TRUE}; \
	    fi; \
	else \
	    ${TRUE}; \
	fi
	-${Q} if [ -f calc-static${EXT} ]; then \
	    if ${CMP} -s calc-static${EXT} \
			 ${T}${BINDIR}/calc-static${EXT}; then \
		${TRUE}; \
	    else \
		echo ${RM} -f ${T}${BINDIR}/calc-static.new${EXT}; \
		${RM} -f ${T}${BINDIR}/calc-static.new${EXT}; \
		echo ${CP} -f calc-static${EXT} \
			 ${T}${BINDIR}/calc-static.new${EXT}; \
		${CP} -f calc-static${EXT} \
			 ${T}${BINDIR}/calc-static.new${EXT}; \
		echo ${CHMOD} 0755 ${T}${BINDIR}/calc-static.new${EXT}; \
		${CHMOD} 0755 ${T}${BINDIR}/calc-static.new${EXT}; \
		echo ${MV} -f ${T}${BINDIR}/calc-static.new${EXT} \
			 ${T}${BINDIR}/calc-static${EXT}; \
		${MV} -f ${T}${BINDIR}/calc-static.new${EXT} \
			 ${T}${BINDIR}/calc-static${EXT}; \
		echo "installed ${T}${BINDIR}/calc-static${EXT}"; \
		echo ${RM} -f ${T}${BINDIR}/calc.new${EXT}; \
		${RM} -f ${T}${BINDIR}/calc.new${EXT}; \
		echo ${LN} -f ${T}${BINDIR}/calc-static${EXT} \
			 ${T}${BINDIR}/calc.new${EXT}; \
		${LN} -f ${T}${BINDIR}/calc-static${EXT} \
			 ${T}${BINDIR}/calc.new${EXT}; \
		echo "${MV} -f ${T}${BINDIR}/calc.new${EXT}" \
			 "${T}${BINDIR}/calc${EXT}"; \
		${MV} -f ${T}${BINDIR}/calc.new${EXT} \
			 ${T}${BINDIR}/calc${EXT}; \
		echo "linked ${T}${BINDIR}/calc-static${EXT}" \
		     "to ${T}${BINDIR}/calc${EXT}"; \
	    fi; \
	elif ${CMP} -s calc${EXT} ${T}${BINDIR}/calc${EXT}; then \
	    ${TRUE}; \
	else \
	    echo ${RM} -f ${T}${BINDIR}/calc.new${EXT}; \
	    ${RM} -f ${T}${BINDIR}/calc.new${EXT}; \
	    echo ${CP} -f calc${EXT} ${T}${BINDIR}/calc.new${EXT}; \
	    ${CP} -f calc${EXT} ${T}${BINDIR}/calc.new${EXT}; \
	    echo ${CHMOD} 0755 ${T}${BINDIR}/calc.new${EXT}; \
	    ${CHMOD} 0755 ${T}${BINDIR}/calc.new${EXT}; \
	    echo ${MV} -f ${T}${BINDIR}/calc.new${EXT} ${T}${BINDIR}/calc${EXT}; \
	    ${MV} -f ${T}${BINDIR}/calc.new${EXT} ${T}${BINDIR}/calc${EXT}; \
	    echo "installed ${T}${BINDIR}/calc${EXT}"; \
	fi
	${V} echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	${Q} cd help; ${MAKE} -f Makefile install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cal =-=-=-=-='
	${Q} cd cal; ${MAKE} -f Makefile install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for custom =-=-=-=-='
	${Q} cd custom; ${MAKE} -f Makefile install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cscript =-=-=-=-='
	${Q} cd cscript; ${MAKE} -f Makefile install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	-${Q} if [ -f libcalc.a ]; then \
	    if ${CMP} -s libcalc.a ${T}${LIBDIR}/libcalc.a; then \
		${TRUE}; \
	    else \
		echo ${RM} -f ${T}${LIBDIR}/libcalc.a.new; \
		${RM} -f ${T}${LIBDIR}/libcalc.a.new; \
		echo ${CP} -f libcalc.a ${T}${LIBDIR}/libcalc.a.new; \
		${CP} -f libcalc.a ${T}${LIBDIR}/libcalc.a.new; \
		echo ${CHMOD} 0644 ${T}${LIBDIR}/libcalc.a.new; \
		${CHMOD} 0644 ${T}${LIBDIR}/libcalc.a.new; \
		echo ${MV} -f ${T}${LIBDIR}/libcalc.a.new ${T}${LIBDIR}/libcalc.a; \
		${MV} -f ${T}${LIBDIR}/libcalc.a.new ${T}${LIBDIR}/libcalc.a; \
		echo ${RANLIB} ${T}${LIBDIR}/libcalc.a; \
		${RANLIB} ${T}${LIBDIR}/libcalc.a; \
		echo "installed ${T}${LIBDIR}/libcalc.a"; \
	   fi; \
	fi
	-${Q} if [ -f libcalc${LIB_EXT_VERSION} ]; then \
	    if ${CMP} -s libcalc${LIB_EXT_VERSION} \
			 ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}; then \
		${TRUE}; \
	    else \
		echo ${RM} -f ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}.new; \
		${RM} -f ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}.new; \
		echo ${CP} -f libcalc${LIB_EXT_VERSION} \
			 ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}.new; \
		${CP} -f libcalc${LIB_EXT_VERSION} \
			 ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}.new; \
		echo ${CHMOD} 0644 ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}.new; \
		${CHMOD} 0644 ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}.new; \
		echo ${MV} -f ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}.new \
			 ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}; \
		${MV} -f ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}.new \
			 ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}; \
		echo "installed ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}"; \
		echo ${LN} -f -s libcalc${LIB_EXT_VERSION} \
			    ${T}${LIBDIR}/libcalc${LIB_EXT}; \
		${LN} -f -s libcalc${LIB_EXT_VERSION} \
			    ${T}${LIBDIR}/libcalc${LIB_EXT}; \
		echo "installed ${T}${LIBDIR}/libcalc${LIB_EXT}"; \
		echo ${LN} -f -s libcalc${LIB_EXT_VERSION} \
			    ${T}${LIBDIR}/libcalc${LIB_EXT_VER}; \
		${LN} -f -s libcalc${LIB_EXT_VERSION} \
			    ${T}${LIBDIR}/libcalc${LIB_EXT_VER}; \
		echo "installed ${T}${LIBDIR}/libcalc${LIB_EXT_VER}"; \
	    fi; \
	fi
	-${Q} if [ -f custom/libcustcalc${LIB_EXT_VERSION} ]; then \
	    if ${CMP} -s custom/libcustcalc${LIB_EXT_VERSION} \
			 ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}; then \
		${TRUE}; \
	    else \
		echo ${RM} -f ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}.new; \
		${RM} -f ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}.new; \
		echo ${CP} -f custom/libcustcalc${LIB_EXT_VERSION} \
			 ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}.new; \
		${CP} -f custom/libcustcalc${LIB_EXT_VERSION} \
			 ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}.new; \
		echo ${CHMOD} 0644 ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}.new; \
		${CHMOD} 0644 ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}.new; \
		echo ${MV} -f ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}.new \
			 ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}; \
		${MV} -f ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}.new \
			 ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}; \
		echo "installed ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}"; \
		echo ${LN} -f -s libcustcalc${LIB_EXT_VERSION} \
			    ${T}${LIBDIR}/libcustcalc${LIB_EXT}; \
		${LN} -f -s libcustcalc${LIB_EXT_VERSION} \
			    ${T}${LIBDIR}/libcustcalc${LIB_EXT}; \
		echo "installed ${T}${LIBDIR}/libcalc${LIB_EXT}"; \
		echo ${LN} -f -s libcustcalc${LIB_EXT_VERSION} \
			    ${T}${LIBDIR}/libcustcalc${LIB_EXT_VER}; \
		${LN} -f -s libcustcalc${LIB_EXT_VERSION} \
			    ${T}${LIBDIR}/libcustcalc${LIB_EXT_VER}; \
		echo "installed ${T}${LIBDIR}/libcalc${LIB_EXT_VER}"; \
		if [ -z "${T}" -o "/" = "${T}" ]; then \
		    if [ ! -z "${LDCONFIG}" ]; then \
			echo "running ${LDCONFIG}"; \
			${LDCONFIG} -v; \
			echo "finished ${LDCONFIG}"; \
		    else \
			echo "use of ${LDCONFIG} is not required on this platform"; \
		    fi; \
		fi; \
	    fi; \
	fi
	-${Q} for i in ${LIB_H_SRC} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    ${RM} -f tmp; \
	    ${SED} -e 's/^\(#[	 ]*include[	 ][	 ]*\)"/\1"calc\//' \
	              $$i > tmp; \
	    if ${CMP} -s tmp ${T}${CALC_INCDIR}/$$i; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${CALC_INCDIR}/$$i.new; \
		${CP} -f tmp ${T}${CALC_INCDIR}/$$i.new; \
		${CHMOD} 0444 ${T}${CALC_INCDIR}/$$i.new; \
		${MV} -f ${T}${CALC_INCDIR}/$$i.new ${T}${CALC_INCDIR}/$$i; \
		echo "installed ${T}${CALC_INCDIR}/$$i"; \
	    fi; \
	    if [ -f "${T}${CALC_INCDIR}/std_arg.h" ]; then \
		${RM} -f ${T}${CALC_INCDIR}/std_arg.h; \
		echo "removed old ${T}${CALC_INCDIR}/std_arg.h"; \
	    fi; \
	    if [ -f "${T}${CALC_INCDIR}/have_malloc.h" ]; then \
		${RM} -f ${T}${CALC_INCDIR}/have_malloc.h; \
		echo "removed old ${T}${CALC_INCDIR}/have_malloc.h"; \
	    fi; \
	    if [ -f "${T}${CALC_INCDIR}/math_error.h" ]; then \
		${RM} -f ${T}${CALC_INCDIR}/math_error.h; \
		echo "removed old ${T}${CALC_INCDIR}/math_error.h"; \
	    fi; \
	    if [ -f "${T}${CALC_INCDIR}/string.h" ]; then \
		${RM} -f ${T}${CALC_INCDIR}/string.h; \
		echo "removed old ${T}${CALC_INCDIR}/string.h"; \
	    fi; \
	done
	-${Q} if [ -z "${MANDIR}" ] || [ ! -s calc.1 ]; then \
	    ${TRUE}; \
	else \
	    if ${CMP} -s calc.1 ${T}${MANDIR}/calc.${MANEXT}; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${MANDIR}/calc.${MANEXT}.new; \
		${CP} -f calc.1 ${T}${MANDIR}/calc.${MANEXT}.new; \
		${CHMOD} 0444 ${T}${MANDIR}/calc.${MANEXT}.new; \
		${MV} -f ${T}${MANDIR}/calc.${MANEXT}.new \
		      ${T}${MANDIR}/calc.${MANEXT}; \
		echo "installed ${T}${MANDIR}/calc.${MANEXT}"; \
	    fi; \
	fi
	-${Q} if [ -z "${CATDIR}" ] || [ ! -s calc.cat1 ]; then \
	    ${TRUE}; \
	else \
	    if ${CMP} -s calc.cat1 ${T}${CATDIR}/calc.${MANEXT}; then \
		${TRUE}; \
	    else \
		${RM} -f ${T}${CATDIR}/calc.${CATEXT}.new; \
		${CP} -f calc.cat1 ${T}${CATDIR}/calc.${CATEXT}.new; \
		${MV} -f ${T}${CATDIR}/calc.${CATEXT}.new \
		         ${T}${CATDIR}/calc.${CATEXT}; \
		echo "installed ${T}${CATDIR}/calc.${CATEXT}"; \
	    fi; \
	fi
	${V} # NOTE: misc install cleanup
	${Q} ${RM} -f tmp
	${V} # NOTE: remove the calcerr.c that was installed by mistake
	${V} #	     under ${INC_DIR} in calc v2.12.9.1
	${Q} ${RM} -f ${T}${CALC_INCDIR}/calcerr.c
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

# Try to remove everything that was installed
#
# NOTE: Keep the uninstall rule in the reverse order of the install rule
#
uninstall: custom/Makefile
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	-${Q} if [ -z "${CATDIR}" ]; then \
	    ${TRUE}; \
	else \
	    if [ -f "${T}${CATDIR}/calc.${CATEXT}" ]; then \
		${RM} -f "${T}${CATDIR}/calc.${CATEXT}"; \
		if [ -f "${T}${CATDIR}/calc.${CATEXT}" ]; then \
		    echo "cannot uninstall ${T}${CATDIR}/calc.${CATEXT}"; \
		else \
		    echo "uninstalled ${T}${CATDIR}/calc.${CATEXT}"; \
		fi; \
	    fi; \
	fi
	-${Q} if [ -z "${MANDIR}" ]; then \
	    ${TRUE}; \
	else \
	    if [ -f "${T}${MANDIR}/calc.${MANEXT}" ]; then \
		${RM} -f "${T}${MANDIR}/calc.${MANEXT}"; \
		if [ -f "${T}${MANDIR}/calc.${MANEXT}" ]; then \
		    echo "cannot uninstall ${T}${MANDIR}/calc.${MANEXT}"; \
		else \
		    echo "uninstalled ${T}${MANDIR}/calc.${MANEXT}"; \
		fi; \
	    fi; \
	fi
	-${Q} for i in ${BUILD_H_SRC} ${LIB_H_SRC} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    if [ -f "${T}${CALC_INCDIR}/$$i" ]; then \
		${RM} -f "${T}${CALC_INCDIR}/$$i"; \
		if [ -f "${T}${CALC_INCDIR}/$$i" ]; then \
		    echo "cannot uninstall ${T}${CALC_INCDIR}/$$i"; \
		else \
		    echo "uninstalled ${T}${CALC_INCDIR}/$$i"; \
		fi; \
	    fi; \
	done
	-${Q} if [ -f "${T}${LIBDIR}/libcustcalc${LIB_EXT}" ]; then \
	    ${RM} -f "${T}${LIBDIR}/libcustcalc${LIB_EXT}"; \
	    if [ -f "${T}${LIBDIR}/libcustcalc${LIB_EXT}" ]; then \
		echo "cannot uninstall ${T}${LIBDIR}/libcustcalc${LIB_EXT}"; \
	    else \
		echo "uninstalled ${T}${LIBDIR}/libcustcalc${LIB_EXT}"; \
	    fi; \
	fi
	-${Q} if [ -f "${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}" ]; then \
	    ${RM} -f "${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}"; \
	    if [ -f "${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}" ]; then \
		echo \
		"cannot uninstall ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}";\
	    else \
		echo "uninstalled ${T}${LIBDIR}/libcustcalc${LIB_EXT_VERSION}";\
	    fi; \
	fi
	-${Q} if [ -f "${T}${LIBDIR}/libcalc${LIB_EXT}" ]; then \
	    ${RM} -f "${T}${LIBDIR}/libcalc${LIB_EXT}"; \
	    if [ -f "${T}${LIBDIR}/libcalc${LIB_EXT}" ]; then \
		echo "cannot uninstall ${T}${LIBDIR}/libcalc${LIB_EXT}"; \
	    else \
		echo "uninstalled ${T}${LIBDIR}/libcalc${LIB_EXT}"; \
	    fi; \
	fi
	-${Q} if [ -f "${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}" ]; then \
	    ${RM} -f "${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}"; \
	    if [ -f "${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}" ]; then \
		${ECHON} "cannot uninstall " \
		echo "${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}"; \
	    else \
		echo "uninstalled ${T}${LIBDIR}/libcalc${LIB_EXT_VERSION}"; \
	    fi; \
	fi
	-${Q} if [ -z "${T}" -o "/" = "${T}" ]; then \
	    if [ ! -z "${LDCONFIG}" ]; then \
		echo "running ${LDCONFIG}"; \
		${LDCONFIG} -v; \
		echo "finished ${LDCONFIG}"; \
	    fi; \
	fi
	-${Q} if [ -f "${T}${LIBDIR}/libcalc.a" ]; then \
	    ${RM} -f "${T}${LIBDIR}/libcalc.a"; \
	    if [ -f "${T}${LIBDIR}/libcalc.a" ]; then \
		echo "cannot uninstall ${T}${LIBDIR}/libcalc.a"; \
	    else \
		echo "uninstalled ${T}${LIBDIR}/libcalc.a"; \
	    fi; \
	fi
	${V} echo '=-=-=-=-= Invoking $@ rule for cscript =-=-=-=-='
	${Q} cd cscript; ${MAKE} -f Makefile uninstall
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for custom =-=-=-=-='
	${Q} cd custom; ${MAKE} -f Makefile uninstall
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cal =-=-=-=-='
	${Q} cd cal; ${MAKE} -f Makefile uninstall
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	${Q} cd help; ${MAKE} -f Makefile uninstall
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	-${Q} if [ -f "${T}${BINDIR}/calc-static${EXT}" ]; then \
	    ${RM} -f "${T}${BINDIR}/calc-static${EXT}"; \
	    if [ -f "${T}${BINDIR}/calc-static${EXT}" ]; then \
		echo "cannot uninstall ${T}${BINDIR}/calc-static${EXT}"; \
	    else \
		echo "uninstalled ${T}${BINDIR}/calc-static${EXT}"; \
	    fi; \
	fi
	-${Q} if [ -f "${T}${BINDIR}/calc${EXT}" ]; then \
	    ${RM} -f "${T}${BINDIR}/calc${EXT}"; \
	    if [ -f "${T}${BINDIR}/calc${EXT}" ]; then \
		echo "cannot uninstall ${T}${BINDIR}/calc${EXT}"; \
	    else \
		echo "uninstalled ${T}${BINDIR}/calc${EXT}"; \
	    fi; \
	fi
	-${Q} for i in ${CATDIR} ${MANDIR} ${SCRIPTDIR} \
		    ${CUSTOMINCDIR} ${CUSTOMHELPDIR} ${CUSTOMCALDIR} \
		    ${CALC_INCDIR} ${LIBDIR} ${INCDIR} ${BINDIR}; do \
	    if [ -d "${T}$$i" ]; then \
		${RMDIR} "${T}$$i" 2>/dev/null; \
		echo "cleaned up ${T}$$i"; \
	    fi; \
	done
	-${Q} if [ ! -z "${T}" ]; then \
	    if [ -d "${T}" ]; then \
		${RMDIR} "${T}" 2>/dev/null; \
		echo "cleaned up ${T}"; \
	    fi; \
	 fi
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

# unbak - remove any .bak files that may have been created
#
unbak:
	${Q} ${RM} -f -v Makefile.bak
	${Q} ${RM} -f -v custom/Makefile.bak

# splint - A tool for statically checking C programs
#
splint: #hsrc
	${SPLINT} ${SPLINT_OPTS} -DCALC_SRC -I. \
	    ${CALCSRC} ${LIBSRC} ${BUILD_C_SRC} ${UTIL_C_SRC}

# strip - for reducing the size of the binary files
#
strip:
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${Q} for i in ${UTIL_PROGS} ${SAMPLE_TARGETS} ${SAMPLE_STATIC_TARGETS} \
		 calc${EXT} calc-static${EXT} ${CALC_DYNAMIC_LIBS} \
		 ${CALC_STATIC_LIBS}; do \
	    if [ -s "$$i" -a -w "$$i" ]; then \
		${STRIP} "$$i"; \
		echo "stripped $$i"; \
	    fi; \
	done
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

# calc-symlink - setup symlinks from standard locations into the ${T} tree
#
calc-symlink:
	${Q}if [ -z "${T}" ]; then \
	    echo "cannot use $@ make rule when T make var is empty" 1>&2; \
	    echo "aborting" 1>&2; \
	    exit 1; \
	fi
	-${Q} for i in	${BINDIR}/calc${EXT} \
			${BINDIR}/calc-static${EXT} \
			${SCRIPTDIR} \
			${LIBDIR}/libcalc${LIB_EXT_VERSION} \
			${LIBDIR}/libcustcalc${LIB_EXT_VERSION} \
			${MANDIR}/calc.${MANEXT} \
			${CALC_SHAREDIR} \
			${CALC_INCDIR} \
			; do \
	    if [ -e "${T}$$i" ]; then \
		    if [ ! -L "$$i" -a "${T}$$i" -ef "$$i" ]; then \
			echo "ERROR: ${T}$$i is the same as $$i" 1>&2; \
		    else \
			if [ -e "$$i" ]; then \
			    echo ${RM} -f "$$i"; \
			    ${RM} -f "$$i"; \
			fi; \
			echo ${LN} -s "${T}$$i" "$$i"; \
			${LN} -s "${T}$$i" "$$i"; \
		    fi; \
	    else \
	        echo "Warning: not found: ${T}$$i" 1>&2; \
	    fi; \
	done
	-${Q} if [ -n "${CATDIR}" ]; then \
	    if [ -e "${T}${CATDIR}/calc.${CATEXT}" ]; then \
		if [ ! -L "${CATDIR}/calc.${CATEXT}" -a \
		     "${T}${CATDIR}/calc.${CATEXT}" -ef \
		     "${CATDIR}/calc.${CATEXT}" ]; then \
			${ECHON} "ERROR: ${T}${CATDIR}/calc.${CATEXT}" 2>&1; \
			echo "is the same as ${CATDIR}/calc.${CATEXT}" 1>&2; \
		else \
		    if [ -e "${CATDIR}/calc.${CATEXT}" ]; then \
			echo ${RM} -f "${CATDIR}/calc.${CATEXT}"; \
			${RM} -f "${CATDIR}/calc.${CATEXT}"; \
		    fi; \
		    echo ${LN} -s "${T}${CATDIR}/calc.${CATEXT}" \
		                  "${CATDIR}/calc.${CATEXT}"; \
		    ${LN} -s "${T}${CATDIR}/calc.${CATEXT}" \
		             "${CATDIR}/calc.${CATEXT}"; \
		fi; \
	    fi; \
	fi

# remove any symlinks that may have been created by calc-symlink
#
calc-unsymlink:
	-${Q} for i in	${BINDIR}/calc${EXT} \
			${BINDIR}/calc-static${EXT} \
			${SCRIPTDIR} \
			${LIBDIR}/libcalc${LIB_EXT_VERSION} \
			${LIBDIR}/libcustcalc${LIB_EXT_VERSION} \
			${MANDIR}/calc.${MANEXT} \
			${CALC_SHAREDIR} \
			${CALC_INCDIR} \
			; do \
	    if [ -L "$$i" ]; then \
		echo ${RM} -f "$$i"; \
		${RM} -f "$$i"; \
	    else \
	        echo "Warning: ignoring non-symlink: $$i" 1>&2; \
	    fi; \
	done
	-${Q} if [ -n "${CATDIR}" ]; then \
	    if [ -L "${CATDIR}/calc.${CATEXT}" ]; then \
		echo ${RM} -f "${CATDIR}/calc.${CATEXT}"; \
		${RM} -f "${CATDIR}/calc.${CATEXT}"; \
	    else \
	        echo "Warning: ignoring non-symlink: ${CATDIR}/calc.${CATEXT}" \
		      1>&2; \
	    fi; \
	fi

# reformat primary (non-built) source using clang-format
#
clang-format: ${C_SRC} ${H_SRC}
	${CLANG_FORMAT} -i --style=file:.clang-format ${C_SRC} ${H_SRC}
	${MAKE} -C custom clang-format

###
#
# make depend stuff
#
###

# DO NOT DELETE THIS LINE -- make depend depends on it.

addop.o: addop.c
addop.o: attribute.h
addop.o: banned.h
addop.o: block.h
addop.o: byteswap.h
addop.o: calc.h
addop.o: cmath.h
addop.o: config.h
addop.o: endian_calc.h
addop.o: errsym.h
addop.o: errtbl.h
addop.o: func.h
addop.o: hash.h
addop.o: have_ban_pragma.h
addop.o: int.h
addop.o: label.h
addop.o: len_bits.h
addop.o: nametype.h
addop.o: opcodes.h
addop.o: qmath.h
addop.o: sha1.h
addop.o: status.chk_c.h
addop.o: str.h
addop.o: symbol.h
addop.o: token.h
addop.o: value.h
addop.o: version.h
addop.o: zmath.h
addop.o: zrand.h
addop.o: zrandom.h
assocfunc.o: assocfunc.c
assocfunc.o: attribute.h
assocfunc.o: banned.h
assocfunc.o: block.h
assocfunc.o: byteswap.h
assocfunc.o: cmath.h
assocfunc.o: config.h
assocfunc.o: endian_calc.h
assocfunc.o: errsym.h
assocfunc.o: errtbl.h
assocfunc.o: hash.h
assocfunc.o: have_ban_pragma.h
assocfunc.o: int.h
assocfunc.o: len_bits.h
assocfunc.o: nametype.h
assocfunc.o: qmath.h
assocfunc.o: sha1.h
assocfunc.o: status.chk_c.h
assocfunc.o: str.h
assocfunc.o: value.h
assocfunc.o: version.h
assocfunc.o: zmath.h
assocfunc.o: zrand.h
assocfunc.o: zrandom.h
blkcpy.o: attribute.h
blkcpy.o: banned.h
blkcpy.o: blkcpy.c
blkcpy.o: blkcpy.h
blkcpy.o: block.h
blkcpy.o: byteswap.h
blkcpy.o: calc.h
blkcpy.o: cmath.h
blkcpy.o: config.h
blkcpy.o: endian_calc.h
blkcpy.o: errsym.h
blkcpy.o: errtbl.h
blkcpy.o: file.h
blkcpy.o: hash.h
blkcpy.o: have_ban_pragma.h
blkcpy.o: int.h
blkcpy.o: len_bits.h
blkcpy.o: nametype.h
blkcpy.o: qmath.h
blkcpy.o: sha1.h
blkcpy.o: status.chk_c.h
blkcpy.o: str.h
blkcpy.o: value.h
blkcpy.o: version.h
blkcpy.o: zmath.h
blkcpy.o: zrand.h
blkcpy.o: zrandom.h
block.o: attribute.h
block.o: banned.h
block.o: block.c
block.o: block.h
block.o: byteswap.h
block.o: cmath.h
block.o: config.h
block.o: endian_calc.h
block.o: errsym.h
block.o: errtbl.h
block.o: hash.h
block.o: have_ban_pragma.h
block.o: int.h
block.o: len_bits.h
block.o: nametype.h
block.o: qmath.h
block.o: sha1.h
block.o: status.chk_c.h
block.o: str.h
block.o: value.h
block.o: version.h
block.o: zmath.h
block.o: zrand.h
block.o: zrandom.h
byteswap.o: attribute.h
byteswap.o: banned.h
byteswap.o: byteswap.c
byteswap.o: byteswap.h
byteswap.o: cmath.h
byteswap.o: endian_calc.h
byteswap.o: errsym.h
byteswap.o: errtbl.h
byteswap.o: have_ban_pragma.h
byteswap.o: int.h
byteswap.o: len_bits.h
byteswap.o: qmath.h
byteswap.o: status.chk_c.h
byteswap.o: version.h
byteswap.o: zmath.h
calc.o: attribute.h
calc.o: banned.h
calc.o: block.h
calc.o: byteswap.h
calc.o: calc.c
calc.o: calc.h
calc.o: cmath.h
calc.o: conf.h
calc.o: config.h
calc.o: custom.h
calc.o: endian_calc.h
calc.o: errsym.h
calc.o: errtbl.h
calc.o: func.h
calc.o: hash.h
calc.o: have_ban_pragma.h
calc.o: have_strlcat.h
calc.o: have_strlcpy.h
calc.o: have_unused.h
calc.o: hist.h
calc.o: int.h
calc.o: label.h
calc.o: len_bits.h
calc.o: lib_calc.h
calc.o: nametype.h
calc.o: opcodes.h
calc.o: qmath.h
calc.o: sha1.h
calc.o: status.chk_c.h
calc.o: str.h
calc.o: strl.h
calc.o: symbol.h
calc.o: token.h
calc.o: value.h
calc.o: version.h
calc.o: zmath.h
calc.o: zrand.h
calc.o: zrandom.h
chk_c.o: banned.h
chk_c.o: chk_c.c
chk_c.o: have_ban_pragma.h
codegen.o: attribute.h
codegen.o: banned.h
codegen.o: block.h
codegen.o: byteswap.h
codegen.o: calc.h
codegen.o: cmath.h
codegen.o: codegen.c
codegen.o: conf.h
codegen.o: config.h
codegen.o: endian_calc.h
codegen.o: errsym.h
codegen.o: errtbl.h
codegen.o: func.h
codegen.o: hash.h
codegen.o: have_ban_pragma.h
codegen.o: have_strlcat.h
codegen.o: have_strlcpy.h
codegen.o: int.h
codegen.o: label.h
codegen.o: len_bits.h
codegen.o: lib_calc.h
codegen.o: nametype.h
codegen.o: opcodes.h
codegen.o: qmath.h
codegen.o: sha1.h
codegen.o: status.chk_c.h
codegen.o: str.h
codegen.o: strl.h
codegen.o: symbol.h
codegen.o: token.h
codegen.o: value.h
codegen.o: version.h
codegen.o: zmath.h
codegen.o: zrand.h
codegen.o: zrandom.h
comfunc.o: attribute.h
comfunc.o: banned.h
comfunc.o: byteswap.h
comfunc.o: cmath.h
comfunc.o: comfunc.c
comfunc.o: config.h
comfunc.o: endian_calc.h
comfunc.o: errsym.h
comfunc.o: errtbl.h
comfunc.o: have_ban_pragma.h
comfunc.o: int.h
comfunc.o: len_bits.h
comfunc.o: qmath.h
comfunc.o: status.chk_c.h
comfunc.o: version.h
comfunc.o: zmath.h
commath.o: attribute.h
commath.o: banned.h
commath.o: byteswap.h
commath.o: cmath.h
commath.o: commath.c
commath.o: endian_calc.h
commath.o: errsym.h
commath.o: errtbl.h
commath.o: have_ban_pragma.h
commath.o: int.h
commath.o: len_bits.h
commath.o: qmath.h
commath.o: status.chk_c.h
commath.o: version.h
commath.o: zmath.h
config.o: attribute.h
config.o: banned.h
config.o: block.h
config.o: byteswap.h
config.o: calc.h
config.o: cmath.h
config.o: config.c
config.o: config.h
config.o: custom.h
config.o: endian_calc.h
config.o: errsym.h
config.o: errtbl.h
config.o: hash.h
config.o: have_ban_pragma.h
config.o: have_strlcat.h
config.o: have_strlcpy.h
config.o: int.h
config.o: len_bits.h
config.o: nametype.h
config.o: qmath.h
config.o: sha1.h
config.o: status.chk_c.h
config.o: str.h
config.o: strl.h
config.o: token.h
config.o: value.h
config.o: version.h
config.o: zmath.h
config.o: zrand.h
config.o: zrandom.h
const.o: attribute.h
const.o: banned.h
const.o: block.h
const.o: byteswap.h
const.o: calc.h
const.o: cmath.h
const.o: config.h
const.o: const.c
const.o: endian_calc.h
const.o: errsym.h
const.o: errtbl.h
const.o: hash.h
const.o: have_ban_pragma.h
const.o: int.h
const.o: len_bits.h
const.o: nametype.h
const.o: qmath.h
const.o: sha1.h
const.o: status.chk_c.h
const.o: str.h
const.o: value.h
const.o: version.h
const.o: zmath.h
const.o: zrand.h
const.o: zrandom.h
custom.o: attribute.h
custom.o: banned.h
custom.o: block.h
custom.o: byteswap.h
custom.o: calc.h
custom.o: cmath.h
custom.o: config.h
custom.o: custom.c
custom.o: custom.h
custom.o: endian_calc.h
custom.o: errsym.h
custom.o: errtbl.h
custom.o: hash.h
custom.o: have_ban_pragma.h
custom.o: int.h
custom.o: len_bits.h
custom.o: nametype.h
custom.o: qmath.h
custom.o: sha1.h
custom.o: status.chk_c.h
custom.o: str.h
custom.o: value.h
custom.o: version.h
custom.o: zmath.h
custom.o: zrand.h
custom.o: zrandom.h
endian.o: banned.h
endian.o: endian.c
endian.o: have_ban_pragma.h
errtbl.o: attribute.h
errtbl.o: banned.h
errtbl.o: block.h
errtbl.o: byteswap.h
errtbl.o: cmath.h
errtbl.o: config.h
errtbl.o: endian_calc.h
errtbl.o: errsym.h
errtbl.o: errtbl.c
errtbl.o: errtbl.h
errtbl.o: func.h
errtbl.o: hash.h
errtbl.o: have_ban_pragma.h
errtbl.o: int.h
errtbl.o: label.h
errtbl.o: len_bits.h
errtbl.o: nametype.h
errtbl.o: qmath.h
errtbl.o: sha1.h
errtbl.o: status.chk_c.h
errtbl.o: str.h
errtbl.o: value.h
errtbl.o: version.h
errtbl.o: zmath.h
errtbl.o: zrand.h
errtbl.o: zrandom.h
file.o: attribute.h
file.o: banned.h
file.o: block.h
file.o: byteswap.h
file.o: calc.h
file.o: cmath.h
file.o: config.h
file.o: endian_calc.h
file.o: errsym.h
file.o: errtbl.h
file.o: file.c
file.o: file.h
file.o: hash.h
file.o: have_ban_pragma.h
file.o: have_strlcat.h
file.o: have_strlcpy.h
file.o: int.h
file.o: len_bits.h
file.o: nametype.h
file.o: qmath.h
file.o: sha1.h
file.o: status.chk_c.h
file.o: str.h
file.o: strl.h
file.o: value.h
file.o: version.h
file.o: zmath.h
file.o: zrand.h
file.o: zrandom.h
func.o: attribute.h
func.o: banned.h
func.o: blkcpy.h
func.o: block.h
func.o: byteswap.h
func.o: calc.h
func.o: cmath.h
func.o: config.h
func.o: custom.h
func.o: endian_calc.h
func.o: errsym.h
func.o: errtbl.h
func.o: file.h
func.o: func.c
func.o: func.h
func.o: hash.h
func.o: have_ban_pragma.h
func.o: have_rusage.h
func.o: have_strlcat.h
func.o: have_strlcpy.h
func.o: have_unused.h
func.o: int.h
func.o: label.h
func.o: len_bits.h
func.o: nametype.h
func.o: opcodes.h
func.o: prime.h
func.o: qmath.h
func.o: sha1.h
func.o: status.chk_c.h
func.o: str.h
func.o: strl.h
func.o: symbol.h
func.o: token.h
func.o: value.h
func.o: version.h
func.o: zmath.h
func.o: zrand.h
func.o: zrandom.h
hash.o: attribute.h
hash.o: banned.h
hash.o: block.h
hash.o: byteswap.h
hash.o: calc.h
hash.o: cmath.h
hash.o: config.h
hash.o: endian_calc.h
hash.o: errsym.h
hash.o: errtbl.h
hash.o: hash.c
hash.o: hash.h
hash.o: have_ban_pragma.h
hash.o: int.h
hash.o: len_bits.h
hash.o: nametype.h
hash.o: qmath.h
hash.o: sha1.h
hash.o: status.chk_c.h
hash.o: str.h
hash.o: value.h
hash.o: version.h
hash.o: zmath.h
hash.o: zrand.h
hash.o: zrandom.h
have_arc4random.o: banned.h
have_arc4random.o: have_arc4random.c
have_arc4random.o: have_ban_pragma.h
have_ban_pragma.o: banned.h
have_ban_pragma.o: have_ban_pragma.c
have_ban_pragma.o: have_ban_pragma.h
have_environ.o: banned.h
have_environ.o: have_ban_pragma.h
have_environ.o: have_environ.c
have_getpgid.o: banned.h
have_getpgid.o: have_ban_pragma.h
have_getpgid.o: have_getpgid.c
have_getsid.o: banned.h
have_getsid.o: have_ban_pragma.h
have_getsid.o: have_getsid.c
have_rusage.o: banned.h
have_rusage.o: have_ban_pragma.h
have_rusage.o: have_rusage.c
have_strlcat.o: banned.h
have_strlcat.o: have_ban_pragma.h
have_strlcat.o: have_strlcat.c
have_strlcpy.o: banned.h
have_strlcpy.o: have_ban_pragma.h
have_strlcpy.o: have_strlcpy.c
have_unused.o: banned.h
have_unused.o: have_ban_pragma.h
have_unused.o: have_unused.c
help.o: banned.h
help.o: block.h
help.o: byteswap.h
help.o: calc.h
help.o: cmath.h
help.o: conf.h
help.o: config.h
help.o: endian_calc.h
help.o: hash.h
help.o: have_ban_pragma.h
help.o: help.c
help.o: int.h
help.o: len_bits.h
help.o: lib_calc.h
help.o: nametype.h
help.o: qmath.h
help.o: sha1.h
help.o: status.chk_c.h
help.o: str.h
help.o: value.h
help.o: version.h
help.o: zmath.h
help.o: zrand.h
help.o: zrandom.h
hist.o: attribute.h
hist.o: banned.h
hist.o: block.h
hist.o: byteswap.h
hist.o: calc.h
hist.o: cmath.h
hist.o: config.h
hist.o: endian_calc.h
hist.o: errsym.h
hist.o: errtbl.h
hist.o: hash.h
hist.o: have_ban_pragma.h
hist.o: have_strlcat.h
hist.o: have_strlcpy.h
hist.o: have_unused.h
hist.o: hist.c
hist.o: hist.h
hist.o: int.h
hist.o: len_bits.h
hist.o: lib_calc.h
hist.o: nametype.h
hist.o: qmath.h
hist.o: sha1.h
hist.o: status.chk_c.h
hist.o: str.h
hist.o: strl.h
hist.o: value.h
hist.o: version.h
hist.o: zmath.h
hist.o: zrand.h
hist.o: zrandom.h
input.o: attribute.h
input.o: banned.h
input.o: block.h
input.o: byteswap.h
input.o: calc.h
input.o: cmath.h
input.o: conf.h
input.o: config.h
input.o: endian_calc.h
input.o: errsym.h
input.o: errtbl.h
input.o: hash.h
input.o: have_ban_pragma.h
input.o: have_strlcat.h
input.o: have_strlcpy.h
input.o: hist.h
input.o: input.c
input.o: int.h
input.o: len_bits.h
input.o: nametype.h
input.o: qmath.h
input.o: sha1.h
input.o: status.chk_c.h
input.o: str.h
input.o: strl.h
input.o: value.h
input.o: version.h
input.o: zmath.h
input.o: zrand.h
input.o: zrandom.h
jump.o: banned.h
jump.o: have_ban_pragma.h
jump.o: jump.c
jump.o: jump.h
label.o: banned.h
label.o: block.h
label.o: byteswap.h
label.o: calc.h
label.o: cmath.h
label.o: config.h
label.o: endian_calc.h
label.o: func.h
label.o: hash.h
label.o: have_ban_pragma.h
label.o: int.h
label.o: label.c
label.o: label.h
label.o: len_bits.h
label.o: nametype.h
label.o: opcodes.h
label.o: qmath.h
label.o: sha1.h
label.o: status.chk_c.h
label.o: str.h
label.o: token.h
label.o: value.h
label.o: version.h
label.o: zmath.h
label.o: zrand.h
label.o: zrandom.h
len_bits.o: banned.h
len_bits.o: endian_calc.h
len_bits.o: have_ban_pragma.h
len_bits.o: len_bits.c
lib_calc.o: attribute.h
lib_calc.o: banned.h
lib_calc.o: block.h
lib_calc.o: byteswap.h
lib_calc.o: calc.h
lib_calc.o: cmath.h
lib_calc.o: conf.h
lib_calc.o: config.h
lib_calc.o: custom.h
lib_calc.o: endian_calc.h
lib_calc.o: errsym.h
lib_calc.o: errtbl.h
lib_calc.o: func.h
lib_calc.o: hash.h
lib_calc.o: have_ban_pragma.h
lib_calc.o: have_strlcat.h
lib_calc.o: have_strlcpy.h
lib_calc.o: int.h
lib_calc.o: label.h
lib_calc.o: len_bits.h
lib_calc.o: lib_calc.c
lib_calc.o: lib_calc.h
lib_calc.o: nametype.h
lib_calc.o: qmath.h
lib_calc.o: sha1.h
lib_calc.o: status.chk_c.h
lib_calc.o: str.h
lib_calc.o: strl.h
lib_calc.o: symbol.h
lib_calc.o: token.h
lib_calc.o: value.h
lib_calc.o: version.h
lib_calc.o: zmath.h
lib_calc.o: zrand.h
lib_calc.o: zrandom.h
lib_util.o: attribute.h
lib_util.o: banned.h
lib_util.o: byteswap.h
lib_util.o: endian_calc.h
lib_util.o: errsym.h
lib_util.o: errtbl.h
lib_util.o: have_ban_pragma.h
lib_util.o: int.h
lib_util.o: len_bits.h
lib_util.o: lib_util.c
lib_util.o: lib_util.h
lib_util.o: status.chk_c.h
lib_util.o: version.h
lib_util.o: zmath.h
listfunc.o: attribute.h
listfunc.o: banned.h
listfunc.o: block.h
listfunc.o: byteswap.h
listfunc.o: cmath.h
listfunc.o: config.h
listfunc.o: endian_calc.h
listfunc.o: errsym.h
listfunc.o: errtbl.h
listfunc.o: hash.h
listfunc.o: have_ban_pragma.h
listfunc.o: int.h
listfunc.o: len_bits.h
listfunc.o: listfunc.c
listfunc.o: nametype.h
listfunc.o: qmath.h
listfunc.o: sha1.h
listfunc.o: status.chk_c.h
listfunc.o: str.h
listfunc.o: value.h
listfunc.o: version.h
listfunc.o: zmath.h
listfunc.o: zrand.h
listfunc.o: zrandom.h
matfunc.o: attribute.h
matfunc.o: banned.h
matfunc.o: block.h
matfunc.o: byteswap.h
matfunc.o: cmath.h
matfunc.o: config.h
matfunc.o: endian_calc.h
matfunc.o: errsym.h
matfunc.o: errtbl.h
matfunc.o: hash.h
matfunc.o: have_ban_pragma.h
matfunc.o: have_unused.h
matfunc.o: int.h
matfunc.o: len_bits.h
matfunc.o: matfunc.c
matfunc.o: nametype.h
matfunc.o: qmath.h
matfunc.o: sha1.h
matfunc.o: status.chk_c.h
matfunc.o: str.h
matfunc.o: value.h
matfunc.o: version.h
matfunc.o: zmath.h
matfunc.o: zrand.h
matfunc.o: zrandom.h
math_error.o: banned.h
math_error.o: block.h
math_error.o: byteswap.h
math_error.o: calc.h
math_error.o: cmath.h
math_error.o: config.h
math_error.o: endian_calc.h
math_error.o: hash.h
math_error.o: have_ban_pragma.h
math_error.o: int.h
math_error.o: len_bits.h
math_error.o: lib_calc.h
math_error.o: math_error.c
math_error.o: nametype.h
math_error.o: qmath.h
math_error.o: sha1.h
math_error.o: status.chk_c.h
math_error.o: str.h
math_error.o: value.h
math_error.o: version.h
math_error.o: zmath.h
math_error.o: zrand.h
math_error.o: zrandom.h
obj.o: attribute.h
obj.o: banned.h
obj.o: block.h
obj.o: byteswap.h
obj.o: calc.h
obj.o: cmath.h
obj.o: config.h
obj.o: endian_calc.h
obj.o: errsym.h
obj.o: errtbl.h
obj.o: func.h
obj.o: hash.h
obj.o: have_ban_pragma.h
obj.o: have_strlcat.h
obj.o: have_strlcpy.h
obj.o: int.h
obj.o: label.h
obj.o: len_bits.h
obj.o: nametype.h
obj.o: obj.c
obj.o: opcodes.h
obj.o: qmath.h
obj.o: sha1.h
obj.o: status.chk_c.h
obj.o: str.h
obj.o: strl.h
obj.o: symbol.h
obj.o: value.h
obj.o: version.h
obj.o: zmath.h
obj.o: zrand.h
obj.o: zrandom.h
opcodes.o: attribute.h
opcodes.o: banned.h
opcodes.o: block.h
opcodes.o: byteswap.h
opcodes.o: calc.h
opcodes.o: cmath.h
opcodes.o: config.h
opcodes.o: custom.h
opcodes.o: endian_calc.h
opcodes.o: errsym.h
opcodes.o: errtbl.h
opcodes.o: file.h
opcodes.o: func.h
opcodes.o: hash.h
opcodes.o: have_ban_pragma.h
opcodes.o: have_unused.h
opcodes.o: hist.h
opcodes.o: int.h
opcodes.o: label.h
opcodes.o: len_bits.h
opcodes.o: lib_calc.h
opcodes.o: nametype.h
opcodes.o: opcodes.c
opcodes.o: opcodes.h
opcodes.o: qmath.h
opcodes.o: sha1.h
opcodes.o: status.chk_c.h
opcodes.o: str.h
opcodes.o: symbol.h
opcodes.o: value.h
opcodes.o: version.h
opcodes.o: zmath.h
opcodes.o: zrand.h
opcodes.o: zrandom.h
pix.o: banned.h
pix.o: byteswap.h
pix.o: endian_calc.h
pix.o: have_ban_pragma.h
pix.o: int.h
pix.o: len_bits.h
pix.o: pix.c
pix.o: prime.h
pix.o: qmath.h
pix.o: status.chk_c.h
pix.o: version.h
pix.o: zmath.h
poly.o: banned.h
poly.o: block.h
poly.o: byteswap.h
poly.o: cmath.h
poly.o: config.h
poly.o: endian_calc.h
poly.o: hash.h
poly.o: have_ban_pragma.h
poly.o: int.h
poly.o: len_bits.h
poly.o: nametype.h
poly.o: poly.c
poly.o: qmath.h
poly.o: sha1.h
poly.o: status.chk_c.h
poly.o: str.h
poly.o: value.h
poly.o: version.h
poly.o: zmath.h
poly.o: zrand.h
poly.o: zrandom.h
prime.o: banned.h
prime.o: byteswap.h
prime.o: endian_calc.h
prime.o: have_ban_pragma.h
prime.o: int.h
prime.o: jump.h
prime.o: len_bits.h
prime.o: prime.c
prime.o: prime.h
prime.o: qmath.h
prime.o: status.chk_c.h
prime.o: version.h
prime.o: zmath.h
qfunc.o: attribute.h
qfunc.o: banned.h
qfunc.o: byteswap.h
qfunc.o: config.h
qfunc.o: endian_calc.h
qfunc.o: errsym.h
qfunc.o: errtbl.h
qfunc.o: have_ban_pragma.h
qfunc.o: int.h
qfunc.o: len_bits.h
qfunc.o: prime.h
qfunc.o: qfunc.c
qfunc.o: qmath.h
qfunc.o: status.chk_c.h
qfunc.o: version.h
qfunc.o: zmath.h
qio.o: attribute.h
qio.o: banned.h
qio.o: byteswap.h
qio.o: config.h
qio.o: endian_calc.h
qio.o: errsym.h
qio.o: errtbl.h
qio.o: have_ban_pragma.h
qio.o: have_unused.h
qio.o: int.h
qio.o: len_bits.h
qio.o: qio.c
qio.o: qmath.h
qio.o: status.chk_c.h
qio.o: version.h
qio.o: zmath.h
qmath.o: attribute.h
qmath.o: banned.h
qmath.o: byteswap.h
qmath.o: config.h
qmath.o: endian_calc.h
qmath.o: errsym.h
qmath.o: errtbl.h
qmath.o: have_ban_pragma.h
qmath.o: int.h
qmath.o: len_bits.h
qmath.o: qmath.c
qmath.o: qmath.h
qmath.o: status.chk_c.h
qmath.o: version.h
qmath.o: zmath.h
qmod.o: attribute.h
qmod.o: banned.h
qmod.o: byteswap.h
qmod.o: config.h
qmod.o: endian_calc.h
qmod.o: errsym.h
qmod.o: errtbl.h
qmod.o: have_ban_pragma.h
qmod.o: int.h
qmod.o: len_bits.h
qmod.o: qmath.h
qmod.o: qmod.c
qmod.o: status.chk_c.h
qmod.o: version.h
qmod.o: zmath.h
qtrans.o: attribute.h
qtrans.o: banned.h
qtrans.o: byteswap.h
qtrans.o: config.h
qtrans.o: endian_calc.h
qtrans.o: errsym.h
qtrans.o: errtbl.h
qtrans.o: have_ban_pragma.h
qtrans.o: int.h
qtrans.o: len_bits.h
qtrans.o: qmath.h
qtrans.o: qtrans.c
qtrans.o: status.chk_c.h
qtrans.o: version.h
qtrans.o: zmath.h
quickhash.o: attribute.h
quickhash.o: banned.h
quickhash.o: block.h
quickhash.o: byteswap.h
quickhash.o: cmath.h
quickhash.o: config.h
quickhash.o: endian_calc.h
quickhash.o: errsym.h
quickhash.o: errtbl.h
quickhash.o: hash.h
quickhash.o: have_ban_pragma.h
quickhash.o: int.h
quickhash.o: len_bits.h
quickhash.o: nametype.h
quickhash.o: qmath.h
quickhash.o: quickhash.c
quickhash.o: sha1.h
quickhash.o: status.chk_c.h
quickhash.o: str.h
quickhash.o: value.h
quickhash.o: version.h
quickhash.o: zmath.h
quickhash.o: zrand.h
quickhash.o: zrandom.h
sample_many.o: attribute.h
sample_many.o: banned.h
sample_many.o: block.h
sample_many.o: byteswap.h
sample_many.o: calc.h
sample_many.o: cmath.h
sample_many.o: config.h
sample_many.o: endian_calc.h
sample_many.o: errsym.h
sample_many.o: errtbl.h
sample_many.o: hash.h
sample_many.o: have_ban_pragma.h
sample_many.o: int.h
sample_many.o: len_bits.h
sample_many.o: lib_util.h
sample_many.o: nametype.h
sample_many.o: qmath.h
sample_many.o: sample_many.c
sample_many.o: sha1.h
sample_many.o: status.chk_c.h
sample_many.o: str.h
sample_many.o: value.h
sample_many.o: version.h
sample_many.o: zmath.h
sample_many.o: zrand.h
sample_many.o: zrandom.h
sample_rand.o: attribute.h
sample_rand.o: banned.h
sample_rand.o: block.h
sample_rand.o: byteswap.h
sample_rand.o: calc.h
sample_rand.o: cmath.h
sample_rand.o: config.h
sample_rand.o: endian_calc.h
sample_rand.o: errsym.h
sample_rand.o: errtbl.h
sample_rand.o: hash.h
sample_rand.o: have_ban_pragma.h
sample_rand.o: int.h
sample_rand.o: len_bits.h
sample_rand.o: lib_util.h
sample_rand.o: nametype.h
sample_rand.o: qmath.h
sample_rand.o: sample_rand.c
sample_rand.o: sha1.h
sample_rand.o: status.chk_c.h
sample_rand.o: str.h
sample_rand.o: value.h
sample_rand.o: version.h
sample_rand.o: zmath.h
sample_rand.o: zrand.h
sample_rand.o: zrandom.h
seed.o: banned.h
seed.o: byteswap.h
seed.o: endian_calc.h
seed.o: errsym.h
seed.o: errtbl.h
seed.o: have_arc4random.h
seed.o: have_ban_pragma.h
seed.o: have_environ.h
seed.o: have_getpgid.h
seed.o: have_rusage.h
seed.o: have_urandom.h
seed.o: int.h
seed.o: len_bits.h
seed.o: qmath.h
seed.o: seed.c
seed.o: status.chk_c.h
seed.o: version.h
seed.o: zmath.h
sha1.o: attribute.h
sha1.o: banned.h
sha1.o: block.h
sha1.o: byteswap.h
sha1.o: cmath.h
sha1.o: config.h
sha1.o: endian_calc.h
sha1.o: errsym.h
sha1.o: errtbl.h
sha1.o: hash.h
sha1.o: have_ban_pragma.h
sha1.o: int.h
sha1.o: len_bits.h
sha1.o: nametype.h
sha1.o: qmath.h
sha1.o: sha1.c
sha1.o: sha1.h
sha1.o: status.chk_c.h
sha1.o: str.h
sha1.o: value.h
sha1.o: version.h
sha1.o: zmath.h
sha1.o: zrand.h
sha1.o: zrandom.h
size.o: attribute.h
size.o: banned.h
size.o: block.h
size.o: byteswap.h
size.o: cmath.h
size.o: config.h
size.o: endian_calc.h
size.o: errsym.h
size.o: errtbl.h
size.o: hash.h
size.o: have_ban_pragma.h
size.o: int.h
size.o: len_bits.h
size.o: nametype.h
size.o: qmath.h
size.o: sha1.h
size.o: size.c
size.o: status.chk_c.h
size.o: str.h
size.o: value.h
size.o: version.h
size.o: zmath.h
size.o: zrand.h
size.o: zrandom.h
str.o: attribute.h
str.o: banned.h
str.o: block.h
str.o: byteswap.h
str.o: calc.h
str.o: cmath.h
str.o: config.h
str.o: endian_calc.h
str.o: errsym.h
str.o: errtbl.h
str.o: hash.h
str.o: have_ban_pragma.h
str.o: have_strlcat.h
str.o: have_strlcpy.h
str.o: int.h
str.o: len_bits.h
str.o: nametype.h
str.o: qmath.h
str.o: sha1.h
str.o: status.chk_c.h
str.o: str.c
str.o: str.h
str.o: strl.h
str.o: value.h
str.o: version.h
str.o: zmath.h
str.o: zrand.h
str.o: zrandom.h
strl.o: banned.h
strl.o: have_ban_pragma.h
strl.o: have_strlcat.h
strl.o: have_strlcpy.h
strl.o: strl.c
strl.o: strl.h
symbol.o: attribute.h
symbol.o: banned.h
symbol.o: block.h
symbol.o: byteswap.h
symbol.o: calc.h
symbol.o: cmath.h
symbol.o: config.h
symbol.o: endian_calc.h
symbol.o: errsym.h
symbol.o: errtbl.h
symbol.o: func.h
symbol.o: hash.h
symbol.o: have_ban_pragma.h
symbol.o: int.h
symbol.o: label.h
symbol.o: len_bits.h
symbol.o: nametype.h
symbol.o: opcodes.h
symbol.o: qmath.h
symbol.o: sha1.h
symbol.o: status.chk_c.h
symbol.o: str.h
symbol.o: symbol.c
symbol.o: symbol.h
symbol.o: token.h
symbol.o: value.h
symbol.o: version.h
symbol.o: zmath.h
symbol.o: zrand.h
symbol.o: zrandom.h
token.o: attribute.h
token.o: banned.h
token.o: block.h
token.o: byteswap.h
token.o: calc.h
token.o: cmath.h
token.o: config.h
token.o: endian_calc.h
token.o: errsym.h
token.o: errtbl.h
token.o: hash.h
token.o: have_ban_pragma.h
token.o: int.h
token.o: len_bits.h
token.o: lib_calc.h
token.o: nametype.h
token.o: qmath.h
token.o: sha1.h
token.o: status.chk_c.h
token.o: str.h
token.o: token.c
token.o: token.h
token.o: value.h
token.o: version.h
token.o: zmath.h
token.o: zrand.h
token.o: zrandom.h
value.o: attribute.h
value.o: banned.h
value.o: block.h
value.o: byteswap.h
value.o: calc.h
value.o: cmath.h
value.o: config.h
value.o: endian_calc.h
value.o: errsym.h
value.o: errtbl.h
value.o: file.h
value.o: func.h
value.o: hash.h
value.o: have_ban_pragma.h
value.o: int.h
value.o: label.h
value.o: len_bits.h
value.o: nametype.h
value.o: opcodes.h
value.o: qmath.h
value.o: sha1.h
value.o: status.chk_c.h
value.o: str.h
value.o: symbol.h
value.o: value.c
value.o: value.h
value.o: version.h
value.o: zmath.h
value.o: zrand.h
value.o: zrandom.h
version.o: banned.h
version.o: block.h
version.o: byteswap.h
version.o: calc.h
version.o: cmath.h
version.o: config.h
version.o: endian_calc.h
version.o: hash.h
version.o: have_ban_pragma.h
version.o: have_strlcat.h
version.o: have_strlcpy.h
version.o: int.h
version.o: len_bits.h
version.o: nametype.h
version.o: qmath.h
version.o: sha1.h
version.o: status.chk_c.h
version.o: str.h
version.o: strl.h
version.o: value.h
version.o: version.c
version.o: version.h
version.o: zmath.h
version.o: zrand.h
version.o: zrandom.h
zfunc.o: attribute.h
zfunc.o: banned.h
zfunc.o: byteswap.h
zfunc.o: endian_calc.h
zfunc.o: errsym.h
zfunc.o: errtbl.h
zfunc.o: have_ban_pragma.h
zfunc.o: int.h
zfunc.o: len_bits.h
zfunc.o: status.chk_c.h
zfunc.o: version.h
zfunc.o: zfunc.c
zfunc.o: zmath.h
zio.o: attribute.h
zio.o: banned.h
zio.o: byteswap.h
zio.o: config.h
zio.o: endian_calc.h
zio.o: errsym.h
zio.o: errtbl.h
zio.o: have_ban_pragma.h
zio.o: int.h
zio.o: len_bits.h
zio.o: qmath.h
zio.o: status.chk_c.h
zio.o: version.h
zio.o: zio.c
zio.o: zmath.h
zmath.o: attribute.h
zmath.o: banned.h
zmath.o: byteswap.h
zmath.o: endian_calc.h
zmath.o: errsym.h
zmath.o: errtbl.h
zmath.o: have_ban_pragma.h
zmath.o: int.h
zmath.o: len_bits.h
zmath.o: status.chk_c.h
zmath.o: version.h
zmath.o: zmath.c
zmath.o: zmath.h
zmod.o: attribute.h
zmod.o: banned.h
zmod.o: byteswap.h
zmod.o: config.h
zmod.o: endian_calc.h
zmod.o: errsym.h
zmod.o: errtbl.h
zmod.o: have_ban_pragma.h
zmod.o: int.h
zmod.o: len_bits.h
zmod.o: qmath.h
zmod.o: status.chk_c.h
zmod.o: version.h
zmod.o: zmath.h
zmod.o: zmod.c
zmul.o: attribute.h
zmul.o: banned.h
zmul.o: byteswap.h
zmul.o: config.h
zmul.o: endian_calc.h
zmul.o: errsym.h
zmul.o: errtbl.h
zmul.o: have_ban_pragma.h
zmul.o: int.h
zmul.o: len_bits.h
zmul.o: qmath.h
zmul.o: status.chk_c.h
zmul.o: version.h
zmul.o: zmath.h
zmul.o: zmul.c
zprime.o: attribute.h
zprime.o: banned.h
zprime.o: block.h
zprime.o: byteswap.h
zprime.o: cmath.h
zprime.o: config.h
zprime.o: endian_calc.h
zprime.o: errsym.h
zprime.o: errtbl.h
zprime.o: hash.h
zprime.o: have_ban_pragma.h
zprime.o: int.h
zprime.o: jump.h
zprime.o: len_bits.h
zprime.o: nametype.h
zprime.o: prime.h
zprime.o: qmath.h
zprime.o: sha1.h
zprime.o: status.chk_c.h
zprime.o: str.h
zprime.o: value.h
zprime.o: version.h
zprime.o: zmath.h
zprime.o: zprime.c
zprime.o: zrand.h
zprime.o: zrandom.h
zrand.o: attribute.h
zrand.o: banned.h
zrand.o: block.h
zrand.o: byteswap.h
zrand.o: cmath.h
zrand.o: config.h
zrand.o: endian_calc.h
zrand.o: errsym.h
zrand.o: errtbl.h
zrand.o: hash.h
zrand.o: have_ban_pragma.h
zrand.o: have_unused.h
zrand.o: int.h
zrand.o: len_bits.h
zrand.o: nametype.h
zrand.o: qmath.h
zrand.o: sha1.h
zrand.o: status.chk_c.h
zrand.o: str.h
zrand.o: value.h
zrand.o: version.h
zrand.o: zmath.h
zrand.o: zrand.c
zrand.o: zrand.h
zrand.o: zrandom.h
zrandom.o: attribute.h
zrandom.o: banned.h
zrandom.o: block.h
zrandom.o: byteswap.h
zrandom.o: cmath.h
zrandom.o: config.h
zrandom.o: endian_calc.h
zrandom.o: errsym.h
zrandom.o: errtbl.h
zrandom.o: hash.h
zrandom.o: have_ban_pragma.h
zrandom.o: have_unused.h
zrandom.o: int.h
zrandom.o: len_bits.h
zrandom.o: nametype.h
zrandom.o: qmath.h
zrandom.o: sha1.h
zrandom.o: status.chk_c.h
zrandom.o: str.h
zrandom.o: value.h
zrandom.o: version.h
zrandom.o: zmath.h
zrandom.o: zrand.h
zrandom.o: zrandom.c
zrandom.o: zrandom.h
