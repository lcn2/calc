/*
 * config - configuration routines
 *
 * Copyright (C) 1999-2007,2021  David I. Bell and Landon Curt Noll
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
 * Under source code control:	1991/07/20 00:21:56
 * File existed as early as:	1991
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>

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

#include "calc.h"
#include "alloc.h"
#include "token.h"
#include "zrand.h"
#include "block.h"
#include "nametype.h"
#include "config.h"
#include "str.h"
#include "custom.h"
#include "strl.h"

#include "have_strdup.h"
#if !defined(HAVE_STRDUP)
# define strdup(x) calc_strdup((CONST char *)(x))
#endif /* HAVE_STRDUP */


#include "banned.h"	/* include after system header <> includes */


/*
 * deal with systems that lack a defined CLK_TCK
 */
#if defined(CLK_TCK)
# define CALC_HZ ((long)(CLK_TCK))
#else
# define CALC_HZ (0L)	/* no defined clock tick rate */
#endif


/*
 * Table of configuration types that can be set or read.
 */
NAMETYPE configs[] = {
	{"all",		CONFIG_ALL},
	{"mode",	CONFIG_MODE},
	{"mode2",	CONFIG_MODE2},
	{"display",	CONFIG_DISPLAY},
	{"epsilon",	CONFIG_EPSILON},
	/*epsilonprec -- tied to epsilon not a configuration type*/
	{"trace",	CONFIG_TRACE},
	{"maxprint",	CONFIG_MAXPRINT},
	{"mul2",	CONFIG_MUL2},
	{"sq2",		CONFIG_SQ2},
	{"pow2",	CONFIG_POW2},
	{"redc2",	CONFIG_REDC2},
	{"tilde",	CONFIG_TILDE},
	{"tab",		CONFIG_TAB},
	{"quomod",	CONFIG_QUOMOD},
	{"quo",		CONFIG_QUO},
	{"mod",		CONFIG_MOD},
	{"sqrt",	CONFIG_SQRT},
	{"appr",	CONFIG_APPR},
	{"cfappr",	CONFIG_CFAPPR},
	{"cfsim",	CONFIG_CFSIM},
	{"outround",	CONFIG_OUTROUND},
	{"round",	CONFIG_ROUND},
	{"leadzero",	CONFIG_LEADZERO},
	{"fullzero",	CONFIG_FULLZERO},
	{"maxscan",	CONFIG_MAXSCAN},
	    {"maxerr",	CONFIG_MAXSCAN},	/* old name for maxscan */
	{"prompt",	CONFIG_PROMPT},
	{"more",	CONFIG_MORE},
	{"blkmaxprint", CONFIG_BLKMAXPRINT},
	{"blkverbose",	CONFIG_BLKVERBOSE},
	{"blkbase",	CONFIG_BLKBASE},
	{"blkfmt",	CONFIG_BLKFMT},
	{"resource_debug", CONFIG_RESOURCE_DEBUG},
	{"lib_debug",	CONFIG_RESOURCE_DEBUG},
	{"calc_debug",	CONFIG_CALC_DEBUG},
	{"user_debug",	CONFIG_USER_DEBUG},
	{"verbose_quit",CONFIG_VERBOSE_QUIT},
	{"ctrl_d",	CONFIG_CTRL_D},
	    {"ctrl-d",	CONFIG_CTRL_D},		/* alias for ctrl_d */
	{"program",	CONFIG_PROGRAM},
	{"basename",	CONFIG_BASENAME},
	{"windows",	CONFIG_WINDOWS},
	{"cygwin",	CONFIG_CYGWIN},
	{"compile_custom",	CONFIG_COMPILE_CUSTOM},
	{"allow_custom",	CONFIG_ALLOW_CUSTOM},
	{"version",	CONFIG_VERSION},
	{"baseb",	CONFIG_BASEB},
	{"redecl_warn",	CONFIG_REDECL_WARN},
	{"dupvar_warn",	CONFIG_DUPVAR_WARN},
	{"hz",		CONFIG_HZ},
	{NULL,		0}
};


/*
 * configurations
 */
