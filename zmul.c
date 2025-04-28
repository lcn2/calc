/*
 * zmul - faster than usual multiplying and squaring routines
 *
 * Copyright (C) 1999-2007,2021-2023  David I. Bell
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
 * Under source code control:   1991/01/09 20:01:31
 * File existed as early as:    1991
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Faster than usual multiplying and squaring routines.
 * The algorithm used is the reasonably simple one from Knuth, volume 2,
 * section 4.3.3.  These recursive routines are of speed O(N^1.585)
 * instead of O(N^2).  The usual multiplication and (almost usual) squaring
 * algorithms are used for small numbers.  On a 386 with its compiler, the
 * two algorithms are equal in speed at about 100 decimal digits.
 */


#include "config.h"
#include "zmath.h"


#include "errtbl.h"
#include "banned.h"     /* include after system header <> includes */


STATIC HALF *tempbuf;           /* temporary buffer for multiply and square */

S_FUNC LEN domul(HALF *v1, LEN size1, HALF *v2, LEN size2, HALF *ans);
S_FUNC LEN dosquare(HALF *vp, LEN size, HALF *ans);


/*
 * Multiply two numbers using the following formula recursively:
 *      (A*S+B)*(C*S+D) = (S^2+S)*A*C + S*(A-B)*(D-C) + (S+1)*B*D
 * where S is a power of 2^16, and so multiplies by it are shifts, and
 * A,B,C,D are the left and right HALFs of the numbers to be multiplied.
 *
 * given:
 *      z1              numbers to multiply
 *      z2              numbers to multiply
 *      res             result of multiplication
 */
void
zmul(ZVALUE z1, ZVALUE z2, ZVALUE *res)
{
        LEN len;                /* size of array */

        if (ziszero(z1) || ziszero(z2)) {
                *res = _zero_;
                return;
        }
        if (zisunit(z1)) {
                zcopy(z2, res);
                res->sign = (z1.sign != z2.sign);
                return;
        }
        if (zisunit(z2)) {
                zcopy(z1, res);
                res->sign = (z1.sign != z2.sign);
                return;
        }

        /*
         * Allocate a temporary buffer for the recursion levels to use.
         * An array needs to be allocated large enough for all of the
         * temporary results to fit in.  This size is about twice the size
         * of the largest original number, since each recursion level uses
         * the size of its given number, and whose size is 1/2 the size of
         * the previous level.  The sum of the infinite series is 2.
         * Add some extra words because of rounding when dividing by 2
         * and also because of the extra word that each multiply needs.
         */
        len = z1.len;
        if (len < z2.len)
                len = z2.len;
        len = 2 * len + 64;
        tempbuf = zalloctemp(len);

        res->sign = (z1.sign != z2.sign);
        res->v = alloc(z1.len + z2.len + 2);
        res->len = domul(z1.v, z1.len, z2.v, z2.len, res->v);
}


/*
 * Recursive routine to multiply two numbers by splitting them up into
 * two numbers of half the size, and using the results of multiplying the
 * sub-pieces.  The result is placed in the indicated array, which must be
 * large enough for the result plus one extra word (size1 + size2 + 1).
 * Returns the actual size of the result with leading zeroes stripped.
 * This also uses a temporary array which must be twice as large as
 * one more than the size of the number at the top level recursive call.
 *
 * given:
 *      v1              first number
 *      size1           size of first number
 *      v2              second number
 *      size2           size of second number
 *      ans             location for result
 */
