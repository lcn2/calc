/*
 * calc - arbitrary precision calculator
 *
 * Copyright (C) 1999  David I. Bell, Landon Curt Noll and Ernest Bowen
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.10 $
 * @(#) $Id: calc.c,v 29.10 2004/02/23 09:21:35 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/calc.c,v $
 *
 * Under source code control:	1990/02/15 01:48:11
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <signal.h>

#if !defined (_WIN32)
# include <pwd.h>
#endif

#include <sys/types.h>
#include <ctype.h>

#if defined(_WIN32)
# include <io.h>
/*
 * getopt.h file is from the Cygwin GNU library
 *
 * See:
 *	http://sources.redhat.com/cygwin/
 */
# include "../getopt/getopt.h"
# define strdup _strdup
# define isatty _isatty
#endif /* Windoz */

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
#include "zmath.h"

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "have_stdlib.h"
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#include "have_strdup.h"
#if !defined(HAVE_STRDUP)
# define strdup(x) calc_strdup((CONST char *)(x))
#endif /* HAVE_STRDUP */

#include "have_unused.h"


/*
 * static definitions and functions
 */
static void intint(int arg);	/* interrupt routine */
static int nextcp(char **cpp, int *ip, int argc, char **argv, BOOL haveendstr);
static void set_run_state(run state);

/*
 * Top level calculator routine.
 */
