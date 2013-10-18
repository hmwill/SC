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

/*
** -------------------------------------------------------------------------
** DDA Engine
** -------------------------------------------------------------------------
*/

#define PRECISION		VPMT_SUBPIXEL_BITS
#define ADD_PRECISION	(PRECISION + VPMT_VIEWPORT_COORD_BITS)
#define EXT_PRECISION	(PRECISION + ADD_PRECISION)
#define HALF			(1 << (PRECISION - 1))
#define ONE				(1 << PRECISION)
#define EXT_ONE			(1 << EXT_PRECISION)
#define MASK_FRAC		(ONE - 1)
#define MASK_INT		(~MASK_FRAC)
#define EXT_MASK_FRAC	(EXT_ONE - 1)
#define EXT_MASK_INT	(~EXT_MASK_FRAC)

typedef struct DDA {
	VPMT_Vec2i current;										   /* current screen coordinates   */
	GLsizei steps;											   /* number of steps left         */
	GLint inc;												   /* major coordinate increment   */
	GLint error;											   /* error value                  */
	GLint delta;											   /* fractional error per step    */
	GLint bigStep;											   /* big step                     */
	GLint smallStep;										   /* small step                   */
} DDA;

/*
** Initialize the DDA for raster scan conversion of a segment between
** from and to.
**
** @param	dda			The DDA data structure to initialize
** @param	preStep		Reference to location where to store pre-step size
** @param	from		Starting point of segment
** @param	to			End point of segment
**
** @return the number of steps the dda should perform
*/
static GLsizei InitializeDDA(struct DDA *dda, VPMT_Vec2 preStep,
							 GLint from_x, GLint from_y, GLint to_x, GLint to_y)
{
	GLint dx = to_x - from_x;								   /* PRECISION fractional bits    */
	GLint dy = to_y - from_y;								   /* PRECISION fractional bits    */
	GLint sx = dx < 0 ? -1 : 1;
	GLint sy = dy < 0 ? -1 : 1;
	GLint dy_dx;
	GLint x_start, x_end, steps, x_pre, y_pre_ext;

	dx *= sx;
	dy *= sy;

	dy_dx = (int)											   /* (((long long) (dy << EXT_PRECISION)) / dx); */
		((dy * 1.0f * EXT_ONE) / dx);

	dda->current[0] = (from_x + sx * HALF - (sx > 0)) >> PRECISION;
	x_start = (dda->current[0] << PRECISION) + HALF;
	x_pre = x_start - from_x;
	y_pre_ext = /*(from_y << ADD_PRECISION) + */ sy * sx * ((x_pre * dy_dx) >> PRECISION);

	preStep[0] = (1.0f / ONE) * x_pre;
	preStep[1] = (1.0f / EXT_ONE) * y_pre_ext;

	x_end = ((to_x + /*-*/ sx * HALF - (sx > 0)) & MASK_INT) + HALF;
	steps = VPMT_MAX(0, abs(x_end - x_start) >> PRECISION);

	dda->current[1] = ((from_y << ADD_PRECISION) + y_pre_ext) >> EXT_PRECISION;

	dda->error = ((from_y << ADD_PRECISION) + y_pre_ext) - (dda->current[1] << EXT_PRECISION);

	if (sx < 0) {
		dda->error = (EXT_ONE - 1) - dda->error;
	}

	dda->smallStep = sy * (dy_dx >> EXT_PRECISION);
	dda->bigStep = dda->smallStep + sy;
	dda->inc = sx;
	dda->steps = steps;

	dda->delta = dy_dx & EXT_MASK_FRAC;

	if (!dda->delta) {
		dda->delta = EXT_ONE;
		dda->smallStep -= sy;
		dda->bigStep -= sy;
	}

	return steps;
}

/*
** Perform a single step of the DDA.
**
** @return GL_TRUE if a big step was taken, GL_FALSE if a small step
*/
VPMT_INLINE static GLboolean StepDDA(struct DDA *dda)
{
	dda->current[0] += dda->inc;
	dda->error += dda->delta;

	if (dda->error >= EXT_ONE) {
		dda->error -= EXT_ONE;
		dda->current[1] += dda->bigStep;
		return GL_TRUE;
	} else {
		dda->current[1] += dda->smallStep;
		return GL_FALSE;
	}
}

/*
** -------------------------------------------------------------------------
** Actual rasterizer code
** -------------------------------------------------------------------------
*/

