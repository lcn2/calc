/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Definitions of general values and related routines used by the calculator.
 */


#if !defined(__VALUE_H__)
#define	__VALUE_H__


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
#define	PRINT_NORMAL	0x00	/* print in normal manner */
#define	PRINT_SHORT	0x01	/* print in short format (no elements) */
#define	PRINT_UNAMBIG	0x02	/* print in non-ambiguous manner */


/*
 * Definition of values of various types.
 */
typedef struct value VALUE;
typedef struct object OBJECT;
typedef struct matrix MATRIX;
typedef struct list LIST;
typedef	struct assoc ASSOC;
typedef	long FILEID;
typedef	struct rand RAND;
typedef	struct random RANDOM;


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
#define	v_file	v_union.vv_file
#define v_num	v_union.vv_num
#define v_com	v_union.vv_com
#define v_addr	v_union.vv_addr
#define v_str	v_union.vv_str
#define v_mat	v_union.vv_mat
#define	v_list	v_union.vv_list
#define	v_assoc	v_union.vv_assoc
#define v_obj	v_union.vv_obj
#define	v_valid	v_union.vv_int
#define v_rand	v_union.vv_rand
#define v_random v_union.vv_random
#define v_config v_union.vv_config
#define v_hash	v_union.vv_hash
#define v_block	v_union.vv_block
#define v_octet	v_union.vv_octet
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
#define	V_LIST	7	/* address of list structure */
#define	V_ASSOC	8	/* address of association structure */
#define V_OBJ	9	/* address of object structure */
#define	V_FILE	10	/* opened file id */
#define V_RAND  11	/* address of additive 55 random state */
#define V_RANDOM 12	/* address of Blum random state */
#define V_CONFIG 13	/* configuration state */
#define V_HASH	14	/* hash state */
#define V_BLOCK	15	/* memory block */
#define V_OCTET 16	/* octet (unsigned char) */
#define V_NBLOCK 17	/* named memory block */
#define V_VPTR	18	/* value address as pointer */
#define V_OPTR	19	/* octet address as pointer */
#define V_SPTR	20	/* string address as pointer */
#define V_NPTR  21	/* number address as pointer */
#define V_MAX	21	/* highest legal value */

#define V_NOSUBTYPE	0       /* subtype has no meaning */
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

#define	NULL_VALUE	((VALUE *) 0)


/*
 * value functions
 */
extern void freevalue(VALUE *vp);
extern void copyvalue(VALUE *vp, VALUE *vres);
extern void negvalue(VALUE *vp, VALUE *vres);
extern void addvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void addnumeric(VALUE *v1, VALUE *v2, VALUE *vres);
extern void subvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void mulvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void orvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void andvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void compvalue(VALUE *vp, VALUE *vres);
extern void xorvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void squarevalue(VALUE *vp, VALUE *vres);
extern void invertvalue(VALUE *vp, VALUE *vres);
extern void roundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void broundvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void setminusvalue(VALUE *, VALUE *, VALUE *);
extern void backslashvalue(VALUE *, VALUE *);
extern void contentvalue(VALUE *, VALUE *);
extern void hashopvalue(VALUE *, VALUE *, VALUE *);
extern void apprvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void intvalue(VALUE *vp, VALUE *vres);
extern void fracvalue(VALUE *vp, VALUE *vres);
extern void incvalue(VALUE *vp, VALUE *vres);
extern void decvalue(VALUE *vp, VALUE *vres);
extern void conjvalue(VALUE *vp, VALUE *vres);
extern void sqrtvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void rootvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void absvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void normvalue(VALUE *vp, VALUE *vres);
extern void shiftvalue(VALUE *v1, VALUE *v2, BOOL rightshift, VALUE *vres);
extern void scalevalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void powivalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void powervalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void divvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void quovalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void modvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern BOOL testvalue(VALUE *vp);
extern BOOL comparevalue(VALUE *v1, VALUE *v2);
extern BOOL acceptvalue(VALUE *v1, VALUE *v2);
extern void relvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void sgnvalue(VALUE *vp, VALUE *vres);
extern QCKHASH hashvalue(VALUE *vp, QCKHASH val);
extern void printvalue(VALUE *vp, int flags);
extern BOOL precvalue(VALUE *v1, VALUE *v2);
extern VALUE error_value(int e);
extern int set_errno(int e);
extern int set_errcount(int e);
extern long countlistitems(LIST *lp);
extern void addlistitems(LIST *lp, VALUE *vres);
extern void addlistinv(LIST *lp, VALUE *vres);
extern void copy2octet(VALUE *, OCTET *);
extern int copystod(VALUE *, long, long, VALUE *, long);
extern void protectall(VALUE *, int);
extern void set_update(int);


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


