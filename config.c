/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Configuration routines.
 */

#include <stdio.h>
#include "calc.h"
#include "token.h"
#include "zrand.h"
#include "block.h"
#include "nametype.h"
#include "config.h"
#include "string.h"


/*
 * Table of configuration types that can be set or read.
 */
NAMETYPE configs[] = {
	{"all",		CONFIG_ALL},
	{"mode",	CONFIG_MODE},
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
	{"blkmaxprint",	CONFIG_BLKMAXPRINT},
	{"blkverbose",	CONFIG_BLKVERBOSE},
	{"blkbase",	CONFIG_BLKBASE},
	{"blkfmt",	CONFIG_BLKFMT},
	{"lib_debug",	CONFIG_LIB_DEBUG},
	{"calc_debug",	CONFIG_CALC_DEBUG},
	{"user_debug",	CONFIG_USER_DEBUG},
	{NULL,		0}
};


/*
 * configurations
 */
CONFIG oldstd = {	/* backward compatible standard configuration */
	MODE_INITIAL,		/* current output mode */
	20,			/* current output digits for float or exp */
	NULL,	/* loaded in at startup - default error for real functions */
	EPSILONPREC_DEFAULT,	/* binary precision of epsilon */
	FALSE,			/* tracing flags */
	MAXPRINT_DEFAULT,	/* number of elements to print */
	MUL_ALG2,		/* size of number to use multiply alg 2 */
	SQ_ALG2,		/* size of number to use square alg 2 */
	POW_ALG2,		/* size of modulus to use REDC for powers */
	REDC_ALG2,		/* size of modulus to use REDC algorithm 2 */
	TRUE,			/* ok to print a tilde on aproximations */
	TRUE,			/* ok to print tab before numeric values */
	0,			/* quomod() default rounding mode */
	2,			/* quotent // default rounding mode */
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
	0,			/* calc library debug level */
	0,			/* internal calc debug level */
	0 			/* user defined debug level */
};
CONFIG newstd = {	/* new non-backward compatible configuration */
	MODE_INITIAL,		/* current output mode */
	10,			/* current output digits for float or exp */
	NULL,	/* loaded in at startup - default error for real functions */
	NEW_EPSILONPREC_DEFAULT,	/* binary precision of epsilon */
	FALSE,			/* tracing flags */
	MAXPRINT_DEFAULT,	/* number of elements to print */
	MUL_ALG2,		/* size of number to use multiply alg 2 */
	SQ_ALG2,		/* size of number to use square alg 2 */
	POW_ALG2,		/* size of modulus to use REDC for powers */
	REDC_ALG2,		/* size of modulus to use REDC algorithm 2 */
	TRUE,			/* ok to print a tilde on aproximations */
	TRUE,			/* ok to print tab before numeric values */
	0,			/* quomod() default rounding mode */
	0,			/* quotent // default rounding mode */
	0,			/* mod % default rounding mode */
	24,			/* sqrt() default rounding mode */
	24,			/* appr() default rounding mode */
	0,			/* cfappr() default rounding mode */
	8,			/* cfsim() default rounding mode */
	24,			/* output default rounding mode */
	24,			/* round()/bround() default rounding mode */
	TRUE,			/* ok to print leading 0 before decimal pt */
	1,			/* ok to print trailing 0's */
	MAXSCANCOUNT,		/* max scan errors before abort */
	"; ",			/* normal prompt */
	";; ",			/* prompt when inside multi-line input */
	BLK_DEF_MAXPRINT,	/* number of octets of a block to print */
	FALSE,			/* skip duplicate block output lines */
	BLK_BASE_HEX,		/* block octet print base */
	BLK_FMT_HD_STYLE,	/* block output format */
	0,			/* calc library debug level */
	0,			/* internal calc debug level */
	0 			/* user defined debug level */
};
CONFIG *conf = NULL;	/* loaded in at startup - current configuration */


/*
 * Possible output modes.
 */
