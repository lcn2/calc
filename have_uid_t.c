/*
 * have_uid_t - Determine if we want or can support uid_t
 *
 * usage:
 *	have_uid_t
 *
 * Not all compilers support uid_t, so this may not compile on your system.
 *
 * This prog outputs several defines:
 *
 *	HAVE_UID_T
 *		defined ==> ok to use uid_t
 *		undefined ==> do not use uid_t
 */
/*
 * Copyright (c) 1995 by Landon Curt Noll.  All Rights Reserved.
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

#if !defined(HAVE_NO_UID_T)
#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include <pwd.h>
#include <sys/types.h>
#endif /* ! HAVE_NO_UID_T */

int
main(void)
{
#if defined(HAVE_NO_UID_T)
	printf("#undef HAVE_UID_T /* no */\n");
#else /* HAVE_NO_UID_T */
	uid_t curds;
	extern uid_t geteuid();

	curds = geteuid();

	printf("#define HAVE_UID_T /* yes */\n");
#endif /* HAVE_NO_UID_T */
	/* exit(0); */
	return 0;
}
