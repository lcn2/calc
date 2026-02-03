/*
 * func - built-in functions implemented here
 *
 * Copyright (C) 1999-2007,2018,2021-2023,2026  David I. Bell, Landon Curt Noll and Ernest Bowen
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
 * Under source code control:   1990/02/15 01:48:15
 * File existed as early as:    before 1990
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/times.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * conditional <system> head includes
 */
#if defined(_WIN32) || defined(_WIN64)
#  include <io.h>
#  define _access access
#endif

#if !defined(FUNCLIST)

#  include "have_rusage.h"
#  if defined(HAVE_GETRUSAGE)
#    include <sys/resource.h>
#  endif

/*
 * calc local src includes
 */
#  include "value.h"
#  include "calc.h"
#  include "opcodes.h"
#  include "token.h"
#  include "label.h"
#  include "func.h"
#  include "symbol.h"
#  include "prime.h"
#  include "file.h"
#  include "custom.h"
#  include "strl.h"
#  include "blkcpy.h"
#  include "have_unused.h"
#  include "attribute.h"
#  include "errtbl.h"

#  if defined(CUSTOM)
#    define E_CUSTOM_ERROR E_NO_C_ARG
#  else
#    define E_CUSTOM_ERROR E_NO_CUSTOM
#  endif

#  include "banned.h"            /* include after system header <> includes */

/*
 * forward declarations
 */
static NUMBER *base_value(long mode, int defval);
static int strscan(char *s, int count, VALUE **vals);
static int filescan(FILEID id, int count, VALUE **vals);
static VALUE f_fsize(VALUE *vp);
static int malloced_putenv(char *str);

/*
 * external declarations
 */
extern char cmdbuf[]; /* command line expression */
extern void matrandperm(MATRIX *M);
extern void listrandperm(LIST *lp);
extern int idungetc(FILEID id, int ch);
extern LIST *associndices(ASSOC *ap, long index);
extern LIST *matindices(MATRIX *mp, long index);

/*
 * malloced environment storage
 */
#  define ENV_POOL_CHUNK (1 < 8) /* env_pool elements to allocate at a time */
struct env_pool {
    char *getenv; /* what getenv() would return, NULL => unused */
    char *putenv; /* pointer given to putenv() */
};
static int env_pool_cnt = 0;           /* number of env_pool elements in use */
static int env_pool_max = 0;           /* number of env_pool elements allocated */
static struct env_pool *e_pool = NULL; /* env_pool elements */

/*
 * constants used for hours or degrees conversion functions
 */
static HALF _nineval_[] = {9};
static HALF _twentyfourval_[] = {24};
static HALF _threesixtyval_[] = {360};
static HALF _fourhundredval_[] = {400};
static NUMBER _qtendivnine_ = {{_tenval_, 1, 0}, {_nineval_, 1, 0}, 1, NULL};
static NUMBER _qninedivten_ = {{_nineval_, 1, 0}, {_tenval_, 1, 0}, 1, NULL};
static NUMBER _qtwentyfour = {{_twentyfourval_, 1, 0}, {_oneval_, 1, 0}, 1, NULL};
static NUMBER _qthreesixty = {{_threesixtyval_, 1, 0}, {_oneval_, 1, 0}, 1, NULL};
static NUMBER _qfourhundred = {{_fourhundredval_, 1, 0}, {_oneval_, 1, 0}, 1, NULL};

/*
 * user-defined error strings
 */
static short nexterrnum = E__USERDEF;
static STRINGHEAD newerrorstr;

#endif

/*
 * arg count definitions
 */
#define IN 1024 /* maximum number of arguments */
#define FE 0x01 /* flag to indicate default epsilon argument */
#define FA 0x02 /* preserve addresses of variables */

/*
 * builtins - List of primitive built-in functions
 */

typedef union {
    char *null; /* no b_numfunc function */
    NUMBER *(*numfunc_0)(void);
#if !defined(FUNCLIST)
    NUMBER *(*numfunc_1)(NUMBER *);
    NUMBER *(*numfunc_2)(NUMBER *, NUMBER *);
    NUMBER *(*numfunc_3)(NUMBER *, NUMBER *, NUMBER *);
    NUMBER *(*numfunc_4)(NUMBER *, NUMBER *, NUMBER *, NUMBER *);
    NUMBER *(*numfunc_cnt)(int, NUMBER **);
#endif
} numfunc;

typedef union {
    char *null; /* no b_valfunc function */
    VALUE (*valfunc_0)(void);
#if !defined(FUNCLIST)
    VALUE (*valfunc_1)(VALUE *);
    VALUE (*valfunc_2)(VALUE *, VALUE *);
    VALUE (*valfunc_3)(VALUE *, VALUE *, VALUE *);
    VALUE (*valfunc_4)(VALUE *, VALUE *, VALUE *, VALUE *);
    VALUE (*valfunc_cnt)(int, VALUE **);
#endif
} valfunc;

struct builtin {
    char *b_name;      /* name of built-in function */
    short b_minargs;   /* minimum number of arguments */
    short b_maxargs;   /* maximum number of arguments */
    short b_flags;     /* special handling flags */
    short b_opcode;    /* opcode which makes the call quick */
    numfunc b_numfunc; /* routine to calculate numeric function */
    valfunc b_valfunc; /* routine to calculate general values */
    char *b_desc;      /* description of function */
};

#if !defined(FUNCLIST)

/*
 * verify_eps - verify that the eps argument is a valid error value
 *
 * The eps argument, when given to a builtin function, overrides
 * the global epsilon value.  As such, the numeric value of eps must be:
 *
 *      0 < eps < 1
 *
 * given:
 *      veps    a eps VALUE passed to a builtin function
 *
 * returns:
 *      true    veps is a non-NULL pointer to a VALUE, and
 *              VALUE type is V_NUM,
 *              eps value is 0 < eps < 1
 *      false   otherwise
 */
static bool
verify_eps(VALUE const *veps)
{
    NUMBER *eps; /* VALUE as a NUMBER */

    /*
     * firewall - must be a non-NULL VALUE ptr for a V_NUM
     */
    if (veps == NULL) {
        return false;
    }
    if (veps->v_type != V_NUM) {
        return false;
    }

    /*
     * numeric value must be valid for an epsilon value
     *
     *      0 < eps < 1
     */
    eps = veps->v_num;
    if (check_epsilon(eps) == false) {
        return false;
    }
    return true;
}

/*
 * name_newerrorstr - obtain the name string for a user-described error code
 *
 * given:
 *      errnum  errnum code to convert
 *
 * returns:
 *      != NULL ==> non-empty name string for a user-described error
 *      == NULL ==> errnum is not valid, or name string is empty, or name string not found
 */
char *
name_newerrorstr(int errnum)
{
    char *cp; /* name string lookup result */

    /*
     * firewall
     */
    if (is_valid_errnum(errnum) == false) {
        /* errnum is not a valid code */
        return NULL;
    }

    /*
     * case: errnum is not a user-described code
     */
    if (errnum < E__USERDEF || errnum > E__USERMAX) {
        /* errnum is not a user-described code */
        return NULL;
    }

    /*
     * case: errnum is outside the current range of user-described codes
     */
    if (errnum >= nexterrnum) {
        /* errnum is refers to an unassigned user-described code */
        return NULL;
    }

    /*
     * attempt to fetch the name string for a user-described error code
     */
    cp = namestr(&newerrorstr, errnum - E__USERDEF);
    if (cp == NULL || cp[0] == '\0') {
        /* name string was not found or was empty for the user-described error code */
        return NULL;
    }

    /*
     * return the name string for the user-described error code
     */
    return cp;
}

static VALUE
f_eval(VALUE *vp)
{
    FUNC *oldfunc;
    FUNC *newfunc;
    VALUE result;
    char *str;
    size_t num;
    long temp_stoponerror; /* temp value of stoponerror */

    if (vp->v_type != V_STR) {
        return error_value(E_EVAL_2);
    }
    str = vp->v_str->s_str;
    num = vp->v_str->s_len;
    switch (openstring(str, num)) {
    case -2:
        return error_value(E_EVAL_3);
    case -1:
        return error_value(E_EVAL_4);
    }
    oldfunc = curfunc;
    enterfilescope();
    temp_stoponerror = stoponerror;
    stoponerror = -1;
    if (evaluate(true)) {
        stoponerror = temp_stoponerror;
        closeinput();
        exitfilescope();
        freevalue(stack--);
        newfunc = curfunc;
        curfunc = oldfunc;
        result = newfunc->f_savedvalue;
        newfunc->f_savedvalue.v_type = V_NULL;
        newfunc->f_savedvalue.v_subtype = V_NOSUBTYPE;
        freenumbers(newfunc);
        if (newfunc != oldfunc) {
            free(newfunc);
        }
        return result;
    }
    stoponerror = temp_stoponerror;
    closeinput();
    exitfilescope();
    newfunc = curfunc;
    curfunc = oldfunc;
    freevalue(&newfunc->f_savedvalue);
    newfunc->f_savedvalue.v_type = V_NULL;
    newfunc->f_savedvalue.v_subtype = V_NOSUBTYPE;
    freenumbers(newfunc);
    if (newfunc != oldfunc) {
        free(newfunc);
    }
    return error_value(E_EVAL);
}

static VALUE
f_prompt(VALUE *vp)
{
    VALUE result;
    char *cp;
    char *newcp;
    size_t len;

    /* initialize VALUE */
    result.v_type = V_STR;
    result.v_subtype = V_NOSUBTYPE;

    openterminal();
    printvalue(vp, PRINT_SHORT);
    math_flush();
    cp = nextline();
    closeinput();
    if (cp == NULL) {
        result.v_type = V_NULL;
        return result;
    }
    if (*cp == '\0') {
        result.v_str = slink(&_nullstring_);
        return result;
    }
    len = strlen(cp);
    newcp = (char *)calloc(len + 1, 1);
    if (newcp == NULL) {
        math_error("Cannot allocate string");
        not_reached();
    }
    strlcpy(newcp, cp, len + 1);
    result.v_str = makestring(newcp);
    return result;
}

static VALUE
f_display(int count, VALUE **vals)
{
    LEN oldvalue;
    VALUE res;

    /* initialize VALUE */
    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;

    oldvalue = conf->outdigits;

    if (count > 0) {
        if (vals[0]->v_type != V_NUM || qisfrac(vals[0]->v_num) || qisneg(vals[0]->v_num) || zge31b(vals[0]->v_num->num)) {
            fprintf(stderr, "Out-of-range arg for display ignored\n");
        } else {
            conf->outdigits = (LEN)qtoi(vals[0]->v_num);
        }
    }
    res.v_num = itoq((long)oldvalue);
    return res;
}

/*ARGSUSED*/
static VALUE
f_null(int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE res;

    /* initialize VALUE */
    res.v_type = V_NULL;
    res.v_subtype = V_NOSUBTYPE;

    return res;
}

static VALUE
f_str(VALUE *vp)
{
    VALUE result;
    char *cp;

    /* initialize VALUE */
    result.v_type = V_STR;
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        result.v_str = makenewstring(vp->v_str->s_str);
        break;
    case V_NULL:
        result.v_str = slink(&_nullstring_);
        break;
    case V_OCTET:
        result.v_str = charstring(*vp->v_octet);
        break;
    case V_NUM:
        math_divertio();
        qprintnum(vp->v_num, MODE_DEFAULT, conf->outdigits);
        cp = math_getdivertedio();
        result.v_str = makestring(cp);
        break;
    case V_COM:
        math_divertio();
        comprint(vp->v_com);
        cp = math_getdivertedio();
        result.v_str = makestring(cp);
        break;
    default:
        return error_value(E_STR);
    }
    return result;
}

static VALUE
f_estr(VALUE *vp)
{
    VALUE result;
    char *cp;

    /* initialize result */
    result.v_type = V_STR;
    result.v_subtype = V_NOSUBTYPE;

    math_divertio();
    printestr(vp);
    cp = math_getdivertedio();
    result.v_str = makestring(cp);
    return result;
}

static VALUE
f_name(VALUE *vp)
{
    VALUE result;
    char *cp;
    char *name;

    /* initialize VALUE */
    result.v_type = V_STR;
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_NBLOCK:
        result.v_type = V_STR;
        result.v_str = makenewstring(vp->v_nblock->name);
        return result;
    case V_FILE:
        name = findfname(vp->v_file);
        if (name == NULL) {
            result.v_type = V_NULL;
            return result;
        }
        math_divertio();
        math_str(name);
        cp = math_getdivertedio();
        break;
    default:
        result.v_type = V_NULL;
        return result;
    }
    result.v_str = makestring(cp);
    return result;
}

static VALUE
f_poly(int count, VALUE **vals)
{
    VALUE *x;
    VALUE result, tmp;
    LIST *clist, *lp;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;
    tmp.v_subtype = V_NOSUBTYPE;

    if (vals[0]->v_type == V_LIST) {
        clist = vals[0]->v_list;
        lp = listalloc();
        while (--count > 0) {
            if ((*++vals)->v_type == V_LIST) {
                insertitems(lp, (*vals)->v_list);
            } else {
                insertlistlast(lp, *vals);
            }
        }
        if (!evalpoly(clist, lp->l_first, &result)) {
            result.v_type = V_NUM;
            result.v_num = qlink(&_qzero_);
        }
        listfree(lp);
        return result;
    }
    x = vals[--count];
    copyvalue(*vals++, &result);
    while (--count > 0) {
        mulvalue(&result, x, &tmp);
        freevalue(&result);
        addvalue(*vals++, &tmp, &result);
        freevalue(&tmp);
    }
    return result;
}

static NUMBER *
f_mne(NUMBER *val1, NUMBER *val2, NUMBER *val3)
{
    NUMBER *tmp, *res;

    tmp = qsub(val1, val2);
    res = itoq((long)!qdivides(tmp, val3));
    qfree(tmp);
    return res;
}

static NUMBER *
f_isrel(NUMBER *val1, NUMBER *val2)
{
    if (qisfrac(val1) || qisfrac(val2)) {
        math_error("Non-integer for isrel");
        not_reached();
    }
    return itoq((long)zrelprime(val1->num, val2->num));
}

static NUMBER *
f_issquare(NUMBER *vp)
{
    return itoq((long)qissquare(vp));
}

static NUMBER *
f_isprime(int count, NUMBER **vals)
{
    NUMBER *err; /* error return, NULL => use math_error */

    /* determine the way we report problems */
    if (count == 2) {
        if (qisfrac(vals[1])) {
            math_error("2nd isprime arg must be an integer");
            not_reached();
        }
        err = vals[1];
    } else {
        err = NULL;
    }

    /* firewall - must be an integer */
    if (qisfrac(vals[0])) {
        if (err) {
            return qlink(err);
        }
        math_error("non-integral arg for builtin function isprime");
        not_reached();
    }

    /* test the integer */
    switch (zisprime(vals[0]->num)) {
    case 0:
        return qlink(&_qzero_);
    case 1:
        return qlink(&_qone_);
    }

    /* error return */
    if (!err) {
        math_error("isprime argument is an odd value > 2^32");
        not_reached();
    }
    return qlink(err);
}

static NUMBER *
f_nprime(int count, NUMBER **vals)
{
    NUMBER *err;    /* error return, NULL => use math_error */
    FULL nxt_prime; /* next prime or 0 */

    /* determine the way we report problems */
    if (count == 2) {
        if (qisfrac(vals[1])) {
            math_error("2nd nextprime arg must be an integer");
            not_reached();
        }
        err = vals[1];
    } else {
        err = NULL;
    }

    /* firewall - must be an integer */
    if (qisfrac(vals[0])) {
        if (err) {
            return qlink(err);
        }
        math_error("non-integral arg 1 for builtin function nextprime");
        not_reached();
    }

    /* test the integer */
    nxt_prime = znprime(vals[0]->num);
    if (nxt_prime > 1) {
        return utoq(nxt_prime);
    } else if (nxt_prime == 0) {
        /* return 2^32+15 */
        return qlink(&_nxtprime_);
    }

    /* error return */
    if (!err) {
        math_error("nextprime arg 1 is >= 2^32");
        not_reached();
    }
    return qlink(err);
}

static NUMBER *
f_pprime(int count, NUMBER **vals)
{
    NUMBER *err;     /* error return, NULL => use math_error */
    FULL prev_prime; /* previous prime or 0 */

    /* determine the way we report problems */
    if (count == 2) {
        if (qisfrac(vals[1])) {
            math_error("2nd prevprime arg must be an integer");
            not_reached();
        }
        err = vals[1];
    } else {
        err = NULL;
    }

    /* firewall - must be an integer */
    if (qisfrac(vals[0])) {
        if (err) {
            return qlink(err);
        }
        math_error("non-integral arg 1 for builtin function prevprime");
        not_reached();
    }

    /* test the integer */
    prev_prime = zpprime(vals[0]->num);
    if (prev_prime > 1) {
        return utoq(prev_prime);
    }
    if (prev_prime == 0) {
        return qlink(&_qzero_);
    }
    /* error return */
    if (!err) {
        if (prev_prime == 0) {
            math_error("prevprime arg 1 is <= 2");
            not_reached();
        } else {
            math_error("prevprime arg 1 is >= 2^32");
            not_reached();
        }
    }
    return qlink(err);
}

static NUMBER *
f_factor(int count, NUMBER **vals)
{
    NUMBER *err;    /* error return, NULL => use math_error */
    ZVALUE limit;   /* highest prime factor in search */
    ZVALUE n;       /* number to factor */
    NUMBER *factor; /* the prime factor found */
    int res;        /* -1 => error, 0 => not found, 1 => factor found */

    /*
     * parse args
     */
    if (count == 3) {
        if (qisfrac(vals[2])) {
            math_error("3rd factor arg must be an integer");
            not_reached();
        }
        err = vals[2];
    } else {
        err = NULL;
    }
    if (count >= 2) {
        if (qisfrac(vals[1])) {
            if (err) {
                return qlink(err);
            }
            math_error("non-integral arg 2 for builtin factor");
            not_reached();
        }
        limit = vals[1]->num;
    } else {
        /* default limit is 2^32-1 */
        utoz((FULL)0xffffffff, &limit);
    }
    if (qisfrac(vals[0])) {
        if (count < 2) {
            zfree(limit);
        }
        if (err) {
            return qlink(err);
        }
        math_error("non-integral arg 1 for builtin pfactor");
        not_reached();
    }
    n = vals[0]->num;

    /*
     * find the smallest prime factor in the range
     */
    factor = qalloc();
    res = zfactor(n, limit, &(factor->num));
    if (res < 0) {
        /* error processing */
        if (err) {
            return qlink(err);
        }
        math_error("limit >= 2^32 for builtin factor");
        not_reached();
    } else if (res == 0) {
        if (count < 2) {
            zfree(limit);
        }
        /* no factor found - qalloc set factor to 1, return 1 */
        return factor;
    }

    /*
     * return the factor found
     */
    if (count < 2) {
        zfree(limit);
    }
    return factor;
}

static NUMBER *
f_pix(int count, NUMBER **vals)
{
    NUMBER *err; /* error return, NULL => use math_error */
    long value;  /* primes <= x, or 0 ==> error */

    /* determine the way we report problems */
    if (count == 2) {
        if (qisfrac(vals[1])) {
            math_error("2nd pix arg must be an integer");
            not_reached();
        }
        err = vals[1];
    } else {
        err = NULL;
    }

    /* firewall - must be an integer */
    if (qisfrac(vals[0])) {
        if (err) {
            return qlink(err);
        }
        math_error("non-integral arg 1 for builtin function pix");
        not_reached();
    }

    /* determine the number of primes <= x */
    value = zpix(vals[0]->num);
    if (value >= 0) {
        return utoq(value);
    }

    /* error return */
    if (!err) {
        math_error("pix arg 1 is >= 2^32");
        not_reached();
    }
    return qlink(err);
}

static NUMBER *
f_prevcand(int count, NUMBER **vals)
{
    ZVALUE zmodulus;
    ZVALUE zresidue;
    ZVALUE zskip;
    ZVALUE *zcount = NULL; /* ptest trial count */
    ZVALUE tmp;
    NUMBER *ans; /* candidate for primality */

    zmodulus = _one_;
    zresidue = _zero_;
    zskip = _one_;
    /*
     * check on the number of args passed and that args passed are ints
     */
    switch (count) {
    case 5:
        if (!qisint(vals[4])) {
            math_error("prevcand 5th arg must both be integer");
            not_reached();
        }
        zmodulus = vals[4]->num;
        /*FALLTHRU*/
    case 4:
        if (!qisint(vals[3])) {
            math_error("prevcand 4th arg must both be integer");
            not_reached();
        }
        zresidue = vals[3]->num;
        /*FALLTHRU*/
    case 3:
        if (!qisint(vals[2])) {
            math_error("prevcand skip arg (3rd) must be an integer or omitted");
            not_reached();
        }
        zskip = vals[2]->num;
        /*FALLTHRU*/
    case 2:
        if (!qisint(vals[1])) {
            math_error("prevcand count arg (2nd) must be an integer or omitted");
            not_reached();
        }
        zcount = &vals[1]->num;
        /*FALLTHRU*/
    case 1:
        if (!qisint(vals[0])) {
            math_error("prevcand search arg (1st) must be an integer");
            not_reached();
        }
        break;
    default:
        math_error("invalid number of args passed to prevcand");
        not_reached();
        break;
    }

    if (zcount == NULL) {
        count = 1; /* default is 1 ptest */
    } else {
        if (zge24b(*zcount)) {
            math_error("prevcand count arg (2nd) must be < 2^24");
            not_reached();
        }
        count = ztoi(*zcount);
    }

    /*
     * find the candidate
     */
    if (zprevcand(vals[0]->num, count, zskip, zresidue, zmodulus, &tmp)) {
        ans = qalloc();
        ans->num = tmp;
        return ans;
    }
    return qlink(&_qzero_);
}

static NUMBER *
f_nextcand(int count, NUMBER **vals)
{
    ZVALUE zmodulus;
    ZVALUE zresidue;
    ZVALUE zskip;
    ZVALUE *zcount = NULL; /* ptest trial count */
    ZVALUE tmp;
    NUMBER *ans; /* candidate for primality */

    zmodulus = _one_;
    zresidue = _zero_;
    zskip = _one_;
    /*
     * check on the number of args passed and that args passed are ints
     */
    switch (count) {
    case 5:
        if (!qisint(vals[4])) {
            math_error("nextcand 5th args must be integer");
            not_reached();
        }
        zmodulus = vals[4]->num;
        /*FALLTHRU*/
    case 4:
        if (!qisint(vals[3])) {
            math_error("nextcand 5th args must be integer");
            not_reached();
        }
        zresidue = vals[3]->num;
        /*FALLTHRU*/
    case 3:
        if (!qisint(vals[2])) {
            math_error("nextcand skip arg (3rd) must be an integer or omitted");
            not_reached();
        }
        zskip = vals[2]->num;
        /*FALLTHRU*/
    case 2:
        if (!qisint(vals[1])) {
            math_error("nextcand count arg (2nd) must be an integer or omitted");
            not_reached();
        }
        zcount = &vals[1]->num;
        /*FALLTHRU*/
    case 1:
        if (!qisint(vals[0])) {
            math_error("nextcand search arg (1st) must be an integer");
            not_reached();
        }
        break;
    default:
        math_error("invalid number of args passed to nextcand");
        not_reached();
    }

    /*
     * check ranges on integers passed
     */
    if (zcount == NULL) {
        count = 1; /* default is 1 ptest */
    } else {
        if (zge24b(*zcount)) {
            math_error("prevcand count arg (2nd) must be < 2^24");
            not_reached();
        }
        count = ztoi(*zcount);
    }

    /*
     * find the candidate
     */
    if (znextcand(vals[0]->num, count, zskip, zresidue, zmodulus, &tmp)) {
        ans = qalloc();
        ans->num = tmp;
        return ans;
    }
    return qlink(&_qzero_);
}

static NUMBER *
f_seed(void)
{
    return pseudo_seed();
}

static NUMBER *
f_rand(int count, NUMBER **vals)
{
    NUMBER *ans;

    /* parse args */
    switch (count) {
    case 0: /* rand() == rand(2^64) */
        /* generate an subtractive 100 shuffle pseudo-random number */
        ans = qalloc();
        zrand(SBITS, &ans->num);
        break;

    case 1: /* rand(limit) */
        if (!qisint(vals[0])) {
            math_error("rand limit must be an integer");
            not_reached();
        }
        if (zislezero(vals[0]->num)) {
            math_error("rand limit must > 0");
            not_reached();
        }
        ans = qalloc();
        zrandrange(_zero_, vals[0]->num, &ans->num);
        break;

    case 2: /* rand(low, limit) */
        /* firewall */
        if (!qisint(vals[0]) || !qisint(vals[1])) {
            math_error("rand range must be integers");
            not_reached();
        }
        ans = qalloc();
        zrandrange(vals[0]->num, vals[1]->num, &ans->num);
        break;

    default:
        math_error("invalid number of args passed to rand");
        not_reached();
        return NULL;
    }

    /* return the subtractive 100 shuffle pseudo-random number */
    return ans;
}

static NUMBER *
f_randbit(int count, NUMBER **vals)
{
    NUMBER *ans;
    ZVALUE ztmp;
    long cnt; /* bits needed or skipped */

    /* parse args */
    if (count == 0) {
        zrand(1, &ztmp);
        ans = ziszero(ztmp) ? qlink(&_qzero_) : qlink(&_qone_);
        zfree(ztmp);
        return ans;
    }

    /*
     * firewall
     */
    if (!qisint(vals[0])) {
        math_error("rand bit count must be an integer");
        not_reached();
    }
    if (zge31b(vals[0]->num)) {
        math_error("huge rand bit count");
        not_reached();
    }

    /*
     * generate an subtractive 100 shuffle pseudo-random number or skip random bits
     */
    ans = qalloc();
    cnt = ztolong(vals[0]->num);
    if (zisneg(vals[0]->num)) {
        /* skip bits */
        zrandskip(cnt);
        itoz(cnt, &ans->num);
    } else {
        /* generate bits */
        zrand(cnt, &ans->num);
    }

    /*
     * return the subtractive 100 shuffle pseudo-random number
     */
    return ans;
}

static VALUE
f_srand(int count, VALUE **vals)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_RAND;
    result.v_subtype = V_NOSUBTYPE;

    /* parse args */
    switch (count) {
    case 0:
        /* get the current subtractive 100 shuffle pseudo-random number generator state */
        result.v_rand = zsrand(NULL, NULL);
        break;

    case 1:
        switch (vals[0]->v_type) {
        case V_NUM: /* srand(seed) */
            /* seed subtractive 100 shuffle pseudo-random number generator and return previous state */
            if (!qisint(vals[0]->v_num)) {
                math_error("srand number seed must be an integer");
                not_reached();
            }
            result.v_rand = zsrand(&vals[0]->v_num->num, NULL);
            break;

        case V_RAND: /* srand(state) */
            /* set subtractive 100 shuffle pseudo-random number generator state and return previous state */
            result.v_rand = zsetrand(vals[0]->v_rand);
            break;

        case V_MAT:
            /* load subtractive 100 table and return prev state */
            result.v_rand = zsrand(NULL, vals[0]->v_mat);
            break;

        default:
            math_error("illegal type of arg passed to srand()");
            not_reached();
            break;
        }
        break;

    default:
        math_error("bad arg count to srand()");
        not_reached();
        break;
    }

    /* return the current state */
    return result;
}

static NUMBER *
f_random(int count, NUMBER **vals)
{
    NUMBER *ans;

    /* parse args */
    switch (count) {
    case 0: /* random() == random(2^64) */
        /* generate a Blum-Blum-Shub random number */
        ans = qalloc();
        zrandom(SBITS, &ans->num);
        break;

    case 1: /* random(limit) */
        if (!qisint(vals[0])) {
            math_error("random limit must be an integer");
            not_reached();
        }
        if (zislezero(vals[0]->num)) {
            math_error("random limit must > 0");
            not_reached();
        }
        ans = qalloc();
        zrandomrange(_zero_, vals[0]->num, &ans->num);
        break;

    case 2: /* random(low, limit) */
        /* firewall */
        if (!qisint(vals[0]) || !qisint(vals[1])) {
            math_error("random range must be integers");
            not_reached();
        }
        ans = qalloc();
        zrandomrange(vals[0]->num, vals[1]->num, &ans->num);
        break;

    default:
        math_error("invalid number of args passed to random");
        not_reached();
        return NULL;
    }

    /* return the Blum-Blum-Shub random number */
    return ans;
}

static NUMBER *
f_randombit(int count, NUMBER **vals)
{
    NUMBER *ans;
    ZVALUE ztmp;
    long cnt; /* bits needed or skipped */

    /* parse args */
    ztmp.len = 0; /* paranoia */
    ztmp.v = NULL;
    ztmp.sign = 0;
    if (count == 0) {
        zrandom(1, &ztmp);
        ans = ziszero(ztmp) ? qlink(&_qzero_) : qlink(&_qone_);
        zfree(ztmp);
        return ans;
    }

    /*
     * firewall
     */
    if (!qisint(vals[0])) {
        math_error("random bit count must be an integer");
        not_reached();
    }
    if (zge31b(vals[0]->num)) {
        math_error("huge random bit count");
        not_reached();
    }

    /*
     * generate a Blum-Blum-Shub random number or skip random bits
     */
    ans = qalloc();
    cnt = ztolong(vals[0]->num);
    if (zisneg(vals[0]->num)) {
        /* skip bits */
        zrandomskip(cnt);
        itoz(cnt, &ans->num);
    } else {
        /* generate bits */
        zrandom(cnt, &ans->num);
    }

    /*
     * return the Blum-Blum-Shub random number
     */
    return ans;
}

static VALUE
f_srandom(int count, VALUE **vals)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_RANDOM;
    result.v_subtype = V_NOSUBTYPE;

    /* parse args */
    switch (count) {
    case 0: /* srandom() */
        /* get the current random state */
        result.v_random = zsetrandom(NULL);
        break;

    case 1: /* srandom(seed) or srandom(state) */
        switch (vals[0]->v_type) {
        case V_NUM: /* srand(seed) */
            /* seed Blum and return previous state */
            if (!qisint(vals[0]->v_num)) {
                math_error("srandom number seed must be an integer");
                not_reached();
            }
            result.v_random = zsrandom1(vals[0]->v_num->num, true);
            break;

        case V_RANDOM: /* srandom(state) */
            /* set subtractive 100 shuffle pseudo-random number generator state and return previous state */
            result.v_random = zsetrandom(vals[0]->v_random);
            break;

        default:
            math_error("illegal type of arg passed to srandom()");
            not_reached();
            break;
        }
        break;

    case 2: /* srandom(seed, newn) */
        if (vals[0]->v_type != V_NUM || !qisint(vals[0]->v_num)) {
            math_error("srandom seed must be an integer");
            not_reached();
        }
        if (vals[1]->v_type != V_NUM || !qisint(vals[1]->v_num)) {
            math_error("srandom Blum modulus must be an integer");
            not_reached();
        }
        result.v_random = zsrandom2(vals[0]->v_num->num, vals[1]->v_num->num);
        break;

    case 4: /* srandom(seed, ip, iq, trials) */
        if (vals[0]->v_type != V_NUM || !qisint(vals[0]->v_num)) {
            math_error("srandom seed must be an integer");
            not_reached();
        }
        if (vals[1]->v_type != V_NUM || !qisint(vals[1]->v_num)) {
            math_error("srandom 2nd arg must be an integer");
            not_reached();
        }
        if (vals[2]->v_type != V_NUM || !qisint(vals[2]->v_num)) {
            math_error("srandom 3rd arg must be an integer");
            not_reached();
        }
        if (vals[3]->v_type != V_NUM || !qisint(vals[3]->v_num)) {
            math_error("srandom 4th arg must be an integer");
            not_reached();
        }
        if (zge24b(vals[3]->v_num->num)) {
            math_error("srandom trials count is excessive");
            not_reached();
        }
        result.v_random = zsrandom4(vals[0]->v_num->num, vals[1]->v_num->num, vals[2]->v_num->num, ztoi(vals[3]->v_num->num));
        break;

    default:
        math_error("bad arg count to srandom()");
        not_reached();
        break;
    }

    /* return the current state */
    return result;
}

static NUMBER *
f_primetest(int count, NUMBER **vals)
{
    /* parse args */
    switch (count) {
    case 1:
        qlink(&_qone_);
        qlink(&_qone_);
        return itoq((long)qprimetest(vals[0], &_qone_, &_qone_));
    case 2:
        qlink(&_qone_);
        return itoq((long)qprimetest(vals[0], vals[1], &_qone_));
    default:
        return itoq((long)qprimetest(vals[0], vals[1], vals[2]));
    }
}

static VALUE
f_setbit(int count, VALUE **vals)
{
    bool r;
    long index;
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    r = (count == 3) ? testvalue(vals[2]) : 1;

    if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num)) {
        return error_value(E_SETBIT_1);
    }
    if (zge31b(vals[1]->v_num->num)) {
        return error_value(E_SETBIT_2);
    }
    if (vals[0]->v_type != V_STR) {
        return error_value(E_SETBIT_3);
    }
    index = qtoi(vals[1]->v_num);
    if (stringsetbit(vals[0]->v_str, index, r)) {
        return error_value(E_SETBIT_2);
    }
    return result;
}

static VALUE
f_digit(int count, VALUE **vals)
{
    VALUE res;
    ZVALUE base;

    if (vals[0]->v_type != V_NUM) {
        return error_value(E_DGT_1);
    }

    if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num)) {
        return error_value(E_DGT_2);
    }

    if (count == 3) {
        if (vals[2]->v_type != V_NUM || qisfrac(vals[2]->v_num)) {
            return error_value(E_DGT_3);
        }
        base = vals[2]->v_num->num;
    } else {
        base = _ten_;
    }
    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;
    res.v_num = qdigit(vals[0]->v_num, vals[1]->v_num->num, base);
    if (res.v_num == NULL) {
        return error_value(E_DGT_3);
    }

    return res;
}

static VALUE
f_digits(int count, VALUE **vals)
{
    ZVALUE base;
    VALUE res;

    if (vals[0]->v_type != V_NUM) {
        return error_value(E_DGTS_1);
    }
    if (count > 1) {
        if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num) || qiszero(vals[1]->v_num) || qisunit(vals[1]->v_num)) {
            return error_value(E_DGTS_2);
        }
        base = vals[1]->v_num->num;
    } else {
        base = _ten_;
    }
    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;
    res.v_num = itoq(qdigits(vals[0]->v_num, base));
    return res;
}

static VALUE
f_places(int count, VALUE **vals)
{
    long places;
    VALUE res;

    if (vals[0]->v_type != V_NUM) {
        return error_value(E_PLCS_1);
    }
    if (count > 1) {
        if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num)) {
            return error_value(E_PLCS_2);
        }
        places = qplaces(vals[0]->v_num, vals[1]->v_num->num);
        if (places == -2) {
            return error_value(E_PLCS_2);
        }
    } else {
        places = qdecplaces(vals[0]->v_num);
    }

    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;
    res.v_num = itoq(places);
    return res;
}

