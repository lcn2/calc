/*
 * u_pfe - pipe/fork/exec
 *
 * Copyright (c) 2024 Viktor Bergquist
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
 */

//cspell:ignore qtoi itoq
//cspell:ignore ARGSUSED

/*
 * ISO C requires a translation unit to contain at least one declaration,
 * so we declare a global variable whose value is based on if CUSTOM is defined.
 */
#include <sys/wait.h>
#if defined(CUSTOM)
int u_pfe_allowed = 1; /* CUSTOM defined */
#else                  /* CUSTOM */
int u_pfe_allowed = 0; /* CUSTOM undefined */
#endif                 /* CUSTOM */


#if defined(CUSTOM)

    #include <sys/time.h>
    #include <sys/errno.h>
    #include <sys/param.h> // MAXPATHLEN attow
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <libgen.h>
    #include <poll.h>
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/resource.h>

    #include "../have_const.h"
    #include "../value.h"
    #include "../custom.h"

    #include "../config.h"
    #include "../calc.h"

    #include "../have_unused.h"


    #include "../banned.h" /* include after system header <> includes */

const char *
type2str(short type)
{
    // originally from c_argv.c
    switch (type) {
        case V_NULL:
            return "null";           // null value
        case V_INT:
            return "int";            // normal integer
        case V_NUM:
            return "rational_value"; // number
        case V_COM:
            return "complex_value";  // complex number
        case V_ADDR:
            return "address";        // address of variable value
        case V_STR:
            return "string";         // address of string
        case V_MAT:
            return "matrix";         // address of matrix structure
        case V_LIST:
            return "list";           // address of list structure
        case V_ASSOC:
            return "assoc";          // address of association structure
        case V_OBJ:
            return "object";         // address of object structure
        case V_FILE:
            return "file";           // opened file id
        case V_RAND:
            return "rand_state";     // subtractive 100 random state
        case V_RANDOM:
            return "random_state";   // address of Blum random state
        case V_CONFIG:
            return "config_state";   // configuration state
        case V_HASH:
            return "hash_state";     // hash state
        case V_BLOCK:
            return "octet_block";    // memory block
        case V_OCTET:
            return "octet";          // octet (unsigned char)
        default:
            return "unknown";
    }
}

/*
 * u_pfe_fork - create process
 *
 * returns:
 *	pid
 */
/*ARGSUSED*/
VALUE
u_pfe_fork(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result;
    int   pid;

    pid = fork();

    result.v_subtype = 0;
    result.v_type    = V_NUM;
    result.v_num     = itoq(pid);
    return result;
}

//cspell:ignore makenewstring
//cspell:ignore makestring
//makenewstring("0");

//cspell:ignore associndex
//cspell:ignore assocalloc

VALUE *
associndex_int(ASSOC *assoc, long index)
{
    VALUE i;
    VALUE indices[1];

    i.v_type = V_NUM;
    i.v_num  = itoq(index);

    indices[0] = i;
    return associndex(assoc, TRUE, 1, indices);
}

VALUE *
associndex_str(ASSOC *assoc, STRING *index)
{
    VALUE i;
    VALUE indices[1];

    i.v_type = V_STR;
    i.v_str  = index;

    indices[0] = i;
    return associndex(assoc, TRUE, 1, indices);
}

void
associndex_int_int(ASSOC *assoc, long index, long value)
{
    VALUE *i  = associndex_int(assoc, index);
    i->v_type = V_NUM;
    i->v_num  = itoq(value);
}

void
associndex_str_int(ASSOC *assoc, STRING *index, long value)
{
    VALUE *i  = associndex_str(assoc, index);
    i->v_type = V_NUM;
    i->v_num  = itoq(value);
}

/*
 * u_pfe_pipe - create descriptor pair for interprocess communication
 *
 * returns:
 *	assoc { [0] = reading, [1] = writing }
 */
/*ARGSUSED*/
VALUE
u_pfe_pipe(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    const char *custname = "pipe";

    int fds[2];
    if (pipe(fds))
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    ASSOC *assoc = assocalloc(2);

    associndex_int_int(assoc, 0, fds[0]);
    associndex_int_int(assoc, 1, fds[1]);

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_ASSOC;
    result.v_assoc   = assoc;
    return result;
}

//cspell:ignore custname
typedef enum {
    VALV_OPT_OOB,
    VALV_OPT_NULL,
    VALV_OPT_GOOD,
    VALV_OPT_BAD,
    VALV_OPT_REF_NULL,
} valv_opt;

