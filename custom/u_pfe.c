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
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/* cspell:ignore qtoi itoq */
/* cspell:ignore ARGSUSED */
/* cspell:ignore custname */
/* cspell:ignore makenewstring */
/* cspell:ignore makestring */
/* makenewstring("0"); */
/* cspell:ignore associndex */
/* cspell:ignore assocalloc */
/* cspell:ignore strp */
/* cspell:ignore listalloc copyvalue insertlistlast */
/* cspell:ignore qisneg qisint ztoi qscale qfree */
/* cspell:ignore pollfds */
/* cspell:ignore nval */
/* cspell:ignore rdband */
/* cspell:ignore wrband */
/* cspell:ignore wrband */
/* cspell:ignore wrnorm */
/* cspell:ignore inputname */
/* cspell:ignore inputisterminal */

/*
 * ISO C requires a translation unit to contain at least one declaration,
 * so we declare a global variable whose value is based on if CUSTOM is defined.
 */

#if defined(CUSTOM)
int u_pfe_allowed = 1; /* CUSTOM defined */
#else                  /* CUSTOM */
int u_pfe_allowed = 0; /* CUSTOM undefined */
#endif                 /* CUSTOM */

#if defined(CUSTOM)

#  include <sys/wait.h>
#  include <sys/time.h>
#  include <sys/errno.h>
#  include <sys/param.h> /* MAXPATHLEN attow */
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <libgen.h>
#  include <poll.h>
#  include <stdio.h>
#  include <string.h>
#  include <unistd.h>
#  include <sys/resource.h>

#  include "../have_const.h"
#  include "../value.h"
#  include "../custom.h"

#  include "../config.h"
#  include "../calc.h"

#  include "../have_unused.h"

#  include "../banned.h" /* include after system header <> includes */

#  define u_pfe_read_STRL_DFLT 1024
#  define pfe_pfe_SIZE_BUFFER 4096
#  define pfe_pfe_SIZE_INIT 4096

typedef enum {
    VALV_OPT_OOB,
    VALV_OPT_NULL,
    VALV_OPT_GOOD,
    VALV_OPT_BAD,
    VALV_OPT_REF_NULL,
} valv_opt;

/*
 * S_FUNC declations
 */
S_FUNC VALUE *pfe_select_TIMEOUT_DFLT = NULL;
S_FUNC VALUE *u_pfe_poll_TIMEOUT_DFLT = NULL;

/*
 * forward declarations
 */
S_FUNC size_t private_strlcat(char *dst, const char *src, size_t dsize);
S_FUNC size_t private_strlcpy(char *dst, const char *src, size_t dsize);
S_FUNC const char *type2str(short type);
S_FUNC VALUE *associndex_int(ASSOC *assoc, long index);
S_FUNC VALUE *associndex_str(ASSOC *assoc, STRING *index);
S_FUNC void associndex_int_int(ASSOC *assoc, long index, long value);
S_FUNC void associndex_str_int(ASSOC *assoc, STRING *index, long value);
S_FUNC valv_opt valv_optional_type_check(int count, VALUE **vals, int idx, const char *UNUSED(name), short wanted);
S_FUNC valv_opt valv_optional_ref_type_check(int count, VALUE **vals, int idx, const char *UNUSED(name), short wanted);
S_FUNC bool valv_type_check(int UNUSED(count), VALUE **vals, int idx, const char *UNUSED(name), short wanted);
S_FUNC void valv_type_require(const char *custname, int count, VALUE **vals, int idx, const char *name, short wanted);
S_FUNC bool valv_ref_type_check(int UNUSED(count), VALUE **vals, int idx, const char *UNUSED(name), short wanted);
S_FUNC void valv_ref_type_require(const char *custname, int count, VALUE **vals, int idx, const char *name, short wanted);
S_FUNC long valv_get_num_long(const char *custname, int count, VALUE **vals, int idx, const char *name);
S_FUNC VALUE *valv_optional_get_num(int count, VALUE **vals, int *idx, const char *name, VALUE *dflt);
#  if 0  /* function defined but not used */
S_FUNC VALUE * valv_optional_ref_num(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt);
#  endif /* function defined but not used */
S_FUNC VALUE *valv_ref_num(const char *custname, int count, VALUE **vals, int idx, const char *name);
#  if 0  /* function defined but not used */
S_FUNC VALUE * valv_get_num(const char *custname, int count, VALUE **vals, int idx,
    const char *name);
#  endif /* function defined but not used */
S_FUNC char *valv_get_str(const char *custname, int count, VALUE **vals, int idx, const char *name);
S_FUNC VALUE *valv_get_strp(const char *custname, int count, VALUE **vals, int idx, const char *name);
#  if 0  /* function defined but not used */
S_FUNC VALUE * valv_optional_ref_list(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt);
#  endif /* function defined but not used */
S_FUNC VALUE *valv_optional_get_list(int count, VALUE **vals, int *idx, const char *name, VALUE *dflt);
S_FUNC VALUE *valv_get_list(const char *custname, int count, VALUE **vals, int idx, const char *name);
S_FUNC VALUE *valv_ref_list(const char *custname, int count, VALUE **vals, int idx, const char *name);
#  if 0  /* function defined but not used */
S_FUNC VALUE * valv_optional_get_assoc(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt);
#  endif /* function defined but not used */
S_FUNC VALUE *valv_optional_ref_assoc(int count, VALUE **vals, int *idx, const char *name, VALUE *dflt);
#  if 0  /* function defined but not used */
S_FUNC VALUE * valv_get_assoc(const char *custname, int count, VALUE **vals, int idx,
    const char *name);
#  endif /* function defined but not used */
S_FUNC VALUE *valv_ref_assoc(const char *custname, int count, VALUE **vals, int idx, const char *name);
S_FUNC VALUE *optional_valv_ref_list2fd_set(const char *custname, int count, VALUE **vals, int *idx, const char *name, fd_set *fds,
                                            int *setsize);
S_FUNC VALUE *alloc_list(LIST *list);
S_FUNC VALUE *alloc_assoc(ASSOC *assoc);
S_FUNC VALUE *list_fd_get(LIST *in, fd_set *fds);
S_FUNC VALUE *alloc_num(NUMBER *num);
S_FUNC VALUE *alloc_str(STRING *str);
S_FUNC char *strext(char **subject, char *with);