static NUMBER *
f_popcnt(int count, NUMBER **vals)
{
    int bitval = 1;

    /*
     * parse args
     */
    if (count == 2 && qiszero(vals[1])) {
        bitval = 0;
    }

    /*
     * count bit values
     */
    if (qisint(vals[0])) {
        return itoq(zpopcnt(vals[0]->num, bitval));
    } else {
        return itoq(zpopcnt(vals[0]->num, bitval) + zpopcnt(vals[0]->den, bitval));
    }
}

static VALUE
f_xor(int count, VALUE **vals)
{
    NUMBER *q, *qtmp;
    STRING *s, *stmp;
    VALUE result;
    int i;
    int type;

    type = vals[0]->v_type;
    result.v_type = type;
    result.v_subtype = vals[0]->v_subtype;
    for (i = 1; i < count; i++) {
        if (vals[i]->v_type != type) {
            return error_value(E_XOR_1);
        }
    }
    switch (type) {
    case V_NUM:
        q = qlink(vals[0]->v_num);
        for (i = 1; i < count; i++) {
            qtmp = qxor(q, vals[i]->v_num);
            qfree(q);
            q = qtmp;
        }
        result.v_num = q;
        break;
    case V_STR:
        s = slink(vals[0]->v_str);
        for (i = 1; i < count; i++) {
            stmp = stringxor(s, vals[i]->v_str);
            sfree(s);
            s = stmp;
        }
        result.v_str = s;
        break;
    default:
        return error_value(E_XOR_2);
    }
    return result;
}

VALUE
minlistitems(LIST *lp)
{
    LISTELEM *ep;
    VALUE *vp;
    VALUE term;
    VALUE rel;
    VALUE min;

    /* initialize VALUEs */
    min.v_type = V_NULL;
    min.v_subtype = V_NOSUBTYPE;
    term.v_type = V_NULL;
    term.v_subtype = V_NOSUBTYPE;

    for (ep = lp->l_first; ep; ep = ep->e_next) {
        vp = &ep->e_value;
        switch (vp->v_type) {
        case V_LIST:
            term = minlistitems(vp->v_list);
            break;
        case V_OBJ:
            term = objcall(OBJ_MIN, vp, NULL_VALUE, NULL_VALUE);
            break;
        default:
            copyvalue(vp, &term);
        }
        if (min.v_type == V_NULL) {
            min = term;
            continue;
        }
        if (term.v_type == V_NULL) {
            continue;
        }
        relvalue(&term, &min, &rel);
        if (rel.v_type != V_NUM) {
            freevalue(&term);
            freevalue(&min);
            freevalue(&rel);
            return error_value(E_LISTMIN);
        }
        if (qisneg(rel.v_num)) {
            freevalue(&min);
            min = term;
        } else {
            freevalue(&term);
        }
        freevalue(&rel);
    }
    return min;
}

VALUE
maxlistitems(LIST *lp)
{
    LISTELEM *ep;
    VALUE *vp;
    VALUE term;
    VALUE rel;
    VALUE max;

    /* initialize VALUEs */
    max.v_type = V_NULL;
    max.v_subtype = V_NOSUBTYPE;
    term.v_type = V_NULL;
    term.v_subtype = V_NOSUBTYPE;

    for (ep = lp->l_first; ep; ep = ep->e_next) {
        vp = &ep->e_value;
        switch (vp->v_type) {
        case V_LIST:
            term = maxlistitems(vp->v_list);
            break;
        case V_OBJ:
            term = objcall(OBJ_MAX, vp, NULL_VALUE, NULL_VALUE);
            break;
        default:
            copyvalue(vp, &term);
        }
        if (max.v_type == V_NULL) {
            max = term;
            continue;
        }
        if (term.v_type == V_NULL) {
            continue;
        }
        relvalue(&max, &term, &rel);
        if (rel.v_type != V_NUM) {
            freevalue(&max);
            freevalue(&term);
            freevalue(&rel);
            return error_value(E_LISTMAX);
        }
        if (qisneg(rel.v_num)) {
            freevalue(&max);
            max = term;
        } else {
            freevalue(&term);
        }
        freevalue(&rel);
    }
    return max;
}

static VALUE
f_min(int count, VALUE **vals)
{
    VALUE min;
    VALUE term;
    VALUE *vp;
    VALUE rel;

    /* initialize VALUEs */
    min.v_type = V_NULL;
    min.v_subtype = V_NOSUBTYPE;
    term.v_type = V_NULL;
    term.v_subtype = V_NOSUBTYPE;

    while (count-- > 0) {
        vp = *vals++;
        switch (vp->v_type) {
        case V_LIST:
            term = minlistitems(vp->v_list);
            break;
        case V_OBJ:
            term = objcall(OBJ_MIN, vp, NULL_VALUE, NULL_VALUE);
            break;
        default:
            copyvalue(vp, &term);
        }
        if (min.v_type == V_NULL) {
            min = term;
            continue;
        }
        if (term.v_type == V_NULL) {
            continue;
        }
        if (term.v_type < 0) {
            freevalue(&min);
            return term;
        }
        relvalue(&term, &min, &rel);
        if (rel.v_type != V_NUM) {
            freevalue(&min);
            freevalue(&term);
            freevalue(&rel);
            return error_value(E_MIN);
        }
        if (qisneg(rel.v_num)) {
            freevalue(&min);
            min = term;
        } else {
            freevalue(&term);
        }
        freevalue(&rel);
    }
    return min;
}

static VALUE
f_max(int count, VALUE **vals)
{
    VALUE max;
    VALUE term;
    VALUE *vp;
    VALUE rel;

    /* initialize VALUEs */
    max.v_type = V_NULL;
    max.v_subtype = V_NOSUBTYPE;
    term.v_type = V_NULL;
    term.v_subtype = V_NOSUBTYPE;

    while (count-- > 0) {
        vp = *vals++;
        switch (vp->v_type) {
        case V_LIST:
            term = maxlistitems(vp->v_list);
            break;
        case V_OBJ:
            term = objcall(OBJ_MAX, vp, NULL_VALUE, NULL_VALUE);
            break;
        default:
            copyvalue(vp, &term);
        }
        if (max.v_type == V_NULL) {
            max = term;
            continue;
        }
        if (term.v_type == V_NULL) {
            continue;
        }
        if (term.v_type < 0) {
            freevalue(&max);
            return term;
        }
        relvalue(&max, &term, &rel);
        if (rel.v_type != V_NUM) {
            freevalue(&max);
            freevalue(&term);
            freevalue(&rel);
            return error_value(E_MAX);
        }
        if (qisneg(rel.v_num)) {
            freevalue(&max);
            max = term;
        } else {
            freevalue(&term);
        }
        freevalue(&rel);
    }
    return max;
}

static NUMBER *
f_gcd(int count, NUMBER **vals)
{
    NUMBER *val, *tmp;

    val = qqabs(*vals);
    while (--count > 0) {
        tmp = qgcd(val, *++vals);
        qfree(val);
        val = tmp;
    }
    return val;
}

static NUMBER *
f_lcm(int count, NUMBER **vals)
{
    NUMBER *val, *tmp;

    val = qqabs(*vals);
    while (--count > 0) {
        tmp = qlcm(val, *++vals);
        qfree(val);
        val = tmp;
        if (qiszero(val)) {
            break;
        }
    }
    return val;
}

static VALUE
f_hash(int count, VALUE **vals)
{
    QCKHASH hash;
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    hash = QUICKHASH_BASIS;
    while (count-- > 0) {
        hash = hashvalue(*vals++, hash);
    }
    result.v_num = utoq((FULL)hash);
    return result;
}

VALUE
sumlistitems(LIST *lp)
{
    LISTELEM *ep;
    VALUE *vp;
    VALUE term;
    VALUE tmp;
    VALUE sum;

    /* initialize VALUEs */
    term.v_type = V_NULL;
    term.v_subtype = V_NOSUBTYPE;
    tmp.v_type = V_NULL;
    tmp.v_subtype = V_NOSUBTYPE;
    sum.v_type = V_NULL;
    sum.v_subtype = V_NOSUBTYPE;

    for (ep = lp->l_first; ep; ep = ep->e_next) {
        vp = &ep->e_value;
        switch (vp->v_type) {
        case V_LIST:
            term = sumlistitems(vp->v_list);
            break;
        case V_OBJ:
            term = objcall(OBJ_SUM, vp, NULL_VALUE, NULL_VALUE);
            break;
        default:
            addvalue(&sum, vp, &tmp);
            freevalue(&sum);
            if (tmp.v_type < 0) {
                return tmp;
            }
            sum = tmp;
            continue;
        }
        addvalue(&sum, &term, &tmp);
        freevalue(&sum);
        freevalue(&term);
        sum = tmp;
        if (sum.v_type < 0) {
            break;
        }
    }
    return sum;
}

static VALUE
f_sum(int count, VALUE **vals)
{
    VALUE tmp;
    VALUE sum;
    VALUE term;
    VALUE *vp;

    /* initialize VALUEs */
    tmp.v_type = V_NULL;
    tmp.v_subtype = V_NOSUBTYPE;
    sum.v_type = V_NULL;
    sum.v_subtype = V_NOSUBTYPE;
    term.v_type = V_NULL;
    term.v_subtype = V_NOSUBTYPE;
    while (count-- > 0) {
        vp = *vals++;
        switch (vp->v_type) {
        case V_LIST:
            term = sumlistitems(vp->v_list);
            break;
        case V_OBJ:
            term = objcall(OBJ_SUM, vp, NULL_VALUE, NULL_VALUE);
            break;
        default:
            addvalue(&sum, vp, &tmp);
            freevalue(&sum);
            if (tmp.v_type < 0) {
                return tmp;
            }
            sum = tmp;
            continue;
        }
        addvalue(&sum, &term, &tmp);
        freevalue(&term);
        freevalue(&sum);
        sum = tmp;
        if (sum.v_type < 0) {
            break;
        }
    }
    return sum;
}

static VALUE
f_avg(int count, VALUE **vals)
{
    VALUE tmp;
    VALUE sum;
    VALUE div;
    long n;

    /* initialize VALUEs */
    tmp.v_type = V_NULL;
    tmp.v_subtype = V_NOSUBTYPE;
    sum.v_type = V_NULL;
    sum.v_subtype = V_NOSUBTYPE;
    div.v_type = V_NULL;
    div.v_subtype = V_NOSUBTYPE;

    n = 0;
    while (count-- > 0) {
        if ((*vals)->v_type == V_LIST) {
            addlistitems((*vals)->v_list, &sum);
            n += countlistitems((*vals++)->v_list);
        } else {
            addvalue(&sum, *vals++, &tmp);
            freevalue(&sum);
            sum = tmp;
            n++;
        }
        if (sum.v_type < 0) {
            return sum;
        }
    }
    if (n < 2) {
        return sum;
    }
    div.v_num = itoq(n);
    div.v_type = V_NUM;
    div.v_subtype = V_NOSUBTYPE;
    divvalue(&sum, &div, &tmp);
    freevalue(&sum);
    qfree(div.v_num);
    return tmp;
}

static VALUE
f_fact(VALUE *vp)
{
    VALUE res;

    /* initialize VALUE */
    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;

    if (vp->v_type == V_OBJ) {
        return objcall(OBJ_FACT, vp, NULL_VALUE, NULL_VALUE);
    }
    if (vp->v_type != V_NUM) {
        math_error("Non-real argument for fact()");
        not_reached();
    }
    res.v_num = qfact(vp->v_num);
    return res;
}

static VALUE
f_hmean(int count, VALUE **vals)
{
    VALUE sum, tmp1, tmp2;
    long n = 0;

    /* initialize VALUEs */
    sum.v_type = V_NULL;
    sum.v_subtype = V_NOSUBTYPE;
    tmp1.v_type = V_NULL;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp2.v_type = V_NULL;
    tmp2.v_subtype = V_NOSUBTYPE;

    while (count-- > 0) {
        if ((*vals)->v_type == V_LIST) {
            addlistinv((*vals)->v_list, &sum);
            n += countlistitems((*vals++)->v_list);
        } else {
            invertvalue(*vals++, &tmp1);
            addvalue(&sum, &tmp1, &tmp2);
            freevalue(&tmp1);
            freevalue(&sum);
            sum = tmp2;
            n++;
        }
    }
    if (n == 0) {
        return sum;
    }
    tmp1.v_type = V_NUM;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp1.v_num = itoq(n);
    divvalue(&tmp1, &sum, &tmp2);
    qfree(tmp1.v_num);
    freevalue(&sum);
    return tmp2;
}

static NUMBER *
f_hnrmod(NUMBER *val1, NUMBER *val2, NUMBER *val3, NUMBER *val4)
{
    ZVALUE answer; /* v mod h*2^n+r */
    NUMBER *res;   /* v mod h*2^n+r */

    /*
     * firewall
     */
    if (qisfrac(val1)) {
        math_error("1st arg of hnrmod (v) must be an integer");
        not_reached();
    }
    if (qisfrac(val2) || qisneg(val2) || qiszero(val2)) {
        math_error("2nd arg of hnrmod (h) must be an integer > 0");
        not_reached();
    }
    if (qisfrac(val3) || qisneg(val3) || qiszero(val3)) {
        math_error("3rd arg of hnrmod (n) must be an integer > 0");
        not_reached();
    }
    if (qisfrac(val4) || !zisabsleone(val4->num)) {
        math_error("4th arg of hnrmod (r) must be -1, 0 or 1");
        not_reached();
    }

    /*
     * perform the val1 mod (val2 * 2^val3 + val4) operation
     */
    zhnrmod(val1->num, val2->num, val3->num, val4->num, &answer);

    /*
     * return the answer
     */
    res = qalloc();
    res->num = answer;
    return res;
}

VALUE
ssqlistitems(LIST *lp)
{
    LISTELEM *ep;
    VALUE *vp;
    VALUE term;
    VALUE tmp;
    VALUE sum;

    /* initialize VALUEs */
    term.v_type = V_NULL;
    term.v_subtype = V_NOSUBTYPE;
    tmp.v_type = V_NULL;
    tmp.v_subtype = V_NOSUBTYPE;
    sum.v_type = V_NULL;
    sum.v_subtype = V_NOSUBTYPE;

    for (ep = lp->l_first; ep; ep = ep->e_next) {
        vp = &ep->e_value;
        if (vp->v_type == V_LIST) {
            term = ssqlistitems(vp->v_list);
        } else {
            squarevalue(vp, &term);
        }
        addvalue(&sum, &term, &tmp);
        freevalue(&sum);
        freevalue(&term);
        sum = tmp;
        if (sum.v_type < 0) {
            break;
        }
    }
    return sum;
}

static VALUE
f_ssq(int count, VALUE **vals)
{
    VALUE tmp;
    VALUE sum;
    VALUE term;
    VALUE *vp;

    /* initialize VALUEs */
    tmp.v_type = V_NULL;
    tmp.v_subtype = V_NOSUBTYPE;
    sum.v_type = V_NULL;
    sum.v_subtype = V_NOSUBTYPE;
    term.v_type = V_NULL;
    term.v_subtype = V_NOSUBTYPE;
    while (count-- > 0) {
        vp = *vals++;
        if (vp->v_type == V_LIST) {
            term = ssqlistitems(vp->v_list);
        } else {
            squarevalue(vp, &term);
        }
        addvalue(&sum, &term, &tmp);
        freevalue(&term);
        freevalue(&sum);
        sum = tmp;
        if (sum.v_type < 0) {
            break;
        }
    }
    return sum;
}

static NUMBER *
f_ismult(NUMBER *val1, NUMBER *val2)
{
    return itoq((long)qdivides(val1, val2));
}

static NUMBER *
f_meq(NUMBER *val1, NUMBER *val2, NUMBER *val3)
{
    NUMBER *tmp, *res;

    tmp = qsub(val1, val2);
    res = itoq((long)qdivides(tmp, val3));
    qfree(tmp);
    return res;
}

static VALUE
f_exp(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    NUMBER *q;
    COMPLEX *c;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_EXP_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute e^x to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        q = qexp(vals[0]->v_num, eps);
        if (q == NULL) {
            return error_value(E_EXP_3);
        }
        result.v_num = q;
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_exp(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_EXP_3);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_EXP_2);
    }
    return result;
}

static VALUE
f_ln(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX ctmp, *c;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_LN_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute natural logarithm to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_LN_3);
        }
        if (!qisneg(vals[0]->v_num)) {
            result.v_num = qln(vals[0]->v_num, err);
            result.v_type = V_NUM;
            return result;
        }
        ctmp.real = vals[0]->v_num;
        ctmp.imag = qlink(&_qzero_);
        ctmp.links = 1;
        c = c_ln(&ctmp, err);
        break;
    case V_COM:
        if (ciszero(vals[0]->v_com)) {
            return error_value(E_LN_3);
        }
        c = c_ln(vals[0]->v_com, err);
        break;
    default:
        return error_value(E_LN_2);
    }

    /* determine if we will return a numeric or complex value */
    result.v_type = V_COM;
    result.v_com = c;
    if (cisreal(c)) {
        result.v_num = c_to_q(c, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_log(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX ctmp, *c;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_LOG_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute logarithm base 10 to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_LOG_5);
        }
        if (!qisneg(vals[0]->v_num)) {
            result.v_num = qlog(vals[0]->v_num, err);
            result.v_type = V_NUM;
            return result;
        }
        ctmp.real = vals[0]->v_num;
        ctmp.imag = qlink(&_qzero_);
        ctmp.links = 1;
        c = c_log(&ctmp, err);
        break;
    case V_COM:
        if (ciszero(vals[0]->v_com)) {
            return error_value(E_LOG_5);
        }
        c = c_log(vals[0]->v_com, err);
        break;
    default:
        return error_value(E_LOG_2);
    }
    if (c == NULL) {
        return error_value(E_LOG_3);
    }

    /* determine if we will return a numeric or complex value */
    result.v_type = V_COM;
    result.v_com = c;
    if (cisreal(c)) {
        result.v_num = c_to_q(c, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_log2(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX ctmp, *c;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_LOG2_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute base 2 logarithm to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_LOG2_4);
        }
        if (!qisneg(vals[0]->v_num) && !qiszero(vals[0]->v_num)) {
            result.v_num = qlog2(vals[0]->v_num, err);
            result.v_type = V_NUM;
            return result;
        }
        ctmp.real = vals[0]->v_num;
        ctmp.imag = qlink(&_qzero_);
        ctmp.links = 1;
        c = c_log2(&ctmp, err);
        break;
    case V_COM:
        if (ciszero(vals[0]->v_com)) {
            return error_value(E_LOG2_4);
        }
        c = c_log2(vals[0]->v_com, err);
        break;
    default:
        return error_value(E_LOG2_2);
    }
    if (c == NULL) {
        return error_value(E_LOG2_3);
    }

    /* determine if we will return a numeric or complex value */
    result.v_type = V_COM;
    result.v_com = c;
    if (cisreal(c)) {
        result.v_num = c_to_q(c, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_logn(int count, VALUE **vals)
{
    VALUE result;                    /* return value */
    COMPLEX ctmp;                    /* intermediate COMPLEX temporary value */
    COMPLEX *p_cval;                 /* pointer to a COMPLEX value */
    NUMBER *err;                     /* epsilon error value */
    bool ln_of_x_is_complex = false; /* taking to value of a COMPLEX x */
    COMPLEX *ln_x_c;                 /* ln(x) where ln_of_x_is_complex is true */
    NUMBER *ln_x_r;                  /* ln(x) where ln_of_x_is_complex is false */
    bool ln_of_n_is_complex = false; /* taking to value of a COMPLEX base n */
    COMPLEX *ln_n_c;                 /* ln(n) where ln_of_n_is_complex is true */
    NUMBER *ln_n_r;                  /* ln(n) where ln_of_n_is_complex is false */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 3) {
        if (verify_eps(vals[2]) == false) {
            return error_value(E_LOGN_1);
        }
        err = vals[2]->v_num;
    }

    /*
     * special case: x and n are both integer powers of 2 and n log 2 != 0
     *
     * If this is the case, we return the integer formed by log2(n) / log2(x).
     */
    ln_x_r = qalloc();
    ln_n_r = qalloc();
    if (vals[0]->v_type == V_NUM && qispowerof2(vals[0]->v_num, &ln_x_r) == true) {
        if (vals[1]->v_type == V_NUM && qispowerof2(vals[1]->v_num, &ln_n_r) == true) {
            if (!qiszero(ln_n_r)) {
                result.v_num = qqdiv(ln_x_r, ln_n_r);
                if (result.v_num == NULL) {
                    return error_value(E_LOGN_4);
                }
                result.v_type = V_NUM;
                qfree(ln_x_r);
                qfree(ln_n_r);
                return result;
            } else {
                qfree(ln_x_r);
                qfree(ln_n_r);
                return error_value(E_LOGN_4);
            }
        }
    }
    qfree(ln_x_r);
    qfree(ln_n_r);

    /*
     * take the natural log of x (value)
     *
     * Look for the case where the natural log of x complex is a real.
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_LOGN_6);
        }
        if (qisneg(vals[0]->v_num)) {
            ctmp.real = vals[0]->v_num;
            ctmp.imag = qlink(&_qzero_);
            ctmp.links = 1;
            ln_x_c = c_ln(&ctmp, err);
            if (ln_x_c == NULL) {
                return error_value(E_LOGN_3);
            }
            if (cisreal(ln_x_c)) {
                ln_x_r = c_to_q(ln_x_c, true);
            } else {
                ln_of_x_is_complex = true;
            }
        } else {
            ln_x_c = NULL; /* avoid ln_x_c may be uninitialized warning later on */
            ln_x_r = qln(vals[0]->v_num, err);
            if (ln_x_r == NULL) {
                return error_value(E_LOGN_3);
            }
        }
        break;
    case V_COM:
        if (ciszero(vals[0]->v_com)) {
            return error_value(E_LOGN_6);
        }
        ln_x_c = c_ln(vals[0]->v_com, err);
        if (ln_x_c == NULL) {
            return error_value(E_LOGN_3);
        }
        if (cisreal(ln_x_c)) {
            ln_x_r = c_to_q(ln_x_c, true);
        } else {
            ln_of_x_is_complex = true;
        }
        break;
    default:
        return error_value(E_LOGN_2);
    }

    /*
     * take the natural log of n (base)
     *
     * Look for the case where the natural log of n complex is a real.
     * Also report an error if the case where the natural log of n is zero.
     */
    switch (vals[1]->v_type) {
    case V_NUM:
        if (qiszero(vals[1]->v_num)) {
            if (ln_of_x_is_complex == true) {
                comfree(ln_x_c);
            } else {
                qfree(ln_x_r);
            }
            return error_value(E_LOGN_4);
        }
        if (qisneg(vals[1]->v_num)) {
            ctmp.real = vals[1]->v_num;
            ctmp.imag = qlink(&_qzero_);
            ctmp.links = 1;
            ln_n_c = c_ln(&ctmp, err);
            if (ln_n_c == NULL) {
                if (ln_of_x_is_complex == true) {
                    comfree(ln_x_c);
                } else {
                    qfree(ln_x_r);
                }
                return error_value(E_LOGN_4);
            }
            if (ciszero(ln_n_c)) {
                comfree(ln_n_c);
                if (ln_of_x_is_complex == true) {
                    comfree(ln_x_c);
                } else {
                    qfree(ln_x_r);
                }
                return error_value(E_LOGN_4);
            }
            if (cisreal(ln_n_c)) {
                ln_n_r = c_to_q(ln_n_c, true);
            } else {
                ln_of_n_is_complex = true;
            }
        } else {
            ln_n_r = qln(vals[1]->v_num, err);
            if (ln_n_r == NULL) {
                if (ln_of_x_is_complex == true) {
                    comfree(ln_x_c);
                } else {
                    qfree(ln_x_r);
                }
                return error_value(E_LOGN_4);
            }
            if (qiszero(ln_n_r)) {
                qfree(ln_n_r);
                if (ln_of_x_is_complex == true) {
                    comfree(ln_x_c);
                } else {
                    qfree(ln_x_r);
                }
                return error_value(E_LOGN_4);
            }
        }
        break;
    case V_COM:
        if (ciszero(vals[1]->v_com)) {
            if (ln_of_x_is_complex == true) {
                comfree(ln_x_c);
            } else {
                qfree(ln_x_r);
            }
            return error_value(E_LOGN_4);
        }
        ln_n_c = c_ln(vals[1]->v_com, err);
        if (ln_n_c == NULL) {
            if (ln_of_x_is_complex == true) {
                comfree(ln_x_c);
            } else {
                qfree(ln_x_r);
            }
            return error_value(E_LOGN_4);
        }
        if (ciszero(ln_n_c)) {
            comfree(ln_n_c);
            if (ln_of_x_is_complex == true) {
                comfree(ln_x_c);
            } else {
                qfree(ln_x_r);
            }
            return error_value(E_LOGN_4);
        }
        if (cisreal(ln_n_c)) {
            ln_n_r = c_to_q(ln_n_c, true);
        } else {
            ln_of_n_is_complex = true;
        }
        break;
    default:
        if (ln_of_x_is_complex == true) {
            comfree(ln_x_c);
        } else {
            qfree(ln_x_r);
        }
        return error_value(E_LOGN_5);
    }

    /*
     * compute ln(x) / ln(n)
     */
    if (ln_of_x_is_complex == true) {
        if (ln_of_n_is_complex == true) {

            /*
             * case: ln(x) is COMPLEX, ln(n) is COMPLEX
             */
            p_cval = c_div(ln_x_c, ln_n_c);
            comfree(ln_x_c);
            comfree(ln_n_c);
            if (p_cval == NULL) {
                return error_value(E_LOGN_3);
            }
            /* check for COMPLEX or NUMBER division */
            if (cisreal(p_cval)) {
                /* ln(x) / ln(n) was NUMBER, not COMPLEX */
                result.v_num = c_to_q(p_cval, true);
                result.v_type = V_NUM;
            } else {
                /* ln(x) / ln(n) is COMPLEX */
                result.v_type = V_COM;
                result.v_com = p_cval;
            }

        } else {

            /*
             * case: ln(x) is COMPLEX, ln(n) is NUMBER
             */
            p_cval = c_divq(ln_x_c, ln_n_r);
            comfree(ln_x_c);
            qfree(ln_n_r);
            if (p_cval == NULL) {
                return error_value(E_LOGN_3);
            }
            /* check for COMPLEX or NUMBER division */
            if (cisreal(p_cval)) {
                /* ln(x) / ln(n) was NUMBER, not COMPLEX */
                result.v_num = c_to_q(p_cval, true);
                result.v_type = V_NUM;
            } else {
                /* ln(x) / ln(n) is COMPLEX */
                result.v_type = V_COM;
                result.v_com = p_cval;
            }
        }

    } else {
        if (ln_of_n_is_complex == true) {

            /*
             * case: ln(x) is NUMBER, ln(n) is COMPLEX
             */
            /* convert ln_x_r into COMPLEX so we can divide */
            ctmp.real = ln_x_r;
            ctmp.imag = qlink(&_qzero_);
            ctmp.links = 1;
            p_cval = c_div(&ctmp, ln_n_c);
            comfree(&ctmp);
            comfree(ln_n_c);
            if (p_cval == NULL) {
                return error_value(E_LOGN_3);
            }
            /* check for COMPLEX or NUMBER division */
            if (cisreal(p_cval)) {
                /* ln(x) / ln(n) was NUMBER, not COMPLEX */
                result.v_num = c_to_q(p_cval, true);
                result.v_type = V_NUM;
            } else {
                /* ln(x) / ln(n) is COMPLEX result */
                result.v_type = V_COM;
                result.v_com = p_cval;
            }

        } else {

            /*
             * case: ln(x) is NUMBER, ln(n) is NUMBER
             */
            result.v_num = qqdiv(ln_x_r, ln_n_r);
            qfree(ln_x_r);
            qfree(ln_n_r);
            if (result.v_com == NULL) {
                return error_value(E_LOGN_3);
            }
            /* ln(x) / ln(n) is NUMBER result */
            result.v_type = V_NUM;
        }
    }

    /* return the resulting logarithm */
    return result;
}

static VALUE
f_cos(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_COS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute cosine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qcos(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_cos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_COS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_COS_2);
    }
    return result;
}

/*
 * f_d2r - convert degrees to radians
 */
static VALUE
f_d2r(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    NUMBER *pidiv180;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_D2R_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute argument*(pi/180) to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        pidiv180 = qpidiv180(eps);
        result.v_num = qmul(vals[0]->v_num, pidiv180);
        result.v_type = V_NUM;
        qfree(pidiv180);
        break;
    case V_COM:
        pidiv180 = qpidiv180(eps);
        result.v_com = c_mulq(vals[0]->v_com, pidiv180);
        result.v_type = V_COM;
        qfree(pidiv180);
        break;
    default:
        return error_value(E_D2R_2);
    }
    return result;
}

/*
 * f_r2d - convert radians to degrees
 */
static VALUE
f_r2d(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    NUMBER *pidiv180;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_R2D_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute argument/(pi/180) to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        pidiv180 = qpidiv180(eps);
        result.v_num = qqdiv(vals[0]->v_num, pidiv180);
        result.v_type = V_NUM;
        qfree(pidiv180);
        break;
    case V_COM:
        pidiv180 = qpidiv180(eps);
        result.v_com = c_divq(vals[0]->v_com, pidiv180);
        result.v_type = V_COM;
        qfree(pidiv180);
        break;
    default:
        return error_value(E_R2D_2);
    }
    return result;
}

/*
 * f_d2r - convert gradians to radians
 */
static VALUE
f_g2r(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    NUMBER *pidiv200;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_G2R_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute argument*(pi/200) to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        pidiv200 = qpidiv200(eps);
        result.v_num = qmul(vals[0]->v_num, pidiv200);
        result.v_type = V_NUM;
        qfree(pidiv200);
        break;
    case V_COM:
        pidiv200 = qpidiv200(eps);
        result.v_com = c_mulq(vals[0]->v_com, pidiv200);
        result.v_type = V_COM;
        qfree(pidiv200);
        break;
    default:
        return error_value(E_G2R_2);
    }
    return result;
}

/*
 * f_r2g - convert radians to gradians
 */
static VALUE
f_r2g(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    NUMBER *pidiv200;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_R2G_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute argument/(pi/200) to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        pidiv200 = qpidiv200(eps);
        result.v_num = qqdiv(vals[0]->v_num, pidiv200);
        result.v_type = V_NUM;
        qfree(pidiv200);
        break;
    case V_COM:
        pidiv200 = qpidiv200(eps);
        result.v_com = c_divq(vals[0]->v_com, pidiv200);
        result.v_type = V_COM;
        qfree(pidiv200);
        break;
    default:
        return error_value(E_R2G_2);
    }
    return result;
}

/*
 * f_d2g - convert degrees to gradians
 *
 * NOTE: The epsilon (vals[1]->v_num) argument is ignored.
 */
/*ARGSUSED*/
static VALUE
f_d2g(int UNUSED(count), VALUE **vals)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /* NOTE: the epsilon (vals[1]->v_num) argument is ignored */

    /* calculate argument * (10/9) */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qmul(vals[0]->v_num, &_qtendivnine_);
        result.v_type = V_NUM;
        break;
    case V_COM:
        result.v_com = c_mulq(vals[0]->v_com, &_qtendivnine_);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_D2G_1);
    }
    return result;
}

/*
 * f_g2d - convert gradians to degrees
 *
 * NOTE: The epsilon (vals[1]->v_num) argument is ignored.
 */
/*ARGSUSED*/
static VALUE
f_g2d(int UNUSED(count), VALUE **vals)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /* NOTE: the epsilon (vals[1]->v_num) argument is ignored */

    /* calculate argument * (9/10) */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qmul(vals[0]->v_num, &_qninedivten_);
        result.v_type = V_NUM;
        break;
    case V_COM:
        result.v_com = c_mulq(vals[0]->v_com, &_qninedivten_);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_G2D_1);
    }
    return result;
}

static VALUE
f_sin(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_SIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute sine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qsin(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_sin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_SIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_SIN_2);
    }
    return result;
}

static VALUE
f_tan(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use err VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_TAN_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute tangent to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qtan(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_tan(vals[0]->v_com, err);
        if (c == NULL) {
            return error_value(E_TAN_5);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_TAN_2);
    }
    return result;
}

static VALUE
f_cot(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use err VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_COT_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute cotangent to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_COT_5);
        }
        result.v_num = qcot(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        if (ciszero(vals[0]->v_com)) {
            return error_value(E_COT_5);
        }
        c = c_cot(vals[0]->v_com, err);
        if (c == NULL) {
            return error_value(E_COT_6);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_COT_2);
    }
    return result;
}

static VALUE
f_sec(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use err VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_SEC_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute secant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qsec(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_sec(vals[0]->v_com, err);
        if (c == NULL) {
            return error_value(E_SEC_5);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_SEC_2);
    }
    return result;
}

static VALUE
f_csc(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use err VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_CSC_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute cosecant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_CSC_5);
        }
        result.v_num = qcsc(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        if (ciszero(vals[0]->v_com)) {
            return error_value(E_CSC_5);
        }
        c = c_csc(vals[0]->v_com, err);
        if (c == NULL) {
            return error_value(E_CSC_6);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_CSC_2);
    }
    return result;
}

static VALUE
f_sinh(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    NUMBER *q;
    COMPLEX *c;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_SINH_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute hyperbolic sine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        q = qsinh(vals[0]->v_num, eps);
        if (q == NULL) {
            return error_value(E_SINH_3);
        }
        result.v_num = q;
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_sinh(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_SINH_3);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_SINH_2);
    }
    return result;
}

static VALUE
f_cosh(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    NUMBER *q;
    COMPLEX *c;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_COSH_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute hyperbolic cosine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        q = qcosh(vals[0]->v_num, eps);
        if (q == NULL) {
            return error_value(E_COSH_3);
        }
        result.v_num = q;
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_cosh(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_COSH_3);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_COSH_2);
    }
    return result;
}

static VALUE
f_tanh(int count, VALUE **vals)
{
    VALUE result;
    VALUE tmp1, tmp2;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp2.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_TANH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute hyperbolic tangent to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qtanh(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        tmp1.v_type = V_COM;
        tmp1.v_com = c_sinh(vals[0]->v_com, err);
        if (tmp1.v_com == NULL) {
            return error_value(E_TANH_3);
        }
        tmp2.v_type = V_COM;
        tmp2.v_com = c_cosh(vals[0]->v_com, err);
        if (tmp2.v_com == NULL) {
            comfree(tmp1.v_com);
            return error_value(E_TANH_4);
        }
        divvalue(&tmp1, &tmp2, &result);
        comfree(tmp1.v_com);
        comfree(tmp2.v_com);
        break;
    default:
        return error_value(E_TANH_2);
    }
    return result;
}

