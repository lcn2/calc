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
 * Prior to calc 2.9.3t9, these routines existed as a calc library called
 * cryrand.cal.  They have been rewritten in C for performance as well
 * as to make them available directly from libcalc.a.
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

/*
 * XXX - Add shs() and md5 hash functions.  Ensure that any object can
 *	 be hashed.  Ensure that if a == b, hash of a == hash of b.
 *	 This can be done by hashing all values of an value that
 *	 are used in the equality test.  Note that the value type should
 *	 also be hashed to help distinguish different value types.
 *	 Also note that objects should hash their name.  The shs() and
 *	 md5() should NOT replace the foohash() functions used by
 *	 associative arrays as those functions need to be fast.
 *
 * XXX - write random() and srandom() help pages
 */

/*
 * AN OVERVIEW OF THE FUNCTIONS:
 *
 * This module contains two pseudo-random number generators:
 *
 *   Additive 55 shuffle generator:
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
 *   Blum-Blum-Shub generator:
 *
 *	We refer to this generator as the Blum generator.
 *
 *	This generator is described in the papers:
 *
 *	    Blum, Blum, and Shub, "Comparison of Two Pseudorandom Number
 *	    Generators", in Chaum, D. et. al., "Advances in Cryptology:
 *	    Proceedings Crypto 82", pp. 61-79, Plenum Press, 1983.
 *
 *	    Blum, Blum, and Shub, "A Simple Unpredictable Pseudo-Random
 *	    Number Generator", SIAM Journal of Computing, v. 15, n. 2,
 *	    1986, pp. 364-383.
 *
 *	    U. V. Vazirani and V. V. Vazirani, "Trapdoor Pseudo-Random
 *	    Number Generators with Applications to Protocol Design",
 *	    Proceedings of the 24th  IEEE Symposium on the Foundations
 *	    of Computer Science, 1983, pp. 23-30.
 *
 *	    U. V. Vazirani and V. V. Vazirani, "Efficient and Secure
 *	    Pseudo-Random Number Generation", Proceedings of the 24th
 *	    IEEE Symposium on the Foundations of Computer Science,
 *	    1984, pp. 458-463.
 *
 *	    U. V. Vazirani and V. V. Vazirani, "Efficient and Secure
 *	    Pseudo-Random Number Generation", Advances in Cryptology -
 *	    Proceedings of CRYPTO '84, Berlin: Springer-Verlag, 1985,
 *	    pp. 193-202.
 *
 *	    Sciences 28, pp. 270-299.
 *
 *	    Bruce Schneier, "Applied Cryptography", John Wiley & Sons,
 *	    1st edition (1994), pp 365-366.
 *
 *	This generator is considered 'strong' in that it passes all
 *	polynomial-time statistical tests.  The sequences produced
 *	are random in an absolutely precise way.  There is absolutely
 *	no better way to predict the sequence than by tossing a coin
 *	(as with TRULY random numbers) EVEN IF YOU KNOW THE MODULUS!
 *	Furthermore, having a large chunk of output from the sequence
 *	does not help.  The BITS THAT FOLLOW OR PRECEDE A SEQUENCE
 *	ARE UNPREDICTABLE!
 *
 *	To compromise the generator, an adversary must either factor the
 *	modulus or perform an exhaustive search just to determine the next
 *	(or previous) bit.  If we make the modulus hard to factor
 *	(such as the product of two large well chosen primes) breaking
 *	the sequence could be intractable for todays computers and methods.
 ****
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
 *		 	additive table.
 *
 * Casual direct use of the shuffle generator may be acceptable.  If one
 * desires cryptographically strong random numbers, or if one is paranoid,
 * one should use the Blum generator instead.
 *
 * The a55 generator as the following calc interfaces:
 *
 *   rand(min,max)		(where min < max)
 *
 *	Print an a55 generator random value over interval [a,b).
 *
 *   rand()
 *
 *	Same as rand(0, 2^64).  Print 64 bits.
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
 *
 ***
 *
 * The Blum generator is the best generator in this package.  It
 * produces a cryptographically strong pseudo-random bit sequence.
 * Internally, a fixed number of bits are generated after each
 * generator iteration.  Any unused bits are saved for the next call
 * to the generator.  The Blum generator is not too slow, though
 * seeding the generator via srandom(seed,plen,qlen) can be slow.
 * Shortcuts and pre-defined generators have been provided for this reason.
 * Use of Blum should be more than acceptable for many applications.
 *
 * The Blum generator as the following calc interfaces:
 *
 *   random(min, max)		(where min < max)
 *   XXX - write this function
 *
 *	Print a Blum generator random value over interval [min,max).
 *
 *   random()
 *   XXX - write this function
 *
 *	Same as random(0, 2^64).  Print 64 bits.
 *
 *   random(lim)		(where 0 > lim)
 *   XXX - write this function
 *
 *	Same as random(0, lim).
 *
 *   randombit(x)		(where x > 0)
 *   XXX - write this function
 *
 *	Same as random(0, 2^x).  Print x bits.
 *
 *   randombit(skip)		(where skip < 0)
 *   XXX - write this function
 *
 *	Skip skip random bits and return the bit skip count (-skip).
 */

