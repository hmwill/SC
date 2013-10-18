/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Bitmap transfer functions.
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

/*
** -------------------------------------------------------------------------
** Forward declarations
** -------------------------------------------------------------------------
*/

void VPMT_ExecDrawPixelsImage(VPMT_Context * context, const VPMT_Image2D * image);

/*
** -------------------------------------------------------------------------
** Exported API entry points
** -------------------------------------------------------------------------
*/

void VPMT_ExecBitmap(VPMT_Context * context, GLsizei width, GLsizei height, GLfloat xorig,
					 GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap)
{
	if (width < 0 || height < 0 || bitmap == NULL) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	if (context->rasterPosValid) {
		if (width != 0 && height != 0) {
			VPMT_Color4ub rgba;
			const VPMT_RasterVertex *rasterPos = &context->rasterPos;
			VPMT_FrameBuffer fb;
			GLuint depth = (GLuint) rasterPos->depth;

			GLsizei unpackAlignment = context->unpackAlignment;
			GLsizei rowOffset =
				unpackAlignment * ((width + 8 * unpackAlignment - 1) / (8 * unpackAlignment));

			/* ancor dst rectangle relative to raster position offset by x/yorig */
			GLint xoffset = VPMT_ROUND(xorig * (1 << VPMT_SUBPIXEL_BITS));
			GLint yoffset = VPMT_ROUND(yorig * (1 << VPMT_SUBPIXEL_BITS));
			GLint x =
				(context->rasterPos.screenCoords[0] + xoffset +
				 (1 << (VPMT_SUBPIXEL_BITS - 1))) >> VPMT_SUBPIXEL_BITS;
			GLint y =
				(context->rasterPos.screenCoords[1] + yoffset +
				 (1 << (VPMT_SUBPIXEL_BITS - 1))) >> VPMT_SUBPIXEL_BITS;

			if (context->writeSurface) {
				context->writeSurface->vtbl->lock(context, context->writeSurface);
			} else {
				VPMT_UpdateActiveSurfaceRect(context, NULL);
			}

			/* determine fragment color based on raster pos attributes */
			rgba =
				VPMT_TexImageUnitsExecute(context->texUnits,
										  VPMT_ConvertVec4ToColor4us(rasterPos->rgba),
										  rasterPos->texCoords, NULL);

			VPMT_FrameBufferInit(&fb, context->writeSurface);
			VPMT_FrameBufferMove(&fb, x, y);

			/* for each pixel in rectangle that is set, submit a fragment */
			while (height--) {
				/* pixel ownership test based on intersection of scissor and surface rect */
				if ((y - context->activeSurfaceRect.origin[1]) >= 0
					&& (GLuint) (y - context->activeSurfaceRect.origin[1]) <
					(GLuint) context->activeSurfaceRect.size.height) {
					const GLubyte *bitmapRow = bitmap;
					GLint xrow = x;
					GLsizei rowCount = width;

					VPMT_FrameBufferSave(&fb);

					while (rowCount > 0) {
						GLubyte pattern = *bitmapRow++;
						GLsizei groupCount = rowCount < 8 ? rowCount : 8;

						while (groupCount--) {
							GLboolean set = pattern & 0x80;

							if (set &&
								(x - context->activeSurfaceRect.origin[0]) >= 0 &&
								(GLuint) (x - context->activeSurfaceRect.origin[0]) <
								(GLuint) context->activeSurfaceRect.size.width) {
								VPMT_Fragment(context, &fb, rgba, depth);
							}

							pattern <<= 1;
							++xrow;
							VPMT_FrameBufferStepX(&fb);
						}

						rowCount -= 8;
					}

					VPMT_FrameBufferRestore(&fb);
				}

				bitmap += rowOffset;
				++y;
				VPMT_FrameBufferStepY(&fb);
			}
		}

		/* increment raster pos by x/ymove */
		context->rasterPos.screenCoords[0] += VPMT_ROUND(xmove * (1 << VPMT_SUBPIXEL_BITS));
		context->rasterPos.screenCoords[1] += VPMT_ROUND(ymove * (1 << VPMT_SUBPIXEL_BITS));

		if (context->writeSurface) {
			context->writeSurface->vtbl->unlock(context, context->writeSurface);
		}
	}
}

