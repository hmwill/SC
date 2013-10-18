/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Texture Image Unit Functions
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
** Module Local Declarations
** -------------------------------------------------------------------------
*/

/**
 * Extract the mipmap filter from the minfilter texture parameter
 * 
 * @param minFilter
 * 		the minfilter texture parameter
 * @return GL_NONE, GL_NEAREST, or GL_LINEAR
 */
static GLenum GetMipmapFilter(GLenum minFilter)
{
	switch (minFilter) {
	default:
		assert(GL_FALSE);

	case GL_NEAREST:
	case GL_LINEAR:
		return 0;

	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_NEAREST:
		return GL_NEAREST;

	case GL_NEAREST_MIPMAP_LINEAR:
	case GL_LINEAR_MIPMAP_LINEAR:
		return GL_LINEAR;
	}
}

/**
 * Extract the sample filter from the minfilter texture parameter
 * 
 * @param minFilter
 * 		the minfilter texture parameter
 * @return GL_NEAREST, or GL_LINEAR
 */
static GLenum GetSampleFilter(GLenum minFilter)
{
	switch (minFilter) {
	default:
		assert(GL_FALSE);

	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		return GL_NEAREST;

	case GL_LINEAR:
	case GL_LINEAR_MIPMAP_LINEAR:
	case GL_LINEAR_MIPMAP_NEAREST:
		return GL_LINEAR;
	}
}

GLboolean VPMT_TexImageUnitUsesBaseColor(const VPMT_TexImageUnit * unit)
{
	if (unit->enabled) {
		const VPMT_Texture2D *texture = unit->boundTexture;
		GLenum baseFormat = GL_RGBA;

		if (!unit->incomplete) {
			baseFormat = texture->mipmaps[0]->pixelFormat->baseFormat;

#if GL_EXT_paletted_texture
			if (baseFormat == GL_COLOR_INDEX) {
				if (texture->palette) {
					baseFormat = texture->palette->pixelFormat->baseFormat;
				} else {
					baseFormat = GL_RGBA;
				}
			}
#endif
		}

		/* execute blending code */
		switch (unit->envMode) {
		case GL_ADD:
		case GL_BLEND:
		case GL_DECAL:
		case GL_MODULATE:
			return GL_TRUE;

		case GL_REPLACE:
			switch (baseFormat) {
			case GL_LUMINANCE:
			case GL_RGB:
			case GL_ALPHA:
				return GL_TRUE;

			case GL_LUMINANCE_ALPHA:
			case GL_RGBA:
				return GL_FALSE;

			default:
				assert(GL_FALSE);
			}

			break;

		default:
			assert(GL_FALSE);
		}
	}

	return GL_TRUE;
}

static VPMT_INLINE GLushort UShortLerp(GLushort first, GLushort second, GLuint lerp)
{
	GLint diff = (GLint) second - (GLint) first;
	GLint scaledDiff = (diff * (GLint) lerp) >> 16;

	return (GLushort) ((GLint) first + scaledDiff);
}

