/*
 * value - definitions of general values  and related routines used by calc
 *
 * Copyright (C) 1999-2007,2014,2021  David I. Bell
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
 * Under source code control:	1993/07/30 19:42:47
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_VALUE_H)
#define INCLUDE_VALUE_H


#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "decl.h"
# include "cmath.h"
# include "config.h"
# include "sha1.h"
# include "calcerr.h"
# include "hash.h"
# include "block.h"
# include "nametype.h"
# include "str.h"
#else
# include <calc/decl.h>
# include <calc/cmath.h>
# include <calc/config.h>
# include <calc/sha1.h>
# include <calc/calcerr.h>
# include <calc/hash.h>
# include <calc/block.h>
# include <calc/nametype.h>
# include <calc/str.h>
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
	unsigned short v_subtype;	/* other data related to some types */
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
E_FUNC void freevalue(VALUE *vp);
E_FUNC void copyvalue(VALUE *vp, VALUE *vres);
E_FUNC void negvalue(VALUE *vp, VALUE *vres);
E_FUNC void addvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void subvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void mulvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void orvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void andvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void compvalue(VALUE *vp, VALUE *vres);
E_FUNC void xorvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void squarevalue(VALUE *vp, VALUE *vres);
E_FUNC void invertvalue(VALUE *vp, VALUE *vres);
E_FUNC void roundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
E_FUNC void broundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
E_FUNC void setminusvalue(VALUE *, VALUE *, VALUE *);
E_FUNC void backslashvalue(VALUE *, VALUE *);
E_FUNC void contentvalue(VALUE *, VALUE *);
E_FUNC void hashopvalue(VALUE *, VALUE *, VALUE *);
E_FUNC void apprvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
E_FUNC void intvalue(VALUE *vp, VALUE *vres);
E_FUNC void fracvalue(VALUE *vp, VALUE *vres);
E_FUNC void incvalue(VALUE *vp, VALUE *vres);
E_FUNC void decvalue(VALUE *vp, VALUE *vres);
E_FUNC void conjvalue(VALUE *vp, VALUE *vres);
E_FUNC void sqrtvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
E_FUNC void rootvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
E_FUNC void absvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void normvalue(VALUE *vp, VALUE *vres);
E_FUNC void shiftvalue(VALUE *v1, VALUE *v2, BOOL rightshift, VALUE *vres);
E_FUNC void scalevalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void powvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void powervalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
E_FUNC void divvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void quovalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
E_FUNC void modvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
E_FUNC BOOL testvalue(VALUE *vp);
E_FUNC BOOL comparevalue(VALUE *v1, VALUE *v2);
E_FUNC BOOL acceptvalue(VALUE *v1, VALUE *v2);
E_FUNC void relvalue(VALUE *v1, VALUE *v2, VALUE *vres);
E_FUNC void sgnvalue(VALUE *vp, VALUE *vres);
E_FUNC QCKHASH hashvalue(VALUE *vp, QCKHASH val);
E_FUNC void printvalue(VALUE *vp, int flags);
E_FUNC void printestr(VALUE *vp);
E_FUNC BOOL precvalue(VALUE *v1, VALUE *v2);
E_FUNC VALUE error_value(int e);
E_FUNC int set_errno(int e);
E_FUNC int set_errcount(int e);
E_FUNC long countlistitems(LIST *lp);
E_FUNC void addlistitems(LIST *lp, VALUE *vres);
E_FUNC void addlistinv(LIST *lp, VALUE *vres);
E_FUNC void copy2octet(VALUE *, OCTET *);
E_FUNC int copystod(VALUE *, long, long, VALUE *, long);
E_FUNC void protecttodepth(VALUE *, int, int);
E_FUNC void set_update(int);


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


