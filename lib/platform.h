/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Platform-specific Declarations and Definitions
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_PLATFORM_H
#define VPMT_PLATFORM_H

#include <stddef.h>

/*
** -------------------------------------------------------------------------
** Not all platform compilers support inline functions
** -------------------------------------------------------------------------
*/

#if defined(_MSC_VER) || defined(__GNUC__)
#	define VPMT_INLINE	__inline
#else
#	define VPMT_INLINE
#endif

/*
** -------------------------------------------------------------------------
** Not all platform runtimes support single precision math functions
** -------------------------------------------------------------------------
*/

#if defined(_WIN32_WCE)
#	define VPMT_EMULATE_FLOAT_MATH
#endif

/*
** -------------------------------------------------------------------------
** Provide memory management debug helper functions
** -------------------------------------------------------------------------
*/

typedef size_t VPMT_Size_t;

void *VPMT_Malloc(VPMT_Size_t size);
void *VPMT_ReAlloc(void * ptr, VPMT_Size_t size);
void VPMT_Free(void *ptr);

void *VPMT_MallocDebug(VPMT_Size_t size, const char *filename, int line);
void *VPMT_ReAllocDebug(void * ptr, VPMT_Size_t size, const char *filename, int line);
void VPMT_FreeDebug(void *ptr, const char *filename, int line);

#if defined(VPMT_MEM_DEBUG)
#	define VPMT_MALLOC(size) VPMT_MallocDebug(size, __FILE__, __LINE__)
	#define VPMT_REALLOC(ptr, size) VPMT_ReAllocDebug(ptr, size, __FILE__, __LINE__)
#	define VPMT_FREE(ptr) VPMT_FreeDebug(ptr, __FILE__, __LINE__)
#else
#	define VPMT_MALLOC(size) VPMT_Malloc(size)
#	define VPMT_REALLOC(ptr, size) VPMT_ReAlloc(ptr, size)
#	define VPMT_FREE(ptr) VPMT_Free(ptr)
#endif

#endif

/* $Id: platform.h 74 2008-11-23 07:25:12Z hmwill $ */
