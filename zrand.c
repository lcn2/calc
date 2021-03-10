/*
 * zrand - subtractive 100 shuffle generator
 *
 * Copyright (C) 1999-2007,2021  Landon Curt Noll
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
 * Under source code control:	1995/01/07 09:45:25
 * File existed as early as:	1994
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * AN OVERVIEW OF THE FUNCTIONS:
 *
 * This module contains an Subtractive 100 shuffle generator wrapped inside
 * of a shuffle generator.
 *
 *	We refer to this generator as the s100 generator.
 *
 *	rand	- s100 shuffle generator
 *	srand	- seed the s100 shuffle generator
 *
 *	This generator has two distinct parts, the s100 generator
 *	and the shuffle generator.
 *
 *	The subtractive 100 generator is described in Knuth's "The Art of
 *	Computer Programming - Seminumerical Algorithms", Vol 2, 3rd edition
 *	(1998), Section 3.6, page 186, formula (2).
 *
 *	The "use only the first 100 our of every 1009" is described in
 *	Knuth's "The Art of Computer Programming - Seminumerical Algorithms",
 *	Vol 2, 3rd edition (1998), Section 3.6, page 188".
 *
 *	The period and other properties of this generator make it very
 *	useful to 'seed' other generators.
 *
 *	The shuffle generator is described in Knuth's "The Art of Computer
 *	Programming - Seminumerical Algorithms", Vol 2, 3rd edition (1998),
 *	Section 3.2.2, page 34, Algorithm B.
 *
 *	The shuffle generator is fast and serves as a fairly good standard
 *	pseudo-random generator.   If you need a fast generator and do not
 *	need a cryptographically strong one, this generator is likely to do
 *	the job.
 *
 *	The shuffle generator is feed values by the subtractive 100 process.
 *
 ******************************************************************************
 *
 * GOALS:
 *
 * The goals of this package are:
 *
 *	all magic numbers are explained
 *
 *	    I distrust systems with constants (magic numbers) and tables
 *	    that have no justification (e.g., DES).  I believe that I have
 *	    done my best to justify all of the magic numbers used.
 *
 *	 full documentation
 *
 *	    You have this source file, plus background publications,
 *	    what more could you ask?
 *
 *	large selection of seeds
 *
 *	    Seeds are not limited to a small number of bits.  A seed
 *	    may be of any size.
 *
 *	the strength of the generators may be tuned to meet the need
 *
 *	    By using the appropriate seed and other arguments, one may
 *	    increase the strength of the generator to suit the need of
 *	    the application.  One does not have just a few levels.
 *
 * Even though I have done my best to implement a good system, you still
 * must use these routines your own risk.
 *
 * Share and enjoy!  :-)
 */

/*
 * ON THE GENERATORS:
 *
 * The subtractive 100 generator has a good period, and is fast.  It is
 * reasonable as generators go, though there are better ones available.
 * The shuffle generator has a very good period, and is fast.  It is
 * fairly good as generators go, particularly when it is feed reasonably
 * random numbers.  Because of this, we use feed values from the subtractive
 * 100 process into the shuffle generator.
 *
 * The s100 generator uses 2 tables:
 *
 *	subtractive table - 100 entries of 64 bits used by the subtractive 100
 *			    part of the s100 generator
 *
 *	shuffle table - 256 entries of 64 bits used by the shuffle
 *			part of the s100 generator and feed by the
 *			subtractive table.
 *
 * Casual direct use of the shuffle generator may be acceptable.  If one
 * desires cryptographically strong random numbers, or if one is paranoid,
 * one should use the Blum generator instead (see zrandom.c).
 *
 * The s100 generator as the following calc interfaces:
 *
 *   rand(min,beyond)		(where min < beyond)
 *
 *	Print an s100 generator random value over interval [a,b).
 *
 *   rand()
 *
 *	Same as rand(0, 2^64).	Print 64 bits.
 *
 *   rand(lim)			(where 0 > lim)
 *
 *	Same as rand(0, lim).
 *
 *   randbit(x)			(where x > 0)
 *
 *	Same as rand(0, 2^x).  Print x bits.
 *
 *   randbit(skip)		(where skip < 0)
 *
 *	Skip random bits and return the bit skip count (-skip).
 */

/*
 * INITIALIZATION AND SEEDS:
 *
 * All generators come already seeded with precomputed initial constants.
 * Thus, it is not required to seed a generator before using it.
 *
 * The s100 generator may be initialized and seeded via srand().
 *
 * Using a seed of '0' will reload generators with their initial states.
 *
 *	srand(0)	restore subtractive 100 generator to the initial state
 *
 * The above single arg calls are fairly fast.
 *
 * Optimal seed range for the s100 generator:
 *
 *	There is no limit on the size of a seed.  On the other hand,
 *	extremely large seeds require large tables and long seed times.
 *	Using a seed in the range of [2^64, 2^64 * 100!) should be
 *	sufficient for most purposes.  An easy way to stay within this
 *	range to to use seeds that are between 21 and 178 digits, or
 *	64 to 588 bits long.
 *
 *	To help make the generator produced by seed S, significantly
 *	different from S+1, seeds are scrambled prior to use.  The
 *	function randreseed64() maps [0,2^64) into [0,2^64) in a 1-to-1
 *	and onto fashion.
 *
 *	The purpose of the randreseed64() is not to add security.  It
 *	simply helps remove the human perception of the relationship
 *	between the seed and the production of the generator.
 *
 *	The randreseed64() process does not reduce the security of the
 *	generators.  Every seed is converted into a different unique seed.
 *	No seed is ignored or favored.
 *
 ******************************************************************************
 *
 * srand(seed)
 *
 *    Seed the s100 generator.
 *
 *    seed != 0:
 *    ---------
 *	Any buffered random bits are flushed.  The subtractive table is loaded
 *	with the default subtractive table.  The low order 64 bits of seed is
 *	xor-ed against each table value.  The subtractive table is shuffled
 *	according to seed/2^64.
 *
 *	The following calc code produces the same effect:
 *
 *	    (* reload default subtractive table xored with low 64 seed bits *)
 *	    seed_xor = seed & ((1<<64)-1);
 *	    for (i=0; i < 100; ++i) {
 *		subtractive[i] = xor(default_subtractive[i], seed_xor);
 *	    }
 *
 *	    (* shuffle the subtractive table *)
 *	    seed >>= 64;
 *	    for (i=100; seed > 0 && i > 0; --i) {
 *		quomod(seed, i+1, seed, j);
 *		swap(subtractive[i], subtractive[j]);
 *	    }
 *
 *	Seed must be >= 0.  All seed values < 0 are reserved for future use.
 *
 *	The subtractive 100 pointers are reset to subtractive[36] and
 *	subtractive[99].  Last the shuffle table is loaded with successive
 *	values from the	subtractive 100 generator.
 *
 *    seed == 0:
 *    ---------
 *	Restore the initial state and modulus of the s100 generator.
 *	After this call, the s100 generator is restored to its initial
 *	state after calc started.
 *
 *	The subtractive 100 pointers are reset to subtractive[36] and
 *	subtractive[99].  Last the shuffle table is loaded with successive
 *	values from the	subtractive 100 generator.
 *
 ******************************************************************************
 *
 * srand(mat100)
 *
 *    Seed the s100 generator.
 *
 *    Any buffered random bits are flushed.  The subtractive table with the
 *    first 100 entries of the array mat100, mod 2^64.
 *
 *    The subtractive 100 pointers are reset to subtractive[36] and
 *    subtractive[99].  Last the shuffle table is loaded with successive
 *    values from the subtractive 100 generator.
 *
 ******************************************************************************
 *
 * srand()
 *
 *    Return current s100 generator state.  This call does not alter
 *    the generator state.
 *
 ******************************************************************************
 *
 * srand(state)
 *
 *    Restore the s100 state and return the previous state.  Note that
 *    the argument state is a rand state value (isrand(state) is true).
 *    Any internally buffered random bits are restored.
 *
 *    The states of the s100 generators can be saved by calling the seed
 *    function with no arguments, and later restored by calling the seed
 *    functions with that same return value.
 *
 *	rand_state = srand();
 *	... generate random bits ...
 *	prev_rand_state = srand(rand_state);
 *	... generate the same random bits ...
 *	srand() == prev_rand_state;		(* is true *)
 *
 *    Saving the state just after seeding a generator and restoring it later
 *    as a very fast way to reseed a generator.
 */

/*
 * TRUTH IN ADVERTISING:
 *
 * A "truth in advertising" issue is the use of the term
 * 'pseudo-random'.  All deterministic generators are pseudo random.
 * This is opposed to true random generators based on some special
 * physical device.
 *
 * A final "truth in advertising" issue deals with how the magic numbers
 * found in this file were generated.  Detains can be found in the
 * various functions, while a overview can be found in the "SOURCE FOR
 * MAGIC NUMBERS" section below.
 */

/*
 * SOURCE OF MAGIC NUMBERS:
 *
 * Most of the magic constants used on this file ultimately are
 * based on LavaRnd.  LavaRnd produced them via a cryptographic
 * of the digitization of chaotic system that consisted of a noisy
 * digital camera and 6 Lava Lite(R) lamps.
 *
 * 	BTW: Lava Lite(R) is a trademark of Haggerty Enterprises, Inc.
 *
 * The first 100 groups of 64 bit bits were used to initialize init_s100.slot.
 *
 * The function, randreseed64(), uses 2 primes to scramble 64 bits
 * into 64 bits.  These primes were also extracted from the Rand
 * Book of Random Numbers.  See randreseed64() for details.
 *
 * The shuffle table size is longer than the 100 entries recommended
 * by Knuth.  We use a power of 2 shuffle table length so that the
 * shuffle process can select a table entry from a new subtractive 100
 * value by extracting its low order bits.  The value 256 is convenient
 * in that it is the size of a byte which allows for easy extraction.
 *
 * We use the upper byte of the subtractive 100 value to select the shuffle
 * table entry because it allows all of 64 bits to play a part in the
 * entry selection.  If we were to select a lower 8 bits in the 64 bit
 * value, carries that propagate above our 8 bits would not impact the
 * s100 generator output.
 *
 ******************************************************************************
 *
 * FOR THE PARANOID:
 *
 * The truly paranoid might suggest that my claims in the MAGIC NUMBERS
 * section are a lie intended to entrap people.	 Well they are not, but
 * if you that paranoid why would you use a non-cryptographically strong
 * pseudo-random number generator in the first place?  You would be
 * better off using the random() builtin function.
 *
 * The two constants that were picked from the Rand Book of Random Numbers
 * The random numbers from the Rand Book of Random Numbers can be
 * verified by anyone who obtains the book.  As these numbers were
 * created before I (Landon Curt Noll) was born (you can look up
 * my birth record if you want), I claim to have no possible influence
 * on their generation.
 *
 * There is a very slight chance that the electronic copy of the
 * Rand Book of Random Numbers that I was given access to differs
 * from the printed text.  I am willing to provide access to this
 * electronic copy should anyone wants to compare it to the printed text.
 *
 * When using the s100 generator, one may select your own 100 subtractive
 * values by calling:
 *
 *	srand(mat100)
 *
 * and avoid using my magic numbers.  The randreseed64 process is NOT
 * applied to the matrix values. Of course, you must pick good subtractive
 * 100 values yourself!
 *
 * One might object to the complexity of the seed scramble/mapping
 * via the randreseed64() function.  The randreseed64() function maps:
 *
 *	0 ==> 0
 *	10239951819489363767 ==> 1363042948800878693
 *
 * so that srand(0) does the default action and randreseed64() remains
 * an 1-to-1 and onto map.  Thus calling srand(0) with the randreseed64()
 * process would be the same as calling srand(4967126403401436567) without
 * it.	No extra security is gained or reduced by using the randreseed64()
 * process.  The meaning of seeds are exchanged, but not lost or favored
 * (used by more than one input seed).
 */

#include <stdio.h>

#include "alloc.h"
#include "zrand.h"
#include "have_const.h"
#include "have_unused.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * default s100 generator state
 *
 * This is the state of the s100 generator after initialization, or srand(0),
 * or srand(0) is called.  The init_s100 value is never changed, only copied.
 */
