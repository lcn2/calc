#!/bin/make
#****h* calc/rpm.mk
#
# rpm.mk - Makefile for building rpm packages for calc
#
# Copyright (C) 2003  Petteri Kettunen and Landon Curt Noll
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
MAKEFILE_REV= $$Revision: 29.12 $$
# @(#) $Id: rpm.mk,v 29.12 2003/02/26 17:36:14 chongo Exp $
# @(#) $Source: /usr/local/src/cmd/calc/RCS/rpm.mk,v $
#
# Under source code control:	2003/02/16 20:21:39
# File existed as early as:	2003
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
#
# calculator by David I. Bell with help/mods from others
# Makefile by Petteri Kettunen with modifications from Landon Curt Noll

# setup
#
SHELL= /bin/sh
RPMBUILD_TOOL= rpmbuild
RPM_TOOL= rpm
MD5SUM= md5sum
SHA1SUM= sha1sum
SED= sed

# rpm-related parameters
#
PROJECT_NAME= calc
PROJECT_VERSION=
PROJECT_RELEASE=
PROJECT= $(PROJECT_NAME)-$(PROJECT_VERSION)
SPECFILE= $(PROJECT_NAME).spec
TARBALL= $(PROJECT).tar.gz
RPM386= $(PROJECT)-$(PROJECT_RELEASE).i386.rpm
RPM686= $(PROJECT)-$(PROJECT_RELEASE).i686.rpm
DRPM386= $(PROJECT_NAME)-devel-$(PROJECT_VERSION)-$(PROJECT_RELEASE).i386.rpm
DRPM686= $(PROJECT_NAME)-devel-$(PROJECT_VERSION)-$(PROJECT_RELEASE).i686.rpm
SRPM= $(PROJECT)-$(PROJECT_RELEASE).src.rpm
TMPDIR= /var/tmp
RHDIR= /usr/src/redhat

all: calc.spec ver_calc
	$(MAKE) -f rpm.mk PROJECT_VERSION="`./ver_calc`" \
		PROJECT_RELEASE="`${SED} -n -e '/^Release:/s/^Release: *//p' \
				  calc.spec.in`" rpm

pkgme: $(PROJECT_NAME)-spec.tar.gz

ver_calc:
	$(MAKE) -f Makefile ver_calc

.PHONY: vers
vers:
	$(MAKE) -f Makefile ver_calc

calc.spec: calc.spec.in ver_calc
	rm -f calc.spec
	${SED} -e 's/<<<PROJECT_VERSION>>>/'"`./ver_calc`"/g \
	    calc.spec.in > calc.spec

.PHONY: srcpkg
srcpkg: make_rhdir
	find . -depth -print | egrep -v '/RCS|/CVS|/NOTES|\.gone' | \
	    cpio -dumpv $(TMPDIR)/$(PROJECT)
	(cd $(TMPDIR); tar cf - $(PROJECT) | \
	  gzip -c > $(RHDIR)/SOURCES/$(TARBALL))
	rm -fr $(TMPDIR)/$(PROJECT)

