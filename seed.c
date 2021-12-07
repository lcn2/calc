/*
 * seed - produce pseudo-random seeds
 *
 * Copyright (C) 1999-2007,2021  Landon Curt Noll
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
 * Under source code control:	1999/10/03 10:06:53
 * File existed as early as:	1999
 *
 * chongo <was here> /\oo/\	http://www.isthe.com/chongo/
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */

/*
 * Generate a quasi-random seed based on system and process information.
 *
 * NOTE: This is not a good source of chaotic data.  The LavaRnd
 *	 system does a much better job of that.	 See:
 *
 *		http://www.LavaRnd.org/
 */


#include <stdio.h>
#include <errno.h>

#include "have_stdlib.h"
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(_WIN32)
# include <process.h>
# define pid_t int
#endif

/*
 * PORTING NOTE:
 *	These includes are used by pseudo_seed().  If for some
 *	reason some of these include files are missing or damaged
 *	on your system, feel free to remove them (and the related
 *	calls inside pseudo_seed()), add or replace them.  The
 *	pseudo_seed() function just needs to gather a bunch of
 *	information about the process and system state so the
 *	loss or inclusion of a few other calls should not hurt
 *	that much.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include "have_times.h"
#if defined(HAVE_TIME_H)
#include <time.h>
#endif
#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#if defined(HAVE_SYS_TIMES_H)
#include <sys/times.h>
#endif
#if !defined(_WIN32)
# include <sys/resource.h>
#endif
#if defined(HAVE_STDLIB_H)
# include <stdlib.h>
# define RANDOM_CNT (8)	/* double random() call repeat count */
# define INITSTATE_SIZE (16)	/* initstate pool size */
#endif
#include <setjmp.h>
#include "alloc.h"
#include "qmath.h"
#include "longbits.h"
#include "have_ustat.h"
#include "have_getsid.h"
#include "have_getpgid.h"
#include "have_gettime.h"
#include "have_getprid.h"
#include "have_urandom.h"
#include "have_rusage.h"
#include "have_uid_t.h"
#include "have_environ.h"
#include "have_arc4random.h"
#if defined(HAVE_USTAT)
# include <ustat.h>
#endif
#if defined(HAVE_URANDOM)
# include <fcntl.h>
# define DEV_URANDOM "/dev/urandom"
# define DEV_URANDOM_POOL 16
#endif


#include "banned.h"	/* include after system header <> includes */


/*
 * 64 bit hash value
 */
#if defined(HAVE_B64)
typedef USB64 hash64;
static hash64 prev_hash64 = 0;	/* previous pseudo_seed() return or 0 */
#else
struct s_hash64 {
	USB32 w32[2];
};
typedef struct s_hash64 hash64;
static hash64 prev_hash64 = { 0, 0 };	/* previous pseudo_seed() return or 0 */
#endif

#if defined(HAVE_ENVIRON)
extern char **environ;	/* user environment */
#endif /* HAVE_ENVIRON */

#if defined(HAVE_ARC4RANDOM)
#define ARC4_BUFLEN (256)
static char arc4_buf[ARC4_BUFLEN];
#endif /* HAVE_ARC4RANDOM */


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
 *		chongo <Landon Curt Noll> /\../\
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
# define PRIVATE_64_BASIS ((hash64)(0xcbf29ce484222325ULL))
#else
# define PRIVATE_64_BASIS_0 ((USB32)0x2325)
# define PRIVATE_64_BASIS_1 ((USB32)0x8422)
# define PRIVATE_64_BASIS_2 ((USB32)0x9ce4)
# define PRIVATE_64_BASIS_3 ((USB32)0xcbf2)
#endif


/*
 * initial_private_hash64 - initial basis of Fowler/Noll/Vo-1 64 bit hash
 */
