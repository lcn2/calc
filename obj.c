/*
 * obj - object handling primitives
 *
 * Copyright (C) 1999-2007,2021  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:19
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */
/*
 * "Object" handling primitives.
 * This simply means that user-specified routines are called to perform
 * the indicated operations.
 */


#include <stdio.h>
#include "calc.h"
#include "opcodes.h"
#include "func.h"
#include "symbol.h"
#include "str.h"
#include "strl.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * Types of values returned by calling object routines.
 */
#define A_VALUE 0	/* returns arbitrary value */
#define A_INT	1	/* returns integer value */
#define A_UNDEF 2	/* returns no value */

/*
 * Error handling actions for when the function is undefined.
 */
#define ERR_NONE	0	/* no special action */
#define ERR_PRINT	1	/* print element */
#define ERR_CMP 2	/* compare two values */
#define ERR_TEST	3	/* test value for nonzero */
#define ERR_POW 4	/* call generic power routine */
#define ERR_ONE 5	/* return number 1 */
#define ERR_INC 6	/* increment by one */
#define ERR_DEC 7	/* decrement by one */
#define ERR_SQUARE 8	/* square value */
#define ERR_VALUE 9	/* return value */
#define ERR_ASSIGN 10	/* assign value */


STATIC struct objectinfo {
	short args;	/* number of arguments */
	short retval;	/* type of return value */
	short error;	/* special action on errors */
	char *name;	/* name of function to call */
	char *comment;	/* useful comment if any */
} objectinfo[] = {
	{1, A_UNDEF, ERR_PRINT,
	 "print",	"print value, default prints elements"},
	{1, A_VALUE, ERR_ONE,
	 "one",	"multiplicative identity, default is 1"},
	{1, A_INT,   ERR_TEST,
	 "test", "logical test (false,true => 0,1), default tests elements"},
	{2, A_VALUE, ERR_NONE,
	 "add",	NULL},
	{2, A_VALUE, ERR_NONE,
	 "sub",	NULL},
	{1, A_VALUE, ERR_NONE,
	 "neg",	"negative"},
	{2, A_VALUE, ERR_NONE,
	 "mul",	NULL},
	{2, A_VALUE, ERR_NONE,
	 "div",	"non-integral division"},
	{1, A_VALUE, ERR_NONE,
	 "inv",	"multiplicative inverse"},
	{2, A_VALUE, ERR_NONE,
	 "abs",	"absolute value within given error"},
	{1, A_VALUE, ERR_NONE,
	 "norm", "square of absolute value"},
	{1, A_VALUE, ERR_NONE,
	 "conj", "conjugate"},
	{2, A_VALUE, ERR_POW,
	 "pow",	"integer power, default does multiply, square, inverse"},
	{1, A_VALUE, ERR_NONE,
	 "sgn",	"sign of value (-1, 0, 1)"},
	{2, A_INT,   ERR_CMP,
	 "cmp",	"equality (equal,nonequal => 0,1), default tests elements"},
	{2, A_VALUE, ERR_NONE,
	 "rel",	"relative order, positive for >, etc."},
	{3, A_VALUE, ERR_NONE,
	 "quo",	"integer quotient"},
	{3, A_VALUE, ERR_NONE,
	 "mod",	"remainder of division"},
	{1, A_VALUE, ERR_NONE,
	 "int",	"integer part"},
	{1, A_VALUE, ERR_NONE,
	 "frac", "fractional part"},
	{1, A_VALUE, ERR_INC,
	 "inc",	"increment, default adds 1"},
	{1, A_VALUE, ERR_DEC,
	 "dec",	"decrement, default subtracts 1"},
	{1, A_VALUE, ERR_SQUARE,
	 "square", "default multiplies by itself"},
	{2, A_VALUE, ERR_NONE,
	 "scale", "multiply by power of 2"},
	{2, A_VALUE, ERR_NONE,
	 "shift", "shift left by n bits (right if negative)"},
	{3, A_VALUE, ERR_NONE,
	 "round", "round to given number of decimal places"},
	{3, A_VALUE, ERR_NONE,
	 "bround", "round to given number of binary places"},
	{3, A_VALUE, ERR_NONE,
	 "root", "root of value within given error"},
	{3, A_VALUE, ERR_NONE,
	 "sqrt", "square root within given error"},
	{2, A_VALUE, ERR_NONE,
	 "or",	"bitwise or"},
	{2, A_VALUE, ERR_NONE,
	 "and",	"bitwise and"},
	{1, A_VALUE, ERR_NONE,
	 "not",	"logical not"},
	{1, A_VALUE, ERR_NONE,
	 "fact", "factorial or postfix !"},
	{1, A_VALUE, ERR_VALUE,
	 "min",	"value for min(...)"},
	{1, A_VALUE, ERR_VALUE,
	 "max",	"value for max(...)"},
	{1, A_VALUE, ERR_VALUE,
	 "sum",	"value for sum(...)"},
	{2, A_UNDEF, ERR_ASSIGN,
	 "assign", "assign, defaults to a = b"},
	{2, A_VALUE, ERR_NONE,
	 "xor",	"value for binary ~"},
	{1, A_VALUE, ERR_NONE,
	 "comp", "value for unary ~"},
	{1, A_VALUE, ERR_NONE,
	 "content", "unary hash op"},
	{2, A_VALUE, ERR_NONE,
	 "hashop",  "binary hash op"},
	{1, A_VALUE, ERR_NONE,
	 "backslash", "unary backslash op"},
	{2, A_VALUE, ERR_NONE,
	 "setminus", "binary backslash op"},
	{1, A_VALUE, ERR_NONE,
	 "plus", "unary + op"},
	{0, 0, 0,
	 NULL, NULL}
};


