#!/bin/make
#
# (Gerneric calc makefile)
#
# Copyright (c) 1995 David I. Bell and Landon Curt Noll
# Permission is granted to use, distribute, or modify this source,
# provided that this copyright notice remains intact.
#
# Arbitrary precision calculator.
#
# calculator by David I. Bell with help/mods from others
# Makefile by Landon Curt Noll

##############################################################################
#-=-=-=-=-=-=-=-=- You may want to change some values below -=-=-=-=-=-=-=-=-#
##############################################################################

# Determine the type of terminal controls that you want to use
#
#	value	  	  meaning
#	--------	  -------
#	(nothing)	  let the makefile guess at what you need
#	-DUSE_TERMIOS	  use struct termios from <termios.h>
#	-DUSE_TERMIO 	  use struct termios from <termio.h>
#	-DUSE_SGTTY    	  use struct sgttyb from <sys/ioctl.h>
#
# If in doubt, leave TERMCONTROL empty.
#
TERMCONTROL=
#TERMCONTROL= -DUSE_TERMIOS
#TERMCONTROL= -DUSE_TERMIO
#TERMCONTROL= -DUSE_SGTTY

# If your system does not have a vsprintf() function, you could be in trouble.
#
#	vsprintf(stream, format, ap)
#
# This function works like sprintf except that the 3rd arg is a va_list
# strarg (or varargs) list.  Some old systems do not have vsprintf().
# If you do not have vsprintf(), then calc will try sprintf() and hope
# for the best.
#
# If HAVE_VSPRINTF is empty, this makefile will run the have_stdvs.c and/or
# have_varvs.c program to determine if vsprintf() is supported.  If
# HAVE_VSPRINTF is set to -DDONT_HAVE_VSPRINTF then calc will hope that
# sprintf() will work.
#
# If in doubt, leave HAVE_VSPRINTF empty.
#
HAVE_VSPRINTF=
#HAVE_VSPRINTF= -DDONT_HAVE_VSPRINTF

# Determine the byte order of your machine
#
#    Big Endian:	Amdahl, 68k, Pyramid, Mips, Sparc, ...
#    Little Endian:	Vax, 32k, Spim (Dec Mips), i386, i486, ...
#
# If in doubt, leave BYTE_ORDER empty.  This makefile will attempt to
# use BYTE_ORDER in <machine/endian.h> or it will attempt to run
# the endian program.  If you get syntax errors when you compile,
# try forcing the value to be BIG_ENDIAN and run the calc regression
# tests. (see the README file)  If the calc regression tests fail, do
# a make clobber and try LITTLE_ENDIAN.  If that fails, ask a wizard
# for help.
#
BYTE_ORDER=
#BYTE_ORDER= BIG_ENDIAN
#BYTE_ORDER= LITTLE_ENDIAN

# Determine the number of bits in a long
#
# If in doubt, leave LONG_BITS empty.  This makefile will run
# the longbits program to determine the length.
#
LONG_BITS=
#LONG_BITS= 32
#LONG_BITS= 64

# Determine if your compiler supports the long long type and if so, its length
#
# If LONGLONG_BITS is set to nothing, the Makefile will run the longlong
# program to determine if it supports them and if so, their length.
# To disable the use of long longs, set LONGLONG_BITS to 0.
# One may hard code the length of a long long by setting LONGLONG_BITS
# to a non-zero value.
#
# On some machines, using long longs will make the cpu intensive
# jobs run faster.  On others, using long longs make things slower.
# On some systems, the regression test runs slower while cpu bound
# jobs run faster.  On others, the reverse is true.
#
# If in doubt, try to leave LONGLONG_BITS empty.  Do a 'make check'
# and change to 'LONGLONG_BITS= 0' if you encounter problems.
#
#LONGLONG_BITS= 0
LONGLONG_BITS=
#LONGLONG_BITS= 64

# Determine if we have the ANSI C fgetpos and fsetpos alternate interface
# to the ftell() and fseek() (with whence set to SEEK_SET) functions.
#
# If HAVE_FPOS is empty, this makefile will run the have_fpos program
# to determine if there is are fgetpos and fsetpos functions.  If HAVE_FPOS
# is set to -DHAVE_NO_FPOS, then calc will use ftell() and fseek().
#
# If in doubt, leave HAVE_FPOS empty.
#
HAVE_FPOS=
#HAVE_FPOS= -DHAVE_NO_FPOS

# Determine if we have ANSI C const.
#
# If HAVE_CONST is empty, this makefile will run the have_const program
# to determine if const is supported.  If HAVE_CONST is set to -DHAVE_NO_CONST,
# then calc will not use const.
#
# If in doubt, leave HAVE_CONST empty.
#
HAVE_CONST=
#HAVE_CONST= -DHAVE_NO_CONST

# Determine if we have uid_t
#
# If HAVE_UID_T is empty, this makefile will run the have_uid_t program
# to determine if const is supported.  If HAVE_UID_T is set to -DHAVE_NO_UID_T,
# then calc will treat uid_t as an unsigned short.  This only matters if
# $HOME is not set and calc must look up the home directory in /etc/passwd.
#
# If in doubt, leave HAVE_UID_T empty.
#
HAVE_UID_T=
#HAVE_UID_T= -DHAVE_NO_UID_T

# Determine if we have memcpy(), memset() and strchr()
#
# If HAVE_NEWSTR is empty, this makefile will run the have_newstr program
# to determine if memcpy(), memset() and strchr() are supported.  If
# HAVE_NEWSTR is set to -DHAVE_NO_NEWSTR, then calc will use bcopy() instead
# of memcpy(), use bfill() instead of memset(), and use index() instead of
# strchr().
#
# If in doubt, leave HAVE_NEWSTR empty.
#
HAVE_NEWSTR=
#HAVE_NEWSTR= -DHAVE_NO_NEWSTR

# Some architectures such as Sparc do not allow one to access 32 bit values
# that are not alligned on a 32 bit boundary.
#
# The Dec Alpha running OSF/1 will produce alignment error messages when
# align32.c tries to figure out if alignment is needed.  Use the
# ALIGN32= -DMUST_ALIGN32 to force alignment and avoid such error messages.
#
# ALIGN32=		     let align32.c figure out if alignment is needed
# ALIGN32= -DMUST_ALIGN32    force 32 bit alignment
# ALIGN32= -UMUST_ALIGN32    allow non-aligment of 32 bit accesses
#
# When in doubt, be safe and pick ALIGN32=-DMUST_ALIGN32.
#
#ALIGN32=
ALIGN32= -DMUST_ALIGN32
#ALIGN32= -UMUST_ALIGN32

# The return value type of main() differs from platform to platform.
# In some cases, a compiler warning is issued because main() does
# or does not return a value.
#
# MAIN= -DMAIN=void	main() is of type void
# MAIN= -DMAIN=int	main() is of type int
#
# When in dobut, try MAIN= -DMAIN=void.  If you get a warning try the other.
#
MAIN= -DMAIN=void
#MAIN= -DMAIN=int

# where to install binary files
#
BINDIR= /usr/local/bin
#BINDIR= /usr/bin
#BINDIR= /usr/contrib/bin

# where to install the *.cal, *.h and *.a files
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

# where man pages are installed
#
# Use MANDIR= to disable installation of the calc man (source) page.
#
MANDIR=
#MANDIR= /usr/local/man/man1
#MANDIR= /usr/man/man1
#MANDIR= /usr/share/man/man1
#MANDIR= /usr/man/u_man/man1
#MANDIR= /usr/contrib/man/man1

# where cat (formatted man) pages are installed
#
# Use CATDIR= to disable installation of the calc cat (formatted) page.
#
CATDIR=
#CATDIR= /usr/local/man/cat1
#CATDIR= /usr/local/catman/cat1
#CATDIR= /usr/man/cat1
#CATDIR= /usr/share/man/cat1
#CATDIR= /usr/man/u_man/cat1
#CATDIR= /usr/contrib/man/cat1

# extenstion to add on to the calc man page filename
#
# This is ignored if CATDIR is empty.
#
MANEXT= 1
#MANEXT= l

# extenstion to add on to the calc man page filename
#
# This is ignored if CATDIR is empty.
#
CATEXT= 1
#CATEXT= 0
#CATEXT= l

# how to format a man page
#
# If CATDIR is non-empty, then
#     If NROFF is non-empty, then
#	  ${NROFF} ${NROFF_ARG} calc.1 > ${CATDIR}/calc.${CATEXT}
#	  is used to built and install the cat page
#     else (NROFF is empty)
#	  ${MANMAKE} calc.1 ${CATDIR}
#	  is used to built and install the cat page
# else
#     The cat page is not built or installed
#
# If in doubt and you don't want to fool with man pages, set MANDIR
# and CATDIR to empty and ignore the lines below.
#
NROFF= nroff
#NROFF=
#NROFF= groff
NROFF_ARG= -man
#NROFF_ARG= -mandoc
MANMAKE= /usr/local/bin/manmake
#MANMAKE= manmake

# If the $CALCPATH environment variable is not defined, then the following
# path will be search for calc lib routines.
#
CALCPATH= .:./lib:~/lib:${LIBDIR}

# If the $CALCRC environment variable is not defined, then the following
# path will be search for calc lib routines.
#
CALCRC= ${LIBDIR}/startup:~/.calcrc

# If the $CALCBINDINGS environment variable is not defined, then the following
# file will be used for the command line and edit history key bindings.
# The $CALCPATH will be used to search for this file.
#
#	${LIBDIR}/bindings	uses ^D for editing
#	${LIBDIR}/altbind	uses ^D for EOF
#
CALCBINDINGS= bindings
#CALCBINDINGS= altbind

# If $PAGER is not set, use this program to display a help file
#
CALCPAGER= more
#CALCPAGER= pg
#CALCPAGER= cat
#CALCPAGER= less

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

# On systems that have dynamic shared libs, you may want want to disable them
# for faster calc startup.
#
#    System type    NO_SHARED recomendation
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

# On some systems where you are disabling dynamic shared libs, you may
# need to pass a special flag to ${CC} during linking stage.
#
#    System type    			    NO_SHARED recomendation
#
#	IRIX with NO_SHARED= -non_shared    LD_NO_SHARED= -Wl,-rdata_shared
#	IRIX with NO_SHARED=		    LD_NO_SHARED=
#	others	    			    LD_NO_SHARED=
#
# If in doubt, use LD_NO_SHARED=
#
LD_NO_SHARED=
#LD_NO_SHARED= -Wl,-rdata_shared

# Some systems require one to use ranlib to add a symbol table to
# a *.a library.  Set RANLIB to the utility that performs this action.
# Set RANLIB to : if your system does not need such a utility.
#
RANLIB=ranlib
#RANLIB=:

# Some systems are able to form lint libs.  How it is formed depends
# on your system.  If you do not care about lint, use : as the
# LINTLIB value.
#
#    System type    LINTLIB recomendation
#
#	BSD	    ${LINT} ${LCFLAGS} ${LINTFLAGS} -u -Ccalc
#	SYSV	    ${LINT} ${LCFLAGS} ${LINTFLAGS} -u -o calc
#	disable     :
#
# If in doubt and you don't care about lint, use LINTLIB= :
#
#LINTLIB= ${LINT} ${LCFLAGS} ${LINTFLAGS} -u -Ccalc
#LINTLIB= ${LINT} ${LCFLAGS} ${LINTFLAGS} -u -o calc
LINTLIB= :

# The lint flags vary from system to system.  Some systems have the
# opposite meaning for the flags below.  Other systems change flag
# meaning altogether.
#
#       System    LINTFLAGS recomendation
#
#	SunOs	  -a -h -v -z
#
# If in doubt and you don't care about lint, set LINTFLAGS to empty.
#
#LINTFLAGS= -a -h -v -z
LINTFLAGS=

