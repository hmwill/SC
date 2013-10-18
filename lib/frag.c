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

/*
** -------------------------------------------------------------------------
** Exported API entry points
** -------------------------------------------------------------------------
*/

void VPMT_ExecAlphaFunc(VPMT_Context * context, GLenum func, GLclampf ref)
{
	VPMT_NOT_RENDERING(context);

	switch (func) {
	case GL_LEQUAL:
	case GL_ALWAYS:
		context->alphaFunc = func;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}

	context->alphaRef = (ref < 0.0f) ? 0.0f : (ref > 1.0f) ? 1.0f : ref;
	context->alphaRefub = (GLubyte) (context->alphaRef * 0xff + 0.5f);
}

void VPMT_ExecBlendFunc(VPMT_Context * context, GLenum sfactor, GLenum dfactor)
{
	VPMT_NOT_RENDERING(context);

	if ((sfactor == GL_SRC_ALPHA && dfactor == GL_ONE_MINUS_SRC_ALPHA) ||
		(sfactor == GL_SRC_ALPHA_SATURATE && dfactor == GL_ONE) ||
		(sfactor == GL_ONE && dfactor == GL_ZERO)) {
		context->blendSrcFactor = sfactor;
		context->blendDstFactor = dfactor;
	} else {
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_ExecClearColor(VPMT_Context * context, GLclampf red, GLclampf green, GLclampf blue,
						 GLclampf alpha)
{
	VPMT_NOT_RENDERING(context);

	context->clearColor[0] = VPMT_CLAMP(red);
	context->clearColor[1] = VPMT_CLAMP(green);
	context->clearColor[2] = VPMT_CLAMP(blue);
	context->clearColor[3] = VPMT_CLAMP(alpha);
}

void VPMT_ExecClearDepthf(VPMT_Context * context, GLclampf depth)
{
	VPMT_NOT_RENDERING(context);

	context->clearDepth = (depth < 0.0f) ? 0.0f : (depth > 1.0f) ? 1.0f : depth;
}

void VPMT_ExecClearStencil(VPMT_Context * context, GLint s)
{
	VPMT_NOT_RENDERING(context);

	context->clearStencil = s &
		(context->stencilBits < 32 ? ((1 << context->stencilBits) - 1) : ~0);
}

void VPMT_ExecColorMask(VPMT_Context * context, GLboolean red, GLboolean green, GLboolean blue,
						GLboolean alpha)
{
	VPMT_NOT_RENDERING(context);

	context->colorWriteMask[0] = red;
	context->colorWriteMask[1] = green;
	context->colorWriteMask[2] = blue;
	context->colorWriteMask[3] = alpha;
}

void VPMT_ExecDepthFunc(VPMT_Context * context, GLenum func)
{
	VPMT_NOT_RENDERING(context);

	switch (func) {
	case GL_LESS:
	case GL_LEQUAL:
	case GL_ALWAYS:
		context->depthFunc = func;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_ExecDepthMask(VPMT_Context * context, GLboolean flag)
{
	VPMT_NOT_RENDERING(context);

	context->depthWriteMask = flag;
}

void VPMT_ExecScissor(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height)
{
	VPMT_NOT_RENDERING(context);

	if (width < 0 || height < 0) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	context->scissor.origin[0] = x;
	context->scissor.origin[1] = y;
	context->scissor.size.width = width;
	context->scissor.size.height = height;
}

void VPMT_ExecStencilFunc(VPMT_Context * context, GLenum func, GLint ref, GLuint mask)
{
	VPMT_NOT_RENDERING(context);

	switch (func) {
	case GL_NEVER:
	case GL_LESS:
	case GL_LEQUAL:
	case GL_EQUAL:
	case GL_GEQUAL:
	case GL_GREATER:
	case GL_NOTEQUAL:
	case GL_ALWAYS:
		context->stencilFunc = func;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}

	context->stencilRef = ref;
	context->stencilMask = mask;
}

void VPMT_ExecStencilMask(VPMT_Context * context, GLuint mask)
{
	VPMT_NOT_RENDERING(context);

	context->stencilWriteMask = mask;
}

static GLboolean IsValidStencilOp(GLenum op)
{
	switch (op) {
	case GL_KEEP:
	case GL_ZERO:
	case GL_REPLACE:
	case GL_INCR:
	case GL_DECR:
	case GL_INVERT:
		return GL_TRUE;

	default:
		return GL_FALSE;
	}
}

void VPMT_ExecStencilOp(VPMT_Context * context, GLenum fail, GLenum zfail, GLenum zpass)
{
	VPMT_NOT_RENDERING(context);

	if (!IsValidStencilOp(fail) || !IsValidStencilOp(zfail) || !IsValidStencilOp(zpass)) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	context->stencilOpFail = fail;
	context->stencilOpZFail = zfail;
	context->stencilOpZPass = zpass;
}

/*
** -------------------------------------------------------------------------
** Update the effective pixel ownership test
** -------------------------------------------------------------------------
*/

void VPMT_UpdateActiveSurfaceRect(VPMT_Context * context, const VPMT_Rect * rect)
{
	const VPMT_Surface *surface = context->writeSurface;

	if (!rect) {
		/* effectively disable all rendering */
		context->activeSurfaceRect.origin[0] = 0;
		context->activeSurfaceRect.origin[1] = 0;
		context->activeSurfaceRect.size.width = 0;
		context->activeSurfaceRect.size.height = 0;
	} else {
		if (context->scissorTestEnabled) {
			VPMT_IntersectRect(&context->activeSurfaceRect, &context->scissor, rect);
		} else {
			context->activeSurfaceRect = *rect;
		}
	}
}

/* $Id: frag.c 74 2008-11-23 07:25:12Z hmwill $ */
