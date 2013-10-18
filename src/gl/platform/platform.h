#ifndef GLES_PLATFORM_PLATFORM_H
#define GLES_PLATFORM_PLATFORM_H 1

/*
** ==========================================================================
**
** $Id: platform.h 60 2007-09-18 01:16:07Z hmwill $
**
** Bindings to underlying OS platform
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


#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#define GLES_ASSERT(c) assert(c)

#ifdef _MSC_VER
#	define GLES_INLINE	__inline
#else /* GNU CC - what's the identifier? */
#	define GLES_INLINE  __inline__
#endif

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

#define GLES_NIL ((const void *) 4)		/* platform dependent invalid		*/
										/* pointer != NULL					*/

#define GLES_BYTE_MIN	SCHAR_MIN		/* minimum value for GLbyte			*/
#define GLES_BYTE_MAX	SCHAR_MAX		/* maximum value for GLbyte			*/
#define GLES_UBYTE_MAX	UCHAR_MAX		/* maximum value for GLubyte		*/
#define GLES_SHORT_MIN	SHRT_MIN		/* minimum value for GLshort		*/
#define GLES_SHORT_MAX	SHRT_MAX		/* maximum value for GLshort		*/
#define GLES_USHORT_MAX	USHRT_MAX		/* maximum value for GLushort		*/
#define GLES_INT_MIN	INT_MIN			/* minimum value for GLint			*/
#define GLES_INT_MAX	INT_MAX			/* maximum value for GLint			*/
#define GLES_UINT_MAX	UINT_MAX		/* maximum value for GLuint			*/

#define GLES_BITS_PER_BYTE	8			/* bits per byte					*/
#define GLES_FLOAT_MASTISSA_BITS	23	/* bits per float mantissa			*/

/*
** --------------------------------------------------------------------------
** Macros
** --------------------------------------------------------------------------
*/

#define GLES_OFFSETOF(T, member) ((GLsizeiptr)&(((T *)0)->member))
#define GLES_ELEMENTSOF(a)	(sizeof(a) / sizeof((a)[0]))
#define GLES_STRINGIFY(s)	#s
#define GLES_LIT(s)			s

/*
** --------------------------------------------------------------------------
** Data Types
** --------------------------------------------------------------------------
*/

typedef	jmp_buf JumpBuffer;

/*
** --------------------------------------------------------------------------
** Inline number conversion functions
** --------------------------------------------------------------------------
*/

/**
 * Clamp a float value to [0.0, 1.0]
 * 
 * @param value
 * 		the value to clamp
 * 
 * @return 
 * 		value clamped to the interval [0.0, 1.0]
 */
GLES_INLINE static GLclampf GlesClampf(GLfloat value) {
	return value > 1.0f ? 1.0f : value < 0.0f ? 0.0f : value;
}

/**
 * Convet a S15.16 fixed point value to float.
 * 
 * @param value
 * 		the value to convert
 * 
 * @return
 * 		the value as floating point number
 */
GLES_INLINE static GLfloat GlesFloatx(GLfixed value) {
	return (GLfloat) ldexp((double) value, -16);
}

/**
 * Convert a floating point number to S15.16 fixed point
 * 
 * @param value
 * 		the value to convert
 * 
 * @return
 * 		the equivalent fixed point number, subject to potential
 * 		over- or underflows
 */
GLES_INLINE static GLfixed GlesFixedf(GLfloat value) {
	return (GLfixed) value * 65536.0f;
}

/**
 * Convert a byte to float and normalize the range, i.e.
 * [GLES_BYTE_MIN, GLES_BYTE_MAX] is mapped to [-1.0, 1.0].
 * 
 * @param value
 * 		the value to convert
 * 		
 * @return
 * 		the converted value
 */
GLES_INLINE static GLfloat GlesNormalizedByte(GLbyte value) {
	if (value >= 0) {
		return (GLfloat) value * (1.0f / GLES_BYTE_MAX);
	} else {
		return (GLfloat) value * (-1.0f / GLES_BYTE_MIN);
	}
}

/**
 * Convert an unsigned byte to float and normalize the range, i.e.
 * [0, GLES_UBYTE_MAX] is mapped to [0.0, 1.0].
 * 
 * @param value
 * 		the value to convert
 * 		
 * @return
 * 		the converted value
 */
GLES_INLINE static GLfloat GlesNormalizedUnsignedByte(GLubyte value) {
	return (GLfloat) value * (1.0f / GLES_UBYTE_MAX);
}

/**
 * Convert a short to float and normalize the range, i.e.
 * [GLES_SHORT_MIN, GLES_SHORT_MAX] is mapped to [-1.0, 1.0].
 * 
 * @param value
 * 		the value to convert
 * 		
 * @return
 * 		the converted value
 */
GLES_INLINE static GLfloat GlesNormalizedShort(GLshort value) {
	if (value >= 0) {
		return (GLfloat) value * (1.0f / GLES_SHORT_MAX);
	} else {
		return (GLfloat) value * (-1.0f / GLES_SHORT_MIN);
	}
}

/**
 * Convert an unsigned short to float and normalize the range, i.e.
 * [0, GLES_USHORT_MAX] is mapped to [0.0, 1.0].
 * 
 * @param value
 * 		the value to convert
 * 		
 * @return
 * 		the converted value
 */
GLES_INLINE static GLfloat GlesNormalizedUnsignedShort(GLushort value) {
	return (GLfloat) value * (1.0f / GLES_USHORT_MAX);
}

/**
 * Convert an int to float and normalize the range, i.e.
 * [GLES_INT_MIN, GLES_INT_MAX] is mapped to [-1.0, 1.0].
 * 
 * @param value
 * 		the value to convert
 * 		
 * @return
 * 		the converted value
 */