/*
 * INITIALIZATION AND SEEDS:
 *
 * All generators come already seeded with precomputed initial constants.
 * Thus, it is not required to seed a generator before using it.
 *
 * The a55 generator may be initialized and seeded via srand().
 * The Blum generator may be initialized and seeded via srandom().
 *
 * Using a seed of '0' will reload generators with their initial states.
 *
 *	srand(0)	restore additive 55 generator to the initial state
 *	srandom(0)	restore Blum generator to the initial state
 *
 * The above single arg calls are fairly fast.
 *
 * The call:
 *
 *	srandom(seed, newn)
 *
 * is fast when the config value "srandom" is 0, 1 or 2.
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
 * Optimal seed range for the Blum generator:
 *
 *	There is no limit on the size of a seed.  On the other hand,
 *	in most cases the seed is taken modulo the Blum modulus.
 *	Using a seed that is too small (except for 0) results in
 *	an internal generator be used to increase its size.
 *
 *	It is faster to use seeds that are in the half open internal
 *	[sqrt(n), n) where n is the Blum modulus.
 *
 *	The default Blum modulus is 256 bits.  The default
 *	optimal size of a seed is between 128 and 256 bits.
 *
 *	The exception is when srandom(seed, plen, qlen) is used.
 *	When seed < 0, the seed is given to an internal a55 generator
 *	and the a55 generator range (negated) applies.  When seed > 0,
 *	the seed is given to an internal Blum generator and the
 *	128 to 256 bit range applies.  The value seed == 0 may also
 *	be used in this type of call.
 *
 *****
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
 ***
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
 ***
 *
 * srand()
 *
 *    Return current a55 generator state.  This call does not alter
 *    the generator state.
 *
 ***
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
 *
 ***
 *
 * srandom(seed)
 * XXX - write this function
 *
 *    Seed the Blum generator using the current Blum modulus.
 *
 *    Here we assume that the Blum modulus is n.  Any internally buffered
 *    random bits are flushed.
 *
 *    seed > 0:
 *    --------
 *	Seed the an internal additive 55 shuffle generator, and use it
 *	to produce an initial quadratic residue in the range:
 *
 *	    [2^(binsize*4/5), 2^(binsize-2))
 *
 *	where 2^(binsize-1) < n <= 2^binsize and 'n' is the current Blum
 *	modulus.  Here, binsize is the smallest power of 2 >= n.
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	    cur_state = srand(seed);
 *	    binsize = highbit(n)+1;	(* n is the current Blum modulus *)
 *	    r = pmod(rand(1<<ceil(binsize*4/5), 1<<(binsize-2)), 2, n);
 *	    junk = srand(cur_state);
 *
 *	Also note that the internal generator that is used to help
 *	generate the quadratic residue is internal to this function only.
 *	It will not impact the use of any other generator.  (The above
 *	code is just an illustration)
 *
 *    seed == 0:
 *    ---------
 *	Restore the initial state and modulus of the Blum generator.
 *	After this call, the Blum  generator is restored to its initial
 *	state after calc started.
 *
 *	The Blum prime factors of the modulus have been disclosed (see
 *	"SOURCE OF MAGIC NUMBERS" below).  If you want to use moduli that
 *	have not been disclosed, use srandom(seed, newn) with the
 *	appropriate args as noted below.
 *
 *	Note that unlike other forms of srandom(seed), seed == 0 does
 *	change the modulus.
 *
 *    seed < 0:
 *    --------
 *	Reserved for future use.
 *
 *****
 *
 * srandom(seed, newn)
 * XXX - write this function
 *
 *   Seed the Blum generator.
 *
 *   Here we attempt to set the Blum modulus to the value newn.
 *   Any internally buffered random bits are flushed.
 *
 *   The config value "srandom" determines what, if any, sanity
 *   checks are performed on newn:
 *
 *	    0	no checks are performed
 *	    1   require newn to be 1 mod 4
 *	    2	require newn to be 1 mod 4 and to pass ptest(newn,0)
 *		(trivial factor check)
 *	    3	require newn to be 1 mod 4 and to pass ptest(newn,1)
 *		(trivial factor and one probable prime test)
 *	    4	require newn to be 1 mod 4 and to pass ptest(newn,25)
 *		(trivial factor and 25 probable prime test)
 *
 *    The default "srandom" value is 3.  All other values are reserved
 *    for future use.
 *
 *    seed == 0, 1007 <= newn:
 *    -----------------------
 *	If 'newn' passes the tests (if applicable) specified by the "srandom"
 *	config value, it becomes the Blum modulus.  Any internally buffered
 *	random bits are flushed.
 *
 *	The initial quadratic residue 'r', is selected as if the following
 *	was executed:
 *
 *	    (* set Blum modulus to newn if allowed by "srandom" config value *)
 *	    (* and then set the initial quadratic residue by the next call *)
 *	    srandom(newn % 2^309);
 *
 *	The first srand() call seeds the additive 55 shuffle generator
 *	with the lower 309 bits of newn.  In actual practice, calc uses
 *	an independent internal rand() state value.
 *
 *    seed > 0, 1007 <= newn:
 *    ----------------------
 *	If 'newn' passes the tests (if applicable) specified by the "srandom"
 *	config value, it becomes the Blum modulus.   Once the Blum modulus
 *	is set, seed is used to seed an internal Additive 55 generator
 *	state which in turn is used to set the initial quadratic residue.
 *
 *	While not as efficient, this call is equivalent to:
 *
 *	    srandom(0, newn);
 *	    srandom(seed);
 *
 *    seed < 0, 1007 <= newn:
 *    ----------------------
 *	Reserved for future use.
 *
 *    any seed, 20 < newn < 1007:
 *    --------------------------
 *	Reserved for future use.
 *
 *    seed == 0, 0 < newn <= 20:
 *    -------------------------
 *	Seed with one of the predefined Blum moduli.
 *
 *	The Blum moduli used by the pre-defined generators were generated
 *	using the above process.  The initial search values for the Blum
 *	primes and the value used for selecting the initial quadratic
 *	residue (by squaring it mod the Blum modulus) were produced by
 *	special purpose hardware that produces cryptographically strong
 *	random numbers.
 *
 *	See the URL:
 *
 *	    http://lavarand.sgi.com
 *
 *	for an explination of how the search points of the generators
 *	were selected.
 *
 *	XXX - This URL is not available on 16Jun96 ... but will be soon.
 *
 *	The purpose of these pre-defined Blum moduli is to provide users with
 *	an easy way to use a generator where the individual Blum primes used
 *	are not well known.  True, these values are in some way "MAGIC", on
 *	the other hand that is their purpose!  If this bothers you, don't
 *	use them.  See the section "FOR THE PARANOID" below for details.
 *
 *	The value 'newn' determines which pre-defined generator is used.
 *	For a given 'newn' the Blum modulus 'n' (product of 2 Blum primes)
 *	and initial quadratic residue 'r' is set as follows:
 *
 *	newn ==  1: (Blum modulus bit length 130)
 *	    n = 0x5049440736fe328caf0db722d83de9361
 *	    r = 0xb226980f11d952e74e5dbb01a4cc42ec
 *
 *	newn ==  2: (Blum modulus bit length 137)
 *	    n = 0x2c5348a2555dd374a18eb286ea9353443f1
 *	    r = 0x40f3d643446cd710e3e893616b21e3a218
 *
 *	newn ==  3: (Blum modulus bit length 147)
 *	    n = 0x9cfd959d6ce4e3a81f1e0f2ca661f11d001f1
 *	    r = 0xfae5b44d9b64ff5cea4f3e142de2a0d7d76a
 *
 *	newn ==  4: (Blum modulus bit length 157)
 *	    n = 0x3070f9245c894ed75df12a1a2decc680dfcc0751
 *	    r = 0x20c2d8131b2bdca2c0af8aa220ddba4b984570
 *
 *	newn ==  5: (Blum modulus bit length 257)
 *	    n = 0x2109b1822db81a85b38f75aac680bc2fa5d3fe1118769a0108b99e5e799
 *		166ef1
 *	    r = 0x5e9b890eae33b792e821a9605f5df6db234f7b7d1e70aeed0e6c77c859e
 *		2efa9
 *
 *	newn ==  6: (Blum modulus bit length 259)
 *	    n = 0xa7bfd9d7d9ada2c79f2dbf2185c6440263a38db775ee732dad85557f1e1
 *		ddf431
 *	    r = 0x5e94a02f88667154e097aedece1c925ce1f3495d2c98eccfc5dc2e80c94
 *		04daf
 *
 *	newn ==  7: (Blum modulus bit length 286)
 *	    n = 0x43d87de8f2399ef237801cd5628643fcff569d6b0dcf53ce52882e7f602
 *	        f9125cf9ec751
 *	    r = 0x13522d1ee014c7bfbe90767acced049d876aefcf18d4dd64f0b58c3992d
 *	        2e5098d25e6
 *
 *	newn ==  8: (Blum modulus bit length 294)
 *	    n = 0x5847126ca7eb4699b7f13c9ce7bdc91fed5bdbd2f99ad4a6c2b59cd9f0b
 *	        c42e66a26742f11
 *	    r = 0x853016dca3269116b7e661fa3d344f9a28e9c9475597b4b8a35da929aae
 *	        95f3a489dc674
 *
 *	newn ==  9: (Blum modulus bit length 533)
 *	    n = 0x39e8be52322fd3218d923814e81b003d267bb0562157a3c1797b4f4a867
 *	        52a84d895c3e08eb61c36a6ff096061c6fd0fdece0d62b16b66b980f95112
 *		745db4ab27e3d1
 *	    r = 0xb458f8ad1e6bbab915bfc01508864b787343bc42a8aa82d9d2880107e3f
 *	        d8357c0bd02de3222796b2545e5ab7d81309a89baedaa5d9e8e59f959601e
 *		f2b87d4ed20d
 *
 *	newn == 10: (Blum modulus bit length 537)
 *	    n = 0x25f2435c9055666c23ef596882d7f98bd1448bf23b50e88250d3cc952c8
 *	        1b3ba524a02fd38582de74511c4008d4957302abe36c6092ce222ef9c73cc
 *		3cdc363b7e64b89
 *	    r = 0x66bb7e47b20e0c18401468787e2b707ca81ec9250df8cfc24b5ffbaaf2c
 *	        f3008ed8b408d075d56f62c669fadc4f1751baf950d145f40ce23442aee59
 *		4f5ad494cfc482
 *
 *	newn == 11: (Blum modulus bit length 542)
 *	    n = 0x497864de82bdb3094217d56b874ecd7769a791ea5ec5446757f3f9b6286
 *	        e58704499daa2dd37a74925873cfa68f27533920ee1a9a729cf522014dab2
 *		2e1a530c546ee069
 *	    r = 0x8684881cb5e630264a4465ae3af8b69ce3163f806549a7732339eea2c54
 *	        d5c590f47fbcedfa07c1ef5628134d918fee5333fed9c094d65461d88b13a
 *		0aded356e38b04
 *
 *	newn == 12: (Blum modulus bit length 549)
 *	    n = 0x3457582ab3c0ccb15f08b8911665b18ca92bb7c2a12b4a1a66ee4251da1
 *	        90b15934c94e315a1bf41e048c7c7ce812fdd25d653416557d3f09887efad
 *		2b7f66d151f14c7b99
 *	    r = 0xdf719bd1f648ed935870babd55490137758ca3b20add520da4c5e8cdcbf
 *	        c4333a13f72a10b604eb7eeb07c573dd2c0208e736fe56ed081aa9488fbc4
 *		5227dd68e207b4a0
 *
 *	newn == 13: (Blum modulus bit length 1048)
 *	    n = 0x1517c19166b7dd21b5af734ed03d833daf66d82959a553563f4345bd439
 *	        510a7bda8ee0cb6bf6a94286bfd66e49e25678c1ee99ceec891da8b18e843
 *		7575113aaf83c638c07137fdd3a76c3a49322a11b5a1a84c32d99cbb2b056
 *		671589917ed14cc7f1b5915f6495dd1892b4ed7417d79a63cc8aaa503a208
 *		e3420cca200323314fc49
 *	    r = 0xd42e8e9a560d1263fa648b04f6a69b706d2bc4918c3317ddd162cb4be7a
 *	        5e3bbdd1564a4aadae9fd9f00548f730d5a68dc146f05216fe509f0b8f404
 *		902692de080bbeda0a11f445ff063935ce78a67445eae5c9cea5a8f6b9883
 *		faeda1bbe5f1ad3ef6409600e2f67b92ed007aba432b567cc26cf3e965e20
 *		722407bfe46b7736f5
 *
 *	newn == 14: (Blum modulus bit length 1054)
 *	    n = 0x5e56a00e93c6f4e87479ac07b9d983d01f564618b314b4bfec7931eee85
 *	        eb909179161e23e78d32110560b22956b22f3bc7e4a034b0586e463fd40c6
 *		f01a33e30ede912acb86a0c1e03483c45f289a271d14bd52792d0a076fdfe
 *		fe32159054b217092237f0767434b3db112fee83005b33f925bacb3185cc4
 *		409a1abdef8c0fc116af01
 *	    r = 0xf7aa7cb67335096ef0c5d09b18f15415b9a564b609913f75f627fc6b0c5
 *	        b686c86563fe86134c5a0ea19d243350dfc6b9936ba1512abafb81a0a6856
 *		c9ae7816bf2073c0fb58d8138352b261a704b3ce64d69dee6339010186b98
 *		3677c84167d4973444194649ad6d71f8fa8f1f1c313edfbbbb6b1b220913c
 *		c8ea47a4db680ff9f190
 *
 *	newn == 15: (Blum modulus bit length 1055)
 *	    n = 0x97dd840b9edfbcdb02c46c175ba81ca845352ebe470be6075326a26770c
 *	        ab84bfc0f2e82aa95aac14f40de42a0590445b902c2b8ebb916753e72ab86
 *		c3278cccc1a783b3e962d81b80df03e4380a8fa08b0d86ed0caa515c196a5
 *		30e49c558ddb53082310b1d0c7aee6f92b619798624ffe6c337299bc51ff5
 *		d2c721061e7597c8d97079
 *	    r = 0xb8220703b8c75869ab99f9b50025daa8d77ca6df8cef423ede521f55b1c
 *	        25d74fbf6d6cc31f5ef45e3b29660ef43797f226860a4aa1023dbe522b1fe
 *		6224d01eb77dee9ad97e8970e4a9e28e7391a6a70557fa0e46eca78866241
 *		ba3c126fc0c5469f8a2f65c33db95d1749d3f0381f401b9201e6abd43d98d
 *		b92e808f0aaa6c3e2110
 *
 *	newn == 16: (Blum modulus bit length 1062)
 *	    n = 0x456e348549b82fbb12b56f84c39f544cb89e43536ae8b2b497d426512c7
 *	        f3c9cc2311e0503928284391959e379587bc173e6bc51ba51c856ba557fee
 *		8dd69cee4bd40845bd34691046534d967e40fe15b6d7cf61e30e283c05be9
 *		93c44b6a2ea8ade0f5578bd3f618336d9731fed1f1c5996a5828d4ca857ac
 *		2dc9bd36184183f6d84346e1
 *	    r = 0xb0d7dcb19fb27a07973e921a4a4b6dcd7895ae8fced828de8a81a3dbf25
 *	        24def719225404bfd4977a1508c4bac0f3bc356e9d83b9404b5bf86f6d19f
 *		f75645dffc9c5cc153a41772670a5e1ae87a9521416e117a0c0d415fb15d2
 *		454809bad45d6972f1ab367137e55ad0560d29ada9a2bcda8f4a70fbe04a1
 *		abe4a570605db87b4e8830
 *
 *	newn == 17: (Blum modulus bit length 2062)
 *	    n = 0x6177813aeac0ffa3040b33be3c0f96e0faf97ca54266bfedd7be68494f7
 *		6a7a91144598bf28b3a5a9dc35a6c9f58d0e5fb19839814bc9d456bff7f29
 *		953bdac7cafd66e2fc30531b8d544d2720b97025e22b1c71fa0b2eb9a499d
 *		49484615d07af7a3c23b568531e9b8507543362027ec5ebe0209b4647b7ff
 *		54be530e9ef50aa819c8ff11f6d7d0a00b25e88f2e6e9de4a7747022b949a
 *		b2c2e1ab0876e2f1177105718c60196f6c3ac0bde26e6cd4e5b8a20e9f0f6
 *		0974f0b3868ff772ab2ceaf77f328d7244c9ad30e11a2700a120a314aff74
 *		c7f14396e2a39cc14a9fa6922ca0fce40304166b249b574ffd9cbb927f766
 *		c9b150e970a8d1edc24ebf72b72051
 * 	    r = 0x53720b6eaf3bc3b8adf1dd665324c2d2fc5b2a62f32920c4e167537284d
 *		a802fc106be4b0399caf97519486f31e0fa45a3a677c6cb265c5551ba4a51
 *		68a7ce3c29731a4e9345eac052ee1b84b7b3a82f906a67aaf7b35949fd7fc
 *		2f9f4fbc8c18689694c8d30810fff31ebee99b1cf029a33bd736750e7fe0a
 *		56f7e1d2a9b5321b5117fe9a10e46bf43c896e4a33faebd584f7431e7edbe
 *		bd1703ccee5771b44f0c149888af1a4264cb9cf2e0294ea7719ed6fda1b09
 *		fa6e016c039aeb6d02a03281bcea8c278dd2a807eacae6e52ade048f58f2e
 *		b5193f4ffb9dd68467bc6f8e9d14286bfef09b0aec414c9dadfbf5c46d945
 *		d147b52aa1e0cbd625800522b41dac
 *
 *	newn == 18: (Blum modulus bit length 2074)
 *	     n= 0x68f2a38fb61b42af07cb724fec0c7c65378efcbafb3514e268d7ee38e21
 *		a5680de03f4e63e1e52bde1218f689900be4e5407950539b9d28e9730e8e6
 *		ad6438008aa956b259cd965f3a9d02e1711e6b344b033de6425625b6346d2
 *		ca62e41605e8eae0a7e2f45c25119ef9eece4d3b18369e753419d94118d51
 *		803842f4de5956b8349e6a0a330145aa4cd1a72afd4ef9db5d8233068e691
 *		18ff4b93bcc67859f211886bb660033f8170640c6e3d61471c3b7dd62c595
 *		b156d77f317dc272d6b7e7f4fdc20ed82f172fe29776f3bddf697fb673c70
 *		defd6476198a408642ed62081447886a625812ac6576310f23036a7cd3c93
 *		1c96f7df128ad4ed841351b18c8b78629
 *	     r= 0x4735e921f1ac6c3f0d5cda84cd835d75358be8966b99ff5e5d36bdb4be1
 *		2c5e1df70ac249c0540a99113a8962778dc75dac65af9f3ab4672b4c575c4
 *		9926f7f3f306fd122ac033961d042c416c3aa43b13ef51b764d505bb1f369
 *		ac7340f8913ddd812e9e75e8fde8c98700e1d3353da18f255e7303db3bcbb
 *		eda4bc5b8d472fbc9697f952cfc243c6f32f3f1bb4541e73ca03f5109df80
 *		37219a06430e88a6e94be870f8d36dbcc381a1c449c357753a535aa5666db
 *		92af2aaf1f50a3ddde95024d9161548c263973665a909bd325441a3c18fc7
 *		0502f2c9a1c944adda164e84a8f3f0230ff2aef8304b5af333077e04920db
 *		a179158f6a2b3afb78df2ef9735ea3c63
 *
 *	newn == 19: (Blum modulus bit length 2133)
 *	     n= 0x230d7ab23bb9e8d6788b252ad6534bdde276540721c3152e410ad4244de
 *		b0df28f4a6de063ba1e51d7cd1736c3d8410e2516b4eb903b8d9206b92026
 *		64cacbd0425c516833770d118bd5011f3de57e8f607684088255bf7da7530
 *		56bf373715ed9a7ab85f698b965593fe2b674225fa0a02ebd87402ffb3d97
 *		172acadaa841664c361f7c11b2af47a472512ee815c970af831f95b737c34
 *		2508e4c23f3148f3cdf622744c1dcfb69a43fd535e55eebcdc992ee62f2b5
 *		2c94ac02e0921884fe275b3a528bdb14167b7dec3f3f390cd5a82d80c6c30
 *		6624cc7a7814fb567cd4d687eede573358f43adfcf1e32f4ee7a2dc4af029
 *		6435ade8099bf0001d4ae0c7d204df490239c12d6b659a79
 *	     r= 0x8f1725f21e245e4fc17982196605b999518b4e21f65126fa6fa759332c8
 *		e27d80158b7537da39d001cc62b83bbef0713b1e82f8293dad522993f86d1
 *		761015414b2900e74fa23f3eaaa55b31cffd2e801fefb0ac73fd99b5d0cf9
 *		a635c3f4c73d8892d36ad053fc17a423cdcbcf07967a8608c7735e287d784
 *		ae089b3ddea9f2d2bb5d43d2ee25be346832e8dd186fc7a88d82847c03d1c
 *		05ee52c1f2a51a85f733338547fdbab657cb64b43d44d41148eb32ea68c7e
 *		66a8d47806f460cd6573b6ca1dd3eeaf1ce8db9621f1e121d2bb4a1878621
 *		dd2dbdd7b5390ab06a5dcd9307d6662eb4248dff2ee263ef2ab778e77724a
 *		14c62406967daa0d9ad4445064483193d53a5b7698ef473
 *
 *	newn == 20: (Blum modulus bit length 2166)
 *	     n= 0x4fd2b820e0d8b13322e890dddc63a0267e5b3a648b03276066a3f356d79
 *		660c67704c1be6803b8e7590ee8a962c8331a05778d010e9ba10804d661f3
 *		354be1932f90babb741bd4302a07a92c42253fd4921864729fb0f0b1e0a42
 *		d66b6777893195abd2ee2141925624bf71ad7328360135c565064ee502773
 *		6f42a78b988f47407ba4f7996892ffdc5cf9e7ab78ac95734dbf4e3a3def1
 *		615b5b4341cfbf6c3d0a61b75f4974080bbac03ee9de55221302b40da0c50
 *		ded31d28a2f04921a532b3a486ae36e0bb5273e811d119adf90299a74e623
 *		3ccce7069676db00a3e8ce255a82fd9748b26546b98c8f4430a8db2a4b230
 *		fa365c51e0985801abba4bbcf3727f7c8765cc914d262fcec3c1d081
 *	     r= 0x46ef0184445feaa3099293ee960da14b0f8b046fa9f608241bc08ddeef1
 *		7ee49194fd9bb2c302840e8da88c4e88df810ce387cc544209ec67656bd1d
 *		a1e9920c7b1aad69448bb58455c9ae4e9cd926911b30d6b5843ff3d306d56
 *		54a41dc20e2de4eb174ec5ac3e6e70849de5d5f9166961207e2d8b31014cf
 *		35f801de8372881ae1ba79e58942e5bef0a7e40f46387bf775c54b1d15a14
 *		40e84beb39cd9e931f5638234ea730ed81d6fca1d7cea9e8ffb171f6ca228
 *		56264a36a2a783fd7ac39361a6598ed3a565d58acf1f5759bd294e5f53131
 *		bc8e4ee3750794df727b29b1f5788ae14e6a1d1a5b26c2947ed46f49e8377
 *		3292d7dd5650580faebf85fd126ac98d98f47cf895abdc7ba048bd1a
 *
 *	NOTE: The Blum moduli associated with 1 <= newn < 12 are subject
 *	      to having their Blum moduli factored, depending in their size,
 *	      by small PCs in a reasonable to large supercomputers/highly
 *	      parallel processors over a long time.  Their value lies in their
 *	      speed relative the the default Blum generator.  As of Jan 1996,
 *	      the Blum moduli associated with 12 <= newn < 20 appear to
 *	      be well beyond the scope of hardware and algorithms.
 *	      See the section titled 'FOR THE PARANOID' for more details.
 *
 *    seed > 0, 0 < newn <= 20:
 *    ------------------------
 *	Use the same pre-defined Blum moduli 'n' noted above but use 'seed'
 *	to find a different quadratic residue 'r'.
 *
 *	While not as efficient, this call is equivalent to:
 *
 *	    srandom(0, newn);
 *	    srandom(seed);
 *
 *    seed < 0, 0 < newn <= 20:
 *    ------------------------
 *	Use the same pre-defined Blum moduli 'n' noted above but use '-seed'
 *	to compute a different quadratic residue 'r'.
 *
 *	This call has the same effect as:
 *
 *	    srandom(0, newn)
 *
 *	followed by the setting of the quadratic residue 'r' as follows:
 *
 *	    r = pmod(seed, 2, n)
 *
 *	where 'n' is the Blum moduli generated by 'srandom(0,newn)' and
 *	'r' is the new quadratic residue.
 *
 *	NOTE: Because no checking on 'seed' is performed, it is recommended
 *	that 'seed' be selected as follows:
 *
 *	    binsize = highbit(n)+1;
 *	    seed = -rand(1<<ceil(binsize*4/5), 1<<(binsize-2));
 *
 *   any seed, newn <= 0:
 *   -------------------
 *     Reserved for future use.
 *
 * srandom()
 * XXX - write this function
 *
 *    Return current Blum generator state.  This call does not alter
 *    the generator state.
 *
 * srandom(state)
 * XXX - write this function
 *
 *	Restore the Blum state and return the previous state.  Note that
 *	the argument state is a random state value (israndom(state) is true).
 *	Any internally buffered random bits are restored.
 *
 *	The states of the Blum generators can be saved by calling the seed
 *	function with no arguments, and later restored by calling the seed
 *	functions with that same return value.
 *
 *	    random_state = srandom();
 *	    ... generate random bits ...
 *	    prev_random_state = srandom(random_state);
 *	    ... generate the same random bits ...
 *	    srandom() == prev_random_state;		(* is true *)
 *
 *	Saving the state just after seeding a generator and restoring it later
 *	as a very fast way to reseed a generator.
 */