extern MATRIX *matadd(MATRIX *m1, MATRIX *m2);
extern MATRIX *matsub(MATRIX *m1, MATRIX *m2);
extern MATRIX *matmul(MATRIX *m1, MATRIX *m2);
extern MATRIX *matneg(MATRIX *m);
extern MATRIX *matalloc(long size);
extern MATRIX *matcopy(MATRIX *m);
extern MATRIX *matinit(MATRIX *m, VALUE *v1, VALUE *v2);
extern MATRIX *matsquare(MATRIX *m);
extern MATRIX *matinv(MATRIX *m);
extern MATRIX *matscale(MATRIX *m, long n);
extern MATRIX *matshift(MATRIX *m, long n);
extern MATRIX *matmulval(MATRIX *m, VALUE *vp);
extern MATRIX *matpowi(MATRIX *m, NUMBER *q);
extern MATRIX *matconj(MATRIX *m);
extern MATRIX *matquoval(MATRIX *m, VALUE *vp, VALUE *v3);
extern MATRIX *matmodval(MATRIX *m, VALUE *vp, VALUE *v3);
extern MATRIX *matint(MATRIX *m);
extern MATRIX *matfrac(MATRIX *m);
extern MATRIX *matappr(MATRIX *m, VALUE *v2, VALUE *v3);
extern VALUE mattrace(MATRIX *m);
extern MATRIX *mattrans(MATRIX *m);
extern MATRIX *matcross(MATRIX *m1, MATRIX *m2);
extern BOOL mattest(MATRIX *m);
extern void matsum(MATRIX *m, VALUE *vres);
extern BOOL matcmp(MATRIX *m1, MATRIX *m2);
extern int matsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
extern int matrsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
extern VALUE matdet(MATRIX *m);
extern VALUE matdot(MATRIX *m1, MATRIX *m2);
extern void matfill(MATRIX *m, VALUE *v1, VALUE *v2);
extern void matfree(MATRIX *m);
extern void matprint(MATRIX *m, long max_print);
extern VALUE *matindex(MATRIX *mp, BOOL create, long dim, VALUE *indices);
extern void matreverse(MATRIX *m);
extern void matsort(MATRIX *m);
extern BOOL matisident(MATRIX *m);
extern MATRIX *matround(MATRIX *m, VALUE *v2, VALUE *v3);
extern MATRIX *matbround(MATRIX *m, VALUE *v2, VALUE *v3);


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


extern void insertlistfirst(LIST *lp, VALUE *vp);
extern void insertlistlast(LIST *lp, VALUE *vp);
extern void insertlistmiddle(LIST *lp, long index, VALUE *vp);
extern void removelistfirst(LIST *lp, VALUE *vp);
extern void removelistlast(LIST *lp, VALUE *vp);
extern void removelistmiddle(LIST *lp, long index, VALUE *vp);
extern void listfree(LIST *lp);
extern void listprint(LIST *lp, long max_print);
extern int listsearch(LIST *lp, VALUE *vp, long start, long end, ZVALUE *index);
extern int listrsearch(LIST *lp, VALUE *vp, long start, long end, ZVALUE *index);
extern BOOL listcmp(LIST *lp1, LIST *lp2);
extern VALUE *listfindex(LIST *lp, long index);
extern LIST *listalloc(void);
extern LIST *listcopy(LIST *lp);
extern void listreverse(LIST *lp);
extern void listsort(LIST *lp);
extern LIST *listappr(LIST *lp, VALUE *v2, VALUE *v3);
extern LIST *listround(LIST *m, VALUE *v2, VALUE *v3);
extern LIST *listbround(LIST *m, VALUE *v2, VALUE *v3);
extern LIST *listquo(LIST *lp, VALUE *v2, VALUE *v3);
extern LIST *listmod(LIST *lp, VALUE *v2, VALUE *v3);
extern BOOL evp(LISTELEM *cp, LISTELEM *x, VALUE *vres);
extern BOOL evalpoly(LIST *clist, LISTELEM *x, VALUE *vres);
extern void insertitems(LIST *lp1, LIST *lp2);
extern LISTELEM *listelement(LIST *, long);
extern LIST *listsegment(LIST *, long, long);


