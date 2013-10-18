/*
** ==========================================================================
**
** $Id: program.c 76 2007-10-20 04:34:44Z hmwill $
**
** Program management functions
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-10-19 21:34:44 -0700 (Fri, 19 Oct 2007) $
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
#include "frontend/linker.h"

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void GlesInitProgram(Program * program) {
	GlesLogInit(&program->log);

	/* state flags */
	
	program->isValid 	= GL_FALSE;
	program->isLinked 	= GL_FALSE;
	program->isDeleted 	= GL_FALSE;

	/* object pointers */
	
	program->vertexShader 	= NULL;
	program->fragmentShader = NULL;
	program->uniformData 	= NULL;
	program->uniformTypes 	= NULL;
	program->executable 	= NULL;
}

Program * GlesGetProgramObject(State * state, GLuint program) {
	if (!program ||
		!GlesIsBoundObject(state->programFreeList, GLES_MAX_PROGRAMS, program)) {
		GlesRecordInvalidValue(state);
		return NULL;
	}
	
	return state->programs + program;
}

static void FreeProgramData(State * state, Program * program) {
	
	GlesLogDeInit(&program->log);

	if (program->uniformData) {
		GlesFree(program->uniformData);
		program->uniformData = NULL;
	}
	
	if (program->uniformTypes) {
		GlesFree(program->uniformTypes);
		program->uniformTypes = NULL;
	}
	
	if (program->executable) {
		GlesDeleteExecutable(state, program->executable);
		program->executable = NULL;
	}
}

void GlesDeleteProgram(State * state, Program * program) {
	FreeProgramData(state, program);
	GlesUnrefShader(state, program->fragmentShader);
	GlesUnrefShader(state, program->vertexShader);
	GlesUnbindObject(state->programFreeList, GLES_MAX_PROGRAMS, 
		program - state->programs);
}

GLboolean GlesValidateProgram(State * state, Program * program, Log * log) {
	
	GLuint uniformIndex;
	
	if (!program->isLinked) {
		GlesLogAppend(log, "Program is not linked.\n", -1);
		return GL_FALSE;
	}
	
	for (uniformIndex = 0; uniformIndex < program->executable->numUniforms; ++uniformIndex) {
		const ShaderVariable * uniform = &program->executable->uniforms[uniformIndex];
		GLenum textureType;
		GLuint unit;
		
		switch (uniform->type) {
		case GL_SAMPLER_2D:		textureType = GL_TEXTURE_2D;		break;
		case GL_SAMPLER_3D:		textureType = GL_TEXTURE_3D;		break;
		case GL_SAMPLER_CUBE:	textureType = GL_TEXTURE_CUBE_MAP;	break;
		default:
			continue;
		}
		
		unit = (GLuint) program->uniformData[uniform->location].x;
		
		if (unit >= GLES_MAX_TEXTURE_UNITS) {
			GlesLogAppend(log, "Invalid texture image unit bound to uniform sampler.\n", -1);
			continue;
		}
		
		if (!state->textureUnits[unit].boundTexture ||
			state->textureUnits[unit].boundTexture->base.textureType != textureType) {
			GlesLogAppend(log, "Uniform is bound to texture image unit with mismatching texture type.\n", -1);
		}
	}
	
	return GL_TRUE;
}

