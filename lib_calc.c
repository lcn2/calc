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
 * chongo was here	/\../\
 */

#include <stdio.h>
#include <setjmp.h>
#include <pwd.h>

#include "calc.h"
#include "zmath.h"
#include "zrandom.h"
#include "conf.h"
#include "token.h"
#include "symbol.h"
#include "func.h"

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_stdlib.h"
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

/*
 * in case we do not have certain .h files
 */
#if !defined(HAVE_STDLIB_H) && !defined(HAVE_UNISTD_H)
#if !defined(HAVE_UID_T) && !defined(_UID_T)
typedef unsigned short uid_t;
#endif
extern char *getenv();
extern uid_t geteuid();
#endif


/*
 * Common definitions
 */
int new_std = FALSE;	/* TRUE (-n) => use newstd configuration */
int abortlevel;		/* current level of aborts */
BOOL inputwait;		/* TRUE if in a terminal input wait */
jmp_buf jmpbuf;		/* for errors */
int start_done = FALSE;	/* TRUE => start up processing finished */
char *program = "calc";	/* our name */
char cmdbuf[MAXCMD+1+1+1];	/* command line expression + "\n\0" + guard */


/*
 * global permissions
 */
int allow_read = TRUE;	/* FALSE => may not open any files for reading */
int allow_write = TRUE;	/* FALSE => may not open any files for writing */
int allow_exec = TRUE;	/* FALSE => may not execute any commands */


/*
 * global flags
 */
int p_flag = FALSE;	/* TRUE => pipe mode */
int q_flag = FALSE;	/* TRUE => don't execute rc files */
int u_flag = FALSE;	/* TRUE => unbuffer stdin and stdout */
int d_flag = FALSE;	/* TRUE => disable heading, lib_debug == 0 */


/*
 * global values
 */
char *calcpath;		/* $CALCPATH or default */
char *calcrc;		/* $CALCRC or default */
char *calcbindings;	/* $CALCBINDINGS or default */
char *home;		/* $HOME or default */
char *pager;		/* $PAGER or default */
char *shell;		/* $SHELL or default */
int stdin_tty = FALSE;	/* TRUE if stdin is a tty */
int interactive = FALSE; /* TRUE if interactive session (no cmd args) */
int post_init = FALSE;	/* TRUE setjmp for math_error is readready */

int no_env = FALSE;	/* TRUE (-e) => ignore env vars on startup */
int errmax = ERRMAX;	/* if >= 0,  maximum value for errcount */

NUMBER *epsilon_default;	/* default allowed error for float calcs */


/*
 * initialization functions
 */
extern void math_setfp(FILE *fp);
extern void file_init(void);
extern void zio_init(void);
extern void initialize(void);
extern void reinitialize(void);


/*
 * static declarations
 */
static int init_done = 0;		/* 1 => we already initialized */
static void initenv(void);


/*
 * libcalc_call_me_first - users of libcalc.a must call this function first!
 *
 * Anything that uses libcalc.a MUST call this function first before doing
 * any other libcalc.a processing.
 */
void
libcalc_call_me_first(void)
{
	/*
	 * do nothing if we are initialized already
	 */
	if (init_done) {
		return;
	}

	/*
	 * setup configuration values
	 */
	oldstd.epsilon = &_qonesqbase_; /* magic to fake early str2q() */
	conf = config_copy(&oldstd); /* more magic to fake early str2q() */
	conf->tab_ok = FALSE;
	oldstd.epsilon = str2q(EPSILON_DEFAULT);
	newstd.epsilon = str2q(NEW_EPSILON_DEFAULT);

	/*
	 * make oldstd our default config
	 */
	config_free(conf);
	if (new_std) {
		conf = config_copy(&newstd);
	} else {
		conf = config_copy(&oldstd);
	}

	/*
	 * -d turns off lib_debug
	 */
	if (d_flag) {
		conf->lib_debug = 0;
	}

	/*
	 * initialize
	 */
	initialize();

	/*
	 * ready to rock & roll ..
	 */
	init_done = 1;
	return;
}


/*
 * initialize - perform the required calc initializations
 */
