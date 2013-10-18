#ifndef GLES_FRONTEND_EXPRESSIONS_H
#define GLES_FRONTEND_EXPRESSIONS_H 1

/*
** ==========================================================================
**
** $Id: expressions.h 60 2007-09-18 01:16:07Z hmwill $			
** 
** Shading Language Front-End: Expression Processing
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

#include "frontend/compiler.h"
#include "frontend/types.h"
#include "frontend/constants.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

typedef enum ExpressionKind {
	ExpressionKindConstant,
	ExpressionKindVectorConstructor,
	ExpressionKindStructConstructor,
	ExpressionKindFixedComponent,	/* fixed vector component	*/
	ExpressionKindRandomComponent,	/* indexed vector component	*/
	ExpressionKindReference,		/* reference of a variable 	*/
} ExpressionKind;

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

struct MemoryPool;
struct TokenString;
union Symbol;


typedef union Expression Expression;

typedef struct ExpressionConstant			ExpressionConstant;
typedef struct ExpressionVectorConstructor	ExpressionVectorConstructor;
typedef struct ExpressionStructConstructor	ExpressionStructConstructor;
typedef struct ExpressionFixedComponent		ExpressionFixedComponent;
typedef struct ExpressionRandomComponent	ExpressionRandomComponent;
typedef struct ExpressionReference			ExpressionReference;

/**
 * Abstract base type for expression nodes
 */
typedef struct ExpressionBase {
	ExpressionKind		kind;
	Type *				type;
} ExpressionBase;

/**
 * A constant term; can represent any type
 */
struct ExpressionConstant {
	ExpressionBase		base;
	Constant *			value;		/**< Constant value array		*/
};

/**
 * Extract a fixed component of a vector.
 */
struct ExpressionFixedComponent {
	ExpressionBase 			base;
	ExpressionReference * 	expr;		/**< Base expression		*/
	GLsizei					index;		/**< component to extract	*/
};

/**
 * Extract a random component of a vector.
 */
struct ExpressionRandomComponent {
	ExpressionBase 				base;
	ExpressionReference *		expr;		/**< Base expression	*/
	ExpressionFixedComponent *	index;		/**< Index expression	*/
};

/**
 * Reference to a value stored in memory or a register; this is the
 * only valid representation of an l-value.
 */
struct ExpressionReference {
	ExpressionBase				base;
	union Symbol *				ref;	/**< Base reference				*/
	ExpressionFixedComponent *	index;	/**< Optional index register	*/
	GLsizeiptr					offset;	/**< constant offset on top		*/
};

typedef union ExpressionVectorElement {
	ExpressionBase *			base;
	ExpressionFixedComponent *	fixedComponent;
	ExpressionConstant *		constant;
} ExpressionVectorElement;

/**
 * Assemble a vector
 */
struct ExpressionVectorConstructor {
	ExpressionBase				base;
	GLboolean					assignable;	/**< if GL_TRUE, then potential l-value */
	ExpressionVectorElement		slots[0];	/**< component values	*/
};

typedef union ExpressionStructElement {
	ExpressionBase *				base;
	ExpressionConstant *			constant;
	ExpressionReference	*			reference;
	ExpressionStructConstructor *	structConstructor;
	ExpressionVectorConstructor *	vectorConstructor;
} ExpressionStructElement;

/**
 * Assemble a struct or matrix; COULD represent an array, but
 * language currently does not support this case.
 */
struct ExpressionStructConstructor {
	ExpressionBase				base;
	ExpressionStructElement		slots[0];
};

union Expression {
	ExpressionBase				base;
	ExpressionConstant			constant;
	ExpressionVectorConstructor	vectorConstructor;
	ExpressionStructConstructor	structConstructor;
	ExpressionFixedComponent	fixedComponent;
	ExpressionRandomComponent	randomComponent;
	ExpressionReference			reference;
};

typedef struct InitializerList {
	struct InitializerList *	next;
	union Symbol *				symbol;
	Expression *				value;
} InitializerList;

/*
** --------------------------------------------------------------------------
** Functions - Primtive expression building blocks
** --------------------------------------------------------------------------
*/

Expression * GlesCloneExpression(Compiler * compiler, Expression * expr);

Expression * GlesCreateExprVoid(Compiler * compiler);
Expression * GlesCreateExprConstant(Compiler * compiler, Type * type, Constant * values);
Expression * GlesCreateExprBoolConstant(Compiler * compiler, GLboolean value);
Expression * GlesCreateExprIntConstant(Compiler * compiler, GLint value);
Expression * GlesCreateExprFloatConstant(Compiler * compiler, GLfloat value);

Expression * GlesCreateExprConstructor(Compiler * compiler, Type * type, GLsizei numSlots, Expression ** slots);
Expression * GlesCreateExprStructComponent(Compiler * compiler, Expression * expr, GLsizei index);
Expression * GlesCreateExprVectorComponent(Compiler * compiler, Expression * expr, GLsizei index);
Expression * GlesCreateExprTypeCast(Compiler * compiler, Type * target, Expression  * source);
Expression * GlesCreateExprReference(Compiler * compiler, Type * type, union Symbol * ref, ExpressionFixedComponent * index, GLsizeiptr offset);

/*
** --------------------------------------------------------------------------
** Functions - Derived expression building blocks
** --------------------------------------------------------------------------
*/

Expression * GlesCreateExprField(Compiler * compiler, Expression * expr, struct TokenString * field);
Expression * GlesCreateExprIndex(Compiler * compiler, Expression * expr, Expression * index);
Expression * GlesCreateExprCall(Compiler * compiler, union Symbol * func, GLsizei numArgs, Expression ** args);

Expression * GlesCreateExprPostIncDec(Compiler * compiler, Expression * expr, GLboolean inc);
Expression * GlesCreateExprPreIncDec(Compiler * compiler, Expression * expr, GLboolean inc);

Expression * GlesCreateExprNegate(Compiler * compiler, Expression * expr, GLboolean plus);
Expression * GlesCreateExprNot(Compiler * compiler, Expression * expr);

Expression * GlesCreateExprMul(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprDiv(Compiler * compiler, Expression * left, Expression * right);

Expression * GlesCreateExprAddSub(Compiler * compiler, Expression * left, Expression * right, GLboolean add);

Expression * GlesCreateExprXor(Compiler * compiler, Expression * left, Expression * right);

Expression * GlesCreateExprAdd(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprSub(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprCompareEQ(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprCompareNE(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprCompareLT(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprCompareLE(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprCompareGT(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprCompareGE(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprAnd(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprOr(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesPrepareExprAnd(Compiler * compiler, Expression * left);
Expression * GlesPrepareExprOr(Compiler * compiler, Expression * left);

Expression * GlesCreateExprAssign(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprAssignAdd(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprAssignSub(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprAssignMul(Compiler * compiler, Expression * left, Expression * right);
Expression * GlesCreateExprAssignDiv(Compiler * compiler, Expression * left, Expression * right);

/*
** --------------------------------------------------------------------------
** Functions - Derived expression building blocks to set a condition code
** register.
** --------------------------------------------------------------------------
*/

Expression /* Reference */ * GlesEvaluate(Compiler * compiler, Expression * expr);

void GlesGenFetch(Compiler * compiler, SrcReg * reg, Expression * expr, const Swizzle * swizzle);
void GlesGenStore(Compiler * compiler, DstReg * reg, Expression * expr, Swizzle * swizzle);

#if GLES_DEBUG
void GlesPrintExpr(Expression * expr);
#endif

#endif /* GLES_FRONTEND_EXPRESSIONS_H */