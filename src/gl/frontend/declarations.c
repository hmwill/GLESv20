/*
** ==========================================================================
**
** $Id: declarations.c 65 2007-09-23 21:01:12Z hmwill $
** 
** Shading Language Front-End: Declaration Processing
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
#include "frontend/declarations.h"
#include "frontend/expressions.h"
#include "frontend/memory.h"
#include "frontend/il.h"

/*
** --------------------------------------------------------------------------
** Module local data
** --------------------------------------------------------------------------
*/

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



GLboolean GlesDeclareStructField(Compiler * compiler, Type * structure, 
						  		 const char * name, GLsizei length,
					  	  		 Type * type) {
	Symbol * symbol;
	GLsizei hash = GlesSymbolHash(name, length);
	
	GLES_ASSERT(structure->base.kind == TypeStruct);
	
	if (GlesSymbolFind(structure->structure.symbols, name, length, hash)) {
		return GL_FALSE;
	}
	
	symbol = 
		GlesSymbolCreate(compiler->moduleMemory, structure->structure.symbols, name, length, hash, type, QualifierField);
		
	symbol->field.index = structure->structure.numFields++;
	symbol->field.offset = structure->base.size;
	structure->base.size += type->base.size;
	
	return GL_TRUE;
}

GLboolean GlesDeclareStruct(Compiler * compiler, 
						  	const char * name, GLsizei length, Type * type) {
	Symbol * symbol;
	GLsizei hash = GlesSymbolHash(name, length);
	
	GLES_ASSERT(type->base.kind == TypeStruct);
	
	if (GlesSymbolFind(compiler->currentScope, name, length, hash)) {
		/* S0024: Redefinition of name in same scope */
				
		GlesCompileError(compiler, ErrS0024);
		return GL_FALSE;
	}
	
	symbol = 
		GlesSymbolCreate(compiler->moduleMemory, compiler->currentScope, name, length, hash, type, QualifierTypeName);
		
	return GL_TRUE;
}	

Symbol * GlesDeclareTempVariable(Compiler * compiler, Type * type) {
	char buffer[20];
	
	return
		GlesSymbolCreate(compiler->moduleMemory, 
						 compiler->currentScope, 
						 buffer, GlesSprintf(buffer, "$$%d", compiler->nextTempId++), ~0, 
						 type, QualifierVariable);
}

/** 
 * Since the memory model implied by the data packing algorithms (as well as the ARB IL
 * memory model) does not support structures, we need to convert compound data structures
 * into variables that are either of primitive types or are 1-dimensional arrays of primitive
 * types.
 * 
 * @param	compiler	reference to compiler object
 * @param	baseName	pointer to base name of variable
 * @param	baseLength	number of characters of base name
 * @param	qualifier	symbol qualifier for the symbol to be expanded
 * @param	type		type of the symbol to be expanded
 * @param	constant	constant flag (only used for parameters)
 */
static void ExpandArraysAndStructures(Compiler * compiler,
									  const char * baseName, GLsizei baseLength, 
							 		  Qualifier qualifier, Type * type, GLboolean constant) {
							 	
	GLsizeiptr baseElements = 1;
								 	
	if (type->base.kind == TypeArray) {
		baseElements = type->array.elements;
		type = type->array.elementType;
	}
	
	if (type->base.kind == TypeStruct) {
		GLsizei index;
		
		for (index = 0; index < type->structure.numFields; ++index) {
			Field * field = type->structure.fields + index;
			GLsizeiptr length = baseLength + field->symbol->base.length + 1;
			char * name = 
				GlesCompoundName(compiler->moduleMemory, baseName, baseLength,
								 field->symbol->base.name, field->symbol->base.length);
								 
			Symbol * symbol;
			Type * fieldType = field->type;
			
			if (fieldType->base.kind == TypeArray) {
				fieldType = GlesTypeArrayCreate(compiler->moduleMemory, fieldType->array.elementType, 
												baseElements * fieldType->array.elements);
			} else if (baseElements != 1) {
				fieldType = GlesTypeArrayCreate(compiler->moduleMemory, fieldType, baseElements);
			}
						
			symbol = 
				GlesSymbolCreate(compiler->moduleMemory, compiler->currentScope, name, length, ~0, fieldType, qualifier);

			if (qualifier == QualifierParameterIn 	||
				qualifier == QualifierParameterOut 	||
				qualifier == QualifierParameterInOut) {
				symbol->parameter.constant = constant;
			}
			
			ExpandArraysAndStructures(compiler, name, length, qualifier, fieldType, constant);
		}
	}
}
							  
