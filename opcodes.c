/*
 * Copyright (c) 1996 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Opcode execution module
 */

#include "calc.h"
#include "opcodes.h"
#include "func.h"
#include "symbol.h"
#include "hist.h"
#include "args.h"
#include "file.h"
#include "zrand.h"
#include "have_fpos.h"

#define	QUICKLOCALS	20		/* local vars to handle quickly */


VALUE *stack;				/* current location of top of stack */
static VALUE stackarray[MAXSTACK];	/* storage for stack */
static VALUE oldvalue;			/* previous calculation value */
static char *funcname;			/* function being executed */
static long funcline;			/* function line being executed */
int dumpnames;				/* names if TRUE, otherwise indices */


/*
 * forward declarations
 */
static void showsizes(void);
static void o_paramaddr(FUNC *fp, int argcnt, VALUE *args, long index);
static void o_getvalue(void);


/*
 * Types of opcodes (depends on arguments saved after the opcode).
 */
#define OPNUL	1	/* opcode has no arguments */
#define OPONE	2	/* opcode has one integer argument */
#define OPTWO	3	/* opcode has two integer arguments */
#define OPJMP	4	/* opcode is a jump (with one pointer argument) */
#define OPRET	5	/* opcode is a return (with no argument) */
#define OPGLB	6	/* opcode has global symbol pointer argument */
#define OPPAR	7	/* opcode has parameter index argument */
#define OPLOC	8	/* opcode needs local variable pointer (with one arg) */
#define OPSTR	9	/* opcode has a string constant arg */
#define OPARG	10	/* opcode is given number of arguments */
#define	OPSTI	11	/* opcode is static initialization */


/*
 * opcode - info about each opcode
 */
struct opcode {
	void (*o_func)();	/* routine to call for opcode */
	int o_type;		/* type of opcode */
	char *o_name;		/* name of opcode */
};


/*
 * external configuration functions
 */
extern void config_value(CONFIG *cfg, int type, VALUE *ret);
extern void setconfig(int type, VALUE *vp);


/*
 * Initialize the stack.
 */
void
initstack(void)
{
	int i;

	/* on first init, setup the stack array */
	if (stack == NULL) {
		for (i=0; i < sizeof(stackarray)/sizeof(stackarray[0]); ++i) {
			stackarray[i].v_type = V_NULL;
			stackarray[i].v_subtype = V_NOSUBTYPE;
		}
		stack = stackarray;

	/* on subsequent inits, free the old stack */
	} else {
		while (stack > stackarray) {
			freevalue(stack--);
		}
	}
}


/*
 * The various opcodes
 */
static void
o_nop(void)
{
}


static void
o_localaddr(FUNC *fp, VALUE *locals, long index)
{
	if ((unsigned long)index >= fp->f_localcount) {
		math_error("Bad local variable index");
		/*NOTREACHED*/
	}
	locals += index;
	stack++;
	stack->v_addr = locals;
	stack->v_type = V_ADDR;
}


/*ARGSUSED*/
static void
o_globaladdr(FUNC *fp, GLOBAL *sp)
{
	if (sp == NULL) {
		math_error("Global variable \"%s\" not initialized", sp->g_name);
		/*NOTREACHED*/
	}
	stack++;
	stack->v_addr = &sp->g_value;
	stack->v_type = V_ADDR;
}


/*ARGSUSED*/
static void
o_paramaddr(FUNC *fp, int argcount, VALUE *args, long index)
{
	if ((unsigned long)index >= argcount) {
		math_error("Bad parameter index");
		/*NOTREACHED*/
	}
	args += index;
	stack++;
	if (args->v_type == V_ADDR)
		stack->v_addr = args->v_addr;
	else
		stack->v_addr = args;
	stack->v_type = V_ADDR;
}


static void
o_localvalue(FUNC *fp, VALUE *locals, long index)
{
	if ((unsigned long)index >= fp->f_localcount) {
		math_error("Bad local variable index");
		/*NOTREACHED*/
	}
	locals += index;
	copyvalue(locals, ++stack);
}


/*ARGSUSED*/
static void
o_globalvalue(FUNC *fp, GLOBAL *sp)
{
	if (sp == NULL) {
		math_error("Global variable not defined");
		/*NOTREACHED*/
	}
	copyvalue(&sp->g_value, ++stack);
}


/*ARGSUSED*/
static void
o_paramvalue(FUNC *fp, int argcount, VALUE *args, long index)
{
	if ((unsigned long)index >= argcount) {
		math_error("Bad paramaeter index");
		/*NOTREACHED*/
	}
	args += index;
	if (args->v_type == V_ADDR)
		args = args->v_addr;
	copyvalue(args, ++stack);
}


static void
o_argvalue(FUNC *fp, int argcount, VALUE *args)
{
	VALUE *vp;
	long index;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if ((vp->v_type != V_NUM) || qisneg(vp->v_num) ||
		qisfrac(vp->v_num)) {
			math_error("Illegal argument for arg function");
			/*NOTREACHED*/
	}
	if (qiszero(vp->v_num)) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = itoq((long) argcount);
		stack->v_type = V_NUM;
		return;
	}
	index = qtoi(vp->v_num) - 1;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	(void) o_paramaddr(fp, argcount, args, index);
}


/*ARGSUSED*/
static void
o_number(FUNC *fp, long arg)
{
	NUMBER *q;

	q = constvalue(arg);
	if (q == NULL) {
		math_error("Numeric constant value not found");
		/*NOTREACHED*/
	}
	stack++;
	stack->v_num = qlink(q);
	stack->v_type = V_NUM;
}


/*ARGSUSED*/
static void
o_imaginary(FUNC *fp, long arg)
{
	NUMBER *q;
	COMPLEX *c;

	q = constvalue(arg);
	if (q == NULL) {
		math_error("Numeric constant value not found");
		/*NOTREACHED*/
	}
	stack++;
	if (qiszero(q)) {
		stack->v_num = qlink(q);
		stack->v_type = V_NUM;
		return;
	}
	c = comalloc();
	c->real = qlink(&_qzero_);
	c->imag = qlink(q);
	stack->v_com = c;
	stack->v_type = V_COM;
}


/*ARGSUSED*/
static void
o_string(FUNC *fp, char *cp)
{
	stack++;
	stack->v_str = cp;
	stack->v_type = V_STR;
	stack->v_subtype = V_STRLITERAL;
}


static void
o_undef(void)
{
	stack++;
	stack->v_type = V_NULL;
}


