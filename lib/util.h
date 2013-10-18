/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Internal utility functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_UTIL_H
#define VPMT_UTIL_H

#define VPMT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VPMT_MAX(a, b) ((a) > (b) ? (a) : (b))
#define VPMT_CLAMP(a)	((a) > 1.0f ? 1.0f : (a) < 0.0f ? 0.0f : (a))
#define VPMT_SELECT(sel, a, b)		((sel) ? (a) : (b))
#define VPMT_ROUND(a)	((GLint) ((a) + 0.5f))
#define VPMT_FRAC(a)	((a) - floorf(a))
#define VPMT_ALIGN(v, a)	((((v) + (a) - 1) / (a)) * (a))
#define VPMT_ELEMENTSOF(a)	((sizeof(a)/sizeof((a)[0])))

#if defined(VPMT_EMULATE_FLOAT_MATH)
#	define VPMT_SINF(a) ((GLfloat) sin(a))
#	define VPMT_COSF(a) ((GLfloat) cos(a))
#	define VPMT_POWF(a, b) ((GLfloat) pow(a, b))
#else
#	define VPMT_SINF(a) sinf(a)
#	define VPMT_COSF(a) cosf(a)
#	define VPMT_POWF(a, b) powf(a, b)
#endif

#define VPMT_UBYTE_TO_FLOAT(a) (((a) & 0xff) * (1.0f/255.0f))
#define VPMT_BYTE_TO_FLOAT(a) ((((a) & 0xff) * 2 + 1) * (1.0f/255.0f))
#define VPMT_FLOAT_TO_UBYTE(a)	(((GLuint)(511 * VPMT_CLAMP(a))) >> 1)

typedef GLint VPMT_Vec2i[2];
typedef GLint VPMT_Vec3i[3];
typedef GLint VPMT_Vec4i[4];

#define VPMT_V2I_MIN(r, a, b) \
	((r)[0] = VPMT_MIN((a)[0], (b)[0]), \
	 (r)[1] = VPMT_MIN((a)[1], (b)[1]))

#define VPMT_V2I_MAX(r, a, b) \
	((r)[0] = VPMT_MAX((a)[0], (b)[0]), \
	 (r)[1] = VPMT_MAX((a)[1], (b)[1]))

typedef GLfloat VPMT_Vec2[2];
typedef GLfloat VPMT_Vec3[3];
typedef GLfloat VPMT_Vec4[4];

#define VPMT_Vec2Copy(r, a)\
	do { (r)[0] = (a)[0]; (r)[1] = (a)[1]; } while(GL_FALSE)

#define VPMT_Vec3Copy(r, a)\
	do { (r)[0] = (a)[0]; (r)[1] = (a)[1]; (r)[2] = (a)[2]; } while(GL_FALSE)

#define VPMT_Vec4Copy(r, a)\
	do { (r)[0] = (a)[0]; (r)[1] = (a)[1]; (r)[2] = (a)[2]; (r)[3] = (a)[3]; } while(GL_FALSE)

#define VPMT_Vec2Cast(r, a, t)\
	do { (r)[0] = (t)(a)[0]; (r)[1] = (t)(a)[1]; } while(GL_FALSE)

#define VPMT_Vec3Cast(r, a, t)\
	do { (r)[0] = (t)(a)[0]; (r)[1] = (t)(a)[1]; (r)[2] = (t)(a)[2]; } while(GL_FALSE)

#define VPMT_Vec4Cast(r, a, t)\
	do { (r)[0] = (t)(a)[0]; (r)[1] = (t)(a)[1]; (r)[2] = (t)(a)[2]; (r)[3] = (t)(a)[3]; } while(GL_FALSE)

#define VPMT_Vec2Clamp(r, a)\
	do { (r)[0] = VPMT_CLAMP((a)[0]); (r)[1] = VPMT_CLAMP((a)[1]); } while(GL_FALSE)

#define VPMT_Vec3Clamp(r, a)\
	do { (r)[0] = VPMT_CLAMP((a)[0]); (r)[1] = VPMT_CLAMP((a)[1]); (r)[2] = VPMT_CLAMP((a)[2]); } while(GL_FALSE)

