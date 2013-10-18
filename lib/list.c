/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Display list functions.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#include "common.h"
#include "GL/gl.h"
#include "context.h"
#include "command.h"
#include "exec.h"
#include "dispatch.h"

/*
** -------------------------------------------------------------------------
** Exported API entry points
** -------------------------------------------------------------------------
*/

static void ExecuteList(VPMT_Context * context, VPMT_CommandBuffer * buffer);

void VPMT_ExecCallList(VPMT_Context * context, GLuint n)
{
	ExecuteList(context, VPMT_HashTableFind(&context->lists, n));
}

void VPMT_ExecCallLists(VPMT_Context * context, GLsizei n, GLenum type, const GLvoid * lists)
{
	GLuint base = context->listBase;

	if (!lists || n < 0) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	switch (type) {
	case GL_BYTE:
		{
			const GLbyte *first = lists;

			while (n--) {
				VPMT_ExecCallList(context, base + *first++);
			}
		}
		break;

	case GL_UNSIGNED_BYTE:
		{
			const GLubyte *first = lists;

			while (n--) {
				VPMT_ExecCallList(context, base + *first++);
			}
		}
		break;

	case GL_SHORT:
		{
			const GLshort *first = lists;

			while (n--) {
				VPMT_ExecCallList(context, base + *first++);
			}
		}
		break;

	case GL_UNSIGNED_SHORT:
		{
			const GLushort *first = lists;

			while (n--) {
				VPMT_ExecCallList(context, base + *first++);
			}
		}
		break;

	case GL_INT:
		{
			const GLint *first = lists;

			while (n--) {
				VPMT_ExecCallList(context, base + *first++);
			}
		}
		break;

	case GL_UNSIGNED_INT:
		{
			const GLuint *first = lists;

			while (n--) {
				VPMT_ExecCallList(context, base + *first++);
			}
		}
		break;

	case GL_FLOAT:
		{
			const GLfloat *first = lists;

			while (n--) {
				VPMT_ExecCallList(context, base + (GLuint) * first++);
			}
		}
		break;

	default:
		VPMT_INVALID_ENUM(context);
		return;
	}
}

void VPMT_ExecEndList(VPMT_Context * context)
{
	GLboolean cleanup = GL_FALSE;

	if (!context->listMode) {
		VPMT_INVALID_OPERATION(context);
		return;
	}
	if (context->compileError) {
		VPMT_SetError(context, context->compileError);
		cleanup = GL_TRUE;
	} else
		if (!VPMT_HashTableInsert(&context->lists, context->listIndex, context->listCommandsHead)) {
		/* out of memory */
		cleanup = GL_TRUE;
	}

	if (cleanup) {
		VPMT_CommandBufferDispose(context->listCommandsHead);
	}

	/* clean up the state */
	context->listMode = 0;
	context->listIndex = 0;
	context->listCommandsHead = NULL;
	context->listCommandsTail = NULL;
	context->compileError = GL_NO_ERROR;
	context->dispatch = &VPMT_DispatchExecute;
}

GLuint VPMT_ExecGenLists(VPMT_Context * context, GLsizei range)
{
	GLuint base;
	GLuint index;

	if (range <= 0) {
		return 0;
	}

	base = VPMT_HashTableFreeKeyBlock(&context->lists, range);

	if (!base) {
		return 0;
	}

	for (index = base; range > 0; --range, ++index) {
		if (!VPMT_HashTableInsert(&context->lists, index, NULL)) {
			VPMT_OUT_OF_MEMORY(context);
			return 0;
		}
	}

	return base;
}

void VPMT_ExecListBase(VPMT_Context * context, GLuint base)
{
	context->listBase = base;
}

void VPMT_ExecNewList(VPMT_Context * context, GLuint list, GLenum mode)
{
	if (context->listMode || context->renderMode != GL_INVALID_MODE) {
		/* nested compilation not allowed */
		VPMT_INVALID_OPERATION(context);
		return;
	}

	if (mode != GL_COMPILE && mode != GL_COMPILE_AND_EXECUTE) {
		VPMT_INVALID_ENUM(context);
		return;
	}

	if (!list) {
		VPMT_INVALID_VALUE(context);
		return;
	}

	/* initialize the list command block */
	context->listCommandsHead = context->listCommandsTail = NULL;
	context->compileError = GL_NO_ERROR;
	context->listMode = mode;
	context->listIndex = list;
	context->dispatch = &VPMT_DispatchRecord;
}

static VPMT_Command *AllocateCommand(VPMT_Context * context, VPMT_Opcode opcode, GLsizei size)
{
	VPMT_CommandBuffer *buffer;
	VPMT_Command *command;

	if (context->compileError == GL_OUT_OF_MEMORY) {
		return NULL;
	}

	buffer = context->listCommandsTail;

	if (buffer == NULL || buffer->total - buffer->used < size) {
		GLubyte *memory = VPMT_MALLOC(sizeof(VPMT_CommandBuffer) + VPMT_COMMAND_BUFFER_SIZE);

		if (!memory) {
			context->compileError = GL_OUT_OF_MEMORY;
			return NULL;
		}

		buffer = (VPMT_CommandBuffer *) memory;
		buffer->commands = memory + sizeof(VPMT_CommandBuffer);
		buffer->total = VPMT_COMMAND_BUFFER_SIZE;
		buffer->used = 0;
		buffer->next = NULL;

		if (!context->listCommandsHead) {
			context->listCommandsHead = context->listCommandsTail = buffer;
		} else {
			context->listCommandsTail->next = buffer;
			context->listCommandsTail = buffer;
		}
	}

	assert(buffer->total - buffer->used >= size);

	command = (VPMT_Command *) (buffer->commands + buffer->used);
	buffer->used += size;
	command->base.opcode = opcode;

	return command;
}

static void AllocateError(VPMT_Context * context, GLenum error)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeError, sizeof(VPMT_CommandError));

	if (command) {
		command->error.error = error;
	}
}

static VPMT_Image2D *AllocateImage(VPMT_Context * context, GLsizei width, GLsizei height,
								   GLenum format, GLenum type, const GLvoid * pixels)
{
	VPMT_Image2D *image;

	assert(type == GL_UNSIGNED_BYTE);
	image = VPMT_Image2DAllocate(VPMT_GetPixelFormat(format, type), width, height);

	if (image) {
		VPMT_Image2D srcImage;
		VPMT_Rect rect;
		const VPMT_PixelFormat *pixelFormat = VPMT_GetPixelFormat(format, type);
		GLsizei pitch = VPMT_ALIGN(width * pixelFormat->size, context->packAlignment);
		VPMT_Image2DInit(&srcImage, pixelFormat, pitch, width, height, (GLubyte *) pixels);

		rect.origin[0] = 0;
		rect.origin[1] = 0;
		rect.size.width = width;
		rect.size.height = height;

		VPMT_Bitblt(image, &rect, &srcImage, NULL);
	}

	return image;
}

static VPMT_INLINE VPMT_Image1D *AllocateImage1D(VPMT_Context * context, GLsizei width,
												 GLenum format, GLenum type, const GLvoid * pixels)
{
	return AllocateImage(context, width, 1, format, type, pixels);
}

