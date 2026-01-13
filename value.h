/*
 * value - definitions of general values  and related routines used by calc
 *
 * Copyright (C) 1999-2007,2014,2021,2023,2026  David I. Bell
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
 * Under source code control:   1993/07/30 19:42:47
 * File existed as early as:    1993
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_VALUE_H)
#  define INCLUDE_VALUE_H

#  if defined(CALC_SRC) /* if we are building from the calc source tree */
#    include "zmath.h"
#    include "qmath.h"
#    include "cmath.h"
#    include "config.h"
#    include "sha1.h"
#    include "hash.h"
#    include "block.h"
#    include "nametype.h"
#    include "str.h"
#    include "zrand.h"
#    include "zrandom.h"
#  else
#    include <calc/zmath.h>
#    include <calc/qmath.h>
#    include <calc/cmath.h>
#    include <calc/config.h>
#    include <calc/sha1.h>
#    include <calc/hash.h>
#    include <calc/block.h>
#    include <calc/nametype.h>
#    include <calc/str.h>
#    include <calc/zrand.h>
#    include <calc/zrandom.h>
#  endif

#  define MAXDIM 4         /* maximum number of dimensions in matrices */
#  define USUAL_ELEMENTS 4 /* usual number of elements for objects */

/*
 * Flags to modify results from the printvalue routine.
 * These flags are OR'd together.
 */
#  define PRINT_NORMAL 0x00  /* print in normal manner */
#  define PRINT_SHORT 0x01   /* print in short format (no elements) */
#  define PRINT_UNAMBIG 0x02 /* print in non-ambiguous manner */

/*
 * Definition of values of various types.
 */
typedef struct object OBJECT;
typedef struct matrix MATRIX;
typedef struct list LIST;
typedef struct assoc ASSOC;
typedef long FILEID;
typedef struct value VALUE;

/*
 * calc values
 *
 * See below for information on what needs to be added for a new type.
 *
 * NOTE: The v_type can be a negative value.  This happens when
 *       an error is returned as a VALUE.
 */
struct value {
    int16_t v_type;           /* type of value - IMPORTANT: v_type < 0 is an error code */
    uint16_t unused;          /* reserved for future use, calloc-ed to 0 */
    uint32_t v_subtype;       /* other data related to some types */
    union {                   /* types of values (see V_XYZ below) */
        long vv_int;          /* 1: small integer value */
        NUMBER *vv_num;       /* 2, 21: real number */
        COMPLEX *vv_com;      /* 3: complex number */
        VALUE *vv_addr;       /* 4, 18: address of variable value */
        STRING *vv_str;       /* 5, 20: string value */
        MATRIX *vv_mat;       /* 6: address of matrix */
        LIST *vv_list;        /* 7: address of list */
        ASSOC *vv_assoc;      /* 8: address of association */
        OBJECT *vv_obj;       /* 9: address of object */
        FILEID vv_file;       /* 10: id of opened file */
        RAND *vv_rand;        /* 11: subtractive 100 random state */
        RANDOM *vv_random;    /* 12: Blum random state */
        CONFIG *vv_config;    /* 13: configuration state */
        HASH *vv_hash;        /* 14: hash state */
        BLOCK *vv_block;      /* 15: memory block */
        OCTET *vv_octet;      /* 16, 19: octet addr (unsigned char) */
        NBLOCK *vv_nblock;    /* 17: named memory block */
    } v_union;
};

/*
 * For ease in referencing
 */
#  define v_int v_union.vv_int
#  define v_file v_union.vv_file
#  define v_num v_union.vv_num
#  define v_com v_union.vv_com
#  define v_addr v_union.vv_addr
#  define v_str v_union.vv_str
#  define v_mat v_union.vv_mat
#  define v_list v_union.vv_list
#  define v_assoc v_union.vv_assoc
#  define v_obj v_union.vv_obj
#  define v_valid v_union.vv_int
#  define v_rand v_union.vv_rand
#  define v_random v_union.vv_random
#  define v_config v_union.vv_config
#  define v_hash v_union.vv_hash
#  define v_block v_union.vv_block
#  define v_octet v_union.vv_octet
#  define v_nblock v_union.vv_nblock