CONFIG oldstd = {	/* backward compatible standard configuration */
	MODE_INITIAL,		/* current output mode */
	MODE2_INITIAL,		/* current secondary output mode */
	20,			/* current output digits for float or exp */
	NULL,	/* loaded in at startup - default error for real functions */
	EPSILONPREC_DEFAULT,	/* binary precision of epsilon */
	FALSE,			/* tracing flags */
	MAXPRINT_DEFAULT,	/* number of elements to print */
	MUL_ALG2,		/* size of number to use multiply alg 2 */
	SQ_ALG2,		/* size of number to use square alg 2 */
	POW_ALG2,		/* size of modulus to use REDC for powers */
	REDC_ALG2,		/* size of modulus to use REDC algorithm 2 */
	TRUE,			/* ok to print a tilde on approximations */
	TRUE,			/* ok to print tab before numeric values */
	0,			/* quomod() default rounding mode */
	2,			/* quotient // default rounding mode */
	0,			/* mod % default rounding mode */
	24,			/* sqrt() default rounding mode */
	24,			/* appr() default rounding mode */
	0,			/* cfappr() default rounding mode */
	8,			/* cfsim() default rounding mode */
	2,			/* output default rounding mode */
	24,			/* round()/bround() default rounding mode */
	FALSE,			/* ok to print leading 0 before decimal pt */
	0,			/* ok to print trailing 0's */
	MAXSCANCOUNT,		/* max scan errors before abort */
	PROMPT1,		/* normal prompt */
	PROMPT2,		/* prompt when inside multi-line input */
	BLK_DEF_MAXPRINT,	/* number of octets of a block to print */
	FALSE,			/* skip duplicate block output lines */
	BLK_BASE_HEX,		/* block octet print base */
	BLK_FMT_HD_STYLE,	/* block output format */
	0,			/* internal calc debug level */
	3,			/* calc resource file debug level */
	0,			/* user defined debug level */
	FALSE,			/* print Quit or abort executed messages */
	CTRL_D_VIRGIN_EOF,	/* ^D only exits on virgin lines */
	NULL,			/* our name */
	NULL,			/* basename of our name */
#if defined(_WIN32)
	TRUE,			/* running under windows */
#else
	FALSE,			/* congrats, you are not using windows */
#endif
#if defined(__CYGWIN__)
	TRUE,			/* compiled under cygwin */
#else
	FALSE,			/* not compiled with cygwin */
#endif
#if defined(CUSTOM)
	TRUE,			/* compiled with -DCUSTOM */
#else
	FALSE,			/* compiled without -DCUSTOM */
#endif
	&allow_custom,		/* *TRUE=> custom functions are enabled */
	NULL,			/* version */
	BASEB,			/* base for calculations */
	TRUE,			/* warn when redeclaring */
	TRUE,			/* warn when variable names collide */
};
CONFIG newstd = {	/* new non-backward compatible configuration */
	MODE_INITIAL,		/* current output mode */
	MODE2_INITIAL,		/* current output mode */
	20,			/* current output digits for float or exp */
	NULL,	/* loaded in at startup - default error for real functions */
	EPSILONPREC_DEFAULT,	/* binary precision of epsilon */
	FALSE,			/* tracing flags */
	MAXPRINT_DEFAULT,	/* number of elements to print */
	MUL_ALG2,		/* size of number to use multiply alg 2 */
	SQ_ALG2,		/* size of number to use square alg 2 */
	POW_ALG2,		/* size of modulus to use REDC for powers */
	REDC_ALG2,		/* size of modulus to use REDC algorithm 2 */
	TRUE,			/* ok to print a tilde on approximations */
	TRUE,			/* ok to print tab before numeric values */
	0,			/* quomod() default rounding mode */
	2,			/* quotient // default rounding mode */
	0,			/* mod % default rounding mode */
	24,			/* sqrt() default rounding mode */
	24,			/* appr() default rounding mode */
	0,			/* cfappr() default rounding mode */
	8,			/* cfsim() default rounding mode */
	24,			/* output default rounding mode */
	24,			/* round()/bround() default rounding mode */
	TRUE,			/* ok to print leading 0 before decimal pt */
	0,			/* ok to print trailing 0's */
	MAXSCANCOUNT,		/* max scan errors before abort */
	"; ",			/* normal prompt */
	";; ",			/* prompt when inside multi-line input */
	BLK_DEF_MAXPRINT,	/* number of octets of a block to print */
	FALSE,			/* skip duplicate block output lines */
	BLK_BASE_HEX,		/* block octet print base */
	BLK_FMT_HD_STYLE,	/* block output format */
	0,			/* internal calc debug level */
	3,			/* calc resource file debug level */
	0,			/* user defined debug level */
	FALSE,			/* print Quit or abort executed messages */
	CTRL_D_VIRGIN_EOF,	/* ^D only exits on virgin lines */
	NULL,			/* our name */
	NULL,			/* basename of our name */
#if defined(_WIN32)
	TRUE,			/* running under windows */
#else
	FALSE,			/* congrats, you are not using windows */
#endif
#if defined(__CYGWIN__)
	TRUE,			/* compiled under cygwin */
#else
	FALSE,			/* not compiled with cygwin */
#endif
#if defined(CUSTOM)
	TRUE,			/* compiled with -DCUSTOM */
#else
	FALSE,			/* compiled without -DCUSTOM */
#endif
	&allow_custom,		/* *TRUE=> custom functions are enabled */
	NULL,			/* version */
	BASEB,			/* base for calculations */
	TRUE,			/* warn when redeclaring */
	TRUE,			/* warn when variable names collide */
};
CONFIG *conf = NULL;	/* loaded in at startup - current configuration */


/*
 * Possible output modes.
 */
STATIC NAMETYPE modes[] = {
	{"fraction",	MODE_FRAC},
	{"frac",	MODE_FRAC},
	{"integer",	MODE_INT},
	{"int",		MODE_INT},
	{"real",	MODE_REAL},
	{"float",	MODE_REAL},
	{"default",	MODE_INITIAL},	/* MODE_REAL */
	{"scientific",	MODE_EXP},
	{"sci",		MODE_EXP},
	{"exp",		MODE_EXP},
	{"hexadecimal", MODE_HEX},
	{"hex",		MODE_HEX},
	{"octal",	MODE_OCTAL},
	{"oct",		MODE_OCTAL},
	{"binary",	MODE_BINARY},
	{"bin",		MODE_BINARY},
	{"float-auto",	MODE_REAL_AUTO},
	{"off",		MODE2_OFF},
	{NULL,		0}
};


/*
 * Possible block base output modes
 */
