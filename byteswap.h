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

#if !defined(BYTESWAP_H)
#define BYTESWAP_H

#include "longbits.h"


/*
 * SWAP_B8_IN_B16 - swap 8 bits in 16 bits
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 16 bit value to swap
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) a 16 bit value.
 */
#define SWAP_B8_IN_B16(dest, src) (					\
	*((USB16*)(dest)) =						\
	  (((*((USB16*)(src))) << 8) | ((*((USB16*)(src))) >> 8))	\
)

/*
 * SWAP_B16_IN_B32 - swap 16 bits in 32 bits
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 32 bit value to swap
 */
#define SWAP_B16_IN_B32(dest, src) (					\
	*((USB32*)(dest)) =						\
	  (((*((USB32*)(src))) << 16) | ((*((USB32*)(src))) >> 16))	\
)

/*
 * SWAP_B8_IN_B32 - swap 8 & 16 bits in 32 bits
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 32 bit value to swap
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) a 32 bit value.
 */
#define SWAP_B8_IN_B32(dest, src) (					\
	SWAP_B16_IN_B32(dest, src),					\
	(*((USB32*)(dest)) =						\
	  ((((*((USB32*)(dest))) & (USB32)0xff00ff00) >> 8) |		\
	  (((*((USB32*)(dest))) & (USB32)0x00ff00ff) << 8)))		\
)

#if defined(HAVE_B64)

/*
 * SWAP_B32_IN_B64 - swap 32 bits in 64 bits
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 64 bit value to swap
 */
#define SWAP_B32_IN_B64(dest, src) (					\
	*((USB64*)(dest)) =						\
	  (((*((USB64*)(src))) << 32) | ((*((USB64*)(src))) >> 32))	\
)

/*
 * SWAP_B16_IN_B64 - swap 16 & 32 bits in 64 bits
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 64 bit value to swap
 */
#define SWAP_B16_IN_B64(dest, src) (					\
	SWAP_B32_IN_B64(dest, src),					\
	(*((USB64*)(dest)) =						\
	  ((((*((USB64*)(dest))) & (USB64)0xffff0000ffff0000) >> 16) |	\
	  (((*((USB64*)(dest))) & (USB64)0x0000ffff0000ffff) << 16)))	\
)

/*
 * SWAP_B8_IN_B64 - swap 16 & 32 bits in 64 bits
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 64 bit value to swap
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) a 64 bit value.
 */
#define SWAP_B8_IN_B64(dest, src) (					\
	SWAP_B16_IN_B64(dest, src),					\
	(*((USB64*)(dest)) =						\
	  ((((*((USB64*)(dest))) & (USB64)0xff00ff00ff00ff00) >> 8) |	\
	  (((*((USB64*)(dest))) & (USB64)0x00ff00ff00ff00ff) << 8)))	\
)

#else /* HAVE_B64 */

/*
 * SWAP_B32_IN_B64 - swap 32 bits in 64 bits (simulated by 2 32 bit values)
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 64 bit value to swap
 */
#define SWAP_B32_IN_B64(dest, src) (					\
	((USB32*)(dest))[1] = ((USB32*)(dest))[0],			\
	((USB32*)(dest))[0] = ((USB32*)(dest))[1]			\
)

/*
 * SWAP_B16_IN_B64 - swap 16 & 32 bits in 64 bits (simulated by 2 32 bit vals)
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 64 bit value to swap
 */
#define SWAP_B16_IN_B64(dest, src) (					\
	SWAP_B16_IN_B32(((USB32*)dest)+1, ((USB32*)src)),		\
	SWAP_B16_IN_B32(((USB32*)dest), ((USB32*)src)+1)		\
)

/*
 * SWAP_B8_IN_B64 - swap 16 & 32 bits in 64 bits (simulated by 2 32 bit vals)
 *
 *	dest	- pointer to where the swapped src wil be put
 *	src	- pointer to a 64 bit value to swap
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) a 64 bit value.
 */
#define SWAP_B8_IN_B64(dest, src) (					\
	SWAP_B8_IN_B32(((USB32*)dest)+1, ((USB32*)src)),		\
	SWAP_B8_IN_B32(((USB32*)dest), ((USB32*)src)+1)			\
)

#endif /* HAVE_B64 */

#if LONG_BITS == 64

#define SWAP_B32_IN_LONG(dest, src)	SWAP_B32_IN_B64(dest, src)
#define SWAP_B16_IN_LONG(dest, src)	SWAP_B16_IN_B64(dest, src)
#define SWAP_B8_IN_LONG(dest, src)	SWAP_B8_IN_B64(dest, src)

#else /* LONG_BITS == 64 */

#define SWAP_B32_IN_LONG(dest, src)	SWAP_B32_IN_B32(dest, src)
#define SWAP_B16_IN_LONG(dest, src)	SWAP_B16_IN_B32(dest, src)
#define SWAP_B8_IN_LONG(dest, src)	SWAP_B8_IN_B32(dest, src)

#endif /* LONG_BITS == 64 */

#endif /* !BYTESWAP_H */
