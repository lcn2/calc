/*
 * func - built-in function interface definitions
 *
 * Copyright (C) 1999  David I. Bell
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.4 $
 * @(#) $Id: func.h,v 29.4 2001/06/08 21:00:58 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/func.h,v $
 *
 * Under source code control:	1990/02/15 01:48:33
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__FUNC_H__)
#define __FUNC_H__


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "calc.h"
# include "label.h"
#else
# include <calc/calc.h>
# include <calc/label.h>
#endif


/*
 * Structure of a function.
 * The f_opcodes array is actually of variable size.
 */
typedef struct func FUNC;
struct func {
	FUNC *f_next;			/* next function in list */
	unsigned long f_opcodecount;	/* size of opcode array */
	unsigned int f_localcount;	/* number of local variables */
	unsigned int f_paramcount;	/* max number of parameters */
	char *f_name;			/* function name */
	VALUE f_savedvalue;		/* saved value of last expression */
	unsigned long f_opcodes[1];	/* array of opcodes (variable length) */
};


/*
 * Amount of space needed to allocate a function of n opcodes.
 */
#define funcsize(n) (sizeof(FUNC) + (n) * sizeof(long))


/*
 * Size of a character pointer rounded up to a number of opcodes.
 */
#define PTR_SIZE ((sizeof(char *) + sizeof(long) - 1) / sizeof(long))


/*
 * The current function being compiled.
 */
extern FUNC *curfunc;


/*
 * Functions to handle functions.
 */
extern FUNC *findfunc(long index);
extern char *namefunc(long index);
extern BOOL evaluate(BOOL nestflag);
extern long adduserfunc(char *name);
extern void rmuserfunc(char *name);
extern void rmalluserfunc(void);
extern long getuserfunc(char *name);
extern void beginfunc(char *name, BOOL newflag);
extern int builtinopcode(long index);
extern char *builtinname(long index);
extern int dumpop(unsigned long *pc);
extern void addop(long op);
extern void endfunc(void);
extern void addopone(long op, long arg);
extern void addoptwo(long op, long arg1, long arg2);
extern void addoplabel(long op, LABEL *label);
extern void addopptr(long op, char *ptr);
extern void writeindexop(void);
extern void showbuiltins(void);
extern int getbuiltinfunc(char *name);
extern void builtincheck(long index, int count);
extern void addopfunction(long op, long index, int count);
extern void showfunctions(void);
extern void initfunctions(void);
extern void clearopt(void);
extern void updateoldvalue(FUNC *fp);
extern void calculate(FUNC *fp, int argcount);
extern VALUE builtinfunc(long index, int argcount, VALUE *stck);
extern void freenumbers(FUNC *);
extern void freefunc(FUNC *);


#endif /* !__FUNC_H__ */
