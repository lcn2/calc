/*
 * custom - interface for custom software and hardware interfaces
 *
 * Copyright (C) 1999-2006,2018,2021  Landon Curt Noll
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
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:	1997/03/03 04:53:08
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


/* these include files are needed regardless of CUSTOM */
#include "have_const.h"
#include "value.h"
#include "custom.h"

#include <stdio.h>

#if defined(CUSTOM)

#include "calc.h"

#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif

#else /* CUSTOM */

#include "config.h"

#endif /* CUSTOM */


#include "banned.h"	/* include after system header <> includes */


BOOL allow_custom = FALSE;	 /* TRUE => custom builtins allowed */


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
	if (conf->calc_debug & CALCDBG_CUSTOM) {
		fprintf(stderr,
		    "%scustom function %s with %d args, %s vals not executed\n",
		    (conf->tab_ok ? "\t" : ""), name, count,
		    (vals == NULL) ? "NULL" : "non-NULL");
	}
	return error_value(E_NO_CUSTOM);

#endif /* CUSTOM */
}


/*
 * showcustom - display the names and brief descriptions of custom functions
 */
/*ARGSUSED*/
void
showcustom(void)
{
#if defined(CUSTOM)

	CONST struct custom *p; /* current function */

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
	size_t snprintf_len;	/* malloced snprintf buffer length */

	/*
	 * firewall
	 */
	if (name == NULL) {
		name = "help";
	}

	/*
	 * form the custom help name
	 */
	snprintf_len = sizeof("custhelp")+1+strlen(name)+1;
	customname = (char *)malloc(snprintf_len+1);
	if (customname == NULL) {
		math_error("bad malloc of customname");
		/*NOTREACHED*/
	}
	snprintf(customname, snprintf_len, "custhelp/%s", name);
	customname[snprintf_len] = '\0';	/* paranoia */

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
	if (conf->calc_debug & CALCDBG_CUSTOM) {
	    fprintf(stderr, "%scustom help for %s unavailable\n",
		(conf->tab_ok ? "\t" : ""),
		((name == NULL) ? "((NULL))" : name));
	}

#endif /* CUSTOM */
}
