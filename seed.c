/*
 * seed - produce a pseudo-random seeds
 *
 * Generate a quasi-random seed based on system and process information.
 *
 * NOTE: This is not a good source of chaotic data.  The lavarand
 *	 system does a much better job of that.  See:
 *
 *		http://lavarand.sgi.com
 *
 * Copyright (c) 1999 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *      supporting documentation
 *      source copies
 *      source works derived from this source
 *      binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo was here      /\../\		{chongo,noll}@{toad,sgi}.com
 *
 * Share and enjoy! :-)
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

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
#include <sys/resource.h>
#include <setjmp.h>
#if !defined(__bsdi)
#include <ustat.h>
#endif /* __bsdi */
#if defined(__linux)
# include <fcntl.h>
# define DEV_URANDOM "/dev/urandom"
# define DEV_URANDOM_POOL 128
#endif /* __linux */
#include "qmath.h"
#include "longbits.h"


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
 * hash_buf - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- the hash value to modify
 *
 * returns:
 *	64 bit hash as a static hash64 structure
 */
static hash64
hash_buf(char *buf, unsigned len)
{
	hash64 hval;		/* current hash value */
#if !defined(HAVE_B64)
	USB32 val[4];		/* hash value in base 2^16 */
	USB32 tmp[4];		/* tmp 64 bit value */
#endif /* HAVE_B64 */
	char *buf_end = buf+len;	/* beyond end of hash area */

	/*
	 * Fowler/Noll/Vo hash - hash each character in the string
	 *
	 * The basis of the hash algorithm was taken from an idea
	 * sent by Email to the IEEE POSIX P1003.2 mailing list from
	 * Phong Vo (kpv@research.att.com) and Glenn Fowler
	 * (gsf@research.att.com).
	 *
	 * See:
	 *	http://reality.sgi.com/chongo/src/fnv/fnv_hash.tar.gz
	 *	http://reality.sgi.com/chongo/src/fnv/h32.c
	 *	http://reality.sgi.com/chongo/src/fnv/h64.c
	 *
	 * for information on 32bit and 64bit Fowler/Noll/Vo hashes.
	 *
	 * Landon Curt Noll (chongo@toad.com) later improved on their
	 * algorithm to come up with Fowler/Noll/Vo hash.
	 *
	 * The 32 hash was able to process 234936 words from the web2 dictionary
	 * without any 32 bit collisions using a constant of
	 * 16777619 = 0x1000193.
	 *
	 * The 64 bit hash uses 1099511628211 = 0x100000001b3 instead.
	 */
#if defined(HAVE_B64)
	/* hash each octet of the buffer */
	for (hval = (hash64)0ULL; buf < buf_end; ++buf) {

	    /* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	    hval *= (hash64)1099511628211ULL;

	    /* xor the bottom with the current octet */
	    hval ^= (hash64)(*buf);
	}

#else /* HAVE_B64 */

	/* hash each octet of the buffer */
	for (val[0]=val[1]=val[2]=val[3]=0; buf < buf_end; ++buf) {

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
	    /* proapage carries */
	    tmp[1] += (tmp[0] >> 16);
	    val[0] = tmp[0] & 0xffff;
	    tmp[2] += (tmp[1] >> 16);
	    val[1] = tmp[1] & 0xffff;
	    val[3] += (tmp[2] >> 16);
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
	hval.w32[1] = val[3]<<16 + val[2];
	hval.w32[0] = val[1]<<16 + val[0];

#endif /* HAVE_B64 */

	/* return our hash value */
	return hval;
}


/*
 * pseudo_seed - seed the generator with a quasi-random seed
 *
 * Generate a quasi-random seed based on system and process information.
 *
 * NOTE: This is not a good source of chaotic data.  The lavarand
 *	 system does a much better job of that.  See:
 *
 *		http://lavarand.sgi.com
 *
 * PORTING NOTE:
 *    If when porting this code to your system and something
 *    won't compile, just remove that line or replace it with
 *    some other system call.  We don't have to have every call
 *    operating below.  We only want to hash the resulting data.
 *
 * returns:
 *	a pseudo-seed as a NUMBER over the range [0, 2^64)
 */
NUMBER *
pseudo_seed(void)
{
    struct {			/* data used for quasi-random seed */
#if defined(__sgi)
	struct timespec sgi_cycle;      /* SGI hardware clock */
	prid_t getprid;                 /* project ID */
	struct timespec realtime;	/* POSIX realtime clock */
#endif /* __sgi */
#if defined(__linux)
	int urandom_fd;			/* open scriptor for /dev/urandom */
	int urandom_ret;		/* read() of /dev/random */
	char urandom_pool[DEV_URANDOM_POOL];	/* /dev/urandom data pool */
#endif /* __linux */
	struct timeval tp;              /* time of day */
	pid_t getpid;                   /* process ID */
	pid_t getppid;                  /* parent process ID */
	uid_t getuid;                   /* real user ID */
	uid_t geteuid;                  /* effective user ID */
	gid_t getgid;                   /* real group ID */
	gid_t getegid;                  /* effective group ID */
	struct stat stat_dot;           /* stat of "." */
	struct stat stat_dotdot;        /* stat of ".." */
	struct stat stat_tmp;           /* stat of "/tmp" */
	struct stat stat_root;		/* stat of "/" */
	struct stat fstat_stdin;        /* stat of stdin */
	struct stat fstat_stdout;       /* stat of stdout */
	struct stat fstat_stderr;       /* stat of stderr */
#if !defined(__bsdi)
	struct ustat ustat_dot;         /* usage stat of "." */
	struct ustat ustat_dotdot;      /* usage stat of ".." */
	struct ustat ustat_tmp;         /* usage stat of "/tmp" */
	struct ustat ustat_root;        /* usage stat of "/" */
	struct ustat ustat_stdin;       /* usage stat of stdin */
	struct ustat ustat_stdout;      /* usage stat of stdout */
	struct ustat ustat_stderr;      /* usage stat of stderr */
	pid_t getsid;                   /* session ID */
	pid_t getpgid;                  /* process group ID */
#endif /* __bsdi */
	struct rusage rusage;           /* resource utilization */
	struct rusage rusage_chld;      /* resource utilization of children */
	struct timeval tp2;             /* time of day again */
	size_t size;			/* size of this data structure */
	jmp_buf env;			/* setjmp() context */
	char *sdata_p;                  /* address of this structure */
    } sdata;
    hash64 hash_val;			/* fnv64 hash of sdata */
    ZVALUE hash;			/* hash_val as a ZVALUE */
    NUMBER *ret;			/* return seed as a NUMBER */

    /*
     * pick up process/system information
     *
     * NOTE:
     *    We do care (that much) if these calls fail.  We do not
     *	  need to process any data in the 'sdata' structure.
     */
#if defined(__sgi)
    (void) clock_gettime(CLOCK_SGI_CYCLE, &sdata.sgi_cycle);
    sdata.getprid = getprid();
    (void) clock_gettime(CLOCK_REALTIME, &sdata.realtime);
#endif /* __sgi */
#if defined(__linux)
    sdata.urandom_fd = open(DEV_URANDOM, O_NONBLOCK|O_RDONLY);
    if (sdata.urandom_fd >= 0) {
	sdata.urandom_ret = read(sdata.urandom_fd,
				&sdata.urandom_pool, DEV_URANDOM_POOL);
	close(sdata.urandom_fd);
    } else {
	memset(&sdata.urandom_pool, EOF, DEV_URANDOM_POOL);
	sdata.urandom_ret = EOF;
    }
#endif /* __linux */
    (void) gettimeofday(&sdata.tp, NULL);
    sdata.getpid = getpid();
    sdata.getppid = getppid();
    sdata.getuid = getuid();
    sdata.geteuid = geteuid();
    sdata.getgid = getgid();
    sdata.getegid = getegid();
    (void) stat(".", &sdata.stat_dot);
    (void) stat("..", &sdata.stat_dotdot);
    (void) stat("/tmp", &sdata.stat_tmp);
    (void) stat("/", &sdata.stat_root);
    (void) fstat(0, &sdata.fstat_stdin);
    (void) fstat(1, &sdata.fstat_stdout);
    (void) fstat(2, &sdata.fstat_stderr);
#if !defined(__bsdi)
    (void) ustat(sdata.stat_dotdot.st_dev, &sdata.ustat_dotdot);
    (void) ustat(sdata.stat_dot.st_dev, &sdata.ustat_dot);
    (void) ustat(sdata.stat_tmp.st_dev, &sdata.ustat_tmp);
    (void) ustat(sdata.stat_root.st_dev, &sdata.ustat_root);
    (void) ustat(sdata.fstat_stdin.st_dev, &sdata.ustat_stdin);
    (void) ustat(sdata.fstat_stdout.st_dev, &sdata.ustat_stdout);
    (void) ustat(sdata.fstat_stderr.st_dev, &sdata.ustat_stderr);
    sdata.getsid = getsid((pid_t)0);
    sdata.getpgid = getpgid((pid_t)0);
#endif /* __bsdi */
    (void) getrusage(RUSAGE_SELF, &sdata.rusage);
    (void) getrusage(RUSAGE_CHILDREN, &sdata.rusage_chld);
    (void) gettimeofday(&sdata.tp2, NULL);
    sdata.size = sizeof(sdata);
    (void) setjmp(sdata.env);
    sdata.sdata_p = (char *)&sdata;

    /*
     * seed the generator with the above data
     */
    hash_val = hash_buf((char *)&sdata, sizeof(sdata));

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
