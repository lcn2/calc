/*
 * Copyright (c) 1995 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Arbitrary precision calculator.
 */

#include <signal.h>
#include <pwd.h>
#include <sys/types.h>

#define CALC_C
#include "calc.h"
#include "hist.h"
#include "func.h"
#include "opcodes.h"
#include "conf.h"
#include "token.h"
#include "symbol.h"
#include "have_uid_t.h"

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
int abortlevel;		/* current level of aborts */
BOOL inputwait;		/* TRUE if in a terminal input wait */
jmp_buf jmpbuf;		/* for errors */
int start_done = FALSE;	/* TRUE => start up processing finished */

extern int isatty(int tty);	/* TRUE if fd is a tty */

static int p_flag = FALSE;	/* TRUE => pipe mode */
static int q_flag = FALSE;	/* TRUE => don't execute rc files */
static int u_flag = FALSE;	/* TRUE => unbuffer stdin and stdout */

/*
 * global permissions
 */
int allow_read = TRUE;	/* FALSE => may not open any files for reading */
int allow_write = TRUE;	/* FALSE => may not open any files for writing */
int allow_exec = TRUE;	/* FALSE => may not execute any commands */

char *calcpath;		/* $CALCPATH or default */
char *calcrc;		/* $CALCRC or default */
char *calcbindings;	/* $CALCBINDINGS or default */
char *home;		/* $HOME or default */
static char *pager;	/* $PAGER or default */
char *shell;		/* $SHELL or default */
int stdin_tty = TRUE;	/* TRUE if stdin is a tty */
int post_init = FALSE;	/* TRUE setjmp for math_error is readready */

/*
 * some help topics are symbols, so we alias them to nice filenames
 */
static struct help_alias {
	char *topic;
	char *filename;
} halias[] = {
	{"=", "assign"},
	{"%", "mod"},
	{"//", "quo"},
	{NULL, NULL}
};

NUMBER *epsilon_default;	/* default allowed error for float calcs */

static void intint(int arg);	/* interrupt routine */
static void initenv(void);	/* initialize environment vars */

extern void file_init(void);
extern void zio_init(void);

char cmdbuf[MAXCMD+1];	/* command line expression */

/*
 * Top level calculator routine.
 */
