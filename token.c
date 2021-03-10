/*
 * token - read input file characters into tokens
 *
 * Copyright (C) 1999-2007,2017,2021  David I. Bell and Ernest Bowen
 *
 * Primary author:  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:25
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <setjmp.h>

#include "calc.h"
#include "alloc.h"
#include "token.h"
#include "str.h"
#include "args.h"
#include "lib_calc.h"


#include "banned.h"	/* include after system header <> includes */


#define isletter(ch)	((((ch) >= 'a') && ((ch) <= 'z')) || \
				(((ch) >= 'A') && ((ch) <= 'Z')))
#define isdigit(ch)	(((ch) >= '0') && ((ch) <= '9'))
#define issymbol(ch)	(isletter(ch) || isdigit(ch) || ((ch) == '_'))
#define isoctal(ch)	(((ch) >= '0') && ((ch) <= '7'))

#define STRBUFSIZE	1024


/*
 * Current token.
 */
STATIC struct {
	short t_type;		/* type of token */
	char *t_sym;		/* symbol name */
	long t_strindex;	/* index of string value */
	long t_numindex;	/* index of numeric value */
} curtoken;


STATIC BOOL rescan;		/* TRUE to reread current token */
STATIC BOOL newlines;		/* TRUE to return newlines as tokens */
STATIC BOOL allsyms;		/* TRUE if always want a symbol token */
STATIC STRINGHEAD strings;	/* list of constant strings */
STATIC char *numbuf;		/* buffer for numeric tokens */
STATIC long numbufsize;		/* current size of numeric buffer */

long errorcount = 0;			/* number of compilation errors */


/*
 * Table of keywords
 */
struct keyword {
	char *k_name;	/* keyword name */
	int k_token;	/* token number */
};

STATIC struct keyword keywords[] = {
	{"if",		T_IF},
	{"else",	T_ELSE},
	{"for",		T_FOR},
	{"while",	T_WHILE},
	{"do",		T_DO},
	{"continue",	T_CONTINUE},
	{"break",	T_BREAK},
	{"goto",	T_GOTO},
	{"return",	T_RETURN},
	{"local",	T_LOCAL},
	{"global",	T_GLOBAL},
	{"static",	T_STATIC},
	{"switch",	T_SWITCH},
	{"case",	T_CASE},
	{"default",	T_DEFAULT},
	{"quit",	T_QUIT},
	{"exit",	T_QUIT},
	{"define",	T_DEFINE},
	{"read",	T_READ},
	{"show",	T_SHOW},
	{"help",	T_HELP},
	{"man",		T_HELP},
	{"write",	T_WRITE},
	{"mat",		T_MAT},
	{"obj",		T_OBJ},
	{"print",	T_PRINT},
	{"cd",		T_CD},
	{"undefine",	T_UNDEFINE},
	{"abort",	T_ABORT},
	{NULL,		0}
};


S_FUNC void eatcomment(void);
S_FUNC void eatstring(int quotechar);
S_FUNC void eatline(void);
S_FUNC int eatsymbol(void);
S_FUNC int eatnumber(void);


/*
 * Initialize all token information.
 */
void
inittokens(void)
{
	initstr(&strings);
	newlines = FALSE;
	allsyms = FALSE;
	rescan = FALSE;
	setprompt(conf->prompt1);
}


/*
 * Set the new token mode according to the specified flag, and return the
 * previous value of the flag.
 */
int
tokenmode(int flag)
{
	int	oldflag;

	oldflag = TM_DEFAULT;
	if (newlines)
		oldflag |= TM_NEWLINES;
	if (allsyms)
		oldflag |= TM_ALLSYMS;
	newlines = FALSE;
	allsyms = FALSE;
	if (flag & TM_NEWLINES)
		newlines = TRUE;
	if (flag & TM_ALLSYMS)
		allsyms = TRUE;
	setprompt(newlines ? conf->prompt1 : conf->prompt2);
	return oldflag;
}


/*
 * Routine to read in the next token from the input stream.
 * The type of token is returned as a value.  If the token is a string or
 * symbol name, information is saved so that the value can be retrieved.
 */
