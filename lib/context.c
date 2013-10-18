/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** General GL state and context functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "command.h"
#include "dispatch.h"
#include "exec.h"

/*
** -------------------------------------------------------------------------
** Local Declarations
** -------------------------------------------------------------------------
*/

#ifndef GL_BOOLEAN
#define GL_BOOLEAN 0x9999
#endif

#ifndef GL_ZEROONE
#define GL_ZEROONE 0x9998
#endif

#define OFFSETOF(structure, field)	(((GLubyte *) &((structure *) 0)->field) - (GLubyte *) 0)
#define O(field)	OFFSETOF(VPMT_Context, field)
#define C(field)	-OFFSETOF(Constants, field)
#define ELEMENTSOF(array)	(sizeof(array) / sizeof(array[0]))

typedef struct Constants {
	GLint padding;
	GLint maxElementsIndices;
	GLint maxElementsVertices;
	GLint maxLights;
	GLint maxListNesting;
	GLint maxTextureSize;
	GLint maxTextureUnits;
	GLint maxViewportDims[2];
	GLint subpixelBits;

	GLfloat pointSizeRange[2];
	GLfloat smoothPointSizeRange[2];
	GLfloat smoothPointSizeGranularity;
	GLfloat lineWidthRange[2];
	GLfloat smoothLineWidthRange[2];
	GLfloat smoothLineWidthGranularity;
} Constants;

static const Constants ConstValues = {
	0,														   /* padding                      */
	VPMT_MAX_ELEMENTS_INDICES,								   /* maxElementsIndices           */
	VPMT_MAX_ELEMENTS_VERTICES,								   /* maxElementsVertices          */
	VPMT_MAX_LIGHTS,										   /* maxLights                    */
	1,														   /* maxListNesting               */
	VPMT_MAX_TEXTURE_SIZE,									   /* maxTextureSize               */
	VPMT_MAX_TEX_UNITS,										   /* maxTextureUnits              */

	{														   /* maxViewportDims[2]           */
	 VPMT_MAX_VIEWPORT_WIDTH,
	 VPMT_MAX_VIEWPORT_HEIGHT},

	VPMT_SUBPIXEL_BITS,										   /* subpixelBits                 */

	{														   /* pointSizeRange[2]            */
	 1.0f,
	 VPMT_MAX_POINT_SIZE},

	{														   /* smoothPointSizeRange[2]      */
	 VPMT_MIN_SMOOTH_POINT_SIZE,
	 VPMT_MAX_SMOOTH_POINT_SIZE},

	VPMT_SMOOTH_POINT_SIZE_GRANULARITY,						   /* smoothPointSizeGranularity   */

	{														   /* lineWidthRange[2]            */
	 1.0f,
	 VPMT_MAX_LINE_WIDTH},

	{														   /* smoothLineWidthRange[2]      */
	 VPMT_MIN_SMOOTH_LINE_WIDTH,
	 VPMT_MAX_SMOOTH_LINE_WIDTH},

	VPMT_SMOOTH_LINE_WIDTH_GRANULARITY						   /* smoothLineWidthGranularity   */
};

typedef struct StateVariable {
	GLenum token;											   /* the token identifying the variable   */
	GLenum type;											   /* the base type of the variable        */
	GLsizei elements;										   /* the number of elements               */
	GLsizei offset;											   /* >= 0: the offset within the context, */
	/* < 0: the offset in constant structure */
	GLsizei index;											   /* offset of GLint index variable       */
	GLsizei stride;											   /* stride to apply to index variable    */
} StateVariable;

