/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Texture State Functions
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_TEX_H
#define VPMT_TEX_H

#include "image.h"

typedef struct VPMT_Texture2D {
	GLuint name;
	VPMT_Image2D *mipmaps[VPMT_MAX_MIPMAP_LEVEL + 1];

#if GL_EXT_paletted_texture
	VPMT_Image1D *palette;									   /* a palette is an image of height 1 */
#endif

	GLenum texWrapS;
	GLenum texWrapT;
	GLenum texMinFilter;
	GLenum texMagFilter;
	GLsizei maxMipmapLevel;
	GLboolean complete;
	GLboolean validated;
} VPMT_Texture2D;

VPMT_Texture2D *VPMT_Texture2DAllocate(GLuint name);
void VPMT_Texture2DDeallocate(VPMT_Texture2D * texture);

void VPMT_Texture2DValidate(VPMT_Texture2D * texture);
GLboolean VPMT_ValidateTextureFormat(GLenum format);
GLboolean VPMT_ValidateInternalFormat(GLenum format);
GLboolean VPMT_ValidateNonIndexedTextureFormat(GLenum format);

GLboolean VPMT_Texture2DIsMipmap(const VPMT_Texture2D * texture);

#endif

/* $Id: tex.h 74 2008-11-23 07:25:12Z hmwill $ */