static VALUE
f_coth(int count, VALUE **vals)
{
    VALUE result;
    VALUE tmp1, tmp2;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp2.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_COTH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute hyperbolic cotangent to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_DIVBYZERO);
        }
        result.v_num = qcoth(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        tmp1.v_type = V_COM;
        tmp1.v_com = c_cosh(vals[0]->v_com, err);
        if (tmp1.v_com == NULL) {
            return error_value(E_COTH_3);
        }
        tmp2.v_type = V_COM;
        tmp2.v_com = c_sinh(vals[0]->v_com, err);
        if (tmp2.v_com == NULL) {
            comfree(tmp1.v_com);
            return error_value(E_COTH_4);
        }
        divvalue(&tmp1, &tmp2, &result);
        comfree(tmp1.v_com);
        comfree(tmp2.v_com);
        break;
    default:
        return error_value(E_COTH_2);
    }
    return result;
}

static VALUE
f_sech(int count, VALUE **vals)
{
    VALUE result;
    VALUE tmp;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;
    tmp.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_SECH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute hyperbolic secant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qsech(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        tmp.v_type = V_COM;
        tmp.v_com = c_cosh(vals[0]->v_com, err);
        if (tmp.v_com == NULL) {
            return error_value(E_SECH_3);
        }
        invertvalue(&tmp, &result);
        comfree(tmp.v_com);
        break;
    default:
        return error_value(E_SECH_2);
    }
    return result;
}

static VALUE
f_csch(int count, VALUE **vals)
{
    VALUE result;
    VALUE tmp;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;
    tmp.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_CSCH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute hyperbolic cosecant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_DIVBYZERO);
        }
        result.v_num = qcsch(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        tmp.v_type = V_COM;
        tmp.v_com = c_sinh(vals[0]->v_com, err);
        if (tmp.v_com == NULL) {
            return error_value(E_CSCH_3);
        }
        invertvalue(&tmp, &result);
        comfree(tmp.v_com);
        break;
    default:
        return error_value(E_CSCH_2);
    }
    return result;
}

static VALUE
f_atan(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ATAN_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse tangent to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qatan(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        tmp = c_atan(vals[0]->v_com, err);
        if (tmp == NULL) {
            return error_value(E_ATAN_3);
        }
        result.v_type = V_COM;
        result.v_com = tmp;
        if (cisreal(tmp)) {
            result.v_num = c_to_q(tmp, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_ATAN_2);
    }
    return result;
}

static VALUE
f_acot(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACOT_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse cotangent to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qacot(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        tmp = c_acot(vals[0]->v_com, err);
        if (tmp == NULL) {
            return error_value(E_ACOT_3);
        }
        result.v_type = V_COM;
        result.v_com = tmp;
        if (cisreal(tmp)) {
            result.v_num = c_to_q(tmp, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_ACOT_2);
    }
    return result;
}

static VALUE
f_asin(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ASIN_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse sine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qasin(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_type = V_COM;
            result.v_com = c_asin(tmp, err);
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_asin(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ASIN_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ASIN_3);
    }
    if (result.v_type == V_COM && cisreal(result.v_com)) {
        result.v_num = c_to_q(result.v_com, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_acos(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACOS_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse cosine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qacos(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_type = V_COM;
            result.v_com = c_acos(tmp, err);
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_acos(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ACOS_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ACOS_3);
    }
    if (result.v_type == V_COM && cisreal(result.v_com)) {
        result.v_num = c_to_q(result.v_com, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_asec(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ASEC_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse secant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_ASEC_3);
        }
        result.v_num = qasec(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_com = c_asec(tmp, err);
            result.v_type = V_COM;
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_asec(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ASEC_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ASEC_3);
    }
    if (result.v_type == V_COM) {
        if (cisreal(result.v_com)) {
            result.v_num = c_to_q(result.v_com, true);
            result.v_type = V_NUM;
        }
    }
    return result;
}

static VALUE
f_acsc(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACSC_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse cosecant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_ACSC_3);
        }
        result.v_num = qacsc(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_com = c_acsc(tmp, err);
            result.v_type = V_COM;
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_acsc(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ACSC_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ACSC_3);
    }
    if (result.v_type == V_COM) {
        if (cisreal(result.v_com)) {
            result.v_num = c_to_q(result.v_com, true);
            result.v_type = V_NUM;
        }
    }
    return result;
}

static VALUE
f_asinh(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ASINH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse hyperbolic sine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qasinh(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        tmp = c_asinh(vals[0]->v_com, err);
        if (tmp == NULL) {
            return error_value(E_ASINH_3);
        }
        result.v_type = V_COM;
        result.v_com = tmp;
        if (cisreal(tmp)) {
            result.v_num = c_to_q(tmp, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_ASINH_2);
    }
    return result;
}

static VALUE
f_acosh(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACOSH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse hyperbolic cosine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qacosh(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_com = c_acosh(tmp, err);
            result.v_type = V_COM;
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_acosh(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ACOSH_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ACOSH_3);
    }
    if (result.v_type == V_COM && cisreal(result.v_com)) {
        result.v_num = c_to_q(result.v_com, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_atanh(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ATANH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse hyperbolic tangent to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qatanh(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_com = c_atanh(tmp, err);
            result.v_type = V_COM;
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_atanh(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ATANH_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ATANH_3);
    }
    if (result.v_type == V_COM) {
        if (cisreal(result.v_com)) {
            result.v_num = c_to_q(result.v_com, true);
            result.v_type = V_NUM;
        }
    }
    return result;
}

static VALUE
f_acoth(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACOTH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse hyperbolic cotangent to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qacoth(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_com = c_acoth(tmp, err);
            result.v_type = V_COM;
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_acoth(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ACOTH_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ACOTH_3);
    }
    if (result.v_type == V_COM) {
        if (cisreal(result.v_com)) {
            result.v_num = c_to_q(result.v_com, true);
            result.v_type = V_NUM;
        }
    }
    return result;
}

static VALUE
f_asech(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_SECH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse hyperbolic secant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_ASECH_3);
        }
        result.v_num = qasech(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_com = c_asech(tmp, err);
            result.v_type = V_COM;
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_asech(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ASECH_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ASECH_3);
    }
    if (result.v_type == V_COM) {
        if (cisreal(result.v_com)) {
            result.v_num = c_to_q(result.v_com, true);
            result.v_type = V_NUM;
        }
    }
    return result;
}

static VALUE
f_acsch(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *tmp;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACSCH_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute inverse hyperbolic cosecant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_ACSCH_3);
        }
        result.v_num = qacsch(vals[0]->v_num, err);
        result.v_type = V_NUM;
        if (result.v_num == NULL) {
            tmp = comalloc();
            qfree(tmp->real);
            tmp->real = qlink(vals[0]->v_num);
            result.v_com = c_acsch(tmp, err);
            result.v_type = V_COM;
            comfree(tmp);
        }
        break;
    case V_COM:
        result.v_com = c_acsch(vals[0]->v_com, err);
        result.v_type = V_COM;
        break;
    default:
        return error_value(E_ACSCH_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_ACSCH_3);
    }
    if (result.v_type == V_COM) {
        if (cisreal(result.v_com)) {
            result.v_num = c_to_q(result.v_com, true);
            result.v_type = V_NUM;
        }
    }
    return result;
}

static VALUE
f_gd(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    COMPLEX *tmp;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_GD_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute Gudermannian function to a given error tolerance
     */
    result.v_type = V_COM;
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            result.v_type = V_NUM;
            result.v_num = qlink(&_qzero_);
            return result;
        }
        tmp = comalloc();
        qfree(tmp->real);
        tmp->real = qlink(vals[0]->v_num);
        result.v_com = c_gd(tmp, eps);
        comfree(tmp);
        break;
    case V_COM:
        result.v_com = c_gd(vals[0]->v_com, eps);
        break;
    default:
        return error_value(E_GD_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_GD_3);
    }
    if (cisreal(result.v_com)) {
        result.v_num = c_to_q(result.v_com, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_agd(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    COMPLEX *tmp;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given as a NUMBER and != 0.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num)) {
            return error_value(E_AGD_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse Gudermannian function to a given error tolerance
     */
    result.v_type = V_COM;
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            result.v_type = V_NUM;
            result.v_num = qlink(&_qzero_);
            return result;
        }
        tmp = comalloc();
        qfree(tmp->real);
        tmp->real = qlink(vals[0]->v_num);
        result.v_com = c_agd(tmp, eps);
        comfree(tmp);
        break;
    case V_COM:
        result.v_com = c_agd(vals[0]->v_com, eps);
        break;
    default:
        return error_value(E_AGD_2);
    }
    if (result.v_com == NULL) {
        return error_value(E_AGD_3);
    }
    if (cisreal(result.v_com)) {
        result.v_num = c_to_q(result.v_com, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_comb(VALUE *v1, VALUE *v2)
{
    long n;
    VALUE result;
    VALUE tmp1, tmp2, div;

    if (v2->v_type != V_NUM || qisfrac(v2->v_num)) {
        return error_value(E_COMB_1);
    }
    result.v_subtype = V_NOSUBTYPE;
    result.v_type = V_NUM;
    if (qisneg(v2->v_num)) {
        result.v_num = qlink(&_qzero_);
        return result;
    }
    if (qiszero(v2->v_num)) {
        result.v_num = qlink(&_qone_);
        return result;
    }
    if (qisone(v2->v_num)) {
        copyvalue(v1, &result);
        return result;
    }
    if (v1->v_type == V_NUM) {
        result.v_num = qcomb(v1->v_num, v2->v_num);
        if (result.v_num == NULL) {
            return error_value(E_COMB_2);
        }
        return result;
    }
    if (zge24b(v2->v_num->num)) {
        return error_value(E_COMB_2);
    }
    n = qtoi(v2->v_num);
    copyvalue(v1, &result);
    decvalue(v1, &tmp1);
    div.v_type = V_NUM;
    div.v_subtype = V_NOSUBTYPE;
    div.v_num = qlink(&_qtwo_);
    n--;
    for (;;) {
        mulvalue(&result, &tmp1, &tmp2);
        freevalue(&result);
        divvalue(&tmp2, &div, &result);
        freevalue(&tmp2);
        if (--n == 0 || !testvalue(&result) || result.v_type < 0) {
            freevalue(&tmp1);
            freevalue(&div);
            return result;
        }
        decvalue(&tmp1, &tmp2);
        freevalue(&tmp1);
        tmp1 = tmp2;
        incvalue(&div, &tmp2);
        freevalue(&div);
        div = tmp2;
    }
}

static VALUE
f_bern(VALUE *vp)
{
    VALUE res;

    if (vp->v_type != V_NUM || qisfrac(vp->v_num)) {
        return error_value(E_BERN);
    }

    res.v_subtype = V_NOSUBTYPE;
    res.v_type = V_NUM;
    res.v_num = qbern(vp->v_num->num);
    if (res.v_num == NULL) {
        return error_value(E_BERN);
    }
    return res;
}

static VALUE
f_freebern(void)
{
    VALUE res;

    qfreebern();
    res.v_type = V_NULL;
    res.v_subtype = V_NOSUBTYPE;
    return res;
}

static VALUE
f_euler(VALUE *vp)
{
    VALUE res;

    if (vp->v_type != V_NUM || qisfrac(vp->v_num)) {
        return error_value(E_EULER);
    }
    res.v_subtype = V_NOSUBTYPE;
    res.v_type = V_NUM;
    res.v_num = qeuler(vp->v_num->num);
    if (res.v_num == NULL) {
        return error_value(E_EULER);
    }
    return res;
}

static VALUE
f_freeeuler(void)
{
    VALUE res;

    qfreeeuler();
    res.v_type = V_NULL;
    res.v_subtype = V_NOSUBTYPE;
    return res;
}

static VALUE
f_catalan(VALUE *vp)
{
    VALUE res;

    if (vp->v_type != V_NUM || qisfrac(vp->v_num) || zge31b(vp->v_num->num)) {
        return error_value(E_CTLN);
    }
    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;
    res.v_num = qcatalan(vp->v_num);
    if (res.v_num == NULL) {
        return error_value(E_CTLN);
    }
    return res;
}

static VALUE
f_arg(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given as a NUMBER and != 0.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num)) {
            return error_value(E_ARG_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute argument (the angle or phase) of a complex number to a given error tolerance
     */
    result.v_type = V_NUM;
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qisneg(vals[0]->v_num)) {
            result.v_num = qpi(err);
        } else {
            result.v_num = qlink(&_qzero_);
        }
        break;
    case V_COM:
        c = vals[0]->v_com;
        if (ciszero(c)) {
            result.v_num = qlink(&_qzero_);
        } else {
            result.v_num = qatan2(c->imag, c->real, err);
        }
        break;
    default:
        return error_value(E_ARG_2);
    }
    return result;
}

static NUMBER *
f_legtoleg(NUMBER *val1, NUMBER *val2)
{
    /* qlegtoleg() performs the val2 != 0 check */
    return qlegtoleg(val1, val2, false);
}

static NUMBER *
f_trunc(int count, NUMBER **vals)
{
    NUMBER *val;

    val = qlink(&_qzero_);
    if (count == 2) {
        val = vals[1];
    }
    return qtrunc(*vals, val);
}

static VALUE
f_bround(int count, VALUE **vals)
{
    VALUE tmp1, tmp2, res;

    /* initialize VALUEs */
    res.v_subtype = V_NOSUBTYPE;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp2.v_subtype = V_NOSUBTYPE;

    if (count > 2) {
        tmp2 = *vals[2];
    } else {
        tmp2.v_type = V_NULL;
    }
    if (count > 1) {
        tmp1 = *vals[1];
    } else {
        tmp1.v_type = V_NULL;
    }
    broundvalue(vals[0], &tmp1, &tmp2, &res);
    return res;
}

static VALUE
f_appr(int count, VALUE **vals)
{
    VALUE tmp1, tmp2, res;

    /* initialize VALUEs */
    res.v_subtype = V_NOSUBTYPE;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp2.v_subtype = V_NOSUBTYPE;

    if (count > 2) {
        copyvalue(vals[2], &tmp2);
    } else {
        tmp2.v_type = V_NULL;
    }
    if (count > 1) {
        copyvalue(vals[1], &tmp1);
    } else {
        tmp1.v_type = V_NULL;
    }
    apprvalue(vals[0], &tmp1, &tmp2, &res);
    freevalue(&tmp1);
    freevalue(&tmp2);
    return res;
}

static VALUE
f_round(int count, VALUE **vals)
{
    VALUE tmp1, tmp2, res;

    /* initialize VALUEs */
    res.v_subtype = V_NOSUBTYPE;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp2.v_subtype = V_NOSUBTYPE;

    if (count > 2) {
        tmp2 = *vals[2];
    } else {
        tmp2.v_type = V_NULL;
    }
    if (count > 1) {
        tmp1 = *vals[1];
    } else {
        tmp1.v_type = V_NULL;
    }
    roundvalue(vals[0], &tmp1, &tmp2, &res);
    return res;
}

static NUMBER *
f_btrunc(int count, NUMBER **vals)
{
    NUMBER *val;

    val = qlink(&_qzero_);
    if (count == 2) {
        val = vals[1];
    }
    return qbtrunc(*vals, val);
}

static VALUE
f_quo(int count, VALUE **vals)
{
    VALUE tmp, res;

    /* initialize VALUEs */
    res.v_subtype = V_NOSUBTYPE;
    tmp.v_subtype = V_NOSUBTYPE;

    if (count > 2) {
        tmp = *vals[2];
    } else {
        tmp.v_type = V_NULL;
    }
    quovalue(vals[0], vals[1], &tmp, &res);
    return res;
}

static VALUE
f_mod(int count, VALUE **vals)
{
    VALUE tmp, res;

    /* initialize VALUEs */
    res.v_subtype = V_NOSUBTYPE;
    tmp.v_subtype = V_NOSUBTYPE;

    if (count > 2) {
        tmp = *vals[2];
    } else {
        tmp.v_type = V_NULL;
    }
    modvalue(vals[0], vals[1], &tmp, &res);
    return res;
}

static VALUE
f_quomod(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4, *v5;
    VALUE result;
    long rnd;
    bool res;
    short s3, s4; /* to preserve subtypes of v3, v4 */

    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];
    v4 = vals[3];

    if (v3->v_type != V_ADDR || v4->v_type != V_ADDR || v3->v_addr == v4->v_addr) {
        return error_value(E_QUOMOD_1);
    }
    if (count == 5) {
        v5 = vals[4];
        if (v5->v_type == V_ADDR) {
            v5 = v5->v_addr;
        }
        if (v5->v_type != V_NUM || qisfrac(v5->v_num) || qisneg(v5->v_num) || zge31b(v5->v_num->num)) {
            return error_value(E_QUOMOD_2);
        }
        rnd = qtoi(v5->v_num);
    } else {
        rnd = conf->quomod;
    }

    if (v1->v_type == V_ADDR) {
        v1 = v1->v_addr;
    }
    if (v2->v_type == V_ADDR) {
        v2 = v2->v_addr;
    }
    v3 = v3->v_addr;
    v4 = v4->v_addr;

    if (v1->v_type != V_NUM || v2->v_type != V_NUM || (v3->v_type != V_NUM && v3->v_type != V_NULL) ||
        (v4->v_type != V_NUM && v4->v_type != V_NULL)) {
        return error_value(E_QUOMOD_2);
    }

    s3 = v3->v_subtype;
    s4 = v4->v_subtype;

    if ((s3 | s4) & V_NOASSIGNTO) {
        return error_value(E_QUOMOD_3);
    }

    freevalue(v3);
    freevalue(v4);

    v3->v_type = V_NUM;
    v4->v_type = V_NUM;

    v3->v_subtype = s3;
    v4->v_subtype = s4;

    res = qquomod(v1->v_num, v2->v_num, &v3->v_num, &v4->v_num, rnd);
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;
    result.v_num = res ? qlink(&_qone_) : qlink(&_qzero_);
    return result;
}

static VALUE
f_d2dms(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4, *v5;
    NUMBER *tmp, *tmp_m;
    VALUE result;
    long rnd;
    short s2, s3, s4; /* to preserve subtypes of v2, v3, v4 */

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];
    v4 = vals[3];

    /* determine rounding mode */
    if (count == 5) {
        v5 = vals[4];
        if (v5->v_type == V_ADDR) {
            v5 = v5->v_addr;
        }
        if (v5->v_type != V_NUM || qisfrac(v5->v_num) || qisneg(v5->v_num) || zge31b(v5->v_num->num)) {
            return error_value(E_D2DMS_4);
        }
        rnd = qtoi(v5->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v2->v_type != V_ADDR || v3->v_type != V_ADDR || v4->v_type != V_ADDR) {
        return error_value(E_D2DMS_1);
    }
    if (v1->v_type == V_ADDR) {
        v1 = v1->v_addr;
    }
    v2 = v2->v_addr;
    v3 = v3->v_addr;
    v4 = v4->v_addr;
    if (v1->v_type != V_NUM || (v2->v_type != V_NUM && v2->v_type != V_NULL) || (v3->v_type != V_NUM && v3->v_type != V_NULL) ||
        (v4->v_type != V_NUM && v4->v_type != V_NULL)) {
        return error_value(E_D2DMS_2);
    }

    /* remember arg subtypes */
    s2 = v2->v_subtype;
    s3 = v3->v_subtype;
    s4 = v4->v_subtype;
    if ((s2 | s3 | s4) & V_NOASSIGNTO) {
        return error_value(E_D2DMS_3);
    }

    /* free old args that will be modified */
    freevalue(v2);
    freevalue(v3);
    freevalue(v4);

    /* set args that will be modified */
    v2->v_type = V_NUM;
    v3->v_type = V_NUM;
    v4->v_type = V_NUM;

    /* restore arg subtypes */
    v2->v_subtype = s2;
    v3->v_subtype = s3;
    v4->v_subtype = s4;

    /*
     * calculate the normalized return value
     *
     * return_value = mod(degs, 360, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(v1->v_num, &_qthreesixty, rnd);

    /*
     * integer number of degrees
     *
     * d = int(return_value);
     */
    v2->v_num = qint(result.v_num);

    /*
     * integer number of minutes
     *
     * tmp = return_value - d;
     * tmp_m = tmp * 60;
     * free(tmp);
     * m = int(tmp_m);
     */
    tmp = qsub(result.v_num, v2->v_num);
    tmp_m = qmuli(tmp, 60);
    qfree(tmp);
    v3->v_num = qint(tmp_m);

    /*
     * number of seconds
     *
     * tmp = tmp_m - m;
     * free(tmp_m);
     * s = tmp * 60;
     * free(tmp);
     */
    tmp = qsub(tmp_m, v3->v_num);
    qfree(tmp_m);
    v4->v_num = qmuli(tmp, 60);
    qfree(tmp);

    /*
     * return the normalized value previously calculated
     */
    return result;
}

static VALUE
f_d2dm(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4;
    NUMBER *tmp;
    VALUE result;
    long rnd;
    short s2, s3; /* to preserve subtypes of v2, v3 */

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];

    /* determine rounding mode */
    if (count == 4) {
        v4 = vals[3];
        if (v4->v_type == V_ADDR) {
            v4 = v4->v_addr;
        }
        if (v4->v_type != V_NUM || qisfrac(v4->v_num) || qisneg(v4->v_num) || zge31b(v4->v_num->num)) {
            return error_value(E_D2DM_4);
        }
        rnd = qtoi(v4->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v2->v_type != V_ADDR || v3->v_type != V_ADDR) {
        return error_value(E_D2DM_1);
    }
    if (v1->v_type == V_ADDR) {
        v1 = v1->v_addr;
    }
    v2 = v2->v_addr;
    v3 = v3->v_addr;
    if (v1->v_type != V_NUM || (v2->v_type != V_NUM && v2->v_type != V_NULL) || (v3->v_type != V_NUM && v3->v_type != V_NULL)) {
        return error_value(E_D2DM_2);
    }

    /* remember arg subtypes */
    s2 = v2->v_subtype;
    s3 = v3->v_subtype;
    if ((s2 | s3) & V_NOASSIGNTO) {
        return error_value(E_D2DM_3);
    }

    /* free old args that will be modified */
    freevalue(v2);
    freevalue(v3);

    /* set args that will be modified */
    v2->v_type = V_NUM;
    v3->v_type = V_NUM;

    /* restore arg subtypes */
    v2->v_subtype = s2;
    v3->v_subtype = s3;

    /*
     * calculate the normalized return value
     *
     * return_value = mod(degs, 360, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(v1->v_num, &_qthreesixty, rnd);

    /*
     * integer number of degrees
     *
     * d = int(return_value);
     */
    v2->v_num = qint(result.v_num);

    /*
     * integer number of minutes
     *
     * tmp = return_value - d;
     * m = tmp * 60;
     * free(tmp);
     */
    tmp = qsub(result.v_num, v2->v_num);
    v3->v_num = qmuli(tmp, 60);
    qfree(tmp);

    /*
     * return the normalized value previously calculated
     */
    return result;
}

static VALUE
f_g2gms(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4, *v5;
    NUMBER *tmp, *tmp_m;
    VALUE result;
    long rnd;
    short s2, s3, s4; /* to preserve subtypes of v2, v3, v4 */

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];
    v4 = vals[3];

    /* determine rounding mode */
    if (count == 5) {
        v5 = vals[4];
        if (v5->v_type == V_ADDR) {
            v5 = v5->v_addr;
        }
        if (v5->v_type != V_NUM || qisfrac(v5->v_num) || qisneg(v5->v_num) || zge31b(v5->v_num->num)) {
            return error_value(E_G2GMS_4);
        }
        rnd = qtoi(v5->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v2->v_type != V_ADDR || v3->v_type != V_ADDR || v4->v_type != V_ADDR) {
        return error_value(E_G2GMS_1);
    }
    if (v1->v_type == V_ADDR) {
        v1 = v1->v_addr;
    }
    v2 = v2->v_addr;
    v3 = v3->v_addr;
    v4 = v4->v_addr;
    if (v1->v_type != V_NUM || (v2->v_type != V_NUM && v2->v_type != V_NULL) || (v3->v_type != V_NUM && v3->v_type != V_NULL) ||
        (v4->v_type != V_NUM && v4->v_type != V_NULL)) {
        return error_value(E_G2GMS_2);
    }

    /* remember arg subtypes */
    s2 = v2->v_subtype;
    s3 = v3->v_subtype;
    s4 = v4->v_subtype;
    if ((s2 | s3 | s4) & V_NOASSIGNTO) {
        return error_value(E_G2GMS_3);
    }

    /* free old args that will be modified */
    freevalue(v2);
    freevalue(v3);
    freevalue(v4);

    /* set args that will be modified */
    v2->v_type = V_NUM;
    v3->v_type = V_NUM;
    v4->v_type = V_NUM;

    /* restore arg subtypes */
    v2->v_subtype = s2;
    v3->v_subtype = s3;
    v4->v_subtype = s4;

    /*
     * calculate the normalized return value
     *
     * return_value = mod(grads, 400, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(v1->v_num, &_qfourhundred, rnd);

    /*
     * integer number of gradians
     *
     * g = int(return_value);
     */
    v2->v_num = qint(result.v_num);

    /*
     * integer number of minutes
     *
     * tmp = return_value - g;
     * tmp_m = tmp * 60;
     * free(tmp);
     * m = int(tmp_m);
     */
    tmp = qsub(result.v_num, v2->v_num);
    tmp_m = qmuli(tmp, 60);
    qfree(tmp);
    v3->v_num = qint(tmp_m);

    /*
     * number of seconds
     *
     * tmp = tmp_m - m;
     * free(tmp_m);
     * s = tmp * 60;
     * free(tmp);
     */
    tmp = qsub(tmp_m, v3->v_num);
    qfree(tmp_m);
    v4->v_num = qmuli(tmp, 60);
    qfree(tmp);

    /*
     * return the normalized value previously calculated
     */
    return result;
}

static VALUE
f_g2gm(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4;
    NUMBER *tmp;
    VALUE result;
    long rnd;
    short s2, s3; /* to preserve subtypes of v2, v3 */

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];

    /* determine rounding mode */
    if (count == 4) {
        v4 = vals[3];
        if (v4->v_type == V_ADDR) {
            v4 = v4->v_addr;
        }
        if (v4->v_type != V_NUM || qisfrac(v4->v_num) || qisneg(v4->v_num) || zge31b(v4->v_num->num)) {
            return error_value(E_G2GM_4);
        }
        rnd = qtoi(v4->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v2->v_type != V_ADDR || v3->v_type != V_ADDR) {
        return error_value(E_G2GM_1);
    }
    if (v1->v_type == V_ADDR) {
        v1 = v1->v_addr;
    }
    v2 = v2->v_addr;
    v3 = v3->v_addr;
    if (v1->v_type != V_NUM || (v2->v_type != V_NUM && v2->v_type != V_NULL) || (v3->v_type != V_NUM && v3->v_type != V_NULL)) {
        return error_value(E_G2GM_2);
    }

    /* remember arg subtypes */
    s2 = v2->v_subtype;
    s3 = v3->v_subtype;
    if ((s2 | s3) & V_NOASSIGNTO) {
        return error_value(E_G2GM_3);
    }

    /* free old args that will be modified */
    freevalue(v2);
    freevalue(v3);

    /* set args that will be modified */
    v2->v_type = V_NUM;
    v3->v_type = V_NUM;

    /* restore arg subtypes */
    v2->v_subtype = s2;
    v3->v_subtype = s3;

    /*
     * calculate the normalized return value
     *
     * return_value = mod(grads, 400, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(v1->v_num, &_qfourhundred, rnd);

    /*
     * integer number of gradians
     *
     * g = int(return_value);
     */
    v2->v_num = qint(result.v_num);

    /*
     * integer number of minutes
     *
     * tmp = return_value - g;
     * m = tmp * 60;
     * free(tmp);
     */
    tmp = qsub(result.v_num, v2->v_num);
    v3->v_num = qmuli(tmp, 60);
    qfree(tmp);

    /*
     * return the normalized value previously calculated
     */
    return result;
}

static VALUE
f_h2hms(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4, *v5;
    NUMBER *tmp, *tmp_m;
    VALUE result;
    long rnd;
    short s2, s3, s4; /* to preserve subtypes of v2, v3, v4 */

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];
    v4 = vals[3];

    /* determine rounding mode */
    if (count == 5) {
        v5 = vals[4];
        if (v5->v_type == V_ADDR) {
            v5 = v5->v_addr;
        }
        if (v5->v_type != V_NUM || qisfrac(v5->v_num) || qisneg(v5->v_num) || zge31b(v5->v_num->num)) {
            return error_value(E_H2HMS_4);
        }
        rnd = qtoi(v5->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v2->v_type != V_ADDR || v3->v_type != V_ADDR || v4->v_type != V_ADDR) {
        return error_value(E_H2HMS_1);
    }
    if (v1->v_type == V_ADDR) {
        v1 = v1->v_addr;
    }
    v2 = v2->v_addr;
    v3 = v3->v_addr;
    v4 = v4->v_addr;
    if (v1->v_type != V_NUM || (v2->v_type != V_NUM && v2->v_type != V_NULL) || (v3->v_type != V_NUM && v3->v_type != V_NULL) ||
        (v4->v_type != V_NUM && v4->v_type != V_NULL)) {
        return error_value(E_H2HMS_2);
    }

    /* remember arg subtypes */
    s2 = v2->v_subtype;
    s3 = v3->v_subtype;
    s4 = v4->v_subtype;
    if ((s2 | s3 | s4) & V_NOASSIGNTO) {
        return error_value(E_H2HMS_3);
    }

    /* free old args that will be modified */
    freevalue(v2);
    freevalue(v3);
    freevalue(v4);

    /* set args that will be modified */
    v2->v_type = V_NUM;
    v3->v_type = V_NUM;
    v4->v_type = V_NUM;

    /* restore arg subtypes */
    v2->v_subtype = s2;
    v3->v_subtype = s3;
    v4->v_subtype = s4;

    /*
     * calculate the normalized return value
     *
     * return_value = mod(hours, 24, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(v1->v_num, &_qtwentyfour, rnd);

    /*
     * integer number of hours
     *
     * h = int(return_value);
     */
    v2->v_num = qint(result.v_num);

    /*
     * integer number of minutes
     *
     * tmp = return_value - h;
     * tmp_m = tmp * 60;
     * free(tmp);
     * m = int(tmp_m);
     */
    tmp = qsub(result.v_num, v2->v_num);
    tmp_m = qmuli(tmp, 60);
    qfree(tmp);
    v3->v_num = qint(tmp_m);

    /*
     * number of seconds
     *
     * tmp = tmp_m - m;
     * free(tmp_m);
     * s = tmp * 60;
     * free(tmp);
     */
    tmp = qsub(tmp_m, v3->v_num);
    qfree(tmp_m);
    v4->v_num = qmuli(tmp, 60);
    qfree(tmp);

    /*
     * return the normalized value previously calculated
     */
    return result;
}

static VALUE
f_h2hm(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4;
    NUMBER *tmp;
    VALUE result;
    long rnd;
    short s2, s3; /* to preserve subtypes of v2, v3 */

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];

    /* determine rounding mode */
    if (count == 4) {
        v4 = vals[3];
        if (v4->v_type == V_ADDR) {
            v4 = v4->v_addr;
        }
        if (v4->v_type != V_NUM || qisfrac(v4->v_num) || qisneg(v4->v_num) || zge31b(v4->v_num->num)) {
            return error_value(E_H2HM_4);
        }
        rnd = qtoi(v4->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v2->v_type != V_ADDR || v3->v_type != V_ADDR) {
        return error_value(E_H2HM_1);
    }
    if (v1->v_type == V_ADDR) {
        v1 = v1->v_addr;
    }
    v2 = v2->v_addr;
    v3 = v3->v_addr;
    if (v1->v_type != V_NUM || (v2->v_type != V_NUM && v2->v_type != V_NULL) || (v3->v_type != V_NUM && v3->v_type != V_NULL)) {
        return error_value(E_H2HM_2);
    }

    /* remember arg subtypes */
    s2 = v2->v_subtype;
    s3 = v3->v_subtype;
    if ((s2 | s3) & V_NOASSIGNTO) {
        return error_value(E_H2HM_3);
    }

    /* free old args that will be modified */
    freevalue(v2);
    freevalue(v3);

    /* set args that will be modified */
    v2->v_type = V_NUM;
    v3->v_type = V_NUM;

    /* restore arg subtypes */
    v2->v_subtype = s2;
    v3->v_subtype = s3;

    /*
     * calculate the normalized return value
     *
     * return_value = mod(hours, 24, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(v1->v_num, &_qtwentyfour, rnd);

    /*
     * integer number of gradians
     *
     * h = int(return_value);
     */
    v2->v_num = qint(result.v_num);

    /*
     * integer number of minutes
     *
     * tmp = return_value - h;
     * m = tmp * 60;
     * free(tmp);
     */
    tmp = qsub(result.v_num, v2->v_num);
    v3->v_num = qmuli(tmp, 60);
    qfree(tmp);

    /*
     * return the normalized value previously calculated
     */
    return result;
}

static VALUE
f_dms2d(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4;
    NUMBER *tmp, *tmp2, *tmp3, *tmp4;
    VALUE result;
    long rnd;

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];

    /* determine rounding mode */
    if (count == 4) {
        v4 = vals[3];
        if (v4->v_type == V_ADDR) {
            v4 = v4->v_addr;
        }
        if (v4->v_type != V_NUM || qisfrac(v4->v_num) || qisneg(v4->v_num) || zge31b(v4->v_num->num)) {
            return error_value(E_DMS2D_2);
        }
        rnd = qtoi(v4->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v1->v_type != V_NUM || v2->v_type != V_NUM || v3->v_type != V_NUM) {
        return error_value(E_DMS2D_1);
    }

    /*
     * compute s/3600
     */
    tmp = qdivi(v3->v_num, 3600);

    /*
     * compute m/60
     */
    tmp2 = qdivi(v2->v_num, 60);

    /*
     * compute m/60 + s/3600
     */
    tmp3 = qqadd(tmp2, tmp);
    qfree(tmp);
    qfree(tmp2);

    /*
     * compute d + m/60 + s/3600
     */
    tmp4 = qqadd(v1->v_num, tmp3);
    qfree(tmp3);

    /*
     * compute mod(d + m/60 + s/3600, 360, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(tmp4, &_qthreesixty, rnd);
    qfree(tmp4);

    /*
     * return  mod(d + m/60 + s/3600, 360, rnd);
     */
    return result;
}

static VALUE
f_dm2d(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3;
    NUMBER *tmp, *tmp2;
    VALUE result;
    long rnd;

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];

    /* determine rounding mode */
    if (count == 3) {
        v3 = vals[2];
        if (v3->v_type == V_ADDR) {
            v3 = v3->v_addr;
        }
        if (v3->v_type != V_NUM || qisfrac(v3->v_num) || qisneg(v3->v_num) || zge31b(v3->v_num->num)) {
            return error_value(E_DM2D_2);
        }
        rnd = qtoi(v3->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v1->v_type != V_NUM || v2->v_type != V_NUM) {
        return error_value(E_DM2D_1);
    }

    /*
     * compute m/60
     */
    tmp = qdivi(v2->v_num, 60);

    /*
     * compute d + m/60
     */
    tmp2 = qqadd(v1->v_num, tmp);
    qfree(tmp);

    /*
     * compute mod(d + m/60, 360, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(tmp2, &_qthreesixty, rnd);
    qfree(tmp2);

    /*
     * return mod(d + m/60, 360, rnd);
     */
    return result;
}

static VALUE
f_gms2g(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4;
    NUMBER *tmp, *tmp2, *tmp3, *tmp4;
    VALUE result;
    long rnd;

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];

    /* determine rounding mode */
    if (count == 4) {
        v4 = vals[3];
        if (v4->v_type == V_ADDR) {
            v4 = v4->v_addr;
        }
        if (v4->v_type != V_NUM || qisfrac(v4->v_num) || qisneg(v4->v_num) || zge31b(v4->v_num->num)) {
            return error_value(E_GMS2G_2);
        }
        rnd = qtoi(v4->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v1->v_type != V_NUM || v2->v_type != V_NUM || v3->v_type != V_NUM) {
        return error_value(E_GMS2G_1);
    }

    /*
     * compute s/3600
     */
    tmp = qdivi(v3->v_num, 3600);

    /*
     * compute m/60
     */
    tmp2 = qdivi(v2->v_num, 60);

    /*
     * compute m/60 + s/3600
     */
    tmp3 = qqadd(tmp2, tmp);
    qfree(tmp);
    qfree(tmp2);

    /*
     * compute g + m/60 + s/3600
     */
    tmp4 = qqadd(v1->v_num, tmp3);
    qfree(tmp3);

    /*
     * compute mod(g + m/60 + s/3600, 400, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(tmp4, &_qfourhundred, rnd);
    qfree(tmp4);

    /*
     * return mod(g + m/60 + s/3600, 400, rnd);
     */
    return result;
}

static VALUE
f_gm2g(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3;
    NUMBER *tmp, *tmp2;
    VALUE result;
    long rnd;

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];

    /* determine rounding mode */
    if (count == 3) {
        v3 = vals[2];
        if (v3->v_type == V_ADDR) {
            v3 = v3->v_addr;
        }
        if (v3->v_type != V_NUM || qisfrac(v3->v_num) || qisneg(v3->v_num) || zge31b(v3->v_num->num)) {
            return error_value(E_GM2G_2);
        }
        rnd = qtoi(v3->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v1->v_type != V_NUM || v2->v_type != V_NUM) {
        return error_value(E_GM2G_1);
    }

    /*
     * compute m/60
     */
    tmp = qdivi(v2->v_num, 60);

    /*
     * compute g + m/60
     */
    tmp2 = qqadd(v1->v_num, tmp);
    qfree(tmp);

    /*
     * compute mod(g + m/60, 400, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(tmp2, &_qfourhundred, rnd);
    qfree(tmp2);

    /*
     * return mod(g + m/60, 400, rnd);
     */
    return result;
}

static VALUE
f_hms2h(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4;
    NUMBER *tmp, *tmp2, *tmp3, *tmp4;
    VALUE result;
    long rnd;

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];
    v3 = vals[2];

    /* determine rounding mode */
    if (count == 4) {
        v4 = vals[3];
        if (v4->v_type == V_ADDR) {
            v4 = v4->v_addr;
        }
        if (v4->v_type != V_NUM || qisfrac(v4->v_num) || qisneg(v4->v_num) || zge31b(v4->v_num->num)) {
            return error_value(E_HMS2H_2);
        }
        rnd = qtoi(v4->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v1->v_type != V_NUM || v2->v_type != V_NUM || v3->v_type != V_NUM) {
        return error_value(E_HMS2H_1);
    }

    /*
     * compute s/3600
     */
    tmp = qdivi(v3->v_num, 3600);

    /*
     * compute m/60
     */
    tmp2 = qdivi(v2->v_num, 60);

    /*
     * compute m/60 + s/3600
     */
    tmp3 = qqadd(tmp2, tmp);
    qfree(tmp);
    qfree(tmp2);

    /*
     * compute h + m/60 + s/3600
     */
    tmp4 = qqadd(v1->v_num, tmp3);
    qfree(tmp3);

    /*
     * compute mod(h + m/60 + s/3600, 24, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(tmp4, &_qtwentyfour, rnd);
    qfree(tmp4);

    /*
     * return mod(d + m/60 + s/3600, 24, rnd);
     */
    return result;
}

static VALUE
f_hm2h(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3;
    NUMBER *tmp, *tmp2;
    VALUE result;
    long rnd;

    /* collect required args */
    v1 = vals[0];
    v2 = vals[1];

    /* determine rounding mode */
    if (count == 3) {
        v3 = vals[2];
        if (v3->v_type == V_ADDR) {
            v3 = v3->v_addr;
        }
        if (v3->v_type != V_NUM || qisfrac(v3->v_num) || qisneg(v3->v_num) || zge31b(v3->v_num->num)) {
            return error_value(E_H2HM_2);
        }
        rnd = qtoi(v3->v_num);
    } else {
        rnd = conf->quomod;
    }

    /* type parse args */
    if (v1->v_type != V_NUM || v2->v_type != V_NUM) {
        return error_value(E_H2HM_1);
    }

    /*
     * compute m/60
     */
    tmp = qdivi(v2->v_num, 60);

    /*
     * compute d + m/60
     */
    tmp2 = qqadd(v1->v_num, tmp);
    qfree(tmp);

    /*
     * compute mod(h + m/60, 24, rnd);
     */
    result.v_type = v1->v_type;
    result.v_subtype = v1->v_subtype;
    result.v_num = qmod(tmp2, &_qtwentyfour, rnd);
    qfree(tmp2);

    /*
     * return mod(h + m/60, 24, rnd);
     */
    return result;
}

static VALUE
f_mmin(VALUE *v1, VALUE *v2)
{
    VALUE sixteen, res;

    /* initialize VALUEs */
    sixteen.v_subtype = V_NOSUBTYPE;
    res.v_subtype = V_NOSUBTYPE;

    sixteen.v_type = V_NUM;
    sixteen.v_num = itoq(16);
    modvalue(v1, v2, &sixteen, &res);
    qfree(sixteen.v_num);
    return res;
}

static NUMBER *
f_near(int count, NUMBER **vals)
{
    NUMBER *err; /* epsilon error tolerance */
    FLAG near;   /* qnear() return value */
    NUMBER *ret; /* return value as NUMBER */

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 3) {
        if (check_epsilon(vals[2]) == false) {
            math_error("Invalid value for near epsilon: must be: 0 < epsilon < 1");
            not_reached();
        }
        err = vals[2];
    }

    /*
     * compute compare nearness of two numbers to a given error tolerance
     */
    near = qnear(vals[0], vals[1], err);
    ret = itoq((long)near);
    return ret;
}

static NUMBER *
f_cfsim(int count, NUMBER **vals)
{
    long R;

    R = (count > 1) ? qtoi(vals[1]) : conf->cfsim;
    return qcfsim(vals[0], R);
}

static NUMBER *
f_cfappr(int count, NUMBER **vals)
{
    long R;
    NUMBER *q; /* approximation limit */

    /*
     * determine epsilon or and approximation limit
     *
     * NOTE: q is not purely an err (epsilon) value.
     *       When q is >= 1, it is approximation limit.
     *       Moreover q can be < 0.  No value check on q is needed.
     */
    q = (count > 1) ? vals[1] : conf->epsilon;

    /*
     * compute approximation using continued fractions
     */
    R = (count > 2) ? qtoi(vals[2]) : conf->cfappr;
    return qcfappr(vals[0], q, R);
}

static VALUE
f_ceil(VALUE *val)
{
    VALUE tmp, res;

    /* initialize VALUEs */
    res.v_subtype = V_NOSUBTYPE;
    tmp.v_subtype = V_NOSUBTYPE;

    tmp.v_type = V_NUM;
    tmp.v_num = qlink(&_qone_);
    apprvalue(val, &tmp, &tmp, &res);
    return res;
}

static VALUE
f_floor(VALUE *val)
{
    VALUE tmp1, tmp2, res;

    /* initialize VALUEs */
    res.v_subtype = V_NOSUBTYPE;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp2.v_subtype = V_NOSUBTYPE;

    tmp1.v_type = V_NUM;
    tmp1.v_num = qlink(&_qone_);
    tmp2.v_type = V_NUM;
    tmp2.v_num = qlink(&_qzero_);
    apprvalue(val, &tmp1, &tmp2, &res);
    return res;
}

static VALUE
f_sqrt(int count, VALUE **vals)
{
    VALUE tmp1, tmp2, result;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;
    tmp1.v_subtype = V_NOSUBTYPE;
    tmp2.v_subtype = V_NOSUBTYPE;

    if (count > 2) {
        tmp2 = *vals[2];
    } else {
        tmp2.v_type = V_NULL;
    }
    if (count > 1) {
        tmp1 = *vals[1];
    } else {
        tmp1.v_type = V_NULL;
    }
    sqrtvalue(vals[0], &tmp1, &tmp2, &result);
    return result;
}

static VALUE
f_root(int count, VALUE **vals)
{
    VALUE *vp, err, result;

    /* initialize VALUEs */
    err.v_subtype = V_NOSUBTYPE;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is != 0.
     */
    if (count > 2) {
        vp = vals[2];
    } else {
        err.v_num = conf->epsilon;
        err.v_type = V_NUM;
        vp = &err;
    }
    if (vp->v_type != V_NUM || qiszero(vp->v_num)) {
        return error_value(E_ROOT_3);
    }

    /*
     * compute root of a number to a given error tolerance
     */
    rootvalue(vals[0], vals[1], vp, &result);
    return result;
}

static VALUE
f_power(int count, VALUE **vals)
{
    VALUE *vp, err, result;

    /* initialize VALUEs */
    err.v_subtype = V_NOSUBTYPE;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is != 0.
     */
    if (count > 2) {
        vp = vals[2];
    } else {
        err.v_num = conf->epsilon;
        err.v_type = V_NUM;
        vp = &err;
    }
    if ((vp->v_type != V_NUM) || qisneg(vp->v_num) || qiszero(vp->v_num)) {
        return error_value(E_POWER_3);
    }

    /*
     * compute evaluate a numerical power to a given error tolerance
     */
    powervalue(vals[0], vals[1], vp, &result);
    return result;
}

static VALUE
f_polar(int count, VALUE **vals)
{
    VALUE *vp, err, result;
    COMPLEX *c;

    /* initialize VALUEs */
    err.v_subtype = V_NOSUBTYPE;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is != 0.
     */
    if (count > 2) {
        vp = vals[2];
        if ((vp->v_type != V_NUM) || qisneg(vp->v_num) || qiszero(vp->v_num)) {
            return error_value(E_POLAR_2);
        }
    } else {
        err.v_num = conf->epsilon;
        err.v_type = V_NUM;
        vp = &err;
    }
    if ((vp->v_type != V_NUM) || qisneg(vp->v_num) || qiszero(vp->v_num)) {
        return error_value(E_POLAR_2);
    }

    /*
     * compute complex number by modulus (radius) and argument (angle) to a given error tolerance
     */
    if ((vals[0]->v_type != V_NUM) || (vals[1]->v_type != V_NUM)) {
        return error_value(E_POLAR_1);
    }
    c = c_polar(vals[0]->v_num, vals[1]->v_num, vp->v_num);
    result.v_com = c;
    result.v_type = V_COM;
    if (cisreal(c)) {
        result.v_num = c_to_q(c, true);
        result.v_type = V_NUM;
    }
    return result;
}

static VALUE
f_ilog(VALUE *v1, VALUE *v2)
{
    VALUE res;

    if (v2->v_type != V_NUM || qisfrac(v2->v_num) || qiszero(v2->v_num) || qisunit(v2->v_num)) {
        return error_value(E_ILOGB);
    }

    switch (v1->v_type) {
    case V_NUM:
        res.v_num = qilog(v1->v_num, v2->v_num->num);
        break;
    case V_COM:
        res.v_num = c_ilog(v1->v_com, v2->v_num->num);
        break;
    default:
        return error_value(E_ILOG);
    }

    if (res.v_num == NULL) {
        return error_value(E_LOGINF);
    }

    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;
    return res;
}

static VALUE
f_ilog2(VALUE *vp)
{
    VALUE res;

    switch (vp->v_type) {
    case V_NUM:
        res.v_num = qilog(vp->v_num, _two_);
        break;
    case V_COM:
        res.v_num = c_ilog(vp->v_com, _two_);
        break;
    default:
        return error_value(E_IBASE2_LOG);
    }

    if (res.v_num == NULL) {
        return error_value(E_LOGINF);
    }

    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;
    return res;
}

static VALUE
f_ilog10(VALUE *vp)
{
    VALUE res;

    switch (vp->v_type) {
    case V_NUM:
        res.v_num = qilog(vp->v_num, _ten_);
        break;
    case V_COM:
        res.v_num = c_ilog(vp->v_com, _ten_);
        break;
    default:
        return error_value(E_IBASE10_LOG);
    }

    if (res.v_num == NULL) {
        return error_value(E_LOGINF);
    }

    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;
    return res;
}

static NUMBER *
f_faccnt(NUMBER *val1, NUMBER *val2)
{
    if (qisfrac(val1) || qisfrac(val2)) {
        math_error("Non-integral argument for fcnt");
    }
    return itoq(zdivcount(val1->num, val2->num));
}

static VALUE
f_matfill(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    v1 = vals[0];
    v2 = vals[1];
    if (v1->v_type != V_ADDR) {
        return error_value(E_MATFILL_1);
    }
    v1 = v1->v_addr;
    if (v1->v_subtype & V_NOCOPYTO) {
        return error_value(E_MATFILL_3);
    }
    if (v1->v_type != V_MAT) {
        return error_value(E_MATFILL_2);
    }
    if (v2->v_type == V_ADDR) {
        v2 = v2->v_addr;
    }
    if (v2->v_subtype & V_NOASSIGNFROM) {
        return error_value(E_MATFILL_4);
    }
    if (count == 3) {
        v3 = vals[2];
        if (v3->v_type == V_ADDR) {
            v3 = v3->v_addr;
        }
        if (v3->v_subtype & V_NOASSIGNFROM) {
            return error_value(E_MATFILL_4);
        }
    } else {
        v3 = NULL;
    }
    matfill(v1->v_mat, v2, v3);
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_matsum(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /* firewall */
    if (vp->v_type != V_MAT) {
        return error_value(E_MATSUM);
    }

    /* sum matrix */
    matsum(vp->v_mat, &result);
    return result;
}

static VALUE
f_isident(VALUE *vp)
{
    VALUE result;

    /* initialize VALUEs */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type == V_MAT) {
        result.v_num = itoq((long)matisident(vp->v_mat));
    } else {
        result.v_num = itoq(0);
    }
    return result;
}

static VALUE
f_mattrace(VALUE *vp)
{
    if (vp->v_type != V_MAT) {
        return error_value(E_MATTRACE_1);
    }
    return mattrace(vp->v_mat);
}

static VALUE
f_mattrans(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_MAT) {
        return error_value(E_MATTRANS_1);
    }
    if (vp->v_mat->m_dim > 2) {
        return error_value(E_MATTRANS_2);
    }
    result.v_type = V_MAT;
    result.v_mat = mattrans(vp->v_mat);
    return result;
}