static const StateVariable StateVariables[] = {
	/*  token,                              type,       el, offset,                     idx,    stride  */
	{GL_RESCALE_NORMAL, GL_BOOLEAN, 1, O(rescaleNormalEnabled)},
	{GL_NORMALIZE, GL_BOOLEAN, 1, O(normalizeEnabled)},
	{GL_LIGHTING, GL_BOOLEAN, 1, O(lightingEnabled)},
	{GL_LIGHT0, GL_BOOLEAN, 1, O(lightEnabled[0])},
	{GL_LIGHT1, GL_BOOLEAN, 1, O(lightEnabled[1])},
	{GL_COLOR_MATERIAL, GL_BOOLEAN, 1, O(colorMaterialEnabled)},
	{GL_POINT_SMOOTH, GL_BOOLEAN, 1, O(pointSmoothEnabled)},
	{GL_LINE_SMOOTH, GL_BOOLEAN, 1, O(lineSmoothEnabled)},
	{GL_LINE_STIPPLE, GL_BOOLEAN, 1, O(lineStippleEnabled)},
	{GL_POLYGON_STIPPLE, GL_BOOLEAN, 1, O(polygonStippleEnabled)},
	{GL_POLYGON_OFFSET_FILL, GL_BOOLEAN, 1, O(polygonOffsetFillEnabled)},
	{GL_CULL_FACE, GL_BOOLEAN, 1, O(cullFaceEnabled)},
	{GL_TEXTURE_2D, GL_BOOLEAN, 1, O(texUnits[0].enabled), O(activeTextureIndex),
	 sizeof(VPMT_TexImageUnit)}
	,
	{GL_SCISSOR_TEST, GL_BOOLEAN, 1, O(scissorTestEnabled)}
	,
	{GL_ALPHA_TEST, GL_BOOLEAN, 1, O(alphaTestEnabled)}
	,
	{GL_BLEND, GL_BOOLEAN, 1, O(blendEnabled)}
	,
	{GL_DEPTH_TEST, GL_BOOLEAN, 1, O(depthTestEnabled)}
	,
	{GL_STENCIL_TEST, GL_BOOLEAN, 1, O(stencilTestEnabled)}
	,

	{GL_ACTIVE_TEXTURE, GL_INT, 1, O(activeTexture)}
	,
	{GL_ALPHA_BITS, GL_INT, 1, O(alphaBits)}
	,
	{GL_ALPHA_TEST_FUNC, GL_INT, 1, O(alphaFunc)}
	,
	{GL_BLEND_DST, GL_INT, 1, O(blendDstFactor)}
	,
	{GL_BLEND_SRC, GL_INT, 1, O(blendSrcFactor)}
	,
	{GL_BLUE_BITS, GL_INT, 1, O(blueBits)}
	,
	{GL_CLIENT_ACTIVE_TEXTURE, GL_INT, 1, O(clientActiveTexture)}
	,
	{GL_COLOR_ARRAY_SIZE, GL_INT, 1, O(colorArray.size)}
	,
	{GL_COLOR_ARRAY_STRIDE, GL_INT, 1, O(colorArray.stride)}
	,
	{GL_COLOR_ARRAY_TYPE, GL_INT, 1, O(colorArray.type)}
	,
	{GL_CULL_FACE_MODE, GL_INT, 1, O(cullFaceMode)}
	,
	{GL_DEPTH_BITS, GL_INT, 1, O(depthBits)}
	,
	{GL_DEPTH_FUNC, GL_INT, 1, O(depthFunc)}
	,
	{GL_FRONT_FACE, GL_INT, 1, O(frontFace)}
	,
	{GL_GREEN_BITS, GL_INT, 1, O(greenBits)}
	,
	{GL_LINE_STIPPLE_PATTERN, GL_INT, 1, O(lineStipplePattern)}
	,
	{GL_LINE_STIPPLE_REPEAT, GL_INT, 1, O(lineStippleRepeat)}
	,
	{GL_LIST_BASE, GL_INT, 1, O(listBase)}
	,
	{GL_MATRIX_MODE, GL_INT, 1, O(matrixMode)}
	,
	{GL_MAX_ELEMENTS_INDICES, GL_INT, 1, C(maxElementsIndices)}
	,
	{GL_MAX_ELEMENTS_VERTICES, GL_INT, 1, C(maxElementsVertices)}
	,
	{GL_MAX_LIGHTS, GL_INT, 1, C(maxLights)}
	,
	{GL_MAX_LIST_NESTING, GL_INT, 1, C(maxListNesting)}
	,
	{GL_MAX_MODELVIEW_STACK_DEPTH, GL_INT, 1, O(modelviewMatrixStack.size)}
	,
	{GL_MAX_PROJECTION_STACK_DEPTH, GL_INT, 1, O(projectionMatrixStack.size)}
	,
	{GL_MAX_TEXTURE_SIZE, GL_INT, 1, C(maxTextureSize)}
	,
	{GL_MAX_TEXTURE_UNITS, GL_INT, 1, C(maxTextureUnits)}
	,
	{GL_MAX_VIEWPORT_DIMS, GL_INT, 2, C(maxViewportDims)}
	,
	{GL_MODELVIEW_STACK_DEPTH, GL_INT, 1, O(modelviewMatrixStack.current)}
	,
	{GL_NORMAL_ARRAY_STRIDE, GL_INT, 1, O(normalArray.stride)}
	,
	{GL_NORMAL_ARRAY_TYPE, GL_INT, 1, O(normalArray.type)}
	,
	{GL_PACK_ALIGNMENT, GL_INT, 1, O(packAlignment)}
	,
	{GL_PERSPECTIVE_CORRECTION_HINT, GL_INT, 1, O(perspectiveCorrectionHint)}
	,
	{GL_POINT_SMOOTH_HINT, GL_INT, 1, O(pointSmoothHint)}
	,
	{GL_PROJECTION_STACK_DEPTH, GL_INT, 1, O(projectionMatrixStack.current)}
	,
	{GL_RED_BITS, GL_INT, 1, O(redBits)}
	,
	{GL_SCISSOR_BOX, GL_INT, 4, O(scissor)}
	,
	{GL_SHADE_MODEL, GL_INT, 1, O(shadeModel)}
	,
	{GL_STENCIL_BITS, GL_INT, 1, O(stencilBits)}
	,
	{GL_STENCIL_CLEAR_VALUE, GL_INT, 1, O(clearStencil)}
	,
	{GL_STENCIL_FAIL, GL_INT, 1, O(stencilOpFail)}
	,
	{GL_STENCIL_FUNC, GL_INT, 1, O(stencilFunc)}
	,
	{GL_STENCIL_PASS_DEPTH_FAIL, GL_INT, 1, O(stencilOpZFail)}
	,
	{GL_STENCIL_PASS_DEPTH_PASS, GL_INT, 1, O(stencilOpZPass)}
	,
	{GL_STENCIL_REF, GL_INT, 1, O(stencilRef)}
	,
	{GL_STENCIL_VALUE_MASK, GL_INT, 1, O(stencilMask)}
	,
	{GL_STENCIL_WRITEMASK, GL_INT, 1, O(stencilWriteMask)}
	,
	{GL_SUBPIXEL_BITS, GL_INT, 1, C(subpixelBits)}
	,
	{GL_TEXTURE_BINDING_2D, GL_INT, 1, O(texUnits[0].boundTexture2D), O(activeTextureIndex),
	 sizeof(VPMT_TexImageUnit)}
	,
	{GL_TEXTURE_COORD_ARRAY_SIZE, GL_INT, 1, O(texCoordArray[0].size), O(clientActiveTextureIndex),
	 sizeof(VPMT_Array)}
	,
	{GL_TEXTURE_COORD_ARRAY_STRIDE, GL_INT, 1, O(texCoordArray[0].stride),
	 O(clientActiveTextureIndex),
	 sizeof(VPMT_Array)}
	,
	{GL_TEXTURE_COORD_ARRAY_TYPE, GL_INT, 1, O(texCoordArray[0].type), O(clientActiveTextureIndex),
	 sizeof(VPMT_Array)}
	,
	{GL_UNPACK_ALIGNMENT, GL_INT, 1, O(unpackAlignment)}
	,
	{GL_VERTEX_ARRAY_SIZE, GL_INT, 1, O(vertexArray.size)}
	,
	{GL_VERTEX_ARRAY_STRIDE, GL_INT, 1, O(vertexArray.stride)}
	,
	{GL_VERTEX_ARRAY_TYPE, GL_INT, 1, O(vertexArray.type)}
	,
	{GL_VIEWPORT, GL_INT, 4, O(viewport)}
	,

	{GL_ALIASED_LINE_WIDTH_RANGE, GL_FLOAT, 2, C(lineWidthRange)}
	,
	{GL_ALIASED_POINT_SIZE_RANGE, GL_FLOAT, 2, C(pointSizeRange)}
	,
	{GL_ALPHA_TEST_REF, GL_FLOAT, 1, O(alphaRef)}
	,
	{GL_COLOR_CLEAR_VALUE, GL_ZEROONE, 4, O(clearColor)}
	,
	{GL_COLOR_WRITEMASK, GL_BOOLEAN, 4, O(colorWriteMask)}
	,
	{GL_DEPTH_WRITEMASK, GL_BOOLEAN, 1, O(depthWriteMask)}
	,
	{GL_CURRENT_COLOR, GL_ZEROONE, 4, O(color)}
	,
	{GL_CURRENT_NORMAL, GL_ZEROONE, 3, O(normal)}
	,
	{GL_CURRENT_RASTER_COLOR, GL_ZEROONE, 4, O(rasterPos.rgba)}
	,
	{GL_CURRENT_RASTER_TEXTURE_COORDS, GL_FLOAT, 4, O(rasterTexCoords), O(activeTextureIndex),
	 sizeof(VPMT_Vec4)}
	,
	{GL_CURRENT_TEXTURE_COORDS, GL_FLOAT, 4, O(texCoords), O(activeTextureIndex), sizeof(VPMT_Vec4)}
	,
	{GL_DEPTH_CLEAR_VALUE, GL_ZEROONE, 1, O(clearDepth)}
	,
	{GL_DEPTH_RANGE, GL_ZEROONE, 2, O(depthRange)}
	,
	{GL_LIGHT_MODEL_AMBIENT, GL_ZEROONE, 4, O(lightModelAmbient)}
	,
	{GL_LINE_WIDTH, GL_FLOAT, 1, O(lineWidth)}
	,
	{GL_MODELVIEW_MATRIX, GL_FLOAT, 16, O(modelviewMatrix[-1]), O(modelviewMatrixStack.current),
	 sizeof(VPMT_Matrix)}
	,
	{GL_POINT_SIZE, GL_FLOAT, 1, O(pointSize)}
	,
	{GL_POLYGON_OFFSET_FACTOR, GL_FLOAT, 1, O(polygonOffsetFactor)}
	,
	{GL_POLYGON_OFFSET_UNITS, GL_FLOAT, 1, O(polygonOffsetUnit)}
	,
	{GL_PROJECTION_MATRIX, GL_FLOAT, 16, O(projectionMatrix[-1]), O(projectionMatrixStack.current),
	 sizeof(VPMT_Matrix)}
	,
	{GL_SMOOTH_LINE_WIDTH_GRANULARITY, GL_FLOAT, 1, C(smoothLineWidthGranularity)}
	,
	{GL_SMOOTH_LINE_WIDTH_RANGE, GL_FLOAT, 2, C(smoothLineWidthRange)}
	,
	{GL_SMOOTH_POINT_SIZE_GRANULARITY, GL_FLOAT, 1, C(smoothPointSizeGranularity)}
	,
	{GL_SMOOTH_POINT_SIZE_RANGE, GL_FLOAT, 2, C(smoothPointSizeRange)}
	,
	{GL_PERSPECTIVE_CORRECTION_HINT, GL_INT, 1, O(perspectiveCorrectionHint)}
	,
	{GL_POINT_SMOOTH_HINT, GL_INT, 1, O(pointSmoothHint)}
	,
	{GL_LINE_SMOOTH_HINT, GL_INT, 1, O(lineSmoothHint)}
	,
};

