/*
 * value - definitions of general values  and related routines used by calc
 *
 * Copyright (C) 1999  David I. Bell
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.7 $
 * @(#) $Id: value.h,v 29.7 2001/06/08 21:00:58 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/value.h,v $
 *
 * Under source code control:	1993/07/30 19:42:47
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__VALUE_H__)
#define __VALUE_H__


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "win32dll.h"
# include "cmath.h"
# include "config.h"
# include "shs.h"
# include "calcerr.h"
# include "hash.h"
# include "block.h"
# include "nametype.h"
# include "string.h"
#else
# include <calc/win32dll.h>
# include <calc/cmath.h>
# include <calc/config.h>
# include <calc/shs.h>
# include <calc/calcerr.h>
# include <calc/hash.h>
# include <calc/block.h>
# include <calc/nametype.h>
# include <calc/string.h>
#endif


#define MAXDIM		4	/* maximum number of dimensions in matrices */
#define USUAL_ELEMENTS	4	/* usual number of elements for objects */


/*
 * Flags to modify results from the printvalue routine.
 * These flags are OR'd together.
 */
#define PRINT_NORMAL	0x00	/* print in normal manner */
#define PRINT_SHORT	0x01	/* print in short format (no elements) */
#define PRINT_UNAMBIG	0x02	/* print in non-ambiguous manner */


/*
 * Definition of values of various types.
 */
typedef struct value VALUE;
typedef struct object OBJECT;
typedef struct matrix MATRIX;
typedef struct list LIST;
typedef struct assoc ASSOC;
typedef long FILEID;
typedef struct rand RAND;
typedef struct random RANDOM;


/*
 * calc values
 *
 * See below for information on what needs to be added for a new type.
 */
struct value {
	short v_type;			/* type of value */
	short v_subtype;		/* other data related to some types */
	union {				/* types of values (see V_XYZ below) */
		long vv_int;		/* 1: small integer value */
		NUMBER *vv_num;		/* 2, 21: real number */
		COMPLEX *vv_com;	/* 3: complex number */
		VALUE *vv_addr;		/* 4, 18: address of variable value */
		STRING *vv_str;		/* 5, 20: string value */
		MATRIX *vv_mat;		/* 6: address of matrix */
		LIST *vv_list;		/* 7: address of list */
		ASSOC *vv_assoc;	/* 8: address of association */
		OBJECT *vv_obj;		/* 9: address of object */
		FILEID vv_file;		/* 10: id of opened file */
		RAND *vv_rand;		/* 11: additive 55 random state */
		RANDOM *vv_random;	/* 12: Blum random state */
		CONFIG *vv_config;	/* 13: configuration state */
		HASH *vv_hash;		/* 14: hash state */
		BLOCK *vv_block;	/* 15: memory block */
		OCTET *vv_octet;	/* 16, 19: octet addr (unsigned char) */
		NBLOCK *vv_nblock;	/* 17: named memory block */
	} v_union;
};


/*
 * For ease in referencing
 */
#define v_int	v_union.vv_int
#define v_file	v_union.vv_file
#define v_num	v_union.vv_num
#define v_com	v_union.vv_com
#define v_addr	v_union.vv_addr
#define v_str	v_union.vv_str
#define v_mat	v_union.vv_mat
#define v_list	v_union.vv_list
#define v_assoc v_union.vv_assoc
#define v_obj	v_union.vv_obj
#define v_valid v_union.vv_int
#define v_rand	v_union.vv_rand
#define v_random v_union.vv_random
#define v_config v_union.vv_config
#define v_hash	v_union.vv_hash
#define v_block v_union.vv_block
#define v_octet v_union.vv_octet
#define v_nblock v_union.vv_nblock


/*
 * Value types.
 *
 * NOTE: The following files should be checked/adjusted for a new type:
 *
 *	size.c		- elm_count(), lsizeof()
 *	help/size	- update what the size() builtin will report
 *	hash.c		- hash_value()
 *	quickhash.c	- hashvalue()
 *	value.c		- freevalue(), copyvalue(), comparevalue(),
 *			  printvalue(),
 *			  and other as needed such as testvalue(), etc.
 *
 * There may be others, but at is at least a start.
 */
