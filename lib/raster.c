/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Rasterization state functions.
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

/*
** -------------------------------------------------------------------------
** Exported API entry points - Points
** -------------------------------------------------------------------------
*/

void VPMT_ExecPointSize(VPMT_Context * context, GLfloat size)
{
	VPMT_NOT_RENDERING(context);

	if (size <= 0.0f) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	context->pointSize = size;
}

/*
** -------------------------------------------------------------------------
** Exported API entry points - Lines
** -------------------------------------------------------------------------
*/

void VPMT_ExecLineStipple(VPMT_Context * context, GLint factor, GLushort pattern)
{
	VPMT_NOT_RENDERING(context);

	context->lineStippleRepeat = factor < 1 ? 1 : factor > 256 ? 256 : factor;
	context->lineStipplePattern = pattern & 0xffff;
}

void VPMT_ExecLineWidth(VPMT_Context * context, GLfloat width)
{
	VPMT_NOT_RENDERING(context);

	if (width <= 0.0f) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	context->lineWidth = width;
}

/*
** -------------------------------------------------------------------------
** Exported API entry points - Polygons
** -------------------------------------------------------------------------
*/

void VPMT_ExecGetPolygonStipple(VPMT_Context * context, GLubyte * mask)
{
	GLsizei row, column;
	GLubyte *src;

	VPMT_NOT_RENDERING(context);

	if (!mask) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	src = context->polygonStipple.bytes;

	for (row = 0; row < 32; ++row) {
		for (column = 0; column < 4; ++column) {
			*mask++ = *src++;
		}

		if (context->packAlignment == 8) {
			mask += 4;
		}
	}
}

void VPMT_ExecPolygonOffset(VPMT_Context * context, GLfloat factor, GLfloat units)
{
	VPMT_NOT_RENDERING(context);

	context->polygonOffsetFactor = factor;
	context->polygonOffsetUnit = units;
}

void VPMT_ExecPolygonStipple(VPMT_Context * context, const GLubyte * mask)
{
	GLsizei row, column;
	GLubyte *dest;
	VPMT_NOT_RENDERING(context);

	if (!mask) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	dest = context->polygonStipple.bytes;

	for (row = 0; row < 32; ++row) {
		for (column = 0; column < 4; ++column) {
			*dest++ = *mask++;
		}

		if (context->unpackAlignment == 8) {
			mask += 4;
		}
	}
}

void VPMT_RasterPrepareInterpolants(VPMT_Context * context)
{
	GLsizei index;
	GLuint rasterInterpolants = 0u;
	GLboolean needsInvW = GL_FALSE;
	GLboolean needsColor = GL_TRUE;

	if (context->depthTestEnabled || (context->depthBits != 0 && context->depthWriteMask)) {
		rasterInterpolants |= VPMT_RasterInterpolateDepth;
	}

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (context->texUnits[index].enabled &&
			context->texUnits[index].boundTexture && !context->texUnits[index].incomplete) {
			rasterInterpolants |= VPMT_RasterInterpolateTexCoord0 << index;

			if (VPMT_Texture2DIsMipmap(context->texUnits[index].boundTexture)) {
				rasterInterpolants |= VPMT_RasterInterpolateRho0 << index;
			}

			needsInvW = GL_TRUE;
			needsColor &= VPMT_TexImageUnitUsesBaseColor(&context->texUnits[index]);
		}
	}

	if (needsInvW) {
		rasterInterpolants |= VPMT_RasterInterpolateInvW;
	}

	if (needsColor) {
		rasterInterpolants |= VPMT_RasterInterpolateColor;
	}

	context->rasterInterpolants = rasterInterpolants;
}

/* $Id: raster.c 74 2008-11-23 07:25:12Z hmwill $ */
