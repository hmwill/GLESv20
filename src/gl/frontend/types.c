/*
** ==========================================================================
**
** $Id: types.c 60 2007-09-18 01:16:07Z hmwill $
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

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "frontend/types.h"
#include "frontend/memory.h"
#include "gl/state.h"

/*
** --------------------------------------------------------------------------
** Module local data
** --------------------------------------------------------------------------
*/

static struct TypeBase
	BasicTypeVoid 			= { TypeVoid,		 0, 0, 0 				},
	BasicTypeBool 			= { TypeBool,		 1, 1, 0 				},
	BasicTypeBoolVec2		= { TypeBoolVec2,	 1, 2, 0 				},
	BasicTypeBoolVec3		= { TypeBoolVec3,	 1, 3, 0 				},
	BasicTypeBoolVec4		= { TypeBoolVec4,	 1, 4, 0 				},
	BasicTypeInt 			= { TypeInt,		 1, 1, 0				},
	BasicTypeIntVec2		= { TypeIntVec2,	 1, 2, 0 				},
	BasicTypeIntVec3		= { TypeIntVec3,	 1, 3, 0			 	},
	BasicTypeIntVec4		= { TypeIntVec4,	 1, 4, 0 				},
	BasicTypeFloat			= { TypeFloat,		 1, 1, 0			 	},
	BasicTypeFloatVec2		= { TypeFloatVec2,	 1, 2, 0			 	},
	BasicTypeFloatVec3		= { TypeFloatVec3,	 1, 3, 0			 	},
	BasicTypeFloatVec4		= { TypeFloatVec4,	 1, 4, 0 				},
	BasicTypeFloatMat2		= { TypeFloatMat2,	 2, 2, 0			 	},
	BasicTypeFloatMat3		= { TypeFloatMat3,	 3, 3, 0			 	},
	BasicTypeFloatMat4		= { TypeFloatMat4,	 4, 4, 0			 	},
	BasicTypeIntL 			= { TypeInt,		 1, 1, PrecisionLow 	},
	BasicTypeIntVec2L		= { TypeIntVec2,	 1, 2, PrecisionLow 	},
	BasicTypeIntVec3L		= { TypeIntVec3,	 1, 3, PrecisionLow 	},
	BasicTypeIntVec4L		= { TypeIntVec4,	 1, 4, PrecisionLow 	},
	BasicTypeFloatL			= { TypeFloat,		 1, 1, PrecisionLow 	},
	BasicTypeFloatVec2L		= { TypeFloatVec2,	 1, 2, PrecisionLow 	},
	BasicTypeFloatVec3L		= { TypeFloatVec3,	 1, 3, PrecisionLow 	},
	BasicTypeFloatVec4L		= { TypeFloatVec4,	 1, 4, PrecisionLow 	},
	BasicTypeFloatMat2L		= { TypeFloatMat2,	 2, 2, PrecisionLow 	},
	BasicTypeFloatMat3L		= { TypeFloatMat3,	 3, 3, PrecisionLow 	},
	BasicTypeFloatMat4L		= { TypeFloatMat4,	 4, 4, PrecisionLow 	},
	BasicTypeIntM 			= { TypeInt,		 1, 1, PrecisionMedium 	},
	BasicTypeIntVec2M		= { TypeIntVec2,	 1, 2, PrecisionMedium 	},
	BasicTypeIntVec3M		= { TypeIntVec3,	 1, 3, PrecisionMedium 	},
	BasicTypeIntVec4M		= { TypeIntVec4,	 1, 4, PrecisionMedium 	},
	BasicTypeFloatM 		= { TypeFloat,		 1, 1, PrecisionMedium 	},
	BasicTypeFloatVec2M		= { TypeFloatVec2,	 1, 2, PrecisionMedium 	},
	BasicTypeFloatVec3M		= { TypeFloatVec3,	 1, 3, PrecisionMedium 	},
	BasicTypeFloatVec4M		= { TypeFloatVec4,	 1, 4, PrecisionMedium 	},
	BasicTypeFloatMat2M		= { TypeFloatMat2,	 2, 2, PrecisionMedium 	},
	BasicTypeFloatMat3M		= { TypeFloatMat3,	 3, 3, PrecisionMedium 	},
	BasicTypeFloatMat4M		= { TypeFloatMat4,	 4, 4, PrecisionMedium 	},
	BasicTypeIntH 			= { TypeInt,		 1, 1, PrecisionHigh 	},
	BasicTypeIntVec2H		= { TypeIntVec2,	 1, 2, PrecisionHigh 	},
	BasicTypeIntVec3H		= { TypeIntVec3,	 1, 3, PrecisionHigh 	},
	BasicTypeIntVec4H		= { TypeIntVec4,	 1, 4, PrecisionHigh 	},
	BasicTypeFloatH			= { TypeFloat,		 1, 1, PrecisionHigh 	},
	BasicTypeFloatVec2H		= { TypeFloatVec2,	 1, 2, PrecisionHigh 	},
	BasicTypeFloatVec3H		= { TypeFloatVec3,	 1, 3, PrecisionHigh 	},
	BasicTypeFloatVec4H		= { TypeFloatVec4,	 1, 4, PrecisionHigh 	},
	BasicTypeFloatMat2H		= { TypeFloatMat2,	 2, 2, PrecisionHigh 	},
	BasicTypeFloatMat3H		= { TypeFloatMat3,	 3, 3, PrecisionHigh 	},
	BasicTypeFloatMat4H		= { TypeFloatMat4,	 4, 4, PrecisionHigh 	},
	BasicTypeSampler2DL		= { TypeSampler2D,	 1, 1, PrecisionLow 	},
	BasicTypeSampler3DL		= { TypeSampler3D,	 1, 1, PrecisionLow 	},
	BasicTypeSamplerCubeL 	= { TypeSamplerCube, 1, 1, PrecisionLow		},
	BasicTypeSampler2DM		= { TypeSampler2D,	 1, 1, PrecisionMedium 	},
	BasicTypeSampler3DM		= { TypeSampler3D,	 1, 1, PrecisionMedium 	},
	BasicTypeSamplerCubeM 	= { TypeSamplerCube, 1, 1, PrecisionMedium	},
	BasicTypeSampler2DH		= { TypeSampler2D,	 1, 1, PrecisionHigh 	},
	BasicTypeSampler3DH		= { TypeSampler3D,	 1, 1, PrecisionHigh 	},
	BasicTypeSamplerCubeH 	= { TypeSamplerCube, 1, 1, PrecisionHigh	};
	