#define VPMT_Vec4Clamp(r, a)\
	do { (r)[0] = VPMT_CLAMP((a)[0]); (r)[1] = VPMT_CLAMP((a)[1]); \
	(r)[2] = VPMT_CLAMP((a)[2]); (r)[3] = VPMT_CLAMP((a)[3]); } while(GL_FALSE)

#define VPMT_MatrixCopy(r, a)\
	do { GLsizei idx; for (idx = 0; idx < 16; ++idx) (r)[idx] = (a)[idx]; } while(GL_FALSE)

#define VPMT_Vec2Add(r, a, b)\
	do { (r)[0] = (a)[0] + (b)[0]; (r)[1] = (a)[1] + (b)[1]; } while(GL_FALSE)

#define VPMT_Vec3Add(r, a, b)\
	do { (r)[0] = (a)[0] + (b)[0]; (r)[1] = (a)[1] + (b)[1]; (r)[2] = (a)[2] + (b)[2]; } while(GL_FALSE)

#define VPMT_Vec4Add(r, a, b)\
	do { (r)[0] = (a)[0] + (b)[0]; (r)[1] = (a)[1] + (b)[1]; (r)[2] = (a)[2] + (b)[2]; (r)[3] = (a)[3] + (b)[3]; } while(GL_FALSE)

#define VPMT_Vec2Sub(r, a, b)\
	do { (r)[0] = (a)[0] - (b)[0]; (r)[1] = (a)[1] - (b)[1]; } while(GL_FALSE)

#define VPMT_Vec3Sub(r, a, b)\
	do { (r)[0] = (a)[0] - (b)[0]; (r)[1] = (a)[1] - (b)[1]; (r)[2] = (a)[2] - (b)[2]; } while(GL_FALSE)

#define VPMT_Vec4Sub(r, a, b)\
	do { (r)[0] = (a)[0] - (b)[0]; (r)[1] = (a)[1] - (b)[1]; (r)[2] = (a)[2] - (b)[2]; (r)[3] = (a)[3] - (b)[3]; } while(GL_FALSE)

#define VPMT_Vec2Mul(r, a, b)\
	do { (r)[0] = (a)[0] * (b)[0]; (r)[1] = (a)[1] * (b)[1]; } while(GL_FALSE)

#define VPMT_Vec3Mul(r, a, b)\
	do { (r)[0] = (a)[0] * (b)[0]; (r)[1] = (a)[1] * (b)[1]; (r)[2] = (a)[2] * (b)[2]; } while(GL_FALSE)

#define VPMT_Vec4Mul(r, a, b)\
	do { (r)[0] = (a)[0] * (b)[0]; (r)[1] = (a)[1] * (b)[1]; (r)[2] = (a)[2] * (b)[2]; (r)[3] = (a)[3] * (b)[3]; } while(GL_FALSE)

#define VPMT_Vec2Scale(r, a, b)\
	do { (r)[0] = (a)[0] * (b); (r)[1] = (a)[1] * (b); } while(GL_FALSE)

#define VPMT_Vec3Scale(r, a, b)\
	do { (r)[0] = (a)[0] * (b); (r)[1] = (a)[1] * (b); (r)[2] = (a)[2] * (b); } while(GL_FALSE)

#define VPMT_Vec4Scale(r, a, b)\
	do { (r)[0] = (a)[0] * (b); (r)[1] = (a)[1] * (b); (r)[2] = (a)[2] * (b); (r)[3] = (a)[3] * (b); } while(GL_FALSE)

#define VPMT_Vec2MulAdd(r, a, b, c)\
	do { (r)[0] = (a)[0] * (b)[0] + (c)[0]; (r)[1] = (a)[1] * (b)[1] + (c)[1]; } while(GL_FALSE)

#define VPMT_Vec3MulAdd(r, a, b, c)\
	do { (r)[0] = (a)[0] * (b)[0] + (c)[0]; (r)[1] = (a)[1] * (b)[1] + (c)[1]; \
	(r)[2] = (a)[2] * (b)[2] + (c)[2]; } while(GL_FALSE)

