/*
 * file - file I/O routines callable by users
 *
 * Copyright (C) 1999-2007,2018,2021-2023,2026  David I. Bell and Landon Curt Noll
 *
 * Primary author:  David I. Bell
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
 * Under source code control:   1991/07/20 00:21:56
 * File existed as early as:    1991
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

/*
 * conditional <system> head includes
 */
#if defined(_WIN32) || defined(_WIN64)
#  include <io.h>
#endif

/*
 * calc local src includes
 */
#include "value.h"
#include "calc.h"
#include "file.h"
#include "strl.h"
#include "attribute.h"
#include "errtbl.h"

#include "banned.h" /* include after all other includes */

#define READSIZE 1024 /* buffer size for reading */

#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

/*
 * external STDIO functions
 */
extern void math_setfp(FILE *fp);
extern FILE *f_open(char *name, char *mode);

/*
 * Table of opened files.
 * The first three entries always correspond to stdin, stdout, and stderr,
 * and cannot be closed.  Their file ids are always 0, 1, and 2.
 */
static FILEIO files[MAXFILES] = {{FILEID_STDIN, NULL, (dev_t)0, (ino_t)0, "(stdin)", true, false, false, false, 'r', "r"},
                                 {FILEID_STDOUT, NULL, (dev_t)0, (ino_t)0, "(stdout)", false, true, false, false, 'w', "w"},
                                 {FILEID_STDERR, NULL, (dev_t)0, (ino_t)0, "(stderr)", false, true, false, false, 'w', "w"}};

static int ioindex[MAXFILES] = {0, 1, 2}; /* Indices for FILEIO table */
static FILEID lastid = FILEID_STDERR;     /* Last allocated file id */
static int idnum = 3;                     /* Number of allocated file ids */

/*
 * off_t range limits - used by in_range_off_t()
 */
static bool range_set = false;          /* true ==> zoff_t_min and zoff_t_max setup */
static ZVALUE zoff_t_min;               /* range_set ==> zoff_t_min is OFF_T_MIN */
static ZVALUE zoff_t_max;               /* range_set ==> zoff_t_nax is OFF_T_MAX */

/* forward static declarations */
static void free_range_off_t(void);
static bool in_range_off_t(ZVALUE zpos);
static int set_open_pos(FILE *fp, ZVALUE zpos);
static int get_open_pos(FILE *fp, ZVALUE *res);
static int ftello_stream(FILE *fp, off_t *fposp);
static off_t z2off_t(ZVALUE zpos);
static ZVALUE off_t2z(off_t siz);
static ZVALUE dev2z(dev_t dev);
static ZVALUE inode2z(ino_t inode);
static void getscanfield(FILE *fp, bool skip, unsigned int width, int scannum, char *scanptr, char **strptr);
static void getscanwhite(FILE *fp, bool skip, unsigned int width, int scannum, char **strptr);
static int fscanfile(FILE *fp, char *fmt, int count, VALUE **vals);
static void freadnum(FILE *fp, VALUE *valptr);
static void freadsum(FILE *fp, VALUE *valptr);
static void freadprod(FILE *fp, VALUE *valptr);
static void fskipnum(FILE *fp);

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
    static int done = 0; /* 1 => routine already called */
    struct stat sbuf;    /* file status */
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
            files[idnum].reading = true;
            files[idnum].writing = true;
            files[idnum].action = 0;
            memset(files[idnum].mode, 0, MODE_LEN + 1);
            /*
             * stat the descriptor to see what we have
             */
            if (fstat(i, &sbuf) >= 0) {
                size_t snprintf_len;          /* malloced snprintf length */
                fp = (FILE *)fdopen(i, "r+"); /*guess mode*/
                if (fp) {
                    strlcpy(files[idnum].mode, "r+", sizeof(files[idnum].mode));
                } else {
                    fp = (FILE *)fdopen(i, "r");
                    if (fp) {
                        strlcpy(files[idnum].mode, "r", sizeof(files[idnum].mode));
                        files[idnum].writing = false;
                    } else {
                        fp = (FILE *)fdopen(i, "w");
                        if (fp) {
                            strlcpy(files[idnum].mode, "w", sizeof(files[idnum].mode));
                            files[idnum].reading = false;
                        } else {
                            continue;
                        }
                    }
                }
                snprintf_len = sizeof("descriptor[12345678901234567890]") + 1;
                tname = (char *)calloc(snprintf_len + 1, 1);
                if (tname == NULL) {
                    math_error("Out of memory for init_file");
                    not_reached();
                }
                snprintf(tname, snprintf_len, "descriptor[%d]", i);
                tname[snprintf_len] = '\0'; /* paranoia */
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
 *      fiop    pointer to FILEIO structure to initialize
 *      name    associated filename (NULL => caller will setup filename)
 *      mode    open mode (one of {r,w,a}{,b}{,+})
 *      sbufp   pointer to stat of open file
 *      id      calc file ID
 *      fp      open file stream
 */
static void
init_fileio(FILEIO *fiop, char *name, char *mode, struct stat *sbufp, FILEID id, FILE *fp)
{
    char modestr[MODE_LEN + 1]; /* mode [rwa]b?\+? */
    size_t namelen;             /* length of name */

    /* clear modestr */
    memset(modestr, 0, sizeof(modestr));

    /* allocate filename if requested */
    namelen = 0;
    if (name != NULL) {
        namelen = strlen(name);
        fiop->name = (char *)calloc(namelen + 1, 1);
        if (fiop->name == NULL) {
            math_error("No memory for filename");
            not_reached();
        }
    }

    /* initialize FILEIO structure */
    if (name != NULL) {
        strlcpy(fiop->name, name, namelen + 1);
    }
    fiop->id = id;
    fiop->fp = fp;
    fiop->dev = sbufp->st_dev;
    fiop->inode = sbufp->st_ino;
    fiop->reading = false;
    fiop->writing = false;
    fiop->appending = false;
    fiop->binary = false;
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
        fiop->reading = true;

        /* note binary mode even though mode is not used / ignored */
        if (strchr(mode, 'b') != NULL) {
            strlcat(modestr, "b", sizeof(modestr));
        }

        /* note if reading and writing */
        if (strchr(mode, '+') != NULL) {
            fiop->writing = true;
            strlcat(modestr, "+", sizeof(modestr));
        }

        /* canonicalize write modes */
    } else if (mode[0] == 'w') {

        /* note write mode */
        strlcpy(modestr, "w", sizeof(modestr));
        fiop->writing = true;

        /* note binary mode even though mode is not used / ignored */
        if (strchr(mode, 'b') != NULL) {
            strlcat(modestr, "b", sizeof(modestr));
        }

        /* note if reading and writing */
        if (strchr(mode, '+') != NULL) {
            fiop->reading = true;
            strlcat(modestr, "+", sizeof(modestr));
        }

        /* canonicalize append modes */
    } else if (mode[0] == 'a') {

        /* note append mode */
        strlcpy(modestr, "a", sizeof(modestr));
        fiop->writing = true;
        fiop->appending = true;

        /* note binary mode even though mode is not used / ignored */
        if (strchr(mode, 'b') != NULL) {
            strlcat(modestr, "b", sizeof(modestr));
        }

        /* note if reading and writing */
        if (strchr(mode, '+') != NULL) {
            fiop->reading = true;
            strlcat(modestr, "+", sizeof(modestr));
        }

        /* canonicalize no I/O modes */
    } else {
        modestr[0] = '\0';
    }
    modestr[MODE_LEN] = '\0'; /* firewall */

    /* record canonical open mode string */
    strlcpy(fiop->mode, modestr, sizeof(fiop->mode));
}

/*
 * openid - open the specified file name for reading or writing
 *
 * given:
 *      name            file name
 *      mode            open mode (one of {r,w,a}{,b}{,+})
 *
 * returns:
 *      >=3 FILEID which can be used to do I/O to the file
 *      <0 if the open failed
 *
 * NOTE: This function will not return 0, 1 or 2 since they are
 *       reserved for stdin, stdout, stderr.  In fact, it must not
 *       return 0, 1, or 2 because it will confuse those who call
 *       the opensearchfile() function
 */