STATIC STRINGHEAD objectnames;	/* names of objects */
STATIC STRINGHEAD elements;	/* element names for parts of objects */
STATIC OBJECTACTIONS **objects; /* table of actions for objects */

#define OBJALLOC 16
STATIC long maxobjcount = 0;

S_FUNC VALUE objpowi(VALUE *vp, NUMBER *q);
S_FUNC BOOL objtest(OBJECT *op);
S_FUNC BOOL objcmp(OBJECT *op1, OBJECT *op2);
S_FUNC void objprint(OBJECT *op);


/*
 * Show all the routine names available for objects.
 */
void
showobjfuncs(void)
{
	register struct objectinfo *oip;

	printf("\nThe following object routines are definable.\n");
	printf("Note: xx represents the actual object type name.\n\n");
	printf("Name	Args	Comments\n");
	for (oip = objectinfo; oip->name; oip++) {
		printf("xx_%-8s %d	%s\n", oip->name, oip->args,
			oip->comment ? oip->comment : "");
	}
	printf("\n");
}


/*
 * Call the appropriate user-defined routine to handle an object action.
 * Returns the value that the routine returned.
 */
VALUE
objcall(int action, VALUE *v1, VALUE *v2, VALUE *v3)
{
	FUNC *fp;		/* function to call */
	STATIC OBJECTACTIONS *oap; /* object to call for */
	struct objectinfo *oip; /* information about action */
	long index;		/* index of function (negative if undefined) */
	VALUE val;		/* return value */
	VALUE tmp;		/* temp value */
	char name[SYMBOLSIZE+1+1];	/* full name of user routine to call */
	size_t namestr_len;	/* length of the namestr() return string */
	char *namestr_ret;	/* namestr() return string */
	size_t opi_name_len;	/* length of the oip name */

	/* initilaize name */
	memset(name, 0, sizeof(name));

	/* initialize VALUEs */
	val.v_subtype = V_NOSUBTYPE;
	tmp.v_subtype = V_NOSUBTYPE;

	if ((unsigned)action > OBJ_MAXFUNC) {
		math_error("Illegal action for object call");
		/*NOTREACHED*/
	}
	oip = &objectinfo[action];
	if (v1->v_type == V_OBJ) {
		oap = v1->v_obj->o_actions;
	} else if (v2->v_type == V_OBJ) {
		oap = v2->v_obj->o_actions;
	} else {
		math_error("Object routine called with non-object");
		/*NOTREACHED*/
	}
	index = oap->oa_indices[action];
	if (index < 0) {
		namestr_ret = namestr(&objectnames, oap->oa_index);
		if (namestr_ret == NULL) {
			math_error("namestr returned NULL!!!");
			/*NOTREACHED*/
		}
		namestr_len = strlen(namestr_ret);
		opi_name_len = strlen(oip->name);
		if (namestr_len > (size_t)SYMBOLSIZE-1-opi_name_len) {
			math_error("namestr returned a strong too long!!!");
			/*NOTREACHED*/
		}
		name[0] = '\0';
		strlcpy(name, namestr_ret, namestr_len+1);
		strlcat(name, "_", sizeof(name));
		strlcat(name, oip->name, sizeof(name));
		index = adduserfunc(name);
		oap->oa_indices[action] = index;
	}
	fp = NULL;
	if (index >= 0)
		fp = findfunc(index);
	if (fp == NULL) {
		switch (oip->error) {
		case ERR_PRINT:
			objprint(v1->v_obj);
			val.v_type = V_NULL;
			break;
		case ERR_CMP:
			val.v_type = V_INT;
			if (v1->v_type != v2->v_type) {
				val.v_int = 1;
				return val;
			}
			val.v_int = objcmp(v1->v_obj, v2->v_obj);
			break;
		case ERR_TEST:
			val.v_type = V_INT;
			val.v_int = objtest(v1->v_obj);
			break;
		case ERR_POW:
			if (v2->v_type != V_NUM) {
				math_error("Non-real power");
				/*NOTREACHED*/
			}
			val = objpowi(v1, v2->v_num);
			break;
		case ERR_ONE:
			val.v_type = V_NUM;
			val.v_num = qlink(&_qone_);
			break;
		case ERR_INC:
			tmp.v_type = V_NUM;
			tmp.v_num = &_qone_;
			val = objcall(OBJ_ADD, v1, &tmp, NULL_VALUE);
			break;
		case ERR_DEC:
			tmp.v_type = V_NUM;
			tmp.v_num = &_qone_;
			val = objcall(OBJ_SUB, v1, &tmp, NULL_VALUE);
			break;
		case ERR_SQUARE:
			val = objcall(OBJ_MUL, v1, v1, NULL_VALUE);
			break;
		case ERR_VALUE:
			copyvalue(v1, &val);
			break;
		case ERR_ASSIGN:
			copyvalue(v2, &tmp);
			tmp.v_subtype |= v1->v_subtype;
			freevalue(v1);
			*v1 = tmp;
			val.v_type = V_NULL;
			break;
		default:
			math_error("Function \"%s\" is undefined",
				    namefunc(index));
			/*NOTREACHED*/
		}
		return val;
	}
	switch (oip->args) {
	case 0:
		break;
	case 1:
		++stack;
		stack->v_addr = v1;
		stack->v_type = V_ADDR;
		break;
	case 2:
		++stack;
		stack->v_addr = v1;
		stack->v_type = V_ADDR;
		++stack;
		stack->v_addr = v2;
		stack->v_type = V_ADDR;
		break;
	case 3:
		++stack;
		stack->v_addr = v1;
		stack->v_type = V_ADDR;
		++stack;
		stack->v_addr = v2;
		stack->v_type = V_ADDR;
		++stack;
		stack->v_addr = v3;
		stack->v_type = V_ADDR;
		break;
	default:
		math_error("Bad number of args to calculate");
		/*NOTREACHED*/
	}
	calculate(fp, oip->args);
	switch (oip->retval) {
	case A_VALUE:
		return *stack--;
	case A_UNDEF:
		freevalue(stack--);
		val.v_type = V_NULL;
		break;
	case A_INT:
		if ((stack->v_type != V_NUM) || qisfrac(stack->v_num)) {
			math_error("Integer return value required");
			/*NOTREACHED*/
		}
		index = qtoi(stack->v_num);
		qfree(stack->v_num);
		stack--;
		val.v_type = V_INT;
		val.v_int = index;
		break;
	default:
		math_error("Bad object return");
		/*NOTREACHED*/
	}
	return val;
}


