/*
 * errtbl - calc computation error codes, symbols and messages
 *
 * This code is used to generate errsym.h and help/errorcodes.
 * This code also verifies the consistency of the error_table[] array.
 *
 * Copyright (C) 1999-2006,2021,2023  Ernest Bowen and Landon Curt Noll
 *
 * Calc is open software; you can redistribute it and/or modify it under
 * the terms of the version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * Calc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU Lesser General
 * Public License for more details.
 *
 * A copy of version 2.1 of the GNU Lesser General Public License is
 * distributed with calc under the filename COPYING-LGPL.  You should have
 * received a copy with calc; if not, write to Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Under source code control:	1996/05/23 17:38:44
 * File existed as early as:	1996
 *
 * Share and enjoy!  :-)		http://www.isthe.com/chongo/tech/comp/calc/
 *
 * HISTORIC NOTE:
 *
 * This file was once called calcerr.tbl.  It once was a simple table of:
 *
 *		SYMBOL	meaning
 *
 *	 until 2023 when the new calc computation error code system was introduced.
 */


/* special comments for the seqcexit tool */
/* exit code out of numerical order - ignore in sequencing - ooo */
/* exit code change of order - use new value in sequencing - coo */


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "func.h"
#include "errtbl.h"
#include "bool.h"


/*
 * Copyright (C) year for generated files
 */
#define ERRTBL_COPYRIGHT_YEAR 2023

/*
 * number of calc computation error codes in the error_table[] array
 *
 * This count does NOT include the initial E__BASE entry NOR
 * the final NULL entry.
 */
#define MY_ECOUNT ((sizeof(error_table) / sizeof(error_table[0])) - 2)

/*
 * number of calc computation error codes in the private_error_alias[] array
 *
 * This count does NOT include the final NULL entry.
 */
#define MY_PRIV_ECOUNT ((sizeof(private_error_alias) / sizeof(private_error_alias[0])) - 1)

/* highest assigned calc computation error code */
#define MY_E__HIGHEST (E__BASE + MY_ECOUNT)

/*
 * errnum waiting to be replaced by E__HIGHEST
 *
 * The INV_ERRNUM must be < 0, AND != NULL_ERRNUM
 */
#define INV_ERRNUM (-2)		/* errnum is waiting to be replaced by E__HIGHEST */


/************************************************************************/
/*									*/
/*  WARNING: The order of ierror_table[] arrays critical!  If you	*/
/*	     change the order, you will break code that depends on the  */
/*	     return value of the errno() builtin function.		*/
/*									*/
/*	     When adding entries to this table add then just before     */
/*	     the NULL pointer (* must be last *) entry.			*/
/*									*/
/************************************************************************/

/*
 * The error_table[] array represents the calc computation error related
 * error codes, symbols and messages.
 *
 * The errnum of the 1st entry error_table[0] must be E__BASE.
 *
 * All errnum for the following entries just be consecutive,
 * except for the final NULL entry.
 *
 * The final entry must have an errnum of NULL_ERRNUM (-1),
 * errsym of NULL and errmsg of NULL.
 *
 * With exception to the 1st E__BASE entry, all other errsym strings
 * must match the following regular expression:
 *
 *	^E_[A-Z][A-Z0-9_]+$
 *
 * NOTE: The above regular expression is more restrictive them the
 *	 E_STRING regular expression from help/errno, help/error,
 *	 and help/strerror.  This is because errsym strings that
 *	 start with "E__" are special symbols that #define-d in errtbl.h,
 *	 AND because errsym strings that are "^E_[0-9]+$" reserved for
 *	 numeric aliases for errnum.
 *
 * If multiple computation error codes are needed for a similar purpose,
 * use E_STRING errsym codes followed by _digit(s).
 *
 * Keep the errmsg short enough that lines in this table are not too long.
 * You might want to keep the length of the errmsg to 80 characters or less.
 *
 * IMPORTANT NOTE:
 *
 * The list of errnum values in error_table[], and the consecutive values
 * starting from the 10000 (E__BASE) entry) is important to maintain for
 * compatibility.  DO NOT change the values of existing errnum codes once
 * a new code is released in a calc stable version.  If a different code
 * is needed, add a new code to the bottom (just above the final NULL entry).
 *
 * Starting with calc version 2.15 the E_STRING errsym values became visible
 * via the error, errno, errsym and strerror builtin interface.  DO NOT change
 * the existing E_STRING errsym codes once a new code is released in a calc
 * stable version.  If a different E_STRING errsym code is needed, add a new
 * entry to the bottom (just above the final NULL entry).
 *
 * These is NO requirement for the errmsg error message to be backward compatible.
 *
 * There is a convention that when a computation error code is no longer needed,
 * that the phrase: "UNUSED ERROR: " be prepended to the errmsg error message.
 * Do not remove such computation error codes, nor change their errnum values,
 * not change their E_STRING errsym codes.
 */