/*ARGSUSED*/
static void
o_matcreate(FUNC *fp, long dim)
{
	register MATRIX *mp;	/* matrix being defined */
	NUMBER *num1;		/* first number from stack */
	NUMBER *num2;		/* second number from stack */
	VALUE *v1, *v2;
	long min[MAXDIM];	/* minimum range */
	long max[MAXDIM];	/* maximum range */
	long i;			/* index */
	long tmp;		/* temporary */
	long size;		/* size of matrix */

	if ((dim <= 0) || (dim > MAXDIM)) {
		math_error("Bad dimension %ld for matrix", dim);
		/*NOTREACHED*/
	}
	size = 1;
	for (i = dim - 1; i >= 0; i--) {
		v1 = &stack[-1];
		v2 = &stack[0];
		if (v1->v_type == V_ADDR)
			v1 = v1->v_addr;
		if (v2->v_type == V_ADDR)
			v2 = v2->v_addr;
		if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
			math_error("Non-numeric bounds for matrix");
			/*NOTREACHED*/
		}
		num1 = v1->v_num;
		num2 = v2->v_num;
		if (qisfrac(num1) || qisfrac(num2)) {
			math_error("Non-integral bounds for matrix");
			/*NOTREACHED*/
		}
		if (zge31b(num1->num) || zge31b(num2->num)) {
			math_error("Very large bounds for matrix");
			/*NOTREACHED*/
		}
		min[i] = qtoi(num1);
		max[i] = qtoi(num2);
		if (min[i] > max[i]) {
			tmp = min[i];
			min[i] = max[i];
			max[i] = tmp;
		}
		size *= (max[i] - min[i] + 1);
		if (size > 10000000) {
			math_error("Very large size for matrix");
			/*NOTREACHED*/
		}
		freevalue(stack--);
		freevalue(stack--);
	}
	mp = matalloc(size);
	mp->m_dim = dim;
	for (i = 0; i < dim; i++) {
		mp->m_min[i] = min[i];
		mp->m_max[i] = max[i];
	}
	stack++;
	stack->v_type = V_MAT;
	stack->v_mat = mp;
}


/*ARGSUSED*/
static void
o_eleminit(FUNC *fp, long index)
{
	VALUE *vp;
	static VALUE *oldvp;
	MATRIX *mp;
	OBJECT *op;
	VALUE tmp;

	vp = &stack[-1];
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	switch (vp->v_type) {
		case V_MAT:
			mp = vp->v_mat;
			if ((index < 0) || (index >= mp->m_size)) {
				math_error("Too many initializer values");
				/*NOTREACHED*/
			}
			oldvp = &mp->m_table[index];
			break;
		case V_OBJ:
			op = vp->v_obj;
			if ((index < 0) || (index >= op->o_actions->count)) {
				math_error("Too many initializer values");
				/*NOTREACHED*/
			}
			oldvp = &op->o_table[index];
			break;
		default:
			math_error("Attempt to initialize non matrix or object");
			/*NOTREACHED*/
	}
	vp = stack--;
	if (vp->v_type == V_ADDR) {
		vp = vp->v_addr;
		if (vp == oldvp)
			return;
		copyvalue(vp, &tmp);
	}
	else
		tmp = *vp;
	freevalue(oldvp);
	*oldvp = tmp;
}


/*
 * o_indexaddr 
 *
 * given:
 *	fp		function to calculate
 *	dim		dimension of matrix
 *	writeflag	nonzero if element will be written
 */
/*ARGSUSED*/
static void
o_indexaddr(FUNC *fp, long dim, long writeflag)
{
	int i;
	BOOL flag;
	VALUE *val;
	VALUE *vp;
	VALUE indices[MAXDIM];	/* index values */

	flag = (writeflag != 0);
	if (dim <= 0)  {
		math_error("Zero or negative dimensions for indexing");
		/*NOTREACHED*/
	}
	val = &stack[-dim];
	if (val->v_type != V_ADDR) {
		math_error("Non-pointer for indexaddr");
		/*NOTREACHED*/
	}
	val = val->v_addr;
	vp = &stack[-dim + 1];
	for (i = 0; i < dim; i++) {
		if (vp->v_type == V_ADDR)
			indices[i] = vp->v_addr[0];
		else
			indices[i] = vp[0];
		vp++;
	}
	switch (val->v_type) {
		case V_MAT:
			vp = matindex(val->v_mat, flag, dim, indices);
			break;
		case V_ASSOC:
			vp = associndex(val->v_assoc, flag, dim, indices);
			break;
		default:
			math_error("Illegal value for indexing");
			/*NOTREACHED*/
	}
	while (dim-- > 0)
		freevalue(stack--);
	stack->v_type = V_ADDR;
	stack->v_addr = vp;
}


/*ARGSUSED*/
static void
o_elemaddr(FUNC *fp, long index)
{
	VALUE *vp;
	MATRIX *mp;
	OBJECT *op;
	int offset;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = stack->v_addr;
	switch (vp->v_type) {
		case V_MAT:
			mp = vp->v_mat;
			if ((index < 0) || (index >= mp->m_size)) {
				math_error("Non-existent element for matrix");
				/*NOTREACHED*/
			}
			stack->v_type = V_ADDR;
			stack->v_addr = &mp->m_table[index];
			return;
		case V_OBJ:
			op = vp->v_obj;
			offset = objoffset(op, index);
			if (offset < 0) {
				math_error("Non-existent element for object");
				/*NOTREACHED*/
			}
			stack->v_type = V_ADDR;
			stack->v_addr = &op->o_table[offset];
			return;
		default:
			math_error("Not indexing matrix or object");
			/*NOTREACHED*/
	}

}


static void
o_elemvalue(FUNC *fp, long index)
{
	o_elemaddr(fp, index);
	copyvalue(stack->v_addr, stack);
}


/*ARGSUSED*/
static void
o_objcreate(FUNC *fp, long arg)
{
	stack++;
	stack->v_type = V_OBJ;
	stack->v_obj = objalloc(arg);
}


static void
o_assign(void)
{
	VALUE *var;		/* variable value */
	VALUE *vp;
	VALUE tmp;

	var = &stack[-1];
	if (var->v_type != V_ADDR) {
		math_error("Assignment into non-variable");
		/*NOTREACHED*/
	}
	var = var->v_addr;
	vp = stack--;
	if (vp->v_type == V_ADDR) {
		vp = vp->v_addr;
		if (vp == var)
			return;
		copyvalue(vp, &tmp);
	}
	else
		tmp = *vp;
	freevalue(var);
	*var = tmp;
}


static void
o_assignpop(void)
{
	VALUE *var;		/* variable value */
	VALUE *vp;
	VALUE tmp;

	var = &stack[-1];
	if (var->v_type != V_ADDR) {
		math_error("Assignment into non-variable");
		/*NOTREACHED*/
	}
	var = var->v_addr;
	vp = &stack[0];
	stack -= 2;
	if (vp->v_type == V_ADDR) {
		vp = vp->v_addr;
		if (vp == var)
			return;
		copyvalue(vp, &tmp);
	}
	else
		tmp = *vp;
	freevalue(var);
	*var = tmp;
}


static void
o_swap(void)
{
	VALUE *v1, *v2;		/* variables to be swapped */
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if ((v1->v_type != V_ADDR) || (v2->v_type != V_ADDR)) {
		math_error("Swapping non-variables");
		/*NOTREACHED*/
	}
	tmp = v1->v_addr[0];
	v1->v_addr[0] = v2->v_addr[0];
	v2->v_addr[0] = tmp;
	stack--;
	stack->v_type = V_NULL;
}