static void RasterVerticalBar(VPMT_Context * context, GLuint x, GLuint y,
							  VPMT_Color4ub rgba, GLuint depth, GLsizei count)
{
	VPMT_FrameBuffer fb;

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, x, y);

	/* pixel ownership test based on intersection of scissor and surface rect */
	if ((x - context->activeSurfaceRect.origin[0]) < 0 ||
		(x - context->activeSurfaceRect.origin[0]) >=
		(GLuint) context->activeSurfaceRect.size.width) {
		return;
	}

	do {
		/* pixel ownership test based on intersection of scissor and surface rect */
		if ((y - context->activeSurfaceRect.origin[1]) < 0
			|| (y - context->activeSurfaceRect.origin[1]) >=
			(GLuint) context->activeSurfaceRect.size.height) {
			continue;
		}

		VPMT_Fragment(context, &fb, rgba, depth);
		VPMT_FrameBufferStepY(&fb);
	} while (--count);
}

static void RasterHorizontalBar(VPMT_Context * context, GLuint x, GLuint y,
								VPMT_Color4ub rgba, GLuint depth, GLsizei count)
{
	VPMT_FrameBuffer fb;

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, x, y);

	/* pixel ownership test based on intersection of scissor and surface rect */
	if ((y - context->activeSurfaceRect.origin[1]) < 0
		|| (y - context->activeSurfaceRect.origin[1]) >=
		(GLuint) context->activeSurfaceRect.size.height) {
		return;
	}

	do {
		/* pixel ownership test based on intersection of scissor and surface rect */
		if ((x - context->activeSurfaceRect.origin[0]) < 0 ||
			(x - context->activeSurfaceRect.origin[0]) >=
			(GLuint) context->activeSurfaceRect.size.width) {
			continue;
		}

		VPMT_Fragment(context, &fb, rgba, depth);
		VPMT_FrameBufferStepX(&fb);
	} while (--count);
}

static void RasterLineFlat(VPMT_Context * context, const VPMT_RasterVertex * a,
						   const VPMT_RasterVertex * b)
{
	VPMT_Color4ub rgba = VPMT_ConvertVec4ToColor4ub(b->rgba);

	DDA dda;
	VPMT_Vec2 preStep;
	GLint width = context->integerLineWidth;

	GLint dx = abs(a->screenCoords[0] - b->screenCoords[0]);
	GLint dy = abs(a->screenCoords[1] - b->screenCoords[1]);

	GLfloat depth, depthInc;								   /* interpolation values */

	GLfloat fdx = dx * (1.0f / ONE), fdy = dy * (1.0f / ONE);

	GLfloat sqrLength = fdx * fdx + fdy * fdy;

	GLfloat factorX = (fdx / sqrLength);
	GLfloat factorY = (fdy / sqrLength);

	depthInc = b->depth - a->depth;

	if (dx >= dy) {
		/* x-major line */
		GLint steps = InitializeDDA(&dda, preStep,
									a->screenCoords[0], a->screenCoords[1] - (width - 1) * HALF,
									b->screenCoords[0], b->screenCoords[1] - (width - 1) * HALF);

		if (steps) {
			GLfloat factor = preStep[0] * factorX + preStep[1] * factorY;

			depth = a->depth + depthInc * factor;

			RasterVerticalBar(context, dda.current[0], dda.current[1], rgba, (GLuint) depth, width);

			while (--steps) {
				factor = StepDDA(&dda) ? factorX + factorY : factorX;

				depth += depthInc * factor;

				RasterVerticalBar(context, dda.current[0], dda.current[1], rgba, (GLuint) depth,
								  width);
			}
		}
	} else {
		/* y-major line; swap x and y */
		GLint steps = InitializeDDA(&dda, preStep,
									a->screenCoords[1], a->screenCoords[0] - (width - 1) * HALF,
									b->screenCoords[1], b->screenCoords[0] - (width - 1) * HALF);

		if (steps) {
			GLfloat factor = preStep[0] * factorY + preStep[1] * factorX;

			depth = a->depth + depthInc * factor;

			RasterHorizontalBar(context, dda.current[1], dda.current[0], rgba, (GLuint) depth,
								width);

			while (--steps) {
				factor = StepDDA(&dda) ? factorX + factorY : factorY;

				depth += depthInc * factor;

				RasterHorizontalBar(context, dda.current[1], dda.current[0], rgba, (GLuint) depth,
									width);
			}
		}
	}
}

