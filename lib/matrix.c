/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Matrix manipulation functions.
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

void VPMT_ExecFrustumf(VPMT_Context * context, GLfloat left, GLfloat right, GLfloat bottom,
					   GLfloat top, GLfloat zNear, GLfloat zFar)
{
	VPMT_Matrix frustum;

	VPMT_NOT_RENDERING(context);

	if (left == right || top == bottom || zNear == zFar) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_MatrixFrustumf(frustum, left, right, bottom, top, zNear, zFar);
	VPMT_MatrixStackMultMatrixf(context->currentMatrixStack, frustum);
}

void VPMT_ExecLoadIdentity(VPMT_Context * context)
{
	VPMT_NOT_RENDERING(context);

	VPMT_MatrixStackLoadIdentity(context->currentMatrixStack);
}

void VPMT_ExecLoadMatrixf(VPMT_Context * context, const GLfloat * m)
{
	VPMT_NOT_RENDERING(context);

	if (!m) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_MatrixStackLoadMatrixf(context->currentMatrixStack, m);
}

void VPMT_ExecMatrixMode(VPMT_Context * context, GLenum mode)
{
	VPMT_NOT_RENDERING(context);

	switch (mode) {
	case GL_MODELVIEW:
		context->currentMatrixStack = &context->modelviewMatrixStack;
		break;

	case GL_PROJECTION:
		context->currentMatrixStack = &context->projectionMatrixStack;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}

	context->matrixMode = mode;
}

void VPMT_ExecMultMatrixf(VPMT_Context * context, const GLfloat * m)
{
	VPMT_NOT_RENDERING(context);

	VPMT_MatrixStackMultMatrixf(context->currentMatrixStack, m);
}

void VPMT_ExecOrthof(VPMT_Context * context, GLfloat left, GLfloat right, GLfloat bottom,
					 GLfloat top, GLfloat zNear, GLfloat zFar)
{
	VPMT_Matrix ortho;

	VPMT_NOT_RENDERING(context);

	if (left == right || top == bottom || zNear == zFar) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_MatrixOrthof(ortho, left, right, bottom, top, zNear, zFar);
	VPMT_MatrixStackMultMatrixf(context->currentMatrixStack, ortho);
}

void VPMT_ExecPopMatrix(VPMT_Context * context)
{
	VPMT_NOT_RENDERING(context);

	if (!VPMT_MatrixStackPopMatrix(context->currentMatrixStack)) {
		VPMT_STACK_UNDERFLOW(context);
		return;
	}
}

void VPMT_ExecPushMatrix(VPMT_Context * context)
{
	VPMT_NOT_RENDERING(context);

	if (!VPMT_MatrixStackPushMatrix(context->currentMatrixStack)) {
		VPMT_STACK_OVERFLOW(context);
		return;
	}
}

void VPMT_ExecRotatef(VPMT_Context * context, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Matrix rotate;

	VPMT_NOT_RENDERING(context);

	VPMT_MatrixRotatef(rotate, angle, x, y, z);
	VPMT_MatrixStackMultMatrixf(context->currentMatrixStack, rotate);
}

void VPMT_ExecScalef(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Matrix scale;

	VPMT_NOT_RENDERING(context);

	VPMT_MatrixScalef(scale, x, y, z);
	VPMT_MatrixStackMultMatrixf(context->currentMatrixStack, scale);
}

void VPMT_ExecTranslatef(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Matrix translate;

	VPMT_NOT_RENDERING(context);

	VPMT_MatrixTranslatef(translate, x, y, z);
	VPMT_MatrixStackMultMatrixf(context->currentMatrixStack, translate);
}

/*
** -------------------------------------------------------------------------
** Internal functions
** -------------------------------------------------------------------------
*/

void VPMT_MatrixStackInitialize(VPMT_MatrixStack * stack, VPMT_Matrix * base, GLsizei size)
{
	stack->base = base;
	stack->size = size;
	stack->current = 1;

	/* initialize current matrix to identity */

	VPMT_MatrixStackLoadIdentity(stack);
}

