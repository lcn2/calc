/*
 * c_register - set or print a custom register value
 *
 * Copyright (C) 2007,2021  Landon Curt Noll
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
 * Under source code control:	2007/07/14 20:23:46
 * File existed as early as:	2007
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if defined(CUSTOM)

#include <stdio.h>

#include "have_const.h"
#include "value.h"
#include "custom.h"

#include "config.h"
#include "calc.h"

#include "have_unused.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * registers
 */
STATIC VALUE custom_reg[CUSTOM_REG_MAX+1];


/*
 * init_custreg - initialize custom registers
 */
void
init_custreg(void)
{
	int i;

	/*
	 * set the registers to zero
	 */
	for (i=0; i < CUSTOM_REG_MAX+1; ++i) {
	    custom_reg[i].v_type = V_NUM;
	    custom_reg[i].v_subtype = V_NOSUBTYPE;
	    custom_reg[i].v_num = itoq(0);
	}
	return;
}


/*
 * c_register - set or print a custom register value
 *
 * given:
 *	vals[i]	  and arg to display information about
 *
 * returns:
 *	count
 */
/*ARGSUSED*/
VALUE
c_register(char *UNUSED(name), int count, VALUE **vals)
{
	VALUE result;		/* what we will return */
	long reg;		/* register number */

	/*
	 * arg check
	 */
	result.v_type = V_NULL;
	if (vals[0]->v_type != V_NUM) {
		math_error("Non-numeric register number");
		/*NOTREACHED*/
	}
	if (qisfrac(vals[0]->v_num)) {
		math_error("Non-integer register number");
		/*NOTREACHED*/
	}
	if (qisneg(vals[0]->v_num)) {
		math_error("register number < 0");
		/*NOTREACHED*/
	}
	if (! qistiny(vals[0]->v_num)) {
		math_error("register is huge");
		/*NOTREACHED*/
	}
	reg = qtoi(vals[0]->v_num);
	if (reg > CUSTOM_REG_MAX) {
		math_error("register is larger than CUSTOM_REG_MAX");
		/*NOTREACHED*/
	}

	/*
	 * print info on each arg
	 */
	/* save previous value */
	copyvalue(&custom_reg[reg], &result);
	/* set the new value if a 2nd arg was given */
	if (count == 2) {
	    copyvalue(vals[1], &custom_reg[reg]);
	}

	/*
	 * return result
	 */
	return result;
}

#endif /* CUSTOM */
