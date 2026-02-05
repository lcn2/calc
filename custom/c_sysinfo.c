/*
 * c_sysinfo - names and values of selected #defines
 *
 * Copyright (C) 1999-2007,2021-2023,2026  Landon Curt Noll
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
 * Under source code control:   1997/03/09 23:14:40
 * File existed as early as:    1997
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * ISO C requires a translation unit to contain at least one declaration,
 * so we declare a global variable whose value is based on if CUSTOM is defined.
 */
#if defined(CUSTOM)
int c_sysinfo_allowed = 1; /* CUSTOM defined */
#else
int c_sysinfo_allowed = 0; /* CUSTOM undefined */
#endif

#if defined(CUSTOM)

/*
 * important <system> header includes
 */
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <ctype.h>
#  include <setjmp.h>
#  include <sys/stat.h>
#  include <stdint.h>
#  include <stdbool.h>
#  include <limits.h>

/*
 * calc local src includes
 */
#  include "../value.h"
#  include "../custom.h"
#  include "../lib_calc.h"
#  include "../calc.h"
#  define CHECK_L_FORMAT
#  include "../conf.h"
#  include "../endian_calc.h"
#  include "../file.h"
#  include "../hist.h"
#  include "../prime.h"
#  include "../attribute.h"
#  include "../have_unused.h"
#  include "../errtbl.h"

#  include "../banned.h" /* include after all other includes */

/*
 * sys_info - names and values of selected #defines
 */
