/*
 * align32 - determine if 32 bit accesses must be aligned
 *
 * This file was written by:
 *
 *	Landon Curt Noll
 *	http://reality.sgi.com/chongo/
 *
 *	chongo <was here> /\../\
 *
 * This code has been placed in the public domain.  Please do not
 * copyright this code.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH  REGARD  TO
 * THIS  SOFTWARE,  INCLUDING  ALL IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS.  IN NO EVENT SHALL  LANDON  CURT
 * NOLL  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM  LOSS  OF
 * USE,  DATA  OR  PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR  IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <signal.h>
#include "longbits.h"

#include "have_unistd.h"
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

static void buserr(void);	/* catch alignment errors */


int
main(void)
{
	char byte[2*sizeof(USB32)];	/* mis-alignment buffer */
	USB32 *p;			/* mis-alignment pointer */
	int i;

#if defined(MUST_ALIGN32)
	/* force alignment */
	printf("#define MUST_ALIGN32\t%c* forced to align 32 bit values *%c\n",
	   '/', '/');
#else
	/* setup to catch alignment bus errors */
	signal(SIGBUS, buserr);
	signal(SIGSEGV, buserr);  /* some systems will generate SEGV instead! */

	/* mis-align our long fetches */
	for (i=0; i < sizeof(USB32); ++i) {
		p = (USB32 *)(byte+i);
		*p = i;
		*p += 1;
	}

	/* if we got here, then we can mis-align longs */
	printf("#undef MUST_ALIGN32\t%c* can mis-align 32 bit values *%c\n",
	   '/', '/');

#endif
	/* exit(0); */
	return 0;
}


/*
 * buserr - catch an alignment error
 *
 * given:
 *      arg             to keep ANSI C happy
 */
/*ARGSUSED*/
static void
buserr(int arg)
{
	/* alignment is required */
	printf("#define MUST_ALIGN32\t%c* must align 32 bit values *%c\n",
	  '/', '/');
	exit(0);
}
