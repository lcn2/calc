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
 * AN OVERVIEW OF THE FUNCTIONS:
 *
 * This module contains a Blum-Blum-Shub generator:
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
 *	Of course the Blum modulus should have a long period.  The default
 *	Blum modulus as well as the compiled in Blum moduli have very long
 *	periods.  When using your own Blum modulus, a little care is needed
 *	to avoid generators with very short periods.  (see below)
 *
 *	To compromise the generator, an adversary must either factor the
 *	modulus or perform an exhaustive search just to determine the next
 *	(or previous) bit.  If we make the modulus hard to factor
 *	(such as the product of two large well chosen primes) breaking
 *	the sequence could be intractable for todays computers and methods.
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
 * ON THE GENERATOR:
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
 *
 *	Print a Blum generator random value over interval [min,max).
 *
 *   random()
 *
 *	Same as random(0, 2^64).  Print 64 bits.
 *
 *   random(lim)		(where 0 > lim)
 *
 *	Same as random(0, lim).
 *
 *   randombit(x)		(where x > 0)
 *
 *	Same as random(0, 2^x).  Print x bits.
 *
 *   randombit(skip)		(where skip < 0)
 *
 *	Skip skip random bits and return the bit skip count (-skip).
 */

/*
 * INITIALIZATION AND SEEDS:
 *
 * All generators come already seeded with precomputed initial constants.
 * Thus, it is not required to seed a generator before using it.
 *
 * The Blum generator may be initialized and seeded via srandom().
 *
 * Using a seed of '0' will reload generators with their initial states.
 *
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
 *	The default Blum modulus is 260 bits long, so when using a the
 *	single arg call, a seed of between 128 and 256 bits is reasonable.
 *
 ******************************************************************************
 *
 * srandom(seed)
 *
 *    We attempt to set the quadratic residue and possibly the Blum modulus.
 *
 *    Any internally buffered random bits are flushed.
 *
 *    The Blum modulus is only set if seed == 0.
 *
 *    The initial quadratic residue is set according to the seed
 *    arg as defined below.
 *
 *    seed >= 2^32:
 *    -------------
 *	Use seed to compute a new quadratic residue for use with
 *	the current Blum modulus.  We will successively square mod Blum
 *	modulus until we get a smaller value (modulus wrap).
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	  n = default_modulus;		(* n is the new Blum modulus *)
 *	  r = seed;
 *	  do {
 *	      last_r = r;
 *	      r = pmod(last_r, 2, n);
 *	  } while (r > last_r);		(* r is the new quadratic residue *)
 *
 *	NOTE: The Blum modulus is not set by this call.
 *
 *    0 < seed < 2^32:
 *    ----------------
 *	Reserved for future use.
 *
 *    seed == 0:
 *    ----------
 *	Restore the initial state and modulus of the Blum generator.
 *	After this call, the Blum  generator is restored to its initial
 *	state after calc started.
 *
 *	The Blum prime factors of the modulus have been disclosed (see
 *	"SOURCE OF MAGIC NUMBERS" below).  If you want to use moduli that
 *	have not been disclosed, use srandom(seed, newn) with the
 *	appropriate args as noted below.
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	  n = default_modulus;	(* as used by the initial state *)
 *	  r = default_residue;	(* as used by the initial state *)
 *
 *	NOTE: The Blum modulus is changed by this call.
 *
 *    seed < 0:
 *    ---------
 *	Reserved for future use.
 *
 ******************************************************************************
 *
 * srandom(seed, newn)
 *
 *    We attempt to set the Blum modulus and quadratic residue.
 *    Any internally buffered random bits are flushed.
 *
 *    If newn == 1 mod 4, then we will assume that it is the
 *    product of two Blum primes (primes == 3 mod 4) and use it
 *    as the Blum modulus.
 *
 *    The new quadratic residue is set according to the seed
 *    arg as defined below.
 *
 *    seed >= 2^32, newn >= 2^32:
 *    ---------------------------
 *	Assuming that 'newn' == 3 mod 4, then we will use it as
 *	the Blum modulus.
 *
 *	We will use the seed arg to compute a new quadratic residue.
 *	We will successively square it mod Blum modulus until we get
 *	a smaller value (modulus wrap).
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	  if (newn % 4 == 1) {
 *	      n = newn;			(* n is the new Blum modulus *)
 *	      r = seed;
 *	      do {
 *		  last_r = r;
 *		  r = pmod(last_r, 2, n);
 *	      } while (r > last_r);	(* r is the new quadratic residue *)
 *	  } else {
 *	      quit "newn (2nd arg) must be 3 mod 4";
 *	  }
 *
 *    0 < seed < 2^32, newn >= 2^32:
 *    ------------------------------
 *	Reserved for future use.
 *
 *    seed == 0, newn >= 2^32:
 *    ------------------------
 *	Assuming that 'newn' == 3 mod 4, then we will use it as
 *	the Blum modulus.
 *
 *	The initial quadratic residue will be as if the default initial
 *	quadratic residue arg was given.
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	  srandom(default_residue, newn)
 *
 *	or in other words:
 *
 *	  if (newn % 4 == 1) {
 *	      n = newn;			(* n is the new Blum modulus *)
 *	      r = default_residue;	(* as used by the initial state *)
 *	      do {
 *		  last_r = r;
 *		  r = pmod(last_r, 2, n);
 *	      } while (r > last_r);	(* r is the new quadratic residue *)
 *	  } else {
 *	      quit "newn (2nd arg) must be 3 mod 4";
 *	  }
 *
 *    seed < 0, newn >= 2^32:
 *    -----------------------
 *	Reserved for future use.
 *
 *    any seed, 20 < newn < 1007:
 *    ---------------------------
 *	Reserved for future use.
 *
 *    seed >= 2^32, 0 < newn <= 20:
 *    -----------------------------
 *	Set the Blum modulus to one of the the pre-defined Blum moduli.
 *	See below for the values of these pre-defined Blum moduli and how
 *	they were computed.
 *
 *	We will use the seed arg to compute a new quadratic residue.
 *	We will successively square it mod Blum modulus until we get
 *	a smaller value (modulus wrap).
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	  n = n[newn];	  	(* n is new Blum modulus, see below *)
 *	  r = seed;
 *	  do {
 *	      last_r = r;
 *	      r = pmod(last_r, 2, n);
 *	  } while (r > last_r);	(* r is the new quadratic residue *)
 *
 *    0 < seed < 2^32, 0 < newn <= 20:
 *    --------------------------------
 *	Reserved for future use.
 *
 *    seed == 0, 0 < newn <= 20:
 *    --------------------------
 *	Set the Blum modulus to one of the the pre-defined Blum moduli.
 *	The new quadratic residue will also be set to one of
 *	the pre-defined quadratic residues.
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	  srandom(r[newn], n[newn])
 *
 *	or in other words:
 *
 *	  n = n[newn];		(* n is the new Blum modulus, see below *)
 *	  r = r[newn];		(* r is the new quadratic residue *)
 *
 *	The pre-defined Blum moduli was computed by searching for Blum
 *	primes (primes == 3 mod 4) starting from new values that
 *	were selected by lavarand, a hardware random number generator.
 *	See the URL:
 *
 *	    http://lavarand.sgi.com
 *	    XXX - This URL is not available on 17Feb97 ... but will be soon.
 *
 *	for an explination of how the lavarand random number generator works.
 *
 *	For a given newn, we select a given bit length.  For 0 < newn <= 20,
 *	the bit length selected was by:
 *
 *	    bitlen = 2^(int((newn-1)/4)+7) + small_random_value;
 *
 *	where small_random_value is also generated by lavarand.  For
 *	1 <= newn <= 16, small_random_value is a random value in [0,40).
 *	For 17 < newn <= 20, small_random_value is a random value in [0,120).
 *	Given two random integers generated by lavarand, we used the following
 *	to compute Blum primes:
 *
 *	  (* find the first Blum prime *)
 *	  fp = int((ip-1)/2);		(* ip was generated by lavarand *)
 *	  do {
 *	      fp = nextcand(fp+2, 25, 0, 3, 4);
 *	      p = 2*fp+1;
 *	  } while (ptest(p, 25) == 0);
 *
 *	  (* find the 2nd Blum prime *)
 *	  fq = int((iq-1)/2);		(* iq was generated by lavarand *)
 *	  do {
 *	      fq = nextcand(fq+2, 25, 0, 3, 4);
 *	      q = 2*fq+1;
 *	  } while (ptest(q, 25) == 0);
 *
 *	  (* compute the Blum modulus *)
 *	  n[newn] = p * q;
 *
 *	The pre-defined quadratic residues was also generated by lavarand.
 *	The value produced by lavarand was squared mod the Blum moduli
 *	that was previously computed.
 *
 *	The purpose of these pre-defined Blum moduli is to provide users with
 *	an easy way to use a generator where the individual Blum primes used
 *	are not well known.  True, these values are in some way "MAGIC", on
 *	the other hand that is their purpose!  If this bothers you, don't
 *	use them.  See the section "FOR THE PARANOID" below for details.
 *
 *	The value 'newn' determines which pre-defined generator is used.
 *	For a given 'newn' the Blum modulus 'n[newn]' (product of 2 Blum
 *	(primes) and new quadratic residue 'r[newn]' is set as follows:
 *
 *	newn ==  1: (Blum modulus bit length 130)
 *	n[ 1] = 0x5049440736fe328caf0db722d83de9361
 *	r[ 1] = 0xb226980f11d952e74e5dbb01a4cc42ec
 *
 *	newn ==  2: (Blum modulus bit length 137)
 *	n[ 2] = 0x2c5348a2555dd374a18eb286ea9353443f1
 *	r[ 2] = 0x40f3d643446cd710e3e893616b21e3a218
 *
 *	newn ==  3: (Blum modulus bit length 147)
 *	n[ 3] = 0x9cfd959d6ce4e3a81f1e0f2ca661f11d001f1
 *	r[ 3] = 0xfae5b44d9b64ff5cea4f3e142de2a0d7d76a
 *
 *	newn ==  4: (Blum modulus bit length 157)
 *	n[ 4] = 0x3070f9245c894ed75df12a1a2decc680dfcc0751
 *	r[ 4] = 0x20c2d8131b2bdca2c0af8aa220ddba4b984570
 *
 *	newn ==  5: (Blum modulus bit length 257)
 *	n[ 5] = 0x2109b1822db81a85b38f75aac680bc2fa5d3fe1118769a0108b99e5e799
 *		166ef1
 *	r[ 5] = 0x5e9b890eae33b792e821a9605f5df6db234f7b7d1e70aeed0e6c77c859e
 *		2efa9
 *
 *	newn ==  6: (Blum modulus bit length 259)
 *	n[ 6] = 0xa7bfd9d7d9ada2c79f2dbf2185c6440263a38db775ee732dad85557f1e1
 *		ddf431
 *	r[ 6] = 0x5e94a02f88667154e097aedece1c925ce1f3495d2c98eccfc5dc2e80c94
 *		04daf
 *
 *	newn ==  7: (Blum modulus bit length 286)
 *	n[ 7] = 0x43d87de8f2399ef237801cd5628643fcff569d6b0dcf53ce52882e7f602
 *	        f9125cf9ec751
 *	r[ 7] = 0x13522d1ee014c7bfbe90767acced049d876aefcf18d4dd64f0b58c3992d
 *	        2e5098d25e6
 *
 *	newn ==  8: (Blum modulus bit length 294)
 *	n[ 8] = 0x5847126ca7eb4699b7f13c9ce7bdc91fed5bdbd2f99ad4a6c2b59cd9f0b
 *	        c42e66a26742f11
 *	r[ 8] = 0x853016dca3269116b7e661fa3d344f9a28e9c9475597b4b8a35da929aae
 *	        95f3a489dc674
 *
 *	newn ==  9: (Blum modulus bit length 533)
 *	n[ 9] = 0x39e8be52322fd3218d923814e81b003d267bb0562157a3c1797b4f4a867
 *	        52a84d895c3e08eb61c36a6ff096061c6fd0fdece0d62b16b66b980f95112
 *		745db4ab27e3d1
 *	r[ 9] = 0xb458f8ad1e6bbab915bfc01508864b787343bc42a8aa82d9d2880107e3f
 *	        d8357c0bd02de3222796b2545e5ab7d81309a89baedaa5d9e8e59f959601e
 *		f2b87d4ed20d
 *
 *	newn == 10: (Blum modulus bit length 537)
 *	n[10] = 0x25f2435c9055666c23ef596882d7f98bd1448bf23b50e88250d3cc952c8
 *	        1b3ba524a02fd38582de74511c4008d4957302abe36c6092ce222ef9c73cc
 *		3cdc363b7e64b89
 *	r[10] = 0x66bb7e47b20e0c18401468787e2b707ca81ec9250df8cfc24b5ffbaaf2c
 *	        f3008ed8b408d075d56f62c669fadc4f1751baf950d145f40ce23442aee59
 *		4f5ad494cfc482
 *
 *	newn == 11: (Blum modulus bit length 542)
 *	n[11] = 0x497864de82bdb3094217d56b874ecd7769a791ea5ec5446757f3f9b6286
 *	        e58704499daa2dd37a74925873cfa68f27533920ee1a9a729cf522014dab2
 *		2e1a530c546ee069
 *	r[11] = 0x8684881cb5e630264a4465ae3af8b69ce3163f806549a7732339eea2c54
 *	        d5c590f47fbcedfa07c1ef5628134d918fee5333fed9c094d65461d88b13a
 *		0aded356e38b04
 *
 *	newn == 12: (Blum modulus bit length 549)
 *	n[12] = 0x3457582ab3c0ccb15f08b8911665b18ca92bb7c2a12b4a1a66ee4251da1
 *	        90b15934c94e315a1bf41e048c7c7ce812fdd25d653416557d3f09887efad
 *		2b7f66d151f14c7b99
 *	r[12] = 0xdf719bd1f648ed935870babd55490137758ca3b20add520da4c5e8cdcbf
 *	        c4333a13f72a10b604eb7eeb07c573dd2c0208e736fe56ed081aa9488fbc4
 *		5227dd68e207b4a0
 *
 *	newn == 13: (Blum modulus bit length 1048)
 *	n[13] = 0x1517c19166b7dd21b5af734ed03d833daf66d82959a553563f4345bd439
 *	        510a7bda8ee0cb6bf6a94286bfd66e49e25678c1ee99ceec891da8b18e843
 *		7575113aaf83c638c07137fdd3a76c3a49322a11b5a1a84c32d99cbb2b056
 *		671589917ed14cc7f1b5915f6495dd1892b4ed7417d79a63cc8aaa503a208
 *		e3420cca200323314fc49
 *	r[13] = 0xd42e8e9a560d1263fa648b04f6a69b706d2bc4918c3317ddd162cb4be7a
 *	        5e3bbdd1564a4aadae9fd9f00548f730d5a68dc146f05216fe509f0b8f404
 *		902692de080bbeda0a11f445ff063935ce78a67445eae5c9cea5a8f6b9883
 *		faeda1bbe5f1ad3ef6409600e2f67b92ed007aba432b567cc26cf3e965e20
 *		722407bfe46b7736f5
 *
 *	newn == 14: (Blum modulus bit length 1054)
 *	n[14] = 0x5e56a00e93c6f4e87479ac07b9d983d01f564618b314b4bfec7931eee85
 *	        eb909179161e23e78d32110560b22956b22f3bc7e4a034b0586e463fd40c6
 *		f01a33e30ede912acb86a0c1e03483c45f289a271d14bd52792d0a076fdfe
 *		fe32159054b217092237f0767434b3db112fee83005b33f925bacb3185cc4
 *		409a1abdef8c0fc116af01
 *	r[14] = 0xf7aa7cb67335096ef0c5d09b18f15415b9a564b609913f75f627fc6b0c5
 *	        b686c86563fe86134c5a0ea19d243350dfc6b9936ba1512abafb81a0a6856
 *		c9ae7816bf2073c0fb58d8138352b261a704b3ce64d69dee6339010186b98
 *		3677c84167d4973444194649ad6d71f8fa8f1f1c313edfbbbb6b1b220913c
 *		c8ea47a4db680ff9f190
 *
 *	newn == 15: (Blum modulus bit length 1055)
 *	n[15] = 0x97dd840b9edfbcdb02c46c175ba81ca845352ebe470be6075326a26770c
 *	        ab84bfc0f2e82aa95aac14f40de42a0590445b902c2b8ebb916753e72ab86
 *		c3278cccc1a783b3e962d81b80df03e4380a8fa08b0d86ed0caa515c196a5
 *		30e49c558ddb53082310b1d0c7aee6f92b619798624ffe6c337299bc51ff5
 *		d2c721061e7597c8d97079
 *	r[15] = 0xb8220703b8c75869ab99f9b50025daa8d77ca6df8cef423ede521f55b1c
 *	        25d74fbf6d6cc31f5ef45e3b29660ef43797f226860a4aa1023dbe522b1fe
 *		6224d01eb77dee9ad97e8970e4a9e28e7391a6a70557fa0e46eca78866241
 *		ba3c126fc0c5469f8a2f65c33db95d1749d3f0381f401b9201e6abd43d98d
 *		b92e808f0aaa6c3e2110
 *
 *	newn == 16: (Blum modulus bit length 1062)
 *	n[16] = 0x456e348549b82fbb12b56f84c39f544cb89e43536ae8b2b497d426512c7
 *	        f3c9cc2311e0503928284391959e379587bc173e6bc51ba51c856ba557fee
 *		8dd69cee4bd40845bd34691046534d967e40fe15b6d7cf61e30e283c05be9
 *		93c44b6a2ea8ade0f5578bd3f618336d9731fed1f1c5996a5828d4ca857ac
 *		2dc9bd36184183f6d84346e1
 *	r[16] = 0xb0d7dcb19fb27a07973e921a4a4b6dcd7895ae8fced828de8a81a3dbf25
 *	        24def719225404bfd4977a1508c4bac0f3bc356e9d83b9404b5bf86f6d19f
 *		f75645dffc9c5cc153a41772670a5e1ae87a9521416e117a0c0d415fb15d2
 *		454809bad45d6972f1ab367137e55ad0560d29ada9a2bcda8f4a70fbe04a1
 *		abe4a570605db87b4e8830
 *
 *	newn == 17: (Blum modulus bit length 2062)
 *	n[17] = 0x6177813aeac0ffa3040b33be3c0f96e0faf97ca54266bfedd7be68494f7
 *		6a7a91144598bf28b3a5a9dc35a6c9f58d0e5fb19839814bc9d456bff7f29
 *		953bdac7cafd66e2fc30531b8d544d2720b97025e22b1c71fa0b2eb9a499d
 *		49484615d07af7a3c23b568531e9b8507543362027ec5ebe0209b4647b7ff
 *		54be530e9ef50aa819c8ff11f6d7d0a00b25e88f2e6e9de4a7747022b949a
 *		b2c2e1ab0876e2f1177105718c60196f6c3ac0bde26e6cd4e5b8a20e9f0f6
 *		0974f0b3868ff772ab2ceaf77f328d7244c9ad30e11a2700a120a314aff74
 *		c7f14396e2a39cc14a9fa6922ca0fce40304166b249b574ffd9cbb927f766
 *		c9b150e970a8d1edc24ebf72b72051
 * 	r[17] = 0x53720b6eaf3bc3b8adf1dd665324c2d2fc5b2a62f32920c4e167537284d
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
 *	n[18] = 0x68f2a38fb61b42af07cb724fec0c7c65378efcbafb3514e268d7ee38e21
 *		a5680de03f4e63e1e52bde1218f689900be4e5407950539b9d28e9730e8e6
 *		ad6438008aa956b259cd965f3a9d02e1711e6b344b033de6425625b6346d2
 *		ca62e41605e8eae0a7e2f45c25119ef9eece4d3b18369e753419d94118d51
 *		803842f4de5956b8349e6a0a330145aa4cd1a72afd4ef9db5d8233068e691
 *		18ff4b93bcc67859f211886bb660033f8170640c6e3d61471c3b7dd62c595
 *		b156d77f317dc272d6b7e7f4fdc20ed82f172fe29776f3bddf697fb673c70
 *		defd6476198a408642ed62081447886a625812ac6576310f23036a7cd3c93
 *		1c96f7df128ad4ed841351b18c8b78629
 *	r[18] = 0x4735e921f1ac6c3f0d5cda84cd835d75358be8966b99ff5e5d36bdb4be1
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
 *	n[19] = 0x230d7ab23bb9e8d6788b252ad6534bdde276540721c3152e410ad4244de
 *		b0df28f4a6de063ba1e51d7cd1736c3d8410e2516b4eb903b8d9206b92026
 *		64cacbd0425c516833770d118bd5011f3de57e8f607684088255bf7da7530
 *		56bf373715ed9a7ab85f698b965593fe2b674225fa0a02ebd87402ffb3d97
 *		172acadaa841664c361f7c11b2af47a472512ee815c970af831f95b737c34
 *		2508e4c23f3148f3cdf622744c1dcfb69a43fd535e55eebcdc992ee62f2b5
 *		2c94ac02e0921884fe275b3a528bdb14167b7dec3f3f390cd5a82d80c6c30
 *		6624cc7a7814fb567cd4d687eede573358f43adfcf1e32f4ee7a2dc4af029
 *		6435ade8099bf0001d4ae0c7d204df490239c12d6b659a79
 *	r[19] = 0x8f1725f21e245e4fc17982196605b999518b4e21f65126fa6fa759332c8
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
 *	n[20] = 0x4fd2b820e0d8b13322e890dddc63a0267e5b3a648b03276066a3f356d79
 *		660c67704c1be6803b8e7590ee8a962c8331a05778d010e9ba10804d661f3
 *		354be1932f90babb741bd4302a07a92c42253fd4921864729fb0f0b1e0a42
 *		d66b6777893195abd2ee2141925624bf71ad7328360135c565064ee502773
 *		6f42a78b988f47407ba4f7996892ffdc5cf9e7ab78ac95734dbf4e3a3def1
 *		615b5b4341cfbf6c3d0a61b75f4974080bbac03ee9de55221302b40da0c50
 *		ded31d28a2f04921a532b3a486ae36e0bb5273e811d119adf90299a74e623
 *		3ccce7069676db00a3e8ce255a82fd9748b26546b98c8f4430a8db2a4b230
 *		fa365c51e0985801abba4bbcf3727f7c8765cc914d262fcec3c1d081
 *	r[20] = 0x46ef0184445feaa3099293ee960da14b0f8b046fa9f608241bc08ddeef1
 *		7ee49194fd9bb2c302840e8da88c4e88df810ce387cc544209ec67656bd1d
 *		a1e9920c7b1aad69448bb58455c9ae4e9cd926911b30d6b5843ff3d306d56
 *		54a41dc20e2de4eb174ec5ac3e6e70849de5d5f9166961207e2d8b31014cf
 *		35f801de8372881ae1ba79e58942e5bef0a7e40f46387bf775c54b1d15a14
 *		40e84beb39cd9e931f5638234ea730ed81d6fca1d7cea9e8ffb171f6ca228
 *		56264a36a2a783fd7ac39361a6598ed3a565d58acf1f5759bd294e5f53131
 *		bc8e4ee3750794df727b29b1f5788ae14e6a1d1a5b26c2947ed46f49e8377
 *		3292d7dd5650580faebf85fd126ac98d98f47cf895abdc7ba048bd1a
 *
 *	NOTE: The Blum moduli associated with 1 <= newn < 9 are subject
 *	      to having their Blum moduli factored, depending in their size,
 *	      by small PCs in a reasonable to large supercomputers/highly
 *	      parallel processors over a long time.  Their value lies in their
 *	      speed relative the the default Blum generator.  As of Jan 1997,
 *	      the Blum moduli associated with 13 <= newn < 20 appear to
 *	      be well beyond the scope of hardware and algorithms,
 *	      and 9 <= newn < 12 might be factorable with extreme difficulty.
 *
 *	The following table may be useful as a guide for how easy it
 *	is to factor the modulus:
 *
 *	1 <= newn <= 4	PC using ECM in a short amount of time
 *	5 <= newn <= 8	Workstation using MPQS in a short amount of time
 *	8 <= newn <= 12	High end supercomputer or high parallel processor
 *			using state of the art factoring over a long time
 *     12 <= newn <= 16	Beyond Feb 1997 systems and factoring methods
 *     17 <= newn <= 20	Well beyond Feb 1997 systems and factoring methods
 *
 *	      See the section titled 'FOR THE PARANOID' for more details.
 *
 *    seed < 0, 0 < newn <= 20:
 *    -------------------------
 *	Reserved for future use.
 *
 ******************************************************************************
 *
 * srandom(seed, ip, iq, trials)
 *
 *    We attempt to set the Blum modulus and quadratic residue.
 *    Any internally buffered random bits are flushed.
 *
 *    Use the ip and iq args as starting points for Blum primes.
 *    The trials arg determines how many ptest cycles are performed
 *    on each candidate.
 *
 *    The new quadratic residue is set according to the seed
 *    arg as defined below.
 *
 *    seed >= 2^32, ip >= 2^16, iq >= 2^16:
 *    -------------------------------------
 *	Set the Blum modulus by searching from the ip and iq search points.
 *
 *	We will use the seed arg to compute a new quadratic residue.
 *	We will successively square it mod Blum modulus until we get
 *	a smaller value (modulus wrap).
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	  p = nextcand(ip-2, trials, 0, 3, 4);    (* find the 1st Blum prime *)
 *	  q = nextcand(iq-2, trials, 0, 3, 4);    (* find the 2nd Blum prime *)
 *	  n = p * q; 	  		(* n is the new Blum modulus *)
 *	  r = seed;
 *	  do {
 *	      last_r = r;
 *	      r = pmod(last_r, 2, n);
 *	  } while (r > last_r);		(* r is the new quadratic residue *)
 *	  srandom(r, n);
 *
 *    any seed, ip <= 2^16 or iq <= 2^16:
 *    -----------------------------------
 *	Reserved for future use.
 *
 *    0 < seed < 2^32, any ip, any iq:
 *    --------------------------------
 *	Reserved for future use.
 *
 *    seed == 0, ip > 2^16, iq > 2^16:
 *    --------------------------------
 *	Set the Blum modulus by searching from the ip and iq search points.
 *	If trials is omitted, 1 is assumed.
 *
 *	The initial quadratic residue will be as if the default initial
 *	quadratic residue arg was given.
 *
 *	The follow calc script produces an equivalent effect:
 *
 *	  srandom(default_residue, ip, iq, trials)
 *
 *	or in other words:
 *
 *	  (* trials, if omitted, is assumed to be 1 *)
 *	  p = nextcand(ip-2, trials, 0, 3, 4);    (* find the 1st Blum prime *)
 *	  q = nextcand(iq-2, trials, 0, 3, 4);    (* find the 2nd Blum prime *)
 *	  n = p * q; 	  		(* n is the new Blum modulus *)
 *	  r = default_residue;		(* as used by the initial state *)
 *	  do {
 *	      last_r = r;
 *	      r = pmod(last_r, 2, n);
 *	  } while (r > last_r);		(* r is the new quadratic residue *)
 *
 *    seed < 0, any ip, any iq:
 *    -------------------------
 *	Reserved for future use.
 *
 ******************************************************************************
 *
 * srandom()
 *
 *    Return current Blum generator state.  This call does not alter
 *    the generator state.
 *
 ******************************************************************************
 *
 * srandom(state)
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
 * When seeding the Blum generator, we disallow seeds < 2^32 in an
 * effort to avoid trivial seed values such as 0, 1 or other small values.
 * The 2^32 lower bound limit was also selected because it provides a
 * large reserved value space for special seeds.  Currently the
 * [1,2^32) seed range is reserved for future use.
 *
 ***
 *
 * When using the 2 arg srandom(seed, newn), we require newn > 2^32
 * to avoid trivial Blum moduli.  The 2^32 lower bound limit was also
 * selected because it provides a large reserved value space for special
 * moduli.  Currently the [21,2^32) newn range is reserved for future use.
 *
 * When using the 3 or 4 arg srandom(seed, ip, iq [, trials]) form,
 * we require ip>2^16 and ip>2^16.  This ensures that the resulting
 * Blum modulus is > 2^32.
 *
 ***
 *
 * Taking some care to select a good initial residue helps eliminate cheap
 * search attacks.  It is true that a subsequent residue could be one of the
 * residues that we would first avoid.  However such an occurrence will
 * happen after the generator is well underway and any such seed information
 * has been lost.
 *
 * In an effort to avoid trivial seed values, we force the seed arg
 * to srandom() to be > 2^32.  We then square this value mod the
 * Blum modulus until it is less than the previous value.  This ensures
 * that the previous seed value is large enough that its square is > Blum
 * modulus, and this the square mod Blum modulus is non-trivial.
 *
 ***
 *
 * The size of default Blum modulus 'n=p*q' was taken to be > 2^259, or
 * 260 bits (79 digits) long.  A modulus > 2^256 will generate 8 bits
 * per crank of the generator.  The period of this generator is long
 * enough to be reasonable, and the modulus is small enough to be fast.
 *
 * The default Blum modulus is not a secure modulus because it can
 * be factored with ease.  As if Feb 1997,  the upper reach of the
 * state of the art for factoring general numbers was around 2^512.
 * Clearly factoring a 260 bit number if well within the reach of even
 * a low life Pentium.
 *
 * The fact that the default modulus can be factored with ease is
 * not a drawback.  Afterall, if we are going to keep to the goal
 * of disclosing the source magic numbers, we need to disclose how
 * the Blum Modulus was produced ... including its factors.  Knowing
 * the facotrs of the Blum modulus does not reduce its quality,
 * only the ability for someone to determine where you are in the
 * sequence.  But heck, the default seed is well known anyway so
 * there is no real loss if the factors are also known.
 *
 ***
 *
 * The default Blum modulus is the product of two Blum probable primes
 * that were selected by the Rand Book of Random Numbers.  Using the '6% rule',
 * a default Blum modulus n=p*q > 2^256 would be satisfied if p were
 * 38 decimal digits and q were 42 decimal digits in length.  We restate
 * the sizes in decimal digits because the Rand Book of Random Numbers is a
 * book of decimal digits.  Using the first 38 rand digits as a
 * starting search point for 'p', and the next 42 digits for a starting
 * search point for 'q'.
 *
 *	(*
 *	 * setup the search points (lines split for readability)
 *	 *)
 *	ip = 10097325337652013586346735487680959091;
 *	iq = 173929274945375420480564894742962480524037;
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
 *	    p= 0x798ac934c7a3318ad446190f3474e57
 *
 *	    q= 0x1ff21d7e1dd7d5965e224d485d84c3ef44f
 *
 * These Blum primes were found after 1.81s of CPU time on a 195 Mhz IP28
 * R10000 version 2.5 processor.  The first Blum prime 'p' was 31716 higher
 * than the initial search value 'ip'.  The second Blum prime 'q' was 18762
 * higher than the initial starting 'iq'.
 *
 * The product of the two Blum primes results in a 260 bit Blum modulus of:
 *
 *	n = 0xf2ac1903156af9e373d78613ed0e8d30284f34b644a9027d9ba55a689d6be18d9
 *
 * The selection if the initial quadratic residue comes from the next
 * unused digits of the Rand Book of Random Numbers.  Now the two initial
 * search values 'ip' and 'iq' used above needed the first 38 digits and
 * the next 42 digits.  Thus we will skip the first 38+42=80 digits
 * and begin to build in initial search value for a quadratic residue (most
 * significant digit first) from the Rand Book of Numbers digits until we
 * have a value whose square mod n > 4th power mod n.  In other words, we
 * need to build ir up from new Rand Book of Random Numbers digits until
 * we find a value in which srandom(ir), for the Blum Modulus 'n' produces
 * an initial quadratic residue on the first loop.
 *
 * Clearly we need to find an ir that is > sqrt(n).  The first ir:
 *
 *	ir = 2063610402008229166508422689531964509303
 *
 * fails the single loop criteria.  So we add the next digit:
 *
 *	ir = 20636104020082291665084226895319645093032
 *
 * Here we find that:
 *
 *	pmod(ir,2,n) > pmod(pmod(ir,2,n),2,n)
 *
 * Thus, for thw Blum modulus 'n', the method outlined for srandom(ir) yields
 * the initial quadratic residue of:
 *
 *    r = 0x748b6d882ff4b074e2f1e93a8627d626506c73ca5a62546c90f23fd7ed3e7b11e
 *
 ***
 *
 * In the above process of finding the Blum primes used in the default
 * Blum modulus, we selected primes of the form:
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
 * The process above resulted in a default generator Blum modulus n > 2^259
 * with period of at least 2^258 bits.  To be exact, the period of the
 * default Blum generator is:
 *
 *	    0x79560c818ab57cf1b9ebc309f68746881adc15e79c05e476f741e5f904b9beb1a
 *
 * which is approximately:
 *
 *	   ~8.781 * 10^77
 *
 * This period is more than long enough for computationally tractable tasks.
 *
 ***
 *
 * The 20 builtin generators, on the other hand, were selected
 * with more care.  While the lower order 20 generators have
 * factorable generators, the higher order are likely to be
 * be beyond the reach for a while.
 *
 * The lengths of the two Blum probable primes 'p' and 'q' used to make up
 * the 20 Blum modului 'n=p*q' differ slightly to avoid certain
 * factorization attacks that work on numbers that are a perfect square,
 * or where the two primes are nearly the same.  I elected to have the
 * sizes differ by up to 6% of the product size to avoid such attacks.
 * Clearly one does not want the size of the two factors to differ
 * by a large percentage: p=3 and q large would result in a easy
 * to factor Blum modulus.  Thus we select sizes that differ by
 * up to 6% but not (significantly) greater than 6%.
 *
 * Using the '6% rule' above, a Blum modulus n=p*q > 2^1024 would have two
 * Blum factors p > 2^482 and q > 2^542.  This is because 482+542 = 1024.
 * The difference 542-482 is ~5.86% of 1024, and is the largest difference
 * that is < 6%.
 *
 ******************************************************************************
 *
 * FOR THE PARANOID:
 *
 * The truly paranoid might suggest that my claims in the MAGIC NUMBERS
 * section are a lie intended to entrap people.  Well they are not, but
 * you need not take my word for it.
 *
 ***
 *
 * One could take issue with the above script that produced a 260 bit
 * Blum modulus.  So if that bothers you, then seed your generator
 * with your own Blum modulus and initial quadratic residue.  And
 * if you are truly paranoid, why would you want to use the default seed,
 * which is well known?
 *
 ***
 *
 * If all the above fails to pacify the truly paranoid, then one may
 * select your own modulus and initial quadratic residue by calling:
 *
 *	srandom(r, n);
 *
 * Of course, you will need to select a correct Blum modulus 'n' as the
 * product of two Blum primes (both 3 mod 4) and with a long period (where
 * lcm(factors of one less than the two Blum primes) is large) and an
 * initial quadratic residue 'r' that is hard to guess selected at random.
 *
 * A simple way to seed the generator would be to:
 *
 *	srandom(ir, ip, iq, 25)
 *
 * where 'ip', 'iq' and 'ir' are large integers that are unlikely to be
 * 'guessed' and where numbers around the size of iq*ir are beyond
 * the current reach of the best factoring methods on the fastest
 * SGI/Cray supercomuters.
 *
 * Of course you can increase the '25' value if 1 of 4^25 odds of a
 * non-prime are too probable for you.
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
 ***
 *
 * The Blum moduli used with the pre-defined generators (via the call
 * srandom(seed, 0<x<=16)) were generated using the above process.  The
 * 'ip', 'iq' and 'ir' values were produced by special purpose hardware
 * that produced cryptographically strong random numbers.  The Blum
 * primes used in these special pre-defined generators are unknown.
 *
 * Not being able to factor 'n=p*q' into 'p' and 'q' does not directly
 * improve the quality Blum generator.  On the other hand, it does
 * improve the security of it.
 *
 * I (Landon Curt Noll) did not keep the search values of these 20 special
 * pre-defined generators.  While some of the smaller Blum moduli is
 * within the range of some factoring methods, others are not.  As of
 * Feb 1997, the following is the estimate of what can factor the
 * pre-defined moduli:
 *
 *	1 <= newn < 4	PC using ECM in a short amount of time
 *	5 <= newn < 8	Workstation using MPQS in a short amount of time
 *	8 <= newn < 12	High end supercomputer or high parallel processor
 *			using state of the art factoring over a long time
 *     12 <= newn < 16	Beyond Feb 1997 systems and factoring methods
 *     17 <= newn < 20	Well beyond Feb 1997 systems and factoring methods
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
 ***
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


#include "zrandom.h"
#include "have_const.h"


/*
 * current Blum generator state
 */
