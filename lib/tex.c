/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Texturing functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "exec.h"


static VPMT_INLINE GLboolean IsCompatibleFormat(GLenum format, GLenum internalFormat)
{
#if GL_EXT_paletted_texture
	if (format == GL_COLOR_INDEX) {
		return internalFormat == GL_COLOR_INDEX8_EXT;
	}
#endif

	return format == internalFormat;
}

/*
** -------------------------------------------------------------------------
** Exported API entry points
** -------------------------------------------------------------------------
*/

void VPMT_ExecDeleteTextures (VPMT_Context * context, GLsizei n, const GLuint *textures)
{
	VPMT_NOT_RENDERING(context);

	if (n < 0 || !textures) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	while (n--) {
		GLuint name = *textures++;
		GLsizei index;
		VPMT_Texture2D * texture;

		if (!name) {
			continue;
		}

		texture = VPMT_HashTableFind(&context->textures, name);

		if (!texture) {
			continue;
		}

		for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
			/* unbind texture image units using a texture to be deleted */
			if (context->texUnits[index].boundTexture == texture) {
				GLsizei saveBinding = context->activeTextureIndex;

				VPMT_ExecActiveTexture(context, index);
				VPMT_ExecBindTexture(context, GL_TEXTURE_2D, 0);
				VPMT_ExecActiveTexture(context, saveBinding);
			}
		}

		VPMT_HashTableRemove(&context->textures, name);
		VPMT_Texture2DDeallocate(texture);
	}
}

void VPMT_ExecGenTextures(VPMT_Context * context, GLsizei n, GLuint * textures)
{
	GLuint base;

	VPMT_NOT_RENDERING(context);

	if (n <= 0 || !textures) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	base = VPMT_HashTableFreeKeyBlock(&context->textures, n);

	if (!base) {
		VPMT_OUT_OF_MEMORY(context);
		return;
	}

	while (n--) {
		VPMT_Texture2D *texture = VPMT_Texture2DAllocate(base);

		if (!texture) {
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		if (!VPMT_HashTableInsert(&context->textures, base, texture)) {
			VPMT_Texture2DDeallocate(texture);
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		*textures++ = base++;
	}
}

void VPMT_ExecGetTexParameteriv(VPMT_Context * context, GLenum target, GLenum pname, GLint * params)
{
	VPMT_Texture2D *texture;

	VPMT_NOT_RENDERING(context);

	texture = context->texUnits[context->activeTextureIndex].boundTexture;

	if (target != GL_TEXTURE_2D) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (pname) {
	case GL_TEXTURE_WRAP_S:
		*params = texture->texWrapS;
		break;

	case GL_TEXTURE_WRAP_T:
		*params = texture->texWrapT;
		break;

	case GL_TEXTURE_MIN_FILTER:
		*params = texture->texMinFilter;
		break;

	case GL_TEXTURE_MAG_FILTER:
		*params = texture->texMagFilter;
		break;

	default:
		VPMT_INVALID_VALUE(context);
		return;
	}
}

void VPMT_ExecTexImage2D(VPMT_Context * context, GLenum target, GLint level, GLint internalformat,
						 GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
						 const GLvoid * pixels)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image;

	VPMT_NOT_RENDERING(context);

	if (type != GL_UNSIGNED_BYTE || !VPMT_ValidateInternalFormat(internalformat) ||
		!VPMT_ValidateTextureFormat(format) || target != GL_TEXTURE_2D) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!IsCompatibleFormat(format, internalformat) || border != 0 || level > VPMT_MAX_MIPMAP_LEVEL
		|| level < 0 || width <= 0 || height <= 0) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	image = texture->mipmaps[level];

#ifdef VPMT_SC_RELAX
	if (image &&
		(image->pixelFormat->internalFormat != internalformat ||
		 image->size.width != width || image->size.height != height)) {
		VPMT_Image2DDeallocate(image);
		texture->mipmaps[level] = image = NULL;
		texture->validated = GL_FALSE;
	}

	if (!image) {
#else
	if (image) {
		if (image->pixelFormat->internalFormat != internalformat ||
			image->size.width != width || image->size.height != height) {
			VPMT_INVALID_VALUE(context);
			return;
		}
	} else {
#endif
		image = VPMT_Image2DAllocate(VPMT_GetPixelFormat(format, type), width, height);

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		texture->mipmaps[level] = image;
	}

	texture->validated = GL_FALSE;

	if (pixels) {
		VPMT_Image2D srcImage;
		VPMT_Rect rect;
		GLsizei pitch;
		const VPMT_PixelFormat *pixelFormat;

		rect.origin[0] = 0;
		rect.origin[1] = 0;
		rect.size.width = width;
		rect.size.height = height;

		pixelFormat = VPMT_GetPixelFormat(format, type);
		pitch = VPMT_ALIGN(width * pixelFormat->size, context->unpackAlignment);
		VPMT_Image2DInit(&srcImage, pixelFormat, pitch, width, height, (GLubyte *) pixels);
		VPMT_Bitblt(image, &rect, &srcImage, NULL);
	}
}

void VPMT_ExecTexParameteri(VPMT_Context * context, GLenum target, GLenum pname, GLint param)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;

	VPMT_NOT_RENDERING(context);

	if (target != GL_TEXTURE_2D) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	switch (pname) {
	case GL_TEXTURE_WRAP_S:
		if (param != GL_REPEAT && param != GL_CLAMP_TO_EDGE) {
			VPMT_INVALID_VALUE(context);
			return;
		}

		texture->texWrapS = param;
		break;

	case GL_TEXTURE_WRAP_T:
		if (param != GL_REPEAT && param != GL_CLAMP_TO_EDGE) {
			VPMT_INVALID_VALUE(context);
			return;
		}

		texture->texWrapT = param;
		break;

	case GL_TEXTURE_MIN_FILTER:
		if (param != GL_NEAREST && param != GL_LINEAR &&
			param != GL_NEAREST_MIPMAP_LINEAR && param != GL_NEAREST_MIPMAP_NEAREST &&
			param != GL_LINEAR_MIPMAP_LINEAR && param != GL_LINEAR_MIPMAP_NEAREST) {
			VPMT_INVALID_VALUE(context);
			return;
		}

		texture->texMinFilter = param;
		break;

	case GL_TEXTURE_MAG_FILTER:
		if (param != GL_NEAREST && param != GL_LINEAR) {
			VPMT_INVALID_VALUE(context);
			return;
		}

		texture->texMagFilter = param;
		break;

	default:
		VPMT_INVALID_VALUE(context);
		return;
	}

	texture->validated = GL_FALSE;
}