STATIC CONST RAND init_s100 = {
	1,		/* seeded */
	0,		/* no buffered bits */
#if FULL_BITS == SBITS		/* buffer */
	{0},
#elif 2*FULL_BITS == SBITS
	{0, 0},
#else
   /\../\	BASEB is assumed to be 16 or 32		/\../\	 !!!
#endif
	INIT_J,			/* j */
	INIT_K,			/* k */
	RAND_CONSEQ_USE,	/* use this many before skipping values */
#if FULL_BITS == SBITS
	{		/* subtractive 100 table */
	    (FULL)U(0xc8c0370c7db7dc19), (FULL)U(0x738e33b940a06fbb),
	    (FULL)U(0x481abb76a859ed2b), (FULL)U(0x74106bb39ccdccb5),
	    (FULL)U(0x05a8eeb5c3173bfc), (FULL)U(0xefd5100d5a02e577),
	    (FULL)U(0xa69271f74030b24a), (FULL)U(0x641282fc16fe22c5),
	    (FULL)U(0x7aa7267c40438da3), (FULL)U(0x1fdf4abdc2d878d1),
	    (FULL)U(0xd9899e7a95702379), (FULL)U(0x5ea8e217d02d7f08),
	    (FULL)U(0x770587fe4d47a353), (FULL)U(0xde7d1bdd0a33a2b8),
	    (FULL)U(0x4378c3c5900e7c45), (FULL)U(0x77c9447819a514f9),
	    (FULL)U(0xfc5edb22843d1d32), (FULL)U(0x4fc42ce5e8ee5e6e),
	    (FULL)U(0xc938713c8488013e), (FULL)U(0x6a318f0320ab0cac),
	    (FULL)U(0x73e6d1a3ffc8bff3), (FULL)U(0x0cd3232a8ca96aa7),
	    (FULL)U(0x605c8036905f770d), (FULL)U(0x4d037b008b8d04a2),
	    (FULL)U(0x1ed81965cb277294), (FULL)U(0x408d9c477a254ff3),
	    (FULL)U(0x8b68587ae26c7377), (FULL)U(0xcff191a48a48832f),
	    (FULL)U(0x12d3df1d8aeb6fe6), (FULL)U(0xb2bf907e1feda37a),
	    (FULL)U(0x4e5f77193bb5f39f), (FULL)U(0x33ebcf6f8f5d1581),
	    (FULL)U(0x203c8e48d33654eb), (FULL)U(0x68d3656ef19c8a4e),
	    (FULL)U(0x3ec20b04986eb2af), (FULL)U(0x5d73a03b062c3841),
	    (FULL)U(0x836ce7095d4e49eb), (FULL)U(0x2310bc40c3f49221),
	    (FULL)U(0x3868ee48a6d0cbf6), (FULL)U(0x67578aa64a43deb1),
	    (FULL)U(0x6e3426c1150dfc26), (FULL)U(0xc541ccaa3131be30),
	    (FULL)U(0xf7e57432cec7aab2), (FULL)U(0x2b35de998cb3c873),
	    (FULL)U(0x7b9f77648663a5d7), (FULL)U(0x23b00e6aa771e5a6),
	    (FULL)U(0x859c775ca9985d05), (FULL)U(0x99636ea16b692f1f),
	    (FULL)U(0x8700ac703730800d), (FULL)U(0x461425024298a753),
	    (FULL)U(0xea4a411b809e955f), (FULL)U(0x3119ad4033709dfb),
	    (FULL)U(0xb76a6c6e5f01cb7c), (FULL)U(0x6109dc8a15984eaf),
	    (FULL)U(0x5d686db9a5ca9505), (FULL)U(0x8e80d7613b7e6add),
	    (FULL)U(0x79cbd718de6f6fd3), (FULL)U(0x40e9cd151da0f699),
	    (FULL)U(0xe82158bab24f312d), (FULL)U(0x79a4c927f5e5c36b),
	    (FULL)U(0xc25247c9a0039333), (FULL)U(0x936871161766d81d),
	    (FULL)U(0x3c6a03b4a6741327), (FULL)U(0xc8a7b6e8c002f29a),
	    (FULL)U(0x0e2a67c67bbd5ea3), (FULL)U(0x0929042d441eabc1),
	    (FULL)U(0x7dbe232a25e82085), (FULL)U(0x8cfb26e544fbac3d),
	    (FULL)U(0x8e40384d388ab983), (FULL)U(0x48dc1230554632f8),
	    (FULL)U(0xab405048ab492397), (FULL)U(0x21c9e2f5a118e387),
	    (FULL)U(0x484d1a8c343b61b5), (FULL)U(0xd49e3decab256f26),
	    (FULL)U(0xe615c7fd78f2d2e3), (FULL)U(0x8442cc33ce6cc2ed),
	    (FULL)U(0x0a3b93d844d4bbf6), (FULL)U(0x2d7e4efe9301de77),
	    (FULL)U(0x33711b76d8790d8a), (FULL)U(0xc07dc30e44df77e7),
	    (FULL)U(0xb9132ed09ddd508f), (FULL)U(0x45d06cf8c6fb43cc),
	    (FULL)U(0x22bed18ad585dd7b), (FULL)U(0x61c6cced10799ffa),
	    (FULL)U(0xd7f2393be4bd9aa9), (FULL)U(0x706753fbcfd55094),
	    (FULL)U(0xf65a6713ede6e446), (FULL)U(0x8bf6dfae47c0d5c3),
	    (FULL)U(0xfb4dfc179f7927d6), (FULL)U(0x12ebbc16e212c297),
	    (FULL)U(0x43c71283a00a954c), (FULL)U(0x8957087ae7bd40a5),
	    (FULL)U(0xb0859d7108344837), (FULL)U(0xfbf4b9a3aeb313f5),
	    (FULL)U(0x5e66e5bece81823a), (FULL)U(0x09a11c6e58ad6da1),
	    (FULL)U(0xc76f4316c608054f), (FULL)U(0xb582136146084099),
	    (FULL)U(0x4210008f17a725ed), (FULL)U(0xe5ff8912d347c481)
	},
	{		/* shuffle table */
	    (FULL)U(0x69a2296cec8abd57), (FULL)U(0x867e186999a6df81),
	    (FULL)U(0xc05ab96bd849a48a), (FULL)U(0x7eb3ce0cfa00554b),
	    (FULL)U(0x520d01f65a5a9acd), (FULL)U(0xd4ef1e3336022d81),
	    (FULL)U(0xaf44772bc6f84f70), (FULL)U(0x647e85a6a7c55173),
	    (FULL)U(0x26746cf1959df8d1), (FULL)U(0x98681a904db28abd),
	    (FULL)U(0xb146c969744c5cd2), (FULL)U(0x8ce69d1f706f88c2),
	    (FULL)U(0xfd12eac421b4a748), (FULL)U(0xf12e70fe2710eea5),
	    (FULL)U(0x0b8f78055901f2b5), (FULL)U(0x48860a764f2c115e),
	    (FULL)U(0x0edf6d2a30767e2c), (FULL)U(0x8a6d7dc5fce2713b),
	    (FULL)U(0x46a362ea4e0e2346), (FULL)U(0x6c369a0a359f5aa7),
	    (FULL)U(0xdfca81fe41def54e), (FULL)U(0x4b73381996c2bc4e),
	    (FULL)U(0x659e8b996f3f14f9), (FULL)U(0x8b97b93493d47e6f),
	    (FULL)U(0xa73a8704dfa10a55), (FULL)U(0x8d9eafe9b06503da),
	    (FULL)U(0x2556fb88f32336b0), (FULL)U(0xe71e9f751002a161),
	    (FULL)U(0x27a7be6e200af907), (FULL)U(0x1b9b734ed028e9a3),
	    (FULL)U(0x950cfeed4c0be0d3), (FULL)U(0xf4c416942536d275),
	    (FULL)U(0xf05a58e85687b76e), (FULL)U(0xba53ac0171a62d54),
	    (FULL)U(0x4b14cbcb285adc96), (FULL)U(0xfdf66eddb00a5557),
	    (FULL)U(0xbb43d58d185b6ea1), (FULL)U(0x905db9cdf355c9a6),
	    (FULL)U(0xfc3a07fc04429c8a), (FULL)U(0x65d7e365aa3a4f7e),
	    (FULL)U(0x2d284c18b243ac65), (FULL)U(0x72fba65d44e417fd),
	    (FULL)U(0x422d50b45c934805), (FULL)U(0xb62a6053d1587441),
	    (FULL)U(0xa5e71ce96f7ae035), (FULL)U(0x93abca2e595c8dd8),
	    (FULL)U(0x534231afe39afad5), (FULL)U(0x08d26cac12eaad56),
	    (FULL)U(0xec18bf8d7fb1b1c2), (FULL)U(0x3d28ea16faf6f09b),
	    (FULL)U(0xea357a7816697dd6), (FULL)U(0x51471ea1420f3f51),
	    (FULL)U(0x5e051aeb7f8946b4), (FULL)U(0x881be0970cf0524c),
	    (FULL)U(0xd558b25b1b31489e), (FULL)U(0x707d1a943a8b065c),
	    (FULL)U(0x37017e66568ff836), (FULL)U(0xb9cd627c24c2f747),
	    (FULL)U(0x1485549ffb1d9ff6), (FULL)U(0x308d32d9bdf2dc6f),
	    (FULL)U(0x4d4142cad543818a), (FULL)U(0x5d9c7aee87ebba43),
	    (FULL)U(0x81c5bdd8e17adb2f), (FULL)U(0x3dc9752ec8d8677a),
	    (FULL)U(0x66b086e6c34e4212), (FULL)U(0x3af7a90dc62b25e3),
	    (FULL)U(0xf8349f7935539315), (FULL)U(0x6bcfd9d5a22917f0),
	    (FULL)U(0x8639bb765f5ee517), (FULL)U(0xd3c5e3698095b092),
	    (FULL)U(0x8a33851e7eb44748), (FULL)U(0x5e29d443ea54bbcf),
	    (FULL)U(0x0f84651f4d59a834), (FULL)U(0x85040beaf1a5f951),
	    (FULL)U(0x3dba1c7498002078), (FULL)U(0x5d70712bf0b2cc15),
	    (FULL)U(0xfa3af8ebcce8e5a7), (FULL)U(0xfb3e223704bba57d),
	    (FULL)U(0x5d3b87858a950434), (FULL)U(0xce3112bdba3f8dcf),
	    (FULL)U(0x44904f55860d3051), (FULL)U(0xcec8fed44ed3e98b),
	    (FULL)U(0x4581698d25d01ea4), (FULL)U(0x11eb68289a9548e0),
	    (FULL)U(0x796cb4c6e911fac8), (FULL)U(0x2164cf26b5fd813e),
	    (FULL)U(0x4ac8e0f5d5de640f), (FULL)U(0xe9e757d78802ab4e),
	    (FULL)U(0x3c97de26f49dfcbd), (FULL)U(0xc604881b6ee6dbe6),
	    (FULL)U(0xa7c22a6e57d6154e), (FULL)U(0x234e2370877b3cc7),
	    (FULL)U(0xc0bdb72bdf1f8358), (FULL)U(0x6522e0fca95b7b55),
	    (FULL)U(0xba174c9022344162), (FULL)U(0x712c9b2d75d48867),
	    (FULL)U(0x240f7e92e59f3700), (FULL)U(0xe83cc2d4ad95d763),
	    (FULL)U(0x8509445a4336d717), (FULL)U(0xf1e572c5dfff1804),
	    (FULL)U(0xed10eb5d623232dd), (FULL)U(0x9205ea1bd4f957e8),
	    (FULL)U(0x4973a54f2ff062f5), (FULL)U(0x26b018f1e8c48cd5),
	    (FULL)U(0x56908401d1c7ed9f), (FULL)U(0x2e48937bdf89a247),
	    (FULL)U(0x9d53069b2be47129), (FULL)U(0x98069e3bc048a2b0),
	    (FULL)U(0xf25b7d651cd83f93), (FULL)U(0x2b004e6ce6f886c8),
	    (FULL)U(0xf618442a5c635935), (FULL)U(0xa502ab5c7198e052),
	    (FULL)U(0xc14241a4a6c41b0b), (FULL)U(0x720e845a7db9b18e),
	    (FULL)U(0x2abb13e94b713918), (FULL)U(0x90fc0c207f52467d),
	    (FULL)U(0x799c8ccd7868d348), (FULL)U(0xf4817ced912a0ea4),
	    (FULL)U(0xd68c0f4cc4903a57), (FULL)U(0xa3171f29e2b7934c),
	    (FULL)U(0xb1158baa0b4ccc22), (FULL)U(0xf5d8555349a29eda),
	    (FULL)U(0x59d1a078959442ef), (FULL)U(0xdb9b4a96a67fd518),
	    (FULL)U(0xcc7ca9eed2870636), (FULL)U(0x548f021cecf59920),
	    (FULL)U(0x25b7f4b6571bc8c5), (FULL)U(0x4fa527473a44f536),
	    (FULL)U(0xb246845fdf0ebdc2), (FULL)U(0xdd8d68ae42058793),
	    (FULL)U(0x3ba133289f6c39fb), (FULL)U(0x8bfdfbf37b6b42af),
	    (FULL)U(0xfb34c5ca7fb2b3b0), (FULL)U(0x2345dcecd428e32a),
	    (FULL)U(0x6891e850ad42b63e), (FULL)U(0x930642c8362c1381),
	    (FULL)U(0x13871e9b1886aff5), (FULL)U(0xd0cf2407482bda55),
	    (FULL)U(0x125b5fc95069bc31), (FULL)U(0x9b71d0a9f07dfa5d),
	    (FULL)U(0x55c044cc6712e524), (FULL)U(0xf0377358bb601978),
	    (FULL)U(0x152ad5f87fa51e8b), (FULL)U(0xe5ebf4789fcdd9af),
	    (FULL)U(0x3d78e18c66ebce7e), (FULL)U(0x8246db72f36aa83f),
	    (FULL)U(0xcc6ddc6d2c64c0a3), (FULL)U(0xa758d6870d91851e),
	    (FULL)U(0x24b20a6f9488ee36), (FULL)U(0xbe11ccdf09798197),
	    (FULL)U(0x11aca01599c1f4e3), (FULL)U(0x40e89e366437ac05),
	    (FULL)U(0xc8bfc7625af675f8), (FULL)U(0x6367c578b577e759),
	    (FULL)U(0x00380346615f0b74), (FULL)U(0xee964cc48de07d81),
	    (FULL)U(0x17f6ac16859d9261), (FULL)U(0x092f4a173a6e2f6c),
	    (FULL)U(0x79981a3db9024b95), (FULL)U(0x36db166004f7f540),
	    (FULL)U(0xc36252cf65a2f1c8), (FULL)U(0x705b6fde124c9bd2),
	    (FULL)U(0x31e58dda85db40ce), (FULL)U(0x6342b1a59f5e8d6d),
	    (FULL)U(0x5c2c67d0bd6d1d4d), (FULL)U(0x1fe5b46fba7e069d),
	    (FULL)U(0x21c46c6cac72e13c), (FULL)U(0xb80c5fd59eb8f52a),
	    (FULL)U(0x56c3aebfa74c92bc), (FULL)U(0xc1aff1fcbf8c4196),
	    (FULL)U(0x2b1df645754ad208), (FULL)U(0x5c734600d46eeb50),
	    (FULL)U(0xe0ff1b126a70a765), (FULL)U(0xd54164977a94547c),
	    (FULL)U(0x67b59d7c4ea35206), (FULL)U(0x53be7146779203b4),
	    (FULL)U(0x6b589fe5414026b8), (FULL)U(0x9e81016c3083bfee),
	    (FULL)U(0xb23526b93b4b7671), (FULL)U(0x4fa9ffb17ee300ba),
	    (FULL)U(0x6217e212ad05fb21), (FULL)U(0xf5b3fcd3b294e6c2),
	    (FULL)U(0xac040bbe216beb2a), (FULL)U(0x1f8d8a5471d0e78c),
	    (FULL)U(0xb6d15b419cfec96b), (FULL)U(0xc5477845d0508c78),
	    (FULL)U(0x5b486e81b4bba621), (FULL)U(0x90c35c94ef4c4121),
	    (FULL)U(0xefce7346f6a6bc55), (FULL)U(0xa27828d925bdb9bb),
	    (FULL)U(0xe3a53095a1f0b205), (FULL)U(0x1bfa6093d9f208ab),
	    (FULL)U(0xfb078f6a6842cdf4), (FULL)U(0x07806d7297133a38),
	    (FULL)U(0x2c6c901ba3ce9592), (FULL)U(0x1f0ab2cfebc1b789),
	    (FULL)U(0x2ce81415e2d03d5e), (FULL)U(0x7da45d5baa9f2417),
	    (FULL)U(0x3be4f76ddd800682), (FULL)U(0xdbf4e4a3364d72d3),
	    (FULL)U(0xb538cccf4fc59da5), (FULL)U(0xb0aa39d5487f66ec),
	    (FULL)U(0x2fd28dfd87927d3d), (FULL)U(0xd14e77f05900c6b1),
	    (FULL)U(0x2523fad25330c7b4), (FULL)U(0x991b5938d82368a4),
	    (FULL)U(0xb7c114432b9c1302), (FULL)U(0xdb842db61394b116),
	    (FULL)U(0x3641548d78ed26d8), (FULL)U(0x274fa8ef0a61dacf),
	    (FULL)U(0xa554ba63112df6f1), (FULL)U(0x7b7fe9856b50438d),
	    (FULL)U(0xc9fa0042bb63bbad), (FULL)U(0x3abf45d0e27f00da),
	    (FULL)U(0xd95faa159f87aabb), (FULL)U(0x4a95012e3488e7ae),
	    (FULL)U(0x1be2bdb90c642d04), (FULL)U(0x145c88818b4abf3e),
	    (FULL)U(0x7f9fb635544cf17f), (FULL)U(0xb8ab2f62cc78db70),
	    (FULL)U(0x8ee64bcdb4242f9a), (FULL)U(0xabd5285895dad129),
	    (FULL)U(0xbe722c2fccf31141), (FULL)U(0x7c330703575e26a9),
	    (FULL)U(0x45d3e3b3361b79e4), (FULL)U(0x241163a754b2e6a6),
	    (FULL)U(0x8f678d7df7cacb77), (FULL)U(0x988a68a483211d19),
	    (FULL)U(0x79599598ba7836f6), (FULL)U(0x4850c887eeda68bf),
	    (FULL)U(0xafa69a718052ce25), (FULL)U(0x8b21efc6bdd73573),
	    (FULL)U(0x89dbae18d0972493), (FULL)U(0x560776bf537d9454),
	    (FULL)U(0x3c009f78165310f2), (FULL)U(0xa36800210160c3af),
	    (FULL)U(0x3353ec3ca643bd40), (FULL)U(0x7e593f99911dab02),
	    (FULL)U(0x72d1ddd94f676e89), (FULL)U(0xfd18b8bd6b43c0ea),
	    (FULL)U(0x43cacef2ddbd697d), (FULL)U(0x2868a4d0acefe884),
	    (FULL)U(0x5f377b63a506f013), (FULL)U(0xeaa0975e05ca662b),
	    (FULL)U(0x3740e6b8eb433931), (FULL)U(0xce85df0008557948),
	    (FULL)U(0x784745fb547e33f9), (FULL)U(0x4a1fc5d4e5c6f598),
	    (FULL)U(0x85fa6fec768430a7), (FULL)U(0x990d0c24d2332a51),
	    (FULL)U(0x55245c2c33b676d5), (FULL)U(0xb1091519e2bcfa71),
	    (FULL)U(0x38521478d23a28d8), (FULL)U(0x9b794f899a573010),
	    (FULL)U(0x61d225e8699bb486), (FULL)U(0x21476d241c2158b0)
	}
#elif 2*FULL_BITS == SBITS
	{		/* subtractive 100 table */
	    (FULL)0x7db7dc19,(FULL)0xc8c0370c,(FULL)0x40a06fbb,(FULL)0x738e33b9,
	    (FULL)0xa859ed2b,(FULL)0x481abb76,(FULL)0x9ccdccb5,(FULL)0x74106bb3,
	    (FULL)0xc3173bfc,(FULL)0x05a8eeb5,(FULL)0x5a02e577,(FULL)0xefd5100d,
	    (FULL)0x4030b24a,(FULL)0xa69271f7,(FULL)0x16fe22c5,(FULL)0x641282fc,
	    (FULL)0x40438da3,(FULL)0x7aa7267c,(FULL)0xc2d878d1,(FULL)0x1fdf4abd,
	    (FULL)0x95702379,(FULL)0xd9899e7a,(FULL)0xd02d7f08,(FULL)0x5ea8e217,
	    (FULL)0x4d47a353,(FULL)0x770587fe,(FULL)0x0a33a2b8,(FULL)0xde7d1bdd,
	    (FULL)0x900e7c45,(FULL)0x4378c3c5,(FULL)0x19a514f9,(FULL)0x77c94478,
	    (FULL)0x843d1d32,(FULL)0xfc5edb22,(FULL)0xe8ee5e6e,(FULL)0x4fc42ce5,
	    (FULL)0x8488013e,(FULL)0xc938713c,(FULL)0x20ab0cac,(FULL)0x6a318f03,
	    (FULL)0xffc8bff3,(FULL)0x73e6d1a3,(FULL)0x8ca96aa7,(FULL)0x0cd3232a,
	    (FULL)0x905f770d,(FULL)0x605c8036,(FULL)0x8b8d04a2,(FULL)0x4d037b00,
	    (FULL)0xcb277294,(FULL)0x1ed81965,(FULL)0x7a254ff3,(FULL)0x408d9c47,
	    (FULL)0xe26c7377,(FULL)0x8b68587a,(FULL)0x8a48832f,(FULL)0xcff191a4,
	    (FULL)0x8aeb6fe6,(FULL)0x12d3df1d,(FULL)0x1feda37a,(FULL)0xb2bf907e,
	    (FULL)0x3bb5f39f,(FULL)0x4e5f7719,(FULL)0x8f5d1581,(FULL)0x33ebcf6f,
	    (FULL)0xd33654eb,(FULL)0x203c8e48,(FULL)0xf19c8a4e,(FULL)0x68d3656e,
	    (FULL)0x986eb2af,(FULL)0x3ec20b04,(FULL)0x062c3841,(FULL)0x5d73a03b,
	    (FULL)0x5d4e49eb,(FULL)0x836ce709,(FULL)0xc3f49221,(FULL)0x2310bc40,
	    (FULL)0xa6d0cbf6,(FULL)0x3868ee48,(FULL)0x4a43deb1,(FULL)0x67578aa6,
	    (FULL)0x150dfc26,(FULL)0x6e3426c1,(FULL)0x3131be30,(FULL)0xc541ccaa,
	    (FULL)0xcec7aab2,(FULL)0xf7e57432,(FULL)0x8cb3c873,(FULL)0x2b35de99,
	    (FULL)0x8663a5d7,(FULL)0x7b9f7764,(FULL)0xa771e5a6,(FULL)0x23b00e6a,
	    (FULL)0xa9985d05,(FULL)0x859c775c,(FULL)0x6b692f1f,(FULL)0x99636ea1,
	    (FULL)0x3730800d,(FULL)0x8700ac70,(FULL)0x4298a753,(FULL)0x46142502,
	    (FULL)0x809e955f,(FULL)0xea4a411b,(FULL)0x33709dfb,(FULL)0x3119ad40,
	    (FULL)0x5f01cb7c,(FULL)0xb76a6c6e,(FULL)0x15984eaf,(FULL)0x6109dc8a,
	    (FULL)0xa5ca9505,(FULL)0x5d686db9,(FULL)0x3b7e6add,(FULL)0x8e80d761,
	    (FULL)0xde6f6fd3,(FULL)0x79cbd718,(FULL)0x1da0f699,(FULL)0x40e9cd15,
	    (FULL)0xb24f312d,(FULL)0xe82158ba,(FULL)0xf5e5c36b,(FULL)0x79a4c927,
	    (FULL)0xa0039333,(FULL)0xc25247c9,(FULL)0x1766d81d,(FULL)0x93687116,
	    (FULL)0xa6741327,(FULL)0x3c6a03b4,(FULL)0xc002f29a,(FULL)0xc8a7b6e8,
	    (FULL)0x7bbd5ea3,(FULL)0x0e2a67c6,(FULL)0x441eabc1,(FULL)0x0929042d,
	    (FULL)0x25e82085,(FULL)0x7dbe232a,(FULL)0x44fbac3d,(FULL)0x8cfb26e5,
	    (FULL)0x388ab983,(FULL)0x8e40384d,(FULL)0x554632f8,(FULL)0x48dc1230,
	    (FULL)0xab492397,(FULL)0xab405048,(FULL)0xa118e387,(FULL)0x21c9e2f5,
	    (FULL)0x343b61b5,(FULL)0x484d1a8c,(FULL)0xab256f26,(FULL)0xd49e3dec,
	    (FULL)0x78f2d2e3,(FULL)0xe615c7fd,(FULL)0xce6cc2ed,(FULL)0x8442cc33,
	    (FULL)0x44d4bbf6,(FULL)0x0a3b93d8,(FULL)0x9301de77,(FULL)0x2d7e4efe,
	    (FULL)0xd8790d8a,(FULL)0x33711b76,(FULL)0x44df77e7,(FULL)0xc07dc30e,
	    (FULL)0x9ddd508f,(FULL)0xb9132ed0,(FULL)0xc6fb43cc,(FULL)0x45d06cf8,
	    (FULL)0xd585dd7b,(FULL)0x22bed18a,(FULL)0x10799ffa,(FULL)0x61c6cced,
	    (FULL)0xe4bd9aa9,(FULL)0xd7f2393b,(FULL)0xcfd55094,(FULL)0x706753fb,
	    (FULL)0xede6e446,(FULL)0xf65a6713,(FULL)0x47c0d5c3,(FULL)0x8bf6dfae,
	    (FULL)0x9f7927d6,(FULL)0xfb4dfc17,(FULL)0xe212c297,(FULL)0x12ebbc16,
	    (FULL)0xa00a954c,(FULL)0x43c71283,(FULL)0xe7bd40a5,(FULL)0x8957087a,
	    (FULL)0x08344837,(FULL)0xb0859d71,(FULL)0xaeb313f5,(FULL)0xfbf4b9a3,
	    (FULL)0xce81823a,(FULL)0x5e66e5be,(FULL)0x58ad6da1,(FULL)0x09a11c6e,
	    (FULL)0xc608054f,(FULL)0xc76f4316,(FULL)0x46084099,(FULL)0xb5821361,
	    (FULL)0x17a725ed,(FULL)0x4210008f,(FULL)0xd347c481,(FULL)0xe5ff8912
	},
	{		/* shuffle table */
	    (FULL)0xec8abd57,(FULL)0x69a2296c,(FULL)0x99a6df81,(FULL)0x867e1869,
	    (FULL)0xd849a48a,(FULL)0xc05ab96b,(FULL)0xfa00554b,(FULL)0x7eb3ce0c,
	    (FULL)0x5a5a9acd,(FULL)0x520d01f6,(FULL)0x36022d81,(FULL)0xd4ef1e33,
	    (FULL)0xc6f84f70,(FULL)0xaf44772b,(FULL)0xa7c55173,(FULL)0x647e85a6,
	    (FULL)0x959df8d1,(FULL)0x26746cf1,(FULL)0x4db28abd,(FULL)0x98681a90,
	    (FULL)0x744c5cd2,(FULL)0xb146c969,(FULL)0x706f88c2,(FULL)0x8ce69d1f,
	    (FULL)0x21b4a748,(FULL)0xfd12eac4,(FULL)0x2710eea5,(FULL)0xf12e70fe,
	    (FULL)0x5901f2b5,(FULL)0x0b8f7805,(FULL)0x4f2c115e,(FULL)0x48860a76,
	    (FULL)0x30767e2c,(FULL)0x0edf6d2a,(FULL)0xfce2713b,(FULL)0x8a6d7dc5,
	    (FULL)0x4e0e2346,(FULL)0x46a362ea,(FULL)0x359f5aa7,(FULL)0x6c369a0a,
	    (FULL)0x41def54e,(FULL)0xdfca81fe,(FULL)0x96c2bc4e,(FULL)0x4b733819,
	    (FULL)0x6f3f14f9,(FULL)0x659e8b99,(FULL)0x93d47e6f,(FULL)0x8b97b934,
	    (FULL)0xdfa10a55,(FULL)0xa73a8704,(FULL)0xb06503da,(FULL)0x8d9eafe9,
	    (FULL)0xf32336b0,(FULL)0x2556fb88,(FULL)0x1002a161,(FULL)0xe71e9f75,
	    (FULL)0x200af907,(FULL)0x27a7be6e,(FULL)0xd028e9a3,(FULL)0x1b9b734e,
	    (FULL)0x4c0be0d3,(FULL)0x950cfeed,(FULL)0x2536d275,(FULL)0xf4c41694,
	    (FULL)0x5687b76e,(FULL)0xf05a58e8,(FULL)0x71a62d54,(FULL)0xba53ac01,
	    (FULL)0x285adc96,(FULL)0x4b14cbcb,(FULL)0xb00a5557,(FULL)0xfdf66edd,
	    (FULL)0x185b6ea1,(FULL)0xbb43d58d,(FULL)0xf355c9a6,(FULL)0x905db9cd,
	    (FULL)0x04429c8a,(FULL)0xfc3a07fc,(FULL)0xaa3a4f7e,(FULL)0x65d7e365,
	    (FULL)0xb243ac65,(FULL)0x2d284c18,(FULL)0x44e417fd,(FULL)0x72fba65d,
	    (FULL)0x5c934805,(FULL)0x422d50b4,(FULL)0xd1587441,(FULL)0xb62a6053,
	    (FULL)0x6f7ae035,(FULL)0xa5e71ce9,(FULL)0x595c8dd8,(FULL)0x93abca2e,
	    (FULL)0xe39afad5,(FULL)0x534231af,(FULL)0x12eaad56,(FULL)0x08d26cac,
	    (FULL)0x7fb1b1c2,(FULL)0xec18bf8d,(FULL)0xfaf6f09b,(FULL)0x3d28ea16,
	    (FULL)0x16697dd6,(FULL)0xea357a78,(FULL)0x420f3f51,(FULL)0x51471ea1,
	    (FULL)0x7f8946b4,(FULL)0x5e051aeb,(FULL)0x0cf0524c,(FULL)0x881be097,
	    (FULL)0x1b31489e,(FULL)0xd558b25b,(FULL)0x3a8b065c,(FULL)0x707d1a94,
	    (FULL)0x568ff836,(FULL)0x37017e66,(FULL)0x24c2f747,(FULL)0xb9cd627c,
	    (FULL)0xfb1d9ff6,(FULL)0x1485549f,(FULL)0xbdf2dc6f,(FULL)0x308d32d9,
	    (FULL)0xd543818a,(FULL)0x4d4142ca,(FULL)0x87ebba43,(FULL)0x5d9c7aee,
	    (FULL)0xe17adb2f,(FULL)0x81c5bdd8,(FULL)0xc8d8677a,(FULL)0x3dc9752e,
	    (FULL)0xc34e4212,(FULL)0x66b086e6,(FULL)0xc62b25e3,(FULL)0x3af7a90d,
	    (FULL)0x35539315,(FULL)0xf8349f79,(FULL)0xa22917f0,(FULL)0x6bcfd9d5,
	    (FULL)0x5f5ee517,(FULL)0x8639bb76,(FULL)0x8095b092,(FULL)0xd3c5e369,
	    (FULL)0x7eb44748,(FULL)0x8a33851e,(FULL)0xea54bbcf,(FULL)0x5e29d443,
	    (FULL)0x4d59a834,(FULL)0x0f84651f,(FULL)0xf1a5f951,(FULL)0x85040bea,
	    (FULL)0x98002078,(FULL)0x3dba1c74,(FULL)0xf0b2cc15,(FULL)0x5d70712b,
	    (FULL)0xcce8e5a7,(FULL)0xfa3af8eb,(FULL)0x04bba57d,(FULL)0xfb3e2237,
	    (FULL)0x8a950434,(FULL)0x5d3b8785,(FULL)0xba3f8dcf,(FULL)0xce3112bd,
	    (FULL)0x860d3051,(FULL)0x44904f55,(FULL)0x4ed3e98b,(FULL)0xcec8fed4,
	    (FULL)0x25d01ea4,(FULL)0x4581698d,(FULL)0x9a9548e0,(FULL)0x11eb6828,
	    (FULL)0xe911fac8,(FULL)0x796cb4c6,(FULL)0xb5fd813e,(FULL)0x2164cf26,
	    (FULL)0xd5de640f,(FULL)0x4ac8e0f5,(FULL)0x8802ab4e,(FULL)0xe9e757d7,
	    (FULL)0xf49dfcbd,(FULL)0x3c97de26,(FULL)0x6ee6dbe6,(FULL)0xc604881b,
	    (FULL)0x57d6154e,(FULL)0xa7c22a6e,(FULL)0x877b3cc7,(FULL)0x234e2370,
	    (FULL)0xdf1f8358,(FULL)0xc0bdb72b,(FULL)0xa95b7b55,(FULL)0x6522e0fc,
	    (FULL)0x22344162,(FULL)0xba174c90,(FULL)0x75d48867,(FULL)0x712c9b2d,
	    (FULL)0xe59f3700,(FULL)0x240f7e92,(FULL)0xad95d763,(FULL)0xe83cc2d4,
	    (FULL)0x4336d717,(FULL)0x8509445a,(FULL)0xdfff1804,(FULL)0xf1e572c5,
	    (FULL)0x623232dd,(FULL)0xed10eb5d,(FULL)0xd4f957e8,(FULL)0x9205ea1b,
	    (FULL)0x2ff062f5,(FULL)0x4973a54f,(FULL)0xe8c48cd5,(FULL)0x26b018f1,
	    (FULL)0xd1c7ed9f,(FULL)0x56908401,(FULL)0xdf89a247,(FULL)0x2e48937b,
	    (FULL)0x2be47129,(FULL)0x9d53069b,(FULL)0xc048a2b0,(FULL)0x98069e3b,
	    (FULL)0x1cd83f93,(FULL)0xf25b7d65,(FULL)0xe6f886c8,(FULL)0x2b004e6c,
	    (FULL)0x5c635935,(FULL)0xf618442a,(FULL)0x7198e052,(FULL)0xa502ab5c,
	    (FULL)0xa6c41b0b,(FULL)0xc14241a4,(FULL)0x7db9b18e,(FULL)0x720e845a,
	    (FULL)0x4b713918,(FULL)0x2abb13e9,(FULL)0x7f52467d,(FULL)0x90fc0c20,
	    (FULL)0x7868d348,(FULL)0x799c8ccd,(FULL)0x912a0ea4,(FULL)0xf4817ced,
	    (FULL)0xc4903a57,(FULL)0xd68c0f4c,(FULL)0xe2b7934c,(FULL)0xa3171f29,
	    (FULL)0x0b4ccc22,(FULL)0xb1158baa,(FULL)0x49a29eda,(FULL)0xf5d85553,
	    (FULL)0x959442ef,(FULL)0x59d1a078,(FULL)0xa67fd518,(FULL)0xdb9b4a96,
	    (FULL)0xd2870636,(FULL)0xcc7ca9ee,(FULL)0xecf59920,(FULL)0x548f021c,
	    (FULL)0x571bc8c5,(FULL)0x25b7f4b6,(FULL)0x3a44f536,(FULL)0x4fa52747,
	    (FULL)0xdf0ebdc2,(FULL)0xb246845f,(FULL)0x42058793,(FULL)0xdd8d68ae,
	    (FULL)0x9f6c39fb,(FULL)0x3ba13328,(FULL)0x7b6b42af,(FULL)0x8bfdfbf3,
	    (FULL)0x7fb2b3b0,(FULL)0xfb34c5ca,(FULL)0xd428e32a,(FULL)0x2345dcec,
	    (FULL)0xad42b63e,(FULL)0x6891e850,(FULL)0x362c1381,(FULL)0x930642c8,
	    (FULL)0x1886aff5,(FULL)0x13871e9b,(FULL)0x482bda55,(FULL)0xd0cf2407,
	    (FULL)0x5069bc31,(FULL)0x125b5fc9,(FULL)0xf07dfa5d,(FULL)0x9b71d0a9,
	    (FULL)0x6712e524,(FULL)0x55c044cc,(FULL)0xbb601978,(FULL)0xf0377358,
	    (FULL)0x7fa51e8b,(FULL)0x152ad5f8,(FULL)0x9fcdd9af,(FULL)0xe5ebf478,
	    (FULL)0x66ebce7e,(FULL)0x3d78e18c,(FULL)0xf36aa83f,(FULL)0x8246db72,
	    (FULL)0x2c64c0a3,(FULL)0xcc6ddc6d,(FULL)0x0d91851e,(FULL)0xa758d687,
	    (FULL)0x9488ee36,(FULL)0x24b20a6f,(FULL)0x09798197,(FULL)0xbe11ccdf,
	    (FULL)0x99c1f4e3,(FULL)0x11aca015,(FULL)0x6437ac05,(FULL)0x40e89e36,
	    (FULL)0x5af675f8,(FULL)0xc8bfc762,(FULL)0xb577e759,(FULL)0x6367c578,
	    (FULL)0x615f0b74,(FULL)0x00380346,(FULL)0x8de07d81,(FULL)0xee964cc4,
	    (FULL)0x859d9261,(FULL)0x17f6ac16,(FULL)0x3a6e2f6c,(FULL)0x092f4a17,
	    (FULL)0xb9024b95,(FULL)0x79981a3d,(FULL)0x04f7f540,(FULL)0x36db1660,
	    (FULL)0x65a2f1c8,(FULL)0xc36252cf,(FULL)0x124c9bd2,(FULL)0x705b6fde,
	    (FULL)0x85db40ce,(FULL)0x31e58dda,(FULL)0x9f5e8d6d,(FULL)0x6342b1a5,
	    (FULL)0xbd6d1d4d,(FULL)0x5c2c67d0,(FULL)0xba7e069d,(FULL)0x1fe5b46f,
	    (FULL)0xac72e13c,(FULL)0x21c46c6c,(FULL)0x9eb8f52a,(FULL)0xb80c5fd5,
	    (FULL)0xa74c92bc,(FULL)0x56c3aebf,(FULL)0xbf8c4196,(FULL)0xc1aff1fc,
	    (FULL)0x754ad208,(FULL)0x2b1df645,(FULL)0xd46eeb50,(FULL)0x5c734600,
	    (FULL)0x6a70a765,(FULL)0xe0ff1b12,(FULL)0x7a94547c,(FULL)0xd5416497,
	    (FULL)0x4ea35206,(FULL)0x67b59d7c,(FULL)0x779203b4,(FULL)0x53be7146,
	    (FULL)0x414026b8,(FULL)0x6b589fe5,(FULL)0x3083bfee,(FULL)0x9e81016c,
	    (FULL)0x3b4b7671,(FULL)0xb23526b9,(FULL)0x7ee300ba,(FULL)0x4fa9ffb1,
	    (FULL)0xad05fb21,(FULL)0x6217e212,(FULL)0xb294e6c2,(FULL)0xf5b3fcd3,
	    (FULL)0x216beb2a,(FULL)0xac040bbe,(FULL)0x71d0e78c,(FULL)0x1f8d8a54,
	    (FULL)0x9cfec96b,(FULL)0xb6d15b41,(FULL)0xd0508c78,(FULL)0xc5477845,
	    (FULL)0xb4bba621,(FULL)0x5b486e81,(FULL)0xef4c4121,(FULL)0x90c35c94,
	    (FULL)0xf6a6bc55,(FULL)0xefce7346,(FULL)0x25bdb9bb,(FULL)0xa27828d9,
	    (FULL)0xa1f0b205,(FULL)0xe3a53095,(FULL)0xd9f208ab,(FULL)0x1bfa6093,
	    (FULL)0x6842cdf4,(FULL)0xfb078f6a,(FULL)0x97133a38,(FULL)0x07806d72,
	    (FULL)0xa3ce9592,(FULL)0x2c6c901b,(FULL)0xebc1b789,(FULL)0x1f0ab2cf,
	    (FULL)0xe2d03d5e,(FULL)0x2ce81415,(FULL)0xaa9f2417,(FULL)0x7da45d5b,
	    (FULL)0xdd800682,(FULL)0x3be4f76d,(FULL)0x364d72d3,(FULL)0xdbf4e4a3,
	    (FULL)0x4fc59da5,(FULL)0xb538cccf,(FULL)0x487f66ec,(FULL)0xb0aa39d5,
	    (FULL)0x87927d3d,(FULL)0x2fd28dfd,(FULL)0x5900c6b1,(FULL)0xd14e77f0,
	    (FULL)0x5330c7b4,(FULL)0x2523fad2,(FULL)0xd82368a4,(FULL)0x991b5938,
	    (FULL)0x2b9c1302,(FULL)0xb7c11443,(FULL)0x1394b116,(FULL)0xdb842db6,
	    (FULL)0x78ed26d8,(FULL)0x3641548d,(FULL)0x0a61dacf,(FULL)0x274fa8ef,
	    (FULL)0x112df6f1,(FULL)0xa554ba63,(FULL)0x6b50438d,(FULL)0x7b7fe985,
	    (FULL)0xbb63bbad,(FULL)0xc9fa0042,(FULL)0xe27f00da,(FULL)0x3abf45d0,
	    (FULL)0x9f87aabb,(FULL)0xd95faa15,(FULL)0x3488e7ae,(FULL)0x4a95012e,
	    (FULL)0x0c642d04,(FULL)0x1be2bdb9,(FULL)0x8b4abf3e,(FULL)0x145c8881,
	    (FULL)0x544cf17f,(FULL)0x7f9fb635,(FULL)0xcc78db70,(FULL)0xb8ab2f62,
	    (FULL)0xb4242f9a,(FULL)0x8ee64bcd,(FULL)0x95dad129,(FULL)0xabd52858,
	    (FULL)0xccf31141,(FULL)0xbe722c2f,(FULL)0x575e26a9,(FULL)0x7c330703,
	    (FULL)0x361b79e4,(FULL)0x45d3e3b3,(FULL)0x54b2e6a6,(FULL)0x241163a7,
	    (FULL)0xf7cacb77,(FULL)0x8f678d7d,(FULL)0x83211d19,(FULL)0x988a68a4,
	    (FULL)0xba7836f6,(FULL)0x79599598,(FULL)0xeeda68bf,(FULL)0x4850c887,
	    (FULL)0x8052ce25,(FULL)0xafa69a71,(FULL)0xbdd73573,(FULL)0x8b21efc6,
	    (FULL)0xd0972493,(FULL)0x89dbae18,(FULL)0x537d9454,(FULL)0x560776bf,
	    (FULL)0x165310f2,(FULL)0x3c009f78,(FULL)0x0160c3af,(FULL)0xa3680021,
	    (FULL)0xa643bd40,(FULL)0x3353ec3c,(FULL)0x911dab02,(FULL)0x7e593f99,
	    (FULL)0x4f676e89,(FULL)0x72d1ddd9,(FULL)0x6b43c0ea,(FULL)0xfd18b8bd,
	    (FULL)0xddbd697d,(FULL)0x43cacef2,(FULL)0xacefe884,(FULL)0x2868a4d0,
	    (FULL)0xa506f013,(FULL)0x5f377b63,(FULL)0x05ca662b,(FULL)0xeaa0975e,
	    (FULL)0xeb433931,(FULL)0x3740e6b8,(FULL)0x08557948,(FULL)0xce85df00,
	    (FULL)0x547e33f9,(FULL)0x784745fb,(FULL)0xe5c6f598,(FULL)0x4a1fc5d4,
	    (FULL)0x768430a7,(FULL)0x85fa6fec,(FULL)0xd2332a51,(FULL)0x990d0c24,
	    (FULL)0x33b676d5,(FULL)0x55245c2c,(FULL)0xe2bcfa71,(FULL)0xb1091519,
	    (FULL)0xd23a28d8,(FULL)0x38521478,(FULL)0x9a573010,(FULL)0x9b794f89,
	    (FULL)0x699bb486,(FULL)0x61d225e8,(FULL)0x1c2158b0,(FULL)0x21476d24
	}
#else
   /\../\	FULL_BITS must be 32 or 64	/\../\	 !!!
#endif
};