struct infoname {
    char *name;    /* name of #define converted to all UPPER_CASE */
    char *meaning; /* brief explanation of the #define */
    char *str;     /* non-NULL ==> value of #define is a string */
    FULL nmbr;     /* if str==NULL ==> value fo #define as a FULL */
};
static struct infoname sys_info[] = {
    {"ADDR_REDUCT", "power of 2 address space reduction", NULL, (FULL)ADDR_REDUCT},
    {"BASE1", "one less than base", NULL, (FULL)BASE},
    {"BASE", "base for calculations", NULL, (FULL)BASE},
    {"BASEB", "bits in the calculation base", NULL, (FULL)BASEB},
    {"BASEB_LOG2", "log base 2 of bits in the calculation base", NULL, (FULL)BASEB_LOG2},
    {"BASEDIG", "number of digits in base", NULL, (FULL)BASEDIG},
    {"BLK_CHUNKSIZE", "default allocation chunk size for blocks", NULL, (FULL)BLK_CHUNKSIZE},
    {"BLK_DEF_MAXPRINT", "default block octets to print", NULL, (FULL)BLK_DEF_MAXPRINT},
    {"BLUM_PREGEN", "non-default predefined Blum generators", NULL, (FULL)BLUM_PREGEN},
    {"CALC_LITTLE_ENDIAN", "Least Significant Byte first symbol", NULL, (FULL)CALC_LITTLE_ENDIAN},
    {"CALC_BIG_ENDIAN", "Most Significant Byte first symbol", NULL, (FULL)CALC_BIG_ENDIAN},
    {"CALC_BYTE_ORDER", "Byte order (CALC_LITTLE_ENDIAN or CALC_BIG_ENDIAN)", NULL, (FULL)CALC_BYTE_ORDER},
    {"CALCEXT", "extension for files read in", CALCEXT, (FULL)0},
    {"CHAR_BIT", "length in bits of a character, or byte", NULL, (FULL)CHAR_BIT},
    {"CUSTOMHELPDIR", "location of the custom help directory", CUSTOMHELPDIR, (FULL)0},
    {"DEFAULTCALCBINDINGS", "default key bindings file", DEFAULTCALCBINDINGS, (FULL)0},
    {"DEFAULTCALCHELP", "help file that -h prints", DEFAULTCALCHELP, (FULL)0},
    {"DEFAULTCALCPAGER", "default pager", DEFAULTCALCPAGER, (FULL)0},
    {"DEFAULTCALCPATH", "default :-separated search path", DEFAULTCALCPATH, (FULL)0},
    {"DEFAULTCALCRC", "default :-separated startup file list", DEFAULTCALCRC, (FULL)0},
    {"DEFAULTSHELL", "default shell to use", DEFAULTSHELL, (FULL)0},
#if defined(DEV_T_BITS)
    {"DEV_T_BITS", "length in bits of the st_dev stat element, or device number", NULL, (FULL)DEV_T_BITS},
#endif
#if defined(DEV_T_LEN)
    {"DEV_T_LEN", "length in bytes of the st_dev stat element, or device number", NULL, (FULL)DEV_T_LEN},
#endif
    {"DISPLAY_DEFAULT", "default digits for float display", NULL, (FULL)DISPLAY_DEFAULT},
    {"EPSILON_DEFAULT", "allowed error for float calculations", EPSILON_DEFAULT, (FULL)0},
    {"EPSILONPREC_DEFAULT", "2^-EPSILON_DEFAULT <= EPSILON_DEFAULT", NULL, (FULL)EPSILONPREC_DEFAULT},
    {"ERRMAX", "default errmax value", NULL, (FULL)ERRMAX},
    {"FULL_BITS", "bits in a FULL", NULL, (FULL)FULL_BITS},
    {"FULL_LEN", "length in bytes of a FULL", NULL, (FULL)FULL_LEN},
    {"HALF_BITS", "bits in a HALF, or alias for BASEB", NULL, (FULL)HALF_BITS},
    {"HALF_LEN", "length in bytes of a HALF", NULL, (FULL)HALF_LEN},
    {"HELPDIR", "location of the help directory", HELPDIR, (FULL)0},
    {"HIST_BINDING_FILE", "Default binding file", HIST_BINDING_FILE, (FULL)0},
    {"HIST_SIZE", "Default history size", NULL, (FULL)HIST_SIZE},
    {"INIT_J", "initial 1st walking subtractive 100 shuffle table index", NULL, (FULL)INIT_J},
    {"INIT_K", "initial 2nd walking subtractive 100 shuffle table index", NULL, (FULL)INIT_K},
#if defined(INO_T_BITS)
    {"INO_T_BITS", "length in bits of the type st_ino, or inode number bit length", NULL, (FULL)INO_T_BITS},
#endif
#if defined(INO_T_LEN)
    {"INO_T_LEN", "length in bits of the type st_ino, or inode number byte length", NULL, (FULL)INO_T_LEN},
#endif
#if defined(INTPTR_LEN)
    {"INTPTR_LEN", "length in bytes of a memory pointer", NULL, (FULL)INTPTR_LEN},
#endif
#if defined(INTPTR_WIDTH)
    {"INTPTR_WIDTH", "bits in a intptr_t", NULL, (FULL)INTPTR_WIDTH},
#endif
    {"LONG_BITS", "bit length of a long", NULL, (FULL)LONG_BITS},
    {"MAP_POPCNT", "number of odd primes in pr_map", NULL, (FULL)MAP_POPCNT},
    {"MAX_CALCRC", "maximum allowed length of $CALCRC", NULL, (FULL)MAX_CALCRC},
    {"MAXCMD", "max length of command invocation", NULL, (FULL)MAXCMD},
    {"MAXDATA", "largest data object in bytes", NULL, (FULL)MAXDATA},
    {"MAXDATA_LOG2", "log base 2 of the largest data object in bytes", NULL, (FULL)MAXDATA_LOG2},
    {"MAXDIM", "max number of dimensions in matrices", NULL, (FULL)MAXDIM},
    {"MAXERROR", "max length of error message string", NULL, (FULL)MAXERROR},
    {"MAXFILES", "max number of opened files", NULL, (FULL)MAXFILES},
    {"MAXFULL", "largest SFULL value", NULL, (FULL)MAXFULL},
    {"MAXHALF", "largest SHALF value", NULL, (FULL)MAXHALF},
    {"MAXLABELS", "max number of user labels in function", NULL, (FULL)MAXLABELS},
    {"MAXLEN_LOG2", "log base 2 of MAXLEN+1", NULL, (FULL)MAXLEN_LOG2},
    {"MAXLEN", "maximum length of internal integer values in in units of HALF", NULL, (FULL)MAXLEN},
    {"MAXLONG", "largest long val", NULL, (FULL)MAXLONG},
    {"MAX_MAP_PRIME", "largest prime in pr_map", NULL, (FULL)MAX_MAP_PRIME},
    {"MAX_MAP_VAL", "largest bit in pr_map", NULL, (FULL)MAX_MAP_VAL},
    {"MAX_PFACT_VAL", "max x, for which pfact(x) is a long", NULL, (FULL)MAX_PFACT_VAL},
    {"MAXPRINT_DEFAULT", "default number of elements printed", NULL, (FULL)MAXPRINT_DEFAULT},
    {"MAXREDC", "number of entries in REDC cache", NULL, (FULL)MAXREDC},
    {"MAXSCANCOUNT", "default max scan errors before an abort", NULL, (FULL)MAXSCANCOUNT},
    {"MAX_SM_PRIME", "largest 32 bit prime", NULL, (FULL)MAX_SM_PRIME},
    {"MAX_SM_VAL", "largest 32 bit value", NULL, (FULL)MAX_SM_VAL},
    {"MAXSTACK", "max depth of evaluation stack", NULL, (FULL)MAXSTACK},
    {"MAXUFULL", "largest FULL value", NULL, (FULL)MAXUFULL},
    {"MAXULONG", "largest unsigned long val", NULL, (FULL)MAXULONG},
    {"MODE_LEN", "maximum length of a file mode string", NULL, (FULL)MODE_LEN},
    {"MUL_ALG2", "default size for alternative multiply", NULL, (FULL)MUL_ALG2},
    {"NXT_MAP_PRIME", "smallest odd prime not in pr_map", NULL, (FULL)NXT_MAP_PRIME},
    {"NXT_PFACT_VAL", "next prime for higher pfact values", NULL, (FULL)NXT_PFACT_VAL},
#if defined(OFF_T_BITS)
    {"OFF_T_BITS", "length in bits of a file offset, or off_t", NULL, (FULL)OFF_T_BITS},
#endif
#if defined(OFF_T_LEN)
    {"OFF_T_LEN", "length in bytes of a file offset, or off_t", NULL, (FULL)OFF_T_LEN},
#endif
    {"PIX_32B", "max pix() value", NULL, (FULL)PIX_32B},
    {"POW_ALG2", "default size for using REDC for powers", NULL, (FULL)POW_ALG2},
#if defined(PTR_BITS)
    {"PTR_BITS", "length in bits of a memory pointer", NULL, (FULL)PTR_BITS},
#endif
    {"REDC_ALG2", "default size using alternative REDC alg", NULL, (FULL)REDC_ALG2},
    {"REGNUM_MAX", "highest custom register number", NULL, (FULL)CUSTOM_REG_MAX},
    {"S100", "slots in an subtractive 100 table", NULL, (FULL)S100},
    {"SBITS", "size of additive or shuffle entry in bits", NULL, (FULL)SBITS},
    {"SBYTES", "size of additive or shuffle entry in bytes", NULL, (FULL)SBYTES},
    {"SCNT", "length of subtractive 100 table in FULLs", NULL, (FULL)SCNT},
    {"SEEDXORBITS", "low bits of subtractive 100 shuffle PRNG seed devoted to xor", NULL, (FULL)SEEDXORBITS},
    {"SHALFS", "size of additive or shuffle entry in HALFs", NULL, (FULL)SHALFS},
    {"SHUFCNT", "size of shuffle table in entries", NULL, (FULL)SHUFCNT},
    {"SHUFLEN", "length of shuffle table in FULLs", NULL, (FULL)SHUFLEN},
    {"SHUFMASK", "mask for shuffle table entry selection", NULL, (FULL)SHUFMASK},
    {"SHUFPOW", "power of 2 size of the shuffle table", NULL, (FULL)SHUFPOW},
    {"SLEN", "number of FULLs in a shuffle table entry", NULL, (FULL)SLEN},
    {"SQ_ALG2", "default size for alternative squaring", NULL, (FULL)SQ_ALG2},
    {"SYMBOLSIZE", "max symbol name size", NULL, (FULL)SYMBOLSIZE},
    {"TEN_MAX", "10^(2^TEN_MAX) is largest base10 conversion const", NULL, (FULL)TEN_MAX},
    {"TOPFULL", "highest bit in FULL", NULL, (FULL)TOPFULL},
    {"TOPHALF", "highest bit in a HALF", NULL, (FULL)TOPHALF},
    {"TOPLONG", "top long bit", NULL, (FULL)TOPLONG},
    {"USUAL_ELEMENTS", "usual number of elements for objects", NULL, (FULL)USUAL_ELEMENTS},

