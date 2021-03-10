/*
 * zio - scanf and printf routines for arbitrary precision integers
 *
 * Copyright (C) 1999-2007,2021  David I. Bell
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
 * Under source code control:	1993/07/30 19:42:48
 * File existed as early as:	1993
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include "alloc.h"
#include "config.h"
#include "zmath.h"
#include "args.h"


#include "banned.h"	/* include after system header <> includes */


#define OUTBUFSIZE	200		/* realloc size for output buffers */

#define PUTCHAR(ch)		math_chr(ch)
#define PUTSTR(str)		math_str(str)
#define PRINTF1(fmt, a1)		math_fmt(fmt, a1)
#define PRINTF2(fmt, a1, a2)		math_fmt(fmt, a1, a2)
#define PRINTF3(fmt, a1, a2, a3)	math_fmt(fmt, a1, a2, a3)
#define PRINTF4(fmt, a1, a2, a3, a4)	math_fmt(fmt, a1, a2, a3, a4)


/*
 * Output state that has been saved when diversions are done.
 */
typedef struct iostate IOSTATE;
struct iostate {
	IOSTATE *oldiostates;		/* previous saved state */
	long outdigits;			/* digits for output */
	int outmode;			/* output mode */
	int outmode2;			/* secondary output mode */
	FILE *outfp;			/* file unit for output (if any) */
	char *outbuf;			/* output string buffer (if any) */
	size_t outbufsize;		/* current size of string buffer */
	size_t outbufused;		/* space used in string buffer */
	BOOL outputisstring;		/* TRUE if output is to string buffer */
};


STATIC IOSTATE	*oldiostates = NULL;	/* list of saved output states */
STATIC FILE	*outfp = NULL;		/* file unit for output */
STATIC char	*outbuf = NULL;		/* current diverted buffer */
STATIC BOOL	outputisstring = FALSE;
STATIC size_t	outbufsize;
STATIC size_t	outbufused;


/*
 * zio_init - perform needed initialization work
 *
 * On some systems, one cannot initialize a pointer to a FILE *.
 * This routine, called once at startup is a work-a-round for
 * systems with such bogons.
 */
void
zio_init(void)
{
	STATIC int done = 0;	/* 1 => routine already called */

	if (!done) {
		outfp = stdout;
		done = 1;
	}
}


/*
 * Routine to output a character either to a FILE
 * handle or into a string.
 */
void
math_chr(int ch)
{
	char	*cp;

	if (!outputisstring) {
		fputc(ch, outfp);
		return;
	}
	if (outbufused >= outbufsize) {
		cp = (char *)realloc(outbuf, outbufsize + OUTBUFSIZE + 1);
		if (cp == NULL) {
			math_error("Cannot realloc output string");
			/*NOTREACHED*/
		}
		outbuf = cp;
		outbufsize += OUTBUFSIZE;
	}
	outbuf[outbufused++] = (char)ch;
}


/*
 * Routine to output a null-terminated string either
 * to a FILE handle or into a string.
 */
void
math_str(char *str)
{
	char	*cp;
	size_t	len;

	if (!outputisstring) {
		fputs(str, outfp);
		return;
	}
	len = strlen(str);
	if ((outbufused + len) > outbufsize) {
		cp = (char *)realloc(outbuf, outbufsize + len + OUTBUFSIZE + 1);
		if (cp == NULL) {
			math_error("Cannot realloc output string");
			/*NOTREACHED*/
		}
		outbuf = cp;
		outbufsize += (len + OUTBUFSIZE);
	}
	memcpy(&outbuf[outbufused], str, len);
	outbufused += len;
}


/*
 * Output a null-terminated string either to a FILE handle or into a string,
 * padded with spaces as needed so as to fit within the specified width.
 * If width is positive, the spaces are added at the front of the string.
 * If width is negative, the spaces are added at the end of the string.
 * The complete string is always output, even if this overflows the width.
 * No characters within the string are handled specially.
 */
void
math_fill(char *str, long width)
{
	if (width > 0) {
		width -= (long)strlen(str);
		while (width-- > 0)
			PUTCHAR(' ');
		PUTSTR(str);
	} else {
		width += (long)strlen(str);
		PUTSTR(str);
		while (width++ < 0)
			PUTCHAR(' ');
	}
}


/*
 * Routine to output a printf-style formatted string either
 * to a FILE handle or into a string.
 */