void VPMT_ExecTexSubImage2D(VPMT_Context * context, GLenum target, GLint level, GLint xoffset,
							GLint yoffset, GLsizei width, GLsizei height, GLenum format,
							GLenum type, const GLvoid * pixels)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image;
	VPMT_Image2D srcImage;
	VPMT_Rect rect;
	GLsizei pitch;
	const VPMT_PixelFormat *pixelFormat;

	VPMT_NOT_RENDERING(context);

	if (type != GL_UNSIGNED_BYTE || !VPMT_ValidateTextureFormat(format) || target != GL_TEXTURE_2D) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (level > VPMT_MAX_MIPMAP_LEVEL || level < 0 || xoffset < 0 || yoffset < 0 ||
		width <= 0 || height <= 0 || !pixels) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	image = texture->mipmaps[level];

	if (!image) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	if (!IsCompatibleFormat(format, image->pixelFormat->internalFormat) ||
		image->size.width < xoffset + width || image->size.height < yoffset + height) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	rect.origin[0] = xoffset;
	rect.origin[1] = yoffset;
	rect.size.width = width;
	rect.size.height = height;

	pixelFormat = image->pixelFormat;
	pitch = VPMT_ALIGN(width * pixelFormat->size, context->packAlignment);
	VPMT_Image2DInit(&srcImage, pixelFormat, pitch, width, height, (GLubyte *) pixels);
	VPMT_Bitblt(image, &rect, &srcImage, NULL);

	texture->validated = GL_FALSE;
}

#if GL_EXT_paletted_texture

