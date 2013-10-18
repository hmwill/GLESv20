#ifndef GLES_FRONTEND_COMPILER_H
#define GLES_FRONTEND_COMPILER_H 1

/*
** ==========================================================================
**
** $Id: compiler.h 65 2007-09-23 21:01:12Z hmwill $			
** 
** Shading Language Compiler
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

#include "gl/state.h"
#include "frontend/il.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

typedef enum CompileError {
	
	ErrI0000,				/* Internal compiler error						*/
	ErrI0001,				/* Out of memory error							*/
	
	/*
	 * Preprocessor errors
	 */
	ErrP0001,				/* Preprocessor syntax error					*/ 
	ErrP0002,				/* #error 										*/
	ErrP0003,				/* #extension if a required extension 			*/
							/* extension_name is not supported, or if all 	*/
							/* is specified.								*/ 
	ErrP0004,				/* High Precision not supported					*/ 
	ErrP0005,				/* #version must be the 1st directive/statement */
							/* in a program									*/ 
	ErrP0006,				/* #line has wrong parameters					*/
	ErrP0007,				/* unsupported shading language version			*/
	ErrP0008,				/* extension must be specified at beginning		*/
	ErrP0009,				/* duplicate macro definition					*/
	ErrP0010,				/* too many parameters for macro definition		*/
	ErrP0011,				/* max. nesting depth of conditionals exceeded	*/
	ErrP0012,				/* max. nesting depth for macro exp. exceeded 	*/
	ErrP0013,				/* floating point arithmetic not allowed		*/
	
	/*
	 * Parser errors
	 */
	 
	ErrL0001,				/* Syntax error									*/ 
	ErrL0002,				/* Undefined identifier.						*/ 
	ErrL0003,				/* Use of reserved keywords						*/ 

	/*
	 * Semantic errors
	 */
	 
	ErrS0001, 				/* Type mismatch in expression. 				*/ 
	ErrS0002, 				/* Array parameter must be an integer			*/ 
	ErrS0003, 				/* if parameter must be a bool					*/ 
	ErrS0004, 				/* Operator not supported for operand types 	*/
	ErrS0005, 				/* ?: parameter must be a bool					*/ 
	ErrS0006, 				/* 2nd and 3rd parameters of ?: must have the 	*/
							/* same type									*/ 
	ErrS0007, 				/* Wrong arguments for constructor.				*/ 
	ErrS0008, 				/* Argument unused in constructor				*/ 
	ErrS0009, 				/* Too few arguments for constructor			*/ 
	ErrS0010, 				/* Cannot construct matrices from matrices		*/ 
	ErrS0011, 				/* Arguments in wrong order for struct 			*/
							/* constructor									*/ 
	ErrS0012, 				/* Expression must be a constant expression		*/ 
	ErrS0013, 				/* Initializer for const value must be a 		*/
							/* constant expression.							*/ 
	ErrS0014, 				/* Initializer for global variable must be		*/
							/* constant expression.							*/ 
	ErrS0015, 				/* Expression must be an integral constant 		*/
							/* expression									*/ 
	ErrS0016, 				/* Non-const index used to access unsized array */
	ErrS0017, 				/* Array size must be greater thn zero.			*/ 
	ErrS0018, 				/* Use of an array in as an actual parameter 	*/
							/* before its size has been declared.			*/ 
	ErrS0019, 				/* Indexing an array with a non constant 		*/
							/* integral expression before its size has been */
							/* declared.									*/ 
	ErrS0020, 				/* Indexing an array with an integral constant 	*/
							/* expression greater than its declared size.	*/ 
	ErrS0021, 				/* Indexing an array with a negative integral 	*/
							/* constant expression							*/ 
	ErrS0022, 				/* Redefinition of variable in same scope		*/ 
	ErrS0023, 				/* Redefinition of function in same scope		*/ 
	ErrS0024, 				/* Redefinition of name in same scope 			*/ 
	ErrS0025, 				/* Field selectors must be from the same set 	*/
	ErrS0026, 				/* Illegal field selector 						*/ 
	ErrS0027, 				/* Target of assignment is not an lvalue		*/ 
	ErrS0028, 				/* Precision used with type other than int or	*/
							/* float.										*/ 
	ErrS0029, 				/* Declaring a main function with the wrong 	*/
							/* signature or return type.					*/ 
	ErrS0030, 				/* Vertex shader does not compute the position 	*/
							/* of the vertex.								*/ 
	ErrS0031, 				/* const variable does not have initializer		*/ 
	ErrS0032, 				/* Use of float or int without a precision 		*/
							/* qualifier where the default precision is not	*/
							/* defined.										*/ 
	ErrS0033, 				/* Expression that does not have an intrinsic 	*/
							/* precision where the default precision is not	*/
							/* defined.										*/ 
	ErrS0034, 				/* Only output variables can be declared 		*/
							/* invariant									*/ 
	ErrS0035, 				/* All uses of invariant must be at the global	*/
							/* scope										*/ 
	ErrS0037, 				/* L-value contains duplicate components		*/
	ErrS0038, 				/* Function declared with a return value but 	*/
							/* return statement has no argument.			*/ 
	ErrS0039, 				/* Function declared void but return statement 	*/
							/* has an argument								*/ 
	ErrS0040, 				/* Function declared with a return value but 	*/
							/* not all paths return a value.				*/ 
	ErrS0041, 				/* Function return type is an array. 			*/
	ErrS0042, 				/* Return type of function definition must 		*/
							/* match return type of function declaration.	*/ 
	ErrS0043, 				/* Parameter qualifiers of function definition 	*/
							/* must match parameter qualifiers of function 	*/
							/* declaration.									*/ 
	ErrS0044, 				/* Declaring an attribute outside of a vertex 	*/
							/* shader										*/ 
	ErrS0045, 				/* Declaring an attribute inside a function 	*/
	ErrS0046, 				/* Declaring a uniform inside a function 		*/
	ErrS0047, 				/* Declaring a varying inside a function 		*/
	ErrS0048, 				/* Illegal data type for varying				*/
	ErrS0049, 				/* Illegal data type for attribute				*/ 
	ErrS0050, 				/* Initializer for attribute 					*/
	ErrS0051, 				/* Initializer for varying 						*/
	ErrS0052, 				/* Initializer for uniform 						*/
	ErrS0053, 				/* Invalid type for conditional expression		*/
	ErrS0054, 				/* Type mismatch for conditional expression		*/
	ErrS0055,				/* Recursive function calls are not allowed		*/	
	
	ErrS0100,				/* Incomplete shader source (undefined functions) */

	/*
	 * Implementation/language limitations
	 */
	 
	ErrX0001,				/* while and do-while not supported				*/
	ErrX0002,				/* continue not supported						*/
	ErrX0003,				/* loop index can be incremented only once		*/
	ErrX0004,				/* loop index must be of type int or float		*/
	ErrX0005,				/* dynamic indexing of vectors and matrices...	*/
	ErrX0006,				/* loop index variable must be initialized...	*/
	ErrX0007,				/* loop index variable must be incremented...	*/
	ErrX0008,				/* loop must be properly bounded  				*/
	ErrX0009,				/* nesting depth for function calls exceeded	*/
	
	/*
	 * Warnings
	 */
	ErrW0001,				/* Potentially unreachable statement			*/
	ErrW0002,				/* Function not guaranteed to return a value	*/
} CompileError;

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

