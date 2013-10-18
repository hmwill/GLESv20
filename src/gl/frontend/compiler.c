/*
** ==========================================================================
**
** $Id: compiler.c 65 2007-09-23 21:01:12Z hmwill $
**
** Top-level Shading Language Compiler
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
#include "frontend/compiler.h"
#include "frontend/memory.h"
#include "frontend/tokenizer.h"
#include "frontend/symbols.h"
#include "frontend/il.h"

/*
** --------------------------------------------------------------------------
** Module-local data
** --------------------------------------------------------------------------
*/

static const char * ErrorMessages[] = {
	"I0000: Internal compiler error",
	"I0001: Out of memory error",
	"P0001: Preprocessor syntax error", 
	"P0002: ",
	"P0003: #extension if a required extension extension_name is not supported, or if all is specified.", 
	"P0004: High Precision not supported",
	"P0005: #version must be the 1st directive/statement in a program", 
	"P0006: #line has wrong parameters", 
	"P0007: Unsupported #version number",
	"P0008: #extension must be the 1st directive/statement in a program", 
	"P0009: Duplicate macro definition",
	"P0010: Too many parameters for macro definition",
	"P0011: Maximum nesting depth of conditionals exceeded",
	"P0012: Maximum nesting depth for macro expansion exceeded",
	"P0013: Floating point arithmetic not allowed in preprocessor expressions",
	
	"L0001: Syntax error",
	"L0002: Undefined identifier.",
	"L0003: Use of reserved keywords",
	"S0001: Type mismatch in expression. e.g. 1 + 1.0;", 
	"S0002: Array parameter must be an integer", 
	"S0003: if parameter must be a bool",
	"S0004: Operator not supported for operand types (e.g. mat4 * vec3)", 
	"S0005: ?: parameter must be a bool",
	"S0006: 2nd and 3rd parameters of ?: must have the same type", 
	"S0007: Wrong arguments for constructor.", 
	"S0008: Argument unused in constructor",
	"S0009: Too few arguments for constructor",
	"S0010: Cannot construct matrices from matrices",
	"S0011: Arguments in wrong order for struct constructor", 
	"S0012: Expression must be a constant expression",
	"S0013: Initializer for const value must be a constant expression.", 
	"S0014: Initializer for global variable must be a constant expression.",
	"S0015: Expression must be an integral constant expression", 
	"S0016: Non-const index used to access unsized array", 
	"S0017: Array size must be greater thn zero.",
	"S0018: Re-declaration of parameter type with different array size.",
	"S0019: Indexing an array with a non constant integral expression before its size has been declared.", 
	"S0020: Indexing an array with an integral constant expression greater than its declared size.", 
	"S0021: Indexing an array with a negative integral constant expression", 
	"S0022: Redefinition of variable in same scope",
	"S0023: Redefinition of function in same scope",
	"S0024: Redefinition of name in same scope (e.g. declaring a function with the same name as a struct)", 
	"S0025: Field selectors must be from the same set (cannot mix xyzw with rgba)", 
	"S0026: Illegal field selector (e.g. using .z with a vec2)", 
	"S0027: Target of assignment is not an lvalue",
	"S0028: Precision used with type other than int or float.",
	"S0029: Declaring a main function with the wrong signature or return type.", 
	"S0030: Vertex shader does not compute the position of the vertex.", 
	"S0031: const variable does not have initializer",
	"S0032: Use of float or int without a precision qualifier where the default precision is not defined.",
	"S0033: Expression that does not have an intrinsic precision where the default precision is not defined.", 
	"S0034: Only output variables can be declared invariant",
	"S0035: All uses of invariant must be at the global scope",
	"S0037: L-value contains duplicate components (e.g. v.xx = q);",
	"S0038: Function declared with a return value but return statement has no argument.", 
	"S0039: Function declared void but return statement has an argument",
	"S0040: Function declared with a return value but not all paths return a value.", 
	"S0041: Function return type is an array.",
	"S0042: Return type of function definition must match return type of function declaration.",
	"S0043: Parameter qualifiers of function definition must match parameter qualifiers of function declaration.", 
	"S0044: Declaring an attribute outside of a vertex shader", 
	"S0045: Declaring an attribute inside a function",
	"S0046: Declaring a uniform inside a function",
	"S0047: Declaring a varying inside a function",
	"S0048: Illegal data type for varying (can only use float, vec2, vec3, vec4, mat2, mat3, and mat4 or arrays thereof).", 
	"S0049: Illegal data type for attribute (can only use float, vec2, vec3, vec4, mat2, mat3, and mat4).", 
	"S0050: Initializer for attribute", 
	"S0051: Initializer for varying", 
	"S0052: Initializer for uniform",
	"S0053: Invalid type for conditional expression",
	"S0054: Type mismatch for conditional expression",
	"S0055: Recursive function calls are not allowed: ",
	
	"S0100: Incomplete shader source (missing function definitions): ",
	
	"X0001: While and do-while loops not supported in this version",
	"X0002: Continue not supported in this version",
	"X0003: Loop index can be incremented only once",
	"X0004: Loop index variable must be of type int or float",
	"X0005: Dynamic indexing of vectors and matrices not implemented yet",
	"X0006: Loop index variable must be initialized to constant expression",
	"X0007: Loop index variable must be incremented with and compared to constant values",
	"X0008: Loop must be properly bounded and have at least one iteration",
	"X0009: Nesting depth for function calls exceeded",
	
	"W0001: Potentially unreachable statement",
	"W0002: Function not guaranteed to return a value",
	
};

