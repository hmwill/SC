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

#include "common.h"
#include "GL/gl.h"
#include "context.h"

void VPMT_HashTableInitialize(VPMT_HashTable * table)
{
	memset(table, 0, sizeof *table);
}

void VPMT_HashTableDeinitialize(VPMT_HashTable * table)
{
	GLsizei index;

	for (index = 0; index < VPMT_HASH_SIZE; ++index) {
		VPMT_HashEntry *entry, *next;

		for (entry = table->buckets[index]; entry; entry = next) {
			next = entry->next;
			VPMT_FREE(entry);
		}
	}
}

void VPMT_HashTableIterate(VPMT_HashTable * table, VPMT_HashTableIterFunc func, void * arg)
{
	GLsizei index;

	for (index = 0; index < VPMT_HASH_SIZE; ++index) {
		VPMT_HashEntry *entry;

		for (entry = table->buckets[index]; entry; entry = entry->next) {
			func(entry->key, entry->value, arg);
		}
	}
}

void *VPMT_HashTableFind(VPMT_HashTable * table, GLuint key)
{

	if (key <= table->maxKey) {
		GLsizei index = key % VPMT_HASH_SIZE;
		VPMT_HashEntry *entry = table->buckets[index];

		while (entry) {
			if (entry->key == key) {
				return entry->value;
			}

			entry = entry->next;
		}
	}

	return NULL;
}

void VPMT_HashTableRemove(VPMT_HashTable * table, GLuint key)
{
	GLsizei index = key % VPMT_HASH_SIZE;
	VPMT_HashEntry **pentry = &table->buckets[index];

	while (*pentry) {
		if ((*pentry)->key == key) {
			VPMT_HashEntry *entry = *pentry;
			*pentry = (*pentry)->next;
			VPMT_FREE(entry);
			return;
		}

		pentry = &(*pentry)->next;
	}
}

GLboolean VPMT_HashTableInsert(VPMT_HashTable * table, GLuint key, void *value)
{
	GLsizei index = key % VPMT_HASH_SIZE;
	VPMT_HashEntry *entry;

	if (key <= table->maxKey) {
		entry = table->buckets[index];

		while (entry) {
			if (entry->key == key) {
				entry->value = value;

				if (table->maxKey < key) {
					table->maxKey = key;
				}

				return GL_TRUE;
			}

			entry = entry->next;
		}
	}

	entry = VPMT_MALLOC(sizeof(VPMT_HashEntry));

	if (!entry) {
		return GL_FALSE;
	}

	entry->next = table->buckets[index];
	entry->key = key;
	entry->value = value;
	table->buckets[index] = entry;

	if (table->maxKey < key) {
		table->maxKey = key;
	}

	return GL_TRUE;
}

GLuint VPMT_HashTableFreeKeyBlock(VPMT_HashTable * table, GLsizei numKeys)
{
	const GLuint maxKey = ~0u;

	/* 
	 ** This is an extremely simplistic implementation because OpenGL SC does not 
	 ** forsee release of resources.
	 */

	if (maxKey - numKeys > table->maxKey) {
		return table->maxKey + 1;
	} else {
		/* no block found */
		return 0;
	}
}

/* $Id: hash.c 74 2008-11-23 07:25:12Z hmwill $ */