# Normally certain files depend on the Makefile.  If the Makefile is
# changed, then certain steps should be redone.  If MAKE_FILE is
# set to Makefile, then these files will depend on Makefile.  If
# MAKE_FILE is empty, they they wont.
#
# If in doubt, set MAKE_FILE to Makefile
#
MAKE_FILE= Makefile
#MAKE_FILE=

# If you do not wish to use purify, leave PURIFY commented out.
#
# If in doubt, leave PURIFY commented out.
#
#PURIFY= purify -logfile=pure.out
#PURIFY= purify

###
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
# CCMAIN are flags for ${CC} when files with main() instead of CFLAGS
# CCSHS are flags given to ${CC} for compiling shs.c instead of CFLAGS
# CCZPRIME are flags given to ${CC} for compiling zprime.c instead of CFLAGS
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
CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
ICFLAGS= ${CCWARN} ${CCMISC}
#
CCMAIN= ${ICFLAGS} ${MAIN}
CCSHS= ${CFLAGS}
CCZPRIME= ${CFLAGS}
#
LCFLAGS=
LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
ILDFLAGS=
#
CC= ${PURIFY} cc
#
###
#
# SGI IRIX5.3 (or earlier) C Compiler
#
# You must set above:
#	RANLIB=:
#	LONGLONG_BITS= 0
#
# for better performance, set the following above:
#	DEBUG= -O2 -g3
#
# If you have the directory /usr/lib/nonshared, then set the following above:
#	NO_SHARED= -non_shared
#	LD_NO_SHARED= -Wl,-rdata_shared
#
#CCWARN= -fullwarn -woff 835
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -use_readonly_const
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} cc
#
###
#
# SGI IRIX6.2 (or later) Mongoose 7.0 (or later) C Compiler for the R4k
#
# You must set above:
#	RANLIB=:
#
# for better performance, set the following above:
#	DEBUG= -O2 -g3
#
# If you have the directory /usr/lib32/nonshared, then set the following above:
#	NO_SHARED= -non_shared
#	LD_NO_SHARED= -Wl,-rdata_shared
#
# woff 1209: cancel 'controlling expression is constant' warnings
#
#CCWARN= -fullwarn -woff 1209
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -use_readonly_const
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CCWARN} ${NO_SHARED} ${CCMISC} -O1
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} cc -n32 -r4000
#
###
#
# SGI IRIX6.2 (or later) Mongoose 7.0 (or later) C Compiler for the R8k
#
# You must set above:
#	RANLIB=:
#
# for better performance, set the following above:
#	DEBUG= -O2 -g3
#
# If you have the directory /usr/lib32/nonshared, then set the following above:
#	NO_SHARED= -non_shared
#	LD_NO_SHARED= -Wl,-rdata_shared
#
# woff 1209: cancel 'controlling expression is constant' warnings
#
#CCWARN= -fullwarn -woff 1209
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -use_readonly_const
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CCWARN} ${NO_SHARED} ${CCMISC} -O1
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} cc -n32 -r8000
#
###
#
# SGI IRIX6.2 (or later) Mongoose 7.0 (or later) C Compiler for the R10k
#
# You must set above:
#       RANLIB=:
#
# for better performance, set the following above:
#	DEBUG= -O2 -g3
#
# If you have the directory /usr/lib32/nonshared, then set the following above:
#	NO_SHARED= -non_shared
#	LD_NO_SHARED= -Wl,-rdata_shared
#
# woff 1209: cancel 'controlling expression is constant' warnings
#
#CCWARN= -fullwarn -woff 1209
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -use_readonly_const
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CCWARN} ${NO_SHARED} ${CCMISC} -O1
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} cc -n32 -r10000
#
###
#
# HP-UX set
#
# for better performance, try set the following above:
#     DEBUG= -O
#
# Warning: Some HP-UX optimizers are brain-damaged.  If 'make check' fails use:
#     DEBUG= -g
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC=
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} cc
#
###
#
# RS6000 set
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -qlanglvl=ansi
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} cc
#
###
#
# Dec Alpha without gcc set
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -DFUNCT_DECL_BUG
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} cc
#
###
#
# BSDI's BSD/OS 2.0 (or later) set
#
# for better performance, set the following above:
#     DEBUG= -O2
#
#CCWARN= -Wall -Wno-implicit -Wno-comment
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -ansi
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} shlicc2
#
###
#
# Solaris 2.x Sun cc compiler
#
# for better performance, set the following above:
#     DEBUG= -O
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC=-Xc
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} cc
#
###
#
# gcc set
#
# for better performance, set the following above:
#     DEBUG= -O
#
#CCWARN= -Wall
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -ansi
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} gcc
#
###
#
# gcc1 set	(some call it gcc1, some call it gcc)
#
# for better performance, set the following above:
#     DEBUG= -O
#
#CCWARN= -Wall
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -ansi
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} gcc1
#CC= ${PURIFY} gcc
#
###
#
# gcc2 set	(some call it gcc2, some call it gcc)
#
# for better performance, set the following above:
#     DEBUG= -O2
#
#CCWARN= -Wall -Wno-implicit -Wno-comment
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -ansi
#
#CFLAGS= ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= ${CCWARN} ${CCMISC}
#
#CCMAIN= ${ICFLAGS} ${MAIN}
#CCSHS= ${CFLAGS}
#CCZPRIME= ${CFLAGS}
#
#LCFLAGS=
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#CC= ${PURIFY} gcc2
#CC= ${PURIFY} gcc

##############################################################################
#-=-=-=-=-=-=-=-=- Be careful if you change something below -=-=-=-=-=-=-=-=-#
##############################################################################

# standard utilities used during make
#
SHELL= /bin/sh
MAKE= make
AWK= awk
SED= sed
SORT= sort
TEE= tee
LINT= lint
CTAGS= ctags
# assume the X11 makedepend tool for the depend rule
MAKEDEPEND= makedepend

# Makefile debug
#
# Q=@	do not echo internal makefile actions (quiet mode)
# Q=	echo internal makefile actions (debug / verbose mode)
#
# V=@:	do not echo debug statements (quiet mode)
# V=@	do echo debug statements (debug / verbose mode)
#
#Q=
Q=@
V=@:
#V=@

# the source files which are built into a math library
#
# There MUST be a .o for every .c in LIBOBJS.
#
# NOTE: calcerr.c is built by this Makefile so it is not a real source.
#
LIBSRC= byteswap.c comfunc.c commath.c config.c jump.c lib_calc.c \
	math_error.c pix.c poly.c prime.c qfunc.c qio.c qmath.c qmod.c \
	qtrans.c zfunc.c zio.c zmath.c zmod.c zmul.c zprime.c zrand.c

# the object files which are built into a math library
#
# There MUST be a .o for every .c in LIBSRC.
#
# NOTE: calcerr.o comes from calcerr.c which is built
#
LIBOBJS= byteswap.o comfunc.o commath.o config.o jump.o lib_calc.o \
	math_error.o pix.o poly.o prime.o qfunc.o qio.o qmath.o qmod.o \
	qtrans.o zfunc.o zio.o zmath.o zmod.o zmul.o zprime.o zrand.o \
	calcerr.o

# the calculator source files
#
# There MUST be a .c for every .o in CALCOBJS.
#
CALCSRC= addop.c assocfunc.c calc.c codegen.c const.c file.c hash.c \
	quickhash.c func.c hist.c input.c label.c listfunc.c matfunc.c obj.c \
	opcodes.c shs.c string.c symbol.c token.c value.c version.c

# we build these .o files for calc
#
# There MUST be a .o for every .c in CALCSRC.
#
CALCOBJS= addop.o assocfunc.o calc.o codegen.o const.o file.o hash.o \
	quickhash.o func.o hist.o input.o label.o listfunc.o matfunc.o obj.o \
	opcodes.o shs.o string.o symbol.o token.o value.o version.o

# we build these .h files during the make
#
BUILD_H_SRC= align32.h args.h calcerr.h conf.h endian_calc.h fposval.h \
	have_const.h have_fpos.h have_malloc.h have_newstr.h have_stdlib.h \
	have_string.h have_uid_t.h have_unistd.h longbits.h longlong.h \
	terminal.h have_times.h

# we build these .c files during the make
#
BUILD_C_SRC= calcerr.c

# these .c files may be used in the process of building BUILD_H_SRC
#
# There MUST be a .c for every .o in UTIL_OBJS.
#
UTIL_C_SRC= align32.c endian.c longbits.c have_newstr.c have_uid_t.c \
	have_const.c have_stdvs.c have_varvs.c fposval.c have_fpos.c longlong.c

# these awk and sed tools are used in the process of building BUILD_H_SRC
# and BUILD_C_SRC
#
UTIL_MISC_SRC= calcerr_h.sed calcerr_h.awk calcerr_c.sed calcerr_c.awk \
	calcerr.tbl check.awk

# these .o files may get built in the process of building BUILD_H_SRC
#
# There MUST be a .o for every .c in UTIL_C_SRC.
#
UTIL_OBJS= endian.o longbits.o have_newstr.o have_uid_t.o \
	have_const.o fposval.o have_fpos.o longlong.o try_strarg.o \
	have_stdvs.o have_varvs.o

# these temp files may be created  (and removed) during the build of BUILD_C_SRC
#
UTIL_TMP= ll_tmp fpos_tmp fposv_tmp const_tmp uid_tmp newstr_tmp vs_tmp

# these utility progs may be used in the process of building BUILD_H_SRC
#
UTIL_PROGS= align32 fposval have_uid_t longlong have_const \
	endian longbits have_newstr have_stdvs have_varvs

# these .h files are needed by programs that use libcalc.a
#
LIB_H_SRC= alloc.h byteswap.h cmath.h config.h jump.h \
	prime.h qmath.h zmath.h zrand.h

# these .h files are neither built, nor required by libcalc.a
#
CALC_H_SRC= calc.h file.h func.h hash.h hist.h label.h opcodes.h \
	shs.h string.h symbol.h token.h value.h

# complete list of .h files found (but not built) in the distribution
#
H_SRC= ${CALC_H_SRC} ${LIB_H_SRC}

# complete list of .c files found (but not built) in the distribution
#
C_SRC= ${LIBSRC} ${CALCSRC} ${UTIL_C_SRC}

# These files are found (but not built) in the distribution
#
DISTLIST= ${C_SRC} ${H_SRC} ${MAKE_FILE} BUGS CHANGES LIBRARY README \
	calc.man lint.sed README.FIRST ${UTIL_MISC_SRC}

# complete list of .o files
#
OBJS= ${LIBOBJS} ${CALCOBJS} ${UTIL_OBJS}

# complete list of progs built
#
PROGS= calc ${UTIL_PROGS}

# complete list of targets
#
TARGETS= calc calc.1 lib/.all help/.all help/builtin


###
#
# The reason for this Makefile  :-)
#
###

all: ${TARGETS}

calc: libcalc.a ${CALCOBJS}
	${CC} ${LDFLAGS} ${CALCOBJS} libcalc.a -o calc

libcalc.a: ${LIBOBJS} ${MAKE_FILE}
	-rm -f libcalc.a
	ar qc libcalc.a ${LIBOBJS}
	${RANLIB} libcalc.a

calc.1: calc.man ${MAKE_FILE}
	-rm -f calc.1
	${SED} -e 's:$${LIBDIR}:${LIBDIR}:g' < calc.man > calc.1

##
#
# Special .o files
#
##

calc.o: calc.c ${MAKE_FILE}
	${CC} ${CCMAIN} ${CCOPT} -c calc.c

hist.o: hist.c ${MAKE_FILE}
	${CC} ${CFLAGS} ${TERMCONTROL} -c hist.c

shs.o: shs.c ${MAKE_FILE}
	${CC} ${CCSHS} -c shs.c

zprime.o: zprime.c ${MAKE_FILE}
	${CC} ${CCZPRIME} -c zprime.c