/*
** -------------------------------------------------------------------------
** Forward Declarations
** -------------------------------------------------------------------------
*/

static void Toggle(VPMT_Context * context, GLenum cap, GLboolean enable);
static const StateVariable *FindVariable(GLenum token);
static const void *VariableAddress(const StateVariable * var, VPMT_Context * context);

/*
** -------------------------------------------------------------------------
** Exported API entry points
** -------------------------------------------------------------------------
*/

void VPMT_ExecDisable(VPMT_Context * context, GLenum cap)
{
	Toggle(context, cap, GL_FALSE);
}

void VPMT_ExecEnable(VPMT_Context * context, GLenum cap)
{
	Toggle(context, cap, GL_TRUE);
}

GLenum VPMT_ExecGetError(VPMT_Context * context)
{
	GLenum result = context->error;
	context->error = GL_NO_ERROR;

	return result;
}

void VPMT_ExecGetBooleanv(VPMT_Context * context, GLenum pname, GLboolean * params)
{
	const StateVariable *var = FindVariable(pname);
	const void *base;
	GLsizei count;

	if (!var) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	base = VariableAddress(var, context);
	count = var->elements;

	switch (var->type) {
	case GL_BOOLEAN:
		{
			const GLubyte *bytes = (const GLubyte *) base;

			while (count--) {
				*params++ = *bytes++;
			}
		}

		break;

	case GL_INT:
		{
			const GLint *ints = (const GLint *) base;

			while (count--) {
				*params++ = (*ints++ != 0);
			}
		}

		break;

	case GL_FLOAT:
	case GL_ZEROONE:
		{
			const GLfloat *floats = (const GLfloat *) base;

			while (count--) {
				*params++ = (*floats++ != 0.0f);
			}
		}

		break;

	default:
		assert(GL_FALSE);
	}
}