#define V_NULL	0	/* null value */
#define V_INT	1	/* normal integer */
#define V_NUM	2	/* number */
#define V_COM	3	/* complex number */
#define V_ADDR	4	/* address of variable value */
#define V_STR	5	/* address of string */
#define V_MAT	6	/* address of matrix structure */
#define V_LIST	7	/* address of list structure */
#define V_ASSOC 8	/* address of association structure */
#define V_OBJ	9	/* address of object structure */
#define V_FILE	10	/* opened file id */
#define V_RAND	11	/* address of additive 55 random state */
#define V_RANDOM 12	/* address of Blum random state */
#define V_CONFIG 13	/* configuration state */
#define V_HASH	14	/* hash state */
#define V_BLOCK 15	/* memory block */
#define V_OCTET 16	/* octet (unsigned char) */
#define V_NBLOCK 17	/* named memory block */
#define V_VPTR	18	/* value address as pointer */
#define V_OPTR	19	/* octet address as pointer */
#define V_SPTR	20	/* string address as pointer */
#define V_NPTR	21	/* number address as pointer */
#define V_MAX	21	/* highest legal value */

#define V_NOSUBTYPE	0	/* subtype has no meaning */
#define V_NOASSIGNTO	1	/* protection status 1 */
#define V_NONEWVALUE	2	/* protection status 2 */
#define V_NONEWTYPE	4	/* protection status 4 */
#define V_NOERROR	8	/* protection status 8 */
#define V_NOCOPYTO	16	/* protection status 16 */
#define V_NOREALLOC	32	/* protection status 32 */
#define V_NOASSIGNFROM	64	/* protection status 64 */
#define V_NOCOPYFROM	128	/* protection status 128 */
#define V_PROTECTALL	256	/* protection status 256 */

#define MAXPROTECT	511

/*
 * At present protect(var, sts) determines bits in var->v_subtype
 * corresponding to 4 * sts.  MAXPROTECT is the sum of the simple
 * (power of two) protection status values.
 */


#define TWOVAL(a,b) ((a) << 5 | (b))	/* for switch of two values */

#define NULL_VALUE	((VALUE *) 0)


/*
 * value functions
 */
extern DLL void freevalue(VALUE *vp);
extern DLL void copyvalue(VALUE *vp, VALUE *vres);
extern DLL void negvalue(VALUE *vp, VALUE *vres);
extern DLL void addvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void subvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void mulvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void orvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void andvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void compvalue(VALUE *vp, VALUE *vres);
extern DLL void xorvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void squarevalue(VALUE *vp, VALUE *vres);
extern DLL void invertvalue(VALUE *vp, VALUE *vres);
extern DLL void roundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern DLL void broundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern DLL void setminusvalue(VALUE *, VALUE *, VALUE *);
extern DLL void backslashvalue(VALUE *, VALUE *);
extern DLL void contentvalue(VALUE *, VALUE *);
extern DLL void hashopvalue(VALUE *, VALUE *, VALUE *);
extern DLL void apprvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern DLL void intvalue(VALUE *vp, VALUE *vres);
extern DLL void fracvalue(VALUE *vp, VALUE *vres);
extern DLL void incvalue(VALUE *vp, VALUE *vres);
extern DLL void decvalue(VALUE *vp, VALUE *vres);
extern DLL void conjvalue(VALUE *vp, VALUE *vres);
extern DLL void sqrtvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern DLL void rootvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern DLL void absvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void normvalue(VALUE *vp, VALUE *vres);
extern DLL void shiftvalue(VALUE *v1, VALUE *v2, BOOL rightshift, VALUE *vres);
extern DLL void scalevalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void powivalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void powervalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern DLL void divvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void quovalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern DLL void modvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern DLL BOOL testvalue(VALUE *vp);
extern DLL BOOL comparevalue(VALUE *v1, VALUE *v2);
extern DLL BOOL acceptvalue(VALUE *v1, VALUE *v2);
extern DLL void relvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern DLL void sgnvalue(VALUE *vp, VALUE *vres);
extern DLL QCKHASH hashvalue(VALUE *vp, QCKHASH val);
extern DLL void printvalue(VALUE *vp, int flags);
extern DLL BOOL precvalue(VALUE *v1, VALUE *v2);
extern DLL VALUE error_value(int e);
extern DLL int set_errno(int e);
extern DLL int set_errcount(int e);
extern DLL long countlistitems(LIST *lp);
extern DLL void addlistitems(LIST *lp, VALUE *vres);
extern DLL void addlistinv(LIST *lp, VALUE *vres);
extern DLL void copy2octet(VALUE *, OCTET *);
extern DLL int copystod(VALUE *, long, long, VALUE *, long);
extern DLL void protectall(VALUE *, int);
extern DLL void set_update(int);


/*
 * Structure of a matrix.
 */
