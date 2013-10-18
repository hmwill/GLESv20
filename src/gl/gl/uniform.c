/*
** ==========================================================================
**
** $Id: uniform.c 67 2007-09-25 05:51:44Z hmwill $
**
** Uniform variable setters and getters
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
#include "frontend/linker.h"


/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static const ShaderVariable * GetUniform(State * state, Program * programObject,  GLint location) {
	if (!programObject || location < 0 || location >= programObject->executable->sizeUniforms) {
		GlesRecordInvalidValue(state);
		return NULL;
	}
	
	return &programObject->executable->uniforms[programObject->uniformTypes[location]];
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY glGetUniformfv (GLuint program, GLint location, GLfloat *params) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	const Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
		params[3] = data->v[3];
		
	case GL_FLOAT_VEC3:
		params[2] = data->v[2];
		
	case GL_FLOAT_VEC2:
		params[1] = data->v[1];
		
	case GL_FLOAT:
		params[0] = data->v[0];
		break;
		
	case GL_FLOAT_MAT2:
		if (uniform->location != location) {
			GlesRecordInvalidOperation(state);
			return;
		}
		
		params[0] = data[0].v[0];
		params[1] = data[0].v[1];
		params[2] = data[1].v[0];
		params[3] = data[1].v[1];
		break;
		
	case GL_FLOAT_MAT3:
		if (uniform->location != location) {
			GlesRecordInvalidOperation(state);
			return;
		}
		
		params[0] = data[0].v[0];
		params[1] = data[0].v[1];
		params[2] = data[0].v[2];
		params[3] = data[1].v[0];
		params[4] = data[1].v[1];
		params[5] = data[1].v[2];
		params[6] = data[2].v[0];
		params[7] = data[2].v[1];
		params[8] = data[2].v[2];
		break;
		
	case GL_FLOAT_MAT4:
		if (uniform->location != location) {
			GlesRecordInvalidOperation(state);
			return;
		}
		
		params[ 0] = data[0].v[0];
		params[ 1] = data[0].v[1];
		params[ 2] = data[0].v[2];
		params[ 3] = data[0].v[3];
		params[ 4] = data[1].v[0];
		params[ 5] = data[1].v[1];
		params[ 6] = data[1].v[2];
		params[ 7] = data[1].v[3];
		params[ 8] = data[2].v[0];
		params[ 9] = data[2].v[1];
		params[10] = data[2].v[2];
		params[11] = data[2].v[3];
		params[12] = data[3].v[0];
		params[13] = data[3].v[1];
		params[14] = data[3].v[2];
		params[15] = data[3].v[3];
		break;
				
	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glGetUniformiv (GLuint program, GLint location, GLint *params) {
	State * state = GLES_GET_STATE();
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	const Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
	case GL_BOOL_VEC4:
		params[3] = data->v[3];
		
	case GL_INT_VEC3:
	case GL_BOOL_VEC3:
		params[2] = data->v[2];
		
	case GL_INT_VEC2:
	case GL_BOOL_VEC2:
		params[1] = data->v[1];
		
	case GL_INT:
	case GL_BOOL:
		params[0] = data->v[0];
		break;
						
	case GL_SAMPLER_2D:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_CUBE:
		params[0] = data->v[0];
		break;
		
	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform1f (GLint location, GLfloat x) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC2:
	case GL_FLOAT:
		data->v[0] = x;
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC2:
	case GL_BOOL:
		data->v[0] = (x != 0.0f);
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform2f (GLint location, GLfloat x, GLfloat y) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC2:
		data->v[0] = x;
		data->v[1] = y;
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC2:
		data->v[0] = (x != 0.0f);
		data->v[1] = (y != 0.0f);
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform3f (GLint location, GLfloat x, GLfloat y, GLfloat z) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
	case GL_FLOAT_VEC3:
		data->v[0] = x;
		data->v[1] = y;
		data->v[2] = z;
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
		data->v[0] = (x != 0.0f);
		data->v[1] = (y != 0.0f);
		data->v[2] = (z != 0.0f);
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
		data->v[0] = x;
		data->v[1] = y;
		data->v[2] = z;
		data->v[3] = w;
		break;

	case GL_BOOL_VEC4:
		data->v[0] = (x != 0.0f);
		data->v[1] = (y != 0.0f);
		data->v[2] = (z != 0.0f);
		data->v[3] = (w != 0.0f);
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform1i (GLint location, GLint x) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
	case GL_INT_VEC3:
	case GL_INT_VEC2:
	case GL_INT:
	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC2:
	case GL_BOOL:
	case GL_SAMPLER_2D:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_CUBE:
		data->v[0] = x;
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform2i (GLint location, GLint x, GLint y) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
	case GL_INT_VEC3:
	case GL_INT_VEC2:
	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC2:
		data->v[0] = x;
		data->v[1] = y;
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform3i (GLint location, GLint x, GLint y, GLint z) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
	case GL_INT_VEC3:
	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
		data->v[0] = x;
		data->v[1] = y;
		data->v[2] = z;
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
	case GL_BOOL_VEC4:
		data->v[0] = x;
		data->v[1] = y;
		data->v[2] = z;
		data->v[3] = w;
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->size < location + count - uniform->location) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC2:
	case GL_FLOAT:
		for (; count > 0; --count, ++data) {
			data->v[0] = *value++;
		}
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC2:
	case GL_BOOL:
		for (; count > 0; --count, ++data) {
			data->v[0] = (*value++ != 0.0f);
		}
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->size < location + count - uniform->location) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC2:
		for (; count > 0; --count, ++data) {
			data->v[0] = *value++;
			data->v[1] = *value++;
		}
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC2:
		for (; count > 0; --count, ++data) {
			data->v[0] = (*value++ != 0.0f);
			data->v[1] = (*value++ != 0.0f);
		}
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->size < location + count - uniform->location) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
	case GL_FLOAT_VEC3:
		for (; count > 0; --count, ++data) {
			data->v[0] = *value++;
			data->v[1] = *value++;
			data->v[2] = *value++;
		}
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
		for (; count > 0; --count, ++data) {
			data->v[0] = (*value++ != 0.0f);
			data->v[1] = (*value++ != 0.0f);
			data->v[2] = (*value++ != 0.0f);
		}
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->size < location + count - uniform->location) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (uniform->type) {
	case GL_FLOAT_VEC4:
		for (; count > 0; --count, ++data) {
			data->v[0] = *value++;
			data->v[1] = *value++;
			data->v[2] = *value++;
			data->v[3] = *value++;
		}
		break;

	case GL_BOOL_VEC4:
		for (; count > 0; --count, ++data) {
			data->v[0] = (*value++ != 0.0f);
			data->v[1] = (*value++ != 0.0f);
			data->v[2] = (*value++ != 0.0f);
			data->v[3] = (*value++ != 0.0f);
		}
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->size < location + count - uniform->location) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
	case GL_INT_VEC3:
	case GL_INT_VEC2:
	case GL_INT:
	case GL_SAMPLER_2D:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_CUBE:
		for (; count > 0; --count, ++data) {
			data->v[0] = *value++;
		}
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC2:
	case GL_BOOL:
		for (; count > 0; --count, ++data) {
			data->v[0] = (*value++ != 0.0f);
		}
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform2iv (GLint location, GLsizei count, const GLint *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->size < location + count - uniform->location) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
	case GL_INT_VEC3:
	case GL_INT_VEC2:
		for (; count > 0; --count, ++data) {
			data->v[0] = *value++;
			data->v[1] = *value++;
		}
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC2:
		for (; count > 0; --count, ++data) {
			data->v[0] = (*value++ != 0.0f);
			data->v[1] = (*value++ != 0.0f);
		}
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform3iv (GLint location, GLsizei count, const GLint *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->size < location + count - uniform->location) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
	case GL_INT_VEC3:
		for (; count > 0; --count, ++data) {
			data->v[0] = *value++;
			data->v[1] = *value++;
			data->v[2] = *value++;
		}
		break;

	case GL_BOOL_VEC4:
	case GL_BOOL_VEC3:
		for (; count > 0; --count, ++data) {
			data->v[0] = (*value++ != 0.0f);
			data->v[1] = (*value++ != 0.0f);
			data->v[2] = (*value++ != 0.0f);
		}
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniform4iv (GLint location, GLsizei count, const GLint *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->size < location + count - uniform->location) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	switch (uniform->type) {
	case GL_INT_VEC4:
		for (; count > 0; --count, ++data) {
			data->v[0] = *value++;
			data->v[1] = *value++;
			data->v[2] = *value++;
			data->v[3] = *value++;
		}
		break;

	case GL_BOOL_VEC4:
		for (; count > 0; --count, ++data) {
			data->v[0] = (*value++ != 0.0f);
			data->v[1] = (*value++ != 0.0f);
			data->v[2] = (*value++ != 0.0f);
			data->v[3] = (*value++ != 0.0f);
		}
		break;

	default:
		GlesRecordInvalidOperation(state);
		return;
	}
}

GL_API void GL_APIENTRY glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->type != GL_FLOAT_MAT2 							||
		(location - uniform->location) % 2 != 0 				||
		uniform->size < (location - uniform->location) / 2 + count) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	if (transpose) {
		for (; count > 0; data += 2, --count, value += 4) {
			data[0].v[0] = value[0];
			data[1].v[0] = value[1];
			data[0].v[1] = value[2];
			data[1].v[1] = value[3];
		}
	} else {
		for (; count > 0; data += 2, --count, value += 4) {
			data[0].v[0] = value[0];
			data[0].v[1] = value[1];
			data[1].v[0] = value[2];
			data[1].v[1] = value[3];
		}
	}
}

GL_API void GL_APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->type != GL_FLOAT_MAT3 							||
		(location - uniform->location) % 3 != 0 				||
		uniform->size < (location - uniform->location) / 3 + count) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	if (transpose) {
		for (; count > 0; data += 3, --count, value += 9) {
			data[0].v[0] = value[0];
			data[1].v[0] = value[1];
			data[2].v[0] = value[2];
			data[0].v[1] = value[3];
			data[1].v[1] = value[4];
			data[2].v[1] = value[5];
			data[0].v[2] = value[6];
			data[1].v[2] = value[7];
			data[2].v[2] = value[8];
		}
	} else {
		for (; count > 0; data += 3, --count, value += 9) {
			data[0].v[0] = value[0];
			data[0].v[1] = value[1];
			data[0].v[2] = value[2];
			data[1].v[0] = value[3];
			data[1].v[1] = value[4];
			data[1].v[2] = value[5];
			data[2].v[0] = value[6];
			data[2].v[1] = value[7];
			data[2].v[2] = value[8];
		}
	}
}

GL_API void GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
	State * state = GLES_GET_STATE();
	GLuint program = state->program;
	Program * programObject = GlesGetProgramObject(state, program);
	const ShaderVariable * uniform = GetUniform(state, programObject, location);
	Vec4f * data = &programObject->uniformData[location];
	
	if (!uniform) {
		return;
	}
	
	if (uniform->type != GL_FLOAT_MAT4 							||
		(location - uniform->location) % 4 != 0 				||
		uniform->size < (location - uniform->location) / 4 + count) {
		GlesRecordInvalidOperation(state);
		return;
	}
	
	if (transpose) {
		for (; count > 0; data += 4, --count, value += 16) {
			data[0].v[0] = value[ 0];
			data[1].v[0] = value[ 1];
			data[2].v[0] = value[ 2];
			data[3].v[0] = value[ 3];
			data[0].v[1] = value[ 4];
			data[1].v[1] = value[ 5];
			data[2].v[1] = value[ 6];
			data[3].v[1] = value[ 7];
			data[0].v[2] = value[ 8];
			data[1].v[2] = value[ 9];
			data[2].v[2] = value[10];
			data[3].v[2] = value[11];
			data[0].v[3] = value[12];
			data[1].v[3] = value[13];
			data[2].v[3] = value[14];
			data[3].v[3] = value[15];
		}
	} else {
		for (; count > 0; data += 4, --count, value += 16) {
			data[0].v[0] = value[ 0];
			data[0].v[1] = value[ 1];
			data[0].v[2] = value[ 2];
			data[0].v[3] = value[ 3];
			data[1].v[0] = value[ 4];
			data[1].v[1] = value[ 5];
			data[1].v[2] = value[ 6];
			data[1].v[3] = value[ 7];
			data[2].v[0] = value[ 8];
			data[2].v[1] = value[ 9];
			data[2].v[2] = value[10];
			data[2].v[3] = value[11];
			data[3].v[0] = value[12];
			data[3].v[1] = value[13];
			data[3].v[2] = value[14];
			data[3].v[3] = value[15];
		}
	}
}