VPMT_Color4ub VPMT_TexImageUnitsExecute(VPMT_TexImageUnit units[VPMT_MAX_TEX_UNITS],
										VPMT_Color4us color, const VPMT_Vec2 coords[],
										const GLfloat * rho)
{
	GLsizei index;

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_TexImageUnit *unit = units + index;

		if (unit->enabled) {
			VPMT_Color4us texColor;
			const VPMT_Texture2D *texture = unit->boundTexture;
			GLenum baseFormat = GL_RGBA;
			GLuint red0x10000, green0x10000, blue0x10000, alpha0x10000;

			if (!unit->incomplete) {
				baseFormat = texture->mipmaps[0]->pixelFormat->baseFormat;

#if GL_EXT_paletted_texture
				if (baseFormat == GL_COLOR_INDEX) {
					if (texture->palette) {
						baseFormat = texture->palette->pixelFormat->baseFormat;
					} else {
						baseFormat = GL_RGBA;
					}
				}
#endif
			}

			texColor = unit->sampler2D(unit, coords[index], rho ? *rho : 1.0f);

			/* execute blending code */
			switch (unit->envMode) {
			case GL_ADD:
				color.red = VPMT_MIN(color.red + texColor.red, VPMT_USHORT_MAX);
				color.green = VPMT_MIN(color.green + texColor.green, VPMT_USHORT_MAX);
				color.blue = VPMT_MIN(color.blue + texColor.blue, VPMT_USHORT_MAX);
				color.alpha = VPMT_MIN(color.alpha + texColor.alpha, VPMT_USHORT_MAX);
				break;

			case GL_BLEND:
				red0x10000 = (GLuint) texColor.red + (texColor.red >> 15);
				green0x10000 = (GLuint) texColor.green + (texColor.green >> 15);
				blue0x10000 = (GLuint) texColor.blue + (texColor.blue >> 15);

				color.red = UShortLerp(color.red, unit->envColor4us.red, red0x10000);
				color.green = UShortLerp(color.green, unit->envColor4us.green, green0x10000);
				color.blue = UShortLerp(color.blue, unit->envColor4us.blue, blue0x10000);

				color.alpha = VPMT_UShortMul(color.alpha, texColor.alpha);
				break;

			case GL_DECAL:
				alpha0x10000 = (GLuint) texColor.alpha + (texColor.alpha >> 15);
				color.red = UShortLerp(color.red, texColor.red, alpha0x10000);
				color.green = UShortLerp(color.green, texColor.green, alpha0x10000);
				color.blue = UShortLerp(color.blue, texColor.blue, alpha0x10000);
				break;

			case GL_MODULATE:
				if (baseFormat != GL_ALPHA) {
					color.red = VPMT_UShortMul(color.red, texColor.red);
					color.green = VPMT_UShortMul(color.green, texColor.green);
					color.blue = VPMT_UShortMul(color.blue, texColor.blue);
				}

				color.alpha = VPMT_UShortMul(color.alpha, texColor.alpha);

				break;

			case GL_REPLACE:
				switch (baseFormat) {
				case GL_LUMINANCE:
				case GL_RGB:
					color.red = texColor.red;
					color.green = texColor.green;
					color.blue = texColor.blue;
					break;

				case GL_ALPHA:
					color.alpha = texColor.alpha;
					break;

				case GL_LUMINANCE_ALPHA:
				case GL_RGBA:
					color = texColor;
					break;

				default:
					assert(GL_FALSE);
				}

				break;

			default:
				assert(GL_FALSE);
			}

			if (rho)
				rho += 1;
		}
	}

	return VPMT_ConvertColor4usToColor4ub(color);
}

#if 0
static VPMT_INLINE GLfloat Log2f(GLfloat value)
{
	GLfloat fraction;
	GLint exponent;

	fraction = frexpf(value, &exponent);

	return ((-1.0f / 3) * fraction + 2) * fraction - 2.0f / 3 + (exponent - 1);
}
#else
static GLfloat Log2f(GLfloat value)
{
	union {
		GLfloat f;
		GLint i;
	} numberbits;

	GLint log2;

	numberbits.f = value;
	log2 = ((numberbits.i >> VPMT_FLOAT_MASTISSA_BITS) & VPMT_UBYTE_MAX) - 128;
	numberbits.i &= ~(VPMT_UBYTE_MAX << VPMT_FLOAT_MASTISSA_BITS);
	numberbits.i += VPMT_BYTE_MAX << VPMT_FLOAT_MASTISSA_BITS;
	numberbits.f = ((-1.0f / 3) * numberbits.f + 2) * numberbits.f - 2.0f / 3;

	return numberbits.f + log2;
}
#endif

#define TEX_COORD_FRAC_BITS	(VPMT_MAX_MIPMAP_LEVEL + 8)
#define TEX_COORD_ONE		(1 << TEX_COORD_FRAC_BITS)
#define TEX_COORD_MAX_FRAC	(TEX_COORD_ONE - 1)

#define WRAP_TEX_COORD(mode, value) \
	((mode) == GL_REPEAT ? (value) & TEX_COORD_MAX_FRAC : \
	(value) < 0 ? 0 : (value) > TEX_COORD_MAX_FRAC ? TEX_COORD_MAX_FRAC : (value))

VPMT_Color4us Image2DNearest(const VPMT_Image2D * image,
#if GL_EXT_paletted_texture
							 const VPMT_Image1D * palette,
#endif
							 GLfloat s, GLenum wrapS, GLfloat t, GLenum wrapT)
{
	GLuint si = (GLuint) (s * TEX_COORD_ONE);
	GLuint ti = (GLuint) (t * TEX_COORD_ONE);

	GLuint x = (WRAP_TEX_COORD(wrapS, si) * image->size.width) >> TEX_COORD_FRAC_BITS;
	GLuint y = (WRAP_TEX_COORD(wrapT, ti) * image->size.height) >> TEX_COORD_FRAC_BITS;

#if GL_EXT_paletted_texture
	return VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, palette, x, y));
#else
	return VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, x, y));
#endif

}

