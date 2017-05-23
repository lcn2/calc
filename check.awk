#!/usr/bin/awk
#
# check - check the regression output for problems
#
# Copyright (C) 1999-2006  Landon Curt Noll
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
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# Under source code control:	1996/05/25 22:07:58
# File existed as early as:	1996
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-)		http://www.isthe.com/chongo/tech/comp/calc/
#
# This awk script will print 3 lines before and after any non-blank line that
# does not begin with a number.	 This allows the 'make debug' rule to remove
# all non-interest lines the 'make check' regression output while providing
# 3 lines of context around unexpected output.
#
BEGIN {
	havebuf0=0;
	buf0=0;
	havebuf1=0;
	buf1=0;
	havebuf2=0;
	buf2=0;
	error = 0;
	end_seen = 0;
}

NF == 0 {
	if (error > 0) {
		if (havebuf2) {
			print buf2;
		}
		--error;
	}
	buf2 = buf1;
	havebuf2 = havebuf1;
	buf1 = buf0;
	havebuf1 = havebuf0;
	buf0 = $0;
	havebuf0 = 1;
	next;
}

/: Ending regression tests$/ {
	end_seen = 1;
}

$1 ~ /^[0-9]+:/ || $1 ~ /^[0-9]+-[0-9]*:/ || $1 ~ /^"\)\)$/ {
	if (error > 0) {
		if (havebuf2) {
			print buf2;
		}
		--error;
	}
	buf2 = buf1;
	havebuf2 = havebuf1;
	buf1 = buf0;
	havebuf1 = havebuf0;
	buf0 = $0;
	havebuf0 = 1;
	next;
}

{
	error = 6;
	if (havebuf2) {
		print buf2;
	}
	buf2 = buf1;
	havebuf2 = havebuf1;
	buf1 = buf0;
	havebuf1 = havebuf0;
	buf0 = $0;
	havebuf0 = 1;
	next;
}

END {
	if (error > 0 && havebuf2) {
		print buf2;
		--error;
	}
	if (error > 0 && havebuf1) {
		print buf1;
		--error;
	}
	if (error > 0 && havebuf0) {
		print buf0;
	}
	if (error > 0 || !end_seen) {
		exit(1);
	} else {
		exit(0);
	}
}