STATIC NAMETYPE blk_base[] = {
	{"hexadecimal", BLK_BASE_HEX},
	{"hex",		BLK_BASE_HEX},
	{"default",	BLK_BASE_HEX},
	{"octal",	BLK_BASE_OCT},
	{"oct",		BLK_BASE_OCT},
	{"character",	BLK_BASE_CHAR},
	{"char",	BLK_BASE_CHAR},
	{"binary",	BLK_BASE_BINARY},
	{"bin",		BLK_BASE_BINARY},
	{"raw",		BLK_BASE_RAW},
	{"none",	BLK_BASE_RAW},
	{NULL,		0}
};


/*
 * Possible block output formats
 */
STATIC NAMETYPE blk_fmt[] = {
	{"lines",	BLK_FMT_LINE},
	{"line",	BLK_FMT_LINE},
	{"strings",	BLK_FMT_STRING},
	{"string",	BLK_FMT_STRING},
	{"str",		BLK_FMT_STRING},
	{"od_style",	BLK_FMT_OD_STYLE},
	{"odstyle",	BLK_FMT_OD_STYLE},
	{"od",		BLK_FMT_OD_STYLE},
	{"hd_style",	BLK_FMT_HD_STYLE},
	{"hdstyle",	BLK_FMT_HD_STYLE},
	{"hd",		BLK_FMT_HD_STYLE},
	{"default",	BLK_FMT_HD_STYLE},
	{NULL,		0}
};


/*
 * Possible ctrl_d styles
 */
STATIC NAMETYPE ctrl_d[] = {
	{"virgin_eof",	CTRL_D_VIRGIN_EOF},
	{"virgineof",	CTRL_D_VIRGIN_EOF},
	{"virgin",	CTRL_D_VIRGIN_EOF},
	{"default",	CTRL_D_VIRGIN_EOF},
	{"never_eof",	CTRL_D_NEVER_EOF},
	{"nevereof",	CTRL_D_NEVER_EOF},
	{"never",	CTRL_D_NEVER_EOF},
	{"empty_eof",	CTRL_D_EMPTY_EOF},
	{"emptyeof",	CTRL_D_EMPTY_EOF},
	{"empty",	CTRL_D_EMPTY_EOF},
	{NULL,		0}
};


/*
 * Possible binary config state values
 */
#define TRUE_STRING	"true"
#define FALSE_STRING	"false"
STATIC NAMETYPE truth[] = {
	{TRUE_STRING,	TRUE},
	{"t",		TRUE},
	{"on",		TRUE},
	{"yes",		TRUE},
	{"y",		TRUE},
	{"set",		TRUE},
	{"1",		TRUE},
	{FALSE_STRING,	FALSE},
	{"f",		FALSE},
	{"off",		FALSE},
	{"no",		FALSE},
	{"n",		FALSE},
	{"unset",	FALSE},
	{"0",		FALSE},
	{NULL,		0}
};


/*
 * declare static functions
 */
S_FUNC long lookup_long(NAMETYPE *set, char *name);
S_FUNC char *lookup_name(NAMETYPE *set, long val);
S_FUNC int getlen(VALUE *vp, LEN *lp);


/*
 * Given a string value which represents a configuration name, return
 * the configuration type for that string.  Returns negative type if
 * the string is unknown.
 *
 * given:
 *	name			configuration name
 */
int
configtype(char *name)
{
	NAMETYPE *cp;		/* current config pointer */

	for (cp = configs; cp->name; cp++) {
		if (strcmp(cp->name, name) == 0)
			return cp->type;
	}
	return -1;
}


/*
 * lookup_long - given a name and a NAMETYPE, return the int
 *
 * given:
 *	set		the NAMESET array of name/int pairs
 *	name		mode name
 *
 * returns:
 *	numeric value of the name or -1 if not found
 */
S_FUNC long
lookup_long(NAMETYPE *set, char *name)
{
	NAMETYPE *cp;		/* current config pointer */

	for (cp = set; cp->name; cp++) {
		if (strcmp(cp->name, name) == 0)
			return cp->type;
	}
	return -1;
}


/*
 * lookup_name - given numeric value and a NAMETYPE, return the name
 *
 * given:
 *	set		the NAMESET array of name/int pairs
 *	val		numeric value to lookup
 *
 * returns:
 *	name of the value found of NULL
 */
S_FUNC char *
lookup_name(NAMETYPE *set, long val)
{
	NAMETYPE *cp;		/* current config pointer */

	for (cp = set; cp->name; cp++) {
		if (val == cp->type)
			return cp->name;
	}
	return NULL;
}


/*
 * Check whether VALUE at vp is a LEN (32-bit signed integer) and if so,
 * copy that integer to lp.
 *
 * Return: 1 ==> not an integer, 2 ==> int > 2^31, 0 ==> OK, -1 ==> error
 */

S_FUNC int
getlen(VALUE *vp, LEN *lp)
{
	if (vp->v_type != V_NUM || !qisint(vp->v_num))
		return 1;
	if (zge31b(vp->v_num->num))
		return 2;
	*lp = ztoi(vp->v_num->num);
	if (*lp < 0)
		return -1;
	return 0;
}


/*
 * Set the specified configuration type to the specified value.
 * An error is generated if the type number or value is illegal.
 */