void VPMT_ExecCopyPixels(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height,
						 GLenum type)
{
	const VPMT_RasterVertex *rasterPos = &context->rasterPos;
	GLsizei i, j;
	GLint xbase, ybase;
	GLuint depth;
	VPMT_Color4ub rgba, fragColor;
	VPMT_FrameBuffer fb;

	if (type != GL_COLOR) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (width <= 0 || height <= 0) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	if (x < 0) {
		width -= x;
		x = 0;
	}

	if (y < 0) {
		height -= y;
		y = 0;
	}

	if (width <= 0 || height <= 0) {
		return;
	}

	if (!context->readSurface || !context->writeSurface) {
		return;
	}

	/* lock the surfaces */
	context->writeSurface->vtbl->lock(context, context->writeSurface);
	context->readSurface->vtbl->lock(context, context->readSurface);

	/* restrict source area to interior of read surface */
	if (x + width > context->readSurface->image.size.width) {
		width = context->readSurface->image.size.width - x;
	}

	if (y + height > context->readSurface->image.size.height) {
		height = context->readSurface->image.size.height - y;
	}

	/* ancor dst rectangle relative to raster position offset by x/yorig */
	xbase =
		(context->rasterPos.screenCoords[0] +
		 (1 << (VPMT_SUBPIXEL_BITS - 1))) >> VPMT_SUBPIXEL_BITS;
	ybase =
		(context->rasterPos.screenCoords[1] +
		 (1 << (VPMT_SUBPIXEL_BITS - 1))) >> VPMT_SUBPIXEL_BITS;
	depth = (GLuint) context->rasterPos.depth;

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, xbase, ybase);

	/* perform the actual copy loop */
	for (j = 0; j < height; ++j) {
		/* pixel ownership test based on intersection of scissor and surface rect */
		if ((y - context->activeSurfaceRect.origin[1]) >= 0
			&& (GLuint) (y - context->activeSurfaceRect.origin[1]) <
			(GLuint) context->activeSurfaceRect.size.height) {
			VPMT_FrameBufferSave(&fb);

			for (i = 0; i < width; ++i) {
#if GL_EXT_paletted_texture
				rgba = VPMT_Image2DRead(&context->readSurface->image, NULL, x + i, y + j);
#else
				rgba = VPMT_Image2DRead(&context->readSurface->image, x + i, y + j);
#endif
				/* determine fragment color based on raster pos attributes */
				fragColor = VPMT_TexImageUnitsExecute(context->texUnits,
													  VPMT_ConvertColor4ubToColor4us(rgba),
													  rasterPos->texCoords, NULL);

				if ((x - context->activeSurfaceRect.origin[0]) >= 0 &&
					(GLuint) (x - context->activeSurfaceRect.origin[0]) <
					(GLuint) context->activeSurfaceRect.size.width) {
					VPMT_Fragment(context, &fb, fragColor, depth);
				}

				VPMT_FrameBufferStepX(&fb);
			}

			VPMT_FrameBufferRestore(&fb);
		}

		VPMT_FrameBufferStepY(&fb);
	}

	/* unlock the surfaces */
	context->writeSurface->vtbl->unlock(context, context->writeSurface);
	context->readSurface->vtbl->unlock(context, context->readSurface);
}

void VPMT_ExecDrawPixels(VPMT_Context * context, GLsizei width, GLsizei height, GLenum format,
						 GLenum type, const GLvoid * pixels)
{
	VPMT_Image2D image;
	GLsizei pitch;
	const VPMT_PixelFormat *pixelFormat;

	if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (width < 0 || height < 0 || pixels == NULL) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	pixelFormat = VPMT_GetPixelFormat(format, type);
	pitch = VPMT_ALIGN(width * pixelFormat->size, context->unpackAlignment);
	VPMT_Image2DInit(&image, pixelFormat, pitch, width, height, (GLvoid *) pixels);
	VPMT_ExecDrawPixelsImage(context, &image);
}

