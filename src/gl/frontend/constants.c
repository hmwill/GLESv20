/*
** ==========================================================================
**
** $Id: constants.c 60 2007-09-18 01:16:07Z hmwill $			
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

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "frontend/constants.h"
#include "frontend/memory.h"

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

Constant * GlesCreateSamplerConstant(struct MemoryPool * pool, GLuint value) {
	Constant * result = GlesMemoryPoolAllocate(pool, sizeof(Constant));
	
	result->samplerValue = value;
	
	return result;
}

Constant * GlesCreateBoolConstant(struct MemoryPool * pool, const GLboolean values[]) {
	Constant * result = GlesMemoryPoolAllocate(pool, sizeof(Constant));
	
	result->boolValue[0] = values[0];
	result->boolValue[1] = values[1];
	result->boolValue[2] = values[2];
	result->boolValue[3] = values[3];
	
	return result;
}

Constant * GlesCreateIntConstant(struct MemoryPool * pool, const GLint values[]) {
	Constant * result = GlesMemoryPoolAllocate(pool, sizeof(Constant));
	
	result->intValue[0] = values[0];
	result->intValue[1] = values[1];
	result->intValue[2] = values[2];
	result->intValue[3] = values[3];
	
	return result;
}

Constant * GlesCreateFloatConstant(struct MemoryPool * pool, const GLfloat values[]) {
	Constant * result = GlesMemoryPoolAllocate(pool, sizeof(Constant));
	
	result->floatValue[0] = values[0];
	result->floatValue[1] = values[1];
	result->floatValue[2] = values[2];
	result->floatValue[3] = values[3];
	
	return result;
}

Constant * GlesConvertConstant(struct MemoryPool * pool, Constant * value, TypeValue src, TypeValue dst) {
	Constant * result;
	GLsizei index;
	
	if (src == dst) {
		return value;
	}
	
	result = GlesMemoryPoolAllocate(pool, sizeof(Constant));
	
	switch (src) {
	case TypeBool:
		switch (dst) {
		case TypeInt:
			for (index = 0; index < 4; ++index) {
				result->intValue[index] = value->boolValue[index];
			}
			
			break;
			
		case TypeFloat:
			for (index = 0; index < 4; ++index) {
				result->floatValue[index] = value->boolValue[index];
			}
			
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
			break;
		}
		
		break;
		
	case TypeInt:
		switch (dst) {
		case TypeBool:
			for (index = 0; index < 4; ++index) {
				result->boolValue[index] = value->intValue[index] != 0;
			}
			
			break;
			
		case TypeFloat:
			for (index = 0; index < 4; ++index) {
				result->floatValue[index] = (GLint) value->intValue[index];
			}
			
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
			break;
		}
		
		break;
		
	case TypeFloat:
		switch (dst) {
		case TypeBool:
			for (index = 0; index < 4; ++index) {
				result->boolValue[index] = value->floatValue[index] != 0.0f;
			}
			
			break;
			
		case TypeInt:
			for (index = 0; index < 4; ++index) {
				result->intValue[index] = (GLint) value->floatValue[index];
			}
			
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
			break;
		}
		
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
		break;
	}
	
	return result;
}

Constant * GlesSwizzleConstant(struct MemoryPool * pool, Constant * value, TypeValue type, const GLsizei swizzle[]) {
	Constant * result;
	GLsizei index;
	
	result = GlesMemoryPoolAllocate(pool, sizeof(Constant));
	
	switch (type) {
	case TypeBool:
		for (index = 0; index < 4; ++index) {
			result->boolValue[index] = value->boolValue[swizzle[index]];
		}
		
		break;
		
	case TypeInt:
		for (index = 0; index < 4; ++index) {
			result->intValue[index] = value->intValue[swizzle[index]];
		}
		
		break;
		
	case TypeFloat:
		for (index = 0; index < 4; ++index) {
			result->floatValue[index] = value->floatValue[swizzle[index]];
		}
		
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
		break;
	}
	
	return result;
}

static GLboolean EqualStruct(Constant * left, Constant * right, Type * type) {

	GLES_ASSERT(type->base.kind == TypeStruct);
	
	GLsizei numFields = type->structure.numFields, index;
	Field * fields = type->structure.fields;
	
	GLboolean result = GL_TRUE;
		
	for (index = 0; index < numFields; ++index) {
		result &=
			GlesCompareConstant(left + fields[index].offset, 
								right + fields[index].offset, 
								fields[index].type);
	}			
	
	return result;
}

static GLboolean EqualArray(Constant * left, Constant * right, Type * type) {
	
	GLsizei elements = type->array.elements, index;
	
	GLES_ASSERT(type->base.kind = TypeArray);
	
	GLboolean result = GL_TRUE;
	
	for (index = 0; index < elements; ++index) {
		result &=
			GlesCompareConstant(left + type->array.elementType->base.size,
								right + type->array.elementType->base.size,
								type->array.elementType);
	}
				
	return result;
}

static GLboolean EqualMatrix(Constant * left, Constant * right, Type * type) {
	GLsizei row, column;
	GLsizei elements = type->base.elements;

	GLboolean result = GL_TRUE;
	
	for (column = 0; column < elements; ++column) {
		for (row = 0; row < elements; ++row) {
			result &=
				left[column].floatValue[row] == right[column].floatValue[row];
		}					
	}
	
	return result;
}

static GLboolean EqualVectorScalar(Constant * left, Constant * right, Type * type) {
	GLsizei index;
	
	if (type->base.kind == TypeBool 	||
		type->base.kind == TypeBoolVec2 ||
		type->base.kind == TypeBoolVec3 ||
		type->base.kind == TypeBoolVec4) {
			
		GLboolean result =
			left[0].boolValue[0] == right[0].boolValue[0];
			
		for (index = 1; index < type->base.elements; ++index) {
			result &=
				left[0].boolValue[index] == right[0].boolValue[index];
		}
		
		return result;
	} else if (type->base.kind == TypeInt	  	||
			   type->base.kind == TypeIntVec2 	||
			   type->base.kind == TypeIntVec3 	||
			   type->base.kind == TypeIntVec4 	||
			   type->base.kind == TypeSampler2D	||
			   type->base.kind == TypeSampler3D ||
			   type->base.kind == TypeSamplerCube) {
			   	
		GLboolean result =
			left[0].intValue[0] == right[0].intValue[0];
			
		for (index = 1; index < type->base.elements; ++index) {
			result &=
				left[0].intValue[index] == right[0].intValue[index];
		}
		
		return result;
	} else {
		GLES_ASSERT(type->base.kind == TypeFloat	 ||
					type->base.kind == TypeFloatVec2 ||
					type->base.kind == TypeFloatVec3 ||
					type->base.kind == TypeFloatVec4);
					
		GLboolean result =
			left[0].floatValue[0] == right[0].floatValue[0];
			
		for (index = 1; index < type->base.elements; ++index) {
			result &=
				left[0].floatValue[index] == right[0].floatValue[index];
		}
		
		return result;
	} 
}

/**
 * Compare two constant values of the given type.
 * 
 * @param	left	first constant to compare
 * @param	right	second constant to compare
 * @param	type	common type of the constants
 * 
 * @return	GL_TRUE if the two values match
 */