CONST struct errtbl error_table[] = {

	/* The E__BASE entry below must start with 10000 and must be first!! */
	{ 10000,	"E__BASE",		"No error" },
	{ 10001,	"E_DIVBYZERO",		"Division by zero" },
	{ 10002,	"E_ZERODIVZERO",	"Indeterminate (0/0)" },
	{ 10003,	"E_ADD",		"Bad arguments for +" },
	{ 10004,	"E_SUB",		"Bad arguments for binary -" },
	{ 10005,	"E_MUL",		"Bad arguments for *" },
	{ 10006,	"E_DIV",		"Bad arguments for /" },
	{ 10007,	"E_NEG",		"Bad argument for unary -" },
	{ 10008,	"E_SQUARE",		"Bad argument for squaring" },
	{ 10009,	"E_INV",		"Bad argument for inverse" },
	{ 10010,	"E_INCV",		"Bad argument for ++" },
	{ 10011,	"E_DECV",		"Bad argument for --" },
	{ 10012,	"E_INT",		"Bad argument for int" },
	{ 10013,	"E_FRAC",		"Bad argument for frac" },
	{ 10014,	"E_CONJ",		"Bad argument for conj" },
	{ 10015,	"E_APPR_1",		"Bad first argument for appr" },
	{ 10016,	"E_APPR_2",		"Bad second argument for appr" },
	{ 10017,	"E_APPR_3",		"Bad third argument for appr" },
	{ 10018,	"E_ROUND_1",		"Bad first argument for round" },
	{ 10019,	"E_ROUND_2",		"Bad second argument for round" },
	{ 10020,	"E_ROUND_3",		"Bad third argument for round" },
	{ 10021,	"E_BROUND_1",		"Bad first argument for bround" },
	{ 10022,	"E_BROUND_2",		"Bad second argument for bround" },
	{ 10023,	"E_BROUND_3",		"Bad third argument for bround" },
	{ 10024,	"E_SQRT_1",		"Bad first argument for sqrt" },
	{ 10025,	"E_SQRT_2",		"Bad second argument for sqrt" },
	{ 10026,	"E_SQRT_3",		"Bad third argument for sqrt" },
	{ 10027,	"E_ROOT_1",		"Bad first argument for root" },
	{ 10028,	"E_ROOT_2",		"Bad second argument for root" },
	{ 10029,	"E_ROOT_3",		"Bad third argument for root" },
	{ 10030,	"E_NORM",		"Bad argument for norm" },
	{ 10031,	"E_SHIFT_1",		"Bad first argument for << or >>" },
	{ 10032,	"E_SHIFT_2",		"Bad second argument for << or >>" },
	{ 10033,	"E_SCALE_1",		"Bad first argument for scale" },
	{ 10034,	"E_SCALE_2",		"Bad second argument for scale" },
	{ 10035,	"E_POWI_1",		"Bad first argument for ^" },
	{ 10036,	"E_POWI_2",		"Bad second argument for ^" },
	{ 10037,	"E_POWER_1",		"Bad first argument for power" },
	{ 10038,	"E_POWER_2",		"Bad second argument for power" },
	{ 10039,	"E_POWER_3",		"Bad third argument for power" },
	{ 10040,	"E_QUO_1",		"Bad first argument for quo or //" },
	{ 10041,	"E_QUO_2",		"Bad second argument for quo or //" },
	{ 10042,	"E_QUO_3",		"Bad third argument for quo" },
	{ 10043,	"E_MOD_1",		"Bad first argument for mod or %" },
	{ 10044,	"E_MOD_2",		"Bad second argument for mod or %" },
	{ 10045,	"E_MOD_3",		"Bad third argument for mod" },
	{ 10046,	"E_SGN",		"Bad argument for sgn" },
	{ 10047,	"E_ABS_1",		"Bad first argument for abs" },
	{ 10048,	"E_ABS_2",		"Bad second argument for abs" },
	{ 10049,	"E_EVAL",		"Scan error in argument for eval" },
	{ 10050,	"E_STR",		"Non-simple type for str" },
	{ 10051,	"E_EXP_1",		"Non-real epsilon for exp" },
	{ 10052,	"E_EXP_2",		"Bad first argument for exp" },
	{ 10053,	"E_FPUTC_1",		"Non-file first argument for fputc" },
	{ 10054,	"E_FPUTC_2",		"Bad second argument for fputc" },
	{ 10055,	"E_FPUTC_3",		"File not open for writing for fputc" },
	{ 10056,	"E_FGETC_1",		"Non-file first argument for fgetc" },
	{ 10057,	"E_FGETC_2",		"File not open for reading for fgetc" },
	{ 10058,	"E_FOPEN_1",		"Non-string arguments for fopen" },
	{ 10059,	"E_FOPEN_2",		"Unrecognized mode for fopen" },
	{ 10060,	"E_FREOPEN_1",		"Non-file first argument for freopen" },
	{ 10061,	"E_FREOPEN_2",		"Non-string or unrecognized mode for freopen" },
	{ 10062,	"E_FREOPEN_3",		"Non-string third argument for freopen" },
	{ 10063,	"E_FCLOSE_1",		"Non-file argument for fclose" },
	{ 10064,	"E_FFLUSH",		"Non-file argument for fflush" },
	{ 10065,	"E_FPUTS_1",		"Non-file first argument for fputs" },
	{ 10066,	"E_FPUTS_2",		"Non-string argument after first for fputs" },
	{ 10067,	"E_FPUTS_3",		"File not open for writing for fputs" },
	{ 10068,	"E_FGETS_1",		"Non-file argument for fgets" },
	{ 10069,	"E_FGETS_2",		"File not open for reading for fgets" },
	{ 10070,	"E_FPUTSTR_1",		"Non-file first argument for fputstr" },
	{ 10071,	"E_FPUTSTR_2",		"Non-string argument after first for fputstr" },
	{ 10072,	"E_FPUTSTR_3",		"File not open for writing for fputstr" },
	{ 10073,	"E_FGETSTR_1",		"Non-file first argument for fgetstr" },
	{ 10074,	"E_FGETSTR_2",		"File not open for reading for fgetstr" },
	{ 10075,	"E_FGETLINE_1",		"Non-file  argument for fgetline" },
	{ 10076,	"E_FGETLINE_2",		"File not open for reading for fgetline" },
	{ 10077,	"E_FGETFIELD_1",	"Non-file argument for fgetfield" },
	{ 10078,	"E_FGETFIELD_2",	"File not open for reading for fgetfield" },
	{ 10079,	"E_REWIND_1",		"Non-file argument for rewind" },
	{ 10080,	"E_FILES",		"Non-integer argument for files" },
	{ 10081,	"E_PRINTF_1",		"Non-string fmt argument for fprint" },
	{ 10082,	"E_PRINTF_2",		"Stdout not open for writing to ???" },
	{ 10083,	"E_FPRINTF_1",		"Non-file first argument for fprintf" },
	{ 10084,	"E_FPRINTF_2",		"Non-string second (fmt) argument for fprintf" },
	{ 10085,	"E_FPRINTF_3",		"File not open for writing for fprintf" },
	{ 10086,	"E_STRPRINTF_1",	"Non-string first (fmt) argument for strprintf" },
	{ 10087,	"E_STRPRINTF_2",	"Error in attempting strprintf ???" },
	{ 10088,	"E_FSCAN_1",		"Non-file first argument for fscan" },
	{ 10089,	"E_FSCAN_2",		"File not open for reading for fscan" },
	{ 10090,	"E_STRSCAN",		"Non-string first argument for strscan" },
	{ 10091,	"E_FSCANF_1",		"Non-file first argument for fscanf" },
	{ 10092,	"E_FSCANF_2",		"Non-string second (fmt) argument for fscanf" },
	{ 10093,	"E_FSCANF_3",		"Non-lvalue argument after second for fscanf" },
	{ 10094,	"E_FSCANF_4",		"File not open for reading or other error for fscanf" },
	{ 10095,	"E_STRSCANF_1",		"Non-string first argument for strscanf" },
	{ 10096,	"E_STRSCANF_2",		"Non-string second (fmt) argument for strscanf" },
	{ 10097,	"E_STRSCANF_3",		"Non-lvalue argument after second for strscanf" },
	{ 10098,	"E_STRSCANF_4",		"Some error in attempting strscanf ???" },
	{ 10099,	"E_SCANF_1",		"Non-string first (fmt) argument for scanf" },
	{ 10100,	"E_SCANF_2",		"Non-lvalue argument after first for scanf" },
	{ 10101,	"E_SCANF_3",		"Some error in attempting scanf ???" },
	{ 10102,	"E_FTELL_1",		"Non-file argument for ftell" },
	{ 10103,	"E_FTELL_2",		"File not open or other error for ftell" },
	{ 10104,	"E_FSEEK_1",		"Non-file first argument for fseek" },
	{ 10105,	"E_FSEEK_2",		"Non-integer or negative second argument for fseek" },
	{ 10106,	"E_FSEEK_3",		"File not open or other error for fseek" },
	{ 10107,	"E_FSIZE_1",		"Non-file argument for fsize" },
	{ 10108,	"E_FSIZE_2",		"File not open or other error for fsize" },
	{ 10109,	"E_FEOF_1",		"Non-file argument for feof" },
	{ 10110,	"E_FEOF_2",		"File not open or other error for feof" },
	{ 10111,	"E_FERROR_1",		"Non-file argument for ferror" },
	{ 10112,	"E_FERROR_2",		"File not open or other error for ferror" },
	{ 10113,	"E_UNGETC_1",		"Non-file argument for ungetc" },
	{ 10114,	"E_UNGETC_2",		"File not open for reading for ungetc" },
	{ 10115,	"E_UNGETC_3",		"Bad second argument or other error for ungetc" },
	{ 10116,	"E_BIGEXP",		"Exponent too big in scanning" },
	{ 10117,	"E_ISATTY_1",		"UNUSED ERROR: E_ISATTY_1 is no longer used" },
	{ 10118,	"E_ISATTY_2",		"UNUSED ERROR: E_ISATTY_2 is no longer used" },
	{ 10119,	"E_ACCESS_1",		"Non-string first argument for access" },
	{ 10120,	"E_ACCESS_2",		"Bad second argument for access" },
	{ 10121,	"E_SEARCH_1",		"Bad first argument for search" },
	{ 10122,	"E_SEARCH_2",		"Bad second argument for search" },
	{ 10123,	"E_SEARCH_3",		"Bad third argument for search" },
	{ 10124,	"E_SEARCH_4",		"Bad fourth argument for search" },
	{ 10125,	"E_SEARCH_5",		"Cannot find fsize or fpos for search" },
	{ 10126,	"E_SEARCH_6",		"File not readable for search" },
	{ 10127,	"E_RSEARCH_1",		"Bad first argument for rsearch" },
	{ 10128,	"E_RSEARCH_2",		"Bad second argument for rsearch" },
	{ 10129,	"E_RSEARCH_3",		"Bad third argument for rsearch" },
	{ 10130,	"E_RSEARCH_4",		"Bad fourth argument for rsearch" },
	{ 10131,	"E_RSEARCH_5",		"Cannot find fsize or fpos for rsearch" },
	{ 10132,	"E_RSEARCH_6",		"File not readable for rsearch" },
	{ 10133,	"E_MANYOPEN",		"Too many open files" },
	{ 10134,	"E_REWIND_2",		"Attempt to rewind a file that is not open" },
	{ 10135,	"E_STRERROR_1",		"Bad argument type for strerror" },
	{ 10136,	"E_STRERROR_2",		"Numeric argument is outside valid errnum range for strerror" },
	{ 10137,	"E_COS_1",		"Bad epsilon for cos" },
	{ 10138,	"E_COS_2",		"Bad first argument for cos" },
	{ 10139,	"E_SIN_1",		"Bad epsilon for sin" },
	{ 10140,	"E_SIN_2",		"Bad first argument for sin" },
	{ 10141,	"E_EVAL_2",		"Non-string argument for eval" },
	{ 10142,	"E_ARG_1",		"Bad epsilon for arg" },
	{ 10143,	"E_ARG_2",		"Bad first argument for arg" },
	{ 10144,	"E_POLAR_1",		"Non-real argument for polar" },
	{ 10145,	"E_POLAR_2",		"Bad epsilon for polar" },
	{ 10146,	"E_FCNT",		"UNUSED ERROR: Non-integral argument for fcnt" },
	{ 10147,	"E_MATFILL_1",		"Non-variable first argument for matfill" },
	{ 10148,	"E_MATFILL_2",		"Non-matrix first argument-value for matfill" },
	{ 10149,	"E_MATDIM",		"Non-matrix argument for matdim" },
	{ 10150,	"E_MATSUM",		"Non-matrix argument for matsum" },
	{ 10151,	"E_ISIDENT",		"UNUSED ERROR: E_ISIDENT is no longer used" },
	{ 10152,	"E_MATTRANS_1",		"Non-matrix argument for mattrans" },
	{ 10153,	"E_MATTRANS_2",		"Non-two-dimensional matrix for mattrans" },
	{ 10154,	"E_DET_1",		"Non-matrix argument for det" },
	{ 10155,	"E_DET_2",		"Matrix for det not of dimension 2" },
	{ 10156,	"E_DET_3",		"Non-square matrix for det" },
	{ 10157,	"E_MATMIN_1",		"Non-matrix first argument for matmin" },
	{ 10158,	"E_MATMIN_2",		"Non-positive-integer second argument for matmin" },
	{ 10159,	"E_MATMIN_3",		"Second argument for matmin exceeds dimension" },
	{ 10160,	"E_MATMAX_1",		"Non-matrix first argument for matmin" },
	{ 10161,	"E_MATMAX_2",		"Second argument for matmax not positive integer" },
	{ 10162,	"E_MATMAX_3",		"Second argument for matmax exceeds dimension" },
	{ 10163,	"E_CP_1",		"Non-matrix argument for cp" },
	{ 10164,	"E_CP_2",		"Non-one-dimensional matrix for cp" },
	{ 10165,	"E_CP_3",		"Matrix size not 3 for cp" },
	{ 10166,	"E_DP_1",		"Non-matrix argument for dp" },
	{ 10167,	"E_DP_2",		"Non-one-dimensional matrix for dp" },
	{ 10168,	"E_DP_3",		"Different-size matrices for dp" },
	{ 10169,	"E_STRLEN",		"Non-string argument for strlen" },
	{ 10170,	"E_STRCAT",		"Non-string argument for strcat" },
	{ 10171,	"E_SUBSTR_1",		"Non-string first argument for strcat" },
	{ 10172,	"E_SUBSTR_2",		"Non-non-negative integer second argument for strcat" },
	{ 10173,	"E_CHAR",		"Bad argument for char" },
	{ 10174,	"E_ORD",		"Non-string argument for ord" },
	{ 10175,	"E_INSERT_1",		"Non-list-variable first argument for insert" },
	{ 10176,	"E_INSERT_2",		"Non-integral second argument for insert" },
	{ 10177,	"E_PUSH",		"Non-list-variable first argument for push" },
	{ 10178,	"E_APPEND",		"Non-list-variable first argument for append" },
	{ 10179,	"E_DELETE_1",		"Non-list-variable first argument for delete" },
	{ 10180,	"E_DELETE_2",		"Non-integral second argument for delete" },
	{ 10181,	"E_POP",		"Non-list-variable argument for pop" },
	{ 10182,	"E_REMOVE",		"Non-list-variable argument for remove" },
	{ 10183,	"E_LN_1",		"Bad epsilon argument for ln" },
	{ 10184,	"E_LN_2",		"Non-numeric first argument for ln" },
	{ 10185,	"E_ERROR_1",		"Invalid argument type for error" },
	{ 10186,	"E_ERROR_2",		"Numeric argument is outside valid errnum range for error" },
	{ 10187,	"E_EVAL_3",		"Attempt to eval at maximum input depth" },
	{ 10188,	"E_EVAL_4",		"Unable to open string for reading" },
	{ 10189,	"E_RM_1",		"First argument for rm is not a non-empty string" },
	{ 10190,	"E_RM_2",		"UNUSED ERROR: Unable to remove a file" },
	{ 10191,	"E_RDPERM",		"UNUSED ERROR: Operation allowed because calc mode disallows read operations" },
	{ 10192,	"E_WRPERM",		"Operation allowed because calc mode disallows write operations" },
	{ 10193,	"E_EXPERM",		"UNUSED ERROR: Operation allowed because calc mode disallows exec operations" },
	{ 10194,	"E_MIN",		"Unordered arguments for min" },
	{ 10195,	"E_MAX",		"Unordered arguments for max" },
	{ 10196,	"E_LISTMIN",		"Unordered items for minimum of list" },
	{ 10197,	"E_LISTMAX",		"Unordered items for maximum of list" },
	{ 10198,	"E_SIZE",		"Size undefined for argument type" },
	{ 10199,	"E_NO_C_ARG",		"Calc must be run with a -C argument to use custom function" },
	{ 10200,	"E_NO_CUSTOM",		"Calc was built with custom functions disabled" },
	{ 10201,	"E_UNK_CUSTOM",		"Custom function unknown, try: show custom" },
	{ 10202,	"E_BLK_1",		"Non-integral length for block" },
	{ 10203,	"E_BLK_2",		"Negative or too-large length for block" },
	{ 10204,	"E_BLK_3",		"Non-integral chunksize for block" },
	{ 10205,	"E_BLK_4",		"Negative or too-large chunksize for block" },
	{ 10206,	"E_BLKFREE_1",		"Named block does not exist for blkfree" },
	{ 10207,	"E_BLKFREE_2",		"Non-integral id specification for blkfree" },
	{ 10208,	"E_BLKFREE_3",		"Block with specified id does not exist" },
	{ 10209,	"E_BLKFREE_4",		"Block already freed" },
	{ 10210,	"E_BLKFREE_5",		"No-realloc protection prevents blkfree" },
	{ 10211,	"E_BLOCKS_1",		"Non-integer argument for blocks" },
	{ 10212,	"E_BLOCKS_2",		"Non-allocated index number for blocks" },
	{ 10213,	"E_COPY_01",		"Non-integer or negative source index for copy" },
	{ 10214,	"E_COPY_02",		"Source index too large for copy" },
	{ 10215,	"E_COPY_03",		"UNUSED ERROR: E_COPY_03 is no longer used" },
	{ 10216,	"E_COPY_04",		"Non-integer or negative number for copy" },
	{ 10217,	"E_COPY_05",		"Number too large for copy" },
	{ 10218,	"E_COPY_06",		"Non-integer or negative destination index for copy" },
	{ 10219,	"E_COPY_07",		"Destination index too large for copy" },
	{ 10220,	"E_COPY_08",		"Freed block source for copy" },
	{ 10221,	"E_COPY_09",		"Unsuitable source type for copy" },
	{ 10222,	"E_COPY_10",		"Freed block destination for copy" },
	{ 10223,	"E_COPY_11",		"Unsuitable destination type for copy" },
	{ 10224,	"E_COPY_12",		"Incompatible source and destination for copy" },
	{ 10225,	"E_COPY_13",		"No-copy-from source variable" },
	{ 10226,	"E_COPY_14",		"No-copy-to destination variable" },
	{ 10227,	"E_COPY_15",		"No-copy-from source named block" },
	{ 10228,	"E_COPY_16",		"No-copy-to destination named block" },
	{ 10229,	"E_COPY_17",		"No-relocate destination for copy" },
	{ 10230,	"E_COPYF_1",		"File not open for copy" },
	{ 10231,	"E_COPYF_2",		"fseek or fsize failure for copy" },
	{ 10232,	"E_COPYF_3",		"fwrite error for copy" },
	{ 10233,	"E_COPYF_4",		"fread error for copy" },
	{ 10234,	"E_PROTECT_1",		"Non-variable first argument for protect" },
	{ 10235,	"E_PROTECT_2",		"Bad second argument for protect" },
	{ 10236,	"E_PROTECT_3",		"Bad third argument for protect" },
	{ 10237,	"E_MATFILL_3",		"No-copy-to destination for matfill" },
	{ 10238,	"E_MATFILL_4",		"No-assign-from source for matfill" },
	{ 10239,	"E_MATTRACE_1",		"Non-matrix argument for mattrace" },
	{ 10240,	"E_MATTRACE_2",		"Non-two-dimensional argument for mattrace" },
	{ 10241,	"E_MATTRACE_3",		"Non-square argument for mattrace" },
	{ 10242,	"E_TAN_1",		"Bad epsilon for tan" },
	{ 10243,	"E_TAN_2",		"Bad argument for tan" },
	{ 10244,	"E_COT_1",		"Bad epsilon for cot" },
	{ 10245,	"E_COT_2",		"Bad argument for cot" },
	{ 10246,	"E_SEC_1",		"Bad epsilon for sec" },
	{ 10247,	"E_SEC_2",		"Bad argument for sec" },
	{ 10248,	"E_CSC_1",		"Bad epsilon for csc" },
	{ 10249,	"E_CSC_2",		"Bad argument for csc" },
	{ 10250,	"E_SINH_1",		"Bad epsilon for sinh" },
	{ 10251,	"E_SINH_2",		"Bad argument for sinh" },
	{ 10252,	"E_COSH_1",		"Bad epsilon for cosh" },
	{ 10253,	"E_COSH_2",		"Bad argument for cosh" },
	{ 10254,	"E_TANH_1",		"Bad epsilon for tanh" },
	{ 10255,	"E_TANH_2",		"Bad argument for tanh" },
	{ 10256,	"E_COTH_1",		"Bad epsilon for coth" },
	{ 10257,	"E_COTH_2",		"Bad argument for coth" },
	{ 10258,	"E_SECH_1",		"Bad epsilon for sech" },
	{ 10259,	"E_SECH_2",		"Bad argument for sech" },
	{ 10260,	"E_CSCH_1",		"Bad epsilon for csch" },
	{ 10261,	"E_CSCH_2",		"Bad argument for csch" },
	{ 10262,	"E_ASIN_1",		"Bad epsilon for asin" },
	{ 10263,	"E_ASIN_2",		"Bad argument for asin" },
	{ 10264,	"E_ACOS_1",		"Bad epsilon for acos" },
	{ 10265,	"E_ACOS_2",		"Bad argument for acos" },
	{ 10266,	"E_ATAN_1",		"Bad epsilon for atan" },
	{ 10267,	"E_ATAN_2",		"Bad argument for atan" },
	{ 10268,	"E_ACOT_1",		"Bad epsilon for acot" },
	{ 10269,	"E_ACOT_2",		"Bad argument for acot" },
	{ 10270,	"E_ASEC_1",		"Bad epsilon for asec" },
	{ 10271,	"E_ASEC_2",		"Bad argument for asec" },
	{ 10272,	"E_ACSC_1",		"Bad epsilon for acsc" },
	{ 10273,	"E_ACSC_2",		"Bad argument for acsc" },
	{ 10274,	"E_ASINH_1",		"Bad epsilon for asin" },
	{ 10275,	"E_ASINH_2",		"Bad argument for asinh" },
	{ 10276,	"E_ACOSH_1",		"Bad epsilon for acosh" },
	{ 10277,	"E_ACOSH_2",		"Bad argument for acosh" },
	{ 10278,	"E_ATANH_1",		"Bad epsilon for atanh" },
	{ 10279,	"E_ATANH_2",		"Bad argument for atanh" },
	{ 10280,	"E_ACOTH_1",		"Bad epsilon for acoth" },
	{ 10281,	"E_ACOTH_2",		"Bad argument for acoth" },
	{ 10282,	"E_ASECH_1",		"UNUSED ERROR: Bad epsilon for asech" },
	{ 10283,	"E_ASECH_2",		"Bad argument for asech" },
	{ 10284,	"E_ACSCH_1",		"Bad epsilon for acsch" },
	{ 10285,	"E_ACSCH_2",		"Bad argument for acsch" },
	{ 10286,	"E_GD_1",		"Bad epsilon for gd" },
	{ 10287,	"E_GD_2",		"Bad argument for gd" },
	{ 10288,	"E_AGD_1",		"Bad epsilon for agd" },
	{ 10289,	"E_AGD_2",		"Bad argument for agd" },
	{ 10290,	"E_LOGINF",		"Log of zero or infinity" },
	{ 10291,	"E_STRADD",		"String addition failure" },
	{ 10292,	"E_STRMUL",		"String multiplication failure" },
	{ 10293,	"E_STRNEG",		"String reversal failure" },
	{ 10294,	"E_STRSUB",		"String subtraction failure" },
	{ 10295,	"E_BIT_1",		"Bad argument type for bit" },
	{ 10296,	"E_BIT_2",		"Index too large for bit" },
	{ 10297,	"E_SETBIT_1",		"Non-integer second argument for setbit" },
	{ 10298,	"E_SETBIT_2",		"Out-of-range index for setbit" },
	{ 10299,	"E_SETBIT_3",		"Non-string first argument for setbit" },
	{ 10300,	"E_OR",			"Bad argument for or" },
	{ 10301,	"E_AND",		"Bad argument for and" },
	{ 10302,	"E_STROR",		"Allocation failure for string or" },
	{ 10303,	"E_STRAND",		"Allocation failure for string and" },
	{ 10304,	"E_XOR",		"Bad argument for xorvalue" },
	{ 10305,	"E_COMP",		"Bad argument for comp" },
	{ 10306,	"E_STRDIFF",		"Allocation failure for string diff" },
	{ 10307,	"E_STRCOMP",		"Allocation failure for string comp" },
	{ 10308,	"E_SEG_1",		"Bad first argument for segment" },
	{ 10309,	"E_SEG_2",		"Bad second argument for segment" },
	{ 10310,	"E_SEG_3",		"Bad third argument for segment" },
	{ 10311,	"E_STRSEG",		"Failure for string segment" },
	{ 10312,	"E_HIGHBIT_1",		"Bad argument type for highbit" },
	{ 10313,	"E_HIGHBIT_2",		"Non-integer argument for highbit" },
	{ 10314,	"E_LOWBIT_1",		"Bad argument type for lowbit" },
	{ 10315,	"E_LOWBIT_2",		"Non-integer argument for lowbit" },
	{ 10316,	"E_CONTENT",		"Bad argument type for unary hash op" },
	{ 10317,	"E_HASHOP",		"Bad argument type for binary hash op" },
	{ 10318,	"E_HEAD_1",		"Bad first argument for head" },
	{ 10319,	"E_HEAD_2",		"Bad second argument for head" },
	{ 10320,	"E_STRHEAD",		"Failure for strhead" },
	{ 10321,	"E_TAIL_1",		"Bad first argument for tail" },
	{ 10322,	"E_TAIL_2",		"UNUSED ERROR: Bad second argument for tail" },
	{ 10323,	"E_STRTAIL",		"Failure for strtail" },
	{ 10324,	"E_STRSHIFT",		"Failure for strshift" },
	{ 10325,	"E_STRCMP",		"Non-string argument for strcmp" },
	{ 10326,	"E_STRNCMP",		"Bad argument type for strncmp" },
	{ 10327,	"E_XOR_1",		"Varying types of argument for xor" },
	{ 10328,	"E_XOR_2",		"Bad argument type for xor" },
	{ 10329,	"E_STRCPY",		"Bad argument type for strcpy" },
	{ 10330,	"E_STRNCPY",		"Bad argument type for strncpy" },
	{ 10331,	"E_BACKSLASH",		"Bad argument type for unary backslash" },
	{ 10332,	"E_SETMINUS",		"Bad argument type for setminus" },
	{ 10333,	"E_INDICES_1",		"Bad first argument type for indices" },
	{ 10334,	"E_INDICES_2",		"Bad second argument for indices" },
	{ 10335,	"E_EXP_3",		"Too-large re(argument) for exp" },
	{ 10336,	"E_SINH_3",		"Too-large re(argument) for sinh" },
	{ 10337,	"E_COSH_3",		"Too-large re(argument) for cosh" },
	{ 10338,	"E_SIN_3",		"Too-large im(argument) for sin" },
	{ 10339,	"E_COS_3",		"Too-large im(argument) for cos" },
	{ 10340,	"E_GD_3",		"Infinite or too-large result for gd" },
	{ 10341,	"E_AGD_3",		"Infinite or too-large result for agd" },
	{ 10342,	"E_POWER_4",		"Too-large value for power" },
	{ 10343,	"E_ROOT_4",		"Too-large value for root" },
	{ 10344,	"E_DGT_1",		"Non-real first arg for digit" },
	{ 10345,	"E_DGT_2",		"Non-integral second arg for digit" },
	{ 10346,	"E_DGT_3",		"Bad third arg for digit" },
	{ 10347,	"E_PLCS_1",		"Bad first argument for places" },
	{ 10348,	"E_PLCS_2",		"Bad second argument for places" },
	{ 10349,	"E_DGTS_1",		"Bad first argument for digits" },
	{ 10350,	"E_DGTS_2",		"Bad second argument for digits" },
	{ 10351,	"E_ILOG",		"Bad first argument for ilog" },
	{ 10352,	"E_ILOGB",		"Bad second argument for ilog" },
	{ 10353,	"E_IBASE10_LOG",	"Bad argument for ilog10" },
	{ 10354,	"E_IBASE2_LOG",		"Bad argument for ilog2" },
	{ 10355,	"E_COMB_1",		"Non-integer second arg for comb" },
	{ 10356,	"E_COMB_2",		"Too-large second arg for comb" },
	{ 10357,	"E_CTLN",		"Bad argument for catalan" },
	{ 10358,	"E_BERN",		"Bad argument for bern" },
	{ 10359,	"E_EULER",		"Bad argument for euler" },
	{ 10360,	"E_SLEEP",		"Bad argument for sleep" },
	{ 10361,	"E_TTY",		"calc_tty failure" },
	{ 10362,	"E_ASSIGN_1",		"No-copy-to destination for octet assign" },
	{ 10363,	"E_ASSIGN_2",		"No-copy-from source for octet assign" },
	{ 10364,	"E_ASSIGN_3",		"No-change destination for octet assign" },
	{ 10365,	"E_ASSIGN_4",		"Non-variable destination for assign" },
	{ 10366,	"E_ASSIGN_5",		"No-assign-to destination for assign" },
	{ 10367,	"E_ASSIGN_6",		"No-assign-from source for assign" },
	{ 10368,	"E_ASSIGN_7",		"No-change destination for assign" },
	{ 10369,	"E_ASSIGN_8",		"No-type-change destination for assign" },
	{ 10370,	"E_ASSIGN_9",		"No-error-value destination for assign" },
	{ 10371,	"E_SWAP_1",		"No-copy argument for octet swap" },
	{ 10372,	"E_SWAP_2",		"No-assign-to-or-from argument for swap" },
	{ 10373,	"E_SWAP_3",		"Non-lvalue argument for swap" },
	{ 10374,	"E_QUOMOD_1",		"Non-lvalue argument 3 or 4 for quomod" },
	{ 10375,	"E_QUOMOD_2",		"Non-real-number arg 1 or 2 or bad arg 5 for quomod" },
	{ 10376,	"E_QUOMOD_3",		"No-assign-to argument 3 or 4 for quomod" },
	{ 10377,	"E_PREINC_1",		"No-copy-to or no-change argument for octet preinc" },
	{ 10378,	"E_PREINC_2",		"Non-variable argument for preinc" },
	{ 10379,	"E_PREINC_3",		"No-assign-to or no-change argument for preinc" },
	{ 10380,	"E_PREDEC_1",		"No-copy-to or no-change argument for octet predec" },
	{ 10381,	"E_PREDEC_2",		"Non-variable argument for predec" },
	{ 10382,	"E_PREDEC_3",		"No-assign-to or no-change argument for predec" },
	{ 10383,	"E_POSTINC_1",		"No-copy-to or no-change argument for octet postinc" },
	{ 10384,	"E_POSTINC_2",		"Non-variable argument for postinc" },
	{ 10385,	"E_POSTINC_3",		"No-assign-to or no-change argument for postinc" },
	{ 10386,	"E_POSTDEC_1",		"No-copy-to or no-change argument for octet postdec" },
	{ 10387,	"E_POSTDEC_2",		"Non-variable argument for postdec" },
	{ 10388,	"E_POSTDEC_3",		"No-assign-to or no-change argument for postdec" },
	{ 10389,	"E_INIT_01",		"Error-type structure for initialization" },
	{ 10390,	"E_INIT_02",		"No-copy-to structure for initialization" },
	{ 10391,	"E_INIT_03",		"Too many initializer values" },
	{ 10392,	"E_INIT_04",		"Attempt to initialize freed named block" },
	{ 10393,	"E_INIT_05",		"Bad structure type for initialization" },
	{ 10394,	"E_INIT_06",		"No-assign-to element for initialization" },
	{ 10395,	"E_INIT_07",		"No-change element for initialization" },
	{ 10396,	"E_INIT_08",		"No-type-change element for initialization" },
	{ 10397,	"E_INIT_09",		"No-error-value element for initialization" },
	{ 10398,	"E_INIT_10",		"No-assign-or-copy-from source for initialization" },
	{ 10399,	"E_LIST_1",		"No-relocate for list insert" },
	{ 10400,	"E_LIST_2",		"No-relocate for list delete" },
	{ 10401,	"E_LIST_3",		"No-relocate for list push" },
	{ 10402,	"E_LIST_4",		"No-relocate for list append" },
	{ 10403,	"E_LIST_5",		"No-relocate for list pop" },
	{ 10404,	"E_LIST_6",		"No-relocate for list remove" },
	{ 10405,	"E_MODIFY_1",		"Non-variable first argument for modify" },
	{ 10406,	"E_MODIFY_2",		"Non-string second argument for modify" },
	{ 10407,	"E_MODIFY_3",		"No-change first argument for modify" },
	{ 10408,	"E_MODIFY_4",		"Undefined function for modify" },
	{ 10409,	"E_MODIFY_5",		"Unacceptable type first argument for modify" },
	{ 10410,	"E_FPATHOPEN_1",	"Non-string arguments for fpathopen" },
	{ 10411,	"E_FPATHOPEN_2",	"Unrecognized mode for fpathopen" },
	{ 10412,	"E_LOG_1",		"Bad epsilon argument for log" },
	{ 10413,	"E_LOG_2",		"Non-numeric first argument for log" },
	{ 10414,	"E_LOG_3",		"Cannot calculate log for this value" },
	{ 10415,	"E_FGETFILE_1",		"Non-file argument for fgetfile" },
	{ 10416,	"E_FGETFILE_2",		"File argument for fgetfile not open for reading" },
	{ 10417,	"E_FGETFILE_3",		"Unable to set file position in fgetfile" },
	{ 10418,	"E_ESTR",		"UNUSED ERROR: Non-representable type for estr" },
	{ 10419,	"E_STRCASECMP",		"Non-string argument for strcasecmp" },
	{ 10420,	"E_STRNCASECMP",	"Bad argument type for strncasecmp" },
	{ 10421,	"E_ISUPPER",		"Bad argument for isupper" },
	{ 10422,	"E_ISLOWER",		"Bad argument for islower" },
	{ 10423,	"E_ISALNUM",		"Bad argument for isalnum" },
	{ 10424,	"E_ISALPHA",		"Bad argument for isalpha" },
	{ 10425,	"E_ISASCII",		"Bad argument for isascii" },
	{ 10426,	"E_ISCNTRL",		"Bad argument for iscntrl" },
	{ 10427,	"E_ISDIGIT",		"Bad argument for isdigit" },
	{ 10428,	"E_ISGRAPH",		"Bad argument for isgraph" },
	{ 10429,	"E_ISPRINT",		"Bad argument for isprint" },
	{ 10430,	"E_ISPUNCT",		"Bad argument for ispunct" },
	{ 10431,	"E_ISSPACE",		"Bad argument for isspace" },
	{ 10432,	"E_ISXDIGIT",		"Bad argument for isxdigit" },
	{ 10433,	"E_STRTOUPPER",		"Bad argument type for strtoupper" },
	{ 10434,	"E_STRTOLOWER",		"Bad argument type for strtolower" },
	{ 10435,	"E_TAN_3",		"UNUSED ERROR: Invalid value for calculating the sin numerator for tan" },
	{ 10436,	"E_TAN_4",		"UNUSED ERROR: Invalid value for calculating the cos denominator for tan" },
	{ 10437,	"E_COT_3",		"UNUSED ERROR: Invalid value for calculating the sin numerator for cot" },
	{ 10438,	"E_COT_4",		"UNUSED ERROR: Invalid value for calculating the cos denominator for cot" },
	{ 10439,	"E_SEC_3",		"UNUSED ERROR: Invalid value for calculating the cos reciprocal for sec" },
	{ 10440,	"E_CSC_3",		"UNUSED ERROR: Invalid value for calculating the sin reciprocal for csc" },
	{ 10441,	"E_TANH_3",		"Invalid value for calculating the sinh numerator for tanh" },
	{ 10442,	"E_TANH_4",		"Invalid value for calculating the cosh denominator for tanh" },
	{ 10443,	"E_COTH_3",		"Invalid value for calculating the sinh numerator for coth" },
	{ 10444,	"E_COTH_4",		"Invalid value for calculating the cosh denominator for coth" },
	{ 10445,	"E_SECH_3",		"Invalid value for calculating the cosh reciprocal for sech" },
	{ 10446,	"E_CSCH_3",		"Invalid value for calculating the sinh reciprocal for csch" },
	{ 10447,	"E_ASIN_3",		"Invalid value for calculating asin" },
	{ 10448,	"E_ACOS_3",		"Invalid value for calculating acos" },
	{ 10449,	"E_ASINH_3",		"Invalid value for calculating asinh" },
	{ 10450,	"E_ACOSH_3",		"Invalid value for calculating acosn" },
	{ 10451,	"E_ATAN_3",		"Invalid value for calculating atan" },
	{ 10452,	"E_ACOT_3",		"Invalid value for calculating acot" },
	{ 10453,	"E_ASEC_3",		"Invalid value for calculating asec" },
	{ 10454,	"E_ACSC_3",		"Invalid value for calculating acsc" },
	{ 10455,	"E_ATANH_3",		"Invalid value for calculating atan" },
	{ 10456,	"E_ACOTH_3",		"Invalid value for calculating acot" },
	{ 10457,	"E_ASECH_3",		"Invalid value for calculating asec" },
	{ 10458,	"E_ACSCH_3",		"Invalid value for calculating acsc" },
	{ 10459,	"E_D2R_1",		"Bad epsilon for converting degrees to radians" },
	{ 10460,	"E_D2R_2",		"Bad first argument converting degrees to radians" },
	{ 10461,	"E_R2D_1",		"Bad epsilon for converting radians to degrees" },
	{ 10462,	"E_R2D_2",		"Bad first argument converting radians to degrees" },
	{ 10463,	"E_G2R_1",		"Bad epsilon for converting gradians to radians" },
	{ 10464,	"E_G2R_2",		"Bad first argument converting gradians to radians" },
	{ 10465,	"E_R2G_1",		"Bad epsilon for converting radians to gradians" },
	{ 10466,	"E_R2G_2",		"Bad first argument converting radians to gradians" },
	{ 10467,	"E_D2G_1",		"Bad first argument converting degrees to gradians" },
	{ 10468,	"E_G2D_1",		"Bad first argument converting gradians to degrees" },
	{ 10469,	"E_D2DMS_1",		"Non-lvalue arguments 2, 3 or 4 for d2dms" },
	{ 10470,	"E_D2DMS_2",		"Non-real-number arg 1 for d2dms" },
	{ 10471,	"E_D2DMS_3",		"No-assign-to argument 2, 3 or 4 for d2dms" },
	{ 10472,	"E_D2DMS_4",		"Invalid rounding arg 5 for d2dms" },
	{ 10473,	"E_D2DM_1",		"Non-lvalue arguments 2 or 3 for d2dm" },
	{ 10474,	"E_D2DM_2",		"Non-real-number arg 1 for d2dm" },
	{ 10475,	"E_D2DM_3",		"No-assign-to argument 2 or 3 for d2dm" },
	{ 10476,	"E_D2DM_4",		"Invalid rounding arg 4 for d2dm" },
	{ 10477,	"E_G2GMS_1",		"Non-lvalue arguments 2, 3 or 4 for g2gms" },
	{ 10478,	"E_G2GMS_2",		"Non-real-number arg 1 for g2gms" },
	{ 10479,	"E_G2GMS_3",		"No-assign-to argument 2 or 3 for g2gms" },
	{ 10480,	"E_G2GMS_4",		"Invalid rounding arg 5 for g2gms" },
	{ 10481,	"E_G2GM_1",		"Non-lvalue arguments 2 or 3 for g2gm" },
	{ 10482,	"E_G2GM_2",		"Non-real-number arg 1 for g2gm" },
	{ 10483,	"E_G2GM_3",		"No-assign-to argument 2, 3 or 4 for g2gm" },
	{ 10484,	"E_G2GM_4",		"Invalid rounding arg 4 for g2gm" },
	{ 10485,	"E_H2HMS_1",		"Non-lvalue arguments 2, 3 or 4 for h2hms" },
	{ 10486,	"E_H2HMS_2",		"Non-real-number arg 1 for h2hms" },
	{ 10487,	"E_H2HMS_3",		"No-assign-to argument 2, 3 or 4 for h2hms" },
	{ 10488,	"E_H2HMS_4",		"Invalid rounding arg 5 for h2hms" },
	{ 10489,	"E_H2HM_1",		"Non-lvalue arguments 2 or 3 for h2hm" },
	{ 10490,	"E_H2HM_2",		"Non-real-number arg 1 for h2hm" },
	{ 10491,	"E_H2HM_3",		"No-assign-to argument 2 or 3 for h2hm" },
	{ 10492,	"E_H2HM_4",		"Invalid rounding arg 4 for h2hm" },
	{ 10493,	"E_DMS2D_1",		"Non-real-number arguments 1, 2 or 3 for dms2d" },
	{ 10494,	"E_DMS2D_2",		"Invalid rounding arg 4 for dms2d" },
	{ 10495,	"E_DM2D_1",		"Non-real-number arguments 1 or 2 for dm2d" },
	{ 10496,	"E_DM2D_2",		"Invalid rounding arg 4 for dm2d" },
	{ 10497,	"E_GMS2G_1",		"Non-real-number arguments 1, 2 or 3 for gms2g" },
	{ 10498,	"E_GMS2G_2",		"Invalid rounding arg 4 for gms2g" },
	{ 10499,	"E_GM2G_1",		"Non-real-number arguments 1 or 2 for gm2g" },
	{ 10500,	"E_GM2G_2",		"Invalid rounding arg 4 for gm2g" },
	{ 10501,	"E_HMS2H_1",		"Non-real-number arguments 1, 2 or 3 for hms2h" },
	{ 10502,	"E_HMS2H_2",		"Invalid rounding arg 4 for hms2h" },
	{ 10503,	"E_HM2H_1",		"UNUSED ERROR: Non-real-number arguments 1 or 2 for hm2h" },
	{ 10504,	"E_HM2H_2",		"UNUSED ERROR: Invalid rounding arg 4 for hm2h" },
	{ 10505,	"E_LOG2_1",		"Bad epsilon argument for log2" },
	{ 10506,	"E_LOG2_2",		"Non-numeric first argument for log2" },
	{ 10507,	"E_LOG2_3",		"Cannot calculate log2 for this value" },
	{ 10508,	"E_LOGN_1",		"Bad epsilon argument for logn" },
	{ 10509,	"E_LOGN_2",		"Non-numeric first argument for logn" },
	{ 10510,	"E_LOGN_3",		"Cannot calculate logn for this value" },
	{ 10511,	"E_LOGN_4",		"Cannot calculate logn for this log base" },
	{ 10512,	"E_LOGN_5",		"Non-numeric second argument for logn" },
	{ 10513,	"E_VERSIN_1",		"Bad epsilon for versin" },
	{ 10514,	"E_VERSIN_2",		"Bad first argument for versin" },
	{ 10515,	"E_VERSIN_3",		"Too-large im(argument) for versin" },
	{ 10516,	"E_AVERSIN_1",		"Bad epsilon for aversin" },
	{ 10517,	"E_AVERSIN_2",		"Bad first argument for aversin" },
	{ 10518,	"E_AVERSIN_3",		"Too-large im(argument) for aversin" },
	{ 10519,	"E_COVERSIN_1",		"Bad epsilon for coversin" },
	{ 10520,	"E_COVERSIN_2",		"Bad first argument for coversin" },
	{ 10521,	"E_COVERSIN_3",		"Too-large im(argument) for coversin" },
	{ 10522,	"E_ACOVERSIN_1",	"Bad epsilon for acoversin" },
	{ 10523,	"E_ACOVERSIN_2",	"Bad first argument for acoversin" },
	{ 10524,	"E_ACOVERSIN_3",	"Too-large im(argument) for acoversin" },
	{ 10525,	"E_VERCOS_1",		"Bad epsilon for vercos" },
	{ 10526,	"E_VERCOS_2",		"Bad first argument for vercos" },
	{ 10527,	"E_VERCOS_3",		"Too-large im(argument) for vercos" },
	{ 10528,	"E_AVERCOS_1",		"Bad epsilon for avercos" },
	{ 10529,	"E_AVERCOS_2",		"Bad first argument for avercos" },
	{ 10530,	"E_AVERCOS_3",		"Too-large im(argument) for avercos" },
	{ 10531,	"E_COVERCOS_1",		"Bad epsilon for covercos" },
	{ 10532,	"E_COVERCOS_2",		"Bad first argument for covercos" },
	{ 10533,	"E_COVERCOS_3",		"Too-large im(argument) for covercos" },
	{ 10534,	"E_ACOVERCOS_1",	"Bad epsilon for acovercos" },
	{ 10535,	"E_ACOVERCOS_2",	"Bad first argument for acovercos" },
	{ 10536,	"E_ACOVERCOS_3",	"Too-large im(argument) for acovercos" },
	{ 10537,	"E_TAN_5",		"Invalid complex argument for tan" },
	{ 10538,	"E_COT_5",		"Invalid zero argument for cot" },
	{ 10539,	"E_COT_6",		"Invalid complex argument for cot" },
	{ 10540,	"E_SEC_5",		"Invalid complex argument for sec" },
	{ 10541,	"E_CSC_5",		"Invalid zero argument for csc" },
	{ 10542,	"E_CSC_6",		"Invalid complex argument for csc" },
	{ 10543,	"E_ERROR_3",		"String argument is not a valid E_STRING for error" },
	{ 10544,	"E_ERROR_4",		"Numeric argument is not an integer for error" },
	{ 10545,	"E_STRERROR_3",		"String argument is not a valid E_STRING for strerror" },
	{ 10546,	"E_STRERROR_4",		"errnum_2_errmsg returned NULL as called from strerror" },
	{ 10547,	"E_STRERROR_5",		"Numeric argument is not an integer for strerror" },
	{ 10548,	"E_ERRNO_1",		"Invalid argument type for errno" },
	{ 10549,	"E_ERRNO_2",		"Numeric argument is outside valid errnum range for errno" },
	{ 10550,	"E_ERRNO_3",		"String argument is not a valid E_STRING for errno" },
	{ 10551,	"E_ERRNO_4",		"Numeric argument is not an integer for errno" },
	{ 10552,	"E_ERRSYM_1",		"Invalid argument type for errsym" },
	{ 10553,	"E_ERRSYM_2",		"Numeric argument is outside valid errnum range for errsym" },
	{ 10554,	"E_ERRSYM_3",		"String argument is not a valid E_STRING for errsym" },
	{ 10555,	"E_ERRSYM_4",		"Numeric argument is not an integer for errsym" },
	{ 10556,	"E_ERRSYM_5",		"Unable to create a valid E_STRING from the errnum for errsym" },
	{ 10557,	"E_HAVERSIN_1",		"Bad epsilon for haversin" },
	{ 10558,	"E_HAVERSIN_2",		"Bad first argument for haversin" },
	{ 10559,	"E_HAVERSIN_3",		"Too-large im(argument) for haversin" },
	{ 10560,	"E_AHAVERSIN_1",	"Bad epsilon for ahaversin" },
	{ 10561,	"E_AHAVERSIN_2",	"Bad first argument for ahaversin" },
	{ 10562,	"E_AHAVERSIN_3",	"Too-large im(argument) for ahaversin" },
	{ 10563,	"E_HACOVERSIN_1",	"Bad epsilon for hacoversin" },
	{ 10564,	"E_HACOVERSIN_2",	"Bad first argument for hacoversin" },
	{ 10565,	"E_HACOVERSIN_3",	"Too-large im(argument) for hacoversin" },
	{ 10566,	"E_AHACOVERSIN_1",	"Bad epsilon for ahacoversin" },
	{ 10567,	"E_AHACOVERSIN_2",	"Bad first argument for achaoversin" },
	{ 10568,	"E_AHACOVERSIN_3",	"Too-large im(argument) for ahacoversin" },
	{ 10569,	"E_HAVERCOS_1",		"Bad epsilon for havercos" },
	{ 10570,	"E_HAVERCOS_2",		"Bad first argument for havercos" },
	{ 10571,	"E_HAVERCOS_3",		"Too-large im(argument) for havercos" },
	{ 10572,	"E_AHAVERCOS_1",	"Bad epsilon for ahavercos" },
	{ 10573,	"E_AHAVERCOS_2",	"Bad first argument for ahavercos" },
	{ 10574,	"E_AHAVERCOS_3",	"Too-large im(argument) for ahavercos" },
	{ 10575,	"E_HACOVERCOS_1",	"Bad epsilon for hacovercos" },
	{ 10576,	"E_HACOVERCOS_2",	"Bad first argument for hacovercos" },
	{ 10577,	"E_HACOVERCOS_3",	"Too-large im(argument) for hacovercos" },
	{ 10578,	"E_AHACOVERCOS_1",	"Bad epsilon for ahacovercos" },
	{ 10579,	"E_AHACOVERCOS_2",	"Bad first argument for achaovercos" },
	{ 10580,	"E_AHACOVERCOS_3",	"Too-large im(argument) for ahacovercos" },
	{ 10581,	"E_EXSEC_1",		"Bad epsilon for exsec" },
	{ 10582,	"E_EXSEC_2",		"Bad argument for exsec" },
	{ 10583,	"E_EXSEC_3",		"Invalid complex argument for exsec" },
	{ 10584,	"E_AEXSEC_1",		"Bad epsilon for aexsec" },
	{ 10585,	"E_AEXSEC_2",		"Bad argument for aexsec" },
	{ 10586,	"E_AEXSEC_3",		"Invalid value for calculating aexsec" },
	{ 10587,	"E_EXCSC_1",		"Bad epsilon for excsc" },
	{ 10588,	"E_EXCSC_2",		"Bad argument for excsc" },
	{ 10589,	"E_EXCSC_3",		"Invalid zero argument for excsc" },
	{ 10590,	"E_EXCSC_4",		"Invalid complex argument for excsc" },
	{ 10591,	"E_AEXCSC_1",		"Bad epsilon for aexcsc" },
	{ 10592,	"E_AEXCSC_2",		"Bad argument for aexcsc" },
	{ 10593,	"E_AEXCSC_3",		"Invalid value for calculating aexcsc" },
	/* IMPORTANT NOTE: add new entries above here and be sure their errnum numeric value is consecutive! */

	/* The next NULL entry must be last */
	{ NULL_ERRNUM,	NULL,			NULL }
};