#define VPMT_Vec4MulAdd(r, a, b, c)\
	do { (r)[0] = (a)[0] * (b)[0] + (c)[0]; (r)[1] = (a)[1] * (b)[1] + (c)[1]; \
	(r)[2] = (a)[2] * (b)[2] + (c)[2]; (r)[3] = (a)[3] * (b)[3] + (c)[3]; } while(GL_FALSE)

#define VPMT_Vec2ScaleAdd(r, a, b, c)\
	do { (r)[0] = (a)[0] * (b) + (c)[0]; (r)[1] = (a)[1] * (b) + (c)[1]; } while(GL_FALSE)

#define VPMT_Vec3ScaleAdd(r, a, b, c)\
	do { (r)[0] = (a)[0] * (b) + (c)[0]; (r)[1] = (a)[1] * (b) + (c)[1]; \
	(r)[2] = (a)[2] * (b) + (c)[2]; } while(GL_FALSE)

#define VPMT_Vec4ScaleAdd(r, a, b, c)\
	do { (r)[0] = (a)[0] * (b) + (c)[0]; (r)[1] = (a)[1] * (b) + (c)[1]; \
	(r)[2] = (a)[2] * (b) + (c)[2]; (r)[3] = (a)[3] * (b) + (c)[3]; } while(GL_FALSE)

#define VPMT_Vec3AddBy2(r, a, b)\
	do { (r)[0] = (a)[0] + ((b)[0] >> 1); (r)[1] = (a)[1] + ((b)[1] >> 1); (r)[2] = (a)[2] + ((b)[2] >> 1); } while(GL_FALSE)

#define VPMT_Vec4AddBy2(r, a, b)\
	do { (r)[0] = (a)[0] + ((b)[0] >> 1); (r)[1] = (a)[1] + ((b)[1] >> 1); (r)[2] = (a)[2] + ((b)[2] >> 1); (r)[3] = (a)[3] + ((b)[3] >> 1); } while(GL_FALSE)

#define VPMT_Vec4AddBy4(r, a, b)\
	do { (r)[0] = (a)[0] + ((b)[0] >> 2); (r)[1] = (a)[1] + ((b)[1] >> 2); (r)[2] = (a)[2] + ((b)[2] >> 2); (r)[3] = (a)[3] + ((b)[3] >> 2); } while(GL_FALSE)

#define VPMT_Vec4SubBy2(r, a, b)\
	do { (r)[0] = (a)[0] - ((b)[0] >> 1); (r)[1] = (a)[1] - ((b)[1] >> 1); (r)[2] = (a)[2] - ((b)[2] >> 1); (r)[3] = (a)[3] - ((b)[3] >> 1); } while(GL_FALSE)

#define VPMT_Vec4SubBy4(r, a, b)\
	do { (r)[0] = (a)[0] - ((b)[0] >> 2); (r)[1] = (a)[1] - ((b)[1] >> 2); (r)[2] = (a)[2] - ((b)[2] >> 2); (r)[3] = (a)[3] - ((b)[3] >> 2); } while(GL_FALSE)

#define VPMT_Vec2Lerp(r, a, b, c)\
	do { (r)[0] = (a)[0] + ((b)[0] - (a)[0]) * (c); \
	(r)[1] = (a)[1] + ((b)[1] - (a)[1]) * (c); } while(GL_FALSE);

#define VPMT_Vec4Lerp(r, a, b, c)\
	do { (r)[0] = (a)[0] + ((b)[0] - (a)[0]) * (c); \
	(r)[1] = (a)[1] + ((b)[1] - (a)[1]) * (c); \
	(r)[2] = (a)[2] + ((b)[2] - (a)[2]) * (c); \
	(r)[3] = (a)[3] + ((b)[3] - (a)[3]) * (c); } while(GL_FALSE);

#define VPMT_Vec2Dot(a, b)	((a)[0] * (b)[0] + (a)[1] * (b)[1])
#define VPMT_Vec3Dot(a, b)	((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
#define VPMT_Vec4Dot(a, b)	((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2] + (a)[3] * (b)[3])

