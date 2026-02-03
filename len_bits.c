/*
 * len_bits - Determine the byte and bit lengths of various types
 *
 * We need LONG_BITS to be an integer, instead defined as sizeof(long)*CHAR_BIT
 * because we want the cpp to be able to perform #if comparison on the value.
 *
 * Copyright (C) 1999-2007,2014,2021,2023,2026  Landon Curt Noll
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
 * Under source code control:   1994/03/18 03:06:18
 * File existed as early as:    1994
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>

/*
 * calc local src includes
 */
#include "endian_calc.h"

#include "banned.h" /* include after all other includes */

int
main(void)
{
    size_t off_t_len = sizeof(off_t);
    size_t off_t_bits = off_t_len * CHAR_BIT;
#if !defined(OFF_T_MAX)
    off_t off_t_max = ((((off_t)1 << (off_t_bits - 2)) - 1) * 2 + 1);
#endif
    size_t dev_t_len = sizeof(dev_t);
    size_t dev_t_bits = dev_t_len * CHAR_BIT;
    size_t ino_t_len = sizeof(ino_t);
    size_t ino_t_bits = ino_t_len * CHAR_BIT;

    /*
     * report size of a long
     */
    printf("#undef LONG_BITS\n");
    printf("#define LONG_BITS %zu\t\t/%s/\n", sizeof(long) * CHAR_BIT, "* bit length of a long *");
    putchar('\n');

    /*
     * report size of an off_t
     */
    printf("#undef OFF_T_LEN\n");
    printf("#define OFF_T_LEN %zu\t\t/%s/\n", off_t_len, "* byte length of an off_t *");
    printf("#undef OFF_T_BITS\n");
    printf("#define OFF_T_BITS %zu\t\t/%s/\n", off_t_bits, "* bit length of an off_t *");

    putchar('\n');
#if !defined(OFF_T_MAX)
    printf("#undef OFF_T_MAX\n");
    printf("#define OFF_T_MAX %lldLL\t\t/%s/\n", (long long)off_t_max, "* maximum off_t *");
    putchar('\n');
#endif

#if !defined(OFF_T_MIN)
    printf("#undef OFF_T_MIN\n");
    /*
     * instead of:
     *
     *      #define OFF_T_MIN (-OFF_T_MAX - 1LL)
     *
     * we use:
     *      #define OFF_T_MIN (-OFF_T_MAX)
     *
     * because there is just too much hassle in dealing with -OFF_T_MAX - 1LL
     * when it comes to negation of that value.
     */
    printf("/* we set OFF_T_MIN to one more than the minimum off_t to make it easier to negate */\n");
    printf("#define OFF_T_MIN (-OFF_T_MAX)\t\t/%s/\n", "* one more than the minimum off_t *");
    putchar('\n');
#endif

#  if CALC_BYTE_ORDER == BIG_ENDIAN
    if (off_t_bits == 64) {
        printf("#define SWAP_HALF_IN_OFF_T(dest, src)\t\t%s\n", "SWAP_HALF_IN_B64(dest, src)\n");
    } else if (off_t_bits == 32) {
        printf("#define SWAP_HALF_IN_OFF_T(dest, src)\t\t%s\n", "SWAP_HALF_IN_B32(dest, src)\n");
    } else {
        printf("#error \"off_t_bits: %d is neither 64, nor 32, "
               "do not (yet) know how to form SWAP_HALF_IN_OFF_T macro for that size\"\n",
               off_t_bits);
    }
#  else
    printf("#define SWAP_HALF_IN_OFF_T(dest, src)\t\t%s\n", "(*(dest) = *(src))");
#  endif
    putchar('\n');

    /*
     * report size of an dev_t
     */
    printf("#undef DEV_T_LEN\n");
    printf("#define DEV_T_LEN %zu\t\t/%s/\n", dev_t_len, "* byte length of an dev_t *");
    printf("#undef DEV_T_BITS\n");
    printf("#define DEV_T_BITS %zu\t\t/%s/\n", dev_t_bits, "* bit length of an dev_t *");
    putchar('\n');

#  if CALC_BYTE_ORDER == BIG_ENDIAN
    if (dev_t_bits == 64) {
        printf("#define SWAP_HALF_IN_DEV_T(dest, src)\t\t%s\n", "SWAP_HALF_IN_B64(dest, src)\n");
    } else if (dev_t_bits == 32) {
        printf("#define SWAP_HALF_IN_DEV_T(dest, src)\t\t%s\n", "SWAP_HALF_IN_B32(dest, src)\n");
    } else if (dev_t_bits == 16) {
        printf("#define SWAP_HALF_IN_DEV_T(dest, src)\t\t%s\n", "SWAP_HALF_IN_B16(dest, src)\n");
    } else {
        printf("#error \"off_t_bits: %d is neither 64, nor 32, nor 16, "
               "do not (yet) know how to form SWAP_HALF_IN_DEV_T macro for that size\"\n",
               off_t_bits);
    }
#  else
    printf("#define SWAP_HALF_IN_DEV_T(dest, src)\t\t%s\n", "(*(dest) = *(src))");
#  endif
    putchar('\n');

    /*
     * report size of an ino_t
     */
    printf("#undef INO_T_LEN\n");
    printf("#define INO_T_LEN %zu\t\t/%s/\n", ino_t_len, "* byte length of an ino_t *");
    printf("#undef INO_T_BITS\n");
    printf("#define INO_T_BITS %zu\t\t/%s/\n", ino_t_bits, "* bit length of an ino_t *");
    putchar('\n');

#  if CALC_BYTE_ORDER == BIG_ENDIAN
    if (ino_t_bits == 64) {
        printf("#define SWAP_HALF_IN_INO_T(dest, src)\t\t%s\n", "SWAP_HALF_IN_B64(dest, src)\n");
    } else if (ino_t_bits == 32) {
        printf("#define SWAP_HALF_IN_INO_T(dest, src)\t\t%s\n", "SWAP_HALF_IN_B32(dest, src)\n");
    } else if (ino_t_bits == 16) {
        printf("#define SWAP_HALF_IN_INO_T(dest, src)\t\t%s\n", "SWAP_HALF_IN_B16(dest, src)\n");
    } else {
        printf("#error \"off_t_bits: %d is neither 64, nor 32, nor 16,"
               "do not (yet) know how to form SWAP_HALF_IN_INO_T macro for that size\"\n",
               off_t_bits);
    }
#  else
    printf("#define SWAP_HALF_IN_INO_T(dest, src)\t\t%s\n", "(*(dest) = *(src))");
#  endif

    /*
     * All Done!!! -- Jessica Noll, Age 2
     */
    exit(0); /*ooo*/
}
