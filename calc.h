/*
 * calc - definitions for calculator program
 *
 * Copyright (C) 1999-2007,2014,2021,2023,2026  David I. Bell
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
 * Under source code control:   1990/02/15 01:48:31
 * File existed as early as:    before 1990
 *
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

#if !defined(INCLUDE_CALC_H)
#  define INCLUDE_CALC_H

/*
 * Configuration definitions
 */
#  define CALCPATH "CALCPATH"             /* environment variable for files */
#  define CALCRC "CALCRC"                 /* environment variable for startup */
#  define CALCBINDINGS "CALCBINDINGS"     /* env variable for hist bindings */
#  define HOME "HOME"                     /* environment variable for home dir */
#  define PAGER "PAGER"                   /* environment variable for help */
#  define SHELL "SHELL"                   /* environment variable for shell */
#  define CALCHISTFILE "CALCHISTFILE"     /* history file environment variable */
#  define CALCHELP "CALCHELP"             /* help directory env variable */
#  define CALCCUSTOMHELP "CALCCUSTOMHELP" /* custom help directory env variable */
#  define DEFAULTCALCBINDINGS "bindings"  /* default calc bindings file */
#  define DEFAULTCALCHELP "help"          /* help file that -h prints */
#  define DEFAULTSHELL "sh"               /* default shell to use */
#  define CALCEXT ".cal"                  /* extension for files read in */
#  define MAX_CALCRC 1024                 /* maximum length of $CALCRC */
#  define HOMECHAR '~'                    /* char which indicates home directory */
#  define DOTCHAR '.'                     /* char which indicates current directory */
#  define PATHCHAR '/'                    /* char which separates path components */
#  if defined(_WIN32) || defined(_WIN64)
#    define LISTCHAR ';' /* char which separates paths in a list */
#  else
#    define LISTCHAR ':' /* char which separates paths in a list */
#  endif
#  define MAXCMD 5120 /* maximum length of command invocation */

#  define SYMBOLSIZE 256 /* maximum symbol name size */
#  define MAXLABELS 100  /* maximum number of user labels in function */
#  define MAXSTACK 2048  /* maximum depth of evaluation stack */
#  define MAXFILES 256   /* maximum number of opened files */
#  define PROMPT1 "> "   /* default normal prompt*/
#  define PROMPT2 ">> "  /* default prompt inside multi-line input */

#  define TRACE_NORMAL 0x00  /* normal trace flags */
#  define TRACE_OPCODES 0x01 /* trace every opcode */
#  define TRACE_NODEBUG 0x02 /* suppress debugging opcodes */
#  define TRACE_LINKS 0x04   /* display links for real and complex numbers */
#  define TRACE_FNCODES 0x08 /* display code for newly defined function */
#  define TRACE_MAX 0x0f     /* maximum value for trace flag */

#  define ABORT_NONE 0      /* abort not needed yet */
#  define ABORT_STATEMENT 1 /* abort on statement boundary */
#  define ABORT_OPCODE 2    /* abort on any opcode boundary */
#  define ABORT_MATH 3      /* abort on any math operation */
#  define ABORT_NOW 4       /* abort right away */

#  define ERRMAX 20 /* default errmax value */
#  define E_OK 0    /* no error */

/*
 * File ids corresponding to standard in, out, error, and when not in use.
 */
#  define FILEID_STDIN ((FILEID)0)
#  define FILEID_STDOUT ((FILEID)1)
#  define FILEID_STDERR ((FILEID)2)
#  define FILEID_NONE ((FILEID) - 1) /* must be < 0 */

/*
 * File I/O routines.
 */
extern FILEID openid(char *name, char *mode);
extern FILEID openpathid(char *name, char *mode, char *pathlist);
extern FILEID indexid(long index);
extern bool validid(FILEID id);
extern bool errorid(FILEID id);
extern bool eofid(FILEID id);
extern int closeid(FILEID id);
extern int getcharid(FILEID id);
extern int idprintf(FILEID id, char *fmt, int count, VALUE **vals);
extern int idfputc(FILEID id, int ch);
extern int idfputs(FILEID id, STRING *str);
extern int printid(FILEID id, int flags);
extern int flushid(FILEID id);
extern int readid(FILEID id, int flags, STRING **retptr);
extern int getloc(FILEID id, ZVALUE *loc);
extern int setloc(FILEID id, ZVALUE zpos);
extern int getsize(FILEID id, ZVALUE *size);
extern int get_device(FILEID id, ZVALUE *dev);
extern int get_inode(FILEID id, ZVALUE *ino);
extern FILEID reopenid(FILEID id, char *mode, char *name);
extern int closeall(void);

