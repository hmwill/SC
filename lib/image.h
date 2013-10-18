/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Image and Pixel Surface Representation
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_IMAGE_H
#define VPMT_IMAGE_H

#include "glint.h"
#include "util.h"

typedef struct VPMT_Image2D VPMT_Image2D;
typedef VPMT_Image2D VPMT_Image1D;

typedef VPMT_Color4ub(*VPMT_Image2DReadFunc) (const VPMT_Image2D * image,
#if GL_EXT_paletted_texture
											  const VPMT_Image1D * palette,
#endif
											  GLuint x, GLuint y);
typedef void (*VPMT_Image2DWriteFunc) (const VPMT_Image2D * image,
									   GLuint x, GLuint y, VPMT_Color4ub rgba);

typedef struct VPMT_PixelFormat {
	GLenum internalFormat;
	GLenum baseFormat;
	GLenum type;
	GLsizei size;
	GLsizei bits;
	GLsizei redBits;
	GLsizei greenBits;
	GLsizei blueBits;
	GLsizei alphaBits;
	GLuint redMask;
	GLuint blueMask;
	GLuint greenMask;
	GLuint alphaMask;
	VPMT_Image2DReadFunc read;
	VPMT_Image2DWriteFunc write;
} VPMT_PixelFormat;

const VPMT_PixelFormat *VPMT_GetPixelFormat(GLenum format, GLenum type);

typedef enum {
	VPMT_DEPTH_16,
	VPMT_DEPTH_24_STENCIL_8,
	VPMT_DEPTH_32
} VPMT_DepthStencilType;

typedef struct VPMT_DepthStencilFormat {
	VPMT_DepthStencilType type;
	GLsizei bits;
	GLsizei depthBits;
	GLsizei stencilBits;
} VPMT_DepthStencilFormat;

const VPMT_DepthStencilFormat *VPMT_GetDepthStencilFormat(VPMT_DepthStencilType type);

struct VPMT_Image2D {
	void *data;
	VPMT_Size size;
	VPMT_Sizef invSize2;
	GLsizei pitch;
	const VPMT_PixelFormat *pixelFormat;
};

VPMT_Image2D *VPMT_Image2DAllocate(const VPMT_PixelFormat * pixelFormat, GLushort width,
								   GLushort height);
void VPMT_Image2DDeallocate(VPMT_Image2D * image);
void VPMT_Image2DInit(VPMT_Image2D * image, const VPMT_PixelFormat * pixelFormat, GLsizei pitch,
					  GLushort width, GLushort height, void *data);

static VPMT_INLINE VPMT_Image1D *VPMT_Image1DAllocate(const VPMT_PixelFormat * pixelFormat,
													  GLushort width)
{
	return (VPMT_Image1D *) VPMT_Image2DAllocate(pixelFormat, width, 1);
}

static VPMT_INLINE void VPMT_Image1DDeallocate(VPMT_Image1D * image)
{
	VPMT_Image2DDeallocate((VPMT_Image2D *) image);
}

static VPMT_INLINE void
VPMT_Image1DInit(VPMT_Image1D * image, const VPMT_PixelFormat * pixelFormat,
				 GLushort width, void *data)
{
	VPMT_Image2DInit((VPMT_Image2D *) image, pixelFormat, 0, width, 1, data);
}

static VPMT_INLINE VPMT_Color4ub VPMT_Image2DRead(const VPMT_Image2D * image,
#if GL_EXT_paletted_texture
												  const VPMT_Image1D * palette,
#endif
												  GLuint x, GLuint y)
{
#if GL_EXT_paletted_texture
	return image->pixelFormat->read(image, palette, x, y);
#else
	return image->pixelFormat->read(image, x, y);
#endif
}

static VPMT_INLINE void VPMT_Image2DWrite(const VPMT_Image2D * image, GLuint x, GLuint y,
										  VPMT_Color4ub rgba)
{
	image->pixelFormat->write(image, x, y, rgba);
}

typedef VPMT_Color4us(*VPMT_Image2DSampleFunc) (const VPMT_Image2D * image,
#if GL_EXT_paletted_texture
												const VPMT_Image1D * palette,
#endif
												GLfloat s, GLenum wrapS, GLfloat t, GLenum wrapT);

#endif

/* $Id: image.h 74 2008-11-23 07:25:12Z hmwill $ */
