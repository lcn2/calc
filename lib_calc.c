/*
 * lib_calc - calc link library initialization and shutdown routines
 *
 * Copyright (C) 1999  Landon Curt Noll
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.1 $
 * @(#) $Id: lib_calc.c,v 29.1 1999/12/14 09:16:11 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/lib_calc.c,v $
 *
 * Under source code control:	1996/06/17 18:06:19
 * File existed as early as:	1996
 *
 * chongo <was here> /\oo/\	http://reality.sgi.com/chongo/
 * Share and enjoy!  :-)	http://reality.sgi.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <pwd.h>

#include "calc.h"
#include "zmath.h"
#include "zrandom.h"
#include "conf.h"
#include "token.h"
#include "symbol.h"
#include "func.h"

#include "have_strdup.h"
#if !defined(HAVE_STRDUP)
# define strdup(x) calc_strdup((CONST char *)(x))
#endif /* HAVE_STRDUP */

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_stdlib.h"
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#include "terminal.h"
#if defined(USE_TERMIOS)

# include <termios.h>
typedef struct termios ttystruct;

#elif defined(USE_TERMIOS)

# include <termio.h>
typedef struct termio ttystruct;

#else /* assume USE_SGTTY */

# include <sys/ioctl.h>
typedef struct sgttyb ttystruct;

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
char *program = "calc";		/* our name */
char *basename = "calc";	/* basename of our name */
char cmdbuf[MAXCMD+1+1+1];	/* command line expression + "\n\0" + guard */
run run_state = RUN_UNKNOWN;	/* calc startup and run state */


/*
 * global permissions
 */
int allow_read = TRUE;	/* FALSE => may not open any files for reading */
int allow_write = TRUE; /* FALSE => may not open any files for writing */
int allow_exec = TRUE;	/* FALSE => may not execute any commands */


/*
 * global flags
 */
int p_flag = FALSE;	/* TRUE => pipe mode */
int q_flag = FALSE;	/* TRUE => don't execute rc files */
int u_flag = FALSE;	/* TRUE => unbuffer stdin and stdout */
int d_flag = FALSE;	/* TRUE => disable heading, resource_debug == 0 */
int c_flag = FALSE;	/* TRUE => continue on error if permitted */
int i_flag = FALSE;	/* TRUE => go interactive if permitted */
int s_flag = FALSE;	/* TRUE => keep args as strings for argv() */


/*
 * global values
 */
char *calcpath = NULL;		/* $CALCPATH or default */
char *calcrc = NULL;		/* $CALCRC or default */
char *calcbindings = NULL;	/* $CALCBINDINGS or default */
char *home = NULL;		/* $HOME or default */
char *pager = NULL;		/* $PAGER or default */
char *shell = NULL;		/* $SHELL or default */
int stdin_tty = FALSE;		/* TRUE if stdin is a tty */
int havecommands = FALSE;	/* TRUE if have one or more cmd args */
int stoponerror = FALSE;	/* >0 => stop, <0 => continue on error */
int post_init = FALSE;		/* TRUE setjmp for math_error is ready */
BOOL abort_now = FALSE;		/* TRUE => go interactive now, if permitted */

int argc_value = 0;		/* count of argv[] strings for argv() builtin */
char **argv_value = NULL;	/* argv[] strings for argv() builtin */

int no_env = FALSE;		/* TRUE (-e) => ignore env vars on startup */
int errmax = ERRMAX;		/* if >= 0,  maximum value for errcount */

NUMBER *epsilon_default;	/* default allowed error for float calcs */

char *calc_debug = NULL;	/* !=NULL => value of config("calc_debug") */
char *resource_debug = NULL;	/* !=NULL => config("resource_debug") value */
char *user_debug = NULL;	/* !=NULL => value of config("user_debug") */


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
static int *fd_setup = NULL;		/* fd's setup for interaction or -1 */
static int fd_setup_len = 0;		/* number of fd's in fd_setup */
static ttystruct *fd_orig = NULL;	/* fd original state */
static ttystruct *fd_cur = NULL;	/* fd current atate */
static void initenv(void);		/* setup calc environment */
static int find_tty_state(int fd);	/* find slot for saved tty state */


/*
 * libcalc_call_me_first - users of libcalc.a must call this function first!
 *
 * Anything that uses libcalc.a MUST call this function first before doing
 * any other libcalc.a processing.
 */
