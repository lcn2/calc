/*
 * Copyright (c) 1996 by Landon Curt Noll.  All Rights Reserved.
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


#include "calc.h"
#include "zmath.h"

static int init_done = 0;		/* 1 => we already initialized */


/*
 * libcalc_call_me_first - users of libcalc.a must call this function
 */
void
libcalc_call_me_first(void)
{
	/*
	 * do nothing if we are initialized already
	 */
	if (init_done) {
		return;
	}

	/* 
	 * setup configuration values 
	 */
	oldstd.epsilon = &_qonesqbase_; /* magic to fake early str2q() */
	conf = config_copy(&oldstd); /* more magic to fake early str2q() */
	oldstd.epsilon = str2q(EPSILON_DEFAULT);
	newstd.epsilon = str2q(NEW_EPSILON_DEFAULT);

	/*
	 * make oldstd our default config
	 */
	conf = config_copy(&oldstd);

	/*
	 * ZVALUE io initialization
	 */
	zio_init();

	/*
	 * ready to rock & roll ..
	 */
	init_done = 1;
	return;
}
