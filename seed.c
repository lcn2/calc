/*
 * seed - produce a pseudo-random seeds
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
#else
struct s_hash64 {
	USB32 w32[2];
};
typedef struct s_hash64 hash64;
#endif


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
 * that will result in a zero hash value.  Worse, appending any
 * about of zero bytes will continue to produce a zero hash value.
 * But that would happen with any initial value so long as the
 * hash of the initial was the `inverse' of the virgin prefix string.
 *
 * But then again for any hash function, there exists sets of data
 * which that the hash of every member is the same value.  That is
 * life with many to few mapping functions.  All we do here is to
 * prevent sets whose members consist of 0 or more bytes of 0's from
 * being such an awkward set.
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
 * prvate_hash64_buf - perform a Fowler/Noll/Vo-1 64 bit hash
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
prvate_hash64_buf(char *buf, unsigned len)
{
	hash64 hval;		/* current hash value */
#if !defined(HAVE_B64)
	USB32 val[4];		/* hash value in base 2^16 */
	USB32 tmp[4];		/* tmp 64 bit value */
#endif /* HAVE_B64 */
	char *buf_end = buf+len;	/* beyond end of hash area */

	/*
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
	 */
#if defined(HAVE_B64)
	/* hash each octet of the buffer */
	for (hval = PRIVATE_64_BASIS; buf < buf_end; ++buf) {

	    /* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	    hval *= (hash64)1099511628211ULL;

	    /* xor the bottom with the current octet */
	    hval ^= (hash64)(*buf);
	}

#else /* HAVE_B64 */

	/* hash each octet of the buffer */
	val[0] = PRIVATE_64_BASIS_0;
	val[1] = PRIVATE_64_BASIS_1;
	val[2] = PRIVATE_64_BASIS_2;
	val[3] = PRIVATE_64_BASIS_3;
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
	    tmp[2] = val[2] * 0x1b3;
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
    struct {			/* data used for quasi-random seed */
#if defined(HAVE_GETTIME)
# if defined(CLOCK_SGI_CYCLE)
	struct timespec sgi_cycle;	/* SGI hardware clock */
# endif
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
	struct stat fstat_stdin;	/* stat of stdin */
	struct stat fstat_stdout;	/* stat of stdout */
	struct stat fstat_stderr;	/* stat of stderr */
#if defined(HAVE_USTAT)
	struct ustat ustat_dot;		/* usage stat of "." */
	struct ustat ustat_dotdot;	/* usage stat of ".." */
	struct ustat ustat_tmp;		/* usage stat of "/tmp" */
	struct ustat ustat_root;	/* usage stat of "/" */
	struct ustat ustat_stdin;	/* usage stat of stdin */
	struct ustat ustat_stdout;	/* usage stat of stdout */
	struct ustat ustat_stderr;	/* usage stat of stderr */
#endif
#if defined(HAVE_GETSID)
	pid_t getsid;			/* session ID */
#endif
#if defined(HAVE_GETPGID)
	pid_t getpgid;			/* process group ID */
#endif
#if defined(HAVE_GETRUSAGE)
	struct rusage rusage;		/* resource utilization */
	struct rusage rusage_chld;	/* resource utilization of children */
#endif
#if defined(HAVE_SYS_TIME_H)
	struct timeval tp2;		/* time of day again */
	struct tms times;		/* process times */
#endif
	time_t time;			/* local time */
	size_t size;			/* size of this data structure */
	jmp_buf env;			/* setjmp() context */
	char *sdata_p;			/* address of this structure */
    } sdata;
    hash64 hash_val;			/* fnv64 hash of sdata */
    ZVALUE hash;			/* hash_val as a ZVALUE */
    NUMBER *ret;			/* return seed as a NUMBER */

    /*
     * pick up process/system information
     *
     * NOTE:
     *	  We do care (that much) if these calls fail.  We do not
     *	  need to process any data in the 'sdata' structure.
     */
#if defined(HAVE_GETTIME)
# if defined(CLOCK_SGI_CYCLE)
    (void) clock_gettime(CLOCK_SGI_CYCLE, &sdata.sgi_cycle);
# endif
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
    (void) fstat(0, &sdata.fstat_stdin);
    (void) fstat(1, &sdata.fstat_stdout);
    (void) fstat(2, &sdata.fstat_stderr);
#if defined(HAVE_USTAT)
    (void) ustat(sdata.stat_dotdot.st_dev, &sdata.ustat_dotdot);
    (void) ustat(sdata.stat_dot.st_dev, &sdata.ustat_dot);
    (void) ustat(sdata.stat_tmp.st_dev, &sdata.ustat_tmp);
    (void) ustat(sdata.stat_root.st_dev, &sdata.ustat_root);
    (void) ustat(sdata.fstat_stdin.st_dev, &sdata.ustat_stdin);
    (void) ustat(sdata.fstat_stdout.st_dev, &sdata.ustat_stdout);
    (void) ustat(sdata.fstat_stderr.st_dev, &sdata.ustat_stderr);
#endif
#if defined(HAVE_GETSID)
    sdata.getsid = getsid((pid_t)0);
#endif
#if defined(HAVE_GETPGID)
    sdata.getpgid = getpgid((pid_t)0);
#endif
#if defined(HAVE_GETRUSAGE)
    (void) getrusage(RUSAGE_SELF, &sdata.rusage);
    (void) getrusage(RUSAGE_CHILDREN, &sdata.rusage_chld);
#endif
#if defined(HAVE_SYS_TIME_H)
    (void) gettimeofday(&sdata.tp2, NULL);
    (void) times(&sdata.times);
#endif
    sdata.time = time(NULL);
    sdata.size = sizeof(sdata);
    (void) setjmp(sdata.env);
    sdata.sdata_p = (char *)&sdata;

    /*
     * seed the generator with the above data
     */
    hash_val = prvate_hash64_buf((char *)&sdata, sizeof(sdata));

    /*
     * load the hash data into the ZVALUE
     *
     * We do not care about byte-order or endian issues, we just
     * want to load in data.
     */
    hash.len = sizeof(hash_val) / sizeof(HALF);
    hash.v = alloc(hash.len);
    hash.sign = 0;
    memcpy((void *)hash.v, (void *)&hash_val, hash.len*sizeof(HALF));
    ztrim(&hash);

    /*
     * return a number
     */
    ret = qalloc();
    ret->num = hash;
    return ret;
}