static void RasterLineStippledFlat(VPMT_Context * context, const VPMT_RasterVertex * a,
								   const VPMT_RasterVertex * b)
{
	VPMT_Color4ub rgba = VPMT_ConvertVec4ToColor4ub(b->rgba);

	DDA dda;
	VPMT_Vec2 preStep;
	GLint width = context->integerLineWidth;

	GLint dx = abs(a->screenCoords[0] - b->screenCoords[0]);
	GLint dy = abs(a->screenCoords[1] - b->screenCoords[1]);

	GLfloat depth, depthInc;								   /* interpolation values */

	GLfloat fdx = dx * (1.0f / ONE), fdy = dy * (1.0f / ONE);

	GLfloat sqrLength = fdx * fdx + fdy * fdy;

	GLfloat factorX = (fdx / sqrLength);
	GLfloat factorY = (fdy / sqrLength);

	GLint lineStipplePattern = context->lineStipplePattern;
	GLint lineStippleRepeat = context->lineStippleRepeat;
	GLsizei lineStipplePatternIndex = context->lineStipplePatternIndex;
	GLint lineStippleCounter = context->lineStippleCounter;

	depthInc = b->depth - a->depth;

	if (dx >= dy) {
		/* x-major line */
		GLint steps = InitializeDDA(&dda, preStep,
									a->screenCoords[0], a->screenCoords[1] - (width - 1) * HALF,
									b->screenCoords[0], b->screenCoords[1] - (width - 1) * HALF);

		if (steps) {
			GLfloat factor = preStep[0] * factorX + preStep[1] * factorY;

			depth = a->depth + depthInc * factor;

			if (lineStipplePattern & (1 << lineStipplePatternIndex)) {
				RasterVerticalBar(context, dda.current[0], dda.current[1], rgba, (GLuint) depth,
								  width);
			}

			if (!--lineStippleCounter) {
				lineStippleCounter = lineStippleRepeat;
				lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
			}

			while (--steps) {
				factor = StepDDA(&dda) ? factorX + factorY : factorX;

				depth += depthInc * factor;

				if (lineStipplePattern & (1 << lineStipplePatternIndex)) {
					RasterVerticalBar(context, dda.current[0], dda.current[1], rgba, (GLuint) depth,
									  width);
				}

				if (!--lineStippleCounter) {
					lineStippleCounter = lineStippleRepeat;
					lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
				}
			}
		}
	} else {
		/* y-major line; swap x and y */
		GLint steps = InitializeDDA(&dda, preStep,
									a->screenCoords[1], a->screenCoords[0] - (width - 1) * HALF,
									b->screenCoords[1], b->screenCoords[0] - (width - 1) * HALF);

		if (steps) {
			GLfloat factor = preStep[0] * factorY + preStep[1] * factorX;

			depth = a->depth + depthInc * factor;

			if (lineStipplePattern & (1 << lineStipplePatternIndex)) {
				RasterHorizontalBar(context, dda.current[1], dda.current[0], rgba, (GLuint) depth,
									width);
			}

			if (!--lineStippleCounter) {
				lineStippleCounter = lineStippleRepeat;
				lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
			}

			while (--steps) {
				factor = StepDDA(&dda) ? factorX + factorY : factorY;

				depth += depthInc * factor;

				if (lineStipplePattern & (1 << lineStipplePatternIndex)) {
					RasterHorizontalBar(context, dda.current[1], dda.current[0], rgba,
										(GLuint) depth, width);
				}

				if (!--lineStippleCounter) {
					lineStippleCounter = lineStippleRepeat;
					lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
				}
			}
		}
	}

	/* save line stipple state back to context */
	context->lineStipplePatternIndex = lineStipplePatternIndex;
	context->lineStippleCounter = lineStippleCounter;
}

static GLfloat CalcLineRhoInvW2(const VPMT_Texture2D * texture,
								const GLfloat rsInvZ[2], const GLfloat rsInvZInc[2],
								GLfloat invW, GLfloat InvWInc, GLfloat stretch)
{
	if (texture->complete &&
		(texture->texMinFilter == GL_NEAREST_MIPMAP_LINEAR ||
		 texture->texMinFilter == GL_NEAREST_MIPMAP_NEAREST ||
		 texture->texMinFilter == GL_LINEAR_MIPMAP_LINEAR ||
		 texture->texMinFilter == GL_LINEAR_MIPMAP_NEAREST)) {
		GLsizei width = texture->mipmaps[0]->size.width;
		GLsizei height = texture->mipmaps[0]->size.height;

		GLfloat invW2 = invW * invW;
		GLfloat dudi = (rsInvZInc[0] * invW - InvWInc * rsInvZ[0]) * width;
		GLfloat dvdi = (rsInvZInc[1] * invW - InvWInc * rsInvZ[1]) * height;

		return sqrtf(dudi * dudi + dvdi * dvdi) * invW2 * stretch;
	} else {
		return 0.0f;
	}
}