static RANDOM blum;


/*
 * Default Blum generator
 *
 * The init_blum generator is established via the srandom(0) call.
 *
 * The z_rdefault ZVALUE is the 'r' (quadratic residue) of init_blum.
 */
static CONST HALF h_ndefvec[] = {
	HVAL(d6be,18d9), HVAL(ba55,a689), HVAL(4a90,27d9), HVAL(84f3,4b64),
	HVAL(d0e8,d302), HVAL(3d78,613e), HVAL(56af,9e37), HVAL(2ac1,9031),
	(HALF)0xf
};
static CONST HALF h_rdefvec[] = {
	HVAL(d3e7,b11e), HVAL(0f23,fd7e), HVAL(a625,46c9), HVAL(06c7,3ca5),
	HVAL(627d,6265), HVAL(2f1e,93a8), HVAL(ff4b,074e), HVAL(48b6,d882),
	(HALF)0x7
};
static CONST RANDOM init_blum = {1, 0, 8, (HALF)0, (HALF)0xff,
				 {(HALF *)h_ndefvec,
				  sizeof(h_ndefvec)/sizeof(HALF), 0},
				 {(HALF *)h_rdefvec,
				  sizeof(h_rdefvec)/sizeof(HALF), 0}};
static CONST HALF h_rdefvec_2[] = {
	HVAL(d3e7,b11e), HVAL(0f23,fd7e), HVAL(a625,46c9), HVAL(06c7,3ca5),
	HVAL(627d,6265), HVAL(2f1e,93a8), HVAL(ff4b,074e), HVAL(48b6,d882),
	(HALF)0x7
};
static CONST ZVALUE z_rdefault = {
    (HALF *)h_rdefvec_2, sizeof(h_rdefvec_2)/sizeof(HALF), 0
};