void
libcalc_call_me_first(void)
{
	char *p;

	/*
	 * do nothing if we are initialized already
	 */
	if (init_done) {
		return;
	}

	/*
	 * Disable SIGPIPE so that the pipe to the help file pager will
	 * not stop calc.
	 */
	(void) signal(SIGPIPE, SIG_IGN);

	/*
	 * determine the basename
	 */
	if (program != NULL) {
		p = strrchr(program, '/');
		if (p == NULL) {
			basename = program;
		} else {
			basename = p+1;
		}
	}

	/*
	 * initialize old and new configuration values
	 */
	oldstd.epsilon = &_qonesqbase_; /* magic to fake early str2q() */
	oldstd.program = strdup(program);
	oldstd.basename = strdup(basename);
	oldstd.version = strdup(version());
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
	 * -d turns off resource_debug
	 */
	if (d_flag) {
		conf->resource_debug = 0;
	}

	/*
	 * -p turns off tab
	 */
	if (p_flag) {
		conf->tab_ok = 0;
	}

	/*
	 * -D flags can change calc_debug, resource_debug of user_debug
	 */
	if (calc_debug) {
		conf->calc_debug = strtol(calc_debug, NULL, 0);
	}
	if (resource_debug) {
		conf->resource_debug = strtol(resource_debug, NULL, 0);
	}
	if (user_debug) {
		conf->user_debug = strtol(user_debug, NULL, 0);
	}

	/*
	 * initialize
	 */
	initialize();

	/*
	 * ready to rock & roll ..
	 */
	if (conf->calc_debug & CALCDBG_RUNSTATE) {
		printf("libcalc_call_me_first: run_state from %s to %s\n",
		    run_state_name(run_state),
		    run_state_name(RUN_BEGIN));
	}
	run_state = RUN_BEGIN;
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
	inittokens();
	(void) openterminal();
}


/*
 * cvmalloc_error - for users of the SGI Workshop Debugger
 *
 * usage:
 *	message - error message passed along via libmalloc_cv.a
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
	char *c;

	/* determine the $CALCPATH value */
	c = (no_env ? NULL : getenv(CALCPATH));
	calcpath = (c ? strdup(c) : NULL);
	if (calcpath == NULL)
		calcpath = DEFAULTCALCPATH;

	/* determine the $CALCRC value */
	c = (no_env ? NULL : getenv(CALCRC));
	calcrc = (c ? strdup(c) : NULL);
	if (calcrc == NULL)
		calcrc = DEFAULTCALCRC;
	if (strlen(calcrc) > MAX_CALCRC) {
		math_error("The $CALCRC variable is longer than %d chars",
			   MAX_CALCRC);
		/*NOTREACHED*/
	}

	/* determine the $CALCBINDINGS value */
	c = (no_env ? NULL : getenv(CALCBINDINGS));
	calcbindings = (c ? strdup(c) : NULL);
	if (calcbindings == NULL)
		calcbindings = DEFAULTCALCBINDINGS;

	/* determine the $HOME value */
	c = (no_env ? NULL : getenv(HOME));
	home = (c ? strdup(c) : NULL);
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
	c = (no_env ? NULL : getenv(PAGER));
	pager = (c ? strdup(c) : NULL);
	if (pager == NULL || *pager == '\0')
		pager = DEFAULTCALCPAGER;

	/* determine the $SHELL value */
	c = (no_env ? NULL : getenv(SHELL));
	shell = (c ? strdup(c) : NULL);
	if (shell == NULL || *shell == '\0')
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
	int i;

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
	 * restore all changed descriptor states
	 */
	if (fd_setup_len > 0) {
		for (i=0; i < fd_setup_len; ++i) {
			if (fd_setup[i] >= 0) {
				if (conf->calc_debug & CALCDBG_TTY)
					printf("libcalc_call_me_last: fd %d "
					       "not in original state, "
					       "restoring it", fd_setup[i]);
				orig_tty(fd_setup[i]);
			}
		}
	}

	/*
	 * all done
	 */
	init_done = 0;
	return;
}


/*
 * run_state_name - return a constant string given a run_state
 */
char *
run_state_name(run state)
{
	switch (state) {
	case RUN_UNKNOWN:
		return "RUN_UNKNOWN";
	case RUN_BEGIN:
		return "RUN_BEGIN";
	case RUN_RCFILES:
		return "RUN_RCFILES";
	case RUN_PRE_CMD_ARGS:
		return "RUN_PRE_CMD_ARGS";
	case RUN_CMD_ARGS:
		return "RUN_CMD_ARGS";
	case RUN_PRE_TOP_LEVEL:
		return "RUN_PRE_TOP_LEVEL";
	case RUN_TOP_LEVEL:
		return "RUN_TOP_LEVEL";
	case RUN_EXIT:
		return "RUN_EXIT";
	case RUN_EXIT_WITH_ERROR:
		return "RUN_EXIT_WITH_ERROR";
	}
	return "RUN_invalid";
}


/*
 * calc_strdup - calc interface to provide or simulate strdup()
 */