S_FUNC LEN
domul(HALF *v1, LEN size1, HALF *v2, LEN size2, HALF *ans)
{
        LEN shift;              /* amount numbers are shifted by */
        LEN sizeA;              /* size of left half of first number */
        LEN sizeB;              /* size of right half of first number */
        LEN sizeC;              /* size of left half of second number */
        LEN sizeD;              /* size of right half of second number */
        LEN sizeAB;             /* size of subtraction of A and B */
        LEN sizeDC;             /* size of subtraction of D and C */
        LEN sizeABDC;           /* size of product of above two results */
        LEN subsize;            /* size of difference of HALFs */
        LEN copysize;           /* size of number left to copy */
        LEN sizetotal;          /* total size of product */
        LEN len;                /* temporary length */
        HALF *baseA;            /* base of left half of first number */
        HALF *baseB;            /* base of right half of first number */
        HALF *baseC;            /* base of left half of second number */
        HALF *baseD;            /* base of right half of second number */
        HALF *baseAB;           /* base of result of subtraction of A and B */
        HALF *baseDC;           /* base of result of subtraction of D and C */
        HALF *baseABDC;         /* base of product of above two results */
        HALF *baseAC;           /* base of product of A and C */
        HALF *baseBD;           /* base of product of B and D */
        FULL carry;             /* carry digit for small multiplications */
        FULL carryACBD;         /* carry from addition of A*C and B*D */
        FULL digit;             /* single digit multiplying by */
        HALF *temp;             /* base for temporary calculations */
        bool neg;               /* whether intermediate term is negative */
        register HALF *hd, *h1=NULL, *h2=NULL;  /* for inner loops */
        SIUNION sival;          /* for addition of digits */

        /* firewall */
        if (v1 == NULL) {
                math_error("%s: v1 NULL", __func__);
                not_reached();
        }
        if (ans == NULL) {
                math_error("%s: ans NULL", __func__);
                not_reached();
        }

        /*
         * Trim the numbers of leading zeroes and initialize the
         * estimated size of the result.
         */
        hd = &v1[size1 - 1];
        while ((*hd == 0) && (size1 > 1)) {
                hd--;
                size1--;
        }
        hd = &v2[size2 - 1];
        while ((*hd == 0) && (size2 > 1)) {
                hd--;
                size2--;
        }
        sizetotal = size1 + size2;

        /*
         * First check for zero answer.
         */
        if (((size1 == 1) && (*v1 == 0)) || ((size2 == 1) && (*v2 == 0))) {
                *ans = 0;
                return 1;
        }

        /*
         * Exchange the two numbers if necessary to make the number of
         * digits of the first number be greater than or equal to the
         * second number.
         */
        if (size1 < size2) {
                len = size1; size1 = size2; size2 = len;
                hd = v1; v1 = v2; v2 = hd;
        }

        /*
         * If the smaller number has only a few digits, then calculate
         * the result in the normal manner in order to avoid the overhead
         * of the recursion for small numbers.  The number of digits where
         * the algorithm changes is settable from 2 to maxint.
         */
        if (size2 < conf->mul2) {
                /*
                 * First clear the top part of the result, and then multiply
                 * by the lowest digit to get the first partial sum.  Later
                 * products will then add into this result.
                 */
                hd = &ans[size1];
                len = size2;
                while (len--)
                        *hd++ = 0;

                digit = *v2++;
                h1 = v1;
                hd = ans;
                carry = 0;
                len = size1;
                while (len >= 4) {      /* expand the loop some */
                        len -= 4;
                        sival.ivalue = ((FULL) *h1++) * digit + carry;
                        /* ignore Saber-C warning #112 - get ushort from uint */
                        /*        OK to ignore on name domul`sival */
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                        sival.ivalue = ((FULL) *h1++) * digit + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                        sival.ivalue = ((FULL) *h1++) * digit + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                        sival.ivalue = ((FULL) *h1++) * digit + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }
                while (len--) {
                        sival.ivalue = ((FULL) *h1++) * digit + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }
                *hd = (HALF)carry;

                /*
                 * Now multiply by the remaining digits of the second number,
                 * adding each product into the final result.
                 */
                h2 = ans;
                while (--size2 > 0) {
                        digit = *v2++;
                        h1 = v1;
                        hd = ++h2;
                        if (digit == 0)
                                continue;
                        carry = 0;
                        len = size1;
                        while (len >= 4) {      /* expand the loop some */
                                len -= 4;
                                sival.ivalue = ((FULL) *h1++) * digit
                                        + ((FULL) *hd) + carry;
                                *hd++ = sival.silow;
                                carry = sival.sihigh;
                                sival.ivalue = ((FULL) *h1++) * digit
                                        + ((FULL) *hd) + carry;
                                *hd++ = sival.silow;
                                carry = sival.sihigh;
                                sival.ivalue = ((FULL) *h1++) * digit
                                        + ((FULL) *hd) + carry;
                                *hd++ = sival.silow;
                                carry = sival.sihigh;
                                sival.ivalue = ((FULL) *h1++) * digit
                                        + ((FULL) *hd) + carry;
                                *hd++ = sival.silow;
                                carry = sival.sihigh;
                        }
                        while (len--) {
                                sival.ivalue = ((FULL) *h1++) * digit
                                        + ((FULL) *hd) + carry;
                                *hd++ = sival.silow;
                                carry = sival.sihigh;
                        }
                        while (carry) {
                                sival.ivalue = ((FULL) *hd) + carry;
                                *hd++ = sival.silow;
                                carry = sival.sihigh;
                        }
                }

                /*
                 * Now return the true size of the number.
                 */
                len = sizetotal;
                hd = &ans[len - 1];
                while ((*hd == 0) && (len > 1)) {
                        hd--;
                        len--;
                }
                return len;
        }

        /*
         * Need to multiply by a large number.
         * Allocate temporary space for calculations, and calculate the
         * value for the shift.  The shift value is 1/2 the size of the
         * larger (first) number (rounded up).  The amount of temporary
         * space needed is twice the size of the shift, plus one more word
         * for the multiply to use.
         */
        shift = (size1 + 1) / 2;
        temp = tempbuf;
        tempbuf += (2 * shift) + 1;

        /*
         * Determine the sizes and locations of all the numbers.
         * The value of sizeC can be negative, and this is checked later.
         * The value of sizeD is limited by the full size of the number.
         */
        baseA = v1 + shift;
        baseB = v1;
        baseC = v2 + ((shift <= size2)  ? shift : size2);
        baseD = v2;
        baseAB = ans;
        baseDC = ans + shift;
        baseAC = ans + shift * 2;
        baseBD = ans;

        sizeA = size1 - shift;
        sizeC = size2 - shift;

        sizeB = shift;
        hd = &baseB[shift - 1];
        while ((*hd == 0) && (sizeB > 1)) {
                hd--;
                sizeB--;
        }

        sizeD = shift;
        if (sizeD > size2)
                sizeD = size2;
        hd = &baseD[sizeD - 1];
        while ((*hd == 0) && (sizeD > 1)) {
                hd--;
                sizeD--;
        }

        /*
         * If the smaller number has a high half of zero, then calculate
         * the result by breaking up the first number into two numbers
         * and combining the results using the obvious formula:
         *      (A*S+B) * D = (A*D)*S + B*D
         */
        if (sizeC <= 0) {
                len = domul(baseB, sizeB, baseD, sizeD, ans);
                hd = &ans[len];
                len = sizetotal - len;
                while (len--)
                        *hd++ = 0;

                /*
                 * Add the second number into the first number, shifted
                 * over at the correct position.
                 */
                len = domul(baseA, sizeA, baseD, sizeD, temp);
                h1 = temp;
                hd = ans + shift;
                carry = 0;
                while (len--) {
                        sival.ivalue = ((FULL) *h1++) + ((FULL) *hd) + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }
                while (carry) {
                        sival.ivalue = ((FULL) *hd) + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }

                /*
                 * Determine the final size of the number and return it.
                 */
                len = sizetotal;
                hd = &ans[len - 1];
                while ((*hd == 0) && (len > 1)) {
                        hd--;
                        len--;
                }
                tempbuf = temp;
                return len;
        }

        /*
         * Now we know that the high HALFs of the numbers are nonzero,
         * so we can use the complete formula.
         *      (A*S+B)*(C*S+D) = (S^2+S)*A*C + S*(A-B)*(D-C) + (S+1)*B*D.
         * The steps are done in the following order:
         *      A-B
         *      D-C
         *      (A-B)*(D-C)
         *      S^2*A*C + B*D
         *      (S^2+S)*A*C + (S+1)*B*D                         (*)
         *      (S^2+S)*A*C + S*(A-B)*(D-C) + (S+1)*B*D
         *
         * Note: step (*) above can produce a result which is larger than
         * the final product will be, and this is where the extra word
         * needed in the product comes from.  After the final subtraction is
         * done, the result fits in the expected size.  Using the extra word
         * is easier than suppressing the carries and borrows everywhere.
         *
         * Begin by forming the product (A-B)*(D-C) into a temporary
         * location that we save until the final step.  Do each subtraction
         * at positions 0 and S.  Be very careful about the relative sizes
         * of the numbers since this result can be negative.  For the first
         * step calculate the absolute difference of A and B into a temporary
         * location at position 0 of the result.  Negate the sign if A is
         * smaller than B.
         */
        neg = false;
        if (sizeA == sizeB) {
                len = sizeA;
                h1 = &baseA[len - 1];
                h2 = &baseB[len - 1];
                while ((len > 1) && (*h1 == *h2)) {
                        len--;
                        h1--;
                        h2--;
                }
        }
        if ((sizeA > sizeB) || ((sizeA == sizeB) && h1 && h2 && (*h1 > *h2))) {
                h1 = baseA;
                h2 = baseB;
                sizeAB = sizeA;
                subsize = sizeB;
        } else {
                neg = !neg;
                h1 = baseB;
                h2 = baseA;
                sizeAB = sizeB;
                subsize = sizeA;
        }
        copysize = sizeAB - subsize;

        hd = baseAB;
        carry = 0;
        while (subsize--) {
                sival.ivalue = BASE1 - ((FULL) *h1++) + ((FULL) *h2++) + carry;
                *hd++ = (HALF)(BASE1 - sival.silow);
                carry = sival.sihigh;
        }
        while (copysize--) {
                sival.ivalue = (BASE1 - ((FULL) *h1++)) + carry;
                *hd++ = (HALF)(BASE1 - sival.silow);
                carry = sival.sihigh;
        }

        hd = &baseAB[sizeAB - 1];
        while ((*hd == 0) && (sizeAB > 1)) {
                hd--;
                sizeAB--;
        }

        /*
         * This completes the calculation of abs(A-B).  For the next step
         * calculate the absolute difference of D and C into a temporary
         * location at position S of the result.  Negate the sign if C is
         * larger than D.
         */
        if (sizeC == sizeD) {
                len = sizeC;
                h1 = &baseC[len - 1];
                h2 = &baseD[len - 1];
                while ((len > 1) && (*h1 == *h2)) {
                        len--;
                        h1--;
                        h2--;
                }
        }
        if ((sizeC > sizeD) || ((sizeC == sizeD) && (*h1 > *h2))) {
                neg = !neg;
                h1 = baseC;
                h2 = baseD;
                sizeDC = sizeC;
                subsize = sizeD;
        } else {
                h1 = baseD;
                h2 = baseC;
                sizeDC = sizeD;
                subsize = sizeC;
        }
        copysize = sizeDC - subsize;

        hd = baseDC;
        carry = 0;
        while (subsize--) {
                sival.ivalue = BASE1 - ((FULL) *h1++) + ((FULL) *h2++) + carry;
                *hd++ = (HALF)(BASE1 - sival.silow);
                carry = sival.sihigh;
        }
        while (copysize--) {
                sival.ivalue = (BASE1 - ((FULL) *h1++)) + carry;
                *hd++ = (HALF)(BASE1 - sival.silow);
                carry = sival.sihigh;
        }
        hd = &baseDC[sizeDC - 1];
        while ((*hd == 0) && (sizeDC > 1)) {
                hd--;
                sizeDC--;
        }

        /*
         * This completes the calculation of abs(D-C).  Now multiply
         * together abs(A-B) and abs(D-C) into a temporary location,
         * which is preserved until the final steps.
         */
        baseABDC = temp;
        sizeABDC = domul(baseAB, sizeAB, baseDC, sizeDC, baseABDC);

        /*
         * Now calculate B*D and A*C into one of their two final locations.
         * Make sure the high order digits of the products are zeroed since
         * this initializes the final result.  Be careful about this zeroing
         * since the size of the high order words might be smaller than
         * the shift size.  Do B*D first since the multiplies use one more
         * word than the size of the product.  Also zero the final extra
         * word in the result for possible carries to use.
         */
        len = domul(baseB, sizeB, baseD, sizeD, baseBD);
        hd = &baseBD[len];
        len = shift * 2 - len;
        while (len--)
                *hd++ = 0;

        len = domul(baseA, sizeA, baseC, sizeC, baseAC);
        hd = &baseAC[len];
        len = sizetotal - shift * 2 - len + 1;
        while (len--)
                *hd++ = 0;

        /*
         * Now add in A*C and B*D into themselves at the other shifted
         * position that they need.  This addition is tricky in order to
         * make sure that the two additions cannot interfere with each other.
         * Therefore we first add in the top half of B*D and the lower half
         * of A*C.  The sources and destinations of these two additions
         * overlap, and so the same answer results from the two additions,
         * thus only two pointers suffice for both additions.  Keep the
         * final carry from these additions for later use since we cannot
         * afford to change the top half of A*C yet.
         */
        h1 = baseBD + shift;
        h2 = baseAC;
        carryACBD = 0;
        len = shift;
        while (len--) {
                sival.ivalue = ((FULL) *h1) + ((FULL) *h2) + carryACBD;
                *h1++ = sival.silow;
                *h2++ = sival.silow;
                carryACBD = sival.sihigh;
        }

        /*
         * Now add in the bottom half of B*D and the top half of A*C.
         * These additions are straightforward, except that A*C should
         * be done first because of possible carries from B*D, and the
         * top half of A*C might not exist.  Add in one of the carries
         * from the previous addition while we are at it.
         */
        h1 = baseAC + shift;
        hd = baseAC;
        carry = carryACBD;
        len = sizetotal - 3 * shift;
        while (len--) {
                sival.ivalue = ((FULL) *h1++) + ((FULL) *hd) + carry;
                *hd++ = sival.silow;
                carry = sival.sihigh;
        }
        while (carry) {
                sival.ivalue = ((FULL) *hd) + carry;
                *hd++ = sival.silow;
                carry = sival.sihigh;
        }

        h1 = baseBD;
        hd = baseBD + shift;
        carry = 0;
        len = shift;
        while (len--) {
                sival.ivalue = ((FULL) *h1++) + ((FULL) *hd) + carry;
                *hd++ = sival.silow;
                carry = sival.sihigh;
        }
        while (carry) {
                sival.ivalue = ((FULL) *hd) + carry;
                *hd++ = sival.silow;
                carry = sival.sihigh;
        }

        /*
         * Now finally add in the other delayed carry from the
         * above addition.
         */
        hd = baseAC + shift;
        while (carryACBD) {
                sival.ivalue = ((FULL) *hd) + carryACBD;
                *hd++ = sival.silow;
                carryACBD = sival.sihigh;
        }

        /*
         * Now finally add or subtract (A-B)*(D-C) into the final result at
         * the correct position (S), according to whether it is positive or
         * negative.  When subtracting, the answer cannot go negative.
         */
        h1 = baseABDC;
        hd = ans + shift;
        carry = 0;
        len = sizeABDC;
        if (neg) {
                while (len--) {
                        sival.ivalue = BASE1 - ((FULL) *hd) +
                                ((FULL) *h1++) + carry;
                        *hd++ = (HALF)(BASE1 - sival.silow);
                        carry = sival.sihigh;
                }
                while (carry) {
                        sival.ivalue = BASE1 - ((FULL) *hd) + carry;
                        *hd++ = (HALF)(BASE1 - sival.silow);
                        carry = sival.sihigh;
                }
        } else {
                while (len--) {
                        sival.ivalue = ((FULL) *h1++) + ((FULL) *hd) + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }
                while (carry) {
                        sival.ivalue = ((FULL) *hd) + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }
        }

        /*
         * Finally determine the size of the final result and return that.
         */
        len = sizetotal;
        hd = &ans[len - 1];
        while ((*hd == 0) && (len > 1)) {
                hd--;
                len--;
        }
        tempbuf = temp;
        return len;
}


