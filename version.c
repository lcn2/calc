/*
 * Copyright (c) 1996 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * version - determine the version of calc
 */

#include "calc.h"

#define MAJOR_VER	2	/* major version */
#define MINOR_VER	10	/* minor version */
#define PATCH_LEVEL	2	/* patch level */
#define SUB_PATCH_LEVEL	"t30"	/* test number or empty string */


void
version(FILE *stream)
{
	fprintf(stream,
	    "C-style arbitrary precision calculator (version %d.%d.%d%s)\n",
	     MAJOR_VER, MINOR_VER, PATCH_LEVEL, SUB_PATCH_LEVEL);
}

/* END CODE */
