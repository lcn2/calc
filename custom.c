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
 * Comments, suggestions, bug fixes and questions about these routines
 * are welcome.  Send EMail to the address given below.
 *
 * Happy bit twiddling,
 *
 *	Landon Curt Noll
 *	http://reality.sgi.com/chongo/
 *
 * chongo <was here> /\../\
 */

/* these include files are needed regardless of CUSTOM */
#include "have_const.h"
#include "value.h"
#include "custom.h"


#if defined(CUSTOM)

#include <stdio.h>

#include "calc.h"

#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif

#else /* CUSTOM */

#include "config.h"

#endif /* CUSTOM */

int allow_custom = FALSE;	 /* TRUE => custom builtins allowed */


/*
 * custom - custom callout function
 */
/*ARGSUSED*/
VALUE
custom(char *name, int count, VALUE **vals)
{
#if defined(CUSTOM)

	CONST struct custom *p;		/* current function */

	/*
	 * search the custom interface table for a function
	 */
	for (p = cust; p->name != NULL; ++p) {

		/* look for the first match */
		if (strcmp(name, p->name) == 0) {

			/* arg count check */
			if (count < p->minargs) {
				math_error("Too few arguments for custom "
				    "function \"%s\"", p->name);
				/*NOTREACHED*/
			}
			if (count > p->maxargs) {
				math_error("Too many arguments for custom "
				    "function \"%s\"", p->name);
				/*NOTREACHED*/
			}

			/* call the custom function */
			return p->func(name, count, vals);
		}
	}

	/*
	 * no such custom function
	 */
	return error_value(E_UNK_CUSTOM);

#else /* CUSTOM */

	fprintf(stderr,
	    "%sCalc was built with custom functions disabled\n",
	    (conf->tab_ok ? "\t" : ""));
	return error_value(E_NO_CUSTOM);

#endif /* CUSTOM */
}


/*
 * showcustom - display the names and brief descriptins of custom functions
 */
/*ARGSUSED*/
void
showcustom(void)
{
#if defined(CUSTOM)

	CONST struct custom *p;	/* current function */

	/*
	 * disable custom functions unless -C was given
	 */
	if (!allow_custom) {
		fprintf(stderr,
		    "%sCalc must be run with a -C argument to "
		    "show custom functions\n",
		    (conf->tab_ok ? "\t" : ""));
		return;
	}

	/*
	 * print header
	 */
	printf("\nName\tArgs\tDescription\n\n");
	for (p = cust; p->name != NULL; ++p) {
		printf("%-9s ", p->name);
		if (p->maxargs == MAX_CUSTOM_ARGS)
			printf("%d+    ", p->minargs);
		else if (p->minargs == p->maxargs)
			printf("%-6d", p->minargs);
		else
			printf("%d-%-4d", p->minargs, p->maxargs);
		printf("%s\n", p->desc);
	}
	printf("\n");

#else /* CUSTOM */

	fprintf(stderr,
	    "%sCalc was built with custom functions disabled\n",
	    (conf->tab_ok ? "\t" : ""));

#endif /* CUSTOM */
}


/*
 * customhelp - standard help interface to a custom function
 *
 * This function assumes that a help file with the same name as
 * the custom function has been installed by the custom/Makefile
 * (as listed in the CUSTOM_HELP makefile variable) under the
 * CUSTOMHELPDIR == HELPDIR/custhelp directory.
 *
 * The help command first does a search in HELPDIR and later
 * in CUSTOMHELPDIR.  If a custom help file has the same name
 * as a file under HELPDIR then help will display the HELPDIR
 * file and NOT the custom file.  This function will ignore
 * and HELPDIR file and work directly with the custom help file.
 *
 * given:
 *	name	name of the custom help file to directly access
 */
/*ARGSUSED*/
void
customhelp(char *name)
{
#if defined(CUSTOM)

	char *customname;	/* a string of the form: custom/name */

	/*
	 * firewall
	 */
	if (name == NULL) {
		name = "help";
	}

	/*
	 * form the custom help name
	 */
	customname = (char *)malloc(sizeof("custhelp")+strlen(name)+1);
	if (customname == NULL) {
		math_error("bad malloc of customname");
		/*NOTREACHED*/
	}
	sprintf(customname, "custhelp/%s", name);

	/*
	 * give help directly to the custom file
	 */
	givehelp(customname);

	/*
	 * all done
	 */
	free(customname);

#else /* CUSTOM */

	fprintf(stderr,
	    "%sCalc was built with custom functions disabled\n",
	    (conf->tab_ok ? "\t" : ""));

#endif /* CUSTOM */
}
