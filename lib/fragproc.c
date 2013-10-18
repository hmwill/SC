/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Fragment processing functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "exec.h"
#include "frame.h"

/*
** -------------------------------------------------------------------------
** Clear buffers
** -------------------------------------------------------------------------
*/
void VPMT_ExecClear(VPMT_Context * context, GLbitfield mask)
{
	GLuint x, y;
	GLsizei width, height;
	VPMT_FrameBuffer fb;
	GLuint clearDepth, clearStencil;
	VPMT_Color4ub clearColor;
	GLboolean colorComponents;

	VPMT_NOT_RENDERING(context);

	if (!context->writeSurface) {
		/* should this flag invalid operation? */
		return;
	}

	clearDepth = (GLuint) (context->depthFixedPointScale * context->clearDepth);
	clearStencil = context->clearStencil;
	clearColor = VPMT_ConvertVec4ToColor4ub(context->clearColor);

	colorComponents =
		(context->writeSurface->image.pixelFormat->redBits && !context->colorWriteMask[0]) ||
		(context->writeSurface->image.pixelFormat->greenBits && !context->colorWriteMask[1]) ||
		(context->writeSurface->image.pixelFormat->blueBits && !context->colorWriteMask[2]) ||
		(context->writeSurface->image.pixelFormat->alphaBits && !context->colorWriteMask[3]);

	context->writeSurface->vtbl->lock(context, context->writeSurface);
	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, context->activeSurfaceRect.origin[0],
						 context->activeSurfaceRect.origin[1]);

	for (y = context->activeSurfaceRect.origin[1], height =
		 context->activeSurfaceRect.size.height; height != 0; --height, ++y) {

		VPMT_FrameBufferSave(&fb);

		for (x = context->activeSurfaceRect.origin[0], width =
			 context->activeSurfaceRect.size.width; width != 0; --width, ++x) {

			if (mask & GL_COLOR_BUFFER_BIT) {
				if (colorComponents) {
					VPMT_Color4ub color = VPMT_FrameReadColor(&fb);

					if (context->colorWriteMask[0]) {
						color.red = clearColor.red;
					}

					if (context->colorWriteMask[1]) {
						color.green = clearColor.green;
					}

					if (context->colorWriteMask[2]) {
						color.blue = clearColor.blue;
					}

					if (context->colorWriteMask[3]) {
						color.alpha = clearColor.alpha;
					}

					VPMT_FrameWriteColor(&fb, color);
				} else {
					VPMT_FrameWriteColor(&fb, clearColor);
				}
			}

			if (mask & GL_DEPTH_BUFFER_BIT) {
				VPMT_FrameWriteDepth(&fb, clearDepth);
			}

			if (mask & GL_STENCIL_BUFFER_BIT) {
				VPMT_FrameWriteStencil(&fb, clearStencil);
			}

			VPMT_FrameBufferStepX(&fb);
		}

		VPMT_FrameBufferRestore(&fb);
		VPMT_FrameBufferStepY(&fb);
	}

	context->writeSurface->vtbl->unlock(context, context->writeSurface);
}

/*
** -------------------------------------------------------------------------
** Generate a single fragment
** -------------------------------------------------------------------------
*/

static VPMT_INLINE GLubyte UByteLerp(GLubyte first, GLubyte second, GLuint lerp)
{
	GLint diff = (GLint) second - (GLint) first;
	GLint scaledDiff = (diff * (GLint) lerp) >> 8;

	return (GLubyte) ((GLint) first + scaledDiff);
}