static VALUE
f_det(VALUE *vp)
{
    if (vp->v_type != V_MAT) {
        return error_value(E_DET_1);
    }

    return matdet(vp->v_mat);
}

static VALUE
f_matdim(VALUE *vp)
{
    VALUE result;

    /* initialize VALUEs */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_OBJ:
        result.v_num = itoq(vp->v_obj->o_actions->oa_count);
        break;
    case V_MAT:
        result.v_num = itoq((long)vp->v_mat->m_dim);
        break;
    default:
        return error_value(E_MATDIM);
    }
    return result;
}

static VALUE
f_matmin(VALUE *v1, VALUE *v2)
{
    VALUE result;
    NUMBER *q;
    long i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_MAT) {
        return error_value(E_MATMIN_1);
    }
    if (v2->v_type != V_NUM) {
        return error_value(E_MATMIN_2);
    }
    q = v2->v_num;
    if (qisfrac(q) || qisneg(q) || qiszero(q)) {
        return error_value(E_MATMIN_2);
    }
    i = qtoi(q);
    if (i > v1->v_mat->m_dim) {
        return error_value(E_MATMIN_3);
    }
    result.v_type = V_NUM;
    result.v_num = itoq(v1->v_mat->m_min[i - 1]);
    return result;
}

static VALUE
f_matmax(VALUE *v1, VALUE *v2)
{
    VALUE result;
    NUMBER *q;
    long i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_MAT) {
        return error_value(E_MATMAX_1);
    }
    if (v2->v_type != V_NUM) {
        return error_value(E_MATMAX_2);
    }
    q = v2->v_num;
    if (qisfrac(q) || qisneg(q) || qiszero(q)) {
        return error_value(E_MATMAX_2);
    }
    i = qtoi(q);
    if (i > v1->v_mat->m_dim) {
        return error_value(E_MATMAX_3);
    }
    result.v_type = V_NUM;
    result.v_num = itoq(v1->v_mat->m_max[i - 1]);
    return result;
}

static VALUE
f_cp(VALUE *v1, VALUE *v2)
{
    MATRIX *m1, *m2;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if ((v1->v_type != V_MAT) || (v2->v_type != V_MAT)) {
        return error_value(E_CP_1);
    }
    m1 = v1->v_mat;
    m2 = v2->v_mat;
    if ((m1->m_dim != 1) || (m2->m_dim != 1)) {
        return error_value(E_CP_2);
    }
    if ((m1->m_size != 3) || (m2->m_size != 3)) {
        return error_value(E_CP_3);
    }
    result.v_type = V_MAT;
    result.v_mat = matcross(m1, m2);
    return result;
}

static VALUE
f_dp(VALUE *v1, VALUE *v2)
{
    MATRIX *m1, *m2;

    if ((v1->v_type != V_MAT) || (v2->v_type != V_MAT)) {
        return error_value(E_DP_1);
    }
    m1 = v1->v_mat;
    m2 = v2->v_mat;
    if ((m1->m_dim != 1) || (m2->m_dim != 1)) {
        return error_value(E_DP_2);
    }
    if (m1->m_size != m2->m_size) {
        return error_value(E_DP_3);
    }
    return matdot(m1, m2);
}

static VALUE
f_strlen(VALUE *vp)
{
    VALUE result;
    long len = 0;
    char *c;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_STR) {
        return error_value(E_STRLEN);
    }
    c = vp->v_str->s_str;
    while (*c++) {
        len++;
    }
    result.v_type = V_NUM;
    result.v_num = itoq(len);
    return result;
}

static VALUE
f_strcmp(VALUE *v1, VALUE *v2)
{
    VALUE result;
    FLAG flag;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_STR || v2->v_type != V_STR) {
        return error_value(E_STRCMP);
    }

    flag = stringrel(v1->v_str, v2->v_str);

    result.v_type = V_NUM;
    result.v_num = itoq((long)flag);
    return result;
}

static VALUE
f_strcasecmp(VALUE *v1, VALUE *v2)
{
    VALUE result;
    FLAG flag;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_STR || v2->v_type != V_STR) {
        return error_value(E_STRCASECMP);
    }

    flag = stringcaserel(v1->v_str, v2->v_str);

    result.v_type = V_NUM;
    result.v_num = itoq((long)flag);
    return result;
}

static VALUE
f_strncmp(VALUE *v1, VALUE *v2, VALUE *v3)
{
    long n1, n2, n;
    FLAG flag;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_STR || v2->v_type != V_STR || v3->v_type != V_NUM || qisneg(v3->v_num) || qisfrac(v3->v_num) ||
        zge31b(v3->v_num->num)) {
        return error_value(E_STRNCMP);
    }
    n1 = v1->v_str->s_len;
    n2 = v2->v_str->s_len;
    n = qtoi(v3->v_num);
    if (n < n1) {
        v1->v_str->s_len = n;
    }
    if (n < n2) {
        v2->v_str->s_len = n;
    }

    flag = stringrel(v1->v_str, v2->v_str);

    v1->v_str->s_len = n1;
    v2->v_str->s_len = n2;

    result.v_type = V_NUM;
    result.v_num = itoq((long)flag);
    return result;
}
static VALUE
f_strncasecmp(VALUE *v1, VALUE *v2, VALUE *v3)
{
    long n1, n2, n;
    FLAG flag;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_STR || v2->v_type != V_STR || v3->v_type != V_NUM || qisneg(v3->v_num) || qisfrac(v3->v_num) ||
        zge31b(v3->v_num->num)) {
        return error_value(E_STRNCASECMP);
    }
    n1 = v1->v_str->s_len;
    n2 = v2->v_str->s_len;
    n = qtoi(v3->v_num);
    if (n < n1) {
        v1->v_str->s_len = n;
    }
    if (n < n2) {
        v2->v_str->s_len = n;
    }

    flag = stringcaserel(v1->v_str, v2->v_str);

    v1->v_str->s_len = n1;
    v2->v_str->s_len = n2;

    result.v_type = V_NUM;
    result.v_num = itoq((long)flag);
    return result;
}

static VALUE
f_strtoupper(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_STR) {
        return error_value(E_STRTOUPPER);
    }

    result.v_str = stringtoupper(vp->v_str);
    result.v_type = V_STR;
    return result;
}

static VALUE
f_strtolower(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_STR) {
        return error_value(E_STRTOLOWER);
    }

    result.v_str = stringtolower(vp->v_str);
    result.v_type = V_STR;
    return result;
}

static VALUE
f_strcat(int count, VALUE **vals)
{
    VALUE **vp;
    char *c, *c1;
    int i;
    long len;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    len = 0;
    result.v_type = V_STR;
    vp = vals;
    for (i = 0; i < count; i++, vp++) {
        if ((*vp)->v_type != V_STR) {
            return error_value(E_STRCAT);
        }
        c = (*vp)->v_str->s_str;
        while (*c++) {
            len++;
        }
    }
    if (len == 0) {
        result.v_str = slink(&_nullstring_);
        return result;
    }
    c = (char *)calloc(len + 1, 1);
    if (c == NULL) {
        math_error("No memory for strcat");
        not_reached();
    }
    result.v_str = stralloc();
    result.v_str->s_str = c;
    result.v_str->s_len = len;
    for (vp = vals; count-- > 0; vp++) {
        c1 = (*vp)->v_str->s_str;
        while (*c1) {
            *c++ = *c1++;
        }
    }
    *c = '\0';
    return result;
}

static VALUE
f_strcpy(VALUE *v1, VALUE *v2)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_STR || v2->v_type != V_STR) {
        return error_value(E_STRCPY);
    }
    result.v_str = stringcpy(v1->v_str, v2->v_str);
    result.v_type = V_STR;
    return result;
}

static VALUE
f_strncpy(VALUE *v1, VALUE *v2, VALUE *v3)
{
    VALUE result;
    long num;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_STR || v2->v_type != V_STR || v3->v_type != V_NUM || qisfrac(v3->v_num) || qisneg(v3->v_num)) {
        return error_value(E_STRNCPY);
    }
    if (zge31b(v3->v_num->num)) {
        num = v2->v_str->s_len;
    } else {
        num = qtoi(v3->v_num);
    }
    result.v_str = stringncpy(v1->v_str, v2->v_str, num);
    result.v_type = V_STR;
    return result;
}

static VALUE
f_substr(VALUE *v1, VALUE *v2, VALUE *v3)
{
    NUMBER *q1, *q2;
    size_t start, len;
    char *cp;
    char *ccp;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_STR) {
        return error_value(E_SUBSTR_1);
    }
    if ((v2->v_type != V_NUM) || (v3->v_type != V_NUM)) {
        return error_value(E_SUBSTR_2);
    }
    q1 = v2->v_num;
    q2 = v3->v_num;
    if (qisfrac(q1) || qisneg(q1) || qisfrac(q2) || qisneg(q2)) {
        return error_value(E_SUBSTR_2);
    }
    start = qtoi(q1);
    len = qtoi(q2);
    if (start > 0) {
        start--;
    }
    result.v_type = V_STR;
    if (start >= v1->v_str->s_len || len == 0) {
        result.v_str = slink(&_nullstring_);
        return result;
    }
    if (len > v1->v_str->s_len - start) {
        len = v1->v_str->s_len - start;
    }
    cp = v1->v_str->s_str + start;
    ccp = (char *)calloc(len + 1, 1);
    if (ccp == NULL) {
        math_error("No memory for substr");
        not_reached();
    }
    result.v_str = stralloc();
    result.v_str->s_len = len;
    result.v_str->s_str = ccp;
    while (len-- > 0) {
        *ccp++ = *cp++;
    }
    *ccp = '\0';
    return result;
}

static VALUE
f_char(VALUE *vp)
{
    char ch;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_NUM:
        if (qisfrac(vp->v_num)) {
            return error_value(E_CHAR);
        }
        ch = (char)vp->v_num->num.v[0];
        if (qisneg(vp->v_num)) {
            ch = -ch;
        }
        break;
    case V_OCTET:
        ch = *vp->v_octet;
        break;
    case V_STR:
        ch = *vp->v_str->s_str;
        break;
    default:
        return error_value(E_CHAR);
    }
    result.v_type = V_STR;
    result.v_str = charstring(ch);
    return result;
}

static VALUE
f_ord(VALUE *vp)
{
    OCTET *c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = (OCTET *)vp->v_str->s_str;
        break;
    case V_OCTET:
        c = vp->v_octet;
        break;
    default:
        return error_value(E_ORD);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((long)(*c & 0xff));
    return result;
}

