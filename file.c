/*
 * file - file I/O routines callable by users
 *
 * Copyright (C) 1999-2007,2018,2021  David I. Bell and Landon Curt Noll
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
 * Under source code control:	1991/07/20 00:21:56
 * File existed as early as:	1991
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <ctype.h>
#include "calc.h"
#include "alloc.h"
#include "longbits.h"
#include "have_fpos.h"
#include "have_fpos_pos.h"
#include "fposval.h"
#include "file.h"
#include "calcerr.h"
#include "strl.h"

#if defined(_WIN32)
# include <io.h>
#endif


#include "banned.h"	/* include after system header <> includes */


#define READSIZE	1024	/* buffer size for reading */

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))

/*
 * external STDIO functions
 */
E_FUNC void math_setfp(FILE *fp);
E_FUNC FILE *f_open(char *name, char *mode);


/*
 * Table of opened files.
 * The first three entries always correspond to stdin, stdout, and stderr,
 * and cannot be closed.  Their file ids are always 0, 1, and 2.
 */
STATIC FILEIO files[MAXFILES] = {
	{FILEID_STDIN,	NULL,  (dev_t)0, (ino_t)0,
	 "(stdin)",  TRUE, FALSE, FALSE, FALSE, 'r', "r"},
	{FILEID_STDOUT, NULL, (dev_t)0, (ino_t)0,
	 "(stdout)", FALSE,  TRUE, FALSE, FALSE, 'w', "w"},
	{FILEID_STDERR, NULL, (dev_t)0, (ino_t)0,
	 "(stderr)", FALSE,  TRUE, FALSE, FALSE, 'w', "w"}
};


STATIC int ioindex[MAXFILES] = {0,1,2}; /* Indices for FILEIO table */
STATIC FILEID lastid = FILEID_STDERR;	/* Last allocated file id */
STATIC int idnum = 3;			/* Number of allocated file ids */


/* forward static declarations */
S_FUNC ZVALUE filepos2z(FILEPOS pos);
S_FUNC FILEPOS z2filepos(ZVALUE pos);
S_FUNC int set_open_pos(FILE *fp, ZVALUE zpos);
S_FUNC int get_open_pos(FILE *fp, ZVALUE *res);
S_FUNC ZVALUE off_t2z(off_t siz);
S_FUNC ZVALUE dev2z(dev_t dev);
S_FUNC ZVALUE inode2z(ino_t inode);
S_FUNC void getscanfield(FILE *fp, BOOL skip, unsigned int width,
			 int scannum, char *scanptr, char **strptr);
S_FUNC void getscanwhite(FILE *fp, BOOL skip, unsigned int width,
			 int scannum, char **strptr);
S_FUNC int fscanfile(FILE *fp, char *fmt, int count, VALUE **vals);
S_FUNC void freadnum(FILE *fp, VALUE *valptr);
S_FUNC void freadsum(FILE *fp, VALUE *valptr);
S_FUNC void freadprod(FILE *fp, VALUE *valptr);
S_FUNC void fskipnum(FILE *fp);


/*
 * file_init - perform needed initialization work
 *
 * On some systems, one cannot initialize a pointer to a FILE *.
 * This routine, called once at startup is a work-a-round for
 * systems with such bogons.
 *
 * We will also probe for any open files beyond stderr and set them up.
 */
void
file_init(void)
{
    STATIC int done = 0;	/* 1 => routine already called */
    struct stat sbuf;		/* file status */
    FILEIO *fiop;
    FILE *fp;
    int i;

    if (!done) {
	/*
	 * setup the default set
	 */
	files[0].fp = stdin;
	files[1].fp = stdout;
	files[2].fp = stderr;
	for (i = 0; i < 3; ++i) {
		if (fstat(i, &sbuf) >= 0) {
			files[i].dev = sbuf.st_dev;
			files[i].inode = sbuf.st_ino;
		}
	}

	/*
	 * note any other files that we can find
	 */
	fiop = &files[3];
	for (i = 3; i < MAXFILES; fiop++, ++i) {
		char *tname;

		fiop->name = NULL;
		files[idnum].reading = TRUE;
		files[idnum].writing = TRUE;
		files[idnum].action = 0;
		memset(files[idnum].mode, 0, MODE_LEN+1);
		/*
		 * stat the descriptor to see what we have
		 */
		if (fstat(i, &sbuf) >= 0) {
			size_t snprintf_len;	/* malloced snprintf length */
			fp = (FILE *) fdopen(i,"r+");	/*guess mode*/
			if (fp) {
				strlcpy(files[idnum].mode, "r+",
					sizeof(files[idnum].mode));
			} else {
				fp = (FILE *) fdopen(i, "r");
				if (fp) {
					strlcpy(files[idnum].mode, "r",
						sizeof(files[idnum].mode));
					files[idnum].writing = FALSE;
				} else {
					fp = (FILE *) fdopen(i, "w");
					if (fp) {
						strlcpy(files[idnum].mode, "w",
						    sizeof(files[idnum].mode));
						files[idnum].reading = FALSE;
					}
					else
						continue;
				}
			}
			snprintf_len =
			  sizeof("descriptor[12345678901234567890]") + 1;
			tname = (char *)malloc(snprintf_len+1);
			if (tname == NULL) {
				math_error("Out of memory for init_file");
				/*NOTREACHED*/
			}
			snprintf(tname, snprintf_len, "descriptor[%d]", i);
			tname[snprintf_len] = '\0';	/* paranoia */
			files[idnum].name = tname;
			files[idnum].id = idnum;
			files[idnum].fp = fp;
			files[idnum].dev = sbuf.st_dev;
			files[idnum].inode = sbuf.st_ino;
			ioindex[idnum] = idnum;
			idnum++;
			lastid++;
		}
	}

	done = 1;
    }
}


/*
 * init_fileio - initialize a FILEIO structure
 *
 * This function initializes a calc FILEIO structure.  It will optionally
 * malloc the filename string if given an non-NULL associated filename.
 * It will canonicalize the file open mode string.
 *
 * given:
 *	fiop	pointer to FILEIO structure to initialize
 *	name	associated filename (NULL => caller will setup filename)
 *	mode	open mode (one of {r,w,a}{,b}{,+})
 *	sbufp	pointer to stat of open file
 *	id	calc file ID
 *	fp	open file stream
 */
S_FUNC void
init_fileio(FILEIO *fiop, char *name, char *mode,
	    struct stat *sbufp, FILEID id, FILE *fp)
{
	char modestr[MODE_LEN+1];	/* mode [rwa]b?\+? */
	size_t namelen;			/* length of name */

	/* clear modestr */
	memset(modestr, 0, sizeof(modestr));

	/* allocate filename if requested */
	namelen = 0;
	if (name != NULL) {
		namelen = strlen(name);
		fiop->name = (char *)malloc(namelen + 1);
		if (fiop->name == NULL) {
			math_error("No memory for filename");
			/*NOTREACHED*/
		}
	}

	/* initialize FILEIO structure */
	if (name != NULL) {
		strlcpy(fiop->name, name, namelen+1);
	}
	fiop->id = id;
	fiop->fp = fp;
	fiop->dev = sbufp->st_dev;
	fiop->inode = sbufp->st_ino;
	fiop->reading = FALSE;
	fiop->writing = FALSE;
	fiop->appending = FALSE;
	fiop->binary = FALSE;
	fiop->action = 0;
	memset(fiop->mode, 0, sizeof(fiop->mode));

	/*
	 * determine file open mode
	 *
	 * While a leading 'r' is for reading and a leading 'w' is
	 * for writing, the presence of a '+' in the string means
	 * both reading and writing.  A leading 'a' means append
	 * which is writing.
	 */
	/* canonicalize read modes */
	if (mode[0] == 'r') {

		/* note read mode */
		strlcpy(modestr, "r", sizeof(modestr));
		fiop->reading = TRUE;

		/* note binary mode even though mode is not used / ignored */
		if (strchr(mode, 'b') != NULL) {
		    strlcat(modestr, "b", sizeof(modestr));
		}

		/* note if reading and writing */
		if (strchr(mode, '+') != NULL) {
			fiop->writing = TRUE;
			strlcat(modestr, "+", sizeof(modestr));
		}

	/* canonicalize write modes */
	} else if (mode[0] == 'w') {

		/* note write mode */
		strlcpy(modestr, "w", sizeof(modestr));
		fiop->writing = TRUE;

		/* note binary mode even though mode is not used / ignored */
		if (strchr(mode, 'b') != NULL) {
		    strlcat(modestr, "b", sizeof(modestr));
		}

		/* note if reading and writing */
		if (strchr(mode, '+') != NULL) {
			fiop->reading = TRUE;
			strlcat(modestr, "+", sizeof(modestr));
		}

	/* canonicalize append modes */
	} else if (mode[0] == 'a') {

		/* note append mode */
		strlcpy(modestr, "a", sizeof(modestr));
		fiop->writing = TRUE;
		fiop->appending = TRUE;

		/* note binary mode even though mode is not used / ignored */
		if (strchr(mode, 'b') != NULL) {
		    strlcat(modestr, "b", sizeof(modestr));
		}

		/* note if reading and writing */
		if (strchr(mode, '+') != NULL) {
			fiop->reading = TRUE;
			strlcat(modestr, "+", sizeof(modestr));
		}

	/* canonicalize no I/O modes */
	} else {
		modestr[0] = '\0';
	}
	modestr[MODE_LEN] = '\0';	/* firewall */

