/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** GL Execution Functions
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_EXEC_H
#define VPMT_EXEC_H

void VPMT_ExecActiveTexture(VPMT_Context * context, GLenum texture);
void VPMT_ExecAlphaFunc(VPMT_Context * context, GLenum func, GLclampf ref);
void VPMT_ExecBegin(VPMT_Context * context, GLenum mode);
void VPMT_ExecBindTexture(VPMT_Context * context, GLenum target, GLuint name);
void VPMT_ExecBitmap(VPMT_Context * context, GLsizei width, GLsizei height, GLfloat xorig,
					 GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap);
void VPMT_ExecBlendFunc(VPMT_Context * context, GLenum sfactor, GLenum dfactor);
void VPMT_ExecCallLists(VPMT_Context * context, GLsizei n, GLenum type, const GLvoid * lists);
void VPMT_ExecClear(VPMT_Context * context, GLbitfield mask);
void VPMT_ExecClearColor(VPMT_Context * context, GLclampf red, GLclampf green, GLclampf blue,
						 GLclampf alpha);
void VPMT_ExecClearDepthf(VPMT_Context * context, GLclampf depth);
void VPMT_ExecClearStencil(VPMT_Context * context, GLint s);
void VPMT_ExecClientActiveTexture(VPMT_Context * context, GLenum texture);
void VPMT_ExecColor4f(VPMT_Context * context, GLfloat red, GLfloat green, GLfloat blue,
					  GLfloat alpha);
void VPMT_ExecColor4fv(VPMT_Context * context, const GLfloat * v);
void VPMT_ExecColor4ub(VPMT_Context * context, GLubyte red, GLubyte green, GLubyte blue,
					   GLubyte alpha);
void VPMT_ExecColorMask(VPMT_Context * context, GLboolean red, GLboolean green, GLboolean blue,
						GLboolean alpha);
void VPMT_ExecColorPointer(VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
						   const GLvoid * pointer);
void VPMT_ExecCopyPixels(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height,
						 GLenum type);
void VPMT_ExecCullFace(VPMT_Context * context, GLenum mode);
void VPMT_ExecDepthFunc(VPMT_Context * context, GLenum func);
void VPMT_ExecDepthMask(VPMT_Context * context, GLboolean flag);
void VPMT_ExecDepthRangef(VPMT_Context * context, GLclampf zNear, GLclampf zFar);
void VPMT_ExecDisable(VPMT_Context * context, GLenum cap);
void VPMT_ExecDisableClientState(VPMT_Context * context, GLenum array);
void VPMT_ExecDrawArrays(VPMT_Context * context, GLenum mode, GLint first, GLsizei count);
void VPMT_ExecDrawElements(VPMT_Context * context, GLenum mode, GLsizei count, GLenum type,
						   const GLvoid * indices);
void VPMT_ExecDrawPixels(VPMT_Context * context, GLsizei width, GLsizei height, GLenum format,
						 GLenum type, const GLvoid * pixels);
void VPMT_ExecEnable(VPMT_Context * context, GLenum cap);
void VPMT_ExecEnableClientState(VPMT_Context * context, GLenum array);
void VPMT_ExecEnd(VPMT_Context * context);
void VPMT_ExecEndList(VPMT_Context * context);
void VPMT_ExecFinish(VPMT_Context * context);
void VPMT_ExecFlush(VPMT_Context * context);
void VPMT_ExecFrontFace(VPMT_Context * context, GLenum mode);
void VPMT_ExecFrustumf(VPMT_Context * context, GLfloat left, GLfloat right, GLfloat bottom,
					   GLfloat top, GLfloat zNear, GLfloat zFar);