##
#
# Doing a 'make check' will cause the regression test suite to be executed.
# This rule will try to build anything that needs to be built as well.
#
# Doing a 'make chk' will cause only the context around interesting
# (and error) messages to be printed.  Unlike 'make check', this
# rule does not cause things to be built.  I.e., the all rule is
# not invoked.
#
##

check: all ./lib/regress.cal ./lib/lucas.cal ./lib/lucas_chk.cal \
	./lib/test1700.cal ./lib/test2300.cal ./lib/test2600.cal \
	./lib/test2700.cal ./lib/test3100.cal ./lib/test3300.cal \
	./lib/test3400.cal ./lib/test3500.cal ./lib/test4000.cal \
	./lib/test4100.cal ./lib/surd.cal
	CALCPATH="./lib" ./calc -q read regress

chk: ./lib/regress.cal ./lib/lucas.cal ./lib/lucas_chk.cal \
	./lib/test1700.cal ./lib/test2300.cal ./lib/test2600.cal \
	./lib/test2700.cal ./lib/test3100.cal ./lib/test3300.cal \
	./lib/test3400.cal ./lib/test3500.cal ./lib/test4000.cal \
	./lib/test4100.cal ./lib/surd.cal check.awk
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	CALCPATH="./lib" ./calc -q read regress 2>&1 | ${AWK} -f check.awk
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

##
#
# The next set of rules cause the .h files BUILD_H_SRC files to be built
# according tot he system and the Makefile variables above.  The hsrc rule
# is a conveient rule to invoke to built all of the BUILD_H_SRC.
#
# We add in the BUILD_C_SRC files because they are similar to the
# BUILD_H_SRC files in terms of the build process.
#
# NOTE: Due to bogus shells found on one common system we must have
#	an non-emoty else clause for every if condition.  *sigh*
#	We also place ; true at the end of some commands to avoid
#	meaningless cosmetic messages by the same system.
#
##

hsrc: ${BUILD_H_SRC} ${BUILD_C_SRC}

conf.h: ${MAKE_FILE}
	-${Q}rm -f conf.h
	${Q}echo 'forming conf.h'
	${Q}echo '/*' > conf.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> conf.h
	${Q}echo ' */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '#if !defined(_CONF_H_)' >> conf.h
	${Q}echo '#define _CONF_H_' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the default :-separated search path */' >> conf.h
	${Q}echo '#ifndef DEFAULTCALCPATH' >> conf.h
	${Q}echo '#define DEFAULTCALCPATH "${CALCPATH}"' >> conf.h
	${Q}echo '#endif /* DEFAULTCALCPATH */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the default :-separated startup file list */' >> conf.h
	${Q}echo '#ifndef DEFAULTCALCRC' >> conf.h
	${Q}echo '#define DEFAULTCALCRC "${CALCRC}"' >> conf.h
	${Q}echo '#endif /* DEFAULTCALCRC */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the default key bindings file */' >> conf.h
	${Q}echo '#ifndef DEFAULTCALCBINDINGS' >> conf.h
	${Q}echo '#define DEFAULTCALCBINDINGS "${CALCBINDINGS}"' >> conf.h
	${Q}echo '#endif /* DEFAULTCALCBINDINGS */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the location of the help directory */' >> conf.h
	${Q}echo '#ifndef HELPDIR' >> conf.h
	${Q}echo '#define HELPDIR "${HELPDIR}"' >> conf.h
	${Q}echo '#endif /* HELPDIR */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the default pager to use */' >> conf.h
	${Q}echo '#ifndef DEFAULTCALCPAGER' >> conf.h
	${Q}echo '#define DEFAULTCALCPAGER "${CALCPAGER}"' >> conf.h
	${Q}echo '#endif /* DEFAULTCALCPAGER */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '#endif /* _CONF_H_ */' >> conf.h
	${Q}echo 'conf.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

endian_calc.h: endian ${MAKE_FILE}
	-${Q}rm -f endian_calc.h
	${Q}echo 'forming endian_calc.h'
	${Q}echo '/*' > endian_calc.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> endian_calc.h
	${Q}echo ' */' >> endian_calc.h
	${Q}echo '' >> endian_calc.h
	${Q}echo '#if !defined(_ENDIAN_CALC_H_)' >> endian_calc.h
	${Q}echo '#define _ENDIAN_CALC_H_' >> endian_calc.h
	${Q}echo '' >> endian_calc.h
	${Q}echo '/* what byte order are we? */' >> endian_calc.h
	-${Q}if [ X"${BYTE_ORDER}" = X ]; then \
	    if [ -f /usr/include/machine/endian.h ]; then \
		echo '#include <machine/endian.h>' >> endian_calc.h; \
	    else \
		./endian >> endian_calc.h; \
	    fi; \
	else \
	    echo "#define BYTE_ORDER ${BYTE_ORDER}" >> endian_calc.h; \
	fi
	${Q}echo '' >> endian_calc.h
	${Q}echo '#endif /* _ENDIAN_CALC_H_ */' >> endian_calc.h
	${Q}echo 'endian_calc.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

longbits.h: longbits ${MAKE_FILE}
	-${Q}rm -f longbits.h
	${Q}echo 'forming longbits.h'
	${Q}echo '/*' > longbits.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> longbits.h
	${Q}echo ' */' >> longbits.h
	${Q}echo '' >> longbits.h
	${Q}echo '#if !defined(_LONGBITS_H_)' >> longbits.h
	${Q}echo '#define _LONGBITS_H_' >> longbits.h
	${Q}echo '' >> longbits.h
	-${Q}if [ X"${LONG_BITS}" = X ]; then \
	    ./longbits >> longbits.h; \
	else \
	    echo "#define LONG_BITS ${LONG_BITS}" >> longbits.h; \
	fi
	${Q}echo '' >> longbits.h
	${Q}echo '#endif /* _LONGBITS_H_ */' >> longbits.h
	${Q}echo 'longbits.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_malloc.h: ${MAKE_FILE}
	-${Q}rm -f have_malloc.h
	${Q}echo 'forming have_malloc.h'
	${Q}echo '/*' > have_malloc.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_malloc.h
	${Q}echo ' */' >> have_malloc.h
	${Q}echo '' >> have_malloc.h
	${Q}echo '#if !defined(_HAVE_MALLOC_H_)' >> have_malloc.h
	${Q}echo '#define _HAVE_MALLOC_H_' >> have_malloc.h
	${Q}echo '' >> have_malloc.h
	${Q}echo '/* do we have /usr/include/malloc.h? */' >> have_malloc.h
	-${Q}if [ -f /usr/include/malloc.h ]; then \
	    echo '#define HAVE_MALLOC_H  /* yes */' >> have_malloc.h; \
	else \
	    echo '#undef HAVE_MALLOC_H  /* no */' >> have_malloc.h; \
	fi
	${Q}echo '' >> have_malloc.h
	${Q}echo '#endif /* _HAVE_MALLOC_H_ */' >> have_malloc.h
	${Q}echo 'have_malloc.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_times.h: ${MAKE_FILE}
	-${Q}rm -f have_times.h
	${Q}echo 'forming have_times.h'
	${Q}echo '/*' > have_times.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_times.h
	${Q}echo ' */' >> have_times.h
	${Q}echo '' >> have_times.h
	${Q}echo '#if !defined(_HAVE_TIMES_H_)' >> have_times.h
	${Q}echo '#define _HAVE_TIMES_H_' >> have_times.h
	${Q}echo '' >> have_times.h
	${Q}echo '/* do we have /usr/include/times.h? */' >> have_times.h
	-${Q}if [ -f /usr/include/times.h ]; then \
	    echo '#define HAVE_TIMES_H  /* yes */' >> have_times.h; \
	else \
	    echo '#undef HAVE_TIMES_H  /* no */' >> have_times.h; \
	fi
	-${Q}if [ -f /usr/include/sys/times.h ]; then \
	    echo '#define HAVE_SYS_TIMES_H  /* yes */' >> have_times.h; \
	else \
	    echo '#undef HAVE_SYS_TIMES_H  /* no */' >> have_times.h; \
	fi
	-${Q}if [ -f /usr/include/time.h ]; then \
	    echo '#define HAVE_TIME_H  /* yes */' >> have_times.h; \
	else \
	    echo '#undef HAVE_TIME_H  /* no */' >> have_times.h; \
	fi
	-${Q}if [ -f /usr/include/sys/time.h ]; then \
	    echo '#define HAVE_SYS_TIME_H  /* yes */' >> have_times.h; \
	else \
	    echo '#undef HAVE_SYS_TIME_H  /* no */' >> have_times.h; \
	fi
	${Q}echo '' >> have_times.h
	${Q}echo '#endif /* _HAVE_TIMES_H_ */' >> have_times.h
	${Q}echo 'have_times.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_stdlib.h: ${MAKE_FILE}
	-${Q}rm -f have_stdlib.h
	${Q}echo 'forming have_stdlib.h'
	${Q}echo '/*' > have_stdlib.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_stdlib.h
	${Q}echo ' */' >> have_stdlib.h
	${Q}echo '' >> have_stdlib.h
	${Q}echo '#if !defined(_HAVE_STDLIB_H_)' >> have_stdlib.h
	${Q}echo '#define _HAVE_STDLIB_H_' >> have_stdlib.h
	${Q}echo '' >> have_stdlib.h
	${Q}echo '/* do we have /usr/include/stdlib.h? */' >> have_stdlib.h
	-${Q}if [ -f /usr/include/stdlib.h ]; then \
	    echo '#define HAVE_STDLIB_H  /* yes */' >> have_stdlib.h; \
	else \
	    echo '#undef HAVE_STDLIB_H  /* no */' >> have_stdlib.h; \
	fi
	${Q}echo '' >> have_stdlib.h
	${Q}echo '#endif /* _HAVE_STDLIB_H_ */' >> have_stdlib.h
	${Q}echo 'have_stdlib.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_unistd.h: ${MAKE_FILE}
	-${Q}rm -f have_unistd.h
	${Q}echo 'forming have_unistd.h'
	${Q}echo '/*' > have_unistd.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_unistd.h
	${Q}echo ' */' >> have_unistd.h
	${Q}echo '' >> have_unistd.h
	${Q}echo '#if !defined(_HAVE_UNISTD_H_)' >> have_unistd.h
	${Q}echo '#define _HAVE_UNISTD_H_' >> have_unistd.h
	${Q}echo '' >> have_unistd.h
	${Q}echo '/* do we have /usr/include/unistd.h? */' >> have_unistd.h
	-${Q}if [ -f /usr/include/unistd.h ]; then \
	    echo '#define HAVE_UNISTD_H  /* yes */' >> have_unistd.h; \
	else \
	    echo '#undef HAVE_UNISTD_H  /* no */' >> have_unistd.h; \
	fi
	${Q}echo '' >> have_unistd.h
	${Q}echo '#endif /* _HAVE_UNISTD_H_ */' >> have_unistd.h
	${Q}echo 'have_unistd.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_string.h: ${MAKE_FILE}
	-${Q}rm -f have_string.h
	${Q}echo 'forming have_string.h'
	${Q}echo '/*' > have_string.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_string.h
	${Q}echo ' */' >> have_string.h
	${Q}echo '' >> have_string.h
	${Q}echo '#if !defined(_HAVE_STRING_H_)' >> have_string.h
	${Q}echo '#define _HAVE_STRING_H_' >> have_string.h
	${Q}echo '' >> have_string.h
	${Q}echo '/* do we have /usr/include/string.h? */' >> have_string.h
	-${Q}if [ -f /usr/include/string.h ]; then \
	    echo '#define HAVE_STRING_H  /* yes */' >> have_string.h; \
	else \
	    echo '#undef HAVE_STRING_H  /* no */' >> have_string.h; \
	fi
	${Q}echo '' >> have_string.h
	${Q}echo '#endif /* _HAVE_STRING_H_ */' >> have_string.h
	${Q}echo 'have_string.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

