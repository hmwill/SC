/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** GL API entry points.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "dispatch.h"
#include "exec.h"

/*
** -------------------------------------------------------------------------
** OpenGL SC 1.0 Core
** -------------------------------------------------------------------------
*/
GLAPI void APIENTRY glActiveTexture(GLenum texture)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ActiveTexture(context, texture);
}

GLAPI void APIENTRY glAlphaFunc(GLenum func, GLclampf ref)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->AlphaFunc(context, func, ref);
}

GLAPI void APIENTRY glBegin(GLenum mode)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Begin(context, mode);
}

GLAPI void APIENTRY glBindTexture(GLenum target, GLuint texture)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->BindTexture(context, target, texture);
}

GLAPI void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
							 GLfloat xmove, GLfloat ymove, const GLubyte * bitmap)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Bitmap(context, width, height, xorig, yorig, xmove, ymove, bitmap);
}

GLAPI void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->BlendFunc(context, sfactor, dfactor);
}

GLAPI void APIENTRY glCallLists(GLsizei n, GLenum type, const GLvoid * lists)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->CallLists(context, n, type, lists);
}

GLAPI void APIENTRY glClear(GLbitfield mask)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Clear(context, mask);
}

GLAPI void APIENTRY glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ClearColor(context, red, green, blue, alpha);
}

GLAPI void APIENTRY glClearDepthf(GLclampf depth)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ClearDepthf(context, depth);
}

GLAPI void APIENTRY glClearStencil(GLint s)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ClearStencil(context, s);
}

GLAPI void APIENTRY glClientActiveTexture(GLenum texture)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ClientActiveTexture(context, texture);
}

GLAPI void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Color4f(context, red, green, blue, alpha);
}

GLAPI void APIENTRY glColor4fv(const GLfloat * v)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Color4fv(context, v);
}

GLAPI void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Color4ub(context, red, green, blue, alpha);
}

GLAPI void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ColorMask(context, red, green, blue, alpha);
}

GLAPI void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ColorPointer(context, size, type, stride, pointer);
}

GLAPI void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->CopyPixels(context, x, y, width, height, type);
}

GLAPI void APIENTRY glCullFace(GLenum mode)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->CullFace(context, mode);
}

GLAPI void APIENTRY glDeleteLists (GLuint list, GLsizei range)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DeleteLists(context, list, range);
}

GLAPI void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DeleteTextures(context, n, textures);
}

GLAPI void APIENTRY glDepthFunc(GLenum func)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DepthFunc(context, func);
}

GLAPI void APIENTRY glDepthMask(GLboolean flag)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DepthMask(context, flag);
}

GLAPI void APIENTRY glDepthRangef(GLclampf zNear, GLclampf zFar)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DepthRangef(context, zNear, zFar);
}

GLAPI void APIENTRY glDisable(GLenum cap)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Disable(context, cap);
}

GLAPI void APIENTRY glDisableClientState(GLenum array)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DisableClientState(context, array);
}

GLAPI void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DrawArrays(context, mode, first, count);
}

GLAPI void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DrawElements(context, mode, count, type, indices);
}

GLAPI void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type,
								 const GLvoid * pixels)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->DrawPixels(context, width, height, format, type, pixels);
}

GLAPI void APIENTRY glEnable(GLenum cap)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Enable(context, cap);
}

GLAPI void APIENTRY glEnableClientState(GLenum array)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->EnableClientState(context, array);
}

GLAPI void APIENTRY glEnd(void)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->End(context);
}

GLAPI void APIENTRY glEndList(void)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->EndList(context);
}

GLAPI void APIENTRY glFinish(void)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Finish(context);
}

GLAPI void APIENTRY glFlush(void)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Flush(context);
}

GLAPI void APIENTRY glFrontFace(GLenum mode)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->FrontFace(context, mode);
}

GLAPI void APIENTRY glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
							   GLfloat zNear, GLfloat zFar)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Frustumf(context, left, right, bottom, top, zNear, zFar);
}

GLAPI GLuint APIENTRY glGenLists(GLsizei range)
{
	VPMT_Context *context = VPMT_CONTEXT();
	return context->dispatch->GenLists(context, range);
}