valv_opt
valv_optional_type_check(int count, VALUE **vals, int idx,
    const char *UNUSED(name), short wanted)
{
    if (idx > count) return VALV_OPT_OOB;
    if (vals[idx]->v_type == V_NULL) return VALV_OPT_NULL;
    if (vals[idx]->v_type == wanted) return VALV_OPT_GOOD;
    return VALV_OPT_BAD;
}
valv_opt
valv_optional_ref_type_check(int count, VALUE **vals, int idx,
    const char *UNUSED(name), short wanted)
{
    if (idx > count) return VALV_OPT_OOB;
    if (vals[idx]->v_type == V_NULL) return VALV_OPT_NULL;
    if (vals[idx]->v_type != V_VPTR) return VALV_OPT_BAD;
    if (vals[idx]->v_addr->v_type == V_NULL) return VALV_OPT_REF_NULL;
    if (vals[idx]->v_addr->v_type == wanted) return VALV_OPT_GOOD;
    return VALV_OPT_BAD;
}
bool
valv_type_check(int UNUSED(count), VALUE **vals, int idx,
    const char *UNUSED(name), short wanted)
{
    return vals[idx]->v_type == wanted;
}
void
valv_type_require(const char *custname, int count, VALUE **vals, int idx,
    const char *name, short wanted)
{
    if (!valv_type_check(count, vals, idx, name, wanted))
        math_error("%s: argment %d (%s) must be of type %s (%s given)", custname,
            idx + 1, name, type2str(wanted), type2str(vals[idx]->v_type));
}
bool
valv_ref_type_check(int UNUSED(count), VALUE **vals, int idx,
    const char *UNUSED(name), short wanted)
{
    return vals[idx]->v_addr->v_type == wanted;
}
void
valv_ref_type_require(const char *custname, int count, VALUE **vals, int idx,
    const char *name, short wanted)
{
    if (!valv_type_check(count, vals, idx, name, V_VPTR))
        math_error("%s: argment %d (%s) must be address (%s given) of type %s",
            custname, idx + 1, name, type2str(vals[idx]->v_type),
            type2str(wanted));
    if (!valv_ref_type_check(count, vals, idx, name, wanted))
        math_error("%s: argment %d (%s) must be address of type %s (%s given)",
            custname, idx + 1, name, type2str(wanted),
            type2str(vals[idx]->v_addr->v_type));
}

long
valv_get_num_long(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_NUM);
    return qtoi(vals[idx]->v_num);
}
VALUE *
valv_optional_get_num(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk = valv_optional_type_check(count, vals, *idx, name, V_NUM);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;       // fall-thru
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:; // fall-thru
    }
    VALUE *num  = vals[*idx];
    *idx       += 1;
    return num;
}
VALUE *
valv_optional_ref_num(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk = valv_optional_ref_type_check(count, vals, *idx, name, V_NUM);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;       // fall-thru
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:; // fall-thru
    }
    VALUE *num = vals[*idx]->v_addr;
    if (chk == VALV_OPT_REF_NULL) {
        num->v_type = V_NUM;
        num->v_num  = dflt->v_num;
    }
    *idx += 1;
    return num;
}
VALUE *
valv_ref_num(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_ref_type_require(custname, count, vals, idx, name, V_NUM);
    return vals[idx]->v_addr;
}
VALUE *
valv_get_num(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_NUM);
    return vals[idx];
}
char *
valv_get_str(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_STR);
    return vals[idx]->v_str->s_str;
}
//cspell:ignore strp
VALUE *
valv_get_strp(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_STR);
    return vals[idx];
}
VALUE *
valv_optional_ref_list(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk = valv_optional_ref_type_check(count, vals, *idx, name, V_LIST);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;       // fall-thru
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:; // fall-thru
    }
    VALUE *list = vals[*idx]->v_addr;
    if (chk == VALV_OPT_REF_NULL) {
        list->v_type = V_LIST;
        list->v_list = dflt->v_list;
    }
    *idx += 1;
    return list;
}
VALUE *
valv_optional_get_list(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk = valv_optional_type_check(count, vals, *idx, name, V_LIST);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;       // fall-thru
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:; // fall-thru
    }
    VALUE *list  = vals[*idx];
    *idx        += 1;
    return list;
}
VALUE *
valv_get_list(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_LIST);
    return vals[idx];
}
VALUE *
valv_ref_list(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_ref_type_require(custname, count, vals, idx, name, V_LIST);
    return vals[idx]->v_addr;
}
VALUE *
valv_optional_get_assoc(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk = valv_optional_type_check(count, vals, *idx, name, V_ASSOC);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;       // fall-thru
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:; // fall-thru
    }
    VALUE *assoc  = vals[*idx];
    *idx         += 1;
    return assoc;
}
VALUE *
valv_optional_ref_assoc(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk = valv_optional_ref_type_check(count, vals, *idx, name, V_ASSOC);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;       // fall-thru
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:; // fall-thru
    }
    VALUE *assoc  = vals[*idx]->v_addr;
    *idx         += 1;
    return assoc;
}
VALUE *
valv_get_assoc(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_ASSOC);
    return vals[idx];
}
VALUE *
valv_ref_assoc(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_ref_type_require(custname, count, vals, idx, name, V_ASSOC);
    return vals[idx]->v_addr;
}

/*
 * u_pfe_execvp - execute a file, possibly looking in paths from environment or system
 *
 * given:
 *	vals[0] file
 *	vals[1] list(arg0,...)
 *
 * returns:
 *	0 on success
 */
/*ARGSUSED*/
VALUE
u_pfe_execvp(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "execvp";

    char  *path      = valv_get_str(custname, count, vals, 0, "file");
    VALUE *args_list = valv_get_list(custname, count, vals, 1, "args");

    char **args;
    if (!(args = malloc(sizeof(char *) * (args_list->v_list->l_count + 1))))
        math_error("%s: "__FILE__ ": %d: malloc: %s", custname, __LINE__,
            strerror(errno));

    LISTELEM *el = args_list->v_list->l_first;
    int       s  = 0;
    for (; el != NULL; el = el->e_next, s++) {
        if (el->e_value.v_type != V_STR)
            math_error("%s: argment 2 (args) element %d must be of type string (%s given)",
                custname, s, type2str(el->e_value.v_type));

        args[s] = el->e_value.v_str->s_str;
    }
    args[s] = NULL;

    if (execvp(path, args))
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    free(args);

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(errno);
    return result;
}

