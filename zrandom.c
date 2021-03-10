/*
 * zrandom - Blum-Blum-Shub pseudo-random generator
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
 * Under source code control:	1997/02/15 04:01:56
 * File existed as early as:	1997
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
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
 *	does not help.	The BITS THAT FOLLOW OR PRECEDE A SEQUENCE
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
 * generator iteration.	 Any unused bits are saved for the next call
 * to the generator.  The Blum generator is not too slow, though
 * seeding the generator via srandom(seed,plen,qlen) can be slow.
 * Shortcuts and pre-defined generators have been provided for this reason.
 * Use of Blum should be more than acceptable for many applications.
 *
 * The Blum generator as the following calc interfaces:
 *
 *   random(min, beyond)		(where min < beyond)
 *
 *	Print a Blum generator random value over interval [min,beyond).
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
 *	Same as random(0, 2^x).	 Print x bits.
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
 *	The follow calc resource file produces an equivalent effect:
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
 *	The follow calc resource file produces an equivalent effect:
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
 *	The follow calc resource file produces an equivalent effect:
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
 *	The follow calc resource file produces an equivalent effect:
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
 *	Set the Blum modulus to one of the pre-defined Blum moduli.
 *	See below for the values of these pre-defined Blum moduli and how
 *	they were computed.
 *
 *	We will use the seed arg to compute a new quadratic residue.
 *	We will successively square it mod Blum modulus until we get
 *	a smaller value (modulus wrap).
 *
 *	The follow calc resource file produces an equivalent effect:
 *
 *	  n = n[newn];		(* n is new Blum modulus, see below *)
 *	  r = seed;
 *	  do {
 *	      last_r = r;
 *	      r = pmod(last_r, 2, n);
 *	  } while (r > last_r); (* r is the new quadratic residue *)
 *
 *    0 < seed < 2^32, 0 < newn <= 20:
 *    --------------------------------
 *	Reserved for future use.
 *
 *    seed == 0, 0 < newn <= 20:
 *    --------------------------
 *	Set the Blum modulus to one of the pre-defined Blum moduli.
 *	The new quadratic residue will also be set to one of
 *	the pre-defined quadratic residues.
 *
 *	The follow calc resource file produces an equivalent effect:
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
 *	were selected by LavaRnd, a hardware random number generator.
 *	See the URL:
 *
 *	    http://www.LavaRnd.org/
 *
 *	for an explanation of how the LavaRnd random number generator works.
 *
 *	For a given newn, we select a given bit length.	 For 0 < newn <= 20,
 *	the bit length selected was by:
 *
 *	    bitlen = 2^(int((newn-1)/4)+7) + small_random_value;
 *
 *	where small_random_value is also generated by LavaRnd.	 For
 *	1 <= newn <= 16, small_random_value is a random value in [0,40).
 *	For 17 < newn <= 20, small_random_value is a random value in [0,120).
 *	Given two random integers generated by LavaRnd, we used the following
 *	to compute Blum primes:
 *
 *	  (* find the first Blum prime *)
 *	  fp = int((ip-1)/2);		(* ip was generated by LavaRnd *)
 *	  do {
 *	      fp = nextcand(fp+2, 25, 0, 3, 4);
 *	      p = 2*fp+1;
 *	  } while (ptest(p, 25) == 0);
 *
 *	  (* find the 2nd Blum prime *)
 *	  fq = int((iq-1)/2);		(* iq was generated by LavaRnd *)
 *	  do {
 *	      fq = nextcand(fq+2, 25, 0, 3, 4);
 *	      q = 2*fq+1;
 *	  } while (ptest(q, 25) == 0);
 *
 *	  (* compute the Blum modulus *)
 *	  n[newn] = p * q;
 *
 *	The pre-defined quadratic residues was also generated by LavaRnd.
 *	The value produced by LavaRnd was squared mod the Blum moduli
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
 *	newn ==	 1: (Blum modulus bit length 130)
 *	n[ 1] = 0x5049440736fe328caf0db722d83de9361
 *	r[ 1] = 0xb226980f11d952e74e5dbb01a4cc42ec
 *
 *	newn ==	 2: (Blum modulus bit length 137)
 *	n[ 2] = 0x2c5348a2555dd374a18eb286ea9353443f1
 *	r[ 2] = 0x40f3d643446cd710e3e893616b21e3a218
 *
 *	newn ==	 3: (Blum modulus bit length 147)
 *	n[ 3] = 0x9cfd959d6ce4e3a81f1e0f2ca661f11d001f1
 *	r[ 3] = 0xfae5b44d9b64ff5cea4f3e142de2a0d7d76a
 *
 *	newn ==	 4: (Blum modulus bit length 157)
 *	n[ 4] = 0x3070f9245c894ed75df12a1a2decc680dfcc0751
 *	r[ 4] = 0x20c2d8131b2bdca2c0af8aa220ddba4b984570
 *
 *	newn ==	 5: (Blum modulus bit length 257)
 *	n[ 5] = 0x2109b1822db81a85b38f75aac680bc2fa5d3fe1118769a0108b99e5e799
 *		166ef1
 *	r[ 5] = 0x5e9b890eae33b792e821a9605f5df6db234f7b7d1e70aeed0e6c77c859e
 *		2efa9
 *
 *	newn ==	 6: (Blum modulus bit length 259)
 *	n[ 6] = 0xa7bfd9d7d9ada2c79f2dbf2185c6440263a38db775ee732dad85557f1e1
 *		ddf431
 *	r[ 6] = 0x5e94a02f88667154e097aedece1c925ce1f3495d2c98eccfc5dc2e80c94
 *		04daf
 *
 *	newn ==	 7: (Blum modulus bit length 286)
 *	n[ 7] = 0x43d87de8f2399ef237801cd5628643fcff569d6b0dcf53ce52882e7f602
 *		f9125cf9ec751
 *	r[ 7] = 0x13522d1ee014c7bfbe90767acced049d876aefcf18d4dd64f0b58c3992d
 *		2e5098d25e6
 *
 *	newn ==	 8: (Blum modulus bit length 294)
 *	n[ 8] = 0x5847126ca7eb4699b7f13c9ce7bdc91fed5bdbd2f99ad4a6c2b59cd9f0b
 *		c42e66a26742f11
 *	r[ 8] = 0x853016dca3269116b7e661fa3d344f9a28e9c9475597b4b8a35da929aae
 *		95f3a489dc674
 *
 *	newn ==	 9: (Blum modulus bit length 533)
 *	n[ 9] = 0x39e8be52322fd3218d923814e81b003d267bb0562157a3c1797b4f4a867
 *		52a84d895c3e08eb61c36a6ff096061c6fd0fdece0d62b16b66b980f95112
 *		745db4ab27e3d1
 *	r[ 9] = 0xb458f8ad1e6bbab915bfc01508864b787343bc42a8aa82d9d2880107e3f
 *		d8357c0bd02de3222796b2545e5ab7d81309a89baedaa5d9e8e59f959601e
 *		f2b87d4ed20d
 *
 *	newn == 10: (Blum modulus bit length 537)
 *	n[10] = 0x25f2435c9055666c23ef596882d7f98bd1448bf23b50e88250d3cc952c8
 *		1b3ba524a02fd38582de74511c4008d4957302abe36c6092ce222ef9c73cc
 *		3cdc363b7e64b89
 *	r[10] = 0x66bb7e47b20e0c18401468787e2b707ca81ec9250df8cfc24b5ffbaaf2c
 *		f3008ed8b408d075d56f62c669fadc4f1751baf950d145f40ce23442aee59
 *		4f5ad494cfc482
 *
 *	newn == 11: (Blum modulus bit length 542)
 *	n[11] = 0x497864de82bdb3094217d56b874ecd7769a791ea5ec5446757f3f9b6286
 *		e58704499daa2dd37a74925873cfa68f27533920ee1a9a729cf522014dab2
 *		2e1a530c546ee069
 *	r[11] = 0x8684881cb5e630264a4465ae3af8b69ce3163f806549a7732339eea2c54
 *		d5c590f47fbcedfa07c1ef5628134d918fee5333fed9c094d65461d88b13a
 *		0aded356e38b04
 *
 *	newn == 12: (Blum modulus bit length 549)
 *	n[12] = 0x3457582ab3c0ccb15f08b8911665b18ca92bb7c2a12b4a1a66ee4251da1
 *		90b15934c94e315a1bf41e048c7c7ce812fdd25d653416557d3f09887efad
 *		2b7f66d151f14c7b99
 *	r[12] = 0xdf719bd1f648ed935870babd55490137758ca3b20add520da4c5e8cdcbf
 *		c4333a13f72a10b604eb7eeb07c573dd2c0208e736fe56ed081aa9488fbc4
 *		5227dd68e207b4a0
 *
 *	newn == 13: (Blum modulus bit length 1048)
 *	n[13] = 0x1517c19166b7dd21b5af734ed03d833daf66d82959a553563f4345bd439
 *		510a7bda8ee0cb6bf6a94286bfd66e49e25678c1ee99ceec891da8b18e843
 *		7575113aaf83c638c07137fdd3a76c3a49322a11b5a1a84c32d99cbb2b056
 *		671589917ed14cc7f1b5915f6495dd1892b4ed7417d79a63cc8aaa503a208
 *		e3420cca200323314fc49
 *	r[13] = 0xd42e8e9a560d1263fa648b04f6a69b706d2bc4918c3317ddd162cb4be7a
 *		5e3bbdd1564a4aadae9fd9f00548f730d5a68dc146f05216fe509f0b8f404
 *		902692de080bbeda0a11f445ff063935ce78a67445eae5c9cea5a8f6b9883
 *		faeda1bbe5f1ad3ef6409600e2f67b92ed007aba432b567cc26cf3e965e20
 *		722407bfe46b7736f5
 *
 *	newn == 14: (Blum modulus bit length 1054)
 *	n[14] = 0x5e56a00e93c6f4e87479ac07b9d983d01f564618b314b4bfec7931eee85
 *		eb909179161e23e78d32110560b22956b22f3bc7e4a034b0586e463fd40c6
 *		f01a33e30ede912acb86a0c1e03483c45f289a271d14bd52792d0a076fdfe
 *		fe32159054b217092237f0767434b3db112fee83005b33f925bacb3185cc4
 *		409a1abdef8c0fc116af01
 *	r[14] = 0xf7aa7cb67335096ef0c5d09b18f15415b9a564b609913f75f627fc6b0c5
 *		b686c86563fe86134c5a0ea19d243350dfc6b9936ba1512abafb81a0a6856
 *		c9ae7816bf2073c0fb58d8138352b261a704b3ce64d69dee6339010186b98
 *		3677c84167d4973444194649ad6d71f8fa8f1f1c313edfbbbb6b1b220913c
 *		c8ea47a4db680ff9f190
 *
 *	newn == 15: (Blum modulus bit length 1055)
 *	n[15] = 0x97dd840b9edfbcdb02c46c175ba81ca845352ebe470be6075326a26770c
 *		ab84bfc0f2e82aa95aac14f40de42a0590445b902c2b8ebb916753e72ab86
 *		c3278cccc1a783b3e962d81b80df03e4380a8fa08b0d86ed0caa515c196a5
 *		30e49c558ddb53082310b1d0c7aee6f92b619798624ffe6c337299bc51ff5
 *		d2c721061e7597c8d97079
 *	r[15] = 0xb8220703b8c75869ab99f9b50025daa8d77ca6df8cef423ede521f55b1c
 *		25d74fbf6d6cc31f5ef45e3b29660ef43797f226860a4aa1023dbe522b1fe
 *		6224d01eb77dee9ad97e8970e4a9e28e7391a6a70557fa0e46eca78866241
 *		ba3c126fc0c5469f8a2f65c33db95d1749d3f0381f401b9201e6abd43d98d
 *		b92e808f0aaa6c3e2110
 *
 *	newn == 16: (Blum modulus bit length 1062)
 *	n[16] = 0x456e348549b82fbb12b56f84c39f544cb89e43536ae8b2b497d426512c7
 *		f3c9cc2311e0503928284391959e379587bc173e6bc51ba51c856ba557fee
 *		8dd69cee4bd40845bd34691046534d967e40fe15b6d7cf61e30e283c05be9
 *		93c44b6a2ea8ade0f5578bd3f618336d9731fed1f1c5996a5828d4ca857ac
 *		2dc9bd36184183f6d84346e1
 *	r[16] = 0xb0d7dcb19fb27a07973e921a4a4b6dcd7895ae8fced828de8a81a3dbf25
 *		24def719225404bfd4977a1508c4bac0f3bc356e9d83b9404b5bf86f6d19f
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
 *	r[17] = 0x53720b6eaf3bc3b8adf1dd665324c2d2fc5b2a62f32920c4e167537284d
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
 *	      speed relative the default Blum generator.  As of Jan 1997,
 *	      the Blum moduli associated with 13 <= newn < 20 appear to
 *	      be well beyond the scope of hardware and algorithms,
 *	      and 9 <= newn < 12 might be factorable with extreme difficulty.
 *
 *	The following table may be useful as a guide for how easy it
 *	is to factor the modulus:
 *
 *	1 <= newn <= 4	PC using ECM in a short amount of time
 *	5 <= newn <= 8	Workstation using MPQS in a short amount of time
 *	8 <= newn <= 12 High end supercomputer or high parallel processor
 *			using state of the art factoring over a long time
 *     12 <= newn <= 16 Beyond Feb 1997 systems and factoring methods
 *     17 <= newn <= 20 Well beyond Feb 1997 systems and factoring methods
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
 *	The follow calc resource file produces an equivalent effect:
 *
 *	  p = nextcand(ip-2, trials, 0, 3, 4);	  (* find the 1st Blum prime *)
 *	  q = nextcand(iq-2, trials, 0, 3, 4);	  (* find the 2nd Blum prime *)
 *	  n = p * q;			(* n is the new Blum modulus *)
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
 *	The follow calc resource file produces an equivalent effect:
 *
 *	  srandom(default_residue, ip, iq, trials)
 *
 *	or in other words:
 *
 *	  (* trials, if omitted, is assumed to be 1 *)
 *	  p = nextcand(ip-2, trials, 0, 3, 4);	  (* find the 1st Blum prime *)
 *	  q = nextcand(iq-2, trials, 0, 3, 4);	  (* find the 2nd Blum prime *)
 *	  n = p * q;			(* n is the new Blum modulus *)
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
 * less than 1 in 4^25.	 For our purposes, this is sufficient as the
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
 * found in this file were generated.  Detains can be found in the
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
 * residues that we would first avoid.	However such an occurrence will
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
 * per crank of the generator.	The period of this generator is long
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
 * The above resource file produces the Blum probable primes and initial
 * quadratic residue (line wrapped for readability):
 *
 *	    p= 0x798ac934c7a3318ad446190f3474e57
 *
 *	    q= 0x1ff21d7e1dd7d5965e224d485d84c3ef44f
 *
 * These Blum primes were found after 1.81s of CPU time on a 195 Mhz IP28
 * R10000 version 2.5 processor.  The first Blum prime 'p' was 31716 higher
 * than the initial search value 'ip'.	The second Blum prime 'q' was 18762
 * higher than the initial starting 'iq'.
 *
 * The product of the two Blum primes results in a 260 bit Blum modulus of:
 *
 *	n = 0xf2ac1903156af9e373d78613ed0e8d30284f34b644a9027d9ba55a689d6be18d9
 *
 * The selection if the initial quadratic residue comes from the next
 * unused digits of the Rand Book of Random Numbers.  Now the two initial
 * search values 'ip' and 'iq' used above needed the first 38 digits and
 * the next 42 digits.	Thus we will skip the first 38+42=80 digits
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
 * a factor.  The calc resource file above ensures that '(p-1)/2' and
 * '(q-1)/2' are probable prime, thus maximizing the period
 * of the default generator to:
 *
 *	lambda(n) = lcm(2,2,fp,fq) = 2*fp*fq = ~2*(p/2)*(q/2) = ~n/2
 *
 * The process above resulted in a default generator Blum modulus n > 2^259
 * with period of at least 2^258 bits.	To be exact, the period of the
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
 * or where the two primes are nearly the same.	 I elected to have the
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
 * section are a lie intended to entrap people.	 Well they are not, but
 * you need not take my word for it.
 *
 ***
 *
 * One could take issue with the above resource file that produced a 260 bit
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
 * where 'ir' is the initial Blum quadratic residue generator.	The 'ir'
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
 * improve the quality Blum generator.	On the other hand, it does
 * improve the security of it.
 *
 * I (Landon Curt Noll) did not keep the search values of these 20 special
 * pre-defined generators.  While some of the smaller Blum moduli is
 * within the range of some factoring methods, others are not.	As of
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
 * It is true that the pre-defined moduli are 'magic'.  On the other
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
#include "have_unused.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * current Blum generator state
 */