static NAMETYPE modes[] = {
	{"frac",	MODE_FRAC},
	{"decimal",	MODE_FRAC},
	{"dec",		MODE_FRAC},
	{"int",		MODE_INT},
	{"real",	MODE_REAL},
	{"exp",		MODE_EXP},
	{"hexadecimal",	MODE_HEX},
	{"hex",		MODE_HEX},
	{"octal",	MODE_OCTAL},
	{"oct",		MODE_OCTAL},
	{"binary",	MODE_BINARY},
	{"bin",		MODE_BINARY},
	{NULL,		0}
};


/*
 * Possible binary config state values
 */
static NAMETYPE truth[] = {
	{"y",		TRUE},
	{"n",		FALSE},
	{"yes",		TRUE},
	{"no",		FALSE},
	{"set",		TRUE},
	{"unset",	FALSE},
	{"on",		TRUE},
	{"off",		FALSE},
	{"true",	TRUE},
	{"false",	FALSE},
	{"t",		TRUE},
	{"f",		FALSE},
	{"1",		TRUE},
	{"0",		FALSE},
	{NULL,		0}
};


/*
 * Possible block base output modes
 */
static NAMETYPE blk_base[] = {
	{"hexadecimal",	BLK_BASE_HEX},
	{"hex",		BLK_BASE_HEX},
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
static NAMETYPE blk_fmt[] = {
	{"line",	BLK_FMT_LINE},
	{"lines",	BLK_FMT_LINE},
	{"str",		BLK_FMT_STRING},
	{"string",	BLK_FMT_STRING},
	{"strings",	BLK_FMT_STRING},
	{"od",		BLK_FMT_OD_STYLE},
	{"odstyle",	BLK_FMT_OD_STYLE},
	{"od_style",	BLK_FMT_OD_STYLE},
	{"hd",		BLK_FMT_HD_STYLE},
	{"hdstyle",	BLK_FMT_HD_STYLE},
	{"hd_style",	BLK_FMT_HD_STYLE},
	{NULL,		0}
};


/*
 * declate static functions
 */
static int modetype(char *name);
static int blkbase(char *name);
static int blkfmt(char *name);
static int truthtype(char *name);
static char *modename(int type);


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
 * Given the name of a mode, convert it to the internal format.
 * Returns -1 if the string is unknown.
 *
 * given:
 *	name			mode name
 */
static int
modetype(char *name)
{
	NAMETYPE *cp;		/* current config pointer */

	for (cp = modes; cp->name; cp++) {
		if (strcmp(cp->name, name) == 0)
			return cp->type;
	}
	return -1;
}


/*
 * Given the name of a block output base, convert it to the internal format.
 * Returns -1 if the string is unknown.
 *
 * given:
 *	name			mode name
 */
static int
blkbase(char *name)
{
	NAMETYPE *cp;		/* current config pointer */

	for (cp = blk_base; cp->name; cp++) {
		if (strcmp(cp->name, name) == 0)
			return cp->type;
	}
	return -1;
}


/*
 * Given the name of a block output format, convert it to the internal format.
 * Returns -1 if the string is unknown.
 *
 * given:
 *	name			mode name
 */
static int
blkfmt(char *name)
{
	NAMETYPE *cp;		/* current config pointer */

	for (cp = blk_fmt; cp->name; cp++) {
		if (strcmp(cp->name, name) == 0)
			return cp->type;
	}
	return -1;
}


/*
 * Given the name of a truth value, convert it to a BOOL or -1.
 * Returns -1 if the string is unknown.
 *
 * given:
 *	name			mode name
 */
static int
truthtype(char *name)
{
	NAMETYPE *cp;		/* current config pointer */

	for (cp = truth; cp->name; cp++) {
		if (strcmp(cp->name, name) == 0)
			return cp->type;
	}
	return -1;
}


/*
 * Given the mode type, convert it to a string representing that mode.
 * Where there are multiple strings representing the same mode, the first
 * one in the table is used.  Returns NULL if the node type is unknown.
 * The returned string cannot be modified.
 */
static char *
modename(int type)
{
	NAMETYPE *cp;		/* current config pointer */

	for (cp = modes; cp->name; cp++) {
		if (type == cp->type)
			return cp->name;
	}
	return NULL;
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
	char *p;

	switch (type) {
	case CONFIG_ALL:
		newconf = NULL;	/* firewall */
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
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for display");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num))
			temp = -1;
		math_setdigits(temp);
		break;

	case CONFIG_MODE:
		if (vp->v_type != V_STR) {
			math_error("Non-string for mode");
			/*NOTREACHED*/
		}
		temp = modetype(vp->v_str->s_str);
		if (temp < 0) {
			math_error("Unknown mode \"%s\"", vp->v_str);
			/*NOTREACHED*/
		}
		math_setmode((int) temp);
		break;

	case CONFIG_EPSILON:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for epsilon");
			/*NOTREACHED*/
		}
		setepsilon(vp->v_num);
		break;

	case CONFIG_MAXPRINT:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for maxprint");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num))
			temp = -1;
		if (temp < 0) {
			math_error("Maxprint value is out of range");
			/*NOTREACHED*/
		}
		conf->maxprint = temp;
		break;

	case CONFIG_MUL2:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for mul2");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q))
			temp = -1;
		if (temp == 0)
			temp = MUL_ALG2;
		if (temp < 2) {
			math_error("Illegal mul2 value");
			/*NOTREACHED*/
		}
		conf->mul2 = (int)temp;
		break;

	case CONFIG_SQ2:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for sq2");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q))
			temp = -1;
		if (temp == 0)
			temp = SQ_ALG2;
		if (temp < 2) {
			math_error("Illegal sq2 value");
			/*NOTREACHED*/
		}
		conf->sq2 = (int)temp;
		break;

	case CONFIG_POW2:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for pow2");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q))
			temp = -1;
		if (temp == 0)
			temp = POW_ALG2;
		if (temp < 1) {
			math_error("Illegal pow2 value");
			/*NOTREACHED*/
		}
		conf->pow2 = (int)temp;
		break;

	case CONFIG_REDC2:
		if (vp->v_type != V_NUM) {
			math_error("Non-numeric for redc2");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q))
			temp = -1;
		if (temp == 0)
			temp = REDC_ALG2;
		if (temp < 1) {
			math_error("Illegal redc2 value");
			/*NOTREACHED*/
		}
		conf->redc2 = (int)temp;
		break;

	case CONFIG_TILDE:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->tilde_ok = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = truthtype(vp->v_str->s_str);
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
			temp = truthtype(vp->v_str->s_str);
			if (temp < 0) {
				math_error("Illegal truth value for tab");
				/*NOTREACHED*/
			}
			conf->tab_ok = (int)temp;
		}
		break;

	case CONFIG_QUOMOD:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for quomod");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal quomod parameter value");
			/*NOTREACHED*/
		}
		conf->quomod = temp;
		break;

	case CONFIG_QUO:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for quo");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal quo parameter value");
			/*NOTREACHED*/
		}
		conf->quo = temp;
		break;

	case CONFIG_MOD:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for mod");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal mod parameter value");
			/*NOTREACHED*/
		}
		conf->mod = temp;
		break;

	case CONFIG_SQRT:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for sqrt");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal sqrt parameter value");
			/*NOTREACHED*/
		}
		conf->sqrt = temp;
		break;

	case CONFIG_APPR:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for appr");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal appr parameter value");
			/*NOTREACHED*/
		}
		conf->appr = temp;
		break;

	case CONFIG_CFAPPR:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for cfappr");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal cfappr parameter value");
			/*NOTREACHED*/
		}
		conf->cfappr = temp;
		break;

	case CONFIG_CFSIM:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for cfsim");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal cfsim parameter value");
			/*NOTREACHED*/
		}
		conf->cfsim = temp;
		break;

	case CONFIG_OUTROUND:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for outround");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal output parameter value");
			/*NOTREACHED*/
		}
		conf->outround = temp;
		break;

	case CONFIG_ROUND:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for round");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || qisneg(q) || !zistiny(q->num)) {
			math_error("Illegal output parameter value");
			/*NOTREACHED*/
		}
		conf->round = temp;
		break;

	case CONFIG_LEADZERO:
		if (vp->v_type == V_NUM) {
			q = vp->v_num;
			conf->leadzero = !qiszero(q);
		} else if (vp->v_type == V_STR) {
			temp = truthtype(vp->v_str->s_str);
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
			temp = truthtype(vp->v_str->s_str);
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
		strcpy(p, vp->v_str->s_str);
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
		strcpy(p, vp->v_str->s_str);
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
			temp = truthtype(vp->v_str->s_str);
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
		temp = blkbase(vp->v_str->s_str);
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
		temp = blkfmt(vp->v_str->s_str);
		if (temp < 0) {
			math_error("Unknown mode \"%s\" for blkfmt",
			    vp->v_str->s_str);
			/*NOTREACHED*/
		}
		conf->blkfmt = temp;
		break;

	case CONFIG_LIB_DEBUG:
		if (vp->v_type != V_NUM) {
			math_error("Non numeric for lib_debug");
			/*NOTREACHED*/
		}
		q = vp->v_num;
		temp = qtoi(q);
		if (qisfrac(q) || !zistiny(q->num)) {
			math_error("Illegal lib_debug parameter value");
			/*NOTREACHED*/
		}
		conf->lib_debug = temp;
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

	default:
		math_error("Setting illegal config parameter");
		/*NOTREACHED*/
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
	*dest = *src;

	/*
	 * clone the pointer values
	 */
	dest->epsilon = qlink(src->epsilon);
	dest->prompt1 = (char *)malloc(strlen(src->prompt1)+1);
	if (dest->prompt1 == NULL) {
		math_error("cannot dup prompt1 for new CONFIG value");
		/*NOTREACHED*/
	}
	strcpy(dest->prompt1, src->prompt1);
	dest->prompt2 = (char *)malloc(strlen(src->prompt2)+1);
	if (dest->prompt2 == NULL) {
		math_error("cannot dup prompt2 for new CONFIG value");
		/*NOTREACHED*/
	}
	strcpy(dest->prompt2, src->prompt2);

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
		vp->v_str = makenewstring(modename(cfg->outmode));
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
		i = cfg->tilde_ok;
		break;

	case CONFIG_TAB:
		i = cfg->tab_ok;
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
		i = cfg->leadzero;
		break;

	case CONFIG_FULLZERO:
		i = cfg->fullzero;
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
		i = cfg->blkverbose;
		break;

	case CONFIG_BLKBASE:
		i = cfg->blkbase;
		break;

	case CONFIG_BLKFMT:
		i = cfg->blkfmt;
		break;

	case CONFIG_LIB_DEBUG:
		i = cfg->lib_debug;
		break;

	case CONFIG_CALC_DEBUG:
		i = cfg->calc_debug;
		break;

	case CONFIG_USER_DEBUG:
		i = cfg->user_debug;
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
		math_error("CONFIG #1 value is invaid");
		/*NOTREACHED*/
	}
	if (cfg2 == NULL || cfg2->epsilon == NULL || cfg2->prompt1 == NULL ||
	    cfg2->prompt2 == NULL) {
		math_error("CONFIG #2 value is invaid");
		/*NOTREACHED*/
	}

	/*
	 * compare
	 */
	return cfg1->traceflags != cfg2->traceflags ||
	       cfg1->outdigits != cfg2->outdigits ||
	       cfg1->outmode != cfg2->outmode ||
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
	       cfg1->lib_debug != cfg2->lib_debug ||
	       cfg1->calc_debug != cfg2->calc_debug ||
	       cfg1->user_debug != cfg2->user_debug;
}