void
math_fmt(char *fmt, ...)
{
	va_list ap;
	char buf[BUFSIZ+1];

	va_start(ap, fmt);
	vsnprintf(buf, BUFSIZ, fmt, ap);
	va_end(ap);
	buf[BUFSIZ] = '\0';	/* paranoia */
	math_str(buf);
}


/*
 * Flush the current output stream.
 */
void
math_flush(void)
{
	if (!outputisstring)
		fflush(outfp);
}


/*
 * Divert further output so that it is saved into a string that will be
 * returned later when the diversion is completed.  The current state of
 * output is remembered for later restoration.	Diversions can be nested.
 * Output diversion is only intended for saving output to "stdout".
 */
void
math_divertio(void)
{
	register IOSTATE *sp;

	sp = (IOSTATE *) malloc(sizeof(IOSTATE));
	if (sp == NULL) {
		math_error("No memory for diverting output");
		/*NOTREACHED*/
	}
	sp->oldiostates = oldiostates;
	sp->outdigits = conf->outdigits;
	sp->outmode = conf->outmode;
	sp->outmode2 = conf->outmode2;
	sp->outfp = outfp;
	sp->outbuf = outbuf;
	sp->outbufsize = outbufsize;
	sp->outbufused = outbufused;
	sp->outputisstring = outputisstring;

	outbufused = 0;
	outbufsize = 0;
	outbuf = (char *) malloc(OUTBUFSIZE + 1);
	if (outbuf == NULL) {
		math_error("Cannot allocate divert string");
		/*NOTREACHED*/
	}
	outbufsize = OUTBUFSIZE;
	outputisstring = TRUE;
	oldiostates = sp;
}


/*
 * Undivert output and return the saved output as a string.  This also
 * restores the output state to what it was before the diversion began.
 * The string needs freeing by the caller when it is no longer needed.
 */
char *
math_getdivertedio(void)
{
	register IOSTATE *sp;
	char *cp;

	sp = oldiostates;
	if (sp == NULL) {
		math_error("No diverted state to restore");
		/*NOTREACHED*/
	}
	cp = outbuf;
	cp[outbufused] = '\0';
	oldiostates = sp->oldiostates;
	conf->outdigits = sp->outdigits;
	conf->outmode = sp->outmode;
	conf->outmode2 = sp->outmode2;
	outfp = sp->outfp;
	outbuf = sp->outbuf;
	outbufsize = sp->outbufsize;
	outbufused = sp->outbufused;
	outbuf = sp->outbuf;
	outputisstring = sp->outputisstring;
	free(sp);
	return cp;
}


/*
 * Clear all diversions and set output back to the original destination.
 * This is called when resetting the global state of the program.
 */
void
math_cleardiversions(void)
{
	while (oldiostates)
		free(math_getdivertedio());
}


/*
 * Set the output routines to output to the specified FILE stream.
 * This interacts with output diversion in the following manner.
 *	STDOUT	diversion	action
 *	----	---------	------
 *	yes	yes		set output to diversion string again.
 *	yes	no		set output to stdout.
 *	no	yes		set output to specified file.
 *	no	no		set output to specified file.
 */
void
math_setfp(FILE *newfp)
{
	outfp = newfp;
	outputisstring = (oldiostates && (newfp == stdout));
}


/*
 * Set the output mode for numeric output.
 * This also returns the previous mode.
 */
int
math_setmode(int newmode)
{
	int oldmode;

	if ((newmode <= MODE_DEFAULT) || (newmode > MODE_MAX)) {
		math_error("Setting illegal output mode");
		/*NOTREACHED*/
	}
	oldmode = conf->outmode;
	conf->outmode = newmode;
	return oldmode;
}


/*
 * Set the secondary output mode for numeric output.
 * This also returns the previous mode.
 */
int
math_setmode2(int newmode)
{
	int oldmode;

	if (newmode != MODE2_OFF && ((newmode <= MODE_DEFAULT) ||
	    (newmode > MODE_MAX))) {
		math_error("Setting illegal secondary output mode");
		/*NOTREACHED*/
	}
	oldmode = conf->outmode2;
	conf->outmode2 = newmode;
	return oldmode;
}


/*
 * Set the number of digits for float or exponential output.
 * This also returns the previous number of digits.
 */
LEN
math_setdigits(LEN newdigits)
{
	LEN olddigits;

	if (newdigits < 0) {
		math_error("Setting illegal number of digits");
		/*NOTREACHED*/
	}
	olddigits = conf->outdigits;
	conf->outdigits = newdigits;
	return olddigits;
}


