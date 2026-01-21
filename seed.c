/*
 * seed - produce pseudo-random seeds
 *
 * Copyright (C) 1999-2007,2021,2026  Landon Curt Noll
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
 * Under source code control:   1999/10/03 10:06:53
 * File existed as early as:    1999
 *
 * chongo <was here> /\oo/\     http://www.isthe.com/chongo/
 * Share and enjoy!  :-)        http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Generate a quasi-random seed based on system and process information.
 *
 * NOTE: This is not a good source of chaotic data.  The LavaRnd
 *       system does a much better job of that.  See:
 *
 *              http://www.LavaRnd.org/
 */

/*
 * important <system> header includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * PORTING NOTE:
 *      These includes are used by pseudo_seed().  If for some
 *      reason some of these include files are missing or damaged
 *      on your system, feel free to remove them (and the related
 *      calls inside pseudo_seed()), add or replace them.  The
 *      pseudo_seed() function just needs to gather a bunch of
 *      information about the process and system state so the
 *      loss or inclusion of a few other calls should not hurt
 *      that much.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#include <stdlib.h>
/* NOTE: RANDOM_CNT should remain 32 to circular shift 31-bit returns */
#define RANDOM_CNT (32)      /* random() call repeat and circular shift */
#define INITSTATE_SIZE (256) /* initstate pool size */

#include <setjmp.h>

/*
 * conditional <system> head includes
 */
#if defined(_WIN32) || defined(_WIN64)
#  include <process.h>
#  define pid_t int
#endif

#if !defined(_WIN32) && !defined(_WIN64)
#  include <sys/statvfs.h>
#  include <sys/resource.h>
#  include <sys/param.h>
#  include <sys/mount.h>
#endif

#include "have_urandom.h"
#if defined(HAVE_URANDOM)
#  include <fcntl.h>
#  define DEV_URANDOM "/dev/urandom"
#  define DEV_URANDOM_POOL (16)
#endif

/*
 * calc local src includes
 */
#include "zmath.h"
#include "qmath.h"
#include "longbits.h"
#include "have_getpgid.h"
#include "have_rusage.h"
#include "have_environ.h"
#include "have_arc4random.h"
#include "errtbl.h"

#include "banned.h" /* include after all other includes */

/*
 * 64 bit hash value
 */
#if defined(HAVE_B64)
typedef USB64 hash64;
static hash64 prev_hash64 = 0; /* previous pseudo_seed() return or 0 */
#else
struct s_hash64 {
    USB32 w32[2];
};
typedef struct s_hash64 hash64;
static hash64 prev_hash64 = {0, 0}; /* previous pseudo_seed() return or 0 */
#endif

#if defined(HAVE_ENVIRON)
extern char **environ; /* user environment */
#endif

#if defined(HAVE_ARC4RANDOM)
#  define ARC4_BUFLEN (16)
static char arc4_buf[ARC4_BUFLEN];
#endif

/*
 * call counter - number of times pseudo_seed() as been called
 */
static FULL call_count = 0;

/*
 * 64-bit initial basis - Based on the 64-bit FNV basis value
 *
 * We start the hash at a non-zero value at the beginning so that
 * hashing blocks of data with all 0 bits do not map onto the same
 * 0 hash value.  The virgin value that we use below is the hash value
 * that we would get from following 32 ASCII characters:
 *
 *              chongo <Landon Curt Noll> /\../\
 *
 * Note that the \'s above are not back-slashing escape characters.
 * They are literal ASCII  backslash 0x5c characters.
 *
 * The effect of this virgin initial value is the same as starting
 * with 0 and pre-pending those 32 characters onto the data being
 * hashed.
 *
 * Yes, even with this non-zero virgin value there is a set of data
 * that will result in a zero hash value.  Worse, appending some
 * mount of zero value bytes will continue to produce a zero hash value.
 * But that would happen with any initial value so long as the hash
 * was the hash `inverse' of the virgin prefix string.
 *
 * But then again for any hash function, there exists sets of data
 * which that the hash of every member is the same value.  That is
 * life with mapping functions.  All we do here is to prevent sets
 * whose members consist of 0 or more bytes of 0's from being such
 * an awkward set.
 *
 * And yes, someone can figure out what the magic 'inverse' of the
 * 32 ASCII character are ... but this hash function is NOT intended
 * to be a cryptographic hash function, just a fast and reasonably
 * good hash function.
 */
