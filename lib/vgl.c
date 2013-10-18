/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** SDL bindings as interim solution until proper EGL implementation in place
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
#include "GL/vgl.h"


GLAPI GLboolean APIENTRY vglInitialize(void)
{
	return VPMT_Initialize();
}

GLAPI GLboolean APIENTRY vglTerminate(void)
{
	VPMT_Terminate();
	return GL_TRUE;
}

GLAPI void (APIENTRY * vglGetProcAddress(const char *procname)) () {

	if (!procname) {
		return NULL;
	}
#if GL_EXT_paletted_texture
	if (!strcmp("glColorSubTableEXT", procname)) {
		return (void (APIENTRY *) ()) &glColorSubTableEXT;
	}

	if (!strcmp("glColorTableEXT", procname)) {
		return (void (APIENTRY *) ()) &glColorTableEXT;
	}

	if (!strcmp("glGetColorTableEXT", procname)) {
		return (void (APIENTRY *) ()) &glGetColorTableEXT;
	}

	if (!strcmp("glGetColorTableParameterivEXT", procname)) {
		return (void (APIENTRY *) ()) &glGetColorTableParameterivEXT;
	}
#endif

	return NULL;
}

GLAPI GLboolean APIENTRY vglDestroySurface(VGL_Surface surface)
{
	VPMT_Surface *wrapper = (VPMT_Surface *) surface;
	wrapper->vtbl->release(wrapper);

	return GL_TRUE;
}

GLAPI GLboolean APIENTRY vglMakeCurrent(VGL_Surface draw, VGL_Surface read)
{
	VPMT_Surface *drawWrapper = (VPMT_Surface *) draw;
	VPMT_Surface *readWrapper = (VPMT_Surface *) read;

	return VPMT_MakeCurrent(drawWrapper, readWrapper);
}

GLAPI VGL_Surface APIENTRY vglGetReadSurface(void)
{
	return (VGL_Surface) VPMT_GetReadSurface();
}

GLAPI VGL_Surface APIENTRY vglGetWriteSurface(void)
{
	return (VGL_Surface) VPMT_GetWriteSurface();
}

/* $Id: vgl.c 74 2008-11-23 07:25:12Z hmwill $ */
