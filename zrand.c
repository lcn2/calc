/*
 * zrand - subtractive 100 shuffle generator
 *
 * Copyright (C) 1999  Landon Curt Noll
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.5 $
 * @(#) $Id: zrand.c,v 29.5 2003/01/14 00:54:24 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/zrand.c,v $
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
 *   rand(min,max)		(where min < max)
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
 * based on LavaRnd.  LavaRnd produced them via a cryprographic
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
 * if you that paranoid why would you use a non-cryprographically strong
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

#include "zrand.h"
#include "have_const.h"


/*
 * default s100 generator state
 *
 * This is the state of the s100 generator after initialization, or srand(0),
 * or srand(0) is called.  The init_s100 value is never changed, only copied.
 */
static CONST RAND init_s100 = {
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
	/* NOTE: Due to a SunOS cc bug, don't put spaces in the SVAL call! */
	{		/* subtractive 100 table */
	    SVAL(c8c0370c,7db7dc19), SVAL(738e33b9,40a06fbb),
	    SVAL(481abb76,a859ed2b), SVAL(74106bb3,9ccdccb5),
	    SVAL(05a8eeb5,c3173bfc), SVAL(efd5100d,5a02e577),
	    SVAL(a69271f7,4030b24a), SVAL(641282fc,16fe22c5),
	    SVAL(7aa7267c,40438da3), SVAL(1fdf4abd,c2d878d1),
	    SVAL(d9899e7a,95702379), SVAL(5ea8e217,d02d7f08),
	    SVAL(770587fe,4d47a353), SVAL(de7d1bdd,0a33a2b8),
	    SVAL(4378c3c5,900e7c45), SVAL(77c94478,19a514f9),
	    SVAL(fc5edb22,843d1d32), SVAL(4fc42ce5,e8ee5e6e),
	    SVAL(c938713c,8488013e), SVAL(6a318f03,20ab0cac),
	    SVAL(73e6d1a3,ffc8bff3), SVAL(0cd3232a,8ca96aa7),
	    SVAL(605c8036,905f770d), SVAL(4d037b00,8b8d04a2),
	    SVAL(1ed81965,cb277294), SVAL(408d9c47,7a254ff3),
	    SVAL(8b68587a,e26c7377), SVAL(cff191a4,8a48832f),
	    SVAL(12d3df1d,8aeb6fe6), SVAL(b2bf907e,1feda37a),
	    SVAL(4e5f7719,3bb5f39f), SVAL(33ebcf6f,8f5d1581),
	    SVAL(203c8e48,d33654eb), SVAL(68d3656e,f19c8a4e),
	    SVAL(3ec20b04,986eb2af), SVAL(5d73a03b,062c3841),
	    SVAL(836ce709,5d4e49eb), SVAL(2310bc40,c3f49221),
	    SVAL(3868ee48,a6d0cbf6), SVAL(67578aa6,4a43deb1),
	    SVAL(6e3426c1,150dfc26), SVAL(c541ccaa,3131be30),
	    SVAL(f7e57432,cec7aab2), SVAL(2b35de99,8cb3c873),
	    SVAL(7b9f7764,8663a5d7), SVAL(23b00e6a,a771e5a6),
	    SVAL(859c775c,a9985d05), SVAL(99636ea1,6b692f1f),
	    SVAL(8700ac70,3730800d), SVAL(46142502,4298a753),
	    SVAL(ea4a411b,809e955f), SVAL(3119ad40,33709dfb),
	    SVAL(b76a6c6e,5f01cb7c), SVAL(6109dc8a,15984eaf),
	    SVAL(5d686db9,a5ca9505), SVAL(8e80d761,3b7e6add),
	    SVAL(79cbd718,de6f6fd3), SVAL(40e9cd15,1da0f699),
	    SVAL(e82158ba,b24f312d), SVAL(79a4c927,f5e5c36b),
	    SVAL(c25247c9,a0039333), SVAL(93687116,1766d81d),
	    SVAL(3c6a03b4,a6741327), SVAL(c8a7b6e8,c002f29a),
	    SVAL(0e2a67c6,7bbd5ea3), SVAL(0929042d,441eabc1),
	    SVAL(7dbe232a,25e82085), SVAL(8cfb26e5,44fbac3d),
	    SVAL(8e40384d,388ab983), SVAL(48dc1230,554632f8),
	    SVAL(ab405048,ab492397), SVAL(21c9e2f5,a118e387),
	    SVAL(484d1a8c,343b61b5), SVAL(d49e3dec,ab256f26),
	    SVAL(e615c7fd,78f2d2e3), SVAL(8442cc33,ce6cc2ed),
	    SVAL(0a3b93d8,44d4bbf6), SVAL(2d7e4efe,9301de77),
	    SVAL(33711b76,d8790d8a), SVAL(c07dc30e,44df77e7),
	    SVAL(b9132ed0,9ddd508f), SVAL(45d06cf8,c6fb43cc),
	    SVAL(22bed18a,d585dd7b), SVAL(61c6cced,10799ffa),
	    SVAL(d7f2393b,e4bd9aa9), SVAL(706753fb,cfd55094),
	    SVAL(f65a6713,ede6e446), SVAL(8bf6dfae,47c0d5c3),
	    SVAL(fb4dfc17,9f7927d6), SVAL(12ebbc16,e212c297),
	    SVAL(43c71283,a00a954c), SVAL(8957087a,e7bd40a5),
	    SVAL(b0859d71,08344837), SVAL(fbf4b9a3,aeb313f5),
	    SVAL(5e66e5be,ce81823a), SVAL(09a11c6e,58ad6da1),
	    SVAL(c76f4316,c608054f), SVAL(b5821361,46084099),
	    SVAL(4210008f,17a725ed), SVAL(e5ff8912,d347c481)
	},
	/* NOTE: Due to a SunOS cc bug, don't put spaces in the SVAL call! */
	{		/* shuffle table */
	    SVAL(69a2296c,ec8abd57), SVAL(867e1869,99a6df81),
	    SVAL(c05ab96b,d849a48a), SVAL(7eb3ce0c,fa00554b),
	    SVAL(520d01f6,5a5a9acd), SVAL(d4ef1e33,36022d81),
	    SVAL(af44772b,c6f84f70), SVAL(647e85a6,a7c55173),
	    SVAL(26746cf1,959df8d1), SVAL(98681a90,4db28abd),
	    SVAL(b146c969,744c5cd2), SVAL(8ce69d1f,706f88c2),
	    SVAL(fd12eac4,21b4a748), SVAL(f12e70fe,2710eea5),
	    SVAL(0b8f7805,5901f2b5), SVAL(48860a76,4f2c115e),
	    SVAL(0edf6d2a,30767e2c), SVAL(8a6d7dc5,fce2713b),
	    SVAL(46a362ea,4e0e2346), SVAL(6c369a0a,359f5aa7),
	    SVAL(dfca81fe,41def54e), SVAL(4b733819,96c2bc4e),
	    SVAL(659e8b99,6f3f14f9), SVAL(8b97b934,93d47e6f),
	    SVAL(a73a8704,dfa10a55), SVAL(8d9eafe9,b06503da),
	    SVAL(2556fb88,f32336b0), SVAL(e71e9f75,1002a161),
	    SVAL(27a7be6e,200af907), SVAL(1b9b734e,d028e9a3),
	    SVAL(950cfeed,4c0be0d3), SVAL(f4c41694,2536d275),
	    SVAL(f05a58e8,5687b76e), SVAL(ba53ac01,71a62d54),
	    SVAL(4b14cbcb,285adc96), SVAL(fdf66edd,b00a5557),
	    SVAL(bb43d58d,185b6ea1), SVAL(905db9cd,f355c9a6),
	    SVAL(fc3a07fc,04429c8a), SVAL(65d7e365,aa3a4f7e),
	    SVAL(2d284c18,b243ac65), SVAL(72fba65d,44e417fd),
	    SVAL(422d50b4,5c934805), SVAL(b62a6053,d1587441),
	    SVAL(a5e71ce9,6f7ae035), SVAL(93abca2e,595c8dd8),
	    SVAL(534231af,e39afad5), SVAL(08d26cac,12eaad56),
	    SVAL(ec18bf8d,7fb1b1c2), SVAL(3d28ea16,faf6f09b),
	    SVAL(ea357a78,16697dd6), SVAL(51471ea1,420f3f51),
	    SVAL(5e051aeb,7f8946b4), SVAL(881be097,0cf0524c),
	    SVAL(d558b25b,1b31489e), SVAL(707d1a94,3a8b065c),
	    SVAL(37017e66,568ff836), SVAL(b9cd627c,24c2f747),
	    SVAL(1485549f,fb1d9ff6), SVAL(308d32d9,bdf2dc6f),
	    SVAL(4d4142ca,d543818a), SVAL(5d9c7aee,87ebba43),
	    SVAL(81c5bdd8,e17adb2f), SVAL(3dc9752e,c8d8677a),
	    SVAL(66b086e6,c34e4212), SVAL(3af7a90d,c62b25e3),
	    SVAL(f8349f79,35539315), SVAL(6bcfd9d5,a22917f0),
	    SVAL(8639bb76,5f5ee517), SVAL(d3c5e369,8095b092),
	    SVAL(8a33851e,7eb44748), SVAL(5e29d443,ea54bbcf),
	    SVAL(0f84651f,4d59a834), SVAL(85040bea,f1a5f951),
	    SVAL(3dba1c74,98002078), SVAL(5d70712b,f0b2cc15),
	    SVAL(fa3af8eb,cce8e5a7), SVAL(fb3e2237,04bba57d),
	    SVAL(5d3b8785,8a950434), SVAL(ce3112bd,ba3f8dcf),
	    SVAL(44904f55,860d3051), SVAL(cec8fed4,4ed3e98b),
	    SVAL(4581698d,25d01ea4), SVAL(11eb6828,9a9548e0),
	    SVAL(796cb4c6,e911fac8), SVAL(2164cf26,b5fd813e),
	    SVAL(4ac8e0f5,d5de640f), SVAL(e9e757d7,8802ab4e),
	    SVAL(3c97de26,f49dfcbd), SVAL(c604881b,6ee6dbe6),
	    SVAL(a7c22a6e,57d6154e), SVAL(234e2370,877b3cc7),
	    SVAL(c0bdb72b,df1f8358), SVAL(6522e0fc,a95b7b55),
	    SVAL(ba174c90,22344162), SVAL(712c9b2d,75d48867),
	    SVAL(240f7e92,e59f3700), SVAL(e83cc2d4,ad95d763),
	    SVAL(8509445a,4336d717), SVAL(f1e572c5,dfff1804),
	    SVAL(ed10eb5d,623232dd), SVAL(9205ea1b,d4f957e8),
	    SVAL(4973a54f,2ff062f5), SVAL(26b018f1,e8c48cd5),
	    SVAL(56908401,d1c7ed9f), SVAL(2e48937b,df89a247),
	    SVAL(9d53069b,2be47129), SVAL(98069e3b,c048a2b0),
	    SVAL(f25b7d65,1cd83f93), SVAL(2b004e6c,e6f886c8),
	    SVAL(f618442a,5c635935), SVAL(a502ab5c,7198e052),
	    SVAL(c14241a4,a6c41b0b), SVAL(720e845a,7db9b18e),
	    SVAL(2abb13e9,4b713918), SVAL(90fc0c20,7f52467d),
	    SVAL(799c8ccd,7868d348), SVAL(f4817ced,912a0ea4),
	    SVAL(d68c0f4c,c4903a57), SVAL(a3171f29,e2b7934c),
	    SVAL(b1158baa,0b4ccc22), SVAL(f5d85553,49a29eda),
	    SVAL(59d1a078,959442ef), SVAL(db9b4a96,a67fd518),
	    SVAL(cc7ca9ee,d2870636), SVAL(548f021c,ecf59920),
	    SVAL(25b7f4b6,571bc8c5), SVAL(4fa52747,3a44f536),
	    SVAL(b246845f,df0ebdc2), SVAL(dd8d68ae,42058793),
	    SVAL(3ba13328,9f6c39fb), SVAL(8bfdfbf3,7b6b42af),
	    SVAL(fb34c5ca,7fb2b3b0), SVAL(2345dcec,d428e32a),
	    SVAL(6891e850,ad42b63e), SVAL(930642c8,362c1381),
	    SVAL(13871e9b,1886aff5), SVAL(d0cf2407,482bda55),
	    SVAL(125b5fc9,5069bc31), SVAL(9b71d0a9,f07dfa5d),
	    SVAL(55c044cc,6712e524), SVAL(f0377358,bb601978),
	    SVAL(152ad5f8,7fa51e8b), SVAL(e5ebf478,9fcdd9af),
	    SVAL(3d78e18c,66ebce7e), SVAL(8246db72,f36aa83f),
	    SVAL(cc6ddc6d,2c64c0a3), SVAL(a758d687,0d91851e),
	    SVAL(24b20a6f,9488ee36), SVAL(be11ccdf,09798197),
	    SVAL(11aca015,99c1f4e3), SVAL(40e89e36,6437ac05),
	    SVAL(c8bfc762,5af675f8), SVAL(6367c578,b577e759),
	    SVAL(00380346,615f0b74), SVAL(ee964cc4,8de07d81),
	    SVAL(17f6ac16,859d9261), SVAL(092f4a17,3a6e2f6c),
	    SVAL(79981a3d,b9024b95), SVAL(36db1660,04f7f540),
	    SVAL(c36252cf,65a2f1c8), SVAL(705b6fde,124c9bd2),
	    SVAL(31e58dda,85db40ce), SVAL(6342b1a5,9f5e8d6d),
	    SVAL(5c2c67d0,bd6d1d4d), SVAL(1fe5b46f,ba7e069d),
	    SVAL(21c46c6c,ac72e13c), SVAL(b80c5fd5,9eb8f52a),
	    SVAL(56c3aebf,a74c92bc), SVAL(c1aff1fc,bf8c4196),
	    SVAL(2b1df645,754ad208), SVAL(5c734600,d46eeb50),
	    SVAL(e0ff1b12,6a70a765), SVAL(d5416497,7a94547c),
	    SVAL(67b59d7c,4ea35206), SVAL(53be7146,779203b4),
	    SVAL(6b589fe5,414026b8), SVAL(9e81016c,3083bfee),
	    SVAL(b23526b9,3b4b7671), SVAL(4fa9ffb1,7ee300ba),
	    SVAL(6217e212,ad05fb21), SVAL(f5b3fcd3,b294e6c2),
	    SVAL(ac040bbe,216beb2a), SVAL(1f8d8a54,71d0e78c),
	    SVAL(b6d15b41,9cfec96b), SVAL(c5477845,d0508c78),
	    SVAL(5b486e81,b4bba621), SVAL(90c35c94,ef4c4121),
	    SVAL(efce7346,f6a6bc55), SVAL(a27828d9,25bdb9bb),
	    SVAL(e3a53095,a1f0b205), SVAL(1bfa6093,d9f208ab),
	    SVAL(fb078f6a,6842cdf4), SVAL(07806d72,97133a38),
	    SVAL(2c6c901b,a3ce9592), SVAL(1f0ab2cf,ebc1b789),
	    SVAL(2ce81415,e2d03d5e), SVAL(7da45d5b,aa9f2417),
	    SVAL(3be4f76d,dd800682), SVAL(dbf4e4a3,364d72d3),
	    SVAL(b538cccf,4fc59da5), SVAL(b0aa39d5,487f66ec),
	    SVAL(2fd28dfd,87927d3d), SVAL(d14e77f0,5900c6b1),
	    SVAL(2523fad2,5330c7b4), SVAL(991b5938,d82368a4),
	    SVAL(b7c11443,2b9c1302), SVAL(db842db6,1394b116),
	    SVAL(3641548d,78ed26d8), SVAL(274fa8ef,0a61dacf),
	    SVAL(a554ba63,112df6f1), SVAL(7b7fe985,6b50438d),
	    SVAL(c9fa0042,bb63bbad), SVAL(3abf45d0,e27f00da),
	    SVAL(d95faa15,9f87aabb), SVAL(4a95012e,3488e7ae),
	    SVAL(1be2bdb9,0c642d04), SVAL(145c8881,8b4abf3e),
	    SVAL(7f9fb635,544cf17f), SVAL(b8ab2f62,cc78db70),
	    SVAL(8ee64bcd,b4242f9a), SVAL(abd52858,95dad129),
	    SVAL(be722c2f,ccf31141), SVAL(7c330703,575e26a9),
	    SVAL(45d3e3b3,361b79e4), SVAL(241163a7,54b2e6a6),
	    SVAL(8f678d7d,f7cacb77), SVAL(988a68a4,83211d19),
	    SVAL(79599598,ba7836f6), SVAL(4850c887,eeda68bf),
	    SVAL(afa69a71,8052ce25), SVAL(8b21efc6,bdd73573),
	    SVAL(89dbae18,d0972493), SVAL(560776bf,537d9454),
	    SVAL(3c009f78,165310f2), SVAL(a3680021,0160c3af),
	    SVAL(3353ec3c,a643bd40), SVAL(7e593f99,911dab02),
	    SVAL(72d1ddd9,4f676e89), SVAL(fd18b8bd,6b43c0ea),
	    SVAL(43cacef2,ddbd697d), SVAL(2868a4d0,acefe884),
	    SVAL(5f377b63,a506f013), SVAL(eaa0975e,05ca662b),
	    SVAL(3740e6b8,eb433931), SVAL(ce85df00,08557948),
	    SVAL(784745fb,547e33f9), SVAL(4a1fc5d4,e5c6f598),
	    SVAL(85fa6fec,768430a7), SVAL(990d0c24,d2332a51),
	    SVAL(55245c2c,33b676d5), SVAL(b1091519,e2bcfa71),
	    SVAL(38521478,d23a28d8), SVAL(9b794f89,9a573010),
	    SVAL(61d225e8,699bb486), SVAL(21476d24,1c2158b0)
	}
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
static CONST FULL def_subtract[SCNT] = {
 /* NOTE: Due to a SunOS cc bug, don't put spaces in the SVAL call! */
	    SVAL(c8c0370c,7db7dc19), SVAL(738e33b9,40a06fbb),
	    SVAL(481abb76,a859ed2b), SVAL(74106bb3,9ccdccb5),
	    SVAL(05a8eeb5,c3173bfc), SVAL(efd5100d,5a02e577),
	    SVAL(a69271f7,4030b24a), SVAL(641282fc,16fe22c5),
	    SVAL(7aa7267c,40438da3), SVAL(1fdf4abd,c2d878d1),
	    SVAL(d9899e7a,95702379), SVAL(5ea8e217,d02d7f08),
	    SVAL(770587fe,4d47a353), SVAL(de7d1bdd,0a33a2b8),
	    SVAL(4378c3c5,900e7c45), SVAL(77c94478,19a514f9),
	    SVAL(fc5edb22,843d1d32), SVAL(4fc42ce5,e8ee5e6e),
	    SVAL(c938713c,8488013e), SVAL(6a318f03,20ab0cac),
	    SVAL(73e6d1a3,ffc8bff3), SVAL(0cd3232a,8ca96aa7),
	    SVAL(605c8036,905f770d), SVAL(4d037b00,8b8d04a2),
	    SVAL(1ed81965,cb277294), SVAL(408d9c47,7a254ff3),
	    SVAL(8b68587a,e26c7377), SVAL(cff191a4,8a48832f),
	    SVAL(12d3df1d,8aeb6fe6), SVAL(b2bf907e,1feda37a),
	    SVAL(4e5f7719,3bb5f39f), SVAL(33ebcf6f,8f5d1581),
	    SVAL(203c8e48,d33654eb), SVAL(68d3656e,f19c8a4e),
	    SVAL(3ec20b04,986eb2af), SVAL(5d73a03b,062c3841),
	    SVAL(836ce709,5d4e49eb), SVAL(2310bc40,c3f49221),
	    SVAL(3868ee48,a6d0cbf6), SVAL(67578aa6,4a43deb1),
	    SVAL(6e3426c1,150dfc26), SVAL(c541ccaa,3131be30),
	    SVAL(f7e57432,cec7aab2), SVAL(2b35de99,8cb3c873),
	    SVAL(7b9f7764,8663a5d7), SVAL(23b00e6a,a771e5a6),
	    SVAL(859c775c,a9985d05), SVAL(99636ea1,6b692f1f),
	    SVAL(8700ac70,3730800d), SVAL(46142502,4298a753),
	    SVAL(ea4a411b,809e955f), SVAL(3119ad40,33709dfb),
	    SVAL(b76a6c6e,5f01cb7c), SVAL(6109dc8a,15984eaf),
	    SVAL(5d686db9,a5ca9505), SVAL(8e80d761,3b7e6add),
	    SVAL(79cbd718,de6f6fd3), SVAL(40e9cd15,1da0f699),
	    SVAL(e82158ba,b24f312d), SVAL(79a4c927,f5e5c36b),
	    SVAL(c25247c9,a0039333), SVAL(93687116,1766d81d),
	    SVAL(3c6a03b4,a6741327), SVAL(c8a7b6e8,c002f29a),
	    SVAL(0e2a67c6,7bbd5ea3), SVAL(0929042d,441eabc1),
	    SVAL(7dbe232a,25e82085), SVAL(8cfb26e5,44fbac3d),
	    SVAL(8e40384d,388ab983), SVAL(48dc1230,554632f8),
	    SVAL(ab405048,ab492397), SVAL(21c9e2f5,a118e387),
	    SVAL(484d1a8c,343b61b5), SVAL(d49e3dec,ab256f26),
	    SVAL(e615c7fd,78f2d2e3), SVAL(8442cc33,ce6cc2ed),
	    SVAL(0a3b93d8,44d4bbf6), SVAL(2d7e4efe,9301de77),
	    SVAL(33711b76,d8790d8a), SVAL(c07dc30e,44df77e7),
	    SVAL(b9132ed0,9ddd508f), SVAL(45d06cf8,c6fb43cc),
	    SVAL(22bed18a,d585dd7b), SVAL(61c6cced,10799ffa),
	    SVAL(d7f2393b,e4bd9aa9), SVAL(706753fb,cfd55094),
	    SVAL(f65a6713,ede6e446), SVAL(8bf6dfae,47c0d5c3),
	    SVAL(fb4dfc17,9f7927d6), SVAL(12ebbc16,e212c297),
	    SVAL(43c71283,a00a954c), SVAL(8957087a,e7bd40a5),
	    SVAL(b0859d71,08344837), SVAL(fbf4b9a3,aeb313f5),
	    SVAL(5e66e5be,ce81823a), SVAL(09a11c6e,58ad6da1),
	    SVAL(c76f4316,c608054f), SVAL(b5821361,46084099),
	    SVAL(4210008f,17a725ed), SVAL(e5ff8912,d347c481)
};


/*
 * Linear Congruential Constants
 *
 *	a = 6316878969928993981 = 0x57aa0ff473c0ccbd
 *	c = 1363042948800878693 = 0x12ea805718e09865
 *
 * These constants are used in the randreseed64().  See below.
 */
/* NOTE: Due to a SunOS cc bug, don't put spaces in the SHVAL call! */
static CONST HALF a_vec[SHALFS] = {SHVAL(57aa,0ff4,73c0,ccbd)};
static CONST HALF c_vec[SHALFS] = {SHVAL(12ea,8057,18e0,9865)};
static CONST ZVALUE a_val = {(HALF *)a_vec, SHALFS, 0};
static CONST ZVALUE c_val = {(HALF *)c_vec, SHALFS, 0};


/*
 * current s100 generator state
 */
static RAND s100;


/*
 * declare static functions
 */
static void randreseed64(ZVALUE seed, ZVALUE *res);
static int slotcp(BITSTR *bitstr, FULL *src, int count);
static void slotcp64(BITSTR *bitstr, FULL *src);


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
static void
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
		if (chunk.len > SHALFS) {
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

		/* no skip, just descrment use counter */
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
 * be transfered to the most significant bit in the destination.
 *
 * given:
 *	bitstr	- most significant destination bit in a bit string
 *	src	- low order FULL in a 64 bit slot
 *	count	- number of bits to transfer (must be 0 < count <= 64)
 *
 * returns:
 *	number of bits transfered
 */
static int
slotcp(BITSTR *bitstr, FULL *src, int count)
{
	HALF *dh;		/* most significant HALF in dest */
	int dnxtbit;		/* next bit beyond most signif in dh */
	int need;		/* number of bits we need to transfer */
	int ret;		/* bits transfered */

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
		/* load the remaining bits from the most signif FULL */
		*dh-- = ((((HALF)src[SLEN-1] & lowhalf[BASEB-dnxtbit])
			   << dnxtbit) | (HALF)(src[0] >> (FULL_BITS-dnxtbit)));
		need -= BASEB;
	} else if (need > 0) {
		/* load the remaining bits from the most signif FULL */
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
		/* no need todo: need -= BASEB, because we are nearly done */
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
 * be transfered to the most significant bit in the destination.
 *
 * given:
 *	bitstr	- most significant destination bit in a bit string
 *	src	- low order FULL in a 64 bit slot
 *
 * returns:
 *	number of bits transfered
 */
static void
slotcp64(BITSTR *bitstr, FULL *src)
{
	HALF *dh = bitstr->loc;		/* most significant HALF in dest */
	int dnxtbit = bitstr->bit+1;	/* next bit beyond most signif in dh */

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
	/* load the remaining bits from the most signif FULL */
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

		/* no skip, just descrment use counter */
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

		/* no skip, just descrment use counter */
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
	int trans;		/* bits transfered */
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

		/* no skip, just descrment use counter */
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

		/* no skip, just descrment use counter */
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
 * zrandrange - generate an s100 random value in the range [low, high)
 *
 * given:
 *	low - low value of range
 *	high - beyond end of range
 *	res - where to place the random bits as ZVALUE
 */
void
zrandrange(CONST ZVALUE low, CONST ZVALUE high, ZVALUE *res)
{
	ZVALUE range;		/* high-low */
	ZVALUE rval;		/* random value [0, 2^bitlen) */
	ZVALUE rangem1;		/* range - 1 */
	long bitlen;		/* smallest power of 2 >= diff */

	/*
	 * firewall
	 */
	if (zrel(low, high) >= 0) {
		math_error("srand low range >= high range");
		/*NOTREACHED*/
	}

	/*
	 * determine the size of the random number needed
	 */
	zsub(high, low, &range);
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
	 * which is the range [low, high)
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
randprint(CONST RAND *state, int flags)
{
	math_str("RAND state");
}