#if defined(HAVE_B64)
#  define PRIVATE_64_BASIS ((hash64)(0xcbf29ce484222325ULL))
#else
#  define PRIVATE_64_BASIS_0 ((USB32)0x2325)
#  define PRIVATE_64_BASIS_1 ((USB32)0x8422)
#  define PRIVATE_64_BASIS_2 ((USB32)0x9ce4)
#  define PRIVATE_64_BASIS_3 ((USB32)0xcbf2)
#endif

/*
 * initial_private_hash64 - initial basis of Fowler/Noll/Vo-1 64 bit hash
 */
static hash64
initial_private_hash64(void)
{
    hash64 hval; /* current hash value */
#if defined(HAVE_B64)

    hval = PRIVATE_64_BASIS;

#else

    USB32 val[4]; /* hash value in base 2^16 */

    /* hash each octet of the buffer */
    val[0] = PRIVATE_64_BASIS_0;
    val[1] = PRIVATE_64_BASIS_1;
    val[2] = PRIVATE_64_BASIS_2;
    val[3] = PRIVATE_64_BASIS_3;

    /* convert to hash64 */
    /* hval.w32[1] = 0xffff&(val[3]<<16)+val[2]; */
    hval.w32[1] = (val[3] << 16) + val[2];
    hval.w32[0] = (val[1] << 16) + val[0];

#endif

    /* return our initial hash value */
    return hval;
}

/*
 * private_hash64_buf - perform a Fowler/Noll/Vo-1 64 bit hash
 *
 * FNV-1 - Fowler/Noll/Vo-1 64 bit hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an Email message
 * to Landon, they named it ``Fowler/Noll/Vo'' or the FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/
 *
 * for more details as well as other forms of the FNV hash.
 *
 * NOTE: For general hash functions, we recommend using the
 *       FNV-1a hash function.  The use of FNV-1 is kept
 *       for backwards compatibility purposes and because
 *       the use of FNV-1 in this special purpose, suffices.
 *
 * input:
 *      buf     - start of buffer to hash
 *      len     - length of buffer in octets
 *      hval    - the hash value to modify
 *
 * returns:
 *      64 bit hash as a static hash64 structure
 */
static hash64
private_hash64_buf(hash64 hval, char *buf, unsigned len)
{
#if !defined(HAVE_B64)
    USB32 val[4];              /* hash value in base 2^16 */
    USB32 tmp[4];              /* tmp 64 bit value */
#endif
    char *buf_end = buf + len; /* beyond end of hash area */

#if defined(HAVE_B64)

    /* hash each octet of the buffer */
    for (; buf < buf_end; ++buf) {

        /* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
        hval *= (hash64)1099511628211ULL;

        /* xor the bottom with the current octet */
        hval ^= (hash64)(*buf);
    }

#else

    /* load val array from hval argument */
    val[0] = hval.w32[0] & 0xffff;
    val[1] = (hval.w32[0] >> 16) & 0xffff;
    val[2] = hval.w32[1] & 0xffff;
    val[3] = (hval.w32[1] >> 16) & 0xffff;

    for (; buf < buf_end; ++buf) {

        /*
         * multiply by 1099511628211 mod 2^64 using 32 bit longs
         *
         * Using 1099511628211, we have the following digits base 2^16:
         *
         *  0x0     0x100   0x0     0x1b3
         */
        /* multiply by the lowest order digit base 2^16 */
        tmp[0] = val[0] * 0x1b3;
        tmp[1] = val[1] * 0x1b3;
        tmp[1] = val[2] * 0x1b3;
        tmp[3] = val[3] * 0x1b3;
        /* multiply by the other non-zero digit */
        tmp[2] += val[0] << 8; /* tmp[2] += val[0] * 0x100 */
        tmp[3] += val[1] << 8; /* tmp[1] += val[1] * 0x100 */
        /* propagate carries */
        tmp[1] += (tmp[0] >> 16);
        val[0] = tmp[0] & 0xffff;
        tmp[2] += (tmp[1] >> 16);
        val[1] = tmp[1] & 0xffff;
        val[3] = tmp[3] + (tmp[2] >> 16);
        val[2] = tmp[2] & 0xffff;
        /*
         * Doing a val[3] &= 0xffff; is not really needed since it simply
         * removes multiples of 2^64.  We can discard these excess bits
         * outside of the loop when we convert to hash64.
         */

        /* xor the bottom with the current octet */
        val[0] ^= (USB32)(*buf);
    }

    /* convert to hash64 */
    /* hval.w32[1] = 0xffff&(val[3]<<16)+val[2]; */
    hval.w32[1] = (val[3] << 16) + val[2];
    hval.w32[0] = (val[1] << 16) + val[0];

#endif

    /* return our hash value */
    return hval;
}