static void ExecuteList(VPMT_Context * context, VPMT_CommandBuffer * buffer)
{
	for (; buffer; buffer = buffer->next) {
		GLsizei offset;

		for (offset = 0; offset < buffer->used;) {

			const VPMT_Command *command = (const VPMT_Command *) (buffer->commands + offset);

			switch (command->base.opcode) {
			case VPMT_OpcodeActiveTexture:
				offset += sizeof(command->activeTexture);
				VPMT_ExecActiveTexture(context, command->activeTexture.texture);
				break;

			case VPMT_OpcodeAlphaFunc:
				offset += sizeof(command->alphaFunc);
				VPMT_ExecAlphaFunc(context, command->alphaFunc.func, command->alphaFunc.ref);
				break;

			case VPMT_OpcodeBegin:
				offset += sizeof(command->begin);
				VPMT_ExecBegin(context, command->begin.mode);
				break;

			case VPMT_OpcodeBindTexture:
				offset += sizeof(command->bindTexture);
				VPMT_ExecBindTexture(context, command->bindTexture.target,
									 command->bindTexture.texture);
				break;

			case VPMT_OpcodeBitmap:
				offset += sizeof(command->bitmap);
				VPMT_ExecBitmap(context, command->bitmap.width, command->bitmap.height,
								command->bitmap.xorig, command->bitmap.yorig, command->bitmap.xmove,
								command->bitmap.ymove, command->bitmap.bitmap);
				break;

			case VPMT_OpcodeBlendFunc:
				offset += sizeof(command->blendFunc);
				VPMT_ExecBlendFunc(context, command->blendFunc.sfactor, command->blendFunc.dfactor);
				break;

			case VPMT_OpcodeClear:
				offset += sizeof(command->clear);
				VPMT_ExecClear(context, command->clear.mask);
				break;

			case VPMT_OpcodeClearColor:
				offset += sizeof(command->clearColor);
				VPMT_ExecClearColor(context, command->clearColor.color[0],
									command->clearColor.color[1], command->clearColor.color[2],
									command->clearColor.color[3]);
				break;

			case VPMT_OpcodeClearDepthf:
				offset += sizeof(command->clearDepth);
				VPMT_ExecClearDepthf(context, command->clearDepth.depth);
				break;

			case VPMT_OpcodeClearStencil:
				offset += sizeof(command->clearStencil);
				VPMT_ExecClearStencil(context, command->clearStencil.s);
				break;

			case VPMT_OpcodeColor:
				offset += sizeof(command->color);
				VPMT_ExecColor4fv(context, command->color.color);
				break;

			case VPMT_OpcodeColorMask:
				offset += sizeof(command->colorMask);
				VPMT_ExecColorMask(context, command->colorMask.mask[0], command->colorMask.mask[1],
								   command->colorMask.mask[2], command->colorMask.mask[3]);
				break;

			case VPMT_OpcodeCopyPixels:
				offset += sizeof(command->copyPixels);
				VPMT_ExecCopyPixels(context, command->copyPixels.rect.origin[0],
									command->copyPixels.rect.origin[1],
									command->copyPixels.rect.size.width,
									command->copyPixels.rect.size.height, command->copyPixels.type);
				break;

			case VPMT_OpcodeCullFace:
				offset += sizeof(command->cullFace);
				VPMT_ExecCullFace(context, command->cullFace.mode);
				break;

			case VPMT_OpcodeDepthFunc:
				offset += sizeof(command->depthFunc);
				VPMT_ExecDepthFunc(context, command->depthFunc.func);
				break;

			case VPMT_OpcodeDepthMask:
				offset += sizeof(command->depthMask);
				VPMT_ExecDepthMask(context, command->depthMask.flag);
				break;

			case VPMT_OpcodeDepthRangef:
				offset += sizeof(command->depthRangef);
				VPMT_ExecDepthRangef(context, command->depthRangef.zNear,
									 command->depthRangef.zFar);
				break;

			case VPMT_OpcodeDrawPixels:
				offset += sizeof(command->drawPixels);
				VPMT_ExecDrawPixelsImage(context, command->drawPixels.image);
				break;

			case VPMT_OpcodeEnd:
				offset += sizeof(command->base);
				VPMT_ExecEnd(context);
				break;

			case VPMT_OpcodeError:
				offset += sizeof(command->error);
				VPMT_SetError(context, command->error.error);
				break;

			case VPMT_OpcodeFrontFace:
				offset += sizeof(command->frontFace);
				VPMT_ExecFrontFace(context, command->frontFace.mode);
				break;

			case VPMT_OpcodeHint:
				offset += sizeof(command->hint);
				VPMT_ExecHint(context, command->hint.target, command->hint.mode);
				break;

			case VPMT_OpcodeLightfv:
				offset += sizeof(command->lighfv);
				VPMT_ExecLightfv(context, command->lighfv.light, command->lighfv.pname,
								 command->lighfv.params);
				break;

			case VPMT_OpcodeLightModelfv:
				offset += sizeof(command->lightModelfv);
				VPMT_ExecLightModelfv(context, command->lightModelfv.pname,
									  command->lightModelfv.params);
				break;

			case VPMT_OpcodeLineStipple:
				offset += sizeof(command->lineStipple);
				VPMT_ExecLineStipple(context, command->lineStipple.factor,
									 command->lineStipple.pattern);
				break;

			case VPMT_OpcodeLineWidth:
				offset += sizeof(command->lineWidth);
				VPMT_ExecLineWidth(context, command->lineWidth.width);
				break;

			case VPMT_OpcodeListBase:
				offset += sizeof(command->listBase);
				VPMT_ExecListBase(context, command->listBase.listBase);
				break;

			case VPMT_OpcodeLoadIdentity:
				offset += sizeof(command->base);
				VPMT_ExecLoadIdentity(context);
				break;

			case VPMT_OpcodeLoadMatrix:
				offset += sizeof(command->loadMatrix);
				VPMT_ExecLoadMatrixf(context, command->loadMatrix.matrix);
				break;

			case VPMT_OpcodeMaterialfv:
				offset += sizeof(command->materialfv);
				VPMT_ExecMaterialfv(context, command->materialfv.face, command->materialfv.pname,
									command->materialfv.params);
				break;

			case VPMT_OpcodeMatrixMode:
				offset += sizeof(command->matrixMode);
				VPMT_ExecMatrixMode(context, command->matrixMode.mode);
				break;

			case VPMT_OpcodeMultMatrix:
				offset += sizeof(command->multMatrix);
				VPMT_ExecMultMatrixf(context, command->multMatrix.matrix);
				break;

			case VPMT_OpcodeMultiTexCoord:
				offset += sizeof(command->multiTexCoord);
				VPMT_ExecMultiTexCoord2f(context, command->multiTexCoord.target,
										 command->multiTexCoord.coords[0],
										 command->multiTexCoord.coords[1]);
				break;

			case VPMT_OpcodeNormal:
				offset += sizeof(command->normal);
				VPMT_ExecNormal3fv(context, command->normal.normal);
				break;

			case VPMT_OpcodePointSize:
				offset += sizeof(command->pointSize);
				VPMT_ExecPointSize(context, command->pointSize.size);
				break;

			case VPMT_OpcodePolygonOffset:
				offset += sizeof(command->polygonOffset);
				VPMT_ExecPolygonOffset(context, command->polygonOffset.factor,
									   command->polygonOffset.units);
				break;

			case VPMT_OpcodePolygonStipple:
				offset += sizeof(command->polygonStipple);
				VPMT_ExecPolygonStipple(context, command->polygonStipple.mask.bytes);
				break;

			case VPMT_OpcodePopMatrix:
				offset += sizeof(command->base);
				VPMT_ExecPopMatrix(context);
				break;

			case VPMT_OpcodePushMatrix:
				offset += sizeof(command->base);
				VPMT_ExecPushMatrix(context);
				break;

			case VPMT_OpcodeRasterPos:
				offset += sizeof(command->rasterPos);
				VPMT_ExecRasterPos3f(context, command->rasterPos.position[0],
									 command->rasterPos.position[1],
									 command->rasterPos.position[2]);
				break;

			case VPMT_OpcodeScissor:
				offset += sizeof(command->scissor);
				VPMT_ExecScissor(context, command->scissor.rect.origin[0],
								 command->scissor.rect.origin[1], command->scissor.rect.size.width,
								 command->scissor.rect.size.height);
				break;

			case VPMT_OpcodeShadeModel:
				offset += sizeof(command->shadeModel);
				VPMT_ExecShadeModel(context, command->shadeModel.mode);
				break;

			case VPMT_OpcodeStencilFunc:
				offset += sizeof(command->stencilFunc);
				VPMT_ExecStencilFunc(context, command->stencilFunc.func, command->stencilFunc.ref,
									 command->stencilFunc.ref);
				break;

			case VPMT_OpcodeStencilMask:
				offset += sizeof(command->stencilMask);
				VPMT_ExecStencilMask(context, command->stencilMask.mask);
				break;

			case VPMT_OpcodeStencilOp:
				offset += sizeof(command->stencilOp);
				VPMT_ExecStencilOp(context, command->stencilOp.fail, command->stencilOp.zfail,
								   command->stencilOp.zpass);
				break;

			case VPMT_OpcodeTexEnvfv:
				offset += sizeof(command->texEnvfv);
				VPMT_ExecTexEnvfv(context, command->texEnvfv.target, command->texEnvfv.pname,
								  command->texEnvfv.params);
				break;

			case VPMT_OpcodeTexEnvi:
				offset += sizeof(command->texEnvi);
				VPMT_ExecTexEnvi(context, command->texEnvi.target, command->texEnvi.pname,
								 command->texEnvi.param);
				break;

			case VPMT_OpcodeTexImage2D:
				offset += sizeof(command->texImage2D);
				VPMT_ExecTexImage2DImage(context, command->texImage2D.target,
										 command->texImage2D.level,
										 command->texImage2D.image->pixelFormat->internalFormat,
										 command->texImage2D.image);
				break;

			case VPMT_OpcodeTexParameteri:
				offset += sizeof(command->texParameteri);
				VPMT_ExecTexParameteri(context, command->texParameteri.target,
									   command->texParameteri.pname, command->texParameteri.param);
				break;

			case VPMT_OpcodeTexSubImage2D:
				offset += sizeof(command->texSubImage2D);
				VPMT_ExecTexSubImage2DImage(context, command->texSubImage2D.target,
											command->texSubImage2D.level,
											command->texSubImage2D.xoffset,
											command->texSubImage2D.yoffset,
											command->texSubImage2D.image);
				break;

			case VPMT_OpcodeToggle:
				offset += sizeof(command->toggle);

				if (command->toggle.enable) {
					VPMT_ExecEnable(context, command->toggle.cap);
				} else {
					VPMT_ExecDisable(context, command->toggle.cap);
				}

				break;

			case VPMT_OpcodeVertex:
				offset += sizeof(command->vertex);
				VPMT_ExecVertex3fv(context, command->vertex.vertex);
				break;

			case VPMT_OpcodeViewport:
				offset += sizeof(command->viewport);
				VPMT_ExecViewport(context, command->viewport.rect.origin[0],
								  command->viewport.rect.origin[1],
								  command->viewport.rect.size.width,
								  command->viewport.rect.size.height);
				break;

#if GL_EXT_paletted_texture
			case VPMT_OpcodeColorSubTable:
				offset += sizeof(command->colorSubTable);
				VPMT_ExecColorSubTableImage(context,
											command->colorSubTable.target,
											command->colorSubTable.start,
											command->colorSubTable.palette);

				break;

			case VPMT_OpcodeColorTable:
				offset += sizeof(command->colorTable);
				VPMT_ExecColorTableImage(context,
										 command->colorTable.target, command->colorTable.palette);
				break;
#endif
			default:
				/* corrupted list: terminate execution */
				assert(GL_FALSE);
				return;
			}
		}
	}
}

