/*
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted.
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
 *			Landon Curt Noll
 *
 *			chongo@toad.com
 *			...!{pyramid,sun,uunet}!hoptoad!chongo
 *
 * chongo was here	/\../\
 */

#if defined(CUSTOM)

#include <stdio.h>

#include "../have_const.h"
#include "../value.h"
#include "../custom.h"

#include "../config.h"
#include "../calc.h"

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
c_argv(char *name, int count, VALUE **vals)
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
		case V_RAND:    /* address of additive 55 random state */
			type = "rand_state";
			break;
		case V_RANDOM:  /* address of Blum random state */
			type = "random_state";
			break;
		case V_CONFIG:  /* configuration state */
			type = "config_state";
			break;
		case V_HASH:	/* hash state */
			type = "hash_state";
			break;
		case V_BLOCK:	/* memory block */
			type = "octet_block";
			break;
#if 0
		/* XXX - V_OCTET is subject to change */
		case V_OCTET: 	/* octet (unsigned char) */
			type = "octet";
			break;
#endif
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
			printf("\tsizeof=%ld\n", lsizeof(vals[i]));
		} else {
			printf("\tsize=%ld\tsizeof=%ld\n",
			    elm_count(vals[i]), lsizeof(vals[i]));
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