void
setconfig(int type, VALUE *vp)
{
	NUMBER *q;
	CONFIG *newconf;	/* new configuration to set */
	long temp;
	LEN len = 0;
	char *p;

	switch (type) {
	case CONFIG_ALL:
		newconf = NULL; /* firewall */
		if (vp->v_type == V_STR) {
			if (strcmp(vp->v_str->s_str, "oldstd") == 0) {
				newconf = &oldstd;
			} else if (strcmp(vp->v_str->s_str, "newstd") == 0) {
				newconf = &newstd;
			} else {
				math_error("CONFIG alias not oldstd or newstd");
				/*NOTREACHED*/
			}
		} else if (vp->v_type != V_CONFIG) {
			math_error("non-CONFIG for all");
			/*NOTREACHED*/
		} else {
			newconf = vp->v_config;
		}

		/* free the current configuration */
		config_free(conf);

		/* set the new configuration */
		conf = config_copy(newconf);
		break;

	case CONFIG_TRACE:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for trace");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || !zistiny(q->num) ||
			((unsigned long) temp > TRACE_MAX)) {
				math_error("Bad trace value");
				/*NOTREACHED*/
		}
		conf->traceflags = (FLAG)temp;
		break;

	case CONFIG_DISPLAY:
		if (getlen(vp, &len)) {
			math_error("Bad value for display");
			/*NOTREACHED*/
		}
		math_setdigits(len);
		break;

	case CONFIG_MODE:
		if (vp->v_type != V_STR) {
			math_error("Non-string for mode");
			/*NOTREACHED*/
		}
		temp = lookup_long(modes, vp->v_str->s_str);
		if (temp < 0) {
			math_error("Unknown mode \"%s\"", vp->v_str->s_str);
			/*NOTREACHED*/
		}
		math_setmode((int) temp);
		break;

	case CONFIG_MODE2:
		if (vp->v_type != V_STR) {
			math_error("Non-string for mode");
			/*NOTREACHED*/
		}
		temp = lookup_long(modes, vp->v_str->s_str);
		if (temp < 0) {
			math_error("Unknown mode \"%s\"", vp->v_str->s_str);
			/*NOTREACHED*/
		}
		math_setmode2((int) temp);
		break;

	case CONFIG_EPSILON:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for epsilon");
			/*NOTREACHED*/
		}
		setepsilon(vp->v_num);
		break;

	case CONFIG_MAXPRINT:
		if (getlen(vp, &len)) {
			math_error("Bad value for maxprint");
			/*NOTREACHED*/
		}
		conf->maxprint = len;
		break;

	case CONFIG_MUL2:
		if (getlen(vp, &len) || len < 0 || len == 1) {
			math_error("Bad value for mul2");
			/*NOTREACHED*/
		}
		if (len == 0)
			len = MUL_ALG2;
		conf->mul2 = len;
		break;

	case CONFIG_SQ2:
		if (getlen(vp, &len) || len < 0 || len == 1) {
			math_error("Bad value for sq2");
			/*NOTREACHED*/
		}
		if (len == 0)
			len = SQ_ALG2;
		conf->sq2 = len;
		break;

	case CONFIG_POW2:
		if (getlen(vp, &len) || len < 0 || len == 1) {
			math_error("Bad value for pow2");
			/*NOTREACHED*/
		}
		if (len == 0)
			len = POW_ALG2;
		conf->pow2 = len;
		break;

	case CONFIG_REDC2:
		if (getlen(vp, &len) || len < 0 || len == 1) {
			math_error("Bad value for redc2");
			/*NOTREACHED*/
		}
		if (len == 0)
			len = REDC_ALG2;
		conf->redc2 = len;
		break;

	case CONFIG_TILDE:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->tilde_ok = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = lookup_long(truth, vp->v_str->s_str);
			if (temp < 0) {
				math_error("Illegal truth value for tilde");
				/*NOTREACHED*/
			}
			conf->tilde_ok = (int)temp;
		}
		break;

	case CONFIG_TAB:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->tab_ok = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = lookup_long(truth, vp->v_str->s_str);
			if (temp < 0) {
				math_error("Illegal truth value for tab");
				/*NOTREACHED*/
			}
			conf->tab_ok = (int)temp;
		}
		break;

	case CONFIG_QUOMOD:
		if (getlen(vp, &len)) {
			math_error("Illegal value for quomod");
			/*NOTREACHED*/
		}
		conf->quomod = len;
		break;

	case CONFIG_QUO:
		if (getlen(vp, &len)) {
			math_error("Illegal value for quo");
			/*NOTREACHED*/
		}
		conf->quo = len;
		break;

	case CONFIG_MOD:
		if (getlen(vp, &len)) {
			math_error("Illegal value for mod");
			/*NOTREACHED*/
		}
		conf->mod = len;
		break;

	case CONFIG_SQRT:
		if (getlen(vp, &len)) {
			math_error("Illegal value for sqrt");
			/*NOTREACHED*/
		}
		conf->sqrt = len;
		break;

	case CONFIG_APPR:
		if (getlen(vp, &len)) {
			math_error("Illegal value for appr");
			/*NOTREACHED*/
		}
		conf->appr = len;
		break;

	case CONFIG_CFAPPR:
		if (getlen(vp, &len)) {
			math_error("Illegal value for cfappr");
			/*NOTREACHED*/
		}
		conf->cfappr = len;
		break;

	case CONFIG_CFSIM:
		if (getlen(vp, &len)) {
			math_error("Illegal value for cfsim");
			/*NOTREACHED*/
		}
		conf->cfsim = len;
		break;

	case CONFIG_OUTROUND:
		if (getlen(vp, &len)) {
			math_error("Illegal value for outround");
			/*NOTREACHED*/
		}
		conf->outround = len;
		break;

	case CONFIG_ROUND:
		if (getlen(vp, &len)) {
			math_error("Illegal value for round");
			/*NOTREACHED*/
		}
		conf->round = len;
		break;

	case CONFIG_LEADZERO:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->leadzero = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = lookup_long(truth, vp->v_str->s_str);
			if (temp < 0) { {
				math_error("Illegal truth value for leadzero");
				/*NOTREACHED*/
			}
			}
			conf->leadzero = (int)temp;
		}
		break;

	case CONFIG_FULLZERO:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->fullzero = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = lookup_long(truth, vp->v_str->s_str);
			if (temp < 0) { {
				math_error("Illegal truth value for fullzero");
				/*NOTREACHED*/
			}
			}
			conf->fullzero = (int)temp;
		}
		break;

	case CONFIG_MAXSCAN:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for maxscancount");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num))
			temp = -1;
		if (temp < 0) {
			math_error("Maxscan value is out of range");
			/*NOTREACHED*/
		}
		conf->maxscancount = temp;
		break;

	case CONFIG_PROMPT:
		if (vp->v_type != V_STR) {
			math_error("Non-string for prompt");
			/*NOTREACHED*/
		}
		p = (char *)malloc(vp->v_str->s_len + 1);
		if (p == NULL) {
			math_error("Cannot duplicate new prompt");
			/*NOTREACHED*/
		}
		strlcpy(p, vp->v_str->s_str, vp->v_str->s_len + 1);
		free(conf->prompt1);
		conf->prompt1 = p;
		break;

	case CONFIG_MORE:
		if (vp->v_type != V_STR) {
			math_error("Non-string for more prompt");
			/*NOTREACHED*/
		}
		p = (char *)malloc(vp->v_str->s_len + 1);
		if (p == NULL) {
			math_error("Cannot duplicate new more prompt");
			/*NOTREACHED*/
		}
		strlcpy(p, vp->v_str->s_str, vp->v_str->s_len + 1);
		free(conf->prompt2);
		conf->prompt2 = p;
		break;

	case CONFIG_BLKMAXPRINT:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for blkmaxprint");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num))
			temp = -1;
		if (temp < 0) {
			math_error("Blkmaxprint value is out of range");
			/*NOTREACHED*/
		}
		conf->blkmaxprint = temp;
		break;

	case CONFIG_BLKVERBOSE:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->blkverbose = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = lookup_long(truth, vp->v_str->s_str);
			if (temp < 0) {
			    math_error("Illegal truth value for blkverbose");
			    /*NOTREACHED*/
			}
			conf->blkverbose = (int)temp;
		}
		break;

	case CONFIG_BLKBASE:
		if (vp->v_type != V_STR) {
			math_error("Non-string for blkbase");
			/*NOTREACHED*/
		}
		temp = lookup_long(blk_base, vp->v_str->s_str);
		if (temp < 0) {
			math_error("Unknown mode \"%s\" for blkbase",
			    vp->v_str->s_str);
			/*NOTREACHED*/
		}
		conf->blkbase = temp;
		break;

	case CONFIG_BLKFMT:
		if (vp->v_type != V_STR) {
			math_error("Non-string for blkfmt");
			/*NOTREACHED*/
		}
		temp = lookup_long(blk_fmt, vp->v_str->s_str);
		if (temp < 0) {
			math_error("Unknown mode \"%s\" for blkfmt",
			    vp->v_str->s_str);
			/*NOTREACHED*/
		}
		conf->blkfmt = temp;
		break;

	case CONFIG_CALC_DEBUG:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for calc_debug");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || !zistiny(q->num)) {
			math_error("Illegal calc_debug parameter value");
			/*NOTREACHED*/
		}
		conf->calc_debug = temp;
		break;

	case CONFIG_RESOURCE_DEBUG:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for resource_debug");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || !zistiny(q->num)) {
			math_error("Illegal resource_debug parameter value");
			/*NOTREACHED*/
		}
		conf->resource_debug = temp;
		break;

	case CONFIG_USER_DEBUG:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for user_debug");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || !zistiny(q->num)) {
			math_error("Illegal user_debug parameter value");
			/*NOTREACHED*/
		}
		conf->user_debug = temp;
		break;

	case CONFIG_VERBOSE_QUIT:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->verbose_quit = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = lookup_long(truth, vp->v_str->s_str);
			if (temp < 0) {
				math_error("Illegal truth value "
					   "for verbose_quit");
				/*NOTREACHED*/
			}
			conf->verbose_quit = (int)temp;
		}
		break;

	case CONFIG_CTRL_D:
		if (vp->v_type != V_STR) {
			math_error("Non-string for ctrl_d");
			/*NOTREACHED*/
		}
		temp = lookup_long(ctrl_d, vp->v_str->s_str);
		if (temp < 0) {
			math_error("Unknown mode \"%s\" for ctrl_d",
			    vp->v_str->s_str);
			/*NOTREACHED*/
		}
		conf->ctrl_d = temp;
		break;

	case CONFIG_PROGRAM:
		math_error("The program config parameter is read-only");
		/*NOTREACHED*/
		abort();

	case CONFIG_BASENAME:
		math_error("The basename config parameter is read-only");
		/*NOTREACHED*/
		abort();

	case CONFIG_WINDOWS:
		math_error("The windows config parameter is read-only");
		/*NOTREACHED*/
		abort();

	case CONFIG_CYGWIN:
		math_error("The cygwin config parameter is read-only");
		/*NOTREACHED*/
		abort();

	case CONFIG_COMPILE_CUSTOM:
		math_error("The custom config parameter is read-only");
		/*NOTREACHED*/
		abort();

	case CONFIG_ALLOW_CUSTOM:
		math_error("The allow_custom config parameter is read-only");
		/*NOTREACHED*/
		abort();

	case CONFIG_VERSION:
		math_error("The version config parameter is read-only");
		/*NOTREACHED*/
		abort();

	case CONFIG_BASEB:
		math_error("The baseb config parameter is read-only");
		/*NOTREACHED*/
		abort();

	case CONFIG_REDECL_WARN:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->redecl_warn = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = lookup_long(truth, vp->v_str->s_str);
			if (temp < 0) {
				math_error("Illegal truth value for "
					   "redecl_warn");
				/*NOTREACHED*/
			}
			conf->redecl_warn = (int)temp;
		}
		break;

	case CONFIG_DUPVAR_WARN:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->dupvar_warn = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = lookup_long(truth, vp->v_str->s_str);
			if (temp < 0) {
				math_error("Illegal truth value for "
					   "dupvar_warn");
				/*NOTREACHED*/
			}
			conf->dupvar_warn = (int)temp;
		}
		break;

	case CONFIG_HZ:
		math_error("The clock tick rate config parameter is read-only");
		/*NOTREACHED*/
		abort();

	default:
		math_error("Setting illegal config parameter");
		/*NOTREACHED*/
		abort();
	}
}