const char BuiltinCommon[] = 
#include "builtin.common.inc"
;

const char BuiltinVertex[] = 
#include "builtin.vert.inc"
;

const char BuiltinInitVertex[] = 
#include "builtin.init.vert.inc"
;

const char BuiltinFragment[] = 
#include "builtin.frag.inc"
;

const char BuiltinInitFragment[] = 
#include "builtin.init.frag.inc"
;
	
/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static GLboolean PrepareCompiler(Compiler * compiler, Shader * shader) {
	
	const char * initStrings[4];
	GLsizei initArgs;
	
	compiler->shader = shader;
	
	compiler->exprMemory = GlesMemoryPoolCreate(GLES_DEFAULT_PAGE_SIZE, &compiler->allocationHandler);
	compiler->moduleMemory = GlesMemoryPoolCreate(GLES_DEFAULT_PAGE_SIZE, &compiler->allocationHandler);
	compiler->resultMemory = GlesMemoryPoolCreate(GLES_DEFAULT_PAGE_SIZE, &compiler->allocationHandler);
	
	initArgs = 0;
	
	switch (shader->type) {
	case GL_VERTEX_SHADER:	 initStrings[initArgs++] = BuiltinInitVertex;	break;
	case GL_FRAGMENT_SHADER: initStrings[initArgs++] = BuiltinInitFragment;	break;
	default:				 ;
	}
	
	initStrings[initArgs++] = BuiltinCommon;

	switch (shader->type) {
	case GL_VERTEX_SHADER:	 initStrings[initArgs++] = BuiltinVertex;	break;
	case GL_FRAGMENT_SHADER: initStrings[initArgs++] = BuiltinFragment;	break;
	default:				 ;
	}
	
	initStrings[initArgs] = NULL;

	GlesPrepareTokenizer(compiler->tokenizer, shader, compiler->moduleMemory,
		initArgs, initStrings);
	
	compiler->globalScope = 
	compiler->currentScope = GlesMemoryPoolAllocate(compiler->moduleMemory, sizeof(Scope));
	
	compiler->nextTempId = 0;
	
	compiler->generator.result = GlesCreateShaderProgram(compiler->resultMemory);
	compiler->generator.currentList = &compiler->generator.result->blocks;
	compiler->generator.instructionCount = 0;
	GlesCreateBlock(&compiler->generator);
	
	compiler->constExpression = GL_FALSE;
	compiler->currentLoop = NULL;
	compiler->structSpecifier = GL_FALSE;
	
#if GLES_DEBUG			
	GlesLogInit(&compiler->preprocLog);
#endif /* GLES_DEBUG */

	GlesLogInit(&compiler->ilLog);

	if (shader->il) {
		shader->il = NULL;
		shader->size = 0;
	}
	
	compiler->pragmaDebug = GL_FALSE;
	compiler->pragmaOptimize = GL_TRUE;
	
	return GL_TRUE;
}

static void SetSymbolFunctionFlags(Compiler * compiler, GLuint flags) {
	GLsizei index;
	Symbol * symbol;

	for (index = 0; index < GLES_SYMBOL_HASH; ++index) {
		for (symbol = compiler->globalScope->buckets[index]; symbol; symbol = symbol->base.next) {
			if (symbol->base.qualifier == QualifierFunction) {
				symbol->base.flags = flags;
			}
		}
	}
}