void VPMT_MatrixStackLoadIdentity(VPMT_MatrixStack * stack)
{
	GLfloat *matrix;

	assert(stack);
	assert(stack->current > 0);
	assert(stack->current <= stack->size);

	matrix = stack->base[stack->current - 1];

	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;

	matrix[1] = matrix[2] = matrix[3] =
		matrix[4] = matrix[6] = matrix[7] =
		matrix[8] = matrix[9] = matrix[11] = matrix[12] = matrix[13] = matrix[14] = 0.0f;
}

void VPMT_MatrixStackLoadMatrixf(VPMT_MatrixStack * stack, const GLfloat * matrix)
{
	GLfloat *target;
	GLsizei index;

	assert(stack);
	assert(stack->current > 0);
	assert(stack->current <= stack->size);

	target = stack->base[stack->current - 1];

	for (index = 0; index < 16; ++index)
		*target++ = *matrix++;
}

static void VPMT_MatrixMultiplyf(GLfloat * dest, const GLfloat * left, const GLfloat * right)
{
	GLsizei row, column;

	for (column = 0; column < 16; column += 4) {
		for (row = 0; row < 4; ++row) {
			dest[row + column] =
				left[row] * right[column] +
				left[row + 4] * right[column + 1] +
				left[row + 8] * right[column + 2] + left[row + 12] * right[column + 3];
		}
	}
}

void VPMT_MatrixStackMultMatrixf(VPMT_MatrixStack * stack, const GLfloat * matrix)
{
	GLfloat *target;
	GLsizei index;
	VPMT_Matrix temp;

	assert(stack->current > 0);
	assert(stack->current <= stack->size);

	target = stack->base[stack->current - 1];

	VPMT_MatrixMultiplyf(temp, target, matrix);

	for (index = 0; index < 16; ++index)
		*target++ = temp[index];
}

GLboolean VPMT_MatrixStackPushMatrix(VPMT_MatrixStack * stack)
{
	if (stack->current < stack->size) {
		memcpy(&stack->base[stack->current], &stack->base[stack->current - 1], sizeof(VPMT_Matrix));
		++stack->current;
		return GL_TRUE;
	} else {
		return GL_FALSE;
	}
}

GLboolean VPMT_MatrixStackPopMatrix(VPMT_MatrixStack * stack)
{
	if (stack->current > 1) {
		--stack->current;
		return GL_TRUE;
	} else {
		return GL_FALSE;
	}
}

#define ELEM(m, row, column) (m[row + column * 4])

void VPMT_MatrixFrustumf(GLfloat * frustum, GLfloat left, GLfloat right, GLfloat bottom,
						 GLfloat top, GLfloat zNear, GLfloat zFar)
{
	frustum[1] = frustum[2] = frustum[3] = 0.0f;
	frustum[4] = frustum[6] = frustum[7] = 0.0f;
	frustum[12] = frustum[13] = frustum[15] = 0.0f;

	frustum[0] = (2.0f * zNear) / (right - left);
	frustum[5] = (2.0f * zNear) / (top - bottom);
	frustum[8] = (right + left) / (right - left);
	frustum[9] = (top + bottom) / (top - bottom);
	frustum[10] = (zFar + zNear) / (zNear - zFar);
	frustum[11] = -1.0f;
	frustum[14] = (2.0f * zFar * zNear) / (zNear - zFar);
}

void VPMT_MatrixOrthof(GLfloat * ortho, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
					   GLfloat zNear, GLfloat zFar)
{
	ortho[1] = ortho[2] = ortho[3] = 0.0f;
	ortho[4] = ortho[6] = ortho[7] = 0.0f;
	ortho[8] = ortho[9] = ortho[11] = 0.0f;

	ortho[0] = 2.0f / (right - left);
	ortho[5] = 2.0f / (top - bottom);
	ortho[10] = 2.0f / (zNear - zFar);

	ortho[12] = (right + left) / (left - right);
	ortho[13] = (top + bottom) / (bottom - top);
	ortho[14] = (zFar + zNear) / (zNear - zFar);
	ortho[15] = 1.0f;
}

