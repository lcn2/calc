/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Add opcodes to a function being compiled.
 */

#include <stdio.h>
#include "calc.h"
#include "opcodes.h"
#include "string.h"
#include "func.h"
#include "token.h"
#include "label.h"
#include "symbol.h"


#define FUNCALLOCSIZE	20	/* reallocate size for functions */
#define OPCODEALLOCSIZE 100	/* reallocate size for opcodes in functions */


static long maxopcodes;		/* number of opcodes available */
static long newindex;		/* index of new function */
static long oldop;		/* previous opcode */
static long oldoldop;		/* opcode before previous opcode */
static long debugline;		/* line number of latest debug opcode */
static long funccount;		/* number of functions */
static long funcavail;		/* available number of functions */
static FUNC *functemplate;	/* function definition template */
static FUNC **functions;	/* table of functions */
static STRINGHEAD funcnames;	/* function names */


/*
 * Initialize the table of user defined functions.
 */
void
initfunctions(void)
{
	initstr(&funcnames);
	maxopcodes = OPCODEALLOCSIZE;
	functemplate = (FUNC *) malloc(funcsize(maxopcodes));
	if (functemplate == NULL) {
		math_error("Cannot allocate function template");
		/*NOTREACHED*/
	}
	functions = (FUNC **) malloc(sizeof(FUNC *) * FUNCALLOCSIZE);
	if (functions == NULL) {
		math_error("Cannot allocate function table");
		/*NOTREACHED*/
	}
	funccount = 0;
	funcavail = FUNCALLOCSIZE;
}


/*
 * Show the list of user defined functions.
 */
void
showfunctions(void)
{
	FUNC **fpp;		/* pointer into function table */
	FUNC *fp;		/* current function */
	long count;

	count = 0;
	if (funccount > 0) {
		for (fpp = &functions[funccount - 1]; fpp >= functions; fpp--) {
			fp = *fpp;
			if (fp == NULL)
				continue;
			if (count++ == 0) {
				printf("Name Arguments\n---- ---------\n");
			}
			printf("%-12s %-2d\n", fp->f_name, fp->f_paramcount);
		}
	}
	if (count > 0) {
		printf("\nNumber: %ld\n", count);
	} else {
		printf("No user functions defined\n");
	}
}


/*
 * Initialize a function for definition.
 * Newflag is TRUE if we should allocate a new function structure,
 * instead of the usual overwriting of the template function structure.
 * The new structure is returned in the global curfunc variable.
 *
 * given:
 *	name		name of function
 *	newflag		TRUE if need new structure
 */
void
beginfunc(char *name, BOOL newflag)
{
	register FUNC *fp;		/* current function */

	newindex = adduserfunc(name);
	maxopcodes = OPCODEALLOCSIZE;
	fp = functemplate;
	if (newflag) {
		fp = (FUNC *) malloc(funcsize(maxopcodes));
		if (fp == NULL) {
			math_error("Cannot allocate temporary function");
			/*NOTREACHED*/
		}
	}
	fp->f_next = NULL;
	fp->f_localcount = 0;
	fp->f_opcodecount = 0;
	fp->f_savedvalue.v_type = V_NULL;
	fp->f_savedvalue.v_subtype = V_NOSUBTYPE;
	fp->f_name = namestr(&funcnames, newindex);
	curfunc = fp;
	initlocals();
	initlabels();
	oldop = OP_NOP;
	oldoldop = OP_NOP;
	debugline = 0;
	errorcount = 0;
}


/*
 * Commit the just defined function for use.
 * This replaces any existing definition for the function.
 * This should only be called for normal user-defined functions.
 */
