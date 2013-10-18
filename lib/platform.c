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

#include "common.h"
#include "GL/gl.h"

void *VPMT_Malloc(VPMT_Size_t size)
{
	return malloc(size);
}

void *VPMT_ReAlloc(void * ptr, VPMT_Size_t size)
{
	return realloc(ptr, size);
}

void VPMT_Free(void *ptr)
{
	free(ptr);
}

typedef struct DebugHeader {
	int magic;
	VPMT_Size_t size;
	const char *filename;
	int line;
} DebugHeader;

#define MAGIC 0x12345678

void *VPMT_MallocDebug(VPMT_Size_t size, const char *filename, int line)
{
	VPMT_Size_t total = size + sizeof(DebugHeader);
	void *node = malloc(total);

	if (node) {
		DebugHeader *header = node;

		header->magic = MAGIC;
		header->size = size;
		header->filename = filename;
		header->line = line;

		return header + 1;
	} else {
		return NULL;
	}
}

void *VPMT_ReAllocDebug(void * ptr, VPMT_Size_t size, const char *filename, int line)
{
	VPMT_Size_t total = size + sizeof(DebugHeader);
	void * node;

	if (ptr) {
		DebugHeader *header = ((DebugHeader *) ptr) - 1;

		if (header->magic != MAGIC) {
			fprintf(stderr, "VPMT_REALLOC of invalid node %p at line %d in %s\n", ptr, line, filename);
			return realloc(ptr, size);
		} else {
			fprintf(stderr, "VPMT_REALLOC of   valid node %p at line %d in %s. "
					"Allocation [size: %d, file: %s, line: %d]\n", ptr, line, filename,
					header->size, header->filename, header->line);
		}
	} else {
		fprintf(stderr, "VPMT_REALLOC of    NULL node at line %d in %s\n", line, filename);
		return VPMT_MallocDebug(size, filename, line);
	}

	node = realloc(((DebugHeader *) ptr) - 1, total);

	if (node) {
		DebugHeader *header = node;

		header->magic = MAGIC;
		header->size = size;
		header->filename = filename;
		header->line = line;

		return header + 1;
	} else {
		return NULL;
	}
}

void VPMT_FreeDebug(void *ptr, const char *filename, int line)
{
	if (ptr) {
		DebugHeader *header = ((DebugHeader *) ptr) - 1;

		if (header->magic != MAGIC) {
			fprintf(stderr, "VPMT_FREE of invalid node %p at line %d in %s\n", ptr, line, filename);
			free(ptr);
		} else {
			fprintf(stderr, "VPMT_FREE of   valid node %p at line %d in %s. "
					"Allocation [size: %d, file: %s, line: %d]\n", ptr, line, filename,
					header->size, header->filename, header->line);
			free(header);
		}
	} else {
		fprintf(stderr, "VPMT_FREE of    NULL node at line %d in %s\n", line, filename);
	}
}

/* $Id: platform.c 74 2008-11-23 07:25:12Z hmwill $ */
