/*
 * blkcpy - general values and related routines used by the calculator
 *
 * Copyright (C) 1999-2007,2021  Landon Curt Noll and Ernest Bowen
 *
 * Primary author:  Landon Curt Noll
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
 * Under source code control:	1997/04/18 20:41:26
 * File existed as early as:	1997
 *
 * Share and enjoy!  :-)	http://www.isthe.com/chongo/tech/comp/calc/
 */


#include <stdio.h>
#include <sys/types.h>
#include "calc.h"
#include "alloc.h"
#include "value.h"
#include "file.h"
#include "blkcpy.h"
#include "str.h"


#include "banned.h"	/* include after system header <> includes */


/*
 * copystod - copy num indexed items from source value to destination value
 *
 * given:
 *	ssi = source starting index
 *	num = number of items (octets or values) to be copied
 *	sdi = destination starting index
 *
 * returns:
 *	zero if successful, otherwise error-code number
 */
int
copystod(VALUE *svp, long ssi, long num, VALUE *dvp, long dsi)
{
	BLOCK *sblk;
	BLOCK *dblk;
	BOOL noreloc;

	sblk = NULL;
	dblk = NULL;

	/*
	 * check protections
	 */
	if (svp->v_subtype & V_NOCOPYFROM)
		return E_COPY13;
	if (dvp->v_subtype & V_NOCOPYTO)
		return E_COPY14;
	noreloc = ((dvp->v_subtype & V_NOREALLOC) != 0);

	/*
	 * determine/check source type
	 */
	switch(svp->v_type) {
	case V_NBLOCK:
		if (svp->v_nblock->subtype & V_NOCOPYFROM)
			return E_COPY15;
		sblk = svp->v_nblock->blk;
		if (sblk->data == NULL)
			return E_COPY8;
		break;
	case V_BLOCK:
		sblk = svp->v_block;
		break;
	case V_STR:
	case V_OCTET:
	case V_NUM:
	case V_FILE:
	case V_MAT:
	case V_LIST:
		break;
	default:
		return E_COPY9;
	}

	/*
	 * determine/check destination type
	 */
	switch(dvp->v_type) {
	case V_NBLOCK:
		if (dvp->v_nblock->subtype & V_NOCOPYTO)
			return E_COPY16;
		noreloc |=((dvp->v_nblock->subtype & V_NOREALLOC) != 0);
		dblk = dvp->v_nblock->blk;
		if (dblk->data == NULL)
			return E_COPY10;
		break;
	case V_BLOCK:
		noreloc = ((dvp->v_subtype & V_NOREALLOC) != 0);
		dblk = dvp->v_block;
		break;
	case V_STR:
	case V_NUM:
	case V_FILE:
	case V_MAT:
	case V_LIST:
		break;
	default:
		return E_COPY11;
	}

	/*
	 * copy based on source
	 */
	switch (svp->v_type) {
	case V_BLOCK:
	case V_NBLOCK:
		/*
		 * copy from a block
		 */
		switch(dvp->v_type) {
		case V_BLOCK:
		case V_NBLOCK:
			return copyblk2blk(sblk, ssi, num, dblk, dsi, noreloc);
		case V_NUM:
		    {
			NUMBER *n;	/* modified number */
			int rt;		/* return code */

			/* copy to a number */
			rt = copyblk2num(sblk, ssi, num, dvp->v_num, dsi, &n);
			if (rt == 0) {
				qfree(dvp->v_num);
				dvp->v_num = n;
			}
			return rt;
		    }
		case V_FILE:
			return copyblk2file(sblk, ssi, num, dvp->v_file, dsi);
		case V_MAT:
			return copyblk2mat(sblk, ssi, num, dvp->v_mat, dsi);
		case V_STR:
			return copyblk2str(sblk, ssi, num, dvp->v_str, dsi);
		}
		return E_COPY12;

	case V_STR:
		switch(dvp->v_type) {
		case V_BLOCK:
		case V_NBLOCK:
			/* copy to a block */
			return copystr2blk(svp->v_str, ssi, num, dblk, dsi,
				noreloc);
		case V_FILE:
			return copystr2file(svp->v_str, ssi, num,
				dvp->v_file, dsi);
		case V_STR:
			return copystr2str(svp->v_str, ssi, num, dvp->v_str,
				dsi);
		}
		return E_COPY12;

	case V_OCTET:
		switch(dvp->v_type) {
		case V_BLOCK:
		case V_NBLOCK:
			return copyostr2blk((char *) svp->v_octet, ssi, num,
				dblk, dsi, noreloc);
		case V_STR:
			return copyostr2str((char *) svp->v_octet, ssi, num,
				dvp->v_str, dsi);
		}
		return E_COPY12;

	case V_NUM:

		/*
		 * copy from a number
		 */
		if (dblk != NULL) {
			/* copy to a block */
			return copynum2blk(svp->v_num, ssi, num, dblk,
				dsi, noreloc);
		}
		switch (dvp->v_type) {
		case V_MAT:
			/* copy to a matrix */
			return E_COPY12;	/* not yet - XXX */
		case V_LIST:
			/* copy to a list */
			return E_COPY12;	/* not yet - XXX */
		}
		break;
	case V_FILE:

		/*
		 * copy from a file
		 */
		if (dblk != NULL) {
			/* copy to a block */
			return copyfile2blk(svp->v_file, ssi, num,
					    dblk, dsi, noreloc);
		}
		switch (dvp->v_type) {
		case V_NUM:
			/* copy to a number */
			return E_COPY12;	/* not yet - XXX */
		}
		break;

	case V_MAT:

		/*
		 * copy from a matrix
		 */
		if (dblk != NULL) {
			/* copy to a block */
			return copymat2blk(svp->v_mat, ssi, num, dblk,
				dsi, noreloc);
		}
		switch (dvp->v_type) {
		case V_MAT:
			/* copy to a matrix */
			return copymat2mat(svp->v_mat, ssi, num,
					   dvp->v_mat, dsi);
		case V_LIST:
			/* copy to a list */
			return copymat2list(svp->v_mat, ssi, num,
					    dvp->v_list, dsi);
		}
		break;

	case V_LIST:

		/*
		 * copy from a list
		 */
		if (dblk != NULL) {
			/* copy to a block */
			return E_COPY12;	/* not yet - XXX */
		}
		switch (dvp->v_type) {
		case V_MAT:
			/* copy to a matrix */
			return copylist2mat(svp->v_list, ssi, num,
					    dvp->v_mat, dsi);
		case V_LIST:
			/* copy to a list */
			return copylist2list(svp->v_list, ssi, num,
					     dvp->v_list, dsi);
		}
		break;
	}

	/*
	 * unsupported copy combination
	 */
	return E_COPY12;
}


