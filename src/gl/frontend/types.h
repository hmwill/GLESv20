#ifndef GLES_FRONTEND_TYPES_H
#define GLES_FRONTEND_TYPES_H 1

/*
** ==========================================================================
**
** $Id: types.h 60 2007-09-18 01:16:07Z hmwill $			
** 
** Shading Language Type Information
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

#include "frontend/memory.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

typedef enum TypeValue {
	TypeInvalid		= GL_INVALID_VALUE,
	TypeVoid		= GL_NONE,
	TypeBool 		= GL_BOOL,
	TypeBoolVec2	= GL_BOOL_VEC2,
	TypeBoolVec3	= GL_BOOL_VEC3,
	TypeBoolVec4	= GL_BOOL_VEC4,
	TypeInt			= GL_INT,
	TypeIntVec2		= GL_INT_VEC2,
	TypeIntVec3		= GL_INT_VEC3,
	TypeIntVec4		= GL_INT_VEC4,
	TypeFloat		= GL_FLOAT,
	TypeFloatVec2	= GL_FLOAT_VEC2,
	TypeFloatVec3	= GL_FLOAT_VEC3,
	TypeFloatVec4	= GL_FLOAT_VEC4,
	TypeFloatMat2	= GL_FLOAT_MAT2,
	TypeFloatMat3	= GL_FLOAT_MAT3,
	TypeFloatMat4	= GL_FLOAT_MAT4,
	TypeSampler2D	= GL_SAMPLER_2D,
	TypeSampler3D	= GL_SAMPLER_3D,
	TypeSamplerCube	= GL_SAMPLER_CUBE,
	TypeArray,
	TypeStruct,
	TypeFunction
} TypeValue;

typedef enum Precision {
	PrecisionUndefined,
	PrecisionLow,
	PrecisionMedium,
	PrecisionHigh
} Precision;

typedef enum ParameterDir {
	ParameterDirIn	 	= 1,
	ParameterDirOut 	= 2,
	ParameterDirInOut 	= 3
} ParameterDir;

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

typedef union Type Type;
typedef struct Parameter Parameter;
typedef struct Field Field;

struct Scope;
struct Log;
struct SymbolField;

struct TypeBase {
	TypeValue	kind;						/* type descriminator value */
	GLsizeiptr	size;						/* size in 4-element words	*/
	GLsizei		elements 	: 6;			/* for basic types			*/
	Precision	prec		: 2;			/* for basic types			*/
};

struct TypeArray {
	struct TypeBase	base;					
	GLsizeiptr	elements;					/* number of elements		*/
	Type *		elementType;				/* element sub-type			*/
};

struct Field {
	Type *		type;						/* type of field			*/
	GLsizeiptr	offset;						/* offset in structure		*/
	struct SymbolField	*	symbol;			/* symbol table entry		*/
};

struct TypeStruct {
	struct TypeBase	base;	
	struct Scope *	symbols;				/* table of fields			*/
	GLsizei			numFields;				/* number of fields			*/
	Field *			fields;					/* array of fields			*/
};

struct Parameter {
	Type *			type;					/* parameter type			*/
	ParameterDir	direction;				/* directionality			*/
};

struct TypeFunction {
	struct TypeBase	base;			
	GLsizei			numParams;				/* number of parameters		*/
	Type *			returnType;				/* function return type		*/
	Parameter 		parameters[0];			/* function parameters		*/
};

union Type {
	struct TypeBase		base;
	struct TypeArray	array;
	struct TypeStruct	structure;
	struct TypeFunction	func;
};

struct Block;

/**
 * List of blocks; contains pointers to first and last block.
 * 
 * This structure should really be defined in a separate file.
 */
typedef struct BlockList {
	struct Block *		head;			/**< pointer to first block			*/
	struct Block *		tail;			/**< pointer to last block			*/
} BlockList;

/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

static GLES_INLINE GLboolean GlesTypeIsPrimitive(const Type * type) {
	switch (type->base.kind) {
	case TypeBool:
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
	case TypeInt:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
	case TypeFloat:
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
	case TypeSampler2D:
	case TypeSampler3D:
	case TypeSamplerCube:
		return GL_TRUE;
		
	default:
		return GL_FALSE;
	}
}

Type * GlesBasicType(TypeValue type, Precision prec);
Type * GlesVectorType(TypeValue type, Precision prec, GLsizei dim);
Type * GlesMatrixType(TypeValue type, Precision prec, GLsizei dim);
Type * GlesElementType(Type * type);

Type * GlesTypeArrayCreate(MemoryPool * pool, Type * baseType, GLsizei size);
Type * GlesTypeStructCreate(MemoryPool * pool);

Type * GlesTypeFunctionCreate(MemoryPool * pool, Type * returnType, 
							  GLsizei numParams);

GLboolean GlesTypeFunctionIsOverload(const Type * first, const Type * second);

GLboolean GlesTypeFunctionReturnTypeMatches(const Type * first, const Type * second);
GLboolean GlesTypeFunctionParamQualifiersMatch(const Type * first, const Type * second);
GLboolean GlesTypeFunctionParamSizesMatch(const Type * first, const Type * second);

GLboolean GlesTypeMatches(const Type * first, const Type * second);

#if GLES_DEBUG
void GlesTypePrint(const Type * type);
#endif

#endif /* GLES_FRONTEND_TYPES_H */