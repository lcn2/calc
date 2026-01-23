/*
 * chk_c - chk C compiler and C include for calc requirements
 *
 * This program, to successfully compile, must include:
 *
 *      #include <stdint.h>
 *      #include <inttypes.h>
 *
 * and must be able to compile all of these types:
 *
 *      int8_t          uint8_t
 *      int16_t         uint16_t
 *      int32_t         uint32_t
 *      int64_t         uint64_t
 *      intptr_t        uintptr_t
 *      intmax_t        uintmax_t
 *
 * be able to sum values from all of those files,
 * plus the INTX_C(val) and UINTX_C(val) composing macros,
 * and be able to print for signed and unsigned values.
 *
 * If you are unable to compile this program, or if this program when
 * compiles does not exit 0, then your C compiler and/or C include
 * fails to meet calc requirements.
 *
 * Compilers that are at least c17 MUST be able to compile this program
 * such that when run will exit 0.
 *
 * Copyright (C) 2023,2026  Landon Curt Noll
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
 * Under source code control:   2023/08/20 23:57:50
 * File existed as early as:    2023
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

/*
 * calc local src includes
 */

#include "banned.h" /* include after all other includes */

/*
 * our version
 */
#define CHK_C_VERSION "2.1 2026-01-22"

/*
 * usage:
 */
static char const *usage = "usage: %s [-h] [-c] [-V]\n"
                           "\n"
                           "\t-h\t\toutput this message and exit\n"
                           "\t-c\t\toutput C compiler and C include info (def: no stdout output)\n"
                           "\t-V\t\tprint version string and exit\n"
                           "\n"
                           "Exit codes:\n"
                           "     0   all OK\n"
                           "     1   C compiler and C include is insufficient for calc\n"
                           "     2   -h and help string printed or -V and version string printed\n"
                           "     3   invalid command line, invalid option or option missing an argument\n"
                           " >= 10   internal error\n"
                           "\n"
                           "chk_c version: %s\n";