FILEID
openid(char *name, char *mode)
{
    FILEIO *fiop; /* file structure */
    FILEID id;    /* new file id */
    FILE *fp;
    struct stat sbuf; /* file status */
    int i;

    /* find the next open slot in the files array */
    if (idnum >= MAXFILES) {
        return -E_MANYOPEN;
    }
    fiop = &files[3];
    for (i = 3; i < MAXFILES; fiop++, i++) {
        if (fiop->name == NULL) {
            break;
        }
    }
    if (i == MAXFILES) {
        math_error("This should not happen in openid()!!!");
    }

    /* open the file */
    fp = f_open(name, mode);
    if (fp == NULL) {
        return FILEID_NONE;
    }
    if (fstat(fileno(fp), &sbuf) < 0) {
        math_error("bad fstat");
        not_reached();
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
 * openpathid - open the specified base filename, or
 *              relative filename along a search path
 *
 * given:
 *      name            file name
 *      mode            open mode (one of {r,w,a}{,b}{,+})
 *      pathlist        list of colon separated paths (or NULL)
 *
 * returns:
 *      >=3 FILEID which can be used to do I/O to the file
 *      <0 if the open failed
 *
 * NOTE: This function will not return 0, 1 or 2 since they are
 *       reserved for stdin, stdout, stderr.  In fact, it must not
 *       return 0, 1, or 2 because it will confuse those who call
 *       the opensearchfile() function
 */
FILEID
openpathid(char *name, char *mode, char *pathlist)
{
    FILEIO *fiop; /* file structure */
    FILEID id;    /* new file id */
    FILE *fp;
    struct stat sbuf; /* file status */
    char *openpath;   /* malloc copy of path that was opened */
    int i;

    /* find the next open slot in the files array */
    if (idnum >= MAXFILES) {
        return -E_MANYOPEN;
    }
    fiop = &files[3];
    for (i = 3; i < MAXFILES; fiop++, i++) {
        if (fiop->name == NULL) {
            break;
        }
    }
    if (i == MAXFILES) {
        math_error("This should not happen in openpathid()!!!");
    }

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
        not_reached();
    }
    if (openpath == NULL) {
        fclose(fp);
        math_error("bad openpath");
        not_reached();
    }

    /* get a new FILEID */
    id = ++lastid;
    ioindex[idnum++] = i;

    /* initialize FILEIO structure */
    init_fileio(fiop, NULL, mode, &sbuf, id, fp);
    fiop->name = openpath; /* already malloced by f_pathopen */

    /* return calc open file ID */
    return id;
}

/*
 * reopenid - reopen a FILEID
 *
 * given:
 *      id      FILEID to reopen
 *      mode    new mode to open as
 *      mode    new mode to open as (one of "r", "w", "a", "r+", "w+", "a+")
 *      name    name of new file
 *
 * returns:
 *      FILEID which can be used to do I/O to the file
 *      <0 if the open failed
 */
FILEID
reopenid(FILEID id, char *mode, char *name)
{
    FILEIO *fiop; /* file structure */
    FILE *fp;
    struct stat sbuf;
    int i;

    /* firewall */
    if ((id == FILEID_STDIN) || (id == FILEID_STDOUT) || (id == FILEID_STDERR)) {
        math_error("Cannot freopen stdin, stdout, or stderr");
        not_reached();
    }

    /* reopen the file */
    fiop = NULL;
    for (i = 3; i < idnum; i++) {
        fiop = &files[ioindex[i]];
        if (fiop->id == id) {
            break;
        }
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
            if (fiop->name == NULL) {
                break;
            }
        }
        if (i >= MAXFILES) {
            math_error("This should not happen in reopenid");
            not_reached();
        }
        fp = f_open(name, mode);
        if (fp == NULL) {
            fprintf(stderr, "Cannot open file\n");
            return FILEID_NONE;
        }
        ioindex[idnum++] = i;
        fiop->id = id;
    } else {
        (void)fclose(fiop->fp);
        if (name == NULL) {
            fp = f_open(fiop->name, mode);
        } else {
            fp = f_open(name, mode);
        }
        if (fp == NULL) {
            free(fiop->name);
            fiop->name = NULL;
            idnum--;
            for (; i < idnum; i++) {
                ioindex[i] = ioindex[i + 1];
            }
            return FILEID_NONE;
        }
    }
    if (fstat(fileno(fp), &sbuf) < 0) {
        math_error("bad fstat");
        not_reached();
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
    FILEIO *fiop; /* file structure */
    int i;

    fiop = NULL;

    if ((id < 0) || (id > lastid)) {
        return NULL;
    }

    for (i = 0; i < idnum; i++) {
        fiop = &files[ioindex[i]];
        if (fiop->id == id) {
            break;
        }
    }

    if (i == idnum) {
        return NULL;
    }

    if (writable >= 0) {
        if ((writable && !fiop->writing) || (!writable && !fiop->reading)) {
            return NULL;
        }
    }
    return fiop;
}

/*
 * Return whether or not a file id is valid.  This is used for if tests.
 */
bool
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

    id = (FILEID)index;

    if ((index < 0) || (id > lastid)) {
        return FILEID_NONE;
    }
    return id;
}

/*
 * Close the specified file id.  Returns true if there was an error.
 * Closing of stdin, stdout, or stderr is illegal, but closing of already
 * closed files is allowed.
 */