/*
 * u_pfe_dup - duplicate a file descriptor
 *
 * given:
 *	vals[0] source fd
 *
 * returns:
 *	target fd
 */
/*ARGSUSED*/
VALUE
u_pfe_dup(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "dup";

    int fd = dup(valv_get_num_long(custname, count, vals, 0, "source"));

    if (fd < 0)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(fd);
    return result;
}

/*
 * u_pfe_dup2 - duplicate a file descriptor
 *
 * given:
 *	vals[0] source fd
 *	vals[1] target fd
 *
 * returns:
 *	target fd
 */
/*ARGSUSED*/
VALUE
u_pfe_dup2(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "dup2";

    int fd = dup2(valv_get_num_long(custname, count, vals, 0, "source"),
        valv_get_num_long(custname, count, vals, 1, "target"));

    if (fd < 0)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(fd);
    return result;
}

/*
 * u_pfe_close - delete a file descriptor
 *
 * given:
 *	vals[0] fd
 *
 * returns:
 *	0 on success
 */
/*ARGSUSED*/
VALUE
u_pfe_close(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "close";

    int e = close(valv_get_num_long(custname, count, vals, 0, "fd"));

    if (e)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(e);
    return result;
}

/*
 * u_pfe_write - write output
 *
 * given:
 *	vals[0] fd
 *	vals[1] string
 *
 * returns:
 *	count of bytes written
 */
/*ARGSUSED*/
VALUE
u_pfe_write(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "write";

    VALUE *strp = valv_get_strp(custname, count, vals, 1, "string");

    ssize_t written = write(valv_get_num_long(custname, count, vals, 0, "fd"),
        strp->v_str->s_str, strp->v_str->s_len);

    if (written < 0)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(written);
    return result;
}

    #define u_pfe_read_STRL_DFLT 1024

/*
 * u_pfe_read - write output
 *
 * given:
 *	vals[0] fd
 *	vals[1] count defaulting to 1024
 *
 * returns:
 *	count of bytes read
 */
/*ARGSUSED*/
VALUE
u_pfe_read(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "read";

    int strl = count < 2 ? u_pfe_read_STRL_DFLT
                         : valv_get_num_long(custname, count, vals, 1, "count");

    char *str = malloc((strl + 1) * sizeof(char));

    ssize_t r = read(valv_get_num_long(custname, count, vals, 0, "fd"), str,
        strl);
    str[r]    = '\0';

    if (r < 0) {
        free(str);
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));
    }

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = !r ? V_NULL : V_STR;
    if (r) result.v_str = makestring(str);
    return result;
}

VALUE *
optional_valv_ref_list2fd_set(const char *custname, int count, VALUE **vals,
    int *idx, const char *name, fd_set *fds, int *setsize)
{
    valv_opt chk = valv_optional_ref_type_check(count, vals, *idx, name, V_LIST);
    switch (chk) {
        case VALV_OPT_BAD:
            return NULL;
        case VALV_OPT_NULL:
            *idx += 1; // fall-thru
        case VALV_OPT_OOB:
            return NULL;
        case VALV_OPT_REF_NULL:
            math_error("%s: argument %d (%s) address of type null is not allowed",
                custname, *idx, name);
        case VALV_OPT_GOOD:; // fall-thru
    }
    VALUE *list   = vals[*idx]->v_addr;
    *idx         += 1;
    LISTELEM *el  = list->v_list->l_first;
    int       s   = 0;
    for (; el != NULL; el = el->e_next, s++) {
        if (el->e_value.v_type != V_NUM)
            math_error("%s: argument %d (%s) element %d must be of type number (%s given)",
                custname, *idx, name, s, type2str(el->e_value.v_type));

        int i = qtoi(el->e_value.v_num);
        if (*setsize < i + 1) *setsize = i + 1;
        FD_SET(i, fds);
    }
    return list;
}

VALUE *
alloc_list(LIST *list)
{
    VALUE *result  = malloc(sizeof(VALUE));
    result->v_type = V_LIST;

    result->v_list = list;
    return result;
}

VALUE *
alloc_assoc(ASSOC *assoc)
{
    VALUE *result  = malloc(sizeof(VALUE));
    result->v_type = V_ASSOC;

    result->v_assoc = assoc;
    return result;
}

//cspell:ignore listalloc copyvalue insertlistlast

VALUE *
list_fd_get(LIST *in, fd_set *fds)
{
    VALUE    *out = alloc_list(listalloc());
    LISTELEM *el  = in->l_first;
    int       s   = 0;
    for (; el != NULL; el = el->e_next, s++) {
        int i = qtoi(el->e_value.v_num);
        if (FD_ISSET(i, fds)) {
            VALUE o;
            copyvalue(&el->e_value, &o);
            insertlistlast(out->v_list, &o);
        }
    }
    return out;
}

