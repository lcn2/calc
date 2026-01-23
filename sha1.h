/*
 * sha1 - new NIST Secure Hash Standard-1 (SHA1)
 *
 * Written 2 September 1992, Peter C. Gutmann.
 *
 * This file is not covered under version 2.1 of the GNU LGPL.
 * This file is covered under "The unlicense":
 *
 *      https://unlicense.org
 *
 * In particular:
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#if !defined(INCLUDE_SHA1_H)
#  define INCLUDE_SHA1_H

/* SHA1_CHUNKSIZE must be a power of 2 - fixed value defined by the algorithm */
#  define SHA1_CHUNKSIZE (1 << 6)
#  define SHA1_CHUNKWORDS (SHA1_CHUNKSIZE / sizeof(uint32_t))

/* SHA1_DIGESTSIZE is a the length of the digest as defined by the algorithm */
#  define SHA1_DIGESTSIZE (20)
#  define SHA1_DIGESTWORDS (SHA1_DIGESTSIZE / sizeof(uint32_t))

/* SHA1_LOW - where low 32 bits of 64 bit count is stored during final */
#  define SHA1_LOW 15

/* SHA1_HIGH - where high 32 bits of 64 bit count is stored during final */
#  define SHA1_HIGH 14

/*
 * The structure for storing SHA1 info
 *
 * We will assume that bit count is a multiple of 8.
 */
typedef struct {
    uint32_t digest[SHA1_DIGESTWORDS]; /* message digest */
    uint32_t countLo;                  /* 64 bit count: bits 3-34 */
    uint32_t countHi;                  /* 64 bit count: bits 35-63 */
    uint32_t datalen;                  /* length of data in data */
    uint32_t data[SHA1_CHUNKWORDS];    /* SHA1 chunk buffer */
} SHA1_INFO;

/*
 * SHA1COUNT(SHA1_INFO*, uint32_t) - update the 64 bit count in an SHA1_INFO
 *
 * We will count bytes and convert to bit count during the final
 * transform.
 */
#  define SHA1COUNT(sha1info, count)                            \
      {                                                         \
          uint32_t tmp_countLo;                                    \
          tmp_countLo = (sha1info)->countLo;                    \
          if (((sha1info)->countLo += (count)) < tmp_countLo) { \
              (sha1info)->countHi++;                            \
          }                                                     \
      }

#endif
