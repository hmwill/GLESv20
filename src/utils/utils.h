#ifndef TESTUTILS_TESTUTILS_H
#define TESTUTILS_TESTUTILS_H

/*
** ==========================================================================
**
** $Id: utils.h 68 2007-09-26 14:54:07Z hmwill $			
** 
** Utility functions for writing test cases
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-26 07:54:07 -0700 (Wed, 26 Sep 2007) $
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
#include <stdio.h>


typedef enum UtilOptionType {
	UtilOptionNone = 0,
	UtilOptionInt = 1,
	UtilOptionString,
	UtilOptionBoolean,
	UtilOptionFloat
} UtilOptionType;
	
typedef struct {
	const char *		sw;			// command line switch letter
    UtilOptionType 		opttyp;		// option type
    void *				var;		// pointer to variable to receive value
} UtilOption;


GLint UtilGetOpts(GLint argc, char **argv, UtilOption opttable[]);

GLboolean UtilLoadShaderSource(GLuint shader, const char * filename);
GLboolean UtilResetState(void);
void UtilSucceedCompile(const char * source, GLenum shaderType);
void UtilFailCompile(const char * source, GLenum shaderType, const char * errorCode);
void UtilLogCompileError(GLuint shader);

void UtilSucceedCompileLink(const char * vertexSource, const char * fragmentSource);

FILE * UtilFopenPath(const char * filename, const char * mode, const char * path);

GLboolean UtilHasCompileError(GLuint shader, const char * errorCode);

#endif /*TESTUTILS_TESTUTILS_H*/