/*
 * private_strlcat - size-bounded string concatenation
 *
 * Sadly, strlcat(3) is not portable enough for a number of supported calc v2 enviroments.
 * So we will include as a S_FUNC function below, strlcat.c from:
 *
 *      https://github.com/libressl/openbsd/blob/master/src/lib/libc/string/strlcat.c
 *
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 */
S_FUNC size_t
private_strlcat(char *dst, const char *src, size_t dsize)
{
    const char *odst = dst;
    const char *osrc = src;
    size_t n = dsize;
    size_t dlen;
    size_t ret;

    /* Find the end of dst and adjust bytes left but don't go past end. */
    while (n-- != 0 && *dst != '\0') {
        dst++;
    }
    dlen = dst - odst;
    n = dsize - dlen;

    if (n-- == 0) {
        return (dlen + strlen(src));
    }
    while (*src != '\0') {
        if (n != 0) {
            *dst++ = *src;
            n--;
        }
        src++;
    }
    *dst = '\0';

    /* count does not include NUL */
    ret = dlen + (src - osrc);

    return ret;
}

/*
 * private_strlcpy - size-bounded string copying
 *
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 *
 * Sadly, strlcpy(3) is not portable enough for a number of supported calc v2 enviroments.
 * So we will include as a S_FUNC function below, strlcpy.c from:
 *
 *      https://github.com/libressl/openbsd/blob/master/src/lib/libc/string/strlcpy.c
 */
S_FUNC size_t
private_strlcpy(char *dst, const char *src, size_t dsize)
{
    const char *osrc = src;
    size_t nleft = dsize;
    size_t ret;

    /* Copy as many bytes as will fit. */
    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0') {
                break;
            }
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0) {
        if (dsize != 0) {
            *dst = '\0'; /* NUL-terminate dst */
        }
        while (*src++) {
            ;
        }
    }

    /* count does not include NUL */
    ret = src - osrc - 1;

    return ret;
}

S_FUNC const char *
type2str(short type)
{
    /* originally from c_argv.c */
    switch (type) {
    case V_NULL:
        return "null"; /* null value */
    case V_INT:
        return "int"; /* normal integer */
    case V_NUM:
        return "rational_value"; /* number */
    case V_COM:
        return "complex_value"; /* complex number */
    case V_ADDR:
        return "address"; /* address of variable value */
    case V_STR:
        return "string"; /* address of string */
    case V_MAT:
        return "matrix"; /* address of matrix structure */
    case V_LIST:
        return "list"; /* address of list structure */
    case V_ASSOC:
        return "assoc"; /* address of association structure */
    case V_OBJ:
        return "object"; /* address of object structure */
    case V_FILE:
        return "file"; /* opened file id */
    case V_RAND:
        return "rand_state"; /* subtractive 100 random state */
    case V_RANDOM:
        return "random_state"; /* address of Blum random state */
    case V_CONFIG:
        return "config_state"; /* configuration state */
    case V_HASH:
        return "hash_state"; /* hash state */
    case V_BLOCK:
        return "octet_block"; /* memory block */
    case V_OCTET:
        return "octet"; /* octet (unsigned char) */
    default:
        return "unknown";
    }
}

/*
 * u_pfe_fork - create process
 *
 * returns:
 *      pid
 */
/*ARGSUSED*/
VALUE
u_pfe_fork(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result;
    int pid;

    pid = fork();

    result.v_subtype = 0;
    result.v_type = V_NUM;
    result.v_num = itoq(pid);

    return result;
}

S_FUNC VALUE *
associndex_int(ASSOC *assoc, long index)
{
    VALUE i;
    VALUE indices[1];
    VALUE *ret;

    i.v_type = V_NUM;
    i.v_num = itoq(index);

    indices[0] = i;

    ret = associndex(assoc, TRUE, 1, indices);

    return ret;
}

S_FUNC VALUE *
associndex_str(ASSOC *assoc, STRING *index)
{
    VALUE i;
    VALUE indices[1];
    VALUE *ret;

    i.v_type = V_STR;
    i.v_str = index;

    indices[0] = i;

    ret = associndex(assoc, TRUE, 1, indices);

    return ret;
}

S_FUNC void
associndex_int_int(ASSOC *assoc, long index, long value)
{
    VALUE *i;

    i = associndex_int(assoc, index);
    i->v_type = V_NUM;
    i->v_num = itoq(value);

    return;
}

S_FUNC void
associndex_str_int(ASSOC *assoc, STRING *index, long value)
{
    VALUE *i;

    i = associndex_str(assoc, index);
    i->v_type = V_NUM;
    i->v_num = itoq(value);

    return;
}

/*
 * u_pfe_pipe - create descriptor pair for interprocess communication
 *
 * returns:
 *      assoc { [0] = reading, [1] = writing }
 */
/*ARGSUSED*/
VALUE
u_pfe_pipe(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result;
    const char *custname = "pipe";
    ASSOC *assoc;
    int fds[2];

    if (pipe(fds)) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    assoc = assocalloc(2);

    associndex_int_int(assoc, 0, fds[0]);
    associndex_int_int(assoc, 1, fds[1]);

    result.v_subtype = 0;
    result.v_type = V_ASSOC;
    result.v_assoc = assoc;

    return result;
}

S_FUNC valv_opt
valv_optional_type_check(int count, VALUE **vals, int idx, const char *UNUSED(name), short wanted)
{
    if (idx > count) {
        return VALV_OPT_OOB;
    }
    if (vals[idx]->v_type == V_NULL) {
        return VALV_OPT_NULL;
    }
    if (vals[idx]->v_type == wanted) {
        return VALV_OPT_GOOD;
    }

    return VALV_OPT_BAD;
}

S_FUNC valv_opt
valv_optional_ref_type_check(int count, VALUE **vals, int idx, const char *UNUSED(name), short wanted)
{
    if (idx > count) {
        return VALV_OPT_OOB;
    }
    if (vals[idx]->v_type == V_NULL) {
        return VALV_OPT_NULL;
    }
    if (vals[idx]->v_type != V_VPTR) {
        return VALV_OPT_BAD;
    }
    if (vals[idx]->v_addr->v_type == V_NULL) {
        return VALV_OPT_REF_NULL;
    }
    if (vals[idx]->v_addr->v_type == wanted) {
        return VALV_OPT_GOOD;
    }

    return VALV_OPT_BAD;
}

S_FUNC bool
valv_type_check(int UNUSED(count), VALUE **vals, int idx, const char *UNUSED(name), short wanted)
{
    bool ret;

    ret = (vals[idx]->v_type == wanted);

    return ret;
}

S_FUNC void
valv_type_require(const char *custname, int count, VALUE **vals, int idx, const char *name, short wanted)
{
    if (!valv_type_check(count, vals, idx, name, wanted)) {
        math_error("%s: argment %d (%s) must be of type %s (%s given)", custname, idx + 1, name, type2str(wanted),
                   type2str(vals[idx]->v_type));
    }

    return;
}