/*
 * default subtractive 100 table
 *
 * The subtractive 100 table in init_s100 has been processed 256 times in order
 * to preload the shuffle table.  The array below is the table before
 * this processing.  These values have came from LavaRnd.
 *
 * This array is never changed, only copied.
 */
STATIC CONST FULL def_subtract[SCNT] = {
#if FULL_BITS == SBITS
	    (FULL)U(0xc8c0370c7db7dc19), (FULL)U(0x738e33b940a06fbb),
	    (FULL)U(0x481abb76a859ed2b), (FULL)U(0x74106bb39ccdccb5),
	    (FULL)U(0x05a8eeb5c3173bfc), (FULL)U(0xefd5100d5a02e577),
	    (FULL)U(0xa69271f74030b24a), (FULL)U(0x641282fc16fe22c5),
	    (FULL)U(0x7aa7267c40438da3), (FULL)U(0x1fdf4abdc2d878d1),
	    (FULL)U(0xd9899e7a95702379), (FULL)U(0x5ea8e217d02d7f08),
	    (FULL)U(0x770587fe4d47a353), (FULL)U(0xde7d1bdd0a33a2b8),
	    (FULL)U(0x4378c3c5900e7c45), (FULL)U(0x77c9447819a514f9),
	    (FULL)U(0xfc5edb22843d1d32), (FULL)U(0x4fc42ce5e8ee5e6e),
	    (FULL)U(0xc938713c8488013e), (FULL)U(0x6a318f0320ab0cac),
	    (FULL)U(0x73e6d1a3ffc8bff3), (FULL)U(0x0cd3232a8ca96aa7),
	    (FULL)U(0x605c8036905f770d), (FULL)U(0x4d037b008b8d04a2),
	    (FULL)U(0x1ed81965cb277294), (FULL)U(0x408d9c477a254ff3),
	    (FULL)U(0x8b68587ae26c7377), (FULL)U(0xcff191a48a48832f),
	    (FULL)U(0x12d3df1d8aeb6fe6), (FULL)U(0xb2bf907e1feda37a),
	    (FULL)U(0x4e5f77193bb5f39f), (FULL)U(0x33ebcf6f8f5d1581),
	    (FULL)U(0x203c8e48d33654eb), (FULL)U(0x68d3656ef19c8a4e),
	    (FULL)U(0x3ec20b04986eb2af), (FULL)U(0x5d73a03b062c3841),
	    (FULL)U(0x836ce7095d4e49eb), (FULL)U(0x2310bc40c3f49221),
	    (FULL)U(0x3868ee48a6d0cbf6), (FULL)U(0x67578aa64a43deb1),
	    (FULL)U(0x6e3426c1150dfc26), (FULL)U(0xc541ccaa3131be30),
	    (FULL)U(0xf7e57432cec7aab2), (FULL)U(0x2b35de998cb3c873),
	    (FULL)U(0x7b9f77648663a5d7), (FULL)U(0x23b00e6aa771e5a6),
	    (FULL)U(0x859c775ca9985d05), (FULL)U(0x99636ea16b692f1f),
	    (FULL)U(0x8700ac703730800d), (FULL)U(0x461425024298a753),
	    (FULL)U(0xea4a411b809e955f), (FULL)U(0x3119ad4033709dfb),
	    (FULL)U(0xb76a6c6e5f01cb7c), (FULL)U(0x6109dc8a15984eaf),
	    (FULL)U(0x5d686db9a5ca9505), (FULL)U(0x8e80d7613b7e6add),
	    (FULL)U(0x79cbd718de6f6fd3), (FULL)U(0x40e9cd151da0f699),
	    (FULL)U(0xe82158bab24f312d), (FULL)U(0x79a4c927f5e5c36b),
	    (FULL)U(0xc25247c9a0039333), (FULL)U(0x936871161766d81d),
	    (FULL)U(0x3c6a03b4a6741327), (FULL)U(0xc8a7b6e8c002f29a),
	    (FULL)U(0x0e2a67c67bbd5ea3), (FULL)U(0x0929042d441eabc1),
	    (FULL)U(0x7dbe232a25e82085), (FULL)U(0x8cfb26e544fbac3d),
	    (FULL)U(0x8e40384d388ab983), (FULL)U(0x48dc1230554632f8),
	    (FULL)U(0xab405048ab492397), (FULL)U(0x21c9e2f5a118e387),
	    (FULL)U(0x484d1a8c343b61b5), (FULL)U(0xd49e3decab256f26),
	    (FULL)U(0xe615c7fd78f2d2e3), (FULL)U(0x8442cc33ce6cc2ed),
	    (FULL)U(0x0a3b93d844d4bbf6), (FULL)U(0x2d7e4efe9301de77),
	    (FULL)U(0x33711b76d8790d8a), (FULL)U(0xc07dc30e44df77e7),
	    (FULL)U(0xb9132ed09ddd508f), (FULL)U(0x45d06cf8c6fb43cc),
	    (FULL)U(0x22bed18ad585dd7b), (FULL)U(0x61c6cced10799ffa),
	    (FULL)U(0xd7f2393be4bd9aa9), (FULL)U(0x706753fbcfd55094),
	    (FULL)U(0xf65a6713ede6e446), (FULL)U(0x8bf6dfae47c0d5c3),
	    (FULL)U(0xfb4dfc179f7927d6), (FULL)U(0x12ebbc16e212c297),
	    (FULL)U(0x43c71283a00a954c), (FULL)U(0x8957087ae7bd40a5),
	    (FULL)U(0xb0859d7108344837), (FULL)U(0xfbf4b9a3aeb313f5),
	    (FULL)U(0x5e66e5bece81823a), (FULL)U(0x09a11c6e58ad6da1),
	    (FULL)U(0xc76f4316c608054f), (FULL)U(0xb582136146084099),
	    (FULL)U(0x4210008f17a725ed), (FULL)U(0xe5ff8912d347c481)
#elif 2*FULL_BITS == SBITS
	    (FULL)0x7db7dc19,(FULL)0xc8c0370c,(FULL)0x40a06fbb,(FULL)0x738e33b9,
	    (FULL)0xa859ed2b,(FULL)0x481abb76,(FULL)0x9ccdccb5,(FULL)0x74106bb3,
	    (FULL)0xc3173bfc,(FULL)0x05a8eeb5,(FULL)0x5a02e577,(FULL)0xefd5100d,
	    (FULL)0x4030b24a,(FULL)0xa69271f7,(FULL)0x16fe22c5,(FULL)0x641282fc,
	    (FULL)0x40438da3,(FULL)0x7aa7267c,(FULL)0xc2d878d1,(FULL)0x1fdf4abd,
	    (FULL)0x95702379,(FULL)0xd9899e7a,(FULL)0xd02d7f08,(FULL)0x5ea8e217,
	    (FULL)0x4d47a353,(FULL)0x770587fe,(FULL)0x0a33a2b8,(FULL)0xde7d1bdd,
	    (FULL)0x900e7c45,(FULL)0x4378c3c5,(FULL)0x19a514f9,(FULL)0x77c94478,
	    (FULL)0x843d1d32,(FULL)0xfc5edb22,(FULL)0xe8ee5e6e,(FULL)0x4fc42ce5,
	    (FULL)0x8488013e,(FULL)0xc938713c,(FULL)0x20ab0cac,(FULL)0x6a318f03,
	    (FULL)0xffc8bff3,(FULL)0x73e6d1a3,(FULL)0x8ca96aa7,(FULL)0x0cd3232a,
	    (FULL)0x905f770d,(FULL)0x605c8036,(FULL)0x8b8d04a2,(FULL)0x4d037b00,
	    (FULL)0xcb277294,(FULL)0x1ed81965,(FULL)0x7a254ff3,(FULL)0x408d9c47,
	    (FULL)0xe26c7377,(FULL)0x8b68587a,(FULL)0x8a48832f,(FULL)0xcff191a4,
	    (FULL)0x8aeb6fe6,(FULL)0x12d3df1d,(FULL)0x1feda37a,(FULL)0xb2bf907e,
	    (FULL)0x3bb5f39f,(FULL)0x4e5f7719,(FULL)0x8f5d1581,(FULL)0x33ebcf6f,
	    (FULL)0xd33654eb,(FULL)0x203c8e48,(FULL)0xf19c8a4e,(FULL)0x68d3656e,
	    (FULL)0x986eb2af,(FULL)0x3ec20b04,(FULL)0x062c3841,(FULL)0x5d73a03b,
	    (FULL)0x5d4e49eb,(FULL)0x836ce709,(FULL)0xc3f49221,(FULL)0x2310bc40,
	    (FULL)0xa6d0cbf6,(FULL)0x3868ee48,(FULL)0x4a43deb1,(FULL)0x67578aa6,
	    (FULL)0x150dfc26,(FULL)0x6e3426c1,(FULL)0x3131be30,(FULL)0xc541ccaa,
	    (FULL)0xcec7aab2,(FULL)0xf7e57432,(FULL)0x8cb3c873,(FULL)0x2b35de99,
	    (FULL)0x8663a5d7,(FULL)0x7b9f7764,(FULL)0xa771e5a6,(FULL)0x23b00e6a,
	    (FULL)0xa9985d05,(FULL)0x859c775c,(FULL)0x6b692f1f,(FULL)0x99636ea1,
	    (FULL)0x3730800d,(FULL)0x8700ac70,(FULL)0x4298a753,(FULL)0x46142502,
	    (FULL)0x809e955f,(FULL)0xea4a411b,(FULL)0x33709dfb,(FULL)0x3119ad40,
	    (FULL)0x5f01cb7c,(FULL)0xb76a6c6e,(FULL)0x15984eaf,(FULL)0x6109dc8a,
	    (FULL)0xa5ca9505,(FULL)0x5d686db9,(FULL)0x3b7e6add,(FULL)0x8e80d761,
	    (FULL)0xde6f6fd3,(FULL)0x79cbd718,(FULL)0x1da0f699,(FULL)0x40e9cd15,
	    (FULL)0xb24f312d,(FULL)0xe82158ba,(FULL)0xf5e5c36b,(FULL)0x79a4c927,
	    (FULL)0xa0039333,(FULL)0xc25247c9,(FULL)0x1766d81d,(FULL)0x93687116,
	    (FULL)0xa6741327,(FULL)0x3c6a03b4,(FULL)0xc002f29a,(FULL)0xc8a7b6e8,
	    (FULL)0x7bbd5ea3,(FULL)0x0e2a67c6,(FULL)0x441eabc1,(FULL)0x0929042d,
	    (FULL)0x25e82085,(FULL)0x7dbe232a,(FULL)0x44fbac3d,(FULL)0x8cfb26e5,
	    (FULL)0x388ab983,(FULL)0x8e40384d,(FULL)0x554632f8,(FULL)0x48dc1230,
	    (FULL)0xab492397,(FULL)0xab405048,(FULL)0xa118e387,(FULL)0x21c9e2f5,
	    (FULL)0x343b61b5,(FULL)0x484d1a8c,(FULL)0xab256f26,(FULL)0xd49e3dec,
	    (FULL)0x78f2d2e3,(FULL)0xe615c7fd,(FULL)0xce6cc2ed,(FULL)0x8442cc33,
	    (FULL)0x44d4bbf6,(FULL)0x0a3b93d8,(FULL)0x9301de77,(FULL)0x2d7e4efe,
	    (FULL)0xd8790d8a,(FULL)0x33711b76,(FULL)0x44df77e7,(FULL)0xc07dc30e,
	    (FULL)0x9ddd508f,(FULL)0xb9132ed0,(FULL)0xc6fb43cc,(FULL)0x45d06cf8,
	    (FULL)0xd585dd7b,(FULL)0x22bed18a,(FULL)0x10799ffa,(FULL)0x61c6cced,
	    (FULL)0xe4bd9aa9,(FULL)0xd7f2393b,(FULL)0xcfd55094,(FULL)0x706753fb,
	    (FULL)0xede6e446,(FULL)0xf65a6713,(FULL)0x47c0d5c3,(FULL)0x8bf6dfae,
	    (FULL)0x9f7927d6,(FULL)0xfb4dfc17,(FULL)0xe212c297,(FULL)0x12ebbc16,
	    (FULL)0xa00a954c,(FULL)0x43c71283,(FULL)0xe7bd40a5,(FULL)0x8957087a,
	    (FULL)0x08344837,(FULL)0xb0859d71,(FULL)0xaeb313f5,(FULL)0xfbf4b9a3,
	    (FULL)0xce81823a,(FULL)0x5e66e5be,(FULL)0x58ad6da1,(FULL)0x09a11c6e,
	    (FULL)0xc608054f,(FULL)0xc76f4316,(FULL)0x46084099,(FULL)0xb5821361,
	    (FULL)0x17a725ed,(FULL)0x4210008f,(FULL)0xd347c481,(FULL)0xe5ff8912
#else
   /\../\	FULL_BITS must be 32 or 64	/\../\	 !!!
#endif
};


