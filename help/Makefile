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

# Standard help files
#
# The obj.file is special and is not listed here.
#
STD_HELP_FILES1= intro overview help command config \
	define environment expression file history interrupt mat
STD_HELP_FILES2= operator statement types usage variable
STD_HELP_FILES3= todo credit
STD_HELP_FILES= ${STD_HELP_FILES1} ${STD_HELP_FILES2} ${STD_HELP_FILES3}
SYMBOL_HELP= assign

# These two lists are prodiced by the detaillist and missinglist rules
# when no WARNINGS are detected.
#
DETAIL_HELP= abs access acos acosh acot acoth acsc acsch append appr archive \
	arg asec asech asin asinh assoc atan atan2 atanh avg base bround \
	btrunc ceil cfappr cfsim char cmdbuf cmp comb conj cos cosh cot coth \
	count cp csc csch ctime delete den det digit digits dp epsilon errno \
	error eval exp fact factor fclose fcnt feof ferror fflush fgetc \
	fgetfield fgetline fgets fgetstr fib files floor fopen forall fprintf \
	fputc fputs fputstr frac frem freopen fscan fscanf fseek fsize ftell \
	gcd gcdrem getenv hash head highbit hmean hypot ilog ilog10 ilog2 im \
	insert int inverse iroot isassoc isatty isconfig iserror iseven \
	isfile ishash isident isint islist ismat ismult isnull isnum isobj \
	isodd isprime isqrt isrand israndom isreal isrel isset issimple issq \
	isstr istype jacobi join lcm lcmfact lfactor list ln lowbit ltol \
	makelist matdim matfill matmax matmin matsum mattrans max meq min \
	minv mmin mne mod modify near newerror nextcand nextprime norm null \
	num ord param perm pfact pi pix places pmod polar poly pop power \
	prevcand prevprime printf prompt ptest push putenv quo quomod rand \
	randbit randperm rcin rcmul rcout rcpow rcsq re rm remove reverse \
	rewind root round rsearch runtime scale scan scanf search sec sech \
	segment select sgn sin sinh size sizeof sort sqrt srand ssq str \
	strcat strerror strlen strpos strprintf strscan strscanf substr swap \
	system tail tan tanh time trunc xor

# Help files that are constructed from other sources
#
# The obj.file is special and is not listed here.
#
BUILT_HELP_FILES= bindings altbind changes libcalc stdlib bugs errorcodes

# Singular files
#
# These files are copies of their plural form.
#
PLURAL_FILES= bindings bugs changes errorcodes types
SINGULAR_FILES= binding bug change errorcode type

# These files are found (but not built) in the distribution
#
DISTLIST= ${STD_HELP_FILES} ${DETAIL_HELP} ${SYMBOL_HELP} ${MAKE_FILE} \
	obj.file builtin.top builtin.end funclist.sed \
	errorcodes.hdr errorcodes.sed

all: ${STD_HELP_FILES} obj.file ${BUILT_HELP_FILES} full \
     ${DETAIL_HELP} ${SINGULAR_FILES} builtin .all

# used by the upper level Makefile to determine of we have done all
#
# NOTE: Due to bogus shells found on one common system we must have
#       an non-emoty else clause for every if condition.  *sigh*
#
.all:
	rm -f .all
	touch .all

bindings: ../lib/bindings
	rm -f bindings
	cp ../lib/bindings bindings
	chmod 0444 bindings
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

altbind: ../lib/altbind
	rm -f altbind
	cp ../lib/altbind altbind
	chmod 0444 altbind
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

stdlib: ../lib/README
	rm -f stdlib
	cp ../lib/README stdlib
	chmod 0444 stdlib
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

changes: ../CHANGES
	rm -f changes
	cp ../CHANGES changes
	chmod 0444 changes
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

libcalc: ../LIBRARY
	rm -f libcalc
	${SED} -e 's:$${LIBDIR}:${LIBDIR}:g' < ../LIBRARY > libcalc
	chmod 0444 libcalc
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

bugs: ../BUGS
	rm -f bugs
	cp ../BUGS bugs
	chmod 0444 bugs
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

errorcodes: ../calcerr.h errorcodes.hdr errorcodes.sed
	rm -f errorcodes
	${CAT} errorcodes.hdr > errorcodes
	${SED} -n -f errorcodes.sed < ../calcerr.h >> errorcodes
	chmod 0444 errorcodes
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= skipping the cat of help/$@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

full: ${STD_HELP_FILES} ${BUILT_HELP_FILES} ${MAKE_FILE}
	${Q}echo "forming full"
	-${Q}rm -f full
	-${Q}for i in ${STD_HELP_FILES1} obj.file ${STD_HELP_FILES2} \
	    ${BUILT_HELP_FILES} ${STD_HELP_FILES3}; do \
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
	done > full
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
bsdilist: ${DISTLIST} ${BUILT_HELP_FILES}
	${Q}for i in ${DISTLIST} ${BUILT_HELP_FILES}; do \
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
		if [ ! -f SCCS/s.$$i ]; then \
		    echo "WARNING: $$i not under SCCS control" 1>&2; \
		else \
		    echo $$i; \
		fi; \
	done | ${SORT}) | ${FMT} -70 | \
	    ${SED} -e '1s/xxxxx/DETAIL_HELP=/' -e '2,$$s/^/	/' \
		   -e 's/$$/ \\/' -e '$$s/ \\$$//'

clean:
	rm -f obj mkbuiltin funclist.c funclist.o funclist

clobber:
	rm -f ${BUILT_HELP_FILES} full builtin .all
	rm -f obj mkbuiltin funclist.c funclist.o funclist ${SINGULAR_FILES}

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
	${Q}for i in ${STD_HELP_FILES} ${BUILT_HELP_FILES} builtin \
		  full ${DETAIL_HELP} ${SINGULAR_FILES} ${SYMBOL_HELP}; do \
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