/*
 * Print the elements of an object in short and unambiguous format.
 * This is the default routine if the user's is not defined.
 *
 * given:
 *	op		object being printed
 */
S_FUNC void
objprint(OBJECT *op)
{
	int count;		/* number of elements */
	int i;			/* index */

	count = op->o_actions->oa_count;
	math_fmt("obj %s {", namestr(&objectnames, op->o_actions->oa_index));
	for (i = 0; i < count; i++) {
		if (i)
			math_str(", ");
		printvalue(&op->o_table[i], PRINT_SHORT | PRINT_UNAMBIG);
	}
	math_chr('}');
}


/*
 * Test an object for being "nonzero".
 * This is the default routine if the user's is not defined.
 * Returns TRUE if any of the elements are "nonzero".
 */
S_FUNC BOOL
objtest(OBJECT *op)
{
	int i;			/* loop counter */

	i = op->o_actions->oa_count;
	while (--i >= 0) {
		if (testvalue(&op->o_table[i]))
			return TRUE;
	}
	return FALSE;
}


/*
 * Compare two objects for equality, returning TRUE if they differ.
 * This is the default routine if the user's is not defined.
 * For equality, all elements must be equal.
 */
S_FUNC BOOL
objcmp(OBJECT *op1, OBJECT *op2)
{
	int i;			/* loop counter */

	if (op1->o_actions != op2->o_actions)
		return TRUE;
	i = op1->o_actions->oa_count;
	while (--i >= 0) {
		if (comparevalue(&op1->o_table[i], &op2->o_table[i]))
			return TRUE;
	}
	return FALSE;
}


