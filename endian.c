/*
 * endian - determine the byte order of your machine
 *
 * If CALC_BYTE_ORDER is defined:
 *
 *       if CALC_BYTE_ORDER is CALC_LITTLE_ENDIAN, then the byte order will be Little Endian, else
 *       if CALC_BYTE_ORDER is CALC_BIG_ENDIAN, then the byte order will be Big Endian, else
 *       a compile error will be thrown.
 *
 * If CALC_BYTE_ORDER is NOT defiled:
 *
 *       This code examine unsigned byte values in static memory and set CALC_BYTE_ORDER accordingly.
 *
 * If you get syntax errors when you compile `endian.c`, then
 * try this to see what might be happening, and fix accordingly:
 *
 *    rm -f endian_calc.h && make endian_calc.h CALC_BYTE_ORDER=-DCALC_ENDIAN_DEBUG Q= S=
 *
 * Copyright (C) 1999-2013,2021,2026  Landon Curt Noll
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
 * Under source code control:   1993/11/15 04:32:58
 * File existed as early as:    1993
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Little Endian 1234:        Vax, 32k, Spim (Dec Mips), i386, i486, i86_64, arm, arm64, ...
 * Big Endian 4321:           IBM, Amdahl, 68k, Pyramid, Mips, Sparc, ...
 *
 * NOTE: calc does NOT (yet?) support pdp Endian.
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/*
 * calc local src includes
 */
#include "banned.h" /* include after all other includes */

/*
 * calc endian defines
 */
#undef CALC_LITTLE_ENDIAN
#define CALC_LITTLE_ENDIAN 1234
#undef CALC_BIG_ENDIAN
#define CALC_BIG_ENDIAN 4321
#undef CALC_PDP_ENDIAN
#define CALC_PDP_ENDIAN 3412

/*
 * byte order array
 *
 * If CALC_BYTE_ORDER is NOT defined, this static array of bytes will help determine the byte order at runtime.
 */
#if !defined(CALC_BYTE_ORDER)

static uint8_t byte[16] = {
    (uint8_t)0x12, (uint8_t)0x36, (uint8_t)0x48, (uint8_t)0x59,
    (uint8_t)0x01, (uint8_t)0x23, (uint8_t)0x45, (uint8_t)0x67,
    (uint8_t)0x9a, (uint8_t)0xbe, (uint8_t)0xc0, (uint8_t)0xd1,
    (uint8_t)0x89, (uint8_t)0xab, (uint8_t)0xcd, (uint8_t)0xef,
};

#  undef CALC_LITTLE_MAGIC_0
#  define CALC_LITTLE_MAGIC_0 0x59483612
#  undef CALC_LITTLE_MAGIC_1
#  define CALC_LITTLE_MAGIC_1 0x67452301
#  undef CALC_LITTLE_MAGIC_2
#  define CALC_LITTLE_MAGIC_2 0xd1c0be9a
#  undef CALC_LITTLE_MAGIC_3
#  define CALC_LITTLE_MAGIC_3 0xefcdab89

#  undef CALC_BIG_MAGIC_0
#  define CALC_BIG_MAGIC_0 0x12364859
#  undef CALC_BIG_MAGIC_1
#  define CALC_BIG_MAGIC_1 0x01234567
#  undef CALC_BIG_MAGIC_2
#  define CALC_BIG_MAGIC_2 0x9abec0d1
#  undef CALC_BIG_MAGIC_3
#  define CALC_BIG_MAGIC_3 0x89abcdef

#  undef CALC_PDP_MAGIC_0
#  define CALC_PDP_MAGIC_0 0x48591236
#  undef CALC_PDP_MAGIC_1
#  define CALC_PDP_MAGIC_1 0x45670123
#  undef CALC_PDP_MAGIC_2
#  define CALC_PDP_MAGIC_2 0xc0d19abe
#  undef CALC_PDP_MAGIC_3
#  define CALC_PDP_MAGIC_3 0xcdef89ab

#endif