GLAPI void APIENTRY glGenTextures(GLsizei n, GLuint * textures)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GenTextures(context, n, textures);
}

GLAPI GLenum APIENTRY glGetError(void)
{
	VPMT_Context *context = VPMT_CONTEXT();
	return context->dispatch->GetError(context);
}

GLAPI void APIENTRY glGetBooleanv(GLenum pname, GLboolean * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetBooleanv(context, pname, params);
}

GLAPI void APIENTRY glGetFloatv(GLenum pname, GLfloat * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetFloatv(context, pname, params);
}

GLAPI void APIENTRY glGetIntegerv(GLenum pname, GLint * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetIntegerv(context, pname, params);
}

GLAPI void APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetLightfv(context, light, pname, params);
}

GLAPI void APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetMaterialfv(context, face, pname, params);
}

GLAPI void APIENTRY glGetPointerv(GLenum pname, GLvoid * *params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetPointerv(context, pname, params);
}

GLAPI void APIENTRY glGetPolygonStipple(GLubyte * mask)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetPolygonStipple(context, mask);
}

GLAPI void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetTexEnvfv(context, target, pname, params);
}

GLAPI void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetTexEnviv(context, target, pname, params);
}

GLAPI void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetTexParameteriv(context, target, pname, params);
}

GLAPI const GLubyte *APIENTRY glGetString(GLenum name)
{
	VPMT_Context *context = VPMT_CONTEXT();
	return context->dispatch->GetString(context, name);
}

GLAPI void APIENTRY glHint(GLenum target, GLenum mode)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Hint(context, target, mode);
}

GLAPI GLboolean APIENTRY glIsEnabled(GLenum cap)
{
	VPMT_Context *context = VPMT_CONTEXT();
	return context->dispatch->IsEnabled(context, cap);
}

GLAPI void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Lightfv(context, light, pname, params);
}

GLAPI void APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->LightModelf(context, pname, param);
}

GLAPI void APIENTRY glLightModelfv(GLenum pname, const GLfloat * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->LightModelfv(context, pname, params);
}

GLAPI void APIENTRY glLineStipple(GLint factor, GLushort pattern)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->LineStipple(context, factor, pattern);
}

GLAPI void APIENTRY glLineWidth(GLfloat width)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->LineWidth(context, width);
}

GLAPI void APIENTRY glListBase(GLuint base)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ListBase(context, base);
}

GLAPI void APIENTRY glLoadIdentity(void)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->LoadIdentity(context);
}

GLAPI void APIENTRY glLoadMatrixf(const GLfloat * m)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->LoadMatrixf(context, m);
}

GLAPI void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Materialf(context, face, pname, param);
}

GLAPI void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Materialfv(context, face, pname, params);
}

GLAPI void APIENTRY glMatrixMode(GLenum mode)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->MatrixMode(context, mode);
}

GLAPI void APIENTRY glMultMatrixf(const GLfloat * m)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->MultMatrixf(context, m);
}

GLAPI void APIENTRY glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->MultiTexCoord2f(context, target, s, t);
}

GLAPI void APIENTRY glNewList(GLuint list, GLenum mode)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->NewList(context, list, mode);
}

GLAPI void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Normal3f(context, nx, ny, nz);
}

GLAPI void APIENTRY glNormal3fv(const GLfloat * v)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Normal3fv(context, v);
}

GLAPI void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid * pointer)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->NormalPointer(context, type, stride, pointer);
}

GLAPI void APIENTRY glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
							 GLfloat zNear, GLfloat zFar)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Orthof(context, left, right, bottom, top, zNear, zFar);
}

GLAPI void APIENTRY glPixelStorei(GLenum pname, GLint param)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->PixelStorei(context, pname, param);
}

GLAPI void APIENTRY glPointSize(GLfloat size)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->PointSize(context, size);
}

GLAPI void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->PolygonOffset(context, factor, units);
}

GLAPI void APIENTRY glPolygonStipple(const GLubyte * mask)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->PolygonStipple(context, mask);
}