/*
 * Value types.
 *
 * NOTE: The following files should be checked/adjusted for a new type:
 *
 *      size.c          - elm_count(), lsizeof()
 *      help/size       - update what the size() builtin will report
 *      hash.c          - hash_value()
 *      quickhash.c     - hashvalue()
 *      value.c         - freevalue(), copyvalue(), comparevalue(),
 *                        printvalue(), testvalue(), relvalue(),
 *                        sgnvalue(), printestr()
 *
 * There may be others, but at is at least a start.
 *
 * NOTE: A v_type < 0 is an error code.  See "errsym.h".
 */
#  define V_NULL 0    /* null value */
#  define V_INT 1     /* normal integer */
#  define V_NUM 2     /* number */
#  define V_COM 3     /* complex number */
#  define V_ADDR 4    /* address of variable value */
#  define V_STR 5     /* address of string */
#  define V_MAT 6     /* address of matrix structure */
#  define V_LIST 7    /* address of list structure */
#  define V_ASSOC 8   /* address of association structure */
#  define V_OBJ 9     /* address of object structure */
#  define V_FILE 10   /* opened file id */
#  define V_RAND 11   /* address of subtractive 100 random state */
#  define V_RANDOM 12 /* address of Blum random state */
#  define V_CONFIG 13 /* configuration state */
#  define V_HASH 14   /* hash state */
#  define V_BLOCK 15  /* memory block */
#  define V_OCTET 16  /* octet (unsigned char) */
#  define V_NBLOCK 17 /* named memory block */
#  define V_VPTR 18   /* value address as pointer */
#  define V_OPTR 19   /* octet address as pointer */
#  define V_SPTR 20   /* string address as pointer */
#  define V_NPTR 21   /* number address as pointer */

#  define V_MAX V_NPTR /* highest legal value - must be last and match highest V_something value */

/*
 * v_subtype values
 */
#  define V_NOSUBTYPE 0        /* subtype has no meaning */
#  define V_NOASSIGNTO 0x001   /* protection status 1 */
#  define V_NONEWVALUE 0x002   /* protection status 2 */
#  define V_NONEWTYPE 0x004    /* protection status 4 */
#  define V_NOERROR 0x008      /* protection status 8 */
#  define V_NOCOPYTO 0x010     /* protection status 16 */
#  define V_NOREALLOC 0x020    /* protection status 32 */
#  define V_NOASSIGNFROM 0x040 /* protection status 64 */
#  define V_NOCOPYFROM 0x080   /* protection status 128 */
#  define V_PROTECTALL 0x100   /* protection status 256 */

#  define MAXPROTECT 0x1ff /* OR of all of the above protection statuses */

/*
 * At present protect(var, sts) determines bits in var->v_subtype
 * corresponding to 4 * sts.  MAXPROTECT is the sum of the simple
 * (power of two) protection status values.
 *
 * XXX - consider issue #52 with respect to protect - XXX
 */

/*
 * NOTE: The shift of 8 in TWOVAL() macro below assumes V_MAX < 1<<8
 *
 * The macro TWOVAL_ARGS_OK(a,b) will return true if both a and b are in range,
 * otherwise it will return false.
 *
 * The TWOVAL_INVALID is a value that TWOVAL_ARGS_OK(a,b) is true,
 * will never match a TWOVAL(a,b) value (i.e., using V_something defined values).
 *
 * The TWOVAL_AS_UINT(a,b) may be assigned to a unsigned int value so that
 * with one switches on that unsigned int, cases with a and/or b being
 * out of range will fall into the default (non-matching) case.
 *
 *      unsigned int twoval_as_uint;
 *
 *      ...
 *
 *      twoval_as_uint = TWOVAL_AS_UINT(a,b);
 *      switch (twoval_as_uint) {
 *              case TWOVAL(V_INT,V_INT):
 *                      ...
 *                      break;
 *              case TWOVAL(V_INT,V_NUM):
 *                      ...
 *                      break;
 *              default:
 *                      ...
 *                      break;
 *      }
 *
 *       If a is NOT 0 <= a <= V_MAX or if b is NOT 0 <= b <= V_MAX,
 *       when () macro returns -1 (all bits set) in order to
 *       not match and true TWOVAL() macro combination that uses
 *       uses a V_something defined value above.
 */

