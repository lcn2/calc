/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Arbitrary precision calculator.
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#include <setjmp.h>

#define CALC_C
#include "calc.h"
#include "hist.h"
#include "func.h"
#include "opcodes.h"
#include "conf.h"
#include "token.h"
#include "symbol.h"
#include "have_uid_t.h"
#include "have_const.h"
#include "custom.h"
#include "math_error.h"
#include "args.h"

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_stdlib.h"
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif


/*
 * external and static definitions
 */
extern int abortlevel;		/* current level of aborts */
extern BOOL inputwait;		/* TRUE if in a terminal input wait */
extern jmp_buf jmpbuf;		/* for errors */
extern int isatty(int tty);	/* TRUE if fd is a tty */

extern int p_flag;		/* TRUE => pipe mode */
extern int q_flag;		/* TRUE => don't execute rc files */
extern int u_flag;		/* TRUE => unbuffer stdin and stdout */

extern char *pager;		/* $PAGER or default */
extern int stdin_tty;		/* TRUE if stdin is a tty */
extern char *program;		/* our name */
extern char cmdbuf[];		/* command line expression */

extern char *version(void);	/* return version string */


/*
 * forward static functions
 */
static void intint(int arg);	/* interrupt routine */


/*
 * Top level calculator routine.
 */
int
main(int argc, char **argv)
{
	static char *str;	/* current option string or expression */
	int want_defhelp = 0;	/* 1=> we only want the default help */
	long i;
	char *p;

	/*
	 * parse args
	 */
	program = argv[0];
	argc--;
	argv++;
	while ((argc > 0) && (**argv == '-')) {
		for (str = &argv[0][1]; *str; str++) switch (*str) {
			case 'C':
#if defined(CUSTOM)
				allow_custom = TRUE;
				break;
#else
				fprintf(stderr,
				    "Calc was built with custom functions "
				    "disabled, -C usage is disallowed\n");
				/*
				 * we are too early in processing to call
				 * libcalc_call_me_last() - nothing to cleanup
				 */
				exit(1);
#endif /* CUSTOM */
			case 'e':
				no_env = TRUE;
				break;
			case 'h':
				want_defhelp = 1;
				break;
			case 'i':
				ign_errmax = TRUE;
				break;
			case 'm':
				if (argv[0][2]) {
					p = &argv[0][2];
				} else if (argc > 1) {
					p = argv[1];
					argc--;
					argv++;
				} else {
					fprintf(stderr, "-m requires an arg\n");
					/*
					 * we are too early in processing to
					 * call libcalc_call_me_last()
					 * nothing to cleanup
					 */
					exit(1);
				}
				if (p[1] != '\0' || *p < '0' || *p > '7') {
					fprintf(stderr, "unknown -m arg\n");
					/*
					 * we are too early in processing to
					 * call libcalc_call_me_last()
					 * nothing to cleanup
					 */
					exit(1);
				}
				allow_read = (((*p-'0') & 04) > 0);
				allow_write = (((*p-'0') & 02) > 0);
				allow_exec = (((*p-'0') & 01) > 0);
				break;
			case 'n':
				new_std = TRUE;
				break;
			case 'p':
				p_flag = TRUE;
				break;
			case 'q':
				q_flag = TRUE;
				break;
			case 'u':
				u_flag = TRUE;
				break;
			case 'v':
				printf("%s (version %s)\n",
				    CALC_TITLE, version());
				/*
				 * we are too early in processing to call
				 * libcalc_call_me_last() - nothing to cleanup
				 */
				exit(0);
			default:
				fprintf(stderr,
			"usage: %s [-C] [-e] [-h] [-i] [-m mode] [-n] [-p]\n",
				    program);
				fprintf(stderr, "\t[-q] [-u] [calc_cmd ...]\n");
				/*
				 * we are too early in processing to call
				 * libcalc_call_me_last() - nothing to cleanup
				 */
				exit(1);
		}
		argc--;
		argv++;
	}
	cmdbuf[0] = '\0';
	str = cmdbuf;
	while (--argc >= 0) {
		i = (long)strlen(*argv);
		if (i+3 >= MAXCMD) {
			fprintf(stderr, "command in arg list too long\n");
			/*
			 * we are too early in processing to call
			 * libcalc_call_me_last() - nothing to cleanup
			 */
			exit(1);
		}
		*str++ = ' ';
		strcpy(str, *argv++);
		str += i;
		str[0] = '\n';
		str[1] = '\0';
	}
	str = cmdbuf;

	/*
	 * unbuffered mode
	 */
	if (u_flag) {
		setbuf(stdin, NULL);
		setbuf(stdout, NULL);
	}

	/*
	 * initialize
	 */
	libcalc_call_me_first();
	stdin_tty = TRUE;	/* assume internactive default */
	conf->tab_ok = TRUE;	/* assume internactive default */
	if (want_defhelp) {
		givehelp(DEFAULTCALCHELP);
		libcalc_call_me_last();
		exit(0);
	}

	/*
	 * if allowed or needed, print version and setup bindings
	 */
	if (*str == '\0') {
		/*
		 * check for pipe mode and/or non-tty stdin
		 */
		if (!p_flag) {
			stdin_tty = isatty(0);	/* assume stdin is on fd 0 */
		}

		/*
		 * empty string arg is no string
		 */
		str = NULL;

		/*
		 * if tty, setup bindings
		 */
		if (stdin_tty) {
			printf("%s (version %s)\n", CALC_TITLE, version());
			printf("[%s]\n\n",
			    "Type \"exit\" to exit, or \"help\" for help.");
			switch (hist_init(calcbindings)) {
			case HIST_NOFILE:
				fprintf(stderr,
				    "Cannot open bindings file \"%s\", %s.\n",
				     calcbindings, "fancy editing disabled");
				break;

			case HIST_NOTTY:
				fprintf(stderr,
					"Cannot set terminal modes, %s.\n",
					"fancy editing disabled");
				break;
			}
		}
	}

	/*
	 * establish error longjump point with initial conditions
	 */
	if (setjmp(jmpbuf) == 0) {

		/*
		 * reset/initialize the computing environment
		 */
		if (post_init) {
			initialize();
		} else {
			/* initialize already done, jmpbuf is ready */
			post_init = TRUE;	
		}

		/*
		 * if arg mode or non-tty mode, just do the work and be gone
		 */
		if (str || !stdin_tty) {
			if (q_flag == FALSE && allow_read) {
				runrcfiles();
				q_flag = TRUE;
			}
			if (str)
				(void) openstring(str);
			else
				(void) openterminal();
			start_done = TRUE;
			getcommands(FALSE);
			libcalc_call_me_last();
			exit(0);
		}
	}
	/* if in arg mode, we should not get here */
	if (str) {
		libcalc_call_me_last();
		exit(1);
	}

	/*
	 * process commands
	 */
	if (!start_done) {
		reinitialize();
	}
	(void) signal(SIGINT, intint);
	start_done = TRUE;
	getcommands(TRUE);

	/*
	 * all done
	 */
	libcalc_call_me_last();
	/* exit(0); */
	return 0;
}


