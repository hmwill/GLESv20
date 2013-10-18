/*
** ==========================================================================
**
** $Id: memory.c 60 2007-09-18 01:16:07Z hmwill $
**
** Shading Language Memory Management
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-17 18:16:07 -0700 (Mon, 17 Sep 2007) $
**
** --------------------------------------------------------------------------
**
** Vincent 3D Rendering Library, Programmable Pipeline Edition
** 
** Copyright (C) 2003-2007 Hans-Martin Will. 
**
** @CDDL_HEADER_START@
**
** The contents of this file are subject to the terms of the
** Common Development and Distribution License, Version 1.0 only
** (the "License").  You may not use this file except in compliance
** with the License.
**
** You can obtain a copy of the license at 
** http://www.vincent3d.com/software/ogles2/license/license.html
** See the License for the specific language governing permissions
** and limitations under the License.
**
** When distributing Covered Code, include this CDDL_HEADER in each
** file and include the License file named LICENSE.TXT in the root folder
** of your distribution.
** If applicable, add the following below this CDDL_HEADER, with the
** fields enclosed by brackets "[]" replaced with your own identifying
** information: Portions Copyright [yyyy] [name of copyright owner]
**
** @CDDL_HEADER_END@
**
** ==========================================================================
*/

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "frontend/memory.h"


/*
** --------------------------------------------------------------------------
** Module local data
** --------------------------------------------------------------------------
*/

#define HEAP_ALIGNMENT	8

typedef struct MemoryPage {
	struct MemoryPage * 	next;			/* next page in pool		*/
	GLubyte * 				base;			/* memory base address		*/
	GLsizeiptr 				total;			/* total memory in page		*/
} MemoryPage;


struct MemoryPool {
	JumpBuffer * 		handler;			/* exception handler		*/
	MemoryPage *		pages;				/* list of pages			*/
	GLsizeiptr 			defaultPageSize;	/* standard page size		*/
	GLsizeiptr 			current;			/* used memory in page		*/
};

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static MemoryPage * CreatePage(GLsizeiptr pageSize) {

	MemoryPage * result = (MemoryPage *) GlesMalloc(sizeof(MemoryPage));
	
	if (result) {
		result->base = (GLubyte *) GlesMalloc(pageSize);
		result->total = pageSize;
		result->next = (MemoryPage *) 0;
	}
	
	return result;

}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/


/**
 * Create a new memory block with the specified default size for pages.
 * 
 * @param defaultPageSize
 * 		the default size for memory pages created by this pool
 * @param handler
 * 		if handler is != NULL, the handler will be invoked using GlesLongjmp whenever
 * 		a memory allocation fails
 * @return
 * 		a newly created memory pool
 */
MemoryPool * GlesMemoryPoolCreate(GLsizeiptr defaultPageSize, JumpBuffer * handler) {
	MemoryPool * pool = (MemoryPool *) GlesMalloc(sizeof(MemoryPool));
	
	if (pool) {
		pool->handler = handler;
		pool->defaultPageSize = defaultPageSize;
		pool->current = 0;
		pool->pages = CreatePage(defaultPageSize);
		
		if (!pool->pages) {
			GlesFree(pool);
			return NULL;
		}
	}
		
	return pool;
}

/**
 * Set the allocation error handler for the given pool.
 * 
 * @param pool
 * 		the pool whose handler to set
 * @param handler
 * 		reference to the environment that handles allocation errors
 */
void GlesMemoryPoolHandler(MemoryPool * pool, JumpBuffer * handler) {
	pool->handler = handler;
}

/**
 * Destroy the given memory pool deallocating any associated memory.
 *
 * @param pool
 * 		the pool to destroy
 */
void GlesMemoryPoolDestroy(MemoryPool * pool) {
	MemoryPage * current, *next;

	for (current = pool->pages; current != (MemoryPage *) 0; current = next) {
		next = current->next;
		GlesFree(current->base);
		GlesFree(current);
	}

	GlesFree(pool);
}

/**
 * Clear the given memory pool and reset to initial size.
 *
 * @param pool
 * 		the pool to destroy
 */
void GlesMemoryPoolClear(MemoryPool * pool) {
	MemoryPage * current, *next;

	for (current = pool->pages; current != (MemoryPage *) 0; current = next) {
		next = current->next;
		GlesFree(current->base);
		GlesFree(current);
	}
	
	pool->pages = NULL;
	pool->current = 0;
}

/**
 * Allocate the given amount of memory within the specified pool.
 * 
 * @param pool
 * 		the pool in which to allocate the memory
 * @param amount
 * 		the amount of memory to allocate
 */
void * GlesMemoryPoolAllocate(MemoryPool * pool, GLsizeiptr amount) {
	void * result;

	amount = (amount + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);

	if (!pool->pages || pool->pages->total - pool->current < amount) {
		MemoryPage * newPage;

		if (amount > pool->defaultPageSize) { 
			newPage = CreatePage(amount);
		} else {
			newPage = CreatePage(pool->defaultPageSize);
		}

		if (!newPage) {
			if (pool->handler) {
				GlesLongjmp(*pool->handler, GL_TRUE);
			} else {
				return NULL;
			}
		}
		
		newPage->next = pool->pages;
		pool->pages = newPage;
		pool->current = 0;
	}

	result = pool->pages->base + pool->current;
	pool->current += amount;

	return result;
}
