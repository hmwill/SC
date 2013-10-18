/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Rasterizer functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "raster.h"
#include "frame.h"

#define SUBPIXEL_MASK ((1 << VPMT_SUBPIXEL_BITS) - 1)
#define HALF (1 << (VPMT_SUBPIXEL_BITS - 1))
#define ONE (1 << VPMT_SUBPIXEL_BITS)
#define PRECISION (1.0f / ONE)

/*
** -------------------------------------------------------------------------
** Actual rasterizer code
** -------------------------------------------------------------------------
*/

VPMT_INLINE GLuint RotateRight(GLuint value, GLuint shift)
{
	/* if possible, use intrinsic rotate right instruction */
	return (value >> shift) | (value << (32 - shift));
}

VPMT_INLINE GLuint RotateLeft(GLuint value, GLuint shift)
{
	/* if possible, use intrinsic rotate left instruction */
	return (value << shift) | (value >> (32 - shift));
}

VPMT_INLINE GLint Min(GLint a, GLint b, GLint c)
{
	return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

VPMT_INLINE GLint Min4(GLint a, GLint b, GLint c, GLint d)
{
	GLint ab = VPMT_MIN(a, b);
	GLint cd = VPMT_MIN(c, d);
	return VPMT_MIN(ab, cd);
}

VPMT_INLINE GLint Max(GLint a, GLint b, GLint c)
{
	return a > b ? (a > c ? a : c) : (b > c ? b : c);
}

VPMT_INLINE GLint Max4(GLint a, GLint b, GLint c, GLint d)
{
	GLint ab = VPMT_MAX(a, b);
	GLint cd = VPMT_MAX(c, d);
	return VPMT_MAX(ab, cd);
}

VPMT_INLINE GLboolean GetPolygonStipple(const VPMT_Context * context, GLint x, GLint y)
{
	if (context->polygonStippleEnabled) {
		GLint xoff = x % 32;
		GLint yoff = y % 32;

		GLint offset = yoff * 4 + xoff / 8;

		return (context->polygonStipple.bytes[offset] & (0x80 >> (xoff & 7))) != 0;
	} else {
		return GL_TRUE;
	}
}

/*
** All the state variables needed for a halfspace rasterizer.
*/
typedef struct RasterVariables {
	GLint minx, maxx, miny, maxy;
	VPMT_Vec4i equ, equ_dx, equ_dy;
} RasterVariables;

static void SetupTriangleRasterVariables(RasterVariables * vars, const VPMT_RasterVertex * a,
										 const VPMT_RasterVertex * b, const VPMT_RasterVertex * c)
{

	GLint x1 = a->screenCoords[0], y1 = a->screenCoords[1];
	GLint x2 = b->screenCoords[0], y2 = b->screenCoords[1];
	GLint x3 = c->screenCoords[0], y3 = c->screenCoords[1];

	/* determine /interpolation parameters */
	GLint dxab = x1 - x2;
	GLint dyab = y1 - y2;
	GLint dxca = x3 - x1;
	GLint dyca = y3 - y1;
	GLint dxbc = x2 - x3;
	GLint dybc = y2 - y3;

	/* Bounding rectangle */
	vars->minx = (Min(x1, x2, x3) + HALF) >> VPMT_SUBPIXEL_BITS;
	vars->maxx = (Max(x1, x2, x3) + HALF) >> VPMT_SUBPIXEL_BITS;
	vars->miny = (Min(y1, y2, y3) + HALF) >> VPMT_SUBPIXEL_BITS;
	vars->maxy = (Max(y1, y2, y3) + HALF) >> VPMT_SUBPIXEL_BITS;

	/* Half-edge function values at origin with fill convention */
	vars->equ[0] = y2 * x1 - x2 * y1 + (dyab < 0 || (dyab == 0 && dxab > 0));
	vars->equ[1] = y3 * x2 - x3 * y2 + (dybc < 0 || (dybc == 0 && dxbc > 0));
	vars->equ[2] = y1 * x3 - x1 * y3 + (dyca < 0 || (dyca == 0 && dxca > 0));

	vars->equ_dy[0] = -dxab << VPMT_SUBPIXEL_BITS;
	vars->equ_dy[1] = -dxbc << VPMT_SUBPIXEL_BITS;
	vars->equ_dy[2] = -dxca << VPMT_SUBPIXEL_BITS;

	vars->equ_dx[0] = dyab << VPMT_SUBPIXEL_BITS;
	vars->equ_dx[1] = dybc << VPMT_SUBPIXEL_BITS;
	vars->equ_dx[2] = dyca << VPMT_SUBPIXEL_BITS;

	/* determine values for pixel corner of (minx, miny) */
	VPMT_Vec3ScaleAdd(vars->equ, vars->equ_dx, vars->minx, vars->equ);
	VPMT_Vec3ScaleAdd(vars->equ, vars->equ_dy, vars->miny, vars->equ);
	VPMT_Vec3AddBy2(vars->equ, vars->equ, vars->equ_dx);
	VPMT_Vec3AddBy2(vars->equ, vars->equ, vars->equ_dy);
}

static void SetupQuadAARasterVariables(RasterVariables * vars, const VPMT_RasterVertex * a,
									   const VPMT_RasterVertex * b, const VPMT_RasterVertex * c,
									   const VPMT_RasterVertex * d)
{
	GLint x1 = a->screenCoords[0], y1 = a->screenCoords[1];
	GLint x2 = b->screenCoords[0], y2 = b->screenCoords[1];
	GLint x3 = c->screenCoords[0], y3 = c->screenCoords[1];
	GLint x4 = d->screenCoords[0], y4 = d->screenCoords[1];

	/* determine interpolation parameters */
	GLint dxab = x1 - x2;
	GLint dyab = y1 - y2;
	GLint dxbc = x2 - x3;
	GLint dybc = y2 - y3;
	GLint dxcd = x3 - x4;
	GLint dycd = y3 - y4;
	GLint dxda = x4 - x1;
	GLint dyda = y4 - y1;

	/* Bounding rectangle */
	vars->minx = (Min4(x1, x2, x3, x4)) >> VPMT_SUBPIXEL_BITS;
	vars->maxx = ((Max4(x1, x2, x3, x4)) >> VPMT_SUBPIXEL_BITS) + 1;
	vars->miny = (Min4(y1, y2, y3, y4)) >> VPMT_SUBPIXEL_BITS;
	vars->maxy = ((Max4(y1, y2, y3, y4)) >> VPMT_SUBPIXEL_BITS) + 1;

	/* Half-edge function values at origin with fill convention */
	vars->equ[0] = y2 * x1 - x2 * y1 + (dyab < 0 || (dyab == 0 && dxab > 0));
	vars->equ[1] = y3 * x2 - x3 * y2 + (dybc < 0 || (dybc == 0 && dxbc > 0));
	vars->equ[2] = y4 * x3 - x4 * y3 + (dycd < 0 || (dycd == 0 && dxcd > 0));
	vars->equ[3] = y1 * x4 - x1 * y4 + (dyda < 0 || (dyda == 0 && dxda > 0));

	vars->equ_dy[0] = -dxab << VPMT_SUBPIXEL_BITS;
	vars->equ_dy[1] = -dxbc << VPMT_SUBPIXEL_BITS;
	vars->equ_dy[2] = -dxcd << VPMT_SUBPIXEL_BITS;
	vars->equ_dy[3] = -dxda << VPMT_SUBPIXEL_BITS;

	vars->equ_dx[0] = dyab << VPMT_SUBPIXEL_BITS;
	vars->equ_dx[1] = dybc << VPMT_SUBPIXEL_BITS;
	vars->equ_dx[2] = dycd << VPMT_SUBPIXEL_BITS;
	vars->equ_dx[3] = dyda << VPMT_SUBPIXEL_BITS;

	/* determine values for pixel corner of (minx, miny) */
	VPMT_Vec4ScaleAdd(vars->equ, vars->equ_dx, vars->minx, vars->equ);
	VPMT_Vec4ScaleAdd(vars->equ, vars->equ_dy, vars->miny, vars->equ);
}

typedef struct Interpolants {
	GLfloat depth;
	GLfloat invW;
	VPMT_Vec4 rgba;
	VPMT_Vec2 texCoordsOverW[VPMT_MAX_TEX_UNITS];
	GLfloat rhoOverW2[VPMT_MAX_TEX_UNITS];
} Interpolants;

typedef struct Interpolation {
	Interpolants current, dx, dy, save;
} Interpolation;

VPMT_INLINE static GLfloat Square(GLfloat value)
{
	return value * value;
}

static GLfloat CalcVertexRho(const VPMT_Texture2D * texture,
							 const GLfloat rsInvZ[2], const GLfloat rsInvZdX[2],
							 const GLfloat rsInvZdY[2], GLfloat invW, GLfloat InvWdX,
							 GLfloat InvWdY)
{
	if (texture->complete &&
		(texture->texMinFilter == GL_NEAREST_MIPMAP_LINEAR ||
		 texture->texMinFilter == GL_NEAREST_MIPMAP_NEAREST ||
		 texture->texMinFilter == GL_LINEAR_MIPMAP_LINEAR ||
		 texture->texMinFilter == GL_LINEAR_MIPMAP_NEAREST)) {
		GLsizei width = texture->mipmaps[0]->size.width;
		GLsizei height = texture->mipmaps[0]->size.height;

		GLfloat A = rsInvZdX[0];
		GLfloat B = rsInvZdY[0];
		GLfloat C = rsInvZ[0];

		GLfloat D = InvWdX;
		GLfloat E = InvWdY;
		GLfloat F = invW;

		GLfloat G = rsInvZdX[1];
		GLfloat H = rsInvZdY[1];
		GLfloat I = rsInvZ[1];

		GLfloat K3 = A * F - C * D;
		GLfloat K4 = G * F - I * D;
		GLfloat K5 = B * F - C * E;
		GLfloat K6 = H * F - I * E;

		GLfloat dudi = Square(K3 * width) + Square(K4 * height);
		GLfloat dvdi = Square(K5 * width) + Square(K6 * height);

		return sqrtf(VPMT_MAX(dudi, dvdi));
	} else {
		return 0.0f;
	}
}

static void InterpolationInit(Interpolation * interp, const VPMT_RasterVertex * a,
							  const VPMT_RasterVertex * b, const VPMT_RasterVertex * c,
							  const VPMT_TexImageUnit units[VPMT_MAX_TEX_UNITS],
							  GLuint rasterInterpolants)
{
	GLsizei index;

	/* determine spanning vectors */
	GLfloat fdxab = (1.0f / (1 << VPMT_SUBPIXEL_BITS)) * (a->screenCoords[0] - b->screenCoords[0]);
	GLfloat fdyab = (1.0f / (1 << VPMT_SUBPIXEL_BITS)) * (a->screenCoords[1] - b->screenCoords[1]);
	GLfloat fdxbc = (1.0f / (1 << VPMT_SUBPIXEL_BITS)) * (b->screenCoords[0] - c->screenCoords[0]);
	GLfloat fdybc = (1.0f / (1 << VPMT_SUBPIXEL_BITS)) * (b->screenCoords[1] - c->screenCoords[1]);

	/* determine area */
	GLfloat area = fdxab * fdybc - fdyab * fdxbc;

	/* initialize to last vertex */
	interp->current.depth = c->depth;
	interp->current.invW = c->invW;

	VPMT_Vec4Copy(interp->current.rgba, c->rgba);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_Vec2Copy(interp->current.texCoordsOverW[index], c->texCoords[index]);
	}

	if (area <= 0.0f) {
		/* effectively flat shade a small area */
		memset(&interp->dx, 0, sizeof(interp->dx));
		memset(&interp->dy, 0, sizeof(interp->dy));
		memset(&interp->current.rhoOverW2, 0, sizeof(interp->current.rhoOverW2));

		return;
	} else {
		GLfloat denominator = 1.0f / area;

		GLfloat scaleBAdX = -fdybc * denominator;
		GLfloat scaleCBdX = fdyab * denominator;
		GLfloat scaleBAdY = fdxbc * denominator;
		GLfloat scaleCBdY = -fdxab * denominator;

		if (rasterInterpolants & VPMT_RasterInterpolateDepth) {
			interp->dx.depth =
				(b->depth - a->depth) * scaleBAdX + (c->depth - b->depth) * scaleCBdX;
			interp->dy.depth =
				(b->depth - a->depth) * scaleBAdY + (c->depth - b->depth) * scaleCBdY;
		}

		if (rasterInterpolants & VPMT_RasterInterpolateInvW) {
			interp->dx.invW = (b->invW - a->invW) * scaleBAdX + (c->invW - b->invW) * scaleCBdX;
			interp->dy.invW = (b->invW - a->invW) * scaleBAdY + (c->invW - b->invW) * scaleCBdY;
		}

		if (rasterInterpolants & VPMT_RasterInterpolateColor) {
			VPMT_Vec4Scale(interp->dx.rgba, b->rgba, scaleBAdX);
			VPMT_Vec4ScaleAdd(interp->dx.rgba, a->rgba, -scaleBAdX, interp->dx.rgba);
			VPMT_Vec4ScaleAdd(interp->dx.rgba, c->rgba, scaleCBdX, interp->dx.rgba);
			VPMT_Vec4ScaleAdd(interp->dx.rgba, b->rgba, -scaleCBdX, interp->dx.rgba);

			VPMT_Vec4Scale(interp->dy.rgba, b->rgba, scaleBAdY);
			VPMT_Vec4ScaleAdd(interp->dy.rgba, a->rgba, -scaleBAdY, interp->dy.rgba);
			VPMT_Vec4ScaleAdd(interp->dy.rgba, c->rgba, scaleCBdY, interp->dy.rgba);
			VPMT_Vec4ScaleAdd(interp->dy.rgba, b->rgba, -scaleCBdY, interp->dy.rgba);
		}

		for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
			const VPMT_Texture2D *texture = units[index].boundTexture;

			if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
				GLfloat rhoAinvW2, rhoBinvW2, rhoCinvW2;
				const VPMT_Texture2D *texture = units[index].boundTexture;

				VPMT_Vec2Scale(interp->dx.texCoordsOverW[index], b->texCoords[index], scaleBAdX);
				VPMT_Vec2ScaleAdd(interp->dx.texCoordsOverW[index], a->texCoords[index], -scaleBAdX,
								  interp->dx.texCoordsOverW[index]);
				VPMT_Vec2ScaleAdd(interp->dx.texCoordsOverW[index], c->texCoords[index], scaleCBdX,
								  interp->dx.texCoordsOverW[index]);
				VPMT_Vec2ScaleAdd(interp->dx.texCoordsOverW[index], b->texCoords[index], -scaleCBdX,
								  interp->dx.texCoordsOverW[index]);

				VPMT_Vec2Scale(interp->dy.texCoordsOverW[index], b->texCoords[index], scaleBAdY);
				VPMT_Vec2ScaleAdd(interp->dy.texCoordsOverW[index], a->texCoords[index], -scaleBAdY,
								  interp->dy.texCoordsOverW[index]);
				VPMT_Vec2ScaleAdd(interp->dy.texCoordsOverW[index], c->texCoords[index], scaleCBdY,
								  interp->dy.texCoordsOverW[index]);
				VPMT_Vec2ScaleAdd(interp->dy.texCoordsOverW[index], b->texCoords[index], -scaleCBdY,
								  interp->dy.texCoordsOverW[index]);

				if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
					rhoAinvW2 =
						CalcVertexRho(texture,
									  a->texCoords[index], interp->dx.texCoordsOverW[index],
									  interp->dy.texCoordsOverW[index], a->invW, interp->dx.invW,
									  interp->dy.invW);

					rhoBinvW2 = CalcVertexRho(texture,
											  b->texCoords[index], interp->dx.texCoordsOverW[index],
											  interp->dy.texCoordsOverW[index], b->invW,
											  interp->dx.invW, interp->dy.invW);

					interp->current.rhoOverW2[index] =
						rhoCinvW2 = CalcVertexRho(texture,
												  c->texCoords[index],
												  interp->dx.texCoordsOverW[index],
												  interp->dy.texCoordsOverW[index], c->invW,
												  interp->dx.invW, interp->dy.invW);

					interp->dx.rhoOverW2[index] =
						(rhoBinvW2 - rhoAinvW2) * scaleBAdX + (rhoCinvW2 - rhoBinvW2) * scaleCBdX;

					interp->dy.rhoOverW2[index] =
						(rhoBinvW2 - rhoAinvW2) * scaleBAdY + (rhoCinvW2 - rhoBinvW2) * scaleCBdY;
				}
			}
		}
	}
}