/*
 * TRUTH IN ADVERTISING:
 *
 * When the call:
 *
 *	srandom(seed, nextcand(ip,25,0,3,4)*nextcand(iq,25,0,3,4))
 *
 * probable primes from nextcand are used.  We use the word
 * 'probable' because of an extremely extremely small chance that a
 * composite (a non-prime) may be returned.
 *
 * We use the builtin function nextcand in its 5 arg form:
 *
 *	nextcand(value, 25, 0, 3, 4)
 *
 * The odds that a number returned by the above call is not prime is
 * less than 1 in 4^25.  For our purposes, this is sufficient as the
 * chance of returning a composite is much smaller than the chance that
 * a hardware glitch will cause nextcand() to return a bogus result.
 *
 * Another "truth in advertising" issue is the use of the term
 * 'pseudo-random'.  All deterministic generators are pseudo random.
 * This is opposed to true random generators based on some special
 * physical device.
 *
 * Even though Blum generator is 'pseudo-random', there is no statistical
 * test, which runs in polynomial time, that can distinguish the Blum
 * generator from a truly random source.  See the comment under
 * the "Blum-Blum-Shub generator" section above.
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
 * We use the upper byte of the additive 55 value to select
 * the shuffle table entry because it allows all of 64 bits
 * to play a part in the entry selection.  If we were to
 * select a lower 8 bits in the 64 bit value, carries that
 * propagate above our 8 bits would not impact the a55
 * generator output.
 *
 * When seeding, the initial quadratic residue used by the Blum generator
 * is taken from the interval:
 *
 *	[2^(binsize*4/5), 2^(binsize-2))
 *
 * where 2^(binsize-1) < n=p*q <= 2^binsize.  Here, binsize is the smallest
 * power of 2 >= n=p*q.
 *
 * The use of the above range insures that the quadratic residue is
 * large, but not too large.  We want to avoid residues that are
 * near 0 or that are near 'n'.  Such residues are trivial or
 * semi-trivial.  Applying the same restriction to the square
 * of the initial residue avoid initial residues near 'sqrt(n)'.
 * Such residues are trivial or semi-trivial as well.
 *
 * Lower bound 2^(binsize*4/5) (4/5 the size of the smallest power of 2 >= n)
 * is used because it avoids initial quadratic residues near 0, n^1/4, n^1/2,
 * n^1/3, n^2/3 and n^3/4.  For a trivial example, take the trivial case of
 * selecting a quadratic residue of 1, 0 or n-1.  Repeated squarings produce
 * poor results.  Similar but far less drastic results come from an
 * initial selection that is near n^1/2 or other small fractional power.
 * While the above initial quadratic residue range range allows for
 * powers of n such as n^3/7, n^5/6, these powers are more complex and
 * produce less obvious patterns when squared mod n.
 *
 * The upper bound of 2^(binsize-2) allows one to avoid initial quadratic
 * residues near 'n'.  Since n could be as small as 2^(binsize-1)+1, we
 * must use the next lower power of 2: 2^(binsize-2) to be sure that we
 * avoid initial quadratic residues near n.
 *
 * Taking some care to select a good initial residue helps eliminate cheap
 * search attacks.  It is true that a subsequent residue could be one of the
 * residues that we would first avoid.  However such an occurrence will
 * happen after the generator is well underway and any such seed information
 * has been lost.
 *
 * The size of Blum modulus 'n=p*q' was taken to be > 2^1024, or 1025 bits
 * (309 digits) long.  As if Jan 1996, the upper reach of the state of
 * the art for factoring general numbers was around 2^512.  We selected
 * 2^1024 because it was twice that size and would hopefully remain well
 * beyond the reach of Number Theory and CPU power for some time.
 *
 * Not being able to factor 'n=p*q' into 'p' and 'q' does not directly
 * improve the quality Blum generator.  On the other hand, it does
 * improve the security of it.
 *
 * The number of bits produced each cycle for a given Blum modulus 'n'
 * is int(log2(log2(n))).  Thus for 2^1024 <= n < 2^2048, 10 bits are
 * produced.  For optimal performance, we use a Blum modulus that is
 * slightly larger than 2^(2^x) to produce 'x' bits at a time.
 *
 * The lengths of the two Blum probable primes 'p' and 'q' used to make up
 * the default Blum modulus 'n=p*q' differ slightly to avoid certain
 * factorization attacks that work on numbers that are a perfect square,
 * or where the two primes are nearly the same.  I elected to have the
 * sizes differ by up to 6% of the product size to avoid such attacks.
 * Clearly one does not want the size of the two factors to differ
 * by a large percentage: p=3 and q large would result in a easy
 * to factor Blum modulus.  Thus we select sizes that differ by
 * up to 6% but not (significantly) greater than 6%.
 *
 * Again, the ability (or lack thereof) to factor 'n=p*q' does not
 * directly relate to the strength of the Blum generator.  We
 * selected n=p*q > 2^1024 mainly because 1024 was a power of 2.
 * Secondly 1024 the first power of 2 beyond 512 which bit size at
 * or near the general factor limit a of Jan 1996.
 *
 * Using the '6% rule' above, a Blum modulus n=p*q > 2^1024 would have two
 * Blum factors p > 2^482 and q > 2^542.  This is because 482+542 = 1024.
 * The difference 542-482 is ~5.86% of 1024, and is the largest difference
 * that is < 6%.
 *
 * The default Blum modulus is the product of two Blum probable primes
 * that were selected by the Rand Book of Random Numbers.  Using the '6% rule',
 * a default Blum modulus n=p*q > 2^1024 would be satisfied if p were
 * 146 decimal digits and q were 164 decimal digits in length.  We restate
 * the sizes in decimal digits because the Rand Book of Random Numbers is a
 * book of decimal digits.  Using the first 146 rand digits as a
 * starting search point for 'p', and the next 164 digits for a starting
 * search point for 'q'.
 *
 *	(*
 *	 * setup the search points (lines split for readability)
 *	 *)
 *	ip = 10097325337652013586346735487680959091173929274945
 *	     37542048056489474296248052403720636104020082291665
 *	     0842268953196450930323209025601595334764350803;
 *	iq = 36069901902529093767071538311311658867674397044362
 *	     76591280799970801573614764032366539895116877121717
 *	     68336606574717340727685036697361706581339885111992
 *	     91703106010805;
 *
 *	(*
 *	 * find the first Blum prime
 *	 *)
 *	fp = int((ip-1)/2);
 *	do {
 *		fp = nextcand(fp+2, 25, 0, 3, 4);
 *		p = 2*fp+1;
 *	} while (ptest(p, 25) == 0);
 *
 *	(*
 *	 * find the 2nd Blum prime
 *	 *)
 *	fq = int((iq-1)/2);
 *	do {
 *		fq = nextcand(fq+2, 25, 0, 3, 4);
 *		q = 2*fq+1;
 *	} while (ptest(q, 25) == 0);
 *
 * The above script produces the Blum probable primes and initial quadratic
 * residue (line wrapped for readability):
 *
 *	    p = 0x33c08d08248479497fe557b0e013b1beb51957cb441840f95d199e40fa9
 *		19faee2444d687775cb391bc703d710bd05f0cb0670b0bd49430ec8f9393e
 *		7
 *
 *	    q = 0xa05970f94cdf85f9773f7772636d591c0575bf5873299b4f48f873529f8
 *		85e91577802c65d629e809e797d130254afb7b1e8a4d7afe4f18facec41c2
 *		7f2bcfa1496e53a7
 *
 * These Blum primes were found after 43m 56s of CPU time on a 150 Mhz IP22
 * R4400 version 5.0 processor.  The first Blum prime 'p' was 411284 higher
 * than the initial search value 'ip'.  The second Blum prime 'q' was 87282
 * higher than the initial starting 'iq'.
 *
 * The product of the two Blum primes results in a 1026 bit Blum modulus of:
 *
 *	    n = 0x206a6cecc22e947050ffcf5eb53742e0a85433800fcaab4452df182bccf
 *		72b874f3abaf118b29d64a859cd9c1465796a1cdf061f9bf3374443da6e1c
 *		fc63b7a7bd90dad9a3853642820ab4664a82ae1951779f3d1af9a70bedfd4
 *		abcd89cdc200cbb917c1f7881fc900163d7a84f5e8e53d5bc5918590c15a4
 *		45430bbee7b60b1
 *
 * The selection if the initial quadratic residue comes from the next
 * unused digits of the Rand Book of Random Numbers.  Now the two initial
 * search values 'ip' and 'iq' used above needed the first 146 digits and
 * the next 164 digits.  Thus we will skip the first 146+164=310 digits
 * and begin to build in initial search value for a quadratic residue (most
 * significant digit first) from the Rand Book of Numbers digits until we
 * have a value that is within the range:
 *
 *	[2^(binsize*4/5), 2^(binsize-2))
 *
 * where 2^(binsize-1) < n=p*q <= 2^binsize.  Here, binsize is the
 * smallest power of 2 >= n=p*q.  Using this method, we arrive at an
 * initial search value for the quadratic residue (wrapped for readability):
 *
 *	   ir = 45571824063530342614867990743923403097328526977602
 *		02051656926866574818730538524718623885796357332135
 *		05325470489055357548284682870983491256247379645753
 *		03529647783580834282609352034435273884359852017767
 *		14905686072210940558609709343350500739981180505431
 *		39808277325072568248294052420152775678518345299634
 *		06288980
 *
 * using the next 308 digits from the Rand Book of Random Numbers.  The
 * (310+309)th digit results in an 'ir' that is outside the range noted above.
 *
 * Using pmod(ir, 2, n), we arrive at the initial quadratic residue of the
 * default Blum generator:
 *
 *	    r = 0x1455b0e84ea73df591501002a7ff7855ef114f4ab34114f7e78208179a7
 *		8b722591126b68629b8e840ef5408f7d46db41b438fba4bfd69a6fa7635ab
 *		fbbfde64a198d62cfab4f03f43fb1f402c63202c7beb0b023034f27c6729b
 *		672fc0ac85e14c610137e7766c67f1ea9cf75e0d60339e254065642e37b7f
 *		4b9462d0687e467
 *
 * In the above process, we selected primes of the form:
 *
 *	2*x + 1		x is also prime
 *
 * because Blum generators with modulus 'n=p*q' have a period:
 *
 *	lambda(n) = lcm(factors of p-1 & q-1)
 *
 * since 'p' and 'q' are both odd, 'p-1' and 'q-1' have 2 as
 * a factor.  The calc script above ensures that '(p-1)/2' and
 * '(q-1)/2' are probable prime, thus maximizing the period
 * of the default generator to:
 *
 *	lambda(n) = lcm(2,2,fp,fq) = 2*fp*fq = ~2*(p/2)*(q/2) = ~n/2
 *
 * The process above resulted in a default generator Blum modulus n > 2^1024
 * with period of at least 2^1023 bits.  To be exact, the period of the
 * default Blum generator is:
 *
 *	    0x9edee4226e56e1ba24e6b20180648967ae10bba409a1a1975e95c9c4be0dc9b7
 *	      4af2d44bd15a117f6a108d043418c88957f4a3e2c10c3267b44332c7445b6a0c
 *	      dcdc2ebefec6f8fa48aff8c9867769c4bfa790acba7e7aaa4b90bc2bff5ba65f
 *	      9e652919cfc51edd706b52c884cf56e8fbd378c1f561c651a9f7000180481e0d2
 *
 * which is approximately:
 *
 *	   ~1.785 * 10^309
 *
 * This period is more than long enough for computationally tractable tasks.
 *
 ****
 *
 * FOR THE PARANOID:
 *
 * The truly paranoid might suggest that my claims in the MAGIC NUMBERS
 * section are a lie intended to entrap people.  Well they are not, but
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
 * it.  No extra security is gained or reduced by using the randreseed64()
 * process.  The meaning of seeds are exchanged, but not lost or favored
 * (used by more than one input seed).
 *
 * One could take issue with the above script that produced a 1028 bit
 * Blum modulus.  As far as I know, 310 digits (1028 bits) is beyond the
 * state of the art of Number Theory and Computation as of 01 Jan 96.
 * It is possible in the future that 310 digit products of two primes could
 * come within reach of factoring in the next few years, but so what?
 * If you are truly paranoid, why would you want to use the default seed,
 * which is well known?
 *
 * If all the above fails to pacify the truly paranoid, then one may
 * select your own modulus and initial quadratic residue by calling:
 *
 *	srandom(s, n);
 *
 * Of course, you will need to select a correct Blum modulus 'n' as the
 * product of two Blum primes (both 3 mod 4) and with a long period (where
 * lcm(factors of one less than the two Blum primes) is large) and an
 * initial quadratic residue 's' that is hard to guess (a large value
 * from the range [n^(4/5), n/2) selected at random.
 *
 * A simple way to seed the generator would be to:
 *
 *	config("srandom", 0);
 *	srandom(s, nextcand(ip,25,0,3,4)*nextcand(iq,25,0,3,4))
 *
 * where 'ip' and 'iq' are large integers that are unlikely to be 'guessed'
 * and where they are selected randomly from the [2^(binsize*4/5),
 * 2^(binsize-2)) where 2^(binsize-1) < n=p*q <= 2^binsize.
 *
 * Of course you can increase the '25' value if 1 of 4^25 odds of a
 * non-prime are too probable for you.  The '0' means don't skip any
 * tests* and the final '3,4' means to select only Blum candidates.
 * The config("srandom", 0) call turns off srandom checks on the 'n''
 * argument.  This is OK to do in the above case because the nextcand()
 * calls ensure proper Blum prime selection.
 *
 * The problem with the above call is that the period of the Blum generator
 * could be small if 'p' and 'q' have lots of small prime factors in common.
 *
 * A better way to do seed the Blum generator yourself is to use the
 * seedrandom(seed1, seed2, size [,tests]) function from "seedrandom.cal"
 * with the args:
 *
 *	seed1 - seed rand() to search for 'p', select from [2^64, 2^308)
 *	seed2 - seed rand() to search for 'q', select from [2^64, 2^308)
 *	size  - minimum bit size of the Blum modulus 'n=p*q'
 *	tests - optional arg for number of pseudo prime tests (default is 25)
 *
 * The seedrandom() function ensures that the Blum generator produced
 * has a maximal period.
 *
 * The following call will seed the Blum generator to an identical state
 * produced by srandom(0):
 *
 *	seedrandom(10097325337652013586346735487680959091173929274945,
 *		   37542048056489474296248052403720636104020082291665,
 *		   1024)
 *
 * The seedrandom() function in seedrandom.cal makes use of the rand()
 * additive 55 generator.  If you object to using rand(), you could
 * substitute your own generator (by rewriting the function).
 *
 * Last, one could use some external random source to select starting
 * search points for 'p', 'q' and the quadratic residue.  One way to
 * do this is:
 *
 *	fp = int((ip-1)/2);
 *	do {
 *	    fp = nextcand(fp+2, tests, 0, 3, 4);
 *	    p = 2*fp+1;
 *	} while (ptest(p, tests) == 0);
 *	fq = int((iq-1)/2);
 *	do {
 *	    fq = nextcand(fq+2, tests, 0, 3, 4);
 *	    q = 2*fq+1;
 *	} while (ptest(q, tests) == 0);
 *	srandom(pmod(ir,2,p*q), p*q);
 *
 * where 'tests' is the number of pseudo prime tests that a candidate must
 * pass before being considered a probable prime (must be >0, perhaps 25), and
 * where 'ip' is the initial search location for the Blum prime 'p', and
 * where 'iq' is the initial search location for the Blum prime 'q', and
 * where 'ir' is the initial Blum quadratic residue generator.  The 'ir'
 * value should be a random value in the range [2^(binsize*4/5), 2^(binsize-2))
 * where 2^(binsize-1) < n=p*q <= 2^binsize.
 *
 * Your external generator would need to generate 'ip', 'iq' and 'ir'.
 * While any value for 'ip' and 'iq will do (provided that their product
 * is large enough to meet your modulus needs), 'ir' should be selected
 * to avoid values near 0 or near 'n' (or ip*iq).
 *
 * The Blum moduli used with the pre-defined generators (via the call
 * srandom(seed, 0<x<=16)) were generated using the above process.  The
 * 'ip', 'iq' and 'ir' values were produced by special purpose hardware
 * that produced cryptographically strong random numbers.  The Blum
 * primes used in these special pre-defined generators are unknown.
 *
 * I (Landon Curt Noll) did not keep the search values of these 20 special
 * pre-defined generators.  While some of the smaller Blum moduli is
 * within the range of some factoring methods, others are not.  As of
 * Jan 1996, the following is the estimate of what can factor the
 * pre-defined moduli:
 *
 *	1 <= newn < 4	PC using ECM in a short amount of time
 *	5 <= newn < 8	Workstation using MPQS in a short amount of time
 *	8 <= newn < 12	High end supercomputer or high parallel processor
 *			using state of the art factoring over a long time
 *     12 <= newn < 16	Beyond Jan 1996 systems and factoring methods
 *     17 <= newn < 20	Well beyond Jan 1996 systems and factoring methods
 *
 * This is not to say that in the future things will not change.  One
 * can say that faster hardware will not help in the factoring of 1024+
 * bit Blum moduli found in 12 <= newn as well as in the default
 * Blum generator.  A combination of better algorithms, helped by faster
 * hardware will be needed.
 *
 * It is true that the the pre-defined moduli are 'magic'.  On the other
 * hand, there purpose was in part to produce users with pre-seeded
 * generators where the individual Blum primes are not well known.  If
 * this bothers you, don't use them and seed your own!
 *
 * The output of the Blum generator has been proven to be cryptographically
 * strong.  I.e., there is absolutely no better way to predict the next
 * bit in the sequence than by tossing a coin (as with TRULY random numbers)
 * EVEN IF YOU HAVE A LARGE NUMBER OF PREVIOUSLY GENERATED BITS AND KNOW
 * A LARGE NUMBER OF BITS THAT FOLLOW THE NEXT BIT!  An adversary would
 * be far better advised to try to factor the modulus or exhaustively search
 * for the quadratic residue in use.
 *
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
   /\../\	BASEB is assumed to be 16 or 32 	/\../\   !!!
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
 * current a55 generator state
 */
