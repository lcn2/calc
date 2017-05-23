#!/usr/bin/awk
#
# calcerr_h - help produce calcerr.h from calcerr.tbl
#
# Copyright (C) 1999  Landon Curt Noll
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
# Under source code control:	1996/05/23 17:38:44
# File existed as early as:	1996
#
# chongo <was here> /\oo/\	http://www.isthe.com/chongo/
# Share and enjoy!  :-) http://www.isthe.com/chongo/tech/comp/calc/
#
BEGIN {
    ebase = 10000;
    printf("#define E__BASE %d\t/* calc errors start above here */\n\n", ebase);
}
NF > 1 {
    if (length($1) > 7) {
	printf("#define %s\t", $1, NR);
    } else {
	printf("#define %s\t\t", $1, NR);
    }
    printf("%d\t/* ", ebase+NR);
    for (i=2; i < NF; ++i) {
	printf("%s ", $i);
    }
    printf("%s */\n", $NF);
}
END {
    printf("\n#define E__HIGHEST\t%d\t/* highest calc error */\n", NR+ebase);
    printf("#define E__COUNT\t\t%d\t/* number of calc errors */\n", NR);
    printf("#define E_USERDEF\t20000\t/* base of user defined errors */\n\n");
    printf("/* names of calc error values */\n");
}
