#!/usr/bin/env make
#
# rpm.mk - Makefile for building rpm packages for calc
#
# Copyright (C) 2003,2014,2021,2023  Petteri Kettunen and Landon Curt Noll
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
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# Under source code control:	2003/02/16 20:21:39
# File existed as early as:	2003
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
#
# calculator by David I. Bell with help/mods from others
# Makefile by Petteri Kettunen with modifications from Landon Curt Noll

# IMPORTANT NOTE: The rpm process assumes that ~/.rpmmacros contains
#                 the following:
#
#	%_topdir	/var/tmp/rpmbuild
#	%_tmppath	%{_topdir}/tmp
#	%_builddir	%{_topdir}/BUILD
#	%_rpmdir	%{_topdir}/RPMS
#	%_sourcedir     %{_topdir}/SOURCES
#	%_specdir	%{_topdir}/SPECS
#	%_srcrpmdir	%{_topdir}/SRPMS
#	%_buildroot	%{_topdir}/BUILDROOT
#	%_rpmfilename	%%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm
#
# If you are signing the rpms, you also need:
#
#	%_signature	gpg
#	%_gpg_path	~/.gnupg
#	%_gpg_name	__YOUR_NAME_HERE__
#	%_gpgbin	/usr/bin/gpg

# IMPORTANT NOTE: Unless the package redhat-rpm-config is installed,
#		  the calc-debuginfo rpm will not be created.
#
# IMPORTANT NOTE: These packages are important for general
#		  rpm processing:
#
#	rpm
#	rpm-python
#	rpmlint
#	rpm-build
#	rpm-libs
#	redhat-rpm-config

# setup
#
SHELL= bash
RPMBUILD_TOOL= rpmbuild
TARCH= x86_64
RPMBUILD_OPTION= -ba --target=$(TARCH) --buildroot=${RPM_BUILD_ROOT}
RPM_TOOL= rpm
SED= sed
FIND= find
BZIP2_PROG= bzip2
TAR= tar
RM= rm
LS= ls
CPIO= cpio
CP= cp
EGREP= egrep
MKDIR= mkdir
GREP= grep
SORT= sort
CHMOD= chmod
LN= ln

# rpm-related parameters
#
NAME= calc
PROJECT_NAME= ${NAME}
PROJECT_VERSION=
PROJECT_RELEASE=
PROJECT= ${PROJECT_NAME}-${PROJECT_VERSION}
SPECFILE= ${PROJECT_NAME}.spec
TARBALL= ${PROJECT}.${TAR}.bz2
RPMx86_64= ${PROJECT}-${PROJECT_RELEASE}.${TARCH}.rpm
DRPMx86_64= \
  ${PROJECT_NAME}-devel-${PROJECT_VERSION}-${PROJECT_RELEASE}.${TARCH}.rpm
SRPM= ${PROJECT}-${PROJECT_RELEASE}.src.rpm
# NOTE: RPM_TOP must be the same as %_topdir in ~/.rpmmacros
RPM_TOP= /var/tmp/rpmbuild
RPM_BUILD_ROOT= ${RPM_TOP}/BUILDROOT
TMPDIR= ${RPM_TOP}/tmp

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

all: ver_calc calc.spec
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	PROJECT_VERSION="`./ver_calc`"; \
	PROJECT_RELEASE="`${SED} -n \
	    -e '/^Release:/s/^Release: *//p' calc.spec.in`"; \
	$(MAKE) -f rpm.mk rpm \
	    PROJECT_VERSION="$$PROJECT_VERSION" \
	    PROJECT_RELEASE="$$PROJECT_RELEASE" Q= S= E=
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

pkgme: $(PROJECT_NAME)-spec.${TAR}.bz2

ver_calc:
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	$(MAKE) -f Makefile ver_calc Q= S= E=
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: vers
vers:
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	$(MAKE) -f Makefile ver_calc Q= S= E=
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