GLES_INLINE static GLfloat GlesNormalizedInt(GLint value) {
	if (value >= 0) {
		return (GLfloat) ((double) value * (1.0 / GLES_INT_MAX));
	} else {
		return (GLfloat) ((double) value * (-1.0 / GLES_INT_MIN));
	}
}

/**
 * Convert an unsigned int to float and normalize the range, i.e.
 * [0, GLES_UINT_MAX] is mapped to [0.0, 1.0].
 * 
 * @param value
 * 		the value to convert
 * 		
 * @return
 * 		the converted value
 */
GLES_INLINE static GLfloat GlesNormalizedUnsignedInt(GLuint value) {
	return (GLfloat) ((double) value * (1.0 / GLES_UINT_MAX));
}


/*
** --------------------------------------------------------------------------
** Math Functions
** --------------------------------------------------------------------------
*/

/**
 * Round a floating point value to the nearest integer
 * 
 * @param value
 * 		the value to round
 * 
 * @return
 * 		the rounded result
 */
GLES_INLINE static GLfloat GlesRoundf(GLfloat value) {
	return roundf(value);
}

/**
 * Round a floating point value to the next lower integer
 * 
 * @param value
 * 		the value to round
 * 
 * @return
 * 		the rounded result
 */
GLES_INLINE static GLfloat GlesFloorf(GLfloat value) {
	return floorf(value);
}

/**
 * Return the fractional part of value.
 * 
 * @param value
 * 
 * @return value - floor(value)
 */
GLES_INLINE static GLfloat GlesFracf(GLfloat value) {
	return value - GlesFloorf(value);
}

/**
 * Convert a floating point value to its absolute value
 * 
 * @param value
 * 		the value to convert
 * 
 * @return
 * 		the converted result
 */
GLES_INLINE static GLfloat GlesFabsf(GLfloat value) {
	return fabsf(value);
}

/**
 * Convert a floating point value to its signum
 * 
 * @param value
 * 		the value to convert
 * 
 * @return
 * 		-1, 0, or one indiciating the sign of value
 */
GLES_INLINE static GLfloat GlesSignf(GLfloat value) {
	return value > 0.0f ? 1.0f : value < 0.0f ? -1.0f : 0.0f;
}

/**
 * The minimum of two integer values
 * 
 * @param a
 * 		first argument
 * @param b
 * 		second argument
 * 
 * @return
 * 		the minimum of the two values
 */
GLES_INLINE static GLint GlesMini(GLint a, GLint b) {
	return a < b ? a : b;
}

/**
 * The maximum of two integer values
 * 
 * @param a
 * 		first argument
 * @param b
 * 		second argument
 * 
 * @return
 * 		the maximum of the two values
 */
GLES_INLINE static GLint GlesMaxi(GLint a, GLint b) {
	return a > b ? a : b;
}

/**
 * The minimum of two floating point values
 * 
 * @param a
 * 		first argument
 * @param b
 * 		second argument
 * 
 * @return
 * 		the minimum of the two values
 */
GLES_INLINE static GLfloat GlesMinf(GLfloat a, GLfloat b) {
	return a < b ? a : b;
}

/**
 * The maximum of two floating point values
 * 
 * @param a
 * 		first argument
 * @param b
 * 		second argument
 * 
 * @return
 * 		the maximum of the two values
 */
GLES_INLINE static GLfloat GlesMaxf(GLfloat a, GLfloat b) {
	return a > b ? a : b;
}

/**
 * Create a floating point number from mantissa and exponent
 * 
 * @param mantissa
 * 		the value to load as mantissa
 * @param exponent
 * 		the value to load as exponent
 * 
 * @return mantissa * 2^exponent
 */
GLES_INLINE static GLfloat GlesLdexpf(GLfloat mantissa, GLint exponent) {
	return ldexpf(mantissa, exponent);
}

/**
 * Exponentiation of a floating point number 
 * 
 * @param base
 * 		the value to use as base
 * @param exponent
 * 		the value to use as exponent
 * 
 * @return mantissa * 2^exponent
 */
GLES_INLINE static GLfloat GlesPowf(GLfloat base, GLfloat exponent) {
	return powf(base, exponent);
}

GLfloat GlesLog2f(GLfloat value);

/*
** --------------------------------------------------------------------------
** Memory Functions
** --------------------------------------------------------------------------
*/

void GlesMemset(void * ptr, GLubyte value, GLsizeiptr size);
void GlesMemcpy(void * dst, const void * src, GLsizeiptr size);
GLint GlesMemcmp(const char * first, const char * second, GLsizeiptr length);
void GlesMemmove(void * dst, const void * src, GLsizeiptr size);

/*
** --------------------------------------------------------------------------
** String Functions
** --------------------------------------------------------------------------
*/

char * GlesStrchr(const char *s, char c);
GLsizei GlesStrlen(const char * string);
GLint GlesStrcmp(const char * first, const char * second);
GLint GlesStrncmp(const char * first, const char * second, GLsizeiptr length);

GLsizei GlesSprintf(char * target, const char * format, ...);

/*
** --------------------------------------------------------------------------
** Memory Management
** --------------------------------------------------------------------------
*/

void * GlesMalloc(GLsizeiptr size);
void GlesFree(void * ptr);

/*
** --------------------------------------------------------------------------
** Long Jumps
** --------------------------------------------------------------------------
*/

GLint GlesSetjmp(JumpBuffer env);
void GlesLongjmp(JumpBuffer env, GLint val);


#endif /* ndef GLES_PLATFORM_PLATFORM_H */