	/* record canonical open mode string */
	strlcpy(fiop->mode, modestr, sizeof(fiop->mode));
}


/*
 * openid - open the specified file name for reading or writing
 *
 * given:
 *	name		file name
 *	mode		open mode (one of {r,w,a}{,b}{,+})
 *
 * returns:
 *	>=3 FILEID which can be used to do I/O to the file
 *	<0 if the open failed
 *
 * NOTE: This function will not return 0, 1 or 2 since they are
 *	 reserved for stdin, stdout, stderr.  In fact, it must not
 *	 return 0, 1, or 2 because it will confuse those who call
 *	 the opensearchfiile() function
 */
FILEID
openid(char *name, char *mode)
{
	FILEIO *fiop;		/* file structure */
	FILEID id;		/* new file id */
	FILE *fp;
	struct stat sbuf;	/* file status */
	int i;

	/* find the next open slot in the files array */
	if (idnum >= MAXFILES)
		return -E_MANYOPEN;
	fiop = &files[3];
	for (i = 3; i < MAXFILES; fiop++,i++) {
		if (fiop->name == NULL)
			break;
	}
	if (i == MAXFILES)
		math_error("This should not happen in openid()!!!");

	/* open the file */
	fp = f_open(name, mode);
	if (fp == NULL) {
		return FILEID_NONE;
	}
	if (fstat(fileno(fp), &sbuf) < 0) {
		math_error("bad fstat");
		/*NOTREACHED*/
	}

	/* get a new FILEID */
	id = ++lastid;
	ioindex[idnum++] = i;

	/* initialize FILEIO structure */
	init_fileio(fiop, name, mode, &sbuf, id, fp);

	/* return calc open file ID */
	return id;
}


/*
 * openpathid - open the specified abse filename, or
 *	        relative filename along a search path
 *
 * given:
 *	name		file name
 *	mode		open mode (one of {r,w,a}{,b}{,+})
 *	pathlist	list of colon separated paths (or NULL)
 *
 * returns:
 *	>=3 FILEID which can be used to do I/O to the file
 *	<0 if the open failed
 *
 * NOTE: This function will not return 0, 1 or 2 since they are
 *	 reserved for stdin, stdout, stderr.  In fact, it must not
 *	 return 0, 1, or 2 because it will confuse those who call
 *	 the opensearchfiile() function
 */
FILEID
openpathid(char *name, char *mode, char *pathlist)
{
	FILEIO *fiop;		/* file structure */
	FILEID id;		/* new file id */
	FILE *fp;
	struct stat sbuf;	/* file status */
	char *openpath;		/* malloc copy of path that was opened */
	int i;

	/* find the next open slot in the files array */
	if (idnum >= MAXFILES)
		return -E_MANYOPEN;
	fiop = &files[3];
	for (i = 3; i < MAXFILES; fiop++,i++) {
		if (fiop->name == NULL)
			break;
	}
	if (i == MAXFILES)
		math_error("This should not happen in openpathid()!!!");

	/* open a file - searching along a path */
	openpath = NULL;
	fp = f_pathopen(name, mode, pathlist, &openpath);
	if (fp == NULL) {
		if (openpath != NULL) {
			/* should not happen, but just in case */
			free(openpath);
		}
		return FILEID_NONE;
	}
	if (fstat(fileno(fp), &sbuf) < 0) {
		if (openpath != NULL) {
			free(openpath);
		}
		math_error("bad fstat");
		/*NOTREACHED*/
	}
	if (openpath == NULL) {
		fclose(fp);
		math_error("bad openpath");
		/*NOTREACHED*/
	}

	/* get a new FILEID */
	id = ++lastid;
	ioindex[idnum++] = i;

	/* initialize FILEIO structure */
	init_fileio(fiop, NULL, mode, &sbuf, id, fp);
	fiop->name = openpath;	/* already malloced by f_pathopen */

	/* return calc open file ID */
	return id;
}


/*
 * reopenid - reopen a FILEID
 *
 * given:
 *	id	FILEID to reopen
 *	mode	new mode to open as
 *	mode	new mode to open as (one of "r", "w", "a", "r+", "w+", "a+")
 *	name	name of new file
 *
 * returns:
 *	FILEID which can be used to do I/O to the file
 *	<0 if the open failed
 */
FILEID
reopenid(FILEID id, char *mode, char *name)
{
	FILEIO *fiop;		/* file structure */
	FILE *fp;
	struct stat sbuf;
	int i;

	/* firewall */
	if ((id == FILEID_STDIN) || (id == FILEID_STDOUT) ||
	    (id == FILEID_STDERR)) {
		math_error("Cannot freopen stdin, stdout, or stderr");
		/*NOTREACHED*/
	}

	/* reopen the file */
	fiop = NULL;
	for (i = 3; i < idnum; i++) {
		fiop = &files[ioindex[i]];
		if (fiop->id == id)
			break;
	}
	if (i == idnum) {
		if (name == NULL) {
			fprintf(stderr, "File not open, need file name\n");
			return FILEID_NONE;
		}
		if (idnum >= MAXFILES) {
			fprintf(stderr, "Too many open files\n");
			return FILEID_NONE;
		}
		for (fiop = &files[3], i = 3; i < MAXFILES; fiop++, i++) {
			if (fiop->name == NULL)
				break;
		}
		if (i >= MAXFILES) {
			math_error("This should not happen in reopenid");
			/*NOTREACHED*/
		}
		fp = f_open(name, mode);
		if (fp == NULL) {
			fprintf(stderr, "Cannot open file\n");
			return FILEID_NONE;
		}
		ioindex[idnum++] = i;
		fiop->id = id;
	} else {
		if (name == NULL)
			fp = freopen(fiop->name, mode, fiop->fp);
		else
			fp = freopen(name, mode, fiop->fp);
		if (fp == NULL) {
			free(fiop->name);
			fiop->name = NULL;
			idnum--;
			for (; i < idnum; i++)
				ioindex[i] = ioindex[i + 1];
			return FILEID_NONE;
		}
	}
	if (fstat(fileno(fp), &sbuf) < 0) {
		math_error("bad fstat");
		/*NOTREACHED*/
	}

	/* initialize FILEIO structure */
	if (name == NULL) {
		if (fiop->name == NULL) {
			math_error("old and new reopen filenames are NULL");
		}
	} else if (fiop->name != NULL) {
		free(fiop->name);
		fiop->name = NULL;
	}
	init_fileio(fiop, name, mode, &sbuf, id, fp);

	/* return calc open file ID */
	return id;
}


/*
 * Find the file I/O structure for the specified file id, and verifies that
 * it is opened in the required manner (0 for reading or 1 for writing).
 * If writable is -1, then no open checks are made at all and NULL is then
 * returned if the id represents a closed file.
 */
FILEIO *
findid(FILEID id, int writable)
{
	FILEIO *fiop;		/* file structure */
	int i;

	fiop = NULL;

	if ((id < 0) || (id > lastid))
		return NULL;

	for (i = 0; i < idnum; i++) {
		fiop = &files[ioindex[i]];
		if (fiop->id == id)
			break;
	}

	if (i == idnum)
		return NULL;

	if (writable >= 0) {
		if ((writable && !fiop->writing) ||
		    (!writable && !fiop->reading)) {
			return NULL;
		}
	}
	return fiop;
}


/*
 * Return whether or not a file id is valid.  This is used for if tests.
 */
BOOL
validid(FILEID id)
{
	return (findid(id, -1) != NULL);
}


/*
 * Return the file with id = index if this is the id of a file that has been
 * opened (it may have since been closed).  Otherwise returns FILEID_NONE.
 */
FILEID
indexid(long index)
{
	FILEID id;

	id = (FILEID) index;

	if ((index < 0) || (id > lastid))
		return FILEID_NONE;
	return id;
}


/*
 * Close the specified file id.	 Returns TRUE if there was an error.
 * Closing of stdin, stdout, or stderr is illegal, but closing of already
 * closed files is allowed.
 */