void VPMT_ExecColorSubTable(VPMT_Context * context, GLenum target, GLsizei start, GLsizei count,
							GLenum format, GLenum type, const GLvoid * table)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image;
	VPMT_Image2D srcImage;
	VPMT_Rect rect;
	const VPMT_PixelFormat *pixelFormat;

	VPMT_NOT_RENDERING(context);

	if (type != GL_UNSIGNED_BYTE || !VPMT_ValidateNonIndexedTextureFormat(format) ||
		target != GL_TEXTURE_2D) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (start < 0 || count <= 0 || !table) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	image = texture->palette;

	if (!image) {
		/* silently create a default palette */
		GLubyte *data;

		/* always allocate at least 256 entries corresponding to 8 bit index values */
		image = VPMT_Image1DAllocate(VPMT_GetPixelFormat(GL_RGBA, GL_UNSIGNED_BYTE), 256);

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		/* init default color value */
		data = image->data;
		data[0] = data[1] = data[2] = data[3] = VPMT_UBYTE_MAX;

		/* override actual dimension */
		image->size.width = 1;
		texture->palette = image;
		texture->validated = GL_FALSE;
	}

	if (!IsCompatibleFormat(format, image->pixelFormat->internalFormat)) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	if (image->size.width < start + count) {
		/* "count is silently clamped so that all modified entries all within the legal range." */
		count = image->size.width - start;

		if (count <= 0) {
			return;
		}
	}

	rect.origin[0] = start;
	rect.origin[1] = 0;
	rect.size.width = count;
	rect.size.height = 1;

	pixelFormat = image->pixelFormat;
	VPMT_Image1DInit(&srcImage, pixelFormat, count, (GLubyte *) table);
	VPMT_Bitblt(image, &rect, &srcImage, NULL);

	texture->validated = GL_FALSE;
}

void VPMT_ExecColorTable(VPMT_Context * context, GLenum target, GLenum internalformat,
						 GLsizei width, GLenum format, GLenum type, const GLvoid * table)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image;

	VPMT_NOT_RENDERING(context);

	if (type != GL_UNSIGNED_BYTE || !VPMT_ValidateNonIndexedTextureFormat(internalformat) ||
		!VPMT_ValidateNonIndexedTextureFormat(format) || target != GL_TEXTURE_2D) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!IsCompatibleFormat(format, internalformat) || width <= 0) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	image = texture->palette;

#ifdef VPMT_SC_RELAX
	if (image &&
		(!IsCompatibleFormat(format, image->pixelFormat->internalFormat) ||
		 image->size.width != width)) {
		VPMT_Image2DDeallocate(image);
		texture->palette = image = NULL;
		texture->validated = GL_FALSE;
	}

	if (!image) {
#else
	if (image) {
		if (!IsCompatibleFormat(format, image->pixelFormat->internalFormat) ||
			image->size.width != width) {
			VPMT_INVALID_VALUE(context);
			return;
		}
	} else {
#endif
		/* always allocate at least 256 entries corresponding to 8 bit index values */
		image = VPMT_Image1DAllocate(VPMT_GetPixelFormat(format, type), VPMT_MAX(256, width));

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		/* override actual dimension */
		image->size.width = width;
		texture->palette = image;
	}

	texture->validated = GL_FALSE;

	if (table) {
		VPMT_Image2D srcImage;
		VPMT_Rect rect;
		const VPMT_PixelFormat *pixelFormat;

		rect.origin[0] = 0;
		rect.origin[1] = 0;
		rect.size.width = width;
		rect.size.height = 1;

		pixelFormat = VPMT_GetPixelFormat(format, type);
		VPMT_Image1DInit(&srcImage, pixelFormat, width, (GLubyte *) table);
		VPMT_Bitblt(image, &rect, &srcImage, NULL);
	}
}