static RAND a55;


/*
 * current Blum generator state
 */
static RANDOM blum;


/*
 * declare static functions
 */
static void randreseed64(ZVALUE seed, ZVALUE *res);
static int slotcp(BITSTR *bitstr, FULL *src, int count);
static void slotcp64(BITSTR *bitstr, FULL *src);


/*
 * Default Blum generator
 *
 * The init_blum generator is established via the srandom(0) call.
 * The random state value is ready to go except for the 'seeded' flag
 * which is set to 0.
 *
 * The post_init_blum is like init_blum except that the 'seeded' flag
 * is set.  This generator is used simply for comparison with an
 * initialized state.
 */
static CONST HALF h_ndefvec[] = {
	HVAL(ee7b,60b1), HVAL(4454,30bb), HVAL(8590,c15a), HVAL(3d5b,c591),
	HVAL(84f5,e8e5), HVAL(0016,3d7a), HVAL(f788,1fc9), HVAL(cbb9,17c1),
	HVAL(89cd,c200), HVAL(dfd4,abcd), HVAL(af9a,70be), HVAL(1779,f3d1),
	HVAL(a82a,e195), HVAL(20ab,4664), HVAL(3853,6428), HVAL(d90d,ad9a),
	HVAL(c63b,7a7b), HVAL(3da6,e1cf), HVAL(bf33,7444), HVAL(cdf0,61f9),
	HVAL(4657,96a1), HVAL(859c,d9c1), HVAL(8b29,d64a), HVAL(f3ab,af11),
	HVAL(cf72,b874), HVAL(2df1,82bc), HVAL(fcaa,b445), HVAL(8543,3800),
	HVAL(5374,2e0a), HVAL(0ffc,f5eb), HVAL(22e9,4705), HVAL(06a6,cecc),
	(HALF)0x2
};
static CONST ZVALUE z_ndefault = {(HALF *)h_ndefvec,
				  sizeof(h_ndefvec)/sizeof(HALF), 0};
static CONST HALF h_rdefvec[] = {
	HVAL(0687,e467), HVAL(f4b9,462d), HVAL(642e,37b7), HVAL(9e25,4065),
	HVAL(5e0d,6033), HVAL(f1ea,9cf7), HVAL(e776,6c67), HVAL(4c61,0137),
	HVAL(c0ac,85e1), HVAL(729b,672f), HVAL(034f,27c6), HVAL(beb0,b023),
	HVAL(c632,02c7), HVAL(3fb1,f402), HVAL(ab4f,03f4), HVAL(198d,62cf),
	HVAL(bbfd,e64a), HVAL(a763,5abf), HVAL(bfd6,9a6f), HVAL(b438,fba4),
	HVAL(7d46,db41), HVAL(0ef5,408f), HVAL(629b,8e84), HVAL(9112,6b68),
	HVAL(a78b,7225), HVAL(7820,8179), HVAL(3411,4f7e), HVAL(f114,f4ab),
	HVAL(7ff7,855e), HVAL(1501,002a), HVAL(ea73,df59), HVAL(455b,0e84),
	(HALF)0x1
};
static CONST ZVALUE z_rdefault = {(HALF *)h_rdefvec,
				  sizeof(h_rdefvec)/sizeof(HALF), 0};
static CONST RANDOM init_blum = {0, 0, 8, (HALF)0, (HALF)0x3ff,
				 (ZVALUE *)&z_ndefault,
				 (ZVALUE *)&z_rdefault};
static CONST RANDOM post_init_blum = {1, 0, 8, (HALF)0, (HALF)0x3ff,
				      (ZVALUE *)&z_ndefault,
				      (ZVALUE *)&z_rdefault};


/*
 * Pre-defined Blum generators
 *
 * These generators are seeded via the srandom(0, newn) where
 * 1 <= newn < BLUM_PREGEN.
 */
static CONST HALF h_nvec1[] = {
	HVAL(83de,9361), HVAL(f0db,722d), HVAL(6fe3,28ca), HVAL(0494,4073),
	(HALF)0x5
};
static CONST ZVALUE z_nvec1 = {(HALF *)h_nvec1,
			       sizeof(h_nvec1)/sizeof(HALF), 0};
static CONST HALF h_rvec1[] = {
	HVAL(a4cc,42ec), HVAL(4e5d,bb01), HVAL(11d9,52e7), HVAL(b226,980f)
};
static CONST ZVALUE z_rvec1 = {(HALF *)h_rvec1,
			       sizeof(h_rvec1)/sizeof(HALF), 0};
static CONST HALF h_nvec2[] = {
	HVAL(3534,43f1), HVAL(eb28,6ea9), HVAL(dd37,4a18), HVAL(348a,2555),
	(HALF)0x2c5
};
static CONST ZVALUE z_nvec2 = {(HALF *)h_nvec2,
			       sizeof(h_nvec2)/sizeof(HALF), 0};
static CONST HALF h_rvec2[] = {
	HVAL(21e3,a218), HVAL(e893,616b), HVAL(6cd7,10e3), HVAL(f3d6,4344),
	(HALF)0x40
};
static CONST ZVALUE z_rvec2 = {(HALF *)h_rvec2,
			       sizeof(h_rvec2)/sizeof(HALF), 0};