static VPMT_INLINE void InterpolationMove(Interpolation * interp, GLfloat deltaX, GLfloat deltaY,
										  GLuint rasterInterpolants)
{
	GLsizei index;

	if (rasterInterpolants & VPMT_RasterInterpolateDepth) {
		interp->current.depth += interp->dx.depth * deltaX + interp->dy.depth * deltaY;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateInvW) {
		interp->current.invW += interp->dx.invW * deltaX + interp->dy.invW * deltaY;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateColor) {
		VPMT_Vec4ScaleAdd(interp->current.rgba, interp->dx.rgba, deltaX, interp->current.rgba);
		VPMT_Vec4ScaleAdd(interp->current.rgba, interp->dy.rgba, deltaY, interp->current.rgba);
	}

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2ScaleAdd(interp->current.texCoordsOverW[index],
							  interp->dx.texCoordsOverW[index], deltaX,
							  interp->current.texCoordsOverW[index]);
			VPMT_Vec2ScaleAdd(interp->current.texCoordsOverW[index],
							  interp->dy.texCoordsOverW[index], deltaY,
							  interp->current.texCoordsOverW[index]);
		}

		if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
			interp->current.rhoOverW2[index] +=
				interp->dx.rhoOverW2[index] * deltaX + interp->dy.rhoOverW2[index] * deltaY;
		}
	}
}

