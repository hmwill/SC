/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Rendering entry points
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

/*
** -------------------------------------------------------------------------
** Forward declarations
** -------------------------------------------------------------------------
*/

static void VertexPoint(VPMT_Context * context);
static void VertexLines(VPMT_Context * context);
static void VertexLineStrip(VPMT_Context * context);
static void VertexLineLoop(VPMT_Context * context);
static void VertexTriangles(VPMT_Context * context);
static void VertexTriangleStrip(VPMT_Context * context);
static void VertexTriangleFan(VPMT_Context * context);

static void EndLineLoop(VPMT_Context * context);

static void TransformVertexLit(VPMT_Context * context, VPMT_Vertex * vertex);
static void TransformVertexUnlit(VPMT_Context * context, VPMT_Vertex * vertex);

static void ProjectVertexToWindowLinear(VPMT_Context * context,
										VPMT_RasterVertex * raster, const VPMT_Vertex * vertex);
static void ProjectVertexToWindowPerspective(VPMT_Context * context,
											 VPMT_RasterVertex * raster,
											 const VPMT_Vertex * vertex);

static void DrawPoint(VPMT_Context * context, VPMT_Vertex * a);
static void DrawLine(VPMT_Context * context, VPMT_Vertex * a, VPMT_Vertex * b);
static void DrawTriangle(VPMT_Context * context, VPMT_Vertex * a, VPMT_Vertex * b, VPMT_Vertex * c);

static void SetTransform(VPMT_Context * context);

/*
** -------------------------------------------------------------------------
** Exported API entry points
** -------------------------------------------------------------------------
*/

void VPMT_ExecBegin(VPMT_Context * context, GLenum mode)
{
	GLsizei index;
	void (*prepareRasterizer) (VPMT_Context * context) = NULL;

	if (context->renderMode != GL_INVALID_MODE) {
		/* no nested begin allowed */
		VPMT_INVALID_OPERATION(context);
		return;
	}

	switch (mode) {
	case GL_POINTS:
		context->vertexFunction = &VertexPoint;
		context->primitiveType = GL_POINTS;
		prepareRasterizer = VPMT_RasterPreparePoint;
		break;

	case GL_LINES:
		context->vertexFunction = &VertexLines;
		context->primitiveType = GL_LINES;
		prepareRasterizer = VPMT_RasterPrepareLine;
		break;

	case GL_LINE_LOOP:
		context->vertexFunction = &VertexLineLoop;
		context->endFunction = &EndLineLoop;
		context->primitiveType = GL_LINES;
		prepareRasterizer = VPMT_RasterPrepareLine;
		break;

	case GL_LINE_STRIP:
		context->vertexFunction = &VertexLineStrip;
		context->primitiveType = GL_LINES;
		prepareRasterizer = VPMT_RasterPrepareLine;
		break;

	case GL_TRIANGLES:
		context->vertexFunction = &VertexTriangles;
		context->primitiveType = GL_TRIANGLES;
		prepareRasterizer = VPMT_RasterPrepareTriangle;
		break;

	case GL_TRIANGLE_FAN:
		context->vertexFunction = &VertexTriangleFan;
		context->primitiveType = GL_TRIANGLES;
		prepareRasterizer = VPMT_RasterPrepareTriangle;
		break;

	case GL_TRIANGLE_STRIP:
		context->vertexFunction = &VertexTriangleStrip;
		context->primitiveType = GL_TRIANGLES;
		prepareRasterizer = VPMT_RasterPrepareTriangle;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}

	context->renderMode = mode;
	context->primitiveState = 0;
	context->nextIndex = 0;

	SetTransform(context);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		if (context->texture2DEnabledMask & (1u << index)) {
			VPMT_Texture2DValidate(context->texUnits[index].boundTexture);
		}
	}

	VPMT_TexImageUnitsPrepare(context->texUnits);

	if (context->writeSurface) {
		context->writeSurface->vtbl->lock(context, context->writeSurface);
	} else {
		VPMT_UpdateActiveSurfaceRect(context, NULL);
	}

	if (prepareRasterizer) {
		prepareRasterizer(context);
	}
}