struct matrix {
	long m_dim;		/* dimension of matrix */
	long m_size;		/* total number of elements */
	long m_min[MAXDIM];	/* minimum bound for indices */
	long m_max[MAXDIM];	/* maximum bound for indices */
	VALUE m_table[1];	/* actually varying length table */
};

#define matsize(n) (sizeof(MATRIX) - sizeof(VALUE) + ((n) * sizeof(VALUE)))


extern DLL MATRIX *matadd(MATRIX *m1, MATRIX *m2);
extern DLL MATRIX *matsub(MATRIX *m1, MATRIX *m2);
extern DLL MATRIX *matmul(MATRIX *m1, MATRIX *m2);
extern DLL MATRIX *matneg(MATRIX *m);
extern DLL MATRIX *matalloc(long size);
extern DLL MATRIX *matcopy(MATRIX *m);
extern DLL MATRIX *matinit(MATRIX *m, VALUE *v1, VALUE *v2);
extern DLL MATRIX *matsquare(MATRIX *m);
extern DLL MATRIX *matinv(MATRIX *m);
extern DLL MATRIX *matscale(MATRIX *m, long n);
extern DLL MATRIX *matshift(MATRIX *m, long n);
extern DLL MATRIX *matmulval(MATRIX *m, VALUE *vp);
extern DLL MATRIX *matpowi(MATRIX *m, NUMBER *q);
extern DLL MATRIX *matconj(MATRIX *m);
extern DLL MATRIX *matquoval(MATRIX *m, VALUE *vp, VALUE *v3);
extern DLL MATRIX *matmodval(MATRIX *m, VALUE *vp, VALUE *v3);
extern DLL MATRIX *matint(MATRIX *m);
extern DLL MATRIX *matfrac(MATRIX *m);
extern DLL MATRIX *matappr(MATRIX *m, VALUE *v2, VALUE *v3);
extern DLL VALUE mattrace(MATRIX *m);
extern DLL MATRIX *mattrans(MATRIX *m);
extern DLL MATRIX *matcross(MATRIX *m1, MATRIX *m2);
extern DLL BOOL mattest(MATRIX *m);
extern DLL void matsum(MATRIX *m, VALUE *vres);
extern DLL BOOL matcmp(MATRIX *m1, MATRIX *m2);
extern DLL int matsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
extern DLL int matrsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
extern DLL VALUE matdet(MATRIX *m);
extern DLL VALUE matdot(MATRIX *m1, MATRIX *m2);
extern DLL void matfill(MATRIX *m, VALUE *v1, VALUE *v2);
extern DLL void matfree(MATRIX *m);
extern DLL void matprint(MATRIX *m, long max_print);
extern DLL VALUE *matindex(MATRIX *mp, BOOL create, long dim, VALUE *indices);
extern DLL void matreverse(MATRIX *m);
extern DLL void matsort(MATRIX *m);
extern DLL BOOL matisident(MATRIX *m);
extern DLL MATRIX *matround(MATRIX *m, VALUE *v2, VALUE *v3);
extern DLL MATRIX *matbround(MATRIX *m, VALUE *v2, VALUE *v3);


/*
 * List definitions.
 * An individual list element.
 */
typedef struct listelem LISTELEM;
struct listelem {
	LISTELEM *e_next;	/* next element in list (or NULL) */
	LISTELEM *e_prev;	/* previous element in list (or NULL) */
	VALUE e_value;		/* value of this element */
};


/*
 * Structure for a list of elements.
 */
struct list {
	LISTELEM *l_first;	/* first list element (or NULL) */
	LISTELEM *l_last;	/* last list element (or NULL) */
	LISTELEM *l_cache;	/* cached list element (or NULL) */
	long l_cacheindex;	/* index of cached element (or undefined) */
	long l_count;		/* total number of elements in the list */
};