static VPMT_INLINE void InterpolationStepX(Interpolation * interp, GLuint rasterInterpolants)
{
	GLsizei index;

	if (rasterInterpolants & VPMT_RasterInterpolateDepth) {
		interp->current.depth += interp->dx.depth;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateInvW) {
		interp->current.invW += interp->dx.invW;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateColor) {
		VPMT_Vec4Add(interp->current.rgba, interp->current.rgba, interp->dx.rgba);
	}

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2Add(interp->current.texCoordsOverW[index],
						 interp->current.texCoordsOverW[index], interp->dx.texCoordsOverW[index]);
		}

		if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
			interp->current.rhoOverW2[index] += interp->dx.rhoOverW2[index];
		}
	}
}

static VPMT_INLINE void InterpolationStepNegX(Interpolation * interp, GLuint rasterInterpolants)
{
	GLsizei index;

	if (rasterInterpolants & VPMT_RasterInterpolateDepth) {
		interp->current.depth -= interp->dx.depth;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateInvW) {
		interp->current.invW -= interp->dx.invW;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateColor) {
		VPMT_Vec4Sub(interp->current.rgba, interp->current.rgba, interp->dx.rgba);
	}

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2Sub(interp->current.texCoordsOverW[index],
						 interp->current.texCoordsOverW[index], interp->dx.texCoordsOverW[index]);
		}

		if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
			interp->current.rhoOverW2[index] -= interp->dx.rhoOverW2[index];
		}
	}
}