GLboolean GlesCompareConstant(Constant * left, Constant * right, Type * type) {

	switch (type->base.kind) {
	case TypeStruct:
		return EqualStruct(left, right, type);
		
	case TypeArray:
		return EqualArray(left, right, type);
		
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		return EqualMatrix(left, right, type);
		
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
		
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		
	case TypeBool:
	case TypeInt:
	case TypeFloat:

	case TypeSampler2D:	
	case TypeSampler3D:	
	case TypeSamplerCube:
		return EqualVectorScalar(left, right, type);
		
	default:
		return GL_FALSE;
	}
}

static GLES_INLINE GLuint HashValue(GLuint oldValue, GLuint newValue) {
	return ((oldValue << 7) + newValue) ^ (oldValue >> 15); 
}

static GLuint HashStruct(Constant * value, Type * type) {

	GLES_ASSERT(type->base.kind == TypeStruct);
	
	GLsizei numFields = type->structure.numFields, index;
	Field * fields = type->structure.fields;
	
	GLuint result = 0;
		
	for (index = 0; index < numFields; ++index) {
		result = HashValue(result,
						   GlesHashConstant(value + fields[index].offset, 
											fields[index].type));
	}			
	
	return result;
}

static GLuint HashArray(Constant * value, Type * type) {
	
	GLsizei elements = type->array.elements, index;
	
	GLES_ASSERT(type->base.kind = TypeArray);
	
	GLuint result = 0;
	
	for (index = 0; index < elements; ++index) {
		result = HashValue(result, 
						   GlesHashConstant(value + type->array.elementType->base.size,
										 	type->array.elementType));
	}
				
	return result;
}