/*
 * The error_table[] array represents the special "E__" E_STRING related
 * error codes, symbols and messages.
 *
 * The purpose of this internal table is to allow functions below to convert
 * between E__" E_STRING related errsym, and their respective errnum
 * calc computation error codes and their respective errmsg messages.
 *
 * In this table, the errnum values must starting with 0 and ascending until
 * the final NULL entry's NULL_ERRNUM (-1) value.  Unlike error_table[],
 * the errnum values in private_error_alias[] do not need to be consecutive.
 *
 * In this table the E_STRING related errsym must match the regular expression:
 *
 *	^E__[A-Z][A-Z0-9_]+$
 *
 * IMPORTANT: DO NOT ADD NEW calc computation TO THIS TABLE!!!
 *	      Add new calc computation to the error_table[] above.
 */
STATIC struct errtbl private_error_alias[] = {

	/* The E__NONE entry below must start with E__NONE (0) and must be first!! */
	{ E__NONE,	"E__NONE",		"calc_errno cleared: libc errno codes above here" },
	{ E__BASE,	"E__BASE",		"calc computation error codes start above here" },
	/* In the next E__HIGHEST entry, errnum and errmsg are changed when verify_error_table() is called */
	{ INV_ERRNUM,	"E__HIGHEST",		"replace this with the E__HIGHEST errmsg" },
	{ E__USERDEF,	"E__USERDEF",		"user defined error codes start here" },
	{ E__USERMAX,	"E__USERMAX",		"maximum user defined error code" },

	/* The next NULL entry must be last */
	{ NULL_ERRNUM,	NULL,			NULL }
};


