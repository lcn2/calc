/*
 * Copyright (c) 1997 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Definitions for calculator program.
 */


#if !defined(__CALC_H__)
#define	__CALC_H__

#include <setjmp.h>

#include "value.h"


/*
 * Configuration definitions
 */
#define	CALCPATH	"CALCPATH"	/* environment variable for files */
#define	CALCRC		"CALCRC"	/* environment variable for startup */
#define	CALCBINDINGS	"CALCBINDINGS"	/* environment variable for hist bindings */
#define	HOME		"HOME"		/* environment variable for home dir */
#define	PAGER		"PAGER"		/* environment variable for help */
#define	SHELL		"SHELL"		/* environment variable for shell */
#define DEFAULTCALCHELP	"help"		/* help file that -h prints */
#define DEFAULTSHELL	"sh"		/* default shell to use */
#define	CALCEXT		".cal"	/* extension for files read in */
#define	MAX_CALCRC	1024	/* maximum length of $CALCRC */
#define	HOMECHAR	'~'	/* char which indicates home directory */
#define DOTCHAR		'.'	/* char which indicates current directory */
#define	PATHCHAR	'/'	/* char which separates path components */
#define	LISTCHAR	':'	/* char which separates paths in a list */
#define	MAXCMD		16384	/* maximum length of command invocation */
#define	MAXERROR	512	/* maximum length of error message string */

#define	SYMBOLSIZE	256	/* maximum symbol name size */
#define	MAXINDICES	20	/* maximum number of indices for objects */
#define	MAXLABELS	100	/* maximum number of user labels in function */
#define	MAXOBJECTS	128	/* maximum number of object types */
#define	MAXSTRING	1024	/* maximum size of string constant */
#define	MAXSTACK	1000	/* maximum depth of evaluation stack */
#define	MAXFILES	20	/* maximum number of opened files */
#define PROMPT1		"> "	/* default normal prompt*/
#define PROMPT2		">> "	/* default prompt inside multi-line input */


#define	TRACE_NORMAL	0x00	/* normal trace flags */
#define	TRACE_OPCODES	0x01	/* trace every opcode */
#define	TRACE_NODEBUG	0x02	/* suppress debugging opcodes */
#define TRACE_LINKS	0x04	/* display links for real and complex numbers */
#define TRACE_FNCODES	0x08	/* display code for newly defined function */
#define	TRACE_MAX	0x0f	/* maximum value for trace flag */

#define ABORT_NONE	0	/* abort not needed yet */
#define ABORT_STATEMENT	1	/* abort on statement boundary */
#define ABORT_OPCODE	2	/* abort on any opcode boundary */
#define ABORT_MATH	3	/* abort on any math operation */
#define ABORT_NOW	4	/* abort right away */

#define ERRMAX 20		/* default errmax value */

/*
 * File ids corresponding to standard in, out, error, and when not in use.
 */
#define	FILEID_STDIN	((FILEID) 0)
#define	FILEID_STDOUT	((FILEID) 1)
#define	FILEID_STDERR	((FILEID) 2)
#define	FILEID_NONE	((FILEID) -1)

/*
 * File I/O routines.
 */
extern FILEID openid(char *name, char *mode);
extern FILEID indexid(long index);
extern BOOL validid(FILEID id);
extern BOOL errorid(FILEID id);
extern BOOL eofid(FILEID id);
extern int closeid(FILEID id);
extern int getcharid(FILEID id);
extern int idprintf(FILEID id, char *fmt, int count, VALUE **vals);
extern int idfputc(FILEID id, int ch);
extern int idfputs(FILEID id, char *str);
extern int printid(FILEID id, int flags);
extern int flushid(FILEID id);
extern int readid(FILEID id, int flags, char **retptr);
extern int getloc(FILEID id, ZVALUE *loc);
extern int setloc(FILEID id, ZVALUE zpos);
extern int getsize(FILEID id, ZVALUE *size);
extern int get_device(FILEID id, ZVALUE *dev);
extern int get_inode(FILEID id, ZVALUE *ino);
extern FILEID reopenid(FILEID id, char *mode, char *name);
extern int closeall(void);
extern int flushall(void);
extern int idfputstr(FILEID id, char *str);
extern int rewindid(FILEID id);
extern void rewindall(void);
extern ZVALUE zfilesize(FILEID id);
extern void showfiles(void);
extern int fscanfid(FILEID id, char *fmt, int count, VALUE **vals);
extern int scanfstr(char *str, char *fmt, int count, VALUE **vals);
extern int ftellid(FILEID id, ZVALUE *res);
extern int fseekid(FILEID id, ZVALUE offset, int whence);
extern int isattyid(FILEID id);
extern int fsearch(FILEID id, char *str, ZVALUE start, ZVALUE end, ZVALUE *res);
extern int frsearch(FILEID id, char *str, ZVALUE first, ZVALUE last, ZVALUE *res);
extern void showconstants(void);
extern void freeconstant(unsigned long);
extern void freestringconstant(long);
extern void trimconstants(void);