STATIC RANDOM blum;


/*
 * Default Blum generator
 *
 * The init_blum generator is established via the srandom(0) call.
 *
 * The z_rdefault ZVALUE is the 'r' (quadratic residue) of init_blum.
 */
#if FULL_BITS == 64
STATIC CONST HALF h_ndefvec[] = {
	(HALF)0xd6be18d9, (HALF)0xba55a689, (HALF)0x4a9027d9, (HALF)0x84f34b64,
	(HALF)0xd0e8d302, (HALF)0x3d78613e, (HALF)0x56af9e37, (HALF)0x2ac19031,
	(HALF)0xf
};
STATIC CONST HALF h_rdefvec[] = {
	(HALF)0xd3e7b11e, (HALF)0x0f23fd7e, (HALF)0xa62546c9, (HALF)0x06c73ca5,
	(HALF)0x627d6265, (HALF)0x2f1e93a8, (HALF)0xff4b074e, (HALF)0x48b6d882,
	(HALF)0x7
};
#elif 2*FULL_BITS == 64
STATIC CONST HALF h_ndefvec[] = {
	(HALF)0x18d9, (HALF)0xd6be, (HALF)0xa689, (HALF)0xba55,
	(HALF)0x27d9, (HALF)0x4a90, (HALF)0x4b64, (HALF)0x84f3,
	(HALF)0xd302, (HALF)0xd0e8, (HALF)0x613e, (HALF)0x3d78,
	(HALF)0x9e37, (HALF)0x56af, (HALF)0x9031, (HALF)0x2ac1,
	(HALF)0xf
};
STATIC CONST HALF h_rdefvec[] = {
	(HALF)0xb11e, (HALF)0xd3e7, (HALF)0xfd7e, (HALF)0x0f23,
	(HALF)0x46c9, (HALF)0xa625, (HALF)0x3ca5, (HALF)0x06c7,
	(HALF)0x6265, (HALF)0x627d, (HALF)0x93a8, (HALF)0x2f1e,
	(HALF)0x074e, (HALF)0xff4b, (HALF)0xd882, (HALF)0x48b6,
	(HALF)0x7
};
#else
   /\../\	FULL_BITS must be 32 or 64	/\../\	 !!!
#endif
STATIC CONST RANDOM init_blum = {1, 0, 8, (HALF)0, (HALF)0xff,
				 {(HALF *)h_ndefvec,
				  sizeof(h_ndefvec)/sizeof(HALF), 0},
				 {(HALF *)h_rdefvec,
				  sizeof(h_rdefvec)/sizeof(HALF), 0}};
#if FULL_BITS == 64
STATIC CONST HALF h_rdefvec_2[] = {
	(HALF)0xd3e7b11e, (HALF)0x0f23fd7e, (HALF)0xa62546c9, (HALF)0x06c73ca5,
	(HALF)0x627d6265, (HALF)0x2f1e93a8, (HALF)0xff4b074e, (HALF)0x48b6d882,
	(HALF)0x7
};
#elif 2*FULL_BITS == 64
STATIC CONST HALF h_rdefvec_2[] = {
	(HALF)0xb11e, (HALF)0xd3e7, (HALF)0xfd7e, (HALF)0x0f23,
	(HALF)0x46c9, (HALF)0xa625, (HALF)0x3ca5, (HALF)0x06c7,
	(HALF)0x6265, (HALF)0x627d, (HALF)0x93a8, (HALF)0x2f1e,
	(HALF)0x074e, (HALF)0xff4b, (HALF)0xd882, (HALF)0x48b6,
	(HALF)0x7
};
#else
   /\../\	FULL_BITS must be 32 or 64	/\../\	 !!!
#endif
STATIC CONST ZVALUE z_rdefault = {
    (HALF *)h_rdefvec_2, sizeof(h_rdefvec_2)/sizeof(HALF), 0
};


/*
 * Pre-defined Blum generators
 *
 * These generators are seeded via the srandom(0, newn) where
 * 1 <= newn < BLUM_PREGEN.
 */