static CONST HALF h_nvec3[] = {
	HVAL(11d0,01f1), HVAL(f2ca,661f), HVAL(3a81,f1e0), HVAL(59d6,ce4e),
	HVAL(0009,cfd9)
};
static CONST ZVALUE z_nvec3 = {(HALF *)h_nvec3,
			       sizeof(h_nvec3)/sizeof(HALF), 0};
static CONST HALF h_rvec3[] = {
	HVAL(a0d7,d76a), HVAL(3e14,2de2), HVAL(ff5c,ea4f), HVAL(b44d,9b64),
	(HALF)0xfae5
};
static CONST ZVALUE z_rvec3 = {(HALF *)h_rvec3,
			       sizeof(h_rvec3)/sizeof(HALF), 0};
static CONST HALF h_nvec4[] = {
	HVAL(dfcc,0751), HVAL(2dec,c680), HVAL(5df1,2a1a), HVAL(5c89,4ed7),
	HVAL(3070,f924)
};
static CONST ZVALUE z_nvec4 = {(HALF *)h_nvec4,
			       sizeof(h_nvec4)/sizeof(HALF), 0};
static CONST HALF h_rvec4[] = {
	HVAL(4b98,4570), HVAL(a220,ddba), HVAL(a2c0,af8a), HVAL(131b,2bdc),
	HVAL(0020,c2d8)
};
static CONST ZVALUE z_rvec4 = {(HALF *)h_rvec4,
			       sizeof(h_rvec4)/sizeof(HALF), 0};
static CONST HALF h_nvec5[] = {
	HVAL(9916,6ef1), HVAL(8b99,e5e7), HVAL(8769,a010), HVAL(5d3f,e111),
	HVAL(680b,c2fa), HVAL(38f7,5aac), HVAL(db81,a85b), HVAL(109b,1822),
	(HALF)0x2
};
static CONST ZVALUE z_nvec5 = {(HALF *)h_nvec5,
			       sizeof(h_nvec5)/sizeof(HALF), 0};
static CONST HALF h_rvec5[] = {
	HVAL(59e2,efa9), HVAL(0e6c,77c8), HVAL(1e70,aeed), HVAL(234f,7b7d),
	HVAL(5f5d,f6db), HVAL(e821,a960), HVAL(ae33,b792), HVAL(5e9b,890e)
};
static CONST ZVALUE z_rvec5 = {(HALF *)h_rvec5,
			       sizeof(h_rvec5)/sizeof(HALF), 0};
static CONST HALF h_nvec6[] = {
	HVAL(e1dd,f431), HVAL(d855,57f1), HVAL(5ee7,32da), HVAL(3a38,db77),
	HVAL(5c64,4026), HVAL(f2db,f218), HVAL(9ada,2c79), HVAL(7bfd,9d7d),
	(HALF)0xa
};
static CONST ZVALUE z_nvec6 = {(HALF *)h_nvec6,
			       sizeof(h_nvec6)/sizeof(HALF), 0};
static CONST HALF h_rvec6[] = {
	HVAL(c940,4daf), HVAL(c5dc,2e80), HVAL(2c98,eccf), HVAL(e1f3,495d),
	HVAL(ce1c,925c), HVAL(e097,aede), HVAL(8866,7154), HVAL(5e94,a02f)
};
static CONST ZVALUE z_rvec6 = {(HALF *)h_rvec6,
			       sizeof(h_rvec6)/sizeof(HALF), 0};
static CONST HALF h_nvec7[] = {
	HVAL(cf9e,c751), HVAL(602f,9125), HVAL(5288,2e7f), HVAL(0dcf,53ce),
	HVAL(ff56,9d6b), HVAL(6286,43fc), HVAL(3780,1cd5), HVAL(f239,9ef2),
	HVAL(43d8,7de8)
};
static CONST ZVALUE z_nvec7 = {(HALF *)h_nvec7,
			       sizeof(h_nvec7)/sizeof(HALF), 0};
static CONST HALF h_rvec7[] = {
	HVAL(098d,25e6), HVAL(3992,d2e5), HVAL(64f0,b58c), HVAL(cf18,d4dd),
	HVAL(9d87,6aef), HVAL(7acc,ed04), HVAL(bfbe,9076), HVAL(1ee0,14c7),
	HVAL(0013,522d)
};
static CONST ZVALUE z_rvec7 = {(HALF *)h_rvec7,
			       sizeof(h_rvec7)/sizeof(HALF), 0};
static CONST HALF h_nvec8[] = {
	HVAL(2674,2f11), HVAL(bc42,e66a), HVAL(b59c,d9f0), HVAL(9ad4,a6c2),
	HVAL(5bdb,d2f9), HVAL(bdc9,1fed), HVAL(f13c,9ce7), HVAL(eb46,99b7),
	HVAL(4712,6ca7), (HALF)0x58
};
static CONST ZVALUE z_nvec8 = {(HALF *)h_nvec8,
			       sizeof(h_nvec8)/sizeof(HALF), 0};
static CONST HALF h_rvec8[] = {
	HVAL(489d,c674), HVAL(aae9,5f3a), HVAL(a35d,a929), HVAL(5597,b4b8),
	HVAL(28e9,c947), HVAL(3d34,4f9a), HVAL(b7e6,61fa), HVAL(a326,9116),
	HVAL(8530,16dc)
};
static CONST ZVALUE z_rvec8 = {(HALF *)h_rvec8,
			       sizeof(h_rvec8)/sizeof(HALF), 0};
static CONST HALF h_nvec9[] = {
	HVAL(ab27,e3d1), HVAL(1274,5db4), HVAL(b980,f951), HVAL(62b1,6b66),
	HVAL(0fde,ce0d), HVAL(6061,c6fd), HVAL(36a6,ff09), HVAL(e08e,b61c),
	HVAL(84d8,95c3), HVAL(4a86,752a), HVAL(c179,7b4f), HVAL(5621,57a3),
	HVAL(3d26,7bb0), HVAL(14e8,1b00), HVAL(218d,9238), HVAL(5232,2fd3),
	HVAL(0039,e8be)
};
static CONST ZVALUE z_nvec9 = {(HALF *)h_nvec9,
			       sizeof(h_nvec9)/sizeof(HALF), 0};
static CONST HALF h_rvec9[] = {
	HVAL(7d4e,d20d), HVAL(601e,f2b8), HVAL(8e59,f959), HVAL(edaa,5d9e),
	HVAL(309a,89ba), HVAL(e5ab,7d81), HVAL(796b,2545), HVAL(02de,3222),
	HVAL(8357,c0bd), HVAL(0107,e3fd), HVAL(82d9,d288), HVAL(bc42,a8aa),
	HVAL(4b78,7343), HVAL(c015,0886), HVAL(bab9,15bf), HVAL(f8ad,1e6b),
	(HALF)0xb458
};
static CONST ZVALUE z_rvec9 = {(HALF *)h_rvec9,
			       sizeof(h_rvec9)/sizeof(HALF), 0};
static CONST HALF h_nvec10[] = {
	HVAL(b7e6,4b89), HVAL(c3cd,c363), HVAL(2ef9,c73c), HVAL(6092,ce22),
	HVAL(02ab,e36c), HVAL(08d4,9573), HVAL(7451,1c40), HVAL(d385,82de),
	HVAL(a524,a02f), HVAL(52c8,1b3b), HVAL(250d,3cc9), HVAL(23b5,0e88),
	HVAL(bd14,48bf), HVAL(882d,7f98), HVAL(c23e,f596), HVAL(c905,5666),
	HVAL(025f,2435)
};
static CONST ZVALUE z_nvec10 = {(HALF *)h_nvec10,
			       sizeof(h_nvec10)/sizeof(HALF), 0};
static CONST HALF h_rvec10[] = {
	HVAL(94cf,c482), HVAL(594f,5ad4), HVAL(2344,2aee), HVAL(145f,40ce),
	HVAL(1baf,950d), HVAL(adc4,f175), HVAL(f62c,669f), HVAL(8d07,5d56),
	HVAL(08ed,8b40), HVAL(aaf2,cf30), HVAL(c24b,5ffb), HVAL(250d,f8cf),
	HVAL(7ca8,1ec9), HVAL(787e,2b70), HVAL(1840,1468), HVAL(47b2,0e0c),
	HVAL(0066,bb7e)
};
static CONST ZVALUE z_rvec10 = {(HALF *)h_rvec10,
			       sizeof(h_rvec10)/sizeof(HALF), 0};
static CONST HALF h_nvec11[] = {
	HVAL(546e,e069), HVAL(2e1a,530c), HVAL(2014,dab2), HVAL(a729,cf52),
	HVAL(920e,e1a9), HVAL(68f2,7533), HVAL(2587,3cfa), HVAL(dd37,a749),
	HVAL(4499,daa2), HVAL(286e,5870), HVAL(57f3,f9b6), HVAL(5ec5,4467),
	HVAL(69a7,91ea), HVAL(874e,cd77), HVAL(4217,d56b), HVAL(82bd,b309),
	HVAL(4978,64de)
};
static CONST ZVALUE z_nvec11 = {(HALF *)h_nvec11,
			       sizeof(h_nvec11)/sizeof(HALF), 0};
static CONST HALF h_rvec11[] = {
	HVAL(56e3,8b04), HVAL(3a0a,ded3), HVAL(461d,88b1), HVAL(9c09,4d65),
	HVAL(e533,3fed), HVAL(34d9,18fe), HVAL(1ef5,6281), HVAL(cedf,a07c),
	HVAL(590f,47fb), HVAL(a2c5,4d5c), HVAL(7323,39ee), HVAL(8065,49a7),
	HVAL(9ce3,163f), HVAL(ae3a,f8b6), HVAL(264a,4465), HVAL(1cb5,e630),
	HVAL(0086,8488)
};
static CONST ZVALUE z_rvec11 = {(HALF *)h_rvec11,
			       sizeof(h_rvec11)/sizeof(HALF), 0};
static CONST HALF h_nvec12[] = {
	HVAL(f14c,7b99), HVAL(7f66,d151), HVAL(87ef,ad2b), HVAL(57d3,f098),
	HVAL(d653,4165), HVAL(812f,dd25), HVAL(48c7,c7ce), HVAL(a1bf,41e0),
	HVAL(4c94,e315), HVAL(190b,1593), HVAL(ee42,51da), HVAL(2b4a,1a66),
	HVAL(2bb7,c2a1), HVAL(65b1,8ca9), HVAL(08b8,9116), HVAL(c0cc,b15f),
	HVAL(5758,2ab3), (HALF)0x34
};
static CONST ZVALUE z_nvec12 = {(HALF *)h_nvec12,
			       sizeof(h_nvec12)/sizeof(HALF), 0};
static CONST HALF h_rvec12[] = {
	HVAL(e207,b4a0), HVAL(5227,dd68), HVAL(9488,fbc4), HVAL(6ed0,81aa),
	HVAL(8e73,6fe5), HVAL(3dd2,c020), HVAL(eeb0,7c57), HVAL(0b60,4eb7),
	HVAL(a13f,72a1), HVAL(cbfc,4333), HVAL(a4c5,e8cd), HVAL(0add,520d),
	HVAL(758c,a3b2), HVAL(5549,0137), HVAL(5870,babd), HVAL(f648,ed93),
	HVAL(df71,9bd1)
};
static CONST ZVALUE z_rvec12 = {(HALF *)h_rvec12,
			       sizeof(h_rvec12)/sizeof(HALF), 0};
static CONST HALF h_nvec13[] = {
	HVAL(3314,fc49), HVAL(cca2,0032), HVAL(208e,3420), HVAL(8aaa,503a),
	HVAL(d79a,63cc), HVAL(b4ed,7417), HVAL(95dd,1892), HVAL(b591,5f64),
	HVAL(d14c,c7f1), HVAL(1589,917e), HVAL(b2b0,5667), HVAL(c32d,99cb),
	HVAL(1b5a,1a84), HVAL(a493,22a1), HVAL(dd3a,76c3), HVAL(8c07,137f),
	HVAL(aaf8,3c63), HVAL(3757,5113), HVAL(a8b1,8e84), HVAL(ceec,891d),
	HVAL(78c1,ee99), HVAL(6e49,e256), HVAL(4286,bfd6), HVAL(cb6b,f6a9),
	HVAL(7bda,8ee0), HVAL(d439,510a), HVAL(63f4,345b), HVAL(959a,5535),
	HVAL(daf6,6d82), HVAL(ed03,d833), HVAL(1b5a,f734), HVAL(166b,7dd2),
	HVAL(0151,7c19)
};
static CONST ZVALUE z_nvec13 = {(HALF *)h_nvec13,
			       sizeof(h_nvec13)/sizeof(HALF), 0};
static CONST HALF h_rvec13[] = {
	HVAL(6b77,36f5), HVAL(2407,bfe4), HVAL(965e,2072), HVAL(cc26,cf3e),
	HVAL(a432,b567), HVAL(2ed0,07ab), HVAL(0e2f,67b9), HVAL(ef64,0960),
	HVAL(be5f,1ad3), HVAL(3fae,da1b), HVAL(a8f6,b988), HVAL(e5c9,cea5),
	HVAL(a674,45ea), HVAL(3935,ce78), HVAL(f445,ff06), HVAL(beda,0a11),
	HVAL(92de,080b), HVAL(f404,9026), HVAL(e509,f0b8), HVAL(6f05,216f),
	HVAL(5a68,dc14), HVAL(548f,730d), HVAL(e9fd,9f00), HVAL(64a4,aada),
	HVAL(e3bb,dd15), HVAL(cb4b,e7a5), HVAL(17dd,d162), HVAL(c491,8c33),
	HVAL(9b70,6d2b), HVAL(8b04,f6a6), HVAL(1263,fa64), HVAL(8e9a,560d),
	(HALF)0xd42e
};
static CONST ZVALUE z_rvec13 = {(HALF *)h_rvec13,
			       sizeof(h_rvec13)/sizeof(HALF), 0};
