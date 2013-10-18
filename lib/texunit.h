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

#ifndef VPMT_TEXUNIT_H
#define VPMT_TEXUNIT_H

#include "tex.h"

typedef struct VPMT_TexImageUnit VPMT_TexImageUnit;

typedef VPMT_Color4us(*VPMT_Sampler2DFunc) (const VPMT_TexImageUnit * unit, const GLfloat * coords,
											GLfloat rho);

struct VPMT_TexImageUnit {
	/* external state */
	VPMT_Vec4 envColor;
	VPMT_Color4us envColor4us;
	VPMT_Texture2D *boundTexture;
	GLint boundTexture2D;									   /* shortcut for state queries */
	GLenum envMode;
	GLboolean enabled, incomplete;

	/* internal state */
	VPMT_Sampler2DFunc sampler2D;
	VPMT_Image2DSampleFunc imageMinSampler;
	VPMT_Image2DSampleFunc imageMagSampler;
	GLfloat magMinSwitchOver;
};

void VPMT_TexImageUnitsInit(VPMT_TexImageUnit units[VPMT_MAX_TEX_UNITS],
							VPMT_Texture2D * texture2d);

VPMT_Color4ub VPMT_TexImageUnitsExecute(VPMT_TexImageUnit units[VPMT_MAX_TEX_UNITS],
										VPMT_Color4us baseColor, const VPMT_Vec2 coords[],
										const GLfloat * rho);

GLboolean VPMT_TexImageUnitUsesBaseColor(const VPMT_TexImageUnit * unit);

void VPMT_TexImageUnitsPrepare(VPMT_TexImageUnit units[VPMT_MAX_TEX_UNITS]);

#endif

/* $Id: texunit.h 74 2008-11-23 07:25:12Z hmwill $ */