#if FULL_BITS == 64
STATIC CONST HALF h_nvec01[] = {
	(HALF)0x83de9361, (HALF)0xf0db722d, (HALF)0x6fe328ca, (HALF)0x04944073,
	(HALF)0x5
};
STATIC CONST HALF h_rvec01[] = {
	(HALF)0xa4cc42ec, (HALF)0x4e5dbb01, (HALF)0x11d952e7, (HALF)0xb226980f
};
STATIC CONST HALF h_nvec02[] = {
	(HALF)0x353443f1, (HALF)0xeb286ea9, (HALF)0xdd374a18, (HALF)0x348a2555,
	(HALF)0x2c5
};
STATIC CONST HALF h_rvec02[] = {
	(HALF)0x21e3a218, (HALF)0xe893616b, (HALF)0x6cd710e3, (HALF)0xf3d64344,
	(HALF)0x40
};
STATIC CONST HALF h_nvec03[] = {
	(HALF)0x11d001f1, (HALF)0xf2ca661f, (HALF)0x3a81f1e0, (HALF)0x59d6ce4e,
	(HALF)0x0009cfd9
};
STATIC CONST HALF h_rvec03[] = {
	(HALF)0xa0d7d76a, (HALF)0x3e142de2, (HALF)0xff5cea4f, (HALF)0xb44d9b64,
	(HALF)0xfae5
};
STATIC CONST HALF h_nvec04[] = {
	(HALF)0xdfcc0751, (HALF)0x2decc680, (HALF)0x5df12a1a, (HALF)0x5c894ed7,
	(HALF)0x3070f924
};
STATIC CONST HALF h_rvec04[] = {
	(HALF)0x4b984570, (HALF)0xa220ddba, (HALF)0xa2c0af8a, (HALF)0x131b2bdc,
	(HALF)0x0020c2d8
};
STATIC CONST HALF h_nvec05[] = {
	(HALF)0x99166ef1, (HALF)0x8b99e5e7, (HALF)0x8769a010, (HALF)0x5d3fe111,
	(HALF)0x680bc2fa, (HALF)0x38f75aac, (HALF)0xdb81a85b, (HALF)0x109b1822,
	(HALF)0x2
};
STATIC CONST HALF h_rvec05[] = {
	(HALF)0x59e2efa9, (HALF)0x0e6c77c8, (HALF)0x1e70aeed, (HALF)0x234f7b7d,
	(HALF)0x5f5df6db, (HALF)0xe821a960, (HALF)0xae33b792, (HALF)0x5e9b890e
};
STATIC CONST HALF h_nvec06[] = {
	(HALF)0xe1ddf431, (HALF)0xd85557f1, (HALF)0x5ee732da, (HALF)0x3a38db77,
	(HALF)0x5c644026, (HALF)0xf2dbf218, (HALF)0x9ada2c79, (HALF)0x7bfd9d7d,
	(HALF)0xa
};
STATIC CONST HALF h_rvec06[] = {
	(HALF)0xc9404daf, (HALF)0xc5dc2e80, (HALF)0x2c98eccf, (HALF)0xe1f3495d,
	(HALF)0xce1c925c, (HALF)0xe097aede, (HALF)0x88667154, (HALF)0x5e94a02f
};
STATIC CONST HALF h_nvec07[] = {
	(HALF)0xcf9ec751, (HALF)0x602f9125, (HALF)0x52882e7f, (HALF)0x0dcf53ce,
	(HALF)0xff569d6b, (HALF)0x628643fc, (HALF)0x37801cd5, (HALF)0xf2399ef2,
	(HALF)0x43d87de8
};
STATIC CONST HALF h_rvec07[] = {
	(HALF)0x098d25e6, (HALF)0x3992d2e5, (HALF)0x64f0b58c, (HALF)0xcf18d4dd,
	(HALF)0x9d876aef, (HALF)0x7acced04, (HALF)0xbfbe9076, (HALF)0x1ee014c7,
	(HALF)0x0013522d
};
STATIC CONST HALF h_nvec08[] = {
	(HALF)0x26742f11, (HALF)0xbc42e66a, (HALF)0xb59cd9f0, (HALF)0x9ad4a6c2,
	(HALF)0x5bdbd2f9, (HALF)0xbdc91fed, (HALF)0xf13c9ce7, (HALF)0xeb4699b7,
	(HALF)0x47126ca7, (HALF)0x58
};
STATIC CONST HALF h_rvec08[] = {
	(HALF)0x489dc674, (HALF)0xaae95f3a, (HALF)0xa35da929, (HALF)0x5597b4b8,
	(HALF)0x28e9c947, (HALF)0x3d344f9a, (HALF)0xb7e661fa, (HALF)0xa3269116,
	(HALF)0x853016dc
};
STATIC CONST HALF h_nvec09[] = {
	(HALF)0xab27e3d1, (HALF)0x12745db4, (HALF)0xb980f951, (HALF)0x62b16b66,
	(HALF)0x0fdece0d, (HALF)0x6061c6fd, (HALF)0x36a6ff09, (HALF)0xe08eb61c,
	(HALF)0x84d895c3, (HALF)0x4a86752a, (HALF)0xc1797b4f, (HALF)0x562157a3,
	(HALF)0x3d267bb0, (HALF)0x14e81b00, (HALF)0x218d9238, (HALF)0x52322fd3,
	(HALF)0x0039e8be
};
STATIC CONST HALF h_rvec09[] = {
	(HALF)0x7d4ed20d, (HALF)0x601ef2b8, (HALF)0x8e59f959, (HALF)0xedaa5d9e,
	(HALF)0x309a89ba, (HALF)0xe5ab7d81, (HALF)0x796b2545, (HALF)0x02de3222,
	(HALF)0x8357c0bd, (HALF)0x0107e3fd, (HALF)0x82d9d288, (HALF)0xbc42a8aa,
	(HALF)0x4b787343, (HALF)0xc0150886, (HALF)0xbab915bf, (HALF)0xf8ad1e6b,
	(HALF)0xb458
};
STATIC CONST HALF h_nvec10[] = {
	(HALF)0xb7e64b89, (HALF)0xc3cdc363, (HALF)0x2ef9c73c, (HALF)0x6092ce22,
	(HALF)0x02abe36c, (HALF)0x08d49573, (HALF)0x74511c40, (HALF)0xd38582de,
	(HALF)0xa524a02f, (HALF)0x52c81b3b, (HALF)0x250d3cc9, (HALF)0x23b50e88,
	(HALF)0xbd1448bf, (HALF)0x882d7f98, (HALF)0xc23ef596, (HALF)0xc9055666,
	(HALF)0x025f2435
};
STATIC CONST HALF h_rvec10[] = {
	(HALF)0x94cfc482, (HALF)0x594f5ad4, (HALF)0x23442aee, (HALF)0x145f40ce,
	(HALF)0x1baf950d, (HALF)0xadc4f175, (HALF)0xf62c669f, (HALF)0x8d075d56,
	(HALF)0x08ed8b40, (HALF)0xaaf2cf30, (HALF)0xc24b5ffb, (HALF)0x250df8cf,
	(HALF)0x7ca81ec9, (HALF)0x787e2b70, (HALF)0x18401468, (HALF)0x47b20e0c,
	(HALF)0x0066bb7e
};
STATIC CONST HALF h_nvec11[] = {
	(HALF)0x546ee069, (HALF)0x2e1a530c, (HALF)0x2014dab2, (HALF)0xa729cf52,
	(HALF)0x920ee1a9, (HALF)0x68f27533, (HALF)0x25873cfa, (HALF)0xdd37a749,
	(HALF)0x4499daa2, (HALF)0x286e5870, (HALF)0x57f3f9b6, (HALF)0x5ec54467,
	(HALF)0x69a791ea, (HALF)0x874ecd77, (HALF)0x4217d56b, (HALF)0x82bdb309,
	(HALF)0x497864de
};
STATIC CONST HALF h_rvec11[] = {
	(HALF)0x56e38b04, (HALF)0x3a0aded3, (HALF)0x461d88b1, (HALF)0x9c094d65,
	(HALF)0xe5333fed, (HALF)0x34d918fe, (HALF)0x1ef56281, (HALF)0xcedfa07c,
	(HALF)0x590f47fb, (HALF)0xa2c54d5c, (HALF)0x732339ee, (HALF)0x806549a7,
	(HALF)0x9ce3163f, (HALF)0xae3af8b6, (HALF)0x264a4465, (HALF)0x1cb5e630,
	(HALF)0x00868488
};
STATIC CONST HALF h_nvec12[] = {
	(HALF)0xf14c7b99, (HALF)0x7f66d151, (HALF)0x87efad2b, (HALF)0x57d3f098,
	(HALF)0xd6534165, (HALF)0x812fdd25, (HALF)0x48c7c7ce, (HALF)0xa1bf41e0,
	(HALF)0x4c94e315, (HALF)0x190b1593, (HALF)0xee4251da, (HALF)0x2b4a1a66,
	(HALF)0x2bb7c2a1, (HALF)0x65b18ca9, (HALF)0x08b89116, (HALF)0xc0ccb15f,
	(HALF)0x57582ab3, (HALF)0x34
};
STATIC CONST HALF h_rvec12[] = {
	(HALF)0xe207b4a0, (HALF)0x5227dd68, (HALF)0x9488fbc4, (HALF)0x6ed081aa,
	(HALF)0x8e736fe5, (HALF)0x3dd2c020, (HALF)0xeeb07c57, (HALF)0x0b604eb7,
	(HALF)0xa13f72a1, (HALF)0xcbfc4333, (HALF)0xa4c5e8cd, (HALF)0x0add520d,
	(HALF)0x758ca3b2, (HALF)0x55490137, (HALF)0x5870babd, (HALF)0xf648ed93,
	(HALF)0xdf719bd1
};
STATIC CONST HALF h_nvec13[] = {
	(HALF)0x3314fc49, (HALF)0xcca20032, (HALF)0x208e3420, (HALF)0x8aaa503a,
	(HALF)0xd79a63cc, (HALF)0xb4ed7417, (HALF)0x95dd1892, (HALF)0xb5915f64,
	(HALF)0xd14cc7f1, (HALF)0x1589917e, (HALF)0xb2b05667, (HALF)0xc32d99cb,
	(HALF)0x1b5a1a84, (HALF)0xa49322a1, (HALF)0xdd3a76c3, (HALF)0x8c07137f,
	(HALF)0xaaf83c63, (HALF)0x37575113, (HALF)0xa8b18e84, (HALF)0xceec891d,
	(HALF)0x78c1ee99, (HALF)0x6e49e256, (HALF)0x4286bfd6, (HALF)0xcb6bf6a9,
	(HALF)0x7bda8ee0, (HALF)0xd439510a, (HALF)0x63f4345b, (HALF)0x959a5535,
	(HALF)0xdaf66d82, (HALF)0xed03d833, (HALF)0x1b5af734, (HALF)0x166b7dd2,
	(HALF)0x01517c19
};
STATIC CONST HALF h_rvec13[] = {
	(HALF)0x6b7736f5, (HALF)0x2407bfe4, (HALF)0x965e2072, (HALF)0xcc26cf3e,
	(HALF)0xa432b567, (HALF)0x2ed007ab, (HALF)0x0e2f67b9, (HALF)0xef640960,
	(HALF)0xbe5f1ad3, (HALF)0x3faeda1b, (HALF)0xa8f6b988, (HALF)0xe5c9cea5,
	(HALF)0xa67445ea, (HALF)0x3935ce78, (HALF)0xf445ff06, (HALF)0xbeda0a11,
	(HALF)0x92de080b, (HALF)0xf4049026, (HALF)0xe509f0b8, (HALF)0x6f05216f,
	(HALF)0x5a68dc14, (HALF)0x548f730d, (HALF)0xe9fd9f00, (HALF)0x64a4aada,
	(HALF)0xe3bbdd15, (HALF)0xcb4be7a5, (HALF)0x17ddd162, (HALF)0xc4918c33,
	(HALF)0x9b706d2b, (HALF)0x8b04f6a6, (HALF)0x1263fa64, (HALF)0x8e9a560d,
	(HALF)0xd42e
};
STATIC CONST HALF h_nvec14[] = {
	(HALF)0xc116af01, (HALF)0xbdef8c0f, (HALF)0xc4409a1a, (HALF)0xacb3185c,
	(HALF)0xb33f925b, (HALF)0xfee83005, (HALF)0x4b3db112, (HALF)0x7f076743,
	(HALF)0x21709223, (HALF)0x2159054b, (HALF)0x6fdfefe3, (HALF)0x792d0a07,
	(HALF)0x1d14bd52, (HALF)0x5f289a27, (HALF)0xe03483c4, (HALF)0xcb86a0c1,
	(HALF)0x0ede912a, (HALF)0xf01a33e3, (HALF)0x63fd40c6, (HALF)0x4b0586e4,
	(HALF)0xbc7e4a03, (HALF)0x956b22f3, (HALF)0x10560b22, (HALF)0x3e78d321,
	(HALF)0x179161e2, (HALF)0xe85eb909, (HALF)0xec7931ee, (HALF)0xb314b4bf,
	(HALF)0x1f564618, (HALF)0xb9d983d0, (HALF)0x7479ac07, (HALF)0x93c6f4e8,
	(HALF)0x5e56a00e
};
STATIC CONST HALF h_rvec14[] = {
	(HALF)0x0ff9f190, (HALF)0x47a4db68, (HALF)0x913cc8ea, (HALF)0xb6b1b220,
	(HALF)0x13edfbbb, (HALF)0xa8f1f1c3, (HALF)0xd6d71f8f, (HALF)0x4194649a,
	(HALF)0x7d497344, (HALF)0x677c8416, (HALF)0x0186b983, (HALF)0xee633901,
	(HALF)0xce64d69d, (HALF)0x61a704b3, (HALF)0x138352b2, (HALF)0xc0fb58d8,
	(HALF)0x16bf2073, (HALF)0x56c9ae78, (HALF)0xb81a0a68, (HALF)0x1512abaf,
	(HALF)0x6b9936ba, (HALF)0x43350dfc, (HALF)0xa0ea19d2, (HALF)0xe86134c5,
	(HALF)0x6c86563f, (HALF)0x6b0c5b68, (HALF)0x75f627fc, (HALF)0xb609913f,
	(HALF)0x15b9a564, (HALF)0x9b18f154, (HALF)0x6ef0c5d0, (HALF)0xb6733509,
	(HALF)0x00f7aa7c
};
STATIC CONST HALF h_nvec15[] = {
	(HALF)0xc8d97079, (HALF)0x061e7597, (HALF)0xf5d2c721, (HALF)0x299bc51f,
	(HALF)0xffe6c337, (HALF)0x19798624, (HALF)0xee6f92b6, (HALF)0x0b1d0c7a,
	(HALF)0xb5308231, (HALF)0x49c558dd, (HALF)0x196a530e, (HALF)0x0caa515c,
	(HALF)0x8b0d86ed, (HALF)0x380a8fa0, (HALF)0x80df03e4, (HALF)0xe962d81b,
	(HALF)0xc1a783b3, (HALF)0xc3278ccc, (HALF)0x3e72ab86, (HALF)0xebb91675,
	(HALF)0xb902c2b8, (HALF)0xa0590445, (HALF)0x4f40de42, (HALF)0xaa95aac1,
	(HALF)0xfc0f2e82, (HALF)0x70cab84b, (HALF)0x5326a267, (HALF)0x470be607,
	(HALF)0x45352ebe, (HALF)0x5ba81ca8, (HALF)0x02c46c17, (HALF)0x9edfbcdb,
	(HALF)0x97dd840b
};
STATIC CONST HALF h_rvec15[] = {
	(HALF)0x6c3e2110, (HALF)0x808f0aaa, (HALF)0xd98db92e, (HALF)0x1e6abd43,
	(HALF)0xf401b920, (HALF)0x9d3f0381, (HALF)0xdb95d174, (HALF)0xa2f65c33,
	(HALF)0x0c5469f8, (HALF)0xa3c126fc, (HALF)0x8866241b, (HALF)0x0e46eca7,
	(HALF)0xa70557fa, (HALF)0x8e7391a6, (HALF)0x70e4a9e2, (HALF)0x9ad97e89,
	(HALF)0x1eb77dee, (HALF)0xfe6224d0, (HALF)0xdbe522b1, (HALF)0xa4aa1023,
	(HALF)0x7f226860, (HALF)0x60ef4379, (HALF)0x45e3b296, (HALF)0xcc31f5ef,
	(HALF)0x74fbf6d6, (HALF)0x55b1c25d, (HALF)0x3ede521f, (HALF)0xdf8cef42,
	(HALF)0xa8d77ca6, (HALF)0xb50025da, (HALF)0x69ab99f9, (HALF)0x03b8c758,
	(HALF)0x00b82207
};
STATIC CONST HALF h_nvec16[] = {
	(HALF)0xd84346e1, (HALF)0x184183f6, (HALF)0x2dc9bd36, (HALF)0x4ca857ac,
	(HALF)0x96a5828d, (HALF)0xed1f1c59, (HALF)0x36d9731f, (HALF)0xbd3f6183,
	(HALF)0xde0f5578, (HALF)0xb6a2ea8a, (HALF)0xbe993c44, (HALF)0x0e283c05,
	(HALF)0xd7cf61e3, (HALF)0x40fe15b6, (HALF)0x534d967e, (HALF)0x34691046,
	(HALF)0xd40845bd, (HALF)0xd69cee4b, (HALF)0x557fee8d, (HALF)0x51c856ba,
	(HALF)0xe6bc51ba, (HALF)0x587bc173, (HALF)0x1959e379, (HALF)0x92828439,
	(HALF)0x311e0503, (HALF)0x7f3c9cc2, (HALF)0xd426512c, (HALF)0xe8b2b497,
	(HALF)0x9e43536a, (HALF)0x9f544cb8, (HALF)0xb56f84c3, (HALF)0xb82fbb12,
	(HALF)0x6e348549, (HALF)0x45
};
STATIC CONST HALF h_rvec16[] = {
	(HALF)0x7b4e8830, (HALF)0x70605db8, (HALF)0xa1abe4a5, (HALF)0xa70fbe04,
	(HALF)0x2bcda8f4, (HALF)0xd29ada9a, (HALF)0x55ad0560, (HALF)0xb367137e,
	(HALF)0xd6972f1a, (HALF)0x809bad45, (HALF)0xb15d2454, (HALF)0x0c0d415f,
	(HALF)0x416e117a, (HALF)0xe87a9521, (HALF)0x670a5e1a, (HALF)0x53a41772,
	(HALF)0xfc9c5cc1, (HALF)0xf75645df, (HALF)0x86f6d19f, (HALF)0x9404b5bf,
	(HALF)0x56e9d83b, (HALF)0xac0f3bc3, (HALF)0xa1508c4b, (HALF)0x4bfd4977,
	(HALF)0x71922540, (HALF)0xf2524def, (HALF)0x8a81a3db, (HALF)0xced828de,
	(HALF)0x7895ae8f, (HALF)0x4a4b6dcd, (HALF)0x973e921a, (HALF)0x9fb27a07,
	(HALF)0xb0d7dcb1
};
STATIC CONST HALF h_nvec17[] = {
	(HALF)0x72b72051, (HALF)0xedc24ebf, (HALF)0xe970a8d1, (HALF)0x66c9b150,
	(HALF)0xcbb927f7, (HALF)0xb574ffd9, (HALF)0x4166b249, (HALF)0x0fce4030,
	(HALF)0xfa6922ca, (HALF)0x39cc14a9, (HALF)0x14396e2a, (HALF)0xaff74c7f,
	(HALF)0xa120a314, (HALF)0xe11a2700, (HALF)0x44c9ad30, (HALF)0x7f328d72,
	(HALF)0xab2ceaf7, (HALF)0x868ff772, (HALF)0x0974f0b3, (HALF)0x20e9f0f6,
	(HALF)0xcd4e5b8a, (HALF)0x0bde26e6, (HALF)0x96f6c3ac, (HALF)0x5718c601,
	(HALF)0x2f117710, (HALF)0x1ab0876e, (HALF)0x49ab2c2e, (HALF)0x747022b9,
	(HALF)0x6e9de4a7, (HALF)0x25e88f2e, (HALF)0xd7d0a00b, (HALF)0xc8ff11f6,
	(HALF)0xf50aa819, (HALF)0xbe530e9e, (HALF)0x47b7ff54, (HALF)0xe0209b46,
	(HALF)0x027ec5eb, (HALF)0x07543362, (HALF)0x531e9b85, (HALF)0x3c23b568,
	(HALF)0x5d07af7a, (HALF)0xd4948461, (HALF)0x2eb9a499, (HALF)0x1c71fa0b,
	(HALF)0x7025e22b, (HALF)0x4d2720b9, (HALF)0x531b8d54, (HALF)0x66e2fc30,
	(HALF)0xdac7cafd, (HALF)0x7f29953b, (HALF)0x9d456bff, (HALF)0x839814bc,
	(HALF)0xd0e5fb19, (HALF)0x5a6c9f58, (HALF)0x3a5a9dc3, (HALF)0x598bf28b,
	(HALF)0xa7a91144, (HALF)0x68494f76, (HALF)0xbfedd7be, (HALF)0x7ca54266,
	(HALF)0x96e0faf9, (HALF)0x33be3c0f, (HALF)0xffa3040b, (HALF)0x813aeac0,
	(HALF)0x6177
};
STATIC CONST HALF h_rvec17[] = {
	(HALF)0x22b41dac, (HALF)0xd6258005, (HALF)0x2aa1e0cb, (HALF)0x45d147b5,
	(HALF)0xbf5c46d9, (HALF)0x14c9dadf, (HALF)0x09b0aec4, (HALF)0x4286bfef,
	(HALF)0xc6f8e9d1, (HALF)0xdd68467b, (HALF)0x93f4ffb9, (HALF)0x58f2eb51,
	(HALF)0x2ade048f, (HALF)0xeacae6e5, (HALF)0x8dd2a807, (HALF)0xbcea8c27,
	(HALF)0x02a03281, (HALF)0x039aeb6d, (HALF)0xfa6e016c, (HALF)0x6fda1b09,
	(HALF)0xea7719ed, (HALF)0xcf2e0294, (HALF)0xa4264cb9, (HALF)0x49888af1,
	(HALF)0x1b44f0c1, (HALF)0x3ccee577, (HALF)0xdbebd170, (HALF)0xf7431e7e,
	(HALF)0xfaebd584, (HALF)0x896e4a33, (HALF)0xe46bf43c, (HALF)0x17fe9a10,
	(HALF)0xb5321b51, (HALF)0xf7e1d2a9, (HALF)0xe7fe0a56, (HALF)0xbd736750,
	(HALF)0xcf029a33, (HALF)0xebee99b1, (HALF)0x810fff31, (HALF)0x694c8d30,
	(HALF)0xc8c18689, (HALF)0xc2f9f4fb, (HALF)0x5949fd7f, (HALF)0x67aaf7b3,
	(HALF)0xa82f906a, (HALF)0x1b84b7b3, (HALF)0xeac052ee, (HALF)0x1a4e9345,
	(HALF)0xce3c2973, (HALF)0x4a5168a7, (HALF)0x5c5551ba, (HALF)0x77c6cb26,
	(HALF)0xfa45a3a6, (HALF)0x486f31e0, (HALF)0xcaf97519, (HALF)0xbe4b0399,
	(HALF)0x802fc106, (HALF)0x537284da, (HALF)0x20c4e167, (HALF)0x2a62f329,
	(HALF)0xc2d2fc5b, (HALF)0xdd665324, (HALF)0xc3b8adf1, (HALF)0x0b6eaf3b,
	(HALF)0x5372
};
STATIC CONST HALF h_nvec18[] = {
	(HALF)0xc8b78629, (HALF)0x41351b18, (HALF)0x28ad4ed8, (HALF)0xc96f7df1,
	(HALF)0x7cd3c931, (HALF)0x0f23036a, (HALF)0xac657631, (HALF)0x6a625812,
	(HALF)0x08144788, (HALF)0x8642ed62, (HALF)0x76198a40, (HALF)0x70defd64,
	(HALF)0x97fb673c, (HALF)0x6f3bddf6, (HALF)0x72fe2977, (HALF)0x20ed82f1,
	(HALF)0x7e7f4fdc, (HALF)0xdc272d6b, (HALF)0x6d77f317, (HALF)0x2c595b15,
	(HALF)0x1c3b7dd6, (HALF)0x6e3d6147, (HALF)0x8170640c, (HALF)0xb660033f,
	(HALF)0xf211886b, (HALF)0xbcc67859, (HALF)0x18ff4b93, (HALF)0x3068e691,
	(HALF)0x9db5d823, (HALF)0x72afd4ef, (HALF)0x5aa4cd1a, (HALF)0xa0a33014,
	(HALF)0x6b8349e6, (HALF)0x2f4de595, (HALF)0xd5180384, (HALF)0x19d94118,
	(HALF)0x369e7534, (HALF)0xce4d3b18, (HALF)0x119ef9ee, (HALF)0xe2f45c25,
	(HALF)0xe8eae0a7, (HALF)0x62e41605, (HALF)0x6346d2ca, (HALF)0x6425625b,
	(HALF)0x44b033de, (HALF)0x1711e6b3, (HALF)0xf3a9d02e, (HALF)0x259cd965,
	(HALF)0x08aa956b, (HALF)0x6ad64380, (HALF)0xe9730e8e, (HALF)0x539b9d28,
	(HALF)0xe5407950, (HALF)0x89900be4, (HALF)0xde1218f6, (HALF)0x63e1e52b,
	(HALF)0x0de03f4e, (HALF)0x8e21a568, (HALF)0x268d7ee3, (HALF)0xafb3514e,
	(HALF)0x5378efcb, (HALF)0xfec0c7c6, (HALF)0xf07cb724, (HALF)0xfb61b42a,
	(HALF)0x068f2a38
};
STATIC CONST HALF h_rvec18[] = {
	(HALF)0x35ea3c63, (HALF)0x8df2ef97, (HALF)0xa2b3afb7, (HALF)0x179158f6,
	(HALF)0x04920dba, (HALF)0xf333077e, (HALF)0xf8304b5a, (HALF)0x230ff2ae,
	(HALF)0x84a8f3f0, (HALF)0xadda164e, (HALF)0xc9a1c944, (HALF)0xc70502f2,
	(HALF)0x41a3c18f, (HALF)0x09bd3254, (HALF)0x973665a9, (HALF)0x1548c263,
	(HALF)0x5024d916, (HALF)0x0a3ddde9, (HALF)0xf2aaf1f5, (HALF)0x666db92a,
	(HALF)0x3a535aa5, (HALF)0x49c35775, (HALF)0xc381a1c4, (HALF)0xf8d36dbc,
	(HALF)0xe94be870, (HALF)0x430e88a6, (HALF)0x37219a06, (HALF)0x5109df80,
	(HALF)0xe73ca03f, (HALF)0xf1bb4541, (HALF)0x3c6f32f3, (HALF)0x952cfc24,
	(HALF)0xfbc9697f, (HALF)0xc5b8d472, (HALF)0xcbbeda4b, (HALF)0x7303db3b,
	(HALF)0xa18f255e, (HALF)0xe1d3353d, (HALF)0xe8c98700, (HALF)0x9e75e8fd,
	(HALF)0x3ddd812e, (HALF)0x7340f891, (HALF)0xb1f369ac, (HALF)0x764d505b,
	(HALF)0xb13ef51b, (HALF)0x16c3aa43, (HALF)0x61d042c4, (HALF)0x22ac0339,
	(HALF)0x3f306fd1, (HALF)0x49926f7f, (HALF)0x2b4c575c, (HALF)0x9f3ab467,
	(HALF)0x5dac65af, (HALF)0x62778dc7, (HALF)0x99113a89, (HALF)0x49c0540a,
	(HALF)0x1df70ac2, (HALF)0x4be12c5e, (HALF)0xe5d36bdb, (HALF)0x66b99ff5,
	(HALF)0x5358be89, (HALF)0x4cd835d7, (HALF)0xf0d5cda8, (HALF)0x1f1ac6c3,
	(HALF)0x04735e92
};
STATIC CONST HALF h_nvec19[] = {
	(HALF)0x6b659a79, (HALF)0x0239c12d, (HALF)0xd204df49, (HALF)0x1d4ae0c7,
	(HALF)0x099bf000, (HALF)0x6435ade8, (HALF)0xdc4af029, (HALF)0x2f4ee7a2,
	(HALF)0xadfcf1e3, (HALF)0x73358f43, (HALF)0x687eede5, (HALF)0xb567cd4d,
	(HALF)0xc7a7814f, (HALF)0xc306624c, (HALF)0xa82d80c6, (HALF)0x3f390cd5,
	(HALF)0x7b7dec3f, (HALF)0x8bdb1416, (HALF)0x275b3a52, (HALF)0x921884fe,
	(HALF)0x94ac02e0, (HALF)0x62f2b52c, (HALF)0xcdc992ee, (HALF)0x35e55eeb,
	(HALF)0x69a43fd5, (HALF)0x44c1dcfb, (HALF)0x3cdf6227, (HALF)0x23f3148f,
	(HALF)0x42508e4c, (HALF)0x95b737c3, (HALF)0x70af831f, (HALF)0x2ee815c9,
	(HALF)0x47a47251, (HALF)0x7c11b2af, (HALF)0x664c361f, (HALF)0xcadaa841,
	(HALF)0x3d97172a, (HALF)0x87402ffb, (HALF)0xa0a02ebd, (HALF)0xb674225f,
	(HALF)0x65593fe2, (HALF)0x85f698b9, (HALF)0x5ed9a7ab, (HALF)0x6bf37371,
	(HALF)0x7da75305, (HALF)0x088255bf, (HALF)0x8f607684, (HALF)0x1f3de57e,
	(HALF)0x118bd501, (HALF)0x6833770d, (HALF)0xd0425c51, (HALF)0x2664cacb,
	(HALF)0x9206b920, (HALF)0xeb903b8d, (HALF)0x0e2516b4, (HALF)0x36c3d841,
	(HALF)0x51d7cd17, (HALF)0xe063ba1e, (HALF)0xf28f4a6d, (HALF)0x244deb0d,
	(HALF)0x2e410ad4, (HALF)0x0721c315, (HALF)0xdde27654, (HALF)0x2ad6534b,
	(HALF)0xd6788b25, (HALF)0xb23bb9e8, (HALF)0x00230d7a
};
STATIC CONST HALF h_rvec19[] = {
	(HALF)0x698ef473, (HALF)0x3d53a5b7, (HALF)0x06448319, (HALF)0xd9ad4445,
	(HALF)0x6967daa0, (HALF)0xa14c6240, (HALF)0x78e77724, (HALF)0x63ef2ab7,
	(HALF)0x8dff2ee2, (HALF)0x662eb424, (HALF)0xcd9307d6, (HALF)0x0ab06a5d,
	(HALF)0xbdd7b539, (HALF)0x8621dd2d, (HALF)0x2bb4a187, (HALF)0x1f1e121d,
	(HALF)0xce8db962, (HALF)0xdd3eeaf1, (HALF)0x573b6ca1, (HALF)0x6f460cd6,
	(HALF)0x6a8d4780, (HALF)0xea68c7e6, (HALF)0x1148eb32, (HALF)0xb43d44d4,
	(HALF)0xb657cb64, (HALF)0x8547fdba, (HALF)0x85f73333, (HALF)0xc1f2a51a,
	(HALF)0x1c05ee52, (HALF)0x2847c03d, (HALF)0xfc7a88d8, (HALF)0x2e8dd186,
	(HALF)0x5be34683, (HALF)0xd43d2ee2, (HALF)0x9f2d2bb5, (HALF)0x89b3ddea,
	(HALF)0x7d784ae0, (HALF)0xc7735e28, (HALF)0x967a8608, (HALF)0xcdcbcf07,
	(HALF)0xfc17a423, (HALF)0xd36ad053, (HALF)0xc73d8892, (HALF)0xa635c3f4,
	(HALF)0x9b5d0cf9, (HALF)0x0ac73fd9, (HALF)0xe801fefb, (HALF)0xb31cffd2,
	(HALF)0xf3eaaa55, (HALF)0x0e74fa23, (HALF)0x5414b290, (HALF)0x6d176101,
	(HALF)0x522993f8, (HALF)0xf8293dad, (HALF)0x713b1e82, (HALF)0xb83bbef0,
	(HALF)0xd001cc62, (HALF)0x7537da39, (HALF)0x7d80158b, (HALF)0x9332c8e2,
	(HALF)0x6fa6fa75, (HALF)0xe21f6512, (HALF)0x999518b4, (HALF)0x2196605b,
	(HALF)0xe4fc1798, (HALF)0x5f21e245, (HALF)0x0008f172
};
STATIC CONST HALF h_nvec20[] = {
	(HALF)0xc3c1d081, (HALF)0x4d262fce, (HALF)0x8765cc91, (HALF)0xf3727f7c,
	(HALF)0xabba4bbc, (HALF)0xe0985801, (HALF)0xfa365c51, (HALF)0xb2a4b230,
	(HALF)0xf4430a8d, (HALF)0x546b98c8, (HALF)0xd9748b26, (HALF)0xe255a82f,
	(HALF)0xb00a3e8c, (HALF)0x7069676d, (HALF)0x6233ccce, (HALF)0x0299a74e,
	(HALF)0xd119adf9, (HALF)0x5273e811, (HALF)0xae36e0bb, (HALF)0x32b3a486,
	(HALF)0xf04921a5, (HALF)0xd31d28a2, (HALF)0xda0c50de, (HALF)0x21302b40,
	(HALF)0xee9de552, (HALF)0x80bbac03, (HALF)0x75f49740, (HALF)0xc3d0a61b,
	(HALF)0x341cfbf6, (HALF)0x1615b5b4, (HALF)0x4e3a3def, (HALF)0x95734dbf,
	(HALF)0xe7ab78ac, (HALF)0xffdc5cf9, (HALF)0xf7996892, (HALF)0x47407ba4,
	(HALF)0xa78b988f, (HALF)0x27736f42, (HALF)0x5064ee50, (HALF)0x60135c56,
	(HALF)0x1ad73283, (HALF)0x25624bf7, (HALF)0x2ee21419, (HALF)0x93195abd,
	(HALF)0x66b67778, (HALF)0xb1e0a42d, (HALF)0x729fb0f0, (HALF)0xd4921864,
	(HALF)0x2c42253f, (HALF)0x302a07a9, (HALF)0xbb741bd4, (HALF)0x932f90ba,
	(HALF)0xf3354be1, (HALF)0x0804d661, (HALF)0x010e9ba1, (HALF)0x1a05778d,
	(HALF)0xa962c833, (HALF)0xe7590ee8, (HALF)0xbe6803b8, (HALF)0xc67704c1,
	(HALF)0x56d79660, (HALF)0x6066a3f3, (HALF)0x648b0327, (HALF)0x267e5b3a,
	(HALF)0xdddc63a0, (HALF)0x3322e890, (HALF)0x20e0d8b1, (HALF)0x004fd2b8
};
STATIC CONST HALF h_rvec20[] = {
	(HALF)0xa048bd1a, (HALF)0x95abdc7b, (HALF)0x98f47cf8, (HALF)0x126ac98d,
	(HALF)0xaebf85fd, (HALF)0x5650580f, (HALF)0x3292d7dd, (HALF)0xf49e8377,
	(HALF)0x2947ed46, (HALF)0xd1a5b26c, (HALF)0xae14e6a1, (HALF)0x9b1f5788,
	(HALF)0x4df727b2, (HALF)0xee375079, (HALF)0x131bc8e4, (HALF)0x294e5f53,
	(HALF)0x1f5759bd, (HALF)0x65d58acf, (HALF)0x598ed3a5, (HALF)0xc39361a6,
	(HALF)0xa783fd7a, (HALF)0x264a36a2, (HALF)0x6ca22856, (HALF)0x8ffb171f,
	(HALF)0x1d7cea9e, (HALF)0xd81d6fca, (HALF)0x34ea730e, (HALF)0x31f56382,
	(HALF)0xb39cd9e9, (HALF)0x440e84be, (HALF)0x4b1d15a1, (HALF)0x7bf775c5,
	(HALF)0xe40f4638, (HALF)0xe5bef0a7, (HALF)0x79e58942, (HALF)0x881ae1ba,
	(HALF)0x01de8372, (HALF)0x14cf35f8, (HALF)0xe2d8b310, (HALF)0x66961207,
	(HALF)0xde5d5f91, (HALF)0xe6e70849, (HALF)0x74ec5ac3, (HALF)0xe2de4eb1,
	(HALF)0x4a41dc20, (HALF)0xd306d565, (HALF)0xb5843ff3, (HALF)0x911b30d6,
	(HALF)0x4e9cd926, (HALF)0x8455c9ae, (HALF)0x69448bb5, (HALF)0x0c7b1aad,
	(HALF)0x1da1e992, (HALF)0xc67656bd, (HALF)0xc544209e, (HALF)0x10ce387c,
	(HALF)0xc4e88df8, (HALF)0x40e8da88, (HALF)0xbb2c3028, (HALF)0x49194fd9,
	(HALF)0xdeef17ee, (HALF)0x241bc08d, (HALF)0x6fa9f608, (HALF)0x4b0f8b04,
	(HALF)0xee960da1, (HALF)0xa3099293, (HALF)0x84445fea, (HALF)0x0046ef01
};
#elif 2*FULL_BITS == 64
STATIC CONST HALF h_nvec01[] = {
	(HALF)0x9361, (HALF)0x83de, (HALF)0x722d, (HALF)0xf0db,
	(HALF)0x28ca, (HALF)0x6fe3, (HALF)0x4073, (HALF)0x0494,
	(HALF)0x5
};
STATIC CONST HALF h_rvec01[] = {
	(HALF)0x42ec, (HALF)0xa4cc, (HALF)0xbb01, (HALF)0x4e5d,
	(HALF)0x52e7, (HALF)0x11d9, (HALF)0x980f, (HALF)0xb226
};
STATIC CONST HALF h_nvec02[] = {
	(HALF)0x43f1, (HALF)0x3534, (HALF)0x6ea9, (HALF)0xeb28,
	(HALF)0x4a18, (HALF)0xdd37, (HALF)0x2555, (HALF)0x348a,
	(HALF)0x2c5
};
STATIC CONST HALF h_rvec02[] = {
	(HALF)0xa218, (HALF)0x21e3, (HALF)0x616b, (HALF)0xe893,
	(HALF)0x10e3, (HALF)0x6cd7, (HALF)0x4344, (HALF)0xf3d6,
	(HALF)0x40
};
STATIC CONST HALF h_nvec03[] = {
	(HALF)0x01f1, (HALF)0x11d0, (HALF)0x661f, (HALF)0xf2ca,
	(HALF)0xf1e0, (HALF)0x3a81, (HALF)0xce4e, (HALF)0x59d6,
	(HALF)0xcfd9, (HALF)0x0009
};
STATIC CONST HALF h_rvec03[] = {
	(HALF)0xd76a, (HALF)0xa0d7, (HALF)0x2de2, (HALF)0x3e14,
	(HALF)0xea4f, (HALF)0xff5c, (HALF)0x9b64, (HALF)0xb44d,
	(HALF)0xfae5
};
STATIC CONST HALF h_nvec04[] = {
	(HALF)0x0751, (HALF)0xdfcc, (HALF)0xc680, (HALF)0x2dec,
	(HALF)0x2a1a, (HALF)0x5df1, (HALF)0x4ed7, (HALF)0x5c89,
	(HALF)0xf924, (HALF)0x3070
};
STATIC CONST HALF h_rvec04[] = {
	(HALF)0x4570, (HALF)0x4b98, (HALF)0xddba, (HALF)0xa220,
	(HALF)0xaf8a, (HALF)0xa2c0, (HALF)0x2bdc, (HALF)0x131b,
	(HALF)0xc2d8, (HALF)0x0020
};
STATIC CONST HALF h_nvec05[] = {
	(HALF)0x6ef1, (HALF)0x9916, (HALF)0xe5e7, (HALF)0x8b99,
	(HALF)0xa010, (HALF)0x8769, (HALF)0xe111, (HALF)0x5d3f,
	(HALF)0xc2fa, (HALF)0x680b, (HALF)0x5aac, (HALF)0x38f7,
	(HALF)0xa85b, (HALF)0xdb81, (HALF)0x1822, (HALF)0x109b,
	(HALF)0x2
};
STATIC CONST HALF h_rvec05[] = {
	(HALF)0xefa9, (HALF)0x59e2, (HALF)0x77c8, (HALF)0x0e6c,
	(HALF)0xaeed, (HALF)0x1e70, (HALF)0x7b7d, (HALF)0x234f,
	(HALF)0xf6db, (HALF)0x5f5d, (HALF)0xa960, (HALF)0xe821,
	(HALF)0xb792, (HALF)0xae33, (HALF)0x890e, (HALF)0x5e9b
};
STATIC CONST HALF h_nvec06[] = {
	(HALF)0xf431, (HALF)0xe1dd, (HALF)0x57f1, (HALF)0xd855,
	(HALF)0x32da, (HALF)0x5ee7, (HALF)0xdb77, (HALF)0x3a38,
	(HALF)0x4026, (HALF)0x5c64, (HALF)0xf218, (HALF)0xf2db,
	(HALF)0x2c79, (HALF)0x9ada, (HALF)0x9d7d, (HALF)0x7bfd,
	(HALF)0xa
};
STATIC CONST HALF h_rvec06[] = {
	(HALF)0x4daf, (HALF)0xc940, (HALF)0x2e80, (HALF)0xc5dc,
	(HALF)0xeccf, (HALF)0x2c98, (HALF)0x495d, (HALF)0xe1f3,
	(HALF)0x925c, (HALF)0xce1c, (HALF)0xaede, (HALF)0xe097,
	(HALF)0x7154, (HALF)0x8866, (HALF)0xa02f, (HALF)0x5e94
};
STATIC CONST HALF h_nvec07[] = {
	(HALF)0xc751, (HALF)0xcf9e, (HALF)0x9125, (HALF)0x602f,
	(HALF)0x2e7f, (HALF)0x5288, (HALF)0x53ce, (HALF)0x0dcf,
	(HALF)0x9d6b, (HALF)0xff56, (HALF)0x43fc, (HALF)0x6286,
	(HALF)0x1cd5, (HALF)0x3780, (HALF)0x9ef2, (HALF)0xf239,
	(HALF)0x7de8, (HALF)0x43d8
};
STATIC CONST HALF h_rvec07[] = {
	(HALF)0x25e6, (HALF)0x098d, (HALF)0xd2e5, (HALF)0x3992,
	(HALF)0xb58c, (HALF)0x64f0, (HALF)0xd4dd, (HALF)0xcf18,
	(HALF)0x6aef, (HALF)0x9d87, (HALF)0xed04, (HALF)0x7acc,
	(HALF)0x9076, (HALF)0xbfbe, (HALF)0x14c7, (HALF)0x1ee0,
	(HALF)0x522d, (HALF)0x0013
};
STATIC CONST HALF h_nvec08[] = {
	(HALF)0x2f11, (HALF)0x2674, (HALF)0xe66a, (HALF)0xbc42,
	(HALF)0xd9f0, (HALF)0xb59c, (HALF)0xa6c2, (HALF)0x9ad4,
	(HALF)0xd2f9, (HALF)0x5bdb, (HALF)0x1fed, (HALF)0xbdc9,
	(HALF)0x9ce7, (HALF)0xf13c, (HALF)0x99b7, (HALF)0xeb46,
	(HALF)0x6ca7, (HALF)0x4712, (HALF)0x58
};
STATIC CONST HALF h_rvec08[] = {
	(HALF)0xc674, (HALF)0x489d, (HALF)0x5f3a, (HALF)0xaae9,
	(HALF)0xa929, (HALF)0xa35d, (HALF)0xb4b8, (HALF)0x5597,
	(HALF)0xc947, (HALF)0x28e9, (HALF)0x4f9a, (HALF)0x3d34,
	(HALF)0x61fa, (HALF)0xb7e6, (HALF)0x9116, (HALF)0xa326,
	(HALF)0x16dc, (HALF)0x8530
};
STATIC CONST HALF h_nvec09[] = {
	(HALF)0xe3d1, (HALF)0xab27, (HALF)0x5db4, (HALF)0x1274,
	(HALF)0xf951, (HALF)0xb980, (HALF)0x6b66, (HALF)0x62b1,
	(HALF)0xce0d, (HALF)0x0fde, (HALF)0xc6fd, (HALF)0x6061,
	(HALF)0xff09, (HALF)0x36a6, (HALF)0xb61c, (HALF)0xe08e,
	(HALF)0x95c3, (HALF)0x84d8, (HALF)0x752a, (HALF)0x4a86,
	(HALF)0x7b4f, (HALF)0xc179, (HALF)0x57a3, (HALF)0x5621,
	(HALF)0x7bb0, (HALF)0x3d26, (HALF)0x1b00, (HALF)0x14e8,
	(HALF)0x9238, (HALF)0x218d, (HALF)0x2fd3, (HALF)0x5232,
	(HALF)0xe8be, (HALF)0x0039
};
STATIC CONST HALF h_rvec09[] = {
	(HALF)0xd20d, (HALF)0x7d4e, (HALF)0xf2b8, (HALF)0x601e,
	(HALF)0xf959, (HALF)0x8e59, (HALF)0x5d9e, (HALF)0xedaa,
	(HALF)0x89ba, (HALF)0x309a, (HALF)0x7d81, (HALF)0xe5ab,
	(HALF)0x2545, (HALF)0x796b, (HALF)0x3222, (HALF)0x02de,
	(HALF)0xc0bd, (HALF)0x8357, (HALF)0xe3fd, (HALF)0x0107,
	(HALF)0xd288, (HALF)0x82d9, (HALF)0xa8aa, (HALF)0xbc42,
	(HALF)0x7343, (HALF)0x4b78, (HALF)0x0886, (HALF)0xc015,
	(HALF)0x15bf, (HALF)0xbab9, (HALF)0x1e6b, (HALF)0xf8ad,
	(HALF)0xb458
};
STATIC CONST HALF h_nvec10[] = {
	(HALF)0x4b89, (HALF)0xb7e6, (HALF)0xc363, (HALF)0xc3cd,
	(HALF)0xc73c, (HALF)0x2ef9, (HALF)0xce22, (HALF)0x6092,
	(HALF)0xe36c, (HALF)0x02ab, (HALF)0x9573, (HALF)0x08d4,
	(HALF)0x1c40, (HALF)0x7451, (HALF)0x82de, (HALF)0xd385,
	(HALF)0xa02f, (HALF)0xa524, (HALF)0x1b3b, (HALF)0x52c8,
	(HALF)0x3cc9, (HALF)0x250d, (HALF)0x0e88, (HALF)0x23b5,
	(HALF)0x48bf, (HALF)0xbd14, (HALF)0x7f98, (HALF)0x882d,
	(HALF)0xf596, (HALF)0xc23e, (HALF)0x5666, (HALF)0xc905,
	(HALF)0x2435, (HALF)0x025f
};
STATIC CONST HALF h_rvec10[] = {
	(HALF)0xc482, (HALF)0x94cf, (HALF)0x5ad4, (HALF)0x594f,
	(HALF)0x2aee, (HALF)0x2344, (HALF)0x40ce, (HALF)0x145f,
	(HALF)0x950d, (HALF)0x1baf, (HALF)0xf175, (HALF)0xadc4,
	(HALF)0x669f, (HALF)0xf62c, (HALF)0x5d56, (HALF)0x8d07,
	(HALF)0x8b40, (HALF)0x08ed, (HALF)0xcf30, (HALF)0xaaf2,
	(HALF)0x5ffb, (HALF)0xc24b, (HALF)0xf8cf, (HALF)0x250d,
	(HALF)0x1ec9, (HALF)0x7ca8, (HALF)0x2b70, (HALF)0x787e,
	(HALF)0x1468, (HALF)0x1840, (HALF)0x0e0c, (HALF)0x47b2,
	(HALF)0xbb7e, (HALF)0x0066
};
STATIC CONST HALF h_nvec11[] = {
	(HALF)0xe069, (HALF)0x546e, (HALF)0x530c, (HALF)0x2e1a,
	(HALF)0xdab2, (HALF)0x2014, (HALF)0xcf52, (HALF)0xa729,
	(HALF)0xe1a9, (HALF)0x920e, (HALF)0x7533, (HALF)0x68f2,
	(HALF)0x3cfa, (HALF)0x2587, (HALF)0xa749, (HALF)0xdd37,
	(HALF)0xdaa2, (HALF)0x4499, (HALF)0x5870, (HALF)0x286e,
	(HALF)0xf9b6, (HALF)0x57f3, (HALF)0x4467, (HALF)0x5ec5,
	(HALF)0x91ea, (HALF)0x69a7, (HALF)0xcd77, (HALF)0x874e,
	(HALF)0xd56b, (HALF)0x4217, (HALF)0xb309, (HALF)0x82bd,
	(HALF)0x64de, (HALF)0x4978
};
STATIC CONST HALF h_rvec11[] = {
	(HALF)0x8b04, (HALF)0x56e3, (HALF)0xded3, (HALF)0x3a0a,
	(HALF)0x88b1, (HALF)0x461d, (HALF)0x4d65, (HALF)0x9c09,
	(HALF)0x3fed, (HALF)0xe533, (HALF)0x18fe, (HALF)0x34d9,
	(HALF)0x6281, (HALF)0x1ef5, (HALF)0xa07c, (HALF)0xcedf,
	(HALF)0x47fb, (HALF)0x590f, (HALF)0x4d5c, (HALF)0xa2c5,
	(HALF)0x39ee, (HALF)0x7323, (HALF)0x49a7, (HALF)0x8065,
	(HALF)0x163f, (HALF)0x9ce3, (HALF)0xf8b6, (HALF)0xae3a,
	(HALF)0x4465, (HALF)0x264a, (HALF)0xe630, (HALF)0x1cb5,
	(HALF)0x8488, (HALF)0x0086
};
STATIC CONST HALF h_nvec12[] = {
	(HALF)0x7b99, (HALF)0xf14c, (HALF)0xd151, (HALF)0x7f66,
	(HALF)0xad2b, (HALF)0x87ef, (HALF)0xf098, (HALF)0x57d3,
	(HALF)0x4165, (HALF)0xd653, (HALF)0xdd25, (HALF)0x812f,
	(HALF)0xc7ce, (HALF)0x48c7, (HALF)0x41e0, (HALF)0xa1bf,
	(HALF)0xe315, (HALF)0x4c94, (HALF)0x1593, (HALF)0x190b,
	(HALF)0x51da, (HALF)0xee42, (HALF)0x1a66, (HALF)0x2b4a,
	(HALF)0xc2a1, (HALF)0x2bb7, (HALF)0x8ca9, (HALF)0x65b1,
	(HALF)0x9116, (HALF)0x08b8, (HALF)0xb15f, (HALF)0xc0cc,
	(HALF)0x2ab3, (HALF)0x5758, (HALF)0x34
};
STATIC CONST HALF h_rvec12[] = {
	(HALF)0xb4a0, (HALF)0xe207, (HALF)0xdd68, (HALF)0x5227,
	(HALF)0xfbc4, (HALF)0x9488, (HALF)0x81aa, (HALF)0x6ed0,
	(HALF)0x6fe5, (HALF)0x8e73, (HALF)0xc020, (HALF)0x3dd2,
	(HALF)0x7c57, (HALF)0xeeb0, (HALF)0x4eb7, (HALF)0x0b60,
	(HALF)0x72a1, (HALF)0xa13f, (HALF)0x4333, (HALF)0xcbfc,
	(HALF)0xe8cd, (HALF)0xa4c5, (HALF)0x520d, (HALF)0x0add,
	(HALF)0xa3b2, (HALF)0x758c, (HALF)0x0137, (HALF)0x5549,
	(HALF)0xbabd, (HALF)0x5870, (HALF)0xed93, (HALF)0xf648,
	(HALF)0x9bd1, (HALF)0xdf71
};
STATIC CONST HALF h_nvec13[] = {
	(HALF)0xfc49, (HALF)0x3314, (HALF)0x0032, (HALF)0xcca2,
	(HALF)0x3420, (HALF)0x208e, (HALF)0x503a, (HALF)0x8aaa,
	(HALF)0x63cc, (HALF)0xd79a, (HALF)0x7417, (HALF)0xb4ed,
	(HALF)0x1892, (HALF)0x95dd, (HALF)0x5f64, (HALF)0xb591,
	(HALF)0xc7f1, (HALF)0xd14c, (HALF)0x917e, (HALF)0x1589,
	(HALF)0x5667, (HALF)0xb2b0, (HALF)0x99cb, (HALF)0xc32d,
	(HALF)0x1a84, (HALF)0x1b5a, (HALF)0x22a1, (HALF)0xa493,
	(HALF)0x76c3, (HALF)0xdd3a, (HALF)0x137f, (HALF)0x8c07,
	(HALF)0x3c63, (HALF)0xaaf8, (HALF)0x5113, (HALF)0x3757,
	(HALF)0x8e84, (HALF)0xa8b1, (HALF)0x891d, (HALF)0xceec,
	(HALF)0xee99, (HALF)0x78c1, (HALF)0xe256, (HALF)0x6e49,
	(HALF)0xbfd6, (HALF)0x4286, (HALF)0xf6a9, (HALF)0xcb6b,
	(HALF)0x8ee0, (HALF)0x7bda, (HALF)0x510a, (HALF)0xd439,
	(HALF)0x345b, (HALF)0x63f4, (HALF)0x5535, (HALF)0x959a,
	(HALF)0x6d82, (HALF)0xdaf6, (HALF)0xd833, (HALF)0xed03,
	(HALF)0xf734, (HALF)0x1b5a, (HALF)0x7dd2, (HALF)0x166b,
	(HALF)0x7c19, (HALF)0x0151
};
STATIC CONST HALF h_rvec13[] = {
	(HALF)0x36f5, (HALF)0x6b77, (HALF)0xbfe4, (HALF)0x2407,
	(HALF)0x2072, (HALF)0x965e, (HALF)0xcf3e, (HALF)0xcc26,
	(HALF)0xb567, (HALF)0xa432, (HALF)0x07ab, (HALF)0x2ed0,
	(HALF)0x67b9, (HALF)0x0e2f, (HALF)0x0960, (HALF)0xef64,
	(HALF)0x1ad3, (HALF)0xbe5f, (HALF)0xda1b, (HALF)0x3fae,
	(HALF)0xb988, (HALF)0xa8f6, (HALF)0xcea5, (HALF)0xe5c9,
	(HALF)0x45ea, (HALF)0xa674, (HALF)0xce78, (HALF)0x3935,
	(HALF)0xff06, (HALF)0xf445, (HALF)0x0a11, (HALF)0xbeda,
	(HALF)0x080b, (HALF)0x92de, (HALF)0x9026, (HALF)0xf404,
	(HALF)0xf0b8, (HALF)0xe509, (HALF)0x216f, (HALF)0x6f05,
	(HALF)0xdc14, (HALF)0x5a68, (HALF)0x730d, (HALF)0x548f,
	(HALF)0x9f00, (HALF)0xe9fd, (HALF)0xaada, (HALF)0x64a4,
	(HALF)0xdd15, (HALF)0xe3bb, (HALF)0xe7a5, (HALF)0xcb4b,
	(HALF)0xd162, (HALF)0x17dd, (HALF)0x8c33, (HALF)0xc491,
	(HALF)0x6d2b, (HALF)0x9b70, (HALF)0xf6a6, (HALF)0x8b04,
	(HALF)0xfa64, (HALF)0x1263, (HALF)0x560d, (HALF)0x8e9a,
	(HALF)0xd42e
};
STATIC CONST HALF h_nvec14[] = {
	(HALF)0xaf01, (HALF)0xc116, (HALF)0x8c0f, (HALF)0xbdef,
	(HALF)0x9a1a, (HALF)0xc440, (HALF)0x185c, (HALF)0xacb3,
	(HALF)0x925b, (HALF)0xb33f, (HALF)0x3005, (HALF)0xfee8,
	(HALF)0xb112, (HALF)0x4b3d, (HALF)0x6743, (HALF)0x7f07,
	(HALF)0x9223, (HALF)0x2170, (HALF)0x054b, (HALF)0x2159,
	(HALF)0xefe3, (HALF)0x6fdf, (HALF)0x0a07, (HALF)0x792d,
	(HALF)0xbd52, (HALF)0x1d14, (HALF)0x9a27, (HALF)0x5f28,
	(HALF)0x83c4, (HALF)0xe034, (HALF)0xa0c1, (HALF)0xcb86,
	(HALF)0x912a, (HALF)0x0ede, (HALF)0x33e3, (HALF)0xf01a,
	(HALF)0x40c6, (HALF)0x63fd, (HALF)0x86e4, (HALF)0x4b05,
	(HALF)0x4a03, (HALF)0xbc7e, (HALF)0x22f3, (HALF)0x956b,
	(HALF)0x0b22, (HALF)0x1056, (HALF)0xd321, (HALF)0x3e78,
	(HALF)0x61e2, (HALF)0x1791, (HALF)0xb909, (HALF)0xe85e,
	(HALF)0x31ee, (HALF)0xec79, (HALF)0xb4bf, (HALF)0xb314,
	(HALF)0x4618, (HALF)0x1f56, (HALF)0x83d0, (HALF)0xb9d9,
	(HALF)0xac07, (HALF)0x7479, (HALF)0xf4e8, (HALF)0x93c6,
	(HALF)0xa00e, (HALF)0x5e56
};
STATIC CONST HALF h_rvec14[] = {
	(HALF)0xf190, (HALF)0x0ff9, (HALF)0xdb68, (HALF)0x47a4,
	(HALF)0xc8ea, (HALF)0x913c, (HALF)0xb220, (HALF)0xb6b1,
	(HALF)0xfbbb, (HALF)0x13ed, (HALF)0xf1c3, (HALF)0xa8f1,
	(HALF)0x1f8f, (HALF)0xd6d7, (HALF)0x649a, (HALF)0x4194,
	(HALF)0x7344, (HALF)0x7d49, (HALF)0x8416, (HALF)0x677c,
	(HALF)0xb983, (HALF)0x0186, (HALF)0x3901, (HALF)0xee63,
	(HALF)0xd69d, (HALF)0xce64, (HALF)0x04b3, (HALF)0x61a7,
	(HALF)0x52b2, (HALF)0x1383, (HALF)0x58d8, (HALF)0xc0fb,
	(HALF)0x2073, (HALF)0x16bf, (HALF)0xae78, (HALF)0x56c9,
	(HALF)0x0a68, (HALF)0xb81a, (HALF)0xabaf, (HALF)0x1512,
	(HALF)0x36ba, (HALF)0x6b99, (HALF)0x0dfc, (HALF)0x4335,
	(HALF)0x19d2, (HALF)0xa0ea, (HALF)0x34c5, (HALF)0xe861,
	(HALF)0x563f, (HALF)0x6c86, (HALF)0x5b68, (HALF)0x6b0c,
	(HALF)0x27fc, (HALF)0x75f6, (HALF)0x913f, (HALF)0xb609,
	(HALF)0xa564, (HALF)0x15b9, (HALF)0xf154, (HALF)0x9b18,
	(HALF)0xc5d0, (HALF)0x6ef0, (HALF)0x3509, (HALF)0xb673,
	(HALF)0xaa7c, (HALF)0x00f7
};
STATIC CONST HALF h_nvec15[] = {
	(HALF)0x7079, (HALF)0xc8d9, (HALF)0x7597, (HALF)0x061e,
	(HALF)0xc721, (HALF)0xf5d2, (HALF)0xc51f, (HALF)0x299b,
	(HALF)0xc337, (HALF)0xffe6, (HALF)0x8624, (HALF)0x1979,
	(HALF)0x92b6, (HALF)0xee6f, (HALF)0x0c7a, (HALF)0x0b1d,
	(HALF)0x8231, (HALF)0xb530, (HALF)0x58dd, (HALF)0x49c5,
	(HALF)0x530e, (HALF)0x196a, (HALF)0x515c, (HALF)0x0caa,
	(HALF)0x86ed, (HALF)0x8b0d, (HALF)0x8fa0, (HALF)0x380a,
	(HALF)0x03e4, (HALF)0x80df, (HALF)0xd81b, (HALF)0xe962,
	(HALF)0x83b3, (HALF)0xc1a7, (HALF)0x8ccc, (HALF)0xc327,
	(HALF)0xab86, (HALF)0x3e72, (HALF)0x1675, (HALF)0xebb9,
	(HALF)0xc2b8, (HALF)0xb902, (HALF)0x0445, (HALF)0xa059,
	(HALF)0xde42, (HALF)0x4f40, (HALF)0xaac1, (HALF)0xaa95,
	(HALF)0x2e82, (HALF)0xfc0f, (HALF)0xb84b, (HALF)0x70ca,
	(HALF)0xa267, (HALF)0x5326, (HALF)0xe607, (HALF)0x470b,
	(HALF)0x2ebe, (HALF)0x4535, (HALF)0x1ca8, (HALF)0x5ba8,
	(HALF)0x6c17, (HALF)0x02c4, (HALF)0xbcdb, (HALF)0x9edf,
	(HALF)0x840b, (HALF)0x97dd
};
STATIC CONST HALF h_rvec15[] = {
	(HALF)0x2110, (HALF)0x6c3e, (HALF)0x0aaa, (HALF)0x808f,
	(HALF)0xb92e, (HALF)0xd98d, (HALF)0xbd43, (HALF)0x1e6a,
	(HALF)0xb920, (HALF)0xf401, (HALF)0x0381, (HALF)0x9d3f,
	(HALF)0xd174, (HALF)0xdb95, (HALF)0x5c33, (HALF)0xa2f6,
	(HALF)0x69f8, (HALF)0x0c54, (HALF)0x26fc, (HALF)0xa3c1,
	(HALF)0x241b, (HALF)0x8866, (HALF)0xeca7, (HALF)0x0e46,
	(HALF)0x57fa, (HALF)0xa705, (HALF)0x91a6, (HALF)0x8e73,
	(HALF)0xa9e2, (HALF)0x70e4, (HALF)0x7e89, (HALF)0x9ad9,
	(HALF)0x7dee, (HALF)0x1eb7, (HALF)0x24d0, (HALF)0xfe62,
	(HALF)0x22b1, (HALF)0xdbe5, (HALF)0x1023, (HALF)0xa4aa,
	(HALF)0x6860, (HALF)0x7f22, (HALF)0x4379, (HALF)0x60ef,
	(HALF)0xb296, (HALF)0x45e3, (HALF)0xf5ef, (HALF)0xcc31,
	(HALF)0xf6d6, (HALF)0x74fb, (HALF)0xc25d, (HALF)0x55b1,
	(HALF)0x521f, (HALF)0x3ede, (HALF)0xef42, (HALF)0xdf8c,
	(HALF)0x7ca6, (HALF)0xa8d7, (HALF)0x25da, (HALF)0xb500,
	(HALF)0x99f9, (HALF)0x69ab, (HALF)0xc758, (HALF)0x03b8,
	(HALF)0x2207, (HALF)0x00b8
};
STATIC CONST HALF h_nvec16[] = {
	(HALF)0x46e1, (HALF)0xd843, (HALF)0x83f6, (HALF)0x1841,
	(HALF)0xbd36, (HALF)0x2dc9, (HALF)0x57ac, (HALF)0x4ca8,
	(HALF)0x828d, (HALF)0x96a5, (HALF)0x1c59, (HALF)0xed1f,
	(HALF)0x731f, (HALF)0x36d9, (HALF)0x6183, (HALF)0xbd3f,
	(HALF)0x5578, (HALF)0xde0f, (HALF)0xea8a, (HALF)0xb6a2,
	(HALF)0x3c44, (HALF)0xbe99, (HALF)0x3c05, (HALF)0x0e28,
	(HALF)0x61e3, (HALF)0xd7cf, (HALF)0x15b6, (HALF)0x40fe,
	(HALF)0x967e, (HALF)0x534d, (HALF)0x1046, (HALF)0x3469,
	(HALF)0x45bd, (HALF)0xd408, (HALF)0xee4b, (HALF)0xd69c,
	(HALF)0xee8d, (HALF)0x557f, (HALF)0x56ba, (HALF)0x51c8,
	(HALF)0x51ba, (HALF)0xe6bc, (HALF)0xc173, (HALF)0x587b,
	(HALF)0xe379, (HALF)0x1959, (HALF)0x8439, (HALF)0x9282,
	(HALF)0x0503, (HALF)0x311e, (HALF)0x9cc2, (HALF)0x7f3c,
	(HALF)0x512c, (HALF)0xd426, (HALF)0xb497, (HALF)0xe8b2,
	(HALF)0x536a, (HALF)0x9e43, (HALF)0x4cb8, (HALF)0x9f54,
	(HALF)0x84c3, (HALF)0xb56f, (HALF)0xbb12, (HALF)0xb82f,
	(HALF)0x8549, (HALF)0x6e34, (HALF)0x45
};
STATIC CONST HALF h_rvec16[] = {
	(HALF)0x8830, (HALF)0x7b4e, (HALF)0x5db8, (HALF)0x7060, (HALF)0xe4a5,
	(HALF)0xa1ab, (HALF)0xbe04, (HALF)0xa70f,
	(HALF)0xa8f4, (HALF)0x2bcd, (HALF)0xda9a, (HALF)0xd29a,
	(HALF)0x0560, (HALF)0x55ad, (HALF)0x137e, (HALF)0xb367,
	(HALF)0x2f1a, (HALF)0xd697, (HALF)0xad45, (HALF)0x809b,
	(HALF)0x2454, (HALF)0xb15d, (HALF)0x415f, (HALF)0x0c0d,
	(HALF)0x117a, (HALF)0x416e, (HALF)0x9521, (HALF)0xe87a,
	(HALF)0x5e1a, (HALF)0x670a, (HALF)0x1772, (HALF)0x53a4,
	(HALF)0x5cc1, (HALF)0xfc9c, (HALF)0x45df, (HALF)0xf756,
	(HALF)0xd19f, (HALF)0x86f6, (HALF)0xb5bf, (HALF)0x9404,
	(HALF)0xd83b, (HALF)0x56e9, (HALF)0x3bc3, (HALF)0xac0f,
	(HALF)0x8c4b, (HALF)0xa150, (HALF)0x4977, (HALF)0x4bfd,
	(HALF)0x2540, (HALF)0x7192, (HALF)0x4def, (HALF)0xf252,
	(HALF)0xa3db, (HALF)0x8a81, (HALF)0x28de, (HALF)0xced8,
	(HALF)0xae8f, (HALF)0x7895, (HALF)0x6dcd, (HALF)0x4a4b,
	(HALF)0x921a, (HALF)0x973e, (HALF)0x7a07, (HALF)0x9fb2,
	(HALF)0xdcb1, (HALF)0xb0d7
};
STATIC CONST HALF h_nvec17[] = {
	(HALF)0x2051, (HALF)0x72b7, (HALF)0x4ebf, (HALF)0xedc2,
	(HALF)0xa8d1, (HALF)0xe970, (HALF)0xb150, (HALF)0x66c9,
	(HALF)0x27f7, (HALF)0xcbb9, (HALF)0xffd9, (HALF)0xb574,
	(HALF)0xb249, (HALF)0x4166, (HALF)0x4030, (HALF)0x0fce,
	(HALF)0x22ca, (HALF)0xfa69, (HALF)0x14a9, (HALF)0x39cc,
	(HALF)0x6e2a, (HALF)0x1439, (HALF)0x4c7f, (HALF)0xaff7,
	(HALF)0xa314, (HALF)0xa120, (HALF)0x2700, (HALF)0xe11a,
	(HALF)0xad30, (HALF)0x44c9, (HALF)0x8d72, (HALF)0x7f32,
	(HALF)0xeaf7, (HALF)0xab2c, (HALF)0xf772, (HALF)0x868f,
	(HALF)0xf0b3, (HALF)0x0974, (HALF)0xf0f6, (HALF)0x20e9,
	(HALF)0x5b8a, (HALF)0xcd4e, (HALF)0x26e6, (HALF)0x0bde,
	(HALF)0xc3ac, (HALF)0x96f6, (HALF)0xc601, (HALF)0x5718,
	(HALF)0x7710, (HALF)0x2f11, (HALF)0x876e, (HALF)0x1ab0,
	(HALF)0x2c2e, (HALF)0x49ab, (HALF)0x22b9, (HALF)0x7470,
	(HALF)0xe4a7, (HALF)0x6e9d, (HALF)0x8f2e, (HALF)0x25e8,
	(HALF)0xa00b, (HALF)0xd7d0, (HALF)0x11f6, (HALF)0xc8ff,
	(HALF)0xa819, (HALF)0xf50a, (HALF)0x0e9e, (HALF)0xbe53,
	(HALF)0xff54, (HALF)0x47b7, (HALF)0x9b46, (HALF)0xe020,
	(HALF)0xc5eb, (HALF)0x027e, (HALF)0x3362, (HALF)0x0754,
	(HALF)0x9b85, (HALF)0x531e, (HALF)0xb568, (HALF)0x3c23,
	(HALF)0xaf7a, (HALF)0x5d07, (HALF)0x8461, (HALF)0xd494,
	(HALF)0xa499, (HALF)0x2eb9, (HALF)0xfa0b, (HALF)0x1c71,
	(HALF)0xe22b, (HALF)0x7025, (HALF)0x20b9, (HALF)0x4d27,
	(HALF)0x8d54, (HALF)0x531b, (HALF)0xfc30, (HALF)0x66e2,
	(HALF)0xcafd, (HALF)0xdac7, (HALF)0x953b, (HALF)0x7f29,
	(HALF)0x6bff, (HALF)0x9d45, (HALF)0x14bc, (HALF)0x8398,
	(HALF)0xfb19, (HALF)0xd0e5, (HALF)0x9f58, (HALF)0x5a6c,
	(HALF)0x9dc3, (HALF)0x3a5a, (HALF)0xf28b, (HALF)0x598b,
	(HALF)0x1144, (HALF)0xa7a9, (HALF)0x4f76, (HALF)0x6849,
	(HALF)0xd7be, (HALF)0xbfed, (HALF)0x4266, (HALF)0x7ca5,
	(HALF)0xfaf9, (HALF)0x96e0, (HALF)0x3c0f, (HALF)0x33be,
	(HALF)0x040b, (HALF)0xffa3, (HALF)0xeac0, (HALF)0x813a,
	(HALF)0x6177
};
STATIC CONST HALF h_rvec17[] = {
	(HALF)0x1dac, (HALF)0x22b4, (HALF)0x8005, (HALF)0xd625,
	(HALF)0xe0cb, (HALF)0x2aa1, (HALF)0x47b5, (HALF)0x45d1,
	(HALF)0x46d9, (HALF)0xbf5c, (HALF)0xdadf, (HALF)0x14c9,
	(HALF)0xaec4, (HALF)0x09b0, (HALF)0xbfef, (HALF)0x4286,
	(HALF)0xe9d1, (HALF)0xc6f8, (HALF)0x467b, (HALF)0xdd68,
	(HALF)0xffb9, (HALF)0x93f4, (HALF)0xeb51, (HALF)0x58f2,
	(HALF)0x048f, (HALF)0x2ade, (HALF)0xe6e5, (HALF)0xeaca,
	(HALF)0xa807, (HALF)0x8dd2, (HALF)0x8c27, (HALF)0xbcea,
	(HALF)0x3281, (HALF)0x02a0, (HALF)0xeb6d, (HALF)0x039a,
	(HALF)0x016c, (HALF)0xfa6e, (HALF)0x1b09, (HALF)0x6fda,
	(HALF)0x19ed, (HALF)0xea77, (HALF)0x0294, (HALF)0xcf2e,
	(HALF)0x4cb9, (HALF)0xa426, (HALF)0x8af1, (HALF)0x4988,
	(HALF)0xf0c1, (HALF)0x1b44, (HALF)0xe577, (HALF)0x3cce,
	(HALF)0xd170, (HALF)0xdbeb, (HALF)0x1e7e, (HALF)0xf743,
	(HALF)0xd584, (HALF)0xfaeb, (HALF)0x4a33, (HALF)0x896e,
	(HALF)0xf43c, (HALF)0xe46b, (HALF)0x9a10, (HALF)0x17fe,
	(HALF)0x1b51, (HALF)0xb532, (HALF)0xd2a9, (HALF)0xf7e1,
	(HALF)0x0a56, (HALF)0xe7fe, (HALF)0x6750, (HALF)0xbd73,
	(HALF)0x9a33, (HALF)0xcf02, (HALF)0x99b1, (HALF)0xebee,
	(HALF)0xff31, (HALF)0x810f, (HALF)0x8d30, (HALF)0x694c,
	(HALF)0x8689, (HALF)0xc8c1, (HALF)0xf4fb, (HALF)0xc2f9,
	(HALF)0xfd7f, (HALF)0x5949, (HALF)0xf7b3, (HALF)0x67aa,
	(HALF)0x906a, (HALF)0xa82f, (HALF)0xb7b3, (HALF)0x1b84,
	(HALF)0x52ee, (HALF)0xeac0, (HALF)0x9345, (HALF)0x1a4e,
	(HALF)0x2973, (HALF)0xce3c, (HALF)0x68a7, (HALF)0x4a51,
	(HALF)0x51ba, (HALF)0x5c55, (HALF)0xcb26, (HALF)0x77c6,
	(HALF)0xa3a6, (HALF)0xfa45, (HALF)0x31e0, (HALF)0x486f,
	(HALF)0x7519, (HALF)0xcaf9, (HALF)0x0399, (HALF)0xbe4b,
	(HALF)0xc106, (HALF)0x802f, (HALF)0x84da, (HALF)0x5372,
	(HALF)0xe167, (HALF)0x20c4, (HALF)0xf329, (HALF)0x2a62,
	(HALF)0xfc5b, (HALF)0xc2d2, (HALF)0x5324, (HALF)0xdd66,
	(HALF)0xadf1, (HALF)0xc3b8, (HALF)0xaf3b, (HALF)0x0b6e,
	(HALF)0x5372
};
STATIC CONST HALF h_nvec18[] = {
	(HALF)0x8629, (HALF)0xc8b7, (HALF)0x1b18, (HALF)0x4135, (HALF)0x4ed8,
	(HALF)0x28ad, (HALF)0x7df1, (HALF)0xc96f,
	(HALF)0xc931, (HALF)0x7cd3, (HALF)0x036a, (HALF)0x0f23,
	(HALF)0x7631, (HALF)0xac65, (HALF)0x5812, (HALF)0x6a62,
	(HALF)0x4788, (HALF)0x0814, (HALF)0xed62, (HALF)0x8642,
	(HALF)0x8a40, (HALF)0x7619, (HALF)0xfd64, (HALF)0x70de,
	(HALF)0x673c, (HALF)0x97fb, (HALF)0xddf6, (HALF)0x6f3b,
	(HALF)0x2977, (HALF)0x72fe, (HALF)0x82f1, (HALF)0x20ed,
	(HALF)0x4fdc, (HALF)0x7e7f, (HALF)0x2d6b, (HALF)0xdc27,
	(HALF)0xf317, (HALF)0x6d77, (HALF)0x5b15, (HALF)0x2c59,
	(HALF)0x7dd6, (HALF)0x1c3b, (HALF)0x6147, (HALF)0x6e3d,
	(HALF)0x640c, (HALF)0x8170, (HALF)0x033f, (HALF)0xb660,
	(HALF)0x886b, (HALF)0xf211, (HALF)0x7859, (HALF)0xbcc6,
	(HALF)0x4b93, (HALF)0x18ff, (HALF)0xe691, (HALF)0x3068,
	(HALF)0xd823, (HALF)0x9db5, (HALF)0xd4ef, (HALF)0x72af,
	(HALF)0xcd1a, (HALF)0x5aa4, (HALF)0x3014, (HALF)0xa0a3,
	(HALF)0x49e6, (HALF)0x6b83, (HALF)0xe595, (HALF)0x2f4d,
	(HALF)0x0384, (HALF)0xd518, (HALF)0x4118, (HALF)0x19d9,
	(HALF)0x7534, (HALF)0x369e, (HALF)0x3b18, (HALF)0xce4d,
	(HALF)0xf9ee, (HALF)0x119e, (HALF)0x5c25, (HALF)0xe2f4,
	(HALF)0xe0a7, (HALF)0xe8ea, (HALF)0x1605, (HALF)0x62e4,
	(HALF)0xd2ca, (HALF)0x6346, (HALF)0x625b, (HALF)0x6425,
	(HALF)0x33de, (HALF)0x44b0, (HALF)0xe6b3, (HALF)0x1711,
	(HALF)0xd02e, (HALF)0xf3a9, (HALF)0xd965, (HALF)0x259c,
	(HALF)0x956b, (HALF)0x08aa, (HALF)0x4380, (HALF)0x6ad6,
	(HALF)0x0e8e, (HALF)0xe973, (HALF)0x9d28, (HALF)0x539b,
	(HALF)0x7950, (HALF)0xe540, (HALF)0x0be4, (HALF)0x8990,
	(HALF)0x18f6, (HALF)0xde12, (HALF)0xe52b, (HALF)0x63e1,
	(HALF)0x3f4e, (HALF)0x0de0, (HALF)0xa568, (HALF)0x8e21,
	(HALF)0x7ee3, (HALF)0x268d, (HALF)0x514e, (HALF)0xafb3,
	(HALF)0xefcb, (HALF)0x5378, (HALF)0xc7c6, (HALF)0xfec0,
	(HALF)0xb724, (HALF)0xf07c, (HALF)0xb42a, (HALF)0xfb61,
	(HALF)0x2a38, (HALF)0x068f
};
STATIC CONST HALF h_rvec18[] = {
	(HALF)0x3c63, (HALF)0x35ea, (HALF)0xef97, (HALF)0x8df2,
	(HALF)0xafb7, (HALF)0xa2b3, (HALF)0x58f6, (HALF)0x1791,
	(HALF)0x0dba, (HALF)0x0492, (HALF)0x077e, (HALF)0xf333,
	(HALF)0x4b5a, (HALF)0xf830, (HALF)0xf2ae, (HALF)0x230f,
	(HALF)0xf3f0, (HALF)0x84a8, (HALF)0x164e, (HALF)0xadda,
	(HALF)0xc944, (HALF)0xc9a1, (HALF)0x02f2, (HALF)0xc705,
	(HALF)0xc18f, (HALF)0x41a3, (HALF)0x3254, (HALF)0x09bd,
	(HALF)0x65a9, (HALF)0x9736, (HALF)0xc263, (HALF)0x1548,
	(HALF)0xd916, (HALF)0x5024, (HALF)0xdde9, (HALF)0x0a3d,
	(HALF)0xf1f5, (HALF)0xf2aa, (HALF)0xb92a, (HALF)0x666d,
	(HALF)0x5aa5, (HALF)0x3a53, (HALF)0x5775, (HALF)0x49c3,
	(HALF)0xa1c4, (HALF)0xc381, (HALF)0x6dbc, (HALF)0xf8d3,
	(HALF)0xe870, (HALF)0xe94b, (HALF)0x88a6, (HALF)0x430e,
	(HALF)0x9a06, (HALF)0x3721, (HALF)0xdf80, (HALF)0x5109,
	(HALF)0xa03f, (HALF)0xe73c, (HALF)0x4541, (HALF)0xf1bb,
	(HALF)0x32f3, (HALF)0x3c6f, (HALF)0xfc24, (HALF)0x952c,
	(HALF)0x697f, (HALF)0xfbc9, (HALF)0xd472, (HALF)0xc5b8,
	(HALF)0xda4b, (HALF)0xcbbe, (HALF)0xdb3b, (HALF)0x7303,
	(HALF)0x255e, (HALF)0xa18f, (HALF)0x353d, (HALF)0xe1d3,
	(HALF)0x8700, (HALF)0xe8c9, (HALF)0xe8fd, (HALF)0x9e75,
	(HALF)0x812e, (HALF)0x3ddd, (HALF)0xf891, (HALF)0x7340,
	(HALF)0x69ac, (HALF)0xb1f3, (HALF)0x505b, (HALF)0x764d,
	(HALF)0xf51b, (HALF)0xb13e, (HALF)0xaa43, (HALF)0x16c3,
	(HALF)0x42c4, (HALF)0x61d0, (HALF)0x0339, (HALF)0x22ac,
	(HALF)0x6fd1, (HALF)0x3f30, (HALF)0x6f7f, (HALF)0x4992,
	(HALF)0x575c, (HALF)0x2b4c, (HALF)0xb467, (HALF)0x9f3a,
	(HALF)0x65af, (HALF)0x5dac, (HALF)0x8dc7, (HALF)0x6277,
	(HALF)0x3a89, (HALF)0x9911, (HALF)0x540a, (HALF)0x49c0,
	(HALF)0x0ac2, (HALF)0x1df7, (HALF)0x2c5e, (HALF)0x4be1,
	(HALF)0x6bdb, (HALF)0xe5d3, (HALF)0x9ff5, (HALF)0x66b9,
	(HALF)0xbe89, (HALF)0x5358, (HALF)0x35d7, (HALF)0x4cd8,
	(HALF)0xcda8, (HALF)0xf0d5, (HALF)0xc6c3, (HALF)0x1f1a,
	(HALF)0x5e92, (HALF)0x0473
};
STATIC CONST HALF h_nvec19[] = {
	(HALF)0x9a79, (HALF)0x6b65, (HALF)0xc12d, (HALF)0x0239,
	(HALF)0xdf49, (HALF)0xd204, (HALF)0xe0c7, (HALF)0x1d4a,
	(HALF)0xf000, (HALF)0x099b, (HALF)0xade8, (HALF)0x6435,
	(HALF)0xf029, (HALF)0xdc4a, (HALF)0xe7a2, (HALF)0x2f4e,
	(HALF)0xf1e3, (HALF)0xadfc, (HALF)0x8f43, (HALF)0x7335,
	(HALF)0xede5, (HALF)0x687e, (HALF)0xcd4d, (HALF)0xb567,
	(HALF)0x814f, (HALF)0xc7a7, (HALF)0x624c, (HALF)0xc306,
	(HALF)0x80c6, (HALF)0xa82d, (HALF)0x0cd5, (HALF)0x3f39,
	(HALF)0xec3f, (HALF)0x7b7d, (HALF)0x1416, (HALF)0x8bdb,
	(HALF)0x3a52, (HALF)0x275b, (HALF)0x84fe, (HALF)0x9218,
	(HALF)0x02e0, (HALF)0x94ac, (HALF)0xb52c, (HALF)0x62f2,
	(HALF)0x92ee, (HALF)0xcdc9, (HALF)0x5eeb, (HALF)0x35e5,
	(HALF)0x3fd5, (HALF)0x69a4, (HALF)0xdcfb, (HALF)0x44c1,
	(HALF)0x6227, (HALF)0x3cdf, (HALF)0x148f, (HALF)0x23f3,
	(HALF)0x8e4c, (HALF)0x4250, (HALF)0x37c3, (HALF)0x95b7,
	(HALF)0x831f, (HALF)0x70af, (HALF)0x15c9, (HALF)0x2ee8,
	(HALF)0x7251, (HALF)0x47a4, (HALF)0xb2af, (HALF)0x7c11,
	(HALF)0x361f, (HALF)0x664c, (HALF)0xa841, (HALF)0xcada,
	(HALF)0x172a, (HALF)0x3d97, (HALF)0x2ffb, (HALF)0x8740,
	(HALF)0x2ebd, (HALF)0xa0a0, (HALF)0x225f, (HALF)0xb674,
	(HALF)0x3fe2, (HALF)0x6559, (HALF)0x98b9, (HALF)0x85f6,
	(HALF)0xa7ab, (HALF)0x5ed9, (HALF)0x7371, (HALF)0x6bf3,
	(HALF)0x5305, (HALF)0x7da7, (HALF)0x55bf, (HALF)0x0882,
	(HALF)0x7684, (HALF)0x8f60, (HALF)0xe57e, (HALF)0x1f3d,
	(HALF)0xd501, (HALF)0x118b, (HALF)0x770d, (HALF)0x6833,
	(HALF)0x5c51, (HALF)0xd042, (HALF)0xcacb, (HALF)0x2664,
	(HALF)0xb920, (HALF)0x9206, (HALF)0x3b8d, (HALF)0xeb90,
	(HALF)0x16b4, (HALF)0x0e25, (HALF)0xd841, (HALF)0x36c3,
	(HALF)0xcd17, (HALF)0x51d7, (HALF)0xba1e, (HALF)0xe063,
	(HALF)0x4a6d, (HALF)0xf28f, (HALF)0xeb0d, (HALF)0x244d,
	(HALF)0x0ad4, (HALF)0x2e41, (HALF)0xc315, (HALF)0x0721,
	(HALF)0x7654, (HALF)0xdde2, (HALF)0x534b, (HALF)0x2ad6,
	(HALF)0x8b25, (HALF)0xd678, (HALF)0xb9e8, (HALF)0xb23b,
	(HALF)0x0d7a, (HALF)0x0023
};
STATIC CONST HALF h_rvec19[] = {
	(HALF)0xf473, (HALF)0x698e, (HALF)0xa5b7, (HALF)0x3d53,
	(HALF)0x8319, (HALF)0x0644, (HALF)0x4445, (HALF)0xd9ad,
	(HALF)0xdaa0, (HALF)0x6967, (HALF)0x6240, (HALF)0xa14c,
	(HALF)0x7724, (HALF)0x78e7, (HALF)0x2ab7, (HALF)0x63ef,
	(HALF)0x2ee2, (HALF)0x8dff, (HALF)0xb424, (HALF)0x662e,
	(HALF)0x07d6, (HALF)0xcd93, (HALF)0x6a5d, (HALF)0x0ab0,
	(HALF)0xb539, (HALF)0xbdd7, (HALF)0xdd2d, (HALF)0x8621,
	(HALF)0xa187, (HALF)0x2bb4, (HALF)0x121d, (HALF)0x1f1e,
	(HALF)0xb962, (HALF)0xce8d, (HALF)0xeaf1, (HALF)0xdd3e,
	(HALF)0x6ca1, (HALF)0x573b, (HALF)0x0cd6, (HALF)0x6f46,
	(HALF)0x4780, (HALF)0x6a8d, (HALF)0xc7e6, (HALF)0xea68,
	(HALF)0xeb32, (HALF)0x1148, (HALF)0x44d4, (HALF)0xb43d,
	(HALF)0xcb64, (HALF)0xb657, (HALF)0xfdba, (HALF)0x8547,
	(HALF)0x3333, (HALF)0x85f7, (HALF)0xa51a, (HALF)0xc1f2,
	(HALF)0xee52, (HALF)0x1c05, (HALF)0xc03d, (HALF)0x2847,
	(HALF)0x88d8, (HALF)0xfc7a, (HALF)0xd186, (HALF)0x2e8d,
	(HALF)0x4683, (HALF)0x5be3, (HALF)0x2ee2, (HALF)0xd43d,
	(HALF)0x2bb5, (HALF)0x9f2d, (HALF)0xddea, (HALF)0x89b3,
	(HALF)0x4ae0, (HALF)0x7d78, (HALF)0x5e28, (HALF)0xc773,
	(HALF)0x8608, (HALF)0x967a, (HALF)0xcf07, (HALF)0xcdcb,
	(HALF)0xa423, (HALF)0xfc17, (HALF)0xd053, (HALF)0xd36a,
	(HALF)0x8892, (HALF)0xc73d, (HALF)0xc3f4, (HALF)0xa635,
	(HALF)0x0cf9, (HALF)0x9b5d, (HALF)0x3fd9, (HALF)0x0ac7,
	(HALF)0xfefb, (HALF)0xe801, (HALF)0xffd2, (HALF)0xb31c,
	(HALF)0xaa55, (HALF)0xf3ea, (HALF)0xfa23, (HALF)0x0e74,
	(HALF)0xb290, (HALF)0x5414, (HALF)0x6101, (HALF)0x6d17,
	(HALF)0x93f8, (HALF)0x5229, (HALF)0x3dad, (HALF)0xf829,
	(HALF)0x1e82, (HALF)0x713b, (HALF)0xbef0, (HALF)0xb83b,
	(HALF)0xcc62, (HALF)0xd001, (HALF)0xda39, (HALF)0x7537,
	(HALF)0x158b, (HALF)0x7d80, (HALF)0xc8e2, (HALF)0x9332,
	(HALF)0xfa75, (HALF)0x6fa6, (HALF)0x6512, (HALF)0xe21f,
	(HALF)0x18b4, (HALF)0x9995, (HALF)0x605b, (HALF)0x2196,
	(HALF)0x1798, (HALF)0xe4fc, (HALF)0xe245, (HALF)0x5f21,
	(HALF)0xf172, (HALF)0x0008
};
STATIC CONST HALF h_nvec20[] = {
	(HALF)0xd081, (HALF)0xc3c1, (HALF)0x2fce, (HALF)0x4d26,
	(HALF)0xcc91, (HALF)0x8765, (HALF)0x7f7c, (HALF)0xf372,
	(HALF)0x4bbc, (HALF)0xabba, (HALF)0x5801, (HALF)0xe098,
	(HALF)0x5c51, (HALF)0xfa36, (HALF)0xb230, (HALF)0xb2a4,
	(HALF)0x0a8d, (HALF)0xf443, (HALF)0x98c8, (HALF)0x546b,
	(HALF)0x8b26, (HALF)0xd974, (HALF)0xa82f, (HALF)0xe255,
	(HALF)0x3e8c, (HALF)0xb00a, (HALF)0x676d, (HALF)0x7069,
	(HALF)0xccce, (HALF)0x6233, (HALF)0xa74e, (HALF)0x0299,
	(HALF)0xadf9, (HALF)0xd119, (HALF)0xe811, (HALF)0x5273,
	(HALF)0xe0bb, (HALF)0xae36, (HALF)0xa486, (HALF)0x32b3,
	(HALF)0x21a5, (HALF)0xf049, (HALF)0x28a2, (HALF)0xd31d,
	(HALF)0x50de, (HALF)0xda0c, (HALF)0x2b40, (HALF)0x2130,
	(HALF)0xe552, (HALF)0xee9d, (HALF)0xac03, (HALF)0x80bb,
	(HALF)0x9740, (HALF)0x75f4, (HALF)0xa61b, (HALF)0xc3d0,
	(HALF)0xfbf6, (HALF)0x341c, (HALF)0xb5b4, (HALF)0x1615,
	(HALF)0x3def, (HALF)0x4e3a, (HALF)0x4dbf, (HALF)0x9573,
	(HALF)0x78ac, (HALF)0xe7ab, (HALF)0x5cf9, (HALF)0xffdc,
	(HALF)0x6892, (HALF)0xf799, (HALF)0x7ba4, (HALF)0x4740,
	(HALF)0x988f, (HALF)0xa78b, (HALF)0x6f42, (HALF)0x2773,
	(HALF)0xee50, (HALF)0x5064, (HALF)0x5c56, (HALF)0x6013,
	(HALF)0x3283, (HALF)0x1ad7, (HALF)0x4bf7, (HALF)0x2562,
	(HALF)0x1419, (HALF)0x2ee2, (HALF)0x5abd, (HALF)0x9319,
	(HALF)0x7778, (HALF)0x66b6, (HALF)0xa42d, (HALF)0xb1e0,
	(HALF)0xb0f0, (HALF)0x729f, (HALF)0x1864, (HALF)0xd492,
	(HALF)0x253f, (HALF)0x2c42, (HALF)0x07a9, (HALF)0x302a,
	(HALF)0x1bd4, (HALF)0xbb74, (HALF)0x90ba, (HALF)0x932f,
	(HALF)0x4be1, (HALF)0xf335, (HALF)0xd661, (HALF)0x0804,
	(HALF)0x9ba1, (HALF)0x010e, (HALF)0x778d, (HALF)0x1a05,
	(HALF)0xc833, (HALF)0xa962, (HALF)0x0ee8, (HALF)0xe759,
	(HALF)0x03b8, (HALF)0xbe68, (HALF)0x04c1, (HALF)0xc677,
	(HALF)0x9660, (HALF)0x56d7, (HALF)0xa3f3, (HALF)0x6066,
	(HALF)0x0327, (HALF)0x648b, (HALF)0x5b3a, (HALF)0x267e,
	(HALF)0x63a0, (HALF)0xdddc, (HALF)0xe890, (HALF)0x3322,
	(HALF)0xd8b1, (HALF)0x20e0, (HALF)0xd2b8, (HALF)0x004f
};
STATIC CONST HALF h_rvec20[] = {
	(HALF)0xbd1a, (HALF)0xa048, (HALF)0xdc7b, (HALF)0x95ab,
	(HALF)0x7cf8, (HALF)0x98f4, (HALF)0xc98d, (HALF)0x126a,
	(HALF)0x85fd, (HALF)0xaebf, (HALF)0x580f, (HALF)0x5650,
	(HALF)0xd7dd, (HALF)0x3292, (HALF)0x8377, (HALF)0xf49e,
	(HALF)0xed46, (HALF)0x2947, (HALF)0xb26c, (HALF)0xd1a5,
	(HALF)0xe6a1, (HALF)0xae14, (HALF)0x5788, (HALF)0x9b1f,
	(HALF)0x27b2, (HALF)0x4df7, (HALF)0x5079, (HALF)0xee37,
	(HALF)0xc8e4, (HALF)0x131b, (HALF)0x5f53, (HALF)0x294e,
	(HALF)0x59bd, (HALF)0x1f57, (HALF)0x8acf, (HALF)0x65d5,
	(HALF)0xd3a5, (HALF)0x598e, (HALF)0x61a6, (HALF)0xc393,
	(HALF)0xfd7a, (HALF)0xa783, (HALF)0x36a2, (HALF)0x264a,
	(HALF)0x2856, (HALF)0x6ca2, (HALF)0x171f, (HALF)0x8ffb,
	(HALF)0xea9e, (HALF)0x1d7c, (HALF)0x6fca, (HALF)0xd81d,
	(HALF)0x730e, (HALF)0x34ea, (HALF)0x6382, (HALF)0x31f5,
	(HALF)0xd9e9, (HALF)0xb39c, (HALF)0x84be, (HALF)0x440e,
	(HALF)0x15a1, (HALF)0x4b1d, (HALF)0x75c5, (HALF)0x7bf7,
	(HALF)0x4638, (HALF)0xe40f, (HALF)0xf0a7, (HALF)0xe5be,
	(HALF)0x8942, (HALF)0x79e5, (HALF)0xe1ba, (HALF)0x881a,
	(HALF)0x8372, (HALF)0x01de, (HALF)0x35f8, (HALF)0x14cf,
	(HALF)0xb310, (HALF)0xe2d8, (HALF)0x1207, (HALF)0x6696,
	(HALF)0x5f91, (HALF)0xde5d, (HALF)0x0849, (HALF)0xe6e7,
	(HALF)0x5ac3, (HALF)0x74ec, (HALF)0x4eb1, (HALF)0xe2de,
	(HALF)0xdc20, (HALF)0x4a41, (HALF)0xd565, (HALF)0xd306,
	(HALF)0x3ff3, (HALF)0xb584, (HALF)0x30d6, (HALF)0x911b,
	(HALF)0xd926, (HALF)0x4e9c, (HALF)0xc9ae, (HALF)0x8455,
	(HALF)0x8bb5, (HALF)0x6944, (HALF)0x1aad, (HALF)0x0c7b,
	(HALF)0xe992, (HALF)0x1da1, (HALF)0x56bd, (HALF)0xc676,
	(HALF)0x209e, (HALF)0xc544, (HALF)0x387c, (HALF)0x10ce,
	(HALF)0x8df8, (HALF)0xc4e8, (HALF)0xda88, (HALF)0x40e8,
	(HALF)0x3028, (HALF)0xbb2c, (HALF)0x4fd9, (HALF)0x4919,
	(HALF)0x17ee, (HALF)0xdeef, (HALF)0xc08d, (HALF)0x241b,
	(HALF)0xf608, (HALF)0x6fa9, (HALF)0x8b04, (HALF)0x4b0f,
	(HALF)0x0da1, (HALF)0xee96, (HALF)0x9293, (HALF)0xa309,
	(HALF)0x5fea, (HALF)0x8444, (HALF)0xef01, (HALF)0x0046
};
#else
   /\../\	FULL_BITS must be 32 or 64	/\../\	 !!!