GLAPI void APIENTRY glPopMatrix(void)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->PopMatrix(context);
}

GLAPI void APIENTRY glPushMatrix(void)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->PushMatrix(context);
}

GLAPI void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->RasterPos3f(context, x, y, z);
}

GLAPI void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
								 GLenum type, GLvoid * pixels)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ReadPixels(context, x, y, width, height, format, type, pixels);
}

GLAPI void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Rotatef(context, angle, x, y, z);
}

GLAPI void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Scalef(context, x, y, z);
}

GLAPI void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Scissor(context, x, y, width, height);
}

GLAPI void APIENTRY glShadeModel(GLenum mode)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ShadeModel(context, mode);
}

GLAPI void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->StencilFunc(context, func, ref, mask);
}

GLAPI void APIENTRY glStencilMask(GLuint mask)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->StencilMask(context, mask);
}

GLAPI void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->StencilOp(context, fail, zfail, zpass);
}

GLAPI void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride,
									  const GLvoid * pointer)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->TexCoordPointer(context, size, type, stride, pointer);
}

GLAPI void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->TexEnvfv(context, target, pname, params);
}

GLAPI void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->TexEnvi(context, target, pname, param);
}

GLAPI void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width,
								 GLsizei height, GLint border, GLenum format, GLenum type,
								 const GLvoid * pixels)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->TexImage2D(context, target, level, internalformat, width, height, border,
								  format, type, pixels);
}

GLAPI void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->TexParameteri(context, target, pname, param);
}

GLAPI void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
									GLsizei width, GLsizei height, GLenum format, GLenum type,
									const GLvoid * pixels)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->TexSubImage2D(context, target, level, xoffset, yoffset, width, height,
									 format, type, pixels);
}

GLAPI void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Translatef(context, x, y, z);
}

GLAPI void APIENTRY glVertex2f(GLfloat x, GLfloat y)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Vertex2f(context, x, y);
}

GLAPI void APIENTRY glVertex2fv(const GLfloat * v)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Vertex2fv(context, v);
}

GLAPI void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Vertex3f(context, x, y, z);
}

GLAPI void APIENTRY glVertex3fv(const GLfloat * v)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Vertex3fv(context, v);
}

GLAPI void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->VertexPointer(context, size, type, stride, pointer);
}

GLAPI void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->Viewport(context, x, y, width, height);
}

/*
** -------------------------------------------------------------------------
** GL_EXT_paletted_texture
** -------------------------------------------------------------------------
*/
#if GL_EXT_paletted_texture

GLAPI void APIENTRY glColorSubTableEXT(GLenum target, GLsizei start, GLsizei count, GLenum format,
									   GLenum type, const GLvoid * table)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ColorSubTable(context, target, start, count, format, type, table);
}

GLAPI void APIENTRY glColorTableEXT(GLenum target, GLenum internalformat, GLsizei width,
									GLenum format, GLenum type, const GLvoid * table)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->ColorTable(context, target, internalformat, width, format, type, table);
}

GLAPI void APIENTRY glGetColorTableEXT(GLenum target, GLenum format, GLenum type, GLvoid * table)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetColorTable(context, target, format, type, table);
}

GLAPI void APIENTRY glGetColorTableParameterivEXT(GLenum target, GLenum pname, GLint * params)
{
	VPMT_Context *context = VPMT_CONTEXT();
	context->dispatch->GetColorTableParameteriv(context, target, pname, params);
}

#endif

