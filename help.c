/*
 * help - display help for calc
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
 * @(#) $Id: help.c,v 29.1 1999/12/14 09:15:40 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/help.c,v $
 *
 * Under source code control:	1997/09/14 10:58:30
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://reality.sgi.com/chongo/
 * Share and enjoy!  :-)	http://reality.sgi.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>

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
	{"copying", "COPYING"},
	{"copying-lgpl", "COPYING-LGPL"},
	{"copying_lgpl", "COPYING-LGPL"},
	{"COPYING_LGPL", "COPYING-LGPL"},
	{"Copyright", "copyright"},
	{"COPYRIGHT", "copyright"},
	{"Copyleft", "copyright"},
	{"COPYLEFT", "copyright"},
	{"stdlib", "resource"},
	{NULL, NULL}
};


/*
 * external values
 */
extern char *pager;		/* $PAGER or default */


/*
 * page_file - output an open file thru a pager
 *
 * The popen() below is fairly safe.  The $PAGER environment variable
 * (supplied by the user) is the command we need to run without args.
 * True, the user could set $PAGER is a bogus prog ... but if -m is
 * 5 or 7 (read and exec allowed), then the calc is given ``permission''
 * to open the help file for reading as well as to exec the pager!
 *
 * given:
 *	stream	open file stream of the file to send to the pager
 */
static void
page_file(FILE *stream)
{
	FILE *cmd;		/* pager command */
	char buf[BUFSIZ+1];	/* I/O buffer */
	char *fgets_ret;	/* fgets() return value */

	/*
	 * flush any pending I/O
	 */
	fflush(stderr);
	fflush(stdout);
	fflush(stdin);

	/*
	 * form a write pipe to a pager
	 */
	cmd = popen(pager, "w");
	if (cmd == NULL) {
		fprintf(stderr, "unable form pipe to pager: %s", pager);
		return;
	}

	/*
	 * read the help file and send non-## lines to the pager
	 * until EOF or error
	 */
	buf[BUFSIZ] = '\0';
	do {

		/*
		 * read the next line that does not begin with ##
		 */
		buf[0] = '\0';
		while ((fgets_ret = fgets(buf, BUFSIZ, stream)) != NULL &&
		       buf[0] == '#' && buf[1] == '#') {
		}

		/*
		 * stop reading when we reach EOF (or error) on help file
		 */
		if (fgets_ret == NULL) {
			fflush(cmd);
			break;
		}

		/*
		 * write the line to pager, if possible
		 */
	} while(fputs(buf, cmd) > 0);

	/*
	 * all done, EOF or error, so just clean up
	 */
	(void) pclose(cmd);
	fflush(stderr);
	fflush(stdout);
	fflush(stdin);
	return;
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
	FILE *stream;		/* help file stream */
	char *helppath;		/* path to the help file */
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

	/*
	 * special case for Copyright and Copyleft
	 */
	if (strcmp(type, "copyright") == 0) {
		fputs(Copyright, stdout);
		fflush(stdout);
		return;
	}

	/*
	 * open the helpfile (looking in HELPDIR first)
	 */
	if (sizeof(CUSTOMHELPDIR) > sizeof(HELPDIR)) {
		helppath = (char *)malloc(sizeof(CUSTOMHELPDIR)+1+strlen(type));
	} else {
		helppath = (char *)malloc(sizeof(HELPDIR)+1+strlen(type));
	}
	if (helppath == NULL) {
		fprintf(stderr, "malloc failure in givehelp()\n");
		return;
	}
	sprintf(helppath, "%s/%s", HELPDIR, type);
	stream = fopen(helppath, "r");
	if (stream != NULL) {

		/*
		 * we have the help file open, now display it
		 */
		page_file(stream);
		(void) fclose(stream);

	/*
	 * open the the helpfile (looking in CUSTOMHELPDIR last)
	 */
	} else {

		sprintf(helppath, "%s/%s", CUSTOMHELPDIR, type);
		stream = fopen(helppath, "r");
		if (stream == NULL) {

			/*
			 * we have the help file open, now display it
			 */
			page_file(stream);
			(void) fclose(stream);

		/*
		 * no such help file
		 */
		} else {
			fprintf(stderr,
				"%s: no such help file, try: help help\n",
				type);
		}
	}

	/*
	 * cleanup
	 */
	free(helppath);
	return;
}