static VPMT_INLINE void InterpolationStepY(Interpolation * interp, GLuint rasterInterpolants)
{
	GLsizei index;

	if (rasterInterpolants & VPMT_RasterInterpolateDepth) {
		interp->current.depth += interp->dy.depth;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateInvW) {
		interp->current.invW += interp->dy.invW;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateColor) {
		VPMT_Vec4Add(interp->current.rgba, interp->current.rgba, interp->dy.rgba);
	}

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2Add(interp->current.texCoordsOverW[index],
						 interp->current.texCoordsOverW[index], interp->dy.texCoordsOverW[index]);
		}

		if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
			interp->current.rhoOverW2[index] += interp->dy.rhoOverW2[index];
		}
	}
}

static VPMT_INLINE void InterpolationSave(Interpolation * interp, GLuint rasterInterpolants)
{
	GLsizei index;

	if (rasterInterpolants & VPMT_RasterInterpolateDepth) {
		interp->save.depth = interp->current.depth;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateInvW) {
		interp->save.invW = interp->current.invW;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateColor) {
		VPMT_Vec4Copy(interp->save.rgba, interp->current.rgba);
	}

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2Copy(interp->save.texCoordsOverW[index],
						  interp->current.texCoordsOverW[index]);
		}

		if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
			interp->save.rhoOverW2[index] = interp->current.rhoOverW2[index];
		}
	}
}

static VPMT_INLINE void InterpolationRestore(Interpolation * interp, GLuint rasterInterpolants)
{
	GLsizei index;

	if (rasterInterpolants & VPMT_RasterInterpolateDepth) {
		interp->current.depth = interp->save.depth;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateInvW) {
		interp->current.invW = interp->save.invW;
	}

	if (rasterInterpolants & VPMT_RasterInterpolateColor) {
		VPMT_Vec4Copy(interp->current.rgba, interp->save.rgba);
	}

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2Copy(interp->current.texCoordsOverW[index],
						  interp->save.texCoordsOverW[index]);
		}

		if (rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
			interp->current.rhoOverW2[index] = interp->save.rhoOverW2[index];
		}
	}
}

