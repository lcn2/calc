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
 * external definitions and functions
 */
extern int abortlevel;		/* current level of aborts */
extern BOOL inputwait;		/* TRUE if in a terminal input wait */
extern jmp_buf jmpbuf;		/* for errors */
extern int isatty(int tty);	/* TRUE if fd is a tty */

extern int p_flag;		/* TRUE => pipe mode */
extern int q_flag;		/* TRUE => don't execute rc files */
extern int u_flag;		/* TRUE => unbuffer stdin and stdout */
extern int d_flag;		/* TRUE => disable heading, lib_debug == 0 */
extern int stoponerror;		/* >0 => stop, <0 => continue, ==0 => use -c */

extern char *pager;		/* $PAGER or default */
extern int stdin_tty;		/* TRUE if stdin is a tty */
extern int interactive;		/* TRUE if interactive session (no cmd args) */
extern char *program;		/* our name */
extern char cmdbuf[];		/* command line expression */

extern char *version(void);	/* return version string */


/*
 * static definitions and functions
 */
static char *usage = "usage: %s [-C] [-e] [-h] [-i] [-m mode] [-n] [-p]\n"
		     "\t[-q] [-u] [-c] [-d] [[--] calc_cmd ...]\n";
static void intint(int arg);	/* interrupt routine */

static int havecommands;
static int c_flag;		/* To permit continuation after error */
static int i_flag;		/* To go interactive if permitted */

/*
 * Top level calculator routine.
 */