/*
 * Print an integer value as a hex number.
 * Width is the number of columns to print the number in, including the
 * sign if required.  If zero, no extra output is done.	 If positive,
 * leading spaces are typed if necessary. If negative, trailing spaces are
 * typed if necessary.	The special characters 0x appear to indicate the
 * number is hex.
 */
/*ARGSUSED*/
void
zprintx(ZVALUE z, long width)
{
	register HALF *hp;	/* current word to print */
	int len;		/* number of halfwords to type */
	char *str;

	if (width) {
		math_divertio();
		zprintx(z, 0L);
		str = math_getdivertedio();
		math_fill(str, width);
		free(str);
		return;
	}
	len = z.len - 1;
	if (zisneg(z))
		PUTCHAR('-');
	if ((len == 0) && (*z.v <= (HALF) 9)) {
		len = '0' + (int)(*z.v);
		PUTCHAR(len & 0xff);
		return;
	}
	hp = z.v + len;
#if BASEB == 32
	PRINTF1("0x%lx", (PRINT) *hp--);
	while (--len >= 0) {
		PRINTF1("%08lx", (PRINT) *hp--);
	}
#else /* BASEB == 32 */
	PRINTF1("0x%lx", (FULL) *hp--);
	while (--len >= 0) {
		PRINTF1("%04lx", (FULL) *hp--);
	}
#endif /* BASEB == 32 */
}


/*
 * Print an integer value as a binary number.
 * The special characters 0b appear to indicate the number is binary.
 */
/*ARGSUSED*/
void
zprintb(ZVALUE z, long width)
{
	register HALF *hp;	/* current word to print */
	int len;		/* number of halfwords to type */
	HALF val;		/* current value */
	HALF mask;		/* current mask */
	int didprint;		/* nonzero if printed some digits */
	int ch;			/* current char */
	char *str;

	if (width) {
		math_divertio();
		zprintb(z, 0L);
		str = math_getdivertedio();
		math_fill(str, width);
		free(str);
		return;
	}
	len = z.len - 1;
	if (zisneg(z))
		PUTCHAR('-');
	if ((len == 0) && (*z.v <= (FULL) 1)) {
		len = '0' + (int)(*z.v);
		PUTCHAR(len & 0xff);
		return;
	}
	hp = z.v + len;
	didprint = 0;
	PUTSTR("0b");
	while (len-- >= 0) {
		val = ((len >= 0) ? *hp-- : *hp);
		mask = ((HALF)1 << (BASEB - 1));
		while (mask) {
			ch = '0' + ((mask & val) != 0);
			if (didprint || (ch != '0')) {
				PUTCHAR(ch & 0xff);
				didprint = 1;
			}
			mask >>= 1;
		}
	}
}


/*
 * Print an integer value as an octal number.
 * The number begins with a leading 0 to indicate that it is octal.
 */