GLuint VPMT_ExecGenLists(VPMT_Context * context, GLsizei range);
void VPMT_ExecGenTextures(VPMT_Context * context, GLsizei n, GLuint * textures);
void VPMT_ExecGetBooleanv(VPMT_Context * context, GLenum pname, GLboolean * params);
GLenum VPMT_ExecGetError(VPMT_Context * context);
void VPMT_ExecGetFloatv(VPMT_Context * context, GLenum pname, GLfloat * params);
void VPMT_ExecGetIntegerv(VPMT_Context * context, GLenum pname, GLint * params);
void VPMT_ExecGetLightfv(VPMT_Context * context, GLenum light, GLenum pname, GLfloat * params);
void VPMT_ExecGetMaterialfv(VPMT_Context * context, GLenum face, GLenum pname, GLfloat * params);
void VPMT_ExecGetPointerv(VPMT_Context * context, GLenum pname, GLvoid ** params);
void VPMT_ExecGetPolygonStipple(VPMT_Context * context, GLubyte * mask);
const GLubyte *VPMT_ExecGetString(VPMT_Context * context, GLenum name);
void VPMT_ExecGetTexEnvfv(VPMT_Context * context, GLenum target, GLenum pname, GLfloat * params);
void VPMT_ExecGetTexEnviv(VPMT_Context * context, GLenum target, GLenum pname, GLint * params);
void VPMT_ExecGetTexParameteriv(VPMT_Context * context, GLenum target, GLenum pname,
								GLint * params);
void VPMT_ExecHint(VPMT_Context * context, GLenum target, GLenum mode);
GLboolean VPMT_ExecIsEnabled(VPMT_Context * context, GLenum cap);
void VPMT_ExecLightfv(VPMT_Context * context, GLenum light, GLenum pname, const GLfloat * params);
void VPMT_ExecLightModelf(VPMT_Context * context, GLenum pname, GLfloat param);
void VPMT_ExecLightModelfv(VPMT_Context * context, GLenum pname, const GLfloat * params);
void VPMT_ExecLineStipple(VPMT_Context * context, GLint factor, GLushort pattern);
void VPMT_ExecLineWidth(VPMT_Context * context, GLfloat width);
void VPMT_ExecListBase(VPMT_Context * context, GLuint base);
void VPMT_ExecLoadIdentity(VPMT_Context * context);
void VPMT_ExecLoadMatrixf(VPMT_Context * context, const GLfloat * m);
void VPMT_ExecMaterialf(VPMT_Context * context, GLenum face, GLenum pname, GLfloat param);
void VPMT_ExecMaterialfv(VPMT_Context * context, GLenum face, GLenum pname, const GLfloat * params);
void VPMT_ExecMatrixMode(VPMT_Context * context, GLenum mode);
void VPMT_ExecMultiTexCoord2f(VPMT_Context * context, GLenum target, GLfloat s, GLfloat t);
void VPMT_ExecMultMatrixf(VPMT_Context * context, const GLfloat * m);
void VPMT_ExecNewList(VPMT_Context * context, GLuint list, GLenum mode);
void VPMT_ExecNormal3f(VPMT_Context * context, GLfloat nx, GLfloat ny, GLfloat nz);
void VPMT_ExecNormal3fv(VPMT_Context * context, const GLfloat * v);
void VPMT_ExecNormalPointer(VPMT_Context * context, GLenum type, GLsizei stride,
							const GLvoid * pointer);
void VPMT_ExecOrthof(VPMT_Context * context, GLfloat left, GLfloat right, GLfloat bottom,
					 GLfloat top, GLfloat zNear, GLfloat zFar);
void VPMT_ExecPixelStorei(VPMT_Context * context, GLenum pname, GLint param);
void VPMT_ExecPointSize(VPMT_Context * context, GLfloat size);
void VPMT_ExecPolygonOffset(VPMT_Context * context, GLfloat factor, GLfloat units);
void VPMT_ExecPolygonStipple(VPMT_Context * context, const GLubyte * mask);
void VPMT_ExecPopMatrix(VPMT_Context * context);
void VPMT_ExecPushMatrix(VPMT_Context * context);
void VPMT_ExecRasterPos3f(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z);
void VPMT_ExecReadPixels(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height,
						 GLenum format, GLenum type, GLvoid * pixels);
