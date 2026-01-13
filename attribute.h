/*
 * attribute - control use of attributes in a backward compatible way
 *
 * Copyright (C) 2022  Landon Curt Noll
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
 * Under source code control:   2022/01/21 22:51:25
 * File existed as early as:    2022
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_ATTRIBUTE_H)
#  define INCLUDE_ATTRIBUTE_H

/*
 * backward compatibility
 *
 * Not all compilers support __attribute__ nor do they suuport __has_builtin.
 * For example, MSVC, TenDRAm and Little C Compiler doesn't support __attribute__.
 * Early gcc does not support __attribute__.
 *
 * Not all compiles have __has_builtin */
#  if !defined(__attribute__) && (defined(__cplusplus) || !defined(__GNUC__) || __GNUC__ == 2 && __GNUC_MINOR__ < 8)
#    define __attribute__(A)
#  endif
#  if !defined __has_builtin
#    define __has_builtin(x) 0
#  endif

/*
 * not_reached
 *
 * In the old days of lint, one could give lint and friends a hint by
 * placing the token NOTREACHED immediately between opening and closing
 * comments.  Modern compilers do not honor such commented tokens
 * and instead rely on features such as __builtin_unreachable
 * and __attribute__((noreturn)).
 *
 * The not_reached will either yield a __builtin_unreachable() feature call,
 * or it will call abort from stdlib.
 */
#  if __has_builtin(__builtin_unreachable)
#    define not_reached() __builtin_unreachable()
#  else
#    define not_reached() abort()
#  endif

#endif