int
main(void)
{
    /*
     * print calc endian defines
     */
     printf("#undef CALC_LITTLE_ENDIAN\n");
     printf("#define CALC_LITTLE_ENDIAN 1234\n");
     printf("#undef CALC_BIG_ENDIAN\n");
     printf("#define CALC_BIG_ENDIAN 4321\n");
     printf("#undef CALC_PDP_ENDIAN\n");
     printf("#define CALC_PDP_ENDIAN 3412 /* calc does NOT (yet?) support pdp Endian */\n");
     putchar('\n');

#if defined(CALC_BYTE_ORDER)

#  if CALC_BYTE_ORDER == CALC_LITTLE_ENDIAN

    printf("#define CALC_BYTE_ORDER    CALC_LITTLE_ENDIAN    "
           "/* CALC_BYTE_ORDER was pre-defined as CALC_LITTLE_ENDIAN */\n");

#  elif CALC_BYTE_ORDER == CALC_BIG_ENDIAN

    printf("#define CALC_BYTE_ORDER    CALC_BIG_ENDIAN    "
           "/* CALC_BYTE_ORDER was pre-defined as CALC_BIG_ENDIAN */\n");

#  elif CALC_BYTE_ORDER == CALC_PDP_ENDIAN

#    error "calc does NOT (yet) support pdp Endian"

#  else

#    error "CALC_BYTE_ORDER was defined but was not CALC_LITTLE_ENDIAN, nor CALC_BIG_ENDIAN, nor CALC_PDP_ENDIAN"

#  endif

#else

    uint32_t *uint32 = (uint32_t *)byte;        /* pointers into the byte order array */

#  if defined(CALC_ENDIAN_DEBUG)

    uint16_t *uint16 = (uint16_t *)byte;
    uint64_t *uint64 = (uint64_t *)byte;

    printf("/* Debug: uint8_t: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x "
                              "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x */\n",
           byte[0], byte[1], byte[2],  byte[3],  byte[4],  byte[5],  byte[6],  byte[7],
           byte[8], byte[9], byte[10], byte[11], byte[12], byte[13], byte[14], byte[15]);

    printf("/* Debug: uint16_t: 0x%04x 0x%04x 0x%04x 0x%04x "
                               "0x%04x 0x%04x 0x%04x 0x%04x */\n",
           uint16[0], uint16[1], uint16[2], uint16[3],
           uint16[4], uint16[5], uint16[6], uint16[7]);

    printf("/* Debug: uint32_t: 0x%08x 0x%08x "
                               "0x%08x 0x%08x */\n",
           uint32[0], uint32[1],
           uint32[2], uint32[3]);

    printf("/* Debug: uint64_t: 0x%016zx "
                               "0x%016zx */\n",
           uint64[0],
           uint64[1]);

#  endif

    /*
     * Determine byte order
     */
    if (uint32[0] == CALC_LITTLE_MAGIC_0) {

#  if defined(CALC_ENDIAN_DEBUG)
        if (uint32[1] != CALC_LITTLE_MAGIC_1) {
            printf("/* WARNING: uint32[1]: 0x%08x != CALC_LITTLE_MAGIC_1: 0x%08x */\n", uint32[1], CALC_LITTLE_MAGIC_1);
        }
        if (uint32[2] != CALC_LITTLE_MAGIC_2) {
            printf("/* WARNING: uint32[2]: 0x%08x != CALC_LITTLE_MAGIC_2: 0x%08x */\n", uint32[2], CALC_LITTLE_MAGIC_2);
        }
        if (uint32[3] != CALC_LITTLE_MAGIC_3) {
            printf("/* WARNING: uint32[3]: 0x%08x != CALC_LITTLE_MAGIC_3: 0x%08x */\n", uint32[3], CALC_LITTLE_MAGIC_3);
        }
        putchar('\n');
#  endif
        /* Least Significant Byte first */
        printf("#define CALC_BYTE_ORDER    CALC_LITTLE_ENDIAN /* CALC_BYTE_ORDER computed to be Little Endian */\n");

    } else if (uint32[0] == CALC_BIG_MAGIC_0) {

#  if defined(CALC_ENDIAN_DEBUG)
        if (uint32[1] != CALC_BIG_MAGIC_1) {
            printf("/* WARNING: uint32[1]: 0x%08x != CALC_BIG_MAGIC_1: 0x%08x */\n", uint32[1], CALC_BIG_MAGIC_1);
        }
        if (uint32[2] != CALC_BIG_MAGIC_2) {
            printf("/* WARNING: uint32[2]: 0x%08x != CALC_BIG_MAGIC_2: 0x%08x */\n", uint32[2], CALC_BIG_MAGIC_2);
        }
        if (uint32[3] != CALC_BIG_MAGIC_3) {
            printf("/* WARNING: uint32[3]: 0x%08x != CALC_BIG_MAGIC_3: 0x%08x */\n", uint32[3], CALC_BIG_MAGIC_3);
        }
        putchar('\n');
#  endif

        /* Most Significant Byte first */
        printf("#define CALC_BYTE_ORDER    CALC_BIG_ENDIAN /* CALC_BYTE_ORDER computed to be Big Endian */\n");

    } else if (uint32[0] == CALC_PDP_MAGIC_0) {

#  if defined(CALC_ENDIAN_DEBUG)
        if (uint32[1] != CALC_PDP_MAGIC_1) {
            printf("/* WARNING: uint32[1]: 0x%08x != CALC_PDP_MAGIC_1: 0x%08x */\n", uint32[1], CALC_PDP_MAGIC_1);
        }
        if (uint32[2] != CALC_PDP_MAGIC_2) {
            printf("/* WARNING: uint32[2]: 0x%08x != CALC_PDP_MAGIC_2: 0x%08x */\n", uint32[2], CALC_PDP_MAGIC_2);
        }
        if (uint32[3] != CALC_PDP_MAGIC_3) {
            printf("/* WARNING: uint32[3]: 0x%08x != CALC_PDP_MAGIC_3: 0x%08x */\n", uint32[3], CALC_PDP_MAGIC_3);
        }
        putchar('\n');
#  endif

        /* Most Significant Byte first */
        printf("#error \"calc does NOT (yet?) support pdp Endian\"\n");

    } else {

        printf("#error \"cannot determine byte order, set ${CALC_BYTE_ORDER} in the Makefile.config\"\n");
        exit(1);

    }
#endif
    exit(0);
}