/*
 * copymat2mat - copy matrix to matrix
 */
int
copymat2mat(MATRIX *smat, long ssi, long num, MATRIX *dmat, long dsi)
{
	long i;
	VALUE *vp;
	VALUE *vq;
	VALUE *vtemp;
	unsigned short subtype;

	if (ssi > smat->m_size)
		return E_COPY2;

	if (num < 0)
		num = smat->m_size - ssi;
	if (ssi + num > smat->m_size)
		return E_COPY5;
	if (num == 0)
		return 0;
	if (dsi < 0)
		dsi = 0;
	if (dsi + num > dmat->m_size)
		return E_COPY7;
	vtemp = (VALUE *) malloc(num * sizeof(VALUE));
	if (vtemp == NULL) {
		math_error("Out of memory for mat-to-mat copy");
		/*NOTREACHED*/
	}
	vp = smat->m_table + ssi;
	vq = vtemp;
	i = num;
	while (i-- > 0)
		copyvalue(vp++, vq++);
	vp = vtemp;
	vq = dmat->m_table + dsi;
	for (i = num; i > 0; i--, vp++, vq++) {
		subtype = vq->v_subtype;
		freevalue(vq);
		*vq = *vp;
		vq->v_subtype |= subtype;
	}
	free(vtemp);
	return 0;
}


