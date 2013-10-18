/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** GL context structures
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_CONTEXT_H
#define VPMT_CONTEXT_H

#include "util.h"
#include "image.h"
#include "matrix.h"
#include "hash.h"
#include "tex.h"
#include "texunit.h"

typedef struct VPMT_Context VPMT_Context;
typedef struct VPMT_Array VPMT_Array;

typedef void (*ArrayFetchFunc) (const VPMT_Array * array, GLsizei index, GLfloat * result);

struct VPMT_Array {
	ArrayFetchFunc fetch;
	const void *pointer;
	GLint size;
	GLint stride;
	GLint effectiveStride;
	GLenum type;
	GLboolean enabled;
};

void VPMT_ArrayInitialize(VPMT_Array * array);

typedef struct VPMT_Pattern {
	GLubyte bytes[128];
} VPMT_Pattern;

struct VPMT_CommandBuffer;
struct VPMT_Context;

typedef struct VPMT_Vertex {
	VPMT_Vec4 vertex;
	VPMT_Vec4 rgba;
	VPMT_Vec2 texCoords[VPMT_MAX_TEX_UNITS];
	GLuint cc;												   /* culling mask */
} VPMT_Vertex;

typedef struct VPMT_RasterVertex {
	VPMT_Vec2i screenCoords;
	GLfloat invW;
	GLfloat depth;
	VPMT_Vec4 rgba;
	VPMT_Vec2 texCoords[VPMT_MAX_TEX_UNITS];
	GLfloat rho[VPMT_MAX_TEX_UNITS];
} VPMT_RasterVertex;

typedef void (*VPMT_VertexFunction) (struct VPMT_Context * context);
typedef void (*VPMT_EndFunction) (struct VPMT_Context * context);
typedef void (*VPMT_TransformFunction) (VPMT_Context * context, VPMT_Vertex * vertex);

/*
** --------------------------------------------------------------------------
** Rendering Surface
** --------------------------------------------------------------------------
*/

struct VPMT_Surface;

/**
 * Virtual function table for render surfaces
 */
typedef struct VPMT_SurfaceVtbl {
	/** increase the reference count for this surface */
	void (*addref) (struct VPMT_Surface * surface);

	/** decrease the reference count for this surface */
	void (*release) (struct VPMT_Surface * surface);

	/** 
	 * lock the surface in memory, that is, pin the surface location to
	 * a referencable memory address.
	 * 
	 * This function will also ensure that anhy memory pointers in
	 * the surface structure will point to valid addresses.
	 */
	void (*lock) (VPMT_Context * context, struct VPMT_Surface * surface);

	/**
	 * unlock the surface, that is, unpin the surface location.
	 * 
	 * After this call, the mamory addresses in the surface structure
	 * can no longer assumed to be valid.
	 */
	void (*unlock) (VPMT_Context * context, struct VPMT_Surface * surface);
} VPMT_SurfaceVtbl;

typedef struct VPMT_Surface {
	GLint refcount;
	const VPMT_SurfaceVtbl *vtbl;
	VPMT_Image2D image;
	GLvoid *depthStencilBuffer;
	const VPMT_DepthStencilFormat *depthStencilFormat;
} VPMT_Surface;

typedef void (*VPMT_RasterPointFunc) (VPMT_Context * context, const VPMT_RasterVertex * a);
typedef void (*VPMT_RasterLineFunc) (VPMT_Context * context, const VPMT_RasterVertex * a,
									 const VPMT_RasterVertex * b);
typedef void (*VPMT_RasterTriangleFunc) (VPMT_Context * context, const VPMT_RasterVertex * a,
										 const VPMT_RasterVertex * b, const VPMT_RasterVertex * c);

struct VPMT_Context {

	/* function dispatch table */
	const struct VPMT_Dispatch *dispatch;

	/* rendering status */
	GLenum renderMode;										   /* current primitive type in begin/end */

	/* current error */
	GLenum error;
	GLenum compileError;

	/* current vertex attributes */
	VPMT_Vec3 vertex;
	VPMT_Vec3 normal;
	VPMT_Vec4 color;
	VPMT_Vec4 texCoords[VPMT_MAX_TEX_UNITS];

	VPMT_RasterPointFunc rasterPoint;
	VPMT_RasterLineFunc rasterLine;
	VPMT_RasterTriangleFunc rasterTriangle;

	/* hints */
	GLenum perspectiveCorrectionHint;
	GLenum pointSmoothHint;
	GLenum lineSmoothHint;