/*
 * Structures for associations.
 * Associations are "indexed" by one or more arbitrary values, and are
 * stored in a hash table with their hash values for quick indexing.
 */
typedef	struct assocelem ASSOCELEM;
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


extern ASSOC *assocalloc(long initsize);
extern ASSOC *assoccopy(ASSOC *ap);
extern void assocfree(ASSOC *ap);
extern void assocprint(ASSOC *ap, long max_print);
extern int assocsearch(ASSOC *ap, VALUE *vp, long start, long end, ZVALUE *index);
extern int assocrsearch(ASSOC *ap, VALUE *vp, long start, long end, ZVALUE *index);
extern BOOL assoccmp(ASSOC *ap1, ASSOC *ap2);
extern VALUE *assocfindex(ASSOC *ap, long index);
extern VALUE *associndex(ASSOC *ap, BOOL create, long dim, VALUE *indices);


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
	char *name;			/* name of object */
	int count;			/* number of elements defined */
	long actions[OBJ_MAXFUNC+1];	/* function indices for actions */
	int elements[1];		/* element indexes (MUST BE LAST) */
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


extern OBJECT *objcopy(OBJECT *op);
extern OBJECT *objalloc(long index);
extern VALUE objcall(int action, VALUE *v1, VALUE *v2, VALUE *v3);
extern void objfree(OBJECT *op);
extern void objuncache(void);
extern int addelement(char *name);
extern int defineobject(char *name, int indices[], int count);
extern int checkobject(char *name);
extern void showobjfuncs(void);
extern void showobjtypes(void);
extern int findelement(char *name);
extern int objoffset(OBJECT *op, long index);


/*
 * Configuration parameter name and type.
 */
extern NAMETYPE configs[];
extern void config_value(CONFIG *cfg, int type, VALUE *ret);
extern void setconfig(int type, VALUE *vp);
extern void config_print(CONFIG *cfg);	/* the CONFIG to print */


/*
 * size, memsize and sizeof support
 */
extern long elm_count(VALUE *vp);
extern long lsizeof(VALUE *vp);
extern long memsize(VALUE *vp);

/*
 * String functions
 */
extern STRING *stringadd(STRING *, STRING *);
extern STRING *stringcopy(STRING *);
extern STRING *stringsub(STRING *, STRING *);
extern STRING *stringmul(NUMBER *, STRING *);
extern STRING *stringand(STRING *, STRING *);
extern STRING *stringor(STRING *, STRING *);
extern STRING *stringxor(STRING *, STRING *);
extern STRING *stringdiff(STRING *, STRING *);
extern STRING *stringsegment(STRING *, long, long);
extern STRING *stringshift(STRING *, long);
extern STRING *stringcomp(STRING *);
extern STRING *stringneg(STRING *);
extern STRING *stringcpy(STRING *, STRING *);
extern STRING *stringncpy(STRING *, STRING *, long);
extern long stringcontent(STRING *s);
extern long stringlowbit(STRING *s);
extern long stringhighbit(STRING *s);
extern BOOL stringcmp(STRING *, STRING *);
extern BOOL stringrel(STRING *, STRING *);
extern int stringbit(STRING *, long);
extern BOOL stringtest(STRING *);
extern int stringsetbit(STRING *, long, BOOL);
extern int stringsearch(STRING *, STRING *, long, long, ZVALUE *);
extern int stringrsearch(STRING *, STRING *, long, long, ZVALUE *);

#endif /* !__VALUE_H__ */
