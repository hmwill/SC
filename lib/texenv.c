/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Texture Environment Functions
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"

/*
** -------------------------------------------------------------------------
** Exported API entry points
** -------------------------------------------------------------------------
*/

void VPMT_ExecActiveTexture(VPMT_Context * context, GLenum texture)
{
	GLint index;

	VPMT_NOT_RENDERING(context);

	if (texture < GL_TEXTURE0 || texture > GL_TEXTURE31) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	index = texture - GL_TEXTURE0;

	if (index >= VPMT_MAX_TEX_UNITS) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	context->activeTexture = texture;
	context->activeTextureIndex = index;
}

void VPMT_ExecBindTexture(VPMT_Context * context, GLenum target, GLuint name)
{
	VPMT_Texture2D *texture;

	VPMT_NOT_RENDERING(context);

	texture = VPMT_HashTableFind(&context->textures, name);

	if (!texture) {
		texture = VPMT_Texture2DAllocate(name);

		if (!texture) {
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		if (!VPMT_HashTableInsert(&context->textures, name, texture)) {
			VPMT_Texture2DDeallocate(texture);
			VPMT_OUT_OF_MEMORY(context);
			return;
		}
	}

	context->texUnits[context->activeTextureIndex].boundTexture = texture;
	context->texUnits[context->activeTextureIndex].boundTexture2D = texture->name;
}

void VPMT_ExecGetTexEnvfv(VPMT_Context * context, GLenum target, GLenum pname, GLfloat * params)
{
	VPMT_NOT_RENDERING(context);

	if (target != GL_TEXTURE_ENV) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (pname) {
	case GL_TEXTURE_ENV_COLOR:
		VPMT_Vec4Copy(params, context->texUnits[context->activeTextureIndex].envColor);
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_ExecGetTexEnviv(VPMT_Context * context, GLenum target, GLenum pname, GLint * params)
{
	VPMT_NOT_RENDERING(context);

	if (target != GL_TEXTURE_ENV) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (pname) {
	case GL_TEXTURE_ENV_MODE:
		*params = context->texUnits[context->activeTextureIndex].envMode;
		break;

	default:
		VPMT_INVALID_VALUE(context);
		return;
	}
}

void VPMT_ExecTexEnvfv(VPMT_Context * context, GLenum target, GLenum pname, const GLfloat * params)
{
	VPMT_NOT_RENDERING(context);

	if (target != GL_TEXTURE_ENV) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (pname) {
	case GL_TEXTURE_ENV_COLOR:
		VPMT_Vec4Copy(context->texUnits[context->activeTextureIndex].envColor, params);
		context->texUnits[context->activeTextureIndex].envColor4us =
			VPMT_ConvertVec4ToColor4us(context->texUnits[context->activeTextureIndex].envColor);
		break;

	default:
		VPMT_INVALID_VALUE(context);
		return;
	}
}

void VPMT_ExecTexEnvi(VPMT_Context * context, GLenum target, GLenum pname, GLint param)
{
	VPMT_NOT_RENDERING(context);

	if (target != GL_TEXTURE_ENV) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	switch (pname) {
	case GL_TEXTURE_ENV_MODE:
		if (param != GL_MODULATE && param != GL_REPLACE && param != GL_DECAL &&
			param != GL_BLEND && param != GL_ADD) {
			VPMT_INVALID_VALUE(context);
			return;
		}

		context->texUnits[context->activeTextureIndex].envMode = param;
		break;

	default:
		VPMT_INVALID_VALUE(context);
		return;
	}
}

/* $Id: texenv.c 74 2008-11-23 07:25:12Z hmwill $ */