void VPMT_ExecColor4f(VPMT_Context * context, GLfloat red, GLfloat green, GLfloat blue,
					  GLfloat alpha)
{
	context->color[0] = red;
	context->color[1] = green;
	context->color[2] = blue;
	context->color[3] = alpha;
}

void VPMT_ExecColor4fv(VPMT_Context * context, const GLfloat * v)
{
	if (!v) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_Vec4Copy(context->color, v);
}

void VPMT_ExecColor4ub(VPMT_Context * context, GLubyte red, GLubyte green, GLubyte blue,
					   GLubyte alpha)
{
	context->color[0] = VPMT_UBYTE_TO_FLOAT(red);
	context->color[1] = VPMT_UBYTE_TO_FLOAT(green);
	context->color[2] = VPMT_UBYTE_TO_FLOAT(blue);
	context->color[3] = VPMT_UBYTE_TO_FLOAT(alpha);
}

void VPMT_ExecCullFace(VPMT_Context * context, GLenum mode)
{
	VPMT_NOT_RENDERING(context);

	switch (mode) {
	case GL_FRONT:
	case GL_BACK:
	case GL_FRONT_AND_BACK:
		context->cullFaceMode = mode;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_ExecDepthRangef(VPMT_Context * context, GLclampf zNear, GLclampf zFar)
{
	VPMT_NOT_RENDERING(context);

	context->depthRange[0] = (zNear < 0.0f) ? 0.0f : (zNear > 1.0f) ? 1.0f : zNear;
	context->depthRange[1] = (zFar < 0.0f) ? 0.0f : (zFar > 1.0f) ? 1.0f : zFar;

	context->depthScale = (context->depthRange[1] - context->depthRange[0]) / 2.0f;
	context->depthOffset = (context->depthRange[1] + context->depthRange[0]) / 2.0f;
}

void VPMT_ExecEnd(VPMT_Context * context)
{
	if (context->renderMode == GL_INVALID_MODE) {
		VPMT_INVALID_OPERATION(context);
		return;
	}

	if (context->endFunction) {
		context->endFunction(context);
	}

	context->renderMode = GL_INVALID_MODE;
	context->endFunction = NULL;
	context->transformFunction = NULL;
	context->vertexFunction = NULL;
	context->primitiveType = -1;

	if (context->writeSurface) {
		context->writeSurface->vtbl->unlock(context, context->writeSurface);
	}
}

void VPMT_ExecFinish(VPMT_Context * context)
{
	/* NOP */
}

void VPMT_ExecFlush(VPMT_Context * context)
{
	/* NOP */
}

void VPMT_ExecFrontFace(VPMT_Context * context, GLenum mode)
{
	VPMT_NOT_RENDERING(context);

	if (mode != GL_CCW && mode != GL_CW) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	context->frontFace = mode;
}

void VPMT_ExecMultiTexCoord2f(VPMT_Context * context, GLenum target, GLfloat s, GLfloat t)
{
	GLsizei index;

	if (target < GL_TEXTURE0 || target > GL_TEXTURE0 + VPMT_MAX_TEX_UNITS) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	index = target - GL_TEXTURE0;

	if (index >= VPMT_MAX_TEX_UNITS) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	context->texCoords[index][0] = s;
	context->texCoords[index][1] = t;
}

void VPMT_ExecNormal3f(VPMT_Context * context, GLfloat nx, GLfloat ny, GLfloat nz)
{

	context->normal[0] = nx;
	context->normal[1] = ny;
	context->normal[2] = nz;
}

void VPMT_ExecNormal3fv(VPMT_Context * context, const GLfloat * v)
{
	if (!v) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_Vec3Copy(context->normal, v);
}

void VPMT_ExecRasterPos3f(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Vertex vertex;

	VPMT_NOT_RENDERING(context);

	context->vertex[0] = x;
	context->vertex[1] = y;
	context->vertex[2] = z;

	SetTransform(context);
	context->transformFunction(context, &vertex);

	if (vertex.cc) {
		context->rasterPosValid = GL_FALSE;
	} else {
		context->rasterPosValid = GL_TRUE;

		/* save state for get functions */
		VPMT_Vec2Copy(context->rasterTexCoords[0], vertex.texCoords[0]);
		VPMT_Vec2Copy(context->rasterTexCoords[1], vertex.texCoords[1]);

		ProjectVertexToWindowLinear(context, &context->rasterPos, &vertex);
	}
}

void VPMT_ExecVertex2f(VPMT_Context * context, GLfloat x, GLfloat y)
{
	if (context->renderMode != GL_INVALID_MODE) {
		context->vertex[0] = x;
		context->vertex[1] = y;
		context->vertex[2] = 0.0f;

		context->vertexFunction(context);
	}
}

void VPMT_ExecVertex2fv(VPMT_Context * context, const GLfloat * v)
{
	if (!v) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	if (context->renderMode != GL_INVALID_MODE) {
		context->vertex[0] = v[0];
		context->vertex[1] = v[1];
		context->vertex[2] = 0.0f;

		context->vertexFunction(context);
	}
}

void VPMT_ExecVertex3f(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z)
{
	if (context->renderMode != GL_INVALID_MODE) {
		context->vertex[0] = x;
		context->vertex[1] = y;
		context->vertex[2] = z;

		context->vertexFunction(context);
	}
}

void VPMT_ExecVertex3fv(VPMT_Context * context, const GLfloat * v)
{
	if (!v) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	if (context->renderMode != GL_INVALID_MODE) {
		VPMT_Vec3Copy(context->vertex, v);
		context->vertexFunction(context);
	}
}

void VPMT_ExecViewport(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height)
{
	VPMT_NOT_RENDERING(context);

	if (width < 0 || height < 0) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	context->viewport.origin[0] = x;
	context->viewport.origin[1] = y;
	context->viewport.size.width = width;
	context->viewport.size.height = height;

	context->viewportTransform.origin[0] = x + width * 0.5f;
	context->viewportTransform.origin[1] = y + height * 0.5f;
	context->viewportTransform.size.width = (GLfloat) width *0.5f;
	context->viewportTransform.size.height = (GLfloat) height *0.5f;
}

/*
** --------------------------------------------------------------------------
** Common Primitive Processing
** --------------------------------------------------------------------------
*/

static void VertexPoint(VPMT_Context * context)
{
	context->transformFunction(context, &context->vertexQueue[0]);
	DrawPoint(context, &context->vertexQueue[0]);
}

static void VertexLines(VPMT_Context * context)
{
	context->transformFunction(context, &context->vertexQueue[context->nextIndex++]);

	if (context->nextIndex == 2) {
		VPMT_LineStippleReset(context);
		DrawLine(context, &context->vertexQueue[0], &context->vertexQueue[1]);
		context->nextIndex = 0;
	}
}

static void VertexLineStrip(VPMT_Context * context)
{
	context->transformFunction(context, &context->vertexQueue[context->nextIndex++]);

	if (context->primitiveState != 0) {
		/* determine line orienation based on parity */
		if (context->nextIndex & 1) {
			DrawLine(context, &context->vertexQueue[1], &context->vertexQueue[0]);
		} else {
			DrawLine(context, &context->vertexQueue[0], &context->vertexQueue[1]);
		}
	} else {
		/* remember that we have seen the first vertex */
		context->primitiveState = 1;
	}

	if (context->nextIndex == 2) {
		context->nextIndex = 0;
	}
}

static void VertexLineLoop(VPMT_Context * context)
{
	context->transformFunction(context, &context->vertexQueue[context->nextIndex++]);

	if (context->primitiveState == 2) {
		/* we have seen at least 3 vertices */
		if (context->nextIndex & 1) {
			DrawLine(context, &context->vertexQueue[1], &context->vertexQueue[2]);
		} else {
			DrawLine(context, &context->vertexQueue[2], &context->vertexQueue[1]);
		}
	} else if (context->primitiveState == 1) {
		/* determine line orienation based on parity */
		DrawLine(context, &context->vertexQueue[0], &context->vertexQueue[1]);

		/* we have seen at least 2 vertices */
		context->primitiveState = 2;
	} else {
		/* remember that we have seen the first vertex */
		context->primitiveState = 1;
	}

	if (context->nextIndex == 3) {
		context->nextIndex = 1;
	}
}

static void EndLineLoop(VPMT_Context * context)
{
	if (context->primitiveState == 2) {
		/* index of last vertex written */
		GLuint prevIndex = 3 - context->nextIndex;

		DrawLine(context, &context->vertexQueue[prevIndex], &context->vertexQueue[0]);
	}
}

static void VertexTriangles(VPMT_Context * context)
{
	context->transformFunction(context, context->vertexQueue + context->nextIndex++);

	if (context->nextIndex == 3) {
		DrawTriangle(context, context->vertexQueue + 0, context->vertexQueue + 1,
					 context->vertexQueue + 2);
		context->nextIndex = 0;
	}
}

static void VertexTriangleStrip(VPMT_Context * context)
{
	context->transformFunction(context, context->vertexQueue + context->nextIndex);

	if (context->primitiveState == 3) {
		/* odd triangle */
		GLuint prevIndex =
			(context->nextIndex - 1) >
			context->nextIndex ? context->nextIndex + 2 : context->nextIndex - 1;
		GLuint prevIndex2 =
			(context->nextIndex - 2) >
			context->nextIndex ? context->nextIndex + 1 : context->nextIndex - 2;
		DrawTriangle(context, context->vertexQueue + prevIndex, context->vertexQueue + prevIndex2,
					 context->vertexQueue + context->nextIndex);
		context->primitiveState = 2;
	} else if (context->primitiveState == 2) {
		/* even triangle */
		GLuint prevIndex =
			(context->nextIndex - 1) >
			context->nextIndex ? context->nextIndex + 2 : context->nextIndex - 1;
		GLuint prevIndex2 =
			(context->nextIndex - 2) >
			context->nextIndex ? context->nextIndex + 1 : context->nextIndex - 2;
		DrawTriangle(context, context->vertexQueue + prevIndex2, context->vertexQueue + prevIndex,
					 context->vertexQueue + context->nextIndex);
		context->primitiveState = 3;
	} else {
		/* remember seen a vertex */
		++context->primitiveState;
	}

	if (++context->nextIndex == 3) {
		context->nextIndex = 0;
	}
}

static void VertexTriangleFan(VPMT_Context * context)
{
	context->transformFunction(context, context->vertexQueue + context->nextIndex);

	if (context->primitiveState == 3) {
		/* even triangle */
		GLuint prevIndex =
			(context->nextIndex - 1) >
			context->nextIndex ? context->nextIndex + 2 : context->nextIndex - 1;
		GLuint prevIndex2 =
			(context->nextIndex - 2) >
			context->nextIndex ? context->nextIndex + 1 : context->nextIndex - 2;
		DrawTriangle(context, context->vertexQueue + prevIndex, context->vertexQueue + prevIndex2,
					 context->vertexQueue + context->nextIndex);
		context->primitiveState = 2;
	} else if (context->primitiveState == 2) {
		/* odd triangle */
		GLuint prevIndex =
			(context->nextIndex - 1) >
			context->nextIndex ? context->nextIndex + 2 : context->nextIndex - 1;
		GLuint prevIndex2 =
			(context->nextIndex - 2) >
			context->nextIndex ? context->nextIndex + 1 : context->nextIndex - 2;
		DrawTriangle(context, context->vertexQueue + prevIndex2, context->vertexQueue + prevIndex,
					 context->vertexQueue + context->nextIndex);
		context->primitiveState = 3;
	} else if (context->primitiveState == 1) {
		/* remember seen second vertex */
		context->primitiveState = 2;
	} else if (context->primitiveState == 0) {
		/* remember seen first vertex */
		context->primitiveState = 1;
	}

	if (++context->nextIndex == 3) {
		context->nextIndex = 1;
	}
}

/*
** --------------------------------------------------------------------------
** Rendring functions
** --------------------------------------------------------------------------
*/

/**
 * Determine the clipping flags for the given vertex.
 * 
 * The bits have the following interpretation:
 * <ul>
 * <li>bit 0: negative x
 * <li>bit 1: positive x
 * <li>bit 2: negative y
 * <li>bit 3: positive y
 * <li>bit 4: negative z
 * <li>bit 5: positive z
 * </ul>
 * 
 * @param vertex
 * 		the vertex to process
 */
VPMT_INLINE static void CalcCC(VPMT_Vertex * vertex)
{
	vertex->cc =
		(vertex->vertex[0] < -vertex->vertex[3]) |
		((vertex->vertex[0] > vertex->vertex[3]) << 1) |
		((vertex->vertex[1] < -vertex->vertex[3]) << 2) |
		((vertex->vertex[1] > vertex->vertex[3]) << 3) |
		((vertex->vertex[2] < -vertex->vertex[3]) << 4) |
		((vertex->vertex[2] > vertex->vertex[3]) << 5);
}

static GLfloat Det(VPMT_Vertex * a, VPMT_Vertex * b, VPMT_Vertex * c)
{
	return +a->vertex[0] * (b->vertex[1] * c->vertex[3] - b->vertex[3] * c->vertex[1])
		- a->vertex[1] * (b->vertex[0] * c->vertex[3] - b->vertex[3] * c->vertex[0])
		+ a->vertex[3] * (b->vertex[0] * c->vertex[1] - b->vertex[1] * c->vertex[0]);
}

VPMT_INLINE static GLfloat InterpolateCoord(GLfloat x0f, GLfloat x1f, GLfloat coeff)
{
	return x0f * (1.0f - coeff) + x1f * coeff;
}

static void Interpolate(VPMT_Vertex * newVertex, const VPMT_Vertex * outside,
						const VPMT_Vertex * inside, GLfloat coeff)
{
	GLsizei index;

	VPMT_Vec4Lerp(newVertex->vertex, inside->vertex, outside->vertex, coeff);
	VPMT_Vec4Lerp(newVertex->rgba, inside->rgba, outside->rgba, coeff);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_Vec2Lerp(newVertex->texCoords[index], inside->texCoords[index],
					  outside->texCoords[index], coeff);
	}
}

static GLsizei ClipFrustrum(VPMT_Context * context, VPMT_Vertex ** input, VPMT_Vertex ** temp,
							GLsizei numVertices, VPMT_Vertex *** result)
{
	GLsizei plane;

	VPMT_Vertex **vilist = input;
	VPMT_Vertex **volist = temp;

	VPMT_Vertex *vprev, *vnext;
	GLsizei i, icnt = numVertices, ocnt = 0;
	VPMT_Vertex *nextTemporary = context->tempVertices;

	GLuint cc = 0;

	for (i = 0; i < icnt; ++i) {
		cc |= vilist[i]->cc;
	}

	if (!cc) {
		*result = input;
		return numVertices;
	}

	for (plane = 0; plane < 6; plane++) {
		GLuint p = 1 << plane;
		GLsizei c;
		GLsizei coord = plane >> 1;
		GLboolean inside, prev_inside;

		if (!(cc & p))
			continue;

		cc = 0;
		ocnt = 0;
		vnext = vilist[icnt - 1];

		inside = !(vnext->cc & p);

		for (c = 0; c < icnt; c++) {

			VPMT_Vertex *voutside, *vinside;
			prev_inside = inside;
			vprev = vnext;
			vnext = vilist[c];
			inside = !(vnext->cc & p);

			if (inside ^ prev_inside) {
				GLfloat ci, wi, co, wo;
				GLfloat num, denom, coeff;
				VPMT_Vertex *newVertex;

				if (inside) {
					vinside = vnext;
					voutside = vprev;
				} else {
					voutside = vnext;
					vinside = vprev;
				}

				ci = vinside->vertex[coord];
				wi = vinside->vertex[3];
				co = voutside->vertex[coord];
				wo = voutside->vertex[3];

				if (!(plane & 1)) {
					wi = -wi;
					wo = -wo;
				}

				num = wi - ci;
				denom = co - ci - wo + wi;
				newVertex = nextTemporary++;
				coeff = num / denom;

				Interpolate(newVertex, voutside, vinside, coeff);
				CalcCC(newVertex);
				cc |= newVertex->cc;

				volist[ocnt++] = newVertex;
			}

			if (inside /*&& c != icnt-1 */ ) {
				volist[ocnt++] = vilist[c];
				cc |= vilist[c]->cc;
			}
		}

		{
			/* swap input and output lists */
			VPMT_Vertex **vtlist;
			vtlist = vilist;
			vilist = volist;
			volist = vtlist;
		}

		icnt = ocnt;

		if (!icnt) {
			*result = NULL;
			return 0;
		}

	}

	*result = vilist;
	return icnt;
}

static void TransformVertexLit(VPMT_Context * context, VPMT_Vertex * vertex)
{
	VPMT_Vec4 eyeCoords;
	VPMT_Vec3 transformedNormal;

	/* transform the vertex */
	VPMT_MatrixTransform4x3(context->modelviewMatrixStack.
							base[context->modelviewMatrixStack.current - 1], eyeCoords,
							context->vertex);
	VPMT_MatrixTransform4x4(context->projectionMatrixStack.
							base[context->projectionMatrixStack.current - 1], vertex->vertex,
							eyeCoords);

	/* transform the normal */
	VPMT_MatrixTransform3x3(context->inverseModelView, transformedNormal, context->normal);

	if (context->normalizeEnabled || context->rescaleNormalEnabled) {
		GLfloat sqrLength = VPMT_Vec3Dot(transformedNormal, transformedNormal);

		if (sqrLength) {
			GLfloat factor = 1.0f / sqrtf(sqrLength);
			VPMT_Vec3Scale(transformedNormal, transformedNormal, factor);
		}
#if 0														   /* no proper implementation of rescale normal */
	} else if (context->rescaleNormalEnabled) {
		assert(GL_FALSE);
#endif
	}

	/* calculate the vertex color */
	VPMT_LightVertex(context, vertex->rgba, eyeCoords, transformedNormal, context->color);

	/* copy texture coordinates */
	VPMT_Vec2Copy(vertex->texCoords[0], context->texCoords[0]);
	VPMT_Vec2Copy(vertex->texCoords[1], context->texCoords[1]);

	CalcCC(vertex);
}

static void TransformVertexUnlit(VPMT_Context * context, VPMT_Vertex * vertex)
{
	VPMT_Vec4 eyeCoords;

	/* transform the vertex */
	VPMT_MatrixTransform4x3(context->modelviewMatrixStack.
							base[context->modelviewMatrixStack.current - 1], eyeCoords,
							context->vertex);
	VPMT_MatrixTransform4x4(context->projectionMatrixStack.
							base[context->projectionMatrixStack.current - 1], vertex->vertex,
							eyeCoords);

	/* vertex color as is */
	VPMT_Vec4Clamp(vertex->rgba, context->color);

	/* copy texture coordinates */
	VPMT_Vec2Copy(vertex->texCoords[0], context->texCoords[0]);
	VPMT_Vec2Copy(vertex->texCoords[1], context->texCoords[1]);

	CalcCC(vertex);
}

static GLint FloatToSubPixels(GLfloat value)
{
	return ((GLint) (value * (1 << (VPMT_SUBPIXEL_BITS + 1))) + 1) >> 1;
}

static void ProjectVertexToWindowPerspective(VPMT_Context * context,
											 VPMT_RasterVertex * raster, const VPMT_Vertex * vertex)
{
	GLsizei index;

	GLfloat invW = vertex->vertex[3] ? 1.0f / vertex->vertex[3] : 0.0f;
	GLfloat depth = vertex->vertex[2] * invW * context->depthScale + context->depthOffset;

	raster->invW = invW;
	raster->depth = VPMT_CLAMP(depth) * context->depthFixedPointScale;

	raster->screenCoords[0] =
		FloatToSubPixels(vertex->vertex[0] * invW * context->viewportTransform.size.width +
						 context->viewportTransform.origin[0]);

	raster->screenCoords[1] =
		FloatToSubPixels(vertex->vertex[1] * invW * context->viewportTransform.size.height +
						 context->viewportTransform.origin[1]);

	VPMT_Vec4Copy(raster->rgba, vertex->rgba);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_Vec2Scale(raster->texCoords[index], vertex->texCoords[index], invW);
	}
}

