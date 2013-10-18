/*
** ==========================================================================
**
** $Id: shader.c 69 2007-09-27 06:15:41Z hmwill $			
** 
** Utility functions around shaders for writing test cases
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-26 23:15:41 -0700 (Wed, 26 Sep 2007) $
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


#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CUnit/Basic.h"


/** Environment variable specifying search path for shader source files */
#define ENV_SHADER_PATH "SHADER_PATH"
#define LINE_SEPARATOR	"\n"


static const char * ShaderPath(void) {
	const char * path = getenv(ENV_SHADER_PATH);
	
	if (path == NULL) {
		return ".";
	} else {
		return path;
	}
}

/**
 * Load a shader source file as shader source into the current shader.
 *  
 * @param shader
 * 			the identifier of the shader to populate
 * @param filename
 * 			the filename of the shader source code
 * 
 * @return	true if successful
 */
GLboolean UtilLoadShaderSource(GLuint shader, const char * filename) {
	const char * shaderPath = ShaderPath();
	FILE * source = UtilFopenPath(filename, "r", shaderPath);
	size_t length;
	char * buffer;
	
	if (!source) {
		fprintf(stderr, "Could not find the shader file \"%s\" in the SHADER_PATH [%s]\n",
				filename,shaderPath);
		return GL_FALSE;
	}
	
	/* determine size of shader file */
	fseek(source, 0, SEEK_END);
	length = ftell(source);
	fseek(source, 0, SEEK_SET);
	
	buffer = malloc(length + 1);
	
	if (!buffer) {
		fprintf(stderr, "Fatal error: Out of memory in %s (%d)\n", __FILE__, __LINE__);
		fclose(source);
		return GL_FALSE;
	}
	
	if (fread(buffer, 1, length, source) != length) {
		fprintf(stderr, "Error reading shader source file %s\n", filename);
	} else {
		buffer[length] = '\0';
		glShaderSource(shader, 1, (const char **) &buffer, NULL);
	}
	
	free(buffer);
	fclose(source);
	
	return GL_TRUE;
}


/**
 * Determine if the given shader has a specific error code in its compile log.
 * 
 * @param	shader		the shader to check
 * @param	errorCode	the specific error message code to look for
 * 
 * @return	GL_TRUE if the compile log for the specified shader contains the error code.
 */
GLboolean UtilHasCompileError(GLuint shader, const char * errorCode) {
	GLint length;
	GLsizei returnedLength;
	char *buffer, *log, *line;
	GLboolean result = GL_FALSE;
	size_t size = strlen(errorCode);

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	buffer = malloc(length);
	
	if (!buffer) {
		fprintf(stderr, "Fatal error: Out of memory in %s (%d)\n", __FILE__, __LINE__);
		return GL_FALSE;
	}
	
	glGetShaderInfoLog(shader, length, &returnedLength, buffer);
	
	if (length != returnedLength + 1) {
		fprintf(stderr, "API inconsistency while retrieving shader compile log: %d != %d\n", length, returnedLength);
		free(buffer);
		return GL_FALSE;
	}
	
	for (log = buffer; (line = strsep(&log, LINE_SEPARATOR)) != NULL; ) {
		if (!strncmp(line, errorCode, size)) {
			result = GL_TRUE;
			break;
		}
	}
	
	free(buffer);
	return result;
}

/**
 * Emit compile log error to test framework log.
 * 
 * @param	shader		the shader to check
 */
void UtilLogCompileError(GLuint shader) {
	GLint length;
	GLsizei returnedLength;
	char *buffer, *log, *line;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	buffer = malloc(length);
	
	if (!buffer) {
		fprintf(stderr, "Fatal error: Out of memory in %s (%d)\n", __FILE__, __LINE__);
		return;
	}
	
	glGetShaderInfoLog(shader, length, &returnedLength, buffer);
	
	for (log = buffer; (line = strsep(&log, LINE_SEPARATOR)) != NULL; ) {
		CU_assertImplementation(CU_FALSE, __LINE__, line, __FILE__, "", CU_FALSE);
		break;
	}
	
	free(buffer);
}

void UtilDumpIntermediate(GLuint shader) {
	GLint length;
	GLsizei returnedLength;
	char *buffer, *log, *line;

	glGetShaderiv(shader, GL_SHADER_INTERMEDIATE_LENGTH_VIN, &length);
	buffer = malloc(length);
	
	if (!buffer) {
		fprintf(stderr, "Fatal error: Out of memory in %s (%d)\n", __FILE__, __LINE__);
		return;
	}
	
	glGetShaderIntermediateVIN(shader, length, &returnedLength, buffer);

	fprintf(stderr, "\n");
	
	for (log = buffer; (line = strsep(&log, LINE_SEPARATOR)) != NULL; ) {
		fprintf(stdout, "%s\n", line);
	}
	
	free(buffer);	
}