/*
 * Raise an object to an integral power.
 * This is the default routine if the user's is not defined.
 * Negative powers mean the positive power of the inverse.
 * Zero means the multiplicative identity.
 *
 * given:
 *	vp		value to be powered
 *	q		power to raise number to
 */
S_FUNC VALUE
objpowi(VALUE *vp, NUMBER *q)
{
	VALUE res, tmp;
	long power;		/* power to raise to */
	FULL bit;		/* current bit value */

	if (qisfrac(q)) {
		math_error("Raising object to non-integral power");
		/*NOTREACHED*/
	}
	if (zge31b(q->num)) {
		math_error("Raising object to very large power");
		/*NOTREACHED*/
	}
	power = ztolong(q->num);
	if (qisneg(q))
		power = -power;
	/*
	 * Handle some low powers specially
	 */
	if ((power <= 2) && (power >= -2)) {
		switch ((int) power) {
		case 0:
			return objcall(OBJ_ONE, vp, NULL_VALUE, NULL_VALUE);
		case 1:
			res.v_obj = objcopy(vp->v_obj);
			res.v_type = V_OBJ;
			res.v_subtype = V_NOSUBTYPE;
			return res;
		case -1:
			return objcall(OBJ_INV, vp, NULL_VALUE, NULL_VALUE);
		case 2:
			return objcall(OBJ_SQUARE, vp, NULL_VALUE, NULL_VALUE);
		}
	}
	if (power < 0)
		power = -power;
	/*
	 * Compute the power by squaring and multiplying.
	 * This uses the left to right method of power raising.
	 */
	bit = TOPFULL;
	while ((bit & power) == 0)
		bit >>= 1L;
	bit >>= 1L;
	res = objcall(OBJ_SQUARE, vp, NULL_VALUE, NULL_VALUE);
	if (bit & power) {
		tmp = objcall(OBJ_MUL, &res, vp, NULL_VALUE);
		objfree(res.v_obj);
		res = tmp;
	}
	bit >>= 1L;
	while (bit) {
		tmp = objcall(OBJ_SQUARE, &res, NULL_VALUE, NULL_VALUE);
		objfree(res.v_obj);
		res = tmp;
		if (bit & power) {
			tmp = objcall(OBJ_MUL, &res, vp, NULL_VALUE);
			objfree(res.v_obj);
			res = tmp;
		}
		bit >>= 1L;
	}
	if (qisneg(q)) {
		tmp = objcall(OBJ_INV, &res, NULL_VALUE, NULL_VALUE);
		objfree(res.v_obj);
		return tmp;
	}
	return res;
}


