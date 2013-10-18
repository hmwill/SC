/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Rasterizer functions
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_RASTER_H
#define VPMT_RASTER_H

#include "context.h"

typedef enum {
	VPMT_RasterInterpolateDepth = 1,
	VPMT_RasterInterpolateInvW = (1 << 1),
	VPMT_RasterInterpolateColor = (1 << 2),
	VPMT_RasterInterpolateFog = (1 << 3),
	VPMT_RasterInterpolateTexCoord0 = (1 << 4),
	VPMT_RasterInterpolateTexCoord1 = (1 << 5),
	VPMT_RasterInterpolateTexCoord2 = (1 << 6),
	VPMT_RasterInterpolateTexCoord3 = (1 << 7),
	VPMT_RasterInterpolateRho0 = (1 << 8),
	VPMT_RasterInterpolateRho1 = (1 << 9),
	VPMT_RasterInterpolateRho2 = (1 << 10),
	VPMT_RasterInterpolateRho3 = (1 << 11)
} VPMT_RasterInterpolants;

void VPMT_RasterPrepareInterpolants(VPMT_Context * context);

void VPMT_RasterPreparePoint(VPMT_Context * context);
void VPMT_RasterPrepareLine(VPMT_Context * context);
void VPMT_RasterPrepareTriangle(VPMT_Context * context);
void VPMT_LineStippleReset(VPMT_Context * context);

void VPMT_RasterQuadAA(VPMT_Context * context, const VPMT_RasterVertex * a,
					   const VPMT_RasterVertex * b, const VPMT_RasterVertex * c,
					   const VPMT_RasterVertex * d);

#endif

/* $Id: raster.h 74 2008-11-23 07:25:12Z hmwill $ */