void VPMT_MatrixRotatef(GLfloat * rotate, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	GLfloat ar = angle * (3.14159265358979323846f / 180.f);
	GLfloat s = VPMT_SINF(ar), c = VPMT_COSF(ar);
	GLfloat sqrLength = (x * x + y * y + z * z);

	/* no check for 0 for sqrLength??? */

	GLfloat invLength = (1.0f / sqrtf(sqrLength));
	GLfloat scaledX = x * invLength;
	GLfloat scaledY = y * invLength;
	GLfloat scaledZ = z * invLength;

	GLfloat c1 = 1.0f - c;

	rotate[0] = scaledX * scaledX * c1 + c;
	rotate[1] = scaledY * scaledX * c1 + s * scaledZ;
	rotate[2] = scaledZ * scaledX * c1 - s * scaledY;
	rotate[3] = 0.0f;
	rotate[4] = scaledX * scaledY * c1 - s * scaledZ;
	rotate[5] = scaledY * scaledY * c1 + c;
	rotate[6] = scaledZ * scaledY * c1 + s * scaledX;
	rotate[7] = 0.0f;
	rotate[8] = scaledX * scaledZ * c1 + s * scaledY;
	rotate[9] = scaledY * scaledZ * c1 - s * scaledX;
	rotate[10] = scaledZ * scaledZ * c1 + c;
	rotate[11] = 0.0f;
	rotate[12] = rotate[13] = rotate[14] = 0.0f;
	rotate[15] = 1.0f;
}

void VPMT_MatrixScalef(GLfloat * scale, GLfloat x, GLfloat y, GLfloat z)
{
	scale[0] = x;
	scale[5] = y;
	scale[10] = z;
	scale[15] = 1.0f;

	scale[1] = scale[2] = scale[3] =
		scale[4] = scale[6] = scale[7] =
		scale[8] = scale[9] = scale[11] = scale[12] = scale[13] = scale[14] = 0.0f;
}

void VPMT_MatrixTranslatef(GLfloat * translate, GLfloat x, GLfloat y, GLfloat z)
{
	translate[0] = translate[5] = translate[10] = translate[15] = 1.0f;

	translate[1] = translate[2] = translate[3] =
		translate[4] = translate[6] = translate[7] =
		translate[8] = translate[9] = translate[11] = 0.0f;

	translate[12] = x;
	translate[13] = y;
	translate[14] = z;
}

void VPMT_MatrixInverse3(GLfloat * result, const GLfloat * matrix)
{
	GLfloat det =
		ELEM(matrix, 0, 0) * ELEM(matrix, 1, 1) * ELEM(matrix, 2, 2) +
		ELEM(matrix, 0, 1) * ELEM(matrix, 1, 2) * ELEM(matrix, 2, 0) +
		ELEM(matrix, 0, 2) * ELEM(matrix, 1, 0) * ELEM(matrix, 2, 1) -
		ELEM(matrix, 0, 0) * ELEM(matrix, 1, 2) * ELEM(matrix, 2, 1) -
		ELEM(matrix, 0, 1) * ELEM(matrix, 1, 0) * ELEM(matrix, 2, 2) -
		ELEM(matrix, 0, 2) * ELEM(matrix, 1, 1) * ELEM(matrix, 2, 0);

	GLfloat invDet = 1.0f / det;

	ELEM(result, 0, 0) =
		(ELEM(matrix, 1, 1) * ELEM(matrix, 2, 2) -
		 ELEM(matrix, 1, 2) * ELEM(matrix, 2, 1)) * invDet;
	ELEM(result, 1, 0) =
		(ELEM(matrix, 0, 2) * ELEM(matrix, 2, 1) -
		 ELEM(matrix, 0, 1) * ELEM(matrix, 2, 2)) * invDet;
	ELEM(result, 2, 0) =
		(ELEM(matrix, 0, 1) * ELEM(matrix, 1, 2) -
		 ELEM(matrix, 0, 2) * ELEM(matrix, 1, 1)) * invDet;

	ELEM(result, 0, 1) =
		(ELEM(matrix, 2, 0) * ELEM(matrix, 1, 2) -
		 ELEM(matrix, 1, 0) * ELEM(matrix, 2, 2)) * invDet;
	ELEM(result, 1, 1) =
		(ELEM(matrix, 0, 0) * ELEM(matrix, 2, 2) -
		 ELEM(matrix, 2, 0) * ELEM(matrix, 0, 2)) * invDet;
	ELEM(result, 2, 1) =
		(ELEM(matrix, 1, 0) * ELEM(matrix, 0, 2) -
		 ELEM(matrix, 0, 0) * ELEM(matrix, 1, 2)) * invDet;

	ELEM(result, 0, 2) =
		(ELEM(matrix, 1, 0) * ELEM(matrix, 2, 1) -
		 ELEM(matrix, 2, 0) * ELEM(matrix, 1, 1)) * invDet;
	ELEM(result, 1, 2) =
		(ELEM(matrix, 2, 0) * ELEM(matrix, 0, 1) -
		 ELEM(matrix, 0, 0) * ELEM(matrix, 2, 1)) * invDet;
	ELEM(result, 2, 2) =
		(ELEM(matrix, 0, 0) * ELEM(matrix, 1, 1) -
		 ELEM(matrix, 1, 0) * ELEM(matrix, 0, 1)) * invDet;

	ELEM(result, 3, 0) = ELEM(result, 3, 1) = ELEM(result, 3, 2) =
		ELEM(result, 0, 3) = ELEM(result, 1, 3) = ELEM(result, 2, 3) = 0.0f;

	ELEM(result, 3, 3) = 1.0f;
}