int
main(int argc, char *argv[])
{
    char *program = "((NULL))";       /* our name */
    int opt;                          /* the -option found or -1 */
    bool c_flag = false;              /* if -h was found */
    int i = -1;                       /* signed int test value */
    unsigned int ui = 1;              /* unsigned int test value */
    long l = -2L;                     /* signed long test value */
    unsigned long ul = 2UL;           /* unsigned long test value */
    long long ll = -4LL;              /* signed long long test value */
    unsigned long long ull = 4ULL;    /* unsigned long long test value */
    int8_t i8 = INT8_C(-8);           /* signed 8-bit test value */
    uint8_t ui8 = UINT8_C(8);         /* unsigned 8-bit test value */
    int16_t i16 = INT16_C(-16);       /* signed 16-bit test value */
    uint16_t ui16 = UINT16_C(16);     /* unsigned 16-bit test value */
    int32_t i32 = INT32_C(-32);       /* signed 32-bit test value */
    uint32_t ui32 = UINT32_C(32);     /* unsigned 32-bit test value */
    int64_t i64 = INT64_C(-64);       /* signed 64-bit test value */
    uint64_t ui64 = UINT64_C(64);     /* unsigned 64-bit test value */
    long lv = -128L;                  /* signed long value */
    unsigned long ulv = 128UL;        /* unsigned signed long value */
    long long llv = -256LL;           /* signed long value */
    unsigned long long ullv = 256ULL; /* unsigned signed long value */
    intptr_t iptr = -512;             /* integer type capable of holding a pointer */
    intptr_t uiptr = 512;             /* unsigned integer type capable of holding a pointer */
    intmax_t imax = INTMAX_C(-1024);  /* maximum sized signed int test value */
    uintmax_t uimax = UINTMAX_C(1024);/* maximum sized unsigned int test value */
    uintmax_t isum = 0;               /* sum of all test values as signed value */
    uintmax_t uisum = 0;              /* sum of all test values as unsigned value */

    /*
     * parse args
     */
    program = argv[0];
    while ((opt = getopt(argc, argv, "hcV")) != -1) {
        switch (opt) {
        case 'h':
            fprintf(stderr, usage, program, CHK_C_VERSION);
            exit(2);
            break;
        case 'c':
            c_flag = true;
            break;
        case 'V':
            printf("%s\n", CHK_C_VERSION);
            exit(2);
            break;
        default:
            fprintf(stderr, usage, program, CHK_C_VERSION);
            exit(3);
        }
    }
    if (optind != argc) {
        fprintf(stderr, usage, program, CHK_C_VERSION);
        exit(3);
    }

    /*
     * check for __STDC__ defined
     */
#if !defined(__STDC__)

    fprintf(stderr, "__STDC__ is not defined\n");
    exit(1);

#endif
    if (c_flag == true) {
        printf("\n__STDC__ is defined\n");
    }

    /*
     * check for __STDC_VERSION__ defined
     */
#if !defined(__STDC_VERSION__)

    fprintf(stderr, "__STDC_VERSION__ is not defined\n");
    exit(1);

#endif
    if (c_flag == true) {
        printf("__STDC_VERSION__: %ld is defined\n", __STDC_VERSION__);
    }

    /*
     * check for __STDC_VERSION__ >= 201710L for c17 or later
     */
#if __STDC_VERSION__ < 201710L

    fprintf(stderr, "__STDC_VERSION__: %ld < 201710L\n", __STDC_VERSION__);
    exit(1);

#endif
    if (c_flag == true) {
        printf("__STDC_VERSION__: %ld >= 201710L\n", __STDC_VERSION__);
    }

    /*
     * calculate sum of all test values as signed value
     */
    isum = i + ui + l + ul + ll + ull + i8 + ui8 + i16 + ui16 + i32 + ui32 + i64 + ui64 +
           lv + ulv + llv + ullv + iptr + uiptr + imax + uimax;
    if (c_flag == true) {
        printf("\nzero valued isum: %jd\n", isum);
    }
    if (isum != 0) {
        fprintf(stderr, "failed to add all required integer types into a signed value\n");
        exit(1);
    }

    /*
     * calculate sum of all test values as unsigned value
     */
    uisum = i + ui + l + ul + ll + ull + i8 + ui8 + i16 + ui16 + i32 + ui32 + i64 + ui64 +
           lv + ulv + llv + ullv + iptr + uiptr + imax + uimax;
    if (c_flag == true) {
        printf("zero valued uisum: %ju\n", uisum);
    }
    if (uisum != 0) {
        fprintf(stderr, "failed to add all required integer types into an unsigned value\n");
        exit(1);
    }

    /*
     * verify various 32, 64 and max PRI macros as well as MIN and MAX constants
     */
    if (c_flag == true) {
        printf("\nsigned integer MIN and MAX\n");
        printf("INT8_MIN: %+" PRId8 "\n", INT8_MIN);
        printf("INT8_MAX: %+" PRId8 "\n", INT8_MAX);
        printf("INT16_MIN: %+" PRId16 "\n", INT16_MIN);
        printf("INT16_MAX: %+" PRId16 "\n", INT16_MAX);
        printf("INT32_MIN: %+" PRId32 "\n", INT32_MIN);
        printf("INT32_MAX: %+" PRId32 "\n", INT32_MAX);
        printf("INT64_MIN: %+" PRId64 "\n", INT64_MIN);
        printf("INT64_MAX: %+" PRId64 "\n", INT64_MAX);
        printf("LONG_MIN: %ld\n", LONG_MIN);
        printf("LONG_MAX: %ld\n", LONG_MAX);
        printf("LLONG_MIN: %lld\n", LLONG_MIN);
        printf("LLONG_MAX: %lld\n", LLONG_MAX);
        printf("INTPTR_MIN: %+" PRIdMAX "\n", INTPTR_MIN);
        printf("INTPTR_MAX: %+" PRIdMAX "\n", INTPTR_MAX);
        printf("INTMAX_MIN: %+" PRIdMAX "\n", INTMAX_MIN);
        printf("INTMAX_MAX: %+" PRIdMAX "\n", INTMAX_MAX);

        printf("\nunsigned integer MAX\n");
        printf("UINT8_MAX: %" PRIu8 "\n", INT8_MAX);
        printf("UINT16_MAX: %" PRIu16 "\n", INT16_MAX);
        printf("UINT32_MAX: %" PRIu32 "\n", INT32_MAX);
        printf("UINT64_MAX: %" PRIu64 "\n", INT64_MAX);
        printf("ULONG_MAX: %lu\n", ULONG_MAX);
        printf("ULLONG_MAX: %llu\n", ULLONG_MAX);
        printf("UINTPTR_MAX: %" PRIuMAX "\n", INTPTR_MAX);
        printf("UINTMAX_MAX: %" PRIuMAX "\n", INTMAX_MAX);

        printf("\ninteger MIN and MAX\n");
        printf("INT8_MIN: %" PRIi8 "\n", INT8_MIN);
        printf("INT8_MAX: %" PRIi8 "\n", INT8_MAX);
        printf("INT16_MIN: %" PRIi16 "\n", INT16_MIN);
        printf("INT16_MAX: %" PRIi16 "\n", INT16_MAX);
        printf("INT32_MIN: %" PRIi32 "\n", INT32_MIN);
        printf("INT32_MAX: %" PRIi32 "\n", INT32_MAX);
        printf("INT64_MIN: %" PRIi64 "\n", INT64_MIN);
        printf("INT64_MAX: %" PRIi64 "\n", INT64_MAX);
        printf("INTPTR_MIN: %" PRIiMAX "\n", INTPTR_MIN);
        printf("INTPTR_MAX: %" PRIiMAX "\n", INTPTR_MAX);
        printf("INTMAX_MIN: %" PRIiMAX "\n", INTMAX_MIN);
        printf("INTMAX_MAX: %" PRIiMAX "\n", INTMAX_MAX);

        printf("\noctal MIN and MAX\n");
        printf("INT8_MIN: %" PRIo8 "\n", INT8_MIN);
        printf("INT8_MAX: %" PRIo8 "\n", INT8_MAX);
        printf("INT16_MIN: %" PRIo16 "\n", INT16_MIN);
        printf("INT16_MAX: %" PRIo16 "\n", INT16_MAX);
        printf("INT32_MIN: %" PRIo32 "\n", INT32_MIN);
        printf("INT32_MAX: %" PRIo32 "\n", INT32_MAX);
        printf("INT64_MIN: %" PRIo64 "\n", INT64_MIN);
        printf("INT64_MAX: %" PRIo64 "\n", INT64_MAX);
        printf("LONG_MIN: %lo\n", LONG_MIN);
        printf("LONG_MAX: %lo\n", LONG_MAX);
        printf("LLONG_MIN: %llo\n", LLONG_MIN);
        printf("LLONG_MAX: %llo\n", LLONG_MAX);
        printf("INTPTR_MIN: %" PRIoMAX "\n", INTPTR_MIN);
        printf("INTPTR_MAX: %" PRIoMAX "\n", INTPTR_MAX);
        printf("INTMAX_MIN: %" PRIoMAX "\n", INTMAX_MIN);
        printf("INTMAX_MAX: %" PRIoMAX "\n", INTMAX_MAX);

        printf("\nhex MIN and MAX\n");
        printf("INT8_MIN: %" PRIx8 "\n", INT8_MIN);
        printf("INT8_MAX: %" PRIx8 "\n", INT8_MAX);
        printf("INT16_MIN: %" PRIx16 "\n", INT16_MIN);
        printf("INT16_MAX: %" PRIx16 "\n", INT16_MAX);
        printf("INT32_MIN: %" PRIx32 "\n", INT32_MIN);
        printf("INT32_MAX: %" PRIx32 "\n", INT32_MAX);
        printf("INT64_MIN: %" PRIx64 "\n", INT64_MIN);
        printf("INT64_MAX: %" PRIx64 "\n", INT64_MAX);
        printf("LONG_MIN: %lx\n", LONG_MIN);
        printf("LONG_MAX: %lx\n", LONG_MAX);
        printf("LLONG_MIN: %llx\n", LLONG_MIN);
        printf("LLONG_MAX: %llx\n", LLONG_MAX);
        printf("INTPTR_MIN: %" PRIxMAX "\n", INTPTR_MIN);
        printf("INTPTR_MAX: %" PRIxMAX "\n", INTPTR_MAX);
        printf("INTMAX_MIN: %" PRIxMAX "\n", INTMAX_MIN);
        printf("INTMAX_MAX: %" PRIxMAX "\n", INTMAX_MAX);

        printf("\nHEX MIN and MAX\n");
        printf("INT8_MIN: %" PRIX8 "\n", INT8_MIN);
        printf("INT8_MAX: %" PRIX8 "\n", INT8_MAX);
        printf("INT16_MIN: %" PRIX16 "\n", INT16_MIN);
        printf("INT16_MAX: %" PRIX16 "\n", INT16_MAX);
        printf("INT32_MIN: %" PRIX32 "\n", INT32_MIN);
        printf("INT32_MAX: %" PRIX32 "\n", INT32_MAX);
        printf("INT64_MIN: %" PRIX64 "\n", INT64_MIN);
        printf("INT64_MAX: %" PRIX64 "\n", INT64_MAX);
        printf("LONG_MIN: %lX\n", LONG_MIN);
        printf("LONG_MAX: %lX\n", LONG_MAX);
        printf("LLONG_MIN: %llX\n", LLONG_MIN);
        printf("LLONG_MAX: %llX\n", LLONG_MAX);
        printf("INTPTR_MIN: %" PRIXMAX "\n", INTPTR_MIN);
        printf("INTPTR_MAX: %" PRIXMAX "\n", INTPTR_MAX);
        printf("INTMAX_MIN: %" PRIXMAX "\n", INTMAX_MIN);
        printf("INTMAX_MAX: %" PRIXMAX "\n", INTMAX_MAX);

        printf("\nsizes\n");
        printf("sizeof(int8_t): %zu\n", sizeof(int8_t));
        printf("sizeof(uint8_t): %zu\n", sizeof(uint8_t));
        printf("sizeof(int16_t): %zu\n", sizeof(int16_t));
        printf("sizeof(uint16_t): %zu\n", sizeof(uint16_t));
        printf("sizeof(int32_t): %zu\n", sizeof(int32_t));
        printf("sizeof(uint32_t): %zu\n", sizeof(uint32_t));
        printf("sizeof(int64_t): %zu\n", sizeof(int64_t));
        printf("sizeof(uint64_t): %zu\n", sizeof(uint64_t));
        printf("sizeof(long): %zu\n", sizeof(long));
        printf("sizeof(unsigned long): %zu\n", sizeof(unsigned long));
        printf("sizeof(long long): %zu\n", sizeof(long long));
        printf("sizeof(unsigned long long): %zu\n", sizeof(unsigned long long));
        printf("sizeof(intptr_t): %zu\n", sizeof(intptr_t));
        printf("sizeof(uintptr_t): %zu\n", sizeof(uintptr_t));
        printf("sizeof(intmax_t): %zu\n", sizeof(intmax_t));
        printf("sizeof(uintmax_t): %zu\n", sizeof(uintmax_t));
    }

    /*
     * All Done!! -- Jessica Noll, age 2
     */
    if (c_flag == true) {
        printf("\nC compiler and select include files appear to support calc.\n");
    }
    exit(0);
}