/*
 * config_copy - copy the configuration from one value to another
 *
 * given:
 *	src	copy this configuration
 *
 * returns:
 *	pointer to the configuration copy
 */
CONFIG *
config_copy(CONFIG *src)
{
	CONFIG *dest;		/* the new CONFIG to return */

	/*
	 * firewall
	 */
	if (src == NULL || src->epsilon == NULL || src->prompt1 == NULL ||
	    src->prompt2 == NULL) {
		math_error("bad CONFIG value");
		/*NOTREACHED*/
	}

	/*
	 * malloc the storage
	 */
	dest = (CONFIG *)malloc(sizeof(CONFIG));
	if (dest == NULL) {
		math_error("malloc of CONFIG failed");
		/*NOTREACHED*/
	}

	/*
	 * copy over the values
	 */
	memcpy((void *)dest, (void *)src, sizeof(CONFIG));

	/*
	 * clone the pointer values
	 */
	dest->epsilon = qlink(src->epsilon);
	dest->prompt1 = strdup(src->prompt1);
	dest->prompt2 = strdup(src->prompt2);
	if (src->program == NULL) {
		dest->program = strdup(program);
	} else {
		dest->program = strdup(src->program);
	}
	if (src->base_name == NULL) {
		dest->base_name = strdup(base_name);
	} else {
		dest->base_name = strdup(src->base_name);
	}
	/* NOTE: allow_custom points to a global variable, so do not clone it */
	if (src->version == NULL) {
		dest->version = strdup(version());
	} else {
		dest->version = strdup(src->version);
	}

	/*
	 * return the new value
	 */
	return dest;
}


