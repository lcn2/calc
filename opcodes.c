/*
 * opcodes - opcode execution module
 *
 * Copyright (C) 1999-2007,2021-2023  David I. Bell and Ernest Bowen
 *
 * Primary author:  David I. Bell
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
 * Under source code control:   1990/02/15 01:48:19
 * File existed as early as:    before 1990
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <sys/types.h>
#include <setjmp.h>

#include "calc.h"
#include "opcodes.h"
#include "func.h"
#include "symbol.h"
#include "hist.h"
#include "file.h"
#include "zrand.h"
#include "zrandom.h"
#include "have_fgetsetpos.h"
#include "custom.h"
#include "lib_calc.h"
#include "block.h"
#include "str.h"

#include "have_unused.h"


#include "errtbl.h"
#include "banned.h"     /* include after system header <> includes */


#define QUICKLOCALS     20              /* local vars to handle quickly */


STATIC VALUE stackarray[MAXSTACK];      /* storage for stack */
STATIC VALUE oldvalue;                  /* previous calculation value */
STATIC bool saveval = true;             /* to enable or disable saving */
STATIC int errcount;                    /* counts calls to error_value */
STATIC bool go;
STATIC long calc_depth;

/*
 * global symbols
 */
VALUE *stack = NULL;            /* current location of top of stack */
int dumpnames = false;          /* names if true, otherwise indices */
char *funcname = NULL;          /* function being executed */
long funcline = 0;              /* function line being executed */
int calc_errno = 0;             /* global calc_errno value */


/*
 * forward declarations
 */
S_FUNC void showsizes(void);
S_FUNC void o_paramaddr(FUNC *fp, int argcnt, VALUE *args, long index);
S_FUNC void o_getvalue(FUNC *fp);


/*
 * Types of opcodes (depends on arguments saved after the opcode).
 */
#define OPNUL   1       /* opcode has no arguments */
#define OPONE   2       /* opcode has one integer argument */
#define OPTWO   3       /* opcode has two integer arguments */
#define OPJMP   4       /* opcode is a jump (with one pointer argument) */
#define OPRET   5       /* opcode is a return (with no argument) */
#define OPGLB   6       /* opcode has global symbol pointer argument */
#define OPPAR   7       /* opcode has parameter index argument */
#define OPLOC   8       /* opcode needs local variable pointer (with one arg) */
#define OPSTR   9       /* opcode has a string constant arg */
#define OPARG   10      /* opcode is given number of arguments */
#define OPSTI   11      /* opcode is static initialization */


/*
 * opcode - info about each opcode
 */
typedef union {
        void (*func_nul)(FUNC *);                       /* OPNUL */
        void (*func_one)(FUNC *, long);                 /* OPONE */
        void (*func_two)(FUNC *, long, long);           /* OPTWO */
        void (*func_jmp)(FUNC *, bool *);               /* OPJMP */
        void (*func_ret)(FUNC *);                       /* OPRET */
        void (*func_glb)(FUNC *, GLOBAL *);             /* OPGLB */
        void (*func_par)(FUNC *, int, VALUE *, long);   /* OPPAR */
        void (*func_loc)(FUNC *, VALUE *, long);        /* OPLOC */
        /* has a string constant arg is unused */       /* OPSTR */
        void (*func_arg)(FUNC *, int, VALUE *);         /* OPARG */
        void (*func_sti)(FUNC *);                       /* OPSTI */
} opfunc;

struct opcode {
        opfunc o_func;          /* routine to call for opcode */
        int o_type;             /* type of opcode */
        char *o_name;           /* name of opcode */
};


/*
 * external configuration functions
 */
E_FUNC void config_value(CONFIG *cfg, int type, VALUE *ret);
E_FUNC void setconfig(int type, VALUE *vp);


/*
 * Initialize the stack.
 */
void
initstack(void)
{
        unsigned int i;

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
        /* initialize calc_depth */

        calc_depth = 0;
}


/*
 * The various opcodes
 */
S_FUNC void
o_nop(FUNC *UNUSED(fp))
{
}