terminal.h: ${MAKE_FILE}
	-${Q}rm -f terminal.h
	${Q}echo 'forming terminal.h'
	${Q}echo '/*' > terminal.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> terminal.h
	${Q}echo ' */' >> terminal.h
	${Q}echo '' >> terminal.h
	${Q}echo '#if !defined(_TERMINAL_H_)' >> terminal.h
	${Q}echo '#define _TERMINAL_H_' >> terminal.h
	${Q}echo '' >> terminal.h
	${Q}echo '/* determine the type of terminal interface */' >> terminal.h
	${Q}echo '#if !defined(USE_TERMIOS)' >> terminal.h
	${Q}echo '#if !defined(USE_TERMIO)' >> terminal.h
	${Q}echo '#if !defined(USE_SGTTY)' >> terminal.h
	-${Q}if [ -f /usr/include/termios.h ]; then \
	    echo '#define USE_TERMIOS  /* <termios.h> */' >> terminal.h; \
	    echo '#undef USE_TERMIO    /* <termio.h> */' >> terminal.h; \
	    echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	elif [ -f /usr/include/termio.h ]; then \
	    echo '#undef USE_TERMIOS   /* <termios.h> */' >> terminal.h; \
	    echo '#define USE_TERMIO   /* <termio.h> */' >> terminal.h; \
	    echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	else \
	    echo '#undef USE_TERMIOS   /* <termios.h> */' >> terminal.h; \
	    echo '#undef USE_TERMIO    /* <termio.h> */' >> terminal.h; \
	    echo '#define USE_SGTTY    /* <sys/ioctl.h> */' >> terminal.h; \
	fi
	${Q}echo '#endif' >> terminal.h
	${Q}echo '#endif' >> terminal.h
	${Q}echo '#endif' >> terminal.h
	${Q}echo '' >> terminal.h
	${Q}echo '#endif /* _TERMINAL_H_ */' >> terminal.h
	${Q}echo 'terminal.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

longlong.h: longlong.c have_stdlib.h have_string.h ${MAKE_FILE}
	-${Q}rm -f longlong longlong.o ll_tmp longlong.h
	${Q}echo 'forming longlong.h'
	${Q}echo '/*' > longlong.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> longlong.h
	${Q}echo ' */' >> longlong.h
	${Q}echo '' >> longlong.h
	${Q}echo '#if !defined(_LONGLONG_H_)' >> longlong.h
	${Q}echo '#define _LONGLONG_H_' >> longlong.h
	${Q}echo '' >> longlong.h
	${Q}echo '/* do we have/want to use a long long type? */' >> longlong.h
	-${Q}rm -f longlong.o longlong
	-${Q}${CC} ${CCMAIN} longlong.c -c 2>/dev/null; true
	-${Q}${CC} ${ILDFLAGS} longlong.o -o longlong 2>/dev/null; true
	-${Q}${SHELL} -c "./longlong ${LONGLONG_BITS} > ll_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s ll_tmp ]; then \
	    cat ll_tmp >> longlong.h; \
	else \
	    echo '#undef HAVE_LONGLONG' >> longlong.h; \
	    echo '#define LONGLONG_BITS 0  /* no */' >> longlong.h; \
	fi
	${Q}echo '' >> longlong.h
	${Q}echo '#endif /* _LONGLONG_H_ */' >> longlong.h
	-${Q}rm -f longlong longlong.o ll_tmp
	${Q}echo 'longlong.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_fpos.h: have_fpos.c ${MAKE_FILE}
	-${Q}rm -f have_fpos have_fpos.o fpos_tmp have_fpos.h
	${Q}echo 'forming have_fpos.h'
	${Q}echo '/*' > have_fpos.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_fpos.h
	${Q}echo ' */' >> have_fpos.h
	${Q}echo '' >> have_fpos.h
	${Q}echo '#if !defined(_HAVE_FPOS_H_)' >> have_fpos.h
	${Q}echo '#define _HAVE_FPOS_H_' >> have_fpos.h
	${Q}echo '' >> have_fpos.h
	${Q}echo '/* do we have fgetpos & fsetpos functions? */' >> have_fpos.h
	-${Q}rm -f have_fpos.o have_fpos
	-${Q}${CC} ${HAVE_FPOS} ${CCMAIN} have_fpos.c -c 2>/dev/null; true
	-${Q}${CC} ${ILDFLAGS} have_fpos.o -o have_fpos 2>/dev/null; true
	-${Q}${SHELL} -c "./have_fpos > fpos_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s fpos_tmp ]; then \
	    cat fpos_tmp >> have_fpos.h; \
	else \
	    echo '#undef HAVE_FPOS  /* no */' >> have_fpos.h; \
	    echo '' >> have_fpos.h; \
	    echo 'typedef long FILEPOS;' >> have_fpos.h; \
	fi
	${Q}echo '' >> have_fpos.h
	${Q}echo '#endif /* _HAVE_FPOS_H_ */' >> have_fpos.h
	-${Q}rm -f have_fpos have_fpos.o fpos_tmp
	${Q}echo 'have_fpos.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

fposval.h: fposval.c have_fpos.h endian_calc.h ${MAKE_FILE}
	-${Q}rm -f fposv_tmp fposval fposval.o fposval.h
	${Q}echo 'forming fposval.h'
	${Q}echo '/*' > fposval.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> fposval.h
	${Q}echo ' */' >> fposval.h
	${Q}echo '' >> fposval.h
	${Q}echo '#if !defined(_FPOSVAL_H_)' >> fposval.h
	${Q}echo '#define _FPOSVAL_H_' >> fposval.h
	${Q}echo '' >> fposval.h
	${Q}echo '/* what are our file position & size types? */' >> fposval.h
	-${Q}rm -f fposval.o fposval
	-${Q}${CC} ${CCMAIN} fposval.c -c 2>/dev/null; true
	-${Q}${CC} ${ILDFLAGS} fposval.o -o fposval 2>/dev/null; true
	${Q}${SHELL} -c "./fposval fposv_tmp >> fposval.h 2>/dev/null" \
	    >/dev/null 2>&1; true
	${Q}echo '' >> fposval.h
	${Q}echo '#endif /* _FPOSVAL_H_ */' >> fposval.h
	-${Q}rm -f fposval fposval.o fposv_tmp
	${Q}echo 'fposval.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_const.h: have_const.c ${MAKE_FILE}
	-${Q}rm -f have_const have_const.o const_tmp have_const.h
	${Q}echo 'forming have_const.h'
	${Q}echo '/*' > have_const.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_const.h
	${Q}echo ' */' >> have_const.h
	${Q}echo '' >> have_const.h
	${Q}echo '#if !defined(_HAVE_CONST_H_)' >> have_const.h
	${Q}echo '#define _HAVE_CONST_H_' >> have_const.h
	${Q}echo '' >> have_const.h
	${Q}echo '/* do we have or want const? */' >> have_const.h
	-${Q}rm -f have_const.o have_const
	-${Q}${CC} ${CCMAIN} ${HAVE_CONST} have_const.c -c 2>/dev/null; true
	-${Q}${CC} ${ILDFLAGS} have_const.o -o have_const 2>/dev/null; true
	-${Q}${SHELL} -c "./have_const > const_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s const_tmp ]; then \
	    cat const_tmp >> have_const.h; \
	else \
	    echo '#undef HAVE_CONST /* no */' >> have_const.h; \
	    echo '#undef CONST' >> have_const.h; \
	    echo '#define CONST /* no */' >> have_const.h; \
	    echo '' >> have_const.h; \
	fi
	${Q}echo '' >> have_const.h
	${Q}echo '#endif /* _HAVE_CONST_H_ */' >> have_const.h
	-${Q}rm -f have_const have_const.o const_tmp
	${Q}echo 'have_const.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

align32.h: align32.c longbits.h have_unistd.h ${MAKE_FILE}
	-${Q}rm -f align32 align32.o align32_tmp align32.h
	${Q}echo 'forming align32.h'
	${Q}echo '/*' > align32.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> align32.h
	${Q}echo ' */' >> align32.h
	${Q}echo '' >> align32.h
	${Q}echo '#if !defined(_MUST_ALIGN32_H_)' >> align32.h
	${Q}echo '#define _MUST_ALIGN32_H_' >> align32.h
	${Q}echo '' >> align32.h
	${Q}echo '/* must we always align 32 bit accesses? */' >> align32.h
	-${Q}if [ X"-DMUST_ALIGN32" = X${ALIGN32} ]; then \
	    echo '/* forced to align 32 bit values */' >> align32.h; \
	    echo '#define MUST_ALIGN32' >> align32.h; \
	else \
	    true; \
	fi
	-${Q}if [ X"-UMUST_ALIGN32" = X${ALIGN32} ]; then \
	    echo '/* forced to not require 32 bit alignment */' >> align32.h; \
	    echo '#undef MUST_ALIGN32' >> align32.h; \
	else \
	    true; \
	fi
	-${Q}if [ X = X${ALIGN32} ]; then \
	    rm -f align32.o align32; \
	    ${CC} ${CCMAIN} ${ALIGN32} align32.c -c 2>/dev/null; \
	    ${CC} ${ILDFLAGS} align32.o -o align32 2>/dev/null; \
	    ${SHELL} -c "./align32 >align32_tmp 2>/dev/null" >/dev/null 2>&1; \
	    if [ -s align32_tmp ]; then \
		cat align32_tmp >> align32.h; \
	    else \
		echo '/* guess we must align 32 bit values */' >> align32.h; \
		echo '#define MUST_ALIGN32' >> align32.h; \
	    fi; \
	    rm -f align32 align32.o align32_tmp core; \
	else \
	    true; \
	fi
	${Q}echo '' >> align32.h
	${Q}echo '#endif /* _MUST_ALIGN32_H_ */' >> align32.h
	${Q}echo 'align32.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_uid_t.h: have_uid_t.c have_unistd.h ${MAKE_FILE}
	-${Q}rm -f have_uid_t have_uid_t.o uid_tmp have_uid_t.h
	${Q}echo 'forming have_uid_t.h'
	${Q}echo '/*' > have_uid_t.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_uid_t.h
	${Q}echo ' */' >> have_uid_t.h
	${Q}echo '' >> have_uid_t.h
	${Q}echo '#if !defined(_HAVE_UID_T_H_)' >> have_uid_t.h
	${Q}echo '#define _HAVE_UID_T_H_' >> have_uid_t.h
	${Q}echo '' >> have_uid_t.h
	${Q}echo '/* do we have or want uid_t? */' >> have_uid_t.h
	-${Q}rm -f have_uid_t.o have_uid_t
	-${Q}${CC} ${CCMAIN} ${HAVE_UID_T} have_uid_t.c -c 2>/dev/null; true
	-${Q}${CC} ${ILDFLAGS} have_uid_t.o -o have_uid_t 2>/dev/null; true
	-${Q}${SHELL} -c "./have_uid_t > uid_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s uid_tmp ]; then \
	    cat uid_tmp >> have_uid_t.h; \
	else \
	    echo '#undef HAVE_UID_T /* no */' >> have_uid_t.h; \
	    echo '' >> have_uid_t.h; \
	fi
	${Q}echo '' >> have_uid_t.h
	${Q}echo '#endif /* _HAVE_UID_T_H_ */' >> have_uid_t.h
	-${Q}rm -f have_uid_t have_uid_t.o uid_tmp
	${Q}echo 'have_uid_t.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_newstr.h: have_newstr.c ${MAKE_FILE}
	-${Q}rm -f have_newstr have_newstr.o newstr_tmp have_newstr.h
	${Q}echo 'forming have_newstr.h'
	${Q}echo '/*' > have_newstr.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_newstr.h
	${Q}echo ' */' >> have_newstr.h
	${Q}echo '' >> have_newstr.h
	${Q}echo '#if !defined(_HAVE_NEWSTR_H_)' >> have_newstr.h
	${Q}echo '#define _HAVE_NEWSTR_H_' >> have_newstr.h
	${Q}echo '' >> have_newstr.h
	${Q}echo '/* do we have or want memcpy(), memset() & strchr()? */' \
							>> have_newstr.h
	-${Q}rm -f have_newstr.o have_newstr
	-${Q}${CC} ${CCMAIN} ${HAVE_NEWSTR} have_newstr.c -c 2>/dev/null; true
	-${Q}${CC} ${ILDFLAGS} have_newstr.o -o have_newstr 2>/dev/null; true
	-${Q}${SHELL} -c "./have_newstr > newstr_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s newstr_tmp ]; then \
	    cat newstr_tmp >> have_newstr.h; \
	else \
	    echo '#undef HAVE_NEWSTR /* no */' >> have_newstr.h; \
	    echo '' >> have_newstr.h; \
	fi
	${Q}echo '' >> have_newstr.h
	${Q}echo '#endif /* _HAVE_NEWSTR_H_ */' >> have_newstr.h
	-${Q}rm -f have_newstr have_newstr.o newstr_tmp
	${Q}echo 'have_newstr.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