int
closeid(FILEID id)
{
	FILEIO *fiop;		/* file structure */
	int i;
	int err;

	fiop = NULL;

	/* firewall */
	if ((id == FILEID_STDIN) || (id == FILEID_STDOUT) ||
	    (id == FILEID_STDERR)) {
		math_error("Cannot close stdin, stdout, or stderr");
		/*NOTREACHED*/
	}

	/* get file structure */
	for (i = 3; i < idnum; i++) {
		fiop = &files[ioindex[i]];
		if (fiop->id == id)
			break;
	}
	if (i == idnum)
		return 1;		/* File not open */
	idnum--;
	for (; i < idnum; i++)
		ioindex[i] = ioindex[i + 1];

	free(fiop->name);
	fiop->name = NULL;

	/* close file and note error state */
	err = ferror(fiop->fp);
	err |= fclose(fiop->fp);
	fiop->fp = NULL;

	/* return success or failure */
	return (err ? EOF : 0);
}


int
closeall(void)
{
	FILEIO *fiop;
	int i;
	int err;

	err = 0;
	for (i = 3; i < idnum; i++) {
		fiop = &files[ioindex[i]];
		if (fiop->fp) {
			free(fiop->name);
			fiop->name = NULL;
			err |= fclose(fiop->fp);
		}
	}
	idnum = 3;
	return err;
}


/*
 * Return whether or not an error occurred to a file.
 */
BOOL
errorid(FILEID id)
{
	FILEIO *fiop;		/* file structure */

	fiop = findid(id, -1);
	if (fiop == NULL)
		return EOF;
	return (ferror(fiop->fp) != 0);
}


/*
 * Return whether or not end of file occurred to a file.
 */
BOOL
eofid(FILEID id)
{
	FILEIO *fiop;		/* file structure */

	fiop = findid(id, -1);
	if (fiop == NULL)
		return EOF;
	return (feof(fiop->fp) != 0);
}


/*
 * Flush output to an opened file.
 */
int
flushid(FILEID id)
{
	FILEIO *fiop;		/* file structure */

	fiop = findid(id, -1);
	if (fiop == NULL)
		return 0;
	if (!fiop->writing || fiop->action == 'r')
		return 0;
	return fflush(fiop->fp);
}


#if !defined(_WIN32)
int
flushall(void)
{
	FILEIO *fiop;
	int i;
	int err;

	err = 0;
	for (i = 3; i < idnum; i++) {
		fiop = &files[ioindex[i]];
		if (fiop->writing && fiop->action != 'r')
			err |= fflush(fiop->fp);
	}
	return err;
}
#endif /* Windoz free systems */


/*
 * Read the next line, string or word from an opened file.
 * Returns a pointer to an allocated string holding a null-terminated
 * or newline terminated string.  Where reading stops is controlled by
 * flags:
 *
 *	bit 0:	at newline
 *	bit 1:	at null character
 *	bit 2:	at white space (also skips leading white space)
 *
 * If neither '\n' nor '\0' is encountered reading continues until EOF.
 * If bit 3 is set the stop character is removed.
 *
 * given:
 *	id		file to read from
 *	flags		read flags (see above)
 *	retstr		returned pointer to string
 */
int
readid(FILEID id, int flags, STRING **retstr)
{
	FILEIO *fiop;		/* file structure */
	FILE *fp;
	char *str;		/* current string */
	unsigned long n;	/* current number characters read into buf */
	unsigned long totlen;	/* total length of string copied from buf */
	char buf[READSIZE];	/* temporary buffer */
	char *b;
	int c;
	BOOL nlstop, nullstop, wsstop, rmstop, done;
	FILEPOS fpos;
	STRING *newstr;

	totlen = 0;
	str = NULL;

	fiop = findid(id, FALSE);
	if (fiop == NULL)
		return 1;
	nlstop = (flags & 1);
	nullstop = (flags & 2);
	wsstop = (flags & 4);
	rmstop = (flags & 8);

	fp = fiop->fp;

	if (fiop->action == 'w') {
		f_tell(fp, &fpos);
		fflush(fp);
		if (f_seek_set(fp, &fpos) < 0)
			return 3;
	}
	fiop->action = 'r';

	if (wsstop) {
		while (isspace(c = fgetc(fp)));
		ungetc(c, fp);
	}

	for (;;) {
		b = buf;
		n = 0;
		do {
			c = fgetc(fp);
			if (c == EOF)
				break;
			n++;
			if (nlstop && c == '\n')
				break;
			if (nullstop && c == '\0')
				break;
			if (wsstop && isspace(c))
				break;
			*b++ = c;
		} while (n < READSIZE);
		done = ((nlstop && c == '\n') || (nullstop && c == '\0') ||
			(wsstop && isspace(c)) || c == EOF);
		if (done && rmstop && c != EOF)
			n--;
		if (totlen)
			str = (char *)realloc(str, totlen + n + 1);
		else
			str = (char *)malloc(n + 1);
		if (str == NULL) {
			math_error("Out of memory for readid");
			/*NOTREACHED*/
		}
		if (n > 0)
			memcpy(&str[totlen], buf, n);
		totlen += n;
		if (done)
			break;
	}
	if (totlen == 0 && c == EOF) {
		free(str);
		return EOF;
	}
	if ((nlstop && c == '\n') && !rmstop)
		str[totlen - 1] = '\n';
	if ((nullstop && c == '\0') && !rmstop)
		str[totlen - 1] = '\0';
	str[totlen] = '\0';
	newstr = stralloc();
	newstr->s_len = totlen;
	newstr->s_str = str;
	*retstr = newstr;
	return 0;
}


/*
 * Return the next character from an opened file.
 * Returns EOF if there was an error or end of file.
 */
int
getcharid(FILEID id)
{
	FILEIO *fiop;
	FILEPOS fpos;

	fiop = findid(id, FALSE);
	if (fiop == NULL)
		return -2;
	if (fiop->action == 'w') {
		f_tell(fiop->fp, &fpos);
		fflush(fiop->fp);
		if (f_seek_set(fiop->fp, &fpos) < 0)
			return -3;
	}
	fiop->action = 'r';

	return fgetc(fiop->fp);
}


/*
 * Print out the name of an opened file.
 * If the file has been closed, a null name is printed.
 * If flags contain PRINT_UNAMBIG then extra information is printed
 * identifying the output as a file and some data about it.
 */
int
printid(FILEID id, int flags)
{
	FILEIO *fiop;		/* file structure */
	FILE *fp;
	ZVALUE pos;		/* file position */

	/*
	 * filewall - file is closed
	 */
	fiop = findid(id, -1);
	if (fiop == NULL) {
		if (flags & PRINT_UNAMBIG)
			math_fmt("FILE %ld closed", id);
		else
			math_str("\"\"");
		return 1;
	}

	/*
	 * print quoted filename and mode
	 */
	if ((flags & PRINT_UNAMBIG) == 0) {
		math_chr('"');
		math_str(fiop->name);
		math_chr('"');
		return 0;
	}
	math_fmt("FILE %ld \"%s\" (%s", id, fiop->name,  fiop->mode);

	/*
	 * print file position
	 */

	fp = fiop->fp;

	if (get_open_pos(fp, &pos) < 0) {
	    if (fileno(fp) > 2)
	   	 math_str("Error while determining file position!");
	    math_chr(')');
	    return 0;
	}

	math_str(", pos ");
	zprintval(pos, 0, 0);
	zfree(pos);

	/*
	 * report special status
	 */
	if (ferror(fp))
		math_str(", error");
	if (feof(fp))
		math_str(", eof");
	math_chr(')');

	printf(" fileno: %d ", fileno(fp));
	return 0;
}


/*
 * Print a formatted string similar to printf.	Various formats of output
 * are possible, depending on the format string AND the actual types of the
 * values.  Mismatches do not cause errors, instead something reasonable is
 * printed instead.  The output goes to the file with the specified id.
 *
 * given:
 *	id		file id to print to
 *	count		print count
 *	fmt		standard format string
 *	vals		table of values to print
 */