S_FUNC void
o_localaddr(FUNC *fp, VALUE *locals, long index)
{
        if ((unsigned long)index >= fp->f_localcount) {
                math_error("Bad local variable index");
                not_reached();
        }
        locals += index;
        stack++;
        stack->v_addr = locals;
        stack->v_type = V_ADDR;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_globaladdr(FUNC *UNUSED(fp), GLOBAL *sp)
{
        if (sp == NULL) {
                math_error("Global variable \"%s\" not initialized",
                            sp->g_name);
                not_reached();
        }
        stack++;
        stack->v_addr = &sp->g_value;
        stack->v_type = V_ADDR;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_paramaddr(FUNC *UNUSED(fp), int argcount, VALUE *args, long index)
{
        if ((long)index >= argcount) {
                math_error("Bad parameter index");
                not_reached();
        }
        args += index;
        stack++;
        if (args->v_type == V_OCTET || args->v_type == V_ADDR) {
                *stack = *args;
                return;
        }
        stack->v_addr = args;
        stack->v_type = V_ADDR;
/*      stack->v_subtype = V_NOSUBTYPE; */ /* XXX ??? */
}


S_FUNC void
o_localvalue(FUNC *fp, VALUE *locals, long index)
{
        if ((unsigned long)index >= fp->f_localcount) {
                math_error("Bad local variable index");
                not_reached();
        }
        locals += index;
        copyvalue(locals, ++stack);
}


S_FUNC void
o_globalvalue(FUNC *UNUSED(fp), GLOBAL *sp)
{
        if (sp == NULL) {
                math_error("Global variable not defined");
                not_reached();
        }
        copyvalue(&sp->g_value, ++stack);
}


S_FUNC void
o_paramvalue(FUNC *UNUSED(fp), int argcount, VALUE *args, long index)
{
        if ((long)index >= argcount) {
                math_error("Bad parameter index");
                not_reached();
        }
        args += index;
        if (args->v_type == V_ADDR)
                args = args->v_addr;
        copyvalue(args, ++stack);
}


S_FUNC void
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
                        not_reached();
        }
        if (qiszero(vp->v_num)) {
                if (stack->v_type == V_NUM)
                        qfree(stack->v_num);
                stack->v_num = itoq((long) argcount);
                stack->v_type = V_NUM;
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        index = qtoi(vp->v_num) - 1;
        if (stack->v_type == V_NUM)
                qfree(stack->v_num);
        stack--;
        (void) o_paramaddr(fp, argcount, args, index);
}


S_FUNC void
o_number(FUNC *UNUSED(fp), long arg)
{
        NUMBER *q;

        q = constvalue(arg);
        if (q == NULL) {
                math_error("Numeric constant value not found");
                not_reached();
        }
        stack++;
        stack->v_num = qlink(q);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_imaginary(FUNC *UNUSED(fp), long arg)
{
        NUMBER *q;
        COMPLEX *c;

        q = constvalue(arg);
        if (q == NULL) {
                math_error("Numeric constant value not found");
                not_reached();
        }
        stack++;
        stack->v_subtype = V_NOSUBTYPE;
        if (qiszero(q)) {
                stack->v_num = qlink(q);
                stack->v_type = V_NUM;
                return;
        }
        c = comalloc();
        qfree(c->imag);
        c->imag = qlink(q);
        stack->v_com = c;
        stack->v_type = V_COM;
}


S_FUNC void
o_string(FUNC *UNUSED(fp), long arg)
{
        stack++;
        stack->v_str = slink(findstring(arg));
        stack->v_type = V_STR;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_undef(FUNC *UNUSED(fp))
{
        stack++;
        stack->v_type = V_NULL;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_matcreate(FUNC *UNUSED(fp), long dim)
{
        register MATRIX *mp;    /* matrix being defined */
        NUMBER *num1;           /* first number from stack */
        NUMBER *num2;           /* second number from stack */
        VALUE *v1, *v2;
        long min[MAXDIM];       /* minimum range */
        long max[MAXDIM];       /* maximum range */
        long i;                 /* index */
        long tmp;               /* temporary */
        long size;              /* size of matrix */

        if ((dim < 0) || (dim > MAXDIM)) {
                math_error("Bad dimension %ld for matrix", dim);
                not_reached();
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
                        not_reached();
                }
                num1 = v1->v_num;
                num2 = v2->v_num;
                if (qisfrac(num1) || qisfrac(num2)) {
                        math_error("Non-integral bounds for matrix");
                        not_reached();
                }
                if (zge31b(num1->num) || zge31b(num2->num)) {
                        math_error("Very large bounds for matrix");
                        not_reached();
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
                        not_reached();
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
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_mat = mp;
}


S_FUNC void
o_eleminit(FUNC *UNUSED(fp), long index)
{
        VALUE *vp;
        STATIC VALUE *oldvp;
        VALUE tmp;
        OCTET *ptr;
        BLOCK *blk;
        unsigned short subtype;

        vp = &stack[-1];
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type < 0) {
                freevalue(stack--);
                error_value(E_INIT_01);
                return;
        }
        if (vp->v_subtype & V_NOCOPYTO) {
                freevalue(stack--);
                error_value(E_INIT_02);
                return;
        }
        switch (vp->v_type) {
        case V_MAT:
                if ((index < 0) || (index >= vp->v_mat->m_size)) {
                        freevalue(stack--);
                        error_value(E_INIT_03);
                        return;
                }
                oldvp = &vp->v_mat->m_table[index];
                break;
        case V_OBJ:
                if (index < 0 || index >= vp->v_obj->o_actions->oa_count) {
                        freevalue(stack--);
                        error_value(E_INIT_03);
                        return;
                }
                oldvp = &vp->v_obj->o_table[index];
                break;
        case V_LIST:
                oldvp = listfindex(vp->v_list, index);
                if (oldvp == NULL) {
                        freevalue(stack--);
                        error_value(E_INIT_03);
                        return;
                }
                break;
        case V_STR:
                if (index < 0 || (size_t)index >= vp->v_str->s_len) {
                        freevalue(stack--);
                        error_value(E_INIT_03);
                        return;
                }
                ptr = (OCTET *)(&vp->v_str->s_str[index]);
                vp = stack;
                if (vp->v_type == V_ADDR)
                        vp = vp->v_addr;
                copy2octet(vp, ptr);
                freevalue(stack--);
                return;
        case V_NBLOCK:
        case V_BLOCK:
                if (vp->v_type == V_NBLOCK) {
                        blk = vp->v_nblock->blk;
                        if (blk->data == NULL) {
                                freevalue(stack--);
                                error_value(E_INIT_04);
                                return;
                        }
                }
                else
                        blk = vp->v_block;
                if (index >= blk->maxsize) {
                        freevalue(stack--);
                        error_value(E_INIT_03);
                        return;
                }
                ptr = blk->data + index;
                vp = stack;
                if (vp->v_type == V_ADDR)
                        vp = vp->v_addr;
                copy2octet(vp, ptr);
                if (index >= blk->datalen)
                        blk->datalen = index + 1;
                freevalue(stack--);
                return;
        default:
                freevalue(stack--);
                error_value(E_INIT_05);
                return;
        }
        vp = stack--;
        subtype = oldvp->v_subtype;
        if (subtype & V_NOASSIGNTO) {
                freevalue(vp);
                error_value(E_INIT_06);
                return;
        }
        if (vp->v_type == V_ADDR) {
                vp = vp->v_addr;
                if (vp == oldvp)
                        return;
                copyvalue(vp, &tmp);
        }
        else
                tmp = *vp;
        if ((subtype & V_NONEWVALUE) && comparevalue(oldvp, &tmp)) {
                freevalue(&tmp);
                error_value(E_INIT_07);
                return;
        }
        if ((subtype & V_NONEWTYPE) && oldvp->v_type != tmp.v_type) {
                freevalue(&tmp);
                error_value(E_INIT_08);
                return;
        }
        if ((subtype & V_NOERROR) && tmp.v_type < 0) {
                error_value(E_INIT_09);
                return;
        }
        if (tmp.v_subtype & (V_NOASSIGNFROM | V_NOCOPYFROM)) {
                freevalue(&tmp);
                error_value(E_INIT_10);
                return;
        }
        tmp.v_subtype |= oldvp->v_subtype;
        freevalue(oldvp);
        *oldvp = tmp;
}


/*
 * o_indexaddr
 *
 * given:
 *      fp              function to calculate
 *      dim             dimension of matrix
 *      writeflag       nonzero if element will be written
 */
S_FUNC void
o_indexaddr(FUNC *UNUSED(fp), long dim, long writeflag)
{
        int i;
        bool flag;
        VALUE *val;
        VALUE *vp;
        VALUE indices[MAXDIM];  /* index values */
        long index;             /* single dimension index for blocks */
        VALUE ret;              /* OCTET from as indexed from a block */
        BLOCK *blk;

        flag = (writeflag != 0);
        if (dim < 0)  {
                math_error("Negative dimension for indexing");
                not_reached();
        }
        val = &stack[-dim];
        if (val->v_type != V_NBLOCK && val->v_type != V_FILE) {
                if (val->v_type != V_ADDR) {
                        math_error("Non-pointer for indexaddr");
                        not_reached();
                }
                val = val->v_addr;
        }
        blk = NULL;
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
        case V_NBLOCK:
        case V_BLOCK:
                if (val->v_type == V_BLOCK)
                        blk = val->v_block;
                else
                        blk = val->v_nblock->blk;
                if (blk->data == NULL) {
                        math_error("Freed block");
                        not_reached();
                }

                /*
                 * obtain single dimensional block index
                 */
                if (dim != 1) {
                        math_error("block has only one dimension");
                        not_reached();
                }
                if (indices[0].v_type != V_NUM) {
                        math_error("Non-numeric index for block");
                        not_reached();
                }
                if (qisfrac(indices[0].v_num)) {
                        math_error("Non-integral index for block");
                        not_reached();
                }
                if (zge31b(indices[0].v_num->num) ||
                    zisneg(indices[0].v_num->num)) {
                        math_error("Index out of bounds for block");
                        not_reached();
                }
                index = ztoi(indices[0].v_num->num);

                if (index >= blk->maxsize) {
                        math_error("Index out of bounds for block");
                        not_reached();
                }
                if (index >= blk->datalen)
                        blk->datalen = index + 1;
                ret.v_type = V_OCTET;
                ret.v_subtype = val->v_subtype;
                ret.v_octet = &blk->data[index];
                freevalue(stack--);
                *stack = ret;
                return;
        case V_STR:
                if (dim != 1) {
                        math_error("string has only one dimension");
                        not_reached();
                }
                if (indices[0].v_type != V_NUM) {
                        math_error("Non-numeric index for string");
                        not_reached();
                }
                if (qisfrac(indices[0].v_num)) {
                        math_error("Non-integral index for string");
                        not_reached();
                }
                if (zge31b(indices[0].v_num->num) ||
                    zisneg(indices[0].v_num->num)) {
                        math_error("Index out of bounds for string");
                        not_reached();
                }
                index = ztoi(indices[0].v_num->num);
                if (index < 0 || (size_t)index >= val->v_str->s_len) {
                        math_error("Index out of bounds for string");
                        not_reached();
                }
                ret.v_type = V_OCTET;
                ret.v_subtype = val->v_subtype;
                ret.v_octet = (OCTET *)(val->v_str->s_str + index);
                freevalue(stack--);
                *stack = ret;
                return;
        case V_LIST:
                if (dim != 1) {
                        math_error("list has only one dimension");
                        not_reached();
                }
                if (indices[0].v_type != V_NUM) {
                        math_error("Non-numeric index for list");
                        not_reached();
                }
                if (qisfrac(indices[0].v_num)) {
                        math_error("Non-integral index for list");
                        not_reached();
                }
                if (zge31b(indices[0].v_num->num) ||
                    zisneg(indices[0].v_num->num)) {
                        math_error("Index out of bounds for list");
                        not_reached();
                }
                index = ztoi(indices[0].v_num->num);
                vp = listfindex(val->v_list, index);
                if (vp == NULL) {
                        math_error("Index out of bounds for list");
                        not_reached();
                }
                break;
        default:
                math_error("Illegal value for indexing");
                not_reached();
        }
        while (dim-- > 0)
                freevalue(stack--);
        stack->v_type = V_ADDR;
        stack->v_addr = vp;
}


S_FUNC void
o_elemaddr(FUNC *UNUSED(fp), long index)
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
                        not_reached();
                }
                vp = &mp->m_table[index];
                break;
        case V_OBJ:
                op = vp->v_obj;
                offset = objoffset(op, index);
                if (offset < 0) {
                        math_error("Non-existent element for object");
                        not_reached();
                }
                vp = &op->o_table[offset];
                break;
        case V_LIST:
                vp = listfindex(vp->v_list, index);
                if (vp == NULL) {
                        math_error("Index out of bounds for list");
                        not_reached();
                }
                break;
        default:
                math_error("Not initializing matrix, object or list");
                not_reached();
        }
        stack->v_type = V_ADDR;
        stack->v_addr = vp;

}


S_FUNC void
o_elemvalue(FUNC *fp, long index)
{
        o_elemaddr(fp, index);
        copyvalue(stack->v_addr, stack);
}


S_FUNC void
o_objcreate(FUNC *UNUSED(fp), long arg)
{
        stack++;
        stack->v_type = V_OBJ;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_obj = objalloc(arg);
}


S_FUNC void
o_assign(FUNC *UNUSED(fp))
{
        VALUE *var;             /* variable value */
        VALUE *vp;
        VALUE tmp;
        unsigned short subtype;
        USB8 octet;

        /*
         * get what we will store into
         */
        var = &stack[-1];

        /*
         * If what we will store into is an OCTET, we must
         * handle this specially.  Only the bottom 8 bits of
         * certain value types will be assigned ... not the
         * entire value.
         */
        if (var->v_type == V_OCTET) {
                if (var->v_subtype & V_NOCOPYTO) {
                        freevalue(stack--);
                        *stack = error_value(E_ASSIGN_1);
                        return;
                }
                vp = stack;
                if (vp->v_type == V_ADDR)
                        vp = vp->v_addr;
                if (vp->v_subtype & V_NOCOPYFROM || vp->v_type < 0) {
                        freevalue(stack--);
                        *stack = error_value(E_ASSIGN_2);
                        return;
                }
                copy2octet(vp, &octet);
                freevalue(stack--);
                if ((var->v_subtype & V_NONEWVALUE) && *var->v_octet != octet) {
                        *stack = error_value(E_ASSIGN_3);
                        return;
                }
                *var->v_octet = octet;
                return;
        }
        if (var->v_type != V_ADDR) {
                freevalue(stack--);
                *stack = error_value(E_ASSIGN_4);
                return;
        }

        var = var->v_addr;
        subtype = var->v_subtype;
        if (subtype & V_NOASSIGNTO) {
                freevalue(stack--);
                *stack = error_value(E_ASSIGN_5);
                return;
        }

        vp = stack;

        if (var->v_type == V_OBJ)  {
                if (vp->v_type == V_ADDR)
                        vp = vp->v_addr;
                (void) objcall(OBJ_ASSIGN, var, vp, NULL_VALUE);
                freevalue(stack--);
                return;
        }

        stack--;

        /*
         * Get what we will store from
         * If what will store from is an address, make a copy
         * of the de-referenced address instead.
         */
        if (vp->v_type == V_ADDR) {
                vp = vp->v_addr;
                if (vp == var)
                        return;
                if (vp->v_subtype & V_NOASSIGNFROM) {
                        *stack = error_value(E_ASSIGN_6);
                        return;
                }
                copyvalue(vp, &tmp);
        } else if (vp->v_type == V_OCTET) {
                copyvalue(vp, &tmp);
        } else {
                tmp = *vp;
        }

        /*
         * Check protection
         */
        if ((subtype & V_NONEWVALUE) && comparevalue(var, &tmp)) {
                freevalue(&tmp);
                *stack = error_value(E_ASSIGN_7);
                return;
        }
        if ((subtype & V_NONEWTYPE) && var->v_type != tmp.v_type) {
                freevalue(&tmp);
                *stack = error_value(E_ASSIGN_8);
                return;
        }
        if ((subtype & V_NOERROR) && tmp.v_type < 0) {
                *stack = error_value(E_ASSIGN_9);
                return;
        }

        /*
         * perform the assignment
         */
        freevalue(var);
        *var = tmp;
        var->v_subtype |= subtype;
}