S_FUNC bool
valv_ref_type_check(int UNUSED(count), VALUE **vals, int idx, const char *UNUSED(name), short wanted)
{
    bool ret;

    ret = (vals[idx]->v_addr->v_type == wanted);

    return ret;
}

S_FUNC void
valv_ref_type_require(const char *custname, int count, VALUE **vals, int idx, const char *name, short wanted)
{
    if (!valv_type_check(count, vals, idx, name, V_VPTR)) {
        math_error("%s: argment %d (%s) must be address (%s given) of type %s", custname, idx + 1, name,
                   type2str(vals[idx]->v_type), type2str(wanted));
    }
    if (!valv_ref_type_check(count, vals, idx, name, wanted)) {
        math_error("%s: argment %d (%s) must be address of type %s (%s given)", custname, idx + 1, name, type2str(wanted),
                   type2str(vals[idx]->v_addr->v_type));
    }

    return;
}

S_FUNC long
valv_get_num_long(const char *custname, int count, VALUE **vals, int idx, const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_NUM);

    return qtoi(vals[idx]->v_num);
}

S_FUNC VALUE *
valv_optional_get_num(int count, VALUE **vals, int *idx, const char *name, VALUE *dflt)
{
    valv_opt chk;
    VALUE *num;

    chk = valv_optional_type_check(count, vals, *idx, name, V_NUM);
    switch (chk) {
    case VALV_OPT_BAD:
        return dflt;
    case VALV_OPT_NULL:
        *idx += 1;
        /*FALLTHRU*/
    case VALV_OPT_OOB:
        return dflt;
    case VALV_OPT_REF_NULL:
    case VALV_OPT_GOOD:;
        /*FALLTHRU*/
    }
    num = vals[*idx];
    *idx += 1;

    return num;
}

#  if 0  /* function defined but not used */
S_FUNC VALUE *
valv_optional_ref_num(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk;
    VALUE *num;

    chk = valv_optional_ref_type_check(count, vals, *idx, name, V_NUM);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;
            /*FALLTHRU*/
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:
            ;
            /*FALLTHRU*/
    }
    num = vals[*idx]->v_addr;
    if (chk == VALV_OPT_REF_NULL) {
        num->v_type = V_NUM;
        num->v_num  = dflt->v_num;
    }
    *idx += 1;

    return num;
}
#  endif /* function defined but not used */

S_FUNC VALUE *
valv_ref_num(const char *custname, int count, VALUE **vals, int idx, const char *name)
{
    valv_ref_type_require(custname, count, vals, idx, name, V_NUM);

    return vals[idx]->v_addr;
}

#  if 0  /* function defined but not used */
S_FUNC VALUE *
valv_get_num(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_NUM);

    return vals[idx];
}
#  endif /* function defined but not used */

S_FUNC char *
valv_get_str(const char *custname, int count, VALUE **vals, int idx, const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_STR);

    return vals[idx]->v_str->s_str;
}

S_FUNC VALUE *
valv_get_strp(const char *custname, int count, VALUE **vals, int idx, const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_STR);

    return vals[idx];
}

#  if 0  /* function defined but not used */
S_FUNC VALUE *
valv_optional_ref_list(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk = valv_optional_ref_type_check(count, vals, *idx, name, V_LIST);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;
            /*FALLTHRU*/
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:
            ;
            /*FALLTHRU*/
    }
    VALUE *list = vals[*idx]->v_addr;
    if (chk == VALV_OPT_REF_NULL) {
        list->v_type = V_LIST;
        list->v_list = dflt->v_list;
    }
    *idx += 1;

    return list;
}
#  endif /* function defined but not used */

S_FUNC VALUE *
valv_optional_get_list(int count, VALUE **vals, int *idx, const char *name, VALUE *dflt)
{
    valv_opt chk;
    VALUE *list;

    chk = valv_optional_type_check(count, vals, *idx, name, V_LIST);
    switch (chk) {
    case VALV_OPT_BAD:
        return dflt;
    case VALV_OPT_NULL:
        *idx += 1;
        /*FALLTHRU*/
    case VALV_OPT_OOB:
        return dflt;
    case VALV_OPT_REF_NULL:
    case VALV_OPT_GOOD:;
        /*FALLTHRU*/
    }
    list = vals[*idx];
    *idx += 1;

    return list;
}

S_FUNC VALUE *
valv_get_list(const char *custname, int count, VALUE **vals, int idx, const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_LIST);

    return vals[idx];
}

S_FUNC VALUE *
valv_ref_list(const char *custname, int count, VALUE **vals, int idx, const char *name)
{
    valv_ref_type_require(custname, count, vals, idx, name, V_LIST);

    return vals[idx]->v_addr;
}

#  if 0  /* function defined but not used */
S_FUNC VALUE *
valv_optional_get_assoc(int count, VALUE **vals, int *idx, const char *name,
    VALUE *dflt)
{
    valv_opt chk;
    VALUE *assoc;

    chk = valv_optional_type_check(count, vals, *idx, name, V_ASSOC);
    switch (chk) {
        case VALV_OPT_BAD:
            return dflt;
        case VALV_OPT_NULL:
            *idx += 1;
            /*FALLTHRU*/
        case VALV_OPT_OOB:
            return dflt;
        case VALV_OPT_REF_NULL:
        case VALV_OPT_GOOD:
            ;
            /*FALLTHRU*/
    }
    assoc  = vals[*idx];
    *idx         += 1;

    return assoc;
}
#  endif /* function defined but not used */

S_FUNC VALUE *
valv_optional_ref_assoc(int count, VALUE **vals, int *idx, const char *name, VALUE *dflt)
{
    valv_opt chk;
    VALUE *assoc;

    chk = valv_optional_ref_type_check(count, vals, *idx, name, V_ASSOC);
    switch (chk) {
    case VALV_OPT_BAD:
        return dflt;
    case VALV_OPT_NULL:
        *idx += 1;
        /*FALLTHRU*/
    case VALV_OPT_OOB:
        return dflt;
    case VALV_OPT_REF_NULL:
    case VALV_OPT_GOOD:;
        /*FALLTHRU*/
    }
    assoc = vals[*idx]->v_addr;
    *idx += 1;
    return assoc;
}

#  if 0  /* function defined but not used */
S_FUNC VALUE *
valv_get_assoc(const char *custname, int count, VALUE **vals, int idx,
    const char *name)
{
    valv_type_require(custname, count, vals, idx, name, V_ASSOC);
    return vals[idx];
}
#  endif /* function defined but not used */

S_FUNC VALUE *
valv_ref_assoc(const char *custname, int count, VALUE **vals, int idx, const char *name)
{
    valv_ref_type_require(custname, count, vals, idx, name, V_ASSOC);

    return vals[idx]->v_addr;
}

