/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Arbitrary precision calculator.
 */

#include <stdio.h>
#include <ctype.h>

#include "calc.h"
#include "conf.h"

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif


/*
 * some help topics are symbols, so we alias them to nice filenames
 */
static struct help_alias {
	char *topic;
	char *filename;
} halias[] = {
	{"=", "address"},
	{"->", "arrow"},
	{"=", "assign"},
	{"*", "dereference"},
	{".", "oldvalue"},
	{"%", "mod"},
	{"//", "quo"},
	{NULL, NULL}
};


/*
 * external values
 */
extern char *pager;		/* $PAGER or default */


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
	char *c;

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

	/*
	 * sanity check on name
	 */
	/* look for /. or a leading . */
	if (strstr(type, "/.") != NULL || type[0] == '.') {
		fprintf(stderr, "bad help name\n");
		return;
	}
	/* look for chars that could be shell meta chars */
	for (c = type; *c; ++c) {
		switch ((int)*c) {
		case '+':
		case ',':
		case '-':
		case '.':
		case '/':
		case '_':
			break;
		default:
			if (!isascii((int)*c) || !isalnum((int)*c)) {
				fprintf(stderr, "bogus char in help name\n");
				return;
			}
			break;
		}
	}


	/* form the help command name */
	helpcmd = (char *)malloc(
		sizeof("if [ ! -r \"")+sizeof(HELPDIR)+1+strlen(type)+
		sizeof("\" ];then ")+
		strlen(pager)+1+1+sizeof(HELPDIR)+1+strlen(type)+1+1+
		sizeof("elif [ ! -r \"")+sizeof(CUSTOMHELPDIR)+1+strlen(type)+
		sizeof("\" ];then ")+
		strlen(pager)+1+1+sizeof(CUSTOMHELPDIR)+1+strlen(type)+1+1+
		sizeof(";else ")+sizeof(ECHO)+
		sizeof("echo no such help, try: help help;fi")+1);
	sprintf(helpcmd,
	    "if [ -r \"%s/%s\" ];then %s \"%s/%s\";"
	    "elif [ -r \"%s/%s\" ];then %s \"%s/%s\";"
	    "else %s no such help, try: help help;fi",
	    HELPDIR, type, pager, HELPDIR, type,
	    CUSTOMHELPDIR, type, pager, CUSTOMHELPDIR, type, ECHO);
	if (conf->calc_debug & CALCDBG_SYSTEM) {
		printf("%s\n", helpcmd);
	}

	/* execute the help command */
	system(helpcmd);
	free(helpcmd);
}

