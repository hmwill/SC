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

#ifndef VPMT_MATRIX_H
#define VPMT_MATRIX_H

#include "util.h"

typedef GLfloat VPMT_Matrix[16];

void VPMT_MatrixFrustumf(GLfloat * frustum, GLfloat left, GLfloat right, GLfloat bottom,
						 GLfloat top, GLfloat zNear, GLfloat zFar);
void VPMT_MatrixOrthof(GLfloat * ortho, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
					   GLfloat zNear, GLfloat zFar);
void VPMT_MatrixRotatef(GLfloat * rotate, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void VPMT_MatrixScalef(GLfloat * scale, GLfloat x, GLfloat y, GLfloat z);
void VPMT_MatrixTranslatef(GLfloat * translate, GLfloat x, GLfloat y, GLfloat z);
void VPMT_MatrixInverse3(GLfloat * result, const GLfloat * matrix);

void VPMT_MatrixTransform4x3(const GLfloat * matrix, GLfloat * result, const GLfloat * vector);
void VPMT_MatrixTransform3x3(const GLfloat * matrix, GLfloat * result, const GLfloat * vector);
void VPMT_MatrixTransform4x4(const GLfloat * matrix, GLfloat * result, const GLfloat * vector);


/*
** Stack of transformation matrices data structure
*/
typedef struct VPMT_MatrixStack {
	VPMT_Matrix *base;										   /* pointer to base of stack array   */
	GLsizei size;											   /* max. number of stack entries     */
	GLsizei current;										   /* pointer to current top of stack  */
} VPMT_MatrixStack;

void VPMT_MatrixStackInitialize(VPMT_MatrixStack * stack, VPMT_Matrix * base, GLsizei size);
void VPMT_MatrixStackLoadIdentity(VPMT_MatrixStack * stack);
void VPMT_MatrixStackLoadMatrixf(VPMT_MatrixStack * stack, const GLfloat * matrix);
void VPMT_MatrixStackMultMatrixf(VPMT_MatrixStack * stack, const GLfloat * matrix);
GLboolean VPMT_MatrixStackPushMatrix(VPMT_MatrixStack * stack);
GLboolean VPMT_MatrixStackPopMatrix(VPMT_MatrixStack * stack);

#endif

/* $Id: matrix.h 74 2008-11-23 07:25:12Z hmwill $ */