static CONST HALF h_nvec14[] = {
	HVAL(c116,af01), HVAL(bdef,8c0f), HVAL(c440,9a1a), HVAL(acb3,185c),
	HVAL(b33f,925b), HVAL(fee8,3005), HVAL(4b3d,b112), HVAL(7f07,6743),
	HVAL(2170,9223), HVAL(2159,054b), HVAL(6fdf,efe3), HVAL(792d,0a07),
	HVAL(1d14,bd52), HVAL(5f28,9a27), HVAL(e034,83c4), HVAL(cb86,a0c1),
	HVAL(0ede,912a), HVAL(f01a,33e3), HVAL(63fd,40c6), HVAL(4b05,86e4),
	HVAL(bc7e,4a03), HVAL(956b,22f3), HVAL(1056,0b22), HVAL(3e78,d321),
	HVAL(1791,61e2), HVAL(e85e,b909), HVAL(ec79,31ee), HVAL(b314,b4bf),
	HVAL(1f56,4618), HVAL(b9d9,83d0), HVAL(7479,ac07), HVAL(93c6,f4e8),
	HVAL(5e56,a00e)
};
static CONST ZVALUE z_nvec14 = {(HALF *)h_nvec14,
			       sizeof(h_nvec14)/sizeof(HALF), 0};
static CONST HALF h_rvec14[] = {
	HVAL(0ff9,f190), HVAL(47a4,db68), HVAL(913c,c8ea), HVAL(b6b1,b220),
	HVAL(13ed,fbbb), HVAL(a8f1,f1c3), HVAL(d6d7,1f8f), HVAL(4194,649a),
	HVAL(7d49,7344), HVAL(677c,8416), HVAL(0186,b983), HVAL(ee63,3901),
	HVAL(ce64,d69d), HVAL(61a7,04b3), HVAL(1383,52b2), HVAL(c0fb,58d8),
	HVAL(16bf,2073), HVAL(56c9,ae78), HVAL(b81a,0a68), HVAL(1512,abaf),
	HVAL(6b99,36ba), HVAL(4335,0dfc), HVAL(a0ea,19d2), HVAL(e861,34c5),
	HVAL(6c86,563f), HVAL(6b0c,5b68), HVAL(75f6,27fc), HVAL(b609,913f),
	HVAL(15b9,a564), HVAL(9b18,f154), HVAL(6ef0,c5d0), HVAL(b673,3509),
	HVAL(00f7,aa7c)
};
static CONST ZVALUE z_rvec14 = {(HALF *)h_rvec14,
			       sizeof(h_rvec14)/sizeof(HALF), 0};
static CONST HALF h_nvec15[] = {
	HVAL(c8d9,7079), HVAL(061e,7597), HVAL(f5d2,c721), HVAL(299b,c51f),
	HVAL(ffe6,c337), HVAL(1979,8624), HVAL(ee6f,92b6), HVAL(0b1d,0c7a),
	HVAL(b530,8231), HVAL(49c5,58dd), HVAL(196a,530e), HVAL(0caa,515c),
	HVAL(8b0d,86ed), HVAL(380a,8fa0), HVAL(80df,03e4), HVAL(e962,d81b),
	HVAL(c1a7,83b3), HVAL(c327,8ccc), HVAL(3e72,ab86), HVAL(ebb9,1675),
	HVAL(b902,c2b8), HVAL(a059,0445), HVAL(4f40,de42), HVAL(aa95,aac1),
	HVAL(fc0f,2e82), HVAL(70ca,b84b), HVAL(5326,a267), HVAL(470b,e607),
	HVAL(4535,2ebe), HVAL(5ba8,1ca8), HVAL(02c4,6c17), HVAL(9edf,bcdb),
	HVAL(97dd,840b)
};
static CONST ZVALUE z_nvec15 = {(HALF *)h_nvec15,
			       sizeof(h_nvec15)/sizeof(HALF), 0};
static CONST HALF h_rvec15[] = {
	HVAL(6c3e,2110), HVAL(808f,0aaa), HVAL(d98d,b92e), HVAL(1e6a,bd43),
	HVAL(f401,b920), HVAL(9d3f,0381), HVAL(db95,d174), HVAL(a2f6,5c33),
	HVAL(0c54,69f8), HVAL(a3c1,26fc), HVAL(8866,241b), HVAL(0e46,eca7),
	HVAL(a705,57fa), HVAL(8e73,91a6), HVAL(70e4,a9e2), HVAL(9ad9,7e89),
	HVAL(1eb7,7dee), HVAL(fe62,24d0), HVAL(dbe5,22b1), HVAL(a4aa,1023),
	HVAL(7f22,6860), HVAL(60ef,4379), HVAL(45e3,b296), HVAL(cc31,f5ef),
	HVAL(74fb,f6d6), HVAL(55b1,c25d), HVAL(3ede,521f), HVAL(df8c,ef42),
	HVAL(a8d7,7ca6), HVAL(b500,25da), HVAL(69ab,99f9), HVAL(03b8,c758),
	HVAL(00b8,2207)
};
static CONST ZVALUE z_rvec15 = {(HALF *)h_rvec15,
			       sizeof(h_rvec15)/sizeof(HALF), 0};
static CONST HALF h_nvec16[] = {
	HVAL(d843,46e1), HVAL(1841,83f6), HVAL(2dc9,bd36), HVAL(4ca8,57ac),
	HVAL(96a5,828d), HVAL(ed1f,1c59), HVAL(36d9,731f), HVAL(bd3f,6183),
	HVAL(de0f,5578), HVAL(b6a2,ea8a), HVAL(be99,3c44), HVAL(0e28,3c05),
	HVAL(d7cf,61e3), HVAL(40fe,15b6), HVAL(534d,967e), HVAL(3469,1046),
	HVAL(d408,45bd), HVAL(d69c,ee4b), HVAL(557f,ee8d), HVAL(51c8,56ba),
	HVAL(e6bc,51ba), HVAL(587b,c173), HVAL(1959,e379), HVAL(9282,8439),
	HVAL(311e,0503), HVAL(7f3c,9cc2), HVAL(d426,512c), HVAL(e8b2,b497),
	HVAL(9e43,536a), HVAL(9f54,4cb8), HVAL(b56f,84c3), HVAL(b82f,bb12),
	HVAL(6e34,8549), (HALF)0x45
};
static CONST ZVALUE z_nvec16 = {(HALF *)h_nvec16,
			       sizeof(h_nvec16)/sizeof(HALF), 0};
static CONST HALF h_rvec16[] = {
	HVAL(7b4e,8830), HVAL(7060,5db8), HVAL(a1ab,e4a5), HVAL(a70f,be04),
	HVAL(2bcd,a8f4), HVAL(d29a,da9a), HVAL(55ad,0560), HVAL(b367,137e),
	HVAL(d697,2f1a), HVAL(809b,ad45), HVAL(b15d,2454), HVAL(0c0d,415f),
	HVAL(416e,117a), HVAL(e87a,9521), HVAL(670a,5e1a), HVAL(53a4,1772),
	HVAL(fc9c,5cc1), HVAL(f756,45df), HVAL(86f6,d19f), HVAL(9404,b5bf),
	HVAL(56e9,d83b), HVAL(ac0f,3bc3), HVAL(a150,8c4b), HVAL(4bfd,4977),
	HVAL(7192,2540), HVAL(f252,4def), HVAL(8a81,a3db), HVAL(ced8,28de),
	HVAL(7895,ae8f), HVAL(4a4b,6dcd), HVAL(973e,921a), HVAL(9fb2,7a07),
	HVAL(b0d7,dcb1)
};
static CONST ZVALUE z_rvec16 = {(HALF *)h_rvec16,
			       sizeof(h_rvec16)/sizeof(HALF), 0};
static CONST HALF h_nvec17[] = {
	HVAL(72b7,2051), HVAL(edc2,4ebf), HVAL(e970,a8d1), HVAL(66c9,b150),
	HVAL(cbb9,27f7), HVAL(b574,ffd9), HVAL(4166,b249), HVAL(0fce,4030),
	HVAL(fa69,22ca), HVAL(39cc,14a9), HVAL(1439,6e2a), HVAL(aff7,4c7f),
	HVAL(a120,a314), HVAL(e11a,2700), HVAL(44c9,ad30), HVAL(7f32,8d72),
	HVAL(ab2c,eaf7), HVAL(868f,f772), HVAL(0974,f0b3), HVAL(20e9,f0f6),
	HVAL(cd4e,5b8a), HVAL(0bde,26e6), HVAL(96f6,c3ac), HVAL(5718,c601),
	HVAL(2f11,7710), HVAL(1ab0,876e), HVAL(49ab,2c2e), HVAL(7470,22b9),
	HVAL(6e9d,e4a7), HVAL(25e8,8f2e), HVAL(d7d0,a00b), HVAL(c8ff,11f6),
	HVAL(f50a,a819), HVAL(be53,0e9e), HVAL(47b7,ff54), HVAL(e020,9b46),
	HVAL(027e,c5eb), HVAL(0754,3362), HVAL(531e,9b85), HVAL(3c23,b568),
	HVAL(5d07,af7a), HVAL(d494,8461), HVAL(2eb9,a499), HVAL(1c71,fa0b),
	HVAL(7025,e22b), HVAL(4d27,20b9), HVAL(531b,8d54), HVAL(66e2,fc30),
	HVAL(dac7,cafd), HVAL(7f29,953b), HVAL(9d45,6bff), HVAL(8398,14bc),
	HVAL(d0e5,fb19), HVAL(5a6c,9f58), HVAL(3a5a,9dc3), HVAL(598b,f28b),
	HVAL(a7a9,1144), HVAL(6849,4f76), HVAL(bfed,d7be), HVAL(7ca5,4266),
	HVAL(96e0,faf9), HVAL(33be,3c0f), HVAL(ffa3,040b), HVAL(813a,eac0),
	(HALF)0x6177
};
static CONST ZVALUE z_nvec17 = {(HALF *)h_nvec17,
			       sizeof(h_nvec17)/sizeof(HALF), 0};
static CONST HALF h_rvec17[] = {
	HVAL(22b4,1dac), HVAL(d625,8005), HVAL(2aa1,e0cb), HVAL(45d1,47b5),
	HVAL(bf5c,46d9), HVAL(14c9,dadf), HVAL(09b0,aec4), HVAL(4286,bfef),
	HVAL(c6f8,e9d1), HVAL(dd68,467b), HVAL(93f4,ffb9), HVAL(58f2,eb51),
	HVAL(2ade,048f), HVAL(eaca,e6e5), HVAL(8dd2,a807), HVAL(bcea,8c27),
	HVAL(02a0,3281), HVAL(039a,eb6d), HVAL(fa6e,016c), HVAL(6fda,1b09),
	HVAL(ea77,19ed), HVAL(cf2e,0294), HVAL(a426,4cb9), HVAL(4988,8af1),
	HVAL(1b44,f0c1), HVAL(3cce,e577), HVAL(dbeb,d170), HVAL(f743,1e7e),
	HVAL(faeb,d584), HVAL(896e,4a33), HVAL(e46b,f43c), HVAL(17fe,9a10),
	HVAL(b532,1b51), HVAL(f7e1,d2a9), HVAL(e7fe,0a56), HVAL(bd73,6750),
	HVAL(cf02,9a33), HVAL(ebee,99b1), HVAL(810f,ff31), HVAL(694c,8d30),
	HVAL(c8c1,8689), HVAL(c2f9,f4fb), HVAL(5949,fd7f), HVAL(67aa,f7b3),
	HVAL(a82f,906a), HVAL(1b84,b7b3), HVAL(eac0,52ee), HVAL(1a4e,9345),
	HVAL(ce3c,2973), HVAL(4a51,68a7), HVAL(5c55,51ba), HVAL(77c6,cb26),
	HVAL(fa45,a3a6), HVAL(486f,31e0), HVAL(caf9,7519), HVAL(be4b,0399),
	HVAL(802f,c106), HVAL(5372,84da), HVAL(20c4,e167), HVAL(2a62,f329),
	HVAL(c2d2,fc5b), HVAL(dd66,5324), HVAL(c3b8,adf1), HVAL(0b6e,af3b),
	(HALF)0x5372
};
static CONST ZVALUE z_rvec17 = {(HALF *)h_rvec17,
			       sizeof(h_rvec17)/sizeof(HALF), 0};
static CONST HALF h_nvec18[] = {
	HVAL(c8b7,8629), HVAL(4135,1b18), HVAL(28ad,4ed8), HVAL(c96f,7df1),
	HVAL(7cd3,c931), HVAL(0f23,036a), HVAL(ac65,7631), HVAL(6a62,5812),
	HVAL(0814,4788), HVAL(8642,ed62), HVAL(7619,8a40), HVAL(70de,fd64),
	HVAL(97fb,673c), HVAL(6f3b,ddf6), HVAL(72fe,2977), HVAL(20ed,82f1),
	HVAL(7e7f,4fdc), HVAL(dc27,2d6b), HVAL(6d77,f317), HVAL(2c59,5b15),
	HVAL(1c3b,7dd6), HVAL(6e3d,6147), HVAL(8170,640c), HVAL(b660,033f),
	HVAL(f211,886b), HVAL(bcc6,7859), HVAL(18ff,4b93), HVAL(3068,e691),
	HVAL(9db5,d823), HVAL(72af,d4ef), HVAL(5aa4,cd1a), HVAL(a0a3,3014),
	HVAL(6b83,49e6), HVAL(2f4d,e595), HVAL(d518,0384), HVAL(19d9,4118),
	HVAL(369e,7534), HVAL(ce4d,3b18), HVAL(119e,f9ee), HVAL(e2f4,5c25),
	HVAL(e8ea,e0a7), HVAL(62e4,1605), HVAL(6346,d2ca), HVAL(6425,625b),
	HVAL(44b0,33de), HVAL(1711,e6b3), HVAL(f3a9,d02e), HVAL(259c,d965),
	HVAL(08aa,956b), HVAL(6ad6,4380), HVAL(e973,0e8e), HVAL(539b,9d28),
	HVAL(e540,7950), HVAL(8990,0be4), HVAL(de12,18f6), HVAL(63e1,e52b),
	HVAL(0de0,3f4e), HVAL(8e21,a568), HVAL(268d,7ee3), HVAL(afb3,514e),
	HVAL(5378,efcb), HVAL(fec0,c7c6), HVAL(f07c,b724), HVAL(fb61,b42a),
	HVAL(068f,2a38)
};
static CONST ZVALUE z_nvec18 = {(HALF *)h_nvec18,
			       sizeof(h_nvec18)/sizeof(HALF), 0};
