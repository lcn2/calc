/*
 * errno - Determine how to declare errno, sys_errlist and sys_nerr
 *
 * On most machines: errno sys_errlist and sys_nerr are declared
 * by either <stdio.h> and/or <errno.h>.  But some systems declare
 * them somewhere else or do not declare them at all!
 *
 * If the system were doing a proper job in headers, this should declare them:
 *
 *	#include <stdio.h>
 *	#include <errno.h>
 *
 * But one some systems one must explicitly declare them as:
 *
 *	extern int errno;
 *	extern const char *const sys_errlist[];
 *	extern int sys_nerr;
 *
 * and on some old systems they must be explicitly and incorrectly declared as:
 *
 *	extern int errno;
 *	extern char *sys_errlist[];
 *	extern int sys_nerr;
 *
 * The purpose of this utility is try and find the right way to declare
 * them and to output the middle of a header file called calc_errno.h.
 */

/*
 * Copyright (c) 1999 by Landon Curt Noll.  All Rights Reserved.
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
 * chongo was here	/\../\
 */

#include <stdio.h>
#if defined(TRY_ERRNO_NO_DECL)
#  include <errno.h>
#endif /* TRY_ERRNO_NO_DECL */

int
main(void)
{

#if defined(TRY_ERRNO_NO_DECL)
	printf("/*\n");
	printf(" * The following comments were produced by calc_errno\n");
	printf(" * in an effort to see if these values were correctly\n");
	printf(" * declared when calc_errno.c was compiled.\n");
	printf(" */\n\n");
	printf("/* Initially errno is %d */\n", errno);
	printf("/* There are %d entries in sys_errlist[] */\n", sys_nerr);
	printf("/* The 2nd sys_errlist entry is \"%s\" */\n\n", sys_errlist[1]);
	printf("/*\n");
	printf(" * Based on the above, calc_errno now knows\n");
	printf(" * how to declare errno and friends.\n");
	printf(" */\n\n");
	printf("#include <stdio.h>\n");
	printf("#include <errno.h>\n");
#elif defined(TRY_ERRNO_OLD_DECL)
	extern int errno;		/* last system error */
	extern char *sys_errlist[];	/* system error messages */
	extern int sys_nerr;		/* number of system errors*/

	printf("/*\n");
	printf(" * The following comments were produced by calc_errno\n");
	printf(" * in an effort to see if these values were correctly\n");
	printf(" * declared when calc_errno.c was compiled.\n");
	printf(" */\n\n");
	printf("/* Initially errno is %d */\n", errno);
	printf("/* There are %d entries in sys_errlist[] */\n", sys_nerr);
	printf("/* The 2nd sys_errlist entry is \"%s\" */\n\n", sys_errlist[1]);
	printf("/*\n");
	printf(" * Based on the above, calc_errno now knows\n");
	printf(" * how to declare errno and friends.\n");
	printf(" */\n\n");
	printf("extern int errno;\t\t/* last system error */\n");
	printf("extern char *sys_errlist[];\t"
	       "/* system error messages */\n");
	printf("extern int sys_nerr;\t\t/* number of system errors*/\n");
#else	/* assume defined(TRY_ERRNO_STD_DECL) */
	extern int errno;			/* last system error */
	extern const char *const sys_errlist[];	/* system error messages */
	extern int sys_nerr;			/* number of system errors*/

	printf("/*\n");
	printf(" * The following comments were produced by calc_errno\n");
	printf(" * in an effort to see if these values were correctly\n");
	printf(" * declared when calc_errno.c was compiled.\n");
	printf(" */\n\n");
	printf("/* Initially errno is %d */\n", errno);
	printf("/* There are %d entries in sys_errlist[] */\n", sys_nerr);
	printf("/* The 2nd sys_errlist entry is \"%s\" */\n\n", sys_errlist[1]);
	printf("/*\n");
	printf(" * Based on the above, calc_errno now knows\n");
	printf(" * how to declare errno and friends.\n");
	printf(" */\n\n");
	printf("extern int errno;\t\t\t/* last system error */\n");
	printf("extern const char *const sys_errlist[];\t"
	       "/* system error messages */\n");
	printf("extern int sys_nerr;\t\t\t/* number of system errors*/\n");
#endif

	/* exit(0); */
	return 0;
}