void VPMT_ExecGetColorTable(VPMT_Context * context, GLenum target, GLenum format, GLenum type,
							GLvoid * table)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image = texture->palette;

	if (target != GL_TEXTURE_2D || !VPMT_ValidateNonIndexedTextureFormat(format) ||
		type != GL_UNSIGNED_BYTE) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!table) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	if (image == NULL) {
		if (format != GL_RGBA) {
			/* no conversions for texture data access, so I assume the same holds here */
			VPMT_INVALID_VALUE(context);
			return;
		} else {
			/* default value is all 1s */
			GLubyte *data = table;
			data[0] = data[1] = data[2] = data[3] = VPMT_UBYTE_MAX;
		}
	} else if (!IsCompatibleFormat(format, image->pixelFormat->internalFormat)) {
		/* no conversions for texture data access, so I assume the same holds here */
		VPMT_INVALID_VALUE(context);
		return;
	} else {
		VPMT_Image2D dstImage;
		VPMT_Rect rect;

		rect.origin[0] = 0;
		rect.origin[1] = 0;
		rect.size.width = image->size.width;
		rect.size.height = 1;

		VPMT_Image2DInit(&dstImage, image->pixelFormat, 0, image->size.width, 1, table);
		VPMT_Bitblt(&dstImage, &rect, image, NULL);
	}
}

void VPMT_ExecGetColorTableParameteriv(VPMT_Context * context, GLenum target, GLenum pname,
									   GLint * params)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image = texture->palette;
	const VPMT_PixelFormat *pixelFormat =
		image ? image->pixelFormat : VPMT_GetPixelFormat(GL_RGBA, GL_UNSIGNED_BYTE);

	if (target != GL_TEXTURE_2D) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (pname) {
	case GL_COLOR_TABLE_FORMAT_EXT:
		*params = pixelFormat->internalFormat;
		break;

	case GL_COLOR_TABLE_WIDTH_EXT:
		if (image) {
			*params = image->size.width;
		} else {
			*params = 1;
		}

		break;

	case GL_COLOR_TABLE_RED_SIZE_EXT:
		*params = pixelFormat->redBits;
		break;

	case GL_COLOR_TABLE_GREEN_SIZE_EXT:
		*params = pixelFormat->greenBits;
		break;

	case GL_COLOR_TABLE_BLUE_SIZE_EXT:
		*params = pixelFormat->blueBits;
		break;

	case GL_COLOR_TABLE_ALPHA_SIZE_EXT:
		*params = pixelFormat->alphaBits;
		break;

	case GL_COLOR_TABLE_LUMINANCE_SIZE_EXT:
	case GL_COLOR_TABLE_INTENSITY_SIZE_EXT:
		/* The extension specification specifies those as valid tokens, */
		/* but does not define the value to return??? */
		*params = 0;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		break;
	}
}

#endif

/*
** -------------------------------------------------------------------------
** Internal functions
** -------------------------------------------------------------------------
*/

GLboolean VPMT_ValidateTextureFormat(GLenum format)
{
	switch (format) {
	case GL_LUMINANCE:
	case GL_ALPHA:
	case GL_LUMINANCE_ALPHA:
	case GL_RGB:
	case GL_RGBA:
#if GL_EXT_paletted_texture
	case GL_COLOR_INDEX:
#endif
		return GL_TRUE;

	default:
		return GL_FALSE;
	}
}

GLboolean VPMT_ValidateInternalFormat(GLenum format)
{
	switch (format) {
	case GL_LUMINANCE:
	case GL_ALPHA:
	case GL_LUMINANCE_ALPHA:
	case GL_RGB:
	case GL_RGBA:
#if GL_EXT_paletted_texture
	case GL_COLOR_INDEX8_EXT:
#endif
		return GL_TRUE;

	default:
		return GL_FALSE;
	}
}

GLboolean VPMT_ValidateNonIndexedTextureFormat(GLenum format)
{
	switch (format) {
	case GL_LUMINANCE:
	case GL_ALPHA:
	case GL_LUMINANCE_ALPHA:
	case GL_RGB:
	case GL_RGBA:
		return GL_TRUE;

	default:
		return GL_FALSE;
	}
}

VPMT_Texture2D *VPMT_Texture2DAllocate(GLuint name)
{
	VPMT_Texture2D *texture = VPMT_MALLOC(sizeof(VPMT_Texture2D));

	if (texture) {
		memset(texture, 0, sizeof(VPMT_Texture2D));

		texture->name = name;
		texture->texMinFilter = GL_NEAREST_MIPMAP_LINEAR;
		texture->texMagFilter = GL_LINEAR;
		texture->texWrapS = GL_REPEAT;
		texture->texWrapT = GL_REPEAT;
	}

	return texture;
}

