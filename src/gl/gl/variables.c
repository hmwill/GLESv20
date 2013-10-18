/*
** ==========================================================================
**
** $Id: variables.c 67 2007-09-25 05:51:44Z hmwill $
**
** Helper functions to work with program variables
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-24 22:51:44 -0700 (Mon, 24 Sep 2007) $
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
#include "gl/state.h"


/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

/**
 * Compare two variable names alphabetically.
 *
 * @param	name1	pointer to first name to compare
 * @param	len1	length of first name to compare
 * @param	name2	pointer to second name to compare
 * @param	len2	length of second name to compare
 *
 * @return	value == 0 if both strings compare equal, <0 if first string
 *			compares less, >0 if first string compares higher
 */
static GLES_INLINE GLint CompareNames(const char * name1, GLsizeiptr len1,
									  const char * name2, GLsizeiptr len2) {
	GLsizeiptr len = len1 < len2 ? len1 : len2;				
	
	while (len-- > 0) {
		GLint diff = (GLint) *name1++ - (GLint) *name2++;
		
		if (diff) {
			return diff;
		}
	}
	
	if (len1 == len2) {
		return 0;
	} else if (len1 < len2) {
		return -1;
	} else {
		return 1;
	}
}

/*
** --------------------------------------------------------------------------
** Public functions
** --------------------------------------------------------------------------
*/

/**
 * Restore the heap structure
 */
static void SiftDown(ShaderVariable * first, GLsizeiptr start, GLsizeiptr end) {
	GLsizeiptr root, child;
	
	root = start;
	
	while (root * 2 + 1 <= end) {
		child = root * 2 + 1;
		
		if (child < end && 
			CompareNames(first[child].name, first[child].length, 
						 first[child + 1].name, first[child + 1].length) < 0) {
			++child;
		}
		
		if (CompareNames(first[root].name, first[root].length,
		 				 first[child].name, first[child].length) < 0) {
			ShaderVariable temp;
			
			temp = first[root];
			first[root] = first[child];
			first[child] = temp;
			
			root = child;
		} else {
			return;
		}
	}
}

/**
 * Convert the shader variable array into a heap structure.
 *
 * @param	first	pointer to the first element of the array
 * @param	count	number of array elements
 */
static void Heapify (ShaderVariable * first, GLsizeiptr count) {
	GLsizeiptr start;
	
	for (start = count / 2 - 1; start >= 0; --start)
	        SiftDown(first, start, count - 1);
}

/**
 * Sort an array of shader variable structures alphabetically.
 *
 * @param	first	the first element of the array to sort
 * @param	count	the number of elements in the array
 */
void GlesSortShaderVariables(ShaderVariable * first, GLsizeiptr count) {
    GLsizeiptr end;
    ShaderVariable temp;

	/* Build the heap structure */
    Heapify(first, count);

    end = count - 1;

    while (end > 0) {
		temp = first[0];
		first[0] = first[end];
		first[end] = temp;
		
		SiftDown(first, 0, --end);
    }
}

/**
 * Find a specific shader variable in an array of sorted variables using
 * binary search.
 *
 * @param	first	the first element of the array to search
 * @param	count	the number of elements in the array to search
 * @param	name	pointer to the variable name to search
 * @param	length	the length of the variable name to search
 */
ShaderVariable * GlesFindShaderVariable(ShaderVariable * first, GLsizeiptr count,
	const char * name, GLsizeiptr length) {
		
	GLsizeiptr low = 0, high = count - 1;
		
	while (low < high) {
		GLsizeiptr mid = (low + high) / 2;
		
		if (CompareNames(first[mid].name, first[mid].length, name, length) < 0) {
			low = mid + 1; 
		} else {
			high = mid; 
		}
	}

	if (low < length && 
		!CompareNames(first[low].name, first[low].length, name, length)) {
		return first + low;
	} else {
		return NULL;
	}
}
