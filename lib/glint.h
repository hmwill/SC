/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Internal GL constants
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_GLINT_H
#define VPMT_GLINT_H

/* BeginMode */
#define GL_INVALID_MODE					  0xFF

/* Re-introduce BGRA texture format */
#define GL_BGRA                           0x80E1

/* Re-introduce 32-bit word encoding of 32-bit RGBA values */
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034

/* Functions */

GLAPI void APIENTRY glDeleteLists (GLuint list, GLsizei range);
GLAPI void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures);

#endif

/* $Id$ */