VALUE *
alloc_num(NUMBER *num)
{
    VALUE *result  = malloc(sizeof(VALUE));
    result->v_type = V_NUM;

    result->v_num = num;
    return result;
}

VALUE *
alloc_str(STRING *str)
{
    VALUE *result  = malloc(sizeof(VALUE));
    result->v_type = V_STR;

    result->v_str = str;
    return result;
}

VALUE *pfe_select_TIMEOUT_DFLT = NULL;

//cspell:ignore qisneg qisint ztoi qscale qfree

/*
 * u_pfe_select - examine file descriptors
 *
 * given:
 *	vals[l1+0] null or address of list (inout\ref) of fds for reading,		wh/ l1 is zero-index of 1st list
 *	vals[l1+1] null or address of list (inout\ref) of fds for writing,		wh/ l1 is zero-index of 1st list
 *	vals[l1+2] null or address of list (inout\ref) of fds for exceptional,	wh/ l1 is zero-index of 1st list
 *	vals[lZ+1] null or timeout defaulting to -1,							wh/ lZ is zero-index of last list
 *
 * returns:
 *	count of fds w/ status
 */
/*ARGSUSED*/
VALUE
u_pfe_select(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "select";
    int         v        = 0;

    int    ssz = 0;
    fd_set rds;
    FD_ZERO(&rds);
    VALUE *rdl = optional_valv_ref_list2fd_set(custname, count, vals, &v,
        "reading", &rds, &ssz);
    fd_set wts;
    FD_ZERO(&wts);
    VALUE *wtl = optional_valv_ref_list2fd_set(custname, count, vals, &v,
        "writing", &wts, &ssz);
    fd_set ers;
    FD_ZERO(&ers);
    VALUE *erl = optional_valv_ref_list2fd_set(custname, count, vals, &v,
        "exceptional", &ers, &ssz);

    if (!pfe_select_TIMEOUT_DFLT) pfe_select_TIMEOUT_DFLT = alloc_num(itoq(-1));

    VALUE *tv = valv_optional_get_num(count, vals, &v, "timeout",
        pfe_select_TIMEOUT_DFLT);

    NUMBER         *tn = tv->v_num;
    struct timeval  t;
    struct timeval *tp;
    // originally from f_sleep() in ../func.c
    if (qisneg(tn)) {
        tp = NULL;
    } else if (qisint(tn)) {
        if (zge31b(tn->num)) math_error("sizeof(timeout) > 31 bytes");
        t.tv_sec  = ztoi(tn->num);
        t.tv_usec = 0;
        tp        = &t;
    } else {
        NUMBER *q1, *q2;
        q1 = qscale(tn, 20); // 2^20 = 1 Mi
        q2 = qint(q1);
        qfree(q1);
        if (zge31b(q2->num)) {
            qfree(q2);
            math_error("sizeof(timeout as microseconds) > 31 bytes");
        }
        long usec = ztoi(q2->num);
        t.tv_sec  = usec / 1000000;
        t.tv_usec = usec - t.tv_sec * 1000000;
        tp        = &t;
        qfree(q2);
    }

    int r = select(ssz, &rds, &wts, &ers, tp);

    if (r < 0)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    if (rdl) *rdl = *list_fd_get(rdl->v_list, &rds);
    if (wtl) *wtl = *list_fd_get(wtl->v_list, &wts);
    if (erl) *erl = *list_fd_get(erl->v_list, &ers);

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(r);
    return result;
}

VALUE *u_pfe_poll_TIMEOUT_DFLT = NULL;

/*
 * u_pfe_poll - synchronous I/O multiplexing
 *
 * tested manually only slightly (as it turned out it wasn't suitable for why it was written)
 *
 * TODO unit/feature test
 *
 * given:
 *	vals[0] list of list(fd[, event1[, ...eventN]])
 *	vals[1] address of list, appended with list(fd[, event1[, ...eventN]])
 *	vals[2] null or integer millisecond timeout defaulting to -1
 *
 * returns:
 *	count of fds w/ status
 */
