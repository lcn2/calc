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
 * @(#) $Revision: 29.4 $
 * @(#) $Id: value.h,v 29.4 2001/02/25 22:07:36 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/value.h,v $
 *
 * Under source code control:	1993/07/30 19:42:47
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__VALUE_H__)
#define __VALUE_H__


#if defined (_WIN32)
#ifdef _EXPORTING
  #define DLL	__declspec(dllexport)
#else
  #define DLL	__declspec(dllimport)
#endif

#else /* Windoz free systems */

  #define DLL

#endif /* Windoz free systems */


#include "cmath.h"
#include "config.h"
#include "shs.h"
#include "calcerr.h"
#include "hash.h"
#include "block.h"
#include "nametype.h"
#include "string.h"

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
DLL extern void freevalue(VALUE *vp);
DLL extern void copyvalue(VALUE *vp, VALUE *vres);
DLL extern void negvalue(VALUE *vp, VALUE *vres);
DLL extern void addvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void subvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void mulvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void orvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void andvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void compvalue(VALUE *vp, VALUE *vres);
DLL extern void xorvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void squarevalue(VALUE *vp, VALUE *vres);
DLL extern void invertvalue(VALUE *vp, VALUE *vres);
DLL extern void roundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
DLL extern void broundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
DLL extern void setminusvalue(VALUE *, VALUE *, VALUE *);
DLL extern void backslashvalue(VALUE *, VALUE *);
DLL extern void contentvalue(VALUE *, VALUE *);
DLL extern void hashopvalue(VALUE *, VALUE *, VALUE *);
DLL extern void apprvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
DLL extern void intvalue(VALUE *vp, VALUE *vres);
DLL extern void fracvalue(VALUE *vp, VALUE *vres);
DLL extern void incvalue(VALUE *vp, VALUE *vres);
DLL extern void decvalue(VALUE *vp, VALUE *vres);
DLL extern void conjvalue(VALUE *vp, VALUE *vres);
DLL extern void sqrtvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
DLL extern void rootvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
DLL extern void absvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void normvalue(VALUE *vp, VALUE *vres);
DLL extern void shiftvalue(VALUE *v1, VALUE *v2, BOOL rightshift, VALUE *vres);
DLL extern void scalevalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void powivalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void powervalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
DLL extern void divvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void quovalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
DLL extern void modvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
DLL extern BOOL testvalue(VALUE *vp);
DLL extern BOOL comparevalue(VALUE *v1, VALUE *v2);
DLL extern BOOL acceptvalue(VALUE *v1, VALUE *v2);
DLL extern void relvalue(VALUE *v1, VALUE *v2, VALUE *vres);
DLL extern void sgnvalue(VALUE *vp, VALUE *vres);
DLL extern QCKHASH hashvalue(VALUE *vp, QCKHASH val);
DLL extern void printvalue(VALUE *vp, int flags);
DLL extern BOOL precvalue(VALUE *v1, VALUE *v2);
DLL extern VALUE error_value(int e);
DLL extern int set_errno(int e);
DLL extern int set_errcount(int e);
DLL extern long countlistitems(LIST *lp);
DLL extern void addlistitems(LIST *lp, VALUE *vres);
DLL extern void addlistinv(LIST *lp, VALUE *vres);
DLL extern void copy2octet(VALUE *, OCTET *);
DLL extern int copystod(VALUE *, long, long, VALUE *, long);
DLL extern void protectall(VALUE *, int);
DLL extern void set_update(int);


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


DLL extern MATRIX *matadd(MATRIX *m1, MATRIX *m2);
DLL extern MATRIX *matsub(MATRIX *m1, MATRIX *m2);
DLL extern MATRIX *matmul(MATRIX *m1, MATRIX *m2);
DLL extern MATRIX *matneg(MATRIX *m);
DLL extern MATRIX *matalloc(long size);
DLL extern MATRIX *matcopy(MATRIX *m);
DLL extern MATRIX *matinit(MATRIX *m, VALUE *v1, VALUE *v2);
DLL extern MATRIX *matsquare(MATRIX *m);
DLL extern MATRIX *matinv(MATRIX *m);
DLL extern MATRIX *matscale(MATRIX *m, long n);
DLL extern MATRIX *matshift(MATRIX *m, long n);
DLL extern MATRIX *matmulval(MATRIX *m, VALUE *vp);
DLL extern MATRIX *matpowi(MATRIX *m, NUMBER *q);
DLL extern MATRIX *matconj(MATRIX *m);
DLL extern MATRIX *matquoval(MATRIX *m, VALUE *vp, VALUE *v3);
DLL extern MATRIX *matmodval(MATRIX *m, VALUE *vp, VALUE *v3);
DLL extern MATRIX *matint(MATRIX *m);
DLL extern MATRIX *matfrac(MATRIX *m);
DLL extern MATRIX *matappr(MATRIX *m, VALUE *v2, VALUE *v3);
DLL extern VALUE mattrace(MATRIX *m);
DLL extern MATRIX *mattrans(MATRIX *m);
DLL extern MATRIX *matcross(MATRIX *m1, MATRIX *m2);
DLL extern BOOL mattest(MATRIX *m);
DLL extern void matsum(MATRIX *m, VALUE *vres);
DLL extern BOOL matcmp(MATRIX *m1, MATRIX *m2);
DLL extern int matsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
DLL extern int matrsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
DLL extern VALUE matdet(MATRIX *m);
DLL extern VALUE matdot(MATRIX *m1, MATRIX *m2);
DLL extern void matfill(MATRIX *m, VALUE *v1, VALUE *v2);
DLL extern void matfree(MATRIX *m);
DLL extern void matprint(MATRIX *m, long max_print);
DLL extern VALUE *matindex(MATRIX *mp, BOOL create, long dim, VALUE *indices);
DLL extern void matreverse(MATRIX *m);
DLL extern void matsort(MATRIX *m);
DLL extern BOOL matisident(MATRIX *m);
DLL extern MATRIX *matround(MATRIX *m, VALUE *v2, VALUE *v3);
DLL extern MATRIX *matbround(MATRIX *m, VALUE *v2, VALUE *v3);


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


DLL extern void insertlistfirst(LIST *lp, VALUE *vp);
DLL extern void insertlistlast(LIST *lp, VALUE *vp);
DLL extern void insertlistmiddle(LIST *lp, long index, VALUE *vp);
DLL extern void removelistfirst(LIST *lp, VALUE *vp);
DLL extern void removelistlast(LIST *lp, VALUE *vp);
DLL extern void removelistmiddle(LIST *lp, long index, VALUE *vp);
DLL extern void listfree(LIST *lp);
DLL extern void listprint(LIST *lp, long max_print);
DLL extern int listsearch(LIST *lp, VALUE *vp, long start, long end, ZVALUE *index);
DLL extern int listrsearch(LIST *lp, VALUE *vp, long start, long end, ZVALUE *index);
DLL extern BOOL listcmp(LIST *lp1, LIST *lp2);
DLL extern VALUE *listfindex(LIST *lp, long index);
DLL extern LIST *listalloc(void);
DLL extern LIST *listcopy(LIST *lp);
DLL extern void listreverse(LIST *lp);
DLL extern void listsort(LIST *lp);
DLL extern LIST *listappr(LIST *lp, VALUE *v2, VALUE *v3);
DLL extern LIST *listround(LIST *m, VALUE *v2, VALUE *v3);
DLL extern LIST *listbround(LIST *m, VALUE *v2, VALUE *v3);
DLL extern LIST *listquo(LIST *lp, VALUE *v2, VALUE *v3);
DLL extern LIST *listmod(LIST *lp, VALUE *v2, VALUE *v3);
DLL extern BOOL evp(LISTELEM *cp, LISTELEM *x, VALUE *vres);
DLL extern BOOL evalpoly(LIST *clist, LISTELEM *x, VALUE *vres);
DLL extern void insertitems(LIST *lp1, LIST *lp2);
DLL extern LISTELEM *listelement(LIST *, long);
DLL extern LIST *listsegment(LIST *, long, long);


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


DLL extern ASSOC *assocalloc(long initsize);
DLL extern ASSOC *assoccopy(ASSOC *ap);
DLL extern void assocfree(ASSOC *ap);
DLL extern void assocprint(ASSOC *ap, long max_print);
DLL extern int assocsearch(ASSOC *ap, VALUE *vp, long start, long end, ZVALUE *index);
DLL extern int assocrsearch(ASSOC *ap, VALUE *vp, long start, long end, ZVALUE *index);
DLL extern BOOL assoccmp(ASSOC *ap1, ASSOC *ap2);
DLL extern VALUE *assocfindex(ASSOC *ap, long index);
DLL extern VALUE *associndex(ASSOC *ap, BOOL create, long dim, VALUE *indices);


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


DLL extern OBJECT *objcopy(OBJECT *op);
DLL extern OBJECT *objalloc(long index);
DLL extern VALUE objcall(int action, VALUE *v1, VALUE *v2, VALUE *v3);
DLL extern void objfree(OBJECT *op);
DLL extern int addelement(char *name);
DLL extern int defineobject(char *name, int indices[], int count);
DLL extern int checkobject(char *name);
DLL extern void showobjfuncs(void);
DLL extern void showobjtypes(void);
DLL extern int findelement(char *name);
DLL extern char *objtypename(unsigned long index);
DLL extern int objoffset(OBJECT *op, long index);


/*
 * Configuration parameter name and type.
 */
extern NAMETYPE configs[];
DLL extern void config_value(CONFIG *cfg, int type, VALUE *ret);
DLL extern void setconfig(int type, VALUE *vp);
DLL extern void config_print(CONFIG *cfg);	/* the CONFIG to print */


/*
 * size, memsize and sizeof support
 */
DLL extern long elm_count(VALUE *vp);
DLL extern long lsizeof(VALUE *vp);
DLL extern long memsize(VALUE *vp);

/*
 * String functions
 */
DLL extern STRING *stringadd(STRING *, STRING *);
DLL extern STRING *stringcopy(STRING *);
DLL extern STRING *stringsub(STRING *, STRING *);
DLL extern STRING *stringmul(NUMBER *, STRING *);
DLL extern STRING *stringand(STRING *, STRING *);
DLL extern STRING *stringor(STRING *, STRING *);
DLL extern STRING *stringxor(STRING *, STRING *);
DLL extern STRING *stringdiff(STRING *, STRING *);
DLL extern STRING *stringsegment(STRING *, long, long);
DLL extern STRING *stringshift(STRING *, long);
DLL extern STRING *stringcomp(STRING *);
DLL extern STRING *stringneg(STRING *);
DLL extern STRING *stringcpy(STRING *, STRING *);
DLL extern STRING *stringncpy(STRING *, STRING *, long);
DLL extern long stringcontent(STRING *s);
DLL extern long stringlowbit(STRING *s);
DLL extern long stringhighbit(STRING *s);
DLL extern BOOL stringcmp(STRING *, STRING *);
DLL extern BOOL stringrel(STRING *, STRING *);
DLL extern int stringbit(STRING *, long);
DLL extern BOOL stringtest(STRING *);
DLL extern int stringsetbit(STRING *, long, BOOL);
DLL extern int stringsearch(STRING *, STRING *, long, long, ZVALUE *);
DLL extern int stringrsearch(STRING *, STRING *, long, long, ZVALUE *);

#endif /* !__VALUE_H__ */