/*
 * Square a number by using the following formula recursively:
 *      (A*S+B)^2 = (S^2+S)*A^2 + (S+1)*B^2 - S*(A-B)^2
 * where S is a power of 2^16, and so multiplies by it are shifts,
 * and A and B are the left and right HALFs of the number to square.
 */
void
zsquare(ZVALUE z, ZVALUE *res)
{
        LEN len;

        /* firewall */
        if (res == NULL) {
                math_error("%s: res NULL", __func__);
                not_reached();
        }

        if (ziszero(z)) {
                *res = _zero_;
                return;
        }
        if (zisunit(z)) {
                *res = _one_;
                return;
        }

        /*
         * Allocate a temporary array if necessary for the recursion to use.
         * The array needs to be allocated large enough for all of the
         * temporary results to fit in.  This size is about 3 times the
         * size of the original number, since each recursion level uses 3/2
         * of the size of its given number, and whose size is 1/2 the size
         * of the previous level.  The sum of the infinite series is 3.
         * Allocate some extra words for rounding up the sizes.
         */
        len = 3 * z.len + 32;
        tempbuf = zalloctemp(len);

        res->sign = 0;
        res->v = alloc((z.len+2) * 2);
        /*
         * Without the memset below, Purify reports that dosquare()
         *       will read uninitialized memory at the dosquare() line below
         *       the comment:
         *
         *              uninitialized memory read (see zsquare)
         *
         * This problem occurs during regression test #622 and may
         * be duplicated by executing:
         *
         *      config("sq2", 2);
         *      0xffff0000ffffffff00000000ffff0000000000000000ffff^2;
         */
        memset((char *)res->v, 0, ((z.len+2) * 2)*sizeof(HALF));
        res->len = dosquare(z.v, z.len, res->v);
}