/*
 * Input routines.
 */
extern int openstring(char *str);
extern int openterminal(void);
extern int opensearchfile(char *name, char *pathlist, char *exten, int reopen_ok);
extern char *nextline(void);
extern int nextchar(void);
extern void reread(void);
extern void resetinput(void);
extern void setprompt(char *);
extern BOOL inputisterminal(void);
extern char *inputname(void);
extern long linenumber(void);
extern void runrcfiles(void);
extern void closeinput(void);

/*
 * Other routines.
 */
extern NUMBER *constvalue(unsigned long index);
extern long addnumber(char *str);
extern long addqconstant(NUMBER *q);
extern void initstack(void);
extern void getcommands(BOOL toplevel);
extern void givehelp(char *type);
extern void libcalc_call_me_first(void);
extern void libcalc_call_me_last(void);
extern void showerrors(void);

/*
 * Initialization
 */
extern void initialize(void);
extern void reinitialize(void);
extern int isatty(int tty);	/* TRUE if fd is a tty */
extern char *version(void);	/* return version string */
extern int post_init;		/* TRUE => setjmp for math_error is ready */

/*
 * global flags and definitions
 */
extern int abortlevel;		/* current level of aborts */
extern BOOL inputwait;		/* TRUE if in a terminal input wait */
extern jmp_buf jmpbuf;		/* for errors */

extern int p_flag;		/* TRUE => pipe mode */
extern int q_flag;		/* TRUE => don't execute rc files */
extern int u_flag;		/* TRUE => unbuffer stdin and stdout */
extern int d_flag;		/* TRUE => disable heading, lib_debug == 0 */
extern int c_flag;		/* TRUE => continue after error if permitted */
extern int i_flag;		/* TRUE => try to go interactive after error */
extern int stoponerror;		/* >0 => stop, <0 => continue, ==0 => use -c */

extern char *pager;		/* $PAGER or default */
extern int stdin_tty;		/* TRUE if stdin is a tty */
extern int havecommands;	/* TRUE if have cmd args) */
extern char *program;		/* our name */
extern char cmdbuf[];		/* command line expression */

extern int abortlevel;		/* current level of aborts */
extern BOOL inputwait;		/* TRUE if in a terminal input wait */
extern VALUE *stack;		/* execution stack */
extern int dumpnames;		/* TRUE => dump names rather than indices */

extern char *calcpath;		/* $CALCPATH or default */
extern char *calcrc;		/* $CALCRC or default */
extern char *calcbindings;	/* $CALCBINDINGS or default */
extern char *home;		/* $HOME or default */
extern char *shell;		/* $SHELL or default */
extern char *program;		/* our name (argv[0]) */

extern int no_env;	/* TRUE (-e) => ignore env vars on startup */
extern int errmax;	/* if >= 0, error when errcount exceeds errmax */
extern int new_std;	/* TRUE (-n) => use newstd configuration */

extern int allow_read;	/* FALSE => may not open any files for reading */
extern int allow_write;	/* FALSE => may not open any files for writing */
extern int allow_exec;	/* FALSE => may not execute any commands */

/*
 * calc startup and run state
 */
typedef enum {
    RUN_UNKNOWN = -1,		/* unknown or unset start state */
    RUN_PRE_BEGIN = 0,		/* pre-startup state, calc execution started */
    RUN_PRE_RCFILES = 1,	/* rc files about to or are being evaluated */
    RUN_POST_RCFILES = 2,	/* rc files have been evaluated */
    RUN_PRE_CMD_ARGS = 3,	/* cmd_args about to or are being evaluated */
    RUN_POST_CMD_ARGS = 4,	/* cmd_args have been evaluated */
    RUN_TOP_LEVEL = 5,		/* running at the top interactive level */
    RUN_NOT_TOP_LEVEL = 6,	/* running not at the top interactive level */
    RUN_STOP_ON_ERROR = 7	/* we need to stop due to errors */
} run;
extern run run_state;

/*
 * calc version information
 */
#define CALC_TITLE "C-style arbitrary precision calculator"
extern int calc_major_ver;
extern int calc_minor_ver;
extern int calc_major_patch;
extern char *calc_minor_patch;
extern char *version(void);	/* return version string */


#endif /* !__CALC_H__ */