args.h: have_stdvs.c have_varvs.c have_string.h have_unistd.h have_string.h
	-${Q}rm -f args.h have_args
	${Q}echo 'forming args.h'
	${Q}echo '/*' > args.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> args.h
	${Q}echo ' */' >> args.h
	${Q}echo '' >> args.h
	${Q}echo '#if !defined(_ARGS_H_)' >> args.h
	${Q}echo '#define _ARGS_H_' >> args.h
	${Q}echo '' >> args.h
	-${Q}rm -f have_stdvs.o have_stdvs
	-${Q}${CC} ${CCMAIN} ${HAVE_VSPRINTF} have_stdvs.c -c 2>/dev/null; true
	-${Q}${CC} ${ILDFLAGS} have_stdvs.o -o have_stdvs 2>/dev/null; true
	-${Q}if ./have_stdvs >>args.h 2>/dev/null; then \
	    touch have_args; \
	else \
	    true; \
	fi
	-${Q}if [ ! -f have_args ] && [ X"${HAVE_VSPRINTF}" = X ]; then \
	    rm -f have_stdvs.o have_stdvs have_varvs.o have_varvs; \
	    ${CC} ${CCMAIN} -DDONT_HAVE_VSPRINTF have_varvs.c -c 2>/dev/null; \
	    ${CC} ${ILDFLAGS} have_varvs.o -o have_varvs 2>/dev/null; \
	    if ./have_varvs >>args.h 2>/dev/null; then \
		touch have_args; \
	    else \
		true; \
	    fi; \
	else \
	    true; \
	fi
	-${Q}if [ -f have_args ]; then \
	    echo 'exit 0' > have_args; \
	else \
	    echo 'exit 1' > have_args; \
	    echo "Unable to determine what type of variable args and"; \
	    echo "what type of vsprintf() should be used.  Set or change"; \
	    echo "the Makefile variable HAVE_VSPRINTF."; \
	fi
	${Q}sh ./have_args
	${Q}echo '' >> args.h
	${Q}echo '#endif /* _ARGS_H_ */' >> args.h
	-${Q}rm -f have_stdvs.o have_varvs.o have_stdvs
	-${Q}rm -f have_varvs have_args core
	${Q}echo 'args.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

calcerr.h: calcerr.tbl calcerr_h.sed calcerr_h.awk ${MAKE_FILE}
	-${Q}rm -f calerr.h
	${Q}echo 'forming calcerr.h'
	${Q}echo '/*' > calcerr.h
	${Q}echo ' * DO NOT EDIT' >> calcerr.h
	${Q}echo ' *' >> calcerr.h
	${Q}echo ' * generated by calcerr.tbl via Makefile' >> calcerr.h
	${Q}echo ' */' >> calcerr.h
	${Q}echo '' >> calcerr.h
	${Q}echo '#if !defined(_CALCERR_H_)' >> calcerr.h
	${Q}echo '#define _CALCERR_H_' >> calcerr.h
	${Q}echo '' >> calcerr.h
	${Q}${SED} -f calcerr_h.sed < calcerr.tbl | \
	    ${AWK} -f calcerr_h.awk >> calcerr.h
	${Q}echo '' >> calcerr.h
	${Q}echo '#endif /* _CALCERR_H_ */' >> calcerr.h
	${Q}echo 'calcerr.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

calcerr.c: calcerr.tbl calcerr_c.sed calcerr_c.awk ${MAKE_FILE}
	-${Q}rm -f calerr.c
	${Q}echo 'forming calcerr.c'
	${Q}echo '/*' > calcerr.c
	${Q}echo ' * DO NOT EDIT' >> calcerr.c
	${Q}echo ' *' >> calcerr.c
	${Q}echo ' * generated by calcerr.tbl via Makefile' >> calcerr.c
	${Q}echo ' */' >> calcerr.c
	${Q}echo '' >> calcerr.c
	${Q}${SED} -f calcerr_c.sed < calcerr.tbl | \
	    ${AWK} -f calcerr_c.awk >> calcerr.c
	${Q}echo 'calcerr.c formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

##
#
# These rules are used in the process of building the BUILD_H_SRC.
#
##

endian.o: endian.c have_unistd.h
	${CC} ${CCMAIN} endian.c -c

endian: endian.o
	${CC} ${ILDFLAGS} endian.o -o endian

longbits.o: longbits.c longlong.h have_unistd.h
	${CC} ${CCMAIN} longbits.c -c

longbits: longbits.o
	${CC} ${ILDFLAGS} longbits.o -o longbits

##
#
# These two .all rules are used to determine of the lower level
# directory has had its all rule performed.
#
##

lib/.all:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for lib =-=-=-=-='
	cd lib; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

help/.all:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for help =-=-=-=-='
	cd help; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

help/builtin: func.c help/builtin.top help/builtin.end help/funclist.sed
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking builtin rule for help =-=-=-=-='
	cd help; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} builtin
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

##
#
# The BSDI cdrom makefile expects certain files to be pre-built in a sub-dir
# called gen_h.  This rule creats this sub-directory so that the release can
# be shipped off to BSDI.  You can ignore this rule.
#
##

bsdi: ${LIB_H_SRC} ${BUILD_H_SRC} calc.1
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	-${Q}if [ ! -d gen_h ]; then \
		echo mkdir gen_h; \
		mkdir gen_h; \
	else \
	    true; \
	fi
	-${Q}for i in ${LIB_H_SRC} ${BUILD_H_SRC}; do \
		echo rm -f gen_h/$$i; \
		rm -f gen_h/$$i; \
		echo cp $$i gen_h; \
		cp $$i gen_h; \
		echo chmod 0444 gen_h/$$i; \
		chmod 0444 gen_h/$$i; \
	done
	cd help; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} bsdi
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

##
#
# These rules help with linting.  Adjust ${LINT}, ${LINTLIB}, ${LINTFLAGS}
# and the lint.sed file as needed for your system.
#
##

llib-lcalc.ln: ${BUILD_H_SRC} ${LIBSRC} ${MAKE_FILE}
	-rm -f llib-lcalc.ln llib.out
	-touch llib-lcalc.ln
	${LINTLIB} ${LIBSRC} 2>&1 | ${SED} -f lint.sed | ${TEE} llib.out

lint: ${BUILD_H_SRC} ${CALCSRC} llib-lcalc.ln lint.sed ${MAKE_FILE}
	-rm -f lint.out
	${LINT} ${LINTFLAGS} ${LCFLAGS} llib-lcalc.ln ${CALCSRC} 2>&1 | \
	    ${SED} -f lint.sed | ${TEE} lint.out

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

depend: hsrc
	${Q}if [ -f Makefile.bak ]; then \
		echo "Makefile.bak exists, remove or move it out of the way"; \
		exit 1; \
	else \
	    true; \
	fi
	${Q}echo forming skel
	-${Q}rm -rf skel
	${Q}mkdir skel
	-${Q}for i in ${C_SRC} ${BUILD_C_SRC}; do \
		${SED} -n '/^#[	 ]*include[ 	]*"/p' "$$i" > "skel/$$i"; \
	done
	-${Q}for i in ${H_SRC} ${BUILD_H_SRC}; do \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#ifndef $$tag" > "skel/$$i"; \
		echo "#define $$tag" >> "skel/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" >> "skel/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/$$i"; \
	done
	-${Q}rm -f skel/makedep.out
	${Q}echo skel formed
	${Q}echo forming dependency list
	${Q}echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > \
	    skel/makedep.out
	${Q}cd skel; \
	    ${MAKEDEPEND} -w 1 -m -f makedep.out ${C_SRC} ${BUILD_C_SRC}
	-${Q}for i in ${C_SRC} ${BUILD_C_SRC}; do \
		echo "$$i" | \
		    ${SED} 's/^\(.*\)\.c/\1.o: \1.c/' >> skel/makedep.out; \
	done
	${Q}echo dependency list formed
	${Q}echo forming new Makefile
	-${Q}rm -f Makefile.bak
	${Q}mv Makefile Makefile.bak
	${Q}${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' Makefile.bak > Makefile
	${Q}echo "" >> Makefile
	${Q}${SED} -n '3,$$p' skel/makedep.out | ${SORT} -u >> Makefile
	-${Q}rm -rf skel
	${Q}echo new Makefile formed

##
#
# File distribution list generation.  You can ignore this section.
#
# We will form the names of source files as if they were in a
# sub-directory called calc.
#
##

distlist: ${DISTLIST}
	${Q}(for i in ${DISTLIST}; do \
		echo calc/$$i; \
	done; \
	(cd help; ${MAKE} distlist \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} SORT=${SORT}); \
	(cd lib; ${MAKE} distlist \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} SORT=${SORT}) ) | ${SORT}

# The bsdi distribution has generated files as well as distributed files.
# The the .h files are placed under calc/gen_h.
#
bsdilist: ${DISTLIST} ${BUILD_H_SRC} calc.1
	${Q}(for i in ${DISTLIST}; do \
		echo calc/$$i; \
	done; \
	for i in ${BUILD_H_SRC}; do \
		echo calc/gen_h/$$i; \
	done; \
	echo calc/calc.1; \
	(cd help; ${MAKE} bsdilist \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} SORT=${SORT}); \
	(cd lib; ${MAKE} bsdilist \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} SORT=${SORT}) ) | ${SORT}

##
#
# debug
#
# make env:
#	* print major makefile variables
#
# make mkdebug:
#	* print major makefile variables
#	* build anything not yet built
#
# make debug:
#	* remove everything that was previously built
#	* print major makefile variables
#	* make everything
#	* run the regression tests
##