void VPMT_RecActiveTexture(VPMT_Context * context, GLenum texture)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeActiveTexture, sizeof(VPMT_CommandActiveTexture));

	if (command) {
		command->activeTexture.texture = texture;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecActiveTexture(context, texture);
	}
}

void VPMT_RecAlphaFunc(VPMT_Context * context, GLenum func, GLclampf ref)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeAlphaFunc, sizeof(VPMT_CommandAlphaFunc));

	if (command) {
		command->alphaFunc.func = func;
		command->alphaFunc.ref = ref;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecAlphaFunc(context, func, ref);
	}
}

void VPMT_RecBegin(VPMT_Context * context, GLenum mode)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeBegin, sizeof(VPMT_CommandBegin));

	if (command) {
		command->begin.mode = mode;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecBegin(context, mode);
	}
}

void VPMT_RecBindTexture(VPMT_Context * context, GLenum target, GLuint name)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeBindTexture, sizeof(VPMT_CommandBindTexture));

	if (command) {
		command->bindTexture.target = target;
		command->bindTexture.texture = name;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecBindTexture(context, target, name);
	}
}

void VPMT_RecBitmap(VPMT_Context * context, GLsizei width, GLsizei height, GLfloat xorig,
					GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap)
{
	if (!bitmap) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		GLsizei numBytes = height * ((width + 7) >> 3);
		GLubyte *copy = VPMT_MALLOC(numBytes);

		if (!copy) {
			AllocateError(context, GL_OUT_OF_MEMORY);
		} else {
			VPMT_Command *command =
				AllocateCommand(context, VPMT_OpcodeBitmap, sizeof(VPMT_CommandBitmap));

			if (command) {
				command->bitmap.width = width;
				command->bitmap.height = height;
				command->bitmap.xorig = xorig;
				command->bitmap.yorig = yorig;
				command->bitmap.xmove = xmove;
				command->bitmap.ymove = ymove;
				command->bitmap.bitmap = copy;

				memcpy(copy, bitmap, numBytes);
			} else {
				VPMT_FREE(copy);
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecBitmap(context, width, height, xorig, yorig, xmove, ymove, bitmap);
	}
}

void VPMT_RecBlendFunc(VPMT_Context * context, GLenum sfactor, GLenum dfactor)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeBlendFunc, sizeof(VPMT_CommandBlendFunc));

	if (command) {
		command->blendFunc.sfactor = sfactor;
		command->blendFunc.dfactor = dfactor;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecBlendFunc(context, sfactor, dfactor);
	}
}