int
main(int argc, char **argv)
{
	int want_defhelp = 0;	/* 1=> we only want the default help */
	int cmdlen;		/* length of the command string */
	extern char *optarg;	/* option argument */
	extern int optind;	/* option index */
	int c;			/* option */
	long i;

	/*
	 * parse args
	 */
	program = argv[0];
	while ((c = getopt(argc, argv, "Cehim:npquvcd")) != -1) {
		switch (c) {
		case 'C':
#if defined(CUSTOM)
			allow_custom = TRUE;
			break;
#else /* CUSTOM */
			/*
			 * we are too early in processing to call
			 * libcalc_call_me_last() - nothing to cleanup
			 */
			fprintf(stderr,
			    "%s: calc was built with custom functions "
			    "disabled, -C usage is disallowed\n", program);
			exit(1);
#endif /* CUSTOM */
		case 'e':
			no_env = TRUE;
			break;
		case 'h':
			want_defhelp = 1;
			break;
		case 'i':
			i_flag = TRUE;
			break;
		case 'm':
			if (optarg[1] == '\0' || *optarg<'0' || *optarg>'7') {
				/*
				 * we are too early in processing to
				 * call libcalc_call_me_last()
				 * nothing to cleanup
				 */
				fprintf(stderr,
					"%s: unknown -m arg\n", program);
				exit(1);
			}
			allow_read = (((*optarg-'0') & 04) > 0);
			allow_write = (((*optarg-'0') & 02) > 0);
			allow_exec = (((*optarg-'0') & 01) > 0);
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
		case 'c':
			c_flag = TRUE;
			break;
		case 'd':
			d_flag = TRUE;
			break;
		case 'v':
			/*
			 * we are too early in processing to call
			 * libcalc_call_me_last() - nothing to cleanup
			 */
			printf("%s (version %s)\n", CALC_TITLE, version());
			exit(0);
		default:
			/*
			 * we are too early in processing to call
			 * libcalc_call_me_last() - nothing to cleanup
			 */
			fprintf(stderr, usage, program);
			exit(1);
		}
	}
	interactive = (optind >= argc);
	havecommands = !interactive;

	/*
	 * look at the length of any trailing command args
	 *
	 * We make room for the trailing '\0\n' as well as an extra guard byte.
	 */
	for (cmdlen=0, i=optind; i < argc; ++i) {
		/* argument + space separator */
		cmdlen += strlen(argv[i]) + 1;
	}
	if (i > MAXCMD) {
		/*
		 * we are too early in processing to call
		 * libcalc_call_me_last() - nothing to cleanup
		 */
		fprintf(stderr,
			"%s: command in arg list is too long\n", program);
		exit(1);
	}

	/*
	 * We will form a command the remaining args separated by spaces.
	 */
	cmdbuf[0] = '\0';
	if (optind < argc) {
		strcpy(cmdbuf, argv[optind]);
		cmdlen = strlen(argv[optind]);
		for (i=optind+1; i < argc; ++i) {
			cmdbuf[cmdlen++] = ' ';
			strcpy(cmdbuf+cmdlen, argv[i]);
			cmdlen += strlen(argv[i]);
		}
		cmdbuf[cmdlen++] = '\n';
		cmdbuf[cmdlen] = '\0';
	}

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
	stdin_tty = isatty(0);		/* assume stdin is on fd 0 */
	if (want_defhelp) {
		givehelp(DEFAULTCALCHELP);
		libcalc_call_me_last();
		exit(0);
	}

	/*
	 * if allowed or needed, print version and setup bindings
	 */
	if (!havecommands && stdin_tty) {
		if (!d_flag) {
			printf("%s (version %s)\n", CALC_TITLE, version());
			printf("[%s]\n\n",
			    "Type \"exit\" to exit, or \"help\" for help.");
		}
		switch (hist_init(calcbindings)) {
			case HIST_NOFILE:
				fprintf(stderr,
				    "%s: Cannot open bindings file \"%s\", "
				    "fancy editing disabled.\n",
				     program, calcbindings);
				break;

			case HIST_NOTTY:
				fprintf(stderr,
					"%s: Cannot set terminal modes, "
					"fancy editing disabled\n", program);
				break;
		}
	}

	/*
	 * establish error longjump point with initial conditions
	 */
	if (setjmp(jmpbuf) == 0) {

		/*
		 * reset/initialize the computing environment
		 */
		if (post_init)
			initialize();
		post_init = TRUE;
	}

	(void) signal(SIGINT, intint);

	if (start_done == 0) {
		if (!q_flag && allow_read) {
			start_done = 1;
			runrcfiles();
		}
		start_done = 2;
	}
	if (start_done == 1) {
		fprintf(stderr, "Execution error in rcfiles\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(FALSE);
			start_done = 2;
		} else {
			if ((havecommands && !i_flag) || !stdin_tty)
				start_done = 7;
			else if (havecommands)
				start_done = 4;
			else
				start_done = 2;
		}
	}
	if (start_done == 2) {
		if (havecommands) {
			start_done = 3;
			(void) openstring(cmdbuf);
			getcommands(FALSE);
		}
		start_done = 4;
	}
	if (start_done == 3) {
		fprintf(stderr, "Execution error in commands\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(FALSE);
			start_done = 4;
		}
		else {
			closeinput();
			if (!stdin_tty || !i_flag)
				start_done = 7;
			else
				start_done = 4;
		}
	}
	if (start_done == 4) {
		if (stdin_tty && ((havecommands && !i_flag) || p_flag))
			start_done = 6;
		else
			openterminal();
	}
	else if (start_done == 5) {
		if (!stdin_tty && (!c_flag || stoponerror) && stoponerror >= 0) {
			start_done = 7;
		}
		else if ((c_flag && !stoponerror) || stoponerror < 0)
			getcommands(FALSE);
		else
			reinitialize();
	}

	if (start_done < 6) {
		start_done = 5;
		getcommands(TRUE);
	}

	/*
	 * all done
	 */
	libcalc_call_me_last();
	return (start_done - 6) ? 1 : 0;
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
	if (funcline && ((funcname && (*funcname != '*')) ||
	    !inputisterminal()))
		fprintf(stderr, "line %ld: ", funcline);
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s\n", buf);
	funcname = NULL;
	if (post_init) {
		longjmp(jmpbuf, 1);
	} else {
		fprintf(stderr, "It is too early provide a command line prompt "
				"so we must simply exit.  Sorry!\n");
		/*
		 * don't call libcalc_call_me_last() -- we might loop
		 * and besides ... this is an unusual internal error case
		 */
		exit(3);
	}
}