int
gettoken(void)
{
	int ch;			/* current input character */
	int type;		/* token type */

	if (rescan) {		/* rescanning */
		rescan = FALSE;
		return curtoken.t_type;
	}
	curtoken.t_sym = NULL;
	curtoken.t_strindex = 0;
	curtoken.t_numindex = 0;
	type = T_NULL;
	while (type == T_NULL) {
		ch = nextchar();
		if (allsyms && ch!=' ' && ch!=';' && ch!='"' &&
				ch!='\'' && ch!='\n' && ch!=EOF) {
			reread();
			type = eatsymbol();
			break;
		}
		switch (ch) {
		case ' ':
		case '\a':
		case '\b':
		case '\f':
		case '\r':
		case '\t':
		case '\v':
		case '\0':
			break;
		case '\n':
			if (newlines)
				type = T_NEWLINE;
			break;
		case EOF: type = T_EOF; break;
		case '{': type = T_LEFTBRACE; break;
		case '}': type = T_RIGHTBRACE; break;
		case '(': type = T_LEFTPAREN; break;
		case ')': type = T_RIGHTPAREN; break;
		case '[': type = T_LEFTBRACKET; break;
		case ']': type = T_RIGHTBRACKET; break;
		case ';': type = T_SEMICOLON; break;
		case ':': type = T_COLON; break;
		case ',': type = T_COMMA; break;
		case '?': type = T_QUESTIONMARK; break;
		case '@': type = T_AT; break;
		case '`': type = T_BACKQUOTE; break;
		case '$': type = T_DOLLAR; break;
		case '"':
		case '\'':
			type = T_STRING;
			eatstring(ch);
			break;
		case '^':
			switch (nextchar()) {
			case '=': type = T_POWEREQUALS; break;
			default: type = T_POWER; reread();
			}
			break;
		case '=':
			switch (nextchar()) {
			case '=': type = T_EQ; break;
			default: type = T_ASSIGN; reread();
			}
			break;
		case '+':
			switch (nextchar()) {
			case '+': type = T_PLUSPLUS; break;
			case '=': type = T_PLUSEQUALS; break;
			default: type = T_PLUS; reread();
			}
			break;
		case '-':
			switch (nextchar()) {
			case '-': type = T_MINUSMINUS; break;
			case '=': type = T_MINUSEQUALS; break;
			case '>': type = T_ARROW; break;
			default: type = T_MINUS; reread();
			}
			break;
		case '*':
			switch (nextchar()) {
			case '=': type = T_MULTEQUALS; break;
			case '*':
				switch (nextchar()) {
				case '=':
					type = T_POWEREQUALS; break;
				default:
					type = T_POWER; reread();
				}
				break;
			default: type = T_MULT; reread();
			}
			break;
		case '/':
			switch (nextchar()) {
			case '/':
				switch (nextchar()) {
				case '=':
					type = T_SLASHSLASHEQUALS;
					break;
				default:
					reread();
					type = T_SLASHSLASH;
					break;
				}
				break;
			case '=': type = T_DIVEQUALS; break;
			case '*': eatcomment(); break;
			default: type = T_DIV; reread();
			}
			break;
		case '%':
			switch (nextchar()) {
			case '=': type = T_MODEQUALS; break;
			default: type = T_MOD; reread();
			}
			break;
		case '<':
			switch (nextchar()) {
			case '=': type = T_LE; break;
			case '<':
				switch (nextchar()) {
				case '=':
					type = T_LSHIFTEQUALS;
					break;
				default:
					reread();
					type = T_LEFTSHIFT;
					break;
				}
				break;
			default: type = T_LT; reread();
			}
			break;
		case '>':
			switch (nextchar()) {
			case '=': type = T_GE; break;
			case '>':
				switch (nextchar()) {
				case '=':
					type = T_RSHIFTEQUALS;
					break;
				default:
					reread();
					type = T_RIGHTSHIFT;
					break;
				}
				break;
			default: type = T_GT; reread();
			}
			break;
		case '&':
			switch (nextchar()) {
			case '&': type = T_ANDAND; break;
			case '=': type = T_ANDEQUALS; break;
			default: type = T_AND; reread(); break;
			}
			break;
		case '|':
			switch (nextchar()) {
			case '|': type = T_OROR; break;
			case '=': type = T_OREQUALS; break;
			default: type = T_OR; reread(); break;
			}
			break;
		case '!':
			switch (nextchar()) {
			case '=': type = T_NE; break;
			default: type = T_NOT; reread(); break;
			}
			break;
		case '#':
			switch(nextchar()) {
			case '=': type = T_HASHEQUALS; break;
			case '!':
			case '#': eatline(); break;
			case '\n': reread(); break;
			default: type = T_HASH; reread();
			}
			break;
		case '~':
			switch (nextchar()) {
			case '=': type = T_TILDEEQUALS; break;
			default: type = T_TILDE; reread();
			}
			break;
		case '\\':
			switch (nextchar()) {
			case '\n': setprompt(conf->prompt2); break;
			case '=': type = T_BACKSLASHEQUALS; break;
			default: type = T_BACKSLASH; reread();
			}
			break;
		default:
			if (isletter(ch) || ch == '_') {
				reread();
				type = eatsymbol();
				break;
			}
			if (isdigit(ch) || (ch == '.')) {
				reread();
				type = eatnumber();
				break;
			}
			scanerror(T_NULL, "Unknown token character '%c'", ch);
		}
	}
	curtoken.t_type = (short)type;
	return type;
}