static void
o_add(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	addvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_sub(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	subvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_mul(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	mulvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_power(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	powivalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_div(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	divvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_quo(void)
{
	VALUE *v1, *v2;
	VALUE tmp, null;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	null.v_type = V_NULL;
	quovalue(v1, v2, &null, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_mod(void)
{
	VALUE *v1, *v2;
	VALUE tmp, null;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	null.v_type = V_NULL;
	modvalue(v1, v2, &null, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_quomod(void)
{
	VALUE *v1, *v2, *v3, *v4;
	VALUE valquo, valmod;
	BOOL res;

	v1 = &stack[-3];
	v2 = &stack[-2];
	v3 = &stack[-1];
	v4 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v3->v_type != V_ADDR) || (v4->v_type != V_ADDR)) {
		math_error("Non-variable for quomod");
		/*NOTREACHED*/
	}
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		math_error("Non-reals for quomod");
		/*NOTREACHED*/
	}
	v3 = v3->v_addr;
	v4 = v4->v_addr;
	valquo.v_type = V_NUM;
	valmod.v_type = V_NUM;
	res = qquomod(v1->v_num, v2->v_num, &valquo.v_num, &valmod.v_num);
	stack -= 2;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = (res ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
	freevalue(v3);
	freevalue(v4);
	*v3 = valquo;
	*v4 = valmod;
}


static void
o_and(void)
{
	VALUE *v1, *v2;
	NUMBER *q;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		math_error("Non-numerics for and");
		/*NOTREACHED*/
	}
	q = qand(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
}


static void
o_or(void)
{
	VALUE *v1, *v2;
	NUMBER *q;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM)) {
		math_error("Non-numerics for or");
		/*NOTREACHED*/
	}
	q = qor(v1->v_num, v2->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
}


static void
o_not(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = testvalue(vp);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qzero_) : qlink(&_qone_));
	stack->v_type = V_NUM;
}


static void
o_negate(void)
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qneg(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return;
	}
	negvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
}


static void
o_invert(void)
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qinv(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return;
	}
	invertvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
}


static void
o_scale(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[0];
	v2 = &stack[-1];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	scalevalue(v2, v1, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_int(void)
{
	VALUE *vp;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	intvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
}


static void
o_frac(void)
{
	VALUE *vp;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	fracvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
}


static void
o_abs(void)
{
	VALUE *v1, *v2;
	NUMBER *q;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_NUM) || (v2->v_type != V_NUM) ||
		!qispos(v2->v_num))
	{
		absvalue(v1, v2, &tmp);
		freevalue(stack--);
		freevalue(stack);
		*stack = tmp;
		return;
	}
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack--;
	if ((stack->v_type == V_NUM) && !qisneg(v1->v_num))
		return;
	q = qabs(v1->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
}


static void
o_norm(void)
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qsquare(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return;
	}
	normvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
}


static void
o_square(void)
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qsquare(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return;
	}
	squarevalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
}


static void
o_istype(void)
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v1->v_type != V_OBJ) || (v2->v_type != V_OBJ))
		r = (v1->v_type == v2->v_type);
	else
		r = (v1->v_obj->o_actions == v2->v_obj->o_actions);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) r);
	stack->v_type = V_NUM;
}


static void
o_isint(void)
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = stack->v_addr;
	if (vp->v_type != V_NUM) {
		freevalue(stack);
		stack->v_num = qlink(&_qzero_);
		stack->v_type = V_NUM;
		return;
	}
	if (qisint(vp->v_num))
		q = qlink(&_qone_);
	else
		q = qlink(&_qzero_);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
}


static void
o_isnum(void)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	switch (vp->v_type) {
		case V_NUM:
			if (stack->v_type == V_NUM)
				qfree(stack->v_num);
			break;
		case V_COM:
			if (stack->v_type == V_COM)
				comfree(stack->v_com);
			break;
		default:
			freevalue(stack);
			stack->v_num = qlink(&_qzero_);
			stack->v_type = V_NUM;
			return;
	}
	stack->v_num = qlink(&_qone_);
	stack->v_type = V_NUM;
}


static void
o_ismat(void)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_MAT) {
		freevalue(stack);
		stack->v_num = qlink(&_qzero_);
		stack->v_type = V_NUM;
		return;
	}
	freevalue(stack);
	stack->v_type = V_NUM;
	stack->v_num = qlink(&_qone_);
}


static void
o_islist(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_LIST);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_isobj(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_OBJ);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_isstr(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_STR);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_isfile(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_FILE);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_isrand(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_RAND);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_israndom(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_RANDOM);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_isconfig(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_CONFIG);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_ishash(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_HASH);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_isassoc(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = (vp->v_type == V_ASSOC);
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_issimple(void)
{
	VALUE *vp;
	int r;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	r = 0;
	switch (vp->v_type) {
		case V_NULL:
		case V_NUM:
		case V_COM:
		case V_STR:
			r = 1;
	}
	freevalue(stack);
	stack->v_num = (r ? qlink(&_qone_) : qlink(&_qzero_));
	stack->v_type = V_NUM;
}


static void
o_isodd(void)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if ((vp->v_type == V_NUM) && qisodd(vp->v_num)) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = qlink(&_qone_);
		stack->v_type = V_NUM;
		return;
	}
	freevalue(stack);
	stack->v_num = qlink(&_qzero_);
	stack->v_type = V_NUM;
}


static void
o_iseven(void)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if ((vp->v_type == V_NUM) && qiseven(vp->v_num)) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = qlink(&_qone_);
		stack->v_type = V_NUM;
		return;
	}
	freevalue(stack);
	stack->v_num = qlink(&_qzero_);
	stack->v_type = V_NUM;
}


static void
o_isreal(void)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = qlink(&_qone_);
		stack->v_type = V_NUM;
		return;
	}
	freevalue(stack);
	stack->v_num = qlink(&_qzero_);
	stack->v_type = V_NUM;
}


static void
o_isnull(void)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NULL) {
		freevalue(stack);
		stack->v_num = qlink(&_qzero_);
		stack->v_type = V_NUM;
		return;
	}
	freevalue(stack);
	stack->v_num = qlink(&_qone_);
	stack->v_type = V_NUM;
}


static void
o_re(void)
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (stack->v_type == V_ADDR) {
			stack->v_num = qlink(vp->v_num);
			stack->v_type = V_NUM;
		}
		return;
	}
	if (vp->v_type != V_COM) {
		math_error("Taking real part of non-number");
		/*NOTREACHED*/
	}
	q = qlink(vp->v_com->real);
	if (stack->v_type == V_COM)
		comfree(stack->v_com);
	stack->v_num = q;
	stack->v_type = V_NUM;
}


static void
o_im(void)
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack->v_num = qlink(&_qzero_);
		stack->v_type = V_NUM;
		return;
	}
	if (vp->v_type != V_COM) {
		math_error("Taking imaginary part of non-number");
		/*NOTREACHED*/
	}
	q = qlink(vp->v_com->imag);
	if (stack->v_type == V_COM)
		comfree(stack->v_com);
	stack->v_num = q;
	stack->v_type = V_NUM;
}


static void
o_conjugate(void)
{
	VALUE *vp;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (stack->v_type == V_ADDR) {
			stack->v_num = qlink(vp->v_num);
			stack->v_type = V_NUM;
		}
		return;
	}
	conjvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
}