/*
 * Recursive routine to square a number by splitting it up into two numbers
 * of half the size, and using the results of squaring the sub-pieces.
 * The result is placed in the indicated array, which must be large
 * enough for the result (size * 2).  Returns the size of the result.
 * This uses a temporary array which must be 3 times as large as the
 * size of the number at the top level recursive call.
 *
 * given:
 *      vp              value to be squared
 *      size            length of value to square
 *      ans             location for result
 */
S_FUNC LEN
dosquare(HALF *vp, LEN size, HALF *ans)
{
        LEN shift;              /* amount numbers are shifted by */
        LEN sizeA;              /* size of left half of number to square */
        LEN sizeB;              /* size of right half of number to square */
        LEN sizeAA;             /* size of square of left half */
        LEN sizeBB;             /* size of square of right half */
        LEN sizeAABB;           /* size of sum of squares of A and B */
        LEN sizeAB;             /* size of difference of A and B */
        LEN sizeABAB;           /* size of square of difference of A and B */
        LEN subsize;            /* size of difference of HALFs */
        LEN copysize;           /* size of number left to copy */
        LEN sumsize;            /* size of sum */
        LEN sizetotal;          /* total size of square */
        LEN len;                /* temporary length */
        LEN len1;               /* another temporary length */
        FULL carry;             /* carry digit for small multiplications */
        FULL digit;             /* single digit multiplying by */
        HALF *temp;             /* base for temporary calculations */
        HALF *baseA;            /* base of left half of number */
        HALF *baseB;            /* base of right half of number */
        HALF *baseAA;           /* base of square of left half of number */
        HALF *baseBB;           /* base of square of right half of number */
        HALF *baseAABB;         /* base of sum of squares of A and B */
        HALF *baseAB;           /* base of difference of A and B */
        HALF *baseABAB;         /* base of square of difference of A and B */
        register HALF *hd, *h1, *h2, *h3;       /* for inner loops */
        SIUNION sival;          /* for addition of digits */

        /* firewall */
        if (vp == NULL) {
                math_error("%s: vp NULL", __func__);
                not_reached();
        }
        if (ans == NULL) {
                math_error("%s: ans NULL", __func__);
                not_reached();
        }

        /*
         * First trim the number of leading zeroes.
         */
        hd = &vp[size - 1];
        while ((*hd == 0) && (size > 1)) {
                size--;
                hd--;
        }
        sizetotal = size + size;

        /*
         * If the number has only a small number of digits, then use the
         * (almost) normal multiplication method.  Multiply each halfword
         * only by those halfwords further on in the number.  Missed terms
         * will then be the same pairs of products repeated, and the squares
         * of each halfword.  The first case is handled by doubling the
         * result.  The second case is handled explicitly.  The number of
         * digits where the algorithm changes is settable from 2 to maxint.
         */
        if (size < conf->sq2) {
                hd = ans;
                len = sizetotal;
                while (len--)
                        *hd++ = 0;

                h2 = vp;
                hd = ans + 1;
                for (len = size; len--; hd += 2) {
                        digit = (FULL) *h2++;
                        if (digit == 0)
                                continue;
                        h3 = h2;
                        h1 = hd;
                        carry = 0;
                        len1 = len;
                        while (len1 >= 4) {     /* expand the loop some */
                                len1 -= 4;
                                sival.ivalue = (digit * ((FULL) *h3++))
                                        + ((FULL) *h1) + carry;
                                *h1++ = sival.silow;
                                sival.ivalue = (digit * ((FULL) *h3++))
                                        + ((FULL) *h1) + ((FULL) sival.sihigh);
                                *h1++ = sival.silow;
                                sival.ivalue = (digit * ((FULL) *h3++))
                                        + ((FULL) *h1) + ((FULL) sival.sihigh);
                                *h1++ = sival.silow;
                                sival.ivalue = (digit * ((FULL) *h3++))
                                        + ((FULL) *h1) + ((FULL) sival.sihigh);
                                *h1++ = sival.silow;
                                carry = sival.sihigh;
                        }
                        while (len1--) {
                                sival.ivalue = (digit * ((FULL) *h3++))
                                        + ((FULL) *h1) + carry;
                                *h1++ = sival.silow;
                                carry = sival.sihigh;
                        }
                        while (carry) {
                                sival.ivalue = ((FULL) *h1) + carry;
                                *h1++ = sival.silow;
                                carry = sival.sihigh;
                        }
                }

                /*
                 * Now double the result.
                 * There is no final carry to worry about because we
                 * handle all digits of the result which must fit.
                 */
                carry = 0;
                hd = ans;
                len = sizetotal;
                while (len--) {
                        digit = ((FULL) *hd);
                        sival.ivalue = digit + digit + carry;
                        /* ignore Saber-C warning #112 - get ushort from uint */
                        /*        OK to ignore on name dosquare`sival */
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }

                /*
                 * Now add in the squares of each halfword.
                 */
                carry = 0;
                hd = ans;
                h3 = vp;
                len = size;
                while (len--) {
                        digit = ((FULL) *h3++);
                        sival.ivalue = digit * digit + ((FULL) *hd) + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                        sival.ivalue = ((FULL) *hd) + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }
                while (carry) {
                        sival.ivalue = ((FULL) *hd) + carry;
                        *hd++ = sival.silow;
                        carry = sival.sihigh;
                }

                /*
                 * Finally return the size of the result.
                 */
                len = sizetotal;
                hd = &ans[len - 1];
                while ((*hd == 0) && (len > 1)) {
                        len--;
                        hd--;
                }
                return len;
        }

        /*
         * The number to be squared is large.
         * Allocate temporary space and determine the sizes and
         * positions of the values to be calculated.
         */
        temp = tempbuf;
        tempbuf += (3 * (size + 1) / 2);

        sizeA = size / 2;
        sizeB = size - sizeA;
        shift = sizeB;
        baseA = vp + sizeB;
        baseB = vp;
        baseAA = &ans[shift * 2];
        baseBB = ans;
        baseAABB = temp;
        baseAB = temp;
        baseABAB = &temp[shift];

        /*
         * Trim the second number of leading zeroes.
         */
        hd = &baseB[sizeB - 1];
        while ((*hd == 0) && (sizeB > 1)) {
                sizeB--;
                hd--;
        }

        /*
         * Now to proceed to calculate the result using the formula.
         *      (A*S+B)^2 = (S^2+S)*A^2 + (S+1)*B^2 - S*(A-B)^2.
         * The steps are done in the following order:
         *      S^2*A^2 + B^2
         *      A^2 + B^2
         *      (S^2+S)*A^2 + (S+1)*B^2
         *      (A-B)^2
         *      (S^2+S)*A^2 + (S+1)*B^2 - S*(A-B)^2.
         *
         * Begin by forming the squares of two the HALFs concatenated
         * together in the final result location.  Make sure that the
         * highest words of the results are zero.
         */
        sizeBB = dosquare(baseB, sizeB, baseBB);
        hd = &baseBB[sizeBB];
        len = shift * 2 - sizeBB;
        while (len--)
                *hd++ = 0;

        sizeAA = dosquare(baseA, sizeA, baseAA);
        hd = &baseAA[sizeAA];
        len = sizetotal - shift * 2 - sizeAA;
        while (len--)
                *hd++ = 0;

        /*
         * Sum the two squares into a temporary location.
         */
        if (sizeAA >= sizeBB) {
                h1 = baseAA;
                h2 = baseBB;
                sizeAABB = sizeAA;
                sumsize = sizeBB;
        } else {
                h1 = baseBB;
                h2 = baseAA;
                sizeAABB = sizeBB;
                sumsize = sizeAA;
        }
        copysize = sizeAABB - sumsize;

        hd = baseAABB;
        carry = 0;
        while (sumsize--) {
                sival.ivalue = ((FULL) *h1++) + ((FULL) *h2++) + carry;
                *hd++ = sival.silow;
                carry = sival.sihigh;
        }
        while (copysize--) {
                sival.ivalue = ((FULL) *h1++) + carry;
                *hd++ = sival.silow;
                carry = sival.sihigh;
        }
        if (carry) {
                *hd = (HALF)carry;
                sizeAABB++;
        }

        /*
         * Add the sum back into the previously calculated squares
         * shifted over to the proper location.
         */
        h1 = baseAABB;
        hd = ans + shift;
        carry = 0;
        len = sizeAABB;
        while (len--) {
                sival.ivalue = ((FULL) *hd) + ((FULL) *h1++) + carry;
                *hd++ = sival.silow;
                carry = sival.sihigh;
        }
        while (carry) {
                /* uninitialized memory read (see zsquare) */
                sival.ivalue = ((FULL) *hd) + carry;
                *hd++ = sival.silow;
                carry = sival.sihigh;
        }

        /*
         * Calculate the absolute value of the difference of the two HALFs
         * into a temporary location.
         */
        if (sizeA == sizeB) {
                len = sizeA;
                h1 = &baseA[len - 1];
                h2 = &baseB[len - 1];
                while ((len > 1) && (*h1 == *h2)) {
                        len--;
                        h1--;
                        h2--;
                }
        }
        if ((sizeA > sizeB) || ((sizeA == sizeB) && (*h1 > *h2))) {
                h1 = baseA;
                h2 = baseB;
                sizeAB = sizeA;
                subsize = sizeB;
        } else {
                h1 = baseB;
                h2 = baseA;
                sizeAB = sizeB;
                subsize = sizeA;
        }
        copysize = sizeAB - subsize;

        hd = baseAB;
        carry = 0;
        while (subsize--) {
                sival.ivalue = BASE1 - ((FULL) *h1++) + ((FULL) *h2++) + carry;
                *hd++ = (HALF)(BASE1 - sival.silow);
                carry = sival.sihigh;
        }
        while (copysize--) {
                sival.ivalue = (BASE1 - ((FULL) *h1++)) + carry;
                *hd++ = (HALF)(BASE1 - sival.silow);
                carry = sival.sihigh;
        }

        hd = &baseAB[sizeAB - 1];
        while ((*hd == 0) && (sizeAB > 1)) {
                sizeAB--;
                hd--;
        }

        /*
         * Now square the number into another temporary location,
         * and subtract that from the final result.
         */
        sizeABAB = dosquare(baseAB, sizeAB, baseABAB);

        h1 = baseABAB;
        hd = ans + shift;
        carry = 0;
        while (sizeABAB--) {
                sival.ivalue = BASE1 - ((FULL) *hd) + ((FULL) *h1++) + carry;
                *hd++ = (HALF)(BASE1 - sival.silow);
                carry = sival.sihigh;
        }
        while (carry) {
                sival.ivalue = BASE1 - ((FULL) *hd) + carry;
                *hd++ = (HALF)(BASE1 - sival.silow);
                carry = sival.sihigh;
        }

        /*
         * Return the size of the result.
         */
        len = sizetotal;
        hd = &ans[len - 1];
        while ((*hd == 0) && (len > 1)) {
                len--;
                hd--;
        }
        tempbuf = temp;
        return len;
}