static void RasterLineShaded(VPMT_Context * context, const VPMT_RasterVertex * a,
							 const VPMT_RasterVertex * b)
{
	GLfloat depth, depthInc;								   /* interpolation values */
	GLfloat invW, invWInc;
	GLfloat W, W2;
	VPMT_Vec2 texCoordsInvW[VPMT_MAX_TEX_UNITS];
	VPMT_Vec4 color, colorInc;
	VPMT_Vec2 texCoords[VPMT_MAX_TEX_UNITS], texCoordsIncW[VPMT_MAX_TEX_UNITS];
	GLfloat rhoInvW[VPMT_MAX_TEX_UNITS];
	GLfloat rhoInc[VPMT_MAX_TEX_UNITS];
	GLfloat rho[VPMT_MAX_TEX_UNITS];
	GLsizei index;
	GLuint rasterInterpolants = context->rasterInterpolants;
	VPMT_Color4ub rgba;

	DDA dda;
	VPMT_Vec2 preStep;
	GLint width = context->integerLineWidth;

	GLint dx = abs(a->screenCoords[0] - b->screenCoords[0]);
	GLint dy = abs(a->screenCoords[1] - b->screenCoords[1]);

	GLfloat fdx = dx * (1.0f / ONE), fdy = dy * (1.0f / ONE);

	GLfloat sqrLength = fdx * fdx + fdy * fdy;
	GLfloat stretch = sqrLength ? 1.0f / sqrtf(sqrLength) : 0.0f;

	GLfloat factorX = (fdx / sqrLength);
	GLfloat factorY = (fdy / sqrLength);
	GLfloat rhoA[VPMT_MAX_TEX_UNITS], rhoB[VPMT_MAX_TEX_UNITS];

	depthInc = (b->depth - a->depth);
	invWInc = (b->depth - a->depth);

	VPMT_Vec4Sub(colorInc, b->rgba, a->rgba);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2Sub(texCoordsIncW[index], b->texCoords[index], a->texCoords[index]);

			if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
				rhoA[index] = CalcLineRhoInvW2(context->texUnits[index].boundTexture,
											   a->texCoords[index], texCoordsIncW[index],
											   a->invW, invWInc, stretch);

				rhoB[index] = CalcLineRhoInvW2(context->texUnits[index].boundTexture,
											   b->texCoords[index], texCoordsIncW[index],
											   b->invW, invWInc, stretch);

				rhoInc[index] = rhoB[index] - rhoA[index];
			}
		}
	}

	if (dx >= dy) {
		/* x-major line */
		GLint steps = InitializeDDA(&dda, preStep,
									a->screenCoords[0], a->screenCoords[1] - (width - 1) * HALF,
									b->screenCoords[0], b->screenCoords[1] - (width - 1) * HALF);

		if (steps) {
			GLfloat factor = preStep[0] * factorX + preStep[1] * factorY;

			depth = a->depth + depthInc * factor;
			invW = a->invW + invWInc * factor;

			W = 1.0f / invW;
			W2 = W * W;

			VPMT_Vec4ScaleAdd(color, colorInc, factor, a->rgba);

			for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
				if (context->texture2DEnabledMask & (1u << index)) {
					VPMT_Vec2ScaleAdd(texCoordsInvW[index], texCoordsIncW[index], factor,
									  a->texCoords[index]);
					VPMT_Vec2Scale(texCoords[index], texCoordsInvW[index], W);

					/* TODO: this could be done only if mapmapping is needed */
					rhoInvW[index] = rhoA[index] + rhoInc[index] * factor;
					rho[index] = rhoInvW[index] * W2;
				}
			}

			rgba =
				VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(color),
										  texCoords, rho);

			RasterVerticalBar(context, dda.current[0], dda.current[1], rgba, (GLuint) depth, width);

			while (--steps) {
				factor = StepDDA(&dda) ? factorX + factorY : factorX;

				depth += depthInc * factor;
				invW += invWInc * factor;

				W = 1.0f / invW;
				W2 = W * W;

				VPMT_Vec4ScaleAdd(color, colorInc, factor, color);

				for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
					if (context->texture2DEnabledMask & (1u << index)) {
						VPMT_Vec2ScaleAdd(texCoordsInvW[index], texCoordsIncW[index], factor,
										  texCoordsInvW[index]);
						VPMT_Vec2Scale(texCoords[index], texCoordsInvW[index], W);

						/* TODO: this could be done only if mapmapping is needed */
						rhoInvW[index] = rhoInvW[index] + rhoInc[index] * factor;
						rho[index] = rhoInvW[index] * W2;
					}
				}

				rgba =
					VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(color),
											  texCoords, rho);

				RasterVerticalBar(context, dda.current[0], dda.current[1], rgba, (GLuint) depth,
								  width);
			}
		}
	} else {
		/* y-major line; swap x and y */
		GLint steps = InitializeDDA(&dda, preStep,
									a->screenCoords[1], a->screenCoords[0] - (width - 1) * HALF,
									b->screenCoords[1], b->screenCoords[0] - (width - 1) * HALF);

		if (steps) {
			GLfloat factor = preStep[0] * factorY + preStep[1] * factorX;

			depth = a->depth + depthInc * factor;
			invW = a->invW + invWInc * factor;

			W = 1.0f / invW;
			W2 = W * W;

			VPMT_Vec4ScaleAdd(color, colorInc, factor, a->rgba);

			for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
				if (context->texture2DEnabledMask & (1u << index)) {
					VPMT_Vec2ScaleAdd(texCoordsInvW[index], texCoordsIncW[index], factor,
									  a->texCoords[index]);
					VPMT_Vec2Scale(texCoords[index], texCoordsInvW[index], W);

					/* TODO: this could be done only if mapmapping is needed */
					rhoInvW[index] = rhoA[index] + rhoInc[index] * factor;
					rho[index] = rhoInvW[index] * W2;
				}
			}

			rgba =
				VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(color),
										  texCoords, rho);

			RasterHorizontalBar(context, dda.current[1], dda.current[0], rgba, (GLuint) depth,
								width);

			while (--steps) {
				factor = StepDDA(&dda) ? factorX + factorY : factorY;

				depth += depthInc * factor;
				invW += invWInc * factor;

				W = 1.0f / invW;
				W2 = W * W;

				VPMT_Vec4ScaleAdd(color, colorInc, factor, color);

				for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
					if (context->texture2DEnabledMask & (1u << index)) {
						VPMT_Vec2ScaleAdd(texCoordsInvW[index], texCoordsIncW[index], factor,
										  texCoordsInvW[index]);
						VPMT_Vec2Scale(texCoords[index], texCoordsInvW[index], W);

						/* TODO: this could be done only if mapmapping is needed */
						rhoInvW[index] = rhoInvW[index] + rhoInc[index] * factor;
						rho[index] = rhoInvW[index] * W2;
					}
				}

				rgba =
					VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(color),
											  texCoords, rho);

				RasterHorizontalBar(context, dda.current[1], dda.current[0], rgba, (GLuint) depth,
									width);
			}
		}
	}
}