void VPMT_ExecRotatef(VPMT_Context * context, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void VPMT_ExecScalef(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z);
void VPMT_ExecScissor(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height);
void VPMT_ExecShadeModel(VPMT_Context * context, GLenum mode);
void VPMT_ExecStencilFunc(VPMT_Context * context, GLenum func, GLint ref, GLuint mask);
void VPMT_ExecStencilMask(VPMT_Context * context, GLuint mask);
void VPMT_ExecStencilOp(VPMT_Context * context, GLenum fail, GLenum zfail, GLenum zpass);
void VPMT_ExecTexCoordPointer(VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
							  const GLvoid * pointer);
void VPMT_ExecTexEnvfv(VPMT_Context * context, GLenum target, GLenum pname, const GLfloat * params);
void VPMT_ExecTexEnvi(VPMT_Context * context, GLenum target, GLenum pname, GLint param);
void VPMT_ExecTexImage2D(VPMT_Context * context, GLenum target, GLint level, GLint internalformat,
						 GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
						 const GLvoid * pixels);
void VPMT_ExecTexParameteri(VPMT_Context * context, GLenum target, GLenum pname, GLint param);
void VPMT_ExecTexSubImage2D(VPMT_Context * context, GLenum target, GLint level, GLint xoffset,
							GLint yoffset, GLsizei width, GLsizei height, GLenum format,
							GLenum type, const GLvoid * pixels);
void VPMT_ExecTranslatef(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z);
void VPMT_ExecVertex2f(VPMT_Context * context, GLfloat x, GLfloat y);
void VPMT_ExecVertex2fv(VPMT_Context * context, const GLfloat * v);
void VPMT_ExecVertex3f(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z);
void VPMT_ExecVertex3fv(VPMT_Context * context, const GLfloat * v);
void VPMT_ExecVertexPointer(VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
							const GLvoid * pointer);
void VPMT_ExecViewport(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height);

/* execution only */
void VPMT_ExecDrawPixelsImage(VPMT_Context * context, const VPMT_Image2D * image);
void VPMT_ExecTexImage2DImage(VPMT_Context * context, GLenum target, GLint level,
							  GLint internalformat, const VPMT_Image2D * srcImage);
void VPMT_ExecTexSubImage2DImage(VPMT_Context * context, GLenum target, GLint level, GLint xoffset,
								 GLint yoffset, const VPMT_Image2D * srcImage);

#if GL_EXT_paletted_texture
void VPMT_ExecColorSubTable(VPMT_Context * context, GLenum target, GLsizei start, GLsizei count,
							GLenum format, GLenum type, const GLvoid * table);
void VPMT_ExecColorTable(VPMT_Context * context, GLenum target, GLenum internalformat,
						 GLsizei width, GLenum format, GLenum type, const GLvoid * table);
void VPMT_ExecGetColorTable(VPMT_Context * context, GLenum target, GLenum format, GLenum type,
							GLvoid * table);
void VPMT_ExecGetColorTableParameteriv(VPMT_Context * context, GLenum target, GLenum pname,
									   GLint * params);

/* execution only */
void VPMT_ExecColorSubTableImage(VPMT_Context * context, GLenum target, GLsizei start,
								 const VPMT_Image1D * srcImage);
void VPMT_ExecColorTableImage(VPMT_Context * context, GLenum target, const VPMT_Image1D * srcImage);

#endif

void VPMT_ExecDeleteLists (VPMT_Context * context, GLuint list, GLsizei range);
void VPMT_ExecDeleteTextures (VPMT_Context * context, GLsizei n, const GLuint *textures);

#endif

/* $Id: exec.h 74 2008-11-23 07:25:12Z hmwill $ */