void VPMT_ExecGetFloatv(VPMT_Context * context, GLenum pname, GLfloat * params)
{
	const StateVariable *var = FindVariable(pname);
	const void *base;
	GLsizei count;

	if (!var) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	base = VariableAddress(var, context);
	count = var->elements;

	switch (var->type) {
	case GL_BOOLEAN:
		{
			const GLubyte *bytes = (const GLubyte *) base;

			while (count--) {
				*params++ = *bytes++ ? 1.0f : 0.0f;
			}
		}

		break;

	case GL_INT:
		{
			const GLint *ints = (const GLint *) base;

			while (count--) {
				*params++ = (GLfloat) * ints++;
			}
		}

		break;

	case GL_FLOAT:
	case GL_ZEROONE:
		{
			const GLfloat *floats = (const GLfloat *) base;

			while (count--) {
				*params++ = *floats++;
			}
		}

		break;

	default:
		assert(GL_FALSE);
	}
}

void VPMT_ExecGetIntegerv(VPMT_Context * context, GLenum pname, GLint * params)
{
	const StateVariable *var = FindVariable(pname);
	const void *base;
	GLsizei count;

	if (!var) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	VPMT_NOT_RENDERING(context);

	base = VariableAddress(var, context);
	count = var->elements;

	switch (var->type) {
	case GL_BOOLEAN:
		{
			const GLubyte *bytes = (const GLubyte *) base;

			while (count--) {
				*params++ = (*bytes++ != 0);
			}
		}

		break;

	case GL_INT:
		{
			const GLint *ints = (const GLint *) base;

			while (count--) {
				*params++ = *ints++;
			}
		}

		break;

	case GL_FLOAT:
	case GL_ZEROONE:
		{
			const GLfloat *floats = (const GLfloat *) base;
			GLboolean isZeroOne = (var->type == GL_ZEROONE);

			if (isZeroOne) {
				while (count--) {
					GLfloat value = *floats++;

					/* special case for normals, colors, depth range, depth clear value */
					/* c = [(2^32 - 1)f - 1]/2 */

					if (value >= 1.0f) {
						value = 1.0f;
					} else if (value <= -1.0f) {
						value = -1.0f;
					}

					*params++ = (GLint) ((0xffffffffu * value - 1) / 2);
				}
			} else {
				while (count--) {
					GLfloat value = *floats++;

					if (value >= INT_MAX) {
						*params++ = INT_MAX;
					} else if (value <= INT_MIN) {
						*params++ = INT_MIN;
					} else {
						*params++ = (GLint) (value + 0.5f);	   /* round to nearest int */
					}
				}
			}
		}

		break;

	default:
		assert(GL_FALSE);
	}
}

