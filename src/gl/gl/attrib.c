/*
** ==========================================================================
**
** $Id: attrib.c 60 2007-09-18 01:16:07Z hmwill $		
**
** Attribute functions
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
#include "gl/state.h"


/*
** --------------------------------------------------------------------------
** Module variables
** --------------------------------------------------------------------------
*/

static GLenum VertexAttribTypeValues[] = {
	GL_BYTE,
	GL_UNSIGNED_BYTE,
	GL_SHORT,
	GL_UNSIGNED_SHORT,
	GL_INT,
	GL_UNSIGNED_INT,
	GL_FLOAT,
	GL_FIXED
};

/*
** --------------------------------------------------------------------------
** Fetch functions for individual array elements based on type
** --------------------------------------------------------------------------
*/

static void FetchByte(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLbyte * values = (const GLbyte *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = values[3];
		
	case 3:
		result->v[2] = values[2];
		
	case 2:
		result->v[1] = values[1];
		
	case 1:
		result->v[0] = values[0];
	}
}

static void FetchUnsignedByte(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLubyte * values = (const GLubyte *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = values[3];
		
	case 3:
		result->v[2] = values[2];
		
	case 2:
		result->v[1] = values[1];
		
	case 1:
		result->v[0] = values[0];
	}
}

static void FetchShort(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLshort * values = (const GLshort *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = values[3];
		
	case 3:
		result->v[2] = values[2];
		
	case 2:
		result->v[1] = values[1];
		
	case 1:
		result->v[0] = values[0];
	}
}

static void FetchUnsignedShort(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLushort * values = (const GLushort *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = values[3];
		
	case 3:
		result->v[2] = values[2];
		
	case 2:
		result->v[1] = values[1];
		
	case 1:
		result->v[0] = values[0];
	}
}

static void FetchInt(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLint * values = (const GLint *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = values[3];
		
	case 3:
		result->v[2] = values[2];
		
	case 2:
		result->v[1] = values[1];
		
	case 1:
		result->v[0] = values[0];
	}
}

static void FetchUnsignedInt(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLuint * values = (const GLuint *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = values[3];
		
	case 3:
		result->v[2] = values[2];
		
	case 2:
		result->v[1] = values[1];
		
	case 1:
		result->v[0] = values[0];
	}
}

static void FetchByteNormalize(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLbyte * values = (const GLbyte *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = GlesNormalizedByte(values[3]);
		
	case 3:
		result->v[2] = GlesNormalizedByte(values[2]);
		
	case 2:
		result->v[1] = GlesNormalizedByte(values[1]);
		
	case 1:
		result->v[0] = GlesNormalizedByte(values[0]);
	}
}

static void FetchUnsignedByteNormalize(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLubyte * values = (const GLubyte *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = GlesNormalizedUnsignedByte(values[3]);
		
	case 3:
		result->v[2] = GlesNormalizedUnsignedByte(values[2]);
		
	case 2:
		result->v[1] = GlesNormalizedUnsignedByte(values[1]);
		
	case 1:
		result->v[0] = GlesNormalizedUnsignedByte(values[0]);
	}
}

static void FetchShortNormalize(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLshort * values = (const GLshort *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = GlesNormalizedShort(values[3]);
		
	case 3:
		result->v[2] = GlesNormalizedShort(values[2]);
		
	case 2:
		result->v[1] = GlesNormalizedShort(values[1]);
		
	case 1:
		result->v[0] = GlesNormalizedShort(values[0]);
	}
}

static void FetchUnsignedShortNormalize(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLushort * values = (const GLushort *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = GlesNormalizedUnsignedShort(values[3]);
		
	case 3:
		result->v[2] = GlesNormalizedUnsignedShort(values[2]);
		
	case 2:
		result->v[1] = GlesNormalizedUnsignedShort(values[1]);
		
	case 1:
		result->v[0] = GlesNormalizedUnsignedShort(values[0]);
	}
}

static void FetchIntNormalize(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLint * values = (const GLint *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = GlesNormalizedInt(values[3]);
		
	case 3:
		result->v[2] = GlesNormalizedInt(values[2]);
		
	case 2:
		result->v[1] = GlesNormalizedInt(values[1]);
		
	case 1:
		result->v[0] = GlesNormalizedInt(values[0]);
	}
}