calc.spec: calc.spec.in ver_calc
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	${RM} -f calc.spec
	${SED} -e 's/<<<PROJECT_VERSION>>>/'"`./ver_calc`"/g \
	    calc.spec.in > calc.spec
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: srcpkg
srcpkg: make_rhdir
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	${V} echo RPM_TOP="${RPM_TOP}"
	${V} echo RPM_BUILD_ROOT="${RPM_BUILD_ROOT}"
	${V} echo PROJECT_VERSION="${PROJECT_VERSION}"
	${V} echo PROJECT_RELEASE="${PROJECT_RELEASE}"
	${RM} -rf "$(TMPDIR)/$(PROJECT)"
	${FIND} . -depth -print | \
	    ${EGREP} -v '/RCS|/CVS|/NOTES|/\.|\.out$$|\.safe$$\.tar\.bz2$$' | \
	    ${EGREP} -v '/old[._-]|\.old$$|\.tar\.gz$$|/ver_calc$$' | \
	    LANG=C ${SORT} | \
	    ${CPIO} -dumpv "$(TMPDIR)/$(PROJECT)"
	${RM} -f "$(TMPDIR)/$(PROJECT)/Makefile"
	${SED} -e 's/^CCWERR=[ 	]*-Werror/CCWERR=/' \
	       -e 's/^#.*CCWERR=.*-Werror$$/#/' \
	       -e 's/^CHECK= check/CHECK= true/' \
	    Makefile > "$(TMPDIR)/$(PROJECT)/Makefile"
	${CHMOD} 0444 "$(TMPDIR)/$(PROJECT)/Makefile"
	cd "$(TMPDIR)"; ${TAR} cf - "$(PROJECT)" | \
	  ${BZIP2_PROG} --best -c -z > "$(RPM_TOP)/SOURCES/$(TARBALL)"
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: rpm
rpm: srcpkg calc.spec
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	${V} echo RPM_TOP="${RPM_TOP}"
	${V} echo RPM_BUILD_ROOT="${RPM_BUILD_ROOT}"
	${V} echo PROJECT_VERSION="${PROJECT_VERSION}"
	${V} echo PROJECT_RELEASE="${PROJECT_RELEASE}"
	$(MAKE) -f Makefile clean Q= S= E=
	${CP} "$(SPECFILE)" "$(RPM_TOP)/SPECS/$(SPECFILE)"
	${RM} -f "$(RPM_TOP)/RPMS/$(TARCH)/$(RPMx86_64)"
	${RM} -f "$(RPM_TOP)/RPMS/$(TARCH)/$(DRPMx86_64)"
	${RM} -f "$(RPM_TOP)/SRPMS/$(SRPM)"
	${RPMBUILD_TOOL} ${RPMBUILD_OPTION} "$(RPM_TOP)/SPECS/$(SPECFILE)"
	@if [ ! -f "$(RPM_TOP)/SRPMS/$(SRPM)" ]; then \
	    echo "SRPMS/$(SRPM) not found" 1>&2; \
	    exit 3; \
	fi
	@echo
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: make_rhdir
make_rhdir:
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	${V} echo RPM_TOP="${RPM_TOP}"
	${V} echo RPM_BUILD_ROOT="${RPM_BUILD_ROOT}"
	${V} echo PROJECT_VERSION="${PROJECT_VERSION}"
	${V} echo PROJECT_RELEASE="${PROJECT_RELEASE}"
	for subdir in "" BUILD RPMS SOURCES SPECS SRPMS tmp; do \
	    i="$(RPM_TOP)/$$subdir"; \
	    if [ ! -d "$$i" ] ; then \
		${MKDIR} -p "$$i"; \
	    fi; \
	    if [ ! -d "$$i" ] ; then \
		echo "FATAL: unable to create: $$i" 1>&2; \
		exit 4; \
	    fi; \
	    if [ ! -w "$$i" ] ; then \
		${CHMOD} +w "$$i"; \
	    fi; \
	    if [ ! -w "$$i" ] ; then \
		echo "FATAL: unable to make writable: $$i" 1>&2; \
		exit 5; \
	    fi; \
	done;
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: -preclean
rpm-preclean:
	${V} echo '=-=-=-=-= ${MAKE_FILE} start of $@ rule =-=-=-=-='
	${Q} if [ -n "$(RPM_TOP)" ]; then \
	    if [ -d "$(RPM_TOP)" ]; then \
		echo ${RM} -rf "$(RPM_TOP)"; \
		${RM} -rf "$(RPM_TOP)"; \
	    fi; \
	    if [ -d "$(RPM_TOP)" ]; then \
		echo "FATAL: cannot rm -rf $(RPM_TOP)" 1>&2; \
		exit 1; \
	    fi; \
	    echo ${MKDIR} -p "$(RPM_TOP)"; \
	    ${MKDIR} -p "$(RPM_TOP)"; \
	    if [ ! -d "$(RPM_TOP)" ]; then \
		echo "FATAL: cannot mkdir -p $(RPM_TOP)" 1>&2; \
		exit 1; \
	    fi; \
	else \
	    echo "FATAL: make symbol RPM_TOP is empty" 1>&2; \
	    exit 2; \
	fi
	${V} echo '=-=-=-=-= ${MAKE_FILE} end of $@ rule =-=-=-=-='