static GLboolean ProcessFunction(Compiler * compiler, Symbol * function,
	GLuint nest, SymbolArray ** parents) {

	if (nest > GLES_MAX_FUNCTION_DEPTH) {
		/* nesting depth exceeded */
		GlesCompileError(compiler, ErrX0009);
		return GL_FALSE;
	}
	
	if (function->base.flags) {
		/* function already processed; just verify no cycles */
		GLsizeiptr index = (*parents)->used;
		
		while (index > 0) {
			--index;
			
			if ((*parents)->values[index] == function) {
				/* have a circular reference */
				GlesCompileErrorSymbol(compiler, ErrS0055, function->base.name, function->base.length);
				return GL_FALSE;
			}
		}
		
		return GL_TRUE;
	} else if (!function->function.label->target) {
		/* function declared but not defined */
		GlesCompileErrorSymbol(compiler, ErrS0100, function->base.name, function->base.length);
		return GL_FALSE;
	} else {
		Block * block, *last;
		
		/* new function to be processed; mark as visited and add to stack */
		
		function->base.flags = nest;
		
		if ((*parents)->used >= (*parents)->allocated) {
			*parents = GlesSymbolArrayGrow(compiler->moduleMemory, *parents);
			GLES_ASSERT((*parents)->used < (*parents)->allocated);
		}
		
		(*parents)->values[(*parents)->used++] = function;
		
		/* append function body to program */

		block = function->function.body.head;
		last = function->function.body.tail;
		GlesInsertBlockList(&compiler->generator, &function->function.body);
		
		/* scan for called functions and call recursively */
		
		for (; block; block = block->next) {
			Inst * inst;
			
			for (inst = block->first; inst; inst = inst->base.next) {
				if (inst->base.kind == InstKindBranch) {
					Label * target = inst->branch.target;
					
					if (target->symbol &&
						target->symbol->base.qualifier == QualifierFunction) {
							
						if (!ProcessFunction(compiler, target->symbol, nest + 1,
								parents)) {
							return GL_FALSE;
						}
					}
				}
			}
			
			if (block == last) {
				break;
			}
		}
		
		(*parents)->used--;
	}
	
	return GL_TRUE;
}

static GLboolean AppendUsedFunctions(Compiler * compiler, Symbol * symbolMain) {
	
	/* list of functons to process */
	SymbolArray * parents = GlesSymbolArrayCreate(compiler->moduleMemory);
	
	/* mark all functions as not visited yet */
	SetSymbolFunctionFlags(compiler, 0u);
	GLES_ASSERT(parents->used < parents->allocated);
	parents->values[parents->used++] = NULL;
	
	return ProcessFunction(compiler, symbolMain, 1, &parents);
}

static GLboolean AllFunctionsDefined(Compiler * compiler) {
	Symbol * symbolMain = GlesSymbolFind(compiler->globalScope, "main", 4, ~0);
	
	if (!symbolMain 													||
		symbolMain->base.qualifier != QualifierFunction 				||
		symbolMain->base.type->func.returnType->base.kind != TypeVoid 	||
		symbolMain->base.type->func.numParams != 0 						||
		symbolMain->function.overload) {
		/* Declaring a main function with the wrong signature or return type. */
		GlesCompileError(compiler, ErrS0029);
		return GL_FALSE;
	}
	
	return AppendUsedFunctions(compiler, symbolMain);
}

static GLboolean Compile(Compiler * compiler) {
	
	if (!GlesParseTranslationUnit(compiler)) {
		return GL_FALSE;
	}
	
	/* verify that no functions are missing; in particular, main should be defined */
	if (!AllFunctionsDefined(compiler)) {
		return GL_FALSE;
	}
	
	/* generate the IL program text and attach to shader */
	GlesWriteShaderProgram(&compiler->ilLog, compiler->generator.result);
	compiler->shader->il = GlesMalloc(compiler->ilLog.logSize);
	
	if (compiler->shader->il) {
		compiler->shader->size = compiler->ilLog.logSize;
		GlesLogExtract(&compiler->ilLog, compiler->ilLog.logSize, compiler->shader->il, NULL);
	} else {
		/* out of memory error */
		GlesCompileError(compiler, ErrI0001);
		return GL_FALSE;
	}
	
	return GL_TRUE;
}