/*
 * u_pfe_execvp - execute a file, possibly looking in paths from environment or system
 *
 * given:
 *      vals[0] file
 *      vals[1] list(arg0,...)
 *
 * returns:
 *      0 on success
 */
/*ARGSUSED*/
VALUE
u_pfe_execvp(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "execvp";
    char *path;
    VALUE *args_list;
    char **args;
    LISTELEM *el;
    int s = 0;

    path = valv_get_str(custname, count, vals, 0, "file");
    args_list = valv_get_list(custname, count, vals, 1, "args");

    if (!(args = malloc(sizeof(char *) * (args_list->v_list->l_count + 1)))) {
        math_error("%s: " __FILE__ ": %d: malloc: %s", custname, __LINE__, strerror(errno));
    }

    el = args_list->v_list->l_first;
    for (; el != NULL; el = el->e_next, s++) {
        if (el->e_value.v_type != V_STR) {
            math_error("%s: argment 2 (args) element %d must be of type string (%s given)", custname, s,
                       type2str(el->e_value.v_type));
        }

        args[s] = el->e_value.v_str->s_str;
    }
    args[s] = NULL;

    if (execvp(path, args)) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    free(args);

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(errno);
    return result;
}

/*
 * u_pfe_dup - duplicate a file descriptor
 *
 * given:
 *      vals[0] source fd
 *
 * returns:
 *      target fd
 */
/*ARGSUSED*/
VALUE
u_pfe_dup(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "dup";
    int fd;

    fd = dup(valv_get_num_long(custname, count, vals, 0, "source"));

    if (fd < 0) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(fd);

    return result;
}

/*
 * u_pfe_dup2 - duplicate a file descriptor
 *
 * given:
 *      vals[0] source fd
 *      vals[1] target fd
 *
 * returns:
 *      target fd
 */
/*ARGSUSED*/
VALUE
u_pfe_dup2(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "dup2";
    int fd;

    fd = dup2(valv_get_num_long(custname, count, vals, 0, "source"), valv_get_num_long(custname, count, vals, 1, "target"));

    if (fd < 0) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(fd);

    return result;
}

/*
 * u_pfe_close - delete a file descriptor
 *
 * given:
 *      vals[0] fd
 *
 * returns:
 *      0 on success
 */
/*ARGSUSED*/
VALUE
u_pfe_close(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "close";
    int e;

    e = close(valv_get_num_long(custname, count, vals, 0, "fd"));

    if (e) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(e);

    return result;
}

/*
 * u_pfe_write - write output
 *
 * given:
 *      vals[0] fd
 *      vals[1] string
 *
 * returns:
 *      count of bytes written
 */
/*ARGSUSED*/
VALUE
u_pfe_write(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "write";
    VALUE *strp;
    ssize_t written;

    strp = valv_get_strp(custname, count, vals, 1, "string");

    written = write(valv_get_num_long(custname, count, vals, 0, "fd"), strp->v_str->s_str, strp->v_str->s_len);

    if (written < 0) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(written);

    return result;
}

/*
 * u_pfe_read - write output
 *
 * given:
 *      vals[0] fd
 *      vals[1] count defaulting to 1024
 *
 * returns:
 *      count of bytes read
 */