/*
 * Pre-defined Blum generators
 *
 * These generators are seeded via the srandom(0, newn) where
 * 1 <= newn < BLUM_PREGEN.
 */
static CONST HALF h_nvec01[] = {
	HVAL(83de,9361), HVAL(f0db,722d), HVAL(6fe3,28ca), HVAL(0494,4073),
	(HALF)0x5
};
static CONST HALF h_rvec01[] = {
	HVAL(a4cc,42ec), HVAL(4e5d,bb01), HVAL(11d9,52e7), HVAL(b226,980f)
};
static CONST HALF h_nvec02[] = {
	HVAL(3534,43f1), HVAL(eb28,6ea9), HVAL(dd37,4a18), HVAL(348a,2555),
	(HALF)0x2c5
};
static CONST HALF h_rvec02[] = {
	HVAL(21e3,a218), HVAL(e893,616b), HVAL(6cd7,10e3), HVAL(f3d6,4344),
	(HALF)0x40
};
static CONST HALF h_nvec03[] = {
	HVAL(11d0,01f1), HVAL(f2ca,661f), HVAL(3a81,f1e0), HVAL(59d6,ce4e),
	HVAL(0009,cfd9)
};
static CONST HALF h_rvec03[] = {
	HVAL(a0d7,d76a), HVAL(3e14,2de2), HVAL(ff5c,ea4f), HVAL(b44d,9b64),
	(HALF)0xfae5
};
static CONST HALF h_nvec04[] = {
	HVAL(dfcc,0751), HVAL(2dec,c680), HVAL(5df1,2a1a), HVAL(5c89,4ed7),
	HVAL(3070,f924)
};
static CONST HALF h_rvec04[] = {
	HVAL(4b98,4570), HVAL(a220,ddba), HVAL(a2c0,af8a), HVAL(131b,2bdc),
	HVAL(0020,c2d8)
};
static CONST HALF h_nvec05[] = {
	HVAL(9916,6ef1), HVAL(8b99,e5e7), HVAL(8769,a010), HVAL(5d3f,e111),
	HVAL(680b,c2fa), HVAL(38f7,5aac), HVAL(db81,a85b), HVAL(109b,1822),
	(HALF)0x2
};
static CONST HALF h_rvec05[] = {
	HVAL(59e2,efa9), HVAL(0e6c,77c8), HVAL(1e70,aeed), HVAL(234f,7b7d),
	HVAL(5f5d,f6db), HVAL(e821,a960), HVAL(ae33,b792), HVAL(5e9b,890e)
};
static CONST HALF h_nvec06[] = {
	HVAL(e1dd,f431), HVAL(d855,57f1), HVAL(5ee7,32da), HVAL(3a38,db77),
	HVAL(5c64,4026), HVAL(f2db,f218), HVAL(9ada,2c79), HVAL(7bfd,9d7d),
	(HALF)0xa
};
static CONST HALF h_rvec06[] = {
	HVAL(c940,4daf), HVAL(c5dc,2e80), HVAL(2c98,eccf), HVAL(e1f3,495d),
	HVAL(ce1c,925c), HVAL(e097,aede), HVAL(8866,7154), HVAL(5e94,a02f)
};
static CONST HALF h_nvec07[] = {
	HVAL(cf9e,c751), HVAL(602f,9125), HVAL(5288,2e7f), HVAL(0dcf,53ce),
	HVAL(ff56,9d6b), HVAL(6286,43fc), HVAL(3780,1cd5), HVAL(f239,9ef2),
	HVAL(43d8,7de8)
};
static CONST HALF h_rvec07[] = {
	HVAL(098d,25e6), HVAL(3992,d2e5), HVAL(64f0,b58c), HVAL(cf18,d4dd),
	HVAL(9d87,6aef), HVAL(7acc,ed04), HVAL(bfbe,9076), HVAL(1ee0,14c7),
	HVAL(0013,522d)
};
static CONST HALF h_nvec08[] = {
	HVAL(2674,2f11), HVAL(bc42,e66a), HVAL(b59c,d9f0), HVAL(9ad4,a6c2),
	HVAL(5bdb,d2f9), HVAL(bdc9,1fed), HVAL(f13c,9ce7), HVAL(eb46,99b7),
	HVAL(4712,6ca7), (HALF)0x58
};
static CONST HALF h_rvec08[] = {
	HVAL(489d,c674), HVAL(aae9,5f3a), HVAL(a35d,a929), HVAL(5597,b4b8),
	HVAL(28e9,c947), HVAL(3d34,4f9a), HVAL(b7e6,61fa), HVAL(a326,9116),
	HVAL(8530,16dc)
};
static CONST HALF h_nvec09[] = {
	HVAL(ab27,e3d1), HVAL(1274,5db4), HVAL(b980,f951), HVAL(62b1,6b66),
	HVAL(0fde,ce0d), HVAL(6061,c6fd), HVAL(36a6,ff09), HVAL(e08e,b61c),
	HVAL(84d8,95c3), HVAL(4a86,752a), HVAL(c179,7b4f), HVAL(5621,57a3),
	HVAL(3d26,7bb0), HVAL(14e8,1b00), HVAL(218d,9238), HVAL(5232,2fd3),
	HVAL(0039,e8be)
};
static CONST HALF h_rvec09[] = {
	HVAL(7d4e,d20d), HVAL(601e,f2b8), HVAL(8e59,f959), HVAL(edaa,5d9e),
	HVAL(309a,89ba), HVAL(e5ab,7d81), HVAL(796b,2545), HVAL(02de,3222),
	HVAL(8357,c0bd), HVAL(0107,e3fd), HVAL(82d9,d288), HVAL(bc42,a8aa),
	HVAL(4b78,7343), HVAL(c015,0886), HVAL(bab9,15bf), HVAL(f8ad,1e6b),
	(HALF)0xb458
};
static CONST HALF h_nvec10[] = {
	HVAL(b7e6,4b89), HVAL(c3cd,c363), HVAL(2ef9,c73c), HVAL(6092,ce22),
	HVAL(02ab,e36c), HVAL(08d4,9573), HVAL(7451,1c40), HVAL(d385,82de),
	HVAL(a524,a02f), HVAL(52c8,1b3b), HVAL(250d,3cc9), HVAL(23b5,0e88),
	HVAL(bd14,48bf), HVAL(882d,7f98), HVAL(c23e,f596), HVAL(c905,5666),
	HVAL(025f,2435)
};
static CONST HALF h_rvec10[] = {
	HVAL(94cf,c482), HVAL(594f,5ad4), HVAL(2344,2aee), HVAL(145f,40ce),
	HVAL(1baf,950d), HVAL(adc4,f175), HVAL(f62c,669f), HVAL(8d07,5d56),
	HVAL(08ed,8b40), HVAL(aaf2,cf30), HVAL(c24b,5ffb), HVAL(250d,f8cf),
	HVAL(7ca8,1ec9), HVAL(787e,2b70), HVAL(1840,1468), HVAL(47b2,0e0c),
	HVAL(0066,bb7e)
};
static CONST HALF h_nvec11[] = {
	HVAL(546e,e069), HVAL(2e1a,530c), HVAL(2014,dab2), HVAL(a729,cf52),
	HVAL(920e,e1a9), HVAL(68f2,7533), HVAL(2587,3cfa), HVAL(dd37,a749),
	HVAL(4499,daa2), HVAL(286e,5870), HVAL(57f3,f9b6), HVAL(5ec5,4467),
	HVAL(69a7,91ea), HVAL(874e,cd77), HVAL(4217,d56b), HVAL(82bd,b309),
	HVAL(4978,64de)
};
static CONST HALF h_rvec11[] = {
	HVAL(56e3,8b04), HVAL(3a0a,ded3), HVAL(461d,88b1), HVAL(9c09,4d65),
	HVAL(e533,3fed), HVAL(34d9,18fe), HVAL(1ef5,6281), HVAL(cedf,a07c),
	HVAL(590f,47fb), HVAL(a2c5,4d5c), HVAL(7323,39ee), HVAL(8065,49a7),
	HVAL(9ce3,163f), HVAL(ae3a,f8b6), HVAL(264a,4465), HVAL(1cb5,e630),
	HVAL(0086,8488)
};
static CONST HALF h_nvec12[] = {
	HVAL(f14c,7b99), HVAL(7f66,d151), HVAL(87ef,ad2b), HVAL(57d3,f098),
	HVAL(d653,4165), HVAL(812f,dd25), HVAL(48c7,c7ce), HVAL(a1bf,41e0),
	HVAL(4c94,e315), HVAL(190b,1593), HVAL(ee42,51da), HVAL(2b4a,1a66),
	HVAL(2bb7,c2a1), HVAL(65b1,8ca9), HVAL(08b8,9116), HVAL(c0cc,b15f),
	HVAL(5758,2ab3), (HALF)0x34
};
static CONST HALF h_rvec12[] = {
	HVAL(e207,b4a0), HVAL(5227,dd68), HVAL(9488,fbc4), HVAL(6ed0,81aa),
	HVAL(8e73,6fe5), HVAL(3dd2,c020), HVAL(eeb0,7c57), HVAL(0b60,4eb7),
	HVAL(a13f,72a1), HVAL(cbfc,4333), HVAL(a4c5,e8cd), HVAL(0add,520d),
	HVAL(758c,a3b2), HVAL(5549,0137), HVAL(5870,babd), HVAL(f648,ed93),
	HVAL(df71,9bd1)
};
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
/*
 * NOTE: set n is found in random_pregen[n-1]
 */