static void CleanupCompiler(Compiler * compiler) {
	
	/* free up any working storage */
	GlesCleanupTokenizer(compiler->tokenizer);

	GlesMemoryPoolDestroy(compiler->exprMemory);
	GlesMemoryPoolDestroy(compiler->moduleMemory);
	GlesMemoryPoolDestroy(compiler->resultMemory);

#if GLES_DEBUG				
	/* merge the log files into the info log attached to the shader */
	GlesLogAppend(&compiler->shader->log, "\n", 1);
	GlesLogAppendLog(&compiler->shader->log, &compiler->preprocLog);
	GlesLogClear(&compiler->preprocLog);
	
	GlesLogAppendLog(&compiler->shader->log, &compiler->ilLog);
	GlesLogClear(&compiler->ilLog);
#endif /* GLES_DEBUG */

	/* detach compiler from shader object */
	compiler->generator.result = NULL;
	compiler->shader = NULL;
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void GlesPragmaOptimize(Compiler * compiler, GLboolean enable) {
	compiler->pragmaOptimize = enable;
}

void GlesPragmaDebug(Compiler * compiler, GLboolean enable) {
	compiler->pragmaDebug = enable;
}

Compiler * GlesCompilerCreate(State * state) {
	Compiler * compiler = GlesMalloc(sizeof(Compiler));
	
	if (!compiler) {
		return NULL;
	}
	
	compiler->state = state;

#if GLES_DEBUG				
	compiler->tokenizer = GlesCreateTokenizer(&compiler->preprocLog, compiler);
#else
	compiler->tokenizer = GlesCreateTokenizer(NULL, compiler);
#endif
	
	if (!compiler->tokenizer) {
		GlesFree(compiler);
		
		return NULL;
	}
		
	return compiler;
}

void GlesCompilerDestroy(Compiler * compiler) {
	GLES_ASSERT(compiler->shader == NULL);
	GLES_ASSERT(compiler->exprMemory == NULL);
	GLES_ASSERT(compiler->moduleMemory == NULL);
	GLES_ASSERT(compiler->resultMemory == NULL);
	GLES_ASSERT(compiler->tokenizer);
	
	GlesDestroyTokenizer(compiler->tokenizer);
	GlesLogDeInit(&compiler->preprocLog);
	GlesLogDeInit(&compiler->ilLog);
	
	GlesFree(compiler);
}

GLboolean GlesCompileShader(Compiler * compiler, Shader * shader) {
	GLboolean result = GL_FALSE;
	
	GLES_ASSERT(compiler->state);
	GLES_ASSERT(compiler->shader == NULL);
	
	if (!GlesSetjmp(compiler->allocationHandler)) {
		if (PrepareCompiler(compiler, shader)) {
			result = Compile(compiler);
		}
	} else {
		GlesCompileError(compiler, ErrI0001);
	}		
		
	CleanupCompiler(compiler);
	
	return result;
}

/**
 * Append an error message to the compile log.
 * 
 * @param	compiler	reference to compiler object
 * @param	error		error message code
 */
void GlesCompileError(Compiler * compiler, CompileError error) {
	GLES_ASSERT(compiler);
	GLES_ASSERT(compiler->shader);
	
	GlesLogAppend(&compiler->shader->log, ErrorMessages[error], GlesStrlen(ErrorMessages[error]));
}

/**
 * Append an error message to the compile log.
 * 
 * @param	compiler	reference to compiler object
 * @param	error		error message code
 * @param	name		name of offending symbol
 * @param	length		string length of symbol name
 */
void GlesCompileErrorSymbol(Compiler * compiler, CompileError error, 
	const char * name, GLsizeiptr length) {
	GLES_ASSERT(compiler);
	GLES_ASSERT(compiler->shader);
	
	GlesLogAppend(&compiler->shader->log, ErrorMessages[error], GlesStrlen(ErrorMessages[error]));
	GlesLogAppend(&compiler->shader->log, name, length);
}

/**
 * Append an error message to the compile log.
 * 
 * @param	compiler	reference to compiler object
 * @param	text		error message text
 * @param	length		error message size
 */
void GlesCompileErrorMessage(Compiler * compiler, const char * text,
	GLsizeiptr length) {
	GlesCompileError(compiler, ErrP0002);
	GlesLogAppend(&compiler->shader->log, text, length);
}