GLboolean GlesPrepareProgram(State * state) {
	Program * programObject = GlesGetProgramObject(state, state->program);
	
	if (!programObject) {
		/* no current program or program not defined */
		GlesRecordInvalidOperation(state);
		return GL_FALSE;
	}
	
	if (!GlesValidateProgram(state, programObject, NULL)) {
		GlesRecordInvalidOperation(state);
		return GL_FALSE;
	}

	/* TODO: This whole block needs to be modified */
	state->vertexContext.state = state;
	state->vertexContext.attrib = &state->currentAttrib[0];
	state->vertexContext.textureImageUnit = state->textureUnits;
	
	state->fragContext.state = state;
	state->fragContext.textureImageUnit = state->textureUnits;
	
	return GL_TRUE;
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY glAttachShader (GLuint program, GLuint shader) {
	State * state = GLES_GET_STATE();
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	Program * programObject = GlesGetProgramObject(state, program);
	
	if (!shaderObject || !programObject) {
		return;
	}
	
	if (shaderObject->isDeleted || programObject->isDeleted) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (shaderObject->type) {
	case GL_FRAGMENT_SHADER:
		GlesUnrefShader(state, programObject->fragmentShader);
		programObject->fragmentShader = shader;
		++shaderObject->attachmentCount;
		break;
	
	case GL_VERTEX_SHADER:
		GlesUnrefShader(state, programObject->vertexShader);
		programObject->vertexShader = shader;
		++shaderObject->attachmentCount;
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
	}
}

GL_API void GL_APIENTRY glBindAttribLocation (GLuint program, GLuint index, const char * name) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	
	if (!programObject) {
		return;
	}
	
	/* TODO: Implement this function */
	
	programObject->isLinked = GL_FALSE;
}

GL_API GLuint GL_APIENTRY glCreateProgram (void) {

	State * state = GLES_GET_STATE();
	GLuint program = GlesBindObject(state->programFreeList, GLES_MAX_PROGRAMS);

	if (program == NIL) {
		GlesRecordError(state, GL_OUT_OF_MEMORY);
		return 0;
	} else {
		GlesInitProgram(state->programs + program);
		return program;
	}
}

GL_API void GL_APIENTRY glDeleteProgram (GLuint program) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	
	if (!program) {
		return;
	}
	
	if (state->program == program) {
		programObject->isDeleted = GL_TRUE;
	} else {
		GlesDeleteProgram(state, programObject);
	}
}

GL_API void GL_APIENTRY glDetachShader (GLuint program, GLuint shader) {
	State * state = GLES_GET_STATE();
	Shader * shaderObject = GlesGetShaderObject(state, shader);
	Program * programObject = GlesGetProgramObject(state, program);
	
	if (shaderObject || !programObject) {
		return;
	}
	
	if (shaderObject->isDeleted || programObject->isDeleted) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (shaderObject->type) {
	case GL_FRAGMENT_SHADER:
		if (programObject->fragmentShader != shader) {
			GlesRecordInvalidOperation(state);
		}
		
		GlesUnrefShader(state, shader);
		programObject->fragmentShader = 0;
		break;
	
	case GL_VERTEX_SHADER:
		if (programObject->vertexShader != shader) {
			GlesRecordInvalidOperation(state);
		}
		
		GlesUnrefShader(state, shader);
		programObject->vertexShader = 0;
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
	}
}

GL_API void GL_APIENTRY glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	GLsizei returnLength, attribLength;
	
	if (!programObject) {
		return;
	}
	
	if (!programObject->isLinked) {
		*name = '\0';
		
		if (*length) {
			*length = 0;
		}
		
		return;		
	}
	
	if (index >= programObject->executable->numVertexAttribs || !type || !size || !name) {
		GlesRecordInvalidValue(state);
		return;
	}
	
	ShaderVariable * attrib = programObject->executable->attribs + index;
	
	*type = attrib->type;
	*size = attrib->size;
	
	attribLength = GlesStrlen(attrib->name);
	returnLength = bufsize < attribLength + 1 ? bufsize - 1 : attribLength;
	GlesMemcpy(name, attrib->name, returnLength);
	name[returnLength] = '\0';
	
	if (length) {
		*length = returnLength;
	}
}