# date format for spec file
.PHONY: logdate
logdate:
	echo "`date +'* %a %b %d %Y'` `whoami`"

.PHONY: chkpkg
chkpkg:
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	for i in "$(RPM_TOP)/RPMS/$(TARCH)/$(RPMx86_64)" \
		 "$(RPM_TOP)/RPMS/$(TARCH)/$(DRPMx86_64)" \
		 "$(RPM_TOP)/SRPMS/$(SRPM)" ; do \
	    echo "***** start $$i" ; \
	    ${RPM_TOOL} -qpi "$$"i ; \
	    echo "***** files $$i" ; \
	    ${RPM_TOOL} -qpl "$$i" ; \
	    echo "***** end $$i" ; \
	done ;
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: chksys
chksys:
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	${RPM_TOOL} -qa | ${GREP} "$(PROJECT_NAME)"
	${RPM_TOOL} -qa | ${GREP} "$(PROJECT_NAME)-devel"
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: test
test: ver_calc
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	@if [ X"`id -u`" != X"0" ]; then \
	    echo "test needs to install, must be root to test" 1>&2; \
	    exit 6; \
	fi
	$(MAKE) -f rpm.mk PROJECT_VERSION="`./ver_calc`" installrpm \
		chksys Q= S= E=
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: installrpm
installrpm:
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	@if [ X"`id -u`" != X"0" ]; then \
	    echo "must be root to install RPMs" 1>&2; \
	    exit 7; \
	fi
	${RPM_TOOL} -ivh "$(RPM_TOP)/RPMS/$(TARCH)/$(RPMx86_64)"
	${RPM_TOOL} -ivh "$(RPM_TOP)/RPMS/$(TARCH)/$(DRPMx86_64)"
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

.PHONY: uninstallrpm
uninstallrpm:
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	@if [ X"`id -u`" != X"0" ]; then \
	    echo "must be root to uninstall RPMs" 1>&2; \
	    exit 8; \
	fi
	${RPM_TOOL} -e "$(PROJECT_NAME)-devel"
	${RPM_TOOL} -e "$(PROJECT_NAME)"
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

$(PROJECT_NAME)-spec.${TAR}.bz2: rpm.mk $(PROJECT_NAME).spec.in
	${V} echo '=-=-=-=-= rpm.mk start of $@ rule =-=-=-=-='
	${RM} -f "$@"
	${TAR} cf - "$^" | ${BZIP2_PROG} --best -c -z > "$@"
	${V} echo '=-=-=-=-= rpm.mk end of $@ rule =-=-=-=-='

#****