/*
 * copyblk2mat - copy block to matrix
 */
int
copyblk2mat(BLOCK *blk, long ssi, long num, MATRIX *dmat, long dsi)
{
	OCTET *op;
	VALUE *vp;
	VALUE *vq;
	VALUE *vtemp;
	long i;
	unsigned short subtype;

	if (ssi > blk->datalen)
		return E_COPY2;
	if (num < 0)
		num = blk->datalen - ssi;
	if (ssi + num > blk->datalen)
		return E_COPY5;
	if (num == 0)
		return 0;
	if (dsi < 0)
		dsi = 0;
	if (dsi + num > dmat->m_size)
		return E_COPY7;
	op = blk->data + ssi;
	vtemp = (VALUE *) malloc(num * sizeof(VALUE));
	if (vtemp == NULL) {
		math_error("Out of memory for block-to-matrix copy");
		/*NOTREACHED*/
	}
	vp = vtemp;
	i = num;
	while (i-- > 0) {
		vp->v_type = V_NUM;
		vp->v_subtype = V_NOSUBTYPE;
		vp->v_num = itoq((long) *op++);
		vp++;
	}
	vp = vtemp;
	vq = dmat->m_table + dsi;
	for (i = num; i > 0; i--, vp++, vq++) {
		subtype = vq->v_subtype;
		freevalue(vq);
		*vq = *vp;
		vq->v_subtype |= subtype;
	}
	free(vtemp);
	return 0;
}


/*
 * copymat2blk - copy matrix to block
 */
int
copymat2blk(MATRIX *smat, long ssi, long num, BLOCK *dblk, long dsi,
	    BOOL noreloc)
{
	long i;
	long newlen;
	long newsize;
	USB8 *newdata;
	VALUE *vp;
	OCTET *op;

	if (ssi > smat->m_size)
		return E_COPY2;
	if (num < 0)
		num = smat->m_size - ssi;
	if (num == 0)
		return 0;
	if (ssi + num > smat->m_size)
		return E_COPY5;
	if (dsi < 0)
		dsi = dblk->datalen;
	newlen = dsi + num;
	if (newlen <= 0)
		return E_COPY7;
	if (newlen >= dblk->maxsize) {
		if (noreloc)
			return E_COPY17;
		newsize = (1 + newlen/dblk->blkchunk) * dblk->blkchunk;
		newdata = (USB8*) realloc(dblk->data, newsize);
		if (newdata == NULL) {
			math_error("Out of memory for matrix-to-block copy");
			/*NOTREACHED*/
		}
		dblk->data = newdata;
		dblk->maxsize = newsize;
	}
	vp = smat->m_table + ssi;
	op = dblk->data + dsi;
	for (i = num; i > 0; i--)
		copy2octet(vp++, op++);
	if (newlen > dblk->datalen)
		dblk->datalen = newlen;
	return 0;
}


/*
 * copymat2list - copy matrix to list
 */
int
copymat2list(MATRIX *smat, long ssi, long num, LIST *lp, long dsi)
{
	VALUE *vp;
	VALUE *vq;
	LISTELEM *ep;
	VALUE *vtemp;
	long i;
	unsigned short subtype;

	if (ssi > smat->m_size)
		return E_COPY2;
	if (num < 0)
		num = smat->m_size - ssi;
	if (num == 0)
		return 0;
	if (ssi + num > smat->m_size)
		return E_COPY5;
	if (dsi < 0)
		dsi = 0;
	if (dsi + num > lp->l_count)
		return E_COPY7;
	vtemp = (VALUE *) malloc(num * sizeof(VALUE));
	if (vtemp == NULL) {
		math_error("Out of memory for matrix-to-list copy");
		/*NOTREACHED*/
	}
	vp = smat->m_table + ssi;
	vq = vtemp;
	i = num;
	while (i-- > 0)
		copyvalue(vp++, vq++);
	vq = vtemp;
	ep = listelement(lp, (long) dsi);
	i = num;
	while (i-- > 0) {
		subtype = ep->e_value.v_subtype;
		freevalue(&ep->e_value);
		ep->e_value = *vq++;
		ep->e_value.v_subtype |= subtype;
		ep = ep->e_next;
	}
	free(vtemp);
	return 0;
}


