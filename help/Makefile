#
# help - makefile for calc help files
#
# Copyright (c) 1994 David I. Bell and Landon Curt Noll
# Permission is granted to use, distribute, or modify this source,
# provided that this copyright notice remains intact.
#
# Arbitrary precision calculator.
#
# calculator by David I. Bell
# makefile by Landon Curt Noll

# required vars
#
SHELL= /bin/sh
MAKE_FILE = Makefile

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
TOPDIR= /usr/local/lib
#TOPDIR= /usr/lib
#TOPDIR= /usr/libdata

LIBDIR= ${TOPDIR}/calc
HELPDIR= ${LIBDIR}/help

# Makefile debug
#
# Q=@	do not echo internal makefile actions (quiet mode)
# Q=	echo internal makefile actions (debug / verbose mode)
#
#Q=
Q=@

# standard tools
#
NATIVE_CC= cc
NATIVE_CFLAGS=
SED= sed
SORT= sort
FMT= fmt
CMP= cmp
CAT= cat

# Standard and Builtin help files
#
STD_HELP_FILES_1= intro overview help
STD_HELP_FILES_2= assoc

BLT_HELP_FILES_3= builtin

STD_HELP_FILES_4= command config custom define environment expression

BLT_HELP_FILES_5= errorcodes

STD_HELP_FILES_6= file history interrupt list mat

# beacuse obj is built special (due to confusion with it as
# a symlink for some built environments, we treat obj specially
# and call it obj.file
#
SPECIAL_HELP_7= obj.file

STD_HELP_FILES_8= operator statement

BLT_HELP_FILES_9= stdlib

STD_HELP_FILES_10= types usage unexpected variable

BLT_HELP_FILES_11= altbind bindings custom_cal libcalc new_custom stdlib

STD_HELP_FILES_12= archive

BLT_HELP_FILES_13= bugs changes

STD_HELP_FILES_14= contrib credit wishlist todo

# These files are used in the following order to construct full
#
FULL_HELP_FILES= ${STD_HELP_FILES_1} ${STD_HELP_FILES_2} \
		${BLT_HELP_FILES_3} ${STD_HELP_FILES_4} \
		${BLT_HELP_FILES_5} ${STD_HELP_FILES_6} \
		${SPECIAL_HELP_7} ${STD_HELP_FILES_8} \
		${BLT_HELP_FILES_9} ${STD_HELP_FILES_10} \
		${BLT_HELP_FILES_11} ${STD_HELP_FILES_12} \
		${BLT_HELP_FILES_13} ${STD_HELP_FILES_14}

# These full files are those who are not built or constrcuted
#
STD_HELP_FILES= ${STD_HELP_FILES_1} ${STD_HELP_FILES_2} \
		${STD_HELP_FILES_4} ${STD_HELP_FILES_6} \
		${STD_HELP_FILES_8} ${STD_HELP_FILES_10} \
		${STD_HELP_FILES_12} ${STD_HELP_FILES_14}

# These full files are those who are built by this Makefile
#
# Note that ${SPECIAL_HELP_7} is not included in this list
# because of problems with its name.
#
BLT_HELP_FILES= ${BLT_HELP_FILES_3} ${BLT_HELP_FILES_5} \
		${BLT_HELP_FILES_9} \
		${BLT_HELP_FILES_11} ${BLT_HELP_FILES_13}

