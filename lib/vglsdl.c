/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** SDL Surface Bindings
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
#include <SDL.h>

typedef struct SdlSurfaceWrapper {
	VPMT_Surface surface;
	SDL_Surface *sdlSurface;
} SdlSurfaceWrapper;

static void AddrefSurface(struct VPMT_Surface *surface)
{
	SdlSurfaceWrapper *wrapper = (SdlSurfaceWrapper *) surface;

	++wrapper->surface.refcount;
}

static void ReleaseSurface(struct VPMT_Surface *surface)
{
	SdlSurfaceWrapper *wrapper = (SdlSurfaceWrapper *) surface;

	if (--wrapper->surface.refcount == 0) {
		if (wrapper->sdlSurface) {
			SDL_FreeSurface(wrapper->sdlSurface);
		}

		if (wrapper->surface.depthStencilBuffer) {
			VPMT_FREE(wrapper->surface.depthStencilBuffer);
		}

		VPMT_FREE(wrapper);
	}
}

static void LockSurface(VPMT_Context * context, struct VPMT_Surface *surface)
{
	SdlSurfaceWrapper *wrapper = (SdlSurfaceWrapper *) surface;
	VPMT_Rect surfaceRect;

	if (SDL_MUSTLOCK(wrapper->sdlSurface) && !wrapper->sdlSurface->locked) {
		SDL_LockSurface(wrapper->sdlSurface);
	}

	wrapper->surface.image.data =
		wrapper->sdlSurface->h <= 1 ?
		(GLubyte *) wrapper->sdlSurface->pixels :
		(GLubyte *) wrapper->sdlSurface->pixels +
		wrapper->sdlSurface->pitch * (wrapper->sdlSurface->h - 1);

	surfaceRect.origin[0] = wrapper->sdlSurface->clip_rect.x;
	surfaceRect.origin[1] = wrapper->sdlSurface->clip_rect.y;
	surfaceRect.size.width = wrapper->sdlSurface->clip_rect.w;
	surfaceRect.size.height = wrapper->sdlSurface->clip_rect.h;

	VPMT_UpdateActiveSurfaceRect(context, &surfaceRect);
}

static void UnlockSurface(VPMT_Context * context, struct VPMT_Surface *surface)
{
	SdlSurfaceWrapper *wrapper = (SdlSurfaceWrapper *) surface;

	if (SDL_MUSTLOCK(wrapper->sdlSurface) && wrapper->sdlSurface->locked) {
		SDL_UnlockSurface(wrapper->sdlSurface);
	}

	wrapper->surface.image.data = NULL;
}

static VPMT_SurfaceVtbl Vtbl = {
	&AddrefSurface,
	&ReleaseSurface,
	&LockSurface,
	&UnlockSurface
};

GLAPI VGL_Surface APIENTRY
vglCreateSurface(GLsizei width, GLsizei height, GLenum format, GLenum type, GLenum depthStencilType)
{
	SdlSurfaceWrapper *wrapper = NULL;

	const VPMT_PixelFormat *pixelFormat = VPMT_GetPixelFormat(format, type);

	const VPMT_DepthStencilFormat *depthStencilFormat =
		VPMT_GetDepthStencilFormat(depthStencilType);

	union {
		GLubyte rgba[4];
		GLuint mask;
	} translate;

	GLuint redMask, greenMask, blueMask, alphaMask;

	/* only allow 32-bit RGBA format at this point; can extend later */
	if (format != GL_RGBA ||
		(type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_INT_8_8_8_8
		 && type != GL_UNSIGNED_INT_8_8_8_8_REV) || !pixelFormat || !depthStencilFormat) {
		return NULL;
	}

	translate.mask = 0, translate.rgba[0] = 0xffu, redMask = translate.mask;
	translate.mask = 0, translate.rgba[1] = 0xffu, greenMask = translate.mask;
	translate.mask = 0, translate.rgba[2] = 0xffu, blueMask = translate.mask;
	translate.mask = 0, translate.rgba[3] = 0 /*xffu */ , alphaMask = translate.mask;

	wrapper = VPMT_MALLOC(sizeof(SdlSurfaceWrapper));

	if (wrapper) {
		GLubyte *baseAddr;

		wrapper->surface.vtbl = &Vtbl;
		wrapper->sdlSurface = NULL;
		wrapper->surface.depthStencilBuffer = NULL;

		wrapper->sdlSurface =
			SDL_CreateRGBSurface(0, width, height, pixelFormat->bits,
								 redMask, greenMask, blueMask, alphaMask);

		if (!wrapper->sdlSurface) {
			goto error;
		}

		if (height <= 1) {
			baseAddr = wrapper->sdlSurface->pixels;
		} else {
			baseAddr = (GLubyte *) wrapper->sdlSurface->pixels +
				(height - 1) * wrapper->sdlSurface->pitch;
		}

		VPMT_Image2DInit(&wrapper->surface.image, pixelFormat,
						 -(GLsizei) wrapper->sdlSurface->pitch, width, height, baseAddr);

		wrapper->surface.depthStencilFormat = depthStencilFormat;
		wrapper->surface.depthStencilBuffer =
			VPMT_MALLOC(height * width * (depthStencilFormat->bits / VPMT_BITS_PER_BYTE));

		if (!wrapper->surface.depthStencilBuffer) {
			goto error;
		}

		wrapper->surface.refcount = 1;

		return (VGL_Surface) & wrapper->surface;
	}

  error:
	if (wrapper) {
		if (wrapper->sdlSurface) {
			SDL_FreeSurface(wrapper->sdlSurface);
		}

		if (wrapper->surface.depthStencilBuffer) {
			VPMT_FREE(wrapper->surface.depthStencilBuffer);
		}

		VPMT_FREE(wrapper);
	}

	return NULL;
}

GLAPI GLboolean APIENTRY vglSwapBuffers(SDL_Surface * display, VGL_Surface surface)
{
	SdlSurfaceWrapper *wrapper = (SdlSurfaceWrapper *) surface;

	if (!SDL_BlitSurface(wrapper->sdlSurface, NULL, display, NULL) && !SDL_Flip(display)) {
		return GL_TRUE;
	} else {
		return GL_FALSE;
	}
}

/* $Id: vglsdl.c 74 2008-11-23 07:25:12Z hmwill $ */
