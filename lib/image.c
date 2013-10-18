/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Image manipulaton functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "image.h"

/*
** -------------------------------------------------------------------------
** Data Tables
** -------------------------------------------------------------------------
*/

#if GL_EXT_paletted_texture
#define OPT_PALETTE const VPMT_Image1D * palette,
#else
#define OPT_PALETTE
#endif

static VPMT_Color4ub ReadLuminance(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y);
static VPMT_Color4ub ReadAlpha(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y);
static VPMT_Color4ub ReadLuminanceAlpha(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y);
static VPMT_Color4ub ReadRGB(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y);
static VPMT_Color4ub ReadRGBA(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y);
static VPMT_Color4ub ReadBGRA(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y);

static VPMT_Color4ub ReadRGBA_8888(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y);
static VPMT_Color4ub ReadRGBA_8888_REV(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y);

static void WriteLuminance(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba);
static void WriteAlpha(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba);
static void WriteLuminanceAlpha(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba);
static void WriteRGB(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba);
static void WriteRGBA(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba);
static void WriteBGRA(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba);

static void WriteRGBA_8888(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba);
static void WriteRGBA_8888_REV(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba);

#if GL_EXT_paletted_texture
static VPMT_Color4ub ReadColorIndex8(const VPMT_Image2D * image, const VPMT_Image1D * palette,
									 GLuint x, GLuint y);
#endif

static const VPMT_PixelFormat LuminanceFormats[] = {
	/*  internalFormat,     format,             type,               size,   bits,   red,    green,  blue,   alpha,  read, write   */
	{GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 1, 8, 8, 0, 0, 0, 0xFF, 0, 0, 0, ReadLuminance, WriteLuminance},
};

static const VPMT_PixelFormat AlphaFormats[] = {
	/*  internalFormat,     format,             type,               size,   bits,   red,    green,  blue,   alpha,  read, write   */
	{GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 1, 8, 0, 0, 0, 8, 0, 0, 0, 0xFF, ReadAlpha, WriteAlpha},
};

static const VPMT_PixelFormat LuminanceAlphaFormats[] = {
	/*  internalFormat,     format,             type,               size,   bits,   red,    green,  blue,   alpha,  read, write   */
#if (VPMT_LITTLE_ENDIAN)
	{GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2, 16, 8, 0, 0, 8,
	 0xFFu, 0, 0, 0xFF00u, ReadLuminanceAlpha, WriteLuminanceAlpha},
#else
	{GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2, 16, 8, 0, 0, 8,
	 0XFF00u, 0, 0, 0xFFu, ReadLuminanceAlpha, WriteLuminanceAlpha},
#endif
};

static const VPMT_PixelFormat RGBFormats[] = {
	/*  internalFormat,     format,             type,               size,   bits,   red,    green,  blue,   alpha,  read, write   */
#if (VPMT_LITTLE_ENDIAN)
	{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 3, 24, 8, 8, 8, 0, 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0, ReadRGB, WriteRGB},
#else
	{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 3, 24, 8, 8, 8, 0, 0xFF000000u, 0x00FF0000u, 0x0000FF00u, 0, ReadRGB, WriteRGB},
#endif
};

static const VPMT_PixelFormat RGBAFormats[] = {
	/*  internalFormat,     format,             type,               size,   bits,   red,    green,  blue,   alpha,  read, write   */
#if (VPMT_LITTLE_ENDIAN)
	{GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4, 32, 8, 8, 8, 8, 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0xFF000000u, ReadRGBA, WriteRGBA},
#else
	{GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4, 32, 8, 8, 8, 8, 0xFF000000u, 0x00FF0000u, 0x0000FF00u, 0x000000FFu, ReadRGBA, WriteRGBA},
#endif
	{GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 4, 32, 8, 8, 8, 8, 0xFF000000u, 0x00FF0000u, 0x0000FF00u, 0x000000FFu, ReadRGBA_8888, WriteRGBA_8888},
	{GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, 32, 8, 8, 8, 8, 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0xFF000000u, ReadRGBA_8888_REV,
	 WriteRGBA_8888_REV},
	//{GL_BGRA, GL_RGBA, GL_UNSIGNED_BYTE, 4, 32, 8, 8, 8, 8, ReadBGRA, WriteBGRA},
};

#if GL_EXT_paletted_texture
static const VPMT_PixelFormat ColorIndexFormats[] = {
	/*  internalFormat,     format,             type,               size,   bits,   red,    green,  blue,   alpha,  read, write   */
	{GL_COLOR_INDEX8_EXT, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, 1, 8, 0, 0, 0, 0, 0, 0, 0, 0, ReadColorIndex8,
	 NULL},
};
#endif

static const VPMT_DepthStencilFormat DepthStencilFormats[] = {
	/*  type,                       bits,       depthBits,  stencilBits */
	{VPMT_DEPTH_16, 16, 16, 0},
	{VPMT_DEPTH_24_STENCIL_8, 32, 24, 8},
	{VPMT_DEPTH_32, 32, 32, 0}
};