static void ProjectVertexToWindowLinear(VPMT_Context * context,
										VPMT_RasterVertex * raster, const VPMT_Vertex * vertex)
{
	GLsizei index;

	GLfloat invW = vertex->vertex[3] ? 1.0f / vertex->vertex[3] : 0.0f;
	GLfloat depth = vertex->vertex[2] * invW * context->depthScale + context->depthOffset;

	raster->invW = invW;
	raster->depth = VPMT_CLAMP(depth) * context->depthFixedPointScale;

	raster->screenCoords[0] =
		FloatToSubPixels(vertex->vertex[0] * invW * context->viewportTransform.size.width +
						 context->viewportTransform.origin[0]);

	raster->screenCoords[1] =
		FloatToSubPixels(vertex->vertex[1] * invW * context->viewportTransform.size.height +
						 context->viewportTransform.origin[1]);

	VPMT_Vec4Copy(raster->rgba, vertex->rgba);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_Vec2Copy(raster->texCoords[index], vertex->texCoords[index]);
	}
}

static void DrawPoint(VPMT_Context * context, VPMT_Vertex * a)
{
	VPMT_RasterVertex ra;

	if (a->cc) {
		/* 
		 * need clarification if clipping behavior is changed to only clip
		 * against front and back plane.
		 */
		return;
	}

	ProjectVertexToWindowLinear(context, &ra, a);
	context->rasterPoint(context, &ra);
}