extern DLL void insertlistfirst(LIST *lp, VALUE *vp);
extern DLL void insertlistlast(LIST *lp, VALUE *vp);
extern DLL void insertlistmiddle(LIST *lp, long index, VALUE *vp);
extern DLL void removelistfirst(LIST *lp, VALUE *vp);
extern DLL void removelistlast(LIST *lp, VALUE *vp);
extern DLL void removelistmiddle(LIST *lp, long index, VALUE *vp);
extern DLL void listfree(LIST *lp);
extern DLL void listprint(LIST *lp, long max_print);
extern DLL int listsearch(LIST *lp, VALUE *vp, long start, long end, ZVALUE *index);
extern DLL int listrsearch(LIST *lp, VALUE *vp, long start, long end, ZVALUE *index);
extern DLL BOOL listcmp(LIST *lp1, LIST *lp2);
extern DLL VALUE *listfindex(LIST *lp, long index);
extern DLL LIST *listalloc(void);
extern DLL LIST *listcopy(LIST *lp);
extern DLL void listreverse(LIST *lp);
extern DLL void listsort(LIST *lp);
extern DLL LIST *listappr(LIST *lp, VALUE *v2, VALUE *v3);
extern DLL LIST *listround(LIST *m, VALUE *v2, VALUE *v3);
extern DLL LIST *listbround(LIST *m, VALUE *v2, VALUE *v3);
extern DLL LIST *listquo(LIST *lp, VALUE *v2, VALUE *v3);
extern DLL LIST *listmod(LIST *lp, VALUE *v2, VALUE *v3);
extern DLL BOOL evp(LISTELEM *cp, LISTELEM *x, VALUE *vres);
extern DLL BOOL evalpoly(LIST *clist, LISTELEM *x, VALUE *vres);
extern DLL void insertitems(LIST *lp1, LIST *lp2);
extern DLL LISTELEM *listelement(LIST *, long);
extern DLL LIST *listsegment(LIST *, long, long);


/*
 * Structures for associations.
 * Associations are "indexed" by one or more arbitrary values, and are
 * stored in a hash table with their hash values for quick indexing.
 */
typedef struct assocelem ASSOCELEM;
struct assocelem {
	ASSOCELEM *e_next;	/* next element in list (or NULL) */
	long e_dim;		/* dimension of indexing for this element */
	QCKHASH e_hash;		/* hash value for this element */
	VALUE e_value;		/* value of association */
	VALUE e_indices[1];	/* index values (variable length) */
};


struct assoc {
	long a_count;		/* number of elements in the association */
	long a_size;		/* current size of association hash table */
	ASSOCELEM **a_table;	/* current hash table for elements */
};


extern DLL ASSOC *assocalloc(long initsize);
extern DLL ASSOC *assoccopy(ASSOC *ap);
extern DLL void assocfree(ASSOC *ap);
extern DLL void assocprint(ASSOC *ap, long max_print);
extern DLL int assocsearch(ASSOC *ap, VALUE *vp, long start, long end, ZVALUE *index);
extern DLL int assocrsearch(ASSOC *ap, VALUE *vp, long start, long end, ZVALUE *index);
extern DLL BOOL assoccmp(ASSOC *ap1, ASSOC *ap2);
extern DLL VALUE *assocfindex(ASSOC *ap, long index);
extern DLL VALUE *associndex(ASSOC *ap, BOOL create, long dim, VALUE *indices);


/*
 * Object actions.
 */
#define OBJ_PRINT	0	/* print the value */
#define OBJ_ONE		1	/* create the multiplicative identity */
#define OBJ_TEST	2	/* test a value for "zero" */
#define OBJ_ADD		3	/* add two values */
#define OBJ_SUB		4	/* subtrace one value from another */
#define OBJ_NEG		5	/* negate a value */
#define OBJ_MUL		6	/* multiply two values */
#define OBJ_DIV		7	/* divide one value by another */
#define OBJ_INV		8	/* invert a value */
#define OBJ_ABS		9	/* take absolute value of value */
#define OBJ_NORM	10	/* take the norm of a value */
#define OBJ_CONJ	11	/* take the conjugate of a value */
#define OBJ_POW		12	/* take the power function */
#define OBJ_SGN		13	/* return the sign of a value */
#define OBJ_CMP		14	/* compare two values for equality */
#define OBJ_REL		15	/* compare two values for inequality */
#define OBJ_QUO		16	/* integer quotient of values */
#define OBJ_MOD		17	/* remainder of division of values */
#define OBJ_INT		18	/* integer part of */
#define OBJ_FRAC	19	/* fractional part of */
#define OBJ_INC		20	/* increment by one */
#define OBJ_DEC		21	/* decrement by one */
#define OBJ_SQUARE	22	/* square value */
#define OBJ_SCALE	23	/* scale by power of two */
#define OBJ_SHIFT	24	/* shift left (or right) by number of bits */
#define OBJ_ROUND	25	/* round to specified decimal places */
#define OBJ_BROUND	26	/* round to specified binary places */
#define OBJ_ROOT	27	/* take nth root of value */
#define OBJ_SQRT	28	/* take square root of value */
#define OBJ_OR		29	/* take bitwise or of values */
#define OBJ_AND		30	/* take bitwise and of values */
#define OBJ_NOT		31	/* take logical not of value */
#define OBJ_FACT	32	/* factorial or postfix ! */
#define OBJ_MIN		33	/* minimum value */
#define OBJ_MAX		34	/* maximum value */
#define OBJ_SUM		35	/* sum value */
#define OBJ_ASSIGN	36	/* assign value */
#define OBJ_XOR		37	/* ~ difference of values */
#define OBJ_COMP	38	/* ~ complement of value */
#define OBJ_CONTENT	39	/* unary hash op */
#define OBJ_HASHOP	40	/* binary hash op */
#define OBJ_BACKSLASH	41	/* unary backslash op */
#define OBJ_SETMINUS	42	/* binary backslash op */
#define OBJ_PLUS	43	/* unary + op */
#define OBJ_MAXFUNC	43	/* highest function */