void
endfunc(void)
{
	register FUNC *fp;		/* function just finished */
	unsigned long size;		/* size of just created function */
	long index;

	if (oldop != OP_RETURN) {
		addop(OP_UNDEF);
		addop(OP_RETURN);
	}
	checklabels();
	if (errorcount) {
		freefunc(curfunc);
		printf("\"%s\": %ld error%s\n", curfunc->f_name, errorcount,
			((errorcount == 1) ? "" : "s"));
		return;
	}
	size = funcsize(curfunc->f_opcodecount);
	fp = (FUNC *) malloc(size);
	if (fp == NULL) {
		math_error("Cannot commit function");
		/*NOTREACHED*/
	}
	memcpy((char *) fp, (char *) curfunc, size);
	if (curfunc != functemplate)
		free(curfunc);
	if (conf->traceflags & TRACE_FNCODES) {
		dumpnames = TRUE;
		for (size = 0; size < fp->f_opcodecount; ) {
			printf("%ld: ", (long)size);
			size += dumpop(&fp->f_opcodes[size]);
		}
	}
	if ((inputisterminal() && conf->lib_debug & LIBDBG_STDIN_FUNC) ||
		(!inputisterminal() && conf->lib_debug & LIBDBG_FILE_FUNC)) {
		printf("%s(", fp->f_name);
		for (index = 0; index <	 fp->f_paramcount; index++) {
			if (index)
				putchar(',');
			printf("%s", paramname(index));
		}
		printf(") ");
		if (functions[newindex])
			printf("re");
		printf("defined\n");
	}
	if (functions[newindex]) {
		freenumbers(functions[newindex]);
		free(functions[newindex]);
	}
	functions[newindex] = fp;
	objuncache();
}


/*
 * Find the user function with the specified name, and return its index.
 * If the function does not exist, its name is added to the function table
 * and an error will be generated when it is called if it is still undefined.
 *
 * given:
 *	name		name of function
 */
long
adduserfunc(char *name)
{
	long index;		/* index of function */

	index = findstr(&funcnames, name);
	if (index >= 0)
		return index;
	if (funccount >= funcavail) {
		functions = (FUNC **) realloc(functions,
			sizeof(FUNC *) * (funcavail + FUNCALLOCSIZE));
		if (functions == NULL) {
			math_error("Failed to reallocate function table");
			/*NOTREACHED*/
		}
		funcavail += FUNCALLOCSIZE;
	}
	if (addstr(&funcnames, name) == NULL) {
		math_error("Cannot save function name");
		/*NOTREACHED*/
	}
	index = funccount++;
	functions[index] = NULL;
	return index;
}

/*
 * Remove user defined function
 */
void
rmuserfunc(char *name)
{
	long index;		/* index of function */

	index = findstr(&funcnames, name);
	if (index < 0) {
		printf("%s() has never been defined\n",
			name);
		return;
	}
	if (functions[index] == NULL)
		return;
	freenumbers(functions[index]);
	free(functions[index]);
	if ((inputisterminal() && conf->lib_debug & LIBDBG_STDIN_FUNC) ||
		    (!inputisterminal() && conf->lib_debug & LIBDBG_FILE_FUNC))
		printf("%s() undefined\n", name);
	functions[index] = NULL;
}


/*
 * Free memory used to store function and its constants
 */
void
freefunc(FUNC *fp)
{
	long i;

	if (fp == NULL)
		return;
	if (conf->traceflags & TRACE_FNCODES) {
		printf("Freeing function \"%s\"\n", fp->f_name);
		dumpnames = FALSE;
		for (i = 0; i < fp->f_opcodecount; ) {
			printf("%ld: ", i);
			i += dumpop(&fp->f_opcodes[i]);
		}
	}
	freenumbers(fp);
	if (fp != functemplate)
		free(fp);
}


void
rmalluserfunc(void)
{
	FUNC **fpp;

	for (fpp = functions; fpp < &functions[funccount]; fpp++) {
		if (*fpp) {
			freefunc(*fpp);
			*fpp = NULL;
		}
	}
}


/*
 * get index of defined user function with specified name, or -1 if there
 * is none or if it has been undefined
 */
long
getuserfunc(char *name)
{
	long index;

	index = findstr(&funcnames, name);
	if (index >= 0 && functions[index] != NULL)
		return index;
	return -1L;
}


/*
 * Clear any optimization that may be done for the next opcode.
 * This is used when defining a label.
 */
void
clearopt(void)
{
	oldop = OP_NOP;
	oldoldop = OP_NOP;
	debugline = 0;
}


/*
 * Find a function structure given its index.
 */
FUNC *
findfunc(long index)
{
	if ((unsigned long) index >= funccount) {
		math_error("Undefined function");
		/*NOTREACHED*/
	}
	return functions[index];
}


/*
 * Return the name of a function given its index.
 */