static RANDOM random_pregen[BLUM_PREGEN] = {
    {1, 0, 7, (HALF)0, (HALF)0x07f,
     {(HALF *)h_nvec01, sizeof(h_nvec01)/sizeof(HALF), 0},
     {(HALF *)h_rvec01, sizeof(h_rvec01)/sizeof(HALF), 0}},
    {1, 0, 7, (HALF)0, (HALF)0x07f,
     {(HALF *)h_nvec02, sizeof(h_nvec02)/sizeof(HALF), 0},
     {(HALF *)h_rvec02, sizeof(h_rvec02)/sizeof(HALF), 0}},
    {1, 0, 7, (HALF)0, (HALF)0x07f,
     {(HALF *)h_nvec03, sizeof(h_nvec03)/sizeof(HALF), 0},
     {(HALF *)h_rvec03, sizeof(h_rvec03)/sizeof(HALF), 0}},
    {1, 0, 7, (HALF)0, (HALF)0x07f,
     {(HALF *)h_nvec04, sizeof(h_nvec04)/sizeof(HALF), 0},
     {(HALF *)h_rvec04, sizeof(h_rvec04)/sizeof(HALF), 0}},
    {1, 0, 8, (HALF)0, (HALF)0x0ff,
     {(HALF *)h_nvec05, sizeof(h_nvec05)/sizeof(HALF), 0},
     {(HALF *)h_rvec05, sizeof(h_rvec05)/sizeof(HALF), 0}},
    {1, 0, 8, (HALF)0, (HALF)0x0ff,
     {(HALF *)h_nvec06, sizeof(h_nvec06)/sizeof(HALF), 0},
     {(HALF *)h_rvec06, sizeof(h_rvec06)/sizeof(HALF), 0}},
    {1, 0, 8, (HALF)0, (HALF)0x0ff,
     {(HALF *)h_nvec07, sizeof(h_nvec07)/sizeof(HALF), 0},
     {(HALF *)h_rvec07, sizeof(h_rvec07)/sizeof(HALF), 0}},
    {1, 0, 8, (HALF)0, (HALF)0x0ff,
     {(HALF *)h_nvec08, sizeof(h_nvec08)/sizeof(HALF), 0},
     {(HALF *)h_rvec08, sizeof(h_rvec08)/sizeof(HALF), 0}},
    {1, 0, 9, (HALF)0, (HALF)0x1ff,
     {(HALF *)h_nvec09, sizeof(h_nvec09)/sizeof(HALF), 0},
     {(HALF *)h_rvec09, sizeof(h_rvec09)/sizeof(HALF), 0}},
    {1, 0, 9, (HALF)0, (HALF)0x1ff,
     {(HALF *)h_nvec10, sizeof(h_nvec10)/sizeof(HALF), 0},
     {(HALF *)h_rvec10, sizeof(h_rvec10)/sizeof(HALF), 0}},
    {1, 0, 9, (HALF)0, (HALF)0x1ff,
     {(HALF *)h_nvec11, sizeof(h_nvec11)/sizeof(HALF), 0},
     {(HALF *)h_rvec11, sizeof(h_rvec11)/sizeof(HALF), 0}},
    {1, 0, 9, (HALF)0, (HALF)0x1ff,
     {(HALF *)h_nvec12, sizeof(h_nvec12)/sizeof(HALF), 0},
     {(HALF *)h_rvec12, sizeof(h_rvec12)/sizeof(HALF), 0}},
    {1, 0, 10, (HALF)0, (HALF)0x3ff,
     {(HALF *)h_nvec13, sizeof(h_nvec13)/sizeof(HALF), 0},
     {(HALF *)h_rvec13, sizeof(h_rvec13)/sizeof(HALF), 0}},
    {1, 0, 10, (HALF)0, (HALF)0x3ff,
     {(HALF *)h_nvec14, sizeof(h_nvec14)/sizeof(HALF), 0},
     {(HALF *)h_rvec14, sizeof(h_rvec14)/sizeof(HALF), 0}},
    {1, 0, 10, (HALF)0, (HALF)0x3ff,
     {(HALF *)h_nvec15, sizeof(h_nvec15)/sizeof(HALF), 0},
     {(HALF *)h_rvec15, sizeof(h_rvec15)/sizeof(HALF), 0}},
    {1, 0, 10, (HALF)0, (HALF)0x3ff,
     {(HALF *)h_nvec16, sizeof(h_nvec16)/sizeof(HALF), 0},
     {(HALF *)h_rvec16, sizeof(h_rvec16)/sizeof(HALF), 0}},
    {1, 0, 11, (HALF)0, (HALF)0x7ff,
     {(HALF *)h_nvec17, sizeof(h_nvec17)/sizeof(HALF), 0},
     {(HALF *)h_rvec17, sizeof(h_rvec17)/sizeof(HALF), 0}},
    {1, 0, 11, (HALF)0, (HALF)0x7ff,
     {(HALF *)h_nvec18, sizeof(h_nvec18)/sizeof(HALF), 0},
     {(HALF *)h_rvec18, sizeof(h_rvec18)/sizeof(HALF), 0}},
    {1, 0, 11, (HALF)0, (HALF)0x7ff,
     {(HALF *)h_nvec19, sizeof(h_nvec19)/sizeof(HALF), 0},
     {(HALF *)h_rvec19, sizeof(h_rvec19)/sizeof(HALF), 0}},
    {1, 0, 11, (HALF)0, (HALF)0x7ff,
     {(HALF *)h_nvec20, sizeof(h_nvec20)/sizeof(HALF), 0},
     {(HALF *)h_rvec20, sizeof(h_rvec20)/sizeof(HALF), 0}}
};