/*
 * copymat2list - copy list to matrix
 */
int
copylist2mat(LIST *lp, long ssi, long num, MATRIX *dmat, long dsi)
{
	VALUE *vp;
	VALUE *vq;
	LISTELEM *ep;
	VALUE *vtemp;
	long i;
	unsigned short subtype;

	if (ssi > lp->l_count)
		return E_COPY2;
	if (num < 0)
		num = lp->l_count - ssi;
	if (num == 0)
		return 0;
	if (ssi + num > lp->l_count)
		return E_COPY5;
	if (dsi < 0)
		dsi = 0;
	if (dsi + num > dmat->m_size)
		return E_COPY7;
	vtemp = (VALUE *) malloc(num * sizeof(VALUE));
	if (vtemp == NULL) {
		math_error("Out of memory for list-to-matrix copy");
		/*NOTREACHED*/
	}
	ep = listelement(lp, (long) ssi);
	vp = vtemp;
	i = num;
	while (i-- > 0) {
		copyvalue(&ep->e_value, vp++);
		ep = ep->e_next;
	}
	vp = vtemp;
	vq = dmat->m_table + dsi;
	for (i = num; i > 0; i--, vp++, vq++) {
		subtype = vq->v_subtype;
		freevalue(vq);
		*vq = *vp;
		vq->v_subtype |= subtype;
	}
	free(vtemp);
	return 0;
}


/*
 * copylist2list - copy list to list
 */
int
copylist2list(LIST *slp, long ssi, long num, LIST *dlp, long dsi)
{
	long i;
	LISTELEM *sep;
	LISTELEM *dep;
	VALUE *vtemp;
	VALUE *vp;
	unsigned short subtype;

	if (ssi > slp->l_count)
		return E_COPY2;
	if (num < 0)
		num = slp->l_count - ssi;
	if (num == 0)
		return 0;
	if (ssi + num > slp->l_count)
		return E_COPY5;
	if (dsi < 0)
		dsi = 0;
	if (dsi + num > dlp->l_count)
		return E_COPY7;
	vtemp = (VALUE *) malloc(num * sizeof(VALUE));
	if (vtemp == NULL) {
		math_error("Out of memory for list-to-list copy");
		/*NOTREACHED*/
	}
	sep = listelement(slp, (long) ssi);
	vp = vtemp;
	i = num;
	while (i-- > 0) {
		copyvalue(&sep->e_value, vp++);
		sep = sep->e_next;
	}
	dep = listelement(dlp, (long) dsi);
	vp = vtemp;
	i = num;
	while (i-- > 0) {
		subtype = dep->e_value.v_subtype;
		freevalue(&dep->e_value);
		dep->e_value = *vp++;
		dep->e_value.v_subtype |= subtype;
		dep = dep->e_next;
	}
	free(vtemp);
	return 0;
}


/*
 * copyblk2file - copy block to file
 */
int
copyblk2file(BLOCK *sblk, long ssi, long num, FILEID id, long dsi)
{
	FILEIO	*fiop;
	FILE	*fp;
	long	numw;

	if (ssi > sblk->datalen)
		return E_COPY2;
	if (num < 0)
		num = sblk->datalen - ssi;
	if (num == 0)
		return 0;

	fiop = findid(id, TRUE);
	if (fiop == NULL)
		return E_COPYF1;
	fp = fiop->fp;
	if (id == 1 || id == 2) {
		numw = idfputstr(id, (char *)sblk->data + ssi);	 /* XXX */
		return 0;
	}
	if (dsi >= 0) {
		if (fseek(fp, dsi, 0))
			return E_COPYF2;
	}
	numw = fwrite(sblk->data + ssi, 1, num, fp);
	if (numw < num)
		return E_COPYF3;
	fflush(fp);
	return 0;
}