void VPMT_MatrixTransform4x3(const GLfloat * matrix, GLfloat * result, const GLfloat * vector)
{
	result[0] =
		ELEM(matrix, 0, 0) * vector[0] + ELEM(matrix, 0, 1) * vector[1] + ELEM(matrix, 0,
																			   2) * vector[2] +
		ELEM(matrix, 0, 3);
	result[1] =
		ELEM(matrix, 1, 0) * vector[0] + ELEM(matrix, 1, 1) * vector[1] + ELEM(matrix, 1,
																			   2) * vector[2] +
		ELEM(matrix, 1, 3);
	result[2] =
		ELEM(matrix, 2, 0) * vector[0] + ELEM(matrix, 2, 1) * vector[1] + ELEM(matrix, 2,
																			   2) * vector[2] +
		ELEM(matrix, 2, 3);
	result[3] =
		ELEM(matrix, 3, 0) * vector[0] + ELEM(matrix, 3, 1) * vector[1] + ELEM(matrix, 3,
																			   2) * vector[2] +
		ELEM(matrix, 3, 3);
}

void VPMT_MatrixTransform3x3(const GLfloat * matrix, GLfloat * result, const GLfloat * vector)
{
	result[0] =
		ELEM(matrix, 0, 0) * vector[0] + ELEM(matrix, 0, 1) * vector[1] + ELEM(matrix, 0,
																			   2) * vector[2];
	result[1] =
		ELEM(matrix, 1, 0) * vector[0] + ELEM(matrix, 1, 1) * vector[1] + ELEM(matrix, 1,
																			   2) * vector[2];
	result[2] =
		ELEM(matrix, 2, 0) * vector[0] + ELEM(matrix, 2, 1) * vector[1] + ELEM(matrix, 2,
																			   2) * vector[2];
}

void VPMT_MatrixTransform4x4(const GLfloat * matrix, GLfloat * result, const GLfloat * vector)
{
	result[0] =
		ELEM(matrix, 0, 0) * vector[0] + ELEM(matrix, 0, 1) * vector[1] + ELEM(matrix, 0,
																			   2) * vector[2] +
		ELEM(matrix, 0, 3) * vector[3];
	result[1] =
		ELEM(matrix, 1, 0) * vector[0] + ELEM(matrix, 1, 1) * vector[1] + ELEM(matrix, 1,
																			   2) * vector[2] +
		ELEM(matrix, 1, 3) * vector[3];
	result[2] =
		ELEM(matrix, 2, 0) * vector[0] + ELEM(matrix, 2, 1) * vector[1] + ELEM(matrix, 2,
																			   2) * vector[2] +
		ELEM(matrix, 2, 3) * vector[3];
	result[3] =
		ELEM(matrix, 3, 0) * vector[0] + ELEM(matrix, 3, 1) * vector[1] + ELEM(matrix, 3,
																			   2) * vector[2] +
		ELEM(matrix, 3, 3) * vector[3];
}

/* $Id: matrix.c 74 2008-11-23 07:25:12Z hmwill $ */