static VALUE
f_isupper(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISUPPER);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((isupper(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_islower(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISLOWER);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((islower(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_isalnum(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISALNUM);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((isalnum(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_isalpha(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISALPHA);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((isalpha(c)) ? 1l : 0l);
    return result;
}

#  if 0  /* XXX - add isascii builtin funcion - XXX */

static VALUE
f_isascii(VALUE *vp)
{
    char c;
    VALUE result;

    result.v_subtype = V_NOSUBTYPE;

    switch(vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISASCII);
    }

    result.v_type = V_NUM;
    result.v_num = itoq( (isascii( c ))?1l:0l);
    return result;
}

#  endif

static VALUE
f_iscntrl(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISCNTRL);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((iscntrl(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_isdigit(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISDIGIT);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((isdigit(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_isgraph(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISGRAPH);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((isgraph(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_isprint(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISPRINT);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((isprint(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_ispunct(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISPUNCT);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((ispunct(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_isspace(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISSPACE);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((isspace(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_isxdigit(VALUE *vp)
{
    char c;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    switch (vp->v_type) {
    case V_STR:
        c = *vp->v_str->s_str;
        break;
    case V_OCTET:
        c = *vp->v_octet;
        break;
    default:
        return error_value(E_ISXDIGIT);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((isxdigit(c)) ? 1l : 0l);
    return result;
}

static VALUE
f_protect(int count, VALUE **vals)
{
    int i, depth;
    VALUE *v1, *v2, *v3;

    VALUE result;
    bool have_nblock;

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    v1 = vals[0];
    have_nblock = (v1->v_type == V_NBLOCK);
    if (!have_nblock) {
        if (v1->v_type != V_ADDR) {
            return error_value(E_PROTECT_1);
        }
        v1 = v1->v_addr;
    }
    if (count == 1) {
        result.v_type = V_NUM;
        if (have_nblock) {
            result.v_num = itoq(v1->v_nblock->subtype);
        } else {
            result.v_num = itoq(v1->v_subtype);
        }
        return result;
    }
    v2 = vals[1];
    if (v2->v_type == V_ADDR) {
        v2 = v2->v_addr;
    }
    if (v2->v_type != V_NUM || qisfrac(v2->v_num) || zge16b(v2->v_num->num)) {
        return error_value(E_PROTECT_2);
    }
    i = qtoi(v2->v_num);
    depth = 0;
    if (count > 2) {
        v3 = vals[2];
        if (v3->v_type == V_ADDR) {
            v3 = v3->v_addr;
        }
        if (v3->v_type != V_NUM || qisfrac(v3->v_num) || qisneg(v3->v_num) || zge31b(v3->v_num->num)) {
            return error_value(E_PROTECT_3);
        }
        depth = qtoi(v3->v_num);
    }
    protecttodepth(v1, i, depth);
    return result;
}

static VALUE
f_size(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * return information about the number of elements
     *
     * This is not the sizeof, see f_sizeof() for that information.
     * This is not the memsize, see f_memsize() for that information.
     *
     * The size of a file is treated in a special way ... we do
     * not use the number of elements, but rather the length
     * of the file as would be reported by fsize().
     */
    if (vp->v_type == V_FILE) {
        return f_fsize(vp);
    } else {
        result.v_type = V_NUM;
        result.v_num = itoq(elm_count(vp));
    }
    return result;
}

static VALUE
f_sizeof(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * return information about memory footprint
     *
     * This is not the number of elements, see f_size() for that info.
     * This is not the memsize, see f_memsize() for that information.
     */
    result.v_num = itoq(lsizeof(vp));
    return result;
}

static VALUE
f_memsize(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * return information about memory footprint
     *
     * This is not the number of elements, see f_size() for that info.
     * This is not the sizeof, see f_sizeof() for that information.
     */
    result.v_num = itoq(memsize(vp));
    return result;
}

static VALUE
f_search(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4;
    NUMBER *start, *end;
    VALUE vsize;
    NUMBER *size;
    ZVALUE pos;
    ZVALUE indx;
    long len;
    ZVALUE zlen, tmp;
    VALUE result;
    long l_start = 0, l_end = 0;
    int i = 0;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;
    vsize.v_subtype = V_NOSUBTYPE;

    v1 = *vals++;
    v2 = *vals++;
    if ((v1->v_type == V_FILE || v1->v_type == V_STR) && v2->v_type != V_STR) {
        return error_value(E_SEARCH_2);
    }
    start = end = NULL;
    if (count > 2) {
        v3 = *vals++;
        if (v3->v_type != V_NUM && v3->v_type != V_NULL) {
            return error_value(E_SEARCH_3);
        }
        if (v3->v_type == V_NUM) {
            start = v3->v_num;
            if (qisfrac(start)) {
                return error_value(E_SEARCH_3);
            }
        }
    }
    if (count > 3) {
        v4 = *vals;
        if (v4->v_type != V_NUM && v4->v_type != V_NULL) {
            return error_value(E_SEARCH_4);
        }
        if (v4->v_type == V_NUM) {
            end = v4->v_num;
            if (qisfrac(end)) {
                return error_value(E_SEARCH_4);
            }
        }
    }
    result.v_type = V_NULL;
    vsize = f_size(v1);
    if (vsize.v_type != V_NUM) {
        return error_value(E_SEARCH_5);
    }
    size = vsize.v_num;
    if (start) {
        if (qisneg(start)) {
            start = qqadd(size, start);
            if (qisneg(start)) {
                qfree(start);
                start = qlink(&_qzero_);
            }
        } else {
            start = qlink(start);
        }
    }
    if (end) {
        if (!qispos(end)) {
            end = qqadd(size, end);
        } else {
            if (qrel(end, size) > 0) {
                end = qlink(size);
            } else {
                end = qlink(end);
            }
        }
    }
    if (v1->v_type == V_FILE) {
        if (count == 2 || (count == 4 && (start == NULL || end == NULL))) {
            i = ftellid(v1->v_file, &pos);
            if (i < 0) {
                qfree(size);
                if (start) {
                    qfree(start);
                }
                if (end) {
                    qfree(end);
                }
                return error_value(E_SEARCH_5);
            }
            if (count == 2 || (count == 4 && end != NULL)) {
                start = qalloc();
                start->num = pos;
            } else {
                end = qalloc();
                end->num = pos;
            }
        }
        if (start == NULL) {
            start = qlink(&_qzero_);
        }
        if (end == NULL) {
            end = size;
        } else {
            qfree(size);
        }
        len = v2->v_str->s_len;
        utoz(len, &zlen);
        zsub(end->num, zlen, &tmp);
        zfree(zlen);
        i = fsearch(v1->v_file, v2->v_str->s_str, start->num, tmp, &indx);
        zfree(tmp);
        if (i == 2) {
            /* report empty string found :-) */
            result.v_type = V_NUM;
            result.v_num = start;
            qfree(end);
            return result;
        }
        if (i == EOF) {
            qfree(start);
            qfree(end);
            return error_value(errno);
        }
        if (i < 0) {
            qfree(start);
            qfree(end);
            return error_value(E_SEARCH_6);
        }
        if (i == 0) {
            result.v_type = V_NUM;
            result.v_num = qalloc();
            result.v_num->num = indx;
            qlink(result.v_num);
        }
        qfree(start);
        qfree(end);
        return result;
    }
    if (start == NULL) {
        start = qlink(&_qzero_);
    }
    if (end == NULL) {
        end = qlink(size);
    }
    if (qrel(start, end) >= 0) {
        qfree(size);
        qfree(start);
        qfree(end);
        return result;
    }
    qfree(size);
    l_start = ztolong(start->num);
    l_end = ztolong(end->num);
    switch (v1->v_type) {
    case V_MAT:
        i = matsearch(v1->v_mat, v2, l_start, l_end, &indx);
        break;
    case V_LIST:
        i = listsearch(v1->v_list, v2, l_start, l_end, &indx);
        break;
    case V_ASSOC:
        i = assocsearch(v1->v_assoc, v2, l_start, l_end, &indx);
        break;
    case V_STR:
        i = stringsearch(v1->v_str, v2->v_str, l_start, l_end, &indx);
        break;
    default:
        qfree(start);
        qfree(end);
        return error_value(E_SEARCH_1);
    }
    qfree(start);
    qfree(end);
    if (i == 0) {
        result.v_type = V_NUM;
        result.v_num = qalloc();
        result.v_num->num = indx;
    }
    return result;
}

static VALUE
f_rsearch(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3, *v4;
    NUMBER *start, *end;
    VALUE vsize;
    NUMBER *size;
    NUMBER *qlen;
    NUMBER *qtmp;
    ZVALUE pos;
    ZVALUE indx;
    VALUE result;
    long l_start = 0, l_end = 0;
    int i;

    /* initialize VALUEs */
    vsize.v_subtype = V_NOSUBTYPE;
    result.v_subtype = V_NOSUBTYPE;

    v1 = *vals++;
    v2 = *vals++;
    if ((v1->v_type == V_FILE || v1->v_type == V_STR) && v2->v_type != V_STR) {
        return error_value(E_RSEARCH_2);
    }
    start = end = NULL;
    if (count > 2) {
        v3 = *vals++;
        if (v3->v_type != V_NUM && v3->v_type != V_NULL) {
            return error_value(E_RSEARCH_3);
        }
        if (v3->v_type == V_NUM) {
            start = v3->v_num;
            if (qisfrac(start)) {
                return error_value(E_RSEARCH_3);
            }
        }
    }
    if (count > 3) {
        v4 = *vals;
        if (v4->v_type != V_NUM && v4->v_type != V_NULL) {
            return error_value(E_RSEARCH_4);
        }
        if (v4->v_type == V_NUM) {
            end = v4->v_num;
            if (qisfrac(end)) {
                return error_value(E_RSEARCH_3);
            }
        }
    }
    result.v_type = V_NULL;
    vsize = f_size(v1);
    if (vsize.v_type != V_NUM) {
        return error_value(E_RSEARCH_5);
    }
    size = vsize.v_num;
    if (start) {
        if (qisneg(start)) {
            start = qqadd(size, start);
            if (qisneg(start)) {
                qfree(start);
                start = qlink(&_qzero_);
            }
        } else {
            start = qlink(start);
        }
    }
    if (end) {
        if (!qispos(end)) {
            end = qqadd(size, end);
        } else {
            if (qrel(end, size) > 0) {
                end = qlink(size);
            } else {
                end = qlink(end);
            }
        }
    }
    if (v1->v_type == V_FILE) {
        if (count == 2 || (count == 4 && (start == NULL || end == NULL))) {
            i = ftellid(v1->v_file, &pos);
            if (i < 0) {
                qfree(size);
                if (start) {
                    qfree(start);
                }
                if (end) {
                    qfree(end);
                }
                return error_value(E_RSEARCH_5);
            }
            if (count == 2 || (count == 4 && end != NULL)) {
                start = qalloc();
                start->num = pos;
            } else {
                end = qalloc();
                end->num = pos;
            }
        }
        qlen = utoq(v2->v_str->s_len);
        qtmp = qsub(size, qlen);
        qfree(size);
        size = qtmp;
        if (count < 4) {
            end = start;
            start = NULL;
        } else {
            qtmp = qsub(end, qlen);
            qfree(end);
            end = qtmp;
        }
        if (end == NULL) {
            end = qlink(size);
        }
        if (start == NULL) {
            start = qlink(&_qzero_);
        }
        if (qrel(end, size) > 0) {
            qfree(end);
            end = qlink(size);
        }
        qfree(qlen);
        qfree(size);
        if (qrel(start, end) > 0) {
            qfree(start);
            qfree(end);
            return result;
        }
        i = frsearch(v1->v_file, v2->v_str->s_str, end->num, start->num, &indx);
        if (i == EOF) {
            qfree(start);
            qfree(end);
            return error_value(errno);
        }
        if (i < 0) {
            qfree(start);
            qfree(end);
            return error_value(E_RSEARCH_6);
        }
        if (i == 0) {
            result.v_type = V_NUM;
            result.v_num = qalloc();
            result.v_num->num = indx;
            qlink(result.v_num);
        }
        qfree(start);
        qfree(end);
        return result;
    }
    if (count < 4) {
        if (start) {
            end = qinc(start);
            qfree(start);
        } else {
            end = qlink(size);
        }
        start = qlink(&_qzero_);
    } else {
        if (start == NULL) {
            start = qlink(&_qzero_);
        }
        if (end == NULL) {
            end = qlink(size);
        }
    }

    qfree(size);
    if (qrel(start, end) >= 0) {
        qfree(start);
        qfree(end);
        return result;
    }
    l_start = ztolong(start->num);
    l_end = ztolong(end->num);
    switch (v1->v_type) {
    case V_MAT:
        i = matrsearch(v1->v_mat, v2, l_start, l_end, &indx);
        break;
    case V_LIST:
        i = listrsearch(v1->v_list, v2, l_start, l_end, &indx);
        break;
    case V_ASSOC:
        i = assocrsearch(v1->v_assoc, v2, l_start, l_end, &indx);
        break;
    case V_STR:
        i = stringrsearch(v1->v_str, v2->v_str, l_start, l_end, &indx);
        break;
    default:
        qfree(start);
        qfree(end);
        return error_value(E_RSEARCH_1);
    }
    qfree(start);
    qfree(end);
    if (i == 0) {
        result.v_type = V_NUM;
        result.v_num = qalloc();
        result.v_num->num = indx;
    }
    return result;
}

static VALUE
f_list(int count, VALUE **vals)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_LIST;
    result.v_subtype = V_NOSUBTYPE;

    result.v_list = listalloc();
    while (count-- > 0) {
        insertlistlast(result.v_list, *vals++);
    }
    return result;
}

/*ARGSUSED*/
static VALUE
f_assoc(int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_ASSOC;
    result.v_subtype = V_NOSUBTYPE;

    result.v_assoc = assocalloc(0L);
    return result;
}

static VALUE
f_indices(VALUE *v1, VALUE *v2)
{
    VALUE result;
    LIST *lp;

    if (v2->v_type != V_NUM || zge31b(v2->v_num->num)) {
        return error_value(E_INDICES_2);
    }

    switch (v1->v_type) {
    case V_ASSOC:
        lp = associndices(v1->v_assoc, qtoi(v2->v_num));
        break;
    case V_MAT:
        lp = matindices(v1->v_mat, qtoi(v2->v_num));
        break;
    default:
        return error_value(E_INDICES_1);
    }

    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;
    if (lp) {
        result.v_type = V_LIST;
        result.v_list = lp;
    }
    return result;
}

static VALUE
f_listinsert(int count, VALUE **vals)
{
    VALUE *v1, *v2, *v3;
    VALUE result;
    long pos;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    v1 = *vals++;
    if ((v1->v_type != V_ADDR) || (v1->v_addr->v_type != V_LIST)) {
        return error_value(E_INSERT_1);
    }
    if (v1->v_addr->v_subtype & V_NOREALLOC) {
        return error_value(E_LIST_1);
    }

    v2 = *vals++;
    if (v2->v_type == V_ADDR) {
        v2 = v2->v_addr;
    }
    if ((v2->v_type != V_NUM) || qisfrac(v2->v_num)) {
        return error_value(E_INSERT_2);
    }
    pos = qtoi(v2->v_num);
    count--;
    while (--count > 0) {
        v3 = *vals++;
        if (v3->v_type == V_ADDR) {
            v3 = v3->v_addr;
        }
        insertlistmiddle(v1->v_addr->v_list, pos++, v3);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_listpush(int count, VALUE **vals)
{
    VALUE result;
    VALUE *v1, *v2;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    v1 = *vals++;
    if ((v1->v_type != V_ADDR) || (v1->v_addr->v_type != V_LIST)) {
        return error_value(E_PUSH);
    }
    if (v1->v_addr->v_subtype & V_NOREALLOC) {
        return error_value(E_LIST_3);
    }

    while (--count > 0) {
        v2 = *vals++;
        if (v2->v_type == V_ADDR) {
            v2 = v2->v_addr;
        }
        insertlistfirst(v1->v_addr->v_list, v2);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_listappend(int count, VALUE **vals)
{
    VALUE *v1, *v2;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    v1 = *vals++;
    if ((v1->v_type != V_ADDR) || (v1->v_addr->v_type != V_LIST)) {
        return error_value(E_APPEND);
    }
    if (v1->v_addr->v_subtype & V_NOREALLOC) {
        return error_value(E_LIST_4);
    }

    while (--count > 0) {
        v2 = *vals++;
        if (v2->v_type == V_ADDR) {
            v2 = v2->v_addr;
        }
        insertlistlast(v1->v_addr->v_list, v2);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_listdelete(VALUE *v1, VALUE *v2)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if ((v1->v_type != V_ADDR) || (v1->v_addr->v_type != V_LIST)) {
        return error_value(E_DELETE_1);
    }
    if (v1->v_addr->v_subtype & V_NOREALLOC) {
        return error_value(E_LIST_2);
    }

    if (v2->v_type == V_ADDR) {
        v2 = v2->v_addr;
    }
    if ((v2->v_type != V_NUM) || qisfrac(v2->v_num)) {
        return error_value(E_DELETE_2);
    }
    removelistmiddle(v1->v_addr->v_list, qtoi(v2->v_num), &result);
    return result;
}

static VALUE
f_listpop(VALUE *vp)
{
    VALUE result;

    if ((vp->v_type != V_ADDR) || (vp->v_addr->v_type != V_LIST)) {
        return error_value(E_POP);
    }

    if (vp->v_addr->v_subtype & V_NOREALLOC) {
        return error_value(E_LIST_5);
    }

    removelistfirst(vp->v_addr->v_list, &result);
    return result;
}

static VALUE
f_listremove(VALUE *vp)
{
    VALUE result;

    if ((vp->v_type != V_ADDR) || (vp->v_addr->v_type != V_LIST)) {
        return error_value(E_REMOVE);
    }

    if (vp->v_addr->v_subtype & V_NOREALLOC) {
        return error_value(E_LIST_6);
    }

    removelistlast(vp->v_addr->v_list, &result);
    return result;
}

/*
 * Return the current user time of calc in seconds.
 */
static NUMBER *
f_usertime(void)
{
#  if defined(HAVE_GETRUSAGE)

    struct rusage usage;   /* system resource usage */
    int who = RUSAGE_SELF; /* obtain time for just this process */
    int status;            /* getrusage() return code */
    NUMBER *ret;           /* CPU time to return */
    NUMBER *secret;        /* whole seconds of CPU time to return */
    NUMBER *usecret;       /* microseconds of CPU time to return */

    /* get the resource information for ourself */
    status = getrusage(who, &usage);
    if (status < 0) {
        /* system call error, so return 0 */
        return qlink(&_qzero_);
    }

    /* add user time */
    secret = stoq(usage.ru_utime.tv_sec);
    usecret = iitoq((long)usage.ru_utime.tv_usec, 1000000L);
    ret = qqadd(secret, usecret);
    qfree(secret);
    qfree(usecret);

    /* return user CPU time */
    return ret;

#  else

    /* not a POSIX system */
    return qlink(&_qzero_);

#  endif
}

/*
 * Return the current kernel time of calc in seconds.
 * This is the kernel mode time only.
 */
static NUMBER *
f_systime(void)
{
#  if defined(HAVE_GETRUSAGE)

    struct rusage usage;   /* system resource usage */
    int who = RUSAGE_SELF; /* obtain time for just this process */
    int status;            /* getrusage() return code */
    NUMBER *ret;           /* CPU time to return */
    NUMBER *secret;        /* whole seconds of CPU time to return */
    NUMBER *usecret;       /* microseconds of CPU time to return */

    /* get the resource information for ourself */
    status = getrusage(who, &usage);
    if (status < 0) {
        /* system call error, so return 0 */
        return qlink(&_qzero_);
    }

    /* add kernel time */
    secret = stoq(usage.ru_stime.tv_sec);
    usecret = iitoq((long)usage.ru_stime.tv_usec, 1000000L);
    ret = qqadd(secret, usecret);
    qfree(secret);
    qfree(usecret);

    /* return kernel CPU time */
    return ret;

#  else

    /* not a POSIX system */
    return qlink(&_qzero_);

#  endif
}

/*
 * Return the current user and kernel time of calc in seconds.
 */
static NUMBER *
f_runtime(void)
{
#  if defined(HAVE_GETRUSAGE)

    struct rusage usage;   /* system resource usage */
    int who = RUSAGE_SELF; /* obtain time for just this process */
    int status;            /* getrusage() return code */
    NUMBER *user;          /* user CPU time to return */
    NUMBER *sys;           /* kernel CPU time to return */
    NUMBER *ret;           /* total CPU time to return */
    NUMBER *secret;        /* whole seconds of CPU time to return */
    NUMBER *usecret;       /* microseconds of CPU time to return */

    /* get the resource information for ourself */
    status = getrusage(who, &usage);
    if (status < 0) {
        /* system call error, so return 0 */
        return qlink(&_qzero_);
    }

    /* add kernel time */
    secret = stoq(usage.ru_stime.tv_sec);
    usecret = iitoq((long)usage.ru_stime.tv_usec, 1000000L);
    sys = qqadd(secret, usecret);
    qfree(secret);
    qfree(usecret);

    /* add user time */
    secret = stoq(usage.ru_utime.tv_sec);
    usecret = iitoq((long)usage.ru_utime.tv_usec, 1000000L);
    user = qqadd(secret, usecret);
    qfree(secret);
    qfree(usecret);

    /* total time is user + kernel */
    ret = qqadd(user, sys);
    qfree(user);
    qfree(sys);

    /* return CPU time */
    return ret;

#  else

    /* not a POSIX system */
    return qlink(&_qzero_);

#  endif
}

/*
 * return the number of second since the Epoch (00:00:00 1 Jan 1970 UTC).
 */
static NUMBER *
f_time(void)
{
    return itoq((long)time(0));
}

/*
 * time in asctime()/ctime() format
 */
static VALUE
f_ctime(void)
{
    VALUE res;
    time_t now; /* the current time */

    /* initialize VALUE */
    res.v_subtype = V_NOSUBTYPE;
    res.v_type = V_STR;

    /* get the time */
    now = time(NULL);
    res.v_str = makenewstring(ctime(&now));
    return res;
}

static VALUE
f_fopen(VALUE *v1, VALUE *v2)
{
    VALUE result;
    FILEID id;
    char *mode;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /* check for a valid mode [rwa][b+\0][b+\0] */
    if (v1->v_type != V_STR || v2->v_type != V_STR) {
        return error_value(E_FOPEN_1);
    }
    mode = v2->v_str->s_str;
    if ((*mode != 'r') && (*mode != 'w') && (*mode != 'a')) {
        return error_value(E_FOPEN_2);
    }
    if (mode[1] != '\0') {
        if (mode[1] != '+' && mode[1] != 'b') {
            return error_value(E_FOPEN_2);
        }
        if (mode[2] != '\0') {
            if ((mode[2] != '+' && mode[2] != 'b') || mode[1] == mode[2]) {
                return error_value(E_FOPEN_2);
            }
            if (mode[3] != '\0') {
                return error_value(E_FOPEN_2);
            }
        }
    }

    /* try to open */
    errno = 0;
    id = openid(v1->v_str->s_str, v2->v_str->s_str);
    if (id == FILEID_NONE) {
        return error_value(errno);
    }
    if (id < 0) {
        return error_value(-id);
    }
    result.v_type = V_FILE;
    result.v_file = id;
    return result;
}

static VALUE
f_fpathopen(int count, VALUE **vals)
{
    VALUE result;
    FILEID id;
    char *mode;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /* check for valid strong */
    if (vals[0]->v_type != V_STR || vals[1]->v_type != V_STR) {
        return error_value(E_FPATHOPEN_1);
    }
    if (count == 3 && vals[2]->v_type != V_STR) {
        return error_value(E_FPATHOPEN_1);
    }

    /* check for a valid mode [rwa][b+\0][b+\0] */
    mode = vals[1]->v_str->s_str;
    if ((*mode != 'r') && (*mode != 'w') && (*mode != 'a')) {
        return error_value(E_FPATHOPEN_2);
    }
    if (mode[1] != '\0') {
        if (mode[1] != '+' && mode[1] != 'b') {
            return error_value(E_FPATHOPEN_2);
        }
        if (mode[2] != '\0') {
            if ((mode[2] != '+' && mode[2] != 'b') || mode[1] == mode[2]) {
                return error_value(E_FPATHOPEN_2);
            }
            if (mode[3] != '\0') {
                return error_value(E_FPATHOPEN_2);
            }
        }
    }

    /* try to open along a path */
    errno = 0;
    if (count == 2) {
        id = openpathid(vals[0]->v_str->s_str, vals[1]->v_str->s_str, calcpath);
    } else {
        id = openpathid(vals[0]->v_str->s_str, vals[1]->v_str->s_str, vals[2]->v_str->s_str);
    }
    if (id == FILEID_NONE) {
        return error_value(errno);
    }
    if (id < 0) {
        return error_value(-id);
    }
    result.v_type = V_FILE;
    result.v_file = id;
    return result;
}

static VALUE
f_freopen(int count, VALUE **vals)
{
    VALUE result;
    FILEID id;
    char *mode;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /* check for a valid mode [rwa][b+\0][b+\0] */
    if (vals[0]->v_type != V_FILE) {
        return error_value(E_FREOPEN_1);
    }
    if (vals[1]->v_type != V_STR) {
        return error_value(E_FREOPEN_2);
    }
    mode = vals[1]->v_str->s_str;
    if ((*mode != 'r') && (*mode != 'w') && (*mode != 'a')) {
        return error_value(E_FREOPEN_2);
    }
    if (mode[1] != '\0') {
        if (mode[1] != '+' && mode[1] != 'b') {
            return error_value(E_FREOPEN_2);
        }
        if (mode[2] != '\0') {
            if ((mode[2] != '+' && mode[2] != 'b') || mode[1] == mode[2]) {
                return error_value(E_FOPEN_2);
            }
            if (mode[3] != '\0') {
                return error_value(E_FREOPEN_2);
            }
        }
    }

    /* try to reopen */
    errno = 0;
    if (count == 2) {
        id = reopenid(vals[0]->v_file, mode, NULL);
    } else {
        if (vals[2]->v_type != V_STR) {
            return error_value(E_FREOPEN_3);
        }
        id = reopenid(vals[0]->v_file, mode, vals[2]->v_str->s_str);
    }

    if (id == FILEID_NONE) {
        return error_value(errno);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_fclose(int count, VALUE **vals)
{
    VALUE result;
    VALUE *vp;
    int n, i = 0;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    errno = 0;
    if (count == 0) {
        i = closeall();
    } else {
        for (n = 0; n < count; n++) {
            vp = vals[n];
            if (vp->v_type != V_FILE) {
                return error_value(E_FCLOSE_1);
            }
        }
        for (n = 0; n < count; n++) {
            vp = vals[n];
            i = closeid(vp->v_file);
            if (i < 0) {
                return error_value(E_REWIND_2);
            }
        }
    }
    if (i < 0) {
        return error_value(errno);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_rm(int count, VALUE **vals)
{
    VALUE result;
    int force; /* true -> -f was given as 1st arg */
    int i;
    int j;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * firewall
     */
    if (!allow_write) {
        return error_value(E_WRPERM);
    }

    /*
     * check on each arg
     */
    for (i = 0; i < count; ++i) {
        if (vals[i]->v_type != V_STR) {
            return error_value(E_RM_1);
        }
        if (vals[i]->v_str->s_str[0] == '\0') {
            return error_value(E_RM_1);
        }
    }

    /*
     * look for a leading -f option
     */
    force = (strcmp(vals[0]->v_str->s_str, "-f") == 0);
    if (force) {
        --count;
        ++vals;
    }

    /*
     * remove file(s)
     */
    for (i = 0; i < count; ++i) {
        j = remove(vals[i]->v_str->s_str);
        if (!force && j < 0) {
            return error_value(errno);
        }
    }
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;
    return result;
}

static VALUE
f_error(int count, VALUE **vals)
{
    VALUE *vp;
    long newerr;

    /*
     * case: error() no args
     */
    if (count == 0) {

        /* fetch but do NOT set errno */
        newerr = set_errno(NULL_ERRNUM);

        /*
         * case: 1 arg
         */
    } else {
        vp = vals[0]; /* get 1st arg */

        /*
         * case: negative or 0 v_type
         */
        if (vp->v_type <= 0) {
            newerr = (long)-vp->v_type;
            if (is_valid_errnum(newerr) == false) {
                return error_value(E_ERROR_2);
            }

            /*
             * case: error(errnum | "E_STRING") arg
             */
        } else {
            switch (vp->v_type) {

            /*
             * case: error("E_STRING")
             */
            case V_STR:
                newerr = errsym_2_errnum(vp->v_str->s_str);
                if (is_valid_errnum(newerr) == false) {
                    return error_value(E_ERROR_3);
                }
                break;

            /*
             * case: error(errnum)
             */
            case V_NUM:
                if (qisfrac(vp->v_num)) {
                    return error_value(E_ERROR_4);
                }
                newerr = qtoi(vp->v_num);
                if (is_valid_errnum(newerr) == false) {
                    return error_value(E_ERROR_2);
                }
                break;

            /*
             * case: invalid type
             */
            default:
                return error_value(E_ERROR_1);
            }
        }
    }

    /*
     * return error
     */
    return error_value(newerr);
}

static VALUE
f_errno(int count, VALUE **vals)
{
    int olderr;               /* previous errno value */
    int newerr = NULL_ERRNUM; /* new errno to set */
    VALUE *vp;                /* arg[1] */
    VALUE result;             /* errno as a VALUE */

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * case: errno() no args
     */
    if (count == 0) {

        /* fetch but do NOT set errno */
        olderr = set_errno(NULL_ERRNUM);

        /*
         * case: 1 arg
         */
    } else {
        vp = vals[0]; /* get 1st arg */

        /*
         * case: negative or 0 v_type
         */
        if (vp->v_type <= 0) {
            newerr = (int)-vp->v_type;
            if (is_valid_errnum(newerr) == false) {
                return error_value(E_ERRNO_2);
            }

            /*
             * case: errno(errnum | "E_STRING") arg
             */
        } else {
            switch (vp->v_type) {

            /*
             * case: errno("E_STRING")
             */
            case V_STR:
                newerr = errsym_2_errnum(vp->v_str->s_str);
                if (is_valid_errnum(newerr) == false) {
                    return error_value(E_ERRNO_3);
                }
                break;

            /*
             * case: errno(errnum)
             */
            case V_NUM:
                if (qisfrac(vp->v_num)) {
                    return error_value(E_ERRNO_4);
                }
                newerr = qtoi(vp->v_num);
                if (is_valid_errnum(newerr) == false) {
                    return error_value(E_ERRNO_2);
                }
                break;

            /*
             * case: invalid type
             */
            default:
                return error_value(E_ERRNO_1);
            }
        }
    }

    /*
     * return errno
     */
    olderr = set_errno(newerr);
    result.v_num = itoq((long)olderr);
    return result;
}

static VALUE
f_strerror(int count, VALUE **vals)
{
    int errnum = NULL_ERRNUM; /* errnum to convert */
    char *errmsg;             /* errnum converted into errmsg string, or NULL */
    bool alloced = false;     /* true ==> errmsg was allocated, false ==> errmsg is static */
    VALUE *vp;                /* arg[1] */
    VALUE result;             /* errmsg string as a VALUE */

    /* initialize VALUE */
    result.v_type = V_STR;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * case: strerror() no args
     */
    if (count == 0) {

        /* fetch but do NOT set errno */
        errnum = set_errno(NULL_ERRNUM);

        /*
         * case: 1 arg
         */
    } else {
        vp = vals[0]; /* get 1st arg */

        /*
         * case: negative or 0 v_type
         */
        if (vp->v_type <= 0) {
            errnum = (int)-vp->v_type;
            if (is_valid_errnum(errnum) == false) {
                return error_value(E_STRERROR_2);
            }

            /*
             * case: strerror(errnum | "E_STRING") arg
             */
        } else {
            switch (vp->v_type) {

            /*
             * case: strerror("E_STRING")
             */
            case V_STR:
                errnum = errsym_2_errnum(vp->v_str->s_str);
                if (is_valid_errnum(errnum) == false) {
                    return error_value(E_STRERROR_3);
                }
                break;

            /*
             * case: strerror(errnum)
             */
            case V_NUM:
                if (qisfrac(vp->v_num)) {
                    return error_value(E_STRERROR_5);
                }
                errnum = qtoi(vp->v_num);
                if (is_valid_errnum(errnum) == false) {
                    return error_value(E_STRERROR_2);
                }
                break;

            /*
             * case: invalid type
             */
            default:
                return error_value(E_STRERROR_1);
            }
        }
    }

    /*
     * convert errnum into errmsg string
     */
    errmsg = errnum_2_errmsg(errnum, &alloced);
    if (errmsg == NULL) {
        /* this should not happen: but in case it does we will throw an error */
        return error_value(E_STRERROR_4);
    }
    result.v_str = makenewstring(errmsg);

    /*
     * free errmsg is it was allocated
     */
    if (alloced == true) {
        free(errmsg);
        alloced = false;
        errmsg = NULL;
    }

    /*
     * return errmsg result as a V_STR
     */
    return result;
}

static VALUE
f_errsym(VALUE *vp)
{
    int errnum = NULL_ERRNUM; /* global calc_errno value */
    bool alloced = false;     /* true ==> errsym is allocated, false ==> errsym is static */
    char *errsym;             /* converted errsym or NULL */
    VALUE result;             /* errno as a VALUE */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * case: negative or 0 v_type OR errno(errnum)
     */
    if (vp->v_type <= 0 || vp->v_type == V_NUM) {

        /*
         * case: negative or 0 v_type
         */
        if (vp->v_type <= 0) {

            /* convert negative type into a errnum calc_errno-like value */
            errnum = (int)-vp->v_type;

            /*
             * case: errno(errnum)
             */
        } else {

            /* use arg[1] integer */
            if (qisfrac(vp->v_num)) {
                return error_value(E_ERRSYM_4);
            }
            errnum = qtoi(vp->v_num);
        }

        /*
         * case: invalid errnum
         */
        if (is_valid_errnum(errnum) == false) {
            return error_value(E_ERRSYM_2);
        }

        /*
         * convert errnum code into errsym "E_STRING"
         */
        errsym = errnum_2_errsym(errnum, &alloced);
        if (errsym == NULL) {
            return error_value(E_ERRSYM_5);
        }
        result.v_type = V_STR;
        result.v_str = makenewstring(errsym);
        if (alloced == true) {
            free(errsym);
            errsym = NULL;
            alloced = false;
        }

        /*
         * case: errno("E_STRING") arg
         */
    } else if (vp->v_type == V_STR) {

        /*
         * convert E_STRING errsym to errno
         */
        errnum = errsym_2_errnum(vp->v_str->s_str);
        if (is_valid_errnum(errnum) == false) {
            return error_value(E_ERRSYM_3);
        }
        result.v_type = V_NUM;
        result.v_num = itoq((long)errnum);
    }

    /*
     * return result
     */
    return result;
}

static VALUE
f_errcount(int count, VALUE **vals)
{
    int newcount, oldcount;
    VALUE *vp;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    newcount = -1;
    if (count > 0) {
        vp = vals[0];

        /* arg must be an integer */
        if (vp->v_type != V_NUM || qisfrac(vp->v_num) || qisneg(vp->v_num) || zge31b(vp->v_num->num)) {
            math_error("errcount argument out of range");
            not_reached();
        }
        newcount = (int)ztoi(vp->v_num->num);
    }
    oldcount = set_errcount(newcount);

    result.v_type = V_NUM;
    result.v_num = itoq((long)oldcount);
    return result;
}

static VALUE
f_errmax(int count, VALUE **vals)
{
    long oldmax;
    VALUE *vp;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    oldmax = errmax;
    if (count > 0) {
        vp = vals[0];

        if (vp->v_type != V_NUM || qisfrac(vp->v_num) || zge31b(vp->v_num->num) || zltnegone(vp->v_num->num)) {
            fprintf(stderr, "Out-of-range arg for errmax ignored\n");
        } else {
            errmax = ztoi(vp->v_num->num);
        }
    }

    result.v_type = V_NUM;
    result.v_num = itoq((long)oldmax);
    return result;
}

static VALUE
f_stoponerror(int count, VALUE **vals)
{
    long oldval;
    VALUE *vp;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    oldval = stoponerror;
    if (count > 0) {
        vp = vals[0];

        if (vp->v_type != V_NUM || qisfrac(vp->v_num) || zge31b(vp->v_num->num) || zltnegone(vp->v_num->num)) {
            fprintf(stderr, "Out-of-range arg for stoponerror ignored\n");
        } else {
            stoponerror = ztoi(vp->v_num->num);
        }
    }

    result.v_type = V_NUM;
    result.v_num = itoq((long)oldval);
    return result;
}

static VALUE
f_iserror(VALUE *vp)
{
    VALUE res;

    /* initialize VALUE */
    res.v_subtype = V_NOSUBTYPE;

    res.v_type = V_NUM;
    res.v_num = itoq((long)((vp->v_type < 0) ? -vp->v_type : 0));
    return res;
}

static VALUE
f_newerror(int count, VALUE **vals)
{
    char *str;
    int index;
    int errnum;

    str = NULL;
    if (count > 0 && vals[0]->v_type == V_STR) {
        str = vals[0]->v_str->s_str;
    }
    if (str == NULL || str[0] == '\0') {
        str = "???";
    }
    if (nexterrnum == E__USERDEF) {
        initstr(&newerrorstr);
    }
    index = findstr(&newerrorstr, str);
    if (index >= 0) {
        errnum = E__USERDEF + index;
    } else {
        if (nexterrnum == E__USERMAX) {
            math_error("Too many new error values");
        }
        errnum = nexterrnum++;
        addstr(&newerrorstr, str);
    }
    return error_value(errnum);
}

static VALUE
f_ferror(VALUE *vp)
{
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FERROR_1);
    }
    i = errorid(vp->v_file);
    if (i < 0) {
        return error_value(E_FERROR_2);
    }
    result.v_type = V_NUM;
    result.v_num = itoq((long)i);
    return result;
}

static VALUE
f_feof(VALUE *vp)
{
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FEOF_1);
    }
    i = eofid(vp->v_file);
    if (i < 0) {
        return error_value(E_FEOF_2);
    }
    result.v_type = V_NUM;
    result.v_num = itoq((long)i);
    return result;
}

static VALUE
f_fflush(int count, VALUE **vals)
{
    VALUE result;
    int i, n;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    i = 0;
    errno = 0;
    if (count == 0) {
#  if !defined(_WIN32) && !defined(_WIN64)

        i = flushall();

#  endif
    } else {
        for (n = 0; n < count; n++) {
            if (vals[n]->v_type != V_FILE) {
                return error_value(E_FFLUSH);
            }
        }
        for (n = 0; n < count; n++) {
            i |= flushid(vals[n]->v_file);
        }
    }
    if (i == EOF) {
        return error_value(errno);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_fsize(VALUE *vp)
{
    VALUE result;
    ZVALUE len; /* file length */
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FSIZE_1);
    }
    i = getsize(vp->v_file, &len);
    if (i == EOF) {
        return error_value(errno);
    }
    if (i) {
        return error_value(E_FSIZE_2);
    }
    result.v_type = V_NUM;
    result.v_num = qalloc();
    result.v_num->num = len;
    return result;
}

static VALUE
f_fseek(int count, VALUE **vals)
{
    VALUE result;
    int whence;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /* firewalls */
    errno = 0;
    if (vals[0]->v_type != V_FILE) {
        return error_value(E_FSEEK_1);
    }
    if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num)) {
        return error_value(E_FSEEK_2);
    }
    if (count == 2) {
        whence = 0;
    } else {
        if (vals[2]->v_type != V_NUM || qisfrac(vals[2]->v_num) || qisneg(vals[2]->v_num)) {
            return error_value(E_FSEEK_2);
        }
        if (vals[2]->v_num->num.len > 1) {
            return error_value(E_FSEEK_2);
        }
        whence = (int)(unsigned int)(vals[2]->v_num->num.v[0]);
        if (whence > 2) {
            return error_value(E_FSEEK_2);
        }
    }

    i = fseekid(vals[0]->v_file, vals[1]->v_num->num, whence);
    result.v_type = V_NULL;
    if (i == EOF) {
        return error_value(errno);
    }
    if (i < 0) {
        return error_value(E_FSEEK_3);
    }
    return result;
}

static VALUE
f_ftell(VALUE *vp)
{
    VALUE result;
    ZVALUE pos; /* current file position */
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    errno = 0;
    if (vp->v_type != V_FILE) {
        return error_value(E_FTELL_1);
    }
    i = ftellid(vp->v_file, &pos);
    if (i < 0) {
        return error_value(E_FTELL_2);
    }

    result.v_type = V_NUM;
    result.v_num = qalloc();
    result.v_num->num = pos;
    return result;
}

static VALUE
f_rewind(int count, VALUE **vals)
{
    VALUE result;
    int n;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (count == 0) {
        rewindall();

    } else {
        for (n = 0; n < count; n++) {
            if (vals[n]->v_type != V_FILE) {
                return error_value(E_REWIND_1);
            }
        }
        for (n = 0; n < count; n++) {
            if (rewindid(vals[n]->v_file) != 0) {
                return error_value(E_REWIND_2);
            }
        }
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_fprintf(int count, VALUE **vals)
{
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vals[0]->v_type != V_FILE) {
        return error_value(E_FPRINTF_1);
    }
    if (vals[1]->v_type != V_STR) {
        return error_value(E_FPRINTF_2);
    }
    i = idprintf(vals[0]->v_file, vals[1]->v_str->s_str, count - 2, vals + 2);
    if (i > 0) {
        return error_value(E_FPRINTF_3);
    }
    result.v_type = V_NULL;
    return result;
}

static int
strscan(char *s, int count, VALUE **vals)
{
    char ch, chtmp;
    char *s0;
    int n = 0;
    VALUE val, result;
    VALUE *var;

    /* initialize VALUEs */
    val.v_subtype = V_NOSUBTYPE;
    result.v_subtype = V_NOSUBTYPE;

    val.v_type = V_STR;
    while (*s != '\0') {
        s--;
        while ((ch = *++s)) {
            if (!isspace((int)ch)) {
                break;
            }
        }
        if (ch == '\0' || count-- == 0) {
            return n;
        }
        s0 = s;
        while ((ch = *++s)) {
            if (isspace((int)ch)) {
                break;
            }
        }
        chtmp = ch;
        *s = '\0';
        n++;
        val.v_str = makenewstring(s0);
        result = f_eval(&val);
        var = *vals++;
        if (var->v_type == V_ADDR) {
            var = var->v_addr;
            freevalue(var);
            *var = result;
        }
        *s = chtmp;
    }
    return n;
}

static int
filescan(FILEID id, int count, VALUE **vals)
{
    STRING *str;
    int i;
    int n = 0;
    VALUE val;
    VALUE result;
    VALUE *var;

    /* initialize VALUEs */
    val.v_type = V_STR;
    val.v_subtype = V_NOSUBTYPE;
    result.v_subtype = V_NOSUBTYPE;

    while (count-- > 0) {

        i = readid(id, 6, &str);

        if (i == EOF) {
            break;
        }
        if (i > 0) {
            return EOF;
        }
        n++;
        val.v_str = str;
        result = f_eval(&val);
        var = *vals++;
        if (var->v_type == V_ADDR) {
            var = var->v_addr;
            freevalue(var);
            *var = result;
        }
    }
    return n;
}

static VALUE
f_scan(int count, VALUE **vals)
{
    char *cp;
    VALUE result;
    int i;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    cp = nextline();
    if (cp == NULL) {
        result.v_type = V_NULL;
        return result;
    }

    i = strscan(cp, count, vals);
    result.v_type = V_NUM;
    result.v_num = itoq((long)i);
    return result;
}

static VALUE
f_strscan(int count, VALUE **vals)
{
    VALUE *vp;
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    vp = *vals;
    if (vp->v_type == V_ADDR) {
        vp = vp->v_addr;
    }
    if (vp->v_type != V_STR) {
        return error_value(E_STRSCAN);
    }

    i = strscan(vp->v_str->s_str, count - 1, vals + 1);

    result.v_type = V_NUM;
    result.v_num = itoq((long)i);
    return result;
}

static VALUE
f_fscan(int count, VALUE **vals)
{
    VALUE *vp;
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    errno = 0;
    vp = *vals;
    if (vp->v_type == V_ADDR) {
        vp = vp->v_addr;
    }
    if (vp->v_type != V_FILE) {
        return error_value(E_FSCAN_1);
    }

    i = filescan(vp->v_file, count - 1, vals + 1);

    if (i == EOF) {
        return error_value(errno);
    }
    if (i < 0) {
        return error_value(E_FSCAN_2);
    }

    result.v_type = V_NUM;
    result.v_num = itoq((long)i);
    return result;
}

static VALUE
f_scanf(int count, VALUE **vals)
{
    VALUE *vp;
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    vp = *vals;
    if (vp->v_type == V_ADDR) {
        vp = vp->v_addr;
    }
    if (vp->v_type != V_STR) {
        return error_value(E_SCANF_1);
    }
    for (i = 1; i < count; i++) {
        if (vals[i]->v_type != V_ADDR) {
            return error_value(E_SCANF_2);
        }
    }
    i = fscanfid(FILEID_STDIN, vp->v_str->s_str, count - 1, vals + 1);
    if (i < 0) {
        return error_value(E_SCANF_3);
    }
    result.v_type = V_NUM;
    result.v_num = itoq((long)i);
    return result;
}

static VALUE
f_strscanf(int count, VALUE **vals)
{
    VALUE *vp, *vq;
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    errno = 0;
    vp = vals[0];
    if (vp->v_type == V_ADDR) {
        vp = vp->v_addr;
    }
    if (vp->v_type != V_STR) {
        return error_value(E_STRSCANF_1);
    }
    vq = vals[1];
    if (vq->v_type == V_ADDR) {
        vq = vq->v_addr;
    }
    if (vq->v_type != V_STR) {
        return error_value(E_STRSCANF_2);
    }
    for (i = 2; i < count; i++) {
        if (vals[i]->v_type != V_ADDR) {
            return error_value(E_STRSCANF_3);
        }
    }
    i = scanfstr(vp->v_str->s_str, vq->v_str->s_str, count - 2, vals + 2);
    if (i == EOF) {
        return error_value(errno);
    }
    if (i < 0) {
        return error_value(E_STRSCANF_4);
    }
    result.v_type = V_NUM;
    result.v_num = itoq((long)i);
    return result;
}

static VALUE
f_fscanf(int count, VALUE **vals)
{
    VALUE *vp, *sp;
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    vp = *vals++;
    if (vp->v_type == V_ADDR) {
        vp = vp->v_addr;
    }
    if (vp->v_type != V_FILE) {
        return error_value(E_FSCANF_1);
    }
    sp = *vals++;
    if (sp->v_type == V_ADDR) {
        sp = sp->v_addr;
    }
    if (sp->v_type != V_STR) {
        return error_value(E_FSCANF_2);
    }
    for (i = 0; i < count - 2; i++) {
        if (vals[i]->v_type != V_ADDR) {
            return error_value(E_FSCANF_3);
        }
    }
    i = fscanfid(vp->v_file, sp->v_str->s_str, count - 2, vals);
    if (i == EOF) {
        result.v_type = V_NULL;
        return result;
    }
    if (i < 0) {
        return error_value(E_FSCANF_4);
    }
    result.v_type = V_NUM;
    result.v_num = itoq((long)i);
    return result;
}

static VALUE
f_fputc(VALUE *v1, VALUE *v2)
{
    VALUE result;
    NUMBER *q;
    int ch;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_FILE) {
        return error_value(E_FPUTC_1);
    }
    switch (v2->v_type) {
    case V_STR:
        ch = v2->v_str->s_str[0];
        break;
    case V_NUM:
        q = v2->v_num;
        if (!qisint(q)) {
            return error_value(E_FPUTC_2);
        }

        ch = qisneg(q) ? (int)(-q->num.v[0] & 0xff) : (int)(q->num.v[0] & 0xff);
        break;
    case V_NULL:
        ch = 0;
        break;
    default:
        return error_value(E_FPUTC_2);
    }
    i = idfputc(v1->v_file, ch);
    if (i > 0) {
        return error_value(E_FPUTC_3);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_fputs(int count, VALUE **vals)
{
    VALUE result;
    int i, err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vals[0]->v_type != V_FILE) {
        return error_value(E_FPUTS_1);
    }
    for (i = 1; i < count; i++) {
        if (vals[i]->v_type != V_STR) {
            return error_value(E_FPUTS_2);
        }
    }
    for (i = 1; i < count; i++) {
        err = idfputs(vals[0]->v_file, vals[i]->v_str);
        if (err > 0) {
            return error_value(E_FPUTS_3);
        }
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_fputstr(int count, VALUE **vals)
{
    VALUE result;
    int i, err;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vals[0]->v_type != V_FILE) {
        return error_value(E_FPUTSTR_1);
    }
    for (i = 1; i < count; i++) {
        if (vals[i]->v_type != V_STR) {
            return error_value(E_FPUTSTR_2);
        }
    }
    for (i = 1; i < count; i++) {
        err = idfputstr(vals[0]->v_file, vals[i]->v_str->s_str);
        if (err > 0) {
            return error_value(E_FPUTSTR_3);
        }
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_printf(int count, VALUE **vals)
{
    VALUE result;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vals[0]->v_type != V_STR) {
        return error_value(E_PRINTF_1);
    }
    i = idprintf(FILEID_STDOUT, vals[0]->v_str->s_str, count - 1, vals + 1);
    if (i) {
        return error_value(E_PRINTF_2);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_strprintf(int count, VALUE **vals)
{
    VALUE result;
    int i;
    char *cp;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vals[0]->v_type != V_STR) {
        return error_value(E_STRPRINTF_1);
    }
    math_divertio();
    i = idprintf(FILEID_STDOUT, vals[0]->v_str->s_str, count - 1, vals + 1);
    if (i) {
        free(math_getdivertedio());
        return error_value(E_STRPRINTF_2);
    }
    cp = math_getdivertedio();
    result.v_type = V_STR;
    result.v_str = makenewstring(cp);
    free(cp);
    return result;
}

static VALUE
f_fgetc(VALUE *vp)
{
    VALUE result;
    int ch;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FGETC_1);
    }
    ch = getcharid(vp->v_file);
    if (ch == -2) {
        return error_value(E_FGETC_2);
    }
    result.v_type = V_NULL;
    if (ch != EOF) {
        result.v_type = V_STR;
        result.v_str = charstring(ch);
    }
    return result;
}

static VALUE
f_ungetc(VALUE *v1, VALUE *v2)
{
    VALUE result;
    NUMBER *q;
    int ch;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    errno = 0;
    if (v1->v_type != V_FILE) {
        return error_value(E_UNGETC_1);
    }
    switch (v2->v_type) {
    case V_STR:
        ch = v2->v_str->s_str[0];
        break;
    case V_NUM:
        q = v2->v_num;
        if (!qisint(q)) {
            return error_value(E_UNGETC_2);
        }
        ch = qisneg(q) ? (int)(-q->num.v[0] & 0xff) : (int)(q->num.v[0] & 0xff);
        break;
    default:
        return error_value(E_UNGETC_2);
    }
    i = idungetc(v1->v_file, ch);
    if (i == EOF) {
        return error_value(errno);
    }
    if (i == -2) {
        return error_value(E_UNGETC_3);
    }
    result.v_type = V_NULL;
    return result;
}

static VALUE
f_fgetline(VALUE *vp)
{
    VALUE result;
    STRING *str;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FGETLINE_1);
    }
    i = readid(vp->v_file, 9, &str);
    if (i > 0) {
        return error_value(E_FGETLINE_2);
    }
    result.v_type = V_NULL;
    if (i == 0) {
        result.v_type = V_STR;
        result.v_str = str;
    }
    return result;
}

static VALUE
f_fgets(VALUE *vp)
{
    VALUE result;
    STRING *str;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FGETS_1);
    }
    i = readid(vp->v_file, 1, &str);
    if (i > 0) {
        return error_value(E_FGETS_2);
    }
    result.v_type = V_NULL;
    if (i == 0) {
        result.v_type = V_STR;
        result.v_str = str;
    }
    return result;
}

static VALUE
f_fgetstr(VALUE *vp)
{
    VALUE result;
    STRING *str;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FGETSTR_1);
    }
    i = readid(vp->v_file, 10, &str);
    if (i > 0) {
        return error_value(E_FGETSTR_2);
    }
    result.v_type = V_NULL;
    if (i == 0) {
        result.v_type = V_STR;
        result.v_str = str;
    }
    return result;
}

static VALUE
f_fgetfield(VALUE *vp)
{
    VALUE result;
    STRING *str;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FGETFIELD_1);
    }
    i = readid(vp->v_file, 14, &str);
    if (i > 0) {
        return error_value(E_FGETFIELD_2);
    }
    result.v_type = V_NULL;
    if (i == 0) {
        result.v_type = V_STR;
        result.v_str = str;
    }
    return result;
}

static VALUE
f_fgetfile(VALUE *vp)
{
    VALUE result;
    STRING *str;
    int i;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_FILE) {
        return error_value(E_FGETFILE_1);
    }
    i = readid(vp->v_file, 0, &str);
    if (i == 1) {
        return error_value(E_FGETFILE_2);
    }
    if (i == 3) {
        return error_value(E_FGETFILE_3);
    }
    result.v_type = V_NULL;
    if (i == 0) {
        result.v_type = V_STR;
        result.v_str = str;
    }
    return result;
}

static VALUE
f_files(int count, VALUE **vals)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (count == 0) {
        result.v_type = V_NUM;
        result.v_num = itoq((long)MAXFILES);
        return result;
    }
    if ((vals[0]->v_type != V_NUM) || qisfrac(vals[0]->v_num)) {
        return error_value(E_FILES);
    }
    result.v_type = V_NULL;
    result.v_file = indexid(qtoi(vals[0]->v_num));
    if (result.v_file != FILEID_NONE) {
        result.v_type = V_FILE;
    }
    return result;
}

static VALUE
f_reverse(VALUE *val)
{
    VALUE res;

    res.v_type = val->v_type;
    res.v_subtype = val->v_subtype;
    switch (val->v_type) {
    case V_MAT:
        res.v_mat = matcopy(val->v_mat);
        matreverse(res.v_mat);
        break;
    case V_LIST:
        res.v_list = listcopy(val->v_list);
        listreverse(res.v_list);
        break;
    case V_STR:
        res.v_str = stringneg(val->v_str);
        if (res.v_str == NULL) {
            return error_value(E_STRNEG);
        }
        break;
    default:
        math_error("Bad argument type for reverse");
        not_reached();
    }
    return res;
}

static VALUE
f_sort(VALUE *val)
{
    VALUE res;

    res.v_type = val->v_type;
    res.v_subtype = val->v_subtype;
    switch (val->v_type) {
    case V_MAT:
        res.v_mat = matcopy(val->v_mat);
        matsort(res.v_mat);
        break;
    case V_LIST:
        res.v_list = listcopy(val->v_list);
        listsort(res.v_list);
        break;
    default:
        math_error("Bad argument type for sort");
        not_reached();
    }
    return res;
}

static VALUE
f_join(int count, VALUE **vals)
{
    LIST *lp;
    LISTELEM *ep;
    VALUE res;

    /* initialize VALUE */
    res.v_subtype = V_NOSUBTYPE;

    lp = listalloc();
    while (count-- > 0) {
        if (vals[0]->v_type != V_LIST) {
            listfree(lp);
            printf("Non-list argument for join\n");
            res.v_type = V_NULL;
            return res;
        }
        for (ep = vals[0]->v_list->l_first; ep; ep = ep->e_next) {
            insertlistlast(lp, &ep->e_value);
        }
        vals++;
    }
    res.v_type = V_LIST;
    res.v_list = lp;
    return res;
}

static VALUE
f_head(VALUE *v1, VALUE *v2)
{
    VALUE res;
    long n;

    /* initialize VALUE */
    res.v_subtype = V_NOSUBTYPE;

    if (v2->v_type != V_NUM || qisfrac(v2->v_num) || zge31b(v2->v_num->num)) {
        return error_value(E_HEAD_2);
    }
    n = qtoi(v2->v_num);

    res.v_type = v1->v_type;
    switch (v1->v_type) {
    case V_LIST:
        if (n == 0) {
            res.v_list = listalloc();
        } else if (n > 0) {
            res.v_list = listsegment(v1->v_list, 0, n - 1);
        } else {
            res.v_list = listsegment(v1->v_list, -n - 1, 0);
        }
        return res;
    case V_STR:
        if (n == 0) {
            res.v_str = slink(&_nullstring_);
        } else if (n > 0) {
            res.v_str = stringsegment(v1->v_str, 0, n - 1);
        } else {
            res.v_str = stringsegment(v1->v_str, -n - 1, 0);
        }
        if (res.v_str == NULL) {
            return error_value(E_STRHEAD);
        }
        return res;
    default:
        return error_value(E_HEAD_1);
    }
}

static VALUE
f_tail(VALUE *v1, VALUE *v2)
{
    long n;
    VALUE res;

    /* initialize VALUE */
    res.v_subtype = V_NOSUBTYPE;

    if (v2->v_type != V_NUM || qisfrac(v2->v_num) || zge31b(v2->v_num->num)) {
        return error_value(E_TAIL_1);
    }
    n = qtoi(v2->v_num);
    res.v_type = v1->v_type;
    switch (v1->v_type) {
    case V_LIST:
        if (n == 0) {
            res.v_list = listalloc();
        } else if (n > 0) {
            res.v_list = listsegment(v1->v_list, v1->v_list->l_count - n, v1->v_list->l_count - 1);
        } else {
            res.v_list = listsegment(v1->v_list, v1->v_list->l_count - 1, v1->v_list->l_count + n);
        }
        return res;
    case V_STR:
        if (n == 0) {
            res.v_str = slink(&_nullstring_);
        } else if (n > 0) {
            res.v_str = stringsegment(v1->v_str, v1->v_str->s_len - n, v1->v_str->s_len - 1);
        } else {
            res.v_str = stringsegment(v1->v_str, v1->v_str->s_len - 1, v1->v_str->s_len + n);
        }
        if (res.v_str == V_NULL) {
            return error_value(E_STRTAIL);
        }
        return res;
    default:
        return error_value(E_TAIL_1);
    }
}

static VALUE
f_segment(int count, VALUE **vals)
{
    VALUE *vp;
    long n1, n2;
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    vp = vals[1];

    if (vp->v_type != V_NUM || qisfrac(vp->v_num) || zge31b(vp->v_num->num)) {
        return error_value(E_SEG_2);
    }
    n1 = qtoi(vp->v_num);
    n2 = n1;
    if (count == 3) {
        vp = vals[2];
        if (vp->v_type != V_NUM || qisfrac(vp->v_num) || zge31b(vp->v_num->num)) {
            return error_value(E_SEG_3);
        }
        n2 = qtoi(vp->v_num);
    }
    vp = vals[0];
    result.v_type = vp->v_type;
    switch (vp->v_type) {
    case V_LIST:
        result.v_list = listsegment(vp->v_list, n1, n2);
        return result;
    case V_STR:
        result.v_str = stringsegment(vp->v_str, n1, n2);
        if (result.v_str == NULL) {
            return error_value(E_STRSEG);
        }
        return result;
    default:
        return error_value(E_SEG_1);
    }
}

static VALUE
f_modify(VALUE *v1, VALUE *v2)
{
    FUNC *fp;
    LISTELEM *ep;
    long s;
    VALUE res;
    VALUE *vp;
    unsigned short subtype;

    if (v1->v_type != V_ADDR) {
        return error_value(E_MODIFY_1);
    }
    v1 = v1->v_addr;
    if (v2->v_type == V_ADDR) {
        v2 = v2->v_addr;
    }
    if (v2->v_type != V_STR) {
        return error_value(E_MODIFY_2);
    }
    if (v1->v_subtype & V_NONEWVALUE) {
        return error_value(E_MODIFY_3);
    }
    fp = findfunc(adduserfunc(v2->v_str->s_str));
    if (!fp) {
        return error_value(E_MODIFY_4);
    }
    switch (v1->v_type) {
    case V_LIST:
        for (ep = v1->v_list->l_first; ep; ep = ep->e_next) {
            subtype = ep->e_value.v_subtype;
            *++stack = ep->e_value;
            calculate(fp, 1);
            stack->v_subtype |= subtype;
            ep->e_value = *stack--;
        }
        break;
    case V_MAT:
        vp = v1->v_mat->m_table;
        s = v1->v_mat->m_size;
        while (s-- > 0) {
            subtype = vp->v_subtype;
            *++stack = *vp;
            calculate(fp, 1);
            stack->v_subtype |= subtype;
            *vp++ = *stack--;
        }
        break;
    case V_OBJ:
        vp = v1->v_obj->o_table;
        s = v1->v_obj->o_actions->oa_count;
        while (s-- > 0) {
            subtype = vp->v_subtype;
            *++stack = *vp;
            calculate(fp, 1);
            stack->v_subtype |= subtype;
            *vp++ = *stack--;
        }
        break;
    default:
        return error_value(E_MODIFY_5);
    }
    res.v_type = V_NULL;
    res.v_subtype = V_NOSUBTYPE;
    return res;
}

static VALUE
f_forall(VALUE *v1, VALUE *v2)
{
    FUNC *fp;
    LISTELEM *ep;
    long s;
    VALUE res;
    VALUE *vp;

    /* initialize VALUE */
    res.v_type = V_NULL;
    res.v_subtype = V_NOSUBTYPE;

    if (v2->v_type != V_STR) {
        math_error("Non-string second argument for forall");
        not_reached();
    }
    fp = findfunc(adduserfunc(v2->v_str->s_str));
    if (!fp) {
        math_error("Undefined function for forall");
        not_reached();
    }
    switch (v1->v_type) {
    case V_LIST:
        for (ep = v1->v_list->l_first; ep; ep = ep->e_next) {
            copyvalue(&ep->e_value, ++stack);
            calculate(fp, 1);
            stack--;
        }
        break;
    case V_MAT:
        vp = v1->v_mat->m_table;
        s = v1->v_mat->m_size;
        while (s-- > 0) {
            copyvalue(vp++, ++stack);
            calculate(fp, 1);
            stack--;
        }
        break;
    default:
        math_error("Non list or matrix first argument for forall");
        not_reached();
    }
    return res;
}

static VALUE
f_select(VALUE *v1, VALUE *v2)
{
    LIST *lp;
    LISTELEM *ep;
    FUNC *fp;
    VALUE res;

    /* initialize VALUE */
    res.v_type = V_LIST;
    res.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_LIST) {
        math_error("Non-list first argument for select");
        not_reached();
    }
    if (v2->v_type != V_STR) {
        math_error("Non-string second argument for select");
        not_reached();
    }
    fp = findfunc(adduserfunc(v2->v_str->s_str));
    if (!fp) {
        math_error("Undefined function for select");
        not_reached();
    }
    lp = listalloc();
    for (ep = v1->v_list->l_first; ep; ep = ep->e_next) {
        copyvalue(&ep->e_value, ++stack);
        calculate(fp, 1);
        if (testvalue(stack)) {
            insertlistlast(lp, &ep->e_value);
        }
        freevalue(stack--);
    }
    res.v_list = lp;
    return res;
}

static VALUE
f_count(VALUE *v1, VALUE *v2)
{
    LISTELEM *ep;
    FUNC *fp;
    long s;
    long n = 0;
    VALUE res;
    VALUE *vp;

    /* initialize VALUE */
    res.v_type = V_NUM;
    res.v_subtype = V_NOSUBTYPE;

    if (v2->v_type != V_STR) {
        math_error("Non-string second argument for select");
        not_reached();
    }
    fp = findfunc(adduserfunc(v2->v_str->s_str));
    if (!fp) {
        math_error("Undefined function for select");
        not_reached();
    }
    switch (v1->v_type) {
    case V_LIST:
        for (ep = v1->v_list->l_first; ep; ep = ep->e_next) {
            copyvalue(&ep->e_value, ++stack);
            calculate(fp, 1);
            if (testvalue(stack)) {
                n++;
            }
            freevalue(stack--);
        }
        break;
    case V_MAT:
        s = v1->v_mat->m_size;
        vp = v1->v_mat->m_table;
        while (s-- > 0) {
            copyvalue(vp++, ++stack);
            calculate(fp, 1);
            if (testvalue(stack)) {
                n++;
            }
            freevalue(stack--);
        }
        break;
    default:
        math_error("Bad argument type for count");
        not_reached();
        break;
    }
    res.v_num = itoq(n);
    return res;
}

static VALUE
f_makelist(VALUE *v1)
{
    LIST *lp;
    VALUE res;
    long n;

    /* initialize VALUE */
    res.v_type = V_NULL;
    res.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_NUM || qisfrac(v1->v_num) || qisneg(v1->v_num)) {
        math_error("Bad argument for makelist");
        not_reached();
    }
    if (zge31b(v1->v_num->num)) {
        math_error("makelist count >= 2^31");
        not_reached();
    }
    n = qtoi(v1->v_num);
    lp = listalloc();
    while (n-- > 0) {
        insertlistlast(lp, &res);
    }
    res.v_type = V_LIST;
    res.v_list = lp;
    return res;
}

static VALUE
f_randperm(VALUE *val)
{
    VALUE res;

    /* initialize VALUE */
    res.v_subtype = V_NOSUBTYPE;

    res.v_type = val->v_type;
    switch (val->v_type) {
    case V_MAT:
        res.v_mat = matcopy(val->v_mat);
        matrandperm(res.v_mat);
        break;
    case V_LIST:
        res.v_list = listcopy(val->v_list);
        listrandperm(res.v_list);
        break;
    default:
        math_error("Bad argument type for randperm");
        not_reached();
    }
    return res;
}

static VALUE
f_cmdbuf(void)
{
    VALUE result;
    char *newcp;
    size_t cmdbuf_len; /* length of cmdbuf string */

    /* initialize VALUE */
    result.v_type = V_STR;
    result.v_subtype = V_NOSUBTYPE;

    cmdbuf_len = strlen(cmdbuf);
    newcp = (char *)calloc(cmdbuf_len + 1, 1);
    if (newcp == NULL) {
        math_error("Cannot allocate string in cmdbuf");
        not_reached();
    }
    strlcpy(newcp, cmdbuf, cmdbuf_len + 1);
    result.v_str = makestring(newcp);
    return result;
}

static VALUE
f_getenv(VALUE *v1)
{
    VALUE result;
    char *str;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (v1->v_type != V_STR) {
        math_error("Non-string argument for getenv");
        not_reached();
    }
    result.v_type = V_STR;
    str = getenv(v1->v_str->s_str);
    if (str == NULL) {
        result.v_type = V_NULL;
    } else {
        result.v_str = makenewstring(str);
    }
    return result;
}

static VALUE
f_isatty(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    result.v_type = V_NUM;
    if (vp->v_type == V_FILE && isattyid(vp->v_file) == 1) {
        result.v_num = itoq(1);
    } else {
        result.v_num = itoq(0);
    }
    return result;
}

static VALUE
f_calc_tty(void)
{
    VALUE res;

    if (!calc_tty(FILEID_STDIN)) {
        return error_value(E_TTY);
    }
    res.v_type = V_NULL;
    res.v_subtype = V_NOSUBTYPE;
    return res;
}

static VALUE
f_inputlevel(void)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    result.v_num = itoq((long)inputlevel());
    return result;
}

static VALUE
f_calclevel(void)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    result.v_num = itoq(calclevel());
    return result;
}

static VALUE
f_calcpath(void)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_STR;
    result.v_subtype = V_NOSUBTYPE;

    result.v_str = makenewstring(calcpath);
    return result;
}

static VALUE
f_access(int count, VALUE **vals)
{
    NUMBER *q;
    int m;
    char *s, *fname;
    VALUE result;
    size_t len;
    int i;

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    errno = 0;
    if (vals[0]->v_type != V_STR) {
        return error_value(E_ACCESS_1);
    }
    fname = vals[0]->v_str->s_str;
    m = 0;
    if (count == 2) {
        switch (vals[1]->v_type) {
        case V_NUM:
            q = vals[1]->v_num;
            if (qisfrac(q) || qisneg(q)) {
                return error_value(E_ACCESS_2);
            }
            m = (int)(q->num.v[0] & 7);
            break;
        case V_STR:
            s = vals[1]->v_str->s_str;
            len = (long)strlen(s);
            while (len-- > 0) {
                switch (*s++) {
                case 'r':
                    m |= 4;
                    break;
                case 'w':
                    m |= 2;
                    break;
                case 'x':
                    m |= 1;
                    break;
                default:
                    return error_value(E_ACCESS_2);
                }
            }
            break;
        case V_NULL:
            break;
        default:
            return error_value(E_ACCESS_2);
        }
    }
    i = access(fname, m);
    if (i) {
        return error_value(errno);
    }
    return result;
}

static VALUE
f_putenv(int count, VALUE **vals)
{
    VALUE result;
    char *putenv_str;

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * parse args
     */
    if (count == 2) {
        size_t snprintf_len; /* malloced snprintf buffer length */

        /* firewall */
        if (vals[0]->v_type != V_STR || vals[1]->v_type != V_STR) {
            math_error("Non-string argument for putenv");
            not_reached();
        }

        /* convert putenv("foo","bar") into putenv("foo=bar") */
        snprintf_len = vals[0]->v_str->s_len + 1 + vals[1]->v_str->s_len;
        putenv_str = (char *)calloc(snprintf_len + 1, 1);
        if (putenv_str == NULL) {
            math_error("Cannot allocate string in putenv");
            not_reached();
        }
        /*
         * The next statement could be:
         *
         *      snprintf(putenv_str, snprintf_len,
         *              "%s=%s", vals[0]->v_str->s_str,
         *              vals[1]->v_str->s_str);
         *
         * however compilers like gcc would issue warnings such as:
         *
         *      null destination pointer
         *
         * even though we check that putenv_str is non-NULL
         * above before using it.  Therefore we call strlcpy()
         * twice and make an assignment instead to avoid such warnings.
         */
        strlcpy(putenv_str, vals[0]->v_str->s_str, vals[0]->v_str->s_len + 1);
        putenv_str[vals[0]->v_str->s_len] = '=';
        strlcpy(putenv_str + vals[0]->v_str->s_len + 1, vals[1]->v_str->s_str, vals[1]->v_str->s_len + 1);
        putenv_str[snprintf_len] = '\0';

    } else {
        /* firewall */
        if (vals[0]->v_type != V_STR) {
            math_error("Non-string argument for putenv");
            not_reached();
        }

        /* putenv(arg) must be of the form "foo=bar" */
        if ((char *)strchr(vals[0]->v_str->s_str, '=') == NULL) {
            math_error("putenv single arg string missing =");
            not_reached();
        }

        /*
         * make a copy of the arg because subsequent changes
         * would change the environment.
         */
        putenv_str = (char *)calloc(vals[0]->v_str->s_len + 1, 1);
        if (putenv_str == NULL) {
            math_error("Cannot allocate string in putenv");
            not_reached();
        }
        strlcpy(putenv_str, vals[0]->v_str->s_str, vals[0]->v_str->s_len + 1);
    }

    /* return putenv result */
    result.v_num = itoq((long)malloced_putenv(putenv_str));
    return result;
}

static VALUE
f_strpos(VALUE *haystack, VALUE *needle)
{
    VALUE result;
    char *cpointer;
    int cindex;

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    if (haystack->v_type != V_STR || needle->v_type != V_STR) {
        math_error("Non-string argument for index");
        not_reached();
    }
    cpointer = strstr(haystack->v_str->s_str, needle->v_str->s_str);
    if (cpointer == NULL) {
        cindex = 0;
    } else {
        cindex = cpointer - haystack->v_str->s_str + 1;
    }
    result.v_num = itoq((long)cindex);
    return result;
}

static VALUE
f_system(VALUE *vp)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;

    if (vp->v_type != V_STR) {
        math_error("Non-string argument for system");
        not_reached();
    }
    if (!allow_exec) {
        math_error("execution disallowed by -m");
        not_reached();
    }
    if (conf->calc_debug & CALCDBG_SYSTEM) {
        printf("%s\n", vp->v_str->s_str);
    }
#  if defined(_WIN32) || defined(_WIN64)

    /* if the execute length is 0 then just return 0 */
    if (vp->v_str->s_len == 0) {
        result.v_num = itoq((long)0);
    } else {
        result.v_num = itoq((long)system(vp->v_str->s_str));
    }

#  else

    result.v_num = itoq((long)system(vp->v_str->s_str));

#  endif
    return result;
}

static VALUE
f_sleep(int count, VALUE **vals)
{
    long time;
    VALUE res;
    NUMBER *q1, *q2;

    res.v_type = V_NULL;
    res.v_subtype = V_NOSUBTYPE;
#  if !defined(_WIN32) && !defined(_WIN64)

    if (count > 0) {
        if (vals[0]->v_type != V_NUM || qisneg(vals[0]->v_num)) {
            return error_value(E_SLEEP);
        }
        if (qisint(vals[0]->v_num)) {
            if (zge31b(vals[0]->v_num->num)) {
                return error_value(E_SLEEP);
            }
            time = ztoi(vals[0]->v_num->num);
            time = sleep(time);
        } else {
            q1 = qscale(vals[0]->v_num, 20);
            q2 = qint(q1);
            qfree(q1);
            if (zge31b(q2->num)) {
                qfree(q2);
                return error_value(E_SLEEP);
            }
            time = ztoi(q2->num);
            qfree(q2);
            /* BSD 4.3 usleep has void return */
            usleep(time);
            return res;
        }
    } else {
        time = sleep(1);
    }
    if (time) {
        res.v_type = V_NUM;
        res.v_num = itoq(time);
    }

#  endif
    return res;
}

/*
 * set the default output base/mode
 */
static NUMBER *
f_base(int count, NUMBER **vals)
{
    long base;        /* output base/mode */
    long oldbase = 0; /* output base/mode */

    /* deal with just a query */
    if (count != 1) {
        return base_value(conf->outmode, conf->outmode);
    }

    /* deal with the special modes first */
    if (qisfrac(vals[0])) {
        return base_value(math_setmode(MODE_FRAC), conf->outmode);
    }
    if (vals[0]->num.len > 64 / BASEB) {
        return base_value(math_setmode(MODE_EXP), conf->outmode);
    }

    /* set the base, if possible */
    base = qtoi(vals[0]);
    switch (base) {
    case -10:
        oldbase = math_setmode(MODE_INT);
        break;
    case 2:
        oldbase = math_setmode(MODE_BINARY);
        break;
    case 8:
        oldbase = math_setmode(MODE_OCTAL);
        break;
    case 10:
        oldbase = math_setmode(MODE_REAL);
        break;
    case 16:
        oldbase = math_setmode(MODE_HEX);
        break;
    case 1000:
        oldbase = math_setmode(MODE_ENG);
        break;
    default:
        math_error("Unsupported base");
        not_reached();
        break;
    }

    /* return the old base */
    return base_value(oldbase, conf->outmode);
}

/*
 * set the default secondary output base/mode
 */
static NUMBER *
f_base2(int count, NUMBER **vals)
{
    long base;        /* output base/mode */
    long oldbase = 0; /* output base/mode */

    /* deal with just a query */
    if (count != 1) {
        return base_value(conf->outmode2, conf->outmode2);
    }

    /* deal with the special modes first */
    if (qisfrac(vals[0])) {
        return base_value(math_setmode2(MODE_FRAC), conf->outmode2);
    }
    if (vals[0]->num.len > 64 / BASEB) {
        return base_value(math_setmode2(MODE_EXP), conf->outmode2);
    }

    /* set the base, if possible */
    base = qtoi(vals[0]);
    switch (base) {
    case 0:
        oldbase = math_setmode2(MODE2_OFF);
        break;
    case -10:
        oldbase = math_setmode2(MODE_INT);
        break;
    case 2:
        oldbase = math_setmode2(MODE_BINARY);
        break;
    case 8:
        oldbase = math_setmode2(MODE_OCTAL);
        break;
    case 10:
        oldbase = math_setmode2(MODE_REAL);
        break;
    case 16:
        oldbase = math_setmode2(MODE_HEX);
        break;
    case 1000:
        oldbase = math_setmode2(MODE_ENG);
        break;
    default:
        math_error("Unsupported base");
        not_reached();
        break;
    }

    /* return the old base */
    return base_value(oldbase, conf->outmode2);
}

/*
 * return a numerical 'value' of the mode/base
 */
static NUMBER *
base_value(long mode, int defval)
{
    NUMBER *result;

    /* return the old base */
    switch (mode) {
    case MODE_DEFAULT:
        switch (defval) {
        case MODE_DEFAULT:
            result = itoq(10);
            break;
        case MODE_FRAC:
            result = qalloc();
            itoz(3, &result->den);
            break;
        case MODE_INT:
            result = itoq(-10);
            break;
        case MODE_REAL:
            result = itoq(10);
            break;
        case MODE_EXP:
            result = qalloc();
            ztenpow(20, &result->num);
            break;
        case MODE_ENG:
            result = itoq(1000);
            break;
        case MODE_HEX:
            result = itoq(16);
            break;
        case MODE_OCTAL:
            result = itoq(8);
            break;
        case MODE_BINARY:
            result = itoq(2);
            break;
        case MODE2_OFF:
            result = itoq(0);
            break;
        default:
            result = itoq(0);
            break;
        }
        break;
    case MODE_FRAC:
        result = qalloc();
        itoz(3, &result->den);
        break;
    case MODE_INT:
        result = itoq(-10);
        break;
    case MODE_REAL:
        result = itoq(10);
        break;
    case MODE_EXP:
        result = qalloc();
        ztenpow(20, &result->num);
        break;
    case MODE_ENG:
        result = itoq(1000);
        break;
    case MODE_HEX:
        result = itoq(16);
        break;
    case MODE_OCTAL:
        result = itoq(8);
        break;
    case MODE_BINARY:
        result = itoq(2);
        break;
    case MODE2_OFF:
        result = itoq(0);
        break;
    default:
        result = itoq(0);
        break;
    }
    return result;
}

static VALUE
f_custom(int count, VALUE **vals)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * disable custom functions unless -C was given
     */
    if (!allow_custom) {
        fprintf(stderr,
#  if defined(CUSTOM)

                "%sCalc must be run with a -C argument to "
                "use custom function\n",
#  else
                "%sCalc was built with custom functions disabled\n",

#  endif
                (conf->tab_ok ? "\t" : ""));
        return error_value(E_CUSTOM_ERROR);
    }

    /*
     * perform the custom operation
     */
    if (count <= 0) {
        /* perform the usage function function */
        showcustom();
    } else {
        /* firewall */
        if (vals[0]->v_type != V_STR) {
            math_error("custom: 1st arg not a string name");
            not_reached();
        }

        /* perform the custom function */
        result = custom(vals[0]->v_str->s_str, count - 1, vals + 1);
    }

    /*
     * return the custom result
     */
    return result;
}

static VALUE
f_blk(int count, VALUE **vals)
{
    int len;   /* number of octets to malloc */
    int chunk; /* block chunk size */
    VALUE result;
    int id;
    VALUE *vp = NULL;
    int type;

    /* initialize VALUE */
    result.v_type = V_BLOCK;
    result.v_subtype = V_NOSUBTYPE;

    type = V_NULL;
    if (count > 0) {
        vp = *vals;
        type = vp->v_type;
        if (type == V_STR || type == V_NBLOCK || type == V_BLOCK) {
            vals++;
            count--;
        }
    }

    len = -1;   /* signal to use old or zero len */
    chunk = -1; /* signal to use old or default chunksize */
    if (count > 0 && vals[0]->v_type != V_NULL) {
        /* parse len */
        if (vals[0]->v_type != V_NUM || qisfrac(vals[0]->v_num)) {
            return error_value(E_BLK_1);
        }
        if (qisneg(vals[0]->v_num) || zge31b(vals[0]->v_num->num)) {
            return error_value(E_BLK_2);
        }
        len = qtoi(vals[0]->v_num);
    }
    if (count > 1 && vals[1]->v_type != V_NULL) {
        /* parse chunk */
        if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num)) {
            return error_value(E_BLK_3);
        }
        if (qisneg(vals[1]->v_num) || zge31b(vals[1]->v_num->num)) {
            return error_value(E_BLK_4);
        }
        chunk = qtoi(vals[1]->v_num);
    }

    if (type == V_STR) {
        result.v_type = V_NBLOCK;
        id = findnblockid(vp->v_str->s_str);
        if (id < 0) {
            /* create new named block */
            result.v_nblock = createnblock(vp->v_str->s_str, len, chunk);
            return result;
        }
        /* reallocate nblock */
        result.v_nblock = reallocnblock(id, len, chunk);
        return result;
    }

    if (type == V_NBLOCK) {
        /* reallocate nblock */
        result.v_type = V_NBLOCK;
        id = vp->v_nblock->id;
        result.v_nblock = reallocnblock(id, len, chunk);
        return result;
    }
    if (type == V_BLOCK) {
        /* reallocate block */
        result.v_type = V_BLOCK;
        result.v_block = copyrealloc(vp->v_block, len, chunk);
        return result;
    }

    /* allocate block */
    result.v_block = blkalloc(len, chunk);
    return result;
}

static VALUE
f_blkfree(VALUE *vp)
{
    int id;
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    id = 0;
    switch (vp->v_type) {
    case V_NBLOCK:
        id = vp->v_nblock->id;
        break;
    case V_STR:
        id = findnblockid(vp->v_str->s_str);
        if (id < 0) {
            return error_value(E_BLKFREE_1);
        }
        break;
    case V_NUM:
        if (qisfrac(vp->v_num) || qisneg(vp->v_num)) {
            return error_value(E_BLKFREE_2);
        }
        if (zge31b(vp->v_num->num)) {
            return error_value(E_BLKFREE_3);
        }
        id = qtoi(vp->v_num);
        break;
    default:
        return error_value(E_BLKFREE_4);
    }
    id = removenblock(id);
    if (id) {
        return error_value(id);
    }
    return result;
}

static VALUE
f_blocks(int count, VALUE **vals)
{
    NBLOCK *nblk;
    VALUE result;
    int id;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    if (count == 0) {
        result.v_type = V_NUM;
        result.v_num = itoq((long)countnblocks());
        return result;
    }
    if (vals[0]->v_type != V_NUM || qisfrac(vals[0]->v_num)) {
        return error_value(E_BLOCKS_1);
    }
    id = (int)qtoi(vals[0]->v_num);

    nblk = findnblock(id);

    if (nblk == NULL) {
        return error_value(E_BLOCKS_2);
    } else {
        result.v_type = V_NBLOCK;
        result.v_nblock = nblk;
    }
    return result;
}

static VALUE
f_free(int count, VALUE **vals)
{
    VALUE result;
    VALUE *val;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    result.v_type = V_NULL;
    while (count-- > 0) {
        val = *vals++;
        if (val->v_type == V_ADDR) {
            freevalue(val->v_addr);
        }
    }
    return result;
}

static VALUE
f_freeglobals(void)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    freeglobals();
    return result;
}

static VALUE
f_freeredc(void)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    freeredcdata();
    return result;
}

static VALUE
f_freestatics(void)
{
    VALUE result;

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    freestatics();
    return result;
}

/*
 * f_copy - copy consecutive items between values
 *
 *      copy(src, dst [, ssi [, num [, dsi]]])
 *
 * Copy 'num' consecutive items from 'src' with index 'ssi' to
 * 'dest', starting at position with index 'dsi'.
 */
static VALUE
f_copy(int count, VALUE **vals)
{
    long ssi = 0;  /* source start index */
    long num = -1; /* number of items to copy (-1 ==> all) */
    long dsi = -1; /* destination start index, -1 ==> default */
    int errtype;   /* error type if unable to perform copy */
    VALUE result;  /* null if successful */

    /* initialize VALUE */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * parse args
     */
    switch (count) {
    case 5:
        /* parse dsi */
        if (vals[4]->v_type != V_NULL) {
            if (vals[4]->v_type != V_NUM || qisfrac(vals[4]->v_num) || qisneg(vals[4]->v_num)) {
                return error_value(E_COPY_06);
            }
            if (zge31b(vals[4]->v_num->num)) {
                return error_value(E_COPY_07);
            }
            dsi = qtoi(vals[4]->v_num);
        }
        /*FALLTHRU*/

    case 4:
        /* parse num */
        if (vals[3]->v_type != V_NULL) {
            if (vals[3]->v_type != V_NUM || qisfrac(vals[3]->v_num) || qisneg(vals[3]->v_num)) {
                return error_value(E_COPY_01);
            }
            if (zge31b(vals[3]->v_num->num)) {
                return error_value(E_COPY_02);
            }
            num = qtoi(vals[3]->v_num);
        }
        /*FALLTHRU*/

    case 3:
        /* parse ssi */
        if (vals[2]->v_type != V_NULL) {
            if (vals[2]->v_type != V_NUM || qisfrac(vals[2]->v_num) || qisneg(vals[2]->v_num)) {
                return error_value(E_COPY_04);
            }
            if (zge31b(vals[2]->v_num->num)) {
                return error_value(E_COPY_05);
            }
            ssi = qtoi(vals[2]->v_num);
        }
        break;
    }

    /*
     * copy
     */
    errtype = copystod(vals[0], ssi, num, vals[1], dsi);
    if (errtype > 0) {
        return error_value(errtype);
    }
    return result;
}

/*
 * f_blkcpy - copy consecutive items between values
 *
 *      copy(dst, src [, num [, dsi [, ssi]]])
 *
 * Copy 'num' consecutive items from 'src' with index 'ssi' to
 * 'dest', starting at position with index 'dsi'.
 */
static VALUE
f_blkcpy(int count, VALUE **vals)
{
    VALUE *args[5];   /* args to re-order */
    VALUE null_value; /* dummy argument */

    /* initialize VALUE */
    null_value.v_subtype = V_NOSUBTYPE;

    /*
     * parse args into f_copy order
     */
    args[0] = vals[1];
    args[1] = vals[0];
    null_value.v_type = V_NULL;
    args[2] = &null_value;
    args[3] = &null_value;
    args[4] = &null_value;
    switch (count) {
    case 5:
        args[2] = vals[4];
        args[4] = vals[3];
        args[3] = vals[2];
        break;
    case 4:
        count = 5;
        args[4] = vals[3];
        args[3] = vals[2];
        break;
    case 3:
        count = 4;
        args[3] = vals[2];
        break;
    }

    /*
     * copy
     */
    return f_copy(count, args);
}

static VALUE
f_sha1(int count, VALUE **vals)
{
    VALUE result;
    HASH *state; /* pointer to hash state to use */
    int i;       /* vals[i] to hash */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * arg check
     */
    if (count == 0) {

        /* return an initial hash state */
        result.v_type = V_HASH;
        result.v_hash = hash_init(SHA1_HASH_TYPE, NULL);

    } else if (count == 1 && vals[0]->v_type == V_HASH && vals[0]->v_hash->hashtype == SHA1_HASH_TYPE) {

        /* if just a hash value, finalize it */
        state = hash_copy(vals[0]->v_hash);
        result.v_type = V_NUM;
        result.v_num = qalloc();
        result.v_num->num = hash_final(state);
        hash_free(state);

    } else {

        /*
         * If the first value is a hash, use that as
         * the initial hash state
         */
        if (vals[0]->v_type == V_HASH && vals[0]->v_hash->hashtype == SHA1_HASH_TYPE) {
            state = hash_copy(vals[0]->v_hash);
            i = 1;

            /*
             * otherwise use the default initial state
             */
        } else {
            state = hash_init(SHA1_HASH_TYPE, NULL);
            i = 0;
        }

        /*
         * hash the remaining values
         */
        do {
            state = hash_value(SHA1_HASH_TYPE, vals[i], state);
        } while (++i < count);

        /*
         * return the current hash state
         */
        result.v_type = V_HASH;
        result.v_hash = state;
    }

    /* return the result */
    return result;
}

static VALUE
f_argv(int count, VALUE **vals)
{
    int arg; /* the argv_value string index */
    VALUE result;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * arg check
     */
    if (count == 0) {

        /* return the argc count */
        result.v_type = V_NUM;
        result.v_num = itoq((long)argc_value);

    } else {

        /* firewall */
        if (vals[0]->v_type != V_NUM || qisfrac(vals[0]->v_num) || qisneg(vals[0]->v_num) || zge31b(vals[0]->v_num->num)) {

            math_error("argv argument must be a integer [0,2^31)");
            not_reached();
        }

        /* determine the arg value of the argv() function */
        arg = qtoi(vals[0]->v_num);

        /* argv(0) is program or script_name if -f filename was used */
        if (arg == 0) {
            if (script_name == NULL) {
                /* paranoia */
                result.v_type = V_NULL;
            } else {
                result.v_type = V_STR;
                result.v_str = makenewstring(script_name);
            }

            /* return the n-th argv string */
        } else if (arg < argc_value && argv_value[arg - 1] != NULL) {
            result.v_type = V_STR;
            result.v_str = makestring(strdup(argv_value[arg - 1]));
        } else {
            result.v_type = V_NULL;
        }
    }

    /* return the result */
    return result;
}

static VALUE
f_version(void)
{
    VALUE result;

    /* return the calc version string */
    result.v_type = V_STR;
    result.v_subtype = V_NOSUBTYPE;
    result.v_str = makestring(strdup(version()));

    return result;
}

/*
 * f_versin - versed trigonometric sine
 */
static VALUE
f_versin(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_VERSIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute versed trigonometric sine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qversin(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_versin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_VERSIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_VERSIN_2);
    }
    return result;
}

/*
 * f_aversin - inverse versed trigonometric sine
 */
static VALUE
f_aversin(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_AVERSIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse versed trigonometric sine to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qaversin_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_aversin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_AVERSIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_AVERSIN_2);
    }
    return result;
}

/*
 * f_coversin - coversed trigonometric sine
 */
static VALUE
f_coversin(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_COVERSIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute coversed trigonometric sine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qcoversin(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_coversin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_COVERSIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_COVERSIN_2);
    }
    return result;
}

/*
 * f_acoversin - inverse coversed trigonometric sine
 */
static VALUE
f_acoversin(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACOVERSIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse coversed trigonometric sine to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qacoversin_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_acoversin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_ACOVERSIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_ACOVERSIN_2);
    }
    return result;
}

/*
 * f_vercos - versed trigonometric cosine
 */
static VALUE
f_vercos(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_VERCOS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute versed trigonometric cosine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qvercos(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_vercos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_VERCOS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_VERCOS_2);
    }
    return result;
}

/*
 * f_avercos - inverse versed trigonometric cosine
 */
static VALUE
f_avercos(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_AVERCOS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse versed trigonometric cosine to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qavercos_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_avercos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_AVERCOS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_AVERCOS_2);
    }
    return result;
}

