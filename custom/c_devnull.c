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


#include "../have_const.h"
#include "../value.h"
#include "../custom.h"


/*
 * c_devnull - a custom function that does nothing
 *
 * This custom function does nothing.  It is useful as a test hook
 * for looking at the general interface.
 */
/*ARGSUSED*/
VALUE
c_devnull(char *name, int count, VALUE **vals)
{
	VALUE result;		/* what we will return */

	/*
	 * return NULL
	 */
	result.v_type = V_NULL;
	return result;
}

#endif /* CUSTOM */