char *
calc_strdup(CONST char *s1)
{
#if defined(HAVE_STRDUP)

	return strdup(s1);

#else /* HAVE_STRDUP */

	char *ret;	/* return string */

	/*
	 * firewall
	 */
	if (s1 == NULL) {
		return NULL;
	}

	/*
	 * allocate duplicate storage
	 */
	ret = (char *)malloc(sizeof(char) * (strlen(s1)+1));

	/*
	 * if we have storage, duplicate the string
	 */
	if (ret != NULL) {
		strcpy(ret, s1);
	}

	/*
	 * return the new string, or NULL if malloc failed
	 */
	return ret;

#endif /* HAVE_STRDUP */
}


/*
 * find_tty_state - establish a new tty state
 *
 * Given:
 *	fd	file descriptor to establish a new tty state for
 *
 * Returns:
 *	indx	The index into fd_setup[], fd_orig[] and fd_cur[] to use or -1
 */
static int
find_tty_state(int fd)
{
	int *new_fd_setup;		/* new fd_setup array */
	ttystruct *new_fd_orig; /* new fd_orig array */
	ttystruct *new_fd_cur;		/* new fd_cur array */
	int i;

	/*
	 * firewall: must be > 0
	 */
	if (fd < 0) {
		/* bad descriptor */
		return -1;
	}

	/*
	 * case: need to initlally malloc some state
	 */
	if (fd_setup_len <= 0 || fd_setup == NULL || fd_orig == NULL) {

		/* setup for a single descriptor */
		fd_setup = (int *)malloc(sizeof(fd_setup[0]));
		if (fd_setup == NULL) {
			return -1;
		}
		fd_setup[0] = -1;
		fd_orig = (ttystruct *)malloc(sizeof(fd_orig[0]));
		if (fd_orig == NULL) {
			return -1;
		}
		fd_cur = (ttystruct *)malloc(sizeof(fd_orig[0]));
		if (fd_cur == NULL) {
			return -1;
		}
		fd_setup_len = 1;
	}

	/*
	 * look for an existing tty state for the descriptor
	 */
	for (i=0; i < fd_setup_len; ++i) {

		/* case: found existing tty state, return index */
		if (fd_setup[i] == fd) {
			return i;
		}
	}

	/*
	 * no tty state exists for the descriptor, look for empty slot
	 */
	for (i=0; i < fd_setup_len; ++i) {

		/* case: found an empty slot, so return it */
		if (fd_setup[i] < 0) {
			return i;
		}
	}

	/*
	 * no empty slots exist, realloc another slot
	 */
	new_fd_setup = (int *)realloc(fd_setup, sizeof(fd_setup[0]) *
				      (fd_setup_len+1));
	if (new_fd_setup == NULL) {
		return -1;
	}
	new_fd_setup[fd_setup_len] = -1;
	new_fd_orig = (ttystruct *)realloc(fd_setup, sizeof(fd_orig[0]) *
					    (fd_setup_len+1));
	if (new_fd_orig == NULL) {
		return -1;
	}
	new_fd_cur = (ttystruct *)realloc(fd_cur, sizeof(fd_cur[0]) *
					  (fd_setup_len+1));
	if (new_fd_cur == NULL) {
		return -1;
	}
	fd_setup = new_fd_setup;
	fd_orig = new_fd_orig;
	fd_cur = new_fd_cur;
	++fd_setup_len;
	/* return the new slot */
	return fd_setup_len-1;
}


/*
 * calc_tty - setup a file descriptor for calc's interactive use
 *
 * Calc wants, in effect, cbreak on and echo off.
 *
 * Given:
 *	fd	the descriptor for calc's interactive use
 *
 * Returns:
 *	TRUE	state change was successful
 *	FALSE	unable to change state of descriptor for interactive use
 */