GL_API void GL_APIENTRY glGetActiveUniform (GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	GLsizei returnLength, attribLength;
	
	if (!programObject) {
		return;
	}
	
	if (!programObject->isLinked) {
		*name = '\0';
		
		if (*length) {
			*length = 0;
		}
		
		return;		
	}
	
	if (index >= programObject->executable->numUniforms || !type || !size || !name || !bufsize) {
		GlesRecordInvalidValue(state);
		return;
	}
	
	ShaderVariable * attrib = programObject->executable->uniforms + index;
	
	*type = attrib->type;
	*size = attrib->size;
	
	attribLength = GlesStrlen(attrib->name);
	returnLength = bufsize < attribLength + 1 ? bufsize - 1 : attribLength;
	GlesMemcpy(name, attrib->name, returnLength);
	name[returnLength] = '\0';
	
	if (length) {
		*length = returnLength;
	}
}

GL_API void GL_APIENTRY glGetAttachedShaders (GLuint program, GLsizei maxcount, GLsizei *count, GLuint *shaders) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	GLsizei returnCount = 0;
	
	if (!programObject) {
		return;
	}
	
	if (!shaders) {
		GlesRecordInvalidValue(state);
		return;
	}
	
	if (returnCount < maxcount && programObject->fragmentShader) {
		*shaders++ = programObject->fragmentShader;
		++returnCount;
	}
	
	if (returnCount < maxcount && programObject->vertexShader) {
		*shaders++ = programObject->vertexShader;
		++returnCount;
	}
	
	if (count) {
		*count = returnCount;
	}
}

GL_API GLint GL_APIENTRY glGetAttribLocation (GLuint program, const char *name) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	GLint index;
	GLsizei length;
	
	if (!programObject) {
		return -1;
	}
	
	if (!name) {
		GlesRecordInvalidValue(state);
		return -1;
	}
	
	length = GlesStrlen(name);
	
	for (index = 0; index < programObject->executable->numVertexAttribs; ++index) {
		if (!GlesStrcmp(programObject->executable->attribs[index].name, name)) {
			return programObject->executable->attribs[index].location;
		}
	}
	
	return -1;
}

GL_API GLint GL_APIENTRY glGetUniformLocation (GLuint program, const char *name) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	GLint index;
	GLsizei length;
	
	if (!programObject) {
		return -1;
	}

	if (!name) {
		GlesRecordInvalidValue(state);
		return -1;
	}
	
	length = GlesStrlen(name);
	
	for (index = 0; index < programObject->executable->numUniforms; ++index) {
		/* treat lookup of array names similar to lookup of first element */
		const ShaderVariable * uniform = programObject->executable->uniforms + index;
		GLsizeiptr attribLength = GlesStrlen(uniform->name);
		
		if ((length == attribLength &&
			!GlesStrncmp(uniform->name, name, length)) ||
			(length + 3 == attribLength &&
			!GlesStrncmp(uniform->name, name, length) &&
			!GlesStrncmp(uniform->name + length, "[0]", 3))) {
			return uniform->location;
		}
	}
	
	return -1;
}

GL_API void GL_APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufsize, GLsizei *length, char *infolog) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	
	if (!programObject) {
		return;
	}
	
	if (!infolog) {
		GlesRecordInvalidValue(state);
		return;
	}
	
	GlesLogExtract(&programObject->log, bufsize, infolog, length);
}