/*
 * Linear Congruential Constants
 *
 *	a = 6316878969928993981 = 0x57aa0ff473c0ccbd
 *	c = 1363042948800878693 = 0x12ea805718e09865
 *
 * These constants are used in the randreseed64().  See below.
 */
#if FULL_BITS == SBITS
STATIC CONST HALF a_vec[SHALFS] = { (HALF)0x73c0ccbd, (HALF)0x57aa0ff4 };
STATIC CONST HALF c_vec[SHALFS] = { (HALF)0x18e09865, (HALF)0x12ea8057 };
#elif 2*FULL_BITS == SBITS
STATIC CONST HALF a_vec[SHALFS] = { (HALF)0xccbd, (HALF)0x73c0,
				    (HALF)0x0ff4, (HALF)0x57aa };
STATIC CONST HALF c_vec[SHALFS] = { (HALF)0x9865, (HALF)0x18e0,
				    (HALF)0x8057, (HALF)0x12ea };
#else
   /\../\	FULL_BITS must be 32 or 64	/\../\	 !!!
#endif
STATIC CONST ZVALUE a_val = {(HALF *)a_vec, SHALFS, 0};
STATIC CONST ZVALUE c_val = {(HALF *)c_vec, SHALFS, 0};


/*
 * current s100 generator state
 */
