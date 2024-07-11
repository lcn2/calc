/*
 * custtbl - custom interface table
 *
 * Copyright (C) 1999-2007,2021,2023  Landon Curt Noll
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
 * Under source code control:	1997/03/09 02:28:54
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <unistd.h>

#include "../have_const.h"
#include "../value.h"
#include "../custom.h"


#include "../errtbl.h"
#include "../banned.h"	/* include after system header <> includes */


/*
 * custom_compiled - determine if custom functions are compiled into libcustcalc
 *
 * returns:
s*	true  ==> libcustcalc was compiled with CUSTOM defined
 *	false ==> libcustcalc was compiled with CUSTOM undefined
 */
E_FUNC bool
custom_compiled(void)
{
#if defined(CUSTOM)
	return true;
#else /* CUSTOM */
	return false;
#endif /* CUSTOM */
}


/*
 * NOTE: See the file HOW_TO_ADD for instructions on how to add custom functions.
 */


#if defined(CUSTOM)


/*
 * add your forward custom function declarations here
 *
 * Declare custom functions as follows:
 *
 *	E_FUNC VALUE c_xyz(char*, int, VALUE**);
 *
 * We suggest that you sort the entries below by name.
 */
E_FUNC VALUE c_argv(char*, int, VALUE**);
E_FUNC VALUE c_devnull(char*, int, VALUE**);
E_FUNC VALUE c_help(char*, int, VALUE**);
E_FUNC VALUE c_sysinfo(char*, int, VALUE**);
E_FUNC VALUE c_pzasusb8(char*, int, VALUE**);
E_FUNC VALUE c_pmodm127(char*, int, VALUE**);
E_FUNC VALUE c_register(char*, int, VALUE**);

#define U_FUNC(name) E_FUNC VALUE name(char*, int, VALUE**);

// pipe/fork/exec
U_FUNC(u_pfe_fork)
U_FUNC(u_pfe_pipe)
U_FUNC(u_pfe_close)
U_FUNC(u_pfe_execvp)
U_FUNC(u_pfe_dup)
U_FUNC(u_pfe_dup2)
U_FUNC(u_pfe_write)
U_FUNC(u_pfe_read)
U_FUNC(u_pfe_select)
U_FUNC(u_pfe_poll)

U_FUNC(u_pfe_wait4)

U_FUNC(u_pfe_pfe)
U_FUNC(u_pfe_pwrite)
U_FUNC(u_pfe_pread)

// vike's various additions
U_FUNC(u_vadd_basename)
U_FUNC(u_vadd_dirname)
U_FUNC(u_vadd_getcwd)
U_FUNC(u_vadd_getpid)
U_FUNC(u_vadd_getppid)
U_FUNC(u_vadd_inputname) //cspell:ignore inputname

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

	{ "pzasusb8", "print ZVALUE as USB8",
	 0, 1, c_pzasusb8 },

	{ "pmodm127", "calculate q mod 2^(2^127-1)",
	 1, 1, c_pmodm127 },

	{ "register", "get or set customer registers",
	 1, 2, c_register },

	{ "fork", "create process",
	 0, 0, u_pfe_fork },
	{ "pipe", "create descriptor pair for interprocess communication",
	 0, 0, u_pfe_pipe },
	{ "dup", "duplicate a file descriptor",
	 1, 1, u_pfe_dup },
	{ "dup2", "duplicate a file descriptor",
	 2, 2, u_pfe_dup2 },
	{ "close", "remove a file descriptor",
	 1, 1, u_pfe_close },
	{ "execvp", "execute a file",
	 2, 2, u_pfe_execvp },
	{ "write", "write output",
	 2, 2, u_pfe_write },
	{ "read", "read input",
	 1, 2, u_pfe_read },
	{ "select", "examine file descriptors",
	 3, 4, u_pfe_select },
	{ "poll", "synchronous I/O multiplexing",
	 2, 3, u_pfe_poll },
	
	{ "wait4", "wait for process",
	 1, 4, u_pfe_wait4 },
	
	{ "pfe", "pipe/fork/exec",
	 4, 4, u_pfe_pfe },
	{ "pwrite", "write and close",
	 2, 2, u_pfe_pwrite },
	{ "pread", "read until eof, close and wait for exit status",
	 3, 3, u_pfe_pread },
	
	{ "getpid", "get calling process identification",
	 0, 0, u_vadd_getpid },
	{ "getppid", "get parent process identification",
	 0, 0, u_vadd_getppid },
	{ "getcwd", "get working directory pathname",
	 0, 0, u_vadd_getcwd },
	{ "inputname", "get name of input",
	 0, 0, u_vadd_inputname },
	{ "basename", "extract the base portion of a pathname",
	 1, 1, u_vadd_basename },
	{ "dirname", "extract the directory part of a pathname",
	 1, 1, u_vadd_dirname },


#endif /* CUSTOM */

	/*
	 * This must be at the end of this table!!!
	 */
	{NULL, NULL,
	 0, 0, NULL}
};
