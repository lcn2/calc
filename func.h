/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */


#if !defined(__FUNC_H__)
#define	__FUNC_H__


#include "calc.h"
#include "label.h"


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