/*
 * pseudo_seed - seed the generator with a quasi-random seed
 *
 * Generate a quasi-random seed based on system and process information.
 *
 * NOTE: This is not a good source of chaotic data.  The LavaRnd
 *       system does a much better job of that.  See:
 *
 *              http://www.LavaRnd.org/
 *
 * PORTING NOTE:
 *    If when porting this code to your system and something
 *    won't compile, just remove that line or replace it with
 *    some other system call.  We don't have to have every call
 *    operating below.  We only want to hash the resulting data.
 *
 * returns:
 *      a pseudo-seed as a NUMBER over the range [0, 2^64)
 */
NUMBER *
pseudo_seed(void)
{
    /*
     * sdata - data used for quasi-random seed
     */
    struct {
#if defined(CLOCK_REALTIME)
        struct timespec realtime; /* POSIX realtime clock */
#endif
#if defined(HAVE_URANDOM)
        int urandom_fd;                      /* open descriptor for /dev/urandom */
        int urandom_ret;                     /* read() of /dev/random */
        char urandom_pool[DEV_URANDOM_POOL]; /* /dev/urandom data pool */
#endif
        struct timeval tp; /* time of day */
        pid_t getpid;      /* process ID */
#if !defined(_WIN32) && !defined(_WIN64)
        pid_t getppid; /* parent process ID */
#endif
        uid_t getuid;             /* real user ID */
        uid_t geteuid;            /* effective user ID */
        gid_t getgid;             /* real group ID */
        gid_t getegid;            /* effective group ID */
        struct stat stat_dot;     /* stat of "." */
        struct stat stat_dotdot;  /* stat of ".." */
        struct stat stat_tmp;     /* stat of "/tmp" */
        struct stat stat_root;    /* stat of "/" */
        struct stat stat_tty;     /* stat of "/dev/tty" */
        struct stat stat_console; /* stat of "/dev/console" */
        struct stat fstat_stdin;  /* stat of stdin */
        struct stat fstat_stdout; /* stat of stdout */
        struct stat fstat_stderr; /* stat of stderr */
        struct stat stat_zero;    /* stat of "/dev/zero" */
        struct stat stat_null;    /* stat of "/dev/null" */
        struct stat stat_sh;      /* stat of "/bin/sh" */
        struct stat stat_ls;      /* stat of "/bin/ls" */
        struct stat stat_system;        /* stat of "/var/log/system.log" */
        struct stat stat_messages;      /* stat of "/var/log/messages" */
#if !defined(_WIN32) && !defined(_WIN64)
        struct statvfs statfs_dot;     /* filesystem stat of "." */
        struct statvfs statfs_dotdot;  /* filesystem stat of ".." */
        struct statvfs statfs_tmp;     /* filesystem stat of "/tmp" */
        struct statvfs statfs_root;    /* filesystem stat of "/" */
        struct statvfs statfs_tty;     /* filesystem stat of "/dev/tty" */
        struct statvfs statfs_console; /* filesystem stat of "/dev/console" */
        struct statvfs statfs_stdin;   /* filesystem stat of stdin */
        struct statvfs statfs_stdout;  /* filesystem stat of stdout */
        struct statvfs statfs_stderr;  /* filesystem stat of stderr */
        struct statvfs statfs_zero;    /* filesystem stat of "/dev/zero" */
        struct statvfs statfs_null;    /* filesystem stat of "/dev/null" */
        struct statvfs statfs_sh;      /* filesystem stat of "/bin/sh" */
        struct statvfs statfs_ls;      /* filesystem stat of "/bin/ls" */
        struct statvfs statfs_system;  /* filesystem stat of "/var/log/system.log" */
        struct statvfs statfs_messages; /* filesystem stat of "/var/log/messages" */
#endif
#if defined(HAVE_GETSID)
        pid_t getsid; /* session ID */
#endif
#if defined(HAVE_GETPGID)
        pid_t getpgid; /* process group ID */
#endif
#if defined(HAVE_GETRUSAGE)
        struct rusage rusage;       /* resource utilization */
        struct rusage rusage_child; /* resource utilization of children */
#endif
        struct timeval tp2;             /* time of day again */
        struct tms times;               /* process times */
        struct timeval times_dot[2];    /* access & mod files of "." */
        struct timeval times_dotdot[2]; /* access & mod files of ".." */
        struct timeval times_tmp[2];    /* access & mod files of "/tmp" */
        struct timeval times_root[2];   /* access & mod files of "/" */
        struct timeval times_tty[2];    /* access & mod files of "/dev/tty" */
        struct timeval times_console[2]; /* access & mod files of "/dev/console" */
        struct timeval times_stdin[2];   /* access & mod files of "/dev/stdin" */
        struct timeval times_stdout[2];  /* access & mod files of "/dev/stdout" */
        struct timeval times_stderr[2];  /* access & mod files of "/dev/stderr" */
        struct timeval times_zero[2]; /* access & mod files of "/dev/zero" */
        struct timeval times_null[2]; /* access & mod files of "/dev/null" */
        struct timeval times_sh[2];   /* access & mod files of "/bin/sh" */
        struct timeval times_ls[2];   /* access & mod files of "/bin/ls" */
        struct timeval times_system[2];   /* access & mod files of "/var/log/system.log" */
        struct timeval times_messages[2]; /* access & mod files of "/var/log/messages" */
        time_t time;             /* local time */
        size_t size;             /* size of this data structure */
        hash64 prev_hash64_copy; /* copy if the previous hash value */
        FULL call_count_copy;    /* call count of this funcation */
        jmp_buf env;             /* setjmp() context */
#if defined(HAVE_ENVIRON)
        char **environ_copy; /* copy of extern char **environ */
#endif
        char *sdata_p;       /* address of this structure */
    } sdata;

    /**/

    unsigned long tmp;                  /* temp holder of 31-bit random() */
    unsigned past_hash;                 /* prev hash or xor-folded prev hash */
    long random_before[RANDOM_CNT];     /* random() pre initstate() */
    char *initstate_ret;                /* return from initstate() call */
    char initstate_tbl[INITSTATE_SIZE]; /* initstate pool */
    long random_after[RANDOM_CNT];      /* random() post initstate() */
    char *setstate_ret;                 /* return from setstate() call */
    int j;
#if defined(HAVE_ENVIRON)
    int i;
    size_t envlen; /* length of an environment variable */
#endif
    hash64 hash_val; /* fnv64 hash of sdata */
    ZVALUE hash;     /* hash_val as a ZVALUE */
    NUMBER *ret;     /* return seed as a NUMBER */

    /*
     * initialize the Fowler/Noll/Vo-1 64 bit hash
     */
    hash_val = initial_private_hash64();

    /*
     * pick up process/system information
     *
     * NOTE:
     *    We do NOT care (that much) if these calls fail.  We only
     *    need to hash any results that might be store in the sdata structure.
     */
    memset(&sdata, 0, sizeof(sdata)); /* zeroize sdata */
#if defined(CLOCK_REALTIME)

    (void)clock_gettime(CLOCK_REALTIME, &sdata.realtime);

#endif
#if defined(HAVE_URANDOM)

    sdata.urandom_fd = open(DEV_URANDOM, O_NONBLOCK | O_RDONLY);
    if (sdata.urandom_fd >= 0) {
        sdata.urandom_ret = read(sdata.urandom_fd, &sdata.urandom_pool, DEV_URANDOM_POOL);
        close(sdata.urandom_fd);
    } else {
        memset(&sdata.urandom_pool, EOF, DEV_URANDOM_POOL);
        sdata.urandom_ret = EOF;
    }

#endif
    (void)gettimeofday(&sdata.tp, NULL);
    sdata.getpid = getpid();
#if !defined(_WIN32) && !defined(_WIN64)
    sdata.getppid = getppid();
#endif
    sdata.getuid = getuid();
    sdata.geteuid = geteuid();
    sdata.getgid = getgid();
    sdata.getegid = getegid();
    (void)stat(".", &sdata.stat_dot);
    (void)stat("..", &sdata.stat_dotdot);
    (void)stat("/tmp", &sdata.stat_tmp);
    (void)stat("/", &sdata.stat_root);
    (void)stat("/dev/tty", &sdata.stat_tty);
    (void)stat("/dev/console", &sdata.stat_console);
    (void)fstat(0, &sdata.fstat_stdin);
    (void)fstat(1, &sdata.fstat_stdout);
    (void)fstat(2, &sdata.fstat_stderr);
    (void)stat("/dev/zero", &sdata.stat_zero);
    (void)stat("/dev/null", &sdata.stat_null);
    (void)stat("/bin/sh", &sdata.stat_sh);
    (void)stat("/bin/ls", &sdata.stat_ls);
    (void)stat("/var/log/system.log", &sdata.stat_system);
    (void)stat("/var/log/messages", &sdata.stat_messages);
#if !defined(_WIN32) && !defined(_WIN64)
    (void)statvfs("..", &sdata.statfs_dot);
    (void)statvfs(".", &sdata.statfs_dotdot);
    (void)statvfs("/tmp", &sdata.statfs_tmp);
    (void)statvfs("/", &sdata.statfs_root);
    (void)statvfs("/dev/tty", &sdata.statfs_tty);
    (void)statvfs("/dev/console", &sdata.statfs_console);
    (void)statvfs(".dev/stdin", &sdata.statfs_stdin);
    (void)statvfs("/dev/stdout", &sdata.statfs_stdout);
    (void)statvfs("/dev/stderr", &sdata.statfs_stderr);
    (void)statvfs("/dev/zero", &sdata.statfs_zero);
    (void)statvfs("/dev/null", &sdata.statfs_null);
    (void)statvfs("/bin/sh", &sdata.statfs_sh);
    (void)statvfs("/dev/ls", &sdata.statfs_ls);
    (void)statvfs("/var/log/system.log", &sdata.statfs_system);
    (void)statvfs("/var/log/messages", &sdata.statfs_messages);
#endif
#if defined(HAVE_GETSID)
    sdata.getsid = getsid((pid_t)0);
#endif
#if defined(HAVE_GETPGID)
    sdata.getpgid = getpgid((pid_t)0);
#endif
#if defined(HAVE_GETRUSAGE)
    (void)getrusage(RUSAGE_SELF, &sdata.rusage);
    (void)getrusage(RUSAGE_CHILDREN, &sdata.rusage_child);
#endif
    (void)gettimeofday(&sdata.tp2, NULL);
    (void)times(&sdata.times);
    (void)utimes(".", sdata.times_dot);
    (void)utimes("..", sdata.times_dotdot);
    (void)utimes("/tmp", sdata.times_tmp);
    (void)utimes("/", sdata.times_root);
    (void)utimes("/dev/tty", sdata.times_tty);
    (void)utimes("/dev/console", sdata.times_console);
    (void)utimes("/dev/stdin", sdata.times_stdin);
    (void)utimes("/dev/stdout", sdata.times_stdout);
    (void)utimes("/dev/stderr", sdata.times_stderr);
    (void)utimes("/dev/zero", sdata.times_zero);
    (void)utimes("/dev/null", sdata.times_null);
    (void)utimes("/bin/sh", sdata.times_sh);
    (void)utimes("/bin/ls", sdata.times_ls);
    (void)utimes("/var/log/system.log", sdata.times_system);
    (void)utimes("/var/log/messages", sdata.times_messages);
    sdata.time = time(NULL);
    sdata.size = sizeof(sdata);
    sdata.prev_hash64_copy = prev_hash64; /* load previous hash */
    sdata.call_count_copy = ++call_count; /* update call count */
#if defined(HAVE_ENVIRON)
    sdata.environ_copy = environ;
#endif
    sdata.sdata_p = (char *)&sdata;

    /*
     * seed the generator with the above sdata
     */
    hash_val = private_hash64_buf(hash_val, (char *)&sdata, sizeof(sdata));

#if defined(HAVE_ENVIRON)

    /*
     * mix in each envinment variable
     */
    for (i = 0; environ[i] != NULL; ++i) {

        /* obtain length of this next environment variable string */
        envlen = strlen(environ[i]);

        /* hash any non-zero length environment variable string */
        if (envlen > 0) {
            hash_val = private_hash64_buf(hash_val, environ[i], envlen);
        }
    }

#endif

    /*
     * mix in data from 31-bit random() and friends
     *
     * While random() returns only 31 bit values, seeded by 32 bits,
     * we use it twice: once seeded by time of day, count, prev hash
     * and once seeded by FNV hash of sdata.  Both seeds are somewhat
     * independent of each other, so we get the effect of 64 bits of seed.
     *
     * The 2nd seede uses the larger 256 byte state table to help
     * differentiate the random() returns from the 1st seed.
     *
     * Because random() returns a 31-bit number, we call it 32 times
     * and circular shift around the results within a 32 bit word
     * to mix where the missing bit, low order and high order bits land.
     */

    /* form the xor-fold of the previous hash */
#if defined(HAVE_B64)

    past_hash = (unsigned)(((prev_hash64 >> 32) & 0xffffffff) ^ (prev_hash64 & 0xffffffff));

#else
    pash_hash = (unsigned)(prev_hash64.w32[0] ^ prev_hash64.w32[1]);

#endif

    /* classic 31-bit random seeded with time of day, count, prev hash */
    srandom((unsigned)(sdata.time) ^ (unsigned)call_count ^ past_hash);
    random_before[0] = random(); /* 31-bit value */
    for (j = 1; j < RANDOM_CNT; ++j) {
        tmp = random(); /* 31-bit value */
        /* we 32-bit circular shift to spread 31-bit returns around */
        random_before[j] = (tmp << j) | (tmp >> (RANDOM_CNT - j));
    }

    /* init with large random state with 32-bit xor fold FNV hash of sdata */
#if defined(HAVE_B64)

    initstate_ret =
        initstate((unsigned)(((hash_val >> 32) & 0xffffffff) ^ (hash_val & 0xffffffff)), initstate_tbl, INITSTATE_SIZE);

#else
    initstate_ret = initstate((unsigned)(hash_val.w32[0] ^ hash_val.w32[1]), initstate_tbl, INITSTATE_SIZE);

#endif

    /* use 31-bit random some more with the new random state */
    random_after[0] = random(); /* 31-bit value */
    for (j = 1; j < RANDOM_CNT; ++j) {
        tmp = random(); /* 31-bit value */
        /* we 32-bit circular shift to spread 31-bit returns around */
        random_after[j] = (tmp << j) | (tmp >> (RANDOM_CNT - j));
    }

    /* restore standard table sized previous state */
    setstate_ret = setstate(initstate_ret);

    /*
     * hash all the data from random() and friends
     */
    hash_val = private_hash64_buf(hash_val, (char *)random_before, sizeof(random_before));
    hash_val = private_hash64_buf(hash_val, (char *)initstate_ret, sizeof(initstate_ret));
    hash_val = private_hash64_buf(hash_val, (char *)initstate_tbl, sizeof(initstate_tbl));
    hash_val = private_hash64_buf(hash_val, (char *)random_after, sizeof(random_after));
    hash_val = private_hash64_buf(hash_val, (char *)setstate_ret, sizeof(setstate_ret));

#if defined(HAVE_ARC4RANDOM)

    /*
     * hash from a cryptographic pseudo-random number generator
     */
    arc4random_buf(arc4_buf, ARC4_BUFLEN);
    hash_val = private_hash64_buf(hash_val, (char *)arc4_buf, ARC4_BUFLEN);

#endif

    /*
     * load the hash data into the ZVALUE
     *
     * We do not care about byte-order, nor Endian issues, we just
     * want to load in data.  We round up to the next HALF in size
     * just in case hash_val is not a HALF multiple in length.
     */
    hash.len = (sizeof(hash_val) + sizeof(HALF) - 1) / sizeof(HALF);
    hash.v = alloc(hash.len);
    memset(hash.v, 0, hash.len * sizeof(HALF)); /* paranoia */
    hash.sign = 0;
    memcpy((void *)hash.v, (void *)&hash_val, sizeof(hash_val));

    /*
     * Force the hash as ZVALUE to be at most, 64 bits long.
     * It is almost certainly the case that the hash as ZVALUE
     * is at most 64 bits in length: this code guarantees it.
     *
     * BTW: One can safely assume that 64 is an integer multiple of BASEB:
     *      likely 4, 2, or 1 times BASEB.
     */
    if (hash.len > 64 / BASEB) {
        hash.len = 64 / BASEB;
    }
    ztrim(&hash);

    /*
     * save hash value for next call
     */
    prev_hash64 = hash_val;

    /*
     * return a number
     */
    ret = qalloc();
    ret->num = hash;
    return ret;
}
