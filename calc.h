/*
 * calc - definitions for calculator program
 *
 * Copyright (C) 1999  David I. Bell
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
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * @(#) $Revision: 29.11 $
 * @(#) $Id: calc.h,v 29.11 2003/08/26 04:36:10 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/calc/RCS/calc.h,v $
 *
 * Under source code control:	1990/02/15 01:48:31
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(__CALC_H__)
#define __CALC_H__

#include <setjmp.h>
#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "win32dll.h"
# include "value.h"
# include "have_const.h"
#else
# include <calc/win32dll.h>
# include <calc/value.h>
# include <calc/have_const.h>
#endif


/*
 * Configuration definitions
 */
#define CALCPATH	"CALCPATH"	/* environment variable for files */
#define CALCRC		"CALCRC"	/* environment variable for startup */
#define CALCBINDINGS	"CALCBINDINGS"	/* env variable for hist bindings */
#define HOME		"HOME"		/* environment variable for home dir */
#define PAGER		"PAGER"		/* environment variable for help */
#define SHELL		"SHELL"		/* environment variable for shell */
#define DEFAULTCALCBINDINGS "bindings"	/* default calc bindings file */
#define DEFAULTCALCHELP "help"		/* help file that -h prints */
#define DEFAULTSHELL	"sh"		/* default shell to use */
#define CALCEXT		".cal"	/* extension for files read in */
#define MAX_CALCRC	1024	/* maximum length of $CALCRC */
#define HOMECHAR	'~'	/* char which indicates home directory */
#define DOTCHAR		'.'	/* char which indicates current directory */
#define PATHCHAR	'/'	/* char which separates path components */
#if defined(__MSDOS__) || defined(__WIN32)
#define LISTCHAR	';'	/* char which separates paths in a list */
#else
#define LISTCHAR	':'	/* char which separates paths in a list */
#endif
#define MAXCMD		16384	/* maximum length of command invocation */
#define MAXERROR	512	/* maximum length of error message string */

#define SYMBOLSIZE	256	/* maximum symbol name size */
#define MAXLABELS	100	/* maximum number of user labels in function */
#define MAXSTRING	1024	/* maximum size of string constant */
#define MAXSTACK	2048	/* maximum depth of evaluation stack */
#define MAXFILES	20	/* maximum number of opened files */
#define PROMPT1		"> "	/* default normal prompt*/
#define PROMPT2		">> "	/* default prompt inside multi-line input */


#define TRACE_NORMAL	0x00	/* normal trace flags */
#define TRACE_OPCODES	0x01	/* trace every opcode */
#define TRACE_NODEBUG	0x02	/* suppress debugging opcodes */
#define TRACE_LINKS	0x04	/* display links for real and complex numbers */
#define TRACE_FNCODES	0x08	/* display code for newly defined function */
#define TRACE_MAX	0x0f	/* maximum value for trace flag */

#define ABORT_NONE	0	/* abort not needed yet */
#define ABORT_STATEMENT 1	/* abort on statement boundary */
#define ABORT_OPCODE	2	/* abort on any opcode boundary */
#define ABORT_MATH	3	/* abort on any math operation */
#define ABORT_NOW	4	/* abort right away */

#define ERRMAX 20		/* default errmax value */

/*
 * File ids corresponding to standard in, out, error, and when not in use.
 */
#define FILEID_STDIN	((FILEID) 0)
#define FILEID_STDOUT	((FILEID) 1)
#define FILEID_STDERR	((FILEID) 2)
#define FILEID_NONE	((FILEID) -1)

/*
 * File I/O routines.
 */
extern DLL FILEID openid(char *name, char *mode);
extern DLL FILEID indexid(long index);
extern DLL BOOL validid(FILEID id);
extern DLL BOOL errorid(FILEID id);
extern DLL BOOL eofid(FILEID id);
extern DLL int closeid(FILEID id);
extern DLL int getcharid(FILEID id);
extern DLL int idprintf(FILEID id, char *fmt, int count, VALUE **vals);
extern DLL int idfputc(FILEID id, int ch);
extern DLL int idfputs(FILEID id, char *str);
extern DLL int printid(FILEID id, int flags);
extern DLL int flushid(FILEID id);
extern DLL int readid(FILEID id, int flags, char **retptr);
extern DLL int getloc(FILEID id, ZVALUE *loc);
extern DLL int setloc(FILEID id, ZVALUE zpos);
extern DLL int getsize(FILEID id, ZVALUE *size);
extern DLL int get_device(FILEID id, ZVALUE *dev);
extern DLL int get_inode(FILEID id, ZVALUE *ino);
extern DLL FILEID reopenid(FILEID id, char *mode, char *name);
extern DLL int closeall(void);

#if !defined(_WIN32)
extern DLL int flushall(void);
#endif

extern DLL int idfputstr(FILEID id, char *str);
extern DLL int rewindid(FILEID id);
extern DLL void rewindall(void);
extern DLL ZVALUE zfilesize(FILEID id);
extern DLL void showfiles(void);
extern DLL int fscanfid(FILEID id, char *fmt, int count, VALUE **vals);
extern DLL int scanfstr(char *str, char *fmt, int count, VALUE **vals);
extern DLL int ftellid(FILEID id, ZVALUE *res);
extern DLL int fseekid(FILEID id, ZVALUE offset, int whence);
extern DLL int isattyid(FILEID id);
extern DLL int fsearch(FILEID id, char *str, ZVALUE start, ZVALUE end, ZVALUE *res);
extern DLL int frsearch(FILEID id, char *str, ZVALUE first, ZVALUE last, ZVALUE *res);
extern DLL void showconstants(void);
extern DLL void freeconstant(unsigned long);
extern DLL void freestringconstant(long);
extern DLL void trimconstants(void);