#endif
/*
 * NOTE: set n is found in random_pregen[n-1]
 */
STATIC RANDOM random_pregen[BLUM_PREGEN] = {
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
 * forward static declarations
 */
S_FUNC void zfree_random(ZVALUE z);


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
		randomfree(&blum);
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
		p_blum = randomcopy(&init_blum);
		randomfree(&blum);
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
				zfree_random(last_r);
			}

			/*
			 * last_r = r;
			 * r = pmod(last_r, 2, n);
			 */
			last_r = r;
			zsquaremod(last_r, blum.n, &r);
		} while (zrel(r, last_r) > 0);
		zfree_random(blum.r);
		blum.r = r;
		/* free temp storage */
		zfree_random(last_r);

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
		randomfree(&blum);
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
	 * Set the Blum modulus to one of the pre-defined Blum moduli.
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
		zfree_random(blum.n);
		zcopy(random_pregen[set-1].n, &blum.n);
		blum.loglogn = random_pregen[set-1].loglogn;
		blum.mask = random_pregen[set-1].mask;

		/*
		 * reset initial seed as well if seed is 0
		 */
		if (ziszero(seed)) {
			zfree_random(blum.r);
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
		zfree_random(blum.n);
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
		randomfree(&blum);
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
	zfree_random(blum.n);
	zmul(p, q, &blum.n);
	/* free temp storage */
	zfree_random(p);
	zfree_random(q);

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
		randomfree(&blum);
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
		randomfree(&blum);
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
			return; /* skip need satisfied */
		}
	}

	/*
	 * skip loglogn bits at a time
	 */
	while (cnt >= loglogn) {

		/* turn the Blum-Blum-Shub crank */
		zsquaremod(blum.r, blum.n, &new_r);
		zfree_random(blum.r);
		blum.r = new_r;
		cnt -= blum.loglogn;
	}

	/*
	 * skip the final bits
	 */
	if (cnt > 0) {

		/* turn the Blum-Blum-Shub crank */
		zsquaremod(blum.r, blum.n, &new_r);
		zfree_random(blum.r);
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
		randomfree(&blum);
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
		zfree_random(blum.r);
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
	zfree_random(blum.r);
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
 * zrandomrange - generate a Blum-Blum-Shub random value in [low, beyond)
 *
 * given:
 *	low - low value of range
 *	beyond - beyond end of range
 *	res - where to place the random bits as ZVALUE
 */
void
zrandomrange(CONST ZVALUE low, CONST ZVALUE beyond, ZVALUE *res)
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
		zfree_random(range);
		zcopy(low, res);
		return;
	}
	zsub(range, _one_, &rangem1);
	bitlen = 1+zhighbit(rangem1);
	zfree_random(rangem1);

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
			zfree_random(rval);
		}
		zrandom(bitlen, &rval);
	} while (zrel(rval, range) >= 0);

	/*
	 * add in low value to produce the range [0+low, diff+low)
	 * which is the range [low, beyond)
	 */
	zadd(rval, low, res);
	zfree_random(rval);
	zfree_random(range);
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
	zfree_random(z1);
	zfree_random(z2);
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
		if (state->r.v == h_rdefvec) {
			ret->r.v = state->r.v;
		} else {
			zcopy(state->r, &ret->r);
		}
	}
	if (state->n.v == NULL) {
		ret->n.v = NULL;
	} else {
		if (state->n.v == h_ndefvec) {
			ret->n.v = state->n.v;
		} else {
			zcopy(state->n, &ret->n);
		}
	}

	/*
	 * return copy
	 */
	return ret;
}


