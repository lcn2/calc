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
 * are welcome.	 Send EMail to the address given below.
 *
 * Happy bit twiddling,
 *
 *	Landon Curt Noll
 *	http://reality.sgi.com/chongo/
 *
 * chongo <was here> /\../\
 */


#include "../have_const.h"
#include "../value.h"
#include "../custom.h"

/*
 * NOTE: See the file CUSTOM for instructions on how to add
 *	 custom functions.
 */


#if defined(CUSTOM)


/*
 * add your forward custom function declaractions here
 *
 * Declare custom functions as follows:
 *
 *	extern VALUE c_xyz(char*, int, VALUE**);
 *
 * We suggest that you sort the entries below by name.
 */
extern VALUE c_argv(char*, int, VALUE**);
extern VALUE c_devnull(char*, int, VALUE**);
extern VALUE c_help(char*, int, VALUE**);
extern VALUE c_sysinfo(char*, int, VALUE**);
extern VALUE c_pzasusb8(char*, int, VALUE**);


#endif /* CUSTOM */


/*
 * custom interface table
 *
 * The order of the elements in struct custom are:
 *
 *	{ "xyz", "brief description of the xyz custom function",
 *	   minimum_args, maximum_args, c_xyz },
 *
 * where:
 *
 *	minimum_args	an int >= 0
 *	maximum_args	an int >= minimum_args and <= MAX_CUSTOM_ARGS
 *
 * Use MAX_CUSTOM_ARGS for maximum_args is the maximum number of args
 * is potentially 'unlimited'.
 *
 * If the brief description cannot fit on the same line as the name
 * without wrapping on a 80 col window, the description is probably
 * too long and will not look nice in the show custom output.
 */
CONST struct custom cust[] = {

#if defined(CUSTOM)


	/*
	 * add your own custom functions here
	 *
	 * We suggest that you sort the entries below by name
	 * so that show custom will produce a nice sorted list.
	 */

	{ "argv", "information about its args, returns arg count",
	 0, MAX_CUSTOM_ARGS, c_argv },

	{ "devnull", "does nothing",
	 0, MAX_CUSTOM_ARGS, c_devnull },

	{ "help", "help for custom functions",
	 1, 1, c_help },

	{ "sysinfo", "return a calc #define value",
	 0, 1, c_sysinfo },

	{ "pzasusb8", "print ZCALUE as USB8",
	 0, 1, c_pzasusb8 },


#endif /* CUSTOM */

	/*
	 * This must be at the end of this table!!!
	 */
	{NULL, NULL,
	 0, 0, NULL}
};