/*
** -------------------------------------------------------------------------
** Dispatch table
** -------------------------------------------------------------------------
*/
VPMT_Dispatch VPMT_DispatchExecute = {
	&VPMT_ExecActiveTexture,
	&VPMT_ExecAlphaFunc,
	&VPMT_ExecBegin,
	&VPMT_ExecBindTexture,
	&VPMT_ExecBitmap,
	&VPMT_ExecBlendFunc,
	&VPMT_ExecCallLists,
	&VPMT_ExecClear,
	&VPMT_ExecClearColor,
	&VPMT_ExecClearDepthf,
	&VPMT_ExecClearStencil,
	&VPMT_ExecClientActiveTexture,
	&VPMT_ExecColor4f,
	&VPMT_ExecColor4fv,
	&VPMT_ExecColor4ub,
	&VPMT_ExecColorMask,
	&VPMT_ExecColorPointer,
	&VPMT_ExecCopyPixels,
	&VPMT_ExecCullFace,
	&VPMT_ExecDeleteLists,
	&VPMT_ExecDeleteTextures,
	&VPMT_ExecDepthFunc,
	&VPMT_ExecDepthMask,
	&VPMT_ExecDepthRangef,
	&VPMT_ExecDisable,
	&VPMT_ExecDisableClientState,
	&VPMT_ExecDrawArrays,
	&VPMT_ExecDrawElements,
	&VPMT_ExecDrawPixels,
	&VPMT_ExecEnable,
	&VPMT_ExecEnableClientState,
	&VPMT_ExecEnd,
	&VPMT_ExecEndList,
	&VPMT_ExecFinish,
	&VPMT_ExecFlush,
	&VPMT_ExecFrontFace,
	&VPMT_ExecFrustumf,
	&VPMT_ExecGenLists,
	&VPMT_ExecGenTextures,
	&VPMT_ExecGetBooleanv,
	&VPMT_ExecGetError,
	&VPMT_ExecGetFloatv,
	&VPMT_ExecGetIntegerv,
	&VPMT_ExecGetLightfv,
	&VPMT_ExecGetMaterialfv,
	&VPMT_ExecGetPointerv,
	&VPMT_ExecGetPolygonStipple,
	&VPMT_ExecGetString,
	&VPMT_ExecGetTexEnvfv,
	&VPMT_ExecGetTexEnviv,
	&VPMT_ExecGetTexParameteriv,
	&VPMT_ExecHint,
	&VPMT_ExecIsEnabled,
	&VPMT_ExecLightfv,
	&VPMT_ExecLightModelf,
	&VPMT_ExecLightModelfv,
	&VPMT_ExecLineStipple,
	&VPMT_ExecLineWidth,
	&VPMT_ExecListBase,
	&VPMT_ExecLoadIdentity,
	&VPMT_ExecLoadMatrixf,
	&VPMT_ExecMaterialf,
	&VPMT_ExecMaterialfv,
	&VPMT_ExecMatrixMode,
	&VPMT_ExecMultMatrixf,
	&VPMT_ExecMultiTexCoord2f,
	&VPMT_ExecNewList,
	&VPMT_ExecNormal3f,
	&VPMT_ExecNormal3fv,
	&VPMT_ExecNormalPointer,
	&VPMT_ExecOrthof,
	&VPMT_ExecPixelStorei,
	&VPMT_ExecPointSize,
	&VPMT_ExecPolygonOffset,
	&VPMT_ExecPolygonStipple,
	&VPMT_ExecPopMatrix,
	&VPMT_ExecPushMatrix,
	&VPMT_ExecRasterPos3f,
	&VPMT_ExecReadPixels,
	&VPMT_ExecRotatef,
	&VPMT_ExecScalef,
	&VPMT_ExecScissor,
	&VPMT_ExecShadeModel,
	&VPMT_ExecStencilFunc,
	&VPMT_ExecStencilMask,
	&VPMT_ExecStencilOp,
	&VPMT_ExecTexCoordPointer,
	&VPMT_ExecTexEnvfv,
	&VPMT_ExecTexEnvi,
	&VPMT_ExecTexImage2D,
	&VPMT_ExecTexParameteri,
	&VPMT_ExecTexSubImage2D,
	&VPMT_ExecTranslatef,
	&VPMT_ExecVertex2f,
	&VPMT_ExecVertex2fv,
	&VPMT_ExecVertex3f,
	&VPMT_ExecVertex3fv,
	&VPMT_ExecVertexPointer,
	&VPMT_ExecViewport,

#if GL_EXT_paletted_texture
	&VPMT_ExecColorSubTable,
	&VPMT_ExecColorTable,
	&VPMT_ExecGetColorTable,
	&VPMT_ExecGetColorTableParameteriv,
#endif
};

/* $Id: gl.c 74 2008-11-23 07:25:12Z hmwill $ */
