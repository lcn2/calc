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
 *      Landon Curt Noll
 *      http://reality.sgi.com/chongo/
 *
 * chongo <was here> /\../\
 */

#if defined(CUSTOM)


#include "../have_const.h"
#include "../value.h"
#include "../custom.h"


/*
 * c_help - custom help function
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
 *	vals[0]	   name of the custom help file to directly access
 */
/*ARGSUSED*/
VALUE
c_help(char *name, int count, VALUE **vals)
{
	VALUE result;		/* what we will return */

	/*
	 * parse args
	 */
	if (vals[0]->v_type != V_STR) {
		math_error("custom help arg 1 must be a string");
		/*NOTREACHED*/
	}

	/*
	 * give the help
	 */
	customhelp((char *)vals[0]->v_str);

	/*
	 * return NULL
	 */
	result.v_type = V_NULL;
	return result;
}

#endif /* CUSTOM */