#  define TWOVAL(a, b) ((unsigned int)(((a) << 8) | (b))) /* logical OR for switch of two V_something values */
#  define TWOVAL_ARGS_OK(a, b) (((a) >= 0) && ((a) <= V_MAX) && ((b) >= 0) && ((b) <= V_MAX))
#  define TWOVAL_INVALID ((unsigned int)(-1)) /* never a valid TWOVAL(a,b) value when a and b are in range */
#  define TWOVAL_AS_UINT(a, b) (TWOVAL_ARGS_OK(a, b) ? TWOVAL(a, b) : TWOVAL_INVALID)

#  define NULL_VALUE ((VALUE *)0)

/*
 * value functions
 */
extern void freevalue(VALUE *vp);
extern void copyvalue(VALUE *vp, VALUE *vres);
extern void negvalue(VALUE *vp, VALUE *vres);
extern void addvalue(VALUE *v1, VALUE *v2, VALUE *vres);
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
extern void shiftvalue(VALUE *v1, VALUE *v2, bool rightshift, VALUE *vres);
extern void scalevalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void powvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void powervalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void divvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void quovalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern void modvalue(VALUE *v1, VALUE *v2, VALUE *v3, VALUE *vres);
extern bool testvalue(VALUE *vp);
extern bool comparevalue(VALUE *v1, VALUE *v2);
extern bool acceptvalue(VALUE *v1, VALUE *v2);
extern void relvalue(VALUE *v1, VALUE *v2, VALUE *vres);
extern void sgnvalue(VALUE *vp, VALUE *vres);
extern QCKHASH hashvalue(VALUE *vp, QCKHASH val);
extern void printvalue(VALUE *vp, int flags);
extern void printestr(VALUE *vp);
extern bool precvalue(VALUE *v1, VALUE *v2);
extern VALUE error_value(int16_t e);
extern int set_errno(int e);
extern int set_errcount(int e);
extern long countlistitems(LIST *lp);
extern void addlistitems(LIST *lp, VALUE *vres);
extern void addlistinv(LIST *lp, VALUE *vres);
extern void copy2octet(VALUE *, OCTET *);
extern void protecttodepth(VALUE *, int, int);
extern void set_update(int);

/*
 * Structure of a matrix.
 */
struct matrix {
    long m_dim;         /* dimension of matrix */
    long m_size;        /* total number of elements */
    long m_min[MAXDIM]; /* minimum bound for indices */
    long m_max[MAXDIM]; /* maximum bound for indices */
    VALUE m_table[1];   /* actually varying length table */
};

#  define matsize(n) (sizeof(MATRIX) - sizeof(VALUE) + ((n) * sizeof(VALUE)))

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
extern bool mattest(MATRIX *m);
extern void matsum(MATRIX *m, VALUE *vres);
extern bool matcmp(MATRIX *m1, MATRIX *m2);
extern int matsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
extern int matrsearch(MATRIX *m, VALUE *vp, long start, long end, ZVALUE *index);
extern VALUE matdet(MATRIX *m);
extern VALUE matdot(MATRIX *m1, MATRIX *m2);
extern void matfill(MATRIX *m, VALUE *v1, VALUE *v2);
extern void matfree(MATRIX *m);
extern void matprint(MATRIX *m, long max_print);
extern VALUE *matindex(MATRIX *mp, bool create, long dim, VALUE *indices);
extern void matreverse(MATRIX *m);
extern void matsort(MATRIX *m);
extern bool matisident(MATRIX *m);
extern MATRIX *matround(MATRIX *m, VALUE *v2, VALUE *v3);
extern MATRIX *matbround(MATRIX *m, VALUE *v2, VALUE *v3);

/*
 * List definitions.
 * An individual list element.
 */
typedef struct listelem LISTELEM;
struct listelem {
    LISTELEM *e_next; /* next element in list (or NULL) */
    LISTELEM *e_prev; /* previous element in list (or NULL) */
    VALUE e_value;    /* value of this element */
};

/*
 * Structure for a list of elements.
 */