/*
 * external values
 */
EXTERN char *program;		/* our name */


/*
 * is_e_digits - E_STRING is of the form E_ followed by digits
 *
 * This function determines of errsym matches the regular expression:
 *
 *	^E_[0-9][0-9]*$
 *
 * There is NO attempt to determine if the number type of digits
 * that follow the E_ forms a valid numeric E_STRING.
 *
 * NOTE: See also e_digits_2_errnum() below.
 *
 * given:
 *	errsym	E_STRING to check
 *
 * returns:
 *	true ==> E_STRING is of the form E_ followed by digits
 *	false ==> E_STRING is NULL and/or not E_ followed by digits
 */
bool
is_e_digits(CONST char *errsym)
{
	CONST char *p;

	/*
	 * firewall
	 */
	if (errsym == NULL) {
		return false;
	}

	/*
	 * must start with E_
	 */
	if (strncmp(errsym, "E_", 2) != 0) {
		return false;
	}

	/*
	 * length must be >= 3
	 */
	if (errsym[2] == '\0') {
		return false;
	}

	/*
	 * check for only digits remaining
	 */
	for (p = errsym+2; *p != '\0'; ++p) {
		if (!isascii(*p) || !isdigit(*p)) {
			return false;
		}
	}

	/*
	 * errsym matches: ^E_[0-9][0-9]*$
	 */
	return true;
}


