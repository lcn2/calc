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
 * Prior to calc 2.9.3t9, these routines existed as a calc library called
 * cryrand.cal.	 They have been rewritten in C for performance as well
 * as to make them available directly from libcalc.a.
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

/*
 * AN OVERVIEW OF THE FUNCTIONS:
 *
 * This module contains an Additive 55 shuffle generator wrapped inside
 * of a shuffle generator.
 *
 *	We refer to this generator as the a55 generator.
 *
 *	rand	- a55 shuffle generator
 *	srand	- seed the a55 shuffle generator
 *
 *	This generator has two distinct parts, the a55 generator
 *	and the shuffle generator.
 *
 *	The additive 55 generator is described in Knuth's "The Art of
 *	Computer Programming - Seminumerical Algorithms", Vol 2, 2nd edition
 *	(1981), Section 3.2.2, page 27, Algorithm A.
 *
 *	The period and other properties of this generator make it very
 *	useful to 'seed' other generators.
 *
 *	The shuffle generator is described in Knuth's "The Art of Computer
 *	Programming - Seminumerical Algorithms", Vol 2, 2nd edition (1981),
 *	Section 3.2.2, page 32, Algorithm B.
 *
 *	The shuffle generator is fast and serves as a fairly good standard
 *	pseudo-random generator.   If you need a fast generator and do not
 *	need a cryptographically strong one, this generator is likely to do
 *	the job.
 *
 *	The shuffle generator is feed values by the additive 55 process.
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
 * The additive 55 generator has a good period, and is fast.  It is
 * reasonable as generators go, though there are better ones available.
 * The shuffle generator has a very good period, and is fast.  It is
 * fairly good as generators go, particularly when it is feed reasonably
 * random numbers.  Because of this, we use feed values from the additive
 * 55 process into the shuffle generator.
 *
 * The a55 generator uses 2 tables:
 *
 *	additive table - 55 entries of 64 bits used by the additive 55
 *			 part of the a55 generator
 *
 *	shuffle table - 256 entries of 64 bits used by the shuffle
 *			part of the a55 generator and feed by the
 *			additive table.
 *
 * Casual direct use of the shuffle generator may be acceptable.  If one
 * desires cryptographically strong random numbers, or if one is paranoid,
 * one should use the Blum generator instead (see zrandom.c).
 *
 * The a55 generator as the following calc interfaces:
 *
 *   rand(min,max)		(where min < max)
 *
 *	Print an a55 generator random value over interval [a,b).
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
 * The a55 generator may be initialized and seeded via srand().
 *
 * Using a seed of '0' will reload generators with their initial states.
 *
 *	srand(0)	restore additive 55 generator to the initial state
 *
 * The above single arg calls are fairly fast.
 *
 * Optimal seed range for the a55 generator:
 *
 *	There is no limit on the size of a seed.  On the other hand,
 *	extremely large seeds require large tables and long seed times.
 *	Using a seed in the range of [2^64, 2^64 * 55!) should be
 *	sufficient for most purposes.  An easy way to stay within this
 *	range to to use seeds that are between 21 and 93 digits, or
 *	64 to 308 bits long.
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
 *    Seed the a55 generator.
 *
 *    seed != 0:
 *    ---------
 *	Any buffered random bits are flushed.  The additive table is loaded
 *	with the default additive table.  The low order 64 bits of seed is
 *	xor-ed against each table value.  The additive table is shuffled
 *	according to seed/2^64.
 *
 *	The following calc code produces the same effect:
 *
 *	    (* reload default additive table xored with low 64 seed bits *)
 *	    seed_xor = seed & ((1<<64)-1);
 *	    for (i=0; i < 55; ++i) {
 *		additive[i] = xor(default_additive[i], seed_xor);
 *	    }
 *
 *	    (* shuffle the additive table *)
 *	    seed >>= 64;
 *	    for (i=55; seed > 0 && i > 0; --i) {
 *		quomod(seed, i+1, seed, j);
 *		swap(additive[i], additive[j]);
 *	    }
 *
 *	Seed must be >= 0.  All seed values < 0 are reserved for future use.
 *
 *	The additive 55 pointers are reset to additive[23] and additive[54].
 *	Last the shuffle table is loaded with successive values from the
 *	additive 55 generator.
 *
 *    seed == 0:
 *    ---------
 *	Restore the initial state and modulus of the a55 generator.
 *	After this call, the a55 generator is restored to its initial
 *	state after calc started.
 *
 *	The additive 55 pointers are reset to additive[23] and additive[54].
 *	Last the shuffle table is loaded with successive values from the
 *	additive 55 generator.
 *
 ******************************************************************************
 *
 * srand(mat55)
 *
 *    Seed the a55 generator.
 *
 *    Any buffered random bits are flushed.  The additive table with the
 *    first 55 entries of the array mat55, mod 2^64.
 *
 *    The additive 55 pointers are reset to additive[23] and additive[54].
 *    Last the shuffle table is loaded with successive values from the
 *    additive 55 generator.
 *
 ******************************************************************************
 *
 * srand()
 *
 *    Return current a55 generator state.  This call does not alter
 *    the generator state.
 *
 ******************************************************************************
 *
 * srand(state)
 *
 *    Restore the a55 state and return the previous state.  Note that
 *    the argument state is a rand state value (isrand(state) is true).
 *    Any internally buffered random bits are restored.
 *
 *    The states of the a55 generators can be saved by calling the seed
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
 * found in this library were generated.  Detains can be found in the
 * various functions, while a overview can be found in the "SOURCE FOR
 * MAGIC NUMBERS" section below.
 */

