/*
** ==========================================================================
**
** $Id: shader.c 64 2007-09-22 20:23:16Z hmwill $
**
** Shader management functions
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-22 13:23:16 -0700 (Sat, 22 Sep 2007) $
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
#include "frontend/compiler.h"


/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

static void InitShader(Shader * shader, GLenum shaderType) {
	GLES_ASSERT(shaderType == GL_FRAGMENT_SHADER || shaderType == GL_VERTEX_SHADER);
	GlesLogInit(&shader->log);
	shader->type = shaderType;
	shader->attachmentCount = 0;
	shader->isDeleted = GL_FALSE;
	shader->text = NULL;
	shader->length = 0;
	shader->il = NULL;
	shader->size = 0;
}

static void FreeShaderSource(Shader * shaderObject) {
	if (shaderObject->text) {
		GlesFree(shaderObject->text);
		shaderObject->text = NULL;
		shaderObject->length = 0;
	}
}

static void FreeShaderIntermediate(Shader * shaderObject) {
	if (shaderObject->il) {
		GlesFree(shaderObject->il);
		shaderObject->il = NULL;
		shaderObject->size = 0;
	}
}

void GlesDeleteShader(State * state, Shader * shader) {
	GLES_ASSERT(shader - state->shaders >= 0 &&
		shader - state->shaders < GLES_MAX_SHADERS);
		
	FreeShaderSource(shader);
	FreeShaderIntermediate(shader);
	GlesLogDeInit(&shader->log);
	GlesUnbindObject(state->shaderFreeList, GLES_MAX_SHADERS, 
		shader - state->shaders);
}

Shader * GlesGetShaderObject(State * state, GLuint shader) {
	if (!GlesIsBoundObject(state->shaderFreeList, GLES_MAX_SHADERS, shader)) {
		GlesRecordInvalidValue(state);
		return NULL;
	}
	
	return state->shaders + shader;
}

void GlesUnrefShader(State * state, GLuint shader) {
	if (shader) {
		Shader * shaderObject = GlesGetShaderObject(state, shader);
		--shaderObject->attachmentCount;
		
		if (!shaderObject->attachmentCount && shaderObject->isDeleted) {
			GlesDeleteShader(state, shaderObject);
		}
	}
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API GLuint GL_APIENTRY glCreateShader (GLenum type) {

	State * state = GLES_GET_STATE();
	GLuint shader;

	if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER) {
		GlesRecordInvalidEnum(state);
		return 0;
	}

	shader = GlesBindObject(state->shaderFreeList, GLES_MAX_SHADERS);

	if (shader == NIL) {
		GlesRecordError(state, GL_OUT_OF_MEMORY);
		return 0;
	} else {
		InitShader(state->shaders + shader, type);
		return shader;
	}
}

GL_API GLboolean GL_APIENTRY glIsShader (GLuint shader) {

	State * state = GLES_GET_STATE();
	return GlesIsBoundObject(state->shaderFreeList, GLES_MAX_SHADERS, shader);
}

GL_API void GL_APIENTRY glDeleteShader (GLuint shader) {
	State * state = GLES_GET_STATE();
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	
	if (!shaderObject) {
		return;
	}
	
	if (shaderObject->attachmentCount) {
		shaderObject->isDeleted = GL_TRUE;
	} else {
		GlesDeleteShader(state, shaderObject);
	}
}

/* OES_shader_source */
GL_API void GL_APIENTRY glCompileShader (GLuint shader) {
	State * state = GLES_GET_STATE();
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	
	if (!shaderObject) {
		return;
	}
	
	GlesLogClear(&shaderObject->log);
	
	if (!state->compiler) {
		state->compiler = GlesCompilerCreate(state);
		
		if (!state->compiler) {
			GlesRecordOutOfMemory(state);
			return;
		}
	}
	
	shaderObject->isCompiled = GlesCompileShader(state->compiler, shaderObject);
	
	GLES_ASSERT(shaderObject->isCompiled == (shaderObject->il != NULL));
}

