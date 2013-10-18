/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Framebuffer Access
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_FRAME_H
#define VPMT_FRAME_H

#include "context.h"

typedef struct VPMT_FrameBuffer VPMT_FrameBuffer;

typedef VPMT_Color4ub(*VPMT_FrameReadColorFunc) (const VPMT_FrameBuffer * fb);
typedef void (*VPMT_FrameWriteColorFunc) (const VPMT_FrameBuffer * fb, VPMT_Color4ub color);
typedef GLuint(*VPMT_FrameReadDepthFunc) (const VPMT_FrameBuffer * fb);
typedef void (*VPMT_FrameWriteDepthFunc) (const VPMT_FrameBuffer * fb, GLuint depth);
typedef GLuint(*VPMT_FrameReadStencilFunc) (const VPMT_FrameBuffer * fb);
typedef void (*VPMT_FrameWriteStencilFunc) (const VPMT_FrameBuffer * fb, GLuint stencil);

struct VPMT_FrameBuffer {
	GLubyte *current[VPMT_MAX_RENDER_BUFFERS];				   /* current addresses of buffer pointers */
	GLsizei dx[VPMT_MAX_RENDER_BUFFERS];					   /* increment in positive X */
	GLsizei dy[VPMT_MAX_RENDER_BUFFERS];					   /* increment in positive Y */
	GLubyte *save[VPMT_MAX_RENDER_BUFFERS];					   /* address save area */

	VPMT_FrameReadColorFunc readColor;
	VPMT_FrameWriteColorFunc writeColor;
	VPMT_FrameReadDepthFunc readDepth;
	VPMT_FrameWriteDepthFunc writeDepth;
	VPMT_FrameReadStencilFunc readStencil;
	VPMT_FrameWriteStencilFunc writeStencil;
};

//void VPMT_FrameBufferClear(VPMT_FrameBuffer * fb, VPMT_Color4ub clearColor, GLuint clearDepth, GLuint clearStencil);
void VPMT_FrameBufferInit(VPMT_FrameBuffer * fb, VPMT_Surface * surface);

static VPMT_INLINE void VPMT_FrameBufferMove(VPMT_FrameBuffer * fb, GLint deltaX, GLint deltaY)
{
	GLsizei index;

	for (index = 0; index < VPMT_MAX_RENDER_BUFFERS; ++index) {
		fb->current[index] += fb->dx[index] * deltaX + fb->dy[index] * deltaY;
	}
}

static VPMT_INLINE void VPMT_FrameBufferStepX(VPMT_FrameBuffer * fb)
{
	GLsizei index;

	for (index = 0; index < VPMT_MAX_RENDER_BUFFERS; ++index) {
		fb->current[index] += fb->dx[index];
	}
}

static VPMT_INLINE void VPMT_FrameBufferStepNegX(VPMT_FrameBuffer * fb)
{
	GLsizei index;

	for (index = 0; index < VPMT_MAX_RENDER_BUFFERS; ++index) {
		fb->current[index] -= fb->dx[index];
	}
}

static VPMT_INLINE void VPMT_FrameBufferStepY(VPMT_FrameBuffer * fb)
{
	GLsizei index;

	for (index = 0; index < VPMT_MAX_RENDER_BUFFERS; ++index) {
		fb->current[index] += fb->dy[index];
	}
}

static VPMT_INLINE void VPMT_FrameBufferSave(VPMT_FrameBuffer * fb)
{
	GLsizei index;

	for (index = 0; index < VPMT_MAX_RENDER_BUFFERS; ++index) {
		fb->save[index] = fb->current[index];
	}
}

static VPMT_INLINE void VPMT_FrameBufferRestore(VPMT_FrameBuffer * fb)
{
	GLsizei index;

	for (index = 0; index < VPMT_MAX_RENDER_BUFFERS; ++index) {
		fb->current[index] = fb->save[index];
	}
}


static VPMT_INLINE VPMT_Color4ub VPMT_FrameReadColor(const VPMT_FrameBuffer * fb)
{
	return fb->readColor(fb);
}

static VPMT_INLINE void VPMT_FrameWriteColor(const VPMT_FrameBuffer * fb, VPMT_Color4ub color)
{
	fb->writeColor(fb, color);
}

static VPMT_INLINE GLuint VPMT_FrameReadDepth(const VPMT_FrameBuffer * fb)
{
	return fb->readDepth(fb);
}

static VPMT_INLINE void VPMT_FrameWriteDepth(const VPMT_FrameBuffer * fb, GLuint depth)
{
	fb->writeDepth(fb, depth);
}

static VPMT_INLINE GLuint VPMT_FrameReadStencil(const VPMT_FrameBuffer * fb)
{
	return fb->readStencil(fb);
}

static VPMT_INLINE void VPMT_FrameWriteStencil(const VPMT_FrameBuffer * fb, GLuint stencil)
{
	fb->writeStencil(fb, stencil);
}

#endif

/* $Id: frame.h 74 2008-11-23 07:25:12Z hmwill $ */
