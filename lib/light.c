/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Light and material functions.
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
** Exported API entry points
** -------------------------------------------------------------------------
*/

void VPMT_ExecGetLightfv(VPMT_Context * context, GLenum light, GLenum pname, GLfloat * params)
{
	GLint index;

	VPMT_NOT_RENDERING(context);

	if (light < GL_LIGHT0 || light > GL_LIGHT1) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	index = light - GL_LIGHT0;

	switch (pname) {
	case GL_AMBIENT:
		VPMT_Vec4Copy(params, context->lightAmbient[index]);
		break;

	case GL_DIFFUSE:
		VPMT_Vec4Copy(params, context->lightDiffuse[index]);
		break;

	case GL_SPECULAR:
		VPMT_Vec4Copy(params, context->lightSpecular[index]);
		break;

	case GL_POSITION:
		VPMT_Vec4Copy(params, context->lightPosition[index]);
		break;

	default:
		VPMT_INVALID_VALUE(context);
		return;
	}
}

void VPMT_ExecGetMaterialfv(VPMT_Context * context, GLenum face, GLenum pname, GLfloat * params)
{
	VPMT_NOT_RENDERING(context);

	if (face != GL_FRONT_AND_BACK) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (pname) {
	case GL_AMBIENT:
		VPMT_Vec4Copy(params, context->materialAmbient);
		break;

	case GL_DIFFUSE:
		VPMT_Vec4Copy(params, context->materialDiffuse);
		break;

	case GL_SPECULAR:
		VPMT_Vec4Copy(params, context->materialSpecular);
		break;

	case GL_EMISSION:
		VPMT_Vec4Copy(params, context->materialEmission);
		break;

	case GL_SHININESS:
		*params = context->materialShininess;
		break;

	default:
		VPMT_INVALID_VALUE(context);
		break;
	}
}

void VPMT_ExecLightfv(VPMT_Context * context, GLenum light, GLenum pname, const GLfloat * params)
{
	GLint index;
	GLfloat factor;
	VPMT_Vec4 transformed;

	VPMT_NOT_RENDERING(context);

	if (light < GL_LIGHT0 || light > GL_LIGHT1) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	index = light - GL_LIGHT0;

	switch (pname) {
	case GL_AMBIENT:
		VPMT_Vec4Copy(context->lightAmbient[index], params);
		break;

	case GL_DIFFUSE:
		VPMT_Vec4Copy(context->lightDiffuse[index], params);
		break;

	case GL_SPECULAR:
		VPMT_Vec4Copy(context->lightSpecular[index], params);
		break;

	case GL_POSITION:
		if (params[3] != 0.0f) {
			/* only directional light is supported */
			VPMT_INVALID_VALUE(context);
			return;
		}

		VPMT_MatrixTransform4x3(context->modelviewMatrixStack.
								base[context->modelviewMatrixStack.current - 1],
								transformed, params);

		VPMT_Vec4Copy(context->lightPosition[index], transformed);

		/* we only have directional light, so let's precalculate the normalized light direction */
		factor = 1.0f / sqrtf(VPMT_Vec3Dot(transformed, transformed));

		VPMT_Vec3Scale(transformed, transformed, factor);
		transformed[3] = 0.0f;
		VPMT_Vec3Copy(context->lightDirection[index], transformed);

		/* precalculate the normalized highlight direction */
		transformed[2] += 1.0f;
		factor = 1.0f / sqrtf(VPMT_Vec3Dot(transformed, transformed));
		VPMT_Vec3Scale(context->lightHighlightDirection[index], transformed, factor);
		context->lightHighlightDirection[index][3] = 0.0f;

		break;

	default:
		VPMT_INVALID_VALUE(context);
		return;
	}
}

void VPMT_ExecLightModelf(VPMT_Context * context, GLenum pname, GLfloat param)
{
	VPMT_INVALID_ENUM(context);

	/* is this actually a mistake in the specification? */
}

