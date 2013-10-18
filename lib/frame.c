/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Framebuffer Access
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "frame.h"


static VPMT_Color4ub ReadColor565(const VPMT_FrameBuffer * fb)
{
	const GLushort *ptr = (const GLushort *) fb->current[0];
	VPMT_Color4ub result;

	result.red = (*ptr & 0xF800) >> 8 | (*ptr & 0xF800) >> 13;
	result.green = (*ptr & 0x07E0) >> 3 | (*ptr & 0x07E0) >> 9;
	result.blue = (*ptr & 0x001F) << 3 | (*ptr & 0x001F) >> 2;
	result.alpha = 0xffu;

	return result;
}

static void WriteColor565(const VPMT_FrameBuffer * fb, VPMT_Color4ub color)
{
	GLushort *ptr = (GLushort *) fb->current[0];

	*ptr =
		((color.red << 8) & 0xF800) | ((color.green << 3) & 0x07E0) | ((color.blue >> 3) & 0x001F);
}

static VPMT_Color4ub ReadColor5551(const VPMT_FrameBuffer * fb)
{
	const GLushort *ptr = (const GLushort *) fb->current[0];
	VPMT_Color4ub result;

	result.red = (*ptr & 0xF800) >> 8 | (*ptr & 0xF800) >> 13;
	result.green = (*ptr & 0x07C0) >> 3 | (*ptr & 0x07C0) >> 8;
	result.blue = (*ptr & 0x003E) << 2 | (*ptr & 0x003E) >> 3;
	result.alpha = (*ptr & 1) ? 0xffu : 0u;

	return result;
}

static void WriteColor5551(const VPMT_FrameBuffer * fb, VPMT_Color4ub color)
{
	GLushort *ptr = (GLushort *) fb->current[0];

	*ptr =
		((color.red << 8) & 0xF800) |
		((color.green << 3) & 0x07C0) | ((color.blue >> 2) & 0x003E) | ((color.alpha >> 7) & 1);
}

static VPMT_Color4ub ReadColor4444(const VPMT_FrameBuffer * fb)
{
	const GLushort *ptr = (const GLushort *) fb->current[0];
	VPMT_Color4ub result;

	result.red = (*ptr & 0xF000) >> 8 | (*ptr & 0xF000) >> 12;
	result.green = (*ptr & 0x0F00) >> 4 | (*ptr & 0x0F00) >> 8;
	result.blue = (*ptr & 0x00F0) | (*ptr & 0x00F0) >> 4;
	result.alpha = (*ptr & 0x000F) << 4 | (*ptr & 0x000F);

	return result;
}

static void WriteColor4444(const VPMT_FrameBuffer * fb, VPMT_Color4ub color)
{
	GLushort *ptr = (GLushort *) fb->current[0];

	*ptr =
		((color.red << 8) & 0xF000) |
		((color.green << 4) & 0x0F00) | ((color.blue) & 0x00F0) | ((color.alpha >> 4) & 0x000F);
}

static VPMT_Color4ub ReadColor8888(const VPMT_FrameBuffer * fb)
{
	const GLuint *ptr = (const GLuint *) fb->current[0];
	VPMT_Color4ub result;

	result.red = (*ptr & 0xFF000000) >> 24;
	result.green = (*ptr & 0x00FF0000) >> 16;
	result.blue = (*ptr & 0x0000FF00) >> 8;
	result.alpha = (*ptr & 0x000000FF);

	return result;
}

static void WriteColor8888(const VPMT_FrameBuffer * fb, VPMT_Color4ub color)
{
	GLuint *ptr = (GLuint *) fb->current[0];

	*ptr = color.red << 24 | color.green << 16 | color.blue << 8 | color.alpha;
}

static VPMT_Color4ub ReadColor8888_REV(const VPMT_FrameBuffer * fb)
{
	const GLuint *ptr = (const GLuint *) fb->current[0];
	VPMT_Color4ub result;

	result.red = (*ptr & 0x000000FF);
	result.green = (*ptr & 0x0000FF00) >> 8;
	result.blue = (*ptr & 0x00FF0000) >> 16;
	result.alpha = (*ptr & 0xFF000000) >> 24;

	return result;
}

static void WriteColor8888_REV(const VPMT_FrameBuffer * fb, VPMT_Color4ub color)
{
	GLuint *ptr = (GLuint *) fb->current[0];

	*ptr = color.alpha << 24 | color.blue << 16 | color.green << 8 | color.red;
}

static GLuint ReadDepth16(const VPMT_FrameBuffer * fb)
{
	const GLushort *ptr = (const GLushort *) fb->current[1];
	return *ptr;
}

static void WriteDepth16(const VPMT_FrameBuffer * fb, GLuint depth)
{
	GLushort *ptr = (GLushort *) fb->current[1];
	assert(!(depth & ~0xffffffu));
	*ptr = depth;
}