/*
 * Definition of an object type.
 * This is actually a varying sized structure.
 */
typedef struct {
	int oa_index;			/* index of object type */
	int oa_count;			/* number of elements defined */
	long oa_indices[OBJ_MAXFUNC+1]; /* function indices for actions */
	int oa_elements[1];		/* element indices (MUST BE LAST) */
} OBJECTACTIONS;

#define objectactionsize(elements) \
	(sizeof(OBJECTACTIONS) + ((elements) - 1) * sizeof(int))


/*
 * Structure of an object.
 * This is actually a varying sized structure.
 * However, there are always at least USUAL_ELEMENTS values in the object.
 */
struct object {
	OBJECTACTIONS *o_actions;	/* action table for this object */
	VALUE o_table[USUAL_ELEMENTS];	/* object values (MUST BE LAST) */
};

#define objectsize(elements) \
	(sizeof(OBJECT) + ((elements) - USUAL_ELEMENTS) * sizeof(VALUE))


extern DLL OBJECT *objcopy(OBJECT *op);
extern DLL OBJECT *objalloc(long index);
extern DLL VALUE objcall(int action, VALUE *v1, VALUE *v2, VALUE *v3);
extern DLL void objfree(OBJECT *op);
extern DLL int addelement(char *name);
extern DLL int defineobject(char *name, int indices[], int count);
extern DLL int checkobject(char *name);
extern DLL void showobjfuncs(void);
extern DLL void showobjtypes(void);
extern DLL int findelement(char *name);
extern DLL char *objtypename(unsigned long index);
extern DLL int objoffset(OBJECT *op, long index);


/*
 * Configuration parameter name and type.
 */
extern NAMETYPE configs[];
extern DLL void config_value(CONFIG *cfg, int type, VALUE *ret);
extern DLL void setconfig(int type, VALUE *vp);
extern DLL void config_print(CONFIG *cfg);	/* the CONFIG to print */


/*
 * size, memsize and sizeof support
 */
extern DLL long elm_count(VALUE *vp);
extern DLL long lsizeof(VALUE *vp);
extern DLL long memsize(VALUE *vp);

/*
 * String functions
 */
extern DLL STRING *stringadd(STRING *, STRING *);
extern DLL STRING *stringcopy(STRING *);
extern DLL STRING *stringsub(STRING *, STRING *);
extern DLL STRING *stringmul(NUMBER *, STRING *);
extern DLL STRING *stringand(STRING *, STRING *);
extern DLL STRING *stringor(STRING *, STRING *);
extern DLL STRING *stringxor(STRING *, STRING *);
extern DLL STRING *stringdiff(STRING *, STRING *);
extern DLL STRING *stringsegment(STRING *, long, long);
extern DLL STRING *stringshift(STRING *, long);
extern DLL STRING *stringcomp(STRING *);
extern DLL STRING *stringneg(STRING *);
extern DLL STRING *stringcpy(STRING *, STRING *);
extern DLL STRING *stringncpy(STRING *, STRING *, long);
extern DLL long stringcontent(STRING *s);
extern DLL long stringlowbit(STRING *s);
extern DLL long stringhighbit(STRING *s);
extern DLL BOOL stringcmp(STRING *, STRING *);
extern DLL BOOL stringrel(STRING *, STRING *);
extern DLL int stringbit(STRING *, long);
extern DLL BOOL stringtest(STRING *);
extern DLL int stringsetbit(STRING *, long, BOOL);
extern DLL int stringsearch(STRING *, STRING *, long, long, ZVALUE *);
extern DLL int stringrsearch(STRING *, STRING *, long, long, ZVALUE *);

#endif /* !__VALUE_H__ */