#  if !defined(_WIN32) && !defined(_WIN64)
extern int flushall(void);
#  endif

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
extern int openstring(char *str, size_t num);
extern int openterminal(void);
extern int opensearchfile(char *name, char *pathlist, char *exten, int reopen_ok);
extern char *nextline(void);
extern int nextchar(void);
extern void reread(void);
extern void resetinput(void);
extern void setprompt(char *);
extern bool inputisterminal(void);
extern int inputlevel(void);
extern long calclevel(void);
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
extern void getcommands(bool toplevel);
extern void givehelp(char *type);
extern void libcalc_call_me_first(void);
extern void libcalc_call_me_last(void);
extern bool calc_tty(int fd);
extern bool orig_tty(int fd);
extern void showerrors(void);
extern char *calc_strdup(const char *);

/*
 * Initialization
 */
extern void initialize(void);
extern void reinitialize(void);
#  if !defined(_WIN32) && !defined(_WIN64)
extern int isatty(int tty); /* true if fd is a tty */
#  endif
extern char *version(void); /* return version string */

/*
 * global flags and definitions
 */
extern int abortlevel; /* current level of aborts */
extern bool inputwait; /* true if in a terminal input wait */

extern int p_flag;       /* true => pipe mode */
extern int q_flag;       /* true => don't execute rc files */
extern int u_flag;       /* true => unbuffer stdin and stdout */
extern int d_flag;       /* true => disable heading, resource_debug */
extern int c_flag;       /* true => continue after error if permitted */
extern int i_flag;       /* true => try to go interactive after error */
extern int s_flag;       /* true => keep args as strings for argv() */
extern long stoponerror; /* >0 => stop, <0 => continue, ==0 => use -c */
extern bool abort_now;   /* true => try to go interactive */

extern int argc_value;    /* count of argv[] strings for argv() builtin */
extern char **argv_value; /* argv[] strings for argv() builtin */

extern char *pager;       /* $PAGER or default */
extern int stdin_tty;     /* true if stdin is a tty */
extern int havecommands;  /* true if have cmd args) */
extern char *program;     /* our name */
extern char *base_name;   /* basename of our name */
extern char cmdbuf[];     /* command line expression */
extern char *script_name; /* program name or -f filename arg or NULL */

extern int abortlevel; /* current level of aborts */
extern bool inputwait; /* true if in a terminal input wait */
extern VALUE *stack;   /* execution stack */
extern int dumpnames;  /* true => dump names rather than indices */
extern int calc_errno; /* global calc_errno value */

extern char *calcpath;     /* $CALCPATH or default */
extern char *calcrc;       /* $CALCRC or default */
extern char *calcbindings; /* $CALCBINDINGS or default */
extern char *home;         /* $HOME or default */
extern char *shell;        /* $SHELL or default */

extern int no_env;      /* true (-e) => ignore env vars on startup */
extern long errmax;     /* if >= 0, error when errcount exceeds errmax */
extern int use_old_std; /* true (-O) => use classic configuration */

extern int allow_read;  /* false => don't open any files for reading */
extern int allow_write; /* false => don't open any files for writing */
extern int allow_exec;  /* false => may not execute any commands */

/*
 * calc startup and run state
 */
typedef enum {
    RUN_ZERO,           /* unknown or unset start state */
    RUN_BEGIN,          /* calc execution starts */
    RUN_RCFILES,        /* rc files being evaluated */
    RUN_PRE_CMD_ARGS,   /* prepare to evaluate cmd args */
    RUN_CMD_ARGS,       /* cmd args being evaluated */
    RUN_PRE_TOP_LEVEL,  /* prepare to start top level activity */
    RUN_TOP_LEVEL,      /* running at top level */
    RUN_EXIT,           /* normal exit from calc */
    RUN_EXIT_WITH_ERROR /* exit with error */
} run;
extern run run_state;
extern char *run_state_name(run state);

/*
 * calc version information
 */
#  define CALC_TITLE "C-style arbitrary precision calculator"
extern int calc_major_ver;
extern int calc_minor_ver;
extern int calc_major_patch;
extern int calc_minor_patch;
extern char *Copyright;
extern char *version(void);

#endif