static void
o_fiaddr(void)
{
	register MATRIX *m;	/* current matrix element */
	NUMBER *q;		/* index value */
	LIST *lp;		/* list header */
	ASSOC *ap;		/* association header */
	VALUE *vp;		/* stack value */
	long index;		/* index value as an integer */

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NUM) {
		math_error("Fast indexing by non-number");
		/*NOTREACHED*/
	}
	q = vp->v_num;
	if (qisfrac(q)) {
		math_error("Fast indexing by non-integer");
		/*NOTREACHED*/
	}
	index = qtoi(q);
	if (zge31b(q->num) || (index < 0)) {
		math_error("Index out of range for fast indexing");
		/*NOTREACHED*/
	}
	if (stack->v_type == V_NUM)
		qfree(q);
	stack--;
	vp = stack;
	if (vp->v_type != V_ADDR) {
		math_error("Bad value for fast indexing");
		/*NOTREACHED*/
	}
	switch (vp->v_addr->v_type) {
		case V_OBJ:
			if (index >= vp->v_addr->v_obj->o_actions->count) {
				math_error("Index out of bounds for object");
				/*NOTREACHED*/
			}
			vp->v_addr = vp->v_addr->v_obj->o_table + index;
			break;
		case V_MAT:
			m = vp->v_addr->v_mat;
			if (index >= m->m_size) {
				math_error("Index out of bounds for matrix");
				/*NOTREACHED*/
			}
			vp->v_addr = m->m_table + index;
			break;
		case V_LIST:
			lp = vp->v_addr->v_list;
			vp->v_addr = listfindex(lp, index);
			if (vp->v_addr == NULL) {
				math_error("Index out of bounds for list");
				/*NOTREACHED*/
			}
			break;
		case V_ASSOC:
			ap = vp->v_addr->v_assoc;
			vp->v_addr = assocfindex(ap, index);
			if (vp->v_addr == NULL) {
				math_error("Index out of bounds for association");
				/*NOTREACHED*/
			}
			break;
		default:
			math_error("Bad variable type for fast indexing");
			/*NOTREACHED*/
	}
}


static void
o_fivalue(void)
{
	(void) o_fiaddr();
	(void) o_getvalue();
}


static void
o_sgn(void)
{
	VALUE *vp;
	NUMBER *q;
	VALUE tmp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		q = qsign(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(vp->v_num);
		stack->v_num = q;
		stack->v_type = V_NUM;
		return;
	}
	sgnvalue(vp, &tmp);
	freevalue(stack);
	*stack = tmp;
}


static void
o_numerator(void)
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NUM) {
		math_error("Numerator of non-number");
		/*NOTREACHED*/
	}
	if ((stack->v_type == V_NUM) && qisint(vp->v_num))
		return;
	q = qnum(vp->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
}


static void
o_denominator(void)
{
	VALUE *vp;
	NUMBER *q;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NUM) {
		math_error("Denominator of non-number");
		/*NOTREACHED*/
	}
	q = qden(vp->v_num);
	if (stack->v_type == V_NUM)
		qfree(stack->v_num);
	stack->v_num = q;
	stack->v_type = V_NUM;
}


static void
o_duplicate(void)
{
	VALUE *vp;

	vp = stack++;
	*stack = *vp;
}


static void
o_dupvalue(void)
{
	if (stack->v_type == V_ADDR)
		copyvalue(stack->v_addr, stack + 1);
	else
		copyvalue(stack, stack + 1);
	stack++;
}


static void
o_pop(void)
{
	freevalue(stack--);
}


static void
o_return(void)
{
}


/*ARGSUSED*/
static void
o_jumpeq(FUNC *fp, BOOL *dojump)
{
	VALUE *vp;
	int i;			/* result of comparison */

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		i = !qiszero(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
	} else {
		i = testvalue(vp);
		freevalue(stack);
	}
	stack--;
	if (!i)
		*dojump = TRUE;
}


/*ARGSUSED*/
static void
o_jumpne(FUNC *fp, BOOL *dojump)
{
	VALUE *vp;
	int i;			/* result of comparison */

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		i = !qiszero(vp->v_num);
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
	} else {
		i = testvalue(vp);
		freevalue(stack);
	}
	stack--;
	if (i)
		*dojump = TRUE;
}


/*ARGSUSED*/
static void
o_condorjump(FUNC *fp, BOOL *dojump)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (!qiszero(vp->v_num)) {
			*dojump = TRUE;
			return;
		}
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack--;
		return;
	}
	if (testvalue(vp))
		*dojump = TRUE;
	else
		freevalue(stack--);
}


/*ARGSUSED*/
static void
o_condandjump(FUNC *fp, BOOL *dojump)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type == V_NUM) {
		if (qiszero(vp->v_num)) {
			*dojump = TRUE;
			return;
		}
		if (stack->v_type == V_NUM)
			qfree(stack->v_num);
		stack--;
		return;
	}
	if (!testvalue(vp))
		*dojump = TRUE;
	else
		freevalue(stack--);
}


/*
 * Compare the top two values on the stack for equality and jump if they are
 * different, popping off the top element, leaving the first one on the stack.
 * If they are equal, pop both values and do not jump.
 */
/*ARGSUSED*/
static void
o_casejump(FUNC *fp, BOOL *dojump)
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = comparevalue(v1, v2);
	freevalue(stack--);
	if (r)
		*dojump = TRUE;
	else
		freevalue(stack--);
}


/*ARGSUSED*/
static void
o_jump(FUNC *fp, BOOL *dojump)
{
	*dojump = TRUE;
}


static void
o_usercall(FUNC *fp, long index, long argcount)
{
	fp = findfunc(index);
	if (fp == NULL) {
		math_error("Function \"%s\" is undefined", namefunc(index));
		/*NOTREACHED*/
	}
	calculate(fp, (int) argcount);
}


/*ARGSUSED*/
static void
o_call(FUNC *fp, long index, long argcount)
{
	VALUE result;

	result = builtinfunc(index, (int) argcount, stack);
	while (--argcount >= 0)
		freevalue(stack--);
	stack++;
	*stack = result;
}


static void
o_getvalue(void)
{
	if (stack->v_type == V_ADDR)
		copyvalue(stack->v_addr, stack);
}


static void
o_cmp(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	relvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_eq(void)
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = comparevalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) (r == 0));
	stack->v_type = V_NUM;
}


static void
o_ne(void)
{
	VALUE *v1, *v2;
	int r;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	r = comparevalue(v1, v2);
	freevalue(stack--);
	freevalue(stack);
	stack->v_num = itoq((long) (r != 0));
	stack->v_type = V_NUM;
}


static void
o_le(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	relvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	if (tmp.v_type == V_NUM && !qispos(tmp.v_num))
		stack->v_num = qlink(&_qone_);
	else
		stack->v_num = qlink(&_qzero_);
	freevalue(&tmp);
	stack->v_type = V_NUM;
}


static void
o_ge(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	relvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	if (tmp.v_type == V_NUM && !qisneg(tmp.v_num))
		stack->v_num = qlink(&_qone_);
	else
		stack->v_num = qlink(&_qzero_);
	freevalue(&tmp);
	stack->v_type = V_NUM;
}


static void
o_lt(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	relvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	if (tmp.v_type == V_NUM && qisneg(tmp.v_num))
		stack->v_num = qlink(&_qone_);
	else
		stack->v_num = qlink(&_qzero_);
	freevalue(&tmp);
	stack->v_type = V_NUM;
}


