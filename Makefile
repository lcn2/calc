#!/bin/make
#
# calc - arbitrary precision calculator
#
# (Generic calc makefile)
#
#  NOTE: This is NOT the calc rpm Makefile.  This Makefile is a generic
#	 Makefile for the people who build calc from the gziped tarball.
#	 Without modification, it not assume the system has readline, ncurses
#	 or less.  It compiles with gcc -O3 -g3 as well.  You can change all
#	 this by modifying the Makefile variables below.
#
#  NOTE: You might want use the READLINE facility and the less pager if
#	 your system supports them.  To do this, set:
#
#	 USE_READLINE= -DUSE_READLINE
#	 READLINE_LIB= -lreadline -lhistory -lncurses
#	 CALCPAGER= less
#
# Copyright (C) 1999-2004  Landon Curt Noll
#
# Calc is open software; you can redistribute it and/or modify it under
# the terms of the version 2.1 of the GNU Lesser General Public License
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
MAKEFILE_REV= $$Revision: 29.73 $$
# @(#) $Id: Makefile.ship,v 29.73 2004/07/28 12:52:37 chongo Exp $
# @(#) $Source: /usr/local/src/cmd/calc/RCS/Makefile.ship,v $
#
# Under source code control:	1990/02/15 01:48:41
# File existed as early as:	before 1990
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
#
# calculator by David I. Bell with help/mods from others
# Makefile by Landon Curt Noll

##############################################################################
#-=-=-=-=-=-=-=-=- You may want to change some values below -=-=-=-=-=-=-=-=-#
##############################################################################

# Determine the type of terminal controls that you want to use
#
#	value		  meaning
#	--------	  -------
#	(nothing)	  let the Makefile guess at what you need
#	-DUSE_TERMIOS	  use struct termios from <termios.h>
#	-DUSE_TERMIO	  use struct termios from <termio.h>
#	-DUSE_SGTTY	  use struct sgttyb from <sys/ioctl.h>
#	-DUSE_NOTHING	  windoz system, don't use any of them
#
# Select TERMCONTROL= -DUSE_TERMIOS for DJGPP.
#
# If in doubt, leave TERMCONTROL empty.
#
TERMCONTROL=
#TERMCONTROL= -DUSE_TERMIOS
#TERMCONTROL= -DUSE_TERMIO
#TERMCONTROL= -DUSE_SGTTY
#TERMCONTROL= -DUSE_WIN32

# If your system does not have a vsprintf() function, you could be in trouble.
#
#	vsprintf(stream, format, ap)
#
# This function works like sprintf except that the 3rd arg is a va_list
# strarg (or varargs) list.  Some old systems do not have vsprintf().
# If you do not have vsprintf(), then calc will try sprintf() and hope
# for the best.
#
# If HAVE_VSPRINTF is empty, this Makefile will run the have_stdvs.c and/or
# have_varvs.c program to determine if vsprintf() is supported.	 If
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
# If in doubt, leave BYTE_ORDER empty.	This Makefile will attempt to
# use BYTE_ORDER in <machine/endian.h> or it will attempt to run
# the endian program.  If you get syntax errors when you compile,
# try forcing the value to be -DBIG_ENDIAN and run the calc regression
# tests. (see the README file)	If the calc regression tests fail, do
# a make clobber and try -DLITTLE_ENDIAN.   If that fails, ask a wizard
# for help.
#
# Select BYTE_ORDER= -DLITTLE_ENDIAN for DJGPP.
#
BYTE_ORDER=
#BYTE_ORDER= -DBIG_ENDIAN
#BYTE_ORDER= -DLITTLE_ENDIAN

# Determine the number of bits in a long
#
# If in doubt, leave LONG_BITS empty.  This Makefile will run
# the longbits program to determine the length.
#
# In order to avoid make brain damage in some systems, we avoid placing
# a space after the ='s below.
#
# Select LONG_BITS= 32 for DJGPP.
#
LONG_BITS=
#LONG_BITS= 32
#LONG_BITS= 64

# Determine if we have the ANSI C fgetpos and fsetpos alternate interface
# to the ftell() and fseek() (with whence set to SEEK_SET) functions.
#
# If HAVE_FPOS is empty, this Makefile will run the have_fpos program
# to determine if there is are fgetpos and fsetpos functions.  If HAVE_FPOS
# is set to -DHAVE_NO_FPOS, then calc will use ftell() and fseek().
#
# If in doubt, leave HAVE_FPOS empty and this Makefile will figure it out.
#
HAVE_FPOS=
#HAVE_FPOS= -DHAVE_NO_FPOS

# Determine if we have an __pos element of a file position (fpos_t) structure.
#
# If HAVE_FPOS_POS is empty, this Makefile will run the have_fpos_pos program
# to determine if fpos_t has a __pos structure element.  If HAVE_FPOS_POS
# is set to -DHAVE_NO_FPOS_POS, then calc assume there is no __pos element.
#
# Select HAVE_FPOS_POS= -DHAVE_NO_FPOS_POS for DJGPP.
#
# If in doubt, leave HAVE_FPOS_POS empty and this Makefile will figure it out.
#
HAVE_FPOS_POS=
#HAVE_FPOS_POS= -DHAVE_NO_FPOS_POS

# Determine the size of the __pos element in fpos_t, if it exists.
#
# If FPOS_POS_BITS is empty, then the Makefile will determine the size of
# the file position value of the __pos element.
#
# If there is no __pos element in fpos_t (say because fpos_t is a scalar),
# leave FPOS_POS_BITS blank.
#
# If in doubt, leave FPOS_POS_BITS empty and this Makefile will figure it out.
#
FPOS_POS_BITS=
#FPOS_POS_BITS= 32
#FPOS_POS_BITS= 64

# Determine the size of a file position value.
#
# If FPOS_BITS is empty, then the Makefile will determine the size of
# the file position value.
#
# Select FPOS_BITS= 32 for DJGPP.
#
# If in doubt, leave FPOS_BITS empty and this Makefile will figure it out.
#
FPOS_BITS=
#FPOS_BITS= 32
#FPOS_BITS= 64

# Determine the size of the off_t file offset element
#
# If OFF_T_BITS is empty, then the Makefile will determine the size of
# the file offset value.
#
# Select OFF_T_BITS= 32 for DJGPP.
#
# If in doubt, leave OFF_T_BITS empty and this Makefile will figure it out.
#
OFF_T_BITS=
#OFF_T_BITS= 32
#OFF_T_BITS= 64

# Determine the size of the dev_t device value
#
# If DEV_BITS is empty, then the Makefile will determine the size of
# the dev_t device value
#
# Select DEV_BITS= 32 for DJGPP.
#
# If in doubt, leave DEV_BITS empty and this Makefile will figure it out.
#
DEV_BITS=
#DEV_BITS= 16
#DEV_BITS= 32
#DEV_BITS= 64

# Determine the size of the ino_t device value
#
# If INODE_BITS is empty, then the Makefile will determine the size of
# the ino_t inode value
#
# Select INODE_BITS= 32 for DJGPP.
#
# If in doubt, leave INODE_BITS empty and this Makefile will figure it out.
#
INODE_BITS=
#INODE_BITS= 16
#INODE_BITS= 32
#INODE_BITS= 64

# Determine if we have an off_t which one can perform arithmetic operations,
# assignments and comparisons.	On some systems off_t is some sort of union
# or struct.
#
# If HAVE_OFFSCL is empty, this Makefile will run the have_offscl program
# to determine if off_t is a scalar.  If HAVE_OFFSCL is set to the value
# -DOFF_T_NON_SCALAR when calc will assume that off_t some sort of
# union or struct which.
#
# If in doubt, leave HAVE_OFFSCL empty and this Makefile will figure it out.
#
HAVE_OFFSCL=
#HAVE_OFFSCL= -DOFF_T_NON_SCALAR

# Determine if we have an fpos_t which one can perform arithmetic operations,
# assignments and comparisons.	On some systems fpos_t is some sort of union
# or struct.  Some systems do not have an fpos_t and long is as a file
# offset instead.
#
# If HAVE_POSSCL is empty, this Makefile will run the have_offscl program
# to determine if off_t is a scalar, or if there is no off_t and long
# (a scalar) should be used instead.  If HAVE_POSSCL is set to the value
# -DFILEPOS_NON_SCALAR when calc will assume that fpos_t exists and is
# some sort of union or struct which.
#
# If in doubt, leave HAVE_POSSCL empty and this Makefile will figure it out.
#
HAVE_POSSCL=
#HAVE_POSSCL= -DFILEPOS_NON_SCALAR

# Determine if we have ANSI C const.
#
# If HAVE_CONST is empty, this Makefile will run the have_const program
# to determine if const is supported.  If HAVE_CONST is set to -DHAVE_NO_CONST,
# then calc will not use const.
#
# If in doubt, leave HAVE_CONST empty and this Makefile will figure it out.
#
HAVE_CONST=
#HAVE_CONST= -DHAVE_NO_CONST

# Determine if we have uid_t
#
# If HAVE_UID_T is empty, this Makefile will run the have_uid_t program
# to determine if const is supported.  If HAVE_UID_T is set to -DHAVE_NO_UID_T,
# then calc will treat uid_t as an unsigned short.  This only matters if
# $HOME is not set and calc must look up the home directory in /etc/passwd.
#
# If in doubt, leave HAVE_UID_T empty and this Makefile will figure it out.
#
HAVE_UID_T=
#HAVE_UID_T= -DHAVE_NO_UID_T

# Determine if we have memcpy(), memset() and strchr()
#
# If HAVE_NEWSTR is empty, this Makefile will run the have_newstr program
# to determine if memcpy(), memset() and strchr() are supported.  If
# HAVE_NEWSTR is set to -DHAVE_NO_NEWSTR, then calc will use bcopy() instead
# of memcpy(), use bfill() instead of memset(), and use index() instead of
# strchr().
#
# If in doubt, leave HAVE_NEWSTR empty and this Makefile will figure it out.
#
HAVE_NEWSTR=
#HAVE_NEWSTR= -DHAVE_NO_NEWSTR

# Determine if we have memmove()
#
# If HAVE_MEMMOVE is empty, this Makefile will run the have_memmv program
# to determine if memmove() is supported.  If HAVE_MEMMOVE is set to
# -DHAVE_NO_MEMMOVE, then calc will use internal functions to simulate
# the memory move function that does correct overlapping memory modes.
#
# If in doubt, leave HAVE_MEMMOVE empty and this Makefile will figure it out.
#
HAVE_MEMMOVE=
#HAVE_MEMMOVE= -DHAVE_NO_MEMMOVE

# Determine if we have ustat()
#
# If HAVE_USTAT is empty, this Makefile will run the have_memmv program
# to determine if ustat() is supported.	 If HAVE_USTAT is set to
# -DHAVE_NO_USTAT, then calc will use internal functions to simulate
# the memory move function that does correct overlapping memory modes.
#
# Select HAVE_USTAT= -DHAVE_NO_USTAT for DJGPP.
#
# If in doubt, leave HAVE_USTAT empty and this Makefile will figure it out.
#
HAVE_USTAT=
#HAVE_USTAT= -DHAVE_NO_USTAT

# Determine if we have getsid()
#
# If HAVE_GETSID is empty, this Makefile will run the have_memmv program
# to determine if getsid() is supported.  If HAVE_GETSID is set to
# -DHAVE_NO_GETSID, then calc will use internal functions to simulate
# the memory move function that does correct overlapping memory modes.
#
# Select HAVE_GETSID= -DHAVE_NO_GETSID for DJGPP.
#
# If in doubt, leave HAVE_GETSID empty and this Makefile will figure it out.
#
HAVE_GETSID=
#HAVE_GETSID= -DHAVE_NO_GETSID

# Determine if we have getpgid()
#
# If HAVE_GETPGID is empty, this Makefile will run the have_memmv program
# to determine if getpgid() is supported.  If HAVE_GETPGID is set to
# -DHAVE_NO_GETPGID, then calc will use internal functions to simulate
# the memory move function that does correct overlapping memory modes.
#
# Select HAVE_GETPGID= -DHAVE_NO_GETPGID for DJGPP.
#
# If in doubt, leave HAVE_GETPGID empty and this Makefile will figure it out.
#
HAVE_GETPGID=
#HAVE_GETPGID= -DHAVE_NO_GETPGID

# Determine if we have clock_gettime()
#
# If HAVE_GETTIME is empty, this Makefile will run the have_memmv program
# to determine if clock_gettime() is supported.	 If HAVE_GETTIME is set to
# -DHAVE_NO_GETTIME, then calc will use internal functions to simulate
# the memory move function that does correct overlapping memory modes.
#
# Select HAVE_GETTIME= -DHAVE_NO_GETTIME for DJGPP.
#
# If in doubt, leave HAVE_GETTIME empty and this Makefile will figure it out.
#
HAVE_GETTIME=
#HAVE_GETTIME= -DHAVE_NO_GETTIME

# Determine if we have getprid()
#
# If HAVE_GETPRID is empty, this Makefile will run the have_memmv program
# to determine if getprid() is supported.  If HAVE_GETPRID is set to
# -DHAVE_NO_GETPRID, then calc will use internal functions to simulate
# the memory move function that does correct overlapping memory modes.
#
# Select HAVE_GETPRID= -DHAVE_NO_GETPRID for DJGPP.
#
# If in doubt, leave HAVE_GETPRID empty and this Makefile will figure it out.
#
HAVE_GETPRID=
#HAVE_GETPRID= -DHAVE_NO_GETPRID

# Determine if we have the /dev/urandom
#
#    HAVE_URANDOM_H=		let the Makefile look /dev/urandom
#    HAVE_URANDOM_H= YES	assume that /dev/urandom exists
#    HAVE_URANDOM_H= NO		assume that /dev/urandom does not exist
#
# Select HAVE_URANDOM_H= NO for DJGPP.
#
# When in doubt, leave HAVE_URANDOM_H empty.
#
HAVE_URANDOM_H=
#HAVE_URANDOM_H= YES
#HAVE_URANDOM_H= NO

# Determine if we have getrusage()
#
# If HAVE_GETRUSAGE is empty, this Makefile will run the have_memmv program
# to determine if getrusage() is supported.  If HAVE_GETRUSAGE is set to
# -DHAVE_NO_GETRUSAGE, then calc will use internal functions to simulate
# the memory move function that does correct overlapping memory modes.
#
# If in doubt, leave HAVE_GETRUSAGE empty and this Makefile will figure it out.
#
HAVE_GETRUSAGE=
#HAVE_GETRUSAGE= -DHAVE_NO_GETRUSAGE

# Determine if we have strdup()
#
# If HAVE_STRDUP is empty, this Makefile will run the have_memmv program
# to determine if strdup() is supported.  If HAVE_STRDUP is set to
# -DHAVE_NO_STRDUP, then calc will use internal functions to simulate
# the memory move function that does correct overlapping memory modes.
#
# If in doubt, leave HAVE_STRDUP empty and this Makefile will figure it out.
#
HAVE_STRDUP=
#HAVE_STRDUP= -DHAVE_NO_STRDUP

# Some architectures such as Sparc do not allow one to access 32 bit values
# that are not alligned on a 32 bit boundary.
#
# The Dec Alpha running OSF/1 will produce alignment error messages when
# align32.c tries to figure out if alignment is needed.	 Use the
# ALIGN32= -DMUST_ALIGN32 to force alignment and avoid such error messages.
#
# ALIGN32=		     let align32.c figure out if alignment is needed
# ALIGN32= -DMUST_ALIGN32    force 32 bit alignment
# ALIGN32= -UMUST_ALIGN32    allow non-alignment of 32 bit accesses
#
# Select ALIGN32= -UMUST_ALIGN32 for DJGPP.
#
# When in doubt, be safe and pick ALIGN32=-DMUST_ALIGN32.
#
ALIGN32=
#ALIGN32= -DMUST_ALIGN32
#ALIGN32= -UMUST_ALIGN32

# Determine if we have the <malloc.h> include file.
#
#    HAVE_MALLOC_H=		let the Makefile look for the include file
#    HAVE_MALLOC_H= YES		assume that the include file exists
#    HAVE_MALLOC_H= NO		assume that the include file does not exist
#
# Select HAVE_MALLOC_H= YES for DJGPP.
#
# When in doubt, leave HAVE_MALLOC_H empty.
#
HAVE_MALLOC_H=
#HAVE_MALLOC_H= YES
#HAVE_MALLOC_H= NO

# Determine if we have the <stdlib.h> include file.
#
#    HAVE_STDLIB_H=		let the Makefile look for the include file
#    HAVE_STDLIB_H= YES		assume that the include file exists
#    HAVE_STDLIB_H= NO		assume that the include file does not exist
#
# Select HAVE_STDLIB_H= YES for DJGPP.
#
# When in doubt, leave HAVE_STDLIB_H empty.
#
HAVE_STDLIB_H=
#HAVE_STDLIB_H= YES
#HAVE_STDLIB_H= NO

# Determine if we have the <string.h> include file.
#
#    HAVE_STRING_H=		let the Makefile look for the include file
#    HAVE_STRING_H= YES		assume that the include file exists
#    HAVE_STRING_H= NO		assume that the include file does not exist
#
# Select HAVE_STRING_H= YES for DJGPP.
#
# When in doubt, leave HAVE_STRING_H empty.
#
HAVE_STRING_H=
#HAVE_STRING_H= YES
#HAVE_STRING_H= NO

# Determine if we have the <times.h> include file.
#
#    HAVE_TIMES_H=		let the Makefile look for the include file
#    HAVE_TIMES_H= YES		assume that the include file exists
#    HAVE_TIMES_H= NO		assume that the include file does not exist
#
# Select HAVE_TIMES_H= NO for DJGPP.
#
# When in doubt, leave HAVE_TIMES_H empty.
#
HAVE_TIMES_H=
#HAVE_TIMES_H= YES
#HAVE_TIMES_H= NO

# Determine if we have the <sys/times.h> include file.
#
#    HAVE_SYS_TIMES_H=		let the Makefile look for the include file
#    HAVE_SYS_TIMES_H= YES	assume that the include file exists
#    HAVE_SYS_TIMES_H= NO	assume that the include file does not exist
#
# Select HAVE_SYS_TIMES_H= YES for DJGPP.
#
# When in doubt, leave HAVE_SYS_TIMES_H empty.
#
HAVE_SYS_TIMES_H=
#HAVE_SYS_TIMES_H= YES
#HAVE_SYS_TIMES_H= NO

# Determine if we have the <time.h> include file.
#
#    HAVE_TIME_H=		let the Makefile look for the include file
#    HAVE_TIME_H= YES		assume that the include file exists
#    HAVE_TIME_H= NO		assume that the include file does not exist
#
# Select HAVE_TIME_H= YES for DJGPP.
#
# When in doubt, leave HAVE_TIME_H empty.
#
HAVE_TIME_H=
#HAVE_TIME_H= YES
#HAVE_TIME_H= NO

# Determine if we have the <sys/time.h> include file.
#
#    HAVE_SYS_TIME_H=		let the Makefile look for the include file
#    HAVE_SYS_TIME_H= YES	assume that the include file exists
#    HAVE_SYS_TIME_H= NO	assume that the include file does not exist
#
# Select HAVE_SYS_TIME_H= YES for DJGPP.
#
# When in doubt, leave HAVE_SYS_TIME_H empty.
#
HAVE_SYS_TIME_H=
#HAVE_SYS_TIME_H= YES
#HAVE_SYS_TIME_H= NO

# Determine if we have the <unistd.h> include file.
#
#    HAVE_UNISTD_H=		let the Makefile look for the include file
#    HAVE_UNISTD_H= YES		assume that the include file exists
#    HAVE_UNISTD_H= NO		assume that the include file does not exist
#
# Select HAVE_UNISTD_H= YES for DJGPP.
#
# When in doubt, leave HAVE_UNISTD_H empty.
#
HAVE_UNISTD_H=
#HAVE_UNISTD_H= YES
#HAVE_UNISTD_H= NO

# Determine if our compiler allows the -Wno-implicit flag
#
#    HAVE_NO_IMPLICIT=		let the Makefile test the -Wno-implicit flag
#    HAVE_NO_IMPLICIT= NO	Our compiler does not have -Wno-implicit flags
#    HAVE_NO_IMPLICIT= YES	Our compiler has a -Wno-implicit flag
#
# Select HAVE_NO_IMPLICIT= for DJGPP.
#
HAVE_NO_IMPLICIT=
# HAVE_NO_IMPLICIT= NO
# HAVE_NO_IMPLICIT= YES

# Determine if our compiler allows the unused attribute
#
# If HAVE_UNUSED is empty, this Makefile will run the have_unused program
# to determine if the unused attribute is supported.  If HAVE_UNUSED is set to
# -DHAVE_NO_UNUSED, then the unused attribute will not be used.
#
# Select HAVE_UNUSED= for DJGPP.
#
# If in doubt, leave HAVE_UNUSED empty and this Makefile will figure it out.
#
HAVE_UNUSED=
#HAVE_UNUSED= -DHAVE_NO_UNUSED

# System include files
#
# ${INCDIR}		where the system include (.h) files are kept
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

# Where to install calc realted things
#
# ${BINDIR}		where to install calc binary files
# ${LIBDIR}		where calc link library (*.a) files are installed
# ${CALC_SHAREDIR}	where to install calc help, .cal, startup, config files
# ${CALC_INCDIR}	where the calc include files are installed
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

#CALC_INCDIR= /usr/local/include/calc
#CALC_INCDIR= /dev/env/DJDIR/include/calc
CALC_INCDIR= ${INCDIR}/calc