/*
 * is_valid_errnum - determine if errnum is within the proper range
 *
 * A valid errnum must be:
 *
 *	E__NONE <= errnum <= E__USERMAX
 *
 * NOTE: This functions does NOT check of the errnum is in
 * some struct errtbl array.  For that see find_errsym_in_errtbl().
 *
 * given:
 *	errnum	errnum to check
 *
 * returns:
 *	true ==> errnum is valid
 *	false ==> errnum is NOT valid
 */
bool
is_valid_errnum(int errnum)
{
	/*
	 * check for range
	 */
	if (errnum < E__NONE) {
		return false;
	}
	if (errnum > E__USERMAX) {
		return false;
	}
	/* errnum is valid */
	return true;
}


/*
 * is_errnum_in_error_table - determine if errnum falls within the error_table[] array bounds
 *
 * This function determines if the errnum is a value that is in the range
 * that the error_table[] array holds.  This function does NOT perform a
 * lookup within the error_table[] array, only a errnum value range check.
 *
 * returns:
 *	true ==> errnum falls within the error_table[] array bounds
 *	false ==> errnum does NOT fall within the error_table[] array bounds
 */
bool
is_errnum_in_error_table(int errnum)
{
	/*
	 * firewall - errnum must be valid
	 */
	if (is_valid_errnum(errnum) == false) {
		/* errnum is not a valid value */
		return false;
	}

	/*
	 * case: errnum is too low for error_table[] array
	 */
	if (errnum < E__BASE) {
		/* errnum is too low */
		return false;
	}

	/*
	 * case: errnum is too high for error_table[] array
	 */
	if ((unsigned long)errnum > MY_E__HIGHEST) {
		/* errnum is too high */
		return false;
	}

	/*
	 * return error_table[] array matching the errnum value
	 */
	return true;
}


/*
 * e_digits_2_errnum - convert an E_STRING of digits into a valid errnum
 *
 * Given an E_STRING errsym matching the regular expression:
 *
 *	^E_[0-9][0-9]*$
 *
 * this function attempts to convert those digits into an errnum, and
 * if successful, that the resulting errnum is valid:
 *
 *	E__NONE <= errnum <= E__USERMAX
 *
 * NOTE: Leading zeros in digits are not allowed.  So the E_STRING errsym
 *	 must match the regular expression:
 *
 *	^(0|[1-9][0-9]*)$
 *
 * This function calls is_e_digits() first to determine that E_STRING
 * is E_ followed by digits.
 *
 * given:
 *	errsym	E_STRING to convert
 *
 * returns:
 *	== NULL_ERRNUM ==> errsym is NULL, or not E_STRING of digits, or
 *			   the resulting errnum would be invald
 *	!= NULL_ERRNUM == valid errnum corresponding to errsym
 */
int
e_digits_2_errnum(CONST char *errsym)
{
	long errnum;		/* digits converted into a possible errnum */

	/*
	 * firewall
	 */
	if (errsym == NULL) {
		/* errsym is NULL */
		return NULL_ERRNUM;
	}

	/*
	 * must be an E_STRING of digits
	 */
	if (is_e_digits(errsym) == false) {
		/* errsym doesn't match: ^E_[0-9][0-9]*$ */
		return NULL_ERRNUM;
	}

	/*
	 * case: for E_0, return E__NONE
	 */
	if (strcmp(errsym, "E_0") == 0) {
		/* quick return for E_0 */
		return E__NONE;
	}

	/*
	 * errnum is not E_0, digits cannot start with "0"
	 */
	if (errsym[2] == '0') {
		/* digits cannot start with 0 unless it is E_0 */
		return NULL_ERRNUM;
	}

	/*
	 * try to convert digits into long
	 */
	errno = 0;
	errnum = strtol(errsym+2, NULL, 10);
	if (errno != 0) {
		/* failed to convert digits in base 10 */
		return NULL_ERRNUM;
	}

	/*
	 * errnum, to be valid, just be in the range [E__NONE,E__USERMAX]
	 */
	if (is_valid_errnum(errnum) == false) {
		/* errnum is out of range */
		return NULL_ERRNUM;
	}

	/*
	 * return converted errnum
	 */
	return errnum;
}