env:
	@echo '=-=-=-=-= dumping major make variables =-=-=-=-='
	@echo "TERMCONTROL=${TERMCONTROL}"; echo ""
	@echo "HAVE_VSPRINTF=${HAVE_VSPRINTF}"; echo ""
	@echo "BYTE_ORDER=${BYTE_ORDER}"; echo ""
	@echo "LONG_BITS=${LONG_BITS}"; echo ""
	@echo "LONGLONG_BITS=${LONGLONG_BITS}"; echo ""
	@echo "HAVE_FPOS=${HAVE_FPOS}"; echo ""
	@echo "HAVE_CONST=${HAVE_CONST}"; echo ""
	@echo "HAVE_UID_T=${HAVE_UID_T}"; echo ""
	@echo "HAVE_NEWSTR=${HAVE_NEWSTR}"; echo ""
	@echo "ALIGN32=${ALIGN32}"; echo ""
	@echo "BINDIR=${BINDIR}"; echo ""
	@echo "TOPDIR=${TOPDIR}"; echo ""
	@echo "LIBDIR=${LIBDIR}"; echo ""
	@echo "HELPDIR=${HELPDIR}"; echo ""
	@echo "MANDIR=${MANDIR}"; echo ""
	@echo "CATDIR=${CATDIR}"; echo ""
	@echo "MANEXT=${MANEXT}"; echo ""
	@echo "CATEXT=${CATEXT}"; echo ""
	@echo "NROFF=${NROFF}"; echo ""
	@echo "NROFF_ARG=${NROFF_ARG}"; echo ""
	@echo "MANMAKE=${MANMAKE}"; echo ""
	@echo "CALCPATH=${CALCPATH}"; echo ""
	@echo "CALCRC=${CALCRC}"; echo ""
	@echo "CALCBINDINGS=${CALCBINDINGS}"; echo ""
	@echo "CALCPAGER=${CALCPAGER}"; echo ""
	@echo "DEBUG=${DEBUG}"; echo ""
	@echo "NO_SHARED=${NO_SHARED}"; echo ""
	@echo "LD_NO_SHARED=${LD_NO_SHARED}"; echo ""
	@echo "RANLIB=${RANLIB}"; echo ""
	@echo "LINTLIB=${LINTLIB}"; echo ""
	@echo "LINTFLAGS=${LINTFLAGS}"; echo ""
	@echo "MAKE_FILE=${MAKE_FILE}"; echo ""
	@echo "CCMAIN=${CCMAIN}"; echo ""
	@echo "CCWARN=${CCWARN}"; echo ""
	@echo "CCOPT=${CCOPT}"; echo ""
	@echo "CCMISC=${CCMISC}"; echo ""
	@echo "CCSHS=${CCSHS}"; echo ""
	@echo "CFLAGS=${CFLAGS}"; echo ""
	@echo "CNOWARN=${CNOWARN}"; echo ""
	@echo "ICFLAGS=${ICFLAGS}"; echo ""
	@echo "LCFLAGS=${LCFLAGS}"; echo ""
	@echo "LDFLAGS=${LDFLAGS}"; echo ""
	@echo "ILDFLAGS=${ILDFLAGS}"; echo ""
	@echo "CC=${CC}"; echo ""
	@echo "SHELL=${SHELL}"; echo ""
	@echo "MAKE=${MAKE}"; echo ""
	@echo "AWK=${AWK}"; echo ""
	@echo "SED=${SED}"; echo ""
	@echo "SORT=${SORT}"; echo ""
	@echo "TEE=${TEE}"; echo ""
	@echo "LINT=${LINT}"; echo ""
	@echo "CTAGS=${CTAGS}"; echo ""
	@echo "MAKEDEPEND=${MAKEDEPEND}"; echo ""
	@echo "Q=${Q}"; echo ""
	@echo "V=${V}"; echo ""
	@echo "LIBSRC=${LIBSRC}"; echo ""
	@echo "LIBOBJS=${LIBOBJS}"; echo ""
	@echo "CALCSRC=${CALCSRC}"; echo ""
	@echo "CALCOBJS=${CALCOBJS}"; echo ""
	@echo "BUILD_H_SRC=${BUILD_H_SRC}"; echo ""
	@echo "BUILD_C_SRC=${BUILD_C_SRC}"; echo ""
	@echo "UTIL_C_SRC=${UTIL_C_SRC}"; echo ""
	@echo "UTIL_MISC_SRC=${UTIL_MISC_SRC}"; echo ""
	@echo "UTIL_OBJS=${UTIL_OBJS}"; echo ""
	@echo "UTIL_TMP=${UTIL_TMP}"; echo ""
	@echo "UTIL_PROGS=${UTIL_PROGS}"; echo ""
	@echo "LIB_H_SRC=${LIB_H_SRC}"; echo ""
	@echo "CALC_H_SRC=${CALC_H_SRC}"; echo ""
	@echo "H_SRC=${H_SRC}"; echo ""
	@echo "C_SRC=${C_SRC}"; echo ""
	@echo "DISTLIST=${DISTLIST}"; echo ""
	@echo "OBJS=${OBJS}"; echo ""
	@echo "PROGS=${PROGS}"; echo ""
	@echo "TARGETS=${TARGETS}"; echo ""
	@echo '=-=-=-=-= end of major make variable dump =-=-=-=-='

mkdebug: env version.c
	@echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Determining the source version =-=-=-=-='
	@${SED} -n '/^#[	 ]*define/p' version.c
	@echo '=-=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ all =-=-=-=-='
	@${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} Q= V=@ all
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Determining the binary version =-=-=-=-='
	-@./calc -v
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= end of $@ rule =-=-=-=-='

debug: env
	@echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ clobber =-=-=-=-='
	@${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} Q= V=@ clobber
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Determining the source version =-=-=-=-='
	@${SED} -n '/^#[	 ]*define/p' version.c
	@echo '=-=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ all =-=-=-=-='
	@${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} Q= V=@ all
	@echo '=-=-=-=-= Determining the binary version =-=-=-=-='
	-@./calc -v
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ chk =-=-=-=-='
	@echo '=-=-=-=-= this may take a while =-=-=-=-='
	@${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} Q= V=@ chk
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= end of $@ rule =-=-=-=-='

##
#
# Utility rules
#
##

tags: ${CALCSRC} ${LIBSRC} ${H_SRC} ${BUILD_H_SRC} ${MAKE_FILE}
	${CTAGS} ${CALCSRC} ${LIBSRC} ${H_SRC} ${BUILD_H_SRC}

lintclean:
	-rm -f llib-lcalc.ln llib.out lint.out

clean:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	-rm -f ${LIBOBJS}
	-rm -f ${CALCOBJS}
	-rm -f ${UTIL_OBJS}
	-rm -f ${UTIL_TMP}
	-rm -f ${UTIL_PROGS}
	${Q}echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	-cd help; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} clean
	${Q}echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${Q}echo '=-=-=-=-= Invoking $@ rule for lib =-=-=-=-='
	-cd lib; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} clean
	${Q}echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	-rm -f funclist.o funclist.c
	${Q}echo remove files that are obsolete
	-rm -f endian.h stdarg.h libcalcerr.a lib/obj help/obj
	-rm -f have_vs.c std_arg.h try_stdarg.c fnvhash.c
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

clobber: lintclean
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	-rm -f ${LIBOBJS}
	-rm -f ${CALCOBJS}
	-rm -f ${UTIL_OBJS}
	-rm -f ${UTIL_TMP}
	-rm -f ${UTIL_PROGS}
	-rm -f tags
	-rm -f ${BUILD_H_SRC}
	-rm -f ${BUILD_C_SRC}
	-rm -f calc *_pure_*.[oa]
	-rm -f libcalc.a *.pure_hardlink
	-rm -f calc.1 *.pure_linkinfo
	-rm -f have_args *.u
	-rm -f calc.pixie calc.rf calc.Counts calc.cord
	-rm -rf gen_h skel Makefile.bak
	${V} echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	-cd help;${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} clobber
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for lib =-=-=-=-='
	-cd lib; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} clobber
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo remove files that are obsolete
	-rm -f endian.h stdarg.h libcalcerr.a lib/obj help/obj
	-rm -f have_vs.c std_arg.h try_stdarg.c fnvhash.c
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

install: calc libcalc.a ${LIB_H_SRC} ${BUILD_H_SRC} calc.1
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	-${Q}if [ ! -d ${TOPDIR} ]; then \
		echo mkdir ${TOPDIR}; \
		mkdir ${TOPDIR}; \
	else \
	    true; \
	fi
	-chmod 0755 ${TOPDIR}
	-${Q}if [ ! -d ${LIBDIR} ]; then \
		echo mkdir ${LIBDIR}; \
		mkdir ${LIBDIR}; \
	else \
	    true; \
	fi
	-chmod 0755 ${LIBDIR}
	-${Q}if [ ! -d ${HELPDIR} ]; then \
		echo mkdir ${HELPDIR}; \
		mkdir ${HELPDIR}; \
	else \
	    true; \
	fi
	-chmod 0755 ${HELPDIR}
	-${Q}if [ ! -d ${BINDIR} ]; then \
		echo mkdir ${BINDIR}; \
		mkdir ${BINDIR}; \
	else \
	    true; \
	fi
	-chmod 0755 ${BINDIR}
	-rm -f ${BINDIR}/calc
	cp calc ${BINDIR}
	-chmod 0555 ${BINDIR}/calc
	${V} echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	cd help; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for lib =-=-=-=-='
	cd lib; ${MAKE} -f Makefile \
	    MAKE_FILE=${MAKE_FILE} TOPDIR=${TOPDIR} LIBDIR=${LIBDIR} \
	    HELPDIR=${HELPDIR} install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	-rm -f ${LIBDIR}/libcalc.a
	cp libcalc.a ${LIBDIR}/libcalc.a
	-chmod 0644 ${LIBDIR}/libcalc.a
	${RANLIB} ${LIBDIR}/libcalc.a
	-${Q}for i in ${LIB_H_SRC} ${BUILD_H_SRC}; do \
		echo rm -f ${LIBDIR}/$$i; \
		rm -f ${LIBDIR}/$$i; \
		echo cp $$i ${LIBDIR}; \
		cp $$i ${LIBDIR}; \
		echo chmod 0444 ${LIBDIR}/$$i; \
		chmod 0444 ${LIBDIR}/$$i; \
	done
	${Q}: If lint was made, install the lint library.
	-${Q}if [ -f llib-lcalc.ln ]; then \
		echo rm -f ${LIBDIR}/llib-lcalc.ln; \
		rm -f ${LIBDIR}/llib-lcalc.ln; \
		echo cp llib-lcalc.ln ${LIBDIR}; \
		cp llib-lcalc.ln ${LIBDIR}; \
		echo chmod 0444 ${LIBDIR}/llib-lcalc.ln; \
		chmod 0444 ${LIBDIR}/llib-lcalc.ln; \
	else \
	    true; \
	fi
	-${Q}if [ -z "${MANDIR}" ]; then \
	    echo '$${MANDIR} is empty, calc man page will not be installed'; \
	else \
	    echo "rm -f ${MANDIR}/calc.${MANEXT}"; \
	    rm -f ${MANDIR}/calc.${MANEXT}; \
	    echo "cp calc.1 ${MANDIR}/calc.${MANEXT}"; \
	    cp calc.1 ${MANDIR}/calc.${MANEXT}; \
	    echo "chmod 0444 ${MANDIR}/calc.${MANEXT}"; \
	    chmod 0444 ${MANDIR}/calc.${MANEXT}; \
	fi
	-${Q}if [ -z "${CATDIR}" ]; then \
	    echo '$${CATDIR} is empty, calc cat page will not be installed'; \
	else \
	    if [ -z "${NROFF}" ]; then \
		echo "${MANMAKE} calc.1 ${CATDIR}"; \
		${MANMAKE} calc.1 ${CATDIR}; \
	    else \
		echo "rm -f ${CATDIR}/calc.${CATEXT}"; \
		rm -f ${CATDIR}/calc.${CATEXT}; \
		echo "${NROFF} ${NROFF_ARG} calc.1 > ${CATDIR}/calc.${CATEXT}";\
		${NROFF} ${NROFF_ARG} calc.1 > ${CATDIR}/calc.${CATEXT}; \
		echo "chmod ${MANMODE} ${MANDIR}/calc.${MANEXT}"; \
		chmod ${MANMODE} ${MANDIR}/calc.${MANEXT}; \
	    fi; \
	fi
	${Q}echo remove files that are obsolete
	-rm -f ${LIBDIR}/endian.h endian.h
	-rm -f ${LIBDIR}/stdarg.h stdarg.h
	-rm -f ${LIBDIR}/prototype.h prototype.h
	-rm -f ${LIBDIR}/libcalcerr.a libcalcerr.a
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