E_FUNC MATRIX *matadd(MATRIX *m1, MATRIX *m2);
E_FUNC MATRIX *matsub(MATRIX *m1, MATRIX *m2);
E_FUNC MATRIX *matmul(MATRIX *m1, MATRIX *m2);
E_FUNC MATRIX *matneg(MATRIX *m);
E_FUNC MATRIX *matalloc(long size);
E_FUNC MATRIX *matcopy(MATRIX *m);
E_FUNC MATRIX *matinit(MATRIX *m, VALUE *v1, VALUE *v2);
E_FUNC MATRIX *matsquare(MATRIX *m);
E_FUNC MATRIX *matinv(MATRIX *m);
E_FUNC MATRIX *matscale(MATRIX *m, long n);
E_FUNC MATRIX *matshift(MATRIX *m, long n);
E_FUNC MATRIX *matmulval(MATRIX *m, VALUE *vp);
E_FUNC MATRIX *matpowi(MATRIX *m, NUMBER *q);
E_FUNC MATRIX *matconj(MATRIX *m);
E_FUNC MATRIX *matquoval(MATRIX *m, VALUE *vp, VALUE *v3);
E_FUNC MATRIX *matmodval(MATRIX *m, VALUE *vp, VALUE *v3);
E_FUNC MATRIX *matint(MATRIX *m);
E_FUNC MATRIX *matfrac(MATRIX *m);
E_FUNC MATRIX *matappr(MATRIX *m, VALUE *v2, VALUE *v3);
E_FUNC VALUE mattrace(MATRIX *m);
E_FUNC MATRIX *mattrans(MATRIX *m);
E_FUNC MATRIX *matcross(MATRIX *m1, MATRIX *m2);
E_FUNC BOOL mattest(MATRIX *m);
E_FUNC void matsum(MATRIX *m, VALUE *vres);
E_FUNC BOOL matcmp(MATRIX *m1, MATRIX *m2);
E_FUNC int matsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
E_FUNC int matrsearch(MATRIX *m, VALUE *vp, long start, long end,
		      ZVALUE *index);
E_FUNC VALUE matdet(MATRIX *m);
E_FUNC VALUE matdot(MATRIX *m1, MATRIX *m2);
E_FUNC void matfill(MATRIX *m, VALUE *v1, VALUE *v2);
E_FUNC void matfree(MATRIX *m);
E_FUNC void matprint(MATRIX *m, long max_print);
E_FUNC VALUE *matindex(MATRIX *mp, BOOL create, long dim, VALUE *indices);
E_FUNC void matreverse(MATRIX *m);
E_FUNC void matsort(MATRIX *m);
E_FUNC BOOL matisident(MATRIX *m);
E_FUNC MATRIX *matround(MATRIX *m, VALUE *v2, VALUE *v3);
E_FUNC MATRIX *matbround(MATRIX *m, VALUE *v2, VALUE *v3);


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


E_FUNC void insertlistfirst(LIST *lp, VALUE *vp);
E_FUNC void insertlistlast(LIST *lp, VALUE *vp);
E_FUNC void insertlistmiddle(LIST *lp, long index, VALUE *vp);
E_FUNC void removelistfirst(LIST *lp, VALUE *vp);
E_FUNC void removelistlast(LIST *lp, VALUE *vp);
E_FUNC void removelistmiddle(LIST *lp, long index, VALUE *vp);
E_FUNC void listfree(LIST *lp);
E_FUNC void listprint(LIST *lp, long max_print);
E_FUNC int listsearch(LIST *lp, VALUE *vp, long start, long end, ZVALUE *index);
E_FUNC int listrsearch(LIST *lp, VALUE *vp, long start, long end,
		       ZVALUE *index);
E_FUNC BOOL listcmp(LIST *lp1, LIST *lp2);
E_FUNC VALUE *listfindex(LIST *lp, long index);
E_FUNC LIST *listalloc(void);
E_FUNC LIST *listcopy(LIST *lp);
E_FUNC void listreverse(LIST *lp);
E_FUNC void listsort(LIST *lp);
E_FUNC LIST *listappr(LIST *lp, VALUE *v2, VALUE *v3);
E_FUNC LIST *listround(LIST *m, VALUE *v2, VALUE *v3);
E_FUNC LIST *listbround(LIST *m, VALUE *v2, VALUE *v3);
E_FUNC LIST *listquo(LIST *lp, VALUE *v2, VALUE *v3);
E_FUNC LIST *listmod(LIST *lp, VALUE *v2, VALUE *v3);
E_FUNC BOOL evp(LISTELEM *cp, LISTELEM *x, VALUE *vres);
E_FUNC BOOL evalpoly(LIST *clist, LISTELEM *x, VALUE *vres);
E_FUNC void insertitems(LIST *lp1, LIST *lp2);
E_FUNC LISTELEM *listelement(LIST *, long);
E_FUNC LIST *listsegment(LIST *, long, long);


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