static VPMT_INLINE VPMT_Color4ub FragmentShader(VPMT_Context * context,
												const Interpolants * current)
{
	GLsizei index;
	GLfloat W = 1.0f / current->invW, W2 = W * W;

	VPMT_Vec2 texCoords[VPMT_MAX_TEX_UNITS];
	GLfloat rho[VPMT_MAX_TEX_UNITS];

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (context->rasterInterpolants & (VPMT_RasterInterpolateTexCoord0 << index)) {
			VPMT_Vec2Scale(texCoords[index], current->texCoordsOverW[index], W);

			if (context->rasterInterpolants & (VPMT_RasterInterpolateRho0 << index)) {
				rho[index] = current->rhoOverW2[index] * W2;
			}
		}
	}

	return VPMT_TexImageUnitsExecute(context->texUnits, VPMT_ConvertVec4ToColor4us(current->rgba),
									 texCoords, rho);
}

static void RasterTriangle(VPMT_Context * context, const VPMT_RasterVertex * a,
						   const VPMT_RasterVertex * b, const VPMT_RasterVertex * c)
{
	GLuint rasterInterpolants = context->rasterInterpolants;
	RasterVariables vars;
	Interpolation interpolation;
	VPMT_FrameBuffer fb;

	GLfloat xStart, yStart, deltaX, deltaY;
	GLint x, y;

	/* Setup the rasterizer variables */
	InterpolationInit(&interpolation, a, b, c, context->texUnits, rasterInterpolants);
	SetupTriangleRasterVariables(&vars, a, b, c);

	vars.maxx =
		VPMT_MIN(vars.maxx,
				 context->activeSurfaceRect.origin[0] + context->activeSurfaceRect.size.width);
	vars.maxy =
		VPMT_MIN(vars.maxy,
				 context->activeSurfaceRect.origin[1] + context->activeSurfaceRect.size.height);
	vars.minx = VPMT_MAX(vars.minx, context->activeSurfaceRect.origin[0]);
	vars.miny = VPMT_MAX(vars.miny, context->activeSurfaceRect.origin[1]);

	/* x and y coordinate of pixel center of min/min-corner of rectangle */
	xStart = vars.minx + 0.5f;
	yStart = vars.miny + 0.5f;

	/* vector from a to (xmin, xmax) pixel center */
	deltaX = xStart - (c->screenCoords[0] * (1.0f / ONE));
	deltaY = yStart - (c->screenCoords[1] * (1.0f / ONE));

	InterpolationMove(&interpolation, deltaX, deltaY, rasterInterpolants);

	if (context->polygonOffsetFillEnabled) {
		interpolation.current.depth +=
			sqrtf(Square(interpolation.dx.depth) +
				  Square(interpolation.dy.depth)) * context->polygonOffsetFactor;

		if (context->writeSurface) {
			interpolation.current.depth += context->polygonOffsetUnit;
		}
	}

	x = vars.minx;

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, vars.minx, vars.miny);

	for (y = vars.miny; y < vars.maxy; y++) {
		VPMT_Vec3i cx;
		GLint xsave;
		GLboolean seenFragment = GL_FALSE;

		VPMT_Vec3Copy(cx, vars.equ);
		InterpolationSave(&interpolation, rasterInterpolants);
		VPMT_FrameBufferSave(&fb);
		xsave = x;

		for (; x < vars.maxx; ++x) {
			if (cx[0] > 0 && cx[1] > 0 && cx[2] > 0) {
				if (GetPolygonStipple(context, x, y)) {
					/* render the fragment */
					VPMT_Color4ub rgba = FragmentShader(context, &interpolation.current);
					VPMT_Fragment(context, &fb, rgba, (GLuint) interpolation.current.depth);
				}

				if (!seenFragment) {
					VPMT_Vec3Copy(vars.equ, cx);
					InterpolationSave(&interpolation, rasterInterpolants);
					VPMT_FrameBufferSave(&fb);
					xsave = x;
					seenFragment = GL_TRUE;
				}
			} else if (seenFragment) {
				break;
			}

			VPMT_Vec3Add(cx, cx, vars.equ_dx);
			InterpolationStepX(&interpolation, rasterInterpolants);
			VPMT_FrameBufferStepX(&fb);
		}

		VPMT_Vec3Copy(cx, vars.equ);
		InterpolationRestore(&interpolation, rasterInterpolants);
		VPMT_FrameBufferRestore(&fb);

		for (x = xsave - 1; x >= vars.minx; --x) {
			VPMT_Vec3Sub(cx, cx, vars.equ_dx);
			InterpolationStepNegX(&interpolation, rasterInterpolants);
			VPMT_FrameBufferStepNegX(&fb);

			if (cx[0] > 0 && cx[1] > 0 && cx[2] > 0) {
				if (GetPolygonStipple(context, x, y)) {
					/* render the fragment */
					VPMT_Color4ub rgba = FragmentShader(context, &interpolation.current);
					VPMT_Fragment(context, &fb, rgba, (GLuint) interpolation.current.depth);
				}

				if (!seenFragment) {
					VPMT_Vec3Copy(vars.equ, cx);
					InterpolationSave(&interpolation, rasterInterpolants);
					VPMT_FrameBufferSave(&fb);
					xsave = x;
					seenFragment = GL_TRUE;
				}
			} else if (seenFragment) {
				break;
			}
		}

		x = xsave;
		VPMT_Vec3Add(vars.equ, vars.equ, vars.equ_dy);
		InterpolationRestore(&interpolation, rasterInterpolants);
		InterpolationStepY(&interpolation, rasterInterpolants);
		VPMT_FrameBufferRestore(&fb);
		VPMT_FrameBufferStepY(&fb);
	}
}

