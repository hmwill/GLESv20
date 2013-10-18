/*
** ==========================================================================
**
** $Id: buffer.c 60 2007-09-18 01:16:07Z hmwill $
**
** Buffer management functions
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

static GLenum BufferUsage[] = {
	GL_STATIC_DRAW,
	GL_DYNAMIC_DRAW
};

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static Buffer * GetBufferForTarget(State * state, GLenum target) {
	switch (target) {
		case GL_ARRAY_BUFFER:
			return state->buffers + state->arrayBuffer;

		case GL_ELEMENT_ARRAY_BUFFER:
			return state->buffers + state->elementArrayBuffer;

		default:
			GlesRecordInvalidEnum(state);
			return NULL;
	}
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void GlesInitBuffer(Buffer * buffer) {
	buffer->data		= NULL;
	buffer->size		= 0;
	buffer->bufferType	= GL_INVALID_ENUM;
	buffer->usage		= GL_INVALID_ENUM;
	buffer->access		= GL_WRITE_ONLY;
	buffer->mapped		= GL_FALSE;
	buffer->mapPointer	= NULL;
}

void GlesDeallocateBuffer(Buffer * buffer) {
	if (buffer->data != NULL) {
		GlesFree(buffer->data);
	}

	GlesInitBuffer(buffer);
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) {
	State * state = GLES_GET_STATE();
	GLuint * bufferRef = NULL;

	switch (target) {
		case GL_ARRAY_BUFFER:
			bufferRef = &state->arrayBuffer;
			break;

		case GL_ELEMENT_ARRAY_BUFFER:
			bufferRef = &state->elementArrayBuffer;
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}

	if (buffer > GLES_MAX_BUFFERS) {
		GlesRecordOutOfMemory(state);
		return;
	}

	*bufferRef = buffer;
}

GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return;
	}

	if (!GlesValidateEnum(state, usage, BufferUsage, GLES_ELEMENTSOF(BufferUsage))) {
		return;
	}

	if (size < 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (buffer->data) {
		GlesDeallocateBuffer(buffer);
	}

	buffer->data = GlesMalloc(size);

	if (buffer->data == NULL) {
		GlesRecordOutOfMemory(state);
		return;
	}

	buffer->usage = usage;
	buffer->size = size;

	GlesMemcpy(buffer->data, data, size);
}

GL_API void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return;
	}

	if (size < 0 || offset < 0 || size + offset > buffer->size) {
		GlesRecordInvalidValue(state);
		return;
	}

	GlesMemcpy((GLubyte * )buffer->data + offset, data, size);
}

GL_API void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers) {
	State * state = GLES_GET_STATE();
	GLsizei attr;
	
	/************************************************************************/
	/* Validate parameters													*/
	/************************************************************************/

	if (n < 0 || buffers == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Delete the individual textures										*/
	/************************************************************************/

	while (n--) {
		if (GlesIsBoundObject(state->bufferFreeList, GLES_MAX_BUFFERS, *buffers)) {
			if (*buffers == state->arrayBuffer) {
				state->arrayBuffer = 0;
			}

			if (*buffers == state->elementArrayBuffer) {
				state->elementArrayBuffer = 0;
			}

			for (attr = 0; attr < GLES_MAX_VERTEX_ATTRIBS; ++attr) {
				if (state->vertexAttribArray[attr].boundBuffer == *buffers) {
					state->vertexAttribArray[attr].boundBuffer = 0;
				}
			}
			
			GlesDeallocateBuffer(state->buffers + *buffers);
		}

		++buffers;
	}
}

GL_API void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) {

	State * state = GLES_GET_STATE();
	GlesGenObjects(state, state->bufferFreeList, GLES_MAX_BUFFERS, n, buffers);
}

GL_API void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return;
	}

	switch (pname) {
	case GL_BUFFER_SIZE:
		params[0] = buffer->size;
		break;

	case GL_BUFFER_USAGE:
		params[0] = buffer->usage;
		break;

	case GL_BUFFER_ACCESS:
		params[0] = buffer->access;
		break;

	case GL_BUFFER_MAPPED:
		params[0] = buffer->mapped;
		break;
		
	default:
		GlesRecordInvalidEnum(state);
		return;
	}
}

GL_API void GL_APIENTRY glGetBufferPointerv (GLenum target, GLenum pname, void* *params) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return;
	}

	switch (pname) {
		case GL_BUFFER_MAP_POINTER:
			params[0] = buffer->mapPointer;
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}
}

GL_API GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) {

	State * state = GLES_GET_STATE();
	return GlesIsBoundObject(state->bufferFreeList, GLES_MAX_BUFFERS, buffer) &&
		state->buffers[buffer].bufferType != GL_INVALID_ENUM;
}

GL_API void* GL_APIENTRY glMapBuffer (GLenum target, GLenum access) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return NULL;
	}

	if (buffer->mapped) {
		GlesRecordInvalidOperation(state);
		return NULL;
	}

	buffer->mapped = GL_TRUE;
	buffer->mapPointer = buffer->data;

	return buffer->mapPointer;
}

GL_API GLboolean GL_APIENTRY glUnmapBuffer (GLenum target) {
	State * state = GLES_GET_STATE();
	Buffer * buffer = GetBufferForTarget(state, target);

	if (buffer == NULL) {
		return GL_FALSE;
	}

	if (!buffer->mapped) {
		GlesRecordInvalidOperation(state);
		return GL_FALSE;
	}

	buffer->mapped = GL_FALSE;
	buffer->mapPointer = NULL;

	return GL_TRUE;
}