static void RasterLineStippledShaded(VPMT_Context * context, const VPMT_RasterVertex * a,
									 const VPMT_RasterVertex * b)
{
	GLfloat depth, depthInc;								   /* interpolation values */
	GLfloat invW, invWInc;
	GLfloat W, W2;
	VPMT_Vec2 texCoordsInvW[VPMT_MAX_TEX_UNITS];
	VPMT_Vec4 color, colorInc;
	VPMT_Vec2 texCoords[VPMT_MAX_TEX_UNITS], texCoordsIncW[VPMT_MAX_TEX_UNITS];
	GLfloat rhoInvW[VPMT_MAX_TEX_UNITS];
	GLfloat rhoInc[VPMT_MAX_TEX_UNITS];
	GLfloat rho[VPMT_MAX_TEX_UNITS];
	GLsizei index;
	GLuint rasterInterpolants = context->rasterInterpolants;

	VPMT_Color4ub rgba;

	DDA dda;
	VPMT_Vec2 preStep;
	GLint width = context->integerLineWidth;

	GLint dx = abs(a->screenCoords[0] - b->screenCoords[0]);
	GLint dy = abs(a->screenCoords[1] - b->screenCoords[1]);

	GLfloat fdx = dx * (1.0f / ONE), fdy = dy * (1.0f / ONE);

	GLfloat sqrLength = fdx * fdx + fdy * fdy;
	GLfloat stretch = sqrLength ? 1.0f / sqrtf(sqrLength) : 0.0f;

	GLfloat factorX = (fdx / sqrLength);
	GLfloat factorY = (fdy / sqrLength);
	GLfloat rhoA[VPMT_MAX_TEX_UNITS], rhoB[VPMT_MAX_TEX_UNITS];

	GLint lineStipplePattern = context->lineStipplePattern;
	GLint lineStippleRepeat = context->lineStippleRepeat;
	GLsizei lineStipplePatternIndex = context->lineStipplePatternIndex;
	GLint lineStippleCounter = context->lineStippleCounter;

	depthInc = (b->depth - a->depth);
	invWInc = (b->depth - a->depth);

	if (rasterInterpolants & VPMT_RasterInterpolateColor) {
		VPMT_Vec4Sub(colorInc, b->rgba, a->rgba);
	}

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2Sub(texCoordsIncW[index], b->texCoords[index], a->texCoords[index]);

			if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
				rhoA[index] = CalcLineRhoInvW2(context->texUnits[index].boundTexture,
											   a->texCoords[index], texCoordsIncW[index],
											   a->invW, invWInc, stretch);

				rhoB[index] = CalcLineRhoInvW2(context->texUnits[index].boundTexture,
											   b->texCoords[index], texCoordsIncW[index],
											   b->invW, invWInc, stretch);

				rhoInc[index] = rhoB[index] - rhoA[index];
			}
		}
	}

	if (dx >= dy) {
		/* x-major line */
		GLint steps = InitializeDDA(&dda, preStep,
									a->screenCoords[0], a->screenCoords[1] - (width - 1) * HALF,
									b->screenCoords[0], b->screenCoords[1] - (width - 1) * HALF);

		if (steps) {
			GLfloat factor = preStep[0] * factorX + preStep[1] * factorY;

			depth = a->depth + depthInc * factor;
			invW = a->invW + invWInc * factor;

			W = 1.0f / invW;
			W2 = W * W;

			VPMT_Vec4ScaleAdd(color, colorInc, factor, a->rgba);

			for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
				if (context->texture2DEnabledMask & (1u << index)) {
					VPMT_Vec2ScaleAdd(texCoordsInvW[index], texCoordsIncW[index], factor,
									  a->texCoords[index]);
					VPMT_Vec2Scale(texCoords[index], texCoordsInvW[index], W);

					/* TODO: this could be done only if mapmapping is needed */
					rhoInvW[index] = rhoA[index] + rhoInc[index] * factor;
					rho[index] = rhoInvW[index] * W2;
				}
			}

			rgba =
				VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(color),
										  texCoords, rho);

			if (lineStipplePattern & (1 << lineStipplePatternIndex)) {
				RasterVerticalBar(context, dda.current[0], dda.current[1], rgba, (GLuint) depth,
								  width);
			}

			if (!--lineStippleCounter) {
				lineStippleCounter = lineStippleRepeat;
				lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
			}

			while (--steps) {
				factor = StepDDA(&dda) ? factorX + factorY : factorX;

				depth += depthInc * factor;
				invW += invWInc * factor;

				W = 1.0f / invW;
				W2 = W * W;

				VPMT_Vec4ScaleAdd(color, colorInc, factor, color);

				for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
					if (context->texture2DEnabledMask & (1u << index)) {
						VPMT_Vec2ScaleAdd(texCoordsInvW[index], texCoordsIncW[index], factor,
										  texCoordsInvW[index]);
						VPMT_Vec2Scale(texCoords[index], texCoordsInvW[index], W);

						/* TODO: this could be done only if mapmapping is needed */
						rhoInvW[index] = rhoInvW[index] + rhoInc[index] * factor;
						rho[index] = rhoInvW[index] * W2;
					}
				}

				rgba =
					VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(color),
											  texCoords, rho);

				if (lineStipplePattern & (1 << lineStipplePatternIndex)) {
					RasterVerticalBar(context, dda.current[0], dda.current[1], rgba, (GLuint) depth,
									  width);
				}

				if (!--lineStippleCounter) {
					lineStippleCounter = lineStippleRepeat;
					lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
				}
			}
		}
	} else {
		/* y-major line; swap x and y */
		GLint steps = InitializeDDA(&dda, preStep,
									a->screenCoords[1], a->screenCoords[0] - (width - 1) * HALF,
									b->screenCoords[1], b->screenCoords[0] - (width - 1) * HALF);

		if (steps) {
			GLfloat factor = preStep[0] * factorY + preStep[1] * factorX;

			depth = a->depth + depthInc * factor;
			invW = a->invW + invWInc * factor;

			W = 1.0f / invW;
			W2 = W * W;

			VPMT_Vec4ScaleAdd(color, colorInc, factor, a->rgba);

			for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
				if (context->texture2DEnabledMask & (1u << index)) {
					VPMT_Vec2ScaleAdd(texCoordsInvW[index], texCoordsIncW[index], factor,
									  a->texCoords[index]);
					VPMT_Vec2Scale(texCoords[index], texCoordsInvW[index], W);

					/* TODO: this could be done only if mapmapping is needed */
					rhoInvW[index] = rhoA[index] + rhoInc[index] * factor;
					rho[index] = rhoInvW[index] * W2;
				}
			}

			rgba =
				VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(color),
										  texCoords, rho);

			if (lineStipplePattern & (1 << lineStipplePatternIndex)) {
				RasterHorizontalBar(context, dda.current[1], dda.current[0], rgba, (GLuint) depth,
									width);
			}

			if (!--lineStippleCounter) {
				lineStippleCounter = lineStippleRepeat;
				lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
			}

			while (--steps) {
				factor = StepDDA(&dda) ? factorX + factorY : factorY;

				depth += depthInc * factor;
				invW += invWInc * factor;

				W = 1.0f / invW;
				W2 = W * W;

				VPMT_Vec4ScaleAdd(color, colorInc, factor, color);

				for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
					if (context->texture2DEnabledMask & (1u << index)) {
						VPMT_Vec2ScaleAdd(texCoordsInvW[index], texCoordsIncW[index], factor,
										  texCoordsInvW[index]);
						VPMT_Vec2Scale(texCoords[index], texCoordsInvW[index], W);

						/* TODO: this could be done only if mapmapping is needed */
						rhoInvW[index] = rhoInvW[index] + rhoInc[index] * factor;
						rho[index] = rhoInvW[index] * W2;
					}
				}

				rgba =
					VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(color),
											  texCoords, rho);

				if (lineStipplePattern & (1 << lineStipplePatternIndex)) {
					RasterHorizontalBar(context, dda.current[1], dda.current[0], rgba,
										(GLuint) depth, width);
				}

				if (!--lineStippleCounter) {
					lineStippleCounter = lineStippleRepeat;
					lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
				}
			}
		}
	}

	/* save line stipple state back to context */
	context->lineStipplePatternIndex = lineStipplePatternIndex;
	context->lineStippleCounter = lineStippleCounter;
}