void VPMT_Texture2DDeallocate(VPMT_Texture2D * texture)
{
	GLsizei index;

	for (index = 0; index <= VPMT_MAX_MIPMAP_LEVEL; ++index) {
		if (texture->mipmaps[index]) {
			VPMT_Image2DDeallocate(texture->mipmaps[index]);
		}
	}

#if GL_EXT_paletted_texture
	if (texture->palette) {
		VPMT_FREE(texture->palette);
	}
#endif

	VPMT_FREE(texture);
}

GLboolean VPMT_Texture2DIsMipmap(const VPMT_Texture2D * texture)
{
	return
		texture->texMinFilter == GL_NEAREST_MIPMAP_LINEAR ||
		texture->texMinFilter == GL_NEAREST_MIPMAP_NEAREST ||
		texture->texMinFilter == GL_LINEAR_MIPMAP_LINEAR ||
		texture->texMinFilter == GL_LINEAR_MIPMAP_NEAREST;
}

void VPMT_Texture2DValidate(VPMT_Texture2D * texture)
{
	GLboolean isMipmap = VPMT_Texture2DIsMipmap(texture);

	if (isMipmap) {
		GLsizei width, height, index;
		const VPMT_PixelFormat *pixelFormat;

		/* check for completeness */
		texture->complete = GL_FALSE;

		if (texture->mipmaps[0] == NULL) {
			return;
		}

		width = texture->mipmaps[0]->size.width;
		height = texture->mipmaps[0]->size.height;
		pixelFormat = texture->mipmaps[0]->pixelFormat;

		/* determine number of mipmap levels */

		for (index = 1; index <= VPMT_MAX_MIPMAP_LEVEL && (width != 1 || height != 1); ++index) {
			width = width > 1 ? width / 2 : 1;
			height = height > 1 ? height / 2 : 1;

			if (texture->mipmaps[index]->pixelFormat != pixelFormat ||
				texture->mipmaps[index]->size.width != width ||
				texture->mipmaps[index]->size.height != height)
				return;
		}

		if (width == 1 && height == 1) {
			texture->maxMipmapLevel = index - 1;
			texture->complete = GL_TRUE;
		}
	} else {
		texture->complete = texture->mipmaps[0] != NULL;
	}
}

void VPMT_ExecTexImage2DImage(VPMT_Context * context, GLenum target, GLint level,
							  GLint internalformat, const VPMT_Image2D * srcImage)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image;
	VPMT_Rect rect;
	GLsizei width = srcImage->size.width;
	GLsizei height = srcImage->size.height;

	VPMT_NOT_RENDERING(context);

	if (!VPMT_ValidateInternalFormat(internalformat)) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (srcImage->pixelFormat->internalFormat != internalformat || level > VPMT_MAX_MIPMAP_LEVEL
		|| level < 0) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	image = texture->mipmaps[level];

	if (image) {
		if (image->pixelFormat->internalFormat != srcImage->pixelFormat->internalFormat ||
			image->pixelFormat->baseFormat != srcImage->pixelFormat->baseFormat ||
			image->pixelFormat->type != srcImage->pixelFormat->type ||
			image->size.width != width || image->size.height != height) {
			VPMT_INVALID_VALUE(context);
			return;
		}
	} else {
		image = VPMT_Image2DAllocate(srcImage->pixelFormat, width, height);

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		texture->mipmaps[level] = image;
	}

	texture->validated = GL_FALSE;

	rect.origin[0] = 0;
	rect.origin[1] = 0;
	rect.size.width = width;
	rect.size.height = height;

	VPMT_Bitblt(image, &rect, srcImage, NULL);
}