/*
 * copyfile2blk - copy file to block
 */
int
copyfile2blk(FILEID id, long ssi, long num, BLOCK *dblk, long dsi, BOOL noreloc)
{
	FILEIO	*fiop;
	FILE	*fp;
	long	numw;
	ZVALUE	fsize;
	long	filelen;
	long	newlen;
	long	newsize;
	OCTET	*newdata;

	if (id < 3)			/* excludes copying from stdin */
		return E_COPYF1;
	fiop = findid(id, FALSE);
	if (fiop == NULL)
		return E_COPYF1;

	fp = fiop->fp;

	if (get_open_siz(fp, &fsize))
		return E_COPYF2;
	if (zge31b(fsize)) {
		zfree(fsize);
		return E_COPY5;
	}
	filelen = ztoi(fsize);
	zfree(fsize);

	if (ssi > filelen)
		return E_COPY2;
	if (num < 0)
		num = filelen - ssi;
	if (num == 0)
		return 0;
	if (ssi + num > filelen)
		return E_COPY5;
	if (fseek(fp, ssi, 0))		/* using system fseek  XXX */
		return E_COPYF2;
	if (dsi < 0)
		dsi = dblk->datalen;
	newlen = dsi + num;
	if (newlen <= 0)
		return E_COPY7;
	if (newlen >= dblk->maxsize) {
		if (noreloc)
			return E_COPY17;
		newsize = (1 + newlen/dblk->blkchunk) * dblk->blkchunk;
		newdata = (USB8*) realloc(dblk->data, newsize);
		if (newdata == NULL) {
			math_error("Out of memory for block-to-block copy");
			/*NOTREACHED*/
		}
		dblk->data = newdata;
		dblk->maxsize = newsize;
	}
	numw = fread(dblk->data + dsi, 1, num, fp);
	if (numw < num)
		return E_COPYF4;
	if (newlen > dblk->datalen)
		dblk->datalen = newlen;
	return 0;
}


/*
 * copystr2file - copy string to file
 */
int
copystr2file(STRING *str, long ssi, long num, FILEID id, long dsi)
{
	long len;
	FILEIO *fiop;
	long numw;
	FILE *fp;

	len = str->s_len;

	if (ssi >= len)
		return E_COPY2;
	if (num < 0)
		num = len - ssi;
	if (num <= 0)			/* Nothing to be copied */
		return 0;
	if (ssi + num > len)
		return E_COPY5;		/* Insufficient memory in str */
	fiop = findid(id, TRUE);
	if (fiop == NULL)
		return E_COPYF1;
	fp = fiop->fp;
	if (id == 1 || id == 2) {
		numw = idfputstr(id, str->s_str + ssi);	 /* XXX */
		return 0;
	}
	if (dsi >= 0) {
		if (fseek(fp, dsi, 0))
			return E_COPYF2;
	}
	numw = fwrite(str->s_str + ssi, 1, num, fp);
	if (numw < num)
		return E_COPYF3;
	fflush(fp);
	return 0;
}


/*
 * copyblk2blk - copy block to block
 */