/*ARGSUSED*/
VALUE
u_pfe_poll(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "poll";
    const char *i_name   = "list of list(fd[, event1[, ...eventN]])";

    VALUE *iv = valv_get_list(custname, count, vals, 0, i_name);
    VALUE *ov = valv_ref_list(custname, count, vals, 1, "list");

    if (!u_pfe_poll_TIMEOUT_DFLT) u_pfe_poll_TIMEOUT_DFLT = alloc_num(itoq(-1));
    int     v  = 2;
    VALUE  *tv = valv_optional_get_num(count, vals, &v, "timeout",
         u_pfe_poll_TIMEOUT_DFLT);
    NUMBER *tn = tv->v_num;
    if (!qisint(tn))
        math_error("%s: argument 2 (timeout) must be integer", custname);
    int t = qtoi(tn);

    LIST          *il      = iv->v_list;
    struct pollfd *pollfds = malloc(il->l_count
                                    * sizeof(struct pollfd)); //cspell:ignore pollfds

    LISTELEM *el = il->l_first;
    int       s  = 0;
    for (; el != NULL; el = el->e_next, s++) {
        if (el->e_value.v_type != V_LIST) {
            free(pollfds);
            math_error("%s: argument %d (%s) element %d must be of type list (%s given)",
                custname, 1, i_name, s, type2str(el->e_value.v_type));
        }

        LIST     *el_list = el->e_value.v_list;
        LISTELEM *el_el;

        el_el = el_list->l_first;

        if (el_el->e_value.v_type != V_NUM) {
            free(pollfds);
            math_error("%s: argument %d (%s) element %d element %d must be of type number (%s given)",
                custname, 1, i_name, s, 1, type2str(el_el->e_value.v_type));
        }
        if (!qisint(el_el->e_value.v_num)) {
            free(pollfds);
            math_error("%s: argument %d (%s) element %d element %d must be integer (%s given)",
                custname, 1, i_name, s, 1, type2str(el_el->e_value.v_type));
        }
        pollfds[s].fd = qtoi(el_el->e_value.v_num);

        pollfds[s].events = 0;

        el_el = el_el->e_next;

        int el_el_s = 1;
        for (; el_el != NULL; el_el = el_el->e_next, el_el_s++) {
            if (el_el->e_value.v_type != V_STR) {
                free(pollfds);
                math_error("%s: argument %d (%s) element %d element %d must be of type string (%s given)",
                    custname, 1, i_name, s, el_el_s,
                    type2str(el_el->e_value.v_type));
            }

            char *el_el_str = el_el->e_value.v_str->s_str;
            if (FALSE)
                ; // man poll|gawk 'match($0,"^\\s*POLL([A-Z]+)",m){print(tolower(m[1]))}'
            else if (!strcmp(el_el_str, "err")) pollfds[s].events |= POLLERR;
            else if (!strcmp(el_el_str, "hup")) pollfds[s].events |= POLLHUP;
            else if (!strcmp(el_el_str, "in")) pollfds[s].events |= POLLIN;
            else if (!strcmp(el_el_str, "nval"))
                pollfds[s].events |= POLLNVAL;   //cspell:ignore nval
            else if (!strcmp(el_el_str, "out")) pollfds[s].events |= POLLOUT;
            else if (!strcmp(el_el_str, "pri")) pollfds[s].events |= POLLPRI;
            else if (!strcmp(el_el_str, "rdband"))
                pollfds[s].events |= POLLRDBAND; //cspell:ignore rdband
            else if (!strcmp(el_el_str, "rdnorm"))
                pollfds[s].events |= POLLRDNORM; //cspell:ignore rdnorm
            else if (!strcmp(el_el_str, "wrband"))
                pollfds[s].events |= POLLWRBAND; //cspell:ignore wrband
            else if (!strcmp(el_el_str, "wrnorm"))
                pollfds[s].events |= POLLWRNORM; //cspell:ignore wrnorm
            else {
                free(pollfds);
                math_error("%s: argument %d (%s) element %d element %d must be a string containing one of the event names from POSIX poll.h or the manual for poll, lowercase without poll prefix (%s given)",
                    custname, 1, i_name, s, el_el_s, el_el->e_value.v_str->s_str);
            }
        }

        pollfds[s].revents = 0;
    }

    int r = poll(pollfds, il->l_count, t);

    if (r < 0) {
        free(pollfds);
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));
    }

    LIST *ol = listalloc();

    for (s = 0; s < il->l_count; s++) {
        VALUE *o = alloc_assoc(assocalloc(0));

        if ((pollfds[s].revents & POLLERR) == POLLERR)
            associndex_str_int(o->v_assoc, makenewstring("err"), POLLERR);
        if ((pollfds[s].revents & POLLHUP) == POLLHUP)
            associndex_str_int(o->v_assoc, makenewstring("hup"), POLLHUP);
        if ((pollfds[s].revents & POLLIN) == POLLIN)
            associndex_str_int(o->v_assoc, makenewstring("in"), POLLIN);
        if ((pollfds[s].revents & POLLNVAL) == POLLNVAL)
            associndex_str_int(o->v_assoc, makenewstring("nval"), POLLNVAL);
        if ((pollfds[s].revents & POLLOUT) == POLLOUT)
            associndex_str_int(o->v_assoc, makenewstring("out"), POLLOUT);
        if ((pollfds[s].revents & POLLPRI) == POLLPRI)
            associndex_str_int(o->v_assoc, makenewstring("pri"), POLLPRI);
        if ((pollfds[s].revents & POLLRDBAND) == POLLRDBAND)
            associndex_str_int(o->v_assoc, makenewstring("rdband"), POLLRDBAND);
        if ((pollfds[s].revents & POLLRDNORM) == POLLRDNORM)
            associndex_str_int(o->v_assoc, makenewstring("rdnorm"), POLLRDNORM);
        if ((pollfds[s].revents & POLLWRBAND) == POLLWRBAND)
            associndex_str_int(o->v_assoc, makenewstring("wrband"), POLLWRBAND);
        if ((pollfds[s].revents & POLLWRNORM) == POLLWRNORM)
            associndex_str_int(o->v_assoc, makenewstring("wrnorm"), POLLWRNORM);

        insertlistlast(ol, o);
    }

    ov->v_list = ol;

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(r);
    return result;
}