STATIC RAND s100;


/*
 * declare static functions
 */
S_FUNC void randreseed64(ZVALUE seed, ZVALUE *res);
S_FUNC int slotcp(BITSTR *bitstr, FULL *src, int count);
S_FUNC void slotcp64(BITSTR *bitstr, FULL *src);


/*
 * randreseed64 - scramble seed in 64 bit chunks
 *
 * given:
 *	a seed
 *
 * returns:
 *	a scrambled seed, or 0 if seed was 0
 *
 * It is 'nice' when a seed of "n" produces a 'significantly different'
 * sequence than a seed of "n+1".  Generators, by convention, assign
 * special significance to the seed of '0'.  It is an unfortunate that
 * people often pick small seed values, particularly when large seed
 * are of significance to the generators found in this file.
 *
 * We will process seed 64 bits at a time until the entire seed has been
 * exhausted.  If a 64 bit chunk is 0, then 0 is returned.  If the 64 bit
 * chunk is non-zero, we will produce a different and unique new scrambled
 * chunk.  In particular, if the seed is 0 we will return 0.  If the seed
 * is non-zero, we will return a different value (though chunks of 64
 * zeros will remain zero).  This scrambling will effectively eliminate
 * the human perceptions that are noted above.
 *
 * It should be noted that the purpose of this process is to scramble a seed
 * ONLY.  We do not care if these generators produce good random numbers.
 * We only want to help eliminate the human factors and perceptions
 * noted above.
 *
 * This function scrambles all 64 bit chunks of a seed, by mapping [0,2^64)
 * into [0,2^64).  This map is one-to-one and onto.  Mapping is performed
 * using  a linear congruence generator of the form:
 *
 *		X1 <-- (a*X0 + c) % m
 *
 * with the exception that:
 *
 *		0 ==> 0			(so that srand(0) acts as default)
 *		randreseed64() is an 1-to-1 and onto map
 *
 * The generator are based on the linear congruential generators found in
 * Knuth's "The Art of Computer Programming - Seminumerical Algorithms",
 * vol 2, 2nd edition (1981), Section 3.6, pages 170-171.
 *
 * Because we process 64 bits we will take:
 *
 *		m = 2^64			(based on note ii)
 *
 * We will scan the Rand Book of Random Numbers to look for an 'a' such that:
 *
 *		a mod 8 == 5			(based on note iii)
 *		0.01*m < a < 0.99*m		(based on note iv)
 *		0.01*2^64 < a < 0.99*2^64
 *
 * To help keep the generators independent, we want:
 *
 *		a is prime
 *
 * The choice of an adder 'c' is considered immaterial according (based
 * in note v).	Knuth suggests 'c==1' or 'c==a'.  We elect to select 'c'
 * using the same process as we used to select 'a'.  The choice is
 * 'immaterial' after all, and as long as:
 *
 *		gcd(c, m) == 1		(based on note v)
 *		gcd(c, 2^64) == 1
 *
 * the concerns are met.   It can be shown that if we have:
 *
 *		gcd(a, c) == 1
 *
 * then the adders and multipliers will be more independent.
 *
 * We will obtain the values 'a' and 'c for our generator from the
 * Rand Book of Random Numbers.	 Because m=2^64 is 20 decimal digits long,
 * we will search the Rand Book of Random Numbers 20 at a time.	 We will
 * skip any of the 100 values that were used to initialize the subtractive 100
 * generators.	The values obtained from the Rand Book of Random Numbers are:
 *
 *		a = 6316878969928993981
 *		c = 1363042948800878693
 *
 * As we stated before, we must map 0 ==> 0.  The linear congruence
 * generator would normally map as follows:
 *
 *	0 ==> 1363042948800878693	(0 ==> c)
 *
 * We can determine which 0<=y<m will produce a given value z by inverting the
 * linear congruence generator:
 *
 *	z = (a*y + c) % m
 *
 *	z = a*y % m + c % m
 *
 *	z-c % m = a*y % m
 *
 *	(z-c % m) * minv(a,m) = (a*y % m) * minv(a,m)
 *		[minv(a,m) is the multiplicative inverse of a % m]
 *
 *	((z-c % m) * minv(a,m)) % m = ((a*y % m) * minv(a,m)) % m
 *
 *	((z-c % m) * minv(a,m)) % m = y % m
 *		[a % m * minv(a,m) = 1 % m by definition]
 *
 *	((z-c) * minv(a,m)) % m = y % m
 *
 *	((z-c) * minv(a,m)) % m = y
 *		[we defined y to be 0<=y<m]
 *
 * To determine which value maps back into 0, we let z = 0 and compute:
 *
 *	((0-c) * minv(a,m)) % m ==> 10239951819489363767
 *
 * and thus we find that the congruence generator would also normally map:
 *
 *	10239951819489363767 ==> 0
 *
 * To overcome this, and preserve the 1-to-1 and onto map, we force:
 *
 *	0 ==> 0
 *	10239951819489363767 ==> 1363042948800878693
 *
 * To repeat, this function converts a values into a seed value.  With the
 * except of 'seed == 0', every value is mapped into a unique seed value.
 * This mapping need not be complex, random or secure.	All we attempt
 * to do here is to allow humans who pick small or successive seed values
 * to obtain reasonably different sequences from the generators below.
 *
 * NOTE: This is NOT a pseudo random number generator.	This function is
 *	 intended to be used internally by ss100rand() and sshufrand().
 */
