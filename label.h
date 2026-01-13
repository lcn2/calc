/*
 * label - label handling routines
 *
 * Copyright (C) 1999-2007,2014,2026  David I. Bell
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   1990/02/15 01:48:33
 * File existed as early as:    before 1990
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_LABEL_H)
#  define INCLUDE_LABEL_H

#  define NULL_LABEL ((LABEL *)0)

/*
 * Label structures.
 */
typedef struct {
    long l_offset; /* offset into code of label */
    long l_chain;  /* offset into code of undefined chain */
    char *l_name;  /* name of label if any */
} LABEL;

extern void initlabels(void);
extern void definelabel(char *name);
extern void addlabel(char *name);
extern void clearlabel(LABEL *lp);
extern void setlabel(LABEL *lp);
extern void uselabel(LABEL *lp);
extern void checklabels(void);

#endif