    /* must be last */
    {NULL, NULL, NULL, (FULL)0}};

/*
 * forward declarations
 */
static void dump_name_meaning(void); /* custom("sysinfo", 0) */
static void dump_name_value(void);   /* custom("sysinfo", 1) */
static void dump_mening_value(void); /* custom("sysinfo", 2) */

/*
 * c_sysinfo - return a calc #define value
 *
 * given:
 *      vals[0]   if given, name of #define to print
 *                otherwise a list of #defines are printed
 *
 * returns:
 *      value of #define if given (int or string)
 *      null if no #define arg was given
 */
/*ARGSUSED*/
VALUE
c_sysinfo(char *UNUSED(name), int count, VALUE **vals)
{
    VALUE result;       /* what we will return */
    struct infoname *p; /* current infoname */
    char *buf;          /* upper case value of vals[0] */
    char *q;            /* to upper case converter */
    char *r;            /* to upper case converter */

    /*
     * we will return NULL if a value was not found
     */
    result.v_type = V_NULL;
    result.v_subtype = V_NOSUBTYPE;

    /*
     * case 0: if no args, then dump the table with no values
     */
    if (count == 0) {

        /* dump the entire table */
        dump_name_meaning();

        /*
         * case 1: numeric arg is given
         */
    } else if (vals[0]->v_type == V_NUM) {

        /* firewall - must be a tiny non-negative integer */
        if (qisneg(vals[0]->v_num) || qisfrac(vals[0]->v_num) || zge31b(vals[0]->v_num->num)) {
            math_error("sysinfo: arg must be string, 0, 1 or 2");
            not_reached();
        }

        /*
         * select action based on numeric value of arg
         */
        switch (z1tol(vals[0]->v_num->num)) {
        case 0: /* print all infonames and meanings */
            dump_name_meaning();
            break;
        case 1: /* print all infonames and values */
            dump_name_value();
            break;
        case 2: /* print all values and meanings */
            dump_mening_value();
            break;
        default:
            math_error("sysinfo: arg must be string, 0, 1 or 2");
            not_reached();
        }

        /*
         * case 2: string arg is given
         *
         * The string is taken to be the infoname we want to print.
         */
    } else if (vals[0]->v_type == V_STR) {

        /* convert vals[0] to upper case string */
        buf = (char *)calloc(strlen((char *)vals[0]->v_str->s_str) + 1, sizeof(char));
        for (q = (char *)vals[0]->v_str->s_str, r = buf; *q; ++q, ++r) {
            if (isascii((int)*q) && islower((int)*q)) {
                *r = toupper((int)*q);
            } else {
                *r = *q;
            }
        }
        *r = '\0';

        /* search the table for the infoname */
        for (p = sys_info; p->name != NULL; ++p) {

            if (strcmp(p->name, buf) == 0) {

                /* found the infoname */
                if (p->str == NULL) {
                    /* return value as integer */
                    result.v_type = V_NUM;
                    result.v_num = utoq(p->nmbr);
                } else {
                    /* return value as string */
                    result.v_type = V_STR;
                    result.v_subtype = V_NOSUBTYPE;
                    result.v_str = makenewstring(p->str);
                }

                /* return found infotype as value */
                break;
            }
        }

        /* free converted upper case string buffer */
        if (buf != NULL) {
            free(buf);
            buf = NULL;
        }

        /*
         * bad arg given
         */
    } else {
        math_error("sysinfo: arg must be string, 0, 1 or 2");
        not_reached();
    }

    /*
     * return what we found or didn't find
     */
    return result;
}