GL_API void GL_APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	
	if (!shaderObject) {
		return;
	}
	
	switch (pname) {
	case GL_SHADER_TYPE:
		*params = shaderObject->type;
		break;
		
	case GL_DELETE_STATUS:
		*params = shaderObject->isDeleted;
		break;
		
	case GL_COMPILE_STATUS:
		*params = shaderObject->isCompiled;
		break;
		
	case GL_INFO_LOG_LENGTH:
		*params = shaderObject->log.logSize + 1;
		break;
		
	case GL_SHADER_SOURCE_LENGTH:
		*params = shaderObject->length + 1;
		break;
		
	case GL_SHADER_INTERMEDIATE_LENGTH_VIN:
		*params = shaderObject->size + 1;
		break;

	default:
		GlesRecordInvalidEnum(state);
		return;
	}
}

GL_API void GL_APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufsize, GLsizei *length, char *infolog) {
	State * state = GLES_GET_STATE();
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	
	if (!shaderObject) {
		return;
	}
	
	if (!infolog) {
		GlesRecordInvalidValue(state);
		return;
	}
	
	GlesLogExtract(&shaderObject->log, bufsize, infolog, length);
}

GL_API void GL_APIENTRY glGetShaderIntermediateVIN (GLuint shader, GLsizei bufsize, GLsizei *length, char *intermediate) {
	State * state = GLES_GET_STATE();
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	GLsizei returnedLength;		
	
	if (!shaderObject) {
		return;
	}
	
	if (!intermediate) {
		GlesRecordInvalidValue(state);
		return;
	}
	
	returnedLength = shaderObject->size + 1 >= bufsize ? bufsize - 1 : shaderObject->size;
	
	GlesMemcpy(intermediate, shaderObject->il, returnedLength);
	intermediate[returnedLength] = '\0';
	
	if (length) {
		*length = returnedLength;
	}
}

GL_API void GL_APIENTRY glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei *length, char *source) {
	State * state = GLES_GET_STATE();
	GLsizei returnedLength;		
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	
	if (!shaderObject) {
		return;
	}
	
	if (!source) {
		GlesRecordInvalidValue(state);
		return;
	}
	
	returnedLength = shaderObject->length + 1 >= bufsize ? bufsize - 1 : shaderObject->length;
	
	GlesMemcpy(source, shaderObject->text, returnedLength);
	source[returnedLength] = '\0';
	
	if (length) {
		*length = returnedLength;
	}
}

GL_API void GL_APIENTRY glReleaseShaderCompilerOES(void) {
	State * state = GLES_GET_STATE();

	if (state->compiler) {
		GlesCompilerDestroy(state->compiler);
		state->compiler = NULL;
	}
}

GL_API void GL_APIENTRY glShaderSource(GLuint shader, GLsizei count, const char **string, const GLint *length) {
	State * state = GLES_GET_STATE();
	GLuint index, totalLength = 0;
	char * text;
		
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	
	if (!shaderObject) {
		return;
	}
	
	FreeShaderSource(shaderObject);

	for (index = 0; index < count; ++index) {
		const char * source = string[index];
		GLuint lineLength = 0;
		
		if (source) {
			if (length && length[index] >= 0) {
				lineLength = length[index];
			} else {
				lineLength = strlen(source);
			}
		}
		
		totalLength += lineLength;
	}
	
	text = shaderObject->text = (char *) GlesMalloc(totalLength + 1);
	
	if (!text) {
		GlesRecordOutOfMemory(state);
		return;
	}
	
	shaderObject->length = totalLength;
	
	for (index = 0; index < count; ++index) {
		const char * source = string[index];
		
		if (source) {
			GLuint lineLength;
			
			if (length && length[index] >= 0) {
				lineLength = length[index];
			} else {
				lineLength = strlen(source);
			}
			
			GlesMemcpy(text, source, lineLength);
			text += lineLength;
		}
	}
	
	/* add string terminator */
	*text = '\0';
}

/* OES_shader_source + OES_shader_binary */
GL_API void GL_APIENTRY glGetShaderPrecisionFormatOES(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) {
	State * state = GLES_GET_STATE();
	
	if (shadertype != GL_VERTEX_SHADER && shadertype != GL_FRAGMENT_SHADER) {
		GlesRecordInvalidEnum(state);
		return;
	}
	
	/* TODO: need to recognize the correct values for precisiontype */
	
	*range = GLES_VERTEX_HIGH_FLOAT_RANGE;
	*precision = GLES_VERTEX_HIGH_FLOAT_PRECISION;
}