E_FUNC ASSOC *assocalloc(long initsize);
E_FUNC ASSOC *assoccopy(ASSOC *ap);
E_FUNC void assocfree(ASSOC *ap);
E_FUNC void assocprint(ASSOC *ap, long max_print);
E_FUNC int assocsearch(ASSOC *ap, VALUE *vp, long start, long end,
		       ZVALUE *index);
E_FUNC int assocrsearch(ASSOC *ap, VALUE *vp, long start, long end,
			ZVALUE *index);
E_FUNC BOOL assoccmp(ASSOC *ap1, ASSOC *ap2);
E_FUNC VALUE *assocfindex(ASSOC *ap, long index);
E_FUNC VALUE *associndex(ASSOC *ap, BOOL create, long dim, VALUE *indices);


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


E_FUNC OBJECT *objcopy(OBJECT *op);
E_FUNC OBJECT *objalloc(long index);
E_FUNC VALUE objcall(int action, VALUE *v1, VALUE *v2, VALUE *v3);
E_FUNC void objfree(OBJECT *op);
E_FUNC int addelement(char *name);
E_FUNC int defineobject(char *name, int indices[], int count);
E_FUNC int checkobject(char *name);
E_FUNC void showobjfuncs(void);
E_FUNC void showobjtypes(void);
E_FUNC int findelement(char *name);
E_FUNC char *objtypename(unsigned long index);
E_FUNC int objoffset(OBJECT *op, long index);


/*
 * Configuration parameter name and type.
 */
EXTERN NAMETYPE configs[];
E_FUNC void config_value(CONFIG *cfg, int type, VALUE *ret);
E_FUNC void setconfig(int type, VALUE *vp);
E_FUNC void config_print(CONFIG *cfg);	/* the CONFIG to print */


/*
 * size, memsize and sizeof support
 */
E_FUNC long elm_count(VALUE *vp);
E_FUNC size_t lsizeof(VALUE *vp);
E_FUNC size_t memsize(VALUE *vp);

/*
 * String functions
 */
E_FUNC STRING *stringadd(STRING *, STRING *);
E_FUNC STRING *stringcopy(STRING *);
E_FUNC STRING *stringsub(STRING *, STRING *);
E_FUNC STRING *stringmul(NUMBER *, STRING *);
E_FUNC STRING *stringand(STRING *, STRING *);
E_FUNC STRING *stringor(STRING *, STRING *);
E_FUNC STRING *stringxor(STRING *, STRING *);
E_FUNC STRING *stringdiff(STRING *, STRING *);
E_FUNC STRING *stringsegment(STRING *, long, long);
E_FUNC STRING *stringshift(STRING *, long);
E_FUNC STRING *stringcomp(STRING *);
E_FUNC STRING *stringneg(STRING *);
E_FUNC STRING *stringtolower(STRING *);
E_FUNC STRING *stringtoupper(STRING *);
E_FUNC STRING *stringcpy(STRING *, STRING *);
E_FUNC STRING *stringncpy(STRING *, STRING *, size_t);
E_FUNC long stringcontent(STRING *s);
E_FUNC long stringlowbit(STRING *s);
E_FUNC long stringhighbit(STRING *s);
E_FUNC BOOL stringcmp(STRING *, STRING *);
E_FUNC BOOL stringrel(STRING *, STRING *);
E_FUNC BOOL stringcaserel(STRING *, STRING *);
E_FUNC int stringbit(STRING *, long);
E_FUNC BOOL stringtest(STRING *);
E_FUNC int stringsetbit(STRING *, long, BOOL);
E_FUNC int stringsearch(STRING *, STRING *, long, long, ZVALUE *);
E_FUNC int stringrsearch(STRING *, STRING *, long, long, ZVALUE *);

#endif /* !INCLUDE_VALUE_H */