/*
 * dump_name_meaning - print all infonames and meanings
 */
static void
dump_name_meaning(void)
{
    struct infoname *p; /* current infoname */

    /* dump the entire table */
    for (p = sys_info; p->name != NULL; ++p) {
        printf("%s%-23s\t%s\n", (conf->tab_ok ? "\t" : ""), p->name, p->meaning);
    }
}

/*
 * dump_name_value - print all infonames and values
 */
static void
dump_name_value(void)
{
    struct infoname *p; /* current infoname */

    /* dump the entire table */
    for (p = sys_info; p->name != NULL; ++p) {
        if (p->str == NULL) {
#  if LONG_BITS == FULL_BITS || FULL_BITS == 32
            printf("%s%-23s\t%-8lu\t(0x%lx)\n", (conf->tab_ok ? "\t" : ""), p->name, (unsigned long)p->nmbr,
                   (unsigned long)p->nmbr);
#  else
            printf("%s%-23s\t%-8llu\t(0x%llx)\n", (conf->tab_ok ? "\t" : ""), p->name, (unsigned long long)p->nmbr,
                   (unsigned long long)p->nmbr);
#  endif
        } else {
            printf("%s%-23s\t\"%s\"\n", (conf->tab_ok ? "\t" : ""), p->name, p->str);
        }
    }
}

/*
 * dump_mening_value - print all values and meanings
 */
static void
dump_mening_value(void)
{
    struct infoname *p; /* current infoname */

    /* dump the entire table */
    for (p = sys_info; p->name != NULL; ++p) {
        if (p->str == NULL) {
#  if LONG_BITS == FULL_BITS || FULL_BITS == 32
            printf("%s%-36.36s\t%-8lu\t(0x%lx)\n", (conf->tab_ok ? "\t" : ""), p->meaning, (unsigned long)p->nmbr,
                   (unsigned long)p->nmbr);
#  else
            printf("%s%-36.36s\t%-8llu\t(0x%llx)\n", (conf->tab_ok ? "\t" : ""), p->meaning, (unsigned long long)p->nmbr,
                   (unsigned long long)p->nmbr);
#  endif
        } else {
            printf("%s%-36.36s\t\"%s\"\n", (conf->tab_ok ? "\t" : ""), p->meaning, p->str);
        }
    }
}

#endif