/*
 * zsrandom1 - seed the Blum generator 1 arg style
 *
 * We will seed the Blum generator according 2 argument
 * function description described at the top of this file.
 * In particular:
 *
 * given:
 *	pseed	 - seed of the generator
 *	need_ret - TRUE=>malloc return previous Blum state, FALSE=>return NULL
 *
 * returns:
 *	previous Blum state
 */
RANDOM *
zsrandom1(CONST ZVALUE seed, BOOL need_ret)
{
	RANDOM *ret;		/* previous Blum state */
	ZVALUE r;		/* quadratic residue */
	ZVALUE last_r;		/* previous quadratic residue */
	RANDOM *p_blum;		/* malloced RANDOM by randomcopy() */

	/*
	 * initialize state if first call
	 */
	if (!blum.seeded) {
		p_blum = randomcopy(&init_blum);
		blum = *p_blum;
		free(p_blum);
	}

	/*
	 * save the current state to return later, if need_ret says so
	 */
	if (need_ret) {
		ret = randomcopy(&blum);
	} else {
		ret = NULL;
	}

	/*
	 * srandom(seed == 0)
	 *
	 * If the init arg is TRUE, then restore the initial state and
	 * modulus of the Blum generator.  After this call, the Blum
	 * generator is restored to its initial state after calc started.
	 */
	if (ziszero(seed)) {

		/* set to the default generator state */
		zfree(blum.n);
		zfree(blum.r);
		p_blum = randomcopy(&init_blum);
		blum = *p_blum;
		free(p_blum);

	/*
	 * srandom(seed >= 2^32)
	 * srandom(seed >= 2^32, newn)
	 *
	 * Use seed to compute a new quadratic residue for use with
	 * the current Blum modulus.  We will successively square mod Blum
	 * modulus until we get a smaller value (modulus wrap).
	 *
	 * The Blum modulus will not be changed.
	 */
	} else if (!zisneg(seed) && zge32b(seed)) {

		/*
		 * square the seed mod the Blum modulus until we wrap
		 */
		zcopy(seed, &r);
		last_r.v = NULL;
		do {
			/* free temp storage */
			if (last_r.v != NULL) {
				zfree(last_r);
			}

			/*
			 * last_r = r;
			 * r = pmod(last_r, 2, n);
			 */
			last_r = r;
			zsquaremod(last_r, blum.n, &r);
		} while (zrel(r, last_r) > 0);
		zfree(blum.r);
		blum.r = r;
		/* free temp storage */
		zfree(last_r);

	/*
	 * reserved seed
	 */
	} else {
		math_error("srandom seed must be 0 or >= 2^32");
		/*NOTREACHED*/
	}

	/*
	 * flush the queued up bits
	 */
	blum.bits = 0;
	blum.buffer = 0;

	/*
	 * return the previous state
	 */
	return ret;
}