/*
 * is_e_1string - E_STRING is of the form E_ followed by letters and digits and underscores
 *
 * This function determines of errsym matches the regular expression:
 *
 *	^E_[A-Z][A-Z0-9_]+$
 *
 * This function will not match the special E_STRING is of the form E__ followed by
 * letters and digits and underscores.  For that see is_e_2string().
 *
 * given:
 *	errsym	E_STRING to check
 *
 * returns:
 *	true ==> E_STRING matches the regular expression: ^E_[A-Z][A-Z0-9_]+$
 *	false ==> E_STRING is NULL and/or does not match expression: ^E_[A-Z][A-Z0-9_]+$
 */
bool
is_e_1string(CONST char *errsym)
{
	CONST char *p;

	/*
	 * firewall
	 */
	if (errsym == NULL) {
		return false;
	}

	/*
	 * must start with E_
	 */
	if (strncmp(errsym, "E_", 2) != 0) {
		return false;
	}

	/*
	 * length must be >= 3
	 */
	if (errsym[2] == '\0') {
		return false;
	}

	/*
	 * errsym, after E_, must be an UPPER case letter
	 */
	if (!isascii(errsym[2]) || !isupper(errsym[2])) {
		return false;
	}

	/*
	 * must match regular expression: ^E_[A-Z][A-Z0-9_]+$
	 */
	for (p = errsym+3; *p != '\0'; ++p) {
		if (!isascii(*p) || !(isupper(*p) || isdigit(*p) || *p == '_')) {
			return false;
		}
	}

	/*
	 * errsym matches: ^E_[A-Z][A-Z0-9_]+$
	 */
	return true;
}


/*
 * is_e_2string - E_STRING is of the form E__ followed by letters and digits and underscores
 *
 * This function determines of errsym matches the regular expression:
 *
 *	^E__[A-Z][A-Z0-9_]+$
 *
 * This function will not match the special E_STRING is of the form E_ followed by
 * letters and digits and underscores.  For that see is_e_1string().
 *
 * given:
 *	errsym	E_STRING to check
 *
 * returns:
 *	true ==> E_STRING matches the regular expression: ^E__[A-Z][A-Z0-9_]+$
 *	false ==> E_STRING is NULL and/or does not match expression: ^E__[A-Z][A-Z0-9_]+$
 */
bool
is_e_2string(CONST char *errsym)
{
	CONST char *p;

	/*
	 * firewall
	 */
	if (errsym == NULL) {
		return false;
	}

	/*
	 * must start with E_
	 */
	if (strncmp(errsym, "E__", 3) != 0) {
		return false;
	}

	/*
	 * length must be >= 4
	 */
	if (errsym[3] == '\0') {
		return false;
	}

	/*
	 * errsym, after E__, must be an UPPER case letter
	 */
	if (!isascii(errsym[3]) || !isupper(errsym[3])) {
		return false;
	}

	/*
	 * must match regular expression: ^E__[A-Z][A-Z0-9_]+$
	 */
	for (p = errsym+4; *p != '\0'; ++p) {
		if (!isascii(*p) || !(isupper(*p) || isdigit(*p) || *p == '_')) {
			return false;
		}
	}

	/*
	 * errsym matches: ^E__[A-Z][A-Z0-9_]+$
	 */
	return true;
}


/*
 * find_errsym_in_errtbl - given an E_STRING find it in a struct errtbl array
 *
 * given:
 *	errsym	E_STRING to check
 *	tbl	pointer to a NULL an array of struct errtbl with a final NULL entry
 *
 * returns:
 *	!= NULL ==> pointer to entry in tbl struct errtbl entry with matching errsym
 *	NULL ==> NULL arg, or errsym not found
 */
struct errtbl *
find_errsym_in_errtbl(CONST char *errsym, CONST struct errtbl *tbl)
{
	CONST struct errtbl *ret;	/* pointer to struct errtbl entry with matching errsym */

	/*
	 * firewall
	 */
	if (errsym == NULL || tbl == NULL) {
		/* invalid NULL arg(s) */
		return NULL;
	}

	/*
	 * scan tbl until final NULL entry
	 */
	for (ret = tbl; ret->errsym != NULL; ++ret) {
		if (strcmp(ret->errsym, errsym) == 0) {
			/* found matching errsym */
			return (struct errtbl *)ret;
		}
	}
	/* no match */
	return NULL;
}


/*
 * find_errnum_in_errtbl - given a valid errnum, search for it in a struct errtbl array
 *
 * This function assumes that the errnum values in a struct errtbl array are
 * both valid errnum and ascending.  The function verify_error_table() will verify that
 * this is the case for both error_table[] and private_error_alias[].
 *
 * See also lookup_errnum_in_error_table() for a faster way to do:
 *
 *	find_errnum_in_errtbl(errnum, error_table); (* use lookup_errnum_in_error_table() instead *)
 *
 * given:
 *	errnum	the errnum code to seach for
 *	tbl	pointer to a NULL an array of struct errtbl with a final NULL entry
 *
 * returns:
 *	!= NULL ==> pointer to entry in tbl struct errtbl entry with matching
 *	NULL ==> NULL arg, or matching errnum not foun
 */
struct errtbl *
find_errnum_in_errtbl(int errnum, CONST struct errtbl *tbl)
{
	CONST struct errtbl *ret;	/* pointer to struct errtbl entry with matching errsym */

	/*
	 * firewall
	 */
	if (tbl == NULL) {
		/* invalid NULL arg */
		return NULL;
	}
	if (is_valid_errnum(errnum) == false) {
		/* invalid errnum */
		return NULL;
	}

	/*
	 * scan tbl until final NULL entry
	 */
	for (ret = tbl; ret->errsym != NULL; ++ret) {

		/* case: found errnum match */
		if (ret->errnum == errnum) {
			/* found matching errnum */
			return (struct errtbl *)ret;
		}

		/* case: we passed errnum in struct errtbl array */
		if (ret->errnum > errnum) {
			/* we have searched beyond errnum: everthing else is > errnum */
			return NULL;
		}
	}
	/* no match */
	return NULL;
}


/*
 * lookup_errnum_in_error_table - lookup errnum within error_table[] array
 *
 * This function searches for errnum within the error_table[] array
 * by using the fact that the error_table[] array contains a compact
 * sequential set of errnum values from E__BASE thru and including E__HIGHEST.
 *
 * This function performs the equivalent of:
 *
 *	find_errnum_in_errtbl(errnum, error_table); (* use lookup_errnum_in_error_table() instead *)
 *
 * given:
 *	errnum	the errnum code to seach for
 *
 * returns:
 *	!= NULL ==> pointer to entry in error_table[] array with matching errnum
 *	NULL ==> NULL arg, or no matching errnum in error_table[] array
 */
CONST struct errtbl *
lookup_errnum_in_error_table(int errnum)
{
	CONST struct errtbl *ret;	/* pointer to struct errtbl entry with matching errsym */
	int offset;			/* offset within the error_table[] array */

	/*
	 * firewall - errnum must be valid
	 */
	if (is_errnum_in_error_table(errnum) == false) {
		/* errnum is not a valid or not within the error_table[] array range */
		return NULL;
	}

	/*
	 * paranoia
	 */
	if (errnum < E__BASE || errnum > (long)MY_E__HIGHEST) {
		/* errnum is outside of the error_table[] array range */
		return NULL;
	}
	offset = errnum - E__BASE;
	if (offset < 0 || offset > (long)(sizeof(error_table)/sizeof(error_table[0]))) {
		/* offset is outside of the error_table[] array */
		return NULL;
	}

	/*
	 * fetch entry from error_table[] array that should have the errnum value
	 */
	ret = &error_table[offset];
	if (ret->errnum != errnum) {
		/* error_table[] entry has the wrong errnum */
		return NULL;
	}

	/*
	 * return error_table[] array matching the errnum value
	 */
	return ret;
}


/*
 * verify_error_table - verify the consistency of the error_table
 *
 * For error_table[]:
 *
 *	The errnum must start with E__BASE and be consecutive until the final entry.
 *
 *	With the exception of the 1st "E__BASE" entry, the errsym must be a non-NULL string that
 *	matches, up until the last entry, the regular expression:
 *
 *		^E_[A-Z][A-Z0-9_]+$
 *
 *	The errmsg must be a non-NULL non-empty string until the last entry.
 *
 *	The final entry must have an errnum of NULL_ERRNUM (-1), errsym of NULL and errmsg of NULL.
 *
 * For private_error_alias[]:
 *
 *	This function also verifies the private_error_alias[] array.
 *
 *	With the exception of the last entry, the errsym must be a non-NULL string that
 *	matches regular expression:
 *
 *		^E__[A-Z][A-Z0-9_]+$
 *
 *	The errmsg of the E__HIGHEST errsym entry points to the
 *	corresponding error_table[] entry's MY_E__HIGHEST errsym
 *	so that E__HIGHEST becomes an alias for the highest assigned
 *	calc computation error code.
 *
 *	The errmsg must be a non-NULL non-empty string until the last entry.
 *
 *	The final entry must have an errnum of NULL_ERRNUM (-1), errsym of NULL and errmsg of NULL.
 *
 * This function does not return on error or if the error_table is invalid.
 *
 * Use the seqcexit tool to keep the exit codes for this function unique, starting with 10.
 * Apply the following command in this soure code:
 *
 *	seqcexit errtbl.c
 *
 * For more information on the seqcexit tool, see:
 *
 *	https://github.com/lcn2/seqcexit
 */
