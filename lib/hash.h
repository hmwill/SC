/*
** -------------------------------------------------------------------------
** Vincent SC 1.0 Rendering Library
**
** Hash table implementation for list management.
**
** Copyright (C) 2008 Vincent Pervasive Media Technologies, LLC. 
**
** Licensed under the Artistic License 2.0.
** -------------------------------------------------------------------------
*/

#ifndef VPMT_HASH_H
#define VPMT_HASH_H

typedef struct VPMT_HashEntry {
	struct VPMT_HashEntry *next;
	GLuint key;
	void *value;
} VPMT_HashEntry;

typedef struct VPMT_HashTable {
	VPMT_HashEntry *buckets[VPMT_HASH_SIZE];
	GLuint maxKey;
} VPMT_HashTable;

typedef void (*VPMT_HashTableIterFunc)(GLuint key, void * value, void * arg);

void VPMT_HashTableInitialize(VPMT_HashTable * table);
void VPMT_HashTableDeinitialize(VPMT_HashTable * table);
void *VPMT_HashTableFind(VPMT_HashTable * table, GLuint key);
void VPMT_HashTableIterate(VPMT_HashTable * table, VPMT_HashTableIterFunc func, void * arg);
void VPMT_HashTableRemove(VPMT_HashTable * table, GLuint key);
GLboolean VPMT_HashTableInsert(VPMT_HashTable * table, GLuint key, void *value);
GLuint VPMT_HashTableFreeKeyBlock(VPMT_HashTable * table, GLsizei numKeys);

#endif

/* $Id: hash.h 74 2008-11-23 07:25:12Z hmwill $ */