VPMT_Color4us Image2DLinear(const VPMT_Image2D * image,
#if GL_EXT_paletted_texture
							const VPMT_Image1D * palette,
#endif
							GLfloat s, GLenum wrapS, GLfloat t, GLenum wrapT)
{
	VPMT_Color4us rgba;

	GLuint sil = (GLuint) ((s - image->invSize2.width) * TEX_COORD_ONE);
	GLuint til = (GLuint) ((t - image->invSize2.height) * TEX_COORD_ONE);
	GLuint sih = (GLuint) ((s + image->invSize2.width) * TEX_COORD_ONE);
	GLuint tih = (GLuint) ((t + image->invSize2.height) * TEX_COORD_ONE);

	/* GL_LINEAR */

	VPMT_Color4us ll, lu, ul, uu;
	GLuint silWidth = (WRAP_TEX_COORD(wrapS, sil) * image->size.width);
	GLuint sihWidth = (WRAP_TEX_COORD(wrapS, sih) * image->size.width);
	GLuint tilHeight = (WRAP_TEX_COORD(wrapT, til) * image->size.height);
	GLuint tihHeight = (WRAP_TEX_COORD(wrapT, tih) * image->size.height);

	GLuint xl = silWidth >> TEX_COORD_FRAC_BITS;
	GLuint xf = (silWidth >> (TEX_COORD_FRAC_BITS - 8)) & VPMT_UBYTE_MAX;
	GLuint xu = sihWidth >> TEX_COORD_FRAC_BITS;

	GLuint yl = tilHeight >> TEX_COORD_FRAC_BITS;
	GLuint yf = (tilHeight >> (TEX_COORD_FRAC_BITS - 8)) & VPMT_UBYTE_MAX;
	GLuint yu = tihHeight >> TEX_COORD_FRAC_BITS;

#if GL_EXT_paletted_texture
	ll = VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, palette, xl, yl));
	lu = VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, palette, xl, yu));
	ul = VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, palette, xu, yl));
	uu = VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, palette, xu, yu));
#else
	ll = VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, xl, yl));
	lu = VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, xl, yu));
	ul = VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, xu, yl));
	uu = VPMT_ConvertColor4ubToColor4us(VPMT_Image2DRead(image, xu, yu));
#endif

	rgba.red =
		((ll.red * (0x100 - xf) + ul.red * xf) * (0x100 - yf) +
		 (lu.red * (0x100 - xf) + uu.red * xf) * yf) >> 16;
	rgba.green =
		((ll.green * (0x100 - xf) + ul.green * xf) * (0x100 - yf) +
		 (lu.green * (0x100 - xf) + uu.green * xf) * yf) >> 16;
	rgba.blue =
		((ll.blue * (0x100 - xf) + ul.blue * xf) * (0x100 - yf) +
		 (lu.blue * (0x100 - xf) + uu.blue * xf) * yf) >> 16;
	rgba.alpha =
		((ll.alpha * (0x100 - xf) + ul.alpha * xf) * (0x100 - yf) +
		 (lu.alpha * (0x100 - xf) + uu.alpha * xf) * yf) >> 16;

	return rgba;
}

static VPMT_Color4us Sampler2DIncomplete(const VPMT_TexImageUnit * unit, const GLfloat * coords,
										 GLfloat rho)
{
	VPMT_Color4us rgba;

	/* 
	 * texture samples as 0 vector if mipmapping is requested, but texture
	 * is not complete.
	 */
	rgba.red = rgba.green = rgba.blue = rgba.alpha = 0;

	return rgba;
}

#if GL_EXT_paletted_texture
#define OPT_PALETTE_ARG(texture) ((texture)->palette),
#else
#define OPT_PALETTE_ARG(texture)
#endif

static VPMT_Color4us Sampler2DNoMipmap(const VPMT_TexImageUnit * unit, const GLfloat * coords,
									   GLfloat rho)
{
	const VPMT_Texture2D *texture = unit->boundTexture;
	return unit->imageMinSampler(texture->mipmaps[0], OPT_PALETTE_ARG(texture)
								 coords[0], texture->texWrapS, coords[1], texture->texWrapT);
}

static VPMT_Color4us Sampler2DNearestMipmap(const VPMT_TexImageUnit * unit, const GLfloat * coords,
											GLfloat rho)
{
	const VPMT_Texture2D *texture = unit->boundTexture;

	/* determine mipmap level */
	GLfloat lambda = Log2f(rho);
	GLenum sampleFilter = GetSampleFilter(texture->texMinFilter);

	if (lambda < unit->magMinSwitchOver) {
		/* magnification; use texture mipmap level */
		return unit->imageMagSampler(texture->mipmaps[0], OPT_PALETTE_ARG(texture)
									 coords[0], texture->texWrapS, coords[1], texture->texWrapT);
	} else if (lambda >= texture->maxMipmapLevel) {
		/* clip at max level */
		return unit->imageMinSampler(texture->mipmaps[texture->maxMipmapLevel],
									 OPT_PALETTE_ARG(texture)
									 coords[0], texture->texWrapS, coords[1], texture->texWrapT);
	} else {
		return unit->imageMinSampler(texture->mipmaps[(GLint) lambda], OPT_PALETTE_ARG(texture)
									 coords[0], texture->texWrapS, coords[1], texture->texWrapT);
	}
}