GLboolean GlesDeclareVariable(Compiler * compiler, 
						  	  const char * name, GLsizei length, 
						  	  Qualifier qualifier, Type * type, GLboolean invariant,
						  	  union Expression * initializer) {
	Symbol * symbol;
	GLsizei hash = GlesSymbolHash(name, length);
	
	//GLES_ASSERT(type->base.kind != TypeStruct);
	
	if (GlesSymbolFind(compiler->currentScope, name, length, hash)) {
		/* S0024: Redefinition of name in same scope */
				
		GlesCompileError(compiler, ErrS0024);
		return GL_FALSE;
	}
	
	symbol = 
		GlesSymbolCreate(compiler->moduleMemory, compiler->currentScope, name, length, hash, type, qualifier);
		
	switch (qualifier) {
	case QualifierAttrib:
		if (compiler->shader->type != GL_VERTEX_SHADER) {
			/* S0044: Declaring an attribute outside of a vertex shader */
			
			GlesCompileError(compiler, ErrS0044);
			return GL_FALSE;
		}
		
		if (compiler->currentScope != compiler->globalScope) {
			/* S0045: Declaring a attribute inside a function */ 
			
			GlesCompileError(compiler, ErrS0045);
			return GL_FALSE;
		}
		
		if (initializer) {
			/* S0050: Initializer for attribute */
			
			GlesCompileError(compiler, ErrS0050);
			return GL_FALSE;
		}
		
		switch (type->base.kind) {
		case TypeFloat:
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			break;
			
		default:
			/* S0049: Illegal data type for attribute (can only use float, vec2, vec3, vec4, mat2, mat3, and mat4). */ 
			GlesCompileError(compiler, ErrS0049);
			return GL_FALSE;
		}
		
		break;
		
	case QualifierVarying:
		if (compiler->currentScope != compiler->globalScope) {
			/* S0047: Declaring a varying inside a function */ 
			
			GlesCompileError(compiler, ErrS0047);
			return GL_FALSE;
		}
		
		if (initializer) {
			/* S0051: Initializer for varying */
			
			GlesCompileError(compiler, ErrS0051);
			return GL_FALSE;
		}
		
		switch (type->base.kind) {
		case TypeFloat:
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			break;
			
		case TypeArray:
			switch (type->array.elementType->base.kind) {
			case TypeFloat:
			case TypeFloatVec2:
			case TypeFloatVec3:
			case TypeFloatVec4:
			case TypeFloatMat2:
			case TypeFloatMat3:
			case TypeFloatMat4:
				break;
		
			default:
				/* S0048: Illegal data type for attribute (can only use float, vec2, vec3, vec4, mat2, mat3, and mat4). */ 
				GlesCompileError(compiler, ErrS0048);
				return GL_FALSE;
			}

			break;
					
		default:
			/* S00498: Illegal data type for attribute (can only use float, vec2, vec3, vec4, mat2, mat3, and mat4). */ 
			GlesCompileError(compiler, ErrS0048);
			return GL_FALSE;
		}

		symbol->variable.invariant = invariant;

		break;
	
	case QualifierUniform:
		if (compiler->currentScope != compiler->globalScope) {
			/* S0046: Declaring a uniform inside a function */ 
			
			GlesCompileError(compiler, ErrS0046);
			return GL_FALSE;
		}
		
		if (initializer) {
			/* S0052: Initializer for uniform */
			
			GlesCompileError(compiler, ErrS0052);
			return GL_FALSE;
		}

		ExpandArraysAndStructures(compiler, name, length, qualifier, type, GL_FALSE);
		
		break;
		
	case QualifierConstant:
		if (!initializer) {
			/* S0031: const variable does not have initializer */
			GlesCompileError(compiler, ErrS0031);
			return GL_FALSE;
		}
		
		if (initializer->base.kind != ExpressionKindConstant) {
			/* S0013: Initializer for const value must be a constant expression. */
			GlesCompileError(compiler, ErrS0013);
			return GL_FALSE;
		}
		
		symbol->variable.initializer = initializer->constant.value;
		
		break;
		
	case QualifierVariable:
		if (initializer &&
			compiler->currentScope == compiler->globalScope && 
			initializer->base.kind != ExpressionKindConstant) {
			/* initializer for global variable must be constant expression */
			GlesCompileError(compiler, ErrS0014);
			return GL_FALSE;
		}
		
		ExpandArraysAndStructures(compiler, name, length, qualifier, type, GL_FALSE);
		
		if (initializer) {
			GlesCreateExprAssign(compiler, 
								 GlesCreateExprReference(compiler, type, symbol, NULL, 0), 
								 initializer);
							 
			if (initializer->base.kind == ExpressionKindConstant) {
				symbol->variable.initializer = initializer->constant.value;
			}
		}
		
		break;
	
	default:
		/* should not reach this */
		GLES_ASSERT(GL_FALSE);
	}
	
	return GL_TRUE;
}

