/*
 * version - determine the version of calc
 *
 * Copyright (C) 1999  David I. Bell and Landon Curt Noll
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
 * @(#) $Revision: 29.28 $
 * @(#) $Id: version.c,v 29.28 2001/06/01 11:27:38 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/version.c,v $
 *
 * Under source code control:	1990/05/22 11:00:58
 * File existed as early as:	1990
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#if defined(CALC_VER)
#include <stdlib.h>
#include <unistd.h>
static char *program;
#else
# include "calc.h"
#endif

#define MAJOR_VER	2	/* major version */
#define MINOR_VER	11	/* minor version */
#define MAJOR_PATCH	5	/* patch level or 0 if no patch */
#define MINOR_PATCH	"4.2"	/* test number or empty string if no patch */

/*
 * calc version constants
 */
int calc_major_ver = MAJOR_VER;
int calc_minor_ver = MINOR_VER;
int calc_major_patch = MAJOR_PATCH;
char *calc_minor_patch = MINOR_PATCH;


/*
 * stored version
 */
static char *stored_version = NULL;	/* version formed if != NULL */


/*
 * stored license info - has a side effect of copyrighting the binary
 */
char *Copyright = "\n"
  "calc - arbitrary precision calculator\n"
  "\n"
  "@(#) Copyright (C) 1999  David I. Bell, Landon Curt Noll and Ernest Bowen\n"
  "\n"
  "Primary author:  David I. Bell\n"
  "\n"
  "Calc is open software; you can redistribute it and/or modify it under\n"
  "the terms of the version 2.1 of the GNU Lesser General Public License\n"
  "as published by the Free Software Foundation.\n"
  "\n"
  "Calc is distributed in the hope that it will be useful, but WITHOUT\n"
  "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n"
  "or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU Lesser General\n"
  "Public License for more details.\n"
  "\n"
  "A copy of version 2.1 of the GNU Lesser General Public License is\n"
  "distributed with calc under the filename COPYING-LGPL.  You should have\n"
  "received a copy with calc; if not, write to Free Software Foundation, Inc.\n"
  "59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.\n"
  "\n"
  "@(#) For license details use the command:\thelp copying\n"
  "The COPYING-LGPL file may be viewed with:\thelp copying-lgpl\n"
  "\n";


/*
 * version - return version string
 *
 * This function returns a malloced version string.  This version
 * string does not contain the title, just:
 *
 *		x.y.ztw
 *		x.y.z
 *		x.y
 */
char *
version(void)
{
	char verbuf[BUFSIZ+1];		/* form version string here */

	/*
	 * return previously stored version if one exists
	 */
	if (stored_version) {
		return stored_version;
	}

	/*
	 * form the version buffer
	 */
	if (sizeof(MINOR_PATCH) > 1) {
	    sprintf(verbuf,
		"%d.%d.%dt%s", calc_major_ver, calc_minor_ver,
		 calc_major_patch, calc_minor_patch);
	} else if (MAJOR_PATCH > 0) {
	    sprintf(verbuf,
		"%d.%d.%d", calc_major_ver, calc_minor_ver, calc_major_patch);
	} else {
	    sprintf(verbuf, "%d.%d", calc_major_ver, calc_minor_ver);
	}

	/*
	 * save the versions string into a newly malloced buffer
	 */
	stored_version = (char *)malloc(strlen(verbuf)+1);
	if (stored_version == NULL) {
		fprintf(stderr, "%s: version formation value\n", program);
		exit(2);
	}
	strcpy(stored_version, verbuf);

	/*
	 * return the newly malloced buffer
	 */
	return stored_version;
}


#if defined(CALC_VER)


/*
 * print_rpm_version_release - print the rpm style version-release
 *
 * This function prints a version-release string, rpm style:
 *
 *		x.y.z-w
 *		x.y.z-0
 *		x.y-0
 */
void
print_rpm_version_release(void)
{
	/*
	 * form the version buffer
	 */
	if (sizeof(MINOR_PATCH) > 1) {
	    printf("%d.%d.%d-%s\n", calc_major_ver, calc_minor_ver,
		 		  calc_major_patch, calc_minor_patch);
	} else if (MAJOR_PATCH > 0) {
	    printf("%d.%d.%d-0\n", calc_major_ver, calc_minor_ver,
	    			 calc_major_patch);
	} else {
	    printf("%d.%d-0\n", calc_major_ver, calc_minor_ver);
	}
	return;
}


/*
 * print_rpm_version - print just the version string, rpm style
 *
 * This function prints a version string, rpm style:
 *
 *		x.y.z
 *		x.y
 */
void
print_rpm_version(void)
{
	/*
	 * form the version buffer
	 */
	if (MAJOR_PATCH > 0) {
	    printf("%d.%d.%d\n", calc_major_ver, calc_minor_ver,
	    			 calc_major_patch);
	} else {
	    printf("%d.%d\n", calc_major_ver, calc_minor_ver);
	}
	return;
}


/*
 * print_rpm_release - print just the release string, rpm style
 *
 * This function prints a release string, rpm style:
 *
 *		w
 *		0
 */
void
print_rpm_release(void)
{
	/*
	 * form the version buffer
	 */
	if (sizeof(MINOR_PATCH) > 1) {
	    printf("%s\n", calc_minor_patch);
	} else {
	    printf("0\n");
	}
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
    if (argc == 2 && strcmp(argv[1], "-R") == 0) {
	print_rpm_version_release();
    } else if (argc == 2 && strcmp(argv[1], "-v") == 0) {
	print_rpm_version();
    } else if (argc == 2 && strcmp(argv[1], "-r") == 0) {
	print_rpm_release();
    } else {
	printf("%s\n", version());
    }
    return 0;
}

#endif /* CALC_VER */
