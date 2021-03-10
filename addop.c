/*
 * addop - add opcodes to a function being compiled
 *
 * Copyright (C) 1999-2007,2021  David I. Bell and Ernest Bowen
 *
 * Primary author:  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:10
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "calc.h"
#include "alloc.h"
#include "opcodes.h"
#include "str.h"
#include "func.h"
#include "token.h"
#include "label.h"
#include "symbol.h"


#include "banned.h"	/* include after system header <> includes */


#define FUNCALLOCSIZE	20	/* reallocate size for functions */
#define OPCODEALLOCSIZE 100	/* reallocate size for opcodes in functions */


STATIC unsigned long maxopcodes;/* number of opcodes available */
STATIC long newindex;		/* index of new function */
STATIC char *newname;		/* name of new function */
STATIC long oldop;		/* previous opcode */
STATIC long oldoldop;		/* opcode before previous opcode */
STATIC long debugline;		/* line number of latest debug opcode */
STATIC long funccount;		/* number of functions */
STATIC long funcavail;		/* available number of functions */
STATIC FUNC *functemplate;	/* function definition template */
STATIC FUNC **functions;	/* table of functions */
STATIC STRINGHEAD funcnames;	/* function names */


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
	FUNC *fp;		/* current function */
	long count;
	long index;

	count = 0;
	if (funccount > 0) {
		if (conf->resource_debug & RSCDBG_FUNC_INFO)
			math_str("Index\tName        \tArgs\tOpcodes\n"
				 "-----\t------     \t---- \t------\n");
		else
			math_str("Name\tArguments\n"
				 "----\t---------\n");
		for (index = 0; index < funccount; index++) {
			fp = functions[index];
			if (conf->resource_debug & RSCDBG_FUNC_INFO) {

				math_fmt("%5ld\t%-12s\t", index,
					namestr(&funcnames,index));
				if (fp) {
					count++;
					math_fmt("%-5d\t%-5ld\n",
					 fp->f_paramcount, fp->f_opcodecount);
				} else {
					math_str("null\t0\n");
				}
			} else {
				if (fp == NULL)
					continue;
				count++;
				math_fmt("%-12s\t%-2d\n", namestr(&funcnames,
					index), fp->f_paramcount);
			}
		}
	}
	if (conf->resource_debug & RSCDBG_FUNC_INFO) {
		math_fmt("\nNumber non-null: %ld\n", count);
		math_fmt("Number null: %ld\n", funccount - count);
		math_fmt("Total number: %ld\n", funccount);
	} else {
		if (count > 0)
			math_fmt("\nNumber: %ld\n", count);
		else
			math_str("No user functions defined\n");
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
	newname = namestr(&funcnames, newindex);
	fp->f_name = newname;
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
	size_t size;			/* size of just created function */
	unsigned long index;

	if (oldop != OP_RETURN) {
		addop(OP_UNDEF);
		addop(OP_RETURN);
	}

	checklabels();

	if (errorcount) {
		scanerror(T_NULL,"Compilation of \"%s\" failed: %ld error(s)",
			 newname, errorcount);
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
	if (newname[0] != '*' && (conf->traceflags & TRACE_FNCODES)) {
		dumpnames = TRUE;
		for (size = 0; size < fp->f_opcodecount; ) {
			printf("%ld: ", (unsigned long)size);
			size += dumpop(&fp->f_opcodes[size]);
		}
	}
	if ((inputisterminal() && conf->resource_debug & RSCDBG_STDIN_FUNC) ||
	    (!inputisterminal() && conf->resource_debug & RSCDBG_FILE_FUNC)) {
		printf("%s(", newname);
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
		warning("No function named \"%s\" to be undefined", name);
		return;
	}
	if (functions[index] == NULL) {
		warning("No defined function \"%s\" to be undefined", name);
		return;
	}
	freenumbers(functions[index]);
	free(functions[index]);
	if ((inputisterminal() && conf->resource_debug & RSCDBG_STDIN_FUNC) ||
	    (!inputisterminal() && conf->resource_debug & RSCDBG_FILE_FUNC))
		printf("%s() undefined\n", name);
	functions[index] = NULL;
}


/*
 * Free memory used to store function and its constants
 */
void
freefunc(FUNC *fp)
{
	long index;
	unsigned long i;

	if (fp == NULL)
		return;
	if (fp == curfunc) {
		index = newindex;
	} else {
		for (index = 0; index < funccount; index++) {
			if (functions[index] == fp)
				break;
		}
		if (index == funccount) {
			math_error("Bad call to freefunc!!!");
			/*NOTREACHED*/
		}
	}
	if (newname[0] != '*' && (conf->traceflags & TRACE_FNCODES)) {
		printf("Freeing function \"%s\"\n",namestr(&funcnames,index));
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
	FUNC *fp;
	long index;

	for (index = 0; index < funccount; index++) {
		fp = functions[index];
		if (fp) {
			freefunc(fp);
			functions[index] = NULL;
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
	if (index >= funccount) {
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
			warning("Constant before comma operator");
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