void VPMT_RecCallLists(VPMT_Context * context, GLsizei n, GLenum type, const GLvoid * lists)
{
	/* no support for nested lists */
	AllocateError(context, GL_INVALID_OPERATION);
}

void VPMT_RecClear(VPMT_Context * context, GLbitfield mask)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeClear, sizeof(VPMT_CommandClear));

	if (command) {
		command->clear.mask = mask;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecClear(context, mask);
	}
}

void VPMT_RecClearColor(VPMT_Context * context, GLclampf red, GLclampf green, GLclampf blue,
						GLclampf alpha)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeClearColor, sizeof(VPMT_CommandClearColor));

	if (command) {
		command->clearColor.color[0] = red;
		command->clearColor.color[1] = green;
		command->clearColor.color[2] = blue;
		command->clearColor.color[3] = alpha;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecClearColor(context, red, green, blue, alpha);
	}
}

void VPMT_RecClearDepthf(VPMT_Context * context, GLclampf depth)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeClearDepthf, sizeof(VPMT_CommandClearDepth));

	if (command) {
		command->clearDepth.depth = depth;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecClearDepthf(context, depth);
	}
}

void VPMT_RecClearStencil(VPMT_Context * context, GLint s)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeClearStencil, sizeof(VPMT_CommandClearStencil));

	if (command) {
		command->clearStencil.s = s;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecClearStencil(context, s);
	}
}

void VPMT_RecColor4f(VPMT_Context * context, GLfloat red, GLfloat green, GLfloat blue,
					 GLfloat alpha)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeColor, sizeof(VPMT_CommandColor));

	if (command) {
		command->color.color[0] = red;
		command->color.color[1] = green;
		command->color.color[2] = blue;
		command->color.color[3] = alpha;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecColor4f(context, red, green, blue, alpha);
	}
}

void VPMT_RecColor4fv(VPMT_Context * context, const GLfloat * v)
{
	if (!v) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeColor, sizeof(VPMT_CommandColor));

		if (command) {
			VPMT_Vec4Copy(command->color.color, v);
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecColor4fv(context, v);
	}
}

void VPMT_RecColor4ub(VPMT_Context * context, GLubyte red, GLubyte green, GLubyte blue,
					  GLubyte alpha)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeColor, sizeof(VPMT_CommandColor));

	if (command) {
		command->color.color[0] = VPMT_UBYTE_TO_FLOAT(red);
		command->color.color[1] = VPMT_UBYTE_TO_FLOAT(green);
		command->color.color[2] = VPMT_UBYTE_TO_FLOAT(blue);
		command->color.color[3] = VPMT_UBYTE_TO_FLOAT(alpha);
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecColor4ub(context, red, green, blue, alpha);
	}
}

void VPMT_RecColorMask(VPMT_Context * context, GLboolean red, GLboolean green, GLboolean blue,
					   GLboolean alpha)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeColorMask, sizeof(VPMT_CommandColorMask));

	if (command) {
		command->colorMask.mask[0] = red;
		command->colorMask.mask[1] = green;
		command->colorMask.mask[2] = blue;
		command->colorMask.mask[3] = alpha;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecColorMask(context, red, green, blue, alpha);
	}
}

void VPMT_RecCopyPixels(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height,
						GLenum type)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeCopyPixels, sizeof(VPMT_CommandCopyPixels));

	if (command) {
		command->copyPixels.rect.origin[0] = x;
		command->copyPixels.rect.origin[1] = y;
		command->copyPixels.rect.size.width = width;
		command->copyPixels.rect.size.height = height;
		command->copyPixels.type = type;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecCopyPixels(context, x, y, width, height, type);
	}
}

void VPMT_RecCullFace(VPMT_Context * context, GLenum mode)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeCullFace, sizeof(VPMT_CommandCullFace));

	if (command) {
		command->cullFace.mode = mode;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecCullFace(context, mode);
	}
}

void VPMT_RecDepthFunc(VPMT_Context * context, GLenum func)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeDepthFunc, sizeof(VPMT_CommandDepthFunc));

	if (command) {
		command->depthFunc.func = func;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecDepthFunc(context, func);
	}
}

void VPMT_RecDepthMask(VPMT_Context * context, GLboolean flag)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeDepthMask, sizeof(VPMT_CommandDepthMask));

	if (command) {
		command->depthMask.flag = flag;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecDepthMask(context, flag);
	}
}

void VPMT_RecDepthRangef(VPMT_Context * context, GLclampf zNear, GLclampf zFar)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeDepthRangef, sizeof(VPMT_CommandDepthRangef));

	if (command) {
		command->depthRangef.zNear = zNear;
		command->depthRangef.zFar = zFar;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecDepthRangef(context, zNear, zFar);
	}
}

void VPMT_RecDisable(VPMT_Context * context, GLenum cap)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeToggle, sizeof(VPMT_CommandToggle));

	if (command) {
		command->toggle.cap = cap;
		command->toggle.enable = GL_FALSE;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecDisable(context, cap);
	}
}

void VPMT_RecDrawPixels(VPMT_Context * context, GLsizei width, GLsizei height, GLenum format,
						GLenum type, const GLvoid * pixels)
{
	if (type != GL_UNSIGNED_BYTE || format != GL_RGBA) {
		AllocateError(context, GL_INVALID_ENUM);
	} else if (width <= 0 || height <= 0 || !pixels) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Image2D *image = AllocateImage(context, width, height, format, type, pixels);

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
		} else {
			VPMT_Command *command =
				AllocateCommand(context, VPMT_OpcodeDrawPixels, sizeof(VPMT_CommandDrawPixels));

			if (command) {
				command->drawPixels.image = image;
			} else {
				VPMT_Image2DDeallocate(image);
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecDrawPixels(context, width, height, format, type, pixels);
	}
}

void VPMT_RecEnable(VPMT_Context * context, GLenum cap)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeToggle, sizeof(VPMT_CommandToggle));

	if (command) {
		command->toggle.cap = cap;
		command->toggle.enable = GL_TRUE;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecEnable(context, cap);
	}
}

void VPMT_RecEnd(VPMT_Context * context)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeEnd, sizeof(VPMT_CommandBase));

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecEnd(context);
	}
}

void VPMT_RecFrontFace(VPMT_Context * context, GLenum mode)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeFrontFace, sizeof(VPMT_CommandFrontFace));

	if (command) {
		command->frontFace.mode = mode;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecFrontFace(context, mode);
	}
}

void VPMT_RecFrustumf(VPMT_Context * context, GLfloat left, GLfloat right, GLfloat bottom,
					  GLfloat top, GLfloat zNear, GLfloat zFar)
{
	if (left == right || top == bottom || zNear == zFar) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeMultMatrix, sizeof(VPMT_CommandMultMatrix));

		if (command) {
			VPMT_MatrixFrustumf(command->multMatrix.matrix, left, right, bottom, top, zNear, zFar);
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecFrustumf(context, left, right, bottom, top, zNear, zFar);
	}
}

