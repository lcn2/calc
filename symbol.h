/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */


#if !defined(__SYMBOL_H__)
#define	__SYMBOL_H__


#include "zmath.h"


/*
 * Symbol Declarations.
 */
#define SYM_UNDEFINED	0	/* undefined symbol */
#define SYM_PARAM	1	/* parameter symbol */
#define SYM_LOCAL	2	/* local symbol */
#define SYM_GLOBAL	3	/* global symbol */
#define	SYM_STATIC	4	/* static symbol */

#define	SCOPE_GLOBAL	0	/* file scope level for global variables */
#define	SCOPE_STATIC	1	/* lowest file scope for static variables */


typedef struct global GLOBAL;
struct global {
	int g_len;		/* length of symbol name */
	short g_filescope;	/* file scope level of symbol (0 if global) */
	short g_funcscope;	/* function scope level of symbol */
	char *g_name;		/* global symbol name */
	VALUE g_value;		/* global symbol value */
	GLOBAL *g_next;		/* next symbol in hash chain */
};


/*
 * Routines to search for global symbols.
 */
extern GLOBAL *addglobal(char *name, BOOL isstatic);
extern GLOBAL *findglobal(char *name);


/*
 * Routines to return names of variables.
 */
extern char *localname(long n);
extern char *paramname(long n);
extern char *globalname(GLOBAL *sp);


/*
 * Routines to handle entering and leaving of scope levels.
 */
extern void resetscopes(void);
extern void enterfilescope(void);
extern void exitfilescope(void);
extern void enterfuncscope(void);
extern void exitfuncscope(void);
extern void endscope (char *name, BOOL isglobal);


/*
 * Other routines.
 */
extern long addlocal(char *name);
extern long findlocal(char *name);
extern long addparam(char *name);
extern long findparam(char *name);
extern void initlocals(void);
extern void initglobals(void);
extern int writeglobals(char *name);
extern int symboltype(char *name);
extern void showglobals(void);
extern void showallglobals(void);
extern void freeglobals(void);
extern void showstatics(void);
extern void freestatics(void);


#endif /* !__SYMBOL_H__ */
