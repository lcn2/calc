/*
 * Copyright (c) 1993 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Label handling routines.
 */

#include "calc.h"
#include "token.h"
#include "label.h"
#include "string.h"
#include "opcodes.h"
#include "func.h"

static long labelcount;			/* number of user labels defined */
static STRINGHEAD labelnames;		/* list of user label names */
static LABEL labels[MAXLABELS];		/* list of user labels */


/*
 * Initialize the table of labels for a function.
 */
void
initlabels(void)
{
	labelcount = 0;
	initstr(&labelnames);
}


/*
 * Define a user named label to have the offset of the next opcode.
 *
 * given:
 *	name		label name
 */
void
definelabel(char *name)
{
	register LABEL *lp;		/* current label */
	long i;				/* current label index */

	i = findstr(&labelnames, name);
	if (i >= 0) {
		lp = &labels[i];
		if (lp->l_offset) {
			scanerror(T_NULL, "Label \"%s\" is multiply defined",
				name);
			return;
		}
		setlabel(lp);
		return;
	}
	if (labelcount >= MAXLABELS) {
		scanerror(T_NULL, "Too many labels in use");
		return;
	}
	lp = &labels[labelcount++];
	lp->l_chain = 0;
	lp->l_offset = (long)curfunc->f_opcodecount;
	lp->l_name = addstr(&labelnames, name);
	clearopt();
}


/*
 * Add the offset corresponding to the specified user label name to the
 * opcode table for a function. If the label is not yet defined, then a
 * chain of undefined offsets is built using the offset value, and it
 * will be fixed up when the label is defined.
 *
 * given:
 *	name		user symbol name
 */
void
addlabel(char *name)
{
	register LABEL *lp;		/* current label */
	long i;				/* counter */

	for (i = labelcount, lp = labels; --i >= 0; lp++) {
		if (strcmp(name, lp->l_name))
			continue;
		uselabel(lp);
		return;
	}
	if (labelcount >= MAXLABELS) {
		scanerror(T_NULL, "Too many labels in use");
		return;
	}
	lp = &labels[labelcount++];
	lp->l_offset = 0;
	lp->l_chain = 0;
	lp->l_name = addstr(&labelnames, name);
	uselabel(lp);
}


/*
 * Check to make sure that all labels are defined.
 */
void
checklabels(void)
{
	register LABEL *lp;		/* label being checked */
	long i;				/* counter */

	for (i = labelcount, lp = labels; --i >= 0; lp++) {
		if (lp->l_offset > 0)
			continue;
		scanerror(T_NULL, "Label \"%s\" was never defined",
			lp->l_name);
	}
}


/*
 * Clear an internal label for use.
 *
 * given:
 *	lp		label being cleared
 */
void
clearlabel(LABEL *lp)
{
	lp->l_offset = 0;
	lp->l_chain = 0;
	lp->l_name = NULL;
}


/*
 * Set any label to have the value of the next opcode in the current
 * function being defined.  If there were forward references to it,
 * all such references are patched up.
 *
 * given:
 *	lp		label being set
 */
void
setlabel(LABEL *lp)
{
	register FUNC *fp;	/* current function */
	unsigned long curfix;	/* offset of current location being fixed */
	unsigned long nextfix;	/* offset of next location to fix up */
	unsigned long offset;	/* offset of this label */

	fp = curfunc;
	offset = fp->f_opcodecount;
	nextfix = lp->l_chain;
	while (nextfix > 0) {
		curfix = nextfix;
		nextfix = fp->f_opcodes[curfix];
		fp->f_opcodes[curfix] = offset;
	}
	lp->l_chain = 0;
	lp->l_offset = (long)offset;
	clearopt();
}


/*
 * Use the specified label at the current location in the function
 * being compiled.  This adds one word to the current function being
 * compiled.  If the label is not yet defined, a patch chain is built
 * so the reference can be fixed when the label is defined.
 *
 * given:
 *	lp		label being used
 */
void
uselabel(LABEL *lp)
{
	unsigned long offset;		/* offset being added */

	offset = curfunc->f_opcodecount;
	if (lp->l_offset > 0) {
		curfunc->f_opcodes[curfunc->f_opcodecount++] = lp->l_offset;
		return;
	}
	curfunc->f_opcodes[curfunc->f_opcodecount++] = lp->l_chain;
	lp->l_chain = (long)offset;
}

/* END CODE */
