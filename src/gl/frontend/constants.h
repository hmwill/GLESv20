#ifndef GLES_FRONTEND_CONSTANTS_H
#define GLES_FRONTEND_CONSTANTS_H 1

/*
** ==========================================================================
**
** $Id: constants.h 60 2007-09-18 01:16:07Z hmwill $			
** 
** Shading Language Constant Values
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

#include "frontend/types.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/


/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

struct MemoryPool;

typedef union Constant {
	GLuint		samplerValue;				/* texture sampler id		*/
	GLboolean	boolValue[4];				/* boolean value			*/
	GLint		intValue[4];				/* integer value			*/
	GLfloat		floatValue[4];				/* floating point value		*/
} Constant;

/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

Constant * GlesCreateSamplerConstant(struct MemoryPool * pool, GLuint value);
Constant * GlesCreateBoolConstant(struct MemoryPool * pool, const GLboolean values[]);
Constant * GlesCreateIntConstant(struct MemoryPool * pool, const GLint values[]);
Constant * GlesCreateFloatConstant(struct MemoryPool * pool, const GLfloat values[]);

Constant * GlesConvertConstant(struct MemoryPool * pool, Constant * value, TypeValue src, TypeValue dst);
Constant * GlesSwizzleConstant(struct MemoryPool * pool, Constant * value, TypeValue type, const GLsizei swizzle[]);

GLboolean GlesCompareConstant(Constant * left, Constant * right, Type * type);
GLuint GlesHashConstant(Constant * value, Type * type);

#endif /* GLES_FRONTEND_CONSTANTS_H */