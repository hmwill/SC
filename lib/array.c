/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Vertex array functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "dispatch.h"

/*
** -------------------------------------------------------------------------
** Forward declarations
** -------------------------------------------------------------------------
*/

static void ToggleClientState(VPMT_Context * context, GLenum array, GLboolean enable);

/*
** --------------------------------------------------------------------------
** Fetch functions for individual array elements based on type
** --------------------------------------------------------------------------
*/

static void FetchByte(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLbyte *values = (const GLbyte *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = values[3];

	case 3:
		result[2] = values[2];

	case 2:
		result[1] = values[1];

	case 1:
		result[0] = values[0];
	}
}

static void FetchUnsignedByte(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLubyte *values = (const GLubyte *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = values[3];

	case 3:
		result[2] = values[2];

	case 2:
		result[1] = values[1];

	case 1:
		result[0] = values[0];
	}
}

static void FetchShort(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLshort *values = (const GLshort *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = values[3];

	case 3:
		result[2] = values[2];

	case 2:
		result[1] = values[1];

	case 1:
		result[0] = values[0];
	}
}

static void FetchUnsignedShort(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLushort *values = (const GLushort *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = values[3];

	case 3:
		result[2] = values[2];

	case 2:
		result[1] = values[1];

	case 1:
		result[0] = values[0];
	}
}

static void FetchInt(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLint *values = (const GLint *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = (GLfloat) values[3];

	case 3:
		result[2] = (GLfloat) values[2];

	case 2:
		result[1] = (GLfloat) values[1];

	case 1:
		result[0] = (GLfloat) values[0];
	}
}

static void FetchUnsignedInt(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLuint *values = (const GLuint *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = (GLfloat) values[3];

	case 3:
		result[2] = (GLfloat) values[2];

	case 2:
		result[1] = (GLfloat) values[1];

	case 1:
		result[0] = (GLfloat) values[0];
	}
}

static void FetchByteNormalize(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLbyte *values = (const GLbyte *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = VPMT_BYTE_TO_FLOAT(values[3]);

	case 3:
		result[2] = VPMT_BYTE_TO_FLOAT(values[2]);

	case 2:
		result[1] = VPMT_BYTE_TO_FLOAT(values[1]);

	case 1:
		result[0] = VPMT_BYTE_TO_FLOAT(values[0]);
	}
}

static void FetchUnsignedByteNormalize(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLubyte *values = (const GLubyte *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = VPMT_UBYTE_TO_FLOAT(values[3]);

	case 3:
		result[2] = VPMT_UBYTE_TO_FLOAT(values[2]);

	case 2:
		result[1] = VPMT_UBYTE_TO_FLOAT(values[1]);

	case 1:
		result[0] = VPMT_UBYTE_TO_FLOAT(values[0]);
	}
}

static void FetchFloat(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	const void *address = (const GLbyte *) array->pointer + index * array->effectiveStride;
	const GLfloat *values = (const GLfloat *) address;

	switch (array->size) {
	default:
	case 4:
		result[3] = values[3];

	case 3:
		result[2] = values[2];

	case 2:
		result[1] = values[1];

	case 1:
		result[0] = values[0];
	}
}

static void FetchError(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	/* nop */
}

static VPMT_INLINE void FetchArray(const VPMT_Array * array, GLsizei index, GLfloat * result)
{
	array->fetch(array, index, result);
}