# By default, these values are based CALC_SHAREDIR, INCDIR, BINDIR
# ---------------------------------------------------------------
# ${HELPDIR}		where the help directory is installed
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

# where man pages are installed
#
# Select MANDIR= /dev/env/DJDIR/man/man1 for DJGPP.
#
# Use MANDIR= to disable installation of the calc man (source) page.
#
#MANDIR=
#MANDIR= /usr/local/man/man1
#MANDIR= /usr/man/man1
MANDIR= /usr/share/man/man1
#MANDIR= /dev/env/DJDIR/man/man1
#MANDIR= /usr/man/u_man/man1
#MANDIR= /usr/contrib/man/man1

# where cat (formatted man) pages are installed
#
# Select CATDIR= /dev/env/DJDIR/man/cat1 for DJGPP.
#
# Use CATDIR= to disable installation of the calc cat (formatted) page.
#
CATDIR=
#CATDIR= /usr/local/man/cat1
#CATDIR= /usr/local/catman/cat1
#CATDIR= /usr/man/cat1
#CATDIR= /usr/share/man/cat1
#CATDIR= /dev/env/DJDIR/man/cat1
#CATDIR= /var/cache/man/cat1
#CATDIR= /usr/man/u_man/cat1
#CATDIR= /usr/contrib/man/cat1

# extension to add on to the calc man page filename
#
# This is ignored if CATDIR is empty.
#
MANEXT= 1
#MANEXT= l

# extension to add on to the calc man page filename
#
# This is ignored if CATDIR is empty.
#
CATEXT= 1
#CATEXT= 1.gz
#CATEXT= 0
#CATEXT= l

# how to format a man page
#
# If CATDIR is non-empty, then
#
#     If NROFF is non-empty, then
#
#	  ${NROFF} ${NROFF_ARG} calc.1 > ${CATDIR}/calc.${CATEXT}
#	  	   is used to built and install the cat page
#
#     else (NROFF is empty)
#
#	  ${MANMAKE} calc.1 ${CATDIR}
#	  	     is used to built and install the cat page
# else
#
#     The cat page is not built or installed
#
# Select NROFF= groff for DJGPP.
#
# If in doubt and you don't want to fool with man pages, set MANDIR
# and CATDIR to empty and ignore the NROFF, NROFF_ARG and MANMAKE
# lines below.
#
#NROFF= nroff
NROFF=
#NROFF= groff
NROFF_ARG= -man
#NROFF_ARG= -mandoc
MANMAKE= /usr/local/bin/manmake
#MANMAKE= manmake
MANMODE= 0444
CATMODE= 0444

# If the $CALCPATH environment variable is not defined, then the following
# path will be search for calc resource file routines.
#
# Select CALCPATH= .;./cal;~/.cal;${CALC_SHAREDIR};${CUSTOMCALDIR} for DJGPP.
#
CALCPATH= .:./cal:~/.cal:${CALC_SHAREDIR}:${CUSTOMCALDIR}
#CALCPATH= .;./cal;~/.cal;${CALC_SHAREDIR};${CUSTOMCALDIR}

# If the $CALCRC environment variable is not defined, then the following
# path will be search for calc resource files.
#
# Select CALCRC= ${CALC_SHAREDIR}/startup;~/.calcrc;./.calcinit for DJGPP.
#
CALCRC= ${CALC_SHAREDIR}/startup:~/.calcrc:./.calcinit
#CALCRC= ${CALC_SHAREDIR}/startup;~/.calcrc;./.calcinit

# Determine of the GNU-readline facility will be used instead of the
# built-in calc binding method.
#
# USE_READLINE=			    Do not use GNU-readline, use calc bindings
# USE_READLINE= -DUSE_READLINE	    Use GNU-readline, do not use calc bindings
#
# NOTE: If you select the 'USE_READLINE= -DUSE_READLINE' mode, you must set:
#
#	READLINE_LIB		The flags needed to link in the readline
#				and history link libraries
#	READLINE_INCLUDE	Where the readline include files reside
#				(leave blank if they are /usr/include/readline)
#
# NOTE: The GNU-readline code is not shipped with calc.	 You must have
#	the appropriate headers and link libs installed on your system in
#	order to use it.
#
# If in doubt, set USE_READLINE, READLINE_LIB and READLINE_INCLUDE to nothing.
#
USE_READLINE=
#USE_READLINE= -DUSE_READLINE
#
READLINE_LIB=
#READLINE_LIB= -lreadline -lhistory -lncurses
#READLINE_LIB= -L/usr/gnu/lib -lreadline -lhistory -lncurses
#READLINE_LIB= -L/usr/local/lib -lreadline -lhistory -lncurses
#
READLINE_INCLUDE=
#READLINE_INCLUDE= -I/usr/gnu/include
#READLINE_INCLUDE= -I/usr/local/include

# If $PAGER is not set, use this program to display a help file
#
# Select CALCPAGER= less.exe -ci for DJGPP.
#
CALCPAGER= more
#CALCPAGER= pg
#CALCPAGER= cat
#CALCPAGER= less
#CALCPAGER= less.exe -ci

# Debug/Optimize options for ${CC} and ${LCC}
#
# Select DEBUG= -O2 -gstabs+ for DJGPP.
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
#DEBUG= -std0 -fast -O4 -static
#
#DEBUG= -g
#DEBUG= -g3
#DEBUG= -gx
#DEBUG= -WM,-g
#
#DEBUG= -O2 -gstabs+

# On systems that have dynamic shared link libs, you may want want to disable
# them for faster calc startup.
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

# On some systems where you are disabling dynamic shared link libs, you may
# need to pass a special flag to ${CC} and ${LCC} during linking stage.
#
#    System type			    NO_SHARED recommendation
#
#	IRIX with NO_SHARED= -non_shared    LD_NO_SHARED= -Wl,-rdata_shared
#	IRIX with NO_SHARED=		    LD_NO_SHARED=
#	others				    LD_NO_SHARED=
#
# If in doubt, use LD_NO_SHARED=
#
LD_NO_SHARED=
#LD_NO_SHARED= -Wl,-rdata_shared

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

# If you do not wish to use purify, leave PURIFY commented out.
#
# If in doubt, leave PURIFY commented out.
#
#PURIFY= purify
#PURIFY= purify -m71-engine
#PURIFY= purify -logfile=pure.out
#PURIFY= purify -m71-engine -logfile=pure.out

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

# The install rule uses:
#
#	${MKDIR} ${MKDIR_ARG}
#
# to create directorties.  Normall this amounts to usins mkdir -p dir ...
# Some older systems may not have mkdir -p.  If you system does not
# make mkdir -p, then set MKDIR_ARG to empty.
#
# MKDIR_ARG= -p			# use mkdir -p when creating paths
# MKDIR_ARG=			# use if system does not understand mkdir -p
#
MKDIR_ARG= -p
#MKDIR_ARG=

# Some out of date operating systems require / want an executable to
# end with a certain file extension.  Some compile systems such as
# Cygwin build calc as calc.exe.  The EXT variable is used to denote
# the extension required by such.
#
# EXT=				# normal Un*x / Linux / GNU/Linux systems
# EXT=.exe			# windoz / Cygwin
#
# If in doubt, use EXT=
#
EXT=
#EXT=.exe

################
# compiler set #
################
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
# LCC how the the C compiler is invoked on locally executed intermediate progs
# CC is how the the C compiler is invoked (with an optional Purify)
#
###
#
# Linux set
#
# Tested on Red Hat 6.0 Linux but should run on almost any Linux release.
#
CCWARN= -Wall -W -Wno-comment
CCOPT= ${DEBUG} ${NO_SHARED}
CCMISC=
#
CFLAGS= -DCALC_SRC ${CCWARN} ${CCOPT} ${CCMISC}
ICFLAGS= -DCALC_SRC ${CCWARN} ${CCMISC}
#
LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
ILDFLAGS=
#
LCC= gcc
CC= ${PURIFY} ${LCC}
#
###
#
# gcc set	(some call it gcc2, some call it gcc)
#
#CCWARN= -Wall -W -Wno-comment
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC=
#
#CFLAGS= -DCALC_SRC ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= -DCALC_SRC ${CCWARN} ${CCMISC}
#
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#LCC= gcc
#LCC= gcc2
#CC= ${PURIFY} ${LCC}
#
###
#
# common cc set
#
# If -O3 -g3 is not supported try:			DEBUG= -O2 -g
# If -O2 -g is not supported try:			DEBUG= -O -g
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC=
#
#CFLAGS= -DCALC_SRC ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= -DCALC_SRC ${CCWARN} ${CCMISC}
#
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#LCC= cc
#CC= ${PURIFY} ${LCC}
#
###
#
# SGI IRIX6.2 (or later) -n32 (v7.1 or later) Compiler
#
# You must set above:					RANLIB=:
#
# If -O3 -g3 is not supported try:			DEBUG= -O2 -g
# If -O2 -g is not supported try:			DEBUG= -O -g
#
# If you have the directory /usr/lib32/nonshared, then set the following above:
#	NO_SHARED= -non_shared
#	LD_NO_SHARED= -Wl,-rdata_shared
#
# woff 1209: cancel 'controlling expression is constant' warnings
#
#CCWARN= -fullwarn -woff 1209
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -rdata_shared
#
#CFLAGS= -DCALC_SRC ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= -DCALC_SRC ${CCWARN} ${CCMISC}
#
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#LCC= cc -n32 -xansi
#CC= ${PURIFY} ${LCC}
#
###
#
# HP-UX set
#
# If -O3 -g3 is not supported try:			DEBUG= -O2 -g
# If -O2 -g is not supported try:			DEBUG= -O -g
#
# Warning: Some HP-UX optimizers are brain-damaged.
# If 'make check' fails use:				DEBUG= -g
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= +e
#
#CFLAGS= -DCALC_SRC ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= -DCALC_SRC ${CCWARN} ${CCMISC}
#
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#LCC= cc
#CC= ${PURIFY} ${LCC}
#
###
#
# AIX RS/6000 set
#
# If -O3 -g3 is not supported try:			DEBUG= -O2 -g
# If -O2 -g is not supported try:			DEBUG= -O -g
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -qlanglvl=ansi
#
#CFLAGS= -DCALC_SRC ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= -DCALC_SRC ${CCWARN} ${CCMISC}
#
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#LCC= cc
#CC= ${PURIFY} ${LCC}
#
###
#
# Solaris Sun cc compiler set
#
# If -O3 -g3 is not supported try:			DEBUG= -O2 -g
# If -O2 -g is not supported try:			DEBUG= -O -g
#
# We need -DFORCE_STDC to make use of ANSI-C like features and
# to avoid the use of -Xc (which as a lose performance wise).
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC= -DFORCE_STDC
#
#CFLAGS= -DCALC_SRC ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= -DCALC_SRC ${CCWARN} ${CCMISC}
#
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#LCC= cc
#CC= ${PURIFY} ${LCC}
#
###
#
# Dec Alpha / Compaq Tru64 cc (non-gnu) compiler set
#
# For better performance, set the following:	DEBUG= -std0 -fast -O4 -static
#
#CCWARN=
#CCOPT= ${DEBUG} ${NO_SHARED}
#CCMISC=
#
#CFLAGS= -DCALC_SRC ${CCWARN} ${CCOPT} ${CCMISC}
#ICFLAGS= -DCALC_SRC ${CCWARN} ${CCMISC} -Wno-unused
#
#LDFLAGS= ${NO_SHARED} ${LD_NO_SHARED}
#ILDFLAGS=
#
#LCC= cc
#CC= ${PURIFY} ${LCC}

##############################################################################
#-=-=-=-=-=-=-=-=- Be careful if you change something below -=-=-=-=-=-=-=-=-#
##############################################################################

# standard utilities used during make
#
SHELL= /bin/sh
LANG= C
MAKE= make
AWK= awk
SED= sed
DIFF= diff
GREP= egrep
SORT= sort
TEE= tee
CTAGS= ctags
CHMOD= chmod
FMT= fmt
XARGS= xargs
CMP= cmp
MKDIR= mkdir
SPLINT = splint
SPLINT_OPTS =
# assume the X11 makedepend tool for the depend rule
MAKEDEPEND= makedepend
# echo command location
#
# Select ECHO= echo for DJGPP.
#
ECHO= /bin/echo
#ECHO= echo

# Makefile debug
#
# Q=@	do not echo internal Makefile actions (quiet mode)
# Q=	echo internal Makefile actions (debug / verbose mode)
#
# V=@:	do not echo debug statements (quiet mode)
# V=@	do echo debug statements (debug / verbose mode)
#
#Q=
Q=@
V=@:
#V=@

# the source files which are built into a math link library
#
# There MUST be a .o for every .c in LIBOBJS
#
LIBSRC= addop.c assocfunc.c blkcpy.c block.c byteswap.c \
	codegen.c comfunc.c commath.c config.c const.c custom.c \
	file.c func.c hash.c help.c hist.c input.c jump.c label.c \
	lib_calc.c lib_util.c listfunc.c matfunc.c math_error.c \
	md5.c obj.c opcodes.c pix.c poly.c prime.c qfunc.c qio.c \
	qmath.c qmod.c qtrans.c quickhash.c seed.c shs.c shs1.c size.c \
	string.c symbol.c token.c value.c version.c zfunc.c zio.c \
	zmath.c zmod.c zmul.c zprime.c zrand.c zrandom.c

# the object files which are built into a math link library
#
# There MUST be a .o for every .c in LIBSRC plus calcerr.o
# which is built via this Makefile.
#
LIBOBJS= addop.o assocfunc.o blkcpy.o block.o byteswap.o calcerr.o \
	codegen.o comfunc.o commath.o config.o const.o custom.o \
	file.o func.o hash.o help.o hist.o input.o jump.o label.o \
	lib_calc.o lib_util.o listfunc.o matfunc.o math_error.o \
	md5.o obj.o opcodes.o pix.o poly.o prime.o qfunc.o qio.o \
	qmath.o qmod.o qtrans.o quickhash.o seed.o shs.o shs1.o size.o \
	string.o symbol.o token.o value.o version.o zfunc.o zio.o \
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

# these .h files are needed by programs that use libcalc.a
#
LIB_H_SRC= alloc.h blkcpy.h block.h byteswap.h calc.h cmath.h \
	config.h custom.h file.h func.h hash.h hist.h jump.h \
	label.h lib_util.h math_error.h md5.h nametype.h \
	opcodes.h prime.h qmath.h shs.h shs1.h string.h \
	symbol.h token.h value.h win32dll.h zmath.h zrand.h zrandom.h

# we build these .h files during the make
#
BUILD_H_SRC= align32.h args.h calcerr.h conf.h endian_calc.h \
	fposval.h have_const.h have_fpos.h have_fpos_pos.h have_malloc.h \
	have_memmv.h have_newstr.h have_offscl.h have_posscl.h \
	have_stdlib.h have_string.h have_times.h have_uid_t.h \
	have_unistd.h longbits.h terminal.h \
	have_ustat.h have_getsid.h have_getpgid.h \
	have_gettime.h have_getprid.h have_urandom.h have_rusage.h \
	have_strdup.h have_unused.h

# we build these .c files during the make
#
BUILD_C_SRC= calcerr.c

# these .c files may be used in the process of building BUILD_H_SRC
#
# There MUST be a .c for every .o in UTIL_OBJS.
#
UTIL_C_SRC= align32.c endian.c longbits.c have_newstr.c have_uid_t.c \
	have_const.c have_stdvs.c have_varvs.c fposval.c have_fpos.c \
	have_fpos_pos.c have_offscl.c have_posscl.c have_memmv.c \
	have_ustat.c have_getsid.c have_getpgid.c \
	have_gettime.c have_getprid.c have_rusage.c have_strdup.c \
	no_implicit.c have_unused.c

# these awk and sed tools are used in the process of building BUILD_H_SRC
# and BUILD_C_SRC
#
UTIL_MISC_SRC= calcerr_h.sed calcerr_h.awk calcerr_c.sed calcerr_c.awk \
	calcerr.tbl check.awk win32.mkdef

# these .o files may get built in the process of building BUILD_H_SRC
#
# There MUST be a .o for every .c in UTIL_C_SRC.
#
UTIL_OBJS= endian.o longbits.o have_newstr.o have_uid_t.o \
	have_const.o fposval.o have_fpos.o have_fpos_pos.o \
	try_strarg.o have_stdvs.o have_varvs.o have_posscl.o have_memmv.o \
	have_ustat.o have_getsid.o have_getpgid.o \
	have_gettime.o have_getprid.o ver_calc.o have_rusage.o have_strdup.o \
	no_implicit.o have_unused.o

# these temp files may be created (and removed) during the build of BUILD_C_SRC
#
UTIL_TMP= ll_tmp fpos_tmp fposv_tmp const_tmp uid_tmp newstr_tmp vs_tmp \
	memmv_tmp offscl_tmp posscl_tmp newstr_tmp \
	getsid_tmp gettime_tmp getprid_tmp rusage_tmp strdup_tmp

# these utility progs may be created in the process of building BUILD_H_SRC
#
UTIL_PROGS= align32${EXT} fposval${EXT} have_uid_t${EXT} have_const${EXT} \
	endian${EXT} longbits${EXT} have_newstr${EXT} have_stdvs${EXT} \
	have_varvs${EXT} have_ustat${EXT} have_getsid${EXT} \
	have_getpgid${EXT} have_gettime${EXT} have_getprid${EXT} \
	ver_calc${EXT} have_strdup${EXT} no_implicit${EXT} no_implicit.arg \
	have_unused${EXT} have_fpos${EXT} have_fpos_pos${EXT} \
	have_offscl${EXT} have_rusage${EXT} have_args.sh

# The complete list of Makefile vars passed down to custom/Makefile.
#
CUSTOM_PASSDOWN= Q="${Q}" \
    INCDIR="${INCDIR}" \
    BINDIR="${BINDIR}" \
    LIBDIR="${LIBDIR}" \
    CALC_SHAREDIR="${CALC_SHAREDIR}" \
    HELPDIR="${HELPDIR}" \
    CALC_INCDIR="${CALC_INCDIR}" \
    CUSTOMCALDIR="${CUSTOMCALDIR}" \
    CUSTOMHELPDIR="${CUSTOMHELPDIR}" \
    CUSTOMINCDIR="${CUSTOMINCDIR}" \
    SCRIPTDIR="${SCRIPTDIR}" \
    DEBUG="${DEBUG}" \
    NO_SHARED="${NO_SHARED}" \
    RANLIB="${RANLIB}" \
    PURIFY="${PURIFY}" \
    ALLOW_CUSTOM="${ALLOW_CUSTOM}" \
    CCWARN="${CCWARN}" \
    CCOPT="${CCOPT}" \
    CCMISC="${CCMISC}" \
    CFLAGS="${CFLAGS} ${ALLOW_CUSTOM}" \
    ICFLAGS="${ICFLAGS}" \
    LDFLAGS="${LDFLAGS}" \
    ILDFLAGS="${ILDFLAGS}" \
    LCC="${LCC}" \
    CC="${CC}" \
    MAKE_FILE=${MAKE_FILE} \
    SED=${SED} \
    CHMOD=${CHMOD} \
    CMP=${CMP} \
    MAKEDEPEND=${MAKEDEPEND} \
    SORT=${SORT} \
    LANG=${LANG} \
    T=$T

# The complete list of Makefile vars passed down to sample/Makefile.
#
SAMPLE_PASSDOWN= Q="${Q}" \
    INCDIR="${INCDIR}" \
    BINDIR="${BINDIR}" \
    LIBDIR="${LIBDIR}" \
    CALC_SHAREDIR="${CALC_SHAREDIR}" \
    HELPDIR="${HELPDIR}" \
    CALC_INCDIR="${CALC_INCDIR}" \
    CUSTOMCALDIR="${CUSTOMCALDIR}" \
    CUSTOMHELPDIR="${CUSTOMHELPDIR}" \
    CUSTOMINCDIR="${CUSTOMINCDIR}" \
    SCRIPTDIR="${SCRIPTDIR}" \
    DEBUG="${DEBUG}" \
    NO_SHARED="${NO_SHARED}" \
    RANLIB="${RANLIB}" \
    PURIFY="${PURIFY}" \
    ALLOW_CUSTOM="${ALLOW_CUSTOM}" \
    CCWARN="${CCWARN}" \
    CCOPT="${CCOPT}" \
    CCMISC="${CCMISC}" \
    CFLAGS="${CFLAGS} ${ALLOW_CUSTOM}" \
    ICFLAGS="${ICFLAGS}" \
    LDFLAGS="${LDFLAGS}" \
    ILDFLAGS="${ILDFLAGS}" \
    CALC_LIBS="../libcalc.a ../custom/libcustcalc.a ${READLINE_LIB}" \
    LCC="${LCC}" \
    CC="${CC}" \
    MAKE_FILE=${MAKE_FILE} \
    SED=${SED} \
    CHMOD=${CHMOD} \
    CMP=${CMP} \
    MAKEDEPEND=${MAKEDEPEND} \
    SORT=${SORT} \
    LANG=${LANG} \
    T=$T