/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static GLboolean TypeMatchesIgnoreSize(const Type * first, const Type * second) {
	
	if (first->base.kind == TypeStruct) {
		return first == second;
	} else if (first->base.kind == TypeArray) {
		return 
			second->base.kind == TypeArray &&
			TypeMatchesIgnoreSize(first->array.elementType, second->array.elementType);
	} else {
		return first->base.kind == second->base.kind;
	}
}

GLboolean GlesTypeMatches(const Type * first, const Type * second) {
	
	if (first->base.kind == TypeStruct) {
		return first == second;
	} else if (first->base.kind == TypeArray) {
		return 
			second->base.kind == TypeArray &&
			first->array.elements == second->array.elements &&
			GlesTypeMatches(first->array.elementType, second->array.elementType);
	} else {
		return first->base.kind == second->base.kind;
	}
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

Type * GlesBasicType(TypeValue type, Precision prec) {
	switch (prec) {
	case PrecisionLow:
		switch (type) {
		case TypeInt:			return (Type *) &BasicTypeIntL;
		case TypeIntVec2:		return (Type *) &BasicTypeIntVec2L;
		case TypeIntVec3:		return (Type *) &BasicTypeIntVec3L;
		case TypeIntVec4:		return (Type *) &BasicTypeIntVec4L;
		case TypeFloat:			return (Type *) &BasicTypeFloatL;
		case TypeFloatVec2:		return (Type *) &BasicTypeFloatVec2L;
		case TypeFloatVec3:		return (Type *) &BasicTypeFloatVec3L;
		case TypeFloatVec4:		return (Type *) &BasicTypeFloatVec4L;
		case TypeFloatMat2:		return (Type *) &BasicTypeFloatMat2L;
		case TypeFloatMat3:		return (Type *) &BasicTypeFloatMat3L;
		case TypeFloatMat4:		return (Type *) &BasicTypeFloatMat4L;
		case TypeSampler2D:		return (Type *) &BasicTypeSampler2DL;
		case TypeSampler3D:		return (Type *) &BasicTypeSampler3DL;
		case TypeSamplerCube:	return (Type *) &BasicTypeSamplerCubeL;
			
		default:
			;
		}
		
		break;
		
	case PrecisionMedium:
		switch (type) {
		case TypeInt:			return (Type *) &BasicTypeIntM;
		case TypeIntVec2:		return (Type *) &BasicTypeIntVec2M;
		case TypeIntVec3:		return (Type *) &BasicTypeIntVec3M;
		case TypeIntVec4:		return (Type *) &BasicTypeIntVec4M;
		case TypeFloat:			return (Type *) &BasicTypeFloatM;
		case TypeFloatVec2:		return (Type *) &BasicTypeFloatVec2M;
		case TypeFloatVec3:		return (Type *) &BasicTypeFloatVec3M;
		case TypeFloatVec4:		return (Type *) &BasicTypeFloatVec4M;
		case TypeFloatMat2:		return (Type *) &BasicTypeFloatMat2M;
		case TypeFloatMat3:		return (Type *) &BasicTypeFloatMat3M;
		case TypeFloatMat4:		return (Type *) &BasicTypeFloatMat4M;
		case TypeSampler2D:		return (Type *) &BasicTypeSampler2DM;
		case TypeSampler3D:		return (Type *) &BasicTypeSampler3DM;
		case TypeSamplerCube:	return (Type *) &BasicTypeSamplerCubeM;
			
		default:
			;
		}
		
		break;
		
	case PrecisionHigh:
		switch (type) {
		case TypeInt:			return (Type *) &BasicTypeIntH;
		case TypeIntVec2:		return (Type *) &BasicTypeIntVec2H;
		case TypeIntVec3:		return (Type *) &BasicTypeIntVec3H;
		case TypeIntVec4:		return (Type *) &BasicTypeIntVec4H;
		case TypeFloat:			return (Type *) &BasicTypeFloatH;
		case TypeFloatVec2:		return (Type *) &BasicTypeFloatVec2H;
		case TypeFloatVec3:		return (Type *) &BasicTypeFloatVec3H;
		case TypeFloatVec4:		return (Type *) &BasicTypeFloatVec4H;
		case TypeFloatMat2:		return (Type *) &BasicTypeFloatMat2H;
		case TypeFloatMat3:		return (Type *) &BasicTypeFloatMat3H;
		case TypeFloatMat4:		return (Type *) &BasicTypeFloatMat4H;
		case TypeSampler2D:		return (Type *) &BasicTypeSampler2DH;
		case TypeSampler3D:		return (Type *) &BasicTypeSampler3DH;
		case TypeSamplerCube:	return (Type *) &BasicTypeSamplerCubeH;
			
		default:
			;
		}
		
		break;

	case PrecisionUndefined:
		switch (type) {
		case TypeVoid:			return (Type *) &BasicTypeVoid;
		case TypeBool:			return (Type *) &BasicTypeBool;
		case TypeBoolVec2:		return (Type *) &BasicTypeBoolVec2;
		case TypeBoolVec3:		return (Type *) &BasicTypeBoolVec3;
		case TypeBoolVec4:		return (Type *) &BasicTypeBoolVec4;

		/* these are only valid for constants */
		case TypeInt:			return (Type *) &BasicTypeInt;
		case TypeIntVec2:		return (Type *) &BasicTypeIntVec2;
		case TypeIntVec3:		return (Type *) &BasicTypeIntVec3;
		case TypeIntVec4:		return (Type *) &BasicTypeIntVec4;
		case TypeFloat:			return (Type *) &BasicTypeFloat;
		case TypeFloatVec2:		return (Type *) &BasicTypeFloatVec2;
		case TypeFloatVec3:		return (Type *) &BasicTypeFloatVec3;
		case TypeFloatVec4:		return (Type *) &BasicTypeFloatVec4;
		case TypeFloatMat2:		return (Type *) &BasicTypeFloatMat2;
		case TypeFloatMat3:		return (Type *) &BasicTypeFloatMat3;
		case TypeFloatMat4:		return (Type *) &BasicTypeFloatMat4;
		
		default:
			;							
		}
		
		break;
	}

	return (Type *) 0;
}

Type * GlesVectorType(TypeValue type, Precision prec, GLsizei dim) {
	GLES_ASSERT(type == TypeBool || type == TypeFloat || type == TypeInt);
	GLES_ASSERT(dim >= 1 && dim <= 4);
	
	switch (type) {
	case TypeBool:
		switch (dim) {
		case 1:	return GlesBasicType(TypeBool,     prec);		
		case 2:	return GlesBasicType(TypeBoolVec2, prec);		
		case 3:	return GlesBasicType(TypeBoolVec3, prec);		
		case 4:	return GlesBasicType(TypeBoolVec4, prec);		
		}
		
		break;
		
	case TypeInt:
		switch (dim) {
		case 1:	return GlesBasicType(TypeInt,     prec);		
		case 2:	return GlesBasicType(TypeIntVec2, prec);		
		case 3:	return GlesBasicType(TypeIntVec3, prec);		
		case 4:	return GlesBasicType(TypeIntVec4, prec);		
		}
	
		break;
	
	case TypeFloat:
		switch (dim) {
		case 1:	return GlesBasicType(TypeFloat,     prec);		
		case 2:	return GlesBasicType(TypeFloatVec2, prec);		
		case 3:	return GlesBasicType(TypeFloatVec3, prec);		
		case 4:	return GlesBasicType(TypeFloatVec4, prec);		
		}
	
		break;	
		
	default:
	;
	}
	
	GLES_ASSERT(GL_FALSE);
	return NULL;
}

Type * GlesMatrixType(TypeValue type, Precision prec, GLsizei dim) {
	GLES_ASSERT(type == TypeFloat);
	GLES_ASSERT(dim >= 1 && dim <= 4);
	
	switch (dim) {
	case 2:	return GlesBasicType(TypeFloatMat2, prec);		
	case 3:	return GlesBasicType(TypeFloatMat3, prec);		
	case 4:	return GlesBasicType(TypeFloatMat4, prec);				
	}
	
	GLES_ASSERT(GL_FALSE);
	return NULL;
}

Type * GlesElementType(Type * type) {
	switch (type->base.kind) {
	case TypeBool:			
	case TypeBoolVec2:		
	case TypeBoolVec3:		
	case TypeBoolVec4:		
		return GlesBasicType(TypeBool, type->base.prec);

	/* these are only valid for constants */
	case TypeInt:			
	case TypeIntVec2:		
	case TypeIntVec3:		
	case TypeIntVec4:		
		return GlesBasicType(TypeInt, type->base.prec);

	case TypeFloat:			
	case TypeFloatVec2:		
	case TypeFloatVec3:		
	case TypeFloatVec4:		
		return GlesBasicType(TypeFloat, type->base.prec);
	
	case TypeFloatMat2:		
		return GlesBasicType(TypeFloatVec2, type->base.prec);
	
	case TypeFloatMat3:		
		return GlesBasicType(TypeFloatVec3, type->base.prec);
	
	case TypeFloatMat4:		
		return GlesBasicType(TypeFloatVec4, type->base.prec);
	
	default:
		GLES_ASSERT(GL_FALSE);
	}
	
	return NULL;
}


Type * GlesTypeArrayCreate(MemoryPool * pool, Type * baseType, GLsizei size) {
	Type * type = (Type *) GlesMemoryPoolAllocate(pool, sizeof(struct TypeArray));
	
	type->base.kind = TypeArray;
	type->base.size = baseType->base.size * size;
	type->base.elements = baseType->base.elements;
	type->array.elementType = baseType;
	type->array.elements = size;
	
	return type;
}

Type * GlesTypeStructCreate(MemoryPool * pool) {
	Type * type = (Type *) GlesMemoryPoolAllocate(pool, sizeof(struct TypeStruct));
	
	type->base.kind = TypeStruct;
	type->base.size = 0;
	type->base.elements = 4;
	type->structure.numFields = 0;
	type->structure.fields = NULL;
	
	return type;
}

Type * GlesTypeFunctionCreate(MemoryPool * pool, Type * returnType, 
							  GLsizei numParams) {
	Type * type = (Type *) 
		GlesMemoryPoolAllocate(pool, sizeof(struct TypeFunction) +
							   numParams * sizeof(Parameter));
	
	type->base.kind = TypeFunction;
	type->base.size = returnType->base.size;
	type->base.elements = returnType->base.elements;
	type->func.returnType = returnType;
	type->func.numParams = numParams;
	
	return type;
}

GLboolean GlesTypeFunctionIsOverload(const Type * first, const Type * second) {
	
	GLsizei index;
	
	GLES_ASSERT(first->base.kind == TypeFunction);
	GLES_ASSERT(second->base.kind == TypeFunction);
	
	if (first->func.numParams != second->func.numParams) {
		return GL_FALSE;
	}
	
	for (index = 0; index < first->func.numParams; ++index) {
		if (!TypeMatchesIgnoreSize(first->func.parameters[index].type,
								   second->func.parameters[index].type)) {
			return GL_FALSE;
		}
	}
	
	return GL_TRUE;
}

GLboolean GlesTypeFunctionReturnTypeMatches(const Type * first, const Type * second) {
	GLES_ASSERT(first->base.kind == TypeFunction);
	GLES_ASSERT(second->base.kind == TypeFunction);

	return GlesTypeMatches(first->func.returnType, second->func.returnType);
}

GLboolean GlesTypeFunctionParamSizesMatch(const Type * first, const Type * second) {
	GLsizei index;
	
	for (index = 0; index < first->func.numParams; ++index) {
		if (!GlesTypeMatches(first->func.parameters[index].type,
						 	 second->func.parameters[index].type)) {
			return GL_FALSE;
		}
	}
	
	return GL_TRUE;
}

GLboolean GlesTypeFunctionParamQualifiersMatch(const Type * first, const Type * second) {

	GLsizei index;
	
	for (index = 0; index < first->func.numParams; ++index) {
		GLES_ASSERT(TypeMatchesIgnoreSize(first->func.parameters[index].type,
								   	 	  second->func.parameters[index].type));
								   	 	  
		if (first->func.parameters[index].direction != second->func.parameters[index].direction) {
			return GL_FALSE;
		}
		
		if (first->func.parameters[index].type->base.prec != 
			second->func.parameters[index].type->base.prec) {
			return GL_FALSE;
		}
	}
	
	return GL_TRUE;
}

#if GLES_DEBUG
void GlesTypePrint(const Type * type) {
	int index;
	const char * comma = "";
	
	if (!type) {
		printf("!type:NIL!");
		return;
	}
	
	switch (type->base.kind) {
		case TypeVoid:			printf("void");			break;
		case TypeBool:			printf("bool");			break;
		case TypeBoolVec2:		printf("bvec2");		break;
		case TypeBoolVec3:		printf("bvec3");		break;
		case TypeBoolVec4:		printf("bvec4");		break;
		case TypeInt:			printf("int");			break;
		case TypeIntVec2:		printf("ivec2");		break;
		case TypeIntVec3:		printf("ivec3");		break;
		case TypeIntVec4:		printf("ivec4");		break;
		case TypeFloat:			printf("float");		break;
		case TypeFloatVec2:		printf("vec2");			break;
		case TypeFloatVec3:		printf("vec3");			break;
		case TypeFloatVec4:		printf("vec4");			break;
		case TypeFloatMat2:		printf("mat2");			break;
		case TypeFloatMat3:		printf("mat3");			break;
		case TypeFloatMat4:		printf("mat4");			break;
		case TypeSampler2D:		printf("sampler2D");	break;
		case TypeSampler3D:		printf("sampler3D");	break;
		case TypeSamplerCube:	printf("samplerCube");	break;
		case TypeArray:			
			GlesTypePrint(type->array.elementType);
			printf("[%d]", type->array.elements);
			break;
		
		case TypeStruct:
			printf("{ %p }", type);
			break;
		
		case TypeFunction:
			GlesTypePrint(type->func.returnType);
			printf("(");
			
			for (index = 0; index < type->func.numParams; ++index) {
				printf(comma);
				comma = ",";
				GlesTypePrint(type->func.parameters[index].type);
			}
			
			printf(")");
			break;
			
		default:
			printf("!type:%d!", type->base.kind);
	}
}
#endif

