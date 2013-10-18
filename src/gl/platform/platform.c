/*
** ==========================================================================
**
** $Id: platform.c 60 2007-09-18 01:16:07Z hmwill $
**
** Platform bindings
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*
** --------------------------------------------------------------------------
** String and Memory Functions
** --------------------------------------------------------------------------
*/

void GlesMemset(void * ptr, GLubyte value, GLsizeiptr size)
{
	memset(ptr, value, size);
}

void GlesMemcpy(void * dst, const void * src, GLsizeiptr size) {
	memcpy(dst, src, size);
}

GLint GlesMemcmp(const char * first, const char * second, GLsizeiptr length) {
	return memcmp(first, second, length);
}

void GlesMemmove(void * dst, const void * src, GLsizeiptr size) {
	memmove(dst, src, size);
}

char * GlesStrchr(const char *s, char c) {
	return strchr(s, c);
}

GLsizei GlesStrlen(const char * string) {
	return strlen(string);
}

GLint GlesStrcmp(const char * first, const char * second) {
	return strcmp(first, second);
}

GLint GlesStrncmp(const char * first, const char * second, GLsizeiptr length) {
	return strncmp(first, second, length);
}

GLsizei GlesSprintf(char * target, const char * format, ...) {
	va_list args;
	GLsizei result;
	
	va_start(args, format);
	result = vsprintf(target, format, args);
	va_end(args);
	
	return result;
}

/*
** --------------------------------------------------------------------------
** Memory Management
** --------------------------------------------------------------------------
*/

void * GlesMalloc(GLsizeiptr size) {
	void * result = malloc(size);

	if (result) {
		GlesMemset(result, 0, size);
	}

	return result;
}

void GlesFree(void * ptr) {
	free(ptr);
}

/*
** --------------------------------------------------------------------------
** Long Jumps
** --------------------------------------------------------------------------
*/

/**
 * The GlesSetjmp() function saves their calling environment in env.  
 * 
 * @param env	where to store the calling environment
 * 
 * @return 0
 */
GLint GlesSetjmp(JumpBuffer env) {
	return setjmp(env);
}

/**
 * This function restores the environment saved by their most recent 
 * respective invocations of the GlesSetjmp() function.
 *
 * @param env	teh environment to restore
 * @param val	the value to return when returning to the specified environment
 */
void GlesLongjmp(JumpBuffer env, GLint val) {
	longjmp(env, val);
}
 
/*
** --------------------------------------------------------------------------
** Math functions
** --------------------------------------------------------------------------
*/

/**
 * Approximate the logarithm of 2 of value.
 * 
 * The original code was described in http://www.flipcode.com/totd/
 * 
 * @param value
 * 		the value whose logarithm should be computed
 * @return
 * 		an approximation of the log2 of value
 */
GLfloat GlesLog2f(GLfloat value) {
	union { 
		GLfloat f; 
		GLint 	i; 
	} numberbits;
	
   GLint log_2;
   
   numberbits.f = value;
   log_2 = ((numberbits.i >> GLES_FLOAT_MASTISSA_BITS) & GLES_UBYTE_MAX) - 128;
   numberbits.i &= ~(GLES_UBYTE_MAX << GLES_FLOAT_MASTISSA_BITS);
   numberbits.i += 127 << GLES_FLOAT_MASTISSA_BITS;
   numberbits.f = ((-1.0f/3) * numberbits.f + 2) * numberbits.f - 2.0f/3;
   
   return numberbits.f + log_2;
}