S_FUNC void
randreseed64(ZVALUE seed, ZVALUE *res)
{
	ZVALUE t;		/* temp value */
	ZVALUE chunk;		/* processed 64 bit chunk value */
	ZVALUE seed64;		/* seed mod 2^64 */
	HALF *v64;		/* 64 bit array of HALFs */
	long chunknum;		/* 64 bit chunk number */

	/*
	 * quickly return 0 if seed is 0
	 */
	if (ziszero(seed) || seed.len <= 0) {
		itoz(0, res);
		return;
	}

	/*
	 * allocate result
	 */
	seed.sign = 0;	/* use the value of seed */
	res->len = (int)(((seed.len+SHALFS-1) / SHALFS) * SHALFS);
	res->v = alloc(res->len);
	res->sign = 0;
	memset(res->v, 0, res->len*sizeof(HALF));  /* default value is 0 */

	/*
	 * process 64 bit chunks until done
	 */
	chunknum = 0;
	while (!zislezero(seed)) {

		/*
		 * grab the lower 64 bits of seed
		 */
		if (zge64b(seed)) {
			v64 = alloc(SHALFS);
			memcpy(v64, seed.v, SHALFS*sizeof(HALF));
			seed64.v = v64;
			seed64.len = SHALFS;
			seed64.sign = 0;
		} else {
			zcopy(seed, &seed64);
		}
		zshiftr(seed, SBITS);
		ztrim(&seed);
		ztrim(&seed64);

		/*
		 * do nothing if chunk is zero
		 */
		if (ziszero(seed64)) {
			++chunknum;
			zfree(seed64);
			continue;
		}

		/*
		 * Compute the linear congruence generator map:
		 *
		 *	X1 <-- (a*X0 + c) mod m
		 *
		 * in other words:
		 *
		 *	chunk == (a_val*seed + c_val) mod 2^64
		 */
		zmul(seed64, a_val, &t);
		zfree(seed64);
		zadd(t, c_val, &chunk);
		zfree(t);

		/*
		 * form chunk mod 2^64
		 */
		if (chunk.len > (SB32)SHALFS) {
			/* result is too large, reduce to 64 bits */
			v64 = alloc(SHALFS);
			memcpy(v64, chunk.v, SHALFS*sizeof(HALF));
			free(chunk.v);
			chunk.v = v64;
			chunk.len = SHALFS;
			ztrim(&chunk);
		}

		/*
		 * Normally, the above equation would map:
		 *
		 *	    f(0) == 1363042948800878693
		 *	    f(10239951819489363767) == 0
		 *
		 * However, we have already forced f(0) == 0.  To preserve the
		 * 1-to-1 and onto map property, we force:
		 *
		 *	    f(10239951819489363767) ==> 1363042948800878693
		 */
		if (ziszero(chunk)) {
			/* load 1363042948800878693 instead of 0 */
			zcopy(c_val, &chunk);
			memcpy(res->v+(chunknum*SHALFS), c_val.v,
			       c_val.len*sizeof(HALF));

		/*
		 * load the 64 bit chunk into the result
		 */
		} else {
			memcpy(res->v+(chunknum*SHALFS), chunk.v,
			       chunk.len*sizeof(HALF));
		}
		++chunknum;
		zfree(chunk);
	}
	ztrim(res);
}


/*
 * zsrand - seed the s100 generator
 *
 * given:
 *	pseed	- ptr to seed of the generator or NULL
 *	pmat100	- subtractive 100 state table or NULL
 *
 * returns:
 *	previous s100 state
 */
RAND *
zsrand(CONST ZVALUE *pseed, CONST MATRIX *pmat100)
{
	RAND *ret;		/* previous s100 state */
	CONST VALUE *v;		/* value from a passed matrix */
	ZVALUE zscram;		/* scrambled 64 bit seed */
	ZVALUE ztmp;		/* temp holding value for zscram */
	ZVALUE seed;		/* to hold *pseed */
	FULL shufxor[SLEN];	/* zshufxor as an 64 bit array of FULLs */
	long indx;		/* index to shuffle slots for seeding */
	int i;

	/*
	 * firewall
	 */
	if (pseed != NULL && zisneg(*pseed)) {
		math_error("neg seeds for srand reserved for future use");
		/*NOTREACHED*/
	}

	/*
	 * initialize state if first call
	 */
	if (!s100.seeded) {
		s100 = init_s100;
	}

	/*
	 * save the current state to return later
	 */
	ret = (RAND *)malloc(sizeof(RAND));
	if (ret == NULL) {
		math_error("cannot allocate RAND state");
		/*NOTREACHED*/
	}
	*ret = s100;

	/*
	 * if call was srand(), just return current state
	 */
	if (pseed == NULL && pmat100 == NULL) {
		return ret;
	}

	/*
	 * if call is srand(0), initialize and return quickly
	 */
	if (pmat100 == NULL && ziszero(*pseed)) {
		s100 = init_s100;
		return ret;
	}

	/*
	 * clear buffered bits, initialize pointers
	 */
	s100.seeded = 0; /* not seeded now */
	s100.j = INIT_J;
	s100.k = INIT_K;
	s100.bits = 0;
	memset(s100.buffer, 0, sizeof(s100.buffer));

	/*
	 * load subtractive table
	 *
	 * We will load the default subtractive table unless we are passed a
	 * matrix.  If we are passed a matrix, we will load the first 100
	 * values mod 2^64 instead.
	 */
	if (pmat100 == NULL) {
		memcpy(s100.slot, def_subtract, sizeof(def_subtract));
	} else {

		/*
		 * use the first 100 entries from the matrix arg
		 */
		if (pmat100->m_size < S100) {
			math_error("matrix for srand has < 100 elements");
			/*NOTREACHED*/
		}
		for (v=pmat100->m_table, i=0; i < S100; ++i, ++v) {

			/* reject if not integer */
			if (v->v_type != V_NUM || qisfrac(v->v_num)) {
			    math_error("matrix for srand must contain ints");
			    /*NOTREACHED*/
			}

			/* load table element from matrix element mod 2^64 */
			SLOAD(s100, i, v->v_num->num);
		}
	}

	/*
	 * scramble the seed in 64 bit chunks
	 */
	if (pseed != NULL) {
		seed.sign = pseed->sign;
		seed.len = pseed->len;
		seed.v = alloc(seed.len);
		zcopyval(*pseed, seed);
		randreseed64(seed, &zscram);
		zfree(seed);
	}

	/*
	 * xor subtractive table with the rehashed lower 64 bits of seed
	 */
	if (pseed != NULL && !ziszero(zscram)) {

		/* xor subtractive table with lower 64 bits of seed */
		SMOD64(shufxor, zscram);
		for (i=0; i < S100; ++i) {
			SXOR(s100, i, shufxor);
		}
	}

	/*
	 * shuffle subtractive 100 table according to seed, if passed
	 */
	if (pseed != NULL && zge64b(zscram)) {

		/* prepare the seed for subtractive slot shuffling */
		zshiftr(zscram, 64);
		ztrim(&zscram);

		/* shuffle subtractive table */
		for (i=S100-1; i > 0 && !zislezero(zscram); --i) {

			/* determine what we will swap with */
			indx = zdivi(zscram, i+1, &ztmp);
			zfree(zscram);
			zscram = ztmp;

			/* do nothing if swap with itself */
			if (indx == i) {
				continue;
			}

			/* swap slot[i] with slot[indx] */
			SSWAP(s100, i, indx);
		}
		zfree(zscram);
	} else if (pseed != NULL) {
		zfree(zscram);
	}

	/*
	 * load the shuffle table
	 *
	 * We will generate SHUFCNT entries from the subtractive 100 slots
	 * and fill the shuffle table in consecutive order.
	 */
	for (i=0; i < SHUFCNT; ++i) {

		/*
		 * skip values if required
		 *
		 * See:
		 *	Knuth's "The Art of Computer Programming -
		 *	Seminumerical Algorithms", Vol 2, 3rd edition (1998),
		 *	Section 3.6, page 188".
		 */
		if (s100.need_to_skip <= 0) {
			int sk;

			/* skip the require number of values */
			for (sk=0; sk < RAND_SKIP; ++sk) {

				/* bump j and k */
				if (++s100.j >= S100) {
					s100.j = 0;
				}
				if (++s100.k >= S100) {
					s100.k = 0;
				}

				/* slot[k] -= slot[j] */
				SSUB(s100, s100.k, s100.j);

				/* NOTE: don't shuffle, no shuffle table yet */
			}

			/* reset the skip count */
			s100.need_to_skip = RAND_CONSEQ_USE;
			if (conf->calc_debug & CALCDBG_RAND) {
			    printf("rand: skipped %d states\n", RAND_SKIP);
			}

		/* no skip, just decrement use counter */
		} else {
			--s100.need_to_skip;
		}

		/* bump j and k */
		if (++s100.j >= S100) {
			s100.j = 0;
		}
		if (++s100.k >= S100) {
			s100.k = 0;
		}

		/* slot[k] -= slot[j] */
		SSUB(s100, s100.k, s100.j);

		/* shuf[i] = slot[k] */
		SSHUF(s100, i, s100.k);
	}

	/*
	 * note that we are seeded
	 */
	s100.seeded = 1;

	/*
	 * return the previous state
	 */
	return ret;
}