int
idprintf(FILEID id, char *fmt, int count, VALUE **vals)
{
	FILEIO *fiop;
	VALUE *vp;
	char *str;
	int ch;
	size_t len;
	int oldmode, newmode;
	long olddigits, newdigits;
	long width, precision;
	BOOL didneg, didprecision;
	FILEPOS fpos;
	BOOL printstring;
	BOOL printchar;

	fiop = findid(id, TRUE);
	if (fiop == NULL)
		return 1;
	if (fiop->action == 'r') {
		f_tell(fiop->fp, &fpos);
		if (f_seek_set(fiop->fp, &fpos) < 0)
			return 3;
	}

	fiop->action = 'w';

	printstring = FALSE;
	printchar = FALSE;

	math_setfp(fiop->fp);

	while ((ch = *fmt++) != '\0') {
		if (ch != '%') {
			math_chr(ch);
			continue;
		}

		/*
		 * Here to handle formats.
		 */
		didneg = FALSE;
		didprecision = FALSE;
		width = 0;
		precision = 0;

		ch = *fmt++;
		if (ch == '-') {
			didneg = TRUE;
			ch = *fmt++;
		}
		while ((ch >= '0') && (ch <= '9')) {
			width = width * 10 + (ch - '0');
			ch = *fmt++;
		}
		if (ch == '.') {
			didprecision = TRUE;
			ch = *fmt++;
			while ((ch >= '0') && (ch <= '9')) {
				precision = precision * 10 + (ch - '0');
				ch = *fmt++;
			}
		}
		if (ch == 'l')
			ch = *fmt++;

		oldmode = conf->outmode;
		newmode = oldmode;
		olddigits = conf->outdigits;
		newdigits = olddigits;
		if (didprecision)
			newdigits = precision;

		switch (ch) {
		case 's':
			printstring = TRUE;
			/*FALLTHRU*/
		case 'c':
			printchar = TRUE;
		case 'd':
			break;
		case 'f':
			newmode = MODE_REAL;
			break;
		case 'e':
			newmode = MODE_EXP;
			break;
		case 'g':
			newmode = MODE_REAL_AUTO;
			break;
		case 'r':
			newmode = MODE_FRAC;
			break;
		case 'o':
			newmode = MODE_OCTAL;
			break;
		case 'x':
			newmode = MODE_HEX;
			break;
		case 'b':
			newmode = MODE_BINARY;
			break;
		case 0:
			math_setfp(stdout);
			return 0;
		default:
			math_chr(ch);
			continue;
		}

		if (--count < 0) {
			while (width-- > 0)
				math_chr(' ');
			continue;
		}
		vp = *vals++;

		math_setdigits(newdigits);
		math_setmode(newmode);

		/*
		 * If there is no width specification, or if the type of
		 * value requires multiple lines, then just output the
		 * value directly.
		 */
		if ((width == 0) ||
			(vp->v_type == V_MAT) || (vp->v_type == V_LIST)) {
			switch(vp->v_type) {
			case V_OCTET:
				if (printstring)
					math_str((char *)vp->v_octet);
				else if (printchar)
					math_chr(*vp->v_octet);
				else
					printvalue(vp, PRINT_NORMAL);
				break;
			case V_BLOCK:
				if (printstring)
					math_str((char *)
						 vp->v_block->data);
				else if (printchar)
					math_chr(*vp->v_block->data);
				else
					printvalue(vp, PRINT_NORMAL);
				break;
			case V_NBLOCK:
				if (printstring) {
					if (vp->v_nblock->blk->data !=
					    NULL)
						math_str((char *)
							 vp->v_nblock
							 ->blk->data);
				} else if (printchar) {
					if (vp->v_nblock->blk->data !=
					    NULL)
						math_chr(*vp->v_nblock->
							 blk->data);
				} else {
					printvalue(vp, PRINT_NORMAL);
				}
				break;
			default:
				printvalue(vp, PRINT_NORMAL);
			}

			math_setmode(oldmode);
			math_setdigits(olddigits);
			continue;
		}


		/*
		 * There is a field width.  Collect the output in a string,
		 * print it padded appropriately with spaces, and free it.
		 * However, if the output contains a newline, then ignore
		 * the field width.
		 */
		math_divertio();
		switch(vp->v_type) {
		case V_OCTET:
			if (printstring)
				math_str((char *)vp->v_octet);
			else if (printchar)
				math_chr(*vp->v_octet);
			else
				printvalue(vp, PRINT_NORMAL);
			break;
		case V_BLOCK:
			if (printstring)
				math_str((char *)vp->v_block->data);
			else if (printchar)
				math_chr(*vp->v_block->data);
			else
				printvalue(vp, PRINT_NORMAL);
			break;
		case V_NBLOCK:
			if (printstring) {
				if (vp->v_nblock->blk->data != NULL)
					math_str((char *)
						 vp->v_nblock->blk->data);
			}
			else if (printchar) {
				if (vp->v_nblock->blk->data != NULL)
					math_chr(*vp->v_nblock->blk->data);
			}
			else
				printvalue(vp, PRINT_NORMAL);
			break;
		default:
			printvalue(vp, PRINT_NORMAL);
		}
		str = math_getdivertedio();
		if (strchr(str, '\n'))
			width = 0;
		len = strlen(str);
		while (!didneg && ((size_t)width > len)) {
			width--;
			math_chr(' ');
		}
		math_str(str);
		free(str);
		while (didneg && ((size_t)width > len)) {
			width--;
			math_chr(' ');
		}
		math_setmode(oldmode);
		math_setdigits(olddigits);
	}
	math_setfp(stdout);
	return 0;
}


/*
 * Write a character to a file.
 *
 * given:
 *	id		file id to print to
 *	ch		character to write
 */
int
idfputc(FILEID id, int ch)
{
	FILEIO *fiop;
	FILEPOS fpos;

	/* get the file info pointer */
	fiop = findid(id, TRUE);
	if (fiop == NULL)
		return 1;
	if (fiop->action == 'r') {
		f_tell(fiop->fp, &fpos);
		if (f_seek_set(fiop->fp, &fpos) < 0)
			return 2;
	}

	fiop->action = 'w';

	/* set output to file */
	math_setfp(fiop->fp);

	/* write char */
	math_chr(ch);

	/* restore output to stdout */
	math_setfp(stdout);
	return 0;
}


/*
 * Unget a character read from a file.
 *
 * given:
 *	id		file id to print to
 *	ch		character to write
 */
int
idungetc(FILEID id, int ch)
{
	FILEIO *fiop;

	fiop = findid(id, FALSE);
	if (fiop == NULL)
		return -2;
	if (fiop->action != 'r')
		return -2;
	return ungetc(ch, fiop->fp);
}


/*
 * Write a string to a file.
 *
 * given:
 *	id		file id to print to
 *	str		string to write
 */
int
idfputs(FILEID id, STRING *str)
{
	FILEIO *fiop;
	FILEPOS fpos;
	FILE *fp;
	char *c;
	long len;

	/* get the file info pointer */
	fiop = findid(id, TRUE);
	if (fiop == NULL)
		return 1;

	if (fiop->action == 'r') {
		f_tell(fiop->fp, &fpos);
		if (f_seek_set(fiop->fp, &fpos) < 0)
			return 2;
	}

	fiop->action = 'w';

	fp = fiop->fp;
	len = str->s_len;
	c = str->s_str;

	while (len-- > 0)
		fputc(*c++, fp);

	return 0;
}


/*
 * Same as idfputs but writes a terminating null character
 *
 * given:
 *	id			file id to print to
 *	str			string to write
 */
int
idfputstr(FILEID id, char *str)
{
	FILEIO *fiop;
	FILEPOS fpos;

	/* get the file info pointer */
	fiop = findid(id, TRUE);
	if (fiop == NULL)
		return 1;

	if (fiop->action == 'r') {
		f_tell(fiop->fp, &fpos);
		if (f_seek_set(fiop->fp, &fpos) < 0)
			return 2;
	}

	fiop->action = 'w';

	/* set output to file */
	math_setfp(fiop->fp);

	/* write the string */
	math_str(str);

	math_chr('\0');

	/* restore output to stdout */
	math_setfp(stdout);
	return 0;
}


int
rewindid(FILEID id)
{
	FILEIO *fiop;
	fiop = findid(id, -1);
	if (fiop == NULL)
		return 1;
	rewind(fiop->fp);
	fiop->action = 0;
	return 0;
}


void
rewindall(void)
{
	FILEIO *fiop;
	int i;

	for (i = 3; i < idnum; i++) {
		fiop = &files[ioindex[i]];
		if (fiop != NULL) {
			(void) rewind(fiop->fp);
			fiop->action = 0;
		}
	}
}


/*
 * filepos2z - convert a positive file position into a ZVALUE
 *
 * given:
 *	pos		file position
 *
 * returns:
 *	file position as a ZVALUE
 *
 * NOTE: Does not support negative file positions.
 */
/*ARGSUSED*/
S_FUNC ZVALUE
filepos2z(FILEPOS pos)
{
	ZVALUE ret;		/* ZVALUE file position to return */

	/*
	 * store FILEPOS in a ZVALUE as a positive value
	 */
	ret.len = FILEPOS_BITS/BASEB;
	ret.v = alloc(ret.len);
	zclearval(ret);
	SWAP_HALF_IN_FILEPOS(ret.v, &pos);
	ret.sign = 0;
	ztrim(&ret);

	/*
	 * return our result
	 */
	return ret;
}


