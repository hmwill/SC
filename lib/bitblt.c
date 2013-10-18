/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Bit transfer engine.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"

static void ClipRectToSize(VPMT_Rect * result, const VPMT_Rect * rect, const VPMT_Size * size)
{
	if (rect->origin[0] < 0) {
		result->origin[0] = 0;
	} else {
		result->origin[0] = rect->origin[0];
	}

	if (rect->origin[1] < 0) {
		result->origin[1] = 0;
	} else {
		result->origin[1] = rect->origin[1];
	}

	if (rect->origin[0] + rect->size.width > size->width) {
		result->size.width = size->width - result->origin[0];
	} else {
		result->size.width = rect->origin[0] + rect->size.width - result->origin[0];
	}

	if (rect->origin[1] + rect->size.height > size->height) {
		result->size.height = size->height - result->origin[1];
	} else {
		result->size.height = rect->origin[1] + rect->size.height - result->origin[1];
	}
}

void VPMT_Bitblt(const VPMT_Image2D * dst, const VPMT_Rect * dstRect, const VPMT_Image2D * src,
				 const GLint * srcPos)
{
	VPMT_Rect actualDstRect;
	GLsizei pixelSize;
	GLsizei dstRowIncrement, srcRowIncrement;
	GLsizei dstSpan, srcSpan, copySpan;
	GLsizei rowCount;
	GLubyte *dstPtr;
	const GLubyte *srcPtr;

	/* initially, we do not support any change of pixel types */
	assert(dst->pixelFormat->internalFormat == src->pixelFormat->internalFormat &&
		dst->pixelFormat->type == src->pixelFormat->type);

	ClipRectToSize(&actualDstRect, dstRect, &dst->size);

	pixelSize = dst->pixelFormat->size;

	dstSpan = dst->size.width * pixelSize;

	dstRowIncrement = dst->pitch;

	srcSpan = src->size.width * pixelSize;

	srcRowIncrement = src->pitch;

	dstPtr = ((GLubyte *) dst->data) + actualDstRect.origin[0] * pixelSize +
		actualDstRect.origin[1] * dstRowIncrement;

	srcPtr = srcPos ?
		((const GLubyte *) src->data) + srcPos[0] * pixelSize + srcPos[1] * srcRowIncrement :
		((const GLubyte *) src->data);

	copySpan = actualDstRect.size.width * pixelSize;

	for (rowCount = actualDstRect.size.height; rowCount > 0;
		 --rowCount, dstPtr += dstRowIncrement, srcPtr += srcRowIncrement) {
		GLubyte *dstPtr0 = dstPtr;
		const GLubyte *srcPtr0 = srcPtr;
		GLsizei columnCount;

		for (columnCount = copySpan; columnCount > 0; --columnCount, ++dstPtr0, ++srcPtr0) {
			*dstPtr0 = *srcPtr0;
		}
	}
}

/* $Id: bitblt.c 74 2008-11-23 07:25:12Z hmwill $ */