	/* enable/disable context variables */
	GLboolean alphaTestEnabled;
	GLboolean blendEnabled;
	GLboolean colorMaterialEnabled;
	GLboolean depthTestEnabled;
	GLboolean lightingEnabled;
	GLboolean normalizeEnabled;
	GLboolean rescaleNormalEnabled;
	GLboolean stencilTestEnabled;

	GLboolean lightEnabled[VPMT_MAX_LIGHTS];

	GLboolean pointSmoothEnabled;
	GLboolean lineSmoothEnabled;
	GLboolean lineStippleEnabled;
	GLboolean cullFaceEnabled;
	GLboolean polygonOffsetFillEnabled;
	GLboolean polygonStippleEnabled;
	GLboolean scissorTestEnabled;

	VPMT_Vec4b colorWriteMask;
	GLboolean depthWriteMask;

	/* integer context variables */
	GLint clientActiveTexture;
	GLsizei clientActiveTextureIndex;
	GLint activeTexture;
	GLsizei activeTextureIndex;
	GLint matrixMode;
	GLint shadeModel;

	VPMT_Rect viewport;
	VPMT_Rect scissor;

	GLint lineStipplePattern;
	GLint lineStippleRepeat;

	GLint cullFaceMode;
	GLint frontFace;

	GLint stencilWriteMask;
	GLint packAlignment;
	GLint unpackAlignment;

	GLenum alphaFunc;
	GLenum depthFunc;
	GLenum blendSrcFactor, blendDstFactor;
	GLenum stencilFunc;
	GLenum stencilOpFail, stencilOpZFail, stencilOpZPass;
	GLint stencilMask, stencilRef;

	VPMT_TexImageUnit texUnits[VPMT_MAX_TEX_UNITS];
	GLuint texture2DEnabledMask;

	VPMT_HashTable textures;

	/* floating point context variables */
	VPMT_Matrix modelviewMatrix[VPMT_MODELVIEW_STACK_DEPTH];
	VPMT_Matrix projectionMatrix[VPMT_PROJECTION_STACK_DEPTH];
	VPMT_MatrixStack modelviewMatrixStack;
	VPMT_MatrixStack projectionMatrixStack;
	VPMT_MatrixStack *currentMatrixStack;
	VPMT_Matrix inverseModelView;							   /* for normal transformation */

	VPMT_Vec4 materialAmbient;
	VPMT_Vec4 materialDiffuse;
	VPMT_Vec4 materialEmission;
	VPMT_Vec4 materialSpecular;
	GLfloat materialShininess;

	VPMT_Vec4 lightModelAmbient;							   /* ambient light            */

	VPMT_Vec4 lightAmbient[VPMT_MAX_LIGHTS];
	VPMT_Vec4 lightDiffuse[VPMT_MAX_LIGHTS];
	VPMT_Vec4 lightSpecular[VPMT_MAX_LIGHTS];
	VPMT_Vec4 lightPosition[VPMT_MAX_LIGHTS];
	VPMT_Vec4 lightDirection[VPMT_MAX_LIGHTS];				   /* derived state, normalized direction */
	VPMT_Vec4 lightHighlightDirection[VPMT_MAX_LIGHTS];		   /* derived state, normalized highlight direction */

	GLfloat depthRange[2];									   /* depth range */
	GLfloat depthScale, depthOffset;
	GLfloat pointSize;
	GLfloat lineWidth;
	GLfloat polygonOffsetFactor;
	GLfloat polygonOffsetUnit;

	GLfloat alphaRef;

	/* arrays */
	VPMT_Array colorArray;
	VPMT_Array normalArray;
	VPMT_Array texCoordArray[VPMT_MAX_TEX_UNITS];
	VPMT_Array vertexArray;

	/* polygon stipple pattern */
	VPMT_Pattern polygonStipple;

	/* whole framebuffer context */
	VPMT_Vec4 clearColor;
	GLfloat clearDepth;
	GLint clearStencil;

	/* framebuffer configuration */
	GLint redBits;
	GLint greenBits;
	GLint blueBits;
	GLint alphaBits;
	GLint depthBits;
	GLint stencilBits;
	GLuint maxDepthValue;
	GLuint maxStencilValue;

	/* display list status */
	VPMT_HashTable lists;									   /* table of all lists currently defined */

	struct VPMT_CommandBuffer *listCommandsHead;
	struct VPMT_CommandBuffer *listCommandsTail;			   /* lists currently being compiled */
	GLint listBase;
	GLuint listIndex;										   /* name of currently compiled list */
	GLenum listMode;										   /* can be 0 if not compiling */

