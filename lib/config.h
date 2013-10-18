/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Configuration parameters
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_CONFIG_H
#define VPMT_CONFIG_H

/*
** -------------------------------------------------------------------------
** Version information
** -------------------------------------------------------------------------
*/

#define VPMT_VENDOR			"Vincent Pervasive Media Technologies, LLC."
#define VPMT_RENDERER		"Vincent SC 1.0 Rendering Library"
#define VPMT_VERSION		"OpenGL ES-SC 1.0"

#define VPMT_SC_RELAX										   /* relax checks to pass conformance test */

/*
** -------------------------------------------------------------------------
** Implementation limits
** -------------------------------------------------------------------------
*/

#define VPMT_MAX_VIEWPORT_WIDTH				2048			   /* max. viewport width      */
#define VPMT_MAX_VIEWPORT_HEIGHT			2048			   /* max. viewport height     */
#define VPMT_VIEWPORT_COORD_BITS			12				   /* number of bits to index  */

#define VPMT_MAX_TEX_UNITS					2				   /* max. number of tex units */
#define VPMT_MAX_LIGHTS						2				   /* max. number of lights    */
#define VPMT_MAX_TEXTURES					64				   /* max. number of textures  */
#define VPMT_MAX_MIPMAP_LEVEL				11				   /* max. number of mipmaps   */
#define VPMT_MAX_TEXTURE_SIZE		(1 << (VPMT_MAX_MIPMAP_LEVEL-1))

#define VPMT_MAX_ELEMENTS_INDICES			16				   /* arbitrary                */
#define VPMT_MAX_ELEMENTS_VERTICES			16				   /* arbitrary                */

#define VPMT_MODELVIEW_STACK_DEPTH			32				   /* max. matrix stack depth  */
#define VPMT_PROJECTION_STACK_DEPTH			2				   /* max. matrix stack depth  */
#define VPMT_SUBPIXEL_BITS					4				   /* log2 sub-divisions of pixel  */

#define VPMT_MAX_POINT_SIZE					16.0f			   /* maximum point size       */
#define VPMT_SMOOTH_POINT_SIZE_GRANULARITY	(1.0f/(1 << VPMT_SUBPIXEL_BITS))	/* granularity */
#define VPMT_MIN_SMOOTH_POINT_SIZE			VPMT_SMOOTH_POINT_SIZE_GRANULARITY	/* minimum smooth point size */
#define VPMT_MAX_SMOOTH_POINT_SIZE			VPMT_MAX_POINT_SIZE	/* maximum smooth point size */

#define VPMT_MAX_LINE_WIDTH					16.0f			   /* maximum line width       */
#define VPMT_SMOOTH_LINE_WIDTH_GRANULARITY	1.0f			   /* granularity              */
#define VPMT_MIN_SMOOTH_LINE_WIDTH			VPMT_SMOOTH_LINE_WIDTH_GRANULARITY	/* minimum smooth line width */
#define VPMT_MAX_SMOOTH_LINE_WIDTH			VPMT_MAX_LINE_WIDTH	/* maximum smooth line width */

/*
** -------------------------------------------------------------------------
** Other configuration parameters
** -------------------------------------------------------------------------
*/

#define VPMT_HASH_SIZE						257				   /* Hash table size          */
#define VPMT_PACK_ALIGNMENT					4				   /* internal alignment       */
#define VPMT_COMMAND_BUFFER_SIZE			512				   /* Display list increment   */
#define VPMT_MAX_RENDER_BUFFERS				2				   /* maximum number of buffers attached to framebuffer */

#define VPMT_FUNCTION_CACHE_ENRIES			128				   /* number of cached functions */
#define VPMT_FUNCTION_CACHE_SIZE			65536			   /* code cache size */
#define VPMT_FUNCTION_STATE_SIZE_INIT		1024			   /* initial state buffer */
#define VPMT_FUNCTION_CODE_SIZE_INIT		4096			   /* initial code buffer */

/*
** -------------------------------------------------------------------------
** Platform constants
** -------------------------------------------------------------------------
*/

#define VPMT_LITTLE_ENDIAN					1				   /* low byte first           */
#define VPMT_BITS_PER_BYTE					8				   /* bits per byte            */
#define VPMT_FLOAT_MASTISSA_BITS			23				   /* bits per mantissa        */

#define VPMT_BYTE_MIN						SCHAR_MIN		   /* minimum value for GLbyte     */
#define VPMT_BYTE_MAX						SCHAR_MAX		   /* maximum value for GLbyte     */
#define VPMT_UBYTE_MAX						UCHAR_MAX		   /* maximum value for GLubyte    */
#define VPMT_SHORT_MIN						SHRT_MIN		   /* minimum value for GLshort    */
#define VPMT_SHORT_MAX						SHRT_MAX		   /* maximum value for GLshort    */
#define VPMT_USHORT_MAX						USHRT_MAX		   /* maximum value for GLushort   */
#define VPMT_INT_MIN						INT_MIN			   /* minimum value for GLint      */
#define VPMT_INT_MAX						INT_MAX			   /* maximum value for GLint      */
#define VPMT_UINT_MAX						UINT_MAX		   /* maximum value for GLuint     */

#endif

/* $Id: config.h 74 2008-11-23 07:25:12Z hmwill $ */