typedef struct Edge {
	GLint x;												   // Current Fix(X) value
	GLint delta;											   // Fix(DX/DY)
} Edge;

#define SWAP(a, b, type) \
	do { type tmp = (a); (a) = (b); (b) = tmp; } while (GL_FALSE)

/* number of fraction bits for edge walking */
#define FRAC_BITS (VPMT_VIEWPORT_COORD_BITS + VPMT_SUBPIXEL_BITS)
#define FRAC_ONE  (1 << FRAC_BITS)
#define FRAC_MASK (FRAC_ONE - 1)

static VPMT_INLINE GLint FixedDiv(GLint a, GLint b)
{
	return (GLint) (((__int64) a << FRAC_BITS) / b);
}

static VPMT_INLINE GLint FixedMul(GLint a, GLint b)
{
	return (GLint) (((__int64) a * b) >> FRAC_BITS);
}

static void RasterTriangleScanLine(VPMT_Context * context, Interpolation * interpolation,
								   VPMT_FrameBuffer * fb, GLsizei length, GLuint stipple)
{
	GLuint rasterInterpolants = context->rasterInterpolants;

	if (length > 0) {
		do {
			// generate a fragment
			if (stipple & 0x80000000u) {
				GLuint depth = (GLuint) interpolation->current.depth;
				VPMT_Color4ub rgba = FragmentShader(context, &interpolation->current);
				VPMT_Fragment(context, fb, rgba, depth);
			}

			stipple = RotateLeft(stipple, 1);

			if (!--length)
				return;

			InterpolationStepX(interpolation, rasterInterpolants);
			VPMT_FrameBufferStepX(fb);
		} while (GL_TRUE);
//  } else if (length < 0) {
//      assert(GL_FALSE);
	}
}