/*
 * randomfree - free a Blum state
 *
 * We avoid freeing the pre-compiled states as they were
 * never malloced in the first place.
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
	zfree_random(state->n);
	zfree_random(state->r);

	/* free it if it is not pre-defined */
	state->seeded = 0;
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
randomprint(CONST RANDOM *UNUSED(state), int UNUSED(flags))
{
	math_str("RANDOM state");
}


/*
 * random_libcalc_cleanup - cleanup code for final libcalc_call_me_last() call
 *
 * This call is needed only by libcalc_call_me_last() to help clean up any
 * unneeded storage.
 *
 * Do not call this function directly!	Let libcalc_call_me_last() do it.
 */
void
random_libcalc_cleanup(void)
{
	/* free our state - let zfree_random protect the default state */
	randomfree(&blum);
	return;
}


/*
 * zfree_random - perform a zfree if we are not trying to free static data
 *
 * given:
 *	z	the ZVALUE to zfree(z) if not pointing to static data
 */
S_FUNC void
zfree_random(ZVALUE z)
{
	if (z.v != NULL &&
	    z.v != h_ndefvec && z.v != h_rdefvec && z.v != h_rdefvec_2 &&
	    z.v != h_nvec01 && z.v != h_rvec01 &&
	    z.v != h_nvec02 && z.v != h_rvec02 &&
	    z.v != h_nvec03 && z.v != h_rvec03 &&
	    z.v != h_nvec04 && z.v != h_rvec04 &&
	    z.v != h_nvec05 && z.v != h_rvec05 &&
	    z.v != h_nvec06 && z.v != h_rvec06 &&
	    z.v != h_nvec07 && z.v != h_rvec07 &&
	    z.v != h_nvec08 && z.v != h_rvec08 &&
	    z.v != h_nvec09 && z.v != h_rvec09 &&
	    z.v != h_nvec10 && z.v != h_rvec10 &&
	    z.v != h_nvec11 && z.v != h_rvec11 &&
	    z.v != h_nvec12 && z.v != h_rvec12 &&
	    z.v != h_nvec13 && z.v != h_rvec13 &&
	    z.v != h_nvec14 && z.v != h_rvec14 &&
	    z.v != h_nvec15 && z.v != h_rvec15 &&
	    z.v != h_nvec16 && z.v != h_rvec16 &&
	    z.v != h_nvec17 && z.v != h_rvec17 &&
	    z.v != h_nvec18 && z.v != h_rvec18 &&
	    z.v != h_nvec19 && z.v != h_rvec19 &&
	    z.v != h_nvec20 && z.v != h_rvec20) {
		zfree(z);
	}
	return;
}