GL_API void GL_APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	GLuint index;
	GLint result;
	
	if (!programObject) {
		return;
	}
	
	switch (pname) {
	case GL_DELETE_STATUS:
		*params = programObject->isDeleted;
		break;
		
	case GL_LINK_STATUS:
		*params = programObject->isLinked;
		break;
		
	case GL_VALIDATE_STATUS:
		*params = programObject->isValid;
		break;
		
	case GL_INFO_LOG_LENGTH:
		*params = programObject->log.logSize;
		break;
		
	case GL_ATTACHED_SHADERS:
		*params = (programObject->fragmentShader != 0) +
			(programObject->vertexShader != 0);
		break;
		
	case GL_ACTIVE_ATTRIBUTES:
		*params = programObject->executable->numVertexAttribs;
		break;
		
	case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
		result = 0;
		
		for (index = 0; index < programObject->executable->numVertexAttribs; ++index) {
			GLsizeiptr attribLength = GlesStrlen(programObject->executable->attribs[index].name);
			
			if (attribLength > result) {
				result = attribLength;
			}
		}
		
		*params = result;
		break;
		
	case GL_ACTIVE_UNIFORMS:
		*params = programObject->executable->numUniforms;
		break;
		
	case GL_ACTIVE_UNIFORM_MAX_LENGTH:
		result = 0;
		
		for (index = 0; index < programObject->executable->numUniforms; ++index) {
			GLsizeiptr attribLength = GlesStrlen(programObject->executable->uniforms[index].name);
			
			if (attribLength > result) {
				result = attribLength;
			}
		}
		
		*params = result;
		break;
		
	default:
		GlesRecordInvalidEnum(state);
		return; 
	}
}

GL_API GLboolean GL_APIENTRY glIsProgram (GLuint program) {

	State * state = GLES_GET_STATE();
	return GlesIsBoundObject(state->programFreeList, GLES_MAX_PROGRAMS, program);
}

GL_API void GL_APIENTRY glLinkProgram (GLuint program) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	Shader * vertexShader, * fragmentShader;
	GLuint index, count;
	Executable * executable;
	
	if (!state->linker) {
		state->linker = GlesLinkerCreate(state);
		
		if (!state->linker) {
			GlesRecordOutOfMemory(state);
			return;
		}
	}

	if (!programObject->fragmentShader || !programObject->vertexShader) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	vertexShader = GlesGetShaderObject(state, programObject->vertexShader);
	fragmentShader = GlesGetShaderObject(state, programObject->fragmentShader);

	if (!vertexShader->isCompiled || !fragmentShader->isCompiled) {
		GlesRecordInvalidOperation(state);
		return;
	}

	FreeProgramData(state, programObject);
	programObject->isLinked = GL_FALSE;

	executable = GlesLinkProgram(state->linker, programObject);
	
	if (!executable) {
		return;
	}
	
	programObject->executable = executable;
	programObject->uniformData = (Vec4f *) GlesMalloc(executable->sizeUniforms * sizeof(Vec4f));
	programObject->uniformTypes = (GLuint *) GlesMalloc(executable->sizeUniforms * sizeof(GLuint));

	if (!programObject->uniformData || !programObject->uniformTypes) {
		GlesRecordOutOfMemory(state);
		FreeProgramData(state, programObject);
		return;
	}
	
	for (index = 0; index < executable->numUniforms; ++index) {
		const ShaderVariable * uniform = &executable->uniforms[index];
		GLuint elements = uniform->size;
		
		GLES_ASSERT(uniform->location + uniform->size <= executable->sizeUniforms);
		
		switch (uniform->type) {
		case GL_FLOAT_MAT2:		elements *= 2;	break;
		case GL_FLOAT_MAT3:		elements *= 3;	break;
		case GL_FLOAT_MAT4:		elements *= 4;	break;
		default:								break;
		}
		
		for (count = 0; count < elements; ++count) {
			/* create indices from actual values back to meta-data */
			programObject->uniformTypes[uniform->location + count] = index;
		}
	}

	programObject->isLinked = GL_TRUE;
}

GL_API void GL_APIENTRY glUseProgram (GLuint program) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	
	if (!programObject) {
		return;
	}
	
	if (programObject->isDeleted) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	if (state->program) {
		Program * oldProgram = GlesGetProgramObject(state, program);
		
		if (oldProgram->isDeleted) {
			GlesDeleteProgram(state, oldProgram);
		}
	}
	
	state->program = program;
}

GL_API void GL_APIENTRY glValidateProgram (GLuint program) {
	
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	
	if (!programObject) {
		return;
	}
	
	GlesLogClear(&programObject->log);	
	programObject->isValid = GlesValidateProgram(state, programObject, &programObject->log);
}