/*
 * Interrupt routine.
 *
 * given:
 *	arg		to keep ANSI C happy
 */
/*ARGSUSED*/
static void
intint(int arg)
{
	(void) signal(SIGINT, intint);
	if (inputwait || (++abortlevel >= ABORT_NOW)) {
		math_error("\nABORT");
		/*NOTREACHED*/
	}
	if (abortlevel >= ABORT_MATH)
		_math_abort_ = TRUE;
	printf("\n[Abort level %d]\n", abortlevel);
}


/*
 * Routine called on any runtime error, to complain about it (with possible
 * arguments), and then longjump back to the top level command scanner.
 */
void
math_error(char *fmt, ...)
{
	va_list ap;
	char buf[MAXERROR+1];

	if (funcname && (*funcname != '*'))
		fprintf(stderr, "\"%s\": ", funcname);
	if (funcline && ((funcname && (*funcname != '*')) || !inputisterminal()))
		fprintf(stderr, "line %ld: ", funcline);
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s\n", buf);
	funcname = NULL;
	if (post_init) {
		longjmp(jmpbuf, 1);
	} else {
		fprintf(stderr, "no jmpbuf jumpback point - ABORTING!!!\n");
		/*
		 * don't call libcalc_call_me_last() -- we might loop
		 * and besides ... this is an unusual internal error case
		 */
		exit(3);
	}
}