void
initialize(void)
{
	/*
	 * ZVALUE io initialization
	 */
	zio_init();

	/*
	 * process the environment
	 */
	initenv();

	/*
	 * initialize I/O
	 */
	file_init();

	/*
	 * initialize file I/O
	 */
	resetinput();

	/*
	 * initialize calc internal data structures
	 */
	inittokens();
	initglobals();
	initfunctions();
	initstack();

	/*
	 * initialize calc math
	 */
	math_cleardiversions();
	math_setfp(stdout);
	math_setmode(MODE_INITIAL);
	math_setdigits((long)DISPLAY_DEFAULT);
	conf->maxprint = MAXPRINT_DEFAULT;
}


/*
 * reinitialize - reinitialize after a longjmp
 */
void
reinitialize(void)
{
	/*
	 * process commands (from stdin, not the command line)
	 */
	abortlevel = 0;
	_math_abort_ = FALSE;
	inputwait = FALSE;
	math_cleardiversions();
	math_setfp(stdout);
	resetscopes();
	resetinput();
	(void) openterminal();
}


/*
 * cvmalloc_error - for users of the SGI Workshop Debugger
 *
 * usage:
 *	message	- error message passed along via libmalloc_cv.a
 */
void
cvmalloc_error(char *message)
{
	/* firewall */
	if (message == NULL) {
		fprintf(stderr, "cvmalloc_error message is NULL\n");
		return;
	}

	/* print message and return */
	fprintf(stderr, "cvmalloc_error: %s\n", message);
	return;
}


/*
 * initenv - obtain $CALCPATH, $CALCRC, $CALCBINDINGS, $HOME, $PAGER
 * and $SHELL values
 *
 * If $CALCPATH, $CALCRC, $CALCBINDINGS, $PAGER or $SHELL do not exist,
 * use the default values.  If $PAGER or $SHELL is an empty string, also
 * use a default value. If $HOME does not exist, or is empty, use the home
 * directory information from the password file.
 */
static void
initenv(void)
{
	struct passwd *ent;		/* our password entry */

	/* determine the $CALCPATH value */
	calcpath = (no_env ? NULL : getenv(CALCPATH));
	if (calcpath == NULL)
		calcpath = DEFAULTCALCPATH;

	/* determine the $CALCRC value */
	calcrc = (no_env ? NULL : getenv(CALCRC));
	if (calcrc == NULL) {
		calcrc = DEFAULTCALCRC;
	}
	if (strlen(calcrc) > MAX_CALCRC) {
		math_error("The $CALCRC variable is longer than %d chars",
			   MAX_CALCRC);
		/*NOTREACHED*/
	}

	/* determine the $CALCBINDINGS value */
	calcbindings = (no_env ? NULL : getenv(CALCBINDINGS));
	if (calcbindings == NULL) {
		calcbindings = DEFAULTCALCBINDINGS;
	}

	/* determine the $HOME value */
	home = (no_env ? NULL : getenv(HOME));
	if (home == NULL || home[0] == '\0') {
		ent = (struct passwd *)getpwuid(geteuid());
		if (ent == NULL) {
			/* just assume . is home if all else fails */
			home = ".";
		}
		home = (char *)malloc(strlen(ent->pw_dir)+1);
		strcpy(home, ent->pw_dir);
	}

	/* determine the $PAGER value */
	pager = (no_env ? NULL : getenv(PAGER));
	if (pager == NULL || *pager == '\0') {
		pager = DEFAULTCALCPAGER;
	}

	/* determine the $SHELL value */
	shell = (no_env ? NULL : getenv(SHELL));
	if (shell == NULL)
		shell = DEFAULTSHELL;
}


/*
 * libcalc_call_me_last - users of libcalc.a can call this function when done
 *
 * Anything that uses libcalc.a can call this function after they are
 * completely finished with libcalc.a processing.  The only effect of
 * this funcion is to free storage that might otherwise go unused.
 *
 * NOTE: If, for any reason, you need to do more libcalc.a processing,
 *	 then you will need to call libcalc_call_me_first() again.
 */
void
libcalc_call_me_last(void)
{
	/*
	 * firewall
	 */
	if (init_done == 0) {
		return;
	}

	/*
	 * free the configuration
	 */
	config_free(conf);

	/*
	 * free Blum generator state
	 */
	random_libcalc_cleanup();

	/*
	 * all done
	 */
	init_done = 0;
	return;
}
