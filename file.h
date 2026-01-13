/*
 * file - file I/O routines callable by users
 *
 * Copyright (C) 1999-2007,2014,2021,2023,2026  David I. Bell and Landon Curt Noll
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
 * Under source code control:   1996/05/24 05:55:58
 * File existed as early as:    1996
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_FILE_H)
#  define INCLUDE_FILE_H

/*
 * Definition of opened files.
 */
#  define MODE_LEN (sizeof("rb+") - 1)
typedef struct {
    FILEID id;               /* id to identify this file */
    FILE *fp;                /* real file structure for I/O */
    dev_t dev;               /* file device */
    ino_t inode;             /* file inode */
    char *name;              /* file name */
    bool reading;            /* true if opened for reading */
    bool writing;            /* true if opened for writing */
    bool appending;          /* true if also opened for appending */
    bool binary;             /* true if binary mode - mode ignored/unused */
    char action;             /* most recent use for 'r', 'w' or 0 */
    char mode[MODE_LEN + 1]; /* open mode */
} FILEIO;

/*
 * external functions
 */
extern FILEIO *findid(FILEID id, int writable);
extern int fgetposid(FILEID id, fpos_t *ptr);
extern int fsetposid(FILEID id, fpos_t *ptr);
extern int get_open_siz(FILE *fp, ZVALUE *res);
extern char *findfname(FILEID);
extern FILE *f_pathopen(char *name, char *mode, char *pathlist, char **openpath);

#endif