/*
 * zsrandom2 - seed the Blum generator 2 arg style
 *
 * We will seed the Blum generator according 2 argument
 * function description described at the top of this file.
 * In particular:
 *
 * given:
 *	pseed	- seed of the generator
 *	newn	- ptr to proposed new n (Blum modulus)
 *
 * returns:
 *	previous Blum state
 */
RANDOM *
zsrandom2(CONST ZVALUE seed, CONST ZVALUE newn)
{
	RANDOM *ret;		/* previous Blum state */
	HALF set;		/* pre-defined set to use */
	FULL nlen;		/* length of newn in bits */
	RANDOM *p_blum;		/* malloced RANDOM by randomcopy() */

	/*
	 * initialize state if first call
	 */
	if (!blum.seeded) {
		p_blum = randomcopy(&init_blum);
		blum = *p_blum;
		free(p_blum);
	}

	/*
	 * save the current state to return later
	 */
	ret = randomcopy(&blum);

	/*
	 * srandom(seed, 0 < newn <= 20)
	 *
	 * Set the Blum modulus to one of the the pre-defined Blum moduli.
	 * The new quadratic residue will also be set to one of
	 * the pre-defined quadratic residues.
	 */
	if (!zisneg(newn) && !zge32b(newn)) {

		/*
		 * preset moduli only if small newn
		 */
		if (ziszero(newn)) {
			math_error("srandom newn == 0 reserved for future use");
			/*NOTREACHED*/
		}
		set = (HALF)z1tol(newn);
		if (!zistiny(newn) || set > BLUM_PREGEN) {
			math_error("srandom small newn must be [1,20]");
			/*NOTREACHED*/
		}
		zfree(blum.n);
		zcopy(random_pregen[set-1].n, &blum.n);
		blum.loglogn = random_pregen[set-1].loglogn;
		blum.mask = random_pregen[set-1].mask;

		/*
		 * reset initial seed as well if seed is 0
		 */
		if (ziszero(seed)) {
			zfree(blum.r);
			zcopy(random_pregen[set-1].r, &blum.r);

		/*
		 * Otherwise non-zero seeds are processed as 1 arg calls
		 */
		} else {
			zsrandom1(seed, FALSE);
		}

	/*
	 * srandom(seed, newn >= 2^32)
	 *
	 * Assuming that 'newn' == 3 mod 4, then we will use it as
	 * the Blum modulus.
	 *
	 * We will use the seed arg to compute a new quadratic residue.
	 * We will successively square it mod Blum modulus until we get
	 * a smaller value (modulus wrap).
	 */
	} else if (!zisneg(newn)) {

		/*
		 * Blum modulus must be 1 mod 4
		 */
		if (newn.v[0] % 4 != 1) {
			math_error("srandom large newn must be 1 mod 4");
			/*NOTREACHED*/
		}

		/*
		 * For correct Blum moduli, hope they are a product
		 * of two primes.
		 */
		/* load modulus */
		zfree(blum.n);
		zcopy(newn, &blum.n);

		/*
		 * setup loglogn and mask
		 *
		 * If the length if excessive, reduce it down
		 * so that loglogn is at most BASEB-1.
		 */
		nlen = (FULL)zhighbit(newn);
		blum.loglogn = BASEB-1;
		if (nlen > 0 && nlen <= TOPHALF) {
			for (blum.loglogn=BASEB-1;
			     ((FULL)1<<blum.loglogn) > nlen && blum.loglogn > 1;
			     --blum.loglogn) {
			}
		}
		blum.mask = ((HALF)1 << blum.loglogn)-1;

		/*
		 * use default initial seed if seed is 0 and process
		 * as if this value is given as a 1 arg call
		 */
		if (ziszero(seed)) {
			(void) zsrandom1(z_rdefault, FALSE);

		/*
		 * Otherwise non-zero seeds are processed as 1 arg calls
		 */
		} else {
			(void) zsrandom1(seed, FALSE);
		}

	/*
	 * reserved newn
	 */
	} else {
		math_error("srandom newn must be [1,20] or >= 2^32");
		/*NOTREACHED*/
	}

	/*
	 * flush the queued up bits
	 */
	blum.bits = 0;
	blum.buffer = 0;

	/*
	 * return the previous state
	 */
	return ret;
}