static void
o_gt(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	relvalue(v1, v2, &tmp);
	freevalue(stack--);
	freevalue(stack);
	if (tmp.v_type == V_NUM && qispos(tmp.v_num))
		stack->v_num = qlink(&_qone_);
	else
		stack->v_num = qlink(&_qzero_);
	freevalue(&tmp);
	stack->v_type = V_NUM;
}


static void
o_preinc(void)
{
	NUMBER *q, **np;
	VALUE *vp, tmp;

	if (stack->v_type != V_ADDR) {
		math_error("Preincrementing non-variable");
		/*NOTREACHED*/
	}
	if (stack->v_addr->v_type == V_NUM) {
		np = &stack->v_addr->v_num;
		q = qinc(*np);
		qfree(*np);
		*np = q;
		return;
	}
	vp = stack->v_addr;
	incvalue(vp, &tmp);
	freevalue(vp);
	*vp = tmp;
}


static void
o_predec(void)
{
	NUMBER *q, **np;
	VALUE *vp, tmp;

	if (stack->v_type != V_ADDR) {
		math_error("Predecrementing non-variable");
		/*NOTREACHED*/
	}
	if (stack->v_addr->v_type == V_NUM) {
		np = &stack->v_addr->v_num;
		q = qdec(*np);
		qfree(*np);
		*np = q;
		return;
	}
	vp = stack->v_addr;
	decvalue(vp, &tmp);
	freevalue(vp);
	*vp = tmp;
}


static void
o_postinc(void)
{
	VALUE *vp;
	VALUE tmp;

	if (stack->v_type != V_ADDR) {
		math_error("Postincrementing non-variable");
		/*NOTREACHED*/
	}
	vp = stack->v_addr;
	copyvalue(vp, stack++);
	incvalue(vp, &tmp);
	freevalue(vp);
	*vp = tmp;
	stack->v_type = V_ADDR;
	stack->v_addr = vp;
}


static void
o_postdec(void)
{
	VALUE *vp;
	VALUE tmp;

	if (stack->v_type != V_ADDR) {
		math_error("Postdecrementing non-variable");
		/*NOTREACHED*/
	}
	vp = stack->v_addr;
	copyvalue(vp, stack++);
	decvalue(vp, &tmp);
	freevalue(vp);
	*vp = tmp;
	stack->v_type = V_ADDR;
	stack->v_addr = vp;
}


static void
o_leftshift(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	shiftvalue(v1, v2, FALSE, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_rightshift(void)
{
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	shiftvalue(v1, v2, TRUE, &tmp);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


/*ARGSUSED*/
static void
o_debug(FUNC *fp, long line)
{
	funcline = line;
	if (abortlevel >= ABORT_STATEMENT) {
		math_error("Calculation aborted at statement boundary");
		/*NOTREACHED*/
	}
}


static void
o_printresult(void)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NULL) {
		if (conf->tab_ok)
		    math_chr('\t');
		printvalue(vp, PRINT_UNAMBIG);
		math_chr('\n');
		math_flush();
	}
	freevalue(stack--);
}


/*ARGSUSED*/
static void
o_print(FUNC *fp, long flags)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	printvalue(vp, (int) flags);
	freevalue(stack--);
	if (conf->traceflags & TRACE_OPCODES)
		printf("\n");
	math_flush();
}


static void
o_printeol(void)
{
	math_chr('\n');
	math_flush();
}


static void
o_printspace(void)
{
	math_chr(' ');
	if (conf->traceflags & TRACE_OPCODES)
		printf("\n");
}


/*ARGSUSED*/
static void
o_printstring(FUNC *fp, char *cp)
{
	math_str(cp);
	if (conf->traceflags & TRACE_OPCODES)
		printf("\n");
	math_flush();
}


static void
o_zero(void)
{
	stack++;
	stack->v_type = V_NUM;
	stack->v_num = qlink(&_qzero_);
}


static void
o_one(void)
{
	stack++;
	stack->v_type = V_NUM;
	stack->v_num = qlink(&_qone_);
}


static void
o_save(FUNC *fp)
{
	VALUE *vp;

	vp = stack;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	freevalue(&fp->f_savedvalue);
	copyvalue(vp, &fp->f_savedvalue);
}


static void
o_oldvalue(void)
{
	copyvalue(&oldvalue, ++stack);
}


static void
o_quit(FUNC *fp, char *cp)
{
	if ((fp->f_name[0] == '*') && (fp->f_name[1] == '\0')) {
		if (cp)
			printf("%s\n", cp);
		hist_term();
		while (stack > stackarray) {
			freevalue(stack--);
		}
		freevalue(stackarray);
		exit(0);
	}
	if (cp) {
		math_error("%s", cp);
		/*NOTREACHED*/
	}
	math_error("quit statement executed");
	/*NOTREACHED*/
}


static void
o_getepsilon(void)
{
	stack++;
	stack->v_type = V_NUM;
	stack->v_num = qlink(conf->epsilon);
}


static void
o_setepsilon(void)
{
	VALUE *vp;
	NUMBER *newep;

	vp = &stack[0];
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_NUM) {
		math_error("Non-numeric for epsilon");
		/*NOTREACHED*/
	}
	newep = vp->v_num;
	stack->v_num = qlink(conf->epsilon);
	setepsilon(newep);
	if (stack->v_type == V_NUM)
		qfree(newep);
	stack->v_type = V_NUM;
}


static void
o_setconfig(void)
{
	int type;
	VALUE *v1, *v2;
	VALUE tmp;

	v1 = &stack[-1];
	v2 = &stack[0];
	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if (v1->v_type != V_STR) {
		math_error("Non-string for config");
		/*NOTREACHED*/
	}
	type = configtype(v1->v_str);
	if (type < 0) {
		math_error("Unknown config name \"%s\"", v1->v_str);
		/*NOTREACHED*/
	}
	config_value(conf, type, &tmp);
	setconfig(type, v2);
	freevalue(stack--);
	freevalue(stack);
	*stack = tmp;
}


static void
o_getconfig(void)
{
	int type;
	VALUE *vp;

	vp = &stack[0];
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_STR) {
		math_error("Non-string for config");
		/*NOTREACHED*/
	}
	type = configtype(vp->v_str);
	if (type < 0) {
		math_error("Unknown config name \"%s\"", vp->v_str);
		/*NOTREACHED*/
	}
	freevalue(stack);
	config_value(conf, type, stack);
}


/*
 * Set the 'old' value to the last value saved during the calculation.
 */
void
updateoldvalue(FUNC *fp)
{
	if (fp->f_savedvalue.v_type == V_NULL)
		return;
	freevalue(&oldvalue);
	oldvalue = fp->f_savedvalue;
	fp->f_savedvalue.v_type = V_NULL;
}


/*
 * Routine called on any runtime error, to complain about it (with possible
 * arguments), and then longjump back to the top level command scanner.
 */