void VPMT_RecHint(VPMT_Context * context, GLenum target, GLenum mode)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeHint, sizeof(VPMT_CommandHint));

	if (command) {
		command->hint.target = target;
		command->hint.mode = mode;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecHint(context, target, mode);
	}
}

void VPMT_RecLightfv(VPMT_Context * context, GLenum light, GLenum pname, const GLfloat * params)
{
	if (!params) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeLightfv, sizeof(VPMT_CommandLightfv));

		if (command) {
			command->lighfv.light = light;
			command->lighfv.pname = pname;

			switch (pname) {
			case GL_AMBIENT:
			case GL_DIFFUSE:
			case GL_SPECULAR:
			case GL_POSITION:
				VPMT_Vec4Copy(command->lighfv.params, params);
				break;
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecLightfv(context, light, pname, params);
	}
}

void VPMT_RecLightModelf(VPMT_Context * context, GLenum pname, GLfloat param)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeLightModelfv, sizeof(VPMT_CommandLightModelfv));

	if (command) {
		command->lighfv.pname = pname;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecLightModelf(context, pname, param);
	}
}

void VPMT_RecLightModelfv(VPMT_Context * context, GLenum pname, const GLfloat * params)
{
	if (!params) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeLightModelfv, sizeof(VPMT_CommandLightModelfv));

		if (command) {
			command->lightModelfv.pname = pname;

			switch (pname) {
			case GL_LIGHT_MODEL_AMBIENT:
				VPMT_Vec4Copy(command->lightModelfv.params, params);
				break;
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecLightModelfv(context, pname, params);
	}
}

void VPMT_RecLineStipple(VPMT_Context * context, GLint factor, GLushort pattern)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeLineStipple, sizeof(VPMT_CommandLineStipple));

	if (command) {
		command->lineStipple.factor = factor;
		command->lineStipple.pattern = pattern;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecLineStipple(context, factor, pattern);
	}
}

void VPMT_RecLineWidth(VPMT_Context * context, GLfloat width)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeLineWidth, sizeof(VPMT_CommandLineWidth));

	if (command) {
		command->lineWidth.width = width;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecLineWidth(context, width);
	}
}

void VPMT_RecListBase(VPMT_Context * context, GLuint base)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeListBase, sizeof(VPMT_CommandListBase));

	if (command) {
		command->listBase.listBase = base;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecListBase(context, base);
	}
}

void VPMT_RecLoadIdentity(VPMT_Context * context)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeLoadIdentity, sizeof(VPMT_CommandBase));

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecLoadIdentity(context);
	}
}

void VPMT_RecLoadMatrixf(VPMT_Context * context, const GLfloat * m)
{
	if (!m) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeLoadMatrix, sizeof(VPMT_CommandLoadMatrix));

		if (command) {
			VPMT_MatrixCopy(command->loadMatrix.matrix, m);
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecLoadMatrixf(context, m);
	}
}

void VPMT_RecMaterialf(VPMT_Context * context, GLenum face, GLenum pname, GLfloat param)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeMaterialfv, sizeof(VPMT_CommandMaterialfv));

	if (command) {
		command->materialfv.face = face;
		command->materialfv.pname = pname;
		command->materialfv.params[0] = param;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecMaterialf(context, face, pname, param);
	}
}

void VPMT_RecMaterialfv(VPMT_Context * context, GLenum face, GLenum pname, const GLfloat * params)
{
	if (!params) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeMaterialfv, sizeof(VPMT_CommandMaterialfv));

		if (command) {
			command->materialfv.face = face;
			command->materialfv.pname = pname;

			switch (pname) {
			case GL_AMBIENT:
			case GL_DIFFUSE:
			case GL_AMBIENT_AND_DIFFUSE:
			case GL_SPECULAR:
			case GL_EMISSION:
				VPMT_Vec4Copy(command->materialfv.params, params);
				break;

			case GL_SHININESS:
				command->materialfv.params[0] = *params;
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecMaterialfv(context, face, pname, params);
	}
}

void VPMT_RecMatrixMode(VPMT_Context * context, GLenum mode)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeMatrixMode, sizeof(VPMT_CommandMatrixMode));

	if (command) {
		command->matrixMode.mode = mode;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecMatrixMode(context, mode);
	}
}

void VPMT_RecMultiTexCoord2f(VPMT_Context * context, GLenum target, GLfloat s, GLfloat t)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeMultiTexCoord, sizeof(VPMT_CommandMultiTexCoord));

	if (command) {
		command->multiTexCoord.target = target;
		command->multiTexCoord.coords[0] = s;
		command->multiTexCoord.coords[1] = t;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecMultiTexCoord2f(context, target, s, t);
	}
}

void VPMT_RecMultMatrixf(VPMT_Context * context, const GLfloat * m)
{
	if (!m) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeMultMatrix, sizeof(VPMT_CommandMultMatrix));

		if (command) {
			VPMT_MatrixCopy(command->multMatrix.matrix, m);
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecMultMatrixf(context, m);
	}
}

void VPMT_RecNormal3f(VPMT_Context * context, GLfloat nx, GLfloat ny, GLfloat nz)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeNormal, sizeof(VPMT_CommandNormal));

	if (command) {
		command->normal.normal[0] = nx;
		command->normal.normal[1] = ny;
		command->normal.normal[2] = nz;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecNormal3f(context, nx, ny, nz);
	}
}

void VPMT_RecNormal3fv(VPMT_Context * context, const GLfloat * v)
{
	if (!v) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeNormal, sizeof(VPMT_CommandNormal));

		if (command) {
			VPMT_Vec3Copy(command->normal.normal, v);
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecNormal3fv(context, v);
	}
}

void VPMT_RecOrthof(VPMT_Context * context, GLfloat left, GLfloat right, GLfloat bottom,
					GLfloat top, GLfloat zNear, GLfloat zFar)
{
	if (left == right || top == bottom || zNear == zFar) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeMultMatrix, sizeof(VPMT_CommandMultMatrix));

		if (command) {
			VPMT_MatrixOrthof(command->multMatrix.matrix, left, right, bottom, top, zNear, zFar);
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecOrthof(context, left, right, bottom, top, zNear, zFar);
	}
}

void VPMT_RecPointSize(VPMT_Context * context, GLfloat size)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodePointSize, sizeof(VPMT_CommandPointSize));

	if (command) {
		command->pointSize.size = size;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecPointSize(context, size);
	}
}

void VPMT_RecPolygonOffset(VPMT_Context * context, GLfloat factor, GLfloat units)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodePolygonOffset, sizeof(VPMT_CommandPolygonOffset));

	if (command) {
		command->polygonOffset.factor = factor;
		command->polygonOffset.units = units;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecPolygonOffset(context, factor, units);
	}
}

void VPMT_RecPolygonStipple(VPMT_Context * context, const GLubyte * mask)
{
	if (!mask) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodePolygonStipple, sizeof(VPMT_CommandPolygonStipple));

		if (command) {
			memcpy(command->polygonStipple.mask.bytes, mask,
				   sizeof command->polygonStipple.mask.bytes);
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecPolygonStipple(context, mask);
	}
}

