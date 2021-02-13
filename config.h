/*
 * config - configuration routines
 *
 * Copyright (C) 1999-2007,2014,2021  Landon Curt Noll and David I. Bell
 *
 * Primary author:  Landon Curt Noll
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
 * Under source code control:	1995/11/01 22:20:17
 * File existed as early as:	1995
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_CONFIG_H)
#define INCLUDE_CONFIG_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "decl.h"
# include "nametype.h"
# include "qmath.h"
#else
# include <calc/decl.h>
# include <calc/nametype.h>
# include <calc/qmath.h>
#endif


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
#define CONFIG_BLKBASE	28
#define CONFIG_BLKFMT	29
#define CONFIG_RESOURCE_DEBUG 30
#define CONFIG_LIB_DEBUG CONFIG_RESOURCE_DEBUG
#define CONFIG_CALC_DEBUG 31
#define CONFIG_USER_DEBUG 32
#define CONFIG_VERBOSE_QUIT 33
#define CONFIG_CTRL_D	34
#define CONFIG_PROGRAM	35
#define CONFIG_BASENAME	36
#define CONFIG_VERSION	37
#define CONFIG_WINDOWS	38
#define CONFIG_MODE2	39
#define CONFIG_CYGWIN	40
#define CONFIG_COMPILE_CUSTOM	41
#define CONFIG_ALLOW_CUSTOM	42
#define CONFIG_BASEB	43
#define CONFIG_REDECL_WARN	44
#define CONFIG_DUPVAR_WARN	45
#define CONFIG_HZ	46


/*
 * config default symbols
 */
#define DISPLAY_DEFAULT 20	/* default digits for float display */
#define EPSILON_DEFAULT "1e-20" /* allowed error for float calculations */
#define EPSILONPREC_DEFAULT 67	/* 67 ==> 2^-67 <= EPSILON_DEFAULT < 2^-66 */
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
 *			  config_value(), config_cmp(),
 *			  and perhaps config_copy(), config_free()
 *	config.h	- CONFIG_XYZ_SYMBOL (see above)
 */
struct config {
	int outmode;		/* current output mode */
	int outmode2;		/* current secondary output mode */
	LEN outdigits;		/* current output digits for float or exp */
	NUMBER *epsilon;	/* default error for real functions */
	    long epsilonprec;	/* epsilon binary precision (tied to epsilon) */
	FLAG traceflags;	/* tracing flags */
	LEN maxprint;		/* number of elements to print */
	LEN mul2;		/* size of number to use multiply algorithm 2 */
	LEN sq2;		/* size of number to use square algorithm 2 */
	LEN pow2;		/* size of modulus to use REDC for powers */
	LEN redc2;		/* size of modulus to use REDC algorithm 2 */
	BOOL tilde_ok;		/* ok to print a tilde on approximations */
	BOOL tab_ok;		/* ok to print tab before numeric values */
	LEN quomod;		/* quomod() default rounding mode */
	LEN quo;		/* quotient // default rounding mode */
	LEN mod;		/* mod % default rounding mode */
	LEN sqrt;		/* sqrt() default rounding mode */
	LEN appr;		/* appr() default rounding mode */
	LEN cfappr;		/* cfappr() default rounding mode */
	LEN cfsim;		/* cfsim() default rounding mode */
	LEN outround;		/* output default rounding mode */
	LEN round;		/* round()/bround() default rounding mode */
	BOOL leadzero;		/* ok to print leading 0 before decimal pt */
	BOOL fullzero;		/* ok to print trailing 0's */
	long maxscancount;	/* max scan errors before abort */
	char *prompt1;		/* normal prompt */
	char *prompt2;		/* prompt when inside multi-line input */
	int blkmaxprint;	/* octets of a block to print, 0 => all */
	BOOL blkverbose;	/* TRUE => print all lines if a block */
	int blkbase;		/* block output base */
	int blkfmt;		/* block output style */
	long calc_debug;	/* internal debug, see CALC_DEBUG_XYZ below */
	long resource_debug;	/* resource debug, see RSCDBG_XYZ below */
	long user_debug;	/* user defined debug value: 0 default */
	BOOL verbose_quit;	/* TRUE => print Quit or abort executed msg */
	int ctrl_d;		/* see CTRL_D_xyz below */
	char *program;		/* our name */
	char *base_name;	/* basename of our name */
	BOOL windows;		/* TRUE => running under MS windows */
	BOOL cygwin;		/* TRUE => compiled with cygwin */
	BOOL compile_custom;	/* TRUE => compiled with -DCUSTOM */
	BOOL *allow_custom;	/* ptr to if custom functions are allowed */
	char *version;		/* calc version string */
	int baseb;		/* base for calculations */
	BOOL redecl_warn;	/* TRUE => warn of redeclaring variables */
	BOOL dupvar_warn;	/* TRUE => warn of var name collisions */
};
typedef struct config CONFIG;


/*
 * resource_debug bit masks
 */
#define RSCDBG_STDIN_FUNC   (0x00000001)    /* interactive func define debug */
#define RSCDBG_FILE_FUNC    (0x00000002)    /* file read func define debug */
#define RSCDBG_FUNC_INFO    (0x00000004)    /* print extra info for show func */
#define RSCDBG_PRINT_DBG    (0x00000008)    /* print debug messages */
#define RSCDBG_MASK	    (0x0000000f)


/*
 * calc_debug bit masks
 */
#define CALCDBG_SYSTEM	    (0x00000001)    /* print system cmd prior to exec */
#define CALCDBG_FUNC_QUIT   (0x00000002)    /* active functions when quit */
#define CALCDBG_HASH_STATE  (0x00000004)    /* hash state details */
#define CALCDBG_BLOCK	    (0x00000008)    /* block debug */
#define CALCDBG_TTY	    (0x00000010)    /* report TTY state changes */
#define CALCDBG_RUNSTATE    (0x00000020)    /* report run_state changes */
#define CALCDBG_RAND	    (0x00000040)    /* report rand() activity */
#define CALCDBG_CUSTOM	    (0x00000080)    /* custom function debug */
#define CALCDBG_MASK	    (0x000000ff)

/*
 * ctrl-d meanings
 */
#define CTRL_D_VIRGIN_EOF  (0)	/* ^D only exits on virgin command lines */
#define CTRL_D_NEVER_EOF   (1)	/* ^D never exits, emacs binding meaning only */
#define CTRL_D_EMPTY_EOF   (2)	/* ^D always exits at start of line */


/*
 * global configuration states and aliases
 */
EXTERN CONFIG *conf;	/* current configuration */
EXTERN CONFIG oldstd;	/* old classic standard configuration */
EXTERN CONFIG newstd;	/* default compatible configuration */
E_FUNC char *calc_debug;	/* !=NULL => value of config("calc_debug") */
E_FUNC char *resource_debug; /* !=NULL => config("resource_debug") value */
E_FUNC char *user_debug;	/* !=NULL => value of config("user_debug") */


/*
 * configuration externals
 */
E_FUNC CONFIG *config_copy(CONFIG *src);
E_FUNC void config_free(CONFIG *cfg);
E_FUNC void config_print(CONFIG *cfg);
E_FUNC int configtype(char*);
E_FUNC void config_print(CONFIG*);
E_FUNC BOOL config_cmp(CONFIG*, CONFIG*);


#endif /* !INCLUDE_CONFIG_H */
