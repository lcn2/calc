/* XXX - this code is currently not really used, but it will be soon */
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
 */

#include "value.h"


/*
 * hash function interface table
 *
 * htbl[i] is the interface for hash algorithm i
 */
static HASHFUNC htbl[HASH_TYPE_MAX+1];


/*
 * static functions
 */
static void load_htbl(void (*h_func)(HASHFUNC*), HASHFUNC*);


/*
 * hash_init - initialize hash function interface table
 *
 * We will load the hash function interface table and ensure that it is
 * completely filled.
 *
 * This function does not return if an error is encountered.
 */
void
hash_init(void)
{
	int i;

	/*
	 * setup
	 */
	for (i=0; i <= HASH_TYPE_MAX; ++i) {
		htbl[i].type = -1;
	}

	/*
	 * setup the hash function interface for all hashes
	 */
	load_htbl(shs_hashfunc, htbl);

	/*
	 * verify that our interface table is fully populated
	 */
	for (i=0; i <= HASH_TYPE_MAX; ++i) {
		if (htbl[i].type != i) {
			fprintf(stderr, "htbl[%d] is bad\n", i);
			exit(1);
		}
	}
}


/*
 * load_htbl - load a hash function interface table slot
 *
 * We will call the h_func function, sanity check the function type
 * and check to be sure that the slot is unused.
 *
 * given:
 *	h_func	- a function that returns a HASHFUNC entry
 *	h	- a array of hash function interfaces
 *
 * This function does not return if an error is encountered.
 */
static void
load_htbl(void (*h_func)(HASHFUNC*), HASHFUNC *h)
{
	HASHFUNC hent;		/* hash function interface entry */

	/*
	 * call the HASHFUNC interface function
	 */
	h_func(&hent);

	/*
	 * sanity check the type
	 */
	if (hent.type < 0 || hent.type > HASH_TYPE_MAX) {
		fprintf(stderr, "bad HASHFUNC type: %d\n", hent.type);
		exit(1);
	}
	if (h[hent.type].type >= 0) {
		fprintf(stderr, "h[%d].type: %d already in use\n",
		    hent.type, h[hent.type].type);
		exit(1);
	}

	/*
	 * load the entry
	 */
	h[hent.type] = hent;
}
