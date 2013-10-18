/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** System-specific Declarations and Definitions
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_SYSTEM_H
#define VPMT_SYSTEM_H


void VPMT_ClearInstructionCache(void * start, GLsizei size);

void * VPMT_AllocateExecutableSegment(GLsizei size);
void VPMT_DeallocateExecutableSegment(void * ptr, GLsizei size);

#endif

