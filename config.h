/*
 * Copyright (c) 1995 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *	supporting documentation
 *	source copies
 *	source works derived from this source
 *	binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Prior to calc 2.9.3t9, these routines existed as a calc library called
 * cryrand.cal.  They have been rewritten in C for performance as well
 * as to make them available directly from libcalc.a.
 *
 * Comments, suggestions, bug fixes and questions about these routines
 * are welcome.  Send EMail to the address given below.
 *
 * Happy bit twiddling,
 *
 *			Landon Curt Noll
 *
 *			chongo@toad.com
 *			...!{pyramid,sun,uunet}!hoptoad!chongo
 *
 * chongo was here	/\../\
 */

#if !defined(CONFIG_H)
#define CONFIG_H

#include "qmath.h"


/*
 * configuration element types
 */
#define CONFIG_ALL	0	/* not a real configuration parameter */
#define CONFIG_MODE	1	/* types of configuration parameters */
#define CONFIG_DISPLAY	2
#define CONFIG_EPSILON	3
#define CONFIG_EPSILONPREC 3	/* not a real type -- tied to CONFIG_EPSILON */
#define CONFIG_TRACE	4
#define CONFIG_MAXPRINT	5
#define	CONFIG_MUL2	6
#define	CONFIG_SQ2	7
#define	CONFIG_POW2	8
#define	CONFIG_REDC2	9
#define CONFIG_TILDE	10
#define CONFIG_TAB	11
#define CONFIG_QUOMOD	12
#define CONFIG_QUO	13
#define CONFIG_MOD	14
#define CONFIG_SQRT	15
#define CONFIG_APPR	16
#define CONFIG_CFAPPR	17
#define CONFIG_CFSIM	18
#define CONFIG_OUTROUND	19
#define CONFIG_ROUND	20
#define CONFIG_LEADZERO	21
#define CONFIG_FULLZERO	22
#define CONFIG_MAXERR	23
#define CONFIG_PROMPT	24
#define CONFIG_MORE	25
#define CONFIG_RANDOM	26


/*
 * config defult symbols
 */
#define DISPLAY_DEFAULT 20	/* default digits for float display */
#define EPSILON_DEFAULT "1e-20"	/* allowed error for float calculations */
#define EPSILONPREC_DEFAULT 67	/* 67 ==> 2^-67 <= EPSILON_DEFAULT < 2^-66 */
#define NEW_EPSILON_DEFAULT "1e-10"	/* newstd EPSILON_DEFAULT */
#define NEW_EPSILONPREC_DEFAULT 34	/* 34 ==> 2^-34 <= 1e-10 < 2^-33 */
#define MAXPRINT_DEFAULT 16	/* default number of elements printed */
#define MAXERRORCOUNT 20	/* default max errors before an abort */


/*
 * configuration object
 */
struct config {
	int outmode;		/* current output mode */
	long outdigits;		/* current output digits for float or exp */
	NUMBER *epsilon;	/* default error for real functions */
	    long epsilonprec;	/* epsilon binary precision (tied to epsilon) */
	FLAG traceflags;	/* tracing flags */
	long maxprint;		/* number of elements to print */
	LEN mul2;		/* size of number to use multiply algorithm 2 */
	LEN sq2;		/* size of number to use square algorithm 2 */
	LEN pow2;		/* size of modulus to use REDC for powers */
	LEN redc2;		/* size of modulus to use REDC algorithm 2 */
	int tilde_ok;		/* ok to print a tilde on aproximations */
	int tab_ok;		/* ok to print tab before numeric values */
	long quomod;		/* quomod() default rounding mode */
	long quo;		/* quotent // default rounding mode */
	long mod;		/* mod % default rounding mode */
	long sqrt;		/* sqrt() default rounding mode */
	long appr;		/* appr() default rounding mode */
	long cfappr;		/* cfappr() default rounding mode */
	long cfsim;		/* cfsim() default rounding mode */
	long outround;		/* output default rounding mode */
	long round;		/* round()/bround() default rounding mode */
	int leadzero;		/* ok to print leading 0 before decimal pt */
	int fullzero;		/* ok to print trailing 0's -- XXX ??? */
	long maxerrorcount;	/* max errors before abort */
	char *prompt1;		/* normal prompt */
	char *prompt2;		/* prompt when inside multi-line input */
	int random;		/* random mode */
};
typedef	struct config CONFIG;


/*
 * global configuration states and aliases
 */
extern CONFIG *conf;		/* current configuration */
extern CONFIG oldstd;		/* backward compatible standard configuration */
extern CONFIG newstd;		/* new non-backward compatible configuration */


/*
 * configuration functions
 */
extern CONFIG *config_copy(CONFIG *src);
extern void config_free(CONFIG *cfg);
extern void config_print(CONFIG *cfg);
extern BOOL config_cmp(CONFIG *cfg1, CONFIG *cfg2);
extern int configtype(char *name);


#endif