/*
 * u_pfe_wait4 - wait for process termination
 *
 * wh/ n1 is zero-index of 1st number,
 * given:
 *	vals[n1+0] pid wh/ null() means any children à la man 2 wait # aka wait(2) and only vals[n1+1] allowed
 *	vals[n1+1] address of assoc of status w/ one pair of key with companion(s): "exited" &then "exitstatus", "signaled" &then "termsig" & "coredump", "stopped" &then "stopsig" acc2 man 2 wait # aka wait(2)
 *	vals[n1+2] null or list of options "nohang" or "untraced" acc2 man TODO test
 *	vals[n1+3] null or address of assoc of usage TODO handle
 *
 * returns:
 *	pid or 0 w/ "nohang"
 */
/*ARGSUSED*/
VALUE
u_pfe_wait4(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname  = "wait4";
    int         v         = 0;
    VALUE      *pid_num   = valv_optional_get_num(count, vals, &v, "pid", NULL);
    VALUE      *stt_assoc = valv_ref_assoc(custname, count, vals, v++,
             "status"); // stat in
    VALUE *opt_list  = valv_optional_get_list(count, vals, &v, "options", NULL);
    VALUE *usg_assoc = valv_optional_ref_assoc(count, vals, &v, "usage", NULL);

    if (!pid_num && opt_list)
        math_error("%s: argument 1 (pid) being null doesn't support options list",
            custname);
    if (!pid_num && usg_assoc)
        math_error("%s: argument 1 (pid) being null doesn't support usage assoc",
            custname);

    int opts = 0;
    if (opt_list) {
        LISTELEM *el = opt_list->v_list->l_first;
        int       s  = 0;
        for (; el != NULL; el = el->e_next, s++) {
            if (el->e_value.v_type != V_STR)
                math_error("%s: options list element %d must be of type string (%s given)",
                    custname, s, type2str(el->e_value.v_type));

            char *opt_str = el->e_value.v_str->s_str;
            if (FALSE)
                ;
            else if (!strcmp(opt_str, "nohang")) opts |= WNOHANG;
            else if (!strcmp(opt_str, "untraced")) opts |= WUNTRACED;
            else
                math_error("%s: options list element %d must be one of \"nohang\" or \"untraced\" (%s given)",
                    custname, s, opt_str);
        }
    }

    int            stt = 0;
    struct rusage  usg;
    struct rusage *usg_p = usg_assoc ? &usg : NULL;
    pid_t          r = pid_num ? wait4(qtoi(pid_num->v_num), &stt, opts, usg_p)
                               : wait(&stt);

    if (0 > r)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    if (WIFEXITED(stt)) {
        associndex_str_int(stt_assoc->v_assoc, makenewstring("exited"), TRUE);
        associndex_str_int(stt_assoc->v_assoc, makenewstring("exitstatus"),
            WEXITSTATUS(stt));
    }
    if (WIFSIGNALED(stt)) {
        associndex_str_int(stt_assoc->v_assoc, makenewstring("signaled"), TRUE);
        associndex_str_int(stt_assoc->v_assoc, makenewstring("termsig"),
            WTERMSIG(stt));
        associndex_str_int(stt_assoc->v_assoc, makenewstring("coredump"),
            WCOREDUMP(stt));
    }
    if (WIFSTOPPED(stt)) {
        associndex_str_int(stt_assoc->v_assoc, makenewstring("stopped"), TRUE);
        associndex_str_int(stt_assoc->v_assoc, makenewstring("stopsig"),
            WSTOPSIG(stt));
    }

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(r);
    return result;
}

/*
 * u_pfe_pfe - pipe/fork/exec
 *
 * given:
 *	vals[0] address of fd for in
 *	vals[1] address of fd for out
 *	vals[2] address of fd for err
 *	vals[3] list of args
 *
 * returns:
 *	forked pid
 */