static void FetchUnsignedIntNormalize(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLuint * values = (const GLuint *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = GlesNormalizedUnsignedInt(values[3]);
		
	case 3:
		result->v[2] = GlesNormalizedUnsignedInt(values[2]);
		
	case 2:
		result->v[1] = GlesNormalizedUnsignedInt(values[1]);
		
	case 1:
		result->v[0] = GlesNormalizedUnsignedInt(values[0]);
	}
}

static void FetchFloat(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLfloat * values = (const GLfloat *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = values[3];
		
	case 3:
		result->v[2] = values[2];
		
	case 2:
		result->v[1] = values[1];
		
	case 1:
		result->v[0] = values[0];
	}
}

static void FetchFixed(const Array * array, GLsizei index, Vec4f * result) {
	const void * address = (const GLbyte *) array->effectivePtr + index * array->stride;
	const GLfixed * values = (const GLfixed *) address;
	
	switch (array->size) {
	default:
	case 4: 
		result->v[3] = GlesFloatx(values[3]);
		
	case 3:
		result->v[2] = GlesFloatx(values[2]);
		
	case 2:
		result->v[1] = GlesFloatx(values[1]);
		
	case 1:
		result->v[0] = GlesFloatx(values[0]);
	}
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

/**
 * Initialize the given vertex attrib array to default state.
 * 
 * @param array
 * 		the array to initialize
 */
void GlesInitArray(Array * array) {
	array->ptr			= NULL;
	array->boundBuffer	= 0;
	array->size			= 0;
	array->stride		= 0;
	array->type			= GL_FLOAT;
	array->normalized	= GL_FALSE;
	array->enabled		= GL_FALSE;
}

/**
 * Prepare the given vertex attrib array for rendering.
 * 
 * This will resolve any bound buffer address, and will also set the
 * correct fetch function for this array.
 * 
 * @param array
 * 		the array to prepare
 */
void GlesPrepareArray(State * state, Array * array) {
	if (array->enabled) {
		if (array->boundBuffer) {
			array->effectivePtr = (const GLbyte *) state->buffers[array->boundBuffer].data +
				((const GLbyte *) array->ptr - (const GLbyte *) 0);
		} else {
			array->effectivePtr = array->ptr;
		}
		
		switch (array->type) {
		case GL_BYTE:
			array->fetchFunc = array->normalized ? FetchByteNormalize : FetchByte;
			break;
			
		case GL_UNSIGNED_BYTE:
			array->fetchFunc = array->normalized ? FetchUnsignedByteNormalize : FetchUnsignedByte;
			break;
			
		case GL_SHORT:
			array->fetchFunc = array->normalized ? FetchShortNormalize : FetchShort;
			break;
			
		case GL_UNSIGNED_SHORT:
			array->fetchFunc = array->normalized ? FetchUnsignedShortNormalize : FetchUnsignedShort;
			break;
			
		case GL_INT:
			array->fetchFunc = array->normalized ? FetchIntNormalize : FetchInt;
			break;
			
		case GL_UNSIGNED_INT:
			array->fetchFunc = array->normalized ? FetchUnsignedIntNormalize : FetchUnsignedInt;
			break;
		
		case GL_FLOAT:
			array->fetchFunc = FetchFloat;
			break;
			
		case GL_FIXED:
			array->fetchFunc = FetchFixed;
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
	} else {
		array->fetchFunc = NULL;
		array->effectivePtr = NULL;
	}
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glDisableVertexAttribArray (GLuint index) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		state->vertexAttribArray[index].enabled = GL_FALSE;
	}
}

GL_API void GL_APIENTRY glEnableVertexAttribArray (GLuint index) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		state->vertexAttribArray[index].enabled = GL_TRUE;
	}
}

GL_API void GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		switch (pname) {
		case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
			*params = (GLfloat) (state->vertexAttribArray[index].enabled == GL_TRUE);
			break;

		case GL_VERTEX_ATTRIB_ARRAY_SIZE:
			*params = (GLfloat) (state->vertexAttribArray[index].size);
			break;

		case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
			*params = (GLfloat) (state->vertexAttribArray[index].stride);
			break;

		case GL_VERTEX_ATTRIB_ARRAY_TYPE:
			*params = (GLfloat) (state->vertexAttribArray[index].type);
			break;

		case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
			*params = (GLfloat) (state->vertexAttribArray[index].normalized == GL_TRUE);
			break;

		case GL_CURRENT_VERTEX_ATTRIB:
			params[0] = state->vertexAttrib[index].x;
			params[1] = state->vertexAttrib[index].y;
			params[2] = state->vertexAttrib[index].z;
			params[3] = state->vertexAttrib[index].w;
			break;

		default:
			GlesRecordInvalidEnum(state);
			break;
		}
	}
}