/*
 * zsrandom4 - seed the Blum generator 4 arg style
 *
 * We will seed the Blum generator according 2 argument
 * function description described at the top of this file.
 * In particular:
 *
 * given:
 *	pseed	- seed of the generator
 *	ip	- initial p search point
 *	iq	- initial q search point
 *	trials	- number of ptests to perform per candidate prime
 *
 * returns:
 *	previous Blum state
 */
RANDOM *
zsrandom4(CONST ZVALUE seed, CONST ZVALUE ip, CONST ZVALUE iq, long trials)
{
	RANDOM *ret;		/* previous Blum state */
	FULL nlen;		/* length of n=p*q in bits */
	ZVALUE p;		/* 1st Blum prime */
	ZVALUE q;		/* 2nd Blum prime */
	RANDOM *p_blum;		/* malloced RANDOM by randomcopy() */

	/*
	 * initialize state if first call
	 */
	if (!blum.seeded) {
		p_blum = randomcopy(&init_blum);
		blum = *p_blum;
		free(p_blum);
	}

	/*
	 * save the current state to return later
	 */
	ret = randomcopy(&blum);

	/*
	 * search the 'p' and 'q' Blum prime (3 mod 4) candidates
	 */
	if (!znextcand(ip, trials, _zero_, zconst[3], zconst[4], &p)) {
		math_error("failed to find 1st Blum prime");
		/*NOTREACHED*/
	}
	if (!znextcand(iq, trials, _zero_, zconst[3], zconst[4], &q)) {
		math_error("failed to find 2nd Blum prime");
		/*NOTREACHED*/
	}

	/*
	 * form the Blum modulus
	 */
	zfree(blum.n);
	zmul(p, q, &blum.n);
	/* free temp storage */
	zfree(p);
	zfree(q);

	/*
	 * form the loglogn and mask
	 */
	nlen = (FULL)zhighbit(blum.n);
	blum.loglogn = BASEB-1;
	if (nlen > 0 && nlen <= TOPHALF) {
		for (blum.loglogn=BASEB-1;
		     ((FULL)1<<blum.loglogn) > nlen && blum.loglogn > 1;
		     --blum.loglogn) {
		}
	}
	blum.mask = ((HALF)1 << blum.loglogn)-1;

	/*
	 * use default initial seed if seed is 0 and process
	 * as if this value is given as a 1 arg call
	 */
	if (ziszero(seed)) {
		(void) zsrandom1(z_rdefault, FALSE);

	/*
	 * Otherwise non-zero seeds are processed as 1 arg calls
	 */
	} else {
		(void) zsrandom1(seed, FALSE);
	}

	/*
	 * flush the queued up bits
	 */
	blum.bits = 0;
	blum.buffer = 0;

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
 *    In particular:
 *
 *	zsetrandom(pseed) is called by: srandom() and srandom(state)
 *
 * returns:
 *	previous RANDOM state
 */
RANDOM *
zsetrandom(CONST RANDOM *state)
{
	RANDOM *ret;		/* previous Blum state */
	RANDOM *p_blum;		/* malloced RANDOM by randomcopy() */

	/*
	 * initialize state if first call
	 */
	if (!blum.seeded) {
		p_blum = randomcopy(&init_blum);
		blum = *p_blum;
		free(p_blum);
	}

	/*
	 * save the current state to return later
	 */
	ret = randomcopy(&blum);

	/*
	 * load the new state
	 */
	if (state != NULL) {
		p_blum = randomcopy(state);
		blum = *p_blum;
		free(p_blum);
	}

	/*
	 * return the previous state
	 */
	return ret;
}


/*
 * zrandomskip - skip s bits via the Blum-Blum-Shub generator
 *
 * given:
 *	count - number of bits to be skipped
 */
void
zrandomskip(long cnt)
{
	ZVALUE new_r;		/* new quadratic residue */
	long loglogn;		/* blum.loglogn */
	RANDOM *p_blum;		/* malloced RANDOM by randomcopy() */

	/*
	 * initialize state if first call
	 */
	if (!blum.seeded) {
		p_blum = randomcopy(&init_blum);
		blum = *p_blum;
		free(p_blum);
	}
	loglogn = (long)blum.loglogn;

	/*
	 * skip required bits in the buffer
	 */
	if (blum.bits > 0) {

		/*
		 * depending in if we have too few or too many in the buffer
		 */
		if (blum.bits <= cnt) {

			/* too few - just toss the buffer bits */
			cnt -= blum.bits;
			blum.bits = 0;
			blum.buffer = 0;

		} else {

			/* buffer contains more bits than we need to toss */
			blum.buffer >>= cnt;
			blum.bits -= cnt;
			return;	/* skip need satisfied */
		}
	}

	/*
	 * skip loglogn bits at a time
	 */
	while (cnt >= loglogn) {

		/* turn the Blum-Blum-Shub crank */
		zsquaremod(blum.r, blum.n, &new_r);
		zfree(blum.r);
		blum.r = new_r;
		cnt -= blum.loglogn;
	}

	/*
	 * skip the final bits
	 */
	if (cnt > 0) {

		/* turn the Blum-Blum-Shub crank */
		zsquaremod(blum.r, blum.n, &new_r);
		zfree(blum.r);
		blum.r = new_r;

		/* fill the buffer with the unused bits */
		blum.bits = loglogn - cnt;
		blum.buffer = (blum.r.v[0] & lowhalf[blum.bits]);
	}
	return;
}


/*
 * zrandom - crank the Blum-Blum-Shub generator for some bits
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
zrandom(long cnt, ZVALUE *res)
{
	BITSTR dest;		/* destination bit string */
	int loglogn;		/* blum.loglogn */
	HALF mask;		/* mask for bottom loglogn bits */
	ZVALUE new_r;		/* new quadratic residue */
	RANDOM *p_blum;		/* malloced RANDOM by randomcopy() */
	int t;			/* temp shift value */

	/*
	 * firewall
	 */
	if (cnt <= 0) {
		if (cnt == 0) {
			/* zero length random number is always 0 */
			itoz(0, res);
			return;
		} else {
			math_error("negative zrandom bit count");
			/*NOTREACHED*/
		}
#if LONG_BITS > 32
	} else if (cnt > (1L<<31)) {
		math_error("huge random count in internal zrandom function");
		/*NOTREACHED*/
#endif
	}

	/*
	 * initialize state if first call
	 */
	if (!blum.seeded) {
		p_blum = randomcopy(&init_blum);
		blum = *p_blum;
		free(p_blum);
	}
	loglogn = blum.loglogn;
	mask = blum.mask;

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
	*dest.loc = 0;

	/*
	 * load from buffer first
	 */
	if (blum.bits > 0) {

		/*
		 * If we need only part of the buffer, use
		 * the top bits and keep the bottom in place.
		 * If we need extactly all of the buffer,
		 * process it as a partial buffer fill.
		 */
		if (dest.len <= blum.bits) {

			/* load part of the buffer */
			*dest.loc = (blum.buffer >> (blum.bits-dest.len));

			/* update buffer */
			blum.buffer &= ((1 << (blum.bits-dest.len))-1);
			blum.bits -= dest.len;

			/* cleanup */
			res->sign = 0;
			ztrim(res);

			/* we are done now */
			return;
		}

		/*
		 * Otherwise we need all of the buffer and then some ...
		 *
		 * dest.len > blum.bits
		 *
		 * NOTE: We use = instead of |= as this will ensure that
		 *	 bit bits above dest.bit are set to 0.
		 */
		if (dest.bit >= blum.bits) {
			/* copy all of buffer into upper element */
			*dest.loc = (blum.buffer << (dest.bit+1-blum.bits));
			dest.bit -= blum.bits;
		} else {
			/* copy buffer into upper and next element */
			t = blum.bits-(dest.bit+1);
			*dest.loc-- = (blum.buffer >> t);
			dest.bit = BASEB-t-1;
			*dest.loc = ((blum.buffer&lowhalf[t]) << (dest.bit+1));
		}
		dest.len -= blum.bits;
	}

	/*
	 * Crank the generator up until, but not including, the
	 * time when we will write into the least significant bit.
	 *
	 * In this loop we know that we have exactly blum.loglogn bits
	 * to load.
	 */
	while (dest.len > loglogn) {

		/*
		 * turn the Blum-Blum-Shub crank
		 */
		zsquaremod(blum.r, blum.n, &new_r);
		zfree(blum.r);
		blum.r = new_r;
		/* peal off the bottom loglogn bits */
		blum.buffer = (blum.r.v[0] & mask);

		/*
		 * load the loglogn bits into dest.loc starting at bit dest.bit
		 */
		if (dest.bit >= loglogn) {
			/* copy all of buffer into upper element */
			*dest.loc |= (blum.buffer << (dest.bit+1-loglogn));
			dest.bit -= loglogn;
		} else {
			/* copy buffer into upper and next element */
			t = loglogn-(dest.bit+1);
			*dest.loc-- |= (blum.buffer >> t);
			dest.bit = BASEB-t-1;
			*dest.loc = ((blum.buffer&lowhalf[t]) << (dest.bit+1));
		}
		dest.len -= loglogn;
	}

	/*
	 * We have a full or less than a full crank (loglogn bits) left
	 * to generate and load into the least significant bits.
	 *
	 * If we have any bits left over, we will save them in the
	 * buffer for use by the next call.
	 */
	/* turn the Blum-Blum-Shub crank */
	zsquaremod(blum.r, blum.n, &new_r);
	zfree(blum.r);
	blum.r = new_r;
	/* peal off the bottom loglogn bits */
	blum.buffer = (blum.r.v[0] & mask);
	blum.bits = loglogn;

	/*
	 * load dest.len bits into the lowest order bits
	 */
	*dest.loc |= (blum.buffer >> (loglogn - dest.len));

	/*
	 * leave any unused bits in the buffer for next time
	 */
	blum.buffer &= lowhalf[loglogn - dest.len];
	blum.bits -= dest.len;

	/*
	 * cleanup
	 */
	res->sign = 0;
	ztrim(res);
}


