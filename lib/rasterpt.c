/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Rasterizer functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "raster.h"
#include "frame.h"

#define PRECISION		VPMT_SUBPIXEL_BITS
#define HALF			(1 << (PRECISION - 1))
#define ONE				(1 << PRECISION)

/*
** -------------------------------------------------------------------------
** Actual rasterizer code
** -------------------------------------------------------------------------
*/

static void RasterPointAliased(VPMT_Context * context, const VPMT_RasterVertex * a)
{
	VPMT_Color4ub rgba;
	GLuint xctr, yctr, xmin, ymin, xmax, ymax, x, y;
	GLuint size = context->integerPointSize;
	GLuint depth = (GLuint) a->depth;
	VPMT_FrameBuffer fb;

	/* 1. determine the center, xmin, max, ymin, ymax */
	if (size & 1) {
		/* odd point size */
		xctr = a->screenCoords[0] >> VPMT_SUBPIXEL_BITS;
		yctr = a->screenCoords[1] >> VPMT_SUBPIXEL_BITS;

		xmin = xctr - (size >> 1);
		ymin = yctr - (size >> 1);

		xmax = xctr + (size >> 1) + 1;
		ymax = yctr + (size >> 1) + 1;
	} else {
		/* even point size */
		xctr = (a->screenCoords[0] + HALF) >> VPMT_SUBPIXEL_BITS;
		yctr = (a->screenCoords[1] + HALF) >> VPMT_SUBPIXEL_BITS;

		xmin = xctr - (size >> 1);
		ymin = yctr - (size >> 1);

		xmax = xctr + (size >> 1);
		ymax = yctr + (size >> 1);
	}

	/* clip to scissor */
	xmin = VPMT_MAX(xmin, (GLuint) context->activeSurfaceRect.origin[0]);
	ymin = VPMT_MAX(ymin, (GLuint) context->activeSurfaceRect.origin[1]);
	xmax =
		VPMT_MIN(xmax,
				 (GLuint) context->activeSurfaceRect.origin[0] +
				 context->activeSurfaceRect.size.width);
	ymax =
		VPMT_MIN(ymax,
				 (GLuint) context->activeSurfaceRect.origin[1] +
				 context->activeSurfaceRect.size.height);

	/* 2. lookup texture color */
	rgba =
		VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(a->rgba),
								  a->texCoords, NULL);

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, xmin, ymin);

	/* 3. Generate fragment for each (xmin, ymin) <= (x, y) < (xmax, ymax) */
	for (y = ymin; y < ymax; ++y) {
		VPMT_FrameBufferSave(&fb);

		for (x = xmin; x < xmax; ++x) {
			VPMT_Fragment(context, &fb, rgba, depth);
			VPMT_FrameBufferStepX(&fb);
		}

		VPMT_FrameBufferRestore(&fb);
		VPMT_FrameBufferStepY(&fb);
	}
}

/* as value 0..256 */
static GLuint PointCoverage(GLsizei xbase, GLsizei ybase, GLsizei sqrRadius)
{
	GLint x, y;
	GLuint count = 0;

	/* we are sampling at every second sub-pixel */
	for (y = 0; y < ONE; y += 2) {
		for (x = 0; x < ONE; x += 2) {
			if ((x + xbase) * (x + xbase) + (y + ybase) * (y + ybase) < sqrRadius) {
				++count;
			}
		}
	}

	return (count * 256) / (HALF * HALF);
}

static void RasterPointAA(VPMT_Context * context, const VPMT_RasterVertex * a)
{
	VPMT_Color4ub rgba;
	GLuint xctr, yctr, xmin, ymin, xmax, ymax, x, y;
	GLsizei xbase0, ybase, xbase1;
	GLuint aaPointSize = context->aaPointSize, sqrPointSize = aaPointSize * aaPointSize;
	GLubyte alpha;
	GLuint depth = (GLuint) a->depth;
	VPMT_FrameBuffer fb;

	/* 1. determine the center, xmin, max, ymin, ymax */
	xctr = a->screenCoords[0];
	yctr = a->screenCoords[1];

	xmin = (xctr - aaPointSize) >> VPMT_SUBPIXEL_BITS;
	ymin = (yctr - aaPointSize) >> VPMT_SUBPIXEL_BITS;
	xmax = (xctr + aaPointSize + ONE - 1) >> VPMT_SUBPIXEL_BITS;
	ymax = (yctr + aaPointSize + ONE - 1) >> VPMT_SUBPIXEL_BITS;

	/* clip to scissor */
	xmin = VPMT_MAX(xmin, (GLuint) context->activeSurfaceRect.origin[0]);
	ymin = VPMT_MAX(ymin, (GLuint) context->activeSurfaceRect.origin[1]);
	xmax =
		VPMT_MIN(xmax,
				 (GLuint) context->activeSurfaceRect.origin[0] +
				 context->activeSurfaceRect.size.width);
	ymax =
		VPMT_MIN(ymax,
				 (GLuint) context->activeSurfaceRect.origin[1] +
				 context->activeSurfaceRect.size.height);

	/* offset by one sub-pixel */
	xbase0 = (xmin << VPMT_SUBPIXEL_BITS) - xctr + 1;
	ybase = (ymin << VPMT_SUBPIXEL_BITS) - yctr + 1;

	/* 2. lookup texture color */
	rgba =
		VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(a->rgba),
								  a->texCoords, NULL);
	alpha = rgba.alpha;

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, xmin, ymin);

	/* 3. Generate fragment for each (xmin, ymin) <= (x, y) < (xmax, ymax) */
	for (y = ymin; y < ymax; ++y, ybase += ONE) {
		VPMT_FrameBufferSave(&fb);
		xbase1 = xbase0;

		for (x = xmin; x < xmax; ++x, xbase1 += ONE) {
			GLuint coverage = PointCoverage(xbase1, ybase, sqrPointSize);

			if (coverage > 0.0f) {
				rgba.alpha = (coverage * alpha) >> 8;
				VPMT_Fragment(context, &fb, rgba, depth);
			}

			VPMT_FrameBufferStepX(&fb);
		}

		VPMT_FrameBufferRestore(&fb);
		VPMT_FrameBufferStepY(&fb);
	}
}

void VPMT_RasterPreparePoint(VPMT_Context * context)
{
	if (context->pointSmoothEnabled) {
		GLint size = VPMT_ROUND(context->pointSize * ONE);

		if (size <= 0) {
			size = 1;
		} else if (size > (GLint) VPMT_MAX_POINT_SIZE * ONE) {
			size = (GLint) VPMT_MAX_POINT_SIZE *ONE;
		}

		context->aaPointSize = size;
		context->rasterPoint = RasterPointAA;
	} else {
		GLint size = VPMT_ROUND(context->pointSize);

		if (size <= 0) {
			size = 1;
		} else if (size > (GLint) VPMT_MAX_POINT_SIZE) {
			size = (GLint) VPMT_MAX_POINT_SIZE;
		}

		context->integerPointSize = size;
		context->rasterPoint = RasterPointAliased;
	}
}

/* $Id: rasterpt.c 74 2008-11-23 07:25:12Z hmwill $ */