int
closeid(FILEID id)
{
    FILEIO *fiop; /* file structure */
    int i;
    int err;

    fiop = NULL;

    /* firewall */
    if ((id == FILEID_STDIN) || (id == FILEID_STDOUT) || (id == FILEID_STDERR)) {
        math_error("Cannot close stdin, stdout, or stderr");
        not_reached();
    }

    /* get file structure */
    for (i = 3; i < idnum; i++) {
        fiop = &files[ioindex[i]];
        if (fiop->id == id) {
            break;
        }
    }
    if (i == idnum) {
        return 1; /* File not open */
    }
    idnum--;
    for (; i < idnum; i++) {
        ioindex[i] = ioindex[i + 1];
    }

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
bool
errorid(FILEID id)
{
    FILEIO *fiop; /* file structure */

    fiop = findid(id, -1);
    if (fiop == NULL) {
        return EOF;
    }
    return (ferror(fiop->fp) != 0);
}

/*
 * Return whether or not end of file occurred to a file.
 */
bool
eofid(FILEID id)
{
    FILEIO *fiop; /* file structure */

    fiop = findid(id, -1);
    if (fiop == NULL) {
        return EOF;
    }
    return (feof(fiop->fp) != 0);
}

/*
 * Flush output to an opened file.
 */
int
flushid(FILEID id)
{
    FILEIO *fiop; /* file structure */

    fiop = findid(id, -1);
    if (fiop == NULL) {
        return 0;
    }
    if (!fiop->writing || fiop->action == 'r') {
        return 0;
    }
    return fflush(fiop->fp);
}

#if !defined(_WIN32) && !defined(_WIN64)

int
flushall(void)
{
    FILEIO *fiop;
    int i;
    int err;

    err = 0;
    for (i = 3; i < idnum; i++) {
        fiop = &files[ioindex[i]];
        if (fiop->writing && fiop->action != 'r') {
            err |= fflush(fiop->fp);
        }
    }
    return err;
}

#endif

/*
 * Read the next line, string or word from an opened file.
 * Returns a pointer to an allocated string holding a null-terminated
 * or newline terminated string.  Where reading stops is controlled by
 * flags:
 *
 *      bit 0:  at newline
 *      bit 1:  at null character
 *      bit 2:  at white space (also skips leading white space)
 *
 * If neither '\n' nor '\0' is encountered reading continues until EOF.
 * If bit 3 is set the stop character is removed.
 *
 * given:
 *      id              file to read from
 *      flags           read flags (see above)
 *      retstr          returned pointer to string
 */
int
readid(FILEID id, int flags, STRING **retstr)
{
    FILEIO *fiop; /* file structure */
    FILE *fp;
    char *str;            /* current string */
    unsigned long n;      /* current number characters read into buf */
    unsigned long totlen; /* total length of string copied from buf */
    char buf[READSIZE];   /* temporary buffer */
    char *b;
    int c;
    bool nlstop, nullstop, wsstop, rmstop, done;
    STRING *newstr;

    totlen = 0;
    str = NULL;

    fiop = findid(id, false);
    if (fiop == NULL) {
        return 1;
    }
    nlstop = (flags & 1);
    nullstop = (flags & 2);
    wsstop = (flags & 4);
    rmstop = (flags & 8);

    fp = fiop->fp;

    if (fiop->action == 'w') {
        fpos_t fpos;    /* current location */

        /*
         * flush and attempt to restore the current location
         */
        fgetpos(fp, &fpos);
        fflush(fp);
        if (fsetpos(fp, &fpos) < 0) {
            return 3;
        }
    }
    fiop->action = 'r';

    if (wsstop) {
        while (isspace(c = fgetc(fp)))
            ;
        ungetc(c, fp);
    }

    for (;;) {
        b = buf;
        n = 0;
        do {
            c = fgetc(fp);
            if (c == EOF) {
                break;
            }
            n++;
            if (nlstop && c == '\n') {
                break;
            }
            if (nullstop && c == '\0') {
                break;
            }
            if (wsstop && isspace(c)) {
                break;
            }
            *b++ = c;
        } while (n < READSIZE);
        done = ((nlstop && c == '\n') || (nullstop && c == '\0') || (wsstop && isspace(c)) || c == EOF);
        if (done && rmstop && c != EOF) {
            n--;
        }
        if (totlen) {
            str = (char *)realloc(str, totlen + n + 1);
        } else {
            str = (char *)calloc(n + 1, 1);
        }
        if (str == NULL) {
            math_error("Out of memory for readid");
            not_reached();
        }
        if (n > 0) {
            memcpy(&str[totlen], buf, n);
        }
        totlen += n;
        if (done) {
            break;
        }
    }
    if (totlen == 0 && c == EOF) {
        free(str);
        return EOF;
    }
    if ((nlstop && c == '\n') && !rmstop) {
        str[totlen - 1] = '\n';
    }
    if ((nullstop && c == '\0') && !rmstop) {
        str[totlen - 1] = '\0';
    }
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

    fiop = findid(id, false);
    if (fiop == NULL) {
        return -2;
    }
    if (fiop->action == 'w') {
        fpos_t fpos;    /* current location */

        /*
         * flush and attempt to restore the current location
         */
        fgetpos(fiop->fp, &fpos);
        fflush(fiop->fp);
        if (fsetpos(fiop->fp, &fpos) < 0) {
            return -3;
        }
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
    FILEIO *fiop; /* file structure */
    FILE *fp;
    ZVALUE pos; /* file position */

    /*
     * filewall - file is closed
     */
    fiop = findid(id, -1);
    if (fiop == NULL) {
        if (flags & PRINT_UNAMBIG) {
            math_fmt("FILE %ld closed", id);
        } else {
            math_str("\"\"");
        }
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
    math_fmt("FILE %ld \"%s\" (%s", id, fiop->name, fiop->mode);

    /*
     * print file position
     */

    fp = fiop->fp;

    if (get_open_pos(fp, &pos) < 0) {
        if (fileno(fp) > 2) {
            math_str("Error while determining file position!");
        }
        math_chr(')');
        return 0;
    }

    math_str(", pos ");
    zprintval(pos, 0, 0);
    zfree(pos);

    /*
     * report special status
     */
    if (ferror(fp)) {
        math_str(", error");
    }
    if (feof(fp)) {
        math_str(", eof");
    }
    math_chr(')');

    printf(" fileno: %d ", fileno(fp));
    return 0;
}

/*
 * Print a formatted string similar to printf.  Various formats of output
 * are possible, depending on the format string AND the actual types of the
 * values.  Mismatches do not cause errors, instead something reasonable is
 * printed instead.  The output goes to the file with the specified id.
 *
 * given:
 *      id              file id to print to
 *      count           print count
 *      fmt             standard format string
 *      vals            table of values to print
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
    bool didneg, didprecision;
    bool printstring;
    bool printchar;

    fiop = findid(id, true);
    if (fiop == NULL) {
        return 1;
    }
    if (fiop->action == 'r') {
        fpos_t fpos;    /* current location */

        /*
         * reset to current location w/o a flush
         */
        fgetpos(fiop->fp, &fpos);
        if (fsetpos(fiop->fp, &fpos) < 0) {
            return 3;
        }
    }

    fiop->action = 'w';

    printstring = false;
    printchar = false;

    math_setfp(fiop->fp);

    while ((ch = *fmt++) != '\0') {
        if (ch != '%') {
            math_chr(ch);
            continue;
        }

        /*
         * Here to handle formats.
         */
        didneg = false;
        didprecision = false;
        width = 0;
        precision = 0;

        ch = *fmt++;
        if (ch == '-') {
            didneg = true;
            ch = *fmt++;
        }
        while ((ch >= '0') && (ch <= '9')) {
            width = width * 10 + (ch - '0');
            ch = *fmt++;
        }
        if (ch == '.') {
            didprecision = true;
            ch = *fmt++;
            while ((ch >= '0') && (ch <= '9')) {
                precision = precision * 10 + (ch - '0');
                ch = *fmt++;
            }
        }
        if (ch == 'l') {
            ch = *fmt++;
        }

        oldmode = conf->outmode;
        newmode = oldmode;
        olddigits = conf->outdigits;
        newdigits = olddigits;
        if (didprecision) {
            newdigits = precision;
        }

        switch (ch) {
        case 's':
            printstring = true;
            /*FALLTHRU*/
        case 'c':
            printchar = true;
        case 'd':
            break;
        case 'f':
            newmode = MODE_REAL;
            break;
        case 'e':
            newmode = MODE_EXP;
            break;
        case 'n':
            newmode = MODE_ENG;
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
            while (width-- > 0) {
                math_chr(' ');
            }
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
        if ((width == 0) || (vp->v_type == V_MAT) || (vp->v_type == V_LIST)) {
            switch (vp->v_type) {
            case V_OCTET:
                if (printstring) {
                    math_str((char *)vp->v_octet);
                } else if (printchar) {
                    math_chr(*vp->v_octet);
                } else {
                    printvalue(vp, PRINT_NORMAL);
                }
                break;
            case V_BLOCK:
                if (printstring) {
                    math_str((char *)vp->v_block->data);
                } else if (printchar) {
                    math_chr(*vp->v_block->data);
                } else {
                    printvalue(vp, PRINT_NORMAL);
                }
                break;
            case V_NBLOCK:
                if (printstring) {
                    if (vp->v_nblock->blk->data != NULL) {
                        math_str((char *)vp->v_nblock->blk->data);
                    }
                } else if (printchar) {
                    if (vp->v_nblock->blk->data != NULL) {
                        math_chr(*vp->v_nblock->blk->data);
                    }
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
        switch (vp->v_type) {
        case V_OCTET:
            if (printstring) {
                math_str((char *)vp->v_octet);
            } else if (printchar) {
                math_chr(*vp->v_octet);
            } else {
                printvalue(vp, PRINT_NORMAL);
            }
            break;
        case V_BLOCK:
            if (printstring) {
                math_str((char *)vp->v_block->data);
            } else if (printchar) {
                math_chr(*vp->v_block->data);
            } else {
                printvalue(vp, PRINT_NORMAL);
            }
            break;
        case V_NBLOCK:
            if (printstring) {
                if (vp->v_nblock->blk->data != NULL) {
                    math_str((char *)vp->v_nblock->blk->data);
                }
            } else if (printchar) {
                if (vp->v_nblock->blk->data != NULL) {
                    math_chr(*vp->v_nblock->blk->data);
                }
            } else {
                printvalue(vp, PRINT_NORMAL);
            }
            break;
        default:
            printvalue(vp, PRINT_NORMAL);
        }
        str = math_getdivertedio();
        if (strchr(str, '\n')) {
            width = 0;
        }
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
 *      id              file id to print to
 *      ch              character to write
 */
int
idfputc(FILEID id, int ch)
{
    FILEIO *fiop;

    /* get the file info pointer */
    fiop = findid(id, true);
    if (fiop == NULL) {
        return 1;
    }
    if (fiop->action == 'r') {
        fpos_t fpos;    /* current location */

        /*
         * reset to current location w/o a flush
         */
        fgetpos(fiop->fp, &fpos);
        if (fsetpos(fiop->fp, &fpos) < 0) {
            return 2;
        }
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
 *      id              file id to print to
 *      ch              character to write
 */
int
idungetc(FILEID id, int ch)
{
    FILEIO *fiop;

    fiop = findid(id, false);
    if (fiop == NULL) {
        return -2;
    }
    if (fiop->action != 'r') {
        return -2;
    }
    return ungetc(ch, fiop->fp);
}

/*
 * Write a string to a file.
 *
 * given:
 *      id              file id to print to
 *      str             string to write
 */
int
idfputs(FILEID id, STRING *str)
{
    FILEIO *fiop;
    FILE *fp;
    char *c;
    long len;

    /* get the file info pointer */
    fiop = findid(id, true);
    if (fiop == NULL) {
        return 1;
    }

    if (fiop->action == 'r') {
        fpos_t fpos;    /* current location */

        /*
         * reset to current location w/o a flush
         */
        fgetpos(fiop->fp, &fpos);
        if (fsetpos(fiop->fp, &fpos) < 0) {
            return 2;
        }
    }

    fiop->action = 'w';

    fp = fiop->fp;
    len = str->s_len;
    c = str->s_str;

    while (len-- > 0) {
        fputc(*c++, fp);
    }

    return 0;
}

/*
 * Same as idfputs but writes a terminating null character
 *
 * given:
 *      id                      file id to print to
 *      str                     string to write
 */
int
idfputstr(FILEID id, char *str)
{
    FILEIO *fiop;

    /* get the file info pointer */
    fiop = findid(id, true);
    if (fiop == NULL) {
        return 1;
    }

    if (fiop->action == 'r') {
        fpos_t fpos;    /* current location */

        /*
         * flush and attempt to restore the current location
         */
        fgetpos(fiop->fp, &fpos);
        if (fsetpos(fiop->fp, &fpos) < 0) {
            return 2;
        }
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
    if (fiop == NULL) {
        return 1;
    }
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
            (void)rewind(fiop->fp);
            fiop->action = 0;
        }
    }
}

/*
 * get_open_pos - get a an open file position
 *
 * given:
 *      fp              open file stream
 *      res             where to place the file position (ZVALUE)
 *
 * returns:
 *      0               res points to the file position
 *      -1              error
 */
static int
get_open_pos(FILE *fp, ZVALUE *res)
{
    off_t fpos; /* current file position */

    /*
     * get the file position
     */
    errno = 0;
    fpos = ftello(fp);
    if (errno != 0 || fpos < 0) {
        /* cannot get file position, return -1 */
        return -1;
    }

    /*
     * update file position and return success
     */
    *res = off_t2z(fpos);
    return 0;
}

/*
 * getloc - get the current position of the file
 *
 * given:
 *      id      file id of the file
 *      loc     pointer to result
 *
 * returns:
 *      0       able to get file position
 *      -1      unable to get file position
 */
int
getloc(FILEID id, ZVALUE *res)
{
    FILEIO *fiop; /* file structure */
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
        not_reached();
    }

    /*
     * return result
     */
    return get_open_pos(fp, res);
}

/*
 * ftello_stream - determine the file position as an ZVALUE of an FILEID
 *
 * given:
 *      id      calc FILEID
 *      fposp   pointer to off_t where to store file position
 *
 * returns:
 *      0 ==> all OK
 *      <0 ==> error
 */
static int
ftello_stream(FILE *fp, off_t *fposp)
{
    off_t fpos; /* current file position */

    /* get the file position */
    errno = 0;
    fpos = ftello(fp);
    if (errno != 0 || fpos < 0) {
        return -3;
    }
    *fposp = fpos;
    return 0;
}

/*
 * ftellid - determine the file position as an ZVALUE of an FILEID
 *
 * given:
 *      id      calc FILEID
 *      res     pointer to ZVALUE where to store file position
 *
 * returns:
 *      0 ==> all OK
 *      <0 ==> error
 */
int
ftellid(FILEID id, ZVALUE *res)
{
    FILEIO *fiop;
    off_t fpos; /* current file position */

    /* get FILEIO */
    fiop = findid(id, -1);
    if (fiop == NULL) {
        return -2;
    }

    /* get the file position */
    if (ftello_stream(fiop->fp, &fpos) < 0) {
        return -2;
    }

    /* convert file position to ZVALUE */
    *res = off_t2z(fpos);
    return 0;
}

/*
 * fseekid - seek on a FILEID
 *
 * given:
 *      id      calc FILEID
 *      offset  file offset relative to whence
 *      whence  0 ==> offset from the beginning of file
 *              1 ==> offset from the current file position
 *              2 ==> offset from the end of the file
 *
 * returns:
 *      0 ==> seek successful
 *      <= 0 ==> error
 *          -1 ==> fseeko() failed
 *          -2 ==> FILEID is invalid and/or was not found
 *          -3 ==> invalid whence value
 *          -4 ==> offset cannot be converted into an off_t
 *
 * NOTE: We let the system determine if the seek is valid.
 *       For example, we do not attempt to calculate the effective
 *       file position to be set is before the beginning of the file.
 *       Instead, we let the system's implementation of the fseeko()
 *       libc function, and it possible error return, determine what happens.
 */
int
fseekid(FILEID id, ZVALUE offset, int whence)
{
    FILEIO *fiop;    /* FILEIO of file */
    off_t off;       /* file offset as a off_t */
    int ret = 0;     /* return code */

    /*
     * verify that offset is in the range [OFF_T_MIN, OFF_T_MAX]
     */
    if (! in_range_off_t(offset)) {
        /* offset cannot be converted into an off_t */
        return -3;
    }
    off = z2off_t(offset);

    /* setup */
    fiop = findid(id, -1);
    if (fiop == NULL) {
        return -2;
    }

    /* seek depending on whence */
    switch (whence) {

    /*
     * seek relative to the beginning of the file
     */
    case 0:
        /* seek there */
        ret = fseeko(fiop->fp, off, SEEK_SET);
        break;

    /*
     * seek relative to the current file position
     */
    case 1:
        /* seek there */
        ret = fseeko(fiop->fp, off, SEEK_CUR);
        break;

    /*
     * seek relative to the end of the file
     */
    case 2:
        /* seek there */
        ret = fseeko(fiop->fp, off, SEEK_END);
        break;

    /*
     * unknown whence value
     */
    default:
        return -3;
    }
    return ret;
}

/*
 * free_range_off_t - free the [zoff_t_min, zoff_t_max] ZVALUE range set
 *
 * This function is called to free [zoff_t_min, zoff_t_max] ZVALUE storage, if set,
 * when calc exits.
 */
static void
free_range_off_t(void)
{
    if (range_set) {
        zfree(zoff_t_min);
        zfree(zoff_t_max);
        range_set = false;
    }
}

/*
 * in_range_off_t - verify that ZVALUE is within off_t range
 *
 * given:
 *      zpos        file position as a ZVALUE
 *
 * returns:
 *      true ==> ZVALUE is within [zoff_t_min, zoff_t_max] off_t range
 *      false ==> ZVALUE cannot be converted onto an off_t
 */
static bool
in_range_off_t(ZVALUE zpos)
{

    /*
     * initialize [zoff_t_min, zoff_t_max] ZVALUE range set if needed
     */
    if (! range_set) {
        /* setup [zoff_t_min, zoff_t_max] ZVALUE range set */
        zoff_t_min = off_t2z(OFF_T_MIN);
        zoff_t_max = off_t2z(OFF_T_MAX);
        range_set = true;
        atexit(free_range_off_t);
    }

    /*
     * verify that zpos is in the range [OFF_T_MIN, OFF_T_MAX]
     */
    if (zrel(zoff_t_min, zpos) > 0 && zrel(zoff_t_max, zpos) < 0) {
        /* cannot set file position, return -1 */
        return false;
    }
    return true;
}

/*
 * set_open_pos - set a an open file position
 *
 * given:
 *      fp              open file stream
 *      zpos            file position (ZVALUE) to set
 *
 * returns:
 *      0               res points to the file position
 *      -1              error
 *
 * NOTE: Due to fsetpos limitation, position is set relative to only
 *       the beginning of the file.
 */
static int
set_open_pos(FILE *fp, ZVALUE zpos)
{
    off_t fpos; /* file position to set as an off_t */

    /*
     * verify that zpos is in the range [OFF_T_MIN, OFF_T_MAX]
     */
    if (! in_range_off_t(zpos)) {
        /* cannot set file position, return -1 */
        return -1;
    }

    /*
     * convert ZVALUE to file position
     */
    fpos = z2off_t(zpos);

    /*
     * set the file position
     */
    if (fseeko(fp, fpos, SEEK_SET) < 0) {
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
 *      id      file id of the file
 *      zpos    file position (ZVALUE) to set
 *
 * returns:
 *      0       able to set file position
 *      -1      unable to set file position
 */
int
setloc(FILEID id, ZVALUE zpos)
{
    FILEIO *fiop; /* file structure */
    FILE *fp;

    /*
     * firewall
     */
    if ((id == FILEID_STDIN) || (id == FILEID_STDOUT) || (id == FILEID_STDERR)) {
        math_error("Cannot fseek stdin, stdout, or stderr");
        not_reached();
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
        not_reached();
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
 *      siz             file offset as off_t
 *      res             where to place the file offset (ZVALUE)
 *
 * returns:
 *      file offset as a ZVALUE
 */
static ZVALUE
off_t2z(off_t siz)
{
    ZVALUE ret; /* ZVALUE file size to return */

    /*
     * store off_t in a ZVALUE
     */
    ret.len = OFF_T_BITS / BASEB;
    ret.v = alloc(ret.len);
    zclearval(ret);
    if (siz >= 0) {
        SWAP_HALF_IN_OFF_T(ret.v, &siz);
        ret.sign = false;
    } else {
        off_t neg_siz = -siz;
        SWAP_HALF_IN_OFF_T(ret.v, &neg_siz);
        ret.sign = true;
    }
    ztrim(&ret);

    /*
     * return our result
     */
    return ret;
}

/*
 * z2off_t - convert a ZVALUE into an off_t
 *
 * given:
 *      zpos            file position as a ZVALUE
 *
 * returns:
 *      file position as a off_t
 */
static off_t
z2off_t(ZVALUE zpos)
{
#if OFF_T_BITS > FULL_BITS
    off_t tmp; /* temp file position as a off_t */
#endif
    off_t ret; /* file position as a off_t */
#if OFF_T_BITS < FULL_BITS
    long pos; /* zpos as a long */
#else
    FULL pos; /* zpos as a FULL */
#endif

    /*
     * quick return if the position can fit into a long
     */
#if OFF_T_BITS == FULL_BITS
    /* ztofull puts the value into native byte order */
    pos = ztofull(zpos);
    memset(&ret, 0, sizeof(ret));
    memcpy((void *)&ret, (void *)&pos, MIN(sizeof(ret), sizeof(pos)));
    if (zpos.sign) {
        ret = -ret;
    }
    return ret;
#elif OFF_T_BITS < FULL_BITS
    /* ztofull puts the value into native byte order */
    pos = ztolong(zpos);
    memset(&ret, 0, sizeof(ret));
    memcpy((void *)&ret, (void *)&pos, MIN(sizeof(ret), sizeof(pos)));
    if (zpos.sign) {
        ret = -ret;
    }
    return ret;
#else
    if (!zgtmaxfull(zpos)) {
        /* ztofull puts the value into native byte order */
        pos = ztofull(zpos);
        memset(&ret, 0, sizeof(ret));
        memcpy((void *)&ret, (void *)&pos, MIN(sizeof(ret), sizeof(pos)));
        if (zpos.sign) {
            ret = -ret;
        }
        return ret;
    }

    /*
     * copy (and swap if needed) lower part of the ZVALUE as needed
     */
    if (zpos.len >= OFF_T_BITS / BASEB) {
        /* copy the lower OFF_T_BITS of the ZVALUE */
        memset(&tmp, 0, sizeof(tmp));
        memcpy(&tmp, zpos.v, MIN(sizeof(tmp), OFF_T_LEN));
    } else {
        /* copy what bits we can into the temp value */
        memset(&tmp, 0, sizeof(tmp));
        memcpy(&tmp, zpos.v, MIN(sizeof(tmp), MIN(zpos.len * BASEB / 8, OFF_T_LEN)));
    }
    /* swap into native byte order */
    SWAP_HALF_IN_OFF_T(&ret, &tmp);
    if (zpos.sign) {
        ret = -ret;
    }

    /*
     * return our result
     */
    return ret;
#endif
}

/*
 * dev2z - convert a stat.st_dev into a ZVALUE
 *
 * given:
 *      dev             device
 *
 * returns:
 *      file size as a ZVALUE
 */
static ZVALUE
dev2z(dev_t dev)
{
    ZVALUE ret; /* ZVALUE file size to return */

    /*
     * store dev_t in a ZVALUE as a positive value
     */
    ret.len = DEV_T_BITS / BASEB;
    ret.v = alloc(ret.len);
    zclearval(ret);
    SWAP_HALF_IN_DEV_T(ret.v, &dev);
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
 *      inode           file size
 *
 * returns:
 *      file size as a ZVALUE
 */
/*ARGSUSED*/
static ZVALUE
inode2z(ino_t inode)
{
    ZVALUE ret; /* ZVALUE file size to return */

    /*
     * store ino_t in a ZVALUE as a positive value
     */
    ret.len = INO_T_BITS / BASEB;
    ret.v = alloc(ret.len);
    zclearval(ret);
    SWAP_HALF_IN_INO_T(ret.v, &inode);
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
 *      fp              open file stream
 *      res             where to place the file size (ZVALUE)
 *
 * returns:
 *      0               res points to the file size
 *      -1              error
 */
int
get_open_siz(FILE *fp, ZVALUE *res)
{
    struct stat buf; /* file status */

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
    *res= off_t2z(buf.st_size);
    return 0;
}

/*
 * getsize - get the current size of the file
 *
 * given:
 *      id      file id of the file
 *      res     pointer to result
 *
 * returns:
 *      0               able to get file size
 *      EOF             system error
 *      other nonzero   file not open or other problem
 */
int
getsize(FILEID id, ZVALUE *res)
{
    FILEIO *fiop; /* file structure */
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
 *      id      file id of the file
 *      dev     pointer to the result
 *
 * returns:
 *      0       able to get device
 *      -1      unable to get device
 */
int
get_device(FILEID id, ZVALUE *dev)
{
    FILEIO *fiop; /* file structure */

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
 *      id      file id of the file
 *      inode   pointer to the result
 *
 * returns:
 *      0       able to get inode
 *      -1      unable to get inode
 */
int
get_inode(FILEID id, ZVALUE *inode)
{
    FILEIO *fiop; /* file structure */

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

static off_t
filesize(FILEIO *fiop)
{
    struct stat sbuf;

    /* return length */
    if (fstat(fileno(fiop->fp), &sbuf) < 0) {
        math_error("bad fstat");
        not_reached();
    }
    return sbuf.st_size;
}

ZVALUE
zfilesize(FILEID id)
{
    FILEIO *fiop;
    off_t len;  /* file length */
    ZVALUE ret; /* file size as a ZVALUE return value */

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
    bool listed[MAXFILES];
    FILEIO *fiop;
    FILE *fp;
    struct stat sbuf;
    ino_t inodes[MAXFILES];
    off_t sizes[MAXFILES];
    int i, j;

    for (i = 0; i < idnum; i++) {
        listed[i] = false;
        fiop = &files[ioindex[i]];
        fp = fiop->fp;
        if (fstat(fileno(fp), &sbuf) < 0) {
            printf("Bad fstat for file %d\n", (int)fiop->id);
            sizes[i] = -1;
        } else {
            inodes[i] = sbuf.st_ino;
            sizes[i] = sbuf.st_size;
        }
    }
    for (i = 0; i < idnum; i++) {
        if (listed[i]) {
            continue;
        }
        fiop = &files[ioindex[i]];
        printf("\t");
        printid(fiop->id, PRINT_UNAMBIG);
        if (sizes[i] == -1) {
            math_chr('\n');
            continue;
        }
        printf(" size = %ld\n", (long int)sizes[i]);
        for (j = i + 1; j < idnum; j++) {
            if (listed[j] || sizes[j] == -1) {
                continue;
            }
            if (inodes[j] == inodes[i]) {
                listed[j] = true;
                fiop = &files[ioindex[j]];
                printf("\t  = ");
                printid(fiop->id, PRINT_UNAMBIG);
                printf("\n");
            }
        }
    }
    printf("\tNumber open = %d\n", idnum);
    printf("\tLastid = %d\n", (int)lastid);
}

/*
 * getscanfield - scan a field separated by some characters
 *
 * given:
 *      fp              FILEID to scan
 *      skip
 *      width           max field width
 *      scannum         Number of characters in scanset
 *      scanptr         string of characters considered separators
 *      strptr          pointer to where the new field pointer may be found
 */
static void
getscanfield(FILE *fp, bool skip, unsigned int width, int scannum, char *scanptr, char **strptr)
{
    char *str = NULL;         /* current string */
    unsigned long len;        /* current length of string */
    unsigned long totlen = 0; /* total length of string */
    char buf[READSIZE];       /* temporary buffer */
    int c;
    char *b;
    bool comp; /* Use complement of scanset */
    unsigned int chnum;

    comp = (scannum < 0);
    if (comp) {
        scannum = -scannum;
    }

    chnum = 0;

    for (;;) {
        len = 0;
        b = buf;
        for (;;) {
            c = fgetc(fp);
            if (c == EOF || c == '\0') {
                break;
            }
            chnum++;
            if (scannum && ((memchr(scanptr, c, scannum) == NULL) ^ comp)) {
                break;
            }
            if (!skip) {
                *b++ = c;
                len++;
                if (len >= READSIZE) {
                    break;
                }
            }
            if (chnum == width) {
                break;
            }
        }
        if (!skip) {
            if (totlen) {
                if (str == NULL) {
                    /* paranoia */
                    math_error("getscanfield: str was NULL while totlen != 0");
                    not_reached();
                } else {
                    str = (char *)realloc(str, totlen + len + 1);
                }
            } else {
                str = (char *)calloc(len + 1, 1);
            }
            if (str == NULL) {
                math_error("getscanfield: Out of memory for scanning");
                not_reached();
            }
            if (len) {
                memcpy(&str[totlen], buf, len);
            }
            totlen += len;
        }
        if (len < READSIZE) {
            break;
        }
    }

    if (!(width && chnum == width) && c != '\0') {
        ungetc(c, fp);
    }

    if (!skip) {
        str[totlen] = '\0';
        *strptr = str;
    }
}

/*
 * getscanwhite - scan a field separated by whitespace
 *
 * given:
 *      fp              FILEID to scan
 *      skip
 *      width           max field width
 *      scannum         Number of characters in scanset
 *      strptr          pointer to where the new field pointer may be found
 */
static void
getscanwhite(FILE *fp, bool skip, unsigned int width, int scannum, char **strptr)
{
    char *str;            /* current string */
    unsigned long len;    /* current length of string */
    unsigned long totlen; /* total length of string */
    char buf[READSIZE];   /* temporary buffer */
    int c;
    char *b;
    bool comp; /* Use complement of scanset */
    unsigned int chnum;

    totlen = 0;
    str = NULL;

    comp = (scannum < 0);
    if (comp) {
        scannum = -scannum;
    }

    chnum = 0;

    for (;;) {
        len = 0;
        b = buf;
        for (;;) {
            c = fgetc(fp);
            if (c == EOF || c == '\0') {
                break;
            }
            chnum++;
            if (scannum && (!isspace(c) ^ comp)) {
                break;
            }
            if (!skip) {
                *b++ = c;
                len++;
                if (len >= READSIZE) {
                    break;
                }
            }
            if (chnum == width) {
                break;
            }
        }
        if (!skip) {
            if (totlen) {
                str = (char *)realloc(str, totlen + len + 1);
            } else {
                str = (char *)calloc(len + 1, 1);
            }
            if (str == NULL) {
                math_error("Out of memory for scanning");
                not_reached();
            }
            if (len) {
                memcpy(&str[totlen], buf, len);
            }
            totlen += len;
        }
        if (len < READSIZE) {
            break;
        }
    }

    if (!(width && chnum == width) && c != '\0') {
        ungetc(c, fp);
    }

    if (!skip) {
        str[totlen] = '\0';
        *strptr = str;
    }
}

static int
fscanfile(FILE *fp, char *fmt, int count, VALUE **vals)
{
    int assnum;    /* Number of assignments made */
    int c;         /* Character read from file */
    char f;        /* Character read from format string */
    int scannum;   /* Number of characters in scanlist */
    char *scanptr; /* Start of scanlist */
    char *str = NULL;
    bool comp = false;      /* True scanset is complementary */
    bool skip = false;      /* True if string to be skipped rather than read */
    int width;
    VALUE *var;             /* lvalue to be assigned to */
    unsigned short subtype; /* for var->v_subtype */
    off_t cur;              /* current location */

    if (feof(fp)) {
        return EOF;
    }

    assnum = 0;

    for (;;) {
        for (;;) {
            f = *fmt++;
            if (isspace((int)f)) {
                getscanwhite(fp, 1, 0, 6, NULL);
                do {
                    f = *fmt++;
                } while (isspace((int)f));
            }
            c = fgetc(fp);
            if (c == EOF) {
                return assnum;
            }
            if (f == '%') {
                f = *fmt++;
                if (f != '%' && f != '\0') {
                    break;
                }
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
        if (skip) {
            f = *fmt++;
        }
        width = 0;
        while (f >= '0' && f <= '9') {
            width = 10 * width + f - '0';
            f = *fmt++;
        }
        switch (f) {
        case 'c':
            if (width == 0) {
                width = 1;
            }
            getscanfield(fp, skip, width, 0, NULL, &str);
            break;
        case 's':
            getscanwhite(fp, 1, 0, 6, NULL);
            if (feof(fp)) {
                return assnum;
            }
            getscanwhite(fp, skip, width, -6, &str);
            break;
        case '[':
            f = *fmt;
            comp = (f == '^');
            if (comp) {
                f = *++fmt;
            }
            scanptr = fmt;
            if (f == '\0') {
                return assnum;
            }
            fmt = strchr((f == ']' ? fmt + 1 : fmt), ']');
            if (fmt == NULL) {
                return assnum;
            }
            scannum = fmt - scanptr;
            if (comp) {
                scannum = -scannum;
            }
            fmt++;
            getscanfield(fp, skip, width, scannum, scanptr, &str);
            break;
        case 'f':
        case 'e':
        case 'r':
        case 'i':
            getscanwhite(fp, 1, 0, 6, NULL);
            if (feof(fp)) {
                return assnum;
            }
            if (skip) {
                fskipnum(fp);
                continue;
            }
            assnum++;
            var = *vals++;
            if (var->v_type != V_ADDR) {
                math_error("fscanfile: i case and var->v_type != V_ADDR");
                not_reached();
            }
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
            if (var->v_type != V_ADDR) {
                math_error("fscanfile: n case and var->v_type != V_ADDR");
                not_reached();
            }
            var = var->v_addr;
            subtype = var->v_subtype;
            freevalue(var);
            var->v_type = V_NUM;
            var->v_num = qalloc();
            errno = 0;
            cur = ftello(fp);
            if (errno != 0 || cur < 0) {
                math_error("fscanfile: failed to ftello file");
                not_reached();
            }
            var->v_num->num = off_t2z(cur);
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
            if (var->v_type != V_ADDR) {
                math_error("fscanfile: assigning to non-variable");
                not_reached();
            }
            var = var->v_addr;
            subtype = var->v_subtype;
            freevalue(var);
            var->v_type = V_STR;
            if (str == NULL) {
                /* paranoia */
                math_error("fscanfile: getscanfield not called and/or str is NULL");
                not_reached();
            }
            var->v_str = makestring(str);
        }
    }
}

int
fscanfid(FILEID id, char *fmt, int count, VALUE **vals)
{
    FILEIO *fiop;
    FILE *fp;
    fpos_t fpos;    /* current location */

    fiop = findid(id, false);
    if (fiop == NULL) {
        return -2;
    }

    fp = fiop->fp;

    if (fiop->action == 'w') {
        fgetpos(fp, &fpos);
        fflush(fp);
        if (fsetpos(fp, &fpos) < 0) {
            return -4;
        }
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
    if (fp == NULL) {
        return EOF;
    }
    fputs(str, fp);
    rewind(fp);
    i = fscanfile(fp, fmt, count, vals);
    fclose(fp);
    return i;
}

/*
 * Read a number in floating-point format from a file.  The first dot,
 * if any, is considered as the decimal point; later dots are ignored.
 * For example, -23.45..67. is interpreted as -23.4567
 * An optional 'e' or 'E' indicates multiplication by a power or 10,
 * e.g. -23.45e-6 has the effect of -23.45 * 10^-6.  The reading
 * ceases when a character other than a digit, a leading sign,
 * a sign immediately following 'e' or 'E', or a dot is encountered.
 * Absence of digits is interpreted as zero.
 */
static void
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
    bool sign, negexp, havedp, imag, exptoobig;

    decimals = 0;
    exp = 0;
    sign = false;
    negexp = false;
    havedp = false;
    imag = false;
    exptoobig = false;

    ch = fgetc(fp);
    if (ch == '+' || ch == '-') {
        if (ch == '-') {
            sign = true;
        }
        ch = fgetc(fp);
    }
    num.v = alloc(1);
    *num.v = 0;
    num.len = 1;
    num.sign = sign;
    for (;;) {
        if (ch >= '0' && ch <= '9') {
            f = (FULL)(ch - '0');
            a = num.v;
            i = num.len;
            while (i-- > 0) {
                f = 10 * (FULL)*a + f;
                *a++ = (HALF)f;
                f >>= BASEB;
            }
            if (f) {
                a = alloc(num.len + 1);
                memcpy(a, num.v, num.len * sizeof(HALF));
                a[num.len] = (HALF)f;
                num.len++;
                freeh(num.v);
                num.v = a;
            }
            if (havedp) {
                decimals++;
            }
        } else if (ch == '.') {
            havedp = true;
        } else {
            break;
        }
        ch = fgetc(fp);
    }
    if (ch == 'e' || ch == 'E') {
        ch = fgetc(fp);
        if (ch == '+' || ch == '-') {
            if (ch == '-') {
                negexp = true;
            }
            ch = fgetc(fp);
        }
        while (ch >= '0' && ch <= '9') {
            if (!exptoobig) {
                exp = (exp * 10) + ch - '0';
                if (exp > 1000000) {
                    exptoobig = true;
                }
            }
            ch = fgetc(fp);
        }
    }
    if (ch == 'i' || ch == 'I') {
        imag = true;
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

static void
freadsum(FILE *fp, VALUE *valptr)
{
    VALUE v1, v2, v3;
    char ch;

    freadprod(fp, &v1);

    ch = fgetc(fp);
    while (ch == '+' || ch == '-') {
        freadprod(fp, &v2);
        if (ch == '+') {
            addvalue(&v1, &v2, &v3);
        } else {
            subvalue(&v1, &v2, &v3);
        }
        freevalue(&v1);
        freevalue(&v2);
        v1 = v3;
        ch = fgetc(fp);
    }
    ungetc(ch, fp);
    *valptr = v1;
}

static void
freadprod(FILE *fp, VALUE *valptr)
{
    VALUE v1, v2, v3;
    char ch;

    freadnum(fp, &v1);
    ch = fgetc(fp);
    while (ch == '*' || ch == '/') {
        freadnum(fp, &v2);
        if (ch == '*') {
            mulvalue(&v1, &v2, &v3);
        } else {
            divvalue(&v1, &v2, &v3);
        }
        freevalue(&v1);
        freevalue(&v2);
        v1 = v3;
        ch = fgetc(fp);
    }
    ungetc(ch, fp);
    *valptr = v1;
}

static void
fskipnum(FILE *fp)
{
    char ch;

    do {
        ch = fgetc(fp);
        if (ch == '+' || ch == '-') {
            ch = fgetc(fp);
        }
        while ((ch >= '0' && ch <= '9') || ch == '.') {
            ch = fgetc(fp);
        }
        if (ch == 'e' || ch == 'E') {
            ch = fgetc(fp);
            if (ch == '+' || ch == '-') {
                ch = fgetc(fp);
            }
            while (ch >= '0' && ch <= '9') {
                ch = fgetc(fp);
            }
        }
        if (ch == 'i' || ch == 'I') {
            ch = fgetc(fp);
        }
    } while (ch == '/' || ch == '*' || ch == '+' || ch == '-');

    ungetc(ch, fp);
}

int
isattyid(FILEID id)
{
    FILEIO *fiop;

    fiop = findid(id, -1);
    if (fiop == NULL) {
        return -2;
    }
    return isatty(fileno(fiop->fp));
}

/*
 * fsearch - search for a string in a file
 *
 * given:
 *      id      FILEID to search
 *      str     string to look for
 *      start   starting file position to begin search (0 ==> beginning of file)
 *      end     ending file position to end search (relative to beginning of file)
 *      res     point to where to record where the string was found
 *
 * returns:
 *      >0 ==> string not found
 *      0 ==> string found
 *      EOF ==> system error
 *      <0 ==> other error (if file not open, etc.)
 *          -1 ==> end of file (EOF) or file read error
 *          -2 ==> FILEID is invalid and/or was not found
 *          -3 ==> fgetpos() failed
 *          -4 ==> fsetpos() failed
 *          -5 ==> ftello_stream() failed
 *          -6 ==> start cannot be converted into an off_t
 *          -7 ==> end cannot be converted into an off_t
 *
 * NOTE: If end < start, 1 (string not found) is returned.
 *
 * XXX - This search is a translation of the original search that did not
 *       work with large files.  The search algorithm used is slow and
 *       should be speed up much more.
 */
int
fsearch(FILEID id, char *str, ZVALUE start, ZVALUE end, ZVALUE *res)
{
    FILEIO *fiop;     /* FILEIO of file id */
    off_t fstart;     /* start as a file offset in the form of an off_t */
    off_t fend;       /* end as a file offset in the form of an off_t */
    fpos_t cur;       /* current file position in the form of an fpos_t */
    off_t str_len;         /* length of string in of an off_t */
    off_t found_pos;       /* file position where the string was found */
    off_t file_zone_len;   /* file search zone length taking length of string into account */
    off_t zone_remain;     /* remaining file search zone length */

    char c;           /* str comparison character */
    int r;            /* character read from file */
    char *s;          /* str comparison pointer */
    int ret = 0;      /* return code */

    /*
     * verify that start is in the range [OFF_T_MIN, OFF_T_MAX]
     */
    if (! in_range_off_t(start)) {
        /* start cannot be converted into an off_t */
        return -6;
    }
    fstart = z2off_t(start);

    /*
     * verify that end is in the range [OFF_T_MIN, OFF_T_MAX]
     */
    if (! in_range_off_t(end)) {
        /* end cannot be converted into an off_t */
        return -7;
    }
    fend = z2off_t(end);

    /* get FILEIO */
    fiop = findid(id, false);
    if (fiop == NULL) {
        return -2;
    }

    /*
     * file setup
     */
    if (fiop->action == 'w') {
        fflush(fiop->fp);
    }

    /*
     * firewall - do nothing if end < start
     */
    if (fend < fstart) {
        return 1;
    }

    /*
     * seek to the start position
     */
    ret = fseeko(fiop->fp, fstart, SEEK_SET);
    if (ret < 0) {
        return EOF;
    }

    /*
     * search setup
     */
    c = *str;       /* note the first str search match character */
    if (c == '\0') {

        /*
         * report empty string found :-)
         */
        return 2;
    }
    ++str;  /* advance to next position in the string */
    str_len = strlen(str);
    file_zone_len = fend - fstart;
    zone_remain = file_zone_len;

    /*
     * search for string in file starting with the current position
     */
    clearerr(fiop->fp);
    while ((r = fgetc(fiop->fp)) != EOF) {

        /*
         * look for the opening string match character
         */
        if ((char)r == c) {

            /*
             * matched first string character, record current file position
             *
             * We will return to this position if the string match fails.
             */
            if (fgetpos(fiop->fp, &cur) < 0) {
                return -3;
            }

            /*
             * search the rest of the string in the file
             */
            for (s = str; *s; ++s) {
                r = fgetc(fiop->fp);
                if ((char)r != *s) {
                    /* failed to match this string character */
                    break;
                }
            }
            if (r == EOF) {
                /*
                 * We encountered end of file or an read error, not finding the string,
                 * before the end of the search area was reached.
                 */
                break;
            }
            if (*s == '\0') {

                /*
                 * string found - note the new file position beyond the found string
                 */
                if (ftello_stream(fiop->fp, &found_pos) < 0) {
                    return -5;
                }

                /*
                 * set res to start of the string
                 */
                --found_pos;
                *res = off_t2z(found_pos - str_len);

                /*
                 * report string found
                 */
                return 0;
            }

            /*
             * string not found, restore file to the previously recoded "current file position"
             */
            if (fsetpos(fiop->fp, &cur) < 0) {
                return -4;
            }
        }

        /*
         * did not find the string, so we will advance in the file search zone area and reduce zone_remain
         */
        if (zone_remain <= 0) {

            /*
             * end of search - we have exhausted the file search zone
             *
             * Move beyond the end of file search zone
             */
            ret = fseeko(fiop->fp, fend+1, SEEK_SET);
            if (ret < 0) {
                return EOF;
            }
            if (ftello_stream(fiop->fp, &found_pos) < 0) {
                return -5;
            }
            break;
        }
        --zone_remain;

        /*
         * keep searching until:
         *
         *      string is found
         *      end of file
         *      read error
         *      end of search zone is reached
         */
    }

    /*
     * if needed, report on file read errors
     */
    if (ferror(fiop->fp)) {
        return EOF;
    }

    /*
     * report string not found
     */
    return 1;
}

/*
 * frsearch - reverse search for a string in a file
 *
 * given:
 *      id      FILEID to search
 *      str     string to look for
 *      first   initial file position to search from (decreasing while pos >= last)
 *      last    final file position to search from
 *      res     point to where to record where the string was found
 *
 * returns:
 *      >0 ==> string not found
 *      0 ==> string found
 *      EOF ==> system error
 *      <0 ==> other error (if file not open, etc.)
 *          -1 ==> end of file (EOF) or file read error
 *          -2 ==> FILEID is invalid and/or was not found
 *          -3 ==> fgetpos() failed
 *          -4 ==> fsetpos() failed
 *          -5 ==> ftello_stream() failed
 *          -6 ==> first cannot be converted into an off_t
 *          -7 ==> last cannot be converted into an off_t
 *
 * XXX - This search is a translation of the original search that did not
 *       work with large files.  The search algorithm used is so slow
 *       as to be painful to the user and needs to be sped up much more.
 */
int
frsearch(FILEID id, char *str, ZVALUE first, ZVALUE last, ZVALUE *res)
{
    FILEIO *fiop;     /* FILEIO of file id */
    off_t ffirst;     /* first as a file offset in the form of a off_t */
    off_t flast;      /* last as a file offset in the form of a off_t */
    off_t cur;        /* current file position */

    char c;           /* str comparison character */
    int r;            /* character read from file */
    char *s;          /* str comparison pointer */
    int ret = 0;      /* return code */

    /*
     * verify that first is in the range [OFF_T_MIN, OFF_T_MAX]
     */
    if (! in_range_off_t(first)) {
        /* first cannot be converted into an off_t */
        return -6;
    }
    ffirst = z2off_t(first);

    /*
     * verify that last is in the range [OFF_T_MIN, OFF_T_MAX]
     */
    if (! in_range_off_t(last)) {
        /* last cannot be converted into an off_t */
        return -7;
    }
    flast = z2off_t(last);

    /* get FILEIO */
    fiop = findid(id, false);
    if (fiop == NULL) {
        return -2;
    }

    /*
     * file setup
     */
    if (fiop->action == 'w') {
        fflush(fiop->fp);
    }

    /*
     * search setup
     */
    c = *str;       /* note the first str search match character */
    if (c == '\0') {

        /*
         * seek to the start position
         */
        ret = fseeko(fiop->fp, ffirst, SEEK_SET);
        if (ret < 0) {
            return EOF;
        }

        /*
         * empty string always match the beginning of the search zone
         */
        *res = off_t2z(ffirst);

        /*
         * report empty string found :-)
         */
        return 0;
    }
    ++str;  /* advance to next position in the string */

    /*
     * search file backward for string
     */
    clearerr(fiop->fp);
    for (cur = ffirst; cur >= flast; --cur) {

        /*
         * set the file position to cur
         */
        if (fseeko(fiop->fp, cur, SEEK_SET) < 0) {
            /* cannot set file position, return EOF */
            return EOF;
        }

        /*
         * read current file character
         */
        r = fgetc(fiop->fp);
        if (r == EOF) {
            /* end of file or file read error */
            return EOF;
        }

        /*
         * look for the opening string match character
         */
        if ((char)r == c) {

            /*
             * matched first string character - search the rest of the string in the file
             */
            s = str;
            while (*s) {
                r = fgetc(fiop->fp);
                if ((char)r != *s) {
                    break;
                }
                s++;
            }
            if (r == EOF) {
                /*
                 * We encountered end of file or an read error, not finding the string
                 */
                return EOF;
            }
            if (*s == '\0') {

                /*
                 * set res to the current file position
                 */
                *res = off_t2z(cur);

                /*
                 * push back in the stream the last character we processed
                 */
                ungetc(r, fiop->fp);

                /*
                 * report string found
                 */
                return 0;
            }
        }

        /*
         * keep searching backwards until:
         *
         *      string is found
         *      end of file
         *      read error
         *      last of search the area is reached
         */
    }

    /*
     * set the file position to last
     */
    if (fseeko(fiop->fp, flast, SEEK_SET) < 0) {
        /* cannot set file position, return EOF */
        return EOF;
    }

    /*
     * if needed, report on file read errors
     */
    if (ferror(fiop->fp)) {
        return EOF;
    }

    /*
     * report string not found
     */
    return 1;
}

char *
findfname(FILEID id)
{
    FILEIO *fiop;

    fiop = findid(id, -1);

    if (fiop == NULL) {
        return NULL;
    }

    return fiop->name;
}