GL_API void GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		switch (pname) {
		case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
			*params = state->vertexAttribArray[index].enabled == GL_TRUE;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_SIZE:
			*params = state->vertexAttribArray[index].size;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
			*params = state->vertexAttribArray[index].stride;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_TYPE:
			*params = state->vertexAttribArray[index].type;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
			*params = state->vertexAttribArray[index].normalized == GL_TRUE;
			break;

		case GL_CURRENT_VERTEX_ATTRIB:
			params[0] = (GLint) state->vertexAttrib[index].x;
			params[1] = (GLint) state->vertexAttrib[index].y;
			params[2] = (GLint) state->vertexAttrib[index].z;
			params[3] = (GLint) state->vertexAttrib[index].w;
			break;

		case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
			*params = state->vertexAttribArray[index].boundBuffer;
			break;
			
		default:
			GlesRecordInvalidEnum(state);
			break;
		}
	}
}

GL_API void GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, void* *pointer) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else if (pname != GL_VERTEX_ATTRIB_ARRAY_POINTER) {
		GlesRecordInvalidEnum(state);
	} else {
		*pointer = (void *) state->vertexAttribArray[index].ptr;
	}
}

GL_API void GL_APIENTRY glVertexAttrib1f (GLuint index, GLfloat x) {
	glVertexAttrib4f(index, x, 0.0f, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib1fv (GLuint index, const GLfloat *values) {
	if (values) {
		glVertexAttrib4f(index, values[0], 0.0f, 0.0f, 1.0f);
	}
}

GL_API void GL_APIENTRY glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y) {
	glVertexAttrib4f(index, x, y, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib2fv (GLuint index, const GLfloat *values) {
	if (values) {
		glVertexAttrib4f(index, values[0], values[1], 0.0f, 1.0f);
	}
}

GL_API void GL_APIENTRY glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z) {
	glVertexAttrib4f(index, x, y, z, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib3fv (GLuint index, const GLfloat *values) {
	if (values) {
		glVertexAttrib4f(index, values[0], values[1], values[2], 1.0f);
	}
}

GL_API void GL_APIENTRY glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
	} else {
		state->vertexAttrib[index].x = x;
		state->vertexAttrib[index].y = y;
		state->vertexAttrib[index].z = z;
		state->vertexAttrib[index].w = w;
	}
}

GL_API void GL_APIENTRY glVertexAttrib4fv (GLuint index, const GLfloat *values) {
	if (values) {
		glVertexAttrib4f(index, values[0], values[1], values[2], values[3]);
	} 
}

GL_API void GL_APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * ptr) {
	State * state = GLES_GET_STATE();

	if (index >= GLES_MAX_VERTEX_ATTRIBS) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (size < 1 || size > 4 || stride < 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (!GlesValidateEnum(state, type, VertexAttribTypeValues, GLES_ELEMENTSOF(VertexAttribTypeValues))) {
		return;
	}

	if (stride == 0) {
		switch (type) {
		case GL_BYTE:			stride = size * sizeof(GLbyte);		break;
		case GL_UNSIGNED_BYTE:	stride = size * sizeof(GLubyte);	break;
		case GL_SHORT:			stride = size * sizeof(GLshort);	break;
		case GL_UNSIGNED_SHORT:	stride = size * sizeof(GLushort);	break;
		case GL_INT:			stride = size * sizeof(GLint);		break;
		case GL_UNSIGNED_INT:	stride = size * sizeof(GLuint);		break;
		case GL_FLOAT:			stride = size * sizeof(GLfloat);	break;
		case GL_FIXED:			stride = size * sizeof(GLfixed);	break;
		}
	}

	state->vertexAttribArray[index].size		= size;
	state->vertexAttribArray[index].type		= type;
	state->vertexAttribArray[index].normalized	= normalized != GL_FALSE;
	state->vertexAttribArray[index].stride		= stride;
	state->vertexAttribArray[index].ptr			= ptr;
	state->vertexAttribArray[index].boundBuffer = state->arrayBuffer;
}