void
verify_error_table(void)
{
	size_t len;		/* length of the error_table[] */
	size_t e__count;	/* computed ECOUNT value */
	size_t priv_len;	/* length of the private_error_alias[] */
	size_t priv_e__count;	/* computed MY_PRIV_ECOUNT value */
	struct errtbl *found;	/* pointer to E__HIGHEST errsym entry in private_error_alias[] or NULL */
	int prev_errnum;	/* previous errnum found in private_error_alias[] */
	long ten;		/* used to try an compute 10^USERMAX_DIGITS */
	VALUE test;		/* value to test if E__USERMAX fits into a v_type */
	size_t i;

	/*
	 * verify error_table[] size
	 */
	len = sizeof(error_table) / sizeof(error_table[0]);
	e__count = len - 2;
	if (e__count != MY_ECOUNT) {
		fprintf(stderr, "**** %s ERROR: error_table length: %zu != MY_ECOUNT+2: %lu\n",
			program, len, (unsigned long)MY_ECOUNT+2);
		exit(10); /*coo*/
	}

	/*
	 * verify the initial errnum of the 1st error_table[] entry
	 */
	if (error_table[0].errnum != E__BASE) {
		fprintf(stderr, "**** %s ERROR: initial entry error_table[0].errnum: %d must == E__BASE: %d\n",
			program, error_table[0].errnum, E__BASE);
		exit(11);
	}
	if (error_table[0].errsym == NULL) {
		fprintf(stderr, "**** %s ERROR: error_table[0].errsym must != NULL\n",
			program);
		exit(12);
	}
	if (strcmp(error_table[0].errsym, "E__BASE") != 0) {
		fprintf(stderr, "**** %s ERROR: error_table[0].errsym: %s must be E__BASE\n",
			program, error_table[0].errsym);
		exit(13);
	}
	if (error_table[0].errmsg == NULL) {
		fprintf(stderr, "**** %s ERROR: error_table[0].errmsg must != NULL\n",
			program);
		exit(14);
	}
	if (strlen(error_table[0].errmsg) <= 0) {
		fprintf(stderr, "**** %s ERROR: error_table[0].errmsg length: %zu must be > 0\n",
			program, strlen(error_table[0].errmsg));
		exit(15);
	}

	/*
	 * verify 2nd thru end to last entry of error_table[]
	 */
	for (i=1; i < len-1; ++i) {

		/*
		 * check errnum - must be consecutive starting with E__BASE
		 */
		if ((size_t)error_table[i].errnum != E__BASE+i) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errnum: %d != %lu\n",
				program, i, error_table[i].errnum, (unsigned long)E__BASE+i);
			exit(16);
		}

		/*
		 * check if errsym is non-NULL
		 */
		if (error_table[i].errsym == NULL) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu]..errsym must != NULL\n",
				program, i);
			exit(17);
		}

		/*
		 * check if errsym is matches the regular expression: ^E_[A-Z][A-Z0-9_]+$
		 */
		if (is_e_1string(error_table[i].errsym) == false) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errsym: %s "
					"must match the regular expression: %s\n",
				program, i, error_table[i].errsym, "^E_[A-Z][A-Z0-9_]+$");
			exit(18);
		}

		/*
		 * verify error_table[] entry errmsg is not NULL
		 */
		if (error_table[i].errmsg == NULL) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errmsg must != NULL\n",
				program, i);
			exit(19);
		}

		/*
		 * verify error_table[] entry errmsg is not empty
		 */
		if (error_table[i].errmsg[0] == '\0') {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errmsg length: %zu must be > 0\n",
				program, i, strlen(error_table[i].errmsg));
			exit(20);
		}
	}

	/*
	 * verify 2nd to last error_table[] entry has the MY_E__HIGHEST errnum
	 */
	if (error_table[len-2].errnum != MY_E__HIGHEST) {
		fprintf(stderr, "**** %s ERROR: highest assigned calc computation error code entry "
				"error_table[%zu].errnum: %d must == %lu\n",
			program, len-2, error_table[len-2].errnum, (unsigned long)MY_E__HIGHEST);
		exit(21);
	}

	/*
	 * verify the last error_table[] entry
	 */
	if (error_table[len-1].errnum != NULL_ERRNUM) {
		fprintf(stderr, "**** %s ERROR: final NULL entry error_table[%zu].errnum: %d must == %d\n",
			program, len-1, error_table[len-1].errnum, NULL_ERRNUM);
		exit(22);
	}
	if (error_table[len-1].errsym != NULL) {
		fprintf(stderr, "**** %s ERROR: final NULL entry error_table[%zu].errsym must == NULL\n",
			program, len-1);
		exit(23);
	}
	if (error_table[len-1].errmsg != NULL) {
		fprintf(stderr, "**** %s ERROR: final NULL entry error_table[%zu].errmsg must == NULL\n",
			program, len-1);
		exit(24);
	}

	/*
	 * verify private_error_alias[] size
	 */
	priv_len = sizeof(private_error_alias) / sizeof(private_error_alias[0]);
	priv_e__count = priv_len - 1;
	if (priv_e__count != MY_PRIV_ECOUNT) {
		fprintf(stderr, "**** %s ERROR: private_error_alias length: %zu must == MY_PRIV_ECOUNT+1: %lu\n",
			program, priv_len, (unsigned long)MY_PRIV_ECOUNT+1);
		exit(25);
	}

	/*
	 * setup the E__HIGHEST entry in private_error_alias[] to be an alias for
	 * the highest assigned calc computation error code from error_table[].
	 */
	found = find_errsym_in_errtbl("E__HIGHEST", private_error_alias);
	if (found == NULL) {
		fprintf(stderr, "**** %s ERROR: private_error_alias missing E__HIGHEST errsym entry",
			program);
		exit(26);
	}
	found->errnum = error_table[len-2].errnum;
	found->errmsg = error_table[len-2].errmsg;

	/*
	 * verify 1st private_error_alias[] entry == (E__NONE) 0
	 */
	if (private_error_alias[0].errnum != 0) {
		fprintf(stderr, "**** %s ERROR: initial entry error_table[0].errnum: %d myst == 0\n",
			program, error_table[0].errnum);
		exit(27);
	}
	if (private_error_alias[0].errnum != E__NONE) {
		fprintf(stderr, "**** %s ERROR: initial entry error_table[0].errnum: %d must == E__NONE: %d\n",
			program, error_table[0].errnum, E__NONE);
		exit(28);
	}

	/*
	 * verify all but the final private_error_alias[] entry
	 */
	prev_errnum = INV_ERRNUM;	/* preset previous errnum to INV_ERRNUM, and <0 value */
	for (i=0; i < priv_len-1; ++i) {

		/*
		 * The private_error_alias[] entry errnum must be >= 0
		 */
		if (private_error_alias[i].errnum < 0) {
			fprintf(stderr, "**** %s ERROR: entry private_error_alias[%zu].errnum: %d must be >= 0\n",
				program, i, private_error_alias[i].errnum);
			exit(29);
		}
		if (i > 0) {
			if (private_error_alias[i].errnum <= prev_errnum) {
				fprintf(stderr, "**** %s ERROR: entry private_error_alias[%zu].errnum: %d "
						"must be > private_error_alias[%zu].errnum: %d\n",
					program, i, private_error_alias[i].errnum, i-1, prev_errnum);
				exit(30);
			}
		}
		prev_errnum = private_error_alias[i].errnum;

		/*
		 * The private_error_alias[] entry errsym must not be NULL
		 */
		if (private_error_alias[i].errsym == NULL) {
			fprintf(stderr, "**** %s ERROR: entry private_error_alias[%zu].errsym must != NULL\n",
				program, i);
			exit(31);
		}

		/*
		 * check errsym that is string matching the regular expression: ^E__[A-Z][A-Z0-9_]+$
		 */
		if (is_e_2string(private_error_alias[i].errsym) == false) {
			fprintf(stderr, "**** %s ERROR: private_error_alias[%zu].errsym: %s "
					"E_STRING must match the regular expression: %s\n",
				program, i, private_error_alias[i].errsym, "^E__[A-Z][A-Z0-9_]+$");
			exit(32);
		}

		/*
		 * verify private_error_alias[] entry errmsg is not NULL
		 */
		if (private_error_alias[i].errmsg == NULL) {
			fprintf(stderr, "**** %s ERROR: private_error_alias[%zu].errmsg must != NULL\n",
				program, i);
			exit(33);
		}

		/*
		 * verify private_error_alias[] entry errmsg is not empty
		 */
		if (private_error_alias[i].errmsg[0] == '\0') {
			fprintf(stderr, "**** %s ERROR: private_error_alias[%zu].errmsg length: %zu must be > 0\n",
				program, i, strlen(private_error_alias[i].errmsg));
			exit(34);
		}
	}

	/*
	 * verify the last private_error_alias[] entry
	 */
	if (private_error_alias[priv_len-1].errnum != NULL_ERRNUM) {
		fprintf(stderr, "**** %s ERROR: final NULL entry private_error_alias[%zu].errnum: %d must == %d\n",
			program, priv_len-1, private_error_alias[priv_len-1].errnum, NULL_ERRNUM);
		exit(35);
	}
	if (private_error_alias[priv_len-1].errsym != NULL) {
		fprintf(stderr, "**** %s ERROR: final NULL entry private_error_alias[%zu].errsym must == NULL\n",
			program, priv_len-1);
		exit(36);
	}
	if (private_error_alias[priv_len-1].errmsg != NULL) {
		fprintf(stderr, "**** %s ERROR: final NULL entry private_error_alias[%zu].errmsg must == NULL\n",
			program, priv_len-1);
		exit(37);
	}

	/*
	 * verify USERMAX_DIGITS is correct with respect to the size of E__USERMAX
	 */
	if (USERMAX_DIGITS <= 0) {
		fprintf(stderr, "**** %s ERROR: USERMAX_DIGITS: %d must be > 0\n",
			program, USERMAX_DIGITS);
		exit(38);
	}
	if (USERMAX_DIGITS > 20) {
		fprintf(stderr, "**** %s ERROR: USERMAX_DIGITS: %d must be <= 20\n",
			program, USERMAX_DIGITS);
		exit(39);
	}
	for (i = 0, ten = 1; ten > 0 && ten <= E__USERMAX; ++i) {
		ten *= 10;
	}
	if (i != USERMAX_DIGITS) {
		fprintf(stderr, "**** %s ERROR: USERMAX_DIGITS: %d must be == %ld\n",
			program, USERMAX_DIGITS, (unsigned long)i);
		exit(40);
	}

	/*
	 * verify that E__USERMAX can fit into a value.v_type
	 *
	 * When a VALUE indicates an error, value.v_type is set to < 0.
	 * So we need to test if -E__USERMAX can be stored into a test.v_type.
	 */
	test.v_type = -E__USERMAX;
	if (-E__USERMAX != test.v_type) {
		fprintf(stderr, "**** %s ERROR: E__USERMAX: %d cannot fit into a value.v_type of size: %lu\n",
			program, E__USERMAX, (unsigned long)sizeof(test.v_type));
		exit(41);
	}
	return;
}


/*
 * errsym_2_errnum - convert a valid errsym to errnum
 *
 * We will convert E_STRING of E_digits into errnum if digits as a valid errnum,
 * otherwise we will search private_error_alias[] if E_STRING is of E__ form,
 * otherwise we will search error_table[] if E_STRING is of E_ form,
 * otherwise we will return NULL_ERRNUM.
 *
 * given:
 *	errsym	pointer to a E_STRING
 *
 * returns:
 *	!= NULL_ERRNUM ==> valid errnum corresponding to a valid E_STRING errsym
 *	== NULL_ERRNUM ==> errsym is NULL, not valid E_STRING errsym,
 *			   or no corresponding errnum found
 */
int
errsym_2_errnum(CONST char *errsym)
{
	struct errtbl *found;	/* pointer to entry in struct errtbl array with matching errsym */
	int errnum;		/* errnum to return */

	/*
	 * firewall
	 */
	if (errsym == NULL) {
		/* errsym is NULL */
		return NULL_ERRNUM;
	}

	/*
	 * case: E_STRING is a E_digits errsym
	 */
	errnum = e_digits_2_errnum(errsym);
	if (is_valid_errnum(errnum) == true) {
		/* digits of E_digits is a valid errnum */
		return errnum;
	}

	/*
	 * case: E_STRING is a E_digits but errnum is not valid
	 */
	if (is_e_digits(errsym) == true) {
		/* E_digits but digits to not form a valid errnum */
		return NULL_ERRNUM;
	}

	/*
	 * case: E_STRING is a known E__ errsym
	 */
	if (is_e_2string(errsym) == true) {

		/*
		 * look in private_error_alias[] for E__ errsym
		 */
		found = find_errsym_in_errtbl(errsym, private_error_alias);
		if (found != NULL) {
			/* return matching errnum */
			return found->errnum;
		}
	}

	/*
	 * case: E_STRING is a known E_ errsym
	 */
	if (is_e_1string(errsym) == true) {

		/*
		 * look in error_table[] for E_ errsym
		 */
		found = find_errsym_in_errtbl(errsym, error_table);
		if (found != NULL) {
			/* return matching errnum */
			return found->errnum;
		}
	}

	/*
	 * cannot convert errsym to a valid errnum
	 */
	return NULL_ERRNUM;
}


/*
 * The code below is not compatible with the errcode executable
 */
#if !defined(ERRCODE_SRC)


/*
 * errnum_2_errsym - convert an errnum value into an errsym E_STRING, possibly malloced
 *
 * given:
 *	errnum		errnum code to convert
 *	palloced	pointer to bool to be set if errsym E_STRING was malloced
 *
 * return:
 *	!= NULL ==> E_STRING for errnum (malloced if *palloced == true, static is not)
 *	== NULL ==> errnum is not valid, or palloced is NULL
 */
char *
errnum_2_errsym(int errnum, bool *palloced)
{
	char *ret;			/* return string, possibly malloced */
	CONST struct errtbl *entry;	/* struct errtbl entry lookup found or NULL */
	size_t snprintf_len;		/* malloced snprintf buffer length */

	/*
	 * firewall
	 */
	if (palloced == NULL) {
		/* palloced is NULL */
		return NULL;
	}
	if (is_valid_errnum(errnum) == false) {
		/* errnum is not a valid code */
		*palloced = false;
		return NULL;
	}

	/*
	 * case: errnum is within in the error_table[] array bounds
	 */
	if (is_errnum_in_error_table(errnum) == true) {

		/*
		 * struct errtbl entry found
		 */
		entry = lookup_errnum_in_error_table(errnum);
		if (entry == NULL) {
			/* lookup failed, return NULL */
			*palloced = false;
			return NULL;
		}

		/*
		 * return non-malloced errsym
		 */
		*palloced = false;
		ret = entry->errsym;
		return ret;
	}

	/*
	 * case: form a malloced E_STRING of E_digits
	 */
	snprintf_len = sizeof("E_")-1 + USERMAX_DIGITS + 1;	/* + 1 for trailing NUL */
	ret = calloc(snprintf_len+1, 1);	/* +1 for paranoia */
	if (ret == NULL) {
		math_error("Out of memory for errnum_2_errsym");
		not_reached();
	}
	*palloced = true;
	snprintf(ret, snprintf_len, "E_%d", errnum);
	ret[snprintf_len] = '\0';	/* paranoia */

	/*
	 * return alloced E_STRING of E_digits
	 */
	return ret;
}