static CONST HALF h_rvec18[] = {
	HVAL(35ea,3c63), HVAL(8df2,ef97), HVAL(a2b3,afb7), HVAL(1791,58f6),
	HVAL(0492,0dba), HVAL(f333,077e), HVAL(f830,4b5a), HVAL(230f,f2ae),
	HVAL(84a8,f3f0), HVAL(adda,164e), HVAL(c9a1,c944), HVAL(c705,02f2),
	HVAL(41a3,c18f), HVAL(09bd,3254), HVAL(9736,65a9), HVAL(1548,c263),
	HVAL(5024,d916), HVAL(0a3d,dde9), HVAL(f2aa,f1f5), HVAL(666d,b92a),
	HVAL(3a53,5aa5), HVAL(49c3,5775), HVAL(c381,a1c4), HVAL(f8d3,6dbc),
	HVAL(e94b,e870), HVAL(430e,88a6), HVAL(3721,9a06), HVAL(5109,df80),
	HVAL(e73c,a03f), HVAL(f1bb,4541), HVAL(3c6f,32f3), HVAL(952c,fc24),
	HVAL(fbc9,697f), HVAL(c5b8,d472), HVAL(cbbe,da4b), HVAL(7303,db3b),
	HVAL(a18f,255e), HVAL(e1d3,353d), HVAL(e8c9,8700), HVAL(9e75,e8fd),
	HVAL(3ddd,812e), HVAL(7340,f891), HVAL(b1f3,69ac), HVAL(764d,505b),
	HVAL(b13e,f51b), HVAL(16c3,aa43), HVAL(61d0,42c4), HVAL(22ac,0339),
	HVAL(3f30,6fd1), HVAL(4992,6f7f), HVAL(2b4c,575c), HVAL(9f3a,b467),
	HVAL(5dac,65af), HVAL(6277,8dc7), HVAL(9911,3a89), HVAL(49c0,540a),
	HVAL(1df7,0ac2), HVAL(4be1,2c5e), HVAL(e5d3,6bdb), HVAL(66b9,9ff5),
	HVAL(5358,be89), HVAL(4cd8,35d7), HVAL(f0d5,cda8), HVAL(1f1a,c6c3),
	HVAL(0473,5e92)
};
static CONST ZVALUE z_rvec18 = {(HALF *)h_rvec18,
			       sizeof(h_rvec18)/sizeof(HALF), 0};
static CONST HALF h_nvec19[] = {
	HVAL(6b65,9a79), HVAL(0239,c12d), HVAL(d204,df49), HVAL(1d4a,e0c7),
	HVAL(099b,f000), HVAL(6435,ade8), HVAL(dc4a,f029), HVAL(2f4e,e7a2),
	HVAL(adfc,f1e3), HVAL(7335,8f43), HVAL(687e,ede5), HVAL(b567,cd4d),
	HVAL(c7a7,814f), HVAL(c306,624c), HVAL(a82d,80c6), HVAL(3f39,0cd5),
	HVAL(7b7d,ec3f), HVAL(8bdb,1416), HVAL(275b,3a52), HVAL(9218,84fe),
	HVAL(94ac,02e0), HVAL(62f2,b52c), HVAL(cdc9,92ee), HVAL(35e5,5eeb),
	HVAL(69a4,3fd5), HVAL(44c1,dcfb), HVAL(3cdf,6227), HVAL(23f3,148f),
	HVAL(4250,8e4c), HVAL(95b7,37c3), HVAL(70af,831f), HVAL(2ee8,15c9),
	HVAL(47a4,7251), HVAL(7c11,b2af), HVAL(664c,361f), HVAL(cada,a841),
	HVAL(3d97,172a), HVAL(8740,2ffb), HVAL(a0a0,2ebd), HVAL(b674,225f),
	HVAL(6559,3fe2), HVAL(85f6,98b9), HVAL(5ed9,a7ab), HVAL(6bf3,7371),
	HVAL(7da7,5305), HVAL(0882,55bf), HVAL(8f60,7684), HVAL(1f3d,e57e),
	HVAL(118b,d501), HVAL(6833,770d), HVAL(d042,5c51), HVAL(2664,cacb),
	HVAL(9206,b920), HVAL(eb90,3b8d), HVAL(0e25,16b4), HVAL(36c3,d841),
	HVAL(51d7,cd17), HVAL(e063,ba1e), HVAL(f28f,4a6d), HVAL(244d,eb0d),
	HVAL(2e41,0ad4), HVAL(0721,c315), HVAL(dde2,7654), HVAL(2ad6,534b),
	HVAL(d678,8b25), HVAL(b23b,b9e8), HVAL(0023,0d7a)
};
static CONST ZVALUE z_nvec19 = {(HALF *)h_nvec19,
			       sizeof(h_nvec19)/sizeof(HALF), 0};
static CONST HALF h_rvec19[] = {
	HVAL(698e,f473), HVAL(3d53,a5b7), HVAL(0644,8319), HVAL(d9ad,4445),
	HVAL(6967,daa0), HVAL(a14c,6240), HVAL(78e7,7724), HVAL(63ef,2ab7),
	HVAL(8dff,2ee2), HVAL(662e,b424), HVAL(cd93,07d6), HVAL(0ab0,6a5d),
	HVAL(bdd7,b539), HVAL(8621,dd2d), HVAL(2bb4,a187), HVAL(1f1e,121d),
	HVAL(ce8d,b962), HVAL(dd3e,eaf1), HVAL(573b,6ca1), HVAL(6f46,0cd6),
	HVAL(6a8d,4780), HVAL(ea68,c7e6), HVAL(1148,eb32), HVAL(b43d,44d4),
	HVAL(b657,cb64), HVAL(8547,fdba), HVAL(85f7,3333), HVAL(c1f2,a51a),
	HVAL(1c05,ee52), HVAL(2847,c03d), HVAL(fc7a,88d8), HVAL(2e8d,d186),
	HVAL(5be3,4683), HVAL(d43d,2ee2), HVAL(9f2d,2bb5), HVAL(89b3,ddea),
	HVAL(7d78,4ae0), HVAL(c773,5e28), HVAL(967a,8608), HVAL(cdcb,cf07),
	HVAL(fc17,a423), HVAL(d36a,d053), HVAL(c73d,8892), HVAL(a635,c3f4),
	HVAL(9b5d,0cf9), HVAL(0ac7,3fd9), HVAL(e801,fefb), HVAL(b31c,ffd2),
	HVAL(f3ea,aa55), HVAL(0e74,fa23), HVAL(5414,b290), HVAL(6d17,6101),
	HVAL(5229,93f8), HVAL(f829,3dad), HVAL(713b,1e82), HVAL(b83b,bef0),
	HVAL(d001,cc62), HVAL(7537,da39), HVAL(7d80,158b), HVAL(9332,c8e2),
	HVAL(6fa6,fa75), HVAL(e21f,6512), HVAL(9995,18b4), HVAL(2196,605b),
	HVAL(e4fc,1798), HVAL(5f21,e245), HVAL(0008,f172)
};
static CONST ZVALUE z_rvec19 = {(HALF *)h_rvec19,
			       sizeof(h_rvec19)/sizeof(HALF), 0};
static CONST HALF h_nvec20[] = {
	HVAL(c3c1,d081), HVAL(4d26,2fce), HVAL(8765,cc91), HVAL(f372,7f7c),
	HVAL(abba,4bbc), HVAL(e098,5801), HVAL(fa36,5c51), HVAL(b2a4,b230),
	HVAL(f443,0a8d), HVAL(546b,98c8), HVAL(d974,8b26), HVAL(e255,a82f),
	HVAL(b00a,3e8c), HVAL(7069,676d), HVAL(6233,ccce), HVAL(0299,a74e),
	HVAL(d119,adf9), HVAL(5273,e811), HVAL(ae36,e0bb), HVAL(32b3,a486),
	HVAL(f049,21a5), HVAL(d31d,28a2), HVAL(da0c,50de), HVAL(2130,2b40),
	HVAL(ee9d,e552), HVAL(80bb,ac03), HVAL(75f4,9740), HVAL(c3d0,a61b),
	HVAL(341c,fbf6), HVAL(1615,b5b4), HVAL(4e3a,3def), HVAL(9573,4dbf),
	HVAL(e7ab,78ac), HVAL(ffdc,5cf9), HVAL(f799,6892), HVAL(4740,7ba4),
	HVAL(a78b,988f), HVAL(2773,6f42), HVAL(5064,ee50), HVAL(6013,5c56),
	HVAL(1ad7,3283), HVAL(2562,4bf7), HVAL(2ee2,1419), HVAL(9319,5abd),
	HVAL(66b6,7778), HVAL(b1e0,a42d), HVAL(729f,b0f0), HVAL(d492,1864),
	HVAL(2c42,253f), HVAL(302a,07a9), HVAL(bb74,1bd4), HVAL(932f,90ba),
	HVAL(f335,4be1), HVAL(0804,d661), HVAL(010e,9ba1), HVAL(1a05,778d),
	HVAL(a962,c833), HVAL(e759,0ee8), HVAL(be68,03b8), HVAL(c677,04c1),
	HVAL(56d7,9660), HVAL(6066,a3f3), HVAL(648b,0327), HVAL(267e,5b3a),
	HVAL(dddc,63a0), HVAL(3322,e890), HVAL(20e0,d8b1), HVAL(004f,d2b8)
};
static CONST ZVALUE z_nvec20 = {(HALF *)h_nvec20,
			       sizeof(h_nvec20)/sizeof(HALF), 0};
static CONST HALF h_rvec20[] = {
	HVAL(a048,bd1a), HVAL(95ab,dc7b), HVAL(98f4,7cf8), HVAL(126a,c98d),
	HVAL(aebf,85fd), HVAL(5650,580f), HVAL(3292,d7dd), HVAL(f49e,8377),
	HVAL(2947,ed46), HVAL(d1a5,b26c), HVAL(ae14,e6a1), HVAL(9b1f,5788),
	HVAL(4df7,27b2), HVAL(ee37,5079), HVAL(131b,c8e4), HVAL(294e,5f53),
	HVAL(1f57,59bd), HVAL(65d5,8acf), HVAL(598e,d3a5), HVAL(c393,61a6),
	HVAL(a783,fd7a), HVAL(264a,36a2), HVAL(6ca2,2856), HVAL(8ffb,171f),
	HVAL(1d7c,ea9e), HVAL(d81d,6fca), HVAL(34ea,730e), HVAL(31f5,6382),
	HVAL(b39c,d9e9), HVAL(440e,84be), HVAL(4b1d,15a1), HVAL(7bf7,75c5),
	HVAL(e40f,4638), HVAL(e5be,f0a7), HVAL(79e5,8942), HVAL(881a,e1ba),
	HVAL(01de,8372), HVAL(14cf,35f8), HVAL(e2d8,b310), HVAL(6696,1207),
	HVAL(de5d,5f91), HVAL(e6e7,0849), HVAL(74ec,5ac3), HVAL(e2de,4eb1),
	HVAL(4a41,dc20), HVAL(d306,d565), HVAL(b584,3ff3), HVAL(911b,30d6),
	HVAL(4e9c,d926), HVAL(8455,c9ae), HVAL(6944,8bb5), HVAL(0c7b,1aad),
	HVAL(1da1,e992), HVAL(c676,56bd), HVAL(c544,209e), HVAL(10ce,387c),
	HVAL(c4e8,8df8), HVAL(40e8,da88), HVAL(bb2c,3028), HVAL(4919,4fd9),
	HVAL(deef,17ee), HVAL(241b,c08d), HVAL(6fa9,f608), HVAL(4b0f,8b04),
	HVAL(ee96,0da1), HVAL(a309,9293), HVAL(8444,5fea), HVAL(0046,ef01)
};
static CONST ZVALUE z_rvec20 = {(HALF *)h_rvec20,
			       sizeof(h_rvec20)/sizeof(HALF), 0};