void VPMT_RecPopMatrix(VPMT_Context * context)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodePopMatrix, sizeof(VPMT_CommandBase));

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecPopMatrix(context);
	}
}

void VPMT_RecPushMatrix(VPMT_Context * context)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodePushMatrix, sizeof(VPMT_CommandBase));

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecPushMatrix(context);
	}
}

void VPMT_RecRasterPos3f(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeRasterPos, sizeof(VPMT_CommandRasterPos));

	if (command) {
		command->rasterPos.position[0] = x;
		command->rasterPos.position[1] = y;
		command->rasterPos.position[2] = z;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecRasterPos3f(context, x, y, z);
	}
}

void VPMT_RecRotatef(VPMT_Context * context, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeMultMatrix, sizeof(VPMT_CommandMultMatrix));

	if (command) {
		VPMT_MatrixRotatef(command->multMatrix.matrix, angle, x, y, z);
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecRotatef(context, angle, x, y, z);
	}
}

void VPMT_RecScalef(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeMultMatrix, sizeof(VPMT_CommandMultMatrix));

	if (command) {
		VPMT_MatrixScalef(command->multMatrix.matrix, x, y, z);
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecScalef(context, x, y, z);
	}
}

void VPMT_RecScissor(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeScissor, sizeof(VPMT_CommandScissor));

	if (command) {
		command->scissor.rect.origin[0] = x;
		command->scissor.rect.origin[1] = y;
		command->scissor.rect.size.width = width;
		command->scissor.rect.size.height = height;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecScissor(context, x, y, width, height);
	}
}

void VPMT_RecShadeModel(VPMT_Context * context, GLenum mode)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeShadeModel, sizeof(VPMT_CommandShadeModel));

	if (command) {
		command->shadeModel.mode = mode;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecShadeModel(context, mode);
	}
}

void VPMT_RecStencilFunc(VPMT_Context * context, GLenum func, GLint ref, GLuint mask)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeStencilFunc, sizeof(VPMT_CommandStencilFunc));

	if (command) {
		command->stencilFunc.func = func;
		command->stencilFunc.ref = ref;
		command->stencilFunc.mask = mask;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecStencilFunc(context, func, ref, mask);
	}
}

void VPMT_RecStencilMask(VPMT_Context * context, GLuint mask)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeStencilMask, sizeof(VPMT_CommandStencilMask));

	if (command) {
		command->stencilMask.mask = mask;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecStencilMask(context, mask);
	}
}

void VPMT_RecStencilOp(VPMT_Context * context, GLenum fail, GLenum zfail, GLenum zpass)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeStencilOp, sizeof(VPMT_CommandStencilOp));

	if (command) {
		command->stencilOp.fail = fail;
		command->stencilOp.zfail = zfail;
		command->stencilOp.zpass = zpass;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecStencilOp(context, fail, zfail, zpass);
	}
}

void VPMT_RecTexEnvfv(VPMT_Context * context, GLenum target, GLenum pname, const GLfloat * params)
{
	if (!params) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeTexEnvfv, sizeof(VPMT_CommandTexEnvfv));

		if (command) {
			command->texEnvfv.target = target;
			command->texEnvfv.pname = pname;

			switch (pname) {
			case GL_TEXTURE_ENV_COLOR:
				VPMT_Vec4Copy(command->texEnvfv.params, params);
				break;
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecTexEnvfv(context, target, pname, params);
	}
}

void VPMT_RecTexEnvi(VPMT_Context * context, GLenum target, GLenum pname, GLint param)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeTexEnvi, sizeof(VPMT_CommandTexEnvi));

	if (command) {
		command->texEnvi.target = target;
		command->texEnvi.pname = pname;
		command->texEnvi.param = param;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecTexEnvi(context, target, pname, param);
	}
}

void VPMT_RecTexImage2D(VPMT_Context * context, GLenum target, GLint level, GLint internalformat,
						GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
						const GLvoid * pixels)
{
	if (type != GL_UNSIGNED_BYTE || !VPMT_ValidateTextureFormat(format) ||
		!VPMT_ValidateInternalFormat(internalformat)) {
		AllocateError(context, GL_INVALID_ENUM);
	} else if (width <= 0 || height <= 0 || !pixels || border != 0) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Image2D *image = AllocateImage(context, width, height, format, type, pixels);

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
		} else {
			VPMT_Command *command =
				AllocateCommand(context, VPMT_OpcodeTexImage2D, sizeof(VPMT_CommandTexImage2D));

			if (command) {
				command->texImage2D.level = level;
				command->texImage2D.target = target;
				command->texImage2D.image = image;
			} else {
				VPMT_Image2DDeallocate(image);
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecTexImage2D(context, target, level, internalformat, width, height, border, format,
							type, pixels);
	}
}

void VPMT_RecTexParameteri(VPMT_Context * context, GLenum target, GLenum pname, GLint param)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeTexParameteri, sizeof(VPMT_CommandTexParameteri));

	if (command) {
		command->texParameteri.target = target;
		command->texParameteri.pname = pname;
		command->texParameteri.param = param;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecTexParameteri(context, target, pname, param);
	}
}

void VPMT_RecTexSubImage2D(VPMT_Context * context, GLenum target, GLint level, GLint xoffset,
						   GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type,
						   const GLvoid * pixels)
{
	if (type != GL_UNSIGNED_BYTE || !VPMT_ValidateTextureFormat(format)) {
		AllocateError(context, GL_INVALID_ENUM);
	} else if (width <= 0 || height <= 0 || !pixels) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Image2D *image = AllocateImage(context, width, height, format, type, pixels);

		if (!image) {
			VPMT_OUT_OF_MEMORY(context);
		} else {
			VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeTexSubImage2D,
													sizeof(VPMT_CommandTexSubImage2D));

			if (command) {
				command->texSubImage2D.level = level;
				command->texSubImage2D.target = target;
				command->texSubImage2D.xoffset = xoffset;
				command->texSubImage2D.yoffset = yoffset;
				command->texSubImage2D.image = image;
			} else {
				VPMT_Image2DDeallocate(image);
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecTexSubImage2D(context, target, level, xoffset, yoffset, width, height, format,
							   type, pixels);
	}
}

void VPMT_RecTranslatef(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeMultMatrix, sizeof(VPMT_CommandMultMatrix));

	if (command) {
		VPMT_MatrixTranslatef(command->multMatrix.matrix, x, y, z);
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecTranslatef(context, x, y, z);
	}
}

void VPMT_RecVertex2f(VPMT_Context * context, GLfloat x, GLfloat y)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeVertex, sizeof(VPMT_CommandVertex));

	if (command) {
		command->vertex.vertex[0] = x;
		command->vertex.vertex[1] = y;
		command->vertex.vertex[2] = 0.0f;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecVertex2f(context, x, y);
	}
}

void VPMT_RecVertex2fv(VPMT_Context * context, const GLfloat * v)
{
	if (!v) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeVertex, sizeof(VPMT_CommandVertex));

		if (command) {
			command->vertex.vertex[0] = v[0];
			command->vertex.vertex[1] = v[1];
			command->vertex.vertex[2] = 0.0f;
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecVertex2fv(context, v);
	}
}