/*
 * Continue to eat up a comment string.
 * The leading slash-asterisk has just been scanned at this point.
 */
S_FUNC void
eatcomment(void)
{
	int ch;
	setprompt(conf->prompt2);
	for (;;) {
		ch = nextchar();
		if (ch == '*') {
			ch = nextchar();
			if (ch == '/')
				break;
			reread();
		}
		if (ch == EOF || ch == '\0') {
			scanerror(T_NULL, "Unterminated comment");
			reread();
			break;
		}
	}
	setprompt(conf->prompt1);
}


/*
 * Continue to eat up a the current line
 * Typically a #! will require the rest of the line to be eaten as if
 * it were a comment.
 */
S_FUNC void
eatline(void)
{
	int ch;		/* chars being eaten */

	do {
		ch = nextchar();
	} while (ch != '\n' && ch != EOF && ch != '\0');
	reread();
}


/*
 * Read in a string and add it to the literal string pool.
 * The leading single or double quote has been read in at this point.
 */
S_FUNC void
eatstring(int quotechar)
{
	register char *cp;	/* current character address */
	int ch, cch;		/* current character */
	int i;			/* index */
	char buf[STRBUFSIZE];	/* buffer for string */
	long len;		/* length in buffer */
	long totlen;		/* total length, including '\0' */
	char *str;
	BOOL done;

	str = buf;
	totlen = 0;
	done = FALSE;

	while (!done) {
	    cp = buf;
	    len = 0;
	    while (!done && len < STRBUFSIZE) {
		ch = nextchar();
		switch (ch) {
		case '\n':
			if (!newlines)
				break;
			/*FALLTHRU*/
		case EOF:
			reread();
			scanerror(T_NULL,
				  "Unterminated string constant");
			done = TRUE;
			ch = '\0';
			break;

		case '\\':
			ch = nextchar();
			if (isoctal(ch)) {
			    ch = ch - '0';
			    for (i = 2; i > 0; i--) {
				cch = nextchar();
				if (!isoctal(cch))
					break;
				ch = 8 * ch + cch - '0';
			    }
			    ch &= 0xff;
			    if (i > 0)
				reread();
			    break;
			}
			switch (ch) {
			case 'n': ch = '\n'; break;
			case 'r': ch = '\r'; break;
			case 't': ch = '\t'; break;
			case 'b': ch = '\b'; break;
			case 'f': ch = '\f'; break;
			case 'v': ch = '\v'; break;
			case 'a': ch = '\007'; break;
			case 'e': ch = '\033'; break;
			case '\n':
				setprompt(conf->prompt2);
				continue;
			case EOF:
				reread();
				continue;
			case 'x':
				ch = 0;
				for (i = 2; i > 0; i--) {
					cch = nextchar();
					if (isdigit(cch))
						ch = 16 * ch + cch - '0';
					else if (cch >= 'a' && cch <= 'f')
						ch = 16 * ch + 10 + cch - 'a';
					else if (cch >= 'A' && cch <= 'F')
						ch = 16 * ch + 10 + cch - 'A';
					else break;
				}
				if (i > 0)
					reread();
			}
			break;
		case '"':
		case '\'':
			if (ch == quotechar) {
				for (;;) {
					ch = nextchar();
					if (ch != ' ' && ch != '\t' &&
						(ch != '\n' ||
							newlines))
						break;
				}
				if (ch == '"' || ch == '\'') {
					quotechar = ch;
					continue;
				}
				reread();
				done = TRUE;
				ch = '\0';
			}
			break;
		    }

		*cp++ = (char) ch;
		len++;
	    }
	    if (!done || totlen) {
		if (totlen)
			str = (char *) realloc(str, totlen + len);
		else
			str = (char *) malloc(len);
		if (str == NULL) {
			math_error("Out of memory for reading tokens");
				/*NOTREACHED*/
		}
		memcpy(str + totlen, buf, len);
		totlen += len;
		len = 0;
	    }
	}
	curtoken.t_strindex = addstring(str, totlen + len);
	if (str != buf)
		free(str);
}