/*
 * Define a (possibly) new class of objects.
 * The list of indexes for the element names is also specified here,
 * and the number of elements defined for the object.
 *
 * given:
 *	name		name of object type
 *	indices		table of indices for elements
 *	count		number of elements defined for the object
 */
int
defineobject(char *name, int indices[], int count)
{
	OBJECTACTIONS *oap;	/* object definition structure */
	STRINGHEAD *hp;
	OBJECTACTIONS **newobjects;
	int index;

	hp = &objectnames;
	if (hp->h_list == NULL)
		initstr(hp);
	index = findstr(hp, name);
	if (index >= 0) {
		/*
		 * Object is already defined.  Give an error unless this
		 * new definition is exactly the same as the old one.
		 */
		oap = objects[index];
		if (oap->oa_count == count) {
			for (index = 0; ; index++) {
				if (index >= count)
					return 0;
				if (oap->oa_elements[index] != indices[index])
					break;
			}
		}
		return 1;
	}

	if (hp->h_count >= maxobjcount) {
		if (maxobjcount == 0) {
			newobjects = (OBJECTACTIONS **) malloc(
				OBJALLOC * sizeof(OBJECTACTIONS *));
			maxobjcount = OBJALLOC;
		} else {
			maxobjcount += OBJALLOC;
			newobjects = (OBJECTACTIONS **) realloc(objects,
				maxobjcount * sizeof(OBJECTACTIONS *));
		}
		if (newobjects == NULL) {
			math_error("Allocation failure for new object type");
			/*NOTREACHED*/
		}
		objects = newobjects;
	}

	oap = (OBJECTACTIONS *) malloc(objectactionsize(count));
	if (oap == NULL) {
		math_error("Cannot allocate object type #0");
		/*NOTREACHED*/
	}
	name = addstr(hp, name);
	if (name == NULL) {
		math_error("Cannot allocate object type #1");
		/*NOTREACHED*/
	}
	oap->oa_count = count;
	for (index = OBJ_MAXFUNC; index >= 0; index--)
		oap->oa_indices[index] = -1;
	for (index = 0; index < count; index++)
		oap->oa_elements[index] = indices[index];
	index = findstr(hp, name);
	oap->oa_index = index;
	objects[index] = oap;
	return 0;
}


/*
 * Check an object name to see if it is currently defined.
 * If so, the index for the object type is returned.
 * If the object name is currently unknown, then -1 is returned.
 */
int
checkobject(char *name)
{
	STRINGHEAD *hp;

	hp = &objectnames;
	if (hp->h_list == NULL)
		return -1;
	return findstr(hp, name);
}


/*
 * Define a (possibly) new element name for an object.
 * Returns an index which identifies the element name.
 */
int
addelement(char *name)
{
	STRINGHEAD *hp;
	int index;

	hp = &elements;
	if (hp->h_list == NULL)
		initstr(hp);
	index = findstr(hp, name);
	if (index >= 0)
		return index;
	if (addstr(hp, name) == NULL) {
		math_error("Cannot allocate element name");
		/*NOTREACHED*/
	}
	return findstr(hp, name);
}


/*
 * Return the index which identifies an element name.
 * Returns minus one if the element name is unknown.
 *
 * given:
 *	name		element name
 */