int
copyblk2blk(BLOCK *sblk, long ssi, long num, BLOCK *dblk, long dsi,
	    BOOL noreloc)
{
	long	newlen;
	long	newsize;
	USB8	*newdata;

	if (ssi > sblk->datalen)
		return E_COPY2;
	if (num < 0)
		num = sblk->datalen - ssi;
	if (num == 0)			/* Nothing to be copied */
		return 0;
	if (ssi + num > sblk->datalen)
		return E_COPY5;
	if (dsi < 0)
		dsi = dblk->datalen;
	newlen = dsi + num;
	if (newlen <= 0)
		return E_COPY7;
	if (newlen >= dblk->maxsize) {
		if (noreloc)
			return E_COPY17;
		newsize = (1 + newlen/dblk->blkchunk) * dblk->blkchunk;
		newdata = (USB8*) realloc(dblk->data, newsize);
		if (newdata == NULL) {
			math_error("Out of memory for block-to-block copy");
			/*NOTREACHED*/
		}
		dblk->data = newdata;
		dblk->maxsize = newsize;
	}
	memmove(dblk->data + dsi, sblk->data + ssi, num);
	if (newlen > dblk->datalen)
		dblk->datalen = newlen;
	return 0;
}


/*
 * copystr2blk - copy string to block
 */
int
copystr2blk(STRING *str, long ssi, long num, BLOCK *dblk, long dsi,
	    BOOL noreloc)
{
	long	len;
	long	newlen;
	long	newsize;
	USB8	*newdata;

	len = str->s_len;

	if (ssi >= len)
		return E_COPY2;
	if (num < 0)
		num = len - ssi;
	if (num <= 0)			/* Nothing to be copied */
		return 0;
	if (dsi < 0)
		dsi = dblk->datalen;
	newlen = dsi + num + 1;
	if (newlen <= 0)
		return E_COPY7;
	if (newlen >= dblk->maxsize) {
		if (noreloc)
			return E_COPY17;
		newsize = (1 + newlen/dblk->blkchunk) * dblk->blkchunk;
		newdata = (USB8*) realloc(dblk->data, newsize);
		if (newdata == NULL) {
			math_error("Out of memory for string-to-block copy");
			/*NOTREACHED*/
		}
		dblk->data = newdata;
		dblk->maxsize = newsize;
	}
	memmove(dblk->data + dsi, str->s_str + ssi, num);
	dblk->data[dsi + num] = '\0';
	if (newlen > dblk->datalen)
		dblk->datalen = newlen;
	return 0;
}


/*
 * copystr2str - copy up to num characters from sstr (starting at
 * index ssi) to dstr (starting at index dsi); num is reduced if there
 * are insufficient characters in sstr or insufficient space in dstr.
 */
int
copystr2str(STRING *sstr, long ssi, long num, STRING *dstr, long dsi)
{
	char *c, *c1;

	if (num < 0 || (size_t)(ssi + num) > sstr->s_len)
		num = sstr->s_len - ssi;
	if (num <= 0)
		return 0;		/* Nothing to be copied */
	if (dsi < 0)			/* default destination index */
		dsi = 0;
	if ((size_t)(dsi + num) > dstr->s_len)
		num = dstr->s_len - dsi;
	c1 = sstr->s_str + ssi;
	c = dstr->s_str + dsi;
	while (num-- > 0)
		*c++ = *c1++;
	return 0;
}


/*
 * copyblk2str - copy up to num characters from sblk (starting at
 * index ssi) to str (starting at index dsi); num is reduced if there
 * is insufficient data in blk or insufficient space in str
 */
int
copyblk2str(BLOCK *sblk, long ssi, long num, STRING *dstr, long dsi)
{
	USB8 *c, *c1;

	if (num < 0 || ssi + num > sblk->datalen)
		num = sblk->datalen - ssi;
	if (num <= 0)
		return 0;		/* Nothing to be copied */
	if (dsi < 0)			/* default destination index */
		dsi = 0;
	if ((size_t)(dsi + num) > dstr->s_len)
		num = dstr->s_len - dsi;
	c1 = sblk->data + ssi;
	c = (USB8 *)dstr->s_str + dsi;
	while (num-- > 0)
		*c++ = *c1++;
	return 0;
}
/*
 * copyostr2str - copy octet-specified string to string
 */