S_FUNC void
o_assignback(FUNC *fp)
{
        VALUE tmp;

        tmp = stack[-1];
        stack[-1] = stack[0];
        stack[0] = tmp;
        o_assign(fp);
}


S_FUNC void
o_assignpop(FUNC *fp)
{
        o_assign(fp);
        stack--;
}


S_FUNC void
o_ptr(FUNC *UNUSED(fp))
{
        switch (stack->v_type) {
        case V_ADDR:
                stack->v_type = V_VPTR;
                break;
        case V_OCTET:
                stack->v_type = V_OPTR;
                break;
        case V_STR:
                sfree(stack->v_str);
                stack->v_type = V_SPTR;
                break;
        case V_NUM:
                qfree(stack->v_num);
                stack->v_type = V_NPTR;
                break;
        default:
                math_error("Addressing non-addressable type");
                not_reached();
        }
}


S_FUNC void
o_deref(FUNC *UNUSED(fp))
{
        VALUE *vp;

        vp = stack;

        if (stack->v_type == V_OCTET) {
                stack->v_num = itoq(*vp->v_octet);
                stack->v_type = V_NUM;
                return;
        }
        if (stack->v_type == V_OPTR) {
                stack->v_type = V_OCTET;
                return;
        }
        if (stack->v_type == V_VPTR) {
                stack->v_type = V_ADDR;
                return;
        }
        if (stack->v_type == V_SPTR) {
                stack->v_type = V_STR;
                return;
        }
        if (stack->v_type == V_NPTR) {
                if (stack->v_num->links == 0) {
                        stack->v_type = V_NULL;
                        return;
                }
                stack->v_type = V_NUM;
                stack->v_num->links++;
                return;
        }
        if (stack->v_type != V_ADDR) {
                math_error("Dereferencing a non-variable");
                not_reached();
        }
        vp = vp->v_addr;
        switch (vp->v_type) {
        case V_ADDR:
        case V_OCTET:
                *stack = *vp;
                break;
        case V_OPTR:
                *stack = *vp;
                stack->v_type = V_OCTET;
                break;
        case V_VPTR:
                *stack = *vp;
                stack->v_type = V_ADDR;
                break;
        case V_SPTR:
                *stack = *vp;
                stack->v_type = V_STR;
                break;
        case V_NPTR:
                if (vp->v_num->links == 0) {
                        stack->v_type = V_NULL;
                        break;
                }
                stack->v_type = V_NUM;
                stack->v_num = vp->v_num;
                stack->v_num->links++;
                break;
        default:
                copyvalue(vp, stack);
        }
}