void VPMT_ExecTexSubImage2DImage(VPMT_Context * context, GLenum target, GLint level, GLint xoffset,
								 GLint yoffset, const VPMT_Image2D * srcImage)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image;
	VPMT_Rect rect;
	GLsizei width = srcImage->size.width;
	GLsizei height = srcImage->size.height;

	VPMT_NOT_RENDERING(context);

	if (level > VPMT_MAX_MIPMAP_LEVEL || level < 0 || xoffset < 0 || yoffset < 0) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	image = texture->mipmaps[level];

	if (!image) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	if (image->pixelFormat->internalFormat != srcImage->pixelFormat->internalFormat ||
		image->size.width < xoffset + width || image->size.height < yoffset + height) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	rect.origin[0] = xoffset;
	rect.origin[1] = yoffset;
	rect.size.width = width;
	rect.size.height = height;

	VPMT_Bitblt(image, &rect, srcImage, NULL);

	texture->validated = GL_FALSE;
}

void VPMT_ExecColorSubTableImage(VPMT_Context * context, GLenum target, GLsizei start,
								 const VPMT_Image1D * srcImage)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image;
	VPMT_Rect rect;
	GLsizei count = srcImage->size.width;
	GLenum format = srcImage->pixelFormat->baseFormat;
	GLenum internalformat = srcImage->pixelFormat->internalFormat;

	VPMT_NOT_RENDERING(context);

	if (target != GL_TEXTURE_2D) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (start < 0 || count <= 0 || !srcImage) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	image = texture->palette;

	if (!image) {
		/* silently create a default palette */
		GLubyte *data;

		/* always allocate at least 256 entries corresponding to 8 bit index values */
		image = VPMT_Image1DAllocate(VPMT_GetPixelFormat(GL_RGBA, GL_UNSIGNED_BYTE), 256);

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		/* init default color value */
		data = image->data;
		data[0] = data[1] = data[2] = data[3] = VPMT_UBYTE_MAX;

		/* override actual dimension */
		image->size.width = 1;
		texture->palette = image;
		texture->validated = GL_FALSE;
	}

	if (!IsCompatibleFormat(format, image->pixelFormat->internalFormat)) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	if (image->size.width < start + count) {
		/* "count is silently clamped so that all modified entries all within the legal range." */
		count = image->size.width - start;

		if (count <= 0) {
			return;
		}
	}

	rect.origin[0] = start;
	rect.origin[1] = 0;
	rect.size.width = srcImage->size.width;
	rect.size.height = 1;

	VPMT_Bitblt(image, &rect, srcImage, NULL);

	texture->validated = GL_FALSE;
}

void VPMT_ExecColorTableImage(VPMT_Context * context, GLenum target, const VPMT_Image1D * srcImage)
{
	VPMT_Texture2D *texture = context->texUnits[context->activeTextureIndex].boundTexture;
	VPMT_Image2D *image = texture->palette;

	assert(target == GL_TEXTURE_2D);

	image = texture->palette;

#ifdef VPMT_SC_RELAX
	if (image &&
		(!IsCompatibleFormat(srcImage->pixelFormat->baseFormat, image->pixelFormat->internalFormat)
		 || image->size.width != srcImage->size.width)) {
		VPMT_Image2DDeallocate(image);
		texture->palette = image = NULL;
		texture->validated = GL_FALSE;
	}

	if (!image) {
#else
	if (image) {
		if (!IsCompatibleFormat
			(srcImage->pixelFormat->baseFormat, image->pixelFormat->internalFormat)
			|| image->size.width != srcImage->size.width) {
			VPMT_INVALID_VALUE(context);
			return;
		}
	} else {
#endif
		/* always allocate at least 256 entries corresponding to 8 bit index values */
		image = VPMT_Image1DAllocate(srcImage->pixelFormat, VPMT_MAX(256, srcImage->size.width));

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
			return;
		}

		/* override actual dimension */
		image->size.width = srcImage->size.width;
		texture->palette = image;
	}

	texture->validated = GL_FALSE;

	if (srcImage) {
		VPMT_Rect rect;

		rect.origin[0] = 0;
		rect.origin[1] = 0;
		rect.size.width = srcImage->size.width;
		rect.size.height = 1;

		VPMT_Bitblt(image, &rect, srcImage, NULL);
	}
}

/* $Id: tex.c 74 2008-11-23 07:25:12Z hmwill $ */
