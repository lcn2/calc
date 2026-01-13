/*
 * int - integer types and integer macros used in calc
 *
 * For general information on C integers, see:
 *
 *      https://en.cppreference.com/w/c/language/arithmetic_types
 *      https://en.cppreference.com/w/c/types/integer
 *
 * Copyright (C) 2023,2026  Landon Curt Noll
 *
 * Primary author:  Landon Curt Noll
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
 * Under source code control:   2023/08/22 21:14:25
 * File existed as early as:    2023
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_INT_H)
#  define INCLUDE_INT_H

/*
 * important include files that have an impact in integer types used in calc
 */

#  if defined(CALC_SRC) /* if we are building from the calc source tree */
#    include "status.chk_c.h"
#    include "charbit.h"
#  else
#    include <calc/status.chk_c.h>
#    include <calc/charbit.h>
#  endif

/*
 * case: C compiler and select include files appear to meet calc requirements
 */
#  if defined(CHK_C)

/*
 * bit width of various signed integers
 *
 * These are standard for c23 or later.
 * Compute them for earlier compiler standards.
 */
#    if !defined(INT8_WIDTH)
#      define INT8_WIDTH (sizeof(int8_t) * CALC_CHARBIT)
#    endif

#    if !defined(INT16_WIDTH)
#      define INT16_WIDTH (sizeof(int16_t) * CALC_CHARBIT)
#    endif

#    if !defined(INT32_WIDTH)
#      define INT32_WIDTH (sizeof(int32_t) * CALC_CHARBIT)
#    endif

#    if !defined(INT64_WIDTH)
#      define INT64_WIDTH (sizeof(int64_t) * CALC_CHARBIT)
#    endif

#    if !defined(INTPTR_WIDTH)
#      define INTPTR_WIDTH (sizeof(intptr_t) * CALC_CHARBIT)
#    endif

#    if !defined(INTMAX_WIDTH)
#      define INTMAX_WIDTH (sizeof(intmax_t) * CALC_CHARBIT)
#    endif

/*
 * bit width of various unsigned integers
 *
 * These are standard for c23 or later.
 * Compute them for earlier compiler standards.
 */
#    if !defined(UINT8_WIDTH)
#      define UINT8_WIDTH (sizeof(uint8_t) * CALC_CHARBIT)
#    endif

#    if !defined(UINT16_WIDTH)
#      define UINT16_WIDTH (sizeof(uint16_t) * CALC_CHARBIT)
#    endif

#    if !defined(UINT32_WIDTH)
#      define UINT32_WIDTH (sizeof(uint32_t) * CALC_CHARBIT)
#    endif

#    if !defined(UINT64_WIDTH)
#      define UINT64_WIDTH (sizeof(uint64_t) * CALC_CHARBIT)
#    endif

#    if !defined(UINTPTR_WIDTH)
#      define UINTPTR_WIDTH (sizeof(uintptr_t) * CALC_CHARBIT)
#    endif

#    if !defined(UINTMAX_WIDTH)
#      define UINTMAX_WIDTH (sizeof(uintmax_t) * CALC_CHARBIT)
#    endif

/*
 * case: C compiler and/or select include files do not meet calc requirements
 */
#  else

/*
 * calc v2 is the last version where one might be able to use an old C compiler
 *         and/or tolerate missing include files
 */
#    if MAJOR_VER < 3

/*
 * bit width of various signed integers
 *
 * These are standard for c23 or later.
 * Compute them for earlier compiler standards.
 */
#      if !defined(INT8_WIDTH)
#        define INT8_WIDTH (8)
#      endif

#      if !defined(INT16_WIDTH)
#        define INT16_WIDTH (16)
#      endif

#      if !defined(INT32_WIDTH)
#        define INT32_WIDTH (32)
#      endif

#      if !defined(INT64_WIDTH)
#        define INT64_WIDTH (64)
#      endif

#      if !defined(INTPTR_WIDTH)
#        define INTPTR_WIDTH (64)
#      endif

#      if !defined(INTMAX_WIDTH)
#        define INTMAX_WIDTH (64)
#      endif

/*
 * bit width of various unsigned integers
 *
 * These are standard for c23 or later.
 * Compute them for earlier compiler standards.
 */
#      if !defined(UINT8_WIDTH)
#        define UINT8_WIDTH (8)
#      endif

#      if !defined(UINT16_WIDTH)
#        define UINT16_WIDTH (16)
#      endif

#      if !defined(UINT32_WIDTH)
#        define UINT32_WIDTH (32)
#      endif

#      if !defined(UINT64_WIDTH)
#        define UINT64_WIDTH (64)
#      endif

#      if !defined(UINTPTR_WIDTH)
#        define UINTPTR_WIDTH (64)
#      endif

#      if !defined(UINTMAX_WIDTH)
#        define UINTMAX_WIDTH (64)
#      endif

#    else

/*
 * calc v3 or later and CHK_C is undefined
 */
#      pragma GCC error "calc v3 and later require C compiler and include files that support CHK_C"

#    endif
#  endif

#endif