S_FUNC void
o_swap(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;         /* variables to be swapped */
        VALUE tmp;
        USB8 usb;

        v1 = stack--;
        v2 = stack;

        if (v1->v_type == V_OCTET && v2->v_type == V_OCTET) {
                if (v1->v_octet != v2->v_octet &&
                                ((v1->v_subtype | v2->v_subtype) &
                        (V_NOCOPYTO | V_NOCOPYFROM))) {
                        *stack = error_value(E_SWAP_1);
                        return;
                }
                usb = *v1->v_octet;
                *v1->v_octet = *v2->v_octet;
                *v2->v_octet = usb;
        } else if (v1->v_type == V_ADDR && v2->v_type == V_ADDR) {
                v1 = v1->v_addr;
                v2 = v2->v_addr;
                if (v1 != v2 && ((v1->v_subtype | v2->v_subtype) &
                                (V_NOASSIGNTO | V_NOASSIGNFROM))) {
                        *stack = error_value(E_SWAP_2);
                        return;
                }
                tmp = *v1;
                *v1 = *v2;
                *v2 = tmp;
        } else {
                *stack = error_value(E_SWAP_3);
                return;
        }
        stack->v_type = V_NULL;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_add(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;
        VALUE w1, w2;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        if (v1->v_type == V_OCTET) {
                w1.v_type = V_NUM;
                w1.v_subtype = V_NOSUBTYPE;
                w1.v_num = itoq(*v1->v_octet);
                v1 = &w1;
        }
        if (v2->v_type == V_OCTET) {
                w2.v_type = V_NUM;
                w2.v_subtype = V_NOSUBTYPE;
                w2.v_num = itoq(*v2->v_octet);
                v2 = &w2;
        }

        addvalue(v1, v2, &tmp);
        if (v1 == &w1)
                qfree(w1.v_num);
        if (v2 == &w2)
                qfree(w2.v_num);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_sub(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;
        VALUE w1, w2;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        if (v1->v_type == V_OCTET) {
                w1.v_type = V_NUM;
                w1.v_subtype = V_NOSUBTYPE;
                w1.v_num = itoq((unsigned char) *v1->v_octet);
                v1 = &w1;
        }
        if (v2->v_type == V_OCTET) {
                w2.v_type = V_NUM;
                w2.v_subtype = V_NOSUBTYPE;
                w2.v_num = itoq((unsigned char) *v2->v_octet);
                v2 = &w2;
        }

        subvalue(v1, v2, &tmp);
        if (v1 == &w1)
                qfree(w1.v_num);
        if (v2 == &w2)
                qfree(w2.v_num);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_mul(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;
        VALUE w1, w2;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        if (v1->v_type == V_OCTET) {
                w1.v_type = V_NUM;
                w1.v_subtype = V_NOSUBTYPE;
                w1.v_num = itoq(*v1->v_octet);
                v1 = &w1;
        }
        if (v2->v_type == V_OCTET) {
                w2.v_type = V_NUM;
                w2.v_subtype = V_NOSUBTYPE;
                w2.v_num = itoq(*v2->v_octet);
                v2 = &w2;
        }
        mulvalue(v1, v2, &tmp);
        if (v1 == &w1)
                qfree(w1.v_num);
        if (v2 == &w2)
                qfree(w2.v_num);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_power(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        powvalue(v1, v2, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_div(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;
        VALUE w1, w2;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        if (v1->v_type == V_OCTET) {
                w1.v_type = V_NUM;
                w1.v_subtype = V_NOSUBTYPE;
                w1.v_num = itoq(*v1->v_octet);
                v1 = &w1;
        }
        if (v2->v_type == V_OCTET) {
                w2.v_type = V_NUM;
                w2.v_subtype = V_NOSUBTYPE;
                w2.v_num = itoq(*v2->v_octet);
                v2 = &w2;
        }
        divvalue(v1, v2, &tmp);
        if (v1 == &w1)
                qfree(w1.v_num);
        if (v2 == &w2)
                qfree(w2.v_num);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_quo(FUNC *UNUSED(fp))
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
        null.v_subtype = V_NOSUBTYPE;
        quovalue(v1, v2, &null, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_mod(FUNC *UNUSED(fp))
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
        null.v_subtype = V_NOSUBTYPE;
        modvalue(v1, v2, &null, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_and(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;

        andvalue(v1, v2, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_or(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;

        orvalue(v1, v2, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}

S_FUNC void
o_xor(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;

        v1 = &stack[-1];
        v2 = &stack[0];

        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;

        xorvalue(v1, v2, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_comp(FUNC *UNUSED(fp))
{
        VALUE *vp;
        VALUE tmp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        compvalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_not(FUNC *UNUSED(fp))
{
        VALUE *vp;
        VALUE val;
        int r = 0;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type == V_OBJ) {
                val = objcall(OBJ_NOT, vp, NULL_VALUE, NULL_VALUE);
                freevalue(stack);
                *stack = val;
                return;
        }
        r = testvalue(vp);
        freevalue(stack);
        stack->v_num = (r ? qlink(&_qzero_) : qlink(&_qone_));
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_plus(FUNC *UNUSED(fp))
{
        VALUE *vp;
        VALUE tmp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;

        tmp.v_type = V_NULL;
        tmp.v_subtype = V_NOSUBTYPE;
        switch (vp->v_type) {
        case V_OBJ:
                tmp = objcall(OBJ_PLUS, vp, NULL_VALUE, NULL_VALUE);
                break;
        case V_LIST:
                addlistitems(vp->v_list, &tmp);
                break;
        default:
                return;
        }
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_negate(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        negvalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_invert(FUNC *UNUSED(fp))
{
        VALUE *vp;
        VALUE tmp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;

        invertvalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_scale(FUNC *UNUSED(fp))
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


S_FUNC void
o_int(FUNC *UNUSED(fp))
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


S_FUNC void
o_frac(FUNC *UNUSED(fp))
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


S_FUNC void
o_abs(FUNC *UNUSED(fp))
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
                !qispos(v2->v_num)) {
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
        q = qqabs(v1->v_num);
        if (stack->v_type == V_NUM)
                qfree(stack->v_num);
        stack->v_num = q;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_norm(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        normvalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_square(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        squarevalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_test(FUNC *UNUSED(fp))
{
        VALUE *vp;
        int i;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        i = testvalue(vp);
        freevalue(stack);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_num = i ? qlink(&_qone_) : qlink(&_qzero_);
}


S_FUNC void
o_links(FUNC *UNUSED(fp))
{
        VALUE *vp;
        long links;
        bool haveaddress;

        vp = stack;
        haveaddress = (vp->v_type == V_ADDR);
        if (haveaddress)
                vp = vp->v_addr;
        switch (vp->v_type) {
        case V_NUM: links = vp->v_num->links; break;
        case V_COM: links = vp->v_com->links; break;
        case V_STR: links = vp->v_str->s_links; break;
        default:
                freevalue(stack);
                return;
        }
        if (links <= 0) {
                math_error("Non-positive links!!!");
                not_reached();
        }
        freevalue(stack);
        if (!haveaddress)
                links--;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_num = itoq(links);
}


S_FUNC void
o_bit(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        long index;
        int r;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        if (v2->v_type != V_NUM || qisfrac(v2->v_num)) {
                freevalue(stack--);
                freevalue(stack);
                *stack = error_value(E_BIT_1);
                return;
        }
        if (zge31b(v2->v_num->num)) {
                freevalue(stack--);
                freevalue(stack);
                *stack = error_value(E_BIT_2);
                return;
        }
        index = qtoi(v2->v_num);
        switch (v1->v_type) {
        case V_NUM:
                r = qisset(v1->v_num, index);
                break;
        case V_STR:
                r = stringbit(v1->v_str, index);
                break;
        default:
                r = 2;
        }
        freevalue(stack--);
        freevalue(stack);
        if (r > 1) {
                *stack = error_value(E_BIT_1);
        } else if (r < 0) {
                stack->v_type = V_NULL;
        } else {
                stack->v_type = V_NUM;
                stack->v_num = itoq(r);
        }
        stack->v_subtype = V_NOSUBTYPE;
}

S_FUNC void
o_highbit(FUNC *UNUSED(fp))
{
        VALUE *vp;
        long index;
        unsigned int u;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        switch (vp->v_type) {
        case V_NUM:
                if (qiszero(vp->v_num)) {
                        index = -1;
                        break;
                }
                if (qisfrac(vp->v_num)) {
                        index = -2;
                        break;
                }
                index = zhighbit(vp->v_num->num);
                break;
        case V_STR:
                index = stringhighbit(vp->v_str);
                break;
        case V_OCTET:
                u = *vp->v_octet;
                for (index = -1; u; u >>= 1, ++index);
                break;
        default:
                index = -3;
        }
        freevalue(stack);
        switch (index) {
        case -3:
                *stack = error_value(E_HIGHBIT_1);
                return;
        case -2:
                *stack = error_value(E_HIGHBIT_2);
                return;
        default:
                stack->v_type = V_NUM;
                stack->v_subtype = V_NOSUBTYPE;
                stack->v_num = itoq(index);
        }
}


S_FUNC void
o_lowbit(FUNC *UNUSED(fp))
{
        VALUE *vp;
        long index;
        unsigned int u;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        switch (vp->v_type) {
        case V_NUM:
                if (qiszero(vp->v_num)) {
                        index = -1;
                        break;
                }
                if (qisfrac(vp->v_num)) {
                        index = -2;
                        break;
                }
                index = zlowbit(vp->v_num->num);
                break;
        case V_STR:
                index = stringlowbit(vp->v_str);
                break;
        case V_OCTET:
                u = *vp->v_octet;
                index = -1;
                if (u) do {
                        ++index;
                        u >>= 1;
                } while (!(u & 1));
                break;
        default:
                index = -3;
        }
        freevalue(stack);
        switch (index) {
        case -3:
                *stack = error_value(E_LOWBIT_1);
                return;
        case -2:
                *stack = error_value(E_LOWBIT_2);
                return;
        default:
                stack->v_type = V_NUM;
                stack->v_subtype = V_NOSUBTYPE;
                stack->v_num = itoq(index);
        }
}


S_FUNC void
o_content(FUNC *UNUSED(fp))
{
        VALUE *vp;
        VALUE tmp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        contentvalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_hashop(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        hashopvalue(v1, v2, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_backslash(FUNC *UNUSED(fp))
{
        VALUE *vp;
        VALUE tmp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        backslashvalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_setminus(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        setminusvalue(v1, v2, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_istype(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isint(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isnum(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        stack->v_num = qlink(&_qone_);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_ismat(FUNC *UNUSED(fp))
{
        VALUE *vp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_MAT) {
                freevalue(stack);
                stack->v_num = qlink(&_qzero_);
                stack->v_type = V_NUM;
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        freevalue(stack);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_num = qlink(&_qone_);
}


S_FUNC void
o_islist(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isobj(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isstr(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isfile(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isrand(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_israndom(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isconfig(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_ishash(FUNC *UNUSED(fp))
{
        VALUE *vp;
        int r;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        r = (vp->v_type == V_HASH);
        if (r != 0)
                r = vp->v_hash->hashtype;
        freevalue(stack);
        stack->v_num = itoq((long) r);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isassoc(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isblock(FUNC *UNUSED(fp))
{
        VALUE *vp;
        long r;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        r = 0;
        if (vp->v_type == V_NBLOCK)
                r = 2;
        else if (vp->v_type == V_BLOCK)
                r = 1;
        freevalue(stack);
        stack->v_num = itoq(r);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isoctet(FUNC *UNUSED(fp))
{
        VALUE *vp;
        long r;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        r = (vp->v_type == V_OCTET);
        freevalue(stack);
        stack->v_num = itoq(r);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isptr(FUNC *UNUSED(fp))
{
        VALUE *vp;
        long r;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        r = 0;
        switch(vp->v_type) {
        case V_OPTR: r = 1; break;
        case V_VPTR: r = 2; break;
        case V_SPTR: r = 3; break;
        case V_NPTR: r = 4; break;
        }
        freevalue(stack);
        stack->v_num = itoq(r);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isdefined(FUNC *UNUSED(fp))
{
        VALUE *vp;
        long r;
        long index;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_STR) {
                math_error("Non-string argument for isdefined");
                not_reached();
        }
        r = 0;
        index = getbuiltinfunc(vp->v_str->s_str);
        if (index >= 0) {
                r = 1;
        } else {
                index = getuserfunc(vp->v_str->s_str);
                if (index >= 0)
                        r = 2;
        }
        freevalue(stack);
        stack->v_num = itoq(r);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isobjtype(FUNC *UNUSED(fp))
{
        VALUE *vp;
        long index;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_STR) {
                math_error("Non-string argument for isobjtype");
                not_reached();
        }
        index = checkobject(vp->v_str->s_str);
        freevalue(stack);
        stack->v_num = itoq(index >= 0);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_issimple(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isodd(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        freevalue(stack);
        stack->v_num = qlink(&_qzero_);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_iseven(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        freevalue(stack);
        stack->v_num = qlink(&_qzero_);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isreal(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        freevalue(stack);
        stack->v_num = qlink(&_qzero_);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_isnull(FUNC *UNUSED(fp))
{
        VALUE *vp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_NULL) {
                freevalue(stack);
                stack->v_num = qlink(&_qzero_);
                stack->v_type = V_NUM;
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        freevalue(stack);
        stack->v_num = qlink(&_qone_);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_re(FUNC *UNUSED(fp))
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
                        stack->v_subtype = V_NOSUBTYPE;
                }
                return;
        }
        if (vp->v_type != V_COM) {
                math_error("Taking real part of non-number");
                not_reached();
        }
        q = qlink(vp->v_com->real);
        if (stack->v_type == V_COM)
                comfree(stack->v_com);
        stack->v_num = q;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_im(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        if (vp->v_type != V_COM) {
                math_error("Taking imaginary part of non-number");
                not_reached();
        }
        q = qlink(vp->v_com->imag);
        if (stack->v_type == V_COM)
                comfree(stack->v_com);
        stack->v_num = q;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_conjugate(FUNC *UNUSED(fp))
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
                        stack->v_subtype = V_NOSUBTYPE;
                }
                return;
        }
        conjvalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_fiaddr(FUNC *UNUSED(fp))
{
        register MATRIX *m;     /* current matrix element */
        LIST *lp;               /* list header */
        ASSOC *ap;              /* association header */
        VALUE *vp;              /* stack value */
        long index;             /* index value as an integer */
        VALUE *res;

        vp = stack;
        res = NULL;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_NUM || qisfrac(vp->v_num)) {
                math_error("Fast indexing by non-integer");
                not_reached();
        }
        index = qtoi(vp->v_num);
        if (zge31b(vp->v_num->num) || (index < 0)) {
                math_error("Index out of range for fast indexing");
                not_reached();
        }
        if (stack->v_type == V_NUM)
                qfree(stack->v_num);
        stack--;
        vp = stack;
        if (vp->v_type != V_ADDR) {
                math_error("Non-pointer for fast indexing");
                not_reached();
        }
        vp = vp->v_addr;
        switch (vp->v_type) {
        case V_OBJ:
                if (index >= vp->v_obj->o_actions->oa_count) {
                        math_error("Index out of bounds for object");
                        not_reached();
                }
                res = vp->v_obj->o_table + index;
                break;
        case V_MAT:
                m = vp->v_mat;
                if (index >= m->m_size) {
                        math_error("Index out of bounds for matrix");
                        not_reached();
                }
                res = m->m_table + index;
                break;
        case V_LIST:
                lp = vp->v_list;
                res = listfindex(lp, index);
                if (res == NULL) {
                        math_error("Index out of bounds for list");
                        not_reached();
                }
                break;
        case V_ASSOC:
                ap = vp->v_assoc;
                res = assocfindex(ap, index);
                if (res == NULL) {
                        math_error("Index out of bounds for association");
                        not_reached();
                }
                break;
        default:
                math_error("Bad variable type for fast indexing");
                not_reached();
        }
        stack->v_addr = res;
}


S_FUNC void
o_fivalue(FUNC *fp)
{
        (void) o_fiaddr(fp);
        (void) o_getvalue(fp);
}


S_FUNC void
o_sgn(FUNC *UNUSED(fp))
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
                stack->v_subtype = V_NOSUBTYPE;
                return;
        }
        sgnvalue(vp, &tmp);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_numerator(FUNC *UNUSED(fp))
{
        VALUE *vp;
        NUMBER *q;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_NUM) {
                math_error("Numerator of non-number");
                not_reached();
        }
        if ((stack->v_type == V_NUM) && qisint(vp->v_num))
                return;
        q = qnum(vp->v_num);
        if (stack->v_type == V_NUM)
                qfree(stack->v_num);
        stack->v_num = q;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_denominator(FUNC *UNUSED(fp))
{
        VALUE *vp;
        NUMBER *q;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_NUM) {
                math_error("Denominator of non-number");
                not_reached();
        }
        q = qden(vp->v_num);
        if (stack->v_type == V_NUM)
                qfree(stack->v_num);
        stack->v_num = q;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_duplicate(FUNC *UNUSED(fp))
{
        VALUE *vp;

        vp = stack++;
        *stack = *vp;
}


S_FUNC void
o_dupvalue(FUNC *UNUSED(fp))
{
        if (stack->v_type == V_ADDR)
                copyvalue(stack->v_addr, stack + 1);
        else
                copyvalue(stack, stack + 1);
        stack++;
}


S_FUNC void
o_pop(FUNC *UNUSED(fp))
{
        freevalue(stack--);
}


S_FUNC void
o_return(FUNC *UNUSED(fp))
{
}


S_FUNC void
o_jumpz(FUNC *UNUSED(fp), bool *dojump)
{
        VALUE *vp;
        int i;                  /* result of comparison */

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
                *dojump = true;
}


S_FUNC void
o_jumpnz(FUNC *UNUSED(fp), bool *dojump)
{
        VALUE *vp;
        int i;                  /* result of comparison */

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
                *dojump = true;
}


/*
 * jumpnn invokes a jump if top value points to a null value
 */
S_FUNC void
o_jumpnn(FUNC *UNUSED(fp), bool *dojump)
{
        if (stack->v_addr->v_type) {
                *dojump = true;
                stack--;
        }
}


S_FUNC void
o_condorjump(FUNC *UNUSED(fp), bool *dojump)
{
        VALUE *vp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type == V_NUM) {
                if (!qiszero(vp->v_num)) {
                        *dojump = true;
                        return;
                }
                if (stack->v_type == V_NUM)
                        qfree(stack->v_num);
                stack--;
                return;
        }
        if (testvalue(vp))
                *dojump = true;
        else
                freevalue(stack--);
}


S_FUNC void
o_condandjump(FUNC *UNUSED(fp), bool *dojump)
{
        VALUE *vp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type == V_NUM) {
                if (qiszero(vp->v_num)) {
                        *dojump = true;
                        return;
                }
                if (stack->v_type == V_NUM)
                        qfree(stack->v_num);
                stack--;
                return;
        }
        if (!testvalue(vp))
                *dojump = true;
        else
                freevalue(stack--);
}


/*
 * Compare the top two values on the stack for equality and jump if they are
 * different, popping off the top element, leaving the first one on the stack.
 * If they are equal, pop both values and do not jump.
 */
S_FUNC void
o_casejump(FUNC *UNUSED(fp), bool *dojump)
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
                *dojump = true;
        else
                freevalue(stack--);
}


S_FUNC void
o_jump(FUNC *UNUSED(fp), bool *dojump)
{
        *dojump = true;
}


S_FUNC void
o_usercall(FUNC *fp, long index, long argcount)
{
        fp = findfunc(index);
        if (fp == NULL) {
                math_error("Function \"%s\" is undefined", namefunc(index));
                not_reached();
        }
        calculate(fp, (int) argcount);
}


S_FUNC void
o_call(FUNC *UNUSED(fp), long index, long argcount)
{
        VALUE result;

        result = builtinfunc(index, (int) argcount, stack);
        while (--argcount >= 0)
                freevalue(stack--);
        stack++;
        *stack = result;
}


S_FUNC void
o_getvalue(FUNC *UNUSED(fp))
{
        if (stack->v_type == V_ADDR)
                copyvalue(stack->v_addr, stack);
}


S_FUNC void
o_cmp(FUNC *UNUSED(fp))
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


S_FUNC void
o_eq(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_ne(FUNC *UNUSED(fp))
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
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_le(FUNC *UNUSED(fp))
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

        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        if (tmp.v_type == V_NUM) {
                stack->v_num =  !qispos(tmp.v_num) ? qlink(&_qone_):
                                qlink(&_qzero_);
        } else if (tmp.v_type == V_COM) {
                stack->v_num = qlink(&_qzero_);
        } else {
                stack->v_type = V_NULL;
        }
        freevalue(&tmp);
}


S_FUNC void
o_ge(FUNC *UNUSED(fp))
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
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        if (tmp.v_type == V_NUM) {
                stack->v_num =  !qisneg(tmp.v_num) ? qlink(&_qone_):
                                qlink(&_qzero_);
        } else if (tmp.v_type == V_COM) {
                stack->v_num = qlink(&_qzero_);
        } else {
                stack->v_type = V_NULL;
        }
        freevalue(&tmp);
}


S_FUNC void
o_lt(FUNC *UNUSED(fp))
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
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        if (tmp.v_type == V_NUM) {
                stack->v_num =  qisneg(tmp.v_num) ? qlink(&_qone_):
                                qlink(&_qzero_);
        } else if (tmp.v_type == V_COM) {
                stack->v_num = qlink(&_qzero_);
        } else {
                stack->v_type = V_NULL;
        }
        freevalue(&tmp);
}


S_FUNC void
o_gt(FUNC *UNUSED(fp))
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
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        if (tmp.v_type == V_NUM) {
                stack->v_num = qispos(tmp.v_num) ? qlink(&_qone_):
                                qlink(&_qzero_);
        } else if (tmp.v_type == V_COM) {
                stack->v_num = qlink(&_qzero_);
        } else {
                stack->v_type = V_NULL;
        }
        freevalue(&tmp);
}


S_FUNC void
o_preinc(FUNC *UNUSED(fp))
{
        VALUE *vp, tmp;

        if (stack->v_type == V_OCTET) {
                if (stack->v_subtype & (V_NONEWVALUE | V_NOCOPYTO)) {
                        *stack = error_value(E_PREINC_1);
                        return;
                }
                stack->v_octet[0] = stack->v_octet[0] + 1;
                return;
        }
        if (stack->v_type != V_ADDR) {
                freevalue(stack);
                *stack = error_value(E_PREINC_2);
                return;
        }
        vp = stack->v_addr;

        if (vp->v_subtype & (V_NONEWVALUE | V_NOASSIGNTO)) {
                *stack = error_value(E_PREINC_3);
                return;
        }
        incvalue(vp, &tmp);
        freevalue(vp);
        *vp = tmp;
}


S_FUNC void
o_predec(FUNC *UNUSED(fp))
{
        VALUE *vp, tmp;

        if (stack->v_type == V_OCTET) {
                if (stack->v_subtype & (V_NONEWVALUE | V_NOCOPYTO)) {
                        *stack = error_value(E_PREDEC_1);
                        return;
                }
                --(*stack->v_octet);
                return;
        }
        if (stack->v_type != V_ADDR) {
                freevalue(stack);
                *stack = error_value(E_PREDEC_2);
                return;
        }
        vp = stack->v_addr;
        if (vp->v_subtype & (V_NONEWVALUE | V_NOASSIGNTO)) {
                *stack = error_value(E_PREDEC_3);
                return;
        }
        decvalue(vp, &tmp);
        freevalue(vp);
        *vp = tmp;
}


S_FUNC void
o_postinc(FUNC *UNUSED(fp))
{
        VALUE *vp;
        VALUE tmp;

        if (stack->v_type == V_OCTET) {
                if (stack->v_subtype & (V_NONEWVALUE | V_NOCOPYTO)) {
                        *stack++ = error_value(E_POSTINC_1);
                        stack->v_type = V_NULL;
                        return;
                }
                stack[1] = stack[0];
                stack->v_type = V_NUM;
                stack->v_subtype = V_NOSUBTYPE;
                stack->v_num = itoq((long) stack->v_octet[0]);
                stack++;
                stack->v_octet[0]++;
                return;
        }
        if (stack->v_type != V_ADDR) {
                stack[1] = *stack;
                *stack = error_value(E_POSTINC_2);
                stack++;
                return;
        }
        vp = stack->v_addr;
        if (vp->v_subtype & V_NONEWVALUE) {
                stack[1] = *stack;
                *stack = error_value(E_POSTINC_3);
                stack++;
                return;
        }
        copyvalue(vp, stack++);
        incvalue(vp, &tmp);
        freevalue(vp);
        *vp = tmp;
        stack->v_type = V_ADDR;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_addr = vp;
}


S_FUNC void
o_postdec(FUNC *UNUSED(fp))
{
        VALUE *vp;
        VALUE tmp;

        if (stack->v_type == V_OCTET) {
                if (stack->v_subtype & (V_NONEWVALUE | V_NOCOPYTO)) {
                        *stack++ = error_value(E_POSTDEC_1);
                        stack->v_type = V_NULL;
                        return;
                }
                stack[1] = stack[0];
                stack->v_type = V_NUM;
                stack->v_num = itoq((long) stack->v_octet[0]);
                stack++;
                stack->v_octet[0]--;
                return;
        }
        if (stack->v_type != V_ADDR) {
                stack[1] = *stack;
                *stack = error_value(E_POSTDEC_2);
                stack++;
                return;
        }
        vp = stack->v_addr;
        if (vp->v_subtype & (V_NONEWVALUE | V_NOASSIGNTO)) {
                stack[1] = *stack;
                *stack = error_value(E_POSTDEC_3);
                stack++;
                return;
        }
        copyvalue(vp, stack++);
        decvalue(vp, &tmp);
        freevalue(vp);
        *vp = tmp;
        stack->v_type = V_ADDR;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_addr = vp;
}


S_FUNC void
o_leftshift(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        shiftvalue(v1, v2, false, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_rightshift(FUNC *UNUSED(fp))
{
        VALUE *v1, *v2;
        VALUE tmp;

        v1 = &stack[-1];
        v2 = &stack[0];
        if (v1->v_type == V_ADDR)
                v1 = v1->v_addr;
        if (v2->v_type == V_ADDR)
                v2 = v2->v_addr;
        shiftvalue(v1, v2, true, &tmp);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_debug(FUNC *UNUSED(fp), long line)
{
        funcline = line;
        if (abortlevel >= ABORT_STATEMENT) {
                math_error("Calculation aborted at statement boundary");
                not_reached();
        }
}


S_FUNC void
o_printresult(FUNC *UNUSED(fp))
{
        VALUE *vp;

        vp = stack;

        /* firewall */
        if (vp == NULL)
                return;

        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;

        /* firewall */
        if (vp == NULL)
                return;

        if (vp->v_type != V_NULL) {
                if (conf->tab_ok)
                    math_chr('\t');
                printvalue(vp, PRINT_UNAMBIG);
                math_chr('\n');
                math_flush();
        }
        freevalue(stack--);
}


S_FUNC void
o_print(FUNC *UNUSED(fp), long flags)
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


S_FUNC void
o_printeol(FUNC *UNUSED(fp))
{
        math_chr('\n');
        math_flush();
}


S_FUNC void
o_printspace(FUNC *UNUSED(fp))
{
        math_chr(' ');
        if (conf->traceflags & TRACE_OPCODES)
                printf("\n");
}


S_FUNC void
o_printstring(FUNC *UNUSED(fp), long index)
{
        STRING *s;
        char *cp;

        s = findstring(index);
        cp = s->s_str;
        math_str(cp);
        if (conf->traceflags & TRACE_OPCODES)
                printf("\n");
        math_flush();
}


S_FUNC void
o_zero(FUNC *UNUSED(fp))
{
        stack++;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_num = qlink(&_qzero_);
}


S_FUNC void
o_one(FUNC *UNUSED(fp))
{
        stack++;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_num = qlink(&_qone_);
}


S_FUNC void
o_save(FUNC *fp)
{
        VALUE *vp;

        if (saveval || fp->f_name[1] == '*') {
                vp = stack;
                if (vp->v_type == V_ADDR)
                        vp = vp->v_addr;
                freevalue(&fp->f_savedvalue);
                copyvalue(vp, &fp->f_savedvalue);
        }
}


S_FUNC void
o_oldvalue(FUNC *UNUSED(fp))
{
        ++stack;
        stack->v_type = V_ADDR;
        stack->v_addr = &oldvalue;
}


void
o_setsaveval(FUNC *UNUSED(fp))
{
        VALUE *vp;

        vp = stack;
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        saveval = testvalue(vp);
        freevalue(stack);
}


S_FUNC void
o_quit(FUNC *fp, long index)
{
        STRING *s;
        char *cp;

        cp = NULL;
        if (index >= 0) {
                s = findstring(index);
                cp = s->s_str;
        }
        if (inputisterminal() && !strcmp(fp->f_name, "*")) {
                if (cp)
                        printf("%s\n", cp);
                hist_term();
                while (stack > stackarray) {
                        freevalue(stack--);
                }
                freevalue(stackarray);
                run_state = RUN_EXIT;
                if (calc_use_scanerr_jmpbuf != 0) {
                        longjmp(calc_scanerr_jmpbuf, 50);
                } else {
                        fprintf(stderr,
                          "calc_scanerr_jmpbuf not setup, exiting code 50\n");
                        libcalc_call_me_last();
                        exit(50);
                }
        }
        if (cp)
                printf("%s\n", cp);
        else if (conf->verbose_quit)
                printf("quit or abort executed\n");
        if (!inputisterminal() && !strcmp(fp->f_name, "*"))
                closeinput();
        go = false;
}


S_FUNC void
o_abort(FUNC *fp, long index)
{
        abort_now = true;
        o_quit(fp, index);
}


S_FUNC void
o_getepsilon(FUNC *UNUSED(fp))
{
        stack++;
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
        stack->v_num = qlink(conf->epsilon);
}


S_FUNC void
o_setepsilon(FUNC *UNUSED(fp))
{
        VALUE *vp;
        NUMBER *newep;

        vp = &stack[0];
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_NUM) {
                math_error("Non-numeric for epsilon");
                not_reached();
        }
        newep = vp->v_num;
        stack->v_num = qlink(conf->epsilon);
        setepsilon(newep);
        if (stack->v_type == V_NUM)
                qfree(newep);
        stack->v_type = V_NUM;
        stack->v_subtype = V_NOSUBTYPE;
}


S_FUNC void
o_setconfig(FUNC *UNUSED(fp))
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
                not_reached();
        }
        type = configtype(v1->v_str->s_str);
        if (type < 0) {
                math_error("Unknown config name \"%s\"",
                                v1->v_str->s_str);
                not_reached();
        }
        config_value(conf, type, &tmp);
        setconfig(type, v2);
        freevalue(stack--);
        freevalue(stack);
        *stack = tmp;
}


S_FUNC void
o_getconfig(FUNC *UNUSED(fp))
{
        int type;
        VALUE *vp;

        vp = &stack[0];
        if (vp->v_type == V_ADDR)
                vp = vp->v_addr;
        if (vp->v_type != V_STR) {
                math_error("Non-string for config");
                not_reached();
        }
        type = configtype(vp->v_str->s_str);
        if (type < 0) {
                math_error("Unknown config name \"%s\"",
                        vp->v_str->s_str);
                not_reached();
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
        fp->f_savedvalue.v_subtype = V_NOSUBTYPE;
}


/*
 * error_value - return error as a value and store type in calc_errno
 */
VALUE
error_value(int e)
{
        VALUE res;

        if (-e > 0)
                e = 0;
        if (is_valid_errnum(e) == false) {
                math_error("Error %d is not a valid errnum in %s", e, __func__);
                not_reached();
        }
        calc_errno = e;
        if (e > 0)
                errcount++;
        if (errmax >= 0 && errcount > errmax) {
                math_error("Error %d caused errcount to exceed errmax", e);
                not_reached();
        }
        res.v_type = (short) -e;
        res.v_subtype = V_NOSUBTYPE;
        return res;
}

/*
 * set_errno - return and set calc_errno if e is a valid errnum value
 */
int
set_errno(int e)
{
        int res;

        res = calc_errno;
        if (is_valid_errnum(e) == true) {
                calc_errno = e;
        }
        return res;
}


/*
 * set_errcount - return and set errcount
 */
int
set_errcount(int e)
{
        int res;

        res = errcount;
        if (e >= 0)
                errcount = e;
        return res;
}


/*
 * Fill a newly created matrix at v1 with copies of value at v2.
 */
S_FUNC void
o_initfill(FUNC *UNUSED(fp))
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
                not_reached();
        }
        s = v1->v_mat->m_size;
        vp = v1->v_mat->m_table;
        while (s-- > 0)
                copyvalue(v2, vp++);
        freevalue(stack--);
}


S_FUNC void
o_show(FUNC *fp, long arg)
{
        unsigned int size;

        switch((int) arg) {
        case 1: showbuiltins(); return;
        case 2: showglobals(); return;
        case 3: showfunctions(); return;
        case 4: showobjfuncs(); return;
        case 5: config_print(conf); putchar('\n'); return;
        case 6: showobjtypes(); return;
        case 7: showfiles(); return;
        case 8: showsizes(); return;
        case 9: showerrors(); return;
        case 10: showcustom(); return;
        case 11: shownblocks(); return;
        case 12: showconstants(); return;
        case 13: showallglobals(); return;
        case 14: showstatics(); return;
        case 15: shownumbers(); return;
        case 16: showredcdata(); return;
        case 17: showstrings(); return;
        case 18: showliterals(); return;
        }
        fp = findfunc(arg - 19);
        if (fp == NULL) {
                printf("Function not defined\n");
                return;
        }
        dumpnames = false;
        for (size = 0; size < fp->f_opcodecount; ) {
                printf("%ld: ", (long)size);
                size += dumpop(&fp->f_opcodes[size]);
        }
}


S_FUNC void
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
        printf("\tSTRING\t\t%4ld\n", (long)sizeof(STRING));
        printf("\tMATRIX\t\t%4ld\n", (long)sizeof(MATRIX));
        printf("\tLIST\t\t%4ld\n", (long)sizeof(LIST));
        printf("\tLISTELEM\t%4ld\n", (long)sizeof(LISTELEM));
        printf("\tOBJECT\t\t%4ld\n", (long)sizeof(OBJECT));
        printf("\tOBJECTACTIONS\t%4ld\n", (long)sizeof(OBJECTACTIONS));
        printf("\tASSOC\t\t%4ld\n", (long)sizeof(ASSOC));
        printf("\tASSOCELEM\t%4ld\n", (long)sizeof(ASSOCELEM));
        printf("\tBLOCK\t\t%4ld\n", (long)sizeof(BLOCK));
        printf("\tNBLOCK\t\t%4ld\n", (long)sizeof(NBLOCK));
        printf("\tCONFIG\t\t%4ld\n", (long)sizeof(CONFIG));
        printf("\tFILEIO\t\t%4ld\n", (long)sizeof(FILEIO));
        printf("\tRAND\t\t%4ld\n", (long)sizeof(RAND));
        printf("\tRANDOM\t\t%4ld\n", (long)sizeof(RANDOM));
}


/*
 * Information about each opcode.
 */
STATIC struct opcode opcodes[MAX_OPCODE+1] = {
        {{.func_nul = o_nop},
         OPNUL,
         "NOP"},                /* no operation */

        {{.func_loc = o_localaddr},
         OPLOC,
         "LOCALADDR"},  /* address of local variable */

        {{.func_glb = o_globaladdr},
         OPGLB,
         "GLOBALADDR"}, /* address of global variable */

        {{.func_par = o_paramaddr},
         OPPAR,
         "PARAMADDR"},  /* address of parameter variable */

        {{.func_loc = o_localvalue},
         OPLOC,
         "LOCALVALUE"}, /* value of local variable */

        {{.func_glb = o_globalvalue},
         OPGLB,
         "GLOBALVALUE"}, /* value of global variable */

        {{.func_par = o_paramvalue},
         OPPAR,
         "PARAMVALUE"}, /* value of parameter variable */

        {{.func_one = o_number},
         OPONE,
         "NUMBER"},     /* constant real numeric value */

        {{.func_two = o_indexaddr},
         OPTWO,
         "INDEXADDR"},  /* array index address */

        {{.func_nul = o_printresult},
         OPNUL,
         "PRINTRESULT"}, /* print result of top-level expression */

        {{.func_nul = o_assign},
         OPNUL,
         "ASSIGN"},     /* assign value to variable */

        {{.func_nul = o_add},
         OPNUL,
         "ADD"},                /* add top two values */

        {{.func_nul = o_sub},
         OPNUL,
         "SUB"},                /* subtract top two values */

        {{.func_nul = o_mul},
         OPNUL,
         "MUL"},                /* multiply top two values */

        {{.func_nul = o_div},
         OPNUL,
         "DIV"},                /* divide top two values */

        {{.func_nul = o_mod},
         OPNUL,
         "MOD"},                /* take mod of top two values */

        {{.func_nul = o_save},
         OPNUL,
         "SAVE"},       /* save value for later use */

        {{.func_nul = o_negate},
         OPNUL,
         "NEGATE"},     /* negate top value */

        {{.func_nul = o_invert},
         OPNUL,
         "INVERT"},     /* invert top value */

        {{.func_nul = o_int},
         OPNUL,
         "INT"},                /* take integer part */

        {{.func_nul = o_frac},
         OPNUL,
         "FRAC"},       /* take fraction part */

        {{.func_nul = o_numerator},
         OPNUL,
         "NUMERATOR"},  /* take numerator */

        {{.func_nul = o_denominator},
         OPNUL,
         "DENOMINATOR"}, /* take denominator */

        {{.func_nul = o_duplicate},
         OPNUL,
         "DUPLICATE"},  /* duplicate top value */

        {{.func_nul = o_pop},
         OPNUL,
         "POP"},                /* pop top value */

        {{.func_ret = o_return},
         OPRET,
         "RETURN"},     /* return value of function */

        {{.func_jmp = o_jumpz},
         OPJMP,
         "JUMPZ"},      /* jump if value zero */

        {{.func_jmp = o_jumpnz},
         OPJMP,
         "JUMPNZ"},     /* jump if value nonzero */

        {{.func_jmp = o_jump},
         OPJMP,
         "JUMP"},       /* jump unconditionally */

        {{.func_two = o_usercall},
         OPTWO,
         "USERCALL"},   /* call a user function */

        {{.func_nul = o_getvalue},
         OPNUL,
         "GETVALUE"},   /* convert address to value */

        {{.func_nul = o_eq},
         OPNUL,
         "EQ"},         /* test elements for equality */

        {{.func_nul = o_ne},
         OPNUL,
         "NE"},         /* test elements for inequality */

        {{.func_nul = o_le},
         OPNUL,
         "LE"},         /* test elements for < =  */

        {{.func_nul = o_ge},
         OPNUL,
         "GE"},         /* test elements for > =  */

        {{.func_nul = o_lt},
         OPNUL,
         "LT"},         /* test elements for < */

        {{.func_nul = o_gt},
         OPNUL,
         "GT"},         /* test elements for > */

        {{.func_nul = o_preinc},
         OPNUL,
         "PREINC"},     /* add one to variable (++x) */

        {{.func_nul = o_predec},
         OPNUL,
         "PREDEC"},     /* subtract one from variable (--x) */

        {{.func_nul = o_postinc},
         OPNUL,
         "POSTINC"},    /* add one to variable (x++) */

        {{.func_nul = o_postdec},
         OPNUL,
         "POSTDEC"},    /* subtract one from variable (x--) */

        {{.func_one = o_debug},
         OPONE,
         "DEBUG"},      /* debugging point */

        {{.func_one = o_print},
         OPONE,
         "PRINT"},      /* print value */

        {{.func_nul = o_assignpop},
         OPNUL,
         "ASSIGNPOP"},  /* assign to variable and pop it */

        {{.func_nul = o_zero},
         OPNUL,
         "ZERO"},       /* put zero on the stack */

        {{.func_nul = o_one},
         OPNUL,
         "ONE"},                /* put one on the stack */

        {{.func_nul = o_printeol},
         OPNUL,
         "PRINTEOL"},   /* print end of line */

        {{.func_nul = o_printspace},
         OPNUL,
         "PRINTSPACE"}, /* print a space */

        {{.func_one = o_printstring},
         OPONE,
         "PRINTSTR"},   /* print constant string */

        {{.func_nul = o_dupvalue},
         OPNUL,
         "DUPVALUE"},   /* duplicate value of top value */

        {{.func_nul = o_oldvalue},
         OPNUL,
         "OLDVALUE"},   /* old value from previous calc */

        {{.func_nul = o_quo},
         OPNUL,
         "QUO"},                /* integer quotient of top values */

        {{.func_nul = o_power},
         OPNUL,
         "POWER"},      /* value raised to a power */

        {{.func_one = o_quit},
         OPONE,
         "QUIT"},       /* quit program */

        {{.func_two = o_call},
         OPTWO,
         "CALL"},       /* call built-in routine */

        {{.func_nul = o_getepsilon},
         OPNUL,
         "GETEPSILON"}, /* get allowed error for calculations */

        {{.func_nul = o_and},
         OPNUL,
         "AND"},                /* arithmetic and or top two values */

        {{.func_nul = o_or},
         OPNUL,
         "OR"},         /* arithmetic or of top two values */

        {{.func_nul = o_not},
         OPNUL,
         "NOT"},                /* logical not or top value */

        {{.func_nul = o_abs},
         OPNUL,
         "ABS"},                /* absolute value of top value */

        {{.func_nul = o_sgn},
         OPNUL,
         "SGN"},                /* sign of number */

        {{.func_nul = o_isint},
         OPNUL,
         "ISINT"},      /* whether number is an integer */

        {{.func_jmp = o_condorjump},
         OPJMP,
         "CONDORJUMP"}, /* conditional or jump */

        {{.func_jmp = o_condandjump},
         OPJMP,
         "CONDANDJUMP"}, /* conditional and jump */

        {{.func_nul = o_square},
         OPNUL,
         "SQUARE"},     /* square top value */

        {{.func_one = o_string},
         OPONE,
         "STRING"},     /* string constant value */

        {{.func_nul = o_isnum},
         OPNUL,
         "ISNUM"},      /* whether value is a number */

        {{.func_nul = o_undef},
         OPNUL,
         "UNDEF"},      /* load undefined value on stack */

        {{.func_nul = o_isnull},
         OPNUL,
         "ISNULL"},     /* whether value is the null value */

        {{.func_arg = o_argvalue},
         OPARG,
         "ARGVALUE"},   /* load value of arg (parameter) n */

        {{.func_one = o_matcreate},
         OPONE,
         "MATCREATE"},  /* create matrix */

        {{.func_nul = o_ismat},
         OPNUL,
         "ISMAT"},      /* whether value is a matrix */

        {{.func_nul = o_isstr},
         OPNUL,
         "ISSTR"},      /* whether value is a string */

        {{.func_nul = o_getconfig},
         OPNUL,
         "GETCONFIG"},  /* get value of configuration parameter */

        {{.func_nul = o_leftshift},
         OPNUL,
         "LEFTSHIFT"},  /* left shift of integer */

        {{.func_nul = o_rightshift},
         OPNUL,
         "RIGHTSHIFT"}, /* right shift of integer */

        {{.func_jmp = o_casejump},
         OPJMP,
         "CASEJUMP"},   /* test case and jump if not matched */

        {{.func_nul = o_isodd},
         OPNUL,
         "ISODD"},      /* whether value is odd integer */

        {{.func_nul = o_iseven},
         OPNUL,
         "ISEVEN"},     /* whether value is even integer */

        {{.func_nul = o_fiaddr},
         OPNUL,
         "FIADDR"},     /* 'fast index' matrix address */

        {{.func_nul = o_fivalue},
         OPNUL,
         "FIVALUE"},    /* 'fast index' matrix value */

        {{.func_nul = o_isreal},
         OPNUL,
         "ISREAL"},     /* whether value is real number */

        {{.func_one = o_imaginary},
         OPONE,
         "IMAGINARY"},  /* constant imaginary numeric value */

        {{.func_nul = o_re},
         OPNUL,
         "RE"},         /* real part of complex number */

        {{.func_nul = o_im},
         OPNUL,
         "IM"},         /* imaginary part of complex number */

        {{.func_nul = o_conjugate},
         OPNUL,
         "CONJUGATE"},  /* complex conjugate */

        {{.func_one = o_objcreate},
         OPONE,
         "OBJCREATE"},  /* create object */

        {{.func_nul = o_isobj},
         OPNUL,
         "ISOBJ"},      /* whether value is an object */

        {{.func_nul = o_norm},
         OPNUL,
         "NORM"},       /* norm of value (square of abs) */

        {{.func_one = o_elemaddr},
         OPONE,
         "ELEMADDR"},   /* address of element of object */

        {{.func_one = o_elemvalue},
         OPONE,
         "ELEMVALUE"},  /* value of element of object */

        {{.func_nul = o_istype},
         OPNUL,
         "ISTYPE"},     /* whether types are the same */

        {{.func_nul = o_scale},
         OPNUL,
         "SCALE"},      /* scale value by a power of two */

        {{.func_nul = o_islist},
         OPNUL,
         "ISLIST"},     /* whether value is a list */

        {{.func_nul = o_swap},
         OPNUL,
         "SWAP"},       /* swap values of two variables */

        {{.func_nul = o_issimple},
         OPNUL,
         "ISSIMPLE"},   /* whether value is simple type */

        {{.func_nul = o_cmp},
         OPNUL,
         "CMP"},                /* compare values returning -1, 0, 1 */

        {{.func_nul = o_setconfig},
         OPNUL,
         "SETCONFIG"},  /* set configuration parameter */

        {{.func_nul = o_setepsilon},
         OPNUL,
         "SETEPSILON"}, /* set allowed error for calculations */

        {{.func_nul = o_isfile},
         OPNUL,
         "ISFILE"},     /* whether value is a file */

        {{.func_nul = o_isassoc},
         OPNUL,
         "ISASSOC"},    /* whether value is an association */

        {{.func_sti = o_nop},
         OPSTI,
         "INITSTATIC"}, /* once only code for static init */

        {{.func_one = o_eleminit},
         OPONE,
         "ELEMINIT"},   /* assign element of matrix or object */

        {{.func_nul = o_isconfig},
         OPNUL,
         "ISCONFIG"},   /* whether value is a configuration state */

        {{.func_nul = o_ishash},
         OPNUL,
         "ISHASH"},     /* whether value is a hash state */

        {{.func_nul = o_isrand},
         OPNUL,
         "ISRAND"},     /* whether value is a rand element */

        {{.func_nul = o_israndom},
         OPNUL,
         "ISRANDOM"},   /* whether value is a random element */

        {{.func_one = o_show},
         OPONE,
         "SHOW"},       /* show current state data */

        {{.func_nul = o_initfill},
         OPNUL,
         "INITFILL"},   /* initially fill matrix */

        {{.func_nul = o_assignback},
         OPNUL,
         "ASSIGNBACK"}, /* assign in reverse order */

        {{.func_nul = o_test},
         OPNUL,
         "TEST"},        /* test that value is "nonzero" */

        {{.func_nul = o_isdefined},
         OPNUL,
         "ISDEFINED"},  /* whether a string names a function */

        {{.func_nul = o_isobjtype},
         OPNUL,
         "ISOBJTYPE"},  /* whether a string names an object type */

        {{.func_nul = o_isblock},
         OPNUL,
         "ISBLK"},      /* whether value is a block */

        {{.func_nul = o_ptr},
         OPNUL,
         "PTR"},                /* octet pointer */

        {{.func_nul = o_deref},
         OPNUL,
         "DEREF"},      /* dereference an octet pointer */

        {{.func_nul = o_isoctet},
         OPNUL,
         "ISOCTET"},    /* whether a value is an octet */

        {{.func_nul = o_isptr},
         OPNUL,
         "ISPTR"},      /* whether a value is a pointer */

        {{.func_nul = o_setsaveval},
         OPNUL,
         "SAVEVAL"},    /* enable or disable saving */

        {{.func_nul = o_links},
         OPNUL,
         "LINKS"},      /* links to number or string */

        {{.func_nul = o_bit},
         OPNUL,
         "BIT"},                /* whether bit is set */

        {{.func_nul = o_comp},
         OPNUL,
         "COMP"},       /* complement value */

        {{.func_nul = o_xor},
         OPNUL,
         "XOR"},                /* xor (~) of values */

        {{.func_nul = o_highbit},
         OPNUL,
         "HIGHBIT"},    /* highbit of value */

        {{.func_nul = o_lowbit},
         OPNUL,
         "LOWBIT"},     /* lowbit of value */

        {{.func_nul = o_content},
         OPNUL,
         "CONTENT"},    /* unary hash op */

        {{.func_nul = o_hashop},
         OPNUL,
         "HASHOP"},     /* binary hash op */

        {{.func_nul = o_backslash},
         OPNUL,
         "BACKSLASH"},  /* unary backslash op */

        {{.func_nul = o_setminus},
         OPNUL,
         "SETMINUS"},   /* binary backslash op */

        {{.func_nul = o_plus},
         OPNUL,
         "PLUS"},       /* unary + op */

        {{.func_jmp = o_jumpnn},
         OPJMP,
         "JUMPNN"},     /* jump if non-null */

        {{.func_one = o_abort},
         OPONE,
         "ABORT"}       /* abort operation */
};


/*
 * Compute the result of a function by interpreting opcodes.
 * Arguments have just been pushed onto the evaluation stack.
 *
 * given:
 *      fp              function to calculate
 *      argcount        number of arguments called with
 */
void
calculate(FUNC *fp, int argcount)
{
        register unsigned long pc;      /* current pc inside function */
        register struct opcode *op;     /* current opcode pointer */
        register VALUE *locals;         /* pointer to local variables */
        long oldline;                   /* old value of line counter */
        unsigned int opnum;             /* current opcode number */
        int origargcount;               /* original number of arguments */
        unsigned int i;                 /* loop counter */
        bool dojump;                    /* true if jump is to occur */
        char *oldname;                  /* old function name being executed */
        VALUE *beginstack;              /* beginning of stack frame */
        VALUE *args;                    /* pointer to function arguments */
        VALUE retval;                   /* function return value */
        VALUE localtable[QUICKLOCALS];  /* some local variables */

        oldname = funcname;
        oldline = funcline;
        funcname = fp->f_name;
        funcline = 0;
        go = true;
        ++calc_depth;
        origargcount = argcount;
        while ((unsigned)argcount < fp->f_paramcount) {
                stack++;
                stack->v_type = V_NULL;
                stack->v_subtype = V_NOSUBTYPE;
                argcount++;
        }
        locals = localtable;
        if (fp->f_localcount > QUICKLOCALS) {
                locals = (VALUE *) malloc(sizeof(VALUE) * fp->f_localcount);
                if (locals == NULL) {
                        math_error("No memory for local variables");
                        not_reached();
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
        while (go) {
                if (abortlevel >= ABORT_OPCODE) {
                        math_error("Calculation aborted in opcode");
                        not_reached();
                }
                if (pc >= fp->f_opcodecount) {
                        math_error("Function pc out of range");
                        not_reached();
                }
                if (stack > &stackarray[MAXSTACK-3]) {
                        math_error("Evaluation stack depth exceeded");
                        not_reached();
                }
                opnum = fp->f_opcodes[pc];
                if (opnum > MAX_OPCODE) {
                        math_error("Function opcode out of range");
                        not_reached();
                }
                op = &opcodes[opnum];
                if (conf->traceflags & TRACE_OPCODES) {
                        dumpnames = false;
                        printf("%8s, pc %4ld:  ", fp->f_name, pc);
                        (void)dumpop(&fp->f_opcodes[pc]);
                }
                /*
                 * Now call the opcode routine appropriately.
                 */
                pc++;
                switch (op->o_type) {
                case OPNUL:     /* no extra arguments */
                        (*op->o_func.func_nul)(fp);
                        break;

                case OPONE:     /* one extra integer argument */
                        (*op->o_func.func_one)(fp, fp->f_opcodes[pc++]);
                        break;

                case OPTWO:     /* two extra integer arguments */
                        (*op->o_func.func_two)(fp, fp->f_opcodes[pc],
                                fp->f_opcodes[pc+1]);
                        pc += 2;
                        break;

                case OPJMP:     /* jump opcodes (one extra pointer arg) */
                        dojump = false;
                        (*op->o_func.func_jmp)(fp, &dojump);
                        if (dojump)
                                pc = fp->f_opcodes[pc];
                        else
                                pc++;
                        break;

                case OPGLB:     /* global symbol reference (pointer arg) */
                        (*op->o_func.func_glb)(fp, (GLOBAL *)(*(&fp->f_opcodes[pc])));
                        pc += PTR_SIZE;
                        break;

                case OPLOC:     /* local variable reference */
                        (*op->o_func.func_loc)(fp, locals, fp->f_opcodes[pc++]);
                        break;

                case OPPAR:     /* parameter variable reference */
                        (*op->o_func.func_par)(fp, argcount, args, fp->f_opcodes[pc++]);
                        break;

                case OPARG:     /* parameter variable reference */
                        (*op->o_func.func_arg)(fp, origargcount, args);
                        break;

                case OPRET:     /* return from function */
                        if (stack->v_type == V_ADDR)
                                copyvalue(stack->v_addr, stack);
                        for (i = 0; i < fp->f_localcount; i++)
                                freevalue(&locals[i]);
                        if (locals != localtable)
                                free(locals);
                        if (stack != &beginstack[1]) {
                                math_error("Misaligned stack");
                                not_reached();
                        }
                        if (argcount > 0) {
                                retval = *stack--;
                                while (--argcount >= 0)
                                        freevalue(stack--);
                                *++stack = retval;
                        }
                        funcname = oldname;
                        funcline = oldline;
                        --calc_depth;
                        return;

                case OPSTI:     /* static initialization code */
                        fp->f_opcodes[pc++ - 1] = OP_JUMP;
                        break;

                default:
                        math_error("Unknown opcode type: %d", op->o_type);
                        not_reached();
                }
        }
        for (i = 0; i < fp->f_localcount; i++)
                freevalue(&locals[i]);
        if (locals != localtable)
                free(locals);
        if (conf->calc_debug & CALCDBG_FUNC_QUIT)
                printf("\t\"%s\": line %ld\n", funcname, funcline);
        while (stack > beginstack)
                freevalue(stack--);
        funcname = oldname;
        funcline = oldline;
        --calc_depth;
        return;
}


/*
 * Dump an opcode at a particular address.
 * Returns the size of the opcode so that it can easily be skipped over.
 *
 * given:
 *      pc              location of the opcode
 */
int
dumpop(unsigned long *pc)
{
        GLOBAL *sp;
        unsigned long op;       /* opcode number */

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
                sp = * (GLOBAL **) pc;
                printf(" %s", sp->g_name);
                if (sp->g_filescope > SCOPE_GLOBAL)
                        printf(" %p", (void *) &sp->g_value);
                putchar('\n');
                return (1 + PTR_SIZE);
        case OP_PARAMADDR: case OP_PARAMVALUE:
                if (dumpnames)
                        printf(" %s\n", paramname((long)*pc));
                else
                        printf(" %ld\n", *pc);
                return 2;
        case OP_PRINTSTRING: case OP_STRING:
                printf(" \"%s\"\n", findstring((long)(*pc))->s_str);
                return 2;
        case OP_QUIT: case OP_ABORT:
                if ((long)(*pc) >= 0)
                    printf(" \"%s\"", findstring((long)(*pc))->s_str);
                putchar('\n');
                return 2;
        case OP_INDEXADDR:
                printf(" %ld %ld\n", pc[0], pc[1]);
                return 3;
        case OP_PRINT: case OP_JUMPZ: case OP_JUMPNZ: case OP_JUMP:
        case OP_CONDORJUMP: case OP_CONDANDJUMP: case OP_CASEJUMP:
        case OP_INITSTATIC: case OP_MATCREATE:
        case OP_SHOW: case OP_ELEMINIT: case OP_ELEMADDR:
        case OP_ELEMVALUE: case OP_JUMPNN:
                printf(" %ld\n", *pc);
                return 2;
        case OP_OBJCREATE:
                printf(" %s\n", objtypename(*pc));
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


/*
 * Free the constant numbers in a function definition
 */
void
freenumbers(FUNC *fp)
{
        unsigned long pc;
        unsigned int opnum;
        struct opcode *op;

        for (pc = 0; pc < fp->f_opcodecount; ) {
                opnum = fp->f_opcodes[pc++];
                op = &opcodes[opnum];
                switch (op->o_type) {
                case OPRET:
                case OPARG:
                case OPNUL:
                        continue;
                case OPONE:
                        switch(opnum) {
                        case OP_NUMBER:
                        case OP_IMAGINARY:
                                freeconstant(fp->f_opcodes[pc]);
                                break;
                        case OP_PRINTSTRING:
                        case OP_STRING:
                        case OP_QUIT:
                                freestringconstant(
                                    (long)fp->f_opcodes[pc]);
                                break;
                        }
                        /*FALLTHRU*/
                case OPLOC:
                case OPPAR:
                case OPJMP:
                case OPSTI:
                        pc++;
                        continue;
                case OPTWO:
                        pc += 2;
                        continue;
                case OPGLB:
                        pc += PTR_SIZE;
                        continue;
                default:
                        math_error("Unknown opcode type for freeing");
                        not_reached();
                }
        }
        if (pc != fp->f_opcodecount) {
                math_error("Incorrect opcodecount ???");
                not_reached();
        }
        trimconstants();
}


long
calclevel(void)
{
        return calc_depth - 1;
}