struct list {
    LISTELEM *l_first; /* first list element (or NULL) */
    LISTELEM *l_last;  /* last list element (or NULL) */
    LISTELEM *l_cache; /* cached list element (or NULL) */
    long l_cacheindex; /* index of cached element (or undefined) */
    long l_count;      /* total number of elements in the list */
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
extern bool listcmp(LIST *lp1, LIST *lp2);
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
extern bool evp(LISTELEM *cp, LISTELEM *x, VALUE *vres);
extern bool evalpoly(LIST *clist, LISTELEM *x, VALUE *vres);
extern void insertitems(LIST *lp1, LIST *lp2);
extern LISTELEM *listelement(LIST *, long);
extern LIST *listsegment(LIST *, long, long);

/*
 * Structures for associations.
 * Associations are "indexed" by one or more arbitrary values, and are
 * stored in a hash table with their hash values for quick indexing.
 */
typedef struct assocelem ASSOCELEM;
struct assocelem {
    ASSOCELEM *e_next;  /* next element in list (or NULL) */
    long e_dim;         /* dimension of indexing for this element */
    QCKHASH e_hash;     /* hash value for this element */
    VALUE e_value;      /* value of association */
    VALUE e_indices[1]; /* index values (variable length) */
};

struct assoc {
    long a_count;        /* number of elements in the association */
    long a_size;         /* current size of association hash table */
    ASSOCELEM **a_table; /* current hash table for elements */
};

extern ASSOC *assocalloc(long initsize);
extern ASSOC *assoccopy(ASSOC *ap);
extern void assocfree(ASSOC *ap);
extern void assocprint(ASSOC *ap, long max_print);
extern int assocsearch(ASSOC *ap, VALUE *vp, long start, long end, ZVALUE *index);
extern int assocrsearch(ASSOC *ap, VALUE *vp, long start, long end, ZVALUE *index);
extern bool assoccmp(ASSOC *ap1, ASSOC *ap2);
extern VALUE *assocfindex(ASSOC *ap, long index);
extern VALUE *associndex(ASSOC *ap, bool create, long dim, VALUE *indices);

/*
 * Object actions.
 */
#  define OBJ_PRINT 0      /* print the value */
#  define OBJ_ONE 1        /* create the multiplicative identity */
#  define OBJ_TEST 2       /* test a value for "zero" */
#  define OBJ_ADD 3        /* add two values */
#  define OBJ_SUB 4        /* sub-trace one value from another */
#  define OBJ_NEG 5        /* negate a value */
#  define OBJ_MUL 6        /* multiply two values */
#  define OBJ_DIV 7        /* divide one value by another */
#  define OBJ_INV 8        /* invert a value */
#  define OBJ_ABS 9        /* take absolute value of value */
#  define OBJ_NORM 10      /* take the norm of a value */
#  define OBJ_CONJ 11      /* take the conjugate of a value */
#  define OBJ_POW 12       /* take the power function */
#  define OBJ_SGN 13       /* return the sign of a value */
#  define OBJ_CMP 14       /* compare two values for equality */
#  define OBJ_REL 15       /* compare two values for inequality */
#  define OBJ_QUO 16       /* integer quotient of values */
#  define OBJ_MOD 17       /* remainder of division of values */
#  define OBJ_INT 18       /* integer part of */
#  define OBJ_FRAC 19      /* fractional part of */
#  define OBJ_INC 20       /* increment by one */
#  define OBJ_DEC 21       /* decrement by one */
#  define OBJ_SQUARE 22    /* square value */
#  define OBJ_SCALE 23     /* scale by power of two */
#  define OBJ_SHIFT 24     /* shift left (or right) by number of bits */
#  define OBJ_ROUND 25     /* round to specified decimal places */
#  define OBJ_BROUND 26    /* round to specified binary places */
#  define OBJ_ROOT 27      /* take nth root of value */
#  define OBJ_SQRT 28      /* take square root of value */
#  define OBJ_OR 29        /* take bitwise or of values */
#  define OBJ_AND 30       /* take bitwise and of values */
#  define OBJ_NOT 31       /* take logical not of value */
#  define OBJ_FACT 32      /* factorial or postfix ! */
#  define OBJ_MIN 33       /* minimum value */
#  define OBJ_MAX 34       /* maximum value */
#  define OBJ_SUM 35       /* sum value */
#  define OBJ_ASSIGN 36    /* assign value */
#  define OBJ_XOR 37       /* ~ difference of values */
#  define OBJ_COMP 38      /* ~ complement of value */
#  define OBJ_CONTENT 39   /* unary hash op */
#  define OBJ_HASHOP 40    /* binary hash op */
#  define OBJ_BACKSLASH 41 /* unary backslash op */
#  define OBJ_SETMINUS 42  /* binary backslash op */
#  define OBJ_PLUS 43      /* unary + op */
#  define OBJ_MAXFUNC 43   /* highest function */

/*
 * Definition of an object type.
 * This is actually a varying sized structure.
 */
typedef struct {
    int oa_index;                     /* index of object type */
    int oa_count;                     /* number of elements defined */
    long oa_indices[OBJ_MAXFUNC + 1]; /* function indices for actions */
    int oa_elements[1];               /* element indices (MUST BE LAST) */
} OBJECTACTIONS;

#  define objectactionsize(elements) (sizeof(OBJECTACTIONS) + ((elements) - 1) * sizeof(int))

/*
 * Structure of an object.
 * This is actually a varying sized structure.
 * However, there are always at least USUAL_ELEMENTS values in the object.
 */
struct object {
    OBJECTACTIONS *o_actions;      /* action table for this object */
    VALUE o_table[USUAL_ELEMENTS]; /* object values (MUST BE LAST) */
};

#  define objectsize(elements) (sizeof(OBJECT) + ((elements) - USUAL_ELEMENTS) * sizeof(VALUE))

extern OBJECT *objcopy(OBJECT *op);
extern OBJECT *objalloc(long index);
extern VALUE objcall(int action, VALUE *v1, VALUE *v2, VALUE *v3);
extern void objfree(OBJECT *op);
extern int addelement(char *name);
extern int defineobject(char *name, int indices[], int count);
extern int checkobject(char *name);
extern void showobjfuncs(void);
extern void showobjtypes(void);
extern int findelement(char *name);
extern char *objtypename(unsigned long index);
extern int objoffset(OBJECT *op, long index);

/*
 * Configuration parameter name and type.
 */
extern NAMETYPE configs[];
extern void config_value(CONFIG *cfg, int type, VALUE *ret);
extern void setconfig(int type, VALUE *vp);
extern void config_print(CONFIG *cfg); /* the CONFIG to print */

/*
 * size, memsize and sizeof support
 */
extern long elm_count(VALUE *vp);
extern size_t lsizeof(VALUE *vp);
extern size_t memsize(VALUE *vp);

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
extern STRING *stringtolower(STRING *);
extern STRING *stringtoupper(STRING *);
extern STRING *stringcpy(STRING *, STRING *);
extern STRING *stringncpy(STRING *, STRING *, size_t);
extern long stringcontent(STRING *s);
extern long stringlowbit(STRING *s);
extern long stringhighbit(STRING *s);
extern bool stringcmp(STRING *, STRING *);
extern FLAG stringrel(STRING *, STRING *);
extern FLAG stringcaserel(STRING *, STRING *);
extern int stringbit(STRING *, long);
extern bool stringtest(STRING *);
extern int stringsetbit(STRING *, long, bool);
extern int stringsearch(STRING *, STRING *, long, long, ZVALUE *);
extern int stringrsearch(STRING *, STRING *, long, long, ZVALUE *);

/*
 * s100 generator function declarations
 */
extern RAND *zsrand(const ZVALUE *seed, const MATRIX *pmat100);
extern RAND *zsetrand(const RAND *state);
extern void zrandskip(long count);
extern void zrand(long count, ZVALUE *res);
extern void zrandrange(const ZVALUE low, const ZVALUE beyond, ZVALUE *res);
extern long irand(long s);
extern RAND *randcopy(const RAND *rand);
extern void randfree(RAND *rand);
extern bool randcmp(const RAND *s1, const RAND *s2);
extern void randprint(const RAND *state, int flags);

#endif