static void SetArray(VPMT_Array * array, GLint size, GLenum type, GLsizei stride,
					 const GLvoid * pointer)
{

	GLsizei elementSize;

	array->pointer = pointer;
	array->size = size;
	array->stride = stride;
	array->type = type;

	switch (type) {
	case GL_BYTE:
		elementSize = sizeof(GLbyte);
		array->fetch = FetchByteNormalize;
		break;
	case GL_UNSIGNED_BYTE:
		elementSize = sizeof(GLubyte);
		array->fetch = FetchUnsignedByteNormalize;
		break;
	case GL_SHORT:
		elementSize = sizeof(GLshort);
		array->fetch = FetchShort;
		break;
	case GL_UNSIGNED_SHORT:
		elementSize = sizeof(GLushort);
		array->fetch = FetchUnsignedShort;
		break;
	case GL_INT:
		elementSize = sizeof(GLint);
		array->fetch = FetchInt;
		break;
	case GL_UNSIGNED_INT:
		elementSize = sizeof(GLuint);
		array->fetch = FetchUnsignedInt;
		break;
	case GL_FLOAT:
		elementSize = sizeof(GLfloat);
		array->fetch = FetchFloat;
		break;
	default:
		assert(GL_FALSE);
		elementSize = 0;
		array->fetch = FetchError;
	}

	if (stride) {
		array->effectiveStride = stride;
	} else {
		array->effectiveStride = size * elementSize;
	}
}

/*
** -------------------------------------------------------------------------
** Exported API entry points
** -------------------------------------------------------------------------
*/

void VPMT_ExecClientActiveTexture(VPMT_Context * context, GLenum texture)
{
	GLint index;

	if (texture < GL_TEXTURE0 || texture > GL_TEXTURE31) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	index = texture - GL_TEXTURE0;

	if (index >= VPMT_MAX_TEX_UNITS) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);
	context->clientActiveTexture = texture;
	context->clientActiveTextureIndex = index;
}

void VPMT_ExecColorPointer(VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
						   const GLvoid * pointer)
{
	if (type != GL_UNSIGNED_BYTE && type != GL_FLOAT) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (size != 3 && size != 4) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);
	SetArray(&context->colorArray, size, type, stride, pointer);
}

void VPMT_ExecDisableClientState(VPMT_Context * context, GLenum array)
{
	ToggleClientState(context, array, GL_FALSE);
}

void VPMT_ExecArrayElement(VPMT_Context * context, GLint i)
{
	GLsizei index;

	/* dereference color array */
	if (context->colorArray.enabled) {
		VPMT_Vec4 color;
		color[3] = 1.0f;
		FetchArray(&context->colorArray, i, color);
		context->dispatch->Color4fv(context, color);
	}

	/* dereference normal array */
	if (context->normalArray.enabled) {
		VPMT_Vec3 normal;
		FetchArray(&context->normalArray, i, normal);
		context->dispatch->Normal3fv(context, normal);
	}

	/* dereference texture array */
	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (context->texCoordArray[index].enabled) {
			VPMT_Vec2 texCoord;
			FetchArray(&context->texCoordArray[index], i, texCoord);
			context->dispatch->MultiTexCoord2f(context, GL_TEXTURE0 + index, texCoord[0],
											   texCoord[1]);
		}
	}

	/* dereference vertex array */
	if (context->vertexArray.enabled) {
		VPMT_Vec3 vertex;
		vertex[2] = 0.0f;
		FetchArray(&context->vertexArray, i, vertex);
		context->dispatch->Vertex3fv(context, vertex);
	}
}

void VPMT_ExecDrawArrays(VPMT_Context * context, GLenum mode, GLint first, GLsizei count)
{
	if (count < 0) {
		VPMT_INVALID_VALUE(context);
		return;
	} else if (mode != GL_POINTS &&
			   mode != GL_LINES && mode != GL_LINE_LOOP && mode != GL_LINE_STRIP &&
			   mode != GL_TRIANGLES && mode != GL_TRIANGLE_STRIP && mode != GL_TRIANGLE_FAN) {
		VPMT_INVALID_ENUM(context);
		return;
	} else if (context->renderMode != GL_INVALID_MODE) {
		/* cannot nest begin/end */
		VPMT_INVALID_OPERATION(context);
		return;
	}

	context->dispatch->Begin(context, mode);

	while (count > 0) {
		VPMT_ExecArrayElement(context, first++);
		--count;
	}

	context->dispatch->End(context);
}

