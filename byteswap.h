/*
 * byteswap - byte swapping macros
 *
 * Copyright (C) 1999,2014,2021,2026  Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:   1995/10/11 04:44:01
 * File existed as early as:    1995
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_BYTESWAP_H)
#  define INCLUDE_BYTESWAP_H

/*
 * SWAP_B8_IN_B16 - swap 8 bits in 16 bits
 *
 *      dest    - pointer to where the swapped src will be put
 *      src     - pointer to a 16 bit value to swap
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) a 16 bit value.
 */
#  define SWAP_B8_IN_B16(dest, src) (*((uint16_t *)(dest)) = (((*((uint16_t *)(src))) << 8) | ((*((uint16_t *)(src))) >> 8)))

/*
 * SWAP_B16_IN_B32 - swap 16 bits in 32 bits
 *
 *      dest    - pointer to where the swapped src will be put
 *      src     - pointer to a 32 bit value to swap
 */
#  define SWAP_B16_IN_B32(dest, src) (*((uint32_t *)(dest)) = (((*((uint32_t *)(src))) << 16) | ((*((uint32_t *)(src))) >> 16)))

/*
 * SWAP_B8_IN_B32 - swap 8 & 16 bits in 32 bits
 *
 *      dest    - pointer to where the swapped src will be put
 *      src     - pointer to a 32 bit value to swap
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) a 32 bit value.
 */
#  define SWAP_B8_IN_B32(dest, src)                                                                            \
      (SWAP_B16_IN_B32(dest, src), (*((uint32_t *)(dest)) = ((((*((uint32_t *)(dest))) & (uint32_t)0xff00ff00) >> 8) |  \
                                                             (((*((uint32_t *)(dest))) & (uint32_t)0x00ff00ff) << 8))))

/*
 * SWAP_B32_IN_B64 - swap 32 bits in 64 bits
 *
 *      dest    - pointer to where the swapped src will be put
 *      src     - pointer to a 64 bit value to swap
 */
#  define SWAP_B32_IN_B64(dest, src) (*((uint64_t *)(dest)) = (((*((uint64_t *)(src))) << 32) | ((*((uint64_t *)(src))) >> 32)))

/*
 * SWAP_B16_IN_B64 - swap 16 & 32 bits in 64 bits
 *
 *      dest    - pointer to where the swapped src will be put
 *      src     - pointer to a 64 bit value to swap
 */
#  define SWAP_B16_IN_B64(dest, src)                                                                                    \
        (SWAP_B32_IN_B64(dest, src), (*((uint64_t *)(dest)) = ((((*((uint64_t *)(dest))) & (uint64_t)0xffff0000ffff0000) >> 16) |  \
                                                               (((*((uint64_t *)(dest))) & (uint64_t)0x0000ffff0000ffff) << 16))))

/*
 * SWAP_B8_IN_B64 - swap 16 & 32 bits in 64 bits
 *
 *      dest    - pointer to where the swapped src will be put
 *      src     - pointer to a 64 bit value to swap
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 * Little Endian) a 64 bit value.
 */
#  define SWAP_B8_IN_B64(dest, src)                                                                                    \
        (SWAP_B16_IN_B64(dest, src), (*((uint64_t *)(dest)) = ((((*((uint64_t *)(dest))) & (uint64_t)0xff00ff00ff00ff00) >> 8) |  \
                                                               (((*((uint64_t *)(dest))) & (uint64_t)0x00ff00ff00ff00ff) << 8))))

#  if LONG_BITS == 64

#    define SWAP_B32_IN_LONG(dest, src) SWAP_B32_IN_B64(dest, src)
#    define SWAP_B16_IN_LONG(dest, src) SWAP_B16_IN_B64(dest, src)
#    define SWAP_B8_IN_LONG(dest, src) SWAP_B8_IN_B64(dest, src)

#  else

#    define SWAP_B32_IN_LONG(dest, src) SWAP_B32_IN_B32(dest, src)
#    define SWAP_B16_IN_LONG(dest, src) SWAP_B16_IN_B32(dest, src)
#    define SWAP_B8_IN_LONG(dest, src) SWAP_B8_IN_B32(dest, src)

#  endif

#endif
