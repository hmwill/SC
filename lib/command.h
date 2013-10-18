/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** GL command structures
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_COMMAND_H
#define VPMT_COMMAND_H

#include "context.h"

typedef enum VPMT_Opcode {
	VPMT_OpcodeActiveTexture,
	VPMT_OpcodeAlphaFunc,
	VPMT_OpcodeBegin,
	VPMT_OpcodeBindTexture,
	VPMT_OpcodeBitmap,
	VPMT_OpcodeBlendFunc,
	VPMT_OpcodeClear,
	VPMT_OpcodeClearColor,
	VPMT_OpcodeClearDepthf,
	VPMT_OpcodeClearStencil,
	VPMT_OpcodeColor,
	VPMT_OpcodeColorMask,
	VPMT_OpcodeCopyPixels,
	VPMT_OpcodeCullFace,
	VPMT_OpcodeDepthFunc,
	VPMT_OpcodeDepthMask,
	VPMT_OpcodeDepthRangef,
	VPMT_OpcodeDrawPixels,
	VPMT_OpcodeEnd,
	VPMT_OpcodeError,
	VPMT_OpcodeFrontFace,
	VPMT_OpcodeHint,
	VPMT_OpcodeLightfv,
	VPMT_OpcodeLightModelfv,
	VPMT_OpcodeLineStipple,
	VPMT_OpcodeLineWidth,
	VPMT_OpcodeListBase,
	VPMT_OpcodeLoadIdentity,
	VPMT_OpcodeLoadMatrix,
	VPMT_OpcodeMaterialfv,
	VPMT_OpcodeMatrixMode,
	VPMT_OpcodeMultMatrix,
	VPMT_OpcodeMultiTexCoord,
	VPMT_OpcodeNormal,
	VPMT_OpcodePointSize,
	VPMT_OpcodePolygonOffset,
	VPMT_OpcodePolygonStipple,
	VPMT_OpcodePopMatrix,
	VPMT_OpcodePushMatrix,
	VPMT_OpcodeRasterPos,
	VPMT_OpcodeScissor,
	VPMT_OpcodeShadeModel,
	VPMT_OpcodeStencilFunc,
	VPMT_OpcodeStencilMask,
	VPMT_OpcodeStencilOp,
	VPMT_OpcodeTexEnvfv,
	VPMT_OpcodeTexEnvi,
	VPMT_OpcodeTexImage2D,
	VPMT_OpcodeTexParameteri,
	VPMT_OpcodeTexSubImage2D,
	VPMT_OpcodeToggle,
	VPMT_OpcodeVertex,
	VPMT_OpcodeViewport,

#if GL_EXT_paletted_texture
	VPMT_OpcodeColorSubTable,
	VPMT_OpcodeColorTable,
#endif

	VPMT_OpcodeInvalid
} VPMT_Opcode;

typedef struct VPMT_CommandBase {
	VPMT_Opcode opcode;
} VPMT_CommandBase;

typedef struct VPMT_CommandActiveTexture {
	VPMT_CommandBase base;
	GLenum texture;
} VPMT_CommandActiveTexture;

typedef struct VPMT_CommandAlphaFunc {
	VPMT_CommandBase base;
	GLenum func;
	GLclampf ref;
} VPMT_CommandAlphaFunc;

typedef struct VPMT_CommandBegin {
	VPMT_CommandBase base;
	GLenum mode;
} VPMT_CommandBegin;

typedef struct VPMT_CommandBindTexture {
	VPMT_CommandBase base;
	GLenum target;
	GLuint texture;
} VPMT_CommandBindTexture;

typedef struct VPMT_CommandBitmap {
	VPMT_CommandBase base;
	GLsizei width;
	GLsizei height;
	GLfloat xorig;
	GLfloat yorig;
	GLfloat xmove;
	GLfloat ymove;
	GLubyte *bitmap;
} VPMT_CommandBitmap;

typedef struct VPMT_CommandBlendFunc {
	VPMT_CommandBase base;
	GLenum sfactor;
	GLenum dfactor;
} VPMT_CommandBlendFunc;

typedef struct VPMT_CommandClear {
	VPMT_CommandBase base;
	GLbitfield mask;
} VPMT_CommandClear;

typedef struct VPMT_CommandClearColor {
	VPMT_CommandBase base;
	VPMT_Vec4 color;
} VPMT_CommandClearColor;

typedef struct VPMT_CommandClearDepth {
	VPMT_CommandBase base;
	GLclampf depth;
} VPMT_CommandClearDepth;

typedef struct VPMT_CommandClearStencil {
	VPMT_CommandBase base;
	GLint s;
} VPMT_CommandClearStencil;

typedef struct VPMT_CommandColor {
	VPMT_CommandBase base;
	VPMT_Vec4 color;
} VPMT_CommandColor;

typedef struct VPMT_CommandColorMask {
	VPMT_CommandBase base;
	VPMT_Vec4b mask;
} VPMT_CommandColorMask;