static GLuint HashMatrix(Constant * value, Type * type) {
	GLsizei row, column;
	GLsizei elements = type->base.elements;

	GLuint result = 0;
	
	for (column = 0; column < elements; ++column) {
		for (row = 0; row < elements; ++row) {
			result = HashValue(result, (GLuint) value[column].floatValue[row]);
		}					
	}
	
	return result;
}

static GLuint HashVectorScalar(Constant * value, Type * type) {
	GLsizei index;
	
	if (type->base.kind == TypeBool 	||
		type->base.kind == TypeBoolVec2 ||
		type->base.kind == TypeBoolVec3 ||
		type->base.kind == TypeBoolVec4) {
			
		GLuint result = (GLuint) value[0].boolValue[0];
			
		for (index = 1; index < type->base.elements; ++index) {
			result = HashValue(result, (GLuint) value[0].boolValue[index]);
		}
		
		return result;
	} else if (type->base.kind == TypeInt	  	||
			   type->base.kind == TypeIntVec2 	||
			   type->base.kind == TypeIntVec3 	||
			   type->base.kind == TypeIntVec4 	||
			   type->base.kind == TypeSampler2D	||
			   type->base.kind == TypeSampler3D ||
			   type->base.kind == TypeSamplerCube) {
			   	
		GLboolean result = (GLuint) value[0].intValue[0];
			
		for (index = 1; index < type->base.elements; ++index) {
			result = HashValue(result, (GLuint) value[0].intValue[index]);
		}
		
		return result;
	} else {
		GLES_ASSERT(type->base.kind == TypeFloat	 ||
					type->base.kind == TypeFloatVec2 ||
					type->base.kind == TypeFloatVec3 ||
					type->base.kind == TypeFloatVec4);
					
		GLboolean result = (GLuint) value[0].floatValue[0];
			
		for (index = 1; index < type->base.elements; ++index) {
			result = HashValue(result, (GLuint) value[0].floatValue[index]);
		}
		
		return result;
	} 
}

/**
 * Compute a hash value for a constant of the given type.
 * 
 * @param	value	first constant to compare
 * @param	type	common type of the constants
 * 
 * @return	GL_TRUE if the two values match
 */
GLuint GlesHashConstant(Constant * value, Type * type) {

	switch (type->base.kind) {
	case TypeStruct:
		return HashStruct(value, type);
		
	case TypeArray:
		return HashArray(value, type);
		
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		return HashMatrix(value, type);
		
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
		
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		
	case TypeBool:
	case TypeInt:
	case TypeFloat:

	case TypeSampler2D:	
	case TypeSampler3D:	
	case TypeSamplerCube:
		return HashVectorScalar(value, type);
		
	default:
		return 0;
	}
}