void VPMT_RecVertex3f(VPMT_Context * context, GLfloat x, GLfloat y, GLfloat z)
{
	VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeVertex, sizeof(VPMT_CommandVertex));

	if (command) {
		command->vertex.vertex[0] = x;
		command->vertex.vertex[1] = y;
		command->vertex.vertex[2] = z;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecVertex3f(context, x, y, z);
	}
}

void VPMT_RecVertex3fv(VPMT_Context * context, const GLfloat * v)
{
	if (!v) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Command *command =
			AllocateCommand(context, VPMT_OpcodeVertex, sizeof(VPMT_CommandVertex));

		if (command) {
			command->vertex.vertex[0] = v[0];
			command->vertex.vertex[1] = v[1];
			command->vertex.vertex[2] = v[2];
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecVertex3fv(context, v);
	}
}

void VPMT_RecViewport(VPMT_Context * context, GLint x, GLint y, GLsizei width, GLsizei height)
{
	VPMT_Command *command =
		AllocateCommand(context, VPMT_OpcodeViewport, sizeof(VPMT_CommandViewport));

	if (command) {
		command->viewport.rect.origin[0] = x;
		command->viewport.rect.origin[1] = y;
		command->viewport.rect.size.width = width;
		command->viewport.rect.size.height = height;
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecViewport(context, x, y, width, height);
	}
}

#if GL_EXT_paletted_texture