typedef struct VPMT_CommandCopyPixels {
	VPMT_CommandBase base;
	VPMT_Rect rect;
	GLenum type;
} VPMT_CommandCopyPixels;

typedef struct VPMT_CommandCullFace {
	VPMT_CommandBase base;
	GLenum mode;
} VPMT_CommandCullFace;

typedef struct VPMT_CommandDepthFunc {
	VPMT_CommandBase base;
	GLenum func;
} VPMT_CommandDepthFunc;

typedef struct VPMT_CommandDepthMask {
	VPMT_CommandBase base;
	GLboolean flag;
} VPMT_CommandDepthMask;

typedef struct VPMT_CommandDepthRangef {
	VPMT_CommandBase base;
	GLclampf zNear;
	GLclampf zFar;
} VPMT_CommandDepthRangef;

typedef struct VPMT_CommandDrawPixels {
	VPMT_CommandBase base;
	VPMT_Image2D *image;
} VPMT_CommandDrawPixels;

typedef struct VPMT_CommandError {
	VPMT_CommandBase base;
	GLenum error;
} VPMT_CommandError;

typedef struct VPMT_CommandFrontFace {
	VPMT_CommandBase base;
	GLenum mode;
} VPMT_CommandFrontFace;

typedef struct VPMT_CommandHint {
	VPMT_CommandBase base;
	GLenum target;
	GLenum mode;
} VPMT_CommandHint;

typedef struct VPMT_CommandLightfv {
	VPMT_CommandBase base;
	GLenum light;
	GLenum pname;
	VPMT_Vec4 params;
} VPMT_CommandLightfv;

typedef struct VPMT_CommandLightModelfv {
	VPMT_CommandBase base;
	GLenum pname;
	VPMT_Vec4 params;
} VPMT_CommandLightModelfv;

typedef struct VPMT_CommandLineStipple {
	VPMT_CommandBase base;
	GLint factor;
	GLushort pattern;
} VPMT_CommandLineStipple;

typedef struct VPMT_CommandLineWidth {
	VPMT_CommandBase base;
	GLfloat width;
} VPMT_CommandLineWidth;

typedef struct VPMT_CommandListBase {
	VPMT_CommandBase base;
	GLuint listBase;
} VPMT_CommandListBase;

typedef struct VPMT_CommandLoadMatrix {
	VPMT_CommandBase base;
	VPMT_Matrix matrix;
} VPMT_CommandLoadMatrix;

typedef struct VPMT_CommandMaterialfv {
	VPMT_CommandBase base;
	GLenum face;
	GLenum pname;
	VPMT_Vec4 params;
} VPMT_CommandMaterialfv;

typedef struct VPMT_CommandMatrixMode {
	VPMT_CommandBase base;
	GLenum mode;
} VPMT_CommandMatrixMode;

typedef struct VPMT_CommandMultMatrix {
	VPMT_CommandBase base;
	VPMT_Matrix matrix;
} VPMT_CommandMultMatrix;

typedef struct VPMT_CommandMultiTexCoord {
	VPMT_CommandBase base;
	GLenum target;
	VPMT_Vec2 coords;
} VPMT_CommandMultiTexCoord;

typedef struct VPMT_CommandNormal {
	VPMT_CommandBase base;
	VPMT_Vec3 normal;
} VPMT_CommandNormal;

typedef struct VPMT_CommandPointSize {
	VPMT_CommandBase base;
	GLfloat size;
} VPMT_CommandPointSize;

typedef struct VPMT_CommandPolygonOffset {
	VPMT_CommandBase base;
	GLfloat factor;
	GLfloat units;
} VPMT_CommandPolygonOffset;

typedef struct VPMT_CommandPolygonStipple {
	VPMT_CommandBase base;
	VPMT_Pattern mask;
} VPMT_CommandPolygonStipple;

typedef struct VPMT_CommandRasterPos {
	VPMT_CommandBase base;
	VPMT_Vec3 position;
} VPMT_CommandRasterPos;

typedef struct VPMT_CommandScissor {
	VPMT_CommandBase base;
	VPMT_Rect rect;
} VPMT_CommandScissor;

typedef struct VPMT_CommandShadeModel {
	VPMT_CommandBase base;
	GLenum mode;
} VPMT_CommandShadeModel;

typedef struct VPMT_CommandStencilFunc {
	VPMT_CommandBase base;
	GLenum func;
	GLint ref;
	GLuint mask;
} VPMT_CommandStencilFunc;

typedef struct VPMT_CommandStencilMask {
	VPMT_CommandBase base;
	GLuint mask;
} VPMT_CommandStencilMask;

typedef struct VPMT_CommandStencilOp {
	VPMT_CommandBase base;
	GLenum fail;
	GLenum zfail;
	GLenum zpass;
} VPMT_CommandStencilOp;

typedef struct VPMT_CommandTexEnvfv {
	VPMT_CommandBase base;
	GLenum target;
	GLenum pname;
	VPMT_Vec4 params;
} VPMT_CommandTexEnvfv;

