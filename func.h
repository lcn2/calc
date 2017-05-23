/*
 * func - built-in function interface definitions
 *
 * Copyright (C) 1999-2007,2014  David I. Bell
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
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:	1990/02/15 01:48:33
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_FUNC_H)
#define INCLUDE_FUNC_H


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
E_FUNC FUNC *curfunc;	/* NOTE: This is a function pointer, we need E_FUNC */


/*
 * Functions to handle functions.
 */
E_FUNC FUNC *findfunc(long index);
E_FUNC char *namefunc(long index);
E_FUNC BOOL evaluate(BOOL nestflag);
E_FUNC long adduserfunc(char *name);
E_FUNC void rmuserfunc(char *name);
E_FUNC void rmalluserfunc(void);
E_FUNC long getuserfunc(char *name);
E_FUNC void beginfunc(char *name, BOOL newflag);
E_FUNC int builtinopcode(long index);
E_FUNC char *builtinname(long index);
E_FUNC int dumpop(unsigned long *pc);
E_FUNC void addop(long op);
E_FUNC void endfunc(void);
E_FUNC void addopone(long op, long arg);
E_FUNC void addoptwo(long op, long arg1, long arg2);
E_FUNC void addoplabel(long op, LABEL *label);
E_FUNC void addopptr(long op, char *ptr);
E_FUNC void writeindexop(void);
E_FUNC void showbuiltins(void);
E_FUNC int getbuiltinfunc(char *name);
E_FUNC void builtincheck(long index, int count);
E_FUNC void addopfunction(long op, long index, int count);
E_FUNC void showfunctions(void);
E_FUNC void initfunctions(void);
E_FUNC void clearopt(void);
E_FUNC void updateoldvalue(FUNC *fp);
E_FUNC void calculate(FUNC *fp, int argcount);
E_FUNC VALUE builtinfunc(long index, int argcount, VALUE *stck);
E_FUNC void freenumbers(FUNC *);
E_FUNC void freefunc(FUNC *);


#endif /* !INCLUDE_FUNC_H */
