#
# lib - makefile for calc library scripts
#
# Copyright (c) 1997 David I. Bell and Landon Curt Noll
# Permission is granted to use, distribute, or modify this source,
# provided that this copyright notice remains intact.
#
# Arbitrary precision calculator.
#
# calculator by David I. Bell
# makefile by Landon Curt Noll

# required vars
#
SHELL = /bin/sh
MAKE_FILE = Makefile

# Normally, the upper level makefile will set these values.  We provide
# a default here just in case you want to build from this directory.
#
# where to install things
TOPDIR= /usr/local/lib
#TOPDIR= /usr/lib
#TOPDIR= /usr/libdata

LIBDIR= ${TOPDIR}/calc

# Makefile debug
#
# Q=@	do not echo internal makefile actions (quiet mode)
# Q=	echo internal makefile actions (debug / verbose mode)
#
#Q=
Q=@

# The calc files to install
#
CALC_FILES= README bigprime.cal deg.cal ellip.cal lucas.cal lucas_chk.cal \
	lucas_tbl.cal mersenne.cal mod.cal pell.cal pi.cal pix.cal \
	pollard.cal poly.cal psqrt.cal quat.cal regress.cal solve.cal \
	sumsq.cal surd.cal unitfrac.cal varargs.cal chrem.cal mfactor.cal \
	bindings altbind randmprime.cal test1700.cal randrun.cal \
	randbitrun.cal bernoulli.cal test2300.cal test2600.cal \
	test2700.cal test3100.cal test3300.cal test3400.cal prompt.cal \
	test3500.cal seedrandom.cal test4000.cal test4100.cal test4600.cal \
	beer.cal hello.cal test5100.cal test5200.cal randombitrun.cal \
	randomrun.cal xx_print.cal natnumset.cal qtime.cal

# These files are found (but not built) in the distribution
#
DISTLIST= ${CALC_FILES} ${MAKE_FILE}

all: ${CALC_FILES} ${MAKE_FILE} .all

# used by the upper level Makefile to determine of we have done all
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
		echo calc/lib/$$i; \
	done

# The bsdi distribution has generated files as well as distributed files.
#
bsdilist: ${DISTLIST}
	${Q}for i in ${DISTLIST}; do \
		echo calc/lib/$$i; \
	done

clean:

clobber:
	rm -f .all

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
	${Q}for i in ${CALC_FILES}; do \
		echo rm -f ${LIBDIR}/$$i; \
		rm -f ${LIBDIR}/$$i; \
		echo cp $$i ${LIBDIR}; \
		cp $$i ${LIBDIR}; \
		echo chmod 0444 ${LIBDIR}/$$i; \
		chmod 0444 ${LIBDIR}/$$i; \
	done
	${Q}echo remove files that are obsolete
	-rm -f nextprime.cal nextprim.cal
	-rm -f test1000.cal test2000.cal ${LIBDIR}/test2000.cal
	-rm -f ${LIBDIR}/nextprime.cal ${LIBDIR}/nextprim.cal
	-rm -f ${LIBDIR}/test1000.cal ${LIBDIR}/cryrand.cal