int
copyostr2str(char *sstr, long ssi, long num, STRING *dstr, long dsi)
{
	size_t len;
	char	*c, *c1;

	len = strlen(sstr);

	if (num < 0 || (size_t)(ssi + num) > len)
		num = len - ssi;
	if (num <= 0)			/* Nothing to be copied */
		return 0;
	if (dsi < 0)
		dsi = 0;		/* Default destination index */
	if ((size_t)(dsi + num) > dstr->s_len)
		num = dstr->s_len - dsi;
	c1 = sstr + ssi;
	c = dstr->s_str + dsi;
	while (num-- > 0)
		*c++ = *c1++;
	return 0;
}


/*
 * copyostr2blk - copy octet-specified string to block
 */
int
copyostr2blk(char *str,long ssi,long num,BLOCK *dblk,long dsi,BOOL noreloc)
{
	size_t	len;
	size_t	newlen;
	size_t	newsize;
	USB8	*newdata;

	len = strlen(str) + 1;

	if (ssi > 0 && (size_t)ssi > len)
		return E_COPY2;
	if (num < 0 || (size_t)(ssi + num) > len)
		num = len - ssi;
	if (num <= 0)			/* Nothing to be copied */
		return 0;
	if (dsi < 0)
		dsi = dblk->datalen;	/* Default destination index */
	newlen = dsi + num;
	if (newlen <= 0)
		return E_COPY7;
	if (newlen >= (size_t)dblk->maxsize) {
		if (noreloc)
			return E_COPY17;
		newsize = (1 + newlen/dblk->blkchunk) * dblk->blkchunk;
		newdata = (USB8*) realloc(dblk->data, newsize);
		if (newdata == NULL) {
			math_error("Out of memory for string-to-block copy");
			/*NOTREACHED*/
		}
		dblk->data = newdata;
		dblk->maxsize = newsize;
	}
	memmove(dblk->data + dsi, str + ssi, num);
	if (newlen > (size_t)dblk->datalen)
		dblk->datalen = newlen;
	return 0;
}
#if !defined(HAVE_MEMMOVE)
/*
 * memmove - simulate the memory move function that deals with overlap
 *
 * Copying between objects that overlap will take place correctly.
 *
 * given:
 *	s1	destination
 *	s2	source
 *	n	octet count
 *
 * returns:
 *	s1
 */
void *
memmove(void *s1, CONST void *s2, MEMMOVE_SIZE_T n)
{
	/*
	 * firewall
	 */
	if (s1 == NULL || s2 == NULL) {
		math_error("bogus memmove NULL ptr");
		/*NOTREACHED*/
	}
	if (n <= 0) {
		/* neg or 0 count does nothing */
		return s1;
	}
	if ((char *)s1 == (char *)s2) {
		/* copy to same location does nothing */
		return s1;
	}

	/*
	 * determine if we need to deal with overlap copy
	 */
	if ((char *)s1 > (char *)s2 && (char *)s1 < (char *)s2+n) {

		/*
		 * we have to copy backwards ... slowly
		 */
		while (n-- > 0) {
			((char *)s1)[n] = ((char *)s2)[n];
		}

	} else {

		/*
		 * safe ... no overlap problems
		 */
		(void) memcpy(s1, s2, n);

	}
	return s1;
}
#endif


/*
 * copynum2blk - copy number numerator to block
 */