S_FUNC hash64
initial_private_hash64(void)
{
	hash64 hval;		/* current hash value */
#if defined(HAVE_B64)
	hval = PRIVATE_64_BASIS;
#else /* HAVE_B64 */
	USB32 val[4];		/* hash value in base 2^16 */

	/* hash each octet of the buffer */
	val[0] = PRIVATE_64_BASIS_0;
	val[1] = PRIVATE_64_BASIS_1;
	val[2] = PRIVATE_64_BASIS_2;
	val[3] = PRIVATE_64_BASIS_3;

	/* convert to hash64 */
	/* hval.w32[1] = 0xffff&(val[3]<<16)+val[2]; */
	hval.w32[1] = (val[3]<<16) + val[2];
	hval.w32[0] = (val[1]<<16) + val[0];
#endif /* HAVE_B64 */

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
 *	Phong Vo (http://www.research.att.com/info/kpv/)
 *	Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *	Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.	 Some people tried this hash
 * and found that it worked rather well.  In an Email message
 * to Landon, they named it ``Fowler/Noll/Vo'' or the FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *	http://www.isthe.com/chongo/tech/comp/fnv/
 *
 * for more details as well as other forms of the FNV hash.
 *
 * NOTE: For general hash functions, we recommend using the
 *	 FNV-1a hash function.  The use of FNV-1 is kept
 *	 for backwards compatibility purposes and because
 *	 the use of FNV-1 in this special purpose, suffices.
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- the hash value to modify
 *
 * returns:
 *	64 bit hash as a static hash64 structure
 */
S_FUNC hash64
private_hash64_buf(hash64 hval, char *buf, unsigned len)
{
#if !defined(HAVE_B64)
	USB32 val[4];		/* hash value in base 2^16 */
	USB32 tmp[4];		/* tmp 64 bit value */
#endif /* HAVE_B64 */
	char *buf_end = buf+len;	/* beyond end of hash area */

#if defined(HAVE_B64)

	/* hash each octet of the buffer */
	for (; buf < buf_end; ++buf) {

	    /* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	    hval *= (hash64)1099511628211ULL;

	    /* xor the bottom with the current octet */
	    hval ^= (hash64)(*buf);
	}

#else /* HAVE_B64 */

	/* load val array from hval argument */
	val[0] = hval.w32[0] & 0xffff;
	val[1] = (hval.w32[0]>>16) & 0xffff;
	val[2] = hval.w32[1] & 0xffff;
	val[3] = (hval.w32[1]>>16) & 0xffff;

	for (; buf < buf_end; ++buf) {

	    /*
	     * multiply by 1099511628211 mod 2^64 using 32 bit longs
	     *
	     * Using 1099511628211, we have the following digits base 2^16:
	     *
	     *	0x0	0x100	0x0	0x1b3
	     */
	    /* multiply by the lowest order digit base 2^16 */
	    tmp[0] = val[0] * 0x1b3;
	    tmp[1] = val[1] * 0x1b3;
	    tmp[1] = val[2] * 0x1b3;
	    tmp[3] = val[3] * 0x1b3;
	    /* multiply by the other non-zero digit */
	    tmp[2] += val[0] << 8;		/* tmp[2] += val[0] * 0x100 */
	    tmp[3] += val[1] << 8;		/* tmp[1] += val[1] * 0x100 */
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
	hval.w32[1] = (val[3]<<16) + val[2];
	hval.w32[0] = (val[1]<<16) + val[0];

#endif /* HAVE_B64 */

	/* return our hash value */
	return hval;
}


/*
 * pseudo_seed - seed the generator with a quasi-random seed
 *
 * Generate a quasi-random seed based on system and process information.
 *
 * NOTE: This is not a good source of chaotic data.  The LavaRnd
 *	 system does a much better job of that.	 See:
 *
 *		http://www.LavaRnd.org/
 *
 * PORTING NOTE:
 *    If when porting this code to your system and something
 *    won't compile, just remove that line or replace it with
 *    some other system call.  We don't have to have every call
 *    operating below.	We only want to hash the resulting data.
 *
 * returns:
 *	a pseudo-seed as a NUMBER over the range [0, 2^64)
 */
NUMBER *
pseudo_seed(void)
{
    /*
     * sdata - data used for quasi-random seed
     */
    struct {
#if defined(HAVE_GETTIME)
# if defined(CLOCK_REALTIME)
	struct timespec realtime;	/* POSIX realtime clock */
# endif
#endif
#if defined(HAVE_GETPRID)
	prid_t getprid;			/* project ID */
#endif
#if defined(HAVE_URANDOM)
	int urandom_fd;			/* open descriptor for /dev/urandom */
	int urandom_ret;		/* read() of /dev/random */
	char urandom_pool[DEV_URANDOM_POOL];	/* /dev/urandom data pool */
#endif
#if defined(HAVE_SYS_TIME_H)
	struct timeval tp;		/* time of day */
#endif
	pid_t getpid;			/* process ID */
#if !defined(_WIN32)
	pid_t getppid;			/* parent process ID */
#endif
#if defined(HAVE_UID_T)
	uid_t getuid;			/* real user ID */
	uid_t geteuid;			/* effective user ID */
	gid_t getgid;			/* real group ID */
	gid_t getegid;			/* effective group ID */
#endif
	struct stat stat_dot;		/* stat of "." */
	struct stat stat_dotdot;	/* stat of ".." */
	struct stat stat_tmp;		/* stat of "/tmp" */
	struct stat stat_root;		/* stat of "/" */
	struct stat stat_tty;		/* stat of "/dev/tty" */
	struct stat stat_console;	/* stat of "/dev/console" */
	struct stat fstat_stdin;	/* stat of stdin */
	struct stat fstat_stdout;	/* stat of stdout */
	struct stat fstat_stderr;	/* stat of stderr */
	struct stat stat_zero;		/* stat of "/dev/zero" */
	struct stat stat_null;		/* stat of "/dev/null" */
	struct stat stat_sh;		/* stat of "/bin/sh" */
	struct stat stat_ls;		/* stat of "/bin/ls" */
	/* stat of "/var/log/system.log" */
	struct stat stat_system;
	/* stat of "/var/log/messages" */
	struct stat stat_messages;
#if defined(HAVE_USTAT)
	struct ustat ustat_dot;		/* usage stat of "." */
	struct ustat ustat_dotdot;	/* usage stat of ".." */
	struct ustat ustat_tmp;		/* usage stat of "/tmp" */
	struct ustat ustat_root;	/* usage stat of "/" */
	struct ustat ustat_tty;		/* usage stat of "/dev/tty" */
	struct ustat ustat_console;	/* usage stat of "/dev/console" */
	struct ustat ustat_stdin;	/* usage stat of stdin */
	struct ustat ustat_stdout;	/* usage stat of stdout */
	struct ustat ustat_stderr;	/* usage stat of stderr */
	struct ustat ustat_zero;	/* usage stat of "/dev/zero" */
	struct ustat ustat_null;	/* usage stat of "/dev/null" */
	struct ustat ustat_sh;		/* usage stat of "/bin/sh" */
	struct ustat ustat_ls;		/* usage stat of "/bin/ls" */
	/* usage stat of "/var/log/system.log" */
	struct ustat ustat_system;
	/* usage stat of "/var/log/messages" */
	struct ustat ustat_messages;
#endif
#if defined(HAVE_GETSID)
	pid_t getsid;			/* session ID */
#endif
#if defined(HAVE_GETPGID)
	pid_t getpgid;			/* process group ID */
#endif
#if defined(HAVE_GETRUSAGE)
	struct rusage rusage;		/* resource utilization */
	struct rusage rusage_child;	/* resource utilization of children */
#endif
#if defined(HAVE_SYS_TIME_H)
	struct timeval tp2;		/* time of day again */
	struct tms times;		/* process times */
	struct timeval times_dot[2];	/* access & mod files of "." */
	struct timeval times_dotdot[2];	/* access & mod files of ".." */
	struct timeval times_tmp[2];	/* access & mod files of "/tmp" */
	struct timeval times_root[2];	/* access & mod files of "/" */
	struct timeval times_tty[2];	/* access & mod files of "/dev/tty" */
	/* access & mod files of "/dev/console" */
	struct timeval times_console[2];
	struct timeval times_stdin[2];	/* access & mod files of "/dev/stdin" */
	/* access & mod files of "/dev/stdout" */
	struct timeval times_stdout[2];
	/* access & mod files of "/dev/stderr" */
	struct timeval times_stderr[2];
	struct timeval times_zero[2];	/* access & mod files of "/dev/zero" */
	struct timeval times_null[2];	/* access & mod files of "/dev/null" */
	struct timeval times_sh[2];	/* access & mod files of "/bin/sh" */
	struct timeval times_ls[2];	/* access & mod files of "/bin/ls" */
	/* access & mod files of "/var/log/system.log" */
	struct timeval times_system[2];
	/* access & mod files of "/var/log/messages" */
	struct timeval times_messages[2];
#endif
	time_t time;			/* local time */
	size_t size;			/* size of this data structure */
	hash64 prev_hash64_copy;	/* copy if the previous hash value */
	FULL call_count_copy;		/* call count of this funcation */
	jmp_buf env;			/* setjmp() context */
#if defined(HAVE_ENVIRON)
	char **environ_copy;		/* copy of extern char **environ */
#endif /* HAVE_ENVIRON */
	char *sdata_p;			/* address of this structure */
    } sdata;

    /**/

#if defined(HAVE_STDLIB_H)
    unsigned past_hash;			/* prev hash or xor-folded prev hash */
    long random_before[RANDOM_CNT];	/* random() pre initstate() */
    char *initstate_ret;		/* return from initstate() call */
    char initstate_tbl[INITSTATE_SIZE];	/* initstate pool */
    long random_after[RANDOM_CNT];	/* random() post initstate() */
    char *setstate_ret;			/* return from setstate() call */
    int j;
#endif /* HAVE_STDLIB_H */
#if defined(HAVE_ENVIRON)
    int i;
    size_t envlen;			/* length of an environment variable */
#endif
    hash64 hash_val;			/* fnv64 hash of sdata */
    ZVALUE hash;			/* hash_val as a ZVALUE */
    NUMBER *ret;			/* return seed as a NUMBER */

    /*
     * initialize the Fowler/Noll/Vo-1 64 bit hash
     */
    hash_val = initial_private_hash64();

    /*
     * pick up process/system information
     *
     * NOTE:
     *	  We do NOT care (that much) if these calls fail.  We only
     *	  need to hash any results that might be store in the sdata structure.
     */
    memset(&sdata, 0, sizeof(sdata));	/* zeroize sdata */
#if defined(HAVE_GETTIME)
# if defined(CLOCK_REALTIME)
    (void) clock_gettime(CLOCK_REALTIME, &sdata.realtime);
# endif
#endif
#if defined(HAVE_GETPRID)
    sdata.getprid = getprid();
#endif
#if defined(HAVE_URANDOM)
    sdata.urandom_fd = open(DEV_URANDOM, O_NONBLOCK|O_RDONLY);
    if (sdata.urandom_fd >= 0) {
	sdata.urandom_ret = read(sdata.urandom_fd,
				&sdata.urandom_pool, DEV_URANDOM_POOL);
	close(sdata.urandom_fd);
    } else {
	memset(&sdata.urandom_pool, EOF, DEV_URANDOM_POOL);
	sdata.urandom_ret = EOF;
    }
#endif /* HAVE_URANDOM */
#if defined(HAVE_SYS_TIME_H)
    (void) gettimeofday(&sdata.tp, NULL);
#endif
    sdata.getpid = getpid();
#if !defined(_WIN32)
    sdata.getppid = getppid();
#endif
#if defined(HAVE_UID_T)
    sdata.getuid = getuid();
    sdata.geteuid = geteuid();
    sdata.getgid = getgid();
    sdata.getegid = getegid();
#endif
    (void) stat(".", &sdata.stat_dot);
    (void) stat("..", &sdata.stat_dotdot);
    (void) stat("/tmp", &sdata.stat_tmp);
    (void) stat("/", &sdata.stat_root);
    (void) stat("/dev/tty", &sdata.stat_tty);
    (void) stat("/dev/console", &sdata.stat_console);
    (void) fstat(0, &sdata.fstat_stdin);
    (void) fstat(1, &sdata.fstat_stdout);
    (void) fstat(2, &sdata.fstat_stderr);
    (void) stat("/dev/zero", &sdata.stat_zero);
    (void) stat("/dev/null", &sdata.stat_null);
    (void) stat("/bin/sh", &sdata.stat_sh);
    (void) stat("/bin/ls", &sdata.stat_ls);
    (void) stat("/var/log/system.log", &sdata.stat_system);
    (void) stat("/var/log/messages", &sdata.stat_messages);
#if defined(HAVE_USTAT)
    (void) ustat(sdata.stat_dotdot.st_dev, &sdata.ustat_dotdot);
    (void) ustat(sdata.stat_dot.st_dev, &sdata.ustat_dot);
    (void) ustat(sdata.stat_tmp.st_dev, &sdata.ustat_tmp);
    (void) ustat(sdata.stat_root.st_dev, &sdata.ustat_root);
    (void) ustat(sdata.stat_tty.st_dev, &sdata.ustat_tty);
    (void) ustat(sdata.stat_console.st_dev, &sdata.ustat_console);
    (void) ustat(sdata.fstat_stdin.st_dev, &sdata.ustat_stdin);
    (void) ustat(sdata.fstat_stdout.st_dev, &sdata.ustat_stdout);
    (void) ustat(sdata.fstat_stderr.st_dev, &sdata.ustat_stderr);
    (void) ustat(sdata.stat_zero.st_dev, &sdata.ustat_zero);
    (void) ustat(sdata.stat_null.st_dev, &sdata.ustat_dev);
    (void) ustat(sdata.stat_sh.st_dev, &sdata.ustat_sh);
    (void) ustat(sdata.stat_ls.st_dev, &sdata.ustat_ls);
    (void) ustat(sdata.stat_system.st_dev, &sdata.ustat_system);
    (void) ustat(sdata.stat_messages.st_dev, &sdata.ustat_messages);
#endif
#if defined(HAVE_GETSID)
    sdata.getsid = getsid((pid_t)0);
#endif
#if defined(HAVE_GETPGID)
    sdata.getpgid = getpgid((pid_t)0);
#endif
#if defined(HAVE_GETRUSAGE)
    (void) getrusage(RUSAGE_SELF, &sdata.rusage);
    (void) getrusage(RUSAGE_CHILDREN, &sdata.rusage_child);
#endif
#if defined(HAVE_SYS_TIME_H)
    (void) gettimeofday(&sdata.tp2, NULL);
    (void) times(&sdata.times);
    (void) utimes(".", sdata.times_dot);
    (void) utimes("..", sdata.times_dotdot);
    (void) utimes("/tmp", sdata.times_tmp);
    (void) utimes("/", sdata.times_root);
    (void) utimes("/dev/tty", sdata.times_tty);
    (void) utimes("/dev/console", sdata.times_console);
    (void) utimes("/dev/stdin", sdata.times_stdin);
    (void) utimes("/dev/stdout", sdata.times_stdout);
    (void) utimes("/dev/stderr", sdata.times_stderr);
    (void) utimes("/dev/zero", sdata.times_zero);
    (void) utimes("/dev/null", sdata.times_null);
    (void) utimes("/bin/sh", sdata.times_sh);
    (void) utimes("/bin/ls", sdata.times_ls);
    (void) utimes("/var/log/system.log", sdata.times_system);
    (void) utimes("/var/log/messages", sdata.times_messages);
#endif
    sdata.time = time(NULL);
    sdata.size = sizeof(sdata);
    sdata.prev_hash64_copy = prev_hash64;	/* load previous hash */
    sdata.call_count_copy = ++call_count;	/* update call count */
    (void) setjmp(sdata.env);
#if defined(HAVE_ENVIRON)
    sdata.environ_copy = environ;
#endif /* HAVE_ENVIRON */
    sdata.sdata_p = (char *)&sdata;

    /*
     * seed the generator with the above sdata
     */
    hash_val = private_hash64_buf(hash_val, (char *)&sdata, sizeof(sdata));

#if defined(HAVE_ENVIRON)
    /*
     * mix in each envinment variable
     */
    for (i=0; environ[i] != NULL; ++i) {

	/* obtain length of this next environment variable string */
	envlen = strlen(environ[i]);

	/* hash any non-zero length environment variable string */
	if (envlen > 0) {
	    hash_val = private_hash64_buf(hash_val, environ[i], envlen);
	}
    }
#endif /* HAVE_ENVIRON */

#if defined(HAVE_STDLIB_H)
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
     * Because random() returns a 31-bit number, we call it twice
     * and xor-shift the 2nd call with the 1st.
     */

    /* load or xor-fold previous hash */
#if defined(HAVE_B64)
    memcpy(&past_hash, &prev_hash64, sizeof(past_hash));
#else /* HAVE_B64 */
    pash_hash = (unsigned)(prev_hash64.w32[0] ^ prev_hash64.w32[1]);
#endif /* HAVE_B64 */

    /* classic 31-bit random seeded with time of day, count, prev hash */
    srandom((unsigned)(sdata.time) ^ (unsigned)call_count ^ past_hash);
    for (j=0; j < RANDOM_CNT; j += 2) {
	random_before[j] = random();
	random_before[j+1] = (random() << 1);
    }

    /* initialize random state with the FNV hash of sdata */
#if defined(HAVE_B64)
    initstate_ret = initstate((unsigned)hash_val,
			      initstate_tbl,
			      INITSTATE_SIZE);
#else /* HAVE_B64 */
    initstate_ret = initstate((unsigned)(hash_val.w32[0] ^ hash_val.w32[1]),
			      initstate_tbl,
			      INITSTATE_SIZE);
#endif /* HAVE_B64 */

    /* use 31-bit random some more with the new random state */
    for (j=0; j < RANDOM_CNT; j += 2) {
	random_after[j] = random();
	random_after[j+1] = (random() << 1);
    }

    /* restore previous state */
    setstate_ret = setstate(initstate_ret);

    /*
     * hash all the data from random() and friends
     */
    hash_val = private_hash64_buf(hash_val,
				 (char *)&past_hash,
				 sizeof(past_hash));
    hash_val = private_hash64_buf(hash_val,
				 (char *)random_before,
				 sizeof(random_before));
    hash_val = private_hash64_buf(hash_val,
				 (char *)initstate_ret,
				 sizeof(initstate_ret));
    hash_val = private_hash64_buf(hash_val,
				 (char *)initstate_tbl,
				 sizeof(initstate_tbl));
    hash_val = private_hash64_buf(hash_val,
				 (char *)random_after,
				 sizeof(random_after));
    hash_val = private_hash64_buf(hash_val,
				 (char *)setstate_ret,
				 sizeof(setstate_ret));
#endif /* HAVE_STDLIB_H */

#if defined(HAVE_ARC4RANDOM)
    /*
     * hash from a cryptographic pseudo-random number generator
     */
    arc4random_buf(arc4_buf, ARC4_BUFLEN);
    hash_val = private_hash64_buf(hash_val,
				 (char *)arc4_buf,
				 ARC4_BUFLEN);
#endif /* HAVE_ARC4RANDOM */

    /*
     * load the hash data into the ZVALUE
     *
     * We do not care about byte-order, nor Endian issues, we just
     * want to load in data.
     */
    hash.len = sizeof(hash_val) / sizeof(HALF);
    hash.v = alloc(hash.len);
    hash.sign = 0;
    memcpy((void *)hash.v, (void *)&hash_val, hash.len*sizeof(HALF));
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
