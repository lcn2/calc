/*
 * calc - definitions for calculator program
 *
 * Copyright (C) 1999-2007,2014,2021  David I. Bell
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
 * Under source code control:	1990/02/15 01:48:31
 * File existed as early as:	before 1990
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#if !defined(INCLUDE_CALC_H)
#define INCLUDE_CALC_H

#include <setjmp.h>
#if defined(CALC_SRC)	/* if we are building from the calc source tree */
# include "decl.h"
# include "value.h"
# include "have_const.h"
#else
# include <calc/decl.h>
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
#define CALCHISTFILE	"CALCHISTFILE"	/* history file environment variable */
#define CALCHELP	"CALCHELP"	/* help directory env variable */
#define CALCCUSTOMHELP	"CALCCUSTOMHELP"/* custom help directory env variable */
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

#define SYMBOLSIZE	256	/* maximum symbol name size */
#define MAXLABELS	100	/* maximum number of user labels in function */
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
#define E_OK 0			/* no error */

/*
 * File ids corresponding to standard in, out, error, and when not in use.
 */
#define FILEID_STDIN	((FILEID) 0)
#define FILEID_STDOUT	((FILEID) 1)
#define FILEID_STDERR	((FILEID) 2)
#define FILEID_NONE	((FILEID) -1)	/* must be < 0 */

/*
 * File I/O routines.
 */
E_FUNC FILEID openid(char *name, char *mode);
E_FUNC FILEID openpathid(char *name, char *mode, char *pathlist);
E_FUNC FILEID indexid(long index);
E_FUNC BOOL validid(FILEID id);
E_FUNC BOOL errorid(FILEID id);
E_FUNC BOOL eofid(FILEID id);
E_FUNC int closeid(FILEID id);
E_FUNC int getcharid(FILEID id);
E_FUNC int idprintf(FILEID id, char *fmt, int count, VALUE **vals);
E_FUNC int idfputc(FILEID id, int ch);
E_FUNC int idfputs(FILEID id, STRING *str);
E_FUNC int printid(FILEID id, int flags);
E_FUNC int flushid(FILEID id);
E_FUNC int readid(FILEID id, int flags, STRING **retptr);
E_FUNC int getloc(FILEID id, ZVALUE *loc);
E_FUNC int setloc(FILEID id, ZVALUE zpos);
E_FUNC int getsize(FILEID id, ZVALUE *size);
E_FUNC int get_device(FILEID id, ZVALUE *dev);
E_FUNC int get_inode(FILEID id, ZVALUE *ino);
E_FUNC FILEID reopenid(FILEID id, char *mode, char *name);
E_FUNC int closeall(void);

#if !defined(_WIN32)
E_FUNC int flushall(void);
#endif

E_FUNC int idfputstr(FILEID id, char *str);
E_FUNC int rewindid(FILEID id);
E_FUNC void rewindall(void);
E_FUNC ZVALUE zfilesize(FILEID id);
E_FUNC void showfiles(void);
E_FUNC int fscanfid(FILEID id, char *fmt, int count, VALUE **vals);
E_FUNC int scanfstr(char *str, char *fmt, int count, VALUE **vals);
E_FUNC int ftellid(FILEID id, ZVALUE *res);
E_FUNC int fseekid(FILEID id, ZVALUE offset, int whence);
E_FUNC int isattyid(FILEID id);
E_FUNC int fsearch(FILEID id, char *str, ZVALUE start, ZVALUE end, ZVALUE *res);
E_FUNC int frsearch(FILEID id, char *str, ZVALUE first, ZVALUE last,
		    ZVALUE *res);
E_FUNC void showconstants(void);
E_FUNC void freeconstant(unsigned long);
E_FUNC void freestringconstant(long);
E_FUNC void trimconstants(void);

/*
 * Input routines.
 */
E_FUNC int openstring(char *str, size_t num);
E_FUNC int openterminal(void);
E_FUNC int opensearchfile(char *name, char *pathlist, char *exten,
			  int reopen_ok);