static void RasterLineAA(VPMT_Context * context, const VPMT_RasterVertex * a,
						 const VPMT_RasterVertex * b)
{
	VPMT_Vec2i delta, ortho;
	VPMT_RasterVertex ra, rb, rc, rd;
	GLint sqrLength;
	GLfloat invLength, factor;

	ra = rb = *a;
	rc = rd = *b;

	/* determine offset of new end points */
	VPMT_Vec2Sub(delta, b->screenCoords, a->screenCoords);
	ortho[0] = -delta[1];
	ortho[1] = delta[0];
	sqrLength = VPMT_Vec2Dot(delta, delta);
	invLength = (GLfloat) ONE / sqrtf((float) sqrLength);
	factor = 0.5f * invLength * context->integerLineWidth;
	ortho[0] = VPMT_ROUND(ortho[0] * factor);
	ortho[1] = VPMT_ROUND(ortho[1] * factor);
	VPMT_Vec2Add(ra.screenCoords, ra.screenCoords, ortho);
	VPMT_Vec2Sub(rb.screenCoords, rb.screenCoords, ortho);
	VPMT_Vec2Sub(rc.screenCoords, rc.screenCoords, ortho);
	VPMT_Vec2Add(rd.screenCoords, rd.screenCoords, ortho);

	VPMT_RasterQuadAA(context, &ra, &rb, &rc, &rd);
}