/**
 * Attempt to successfully compile the given shader source for the given 
 * shader type.
 * 
 * @param source		Shader source file to compiler
 * @param shaderType 	GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 */
void UtilSucceedCompile(const char * source, GLenum shaderType) {
	GLuint shader;
	GLint compileStatus;
	
	/* ensure we are entering clean */
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	shader = glCreateShader(shaderType);
	CU_ASSERT(shader != 0);
	
	CU_ASSERT(UtilLoadShaderSource(shader, source));
	CU_ASSERT(glGetError() == GL_NO_ERROR);

	glCompileShader(shader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	
	if (compileStatus == GL_TRUE) {
		fprintf(stdout, "\n=====================================\n");
		fprintf(stdout, "Compilation result for %s\n", source);
		fprintf(stdout, "=====================================\n");
		UtilDumpIntermediate(shader);
		CU_PASS("Compilation succeeded.")
	} else {
		UtilLogCompileError(shader);
	}
		
	glDeleteShader(shader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
}

/**
 * Attempt to compile the given shader source for the given shader type. The 
 * compilation is expected to fail.
 * 
 * @param source		Shader source file to compiler
 * @param shaderType 	GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 * @param errorCode		the expected compile error code
 */
void UtilFailCompile(const char * source, GLenum shaderType,
	const char * errorCode) {
	GLuint shader;
	GLint compileStatus;
	
	/* ensure we are entering clean */
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	shader = glCreateShader(shaderType);
	CU_ASSERT(shader != 0);
	
	CU_ASSERT(UtilLoadShaderSource(shader, source));
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	
	glCompileShader(shader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	CU_ASSERT(compileStatus == GL_FALSE);
	CU_ASSERT(UtilHasCompileError(shader, errorCode));
	
	glDeleteShader(shader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
}

/**
 * Compile and link a pair of vertex and fragment shader. The compilation
 * and linking is expected to succeed.
 *
 * @param vertexSource		Vertex shader source file to compiler
 * @param fragmentSource	Fragment shader source file to compiler
 */
void UtilSucceedCompileLink(const char * vertexSource, 
	const char * fragmentSource) {
	
	GLuint vertexShader, fragmentShader;
	GLuint program;
	GLint compileStatus, linkStatus;
	
	/* Create the shader objects */
	
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	CU_ASSERT(vertexShader != 0);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	CU_ASSERT(fragmentShader != 0);
	
	/* Create the program object */
	
	program = glCreateProgram();
	CU_ASSERT(program != 0);
	glAttachShader(program, vertexShader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	glAttachShader(program, fragmentShader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	
	/* Load the shader source files */
	
	CU_ASSERT(UtilLoadShaderSource(vertexShader, vertexSource));
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	CU_ASSERT(UtilLoadShaderSource(fragmentShader, fragmentSource));
	CU_ASSERT(glGetError() == GL_NO_ERROR);

	/* Compile the two shaders */
	
	glCompileShader(vertexShader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	glCompileShader(fragmentShader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);

	if (compileStatus == GL_TRUE) {
		fprintf(stdout, "\n=====================================\n");
		fprintf(stdout, "Compilation result for %s\n", vertexSource);
		fprintf(stdout, "=====================================\n");
		UtilDumpIntermediate(vertexShader);
	} else {
		UtilLogCompileError(vertexShader);
	}

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
	
	if (compileStatus == GL_TRUE) {
		fprintf(stdout, "\n=====================================\n");
		fprintf(stdout, "Compilation result for %s\n", fragmentSource);
		fprintf(stdout, "=====================================\n");
		UtilDumpIntermediate(fragmentShader);
	} else {
		UtilLogCompileError(fragmentShader);
	}

	/* Link the program */
	
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	CU_ASSERT(linkStatus == GL_FALSE);
	
	glLinkProgram(program);
	
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	CU_ASSERT(linkStatus == GL_TRUE);
	
	/* Clean up */
	
	glDeleteShader(vertexShader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	glDeleteShader(fragmentShader);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
	glDeleteProgram(program);
	CU_ASSERT(glGetError() == GL_NO_ERROR);
}