static void RasterTriangleScan(VPMT_Context * context, const VPMT_RasterVertex * a,
							   const VPMT_RasterVertex * b, const VPMT_RasterVertex * c)
{
	GLuint rasterInterpolants = context->rasterInterpolants;
	VPMT_FrameBuffer fb;
	Interpolation interpolation;
	Edge left, right, mid;
	GLint y, startY, midY, endY;
	GLboolean switchLeft;

	/* clip rectangle - this test will become conditional */
	GLint minX = context->activeSurfaceRect.origin[0];
	GLint minY = context->activeSurfaceRect.origin[1];
	GLint maxX = context->activeSurfaceRect.origin[0] + context->activeSurfaceRect.size.width;
	GLint maxY = context->activeSurfaceRect.origin[1] + context->activeSurfaceRect.size.height;

	GLint x1 = (a->screenCoords[0] + HALF) << VPMT_VIEWPORT_COORD_BITS;
	GLint x2 = (b->screenCoords[0] + HALF) << VPMT_VIEWPORT_COORD_BITS;
	GLint x3 = (c->screenCoords[0] + HALF) << VPMT_VIEWPORT_COORD_BITS;
	GLint y1 = (a->screenCoords[1] + HALF) << VPMT_VIEWPORT_COORD_BITS;
	GLint y2 = (b->screenCoords[1] + HALF) << VPMT_VIEWPORT_COORD_BITS;
	GLint y3 = (c->screenCoords[1] + HALF) << VPMT_VIEWPORT_COORD_BITS;

	if (y1 > y2) {
		SWAP(y1, y2, GLint);
		SWAP(x1, x2, GLint);
	}

	if (y1 > y3) {
		SWAP(y1, y3, GLint);
		SWAP(x1, x3, GLint);
	}

	if (y2 > y3) {
		SWAP(y2, y3, GLint);
		SWAP(x2, x3, GLint);
	}

	startY = (y1 - 1) >> FRAC_BITS;
	midY = (y2 - 1) >> FRAC_BITS;
	endY = (y3 - 1) >> FRAC_BITS;

	/* y-culling */
	if (startY >= maxY || endY <= minY || maxY <= minY) {
		return;
	}

	/* not enough y-extent */
	if (endY == startY) {
		return;
	}

	if (midY != endY && midY < maxY) {
		mid.delta = FixedDiv(x3 - x2, y3 - y2);
		mid.x = x2 + FixedMul(mid.delta, (FRAC_ONE - (y2 & FRAC_MASK)) & FRAC_MASK);
	}

	if (midY == startY) {
		midY = endY /* + 1 */ ;
		left = mid;
	} else {
		left.delta = FixedDiv(x2 - x1, y2 - y1);
		left.x = x1 + FixedMul(left.delta, (FRAC_ONE - (y1 & FRAC_MASK)) & FRAC_MASK);
	}

	right.delta = FixedDiv(x3 - x1, y3 - y1);
	right.x = x1 + FixedMul(right.delta, (FRAC_ONE - (y1 & FRAC_MASK)) & FRAC_MASK);

	if ((left.x > right.x) || (left.x == right.x) && (left.delta > right.delta)) {
		SWAP(left, right, Edge);
		switchLeft = GL_FALSE;
	} else {
		switchLeft = GL_TRUE;
	}

	if (startY < minY) {
		y = minY;
		left.x += left.delta * (y - startY);
		right.x += right.delta * (y - startY);
	} else {
		y = startY;
	}

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, 0, y);

	InterpolationInit(&interpolation, a, b, c, context->texUnits, rasterInterpolants);
	InterpolationMove(&interpolation, 0.5f - c->screenCoords[0] * PRECISION,
					  0.5f - c->screenCoords[1] * PRECISION + y, rasterInterpolants);

	if (context->polygonOffsetFillEnabled) {
		interpolation.current.depth +=
			sqrtf(Square(interpolation.dx.depth) +
				  Square(interpolation.dy.depth)) * context->polygonOffsetFactor;

		if (context->writeSurface) {
			interpolation.current.depth += context->polygonOffsetUnit;
		}
	}

	/* clip */
	if (endY >= maxY) {
		endY = maxY;
	}

	if (y >= midY) {
		if (switchLeft) {
			left = mid;
			left.x += left.delta * (y - midY);
		} else {
			right = mid;
			right.x += right.delta * (y - midY);
		}

		goto part2;
	}

	if (midY >= maxY) {
		midY = maxY;
	}

	for (;;) {
		GLint leftBoundary = left.x >> FRAC_BITS;
		GLint rightBoundary = right.x >> FRAC_BITS;
		GLint width;
		GLuint stipple;

		/* clipping - will be conditional */
		leftBoundary = leftBoundary < minX ? minX : leftBoundary;
		rightBoundary = rightBoundary >= maxX ? maxX : rightBoundary;

		if ((width = rightBoundary - leftBoundary) > 0) {
			VPMT_FrameBufferSave(&fb);
			VPMT_FrameBufferMove(&fb, leftBoundary, 0);
			InterpolationSave(&interpolation, rasterInterpolants);
			InterpolationMove(&interpolation, (GLfloat) leftBoundary, 0, rasterInterpolants);

			if (context->polygonStippleEnabled) {
				GLuint offset = y % 32;
				stipple =
					(context->polygonStipple.bytes[offset] << 24) |
					(context->polygonStipple.bytes[offset + 1] << 16) |
					(context->polygonStipple.bytes[offset + 2] << 8) |
					(context->polygonStipple.bytes[offset + 3]);

				stipple = RotateLeft(stipple, leftBoundary % 32);
			} else {
				stipple = ~0u;
			}

			RasterTriangleScanLine(context, &interpolation, &fb, width, stipple);
			VPMT_FrameBufferRestore(&fb);
			InterpolationRestore(&interpolation, rasterInterpolants);
		}

		left.x += left.delta;
		right.x += right.delta;

		/* y is incremented here! */
		if (++y == midY) {
			break;
		}

		VPMT_FrameBufferStepY(&fb);
		InterpolationStepY(&interpolation, rasterInterpolants);
	}

	if (midY == endY) {
		return;
	}

	/* tail increments from previous loop */
	VPMT_FrameBufferStepY(&fb);
	InterpolationStepY(&interpolation, rasterInterpolants);

	if (switchLeft) {
		left = mid;
	} else {
		right = mid;
	}

  part2:
	for (;;) {
		GLint leftBoundary = left.x >> FRAC_BITS;
		GLint rightBoundary = right.x >> FRAC_BITS;
		GLint width;
		GLuint stipple;

		/* clipping - will be conditional */
		leftBoundary = leftBoundary < minX ? minX : leftBoundary;
		rightBoundary = rightBoundary >= maxX ? maxX : rightBoundary;

		if ((width = rightBoundary - leftBoundary) > 0) {
			VPMT_FrameBufferSave(&fb);
			VPMT_FrameBufferMove(&fb, leftBoundary, 0);
			InterpolationSave(&interpolation, rasterInterpolants);
			InterpolationMove(&interpolation, (GLfloat) leftBoundary, 0, rasterInterpolants);

			if (context->polygonStippleEnabled) {
				GLuint offset = y % 32;
				stipple =
					(context->polygonStipple.bytes[offset] << 24) |
					(context->polygonStipple.bytes[offset + 1] << 16) |
					(context->polygonStipple.bytes[offset + 2] << 8) |
					(context->polygonStipple.bytes[offset + 3]);

				stipple = RotateLeft(stipple, leftBoundary % 32);
			} else {
				stipple = ~0u;
			}

			RasterTriangleScanLine(context, &interpolation, &fb, width, stipple);
			VPMT_FrameBufferRestore(&fb);
			InterpolationRestore(&interpolation, rasterInterpolants);
		}

		left.x += left.delta;
		right.x += right.delta;

		/* y is incremented here! */
		if (++y == endY) {
			return;
		}

		VPMT_FrameBufferStepY(&fb);
		InterpolationStepY(&interpolation, rasterInterpolants);
	}
}

void VPMT_RasterPrepareTriangle(VPMT_Context * context)
{
	VPMT_RasterPrepareInterpolants(context);
	//context->rasterTriangle = RasterTriangle;
	context->rasterTriangle = RasterTriangleScan;
}

static VPMT_INLINE GLboolean Inside3(const VPMT_Vec3i values)
{
	return values[0] > 0 && values[1] > 0 && values[2] > 0;
}

static VPMT_INLINE GLboolean Inside4(const VPMT_Vec4i values)
{
	return values[0] > 0 && values[1] > 0 && values[2] > 0 && values[3] > 0;
}