struct MemoryPool;
struct SymbolFunction;
struct Scope;
struct BlockList;
struct ShaderProgram;
struct InitializerList;
struct ForLoop;

/**
 * Shading language compiler.
 * 
 * This structure aggregates all the top level objects constituting the
 * shading language compiler, including references to the GL state the
 * compiler is attached to, the shader object being compiled, and any
 * results.
 */
typedef struct Compiler {
	State * 				state;			/**< state this compiler is attached to */
	Shader * 				shader;			/**< shader object to be compiled */
	
	JumpBuffer				allocationHandler;	/**< handler to use for pool allocation failures */
	
	struct MemoryPool *		exprMemory;		/**< any storage only needed during compilation: expression scope */
	struct MemoryPool *		moduleMemory;	/**< any storage only needed during compilation: module scope */
	struct MemoryPool *		resultMemory;	/**< any storage to be persisteted as compilation result */
	
	struct Tokenizer *		tokenizer;		/**< tokenizer to use by compiler */
	
	struct Scope *			globalScope;	/**< global scope */
	struct Scope *			currentScope;	/**< current scope for nested blocks */
	
	struct SymbolFunction *	currentFunction;/**< currently parsed function	*/
	
	struct InitializerList*	initializers;	/**< At certain points we need to collect initialization expressions */
	GLboolean				collectInitializers;	/**< If GL_TRUE, we should collect initializers */

	ShaderProgramGenerator	generator;		/**< IL language generation structure */
		
	GLuint					numErrors;		/**< number of errors */
	GLuint					numWarnings;	/**< number of warnings */
	
	GLuint					nextTempId;		/**< sequence for temporary variables */
	GLboolean				constExpression;/**< currently evaluating a const expression */
	GLboolean				structSpecifier;/**< currently evaluating a struct specifier */
	struct ForLoop *		currentLoop;	/**< within a breakable loop */
	
	Log						ilLog;			/**< generated IL log */
#if GLES_DEBUG			
	Log						preprocLog;		/**< preprocessor log */
#endif /* GLES_DEBUG */

	GLboolean				pragmaOptimize;	/**< compiler pragma setting */
	GLboolean				pragmaDebug;	/**< compiler pragma setting */
} Compiler;

/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

Compiler * GlesCompilerCreate(State * state);
void GlesCompilerDestroy(Compiler * compiler);

GLboolean GlesCompileShader(Compiler * compiler, Shader * shader);
void GlesCompileError(Compiler * compiler, CompileError error);
void GlesCompileErrorSymbol(Compiler * compiler, CompileError error, 
	const char * name, GLsizeiptr length);
void GlesCompileErrorMessage(Compiler * compiler, const char * first,
	GLsizeiptr length);

/* parser entry point is also here */
GLboolean GlesParseTranslationUnit(Compiler * compiler);

void GlesPragmaOptimize(Compiler * compiler, GLboolean enable);
void GlesPragmaDebug(Compiler * compiler, GLboolean enable);

#endif /* GLES_FRONTEND_COMPILER_H */