/*ARGSUSED*/
VALUE
u_pfe_read(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "read";
    int strl;
    char *str;
    ssize_t r;

    strl = count < 2 ? u_pfe_read_STRL_DFLT : valv_get_num_long(custname, count, vals, 1, "count");

    str = malloc((strl + 1) * sizeof(char));

    r = read(valv_get_num_long(custname, count, vals, 0, "fd"), str, strl);
    str[r] = '\0';

    if (r < 0) {
        free(str);
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = !r ? V_NULL : V_STR;
    if (r) {
        result.v_str = makestring(str);
    }

    return result;
}

S_FUNC VALUE *
optional_valv_ref_list2fd_set(const char *custname, int count, VALUE **vals, int *idx, const char *name, fd_set *fds, int *setsize)
{
    valv_opt chk;
    VALUE *list;
    LISTELEM *el;
    int s = 0;
    int i;

    chk = valv_optional_ref_type_check(count, vals, *idx, name, V_LIST);
    switch (chk) {
    case VALV_OPT_BAD:
        return NULL;
    case VALV_OPT_NULL:
        *idx += 1;
        /*FALLTHRU*/
    case VALV_OPT_OOB:
        return NULL;
    case VALV_OPT_REF_NULL:
        math_error("%s: argument %d (%s) address of type null is not allowed", custname, *idx, name);
    case VALV_OPT_GOOD:;
        /*FALLTHRU*/
    }
    list = vals[*idx]->v_addr;
    *idx += 1;
    el = list->v_list->l_first;
    for (; el != NULL; el = el->e_next, s++) {
        if (el->e_value.v_type != V_NUM) {
            math_error("%s: argument %d (%s) element %d must be of type number (%s given)", custname, *idx, name, s,
                       type2str(el->e_value.v_type));
        }

        i = qtoi(el->e_value.v_num);
        if (*setsize < i + 1) {
            *setsize = i + 1;
        }
        FD_SET(i, fds);
    }

    return list;
}

S_FUNC VALUE *
alloc_list(LIST *list)
{
    VALUE *result;

    result = malloc(sizeof(VALUE));
    result->v_type = V_LIST;

    result->v_list = list;

    return result;
}

S_FUNC VALUE *
alloc_assoc(ASSOC *assoc)
{
    VALUE *result;

    result = malloc(sizeof(VALUE));
    result->v_type = V_ASSOC;

    result->v_assoc = assoc;

    return result;
}

S_FUNC VALUE *
list_fd_get(LIST *in, fd_set *fds)
{
    VALUE *out;
    LISTELEM *el;
    int s = 0;
    int i;
    VALUE o;

    out = alloc_list(listalloc());
    el = in->l_first;

    for (; el != NULL; el = el->e_next, s++) {
        i = qtoi(el->e_value.v_num);
        if (FD_ISSET(i, fds)) {
            copyvalue(&el->e_value, &o);
            insertlistlast(out->v_list, &o);
        }
    }

    return out;
}

S_FUNC VALUE *
alloc_num(NUMBER *num)
{
    VALUE *result;

    result = malloc(sizeof(VALUE));
    result->v_type = V_NUM;

    result->v_num = num;

    return result;
}

S_FUNC VALUE *
alloc_str(STRING *str)
{
    VALUE *result;

    result = malloc(sizeof(VALUE));
    result->v_type = V_STR;

    result->v_str = str;

    return result;
}

/*
 * u_pfe_select - examine file descriptors
 *
 * given:
 *      vals[l1+0] null or address of list (inout\ref) of fds for reading,
 *                 wh/ l1 is zero-index of 1st list
 *      vals[l1+1] null or address of list (inout\ref) of fds for writing,
 *                 wh/ l1 is zero-index of 1st list
 *      vals[l1+2] null or address of list (inout\ref) of fds for exceptional,
 *                 wh/ l1 is zero-index of 1st list
 *      vals[lZ+1] null or timeout defaulting to -1,
 *                 wh/ lZ is zero-index of last list
 *
 * returns:
 *      count of fds w/ status
 */
/*ARGSUSED*/
VALUE
u_pfe_select(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "select";
    int v = 0;
    VALUE *rdl;
    VALUE *wtl;
    fd_set rds;
    fd_set wts;
    fd_set ers;
    VALUE *erl;
    VALUE *tv;
    NUMBER *tn;
    struct timeval t;
    struct timeval *tp;
    int ssz = 0;

    FD_ZERO(&rds);
    rdl = optional_valv_ref_list2fd_set(custname, count, vals, &v, "reading", &rds, &ssz);
    FD_ZERO(&wts);
    wtl = optional_valv_ref_list2fd_set(custname, count, vals, &v, "writing", &wts, &ssz);
    FD_ZERO(&ers);
    erl = optional_valv_ref_list2fd_set(custname, count, vals, &v, "exceptional", &ers, &ssz);

    if (!pfe_select_TIMEOUT_DFLT) {
        pfe_select_TIMEOUT_DFLT = alloc_num(itoq(-1));
    }

    tv = valv_optional_get_num(count, vals, &v, "timeout", pfe_select_TIMEOUT_DFLT);

    tn = tv->v_num;
    /* originally from f_sleep() in ../func.c */
    if (qisneg(tn)) {
        tp = NULL;
    } else if (qisint(tn)) {
        if (zge31b(tn->num)) {
            math_error("sizeof(timeout) > 31 bytes");
        }
        t.tv_sec = ztoi(tn->num);
        t.tv_usec = 0;
        tp = &t;
    } else {
        NUMBER *q1, *q2;
        q1 = qscale(tn, 20); /* 2^20 = 1 Mi */
        q2 = qint(q1);
        qfree(q1);
        if (zge31b(q2->num)) {
            qfree(q2);
            math_error("sizeof(timeout as microseconds) > 31 bytes");
        }
        long usec = ztoi(q2->num);
        t.tv_sec = usec / 1000000;
        t.tv_usec = usec - t.tv_sec * 1000000;
        tp = &t;
        qfree(q2);
    }

    int r = select(ssz, &rds, &wts, &ers, tp);

    if (r < 0) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    if (rdl) {
        *rdl = *list_fd_get(rdl->v_list, &rds);
    }
    if (wtl) {
        *wtl = *list_fd_get(wtl->v_list, &wts);
    }
    if (erl) {
        *erl = *list_fd_get(erl->v_list, &ers);
    }

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(r);

    return result;
}

/*
 * u_pfe_poll - synchronous I/O multiplexing
 *
 * tested manually only slightly (as it turned out it wasn't suitable for why it was written)
 *
 * given:
 *      vals[0] list of list(fd[, event1[, ...eventN]])
 *      vals[1] address of list, appended with list(fd[, event1[, ...eventN]])
 *      vals[2] null or integer millisecond timeout defaulting to -1
 *
 * returns:
 *      count of fds w/ status
 */
/*ARGSUSED*/
VALUE
u_pfe_poll(char *UNUSED(name), int count, VALUE **vals)
{
    const char *custname = "poll";
    const char *i_name = "list of list(fd[, event1[, ...eventN]])";
    VALUE result;
    int v = 2;
    VALUE *tv;
    NUMBER *tn;
    int t;
    LIST *il;
    struct pollfd *pollfds;
    LISTELEM *el;
    int s = 0;
    LIST *el_list;
    LISTELEM *el_el;
    int el_el_s;
    char *el_el_str;
    int r;
    LIST *ol;
    VALUE *o;
    VALUE *iv;
    VALUE *ov;

    iv = valv_get_list(custname, count, vals, 0, i_name);
    ov = valv_ref_list(custname, count, vals, 1, "list");

    if (!u_pfe_poll_TIMEOUT_DFLT) {
        u_pfe_poll_TIMEOUT_DFLT = alloc_num(itoq(-1));
    }
    tv = valv_optional_get_num(count, vals, &v, "timeout", u_pfe_poll_TIMEOUT_DFLT);
    tn = tv->v_num;
    if (!qisint(tn)) {
        math_error("%s: argument 2 (timeout) must be integer", custname);
    }
    t = qtoi(tn);

    il = iv->v_list;
    pollfds = malloc(il->l_count * sizeof(struct pollfd));

    el = il->l_first;
    for (; el != NULL; el = el->e_next, s++) {
        if (el->e_value.v_type != V_LIST) {
            free(pollfds);
            math_error("%s: argument %d (%s) element %d must be of type list (%s given)", custname, 1, i_name, s,
                       type2str(el->e_value.v_type));
        }

        el_list = el->e_value.v_list;

        el_el = el_list->l_first;

        if (el_el->e_value.v_type != V_NUM) {
            free(pollfds);
            math_error("%s: argument %d (%s) element %d element %d must be of type number (%s given)", custname, 1, i_name, s, 1,
                       type2str(el_el->e_value.v_type));
        }
        if (!qisint(el_el->e_value.v_num)) {
            free(pollfds);
            math_error("%s: argument %d (%s) element %d element %d must be integer (%s given)", custname, 1, i_name, s, 1,
                       type2str(el_el->e_value.v_type));
        }
        pollfds[s].fd = qtoi(el_el->e_value.v_num);

        pollfds[s].events = 0;

        el_el = el_el->e_next;

        el_el_s = 1;
        for (; el_el != NULL; el_el = el_el->e_next, el_el_s++) {
            if (el_el->e_value.v_type != V_STR) {
                free(pollfds);
                math_error("%s: argument %d (%s) element %d element %d must be of type string (%s given)", custname, 1, i_name, s,
                           el_el_s, type2str(el_el->e_value.v_type));
            }

            el_el_str = el_el->e_value.v_str->s_str;
            if (FALSE) {
                ; /* man poll|gawk 'match($0,"^\\s*POLL([A-Z]+)",m){print(tolower(m[1]))}' */
            } else if (!strcmp(el_el_str, "err")) {
                pollfds[s].events |= POLLERR;
            } else if (!strcmp(el_el_str, "hup")) {
                pollfds[s].events |= POLLHUP;
            } else if (!strcmp(el_el_str, "in")) {
                pollfds[s].events |= POLLIN;
            } else if (!strcmp(el_el_str, "nval")) {
                pollfds[s].events |= POLLNVAL;
            } else if (!strcmp(el_el_str, "out")) {
                pollfds[s].events |= POLLOUT;
            } else if (!strcmp(el_el_str, "pri")) {
                pollfds[s].events |= POLLPRI;
            } else if (!strcmp(el_el_str, "rdband")) {
                pollfds[s].events |= POLLRDBAND;
            } else if (!strcmp(el_el_str, "rdnorm")) {
                pollfds[s].events |= POLLRDNORM;
            } else if (!strcmp(el_el_str, "wrband")) {
                pollfds[s].events |= POLLWRBAND;
            } else if (!strcmp(el_el_str, "wrnorm")) {
                pollfds[s].events |= POLLWRNORM;
            } else {
                free(pollfds);
                math_error("%s: argument %d (%s) element %d element %d must be a string containing one of the "
                           "event names from POSIX poll.h or the manual for poll, lowercase without poll prefix (%s given)",
                           custname, 1, i_name, s, el_el_s, el_el->e_value.v_str->s_str);
            }
        }

        pollfds[s].revents = 0;
    }

    r = poll(pollfds, il->l_count, t);

    if (r < 0) {
        free(pollfds);
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    ol = listalloc();

    for (s = 0; s < il->l_count; s++) {
        o = alloc_assoc(assocalloc(0));

        if ((pollfds[s].revents & POLLERR) == POLLERR) {
            associndex_str_int(o->v_assoc, makenewstring("err"), POLLERR);
        }
        if ((pollfds[s].revents & POLLHUP) == POLLHUP) {
            associndex_str_int(o->v_assoc, makenewstring("hup"), POLLHUP);
        }
        if ((pollfds[s].revents & POLLIN) == POLLIN) {
            associndex_str_int(o->v_assoc, makenewstring("in"), POLLIN);
        }
        if ((pollfds[s].revents & POLLNVAL) == POLLNVAL) {
            associndex_str_int(o->v_assoc, makenewstring("nval"), POLLNVAL);
        }
        if ((pollfds[s].revents & POLLOUT) == POLLOUT) {
            associndex_str_int(o->v_assoc, makenewstring("out"), POLLOUT);
        }
        if ((pollfds[s].revents & POLLPRI) == POLLPRI) {
            associndex_str_int(o->v_assoc, makenewstring("pri"), POLLPRI);
        }
        if ((pollfds[s].revents & POLLRDBAND) == POLLRDBAND) {
            associndex_str_int(o->v_assoc, makenewstring("rdband"), POLLRDBAND);
        }
        if ((pollfds[s].revents & POLLRDNORM) == POLLRDNORM) {
            associndex_str_int(o->v_assoc, makenewstring("rdnorm"), POLLRDNORM);
        }
        if ((pollfds[s].revents & POLLWRBAND) == POLLWRBAND) {
            associndex_str_int(o->v_assoc, makenewstring("wrband"), POLLWRBAND);
        }
        if ((pollfds[s].revents & POLLWRNORM) == POLLWRNORM) {
            associndex_str_int(o->v_assoc, makenewstring("wrnorm"), POLLWRNORM);
        }

        insertlistlast(ol, o);
    }

    ov->v_list = ol;

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(r);

    return result;
}

/*
 * u_pfe_wait4 - wait for process termination
 *
 * wh/ n1 is zero-index of 1st number,
 * given:
 *      vals[n1+0] pid wh/ null() means any children a la man 2 wait
 *                 # aka wait(2) and only vals[n1+1] allowed
 *      vals[n1+1] address of assoc of status w/ one pair of key with companion(s): "exited" and then
 *                 "exitstatus","signaled" and then
 *                 "termsig" and "coredump", "stopped" and then
 *                 "stopsig" acc2 man 2 wait
 *                 # aka wait(2)
 *      vals[n1+2] null or list of options "nohang" or "untraced" acc2 man TODO test
 *      vals[n1+3] null or address of assoc of usage TODO handle
 *
 * returns:
 *      pid or 0 w/ "nohang"
 */
/*ARGSUSED*/
VALUE
u_pfe_wait4(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "wait4";
    int v = 0;
    int stt = 0;
    struct rusage usg;
    struct rusage *usg_p;
    pid_t r;
    int opts = 0;
    VALUE *pid_num;
    VALUE *stt_assoc;
    VALUE *opt_list;
    VALUE *usg_assoc;
    LISTELEM *el;
    int s = 0;
    char *opt_str;

    pid_num = valv_optional_get_num(count, vals, &v, "pid", NULL);
    stt_assoc = valv_ref_assoc(custname, count, vals, v++, "status"); /* stat in */
    opt_list = valv_optional_get_list(count, vals, &v, "options", NULL);
    usg_assoc = valv_optional_ref_assoc(count, vals, &v, "usage", NULL);

    if (!pid_num && opt_list) {
        math_error("%s: argument 1 (pid) being null doesn't support options list", custname);
    }
    if (!pid_num && usg_assoc) {
        math_error("%s: argument 1 (pid) being null doesn't support usage assoc", custname);
    }

    if (opt_list) {
        el = opt_list->v_list->l_first;
        for (; el != NULL; el = el->e_next, s++) {
            if (el->e_value.v_type != V_STR) {
                math_error("%s: options list element %d must be of type string (%s given)", custname, s,
                           type2str(el->e_value.v_type));
            }

            opt_str = el->e_value.v_str->s_str;
            if (FALSE) {
                ;
            } else if (!strcmp(opt_str, "nohang")) {
                opts |= WNOHANG;
            } else if (!strcmp(opt_str, "untraced")) {
                opts |= WUNTRACED;
            } else {
                math_error("%s: options list element %d must be one of \"nohang\" or \"untraced\" (%s given)", custname, s,
                           opt_str);
            }
        }
    }

    usg_p = usg_assoc ? &usg : NULL;
    r = pid_num ? wait4(qtoi(pid_num->v_num), &stt, opts, usg_p) : wait(&stt);

    if (0 > r) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    if (WIFEXITED(stt)) {
        associndex_str_int(stt_assoc->v_assoc, makenewstring("exited"), TRUE);
        associndex_str_int(stt_assoc->v_assoc, makenewstring("exitstatus"), WEXITSTATUS(stt));
    }
    if (WIFSIGNALED(stt)) {
        associndex_str_int(stt_assoc->v_assoc, makenewstring("signaled"), TRUE);
        associndex_str_int(stt_assoc->v_assoc, makenewstring("termsig"), WTERMSIG(stt));
        associndex_str_int(stt_assoc->v_assoc, makenewstring("coredump"), WCOREDUMP(stt));
    }
    if (WIFSTOPPED(stt)) {
        associndex_str_int(stt_assoc->v_assoc, makenewstring("stopped"), TRUE);
        associndex_str_int(stt_assoc->v_assoc, makenewstring("stopsig"), WSTOPSIG(stt));
    }

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(r);

    return result;
}

/*
 * u_pfe_pfe - pipe/fork/exec
 *
 * given:
 *      vals[0] address of fd for in
 *      vals[1] address of fd for out
 *      vals[2] address of fd for err
 *      vals[3] list of args
 *
 * returns:
 *      forked pid
 */
/*ARGSUSED*/
VALUE
u_pfe_pfe(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "pfe";
    LISTELEM *el;
    int s;
    char **args;
    VALUE *args_list;
    VALUE *iv;
    VALUE *ov;
    VALUE *ev;
    int cci;
    int cco;
    int cce;
    int pci;
    int pco;
    int pce;
    int ia[2]; /* in array */
    int oa[2]; /* out array */
    int ea[2]; /* err array */
    pid_t child;

    if (pipe(ia)) {
        math_error("%s: " __FILE__ ": %d: (in) pipe: %s", custname, __LINE__, strerror(errno));
    }
    if (pipe(oa)) {
        math_error("%s: " __FILE__ ": %d: (out) pipe: %s", custname, __LINE__, strerror(errno));
    }
    if (pipe(ea)) {
        math_error("%s: " __FILE__ ": %d: (err) pipe: %s", custname, __LINE__, strerror(errno));
    }
    /* in, out, err values */
    iv = valv_ref_num(custname, count, vals, 0, "in");
    ov = valv_ref_num(custname, count, vals, 1, "out");
    ev = valv_ref_num(custname, count, vals, 2, "err");
    iv->v_num = itoq(ia[1]);
    ov->v_num = itoq(oa[0]);
    ev->v_num = itoq(ea[0]);

    child = fork();
    if (child) {
        /* child: close (parent in, out, err) */
        cci = !close(ia[0]) ? 0 : errno;
        cco = !close(oa[1]) ? 0 : errno;
        cce = !close(ea[1]) ? 0 : errno;

        if (cci) {
            math_error("%s: " __FILE__ ": %d: child: (parent in) close: %s", custname, __LINE__, strerror(cci));
        }
        if (cco) {
            math_error("%s: " __FILE__ ": %d: child: (parent out) close: %s", custname, __LINE__, strerror(cco));
        }
        if (cce) {
            math_error("%s: " __FILE__ ": %d: child: (parent err) close: %s", custname, __LINE__, strerror(cce));
        }

        result.v_subtype = 0;
        result.v_type = V_NUM;

        result.v_num = itoq(child);

        return result;

    } else {
        /* parent: close (child in, out, err) */
        pci = !close(ia[1]) ? 0 : errno;
        pco = !close(oa[0]) ? 0 : errno;
        pce = !close(ea[0]) ? 0 : errno;

        if (pci) {
            math_error("%s: " __FILE__ ": %d: parent: (child in) close: %s", custname, __LINE__, strerror(pci));
        }
        if (pco) {
            math_error("%s: " __FILE__ ": %d: parent: (child out) close: %s", custname, __LINE__, strerror(pco));
        }
        if (pce) {
            math_error("%s: " __FILE__ ": %d: parent: (child err) close: %s", custname, __LINE__, strerror(pce));
        }

        if (dup2(ia[0], 0) != 0) {
            math_error("%s: " __FILE__ ": %d: mismatch: (in) dup2: %s", custname, __LINE__, strerror(errno));
        }
        if (dup2(oa[1], 1) != 1) {
            math_error("%s: " __FILE__ ": %d: mismatch: (out) dup2: %s", custname, __LINE__, strerror(errno));
        }
        if (dup2(ea[1], 2) != 2) {
            math_error("%s: " __FILE__ ": %d: mismatch: (err) dup2: %s", custname, __LINE__, strerror(errno));
        }

        args_list = valv_get_list(custname, count, vals, 3, "args");

        if (!(args = malloc(sizeof(char *) * (args_list->v_list->l_count + 1)))) {
            math_error("%s: " __FILE__ ": %d: malloc: %s", custname, __LINE__, strerror(errno));
        }

        el = args_list->v_list->l_first;
        s = 0;
        for (; el != NULL; el = el->e_next, s++) {
            if (el->e_value.v_type != V_STR) {
                math_error("%s: argment 2 (args) element %d must be of type string (%s given)", custname, s,
                           type2str(el->e_value.v_type));
            }

            args[s] = el->e_value.v_str->s_str;
        }
        args[s] = NULL;

        if (execvp(args[0], args)) {
            math_error("%s: " __FILE__ ": %d: execvp: %s", custname, __LINE__, strerror(errno));
        }

        free(args);

        result.v_subtype = 0;
        result.v_type = V_NUM;

        result.v_num = itoq(errno);

        return result;
    }
}

/*
 * u_pfe_pwrite - write and close
 *
 * given:
 *      vals[0] fd
 *      vals[1] string
 *
 * returns:
 *      count of bytes written
 */
/*ARGSUSED*/
VALUE
u_pfe_pwrite(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "pwrite";
    VALUE *strp;
    int fd;
    ssize_t w;
    int e;

    strp = valv_get_strp(custname, count, vals, 1, "string");

    fd = valv_get_num_long(custname, count, vals, 0, "fd");

    w = write(fd, strp->v_str->s_str, strp->v_str->s_len);

    if (w < 0) {
        math_error("%s: " __FILE__ ": %d: write: %s", custname, __LINE__, strerror(errno));
    }

    e = close(fd);

    if (e) {
        math_error("%s: " __FILE__ ": %d: close: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(w);

    return result;
}

S_FUNC char *
strext(char **subject, char *with)
{
    size_t n;

    n = strlen(*subject) + strlen(with);
    if ((*subject = realloc(*subject, n + 1)) == NULL) {
        free(*subject);
        return NULL;
    }
    private_strlcat(*subject, with, n + 1);

    return *subject;
}

/*
 * u_pfe_pread - read until eof, close and wait for exit status
 *
 * if wait4() encounters a signalled process,this function returns the signal + 128, like bash(1).
 *
 * given:
 *      vals[0] pid
 *      vals[1] out
 *      vals[1] err
 *
 * returns:
 *      list(status, stdout_string, stderr_string)
 */
/*ARGSUSED*/
VALUE
u_pfe_pread(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "pread";
    int pid;
    int out;
    int err;
    int ssz = 0;
    fd_set nvm;  /* never mind */
    fd_set nvm2; /* never mind II */
    fd_set rbs;  /* read base set */
    /* Keep track of rbs */
    bool eoo = NULL;
    bool eoe = NULL;
    char *o;
    char *e;
    char ob[pfe_pfe_SIZE_BUFFER]; /* out buffer */
    char eb[pfe_pfe_SIZE_BUFFER]; /* err buffer */
    int pco = 0;
    int pce = 0;
    fd_set rcs; /* read check set */
    int stt = 0;
    pid_t w;
    NUMBER *r;
    LIST *list;

    pid = valv_get_num_long(custname, count, vals, 0, "pid");
    out = valv_get_num_long(custname, count, vals, 1, "out");
    err = valv_get_num_long(custname, count, vals, 2, "err");

    FD_ZERO(&nvm);
    FD_ZERO(&nvm2);

    FD_ZERO(&rbs);
    FD_SET(out, &rbs);
    if (out > ssz) {
        ssz = out;
    }
    FD_SET(err, &rbs);
    if (err > ssz) {
        ssz = err;
    }

    o = malloc(pfe_pfe_SIZE_INIT * sizeof(char));
    o[0] = '\0';
    e = malloc(pfe_pfe_SIZE_INIT * sizeof(char));
    e[0] = '\0';

    while (!eoo || !eoe) {
        memmove(&rcs, &rbs, sizeof(fd_set));

        int r = select(1 + ssz, &rcs, &nvm, &nvm2, NULL);
        if (r < 0) {
            math_error("%s: " __FILE__ ": %d: select: %s", custname, __LINE__, strerror(errno));
        }

        if (FD_ISSET(out, &rcs)) {
            r = read(out, &ob, pfe_pfe_SIZE_BUFFER);
            if (r < 0) {
                math_error("%s: " __FILE__ ": %d: (out, %lu) read: %s",
                           custname, __LINE__, (unsigned long)strlen(o), strerror(errno));
            }
            if (r) {
                ob[r] = '\0';
                strext(&o, ob);
            } else {
                if (close(out)) {
                    pco = errno;
                }
                FD_CLR(out, &rbs);
                eoo = TRUE;
            }
        }
        if (FD_ISSET(err, &rcs)) {
            r = read(err, &eb, pfe_pfe_SIZE_BUFFER);
            if (r < 0) {
                math_error("%s: " __FILE__ ": %d: (err, %lu) read: %s",
                           custname, __LINE__,(unsigned long)strlen(e), strerror(errno));
            }
            if (r) {
                eb[r] = '\0';
                strext(&e, eb);
            } else {
                if (close(err)) {
                    pce = errno;
                }
                FD_CLR(err, &rbs);
                eoe = TRUE;
            }
        }
    }

    if (pco) {
        math_error("%s: " __FILE__ ": %d: (out) close: %s", custname, __LINE__, strerror(pco));
    }
    if (pce) {
        math_error("%s: " __FILE__ ": %d: (err) close: %s", custname, __LINE__, strerror(pce));
    }

    w = wait4(pid, &stt, 0, NULL);

    if (0 > w) {
        math_error("%s: " __FILE__ ": %d: wait4: %s", custname, __LINE__, strerror(errno));
    }

    r = itoq((!WIFEXITED(stt) ? 0 : WEXITSTATUS(stt)) + (!WIFSTOPPED(stt) ? 0 : 128 + WSTOPSIG(stt)) +
             (!WIFSIGNALED(stt) ? 0 : 128 + WTERMSIG(stt)));

    list = listalloc();
    insertlistlast(list, alloc_num(r));
    insertlistlast(list, alloc_str(makestring(o)));
    insertlistlast(list, alloc_str(makestring(e)));

    result.v_subtype = 0;
    result.v_type = V_LIST;

    result.v_list = list;

    return result;
}

/*
 * u_vadd_getpid - get calling process identification
 *
 * returns:
 *      pid
 */
/*ARGSUSED*/
VALUE
u_vadd_getpid(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result;
    const char *UNUSED(custname) = "getpid";
    pid_t pid;

    pid = getpid();

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(pid);

    return result;
}

/*
 * u_vadd_getppid - get parent process identification
 *
 * returns:
 *      pid
 */
/*ARGSUSED*/
VALUE
u_vadd_getppid(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result;
    const char *UNUSED(custname) = "getppid";
    pid_t pid;

    pid = getppid();

    result.v_subtype = 0;
    result.v_type = V_NUM;

    result.v_num = itoq(pid);

    return result;
}

/*
 * u_vadd_getcwd - get working directory pathname
 *
 * returns:
 *      path
 */
/*ARGSUSED*/
VALUE
u_vadd_getcwd(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result;
    const char *custname = "getcwd";
    char *buf;

    /*
     * doing equiv(malloc) COULD use MAXPATHLEN from <sys/param.h>
     * and s/makestring/makenewstring/ below I guess
     */
    buf = getcwd(NULL, -1);
    if (!buf) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = V_STR;

    result.v_str = makestring(buf);

    return result;
}

/*
 * u_vadd_inputname - get working directory pathname
 *
 * returns:
 *      name
 */
/*ARGSUSED*/
VALUE
u_vadd_inputname(char *UNUSED(name), int UNUSED(count), VALUE **UNUSED(vals))
{
    VALUE result;
    const char *UNUSED(custname) = "inputname";
    char *buf;

    buf = inputname();
    if (!buf) {
        buf = inputisterminal() ? "<terminal>" : "<anonymous>";
    }

    result.v_subtype = 0;
    result.v_type = V_STR;

    result.v_str = makenewstring(buf);

    return result;
}

/*
 * u_vadd_basename - extract the base portion of a pathname
 *
 * given:
 *      vals[0] path
 *
 * returns:
 *      path
 */
/*ARGSUSED*/
VALUE
u_vadd_basename(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "basename";
    char buf[MAXPATHLEN];
    VALUE *strp;

    strp = valv_get_strp(custname, count, vals, 0, "path");

    private_strlcpy(buf, basename(strp->v_str->s_str), sizeof(buf));
    if (!*buf) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = V_STR;

    result.v_str = makenewstring(buf);

    return result;
}

/*
 * u_vadd_dirname - extract the base portion of a pathname
 *
 * given:
 *      vals[0] path
 *
 * returns:
 *      path
 */
/*ARGSUSED*/
VALUE
u_vadd_dirname(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;
    const char *custname = "dirname";
    VALUE *strp;
    char buf[MAXPATHLEN];

    fd_set nvm; /* never mind */
    FD_ZERO(&nvm);

    strp = valv_get_strp(custname, count, vals, 0, "path");

    private_strlcpy(buf, dirname(strp->v_str->s_str), sizeof(buf));
    if (!*buf) {
        math_error("%s: " __FILE__ ": %d: %s", custname, __LINE__, strerror(errno));
    }

    result.v_subtype = 0;
    result.v_type = V_STR;

    result.v_str = makenewstring(buf);

    return result;
}

#endif /* CUSTOM */