void
math_error(char *fmt, ...)
{
	va_list ap;
	char buf[MAXERROR+1];

	if (funcname && (*funcname != '*'))
		fprintf(stderr, "\"%s\": ", funcname);
	if (funcline && ((funcname && (*funcname != '*')) || !inputisterminal()))
		fprintf(stderr, "line %ld: ", funcline);
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s\n", buf);
	funcname = NULL;
	if (post_init) {
		longjmp(jmpbuf, 1);
	} else {
		fprintf(stderr, "no jmpbuf jumpback point - ABORTING!!!\n");
		exit(3);
	}
}


/*
 * error_value - return error as a value
 */
VALUE
error_value(int e)
{
	VALUE res;

	if (-e > 0)
		e = 0;
	res.v_type = -e;
	return res;
}


/*
 * Fill a newly created matrix at v1 with copies of value at v2.
 */

static void
o_initfill(void)
{
	VALUE *v1, *v2;
	int s;
	VALUE *vp;

	v1 = &stack[-1];
	v2 = &stack[0];

	if (v1->v_type == V_ADDR)
		v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if (v1->v_type != V_MAT) {
		math_error("Non-matrix argument for o_initfill");
		/*NOTREACHED*/
	}
	s = v1->v_mat->m_size;
	vp = v1->v_mat->m_table;
	while (s-- > 0)
		copyvalue(v2, vp++);
	freevalue(stack--);
}
		

/*ARGSUSED*/
static void
o_show(FUNC *fp, long arg)
{
	int size;

	switch((int) arg) {
		case 1: showbuiltins(); return;
		case 2: showglobals(); return;
		case 3: showfunctions(); return;
		case 4: showobjfuncs(); return;
		case 5: config_print(conf); putchar('\n'); return;
		case 6: showobjtypes(); return;
		case 7: showfiles(); return;
		case 8: showsizes(); return;
	}
	fp = findfunc(arg - 9);
	if (fp == NULL) {
		printf("Function not defined\n");
		return;
	}
	dumpnames = FALSE;
	for (size = 0; size < fp->f_opcodecount; ) {
		printf("%ld: ", (long)size);
		size += dumpop(&fp->f_opcodes[size]);
	}
}


static void
showsizes(void)
{
	printf("\tchar\t\t%4ld\n", (long)sizeof(char));
	printf("\tshort\t\t%4ld\n", (long)sizeof(short));
	printf("\tint\t\t%4ld\n", (long)sizeof(int));
	printf("\tlong\t\t%4ld\n", (long)sizeof(long));
	printf("\tpointer\t\t%4ld\n", (long)sizeof(void *));
	printf("\tFILEPOS\t\t%4ld\n", (long)sizeof(FILEPOS));
	printf("\toff_t\t\t%4ld\n", (long)sizeof(off_t));
	printf("\tHALF\t\t%4ld\n", (long)sizeof(HALF));
	printf("\tFULL\t\t%4ld\n", (long)sizeof(FULL));
	printf("\tVALUE\t\t%4ld\n", (long)sizeof(VALUE));
	printf("\tNUMBER\t\t%4ld\n", (long)sizeof(NUMBER));
	printf("\tZVALUE\t\t%4ld\n", (long)sizeof(ZVALUE));
	printf("\tCOMPLEX\t\t%4ld\n", (long)sizeof(COMPLEX));
	printf("\tMATRIX\t\t%4ld\n", (long)sizeof(MATRIX));
	printf("\tLIST\t\t%4ld\n", (long)sizeof(LIST));
	printf("\tLISTELEM\t%4ld\n", (long)sizeof(LISTELEM));
	printf("\tOBJECT\t\t%4ld\n", (long)sizeof(OBJECT));
	printf("\tOBJECTACTIONS\t%4ld\n", (long)sizeof(OBJECTACTIONS));
	printf("\tASSOC\t\t%4ld\n", (long)sizeof(ASSOC));
	printf("\tASSOCELEM\t%4ld\n", (long)sizeof(ASSOCELEM));
	printf("\tCONFIG\t\t%4ld\n", (long)sizeof(CONFIG));
	printf("\tFILEIO\t\t%4ld\n", (long)sizeof(FILEIO));
	printf("\tRAND\t\t%4ld\n", (long)sizeof(RAND));
	printf("\tRANDOM\t\t%4ld\n", (long)sizeof(RANDOM));
}


/*
 * Information about each opcode.
 */