/*
 * config_free - free a CONFIG value
 *
 * given:
 *	cfg		the CONFIG value to free
 */
void
config_free(CONFIG *cfg)
{
	/*
	 * firewall
	 */
	if (cfg == NULL) {
		return;
	}

	/*
	 * free pointer values
	 */
	if (cfg->epsilon != NULL) {
		qfree(cfg->epsilon);
	}
	if (cfg->prompt1 != NULL) {
		free(cfg->prompt1);
	}
	if (cfg->prompt2 != NULL) {
		free(cfg->prompt2);
	}
	if (cfg->program != NULL) {
		free(cfg->program);
	}
	if (cfg->base_name != NULL) {
		free(cfg->base_name);
	}
	/* NOTE: allow_custom points to a global variable, so do not free it */
	if (cfg->version != NULL) {
		free(cfg->version);
	}

	/*
	 * free the CONFIG value itself
	 */
	free(cfg);
	return;
}


/*
 * config_value - return a CONFIG element as a value
 *
 * given:
 *	cfg		CONFIG from which an element will be returned
 *	type		the type of CONFIG element to print
 *	ret		where to return the value
 *
 * returns:
 *	ret points to the VALUE returned
 */
void
config_value(CONFIG *cfg, int type, VALUE *vp)
{
	long i=0;
	char *p;

	/*
	 * firewall
	 */
	if (cfg == NULL || cfg->epsilon == NULL || cfg->prompt1 == NULL ||
	    cfg->prompt2 == NULL) {
		math_error("bad CONFIG value");
		/*NOTREACHED*/
	}

	/*
	 * convert element to value
	 */
	vp->v_type = V_NUM;
	vp->v_subtype = V_NOSUBTYPE;
	switch (type) {
	case CONFIG_ALL:
		vp->v_type = V_CONFIG;
		vp->v_config = config_copy(conf);
		return;

	case CONFIG_TRACE:
		i = cfg->traceflags;
		break;

	case CONFIG_DISPLAY:
		i = cfg->outdigits;
		break;

	case CONFIG_MODE:
		vp->v_type = V_STR;
		p = lookup_name(modes, cfg->outmode);
		if (p == NULL) {
			math_error("invalid output mode: %d", cfg->outmode);
			/*NOTREACHED*/
		}
		vp->v_str = makenewstring(p);
		return;

	case CONFIG_MODE2:
		vp->v_type = V_STR;
		p = lookup_name(modes, cfg->outmode2);
		if (p == NULL) {
			math_error("invalid secondary output mode: %d",
				   cfg->outmode2);
			/*NOTREACHED*/
		}
		vp->v_str = makenewstring(p);
		return;

	case CONFIG_EPSILON:
		vp->v_num = qlink(cfg->epsilon);
		return;

	case CONFIG_MAXPRINT:
		i = cfg->maxprint;
		break;

	case CONFIG_MUL2:
		i = cfg->mul2;
		break;

	case CONFIG_SQ2:
		i = cfg->sq2;
		break;

	case CONFIG_POW2:
		i = cfg->pow2;
		break;

	case CONFIG_REDC2:
		i = cfg->redc2;
		break;

	case CONFIG_TILDE:
		i = (cfg->tilde_ok ? 1 : 0);
		break;

	case CONFIG_TAB:
		i = (cfg->tab_ok ? 1 : 0);
		break;

	case CONFIG_QUOMOD:
		i = cfg->quomod;
		break;

	case CONFIG_QUO:
		i = cfg->quo;
		break;

	case CONFIG_MOD:
		i = cfg->mod;
		break;

	case CONFIG_SQRT:
		i = cfg->sqrt;
		break;

	case CONFIG_APPR:
		i = cfg->appr;
		break;

	case CONFIG_CFAPPR:
		i = cfg->cfappr;
		break;

	case CONFIG_CFSIM:
		i = cfg->cfsim;
		break;

	case CONFIG_OUTROUND:
		i = cfg->outround;
		break;

	case CONFIG_ROUND:
		i = cfg->round;
		break;

	case CONFIG_LEADZERO:
		i = (cfg->leadzero ? 1 : 0);
		break;

	case CONFIG_FULLZERO:
		i = (cfg->fullzero ? 1 : 0);
		break;

	case CONFIG_MAXSCAN:
		i = cfg->maxscancount;
		break;

	case CONFIG_PROMPT:
		vp->v_type = V_STR;
		vp->v_str = makenewstring(cfg->prompt1);
		return;

	case CONFIG_MORE:
		vp->v_type = V_STR;
		vp->v_str = makenewstring(cfg->prompt2);
		return;

	case CONFIG_BLKMAXPRINT:
		i = cfg->blkmaxprint;
		break;

	case CONFIG_BLKVERBOSE:
		i = (cfg->blkverbose ? 1 : 0);
		break;

	case CONFIG_BLKBASE:
		vp->v_type = V_STR;
		p = lookup_name(blk_base, cfg->blkbase);
		if (p == NULL) {
			math_error("invalid block base: %d", cfg->blkbase);
			/*NOTREACHED*/
		}
		vp->v_str = makenewstring(p);
		return;

	case CONFIG_BLKFMT:
		vp->v_type = V_STR;
		p = lookup_name(blk_fmt, cfg->blkfmt);
		if (p == NULL) {
			math_error("invalid block format: %d", cfg->blkfmt);
			/*NOTREACHED*/
		}
		vp->v_str = makenewstring(p);
		return;

	case CONFIG_CALC_DEBUG:
		i = cfg->calc_debug;
		break;

	case CONFIG_RESOURCE_DEBUG:
		i = cfg->resource_debug;
		break;

	case CONFIG_USER_DEBUG:
		i = cfg->user_debug;
		break;

	case CONFIG_VERBOSE_QUIT:
		i = (cfg->verbose_quit ? 1 : 0);
		break;

	case CONFIG_CTRL_D:
		vp->v_type = V_STR;
		p = lookup_name(ctrl_d, cfg->ctrl_d);
		if (p == NULL) {
			math_error("invalid Control-D: %d", cfg->ctrl_d);
			/*NOTREACHED*/
		}
		vp->v_str = makenewstring(p);
		return;

	case CONFIG_PROGRAM:
		vp->v_type = V_STR;
		if (cfg->base_name == NULL) {
			vp->v_str = makestring(strdup(program));
		} else {
			vp->v_str = makenewstring(cfg->program);
		}
		return;

	case CONFIG_BASENAME:
		vp->v_type = V_STR;
		if (cfg->base_name == NULL) {
			vp->v_str = makestring(strdup(base_name));
		} else {
			vp->v_str = makenewstring(cfg->base_name);
		}
		return;

	case CONFIG_WINDOWS:
		i = (cfg->windows ? 1 : 0);
		break;

	case CONFIG_CYGWIN:
		i = (cfg->cygwin ? 1 : 0);
		break;

	case CONFIG_COMPILE_CUSTOM:
		i = (cfg->compile_custom ? 1 : 0);
		break;

	case CONFIG_ALLOW_CUSTOM:
		/* firewall */
		if (cfg->allow_custom == NULL) {
			cfg->allow_custom = &allow_custom;
		}
		i = (*(cfg->allow_custom) ? 1 : 0);
		break;

	case CONFIG_VERSION:
		vp->v_type = V_STR;
		if (cfg->version == NULL) {
			vp->v_str = makestring(strdup(version()));
		} else {
			vp->v_str = makenewstring(cfg->version);
		}
		return;

	case CONFIG_BASEB:
		i = BASEB;
		break;

	case CONFIG_REDECL_WARN:
		i = (cfg->redecl_warn ? 1 : 0);
		break;

	case CONFIG_DUPVAR_WARN:
		i = (cfg->dupvar_warn ? 1 : 0);
		break;

	case CONFIG_HZ:
		i = CALC_HZ;
		break;

	default:
		math_error("Getting illegal CONFIG element");
		/*NOTREACHED*/
	}

	/*
	 * if we got this far, we have a V_NUM in i
	 */
	vp->v_num = itoq(i);
	return;
}