/*
 * f_covercos - coversed trigonometric cosine
 */
static VALUE
f_covercos(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_COVERCOS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute coversed trigonometric cosine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qcovercos(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_covercos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_COVERCOS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_COVERCOS_2);
    }
    return result;
}

/*
 * f_acovercos - inverse coversed trigonometric cosine
 */
static VALUE
f_acovercos(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACOVERCOS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse coversed trigonometric cosine to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qacovercos_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_acovercos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_ACOVERCOS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_ACOVERCOS_2);
    }
    return result;
}

/*
 * f_haversin - half versed trigonometric sine
 */
static VALUE
f_haversin(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_HAVERSIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute half versed trigonometric sine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qhaversin(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_haversin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_HAVERSIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_HAVERSIN_2);
    }
    return result;
}

/*
 * f_ahaversin - inverse half versed trigonometric sine
 */
static VALUE
f_ahaversin(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_AHAVERSIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse half versed trigonometric sine to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qahaversin_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_ahaversin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_AHAVERSIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_AHAVERSIN_2);
    }
    return result;
}

/*
 * f_hacoversin - half coversed trigonometric sine
 */
static VALUE
f_hacoversin(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_HACOVERSIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute half coversed trigonometric sine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qhacoversin(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_hacoversin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_HACOVERSIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_HACOVERSIN_2);
    }
    return result;
}

