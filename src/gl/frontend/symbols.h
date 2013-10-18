#ifndef GLES_FRONTEND_SYMBOLS_H
#define GLES_FRONTEND_SYMBOLS_H 1

/*
** ==========================================================================
**
** $Id: symbols.h 65 2007-09-23 21:01:12Z hmwill $			
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

#include "frontend/types.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

typedef enum Qualifier {
	QualifierVariable,						/* general variable 			*/
	QualifierFunction,						/* user-defined function 		*/
	QualifierParameterIn,					/* input parameter		 		*/
	QualifierParameterOut,					/* output parameter		 		*/
	QualifierParameterInOut,				/* input/output parameter		*/
	QualifierConstant,						/* constant value				*/
	QualifierTypeName,						/* type name					*/
	QualifierField,							/* struct member	 			*/
	QualifierAttrib,						/* vertex attrib				*/
	QualifierUniform,						/* uniform value				*/
	QualifierVarying,						/* varying value				*/
	
	/*
	 * 7.1 Special objects in vertex shaders
	 */
	 
	QualifierPosition,						/* gl_Position variable			*/
	QualifierPointSize,						/* gl_PointSize					*/
	
	/*
	 * 7.2 Special objects in fragment shaders
	 */
	 
	QualifierFragCoord,						/* gl_FragCoord					*/
	QualifierFrontFacing,					/* gl_FrontFacing				*/
	QualifierFragColor,						/* gl_FragColor					*/
	QualifierFragData,						/* gl_FragData					*/
	QualifierPointCoord,					/* gl_PointCoord				*/
	
} Qualifier;

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

typedef union Symbol Symbol;
typedef struct Scope Scope;

struct MemoryPool;
union Constant;
struct Block;
struct Label;
union ProgVar;

typedef struct SymbolBase {
	Scope *			scope;					/* enclosing scope 					*/
	Symbol *		next;					/* linked list of symbols in scope	*/
	char *			name;					/* name of symbol 					*/
	GLsizeiptr		length;					/* length of name					*/
	Type *			type;					/* type of symbol					*/
	Qualifier		qualifier;				/* kind of symbol					*/
	GLuint			flags;					/* temp. bits for tree traversal	*/
} SymbolBase;

typedef struct SymbolVariable {
	SymbolBase		base;	
	union ProgVar *	progVar;				/* linked to IL output variable		*/
	union Constant*	initializer;			/* optional initialization value	*/
	GLboolean		invariant;				/* invariant flag					*/
} SymbolVariable;

typedef struct SymbolParameter {
	SymbolVariable	variable;	
	GLsizei			index;					/* param. index in function declaration */
	GLboolean		constant;				/* constant parameter?					*/
} SymbolParameter;

typedef struct SymbolField {
	SymbolBase		base;	
	GLsizeiptr		offset;					/* offset of symbol in structure		*/
	GLsizei			index;					/* param. index in function declaration */
} SymbolField;

typedef struct SymbolFunction {
	SymbolBase			base;
	BlockList			body;				/* function body; 0 for declaration 		*/
	Symbol *			overload;			/* next overloaded function for same name	*/
	Symbol *			result;				/* result value								*/
	Scope *				parameterScope;		/* parameter scope							*/
	struct Label *		label;				/* branch label of entry point				*/
} SymbolFunction;

union Symbol {
	SymbolBase		base;
	SymbolVariable	variable;
	SymbolParameter	parameter;
	SymbolFunction	function;
	SymbolField		field;
};

struct Scope {
	struct Scope *	parent;				/* parent scope						*/
	Precision		defaultIntPrec;		/* default precision for this scope */
	Precision		defaultFltPrec;		/* default precision for this scope */
	Precision		defaultS2DPrec;		/* default precision for this scope */
	Precision		defaultS3DPrec;		/* default precision for this scope */
	Precision		defaultSCubPrec;	/* default precision for this scope */
	Symbol *		buckets[GLES_SYMBOL_HASH];	/* symbols in this scope	*/
};

typedef struct SymbolArray {
	GLsizeiptr		allocated;			/* number of entries allocated		*/
	GLsizeiptr		used;				/* number of used entries			*/
	
	Symbol *		values[0];			/* size determined dynamically		*/
} SymbolArray;

/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

Scope * GlesScopeCreate(struct MemoryPool * pool, Scope * parent);
Precision GlesDefaultPrecisionForType(Scope * scope, TypeValue typeValue);

Symbol * GlesSymbolCreate(struct MemoryPool * pool, Scope * scope, 
						  const char * name, GLsizei length, GLsizei hash,
					  	  Type * type, Qualifier qualifier);

Symbol * GlesVariableCreate(struct MemoryPool * pool, Scope * scope, 
						  const char * name, GLsizei length, GLsizei hash,
					  	  Type * type, Qualifier qualifier, GLboolean invariant);

Symbol * GlesSymbolFind(Scope * scope, 
						const char * name, GLsizei length, GLsizei hash);

Symbol * GlesSymbolFindNested(Scope * scope, 
							  const char * name, GLsizei length, GLsizei hash);

GLsizei GlesSymbolHash(const char * name, GLsizei length);

char * GlesCompoundName(struct MemoryPool * pool,
						const char * base, GLsizei baseLength,
						const char * field, GLsizei fieldLength);

SymbolArray * GlesSymbolArrayCreate(struct MemoryPool * pool);
SymbolArray * GlesSymbolArrayGrow(struct MemoryPool * pool, SymbolArray * old);
												  	
#endif /* GLES_FRONTEND_SYMBOLS_H */