GLboolean GlesDeclareParameter(Compiler * compiler, 
						  	   const char * name, GLsizei length, 
						  	   Qualifier qualifier, Type * type, 
							   GLsizei index, GLboolean constant) {
	Symbol * symbol;
	GLsizei hash = GlesSymbolHash(name, length);
	
	GLES_ASSERT(type->base.kind != TypeStruct);
	GLES_ASSERT(qualifier == QualifierParameterIn 	||
				qualifier == QualifierParameterOut 	||
				qualifier == QualifierParameterInOut);
	
	if (GlesSymbolFind(compiler->currentScope, name, length, hash)) {
		/* S0024: Redefinition of name in same scope */
				
		GlesCompileError(compiler, ErrS0024);
		return GL_FALSE;
	}
	
	symbol = 
		GlesSymbolCreate(compiler->moduleMemory, compiler->currentScope, name, length, hash, type, qualifier);
		
	symbol->parameter.constant = constant;
	symbol->parameter.index = index;
	
	ExpandArraysAndStructures(compiler, name, length, qualifier, type, GL_FALSE);
		
	
	return GL_TRUE;
}

GLboolean GlesDeclareInvariant(Compiler * compiler, 
							   const char * name, GLsizei length) {
	Symbol * symbol;
	
	GLES_ASSERT(compiler->currentScope == compiler->globalScope);
	
	symbol = GlesSymbolFind(compiler->currentScope, name, length, ~0);
	
	if (symbol == NULL) {
		/* L0002: Undefined identifier */
		
		GlesCompileError(compiler, ErrL0002);
		return GL_FALSE;
	}
	
	switch (symbol->base.qualifier) {
	case QualifierVarying:
		symbol->variable.invariant = GL_TRUE;
		break;

	case QualifierPosition:
	case QualifierPointSize:
	case QualifierFragColor:
	case QualifierFragData:
		/* TODO: not sure yet what to do... */
		GLES_ASSERT(GL_FALSE);
		
		break;
		
	default:
		/* S0034: Only output variables can be declared invariant */
		
		GlesCompileError(compiler, ErrS0034);
		return GL_FALSE;
	} 
	
	return GL_TRUE;
}