/*
 * f_ahacoversin - inverse half coversed trigonometric sine
 */
static VALUE
f_ahacoversin(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_AHACOVERSIN_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse half coversed trigonometric sine to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qahacoversin_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_ahacoversin(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_AHACOVERSIN_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_AHACOVERSIN_2);
    }
    return result;
}

/*
 * f_havercos - half versed trigonometric cosine
 */
static VALUE
f_havercos(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_HAVERCOS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute half versed trigonometric cosine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qhavercos(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_havercos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_HAVERCOS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_HAVERCOS_2);
    }
    return result;
}

/*
 * f_ahavercos - inverse half versed trigonometric cosine
 */
static VALUE
f_ahavercos(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_AHAVERCOS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse half versed trigonometric cosine to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qahavercos_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_ahavercos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_AHAVERCOS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_AHAVERCOS_2);
    }
    return result;
}

/*
 * f_hacovercos - half coversed trigonometric cosine
 */
static VALUE
f_hacovercos(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *eps;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_HACOVERCOS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute half coversed trigonometric cosine to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qhacovercos(vals[0]->v_num, eps);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_hacovercos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_HACOVERCOS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_HACOVERCOS_2);
    }
    return result;
}

/*
 * f_ahacovercos - inverse half coversed trigonometric cosine
 */
static VALUE
f_ahacovercos(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_AHACOVERCOS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse half coversed trigonometric cosine to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qahacovercos_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_ahacovercos(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_AHACOVERCOS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_AHACOVERCOS_2);
    }
    return result;
}

/*
 * f_exsec - exterior trigonometric secant
 */
static VALUE
f_exsec(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use err VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_EXSEC_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute exterior trigonometric secant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qexsec(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_exsec(vals[0]->v_com, err);
        if (c == NULL) {
            return error_value(E_EXSEC_3);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_EXSEC_2);
    }
    return result;
}

/*
 * f_aexsec - inverse exterior trigonometric secant
 */
static VALUE
f_aexsec(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_AEXSEC_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse exterior trigonometric secant to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* firewall */
        if (qisnegone(vals[0]->v_num)) {
            return error_value(E_AEXSEC_3);
        }

        /* try to compute result using real trig function */
        result.v_num = qaexsec_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_aexsec(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_AEXSEC_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_AEXSEC_2);
    }
    return result;
}

/*
 * f_excsc - exterior trigonometric cosecant
 */
static VALUE
f_excsc(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use err VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_EXCSC_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute exterior trigonometric cosecant to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        if (qiszero(vals[0]->v_num)) {
            return error_value(E_EXCSC_3);
        }
        result.v_num = qexcsc(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        if (ciszero(vals[0]->v_com)) {
            return error_value(E_EXCSC_3);
        }
        c = c_excsc(vals[0]->v_com, err);
        if (c == NULL) {
            return error_value(E_EXCSC_4);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_EXCSC_2);
    }
    return result;
}

/*
 * f_aexcsc - exterior trigonometric cosecant
 */
static VALUE
f_aexcsc(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_AEXCSC_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse exterior trigonometric cosecant to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* firewall */
        if (qisnegone(vals[0]->v_num)) {
            return error_value(E_AEXCSC_3);
        }

        /* try to compute result using real trig function */
        result.v_num = qaexcsc_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_aexcsc(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_AEXCSC_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_AEXCSC_2);
    }
    return result;
}

/*
 * f_crd - trigonometric chord of a unit circle
 */
static VALUE
f_crd(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use err VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_CRD_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute chord of a unit circle to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qcrd(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_crd(vals[0]->v_com, err);
        if (c == NULL) {
            return error_value(E_CRD_3);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_CRD_2);
    }
    return result;
}

/*
 * f_acrd - inverse trigonometric chord of a unit circle
 */
static VALUE
f_acrd(int count, VALUE **vals)
{
    VALUE result; /* value to return */
    COMPLEX *c;   /* COMPLEX trig result */
    NUMBER *eps;  /* epsilon error tolerance */

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_ACRD_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute inverse trigonometric chord of a unit circle to a given error tolerance
     */
    if (vals[0]->v_type == V_NUM) {

        /* try to compute result using real trig function */
        result.v_num = qacrd_or_NULL(vals[0]->v_num, eps);

        /*
         * case: trig function returned a NUMBER
         */
        if (result.v_num != NULL) {
            result.v_type = V_NUM;

            /*
             * case: trig function returned NULL - need to try COMPLEX trig function
             */
        } else {
            /* convert NUMBER argument from NUMBER to COMPLEX */
            vals[0]->v_com = qqtoc(vals[0]->v_num, &_qzero_);
            vals[0]->v_type = V_COM;
        }
    }
    if (vals[0]->v_type == V_COM) {

        /*
         * case: argument was COMPLEX or
         *       trig function returned NULL and argument was converted to COMPLEX
         */
        c = c_acrd(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_ACRD_3);
        }
        result.v_com = c;
        result.v_type = V_COM;

        /*
         * case: complex trig function returned real, convert result to NUMBER
         */
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
    }
    if (vals[0]->v_type != V_NUM && vals[0]->v_type != V_COM) {

        /*
         * case: argument type is not valid for this function
         */
        return error_value(E_ACRD_2);
    }
    return result;
}

/*
 * f_cas - trigonometric chord of a unit circle
 */
static VALUE
f_cas(int count, VALUE **vals)
{
    VALUE result;
    COMPLEX *c;
    NUMBER *err;

    /* initialize VALUEs */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use err VALUE arg if given and value is in a valid range.
     */
    err = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_CAS_1);
        }
        err = vals[1]->v_num;
    }

    /*
     * compute chord of a unit circle to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        result.v_num = qcas(vals[0]->v_num, err);
        result.v_type = V_NUM;
        break;
    case V_COM:
        c = c_cas(vals[0]->v_com, err);
        if (c == NULL) {
            return error_value(E_CAS_3);
        }
        result.v_com = c;
        result.v_type = V_COM;
        if (cisreal(c)) {
            result.v_num = c_to_q(c, true);
            result.v_type = V_NUM;
        }
        break;
    default:
        return error_value(E_CAS_2);
    }
    return result;
}

/*
 * f_cis - Euler's formula
 */
static VALUE
f_cis(int count, VALUE **vals)
{
    VALUE result;
    NUMBER *eps;
    COMPLEX *c;
    COMPLEX *ctmp;

    /* initialize VALUE */
    result.v_subtype = V_NOSUBTYPE;

    /*
     * set error tolerance for builtin function
     *
     * Use eps VALUE arg if given and value is in a valid range.
     */
    eps = conf->epsilon;
    if (count == 2) {
        if (verify_eps(vals[1]) == false) {
            return error_value(E_CIS_1);
        }
        eps = vals[1]->v_num;
    }

    /*
     * compute Euler's formula to a given error tolerance
     */
    switch (vals[0]->v_type) {
    case V_NUM:
        /*
         * convert arg to COMPLEX
         */
        ctmp = q_to_c(vals[0]->v_num);

        /*
         * compute cis of argument
         */
        c = c_cis(ctmp, eps);
        comfree(ctmp);
        if (c == NULL) {
            return error_value(E_CIS_3);
        }
        break;
    case V_COM:

        /*
         * compute cis of argument
         */
        c = c_cis(vals[0]->v_com, eps);
        if (c == NULL) {
            return error_value(E_CIS_3);
        }
        break;
    default:
        return error_value(E_CIS_2);
    }

    /*
     * case: return NUMBER value
     */
    if (cisreal(c)) {
        result.v_num = c_to_q(c, true);
        result.v_type = V_NUM;
        return result;
    }

    /*
     * case: return COMPLEX value
     */
    result.v_com = c;
    result.v_type = V_COM;
    return result;
}

#endif

/*
 * builtins - List of primitive built-in functions
 *
 * NOTE:  This table is also used by the help/Makefile builtin rule to
 *        form the builtin help file.  This rule will cause a sed script
 *        to strip this table down into a just the information needed
 *        to print builtin function list: b_name, b_minargs, b_maxargs
 *        and b_desc.  All other struct elements will be converted to 0.
 *        The sed script expects to find entries of the form:
 *
 *              {"...", number, number, stuff, stuff, stuff, stuff,
 *               "...."},
 *
 *        please keep this table in that form.
 *
 *        For nice output, when the description of function (b_desc)
 *        gets too long (extends into col 79) you should chop the
 *        line and add "\n\t\t\t", that's newline and 3 tabs.
 *        For example the description:
 *
 *              ... very long description that goes beyond col 79
 *
 *        should be written as:
 *
 *              "... very long description that\n\t\t\tgoes beyond col 79"},
 *
 * fields:
 *      b_name          name of built-in function
 *      b_minargs       minimum number of arguments
 *      b_maxargs       maximum number of arguments
 *      b_flags         special handling flags
 *      b_opcode        opcode which makes the call quick
 *      b_numfunc       routine to calculate numeric function
 *      b_valfunc       routine to calculate general values
 *      b_desc          description of function
 */