static void RasterLineStippledAA(VPMT_Context * context, const VPMT_RasterVertex * a,
								 const VPMT_RasterVertex * b)
{
	VPMT_RasterLineFunc segment = RasterLineAA;
	GLsizei lineStipplePatternIndex = context->lineStipplePatternIndex;
	GLint lineStippleCounter = context->lineStippleCounter;
	GLint lineStipplePattern = context->lineStipplePattern;
	GLint lineStippleRepeat = context->lineStippleRepeat;

	VPMT_Vec2i delta;
	GLfloat length, invLength;

	VPMT_RasterVertex start, end;

	VPMT_Vec2 stepScreenCoords, deltaScreenCoords;
	GLfloat stepDepth, deltaDepth;
	GLfloat stepInvW, deltaInvW;
	VPMT_Vec4 stepRgba, deltaRgba;
	VPMT_Vec2 stepTexCoords[VPMT_MAX_TEX_UNITS], deltaTexCoords[VPMT_MAX_TEX_UNITS];

	GLsizei steps, index;
	GLboolean inSegment = GL_FALSE;

	end = *b;												   /* initialize attributes */

	VPMT_Vec2Sub(delta, b->screenCoords, a->screenCoords);
	length = sqrtf((GLfloat) VPMT_Vec2Dot(delta, delta));
	steps = (GLsizei) length >> VPMT_SUBPIXEL_BITS;
	invLength = ONE / length;

	stepScreenCoords[0] = (GLfloat) a->screenCoords[0];
	stepScreenCoords[1] = (GLfloat) a->screenCoords[1];
	stepDepth = a->depth;
	stepInvW = a->invW;

	VPMT_Vec4Copy(stepRgba, a->rgba);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_Vec2Copy(stepTexCoords[index], a->texCoords[index]);
	}

	deltaScreenCoords[0] = delta[0] * invLength;
	deltaScreenCoords[1] = delta[1] * invLength;
	deltaDepth = (b->depth - a->depth) * invLength;
	deltaInvW = (b->invW - a->invW) * invLength;

	VPMT_Vec4Sub(deltaRgba, b->rgba, a->rgba);
	VPMT_Vec4Scale(deltaRgba, deltaRgba, invLength);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_Vec2Sub(deltaTexCoords[index], b->texCoords[index], a->texCoords[index]);
		VPMT_Vec2Scale(deltaTexCoords[index], deltaTexCoords[index], invLength);
	}

	for (; steps > 0; --steps) {
		GLboolean stipple = (lineStipplePattern & (1 << lineStipplePatternIndex)) != GL_FALSE;

		if (!--lineStippleCounter) {
			lineStippleCounter = lineStippleRepeat;
			lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
		}

		if (inSegment && !stipple) {
			/* end a segment */
			inSegment = GL_FALSE;
			end.screenCoords[0] = VPMT_ROUND(stepScreenCoords[0]);
			end.screenCoords[1] = VPMT_ROUND(stepScreenCoords[1]);

			end.depth = stepDepth;
			end.invW = stepInvW;

			VPMT_Vec4Copy(end.rgba, stepRgba);

			for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
				VPMT_Vec2Copy(end.texCoords[index], stepTexCoords[index]);
			}

			segment(context, &start, &end);
		} else if (!inSegment && stipple) {
			/* begin a segment */
			inSegment = GL_TRUE;

			start.screenCoords[0] = VPMT_ROUND(stepScreenCoords[0]);
			start.screenCoords[1] = VPMT_ROUND(stepScreenCoords[1]);

			start.depth = stepDepth;
			start.invW = stepInvW;

			VPMT_Vec4Copy(start.rgba, stepRgba);

			for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
				VPMT_Vec2Copy(start.texCoords[index], stepTexCoords[index]);
			}
		}

		stepScreenCoords[0] += deltaScreenCoords[0];
		stepScreenCoords[1] += deltaScreenCoords[1];
		stepDepth += deltaDepth;
		stepInvW += deltaInvW;

		VPMT_Vec4Add(stepRgba, stepRgba, deltaRgba);

		for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
			VPMT_Vec2Add(stepTexCoords[index], stepTexCoords[index], deltaTexCoords[index]);
		}
	}

	if (VPMT_FRAC(length)) {
		/* fractional left-over */
		GLboolean stipple = (lineStipplePattern & (1 << lineStipplePatternIndex)) != GL_FALSE;

		if (!--lineStippleCounter) {
			lineStippleCounter = lineStippleRepeat;
			lineStipplePatternIndex = (lineStipplePatternIndex + 1) & 15;
		}

		if (inSegment) {
			/* end a segment */
			if (!stipple) {
				end.screenCoords[0] = VPMT_ROUND(stepScreenCoords[0]);
				end.screenCoords[1] = VPMT_ROUND(stepScreenCoords[1]);

				end.depth = stepDepth;
				end.invW = stepInvW;

				VPMT_Vec4Copy(end.rgba, stepRgba);

				for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
					VPMT_Vec2Copy(end.texCoords[index], stepTexCoords[index]);
				}

				segment(context, &start, &end);
			} else {
				segment(context, &start, b);
			}
		} else if (!inSegment && stipple) {
			/* begin a segment */
			start.screenCoords[0] = VPMT_ROUND(stepScreenCoords[0]);
			start.screenCoords[1] = VPMT_ROUND(stepScreenCoords[1]);

			start.depth = stepDepth;
			start.invW = stepInvW;

			VPMT_Vec4Copy(start.rgba, stepRgba);

			for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
				VPMT_Vec2Copy(start.texCoords[index], stepTexCoords[index]);
			}

			segment(context, &start, b);
		}
	} else if (inSegment) {
		segment(context, &start, b);
	}

	context->lineStipplePatternIndex = lineStipplePatternIndex;
	context->lineStippleCounter = lineStippleCounter;
}