typedef struct VPMT_CommandTexEnvi {
	VPMT_CommandBase base;
	GLenum target;
	GLenum pname;
	GLint param;
} VPMT_CommandTexEnvi;

typedef struct VPMT_CommandTexImage2D {
	VPMT_CommandBase base;
	GLenum target;
	GLint level;
	VPMT_Image2D *image;
} VPMT_CommandTexImage2D;

typedef struct VPMT_CommandTexParameteri {
	VPMT_CommandBase base;
	GLenum target;
	GLenum pname;
	GLint param;
} VPMT_CommandTexParameteri;

typedef struct VPMT_CommandTexSubImage2D {
	VPMT_CommandBase base;
	GLenum target;
	GLint level;
	VPMT_Image2D *image;
	GLint xoffset;
	GLint yoffset;
} VPMT_CommandTexSubImage2D;

typedef struct VPMT_CommandToggle {
	VPMT_CommandBase base;
	GLenum cap;
	GLboolean enable;
} VPMT_CommandToggle;

typedef struct VPMT_CommandVertex {
	VPMT_CommandBase base;
	VPMT_Vec3 vertex;
} VPMT_CommandVertex;

typedef struct VPMT_CommandViewport {
	VPMT_CommandBase base;
	VPMT_Rect rect;
} VPMT_CommandViewport;

#if GL_EXT_paletted_texture

typedef struct VPMT_CommandColorSubTable {
	VPMT_CommandBase base;
	GLenum target;
	GLsizei start;
	VPMT_Image1D *palette;
} VPMT_CommandColorSubTable;

typedef struct VPMT_CommandColorTable {
	VPMT_CommandBase base;
	GLenum target;
	VPMT_Image1D *palette;
} VPMT_CommandColorTable;

#endif

typedef union VPMT_Command {
	VPMT_CommandBase base;
	VPMT_CommandActiveTexture activeTexture;
	VPMT_CommandAlphaFunc alphaFunc;
	VPMT_CommandBegin begin;
	VPMT_CommandBindTexture bindTexture;
	VPMT_CommandBitmap bitmap;
	VPMT_CommandBlendFunc blendFunc;
	VPMT_CommandClear clear;
	VPMT_CommandClearColor clearColor;
	VPMT_CommandClearDepth clearDepth;
	VPMT_CommandClearStencil clearStencil;
	VPMT_CommandColor color;
	VPMT_CommandColorMask colorMask;
	VPMT_CommandCopyPixels copyPixels;
	VPMT_CommandCullFace cullFace;
	VPMT_CommandDepthFunc depthFunc;
	VPMT_CommandDepthMask depthMask;
	VPMT_CommandDepthRangef depthRangef;
	VPMT_CommandDrawPixels drawPixels;
	VPMT_CommandError error;
	VPMT_CommandFrontFace frontFace;
	VPMT_CommandHint hint;
	VPMT_CommandLightfv lighfv;
	VPMT_CommandLightModelfv lightModelfv;
	VPMT_CommandLineStipple lineStipple;
	VPMT_CommandLineWidth lineWidth;
	VPMT_CommandListBase listBase;
	VPMT_CommandLoadMatrix loadMatrix;
	VPMT_CommandMaterialfv materialfv;
	VPMT_CommandMatrixMode matrixMode;
	VPMT_CommandMultMatrix multMatrix;
	VPMT_CommandMultiTexCoord multiTexCoord;
	VPMT_CommandNormal normal;
	VPMT_CommandPointSize pointSize;
	VPMT_CommandPolygonOffset polygonOffset;
	VPMT_CommandPolygonStipple polygonStipple;
	VPMT_CommandRasterPos rasterPos;
	VPMT_CommandScissor scissor;
	VPMT_CommandShadeModel shadeModel;
	VPMT_CommandStencilFunc stencilFunc;
	VPMT_CommandStencilMask stencilMask;
	VPMT_CommandStencilOp stencilOp;
	VPMT_CommandTexEnvfv texEnvfv;
	VPMT_CommandTexEnvi texEnvi;
	VPMT_CommandTexImage2D texImage2D;
	VPMT_CommandTexParameteri texParameteri;
	VPMT_CommandTexSubImage2D texSubImage2D;
	VPMT_CommandToggle toggle;
	VPMT_CommandVertex vertex;
	VPMT_CommandViewport viewport;

#if GL_EXT_paletted_texture
	VPMT_CommandColorSubTable colorSubTable;
	VPMT_CommandColorTable colorTable;
#endif

} VPMT_Command;

typedef struct VPMT_CommandBuffer {
	struct VPMT_CommandBuffer *next;						   /* next buffer belonging to list    */
	GLsizei used;											   /* amount of command storage used   */
	GLsizei total;											   /* total command storage in buffer  */
	GLubyte *commands;										   /* actual command array             */
} VPMT_CommandBuffer;

void VPMT_CommandBufferDispose(VPMT_CommandBuffer * commands);

#endif

/* $Id: command.h 74 2008-11-23 07:25:12Z hmwill $ */