/*
 * Read in a symbol name which may or may not be a keyword.
 * If allsyms is set, keywords are not looked up and almost all chars
 * will be accepted for the symbol.  Returns the type of symbol found.
 */
S_FUNC int
eatsymbol(void)
{
	register struct keyword *kp;	/* pointer to current keyword */
	register char *cp;		/* current character pointer */
	int ch;				/* current character */
	int cc;				/* character count */
	STATIC char buf[SYMBOLSIZE+1];	/* temporary buffer */

	cp = buf;
	cc = SYMBOLSIZE;
	if (allsyms) {
		for (;;) {
			ch = nextchar();
			if (ch == ' ' || ch == ';' ||
					 ch == '\n' || ch == EOF)
				break;
			if (cc-- > 0)
				*cp++ = (char) ch;
		}
		reread();
		*cp = '\0';
		if (cc < 0)
			scanerror(T_NULL, "Symbol too long");
		curtoken.t_sym = buf;
		return T_SYMBOL;
	}
	for (;;) {
		ch = nextchar();
		if (!issymbol(ch))
			break;
		if (cc-- > 0)
			*cp++ = (char)ch;
	}
	reread();
	*cp = '\0';
	if (cc < 0)
		scanerror(T_NULL, "Symbol too long");
	for (kp = keywords; kp->k_name; kp++)
		if (strcmp(kp->k_name, buf) == 0)
			return kp->k_token;
	curtoken.t_sym = buf;
	return T_SYMBOL;
}


/*
 * Read in and remember a possibly numeric constant value.
 * The constant is inserted into a constant table so further uses
 * of the same constant will not take more memory.  This can also
 * return just a period, which is used for element accesses and for
 * the old numeric value.
 */
S_FUNC int
eatnumber(void)
{
	register char *cp;	/* current character pointer */
	long len;		/* parsed size of number */
	long res;		/* result of parsing number */

	if (numbufsize == 0) {
		numbuf = (char *)malloc(128+1);
		if (numbuf == NULL)
			math_error("Cannot allocate number buffer");
		numbufsize = 128;
	}
	cp = numbuf;
	len = 0;
	for (;;) {
		if (len >= numbufsize) {
			cp = (char *)realloc(numbuf, numbufsize + 1001);
			if (cp == NULL) {
				math_error("Cannot reallocate number buffer");
				/*NOTREACHED*/
			}
			numbuf = cp;
			numbufsize += 1000;
			cp = &numbuf[len];
		}
		*cp = nextchar();
		*(++cp) = '\0';
		if ((numbuf[0] == '.') && isletter(numbuf[1])) {
			reread();
			return T_PERIOD;
		}
		res = qparse(numbuf, QPF_IMAG);
		if (res < 0) {
			reread();
			scanerror(T_NULL, "Badly formatted number");
			curtoken.t_numindex = addnumber("0");
			return T_NUMBER;
		}
		if (res != ++len)
			break;
	}
	cp[-1] = '\0';
	reread();
	if ((numbuf[0] == '.') && (numbuf[1] == '\0')) {
		curtoken.t_numindex = 0;
		return T_OLDVALUE;
	}
	cp -= 2;
	res = T_NUMBER;
	if ((*cp == 'i') || (*cp == 'I')) {
		*cp = '\0';
		res = T_IMAGINARY;
	}
	curtoken.t_numindex = addnumber(numbuf);
	return (int)res;
}


/*
 * Return the index for string value of the current token.
 */
long
tokenstring(void)
{
	return curtoken.t_strindex;
}


/*
 * Return the constant index of a numeric token.
 */
long
tokennumber(void)
{
	return curtoken.t_numindex;
}

/*
 * Return the address of a symbol
 */
char *
tokensymbol(void)
{
	return curtoken.t_sym;
}

/*
 * Push back the token just read so that it will be seen again.
 */