/*
** -------------------------------------------------------------------------
** Internal functions
** -------------------------------------------------------------------------
*/

static const VPMT_PixelFormat *SearchFormats(const VPMT_PixelFormat * formats, GLsizei size,
											 GLenum type)
{
	while (size--) {
		if (formats->type == type) {
			return formats;
		}

		++formats;
	}

	return NULL;
}

const VPMT_PixelFormat *VPMT_GetPixelFormat(GLenum format, GLenum type)
{
	switch (format) {
	case GL_LUMINANCE:
		return SearchFormats(LuminanceFormats, VPMT_ELEMENTSOF(LuminanceFormats), type);
	case GL_ALPHA:
		return SearchFormats(AlphaFormats, VPMT_ELEMENTSOF(AlphaFormats), type);
	case GL_LUMINANCE_ALPHA:
		return SearchFormats(LuminanceAlphaFormats, VPMT_ELEMENTSOF(LuminanceAlphaFormats), type);
	case GL_RGB:
		return SearchFormats(RGBFormats, VPMT_ELEMENTSOF(RGBFormats), type);
	case GL_RGBA:
		return SearchFormats(RGBAFormats, VPMT_ELEMENTSOF(RGBAFormats), type);
#if GL_EXT_paletted_texture
	case GL_COLOR_INDEX8_EXT:
	case GL_COLOR_INDEX:
		return SearchFormats(ColorIndexFormats, VPMT_ELEMENTSOF(ColorIndexFormats), type);
#endif
	default:
		return NULL;
	}
}

const VPMT_DepthStencilFormat *VPMT_GetDepthStencilFormat(VPMT_DepthStencilType type)
{
	switch (type) {
	case VPMT_DEPTH_16:
		return DepthStencilFormats;
	case VPMT_DEPTH_24_STENCIL_8:
		return DepthStencilFormats + 1;
	case VPMT_DEPTH_32:
		return DepthStencilFormats + 2;
	default:
		return NULL;
	}
}

VPMT_Image2D *VPMT_Image2DAllocate(const VPMT_PixelFormat * pixelFormat, GLushort width,
								   GLushort height)
{
	GLsizei size;
	void *data;
	VPMT_Image2D *image;

	assert(pixelFormat);

	image = VPMT_MALLOC(sizeof(VPMT_Image2D));

	if (!image) {
		return NULL;
	}

	size = width * height * pixelFormat->size;
	data = VPMT_MALLOC(size);

	if (!data) {
		VPMT_FREE(image);
		return NULL;
	}

	memset(data, 0, size);
	VPMT_Image2DInit(image, pixelFormat, width * pixelFormat->size, width, height, data);

	return image;
}

void VPMT_Image2DDeallocate(VPMT_Image2D * image)
{
	assert(image);
	assert(image->data);

	VPMT_FREE(image->data);
	image->data = NULL;
	VPMT_FREE(image);
}

void VPMT_Image2DInit(VPMT_Image2D * image, const VPMT_PixelFormat * pixelFormat, GLsizei pitch,
					  GLushort width, GLushort height, void *data)
{
	assert(image);
	assert(pixelFormat);

	image->data = data;
	image->pixelFormat = pixelFormat;
	image->pitch = pitch;
	image->size.width = width;
	image->size.height = height;

	image->invSize2.width = width ? 1.0f / (2.0f * width) : 0.0f;
	image->invSize2.height = height ? 1.0f / (2.0f * height) : 0.0f;
}

static VPMT_Color4ub ReadLuminance(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y)
{
	const GLubyte *base = ((const GLubyte *) image->data) +
		y * image->pitch + x * image->pixelFormat->size;
	VPMT_Color4ub rgba;

	rgba.red = rgba.green = rgba.blue = base[0];
	rgba.alpha = 0xffu;

	return rgba;
}

static void WriteLuminance(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba)
{
	GLubyte *base = ((GLubyte *) image->data) + y * image->pitch + x * image->pixelFormat->size;

	base[0] = rgba.red;
}

static VPMT_Color4ub ReadAlpha(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y)
{
	const GLubyte *base = ((const GLubyte *) image->data) +
		y * image->pitch + x * image->pixelFormat->size;
	VPMT_Color4ub rgba;

	rgba.red = rgba.green = rgba.blue = 0;
	rgba.alpha = base[0];

	return rgba;
}

static void WriteAlpha(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba)
{
	GLubyte *base = ((GLubyte *) image->data) + y * image->pitch + x * image->pixelFormat->size;

	base[0] = rgba.alpha;
}

static VPMT_Color4ub ReadLuminanceAlpha(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y)
{
	const GLubyte *base = ((const GLubyte *) image->data) +
		y * image->pitch + x * image->pixelFormat->size;
	VPMT_Color4ub rgba;

	rgba.red = rgba.green = rgba.blue = base[0];
	rgba.alpha = base[1];

	return rgba;
}