/*
 * config_cmp - compare two CONFIG states
 *
 * given:
 *	cfg1 - 1st CONFIG to compare
 *	cfg2 - 2nd CONFIG to compare
 *
 * return:
 *	TRUE if configurations differ
 */
BOOL
config_cmp(CONFIG *cfg1, CONFIG *cfg2)
{
	/*
	 * firewall
	 */
	if (cfg1 == NULL || cfg1->epsilon == NULL || cfg1->prompt1 == NULL ||
	    cfg1->prompt2 == NULL) {
		math_error("CONFIG #1 value is invalid");
		/*NOTREACHED*/
	}
	if (cfg2 == NULL || cfg2->epsilon == NULL || cfg2->prompt1 == NULL ||
	    cfg2->prompt2 == NULL) {
		math_error("CONFIG #2 value is invalid");
		/*NOTREACHED*/
	}

	/*
	 * compare
	 */
	return cfg1->traceflags != cfg2->traceflags ||
	       cfg1->outdigits != cfg2->outdigits ||
	       cfg1->outmode != cfg2->outmode ||
	       cfg1->outmode2 != cfg2->outmode2 ||
	       qcmp(cfg1->epsilon, cfg2->epsilon) ||
	       cfg1->epsilonprec != cfg2->epsilonprec ||
	       cfg1->maxprint != cfg2->maxprint ||
	       cfg1->mul2 != cfg2->mul2 ||
	       cfg1->sq2 != cfg2->sq2 ||
	       cfg1->pow2 != cfg2->pow2 ||
	       cfg1->redc2 != cfg2->redc2 ||
	       cfg1->tilde_ok != cfg2->tilde_ok ||
	       cfg1->tab_ok != cfg2->tab_ok ||
	       cfg1->quomod != cfg2->quomod ||
	       cfg1->quo != cfg2->quo ||
	       cfg1->mod != cfg2->mod ||
	       cfg1->sqrt != cfg2->sqrt ||
	       cfg1->appr != cfg2->appr ||
	       cfg1->cfappr != cfg2->cfappr ||
	       cfg1->cfsim != cfg2->cfsim ||
	       cfg1->outround != cfg2->outround ||
	       cfg1->round != cfg2->round ||
	       cfg1->leadzero != cfg2->leadzero ||
	       cfg1->fullzero != cfg2->fullzero ||
	       cfg1->maxscancount != cfg2->maxscancount ||
	       strcmp(cfg1->prompt1, cfg2->prompt1) != 0 ||
	       strcmp(cfg1->prompt2, cfg2->prompt2) != 0 ||
	       cfg1->blkmaxprint != cfg2->blkmaxprint ||
	       cfg1->blkverbose != cfg2->blkverbose ||
	       cfg1->blkbase != cfg2->blkbase ||
	       cfg1->blkfmt != cfg2->blkfmt ||
	       cfg1->calc_debug != cfg2->calc_debug ||
	       cfg1->resource_debug != cfg2->resource_debug ||
	       cfg1->user_debug != cfg2->user_debug ||
	       cfg1->verbose_quit != cfg2->verbose_quit ||
	       cfg1->ctrl_d != cfg2->ctrl_d ||

	       (cfg1->program == NULL && cfg2->program != NULL) ||
	       (cfg1->program != NULL && cfg2->program == NULL) ||
	       (cfg1->program != NULL && cfg2->program != NULL &&
		strcmp(cfg1->program, cfg2->program) != 0) ||

	       (cfg1->base_name == NULL && cfg2->base_name != NULL) ||
	       (cfg1->base_name != NULL && cfg2->base_name == NULL) ||
	       (cfg1->base_name != NULL && cfg2->base_name != NULL &&
		strcmp(cfg1->base_name, cfg2->base_name) != 0) ||

	       cfg1->windows != cfg2->windows ||

	       cfg1->cygwin != cfg2->cygwin ||

	       cfg1->compile_custom != cfg2->compile_custom ||

	       (cfg1->allow_custom == NULL && cfg2->allow_custom != NULL) ||
	       (cfg1->allow_custom != NULL && cfg2->allow_custom == NULL) ||
	       (cfg1->allow_custom != NULL && cfg2->allow_custom != NULL &&
		*(cfg1->allow_custom) != *(cfg2->allow_custom)) ||

	       (cfg1->version == NULL && cfg2->version != NULL) ||
	       (cfg1->version != NULL && cfg2->version == NULL) ||
	       (cfg1->version != NULL && cfg2->version != NULL &&
		strcmp(cfg1->version, cfg2->version) != 0) ||

	       cfg1->baseb != cfg2->baseb;
}