/*
 * Input routines.
 */
extern DLL int openstring(char *str, long num);
extern DLL int openterminal(void);
extern DLL int opensearchfile(char *name, char *pathlist, char *exten, int reopen_ok);
extern DLL char *nextline(void);
extern DLL int nextchar(void);
extern DLL void reread(void);
extern DLL void resetinput(void);
extern DLL void setprompt(char *);
extern DLL BOOL inputisterminal(void);
extern DLL int inputlevel(void);
extern DLL long calclevel(void);
extern DLL char *inputname(void);
extern DLL long linenumber(void);
extern DLL void runrcfiles(void);
extern DLL void closeinput(void);

/*
 * Other routines.
 */
extern DLL NUMBER *constvalue(unsigned long index);
extern DLL long addnumber(char *str);
extern DLL long addqconstant(NUMBER *q);
extern DLL void initstack(void);
extern DLL void getcommands(BOOL toplevel);
extern DLL void givehelp(char *type);
extern DLL void libcalc_call_me_first(void);
extern DLL void libcalc_call_me_last(void);
extern DLL BOOL calc_tty(int fd);
extern DLL BOOL orig_tty(int fd);
extern DLL void showerrors(void);
extern DLL char *calc_strdup(CONST char *);

/*
 * Initialization
 */
extern DLL void initialize(void);
extern DLL void reinitialize(void);
#if !defined (_WIN32)
extern DLL int isatty(int tty);	/* TRUE if fd is a tty */
#endif
extern DLL char *version(void);	/* return version string */
extern DLL int post_init;	/* TRUE => math_error setjmp is ready */

/*
 * global flags and definitions
 */
extern DLL int abortlevel;	/* current level of aborts */
extern DLL BOOL inputwait;	/* TRUE if in a terminal input wait */
extern DLL jmp_buf jmpbuf;	/* for errors */

extern DLL int p_flag;		/* TRUE => pipe mode */
extern DLL int q_flag;		/* TRUE => don't execute rc files */
extern DLL int u_flag;		/* TRUE => unbuffer stdin and stdout */
extern DLL int d_flag;		/* TRUE => disable heading, resource_debug */
extern DLL int c_flag;		/* TRUE => continue after error if permitted */
extern DLL int i_flag;		/* TRUE => try to go interactive after error */
extern DLL int s_flag;		/* TRUE => keep args as strings for argv() */
extern DLL int stoponerror;	/* >0 => stop, <0 => continue, ==0 => use -c */
extern DLL BOOL abort_now;	/* TRUE => try to go interactive */

extern DLL int argc_value;	/* count of argv[] strings for argv() builtin */
extern DLL char **argv_value;	/* argv[] strings for argv() builtin */

extern DLL char *pager;		/* $PAGER or default */
extern DLL int stdin_tty;	/* TRUE if stdin is a tty */
extern DLL int havecommands;	/* TRUE if have cmd args) */
extern DLL char *program;	/* our name */
extern DLL char *base_name;	/* basename of our name */
extern DLL char cmdbuf[];	/* command line expression */

extern DLL int abortlevel;	/* current level of aborts */
extern DLL BOOL inputwait;	/* TRUE if in a terminal input wait */
extern DLL VALUE *stack;	/* execution stack */
extern DLL int dumpnames;	/* TRUE => dump names rather than indices */

extern DLL char *calcpath;	/* $CALCPATH or default */
extern DLL char *calcrc;	/* $CALCRC or default */
extern DLL char *calcbindings;	/* $CALCBINDINGS or default */
extern DLL char *home;		/* $HOME or default */
extern DLL char *shell;		/* $SHELL or default */
extern DLL char *program;	/* our name (argv[0]) */

extern DLL int no_env;		/* TRUE (-e) => ignore env vars on startup */
extern DLL int errmax;	/* if >= 0, error when errcount exceeds errmax */
extern DLL int use_old_std;	/* TRUE (-O) => use classic configuration */

extern DLL int allow_read;	/* FALSE => dont open any files for reading */
extern DLL int allow_write; 	/* FALSE => dont open any files for writing */
extern DLL int allow_exec;	/* FALSE => may not execute any commands */

/*
 * calc startup and run state
 */
typedef enum {
    RUN_ZERO,			/* unknown or unset start state */
    RUN_BEGIN,			/* calc execution starts */
    RUN_RCFILES,		/* rc files being evaluated */
    RUN_PRE_CMD_ARGS,		/* prepare to evaluate cmd args */
    RUN_CMD_ARGS,		/* cmd args being evaluated */
    RUN_PRE_TOP_LEVEL,		/* prepare to start top level activity */
    RUN_TOP_LEVEL,		/* running at top level */
    RUN_EXIT,			/* normal exit from calc */
    RUN_EXIT_WITH_ERROR		/* exit with error */
} run;
extern DLL run run_state;
extern DLL char *run_state_name(run state);

/*
 * calc version information
 */
#define CALC_TITLE "C-style arbitrary precision calculator"
extern int calc_major_ver;
extern int calc_minor_ver;
extern int calc_major_patch;
extern int calc_minor_patch;
extern char *Copyright;
extern DLL char *version(void);


#endif /* !__CALC_H__ */