void VPMT_ExecPixelStorei(VPMT_Context * context, GLenum pname, GLint param)
{
	VPMT_NOT_RENDERING(context);

	switch (pname) {
	case GL_PACK_ALIGNMENT:
		if (param != 1 && param != 2 && param != 4 && param != 8) {
			VPMT_INVALID_VALUE(context);
			return;
		}

		context->packAlignment = param;
		break;

	case GL_UNPACK_ALIGNMENT:
		if (param != 1 && param != 2 && param != 4 && param != 8) {
			VPMT_INVALID_VALUE(context);
			return;
		}

		context->unpackAlignment = param;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_ExecReadPixels(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height,
						 GLenum format, GLenum type, GLvoid * pixels)
{
	VPMT_Image2D dst;

	if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (width < 0 || height < 0 || !pixels) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	if (context->readSurface) {
		VPMT_Vec2i srcPos;
		VPMT_Rect dstRect;
		GLsizei pitch;
		const VPMT_PixelFormat *pixelFormat;

		pixelFormat = VPMT_GetPixelFormat(format, type);
		pitch = VPMT_ALIGN(width * pixelFormat->size, context->packAlignment);
		VPMT_Image2DInit(&dst, pixelFormat, pitch, width, height, pixels);
		context->readSurface->vtbl->lock(context, context->readSurface);

		/* adjust copy range to surface area */
		if (x < 0) {
			srcPos[0] = 0;
			dstRect.origin[0] = -x;
			dstRect.size.width = width + x;
		} else {
			srcPos[0] = x;
			dstRect.origin[0] = 0;
			dstRect.size.width = width;
		}

		if (y < 0) {
			srcPos[1] = 0;
			dstRect.origin[1] = -y;
			dstRect.size.height = height + y;
		} else {
			srcPos[1] = y;
			dstRect.origin[1] = 0;
			dstRect.size.height = height;
		}

		if (srcPos[0] + dstRect.size.width > context->readSurface->image.size.width) {
			dstRect.size.width = context->readSurface->image.size.width - srcPos[0];
		}

		if (srcPos[1] + dstRect.size.height > context->readSurface->image.size.height) {
			dstRect.size.height = context->readSurface->image.size.height - srcPos[1];
		}

		/* perform block-copy (bitblt) */
		VPMT_Bitblt(&dst, &dstRect, &context->readSurface->image, srcPos);

		/* unlock surfaces */
		context->readSurface->vtbl->unlock(context, context->readSurface);
	}
}

/*
** -------------------------------------------------------------------------
** Internal functions
** -------------------------------------------------------------------------
*/

void VPMT_ExecDrawPixelsImage(VPMT_Context * context, const VPMT_Image2D * image)
{
	const VPMT_RasterVertex *rasterPos = &context->rasterPos;
	GLuint depth = (GLuint) rasterPos->depth;
	GLsizei width = image->size.width;
	GLsizei height = image->size.height;
	VPMT_FrameBuffer fb;

	/* ancor dst rectangle relative to raster position offset by x/yorig */
	GLint xbase =
		(context->rasterPos.screenCoords[0] +
		 (1 << (VPMT_SUBPIXEL_BITS - 1))) >> VPMT_SUBPIXEL_BITS;
	GLint ybase =
		(context->rasterPos.screenCoords[1] +
		 (1 << (VPMT_SUBPIXEL_BITS - 1))) >> VPMT_SUBPIXEL_BITS;

	GLint x, y;

	VPMT_NOT_RENDERING(context);

	if (context->writeSurface) {
		context->writeSurface->vtbl->lock(context, context->writeSurface);
	} else {
		VPMT_UpdateActiveSurfaceRect(context, NULL);
	}

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, xbase, ybase);

	/* for each pixel in rectangle that is set, submit a fragment */
	for (y = 0; y < height; ++y) {
		/* pixel ownership test based on intersection of scissor and surface rect */
		if ((y - context->activeSurfaceRect.origin[1]) >= 0
			&& (GLuint) (y - context->activeSurfaceRect.origin[1]) <
			(GLuint) context->activeSurfaceRect.size.height) {
			VPMT_FrameBufferSave(&fb);

			for (x = 0; x < width; ++x) {
				VPMT_Color4ub rgba, fragColor;

				/* look up the bitmap color */
#if GL_EXT_paletted_texture
				rgba = VPMT_Image2DRead(image, NULL, x, y);
#else
				rgba = VPMT_Image2DRead(image, x, y);
#endif
				/* determine fragment color based on raster pos attributes */
				fragColor = VPMT_TexImageUnitsExecute(context->texUnits,
													  VPMT_ConvertColor4ubToColor4us(rgba),
													  rasterPos->texCoords, NULL);

				if ((x - context->activeSurfaceRect.origin[0]) >= 0 &&
					(GLuint) (x - context->activeSurfaceRect.origin[0]) <
					(GLuint) context->activeSurfaceRect.size.width) {
					VPMT_Fragment(context, &fb, fragColor, depth);
				}

				VPMT_FrameBufferStepX(&fb);
			}

			VPMT_FrameBufferRestore(&fb);
		}

		VPMT_FrameBufferStepY(&fb);
	}

	if (context->writeSurface) {
		context->writeSurface->vtbl->unlock(context, context->writeSurface);
	}
}

/* $Id: bitmap.c 74 2008-11-23 07:25:12Z hmwill $ */