void VPMT_ExecLightModelfv(VPMT_Context * context, GLenum pname, const GLfloat * params)
{
	VPMT_NOT_RENDERING(context);

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (pname) {
	case GL_LIGHT_MODEL_AMBIENT:
		VPMT_Vec4Copy(context->lightModelAmbient, params);
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_ExecMaterialf(VPMT_Context * context, GLenum face, GLenum pname, GLfloat param)
{
	if (face != GL_FRONT_AND_BACK) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	switch (pname) {
	case GL_SHININESS:
		if (param < 0.0f || param > 128.0f) {
			VPMT_INVALID_VALUE(context);
		} else {
			context->materialShininess = param;
		}

		break;

	default:
		VPMT_INVALID_VALUE(context);
		break;
	}
}

void VPMT_ExecMaterialfv(VPMT_Context * context, GLenum face, GLenum pname, const GLfloat * params)
{
	if (face != GL_FRONT_AND_BACK) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!params) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (pname) {
	case GL_AMBIENT:
		VPMT_Vec4Copy(context->materialAmbient, params);
		break;

	case GL_DIFFUSE:
		VPMT_Vec4Copy(context->materialDiffuse, params);
		break;

	case GL_AMBIENT_AND_DIFFUSE:
		VPMT_Vec4Copy(context->materialAmbient, params);
		VPMT_Vec4Copy(context->materialDiffuse, params);
		break;

	case GL_SPECULAR:
		VPMT_Vec4Copy(context->materialSpecular, params);
		break;

	case GL_EMISSION:
		VPMT_Vec4Copy(context->materialEmission, params);
		break;

	case GL_SHININESS:
		if (*params < 0.0f || *params > 128.0f) {
			VPMT_INVALID_VALUE(context);
		} else {
			context->materialShininess = *params;
		}

		break;

	default:
		VPMT_INVALID_VALUE(context);
		break;
	}
}

void VPMT_ExecShadeModel(VPMT_Context * context, GLenum mode)
{
	VPMT_NOT_RENDERING(context);

	switch (mode) {
	case GL_FLAT:
	case GL_SMOOTH:
		context->shadeModel = mode;
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_LightVertex(VPMT_Context * context, GLfloat * resultColor,
					  const GLfloat * eyeCoords, const GLfloat * eyeNormal,
					  const GLfloat * vertexColor)
{
	/* light calculation */
	GLsizei index;

	const GLfloat *ambientMaterial;
	const GLfloat *diffuseMaterial;

	if (context->colorMaterialEnabled) {
		ambientMaterial = vertexColor;
		diffuseMaterial = vertexColor;
	} else {
		ambientMaterial = context->materialAmbient;
		diffuseMaterial = context->materialDiffuse;
	}

	/* alpha is always determined by alpha of diffuse material color */
	resultColor[3] = diffuseMaterial[3];

	VPMT_Vec3Copy(resultColor, context->materialEmission);
	VPMT_Vec3MulAdd(resultColor, ambientMaterial, context->lightModelAmbient, resultColor);

	for (index = 0; index < VPMT_MAX_LIGHTS; ++index) {
		if (context->lightEnabled[index]) {
			GLfloat dotProduct = VPMT_Vec3Dot(context->lightDirection[index], eyeNormal);

			VPMT_Vec3MulAdd(resultColor, ambientMaterial, context->lightAmbient[index],
							resultColor);

			if (dotProduct > 0.0f) {
				VPMT_Vec3 scaledMaterial;

				VPMT_Vec3Scale(scaledMaterial, diffuseMaterial, dotProduct);
				VPMT_Vec3MulAdd(resultColor, context->lightDiffuse[index], scaledMaterial,
								resultColor);

				dotProduct = VPMT_Vec3Dot(context->lightHighlightDirection[index], eyeNormal);

				if (dotProduct > 0.0f) {
					/* add the specular component */
					GLfloat factor = VPMT_POWF(dotProduct, context->materialShininess);
					VPMT_Vec3Scale(scaledMaterial, context->materialSpecular, factor);
					VPMT_Vec3MulAdd(resultColor, context->lightSpecular[index], scaledMaterial,
									resultColor);
				}
			}
		}
	}

	/* clamp result to [0, 1] */
	VPMT_Vec4Clamp(resultColor, resultColor);
}

/* $Id: light.c 74 2008-11-23 07:25:12Z hmwill $ */
