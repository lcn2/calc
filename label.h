/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */


#if !defined(__LABEL_H__)
#define	__LABEL_H__


#include "zmath.h"


#define	NULL_LABEL	((LABEL *) 0)


/*
 * Label structures.
 */
typedef struct {
	long l_offset;		  /* offset into code of label */
	long l_chain;		  /* offset into code of undefined chain */
	char *l_name;		  /* name of label if any */
} LABEL;


extern void initlabels(void);
extern void definelabel(char *name);
extern void addlabel(char *name);
extern void clearlabel(LABEL *lp);
extern void setlabel(LABEL *lp);
extern void uselabel(LABEL *lp);
extern void checklabels(void);


#endif /* !__LABEL_H__ */