void VPMT_RecColorSubTable(VPMT_Context * context, GLenum target, GLsizei start, GLsizei count,
						   GLenum format, GLenum type, const GLvoid * table)
{
	if (target != GL_TEXTURE_2D || format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
		AllocateError(context, GL_INVALID_ENUM);
	} else if (start < 0 || count <= 0 || !table) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Image1D *palette = AllocateImage1D(context, count, format, type, table);

		if (!palette) {
			VPMT_OUT_OF_MEMORY(context);
		} else {
			VPMT_Command *command = AllocateCommand(context, VPMT_OpcodeColorSubTable,
													sizeof(VPMT_CommandColorSubTable));

			if (command) {
				command->colorSubTable.target = target;
				command->colorSubTable.start = start;
				command->colorSubTable.palette = palette;
			} else {
				VPMT_Image1DDeallocate(palette);
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecColorSubTable(context, target, start, count, format, type, table);
	}
}

void VPMT_RecColorTable(VPMT_Context * context, GLenum target, GLenum internalformat, GLsizei width,
						GLenum format, GLenum type, const GLvoid * table)
{
	if (target != GL_TEXTURE_2D || format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
		AllocateError(context, GL_INVALID_ENUM);
	} else if (width <= 0 || !table) {
		AllocateError(context, GL_INVALID_VALUE);
	} else {
		VPMT_Image1D *palette = AllocateImage1D(context, width, format, type, table);

		if (!palette) {
			VPMT_OUT_OF_MEMORY(context);
		} else {
			VPMT_Command *command =
				AllocateCommand(context, VPMT_OpcodeColorTable, sizeof(VPMT_CommandColorTable));

			if (command) {
				command->colorTable.target = target;
				command->colorTable.palette = palette;
			} else {
				VPMT_Image1DDeallocate(palette);
			}
		}
	}

	if (context->listMode == GL_COMPILE_AND_EXECUTE) {
		VPMT_ExecColorTable(context, target, internalformat, width, format, type, table);
	}
}

#endif

static void CleanupList(VPMT_CommandBuffer * buffer)
{
	for (; buffer; buffer = buffer->next) {
		GLsizei offset;

		for (offset = 0; offset < buffer->used;) {

			const VPMT_Command *command = (const VPMT_Command *) (buffer->commands + offset);

			switch (command->base.opcode) {
			case VPMT_OpcodeActiveTexture:
				offset += sizeof(command->activeTexture);
				break;

			case VPMT_OpcodeAlphaFunc:
				offset += sizeof(command->alphaFunc);
				break;

			case VPMT_OpcodeBegin:
				offset += sizeof(command->begin);
				break;

			case VPMT_OpcodeBindTexture:
				offset += sizeof(command->bindTexture);
				break;

			case VPMT_OpcodeBitmap:
				offset += sizeof(command->bitmap);
				VPMT_FREE(command->bitmap.bitmap);
				break;

			case VPMT_OpcodeBlendFunc:
				offset += sizeof(command->blendFunc);
				break;

			case VPMT_OpcodeClear:
				offset += sizeof(command->clear);
				break;

			case VPMT_OpcodeClearColor:
				offset += sizeof(command->clearColor);
				break;

			case VPMT_OpcodeClearDepthf:
				offset += sizeof(command->clearDepth);
				break;

			case VPMT_OpcodeClearStencil:
				offset += sizeof(command->clearStencil);
				break;

			case VPMT_OpcodeColor:
				offset += sizeof(command->color);
				break;

			case VPMT_OpcodeColorMask:
				offset += sizeof(command->colorMask);
				break;

			case VPMT_OpcodeCopyPixels:
				offset += sizeof(command->copyPixels);
				break;

			case VPMT_OpcodeCullFace:
				offset += sizeof(command->cullFace);
				break;

			case VPMT_OpcodeDepthFunc:
				offset += sizeof(command->depthFunc);
				break;

			case VPMT_OpcodeDepthMask:
				offset += sizeof(command->depthMask);
				break;

			case VPMT_OpcodeDepthRangef:
				offset += sizeof(command->depthRangef);
				break;

			case VPMT_OpcodeDrawPixels:
				offset += sizeof(command->drawPixels);
				VPMT_Image2DDeallocate(command->drawPixels.image);
				break;

			case VPMT_OpcodeEnd:
				offset += sizeof(command->base);
				break;

			case VPMT_OpcodeError:
				offset += sizeof(command->error);
				break;

			case VPMT_OpcodeFrontFace:
				offset += sizeof(command->frontFace);
				break;

			case VPMT_OpcodeHint:
				offset += sizeof(command->hint);
				break;

			case VPMT_OpcodeLightfv:
				offset += sizeof(command->lighfv);
				break;

			case VPMT_OpcodeLightModelfv:
				offset += sizeof(command->lightModelfv);
				break;

			case VPMT_OpcodeLineStipple:
				offset += sizeof(command->lineStipple);
				break;

			case VPMT_OpcodeLineWidth:
				offset += sizeof(command->lineWidth);
				break;

			case VPMT_OpcodeListBase:
				offset += sizeof(command->listBase);
				break;

			case VPMT_OpcodeLoadIdentity:
				offset += sizeof(command->base);
				break;

			case VPMT_OpcodeLoadMatrix:
				offset += sizeof(command->loadMatrix);
				break;

			case VPMT_OpcodeMaterialfv:
				offset += sizeof(command->materialfv);
				break;

			case VPMT_OpcodeMatrixMode:
				offset += sizeof(command->matrixMode);
				break;

			case VPMT_OpcodeMultMatrix:
				offset += sizeof(command->multMatrix);
				break;

			case VPMT_OpcodeMultiTexCoord:
				offset += sizeof(command->multiTexCoord);
				break;

			case VPMT_OpcodeNormal:
				offset += sizeof(command->normal);
				break;

			case VPMT_OpcodePointSize:
				offset += sizeof(command->pointSize);
				break;

			case VPMT_OpcodePolygonOffset:
				offset += sizeof(command->polygonOffset);
				break;

			case VPMT_OpcodePolygonStipple:
				offset += sizeof(command->polygonStipple);
				break;

			case VPMT_OpcodePopMatrix:
				offset += sizeof(command->base);
				break;

			case VPMT_OpcodePushMatrix:
				offset += sizeof(command->base);
				break;

			case VPMT_OpcodeRasterPos:
				offset += sizeof(command->rasterPos);
				break;

			case VPMT_OpcodeScissor:
				offset += sizeof(command->scissor);
				break;

			case VPMT_OpcodeShadeModel:
				offset += sizeof(command->shadeModel);
				break;

			case VPMT_OpcodeStencilFunc:
				offset += sizeof(command->stencilFunc);
				break;

			case VPMT_OpcodeStencilMask:
				offset += sizeof(command->stencilMask);
				break;

			case VPMT_OpcodeStencilOp:
				offset += sizeof(command->stencilOp);
				break;

			case VPMT_OpcodeTexEnvfv:
				offset += sizeof(command->texEnvfv);
				break;

			case VPMT_OpcodeTexEnvi:
				offset += sizeof(command->texEnvi);
				break;

			case VPMT_OpcodeTexImage2D:
				offset += sizeof(command->texImage2D);
				VPMT_Image2DDeallocate(command->texImage2D.image);
				break;

			case VPMT_OpcodeTexParameteri:
				offset += sizeof(command->texParameteri);
				break;

			case VPMT_OpcodeTexSubImage2D:
				offset += sizeof(command->texSubImage2D);
				VPMT_Image2DDeallocate(command->texSubImage2D.image);
				break;

			case VPMT_OpcodeToggle:
				offset += sizeof(command->toggle);
				break;

			case VPMT_OpcodeVertex:
				offset += sizeof(command->vertex);
				break;

			case VPMT_OpcodeViewport:
				offset += sizeof(command->viewport);
				break;

#if GL_EXT_paletted_texture
			case VPMT_OpcodeColorSubTable:
				offset += sizeof(command->colorSubTable);
				VPMT_Image1DDeallocate(command->colorSubTable.palette);

				break;

			case VPMT_OpcodeColorTable:
				offset += sizeof(command->colorTable);
				VPMT_Image1DDeallocate(command->colorTable.palette);
				break;
#endif
			default:
				/* corrupted list: terminate traversal */
				assert(GL_FALSE);
				return;
			}
		}
	}
}

void VPMT_CommandBufferDispose(VPMT_CommandBuffer * commands)
{
	CleanupList(commands);

	while (commands) {
		VPMT_CommandBuffer *next = commands->next;

		VPMT_FREE(commands);
		commands = next;
	}
}

void VPMT_ExecDeleteLists (VPMT_Context * context, GLuint list, GLsizei range)
{
	VPMT_NOT_RENDERING(context);

	while (range-- > 0) {
		VPMT_CommandBuffer * commands = VPMT_HashTableFind(&context->lists, list);

		if (commands) {
			VPMT_CommandBufferDispose(commands);
			VPMT_HashTableRemove(&context->lists, list);
		}

		++list;
	}
}

VPMT_Dispatch VPMT_DispatchRecord = {
	&VPMT_RecActiveTexture,
	&VPMT_RecAlphaFunc,
	&VPMT_RecBegin,
	&VPMT_RecBindTexture,
	&VPMT_RecBitmap,
	&VPMT_RecBlendFunc,
	&VPMT_RecCallLists,
	&VPMT_RecClear,
	&VPMT_RecClearColor,
	&VPMT_RecClearDepthf,
	&VPMT_RecClearStencil,
	&VPMT_ExecClientActiveTexture,
	&VPMT_RecColor4f,
	&VPMT_RecColor4fv,
	&VPMT_RecColor4ub,
	&VPMT_RecColorMask,
	&VPMT_ExecColorPointer,
	&VPMT_RecCopyPixels,
	&VPMT_RecCullFace,
	&VPMT_ExecDeleteLists,
	&VPMT_ExecDeleteTextures,
	&VPMT_RecDepthFunc,
	&VPMT_RecDepthMask,
	&VPMT_RecDepthRangef,
	&VPMT_RecDisable,
	&VPMT_ExecDisableClientState,
	&VPMT_ExecDrawArrays,
	&VPMT_ExecDrawElements,
	&VPMT_RecDrawPixels,
	&VPMT_RecEnable,
	&VPMT_ExecEnableClientState,
	&VPMT_RecEnd,
	&VPMT_ExecEndList,
	&VPMT_ExecFinish,
	&VPMT_ExecFlush,
	&VPMT_RecFrontFace,
	&VPMT_RecFrustumf,
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
	&VPMT_RecHint,
	&VPMT_ExecIsEnabled,
	&VPMT_RecLightfv,
	&VPMT_RecLightModelf,
	&VPMT_RecLightModelfv,
	&VPMT_RecLineStipple,
	&VPMT_RecLineWidth,
	&VPMT_RecListBase,
	&VPMT_RecLoadIdentity,
	&VPMT_RecLoadMatrixf,
	&VPMT_RecMaterialf,
	&VPMT_RecMaterialfv,
	&VPMT_RecMatrixMode,
	&VPMT_RecMultMatrixf,
	&VPMT_RecMultiTexCoord2f,
	&VPMT_ExecNewList,
	&VPMT_RecNormal3f,
	&VPMT_RecNormal3fv,
	&VPMT_ExecNormalPointer,
	&VPMT_RecOrthof,
	&VPMT_ExecPixelStorei,
	&VPMT_RecPointSize,
	&VPMT_RecPolygonOffset,
	&VPMT_RecPolygonStipple,
	&VPMT_RecPopMatrix,
	&VPMT_RecPushMatrix,
	&VPMT_RecRasterPos3f,
	&VPMT_ExecReadPixels,
	&VPMT_RecRotatef,
	&VPMT_RecScalef,
	&VPMT_RecScissor,
	&VPMT_RecShadeModel,
	&VPMT_RecStencilFunc,
	&VPMT_RecStencilMask,
	&VPMT_RecStencilOp,
	&VPMT_ExecTexCoordPointer,
	&VPMT_RecTexEnvfv,
	&VPMT_RecTexEnvi,
	&VPMT_RecTexImage2D,
	&VPMT_RecTexParameteri,
	&VPMT_RecTexSubImage2D,
	&VPMT_RecTranslatef,
	&VPMT_RecVertex2f,
	&VPMT_RecVertex2fv,
	&VPMT_RecVertex3f,
	&VPMT_RecVertex3fv,
	&VPMT_ExecVertexPointer,
	&VPMT_RecViewport,

#if GL_EXT_paletted_texture
	&VPMT_RecColorSubTable,
	&VPMT_RecColorTable,
	&VPMT_ExecGetColorTable,
	&VPMT_ExecGetColorTableParameteriv,
#endif
};

/* $Id: list.c 74 2008-11-23 07:25:12Z hmwill $ */