/*
 * SOURCE OF MAGIC NUMBERS:
 *
 * Most of the magic constants used on this file ultimately are
 * based on the Rand Book of Random Numbers.  The Rand Book of Random
 * Numbers contains 10^6 decimal digits, generated by a physical process.
 * This book, produced by the Rand corporation in the 1950's is considered
 * a standard against which other generators may be measured.
 *
 * The Rand Book of Random Numbers was read groups of 20 digits.
 * The first 55 groups < 2^64 were used to initialize init_a55.slot.
 * The size of 20 digits was used because 2^64 is 20 digits long.
 * The restriction of < 2^64 was used to prevent modulus biasing.
 * (see the note on  modulus biasing in rand()).
 *
 * The function, randreseed64(), uses 2 primes to scramble 64 bits
 * into 64 bits.  These primes were also extracted from the Rand
 * Book of Random Numbers.  See randreseed64() for details.
 *
 * The shuffle table size is longer than the 100 entries recommended
 * by Knuth.  We use a power of 2 shuffle table length so that the
 * shuffle process can select a table entry from a new additive 55
 * value by extracting its low order bits.  The value 256 is convenient
 * in that it is the size of a byte which allows for easy extraction.
 *
 * We use the upper byte of the additive 55 value to select the shuffle
 * table entry because it allows all of 64 bits to play a part in the
 * entry selection.  If we were to select a lower 8 bits in the 64 bit
 * value, carries that propagate above our 8 bits would not impact the
 * a55 generator output.
 *
 ******************************************************************************
 *
 * FOR THE PARANOID:
 *
 * The truly paranoid might suggest that my claims in the MAGIC NUMBERS
 * section are a lie intended to entrap people.	 Well they are not, but
 * you need not take my word for it.
 *
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
 * When using the a55 generator, one may select your own 55 additive
 * values by calling:
 *
 *	srand(mat55)
 *
 * and avoid using my magic numbers.  The randreseed64 process is NOT
 * applied to the matrix values. Of course, you must pick good additive
 * 55 values yourself!
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


#include "zrand.h"
#include "have_const.h"


/*
 * default a55 generator state
 *
 * This is the state of the a55 generator after initialization, or srand(0),
 * or srand(0) is called.  The init_a55 value is never changed, only copied.
 */