# The compelte list of Makefile vars passed down to help/Makefile.
#
HELP_PASSDOWN= Q="${Q}" \
    INCDIR="${INCDIR}" \
    BINDIR="${BINDIR}" \
    LIBDIR="${LIBDIR}" \
    CALC_SHAREDIR="${CALC_SHAREDIR}" \
    HELPDIR="${HELPDIR}" \
    CALC_INCDIR="${CALC_INCDIR}" \
    CUSTOMCALDIR="${CUSTOMCALDIR}" \
    CUSTOMHELPDIR="${CUSTOMHELPDIR}" \
    CUSTOMINCDIR="${CUSTOMINCDIR}" \
    SCRIPTDIR="${SCRIPTDIR}" \
    ICFLAGS="${ICFLAGS}" \
    ILDFLAGS="${ILDFLAGS}" \
    LCC="${LCC}" \
    MAKE_FILE=${MAKE_FILE} \
    SED=${SED} \
    CHMOD=${CHMOD} \
    CMP=${CMP} \
    FMT=${FMT} \
    LANG=${LANG} \
    EXT=${EXT} \
    T=$T

# The compelte list of Makefile vars passed down to cal/Makefile.
#
CAL_PASSDOWN= Q="${Q}" \
    INCDIR="${INCDIR}" \
    BINDIR="${BINDIR}" \
    LIBDIR="${LIBDIR}" \
    CALC_SHAREDIR="${CALC_SHAREDIR}" \
    HELPDIR="${HELPDIR}" \
    CALC_INCDIR="${CALC_INCDIR}" \
    CUSTOMCALDIR="${CUSTOMCALDIR}" \
    CUSTOMHELPDIR="${CUSTOMHELPDIR}" \
    CUSTOMINCDIR="${CUSTOMINCDIR}" \
    SCRIPTDIR="${SCRIPTDIR}" \
    MAKE_FILE=${MAKE_FILE} \
    CHMOD=${CHMOD} \
    CMP=${CMP} \
    LANG=${LANG} \
    T=$T

# The compelte list of Makefile vars passed down to cscript/Makefile.
#
CSCRIPT_PASSDOWN= Q="${Q}" \
    INCDIR="${INCDIR}" \
    BINDIR="${BINDIR}" \
    LIBDIR="${LIBDIR}" \
    CALC_SHAREDIR="${CALC_SHAREDIR}" \
    HELPDIR="${HELPDIR}" \
    CALC_INCDIR="${CALC_INCDIR}" \
    CUSTOMCALDIR="${CUSTOMCALDIR}" \
    CUSTOMHELPDIR="${CUSTOMHELPDIR}" \
    CUSTOMINCDIR="${CUSTOMINCDIR}" \
    SCRIPTDIR="${SCRIPTDIR}" \
    MAKE_FILE=${MAKE_FILE} \
    CHMOD=${CHMOD} \
    SED=${SED} \
    FMT=${FMT} \
    SORT=${SORT} \
    CMP=${CMP} \
    LANG=${LANG} \
    T=$T

# complete list of .h files found (but not built) in the distribution
#
H_SRC= ${LIB_H_SRC}

# complete list of .c files found (but not built) in the distribution
#
C_SRC= ${LIBSRC} ${CALCSRC} ${UTIL_C_SRC}

# The list of files that describe calc's GNU Lesser General Public License
#
LICENSE= COPYING COPYING-LGPL

# These files are found (but not built) in the distribution
#
DISTLIST= ${C_SRC} ${H_SRC} ${MAKE_FILE} BUGS CHANGES LIBRARY README \
	  README.WINDOWS calc.man HOWTO.INSTALL ${UTIL_MISC_SRC} ${LICENSE} \
	  calc.spec.in rpm.mk

# These files are used to make (but not built) a calc .a link library
#
CALCLIBLIST= ${LIBSRC} ${UTIL_C_SRC} ${LIB_H_SRC} ${MAKE_FILE} \
	     ${UTIL_MISC_SRC} BUGS CHANGES LIBRARY

# complete list of .o files
#
OBJS= ${LIBOBJS} ${CALCOBJS} ${UTIL_OBJS}

# Libaraies created and used to build calc
#
CALC_LIBS= libcalc.a custom/libcustcalc.a

# list of sample programs to that need to be built to satisfy sample/.all
#
# NOTE: This list MUST be co-ordinated with the ${SAMPLE_TARGETS} variable
#	in the sample/Makefile
#
SAMPLE_TARGETS= sample/test_random sample/many_random

# list of cscript programs to that need to be built to satisfy cscript/.all
#
# NOTE: This list MUST be co-ordinated with the ${CSCRIPT_TARGETS} variable
#	in the cscript/Makefile
#
CSCRIPT_TARGETS= cscript/mersenne cscript/piforever cscript/plus \
		 cscript/square cscript/fproduct cscript/powerterm

# complete list of progs built
#
PROGS= calc${EXT} ${UTIL_PROGS}

# complete list of targets
#
TARGETS= ${LICENSE} ${CALC_LIBS} custom/.all calc${EXT} sample/.all \
	 cal/.all help/.all help/builtin cscript/.all calc.1


###
#
# The reason for this Makefile	:-)
#
###

all: .hsrc ${TARGETS}

calc${EXT}: .hsrc ${CALC_LIBS} ${CALCOBJS}
	${RM} -f $@
	${CC} ${LDFLAGS} ${CALCOBJS} ${CALC_LIBS} ${LD_DEBUG} \
	      ${READLINE_LIB} -o $@

libcalc.a: ${LIBOBJS} ${MAKE_FILE}
	-rm -f libcalc.a
	ar qc libcalc.a ${LIBOBJS}
	${RANLIB} libcalc.a
	${CHMOD} 0644 libcalc.a

calc.1: calc.man ${MAKE_FILE}
	-rm -f calc.1
	${SED} -e 's:$${LIBDIR}:${LIBDIR}:g' \
	       -e 's,$${BINDIR},${BINDIR},g' \
	       -e 's,$${CALCPATH},${CALCPATH},g' \
	       -e 's,$${SCRIPTDIR},${SCRIPTDIR},g' \
	       -e 's,$${CALC_INCDIR},${CALC_INCDIR},g' \
	       -e 's,$${CUSTOMCALDIR},${CUSTOMCALDIR},g' \
	       -e 's,$${CUSTOMINCDIR},${CUSTOMINCDIR},g' \
	       -e 's,$${CUSTOMHELPDIR},${CUSTOMHELPDIR},g' \
	       -e 's,$${CALCRC},${CALCRC},g' < calc.man > calc.1

##
#
# Special .o files
#
##

calc.o: calc.c ${MAKE_FILE}
	${CC} ${CFLAGS} ${ALLOW_CUSTOM} -c calc.c

config.o: config.c ${MAKE_FILE}
	${CC} ${CFLAGS} ${ALLOW_CUSTOM} -c config.c

custom.o: custom.c ${MAKE_FILE}
	${CC} ${CFLAGS} ${ALLOW_CUSTOM} -c custom.c

hist.o: hist.c ${MAKE_FILE}
	${CC} ${CFLAGS} ${TERMCONTROL} ${USE_READLINE} ${READLINE_INCLUDE} -c hist.c

func.o: func.c ${MAKE_FILE}
	${CC} ${CFLAGS} ${ALLOW_CUSTOM} -c func.c

seed.o: seed.c no_implicit.arg ${MAKE_FILE}
	${CC} ${CFLAGS} `cat no_implicit.arg` -c seed.c

##
#
# The next set of rules cause the .h files BUILD_H_SRC files to be built
# according tot he system and the Makefile variables above.  The hsrc rule
# is a convenient rule to invoke to built all of the BUILD_H_SRC.
#
# We add in the BUILD_C_SRC files because they are similar to the
# BUILD_H_SRC files in terms of the build process.
#
# NOTE: Due to bogus shells found on one common system we must have
#	an non-empty else clause for every if condition.  *sigh*
#	We also place ; true at the end of some commands to avoid
#	meaningless cosmetic messages by the same system.
#
##

hsrc: ${BUILD_H_SRC} ${BUILD_C_SRC}

.hsrc: ${BUILD_H_SRC} ${BUILD_C_SRC}
	-${Q}rm -f .hsrc
	-${Q}touch .hsrc

conf.h: ${MAKE_FILE}
	-${Q}rm -f conf.h
	${Q}echo 'forming conf.h'
	${Q}echo '/*' > conf.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> conf.h
	${Q}echo ' */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '#if !defined(__CONF_H__)' >> conf.h
	${Q}echo '#define __CONF_H__' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the default :-separated search path */' >> conf.h
	${Q}echo '#if !defined(DEFAULTCALCPATH)' >> conf.h
	${Q}echo '#define DEFAULTCALCPATH "${CALCPATH}"' >> conf.h
	${Q}echo '#endif /* DEFAULTCALCPATH */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the default :-separated startup file list */' >> conf.h
	${Q}echo '#if !defined(DEFAULTCALCRC)' >> conf.h
	${Q}echo '#define DEFAULTCALCRC "${CALCRC}"' >> conf.h
	${Q}echo '#endif /* DEFAULTCALCRC */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the location of the help directory */' >> conf.h
	${Q}echo '#if !defined(HELPDIR)' >> conf.h
	${Q}echo '#define HELPDIR "${HELPDIR}"' >> conf.h
	${Q}echo '#endif /* HELPDIR */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the location of the custom help directory */' >> conf.h
	${Q}echo '#if !defined(CUSTOMHELPDIR)' >> conf.h
	${Q}echo '#define CUSTOMHELPDIR "${CUSTOMHELPDIR}"' >> conf.h
	${Q}echo '#endif /* CUSTOMHELPDIR */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* the default pager to use */' >> conf.h
	${Q}echo '#if !defined(DEFAULTCALCPAGER)' >> conf.h
	${Q}echo '#define DEFAULTCALCPAGER "${CALCPAGER}"' >> conf.h
	${Q}echo '#endif /* DEFAULTCALCPAGER */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '/* where the echo command is located */' >> conf.h
	${Q}echo '#if !defined(ECHO_PROG)' >> conf.h
	${Q}echo '#define ECHO_PROG "${ECHO}"' >> conf.h
	${Q}echo '#endif /* ECHO_PROG */' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '' >> conf.h
	${Q}echo '#endif /* !__CONF_H__ */' >> conf.h
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

endian_calc.h: endian${EXT} ${MAKE_FILE}
	-${Q}rm -f endian_calc.h
	${Q}echo 'forming endian_calc.h'
	${Q}echo '/*' > endian_calc.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> endian_calc.h
	${Q}echo ' */' >> endian_calc.h
	${Q}echo '' >> endian_calc.h
	${Q}echo '' >> endian_calc.h
	${Q}echo '#if !defined(__ENDIAN_CALC_H__)' >> endian_calc.h
	${Q}echo '#define __ENDIAN_CALC_H__' >> endian_calc.h
	${Q}echo '' >> endian_calc.h
	${Q}echo '' >> endian_calc.h
	${Q}echo '/* what byte order are we? */' >> endian_calc.h
	-${Q}if [ X"${BYTE_ORDER}" = X ]; then \
	    if [ -f ${INCDIR}/endian.h ]; then \
		echo '#include <endian.h>' >> endian_calc.h; \
		echo '#define CALC_BYTE_ORDER BYTE_ORDER' >> endian_calc.h; \
	    elif [ -f ${INCDIR}/machine/endian.h ]; then \
		echo '#include <machine/endian.h>' >> endian_calc.h; \
		echo '#define CALC_BYTE_ORDER BYTE_ORDER' >> endian_calc.h; \
	    elif [ -f ${INCDIR}/sys/endian.h ]; then \
		echo '#include <sys/endian.h>' >> endian_calc.h; \
		echo '#define CALC_BYTE_ORDER BYTE_ORDER' >> endian_calc.h; \
	    elif [ -f /usr/include/endian.h ]; then \
		echo '#include <endian.h>' >> endian_calc.h; \
		echo '#define CALC_BYTE_ORDER BYTE_ORDER' >> endian_calc.h; \
	    elif [ -f /usr/include/machine/endian.h ]; then \
		echo '#include <machine/endian.h>' >> endian_calc.h; \
		echo '#define CALC_BYTE_ORDER BYTE_ORDER' >> endian_calc.h; \
	    elif [ -f /usr/include/sys/endian.h ]; then \
		echo '#include <sys/endian.h>' >> endian_calc.h; \
		echo '#define CALC_BYTE_ORDER BYTE_ORDER' >> endian_calc.h; \
	    else \
		./endian${EXT} >> endian_calc.h; \
	    fi; \
	else \
	    ./endian${EXT} >> endian_calc.h; \
	fi
	${Q}echo '' >> endian_calc.h
	${Q}echo '' >> endian_calc.h
	${Q}echo '#endif /* !__ENDIAN_CALC_H__ */' >> endian_calc.h
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

longbits.h: longbits${EXT} ${MAKE_FILE}
	-${Q}rm -f longbits.h
	${Q}echo 'forming longbits.h'
	${Q}echo '/*' > longbits.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> longbits.h
	${Q}echo ' */' >> longbits.h
	${Q}echo '' >> longbits.h
	${Q}echo '' >> longbits.h
	${Q}echo '#if !defined(__LONGBITS_H__)' >> longbits.h
	${Q}echo '#define __LONGBITS_H__' >> longbits.h
	${Q}echo '' >> longbits.h
	${Q}echo '' >> longbits.h
	${Q}./longbits${EXT} ${LONG_BITS} >> longbits.h
	${Q}echo '' >> longbits.h
	${Q}echo '' >> longbits.h
	${Q}echo '#endif /* !__LONGBITS_H__ */' >> longbits.h
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
	${Q}echo '' >> have_malloc.h
	${Q}echo '#if !defined(__HAVE_MALLOC_H__)' >> have_malloc.h
	${Q}echo '#define __HAVE_MALLOC_H__' >> have_malloc.h
	${Q}echo '' >> have_malloc.h
	${Q}echo '' >> have_malloc.h
	${Q}echo '/* do we have <malloc.h>? */' >> have_malloc.h
	-${Q}if [ X"${HAVE_MALLOC_H}" = X"YES" ]; then \
	    echo '#define HAVE_MALLOC_H	 /* yes */' >> have_malloc.h; \
	elif [ X"${HAVE_MALLOC_H}" = X"NO" ]; then \
	    echo '#undef HAVE_MALLOC_H	/* no */' >> have_malloc.h; \
	elif [ -f ${INCDIR}/malloc.h ]; then \
	    echo '#define HAVE_MALLOC_H	 /* yes */' >> have_malloc.h; \
	elif [ -f /usr/include/malloc.h ]; then \
	    echo '#define HAVE_MALLOC_H	 /* yes */' >> have_malloc.h; \
	else \
	    echo '#undef HAVE_MALLOC_H	/* no */' >> have_malloc.h; \
	fi
	${Q}echo '' >> have_malloc.h
	${Q}echo '' >> have_malloc.h
	${Q}echo '#endif /* !__HAVE_MALLOC_H__ */' >> have_malloc.h
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
	${Q}echo '' >> have_times.h
	${Q}echo '#if !defined(__HAVE_TIMES_H__)' >> have_times.h
	${Q}echo '#define __HAVE_TIMES_H__' >> have_times.h
	${Q}echo '' >> have_times.h
	${Q}echo '' >> have_times.h
	${Q}echo '/* do we have <times.h>? */' >> have_times.h
	-${Q}if [ X"${HAVE_TIMES_H}" = X"YES" ]; then \
	    echo '#define HAVE_TIMES_H	/* yes */' >> have_times.h; \
	elif [ X"${HAVE_TIMES_H}" = X"NO" ]; then \
	    echo '#undef HAVE_TIMES_H  /* no */' >> have_times.h; \
	elif [ -f ${INCDIR}/times.h ]; then \
	    echo '#define HAVE_TIMES_H	/* yes */' >> have_times.h; \
	elif [ -f /usr/include/times.h ]; then \
	    echo '#define HAVE_TIMES_H	/* yes */' >> have_times.h; \
	else \
	    echo '#undef HAVE_TIMES_H  /* no */' >> have_times.h; \
	fi
	-${Q}if [ X"${HAVE_SYS_TIMES_H}" = X"YES" ]; then \
	    echo '#define HAVE_SYS_TIMES_H	/* yes */' >> have_times.h; \
	elif [ X"${HAVE_SYS_TIMES_H}" = X"NO" ]; then \
	    echo '#undef HAVE_SYS_TIMES_H  /* no */' >> have_times.h; \
	elif [ -f ${INCDIR}/sys/times.h ]; then \
	    echo '#define HAVE_SYS_TIMES_H  /* yes */' >> have_times.h; \
	elif [ -f /usr/include/sys/times.h ]; then \
	    echo '#define HAVE_SYS_TIMES_H  /* yes */' >> have_times.h; \
	else \
	    echo '#undef HAVE_SYS_TIMES_H  /* no */' >> have_times.h; \
	fi
	-${Q}if [ X"${HAVE_TIME_H}" = X"YES" ]; then \
	    echo '#define HAVE_TIME_H	/* yes */' >> have_times.h; \
	elif [ X"${HAVE_TIME_H}" = X"NO" ]; then \
	    echo '#undef HAVE_TIME_H  /* no */' >> have_times.h; \
	elif [ -f ${INCDIR}/time.h ]; then \
	    echo '#define HAVE_TIME_H  /* yes */' >> have_times.h; \
	elif [ -f /usr/include/time.h ]; then \
	    echo '#define HAVE_TIME_H  /* yes */' >> have_times.h; \
	else \
	    echo '#undef HAVE_TIME_H  /* no */' >> have_times.h; \
	fi
	-${Q}if [ X"${HAVE_SYS_TIME_H}" = X"YES" ]; then \
	    echo '#define HAVE_SYS_TIME_H	/* yes */' >> have_times.h; \
	elif [ X"${HAVE_SYS_TIME_H}" = X"NO" ]; then \
	    echo '#undef HAVE_SYS_TIME_H  /* no */' >> have_times.h; \
	elif [ -f ${INCDIR}/sys/time.h ]; then \
	    echo '#define HAVE_SYS_TIME_H  /* yes */' >> have_times.h; \
	elif [ -f /usr/include/sys/time.h ]; then \
	    echo '#define HAVE_SYS_TIME_H  /* yes */' >> have_times.h; \
	else \
	    echo '#undef HAVE_SYS_TIME_H  /* no */' >> have_times.h; \
	fi
	${Q}echo '' >> have_times.h
	${Q}echo '' >> have_times.h
	${Q}echo '#endif /* !__HAVE_TIMES_H__ */' >> have_times.h
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
	${Q}echo '' >> have_stdlib.h
	${Q}echo '#if !defined(__HAVE_STDLIB_H__)' >> have_stdlib.h
	${Q}echo '#define __HAVE_STDLIB_H__' >> have_stdlib.h
	${Q}echo '' >> have_stdlib.h
	${Q}echo '' >> have_stdlib.h
	${Q}echo '/* do we have <stdlib.h>? */' >> have_stdlib.h
	-${Q}if [ X"${HAVE_STDLIB_H}" = X"YES" ]; then \
	    echo '#define HAVE_STDLIB_H	/* yes */' >> have_stdlib.h; \
	elif [ X"${HAVE_STDLIB_H}" = X"NO" ]; then \
	    echo '#undef HAVE_STDLIB_H  /* no */' >> have_stdlib.h; \
	elif [ -f ${INCDIR}/stdlib.h ]; then \
	    echo '#define HAVE_STDLIB_H	 /* yes */' >> have_stdlib.h; \
	elif [ -f /usr/include/stdlib.h ]; then \
	    echo '#define HAVE_STDLIB_H	 /* yes */' >> have_stdlib.h; \
	else \
	    echo '#undef HAVE_STDLIB_H	/* no */' >> have_stdlib.h; \
	fi
	${Q}echo '' >> have_stdlib.h
	${Q}echo '' >> have_stdlib.h
	${Q}echo '#endif /* !__HAVE_STDLIB_H__ */' >> have_stdlib.h
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
	${Q}echo '' >> have_unistd.h
	${Q}echo '#if !defined(__HAVE_UNISTD_H__)' >> have_unistd.h
	${Q}echo '#define __HAVE_UNISTD_H__' >> have_unistd.h
	${Q}echo '' >> have_unistd.h
	${Q}echo '' >> have_unistd.h
	${Q}echo '/* do we have <unistd.h>? */' >> have_unistd.h
	-${Q}if [ X"${HAVE_UNISTD_H}" = X"YES" ]; then \
	    echo '#define HAVE_UNISTD_H	/* yes */' >> have_unistd.h; \
	elif [ X"${HAVE_UNISTD_H}" = X"NO" ]; then \
	    echo '#undef HAVE_UNISTD_H  /* no */' >> have_unistd.h; \
	elif [ -f ${INCDIR}/unistd.h ]; then \
	    echo '#define HAVE_UNISTD_H	 /* yes */' >> have_unistd.h; \
	elif [ -f /usr/include/unistd.h ]; then \
	    echo '#define HAVE_UNISTD_H	 /* yes */' >> have_unistd.h; \
	else \
	    echo '#undef HAVE_UNISTD_H	/* no */' >> have_unistd.h; \
	fi
	${Q}echo '' >> have_unistd.h
	${Q}echo '' >> have_unistd.h
	${Q}echo '#endif /* !__HAVE_UNISTD_H__ */' >> have_unistd.h
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
	${Q}echo '' >> have_string.h
	${Q}echo '#if !defined(__HAVE_STRING_H__)' >> have_string.h
	${Q}echo '#define __HAVE_STRING_H__' >> have_string.h
	${Q}echo '' >> have_string.h
	${Q}echo '' >> have_string.h
	${Q}echo '/* do we have <string.h>? */' >> have_string.h
	-${Q}if [ X"${HAVE_STRING_H}" = X"YES" ]; then \
	    echo '#define HAVE_STRING_H	/* yes */' >> have_string.h; \
	elif [ X"${HAVE_STRING_H}" = X"NO" ]; then \
	    echo '#undef HAVE_STRING_H  /* no */' >> have_string.h; \
	elif [ -f ${INCDIR}/string.h ]; then \
	    echo '#define HAVE_STRING_H	 /* yes */' >> have_string.h; \
	elif [ -f /usr/include/string.h ]; then \
	    echo '#define HAVE_STRING_H	 /* yes */' >> have_string.h; \
	else \
	    echo '#undef HAVE_STRING_H	/* no */' >> have_string.h; \
	fi
	${Q}echo '' >> have_string.h
	${Q}echo '' >> have_string.h
	${Q}echo '#endif /* !__HAVE_STRING_H__ */' >> have_string.h
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
	${Q}echo '' >> terminal.h
	${Q}echo '#if !defined(__TERMINAL_H__)' >> terminal.h
	${Q}echo '#define __TERMINAL_H__' >> terminal.h
	${Q}echo '' >> terminal.h
	${Q}echo '' >> terminal.h
	${Q}echo '/* determine the type of terminal interface */' >> terminal.h
	${Q}echo '#if !defined(USE_TERMIOS)' >> terminal.h
	${Q}echo '#if !defined(USE_TERMIO)' >> terminal.h
	${Q}echo '#if !defined(USE_SGTTY)' >> terminal.h
	-${Q}if [ X"${TERMCONTROL}" = X"-DUSE_WIN32" ]; then \
	    echo '/* windoz, use none of these modes */' >> terminal.h; \
	    echo '#undef USE_TERMIOS   /* <termios.h> */' >> terminal.h; \
	    echo '#undef USE_TERMIO    /* <termio.h> */' >> terminal.h; \
	    echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	elif [ -f ${INCDIR}/termios.h ]; then \
	    echo '/* use termios */' >> terminal.h; \
	    echo '#define USE_TERMIOS  /* <termios.h> */' >> terminal.h; \
	    echo '#undef USE_TERMIO    /* <termio.h> */' >> terminal.h; \
	    echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	elif [ -f ${INCDIR}/termio.h ]; then \
	    echo '/* use termio */' >> terminal.h; \
	    echo '#undef USE_TERMIOS   /* <termios.h> */' >> terminal.h; \
	    echo '#define USE_TERMIO   /* <termio.h> */' >> terminal.h; \
	    echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	elif [ -f /usr/include/termios.h ]; then \
	    echo '/* use termios */' >> terminal.h; \
	    echo '#define USE_TERMIOS  /* <termios.h> */' >> terminal.h; \
	    echo '#undef USE_TERMIO    /* <termio.h> */' >> terminal.h; \
	    echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	elif [ -f /usr/include/termio.h ]; then \
	    echo '/* use termio */' >> terminal.h; \
	    echo '#undef USE_TERMIOS   /* <termios.h> */' >> terminal.h; \
	    echo '#define USE_TERMIO   /* <termio.h> */' >> terminal.h; \
	    echo '#undef USE_SGTTY     /* <sys/ioctl.h> */' >> terminal.h; \
	else \
	    echo '/* use sgtty */' >> terminal.h; \
	    echo '#undef USE_TERMIOS   /* <termios.h> */' >> terminal.h; \
	    echo '#undef USE_TERMIO    /* <termio.h> */' >> terminal.h; \
	    echo '#define USE_SGTTY    /* <sys/ioctl.h> */' >> terminal.h; \
	fi
	${Q}echo '#endif' >> terminal.h
	${Q}echo '#endif' >> terminal.h
	${Q}echo '#endif' >> terminal.h
	${Q}echo '' >> terminal.h
	${Q}echo '' >> terminal.h
	${Q}echo '#endif /* !__TERMINAL_H__ */' >> terminal.h
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