static struct opcode opcodes[MAX_OPCODE+1] = {
	{o_nop,		OPNUL,  "NOP"},		/* no operation */
	{o_localaddr,	OPLOC,  "LOCALADDR"},	/* address of local variable */
	{o_globaladdr,	OPGLB,  "GLOBALADDR"},	/* address of global variable */
	{o_paramaddr,	OPPAR,  "PARAMADDR"},	/* address of paramater variable */
	{o_localvalue,	OPLOC,  "LOCALVALUE"},	/* value of local variable */
	{o_globalvalue,	OPGLB,  "GLOBALVALUE"},	/* value of global variable */
	{o_paramvalue,	OPPAR,  "PARAMVALUE"}, 	/* value of paramater variable */
	{o_number,	OPONE,  "NUMBER"},	/* constant real numeric value */
	{o_indexaddr,	OPTWO,  "INDEXADDR"},	/* array index address */
	{o_printresult,	OPNUL,  "PRINTRESULT"},	/* print result of top-level expression */
	{o_assign,	OPNUL,  "ASSIGN"},	/* assign value to variable */
	{o_add,		OPNUL,  "ADD"},		/* add top two values */
	{o_sub,		OPNUL,  "SUB"},		/* subtract top two values */
	{o_mul,		OPNUL,  "MUL"},		/* multiply top two values */
	{o_div,		OPNUL,  "DIV"},		/* divide top two values */
	{o_mod,		OPNUL,  "MOD"},		/* take mod of top two values */
	{o_save,	OPNUL,  "SAVE"},	/* save value for later use */
	{o_negate,	OPNUL,  "NEGATE"},	/* negate top value */
	{o_invert,	OPNUL,  "INVERT"},	/* invert top value */
	{o_int,		OPNUL,  "INT"},		/* take integer part */
	{o_frac,	OPNUL,  "FRAC"},	/* take fraction part */
	{o_numerator,	OPNUL,  "NUMERATOR"},	/* take numerator */
	{o_denominator,	OPNUL,  "DENOMINATOR"},	/* take denominator */
	{o_duplicate,	OPNUL,  "DUPLICATE"},	/* duplicate top value */
	{o_pop,		OPNUL,  "POP"},		/* pop top value */
	{o_return,	OPRET,  "RETURN"},	/* return value of function */
	{o_jumpeq,	OPJMP,  "JUMPEQ"},	/* jump if value zero */
	{o_jumpne,	OPJMP,  "JUMPNE"},	/* jump if value nonzero */
	{o_jump,	OPJMP,  "JUMP"},	/* jump unconditionally */
	{o_usercall,	OPTWO,  "USERCALL"},	/* call a user function */
	{o_getvalue,	OPNUL,  "GETVALUE"},	/* convert address to value */
	{o_eq,		OPNUL,  "EQ"},		/* test elements for equality */
	{o_ne,		OPNUL,  "NE"},		/* test elements for inequality */
	{o_le,		OPNUL,  "LE"},		/* test elements for <= */
	{o_ge,		OPNUL,  "GE"},		/* test elements for >= */
	{o_lt,		OPNUL,  "LT"},		/* test elements for < */
	{o_gt,		OPNUL,  "GT"},		/* test elements for > */
	{o_preinc,	OPNUL,  "PREINC"},	/* add one to variable (++x) */
	{o_predec,	OPNUL,  "PREDEC"},	/* subtract one from variable (--x) */
	{o_postinc,	OPNUL,  "POSTINC"},	/* add one to variable (x++) */
	{o_postdec,	OPNUL,  "POSTDEC"},	/* subtract one from variable (x--) */
	{o_debug,	OPONE,  "DEBUG"},	/* debugging point */
	{o_print,	OPONE,  "PRINT"},	/* print value */
	{o_assignpop,	OPNUL,  "ASSIGNPOP"},	/* assign to variable and pop it */
	{o_zero,	OPNUL,  "ZERO"},	/* put zero on the stack */
	{o_one,		OPNUL,  "ONE"},		/* put one on the stack */
	{o_printeol,	OPNUL,  "PRINTEOL"},	/* print end of line */
	{o_printspace,	OPNUL,  "PRINTSPACE"},	/* print a space */
	{o_printstring,	OPSTR,  "PRINTSTR"},	/* print constant string */
	{o_dupvalue,	OPNUL,  "DUPVALUE"},	/* duplicate value of top value */
	{o_oldvalue,	OPNUL,  "OLDVALUE"},	/* old value from previous calc */
	{o_quo,		OPNUL,  "QUO"},		/* integer quotient of top values */
	{o_power,	OPNUL,  "POWER"},	/* value raised to a power */
	{o_quit,	OPSTR,  "QUIT"},	/* quit program */
	{o_call,	OPTWO,  "CALL"},	/* call built-in routine */
	{o_getepsilon,	OPNUL,  "GETEPSILON"},	/* get allowed error for calculations */
	{o_and,		OPNUL,  "AND"},		/* arithmetic and or top two values */
	{o_or,		OPNUL,  "OR"},		/* arithmetic or of top two values */
	{o_not,		OPNUL,  "NOT"},		/* logical not or top value */
	{o_abs,		OPNUL,  "ABS"},		/* absolute value of top value */
	{o_sgn,		OPNUL,  "SGN"},		/* sign of number */
	{o_isint,	OPNUL,  "ISINT"},	/* whether number is an integer */
	{o_condorjump,	OPJMP,  "CONDORJUMP"},	/* conditional or jump */
	{o_condandjump,	OPJMP,  "CONDANDJUMP"},	/* conditional and jump */
	{o_square,	OPNUL,  "SQUARE"},	/* square top value */
	{o_string,	OPSTR,  "STRING"},	/* string constant value */
	{o_isnum,	OPNUL,  "ISNUM"},	/* whether value is a number */
	{o_undef,	OPNUL,  "UNDEF"},	/* load undefined value on stack */
	{o_isnull,	OPNUL,  "ISNULL"},	/* whether value is the null value */
	{o_argvalue,	OPARG,  "ARGVALUE"},	/* load value of arg (parameter) n */
	{o_matcreate,	OPONE,  "MATCREATE"},	/* create matrix */
	{o_ismat,	OPNUL,  "ISMAT"},	/* whether value is a matrix */
	{o_isstr,	OPNUL,  "ISSTR"},	/* whether value is a string */
	{o_getconfig,	OPNUL,  "GETCONFIG"},	/* get value of configuration parameter */
	{o_leftshift,	OPNUL,  "LEFTSHIFT"},	/* left shift of integer */
	{o_rightshift,	OPNUL,  "RIGHTSHIFT"},	/* right shift of integer */
	{o_casejump,	OPJMP,  "CASEJUMP"},	/* test case and jump if not matched */
	{o_isodd,	OPNUL,  "ISODD"},	/* whether value is odd integer */
	{o_iseven,	OPNUL,  "ISEVEN"},	/* whether value is even integer */
	{o_fiaddr,	OPNUL,  "FIADDR"},	/* 'fast index' matrix address */
	{o_fivalue,	OPNUL,  "FIVALUE"},	/* 'fast index' matrix value */
	{o_isreal,	OPNUL,  "ISREAL"},	/* whether value is real number */
	{o_imaginary,	OPONE,  "IMAGINARY"},	/* constant imaginary numeric value */
	{o_re,		OPNUL,  "RE"},		/* real part of complex number */
	{o_im,		OPNUL,  "IM"},		/* imaginary part of complex number */
	{o_conjugate,	OPNUL,  "CONJUGATE"},	/* complex conjugate */
	{o_objcreate,	OPONE,  "OBJCREATE"},	/* create object */
	{o_isobj,	OPNUL,  "ISOBJ"},	/* whether value is an object */
	{o_norm,	OPNUL,  "NORM"},	/* norm of value (square of abs) */
	{o_elemaddr,	OPONE,  "ELEMADDR"},	/* address of element of object */
	{o_elemvalue,	OPONE,  "ELEMVALUE"},	/* value of element of object */
	{o_istype,	OPNUL,  "ISTYPE"},	/* whether types are the same */
	{o_scale,	OPNUL,  "SCALE"},	/* scale value by a power of two */
	{o_islist,	OPNUL,	"ISLIST"},	/* whether value is a list */
	{o_swap,	OPNUL,	"SWAP"},	/* swap values of two variables */
	{o_issimple,	OPNUL,	"ISSIMPLE"},	/* whether value is simple type */
	{o_cmp,		OPNUL,	"CMP"},		/* compare values returning -1, 0, 1 */
	{o_quomod,	OPNUL,	"QUOMOD"},	/* calculate quotient and remainder */
	{o_setconfig,	OPNUL,	"SETCONFIG"},	/* set configuration parameter */
	{o_setepsilon,	OPNUL,  "SETEPSILON"},	/* set allowed error for calculations */
	{o_isfile,	OPNUL,  "ISFILE"},	/* whether value is a file */
	{o_isassoc,	OPNUL,  "ISASSOC"},	/* whether value is an association */
	{o_nop,		OPSTI,  "INITSTATIC"},	/* once only code for static init */
	{o_eleminit,	OPONE,	"ELEMINIT"},	/* assign element of matrix or object */
	{o_isconfig,	OPNUL,  "ISCONFIG"},	/* whether value is a configuration state */
	{o_ishash,	OPNUL,  "ISHASH"},	/* whether value is a hash state */
	{o_isrand,	OPNUL,  "ISRAND"},	/* whether value is a rand element */
	{o_israndom,	OPNUL,  "ISRANDOM"},	/* whether value is a random element */
	{o_show,	OPONE,	"SHOW"},	/* show current state data */
	{o_initfill,	OPNUL,	"INITFILL"}	/* initially fill matrix */
};


/*
 * Compute the result of a function by interpreting opcodes.
 * Arguments have just been pushed onto the evaluation stack.
 *
 * given:
 *	fp		function to calculate
 *	argcount	number of arguments called with
 */