/*
 * zsetrand - set the s100 generator state
 *
 * given:
 *	state - the state to copy
 *
 * returns:
 *	previous s100 state
 */
RAND *
zsetrand(CONST RAND *state)
{
	RAND *ret;		/* previous s100 state */

	/*
	 * initialize state if first call
	 */
	if (!s100.seeded) {
		s100 = init_s100;
	}

	/*
	 * save the current state to return later
	 */
	ret = randcopy(&s100);

	/*
	 * load the new state
	 */
	s100 = *state;

	/*
	 * return the previous state
	 */
	return ret;
}


/*
 * slotcp - copy up to 64 bits from a 64 bit array of FULLs to some HALFs
 *
 * We will copy data from an array of FULLs into an array of HALFs.
 * The destination within the HALFs is some bit location found in bitstr.
 * We will attempt to copy 64 bits, but if there is not enough room
 * (bits not yet loaded) in the destination bit string we will transfer
 * what we can.
 *
 * The src slot is 64 bits long and is stored as an array of FULLs.
 * When FULL_BITS is 64 the element is 1 FULL, otherwise FULL_BITS
 * is 32 bits and the element is 2 FULLs.  The most significant bit
 * in the array (highest bit in the last FULL of the array) is to
 * be transferred to the most significant bit in the destination.
 *
 * given:
 *	bitstr	- most significant destination bit in a bit string
 *	src	- low order FULL in a 64 bit slot
 *	count	- number of bits to transfer (must be 0 < count <= 64)
 *
 * returns:
 *	number of bits transferred
 */
S_FUNC int
slotcp(BITSTR *bitstr, FULL *src, int count)
{
	HALF *dh;		/* most significant HALF in dest */
	int dnxtbit;		/* next bit beyond most significant in dh */
	int need;		/* number of bits we need to transfer */
	int ret;		/* bits transferred */

	/*
	 * determine how many bits we actually need to transfer
	 */
	dh = bitstr->loc;
	dnxtbit = bitstr->bit+1;
	count &= (SBITS-1);
	need = (bitstr->len < count) ? bitstr->len : count;

	/*
	 * prepare for the return
	 *
	 * Note the final bitstr location after we have moved the
	 * position down 'need' bits.
	 */
	bitstr->len -= need;
	bitstr->loc -= need / BASEB;
	bitstr->bit -= need % BASEB;
	if (bitstr->bit < 0) {
		--bitstr->loc;
		bitstr->bit += BASEB;
	}
	ret = need;

	/*
	 * deal with aligned copies quickly
	 */
	if (dnxtbit == BASEB) {
		if (need == SBITS) {
#if 2*FULL_BITS == SBITS
			*dh-- = (HALF)(src[SLEN-1] >> BASEB);
			*dh-- = (HALF)(src[SLEN-1]);
#endif
			*dh-- = (HALF)(src[0] >> BASEB);
			*dh = (HALF)(src[0]);
#if 2*FULL_BITS == SBITS
		} else if (need > FULL_BITS+BASEB) {
			*dh-- = (HALF)(src[SLEN-1] >> BASEB);
			*dh-- = (HALF)(src[SLEN-1]);
			*dh-- = (HALF)(src[0] >> BASEB);
			*dh = ((HALF)src[0] &
				     highhalf[need-FULL_BITS-BASEB]);
		} else if (need > FULL_BITS) {
			*dh-- = (HALF)(src[SLEN-1] >> BASEB);
			*dh-- = (HALF)(src[SLEN-1]);
			*dh = ((HALF)(src[0] >> BASEB) &
				     highhalf[need-FULL_BITS]);
#endif
		} else if (need > BASEB) {
			*dh-- = (HALF)(src[SLEN-1] >> BASEB);
			*dh = ((HALF)(src[SLEN-1]) & highhalf[need-BASEB]);
		} else {
			*dh = ((HALF)(src[SLEN-1] >> BASEB) & highhalf[need]);
		}
		return ret;
	}

	/*
	 * load the most significant HALF
	 */
	if (need >= dnxtbit) {
		/* fill up the most significant HALF */
		*dh-- |= (HALF)(src[SLEN-1] >> (FULL_BITS-dnxtbit));
		need -= dnxtbit;
	} else if (need > 0) {
		/* we exhaust our need before 1st half is filled */
		*dh |= (HALF)((src[SLEN-1] >> (FULL_BITS-need)) <<
				(dnxtbit-need));
		return ret;	/* our need has been filled */
	} else {
		return ret;	/* our need has been filled */
	}

	/*
	 * load the 2nd most significant HALF
	 */
	if (need > BASEB) {
		/* fill	 up the 2nd most significant HALF */
		*dh-- = (HALF)(src[SLEN-1] >> (BASEB-dnxtbit));
		need -= BASEB;
	} else if (need > 0) {
		/* we exhaust our need before 2nd half is filled */
		*dh |= ((HALF)(src[SLEN-1] >> (BASEB-dnxtbit)) &
			 highhalf[need]);
		return ret;	/* our need has been filled */
	} else {
		return ret;	/* our need has been filled */
	}

	/*
	 * load the 3rd most significant HALF
	 *
	 * At this point we know that our 3rd HALF will force us
	 * to cross into a second FULL for systems with 32 bit FULLs.
	 * We know this because the aligned case has been previously
	 * taken care of above.
	 *
	 * For systems that have 64 bit FULLs (and 32 bit HALFs) this
	 * is will be our least significant HALF.  We also know that
	 * the need must be < BASEB.
	 */
#if FULL_BITS == SBITS
	*dh |= (((HALF)src[0] & highhalf[dnxtbit+need]) << dnxtbit);
#else
	if (need > BASEB) {
		/* load the remaining bits from the most significant FULL */
		*dh-- = ((((HALF)src[SLEN-1] & lowhalf[BASEB-dnxtbit])
			   << dnxtbit) | (HALF)(src[0] >> (FULL_BITS-dnxtbit)));
		need -= BASEB;
	} else if (need > 0) {
		/* load the remaining bits from the most significant FULL */
		*dh-- |= (((((HALF)src[SLEN-1] & lowhalf[BASEB-dnxtbit])
			 << dnxtbit) | (HALF)(src[0] >> (FULL_BITS-dnxtbit))) &
			  highhalf[need]);
		return ret;	/* our need has been filled */
	} else {
		return ret;	/* our need has been filled */
	}

	/*
	 * load the 4th most significant HALF
	 *
	 * At this point, only 32 bit FULLs are operating.
	 */
	if (need > BASEB) {
		/* fill	 up the 2nd most significant HALF */
		*dh-- = (HALF)(src[0] >> (BASEB-dnxtbit));
		/* no need TODO: need -= BASEB, because we are nearly done */
	} else if (need > 0) {
		/* we exhaust our need before 2nd half is filled */
		*dh |= ((HALF)(src[0] >> (BASEB-dnxtbit)) &
			 highhalf[need]);
		return ret;	/* our need has been filled */
	} else {
		return ret;	/* our need has been filled */
	}

	/*
	 * load the 5th and least significant HALF
	 *
	 * At this point we know that the need will be satisfied.
	 */
	*dh |= (((HALF)src[0] & lowhalf[BASEB-dnxtbit]) << dnxtbit);
#endif
	return ret;	/* our need has been filled */
}


/*
 * slotcp64 - copy 64 bits from a 64 bit array of FULLs to some HALFs
 *
 * We will copy data from an array of FULLs into an array of HALFs.
 * The destination within the HALFs is some bit location found in bitstr.
 * Unlike slotcp(), this function will always copy 64 bits.
 *
 * The src slot is 64 bits long and is stored as an array of FULLs.
 * When FULL_BITS is 64 this array is 1 FULL, otherwise FULL_BITS
 * is 32 bits and the array is 2 FULLs.	 The most significant bit
 * in the array (highest bit in the last FULL of the array) is to
 * be transferred to the most significant bit in the destination.
 *
 * given:
 *	bitstr	- most significant destination bit in a bit string
 *	src	- low order FULL in a 64 bit slot
 *
 * returns:
 *	number of bits transferred
 */
S_FUNC void
slotcp64(BITSTR *bitstr, FULL *src)
{
	HALF *dh = bitstr->loc;	     /* most significant HALF in dest */
	int dnxtbit = bitstr->bit+1; /* next dh bit beyond most significant */

	/*
	 * prepare for the return
	 *
	 * Since we are moving the point 64 bits down, we know that
	 * the bit location (bitstr->bit) will remain the same.
	 */
	bitstr->len -= SBITS;
	bitstr->loc -= SBITS/BASEB;

	/*
	 * deal with aligned copies quickly
	 */
	if (dnxtbit == BASEB) {
#if 2*FULL_BITS == SBITS
		*dh-- = (HALF)(src[SLEN-1] >> BASEB);
		*dh-- = (HALF)(src[SLEN-1]);
#endif
		*dh-- = (HALF)(src[0] >> BASEB);
		*dh = (HALF)(src[0]);
		return;
	}

	/*
	 * load the most significant HALF
	 */
	*dh-- |= (HALF)(src[SLEN-1] >> (FULL_BITS-dnxtbit));

	/*
	 * load the 2nd most significant HALF
	 */
	*dh-- = (HALF)(src[SLEN-1] >> (BASEB-dnxtbit));

	/*
	 * load the 3rd most significant HALF
	 *
	 * At this point we know that our 3rd HALF will force us
	 * to cross into a second FULL for systems with 32 bit FULLs.
	 * We know this because the aligned case has been previously
	 * taken care of above.
	 *
	 * For systems that have 64 bit FULLs (and 32 bit HALFs) this
	 * is will be our least significant HALF.
	 */
#if FULL_BITS == SBITS
	*dh |= (((HALF)src[0] & lowhalf[BASEB-dnxtbit]) << dnxtbit);
#else
	/* load the remaining bits from the most significant FULL */
	*dh-- = ((((HALF)src[SLEN-1] & lowhalf[BASEB-dnxtbit])
		   << dnxtbit) | (HALF)(src[0] >> (FULL_BITS-dnxtbit)));

	/*
	 * load the 4th most significant HALF
	 *
	 * At this point, only 32 bit FULLs are operating.
	 */
	*dh-- = (HALF)(src[0] >> (BASEB-dnxtbit));

	/*
	 * load the 5th and least significant HALF
	 *
	 * At this point we know that the need will be satisfied.
	 */
	*dh |= (((HALF)src[0] & lowhalf[BASEB-dnxtbit]) << dnxtbit);
#endif
}


/*
 * zrandskip - skip s s100 bits
 *
 * given:
 *	count - number of bits to be skipped
 */
void
zrandskip(long cnt)
{
	int indx;		/* shuffle entry index */

	/*
	 * initialize state if first call
	 */
	if (!s100.seeded) {
		s100 = init_s100;
	}

	/*
	 * skip required bits in the buffer
	 */
	if (s100.bits > 0 && s100.bits <= cnt) {

		/* just toss the buffer bits */
		cnt -= s100.bits;
		s100.bits = 0;
		memset(s100.buffer, 0, sizeof(s100.buffer));

	} else if (s100.bits > 0 && s100.bits > cnt) {

		/* buffer contains more bits than we need to toss */
#if FULL_BITS == SBITS
		s100.buffer[0] <<= cnt;
#else
		if (cnt >= FULL_BITS) {
			s100.buffer[SLEN-1] = (s100.buffer[0] << cnt);
			s100.buffer[0] = 0;
		} else {
			s100.buffer[SLEN-1] =
			    ((s100.buffer[SLEN-1] << cnt) |
			     (s100.buffer[0] >> (FULL_BITS-cnt)));
			s100.buffer[0] <<= cnt;
		}
#endif
		s100.bits -= cnt;
		return; /* skip need satisfied */
	}

	/*
	 * skip 64 bits at a time
	 */
	while (cnt >= SBITS) {

		/*
		 * skip values if required
		 *
		 * NOTE: This skip loop is part of the algorithm, not part
		 *	 of the builtin function request.
		 *
		 * See:
		 *	Knuth's "The Art of Computer Programming -
		 *	Seminumerical Algorithms", Vol 2, 3rd edition (1998),
		 *	Section 3.6, page 188".
		 */
		if (s100.need_to_skip <= 0) {
			int sk;

			/* skip the require number of values */
			for (sk=0; sk < RAND_SKIP; ++sk) {
				/* bump j and k */
				if (++s100.j >= S100) {
					s100.j = 0;
				}
				if (++s100.k >= S100) {
					s100.k = 0;
				}

				/* slot[k] -= slot[j] */
				SSUB(s100, s100.k, s100.j);

				/* store s100.k into s100.slot[indx] */
				indx = SINDX(s100, s100.k);
				SSHUF(s100, indx, s100.k);
			}

			/* reset the skip count */
			s100.need_to_skip = RAND_CONSEQ_USE;
			if (conf->calc_debug & CALCDBG_RAND) {
			    printf("rand: skipped %d states\n", RAND_SKIP);
			}

		/* no skip, just decrement use counter */
		} else {
			--s100.need_to_skip;
		}

		/* bump j and k */
		if (++s100.j >= S100) {
			s100.j = 0;
		}
		if (++s100.k >= S100) {
			s100.k = 0;
		}

		/* slot[k] -= slot[j] */
		SSUB(s100, s100.k, s100.j);

		/* we will ignore the output value of s100.slot[indx] */
		indx = SINDX(s100, s100.k);
		cnt -= SBITS;

		/* store s100.k into s100.slot[indx] */
		SSHUF(s100, indx, s100.k);
	}

	/*
	 * skip the final bits
	 */
	if (cnt > 0) {

		/*
		 * skip values if required
		 *
		 * NOTE: This skip loop is part of the algorithm, not part
		 *	 of the builtin function request.
		 *
		 * See:
		 *	Knuth's "The Art of Computer Programming -
		 *	Seminumerical Algorithms", Vol 2, 3rd edition (1998),
		 *	Section 3.6, page 188".
		 */
		if (s100.need_to_skip <= 0) {
			int sk;

			/* skip the require number of values */
			for (sk=0; sk < RAND_SKIP; ++sk) {

				/* bump j and k */
				if (++s100.j >= S100) {
					s100.j = 0;
				}
				if (++s100.k >= S100) {
					s100.k = 0;
				}

				/* slot[k] -= slot[j] */
				SSUB(s100, s100.k, s100.j);

				/* store s100.k into s100.slot[indx] */
				indx = SINDX(s100, s100.k);
				SSHUF(s100, indx, s100.k);
			}

			/* reset the skip count */
			s100.need_to_skip = RAND_CONSEQ_USE;
			if (conf->calc_debug & CALCDBG_RAND) {
			    printf("rand: skipped %d states\n", RAND_SKIP);
			}

		/* no skip, just decrement use counter */
		} else {
			--s100.need_to_skip;
		}

		/* bump j and k */
		if (++s100.j >= S100) {
			s100.j = 0;
		}
		if (++s100.k >= S100) {
			s100.k = 0;
		}

		/* slot[k] -= slot[j] */
		SSUB(s100, s100.k, s100.j);

		/* we will ignore the output value of s100.slot[indx] */
		indx = SINDX(s100, s100.k);

		/*
		 * We know the buffer is empty, so fill it
		 * with any unused bits.  Copy SBITS-trans bits
		 * from slot[indx] into buffer.
		 */
		s100.bits = (int)(SBITS-cnt);
		memcpy(s100.buffer, &s100.shuf[indx*SLEN],
		       sizeof(s100.buffer));

		/*
		 * shift the buffer bits all the way up to
		 * the most significant bit
		 */
#if FULL_BITS == SBITS
		s100.buffer[0] <<= cnt;
#else
		if (cnt >= FULL_BITS) {
			s100.buffer[SLEN-1] = (s100.buffer[0] << cnt);
			s100.buffer[0] = 0;
		} else {
			s100.buffer[SLEN-1] =
			    ((s100.buffer[SLEN-1] << cnt) |
			     (s100.buffer[0] >> (FULL_BITS-cnt)));
			s100.buffer[0] <<= cnt;
		}
#endif

		/* store s100.k into s100.slot[indx] */
		SSHUF(s100, indx, s100.k);
	}
}


