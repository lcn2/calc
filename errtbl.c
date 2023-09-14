/*
 * errtbl - calc computation error codes, symbols and messages
 *
 * This code is used to generate errsym.h and help/errorcodes.
 * This code also verifies the consistency of the error_table[] array.
 *
 * NOTE: This file was once called calcerr.tbl.
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
 */

#include <stdio.h>

#include "errtbl.h"

#define ERRTBL_COPYRIGHT_YEAR 2023	/* Copyright (C) year for generated files */

/* number of calc computation error codes */
#define MY_ECOUNT ((sizeof(error_table) / sizeof(error_table[0])) - 2)

/* highest assigned calc computation error code */
#define MY_E__HIGHEST (E__BASE + MY_ECOUNT)


/************************************************************************/
/*									*/
/*  WARNING: The order of the lines below is critical!  If you change	*/
/*	     the order, you will break code that depends on the return	*/
/*	     value of the errno() builtin function.			*/
/*									*/
/*	     When adding entries to this table add then just before the */
/*	     NULL pointer (* must be last *) entry.			*/
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
 * The final entry must have an errnum of -1, errsym of NULL and errmsg of NULL.
 *
 * With exception to the 1st E__BASE entry, all other errsym strings
 * must match the following regular expression:
 *
 *	^E_[A-Z][A-Z0-9_]+$
 *
 * NOTE: The above regular expression is more restrictive them the
 *	 "E_STRING" regular expression from help/errno, help/error,
 *	 and help/strerror.  This is because errsym strings that
 *	 start with "E__" are special symbols that #define-d in errtbl.h,
 *	 AND because errsym strings that are "^E_[0-9]+$" reserved for
 *	 numeric aliases for errnum.
 *
 * Keep the errmsg short enough that lines in this table are not too long.
 * You might want to keep the length of the errmsg to 80 characters or less.
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
	{ 10015,	"E_APPR",		"Bad first argument for appr" },
	{ 10016,	"E_APPR2",		"Bad second argument for appr" },
	{ 10017,	"E_APPR3",		"Bad third argument for appr" },
	{ 10018,	"E_ROUND",		"Bad first argument for round" },
	{ 10019,	"E_ROUND2",		"Bad second argument for round" },
	{ 10020,	"E_ROUND3",		"Bad third argument for round" },
	{ 10021,	"E_BROUND",		"Bad first argument for bround" },
	{ 10022,	"E_BROUND2",		"Bad second argument for bround" },
	{ 10023,	"E_BROUND3",		"Bad third argument for bround" },
	{ 10024,	"E_SQRT",		"Bad first argument for sqrt" },
	{ 10025,	"E_SQRT2",		"Bad second argument for sqrt" },
	{ 10026,	"E_SQRT3",		"Bad third argument for sqrt" },
	{ 10027,	"E_ROOT",		"Bad first argument for root" },
	{ 10028,	"E_ROOT2",		"Bad second argument for root" },
	{ 10029,	"E_ROOT3",		"Bad third argument for root" },
	{ 10030,	"E_NORM",		"Bad argument for norm" },
	{ 10031,	"E_SHIFT",		"Bad first argument for << or >>" },
	{ 10032,	"E_SHIFT2",		"Bad second argument for << or >>" },
	{ 10033,	"E_SCALE",		"Bad first argument for scale" },
	{ 10034,	"E_SCALE2",		"Bad second argument for scale" },
	{ 10035,	"E_POWI",		"Bad first argument for ^" },
	{ 10036,	"E_POWI2",		"Bad second argument for ^" },
	{ 10037,	"E_POWER",		"Bad first argument for power" },
	{ 10038,	"E_POWER2",		"Bad second argument for power" },
	{ 10039,	"E_POWER3",		"Bad third argument for power" },
	{ 10040,	"E_QUO",		"Bad first argument for quo or //" },
	{ 10041,	"E_QUO2",		"Bad second argument for quo or //" },
	{ 10042,	"E_QUO3",		"Bad third argument for quo" },
	{ 10043,	"E_MOD",		"Bad first argument for mod or %" },
	{ 10044,	"E_MOD2",		"Bad second argument for mod or %" },
	{ 10045,	"E_MOD3",		"Bad third argument for mod" },
	{ 10046,	"E_SGN",		"Bad argument for sgn" },
	{ 10047,	"E_ABS",		"Bad first argument for abs" },
	{ 10048,	"E_ABS2",		"Bad second argument for abs" },
	{ 10049,	"E_EVAL",		"Scan error in argument for eval" },
	{ 10050,	"E_STR",		"Non-simple type for str" },
	{ 10051,	"E_EXP1",		"Non-real epsilon for exp" },
	{ 10052,	"E_EXP2",		"Bad first argument for exp" },
	{ 10053,	"E_FPUTC1",		"Non-file first argument for fputc" },
	{ 10054,	"E_FPUTC2",		"Bad second argument for fputc" },
	{ 10055,	"E_FPUTC3",		"File not open for writing for fputc" },
	{ 10056,	"E_FGETC1",		"Non-file first argument for fgetc" },
	{ 10057,	"E_FGETC2",		"File not open for reading for fgetc" },
	{ 10058,	"E_FOPEN1",		"Non-string arguments for fopen" },
	{ 10059,	"E_FOPEN2",		"Unrecognized mode for fopen" },
	{ 10060,	"E_FREOPEN1",		"Non-file first argument for freopen" },
	{ 10061,	"E_FREOPEN2",		"Non-string or unrecognized mode for freopen" },
	{ 10062,	"E_FREOPEN3",		"Non-string third argument for freopen" },
	{ 10063,	"E_FCLOSE1",		"Non-file argument for fclose" },
	{ 10064,	"E_FFLUSH",		"Non-file argument for fflush" },
	{ 10065,	"E_FPUTS1",		"Non-file first argument for fputs" },
	{ 10066,	"E_FPUTS2",		"Non-string argument after first for fputs" },
	{ 10067,	"E_FPUTS3",		"File not open for writing for fputs" },
	{ 10068,	"E_FGETS1",		"Non-file argument for fgets" },
	{ 10069,	"E_FGETS2",		"File not open for reading for fgets" },
	{ 10070,	"E_FPUTSTR1",		"Non-file first argument for fputstr" },
	{ 10071,	"E_FPUTSTR2",		"Non-string argument after first for fputstr" },
	{ 10072,	"E_FPUTSTR3",		"File not open for writing for fputstr" },
	{ 10073,	"E_FGETSTR1",		"Non-file first argument for fgetstr" },
	{ 10074,	"E_FGETSTR2",		"File not open for reading for fgetstr" },
	{ 10075,	"E_FGETLINE1",		"Non-file  argument for fgetline" },
	{ 10076,	"E_FGETLINE2",		"File not open for reading for fgetline" },
	{ 10077,	"E_FGETFIELD1",		"Non-file argument for fgetfield" },
	{ 10078,	"E_FGETFIELD2",		"File not open for reading for fgetfield" },
	{ 10079,	"E_REWIND1",		"Non-file argument for rewind" },
	{ 10080,	"E_FILES",		"Non-integer argument for files" },
	{ 10081,	"E_PRINTF1",		"Non-string fmt argument for fprint" },
	{ 10082,	"E_PRINTF2",		"Stdout not open for writing to ???" },
	{ 10083,	"E_FPRINTF1",		"Non-file first argument for fprintf" },
	{ 10084,	"E_FPRINTF2",		"Non-string second (fmt) argument for fprintf" },
	{ 10085,	"E_FPRINTF3",		"File not open for writing for fprintf" },
	{ 10086,	"E_STRPRINTF1",		"Non-string first (fmt) argument for strprintf" },
	{ 10087,	"E_STRPRINTF2",		"Error in attempting strprintf ???" },
	{ 10088,	"E_FSCAN1",		"Non-file first argument for fscan" },
	{ 10089,	"E_FSCAN2",		"File not open for reading for fscan" },
	{ 10090,	"E_STRSCAN",		"Non-string first argument for strscan" },
	{ 10091,	"E_FSCANF1",		"Non-file first argument for fscanf" },
	{ 10092,	"E_FSCANF2",		"Non-string second (fmt) argument for fscanf" },
	{ 10093,	"E_FSCANF3",		"Non-lvalue argument after second for fscanf" },
	{ 10094,	"E_FSCANF4",		"File not open for reading or other error for fscanf" },
	{ 10095,	"E_STRSCANF1",		"Non-string first argument for strscanf" },
	{ 10096,	"E_STRSCANF2",		"Non-string second (fmt) argument for strscanf" },
	{ 10097,	"E_STRSCANF3",		"Non-lvalue argument after second for strscanf" },
	{ 10098,	"E_STRSCANF4",		"Some error in attempting strscanf ???" },
	{ 10099,	"E_SCANF1",		"Non-string first (fmt) argument for scanf" },
	{ 10100,	"E_SCANF2",		"Non-lvalue argument after first for scanf" },
	{ 10101,	"E_SCANF3",		"Some error in attempting scanf ???" },
	{ 10102,	"E_FTELL1",		"Non-file argument for ftell" },
	{ 10103,	"E_FTELL2",		"File not open or other error for ftell" },
	{ 10104,	"E_FSEEK1",		"Non-file first argument for fseek" },
	{ 10105,	"E_FSEEK2",		"Non-integer or negative second argument for fseek" },
	{ 10106,	"E_FSEEK3",		"File not open or other error for fseek" },
	{ 10107,	"E_FSIZE1",		"Non-file argument for fsize" },
	{ 10108,	"E_FSIZE2",		"File not open or other error for fsize" },
	{ 10109,	"E_FEOF1",		"Non-file argument for feof" },
	{ 10110,	"E_FEOF2",		"File not open or other error for feof" },
	{ 10111,	"E_FERROR1",		"Non-file argument for ferror" },
	{ 10112,	"E_FERROR2",		"File not open or other error for ferror" },
	{ 10113,	"E_UNGETC1",		"Non-file argument for ungetc" },
	{ 10114,	"E_UNGETC2",		"File not open for reading for ungetc" },
	{ 10115,	"E_UNGETC3",		"Bad second argument or other error for ungetc" },
	{ 10116,	"E_BIGEXP",		"Exponent too big in scanning" },
	{ 10117,	"E_ISATTY1",		"UNUSED ERROR: E_ISATTY1 is no longer used" },
	{ 10118,	"E_ISATTY2",		"UNUSED ERROR: E_ISATTY2 is no longer used" },
	{ 10119,	"E_ACCESS1",		"Non-string first argument for access" },
	{ 10120,	"E_ACCESS2",		"Bad second argument for access" },
	{ 10121,	"E_SEARCH1",		"Bad first argument for search" },
	{ 10122,	"E_SEARCH2",		"Bad second argument for search" },
	{ 10123,	"E_SEARCH3",		"Bad third argument for search" },
	{ 10124,	"E_SEARCH4",		"Bad fourth argument for search" },
	{ 10125,	"E_SEARCH5",		"Cannot find fsize or fpos for search" },
	{ 10126,	"E_SEARCH6",		"File not readable for search" },
	{ 10127,	"E_RSEARCH1",		"Bad first argument for rsearch" },
	{ 10128,	"E_RSEARCH2",		"Bad second argument for rsearch" },
	{ 10129,	"E_RSEARCH3",		"Bad third argument for rsearch" },
	{ 10130,	"E_RSEARCH4",		"Bad fourth argument for rsearch" },
	{ 10131,	"E_RSEARCH5",		"Cannot find fsize or fpos for rsearch" },
	{ 10132,	"E_RSEARCH6",		"File not readable for rsearch" },
	{ 10133,	"E_MANYOPEN",		"Too many open files" },
	{ 10134,	"E_REWIND2",		"Attempt to rewind a file that is not open" },
	{ 10135,	"E_STRERROR1",		"Bad argument type for strerror" },
	{ 10136,	"E_STRERROR2",		"Index out of range for strerror" },
	{ 10137,	"E_COS1",		"Bad epsilon for cos" },
	{ 10138,	"E_COS2",		"Bad first argument for cos" },
	{ 10139,	"E_SIN1",		"Bad epsilon for sin" },
	{ 10140,	"E_SIN2",		"Bad first argument for sin" },
	{ 10141,	"E_EVAL2",		"Non-string argument for eval" },
	{ 10142,	"E_ARG1",		"Bad epsilon for arg" },
	{ 10143,	"E_ARG2",		"Bad first argument for arg" },
	{ 10144,	"E_POLAR1",		"Non-real argument for polar" },
	{ 10145,	"E_POLAR2",		"Bad epsilon for polar" },
	{ 10146,	"E_FCNT",		"UNUSED ERROR: Non-integral argument for fcnt" },
	{ 10147,	"E_MATFILL1",		"Non-variable first argument for matfill" },
	{ 10148,	"E_MATFILL2",		"Non-matrix first argument-value for matfill" },
	{ 10149,	"E_MATDIM",		"Non-matrix argument for matdim" },
	{ 10150,	"E_MATSUM",		"Non-matrix argument for matsum" },
	{ 10151,	"E_ISIDENT",		"UNUSED ERROR: E_ISIDENT is no longer used" },
	{ 10152,	"E_MATTRANS1",		"Non-matrix argument for mattrans" },
	{ 10153,	"E_MATTRANS2",		"Non-two-dimensional matrix for mattrans" },
	{ 10154,	"E_DET1",		"Non-matrix argument for det" },
	{ 10155,	"E_DET2",		"Matrix for det not of dimension 2" },
	{ 10156,	"E_DET3",		"Non-square matrix for det" },
	{ 10157,	"E_MATMIN1",		"Non-matrix first argument for matmin" },
	{ 10158,	"E_MATMIN2",		"Non-positive-integer second argument for matmin" },
	{ 10159,	"E_MATMIN3",		"Second argument for matmin exceeds dimension" },
	{ 10160,	"E_MATMAX1",		"Non-matrix first argument for matmin" },
	{ 10161,	"E_MATMAX2",		"Second argument for matmax not positive integer" },
	{ 10162,	"E_MATMAX3",		"Second argument for matmax exceeds dimension" },
	{ 10163,	"E_CP1",		"Non-matrix argument for cp" },
	{ 10164,	"E_CP2",		"Non-one-dimensional matrix for cp" },
	{ 10165,	"E_CP3",		"Matrix size not 3 for cp" },
	{ 10166,	"E_DP1",		"Non-matrix argument for dp" },
	{ 10167,	"E_DP2",		"Non-one-dimensional matrix for dp" },
	{ 10168,	"E_DP3",		"Different-size matrices for dp" },
	{ 10169,	"E_STRLEN",		"Non-string argument for strlen" },
	{ 10170,	"E_STRCAT",		"Non-string argument for strcat" },
	{ 10171,	"E_SUBSTR1",		"Non-string first argument for strcat" },
	{ 10172,	"E_SUBSTR2",		"Non-non-negative integer second argument for strcat" },
	{ 10173,	"E_CHAR",		"Bad argument for char" },
	{ 10174,	"E_ORD",		"Non-string argument for ord" },
	{ 10175,	"E_INSERT1",		"Non-list-variable first argument for insert" },
	{ 10176,	"E_INSERT2",		"Non-integral second argument for insert" },
	{ 10177,	"E_PUSH",		"Non-list-variable first argument for push" },
	{ 10178,	"E_APPEND",		"Non-list-variable first argument for append" },
	{ 10179,	"E_DELETE1",		"Non-list-variable first argument for delete" },
	{ 10180,	"E_DELETE2",		"Non-integral second argument for delete" },
	{ 10181,	"E_POP",		"Non-list-variable argument for pop" },
	{ 10182,	"E_REMOVE",		"Non-list-variable argument for remove" },
	{ 10183,	"E_LN1",		"Bad epsilon argument for ln" },
	{ 10184,	"E_LN2",		"Non-numeric first argument for ln" },
	{ 10185,	"E_ERROR1",		"Non-integer argument for error" },
	{ 10186,	"E_ERROR2",		"Argument outside range for error" },
	{ 10187,	"E_EVAL3",		"Attempt to eval at maximum input depth" },
	{ 10188,	"E_EVAL4",		"Unable to open string for reading" },
	{ 10189,	"E_RM1",		"First argument for rm is not a non-empty string" },
	{ 10190,	"E_RM2",		"UNUSED ERROR: Unable to remove a file" },
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
	{ 10202,	"E_BLK1",		"Non-integral length for block" },
	{ 10203,	"E_BLK2",		"Negative or too-large length for block" },
	{ 10204,	"E_BLK3",		"Non-integral chunksize for block" },
	{ 10205,	"E_BLK4",		"Negative or too-large chunksize for block" },
	{ 10206,	"E_BLKFREE1",		"Named block does not exist for blkfree" },
	{ 10207,	"E_BLKFREE2",		"Non-integral id specification for blkfree" },
	{ 10208,	"E_BLKFREE3",		"Block with specified id does not exist" },
	{ 10209,	"E_BLKFREE4",		"Block already freed" },
	{ 10210,	"E_BLKFREE5",		"No-realloc protection prevents blkfree" },
	{ 10211,	"E_BLOCKS1",		"Non-integer argument for blocks" },
	{ 10212,	"E_BLOCKS2",		"Non-allocated index number for blocks" },
	{ 10213,	"E_COPY1",		"Non-integer or negative source index for copy" },
	{ 10214,	"E_COPY2",		"Source index too large for copy" },
	{ 10215,	"E_COPY3",		"UNUSED ERROR: E_COPY3 is no longer used" },
	{ 10216,	"E_COPY4",		"Non-integer or negative number for copy" },
	{ 10217,	"E_COPY5",		"Number too large for copy" },
	{ 10218,	"E_COPY6",		"Non-integer or negative destination index for copy" },
	{ 10219,	"E_COPY7",		"Destination index too large for copy" },
	{ 10220,	"E_COPY8",		"Freed block source for copy" },
	{ 10221,	"E_COPY9",		"Unsuitable source type for copy" },
	{ 10222,	"E_COPY10",		"Freed block destination for copy" },
	{ 10223,	"E_COPY11",		"Unsuitable destination type for copy" },
	{ 10224,	"E_COPY12",		"Incompatible source and destination for copy" },
	{ 10225,	"E_COPY13",		"No-copy-from source variable" },
	{ 10226,	"E_COPY14",		"No-copy-to destination variable" },
	{ 10227,	"E_COPY15",		"No-copy-from source named block" },
	{ 10228,	"E_COPY16",		"No-copy-to destination named block" },
	{ 10229,	"E_COPY17",		"No-relocate destination for copy" },
	{ 10230,	"E_COPYF1",		"File not open for copy" },
	{ 10231,	"E_COPYF2",		"fseek or fsize failure for copy" },
	{ 10232,	"E_COPYF3",		"fwrite error for copy" },
	{ 10233,	"E_COPYF4",		"fread error for copy" },
	{ 10234,	"E_PROTECT1",		"Non-variable first argument for protect" },
	{ 10235,	"E_PROTECT2",		"Bad second argument for protect" },
	{ 10236,	"E_PROTECT3",		"Bad third argument for protect" },
	{ 10237,	"E_MATFILL3",		"No-copy-to destination for matfill" },
	{ 10238,	"E_MATFILL4",		"No-assign-from source for matfill" },
	{ 10239,	"E_MATTRACE1",		"Non-matrix argument for mattrace" },
	{ 10240,	"E_MATTRACE2",		"Non-two-dimensional argument for mattrace" },
	{ 10241,	"E_MATTRACE3",		"Non-square argument for mattrace" },
	{ 10242,	"E_TAN1",		"Bad epsilon for tan" },
	{ 10243,	"E_TAN2",		"Bad argument for tan" },
	{ 10244,	"E_COT1",		"Bad epsilon for cot" },
	{ 10245,	"E_COT2",		"Bad argument for cot" },
	{ 10246,	"E_SEC1",		"Bad epsilon for sec" },
	{ 10247,	"E_SEC2",		"Bad argument for sec" },
	{ 10248,	"E_CSC1",		"Bad epsilon for csc" },
	{ 10249,	"E_CSC2",		"Bad argument for csc" },
	{ 10250,	"E_SINH1",		"Bad epsilon for sinh" },
	{ 10251,	"E_SINH2",		"Bad argument for sinh" },
	{ 10252,	"E_COSH1",		"Bad epsilon for cosh" },
	{ 10253,	"E_COSH2",		"Bad argument for cosh" },
	{ 10254,	"E_TANH1",		"Bad epsilon for tanh" },
	{ 10255,	"E_TANH2",		"Bad argument for tanh" },
	{ 10256,	"E_COTH1",		"Bad epsilon for coth" },
	{ 10257,	"E_COTH2",		"Bad argument for coth" },
	{ 10258,	"E_SECH1",		"Bad epsilon for sech" },
	{ 10259,	"E_SECH2",		"Bad argument for sech" },
	{ 10260,	"E_CSCH1",		"Bad epsilon for csch" },
	{ 10261,	"E_CSCH2",		"Bad argument for csch" },
	{ 10262,	"E_ASIN1",		"Bad epsilon for asin" },
	{ 10263,	"E_ASIN2",		"Bad argument for asin" },
	{ 10264,	"E_ACOS1",		"Bad epsilon for acos" },
	{ 10265,	"E_ACOS2",		"Bad argument for acos" },
	{ 10266,	"E_ATAN1",		"Bad epsilon for atan" },
	{ 10267,	"E_ATAN2",		"Bad argument for atan" },
	{ 10268,	"E_ACOT1",		"Bad epsilon for acot" },
	{ 10269,	"E_ACOT2",		"Bad argument for acot" },
	{ 10270,	"E_ASEC1",		"Bad epsilon for asec" },
	{ 10271,	"E_ASEC2",		"Bad argument for asec" },
	{ 10272,	"E_ACSC1",		"Bad epsilon for acsc" },
	{ 10273,	"E_ACSC2",		"Bad argument for acsc" },
	{ 10274,	"E_ASINH1",		"Bad epsilon for asin" },
	{ 10275,	"E_ASINH2",		"Bad argument for asinh" },
	{ 10276,	"E_ACOSH1",		"Bad epsilon for acosh" },
	{ 10277,	"E_ACOSH2",		"Bad argument for acosh" },
	{ 10278,	"E_ATANH1",		"Bad epsilon for atanh" },
	{ 10279,	"E_ATANH2",		"Bad argument for atanh" },
	{ 10280,	"E_ACOTH1",		"Bad epsilon for acoth" },
	{ 10281,	"E_ACOTH2",		"Bad argument for acoth" },
	{ 10282,	"E_ASECH1",		"UNUSED ERROR: Bad epsilon for asech" },
	{ 10283,	"E_ASECH2",		"Bad argument for asech" },
	{ 10284,	"E_ACSCH1",		"Bad epsilon for acsch" },
	{ 10285,	"E_ACSCH2",		"Bad argument for acsch" },
	{ 10286,	"E_GD1",		"Bad epsilon for gd" },
	{ 10287,	"E_GD2",		"Bad argument for gd" },
	{ 10288,	"E_AGD1",		"Bad epsilon for agd" },
	{ 10289,	"E_AGD2",		"Bad argument for agd" },
	{ 10290,	"E_LOGINF",		"Log of zero or infinity" },
	{ 10291,	"E_STRADD",		"String addition failure" },
	{ 10292,	"E_STRMUL",		"String multiplication failure" },
	{ 10293,	"E_STRNEG",		"String reversal failure" },
	{ 10294,	"E_STRSUB",		"String subtraction failure" },
	{ 10295,	"E_BIT1",		"Bad argument type for bit" },
	{ 10296,	"E_BIT2",		"Index too large for bit" },
	{ 10297,	"E_SETBIT1",		"Non-integer second argument for setbit" },
	{ 10298,	"E_SETBIT2",		"Out-of-range index for setbit" },
	{ 10299,	"E_SETBIT3",		"Non-string first argument for setbit" },
	{ 10300,	"E_OR",			"Bad argument for or" },
	{ 10301,	"E_AND",		"Bad argument for and" },
	{ 10302,	"E_STROR",		"Allocation failure for string or" },
	{ 10303,	"E_STRAND",		"Allocation failure for string and" },
	{ 10304,	"E_XOR",		"Bad argument for xorvalue" },
	{ 10305,	"E_COMP",		"Bad argument for comp" },
	{ 10306,	"E_STRDIFF",		"Allocation failure for string diff" },
	{ 10307,	"E_STRCOMP",		"Allocation failure for string comp" },
	{ 10308,	"E_SEG1",		"Bad first argument for segment" },
	{ 10309,	"E_SEG2",		"Bad second argument for segment" },
	{ 10310,	"E_SEG3",		"Bad third argument for segment" },
	{ 10311,	"E_STRSEG",		"Failure for string segment" },
	{ 10312,	"E_HIGHBIT1",		"Bad argument type for highbit" },
	{ 10313,	"E_HIGHBIT2",		"Non-integer argument for highbit" },
	{ 10314,	"E_LOWBIT1",		"Bad argument type for lowbit" },
	{ 10315,	"E_LOWBIT2",		"Non-integer argument for lowbit" },
	{ 10316,	"E_CONTENT",		"Bad argument type for unary hash op" },
	{ 10317,	"E_HASHOP",		"Bad argument type for binary hash op" },
	{ 10318,	"E_HEAD1",		"Bad first argument for head" },
	{ 10319,	"E_HEAD2",		"Bad second argument for head" },
	{ 10320,	"E_STRHEAD",		"Failure for strhead" },
	{ 10321,	"E_TAIL1",		"Bad first argument for tail" },
	{ 10322,	"E_TAIL2",		"UNUSED ERROR: Bad second argument for tail" },
	{ 10323,	"E_STRTAIL",		"Failure for strtail" },
	{ 10324,	"E_STRSHIFT",		"Failure for strshift" },
	{ 10325,	"E_STRCMP",		"Non-string argument for strcmp" },
	{ 10326,	"E_STRNCMP",		"Bad argument type for strncmp" },
	{ 10327,	"E_XOR1",		"Varying types of argument for xor" },
	{ 10328,	"E_XOR2",		"Bad argument type for xor" },
	{ 10329,	"E_STRCPY",		"Bad argument type for strcpy" },
	{ 10330,	"E_STRNCPY",		"Bad argument type for strncpy" },
	{ 10331,	"E_BACKSLASH",		"Bad argument type for unary backslash" },
	{ 10332,	"E_SETMINUS",		"Bad argument type for setminus" },
	{ 10333,	"E_INDICES1",		"Bad first argument type for indices" },
	{ 10334,	"E_INDICES2",		"Bad second argument for indices" },
	{ 10335,	"E_EXP3",		"Too-large re(argument) for exp" },
	{ 10336,	"E_SINH3",		"Too-large re(argument) for sinh" },
	{ 10337,	"E_COSH3",		"Too-large re(argument) for cosh" },
	{ 10338,	"E_SIN3",		"Too-large im(argument) for sin" },
	{ 10339,	"E_COS3",		"Too-large im(argument) for cos" },
	{ 10340,	"E_GD3",		"Infinite or too-large result for gd" },
	{ 10341,	"E_AGD3",		"Infinite or too-large result for agd" },
	{ 10342,	"E_POWER4",		"Too-large value for power" },
	{ 10343,	"E_ROOT4",		"Too-large value for root" },
	{ 10344,	"E_DGT1",		"Non-real first arg for digit" },
	{ 10345,	"E_DGT2",		"Non-integral second arg for digit" },
	{ 10346,	"E_DGT3",		"Bad third arg for digit" },
	{ 10347,	"E_PLCS1",		"Bad first argument for places" },
	{ 10348,	"E_PLCS2",		"Bad second argument for places" },
	{ 10349,	"E_DGTS1",		"Bad first argument for digits" },
	{ 10350,	"E_DGTS2",		"Bad second argument for digits" },
	{ 10351,	"E_ILOG",		"Bad first argument for ilog" },
	{ 10352,	"E_ILOGB",		"Bad second argument for ilog" },
	{ 10353,	"E_ILOG10",		"Bad argument for ilog10" },
	{ 10354,	"E_ILOG2",		"Bad argument for ilog2" },
	{ 10355,	"E_COMB1",		"Non-integer second arg for comb" },
	{ 10356,	"E_COMB2",		"Too-large second arg for comb" },
	{ 10357,	"E_CTLN",		"Bad argument for catalan" },
	{ 10358,	"E_BERN",		"Bad argument for bern" },
	{ 10359,	"E_EULER",		"Bad argument for euler" },
	{ 10360,	"E_SLEEP",		"Bad argument for sleep" },
	{ 10361,	"E_TTY",		"calc_tty failure" },
	{ 10362,	"E_ASSIGN1",		"No-copy-to destination for octet assign" },
	{ 10363,	"E_ASSIGN2",		"No-copy-from source for octet assign" },
	{ 10364,	"E_ASSIGN3",		"No-change destination for octet assign" },
	{ 10365,	"E_ASSIGN4",		"Non-variable destination for assign" },
	{ 10366,	"E_ASSIGN5",		"No-assign-to destination for assign" },
	{ 10367,	"E_ASSIGN6",		"No-assign-from source for assign" },
	{ 10368,	"E_ASSIGN7",		"No-change destination for assign" },
	{ 10369,	"E_ASSIGN8",		"No-type-change destination for assign" },
	{ 10370,	"E_ASSIGN9",		"No-error-value destination for assign" },
	{ 10371,	"E_SWAP1",		"No-copy argument for octet swap" },
	{ 10372,	"E_SWAP2",		"No-assign-to-or-from argument for swap" },
	{ 10373,	"E_SWAP3",		"Non-lvalue argument for swap" },
	{ 10374,	"E_QUOMOD1",		"Non-lvalue argument 3 or 4 for quomod" },
	{ 10375,	"E_QUOMOD2",		"Non-real-number arg 1 or 2 or bad arg 5 for quomod" },
	{ 10376,	"E_QUOMOD3",		"No-assign-to argument 3 or 4 for quomod" },
	{ 10377,	"E_PREINC1",		"No-copy-to or no-change argument for octet preinc" },
	{ 10378,	"E_PREINC2",		"Non-variable argument for preinc" },
	{ 10379,	"E_PREINC3",		"No-assign-to or no-change argument for preinc" },
	{ 10380,	"E_PREDEC1",		"No-copy-to or no-change argument for octet predec" },
	{ 10381,	"E_PREDEC2",		"Non-variable argument for predec" },
	{ 10382,	"E_PREDEC3",		"No-assign-to or no-change argument for predec" },
	{ 10383,	"E_POSTINC1",		"No-copy-to or no-change argument for octet postinc" },
	{ 10384,	"E_POSTINC2",		"Non-variable argument for postinc" },
	{ 10385,	"E_POSTINC3",		"No-assign-to or no-change argument for postinc" },
	{ 10386,	"E_POSTDEC1",		"No-copy-to or no-change argument for octet postdec" },
	{ 10387,	"E_POSTDEC2",		"Non-variable argument for postdec" },
	{ 10388,	"E_POSTDEC3",		"No-assign-to or no-change argument for postdec" },
	{ 10389,	"E_INIT1",		"Error-type structure for initialization" },
	{ 10390,	"E_INIT2",		"No-copy-to structure for initialization" },
	{ 10391,	"E_INIT3",		"Too many initializer values" },
	{ 10392,	"E_INIT4",		"Attempt to initialize freed named block" },
	{ 10393,	"E_INIT5",		"Bad structure type for initialization" },
	{ 10394,	"E_INIT6",		"No-assign-to element for initialization" },
	{ 10395,	"E_INIT7",		"No-change element for initialization" },
	{ 10396,	"E_INIT8",		"No-type-change element for initialization" },
	{ 10397,	"E_INIT9",		"No-error-value element for initialization" },
	{ 10398,	"E_INIT10",		"No-assign-or-copy-from source for initialization" },
	{ 10399,	"E_LIST1",		"No-relocate for list insert" },
	{ 10400,	"E_LIST2",		"No-relocate for list delete" },
	{ 10401,	"E_LIST3",		"No-relocate for list push" },
	{ 10402,	"E_LIST4",		"No-relocate for list append" },
	{ 10403,	"E_LIST5",		"No-relocate for list pop" },
	{ 10404,	"E_LIST6",		"No-relocate for list remove" },
	{ 10405,	"E_MODIFY1",		"Non-variable first argument for modify" },
	{ 10406,	"E_MODIFY2",		"Non-string second argument for modify" },
	{ 10407,	"E_MODIFY3",		"No-change first argument for modify" },
	{ 10408,	"E_MODIFY4",		"Undefined function for modify" },
	{ 10409,	"E_MODIFY5",		"Unacceptable type first argument for modify" },
	{ 10410,	"E_FPATHOPEN1",		"Non-string arguments for fpathopen" },
	{ 10411,	"E_FPATHOPEN2",		"Unrecognized mode for fpathopen" },
	{ 10412,	"E_LOG1",		"Bad epsilon argument for log" },
	{ 10413,	"E_LOG2",		"Non-numeric first argument for log" },
	{ 10414,	"E_LOG3",		"Cannot calculate log for this value" },
	{ 10415,	"E_FGETFILE1",		"Non-file argument for fgetfile" },
	{ 10416,	"E_FGETFILE2",		"File argument for fgetfile not open for reading" },
	{ 10417,	"E_FGETFILE3",		"Unable to set file position in fgetfile" },
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
	{ 10435,	"E_TAN3",		"UNUSED ERROR: Invalid value for calculating the sin numerator for tan" },
	{ 10436,	"E_TAN4",		"UNUSED ERROR: Invalid value for calculating the cos denominator for tan" },
	{ 10437,	"E_COT3",		"UNUSED ERROR: Invalid value for calculating the sin numerator for cot" },
	{ 10438,	"E_COT4",		"UNUSED ERROR: Invalid value for calculating the cos denominator for cot" },
	{ 10439,	"E_SEC3",		"UNUSED ERROR: Invalid value for calculating the cos reciprocal for sec" },
	{ 10440,	"E_CSC3",		"UNUSED ERROR: Invalid value for calculating the sin reciprocal for csc" },
	{ 10441,	"E_TANH3",		"Invalid value for calculating the sinh numerator for tanh" },
	{ 10442,	"E_TANH4",		"Invalid value for calculating the cosh denominator for tanh" },
	{ 10443,	"E_COTH3",		"Invalid value for calculating the sinh numerator for coth" },
	{ 10444,	"E_COTH4",		"Invalid value for calculating the cosh denominator for coth" },
	{ 10445,	"E_SECH3",		"Invalid value for calculating the cosh reciprocal for sech" },
	{ 10446,	"E_CSCH3",		"Invalid value for calculating the sinh reciprocal for csch" },
	{ 10447,	"E_ASIN3",		"Invalid value for calculating asin" },
	{ 10448,	"E_ACOS3",		"Invalid value for calculating acos" },
	{ 10449,	"E_ASINH3",		"Invalid value for calculating asinh" },
	{ 10450,	"E_ACOSH3",		"Invalid value for calculating acosn" },
	{ 10451,	"E_ATAN3",		"Invalid value for calculating atan" },
	{ 10452,	"E_ACOT3",		"Invalid value for calculating acot" },
	{ 10453,	"E_ASEC3",		"Invalid value for calculating asec" },
	{ 10454,	"E_ACSC3",		"Invalid value for calculating acsc" },
	{ 10455,	"E_ATANH3",		"Invalid value for calculating atan" },
	{ 10456,	"E_ACOTH3",		"Invalid value for calculating acot" },
	{ 10457,	"E_ASECH3",		"Invalid value for calculating asec" },
	{ 10458,	"E_ACSCH3",		"Invalid value for calculating acsc" },
	{ 10459,	"E_D2R1",		"Bad epsilon for converting degrees to radians" },
	{ 10460,	"E_D2R2",		"Bad first argument converting degrees to radians" },
	{ 10461,	"E_R2D1",		"Bad epsilon for converting radians to degrees" },
	{ 10462,	"E_R2D2",		"Bad first argument converting radians to degrees" },
	{ 10463,	"E_G2R1",		"Bad epsilon for converting gradians to radians" },
	{ 10464,	"E_G2R2",		"Bad first argument converting gradians to radians" },
	{ 10465,	"E_R2G1",		"Bad epsilon for converting radians to gradians" },
	{ 10466,	"E_R2G2",		"Bad first argument converting radians to gradians" },
	{ 10467,	"E_D2G1",		"Bad first argument converting degrees to gradians" },
	{ 10468,	"E_G2D1",		"Bad first argument converting gradians to degrees" },
	{ 10469,	"E_D2DMS1",		"Non-lvalue arguments 2, 3 or 4 for d2dms" },
	{ 10470,	"E_D2DMS2",		"Non-real-number arg 1 for d2dms" },
	{ 10471,	"E_D2DMS3",		"No-assign-to argument 2, 3 or 4 for d2dms" },
	{ 10472,	"E_D2DMS4",		"Invalid rounding arg 5 for d2dms" },
	{ 10473,	"E_D2DM1",		"Non-lvalue arguments 2 or 3 for d2dm" },
	{ 10474,	"E_D2DM2",		"Non-real-number arg 1 for d2dm" },
	{ 10475,	"E_D2DM3",		"No-assign-to argument 2 or 3 for d2dm" },
	{ 10476,	"E_D2DM4",		"Invalid rounding arg 4 for d2dm" },
	{ 10477,	"E_G2GMS1",		"Non-lvalue arguments 2, 3 or 4 for g2gms" },
	{ 10478,	"E_G2GMS2",		"Non-real-number arg 1 for g2gms" },
	{ 10479,	"E_G2GMS3",		"No-assign-to argument 2 or 3 for g2gms" },
	{ 10480,	"E_G2GMS4",		"Invalid rounding arg 5 for g2gms" },
	{ 10481,	"E_G2GM1",		"Non-lvalue arguments 2 or 3 for g2gm" },
	{ 10482,	"E_G2GM2",		"Non-real-number arg 1 for g2gm" },
	{ 10483,	"E_G2GM3",		"No-assign-to argument 2, 3 or 4 for g2gm" },
	{ 10484,	"E_G2GM4",		"Invalid rounding arg 4 for g2gm" },
	{ 10485,	"E_H2HMS1",		"Non-lvalue arguments 2, 3 or 4 for h2hms" },
	{ 10486,	"E_H2HMS2",		"Non-real-number arg 1 for h2hms" },
	{ 10487,	"E_H2HMS3",		"No-assign-to argument 2, 3 or 4 for h2hms" },
	{ 10488,	"E_H2HMS4",		"Invalid rounding arg 5 for h2hms" },
	{ 10489,	"E_H2HM1",		"Non-lvalue arguments 2 or 3 for h2hm" },
	{ 10490,	"E_H2HM2",		"Non-real-number arg 1 for h2hm" },
	{ 10491,	"E_H2HM3",		"No-assign-to argument 2 or 3 for h2hm" },
	{ 10492,	"E_H2HM4",		"Invalid rounding arg 4 for h2hm" },
	{ 10493,	"E_DMS2D1",		"Non-real-number arguments 1, 2 or 3 for dms2d" },
	{ 10494,	"E_DMS2D2",		"Invalid rounding arg 4 for dms2d" },
	{ 10495,	"E_DM2D1",		"Non-real-number arguments 1 or 2 for dm2d" },
	{ 10496,	"E_DM2D2",		"Invalid rounding arg 4 for dm2d" },
	{ 10497,	"E_GMS2G1",		"Non-real-number arguments 1, 2 or 3 for gms2g" },
	{ 10498,	"E_GMS2G2",		"Invalid rounding arg 4 for gms2g" },
	{ 10499,	"E_GM2G1",		"Non-real-number arguments 1 or 2 for gm2g" },
	{ 10500,	"E_GM2G2",		"Invalid rounding arg 4 for gm2g" },
	{ 10501,	"E_HMS2H1",		"Non-real-number arguments 1, 2 or 3 for hms2h" },
	{ 10502,	"E_HMS2H2",		"Invalid rounding arg 4 for hms2h" },
	{ 10503,	"E_HM2H1",		"UNUSED ERROR: Non-real-number arguments 1 or 2 for hm2h" },
	{ 10504,	"E_HM2H2",		"UNUSED ERROR: Invalid rounding arg 4 for hm2h" },
	{ 10505,	"E_LOG2_1",		"Bad epsilon argument for log2" },
	{ 10506,	"E_LOG2_2",		"Non-numeric first argument for log2" },
	{ 10507,	"E_LOG2_3",		"Cannot calculate log2 for this value" },
	{ 10508,	"E_LOGN_1",		"Bad epsilon argument for logn" },
	{ 10509,	"E_LOGN_2",		"Non-numeric first argument for logn" },
	{ 10510,	"E_LOGN_3",		"Cannot calculate logn for this value" },
	{ 10511,	"E_LOGN_4",		"Cannot calculate logn for this log base" },
	{ 10512,	"E_LOGN_5",		"Non-numeric second argument for logn" },
	{ 10513,	"E_VERSIN1",		"Bad epsilon for versin" },
	{ 10514,	"E_VERSIN2",		"Bad first argument for versin" },
	{ 10515,	"E_VERSIN3",		"Too-large im(argument) for versin" },
	{ 10516,	"E_AVERSIN1",		"Bad epsilon for aversin" },
	{ 10517,	"E_AVERSIN2",		"Bad first argument for aversin" },
	{ 10518,	"E_AVERSIN3",		"Too-large im(argument) for aversin" },
	{ 10519,	"E_COVERSIN1",		"Bad epsilon for coversin" },
	{ 10520,	"E_COVERSIN2",		"Bad first argument for coversin" },
	{ 10521,	"E_COVERSIN3",		"Too-large im(argument) for coversin" },
	{ 10522,	"E_ACOVERSIN1",		"Bad epsilon for acoversin" },
	{ 10523,	"E_ACOVERSIN2",		"Bad first argument for acoversin" },
	{ 10524,	"E_ACOVERSIN3",		"Too-large im(argument) for acoversin" },
	{ 10525,	"E_VERCOS1",		"Bad epsilon for vercos" },
	{ 10526,	"E_VERCOS2",		"Bad first argument for vercos" },
	{ 10527,	"E_VERCOS3",		"Too-large im(argument) for vercos" },
	{ 10528,	"E_AVERCOS1",		"Bad epsilon for avercos" },
	{ 10529,	"E_AVERCOS2",		"Bad first argument for avercos" },
	{ 10530,	"E_AVERCOS3",		"Too-large im(argument) for avercos" },
	{ 10531,	"E_COVERCOS1",		"Bad epsilon for covercos" },
	{ 10532,	"E_COVERCOS2",		"Bad first argument for covercos" },
	{ 10533,	"E_COVERCOS3",		"Too-large im(argument) for covercos" },
	{ 10534,	"E_ACOVERCOS1",		"Bad epsilon for acovercos" },
	{ 10535,	"E_ACOVERCOS2",		"Bad first argument for acovercos" },
	{ 10536,	"E_ACOVERCOS3",		"Too-large im(argument) for acovercos" },
	{ 10537,	"E_TAN5",		"Invalid complex argument for tan" },
	{ 10538,	"E_COT5",		"Invalid zero argument for cot" },
	{ 10539,	"E_COT6",		"Invalid complex argument for cot" },
	{ 10540,	"E_SEC5",		"Invalid complex argument for sec" },
	{ 10541,	"E_CSC5",		"Invalid zero argument for cot" },
	{ 10542,	"E_CSC6",		"Invalid complex argument for csc" },
	/* add new entries above here and be sure their errnum numeric value is consecutive */

	/* The next NULL entry must be last */
	{ -1,		NULL,			NULL }
};



