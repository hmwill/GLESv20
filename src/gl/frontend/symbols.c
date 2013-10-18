/*
** ==========================================================================
**
** $Id: symbols.c 65 2007-09-23 21:01:12Z hmwill $
** 
** Shading Language Symbol Information
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-23 14:01:12 -0700 (Sun, 23 Sep 2007) $
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
#include "frontend/symbols.h"

/*
** --------------------------------------------------------------------------
** Module local data
** --------------------------------------------------------------------------
*/

#define SYMBOL_ARRAY_INIT_SIZE		32		/* creation size for sym. arrays */

/*
** --------------------------------------------------------------------------
** Module local function
** --------------------------------------------------------------------------
*/

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

GLsizei GlesSymbolHash(const char * name, GLsizeiptr length) {
  	GLsizei hval = length;

  	switch (hval) {
    default:
    	hval +=  name[7];

	case 7:
	case 6:
	case 5:
	case 4:
	case 3:
		hval += name[2];
		
	case 2:
	case 1:
	    hval += name[0];
	    break;
	}
  
  	return (hval + name[length - 1]) % GLES_SYMBOL_HASH;
}

Scope * GlesScopeCreate(struct MemoryPool * pool, Scope * parent) {
	Scope * scope = GlesMemoryPoolAllocate(pool, sizeof(Scope));
	
	scope->parent = parent;
	
	if (parent) {
		/* inherit default precisions from parent */
		
		scope->defaultIntPrec  = parent->defaultIntPrec;
		scope->defaultFltPrec  = parent->defaultFltPrec;
		scope->defaultS2DPrec  = parent->defaultS2DPrec;
		scope->defaultS3DPrec  = parent->defaultS3DPrec;
		scope->defaultSCubPrec = parent->defaultSCubPrec;
	} else {
		/* undefined default precisions; will load initial values into gloval scope */
		
		scope->defaultIntPrec  = PrecisionUndefined;
		scope->defaultFltPrec  = PrecisionUndefined;
		scope->defaultS2DPrec  = PrecisionUndefined;
		scope->defaultS3DPrec  = PrecisionUndefined;
		scope->defaultSCubPrec = PrecisionUndefined;
	}
	
	return scope;
}

Precision GlesDefaultPrecisionForType(Scope * scope, TypeValue typeValue) {
	switch (typeValue) {
	case TypeFloat:
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		return scope->defaultFltPrec;
		
	case TypeInt:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		return scope->defaultIntPrec;
		
	case TypeSampler2D:
		return scope->defaultS2DPrec;
		
	case TypeSampler3D:
		return scope->defaultS3DPrec;
		
	case TypeSamplerCube:
		return scope->defaultSCubPrec;
		
	default:
		return PrecisionUndefined;
	}
}

Symbol * GlesSymbolCreate(struct MemoryPool * pool, Scope * scope, 
						  const char * name, GLsizei length, GLsizei hash,
					  	  Type * type, Qualifier qualifier) {
	GLsizeiptr size;
	Symbol * symbol;
	
	switch (qualifier) {
	case QualifierConstant:					/* constant value				*/
	case QualifierVariable:					/* general variable 			*/
	case QualifierAttrib:					/* vertex attrib				*/
	case QualifierUniform:					/* uniform value				*/
	case QualifierVarying:					/* varying value				*/
		size = sizeof(SymbolVariable);
		break;
		
	case QualifierField:					/* struct member				*/
		size = sizeof(SymbolField);
		break;
		
	case QualifierFunction:					/* user-defined function 		*/
		size = sizeof(SymbolFunction);
		break;
		
	case QualifierParameterIn:				/* input parameter		 		*/
	case QualifierParameterOut:				/* output parameter		 		*/
	case QualifierParameterInOut:			/* input/output parameter		*/
		size = sizeof(SymbolParameter);
		break;
		
	case QualifierTypeName:					/* type name					*/
	case QualifierPosition:					/* gl_Position variable			*/
	case QualifierPointSize:				/* gl_PointSize					*/
	case QualifierFragCoord:				/* gl_FragCoord					*/
	case QualifierFrontFacing:				/* gl_FrontFacing				*/
	case QualifierFragColor:				/* gl_FragColor					*/
	case QualifierFragData:					/* gl_FragData					*/
	case QualifierPointCoord:				/* gl_PointCoord				*/
		size = sizeof(SymbolBase);
		break;
			
	default:			
		GLES_ASSERT(GL_FALSE);
		return NULL;
	}
	
	symbol = (Symbol *) GlesMemoryPoolAllocate(pool, size);
	symbol->base.name = (char *) GlesMemoryPoolAllocate(pool, length);
	GlesMemcpy(symbol->base.name, name, length);
	symbol->base.length = length;
	symbol->base.type = type;
	symbol->base.qualifier = qualifier;
	symbol->base.scope = scope;
	/*implied: symbol->base.next = NULL;*/
	
	if (hash == ~0) {
		hash = GlesSymbolHash(name, length);
	}
	
	symbol->base.next = scope->buckets[hash];
	scope->buckets[hash] = symbol;
	
	return symbol;
}