static void DrawLine(VPMT_Context * context, VPMT_Vertex * a, VPMT_Vertex * b)
{
	VPMT_Vertex *list[4], **vertices = list, *temp[4];

	if (a->cc & b->cc) {
		/* outside of frustrum */
		return;
	}

	vertices[0] = a;
	vertices[1] = b;

	if (ClipFrustrum(context, vertices, temp, 2, &vertices) >= 2) {
		VPMT_RasterVertex ra, rb;

		ProjectVertexToWindowPerspective(context, &ra, vertices[0]);
		ProjectVertexToWindowPerspective(context, &rb, vertices[1]);

		if (context->shadeModel == GL_FLAT) {
			VPMT_Vec4Copy(ra.rgba, b->rgba);
			VPMT_Vec4Copy(rb.rgba, b->rgba);
		}

		context->rasterLine(context, &ra, &rb);
	}
}

static void DrawTriangle(VPMT_Context * context, VPMT_Vertex * a, VPMT_Vertex * b, VPMT_Vertex * c)
{
	VPMT_Vertex *list[8], **vertices = list, *temp[8];
	GLsizei numVertices, index;
	GLboolean cw, backFace;

	if (a->cc & b->cc & c->cc) {
		/* outside of frustrum */
		return;
	}

	cw = Det(a, b, c) < 0.0f;
	backFace = cw ^ (context->frontFace == GL_CW);

	if (context->cullFaceEnabled) {
		if ((context->cullFaceMode == GL_FRONT) ^ backFace) {
			return;
		} else if (context->cullFaceMode == GL_FRONT_AND_BACK) {
			return;
		}
	}

	if (cw) {
		vertices[0] = b;
		vertices[1] = a;
	} else {
		vertices[0] = a;
		vertices[1] = b;
	}

	vertices[2] = c;

	numVertices = ClipFrustrum(context, vertices, temp, 3, &vertices);

	if (numVertices >= 3) {
		VPMT_RasterVertex ra, rbc[2];

		ProjectVertexToWindowPerspective(context, &ra, vertices[0]);
		ProjectVertexToWindowPerspective(context, rbc + 1, vertices[1]);

		if (context->shadeModel == GL_FLAT) {
			VPMT_Vec4Copy(ra.rgba, c->rgba);
			VPMT_Vec4Copy(rbc[1].rgba, c->rgba);
		}

		for (index = 2; index < numVertices; ++index) {
			GLsizei select = index & 1;

			ProjectVertexToWindowPerspective(context, rbc + select, vertices[index]);

			if (context->shadeModel == GL_FLAT) {
				VPMT_Vec4Copy(rbc[select].rgba, c->rgba);
			}

			context->rasterTriangle(context, &ra, rbc + (1 - select), rbc + select);
		}
	}
}

static void SetTransform(VPMT_Context * context)
{
	if (context->lightingEnabled) {
		context->transformFunction = TransformVertexLit;
		VPMT_MatrixInverse3(context->inverseModelView,
							context->modelviewMatrixStack.base[context->modelviewMatrixStack.
															   current - 1]);
	} else {
		/* Premultiply modelview and projection matrix */
		context->transformFunction = TransformVertexUnlit;
	}
}

/* $Id: render.c 74 2008-11-23 07:25:12Z hmwill $ */