/*ARGSUSED*/
VALUE
u_pfe_pfe(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "pfe";

    int ia[2]; // in array
    if (pipe(ia))
        math_error("%s: "__FILE__ ": %d: (in) pipe: %s", custname, __LINE__,
            strerror(errno));
    int oa[2]; // out array
    if (pipe(oa))
        math_error("%s: "__FILE__ ": %d: (out) pipe: %s", custname, __LINE__,
            strerror(errno));
    int ea[2]; // err array
    if (pipe(ea))
        math_error("%s: "__FILE__ ": %d: (err) pipe: %s", custname, __LINE__,
            strerror(errno));
    // in, out, err values
    VALUE *iv = valv_ref_num(custname, count, vals, 0, "in");
    VALUE *ov = valv_ref_num(custname, count, vals, 1, "out");
    VALUE *ev = valv_ref_num(custname, count, vals, 2, "err");
    iv->v_num = itoq(ia[1]);
    ov->v_num = itoq(oa[0]);
    ev->v_num = itoq(ea[0]);

    pid_t child = fork();
    if (child) {
        // child: close (parent in, out, err)
        int cci = !close(ia[0]) ? 0 : errno;
        int cco = !close(oa[1]) ? 0 : errno;
        int cce = !close(ea[1]) ? 0 : errno;

        if (cci)
            math_error("%s: "__FILE__ ": %d: child: (parent in) close: %s",
                custname, __LINE__, strerror(cci));
        if (cco)
            math_error("%s: "__FILE__ ": %d: child: (parent out) close: %s",
                custname, __LINE__, strerror(cco));
        if (cce)
            math_error("%s: "__FILE__ ": %d: child: (parent err) close: %s",
                custname, __LINE__, strerror(cce));

        VALUE result;
        result.v_subtype = 0;
        result.v_type    = V_NUM;

        result.v_num = itoq(child);
        return result;

    } else {
        // parent: close (child in, out, err)
        int pci = !close(ia[1]) ? 0 : errno;
        int pco = !close(oa[0]) ? 0 : errno;
        int pce = !close(ea[0]) ? 0 : errno;

        if (pci)
            math_error("%s: "__FILE__ ": %d: parent: (child in) close: %s",
                custname, __LINE__, strerror(pci));
        if (pco)
            math_error("%s: "__FILE__ ": %d: parent: (child out) close: %s",
                custname, __LINE__, strerror(pco));
        if (pce)
            math_error("%s: "__FILE__ ": %d: parent: (child err) close: %s",
                custname, __LINE__, strerror(pce));

        if (dup2(ia[0], 0) != 0)
            math_error("%s: "__FILE__ ": %d: mismatch: (in) dup2: %s", custname,
                __LINE__, strerror(errno));
        if (dup2(oa[1], 1) != 1)
            math_error("%s: "__FILE__ ": %d: mismatch: (out) dup2: %s",
                custname, __LINE__, strerror(errno));
        if (dup2(ea[1], 2) != 2)
            math_error("%s: "__FILE__ ": %d: mismatch: (err) dup2: %s",
                custname, __LINE__, strerror(errno));

        VALUE *args_list = valv_get_list(custname, count, vals, 3, "args");

        char **args;
        if (!(args = malloc(sizeof(char *) * (args_list->v_list->l_count + 1))))
            math_error("%s: "__FILE__ ": %d: malloc: %s", custname, __LINE__,
                strerror(errno));

        LISTELEM *el = args_list->v_list->l_first;
        int       s  = 0;
        for (; el != NULL; el = el->e_next, s++) {
            if (el->e_value.v_type != V_STR)
                math_error("%s: argment 2 (args) element %d must be of type string (%s given)",
                    custname, s, type2str(el->e_value.v_type));

            args[s] = el->e_value.v_str->s_str;
        }
        args[s] = NULL;

        if (execvp(args[0], args))
            math_error("%s: "__FILE__ ": %d: execvp: %s", custname, __LINE__,
                strerror(errno));

        free(args);

        VALUE result;
        result.v_subtype = 0;
        result.v_type    = V_NUM;

        result.v_num = itoq(errno);
        return result;
    }
}

/*
 * u_pfe_pwrite - write and close
 *
 * given:
 *	vals[0] fd
 *	vals[1] string
 *
 * returns:
 *	count of bytes written
 */
/*ARGSUSED*/
VALUE
u_pfe_pwrite(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "pwrite";

    VALUE *strp = valv_get_strp(custname, count, vals, 1, "string");

    int fd = valv_get_num_long(custname, count, vals, 0, "fd");

    ssize_t w = write(fd, strp->v_str->s_str, strp->v_str->s_len);

    if (w < 0)
        math_error("%s: "__FILE__ ": %d: write: %s", custname, __LINE__,
            strerror(errno));

    int e = close(fd);

    if (e)
        math_error("%s: "__FILE__ ": %d: close: %s", custname, __LINE__,
            strerror(errno));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(w);
    return result;
}

    #define pfe_pfe_SIZE_BUFFER 4096
    #define pfe_pfe_SIZE_INIT   4096

char *
strext(char **subject, char *with)
{
    size_t n = strlen(*subject) + strlen(with);
    if ((*subject = realloc(*subject, n + 1)) == NULL) {
        free(*subject);
        return NULL;
    }
    strlcat(*subject, with, n + 1);
    return *subject;
}

/*
 * u_pfe_pread - read until eof, close and wait for exit status
 *
 * if wait4() encounters a signalled process, this function returns the signal + 128, like bash(1).
 *
 * given:
 *	vals[0] pid
 *	vals[1] out
 *	vals[1] err
 *
 * returns:
 *	list(status, stdout_string, stderr_string)
 */
