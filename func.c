/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Built-in functions implemented here
 */


#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

#if defined(FUNCLIST)

#define CONST /* disabled for FUNCLIST in case NATIVE_CC doesn't have it */

#else /* FUNCLIST */

#include "calc_errno.h"

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_stdlib.h"
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#include "have_times.h"
#if defined(HAVE_TIME_H)
#include <time.h>
#endif
#if defined(HAVE_TIMES_H)
#include <times.h>
#endif
#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#if defined(HAVE_SYS_TIMES_H)
#include <sys/times.h>
#endif

#include "have_const.h"
#include "calc.h"
#include "calcerr.h"
#include "opcodes.h"
#include "token.h"
#include "func.h"
#include "string.h"
#include "symbol.h"
#include "prime.h"
#include "file.h"
#include "zrand.h"
#include "zrandom.h"
#include "custom.h"

#if defined(CUSTOM)
# define E_CUSTOM_ERROR E_NO_C_ARG
#else
# define E_CUSTOM_ERROR E_NO_CUSTOM
#endif


/*
 * forward declarations
 */
static NUMBER *base_value(long mode);
static int strscan(char *s, int count, VALUE **vals);
static int filescan(FILEID id, int count, VALUE **vals);
static VALUE f_eval(VALUE *vp);
static VALUE f_fsize(VALUE *vp);



/*
 * external declarations
 */
extern char cmdbuf[];				/* command line expression */
extern CONST char *error_table[E__COUNT+2];	/* calc coded error messages */
extern void matrandperm(MATRIX *M);
extern void listrandperm(LIST *lp);
extern int idungetc(FILEID id, int ch);

extern int stoponerror;


/*
 * if HZ & CLK_TCK are not defined, pick typical values, hope for the best
 */
#if !defined(HZ)
#  define HZ 60
#endif
#if !defined(CLK_TCK)
# undef CLK_TCK
# define CLK_TCK HZ
#endif


/*
 * user-defined error strings
 */
static short nexterrnum = E_USERDEF;
static STRINGHEAD newerrorstr;

#endif /* !FUNCLIST */


/*
 * arg count definitons
 */
#define IN 100		/* maximum number of arguments */
#define	FE 0x01		/* flag to indicate default epsilon argument */
#define	FA 0x02		/* preserve addresses of variables */


/*
 * builtins - List of primitive built-in functions
 */
struct builtin {
	char *b_name;		/* name of built-in function */
	short b_minargs;	/* minimum number of arguments */
	short b_maxargs;	/* maximum number of arguments */
	short b_flags;		/* special handling flags */
	short b_opcode;		/* opcode which makes the call quick */
	NUMBER *(*b_numfunc)();	/* routine to calculate numeric function */
	VALUE (*b_valfunc)();	/* routine to calculate general values */
	char *b_desc;		/* description of function */
};


#if !defined(FUNCLIST)

static VALUE
f_eval(VALUE *vp)
{
	FUNC	*oldfunc;
	FUNC	*newfunc;
	VALUE	result;
	char	*cp;

	if (vp->v_type != V_STR)
		return error_value(E_EVAL2);
	cp = vp->v_str->s_str;
	switch (openstring(cp)) {
		case -2:
			return error_value(E_EVAL3);
		case -1:
			return error_value(E_EVAL4);
	}
	oldfunc = curfunc;
	enterfilescope();
	if (evaluate(TRUE)) {
		exitfilescope();
		freevalue(stack--);
		newfunc = curfunc;
		curfunc = oldfunc;
		result = newfunc->f_savedvalue;
		newfunc->f_savedvalue.v_type = V_NULL;
		freenumbers(newfunc);
		if (newfunc != oldfunc)
			free(newfunc);
		return result;
	}
	exitfilescope();
	newfunc = curfunc;
	curfunc = oldfunc;
	freevalue(&newfunc->f_savedvalue);
	newfunc->f_savedvalue.v_type = V_NULL;
	freenumbers(newfunc);
	if (newfunc != oldfunc)
		free(newfunc);
	return error_value(E_EVAL);
}


static VALUE
f_prompt(VALUE *vp)
{
	VALUE result;
	char *cp;
	char *newcp;
	unsigned int len;

	result.v_type = V_STR;
	if (inputisterminal()) {
		printvalue(vp, PRINT_SHORT);
		math_flush();
	}
	cp = nextline();
	if (cp == NULL) {
		math_error("End of file while prompting");
		/*NOTREACHED*/
	}
	if (*cp == '\0') {
		result.v_str = slink(&_nullstring_);
		return result;
	}
	len = strlen(cp);
	newcp = (char *) malloc(len + 1);
	if (newcp == NULL) {
		math_error("Cannot allocate string");
		/*NOTREACHED*/
	}
	strcpy(newcp, cp);
	result.v_str = makestring(newcp);
	result.v_type = V_STR;
	return result;
}


static VALUE
f_display(int count, VALUE **vals)
{
	long oldvalue;
	VALUE res;

	oldvalue = conf->outdigits;

	if (count > 0) {
		if (vals[0]->v_type != V_NUM || qisfrac(vals[0]->v_num) ||
			qisneg(vals[0]->v_num) || zge31b(vals[0]->v_num->num))
			fprintf(stderr,
				 "Out-of-range arg for display ignored\n");
		else
			conf->outdigits = qtoi(vals[0]->v_num);
	}
	res.v_type = V_NUM;
	res.v_num = itoq(oldvalue);
	return res;
}


/*ARGSUSED*/
static VALUE
f_null(int count, VALUE **vals)
{
	VALUE res;

	res.v_type = 0;
	return res;
}