/* coverage as value 0 .. 256 */
static GLuint PixelCoverage(const VPMT_Vec4i base, const VPMT_Vec4i dx, const VPMT_Vec4i dy)
{
	GLsizei count = 0;
	VPMT_Vec4i temp;

	VPMT_Vec4AddBy4(temp, base, dx);
	count += (Inside4(base) != GL_FALSE);
	VPMT_Vec4AddBy2(temp, temp, dx);
	VPMT_Vec4AddBy4(temp, temp, dy);
	count += (Inside4(temp) != GL_FALSE);
	VPMT_Vec4SubBy4(temp, temp, dx);
	VPMT_Vec4AddBy2(temp, temp, dy);
	count += (Inside4(temp) != GL_FALSE);
	VPMT_Vec4SubBy2(temp, temp, dx);
	VPMT_Vec4SubBy4(temp, temp, dy);
	count += (Inside4(temp) != GL_FALSE);

	return count * 0x40u;
}

void VPMT_RasterQuadAA(VPMT_Context * context, const VPMT_RasterVertex * a,
					   const VPMT_RasterVertex * b, const VPMT_RasterVertex * c,
					   const VPMT_RasterVertex * d)
{
	GLuint rasterInterpolants = context->rasterInterpolants;
	RasterVariables vars;
	Interpolation interpolation;
	VPMT_FrameBuffer fb;

	GLfloat xStart, yStart, deltaX, deltaY;
	GLint x, y;

	/* Setup the rasterizer variables */
	InterpolationInit(&interpolation, a, b, c, context->texUnits, rasterInterpolants);
	SetupQuadAARasterVariables(&vars, a, b, c, d);

	vars.maxx =
		VPMT_MIN(vars.maxx,
				 context->activeSurfaceRect.origin[0] + context->activeSurfaceRect.size.width);
	vars.maxy =
		VPMT_MIN(vars.maxy,
				 context->activeSurfaceRect.origin[1] + context->activeSurfaceRect.size.height);
	vars.minx = VPMT_MAX(vars.minx, context->activeSurfaceRect.origin[0]);
	vars.miny = VPMT_MAX(vars.miny, context->activeSurfaceRect.origin[1]);

	/* x and y coordinate of pixel center of min/min-corner of rectangle */
	xStart = vars.minx + 0.5f;
	yStart = vars.miny + 0.5f;

	/* vector from a to (xmin, xmax) pixel center */
	deltaX = xStart - (c->screenCoords[0] * (1.0f / ONE));
	deltaY = yStart - (c->screenCoords[1] * (1.0f / ONE));

	InterpolationMove(&interpolation, deltaX, deltaY, rasterInterpolants);

	x = vars.minx;

	VPMT_FrameBufferInit(&fb, context->writeSurface);
	VPMT_FrameBufferMove(&fb, vars.minx, vars.miny);

	for (y = vars.miny; y < vars.maxy; y++) {
		VPMT_Vec4i cx;
		GLint xsave;
		GLboolean seenFragment = GL_FALSE;

		VPMT_Vec4Copy(cx, vars.equ);
		InterpolationSave(&interpolation, rasterInterpolants);
		VPMT_FrameBufferSave(&fb);
		xsave = x;

		for (; x < vars.maxx; ++x) {
			GLuint coverage = PixelCoverage(cx, vars.equ_dx, vars.equ_dy);

			if (coverage) {
				if (GetPolygonStipple(context, x, y)) {
					/* render the fragment */
					VPMT_Color4ub rgba = FragmentShader(context, &interpolation.current);
					rgba.alpha = (coverage * rgba.alpha) >> 8;
					VPMT_Fragment(context, &fb, rgba, (GLuint) interpolation.current.depth);
				}

				if (!seenFragment) {
					VPMT_Vec4Copy(vars.equ, cx);
					InterpolationSave(&interpolation, rasterInterpolants);
					VPMT_FrameBufferSave(&fb);
					xsave = x;
					seenFragment = GL_TRUE;
				}
			} else if (seenFragment) {
				break;
			}

			VPMT_Vec4Add(cx, cx, vars.equ_dx);
			InterpolationStepX(&interpolation, rasterInterpolants);
			VPMT_FrameBufferStepX(&fb);
		}

		VPMT_Vec4Copy(cx, vars.equ);
		InterpolationRestore(&interpolation, rasterInterpolants);
		VPMT_FrameBufferRestore(&fb);

		for (x = xsave - 1; x >= vars.minx; --x) {
			GLuint coverage;

			VPMT_Vec4Sub(cx, cx, vars.equ_dx);
			InterpolationStepNegX(&interpolation, rasterInterpolants);
			VPMT_FrameBufferStepNegX(&fb);

			coverage = PixelCoverage(cx, vars.equ_dx, vars.equ_dy);

			if (coverage) {
				if (GetPolygonStipple(context, x, y)) {
					/* render the fragment */
					VPMT_Color4ub rgba = FragmentShader(context, &interpolation.current);
					rgba.alpha = (coverage * rgba.alpha) >> 8;
					VPMT_Fragment(context, &fb, rgba, (GLuint) interpolation.current.depth);
				}

				if (!seenFragment) {
					VPMT_Vec4Copy(vars.equ, cx);
					InterpolationSave(&interpolation, rasterInterpolants);
					VPMT_FrameBufferSave(&fb);
					xsave = x;
					seenFragment = GL_TRUE;
				}
			} else if (seenFragment) {
				break;
			}
		}

		x = xsave;
		VPMT_Vec4Add(vars.equ, vars.equ, vars.equ_dy);
		InterpolationRestore(&interpolation, rasterInterpolants);
		InterpolationStepY(&interpolation, rasterInterpolants);
		VPMT_FrameBufferRestore(&fb);
		VPMT_FrameBufferStepY(&fb);
	}
}

/* $Id: rasterpg.c 74 2008-11-23 07:25:12Z hmwill $ */