static void WriteLuminanceAlpha(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba)
{
	GLubyte *base = ((GLubyte *) image->data) + y * image->pitch + x * image->pixelFormat->size;

	base[0] = rgba.red;
	base[1] = rgba.alpha;
}

static VPMT_Color4ub ReadRGB(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y)
{
	const GLubyte *base = ((const GLubyte *) image->data) +
		y * image->pitch + x * image->pixelFormat->size;
	VPMT_Color4ub rgba;

	rgba.red = base[0];
	rgba.green = base[1];
	rgba.blue = base[2];
	rgba.alpha = 0xffu;

	return rgba;
}

static void WriteRGB(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba)
{
	GLubyte *base = ((GLubyte *) image->data) + y * image->pitch + x * image->pixelFormat->size;

	base[0] = rgba.red;
	base[1] = rgba.green;
	base[2] = rgba.blue;
}

static VPMT_Color4ub ReadRGBA(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y)
{
	const GLubyte *base = ((const GLubyte *) image->data) +
		y * image->pitch + x * image->pixelFormat->size;
	VPMT_Color4ub rgba;

	rgba.red = base[0];
	rgba.green = base[1];
	rgba.blue = base[2];
	rgba.alpha = base[3];

	return rgba;
}

static void WriteRGBA(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba)
{
	GLubyte *base = ((GLubyte *) image->data) + y * image->pitch + x * image->pixelFormat->size;

	base[0] = rgba.red;
	base[1] = rgba.green;
	base[2] = rgba.blue;
	base[3] = rgba.alpha;
}

static VPMT_Color4ub ReadRGBA_8888(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y)
{
	const GLuint *base = (const GLuint *) (((const GLubyte *) image->data) +
										   y * image->pitch + (x << 2));
	GLuint value = *base;
	VPMT_Color4ub rgba;

	rgba.alpha = value & 0xffu;
	rgba.blue = (value >>= 8) & 0xffu;
	rgba.green = (value >>= 8) & 0xffu;
	rgba.red = (value >>= 8) & 0xffu;

	return rgba;
}

static void WriteRGBA_8888(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba)
{
	GLuint *base = (GLuint *) (((GLubyte *) image->data) + y * image->pitch + (x << 2));

	*base = ((GLuint) rgba.red << 24) |
		((GLuint) rgba.green << 16) | ((GLuint) rgba.blue << 8) | (GLuint) rgba.alpha;
}

static VPMT_Color4ub ReadRGBA_8888_REV(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y)
{
	const GLuint *base = (const GLuint *) (((const GLubyte *) image->data) +
										   y * image->pitch + (x << 2));
	GLuint value = *base;
	VPMT_Color4ub rgba;

	rgba.red = value & 0xffu;
	rgba.green = (value >>= 8) & 0xffu;
	rgba.blue = (value >>= 8) & 0xffu;
	rgba.alpha = (value >>= 8) & 0xffu;

	return rgba;
}

static void WriteRGBA_8888_REV(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba)
{
	GLuint *base = (GLuint *) (((GLubyte *) image->data) + y * image->pitch + (x << 2));

	*base = (GLuint) rgba.red |
		((GLuint) rgba.green << 8) | ((GLuint) rgba.blue << 16) | ((GLuint) rgba.alpha << 24);
}

static VPMT_Color4ub ReadBGRA(const VPMT_Image2D * image, OPT_PALETTE GLuint x, GLuint y)
{
	const GLubyte *base = ((const GLubyte *) image->data) +
		y * image->pitch + x * image->pixelFormat->size;
	VPMT_Color4ub rgba;

	rgba.red = base[2];
	rgba.green = base[1];
	rgba.blue = base[0];
	rgba.alpha = base[3];

	return rgba;
}

static void WriteBGRA(const VPMT_Image2D * image, GLuint x, GLuint y, VPMT_Color4ub rgba)
{
	GLubyte *base = ((GLubyte *) image->data) + y * image->pitch + x * image->pixelFormat->size;

	base[0] = rgba.blue;
	base[1] = rgba.green;
	base[2] = rgba.red;
	base[3] = rgba.alpha;
}

#if GL_EXT_paletted_texture
static VPMT_Color4ub ReadColorIndex8(const VPMT_Image2D * image, const VPMT_Image1D * palette,
									 GLuint x, GLuint y)
{
	const GLubyte *base = ((const GLubyte *) image->data) +
		y * image->pitch + x * image->pixelFormat->size;
	GLubyte index = *base;

	if (palette) {
		return palette->pixelFormat->read(palette, NULL, index, 0);
	} else {
		VPMT_Color4ub rgba;

		rgba.red = rgba.green = rgba.blue = rgba.alpha = 0;

		return rgba;
	}
}
#endif

/* $Id: image.c 74 2008-11-23 07:25:12Z hmwill $ */