static const struct builtin builtins[] = {
    /* clang-format off */
        {"abs", 1, 2, 0, OP_ABS, {.null = NULL}, {.null = NULL},
         "absolute value, within accuracy b"},
        {"access", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_access},
         "determine accessibility of file a for mode b"},
        {"acos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acos},
         "inverse cosine of a, within accuracy b"},
        {"acosh", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acosh},
         "inverse hyperbolic cosine of a, within accuracy b"},
        {"acot", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acot},
         "inverse cotangent of a, within accuracy b"},
        {"acoth", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acoth},
         "inverse hyperbolic cotangent of a, within accuracy b"},
        {"acovercos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acovercos},
         "inverse coversed cosine of a, within accuracy b"},
        {"acoversin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acoversin},
         "inverse coversed sine of a, within accuracy b"},
        {"acrd", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acrd},
         "angle of unit circle chord with length a, within accuracy b"},
        {"acsc", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acsc},
         "inverse cosecant of a, within accuracy b"},
        {"acsch", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_acsch},
         "inverse csch of a, within accuracy b"},
        {"aexcsc", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_aexcsc},
         "inverse exterior cosecant of a, within accuracy b"},
        {"aexsec", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_aexsec},
         "inverse exterior secant of a, within accuracy b"},
        {"agd", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_agd},
         "inverse Gudermannian function"},
        {"ahacovercos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_ahacovercos},
         "inverse half coversed cosine of a, within accuracy b"},
        {"ahacoversin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_ahacoversin},
         "inverse half coversed sine of a, within accuracy b"},
        {"ahavercos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_ahavercos},
         "inverse half versed cosine of a, within accuracy b"},
        {"ahaversin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_ahaversin},
         "inverse half versed sine of a, within accuracy b"},
        {"append", 1, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_listappend},
         "append values to end of list"},
        {"appr", 1, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_appr},
         "approximate a by multiple of b using rounding c"},
        {"arg", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_arg},
         "argument (the angle) of complex number"},
        {"argv", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_argv},
         "calc argc or argv string"},
        {"asec", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_asec},
         "inverse secant of a, within accuracy b"},
        {"asech", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_asech},
         "inverse hyperbolic secant of a, within accuracy b"},
        {"asin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_asin},
         "inverse sine of a, within accuracy b"},
        {"asinh", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_asinh},
         "inverse hyperbolic sine of a, within accuracy b"},
        {"assoc", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_assoc},
         "create new association array"},
        {"atan", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_atan},
         "inverse tangent of a, within accuracy b"},
        {"atan2", 2, 3, FE, OP_NOP, {.numfunc_3 = qatan2}, {.null = NULL},
         "angle to point (b,a) within accuracy c"},
        {"atanh", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_atanh},
         "inverse hyperbolic tangent of a, within accuracy b"},
        {"avercos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_avercos},
         "inverse versed cosine of a, within accuracy b"},
        {"aversin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_aversin},
         "inverse versed sine of a, within accuracy b"},
        {"avg", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_avg},
         "arithmetic mean of values"},
        {"base", 0, 1, 0, OP_NOP, {.numfunc_cnt = f_base}, {.null = NULL},
         "set default output base"},
        {"base2", 0, 1, 0, OP_NOP, {.numfunc_cnt = f_base2}, {.null = NULL},
         "set default secondary output base"},
        {"bernoulli", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_bern},
         "Bernoulli number for index a"},
        {"bit", 2, 2, 0, OP_BIT, {.null = NULL}, {.null = NULL},
         "whether bit b in value a is set"},
        {"blk", 0, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_blk},
         "block with or without name, octet number, chunksize"},
        {"blkcpy", 2, 5, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_blkcpy},
         "copy value to/from a block: blkcpy(d,s,len,di,si)"},
        {"blkfree", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_blkfree},
         "free all storage from a named block"},
        {"blocks", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_blocks},
         "named block with specified index, or null value"},
        {"bround", 1, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_bround},
         "round value a to b number of binary places"},
        {"btrunc", 1, 2, 0, OP_NOP, {.numfunc_cnt = f_btrunc}, {.null = NULL},
         "truncate a to b number of binary places"},
        {"calclevel", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_calclevel},
         "current calculation level"},
        {"calcpath", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_calcpath},
         "current CALCPATH search path value"},
        {"calc_tty", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_calc_tty},
         "set tty for interactivity"},
        {"cas", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_cas},
         "cosine plus sine, within accuracy b"},
        {"catalan", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_catalan},
         "catalan number for index a"},
        {"ceil", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_ceil},
         "smallest integer greater than or equal to number"},
        {"cfappr", 1, 3, 0, OP_NOP, {.numfunc_cnt = f_cfappr}, {.null = NULL},
         "approximate a, within accuracy b using\n"
         "\t\t\tcontinued fractions"},
        {"cfsim", 1, 2, 0, OP_NOP, {.numfunc_cnt = f_cfsim}, {.null = NULL},
         "simplify number using continued fractions"},
        {"char", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_char},
         "character corresponding to integer value"},
        {"cis", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_cis},
         "Euler's formula, within accuracy b"},
        {"cmdbuf", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_cmdbuf},
         "command buffer"},
        {"cmp", 2, 2, 0, OP_CMP, {.null = NULL}, {.null = NULL},
         "compare values returning -1, 0, or 1"},
        {"comb", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_comb},
         "combinatorial number a!/b!(a-b)!"},
        {"config", 1, 2, 0, OP_SETCONFIG, {.null = NULL}, {.null = NULL},
         "set or read configuration value"},
        {"conj", 1, 1, 0, OP_CONJUGATE, {.null = NULL}, {.null = NULL},
         "complex conjugate of value"},
        {"copy", 2, 5, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_copy},
         "copy value to/from a block: copy(s,d,len,si,di)"},
        {"cos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_cos},
         "cosine of value a, within accuracy b"},
        {"cosh", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_cosh},
         "hyperbolic cosine of a, within accuracy b"},
        {"cot", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_cot},
         "cotangent of a, within accuracy b"},
        {"coth", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_coth},
         "hyperbolic cotangent of a, within accuracy b"},
        {"count", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_count},
         "count listr/matrix elements satisfying some condition"},
        {"covercos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_covercos},
         "coversed cosine of value a, within accuracy b"},
        {"coversin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_coversin},
         "coversed sine of value a, within accuracy b"},
        {"cp", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_cp},
         "cross product of two vectors"},
        {"crd", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_crd},
         "length of unit circle chord with angle a, within accuracy b"},
        {"csc", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_csc},
         "cosecant of a, within accuracy b"},
        {"csch", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_csch},
         "hyperbolic cosecant of a, within accuracy b"},
        {"ctime", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_ctime},
         "date and time as string"},
        {"custom", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_custom},
         "custom builtin function interface"},
        {"d2dm", 3, 4, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_d2dm},
         "convert a to b deg, c min, rounding type d\n"},
        {"d2dms", 4, 5, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_d2dms},
         "convert a to b deg, c min, d sec, rounding type e\n"},
        {"d2g", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_d2g},
         "convert degrees to gradians"},
        {"d2r", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_d2r},
         "convert degrees to radians"},
        {"delete", 2, 2, FA, OP_NOP, {.null = NULL}, {.valfunc_2 = f_listdelete},
         "delete element from list a at position b"},
        {"den", 1, 1, 0, OP_DENOMINATOR, {.numfunc_1 = qden}, {.null = NULL},
         "denominator of fraction"},
        {"det", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_det},
         "determinant of matrix"},
        {"digit", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_digit},
         "digit at specified decimal place of number"},
        {"digits", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_digits},
         "number of digits in base b representation of a"},
        {"display", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_display},
         "number of decimal digits for displaying numbers"},
        {"dm2d", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_dm2d},
         "convert a deg, b min to degrees, rounding type c\n"},
        {"dms2d", 3, 4, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_dms2d},
         "convert a deg, b min, c sec to degrees, rounding type d\n"},
        {"dp", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_dp},
         "dot product of two vectors"},
        {"epsilon", 0, 1, 0, OP_SETEPSILON, {.null = NULL}, {.null = NULL},
         "set or read allowed error for real calculations"},
        {"errcount", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_errcount},
         "set or read error count"},
        {"errmax", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_errmax},
         "set or read maximum for error count"},
        {"errno", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_errno},
         "set or read calc_errno"},
        {"error", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_error},
         "generate error value"},
        {"errsym", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_errsym},
         "convert between E_STRING errsym into a errnum number"},
        {"estr", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_estr},
         "exact text string representation of value"},
        {"euler", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_euler},
         "Euler number"},
        {"eval", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_eval},
         "evaluate expression from string to value"},
        {"excsc", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_excsc},
         "exterior cosecant of a, within accuracy b"},
        {"exp", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_exp},
         "exponential of value a, within accuracy b"},
        {"exsec", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_exsec},
         "exterior secant of a, within accuracy b"},
        {"fact", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_fact},
         "factorial"},
        {"factor", 1, 3, 0, OP_NOP, {.numfunc_cnt = f_factor}, {.null = NULL},
         "lowest prime factor < b of a, return c if error"},
        {"fclose", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fclose},
         "close file"},
        {"fcnt", 2, 2, 0, OP_NOP, {.numfunc_2 = f_faccnt}, {.null = NULL},
         "count of times one number divides another"},
        {"feof", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_feof},
         "whether EOF reached for file"},
        {"ferror", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_ferror},
         "whether error occurred for file"},
        {"fflush", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fflush},
         "flush output to file(s)"},
        {"fgetc", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_fgetc},
         "read next char from file"},
        {"fgetfield", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_fgetfield},
         "read next white-space delimited field from file"},
        {"fgetfile", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_fgetfile},
         "read to end of file"},
        {"fgetline", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_fgetline},
         "read next line from file, newline removed"},
        {"fgets", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_fgets},
         "read next line from file, newline is kept"},
        {"fgetstr", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_fgetstr},
         "read next null-terminated string from file, null\n"
         "\t\t\tcharacter is kept"},
        {"fib", 1, 1, 0, OP_NOP, {.numfunc_1 = qfib}, {.null = NULL},
         "Fibonacci number F(n)"},
        {"files", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_files},
         "return opened file or max number of opened files"},
        {"floor", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_floor},
         "greatest integer less than or equal to number"},
        {"fopen", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_fopen},
         "open file name a in mode b"},
        {"forall", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_forall},
         "do function for all elements of list or matrix"},
        {"fpathopen", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fpathopen},
         "open file name a in mode b, search for a along\n"
         "\t\t\tCALCPATH or path c"},
        {"fprintf", 2, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fprintf},
         "print formatted output to opened file"},
        {"fputc", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_fputc},
         "write a character to a file"},
        {"fputs", 2, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fputs},
         "write one or more strings to a file"},
        {"fputstr", 2, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fputstr},
         "write one or more null-terminated strings to a file"},
        {"frac", 1, 1, 0, OP_FRAC, {.numfunc_1 = qfrac}, {.null = NULL},
         "fractional part of value"},
        {"free", 0, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_free},
         "free listed or all global variables"},
        {"freebernoulli", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_freebern},
         "free stored Bernoulli numbers"},
        {"freeeuler", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_freeeuler},
         "free stored Euler numbers"},
        {"freeglobals", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_freeglobals},
         "free all global and visible static variables"},
        {"freeredc", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_freeredc},
         "free redc data cache"},
        {"freestatics", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_freestatics},
         "free all un-scoped static variables"},
        {"frem", 2, 2, 0, OP_NOP, {.numfunc_2 = qfacrem}, {.null = NULL},
         "number with all occurrences of factor removed"},
        {"freopen", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_freopen},
         "reopen a file stream to a named file"},
        {"fscan", 2, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fscan},
         "scan a file for assignments to one or\n"
         "\t\t\tmore variables"},
        {"fscanf", 2, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fscanf},
         "formatted scan of a file for assignment to one\n"
         "\t\t\tor more variables"},
        {"fseek", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_fseek},
         "seek to position b (offset from c) in file a"},
        {"fsize", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_fsize},
         "return the size of the file"},
        {"ftell", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_ftell},
         "return the file position"},
        {"g2d", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_g2d},
         "convert gradians to degrees"},
        {"g2gm", 3, 4, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_g2gm},
         "convert a to b grads, c min, rounding type d\n"},
        {"g2gms", 4, 5, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_g2gms},
         "convert a to b grads, c min, d sec, rounding type e\n"},
        {"g2r", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_g2r},
         "convert gradians to radians"},
        {"gcd", 1, IN, 0, OP_NOP, {.numfunc_cnt = f_gcd}, {.null = NULL},
         "greatest common divisor"},
        {"gcdrem", 2, 2, 0, OP_NOP, {.numfunc_2 = qgcdrem}, {.null = NULL},
         "a divided repeatedly by gcd with b"},
        {"gd", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_gd},
         "Gudermannian function"},
        {"getenv", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_getenv},
         "value of environment variable (or NULL)"},
        {"gm2g", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_gm2g},
         "convert a grads, b min to grads, rounding type c\n"},
        {"gms2g", 3, 4, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_gms2g},
         "convert a grads, b min, c sec to grads, rounding type d\n"},
        {"h2hm", 3, 4, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_h2hm},
         "convert a to b hours, c min, rounding type d\n"},
        {"h2hms", 4, 5, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_h2hms},
         "convert a to b hours, c min, d sec, rounding type e\n"},
        {"hacovercos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_hacovercos},
         "half coversed cosine of value a, within accuracy b"},
        {"hacoversin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_hacoversin},
         "half coversed sine of value a, within accuracy b"},
        {"hash", 1, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_hash},
         "return non-negative hash value for one or\n"
         "\t\t\tmore values"},
        {"havercos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_havercos},
         "half versed cosine of value a, within accuracy b"},
        {"haversin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_haversin},
         "half versed sine of value a, within accuracy b"},
        {"head", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_head},
         "return list of specified number at head of a list"},
        {"highbit", 1, 1, 0, OP_HIGHBIT, {.null = NULL}, {.null = NULL},
         "high bit number in base 2 representation"},
        {"hm2h", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_hm2h},
         "convert a hours, b min to hours, rounding type c\n"},
        {"hmean", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_hmean},
         "harmonic mean of values"},
        {"hms2h", 3, 4, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_hms2h},
         "convert a hours, b min, c sec to hours, rounding type d\n"},
        {"hnrmod", 4, 4, 0, OP_NOP, {.numfunc_4 = f_hnrmod}, {.null = NULL},
         "v mod h*2^n+r, h>0, n>0, r  =  -1, 0 or 1"},
        {"hypot", 2, 3, FE, OP_NOP, {.numfunc_3 = qhypot}, {.null = NULL},
         "hypotenuse of right triangle, within accuracy c"},
        {"ilog", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_ilog},
         "integral log of a to integral base b"},
        {"ilog10", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_ilog10},
         "integral log of a number base 10"},
        {"ilog2", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_ilog2},
         "integral log of a number base 2"},
        {"ilogn", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_ilog},
         "same is ilog"},
        {"im", 1, 1, 0, OP_IM, {.null = NULL}, {.null = NULL},
         "imaginary part of complex number"},
        {"indices", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_indices},
         "indices of a specified assoc or mat value"},
        {"inputlevel", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_inputlevel},
         "current input depth"},
        {"insert", 2, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_listinsert},
         "insert values c ... into list a at position b"},
        {"int", 1, 1, 0, OP_INT, {.numfunc_1 = qint}, {.null = NULL},
         "integer part of value"},
        {"inverse", 1, 1, 0, OP_INVERT, {.null = NULL}, {.null = NULL},
         "multiplicative inverse of value"},
        {"iroot", 2, 2, 0, OP_NOP, {.numfunc_2 = qiroot}, {.null = NULL},
         "integer b'th root of a"},
        {"isalnum", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isalnum},
         "whether character is alpha-numeric"},
        {"isalpha", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isalpha},
         "whether character is alphabetic"},
        {"isassoc", 1, 1, 0, OP_ISASSOC, {.null = NULL}, {.null = NULL},
         "whether a value is an association"},
        {"isatty", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isatty},
         "whether a file is a tty"},
        {"isblk", 1, 1, 0, OP_ISBLK, {.null = NULL}, {.null = NULL},
         "whether a value is a block"},
        {"iscntrl", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_iscntrl},
         "whether character is a control character"},
        {"isconfig", 1, 1, 0, OP_ISCONFIG, {.null = NULL}, {.null = NULL},
         "whether a value is a config state"},
        {"isdefined", 1, 1, 0, OP_ISDEFINED, {.null = NULL}, {.null = NULL},
         "whether a string names a function"},
        {"isdigit", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isdigit},
         "whether character is a digit"},
        {"iserror", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_iserror},
         "where a value is an error"},
        {"iseven", 1, 1, 0, OP_ISEVEN, {.null = NULL}, {.null = NULL},
         "whether a value is an even integer"},
        {"isfile", 1, 1, 0, OP_ISFILE, {.null = NULL}, {.null = NULL},
         "whether a value is a file"},
        {"isgraph", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isgraph},
         "whether character is a graphical character"},
        {"ishash", 1, 1, 0, OP_ISHASH, {.null = NULL}, {.null = NULL},
         "whether a value is a hash state"},
        {"isident", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isident},
         "returns 1 if identity matrix"},
        {"isint", 1, 1, 0, OP_ISINT, {.null = NULL}, {.null = NULL},
         "whether a value is an integer"},
        {"islist", 1, 1, 0, OP_ISLIST, {.null = NULL}, {.null = NULL},
         "whether a value is a list"},
        {"islower", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_islower},
         "whether character is lower case"},
        {"ismat", 1, 1, 0, OP_ISMAT, {.null = NULL}, {.null = NULL},
         "whether a value is a matrix"},
        {"ismult", 2, 2, 0, OP_NOP, {.numfunc_2 = f_ismult}, {.null = NULL},
         "whether a is a multiple of b"},
        {"isnull", 1, 1, 0, OP_ISNULL, {.null = NULL}, {.null = NULL},
         "whether a value is the null value"},
        {"isnum", 1, 1, 0, OP_ISNUM, {.null = NULL}, {.null = NULL},
         "whether a value is a number"},
        {"isobj", 1, 1, 0, OP_ISOBJ, {.null = NULL}, {.null = NULL},
         "whether a value is an object"},
        {"isobjtype", 1, 1, 0, OP_ISOBJTYPE, {.null = NULL}, {.null = NULL},
         "whether a string names an object type"},
        {"isoctet", 1, 1, 0, OP_ISOCTET, {.null = NULL}, {.null = NULL},
         "whether a value is an octet"},
        {"isodd", 1, 1, 0, OP_ISODD, {.null = NULL}, {.null = NULL},
         "whether a value is an odd integer"},
        {"isprime", 1, 2, 0, OP_NOP, {.numfunc_cnt = f_isprime}, {.null = NULL},
         "whether a is a small prime, return b if error"},
        {"isprint", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isprint},
         "whether character is printable"},
        {"isptr", 1, 1, 0, OP_ISPTR, {.null = NULL}, {.null = NULL},
         "whether a value is a pointer"},
        {"ispunct", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_ispunct},
         "whether character is a punctuation"},
        {"isqrt", 1, 1, 0, OP_NOP, {.numfunc_1 = qisqrt}, {.null = NULL},
         "integer part of square root"},
        {"isrand", 1, 1, 0, OP_ISRAND, {.null = NULL}, {.null = NULL},
         "whether a value is a subtractive 100 state"},
        {"israndom", 1, 1, 0, OP_ISRANDOM, {.null = NULL}, {.null = NULL},
         "whether a value is a Blum state"},
        {"isreal", 1, 1, 0, OP_ISREAL, {.null = NULL}, {.null = NULL},
         "whether a value is a real number"},
        {"isrel", 2, 2, 0, OP_NOP, {.numfunc_2 = f_isrel}, {.null = NULL},
         "whether two numbers are relatively prime"},
        {"issimple", 1, 1, 0, OP_ISSIMPLE, {.null = NULL}, {.null = NULL},
         "whether value is a simple type"},
        {"isspace", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isspace},
         "whether character is a space character"},
        {"issq", 1, 1, 0, OP_NOP, {.numfunc_1 = f_issquare}, {.null = NULL},
         "whether or not number is a square"},
        {"isstr", 1, 1, 0, OP_ISSTR, {.null = NULL}, {.null = NULL},
         "whether a value is a string"},
        {"istype", 2, 2, 0, OP_ISTYPE, {.null = NULL}, {.null = NULL},
         "whether the type of a is same as the type of b"},
        {"isupper", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isupper},
         "whether character is upper case"},
        {"isxdigit", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_isxdigit},
         "whether character is a hexadecimal digit"},
        {"jacobi", 2, 2, 0, OP_NOP, {.numfunc_2 = qjacobi}, {.null = NULL},
         "-1  = > a is not quadratic residue mod b\n"
         "\t\t\t1  = > b is composite, or a is quad residue of b"},
        {"join", 1, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_join},
         "join one or more lists into one list"},
        {"lcm", 1, IN, 0, OP_NOP, {.numfunc_cnt = f_lcm}, {.null = NULL},
         "least common multiple"},
        {"lcmfact", 1, 1, 0, OP_NOP, {.numfunc_1 = qlcmfact}, {.null = NULL},
         "lcm of all integers up till number"},
        {"lfactor", 2, 2, 0, OP_NOP, {.numfunc_2 = qlowfactor}, {.null = NULL},
         "lowest prime factor of a in first b primes"},
        {"links", 1, 1, 0, OP_LINKS, {.null = NULL}, {.null = NULL},
         "links to number or string value"},
        {"list", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_list},
         "create list of specified values"},
        {"ln", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_ln},
         "natural logarithm of value a, within accuracy b"},
        {"log", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_log},
         "base 10 logarithm of value a, within accuracy b"},
        {"log2", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_log2},
         "base 2 logarithm of value a, within accuracy b"},
        {"logn", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_logn},
         "base b logarithm of value a, within accuracy c"},
        {"lowbit", 1, 1, 0, OP_LOWBIT, {.null = NULL}, {.null = NULL},
         "low bit number in base 2 representation"},
        {"ltol", 1, 2, FE, OP_NOP, {.numfunc_2 = f_legtoleg}, {.null = NULL},
         "leg-to-leg of unit right triangle (sqrt(1 - a^2))"},
        {"makelist", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_makelist},
         "create a list with a null elements"},
        {"matdim", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_matdim},
         "number of dimensions of matrix"},
        {"matfill", 2, 3, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_matfill},
         "fill matrix with value b (value c on diagonal)"},
        {"matmax", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_matmax},
         "maximum index of matrix a dim b"},
        {"matmin", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_matmin},
         "minimum index of matrix a dim b"},
        {"matsum", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_matsum},
         "sum the numeric values in a matrix"},
        {"mattrace", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_mattrace},
         "return the trace of a square matrix"},
        {"mattrans", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_mattrans},
         "transpose of matrix"},
        {"max", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_max},
         "maximum value"},
        {"memsize", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_memsize},
         "number of octets used by the value, including overhead"},
        {"meq", 3, 3, 0, OP_NOP, {.numfunc_3 = f_meq}, {.null = NULL},
         "whether a and b are equal modulo c"},
        {"min", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_min},
         "minimum value"},
        {"minv", 2, 2, 0, OP_NOP, {.numfunc_2 = qminv}, {.null = NULL},
         "inverse of a modulo b"},
        {"mmin", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_mmin},
         "a mod b value with smallest abs value"},
        {"mne", 3, 3, 0, OP_NOP, {.numfunc_3 = f_mne}, {.null = NULL},
         "whether a and b are not equal modulo c"},
        {"mod", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_mod},
         "residue of a modulo b, rounding type c"},
        {"modify", 2, 2, FA, OP_NOP, {.null = NULL}, {.valfunc_2 = f_modify},
         "modify elements of a list or matrix"},
        {"name", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_name},
         "name assigned to block or file"},
        {"near", 2, 3, 0, OP_NOP, {.numfunc_cnt = f_near}, {.null = NULL},
         "sign of (abs(a-b) - c)"},
        {"newerror", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_newerror},
         "create new error type with message a"},
        {"nextcand", 1, 5, 0, OP_NOP, {.numfunc_cnt = f_nextcand}, {.null = NULL},
         "smallest value  =  =  d mod e > a, ptest(a,b,c) true"},
        {"nextprime", 1, 2, 0, OP_NOP, {.numfunc_cnt = f_nprime}, {.null = NULL},
         "return next small prime, return b if err"},
        {"norm", 1, 1, 0, OP_NORM, {.null = NULL}, {.null = NULL},
         "norm of a value (square of absolute value)"},
        {"null", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_null},
         "null value"},
        {"num", 1, 1, 0, OP_NUMERATOR, {.numfunc_1 = qnum}, {.null = NULL},
         "numerator of fraction"},
        {"ord", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_ord},
         "integer corresponding to character value"},
        {"param", 1, 1, 0, OP_ARGVALUE, {.null = NULL}, {.null = NULL},
         "value of parameter n (or parameter count if n\n"
         "\t\t\tis zero)"},
        {"perm", 2, 2, 0, OP_NOP, {.numfunc_2 = qperm}, {.null = NULL},
         "permutation number a!/(a-b)!"},
        {"pfact", 1, 1, 0, OP_NOP, {.numfunc_1 = qpfact}, {.null = NULL},
         "product of primes up till number"},
        {"pi", 0, 1, FE, OP_NOP, {.numfunc_1 = qpi}, {.null = NULL},
         "value of pi, within accuracy a"},
        {"pix", 1, 2, 0, OP_NOP, {.numfunc_cnt = f_pix}, {.null = NULL},
         "number of primes < =  a < 2^32, return b if error"},
        {"places", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_places},
         "places after \"decimal\" point (-1 if infinite)"},
        {"pmod", 3, 3, 0, OP_NOP, {.numfunc_3 = qpowermod}, {.null = NULL},
         "mod of a power (a ^ b (mod c))"},
        {"polar", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_polar},
         "complex value of polar coordinate (a * exp(b*1i))"},
        {"poly", 1, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_poly},
         "evaluates a polynomial given its coefficients\n"
         "\t\t\tor coefficient-list"},
        {"pop", 1, 1, FA, OP_NOP, {.null = NULL}, {.valfunc_1 = f_listpop},
         "pop value from front of list"},
        {"popcnt", 1, 2, 0, OP_NOP, {.numfunc_cnt = f_popcnt}, {.null = NULL},
         "number of bits in a that match b (or 1)"},
        {"power", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_power},
         "value a raised to the power b, within accuracy c"},
        {"prevcand", 1, 5, 0, OP_NOP, {.numfunc_cnt = f_prevcand}, {.null = NULL},
         "largest value  =  =  d mod e < a, ptest(a,b,c) true"},
        {"prevprime", 1, 2, 0, OP_NOP, {.numfunc_cnt = f_pprime}, {.null = NULL},
         "return previous small prime, return b if err"},
        {"printf", 1, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_printf},
         "print formatted output to stdout"},
        {"prompt", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_prompt},
         "prompt for input line using value a"},
        {"protect", 1, 3, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_protect},
         "read or set protection level for variable"},
        {"ptest", 1, 3, 0, OP_NOP, {.numfunc_cnt = f_primetest}, {.null = NULL},
         "probabilistic primality test"},
        {"push", 1, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_listpush},
         "push values onto front of list"},
        {"putenv", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_putenv},
         "define an environment variable"},
        {"quo", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_quo},
         "integer quotient of a by b, rounding type c"},
        {"quomod", 4, 5, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_quomod},
         "set c and d to quotient and remainder of a\n"
         "\t\t\tdivided by b"},
        {"r2d", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_r2d},
         "convert radians to degrees"},
        {"r2g", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_r2g},
         "convert radians to gradians"},
        {"rand", 0, 2, 0, OP_NOP, {.numfunc_cnt = f_rand}, {.null = NULL},
         "subtractive 100 random number [0,2^64), [0,a), or [a,b)"},
        {"randbit", 0, 1, 0, OP_NOP, {.numfunc_cnt = f_randbit}, {.null = NULL},
         "subtractive 100 random number [0,2^a)"},
        {"random", 0, 2, 0, OP_NOP, {.numfunc_cnt = f_random}, {.null = NULL},
         "Blum-Blum-Shub random number [0,2^64), [0,a), or [a,b)"},
        {"randombit", 0, 1, 0, OP_NOP, {.numfunc_cnt = f_randombit}, {.null = NULL},
         "Blum-Blum-Sub random number [0,2^a)"},
        {"randperm", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_randperm},
         "random permutation of a list or matrix"},
        {"rcin", 2, 2, 0, OP_NOP, {.numfunc_2 = qredcin}, {.null = NULL},
         "convert normal number a to REDC number mod b"},
        {"rcmul", 3, 3, 0, OP_NOP, {.numfunc_3 = qredcmul}, {.null = NULL},
         "multiply REDC numbers a and b mod c"},
        {"rcout", 2, 2, 0, OP_NOP, {.numfunc_2 = qredcout}, {.null = NULL},
         "convert REDC number a mod b to normal number"},
        {"rcpow", 3, 3, 0, OP_NOP, {.numfunc_3 = qredcpower}, {.null = NULL},
         "raise REDC number a to power b mod c"},
        {"rcsq", 2, 2, 0, OP_NOP, {.numfunc_2 = qredcsquare}, {.null = NULL},
         "square REDC number a mod b"},
        {"re", 1, 1, 0, OP_RE, {.null = NULL}, {.null = NULL},
         "real part of complex number"},
        {"remove", 1, 1, FA, OP_NOP, {.null = NULL}, {.valfunc_1 = f_listremove},
         "remove value from end of list"},
        {"reverse", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_reverse},
         "reverse a copy of a matrix or list"},
        {"rewind", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_rewind},
         "rewind file(s)"},
        {"rm", 1, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_rm},
         "remove file(s), -f turns off no-such-file errors"},
        {"root", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_root},
         "value a taken to the b'th root, within accuracy c"},
        {"round", 1, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_round},
         "round value a to b number of decimal places"},
        {"rsearch", 2, 4, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_rsearch},
         "reverse search matrix or list for value b\n"
         "\t\t\tstarting at index c"},
        {"runtime", 0, 0, 0, OP_NOP, {.numfunc_0 = f_runtime}, {.null = NULL},
         "user and kernel mode CPU time in seconds"},
        {"saveval", 1, 1, 0, OP_SAVEVAL, {.null = NULL}, {.null = NULL},
         "set flag for saving values"},
        {"scale", 2, 2, 0, OP_SCALE, {.null = NULL}, {.null = NULL},
         "scale value up or down by a power of two"},
        {"scan", 1, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_scan},
         "scan standard input for assignment to one\n"
         "\t\t\tor more variables"},
        {"scanf", 2, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_scanf},
         "formatted scan of standard input for assignment\n"
         "\t\t\tto variables"},
        {"search", 2, 4, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_search},
         "search matrix or list for value b starting\n"
         "\t\t\tat index c"},
        {"sec", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_sec},
         "secant of a, within accuracy b"},
        {"sech", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_sech},
         "hyperbolic secant of a, within accuracy b"},
        {"seed", 0, 0, 0, OP_NOP, {.numfunc_0 = f_seed}, {.null = NULL},
         "return a 64 bit seed for a pseudo-random generator"},
        {"segment", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_segment},
         "specified segment of specified list"},
        {"select", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_select},
         "form sublist of selected elements from list"},
        {"setbit", 2, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_setbit},
         "set specified bit in string"},
        {"sgn", 1, 1, 0, OP_SGN, {.numfunc_1 = qsign}, {.null = NULL},
         "sign of value (-1, 0, 1)"},
        {"sha1", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_sha1},
         "Secure Hash Algorithm (SHS-1 FIPS Pub 180-1)"},
        {"sin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_sin},
         "sine of value a, within accuracy b"},
        {"sinh", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_sinh},
         "hyperbolic sine of a, within accuracy b"},
        {"size", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_size},
         "total number of elements in value"},
        {"sizeof", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_sizeof},
         "number of octets used to hold the value"},
        {"sleep", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_sleep},
         "suspend operation for a seconds"},
        {"sort", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_sort},
         "sort a copy of a matrix or list"},
        {"sqrt", 1, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_sqrt},
         "square root of value a, within accuracy b"},
        {"srand", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_srand},
         "seed the rand() function"},
        {"srandom", 0, 4, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_srandom},
         "seed the random() function"},
        {"ssq", 1, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_ssq},
         "sum of squares of values"},
        {"stoponerror", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_stoponerror},
         "assign value to stoponerror flag"},
        {"str", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_str},
         "simple value converted to string"},
        {"strcasecmp", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_strcasecmp},
         "compare two strings case independent"},
        {"strcat", 1,IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_strcat},
         "concatenate strings together"},
        {"strcmp", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_strcmp},
         "compare two strings"},
        {"strcpy", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_strcpy},
         "copy string to string"},
        {"strerror", 0, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_strerror},
         "string describing error type"},
        {"strlen", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_strlen},
         "length of string"},
        {"strncasecmp", 3, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_3 = f_strncasecmp},
         "compare strings a, b to c characters case independent"},
        {"strncmp", 3, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_3 = f_strncmp},
         "compare strings a, b to c characters"},
        {"strncpy", 3, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_3 = f_strncpy},
         "copy up to c characters from string to string"},
        {"strpos", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_strpos},
         "index of first occurrence of b in a"},
        {"strprintf", 1, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_strprintf},
         "return formatted output as a string"},
        {"strscan", 2, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_strscan},
         "scan a string for assignments to one or more variables"},
        {"strscanf", 2, IN, FA, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_strscanf},
         "formatted scan of string for assignments to variables"},
        {"strtolower", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_strtolower},
         "Make string lower case"},
        {"strtoupper", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_strtoupper},
         "Make string upper case"},
        {"substr", 3, 3, 0, OP_NOP, {.null = NULL}, {.valfunc_3 = f_substr},
         "substring of a from position b for c chars"},
        {"sum", 0, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_sum},
         "sum of list or object sums and/or other terms"},
        {"swap", 2, 2, 0, OP_SWAP, {.null = NULL}, {.null = NULL},
         "swap values of variables a and b (can be dangerous)"},
        {"system", 1, 1, 0, OP_NOP, {.null = NULL}, {.valfunc_1 = f_system},
         "call Unix command"},
        {"systime", 0, 0, 0, OP_NOP, {.numfunc_0 = f_systime}, {.null = NULL},
         "kernel mode CPU time in seconds"},
        {"tail", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_tail},
         "retain list of specified number at tail of list"},
        {"tan", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_tan},
         "tangent of a, within accuracy b"},
        {"tanh", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_tanh},
         "hyperbolic tangent of a, within accuracy b"},
        {"test", 1, 1, 0, OP_TEST, {.null = NULL}, {.null = NULL},
         "test that value is nonzero"},
        {"time", 0, 0, 0, OP_NOP, {.numfunc_0 = f_time}, {.null = NULL},
         "number of seconds since 00:00:00 1 Jan 1970 UTC"},
        {"trunc", 1, 2, 0, OP_NOP, {.numfunc_cnt = f_trunc}, {.null = NULL},
         "truncate a to b number of decimal places"},
        {"ungetc", 2, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_2 = f_ungetc},
         "unget char read from file"},
        {"usertime", 0, 0, 0, OP_NOP, {.numfunc_0 = f_usertime}, {.null = NULL},
         "user mode CPU time in seconds"},
        {"vercos", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_vercos},
         "versed cosine of value a, within accuracy b"},
        {"versin", 1, 2, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_versin},
         "versed sine of value a, within accuracy b"},
        {"version", 0, 0, 0, OP_NOP, {.null = NULL}, {.valfunc_0 = f_version},
         "calc version string"},
        {"xor", 1, IN, 0, OP_NOP, {.null = NULL}, {.valfunc_cnt = f_xor},
         "logical xor"},

        /* end of table */
        {NULL, 0, 0, 0, 0, {.null = NULL}, {.null = NULL},
         NULL}
    /* clang-format on */
};

/*
 * Show the list of primitive built-in functions
 *
 * When FUNCLIST is defined, we are being compiled by rules from the help
 * sub-directory to form a program that will produce the main part of the
 * builtin help file.
 *
 * See the builtin rule in the help/Makefile for details.
 */
#if defined(FUNCLIST)

int
main(void)
{
    const struct builtin *bp; /* current function */

    printf("\nName\tArgs\tDescription\n\n");
    for (bp = builtins; bp->b_name; bp++) {
        printf("%-9s ", bp->b_name);
        if (bp->b_maxargs == IN) {
            printf("%d+    ", bp->b_minargs);
        } else if (bp->b_minargs == bp->b_maxargs) {
            printf("%-6d", bp->b_minargs);
        } else {
            printf("%d-%-4d", bp->b_minargs, bp->b_maxargs);
        }
        printf("%s\n", bp->b_desc);
    }
    printf("\n");
    return 0; /* exit(0); */
}

#else

void
showbuiltins(void)
{
    const struct builtin *bp; /* current function */
    int i;

    printf("\nName\tArgs\tDescription\n\n");
    for (bp = builtins, i = 0; bp->b_name; bp++, i++) {
        printf("%-14s ", bp->b_name);
        if (bp->b_maxargs == IN) {
            printf("%d+    ", bp->b_minargs);
        } else if (bp->b_minargs == bp->b_maxargs) {
            printf("%-6d", bp->b_minargs);
        } else {
            printf("%d-%-4d", bp->b_minargs, bp->b_maxargs);
        }
        printf("%s\n", bp->b_desc);
        if (i == 32) {
            i = 0;
            if (getchar() == 27) {
                break;
            }
        }
    }
    printf("\n");
}

#endif

#if !defined(FUNCLIST)

/*
 * Call a built-in function.
 * Arguments to the function are on the stack, but are not removed here.
 * Functions are either purely numeric, or else can take any value type.
 *
 * given:
 *      index           index on where to scan in builtin table
 *      argcount        number of args
 *      stck            arguments on the stack
 */
VALUE
builtinfunc(long index, int argcount, VALUE *stck)
{
    VALUE *sp;                /* pointer to stack entries */
    VALUE **vpp;              /* pointer to current value address */
    const struct builtin *bp; /* builtin function to be called */
    NUMBER *numargs[IN];      /* numeric arguments for function */
    VALUE *valargs[IN];       /* addresses of actual arguments */
    VALUE result;             /* general result of function */
    long i;

    if ((unsigned long)index >= (sizeof(builtins) / sizeof(builtins[0])) - 1) {
        math_error("Bad built-in function index");
        not_reached();
    }
    bp = &builtins[index];
    if (argcount < bp->b_minargs) {
        math_error("Too few arguments for builtin function \"%s\"", bp->b_name);
        not_reached();
    }
    if ((argcount > bp->b_maxargs) || (argcount > IN)) {
        math_error("Too many arguments for builtin function \"%s\"", bp->b_name);
        not_reached();
    }
    /*
     * If an address was passed, then point at the real variable,
     * otherwise point at the stack value itself (unless the function
     * is very special).
     */
    sp = stck - argcount + 1;
    vpp = valargs;
    for (i = argcount; i > 0; i--) {
        if ((sp->v_type != V_ADDR) || (bp->b_flags & FA)) {
            *vpp = sp;
        } else {
            *vpp = sp->v_addr;
        }
        sp++;
        vpp++;
    }
    /*
     * Handle general values if the function accepts them.
     */
    if (bp->b_valfunc.null != NULL) {
        vpp = valargs;
        if ((bp->b_minargs == 1) && (bp->b_maxargs == 1)) {
            result = (*bp->b_valfunc.valfunc_1)(vpp[0]);
        } else if ((bp->b_minargs == 2) && (bp->b_maxargs == 2)) {
            result = (*bp->b_valfunc.valfunc_2)(vpp[0], vpp[1]);
        } else if ((bp->b_minargs == 3) && (bp->b_maxargs == 3)) {
            result = (*bp->b_valfunc.valfunc_3)(vpp[0], vpp[1], vpp[2]);
        } else if ((bp->b_minargs == 4) && (bp->b_maxargs == 4)) {
            result = (*bp->b_valfunc.valfunc_4)(vpp[0], vpp[1], vpp[2], vpp[3]);
        } else {
            result = (*bp->b_valfunc.valfunc_cnt)(argcount, vpp);
        }
        return result;
    }
    /*
     * Function must be purely numeric, so handle that.
     */
    vpp = valargs;
    for (i = 0; i < argcount; i++) {
        if ((*vpp)->v_type != V_NUM) {
            math_error("Non-real argument for builtin function %s", bp->b_name);
            not_reached();
        }
        numargs[i] = (*vpp)->v_num;
        vpp++;
    }
    result.v_type = V_NUM;
    result.v_subtype = V_NOSUBTYPE;
    if (!(bp->b_flags & FE) && (bp->b_minargs != bp->b_maxargs)) {
        result.v_num = (*bp->b_numfunc.numfunc_cnt)(argcount, numargs);
        return result;
    }
    if ((bp->b_flags & FE) && (argcount < bp->b_maxargs)) {
        numargs[argcount++] = conf->epsilon;
    }

    switch (argcount) {
    case 0:
        result.v_num = (*bp->b_numfunc.numfunc_0)();
        break;
    case 1:
        result.v_num = (*bp->b_numfunc.numfunc_1)(numargs[0]);
        break;
    case 2:
        result.v_num = (*bp->b_numfunc.numfunc_2)(numargs[0], numargs[1]);
        break;
    case 3:
        result.v_num = (*bp->b_numfunc.numfunc_3)(numargs[0], numargs[1], numargs[2]);
        break;
    case 4:
        result.v_num = (*bp->b_numfunc.numfunc_4)(numargs[0], numargs[1], numargs[2], numargs[3]);
        break;
    default:
        math_error("Bad builtin function call");
        not_reached();
    }
    return result;
}

/*
 * Return the index of a built-in function given its name.
 * Returns minus one if the name is not known.
 */
int
getbuiltinfunc(char *name)
{
    const struct builtin *bp;

    for (bp = builtins; bp->b_name; bp++) {
        if ((*name == *bp->b_name) && (strcmp(name, bp->b_name) == 0)) {
            return (bp - builtins);
        }
    }
    return -1;
}

/*
 * Given the index of a built-in function, return its name.
 */
char *
builtinname(long index)
{
    if ((unsigned long)index >= (sizeof(builtins) / sizeof(builtins[0])) - 1) {
        return "";
    }
    return builtins[index].b_name;
}

/*
 * Given the index of a built-in function, and the number of arguments seen,
 * determine if the number of arguments are legal.  This routine is called
 * during parsing time.
 */
void
builtincheck(long index, int count)
{
    const struct builtin *bp;

    if ((unsigned long)index >= (sizeof(builtins) / sizeof(builtins[0])) - 1) {
        math_error("Unknown built in index");
        not_reached();
    }
    bp = &builtins[index];
    if (count < bp->b_minargs) {
        scanerror(T_NULL, "Too few arguments for builtin function \"%s\"", bp->b_name);
    }
    if (count > bp->b_maxargs) {
        scanerror(T_NULL, "Too many arguments for builtin function \"%s\"", bp->b_name);
    }
}

/*
 * Return the opcode for a built-in function that can be used to avoid
 * the function call at all.
 */
int
builtinopcode(long index)
{
    if ((unsigned long)index >= (sizeof(builtins) / sizeof(builtins[0])) - 1) {
        return OP_NOP;
    }
    return builtins[index].b_opcode;
}

/*
 * Show the error-values created by newerror(str).
 */
void
showerrors(void)
{
    int i;

    if (nexterrnum == E__USERDEF) {
        printf("No new error-values created\n");
    }
    for (i = E__USERDEF; i < nexterrnum; i++) {
        printf("%d: %s\n", i, namestr(&newerrorstr, i - E__USERDEF));
    }
}

/*
 * malloced_putenv - Keep track of malloced environment variable storage
 *
 * given:
 *      str     a malloced string which will be given to putenv
 *
 * returns:
 *      putenv() return value
 *
 * NOTE: The caller MUST pass a string that the caller has previously malloced.
 */
static int
malloced_putenv(char *str)
{
    char *value;          /* location of the value part of the str argument */
    char *old_val;        /* previously stored (or inherited) env value */
    int found_cnt;        /* number of active env_pool entries found */
    struct env_pool *new; /* new e_pool */
    int i;

    /*
     * firewall
     */
    if (str == NULL) {
        math_error("malloced_putenv given a NULL pointer!!");
        not_reached();
    }
    if (str[0] == '=') {
        math_error("malloced_putenv = is first character in string!!");
        not_reached();
    }

    /*
     * determine the place where getenv would return
     */
    value = strchr(str, '=');
    if (value == NULL) {
        math_error("malloced_putenv = not found in string!!");
        not_reached();
    }
    ++value;

    /*
     * lookup for an existing environment value
     */
    *(value - 1) = '\0';
    old_val = getenv(str);
    *(value - 1) = '=';

    /*
     * If we have the value in our environment, look for a
     * previously malloced string and free it
     */
    if (old_val != NULL && env_pool_cnt > 0) {
        for (i = 0, found_cnt = 0; i < env_pool_max && found_cnt < env_pool_cnt; ++i) {

            /* skip an unused entry */
            if (e_pool[i].getenv == NULL) {
                continue;
            }
            ++found_cnt;

            /* look for the 1st match */
            if (e_pool[i].getenv == value) {

                /* found match, free the storage */
                if (e_pool[i].putenv != NULL) {
                    free(e_pool[i].putenv);
                }
                e_pool[i].getenv = NULL;
                --env_pool_cnt;
                break;
            }
        }
    }

    /*
     * ensure that we have room in the e_pool
     */
    if (env_pool_max == 0) {

        /* allocate an initial pool (with one extra guard value) */
        new = (struct env_pool *)calloc(ENV_POOL_CHUNK + 1, sizeof(struct env_pool));
        if (new == NULL) {
            math_error("malloced_putenv malloc failed");
            not_reached();
        }
        e_pool = new;
        env_pool_max = ENV_POOL_CHUNK;
        for (i = 0; i <= ENV_POOL_CHUNK; ++i) {
            e_pool[i].getenv = NULL;
        }

    } else if (env_pool_cnt >= env_pool_max) {

        /* expand the current pool (with one extra guard value) */
        new = (struct env_pool *)realloc(e_pool, (env_pool_max + ENV_POOL_CHUNK + 1) * sizeof(struct env_pool));
        if (new == NULL) {
            math_error("malloced_putenv realloc failed");
            not_reached();
        }
        e_pool = new;
        for (i = env_pool_max; i <= env_pool_max + ENV_POOL_CHUNK; ++i) {
            e_pool[i].getenv = NULL;
        }
        env_pool_max += ENV_POOL_CHUNK;
    }

    /*
     * store our data into the first e_pool entry
     */
    for (i = 0; i < env_pool_max; ++i) {

        /* skip used entries */
        if (e_pool[i].getenv != NULL) {
            continue;
        }

        /* store in this free entry and stop looping */
        e_pool[i].getenv = value;
        e_pool[i].putenv = str;
        ++env_pool_cnt;
        break;
    }
    if (i >= env_pool_max) {
        math_error("malloced_putenv missed unused entry!!");
        not_reached();
    }

    /*
     * finally, do the putenv action
     */
    return putenv(str);
}

#endif
