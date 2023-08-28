/*
 * version - determine the version of calc
 *
 * Copyright (C) 1999-2023  David I. Bell and Landon Curt Noll
 *
 * See "version.h" for the actual calc version constants.
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
 * Under source code control:	1990/05/22 11:00:58
 * File existed as early as:	1990
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <errno.h>
#include "have_string.h"
#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include "version.h"
#if defined(CALC_VER)
# include <stdlib.h>
# include <unistd.h>
# define STATIC static
static char *program;
#else
# include "calc.h"
#endif
#include "str.h"
#include "strl.h"

#include "have_unused.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * calc version constants
 */
int calc_major_ver = MAJOR_VER;
int calc_minor_ver = MINOR_VER;
int calc_major_patch = MAJOR_PATCH;
int calc_minor_patch = MINOR_PATCH;


/*
 * stored version
 */
STATIC char *stored_version = NULL;	/* version formed if != NULL */


/*
 * stored license info - has a side effect of copyrighting the binary
 */
char *Copyright = "\n"
  "calc - arbitrary precision calculator\n"
  "\n"
  "Copyright (C) 1999-2023  David I. Bell, Landon Curt Noll "
					   "and Ernest Bowen\n"
  "\n"
  "Initial author:  David I. Bell\n"
  "\n"
  "Calc is open software; you can redistribute it and/or modify it under\n"
  "the terms of the version 2.1 of the GNU Lesser General Public License\n"
  "as published by the Free Software Foundation.\n";
char *Usability = "\n"
  "Calc is distributed in the hope that it will be useful, but WITHOUT\n"
  "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n"
  "or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU Lesser General\n"
  "Public License for more details.\n";
char *COPYING_LGPL = "\n"
  "A copy of version 2.1 of the GNU Lesser General Public License is\n"
  "distributed with calc under the filename COPYING-LGPL.  You should have\n"
  "received a copy with calc; if not, write to Free Software Foundation, Inc.\n"
  "51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
  "\n"
  "For license details use the command:\thelp copying\n"
  "The COPYING-LGPL file may be viewed with:\thelp copying-lgpl\n"
  "\n";


/*
 * version - return version string
 *
 * This function returns a malloced version string.  This version
 * string does not contain the title, just:
 *
 *		x.y.z.w
 */
char *
version(void)
{
	char verbuf[BUFSIZ+1];		/* form version string here */
	size_t len;			/* length of version string */

	/*
	 * return previously stored version if one exists
	 */
	if (stored_version) {
		return stored_version;
	}

	/*
	 * form the version buffer
	 */
	snprintf(verbuf, BUFSIZ,
	    "%d.%d.%d.%d", calc_major_ver, calc_minor_ver,
	     calc_major_patch, calc_minor_patch);
	verbuf[BUFSIZ] = '\0';	/* paranoia */

	/*
	 * save the versions string into a newly malloced buffer
	 */
	len = strlen(verbuf);
	stored_version = (char *)calloc(len+1, sizeof(verbuf[0]));
	if (stored_version == NULL) {
		fprintf(stderr, "%s: cannot malloc version string\n", program);
		exit(70);
	}
	strlcpy(stored_version, verbuf, len+1);

	/*
	 * return the newly malloced buffer
	 */
	return stored_version;
}


#if defined(CALC_VER)


/*
 * print_3_level_version - print just the major part version string
 *
 * This function prints the major part version string:
 *
 *		x.y.z
 */
void
print_3_level_version(void)
{
	/*
	 * form the version buffer
	 */
	printf("%d.%d.%d\n", calc_major_ver, calc_minor_ver,
			     calc_major_patch);
	return;
}


/*
 * version - print the calc version
 */
/*ARGSUSED*/
int
main(int argc, char *argv[])
{
	program = argv[0];
	/*
	 * case: -V - print 3-level version
	 */
	if (argc == 2 && strcmp(argv[1], "-V") == 0) {
		print_3_level_version();

	/*
	 * case: no args - print 4-level version
	 */
	} else if (argc == 1) {
		printf("%s\n", version());

	/*
	 * case: -h or wrong number of args or invalid options
	 */
	} else {
		fprintf(stderr,
		    "usage: %s [-V | -h]\n"
		    "\n"
		    "    -h	print this message and exit non-zero\n"
		    "    -V	print 3-level version (def: print 4-level version)\n",
		    program);
		exit(75);
	}
	return 0;
}

#endif /* CALC_VER */