/*ARGSUSED*/
VALUE
u_pfe_pread(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "pread";

    int pid = valv_get_num_long(custname, count, vals, 0, "pid");
    int out = valv_get_num_long(custname, count, vals, 1, "out");
    int err = valv_get_num_long(custname, count, vals, 2, "err");

    fd_set nvm; // never mind
    FD_ZERO(&nvm);
    fd_set rbs; // read base set
    FD_ZERO(&rbs);
    int ssz = 0;
    FD_SET(out, &rbs);
    if (out > ssz) ssz = out;
    FD_SET(err, &rbs);
    if (err > ssz) ssz = err;

    // Keep track of rbs
    bool eoo = NULL; // end of out
    bool eoe = NULL; // end of err

    char *o = malloc(pfe_pfe_SIZE_INIT * sizeof(char));
    o[0]    = '\0';
    char *e = malloc(pfe_pfe_SIZE_INIT * sizeof(char));
    e[0]    = '\0';

    char ob[pfe_pfe_SIZE_BUFFER]; // out buffer
    char eb[pfe_pfe_SIZE_BUFFER]; // err buffer

    int pco = 0;
    int pce = 0;

    fd_set rcs; // read check set
    while (!eoo || !eoe) {
        memmove(&rcs, &rbs, sizeof(fd_set));

        int r = select(1 + ssz, &rcs, &nvm, &nvm, NULL);
        if (r < 0)
            math_error("%s: "__FILE__ ": %d: select: %s", custname, __LINE__,
                strerror(errno));

        if (FD_ISSET(out, &rcs)) {
            r = read(out, &ob, pfe_pfe_SIZE_BUFFER);
            if (r < 0)
                math_error("%s: "__FILE__ ": %d: (out, %lu) read: %s", custname,
                    __LINE__, strlen(o), strerror(errno));
            if (r) {
                ob[r] = '\0';
                strext(&o, ob);
            } else {
                if (close(out)) pco = errno;
                FD_CLR(out, &rbs);
                eoo = TRUE;
            }
        }
        if (FD_ISSET(err, &rcs)) {
            r = read(err, &eb, pfe_pfe_SIZE_BUFFER);
            if (r < 0)
                math_error("%s: "__FILE__ ": %d: (err, %lu) read: %s", custname,
                    __LINE__, strlen(e), strerror(errno));
            if (r) {
                eb[r] = '\0';
                strext(&e, eb);
            } else {
                if (close(err)) pce = errno;
                FD_CLR(err, &rbs);
                eoe = TRUE;
            }
        }
    }

    if (pco)
        math_error("%s: "__FILE__ ": %d: (out) close: %s", custname, __LINE__,
            strerror(pco));
    if (pce)
        math_error("%s: "__FILE__ ": %d: (err) close: %s", custname, __LINE__,
            strerror(pce));

    int   stt = 0;
    pid_t w   = wait4(pid, &stt, 0, NULL);

    if (0 > w)
        math_error("%s: "__FILE__ ": %d: wait4: %s", custname, __LINE__,
            strerror(errno));

    NUMBER *r = itoq((!WIFEXITED(stt) ? 0 : WEXITSTATUS(stt))
                     + (!WIFSTOPPED(stt) ? 0 : 128 + WSTOPSIG(stt))
                     + (!WIFSIGNALED(stt) ? 0 : 128 + WTERMSIG(stt)));

    LIST *list = listalloc();
    insertlistlast(list, alloc_num(r));
    insertlistlast(list, alloc_str(makestring(o)));
    insertlistlast(list, alloc_str(makestring(e)));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_LIST;

    result.v_list = list;
    return result;
}

/*
 * u_vadd_getpid - get calling process identification
 *
 * returns:
 *	pid
 */
/*ARGSUSED*/
VALUE
u_vadd_getpid(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    const char *UNUSED(custname) = "getpid";

    pid_t pid = getpid();

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(pid);
    return result;
}

/*
 * u_vadd_getppid - get parent process identification
 *
 * returns:
 *	pid
 */
/*ARGSUSED*/
VALUE
u_vadd_getppid(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    const char *UNUSED(custname) = "getppid";

    pid_t pid = getppid();

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_NUM;

    result.v_num = itoq(pid);
    return result;
}

/*
 * u_vadd_getcwd - get working directory pathname
 *
 * returns:
 *	path
 */
/*ARGSUSED*/
VALUE
u_vadd_getcwd(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    const char *custname = "getcwd";

    char *buf = getcwd(NULL,
        -1); // doing equiv(malloc) COULD use MAXPATHLEN from <sys/param.h> and s/makestring/makenewstring/ below I guess
    if (!buf)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_STR;

    result.v_str = makestring(buf);
    return result;
}

/*
 * u_vadd_inputname - get working directory pathname
 *
 * returns:
 *	name
 */
/*ARGSUSED*/
VALUE
u_vadd_inputname(char *UNUSED(name), int UNUSED(count),
    VALUE **UNUSED(vals)) //cspell:ignore inputname
{
    const char *UNUSED(custname) = "inputname";

    char *buf = inputname();
    if (!buf)
        buf = inputisterminal() ? "<terminal>"
                                : "<anonymous>"; //cspell:ignore inputisterminal

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_STR;

    result.v_str = makenewstring(buf);
    return result;
}

/*
 * u_vadd_basename - extract the base portion of a pathname
 *
 * given:
 *	vals[0] path
 *
 * returns:
 *	path
 */
/*ARGSUSED*/
VALUE
u_vadd_basename(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "basename";

    VALUE *strp = valv_get_strp(custname, count, vals, 0, "path");

    char buf[MAXPATHLEN];
    strlcpy(buf, basename(strp->v_str->s_str), sizeof(buf));
    if (!*buf)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_STR;

    result.v_str = makenewstring(buf);
    return result;
}

/*
 * u_vadd_dirname - extract the base portion of a pathname
 *
 * given:
 *	vals[0] path
 *
 * returns:
 *	path
 */
/*ARGSUSED*/
VALUE
u_vadd_dirname(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "dirname";

    VALUE *strp = valv_get_strp(custname, count, vals, 0, "path");

    char buf[MAXPATHLEN];
    strlcpy(buf, dirname(strp->v_str->s_str), sizeof(buf));
    if (!*buf)
        math_error("%s: "__FILE__ ": %d: %s", custname, __LINE__,
            strerror(errno));

    VALUE result;
    result.v_subtype = 0;
    result.v_type    = V_STR;

    result.v_str = makenewstring(buf);
    return result;
}

#endif /* CUSTOM */