static VALUE
f_str(VALUE *vp)
{
	VALUE result;
	char *cp;

	result.v_type = V_STR;
	switch (vp->v_type) {
		case V_STR:
			result.v_str = stringcopy(vp->v_str);
			break;
		case V_NULL:
			result.v_str = slink(&_nullstring_);
			break;
		case V_OCTET:
			result.v_str = charstring(*vp->v_octet);
			break;
		case V_NUM:
			math_divertio();
			qprintnum(vp->v_num, MODE_DEFAULT);
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
f_name(VALUE *vp)
{
	VALUE result;
	char *cp;
	char *name;

	result.v_type = V_STR;
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
	result.v_type = V_STR;
	return result;
}



static VALUE
f_poly(int count, VALUE **vals)
{
	VALUE *x;
	VALUE result, tmp;
	LIST *clist, *lp;

	if (vals[0]->v_type == V_LIST) {
		clist = vals[0]->v_list;
		lp = listalloc();
		while (--count > 0) {
			if ((*++vals)->v_type == V_LIST)
				insertitems(lp, (*vals)->v_list);
			else
				insertlistlast(lp, *vals);
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
	res = itoq((long) !qdivides(tmp, val3));
	qfree(tmp);
	return res;
}


static NUMBER *
f_isrel(NUMBER *val1, NUMBER *val2)
{
	if (qisfrac(val1) || qisfrac(val2)) {
		math_error("Non-integer for isrel");
		/*NOTREACHED*/
	}
	return itoq((long) zrelprime(val1->num, val2->num));
}


static NUMBER *
f_issquare(NUMBER *vp)
{
	return itoq((long) qissquare(vp));
}


static NUMBER *
f_isprime(int count, NUMBER **vals)
{
	NUMBER *err;		/* error return, NULL => use math_error */

	/* determine the way we report problems */
	if (count == 2) {
		if (qisfrac(vals[1])) {
			math_error("2nd isprime arg must be an integer");
			/*NOTREACHED*/
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
		/*NOTREACHED*/
	}

	/* test the integer */
	switch (zisprime(vals[0]->num)) {
	case 0: return qlink(&_qzero_);
	case 1: return qlink(&_qone_);
	}

	/* error return */
	if (!err) {
		math_error("isprime argument is an odd value > 2^32");
		/*NOTREACHED*/
	}
	return qlink(err);
}


static NUMBER *
f_nprime(int count, NUMBER **vals)
{
	NUMBER *err;		/* error return, NULL => use math_error */
	FULL nxt_prime;		/* next prime or 0 */

	/* determine the way we report problems */
	if (count == 2) {
		if (qisfrac(vals[1])) {
			math_error("2nd nprime arg must be an integer");
			/*NOTREACHED*/
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
		math_error("non-integral arg 1 for builtin function nprime");
		/*NOTREACHED*/
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
		math_error("nprime arg 1 is >= 2^32");
		/*NOTREACHED*/
	}
	return qlink(err);
}


static NUMBER *
f_pprime(int count, NUMBER **vals)
{
	NUMBER *err;		/* error return, NULL => use math_error */
	FULL prev_prime;	/* previous prime or 0 */

	/* determine the way we report problems */
	if (count == 2) {
		if (qisfrac(vals[1])) {
			math_error("2nd pprime arg must be an integer");
			/*NOTREACHED*/
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
		math_error("non-integral arg 1 for builtin function pprime");
		/*NOTREACHED*/
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
			math_error("pprime arg 1 is <= 2");
			/*NOTREACHED*/
		} else {
			math_error("pprime arg 1 is >= 2^32");
			/*NOTREACHED*/
		}
	}
	return qlink(err);
}


static NUMBER *
f_factor(int count, NUMBER **vals)
{
	NUMBER *err;	/* error return, NULL => use math_error */
	ZVALUE limit;	/* highest prime factor in search */
	ZVALUE n;	/* number to factor */
	NUMBER *factor;	/* the prime factor found */
	int res;	/* -1 => error, 0 => not found, 1 => factor found */

	/*
	 * parse args
	 */
	if (count == 3) {
		if (qisfrac(vals[2])) {
			math_error("3rd factor arg must be an integer");
			/*NOTREACHED*/
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
			/*NOTREACHED*/
		}
		limit = vals[1]->num;
	} else {
		/* default limit is 2^32-1 */
		utoz((FULL)0xffffffff, &limit);
	}
	if (qisfrac(vals[0])) {
		if (count < 2)
			zfree(limit);
		if (err) {
			return qlink(err);
		}
		math_error("non-integral arg 1 for builtin pfactor");
		/*NOTREACHED*/
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
		/*NOTREACHED*/
	} else if (res == 0) {
		if (count < 2)
			zfree(limit);
		/* no factor found - qalloc set factor to 1, return 1 */
		return factor;
	}

	/*
	 * return the factor found
	 */
	if (count < 2)
		zfree(limit);
	return factor;
}


static NUMBER *
f_pix(int count, NUMBER **vals)
{
	NUMBER *err;		/* error return, NULL => use math_error */
	long value;		/* primes <= x, or 0 ==> error */

	/* determine the way we report problems */
	if (count == 2) {
		if (qisfrac(vals[1])) {
			math_error("2nd pix arg must be an integer");
			/*NOTREACHED*/
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
		/*NOTREACHED*/
	}

	/* determine the number of primes <= x */
	value = zpix(vals[0]->num);
	if (value >= 0) {
		return utoq(value);
	}

	/* error return */
	if (!err) {
		math_error("pix arg 1 is >= 2^32");
		/*NOTREACHED*/
	}
	return qlink(err);
}


static NUMBER *
f_prevcand(int count, NUMBER **vals)
{
	ZVALUE zmodulus;
	ZVALUE zresidue;
	ZVALUE zskip;
	ZVALUE *zcount = NULL;		/* ptest trial count */
	ZVALUE tmp;
	NUMBER *ans;			/* candidate for primality */

	zmodulus = _one_;
	zresidue = _zero_;
	zskip = _one_;
	/*
	 * check on the number of args passed and that args passed are ints
	 */
	switch (count) {
	case 5:
		if (!qisint(vals[4])) {
			math_error( "prevcand 5th arg must both be integer");
			/*NOTREACHED*/
		}
		zmodulus = vals[4]->num;
		/*FALLTHRU*/
	case 4:
		if (!qisint(vals[3])) {
			math_error( "prevcand 4th arg must both be integer");
			/*NOTREACHED*/
		}
		zresidue = vals[3]->num;
		/*FALLTHRU*/
	case 3:
		if (!qisint(vals[2])) {
			math_error(
		    "prevcand skip arg (3rd) must be an integer or omitted");
			/*NOTREACHED*/
		}
		zskip = vals[2]->num;
		/*FALLTHRU*/
	case 2:
		if (!qisint(vals[1])) {
			math_error(
		    "prevcand count arg (2nd) must be an integer or omitted");
			/*NOTREACHED*/
		}
		zcount = &vals[1]->num;
		/*FALLTHRU*/
	case 1:
		if (!qisint(vals[0])) {
			math_error(
		    "prevcand search arg (1st) must be an integer");
			/*NOTREACHED*/
		}
		break;
	default:
		math_error("invalid number of args passed to prevcand");
		/*NOTREACHED*/
	}

	if (zcount == NULL) {
		count = 1;	/* default is 1 ptest */
	} else {
		if (zge24b(*zcount)) {
			math_error("prevcand count arg (2nd) must be < 2^24");
			/*NOTREACHED*/
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
	ZVALUE *zcount = NULL;		/* ptest trial count */
	ZVALUE tmp;
	NUMBER *ans;			/* candidate for primality */

	zmodulus = _one_;
	zresidue = _zero_;
	zskip = _one_;
	/*
	 * check on the number of args passed and that args passed are ints
	 */
	switch (count) {
	case 5:
		if (!qisint(vals[4])) {
			math_error(
		    "nextcand 5th args must be integer");
			/*NOTREACHED*/
		}
		zmodulus = vals[4]->num;
		/*FALLTHRU*/
	case 4:
		if (!qisint(vals[3])) {
			math_error(
		    "nextcand 5th args must be integer");
			/*NOTREACHED*/
		}
		zresidue = vals[3]->num;
		/*FALLTHRU*/
	case 3:
		if (!qisint(vals[2])) {
			math_error(
		    "nextcand skip arg (3rd) must be an integer or omitted");
			/*NOTREACHED*/
		}
		zskip = vals[2]->num;
		/*FALLTHRU*/
	case 2:
		if (!qisint(vals[1])) {
			math_error(
		    "nextcand count arg (2nd) must be an integer or omitted");
			/*NOTREACHED*/
		}
		zcount = &vals[1]->num;
		/*FALLTHRU*/
	case 1:
		if (!qisint(vals[0])) {
			math_error(
		    "nextcand search arg (1st) must be an integer");
			/*NOTREACHED*/
		}
		break;
	default:
		math_error("invalid number of args passed to nextcand");
		/*NOTREACHED*/
	}

	/*
	 * check ranges on integers passed
	 */
	if (zcount == NULL) {
		count = 1;	/* default is 1 ptest */
	} else {
		if (zge24b(*zcount)) {
			math_error("prevcand count arg (2nd) must be < 2^24");
			/*NOTREACHED*/
		}
		count = ztoi(*zcount);
	}

	/*
	 * find the candidate
	 */
	if  (znextcand(vals[0]->num, count, zskip, zresidue, zmodulus, &tmp)) {
		ans = qalloc();
		ans->num = tmp;
		return ans;
	}
	return qlink(&_qzero_);;
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
	case 0:			/* rand() == rand(2^64) */
		/* generate an a55 random number */
		ans = qalloc();
		zrand(SBITS, &ans->num);
		break;

	case 1:			/* rand(limit) */
		if (!qisint(vals[0])) {
			math_error("rand limit must be an integer");
			/*NOTREACHED*/
		}
		if (zislezero(vals[0]->num)) {
			math_error("rand limit must > 0");
			/*NOTREACHED*/
		}
		ans = qalloc();
		zrandrange(_zero_, vals[0]->num, &ans->num);
		break;

	case 2:			/* rand(low, limit) */
		/* firewall */
		if (!qisint(vals[0]) || !qisint(vals[1])) {
			math_error("rand range must be integers");
			/*NOTREACHED*/
		}
		ans = qalloc();
		zrandrange(vals[0]->num, vals[1]->num, &ans->num);
		break;

	default:
		math_error("invalid number of args passed to rand");
		/*NOTREACHED*/
		return NULL;
	}

	/* return the a55 random number */
	return ans;
}


static NUMBER *
f_randbit(int count, NUMBER **vals)
{
	NUMBER *ans;
	ZVALUE ztmp;
	long cnt;		/* bits needed or skipped */

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
		/*NOTREACHED*/
	}
	if (zge31b(vals[0]->num)) {
		math_error("huge rand bit count");
		/*NOTREACHED*/
	}

	/*
	 * generate an a55 random number or skip random bits
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
	 * return the a55 random number
	 */
	return ans;
}


static VALUE
f_srand(int count, VALUE **vals)
{
	VALUE result;

	/* parse args */
	switch (count) {
	case 0:
		/* get the current a55 state */
		result.v_rand = zsrand(NULL, NULL);
		break;

	case 1:
		switch (vals[0]->v_type) {
		case V_NUM:		/* srand(seed) */
			/* seed a55 and return previous state */
			if (!qisint(vals[0]->v_num)) {
				math_error(
				  "srand number seed must be an integer");
			        /*NOTREACHED*/
			}
			result.v_rand = zsrand(&vals[0]->v_num->num, NULL);
			break;

		case V_RAND:		/* srand(state) */
			/* set a55 state and return previous state */
			result.v_rand = zsetrand(vals[0]->v_rand);
			break;

		case V_MAT:
			/* load additive 55 table and return previous state */
			result.v_rand = zsrand(NULL, vals[0]->v_mat);
			break;

		default:
			math_error("illegal type of arg passsed to srand()");
			/*NOTREACHED*/
			break;
		}
		break;

	default:
		math_error("bad arg count to srand()");
		/*NOTREACHED*/
		break;
	}

	/* return the current state */
	result.v_type = V_RAND;
	return result;
}


static NUMBER *
f_random(int count, NUMBER **vals)
{
	NUMBER *ans;

	/* parse args */
	switch (count) {
	case 0:			/* random() == random(2^64) */
		/* generate a Blum-Blum-Shub random number */
		ans = qalloc();
		zrandom(SBITS, &ans->num);
		break;

	case 1:			/* random(limit) */
		if (!qisint(vals[0])) {
			math_error("random limit must be an integer");
			/*NOTREACHED*/
		}
		if (zislezero(vals[0]->num)) {
			math_error("random limit must > 0");
			/*NOTREACHED*/
		}
		ans = qalloc();
		zrandomrange(_zero_, vals[0]->num, &ans->num);
		break;

	case 2:			/* random(low, limit) */
		/* firewall */
		if (!qisint(vals[0]) || !qisint(vals[1])) {
			math_error("random range must be integers");
			/*NOTREACHED*/
		}
		ans = qalloc();
		zrandomrange(vals[0]->num, vals[1]->num, &ans->num);
		break;

	default:
		math_error("invalid number of args passed to random");
		/*NOTREACHED*/
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
	long cnt;		/* bits needed or skipped */

	/* parse args */
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
		/*NOTREACHED*/
	}
	if (zge31b(vals[0]->num)) {
		math_error("huge random bit count");
		/*NOTREACHED*/
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

	/* parse args */
	switch (count) {
	case 0:		/* srandom() */
		/* get the current random state */
		result.v_random = zsetrandom(NULL);
		break;

	case 1:		/* srandom(seed) or srandom(state) */
		switch (vals[0]->v_type) {
		case V_NUM:		/* srand(seed) */
			/* seed Blum and return previous state */
			if (!qisint(vals[0]->v_num)) {
				math_error(
				  "srandom number seed must be an integer");
			        /*NOTREACHED*/
			}
			result.v_random = zsrandom1(vals[0]->v_num->num, TRUE);
			break;

		case V_RANDOM:		/* srandom(state) */
			/* set a55 state and return previous state */
			result.v_random = zsetrandom(vals[0]->v_random);
			break;

		default:
			math_error("illegal type of arg passsed to srandom()");
			/*NOTREACHED*/
			break;
		}
		break;

	case 2:		/* srandom(seed, newn) */
		if (vals[0]->v_type != V_NUM || !qisint(vals[0]->v_num)) {
			math_error("srandom seed must be an integer");
			/*NOTREACHED*/
		}
		if (vals[1]->v_type != V_NUM || !qisint(vals[1]->v_num)) {
			math_error("srandom Blum modulus must be an integer");
			/*NOTREACHED*/
		}
		result.v_random = zsrandom2(vals[0]->v_num->num,
					    vals[1]->v_num->num);
		break;

	case 4:		/* srandom(seed, ip, iq, trials) */
		if (vals[0]->v_type != V_NUM || !qisint(vals[0]->v_num)) {
			math_error("srandom seed must be an integer");
			/*NOTREACHED*/
		}
		if (vals[1]->v_type != V_NUM || !qisint(vals[1]->v_num)) {
			math_error("srandom 2nd arg must be an integer");
			/*NOTREACHED*/
		}
		if (vals[2]->v_type != V_NUM || !qisint(vals[2]->v_num)) {
			math_error("srandom 3rd arg must be an integer");
			/*NOTREACHED*/
		}
		if (vals[3]->v_type != V_NUM || !qisint(vals[3]->v_num)) {
			math_error("srandom 4th arg must be an integer");
			/*NOTREACHED*/
		}
		if (zge24b(vals[3]->v_num->num)) {
			math_error("srandom trials count is excessive");
			/*NOTREACHED*/
		}
		result.v_random = zsrandom4(vals[0]->v_num->num,
					    vals[1]->v_num->num,
					    vals[2]->v_num->num,
					    ztoi(vals[3]->v_num->num));
		break;

	default:
		math_error("bad arg count to srandom()");
		/*NOTREACHED*/
		break;
	}

	/* return the current state */
	result.v_type = V_RANDOM;
	return result;
}


static NUMBER *
f_primetest(int count, NUMBER **vals)
{
	/* parse args */
	switch (count) {
	case 1: return itoq((long) qprimetest(vals[0], &_qone_, &_qone_));
	case 2: return itoq((long) qprimetest(vals[0], vals[1], &_qone_));
	default: return itoq((long) qprimetest(vals[0], vals[1], vals[2]));
	}
}


static VALUE
f_setbit(int count, VALUE **vals)
{
	BOOL r;
	long index;
	VALUE result;

	r = (count == 3) ? testvalue(vals[2]) : 1;

	if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num))
		return error_value(E_SETBIT1);
	if (zge31b(vals[1]->v_num->num))
		return error_value(E_SETBIT2);
	if (vals[0]->v_type != V_STR)
		return error_value(E_SETBIT3);
	index = qtoi(vals[1]->v_num);
	if (stringsetbit(vals[0]->v_str, index, r))
		return error_value(E_SETBIT2);
	result.v_type = V_NULL;
	return result;
}


static NUMBER *
f_digit(NUMBER *val1, NUMBER *val2)
{
	if (qisfrac(val2)) {
		math_error("Non-integral digit position");
		/*NOTREACHED*/
	}
	if (qiszero(val1) || (qisint(val1) && qisneg(val2)))
		return qlink(&_qzero_);
	if (zge31b(val2->num)) {
		if (qisneg(val2)) {
			math_error("Very large digit position");
			/*NOTREACHED*/
		}
		return qlink(&_qzero_);
	}
	return itoq((long) qdigit(val1, qtoi(val2)));
}


static NUMBER *
f_digits(NUMBER *val)
{
	return itoq((long) qdigits(val));
}


static NUMBER *
f_places(NUMBER *val)
{
	return itoq((long) qplaces(val));
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
		return itoq(zpopcnt(vals[0]->num, bitval) +
			    zpopcnt(vals[0]->den, bitval));
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
	for (i = 1; i < count; i++) {
		if (vals[i]->v_type != type)
			return error_value(E_XOR1);
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
			return error_value(E_XOR2);
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

	min.v_type = V_NULL;
	term.v_type = V_NULL;

	for (ep = lp->l_first; ep; ep = ep->e_next) {
		vp = &ep->e_value;
		switch(vp->v_type) {
			case V_LIST:
				term = minlistitems(vp->v_list);
				break;
			case V_OBJ:
				term = objcall(OBJ_MIN, vp,
					       NULL_VALUE, NULL_VALUE);
				break;
			default:
				copyvalue(vp, &term);
		}
		if (min.v_type == V_NULL) {
			min = term;
			continue;
		}
		if (term.v_type == V_NULL)
			continue;
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
		}
		else
			freevalue(&term);
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

	max.v_type = V_NULL;
	term.v_type = V_NULL;

	for (ep = lp->l_first; ep; ep = ep->e_next) {
		vp = &ep->e_value;
		switch(vp->v_type) {
			case V_LIST:
				term = maxlistitems(vp->v_list);
				break;
			case V_OBJ:
				term = objcall(OBJ_MAX, vp,
					       NULL_VALUE, NULL_VALUE);
				break;
			default:
				copyvalue(vp, &term);
		}
		if (max.v_type == V_NULL) {
			max = term;
			continue;
		}
		if (term.v_type == V_NULL)
			continue;
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
		}
		else
			freevalue(&term);
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

	min.v_type = V_NULL;
	term.v_type = V_NULL;

	while (count-- > 0) {
		vp = *vals++;
		switch(vp->v_type) {
			case V_LIST:
				term = minlistitems(vp->v_list);
				break;
			case V_OBJ:
				term = objcall(OBJ_MIN, vp,
					      NULL_VALUE, NULL_VALUE);
				break;
			default:
				copyvalue(vp, &term);
		}
		if (min.v_type == V_NULL) {
			min = term;
			continue;
		}
		if (term.v_type == V_NULL)
			continue;
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
		}
		else
			freevalue(&term);
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

	max.v_type = V_NULL;
	term.v_type = V_NULL;

	while (count-- > 0) {
		vp = *vals++;
		switch(vp->v_type) {
			case V_LIST:
				term = maxlistitems(vp->v_list);
				break;
			case V_OBJ:
				term = objcall(OBJ_MAX, vp,
					       NULL_VALUE, NULL_VALUE);
				break;
			default:
				copyvalue(vp, &term);
		}
		if (max.v_type == V_NULL) {
			max = term;
			continue;
		}
		if (term.v_type == V_NULL)
			continue;
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
		}
		else
			freevalue(&term);
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
		if (qiszero(val))
			break;
	}
	return val;
}


static VALUE
f_hash(int count, VALUE **vals)
{
	QCKHASH hash;
	long lhash;
	VALUE result;

	hash = (QCKHASH)0;
	while (count-- > 0)
		hash = hashvalue(*vals++, hash);
	lhash = (long) hash;
	if (lhash < 0)
		lhash = -lhash;
	result.v_num = itoq(lhash);
	result.v_type = V_NUM;
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

	sum.v_type = V_NULL;
	term.v_type = V_NULL;

	for (ep = lp->l_first; ep; ep = ep->e_next) {
		vp = &ep->e_value;
		switch(vp->v_type) {
			case V_LIST:
				term = sumlistitems(vp->v_list);
				break;
			case V_OBJ:
				term = objcall(OBJ_SUM, vp,
					       NULL_VALUE, NULL_VALUE);
				break;
			default:
				copyvalue(vp, &term);
		}
		if (sum.v_type == V_NULL) {
			sum = term;
			continue;
		}
		if (term.v_type == V_NULL)
			continue;
		addvalue(&sum, &term, &tmp);
		freevalue(&sum);
		freevalue(&term);
		sum = tmp;
		if (sum.v_type < 0)
			break;
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

	sum.v_type = V_NULL;
	term.v_type = V_NULL;

	while (count-- > 0) {
		vp = *vals++;
		switch(vp->v_type) {
			case V_LIST:
				term = sumlistitems(vp->v_list);
				break;
			case V_OBJ:
				term = objcall(OBJ_SUM, vp,
					      NULL_VALUE, NULL_VALUE);
				break;
			default:
				copyvalue(vp, &term);
		}
		if (sum.v_type == V_NULL) {
			sum = term;
			continue;
		}
		if (term.v_type == V_NULL)
			continue;
		if (term.v_type < 0) {
			freevalue(&sum);
			return term;
		}
		addvalue(&sum, &term, &tmp);
		freevalue(&term);
		freevalue(&sum);
		sum = tmp;
		if (sum.v_type < 0)
			return sum;
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

	sum.v_type = V_NULL;
	n = 0;
	while (count-- > 0) {
		if ((*vals)->v_type == V_LIST) {
			addlistitems((*vals)->v_list, &sum);
			n += countlistitems((*vals++)->v_list);
		}
		else {
			addvalue(&sum, *vals++, &tmp);
			freevalue(&sum);
			sum = tmp;
			n++;
		}
		if (sum.v_type < 0)
			return sum;
	}
	if (n < 2)
		return sum;
	div.v_num = itoq(n);
	div.v_type = V_NUM;
	divvalue(&sum, &div, &tmp);
	freevalue(&sum);
	qfree(div.v_num);
	return tmp;
}


static VALUE
f_fact(VALUE *vp)
{
	VALUE res;

	if (vp->v_type == V_OBJ) {
		return objcall(OBJ_FACT, vp, NULL_VALUE, NULL_VALUE);
	}
	if (vp->v_type != V_NUM) {
		math_error("Non-real argument for fact()");
		/*NOTREACHED*/
	}
	res.v_type = V_NUM;
	res.v_num = qfact(vp->v_num);
	return res;
}


static VALUE
f_hmean(int count, VALUE **vals)
{
	VALUE sum, tmp1, tmp2;
	long n = 0;

	sum.v_type = V_NULL;
	while (count-- > 0) {
		if ((*vals)->v_type == V_LIST) {
			addlistinv((*vals)->v_list, &sum);
			n += countlistitems((*vals++)->v_list);
		}
		else {
			invertvalue(*vals++, &tmp1);
			addvalue(&sum, &tmp1, &tmp2);
			freevalue(&tmp1);
			freevalue(&sum);
			sum = tmp2;
			n++;
		}
	}
	if (n == 0)
		return sum;
	tmp1.v_type = V_NUM;
	tmp1.v_num = itoq(n);
	divvalue(&tmp1, &sum, &tmp2);
	qfree(tmp1.v_num);
	freevalue(&sum);
	return tmp2;
}


static NUMBER *
f_hnrmod(NUMBER *val1, NUMBER *val2, NUMBER *val3, NUMBER *val4)
{
	ZVALUE answer;			/* v mod h*2^n+r */
	NUMBER *res;			/* v mod h*2^n+r */

	/*
	 * firewall
	 */
	if (qisfrac(val1)) {
		math_error("1st arg of hnrmod (v) must be an integer");
		/*NOTREACHED*/
	}
	if (qisfrac(val2) || qisneg(val2) || qiszero(val2)) {
		math_error("2nd arg of hnrmod (h) must be an integer > 0");
		/*NOTREACHED*/
	}
	if (qisfrac(val3) || qisneg(val3) || qiszero(val3)) {
		math_error("3rd arg of hnrmod (n) must be an integer > 0");
		/*NOTREACHED*/
	}
	if (qisfrac(val4) || !zisabsleone(val4->num)) {
		math_error("4th arg of hnrmod (r) must be -1, 0 or 1");
		/*NOTREACHED*/
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


static VALUE
f_ssq(int count, VALUE **vals)
{
	VALUE result, tmp1, tmp2;

	squarevalue(*vals++, &result);
	while (--count > 0) {
		squarevalue(*vals++, &tmp1);
		addvalue(&tmp1, &result, &tmp2);
		freevalue(&tmp1);
		freevalue(&result);
		result = tmp2;
	}
	return result;
}


static NUMBER *
f_ismult(NUMBER *val1, NUMBER *val2)
{
	return itoq((long) qdivides(val1, val2));
}


static NUMBER *
f_meq(NUMBER *val1, NUMBER *val2, NUMBER *val3)
{
	NUMBER *tmp, *res;

	tmp = qsub(val1, val2);
	res = itoq((long) qdivides(tmp, val3));
	qfree(tmp);
	return res;
}


static VALUE
f_exp(int count, VALUE **vals)
{
	VALUE result;
	NUMBER *err;
	COMPLEX *c;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_EXP1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qexp(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			c = cexp(vals[0]->v_com, err);
			result.v_com = c;
			result.v_type = V_COM;
			if (cisreal(c)) {
				result.v_num = qlink(c->real);
				result.v_type = V_NUM;
				comfree(c);
			}
			break;
		default:
			return error_value(E_EXP2);
	}
	return result;
}


static VALUE
f_ln(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX ctmp, *c;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM)
			return error_value(E_LN1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (!qisneg(vals[0]->v_num) && !qiszero(vals[0]->v_num)) {
				result.v_num = qln(vals[0]->v_num, err);
				result.v_type = V_NUM;
				return result;
			}
			ctmp.real = vals[0]->v_num;
			ctmp.imag = &_qzero_;
			ctmp.links = 1;
			c = cln(&ctmp, err);
			break;
		case V_COM:
			c = cln(vals[0]->v_com, err);
			break;
		default:
			return error_value(E_LN2);
	}
	result.v_type = V_COM;
	result.v_com = c;
	if (cisreal(c)) {
		result.v_num = qlink(c->real);
		result.v_type = V_NUM;
		comfree(c);
	}
	return result;
}


static VALUE
f_cos(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *c;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_COS1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qcos(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			c = ccos(vals[0]->v_com, err);
			result.v_com = c;
			result.v_type = V_COM;
			if (cisreal(c)) {
				result.v_num = qlink(c->real);
				result.v_type = V_NUM;
				comfree(c);
			}
			break;
		default:
			return error_value(E_COS2);
	}
	return result;
}


static VALUE
f_sin(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *c;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_COS1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qsin(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			c = csin(vals[0]->v_com, err);
			result.v_com = c;
			result.v_type = V_COM;
			if (cisreal(c)) {
				result.v_num = qlink(c->real);
				result.v_type = V_NUM;
				comfree(c);
			}
			break;
		default:
			return error_value(E_COS2);
	}
	return result;
}


static VALUE
f_tan(int count, VALUE **vals)
{
	VALUE result;
	VALUE tmp1, tmp2;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_TAN1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qtan(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp1.v_type = V_COM;
			tmp1.v_com = csin(vals[0]->v_com, err);
			tmp2.v_type = V_COM;
			tmp2.v_com = ccos(vals[0]->v_com, err);
			divvalue(&tmp1, &tmp2, &result);
			comfree(tmp1.v_com);
			comfree(tmp2.v_com);
			break;
		default:
			return error_value(E_TAN2);
	}
	return result;
}

static VALUE
f_sec(int count, VALUE **vals)
{
	VALUE result;
	VALUE tmp;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_SEC1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qsec(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp.v_type = V_COM;
			tmp.v_com = ccos(vals[0]->v_com, err);
			invertvalue(&tmp, &result);
			comfree(tmp.v_com);
			break;
		default:
			return error_value(E_SEC2);
	}
	return result;
}


static VALUE
f_cot(int count, VALUE **vals)
{
	VALUE result;
	VALUE tmp1, tmp2;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_COT1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qiszero(vals[0]->v_num))
				return error_value(E_1OVER0);
			result.v_num = qcot(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp1.v_type = V_COM;
			tmp1.v_com = ccos(vals[0]->v_com, err);
			tmp2.v_type = V_COM;
			tmp2.v_com = csin(vals[0]->v_com, err);
			divvalue(&tmp1, &tmp2, &result);
			comfree(tmp1.v_com);
			comfree(tmp2.v_com);
			break;
		default:
			return error_value(E_COT2);
	}
	return result;
}


static VALUE
f_csc(int count, VALUE **vals)
{
	VALUE result;
	VALUE tmp;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_CSC1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qiszero(vals[0]->v_num))
				return error_value(E_1OVER0);
			result.v_num = qcsc(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp.v_type = V_COM;
			tmp.v_com = csin(vals[0]->v_com, err);
			invertvalue(&tmp, &result);
			comfree(tmp.v_com);
			break;
		default:
			return error_value(E_CSC2);
	}
	return result;
}

static VALUE
f_sinh(int count, VALUE **vals)
{
	VALUE result;
	NUMBER *err;
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_SINH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qsinh(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			result.v_com = csinh(vals[0]->v_com, err);
			result.v_type = V_COM;
			if (cisreal(result.v_com)) {
				q = qlink(result.v_com->real);
				comfree(result.v_com);
				result.v_num = q;
				result.v_type = V_NUM;
			}
			break;
		default:
			return error_value(E_SINH2);
	}
	return result;
}

static VALUE
f_cosh(int count, VALUE **vals)
{
	VALUE result;
	NUMBER *err;
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_COSH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qcosh(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			result.v_com = ccosh(vals[0]->v_com, err);
			result.v_type = V_COM;
			if (cisreal(result.v_com)) {
				q = qlink(result.v_com->real);
				comfree(result.v_com);
				result.v_num = q;
				result.v_type = V_NUM;
			}
			break;
		default:
			return error_value(E_COSH2);
	}
	return result;
}


static VALUE
f_tanh(int count, VALUE **vals)
{
	VALUE result;
	VALUE tmp1, tmp2;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_TANH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qtanh(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp1.v_type = V_COM;
			tmp1.v_com = csinh(vals[0]->v_com, err);
			tmp2.v_type = V_COM;
			tmp2.v_com = ccosh(vals[0]->v_com, err);
			divvalue(&tmp1, &tmp2, &result);
			comfree(tmp1.v_com);
			comfree(tmp2.v_com);
			break;
		default:
			return error_value(E_TANH2);
	}
	return result;
}


static VALUE
f_coth(int count, VALUE **vals)
{
	VALUE result;
	VALUE tmp1, tmp2;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_COTH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qiszero(vals[0]->v_num))
				return error_value(E_1OVER0);
			result.v_num = qcoth(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp1.v_type = V_COM;
			tmp1.v_com = ccosh(vals[0]->v_com, err);
			tmp2.v_type = V_COM;
			tmp2.v_com = csinh(vals[0]->v_com, err);
			divvalue(&tmp1, &tmp2, &result);
			comfree(tmp1.v_com);
			comfree(tmp2.v_com);
			break;
		default:
			return error_value(E_COTH2);
	}
	return result;
}


static VALUE
f_sech(int count, VALUE **vals)
{
	VALUE result;
	VALUE tmp;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_SECH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qsech(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp.v_type = V_COM;
			tmp.v_com = ccosh(vals[0]->v_com, err);
			invertvalue(&tmp, &result);
			comfree(tmp.v_com);
			break;
		default:
			return error_value(E_SECH2);
	}
	return result;
}


static VALUE
f_csch(int count, VALUE **vals)
{
	VALUE result;
	VALUE tmp;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_CSCH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qiszero(vals[0]->v_num))
				return error_value(E_1OVER0);
			result.v_num = qcsch(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp.v_type = V_COM;
			tmp.v_com = csinh(vals[0]->v_com, err);
			invertvalue(&tmp, &result);
			comfree(tmp.v_com);
			break;
		default:
			return error_value(E_CSCH2);
	}
	return result;
}


static VALUE
f_atan(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *tmp;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ATAN1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qatan(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp = catan(vals[0]->v_com, err);
			if (tmp == NULL)
				return error_value(E_LOGINF);
			result.v_type = V_COM;
			result.v_com = tmp;
			if (cisreal(tmp)) {
				result.v_num = qlink(tmp->real);
				result.v_type = V_NUM;
				comfree(tmp);
			}
			break;
		default:
			return error_value(E_ATAN2);
	}
	return result;
}


static VALUE
f_acot(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *tmp;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ACOT1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qacot(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp = cacot(vals[0]->v_com, err);
			if (tmp == NULL)
				return error_value(E_LOGINF);
			result.v_type = V_COM;
			result.v_com = tmp;
			if (cisreal(tmp)) {
				result.v_num = qlink(tmp->real);
				result.v_type = V_NUM;
				comfree(tmp);
			}
			break;
		default:
			return error_value(E_ACOT2);
	}
	return result;
}

static VALUE
f_asin(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *tmp;
	NUMBER *err;
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ASIN1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qasin(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_type = V_COM;
				result.v_com = casin(tmp, err);
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = casin(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ASIN2);
	}
	if (result.v_type == V_COM && cisreal(result.v_com)) {
		q = qlink(result.v_com->real);
		comfree(result.v_com);
		result.v_type = V_NUM;
		result.v_num = q;
	}
	return result;
}

static VALUE
f_acos(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *tmp;
	NUMBER *err;
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ACOS1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qacos(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_type = V_COM;
				result.v_com = cacos(tmp, err);
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = cacos(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ACOS2);
	}
	if (result.v_type == V_COM && cisreal(result.v_com)) {
		q = qlink(result.v_com->real);
		comfree(result.v_com);
		result.v_type = V_NUM;
		result.v_num = q;
	}
	return result;
}


static VALUE
f_asec(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *tmp;
	NUMBER *err;
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ASEC1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qiszero(vals[0]->v_num))
				return error_value(E_LOGINF);
			result.v_num = qasec(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_com = casec(tmp, err);
				result.v_type = V_COM;
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = casec(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ASEC2);
	}
	if (result.v_type == V_COM) {
		if (result.v_com == NULL)
			return error_value(E_LOGINF);
		if (cisreal(result.v_com)) {
			q = qlink(result.v_com->real);
			comfree(result.v_com);
			result.v_type = V_NUM;
			result.v_num = q;
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
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ACSC1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qiszero(vals[0]->v_num))
				return error_value(E_LOGINF);
			result.v_num = qacsc(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_com = cacsc(tmp, err);
				result.v_type = V_COM;
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = cacsc(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ACSC2);
	}
	if (result.v_type == V_COM) {
		if (result.v_com == NULL)
			return error_value(E_LOGINF);
		if (cisreal(result.v_com)) {
			q = qlink(result.v_com->real);
			comfree(result.v_com);
			result.v_type = V_NUM;
			result.v_num = q;
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

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ASINH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qasinh(vals[0]->v_num, err);
			result.v_type = V_NUM;
			break;
		case V_COM:
			tmp = casinh(vals[0]->v_com, err);
			result.v_type = V_COM;
			result.v_com = tmp;
			if (cisreal(tmp)) {
				result.v_num = qlink(tmp->real);
				result.v_type = V_NUM;
				comfree(tmp);
			}
			break;
		default:
			return error_value(E_ASINH2);
	}
	return result;
}


static VALUE
f_acosh(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *tmp;
	NUMBER *err;
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ACOSH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qacosh(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_com = cacosh(tmp, err);
				result.v_type = V_COM;
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = cacosh(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ACOSH2);
	}
	if (result.v_type == V_COM && cisreal(result.v_com)) {
		q = qlink(result.v_com->real);
		comfree(result.v_com);
		result.v_type = V_NUM;
		result.v_num = q;
	}
	return result;
}


static VALUE
f_atanh(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *tmp;
	NUMBER *err;
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ATANH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qatanh(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_com = catanh(tmp, err);
				result.v_type = V_COM;
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = catanh(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ATANH2);
	}
	if (result.v_type == V_COM) {
		if (result.v_com == NULL)
			return error_value(E_LOGINF);
		if (cisreal(result.v_com)) {
			q = qlink(result.v_com->real);
			comfree(result.v_com);
			result.v_type = V_NUM;
			result.v_num = q;
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
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ACOTH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			result.v_num = qacoth(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_com = cacoth(tmp, err);
				result.v_type = V_COM;
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = cacoth(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ACOTH2);
	}
	if (result.v_type == V_COM) {
		if (result.v_com == NULL)
			return error_value(E_LOGINF);
		if (cisreal(result.v_com)) {
			q = qlink(result.v_com->real);
			comfree(result.v_com);
			result.v_type = V_NUM;
			result.v_num = q;
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
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_SECH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qiszero(vals[0]->v_num))
				return error_value(E_LOGINF);
			result.v_num = qasech(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_com = casech(tmp, err);
				result.v_type = V_COM;
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = casech(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ASECH2);
	}
	if (result.v_type == V_COM) {
		if (result.v_com == NULL)
			return error_value(E_LOGINF);
		if (cisreal(result.v_com)) {
			q = qlink(result.v_com->real);
			comfree(result.v_com);
			result.v_type = V_NUM;
			result.v_num = q;
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
	NUMBER *q;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ACSCH1);
		err = vals[1]->v_num;
	}
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qiszero(vals[0]->v_num))
				return error_value(E_LOGINF);
			result.v_num = qacsch(vals[0]->v_num, err);
			result.v_type = V_NUM;
			if (result.v_num == NULL) {
				tmp = comalloc();
				qfree(tmp->real);
				tmp->real = qlink(vals[0]->v_num);
				result.v_com = cacsch(tmp, err);
				result.v_type = V_COM;
				comfree(tmp);
			}
			break;
		case V_COM:
			result.v_com = cacsch(vals[0]->v_com, err);
			result.v_type = V_COM;
			break;
		default:
			return error_value(E_ACSCH2);
	}
	if (result.v_type == V_COM) {
		if (result.v_com == NULL)
			return error_value(E_LOGINF);
		if (cisreal(result.v_com)) {
			q = qlink(result.v_com->real);
			comfree(result.v_com);
			result.v_type = V_NUM;
			result.v_num = q;
		}
	}
	return result;
}


static VALUE
f_gd(int count, VALUE **vals)
{
	VALUE result;
	NUMBER *err;
	NUMBER *q;
	COMPLEX *tmp;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_GD1);
		err = vals[1]->v_num;
	}
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
			result.v_com = cgd(tmp, err);
			comfree(tmp);
			break;
		case V_COM:
			result.v_com = cgd(vals[0]->v_com, err);
			break;
		default:
			return error_value(E_GD2);
	}
	if (result.v_com == NULL)
		return error_value(E_LOGINF);
	if (cisreal(result.v_com)) {
		q = qlink(result.v_com->real);
		comfree(result.v_com);
		result.v_num = q;
		result.v_type = V_NUM;
	}
	return result;
}


static VALUE
f_agd(int count, VALUE **vals)
{
	VALUE result;
	NUMBER *err;
	NUMBER *q;
	COMPLEX *tmp;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_AGD1);
		err = vals[1]->v_num;
	}
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
			result.v_com = cagd(tmp, err);
			comfree(tmp);
			break;
		case V_COM:
			result.v_com = cagd(vals[0]->v_com, err);
			break;
		default:
			return error_value(E_AGD2);
	}
	if (result.v_com == NULL)
			return error_value(E_LOGINF);
	if (cisreal(result.v_com)) {
		q = qlink(result.v_com->real);
		comfree(result.v_com);
		result.v_num = q;
		result.v_type = V_NUM;
	}
	return result;
}


static VALUE
f_arg(int count, VALUE **vals)
{
	VALUE result;
	COMPLEX *c;
	NUMBER *err;

	err = conf->epsilon;
	if (count == 2) {
		if (vals[1]->v_type != V_NUM || qiszero(vals[1]->v_num))
			return error_value(E_ARG1);
		err = vals[1]->v_num;
	}
	result.v_type = V_NUM;
	switch (vals[0]->v_type) {
		case V_NUM:
			if (qisneg(vals[0]->v_num))
				result.v_num = qpi(err);
			else
				result.v_num = qlink(&_qzero_);
			break;
		case V_COM:
			c = vals[0]->v_com;
			if (ciszero(c))
				result.v_num = qlink(&_qzero_);
			else
				result.v_num = qatan2(c->imag, c->real, err);
			break;
		default:
			return error_value(E_ARG2);
	}
	return result;
}


static NUMBER *
f_legtoleg(NUMBER *val1, NUMBER *val2)
{
	return qlegtoleg(val1, val2, FALSE);
}


static NUMBER *
f_trunc(int count, NUMBER **vals)
{
	NUMBER *val;

	val = &_qzero_;
	if (count == 2)
		val = vals[1];
	return qtrunc(*vals, val);
}


static VALUE
f_bround(int count, VALUE **vals)
{
	VALUE tmp1, tmp2, res;

	if (count > 2)
		tmp2 = *vals[2];
	else
		tmp2.v_type = V_NULL;
	if (count > 1)
		tmp1 = *vals[1];
	else
		tmp1.v_type = V_NULL;
	broundvalue(vals[0], &tmp1, &tmp2, &res);
	return res;
}


static VALUE
f_appr(int count, VALUE **vals)
{
	VALUE tmp1, tmp2, res;

	if (count > 2)
		copyvalue(vals[2], &tmp2);
	else
		tmp2.v_type = V_NULL;
	if (count > 1)
		copyvalue(vals[1], &tmp1);
	else
		tmp1.v_type = V_NULL;
	apprvalue(vals[0], &tmp1, &tmp2, &res);
	freevalue(&tmp1);
	freevalue(&tmp2);
	return res;
}

static VALUE
f_round(int count, VALUE **vals)
{
	VALUE tmp1, tmp2, res;

	if (count > 2)
		tmp2 = *vals[2];
	else
		tmp2.v_type = V_NULL;
	if (count > 1)
		tmp1 = *vals[1];
	else
		tmp1.v_type = V_NULL;
	roundvalue(vals[0], &tmp1, &tmp2, &res);
	return res;
}


static NUMBER *
f_btrunc(int count, NUMBER **vals)
{
	NUMBER *val;

	val = &_qzero_;
	if (count == 2)
		val = vals[1];
	return qbtrunc(*vals, val);
}


static VALUE
f_quo(int count, VALUE **vals)
{
	VALUE tmp, res;

	if (count > 2)
		tmp = *vals[2];
	else
		tmp.v_type = V_NULL;
	quovalue(vals[0], vals[1], &tmp, &res);
	return res;
}


static VALUE
f_mod(int count, VALUE **vals)
{
	VALUE tmp, res;

	if (count > 2)
		tmp = *vals[2];
	else
		tmp.v_type = V_NULL;
	modvalue(vals[0], vals[1], &tmp, &res);
	return res;
}


static VALUE
f_mmin(VALUE *v1, VALUE *v2)
{
	VALUE sixteen, res;

	sixteen.v_type = V_NUM;
	sixteen.v_num = itoq(16);
	modvalue(v1, v2, &sixteen, &res);
	qfree(sixteen.v_num);
	return res;
}


static NUMBER *
f_near(int count, NUMBER **vals)
{
	NUMBER *val;

	val = conf->epsilon;
	if (count == 3)
		val = vals[2];
	return itoq((long) qnear(vals[0], vals[1], val));
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
	NUMBER *q;

	R = (count > 2) ? qtoi(vals[2]) : conf->cfappr;
	q = (count > 1) ? vals[1] : conf->epsilon;

	return qcfappr(vals[0], q, R);
}


static VALUE
f_ceil(VALUE *val)
{
	VALUE tmp, res;

	tmp.v_type = V_NUM;
	tmp.v_num = &_qone_;
	apprvalue(val, &tmp, &tmp, &res);
	return res;
}


static VALUE
f_floor(VALUE *val)
{
	VALUE tmp1, tmp2, res;

	tmp1.v_type = V_NUM;
	tmp1.v_num = &_qone_;
	tmp2.v_type = V_NUM;
	tmp2.v_num = &_qzero_;
	apprvalue(val, &tmp1, &tmp2, &res);
	return res;
}


static VALUE
f_sqrt(int count, VALUE **vals)
{
	VALUE tmp1, tmp2, result;

	if (count > 2)
		tmp2 = *vals[2];
	else
		tmp2.v_type = V_NULL;
	if (count > 1)
		tmp1 = *vals[1];
	else
		tmp1.v_type = V_NULL;
	sqrtvalue(vals[0], &tmp1, &tmp2, &result);
	return result;
}


static VALUE
f_root(int count, VALUE **vals)
{
	VALUE *vp, err, result;

	if (count > 2)
		vp = vals[2];
	else {
		err.v_num = conf->epsilon;
		err.v_type = V_NUM;
		vp = &err;
	}
	rootvalue(vals[0], vals[1], vp, &result);
	return result;
}


static VALUE
f_power(int count, VALUE **vals)
{
	VALUE *vp, err, result;

	if (count > 2)
		vp = vals[2];
	else {
		err.v_num = conf->epsilon;
		err.v_type = V_NUM;
		vp = &err;
	}
	powervalue(vals[0], vals[1], vp, &result);
	return result;
}


static VALUE
f_polar(int count, VALUE **vals)
{
	VALUE *vp, err, result;
	COMPLEX *c;

	if (count > 2)
		vp = vals[2];
	else {
		err.v_num = conf->epsilon;
		err.v_type = V_NUM;
		vp = &err;
	}
	if ((vals[0]->v_type != V_NUM) || (vals[1]->v_type != V_NUM))
		return error_value(E_POLAR1);
	if ((vp->v_type != V_NUM) || qisneg(vp->v_num) || qiszero(vp->v_num))
		return error_value(E_POLAR2);
	c = cpolar(vals[0]->v_num, vals[1]->v_num, vp->v_num);
	result.v_com = c;
	result.v_type = V_COM;
	if (cisreal(c)) {
		result.v_num = qlink(c->real);
		result.v_type = V_NUM;
		comfree(c);
	}
	return result;
}


static NUMBER *
f_ilog(NUMBER *val1, NUMBER *val2)
{
	return itoq(qilog(val1, val2));
}


static NUMBER *
f_ilog2(NUMBER *val)
{
	return itoq(qilog2(val));
}


static NUMBER *
f_ilog10(NUMBER *val)
{
	return itoq(qilog10(val));
}


static NUMBER *
f_faccnt(NUMBER *val1, NUMBER *val2)
{
	if (qisfrac(val1) || qisfrac(val2))
		math_error("Non-integral argument for fcnt");
	return itoq(zdivcount(val1->num, val2->num));
}


static VALUE
f_matfill(int count, VALUE **vals)
{
	VALUE *v1, *v2, *v3;
	VALUE result;

	v1 = vals[0];
	v2 = vals[1];
	if (v1->v_type != V_ADDR)
		return error_value(E_MATFILL1);
	v1 = v1->v_addr;
	if (v1->v_subtype & V_NOCOPYTO)
		return error_value(E_MATFILL3);
	if (v1->v_type != V_MAT)
		return error_value(E_MATFILL2);
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if (v2->v_subtype & V_NOASSIGNFROM)
		return error_value(E_MATFILL4);
	if (count == 3) {
		v3 = vals[2];
		if (v3->v_type == V_ADDR)
			v3 = v3->v_addr;
		if (v3->v_subtype & V_NOASSIGNFROM)
			return error_value(E_MATFILL4);
	}
	else
		v3 = NULL;
	matfill(v1->v_mat, v2, v3);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_matsum(VALUE *vp)
{
	VALUE result;

	/* firewall */
	if (vp->v_type != V_MAT)
		return error_value(E_MATSUM);

	/* sum matrix */
	matsum(vp->v_mat, &result);
	return result;
}


static VALUE
f_isident(VALUE *vp)
{
	VALUE result;

	result.v_type = V_NUM;
	if (vp->v_type == V_MAT) {
		result.v_num = itoq((long) matisident(vp->v_mat));
	} else {
		result.v_num = itoq(0);
	}
	return result;
}


static VALUE
f_mattrace(VALUE *vp)
{
	if (vp->v_type != V_MAT)
		return error_value(E_MATTRACE1);
	return mattrace(vp->v_mat);
}


static VALUE
f_mattrans(VALUE *vp)
{
	VALUE result;

	if (vp->v_type != V_MAT)
		return error_value(E_MATTRANS1);
	if (vp->v_mat->m_dim != 2)
		return error_value(E_MATTRANS2);
	result.v_type = V_MAT;
	result.v_mat = mattrans(vp->v_mat);
	return result;
}


static VALUE
f_det(VALUE *vp)
{
	MATRIX *m;

	if (vp->v_type != V_MAT)
		return error_value(E_DET1);
	m = vp->v_mat;
	if (m->m_dim != 2)
		return error_value(E_DET2);
	if ((m->m_max[0] - m->m_min[0]) != (m->m_max[1] - m->m_min[1]))
		return error_value(E_DET3);

	return matdet(vp->v_mat);
}


static VALUE
f_matdim(VALUE *vp)
{
	VALUE result;

	result.v_type = V_NUM;

	switch(vp->v_type) {
		case V_OBJ:
			result.v_num = itoq(vp->v_obj->o_actions->count);
			break;
		case V_MAT:
			result.v_num = itoq((long) vp->v_mat->m_dim);
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

	if (v1->v_type != V_MAT)
		return error_value(E_MATMIN1);
	if (v2->v_type != V_NUM)
		return error_value(E_MATMIN2);
	q = v2->v_num;
	if (qisfrac(q) || qisneg(q) || qiszero(q))
		return error_value(E_MATMIN2);
	i = qtoi(q);
	if (i > v1->v_mat->m_dim)
		return error_value(E_MATMIN3);
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

	if (v1->v_type != V_MAT)
		return error_value(E_MATMAX1);
	if (v2->v_type != V_NUM)
		return error_value(E_MATMAX2);
	q = v2->v_num;
	if (qisfrac(q) || qisneg(q) || qiszero(q))
		return error_value(E_MATMAX2);
	i = qtoi(q);
	if (i > v1->v_mat->m_dim)
		return error_value(E_MATMAX3);
	result.v_type = V_NUM;
	result.v_num = itoq(v1->v_mat->m_max[i - 1]);
	return result;
}


static VALUE
f_cp(VALUE *v1, VALUE *v2)
{
	MATRIX *m1, *m2;
	VALUE result;

	if ((v1->v_type != V_MAT) || (v2->v_type != V_MAT))
		return error_value(E_CP1);
	m1 = v1->v_mat;
	m2 = v2->v_mat;
	if ((m1->m_dim != 1) || (m2->m_dim != 1))
		return error_value(E_CP2);
	if ((m1->m_size != 3) || (m2->m_size != 3))
		return error_value(E_CP3);
	result.v_type = V_MAT;
	result.v_mat = matcross(m1, m2);
	return result;
}


static VALUE
f_dp(VALUE *v1, VALUE *v2)
{
	MATRIX *m1, *m2;

	if ((v1->v_type != V_MAT) || (v2->v_type != V_MAT))
		return error_value(E_DP1);
	m1 = v1->v_mat;
	m2 = v2->v_mat;
	if ((m1->m_dim != 1) || (m2->m_dim != 1))
		return error_value(E_DP2);
	if (m1->m_size != m2->m_size)
		return error_value(E_DP3);
	return matdot(m1, m2);
}


static VALUE
f_strlen(VALUE *vp)
{
	VALUE result;
	long len = 0;
	char *c;

	if (vp->v_type != V_STR)
		return error_value(E_STRLEN);
	c = vp->v_str->s_str;
	while (*c++)
		len++;
	result.v_type = V_NUM;
	result.v_num = itoq(len);
	return result;
}


static VALUE
f_strcmp(VALUE *v1, VALUE *v2)
{
	unsigned char *c1, *c2;
	VALUE result;

	if (v1->v_type != V_STR || v2->v_type != V_STR)
		return error_value(E_STRCMP);

	c1 = (unsigned char *)v1->v_str->s_str;
	c2 = (unsigned char *)v2->v_str->s_str;

	result.v_type = V_NUM;
	for (; *c1 == *c2; ++c1, ++c2) {
		if (*c1 == '\0') {
			result.v_num = qlink(&_qzero_);
			return result;
		}
	}
	result.v_num = (*c1 > *c2) ? qlink(&_qone_) : qlink(&_qnegone_);
	return result;
}


static VALUE
f_strncmp(VALUE *v1, VALUE *v2, VALUE *v3)
{
	unsigned char *c1, *c2;
	long i;
	VALUE result;

	if (v1->v_type != V_STR || v2->v_type != V_STR ||
		v3->v_type != V_NUM || qisneg(v3->v_num) ||
		qisfrac(v3->v_num) || zge31b(v3->v_num->num))
			return error_value(E_STRNCMP);
	i = qtoi(v3->v_num);
	for (c1 = (unsigned char *)v1->v_str->s_str,
	     c2 = (unsigned char *)v2->v_str->s_str;
	     i > 0 && *c1 == *c2; ++c1, ++c2, --i) {
		if (*c1 == '\0')
			break;
	}
	result.v_type = V_NUM;
	if (i == 0 || *c1 == *c2)
		result.v_num = qlink(&_qzero_);
	else
		result.v_num = (*c1>*c2) ? qlink(&_qone_) : qlink(&_qnegone_);
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

	len = 0;
	result.v_type = V_STR;
	vp = vals;
	for (i = 0; i < count; i++, vp++) {
		if ((*vp)->v_type != V_STR)
			return error_value(E_STRCAT);
		c = (*vp)->v_str->s_str;
		while (*c++)
			len++;
	}
	if (len == 0) {
		result.v_str = slink(&_nullstring_);
		return result;
	}
	c = (char *) malloc(len + 1) ;
	if (c == NULL) {
		math_error("No memory for strcat");
		/*NOTREACHED*/
	}
	result.v_str = stralloc();
	result.v_str->s_str = c;
	result.v_str->s_len = len;
	for (vp = vals; count-- > 0; vp++) {
		c1 = (*vp)->v_str->s_str;
		while (*c1)
			*c++ = *c1++;
	}
	*c = '\0';
	return result;
}


static VALUE
f_strcpy(VALUE *v1, VALUE *v2)
{
	VALUE result;

	if (v1->v_type != V_STR || v2->v_type != V_STR)
		return error_value(E_STRCPY);
	result.v_str = stringcpy(v1->v_str, v2->v_str);
	result.v_type = V_STR;
	return result;
}


static VALUE
f_strncpy(VALUE *v1, VALUE *v2, VALUE *v3)
{
	VALUE result;
	long num;

	if (v1->v_type != V_STR || v2->v_type != V_STR ||
		v3->v_type != V_NUM || qisfrac(v3->v_num) || qisneg(v3->v_num))
			return error_value(E_STRNCPY);
	if (zge31b(v3->v_num->num))
		num = v2->v_str->s_len;
	else
		num = qtoi(v3->v_num);
	result.v_str = stringncpy(v1->v_str, v2->v_str, num);
	result.v_type = V_STR;
	return result;
}


static VALUE
f_substr(VALUE *v1, VALUE *v2, VALUE *v3)
{
	NUMBER *q1, *q2;
	long i1, i2, len;
	char *cp;
	char *ccp;
	VALUE result;

	if (v1->v_type != V_STR)
		return error_value(E_SUBSTR1);
	if ((v2->v_type != V_NUM) || (v3->v_type != V_NUM))
		return error_value(E_SUBSTR2);
	q1 = v2->v_num;
	q2 = v3->v_num;
	if (qisfrac(q1) || qisneg(q1) || qisfrac(q2) || qisneg(q2))
		return error_value(E_SUBSTR2);
	i1 = qtoi(q1);
	i2 = qtoi(q2);
	cp = v1->v_str->s_str;
	len = (long)strlen(cp);
	result.v_type = V_STR;
	if (i1 > 0)
		i1--;
	if (i1 >= len) {	/* indexing off of end */
		result.v_str = slink(&_nullstring_);
		return result;
	}
	cp += i1;
	len -= i1;
	if (len > i2)
		len = i2;
	ccp = (char *) malloc(len + 1);
	if (ccp == NULL) {
		math_error("No memory for substr");
		/*NOTREACHED*/
	}
	strncpy(ccp, cp, len);
	ccp[len] = '\0';
	result.v_str = makestring(ccp);
	return result;
}


static VALUE
f_char(VALUE *vp)
{
	char ch;
	VALUE result;

	switch(vp->v_type) {
		case V_NUM:
			if (qisfrac(vp->v_num))
				return error_value(E_CHAR);
			ch = (char) vp->v_num->num.v[0];
			if (qisneg(vp->v_num))
				ch = -ch;
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

	switch(vp->v_type) {
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
	result.v_num = itoq((long) (*c & 0xff));
	return result;
}


static VALUE
f_protect(int count, VALUE **vals)
{
	int i;
	VALUE *v1, *v2;
	VALUE result;
	BOOL have_nblock;

	result.v_subtype = V_NOSUBTYPE;
	result.v_type = V_NULL;
	v1 = vals[0];
	have_nblock = (v1->v_type == V_NBLOCK);
	if (!have_nblock) {
		if (v1->v_type != V_ADDR)
			return error_value(E_PROTECT1);
		v1 = v1->v_addr;
	}
	if (count == 1) {
		result.v_type = V_NUM;
		if (have_nblock)
			result.v_num = itoq(v1->v_nblock->subtype);
		else
			result.v_num = itoq(v1->v_subtype);
		return result;
	}
	v2 = vals[1];
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if (v2->v_type != V_NUM || qisfrac(v2->v_num))
		return error_value(E_PROTECT2);
	if (qisneg(v2->v_num) || zge31b(v2->v_num->num))
		return error_value(E_PROTECT3);
	i = qtoi(v2->v_num);
	if (i > MAXPROTECT)
		return error_value(E_PROTECT3);
	if (have_nblock) {
		v1->v_nblock->subtype |= i;
		return result;
	}
	if (i & V_PROTECTALL) {
		protectall(v1, i);
		return result;
	}
	v1->v_subtype |= i;
	return result;
}


static VALUE
f_size(VALUE *vp)
{
	VALUE result;

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

	/*
	 * return information about memory footprint
	 *
	 * This is not the number of elements, see f_size() for that info.
	 * This is not the memsize, see f_memsize() for that information.
	 */
	result.v_type = V_NUM;
	result.v_num = itoq(lsizeof(vp));
	return result;
}


static VALUE
f_memsize(VALUE *vp)
{
	VALUE result;

	/*
	 * return information about memory footprint
	 *
	 * This is not the number of elements, see f_size() for that info.
	 * This is not the sizeof, see f_sizeof() for that information.
	 */
	result.v_type = V_NUM;
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

	v1 = *vals++;
	v2 = *vals++;
	if ((v1->v_type == V_FILE || v1->v_type == V_STR) &&
		 v2->v_type != V_STR)
			return error_value(E_SEARCH2);
	start = end = NULL;
	if (count > 2) {
		v3 = *vals++;
		if (v3->v_type != V_NUM && v3->v_type != V_NULL)
			return error_value(E_SEARCH3);
		if (v3->v_type == V_NUM) {
			start = v3->v_num;
			if (qisfrac(start))
				return error_value(E_SEARCH3);
		}
	}
	if (count > 3) {
		v4 = *vals;
		if (v4->v_type != V_NUM && v4->v_type != V_NULL)
			return error_value(E_SEARCH4);
		if (v4->v_type == V_NUM) {
			end = v4->v_num;
			if (qisfrac(end))
				return error_value(E_SEARCH4);
		}
	}
	result.v_type = V_NULL;
	vsize = f_size(v1);
	if (vsize.v_type != V_NUM)
		return error_value(E_SEARCH5);
	size = vsize.v_num;
	if (start) {
		if (qisneg(start)) {
			start = qqadd(size, start);
			if (qisneg(start)) {
				qfree(start);
				start = qlink(&_qzero_);
			}
		}
		else
			start = qlink(start);
	}
	if (end) {
		if (!qispos(end))
			end = qqadd(size, end);
		else {
			if (qrel(end, size) > 0)
				end = qlink(size);
			else
				end = qlink(end);
		}
	}
	if (v1->v_type == V_FILE) {
		if (count == 2|| (count == 4 &&
				 (start == NULL || end == NULL))) {
			i = ftellid(v1->v_file, &pos);
			if (i < 0) {
				qfree(size);
				if (start)
					qfree(start);
				if (end)
					qfree(end);
				return error_value(E_SEARCH5);
			}
			if (count == 2 || (count == 4 && end != NULL)) {
				start = qalloc();
				start->num = pos;
			}
			else {
				end = qalloc();
				end->num = pos;
			}
		}
		if (start == NULL)
			start = qlink(&_qzero_);
		if (end == NULL)
			end = size;
		else
			qfree(size);
		len = v2->v_str->s_len;
		utoz(len, &zlen);
		zsub(end->num, zlen, &tmp);
		zfree(zlen);
		i = fsearch(v1->v_file, v2->v_str->s_str,
			 start->num, tmp, &indx);
		zfree(tmp);
		if (i == 2) {
			result.v_type = V_NUM;
			result.v_num = start;
			qfree(end);
			return result;
		}
		qfree(start);
		qfree(end);
		if (i == EOF)
			return error_value(errno);
		if (i < 0)
			return error_value(E_SEARCH6);
		if (i == 0) {
			result.v_type = V_NUM;
			result.v_num = qalloc();
			result.v_num->num = indx;
		}
		return result;
	}
	if (start == NULL)
		start = qlink(&_qzero_);
	if (end == NULL)
		end = qlink(size);
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
			i = stringsearch(v1->v_str, v2->v_str, l_start, l_end,
					&indx);
			break;
		default:
			qfree(start);
			qfree(end);
			return error_value(E_SEARCH1);
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

	v1 = *vals++;
	v2 = *vals++;
	if ((v1->v_type == V_FILE || v1->v_type == V_STR) &&
		v2->v_type != V_STR)
			return error_value(E_RSEARCH2);
	start = end = NULL;
	if (count > 2) {
		v3 = *vals++;
		if (v3->v_type != V_NUM && v3->v_type != V_NULL)
			return error_value(E_RSEARCH3);
		if (v3->v_type == V_NUM) {
			start = v3->v_num;
			if (qisfrac(start))
				return error_value(E_RSEARCH3);
		}
	}
	if (count > 3) {
		v4 = *vals;
		if (v4->v_type != V_NUM && v4->v_type != V_NULL)
			return error_value(E_RSEARCH4);
		if (v4->v_type == V_NUM) {
			end = v4->v_num;
			if (qisfrac(end))
				return error_value(E_RSEARCH3);
		}
	}
	result.v_type = V_NULL;
	vsize = f_size(v1);
	if (vsize.v_type != V_NUM)
		return error_value(E_RSEARCH5);
	size = vsize.v_num;
	if (start) {
		if (qisneg(start)) {
			start = qqadd(size, start);
			if (qisneg(start)) {
				qfree(start);
				start = qlink(&_qzero_);
			}
		}
		else
			start = qlink(start);
	}
	if (end) {
		if (!qispos(end))
			end = qqadd(size, end);
		else {
			if (qrel(end, size) > 0)
				end = qlink(size);
			else
				end = qlink(end);
		}
	}
	if (v1->v_type == V_FILE) {
		if (count == 2 || (count == 4 &&
				(start == NULL || end == NULL))) {
			i = ftellid(v1->v_file, &pos);
			if (i < 0) {
				qfree(size);
				if (start)
					qfree(start);
				if (end)
					qfree(end);
				return error_value(E_RSEARCH5);
			}
			if (count == 2 || (count == 4 && end != NULL)) {
				start = qalloc();
				start->num = pos;
			}
			else {
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
		}
		else {
			qtmp = qsub(end, qlen);
			qfree(end);
			end = qtmp;
		}
		if (end == NULL)
			end = qlink(size);
		if (start == NULL)
			start = qlink(&_qzero_);
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
		i = frsearch(v1->v_file, v2->v_str->s_str,
			end->num,start->num, &indx);
		qfree(start);
		qfree(end);
		if (i == EOF)
			return error_value(errno);
		if (i < 0)
			return error_value(E_RSEARCH6);
		if (i == 0) {
			result.v_type = V_NUM;
			result.v_num = qalloc();
			result.v_num->num = indx;
		}
		return result;
	}
	if (count < 4) {
		if (start) {
			end = qinc(start);
			qfree(start);
		}
		else
			end = qlink(size);
		start = qlink(&_qzero_);
	}
	else {
		if (start == NULL)
			start = qlink(&_qzero_);
		if (end == NULL)
			end = qlink(size);
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
			i = stringrsearch(v1->v_str, v2->v_str, l_start,
				l_end, &indx);
			break;
		default:
			qfree(start);
			qfree(end);
			return error_value(E_RSEARCH1);
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

	result.v_type = V_LIST;
	result.v_list = listalloc();
	while (count-- > 0)
		insertlistlast(result.v_list, *vals++);
	return result;
}


/*ARGSUSED*/
static VALUE
f_assoc(int count, VALUE **vals)
{
	VALUE result;

	result.v_type = V_ASSOC;
	result.v_subtype = V_NOSUBTYPE;
	result.v_assoc = assocalloc(0L);
	return result;
}


static VALUE
f_listinsert(int count, VALUE **vals)
{
	VALUE *v1, *v2, *v3;
	VALUE result;
	long pos;

	v1 = *vals++;
	if ((v1->v_type != V_ADDR) || (v1->v_addr->v_type != V_LIST))
		return error_value(E_INSERT1);
	if (v1->v_addr->v_subtype & V_NOREALLOC) {
		math_error("No-relocate list for insert");
		/*NOTREACHED*/
	}
	v2 = *vals++;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v2->v_type != V_NUM) || qisfrac(v2->v_num))
		return error_value(E_INSERT2);
	pos = qtoi(v2->v_num);
	count--;
	while (--count > 0) {
		v3 = *vals++;
		if (v3->v_type == V_ADDR)
			v3 = v3->v_addr;
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

	v1 = *vals++;
	if ((v1->v_type != V_ADDR) || (v1->v_addr->v_type != V_LIST))
		return error_value(E_PUSH);
	if (v1->v_addr->v_subtype & V_NOREALLOC) {
		math_error("No-relocate list for push");
		/*NOTREACHED*/
	}
	while (--count > 0) {
		v2 = *vals++;
		if (v2->v_type == V_ADDR)
			v2 = v2->v_addr;
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

	v1 = *vals++;
	if ((v1->v_type != V_ADDR) || (v1->v_addr->v_type != V_LIST))
		return error_value(E_APPEND);
	if (v1->v_addr->v_subtype & V_NOREALLOC) {
		math_error("No-relocate list for append");
		/*NOTREACHED*/
	}
	while (--count > 0) {
		v2 = *vals++;
		if (v2->v_type == V_ADDR)
			v2 = v2->v_addr;
		insertlistlast(v1->v_addr->v_list, v2);
	}
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_listdelete(VALUE *v1, VALUE *v2)
{
	VALUE result;

	if ((v1->v_type != V_ADDR) || (v1->v_addr->v_type != V_LIST))
		return error_value(E_DELETE1);
	if (v1->v_addr->v_subtype & V_NOREALLOC) {
		math_error("No-relocate list for delete");
		/*NOTREACHED*/
	}
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if ((v2->v_type != V_NUM) || qisfrac(v2->v_num))
		return error_value(E_DELETE2);
	removelistmiddle(v1->v_addr->v_list, qtoi(v2->v_num), &result);
	return result;
}


static VALUE
f_listpop(VALUE *vp)
{
	VALUE result;

	if ((vp->v_type != V_ADDR) || (vp->v_addr->v_type != V_LIST))
		return error_value(E_POP);
	if (vp->v_addr->v_subtype & V_NOREALLOC) {
		math_error("No-relocate list for pop");
		/*NOTREACHED*/
	}
	removelistfirst(vp->v_addr->v_list, &result);
	return result;
}


static VALUE
f_listremove(VALUE *vp)
{
	VALUE result;

	if ((vp->v_type != V_ADDR) || (vp->v_addr->v_type != V_LIST))
		return error_value(E_REMOVE);
	if (vp->v_addr->v_subtype & V_NOREALLOC) {
		math_error("No-relocate list for remove");
		/*NOTREACHED*/
	}
	removelistlast(vp->v_addr->v_list, &result);
	return result;
}


/*
 * Return the current runtime of calc in seconds.
 * This is the user mode time only.
 */
static NUMBER *
f_runtime(void)
{
	struct tms buf;

	times(&buf);
	return iitoq((long) buf.tms_utime, (long) CLK_TCK);
}


/*
 * return the number of second since the Epoch (00:00:00 1 Jan 1970 UTC).
 */
static NUMBER *
f_time(void)
{
	return itoq((long) time(0));
}


/*
 * time in asctime()/ctime() format
 */
static VALUE
f_ctime(void)
{
	time_t systime;
	char *str;
	VALUE res;

	str = (char *) malloc(26);
	if (str == NULL) {
		math_error("No memory for ctime()");
		/*NOTREACHED*/
	}
	systime = time(NULL);
	strcpy(str, ctime(&systime));
	str[24] = '\0';
	res.v_str = makestring(str);
	res.v_type = V_STR;
	return res;
}


static VALUE
f_fopen(VALUE *v1, VALUE *v2)
{
	VALUE result;
	FILEID id;
	char *mode;

	if (v1->v_type != V_STR || v2->v_type != V_STR)
		return error_value(E_FOPEN1);
	mode = v2->v_str->s_str;

	if ((*mode != 'r') && (*mode != 'w') && (*mode != 'a'))
		return error_value(E_FOPEN2);
	if (mode[1] != '\0') {
		if (mode[1] != '+')
			return error_value(E_FOPEN2);
		if (mode[2] != '\0')
			return error_value(E_FOPEN2);
	}
	errno = 0;
	id = openid(v1->v_str->s_str, v2->v_str->s_str);
	if (id == FILEID_NONE)
		return error_value(errno);
	if (id < 0)
		return error_value(E_FOPEN3);
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

	if (vals[0]->v_type != V_FILE)
		return error_value(E_FREOPEN1);
	if (vals[1]->v_type != V_STR)
		return error_value(E_FREOPEN2);

	mode = vals[1]->v_str->s_str;

	if ((*mode != 'r') && (*mode != 'w') && (*mode != 'a'))
		return error_value(E_FREOPEN2);
	if (mode[1] != '\0') {
		if (mode[1] != '+')
			return error_value(E_FREOPEN2);
		if (mode[2] != '\0')
			return error_value(E_FREOPEN2);
	}
	errno = 0;
	if (count == 2)
		id = reopenid(vals[0]->v_file, mode, NULL);
	else {
		if (vals[2]->v_type != V_STR)
			return error_value(E_FREOPEN3);
		id = reopenid(vals[0]->v_file, mode,
			vals[2]->v_str->s_str);
	}

	if (id == FILEID_NONE)
		return error_value(errno);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_errno(int count, VALUE **vals)
{
	int newerr, olderr;
	VALUE *vp;
	VALUE result;

	newerr = -1;
	result.v_type = V_NUM;

	if (count > 0) {
		vp = vals[0];

		if (vp->v_type <= 0) {
			newerr = (int) -vp->v_type;
			(void) set_errno(newerr);
			result.v_num = itoq((long) newerr);
			return result;
		}

		/* arg must be an integer */
		if (vp->v_type != V_NUM || qisfrac(vp->v_num) ||
		qisneg(vp->v_num) || zge16b(vp->v_num->num)) {
			math_error("errno argument out of range");
			/*NOTREACHED*/
		}
		newerr = (int) ztoi(vp->v_num->num);
	}
	olderr = set_errno(newerr);

	result.v_num = itoq((long) olderr);
	return result;
}



static VALUE
f_errcount(int count, VALUE **vals)
{
	int newcount, oldcount;
	VALUE *vp;
	VALUE result;

	newcount = -1;
	if (count > 0) {
		vp = vals[0];

		/* arg must be an integer */
		if (vp->v_type != V_NUM || qisfrac(vp->v_num) ||
		qisneg(vp->v_num) || zge31b(vp->v_num->num)) {
			math_error("errcount argument out of range");
			/*NOTREACHED*/
		}
		newcount = (int) ztoi(vp->v_num->num);
	}
	oldcount = set_errcount(newcount);

	result.v_type = V_NUM;
	result.v_num = itoq((long) oldcount);
	return result;
}


static VALUE
f_errmax(int count, VALUE **vals)
{
	int oldmax;
	VALUE *vp;
	VALUE result;

	oldmax = errmax;
	if (count > 0) {
		vp = vals[0];

		if (vp->v_type != V_NUM || qisfrac(vp->v_num) ||
				zge31b(vp->v_num->num))
			fprintf(stderr,
				 "Out-of-range arg for errmax ignored\n");
		else
			errmax = (int) ztoi(vp->v_num->num);
	}

	result.v_type = V_NUM;
	result.v_num = itoq((long) oldmax);
	return result;
}


static VALUE
f_stoponerror(int count, VALUE **vals)
{
	int oldval;
	VALUE *vp;
	VALUE result;

	oldval = stoponerror;
	if (count > 0) {
		vp = vals[0];

		if (vp->v_type != V_NUM || qisfrac(vp->v_num) ||
				zge31b(vp->v_num->num))
			fprintf(stderr,
				 "Out-of-range arg for stoponerror ignored\n");
		else
			stoponerror = (int) ztoi(vp->v_num->num);
	}

	result.v_type = V_NUM;
	result.v_num = itoq((long) oldval);
	return result;
}

static VALUE
f_fclose(int count, VALUE **vals)
{
	VALUE result;
	VALUE *vp;
	int n, i=0;

	errno = 0;
	if (count == 0) {
		i = closeall();
	} else {
		for (n = 0; n < count; n++) {
			vp = vals[n];
			if (vp->v_type != V_FILE)
				return error_value(E_FCLOSE1);
		}
		for (n = 0; n < count; n++) {
			vp = vals[n];
			i = closeid(vp->v_file);
			if (i < 0)
				return error_value(E_REWIND2);
		}
	}
	if (i < 0)
		return error_value(errno);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_rm(int count, VALUE **vals)
{
	VALUE result;
	int force;	/* TRUE -> -f was given as 1st arg */
	int i;
	int j;

	/*
	 * firewall
	 */
	if (!allow_write)
		return error_value(E_WRPERM);

	/*
	 * check on each arg
	 */
	for (i=0; i < count; ++i) {
		if (vals[i]->v_type != V_STR)
			return error_value(E_RM1);
		if (vals[i]->v_str->s_str[0] == '\0')
			return error_value(E_RM1);
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
	for (i=0; i < count; ++i) {
		j = remove(vals[i]->v_str->s_str);
		if (!force && j < 0)
			return error_value(errno);
	}
	result.v_type = V_NULL;
	result.v_subtype = V_NOSUBTYPE;
	return result;
}


static VALUE
f_newerror(int count, VALUE **vals)
{
	char *str;
	int index;
	int errnum;

	str = NULL;
	if (count > 0 && vals[0]->v_type == V_STR)
		str = vals[0]->v_str->s_str;
	if (str == NULL || str[0] == '\0')
		str = "???";
	if (nexterrnum == E_USERDEF)
		initstr(&newerrorstr);
	index = findstr(&newerrorstr, str);
	if (index >= 0)
		errnum = E_USERDEF + index;
	else {
		if (nexterrnum == 32767)
			math_error("Too many new error values");
		errnum = nexterrnum++;
		addstr(&newerrorstr, str);
	}
	return error_value(errnum);
}


static VALUE
f_strerror(int count, VALUE **vals)
{
	VALUE *vp;
	VALUE result;
	long i;
	char *cp;

	if (count > 0) {
		vp = vals[0];
		if (vp->v_type < 0)
			i = (long) -vp->v_type;
		else {
			if (vp->v_type != V_NUM || qisfrac(vp->v_num))
				return error_value(E_STRERROR1);
			i = qtoi(vp->v_num);
			if (i < 0 || i > 32767)
				return error_value(E_STRERROR2);
		}
	}
	else
		i = set_errno(-1);

	result.v_type = V_STR;

	if (i == 0)
		i = E__BASE;

	if (i >= nexterrnum || (i > E__HIGHEST && i < E_USERDEF)
			|| (i < E__BASE && i >= sys_nerr)) {
		cp = (char *) malloc(12);
		if (cp == NULL) {
			math_error("Out of memory for strerror");
			/*NOTREACHED*/
		}
		sprintf(cp, "Error %ld", i);
		result.v_str = makestring(cp);
		return result;
	}

	if (i < E__BASE)		/* system error */
		cp = (char *) sys_errlist[i];

	else if (i >= E_USERDEF)	/* user-described error */
		cp = namestr(&newerrorstr, i - E_USERDEF);

	else				/* calc-described error */
		cp = (char *)error_table[i - E__BASE];

	result.v_str = makenewstring(cp);
	return result;
}


static VALUE
f_ferror(VALUE *vp)
{
	VALUE result;
	int i;

	if (vp->v_type != V_FILE)
		return error_value(E_FERROR1);
	i = errorid(vp->v_file);
	if (i < 0)
		return error_value(E_FERROR2);
	result.v_type = V_NUM;
	result.v_num = itoq((long) i);
	return result;
}


static VALUE
f_feof(VALUE *vp)
{
	VALUE result;
	int i;

	if (vp->v_type != V_FILE)
		return error_value(E_FEOF1);
	i = eofid(vp->v_file);
	if (i < 0)
		return error_value(E_FEOF2);
	result.v_type = V_NUM;
	result.v_num = itoq((long) i);
	return result;
}


static VALUE
f_fflush(int count, VALUE **vals)
{
	VALUE result;
	int i, n;

	i = 0;
	errno = 0;
	if (count == 0)
		i = flushall();
	else {
		for (n = 0; n < count; n++) {
			if (vals[n]->v_type != V_FILE)
				return error_value(E_FFLUSH);
		}
		for (n = 0; n < count; n++) {
			i |= flushid(vals[n]->v_file);
		}
	}
	if (i == EOF)
		return error_value(errno);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_error(int count, VALUE **vals)
{
	VALUE *vp;
	long r;

	if (count > 0) {
		vp = vals[0];

		if (vp->v_type <= 0)
			r = (long) -vp->v_type;
		else {
			if (vp->v_type != V_NUM || qisfrac(vp->v_num))
				r = E_ERROR1;
			else {
				r = qtoi(vp->v_num);
				if (r < 0 || r >= 32768)
					r = E_ERROR2;
			}
		}
	}
	else
		r = set_errno(-1);

	return error_value(r);
}


static VALUE
f_iserror(VALUE *vp)
{
	VALUE res;

	res.v_type = V_NUM;
	res.v_num = itoq((long)((vp->v_type < 0) ? - vp->v_type : 0));
	return res;
}


static VALUE
f_fsize(VALUE *vp)
{
	VALUE result;
	ZVALUE len;		/* file length */
	int i;

	if (vp->v_type != V_FILE)
		return error_value(E_FSIZE1);
	i = getsize(vp->v_file, &len);
	if (i == EOF)
		return error_value(errno);
	if (i)
		return error_value(E_FSIZE2);
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

	/* firewalls */
	errno = 0;
	if (vals[0]->v_type != V_FILE)
		return error_value(E_FSEEK1);
	if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num))
		return error_value(E_FSEEK2);
	if (count == 2)
		whence = 0;
	else {
		if (vals[2]->v_type != V_NUM || qisfrac(vals[2]->v_num) ||
			qisneg(vals[2]->v_num))
			return error_value(E_FSEEK2);
		if (vals[2]->v_num->num.len > 1)
			return error_value (E_FSEEK2);
		whence = (int)(unsigned int)(vals[2]->v_num->num.v[0]);
		if (whence > 2)
			return error_value (E_FSEEK2);
	}

	i = fseekid(vals[0]->v_file, vals[1]->v_num->num, whence);
	result.v_type = V_NULL;
	if (i == EOF)
		return error_value(errno);
	if (i < 0)
		return error_value(E_FSEEK3);
	return result;
}


static VALUE
f_ftell(VALUE *vp)
{
	VALUE result;
	ZVALUE pos;		/* current file position */
	int i;

	errno = 0;
	if (vp->v_type != V_FILE)
		return error_value(E_FTELL1);
	i = ftellid(vp->v_file, &pos);
	if (i < 0)
		return error_value(E_FTELL2);

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

	if (count == 0)
		rewindall();

	else {
		for (n = 0; n < count; n++) {
			if (vals[n]->v_type != V_FILE)
				return error_value(E_REWIND1);
		}
		for (n = 0; n < count; n++) {
			if (rewindid(vals[n]->v_file) != 0) {
				return error_value(E_REWIND2);
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

	if (vals[0]->v_type != V_FILE)
		return error_value(E_FPRINTF1);
	if (vals[1]->v_type != V_STR)
		return error_value(E_FPRINTF2);
	i = idprintf(vals[0]->v_file, vals[1]->v_str->s_str,
			 count - 2, vals + 2);
	if (i > 0)
		return error_value(E_FPRINTF3);
	result.v_type = V_NULL;
	return result;
}


static int
strscan(char *s, int count, VALUE **vals)
{
	char	ch, chtmp;
	char	*s0;
	int	n = 0;
	VALUE	val, result;
	VALUE	*var;

	val.v_type = V_STR;
	while (*s != '\0') {
		s--;
		while ((ch = *++s)) {
			if (!isspace((int)ch))
				break;
		}
		if (ch == '\0' || count-- == 0)
			return n;
		s0 = s;
		while ((ch = *++s)) {
			if (isspace((int)ch))
				break;
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
	char	*str;
	int 	i;
	int	n = 0;
	VALUE	val;
	VALUE	result;
	VALUE	*var;

	val.v_type = V_STR;

	while (count-- > 0) {

		i = readid(id, 6, &str);

		if (i == EOF)
			break;
		if (i > 0)
			return EOF;
		n++;
		val.v_str = makenewstring(str);
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

	cp = nextline();
	if (cp == NULL) {
		result.v_type = V_NULL;
		return result;
	}

	i = strscan(cp, count, vals);

	result.v_type = V_NUM;
	result.v_num = itoq((long) i);
	return result;
}


static VALUE
f_strscan(int count, VALUE **vals)
{
	VALUE *vp;
	VALUE result;
	int i;

	vp = *vals;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_STR)
		return error_value(E_STRSCAN);

	i = strscan(vp->v_str->s_str, count - 1, vals + 1);

	result.v_type = V_NUM;
	result.v_num = itoq((long) i);
	return result;
}


static VALUE
f_fscan(int count, VALUE **vals)
{
	VALUE *vp;
	VALUE result;
	int i;

	errno = 0;
	vp = *vals;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_FILE)
		return error_value(E_FSCAN1);

	i = filescan(vp->v_file, count - 1, vals + 1);

	if (i == EOF)
		return error_value(errno);
	if (i < 0)
		return error_value(E_FSCAN2);

	result.v_type = V_NUM;
	result.v_num = itoq((long) i);
	return result;
}


static VALUE
f_scanf(int count, VALUE **vals)
{
	VALUE *vp;
	VALUE result;
	int i;

	vp = *vals;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_STR)
		return error_value(E_SCANF1);
	for (i = 1; i < count; i++) {
		if (vals[i]->v_type != V_ADDR)
			return error_value(E_SCANF2);
	}
	i = fscanfid(FILEID_STDIN, vp->v_str->s_str, count - 1, vals + 1);
	if (i < 0)
		return error_value(E_SCANF3);
	result.v_type = V_NUM;
	result.v_num = itoq((long) i);
	return result;
}


static VALUE
f_strscanf(int count, VALUE **vals)
{
	VALUE *vp, *vq;
	VALUE result;
	int i;

	errno = 0;
	vp = vals[0];
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_STR)
		return error_value(E_STRSCANF1);
	vq = vals[1];
	if (vq->v_type == V_ADDR)
		vq = vq->v_addr;
	if (vq->v_type != V_STR)
		return error_value(E_STRSCANF2);
	for (i = 2; i < count; i++) {
		if (vals[i]->v_type != V_ADDR)
			return error_value(E_STRSCANF3);
	}
	i = scanfstr(vp->v_str->s_str, vq->v_str->s_str,
			 count - 2, vals + 2);
	if (i == EOF)
		return error_value(errno);
	if (i < 0)
		return error_value(E_STRSCANF4);
	result.v_type = V_NUM;
	result.v_num = itoq((long) i);
	return result;
}


static VALUE
f_fscanf(int count, VALUE **vals)
{
	VALUE *vp, *sp;
	VALUE result;
	int i;

	vp = *vals++;
	if (vp->v_type == V_ADDR)
		vp = vp->v_addr;
	if (vp->v_type != V_FILE)
		return error_value(E_FSCANF1);
	sp = *vals++;
	if (sp->v_type == V_ADDR)
		sp = sp->v_addr;
	if (sp->v_type != V_STR)
		return error_value(E_FSCANF2);
	for (i = 0; i < count - 2; i++) {
		if (vals[i]->v_type != V_ADDR)
			return error_value(E_FSCANF3);
	}
	i = fscanfid(vp->v_file, sp->v_str->s_str, count - 2, vals);
	if (i == EOF) {
		result.v_type = V_NULL;
		return result;
	}
	if (i < 0)
		return error_value(E_FSCANF4);
	result.v_type = V_NUM;
	result.v_num = itoq((long) i);
	return result;
}


static VALUE
f_fputc(VALUE *v1, VALUE *v2)
{
	VALUE result;
	NUMBER *q;
	int ch;
	int i;

	if (v1->v_type != V_FILE)
		return error_value(E_FPUTC1);
	switch (v2->v_type) {
		case V_STR:
			ch = v2->v_str->s_str[0];
			break;
		case V_NUM:
			q = v2->v_num;
			if (!qisint(q))
				return error_value(E_FPUTC2);

			ch = qisneg(q) ? (int)(-q->num.v[0] & 0xff) :
					 (int)(q->num.v[0] & 0xff);
			break;
		case V_NULL:
			ch = 0;
			break;
		default:
			return error_value(E_FPUTC2);
	}
	i = idfputc(v1->v_file, ch);
	if (i > 0)
		return error_value(E_FPUTC3);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_fputs(int count, VALUE **vals)
{
	VALUE result;
	int i, err;

	if (vals[0]->v_type != V_FILE)
		return error_value(E_FPUTS1);
	for (i = 1; i < count; i++) {
		if (vals[i]->v_type != V_STR)
			return error_value(E_FPUTS2);
	}
	for (i = 1; i < count; i++) {
		err = idfputs(vals[0]->v_file, vals[i]->v_str->s_str);
		if (err > 0)
			return error_value(E_FPUTS3);
	}
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_fputstr(int count, VALUE **vals)
{
	VALUE result;
	int i, err;

	if (vals[0]->v_type != V_FILE)
		return error_value(E_FPUTSTR1);
	for (i = 1; i < count; i++) {
		if (vals[i]->v_type != V_STR)
			return error_value(E_FPUTSTR2);
	}
	for (i = 1; i < count; i++) {
		err = idfputstr(vals[0]->v_file,
			vals[i]->v_str->s_str);
		if (err > 0)
			return error_value(E_FPUTSTR3);
	}
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_printf(int count, VALUE **vals)
{
	VALUE result;
	int i;

	if (vals[0]->v_type != V_STR)
		return error_value(E_PRINTF1);
	i = idprintf(FILEID_STDOUT, vals[0]->v_str->s_str,
			 count - 1, vals + 1);
	if (i)
		return error_value(E_PRINTF2);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_strprintf(int count, VALUE **vals)
{
	VALUE result;
	int i;
	char *cp;

	if (vals[0]->v_type != V_STR)
		return error_value(E_STRPRINTF1);
	math_divertio();
	i = idprintf(FILEID_STDOUT, vals[0]->v_str->s_str,
		 count - 1, vals + 1);
	if (i) {
		free(math_getdivertedio());
		return error_value(E_STRPRINTF2);
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

	if (vp->v_type != V_FILE)
		return error_value(E_FGETC1);
	ch = getcharid(vp->v_file);
	if (ch == -2)
		return error_value(E_FGETC2);
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

	errno = 0;
	if (v1->v_type != V_FILE)
		return error_value(E_UNGETC1);
	switch (v2->v_type) {
		case V_STR:
			ch = v2->v_str->s_str[0];
			break;
		case V_NUM:
			q = v2->v_num;
			if (!qisint(q))
				return error_value(E_UNGETC2);
			ch = qisneg(q) ? (int)(-q->num.v[0] & 0xff) :
					 (int)(q->num.v[0] & 0xff);
			break;
		default:
			return error_value(E_UNGETC2);
	}
	i = idungetc(v1->v_file, ch);
	if (i == EOF)
		return error_value(errno);
	if (i == -2)
		return error_value(E_UNGETC3);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_fgetline(VALUE *vp)
{
	VALUE result;
	char *str;
	int i;

	if (vp->v_type != V_FILE)
		return error_value(E_FGETLINE1);
	i = readid(vp->v_file, 9, &str);
	if (i > 0)
		return error_value(E_FGETLINE2);
	result.v_type = V_NULL;
	if (i == 0) {
		result.v_type = V_STR;
		result.v_str = makestring(str);
	}
	return result;
}


static VALUE
f_fgets(VALUE *vp)
{
	VALUE result;
	char *str;
	int i;

	if (vp->v_type != V_FILE)
		return error_value(E_FGETS1);
	i = readid(vp->v_file, 1, &str);
	if (i > 0)
		return error_value(E_FGETS2);
	result.v_type = V_NULL;
	if (i == 0) {
		result.v_type = V_STR;
		result.v_str = makestring(str);
	}
	return result;
}


static VALUE
f_fgetstr(VALUE *vp)
{
	VALUE result;
	char *str;
	int i;

	if (vp->v_type != V_FILE)
		return error_value(E_FGETSTR1);
	i = readid(vp->v_file, 10, &str);
	if (i > 0)
		return error_value(E_FGETSTR2);
	result.v_type = V_NULL;
	if (i == 0) {
		result.v_type = V_STR;
		result.v_str = makestring(str);
	}
	return result;
}


static VALUE
f_fgetfield(VALUE *vp)
{
	VALUE result;
	char *str;
	int i;

	if (vp->v_type != V_FILE)
		return error_value(E_FGETWORD1);
	i = readid(vp->v_file, 14, &str);
	if (i > 0)
		return error_value(E_FGETWORD2);
	result.v_type = V_NULL;
	if (i == 0) {
		result.v_type = V_STR;
		result.v_str = makestring(str);
	}
	return result;
}


static VALUE
f_files(int count, VALUE **vals)
{
	VALUE result;

	if (count == 0) {
		result.v_type = V_NUM;
		result.v_num = itoq((long) MAXFILES);
		return result;
	}
	if ((vals[0]->v_type != V_NUM) || qisfrac(vals[0]->v_num))
		return error_value(E_FILES);
	result.v_type = V_NULL;
	result.v_file = indexid(qtoi(vals[0]->v_num));
	if (result.v_file != FILEID_NONE)
		result.v_type = V_FILE;
	return result;
}


static VALUE
f_reverse(VALUE *val)
{
	VALUE res;

	res.v_type = val->v_type;
	switch(val->v_type) {
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
			if (res.v_str == NULL)
				return error_value(E_STRNEG);
			break;
		default:
			math_error("Bad argument type for reverse");
			/*NOTREACHED*/
	}
	return res;
}


static VALUE
f_sort(VALUE *val)
{
	VALUE res;

	res.v_type = val->v_type;
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
			/*NOTREACHED*/
	}
	return res;
}


static VALUE
f_join(int count, VALUE **vals)
{
	LIST *lp;
	LISTELEM *ep;
	VALUE res;

	lp = listalloc();
	while (count-- > 0) {
		if (vals[0]->v_type != V_LIST) {
			listfree(lp);
			printf("Non-list argument for join\n");
			res.v_type = V_NULL;
			return res;
		}
		for (ep = vals[0]->v_list->l_first; ep; ep = ep->e_next)
			insertlistlast(lp, &ep->e_value);
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

	if (v2->v_type != V_NUM || qisfrac(v2->v_num) ||
		zge31b(v2->v_num->num))
			return error_value(E_HEAD2);
	n = qtoi(v2->v_num);

	res.v_type = v1->v_type;
	switch (v1->v_type) {
		case V_LIST:
			if (n == 0)
				res.v_list = listalloc();
			else if (n > 0)
				res.v_list = listsegment(v1->v_list,0,n-1);
			else
				res.v_list = listsegment(v1->v_list,-n-1,0);
			return res;
		case V_STR:
			if (n == 0)
				res.v_str = slink(&_nullstring_);
			else if (n > 0)
				res.v_str = stringsegment(v1->v_str,0,n-1);
			else
				res.v_str = stringsegment(v1->v_str,-n-1,0);
			if (res.v_str == NULL)
				return error_value(E_STRHEAD);
			return res;
		default:
			return error_value(E_HEAD1);
	}
}


static VALUE
f_tail(VALUE *v1, VALUE *v2)
{
	long n;
	VALUE res;

	if (v2->v_type != V_NUM || qisfrac(v2->v_num) ||
		zge31b(v2->v_num->num))
			return error_value(E_TAIL1);
	n = qtoi(v2->v_num);
	res.v_type = v1->v_type;
	switch (v1->v_type) {
		case V_LIST:
			if (n == 0)
				res.v_list = listalloc();
			else if (n > 0) {
				res.v_list = listsegment(v1->v_list,
					v1->v_list->l_count - n,
					v1->v_list->l_count - 1);
			}
			else {
				res.v_list = listsegment(v1->v_list,
					v1->v_list->l_count - 1,
					v1->v_list->l_count + n);
			}
			return res;
		case V_STR:
			if (n == 0)
				res.v_str = slink(&_nullstring_);
			else if (n > 0) {
				res.v_str = stringsegment(v1->v_str,
					v1->v_str->s_len - n,
					v1->v_str->s_len - 1);
			}
			else {
				res.v_str = stringsegment(v1->v_str,
					v1->v_str->s_len - 1,
					v1->v_str->s_len + n);
			}
			if (res.v_str == V_NULL)
				return error_value(E_STRTAIL);
			return res;
		default:
			return error_value(E_TAIL1);
	}
}


static VALUE
f_segment(int count, VALUE **vals)
{
	VALUE *vp;
	long n1, n2;
	VALUE result;

	vp = vals[1];

	if (vp->v_type != V_NUM || qisfrac(vp->v_num) || zge31b(vp->v_num->num))
		return error_value(E_SEG2);
	n1 = qtoi(vp->v_num);
	n2 = n1;
	if (count == 3) {
		vp = vals[2];
		if (vp->v_type != V_NUM || qisfrac(vp->v_num) ||
			zge31b(vp->v_num->num))
				return error_value(E_SEG3);
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
			if (result.v_str == NULL)
				return error_value(E_STRSEG);
			return result;
		default:
			return error_value(E_SEG1);
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

	if (v1->v_type != V_ADDR) {
		math_error("Non-variable first argument for modify");
		/*NOTREACHED*/
	}
	v1 = v1->v_addr;
	if (v2->v_type == V_ADDR)
		v2 = v2->v_addr;
	if (v2->v_type != V_STR) {
		math_error("Non-string second argument for modify");
		/*NOTREACHED*/
	}
	fp = findfunc(adduserfunc(v2->v_str->s_str));
	if (!fp) {
		math_error("Undefined function for modify");
		/*NOTREACHED*/
	}
	switch (v1->v_type) {
		case V_LIST:
			for (ep = v1->v_list->l_first; ep; ep = ep->e_next) {
				*++stack = ep->e_value;
				calculate(fp, 1);
				ep->e_value = *stack--;
			}
			break;
		case V_MAT:
			vp = v1->v_mat->m_table;
			s = v1->v_mat->m_size;
			while (s-- > 0) {
				*++stack = *vp;
				calculate(fp, 1);
				*vp++ = *stack--;
			}
			break;
		default:
		    math_error("Non list or matrix first argument for modify");
		    /*NOTREACHED*/
	}
	res.v_type = V_NULL;
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

	if (v2->v_type != V_STR) {
		math_error("Non-string second argument for forall");
		/*NOTREACHED*/
	}
	fp = findfunc(adduserfunc(v2->v_str->s_str));
	if (!fp) {
		math_error("Undefined function for forall");
		/*NOTREACHED*/
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
		    /*NOTREACHED*/
	}
	res.v_type = V_NULL;
	return res;
}


static VALUE
f_select(VALUE *v1, VALUE *v2)
{
	LIST *lp;
	LISTELEM *ep;
	FUNC *fp;
	VALUE res;

	if (v1->v_type != V_LIST) {
		math_error("Non-list first argument for select");
		/*NOTREACHED*/
	}
	if (v2->v_type != V_STR) {
		math_error("Non-string second argument for select");
		/*NOTREACHED*/
	}
	fp = findfunc(adduserfunc(v2->v_str->s_str));
	if (!fp) {
		math_error("Undefined function for select");
		/*NOTREACHED*/
	}
	lp = listalloc();
	for (ep = v1->v_list->l_first; ep; ep = ep->e_next) {
		copyvalue(&ep->e_value, ++stack);
		calculate(fp, 1);
		if (testvalue(stack))
			insertlistlast(lp, &ep->e_value);
		freevalue(stack--);
	}
	res.v_type = V_LIST;
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

	if (v2->v_type != V_STR) {
		math_error("Non-string second argument for select");
		/*NOTREACHED*/
	}
	fp = findfunc(adduserfunc(v2->v_str->s_str));
	if (!fp) {
		math_error("Undefined function for select");
		/*NOTREACHED*/
	}
	switch (v1->v_type) {
		case V_LIST:
			for (ep = v1->v_list->l_first; ep; ep = ep->e_next) {
				copyvalue(&ep->e_value, ++stack);
				calculate(fp, 1);
				if (testvalue(stack))
					n++;
				freevalue(stack--);
			}
			break;
		case V_MAT:
			s = v1->v_mat->m_size;
			vp = v1->v_mat->m_table;
			while (s-- > 0) {
				copyvalue(vp++, ++stack);
				calculate(fp, 1);
				if (testvalue(stack))
					n++;
				freevalue(stack--);
			}
			break;
		default:
			math_error("Bad argument type for count");
			/*NOTREACHED*/
	}
	res.v_type = V_NUM;
	res.v_num = itoq(n);
	return res;
}


static VALUE
f_makelist(VALUE *v1)
{
	LIST *lp;
	VALUE res;
	long n;

	if (v1->v_type != V_NUM || qisfrac(v1->v_num) || qisneg(v1->v_num)) {
		math_error("Bad argument for makelist");
		/*NOTREACHED*/
	}
	if (zge31b(v1->v_num->num)) {
		math_error("makelist count >= 2^31");
		/*NOTREACHED*/
	}
	n = qtoi(v1->v_num);
	lp = listalloc();
	res.v_type = V_NULL;
	res.v_subtype = V_NOSUBTYPE;
	while (n-- > 0)
		insertlistlast(lp, &res);
	res.v_type = V_LIST;
	res.v_list = lp;
	return res;
}


static VALUE
f_randperm(VALUE *val)
{
	VALUE res;

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
			/*NOTREACHED*/
	}
	return res;
}


static VALUE
f_cmdbuf(void)
{
	VALUE result;
        char *newcp;

	newcp = (char *)malloc(strlen(cmdbuf) + 1);
	strcpy(newcp, cmdbuf);
        result.v_type = V_STR;
	result.v_str = makestring(newcp);
	return result;
}


static VALUE
f_getenv(VALUE *v1)
{
	VALUE result;
	char *str;

	if (v1->v_type != V_STR) {
		math_error("Non-string argument for getenv");
		/*NOTREACHED*/
	}
        result.v_type = V_STR;
	str = getenv(v1->v_str->s_str);
	if (str == NULL)
		result.v_type = V_NULL;
	else
		result.v_str = makenewstring(str);
	return result;
}


static VALUE
f_isatty(VALUE *vp)
{
	VALUE result;

	result.v_type = V_NUM;
	if (vp->v_type == V_FILE && isattyid(vp->v_file) == 1) {
		result.v_num = itoq(1);
	} else {
		result.v_num = itoq(0);
	}
	return result;
}


static VALUE
f_access(int count, VALUE **vals)
{
	NUMBER *q;
	int m;
	char *s, *fname;
	VALUE result;
	long i;

	errno = 0;
	if (vals[0]->v_type != V_STR)
		return error_value(E_ACCESS1);
	fname = vals[0]->v_str->s_str;
	m = 0;
	if (count == 2) {
		switch (vals[1]->v_type) {
			case V_NUM:
				q = vals[1]->v_num;
				if (qisfrac(q) || qisneg(q))
					return error_value(E_ACCESS2);
				m = (int)(q->num.v[0] & 7);
				break;
			case V_STR:
				s = vals[1]->v_str->s_str;
				i = (long)strlen(s);
				while (i-- > 0) {
					switch (*s++) {
					case 'r': m |= 4; break;
					case 'w': m |= 2; break;
					case 'x': m |= 1; break;
					default: return error_value(E_ACCESS2);
					}
				}
				break;
			case V_NULL:
				break;
			default:
				return error_value(E_ACCESS2);
		}
	}
	i = access(fname, m);
	if (i)
		return error_value(errno);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_putenv(int count, VALUE **vals)
{
	VALUE result;
        char *putenv_str;

	/*
	 * parse args
	 */
	if (count == 2) {
		/* firewall */
		if (vals[0]->v_type != V_STR || vals[1]->v_type != V_STR) {
			math_error("Non-string argument for putenv");
			/*NOTREACHED*/
		}

		/* convert putenv("foo","bar") into putenv("foo=bar") */
		putenv_str = (char *)malloc(vals[0]->v_str->s_len + 1 +
			    vals[1]->v_str->s_len + 1);
		if (putenv_str == NULL) {
			math_error("Cannot allocate string in putenv");
			/*NOTREACHED*/
		}
		sprintf(putenv_str, "%s=%s", vals[0]->v_str->s_str,
			vals[1]->v_str->s_str);


	} else {
		/* firewall */
		if (vals[0]->v_type != V_STR) {
			math_error("Non-string argument for putenv");
			/*NOTREACHED*/
		}

		/* putenv(arg) must be of the form "foo=bar" */
		if ((char *)strchr(vals[0]->v_str->s_str, '=') == NULL) {
			math_error("putenv single arg string missing =");
			/*NOTREACHED*/
		}

		/*
		 * make a copy of the arg because subsequent changes
		 * would change the environment.
		 */
		putenv_str = (char *)malloc(vals[0]->v_str->s_len + 1);
		if (putenv_str == NULL) {
			math_error("Cannot allocate string in putenv");
			/*NOTREACHED*/
		}
		strcpy(putenv_str, vals[0]->v_str->s_str);
	}

	/* return putenv result */
	result.v_type = V_NUM;
	result.v_num = itoq((long) putenv(putenv_str));
	free(putenv_str);
	return result;
}


static VALUE
f_strpos(VALUE *haystack, VALUE *needle)
{
	VALUE result;
        char *cpointer;
        int cindex;

	if (haystack->v_type != V_STR || needle->v_type != V_STR) {
		math_error("Non-string argument for index");
		/*NOTREACHED*/
	}
	result.v_type = V_NUM;
        cpointer = strstr(haystack->v_str->s_str,
		needle->v_str->s_str);
        if (cpointer == NULL)
		cindex = 0;
        else
		cindex = cpointer - haystack->v_str->s_str + 1;
	result.v_num = itoq((long) cindex);
	return result;
}


static VALUE
f_system(VALUE *vp)
{
	VALUE result;

	if (vp->v_type != V_STR) {
		math_error("Non-string argument for system");
		/*NOTREACHED*/
	}
	if (!allow_exec) {
		math_error("execution disallowed by -m");
		/*NOTREACHED*/
	}
	result.v_type = V_NUM;
	result.v_num = itoq((long) system(vp->v_str->s_str));
	return result;
}


/*
 * set the default output base/mode
 */
static NUMBER *
f_base(int count, NUMBER **vals)
{
	long base;	/* output base/mode */
	long oldbase=0;	/* output base/mode */

	/* deal with just a query */
	if (count != 1) {
		return base_value(conf->outmode);
	}

	/* deal with the specal modes first */
	if (qisfrac(vals[0])) {
		return base_value(math_setmode(MODE_FRAC));
	}
	if (vals[0]->num.len > 64/BASEB) {
		return base_value(math_setmode(MODE_EXP));
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
	default:
		math_error("Unsupported base");
		/*NOTREACHED*/
		break;
	}

	/* return the old base */
	return base_value(oldbase);
}


/*
 * return a numerical 'value' of the mode/base
 */
static NUMBER *
base_value(long mode)
{
	NUMBER *result;

	/* return the old base */
	switch (mode) {
	case MODE_DEFAULT:
		switch (conf->outmode) {
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
		case MODE_HEX:
			result = itoq(16);
			break;
		case MODE_OCTAL:
			result = itoq(8);
			break;
		case MODE_BINARY:
			result = itoq(2);
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
	case MODE_HEX:
		result = itoq(16);
		break;
	case MODE_OCTAL:
		result = itoq(8);
		break;
	case MODE_BINARY:
		result = itoq(2);
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

	/*
	 * disable custom functions unless -C was given
	 */
	if (!allow_custom) {
		fprintf(stderr,
#if defined(CUSTOM)
		    "%sCalc must be run with a -C argument to "
		    "use custom function\n",
#else /* CUSTOM */
		    "%sCalc was built with custom functions disabled\n",
#endif /* CUSTOM */
		    (conf->tab_ok ? "\t" : ""));
		return error_value(E_CUSTOM_ERROR);
	}

	/*
	 * perform the custom operation
	 */
	if (count <= 0) {
		/* perform the usage function function */
		showcustom();
		result.v_type = V_NULL;
	} else {
		/* firewall */
		if (vals[0]->v_type != V_STR) {
			math_error("custom: 1st arg not a string name");
			/*NOTREACHED*/
		}

		/* perform the custom function */
		result = custom(vals[0]->v_str->s_str, count-1, vals+1);
	}

	/*
	 * return the custom result
	 */
	return result;
}


static VALUE
f_blk(int count, VALUE **vals)
{
	int len;		/* number of octets to malloc */
	int chunk;		/* block chunk size */
	VALUE result;
	int id;
	VALUE *vp;
	int type;

	vp = *vals;
	type = 0;
	result.v_subtype = V_NOSUBTYPE;
	if (count > 0) {
		type = vp->v_type;
		if (type == V_STR || type == V_NBLOCK || type == V_BLOCK) {
			vals++;
			count--;
		}
	}

	len = -1;		/* signal to use old or zero len */
	chunk = -1;		/* signal to use old or default chunksize */
	if (count > 0 && vals[0]->v_type != V_NULL) {
		/* parse len */
		if (vals[0]->v_type != V_NUM || qisfrac(vals[0]->v_num))
			return error_value(E_BLK1);
		if (qisneg(vals[0]->v_num) || zge31b(vals[0]->v_num->num))
			return error_value(E_BLK2);
		len = qtoi(vals[0]->v_num);
	}
	if (count > 1 && vals[1]->v_type != V_NULL) {
		/* parse chunk */
		if (vals[1]->v_type != V_NUM || qisfrac(vals[1]->v_num))
			return error_value(E_BLK3);
		if (qisneg(vals[1]->v_num) || zge31b(vals[1]->v_num->num))
			return error_value(E_BLK4);
		chunk = qtoi(vals[1]->v_num);
	}

	if (type == V_STR) {
		result.v_type = V_NBLOCK;
		id = findnblockid(vp->v_str->s_str);
		if (id < 0) {
			/* create new named block */
			result.v_nblock = createnblock(vp->v_str->s_str,
				 len, chunk);
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
	result.v_type = V_BLOCK;
	return result;
}


static VALUE
f_blkfree(VALUE *vp)
{
	int id;
	VALUE result;

	id = 0;
	switch (vp->v_type) {
		case V_NBLOCK:
			id = vp->v_nblock->id;
			break;
		case V_STR:
			id = findnblockid(vp->v_str->s_str);
			if (id < 0)
				return error_value(E_BLKFREE1);
			break;
		case V_NUM:
			if (qisfrac(vp->v_num) || qisneg(vp->v_num))
				return error_value(E_BLKFREE2);
			if (zge31b(vp->v_num->num))
				return error_value(E_BLKFREE3);
			id = qtoi(vp->v_num);
			break;
		default:
			return error_value(E_BLKFREE4);
	}
	id = removenblock(id);
	if (id)
		return error_value(id);
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_blocks(int count, VALUE **vals)
{
	NBLOCK *nblk;
	VALUE result;
	int id;

	if (count == 0) {
		result.v_type = V_NUM;
		result.v_num = itoq((long) countnblocks());
		return result;
	}
	if (vals[0]->v_type != V_NUM || qisfrac(vals[0]->v_num))
		return error_value(E_BLOCKS1);
	id = (int) qtoi(vals[0]->v_num);

	nblk = findnblock(id);

	if (nblk == NULL)
		return error_value(E_BLOCKS2);
	else {
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

	result.v_type = V_NULL;
	while (count-- > 0) {
		val = *vals++;
		if (val->v_type == V_ADDR)
			freevalue(val->v_addr);
	}
	return result;
}

static VALUE
f_freeglobals(void)
{
	VALUE result;

	freeglobals();
	result.v_type = V_NULL;
	return result;
}

static VALUE
f_freeredc(void)
{
	VALUE result;
	freeredcdata();
	result.v_type = V_NULL;
	return result;
}


static VALUE
f_freestatics(void)
{
	VALUE result;

	freestatics();
	result.v_type = V_NULL;
	return result;
}


/*
 * f_copy - copy consecutive items between values
 *
 *	copy(src, dst [, ssi [, num [, dsi]]])
 *
 * Copy 'num' consecutive items from 'src' with index 'ssi' to
 * 'dest', starting at position with index 'dsi'.
 */
static VALUE
f_copy(int count, VALUE **vals)
{
	long ssi = 0;	/* source start index */
	long num = -1;	/* number of items to copy (-1 ==> all) */
	long dsi = -1;	/* destination start index, -1 ==> default */
	int errtype;	/* error type if unable to perform copy */
	VALUE result;	/* null if successful */

	/*
	 * parse args
	 */
	switch(count) {
	case 5:
		/* parse dsi */
		if (vals[4]->v_type != V_NULL) {
			if (vals[4]->v_type != V_NUM ||
			    qisfrac(vals[4]->v_num) ||
			    qisneg(vals[4]->v_num)) {
				return error_value(E_COPY6);
			}
			if (zge31b(vals[4]->v_num->num)) {
				return error_value(E_COPY7);
			}
			dsi = qtoi(vals[4]->v_num);
		}
		/*FALLTHRU*/

	case 4:
		/* parse num */
		if (vals[3]->v_type != V_NULL) {
			if (vals[3]->v_type != V_NUM ||
			    qisfrac(vals[3]->v_num) ||
			    qisneg(vals[3]->v_num)) {
				return error_value(E_COPY1);
			}
			if (zge31b(vals[3]->v_num->num)) {
				return error_value(E_COPY2);
			}
			num = qtoi(vals[3]->v_num);
		}
		/*FALLTHRU*/

	case 3:
		/* parse ssi */
		if (vals[2]->v_type != V_NULL) {
			if (vals[2]->v_type != V_NUM ||
			    qisfrac(vals[2]->v_num) ||
			    qisneg(vals[2]->v_num)) {
				return error_value(E_COPY4);
			}
			if (zge31b(vals[2]->v_num->num)) {
				return error_value(E_COPY5);
			}
			ssi = qtoi(vals[2]->v_num);
		}
		break;
	}

	/*
	 * copy
	 */
	errtype = copystod(vals[0], ssi, num, vals[1], dsi);
	if (errtype > 0)
		return error_value(errtype);
	result.v_type = V_NULL;
	return result;
}


/*
 * f_blkcpy - copy consecutive items between values
 *
 *	copy(dst, src [, num [, dsi [, ssi]]])
 *
 * Copy 'num' consecutive items from 'src' with index 'ssi' to
 * 'dest', starting at position with index 'dsi'.
 */
static VALUE
f_blkcpy(int count, VALUE **vals)
{
	VALUE *args[5];		/* args to re-order */
	VALUE null_value;	/* dummy argument */

	/*
	 * parse args into f_copy order
	 */
	args[0] = vals[1];
	args[1] = vals[0];
	switch(count) {
	case 5:
		args[2] = vals[4];
		args[4] = vals[3];
		args[3] = vals[2];
		break;
	case 4:
		count = 5;
		args[4] = vals[3];
		args[3] = vals[2];
		null_value.v_type = V_NULL;
		args[2] = &null_value;
		break;
	case 3:
		count = 4;
		args[3] = vals[2];
		null_value.v_type = V_NULL;
		args[2] = &null_value;
		break;
	}

	/*
	 * copy
	 */
	return f_copy(count, args);
}


static VALUE
f_sha(int count, VALUE **vals)
{
	VALUE result;
	HASH *state;		/* pointer to hash state to use */
	int i;			/* vals[i] to hash */

	state = NULL;

	/*
	 * arg check
	 */
	if (count == 0) {

		/* return an initial hash state */
		result.v_type = V_HASH;
		result.v_hash = hash_init(SHS_HASH_TYPE, NULL);

	} else if (count == 1 && vals[0]->v_type == V_HASH &&
		vals[0]->v_hash->hashtype == SHS_HASH_TYPE) {

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
		if (vals[0]->v_type == V_HASH &&
			vals[0]->v_hash->hashtype == SHS_HASH_TYPE) {
			state = hash_copy(vals[0]->v_hash);
			i = 1;

		/*
		 * otherwise use the default initial state
		 */
		} else {
			state = hash_init(SHS_HASH_TYPE, NULL);
			i = 0;
		}

		/*
		 * hash the remaining values
		 */
		do {
			state = hash_value(SHS_HASH_TYPE, vals[i], state);
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
f_sha1(int count, VALUE **vals)
{
	VALUE result;
	HASH *state;		/* pointer to hash state to use */
	int i;			/* vals[i] to hash */

	/*
	 * arg check
	 */
	if (count == 0) {

		/* return an initial hash state */
		result.v_type = V_HASH;
		result.v_hash = hash_init(SHS1_HASH_TYPE, NULL);

	} else if (count == 1 && vals[0]->v_type == V_HASH &&
		vals[0]->v_hash->hashtype == SHS1_HASH_TYPE) {

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
		if (vals[0]->v_type == V_HASH &&
			vals[0]->v_hash->hashtype == SHS1_HASH_TYPE) {
			state = hash_copy(vals[0]->v_hash);
			i = 1;

		/*
		 * otherwise use the default initial state
		 */
		} else {
			state = hash_init(SHS1_HASH_TYPE, NULL);
			i = 0;
		}

		/*
		 * hash the remaining values
		 */
		do {
			state = hash_value(SHS1_HASH_TYPE, vals[i], state);
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
f_md5(int count, VALUE **vals)
{
	VALUE result;
	HASH *state;		/* pointer to hash state to use */
	int i;			/* vals[i] to hash */

	state = NULL;

	/*
	 * arg check
	 */
	if (count == 0) {

		/* return an initial hash state */
		result.v_type = V_HASH;
		result.v_hash = hash_init(MD5_HASH_TYPE, NULL);

	} else if (count == 1 && vals[0]->v_type == V_HASH &&
		vals[0]->v_hash->hashtype == MD5_HASH_TYPE) {

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
		if (vals[0]->v_type == V_HASH &&
			vals[0]->v_hash->hashtype == MD5_HASH_TYPE) {
			state = hash_copy(vals[0]->v_hash);
			i = 1;

		/*
		 * otherwise use the default initial state
		 */
		} else {
			state = hash_init(MD5_HASH_TYPE, NULL);
			i = 0;
		}

		/*
		 * hash the remaining values
		 */
		do {
			state = hash_value(MD5_HASH_TYPE, vals[i], state);
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


#endif /* !FUNCLIST */


/*
 * builtins - List of primitive built-in functions
 *
 * NOTE:  This table is also used by the help/Makefile builtin rule to
 *	  form the builtin help file.  This rule will cause a sed script
 *	  to strip this table down into a just the information needed
 *	  to print builtin function list: b_name, b_minargs, b_maxargs
 *	  and b_desc.  All other struct elements will be converted to 0.
 *	  The sed script expects to find entries of the form:
 *
 *		{"...", number, number, stuff, stuff, stuff, stuff,
 *		 "...."},
 *
 *	  please keep this table in that form.
 *
 *	  For nice output, when the description of function (b_desc)
 *	  gets too long (extends into col 79) you should chop the
 *	  line and add "\n\t\t    ", thats newline, 2 tabs a 4 spaces.
 *	  For example the description:
 *
 *		... very long description that goes beyond col 79
 *
 *	  should be written as:
 *
 *		"... very long description that\n\t\t    goes beyond col 79"},
 *
 * fields:
 *	b_name		name of built-in function
 *	b_minargs	minimum number of arguments
 *	b_maxargs	maximum number of arguments
 *	b_flags		special handling flags
 *	b_opcode	opcode which makes the call quick
 *	b_numfunc	routine to calculate numeric function
 *	b_valfunc	routine to calculate general values
 *	b_desc		description of function
 */
static CONST struct builtin builtins[] = {
	{"abs", 1, 2, 0, OP_ABS, 0, 0,
	 "absolute value within accuracy b"},
	{"access", 1, 2, 0, OP_NOP, 0, f_access,
	 "determine accessibility of file a for mode b"},
	{"acos", 1, 2, 0, OP_NOP, 0, f_acos,
	 "arccosine of a within accuracy b"},
	{"acosh", 1, 2, 0, OP_NOP, 0, f_acosh,
	 "inverse hyperbolic cosine of a within accuracy b"},
	{"acot", 1, 2, 0, OP_NOP, 0, f_acot,
	 "arccotangent of a within accuracy b"},
	{"acoth", 1, 2, 0, OP_NOP, 0, f_acoth,
	 "inverse hyperbolic cotangent of a within accuracy b"},
	{"acsc", 1, 2, 0, OP_NOP, 0, f_acsc,
	 "arccosecant of a within accuracy b"},
	{"acsch", 1, 2, 0, OP_NOP, 0, f_acsch,
	 "inverse csch of a within accuracy b"},
	{"agd", 1, 2, 0, OP_NOP, 0, f_agd,
	 "inverse gudermannian function"},
	{"append", 1, IN, FA, OP_NOP, 0, f_listappend,
	 "append values to end of list"},
	{"appr", 1, 3, 0, OP_NOP, 0, f_appr,
	 "approximate a by multiple of b using rounding c"},
	{"arg", 1, 2, 0, OP_NOP, 0, f_arg,
	 "argument (the angle) of complex number"},
	{"asec", 1, 2, 0, OP_NOP, 0, f_asec,
	 "arcsecant of a within accuracy b"},
	{"asech", 1, 2, 0, OP_NOP, 0, f_asech,
	 "inverse hyperbolic secant of a within accuracy b"},
	{"asin", 1, 2, 0, OP_NOP, 0, f_asin,
	 "arcsine of a within accuracy b"},
	{"asinh", 1, 2, 0, OP_NOP, 0, f_asinh,
	 "inverse hyperbolic sine of a within accuracy b"},
	{"assoc", 0, 0, 0, OP_NOP, 0, f_assoc,
	 "create new association array"},
	{"atan", 1, 2, 0, OP_NOP, 0, f_atan,
	 "arctangent of a within accuracy b"},
	{"atan2", 2, 3, FE, OP_NOP, qatan2, 0,
	 "angle to point (b,a) within accuracy c"},
	{"atanh", 1, 2, 0, OP_NOP, 0, f_atanh,
	 "inverse hyperbolic tangent of a within accuracy b"},
	{"avg", 0, IN, 0, OP_NOP, 0, f_avg,
	 "arithmetic mean of values"},
	{"base", 0, 1, 0, OP_NOP, f_base, 0,
	 "set default output base"},
	{"bit", 2, 2, 0, OP_BIT, 0, 0,
	 "whether bit b in value a is set"},
	{"blk", 0, 3, 0, OP_NOP, 0, f_blk,
	 "block with or without name, octet number, chunksize"},
	{"blkcpy", 2, 5, 0, OP_NOP, 0, f_blkcpy,
	 "copy value to/from a block: blkcpy(d,s,len,di,si)"},
	{"blkfree", 1, 1, 0, OP_NOP, 0, f_blkfree,
	 "free all storage from a named block"},
	{"blocks", 0, 1, 0, OP_NOP, 0, f_blocks,
	 "named block with specified index, or null value"},
	{"bround", 1, 3, 0, OP_NOP, 0, f_bround,
	 "round value a to b number of binary places"},
	{"btrunc", 1, 2, 0, OP_NOP, f_btrunc, 0,
	 "truncate a to b number of binary places"},
	{"ceil", 1, 1, 0, OP_NOP, 0, f_ceil,
	 "smallest integer greater than or equal to number"},
	{"cfappr", 1, 3, 0, OP_NOP, f_cfappr, 0,
	 "approximate a within accuracy b using\n\t\t    continued fractions"},
	{"cfsim", 1, 2, 0, OP_NOP, f_cfsim, 0,
	 "simplify number using continued fractions"},
	{"char", 1, 1, 0, OP_NOP, 0, f_char,
	 "character corresponding to integer value"},
	{"cmdbuf", 0, 0, 0, OP_NOP, 0, f_cmdbuf,
	 "command buffer"},
	{"cmp", 2, 2, 0, OP_CMP, 0, 0,
	 "compare values returning -1, 0, or 1"},
	{"comb", 2, 2, 0, OP_NOP, qcomb, 0,
	 "combinatorial number a!/b!(a-b)!"},
	{"config", 1, 2, 0, OP_SETCONFIG, 0, 0,
	 "set or read configuration value"},
	{"conj", 1, 1, 0, OP_CONJUGATE, 0, 0,
	 "complex conjugate of value"},
	{"copy", 2, 5, 0, OP_NOP, 0, f_copy,
	 "copy value to/from a block: copy(s,d,len,si,di)"},
	{"cos", 1, 2, 0, OP_NOP, 0, f_cos,
	 "cosine of value a within accuracy b"},
	{"cosh", 1, 2, 0, OP_NOP, 0, f_cosh,
	 "hyperbolic cosine of a within accuracy b"},
	{"cot", 1, 2, 0, OP_NOP, 0, f_cot,
	 "cotangent of a within accuracy b"},
	{"coth", 1, 2, 0, OP_NOP, 0, f_coth,
	 "hyperbolic cotangent of a within accuracy b"},
	{"count", 2, 2, 0, OP_NOP, 0, f_count,
	 "count listr/matrix elements satisfying some condition"},
	{"cp", 2, 2, 0, OP_NOP, 0, f_cp,
	 "cross product of two vectors"},
	{"csc", 1, 2, 0, OP_NOP, 0, f_csc,
	 "cosecant of a within accuracy b"},
	{"csch", 1, 2, 0, OP_NOP, 0, f_csch,
	 "hyperbolic cosecant of a within accuracy b"},
	{"ctime", 0, 0, 0, OP_NOP, 0, f_ctime,
	 "date and time as string"},
	{"custom", 0, IN, 0, OP_NOP, 0, f_custom,
	 "custom builtin function interface"},
	{"delete", 2, 2, FA, OP_NOP, 0, f_listdelete,
	 "delete element from list a at position b"},
	{"den", 1, 1, 0, OP_DENOMINATOR, qden, 0,
	 "denominator of fraction"},
	{"det", 1, 1, 0, OP_NOP, 0, f_det,
	 "determinant of matrix"},
	{"digit", 2, 2, 0, OP_NOP, f_digit, 0,
	 "digit at specified decimal place of number"},
	{"digits", 1, 1, 0, OP_NOP, f_digits, 0,
	 "number of digits in number"},
	{"display", 0, 1, 0, OP_NOP, 0, f_display,
	 "number of decimal digits for displaying numbers"},
	{"dp", 2, 2, 0, OP_NOP, 0, f_dp,
	 "dot product of two vectors"},
	{"epsilon", 0, 1, 0, OP_SETEPSILON, 0, 0,
	 "set or read allowed error for real calculations"},
	{"errcount", 0, 1, 0, OP_NOP, 0, f_errcount,
	 "set or read error count"},
	{"errmax", 0, 1, 0, OP_NOP, 0, f_errmax,
	 "set or read maximum for error count"},
	{"errno", 0, 1, 0, OP_NOP, 0, f_errno,
	 "set or read calc_errno"},
	{"error", 0, 1, 0, OP_NOP, 0, f_error,
	 "generate error value"},
	{"eval", 1, 1, 0, OP_NOP, 0, f_eval,
	 "evaluate expression from string to value"},
	{"exp", 1, 2, 0, OP_NOP, 0, f_exp,
	 "exponential of value a within accuracy b"},
	{"factor", 1, 3, 0, OP_NOP, f_factor, 0,
	 "lowest prime factor < b of a, return c if error"},
	{"fcnt", 2, 2, 0, OP_NOP, f_faccnt, 0,
	 "count of times one number divides another"},
	{"fib", 1, 1, 0, OP_NOP, qfib, 0,
	 "Fibonacci number F(n)"},
	{"forall", 2, 2, 0, OP_NOP, 0, f_forall,
	 "do function for all elements of list or matrix"},
	{"frem", 2, 2, 0, OP_NOP, qfacrem, 0,
	 "number with all occurrences of factor removed"},
	{"fact", 1, 1, 0, OP_NOP, 0, f_fact,
	 "factorial"},
	{"fclose", 0, IN, 0, OP_NOP, 0, f_fclose,
	 "close file"},
	{"feof", 1, 1, 0, OP_NOP, 0, f_feof,
	 "whether EOF reached for file"},
	{"ferror", 1, 1, 0, OP_NOP, 0, f_ferror,
	 "whether error occurred for file"},
	{"fflush", 0, IN, 0, OP_NOP, 0, f_fflush,
	 "flush output to file(s)"},
	{"fgetc", 1, 1, 0, OP_NOP, 0, f_fgetc,
	 "read next char from file"},
	{"fgetfield", 1, 1, 0, OP_NOP, 0, f_fgetfield,
	 "read next white-space delimited field from file"},
	{"fgetline", 1, 1, 0, OP_NOP, 0, f_fgetline,
	 "read next line from file, newline removed"},
	{"fgets", 1, 1, 0, OP_NOP, 0, f_fgets,
	 "read next line from file, newline is kept"},
	{"fgetstr", 1, 1, 0, OP_NOP, 0, f_fgetstr,
	 "read next null-terminated string from file, null character is kept"},
	{"files", 0, 1, 0, OP_NOP, 0, f_files,
	 "return opened file or max number of opened files"},
	{"floor", 1, 1, 0, OP_NOP, 0, f_floor,
	 "greatest integer less than or equal to number"},
	{"fopen", 2, 2, 0, OP_NOP, 0, f_fopen,
	 "open file name a in mode b"},
	{"fprintf", 2, IN, 0, OP_NOP, 0, f_fprintf,
	 "print formatted output to opened file"},
	{"fputc", 2, 2, 0, OP_NOP, 0, f_fputc,
	 "write a character to a file"},
	{"fputs", 2, IN, 0, OP_NOP, 0, f_fputs,
	 "write one or more strings to a file"},
	{"fputstr", 2, IN, 0, OP_NOP, 0, f_fputstr,
	 "write one or more null-terminated strings to a file"},
	{"free", 0, IN, FA, OP_NOP, 0, f_free,
	 "free listed or all global variables"},
	{"freeglobals", 0, 0, 0, OP_NOP, 0, f_freeglobals,
	 "free all global and visible static variables"},
	{"freeredc", 0, 0, 0, OP_NOP, 0, f_freeredc,
	 "free redc data cache"},
	{"freestatics", 0, 0, 0, OP_NOP, 0, f_freestatics,
	 "free all unscoped static variables"},
	{"freopen", 2, 3, 0, OP_NOP, 0, f_freopen,
	 "reopen a file stream to a named file"},
	{"fscan", 2, IN, FA, OP_NOP, 0, f_fscan,
	 "scan a file for assignments to one or more variables"},
	{"fscanf", 2, IN, FA, OP_NOP, 0, f_fscanf,
	 "formatted scan of a file for assignment to one or more variables"},
	{"fseek", 2, 3, 0, OP_NOP, 0, f_fseek,
	 "seek to position b (offset from c) in file a"},
	{"fsize", 1, 1, 0, OP_NOP, 0, f_fsize,
	 "return the size of the file"},
	{"ftell", 1, 1, 0, OP_NOP, 0, f_ftell,
	 "return the file position"},
	{"frac", 1, 1, 0, OP_FRAC, qfrac, 0,
	 "fractional part of value"},
	{"gcd", 1, IN, 0, OP_NOP, f_gcd, 0,
	 "greatest common divisor"},
	{"gcdrem", 2, 2, 0, OP_NOP, qgcdrem, 0,
	 "a divided repeatedly by gcd with b"},
	{"gd", 1, 2, 0, OP_NOP, 0, f_gd,
	 "gudermannian function"},
	{"getenv", 1, 1, 0, OP_NOP, 0, f_getenv,
	 "value of environment variable (or NULL)"},
	{"hash", 1, IN, 0, OP_NOP, 0, f_hash,
	 "return non-negative hash value for one or\n\t\t    more values"},
	{"head", 2, 2, 0, OP_NOP, 0, f_head,
	 "return list of specified number at head of a list"},
	{"highbit", 1, 1, 0, OP_HIGHBIT, 0, 0,
	 "high bit number in base 2 representation"},
	{"hmean", 0, IN, 0, OP_NOP, 0, f_hmean,
	 "harmonic mean of values"},
	{"hnrmod", 4, 4, 0, OP_NOP, f_hnrmod, 0,
	 "v mod h*2^n+r, h>0, n>0, r = -1, 0 or 1"},
	{"hypot", 2, 3, FE, OP_NOP, qhypot, 0,
	 "hypotenuse of right triangle within accuracy c"},
	{"ilog", 2, 2, 0, OP_NOP, f_ilog, 0,
	 "integral log of one number with another"},
	{"ilog10", 1, 1, 0, OP_NOP, f_ilog10, 0,
	 "integral log of a number base 10"},
	{"ilog2", 1, 1, 0, OP_NOP, f_ilog2, 0,
	 "integral log of a number base 2"},
	{"im", 1, 1, 0, OP_IM, 0, 0,
	 "imaginary part of complex number"},
	{"insert", 2, IN, FA, OP_NOP, 0, f_listinsert,
	 "insert values c ... into list a at position b"},
	{"int", 1, 1, 0, OP_INT, qint, 0,
	 "integer part of value"},
	{"inverse", 1, 1, 0, OP_INVERT, 0, 0,
	 "multiplicative inverse of value"},
	{"iroot", 2, 2, 0, OP_NOP, qiroot, 0,
	 "integer b'th root of a"},
	{"isassoc", 1, 1, 0, OP_ISASSOC, 0, 0,
	 "whether a value is an association"},
	{"isatty", 1, 1, 0, OP_NOP, 0, f_isatty,
	 "whether a file is a tty"},
	{"isblk", 1, 1, 0, OP_ISBLK, 0, 0,
	 "whether a value is a block"},
	{"isconfig", 1, 1, 0, OP_ISCONFIG, 0, 0,
	 "whether a value is a config state"},
	{"isdefined", 1, 1, 0, OP_ISDEFINED, 0, 0,
	 "whether a string names a function"},
	{"iserror", 1, 1, 0, OP_NOP, 0, f_iserror,
	 "where a value is an error"},
	{"iseven", 1, 1, 0, OP_ISEVEN, 0, 0,
	 "whether a value is an even integer"},
	{"isfile", 1, 1, 0, OP_ISFILE, 0, 0,
	 "whether a value is a file"},
	{"ishash", 1, 1, 0, OP_ISHASH, 0, 0,
	 "whether a value is a hash state"},
	{"isident", 1, 1, 0, OP_NOP, 0, f_isident,
	 "returns 1 if identity matrix"},
	{"isint", 1, 1, 0, OP_ISINT, 0, 0,
	 "whether a value is an integer"},
	{"islist", 1, 1, 0, OP_ISLIST, 0, 0,
	 "whether a value is a list"},
	{"ismat", 1, 1, 0, OP_ISMAT, 0, 0,
	 "whether a value is a matrix"},
	{"ismult", 2, 2, 0, OP_NOP, f_ismult, 0,
	 "whether a is a multiple of b"},
	{"isnull", 1, 1, 0, OP_ISNULL, 0, 0,
	 "whether a value is the null value"},
	{"isnum", 1, 1, 0, OP_ISNUM, 0, 0,
	 "whether a value is a number"},
	{"isobj", 1, 1, 0, OP_ISOBJ, 0, 0,
	 "whether a value is an object"},
	{"isobjtype", 1, 1, 0, OP_ISOBJTYPE, 0,0,
	 "whether a string names an object type"},
	{"isodd", 1, 1, 0, OP_ISODD, 0, 0,
	 "whether a value is an odd integer"},
	{"isoctet", 1, 1, 0, OP_ISOCTET, 0, 0,
	 "whether a value is an octet"},
	{"isprime", 1, 2, 0, OP_NOP, f_isprime, 0,
	 "whether a is a small prime, return b if error"},
	{"isptr", 1, 1, 0, OP_ISPTR, 0, 0,
	 "whether a value is a pointer"},
	{"isqrt", 1, 1, 0, OP_NOP, qisqrt, 0,
	 "integer part of square root"},
	{"isrand", 1, 1, 0, OP_ISRAND, 0, 0,
	 "whether a value is a additive 55 state"},
	{"israndom", 1, 1, 0, OP_ISRANDOM, 0, 0,
	 "whether a value is a Blum state"},
	{"isreal", 1, 1, 0, OP_ISREAL, 0, 0,
	 "whether a value is a real number"},
	{"isrel", 2, 2, 0, OP_NOP, f_isrel, 0,
	 "whether two numbers are relatively prime"},
	{"isstr", 1, 1, 0, OP_ISSTR, 0, 0,
	 "whether a value is a string"},
	{"issimple", 1, 1, 0, OP_ISSIMPLE, 0, 0,
	 "whether value is a simple type"},
	{"issq", 1, 1, 0, OP_NOP, f_issquare, 0,
	 "whether or not number is a square"},
	{"istype", 2, 2, 0, OP_ISTYPE, 0, 0,
	 "whether the type of a is same as the type of b"},
	{"jacobi", 2, 2, 0, OP_NOP, qjacobi, 0,
	 "-1 => a is not quadratic residue mod b\n\t\t    1 => b is composite, or a is quad residue of b"},
	{"join", 1, IN, 0, OP_NOP, 0, f_join,
	 "join one or more lists into one list"},
	{"lcm", 1, IN, 0, OP_NOP, f_lcm, 0,
	 "least common multiple"},
	{"lcmfact", 1, 1, 0, OP_NOP, qlcmfact, 0,
	 "lcm of all integers up till number"},
	{"lfactor", 2, 2, 0, OP_NOP, qlowfactor, 0,
	 "lowest prime factor of a in first b primes"},
	{"links", 1, 1, 0, OP_LINKS, 0, 0,
	 "links to number or string value"},
	{"list", 0, IN, 0, OP_NOP, 0, f_list,
	 "create list of specified values"},
	{"ln", 1, 2, 0, OP_NOP, 0, f_ln,
	 "natural logarithm of value a within accuracy b"},
	{"lowbit", 1, 1, 0, OP_LOWBIT, 0, 0,
	 "low bit number in base 2 representation"},
	{"ltol", 1, 2, FE, OP_NOP, f_legtoleg, 0,
	 "leg-to-leg of unit right triangle (sqrt(1 - a^2))"},
	{"makelist", 1, 1, 0, OP_NOP, 0, f_makelist,
	 "create a list with a null elements"},
	{"matdim", 1, 1, 0, OP_NOP, 0, f_matdim,
	 "number of dimensions of matrix"},
	{"matfill", 2, 3, FA, OP_NOP, 0, f_matfill,
	 "fill matrix with value b (value c on diagonal)"},
	{"matmax", 2, 2, 0, OP_NOP, 0, f_matmax,
	 "maximum index of matrix a dim b"},
	{"matmin", 2, 2, 0, OP_NOP, 0, f_matmin,
	 "minimum index of matrix a dim b"},
	{"matsum", 1, 1, 0, OP_NOP, 0, f_matsum,
	 "sum the numeric values in a matrix"},
	{"mattrace", 1, 1, 0, OP_NOP, 0, f_mattrace,
	 "return the trace of a square matrix"},
	{"mattrans", 1, 1, 0, OP_NOP, 0, f_mattrans,
	 "transpose of matrix"},
	{"max", 0, IN, 0, OP_NOP, 0, f_max,
	 "maximum value"},
	{"md5", 0, IN, 0, OP_NOP, 0, f_md5,
	 "MD5 Hash Algorithm"},
	{"memsize", 1, 1, 0, OP_NOP, 0, f_memsize,
	 "number of octets used by the value, including overhead"},
	{"meq", 3, 3, 0, OP_NOP, f_meq, 0,
	 "whether a and b are equal modulo c"},
	{"min", 0, IN, 0, OP_NOP, 0, f_min,
	 "minimum value"},
	{"minv", 2, 2, 0, OP_NOP, qminv, 0,
	 "inverse of a modulo b"},
	{"mmin", 2, 2, 0, OP_NOP, 0, f_mmin,
	 "a mod b value with smallest abs value"},
	{"mne", 3, 3, 0, OP_NOP, f_mne, 0,
	 "whether a and b are not equal modulo c"},
	{"mod", 2, 3, 0, OP_NOP, 0, f_mod,
	 "residue of a modulo b, rounding type c"},
	{"modify", 2, 2, FA, OP_NOP, 0, f_modify,
	 "modify elements of a list or matrix"},
	{"name", 1, 1, 0, OP_NOP, 0, f_name,
	 "name assigned to block or file"},
	{"near", 2, 3, 0, OP_NOP, f_near, 0,
	 "sign of (abs(a-b) - c)"},
	{"newerror", 0, 1, 0, OP_NOP, 0, f_newerror,
	 "create new error type with message a"},
	{"nextcand", 1, 5, 0, OP_NOP, f_nextcand, 0,
	 "smallest value == d mod e > a, ptest(a,b,c) true"},
	{"nextprime", 1, 2, 0, OP_NOP, f_nprime, 0,
	 "return next small prime, return b if err"},
	{"norm", 1, 1, 0, OP_NORM, 0, 0,
	 "norm of a value (square of absolute value)"},
	{"null", 0, IN, 0, OP_NOP, 0, f_null,
	 "null value"},
	{"num", 1, 1, 0, OP_NUMERATOR, qnum, 0,
	 "numerator of fraction"},
	{"ord", 1, 1, 0, OP_NOP, 0, f_ord,
	 "integer corresponding to character value"},
	{"param", 1, 1, 0, OP_ARGVALUE, 0, 0,
	 "value of parameter n (or parameter count if n\n\t\t    is zero)"},
	{"perm", 2, 2, 0, OP_NOP, qperm, 0,
	 "permutation number a!/(a-b)!"},
	{"prevcand", 1, 5, 0, OP_NOP, f_prevcand, 0,
	 "largest value == d mod e < a, ptest(a,b,c) true"},
	{"prevprime", 1, 2, 0, OP_NOP, f_pprime, 0,
	 "return previous small prime, return b if err"},
	{"pfact", 1, 1, 0, OP_NOP, qpfact, 0,
	 "product of primes up till number"},
	{"pi", 0, 1, FE, OP_NOP, qpi, 0,
	 "value of pi accurate to within epsilon"},
	{"pix", 1, 2, 0, OP_NOP, f_pix, 0,
	 "number of primes <= a < 2^32, return b if error"},
	{"places", 1, 1, 0, OP_NOP, f_places, 0,
	 "places after decimal point (-1 if infinite)"},
	{"pmod", 3, 3, 0, OP_NOP, qpowermod,0,
	 "mod of a power (a ^ b (mod c))"},
	{"polar", 2, 3, 0, OP_NOP, 0, f_polar,
	 "complex value of polar coordinate (a * exp(b*1i))"},
	{"poly", 1, IN, 0, OP_NOP, 0, f_poly,
	 "evaluates a polynomial given its coefficients or coefficient-list"},
	{"pop", 1, 1, FA, OP_NOP, 0, f_listpop,
	 "pop value from front of list"},
	{"popcnt", 1, 2, 0, OP_NOP, f_popcnt, 0,
	 "number of bits in a that match b (or 1)"},
	{"power", 2, 3, 0, OP_NOP, 0, f_power,
	 "value a raised to the power b within accuracy c"},
	{"protect", 1, 2, FA, OP_NOP, 0, f_protect,
	 "read or set protection level for variable"},
	{"ptest", 1, 3, 0, OP_NOP, f_primetest, 0,
	 "probabilistic primality test"},
	{"printf", 1, IN, 0, OP_NOP, 0, f_printf,
	 "print formatted output to stdout"},
	{"prompt", 1, 1, 0, OP_NOP, 0, f_prompt,
	 "prompt for input line using value a"},
	{"push", 1, IN, FA, OP_NOP, 0, f_listpush,
	 "push values onto front of list"},
	{"putenv", 1, 2, 0, OP_NOP, 0, f_putenv,
	 "define an environment variable"},
	{"quo", 2, 3, 0, OP_NOP, 0, f_quo,
	 "integer quotient of a by b, rounding type c"},
	{"quomod", 4, 4, 0, OP_QUOMOD, 0, 0,
	 "set c and d to quotient and remainder of a\n\t\t    divided by b"},
	{"rand", 0, 2, 0, OP_NOP, f_rand, 0,
	 "additive 55 random number [0,2^64), [0,a), or [a,b)"},
	{"randbit", 0, 1, 0, OP_NOP, f_randbit, 0,
	 "additive 55 random number [0,2^a)"},
	{"random", 0, 2, 0, OP_NOP, f_random, 0,
	 "Blum-Blum-Shub random number [0,2^64), [0,a), or [a,b)"},
	{"randombit", 0, 1, 0, OP_NOP, f_randombit, 0,
	 "Blum-Blum-Sub random number [0,2^a)"},
	{"randperm", 1, 1, 0, OP_NOP, 0, f_randperm,
	 "random permutation of a list or matrix"},
	{"rcin", 2, 2, 0, OP_NOP, qredcin, 0,
	 "convert normal number a to REDC number mod b"},
	{"rcmul", 3, 3, 0, OP_NOP, qredcmul, 0,
	 "multiply REDC numbers a and b mod c"},
	{"rcout", 2, 2, 0, OP_NOP, qredcout, 0,
	 "convert REDC number a mod b to normal number"},
	{"rcpow", 3, 3, 0, OP_NOP, qredcpower, 0,
	 "raise REDC number a to power b mod c"},
	{"rcsq", 2, 2, 0, OP_NOP, qredcsquare, 0,
	 "square REDC number a mod b"},
	{"re", 1, 1, 0, OP_RE, 0, 0,
	 "real part of complex number"},
	{"remove", 1, 1, FA, OP_NOP, 0, f_listremove,
	 "remove value from end of list"},
	{"reverse", 1, 1, 0, OP_NOP, 0, f_reverse,
	 "reverse a copy of a matrix or list"},
	{"rewind", 0, IN, 0, OP_NOP, 0, f_rewind,
	 "rewind file(s)"},
	{"rm", 1, IN, 0, OP_NOP, 0, f_rm,
	 "remove file(s), -f turns off no-such-file errors"},
	{"root", 2, 3, 0, OP_NOP, 0, f_root,
	 "value a taken to the b'th root within accuracy c"},
	{"round", 1, 3, 0, OP_NOP, 0, f_round,
	 "round value a to b number of decimal places"},
	{"rsearch", 2, 4, 0, OP_NOP, 0, f_rsearch,
	 "reverse search matrix or list for value b\n\t\t    starting at index c"},
	{"runtime", 0, 0, 0, OP_NOP, f_runtime, 0,
	 "user mode cpu time in seconds"},
	{"saveval", 1, 1, 0, OP_SAVEVAL, 0, 0,
	 "set flag for saving values"},
	{"scale", 2, 2, 0, OP_SCALE, 0, 0,
	 "scale value up or down by a power of two"},
	{"scan", 1, IN, FA, OP_NOP, 0, f_scan,
	 "scan standard input for assignment to one or more variables"},
	{"scanf", 2, IN, FA, OP_NOP, 0, f_scanf,
	 "formatted scan of standard input for assignment to variables"},
	{"search", 2, 4, 0, OP_NOP, 0, f_search,
	 "search matrix or list for value b starting\n\t\t    at index c"},
	{"sec", 1, 2, 0, OP_NOP, 0, f_sec,
	 "sec of a within accuracy b"},
	{"sech", 1, 2, 0, OP_NOP, 0, f_sech,
	 "hyperbolic secant of a within accuracy b"},
	{"seed", 0, 0, 0, OP_NOP, f_seed, 0,
	 "return a 64 bit seed for a psuedo-random generator"},
	{"segment", 2, 3, 0, OP_NOP, 0, f_segment,
	 "specified segment of specified list"},
	{"select", 2, 2, 0, OP_NOP, 0, f_select,
	 "form sublist of selected elements from list"},
	{"setbit", 2, 3, 0, OP_NOP, 0, f_setbit,
	 "set specified bit in string"},
	{"sgn", 1, 1, 0, OP_SGN, qsign, 0,
	 "sign of value (-1, 0, 1)"},
	{"sha", 0, IN, 0, OP_NOP, 0, f_sha,
	 "old Secure Hash Algorithm (SHS FIPS Pub 180)"},
	{"sha1", 0, IN, 0, OP_NOP, 0, f_sha1,
	 "Secure Hash Algorithm (SHS-1 FIPS Pub 180-1)"},
	{"sin", 1, 2, 0, OP_NOP, 0, f_sin,
	 "sine of value a within accuracy b"},
	{"sinh", 1, 2, 0, OP_NOP, 0, f_sinh,
	 "hyperbolic sine of a within accuracy b"},
	{"size", 1, 1, 0, OP_NOP, 0, f_size,
	 "total number of elements in value"},
	{"sizeof", 1, 1, 0, OP_NOP, 0, f_sizeof,
	 "number of octets used to hold the value"},
	{"sort", 1, 1, 0, OP_NOP, 0, f_sort,
	 "sort a copy of a matrix or list"},
	{"sqrt", 1, 3, 0, OP_NOP, 0, f_sqrt,
	 "square root of value a within accuracy b"},
	{"srand", 0, 1, 0, OP_NOP, 0, f_srand,
	 "seed the rand() function"},
	{"srandom", 0, 4, 0, OP_NOP, 0, f_srandom,
	 "seed the random() function"},
	{"ssq", 1, IN, 0, OP_NOP, 0, f_ssq,
	 "sum of squares of values"},
	{"stoponerror", 0, 1, 0, OP_NOP, 0, f_stoponerror,
	 "assign value to stoponerror flag"},
	{"str", 1, 1, 0, OP_NOP, 0, f_str,
	 "simple value converted to string"},
	{"strcat", 1,IN, 0, OP_NOP, 0, f_strcat,
	 "concatenate strings together"},
	{"strcmp", 2, 2, 0, OP_NOP, 0, f_strcmp,
	 "compare two null-terminated strings"},
	{"strcpy", 2, 2, 0, OP_NOP, 0, f_strcpy,
	 "copy null-terminated string to string"},
	{"strerror", 0, 1, 0, OP_NOP, 0, f_strerror,
	 "string describing error type"},
	{"strlen", 1, 1, 0, OP_NOP, 0, f_strlen,
	 "length of string"},
	{"strncmp", 3, 3, 0, OP_NOP, 0, f_strncmp,
	 "compare strings a, b to c characters"},
	{"strncpy", 3, 3, 0, OP_NOP, 0, f_strncpy,
	 "copy up to c characters from string to string"},
	{"strpos", 2, 2, 0, OP_NOP, 0, f_strpos,
	 "index of first occurrence of b in a"},
	{"strprintf", 1, IN, 0, OP_NOP, 0, f_strprintf,
	 "return formatted output as a string"},
	{"strscan", 2, IN, FA, OP_NOP, 0, f_strscan,
	 "scan a string for assignments to one or more variables"},
	{"strscanf", 2, IN, FA, OP_NOP, 0, f_strscanf,
	 "formatted scan of string for assignments to variables"},
	{"substr", 3, 3, 0, OP_NOP, 0, f_substr,
	 "substring of a from position b for c chars"},
	{"sum", 0, IN, 0, OP_NOP, 0, f_sum,
	 "sum of list or object sums and/or other terms"},
	{"swap", 2, 2, 0, OP_SWAP, 0, 0,
	 "swap values of variables a and b (can be dangerous)"},
	{"system", 1, 1, 0, OP_NOP, 0, f_system,
	 "call Unix command"},
	{"tail", 2, 2, 0, OP_NOP, 0, f_tail,
	 "retain list of specified number at tail of list"},
	{"tan", 1, 2, 0, OP_NOP, 0, f_tan,
	 "tangent of a within accuracy b"},
	{"tanh", 1, 2, 0, OP_NOP, 0, f_tanh,
	 "hyperbolic tangent of a within accuracy b"},
	{"test", 1, 1, 0, OP_TEST, 0, 0,
	 "test that value is nonzero"},
	{"time", 0, 0, 0, OP_NOP, f_time, 0,
	 "number of seconds since 00:00:00 1 Jan 1970 UTC"},
	{"trunc", 1, 2, 0, OP_NOP, f_trunc, 0,
	 "truncate a to b number of decimal places"},
	{"ungetc", 2, 2, 0, OP_NOP, 0, f_ungetc,
	 "unget char read from file"},
	{"xor", 1, IN, 0, OP_NOP, 0, f_xor,
	 "logical xor"},

	/* end of table */
	{NULL, 0, 0, 0, 0, 0, 0,
	 NULL}
};


/*
 * Show the list of primitive built-in functions
 *
 * When FUNCLIST is defined, we are being compiled by rules from the help
 * sub-directory to form a program that will produce the main part of the
 * buiiltin help file.  These rules will convert the following function
 * name into main and remove the 'sed me out' line.
 *
 * See the builtin rule in the help/Makefile for details.
 */
void	/* sed me out */
showbuiltins(void)
{
	CONST struct builtin *bp;	/* current function */

	printf("\nName\tArgs\tDescription\n\n");
	for (bp = builtins; bp->b_name; bp++) {
		printf("%-9s ", bp->b_name);
		if (bp->b_maxargs == IN)
			printf("%d+    ", bp->b_minargs);
		else if (bp->b_minargs == bp->b_maxargs)
			printf("%-6d", bp->b_minargs);
		else
			printf("%d-%-4d", bp->b_minargs, bp->b_maxargs);
		printf("%s\n", bp->b_desc);
	}
	printf("\n");
}


#if !defined(FUNCLIST)

/*
 * Call a built-in function.
 * Arguments to the function are on the stack, but are not removed here.
 * Functions are either purely numeric, or else can take any value type.
 *
 * given:
 *	index		index on where to scan in builtin table
 *	argcount	number of args
 *	stck		arguments on the stack
 */
VALUE
builtinfunc(long index, int argcount, VALUE *stck)
{
	VALUE *sp;			/* pointer to stack entries */
	VALUE **vpp;			/* pointer to current value address */
	CONST struct builtin *bp;	/* builtin function to be called */
	NUMBER *numargs[IN];		/* numeric arguments for function */
	VALUE *valargs[IN];		/* addresses of actual arguments */
	VALUE result;			/* general result of function */
	long i;

	if ((unsigned long)index >=
	    (sizeof(builtins) / sizeof(builtins[0])) - 1) {
		math_error("Bad built-in function index");
		/*NOTREACHED*/
	}
	bp = &builtins[index];
	if (argcount < bp->b_minargs) {
		math_error("Too few arguments for builtin function \"%s\"",
		    bp->b_name);
		/*NOTREACHED*/
	}
	if ((argcount > bp->b_maxargs) || (argcount > IN)) {
		math_error("Too many arguments for builtin function \"%s\"",
		    bp->b_name);
		/*NOTREACHED*/
	}
	/*
	 * If an address was passed, then point at the real variable,
	 * otherwise point at the stack value itself (unless the function
	 * is very special).
	 */
	sp = stck - argcount + 1;
	vpp = valargs;
	for (i = argcount; i > 0; i--) {
		if ((sp->v_type != V_ADDR) || (bp->b_flags & FA))
			*vpp = sp;
		else
			*vpp = sp->v_addr;
		sp++;
		vpp++;
	}
	/*
	 * Handle general values if the function accepts them.
	 */
	if (bp->b_valfunc) {
		vpp = valargs;
		if ((bp->b_minargs == 1) && (bp->b_maxargs == 1))
			result = (*bp->b_valfunc)(vpp[0]);
		else if ((bp->b_minargs == 2) && (bp->b_maxargs == 2))
			result = (*bp->b_valfunc)(vpp[0], vpp[1]);
		else if ((bp->b_minargs == 3) && (bp->b_maxargs == 3))
			result = (*bp->b_valfunc)(vpp[0], vpp[1], vpp[2]);
		else if ((bp->b_minargs == 4) && (bp->b_maxargs == 4))
			result = (*bp->b_valfunc)(vpp[0],vpp[1],vpp[2],vpp[3]);
		else
			result = (*bp->b_valfunc)(argcount, vpp);
		return result;
	}
	/*
	 * Function must be purely numeric, so handle that.
	 */
	vpp = valargs;
	for (i = 0; i < argcount; i++) {
		if ((*vpp)->v_type != V_NUM) {
			math_error("Non-real argument for builtin function %s", bp->b_name);
			/*NOTREACHED*/
		}
		numargs[i] = (*vpp)->v_num;
		vpp++;
	}
	result.v_type = V_NUM;
	if (!(bp->b_flags & FE) && (bp->b_minargs != bp->b_maxargs)) {
		result.v_num = (*bp->b_numfunc)(argcount, numargs);
		return result;
	}
	if ((bp->b_flags & FE) && (argcount < bp->b_maxargs))
		numargs[argcount++] = conf->epsilon;

	switch (argcount) {
		case 0:
			result.v_num = (*bp->b_numfunc)();
			break;
		case 1:
			result.v_num = (*bp->b_numfunc)(numargs[0]);
			break;
		case 2:
			result.v_num = (*bp->b_numfunc)(numargs[0], numargs[1]);
			break;
		case 3:
			result.v_num = (*bp->b_numfunc)(numargs[0],
					numargs[1], numargs[2]);
			break;
		case 4:
			result.v_num = (*bp->b_numfunc)(numargs[0], numargs[1],
					numargs[2], numargs[3]);
			break;
		default:
			math_error("Bad builtin function call");
			/*NOTREACHED*/
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
	CONST struct builtin *bp;

	for (bp = builtins; bp->b_name; bp++) {
		if ((*name == *bp->b_name) && (strcmp(name, bp->b_name) == 0))
		return (bp - builtins);
	}
	return -1;
}


/*
 * Given the index of a built-in function, return its name.
 */
char *
builtinname(long index)
{
	if ((unsigned long)index >=
	    (sizeof(builtins) / sizeof(builtins[0])) - 1)
		return "";
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
	CONST struct builtin *bp;

	if ((unsigned long)index >=
	    (sizeof(builtins) / sizeof(builtins[0])) - 1) {
		math_error("Unknown built in index");
		/*NOTREACHED*/
	}
	bp = &builtins[index];
	if (count < bp->b_minargs)
		scanerror(T_NULL,
		    "Too few arguments for builtin function \"%s\"",
	bp->b_name);
	if (count > bp->b_maxargs)
		scanerror(T_NULL,
		    "Too many arguments for builtin function \"%s\"",
		    bp->b_name);
}


/*
 * Return the opcode for a built-in function that can be used to avoid
 * the function call at all.
 */
int
builtinopcode(long index)
{
	if ((unsigned long)index >=
	    (sizeof(builtins) / sizeof(builtins[0])) - 1)
		return OP_NOP;
	return builtins[index].b_opcode;
}

/*
 * Show the error-values created by newerror(str).
 */
void
showerrors(void)
{
	int i;

	if (nexterrnum == E_USERDEF)
		printf("No new error-values created\n");
	for (i = E_USERDEF; i < nexterrnum; i++)
		printf("%d: %s\n", i,
			namestr(&newerrorstr, i - E_USERDEF));
}


#endif /* FUNCLIST */