Symbol * GlesVariableCreate(struct MemoryPool * pool, Scope * scope, 
						  const char * name, GLsizei length, GLsizei hash,
					  	  Type * type, Qualifier qualifier, GLboolean invariant) {
	Symbol * symbol = GlesSymbolCreate(pool, scope, name, length, hash, type, qualifier);
	
	if (symbol && qualifier == QualifierVarying) {
		symbol->variable.invariant = invariant;
	}
	
	return symbol;
}

Symbol * GlesSymbolFind(Scope * scope, 
						const char * name, GLsizei length, GLsizei hash) {
	Symbol * symbol;
	
	if (hash == ~0) {
		hash = GlesSymbolHash(name, length);
	}
	
	symbol = scope->buckets[hash];
	
	while (symbol) {
		if (length == symbol->base.length && 
			!GlesMemcmp(symbol->base.name, name, length)) {
			return symbol;
		}
		
		symbol = symbol->base.next;
	}
	
	return NULL;
}

Symbol * GlesSymbolFindNested(Scope * scope, 
							  const char * name, GLsizei length, GLsizei hash) {
	if (hash == ~0) {
		hash = GlesSymbolHash(name, length);
	}
	
	while (scope) {
		Symbol * symbol = GlesSymbolFind(scope, name, length, hash);
		
		if (symbol) {
			return symbol;
		}
		
		scope = scope->parent;
	}
	
	return NULL;
}

char * GlesCompoundName(struct MemoryPool * pool,
						const char * base, GLsizei baseLength,
						const char * field, GLsizei fieldLength) {
	GLsizeiptr length = baseLength + fieldLength + 1;
	char * name = GlesMemoryPoolAllocate(pool, length);
	
	GlesMemcpy(name, base, baseLength);
	name[baseLength] = '.';
	GlesMemcpy(name + baseLength + 1, field, fieldLength);
		
	return name;
}

SymbolArray * GlesSymbolArrayCreate(struct MemoryPool * pool) {
	GLsizeiptr size = sizeof(struct SymbolArray) + 
		sizeof(Symbol *) * SYMBOL_ARRAY_INIT_SIZE;
		
	SymbolArray * array = GlesMemoryPoolAllocate(pool, size);
	
	if (array) {
		array->allocated = SYMBOL_ARRAY_INIT_SIZE;
		array->used = 0;
	}
	
	return array;
}

SymbolArray * GlesSymbolArrayGrow(struct MemoryPool * pool, SymbolArray * old) {
	GLsizeiptr newSize = old->allocated * 2;
	GLsizeiptr size = sizeof(struct SymbolArray) + 
		sizeof(Symbol *) * newSize;
		
	SymbolArray * array = GlesMemoryPoolAllocate(pool, size);
	
	if (array) {
		array->allocated = newSize;
		array->used = old->used;
		GlesMemcpy(array->values, old->values, old->used * sizeof(Symbol *));
	}
	
	return array;
}