have_fpos.h: have_fpos.c ${MAKE_FILE}
	-${Q}rm -f fpos_tmp have_fpos.h
	${Q}echo 'forming have_fpos.h'
	${Q}echo '/*' > have_fpos.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_fpos.h
	${Q}echo ' */' >> have_fpos.h
	${Q}echo '' >> have_fpos.h
	${Q}echo '' >> have_fpos.h
	${Q}echo '#if !defined(__HAVE_FPOS_H__)' >> have_fpos.h
	${Q}echo '#define __HAVE_FPOS_H__' >> have_fpos.h
	${Q}echo '' >> have_fpos.h
	${Q}echo '' >> have_fpos.h
	${Q}echo '/* do we have fgetpos & fsetpos functions? */' >> have_fpos.h
	-${Q}rm -f have_fpos.o have_fpos${EXT}
	-${Q}${LCC} ${HAVE_FPOS} ${ICFLAGS} have_fpos.c -c >/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_fpos.o -o have_fpos${EXT} >/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_fpos${EXT} > fpos_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s fpos_tmp ]; then \
	    cat fpos_tmp >> have_fpos.h; \
	else \
	    echo '#undef HAVE_FPOS  /* no */' >> have_fpos.h; \
	    echo '' >> have_fpos.h; \
	    echo 'typedef long FILEPOS;' >> have_fpos.h; \
	fi
	${Q}echo '' >> have_fpos.h
	${Q}echo '' >> have_fpos.h
	${Q}echo '#endif /* !__HAVE_FPOS_H__ */' >> have_fpos.h
	-${Q}rm -f have_fpos${EXT} have_fpos.o fpos_tmp
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

have_fpos_pos.h: have_fpos_pos.c have_fpos.h have_posscl.h ${MAKE_FILE}
	-${Q}rm -f fpos_tmp have_fpos_pos.h
	${Q}echo 'forming have_fpos_pos.h'
	${Q}echo '/*' > have_fpos_pos.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' \
		>> have_fpos_pos.h
	${Q}echo ' */' >> have_fpos_pos.h
	${Q}echo '' >> have_fpos_pos.h
	${Q}echo '' >> have_fpos_pos.h
	${Q}echo '#if !defined(__HAVE_FPOS_POS_H__)' >> have_fpos_pos.h
	${Q}echo '#define __HAVE_FPOS_POS_H__' >> have_fpos_pos.h
	${Q}echo '' >> have_fpos_pos.h
	${Q}echo '' >> have_fpos_pos.h
	${Q}echo '/* do we have fgetpos & fsetpos functions? */' \
		>> have_fpos_pos.h
	-${Q}rm -f have_fpos_pos.o have_fpos_pos${EXT}
	-${Q}${LCC} ${HAVE_FPOS} ${HAVE_FPOS_POS} \
		    ${ICFLAGS} have_fpos_pos.c -c >/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_fpos_pos.o -o have_fpos_pos${EXT} \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_fpos_pos${EXT} > fpos_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s fpos_tmp ]; then \
	    cat fpos_tmp >> have_fpos_pos.h; \
	else \
	    echo '#undef HAVE_FPOS_POS  /* no */' >> have_fpos_pos.h; \
	    echo '' >> have_fpos_pos.h; \
	    echo '#undef FPOS_POS_BITS' >> have_fpos_pos.h; \
	fi
	${Q}echo '' >> have_fpos_pos.h
	${Q}echo '' >> have_fpos_pos.h
	${Q}echo '#endif /* !__HAVE_FPOS_POS_H__ */' >> have_fpos_pos.h
	-${Q}rm -f have_fpos_pos${EXT} have_fpos_pos.o fpos_tmp
	${Q}echo 'have_fpos_pos.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

fposval.h: fposval.c have_fpos.h have_fpos_pos.h have_offscl.h have_posscl.h \
	   endian_calc.h ${MAKE_FILE}
	-${Q}rm -f fposv_tmp fposval.h
	${Q}echo 'forming fposval.h'
	${Q}echo '/*' > fposval.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> fposval.h
	${Q}echo ' */' >> fposval.h
	${Q}echo '' >> fposval.h
	${Q}echo '' >> fposval.h
	${Q}echo '#if !defined(__FPOSVAL_H__)' >> fposval.h
	${Q}echo '#define __FPOSVAL_H__' >> fposval.h
	${Q}echo '' >> fposval.h
	${Q}echo '' >> fposval.h
	${Q}echo '/* what are our file position & size types? */' >> fposval.h
	-${Q}rm -f fposval.o fposval${EXT}
	-${Q}${LCC} ${ICFLAGS} ${FPOS_BITS} ${OFF_T_BITS} \
		    ${DEV_BITS} ${INODE_BITS} fposval.c -c >/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} fposval.o -o fposval${EXT} >/dev/null 2>&1; true
	${Q}${SHELL} -c "./fposval${EXT} fposv_tmp >> fposval.h 2>/dev/null" \
	    >/dev/null 2>&1; true
	${Q}echo '' >> fposval.h
	${Q}echo '' >> fposval.h
	${Q}echo '#endif /* !__FPOSVAL_H__ */' >> fposval.h
	-${Q}rm -f fposval${EXT} fposval.o fposv_tmp
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
	-${Q}rm -f have_const const_tmp have_const.h
	${Q}echo 'forming have_const.h'
	${Q}echo '/*' > have_const.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_const.h
	${Q}echo ' */' >> have_const.h
	${Q}echo '' >> have_const.h
	${Q}echo '' >> have_const.h
	${Q}echo '#if !defined(__HAVE_CONST_H__)' >> have_const.h
	${Q}echo '#define __HAVE_CONST_H__' >> have_const.h
	${Q}echo '' >> have_const.h
	${Q}echo '' >> have_const.h
	${Q}echo '/* do we have or want const? */' >> have_const.h
	-${Q}rm -f have_const.o have_const${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_CONST} have_const.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_const.o -o have_const${EXT} >/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_const${EXT} > const_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s const_tmp ]; then \
	    cat const_tmp >> have_const.h; \
	else \
	    echo '#undef HAVE_CONST /* no */' >> have_const.h; \
	    echo '#undef CONST' >> have_const.h; \
	    echo '#define CONST /* no */' >> have_const.h; \
	fi
	${Q}echo '' >> have_const.h
	${Q}echo '' >> have_const.h
	${Q}echo '#endif /* !__HAVE_CONST_H__ */' >> have_const.h
	-${Q}rm -f have_const${EXT} have_const.o const_tmp
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

have_offscl.h: have_offscl.c ${MAKE_FILE}
	-${Q}rm -f offscl_tmp have_offscl.h
	${Q}echo 'forming have_offscl.h'
	${Q}echo '/*' > have_offscl.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_offscl.h
	${Q}echo ' */' >> have_offscl.h
	${Q}echo '' >> have_offscl.h
	${Q}echo '' >> have_offscl.h
	${Q}echo '#if !defined(__HAVE_OFFSCL_H__)' >> have_offscl.h
	${Q}echo '#define __HAVE_OFFSCL_H__' >> have_offscl.h
	${Q}echo '' >> have_offscl.h
	${Q}echo '' >> have_offscl.h
	-${Q}rm -f have_offscl.o have_offscl${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_OFFSCL} have_offscl.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_offscl.o -o have_offscl${EXT} \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_offscl${EXT} > offscl_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s offscl_tmp ]; then \
	    cat offscl_tmp >> have_offscl.h; \
	else \
	    echo '#undef HAVE_OFF_T_SCALAR /* off_t is not a simple value */' \
		>> have_offscl.h; \
	fi
	${Q}echo '' >> have_offscl.h
	${Q}echo '' >> have_offscl.h
	${Q}echo '#endif /* !__HAVE_OFFSCL_H__ */' >> have_offscl.h
	-${Q}rm -f have_offscl${EXT} have_offscl.o offscl_tmp
	${Q}echo 'have_offscl.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_posscl.h: have_posscl.c have_fpos.h ${MAKE_FILE}
	-${Q}rm -f have_posscl have_posscl.o posscl_tmp have_posscl.h
	${Q}echo 'forming have_posscl.h'
	${Q}echo '/*' > have_posscl.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_posscl.h
	${Q}echo ' */' >> have_posscl.h
	${Q}echo '' >> have_posscl.h
	${Q}echo '' >> have_posscl.h
	${Q}echo '#if !defined(__HAVE_POSSCL_H__)' >> have_posscl.h
	${Q}echo '#define __HAVE_POSSCL_H__' >> have_posscl.h
	${Q}echo '' >> have_posscl.h
	${Q}echo '' >> have_posscl.h
	-${Q}rm -f have_posscl.o have_posscl
	-${Q}${LCC} ${ICFLAGS} ${HAVE_POSSCL} have_posscl.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_posscl.o -o have_posscl \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_posscl > posscl_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s posscl_tmp ]; then \
	    cat posscl_tmp >> have_posscl.h; \
	else \
	    echo '/* FILEPOS is not a simple value */' >> have_posscl.h; \
	    echo '#undef HAVE_FILEPOS_SCALAR' >> have_posscl.h; \
	fi
	${Q}echo '' >> have_posscl.h
	${Q}echo '' >> have_posscl.h
	${Q}echo '#endif /* !__HAVE_POSSCL_H__ */' >> have_posscl.h
	-${Q}rm -f have_posscl have_posscl.o posscl_tmp
	${Q}echo 'have_posscl.h formed'
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
	-${Q}rm -f align32 align32_tmp align32.h
	${Q}echo 'forming align32.h'
	${Q}echo '/*' > align32.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> align32.h
	${Q}echo ' */' >> align32.h
	${Q}echo '' >> align32.h
	${Q}echo '' >> align32.h
	${Q}echo '#if !defined(__MUST_ALIGN32_H__)' >> align32.h
	${Q}echo '#define __MUST_ALIGN32_H__' >> align32.h
	${Q}echo '' >> align32.h
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
	    rm -f align32.o align32${EXT}; \
	    ${LCC} ${ICFLAGS} ${ALIGN32} align32.c -c >/dev/null 2>&1; \
	    ${LCC} ${ILDFLAGS} align32.o -o align32${EXT} >/dev/null 2>&1; \
	    ${SHELL} -c "./align32${EXT} >align32_tmp 2>/dev/null" >/dev/null 2>&1; \
	    if [ -s align32_tmp ]; then \
		cat align32_tmp >> align32.h; \
	    else \
		echo '/* guess we must align 32 bit values */' >> align32.h; \
		echo '#define MUST_ALIGN32' >> align32.h; \
	    fi; \
	    rm -f align32${EXT} align32.o align32_tmp core; \
	else \
	    true; \
	fi
	${Q}echo '' >> align32.h
	${Q}echo '' >> align32.h
	${Q}echo '#endif /* !__MUST_ALIGN32_H__ */' >> align32.h
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
	-${Q}rm -f have_uid_t uid_tmp have_uid_t.h
	${Q}echo 'forming have_uid_t.h'
	${Q}echo '/*' > have_uid_t.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_uid_t.h
	${Q}echo ' */' >> have_uid_t.h
	${Q}echo '' >> have_uid_t.h
	${Q}echo '' >> have_uid_t.h
	${Q}echo '#if !defined(__HAVE_UID_T_H__)' >> have_uid_t.h
	${Q}echo '#define __HAVE_UID_T_H__' >> have_uid_t.h
	${Q}echo '' >> have_uid_t.h
	${Q}echo '' >> have_uid_t.h
	${Q}echo '/* do we have or want uid_t? */' >> have_uid_t.h
	-${Q}rm -f have_uid_t.o have_uid_t${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_UID_T} have_uid_t.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_uid_t.o -o have_uid_t${EXT} >/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_uid_t${EXT} > uid_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s uid_tmp ]; then \
	    cat uid_tmp >> have_uid_t.h; \
	else \
	    echo '#undef HAVE_UID_T /* no */' >> have_uid_t.h; \
	fi
	${Q}echo '' >> have_uid_t.h
	${Q}echo '' >> have_uid_t.h
	${Q}echo '#endif /* !__HAVE_UID_T_H__ */' >> have_uid_t.h
	-${Q}rm -f have_uid_t${EXT} have_uid_t.o uid_tmp
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
	-${Q}rm -f newstr_tmp have_newstr.h
	${Q}echo 'forming have_newstr.h'
	${Q}echo '/*' > have_newstr.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_newstr.h
	${Q}echo ' */' >> have_newstr.h
	${Q}echo '' >> have_newstr.h
	${Q}echo '' >> have_newstr.h
	${Q}echo '#if !defined(__HAVE_NEWSTR_H__)' >> have_newstr.h
	${Q}echo '#define __HAVE_NEWSTR_H__' >> have_newstr.h
	${Q}echo '' >> have_newstr.h
	${Q}echo '' >> have_newstr.h
	${Q}echo '/* do we have or want memcpy(), memset() & strchr()? */' \
							>> have_newstr.h
	-${Q}rm -f have_newstr.o have_newstr${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_NEWSTR} have_newstr.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_newstr.o -o have_newstr${EXT} \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_newstr${EXT} > newstr_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s newstr_tmp ]; then \
	    cat newstr_tmp >> have_newstr.h; \
	else \
	    echo '#undef HAVE_NEWSTR /* no */' >> have_newstr.h; \
	fi
	${Q}echo '' >> have_newstr.h
	${Q}echo '' >> have_newstr.h
	${Q}echo '#endif /* !__HAVE_NEWSTR_H__ */' >> have_newstr.h
	-${Q}rm -f have_newstr${EXT} have_newstr.o newstr_tmp
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