/*
 * errnum_2_errmsg - convert an errnum value into an errmsg string, possibly malloced
 *
 * given:
 *	errnum		errnum code to convert
 *	palloced	pointer to bool to be set if errmsg string was malloced
 *
 * return:
 *	!= NULL ==> errmsg string for errnum (malloced if *palloced == true, static is not)
 *	== NULL ==> errnum is not valid, or palloced is NULL
 */
char *
errnum_2_errmsg(int errnum, bool *palloced)
{
	char *ret;			/* return string, possibly malloced */
	CONST struct errtbl *entry;	/* struct errtbl entry lookup found or NULL */
	size_t snprintf_len;		/* malloced snprintf buffer length */

	/*
	 * firewall
	 */
	if (palloced == NULL) {
		/* palloced is NULL */
		return NULL;
	}
	if (is_valid_errnum(errnum) == false) {
		/* errnum is not a valid code */
		*palloced = false;
		return NULL;
	}

	/*
	 * case: errnum is within in the error_table[] array bounds
	 */
	if (is_errnum_in_error_table(errnum) == true) {

		/*
		 * struct errtbl entry found
		 */
		entry = lookup_errnum_in_error_table(errnum);
		if (entry == NULL) {
			/* lookup failed, return NULL */
			*palloced = false;
			return NULL;
		}

		/*
		 * return non-malloced errmsg
		 */
		*palloced = false;
		ret = entry->errmsg;

	/*
	 * case: errnum is a user-described errno
	 */
	} else if (errnum >= E__USERDEF && errnum <= E__USERMAX) {

		/*
		 * return non-malloced errmsg
		 */
		*palloced = false;
		ret = name_newerrorstr(errnum);
		if (ret == NULL) {
			snprintf_len = sizeof("Unknown user error ")-1 + USERMAX_DIGITS + 1;	/* + 1 for trailing NUL */
			ret = calloc(snprintf_len+1, 1);	/* +1 for paranoia */
			if (ret == NULL) {
				math_error("Out of memory #0 for errnum_2_errmsg");
				not_reached();
			}
			*palloced = true;
			snprintf(ret, snprintf_len, "Unknown user error %d", errnum);
			ret[snprintf_len] = '\0';	/* paranoia */
		}

	/*
	 * case: errnum is 0
	 */
	} else if (errnum == E__NONE) {

		/*
		 * return "No error"
		 */
		*palloced = false;
		ret = "No error";

	/*
	 * case: errnum is a system errno
	 */
	} else if (errnum > E__NONE && errnum < E__BASE) {

		/*
		 * return non-malloced result of strerror()
		 */
		*palloced = false;
		ret = strerror(errnum);
		if (ret == NULL) {
			snprintf_len = sizeof("Unknown system error ")-1 + USERMAX_DIGITS + 1;	/* + 1 for trailing NUL */
			ret = calloc(snprintf_len+1, 1);	/* +1 for paranoia */
			if (ret == NULL) {
				math_error("Out of memory #1 for errnum_2_errmsg");
				not_reached();
			}
			*palloced = true;
			snprintf(ret, snprintf_len, "Unknown system error %d", errnum);
			ret[snprintf_len] = '\0';	/* paranoia */
		}

	/*
	 * case: unknown errnum
	 */
	} else {
		snprintf_len = sizeof("Unknown error ")-1 + USERMAX_DIGITS + 1;	/* + 1 for trailing NUL */
		ret = calloc(snprintf_len+1, 1);	/* +1 for paranoia */
		if (ret == NULL) {
			math_error("Out of memory #2 for errnum_2_errmsg");
			not_reached();
		}
		*palloced = true;
		snprintf(ret, snprintf_len, "Unknown error %d", errnum);
		ret[snprintf_len] = '\0';	/* paranoia */
	}

	/*
	 * return possibly allocated errmsg string
	 */
	return ret;
}


/*
 * errsym_2_errmsg - convert convert an E_STRING into an errmsg string, possibly malloced
 *
 * given:
 *	errsym		E_STRING to convert
 *	palloced	pointer to bool to be set if errmsg string was malloced
 *
 * return:
 *	!= NULL ==> errmsg string for errsym (malloced if *palloced == true, static is not)
 *	== NULL ==> errsym is not valid, or palloced is NULL, or errsym is NULL
 */
char *
errsym_2_errmsg(CONST char *errsym, bool *palloced)
{
	int errnum;	/* errsym E_STRING converted to errnum code or NULL_ERRNUM */
	char *ret;	/* errmsg string, possibly malloced, or NULL */

	/*
	 * firewall
	 */
	if (palloced == NULL) {
		/* palloced is NULL */
		return NULL;
	}
	if (errsym == NULL) {
		/* errsym is NULL */
		*palloced = false;
		return NULL;
	}

	/*
	 * convert errsym E_STRING to errnum
	 */
	errnum = errsym_2_errnum(errsym);
	if (is_valid_errnum(errnum) == false) {
		/* errsym did not convert into a valid errnum code */
		*palloced = false;
		return NULL;
	}

	/*
	 * return errnum value into an errmsg string, possibly malloced or return NULL
	 */
	ret = errnum_2_errmsg(errnum, palloced);
	return ret;
}


#endif /* !ERRCODE_SRC */


/*
 * The code below is used to form the errcode executable and is NOT part of libcalc
 */
#if defined(ERRCODE_SRC)


/*
 * errcode command line
 */
CONST char *usage =
	"usage: %s [-h] [-e | -d]\n"
	"\n"
	"\t-h\tprint help and exit\n"
	"\t-e\tprint the contents of help/errorcodes\n"
	"\t-d\tprint the contents of errsym.h\n"
	"\n"
	"Exit codes:\n"
	"    0\t\tall is OK\n"
	"    2\t\t-h and help string\n"
	"    3\t\tcommand line error\n"
	" >=10\t\terror_table[] and/or private_error_alias[] is invalid\n";


char *program = "((NULL))";		/* our name */


/*
 * print_errorcodes - print the help/errorcodes file
 */
S_FUNC void
print_errorcodes(void)
{
	/*
	 * print the help/errorcodes file header
	 */
	size_t len;		/* length of the error_table */
	size_t i;

	/*
	 * print the help/errorcodes file header
	 */
	printf("## DO NOT EDIT - This file was generated by errcode -e via the Makefile\n"
	       "Calc computation error codes\n"
	       "\n"
	       "\"E_STRING\"\terrnum\tCalc computation error message\n"
	       "----------\t------\t------------------------------\n"
	       "\n"
	       "\"E__NONE\"\t%d\tcalc_errno cleared: libc errno codes above here\n"
	       "\"E__BASE\"\t%d\tReserved for \"No error\" calc internal state\n"
	       "\n",
	       E__NONE, E__BASE);

	/*
	 * print the error message lines
	 */
	len = sizeof(error_table) / sizeof(error_table[0]);
	for (i=1; i < len-1; ++i) {
		if (strlen(error_table[i].errsym) < 6) {
			printf("\"%s\"\t\t%d\t%s\n",
			       error_table[i].errsym, error_table[i].errnum, error_table[i].errmsg);
		} else {
			printf("\"%s\"\t%d\t%s\n",
			       error_table[i].errsym, error_table[i].errnum, error_table[i].errmsg);
		}
	}

	/*
	 * print the help/errorcodes file trailer
	 */
	printf("\n"
	      "\"E__HIGHEST\"\t%lu\thighest assigned calc computation error code\n"
	      "\"E__USERDEF\"\t%lu\tuser defined error codes start here\n"
	      "\"E__USERMAX\"\t%lu\tmaximum user defined error code\n",
	      (unsigned long)MY_E__HIGHEST, (unsigned long)E__USERDEF, (unsigned long)E__USERMAX);
	printf("\n"
	       "## Copyright (C) %d  Landon Curt Noll\n"
	       "##\n"
	       "## Calc is open software; you can redistribute it and/or modify it under\n"
	       "## the terms of the version 2.1 of the GNU Lesser General Public License\n"
	       "## as published by the Free Software Foundation.\n"
	       "##\n"
	       "## Calc is distributed in the hope that it will be useful, but WITHOUT\n"
	       "## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n"
	       "## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General\n"
	       "## Public License for more details.\n"
	       "##\n"
	       "## A copy of version 2.1 of the GNU Lesser General Public License is\n"
	       "## distributed with calc under the filename COPYING-LGPL.  You should have\n"
	       "## received a copy with calc; if not, write to Free Software Foundation, Inc.\n"
	       "## 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
	       "##\n"
	       "## Under source code control:   n/a\n"
	       "## File existed as early as:    n/a\n"
	       "##\n"
	       "## chongo <was here> /\\oo/\\     http://www.isthe.com/chongo/\n"
	       "## Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/\n",
	       ERRTBL_COPYRIGHT_YEAR);
	return;
}


/*
 * print_errsym - print the contents of errsym.h
 */
S_FUNC void
print_errsym(void)
{
	size_t len;		/* length of the error_table */
	size_t i;

	/*
	 * print the errsym.h file header
	 */
	printf("/*\n"
	       " * DO NOT EDIT - This file was generated by errcode -d via the Makefile\n"
	       " *\n"
	       " * Copyright (C) %d  Landon Curt Noll\n"
	       " *\n"
	       " * Calc is open software; you can redistribute it and/or modify it under\n"
	       " * the terms of the version 2.1 of the GNU Lesser General Public License\n"
	       " * as published by the Free Software Foundation.\n"
	       " *\n"
	       " * Calc is distributed in the hope that it will be useful, but WITHOUT\n"
	       " * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n"
	       " * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General\n"
	       " * Public License for more details.\n"
	       " *\n"
	       " * A copy of version 2.1 of the GNU Lesser General Public License is\n"
	       " * distributed with calc under the filename COPYING-LGPL.  You should have\n"
	       " * received a copy with calc; if not, write to Free Software Foundation, Inc.\n"
	       " * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
	       " *\n"
	       " * Under source code control:   n/a\n"
	       " * File existed as early as:    n/a\n"
	       " *\n"
	       " * chongo <was here> /\\oo/\\     http://www.isthe.com/chongo/\n"
	       " * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/\n"
	       " */\n"
	       "\n"
	       "\n"
	       "#if !defined(INCLUDE_ERRSYM_H)\n"
	       "#define INCLUDE_ERRSYM_H\n"
	       "\n"
	       "\n",
	       ERRTBL_COPYRIGHT_YEAR);

	/*
	 * print the #define lines
	 */
	len = sizeof(error_table) / sizeof(error_table[0]);
	for (i=1; i < len-1; ++i) {
		if (strlen(error_table[i].errsym) < 8) {
			printf("#define %s\t\t%d\t/* %s */\n",
			       error_table[i].errsym, error_table[i].errnum, error_table[i].errmsg);
		} else {
			printf("#define %s\t%d\t/* %s */\n",
			       error_table[i].errsym, error_table[i].errnum, error_table[i].errmsg);
		}
	}

	/*
	 * print the errsym.h file trailer
	 */
	printf("\n"
	       "#define E__HIGHEST\t%lu\t/* highest assigned calc computation error code */\n"
	       "\n"
	       "#define ECOUNT\t%lu\t/* number of calc computation error codes w/o E__BASE */\n",
	       (unsigned long)MY_E__HIGHEST, (unsigned long)MY_ECOUNT);
	printf("\n"
	       "\n"
	       "#endif /* !INCLUDE_ERRSYM_H */\n");
	return;
}


int
main(int argc, char *argv[])
{
	extern char *optarg;		/* argv index of the next arg */
	extern int optind;		/* argv index of the next arg */
	int e_flag = 0;			/* 1 ==> -e flag was used */
	int d_flag = 0;			/* 1 ==> -s flag was used */
	int i;

	/*
	 * parse args
	 */
	program = argv[0];
	while ((i = getopt(argc, argv, "hed")) != -1) {
		switch (i) {
		case 'h':
			fprintf(stderr, usage, program);
			exit(2); /*ooo*/
		case 'e':
			e_flag = 1;
			break;
		case 'd':
			d_flag = 1;
			break;
		default:
		    fprintf(stderr, "**** %s ERROR: invalid command line\n", program);
			fprintf(stderr, usage, program);
			exit(3); /*ooo*/
		}
	}
	if (e_flag && d_flag) {
		fprintf(stderr, "**** %s ERROR: -e and -d conflict\n", program);
		fprintf(stderr, usage, program);
		exit(3); /*ooo*/
	}

	/*
	 * verify the consistency of the error_table
	 */
	 verify_error_table();

	/*
	 * -e - print the contents of help/errorcodes
	 */
	if (e_flag) {
		print_errorcodes();
	}

	/*
	 * -d - print the contents of errsym.h
	 */
	if (d_flag) {
		print_errsym();
	}

	/*
	 * All Done!!! -- Jessica Noll, Age 2
	 */
	exit(0); /*ooo*/
}


#endif /* ERRCODE_SRC */