/*
 * Return a pointer to a buffer to be used for holding a temporary number.
 * The buffer will be at least as large as the specified number of HALFs,
 * and remains valid until the next call to this routine.  The buffer cannot
 * be freed by the caller.  There is only one temporary buffer, and so as to
 * avoid possible conflicts this is only used by the lowest level routines
 * such as divide, multiply, and square.
 *
 * given:
 *      len             required number of HALFs in buffer
 */
HALF *
zalloctemp(LEN len)
{
        HALF *hp;
        STATIC LEN buflen;      /* current length of temp buffer */
        STATIC HALF *bufptr;    /* pointer to current temp buffer */

        if (len <= buflen)
                return bufptr;

        /*
         * We need to grow the temporary buffer.
         * First free any existing buffer, and then allocate the new one.
         * While we are at it, make the new buffer bigger than necessary
         * in order to reduce the number of reallocations.
         */
        len += 100;
        if (buflen) {
                buflen = 0;
                free(bufptr);
        }
        /* don't call alloc() because _math_abort_ may not be set right */
        hp = (HALF *) malloc((len+1) * sizeof(HALF));
        if (hp == NULL) {
                math_error("No memory for temp buffer");
                not_reached();
        }
        bufptr = hp;
        buflen = len;
        return hp;
}