void VPMT_ExecDrawElements(VPMT_Context * context, GLenum mode, GLsizei count, GLenum type,
						   const GLvoid * indices)
{
	if (count < 0 || !indices) {
		VPMT_INVALID_VALUE(context);
		return;
	} else if ((mode != GL_POINTS &&
				mode != GL_LINES && mode != GL_LINE_LOOP && mode != GL_LINE_STRIP &&
				mode != GL_TRIANGLES && mode != GL_TRIANGLE_STRIP && mode != GL_TRIANGLE_FAN) ||
			   (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)) {
		VPMT_INVALID_ENUM(context);
		return;
	} else if (context->renderMode != GL_INVALID_MODE) {
		/* cannot nest begin/end */
		VPMT_INVALID_OPERATION(context);
		return;
	}

	context->dispatch->Begin(context, mode);

	switch (type) {
	case GL_UNSIGNED_BYTE:
		{
			const GLubyte *ptr = indices;

			while (count > 0) {
				VPMT_ExecArrayElement(context, *ptr++);
				--count;
			}
		}

		break;

	case GL_UNSIGNED_SHORT:
		{
			const GLushort *ptr = indices;

			while (count > 0) {
				VPMT_ExecArrayElement(context, *ptr++);
				--count;
			}
		}

		break;

	case GL_UNSIGNED_INT:
		{
			const GLuint *ptr = indices;

			while (count > 0) {
				VPMT_ExecArrayElement(context, *ptr++);
				--count;
			}
		}

		break;

	default:
		assert(GL_FALSE);
	}

	context->dispatch->End(context);
}

void VPMT_ExecEnableClientState(VPMT_Context * context, GLenum array)
{
	ToggleClientState(context, array, GL_TRUE);
}

void VPMT_ExecGetPointerv(VPMT_Context * context, GLenum pname, GLvoid ** params)
{
	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	switch (pname) {
	case GL_VERTEX_ARRAY_POINTER:
		*params = (void *) context->vertexArray.pointer;
		break;

	case GL_COLOR_ARRAY_POINTER:
		*params = (void *) context->colorArray.pointer;
		break;

	case GL_NORMAL_ARRAY_POINTER:
		*params = (void *) context->normalArray.pointer;
		break;

	case GL_TEXTURE_COORD_ARRAY_POINTER:
		*params = (void *) context->texCoordArray[context->clientActiveTextureIndex].pointer;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_ExecNormalPointer(VPMT_Context * context, GLenum type, GLsizei stride,
							const GLvoid * pointer)
{
	if (type != GL_FLOAT) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	SetArray(&context->normalArray, 3, type, stride, pointer);
}

void VPMT_ExecTexCoordPointer(VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
							  const GLvoid * pointer)
{
	if (type != GL_FLOAT) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (size != 2) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);
	SetArray(&context->texCoordArray[context->clientActiveTextureIndex], size, type, stride,
			 pointer);
}

void VPMT_ExecVertexPointer(VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
							const GLvoid * pointer)
{

	if (type != GL_FLOAT) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (size != 2 && size != 3) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);
	SetArray(&context->vertexArray, size, type, stride, pointer);
}

/*
** -------------------------------------------------------------------------
** Internal functions
** -------------------------------------------------------------------------
*/

void VPMT_ArrayInitialize(VPMT_Array * array)
{
	assert(array);

	array->pointer = 0;
	array->size = 4;
	array->type = GL_FLOAT;
	array->stride = 0;
	array->effectiveStride = sizeof(GLfloat) * 4;
	array->enabled = GL_FALSE;
	array->fetch = FetchError;
}

static void ToggleClientState(VPMT_Context * context, GLenum array, GLboolean enable)
{
	VPMT_NOT_RENDERING(context);

	switch (array) {
	case GL_VERTEX_ARRAY:
		context->vertexArray.enabled = enable;
		break;

	case GL_COLOR_ARRAY:
		context->colorArray.enabled = enable;
		break;

	case GL_NORMAL_ARRAY:
		context->normalArray.enabled = enable;
		break;

	case GL_TEXTURE_COORD_ARRAY:
		context->texCoordArray[context->clientActiveTextureIndex].enabled = enable;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

/* $Id: array.c 74 2008-11-23 07:25:12Z hmwill $ */