void
calculate(FUNC *fp, int argcount)
{
	register unsigned long pc;	/* current pc inside function */
	register struct opcode *op;	/* current opcode pointer */
	register VALUE *locals;		/* pointer to local variables */
	long oldline;			/* old value of line counter */
	unsigned int opnum;		/* current opcode number */
	int origargcount;		/* original number of arguments */
	int i;				/* loop counter */
	BOOL dojump;			/* TRUE if jump is to occur */
	char *oldname;			/* old function name being executed */
	VALUE *beginstack;		/* beginning of stack frame */
	VALUE *args;			/* pointer to function arguments */
	VALUE retval;			/* function return value */
	VALUE localtable[QUICKLOCALS];	/* some local variables */

	oldname = funcname;
	oldline = funcline;
	funcname = fp->f_name;
	funcline = 0;
	origargcount = argcount;
	while (argcount < fp->f_paramcount) {
		stack++;
		stack->v_type = V_NULL;
		argcount++;
	}
	locals = localtable;
	if (fp->f_localcount > QUICKLOCALS) {
		locals = (VALUE *) malloc(sizeof(VALUE) * fp->f_localcount);
		if (locals == NULL) {
			math_error("No memory for local variables");
			/*NOTREACHED*/
		}
	}
	for (i = 0; i < fp->f_localcount; i++) {
		locals[i].v_num = qlink(&_qzero_);
		locals[i].v_type = V_NUM;
		locals[i].v_subtype = V_NOSUBTYPE;
	}
	pc = 0;
	beginstack = stack;
	args = beginstack - (argcount - 1);
	for (;;) {
		if (abortlevel >= ABORT_OPCODE) {
			math_error("Calculation aborted in opcode");
			/*NOTREACHED*/
		}
		if (pc >= fp->f_opcodecount) {
			math_error("Function pc out of range");
			/*NOTREACHED*/
		}
		if (stack > &stackarray[MAXSTACK-3]) {
			math_error("Evaluation stack depth exceeded");
			/*NOTREACHED*/
		}
		opnum = fp->f_opcodes[pc];
		if (opnum > MAX_OPCODE) {
			math_error("Function opcode out of range");
			/*NOTREACHED*/
		}
		op = &opcodes[opnum];
		if (conf->traceflags & TRACE_OPCODES) {
			dumpnames = FALSE;
			printf("%8s, pc %4ld:  ", fp->f_name, pc);
			(void)dumpop(&fp->f_opcodes[pc]);
		}
		/*
		 * Now call the opcode routine appropriately.
		 */
		pc++;
		switch (op->o_type) {
		case OPNUL:	/* no extra arguments */
			/* ignore Saber-C warning #65 - has 1 arg, expected 0 */
			/* 	  ok to ignore in proc calculate */
			(*op->o_func)(fp);
			break;

		case OPONE:	/* one extra integer argument */
			(*op->o_func)(fp, fp->f_opcodes[pc++]);
			break;

		case OPTWO:	/* two extra integer arguments */
			(*op->o_func)(fp, fp->f_opcodes[pc],
				fp->f_opcodes[pc+1]);
			pc += 2;
			break;

		case OPJMP:	/* jump opcodes (one extra pointer arg) */
			dojump = FALSE;
			(*op->o_func)(fp, &dojump);
			if (dojump)
				pc = fp->f_opcodes[pc];
			else
				pc++;
			break;

		case OPGLB:	/* global symbol reference (pointer arg) */
		case OPSTR:	/* string constant address */
			/* ignore Saber-C warning #68 - benign type mismatch */
			/* 	  ok to ignore in proc calculate */
			(*op->o_func)(fp, *((char **) &fp->f_opcodes[pc]));
			pc += PTR_SIZE;
			break;

		case OPLOC:	/* local variable reference */
			(*op->o_func)(fp, locals, fp->f_opcodes[pc++]);
			break;

		case OPPAR:	/* parameter variable reference */
			(*op->o_func)(fp, argcount, args, fp->f_opcodes[pc++]);
			break;

		case OPARG:	/* parameter variable reference */
			(*op->o_func)(fp, origargcount, args);
			break;

		case OPRET:	/* return from function */
			if (stack->v_type == V_ADDR)
				copyvalue(stack->v_addr, stack);
			for (i = 0; i < fp->f_localcount; i++)
				freevalue(&locals[i]);
			if (locals != localtable)
				free(locals);
			if (stack != &beginstack[1]) {
				math_error("Misaligned stack");
				/*NOTREACHED*/
			}
			if (argcount <= 0) {
				funcname = oldname;
				funcline = oldline;
				return;
			}
			retval = *stack--;
			while (--argcount >= 0)
				freevalue(stack--);
			*++stack = retval;
			funcname = oldname;
			funcline = oldline;
			return;

		case OPSTI:	/* static initialization code */
			fp->f_opcodes[pc++ - 1] = OP_JUMP;
			break;

		default:
			math_error("Unknown opcode type: %d", op->o_type);
			/*NOTREACHED*/
		}
	}
}


/*
 * Dump an opcode at a particular address.
 * Returns the size of the opcode so that it can easily be skipped over.
 *
 * given:
 *	pc		location of the opcode
 */
int
dumpop(unsigned long *pc)
{
	unsigned long op;	/* opcode number */

	op = *pc++;
	if (op <= MAX_OPCODE)
		printf("%s", opcodes[op].o_name);
	else
		printf("OP%ld", op);
	switch (op) {
		case OP_LOCALADDR: case OP_LOCALVALUE:
			if (dumpnames)
				printf(" %s\n", localname((long)*pc));
			else
				printf(" %ld\n", *pc);
			return 2;
		case OP_GLOBALADDR: case OP_GLOBALVALUE:
			printf(" %s\n", globalname(*((GLOBAL **) pc)));
			return (1 + PTR_SIZE);
		case OP_PARAMADDR: case OP_PARAMVALUE:
			if (dumpnames)
				printf(" %s\n", paramname((long)*pc));
			else
				printf(" %ld\n", *pc);
			return 2;
		case OP_PRINTSTRING: case OP_STRING:
			printf(" \"%s\"\n", *((char **) pc));
			return (1 + PTR_SIZE);
		case OP_QUIT:
			if (*(char **) pc)
				printf(" \"%s\"\n", *((char **) pc));
			else
				printf("\n");
			return (1 + PTR_SIZE);
		case OP_INDEXADDR:
			printf(" %ld %ld\n", pc[0], pc[1]);
			return 3;
		case OP_PRINT: case OP_JUMPEQ: case OP_JUMPNE: case OP_JUMP:
		case OP_CONDORJUMP: case OP_CONDANDJUMP: case OP_CASEJUMP:
		case OP_INITSTATIC: case OP_MATCREATE: case OP_OBJCREATE:
		case OP_SHOW: case OP_ELEMINIT: case OP_ELEMADDR:
		case OP_ELEMVALUE:
			printf(" %ld\n", *pc);
			return 2;
		case OP_NUMBER: case OP_IMAGINARY:
			qprintf(" %r", constvalue(*pc));
			printf("\n");
			return 2;
		case OP_DEBUG:
			printf(" line %ld\n", *pc);
			return 2;
		case OP_CALL:
			printf(" %s with %ld args\n",
			    builtinname((long)pc[0]), (long)pc[1]);
			return 3;
		case OP_USERCALL:
			printf(" %s with %ld args\n",
			    namefunc((long)pc[0]), (long)pc[1]);
			return 3;
		default:
			printf("\n");
			return 1;
	}
}