have_memmv.h: have_memmv.c ${MAKE_FILE}
	-${Q}rm -f have_memmv have_memmv.o memmv_tmp have_memmv.h
	${Q}echo 'forming have_memmv.h'
	${Q}echo '/*' > have_memmv.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_memmv.h
	${Q}echo ' */' >> have_memmv.h
	${Q}echo '' >> have_memmv.h
	${Q}echo '' >> have_memmv.h
	${Q}echo '#if !defined(__HAVE_MEMMV_H__)' >> have_memmv.h
	${Q}echo '#define __HAVE_MEMMV_H__' >> have_memmv.h
	${Q}echo '' >> have_memmv.h
	${Q}echo '' >> have_memmv.h
	${Q}echo '/* do we have or want memmove()? */' >> have_memmv.h
	-${Q}rm -f have_memmv.o have_memmv
	-${Q}${LCC} ${ICFLAGS} ${HAVE_MEMMOVE} have_memmv.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_memmv.o -o have_memmv \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_memmv > memmv_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s memmv_tmp ]; then \
	    cat memmv_tmp >> have_memmv.h; \
	else \
	    echo '#undef HAVE_MEMMOVE /* no */' >> have_memmv.h; \
	fi
	${Q}echo '' >> have_memmv.h
	${Q}echo '' >> have_memmv.h
	${Q}echo '#endif /* !__HAVE_MEMMV_H__ */' >> have_memmv.h
	-${Q}rm -f have_memmv have_memmv.o memmv_tmp
	${Q}echo 'have_memmv.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_ustat.h: have_ustat.c ${MAKE_FILE}
	-${Q}rm -f ustat_tmp have_ustat.h
	${Q}echo 'forming have_ustat.h'
	${Q}echo '/*' > have_ustat.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_ustat.h
	${Q}echo ' */' >> have_ustat.h
	${Q}echo '' >> have_ustat.h
	${Q}echo '' >> have_ustat.h
	${Q}echo '#if !defined(__HAVE_USTAT_H__)' >> have_ustat.h
	${Q}echo '#define __HAVE_USTAT_H__' >> have_ustat.h
	${Q}echo '' >> have_ustat.h
	${Q}echo '' >> have_ustat.h
	${Q}echo '/* do we have or want ustat()? */' >> have_ustat.h
	-${Q}rm -f have_ustat.o have_ustat${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_USTAT} have_ustat.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_ustat.o -o have_ustat${EXT} \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_ustat${EXT} > ustat_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s ustat_tmp ]; then \
	    cat ustat_tmp >> have_ustat.h; \
	else \
	    echo '#undef HAVE_USTAT /* no */' >> have_ustat.h; \
	fi
	${Q}echo '' >> have_ustat.h
	${Q}echo '' >> have_ustat.h
	${Q}echo '#endif /* !__HAVE_USTAT_H__ */' >> have_ustat.h
	-${Q}rm -f have_ustat${EXT} have_ustat.o ustat_tmp
	${Q}echo 'have_ustat.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_getsid.h: have_getsid.c ${MAKE_FILE}
	-${Q}rm -f getsid_tmp have_getsid.h
	${Q}echo 'forming have_getsid.h'
	${Q}echo '/*' > have_getsid.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_getsid.h
	${Q}echo ' */' >> have_getsid.h
	${Q}echo '' >> have_getsid.h
	${Q}echo '' >> have_getsid.h
	${Q}echo '#if !defined(__HAVE_GETSID_H__)' >> have_getsid.h
	${Q}echo '#define __HAVE_GETSID_H__' >> have_getsid.h
	${Q}echo '' >> have_getsid.h
	${Q}echo '' >> have_getsid.h
	${Q}echo '/* do we have or want getsid()? */' >> have_getsid.h
	-${Q}rm -f have_getsid.o have_getsid${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_GETSID} have_getsid.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_getsid.o -o have_getsid \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_getsid${EXT} > getsid_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s getsid_tmp ]; then \
	    cat getsid_tmp >> have_getsid.h; \
	else \
	    echo '#undef HAVE_GETSID /* no */' >> have_getsid.h; \
	fi
	${Q}echo '' >> have_getsid.h
	${Q}echo '' >> have_getsid.h
	${Q}echo '#endif /* !__HAVE_GETSID_H__ */' >> have_getsid.h
	-${Q}rm -f have_getsid${EXT} have_getsid.o getsid_tmp
	${Q}echo 'have_getsid.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_getpgid.h: have_getpgid.c ${MAKE_FILE}
	-${Q}rm -f getpgid_tmp have_getpgid.h
	${Q}echo 'forming have_getpgid.h'
	${Q}echo '/*' > have_getpgid.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_getpgid.h
	${Q}echo ' */' >> have_getpgid.h
	${Q}echo '' >> have_getpgid.h
	${Q}echo '' >> have_getpgid.h
	${Q}echo '#if !defined(__HAVE_GETPGID_H__)' >> have_getpgid.h
	${Q}echo '#define __HAVE_GETPGID_H__' >> have_getpgid.h
	${Q}echo '' >> have_getpgid.h
	${Q}echo '' >> have_getpgid.h
	${Q}echo '/* do we have or want getpgid()? */' >> have_getpgid.h
	-${Q}rm -f have_getpgid.o have_getpgid${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_GETPGID} have_getpgid.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_getpgid.o -o have_getpgid${EXT} \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_getpgid${EXT} > getpgid_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s getpgid_tmp ]; then \
	    cat getpgid_tmp >> have_getpgid.h; \
	else \
	    echo '#undef HAVE_GETPGID /* no */' >> have_getpgid.h; \
	fi
	${Q}echo '' >> have_getpgid.h
	${Q}echo '' >> have_getpgid.h
	${Q}echo '#endif /* !__HAVE_GETPGID_H__ */' >> have_getpgid.h
	-${Q}rm -f have_getpgid${EXT} have_getpgid.o getpgid_tmp
	${Q}echo 'have_getpgid.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_gettime.h: have_gettime.c ${MAKE_FILE}
	-${Q}rm -f gettime_tmp have_gettime.h
	${Q}echo 'forming have_gettime.h'
	${Q}echo '/*' > have_gettime.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_gettime.h
	${Q}echo ' */' >> have_gettime.h
	${Q}echo '' >> have_gettime.h
	${Q}echo '' >> have_gettime.h
	${Q}echo '#if !defined(__HAVE_GETTIME_H__)' >> have_gettime.h
	${Q}echo '#define __HAVE_GETTIME_H__' >> have_gettime.h
	${Q}echo '' >> have_gettime.h
	${Q}echo '' >> have_gettime.h
	${Q}echo '/* do we have or want clock_gettime()? */' >> have_gettime.h
	-${Q}rm -f have_gettime.o have_gettime${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_GETTIME} have_gettime.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_gettime.o -o have_gettime \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_gettime${EXT} > gettime_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s gettime_tmp ]; then \
	    cat gettime_tmp >> have_gettime.h; \
	else \
	    echo '#undef HAVE_GETTIME /* no */' >> have_gettime.h; \
	fi
	${Q}echo '' >> have_gettime.h
	${Q}echo '' >> have_gettime.h
	${Q}echo '#endif /* !__HAVE_GETTIME_H__ */' >> have_gettime.h
	-${Q}rm -f have_gettime${EXT} have_gettime.o gettime_tmp
	${Q}echo 'have_gettime.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_getprid.h: have_getprid.c ${MAKE_FILE}
	-${Q}rm -f getprid_tmp have_getprid.h
	${Q}echo 'forming have_getprid.h'
	${Q}echo '/*' > have_getprid.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_getprid.h
	${Q}echo ' */' >> have_getprid.h
	${Q}echo '' >> have_getprid.h
	${Q}echo '' >> have_getprid.h
	${Q}echo '#if !defined(__HAVE_GETPRID_H__)' >> have_getprid.h
	${Q}echo '#define __HAVE_GETPRID_H__' >> have_getprid.h
	${Q}echo '' >> have_getprid.h
	${Q}echo '' >> have_getprid.h
	${Q}echo '/* do we have or want getprid()? */' >> have_getprid.h
	-${Q}rm -f have_getprid.o have_getprid${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_GETPRID} have_getprid.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_getprid.o -o have_getprid${EXT} \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_getprid${EXT} > getprid_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s getprid_tmp ]; then \
	    cat getprid_tmp >> have_getprid.h; \
	else \
	    echo '#undef HAVE_GETPRID /* no */' >> have_getprid.h; \
	fi
	${Q}echo '' >> have_getprid.h
	${Q}echo '' >> have_getprid.h
	${Q}echo '#endif /* !__HAVE_GETPRID_H__ */' >> have_getprid.h
	-${Q}rm -f have_getprid${EXT} have_getprid.o getprid_tmp
	${Q}echo 'have_getprid.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_urandom.h: ${MAKE_FILE}
	-${Q}rm -f have_urandom.h
	${Q}echo 'forming have_urandom.h'
	${Q}echo '/*' > have_urandom.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_urandom.h
	${Q}echo ' */' >> have_urandom.h
	${Q}echo '' >> have_urandom.h
	${Q}echo '' >> have_urandom.h
	${Q}echo '#if !defined(__HAVE_URANDOM_H__)' >> have_urandom.h
	${Q}echo '#define __HAVE_URANDOM_H__' >> have_urandom.h
	${Q}echo '' >> have_urandom.h
	${Q}echo '' >> have_urandom.h
	${Q}echo '/* do we have /dev/urandom? */' >> have_urandom.h
	-${Q}if [ X"${HAVE_URANDOM_H}" = X"YES" ]; then \
	    echo '#define HAVE_URANDOM_H	/* yes */' >> have_urandom.h; \
	elif [ X"${HAVE_URANDOM_H}" = X"NO" ]; then \
	    echo '#undef HAVE_URANDOM_H  /* no */' >> have_urandom.h; \
	elif [ -e /dev/urandom ] 2>/dev/null; then \
	    echo '#define HAVE_URANDOM_H  /* yes */' >> have_urandom.h; \
	else \
	    echo '#undef HAVE_URANDOM_H	 /* no */' >> have_urandom.h; \
	fi
	${Q}echo '' >> have_urandom.h
	${Q}echo '' >> have_urandom.h
	${Q}echo '#endif /* !__HAVE_URANDOM_H__ */' >> have_urandom.h
	${Q}echo 'have_urandom.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_rusage.h: have_rusage.c ${MAKE_FILE}
	-${Q}rm -f rusage_tmp have_rusage.h
	${Q}echo 'forming have_rusage.h'
	${Q}echo '/*' > have_rusage.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_rusage.h
	${Q}echo ' */' >> have_rusage.h
	${Q}echo '' >> have_rusage.h
	${Q}echo '' >> have_rusage.h
	${Q}echo '#if !defined(__HAVE_RUSAGE_H__)' >> have_rusage.h
	${Q}echo '#define __HAVE_RUSAGE_H__' >> have_rusage.h
	${Q}echo '' >> have_rusage.h
	${Q}echo '' >> have_rusage.h
	${Q}echo '/* do we have or want getrusage()? */' >> have_rusage.h
	-${Q}rm -f have_rusage.o have_rusage${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_GETRUSAGE} have_rusage.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_rusage.o -o have_rusage${EXT} \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_rusage${EXT} > rusage_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s rusage_tmp ]; then \
	    cat rusage_tmp >> have_rusage.h; \
	else \
	    echo '#undef HAVE_GETRUSAGE /* no */' >> have_rusage.h; \
	fi
	${Q}echo '' >> have_rusage.h
	${Q}echo '' >> have_rusage.h
	${Q}echo '#endif /* !__HAVE_RUSAGE_H__ */' >> have_rusage.h
	-${Q}rm -f have_rusage${EXT} have_rusage.o rusage_tmp
	${Q}echo 'have_rusage.h formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_strdup.h: have_strdup.c ${MAKE_FILE}
	-${Q}rm -f strdup_tmp have_strdup.h
	${Q}echo 'forming have_strdup.h'
	${Q}echo '/*' > have_strdup.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_strdup.h
	${Q}echo ' */' >> have_strdup.h
	${Q}echo '' >> have_strdup.h
	${Q}echo '' >> have_strdup.h
	${Q}echo '#if !defined(__HAVE_RUSAGE_H__)' >> have_strdup.h
	${Q}echo '#define __HAVE_RUSAGE_H__' >> have_strdup.h
	${Q}echo '' >> have_strdup.h
	${Q}echo '' >> have_strdup.h
	${Q}echo '/* do we have or want getstrdup()? */' >> have_strdup.h
	-${Q}rm -f have_strdup.o have_strdup${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_STRDUP} have_strdup.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_strdup.o -o have_strdup \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_strdup${EXT} > strdup_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s strdup_tmp ]; then \
	    cat strdup_tmp >> have_strdup.h; \
	else \
	    echo '#undef HAVE_STRDUP /* no */' >> have_strdup.h; \
	fi
	${Q}echo '' >> have_strdup.h
	${Q}echo '' >> have_strdup.h
	${Q}echo '#endif /* !__HAVE_RUSAGE_H__ */' >> have_strdup.h
	-${Q}rm -f have_strdup${EXT} have_strdup.o strdup_tmp
	${Q}echo 'have_strdup.h formed'
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
	-${Q}rm -f args.h
	${Q}echo 'forming args.h'
	${Q}echo '/*' > args.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> args.h
	${Q}echo ' */' >> args.h
	${Q}echo '' >> args.h
	${Q}echo '' >> args.h
	${Q}echo '#if !defined(__ARGS_H__)' >> args.h
	${Q}echo '#define __ARGS_H__' >> args.h
	${Q}echo '' >> args.h
	${Q}echo '' >> args.h
	-${Q}rm -f have_stdvs.o have_stdvs${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_VSPRINTF} have_stdvs.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_stdvs.o -o have_stdvs${EXT} >/dev/null 2>&1; true
	-${Q}if ./have_stdvs${EXT} >>args.h 2>/dev/null; then \
	    touch have_args.sh; \
	else \
	    true; \
	fi
	-${Q}if [ ! -f have_args.sh ] && [ X"${HAVE_VSPRINTF}" = X ]; then \
	    rm -f have_stdvs.o have_stdvs${EXT} have_varvs.o have_varvs${EXT}; \
	    ${LCC} ${ICFLAGS} -DDONT_HAVE_VSPRINTF have_varvs.c -c \
		    2>/dev/null; \
	    ${LCC} ${ILDFLAGS} have_varvs.o -o have_varvs${EXT} 2>/dev/null; \
	    if ./have_varvs${EXT} >>args.h 2>/dev/null; then \
		touch have_args.sh; \
	    else \
		true; \
	    fi; \
	else \
	    true; \
	fi
	-${Q}if [ -f have_args.sh ]; then \
	    echo 'exit 0' > have_args.sh; \
	else \
	    echo 'exit 1' > have_args.sh; \
	    echo "Unable to determine what type of variable args and"; \
	    echo "what type of vsprintf() should be used.  Set or change"; \
	    echo "the Makefile variable HAVE_VSPRINTF."; \
	fi
	${Q}sh ./have_args.sh
	${Q}echo '' >> args.h
	${Q}echo '' >> args.h
	${Q}echo '#endif /* !__ARGS_H__ */' >> args.h
	-${Q}rm -f have_stdvs.o have_varvs.o have_varvs${EXT} have_args.sh core
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
	${Q}echo '' >> calcerr.h
	${Q}echo '#if !defined(__CALCERR_H__)' >> calcerr.h
	${Q}echo '#define __CALCERR_H__' >> calcerr.h
	${Q}echo '' >> calcerr.h
	${Q}echo '' >> calcerr.h
	${Q}${SED} -f calcerr_h.sed < calcerr.tbl | \
	    ${AWK} -f calcerr_h.awk >> calcerr.h
	${Q}echo '' >> calcerr.h
	${Q}echo '' >> calcerr.h
	${Q}echo '#endif /* !__CALCERR_H__ */' >> calcerr.h
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

no_implicit.arg: no_implicit.c ${MAKE_FILE}
	-${Q}rm -f no_implicit${EXT} no_implicit.o no_implicit.arg
	${Q}echo 'forming no_implicit.arg'
	-${Q}if [ X"${HAVE_NO_IMPLICIT}" = X"YES" ]; then \
	     echo ""'-Wno-implicit' > no_implicit.arg; \
	elif [ X"${HAVE_NO_IMPLICIT}" = X"NO" ]; then \
	    touch no_implicit.arg; \
	else \
	    ${LCC} -Wno-implicit ${ICFLAGS} -DHAVE_NO_IMPLICIT \
		    no_implicit.c -c >/dev/null 2>&1; \
	    ${LCC} ${ILDFLAGS} no_implicit.o -o no_implicit${EXT} >/dev/null 2>&1; \
	    ${SHELL} -c "./no_implicit${EXT} > no_implicit.arg 2>/dev/null" \
	    	>/dev/null 2>&1; true; \
	fi
	${Q}echo 'no_implicit.arg formed'
	-@if [ -z "${Q}" ]; then \
	    echo ''; \
	    echo '=-=-= start of $@ =-=-='; \
	    cat $@; \
	    echo '=-=-= end of $@ =-=-='; \
	    echo ''; \
	else \
	    true; \
	fi

have_unused.h: have_unused.c ${MAKE_FILE}
	-${Q}rm -f unused_tmp have_unused.h
	${Q}echo 'forming have_unused.h'
	${Q}echo '/*' > have_unused.h
	${Q}echo ' * DO NOT EDIT -- generated by the Makefile' >> have_unused.h
	${Q}echo ' */' >> have_unused.h
	${Q}echo '' >> have_unused.h
	${Q}echo '' >> have_unused.h
	${Q}echo '#if !defined(__HAVE_UNUSED_H__)' >> have_unused.h
	${Q}echo '#define __HAVE_UNUSED_H__' >> have_unused.h
	${Q}echo '' >> have_unused.h
	${Q}echo '' >> have_unused.h
	${Q}echo '/* do we have/want the unused attribute? */' >> have_unused.h
	-${Q}rm -f have_unused.o have_unused${EXT}
	-${Q}${LCC} ${ICFLAGS} ${HAVE_UNUSED} have_unused.c -c \
		>/dev/null 2>&1; true
	-${Q}${LCC} ${ILDFLAGS} have_unused.o -o have_unused \
		>/dev/null 2>&1; true
	-${Q}${SHELL} -c "./have_unused${EXT} > unused_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-${Q}if [ -s unused_tmp ]; then \
	    cat unused_tmp >> have_unused.h; \
	else \
	    echo '#undef HAVE_UNUSED /* no */' >> have_unused.h; \
	fi
	${Q}echo '' >> have_unused.h
	${Q}echo '' >> have_unused.h
	${Q}echo '#endif /* !__HAVE_UNUSED_H__ */' >> have_unused.h
	-${Q}rm -f have_unused${EXT} have_unused.o unused_tmp
	${Q}echo 'have_unused.h formed'
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
# Build .h files for windoz based systems
#
# This is really a internal utility rule that is used to create the
# win32 sub-directory for distribution.
#
##

win32_hsrc: ${MAKE_FILE} win32.mkdef
	${Q}echo 'forming win32 directory'
	${Q}rm -rf win32
	${Q}${MKDIR} win32
	${Q}cp ${UTIL_C_SRC} win32
	${Q}cp ${UTIL_MISC_SRC} Makefile win32
	${Q}(cd win32; \
	 echo "cd win32"; \
	 echo "$(MAKE) hsrc `cat win32.mkdef` EXT="; \
	 $(MAKE) hsrc `cat win32.mkdef` EXT=; \
	 rm -f ${UTIL_C_SRC}; \
	 rm -f ${UTIL_MISC_SRC}; \
	 rm -f ${UTIL_OBJS}; \
	 rm -f ${UTIL_PROGS}; \
	 rm -f Makefile)
	${Q}echo 'win32 directory formed'


##
#
# These rules are used in the process of building the BUILD_H_SRC.
#
##

endian.o: endian.c have_unistd.h
	${LCC} ${ICFLAGS} ${BYTE_ORDER} endian.c -c

endian${EXT}: endian.o
	${RM} -f $@
	${LCC} ${ICFLAGS} endian.o -o $@

longbits.o: longbits.c have_unistd.h
	${LCC} ${ICFLAGS} longbits.c -c

longbits${EXT}: longbits.o
	${RM} -f $@
	${LCC} ${ICFLAGS} longbits.o -o $@

##
#
# These two .all rules are used to determine of the lower level
# directory has had its all rule performed.
#
##

cal/.all:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for cal =-=-=-=-='
	cd cal; ${MAKE} -f Makefile ${CAL_PASSDOWN} all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

help/.all:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for help =-=-=-=-='
	cd help; ${MAKE} -f Makefile ${HELP_PASSDOWN} all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

help/builtin: func.c help/builtin.top help/builtin.end help/funclist.sed
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking builtin rule for help =-=-=-=-='
	cd help; ${MAKE} -f Makefile ${HELP_PASSDOWN} builtin
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

cscript/.all: ${CSCRIPT_TARGETS}

${CSCRIPT_TARGETS}:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for cscript =-=-=-=-='
	cd cscript; ${MAKE} -f Makefile ${CSCRIPT_PASSDOWN} all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

sample/.all: ${SAMPLE_TARGETS}

custom/.all:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for custom =-=-=-=-='
	cd custom; ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

${SAMPLE_TARGETS}: libcalc.a
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for cscript =-=-=-=-='
	cd sample; ${MAKE} -f Makefile ${SAMPLE_PASSDOWN} all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

# This is a special rule that first tries to determine of a lower level
# make is needed, and it so a make will be performed.  Because it is
# triggered as the first dependent of the all rule, it will ensure
# that custom/libcustcalc.a is ready.
#
custom/libcustcalc:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for custom =-=-=-=-='
	-${Q}rm -f .libcustcalc_error
	-${Q}NEED="`cd custom; ${MAKE} -n -f Makefile all`"; \
	 if [ ! -z "$$NEED" ]; then \
	    echo "	cd custom; ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} all";\
	    cd custom; ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} all; \
	    status="$$?"; \
	    if [ "$$status" -ne 0 ]; then \
		echo "$$status" > ../.libcustcalc_error; \
	    fi; \
	fi
	${Q}if [ -f .libcustcalc_error ]; then \
	    echo "custom make failed, code: `cat .libcustcalc_error`" 1>&2; \
	    exit "`cat .libcustcalc_error`"; \
	else \
	    true ; \
	fi
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