/*
 * zrand - crank the s100 generator for some bits
 *
 * We will load the ZVALUE with random bits starting from the
 * most significant and ending with the lowest bit in the
 * least significant HALF.
 *
 * given:
 *	count - number of bits required
 *	res - where to place the random bits as ZVALUE
 */
void
zrand(long cnt, ZVALUE *res)
{
	BITSTR dest;		/* destination bit string */
	int trans;		/* bits transferred */
	int indx;		/* shuffle entry index */

	/*
	 * firewall
	 */
	if (cnt <= 0) {
		if (cnt == 0) {
			/* zero length random number is always 0 */
			itoz(0, res);
			return;
		} else {
			math_error("negative zrand bit count");
			/*NOTREACHED*/
		}
#if LONG_BITS > 32
	} else if (cnt > (1L<<31)) {
		math_error("huge rand bit count in internal zrand function");
		/*NOTREACHED*/
#endif
	}

	/*
	 * initialize state if first call
	 */
	if (!s100.seeded) {
		s100 = init_s100;
	}

	/*
	 * allocate storage
	 */
	res->len = (LEN)((cnt+BASEB-1)/BASEB);
	res->v = alloc((LEN)((cnt+BASEB-1)/BASEB));

	/*
	 * dest bit string
	 */
	dest.len = (int)cnt;
	dest.loc = res->v + (((cnt+BASEB-1)/BASEB)-1);
	dest.bit = (int)((cnt-1) % BASEB);
	memset(res->v, 0, (LEN)((cnt+BASEB-1)/BASEB)*sizeof(HALF));

	/*
	 * load from buffer first
	 */
	if (s100.bits > 0) {

		/*
		 * We know there are only s100.bits in the buffer, so
		 * transfer as much as we can (treating it as a slot)
		 * and return the bit transfer count.
		 */
		trans = slotcp(&dest, s100.buffer, s100.bits);

		/*
		 * If we need to keep bits in the buffer,
		 * shift the buffer bits all the way up to
		 * the most significant unused bit.
		 */
		if (trans < s100.bits) {
#if FULL_BITS == SBITS
			s100.buffer[0] <<= trans;
#else
			if (trans >= FULL_BITS) {
				s100.buffer[SLEN-1] =
				    (s100.buffer[0] << (trans-FULL_BITS));
				s100.buffer[0] = 0;
			} else {
				s100.buffer[SLEN-1] =
				    ((s100.buffer[SLEN-1] << trans) |
				     (s100.buffer[0] >> (FULL_BITS-trans)));
				s100.buffer[0] <<= trans;
			}
#endif
		}
		/* note that we have fewer bits in the buffer */
		s100.bits -= trans;
	}

	/*
	 * spin the generator until we need less than 64 bits
	 *
	 * The buffer did not contain enough bits, so we crank the
	 * s100 generator and load then 64 bits at a time.
	 */
	while (dest.len >= SBITS) {

		/*
		 * skip values if required
		 *
		 * See:
		 *	Knuth's "The Art of Computer Programming -
		 *	Seminumerical Algorithms", Vol 2, 3rd edition (1998),
		 *	Section 3.6, page 188".
		 */
		if (s100.need_to_skip <= 0) {
			int sk;

			/* skip the require number of values */
			for (sk=0; sk < RAND_SKIP; ++sk) {

				/* bump j and k */
				if (++s100.j >= S100) {
					s100.j = 0;
				}
				if (++s100.k >= S100) {
					s100.k = 0;
				}

				/* slot[k] -= slot[j] */
				SSUB(s100, s100.k, s100.j);

				/* select slot index to output */
				indx = SINDX(s100, s100.k);

				/* store s100.k into s100.slot[indx] */
				SSHUF(s100, indx, s100.k);
			}

			/* reset the skip count */
			s100.need_to_skip = RAND_CONSEQ_USE;
			if (conf->calc_debug & CALCDBG_RAND) {
			    printf("rand: skipped %d states\n", RAND_SKIP);
			}

		/* no skip, just decrement use counter */
		} else {
			--s100.need_to_skip;
		}

		/* bump j and k */
		if (++s100.j >= S100) {
			s100.j = 0;
		}
		if (++s100.k >= S100) {
			s100.k = 0;
		}

		/* slot[k] -= slot[j] */
		SSUB(s100, s100.k, s100.j);

		/* select slot index to output */
		indx = SINDX(s100, s100.k);

		/* move up to 64 bits from slot[indx] to dest */
		slotcp64(&dest, &s100.shuf[indx*SLEN]);

		/* store s100.k into s100.slot[indx] */
		SSHUF(s100, indx, s100.k);
	}

	/*
	 * spin the generator one last time to fill out the remaining need
	 */
	if (dest.len > 0) {

		/*
		 * skip values if required
		 *
		 * See:
		 *	Knuth's "The Art of Computer Programming -
		 *	Seminumerical Algorithms", Vol 2, 3rd edition (1998),
		 *	Section 3.6, page 188".
		 */
		if (s100.need_to_skip <= 0) {
			int sk;

			/* skip the require number of values */
			for (sk=0; sk < RAND_SKIP; ++sk) {
				/* bump j and k */
				if (++s100.j >= S100) {
					s100.j = 0;
				}
				if (++s100.k >= S100) {
					s100.k = 0;
				}

				/* slot[k] -= slot[j] */
				SSUB(s100, s100.k, s100.j);

				/* select slot index to output */
				indx = SINDX(s100, s100.k);

				/* store s100.k into s100.slot[indx] */
				SSHUF(s100, indx, s100.k);
			}

			/* reset the skip count */
			s100.need_to_skip = RAND_CONSEQ_USE;
			if (conf->calc_debug & CALCDBG_RAND) {
			    printf("rand: skipped %d states\n", RAND_SKIP);
			}

		/* no skip, just decrement use counter */
		} else {
			--s100.need_to_skip;
		}

		/* bump j and k */
		if (++s100.j >= S100) {
			s100.j = 0;
		}
		if (++s100.k >= S100) {
			s100.k = 0;
		}

		/* slot[k] -= slot[j] */
		SSUB(s100, s100.k, s100.j);

		/* select slot index to output */
		indx = SINDX(s100, s100.k);

		/* move up to 64 bits from slot[indx] to dest */
		trans = slotcp(&dest, &s100.shuf[indx*SLEN], dest.len);

		/* buffer up unused bits if we are done */
		if (trans != SBITS) {

			/*
			 * We know the buffer is empty, so fill it
			 * with any unused bits.  Copy SBITS-trans bits
			 * from slot[indx] into buffer.
			 */
			s100.bits = SBITS-trans;
			memcpy(s100.buffer, &s100.shuf[indx*SLEN],
			       sizeof(s100.buffer));

			/*
			 * shift the buffer bits all the way up to
			 * the most significant bit
			 */
#if FULL_BITS == SBITS
			s100.buffer[0] <<= trans;
#else
			if (trans >= FULL_BITS) {
				s100.buffer[SLEN-1] =
				    (s100.buffer[0] << (trans-FULL_BITS));
				s100.buffer[0] = 0;
			} else {
				s100.buffer[SLEN-1] =
				    ((s100.buffer[SLEN-1] << trans) |
				     (s100.buffer[0] >> (FULL_BITS-trans)));
				s100.buffer[0] <<= trans;
			}
#endif
		}

		/* store s100.k into s100.slot[indx] */
		SSHUF(s100, indx, s100.k);
	}
	res->sign = 0;
	ztrim(res);
}


/*
 * zrandrange - generate an s100 random value in the range [low, beyond)
 *
 * given:
 *	low - low value of range
 *	beyond - beyond end of range
 *	res - where to place the random bits as ZVALUE
 */
void
zrandrange(CONST ZVALUE low, CONST ZVALUE beyond, ZVALUE *res)
{
	ZVALUE range;		/* beyond-low */
	ZVALUE rval;		/* random value [0, 2^bitlen) */
	ZVALUE rangem1;		/* range - 1 */
	long bitlen;		/* smallest power of 2 >= diff */

	/*
	 * firewall
	 */
	if (zrel(low, beyond) >= 0) {
		math_error("srand low range >= beyond range");
		/*NOTREACHED*/
	}

	/*
	 * determine the size of the random number needed
	 */
	zsub(beyond, low, &range);
	if (zisone(range)) {
		zfree(range);
		*res = low;
		return;
	}
	zsub(range, _one_, &rangem1);
	bitlen = 1+zhighbit(rangem1);
	zfree(rangem1);

	/*
	 * generate a random value between [0, diff)
	 *
	 * We will not fall into the trap of thinking that we can simply take
	 * a value mod 'range'.	 Consider the case where 'range' is '80'
	 * and we are given pseudo-random numbers [0,100).  If we took them
	 * mod 80, then the numbers [0,20) would be produced more frequently
	 * because the numbers [81,100) mod 80 wrap back into [0,20).
	 */
	rval.v = NULL;
	do {
		if (rval.v != NULL) {
			zfree(rval);
		}
		zrand(bitlen, &rval);
	} while (zrel(rval, range) >= 0);

	/*
	 * add in low value to produce the range [0+low, diff+low)
	 * which is the range [low, beyond)
	 */
	zadd(rval, low, res);
	zfree(rval);
	zfree(range);
}


/*
 * irand - generate an s100 random long in the range [0, s)
 *
 * given:
 *	s - limit of the range
 *
 * returns:
 *	random long in the range [0, s)
 */
long
irand(long s)
{
	ZVALUE z1, z2;
	long res;

	if (s <= 0) {
		math_error("Non-positive argument for irand()");
		/*NOTREACHED*/
	}
	if (s == 1)
		return 0;
	itoz(s, &z1);
	zrandrange(_zero_, z1, &z2);
	res = ztoi(z2);
	zfree(z1);
	zfree(z2);
	return res;
}


/*
 * randcopy - make a copy of an s100 state
 *
 * given:
 *	state - the state to copy
 *
 * returns:
 *	a malloced copy of the state
 */
RAND *
randcopy(CONST RAND *state)
{
	RAND *ret;	/* return copy of state */

	/*
	 * malloc state
	 */
	ret = (RAND *)malloc(sizeof(RAND));
	if (ret == NULL) {
		math_error("can't allocate RAND state");
		/*NOTREACHED*/
	}
	memcpy(ret, state, sizeof(RAND));

	/*
	 * return copy
	 */
	return ret;
}


/*
 * randfree - free an s100 state
 *
 * given:
 *	state - the state to free
 */
void
randfree(RAND *state)
{
	/* free it */
	free(state);
}


/*
 * randcmp - compare two s100 states
 *
 * given:
 *	s1 - first state to compare
 *	s2 - second state to compare
 *
 * return:
 *	TRUE if states differ
 */
BOOL
randcmp(CONST RAND *s1, CONST RAND *s2)
{
	/*
	 * assume uninitialized state == the default seeded state
	 */
	if (!s1->seeded) {
		if (!s2->seeded) {
			/* uninitialized == uninitialized */
			return FALSE;
		} else {
			/* uninitialized only equals default state */
			return randcmp(s2, &init_s100);
		}
	} else if (!s2->seeded) {
		/* uninitialized only equals default state */
		return randcmp(s1, &init_s100);
	}

	/* compare states */
	return (BOOL)(memcmp(s1, s2, sizeof(RAND)) != 0);
}


/*
 * randprint - print an s100 state
 *
 * given:
 *	state - state to print
 *	flags - print flags passed from printvalue() in value.c
 */
/*ARGSUSED*/
void
randprint(CONST RAND *UNUSED(state), int UNUSED(flags))
{
	math_str("RAND state");
}
