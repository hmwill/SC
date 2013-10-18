/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Context and surface creation and binding
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC.
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_VGL_H
#define VPMT_VGL_H

#include "gl.h"
#include <SDL.h>

typedef void *VGL_Surface;


GLAPI GLboolean APIENTRY vglInitialize(void);
GLAPI GLboolean APIENTRY vglTerminate(void);
GLAPI void (APIENTRY * vglGetProcAddress(const char *procname)) ();
GLAPI GLboolean APIENTRY vglDestroySurface(VGL_Surface surface);
GLAPI GLboolean APIENTRY vglMakeCurrent(VGL_Surface draw, VGL_Surface read);
GLAPI VGL_Surface APIENTRY vglGetReadSurface(void);
GLAPI VGL_Surface APIENTRY vglGetWriteSurface(void);

/* SDL bindings */
GLAPI VGL_Surface APIENTRY vglCreateSurface(GLsizei width, GLsizei height, GLenum format, GLenum type,
											GLenum depthStencilType);
GLAPI GLboolean APIENTRY vglSwapBuffers(SDL_Surface * display, VGL_Surface surface);



#endif