.PHONY: rpm
rpm: srcpkg calc.spec
	$(MAKE) -f Makefile clean
	cp $(SPECFILE) $(RHDIR)/SPECS/$(SPECFILE)
	rm -f $(RHDIR)/RPMS/i386/$(RPM386)
	rm -f $(RHDIR)/RPMS/i386/$(DRPM386)
	rm -f $(RHDIR)/SRPMS/$(SRPM)
	${RPMBUILD_TOOL} -ba $(RHDIR)/SPECS/$(SPECFILE)
	@if [ ! -f "RPMS/i386/$(RPM686)" ]; then \
	    rm -f "$(RHDIR)/RPMS/i386/$(RPM686)"; \
	    echo mv -f "$(RHDIR)/RPMS/i386/$(RPM386)" \
	    	       "$(RHDIR)/RPMS/i386/$(RPM686)"; \
	    mv -f "$(RHDIR)/RPMS/i386/$(RPM386)" \
	    	  "$(RHDIR)/RPMS/i386/$(RPM686)"; \
	else \
	    echo "RPMS/i386/$(RPM686) not found" 1>&2; \
	    exit 1; \
	fi
	@if [ ! -f "RPMS/i386/$(DRPM386)" ]; then \
	    rm -f "$(RHDIR)/RPMS/i386/$(DRPM686)"; \
	    echo mv -f "$(RHDIR)/RPMS/i386/$(DRPM386)" \
	    	       "$(RHDIR)/RPMS/i386/$(DRPM686)"; \
	    mv -f "$(RHDIR)/RPMS/i386/$(DRPM386)" \
	    	  "$(RHDIR)/RPMS/i386/$(DRPM686)"; \
	else \
	    echo "RPMS/i386/$(DRPM386) not found" 1>&2; \
	    exit 2; \
	fi
	@if [ ! -f "$(RHDIR)/SRPMS/$(SRPM)" ]; then \
	    echo "SRPMS/$(SRPM) not found" 1>&2; \
	    exit 3; \
	fi
	@echo
	@echo "RPM package sizes:"
	@echo
	@cd $(RHDIR); ls -1s RPMS/i386/$(RPM686) \
	    RPMS/i386/$(DRPM686) SRPMS/$(SRPM)
	@echo
	@echo "RPM package md5 hashes:"
	@echo
	-@cd $(RHDIR); ${MD5SUM} RPMS/i386/$(RPM686) \
	    RPMS/i386/$(DRPM686) SRPMS/$(SRPM)
	@echo
	@echo "RPM package sha1 hashes:"
	@echo
	-@cd $(RHDIR); ${SHA1SUM} RPMS/i386/$(RPM686) \
	    RPMS/i386/$(DRPM686) SRPMS/$(SRPM)
	@echo
	@echo "RPM package locations:"
	@echo
	@ls -1 $(RHDIR)/RPMS/i386/$(RPM686) \
	    $(RHDIR)/RPMS/i386/$(DRPM686) $(RHDIR)/SRPMS/$(SRPM)
	@echo
	@echo "All done! -- Jessica Noll, Age 2"
	@echo

.PHONY: make_rhdir
make_rhdir:
	for i in $(RHDIR) $(RHDIR)/RPMS $(RHDIR)/SOURCES \
	  $(RHDIR)/SPECS $(RHDIR)/SRPMS $(RHDIR)/BUILD; do \
	    if [ ! -d $$i ] ; then \
		mkdir -p $$i; \
	    fi; \
	done;

# date format for spec file
.PHONY: logdate
logdate:
	echo "`date +'* %a %b %d %Y'` `whoami`"

.PHONY: chkpkg
chkpkg:
	for i in $(RHDIR)/RPMS/i386/$(RPM686) $(RHDIR)/RPMS/i386/$(DRPM686) \
	  $(RHDIR)/SRPMS/$(SRPM) ; do \
	    echo "***** start $$i" ; \
		${RPM_TOOL} -qpi $$i ; \
		echo "***** files $$i" ; \
	    ${RPM_TOOL} -qpl $$i ; \
	    echo "***** end $$i" ; \
	done ;

.PHONY: chksys
chksys:
	${RPM_TOOL} -qa | grep $(PROJECT_NAME)
	${RPM_TOOL} -qa | grep $(PROJECT_NAME)-devel

.PHONY: test
test: ver_calc
	@if [ X"`id -u`" != X"0" ]; then \
	    echo "test needs to install, must be root to test" 1>&2; \
	    exit 4; \
	fi
	$(MAKE) -f rpm.mk PROJECT_VERSION="`./ver_calc`" installrpm chksys

.PHONY: installrpm
installrpm:
	@if [ X"`id -u`" != X"0" ]; then \
	    echo "must be root to install RPMs" 1>&2; \
	    exit 5; \
	fi
	${RPM_TOOL} -ivh $(RHDIR)/RPMS/i386/$(RPM686)
	${RPM_TOOL} -ivh $(RHDIR)/RPMS/i386/$(DRPM686)

.PHONY: uninstallrpm
uninstallrpm:
	@if [ X"`id -u`" != X"0" ]; then \
	    echo "must be root to uninstall RPMs" 1>&2; \
	    exit 6; \
	fi
	${RPM_TOOL} -e $(PROJECT_NAME)-devel
	${RPM_TOOL} -e $(PROJECT_NAME)

$(PROJECT_NAME)-spec.tar.gz: rpm.mk $(PROJECT_NAME).spec.in
	tar cf - $^ | gzip -c > $@

#****