/*
 * The code below is used to form the errcode executable and is NOT part of libcalc
 */
#if defined(ERRCODE_SRC)


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


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
	" >=10\t\terror_table is invalid\n";


char *program = "((NULL))";		/* our name */


/*
 * verify_error_table - verify the consistency of the error_table
 *
 * The errnum must start with E__BASE and be consecutive until the final entry.
 *
 * The errsym must be a non-NULL string that matches regular expression: ^E_[A-Z0-9_]+$ until the last entry.
 *
 * The errnsf must be a non-NULL non-empty string until the last entry.
 *
 * The final entry must have an errnum of -1, errsym of NULL and errmsg of NULL.
 *
 * This function does not return on error or if the error_table is invalid.
 */
S_FUNC void
verify_error_table(void)
{
	size_t len;		/* length of the error_table */
	size_t e__count;	/* computed ECOUNT value */
	char *p;
	size_t i;

	/*
	 * verify error_table size
	 */
	len = sizeof(error_table) / sizeof(error_table[0]);
	e__count = len - 2;
	if (e__count != MY_ECOUNT) {
		fprintf(stderr, "**** %s ERROR: error_table length: %zu != MY_ECOUNT+2: %lu\n",
			program, len, MY_ECOUNT+2);
		exit(10);
	}

	/*
	 * verify the initial errnum of the 1st error_table entry
	 */
	if (error_table[0].errnum != E__BASE) {
		fprintf(stderr, "**** %s ERROR: initial entry error_table[0].errnum: %d != E__BASE: %d\n",
			program, error_table[0].errnum, E__BASE);
		exit(11);
	}
	if (error_table[0].errsym == NULL) {
		fprintf(stderr, "**** %s ERROR: error_table[0].errsym is NULL\n",
			program);
		exit(12);
	}
	if (strcmp(error_table[0].errsym, "E__BASE") != 0) {
		fprintf(stderr, "**** %s ERROR: error_table[0].errsym: %s != E__BASE\n",
			program, error_table[0].errsym);
		exit(13);
	}
	if (error_table[0].errmsg == NULL) {
		fprintf(stderr, "**** %s ERROR: error_table[0].errmsg is NULL\n",
			program);
		exit(14);
	}
	if (strlen(error_table[0].errmsg) <= 0) {
		fprintf(stderr, "**** %s ERROR: error_table[0].errmsg length: %zu must be > 0\n",
			program, strlen(error_table[0].errmsg));
		exit(15);
	}

	/*
	 * verify 2nd thru end to last entry of error_table
	 */
	for (i=1; i < len-1; ++i) {

		/*
		 * check errnum - must be consecutive beyond E__BASE
		 */
		if ((unsigned long)(error_table[i].errnum) != E__BASE+i) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errnum: %d != %ld\n",
				program, i, error_table[i].errnum, E__BASE+i);
			exit(16);
		}

		/*
		 * check errsym is a non-NULL string matching the regular expression: ^E_[A-Z0-9_]+$
		 */
		if (error_table[i].errsym == NULL) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errsym is NULL\n",
				program, i);
			exit(17);
		}
		if (strlen(error_table[i].errsym) < 3) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errsym length: %zu must be >= 3\n",
				program, i, strlen(error_table[i].errsym));
			exit(18);
		}
		if (strncmp(error_table[i].errsym, "E_", 2) != 0) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errsym: %s must start with E_\n",
				program, i, error_table[i].errsym);
			exit(19);
		}
		p = error_table[i].errsym+2;
		if (!isascii(*p) || !isupper(*p)) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errsym: %s "
					"3rd E_STRING char is not UPPER letter, "
					"must match the regular expression: %s\n",
				program, i, error_table[i].errsym, "^E_[A-Z][A-Z0-9_]+$");
			exit(20);
		}
		for (p = error_table[i].errsym+3; *p != '\0'; ++p) {
			if (!isascii(*p) || !(isupper(*p) || isdigit(*p) || *p == '_')) {
				fprintf(stderr, "**** %s ERROR: error_table[%zu].errsym: %s "
						"E_STRING must match the regular expression: %s\n",
					program, i, error_table[i].errsym, "^E_[A-Z0-9_]+$");
				exit(21);
			}
		}

		/*
		 * verify errmsg is not empty
		 */
		if (strlen(error_table[i].errmsg) <= 0) {
			fprintf(stderr, "**** %s ERROR: error_table[%zu].errmsg length: %zu must be > 0\n",
				program, i, strlen(error_table[i].errmsg));
			exit(22);
		}
	}

	/*
	 * verify the last error_table entry
	 */
	if (error_table[len-1].errnum != -1) {
		fprintf(stderr, "**** %s ERROR: final NULL entry error_table[%zu].errnum: %d != -1\n",
			program, len-1, error_table[len-1].errnum);
		exit(23);
	}
	if (error_table[len-1].errsym != NULL) {
		fprintf(stderr, "**** %s ERROR: final NULL entry error_table[%zu].errsym != NULL\n",
			program, len-1);
		exit(24);
	}
	if (error_table[len-1].errmsg != NULL) {
		fprintf(stderr, "**** %s ERROR: final NULL entry error_table[%zu].errmsg != NULL\n",
			program, len-1);
		exit(25);
	}
	return;
}


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
	      "\"E__HIGHEST\"\t%ld\thighest assigned calc computation error code\n"
	      "\"E__USERDEF\"\t%d\tuser defined error codes start here\n"
	      "\"E__USERMAX\"\t%d\tmaximum user defined error code\n",
	      MY_E__HIGHEST, E__USERDEF, E__USERMAX);
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
	       "#define E__HIGHEST\t%ld\t/* highest assigned calc computation error code */\n"
	       "\n"
	       "#define ECOUNT\t%ld\t/* number of calc computation error codes w/o E__BASE */\n",
	       MY_E__HIGHEST, MY_ECOUNT);
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
			exit(2);
		case 'e':
			e_flag = 1;
			break;
		case 'd':
			d_flag = 1;
			break;
		default:
		    fprintf(stderr, "**** %s ERROR: invalid command line\n", program);
			fprintf(stderr, usage, program);
			exit(3);
		}
	}
	if (e_flag && d_flag) {
		fprintf(stderr, "**** %s ERROR: -e and -d conflict\n", program);
		fprintf(stderr, usage, program);
		exit(3);
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
	exit(0);
}


#endif /* ERRCODE_SRC */