# This list is prodiced by the detaillist rule when no WARNINGS are detected.
#
DETAIL_HELP= abs access acos acosh acot acoth acsc acsch address agd append \
	appr arg arrow asec asech asin asinh assign atan atan2 atanh avg \
	base bit blk blkcpy blkfree blocks bround btrunc calclevel ceil \
	cfappr cfsim char cmdbuf cmp comb conj cos cosh cot coth count cp \
	csc csch ctime delete den dereference det digit digits dp epsilon \
	errcount errmax errno error eval exp fact factor fclose fcnt feof \
	ferror fflush fgetc fgetfield fgetline fgets fgetstr fib files \
	floor fopen forall fprintf fputc fputs fputstr frac free freeglobals \
	freeredc freestatics frem freopen fscan fscanf fseek fsize ftell gcd \
	gcdrem gd getenv hash head highbit hmean hnrmod hypot ilog ilog10 \
	ilog2 im inputlevel insert int inverse iroot isassoc isatty isblk \
	isconfig isdefined iserror iseven isfile ishash isident isint islist \
	ismat ismult isnull isnum isobj isobjtype isodd isprime isptr isqrt \
	isrand israndom isreal isrel issimple issq isstr istype jacobi join \
	lcm lcmfact lfactor ln lowbit ltol makelist matdim matfill matmax \
	matmin matsum mattrace mattrans max md5 memsize meq min minv mmin \
	mne mod modify name near newerror nextcand nextprime norm null \
	num oldvalue ord param perm pfact pi pix places pmod polar poly \
	pop popcnt power prevcand prevprime printf prompt protect ptest \
	push putenv quo quomod rand randbit random randombit randperm rcin \
	rcmul rcout rcpow rcsq re remove reverse rewind rm root round rsearch \
	runtime saveval scale scan scanf search sec sech seed segment select \
	sgn sha sha1 sin sinh size sizeof sort sqrt srand srandom ssq str \
	strcat strerror strlen strpos strprintf strscan strscanf substr \
	sum swap system tail tan tanh test time trunc xor

# This list is of files that are clones of DETAIL_HELP files.  They are
# built from DETAIL_HELP files.
#
DETAIL_CLONE= copy

# Singular files
#
# These files are copies of their plural form.
#
PLURAL_FILES= bindings bugs changes errorcodes types
SINGULAR_FILES= binding bug change errorcode type

# These files are found (but not built) in the distribution
#
DISTLIST= ${STD_HELP_FILES} ${DETAIL_HELP} ${MAKE_FILE} \
	obj.file builtin.top builtin.end funclist.sed \
	errorcodes.hdr errorcodes.sed

all: ${FULL_HELP_FILES} full ${DETAIL_HELP} ${DETAIL_CLONE} \
     ${SINGULAR_FILES} calc .all

# used by the upper level Makefile to determine of we have done all
#
# NOTE: Due to bogus shells found on one common system we must have
#       an non-emoty else clause for every if condition.  *sigh*
#
.all:
	rm -f .all
	touch .all

bindings: ../lib/bindings
	rm -f $@
	cp ../lib/bindings $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

altbind: ../lib/altbind
	rm -f $@
	cp ../lib/altbind $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

stdlib: ../lib/README
	rm -f $@
	cp ../lib/README $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

changes: ../CHANGES
	rm -f $@
	cp ../CHANGES $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

libcalc: ../LIBRARY
	rm -f $@
	${SED} -e 's:$${LIBDIR}:${LIBDIR}:g' < ../LIBRARY > $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

bugs: ../BUGS
	rm -f $@
	cp ../BUGS $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

errorcodes: ../calcerr.h errorcodes.hdr errorcodes.sed
	rm -f $@
	${CAT} errorcodes.hdr > $@
	${SED} -n -f errorcodes.sed < ../calcerr.h >> $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

calc: usage
	rm -f $@
	cp usage $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

custom_cal: ../custom/CUSTOM_CAL
	rm -f $@
	cp ../custom/CUSTOM_CAL $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

new_custom: ../custom/HOW_TO_ADD
	rm -f $@
	cp ../custom/HOW_TO_ADD $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

copy: blkcpy
	rm -f $@
	cp blkcpy $@
	chmod 0444 $@
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

full: ${FULL_HELP_FILES} ${MAKE_FILE}
	${Q}echo "forming full"
	-${Q}rm -f $@
	-${Q}for i in ${FULL_HELP_FILES}; do \
	    if [ Xintro != X"$$i" ]; then \
		echo ""; \
	    else \
		true; \
	    fi; \
	    if [ Xobj.file = X"$$i" ]; then \
		j=obj; \
	    else \
		j=$$i; \
	    fi; \
	    echo "*************"; \
	    echo "* $$j"; \
	    echo "*************"; \
	    echo ""; \
	    cat $$i; \
	done > $@
	${Q}echo "full formed"
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

# Singular files are the same files as their plural form.
#
${SINGULAR_FILES}: ${PLURAL_FILES}
	${Q}for i in ${SINGULAR_FILES}; do \
	    echo "rm -f $${i}"; \
	    rm -f $${i}; \
	    echo "cp $${i}s $${i}"; \
	    cp $${i}s $${i}; \
	done
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/SINGULAR_FILES =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

