/*
 * Copyright (c) 1999 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * version - determine the version of calc
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
#define MAJOR_PATCH	0	/* patch level or 0 if no patch */
#define MINOR_PATCH	"9.4.3"	/* test number or empty string if no patch */

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
 * version - print the calc version
 */
/*ARGSUSED*/
int
main(int argc, char *argv[])
{
    program = argv[0];
    printf("%s\n", version());
    return 0;
}

#endif /* CALC_VER */