char *
namefunc(long index)
{
	return namestr(&funcnames, index);
}


/*
 * Let a matrix indexing operation know that it will be treated as a write
 * reference instead of just as a read reference.
 */
void
writeindexop(void)
{
	if (oldop == OP_INDEXADDR)
		curfunc->f_opcodes[curfunc->f_opcodecount - 1] = TRUE;
}


/*
 * Add an opcode to the current function being compiled.
 * Note: This can change the curfunc global variable when the
 * function needs expanding.
 */
void
addop(long op)
{
	register FUNC *fp;		/* current function */
	NUMBER *q, *q1, *q2;
	unsigned long count;
	BOOL cut;
	int diff;

	fp = curfunc;
	count = fp->f_opcodecount;
	cut = TRUE;
	diff = 2;
	q = NULL;
	if ((count + 5) >= maxopcodes) {
		maxopcodes += OPCODEALLOCSIZE;
		fp = (FUNC *) malloc(funcsize(maxopcodes));
		if (fp == NULL) {
			math_error("cannot malloc function");
			/*NOTREACHED*/
		}
		memcpy((char *) fp, (char *) curfunc,
			funcsize(curfunc->f_opcodecount));
		if (curfunc != functemplate)
			free(curfunc);
		curfunc = fp;
	}

	/*
	 * Check the current opcode against the previous opcode and try to
	 * slightly optimize the code depending on the various combinations.
	 */
	switch (op) {
		case OP_GETVALUE:
			switch (oldop) {
				case OP_NUMBER:
				case OP_ZERO:
				case OP_ONE:
				case OP_IMAGINARY:
				case OP_GETEPSILON:
				case OP_SETEPSILON:
				case OP_STRING:
				case OP_UNDEF:
				case OP_GETCONFIG:
				case OP_SETCONFIG:
					return;
				case OP_DUPLICATE:
					diff = 1;
					oldop = OP_DUPVALUE;
					break;
				case OP_FIADDR:
					diff = 1;
					oldop = OP_FIVALUE;
					break;
				case OP_GLOBALADDR:
					diff = 1 + PTR_SIZE;
					oldop = OP_GLOBALVALUE;
					break;
				case OP_LOCALADDR:
					oldop = OP_LOCALVALUE;
					break;
				case OP_PARAMADDR:
					oldop = OP_PARAMVALUE;
					break;
				case OP_ELEMADDR:
					oldop = OP_ELEMVALUE;
					break;
				default:
					cut = FALSE;

			}
			if (cut) {
				fp->f_opcodes[count - diff] = oldop;
				return;
			}
			break;
		case OP_POP:
			switch (oldop) {
			case OP_ASSIGN:
				fp->f_opcodes[count-1] = OP_ASSIGNPOP;
				oldop = OP_ASSIGNPOP;
				return;
			case OP_NUMBER:
			case OP_IMAGINARY:
				q = constvalue(fp->f_opcodes[count-1]);
				qfree(q);
				break;
			case OP_STRING:
				sfree(findstring((long)fp->f_opcodes[count-1]));
				break;
			case OP_LOCALADDR:
			case OP_PARAMADDR:
				break;
			case OP_GLOBALADDR:
				diff = 1 + PTR_SIZE;
				break;
			case OP_UNDEF:
				fp->f_opcodecount -= 1;
				oldop = OP_NOP;
				oldoldop = OP_NOP;
				return;
			default:
				cut = FALSE;
			}
			if (cut) {
				fp->f_opcodecount -= diff;
				oldop = OP_NOP;
				oldoldop = OP_NOP;
				fprintf(stderr,
					"Line %ld: unused value ignored\n",
					linenumber());
				return;
			}
			break;
		case OP_NEGATE:
			if (oldop == OP_NUMBER) {
				q = constvalue(fp->f_opcodes[count-1]);
				fp->f_opcodes[count-1] = addqconstant(qneg(q));
				qfree(q);
				return;
			}
	}
	if (oldop == OP_NUMBER) {
		if (oldoldop == OP_NUMBER) {
			q1 = constvalue(fp->f_opcodes[count - 3]);
			q2 = constvalue(fp->f_opcodes[count - 1]);
			switch (op) {
				case OP_DIV:
					if (qiszero(q2)) {
						cut = FALSE;
						break;
					}
					q = qqdiv(q1,q2);
					break;
				case OP_MUL:
					q = qmul(q1,q2);
					break;
				case OP_ADD:
					q = qqadd(q1,q2);
					break;
				case OP_SUB:
					q = qsub(q1,q2);
					break;
				case OP_POWER:
					if (qisfrac(q2) || qisneg(q2))
						cut = FALSE;
					else
						q = qpowi(q1,q2);
					break;
				default:
					cut = FALSE;
			}
			if (cut) {
				qfree(q1);
				qfree(q2);
				fp->f_opcodes[count - 3] = addqconstant(q);
				fp->f_opcodecount -= 2;
				oldoldop = OP_NOP;
				return;
			}
		} else if (op != OP_NUMBER) {
			q = constvalue(fp->f_opcodes[count - 1]);
			if (op == OP_POWER) {
				if (qcmpi(q, 2L) == 0) {
					fp->f_opcodecount--;
					fp->f_opcodes[count - 2] = OP_SQUARE;
					qfree(q);
					oldop = OP_SQUARE;
					return;
				}
				if (qcmpi(q, 4L) == 0) {
					fp->f_opcodes[count - 2] = OP_SQUARE;
					fp->f_opcodes[count - 1] = OP_SQUARE;
					qfree(q);
					oldop = OP_SQUARE;
					return;
				}
			}
			if (qiszero(q)) {
				qfree(q);
				fp->f_opcodes[count - 2] = OP_ZERO;
				fp->f_opcodecount--;
			} else if (qisone(q)) {
				qfree(q);
				fp->f_opcodes[count - 2] = OP_ONE;
				fp->f_opcodecount--;
			}
		}
	}
	/*
	 * No optimization possible, so store the opcode.
	 */
	fp->f_opcodes[fp->f_opcodecount] = op;
	fp->f_opcodecount++;
	oldoldop = oldop;
	oldop = op;
}