# This is the real custom/libcustcalc.a rule.
#
custom/libcustcalc.a:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking all rule for custom =-=-=-=-='
	${Q}cd custom; ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} all
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

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
	${V} echo '=-=-=-=-= Invoking depend rule for custom =-=-=-=-='
	-${Q}(cd custom; ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} depend)
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking depend rule for sample =-=-=-=-='
	-${Q}(cd sample; ${MAKE} -f Makefile ${SAMPLE_PASSDOWN} depend)
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${Q}echo forming skel
	-${Q}rm -rf skel
	${Q}${MKDIR} skel
	-${Q}for i in ${C_SRC} ${BUILD_C_SRC}; do \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" | \
		    ${GREP} -v '\.\./getopt/getopt\.h' > "skel/$$i"; \
	done
	${Q}${MKDIR} skel/custom
	-${Q}for i in ${H_SRC} ${BUILD_H_SRC} custom.h /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		tag="`echo $$i | ${SED} 's/[\.+,:]/_/g'`"; \
		echo "#if !defined($$tag)" > "skel/$$i"; \
		echo "#define $$tag" >> "skel/$$i"; \
		${SED} -n '/^#[	 ]*include[	 ]*"/p' "$$i" | \
		    LANG=C ${SORT} -u >> "skel/$$i"; \
		echo '#endif /* '"$$tag"' */' >> "skel/$$i"; \
	    fi; \
	done
	-${Q}rm -f skel/makedep.out
	${Q}echo skel formed
	${Q}echo forming dependency list
	${Q}echo "# DO NOT DELETE THIS LINE -- make depend depends on it." > \
	    skel/makedep.out
	${Q}cd skel; \
	    ${MAKEDEPEND} -w 1 -f makedep.out ${C_SRC} ${BUILD_C_SRC}
	-${Q}for i in ${C_SRC} ${BUILD_C_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo "$$i" | ${SED} 's/^\(.*\)\.c/\1.o: \1.c/'; \
	    fi; \
	done >> skel/makedep.out
	${Q}echo dependency list formed
	${Q}echo forming new Makefile
	-${Q}rm -f Makefile.bak
	${Q}mv Makefile Makefile.bak
	${Q}${SED} -n '1,/^# DO NOT DELETE THIS LINE/p' Makefile.bak > Makefile
	${Q}echo "" >> Makefile
	${Q}${SED} -n '3,$$p' skel/makedep.out | LANG=C ${SORT} -u >> Makefile
	-${Q}rm -rf skel
	-${Q}if ${CMP} -s Makefile.bak Makefile; then \
		echo 'Makefile was already up to date'; \
		mv -f Makefile.bak Makefile; \
	else \
		echo 'new Makefile formed'; \
	fi

# generate the list of h files for lower level depend use
#
h_list:
	-${Q}for i in ${H_SRC} ${BUILD_H_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i; \
	    fi; \
	done

# print the calc version
#
ver_calc${EXT}: version.c have_unused.h
	-rm -f $@
	${LCC} ${ICFLAGS} -DCALC_VER ${ILDFLAGS} version.c -o $@

##
#
# File distribution list generation.  You can ignore this section.
#
# We will form the names of source files as if they were in a
# sub-directory called calc.
#
##

distlist: ${DISTLIST}
	${Q}(for i in ${DISTLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i; \
	    fi; \
	done; \
	for i in ${BUILD_H_SRC} ${BUILD_C_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo win32/$$i; \
	    fi; \
	done; \
	(cd help; ${MAKE} ${HELP_PASSDOWN} $@); \
	(cd cal; ${MAKE} ${CAL_PASSDOWN} $@); \
	(cd custom; ${MAKE} ${CUSTOM_PASSDOWN} $@); \
	(cd cscript; ${MAKE} ${CSCRIPT_PASSDOWN} $@); \
	(cd sample; ${MAKE} ${SAMPLE_PASSDOWN} $@)) | LANG=C ${SORT}

distdir:
	${Q}(echo .; \
	echo win32; \
	(cd help; ${MAKE} ${HELP_PASSDOWN} $@); \
	(cd cal; ${MAKE} ${CAL_PASSDOWN} $@); \
	(cd custom; ${MAKE} ${CUSTOM_PASSDOWN} $@); \
	(cd cscript; ${MAKE} ${CSCRIPT_PASSDOWN} $@); \
	(cd sample; ${MAKE} ${SAMPLE_PASSDOWN} $@)) | LANG=C ${SORT}

calcliblist:
	${Q}(for i in ${CALCLIBLIST} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo $$i; \
	    fi; \
	done; \
	(cd help; ${MAKE} ${HELP_PASSDOWN} $@); \
	(cd cal; ${MAKE} ${CAL_PASSDOWN} $@); \
	(cd custom; ${MAKE} ${CUSTOM_PASSDOWN} $@); \
	(cd cscript; ${MAKE} ${CSCRIPT_PASSDOWN} $@); \
	(cd sample; ${MAKE} ${SAMPLE_PASSDOWN} $@)) | LANG=C ${SORT}

calcliblistfmt:
	${Q}${MAKE} calcliblist | ${FMT} -64 | ${SED} -e 's/^/	/'

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

check: all ./cal/regress.cal
	${CALC_ENV} ./calc${EXT} -d -q read regress

chk: ./cal/regress.cal
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${CALC_ENV} ./calc${EXT} -d -q read regress 2>&1 | ${AWK} -f check.awk
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

##
#
# debug
#
# make env:
#	* print major Makefile variables
#
# make mkdebug:
#	* print major Makefile variables
#	* build anything not yet built
#
# make debug:
#	* remove everything that was previously built
#	* print major Makefile variables
#	* make everything
#	* run the regression tests
##

env:
	@echo '=-=-=-=-= dumping major make variables =-=-=-=-='
	@echo 'MAKEFILE_REV=${MAKEFILE_REV}'; echo ''
	@echo 'TERMCONTROL=${TERMCONTROL}'; echo ''
	@echo 'HAVE_VSPRINTF=${HAVE_VSPRINTF}'; echo ''
	@echo 'BYTE_ORDER=${BYTE_ORDER}'; echo ''
	@echo 'LONG_BITS=${LONG_BITS}'; echo ''
	@echo 'HAVE_FPOS=${HAVE_FPOS}'; echo ''
	@echo 'HAVE_FPOS_POS=${HAVE_FPOS_POS}'; echo ''
	@echo 'HAVE_OFFSCL=${HAVE_OFFSCL}'; echo ''
	@echo 'HAVE_POSSCL=${HAVE_POSSCL}'; echo ''
	@echo 'HAVE_CONST=${HAVE_CONST}'; echo ''
	@echo 'HAVE_UID_T=${HAVE_UID_T}'; echo ''
	@echo 'HAVE_NEWSTR=${HAVE_NEWSTR}'; echo ''
	@echo 'HAVE_USTAT=${HAVE_USTAT}'; echo ''
	@echo 'HAVE_GETSID=${HAVE_GETSID}'; echo ''
	@echo 'HAVE_GETPGID=${HAVE_GETPGID}'; echo ''
	@echo 'HAVE_GETTIME=${HAVE_GETTIME}'; echo ''
	@echo 'HAVE_GETPRID=${HAVE_GETPRID}'; echo ''
	@echo 'HAVE_URANDOM=${HAVE_URANDOM}'; echo ''
	@echo 'ALIGN32=${ALIGN32}'; echo ''
	@echo 'BINDIR=${BINDIR}'; echo ''
	@echo 'CALC_SHAREDIR=${CALC_SHAREDIR}'; echo ''
	@echo 'LIBDIR=${LIBDIR}'; echo ''
	@echo 'HELPDIR=${HELPDIR}'; echo ''
	@echo 'CUSTOMCALDIR=${CUSTOMCALDIR}'; echo ''
	@echo 'CUSTOMINCDIR=${CUSTOMINCDIR}'; echo ''
	@echo 'CUSTOMHELPDIR=${CUSTOMHELPDIR}'; echo ''
	@echo 'SCRIPTDIR=${SCRIPTDIR}'; echo ''
	@echo 'MANDIR=${MANDIR}'; echo ''
	@echo 'CATDIR=${CATDIR}'; echo ''
	@echo 'MANEXT=${MANEXT}'; echo ''
	@echo 'CATEXT=${CATEXT}'; echo ''
	@echo 'NROFF=${NROFF}'; echo ''
	@echo 'NROFF_ARG=${NROFF_ARG}'; echo ''
	@echo 'MANMAKE=${MANMAKE}'; echo ''
	@echo 'CALCPATH=${CALCPATH}'; echo ''
	@echo 'CALCRC=${CALCRC}'; echo ''
	@echo 'CALCPAGER=${CALCPAGER}'; echo ''
	@echo 'DEBUG=${DEBUG}'; echo ''
	@echo 'NO_SHARED=${NO_SHARED}'; echo ''
	@echo 'LD_NO_SHARED=${LD_NO_SHARED}'; echo ''
	@echo 'RANLIB=${RANLIB}'; echo ''
	@echo 'MAKE_FILE=${MAKE_FILE}'; echo ''
	@echo 'PURIFY=${PURIFY}'; echo ''
	@echo 'LD_DEBUG=${LD_DEBUG}'; echo ''
	@echo 'CALC_ENV=${CALC_ENV}'; echo ''
	@echo 'ALLOW_CUSTOM=${ALLOW_CUSTOM}'; echo ''
	@echo 'CCOPT=${CCOPT}'; echo ''
	@echo 'CCWARN=${CCWARN}'; echo ''
	@echo 'CCMISC=${CCMISC}'; echo ''
	@echo 'CFLAGS=${CFLAGS}'; echo ''
	@echo 'ICFLAGS=${ICFLAGS}'; echo ''
	@echo 'LDFLAGS=${LDFLAGS}'; echo ''
	@echo 'ILDFLAGS=${ILDFLAGS}'; echo ''
	@echo 'LCC=${LCC}'; echo ''
	@echo 'CC=${CC}'; echo ''
	@echo 'SHELL=${SHELL}'; echo ''
	@echo 'MAKE=${MAKE}'; echo ''
	@echo 'AWK=${AWK}'; echo ''
	@echo 'SED=${SED}'; echo ''
	@echo 'GREP=${GREP}'; echo ''
	@echo 'LANG=${LANG}'; echo ''
	@echo 'T=$T'; echo ''
	@echo 'SORT=${SORT}'; echo ''
	@echo 'TEE=${TEE}'; echo ''
	@echo 'CTAGS=${CTAGS}'; echo ''
	@echo 'MAKEDEPEND=${MAKEDEPEND}'; echo ''
	@echo 'MKDIR_ARG=${MKDIR_ARG}'; echo ''
	@echo 'EXT=${EXT}'; echo ''
	@echo 'Q=${Q}'; echo ''
	@echo 'V=${V}'; echo ''
	@echo 'LIBSRC=${LIBSRC}'; echo ''
	@echo 'LIBOBJS=${LIBOBJS}'; echo ''
	@echo 'CALCSRC=${CALCSRC}'; echo ''
	@echo 'CALCOBJS=${CALCOBJS}'; echo ''
	@echo 'BUILD_H_SRC=${BUILD_H_SRC}'; echo ''
	@echo 'BUILD_C_SRC=${BUILD_C_SRC}'; echo ''
	@echo 'UTIL_C_SRC=${UTIL_C_SRC}'; echo ''
	@echo 'UTIL_MISC_SRC=${UTIL_MISC_SRC}'; echo ''
	@echo 'UTIL_OBJS=${UTIL_OBJS}'; echo ''
	@echo 'UTIL_TMP=${UTIL_TMP}'; echo ''
	@echo 'UTIL_PROGS=${UTIL_PROGS}'; echo ''
	@echo 'LIB_H_SRC=${LIB_H_SRC}'; echo ''
	@echo 'CUSTOM_PASSDOWN=${CUSTOM_PASSDOWN}'; echo ''
	@echo 'SAMPLE_PASSDOWN=${SAMPLE_PASSDOWN}'; echo ''
	@echo 'HELP_PASSDOWN=${HELP_PASSDOWN}'; echo ''
	@echo 'CAL_PASSDOWN=${CAL_PASSDOWN}'; echo ''
	@echo 'CSCRIPT_PASSDOWN=${CSCRIPT_PASSDOWN}'; echo ''
	@echo 'H_SRC=${H_SRC}'; echo ''
	@echo 'C_SRC=${C_SRC}'; echo ''
	@echo 'DISTLIST=${DISTLIST}'; echo ''
	@echo 'OBJS=${OBJS}'; echo ''
	@echo 'CALC_LIBS=${CALC_LIBS}'; echo ''
	@echo 'PROGS=${PROGS}'; echo ''
	@echo 'TARGETS=${TARGETS}'; echo ''
	@echo '=-=-=-=-= end of major make variable dump =-=-=-=-='

mkdebug: env version.c rpm.release
	@echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Determining the source version =-=-=-=-='
	@${MAKE} -f Makefile Q= V=@ ver_calc${EXT}
	-@./ver_calc${EXT}
	-@./ver_calc${EXT} -r rpm.release
	@echo '=-=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ all =-=-=-=-='
	@${MAKE} -f Makefile Q= V=@ all
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Determining the binary version =-=-=-=-='
	-@./calc${EXT} -e -q -v
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= end of $@ rule =-=-=-=-='

debug: env rpm.release
	@echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ clobber =-=-=-=-='
	@${MAKE} -f Makefile Q= V=@ clobber
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Determining the source version =-=-=-=-='
	@${MAKE} -f Makefile Q= V=@ ver_calc${EXT}
	-@./ver_calc${EXT}
	-@./ver_calc${EXT} -r rpm.release
	@echo '=-=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ all =-=-=-=-='
	@${MAKE} -f Makefile Q= V=@ all
	@echo '=-=-=-=-= Determining the binary version =-=-=-=-='
	-@./calc${EXT} -e -q -v
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= Invoking ${MAKE} -f Makefile Q= V=@ chk =-=-=-=-='
	@echo '=-=-=-=-= this may take a while =-=-=-=-='
	@${MAKE} -f Makefile Q= V=@ chk
	@echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	@echo '=-=-=-=-= end of $@ rule =-=-=-=-='

##
#
# make run
#	* only run calc interactively with the ${CALC_ENV} environment
#
# make cvd
#	* run the SGI WorkShop debugger on calc with the ${CALC_ENV} environment
#
# make dbx
#	* run the dbx debugger on calc with the ${CALC_ENV} environment
#
# make gdb
#	* run the gdb debugger on calc with the ${CALC_ENV} environment
#
##

run:
	${CALC_ENV} ./calc${EXT}

cvd:
	${CALC_ENV} cvd ./calc${EXT}

dbx:
	${CALC_ENV} dbx ./calc${EXT}

gdb:
	${CALC_ENV} gdb ./calc${EXT}


##
#
# rpm rules
#
##

rpm: clobber rpm.mk calc.spec.in
	rm -rf /var/tmp/redhat
	${MAKE} -f rpm.mk RHDIR=/var/tmp/redhat TMPDIR=/var/tmp/redhat


##
#
# Utility rules
#
##

# Form the installed file list
#
inst_files: ${MAKE_FILE} help/Makefile cal/Makefile custom/Makefile \
	   cscript/Makefile ver_calc${EXT}
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	${Q}rm -f inst_files
	${Q}echo ${BINDIR}/calc${EXT} > inst_files
	${Q}cd help; LANG=C \
	    ${MAKE} -f Makefile ${HELP_PASSDOWN} echo_inst_files | \
	    ${GREP} '__file__..' | ${SED} -e s'/.*__file__ //' >> ../inst_files
	${Q}cd cal; LANG=C \
	    ${MAKE} -f Makefile ${CAL_PASSDOWN} echo_inst_files | \
	    ${GREP} '__file__..' | ${SED} -e s'/.*__file__ //' >> ../inst_files
	${Q}cd custom; LANG=C \
	    ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} echo_inst_files | \
	    ${GREP} '__file__..' | ${SED} -e s'/.*__file__ //' >> ../inst_files
	${Q}cd cscript; LANG=C \
	    ${MAKE} -f Makefile ${CSCRIPT_PASSDOWN} echo_inst_files | \
	    ${GREP} '__file__..' | ${SED} -e s'/.*__file__ //' >> ../inst_files
	${Q}echo ${LIBDIR}/libcalc.a >> inst_files
	${Q}for i in ${LIB_H_SRC} ${BUILD_H_SRC} /dev/null; do \
	    if [ X"$$i" != X"/dev/null" ]; then \
		echo ${CALC_INCDIR}/$$i; \
	    fi; \
	done >> inst_files
	${Q}if [ ! -z "${MANDIR}" ]; then \
	    echo ${MANDIR}/calc.${MANEXT}; \
	fi >> inst_files
	${Q}LANG=C ${SORT} -u inst_files -o inst_files
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

# The olduninstall rule will remove calc files from the older, histroic
# locations under the /usr/local directory.  If you are using the
# new default values for ${BINDIR}, ${CALC_SHAREDIR}, ${INCDIR} and ${LIBDIR}
# then you can use this rule to clean out the older calc stuff under
# the /usr/local directory.
#
olduninstall:
	-rm -f inst_files
	${MAKE} -f Makefile \
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
	-${XARGS} rm -f < inst_files
	-rmdir /usr/local/lib/calc/help/custhelp
	-rmdir /usr/local/lib/calc/help
	-rmdir /usr/local/lib/calc/custom
	-rmdir /usr/local/lib/calc
	-rmdir /usr/local/include/calc
	-rmdir /usr/local/bin/cscript
	-rm -f inst_files

tags: ${CALCSRC} ${LIBSRC} ${H_SRC} ${BUILD_H_SRC} ${MAKE_FILE}
	-${CTAGS} ${CALCSRC} ${LIBSRC} ${H_SRC} ${BUILD_H_SRC} 2>&1 | \
	    ${GREP} -v 'Duplicate entry|Second entry ignored'

clean:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	-rm -f ${LIBOBJS}
	-rm -f ${CALCOBJS}
	-rm -f ${UTIL_OBJS}
	-rm -f ${UTIL_TMP}
	-rm -f ${UTIL_PROGS}
	-rm -f .libcustcalc_error
	-rm -f calc.spec.sed
	${Q}echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	-cd help; ${MAKE} -f Makefile ${HELP_PASSDOWN} clean
	${Q}echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${Q}echo '=-=-=-=-= Invoking $@ rule for cal =-=-=-=-='
	-cd cal; ${MAKE} -f Makefile ${CAL_PASSDOWN} clean
	${Q}echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for custom =-=-=-=-='
	cd custom; ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} clean
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for sample =-=-=-=-='
	cd sample; ${MAKE} -f Makefile ${SAMPLE_PASSDOWN} clean
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cscript =-=-=-=-='
	cd cscript; ${MAKE} -f Makefile ${CSCRIPT_PASSDOWN} clean
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${Q}echo remove files that are obsolete
	-rm -rf lib
	-rm -f endian.h stdarg.h libcalcerr.a cal/obj help/obj
	-rm -f have_vs.c std_arg.h try_stdarg.c fnvhash.c
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

clobber:
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	-rm -f ${LIBOBJS}
	-rm -f ${CALCOBJS}
	-rm -f ${UTIL_OBJS}
	-rm -f ${UTIL_TMP}
	-rm -f ${UTIL_PROGS}
	-rm -f tags .hsrc hsrc
	-rm -f ${BUILD_H_SRC}
	-rm -f ${BUILD_C_SRC}
	-rm -f calc${EXT} *_pure_*.[oa]
	-rm -f libcalc.a *.pure_hardlink
	-rm -f calc.1 *.pure_linkinfo
	-rm -f *.u
	-rm -f calc.pixie calc.rf calc.Counts calc.cord
	-rm -rf gen_h skel Makefile.bak tmp.patch
	-rm -f calc.spec inst_files rpm.mk.patch tmp
	${V} echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	-cd help; ${MAKE} -f Makefile ${HELP_PASSDOWN} clobber
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cal =-=-=-=-='
	-cd cal; ${MAKE} -f Makefile ${CAL_PASSDOWN} clobber
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for custom =-=-=-=-='
	cd custom; ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} clobber
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for sample =-=-=-=-='
	cd sample; ${MAKE} -f Makefile ${SAMPLE_PASSDOWN} clobber
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cscript =-=-=-=-='
	cd cscript; ${MAKE} -f Makefile ${CSCRIPT_PASSDOWN} clobber
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo remove files that are obsolete
	-rm -rf lib
	-rm -f endian.h stdarg.h libcalcerr.a cal/obj help/obj
	-rm -f have_vs.c std_arg.h try_stdarg.c fnvhash.c calc.spec
	-rm -rf win32
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

install: calc libcalc.a ${LIB_H_SRC} ${BUILD_H_SRC} calc.1
	${V} echo '=-=-=-=-= start of $@ rule =-=-=-=-='
	-${Q}if [ ! -z "$T" ]; then \
	    if [ ! -d $T ]; then \
		echo ${MKDIR} ${MKDIR_ARG} $T; \
		${MKDIR} ${MKDIR_ARG} $T; \
		echo ${CHMOD} 0755 $T; \
		${CHMOD} 0755 $T; \
	    fi; \
	fi
	-${Q}if [ ! -d $T${BINDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${BINDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${BINDIR}; \
	    echo ${CHMOD} 0755 $T${BINDIR}; \
	    ${CHMOD} 0755 $T${BINDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${INCDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${INCDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${INCDIR}; \
	    echo ${CHMOD} 0755 $T${INCDIR}; \
	    ${CHMOD} 0755 $T${INCDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${LIBDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${LIBDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${LIBDIR}; \
	    echo ${CHMOD} 0755 $T${LIBDIR}; \
	    ${CHMOD} 0755 $T${LIBDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${CALC_SHAREDIR} ]; then \
	    ${MKDIR} ${MKDIR_ARG} $T${CALC_SHAREDIR}; \
	    echo ${MKDIR} ${MKDIR_ARG} $T${CALC_SHAREDIR}; \
	    echo ${CHMOD} 0755 $T${CALC_SHAREDIR}; \
	    ${CHMOD} 0755 $T${CALC_SHAREDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${HELPDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${HELPDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${HELPDIR}; \
	    echo ${CHMOD} 0755 $T${HELPDIR}; \
	    ${CHMOD} 0755 $T${HELPDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${CALC_INCDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${CALC_INCDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${CALC_INCDIR}; \
	    echo ${CHMOD} 0755 $T${CALC_INCDIR}; \
	    ${CHMOD} 0755 $T${CALC_INCDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${CUSTOMCALDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${CUSTOMCALDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${CUSTOMCALDIR}; \
	    echo ${CHMOD} 0755 $T${CUSTOMCALDIR}; \
	    ${CHMOD} 0755 $T${CUSTOMCALDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${CUSTOMHELPDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${CUSTOMHELPDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${CUSTOMHELPDIR}; \
	    echo ${CHMOD} 0755 $T${CUSTOMHELPDIR}; \
	    ${CHMOD} 0755 $T${CUSTOMHELPDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${CUSTOMINCDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${CUSTOMINCDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${CUSTOMINCDIR}; \
	    echo ${CHMOD} 0755 $T${CUSTOMINCDIR}; \
	    ${CHMOD} 0755 $T${CUSTOMINCDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -d $T${SCRIPTDIR} ]; then \
	    echo ${MKDIR} ${MKDIR_ARG} $T${SCRIPTDIR}; \
	    ${MKDIR} ${MKDIR_ARG} $T${SCRIPTDIR}; \
	    echo ${CHMOD} 0755 $T${SCRIPTDIR}; \
	    ${CHMOD} 0755 $T${SCRIPTDIR}; \
	else \
	    true; \
	fi
	-${Q}if [ ! -z "${MANDIR}" ]; then \
	    if [ ! -d $T${MANDIR} ]; then \
		echo ${MKDIR} ${MKDIR_ARG} $T${MANDIR}; \
		${MKDIR} ${MKDIR_ARG} $T${MANDIR}; \
		echo ${CHMOD} 0755 $T${MANDIR}; \
		${CHMOD} 0755 $T${MANDIR}; \
	    else \
		true; \
	    fi; \
	else \
	    true; \
	fi
	-${Q}if [ ! -z "${CATDIR}" ]; then \
	    if [ ! -d $T${CATDIR} ]; then \
		echo ${MKDIR} ${MKDIR_ARG} $T${CATDIR}; \
		${MKDIR} ${MKDIR_ARG} $T${CATDIR}; \
		echo ${CHMOD} 0755 $T${CATDIR}; \
		${CHMOD} 0755 $T${CATDIR}; \
	    else \
		true; \
	    fi; \
	else \
	    true; \
	fi
	-${Q}if ${CMP} -s calc${EXT} $T${BINDIR}/calc${EXT}; then \
	    true; \
	else \
	    rm -f $T${BINDIR}/calc.new${EXT}; \
	    cp -f calc${EXT} $T${BINDIR}/calc.new${EXT}; \
	    ${CHMOD} 0555 $T${BINDIR}/calc.new${EXT}; \
	    mv -f $T${BINDIR}/calc.new${EXT} $T${BINDIR}/calc${EXT}; \
	    echo "installed $T${BINDIR}/calc${EXT}"; \
	fi
	${V} echo '=-=-=-=-= Invoking $@ rule for help =-=-=-=-='
	${Q}cd help; ${MAKE} -f Makefile ${HELP_PASSDOWN} install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cal =-=-=-=-='
	${Q}cd cal; ${MAKE} -f Makefile ${CAL_PASSDOWN} install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for custom =-=-=-=-='
	${Q}cd custom; ${MAKE} -f Makefile ${CUSTOM_PASSDOWN} install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for sample =-=-=-=-='
	${Q}cd sample; ${MAKE} -f Makefile ${SAMPLE_PASSDOWN} install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	${V} echo '=-=-=-=-= Invoking $@ rule for cscript =-=-=-=-='
	${Q}cd cscript; ${MAKE} -f Makefile ${CSCRIPT_PASSDOWN} install
	${V} echo '=-=-=-=-= Back to the main Makefile for $@ rule =-=-=-=-='
	-${Q}if ${CMP} -s libcalc.a $T${LIBDIR}/libcalc.a; then \
		true; \
	else \
		rm -f $T${LIBDIR}/libcalc.a.new; \
		cp -f libcalc.a $T${LIBDIR}/libcalc.a.new; \
		mv -f $T${LIBDIR}/libcalc.a.new $T${LIBDIR}/libcalc.a; \
		${RANLIB} $T${LIBDIR}/libcalc.a; \
		${CHMOD} 0444 $T${LIBDIR}/libcalc.a; \
		echo "installed $T${LIBDIR}/libcalc.a"; \
	fi
	-${Q}for i in ${LIB_H_SRC} ${BUILD_H_SRC} /dev/null; do \
	    if [ "$$i" = "/dev/null" ]; then \
		continue; \
	    fi; \
	    rm -f tmp; \
	    ${SED} -e 's/^\(#[ 	]*include[ 	][ 	]*\)"/\1"calc\//' $$i > tmp; \
	    if ${CMP} -s tmp $T${CALC_INCDIR}/$$i; then \
		true; \
	    else \
		rm -f $T${CALC_INCDIR}/$$i.new; \
		cp -f tmp $T${CALC_INCDIR}/$$i.new; \
		${CHMOD} 0444 $T${CALC_INCDIR}/$$i.new; \
		mv -f $T${CALC_INCDIR}/$$i.new $T${CALC_INCDIR}/$$i; \
		echo "installed $T${CALC_INCDIR}/$$i"; \
	    fi; \
	done
	${Q}rm -f tmp
	-${Q}if [ -z "${MANDIR}" ]; then \
	    true; \
	else \
	    if ${CMP} -s calc.1 $T${MANDIR}/calc.${MANEXT}; then \
		true; \
	    else \
		rm -f $T${MANDIR}/calc.${MANEXT}.new; \
		cp -f calc.1 $T${MANDIR}/calc.${MANEXT}.new; \
		${CHMOD} 0444 $T${MANDIR}/calc.${MANEXT}.new; \
		mv -f $T${MANDIR}/calc.${MANEXT}.new \
		      $T${MANDIR}/calc.${MANEXT}; \
		echo "installed $T${MANDIR}/calc.${MANEXT}"; \
	    fi; \
	fi
	-${Q}if [ -z "${CATDIR}" ]; then \
	    true; \
	else \
	    if ${CMP} -s calc.1 $T${MANDIR}/calc.${MANEXT}; then \
		true; \
	    else \
		if [ -z "${NROFF}" ]; then \
		    echo "${MANMAKE} calc.1 $T${CATDIR}"; \
		    ${MANMAKE} calc.1 $T${CATDIR}; \
		else \
		    rm -f $T${CATDIR}/calc.${CATEXT}.new; \
		    ${NROFF} ${NROFF_ARG} calc.1 > \
			     $T${CATDIR}/calc.${CATEXT}.new; \
		    ${CHMOD} ${MANMODE} $T${MANDIR}/calc.${CATEXT}.new; \
		    mv -f $T${CATDIR}/calc.${CATEXT}.new \
			  $T${CATDIR}/calc.${CATEXT}; \
		    echo "installed $T${CATDIR}/calc.${CATEXT}"; \
		fi; \
	    fi; \
	fi
	${V} echo '=-=-=-=-= end of $@ rule =-=-=-=-='

# splint - A tool for statically checking C programs
#
splint: #hsrc
	${SPLINT} ${SPLINT_OPTS} -DCALC_SRC -I. \
	    ${CALCSRC} ${LIBSRC} ${BUILD_C_SRC} ${UTIL_C_SRC}

##
#
# make depend stuff
#
##

# DO NOT DELETE THIS LINE -- make depend depends on it.

addop.o: addop.c
addop.o: alloc.h
addop.o: block.h
addop.o: byteswap.h
addop.o: calc.h
addop.o: calcerr.h
addop.o: cmath.h
addop.o: config.h
addop.o: endian_calc.h
addop.o: func.h
addop.o: hash.h
addop.o: have_const.h
addop.o: have_malloc.h
addop.o: have_memmv.h
addop.o: have_newstr.h
addop.o: have_stdlib.h
addop.o: have_string.h
addop.o: label.h
addop.o: longbits.h
addop.o: md5.h
addop.o: nametype.h
addop.o: opcodes.h
addop.o: qmath.h
addop.o: shs.h
addop.o: shs1.h
addop.o: string.h
addop.o: symbol.h
addop.o: token.h
addop.o: value.h
addop.o: win32dll.h
addop.o: zmath.h
align32.o: align32.c
align32.o: have_unistd.h
align32.o: longbits.h
assocfunc.o: alloc.h
assocfunc.o: assocfunc.c
assocfunc.o: block.h
assocfunc.o: byteswap.h
assocfunc.o: calcerr.h
assocfunc.o: cmath.h
assocfunc.o: config.h
assocfunc.o: endian_calc.h
assocfunc.o: hash.h
assocfunc.o: have_malloc.h
assocfunc.o: have_memmv.h
assocfunc.o: have_newstr.h
assocfunc.o: have_stdlib.h
assocfunc.o: have_string.h
assocfunc.o: longbits.h
assocfunc.o: md5.h
assocfunc.o: nametype.h
assocfunc.o: qmath.h
assocfunc.o: shs.h
assocfunc.o: shs1.h
assocfunc.o: string.h
assocfunc.o: value.h
assocfunc.o: win32dll.h
assocfunc.o: zmath.h
blkcpy.o: alloc.h
blkcpy.o: blkcpy.c
blkcpy.o: blkcpy.h
blkcpy.o: block.h
blkcpy.o: byteswap.h
blkcpy.o: calc.h
blkcpy.o: calcerr.h
blkcpy.o: cmath.h
blkcpy.o: config.h
blkcpy.o: endian_calc.h
blkcpy.o: file.h
blkcpy.o: hash.h
blkcpy.o: have_const.h
blkcpy.o: have_fpos.h
blkcpy.o: have_malloc.h
blkcpy.o: have_memmv.h
blkcpy.o: have_newstr.h
blkcpy.o: have_stdlib.h
blkcpy.o: have_string.h
blkcpy.o: longbits.h
blkcpy.o: md5.h
blkcpy.o: nametype.h
blkcpy.o: qmath.h
blkcpy.o: shs.h
blkcpy.o: shs1.h
blkcpy.o: string.h
blkcpy.o: value.h
blkcpy.o: win32dll.h
blkcpy.o: zmath.h
block.o: alloc.h
block.o: block.c
block.o: block.h
block.o: byteswap.h
block.o: calcerr.h
block.o: cmath.h
block.o: config.h
block.o: endian_calc.h
block.o: hash.h
block.o: have_malloc.h
block.o: have_memmv.h
block.o: have_newstr.h
block.o: have_stdlib.h
block.o: have_string.h
block.o: longbits.h
block.o: md5.h
block.o: nametype.h
block.o: qmath.h
block.o: shs.h
block.o: shs1.h
block.o: string.h
block.o: value.h
block.o: win32dll.h
block.o: zmath.h
byteswap.o: alloc.h
byteswap.o: byteswap.c
byteswap.o: byteswap.h
byteswap.o: cmath.h
byteswap.o: endian_calc.h
byteswap.o: have_malloc.h
byteswap.o: have_memmv.h
byteswap.o: have_newstr.h
byteswap.o: have_stdlib.h
byteswap.o: have_string.h
byteswap.o: longbits.h
byteswap.o: qmath.h
byteswap.o: win32dll.h
byteswap.o: zmath.h
calc.o: alloc.h
calc.o: args.h
calc.o: block.h
calc.o: byteswap.h
calc.o: calc.c
calc.o: calc.h
calc.o: calcerr.h
calc.o: cmath.h
calc.o: conf.h
calc.o: config.h
calc.o: custom.h
calc.o: endian_calc.h
calc.o: func.h
calc.o: hash.h
calc.o: have_const.h
calc.o: have_malloc.h
calc.o: have_memmv.h
calc.o: have_newstr.h
calc.o: have_stdlib.h
calc.o: have_strdup.h
calc.o: have_string.h
calc.o: have_uid_t.h
calc.o: have_unistd.h
calc.o: have_unused.h
calc.o: hist.h
calc.o: label.h
calc.o: longbits.h
calc.o: math_error.h
calc.o: md5.h
calc.o: nametype.h
calc.o: opcodes.h
calc.o: qmath.h
calc.o: shs.h
calc.o: shs1.h
calc.o: string.h
calc.o: symbol.h
calc.o: token.h
calc.o: value.h
calc.o: win32dll.h
calc.o: zmath.h
calcerr.o: calcerr.c
calcerr.o: calcerr.h
calcerr.o: have_const.h
codegen.o: alloc.h
codegen.o: block.h
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
codegen.o: have_const.h
codegen.o: have_malloc.h
codegen.o: have_memmv.h
codegen.o: have_newstr.h
codegen.o: have_stdlib.h
codegen.o: have_string.h
codegen.o: have_unistd.h
codegen.o: label.h
codegen.o: longbits.h
codegen.o: md5.h
codegen.o: nametype.h
codegen.o: opcodes.h
codegen.o: qmath.h
codegen.o: shs.h
codegen.o: shs1.h
codegen.o: string.h
codegen.o: symbol.h
codegen.o: token.h
codegen.o: value.h
codegen.o: win32dll.h
codegen.o: zmath.h
comfunc.o: alloc.h
comfunc.o: byteswap.h
comfunc.o: cmath.h
comfunc.o: comfunc.c
comfunc.o: config.h
comfunc.o: endian_calc.h
comfunc.o: have_malloc.h
comfunc.o: have_memmv.h
comfunc.o: have_newstr.h
comfunc.o: have_stdlib.h
comfunc.o: have_string.h
comfunc.o: longbits.h
comfunc.o: nametype.h
comfunc.o: qmath.h
comfunc.o: win32dll.h
comfunc.o: zmath.h
commath.o: alloc.h
commath.o: byteswap.h
commath.o: cmath.h
commath.o: commath.c
commath.o: endian_calc.h
commath.o: have_malloc.h
commath.o: have_memmv.h
commath.o: have_newstr.h
commath.o: have_stdlib.h
commath.o: have_string.h
commath.o: longbits.h
commath.o: qmath.h
commath.o: win32dll.h
commath.o: zmath.h
config.o: alloc.h
config.o: block.h
config.o: byteswap.h
config.o: calc.h
config.o: calcerr.h
config.o: cmath.h
config.o: config.c
config.o: config.h
config.o: custom.h
config.o: endian_calc.h
config.o: hash.h
config.o: have_const.h
config.o: have_malloc.h
config.o: have_memmv.h
config.o: have_newstr.h
config.o: have_stdlib.h
config.o: have_strdup.h
config.o: have_string.h
config.o: longbits.h
config.o: md5.h
config.o: nametype.h
config.o: qmath.h
config.o: shs.h
config.o: shs1.h
config.o: string.h
config.o: token.h
config.o: value.h
config.o: win32dll.h
config.o: zmath.h
config.o: zrand.h
const.o: alloc.h
const.o: block.h
const.o: byteswap.h
const.o: calc.h
const.o: calcerr.h
const.o: cmath.h
const.o: config.h
const.o: const.c
const.o: endian_calc.h
const.o: hash.h
const.o: have_const.h
const.o: have_malloc.h
const.o: have_memmv.h
const.o: have_newstr.h
const.o: have_stdlib.h
const.o: have_string.h
const.o: longbits.h
const.o: md5.h
const.o: nametype.h
const.o: qmath.h
const.o: shs.h
const.o: shs1.h
const.o: string.h
const.o: value.h
const.o: win32dll.h
const.o: zmath.h
custom.o: alloc.h
custom.o: block.h
custom.o: byteswap.h
custom.o: calc.h
custom.o: calcerr.h
custom.o: cmath.h
custom.o: config.h
custom.o: custom.c
custom.o: custom.h
custom.o: endian_calc.h
custom.o: hash.h
custom.o: have_const.h
custom.o: have_malloc.h
custom.o: have_memmv.h
custom.o: have_newstr.h
custom.o: have_stdlib.h
custom.o: have_string.h
custom.o: longbits.h
custom.o: md5.h
custom.o: nametype.h
custom.o: qmath.h
custom.o: shs.h
custom.o: shs1.h
custom.o: string.h
custom.o: value.h
custom.o: win32dll.h
custom.o: zmath.h
endian.o: endian.c
endian.o: have_stdlib.h
endian.o: have_unistd.h
file.o: alloc.h
file.o: block.h
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
file.o: have_const.h
file.o: have_fpos.h
file.o: have_fpos_pos.h
file.o: have_malloc.h
file.o: have_memmv.h
file.o: have_newstr.h
file.o: have_stdlib.h
file.o: have_string.h
file.o: have_unistd.h
file.o: longbits.h
file.o: md5.h
file.o: nametype.h
file.o: qmath.h
file.o: shs.h
file.o: shs1.h
file.o: string.h
file.o: value.h
file.o: win32dll.h
file.o: zmath.h
fposval.o: endian_calc.h
fposval.o: fposval.c
fposval.o: have_fpos.h
fposval.o: have_fpos_pos.h
fposval.o: have_offscl.h
fposval.o: have_posscl.h
func.o: alloc.h
func.o: block.h
func.o: byteswap.h
func.o: calc.h
func.o: calcerr.h
func.o: cmath.h
func.o: config.h
func.o: custom.h
func.o: endian_calc.h
func.o: file.h
func.o: func.c
func.o: func.h
func.o: hash.h
func.o: have_const.h
func.o: have_fpos.h
func.o: have_malloc.h
func.o: have_memmv.h
func.o: have_newstr.h
func.o: have_stdlib.h
func.o: have_strdup.h
func.o: have_string.h
func.o: have_times.h
func.o: have_unistd.h
func.o: have_unused.h
func.o: label.h
func.o: longbits.h
func.o: md5.h
func.o: nametype.h
func.o: opcodes.h
func.o: prime.h
func.o: qmath.h
func.o: shs.h
func.o: shs1.h
func.o: string.h
func.o: symbol.h
func.o: token.h
func.o: value.h
func.o: win32dll.h
func.o: zmath.h
func.o: zrand.h
func.o: zrandom.h
hash.o: alloc.h
hash.o: block.h
hash.o: byteswap.h
hash.o: calc.h
hash.o: calcerr.h
hash.o: cmath.h
hash.o: config.h
hash.o: endian_calc.h
hash.o: hash.c
hash.o: hash.h
hash.o: have_const.h
hash.o: have_malloc.h
hash.o: have_memmv.h
hash.o: have_newstr.h
hash.o: have_stdlib.h
hash.o: have_string.h
hash.o: longbits.h
hash.o: md5.h
hash.o: nametype.h
hash.o: qmath.h
hash.o: shs.h
hash.o: shs1.h
hash.o: string.h
hash.o: value.h
hash.o: win32dll.h
hash.o: zmath.h
hash.o: zrand.h
hash.o: zrandom.h
have_const.o: have_const.c
have_fpos.o: have_fpos.c
have_fpos_pos.o: have_fpos.h
have_fpos_pos.o: have_fpos_pos.c
have_fpos_pos.o: have_posscl.h
have_getpgid.o: have_getpgid.c
have_getprid.o: have_getprid.c
have_getsid.o: have_getsid.c
have_gettime.o: have_gettime.c
have_memmv.o: have_memmv.c
have_newstr.o: have_newstr.c
have_offscl.o: have_offscl.c
have_posscl.o: have_fpos.h
have_posscl.o: have_posscl.c
have_rusage.o: have_rusage.c
have_stdvs.o: have_stdvs.c
have_stdvs.o: have_string.h
have_stdvs.o: have_unistd.h
have_strdup.o: have_strdup.c
have_uid_t.o: have_uid_t.c
have_uid_t.o: have_unistd.h
have_unused.o: have_unused.c
have_ustat.o: have_ustat.c
have_varvs.o: have_string.h
have_varvs.o: have_unistd.h
have_varvs.o: have_varvs.c
help.o: alloc.h
help.o: block.h
help.o: byteswap.h
help.o: calc.h
help.o: calcerr.h
help.o: cmath.h
help.o: conf.h
help.o: config.h
help.o: endian_calc.h
help.o: hash.h
help.o: have_const.h
help.o: have_malloc.h
help.o: have_memmv.h
help.o: have_newstr.h
help.o: have_stdlib.h
help.o: have_string.h
help.o: have_unistd.h
help.o: help.c
help.o: longbits.h
help.o: md5.h
help.o: nametype.h
help.o: qmath.h
help.o: shs.h
help.o: shs1.h
help.o: string.h
help.o: value.h
help.o: win32dll.h
help.o: zmath.h
hist.o: alloc.h
hist.o: block.h
hist.o: byteswap.h
hist.o: calc.h
hist.o: calcerr.h
hist.o: cmath.h
hist.o: config.h
hist.o: endian_calc.h
hist.o: hash.h
hist.o: have_const.h
hist.o: have_malloc.h
hist.o: have_memmv.h
hist.o: have_newstr.h
hist.o: have_stdlib.h
hist.o: have_strdup.h
hist.o: have_string.h
hist.o: have_unistd.h
hist.o: have_unused.h
hist.o: hist.c
hist.o: hist.h
hist.o: longbits.h
hist.o: md5.h
hist.o: nametype.h
hist.o: qmath.h
hist.o: shs.h
hist.o: shs1.h
hist.o: string.h
hist.o: value.h
hist.o: win32dll.h
hist.o: zmath.h
input.o: alloc.h
input.o: block.h
input.o: byteswap.h
input.o: calc.h
input.o: calcerr.h
input.o: cmath.h
input.o: conf.h
input.o: config.h
input.o: endian_calc.h
input.o: hash.h
input.o: have_const.h
input.o: have_malloc.h
input.o: have_memmv.h
input.o: have_newstr.h
input.o: have_stdlib.h
input.o: have_string.h
input.o: have_unistd.h
input.o: hist.h
input.o: input.c
input.o: longbits.h
input.o: md5.h
input.o: nametype.h
input.o: qmath.h
input.o: shs.h
input.o: shs1.h
input.o: string.h
input.o: value.h
input.o: win32dll.h
input.o: zmath.h
jump.o: have_const.h
jump.o: jump.c
jump.o: jump.h
label.o: alloc.h
label.o: block.h
label.o: byteswap.h
label.o: calc.h
label.o: calcerr.h
label.o: cmath.h
label.o: config.h
label.o: endian_calc.h
label.o: func.h
label.o: hash.h
label.o: have_const.h
label.o: have_malloc.h
label.o: have_memmv.h
label.o: have_newstr.h
label.o: have_stdlib.h
label.o: have_string.h
label.o: label.c
label.o: label.h
label.o: longbits.h
label.o: md5.h
label.o: nametype.h
label.o: opcodes.h
label.o: qmath.h
label.o: shs.h
label.o: shs1.h
label.o: string.h
label.o: token.h
label.o: value.h
label.o: win32dll.h
label.o: zmath.h
lib_calc.o: alloc.h
lib_calc.o: block.h
lib_calc.o: byteswap.h
lib_calc.o: calc.h
lib_calc.o: calcerr.h
lib_calc.o: cmath.h
lib_calc.o: conf.h
lib_calc.o: config.h
lib_calc.o: endian_calc.h
lib_calc.o: func.h
lib_calc.o: hash.h
lib_calc.o: have_const.h
lib_calc.o: have_malloc.h
lib_calc.o: have_memmv.h
lib_calc.o: have_newstr.h
lib_calc.o: have_stdlib.h
lib_calc.o: have_strdup.h
lib_calc.o: have_string.h
lib_calc.o: have_unistd.h
lib_calc.o: label.h
lib_calc.o: lib_calc.c
lib_calc.o: longbits.h
lib_calc.o: md5.h
lib_calc.o: nametype.h
lib_calc.o: qmath.h
lib_calc.o: shs.h
lib_calc.o: shs1.h
lib_calc.o: string.h
lib_calc.o: symbol.h
lib_calc.o: terminal.h
lib_calc.o: token.h
lib_calc.o: value.h
lib_calc.o: win32dll.h
lib_calc.o: zmath.h
lib_calc.o: zrandom.h
lib_util.o: alloc.h
lib_util.o: byteswap.h
lib_util.o: endian_calc.h
lib_util.o: have_malloc.h
lib_util.o: have_memmv.h
lib_util.o: have_newstr.h
lib_util.o: have_stdlib.h
lib_util.o: have_string.h
lib_util.o: lib_util.c
lib_util.o: lib_util.h
lib_util.o: longbits.h
lib_util.o: win32dll.h
lib_util.o: zmath.h
listfunc.o: alloc.h
listfunc.o: block.h
listfunc.o: byteswap.h
listfunc.o: calcerr.h
listfunc.o: cmath.h
listfunc.o: config.h
listfunc.o: endian_calc.h
listfunc.o: hash.h
listfunc.o: have_const.h
listfunc.o: have_malloc.h
listfunc.o: have_memmv.h
listfunc.o: have_newstr.h
listfunc.o: have_stdlib.h
listfunc.o: have_string.h
listfunc.o: listfunc.c
listfunc.o: longbits.h
listfunc.o: md5.h
listfunc.o: nametype.h
listfunc.o: qmath.h
listfunc.o: shs.h
listfunc.o: shs1.h
listfunc.o: string.h
listfunc.o: value.h
listfunc.o: win32dll.h
listfunc.o: zmath.h
listfunc.o: zrand.h
longbits.o: have_stdlib.h
longbits.o: have_unistd.h
longbits.o: longbits.c
matfunc.o: alloc.h
matfunc.o: block.h
matfunc.o: byteswap.h
matfunc.o: calcerr.h
matfunc.o: cmath.h
matfunc.o: config.h
matfunc.o: endian_calc.h
matfunc.o: hash.h
matfunc.o: have_const.h
matfunc.o: have_malloc.h
matfunc.o: have_memmv.h
matfunc.o: have_newstr.h
matfunc.o: have_stdlib.h
matfunc.o: have_string.h
matfunc.o: have_unused.h
matfunc.o: longbits.h
matfunc.o: matfunc.c
matfunc.o: md5.h
matfunc.o: nametype.h
matfunc.o: qmath.h
matfunc.o: shs.h
matfunc.o: shs1.h
matfunc.o: string.h
matfunc.o: value.h
matfunc.o: win32dll.h
matfunc.o: zmath.h
matfunc.o: zrand.h
math_error.o: alloc.h
math_error.o: args.h
math_error.o: block.h
math_error.o: byteswap.h
math_error.o: calc.h
math_error.o: calcerr.h
math_error.o: cmath.h
math_error.o: config.h
math_error.o: endian_calc.h
math_error.o: hash.h
math_error.o: have_const.h
math_error.o: have_malloc.h
math_error.o: have_memmv.h
math_error.o: have_newstr.h
math_error.o: have_stdlib.h
math_error.o: have_string.h
math_error.o: longbits.h
math_error.o: math_error.c
math_error.o: math_error.h
math_error.o: md5.h
math_error.o: nametype.h
math_error.o: qmath.h
math_error.o: shs.h
math_error.o: shs1.h
math_error.o: string.h
math_error.o: value.h
math_error.o: win32dll.h
math_error.o: zmath.h
md5.o: align32.h
md5.o: alloc.h
md5.o: block.h
md5.o: byteswap.h
md5.o: calcerr.h
md5.o: cmath.h
md5.o: config.h
md5.o: endian_calc.h
md5.o: hash.h
md5.o: have_malloc.h
md5.o: have_memmv.h
md5.o: have_newstr.h
md5.o: have_stdlib.h
md5.o: have_string.h
md5.o: longbits.h
md5.o: md5.c
md5.o: md5.h
md5.o: nametype.h
md5.o: qmath.h
md5.o: shs.h
md5.o: shs1.h
md5.o: string.h
md5.o: value.h
md5.o: win32dll.h
md5.o: zmath.h
no_implicit.o: no_implicit.c
obj.o: alloc.h
obj.o: block.h
obj.o: byteswap.h
obj.o: calc.h
obj.o: calcerr.h
obj.o: cmath.h
obj.o: config.h
obj.o: endian_calc.h
obj.o: func.h
obj.o: hash.h
obj.o: have_const.h
obj.o: have_malloc.h
obj.o: have_memmv.h
obj.o: have_newstr.h
obj.o: have_stdlib.h
obj.o: have_string.h
obj.o: label.h
obj.o: longbits.h
obj.o: md5.h
obj.o: nametype.h
obj.o: obj.c
obj.o: opcodes.h
obj.o: qmath.h
obj.o: shs.h
obj.o: shs1.h
obj.o: string.h
obj.o: symbol.h
obj.o: value.h
obj.o: win32dll.h
obj.o: zmath.h
opcodes.o: alloc.h
opcodes.o: block.h
opcodes.o: byteswap.h
opcodes.o: calc.h
opcodes.o: calcerr.h
opcodes.o: cmath.h
opcodes.o: config.h
opcodes.o: custom.h
opcodes.o: endian_calc.h
opcodes.o: file.h
opcodes.o: func.h
opcodes.o: hash.h
opcodes.o: have_const.h
opcodes.o: have_fpos.h
opcodes.o: have_malloc.h
opcodes.o: have_memmv.h
opcodes.o: have_newstr.h
opcodes.o: have_stdlib.h
opcodes.o: have_string.h
opcodes.o: have_unused.h
opcodes.o: hist.h
opcodes.o: label.h
opcodes.o: longbits.h
opcodes.o: math_error.h
opcodes.o: md5.h
opcodes.o: nametype.h
opcodes.o: opcodes.c
opcodes.o: opcodes.h
opcodes.o: qmath.h
opcodes.o: shs.h
opcodes.o: shs1.h
opcodes.o: string.h
opcodes.o: symbol.h
opcodes.o: value.h
opcodes.o: win32dll.h
opcodes.o: zmath.h
opcodes.o: zrand.h
opcodes.o: zrandom.h
pix.o: alloc.h
pix.o: byteswap.h
pix.o: endian_calc.h
pix.o: have_const.h
pix.o: have_malloc.h
pix.o: have_memmv.h
pix.o: have_newstr.h
pix.o: have_stdlib.h
pix.o: have_string.h
pix.o: longbits.h
pix.o: pix.c
pix.o: prime.h
pix.o: qmath.h
pix.o: win32dll.h
pix.o: zmath.h
poly.o: alloc.h
poly.o: block.h
poly.o: byteswap.h
poly.o: calcerr.h
poly.o: cmath.h
poly.o: config.h
poly.o: endian_calc.h
poly.o: hash.h
poly.o: have_malloc.h
poly.o: have_memmv.h
poly.o: have_newstr.h
poly.o: have_stdlib.h
poly.o: have_string.h
poly.o: longbits.h
poly.o: md5.h
poly.o: nametype.h
poly.o: poly.c
poly.o: qmath.h
poly.o: shs.h
poly.o: shs1.h
poly.o: string.h
poly.o: value.h
poly.o: win32dll.h
poly.o: zmath.h
prime.o: alloc.h
prime.o: byteswap.h
prime.o: endian_calc.h
prime.o: have_const.h
prime.o: have_malloc.h
prime.o: have_memmv.h
prime.o: have_newstr.h
prime.o: have_stdlib.h
prime.o: have_string.h
prime.o: jump.h
prime.o: longbits.h
prime.o: prime.c
prime.o: prime.h
prime.o: qmath.h
prime.o: win32dll.h
prime.o: zmath.h
qfunc.o: alloc.h
qfunc.o: byteswap.h
qfunc.o: config.h
qfunc.o: endian_calc.h
qfunc.o: have_const.h
qfunc.o: have_malloc.h
qfunc.o: have_memmv.h
qfunc.o: have_newstr.h
qfunc.o: have_stdlib.h
qfunc.o: have_string.h
qfunc.o: longbits.h
qfunc.o: nametype.h
qfunc.o: prime.h
qfunc.o: qfunc.c
qfunc.o: qmath.h
qfunc.o: win32dll.h
qfunc.o: zmath.h
qio.o: alloc.h
qio.o: args.h
qio.o: byteswap.h
qio.o: config.h
qio.o: endian_calc.h
qio.o: have_malloc.h
qio.o: have_memmv.h
qio.o: have_newstr.h
qio.o: have_stdlib.h
qio.o: have_string.h
qio.o: have_unused.h
qio.o: longbits.h
qio.o: nametype.h
qio.o: qio.c
qio.o: qmath.h
qio.o: win32dll.h
qio.o: zmath.h
qmath.o: alloc.h
qmath.o: byteswap.h
qmath.o: config.h
qmath.o: endian_calc.h
qmath.o: have_malloc.h
qmath.o: have_memmv.h
qmath.o: have_newstr.h
qmath.o: have_stdlib.h
qmath.o: have_string.h
qmath.o: longbits.h
qmath.o: nametype.h
qmath.o: qmath.c
qmath.o: qmath.h
qmath.o: win32dll.h
qmath.o: zmath.h
qmod.o: alloc.h
qmod.o: byteswap.h
qmod.o: config.h
qmod.o: endian_calc.h
qmod.o: have_malloc.h
qmod.o: have_memmv.h
qmod.o: have_newstr.h
qmod.o: have_stdlib.h
qmod.o: have_string.h
qmod.o: longbits.h
qmod.o: nametype.h
qmod.o: qmath.h
qmod.o: qmod.c
qmod.o: win32dll.h
qmod.o: zmath.h
qtrans.o: alloc.h
qtrans.o: byteswap.h
qtrans.o: endian_calc.h
qtrans.o: have_malloc.h
qtrans.o: have_memmv.h
qtrans.o: have_newstr.h
qtrans.o: have_stdlib.h
qtrans.o: have_string.h
qtrans.o: longbits.h
qtrans.o: qmath.h
qtrans.o: qtrans.c
qtrans.o: win32dll.h
qtrans.o: zmath.h
quickhash.o: alloc.h
quickhash.o: block.h
quickhash.o: byteswap.h
quickhash.o: calcerr.h
quickhash.o: cmath.h
quickhash.o: config.h
quickhash.o: endian_calc.h
quickhash.o: hash.h
quickhash.o: have_const.h
quickhash.o: have_malloc.h
quickhash.o: have_memmv.h
quickhash.o: have_newstr.h
quickhash.o: have_stdlib.h
quickhash.o: have_string.h
quickhash.o: longbits.h
quickhash.o: md5.h
quickhash.o: nametype.h
quickhash.o: qmath.h
quickhash.o: quickhash.c
quickhash.o: shs.h
quickhash.o: shs1.h
quickhash.o: string.h
quickhash.o: value.h
quickhash.o: win32dll.h
quickhash.o: zmath.h
quickhash.o: zrand.h
quickhash.o: zrandom.h
seed.o: alloc.h
seed.o: byteswap.h
seed.o: endian_calc.h
seed.o: have_getpgid.h
seed.o: have_getprid.h
seed.o: have_getsid.h
seed.o: have_gettime.h
seed.o: have_malloc.h
seed.o: have_memmv.h
seed.o: have_newstr.h
seed.o: have_rusage.h
seed.o: have_stdlib.h
seed.o: have_string.h
seed.o: have_times.h
seed.o: have_uid_t.h
seed.o: have_unistd.h
seed.o: have_urandom.h
seed.o: have_ustat.h
seed.o: longbits.h
seed.o: qmath.h
seed.o: seed.c
seed.o: win32dll.h
seed.o: zmath.h
shs.o: align32.h
shs.o: alloc.h
shs.o: block.h
shs.o: byteswap.h
shs.o: calcerr.h
shs.o: cmath.h
shs.o: config.h
shs.o: endian_calc.h
shs.o: hash.h
shs.o: have_malloc.h
shs.o: have_memmv.h
shs.o: have_newstr.h
shs.o: have_stdlib.h
shs.o: have_string.h
shs.o: longbits.h
shs.o: md5.h
shs.o: nametype.h
shs.o: qmath.h
shs.o: shs.c
shs.o: shs.h
shs.o: shs1.h
shs.o: string.h
shs.o: value.h
shs.o: win32dll.h
shs.o: zmath.h
shs1.o: align32.h
shs1.o: alloc.h
shs1.o: block.h
shs1.o: byteswap.h
shs1.o: calcerr.h
shs1.o: cmath.h
shs1.o: config.h
shs1.o: endian_calc.h
shs1.o: hash.h
shs1.o: have_malloc.h
shs1.o: have_memmv.h
shs1.o: have_newstr.h
shs1.o: have_stdlib.h
shs1.o: have_string.h
shs1.o: longbits.h
shs1.o: md5.h
shs1.o: nametype.h
shs1.o: qmath.h
shs1.o: shs.h
shs1.o: shs1.c
shs1.o: shs1.h
shs1.o: string.h
shs1.o: value.h
shs1.o: win32dll.h
shs1.o: zmath.h
size.o: alloc.h
size.o: block.h
size.o: byteswap.h
size.o: calcerr.h
size.o: cmath.h
size.o: config.h
size.o: endian_calc.h
size.o: hash.h
size.o: have_const.h
size.o: have_malloc.h
size.o: have_memmv.h
size.o: have_newstr.h
size.o: have_stdlib.h
size.o: have_string.h
size.o: longbits.h
size.o: md5.h
size.o: nametype.h
size.o: qmath.h
size.o: shs.h
size.o: shs1.h
size.o: size.c
size.o: string.h
size.o: value.h
size.o: win32dll.h
size.o: zmath.h
size.o: zrand.h
size.o: zrandom.h
string.o: alloc.h
string.o: block.h
string.o: byteswap.h
string.o: calc.h
string.o: calcerr.h
string.o: cmath.h
string.o: config.h
string.o: endian_calc.h
string.o: hash.h
string.o: have_const.h
string.o: have_malloc.h
string.o: have_memmv.h
string.o: have_newstr.h
string.o: have_stdlib.h
string.o: have_string.h
string.o: longbits.h
string.o: md5.h
string.o: nametype.h
string.o: qmath.h
string.o: shs.h
string.o: shs1.h
string.o: string.c
string.o: string.h
string.o: value.h
string.o: win32dll.h
string.o: zmath.h
symbol.o: alloc.h
symbol.o: block.h
symbol.o: byteswap.h
symbol.o: calc.h
symbol.o: calcerr.h
symbol.o: cmath.h
symbol.o: config.h
symbol.o: endian_calc.h
symbol.o: func.h
symbol.o: hash.h
symbol.o: have_const.h
symbol.o: have_malloc.h
symbol.o: have_memmv.h
symbol.o: have_newstr.h
symbol.o: have_stdlib.h
symbol.o: have_string.h
symbol.o: label.h
symbol.o: longbits.h
symbol.o: md5.h
symbol.o: nametype.h
symbol.o: opcodes.h
symbol.o: qmath.h
symbol.o: shs.h
symbol.o: shs1.h
symbol.o: string.h
symbol.o: symbol.c
symbol.o: symbol.h
symbol.o: token.h
symbol.o: value.h
symbol.o: win32dll.h
symbol.o: zmath.h
token.o: alloc.h
token.o: args.h
token.o: block.h
token.o: byteswap.h
token.o: calc.h
token.o: calcerr.h
token.o: cmath.h
token.o: config.h
token.o: endian_calc.h
token.o: hash.h
token.o: have_const.h
token.o: have_malloc.h
token.o: have_memmv.h
token.o: have_newstr.h
token.o: have_stdlib.h
token.o: have_string.h
token.o: longbits.h
token.o: math_error.h
token.o: md5.h
token.o: nametype.h
token.o: qmath.h
token.o: shs.h
token.o: shs1.h
token.o: string.h
token.o: token.c
token.o: token.h
token.o: value.h
token.o: win32dll.h
token.o: zmath.h
value.o: alloc.h
value.o: block.h
value.o: byteswap.h
value.o: calc.h
value.o: calcerr.h
value.o: cmath.h
value.o: config.h
value.o: endian_calc.h
value.o: file.h
value.o: func.h
value.o: hash.h
value.o: have_const.h
value.o: have_fpos.h
value.o: have_malloc.h
value.o: have_memmv.h
value.o: have_newstr.h
value.o: have_stdlib.h
value.o: have_string.h
value.o: label.h
value.o: longbits.h
value.o: md5.h
value.o: nametype.h
value.o: opcodes.h
value.o: qmath.h
value.o: shs.h
value.o: shs1.h
value.o: string.h
value.o: symbol.h
value.o: value.c
value.o: value.h
value.o: win32dll.h
value.o: zmath.h
value.o: zrand.h
value.o: zrandom.h
version.o: alloc.h
version.o: block.h
version.o: byteswap.h
version.o: calc.h
version.o: calcerr.h
version.o: cmath.h
version.o: config.h
version.o: endian_calc.h
version.o: hash.h
version.o: have_const.h
version.o: have_malloc.h
version.o: have_memmv.h
version.o: have_newstr.h
version.o: have_stdlib.h
version.o: have_string.h
version.o: have_unused.h
version.o: longbits.h
version.o: md5.h
version.o: nametype.h
version.o: qmath.h
version.o: shs.h
version.o: shs1.h
version.o: string.h
version.o: value.h
version.o: version.c
version.o: win32dll.h
version.o: zmath.h
zfunc.o: alloc.h
zfunc.o: byteswap.h
zfunc.o: endian_calc.h
zfunc.o: have_malloc.h
zfunc.o: have_memmv.h
zfunc.o: have_newstr.h
zfunc.o: have_stdlib.h
zfunc.o: have_string.h
zfunc.o: longbits.h
zfunc.o: win32dll.h
zfunc.o: zfunc.c
zfunc.o: zmath.h
zio.o: alloc.h
zio.o: args.h
zio.o: byteswap.h
zio.o: config.h
zio.o: endian_calc.h
zio.o: have_malloc.h
zio.o: have_memmv.h
zio.o: have_newstr.h
zio.o: have_stdlib.h
zio.o: have_string.h
zio.o: longbits.h
zio.o: nametype.h
zio.o: qmath.h
zio.o: win32dll.h
zio.o: zio.c
zio.o: zmath.h
zmath.o: alloc.h
zmath.o: byteswap.h
zmath.o: endian_calc.h
zmath.o: have_malloc.h
zmath.o: have_memmv.h
zmath.o: have_newstr.h
zmath.o: have_stdlib.h
zmath.o: have_string.h
zmath.o: longbits.h
zmath.o: win32dll.h
zmath.o: zmath.c
zmath.o: zmath.h
zmod.o: alloc.h
zmod.o: byteswap.h
zmod.o: config.h
zmod.o: endian_calc.h
zmod.o: have_malloc.h
zmod.o: have_memmv.h
zmod.o: have_newstr.h
zmod.o: have_stdlib.h
zmod.o: have_string.h
zmod.o: longbits.h
zmod.o: nametype.h
zmod.o: qmath.h
zmod.o: win32dll.h
zmod.o: zmath.h
zmod.o: zmod.c
zmul.o: alloc.h
zmul.o: byteswap.h
zmul.o: config.h
zmul.o: endian_calc.h
zmul.o: have_malloc.h
zmul.o: have_memmv.h
zmul.o: have_newstr.h
zmul.o: have_stdlib.h
zmul.o: have_string.h
zmul.o: longbits.h
zmul.o: nametype.h
zmul.o: qmath.h
zmul.o: win32dll.h
zmul.o: zmath.h
zmul.o: zmul.c
zprime.o: alloc.h
zprime.o: block.h
zprime.o: byteswap.h
zprime.o: calcerr.h
zprime.o: cmath.h
zprime.o: config.h
zprime.o: endian_calc.h
zprime.o: hash.h
zprime.o: have_const.h
zprime.o: have_malloc.h
zprime.o: have_memmv.h
zprime.o: have_newstr.h
zprime.o: have_stdlib.h
zprime.o: have_string.h
zprime.o: jump.h
zprime.o: longbits.h
zprime.o: md5.h
zprime.o: nametype.h
zprime.o: prime.h
zprime.o: qmath.h
zprime.o: shs.h
zprime.o: shs1.h
zprime.o: string.h
zprime.o: value.h
zprime.o: win32dll.h
zprime.o: zmath.h
zprime.o: zprime.c
zprime.o: zrand.h
zrand.o: alloc.h
zrand.o: block.h
zrand.o: byteswap.h
zrand.o: calcerr.h
zrand.o: cmath.h
zrand.o: config.h
zrand.o: endian_calc.h
zrand.o: hash.h
zrand.o: have_const.h
zrand.o: have_malloc.h
zrand.o: have_memmv.h
zrand.o: have_newstr.h
zrand.o: have_stdlib.h
zrand.o: have_string.h
zrand.o: have_unused.h
zrand.o: longbits.h
zrand.o: md5.h
zrand.o: nametype.h
zrand.o: qmath.h
zrand.o: shs.h
zrand.o: shs1.h
zrand.o: string.h
zrand.o: value.h
zrand.o: win32dll.h
zrand.o: zmath.h
zrand.o: zrand.c
zrand.o: zrand.h
zrandom.o: alloc.h
zrandom.o: block.h
zrandom.o: byteswap.h
zrandom.o: calcerr.h
zrandom.o: cmath.h
zrandom.o: config.h
zrandom.o: endian_calc.h
zrandom.o: hash.h
zrandom.o: have_const.h
zrandom.o: have_malloc.h
zrandom.o: have_memmv.h
zrandom.o: have_newstr.h
zrandom.o: have_stdlib.h
zrandom.o: have_string.h
zrandom.o: have_unused.h
zrandom.o: longbits.h
zrandom.o: md5.h
zrandom.o: nametype.h
zrandom.o: qmath.h
zrandom.o: shs.h
zrandom.o: shs1.h
zrandom.o: string.h
zrandom.o: value.h
zrandom.o: win32dll.h
zrandom.o: zmath.h
zrandom.o: zrandom.c
zrandom.o: zrandom.h