##
#
# make depend stuff
#
##

# DO NOT DELETE THIS LINE -- make depend depends on it.

addop.o: addop.c
addop.o: alloc.h
addop.o: byteswap.h
addop.o: calc.h
addop.o: calcerr.h
addop.o: cmath.h
addop.o: config.h
addop.o: endian_calc.h
addop.o: func.h
addop.o: hash.h
addop.o: have_malloc.h
addop.o: have_newstr.h
addop.o: have_stdlib.h
addop.o: have_string.h
addop.o: label.h
addop.o: longbits.h
addop.o: opcodes.h
addop.o: qmath.h
addop.o: shs.h
addop.o: string.h
addop.o: symbol.h
addop.o: token.h
addop.o: value.h
addop.o: zmath.h
align32.o: align32.c
align32.o: have_unistd.h
align32.o: longbits.h
assocfunc.o: alloc.h
assocfunc.o: assocfunc.c
assocfunc.o: byteswap.h
assocfunc.o: calcerr.h
assocfunc.o: cmath.h
assocfunc.o: config.h
assocfunc.o: endian_calc.h
assocfunc.o: hash.h
assocfunc.o: have_malloc.h
assocfunc.o: have_newstr.h
assocfunc.o: have_stdlib.h
assocfunc.o: have_string.h
assocfunc.o: longbits.h
assocfunc.o: qmath.h
assocfunc.o: shs.h
assocfunc.o: value.h
assocfunc.o: zmath.h
byteswap.o: alloc.h
byteswap.o: byteswap.c
byteswap.o: byteswap.h
byteswap.o: cmath.h
byteswap.o: endian_calc.h
byteswap.o: have_malloc.h
byteswap.o: have_newstr.h
byteswap.o: have_stdlib.h
byteswap.o: have_string.h
byteswap.o: longbits.h
byteswap.o: qmath.h
byteswap.o: zmath.h
calc.o: alloc.h
calc.o: byteswap.h
calc.o: calc.c
calc.o: calc.h
calc.o: calcerr.h
calc.o: cmath.h
calc.o: conf.h
calc.o: config.h
calc.o: endian_calc.h
calc.o: func.h
calc.o: hash.h
calc.o: have_malloc.h
calc.o: have_newstr.h
calc.o: have_stdlib.h
calc.o: have_string.h
calc.o: have_uid_t.h
calc.o: have_unistd.h
calc.o: hist.h
calc.o: label.h
calc.o: longbits.h
calc.o: opcodes.h
calc.o: qmath.h
calc.o: shs.h
calc.o: symbol.h
calc.o: token.h
calc.o: value.h
calc.o: zmath.h
calcerr.o: calcerr.c
calcerr.o: calcerr.h
calcerr.o: have_const.h
codegen.o: alloc.h
codegen.o: byteswap.h
codegen.o: calc.h
codegen.o: calcerr.h
codegen.o: cmath.h
codegen.o: codegen.c
codegen.o: conf.h
codegen.o: config.h
codegen.o: endian_calc.h
codegen.o: func.h
codegen.o: hash.h
codegen.o: have_malloc.h
codegen.o: have_newstr.h
codegen.o: have_stdlib.h
codegen.o: have_string.h
codegen.o: have_unistd.h
codegen.o: label.h
codegen.o: longbits.h
codegen.o: opcodes.h
codegen.o: qmath.h
codegen.o: shs.h
codegen.o: string.h
codegen.o: symbol.h
codegen.o: token.h
codegen.o: value.h
codegen.o: zmath.h
comfunc.o: alloc.h
comfunc.o: byteswap.h
comfunc.o: cmath.h
comfunc.o: comfunc.c
comfunc.o: config.h
comfunc.o: endian_calc.h
comfunc.o: have_malloc.h
comfunc.o: have_newstr.h
comfunc.o: have_stdlib.h
comfunc.o: have_string.h
comfunc.o: longbits.h
comfunc.o: qmath.h
comfunc.o: zmath.h
commath.o: alloc.h
commath.o: byteswap.h
commath.o: cmath.h
commath.o: commath.c
commath.o: endian_calc.h
commath.o: have_malloc.h
commath.o: have_newstr.h
commath.o: have_stdlib.h
commath.o: have_string.h
commath.o: longbits.h
commath.o: qmath.h
commath.o: zmath.h
config.o: alloc.h
config.o: byteswap.h
config.o: calc.h
config.o: calcerr.h
config.o: cmath.h
config.o: config.c
config.o: config.h
config.o: endian_calc.h
config.o: hash.h
config.o: have_const.h
config.o: have_malloc.h
config.o: have_newstr.h
config.o: have_stdlib.h
config.o: have_string.h
config.o: longbits.h
config.o: qmath.h
config.o: shs.h
config.o: token.h
config.o: value.h
config.o: zmath.h
config.o: zrand.h
const.o: alloc.h
const.o: byteswap.h
const.o: calc.h
const.o: calcerr.h
const.o: cmath.h
const.o: config.h
const.o: const.c
const.o: endian_calc.h
const.o: hash.h
const.o: have_malloc.h
const.o: have_newstr.h
const.o: have_stdlib.h
const.o: have_string.h
const.o: longbits.h
const.o: qmath.h
const.o: shs.h
const.o: value.h
const.o: zmath.h
endian.o: endian.c
endian.o: have_unistd.h
file.o: alloc.h
file.o: byteswap.h
file.o: calc.h
file.o: calcerr.h
file.o: cmath.h
file.o: config.h
file.o: endian_calc.h
file.o: file.c
file.o: file.h
file.o: fposval.h
file.o: hash.h
file.o: have_fpos.h
file.o: have_malloc.h
file.o: have_newstr.h
file.o: have_stdlib.h
file.o: have_string.h
file.o: longbits.h
file.o: qmath.h
file.o: shs.h
file.o: value.h
file.o: zmath.h
fposval.o: endian_calc.h
fposval.o: fposval.c
fposval.o: have_fpos.h
func.o: alloc.h
func.o: byteswap.h
func.o: calc.h
func.o: calcerr.h
func.o: cmath.h
func.o: config.h
func.o: endian_calc.h
func.o: file.h
func.o: func.c
func.o: func.h
func.o: hash.h
func.o: have_const.h
func.o: have_fpos.h
func.o: have_malloc.h
func.o: have_newstr.h
func.o: have_stdlib.h
func.o: have_string.h
func.o: have_times.h
func.o: have_unistd.h
func.o: label.h
func.o: longbits.h
func.o: opcodes.h
func.o: prime.h
func.o: qmath.h
func.o: shs.h
func.o: string.h
func.o: symbol.h
func.o: token.h
func.o: value.h
func.o: zmath.h
func.o: zrand.h
hash.o: alloc.h
hash.o: byteswap.h
hash.o: calcerr.h
hash.o: cmath.h
hash.o: config.h
hash.o: endian_calc.h
hash.o: hash.c
hash.o: hash.h
hash.o: have_malloc.h
hash.o: have_newstr.h
hash.o: have_stdlib.h
hash.o: have_string.h
hash.o: longbits.h
hash.o: qmath.h
hash.o: shs.h
hash.o: value.h
hash.o: zmath.h
have_const.o: have_const.c
have_fpos.o: have_fpos.c
have_newstr.o: have_newstr.c
have_stdvs.o: have_stdvs.c
have_stdvs.o: have_string.h
have_stdvs.o: have_unistd.h
have_uid_t.o: have_uid_t.c
have_uid_t.o: have_unistd.h
have_varvs.o: have_string.h
have_varvs.o: have_unistd.h
have_varvs.o: have_varvs.c
hist.o: alloc.h
hist.o: byteswap.h
hist.o: calc.h
hist.o: calcerr.h
hist.o: cmath.h
hist.o: config.h
hist.o: endian_calc.h
hist.o: hash.h
hist.o: have_malloc.h
hist.o: have_newstr.h
hist.o: have_stdlib.h
hist.o: have_string.h
hist.o: have_unistd.h
hist.o: hist.c
hist.o: hist.h
hist.o: longbits.h
hist.o: qmath.h
hist.o: shs.h
hist.o: terminal.h
hist.o: value.h
hist.o: zmath.h
input.o: alloc.h
input.o: byteswap.h
input.o: calc.h
input.o: calcerr.h
input.o: cmath.h
input.o: conf.h
input.o: config.h
input.o: endian_calc.h
input.o: hash.h
input.o: have_malloc.h
input.o: have_newstr.h
input.o: have_stdlib.h
input.o: have_string.h
input.o: hist.h
input.o: input.c
input.o: longbits.h
input.o: qmath.h
input.o: shs.h
input.o: value.h
input.o: zmath.h
jump.o: have_const.h
jump.o: jump.c
jump.o: jump.h
label.o: alloc.h
label.o: byteswap.h
label.o: calc.h
label.o: calcerr.h
label.o: cmath.h
label.o: config.h
label.o: endian_calc.h
label.o: func.h
label.o: hash.h
label.o: have_malloc.h
label.o: have_newstr.h
label.o: have_stdlib.h
label.o: have_string.h
label.o: label.c
label.o: label.h
label.o: longbits.h
label.o: opcodes.h
label.o: qmath.h
label.o: shs.h
label.o: string.h
label.o: token.h
label.o: value.h
label.o: zmath.h
lib_calc.o: alloc.h
lib_calc.o: byteswap.h
lib_calc.o: calc.h
lib_calc.o: calcerr.h
lib_calc.o: cmath.h
lib_calc.o: config.h
lib_calc.o: endian_calc.h
lib_calc.o: hash.h
lib_calc.o: have_malloc.h
lib_calc.o: have_newstr.h
lib_calc.o: have_stdlib.h
lib_calc.o: have_string.h
lib_calc.o: lib_calc.c
lib_calc.o: longbits.h
lib_calc.o: qmath.h
lib_calc.o: shs.h
lib_calc.o: value.h
lib_calc.o: zmath.h
listfunc.o: alloc.h
listfunc.o: byteswap.h
listfunc.o: calcerr.h
listfunc.o: cmath.h
listfunc.o: config.h
listfunc.o: endian_calc.h
listfunc.o: hash.h
listfunc.o: have_const.h
listfunc.o: have_malloc.h
listfunc.o: have_newstr.h
listfunc.o: have_stdlib.h
listfunc.o: have_string.h
listfunc.o: listfunc.c
listfunc.o: longbits.h
listfunc.o: qmath.h
listfunc.o: shs.h
listfunc.o: value.h
listfunc.o: zmath.h
listfunc.o: zrand.h
longbits.o: have_unistd.h
longbits.o: longbits.c
longbits.o: longlong.h
longlong.o: have_stdlib.h
longlong.o: have_string.h
longlong.o: longlong.c
matfunc.o: alloc.h
matfunc.o: byteswap.h
matfunc.o: calcerr.h
matfunc.o: cmath.h
matfunc.o: config.h
matfunc.o: endian_calc.h
matfunc.o: hash.h
matfunc.o: have_const.h
matfunc.o: have_malloc.h
matfunc.o: have_newstr.h
matfunc.o: have_stdlib.h
matfunc.o: have_string.h
matfunc.o: longbits.h
matfunc.o: matfunc.c
matfunc.o: qmath.h
matfunc.o: shs.h
matfunc.o: value.h
matfunc.o: zmath.h
matfunc.o: zrand.h
math_error.o: alloc.h
math_error.o: args.h
math_error.o: byteswap.h
math_error.o: calc.h
math_error.o: calcerr.h
math_error.o: cmath.h
math_error.o: config.h
math_error.o: endian_calc.h
math_error.o: hash.h
math_error.o: have_malloc.h
math_error.o: have_newstr.h
math_error.o: have_stdlib.h
math_error.o: have_string.h
math_error.o: longbits.h
math_error.o: math_error.c
math_error.o: qmath.h
math_error.o: shs.h
math_error.o: value.h
math_error.o: zmath.h
obj.o: alloc.h
obj.o: byteswap.h
obj.o: calc.h
obj.o: calcerr.h
obj.o: cmath.h
obj.o: config.h
obj.o: endian_calc.h
obj.o: func.h
obj.o: hash.h
obj.o: have_malloc.h
obj.o: have_newstr.h
obj.o: have_stdlib.h
obj.o: have_string.h
obj.o: label.h
obj.o: longbits.h
obj.o: obj.c
obj.o: opcodes.h
obj.o: qmath.h
obj.o: shs.h
obj.o: string.h
obj.o: symbol.h
obj.o: value.h
obj.o: zmath.h
opcodes.o: alloc.h
opcodes.o: args.h
opcodes.o: byteswap.h
opcodes.o: calc.h
opcodes.o: calcerr.h
opcodes.o: cmath.h
opcodes.o: config.h
opcodes.o: endian_calc.h
opcodes.o: file.h
opcodes.o: func.h
opcodes.o: hash.h
opcodes.o: have_const.h
opcodes.o: have_fpos.h
opcodes.o: have_malloc.h
opcodes.o: have_newstr.h
opcodes.o: have_stdlib.h
opcodes.o: have_string.h
opcodes.o: hist.h
opcodes.o: label.h
opcodes.o: longbits.h
opcodes.o: opcodes.c
opcodes.o: opcodes.h
opcodes.o: qmath.h
opcodes.o: shs.h
opcodes.o: symbol.h
opcodes.o: value.h
opcodes.o: zmath.h
opcodes.o: zrand.h
pix.o: alloc.h
pix.o: byteswap.h
pix.o: endian_calc.h
pix.o: have_const.h
pix.o: have_malloc.h
pix.o: have_newstr.h
pix.o: have_stdlib.h
pix.o: have_string.h
pix.o: longbits.h
pix.o: pix.c
pix.o: prime.h
pix.o: qmath.h
pix.o: zmath.h
poly.o: alloc.h
poly.o: byteswap.h
poly.o: calcerr.h
poly.o: cmath.h
poly.o: config.h
poly.o: endian_calc.h
poly.o: hash.h
poly.o: have_malloc.h
poly.o: have_newstr.h
poly.o: have_stdlib.h
poly.o: have_string.h
poly.o: longbits.h
poly.o: poly.c
poly.o: qmath.h
poly.o: shs.h
poly.o: value.h
poly.o: zmath.h
prime.o: alloc.h
prime.o: byteswap.h
prime.o: endian_calc.h
prime.o: have_const.h
prime.o: have_malloc.h
prime.o: have_newstr.h
prime.o: have_stdlib.h
prime.o: have_string.h
prime.o: jump.h
prime.o: longbits.h
prime.o: prime.c
prime.o: prime.h
prime.o: qmath.h
prime.o: zmath.h
qfunc.o: alloc.h
qfunc.o: byteswap.h
qfunc.o: config.h
qfunc.o: endian_calc.h
qfunc.o: have_const.h
qfunc.o: have_malloc.h
qfunc.o: have_newstr.h
qfunc.o: have_stdlib.h
qfunc.o: have_string.h
qfunc.o: longbits.h
qfunc.o: prime.h
qfunc.o: qfunc.c
qfunc.o: qmath.h
qfunc.o: zmath.h
qio.o: alloc.h
qio.o: args.h
qio.o: byteswap.h
qio.o: config.h
qio.o: endian_calc.h
qio.o: have_malloc.h
qio.o: have_newstr.h
qio.o: have_stdlib.h
qio.o: have_string.h
qio.o: longbits.h
qio.o: qio.c
qio.o: qmath.h
qio.o: zmath.h
qmath.o: alloc.h
qmath.o: byteswap.h
qmath.o: config.h
qmath.o: endian_calc.h
qmath.o: have_malloc.h
qmath.o: have_newstr.h
qmath.o: have_stdlib.h
qmath.o: have_string.h
qmath.o: longbits.h
qmath.o: qmath.c
qmath.o: qmath.h
qmath.o: zmath.h
qmod.o: alloc.h
qmod.o: byteswap.h
qmod.o: config.h
qmod.o: endian_calc.h
qmod.o: have_malloc.h
qmod.o: have_newstr.h
qmod.o: have_stdlib.h
qmod.o: have_string.h
qmod.o: longbits.h
qmod.o: qmath.h
qmod.o: qmod.c
qmod.o: zmath.h
qtrans.o: alloc.h
qtrans.o: byteswap.h
qtrans.o: endian_calc.h
qtrans.o: have_malloc.h
qtrans.o: have_newstr.h
qtrans.o: have_stdlib.h
qtrans.o: have_string.h
qtrans.o: longbits.h
qtrans.o: qmath.h
qtrans.o: qtrans.c
qtrans.o: zmath.h
quickhash.o: alloc.h
quickhash.o: byteswap.h
quickhash.o: calcerr.h
quickhash.o: cmath.h
quickhash.o: config.h
quickhash.o: endian_calc.h
quickhash.o: hash.h
quickhash.o: have_const.h
quickhash.o: have_malloc.h
quickhash.o: have_newstr.h
quickhash.o: have_stdlib.h
quickhash.o: have_string.h
quickhash.o: longbits.h
quickhash.o: qmath.h
quickhash.o: quickhash.c
quickhash.o: shs.h
quickhash.o: value.h
quickhash.o: zmath.h
quickhash.o: zrand.h
shs.o: align32.h
shs.o: alloc.h
shs.o: byteswap.h
shs.o: calc.h
shs.o: calcerr.h
shs.o: cmath.h
shs.o: config.h
shs.o: endian_calc.h
shs.o: hash.h
shs.o: have_const.h
shs.o: have_malloc.h
shs.o: have_newstr.h
shs.o: have_stdlib.h
shs.o: have_string.h
shs.o: longbits.h
shs.o: qmath.h
shs.o: shs.c
shs.o: shs.h
shs.o: value.h
shs.o: zmath.h
shs.o: zrand.h
string.o: alloc.h
string.o: byteswap.h
string.o: calc.h
string.o: calcerr.h
string.o: cmath.h
string.o: config.h
string.o: endian_calc.h
string.o: hash.h
string.o: have_malloc.h
string.o: have_newstr.h
string.o: have_stdlib.h
string.o: have_string.h
string.o: longbits.h
string.o: qmath.h
string.o: shs.h
string.o: string.c
string.o: string.h
string.o: value.h
string.o: zmath.h
symbol.o: alloc.h
symbol.o: byteswap.h
symbol.o: calc.h
symbol.o: calcerr.h
symbol.o: cmath.h
symbol.o: config.h
symbol.o: endian_calc.h
symbol.o: func.h
symbol.o: hash.h
symbol.o: have_malloc.h
symbol.o: have_newstr.h
symbol.o: have_stdlib.h
symbol.o: have_string.h
symbol.o: label.h
symbol.o: longbits.h
symbol.o: opcodes.h
symbol.o: qmath.h
symbol.o: shs.h
symbol.o: string.h
symbol.o: symbol.c
symbol.o: symbol.h
symbol.o: token.h
symbol.o: value.h
symbol.o: zmath.h
token.o: alloc.h
token.o: args.h
token.o: byteswap.h
token.o: calc.h
token.o: calcerr.h
token.o: cmath.h
token.o: config.h
token.o: endian_calc.h
token.o: hash.h
token.o: have_malloc.h
token.o: have_newstr.h
token.o: have_stdlib.h
token.o: have_string.h
token.o: longbits.h
token.o: qmath.h
token.o: shs.h
token.o: string.h
token.o: token.c
token.o: token.h
token.o: value.h
token.o: zmath.h
value.o: alloc.h
value.o: byteswap.h
value.o: calc.h
value.o: calcerr.h
value.o: cmath.h
value.o: config.h
value.o: endian_calc.h
value.o: func.h
value.o: hash.h
value.o: have_const.h
value.o: have_malloc.h
value.o: have_newstr.h
value.o: have_stdlib.h
value.o: have_string.h
value.o: label.h
value.o: longbits.h
value.o: opcodes.h
value.o: qmath.h
value.o: shs.h
value.o: string.h
value.o: symbol.h
value.o: value.c
value.o: value.h
value.o: zmath.h
value.o: zrand.h
version.o: alloc.h
version.o: byteswap.h
version.o: calc.h
version.o: calcerr.h
version.o: cmath.h
version.o: config.h
version.o: endian_calc.h
version.o: hash.h
version.o: have_malloc.h
version.o: have_newstr.h
version.o: have_stdlib.h
version.o: have_string.h
version.o: longbits.h
version.o: qmath.h
version.o: shs.h
version.o: value.h
version.o: version.c
version.o: zmath.h
zfunc.o: alloc.h
zfunc.o: byteswap.h
zfunc.o: endian_calc.h
zfunc.o: have_malloc.h
zfunc.o: have_newstr.h
zfunc.o: have_stdlib.h
zfunc.o: have_string.h
zfunc.o: longbits.h
zfunc.o: zfunc.c
zfunc.o: zmath.h
zio.o: alloc.h
zio.o: args.h
zio.o: byteswap.h
zio.o: config.h
zio.o: endian_calc.h
zio.o: have_malloc.h
zio.o: have_newstr.h
zio.o: have_stdlib.h
zio.o: have_string.h
zio.o: longbits.h
zio.o: qmath.h
zio.o: zio.c
zio.o: zmath.h
zmath.o: alloc.h
zmath.o: byteswap.h
zmath.o: endian_calc.h
zmath.o: have_malloc.h
zmath.o: have_newstr.h
zmath.o: have_stdlib.h
zmath.o: have_string.h
zmath.o: longbits.h
zmath.o: zmath.c
zmath.o: zmath.h
zmod.o: alloc.h
zmod.o: byteswap.h
zmod.o: config.h
zmod.o: endian_calc.h
zmod.o: have_malloc.h
zmod.o: have_newstr.h
zmod.o: have_stdlib.h
zmod.o: have_string.h
zmod.o: longbits.h
zmod.o: qmath.h
zmod.o: zmath.h
zmod.o: zmod.c
zmul.o: alloc.h
zmul.o: byteswap.h
zmul.o: config.h
zmul.o: endian_calc.h
zmul.o: have_malloc.h
zmul.o: have_newstr.h
zmul.o: have_stdlib.h
zmul.o: have_string.h
zmul.o: longbits.h
zmul.o: qmath.h
zmul.o: zmath.h
zmul.o: zmul.c
zprime.o: alloc.h
zprime.o: byteswap.h
zprime.o: calcerr.h
zprime.o: cmath.h
zprime.o: config.h
zprime.o: endian_calc.h
zprime.o: hash.h
zprime.o: have_const.h
zprime.o: have_malloc.h
zprime.o: have_newstr.h
zprime.o: have_stdlib.h
zprime.o: have_string.h
zprime.o: jump.h
zprime.o: longbits.h
zprime.o: prime.h
zprime.o: qmath.h
zprime.o: shs.h
zprime.o: value.h
zprime.o: zmath.h
zprime.o: zprime.c
zprime.o: zrand.h
zrand.o: alloc.h
zrand.o: byteswap.h
zrand.o: calcerr.h
zrand.o: cmath.h
zrand.o: config.h
zrand.o: endian_calc.h
zrand.o: hash.h
zrand.o: have_const.h
zrand.o: have_malloc.h
zrand.o: have_newstr.h
zrand.o: have_stdlib.h
zrand.o: have_string.h
zrand.o: longbits.h
zrand.o: qmath.h
zrand.o: shs.h
zrand.o: value.h
zrand.o: zmath.h
zrand.o: zrand.c
zrand.o: zrand.h