void VPMT_RasterPrepareLine(VPMT_Context * context)
{
	GLint width = VPMT_ROUND(context->lineWidth);

	if (width <= 0) {
		width = 1;
	} else if (width > (GLint) VPMT_MAX_LINE_WIDTH) {
		width = (GLint) VPMT_MAX_LINE_WIDTH;
	}

	context->integerLineWidth = width;

	VPMT_LineStippleReset(context);
	VPMT_RasterPrepareInterpolants(context);

	if (context->lineSmoothEnabled) {
		if (context->shadeModel == GL_FLAT && !context->texture2DEnabledMask) {
			if (context->lineStippleEnabled) {
				context->rasterLine = RasterLineStippledAA;
			} else {
				context->rasterLine = RasterLineAA;
			}
		} else {
			if (context->lineStippleEnabled) {
				context->rasterLine = RasterLineStippledAA;
			} else {
				context->rasterLine = RasterLineAA;
			}
		}
	} else {
		if (context->shadeModel == GL_FLAT && !context->texture2DEnabledMask) {
			if (context->lineStippleEnabled) {
				context->rasterLine = RasterLineStippledFlat;
			} else {
				context->rasterLine = RasterLineFlat;
			}
		} else {
			if (context->lineStippleEnabled) {
				context->rasterLine = RasterLineStippledShaded;
			} else {
				context->rasterLine = RasterLineShaded;
			}
		}
	}
}

void VPMT_LineStippleReset(VPMT_Context * context)
{
	context->lineStipplePatternIndex = 0;
	context->lineStippleCounter = context->lineStippleRepeat;
}

/* $Id: rasterln.c 74 2008-11-23 07:25:12Z hmwill $ */
