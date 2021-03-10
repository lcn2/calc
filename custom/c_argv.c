/*
 * c_argv - a custom function display info about its args
 *
 * Copyright (C) 1999-2006,2021  Landon Curt Noll
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
 * Under source code control:	1997/03/09 20:27:37
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if defined(CUSTOM)

#include <stdio.h>

#include "have_const.h"
#include "value.h"
#include "custom.h"

#include "config.h"
#include "calc.h"

#include "have_unused.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * c_argv - a custom function display info about its args
 *
 * given:
 *	vals[i]	  and arg to display information about
 *
 * returns:
 *	count
 */
/*ARGSUSED*/
VALUE
c_argv(char *UNUSED(name), int count, VALUE **vals)
{
	VALUE result;		/* what we will return */
	ZVALUE zfilelen;	/* length of a file as a ZVALUE */
	NUMBER *filelen;	/* pointer to length of a file as a NUMER */
	char *type;		/* the name of the arg type */
	int i;

	/*
	 * print info on each arg
	 */
	for (i=0; i < count; ++i) {

		/*
		 * print arg number with leading tab as configured
		 */
		printf("%sarg[%d]", (conf->tab_ok ? "\t" : ""), i);

		/*
		 * print the arg type
		 */
		switch (vals[i]->v_type) {
		case V_NULL:	/* null value */
			type = "null";
			break;
		case V_INT:	/* normal integer */
			type = "int";
			break;
		case V_NUM:	/* number */
			type = "rational_value";
			break;
		case V_COM:	/* complex number */
			type = "complex_value";
			break;
		case V_ADDR:	/* address of variable value */
			type = "address";
			break;
		case V_STR:	/* address of string */
			type = "string";
			break;
		case V_MAT:	/* address of matrix structure */
			type = "matrix";
			break;
		case V_LIST:	/* address of list structure */
			type = "list";
			break;
		case V_ASSOC:	/* address of association structure */
			type = "assoc";
			break;
		case V_OBJ:	/* address of object structure */
			type = "ocject";
			break;
		case V_FILE:	/* opened file id */
			type = "file";
			break;
		case V_RAND:	/* address of additive 55 random state */
			type = "rand_state";
			break;
		case V_RANDOM:	/* address of Blum random state */
			type = "random_state";
			break;
		case V_CONFIG:	/* configuration state */
			type = "config_state";
			break;
		case V_HASH:	/* hash state */
			type = "hash_state";
			break;
		case V_BLOCK:	/* memory block */
			type = "octet_block";
			break;
		case V_OCTET:	/* octet (unsigned char) */
			type = "octet";
			break;
		default:
			type = "unknown";
			break;
		}
		printf("\t%-16s", type);

		/*
		 * print size and sizeof information
		 *
		 * We have to treat files in a special way
		 * because their length can be very long.
		 */
		if (vals[i]->v_type == V_FILE) {
			/* get the file length */
			if (getsize(vals[i]->v_file, &zfilelen) == 0) {
				filelen = qalloc();
				filelen->num = zfilelen;
				qprintfd(filelen, 0L);
				qfree(filelen);
			} else {
				/* getsize error */
				printf("\tsize=unknown");
			}
			printf("\tsizeof=%ld\n", (long int)lsizeof(vals[i]));
		} else {
			printf("\tsize=%ld\tsizeof=%ld\n",
			    elm_count(vals[i]), (long int)lsizeof(vals[i]));
		}
	}

	/*
	 * return count
	 */
	result.v_type = V_NUM;
	result.v_num = itoq(count);
	return result;
}

#endif /* CUSTOM */