E_FUNC char *nextline(void);
E_FUNC int nextchar(void);
E_FUNC void reread(void);
E_FUNC void resetinput(void);
E_FUNC void setprompt(char *);
E_FUNC BOOL inputisterminal(void);
E_FUNC int inputlevel(void);
E_FUNC long calclevel(void);
E_FUNC char *inputname(void);
E_FUNC long linenumber(void);
E_FUNC void runrcfiles(void);
E_FUNC void closeinput(void);

/*
 * Other routines.
 */
E_FUNC NUMBER *constvalue(unsigned long index);
E_FUNC long addnumber(char *str);
E_FUNC long addqconstant(NUMBER *q);
E_FUNC void initstack(void);
E_FUNC void getcommands(BOOL toplevel);
E_FUNC void givehelp(char *type);
E_FUNC void libcalc_call_me_first(void);
E_FUNC void libcalc_call_me_last(void);
E_FUNC BOOL calc_tty(int fd);
E_FUNC BOOL orig_tty(int fd);
E_FUNC void showerrors(void);
E_FUNC char *calc_strdup(CONST char *);

/*
 * Initialization
 */
E_FUNC void initialize(void);
E_FUNC void reinitialize(void);
#if !defined (_WIN32)
E_FUNC int isatty(int tty);	/* TRUE if fd is a tty */
#endif
E_FUNC char *version(void);	/* return version string */

/*
 * global flags and definitions
 */
EXTERN int abortlevel;	/* current level of aborts */
EXTERN BOOL inputwait;	/* TRUE if in a terminal input wait */

EXTERN int p_flag;		/* TRUE => pipe mode */
EXTERN int q_flag;		/* TRUE => don't execute rc files */
EXTERN int u_flag;		/* TRUE => unbuffer stdin and stdout */
EXTERN int d_flag;		/* TRUE => disable heading, resource_debug */
EXTERN int c_flag;		/* TRUE => continue after error if permitted */
EXTERN int i_flag;		/* TRUE => try to go interactive after error */
E_FUNC int s_flag;		/* TRUE => keep args as strings for argv() */
EXTERN long stoponerror;	/* >0 => stop, <0 => continue, ==0 => use -c */
EXTERN BOOL abort_now;	/* TRUE => try to go interactive */

E_FUNC int argc_value;	/* count of argv[] strings for argv() builtin */
E_FUNC char **argv_value;	/* argv[] strings for argv() builtin */

EXTERN char *pager;		/* $PAGER or default */
EXTERN int stdin_tty;	/* TRUE if stdin is a tty */
EXTERN int havecommands;	/* TRUE if have cmd args) */
EXTERN char *program;	/* our name */
EXTERN char *base_name;	/* basename of our name */
EXTERN char cmdbuf[];	/* command line expression */
EXTERN char *script_name;	/* program name or -f filename arg or NULL */

EXTERN int abortlevel;	/* current level of aborts */
EXTERN BOOL inputwait;	/* TRUE if in a terminal input wait */
EXTERN VALUE *stack;	/* execution stack */
EXTERN int dumpnames;	/* TRUE => dump names rather than indices */

EXTERN char *calcpath;	/* $CALCPATH or default */
EXTERN char *calcrc;	/* $CALCRC or default */
EXTERN char *calcbindings;	/* $CALCBINDINGS or default */
EXTERN char *home;		/* $HOME or default */
EXTERN char *shell;		/* $SHELL or default */

EXTERN int no_env;	/* TRUE (-e) => ignore env vars on startup */
EXTERN long errmax;	/* if >= 0, error when errcount exceeds errmax */
EXTERN int use_old_std;	/* TRUE (-O) => use classic configuration */

EXTERN int allow_read;	/* FALSE => don't open any files for reading */
EXTERN int allow_write; 	/* FALSE => don't open any files for writing */
EXTERN int allow_exec;	/* FALSE => may not execute any commands */

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
EXTERN run run_state;
E_FUNC char *run_state_name(run state);

/*
 * calc version information
 */
#define CALC_TITLE "C-style arbitrary precision calculator"
EXTERN int calc_major_ver;
EXTERN int calc_minor_ver;
EXTERN int calc_major_patch;
EXTERN int calc_minor_patch;
EXTERN char *Copyright;
E_FUNC char *version(void);


#endif /* !INCLUDE_CALC_H */