# Form the builtin file
#
# We ave a "chicken-and-egg" problem.  We want the builtn help file to
# accurately reflect the function list.  It would be nice if we could
# just execute calc show builtin, but calc may not have been built or
# buildable at this point.  The hack-a-round used is to convert ../func.c
# into a standalone program that generates a suitable function list
# that is standwiched between the top and bottom builtin help text.
#
# We form funclist.c by sedding out unwanted stuff from builtins table,
# converting NUMBER* and VALUE into harmless types and converting
# the showbuiltins() function into main().  Combined with the -DFUNCLIST
# we will avoid all of the complex calc types, macros and defines and
# be left with just main() and a mininal builtins table.
#
# Building funclist.o a portable fashion is ugly because some systems
# do not treat -I.. correctly!
#
builtin: builtin.top builtin.end ../func.c funclist.sed
	${Q}echo "forming builtin help file"
	-${Q}rm -f builtin
	${Q}cat builtin.top > builtin
	-${Q}rm -f funclist.c
	${Q}${SED} -n -f funclist.sed ../func.c > funclist.c

	-${Q}rm -f ../funclist.c ../funclist.o ../funclist funclist
	${Q}cp funclist.c ..
	-${Q}(cd ..; \
	 ${NATIVE_CC} ${NATIVE_CFLAGS} -DFUNCLIST funclist.c -o funclist; \
	 mv funclist help; \
	 rm -f funclist.c funclist.o funclist)
	${Q}./funclist | \
	    ${SED} -e 's/^/	/' -e 's/[	 ][	 ]*$$//' >> builtin
	${Q}cat builtin.end >> builtin
	${Q}echo "builtin help file formed"
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

##
#
# File list generation.  You can ignore this section.
#
#
# We will form the names of source files as if they were in a
# sub-directory called calc/help.
#
##

distlist: ${DISTLIST}
	${Q}for i in ${DISTLIST}; do \
		echo calc/help/$$i; \
	done | ${SORT}

# The bsdi distribution has generated files as well as distributed files.
#
bsdilist: ${DISTLIST} ${BLT_HELP_FILES}
	${Q}for i in ${DISTLIST} ${BLT_HELP_FILES}; do \
		echo calc/help/$$i; \
	done | ${SORT}

# The BSDI cdrom makefile expects all help files to be pre-built.  This rule
# creats these fils so that the release can be shipped off to BSDI.  You can
# ignore this rule.
#
bsdi: all
	rm -f obj
	cp obj.file obj

# These next rule help me form the ${DETAIL_HELP} makefile variables above.
#
detaillist:
	${Q}-(echo "xxxxx"; \
	   for i in ${DETAIL_HELP}; do \
		if [ ! -f RCS/$$i,v ]; then \
		    echo "WARNING: $$i not under RCS control" 1>&2; \
		else \
		    echo $$i; \
		fi; \
	done | ${SORT}) | ${FMT} -70 | \
	    ${SED} -e '1s/xxxxx/DETAIL_HELP=/' -e '2,$$s/^/	/' \
		   -e 's/$$/ \\/' -e '$$s/ \\$$//'

clean:
	rm -f obj mkbuiltin funclist.c funclist.o funclist

clobber:
	rm -f ${BLT_HELP_FILES} full .all calc
	rm -f obj mkbuiltin funclist.c funclist.o funclist
	rm -f ${SINGULAR_FILES} ${DETAIL_CLONE}

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
	${Q}for i in ${STD_HELP_FILES} ${BLT_HELP_FILES} builtin \
		  full ${DETAIL_HELP} ${SINGULAR_FILES}; do \
		echo rm -f ${HELPDIR}/$$i; \
		rm -f ${HELPDIR}/$$i; \
		echo cp $$i ${HELPDIR}; \
		cp $$i ${HELPDIR}; \
		echo chmod 0444 ${HELPDIR}/$$i; \
		chmod 0444 ${HELPDIR}/$$i; \
	done
	rm -f ${HELPDIR}/obj
	cp obj.file ${HELPDIR}/obj
	chmod 0444 ${HELPDIR}/obj
	# remove dead files
	-@rm -f rmblk block