/* XXX - should be static -- change back once Blum generators use this array */
RANDOM random_pregen[BLUM_PREGEN] = {
    {1, 0, 7, (HALF)0, (HALF)0x07f, (ZVALUE *)&z_nvec1,  (ZVALUE *)&z_rvec1},
    {1, 0, 7, (HALF)0, (HALF)0x07f, (ZVALUE *)&z_nvec2,  (ZVALUE *)&z_rvec2},
    {1, 0, 7, (HALF)0, (HALF)0x07f, (ZVALUE *)&z_nvec3,  (ZVALUE *)&z_rvec3},
    {1, 0, 7, (HALF)0, (HALF)0x07f, (ZVALUE *)&z_nvec4,  (ZVALUE *)&z_rvec4},
    {1, 0, 8, (HALF)0, (HALF)0x0ff, (ZVALUE *)&z_nvec5,  (ZVALUE *)&z_rvec5},
    {1, 0, 8, (HALF)0, (HALF)0x0ff, (ZVALUE *)&z_nvec6,  (ZVALUE *)&z_rvec6},
    {1, 0, 8, (HALF)0, (HALF)0x0ff, (ZVALUE *)&z_nvec7,  (ZVALUE *)&z_rvec7},
    {1, 0, 8, (HALF)0, (HALF)0x0ff, (ZVALUE *)&z_nvec8,  (ZVALUE *)&z_rvec8},
    {1, 0, 9, (HALF)0, (HALF)0x1ff, (ZVALUE *)&z_nvec9,  (ZVALUE *)&z_rvec9},
    {1, 0, 9, (HALF)0, (HALF)0x1ff, (ZVALUE *)&z_nvec10, (ZVALUE *)&z_rvec10},
    {1, 0, 9, (HALF)0, (HALF)0x1ff, (ZVALUE *)&z_nvec11, (ZVALUE *)&z_rvec11},
    {1, 0, 9, (HALF)0, (HALF)0x1ff, (ZVALUE *)&z_nvec12, (ZVALUE *)&z_rvec12},
    {1, 0, 10, (HALF)0, (HALF)0x3ff, (ZVALUE *)&z_nvec13, (ZVALUE *)&z_rvec13},
    {1, 0, 10, (HALF)0, (HALF)0x3ff, (ZVALUE *)&z_nvec14, (ZVALUE *)&z_rvec14},
    {1, 0, 10, (HALF)0, (HALF)0x3ff, (ZVALUE *)&z_nvec15, (ZVALUE *)&z_rvec15},
    {1, 0, 10, (HALF)0, (HALF)0x3ff, (ZVALUE *)&z_nvec16, (ZVALUE *)&z_rvec16},
    {1, 0, 11, (HALF)0, (HALF)0x7ff, (ZVALUE *)&z_nvec17, (ZVALUE *)&z_rvec17},
    {1, 0, 11, (HALF)0, (HALF)0x7ff, (ZVALUE *)&z_nvec18, (ZVALUE *)&z_rvec18},
    {1, 0, 11, (HALF)0, (HALF)0x7ff, (ZVALUE *)&z_nvec19, (ZVALUE *)&z_rvec19},
    {1, 0, 11, (HALF)0, (HALF)0x7ff, (ZVALUE *)&z_nvec20, (ZVALUE *)&z_rvec20}
};


/*
 * Linear Congruential Constants
 *
 * 	a = 6316878969928993981 = 0x57aa0ff473c0ccbd
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
 * in note v).  Knuth suggests 'c==1' or 'c==a'.  We elect to select 'c'
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
 * Rand Book of Random Numbers.  Because m=2^64 is 20 decimal digits long,
 * we will search the Rand Book of Random Numbers 20 at a time.  We will
 * skip any of the 55 values that were used to initialize the additive 55
 * generators.  The values obtained from the Rand Book of Random Numbers are:
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
 *	((0-c) * minv(a,m)) % m	==> 10239951819489363767
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
 * This mapping need not be complex, random or secure.  All we attempt
 * to do here is to allow humans who pick small or successive seed values
 * to obtain reasonably different sequences from the generators below.
 *
 * NOTE: This is NOT a pseudo random number generator.  This function is
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
 *	pseed   - ptr to seed of the generator or NULL
 *	pmat55  - additive 55 state table or NULL
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
	a55.seeded = 0;	/* not seeded now */
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
        		indx = zdivi(zscram, i+1, &zscram);

        		/* do nothing if swap with itself */
        		if (indx == i) {
        			continue;
        		}

        		/* swap slot[i] with slot[indx] */
        		SSWAP(a55, i, indx);
        	}
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
 * zsrandom - seed the Blum generator
 *
 * seed > 0, n == NULL:
 *
 *	Seed the an internal additive 55 shuffle generator, and use it
 *	to produce an initial quadratic residue in the range:
 *
 *	    [2^(binsize*4/5), 2^(binsize-2))
 *
 *	where 2^(binsize-1) < n <= 2^binsize and 'n' is the current Blum
 *	modulus.  Here, binsize is the smallest power of 2 >= n.
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	    cur_state = srand(seed);
 *	    binsize = highbit(n)+1;	(* n is the current Blum modulus *)
 *	    r = pmod(rand(1<<ceil(binsize*4/5), 1<<(binsize-2)), 2, n);
 *	    junk = srand(cur_state);
 *
 * seed == 0, n == NULL:
 *
 *	Restore the initial state and modulus of the Blum generator.
 *	After this call, the Blum  generator is restored to its initial
 *	state after calc started.
 *
 * seed < 0, n == NULL:
 *
 *	Reserved for future use.
 *
 * seed == 0, n >= 1007:
 *
 *	If 'n' passes the tests (if applicable) specified by the "srandom"
 *	config value, it becomes the Blum modulus.  Any internally buffered
 *	random bits are flushed.
 *
 *	The initial quadratic residue 'r', is selected as if the following
 *	was executed:
 *
 *	    (* set Blum modulus to newn if allowed by "srandom" config value *)
 *	    (* and then set the initial quadratic residue by the next call *)
 *	    srandom(n % 2^309);
 *
 *	The first srand() call seeds the additive 55 shuffle generator
 *	with the lower 309 bits of n.  In actual practice, calc uses
 *	an independent internal rand() state value.
 *
 * seed > 0, n >= 1007:
 *
 *	If 'n' passes the tests (if applicable) specified by the "srandom"
 *	config value, it becomes the Blum modulus.   Once the Blum modulus
 *	is set, seed is used to seed an internal Additive 55 generator
 *	state which in turn is used to set the initial quadratic residue.
 *
 *	While not as efficient, this call is equivalent to:
 *
 *	    srandom(0, n);
 *	    srandom(seed);
 *
 * seed < 0, n >= NULL:
 *
 *	Reserved for future use.
 *
 * any seed, 20 < n < 1007:
 *
 *	Reserved for future use.
 *
 * any seed, n < 0:
 *
 *	Reserved for future use.
 *
 * seed == 0, 0 < n <= 20:
 *
 *	Seed with one of the predefined Blum moduli.  (see the comments
 *	near the top under the section 'INITIALIZATION AND SEEDS')
 *
 * seed > 0, 0 < n <= 20:
 *
 *	Use the same pre-defined Blum moduli 'n' noted above but use 'seed'
 *	to find a different quadratic residue 'r'.
 *
 *	While not as efficient, this call is equivalent to:
 *
 *	    srandom(0, n);
 *	    srandom(seed);
 *
 * seed < 0, 0 < n <= 20:
 *
 *	Use the same pre-defined Blum moduli 'n' noted above but use '-seed'
 *	to compute a different quadratic residue 'r'.
 *
 *	This call has the same effect as:
 *
 *	    srandom(0, n)
 *
 *	followed by the setting of the quadratic residue 'r' as follows:
 *
 *	    r = pmod(seed, 2, n)
 *
 *	where 'n' is the Blum moduli generated by 'srandom(0,newn)' and
 *	'r' is the new quadratic residue.
 *
 * given:
 *	pseed   - seed of the generator or NULL
 *	n    - ptr to n (Blum modulus), or NULL
 *
 * returns:
 *	previous Blum state
 */
/*XXX - use them*/
/*ARGSUSED*/
RANDOM *
zsrandom(CONST ZVALUE seed, CONST ZVALUE *n)
{
	RANDOM *ret;		/* previous Blum state */

	/*
	 * initialize state if first call
	 */
	if (!blum.seeded) {
		blum = init_blum;
		zcopy(*(init_blum.n), blum.n);
		zcopy(*(init_blum.r), blum.r);
		blum.seeded = 1;
	}

	/*
	 * save the current state to return later
	 */
	ret = (RANDOM *)malloc(sizeof(RANDOM));
	if (ret == NULL) {
		math_error("cannot allocate RANDOM state");
		/*NOTREACHED*/
	}
	/* move the ZVALUES over to ret */
	*ret = blum;


	/* XXX - finish this function */

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
 * zsetrandom - set the Blum generator state
 *
 * given:
 *	state - the state to copy
 *
 * returns:
 *	previous RANDOM state
 */
RANDOM *
zsetrandom(CONST RANDOM *state)
{
	RANDOM *ret;		/* previous Blum state */

	/*
	 * initialize state if first call
	 */
	if (!blum.seeded) {
		blum = init_blum;
		zcopy(*(init_blum.n), blum.n);
		zcopy(*(init_blum.r), blum.r);
		blum.seeded = 1;
	}

	/*
	 * save the current state to return later
	 */
	ret = (RANDOM *)malloc(sizeof(RANDOM));
	if (ret == NULL) {
		math_error("cannot allocate RANDOM state");
		/*NOTREACHED*/
	}
	/* move the ZVALUES over to ret */
	*ret = blum;

	/*
	 * load the new state
	 */
	if (state != NULL) {
		blum.seeded = 0;	/* avoid being caught while copying */
		blum.bits = state->bits;
		blum.buffer = state->buffer;
		zcopy(*(state->n), blum.n);
		zcopy(*(state->r), blum.r);
		blum.seeded = 1;
	}

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
 *	src     - low order FULL in a 64 bit slot
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
		/* fill  up the 2nd most significant HALF */
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
		/* fill  up the 2nd most significant HALF */
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
 * is 32 bits and the array is 2 FULLs.  The most significant bit
 * in the array (highest bit in the last FULL of the array) is to
 * be transfered to the most significant bit in the destination.
 *
 * given:
 *	bitstr	- most significant destination bit in a bit string
 *	src     - low order FULL in a 64 bit slot
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
 * zrandskip - skip s bits
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
		return;	/* skip need satisfied */
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
 * given:
 *	count - number of bits required
 *	res - where to place the random bits as ZVALUE
 */
void
zrand(long cnt, ZVALUE *res)
{
	long hlen;		/* length of ZVALUE in HALFs */
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
	hlen = (cnt+BASEB-1)/BASEB;
	res->len = (LEN)hlen;
	res->v = alloc((LEN)hlen);
	memset(res->v, 0, hlen*sizeof(HALF));

	/*
	 * dest bit string
	 */
	dest.len = (int)cnt;
	dest.loc = res->v + (hlen-1);
	dest.bit = (int)((cnt-1) % BASEB);

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
 * zrandrange - generate a random value in the range [low, high)
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
	 * a value mod 'range'.  Consider the case where 'range' is '80'
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
 * irand - generate a random long in the range [0, s)
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
 * randomcopy - make a copy of a Blum state
 *
 * given:
 *	state - the state to copy
 *
 * returns:
 *	a malloced copy of the state
 */
RANDOM *
randomcopy(CONST RANDOM *state)
{
	RANDOM *ret;	/* return copy of state */

	/*
	 * malloc state
	 */
	ret = (RANDOM *)malloc(sizeof(RANDOM));
	if (ret == NULL) {
		math_error("can't allocate RANDOM state");
		/*NOTREACHED*/
	}

	/*
	 * clone data
	 */
	*ret = *state;
	if (state->r->v == NULL) {
		ret->r->v = NULL;
	} else {
		zcopy(*(state->r), ret->r);
	}
	if (state->n->v == NULL) {
		ret->n->v = NULL;
	} else {
		zcopy(*(state->n), ret->n);
	}

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
 * randomfree - free a Blum state
 *
 * given:
 *	state - the state to free
 */
void
randomfree(RANDOM *state)
{
	/* free the values */
	state->seeded = 0;
	zfree(*state->n);
	zfree(*state->r);

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
			return TRUE;
		} else {
			/* uninitialized only equals default state */
			return randcmp(s2, &init_a55);
		}
	} else if (!s2->seeded) {
		if (!s1->seeded) {
			/* uninitialized == uninitialized */
			return TRUE;
		} else {
			/* uninitialized only equals default state */
			return randcmp(s1, &init_a55);
		}
	}

	/* compare states */
	return (BOOL)(memcmp(s1, s2, sizeof(RAND)) != 0);
}


/*
 * randomcmp - compare two Blum states
 *
 * given:
 *	s1 - first state to compare
 *	s2 - second state to compare
 *
 * return:
 *	TRUE if states differ
 */
BOOL
randomcmp(CONST RANDOM *s1, CONST RANDOM *s2)
{
	/*
	 * assume uninitialized state == the default seeded state
	 */
	if (!s1->seeded) {
		if (!s2->seeded) {
			/* uninitialized == uninitialized */
			return TRUE;
		} else {
			/* uninitialized only equals default state */
			return randomcmp(s2, &post_init_blum);
		}
	} else if (!s2->seeded) {
		/* uninitialized only equals default state */
		return randomcmp(s1, &post_init_blum);
	}

	/*
	 * compare operating mask parameters
	 */
	if ((s1->loglogn != s2->loglogn) || (s1->mask != s2->mask)) {
		return FALSE;
	}

	/*
	 * compare bit buffer
	 */
	if ((s1->bits != s2->bits) || (s1->buffer != s2->buffer)) {
		return FALSE;
	}

	/*
	 * compare quadratic residues
	 */
	if (!zcmp(*(s1->r), *(s2->r))) {
		return FALSE;
	}

	/*
	 * compare moduli
	 */
	if (!zcmp(*(s1->n), *(s2->n))) {
		return FALSE;
	}

	/*
	 * they are equal
	 */
	return TRUE;
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


/*
 * randomprint - print a Blum state
 *
 * given:
 *	state - state to print
 *	flags - print flags passed from printvalue() in value.c
 */
/*ARGSUSED*/
void
randomprint(CONST RANDOM *state, int flags)
{
	math_str("RANDOM state");
}