static GLuint ReadDepthD24S8(const VPMT_FrameBuffer * fb)
{
	const GLuint *ptr = (const GLuint *) fb->current[1];
	return *ptr >> 8;
}

static void WriteDepthD24S8(const VPMT_FrameBuffer * fb, GLuint depth)
{
	GLuint *ptr = (GLuint *) fb->current[1];
	assert(!(depth & ~0xffffffu));
	*ptr = (*ptr & 0xff) | (depth << 8);
}

static GLuint ReadDepth32(const VPMT_FrameBuffer * fb)
{
	const GLuint *ptr = (const GLuint *) fb->current[1];
	return *ptr;
}

static void WriteDepth32(const VPMT_FrameBuffer * fb, GLuint depth)
{
	GLuint *ptr = (GLuint *) fb->current[1];
	*ptr = depth;
}

static GLuint ReadStencil0(const VPMT_FrameBuffer * fb)
{
	return 0u;
}

static void WriteStencil0(const VPMT_FrameBuffer * fb, GLuint stencil)
{
	assert(!stencil);
}

static GLuint ReadStencilD24S8(const VPMT_FrameBuffer * fb)
{
	const GLuint *ptr = (const GLuint *) fb->current[1];
	return *ptr & 0xffu;
}

static void WriteStencilD24S8(const VPMT_FrameBuffer * fb, GLuint stencil)
{
	GLuint *ptr = (GLuint *) fb->current[1];
	assert(!(stencil & ~0xffu));
	*ptr = (stencil & 0xffu) | (*ptr & 0xffffff00u);
}

/*
** -------------------------------------------------------------------------
** Exported Functions
** -------------------------------------------------------------------------
*/

void VPMT_FrameBufferInit(VPMT_FrameBuffer * fb, VPMT_Surface * surface)
{
	const VPMT_PixelFormat *pixelFormat = surface->image.pixelFormat;

	/* this code assumes 2 buffers; RGBA and depth/stencil */
	fb->current[0] = ((GLubyte *) surface->image.data);
	fb->dx[0] = surface->image.pixelFormat->size;
	fb->dy[0] = surface->image.pitch;

	fb->current[1] = ((GLubyte *) surface->depthStencilBuffer);
	fb->dx[1] = surface->depthStencilFormat->bits / 8;
	fb->dy[1] = surface->depthStencilFormat->bits / 8 * surface->image.size.width;

	switch (pixelFormat->baseFormat) {
	case GL_RGB:
		switch (pixelFormat->type) {
		case GL_UNSIGNED_SHORT_5_6_5:
			fb->readColor = ReadColor565;
			fb->writeColor = WriteColor565;
			break;

		default:
			assert(GL_FALSE);
			fb->readColor = NULL;
			fb->writeColor = NULL;
		}

		break;

	case GL_RGBA:
		switch (pixelFormat->type) {
		case GL_UNSIGNED_SHORT_4_4_4_4:
			fb->readColor = ReadColor4444;
			fb->writeColor = WriteColor4444;
			break;

		case GL_UNSIGNED_SHORT_5_5_5_1:
			fb->readColor = ReadColor5551;
			fb->writeColor = WriteColor5551;
			break;

#if (!VPMT_LITTLE_ENDIAN)
		case GL_UNSIGNED_BYTE:
#endif
		case GL_UNSIGNED_INT_8_8_8_8:
			fb->readColor = ReadColor8888;
			fb->writeColor = WriteColor8888;
			break;

#if (VPMT_LITTLE_ENDIAN)
		case GL_UNSIGNED_BYTE:
#endif
		case GL_UNSIGNED_INT_8_8_8_8_REV:
			fb->readColor = ReadColor8888_REV;
			fb->writeColor = WriteColor8888_REV;
			break;

		default:
			assert(GL_FALSE);
			fb->readColor = NULL;
			fb->writeColor = NULL;
		}

		break;
	}

	switch (surface->depthStencilFormat->type) {
	case VPMT_DEPTH_16:
		fb->readDepth = ReadDepth16;
		fb->writeDepth = WriteDepth16;
		fb->readStencil = ReadStencil0;
		fb->writeStencil = WriteStencil0;
		break;

	case VPMT_DEPTH_24_STENCIL_8:
		fb->readDepth = ReadDepthD24S8;
		fb->writeDepth = WriteDepthD24S8;
		fb->readStencil = ReadStencilD24S8;
		fb->writeStencil = WriteStencilD24S8;
		break;

	case VPMT_DEPTH_32:
		fb->readDepth = ReadDepth32;
		fb->writeDepth = WriteDepth32;
		fb->readStencil = ReadStencil0;
		fb->writeStencil = WriteStencil0;
		break;

	default:
		fb->readDepth = NULL;
		fb->writeDepth = NULL;
		fb->readStencil = NULL;
		fb->writeStencil = NULL;
	}
}

/* $Id: frame.c 74 2008-11-23 07:25:12Z hmwill $ */