int
main(int argc, char **argv)
{
	int want_defhelp = 0;	/* 1=> we only want the default help */
	int cmdlen;		/* length of the command string */
	int newcmdlen;
	int c;			/* option */
	int index;
	int maxindex;
	char *cp;
	char *endcp;
	char *bp;
	BOOL done = FALSE;
	BOOL havearg;
	BOOL haveendstr;
	int len;

	/*
	 * parse args
	 */
	program = argv[0];

	cmdbuf[0] = '\0';
	cmdlen = 0;

	/* process command line options */

	index = 1;
	cp = endcp = NULL;
	maxindex = argc;
	havecommands = FALSE;
	while (index < maxindex && !done) {
		cp = argv[index];
		if (*cp == '\0') {
			index++;
			continue;
		}
		for (;;) {
			havearg = FALSE;
			if (*cp != '-') {
				done = TRUE;
				break;
			}
			++cp;
			if (*cp == '-') {
				cp++;
				while (*cp == ' ')
					++cp;
				done = TRUE;
				break;
			}

			for (;;) {
				c = *cp;
				if (c == '\0' || c == ' ')
					break;
				switch (c) {
				case 'C':
#if defined(CUSTOM)
					allow_custom = TRUE;
					break;
#else /* CUSTOM */
					/*
					 * we are too early in processing to
					 * call libcalc_call_me_last() -
					 * nothing to cleanup
					 */
					fprintf(stderr,
					    "%s: calc was built with custom"
					    " functions disabled, -C usage is"
					    " disallowed\n", program);
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
					cp++;
					while (*cp == ' ' || *cp == '\t')
						cp++;
					if (*cp == '\0') {
						index++;
						if (index >= argc) {
							fprintf(stderr,
								"-m expects"
								" argument");
							exit (1);
						}
						cp = argv[index];
					}

					if (*cp < '0' || *cp > '7') {
						/*
						 * we are too early in
						 * processing to call
						 * libcalc_call_me_last()
						 * nothing to cleanup
						 */
						fprintf(stderr,
							"%s: unknown -m arg\n",
							program);
						exit(1);
					}
					allow_read = (((*cp-'0') & 04) > 0);
					allow_write = (((*cp-'0') & 02) > 0);
					allow_exec = (((*cp-'0') & 01) > 0);
					cp++;
					if (*cp != ' ' && *cp != '\0') {
						fprintf(stderr, "??? m-arg");
						exit(1);
					}
					havearg = TRUE;
					break;
				case 'n':
					/*
					 * -n is deprecated and may be reused
					 * for another purpose in the future
					 */
					break;
				case 'O':
					use_old_std = TRUE;
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
					 * we are too early in processing to
					 * call libcalc_call_me_last() -
					 * nothing to cleanup
					 */
					printf("%s (version %s)\n",
					       CALC_TITLE, version());
					exit(0);
				case 'D':
					/*
					 * parse the -D arg
					 *
					 * Could be:
					 *
					 * calc_debug
					 * calc_debug:resource_debug
					 * calc_debug:resource_debug:user_debug
					 */
					if (nextcp(&cp, &index, argc, argv,
						   FALSE)) {
					    fprintf(stderr,
					    	    "-D expects argument\n");
					    exit (1);
					}
					havearg = TRUE;
					if (*cp != ':') {
						if (*cp < '0' || *cp > '9') {
						    fprintf(stderr,
						    	    "-D expects"
							    " integer\n");
						    exit (1);
						}
						calc_debug = cp;
						(void) strtol(cp, &endcp, 10);
						cp = endcp;
						if (*cp != '\0' &&
						    *cp != ' ' && *cp != ':') {
						    fprintf(stderr,
						    	    "Bad syntax im -D"
							    " arg\n");
						    exit (1);
						}
						if (*cp != ':') {
						    if (nextcp(&cp, &index,
						    	       argc, argv,
							       FALSE)
								|| *cp != ':')
							break;
						}
					}
					if (nextcp(&cp, &index, argc, argv,
						   FALSE)) {
						fprintf(stderr,
							"-D : expects"
							" argument\n");
						exit (1);
					}
					if (*cp != ':') {
						if (*cp < '0' || *cp > '9') {
						    fprintf(stderr,
						    	    "-D : expects"
							    " integer\n");
						    exit (1);
						}
						resource_debug = cp;
						(void) strtol(cp, &endcp, 10);
						cp = endcp;
						if (*cp != '\0' &&
						    *cp != ' ' && *cp != ':') {
						    fprintf(stderr,
						    	    "Bad syntax im -D"
							    " : arg\n");
						    exit (1);
						}
						if (*cp != ':') {
							if (nextcp(&cp, &index,
						    	           argc, argv,
							           FALSE)
							    || *cp != ':') {
								break;
							}
						}
					}
					if (nextcp(&cp, &index, argc, argv,
						   FALSE)) {
					    fprintf(stderr, "-D : : expects"
					    	    " argument\n");
					    exit (1);
					}
					if (*cp < '0' || *cp > '9') {
					    fprintf(stderr, "-D :: expects"
					    	    " integer\n");
					    exit (1);
					}
					user_debug = cp;
					(void) strtol(cp, &endcp, 10);
					cp = endcp;
					if (*cp != '\0' && *cp != ' ') {
						fprintf(stderr, "Bad syntax in"
							" -D : : arg\n");
						exit (1);
					}
					break;
				case 'f':
					haveendstr = (cp[1] == '\0');
					if (nextcp(&cp, &index, argc, argv,
						   haveendstr)) {
					    fprintf(stderr, "-f expects"
						    " filename\n");
					    exit (1);
					}
					if (*cp == ';') {
						fprintf(stderr,
							"-f expects"
							" filename\n");
						exit (1);
					}
					havearg = TRUE;
					if (cmdlen > 0)
						cmdbuf[cmdlen++] = ' ';
					strcpy(cmdbuf + cmdlen, "read ");
					cmdlen += 5;
					if (strncmp(cp, "-once", 5) == 0 &&
					    (cp[5] == '\0' || cp[5] == ' ')) {
						cp += 5;
						haveendstr = (*cp == '\0');
						strcpy(cmdbuf+cmdlen, "-once ");
						cmdlen += 6;
						if (nextcp(&cp, &index, argc,
							   argv, haveendstr)) {
						    fprintf(stderr, "-f -once"
						    	    " expects"
							    " filename\n");
						    exit (1);
						}
					}
					bp = cmdbuf + cmdlen;
					if (haveendstr) {
						len = strlen(cp);
						if (len == 0) {
							fprintf(stderr,
								"Null"
								" filename!");
							exit (1);
						}
						if (cmdlen + len + 2 > MAXCMD) {
							fprintf(stderr,
								"Commands too"
								" long");
							exit (1);
						}
						/* XXX What if *cp = '\''? */
						*bp++ = '\'';
						strcpy(bp, cp);
						bp += len;
						*bp++ = '\'';
						cp += len;
						cmdlen += len + 2;
					} else {
						do {
							if (cmdlen > MAXCMD) {
								fprintf(stderr,
								   "Commands"
								   " too long");
								exit (1);
							}
							*bp++ =  *cp++;
							cmdlen++;
						} while (*cp != '\0' &&
							 *cp != ';' &&
							 *cp != ' ');
					}
					if (*cp == ';')
						cp++;
					*bp++ = ';';
					cmdlen++;
					break;

				case 's':
					s_flag = TRUE;
					maxindex = index + 1;
					break;
				default:
					/*
					 * we are too early in processing to
					 * call libcalc_call_me_last() -
					 * nothing to cleanup
					 */
					fprintf(stderr, "Illegal option -%c\n",
							c);
					fprintf(stderr,
		"usage: %s [-a] [-c] [-C] [-d] [-e] [-h] [-i] [-m mode]\n"
		"\t[-D calc_debug[:resource_debug[:user_debug]]]\n"
		"\t[-O] [-p] [-q] [-u] [-v] "
		"[--] [calc_cmd ...]\n",
		program);
					exit(1);
			}
			if (havearg)
				break;
			cp++;
			}
			while (*cp == ' ')
				cp++;
			if (*cp == '\0') {
				index++;
				break;
			}
		}
	}

	while (index < maxindex) {
		if (cmdlen > 0)
			cmdbuf[cmdlen++] = ' ';
		newcmdlen = cmdlen + strlen(cp);
		if (newcmdlen > MAXCMD) {
			fprintf(stderr,
				"%s: commands too long\n",
				program);
			exit(1);
		}
		strcpy(cmdbuf + cmdlen, cp);
		cmdlen = newcmdlen;
		index++;
		if (index < maxindex)
			cp = argv[index];
	}

	havecommands = (cmdlen > 0);

	if (havecommands) {
		cmdbuf[cmdlen++] = '\n';
		cmdbuf[cmdlen] = '\0';
	}

	argc_value = argc - maxindex;
	argv_value = argv + maxindex;

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
	if (conf->calc_debug & CALCDBG_TTY)
		printf("main: stdin_tty is %d\n", stdin_tty);
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
			printf("Calc is open software. For license details "
			       "type:  help copyright\n");
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
	if (run_state == RUN_BEGIN) {
		if (!q_flag && allow_read) {
			set_run_state(RUN_RCFILES);
			runrcfiles();
		}
		set_run_state(RUN_PRE_CMD_ARGS);
	}

	while (run_state == RUN_RCFILES) {
		fprintf(stderr, "Error in rcfiles\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(FALSE);
			if (inputlevel() == 0) {
				closeinput();
				runrcfiles();
				set_run_state(RUN_PRE_CMD_ARGS);
			} else {
				closeinput();
			}
		} else {
			if ((havecommands && !i_flag) || !stdin_tty) {
				set_run_state(RUN_EXIT_WITH_ERROR);
			} else {
				set_run_state(RUN_PRE_CMD_ARGS);
			}
		}
	}

	if (run_state == RUN_PRE_CMD_ARGS) {
		if (havecommands) {
			set_run_state(RUN_CMD_ARGS);
			(void) openstring(cmdbuf, (long) strlen(cmdbuf));
			getcommands(FALSE);
			closeinput();
		}
		set_run_state(RUN_PRE_TOP_LEVEL);
	}

	while (run_state == RUN_CMD_ARGS) {
		fprintf(stderr, "Error in commands\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(FALSE);
			if (inputlevel() == 0)
				set_run_state(RUN_PRE_TOP_LEVEL);
			closeinput();
		} else {
			closeinput();
			if (!stdin_tty || !i_flag || p_flag) {
				set_run_state(RUN_EXIT_WITH_ERROR);
			} else {
				set_run_state(RUN_PRE_TOP_LEVEL);
			}
		}
	}

	if (run_state == RUN_PRE_TOP_LEVEL) {
		if (stdin_tty &&
		    (((havecommands) && !i_flag) || p_flag)) {
			set_run_state(RUN_EXIT);
		} else {
			if (stdin_tty) {
				reinitialize();
			} else {
				resetinput();
				openterminal();
			}
			set_run_state(RUN_TOP_LEVEL);
			getcommands(TRUE);
		}
		if (p_flag || (!i_flag && havecommands))
			set_run_state(RUN_EXIT);
	}

	while (run_state == RUN_TOP_LEVEL) {
		if (conf->calc_debug & CALCDBG_RUNSTATE)
			printf("main: run_state = TOP_LEVEL\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(TRUE);
			if (!inputisterminal()) {
				closeinput();
				continue;
			}
			if (!p_flag && i_flag && !stdin_tty) {
				closeinput();
				if(!freopen("/dev/tty", "r", stdin)) {
#if defined (_WIN32)
					fprintf(stderr,
						"/dev/tty does not exist on "
						"this operating system.  "
						"Change operating systems\n"
						"or don't use this calc mode "
						"in the future, sorry!\n");
#else /* Windoz free systems */
					fprintf(stderr,
						"Unable to associate stdin"
						" with /dev/tty");
#endif /* Windoz free systems */
					set_run_state(RUN_EXIT_WITH_ERROR);
					break;
				}

				stdin_tty = TRUE;
				if (conf->calc_debug & CALCDBG_TTY)
					printf("main: stdin_tty is %d\n",
					       stdin_tty);
				reinitialize();
			}
		} else {
			if (stdin_tty) {
				reinitialize();
				getcommands(TRUE);
			} else if (inputisterminal() &&
					!p_flag && (!havecommands||i_flag)) {
				closeinput();
				if(!freopen("/dev/tty", "r", stdin)) {
#if defined (_WIN32)
					fprintf(stderr,
						"/dev/tty does not exist on "
						"this operating system.  "
						"Change operating systems\n"
						"or don't use this calc mode "
						"in the future, sorry!\n");
#else /* Windoz free systems */
					fprintf(stderr,
						"Unable to associate stdin"
						" with /dev/tty");
#endif /* Windoz free systems */
					set_run_state(RUN_EXIT_WITH_ERROR);
					break;
				}
				stdin_tty = TRUE;
				if (conf->calc_debug & CALCDBG_TTY)
					printf("main: stdin_tty is %d\n",
					       stdin_tty);
				reinitialize();
			} else {
				set_run_state(RUN_EXIT_WITH_ERROR);
			}
		}
	}
	if (conf->calc_debug & CALCDBG_RUNSTATE)
		printf("main: run_state = %s\n", run_state_name(run_state));

	/*
	 * all done
	 */
	libcalc_call_me_last();
	return (run_state == RUN_EXIT_WITH_ERROR ||
		run_state == RUN_ZERO) ? 1 : 0;
}


/*
 * Interrupt routine.
 *
 * given:
 *	arg		to keep ANSI C happy
 */
/*ARGSUSED*/
static void
intint(int UNUSED arg)
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

static int
nextcp(char **cpp, int *ip, int argc, char **argv, BOOL haveendstr)
{
	char *cp;
	int index;

	cp = *cpp;
	index = *ip;


	if (haveendstr) {
		index++;
		*ip = index;
		if (index >= argc)
			return 1;
		*cpp = argv[index];
		return 0;
	}

	if (*cp != '\0')
		cp++;
	for (;;) {
		if (*cp == '\0') {
			index++;
			*ip = index;
			if (index >= argc)
				return 1;
			cp = argv[index];
		}
		while (*cp == ' ')
			cp++;
		if (*cp != '\0')
			break;
	}
	*cpp = cp;
	return 0;
}


static void
set_run_state(run state)
{
	if (conf->calc_debug & CALCDBG_RUNSTATE)
		printf("main: run_state from %s to %s\n",
			    run_state_name(run_state),
			    run_state_name(state));
	run_state = state;
}