/*
 * zrandomrange - generate a Blum-Blum-Shub random value in [low, high)
 *
 * given:
 *	low - low value of range
 *	high - beyond end of range
 *	res - where to place the random bits as ZVALUE
 */
void
zrandomrange(CONST ZVALUE low, CONST ZVALUE high, ZVALUE *res)
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
		zrandom(bitlen, &rval);
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
 * irandom - generate a Blum-Blum-Shub random long in the range [0, s)
 *
 * given:
 *	s - limit of the range
 *
 * returns:
 *	random long in the range [0, s)
 */
long
irandom(long s)
{
	ZVALUE z1, z2;
	long res;

	if (s <= 0) {
		math_error("Non-positive argument for irandom()");
		/*NOTREACHED*/
	}
	if (s == 1)
		return 0;
	itoz(s, &z1);
	zrandomrange(_zero_, z1, &z2);
	res = ztoi(z2);
	zfree(z1);
	zfree(z2);
	return res;
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
	if (state->r.v == NULL) {
		ret->r.v = NULL;
	} else {
		zcopy(state->r, &ret->r);
	}
	if (state->n.v == NULL) {
		ret->n.v = NULL;
	} else {
		zcopy(state->n, &ret->n);
	}

	/*
	 * return copy
	 */
	return ret;
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
	/* avoid free of the pre-defined states */
	if (state == &init_blum) {
		return;
	}
	if (state >= &random_pregen[0] &&
	    state <= &random_pregen[BLUM_PREGEN-1]) {
		return;
	}

	/* free the values */
	state->seeded = 0;
	zfree(state->n);
	zfree(state->r);

	/* free it if it is not pre-defined */
	if (state != &blum) {
		free(state);
	}
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
			return FALSE;
		} else {
			/* uninitialized only equals default state */
			return randomcmp(s2, &init_blum);
		}
	} else if (!s2->seeded) {
		/* uninitialized only equals default state */
		return randomcmp(s1, &init_blum);
	}

	/*
	 * compare operating mask parameters
	 */
	if ((s1->loglogn != s2->loglogn) || (s1->mask != s2->mask)) {
		return TRUE;
	}

	/*
	 * compare bit buffer
	 */
	if ((s1->bits != s2->bits) || (s1->buffer != s2->buffer)) {
		return TRUE;
	}

	/*
	 * compare quadratic residues and moduli
	 */
	return (zcmp(s1->r, s2->r) && zcmp(s1->n, s2->n));
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


/*
 * random_libcalc_cleanup - cleanup code for final libcalc_call_me_last() call
 *
 * This call is needed only by libcalc_call_me_last() to help clean up any
 * unneeded storage.
 *
 * Do not call this function directly!  Let libcalc_call_me_last() do it.
 */
void
random_libcalc_cleanup(void)
{
	/* free if we are seeded now */
	if (blum.seeded) {
		randomfree(&blum);
	}
	return;
}
