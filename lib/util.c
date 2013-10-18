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

#include "common.h"
#include "GL/gl.h"
#include "util.h"

void VPMT_IntersectRect(VPMT_Rect * result, const VPMT_Rect * a, const VPMT_Rect * b)
{
	result->origin[0] = VPMT_MAX(a->origin[0], b->origin[0]);
	result->origin[1] = VPMT_MAX(a->origin[1], b->origin[1]);

	result->size.width = VPMT_MIN(a->size.width + a->origin[0], b->size.width + b->origin[0])
		- result->origin[0];

	result->size.height = VPMT_MIN(a->size.height + a->origin[1], b->size.height + b->origin[1])
		- result->origin[1];
}

/* $Id: util.c 74 2008-11-23 07:25:12Z hmwill $ */