static CONST RAND init_a55 = {
	1,		/* seeded */
	0,		/* no buffered bits */
#if FULL_BITS == SBITS		/* buffer */
	{0},
#elif 2*FULL_BITS == SBITS
	{0, 0},
#else
   /\../\	BASEB is assumed to be 16 or 32		/\../\	 !!!
#endif
	3,		/* j */
	34,		/* k */
	/* NOTE: Due to a SunOS cc bug, don't put spaces in the SVAL call! */
	{		/* additive 55 table */
  SVAL(23a252f6,0bae4907), SVAL(4f4ad31f,3818af34), SVAL(689806c4,03319a17),
  SVAL(e6354a5d,cee6bf3b), SVAL(3893b3e6,e012a049), SVAL(e39d8c36,8506bdaf),
  SVAL(e16a0aa7,982d6183), SVAL(5a3826bd,fe1de81a), SVAL(ced972b3,bed35fcd),
  SVAL(8e903ee9,116918a8), SVAL(756befb5,7b299d56), SVAL(98cb4420,76a5f6c5),
  SVAL(d310cd35,0ba6974e), SVAL(d1b647f8,e6380d89), SVAL(b5ea8137,6f5ae1dd),
  SVAL(cf52ec42,dc76ca61), SVAL(ee2a9fd2,f2f53952), SVAL(02068a04,8cbb4a4f),
  SVAL(2795a393,91e5aa61), SVAL(50094e9e,362288a0), SVAL(557a341d,1a05658b),
  SVAL(0974456c,230c81d0), SVAL(e4b28a42,2c257199), SVAL(8bd702b6,6b28872c),
  SVAL(b8a8aeb0,8168eadc), SVAL(231435e7,032dce9f), SVAL(330005e6,cfcf3824),
  SVAL(ecf49311,56732afe), SVAL(fa2bed49,9a7244cd), SVAL(847c40e5,213a970a),
  SVAL(c685f1a0,b28f5cd8), SVAL(7b097038,1d22ad26), SVAL(ce00b094,05b5d608),
  SVAL(a3572534,b519bc02), SVAL(74cbc38f,e7cd0932), SVAL(165c0566,4f19607e),
  SVAL(a459c365,1778672b), SVAL(fd38ee3c,8cf74e30), SVAL(834380f7,447e23f3),
  SVAL(ec47e111,98d1f52d), SVAL(43089001,90fb27a0), SVAL(e0decf74,00327bc7),
  SVAL(912114f1,8ede5d9c), SVAL(32848cd9,b3f1e3fa), SVAL(37dbc32b,9b57118d),
  SVAL(6086ea0a,1f29cff6), SVAL(b8b022ea,7985e577), SVAL(b54f7270,d5a7acd4),
  SVAL(26514f0a,905fd370), SVAL(c6cc93de,0b040350), SVAL(e531c4a7,f0bccac2),
  SVAL(e7354f4b,9d73756e), SVAL(5101eb63,ba39250c), SVAL(d62ac371,46f578aa),
  SVAL(55c023d0,ece39b11)
	},
	/* NOTE: Due to a SunOS cc bug, don't put spaces in the SVAL call! */
	{		/* shuffle table */
  SVAL(04635952,eab4809d), SVAL(1e191dc1,e2d8859c), SVAL(f466739b,a78cc47e),
  SVAL(bcb4460e,4cc9518b), SVAL(bb371472,5471ebc2), SVAL(13dca6fe,88234953),
  SVAL(5c92d8e1,3cd3d726), SVAL(3680546b,49af9c38), SVAL(0db16bc0,b8a095f8),
  SVAL(7eebfd03,b38af9e6), SVAL(d9bf3100,0c89b56f), SVAL(9c8b73a7,143d7ea2),
  SVAL(c486b166,388f9925), SVAL(c4989f42,ae295fc5), SVAL(6d5578e9,1dd0107c),
  SVAL(a56f369e,3b91befa), SVAL(9751ca45,397d0ee8), SVAL(edc2461a,688373d7),
  SVAL(d357ea0d,4934c3c8), SVAL(6437bab1,dcaf0dbc), SVAL(132d70f5,d1676791),
  SVAL(bdbf7ed9,3e8f804b), SVAL(f22da061,3e19870b), SVAL(a813fb55,a103e02f),
  SVAL(3063a143,6c357c03), SVAL(36e17210,72a5e5bf), SVAL(4f52d0b2,244f26fa),
  SVAL(ffc8c6e8,a26881eb), SVAL(bc44fe26,6f2d154f), SVAL(a66945a3,cab1b17b),
  SVAL(2431ffa7,f9a6d522), SVAL(09e7d9c8,e1cf82eb), SVAL(6d0471fb,73dda812),
  SVAL(80e5d3c7,72108856), SVAL(cecfc4ca,eafe3c51), SVAL(9256bed4,c3b3c9b0),
  SVAL(3e5826a6,2674112f), SVAL(bd25eeb0,d169ae1d), SVAL(cd15809d,2abdf0a3),
  SVAL(c907ba35,357c1077), SVAL(d7e19227,1e7843ee), SVAL(12ccb231,683a3d9c),
  SVAL(9272228b,66b9cd03), SVAL(105612d6,300166b0), SVAL(c2659fe8,b8b93dae),
  SVAL(e10ac791,5abd4030), SVAL(0d347031,b03724a9), SVAL(1f8afbee,41e2a46d),
  SVAL(4db3ca0b,727008e0), SVAL(78412641,654ecdfd), SVAL(34077f49,5f96bcaf),
  SVAL(d862b34a,21fe900e), SVAL(708c906d,48e05f2b), SVAL(2da50dd6,b87027b0),
  SVAL(8c2c1537,34b0ec0d), SVAL(34c6fa96,56e9fca0), SVAL(54fa8fd2,557e6b5b),
  SVAL(43b9444d,cbdbeb78), SVAL(bc7d0cf6,ef31d376), SVAL(777c1298,c39f0111),
  SVAL(ba45eca2,52d4face), SVAL(80c4d889,367aac48), SVAL(40682e34,2b7f1f23),
  SVAL(7ab5ddbc,2c7e3e0a), SVAL(ffd1d0cb,259b823c), SVAL(a88ef5ca,f787f1c0),
  SVAL(2ee2327b,d7f14852), SVAL(02ded80c,5f03aa54), SVAL(81be8df3,7f930de2),
  SVAL(3a6af986,488e011f), SVAL(6e76f0d3,710dcf71), SVAL(6f335c6c,57f552d6),
  SVAL(008ef84b,d0bdb173), SVAL(65ca0c98,afee90cb), SVAL(748dcd88,0cb0746c),
  SVAL(d59310de,8a20a53f), SVAL(9eca466a,994cc07b), SVAL(ff621092,ee50abb4),
  SVAL(c79ef743,e2e6849c), SVAL(7e176b4e,dea584e3), SVAL(af229851,d7f4b3bc),
  SVAL(835a4ffb,83e5e3a9), SVAL(d82b7a32,c46711f9), SVAL(2cd18e93,b80d747a),
  SVAL(d40e537a,8321d92b), SVAL(b05e14df,2e57c12f), SVAL(3eaed45f,38b97f8b),
  SVAL(c1ff01cd,c95c136d), SVAL(c49f1815,3dec73ce), SVAL(8b4cd1c1,da300fc7),
  SVAL(09d2d16d,8752cac1), SVAL(f89e1348,79490bfd), SVAL(3deac73a,07e45a65),
  SVAL(0d7daed1,563d0fc6), SVAL(43bd97f1,61fa4e81), SVAL(d7b362f2,4413c62a),
  SVAL(bb5ba7fc,5fc22f5c), SVAL(c1545507,3eab1555), SVAL(1334eae2,8f051104),
  SVAL(44242ddc,384c4b90), SVAL(1b75c117,a34b414f), SVAL(7bab6105,2144f41a),
  SVAL(8ebe585a,99d7f743), SVAL(4e42c257,432dba53), SVAL(de0b32da,153d5ec8),
  SVAL(a8954cd1,6c47311b), SVAL(adf5c428,ac1f354d), SVAL(0f56d6d7,e22d1fa6),
  SVAL(2d071e69,a6c0d364), SVAL(53cb0c7b,179770a9), SVAL(b2de65e5,358f8183),
  SVAL(041d2824,2d731f17), SVAL(c7139449,4fc1cf21), SVAL(94a88729,b398e56f),
  SVAL(a44da12c,7bac758b), SVAL(8e54401c,d5f6d3f9), SVAL(3122ed68,64d26d77),
  SVAL(7f170293,64389eae), SVAL(3cb4df89,f5da5177), SVAL(c470e8e0,6387f60a),
  SVAL(33dbc78c,d1b80187), SVAL(38b503e9,5f441313), SVAL(fb7ceb54,d84cb651),
  SVAL(bfa9552d,87776847), SVAL(47e8a857,9ecb10e5), SVAL(b23488c4,d3081df2),
  SVAL(46e6bf5e,9c091900), SVAL(bbeaa048,307fe0cf), SVAL(271e619f,ee99a620),
  SVAL(87c2b86a,9bb58570), SVAL(19b73eba,c26cf0cf), SVAL(ba400782,3c9801ca),
  SVAL(7b0d7198,0f959fce), SVAL(565d4f9e,7cbe7bdf), SVAL(cc5a2da6,21d33f36),
  SVAL(8d2dcb2b,ed321284), SVAL(2bef9ccc,f02d14c4), SVAL(86213e5b,70864746),
  SVAL(3c28656b,9a3a9420), SVAL(011571e4,29e2ac8f), SVAL(0429215a,45ef31d8),
  SVAL(f18d3a44,6e49010e), SVAL(c61c29f1,f6cf3284), SVAL(8bb2ac5e,8dae42ef),
  SVAL(1ff558eb,8dc8f536), SVAL(ae20729a,02ff404c), SVAL(86f25365,4f3fdff6),
  SVAL(6f0db4a2,6cb6c7dc), SVAL(8c94b164,ba75ae74), SVAL(8072777b,57d49ff8),
  SVAL(9c244bd2,a79bbc34), SVAL(ef376f89,317a30e3), SVAL(fa0958f0,9def2868),
  SVAL(0eb1d637,6751c755), SVAL(03cd8309,bfc3b3d7), SVAL(635e696f,42165234),
  SVAL(2ddfe9c9,f44d120c), SVAL(d5a517b9,35e11043), SVAL(0a2d629f,73ad9b22),
  SVAL(0529947a,03d704e8), SVAL(3058053c,07fcb68b), SVAL(c7ad02e3,6e8c261c),
  SVAL(c996de5a,1ec52170), SVAL(a8149001,b6567332), SVAL(aa285c19,9455ec88),
  SVAL(7f38938b,5762c0b9), SVAL(914af350,1aa5319b), SVAL(f3033116,3feee3e5),
  SVAL(1ac9c585,241f2cb5), SVAL(e0760698,15e709ab), SVAL(8f69b200,ffd98088),
  SVAL(354c0ec2,aac19f4f), SVAL(70a43cd7,d2819fbc), SVAL(02d1097b,eca983fb),
  SVAL(5023953e,f13638f9), SVAL(53d12078,5f80f6bd), SVAL(e6d57683,6243535f),
  SVAL(826f3eba,278c9647), SVAL(2eb709cf,f42e3023), SVAL(d47d59bc,5940bf59),
  SVAL(32a70040,2adcbdea), SVAL(e30b0b31,43a4d534), SVAL(ab220fd1,61fa11b2),
  SVAL(2127ba90,8c88ce88), SVAL(96748ea2,03074cc5), SVAL(1d84c1c4,8230a4a6),
  SVAL(1d9e70f1,7eae53fe), SVAL(a8ed5b62,03e2b1da), SVAL(2c026757,b29f8c22),
  SVAL(d6879045,9580da58), SVAL(92575fa5,f109176c), SVAL(5c47a208,f829cb4f),
  SVAL(4dce413e,df126d62), SVAL(05bf43c5,b8ffb590), SVAL(a92a01e5,e0391fc1),
  SVAL(ae517d73,da451e60), SVAL(70c5cdcf,c5abc1c7), SVAL(57671d42,1174641f),
  SVAL(7eb5dd74,cd9d26d4), SVAL(3abf1e70,b1e821eb), SVAL(8e967932,18e649f7),
  SVAL(165c0566,4f19607e), SVAL(a459c365,1778672b), SVAL(fd38ee3c,8cf74e30),
  SVAL(834380f7,447e23f3), SVAL(ec47e111,98d1f52d), SVAL(43089001,90fb27a0),
  SVAL(e0decf74,00327bc7), SVAL(912114f1,8ede5d9c), SVAL(32848cd9,b3f1e3fa),
  SVAL(37dbc32b,9b57118d), SVAL(6086ea0a,1f29cff6), SVAL(b8b022ea,7985e577),
  SVAL(b54f7270,d5a7acd4), SVAL(26514f0a,905fd370), SVAL(c6cc93de,0b040350),
  SVAL(e531c4a7,f0bccac2), SVAL(e7354f4b,9d73756e), SVAL(5101eb63,ba39250c),
  SVAL(d62ac371,46f578aa), SVAL(55c023d0,ece39b11), SVAL(23a252f6,0bae4907),
  SVAL(4f4ad31f,3818af34), SVAL(689806c4,03319a17), SVAL(e6354a5d,cee6bf3b),
  SVAL(3893b3e6,e012a049), SVAL(e39d8c36,8506bdaf), SVAL(e16a0aa7,982d6183),
  SVAL(5a3826bd,fe1de81a), SVAL(ced972b3,bed35fcd), SVAL(8e903ee9,116918a8),
  SVAL(756befb5,7b299d56), SVAL(98cb4420,76a5f6c5), SVAL(d310cd35,0ba6974e),
  SVAL(d1b647f8,e6380d89), SVAL(b5ea8137,6f5ae1dd), SVAL(cf52ec42,dc76ca61),
  SVAL(ee2a9fd2,f2f53952), SVAL(02068a04,8cbb4a4f), SVAL(2795a393,91e5aa61),
  SVAL(50094e9e,362288a0), SVAL(557a341d,1a05658b), SVAL(0974456c,230c81d0),
  SVAL(e4b28a42,2c257199), SVAL(8bd702b6,6b28872c), SVAL(b8a8aeb0,8168eadc),
  SVAL(231435e7,032dce9f), SVAL(330005e6,cfcf3824), SVAL(ecf49311,56732afe),
  SVAL(fa2bed49,9a7244cd), SVAL(847c40e5,213a970a), SVAL(c685f1a0,b28f5cd8),
  SVAL(7b097038,1d22ad26), SVAL(ce00b094,05b5d608), SVAL(a3572534,b519bc02),
  SVAL(74cbc38f,e7cd0932)
	}
};

