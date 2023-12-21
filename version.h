/*
 * version - determine the version of calc
 *
 * Copyright (C) 2023  David I. Bell and Landon Curt Noll
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
 * Under source code control:	2023/08/19 17:32:42
 * File existed as early as:	2023
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_VERSION_H)
#define INCLUDE_VERSION_H


/*
 * MAJOR_VER
 *
 *	The MAJOR_VER is 2 is the classical version of calc.
 *
 *      One of the main reasons why MAJOR_VER might incremented is
 *	if fundamental calc data objects (such as when ZVALUE or NUMBER
 *	or COMPLEX need to change) that would cause an incompatibility
 *      with existing hardware accelerators that are using fundamental
 *	calc data objects.
 *
 * MINOR_VER
 *
 *      The MINOR_VER changes when there are incompatible changes to the calc library
 *      or calc custom library.   The MINOR_VER might change if we need to make a major
 *      change to the math engine.  For example, when the way 0^x was evaluated, we
 *      changed MINOR_VER from 13 to 14.
 *
 * MAJOR_PATCH
 *
 *      The MAJOR_PATCH changes when there is an update to the calc library
 *      or calc custom library.  For example, the MAJOR_PATCH might increment when there
 *      are new builtin functions available, or when there is a change to how existing
 *      builtin functions process arguments.
 *
 * MINOR_PATCH
 *
 *      The MINOR_PATCH changes whenever there is any change in the calc release.
 *      For example, when the documentation changes, the MINOR_PATCH will increment.
 *      Moreover, when we are working towards a new production release,
 *      bug fix and improvement updates will cause MINOR_PATCH to increment.
 */
#define MAJOR_VER	2	/* level 1: major library version */
#define MINOR_VER	15	/* level 2: minor library version */
#define MAJOR_PATCH	0	/* level 3: major software version level */
#define MINOR_PATCH	3	/* level 4: minor software version level */


#endif /* !INCLUDE_VERSION_H*/