	VPMT_Vertex vertexQueue[4];								   /* array of vertices during begin/end */
	VPMT_Vertex tempVertices[12];							   /* temp. vertices for cipping */
	VPMT_VertexFunction vertexFunction;
	VPMT_EndFunction endFunction;
	VPMT_TransformFunction transformFunction;				   /* vertex transformation */
	GLuint nextIndex;										   /* next index in queue to fill */
	GLuint primitiveState;									   /* state flag for primitive */
	GLenum primitiveType;									   /* primitive type being rendered */
	GLint integerLineWidth;									   /* integer line width for rasterizer */
	GLint integerPointSize;									   /* integer point size for rasterizer */
	GLint aaPointSize;										   /* point size in sub-pixels for AA */

	GLboolean rasterPosValid;								   /* is the raster position valid? */
	VPMT_RasterVertex rasterPos;
	VPMT_Vec4 rasterTexCoords[VPMT_MAX_TEX_UNITS];

	VPMT_Rectf viewportTransform;							   /* ciewport transformation values */

	GLsizei lineStipplePatternIndex;						   /* index int stipple pattern */
	GLint lineStippleCounter;								   /* repeat counter */

	VPMT_Rect activeSurfaceRect;							   /* intersection of scissor & surface rect */
	VPMT_Surface *readSurface;								   /* surface to read from */
	VPMT_Surface *writeSurface;								   /* surface to write to */
	GLfloat depthFixedPointScale;							   /* factor to convert FP depth value to fixed */

	/* rasterizer execution flags */
	GLuint rasterInterpolants;								   /* which vairables to interpolate */
	GLubyte alphaRefub;										   /* alpha reference as unsigned byte */
};

/* exported functions to initialize the library */
GLAPI GLboolean APIENTRY VPMT_Initialize(void);
GLAPI void APIENTRY VPMT_Terminate(void);

/* context management */
GLboolean VPMT_ContextInitialize(VPMT_Context * context);
void VPMT_ContextDeinitialize(VPMT_Context * context);

extern VPMT_Context VPMT_GlobalContext;
#define VPMT_CONTEXT() (&VPMT_GlobalContext)

/* exported functions to set and get surfaces */
GLAPI GLboolean APIENTRY VPMT_MakeCurrent(VPMT_Surface * draw, VPMT_Surface * read);
GLAPI VPMT_Surface *APIENTRY VPMT_GetReadSurface(void);
GLAPI VPMT_Surface *APIENTRY VPMT_GetWriteSurface(void);

/* macros for common diagnostics */
extern void VPMT_SetError(VPMT_Context * context, GLenum code);
extern void VPMT_SetOutOfMemoryError(VPMT_Context * context);
extern void VPMT_SetInvalidEnumError(VPMT_Context * context);
extern void VPMT_SetInvalidModeError(VPMT_Context * context);
extern void VPMT_SetInvalidValueError(VPMT_Context * context);
extern void VPMT_SetInvalidOperationError(VPMT_Context * context);

#define VPMT_OUT_OF_MEMORY(context) VPMT_SetOutOfMemoryError(context)
#define VPMT_INVALID_ENUM(context) VPMT_SetInvalidEnumError(context)
#define VPMT_INVALID_MODE(context) VPMT_SetInvalidModeError(context)
#define VPMT_INVALID_VALUE(context) VPMT_SetInvalidValueError(context)
#define VPMT_INVALID_OPERATION(context) VPMT_SetInvalidOperationError(context)
#define VPMT_STACK_UNDERFLOW(context) VPMT_SetError(context, GL_STACK_UNDERFLOW)
#define VPMT_STACK_OVERFLOW(context) VPMT_SetError(context, GL_STACK_OVERFLOW)

#define VPMT_NOT_RENDERING(context) \
	do { if (context->renderMode != GL_INVALID_MODE) { VPMT_INVALID_OPERATION(context); return; } } while(GL_FALSE)

#define VPMT_NOT_RENDERING_RETURN(context, value) \
	do { if (context->renderMode != GL_INVALID_MODE) { VPMT_INVALID_OPERATION(context); return (value); } } while(GL_FALSE)

/* misc. global declarations */
extern void VPMT_Bitblt(const VPMT_Image2D * dst, const VPMT_Rect * dstRect,
						const VPMT_Image2D * src, const GLint * srcPos);

void VPMT_LightVertex(VPMT_Context * context, GLfloat * resultColor,
					  const GLfloat * eyeCoords, const GLfloat * eyeNormal,
					  const GLfloat * vertexColor);

void VPMT_Fragment(VPMT_Context * context, struct VPMT_FrameBuffer *fb, VPMT_Color4ub newColor,
				   GLuint depth);

void VPMT_UpdateActiveSurfaceRect(VPMT_Context * context, const VPMT_Rect * rect);


#endif

/* $Id: context.h 74 2008-11-23 07:25:12Z hmwill $ */