/*
 * default additive 55 table
 *
 * The additive 55 table in init_a55 has been processed 256 times in order
 * to preload the shuffle table.  The array below is the table before
 * this processing.  These values have came from the Rand Book of Random
 * Numbers.  See " SOURCE OF MAGIC NUMBERS" above.
 *
 * This array is never changed, only copied.
 */
static CONST FULL additive[SCNT] = {
 /* NOTE: Due to a SunOS cc bug, don't put spaces in the SVAL call! */
 SVAL(8c20e7e5,8c4caa12), SVAL(74e36781,06254c77), SVAL(8220c179,faf7f81d),
 SVAL(b1bf27df,ef95b553), SVAL(a8eaced0,37c8387d), SVAL(1c78f31a,d6da0de2),
 SVAL(30fbd3f5,529499ea), SVAL(bec61787,279b7382), SVAL(f26c9cd7,e907360e),
 SVAL(c7a3b243,6e54caa9), SVAL(c56bc944,a4fba0b4), SVAL(9a0b31be,9a3ed149),
 SVAL(64058973,199388ce), SVAL(d6c04cb7,3cc1bc11), SVAL(ea18e829,beb6447b),
 SVAL(3e5c3521,ce8fc4e0), SVAL(b4b4c4e9,0cd2ebaa), SVAL(dd713b28,f6b87567),
 SVAL(18685941,e53d4031), SVAL(1560704f,c6d789a8), SVAL(4a0a3031,01a25097),
 SVAL(8a6866cd,c974215c), SVAL(1fdac9ac,989e4aaa), SVAL(d0721d52,6248e6fa),
 SVAL(91f835dc,568bdb8a), SVAL(7f830c1a,a1677807), SVAL(3a938494,51d1596e),
 SVAL(0977ec92,64dc366f), SVAL(6af1d82e,505b10d6), SVAL(4019e5c6,65f9c944),
 SVAL(05848075,f71b024e), SVAL(4eeb5439,91052276), SVAL(8c7f602b,ca83c3d8),
 SVAL(121b7ebc,9e34eac6), SVAL(d71faa62,6f41ddee), SVAL(2a7b7fa7,9e50c7dc),
 SVAL(609315cf,9495d6f7), SVAL(96952c31,e10e546b), SVAL(bb564e74,7cdb7a7f),
 SVAL(58f59523,6aed4a08), SVAL(390d8131,5bb0882d), SVAL(f5e6aee4,527c4e61),
 SVAL(4bcf616f,f771cd8b), SVAL(fdcd00a6,0a8fdde9), SVAL(73b54ea8,3ced2fb4),
 SVAL(67c53993,74a565af), SVAL(883931a9,08659585), SVAL(5ff183f1,09ec9509),
 SVAL(a4e93c34,1c1a0a35), SVAL(cfcfc497,82e7aef3), SVAL(c5354254,5097287d),
 SVAL(b2cd1194,0a50dee0), SVAL(3b776d75,7a56a0a5), SVAL(e41819e1,93ad0bde),
 SVAL(33f13c00,886b99a3)
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
 * current a55 generator state
 */
static RAND a55;


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
 * skip any of the 55 values that were used to initialize the additive 55
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
 *	 intended to be used internally by sa55rand() and sshufrand().
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
 * zsrand - seed the a55 generator
 *
 * given:
 *	pseed	- ptr to seed of the generator or NULL
 *	pmat55	- additive 55 state table or NULL
 *
 * returns:
 *	previous a55 state
 */
RAND *
zsrand(CONST ZVALUE *pseed, CONST MATRIX *pmat55)
{
	RAND *ret;		/* previous a55 state */
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
	if (!a55.seeded) {
		a55 = init_a55;
	}

	/*
	 * save the current state to return later
	 */
	ret = (RAND *)malloc(sizeof(RAND));
	if (ret == NULL) {
		math_error("cannot allocate RAND state");
		/*NOTREACHED*/
	}
	*ret = a55;

	/*
	 * if call was srand(), just return current state
	 */
	if (pseed == NULL && pmat55 == NULL) {
		return ret;
	}

	/*
	 * if call is srand(0), initialize and return quickly
	 */
	if (pmat55 == NULL && ziszero(*pseed)) {
		a55 = init_a55;
		return ret;
	}

	/*
	 * clear buffered bits, initialize pointers
	 */
	a55.seeded = 0; /* not seeded now */
	a55.j = INIT_J-1;
	a55.k = INIT_K-1;
	a55.bits = 0;
	memset(a55.buffer, 0, sizeof(a55.buffer));

	/*
	 * load additive table
	 *
	 * We will load the default additive table unless we are passed a
	 * matrix.  If we are passed a matrix, we will load the first 55
	 * values mod 2^64 instead.
	 */
	if (pmat55 == NULL) {
		memcpy(a55.slot, additive, sizeof(additive));
	} else {

		/*
		 * use the first 55 entries from the matrix arg
		 */
		if (pmat55->m_size < A55) {
			math_error("matrix for srand has < 55 elements");
			/*NOTREACHED*/
		}
		for (v=pmat55->m_table, i=0; i < A55; ++i, ++v) {

			/* reject if not integer */
			if (v->v_type != V_NUM || qisfrac(v->v_num)) {
			    math_error("matrix for srand must contain ints");
			    /*NOTREACHED*/
			}

			/* load table element from matrix element mod 2^64 */
			SLOAD(a55, i, v->v_num->num);
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
	 * xor additive table with the rehashed lower 64 bits of seed
	 */
	if (pseed != NULL && !ziszero(zscram)) {

		/* xor additive table with lower 64 bits of seed */
		SMOD64(shufxor, zscram);
		for (i=0; i < A55; ++i) {
			SXOR(a55, i, shufxor);
		}
	}

	/*
	 * shuffle additive 55 table according to seed, if passed
	 */
	if (pseed != NULL && zge64b(zscram)) {

		/* prepare the seed for additive slot shuffling */
		zshiftr(zscram, 64);
		ztrim(&zscram);

		/* shuffle additive table */
		for (i=A55-1; i > 0 && !zislezero(zscram); --i) {

			/* determine what we will swap with */
			indx = zdivi(zscram, i+1, &ztmp);
			zfree(zscram);
			zscram = ztmp;

			/* do nothing if swap with itself */
			if (indx == i) {
				continue;
			}

			/* swap slot[i] with slot[indx] */
			SSWAP(a55, i, indx);
		}
		zfree(zscram);
	} else if (pseed != NULL) {
		zfree(zscram);
	}

	/*
	 * load the shuffle table
	 *
	 * We will generate SHUFCNT entries from the additive 55 slots
	 * and fill the shuffle table in consecutive order.
	 */
	for (i=0; i < SHUFCNT; ++i) {

		/* bump j and k */
		if (++a55.j >= A55) {
			a55.j = 0;
		}
		if (++a55.k >= A55) {
			a55.k = 0;
		}

		/* slot[k] += slot[j] */
		SADD(a55, a55.k, a55.j);

		/* shuf[i] = slot[k] */
		SSHUF(a55, i, a55.k);
	}

	/*
	 * note that we are seeded
	 */
	a55.seeded = 1;

	/*
	 * return the previous state
	 */
	return ret;
}


/*
 * zsetrand - set the a55 generator state
 *
 * given:
 *	state - the state to copy
 *
 * returns:
 *	previous a55 state
 */
RAND *
zsetrand(CONST RAND *state)
{
	RAND *ret;		/* previous a55 state */

	/*
	 * initialize state if first call
	 */
	if (!a55.seeded) {
		a55 = init_a55;
	}

	/*
	 * save the current state to return later
	 */
	ret = randcopy(&a55);

	/*
	 * load the new state
	 */
	a55 = *state;

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
 * zrandskip - skip s a55 bits
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
	if (!a55.seeded) {
		a55 = init_a55;
	}

	/*
	 * skip required bits in the buffer
	 */
	if (a55.bits > 0 && a55.bits <= cnt) {

		/* just toss the buffer bits */
		cnt -= a55.bits;
		a55.bits = 0;
		memset(a55.buffer, 0, sizeof(a55.buffer));

	} else if (a55.bits > 0 && a55.bits > cnt) {

		/* buffer contains more bits than we need to toss */
#if FULL_BITS == SBITS
		a55.buffer[0] <<= cnt;
#else
		if (cnt >= FULL_BITS) {
			a55.buffer[SLEN-1] = (a55.buffer[0] << cnt);
			a55.buffer[0] = 0;
		} else {
			a55.buffer[SLEN-1] =
			    ((a55.buffer[SLEN-1] << cnt) |
			     (a55.buffer[0] >> (FULL_BITS-cnt)));
			a55.buffer[0] <<= cnt;
		}
#endif
		a55.bits -= cnt;
		return; /* skip need satisfied */
	}

	/*
	 * skip 64 bits at a time
	 */
	while (cnt >= SBITS) {

		/* bump j and k */
		if (++a55.j >= A55) {
			a55.j = 0;
		}
		if (++a55.k >= A55) {
			a55.k = 0;
		}

		/* slot[k] += slot[j] */
		SADD(a55, a55.k, a55.j);

		/* we will ignore the output value of a55.slot[indx] */
		indx = SINDX(a55, a55.k);
		cnt -= SBITS;

		/* store a55.k into a55.slot[indx] */
		SSHUF(a55, indx, a55.k);
	}

	/*
	 * skip the final bits
	 */
	if (cnt > 0) {

		/* bump j and k */
		if (++a55.j >= A55) {
			a55.j = 0;
		}
		if (++a55.k >= A55) {
			a55.k = 0;
		}

		/* slot[k] += slot[j] */
		SADD(a55, a55.k, a55.j);

		/* we will ignore the output value of a55.slot[indx] */
		indx = SINDX(a55, a55.k);

		/*
		 * We know the buffer is empty, so fill it
		 * with any unused bits.  Copy SBITS-trans bits
		 * from slot[indx] into buffer.
		 */
		a55.bits = (int)(SBITS-cnt);
		memcpy(a55.buffer, &a55.shuf[indx*SLEN],
		       sizeof(a55.buffer));

		/*
		 * shift the buffer bits all the way up to
		 * the most significant bit
		 */
#if FULL_BITS == SBITS
		a55.buffer[0] <<= cnt;
#else
		if (cnt >= FULL_BITS) {
			a55.buffer[SLEN-1] = (a55.buffer[0] << cnt);
			a55.buffer[0] = 0;
		} else {
			a55.buffer[SLEN-1] =
			    ((a55.buffer[SLEN-1] << cnt) |
			     (a55.buffer[0] >> (FULL_BITS-cnt)));
			a55.buffer[0] <<= cnt;
		}
#endif

		/* store a55.k into a55.slot[indx] */
		SSHUF(a55, indx, a55.k);
	}
}


/*
 * zrand - crank the a55 generator for some bits
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
	if (!a55.seeded) {
		a55 = init_a55;
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
	if (a55.bits > 0) {

		/*
		 * We know there are only a55.bits in the buffer, so
		 * transfer as much as we can (treating it as a slot)
		 * and return the bit transfer count.
		 */
		trans = slotcp(&dest, a55.buffer, a55.bits);

		/*
		 * If we need to keep bits in the buffer,
		 * shift the buffer bits all the way up to
		 * the most significant unused bit.
		 */
		if (trans < a55.bits) {
#if FULL_BITS == SBITS
			a55.buffer[0] <<= trans;
#else
			if (trans >= FULL_BITS) {
				a55.buffer[SLEN-1] =
				    (a55.buffer[0] << (trans-FULL_BITS));
				a55.buffer[0] = 0;
			} else {
				a55.buffer[SLEN-1] =
				    ((a55.buffer[SLEN-1] << trans) |
				     (a55.buffer[0] >> (FULL_BITS-trans)));
				a55.buffer[0] <<= trans;
			}
#endif
		}
		/* note that we have fewer bits in the buffer */
		a55.bits -= trans;
	}

	/*
	 * spin the generator until we need less than 64 bits
	 *
	 * The buffer did not contain enough bits, so we crank the
	 * a55 generator and load then 64 bits at a time.
	 */
	while (dest.len >= SBITS) {

		/* bump j and k */
		if (++a55.j >= A55) {
			a55.j = 0;
		}
		if (++a55.k >= A55) {
			a55.k = 0;
		}

		/* slot[k] += slot[j] */
		SADD(a55, a55.k, a55.j);

		/* select slot index to output */
		indx = SINDX(a55, a55.k);

		/* move up to 64 bits from slot[indx] to dest */
		slotcp64(&dest, &a55.shuf[indx*SLEN]);

		/* store a55.k into a55.slot[indx] */
		SSHUF(a55, indx, a55.k);
	}

	/*
	 * spin the generator one last time to fill out the remaining need
	 */
	if (dest.len > 0) {

		/* bump j and k */
		if (++a55.j >= A55) {
			a55.j = 0;
		}
		if (++a55.k >= A55) {
			a55.k = 0;
		}

		/* slot[k] += slot[j] */
		SADD(a55, a55.k, a55.j);

		/* select slot index to output */
		indx = SINDX(a55, a55.k);

		/* move up to 64 bits from slot[indx] to dest */
		trans = slotcp(&dest, &a55.shuf[indx*SLEN], dest.len);

		/* buffer up unused bits if we are done */
		if (trans != SBITS) {

			/*
			 * We know the buffer is empty, so fill it
			 * with any unused bits.  Copy SBITS-trans bits
			 * from slot[indx] into buffer.
			 */
			a55.bits = SBITS-trans;
			memcpy(a55.buffer, &a55.shuf[indx*SLEN],
			       sizeof(a55.buffer));

			/*
			 * shift the buffer bits all the way up to
			 * the most significant bit
			 */
#if FULL_BITS == SBITS
			a55.buffer[0] <<= trans;
#else
			if (trans >= FULL_BITS) {
				a55.buffer[SLEN-1] =
				    (a55.buffer[0] << (trans-FULL_BITS));
				a55.buffer[0] = 0;
			} else {
				a55.buffer[SLEN-1] =
				    ((a55.buffer[SLEN-1] << trans) |
				     (a55.buffer[0] >> (FULL_BITS-trans)));
				a55.buffer[0] <<= trans;
			}
#endif
		}

		/* store a55.k into a55.slot[indx] */
		SSHUF(a55, indx, a55.k);
	}
	res->sign = 0;
	ztrim(res);
}


/*
 * zrandrange - generate an a55 random value in the range [low, high)
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
 * irand - generate an a55 random long in the range [0, s)
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
 * randcopy - make a copy of an a55 state
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
	*ret = *state;

	/*
	 * return copy
	 */
	return ret;
}


/*
 * randfree - free an a55 state
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
 * randcmp - compare two a55 states
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
			return randcmp(s2, &init_a55);
		}
	} else if (!s2->seeded) {
		/* uninitialized only equals default state */
		return randcmp(s1, &init_a55);
	}

	/* compare states */
	return (BOOL)(memcmp(s1, s2, sizeof(RAND)) != 0);
}


/*
 * randprint - print an a55 state
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