const GLubyte *VPMT_ExecGetString(VPMT_Context * context, GLenum name)
{
	VPMT_NOT_RENDERING_RETURN(context, NULL);

	switch (name) {
	case GL_RENDERER:
		return VPMT_RENDERER;

	case GL_VERSION:
		return VPMT_VERSION;

	case GL_EXTENSIONS:
		return ""
#if GL_OES_single_precision
			" GL_OES_single_precision"
#endif
#if GL_EXT_paletted_texture
			" GL_EXT_paletted_texture"
#endif
			;

	case GL_VENDOR:
		return VPMT_VENDOR;

	default:
		VPMT_INVALID_ENUM(context);
		return 0;
	}
}

void VPMT_ExecHint(VPMT_Context * context, GLenum target, GLenum mode)
{
	VPMT_NOT_RENDERING(context);

	if (mode != GL_DONT_CARE && mode != GL_NICEST && mode != GL_FASTEST) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	switch (target) {
	case GL_PERSPECTIVE_CORRECTION_HINT:
		context->perspectiveCorrectionHint = mode;
		break;

	case GL_POINT_SMOOTH_HINT:
		context->pointSmoothHint = mode;
		break;

	case GL_LINE_SMOOTH_HINT:
		context->lineSmoothHint = mode;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

GLboolean VPMT_ExecIsEnabled(VPMT_Context * context, GLenum cap)
{
	VPMT_NOT_RENDERING_RETURN(context, GL_FALSE);

	switch (cap) {
	case GL_RESCALE_NORMAL:
		return context->rescaleNormalEnabled;

	case GL_NORMALIZE:
		return context->normalizeEnabled;

	case GL_LIGHTING:
		return context->lightingEnabled;

	case GL_LIGHT0:
		return context->lightEnabled[0];

	case GL_LIGHT1:
		return context->lightEnabled[1];

	case GL_COLOR_MATERIAL:
		return context->colorMaterialEnabled;

	case GL_POINT_SMOOTH:
		return context->pointSmoothEnabled;

	case GL_LINE_SMOOTH:
		return context->lineSmoothEnabled;

	case GL_LINE_STIPPLE:
		return context->lineStippleEnabled;

	case GL_POLYGON_STIPPLE:
		return context->polygonStippleEnabled;

	case GL_POLYGON_OFFSET_FILL:
		return context->polygonOffsetFillEnabled;

	case GL_CULL_FACE:
		return context->cullFaceEnabled;

	case GL_TEXTURE_2D:
		return context->texUnits[context->activeTextureIndex].enabled;

	case GL_SCISSOR_TEST:
		return context->scissorTestEnabled;

	case GL_ALPHA_TEST:
		return context->alphaTestEnabled;

	case GL_BLEND:
		return context->blendEnabled;

	case GL_DEPTH_TEST:
		return context->depthTestEnabled;

	case GL_STENCIL_TEST:
		return context->stencilTestEnabled;

	case GL_VERTEX_ARRAY:
		return context->vertexArray.enabled;

	case GL_COLOR_ARRAY:
		return context->colorArray.enabled;

	case GL_NORMAL_ARRAY:
		return context->normalArray.enabled;

	case GL_TEXTURE_COORD_ARRAY:
		return context->texCoordArray[context->clientActiveTextureIndex].enabled;

	default:
		VPMT_INVALID_ENUM(context);
		return GL_FALSE;
	}
}

/*
** -------------------------------------------------------------------------
** Internal functions
** -------------------------------------------------------------------------
*/

GLboolean VPMT_ContextInitialize(VPMT_Context * context)
{
	GLsizei index;
	VPMT_Texture2D *texture2d;

	static GLfloat DARK_GREY[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	static GLfloat LIGHT_GREY[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	static GLfloat BLACK[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static GLfloat WHITE[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat INF_Z[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	static GLfloat NO_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	memset(context, 0, sizeof(context));
	context->dispatch = &VPMT_DispatchExecute;

	/* error context */
	context->error = GL_NO_ERROR;

	/* current primitive type; this needs to happen here so we can call state changing functions */
	context->renderMode = GL_INVALID_MODE;

	/* initialize matrix context */
	VPMT_MatrixStackInitialize(&context->modelviewMatrixStack, context->modelviewMatrix,
							   VPMT_MODELVIEW_STACK_DEPTH);
	VPMT_MatrixStackInitialize(&context->projectionMatrixStack, context->projectionMatrix,
							   VPMT_PROJECTION_STACK_DEPTH);

	context->matrixMode = GL_MODELVIEW;
	context->currentMatrixStack = &context->modelviewMatrixStack;

	/* initialize array context */
	context->clientActiveTexture = GL_TEXTURE0;
	context->clientActiveTextureIndex = 0;

	VPMT_ArrayInitialize(&context->vertexArray);
	VPMT_ArrayInitialize(&context->normalArray);
	VPMT_ArrayInitialize(&context->colorArray);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		VPMT_ArrayInitialize(context->texCoordArray + index);
	}

	/* initialize hint context */
	context->perspectiveCorrectionHint = GL_DONT_CARE;
	context->pointSmoothHint = GL_DONT_CARE;
	context->lineSmoothHint = GL_DONT_CARE;

	/* initialize the viewport */
	VPMT_ExecDepthRangef(context, 0.0f, 1.0f);

	/* various enable/disable settings */
	context->rescaleNormalEnabled = GL_FALSE;
	context->normalizeEnabled = GL_FALSE;

	/* initialize texture context */
	context->activeTexture = GL_TEXTURE0;
	context->activeTextureIndex = 0;

	/* current coordinates etc. */

	context->rasterPosValid = GL_TRUE;
	context->rasterPos.screenCoords[0] = 0;
	context->rasterPos.screenCoords[1] = 0;
	context->rasterPos.invW = context->depthOffset;
	context->rasterPos.depth = 0;

	VPMT_Vec4Copy(context->color, WHITE);
	VPMT_Vec4Copy(context->rasterPos.rgba, WHITE);

	for (index = 0; index < VPMT_MAX_TEX_UNITS; ++index) {
		context->texCoords[index][0] = 0.0f;
		context->texCoords[index][1] = 0.0f;
		context->texCoords[index][2] = 0.0f;
		context->texCoords[index][3] = 1.0f;

		context->rasterTexCoords[index][0] = 0.0f;
		context->rasterTexCoords[index][1] = 0.0f;
		context->rasterTexCoords[index][2] = 0.0f;
		context->rasterTexCoords[index][3] = 1.0f;
	}

	context->normal[0] = context->normal[1] = 0.0f;
	context->normal[2] = 1.0f;

	/* light context */
	context->lightingEnabled = GL_FALSE;

	for (index = 0; index < VPMT_MAX_LIGHTS; ++index) {
		context->lightEnabled[index] = GL_FALSE;

		VPMT_Vec4Copy(context->lightAmbient[index], BLACK);
		VPMT_ExecLightfv(context, GL_LIGHT0 + index, GL_POSITION, INF_Z);
		//VPMT_Vec4Copy(context->lightPosition[index], INF_Z);
	}

	VPMT_Vec4Copy(context->lightDiffuse[0], WHITE);
	VPMT_Vec4Copy(context->lightSpecular[0], WHITE);
	VPMT_Vec4Copy(context->lightDiffuse[1], BLACK);
	VPMT_Vec4Copy(context->lightSpecular[1], BLACK);

	context->colorMaterialEnabled = GL_FALSE;
	context->shadeModel = GL_SMOOTH;

	VPMT_Vec4Copy(context->materialAmbient, DARK_GREY);
	VPMT_Vec4Copy(context->materialDiffuse, LIGHT_GREY);
	VPMT_Vec4Copy(context->materialSpecular, BLACK);
	VPMT_Vec4Copy(context->materialEmission, BLACK);
	context->materialShininess = 0.0f;

	VPMT_Vec4Copy(context->lightModelAmbient, DARK_GREY);

	/* various setup settings */
	context->frontFace = GL_CCW;

	/* rasterization settings */
	context->pointSmoothEnabled = GL_FALSE;
	context->pointSize = 1.0f;

	context->lineSmoothEnabled = GL_FALSE;
	context->lineWidth = 1.0f;
	context->lineStippleEnabled = GL_FALSE;
	context->lineStipplePattern = 0xffff;
	context->lineStippleRepeat = 1;

	context->polygonOffsetFactor = 1.0f;
	context->polygonOffsetUnit = 0.0f;
	context->polygonOffsetFillEnabled = GL_FALSE;
	context->polygonStippleEnabled = GL_FALSE;

	memset(context->polygonStipple.bytes, 0xffu, sizeof(context->polygonStipple.bytes));
	context->cullFaceEnabled = GL_FALSE;
	context->cullFaceMode = GL_BACK;

	/* fragment processing */
	context->scissorTestEnabled = GL_FALSE;
	context->scissor = context->viewport;
	context->alphaTestEnabled = GL_FALSE;
	context->alphaFunc = GL_ALWAYS;
	context->alphaRef = 0.0f;
	context->blendEnabled = GL_FALSE;
	context->depthTestEnabled = GL_FALSE;
	context->stencilTestEnabled = GL_FALSE;

	context->stencilWriteMask = ~0;
	context->colorWriteMask[0] = context->colorWriteMask[1] =
		context->colorWriteMask[2] = context->colorWriteMask[3] = GL_TRUE;
	context->depthWriteMask = GL_TRUE;

	context->depthFunc = GL_LESS;
	context->blendSrcFactor = GL_ONE;
	context->blendDstFactor = GL_ZERO;

	context->stencilFunc = GL_ALWAYS;
	context->stencilRef = 0;
	context->stencilMask = ~0;
	context->stencilOpFail = GL_KEEP;
	context->stencilOpZFail = GL_KEEP;
	context->stencilOpZPass = GL_KEEP;

	/* whole framebuffer context */
	context->clearDepth = 1.0f;
	context->clearStencil = 0;
	VPMT_Vec4Copy(context->clearColor, NO_COLOR);

	/* bitmap context */

	context->packAlignment = 4;
	context->unpackAlignment = 4;

	/* texture context */

	VPMT_HashTableInitialize(&context->textures);

	texture2d = VPMT_Texture2DAllocate(0);

	if (!texture2d) {
		goto cleanup;
	}

	if (!VPMT_HashTableInsert(&context->textures, 0, texture2d)) {
		VPMT_Texture2DDeallocate(texture2d);
		goto cleanup;
	}

	VPMT_TexImageUnitsInit(context->texUnits, texture2d);

	/* list context */
	context->listBase = 0;
	context->listIndex = 0;
	context->listMode = 0;
	context->listCommandsHead = NULL;
	context->listCommandsTail = NULL;

	VPMT_HashTableInitialize(&context->lists);

	/* surfaces and pixel owner ship test */
	context->readSurface = context->writeSurface = NULL;

	return GL_TRUE;

  cleanup:
	VPMT_ContextDeinitialize(context);
	return GL_FALSE;
}

static void FreeTexture(GLuint id, void * obj, void * arg) 
{
	VPMT_Texture2DDeallocate((VPMT_Texture2D *) obj);
}

static void FreeList(GLuint id, void * obj, void * arg) 
{
	VPMT_CommandBufferDispose((VPMT_CommandBuffer *) obj);
}

void VPMT_ContextDeinitialize(VPMT_Context * context)
{
	/* remove references to surfaces */
	VPMT_MakeCurrent(NULL, NULL);

	/* remove all display lists */
	VPMT_HashTableIterate(&context->lists, FreeList, context);
	VPMT_HashTableDeinitialize(&context->lists);

	/* remove all texture data */
	VPMT_HashTableIterate(&context->textures, FreeTexture, context);
	VPMT_HashTableDeinitialize(&context->textures);
}

static void Toggle(VPMT_Context * context, GLenum cap, GLboolean enable)
{
	VPMT_NOT_RENDERING(context);

	switch (cap) {
	case GL_RESCALE_NORMAL:
		context->rescaleNormalEnabled = enable;
		break;

	case GL_NORMALIZE:
		context->normalizeEnabled = enable;
		break;

	case GL_LIGHTING:
		context->lightingEnabled = enable;
		break;

	case GL_LIGHT0:
		context->lightEnabled[0] = enable;
		break;

	case GL_LIGHT1:
		context->lightEnabled[1] = enable;
		break;

	case GL_COLOR_MATERIAL:
		context->colorMaterialEnabled = enable;
		break;

	case GL_POINT_SMOOTH:
		context->pointSmoothEnabled = enable;
		break;

	case GL_LINE_SMOOTH:
		context->lineSmoothEnabled = enable;
		break;

	case GL_LINE_STIPPLE:
		context->lineStippleEnabled = enable;
		break;

	case GL_POLYGON_STIPPLE:
		context->polygonStippleEnabled = enable;
		break;

	case GL_POLYGON_OFFSET_FILL:
		context->polygonOffsetFillEnabled = enable;
		break;

	case GL_CULL_FACE:
		context->cullFaceEnabled = enable;
		break;

	case GL_TEXTURE_2D:
		context->texUnits[context->activeTextureIndex].enabled = enable;

		if (enable) {
			context->texture2DEnabledMask |= 1u << context->activeTextureIndex;
		} else {
			context->texture2DEnabledMask &= ~(1u << context->activeTextureIndex);
		}

		break;

	case GL_SCISSOR_TEST:
		if (context->scissorTestEnabled != enable) {
			context->scissorTestEnabled = enable;
		}

		break;

	case GL_ALPHA_TEST:
		context->alphaTestEnabled = enable;
		break;

	case GL_BLEND:
		context->blendEnabled = enable;
		break;

	case GL_DEPTH_TEST:
		context->depthTestEnabled = enable;
		break;

	case GL_STENCIL_TEST:
		context->stencilTestEnabled = enable;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

static const StateVariable *FindVariable(GLenum token)
{
	GLsizei index;
	GLsizei max = ELEMENTSOF(StateVariables);

	for (index = 0; index < max; ++index) {
		if (StateVariables[index].token == token)
			break;
	}

	if (index == max) {
		return NULL;
	} else {
		return StateVariables + index;
	}
}

static const void *VariableAddress(const StateVariable * var, VPMT_Context * context)
{
	const GLubyte *base;

	if (var->offset >= 0) {
		base = ((const GLubyte *) context) + var->offset;
	} else {
		base = ((const GLubyte *) &ConstValues) - var->offset;
	}

	if (var->stride) {
		const GLubyte *indexAddr = ((const GLubyte *) context) + var->index;
		GLint index = *(const GLint *) indexAddr;
		base += var->stride * index;
	}

	return base;
}

GLAPI GLboolean APIENTRY VPMT_Initialize(void)
{
	return VPMT_ContextInitialize(VPMT_CONTEXT());
}

GLAPI void APIENTRY VPMT_Terminate(void)
{
	VPMT_ContextDeinitialize(VPMT_CONTEXT());
}

GLAPI GLboolean APIENTRY VPMT_MakeCurrent(VPMT_Surface * writeSurface, VPMT_Surface * readSurface)
{
	VPMT_Context *context = VPMT_CONTEXT();

	if (readSurface) {
		readSurface->vtbl->addref(readSurface);
	}

	if (writeSurface) {
		const VPMT_PixelFormat *pixelFormat = writeSurface->image.pixelFormat;
		const VPMT_DepthStencilFormat *depthFormat = writeSurface->depthStencilFormat;

		writeSurface->vtbl->addref(writeSurface);

		context->redBits = pixelFormat->redBits;
		context->greenBits = pixelFormat->greenBits;
		context->blueBits = pixelFormat->blueBits;
		context->alphaBits = pixelFormat->alphaBits;

		context->depthBits = depthFormat->depthBits;
		context->stencilBits = depthFormat->stencilBits;

		context->maxStencilValue = (1ul << context->stencilBits) - 1;
		context->maxDepthValue = (1ul << depthFormat->depthBits) - 1;

		/* for 32-bit depth buffer we are loosing precision, so really only up to D24 is conformant */
		context->depthFixedPointScale = (depthFormat->depthBits > 24) ?
			(GLfloat) (context->maxDepthValue & ~((1 << (depthFormat->depthBits - 24)) - 1)) :
			(GLfloat) context->maxDepthValue;
	}

	if (context->readSurface) {
		context->readSurface->vtbl->release(context->readSurface);
	}

	if (context->writeSurface) {
		context->writeSurface->vtbl->release(context->writeSurface);
	} else {
		if (writeSurface) {
			/* Initialize the viewport if we do not have a surface bound */
			VPMT_ExecViewport(context, 0, 0, writeSurface->image.size.width,
							  writeSurface->image.size.height);
		}
	}

	context->readSurface = readSurface;
	context->writeSurface = writeSurface;

	return GL_TRUE;
}

GLAPI VPMT_Surface *APIENTRY VPMT_GetReadSurface(void)
{
	VPMT_Context *context = VPMT_CONTEXT();

	return context->readSurface;
}

GLAPI VPMT_Surface *APIENTRY VPMT_GetWriteSurface(void)
{
	VPMT_Context *context = VPMT_CONTEXT();

	return context->writeSurface;
}

static VPMT_INLINE void SetError(VPMT_Context * context, GLenum code)
{
	if (context->error == GL_NO_ERROR) {
		context->error = code;
	}
}

void VPMT_SetError(VPMT_Context * context, GLenum code)
{
	SetError(context, code);
}

void VPMT_SetOutOfMemoryError(VPMT_Context * context)
{
	SetError(context, GL_OUT_OF_MEMORY);
}

void VPMT_SetInvalidEnumError(VPMT_Context * context)
{
	SetError(context, GL_INVALID_ENUM);
}

void VPMT_SetInvalidModeError(VPMT_Context * context)
{
	SetError(context, GL_INVALID_MODE);
}

void VPMT_SetInvalidValueError(VPMT_Context * context)
{
	SetError(context, GL_INVALID_VALUE);
}

void VPMT_SetInvalidOperationError(VPMT_Context * context)
{
	SetError(context, GL_INVALID_OPERATION);
}

VPMT_Context VPMT_GlobalContext;

/* $Id: context.c 74 2008-11-23 07:25:12Z hmwill $ */
