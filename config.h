/*
 * Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
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
 * cryrand.cal.	 They have been rewritten in C for performance as well
 * as to make them available directly from libcalc.a.
 *
 * Comments, suggestions, bug fixes and questions about these routines
 * are welcome.	 Send EMail to the address given below.
 *
 * Happy bit twiddling,
 *
 *	Landon Curt Noll
 *	http://reality.sgi.com/chongo/
 *
 * chongo <was here> /\../\
 */


#if !defined(__CONFIG_H__)
#define __CONFIG_H__


#include "nametype.h"
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
#define CONFIG_MAXPRINT 5
#define CONFIG_MUL2	6
#define CONFIG_SQ2	7
#define CONFIG_POW2	8
#define CONFIG_REDC2	9
#define CONFIG_TILDE	10
#define CONFIG_TAB	11
#define CONFIG_QUOMOD	12
#define CONFIG_QUO	13
#define CONFIG_MOD	14
#define CONFIG_SQRT	15
#define CONFIG_APPR	16
#define CONFIG_CFAPPR	17
#define CONFIG_CFSIM	18
#define CONFIG_OUTROUND 19
#define CONFIG_ROUND	20
#define CONFIG_LEADZERO 21
#define CONFIG_FULLZERO 22
#define CONFIG_MAXSCAN	23
#define CONFIG_PROMPT	24
#define CONFIG_MORE	25
#define CONFIG_BLKMAXPRINT 26
#define CONFIG_BLKVERBOSE 27
#define CONFIG_BLKBASE 28
#define CONFIG_BLKFMT 29
#define CONFIG_LIB_DEBUG 30
#define CONFIG_CALC_DEBUG 31
#define CONFIG_USER_DEBUG 32
#define CONFIG_VERBOSE_QUIT 33
#define CONFIG_CTRL_D 34


/*
 * config default symbols
 */
#define DISPLAY_DEFAULT 20	/* default digits for float display */
#define EPSILON_DEFAULT "1e-20" /* allowed error for float calculations */
#define EPSILONPREC_DEFAULT 67	/* 67 ==> 2^-67 <= EPSILON_DEFAULT < 2^-66 */
#define NEW_EPSILON_DEFAULT "1e-10"	/* newstd EPSILON_DEFAULT */
#define NEW_EPSILONPREC_DEFAULT 34	/* 34 ==> 2^-34 <= 1e-10 < 2^-33 */
#define MAXPRINT_DEFAULT 16	/* default number of elements printed */
#define MAXSCANCOUNT 20		/* default max scan errors before an abort */


/*
 * configuration object
 *
 * If you add elements to this structure, you need to also update:
 *
 *	quickhash.c	- config_hash()
 *	hash.c		- hash_value()
 *	config.c	- configs[], oldstd, newstd, setconfig(),
 *			  config_value(), config_cmp()
 *	config.h	- CONFIG_XYZ_SYMBOL (see above)
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
	BOOL tilde_ok;		/* ok to print a tilde on aproximations */
	BOOL tab_ok;		/* ok to print tab before numeric values */
	long quomod;		/* quomod() default rounding mode */
	long quo;		/* quotent // default rounding mode */
	long mod;		/* mod % default rounding mode */
	long sqrt;		/* sqrt() default rounding mode */
	long appr;		/* appr() default rounding mode */
	long cfappr;		/* cfappr() default rounding mode */
	long cfsim;		/* cfsim() default rounding mode */
	long outround;		/* output default rounding mode */
	long round;		/* round()/bround() default rounding mode */
	BOOL leadzero;		/* ok to print leading 0 before decimal pt */
	BOOL fullzero;		/* ok to print trailing 0's */
	long maxscancount;	/* max scan errors before abort */
	char *prompt1;		/* normal prompt */
	char *prompt2;		/* prompt when inside multi-line input */
	int blkmaxprint;	/* octets of a block to print, 0 => all */
	BOOL blkverbose;	/* TRUE => print all lines if a block */
	int blkbase;		/* block output base */
	int blkfmt;		/* block output style */
	long calc_debug;	/* internal debug, see CALC_DEBUG_XXX below */
	long lib_debug;		/* library debug, see LIB_DEBUG_XXX below */
	long user_debug;	/* user defined debug value: 0 default */
	BOOL verbose_quit;	/* TRUE => print Quit or abort executed msg */
	int ctrl_d;		/* see CTRL_D_xyz below */
};
typedef struct config CONFIG;


/*
 * lib_debug bit masks
 */
#define LIBDBG_STDIN_FUNC   (0x00000001)    /* interactive func define debug */
#define LIBDBG_FILE_FUNC    (0x00000002)    /* file read func define debug */
#define LIBDBG_FUNC_INFO    (0x00000004)    /* print extra info for show func */
#define LIBDBG_MASK	    (0x00000007)


/*
 * calc_debug bit masks
 */
#define CALCDBG_SYSTEM	    (0x00000001)    /* print system cmd prior to exec */
#define CALCDBG_FUNC_QUIT   (0x00000002)    /* active functions when quit */
#define CALCDBG_HASH_STATE  (0x00000004)    /* hash state details */
#define CALCDBG_BLOCK	    (0x00000008)    /* block debug */
#define CALCDBG_TTY	    (0x00000010)    /* report TTY state changes */
#define CALCDBG_RUNSTATE    (0x00000020)    /* report run_state changes */
#define CALCDBG_MASK	    (0x0000003f)

/*
 * ctrl-d meanings
 */
#define CTRL_D_VIRGIN	(0)	/* ^D only exits on virgin command lines */
#define CTRL_D_EMACS	(1)	/* ^D never exits, emacs binding meaning only */
#define CTRL_D_EOF	(2)	/* ^D always exits at start of line */


/*
 * global configuration states and aliases
 */
extern CONFIG *conf;		/* current configuration */
extern CONFIG oldstd;		/* backward compatible standard configuration */
extern CONFIG newstd;		/* new non-backward compatible configuration */
extern char *calc_debug;	/* !=NULL => value of config("calc_debug") */
extern char *lib_debug;		/* !=NULL => value of config("lib_debug") */
extern char *user_debug;	/* !=NULL => value of config("user_debug") */


/*
 * configuration externals
 */
extern CONFIG *config_copy(CONFIG *src);
extern void config_free(CONFIG *cfg);
extern void config_print(CONFIG *cfg);
extern int configtype(char*);
extern void config_print(CONFIG*);
extern BOOL config_cmp(CONFIG*, CONFIG*);


#endif /* !__CONFIG_H__ */