/*
 * Add an opcode and and one integer argument to the current function
 * being compiled.
 */
void
addopone(long op, long arg)
{
	if (op == OP_DEBUG) {
		if ((conf->traceflags & TRACE_NODEBUG) || (arg == debugline))
			return;
		debugline = arg;
		if (oldop == OP_DEBUG) {
			curfunc->f_opcodes[curfunc->f_opcodecount - 1] = arg;
			return;
		}
	}
	addop(op);
	curfunc->f_opcodes[curfunc->f_opcodecount] = arg;
	curfunc->f_opcodecount++;
}


/*
 * Add an opcode and and two integer arguments to the current function
 * being compiled.
 */
void
addoptwo(long op, long arg1, long arg2)
{
	addop(op);
	curfunc->f_opcodes[curfunc->f_opcodecount++] = arg1;
	curfunc->f_opcodes[curfunc->f_opcodecount++] = arg2;
}


/*
 * Add an opcode and a character pointer to the function being compiled.
 */
void
addopptr(long op, char *ptr)
{
	char **ptraddr;

	addop(op);
	ptraddr = (char **) &curfunc->f_opcodes[curfunc->f_opcodecount];
	*ptraddr = ptr;
	curfunc->f_opcodecount += PTR_SIZE;
}


/*
 * Add an opcode and an index and an argument count for a function call.
 */
void
addopfunction(long op, long index, int count)
{
	long newop;

	if ((op == OP_CALL) && ((newop = builtinopcode(index)) != OP_NOP)) {
		if ((newop == OP_SETCONFIG) && (count == 1))
			newop = OP_GETCONFIG;
		if ((newop == OP_SETEPSILON) && (count == 0))
			newop = OP_GETEPSILON;
		if ((newop == OP_ABS) && (count == 1))
			addop(OP_GETEPSILON);
		addop(newop);
		return;
	}
	addop(op);
	curfunc->f_opcodes[curfunc->f_opcodecount++] = index;
	curfunc->f_opcodes[curfunc->f_opcodecount++] = count;
}


/*
 * Add a jump-type opcode and a label to the function being compiled.
 *
 * given:
 *	label		label to be added
 */
void
addoplabel(long op, LABEL *label)
{
	addop(op);
	uselabel(label);
}