MAIN
main(int argc, char **argv)
{
	static char *str;	/* current option string or expression */
	int want_defhelp = 0;	/* 1=> we only want the default help */
	long i;
	char *p;

	/*
	 * parse args
	 */
	argc--;
	argv++;
	while ((argc > 0) && (**argv == '-')) {
		for (str = &argv[0][1]; *str; str++) switch (*str) {
			case 'h':
				want_defhelp = 1;
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
					exit(1);
				}
				if (p[1] != '\0' || *p < '0' || *p > '7') {
					fprintf(stderr, "unknown -m arg\n");
					exit(1);
				}
				allow_read = (((*p-'0') & 04) > 0);
				allow_write = (((*p-'0') & 02) > 0);
				allow_exec = (((*p-'0') & 01) > 0);
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
				version(stdout);
				exit(0);
			default:
				fprintf(stderr, "Unknown option\n");
				exit(1);
		}
		argc--;
		argv++;
	}
	str = cmdbuf;
	*str = '\0';
	while (--argc >= 0) {
		i = (long)strlen(*argv);
		if (str+1+i+2 >= cmdbuf+MAXCMD) {
			fprintf(stderr, "command in arg list too long\n");
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
	hash_init();
	file_init();
	initenv();
	resetinput();
	if (want_defhelp) {
		givehelp(DEFAULTCALCHELP);
		exit(0);
	}

	/*
	 * if allowed or needed, print version and setup bindings
	 */
	if (*str == '\0') {
		/*
		 * check for pipe mode and/or non-tty stdin
		 */
		if (p_flag) {
			stdin_tty = FALSE;    /* stdin not a tty in pipe mode */
			conf->tab_ok = FALSE; /* config("tab",0) if pipe mode */
		} else {
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
			version(stdout);
			printf("[%s]\n\n",
			    "Type \"exit\" to exit, or \"help\" for help.");
		}
		if (stdin_tty) {
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
	} else {

		/*
		 * process args, not stdin
		 */
		stdin_tty = FALSE;	/* stdin not a tty in arg mode */
		conf->tab_ok = FALSE;	/* config("tab",0) if pipe mode */
	}

	/*
	 * establish error longjump point with initial conditions
	 */
	if (setjmp(jmpbuf) == 0) {

		/*
		 * reset/initialize the computing environment
		 */
		post_init = TRUE;	/* jmpbuf is ready for math_error() */
		inittokens();
		initglobals();
		initfunctions();
		initstack();
		resetinput();
		math_cleardiversions();
		math_setfp(stdout);
		math_setmode(MODE_INITIAL);
		math_setdigits((long)DISPLAY_DEFAULT);
		conf->maxprint = MAXPRINT_DEFAULT;

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
			exit(0);
		}
	}
	start_done = TRUE;

	/*
	 * if in arg mode, we should not get here
	 */
	if (str)
		exit(1);

	/*
	 * process commands (from stdin, not the command line)
	 */
	abortlevel = 0;
	_math_abort_ = FALSE;
	inputwait = FALSE;
	(void) signal(SIGINT, intint);
	math_cleardiversions();
	math_setfp(stdout);
	resetscopes();
	resetinput();
	if (q_flag == FALSE && allow_read) {
		q_flag = TRUE;
		runrcfiles();
	}
	(void) openterminal();
	getcommands(TRUE);

	/*
	 * all done
	 */
	exit(0);
	/*NOTREACHED*/
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
	calcpath = getenv(CALCPATH);
	if (calcpath == NULL)
		calcpath = DEFAULTCALCPATH;

	/* determine the $CALCRC value */
	calcrc = getenv(CALCRC);
	if (calcrc == NULL) {
		calcrc = DEFAULTCALCRC;
	}

	/* determine the $CALCBINDINGS value */
	calcbindings = getenv(CALCBINDINGS);
	if (calcbindings == NULL) {
		calcbindings = DEFAULTCALCBINDINGS;
	}

	/* determine the $HOME value */
	home = getenv(HOME);
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
	pager = getenv(PAGER);
	if (pager == NULL || *pager == '\0') {
		pager = DEFAULTCALCPAGER;
	}

	/* determine the $SHELL value */
	shell = getenv(SHELL);
	if (shell == NULL)
		shell = DEFAULTSHELL;
}


/*
 * givehelp - display a help file
 *
 * given:
 *	type		the type of help to give, NULL => index
 */
void
givehelp(char *type)
{
	struct help_alias *p;	/* help alias being considered */
	char *helpcmd;		/* what to execute to print help */

	/*
	 * check permissions to see if we are allowed to help
	 */
	if (!allow_exec || !allow_read) {
		fprintf(stderr,
		    "sorry, help is only allowed with -m mode 5 or 7\n");
		return;
	}

	/* catch the case where we just print the index */
	if (type == NULL) {
		type = DEFAULTCALCHELP;		/* the help index file */
	}

	/* alias the type of help, if needed */
	for (p=halias; p->topic; ++p) {
		if (strcmp(type, p->topic) == 0) {
			type = p->filename;
			break;
		}
	}

	/* form the help command name */
	helpcmd = (char *)malloc(
		sizeof("if [ ! -d \"")+sizeof(HELPDIR)+1+strlen(type)+
		sizeof("\" ];then ")+
		strlen(pager)+1+1+sizeof(HELPDIR)+1+strlen(type)+1+1+
		sizeof(";else echo no such help;fi"));
	sprintf(helpcmd,
	    "if [ -r \"%s/%s\" ];then %s \"%s/%s\";else echo no such help;fi",
	    HELPDIR, type, pager, HELPDIR, type);

	/* execute the help command */
	system(helpcmd);
	free(helpcmd);
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

/* END CODE */