void
rescantoken(void)
{
	rescan = TRUE;
}


/*
 * Describe an error message.
 * Then skip to the next specified token (or one more powerful).
 */
void
scanerror(int skip, char *fmt, ...)
{
	va_list ap;
	char *name;		/* name of file with error */
	int len;		/* length of error msg buffer */

	/* count the error */
	errorcount++;

	/* form the error message */
	name = inputname();
	va_start(ap, fmt);
	if (name) {
		snprintf(calc_err_msg, MAXERROR, "\"%s\", line %ld: ",
			 name, linenumber());
		calc_err_msg[MAXERROR] = '\0';	/* paranoia */
		len = strlen(calc_err_msg);
		if (len < MAXERROR) {
			vsnprintf(calc_err_msg+len, MAXERROR-len, fmt, ap);
		}
	} else {
		vsnprintf(calc_err_msg, MAXERROR, fmt, ap);
	}
	va_end(ap);
	calc_err_msg[MAXERROR] = '\0';	/* paranoia */

	/* print error message if allowed */
	if (calc_print_scanerr_msg != 0) {
		fprintf(stderr, "%s\n\n", calc_err_msg);
	}

	/* bail out if continuation not permitted */
	if ((!c_flag && !stoponerror) || stoponerror > 0) {
		if (calc_use_scanerr_jmpbuf != 0) {
			longjmp(calc_scanerr_jmpbuf, 60);
			/*NOTREACHED*/
		} else {
			fprintf(stderr,
			  "calc_scanerr_jmpbuf not setup, exiting code 60\n");
			libcalc_call_me_last();
			exit(60);
		}
	}

	/* bail out if too many errors */
	if (conf->maxscancount > 0 && errorcount > conf->maxscancount) {
		fprintf(stderr, "Too many scan errors, compilation aborted.\n");
		if (calc_use_scanerr_jmpbuf != 0) {
			longjmp(calc_scanerr_jmpbuf, 61);
			/*NOTREACHED*/
		} else {
			fprintf(stderr,
			  "calc_scanerr_jmpbuf not ready: exit 61\n");
			libcalc_call_me_last();
			exit(61);
		}
	}

	/* post-error report processing */
	switch (skip) {
	case T_NULL:
		return;
	case T_COMMA:
		rescan = TRUE;
		for (;;) {
			switch (gettoken()) {
			case T_NEWLINE:
			case T_SEMICOLON:
			case T_LEFTBRACE:
			case T_RIGHTBRACE:
			case T_EOF:
			case T_COMMA:
				rescan = TRUE;
				return;
			}
		}
	default:
		snprintf(calc_err_msg, MAXERROR,
			 "Unknown skip token for scanerror\n");
		calc_err_msg[MAXERROR] = '\0';	/* paranoia */
		if (calc_print_scanerr_msg != 0) {
			fprintf(stderr, "%s\n\n", calc_err_msg);
		}
		/* fall into semicolon case */
		/*FALLTHRU*/
	case T_SEMICOLON:
		rescan = TRUE;
		for (;;) {
			switch (gettoken()) {
			case T_NEWLINE:
			case T_SEMICOLON:
			case T_LEFTBRACE:
			case T_RIGHTBRACE:
			case T_EOF:
				rescan = TRUE;
				return;
			}
		}
	}
}

/*
 * Display a warning and return to compiling
 */
void
warning(char *fmt, ...)
{
	va_list ap;
	char *name;		/* name of file with error */
	int len;		/* length of error msg buffer */

	/* count this warning */
	++calc_warn_cnt;

	/* form the error message */
	name = inputname();
	va_start(ap, fmt);
	if (name) {
		snprintf(calc_warn_msg, MAXERROR, "\"%s\", line %ld: ",
			 name, linenumber());
		calc_warn_msg[MAXERROR] = '\0';	/* paranoia */
		len = strlen(calc_warn_msg);
		if (len < MAXERROR) {
			vsnprintf(calc_warn_msg+len, MAXERROR-len, fmt, ap);
		}
	} else {
		vsnprintf(calc_warn_msg, MAXERROR, fmt, ap);
	}
	va_end(ap);
	calc_warn_msg[MAXERROR] = '\0';	/* paranoia */

	/* print the warning if allowed */
	if (calc_print_scanwarn_msg != 0) {
		fprintf(stderr, "Warning: %s\n", calc_warn_msg);
	}
	return;
}