Symbol * GlesDeclareFunction(Compiler * compiler,
							 const char * name, GLsizei length, 
							 Type * returnType, 
							 GLsizei numParameters, Scope * parameters) {
	GLsizei hash = GlesSymbolHash(name, length);
	GLsizei index;
	Type * type;
	Symbol * function;
							  	
	if (returnType->base.kind == TypeArray) {
		/* S0041: Function return type is an array. */
		
		GlesCompileError(compiler, ErrS0041);
		return NULL;
	}
	
	type = GlesTypeFunctionCreate(compiler->moduleMemory, returnType, numParameters);
	
	for (index = 0; index < GLES_ELEMENTSOF(parameters->buckets); ++index) {
		Symbol * symbol;
		
		for (symbol = parameters->buckets[index]; symbol; symbol = symbol->base.next) {
			GLES_ASSERT(symbol->base.qualifier == QualifierParameterIn 	||
						symbol->base.qualifier == QualifierParameterOut	||
						symbol->base.qualifier == QualifierParameterInOut);
			GLES_ASSERT(symbol->parameter.index < numParameters);
			type->func.parameters[symbol->parameter.index].type = symbol->base.type;
			
			switch (symbol->base.qualifier) {
			case QualifierParameterIn:	
				type->func.parameters[symbol->parameter.index].direction = ParameterDirIn;
				break;
			
			case QualifierParameterOut:	
				type->func.parameters[symbol->parameter.index].direction = ParameterDirOut;
				break;
			
			case QualifierParameterInOut:	
				type->func.parameters[symbol->parameter.index].direction = ParameterDirInOut;
				break;
			
			default:
				GLES_ASSERT(GL_FALSE);
			}
		}
	}
	
	for (index = 0; index < numParameters; ++index) {
		GLES_ASSERT(type->func.parameters[index].type != NULL);
	}
	
	/* now hat we have created the function type, let's determine if the function
	 * has already been declared, and if so, see if the types match.
	 * 
	 * Since functions can be overloaded, type matching for the purpose of overload
	 * detection does not test for parameter direction or sizes of array type.
	 * 
	 * However, in the case of an overload situation, we need to verify that directions and
	 * sizes actually match.
	 * 
	 * If no overload is occurring then we simply introduce a new function for the
	 * given type in the current scope.
	 */
	
	function = GlesSymbolFind(compiler->currentScope, name, length, hash);
	
	if (function != NULL) {
		Symbol * overload;
		
		for (overload = function; overload; overload = overload->function.overload) {
			if (GlesTypeFunctionIsOverload(type, overload->base.type)) {
				if (!GlesTypeFunctionReturnTypeMatches(type, overload->base.type)) {
					/* S0042: Return type of function definition must match return	*/ 
					/* type of function declaration.								*/
					
					GlesCompileError(compiler, ErrS0042);
					return NULL; 
				}
				
				if (!GlesTypeFunctionParamQualifiersMatch(type, overload->base.type)) {
					/* S0043: Parameter qualifiers of function definition must 		*/
					/* match parameter qualifiers of function declaration.			*/ 
					
					GlesCompileError(compiler, ErrS0043);
					return NULL; 
				}
				
				if (!GlesTypeFunctionParamSizesMatch(type, overload->base.type)) {
					/* S0018: Re-declaration of parameter type with different 		*/
					/* array size.													*/
					
					GlesCompileError(compiler, ErrS0018);
					return NULL; 
				}
				/*function = overload;
				break;*/
				return overload;
			}
		}

		overload = function;
		
		function = 		
			GlesSymbolCreate(compiler->moduleMemory, compiler->currentScope, 
							 name, length, hash, type, QualifierFunction);
		
		function->function.overload = overload;
	} else {
		function = 		
			GlesSymbolCreate(compiler->moduleMemory, compiler->currentScope, 
							 name, length, hash, type, QualifierFunction);
	}
	
	if (returnType && returnType->base.kind != TypeVoid) {
		function->function.result = GlesDeclareTempVariable(compiler, returnType);
	} else {
		function->function.result = NULL;
	}
	
	function->function.parameterScope = parameters;
	function->function.label = GlesCreateLabel(&compiler->generator, function);
	
	return function;
}


GLboolean GlesFinishStructDeclaration(Compiler * compiler, Type * type) {
	GLsizei index;
	Scope * fields;
	GLES_ASSERT(type->base.kind == TypeStruct);
	
	fields = type->structure.symbols;
	type->structure.fields = (Field *) 
		GlesMemoryPoolAllocate(compiler->resultMemory, type->structure.numFields * sizeof(struct Field));
	
	for (index = 0; index < GLES_ELEMENTSOF(fields->buckets); ++index) {
		Symbol * symbol;
		
		for (symbol = fields->buckets[index]; symbol; symbol = symbol->base.next) {
			GLES_ASSERT(symbol->base.qualifier == QualifierField);
			GLES_ASSERT(symbol->field.index < type->structure.numFields);
			type->structure.fields[symbol->field.index].type = symbol->base.type;
			type->structure.fields[symbol->field.index].symbol = &symbol->field;
		}
	}
	
	return GL_TRUE;
}

Symbol * GlesSymbolFindExpansion(Compiler * compiler, 
								 Symbol * base, SymbolField * field) {

	char * name =
		GlesCompoundName(compiler->exprMemory,
						 base->base.name, base->base.length,
						 field->base.name, field->base.length);
						 
	GLsizei length = base->base.length + field->base.length + 1;
	
	Symbol * expansion =
		GlesSymbolFindNested(compiler->currentScope, name, length, ~0);
		
	GLES_ASSERT(expansion);
	
	return expansion;
}