/*ARGSUSED*/
void
zprinto(ZVALUE z, long width)
{
	register HALF *hp;	/* current word to print */
	int len;		/* number of halfwords to type */
#if BASEB == 32			/* Yes, the larger base needs a smaller type! */
	HALF num1='0';		/* numbers to type */
	HALF num2=(HALF)0;	/* numbers to type */
	HALF num3;		/* numbers to type */
	HALF num4;		/* numbers to type */
#else
	FULL num1='0';		/* numbers to type */
	FULL num2=(FULL)0;	/* numbers to type */
#endif
	int rem;		/* remainder number of halfwords */
	char *str;

	if (width) {
		math_divertio();
		zprinto(z, 0L);
		str = math_getdivertedio();
		math_fill(str, width);
		free(str);
		return;
	}
	if (zisneg(z))
		PUTCHAR('-');
	len = z.len;
	if ((len == 1) && (*z.v <= (FULL) 7)) {
		num1 = '0' + (int)(*z.v);
		PUTCHAR((int)(num1 & 0xff));
		return;
	}
	hp = z.v + len - 1;
	rem = len % 3;
#if BASEB == 32
	switch (rem) {	/* handle odd amounts first */
	case 0:
		num1 = ((hp[0]) >> 8);
		num2 = (((hp[0] & 0xff) << 16) + (hp[-1] >> 16));
		num3 = (((hp[-1] & 0xffff) << 8) + (hp[-2] >> 24));
		num4 = (hp[-2] & 0xffffff);
		if (num1) {
			PRINTF4("0%lo%08lo%08lo%08lo",
			    (PRINT) num1, (PRINT) num2,
			    (PRINT) num3, (PRINT) num4);
		} else {
			PRINTF3("0%lo%08lo%08lo",
			    (PRINT) num2, (PRINT) num3, (PRINT) num4);
		}
		rem = 3;
		break;
	case 1:
		PRINTF1("0%lo", (PRINT) hp[0]);
		break;
	case 2:
		num1 = ((hp[0]) >> 16);
		num2 = (((hp[0] & 0xffff) << 8) + (hp[-1] >> 24));
		num3 = (hp[-1] & 0xffffff);
		if (num1) {
			PRINTF3("0%lo%08lo%08lo",
			    (PRINT) num1, (PRINT) num2, (PRINT) num3);
		} else {
			PRINTF2("0%lo%08lo", (PRINT) num2, (PRINT) num3);
		}
		break;
	}
	len -= rem;
	if (len > 0) {
		hp -= rem;
		while (len > 0) {	/* finish in groups of 3 words */
			PRINTF4("%08lo%08lo%08lo%08lo",
			    (PRINT) ((hp[0]) >> 8),
			    (PRINT) (((hp[0] & 0xff) << 16) + (hp[-1] >> 16)),
			    (PRINT) (((hp[-1] & 0xffff) << 8) + (hp[-2] >> 24)),
			    (PRINT) (hp[-2] & 0xffffff));
			hp -= 3;
			len -= 3;
		}
	}
#else
	switch (rem) {	/* handle odd amounts first */
	case 0:
		num1 = ((((FULL) hp[0]) << 8) + (((FULL) hp[-1]) >> 8));
		num2 = ((((FULL) (hp[-1] & 0xff)) << 16) + ((FULL) hp[-2]));
		rem = 3;
		break;
	case 1:
		num1 = 0;
		num2 = (FULL) hp[0];
		break;
	case 2:
		num1 = (((FULL) hp[0]) >> 8);
		num2 = ((((FULL) (hp[0] & 0xff)) << 16) + ((FULL) hp[-1]));
		break;
	}
	if (num1) {
		PRINTF2("0%lo%08lo", num1, num2);
	} else {
		PRINTF1("0%lo", num2);
	}
	len -= rem;
	if (len > 0) {
		hp -= rem;
		while (len > 0) {	/* finish in groups of 3 halfwords */
			PRINTF2("%08lo%08lo",
			    ((((FULL) hp[0]) << 8) + (((FULL) hp[-1]) >> 8)),
			    ((((FULL) (hp[-1] & 0xff))<<16) + ((FULL) hp[-2])));
			hp -= 3;
			len -= 3;
		}
	}
#endif
}


/*
 * Print a decimal integer to the terminal.
 * This works by dividing the number by 10^2^N for some N, and
 * then doing this recursively on the quotient and remainder.
 * Decimals supplies number of decimal places to print, with a decimal
 * point at the right location, with zero meaning no decimal point.
 * Width is the number of columns to print the number in, including the
 * decimal point and sign if required.	If zero, no extra output is done.
 * If positive, leading spaces are typed if necessary. If negative, trailing
 * spaces are typed if necessary.  As examples of the effects of these values,
 * (345,0,0) = "345", (345,2,0) = "3.45", (345,5,8) = "	 .00345".
 *
 * given:
 *	z		number to be printed
 *	decimals	number of decimal places
 *	width		number of columns to print in
 */