BOOL
calc_tty(int fd)
{
	int slot;	/* the saved descriptor slot or -1 */

	/*
	 * grab the saved slot for this descriptor
	 */
	slot = find_tty_state(fd);
	if (slot < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("calc_tty: Cannot get saved descriptor slot\n");
		return FALSE;
	}

#if defined(USE_SGTTY)

	/*
	 * USE_SGTTY tty state method
	 */
	/* save original state if needed */
	if (fd_setup[slot] < 0 && ioctl(fd, TIOCGETP, fd_orig+slot) < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("calc_tty: Cannot TIOCGETP fd %d\n", fd);
		return FALSE;
	}
	/* setup for new state */
	fd_cur[slot] = fd_orig[slot];
	fd_cur[slot].sg_flags &= ~ECHO;
	fd_cur[slot].sg_flags |= CBREAK;
	/* enable new state */
	if (ioctl(fd, TIOCSETP, fd_cur+slot) < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("calc_tty: Cannot TIOCSETP fd %d\n", fd);
		return FALSE;
	}
	if (conf->calc_debug & CALCDBG_TTY)
		printf("calc_tty: stty -ECHO +CBREAK: fd %d\n", fd);

#elif defined(USE_TERMIO)

	/*
	 * USE_TERMIO tty state method
	 */
	if (fd_setup[slot] < 0 && ioctl(fd, TCGETA, fd_orig+slot) < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("calc_tty: Cannot TCGETA fd %d\n", fd);
		return FALSE;
	}
	/* setup for new state */
	fd_cur[slot] = fd_orig[slot];
	fd_cur[slot].c_lflag &= ~(ECHO | ECHOE | ECHOK);
	fd_cur[slot].c_iflag |= ISTRIP;
	fd_cur[slot].c_lflag &= ~ICANON;
	fd_cur[slot].c_cc[VMIN] = 1;
	fd_cur[slot].c_cc[VTIME] = 0;
	/* enable new state */
	if (ioctl(fd, TCSETAW, fd_cur+slot) < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("calc_tty: Cannot TCSETAW fd %d\n", fd);
		return FALSE;
	}
	if (conf->calc_debug & CALCDBG_TTY)
		printf("calc_tty: stty -ECHO -ECHOE -ECHOK -ICANON +ISTRIP "
		       "VMIN=1 VTIME=0: fd %d\n", fd);

#else /* assume USE_SGTTY */

	/*
	 * assume USE_SGTTY tty state method
	 */
	if (fd_setup[slot] < 0 && tcgetattr(fd, fd_orig+slot) < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("calc_tty: Cannot tcgetattr fd %d\n", fd);
		return FALSE;
	}
	/* setup for new state */
	fd_cur[slot] = fd_orig[slot];
	fd_cur[slot].c_lflag &= ~(ECHO | ECHOE | ECHOK);
	fd_cur[slot].c_iflag |= ISTRIP;
	fd_cur[slot].c_lflag &= ~ICANON;
	fd_cur[slot].c_cc[VMIN] = 1;
	fd_cur[slot].c_cc[VTIME] = 0;
	/* enable new state */
	if (tcsetattr(fd, TCSANOW, fd_cur+slot) < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("calc_tty: Cannot tcsetattr fd %d\n", fd);
		return FALSE;
	}
	if (conf->calc_debug & CALCDBG_TTY)
		printf("calc_tty: stty -ECHO -ECHOE -ECHOK -ICANON +ISTRIP "
		       "VMIN=1 VTIME=0: fd %d\n", fd);
#endif

	/*
	 * note that the tty slot is on use
	 */
	fd_setup[slot] = fd;
	return TRUE;
}


/*
 * orig_tty - restore the original state of a file descriptor
 *
 * This routine will restore the state of a descriptor to its calc
 * startup value if it was set for interactive use by calc_tty().
 *
 * Given:
 *	fd	the descriptor to restore
 *
 * Returns:
 *	TRUE	restore was successful
 *	FALSE	unable to restore descriptor to original state
 */
BOOL
orig_tty(int fd)
{
	int slot;	/* the saved descriptor slot or -1 */

	/*
	 * find the saved slot for this descriptor
	 */
	slot = find_tty_state(fd);
	if (slot < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("orig_tty: Cannot get saved descriptor slot\n");
		return FALSE;
	}

	/*
	 * do nothing if no state was saved for this descriptor
	 */
	if (fd_setup[slot] < 0) {
		if (conf->calc_debug & CALCDBG_TTY)
			printf("orig_tty: no state saved for fd %d\n", fd);
		return FALSE;
	}

#if defined(USE_SGTTY)

	/*
	 * USE_SGTTY tty state method
	 */
	(void) ioctl(fd, TIOCSETP, fd_orig+slot);
	if (conf->calc_debug & CALCDBG_TTY)
		printf("orig_tty: TIOCSETP restored fd %d\n", fd);

#elif defined(USE_TERMIO)

	/*
	 * USE_TERMIO tty state method
	 */
	(void) ioctl(fd, TCSETAW, fd_orig+slot);
	if (conf->calc_debug & CALCDBG_TTY)
		printf("orig_tty: TCSETAW restored fd %d\n", fd);

#else /* assume USE_SGTTY */

	/*
	 * assume USE_SGTTY tty state method
	 */
	(void) tcsetattr(fd, TCSANOW, fd_orig+slot);
	if (conf->calc_debug & CALCDBG_TTY)
		printf("orig_tty: TCSANOW restored fd %d\n", fd);
#endif

	/*
	 * note new current state
	 */
	fd_cur[slot] = fd_orig[slot];

	/*
	 * Since current state is the orignal state, we can free up
	 * this slot.  This also prevents functins such as the
	 * libcalc_call_me_last() function from re-restoring it.
	 */
	fd_setup[slot] = -1;
	return TRUE;
}