typedef GLboolean VPMT_Vec2b[2];
typedef GLboolean VPMT_Vec3b[3];
typedef GLboolean VPMT_Vec4b[4];

typedef struct VPMT_Size {
	GLsizei width, height;
} VPMT_Size;

typedef struct VPMT_Sizef {
	GLfloat width, height;
} VPMT_Sizef;

typedef struct VPMT_Rect {
	VPMT_Vec2i origin;
	VPMT_Size size;
} VPMT_Rect;

void VPMT_IntersectRect(VPMT_Rect * result, const VPMT_Rect * a, const VPMT_Rect * b);

typedef struct VPMT_Rectf {
	VPMT_Vec2 origin;
	VPMT_Sizef size;
} VPMT_Rectf;

typedef struct VPMT_Color4ub {
	GLubyte red;
	GLubyte green;
	GLubyte blue;
	GLubyte alpha;
} VPMT_Color4ub;

typedef struct VPMT_Color4us {
	GLushort red;
	GLushort green;
	GLushort blue;
	GLushort alpha;
} VPMT_Color4us;

typedef struct VPMT_Color4f {
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;
} VPMT_Color4f;

static VPMT_INLINE GLubyte VPMT_UByteMul(GLubyte a, GLubyte b)
{
	GLushort c = (GLushort) a * (GLushort) b;

	return (c + (c >> 8) + 1) >> 8;
}

static VPMT_INLINE GLushort VPMT_UShortMul(GLushort a, GLushort b)
{
	GLuint c = (GLuint) a * (GLuint) b;

	return (c + (c >> 16) + 1) >> 16;
}

static VPMT_INLINE GLubyte VPMT_UByteMul256(GLubyte a, GLuint b)
{
	return ((a * b) + 0x80u) >> 8;
}

static VPMT_INLINE VPMT_Color4ub VPMT_ConvertVec4ToColor4ub(const GLfloat * rgba)
{
	VPMT_Color4ub retval;

	retval.red = (GLubyte) (0xff * VPMT_CLAMP(rgba[0]));
	retval.green = (GLubyte) (0xff * VPMT_CLAMP(rgba[1]));
	retval.blue = (GLubyte) (0xff * VPMT_CLAMP(rgba[2]));
	retval.alpha = (GLubyte) (0xff * VPMT_CLAMP(rgba[3]));

	return retval;
}

static VPMT_INLINE VPMT_Color4us VPMT_ConvertVec4ToColor4us(const GLfloat * rgba)
{
	VPMT_Color4us retval;

	retval.red = (GLushort) (0xffff * VPMT_CLAMP(rgba[0]));
	retval.green = (GLushort) (0xffff * VPMT_CLAMP(rgba[1]));
	retval.blue = (GLushort) (0xffff * VPMT_CLAMP(rgba[2]));
	retval.alpha = (GLushort) (0xffff * VPMT_CLAMP(rgba[3]));

	return retval;
}

static VPMT_INLINE VPMT_Color4us VPMT_ConvertColor4ubToColor4us(VPMT_Color4ub color)
{
	VPMT_Color4us retval;

	retval.red = color.red | (color.red << 8);
	retval.green = color.green | (color.green << 8);
	retval.blue = color.blue | (color.blue << 8);
	retval.alpha = color.alpha | (color.alpha << 8);

	return retval;
}

static VPMT_INLINE VPMT_Color4ub VPMT_ConvertColor4usToColor4ub(VPMT_Color4us color)
{
	VPMT_Color4ub retval;

	retval.red = (GLubyte) (color.red >> 8);
	retval.green = (GLubyte) (color.green >> 8);
	retval.blue = (GLubyte) (color.blue >> 8);
	retval.alpha = (GLubyte) (color.alpha >> 8);

	return retval;
}

static VPMT_INLINE GLuint VPMT_Color255To256(GLubyte color)
{
	return color + (color >> 7);
}



#endif

/* $Id: util.h 74 2008-11-23 07:25:12Z hmwill $ */