/*
 * z2filepos - convert a positive ZVALUE file position to a FILEPOS
 *
 * given:
 *	zpos		file position as a ZVALUE
 *
 * returns:
 *	file position as a FILEPOS
 *
 * NOTE: Does not support negative file positions.
 */
S_FUNC FILEPOS
z2filepos(ZVALUE zpos)
{
#if FILEPOS_BITS > FULL_BITS
	FILEPOS tmp;		/* temp file position as a FILEPOS */
#endif
	FILEPOS ret;		/* file position as a FILEPOS */
#if FILEPOS_BITS < FULL_BITS
	long pos;		/* zpos as a long */
#else
	FULL pos;		/* zpos as a FULL */
#endif

	/*
	 * firewall
	 */
	zpos.sign = 0;	/* deal only with the absolute value */

	/*
	 * quick return if the position can fit into a long
	 */
#if FILEPOS_BITS == FULL_BITS
	/* ztofull puts the value into native byte order */
	pos = ztofull(zpos);
	memset(&ret, 0, sizeof(ret));	/* FILEPOS could be non-scalar */
	memcpy((void *)&ret, (void *)&pos, MIN(sizeof(ret), sizeof(pos)));
	return ret;
#elif FILEPOS_BITS < FULL_BITS
	/* ztofull puts the value into native byte order */
	pos = ztolong(zpos);
	memset(&ret, 0, sizeof(ret));	/* FILEPOS could be non-scalar */
	memcpy((void *)&ret, (void *)&pos, MIN(sizeof(ret), sizeof(pos)));
	return ret;
#else /* FILEPOS_BITS > FULL_BITS */
	if (!zgtmaxfull(zpos)) {
		/* ztofull puts the value into native byte order */
		pos = ztofull(zpos);
		memset(&ret, 0, sizeof(ret)); /* FILEPOS could be non-scalar */
		memcpy((void *)&ret, (void *)&pos,
				     MIN(sizeof(ret), sizeof(pos)));
		return ret;
	}

	/*
	 * copy (and swap if needed) lower part of the ZVALUE as needed
	 */
	if (zpos.len >= FILEPOS_BITS/BASEB) {
		/* copy the lower FILEPOS_BITS of the ZVALUE */
		memset(&tmp, 0, sizeof(tmp)); /* FILEPOS could be non-scalar */
		memcpy(&tmp, zpos.v, MIN(sizeof(tmp), FILEPOS_LEN);
	} else {
		/* copy what bits we can into the temp value */
		memset(&tmp, 0, sizeof(tmp)); /* FILEPOS could be non-scalar */
		memcpy(&tmp, zpos.v, MIN(sizeof(tmp),
			     MIN(zpos.len*BASEB/8, FILEPOS_LEN)));
	}
	/* swap into native byte order */
	SWAP_HALF_IN_FILEPOS(&ret, &tmp);

	/*
	 * return our result
	 */
	return ret;
#endif /* FILEPOS_BITS <= FULL_BITS */
}


/*
 * get_open_pos - get a an open file position
 *
 * given:
 *	fp		open file stream
 *	res		where to place the file position (ZVALUE)
 *
 * returns:
 *	0		res points to the file position
 *	-1		error
 */
S_FUNC int
get_open_pos(FILE *fp, ZVALUE *res)
{
	FILEPOS pos;		/* current file position */

	/*
	 * get the file position
	 */
	if (f_tell(fp, &pos) < 0) {
	    /* cannot get file position, return -1 */
	    return -1;
	}

	/*
	 * update file position and return success
	 */
	*res = filepos2z(pos);
	return 0;
}


/*
 * getloc - get the current position of the file
 *
 * given:
 *	id	file id of the file
 *	loc	pointer to result
 *
 * returns:
 *	0	able to get file position
 *	-1	unable to get file position
 */
int
getloc(FILEID id, ZVALUE *res)
{
	FILEIO *fiop;		/* file structure */
	FILE *fp;

	/*
	 * convert id to stream
	 */
	fiop = findid(id, -1);
	if (fiop == NULL) {
		/* file not open */
		return -1;
	}
	fp = fiop->fp;
	if (fp == NULL) {
		math_error("Bogus internal file pointer!");
		/*NOTREACHED*/
	}

	/*
	 * return result
	 */
	return get_open_pos(fp, res);
}


int
ftellid(FILEID id, ZVALUE *res)
{
	FILEIO *fiop;
	FILEPOS fpos;		/* current file position */

	/* get FILEIO */
	fiop = findid(id, -1);
	if (fiop == NULL)
		return -2;

	/* get the file position */
	if (f_tell(fiop->fp, &fpos) < 0)
		return -3;

	/* convert file position to ZVALUE */
	*res = filepos2z(fpos);
	return 0;
}


int
fseekid(FILEID id, ZVALUE offset, int whence)
{
	FILEIO *fiop;		/* FILEIO of file */
	FILEPOS off;		/* offset as a FILEPOS */
	ZVALUE cur, tmp;	/* current or end of file location */
	int ret = 0;		/* return code */

	/* setup */
	fiop = findid(id, -1);
	if (fiop == NULL)
		return -2;

	/* seek depending on whence */
	switch (whence) {
	case 0:
		/* construct seek position, off = offset */
		if (zisneg(offset))
			return -3;
		off = z2filepos(offset);

		/* seek there */
		ret = f_seek_set(fiop->fp, &off);
		break;

	case 1:
		/* construct seek position, off = cur+offset */
		f_tell(fiop->fp, &off);
		cur = filepos2z(off);
		zadd(cur, offset, &tmp);
		zfree(cur);
		if (zisneg(tmp)) {
			zfree(tmp);
			return -3;
		}
		off = z2filepos(tmp);
		zfree(tmp);

		/* seek there */
		ret = f_seek_set(fiop->fp, &off);
		break;

	case 2:
		/* construct seek position, off = len+offset */
		if (get_open_siz(fiop->fp, &cur) < 0)
			return -4;
		zadd(cur, offset, &tmp);
		zfree(cur);
		if (zisneg(tmp)) {
			zfree(tmp);
			return -3;
		}
		off = z2filepos(tmp);
		zfree(tmp);

		/* seek there */
		ret = f_seek_set(fiop->fp, &off);
		break;

	default:
		return -5;
	}
	return ret;
}


/*
 * set_open_pos - set a an open file position
 *
 * given:
 *	fp		open file stream
 *	zpos		file position (ZVALUE) to set
 *
 * returns:
 *	0		res points to the file position
 *	-1		error
 *
 * NOTE: Due to fsetpos limitation, position is set relative to only
 *	 the beginning of the file.
 */
S_FUNC int
set_open_pos(FILE *fp, ZVALUE zpos)
{
	FILEPOS pos;		/* current file position */

	/*
	 * convert ZVALUE to file position
	 */
	pos = z2filepos(zpos);

	/*
	 * set the file position
	 */
	if (f_seek_set(fp, &pos) < 0) {
	    /* cannot set file position, return -1 */
	    return -1;
	}

	/*
	 * return success
	 */
	return 0;
}


/*
 * setloc - set the current position of the file
 *
 * given:
 *	id	file id of the file
 *	zpos	file position (ZVALUE) to set
 *
 * returns:
 *	0	able to set file position
 *	-1	unable to set file position
 */
int
setloc(FILEID id, ZVALUE zpos)
{
	FILEIO *fiop;		/* file structure */
	FILE *fp;

	/*
	 * firewall
	 */
	if ((id == FILEID_STDIN) || (id == FILEID_STDOUT) ||
	    (id == FILEID_STDERR)) {
		math_error("Cannot fseek stdin, stdout, or stderr");
		/*NOTREACHED*/
	}

	/*
	 * convert id to stream
	 */
	fiop = findid(id, -1);
	if (fiop == NULL) {
		/* file not open */
		return -1;
	}
	fp = fiop->fp;
	if (fp == NULL) {
		math_error("Bogus internal file pointer!");
		/*NOTREACHED*/
	}

	fiop->action = 0;

	/*
	 * return result
	 */
	return set_open_pos(fp, zpos);
}


/*
 * off_t2z - convert an off_t into a ZVALUE
 *
 * given:
 *	siz		file size
 *
 * returns:
 *	file size as a ZVALUE
 */
/*ARGSUSED*/
S_FUNC ZVALUE
off_t2z(off_t siz)
{
	ZVALUE ret;		/* ZVALUE file size to return */

	/*
	 * store off_t in a ZVALUE as a positive value
	 */
	ret.len = OFF_T_BITS/BASEB;
	ret.v = alloc(ret.len);
	zclearval(ret);
	SWAP_HALF_IN_OFF_T(ret.v, &siz);
	ret.sign = 0;
	ztrim(&ret);

	/*
	 * return our result
	 */
	return ret;
}


/*
 * dev2z - convert a stat.st_dev into a ZVALUE
 *
 * given:
 *	dev		device
 *
 * returns:
 *	file size as a ZVALUE
 */
S_FUNC ZVALUE
dev2z(dev_t dev)
{
	ZVALUE ret;		/* ZVALUE file size to return */

	/*
	 * store off_t in a ZVALUE as a positive value
	 */
	ret.len = DEV_BITS/BASEB;
	ret.v = alloc(ret.len);
	zclearval(ret);
	SWAP_HALF_IN_DEV(ret.v, &dev);
	ret.sign = 0;
	ztrim(&ret);

	/*
	 * return our result
	 */
	return ret;
}


/*
 * inode2z - convert a stat.st_ino into a ZVALUE
 *
 * given:
 *	inode		file size
 *
 * returns:
 *	file size as a ZVALUE
 */
/*ARGSUSED*/
S_FUNC ZVALUE
inode2z(ino_t inode)
{
	ZVALUE ret;		/* ZVALUE file size to return */

	/*
	 * store off_t in a ZVALUE as a positive value
	 */
	ret.len = INODE_BITS/BASEB;
	ret.v = alloc(ret.len);
	zclearval(ret);
	SWAP_HALF_IN_INODE(ret.v, &inode);
	ret.sign = 0;
	ztrim(&ret);

	/*
	 * return our result
	 */
	return ret;
}


/*
 * get_open_siz - get a an open file size
 *
 * given:
 *	fp		open file stream
 *	res		where to place the file size (ZVALUE)
 *
 * returns:
 *	0		res points to the file size
 *	-1		error
 */
int
get_open_siz(FILE *fp, ZVALUE *res)
{
	struct stat buf;	/* file status */

	/*
	 * get the file size
	 */
	if (fstat(fileno(fp), &buf) < 0) {
		/* stat error */
		return -1;
	}

	/*
	 * update file size and return success
	 */
	*res = off_t2z(buf.st_size);
	return 0;
}


/*
 * getsize - get the current size of the file
 *
 * given:
 *	id	file id of the file
 *	res	pointer to result
 *
 * returns:
 *	0		able to get file size
 *	EOF		system error
 *	other nonzero	file not open or other problem
 */
int
getsize(FILEID id, ZVALUE *res)
{
	FILEIO *fiop;		/* file structure */
	FILE *fp;

	/*
	 * convert id to stream
	 */
	fiop = findid(id, -1);
	if (fiop == NULL) {
		/* file not open */
		return 1;
	}
	fp = fiop->fp;
	if (fp == NULL) {
		return 2;
	}

	/*
	 * return result
	 */
	return get_open_siz(fp, res);
}


/*
 * getdevice - get the device of the file
 *
 * given:
 *	id	file id of the file
 *	dev	pointer to the result
 *
 * returns:
 *	0	able to get device
 *	-1	unable to get device
 */
int
get_device(FILEID id, ZVALUE *dev)
{
	FILEIO *fiop;		/* file structure */

	/*
	 * convert id to stream
	 */
	fiop = findid(id, -1);
	if (fiop == NULL) {
		/* file not open */
		return -1;
	}

	/*
	 * return result
	 */
	*dev = dev2z(fiop->dev);
	return 0;
}


/*
 * getinode - get the inode of the file
 *
 * given:
 *	id	file id of the file
 *	inode	pointer to the result
 *
 * returns:
 *	0	able to get inode
 *	-1	unable to get inode
 */
int
get_inode(FILEID id, ZVALUE *inode)
{
	FILEIO *fiop;		/* file structure */

	/*
	 * convert id to stream
	 */
	fiop = findid(id, -1);
	if (fiop == NULL) {
		/* file not open */
		return -1;
	}

	/*
	 * return result
	 */
	*inode = inode2z(fiop->inode);
	return 0;
}


S_FUNC off_t
filesize(FILEIO *fiop)
{
	struct stat sbuf;

	/* return length */
	if (fstat(fileno(fiop->fp), &sbuf) < 0) {
		math_error("bad fstat");
		/*NOTREACHED*/
	}
	return sbuf.st_size;
}


ZVALUE
zfilesize(FILEID id)
{
	FILEIO *fiop;
	off_t len;		/* file length */
	ZVALUE ret;		/* file size as a ZVALUE return value */

	/* file FILEIO */
	fiop = findid(id, -1);
	if (fiop == NULL) {
		/* return neg value for non-file error */
		itoz(-1, &ret);
		return ret;
	}

	/* get length */
	len = filesize(fiop);
	ret = off_t2z(len);
	return ret;
}


void
showfiles(void)
{
	BOOL listed[MAXFILES];
	FILEIO *fiop;
	FILE *fp;
	struct stat sbuf;
	ino_t inodes[MAXFILES];
	off_t sizes[MAXFILES];
	int i, j;

	for (i = 0; i < idnum; i++) {
		listed[i] = FALSE;
		fiop = &files[ioindex[i]];
		fp = fiop->fp;
		if (fstat(fileno(fp), &sbuf) < 0) {
			printf("Bad fstat for file %d\n", (int) fiop->id);
			sizes[i] = -1;
		} else {
			inodes[i] = sbuf.st_ino;
			sizes[i] = sbuf.st_size;
		}
	}
	for (i = 0; i < idnum; i++) {
		if (listed[i])
			continue;
		fiop = &files[ioindex[i]];
		printf("\t");
		printid(fiop->id, PRINT_UNAMBIG);
		if (sizes[i] == -1) {
			math_chr('\n');
			continue;
		}
		printf(" size = %lld\n", (long long int)sizes[i]);
		for (j = i + 1; j < idnum; j++) {
			if (listed[j] || sizes[j] == -1)
				continue;
			if (inodes[j] == inodes[i]) {
				listed[j] = TRUE;
				fiop = &files[ioindex[j]];
				printf("\t  = ");
				printid(fiop->id, PRINT_UNAMBIG);
				printf("\n");
			}
		}
	}
	printf("\tNumber open = %d\n", idnum);
	printf("\tLastid = %d\n", (int) lastid);
}


/*
 * getscanfield - scan a field separated by some characters
 *
 * given:
 *	fp		FILEID to scan
 *	skip
 *	width		max field width
 *	scannum		Number of characters in scanset
 *	scanptr		string of characters considered separators
 *	strptr		pointer to where the new field pointer may be found
 */
S_FUNC void
getscanfield(FILE *fp, BOOL skip, unsigned int width, int scannum,
	     char *scanptr, char **strptr)
{
	char *str;		/* current string */
	unsigned long len;	/* current length of string */
	unsigned long totlen;	/* total length of string */
	char buf[READSIZE];	/* temporary buffer */
	int c;
	char *b;
	BOOL comp;		/* Use complement of scanset */
	unsigned int chnum;

	totlen = 0;
	str = NULL;

	comp = (scannum < 0);
	if (comp)
		scannum = -scannum;

	chnum = 0;

	for (;;) {
		len = 0;
		b = buf;
		for(;;) {
			c = fgetc(fp);
			if (c == EOF || c == '\0')
				break;
			chnum++;
			if(scannum &&
			   ((memchr(scanptr,c,scannum)==NULL) ^ comp))
				break;
			if (!skip) {
				*b++ = c;
				len++;
				if (len >= READSIZE)
					break;
			}
			if (chnum == width)
				break;
		}
		if (!skip) {
			if (totlen)
				str = (char *) realloc(str, totlen + len + 1);
			else
				str = (char *) malloc(len + 1);
			if (str == NULL) {
				math_error("Out of memory for scanning");
				/*NOTREACHED*/
			}
			if (len)
				memcpy(&str[totlen], buf, len);
			totlen += len;
		}
		if (len < READSIZE)
			break;
	}

	if (!(width && chnum == width) && c != '\0')
		ungetc(c, fp);

	if (!skip) {
		str[totlen] = '\0';
		*strptr = str;
	}
}


/*
 * getscanwhite - scan a field separated by whitespace
 *
 * given:
 *	fp		FILEID to scan
 *	skip
 *	width		max field width
 *	scannum		Number of characters in scanset
 *	strptr		pointer to where the new field pointer may be found
 */
S_FUNC void
getscanwhite(FILE *fp, BOOL skip, unsigned int width, int scannum,
	     char **strptr)
{
	char *str;		/* current string */
	unsigned long len;	/* current length of string */
	unsigned long totlen;	/* total length of string */
	char buf[READSIZE];	/* temporary buffer */
	int c;
	char *b;
	BOOL comp;		/* Use complement of scanset */
	unsigned int chnum;

	totlen = 0;
	str = NULL;

	comp = (scannum < 0);
	if (comp)
		scannum = -scannum;

	chnum = 0;

	for (;;) {
		len = 0;
		b = buf;
		for(;;) {
			c = fgetc(fp);
			if (c == EOF || c == '\0')
				break;
			chnum++;
			if(scannum && (!isspace(c) ^ comp))
				break;
			if (!skip) {
				*b++ = c;
				len++;
				if (len >= READSIZE)
					break;
			}
			if (chnum == width)
				break;
		}
		if (!skip) {
			if (totlen)
				str = (char *) realloc(str, totlen + len + 1);
			else
				str = (char *) malloc(len + 1);
			if (str == NULL) {
				math_error("Out of memory for scanning");
				/*NOTREACHED*/
			}
			if (len)
				memcpy(&str[totlen], buf, len);
			totlen += len;
		}
		if (len < READSIZE)
			break;
	}

	if (!(width && chnum == width) && c != '\0')
		ungetc(c, fp);

	if (!skip) {
		str[totlen] = '\0';
		*strptr = str;
	}
}


S_FUNC int
fscanfile(FILE *fp, char *fmt, int count, VALUE **vals)
{
	int assnum;	/* Number of assignments made */
	int c;		/* Character read from file */
	char f;		/* Character read from format string */
	int scannum;	/* Number of characters in scanlist */
	char *scanptr;	/* Start of scanlist */
	char *str;
	BOOL comp;	/* True scanset is complementary */
	BOOL skip;	/* True if string to be skipped rather than read */
	int width;
	VALUE *var;	/* lvalue to be assigned to */
	unsigned short subtype;	/* for var->v_subtype */
	FILEPOS cur;	/* current location */

	if (feof(fp))
		return EOF;

	assnum = 0;

	for (;;) {
		for (;;) {
			f = *fmt++;
			if (isspace((int)f)) {
				getscanwhite(fp,1,0,6,NULL);
				do {
					f = *fmt++;
				} while (isspace((int)f));
			}
			c = fgetc(fp);
			if (c == EOF)
				return assnum;
			if (f == '%') {
				f = *fmt++;
				if (f != '%' && f != '\0')
					break;
			}
			if (f != c || f == '\0') {
				ungetc(c, fp);
				return assnum;
			}
		}
		ungetc(c, fp);
		skip = (f == '*');
		if (!skip && count == 0) {
			return assnum;
		}
		if (skip)
			f = *fmt++;
		width = 0;
		while (f >= '0' && f <= '9') {
			width = 10 * width + f - '0';
			f = *fmt++;
		}
		switch (f) {
		case 'c':
			if (width == 0)
				width = 1;
			getscanfield(fp,skip,width,0,NULL,&str);
			break;
		case 's':
			getscanwhite(fp,1,0,6,NULL);
			if (feof(fp))
				return assnum;
			getscanwhite(fp,skip,width,-6,&str);
			break;
		case '[':
			f = *fmt;
			comp = (f == '^');
			if (comp)
				f = *++fmt;
			scanptr = fmt;
			if (f == '\0')
				return assnum;
			fmt = strchr((f == ']' ? fmt + 1 : fmt), ']');
			if (fmt == NULL)
				return assnum;
			scannum = fmt - scanptr;
			if (comp)
				scannum = -scannum;
			fmt++;
			getscanfield(fp,skip,
				 width,scannum,scanptr,&str);
			break;
		case 'f':
		case 'e':
		case 'r':
		case 'i':
			getscanwhite(fp,1,0,6, NULL);
			if (feof(fp))
				return assnum;
			if (skip) {
				fskipnum(fp);
				continue;
			}
			assnum++;
			var = *vals++;
			if (var->v_type != V_ADDR)
			math_error("This should not happen!!");
			var = var->v_addr;
			subtype = var->v_subtype;
			freevalue(var);
			count--;
			freadsum(fp, var);
			var->v_subtype = subtype;
			continue;
		case 'n':
			assnum++;
			var = *vals++;
			count--;
			if (var->v_type != V_ADDR)
				math_error("This should not happen!!");
			var = var->v_addr;
			subtype = var->v_subtype;
			freevalue(var);
			var->v_type = V_NUM;
			var->v_num = qalloc();
			f_tell(fp, &cur);
			var->v_num->num = filepos2z(cur);
			var->v_subtype = subtype;
			continue;
		default:
			fprintf(stderr, "Unsupported scan specifier");
			return assnum;
		}
		if (!skip) {
			assnum++;
			var = *vals++;
			count--;
			if (var->v_type != V_ADDR)
				math_error("Assigning to nonvariable");
			var = var->v_addr;
			subtype = var->v_subtype;
			freevalue(var);
			var->v_type = V_STR;
			var->v_str = makestring(str);
		}
	}
}


int
fscanfid(FILEID id, char *fmt, int count, VALUE **vals)
{
	FILEIO *fiop;
	FILE *fp;
	FILEPOS fpos;

	fiop = findid(id, FALSE);
	if (fiop == NULL)
		return -2;

	fp = fiop->fp;

	if (fiop->action == 'w') {
		f_tell(fp, &fpos);
		fflush(fp);
		if (f_seek_set(fp, &fpos) < 0)
			return -4;
	}
	fiop->action = 'r';

	return fscanfile(fp, fmt, count, vals);
}


int
scanfstr(char *str, char *fmt, int count, VALUE **vals)
{
	FILE *fp;
	int i;

	fp = tmpfile();
	if (fp == NULL)
		return EOF;
	fputs(str, fp);
	rewind(fp);
	i = fscanfile(fp, fmt, count, vals);
	fclose(fp);
	return i;
}


/*
 * Read a number in floating-point format from a file.	The first dot,
 * if any, is considered as the decimal point; later dots are ignored.
 * For example, -23.45..67. is interpreted as -23.4567
 * An optional 'e' or 'E' indicates multiplication by a power or 10,
 * e.g. -23.45e-6 has the effect of -23.45 * 10^-6.  The reading
 * ceases when a character other than a digit, a leading sign,
 * a sign immediately following 'e' or 'E', or a dot is encountered.
 * Absence of digits is interpreted as zero.
 */
S_FUNC void
freadnum(FILE *fp, VALUE *valptr)
{
	ZVALUE num, zden, newnum, newden, div, tmp;
	NUMBER *q;
	COMPLEX *c;
	VALUE val;
	char ch;
	LEN i;
	HALF *a;
	FULL f;
	long decimals, exp;
	BOOL sign, negexp, havedp, imag, exptoobig;

	decimals = 0;
	exp = 0;
	sign = FALSE;
	negexp = FALSE;
	havedp = FALSE;
	imag = FALSE;
	exptoobig = FALSE;

	ch = fgetc(fp);
	if (ch == '+' || ch == '-') {
		if (ch == '-')
			sign = TRUE;
		ch = fgetc(fp);
	}
	num.v = alloc(1);
	*num.v = 0;
	num.len = 1;
	num.sign = sign;
	for (;;) {
		if (ch >= '0' && ch <= '9') {
			f = (FULL) (ch - '0');
			a = num.v;
			i = num.len;
			while (i-- > 0) {
				f = 10 * (FULL) *a + f;
				*a++ = (HALF) f;
				f >>= BASEB;
			}
			if (f) {
				a = alloc(num.len + 1);
				memcpy(a, num.v, num.len * sizeof(HALF));
				a[num.len] = (HALF) f;
				num.len++;
				freeh(num.v);
				num.v = a;
			}
			if (havedp)
				decimals++;
		}
		else if (ch == '.')
			havedp = TRUE;
		else
			break;
		ch = fgetc(fp);
	}
	if (ch == 'e' || ch == 'E') {
		ch = fgetc(fp);
		if (ch == '+' || ch == '-') {
			if (ch == '-')
				negexp = TRUE;
			ch = fgetc(fp);
		}
		while (ch >= '0' && ch <= '9') {
			if (!exptoobig) {
				exp = (exp * 10) + ch - '0';
				if (exp > 1000000)
					exptoobig = TRUE;
			}
			ch = fgetc(fp);
		}
	}
	if (ch == 'i' || ch == 'I') {
		imag = TRUE;
	} else {
		ungetc(ch, fp);
	}

	if (ziszero(num)) {
		zfree(num);
		val.v_type = V_NUM;
		val.v_subtype = V_NOSUBTYPE;
		val.v_num = qlink(&_qzero_);
		*valptr = val;
		return;
	}
	if (exptoobig) {
		zfree(num);
		*valptr = error_value(E_BIGEXP);
		return;
	}
	ztenpow(decimals, &zden);
	if (exp) {
		ztenpow(exp, &tmp);
		if (negexp) {
			zmul(zden, tmp, &newden);
			zfree(zden);
			zden = newden;
		} else {
			zmul(num, tmp, &newnum);
			zfree(num);
			num = newnum;
		}
		zfree(tmp);
	}
	if (!zisunit(num) && !zisunit(zden)) {
		zgcd(num, zden, &div);
		if (!zisunit(div)) {
			zequo(num, div, &newnum);
			zfree(num);
			zequo(zden, div, &newden);
			zfree(zden);
			num = newnum;
			zden = newden;
		}
		zfree(div);
	}
	q = qalloc();
	q->num = num;
	q->den = zden;
	if (imag) {
		c = comalloc();
		qfree(c->imag);
		c->imag = q;
		val.v_type = V_COM;
		val.v_com = c;
	} else {
		val.v_type = V_NUM;
		val.v_num = q;
	}
	val.v_subtype = V_NOSUBTYPE;
	*valptr = val;
}


S_FUNC void
freadsum(FILE *fp, VALUE *valptr)
{
	VALUE v1, v2, v3;
	char ch;


	freadprod(fp, &v1);

	ch = fgetc(fp);
	while (ch == '+' || ch == '-') {
		freadprod(fp, &v2);
		if (ch == '+')
			addvalue(&v1, &v2, &v3);
		else
			subvalue(&v1, &v2, &v3);
		freevalue(&v1);
		freevalue(&v2);
		v1 = v3;
		ch = fgetc(fp);
	}
	ungetc(ch, fp);
	*valptr = v1;
}


S_FUNC void
freadprod(FILE *fp, VALUE *valptr)
{
	VALUE v1, v2, v3;
	char ch;

	freadnum(fp, &v1);
	ch = fgetc(fp);
	while (ch == '*' || ch == '/') {
		freadnum(fp, &v2);
		if (ch == '*')
			mulvalue(&v1, &v2, &v3);
		else
			divvalue(&v1, &v2, &v3);
		freevalue(&v1);
		freevalue(&v2);
		v1 = v3;
		ch = fgetc(fp);
	}
	ungetc(ch, fp);
	*valptr = v1;
}


S_FUNC void
fskipnum(FILE *fp)
{
	char ch;

	do {
		ch = fgetc(fp);
		if (ch == '+' || ch == '-')
			ch = fgetc(fp);
		while ((ch >= '0' && ch <= '9') || ch == '.')
			ch = fgetc(fp);
		if (ch == 'e' || ch == 'E') {
			ch = fgetc(fp);
			if (ch == '+' || ch == '-')
				ch = fgetc(fp);
			while (ch >= '0' && ch <= '9')
				ch = fgetc(fp);
		}
		if (ch == 'i' || ch == 'I')
			ch = fgetc(fp);
	} while (ch == '/' || ch == '*' || ch == '+' || ch == '-');

	ungetc(ch, fp);
}


int
isattyid(FILEID id)
{
	FILEIO *fiop;

	fiop = findid(id, -1);
	if (fiop == NULL)
		return -2;
	return isatty(fileno(fiop->fp));
}


/*
 * fsearch - search for a string in a file
 *
 * given:
 *	id	FILEID to search
 *	str	string to look for
 *	pos	file position to start at (NULL => current position)
 *
 * returns:
 *	EOF if system error
 *	other negative integer if file not open, etc.
 *	positive integer if string not found
 *	zero if string found, position stored at res
 *
 * XXX - This search is a translation of the original search that did not
 *	 work with large files.	 The search algorithm used is slow and
 *	 should be speed up much more.
 */
int
fsearch(FILEID id, char *str, ZVALUE start, ZVALUE end, ZVALUE *res)
{
	FILEIO *fiop;		/* FILEIO of file id */
	FILEPOS cur;		/* current file position */
	ZVALUE tmp, tmp2;	/* temporary ZVALUEs */
	char c;			/* str comparison character */
	int r;			/* character read from file */
	char *s;		/* str comparison pointer */
	long k = 0;

	/* get FILEIO */
	fiop = findid(id, FALSE);
	if (fiop == NULL)
		return -2;

	/*
	 * file setup
	 */
	if (fiop->action == 'w')
		fflush(fiop->fp);

	zsub(end, start, &tmp2);

	if (zisneg(tmp2)) {
		zfree(tmp2);
		return 1;
	}

	tmp.sign  = 0;
	tmp.len = tmp2.len;
	tmp.v = alloc(tmp.len);
	zcopyval(tmp2, tmp);
	zfree(tmp2);

	cur = z2filepos(start);

	if (f_seek_set(fiop->fp, &cur) < 0) {
		zfree(tmp);
		return EOF;
	}

	/*
	 * search setup
	 */
	/* note the first str search character */
	c = *str++;

	if (c == '\0') {
		zfree(tmp);
		return 2;
	}
	clearerr(fiop->fp);
	while ((r = fgetc(fiop->fp)) != EOF) {
		if ((char)r == c) {
			(void) f_tell(fiop->fp, &cur);
			s = str;
			while (*s) {
				r = fgetc(fiop->fp);
				if ((char)r != *s)
					break;
				s++;
			}
			if (r == EOF)
				break;
			if (*s == '\0') {
				zfree(tmp);
				tmp = filepos2z(cur);
				zsub(tmp, _one_, res);
				zfree(tmp);
				return 0;
			}
			(void) f_seek_set(fiop->fp, &cur);
		}
		if (*tmp.v) {
			(*tmp.v)--;
		} else {
			if (tmp.len == 1)
				break;
			k = 0;
			do {
				tmp.v[k++] = BASE1;
			}
			while (k < tmp.len && tmp.v[k] == 0);
			if (k == tmp.len) {
				math_error("This should not happen");
				/*NOTREACHED*/
			}
			tmp.v[k]--;
			if (tmp.v[tmp.len - 1] == 0)
				tmp.len--;
		}
	}
	zfree(tmp);
	if (ferror(fiop->fp))
		return EOF;
	return 1;
}


/*
 * frsearch - reverse search for a string in a file
 *
 * given:
 *	id	FILEID to search
 *	str	string to look for
 *	search starts at pos = first and continues for decreasing
 *		pos >= last
 *
 * returns:
 *	EOF if system error
 *	other negative integer if file not open, etc.
 *	positive integer if string not found
 *	zero if string found, position stored at res
 *
 * XXX - This search is a translation of the original search that did not
 *	 work with large files.	 The search algorithm used is so slow
 *	 as to be painful to the user and needs to be sped up much more.
 */
int
frsearch(FILEID id, char *str, ZVALUE first, ZVALUE last, ZVALUE *res)
{
	FILEIO *fiop;		/* FILEIO of file id */
	FILEPOS cur;		/* current file position */
	ZVALUE pos;		/* current file position as ZVALUE */
	ZVALUE tmp;		/* temporary ZVALUEs */
	char c;			/* str comparison character */
	int r;			/* character read from file */
	char *s;		/* str comparison pointer */

	/* get FILEIO */
	fiop = findid(id, FALSE);
	if (fiop == NULL)
		return -2;

	/*
	 * file setup
	 */
	if (fiop->action == 'w')
		fflush(fiop->fp);

	zcopy(first, &pos);

	/*
	 * search setup
	 */
	/* note the first str search character */
	c = *str++;

	if (c == '\0') {
		cur = z2filepos(pos);
		if (f_seek_set(fiop->fp, &cur) < 0) {
			zfree(pos);
			return EOF;
		}
		*res = pos;
		return 0;
	}

	clearerr(fiop->fp);

	while(zrel(pos, last) >= 0) {
		cur = z2filepos(pos);
		if (f_seek_set(fiop->fp, &cur) < 0) {
			zfree(pos);
			return EOF;
		}
		r = fgetc(fiop->fp);
		if (r == EOF) {
			zfree(pos);
			return EOF;
		}
		if ((char) r == c) {
			s = str;
			while (*s) {
				r = fgetc(fiop->fp);
				if ((char)r != *s)
					break;
				s++;
			}
			if (r == EOF) {
				zfree(pos);
				return EOF;
			}
			if (*s == '\0') {
				*res = pos;
				ungetc(r, fiop->fp);
				return 0;
			}
		}
		zsub(pos, _one_, &tmp);
		zfree(pos);
		pos = tmp;
	}
	cur = z2filepos(last);
	f_seek_set(fiop->fp, &cur);
	zfree(pos);
	if (ferror(fiop->fp))
		return EOF;
	return 1;
}


char *
findfname(FILEID id)
{
	FILEIO *fiop;

	fiop = findid(id, -1);

	if (fiop == NULL)
		return NULL;

	return fiop->name;
}