static VPMT_Color4us Sampler2DLinearMipmap(const VPMT_TexImageUnit * unit, const GLfloat * coords,
										   GLfloat rho)
{
	const VPMT_Texture2D *texture = unit->boundTexture;

	/* determine mipmap level */
	GLfloat lambda = Log2f(rho);
	GLenum sampleFilter = GetSampleFilter(texture->texMinFilter);

	if (lambda < unit->magMinSwitchOver) {
		/* magnification; use texture mipmap level */
		return unit->imageMagSampler(texture->mipmaps[0], OPT_PALETTE_ARG(texture)
									 coords[0], texture->texWrapS, coords[1], texture->texWrapT);
	} else if (lambda >= texture->maxMipmapLevel) {
		/* clip at max level */
		return unit->imageMinSampler(texture->mipmaps[texture->maxMipmapLevel],
									 OPT_PALETTE_ARG(texture)
									 coords[0], texture->texWrapS, coords[1], texture->texWrapT);
	} else {
		GLuint mipmapBlend = VPMT_USHORT_MAX & (GLuint) (lambda * (VPMT_USHORT_MAX + 1ul));
		VPMT_Color4us lower, higher, rgba;

		lower = unit->imageMinSampler(texture->mipmaps[(GLint) lambda], OPT_PALETTE_ARG(texture)
									  coords[0], texture->texWrapS, coords[1], texture->texWrapT);
		higher =
			unit->imageMinSampler(texture->mipmaps[(GLint) lambda + 1], OPT_PALETTE_ARG(texture)
								  coords[0], texture->texWrapS, coords[1], texture->texWrapT);

		rgba.red = UShortLerp(lower.red, higher.red, mipmapBlend);
		rgba.green = UShortLerp(lower.green, higher.green, mipmapBlend);
		rgba.blue = UShortLerp(lower.blue, higher.blue, mipmapBlend);
		rgba.alpha = UShortLerp(lower.alpha, higher.alpha, mipmapBlend);

		return rgba;
	}
}

void VPMT_TexImageUnitsInit(VPMT_TexImageUnit units[VPMT_MAX_TEX_UNITS], VPMT_Texture2D * texture2d)
{
	static GLfloat NO_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLsizei index;

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		units[index].boundTexture = texture2d;
		units[index].boundTexture2D = texture2d->name;
		units[index].envMode = GL_MODULATE;
		VPMT_Vec4Copy(units[index].envColor, NO_COLOR);
		units[index].envColor4us = VPMT_ConvertVec4ToColor4us(units[index].envColor);
	}
}

void VPMT_TexImageUnitsPrepare(VPMT_TexImageUnit units[VPMT_MAX_TEX_UNITS])
{
	GLsizei index;

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_TexImageUnit *unit = units + index;

		if (unit->enabled) {
			const VPMT_Texture2D *texture = unit->boundTexture;
			GLenum mipmapFilter = GetMipmapFilter(texture->texMinFilter);
			GLenum sampleFilter = GetSampleFilter(texture->texMinFilter);

			unit->incomplete = !texture->complete;

			if (!texture->complete) {
				unit->sampler2D = Sampler2DIncomplete;
			} else if (mipmapFilter) {
				if (mipmapFilter == GL_NEAREST) {
					unit->sampler2D = Sampler2DNearestMipmap;
				} else {
					unit->sampler2D = Sampler2DLinearMipmap;
				}
			} else {
				unit->sampler2D = Sampler2DNoMipmap;
			}

			if (texture->texMagFilter == GL_NEAREST) {
				unit->imageMagSampler = Image2DNearest;
			} else {
				unit->imageMagSampler = Image2DLinear;
			}

			if (sampleFilter == GL_NEAREST) {
				unit->imageMinSampler = Image2DNearest;
			} else {
				unit->imageMinSampler = Image2DLinear;
			}

			unit->magMinSwitchOver =
				(sampleFilter == GL_NEAREST && texture->texMagFilter == GL_LINEAR) ? 0.5f : 0.0f;
		} else {
			unit->sampler2D = NULL;
			unit->imageMagSampler = NULL;
			unit->imageMinSampler = NULL;
		}
	}
}

/* $Id: texunit.c 74 2008-11-23 07:25:12Z hmwill $ */
