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
 * @(#) $Revision: 29.4 $
 * @(#) $Id: calc.c,v 29.4 2000/06/07 14:02:13 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/calc.c,v $
 *
 * Under source code control:	1990/02/15 01:48:11
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
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

#include "have_strdup.h"
#if !defined(HAVE_STRDUP)
# define strdup(x) calc_strdup((CONST char *)(x))
#endif /* HAVE_STRDUP */


/*
 * static definitions and functions
 */
static void intint(int arg);	/* interrupt routine */
static int script_args(int argc,char ***argv,char **shellfile,char **program);


/*
 * Top level calculator routine.
 */
int
main(int argc, char **argv)
{
	int want_defhelp = 0;	/* 1=> we only want the default help */
	int cmdlen;		/* length of the command string */
	char *shellfile = NULL; /* != NULL ==> name of calc shell script */
	extern char *optarg;	/* option argument */
	extern int optind;	/* option index */
	int c;			/* option */
	char *p;
	long i;

	/*
	 * parse args
	 */
	program = argv[0];
	/* catch the case of a leading -S option */
	if (argc > 2 && strncmp(argv[1], "-S", 2) == 0) {
		/* convert the calc shell options into command line options */
		argc = script_args(argc, &argv, &shellfile, &program);
		/* -S implies -s */
		s_flag = TRUE;
	}
	/* process command line options */
	while ((c = getopt(argc, argv, "Cehim:npquvcdD:s")) != -1) {
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
			if (optarg[1] != '\0' || *optarg<'0' || *optarg>'7') {
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
		case 'D':
			/*
			 * parse the -D optarg
			 *
			 * Could be calc_debug
			 *	 or calc_debug:resource_debug
			 *	 or calc_debug:resource_debug:user_debug
			 */
			calc_debug = optarg;
			p = strchr(optarg, ':');
			if (p != NULL) {
				*p = '\0';
				resource_debug = p+1;
				p = strchr(resource_debug, ':');
				if (p != NULL) {
					*p = '\0';
					user_debug = p+1;
				}
			}
			break;
		case 's':
			s_flag = TRUE;
			break;
		case 'S':
			/*FALLTHRU*/
		default:
			/*
			 * we are too early in processing to call
			 * libcalc_call_me_last() - nothing to cleanup
			 */
			fprintf(stderr,
			  "usage: %s [-c] [-C] [-d] [-e] [-h] [-i] [-m mode]\n"
			  "\t[-D calc_debug[:resource_debug[:user_debug]]]\n"
			  "\t[-n] [-p] [-q] [-s] [-u] [-v] "
			  "[[--] calc_cmd ...]\n",
			  program);
			exit(1);
		}
	}

	/*
	 * If -S was given via a calc shell script, imply -p and -d
	 * unless -i was also given.
	 */
	if (shellfile != NULL && !i_flag) {
		p_flag = TRUE;
		d_flag = TRUE;
	}

	/*
	 * look at the length of any trailing command args
	 *
	 * We make room for the trailing '\0\n' as well as an extra guard byte.
	 *
	 * However, if -S is in effect, we will pretend that we have no
	 * command args and allow the argv() builtin access to the strings.
	 */
	cmdbuf[0] = '\0';
	if (s_flag) {
		havecommands = FALSE;
		argc_value = argc - optind;
		argv_value = argv + optind;
	} else {
		havecommands = (optind < argc);
		for (cmdlen=0, i=optind; i < argc; ++i) {
			/* argument + space separator */
			cmdlen += strlen(argv[i]) + 1;
		}
		if (cmdlen > MAXCMD) {
			/*
			 * we are too early in processing to call
			 * libcalc_call_me_last() - nothing to cleanup
			 */
			fprintf(stderr,
				"%s: command in arg list is too long\n",
				program);
			exit(1);
		}

		/*
		 * Form a command the remaining args separated by spaces.
		 */
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
		argc_value = 0;
		argv_value = argv+argc;
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
			if (conf->calc_debug & CALCDBG_RUNSTATE)
				printf("main: run_state from %s to %s\n",
				    run_state_name(run_state),
				    run_state_name(RUN_RCFILES));
			run_state = RUN_RCFILES;
			runrcfiles();
		}
		if (conf->calc_debug & CALCDBG_RUNSTATE)
			printf("main: run_state from %s to %s\n",
			    run_state_name(run_state),
			    run_state_name(RUN_PRE_CMD_ARGS));
		run_state = RUN_PRE_CMD_ARGS;
	}

	while (run_state == RUN_RCFILES) {
		fprintf(stderr, "Error in rcfiles\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(FALSE);
			if (inputlevel() == 0) {
				closeinput();
				runrcfiles();
				if (conf->calc_debug & CALCDBG_RUNSTATE)
				    printf("main: run_state from %s to %s\n",
					run_state_name(run_state),
					run_state_name(RUN_PRE_CMD_ARGS));
				run_state = RUN_PRE_CMD_ARGS;
			} else {
				closeinput();
			}
		} else {
			if ((havecommands && !i_flag) || !stdin_tty) {
				if (conf->calc_debug & CALCDBG_RUNSTATE)
				    printf("main: run_state from %s to %s\n",
					run_state_name(run_state),
					run_state_name(RUN_EXIT_WITH_ERROR));
				run_state = RUN_EXIT_WITH_ERROR;
			} else {
				if (conf->calc_debug & CALCDBG_RUNSTATE)
				    printf("main: run_state from %s to %s\n",
					run_state_name(run_state),
					run_state_name(RUN_PRE_CMD_ARGS));
				run_state = RUN_PRE_CMD_ARGS;
			}
		}
	}

	if (run_state == RUN_PRE_CMD_ARGS) {
		if (havecommands) {
			if (conf->calc_debug & CALCDBG_RUNSTATE)
				printf("main: run_state from %s to %s\n",
				    run_state_name(run_state),
				    run_state_name(RUN_CMD_ARGS));
			run_state = RUN_CMD_ARGS;
			(void) openstring(cmdbuf, (long) strlen(cmdbuf));
			getcommands(FALSE);
			closeinput();
		} else if (shellfile != NULL) {
			/* XXX - shellfile stuff needs it own run_state name */
			if (conf->calc_debug & CALCDBG_RUNSTATE)
				printf("main: run_state from %s to %s\n",
				    run_state_name(run_state),
				    run_state_name(RUN_CMD_ARGS));
			run_state = RUN_CMD_ARGS;
			getshellfile(shellfile);
		}
		if (conf->calc_debug & CALCDBG_RUNSTATE)
			printf("main: run_state from %s to %s\n",
			    run_state_name(run_state),
			    run_state_name(RUN_PRE_TOP_LEVEL));
		run_state = RUN_PRE_TOP_LEVEL;
	}

	while (run_state == RUN_CMD_ARGS) {
		fprintf(stderr, "Error in commands\n");
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(FALSE);
			if (inputlevel() == 0)
				if (conf->calc_debug & CALCDBG_RUNSTATE)
				    printf("main: run_state from %s to %s\n",
					run_state_name(run_state),
					run_state_name(RUN_PRE_TOP_LEVEL));
				run_state = RUN_PRE_TOP_LEVEL;
			closeinput();
		} else {
			closeinput();
			if (!stdin_tty || !i_flag || p_flag) {
				if (conf->calc_debug & CALCDBG_RUNSTATE)
				    printf("main: run_state from %s to %s\n",
					run_state_name(run_state),
					run_state_name(RUN_EXIT_WITH_ERROR));
				run_state = RUN_EXIT_WITH_ERROR;
			} else {
				if (conf->calc_debug & CALCDBG_RUNSTATE)
				    printf("main: run_state from %s to %s\n",
					run_state_name(run_state),
					run_state_name(RUN_PRE_TOP_LEVEL));
				run_state = RUN_PRE_TOP_LEVEL;
			}
		}
	}

	if (run_state == RUN_PRE_TOP_LEVEL) {
		if (stdin_tty &&
		    (((havecommands || shellfile) && !i_flag) || p_flag)) {
			if (conf->calc_debug & CALCDBG_RUNSTATE)
				printf("main: run_state from %s to %s\n",
				    run_state_name(run_state),
				    run_state_name(RUN_EXIT));
			run_state = RUN_EXIT;
		} else {
			if (stdin_tty) {
				reinitialize();
			} else {
				resetinput();
				openterminal();
			}
			if (conf->calc_debug & CALCDBG_RUNSTATE)
				printf("main: run_state from %s to %s\n",
				    run_state_name(run_state),
				    run_state_name(RUN_TOP_LEVEL));
			run_state = RUN_TOP_LEVEL;
			getcommands(TRUE);
		}
	}

	while (run_state == RUN_TOP_LEVEL) {
		if ((c_flag && !stoponerror) || stoponerror < 0) {
			getcommands(TRUE);
			if (!inputisterminal())
				closeinput();
		} else {
			if (stdin_tty) {
				reinitialize();
				getcommands(TRUE);
			} else {
				if (conf->calc_debug & CALCDBG_RUNSTATE)
				    printf("main: run_state from %s to %s\n",
					run_state_name(run_state),
					run_state_name(RUN_EXIT_WITH_ERROR));
				run_state = RUN_EXIT_WITH_ERROR;
			}
		}
	}

	/*
	 * all done
	 */
	libcalc_call_me_last();
	return (run_state == RUN_EXIT_WITH_ERROR ||
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


/*
 * script_args - concert shell script options into command line form
 *
 * When a calc shell script is executed, the args are presented to calc
 * in a different form.	 Consider the a calc shell script called /tmp/calcit:
 *
 *	#!/usr/local/bin/calc -S -q -p
 *	a=eval(prompt("Enter a: "));
 *	b=eval(prompt("Enter b: "));
 *	print a+b;
 *
 * When it is executed as:
 *
 *	/tmp/calcit -D 31
 *
 * then calc will receive the following as args to main():
 *
 *	argc:	  5
 *	argv[0]:  "/usr/local/bin/calc"
 *	argv[1]:  "-S -q -p -e"
 *	argv[2]:  "/tmp/calcit"
 *	argv[3]:  "-D"
 *	argv[4]:  "31"
 *	argv[5]:  NULL
 *
 * NOTE: The user MUST put -S as the first characters on the calc shell
 *	 script #! line, right after the calc binary path.
 *
 * NOTE: The arg supplied on the #! calc shell script line are returned
 *	 as a single string.  All tabs are converted into spaces.
 *
 * We must remember the calc script filename, break apart the #! args
 * and remove the -S argument.	In the above case we would return:
 *
 *	argc:	  6
 *	argv[0]:  "/usr/local/bin/calc"
 *	argv[1]:  "-q"
 *	argv[2]:  "-p"
 *	argv[3]:  "-e"
 *	argv[4]:  "-D"
 *	argv[5]:  "31"
 *	argv[6]:  NULL
 *
 *	shellfile: "/tmp/calcit"
 *	s_flag:	   TRUE
 */
static int
script_args(int argc, char ***argv_p, char **shellfile_p, char **program_p)
{
	char **argv;		/* new argv to return */
	char *shellfile;	/* shell file pathname to return */
	int delta;		/* the change needed in argc */
	int i;
	int j;
	char *p;
	char *q;

	/*
	 * must have at least 3 args and the 2nd must start with -S
	 */
	argv = *argv_p;
	if (argc < 3 || strncmp(argv[1], "-S", 2) != 0) {
		/*
		 * we are too early in processing to call
		 * libcalc_call_me_last() - nothing to cleanup
		 */
		fprintf(stderr,
		    "%s: FATAL: bad args passed to script_args\n", program);
		exit(1);
	}
	shellfile = argv[2];

	/*
	 * count the additional args beyond the -S
	 */
	if (argv[1][2] == ' ') {

		/*
		 * process args beyond -S on the #!/usr/local/bin line
		 */
		p = argv[1]+3 + strspn(argv[1]+3," ");
		q = p;
		if (q == '\0') {

			/* only trailing spaces after -S, ignore them */
			for (i = 3; i <= argc; ++i) {
				argv[i-2] = argv[i];
			}
			argc -= 2;

		} else {

			/* count the space separated strings that follow -S */
			for (delta = -1; p != NULL && *p;
			     p = strchr(p+1,' '), ++delta) {
				/* skip multiple spaces in a row */
				p += strspn(p, " ");
			}

			/* allocate the new set of argv pointers */
			argv = (char **)malloc(sizeof(char *) * argc+delta);
			if (argv == NULL) {
				/*
				 * we are too early in processing to call
				 * libcalc_call_me_last() - nothing to cleanup
				 */
				fprintf(stderr,
				    "%s: failed to malloc %d pointers\n",
				    shellfile, argc+delta);
				exit(1);
			}

			/* we have the same 0th arg */
			argv[0] = (*argv_p)[0];

			/* args may be read-only, so duplicate 1st arg */
			p = strdup(q);
			if (p == NULL) {
				/*
				 * we are too early in processing to call
				 * libcalc_call_me_last() - nothing to cleanup
				 */
				fprintf(stderr,
				    "%s: failed to duplicate 1st arg\n",
				    shellfile);
				exit(1);
			}

			/* tokenize the 1st arg */
			for (p=strtok(q," "), i=1; p != NULL;
			     p=strtok(NULL," "), ++i) {
				argv[i] = p;
			}

			/* save the 3rd and later args */
			for (j=3; (*argv_p)[j] != NULL; ++j, ++i) {
				argv[i] = (*argv_p)[j];
			}
			argv[i] = NULL;

			/* set argc */
			argc = i;
		}

	/*
	 * catch the case of #!/usr/local/bin -Stuff_not_extra_args
	 */
	} else if (argv[1][2] != '\0') {

		/*
		 * we are too early in processing to call
		 * libcalc_call_me_last() - nothing to cleanup
		 */
		fprintf(stderr,
		    "%s: malformed #! line, -S must be "
		    "followed by space or newline\n",
		    shellfile);
		exit(1);

	/*
	 * Only -S was given in the #!/usr/local/bin line, so we just
	 * toss the 2nd and 3rd args
	 */
	} else {
		for (i = 3; i <= argc; ++i) {
			argv[i-2] = argv[i];
		}
		argc -= 2;
	}

	/*
	 * return and set the argc, argv, shellfile_p and s_flag values
	 */
	*argv_p = argv;
	*shellfile_p = shellfile;
	*program_p = shellfile;
	s_flag = TRUE;
	return argc;
}
