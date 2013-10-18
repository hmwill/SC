/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** GL Function Dispatch Table
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_DISPATCH_H
#define VPMT_DISPATCH_H

typedef struct VPMT_Dispatch {
	void (*ActiveTexture) (VPMT_Context * context, GLenum texture);
	void (*AlphaFunc) (VPMT_Context * context, GLenum func, GLclampf ref);
	void (*Begin) (VPMT_Context * context, GLenum mode);
	void (*BindTexture) (VPMT_Context * context, GLenum target, GLuint texture);
	void (*Bitmap) (VPMT_Context * context, GLsizei width, GLsizei height, GLfloat xorig,
					GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap);
	void (*BlendFunc) (VPMT_Context * context, GLenum sfactor, GLenum dfactor);
	void (*CallLists) (VPMT_Context * context, GLsizei n, GLenum type, const GLvoid * lists);
	void (*Clear) (VPMT_Context * context, GLbitfield mask);
	void (*ClearColor) (VPMT_Context * context, GLclampf red, GLclampf green, GLclampf blue,
						GLclampf alpha);
	void (*ClearDepthf) (VPMT_Context * context, GLclampf depth);
	void (*ClearStencil) (VPMT_Context * context, GLint s);
	void (*ClientActiveTexture) (VPMT_Context * context, GLenum texture);
	void (*Color4f) (VPMT_Context * context, GLfloat red, GLfloat green, GLfloat blue,
					 GLfloat alpha);
	void (*Color4fv) (VPMT_Context * context, const GLfloat * v);
	void (*Color4ub) (VPMT_Context * context, GLubyte red, GLubyte green, GLubyte blue,
					  GLubyte alpha);
	void (*ColorMask) (VPMT_Context * context, GLboolean red, GLboolean green, GLboolean blue,
					   GLboolean alpha);
	void (*ColorPointer) (VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
						  const GLvoid * pointer);
	void (*CopyPixels) (VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height,
						GLenum type);
	void (*CullFace) (VPMT_Context * context, GLenum mode);
	void (*DeleteLists) (VPMT_Context * context, GLuint list, GLsizei range);
	void (*DeleteTextures) (VPMT_Context * context, GLsizei n, const GLuint *textures);
	void (*DepthFunc) (VPMT_Context * context, GLenum func);
	void (*DepthMask) (VPMT_Context * context, GLboolean flag);
	void (*DepthRangef) (VPMT_Context * context, GLclampf zNear, GLclampf zFar);
	void (*Disable) (VPMT_Context * context, GLenum cap);
	void (*DisableClientState) (VPMT_Context * context, GLenum array);
	void (*DrawArrays) (VPMT_Context * context, GLenum mode, GLint first, GLsizei count);
	void (*DrawElements) (VPMT_Context * context, GLenum mode, GLsizei count, GLenum type,
						  const GLvoid * indices);
	void (*DrawPixels) (VPMT_Context * context, GLsizei width, GLsizei height, GLenum format,
						GLenum type, const GLvoid * pixels);
	void (*Enable) (VPMT_Context * context, GLenum cap);
	void (*EnableClientState) (VPMT_Context * context, GLenum array);
	void (*End) (VPMT_Context * context);
	void (*EndList) (VPMT_Context * context);
	void (*Finish) (VPMT_Context * context);
	void (*Flush) (VPMT_Context * context);
	void (*FrontFace) (VPMT_Context * context, GLenum mode);
	void (*Frustumf) (VPMT_Context * context, GLfloat left, GLfloat right, GLfloat bottom,
					  GLfloat top, GLfloat zNear, GLfloat zFar);
	 GLuint(*GenLists) (VPMT_Context * context, GLsizei range);
	void (*GenTextures) (VPMT_Context * context, GLsizei n, GLuint * textures);
	void (*GetBooleanv) (VPMT_Context * context, GLenum pname, GLboolean * params);
	 GLenum(*GetError) (VPMT_Context * context);
	void (*GetFloatv) (VPMT_Context * context, GLenum pname, GLfloat * params);
	void (*GetIntegerv) (VPMT_Context * context, GLenum pname, GLint * params);
	void (*GetLightfv) (VPMT_Context * context, GLenum light, GLenum pname, GLfloat * params);
	void (*GetMaterialfv) (VPMT_Context * context, GLenum face, GLenum pname, GLfloat * params);
	void (*GetPointerv) (VPMT_Context * context, GLenum pname, GLvoid * *params);
	void (*GetPolygonStipple) (VPMT_Context * context, GLubyte * mask);
	const GLubyte *(*GetString) (VPMT_Context * context, GLenum name);
	void (*GetTexEnvfv) (VPMT_Context * context, GLenum target, GLenum pname, GLfloat * params);
	void (*GetTexEnviv) (VPMT_Context * context, GLenum target, GLenum pname, GLint * params);
	void (*GetTexParameteriv) (VPMT_Context * context, GLenum target, GLenum pname, GLint * params);
	void (*Hint) (VPMT_Context * context, GLenum target, GLenum mode);
	 GLboolean(*IsEnabled) (VPMT_Context * context, GLenum cap);
	void (*Lightfv) (VPMT_Context * context, GLenum light, GLenum pname, const GLfloat * params);
	void (*LightModelf) (VPMT_Context * context, GLenum pname, GLfloat param);
	void (*LightModelfv) (VPMT_Context * context, GLenum pname, const GLfloat * params);
	void (*LineStipple) (VPMT_Context * context, GLint factor, GLushort pattern);
	void (*LineWidth) (VPMT_Context * context, GLfloat width);
	void (*ListBase) (VPMT_Context * context, GLuint base);
	void (*LoadIdentity) (VPMT_Context * context);
	void (*LoadMatrixf) (VPMT_Context * context, const GLfloat * m);
	void (*Materialf) (VPMT_Context * context, GLenum face, GLenum pname, GLfloat param);
	void (*Materialfv) (VPMT_Context * context, GLenum face, GLenum pname, const GLfloat * params);
	void (*MatrixMode) (VPMT_Context * context, GLenum mode);
	void (*MultMatrixf) (VPMT_Context * context, const GLfloat * m);
	void (*MultiTexCoord2f) (VPMT_Context * context, GLenum target, GLfloat s, GLfloat t);
	void (*NewList) (VPMT_Context * context, GLuint list, GLenum mode);
	void (*Normal3f) (VPMT_Context * context, GLfloat nx, GLfloat ny, GLfloat nz);
	void (*Normal3fv) (VPMT_Context * context, const GLfloat * v);
	void (*NormalPointer) (VPMT_Context * context, GLenum type, GLsizei stride,
						   const GLvoid * pointer);
	void (*Orthof) (VPMT_Context * context, GLfloat left, GLfloat right, GLfloat bottom,
					GLfloat top, GLfloat zNear, GLfloat zFar);
	void (*PixelStorei) (VPMT_Context * context, GLenum pname, GLint param);
	void (*PointSize) (VPMT_Context * context, GLfloat size);
	void (*PolygonOffset) (VPMT_Context * context, GLfloat factor, GLfloat units);
	void (*PolygonStipple) (VPMT_Context * context, const GLubyte * mask);
	void (*PopMatrix) (VPMT_Context * context);
	void (*PushMatrix) (VPMT_Context * context);
	void (*RasterPos3f) (VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z);
	void (*ReadPixels) (VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height,
						GLenum format, GLenum type, GLvoid * pixels);
	void (*Rotatef) (VPMT_Context * context, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void (*Scalef) (VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z);
	void (*Scissor) (VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height);
	void (*ShadeModel) (VPMT_Context * context, GLenum mode);
	void (*StencilFunc) (VPMT_Context * context, GLenum func, GLint ref, GLuint mask);
	void (*StencilMask) (VPMT_Context * context, GLuint mask);
	void (*StencilOp) (VPMT_Context * context, GLenum fail, GLenum zfail, GLenum zpass);
	void (*TexCoordPointer) (VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
							 const GLvoid * pointer);
	void (*TexEnvfv) (VPMT_Context * context, GLenum target, GLenum pname, const GLfloat * params);
	void (*TexEnvi) (VPMT_Context * context, GLenum target, GLenum pname, GLint param);
	void (*TexImage2D) (VPMT_Context * context, GLenum target, GLint level, GLint internalformat,
						GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
						const GLvoid * pixels);
	void (*TexParameteri) (VPMT_Context * context, GLenum target, GLenum pname, GLint param);
	void (*TexSubImage2D) (VPMT_Context * context, GLenum target, GLint level, GLint xoffset,
						   GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type,
						   const GLvoid * pixels);
	void (*Translatef) (VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z);
	void (*Vertex2f) (VPMT_Context * context, GLfloat x, GLfloat y);
	void (*Vertex2fv) (VPMT_Context * context, const GLfloat * v);
	void (*Vertex3f) (VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z);
	void (*Vertex3fv) (VPMT_Context * context, const GLfloat * v);
	void (*VertexPointer) (VPMT_Context * context, GLint size, GLenum type, GLsizei stride,
						   const GLvoid * pointer);
	void (*Viewport) (VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height);

#if GL_EXT_paletted_texture
	void (*ColorSubTable) (VPMT_Context * context, GLenum target, GLsizei start, GLsizei count,
						   GLenum format, GLenum type, const GLvoid * table);
	void (*ColorTable) (VPMT_Context * context, GLenum target, GLenum internalformat, GLsizei width,
						GLenum format, GLenum type, const GLvoid * table);
	void (*GetColorTable) (VPMT_Context * context, GLenum target, GLenum format, GLenum type,
						   GLvoid * table);
	void (*GetColorTableParameteriv) (VPMT_Context * context, GLenum target, GLenum pname,
									  GLint * params);
#endif
} VPMT_Dispatch;

extern struct VPMT_Dispatch VPMT_DispatchExecute, VPMT_DispatchRecord;

#endif

/* $Id: dispatch.h 74 2008-11-23 07:25:12Z hmwill $ */