int
findelement(char *name)
{
	if (elements.h_list == NULL)
		return -1;
	return findstr(&elements, name);
}


/*
 * Returns the name of object type with specified index
 */
char *
objtypename(unsigned long index)
{
	return namestr(&objectnames, (long)index);
}


/*
 * Return the value table offset to be used for an object element name.
 * This converts the element index from the element table into an offset
 * into the object value array.	 Returns -1 if the element index is unknown.
 */
int
objoffset(OBJECT *op, long index)
{
	register OBJECTACTIONS *oap;
	int offset;			/* offset into value array */

	oap = op->o_actions;
	for (offset = oap->oa_count - 1; offset >= 0; offset--) {
		if (oap->oa_elements[offset] == index)
			return offset;
	}
	return -1;
}


/*
 * Allocate a new object structure with the specified index.
 */
OBJECT *
objalloc(long index)
{
	OBJECTACTIONS *oap;
	OBJECT *op;
	VALUE *vp;
	int i;

	if (index < 0 || index > maxobjcount) {
		math_error("Allocating bad object index");
		/*NOTREACHED*/
	}
	oap = objects[index];
	if (oap == NULL) {
		math_error("Object type not defined");
		/*NOTREACHED*/
	}
	i = oap->oa_count;
	if (i < USUAL_ELEMENTS)
		i = USUAL_ELEMENTS;
	if (i == USUAL_ELEMENTS)
		op = (OBJECT *) malloc(sizeof(OBJECT));
	else
		op = (OBJECT *) malloc(objectsize(i));
	if (op == NULL) {
		math_error("Cannot allocate object");
		/*NOTREACHED*/
	}
	op->o_actions = oap;
	vp = op->o_table;
	for (i = oap->oa_count; i-- > 0; vp++) {
		vp->v_num = qlink(&_qzero_);
		vp->v_type = V_NUM;
		vp->v_subtype = V_NOSUBTYPE;
	}
	return op;
}


/*
 * Free an object structure.
 */
void
objfree(OBJECT *op)
{
	VALUE *vp;
	int i;

	vp = op->o_table;
	for (i = op->o_actions->oa_count; i-- > 0; vp++) {
		if (vp->v_type == V_NUM) {
			qfree(vp->v_num);
		} else {
			freevalue(vp);
		}
	}
	if (op->o_actions->oa_count <= USUAL_ELEMENTS)
		free(op);
	else
		free((char *) op);
}


/*
 * Copy an object value
 */
OBJECT *
objcopy(OBJECT *op)
{
	VALUE *v1, *v2;
	OBJECT *np;
	int i;

	i = op->o_actions->oa_count;
	if (i < USUAL_ELEMENTS)
		i = USUAL_ELEMENTS;
	if (i == USUAL_ELEMENTS)
		np = (OBJECT *) malloc(sizeof(OBJECT));
	else
		np = (OBJECT *) malloc(objectsize(i));
	if (np == NULL) {
		math_error("Cannot allocate object");
		/*NOTREACHED*/
	}
	np->o_actions = op->o_actions;
	v1 = op->o_table;
	v2 = np->o_table;
	for (i = op->o_actions->oa_count; i-- > 0; v1++, v2++) {
		copyvalue(v1, v2);
	}
	return np;
}


/*
 * Show object types that have been defined.
 */
void
showobjtypes(void)
{
	STRINGHEAD *hp;
	OBJECTACTIONS *oap;
	STRINGHEAD *ep;
	int index, i;

	hp = &objectnames;
	ep = &elements;
	if (hp->h_count == 0) {
		printf("No object types defined\n");
		return;
	}
	for (index = 0; index < hp->h_count; index++) {
		oap = objects[index];
		printf("\t%s\t{", namestr(&objectnames, index));
		for (i = 0; i < oap->oa_count; i++) {
			if (i) printf(",");
			printf("%s", namestr(ep, oap->oa_elements[i]));
		}
		printf("}\n");
	}

}


/* END CODE */