void VPMT_Fragment(VPMT_Context * context, VPMT_FrameBuffer * fb, VPMT_Color4ub newColor,
				   GLuint depth)
{
	GLuint oldDepth, newDepth;
	GLuint oldStencil, newStencil;
	VPMT_Color4ub oldColor;
	GLboolean hasMaskedColor, hasEnabledColor;
	GLboolean hasEnabledDepthStencil;
	GLboolean writeColor = GL_TRUE;

	/* alpha test */
	if (context->alphaTestEnabled && context->alphaFunc == GL_LEQUAL) {
		if (newColor.alpha > context->alphaRefub) {
			return;
		}
	}

	/* depth & stencil test */
	hasEnabledDepthStencil =
		context->depthWriteMask || context->depthTestEnabled || context->stencilTestEnabled;

	if (hasEnabledDepthStencil) {

		GLboolean depthTest;
		GLuint stencilBufferMask = (1 << context->stencilBits) - 1;

		oldDepth = VPMT_FrameReadDepth(fb);
		oldStencil = VPMT_FrameReadStencil(fb);

		if (context->depthTestEnabled) {
			switch (context->depthFunc) {
			case GL_LESS:
				depthTest = depth < oldDepth;
				break;
			case GL_LEQUAL:
				depthTest = depth <= oldDepth;
				break;
			case GL_ALWAYS:
				depthTest = GL_TRUE;
				break;
			default:
				assert(GL_FALSE);
				break;
			}
		} else {
			depthTest = GL_TRUE;
		}

		if (context->stencilTestEnabled) {

			GLboolean stencilTest;
			GLenum op;
			GLuint maskedRef = stencilBufferMask & context->stencilMask & context->stencilRef;
			GLuint maskedValue = stencilBufferMask & context->stencilMask & oldStencil;

			switch (context->stencilFunc) {
			case GL_NEVER:
				stencilTest = GL_FALSE;
				break;
			case GL_LESS:
				stencilTest = maskedRef < maskedValue;
				break;
			case GL_LEQUAL:
				stencilTest = maskedRef <= maskedValue;
				break;
			case GL_GREATER:
				stencilTest = maskedRef > maskedValue;
				break;
			case GL_GEQUAL:
				stencilTest = maskedRef >= maskedValue;
				break;
			case GL_EQUAL:
				stencilTest = maskedRef == maskedValue;
				break;
			case GL_NOTEQUAL:
				stencilTest = maskedRef != maskedValue;
				break;
			case GL_ALWAYS:
				stencilTest = GL_TRUE;
				break;
			default:
				assert(GL_FALSE);
				break;
			}

			if (!stencilTest) {
				op = context->stencilOpFail;
				writeColor = GL_FALSE;
				newDepth = oldDepth;
			} else if (!depthTest) {
				op = context->stencilOpZFail;
				writeColor = GL_FALSE;
				newDepth = oldDepth;
			} else {
				op = context->stencilOpZPass;
				newDepth = depth;
			}

			switch (op) {
			case GL_KEEP:
				newStencil = oldStencil;
				break;
			case GL_ZERO:
				newStencil = 0;
				break;
			case GL_REPLACE:
				newStencil = context->stencilRef;
				break;
			case GL_INCR:
				newStencil = (oldStencil < stencilBufferMask) ? oldStencil + 1 : oldStencil;
				break;
			case GL_DECR:
				newStencil = (oldStencil > 0) ? oldStencil - 1 : 0;
				break;
			case GL_INVERT:
				newStencil = ~oldStencil & stencilBufferMask;
				break;
			default:
				assert(GL_FALSE);
				break;
			}
		} else if (!depthTest) {
			return;
		} else {
			newStencil = oldStencil;
			newDepth = depth;
		}

		newDepth = VPMT_SELECT(context->depthWriteMask, newDepth, oldDepth);
		newStencil =
			((context->stencilWriteMask & newStencil) |
			 (~context->stencilWriteMask & oldStencil)) & stencilBufferMask;

		VPMT_FrameWriteDepth(fb, newDepth);
		VPMT_FrameWriteStencil(fb, newStencil);
	}

	/* is there any color component that needs to be written? */
	hasEnabledColor =
		(context->redBits && context->colorWriteMask[0]) ||
		(context->greenBits && context->colorWriteMask[1]) ||
		(context->blueBits && context->colorWriteMask[2]) ||
		(context->alphaBits && context->colorWriteMask[3]);

	if (!hasEnabledColor || !writeColor) {
		return;
	}

	/* is there any color component that needs to be preserved? */
	hasMaskedColor =
		(context->redBits && !context->colorWriteMask[0]) ||
		(context->greenBits && !context->colorWriteMask[1]) ||
		(context->blueBits && !context->colorWriteMask[2]) ||
		(context->alphaBits && !context->colorWriteMask[3]);

	/* perform blend */
	if ((context->blendEnabled && context->blendSrcFactor != GL_ONE) || hasMaskedColor) {

		oldColor = VPMT_FrameReadColor(fb);

		if (context->blendEnabled) {
			/* OpenGL SC only provides the following two blend modes */
			if (context->blendSrcFactor == GL_SRC_ALPHA
				&& context->blendDstFactor == GL_ONE_MINUS_SRC_ALPHA) {
				GLuint alpha256 = VPMT_Color255To256(newColor.alpha);

				newColor.red = UByteLerp(oldColor.red, newColor.red, alpha256);
				newColor.green = UByteLerp(oldColor.green, newColor.green, alpha256);
				newColor.blue = UByteLerp(oldColor.blue, newColor.blue, alpha256);
				newColor.alpha = UByteLerp(oldColor.alpha, newColor.alpha, alpha256);

			} else if (context->blendSrcFactor == GL_SRC_ALPHA_SATURATE
					   && context->blendDstFactor == GL_ONE) {
				GLubyte alpha = VPMT_MIN(newColor.alpha, 0xffu - oldColor.alpha);
				GLuint alpha256 = VPMT_Color255To256(alpha);

				newColor.red = (GLubyte) VPMT_MIN(VPMT_UBYTE_MAX, (GLushort) oldColor.red + VPMT_UByteMul256(newColor.red, alpha256));
				newColor.green = (GLubyte) VPMT_MIN(VPMT_UBYTE_MAX, (GLushort) oldColor.green + VPMT_UByteMul256(newColor.green, alpha256));
				newColor.blue = (GLubyte) VPMT_MIN(VPMT_UBYTE_MAX, (GLushort) oldColor.blue + VPMT_UByteMul256(newColor.blue, alpha256));
				newColor.alpha = oldColor.alpha + alpha;	   /* this is clamped at 0..1 */
			} else {
				assert(context->blendSrcFactor == GL_ONE && context->blendDstFactor == GL_ZERO);
			}
		}

		/* multiplex color components based on colorWriteMask */
		newColor.red = VPMT_SELECT(context->colorWriteMask[0], newColor.red, oldColor.red);
		newColor.green = VPMT_SELECT(context->colorWriteMask[1], newColor.green, oldColor.green);
		newColor.blue = VPMT_SELECT(context->colorWriteMask[2], newColor.blue, oldColor.blue);
		newColor.alpha = VPMT_SELECT(context->colorWriteMask[3], newColor.alpha, oldColor.alpha);
	}

	/* write to framebuffer */
	VPMT_FrameWriteColor(fb, newColor);
}

/* $Id: fragproc.c 74 2008-11-23 07:25:12Z hmwill $ */