void
zprintval(ZVALUE z, long decimals, long width)
{
	int depth;		/* maximum depth */
	int n;			/* current index into array */
	long i;			/* number to print */
	long leadspaces;	/* number of leading spaces to print */
	long putpoint;		/* digits until print decimal point */
	long digits;		/* number of digits of raw number */
	BOOL output;		/* TRUE if have output something */
	BOOL neg;		/* TRUE if negative */
	ZVALUE quo, rem;	/* quotient and remainder */
	ZVALUE leftnums[32];	/* left parts of the number */
	ZVALUE rightnums[32];	/* right parts of the number */

	if (decimals < 0)
		decimals = 0;
	if (width < 0)
		width = 0;
	neg = (z.sign != 0);

	leadspaces = width - neg - (decimals > 0);
	z.sign = 0;
	/*
	 * Find the 2^N power of ten which is greater than or equal
	 * to the number, calculating it the first time if necessary.
	 */
	_tenpowers_[0] = _ten_;
	depth = 0;
	while ((_tenpowers_[depth].len < z.len) ||
	       (zrel(_tenpowers_[depth], z) <= 0)) {
		depth++;
		if (_tenpowers_[depth].len == 0) {
			if (depth <= TEN_MAX) {
				zsquare(_tenpowers_[depth-1],
					 &_tenpowers_[depth]);
			} else {
				math_error("cannot compute 10^2^(TEN_MAX+1)");
				/*NOTREACHED*/
			}
		}
	}
	/*
	 * Divide by smaller 2^N powers of ten until the parts are small
	 * enough to output.  This algorithm walks through a binary tree
	 * where each node is a piece of the number to print, and such that
	 * we visit left nodes first.  We do the needed recursion in line.
	 */
	digits = 1;
	output = FALSE;
	n = 0;
	putpoint = 0;
	rightnums[0].len = 0;
	leftnums[0] = z;
	for (;;) {
		while (n < depth) {
			i = depth - n - 1;
			zdiv(leftnums[n], _tenpowers_[i], &quo, &rem, 0);
			if (!ziszero(quo))
				digits += (1L << i);
			n++;
			leftnums[n] = quo;
			rightnums[n] = rem;
		}
		i = (long)(leftnums[n].v[0]);
		if (output || i || (n == 0)) {
			if (!output) {
				output = TRUE;
				if (decimals < digits)
					leadspaces -= digits;
				else
					leadspaces -= decimals+conf->leadzero;
				while (--leadspaces >= 0)
					PUTCHAR(' ');
				if (neg)
					PUTCHAR('-');
				if (decimals) {
					putpoint = (digits - decimals);
					if (putpoint <= 0) {
						if (conf->leadzero)
							PUTCHAR('0');
						PUTCHAR('.');
						while (++putpoint <= 0)
							PUTCHAR('0');
						putpoint = 0;
					}
				}
			}
			i += '0';
			PUTCHAR((int)(i & 0xff));
			if (--putpoint == 0)
				PUTCHAR('.');
		}
		while (rightnums[n].len == 0) {
			if (n <= 0)
				return;
			if (leftnums[n].len)
				zfree(leftnums[n]);
			n--;
		}
		zfree(leftnums[n]);
		leftnums[n] = rightnums[n];
		rightnums[n].len = 0;
	}
}


/*
 * Read an integer value in decimal, hex, octal, or binary.
 * Hex numbers are indicated by a leading "0x", binary with a leading "0b",
 * and octal by a leading "0".	Periods are skipped over, but any other
 * extraneous character stops the scan.
 */
void
str2z(char *s, ZVALUE *res)
{
	ZVALUE z, ztmp, digit;
	HALF digval;
	BOOL minus;
	long shift;

	minus = FALSE;
	shift = 0;
	if (*s == '+')
		s++;
	else if (*s == '-') {
		minus = TRUE;
		s++;
	}
	if (*s == '0') {		/* possibly hex, octal, or binary */
		s++;
		if ((*s >= '0') && (*s <= '7')) {
			shift = 3;
		} else if ((*s == 'x') || (*s == 'X')) {
			shift = 4;
			s++;
		} else if ((*s == 'b') || (*s == 'B')) {
			shift = 1;
			s++;
		}
	}
	digit.v = &digval;
	digit.len = 1;
	digit.sign = 0;
	z = _zero_;
	while (*s) {
		digval = *s++;
		if ((digval >= '0') && (digval <= '9'))
			digval -= '0';
		else if ((digval >= 'a') && (digval <= 'f') && shift)
			digval -= ('a' - 10);
		else if ((digval >= 'A') && (digval <= 'F') && shift)
			digval -= ('A' - 10);
		else if (digval == '.')
			continue;
		else
			break;
		if (shift)
			zshift(z, shift, &ztmp);
		else
			zmuli(z, 10L, &ztmp);
		zfree(z);
		zadd(ztmp, digit, &z);
		zfree(ztmp);
	}
	ztrim(&z);
	if (minus && !ziszero(z))
		z.sign = 1;
	*res = z;
}


void
fitzprint(ZVALUE z, long digits, long show)
{
	ZVALUE ztmp1, ztmp2;
	long i;

	if (digits <= show) {
		zprintval(z, 0, 0);
		return;
	}
	show /= 2;
	ztenpow(digits - show, &ztmp1);
	(void) zquo(z, ztmp1, &ztmp2, 1);
	zprintval(ztmp2, 0, 0);
	zfree(ztmp1);
	zfree(ztmp2);
	printf("...");
	ztenpow(show, &ztmp1);
	(void) zmod(z, ztmp1, &ztmp2, 0);
	i = zdigits(ztmp2);
	while (i++ < show)
		printf("0");
	zprintval(ztmp2, 0, 0);
	zfree(ztmp1);
	zfree(ztmp2);
}

/* END CODE */