int
copynum2blk(NUMBER *snum, long ssi, long num, BLOCK *dblk, long dsi,
	    BOOL noreloc)
{
	size_t	newlen;
	size_t	newsize;
	USB8	*newdata;
#if CALC_BYTE_ORDER == BIG_ENDIAN
	ZVALUE	*swnum; /* byte swapped numerator */
#endif

	if (ssi > snum->num.len)
		return E_COPY2;
	if (num < 0)
		num = snum->num.len - ssi;
	if (num == 0)			/* Nothing to be copied */
		return 0;
	if (ssi + num > snum->num.len)
		return E_COPY5;
	if (dsi < 0)
		dsi = dblk->datalen;
	newlen = dsi + (num*sizeof(HALF));
	if (newlen <= 0)
		return E_COPY7;
	if (newlen >= (size_t)dblk->maxsize) {
		if (noreloc)
			return E_COPY17;
		newsize = (1 + newlen/dblk->blkchunk) * dblk->blkchunk;
		newdata = (USB8*) realloc(dblk->data, newsize);
		if (newdata == NULL) {
			math_error("Out of memory for num-to-block copy");
			/*NOTREACHED*/
		}
		dblk->data = newdata;
		dblk->maxsize = newsize;
	}
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	memmove(dblk->data+dsi, (char *)(snum->num.v+ssi), num*sizeof(HALF));
#else
	swnum = swap_b8_in_ZVALUE(NULL, &(snum->num), FALSE);
	memmove(dblk->data+dsi, (char *)(swnum->v+ssi), num*sizeof(HALF));
	zfree(*swnum);
#endif
	if (newlen > (size_t)dblk->datalen)
		dblk->datalen = newlen;
	return 0;
}


/*
 * copyblk2num - copy block to number
 */
int
copyblk2num(BLOCK *sblk, long ssi, long num, NUMBER *dnum, long dsi,
	    NUMBER **res)
{
	size_t	newlen;
	NUMBER *ret;		/* cloned and modified numerator */
#if CALC_BYTE_ORDER == BIG_ENDIAN
	HALF *swapped;		/* byte swapped input data */
	unsigned long halflen;	/* length of the input rounded up to HALFs */
	HALF *h;		/* copy byteswap pointer */
	unsigned long i;
#endif

	if (ssi > sblk->datalen)
		return E_COPY2;
	if (num < 0)
		num = sblk->datalen - ssi;
	if (num == 0)			/* Nothing to be copied */
		return 0;
	if (ssi + num > sblk->datalen)
		return E_COPY5;
	if (dsi < 0)
		dsi = dnum->num.len;
	newlen = dsi + ((num+sizeof(HALF)-1)/sizeof(HALF));
	if (newlen <= 0)
		return E_COPY7;

	/* quasi-clone the numerator to the new size */
	ret = qalloc();
	ret->num.sign = dnum->num.sign;
	ret->num.v = alloc(newlen);
	ret->num.len = newlen;
	/* ensure that any trailing octets will be zero filled */
	ret->num.v[newlen-1] = 0;
	zcopyval(dnum->num, ret->num);
	if (!zisunit(ret->den)) {
		ret->den.len = dnum->den.len;
		ret->den.v = alloc(dnum->den.len);
		zcopyval(dnum->den, ret->den);
	}

	/* move the data */
#if CALC_BYTE_ORDER == LITTLE_ENDIAN
	memmove((char *)(ret->num.v + dsi), sblk->data + ssi, num);
#else
	/* form a HALF aligned copy of the input */
	halflen = (num+sizeof(HALF)-1) / sizeof(HALF);
	swapped = (HALF *)malloc(halflen * sizeof(HALF));
	if (swapped == NULL) {
		math_error("Out of memory for block-to-num copy");
		/*NOTREACHED*/
	}
	/* ensure that any trailing octets will be zero filled */
	swapped[halflen-1] = 0;
	memcpy(swapped, sblk->data + ssi, num);
	/* byte swap the copy of the input */
	for (i=0, h=swapped; i < halflen; ++i, ++h) {
		SWAP_B8_IN_HALF(h, h);
	}
	/* copy over whole byte-swapped HALFs */
	memcpy((char *)(ret->num.v + dsi), swapped,
	       (num/sizeof(HALF))*sizeof(HALF));
	/* copy over any octets in the last partial HALF */
	i = num % sizeof(HALF);
	if (i != 0) {
		memcpy((char *)(ret->num.v + dsi)+num-i,
		       (char *)swapped + num-i, i);
	}
	free(swapped);
#endif
	/* save new number */
	*res = ret;
	return 0;
}
