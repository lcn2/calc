/*
 * Copyright (c) 1996 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * File I/O routines callable by users.
 */

#include "have_fpos.h"


/*
 * Definition of opened files.
 */
typedef struct {
	FILEID id;		/* id to identify this file */
	FILE *fp;		/* real file structure for I/O */
	dev_t dev;		/* file device */
	ino_t inode;		/* file inode */
	char *name;		/* file name */
	BOOL reading;		/* TRUE if opened for reading */
	BOOL writing;		/* TRUE if opened for writing */
	char action;		/* most recent use for 'r', 'w' or 0 */
	char mode[3];		/* open mode */
} FILEIO;


/*
 * fgetpos/fsetpos vs fseek/ftell interface
 *
 * f_seek_set(FILE *stream, FILEPOS *loc)
 *	Seek loc bytes from the beginning of the open file, stream.
 *
 * f_tell(FILE *stream, FILEPOS *loc)
 *	Set loc to bytes from the beinning of the open file, stream.
 *
 * We assume that if your system does not have fgetpos/fsetpos,
 * then it will have a FILEPOS that is a scalar type (e.g., long).
 * Some obscure systems without fgetpos/fsetpos may not have a simple
 * scalar type.  In these cases the f_tell macro below will fail.
 */
#if defined(HAVE_FPOS)

#define f_seek_set(stream, loc) fsetpos((FILE*)(stream), (FILEPOS*)(loc))
#define f_tell(stream, loc) fgetpos((FILE*)(stream), (FILEPOS*)(loc))

#else

#define f_seek_set(stream, loc)  \
	fseek((FILE*)(stream), *(FILEPOS*)(loc), SEEK_SET)
#define f_tell(stream, loc) (*((FILEPOS*)(loc)) = ftell((FILE*)(stream)))

#endif


/*
 * external functions
 */
extern int fgetposid(FILEID id, FILEPOS *ptr);
extern int fsetposid(FILEID id, FILEPOS *ptr);
