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
 * static definitions and functions
 */
static char *usage = "usage: %s [-c] [-C] [-d] [-e] [-h] [-i] [-m mode]\n"
		     "\t[-n] [-p] [-q] [-u] [-v] [[--] calc_cmd ...]\n";
static void intint(int arg);	/* interrupt routine */


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
	havecommands = (optind < argc);

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

	/*
	 * (re)establish the interrupt handler
	 */
	(void) signal(SIGINT, intint);

	/*
	 * execute calc code based on the run state
	 */
	if (run_state == RUN_PRE_BEGIN) {
		if (!q_flag && allow_read) {
			run_state = RUN_PRE_RCFILES;
			runrcfiles();
		}
		run_state = RUN_POST_RCFILES;
	}
	if (run_state == RUN_PRE_RCFILES) {
		fprintf(stderr, "Execution error in rcfiles\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(FALSE);
			run_state = RUN_POST_RCFILES;
		} else {
			if ((havecommands && !i_flag) || !stdin_tty)
				run_state = RUN_STOP_ON_ERROR;
			else if (havecommands)
				run_state = RUN_POST_CMD_ARGS;
			else
				run_state = RUN_POST_RCFILES;
		}
	}
	if (run_state == RUN_POST_RCFILES) {
		if (havecommands) {
			run_state = RUN_PRE_CMD_ARGS;
			(void) openstring(cmdbuf);
			getcommands(FALSE);
		}
		run_state = RUN_POST_CMD_ARGS;
	}
	if (run_state == RUN_PRE_CMD_ARGS) {
		fprintf(stderr, "Execution error in commands\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(FALSE);
			run_state = RUN_POST_CMD_ARGS;
		} else {
			closeinput();
			if (!stdin_tty || !i_flag)
				run_state = RUN_STOP_ON_ERROR;
			else
				run_state = RUN_POST_CMD_ARGS;
		}
	}
	if (run_state == RUN_POST_CMD_ARGS) {
		if (stdin_tty && ((havecommands && !i_flag) || p_flag))
			run_state = RUN_NOT_TOP_LEVEL;
		else
			openterminal();
	} else if (run_state == RUN_TOP_LEVEL) {
		if (!stdin_tty && (!c_flag || stoponerror) &&
		    stoponerror >= 0) {
			run_state = RUN_STOP_ON_ERROR;
		} else if ((c_flag && !stoponerror) || stoponerror < 0)
			getcommands(FALSE);
		else
			reinitialize();
	}

	if (run_state < RUN_NOT_TOP_LEVEL) {
		run_state = RUN_TOP_LEVEL;
		getcommands(TRUE);
	}

	/*
	 * all done
	 */
	libcalc_call_me_last();
	return (run_state == RUN_STOP_ON_ERROR ||
		run_state == RUN_UNKNOWN) ? 1 : 0;
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
