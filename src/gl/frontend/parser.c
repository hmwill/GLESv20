/*
** ==========================================================================
**
** $Id: parser.c 65 2007-09-23 21:01:12Z hmwill $
**
** Parser for Shading Language Compiler
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
#include "frontend/memory.h"
#include "frontend/tokenizer.h"
#include "frontend/compiler.h"
#include "frontend/types.h"
#include "frontend/symbols.h"
#include "frontend/expressions.h"
#include "frontend/declarations.h"
#include "frontend/statements.h"
#include "frontend/il.h"
#include "frontend/asm.h"

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static GLboolean ParseTypeSpecifier(Compiler * compiler, Type ** type);
static GLboolean ParseTypeSpecifierNoPrec(Compiler * compiler, Precision modifier, Type ** type);
static GLboolean ParseFullySpecifiedType(Compiler * compiler, Type ** type, Qualifier * qualifier, GLboolean * invariant);
static GLboolean ParseLocalDeclaration(Compiler * compiler);
static GLboolean ParseOptArrayOrInitializer(Compiler * compiler, Type * baseType, Type ** type, Expression ** initializer);
static GLboolean ParseInitDeclaratorListTail(Compiler * compiler, Type * baseType, Qualifier qualifier, GLboolean invariant);


static GLboolean ParsePrimaryExpression(Compiler * compiler, Expression ** expression);
static GLboolean ParsePostfixExpressionTail(Compiler * compiler, Expression * lhs, Expression ** expression);
static GLboolean ParseUnaryExpression(Compiler * compiler, Expression ** expression);
static GLboolean ParseBinaryExpressionTail(Compiler * compiler, Expression * lhs, Expression ** expression);
static GLboolean ParseBinaryExpression(Compiler * compiler, Expression ** expression);
static GLboolean ParseConditionalExpressionTail(Compiler * compiler, Expression * lhs, Expression ** expression);
static GLboolean ParseConditionalExpression(Compiler * compiler, Expression ** expression);
static GLboolean ParseAssignmentExpression(Compiler * compiler, Expression ** expression);
static GLboolean ParseExpression(Compiler * compiler, Expression ** expression);

static GLboolean ParseConstantExpression(Compiler * compiler, Expression ** expression);

static GLboolean ParseStatement(Compiler * compiler, ForLoop ** continuation, GLuint * continuationMask);
static GLboolean ParseCompoundStatement(Compiler * compiler, Scope * newScope, ForLoop ** continuation, GLuint * continuationMask);

/**
 * Report a syntax error
 * 
 * @param	compiler	reference to compiler object
 */
static GLES_INLINE void SyntaxError(Compiler * compiler) {
	/* L0001: Syntax error */
	
	GlesCompileError(compiler, ErrL0001);
}

/**
 * Verify that the next token is of the given token type. If so, advance
 * to the next token, otherwise report a syntax error.
 * 
 * @param	compiler 	reference to compiler object
 * @param 	tokenType	expected token type
 * 
 * @return	GL_TRUE if successful, GL_FALSE on error
 */
static GLboolean MustBe(Compiler * compiler, TokenType tokenType) {
	if (compiler->tokenizer->token.tokenType != tokenType) {
		SyntaxError(compiler);
		return GL_FALSE;
	} else {
		return GlesNextToken(compiler->tokenizer);
	}
}

#define MUSTBE(compiler, tokenType) if (!MustBe(compiler, tokenType)) return GL_FALSE
#define ADVANCE(compiler) if (!GlesNextToken(compiler->tokenizer)) return GL_FALSE;

/*
** --------------------------------------------------------------------------
** Type parsing functions
** --------------------------------------------------------------------------
*/

/**
 * Parse an optional precision modifier and return its value. If a precision 
 * modifier was detected, the parser advances to the next token.
 * 
 * precision_modifier ::= [ "highp" | "mediump" | "lowp" ].
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	the associated precision value or PrecisionUndefined if no
 * precision modifier was present.
 */
static Precision ParsePrecisionModifier(Compiler * compiler) {
	Precision precision;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeHighPrecision:
		precision = PrecisionHigh;
		break;
		
	case TokenTypeMediumPrecision:
		precision = PrecisionMedium;
		break;
		
	case TokenTypeLowPrecision:
		precision = PrecisionLow;
		break;
		
	default:
		return PrecisionUndefined;
	}

	ADVANCE(compiler);
	return precision;	
}

/**
 * Detect a struct declarator, i.e.
 * 
 * struct_declarator ::= <identifier> [ "[" constant_expression "]" ].
 * 
 * @param	compiler	reference to compiler object
 * @param	baseType	base type for declarator
 * @param	type		out: reference to resulting type; either baseType or derived array type
 * @param	name		out: container for identifier value
 * 
 * @return	GL_TRUE if successful, GL_FALSE on error
 */
static GLboolean ParseStructDeclarator(Compiler * compiler, Type * baseType, Type ** type,
	TokenString * name) {
	*name = compiler->tokenizer->token.s;
	MUSTBE(compiler, TokenTypeIdentifier);
	
	if (compiler->tokenizer->token.tokenType == TokenTypeLeftBracket) {
		
		Expression * size = NULL;
		
		if (!ParseConstantExpression(compiler, &size)) {
			return GL_FALSE;
		}

		GLES_ASSERT(size->base.kind == ExpressionKindConstant);
		
		if (size->base.type->base.kind != TypeInt) {
			/* S0015: Expression must be an integral constant expression */
			
			GlesCompileError(compiler, ErrS0015);
			return GL_FALSE;
		}
		
		if (size->constant.value->intValue[0] < 0) {
			/* S0017: Array size must be greater than zero. */
			
			GlesCompileError(compiler, ErrS0017);
			return GL_FALSE;
		}
				
		*type = GlesTypeArrayCreate(compiler->moduleMemory, baseType, size->constant.value->intValue[0]);
		return MustBe(compiler, TokenTypeRightBracket);
	} else {	
		*type = baseType;
		return GL_TRUE;
	}
}

/**
 * Detect a struct declaration and record any declarations in the symbol table, i.e.
 * 
 * struct_declaration ::= type_specifier struct_declarator { "," struct_declarator } ";".
 * 
 * @param	compiler	reference to compiler object
 * @param	structType	reference of the structure to be defined
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseStructDeclaration(Compiler * compiler, Type * structType) {
	Type * baseType, *type;
	TokenString name;

	GLES_ASSERT(structType->base.kind == TypeStruct);
	GLES_ASSERT(structType->structure.fields == NULL);
	
	if (!ParseTypeSpecifier(compiler, &baseType)) {
		return GL_FALSE;
	}
	
	if (!ParseStructDeclarator(compiler, baseType, &type, &name) ||
		!GlesDeclareStructField(compiler, structType, 
								name.first, name.length,
								type)) {
		return GL_FALSE;
	}
	
	while (compiler->tokenizer->token.tokenType == TokenTypeComma) {
		ADVANCE(compiler);

		if (!ParseStructDeclarator(compiler, baseType, &type, &name) ||
		!GlesDeclareStructField(compiler, structType, 
								name.first, name.length,
								type)) {
			return GL_FALSE;
		}
	}
	
	return MustBe(compiler, TokenTypeSemicolon);
}

/**
 * Parse a struct specifier and record the type and optional type name
 * declaration in the symbol table.
 * 
 * struct_specifier ::= 
 * 		"struct" [ <identifier> ] "{" { struct_declaration } "}".
 * 
 * @param	compiler	reference to compiler object
 * @param	type		out: reference to resulting type
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseStructSpecifier(Compiler * compiler, Type ** type) {
	MUSTBE(compiler, TokenTypeStruct);
	
	if (compiler->structSpecifier) {
		/* do not allow nested struct specifications */
		SyntaxError(compiler);
		return GL_FALSE;
	}
	
	compiler->structSpecifier = GL_TRUE;
	*type = GlesTypeStructCreate(compiler->moduleMemory);

	if (compiler->tokenizer->token.tokenType == TokenTypeIdentifier) {
		if (!GlesDeclareStruct(compiler,
							   compiler->tokenizer->token.s.first,
							   compiler->tokenizer->token.s.length,
							   *type)) {									  
			return GL_FALSE;
		}
		
		ADVANCE(compiler);
	}
	
	MUSTBE(compiler, TokenTypeLeftBrace);
	GLES_ASSERT(!(*type)->structure.symbols);

	(*type)->structure.symbols =
		GlesScopeCreate(compiler->moduleMemory, compiler->currentScope);
		
	compiler->currentScope = (*type)->structure.symbols;	
	
	while (compiler->tokenizer->token.tokenType != TokenTypeRightBrace) {
		if (!ParseStructDeclaration(compiler, *type)) {
			return GL_FALSE;
		}
	}
	
	compiler->currentScope = compiler->currentScope->parent;
	
	if (!GlesFinishStructDeclaration(compiler, *type)) {
		return GL_FALSE;
	}
	
	compiler->structSpecifier = GL_FALSE;
	
	return MustBe(compiler, TokenTypeRightBrace);
}

/**
 * Convert a primitive type token in the current look-ahead
 * to the associated type value.
 * 
 * @param	compiler	reference to compiler object.
 * 
 * @return	the associated type value or TypeInvalid if the token does
 * 			not represent a proper primitive type
 */
static TypeValue DecodeType(Compiler * compiler) {
	TypeValue typeValue;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeVoid:			typeValue = TypeVoid;			break;
	case TokenTypeFloat:		typeValue = TypeFloat;			break;
	case TokenTypeInt:			typeValue = TypeInt;			break;
	case TokenTypeBool:			typeValue = TypeBool;			break;
	case TokenTypeFloatVec2:	typeValue = TypeFloatVec2;		break;
	case TokenTypeFloatVec3:	typeValue = TypeFloatVec3;		break;
	case TokenTypeFloatVec4:	typeValue = TypeFloatVec4;		break;
	case TokenTypeIntVec2:		typeValue = TypeIntVec2;		break;
	case TokenTypeIntVec3:		typeValue = TypeIntVec3;		break;
	case TokenTypeIntVec4:		typeValue = TypeIntVec4;		break;
	case TokenTypeBoolVec2:		typeValue = TypeBoolVec2;		break;
	case TokenTypeBoolVec3:		typeValue = TypeBoolVec3;		break;
	case TokenTypeBoolVec4:		typeValue = TypeBoolVec4;		break;
	case TokenTypeFloatMat2:	typeValue = TypeFloatMat2;		break;
	case TokenTypeFloatMat3:	typeValue = TypeFloatMat3;		break;
	case TokenTypeFloatMat4:	typeValue = TypeFloatMat4;		break;
	case TokenTypeSampler2D:	typeValue = TypeSampler2D;		break;
	case TokenTypeSampler3D:	typeValue = TypeSampler3D;		break;
	case TokenTypeSamplerCube:	typeValue = TypeSamplerCube;	break;
	default:					typeValue = TypeInvalid;		break;
	}
	
	return typeValue;
}

/**
 * Detect a type specifier, i.e. a built-in primitive type, a previously defined type
 * name or a structure declaration.
 * 
 * type_specifier_no_prec ::=
 * 		"void" | "float" | "int" | "bool" 
 * 		| "vec2" | "vec3" | "vec4" | "bvec2" | "bvec3" | "bvec4"
 * 		| "ivec2" | "ivec3" | "ivec4" | "mat2" | "mat3" | "mat4"
 * 		| "sampler2D" | "sampler3D" | "samplerCube"
 * 		| <identifier> | struct_specifier.
 * 
 * @param	compiler	reference to compiler object
 * @param	precision	a precision modifier to apply
 * @param	type		out: reference to resulting type
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseTypeSpecifierNoPrec(Compiler * compiler, Precision precision, Type ** type) {
	
	TypeValue typeValue;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeVoid:		
	case TokenTypeFloat:	
	case TokenTypeInt:		
	case TokenTypeBool:		
	case TokenTypeFloatVec2:
	case TokenTypeFloatVec3:
	case TokenTypeFloatVec4:
	case TokenTypeIntVec2:	
	case TokenTypeIntVec3:	
	case TokenTypeIntVec4:	
	case TokenTypeBoolVec2:	
	case TokenTypeBoolVec3:	
	case TokenTypeBoolVec4:	
	case TokenTypeFloatMat2:
	case TokenTypeFloatMat3:
	case TokenTypeFloatMat4:
	case TokenTypeSampler2D:
	case TokenTypeSampler3D:	
	case TokenTypeSamplerCube:	
		typeValue = DecodeType(compiler);
		break;

 	case TokenTypeIdentifier:
 		{
 			Symbol * symbol = 
 				GlesSymbolFindNested(compiler->currentScope,
 									 compiler->tokenizer->token.s.first,
 									 compiler->tokenizer->token.s.length,
 									 ~0);
 									 
 			if (!symbol || symbol->base.qualifier != QualifierTypeName) {
 				/* type name expected */
 				SyntaxError(compiler);
 				return GL_FALSE;
 			} else {
 				*type = symbol->base.type;
				ADVANCE(compiler);
				return GL_TRUE;
 			}
 		}
 		
	case TokenTypeStruct:		
        return ParseStructSpecifier(compiler, type);
        
    default:
    	SyntaxError(compiler);
    	return GL_FALSE;
   	}

	ADVANCE(compiler);
	
	if (precision == PrecisionUndefined) {
		precision = GlesDefaultPrecisionForType(compiler->currentScope, typeValue);
	}
	
	*type = GlesBasicType(typeValue, precision);
	
	if (!*type) {
		/* S0033: Expression that does not have an intrinsic precision where the default precision is not defined. */
		/* TODO: is this the right message? */
		
		GlesCompileError(compiler, ErrS0033);
		return GL_FALSE;
	}
	
	return GL_TRUE;
	
}

/**
 * Parse a type specifier, i.e.
 * 
 * type_specifier ::= precision_modifier type_specifier_no_prec.
 * 
 * @param	compiler	reference to compiler object
 * @param	type		out: reference to resulting type
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseTypeSpecifier(Compiler * compiler, Type ** type) {
	Precision precision = ParsePrecisionModifier(compiler);
	
	return ParseTypeSpecifierNoPrec(compiler, precision, type);
}

/**
 * Parse a fully specified (qualified?) type, i.e.
 * 
 * fully_specified_type ::= 
 * 		[ [ "invariant" ] "varying" | "const" | "attribute" | "uniform" ] type_specifier.
 * 
 * @param	compiler	reference to compiler object
 * @param	type		out: reference to resulting type
 * @param	qualifier	out: qualifier value
 * @param	invariant	out: invariant flag for varyings
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseFullySpecifiedType(Compiler * compiler, Type ** type, Qualifier * qualifier, GLboolean * invariant) {
	
	*qualifier = QualifierVariable;
	*invariant = GL_FALSE;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeInvariant:
		ADVANCE(compiler);
		MUSTBE(compiler, TokenTypeVarying);
		*qualifier = QualifierVarying;
		*invariant = GL_TRUE;
		goto specifier;
		
	case TokenTypeConst:
		*qualifier = QualifierConstant;
		ADVANCE(compiler);
		goto specifier;
		
	case TokenTypeAttribute:
		*qualifier = QualifierAttrib;
		ADVANCE(compiler);
		goto specifier;
		
	case TokenTypeVarying:
		*qualifier = QualifierVarying;
		ADVANCE(compiler);
		goto specifier;
		
	case TokenTypeUniform:
		*qualifier = QualifierUniform;
		ADVANCE(compiler);
		
	specifier:	
	default:
		return ParseTypeSpecifier(compiler, type);
	}
}

/*
** --------------------------------------------------------------------------
** Expression parsing functions
** --------------------------------------------------------------------------
*/

/**
 * Parse an argument list for a function call or a constructor invocation.
 * 
 * arguments ::= "(" [ assignment_expression { "," assignment_expression } ] ")".
 * 
 * @param	compiler	reference to compiler object
 * @param	numArgs		out: number of arguments parsed
 * @param	args		out: array of expression references
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */ 
static GLboolean ParseArguments(Compiler * compiler, GLsizei * numArgs, Expression *** args) {
	MUSTBE(compiler, TokenTypeLeftParen);

	if (compiler->tokenizer->token.tokenType != TokenTypeRightParen) {
		GLsizei allocated = 4;
		*args = GlesMemoryPoolAllocate(compiler->exprMemory, allocated * sizeof(Expression *));
		
		if (!ParseAssignmentExpression(compiler, &(*args)[(*numArgs)++])) {
			return GL_FALSE;
		}
		
		while (compiler->tokenizer->token.tokenType == TokenTypeComma) {
			ADVANCE(compiler);
			
			if (*numArgs == allocated) {
				Expression ** oldArgs = *args;
				*args = GlesMemoryPoolAllocate(compiler->exprMemory, 2 * allocated * sizeof(Expression *));
				GlesMemcpy(*args, oldArgs, allocated * sizeof(Expression *));
				allocated *= 2;				
			}

			if (!ParseAssignmentExpression(compiler, &(*args)[(*numArgs)++])) {
				return GL_FALSE;
			}			
		}
	}
	
	return MustBe(compiler, TokenTypeRightParen);	
}

/**
 * Detect a primary expression and convert it to an expression object.
 * 
 * primary_expression ::=
 * 		  "(" expression ")"
 * 		| <identifier> [ arguments ]
 * 		| ( "float" | "bool"  | "int" 
 * 		  | "vec2"  | "vec3"  | "vec4" 
 * 		  | "bvec2" | "bvec3" | "bvec4" 
 * 		  | "ivec2" | "ivec3" | "ivec4" 
 * 		  | "mat2"  | "mat3"  | "mat4" ) 
 * 		  arguments
 * 		| "true" 
 * 		| "false"
 * 		| <intConstant>
 * 		| <floatConstant"
 * 
 * @param	compiler	reference to compiler object
 * @param	expression	out: rference to resulting expression
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParsePrimaryExpression(Compiler * compiler, Expression ** expression) {
	TypeValue typeValue;
	Type * type = NULL;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeLeftParen:
		ADVANCE(compiler);
		
		if (!ParseExpression(compiler, expression)) {
			return GL_FALSE;
		}
		
		return MustBe(compiler, TokenTypeRightParen);
		
	case TokenTypeIdentifier:
		{
 			Symbol * symbol = 
 				GlesSymbolFindNested(compiler->currentScope,
 									 compiler->tokenizer->token.s.first,
 									 compiler->tokenizer->token.s.length,
 									 ~0);
 						
 			if (!symbol) {
 				/* L0002: Undefined identifier */
 				GlesCompileError(compiler, ErrL0002);
 				return GL_FALSE;
 			}
 						 
 			if (symbol->base.qualifier == QualifierTypeName) {
 				type = symbol->base.type;
 				goto constructor;
 			}

			ADVANCE(compiler);
		
			if (compiler->tokenizer->token.tokenType == TokenTypeLeftParen) {
				/* function call arguments or constructor arguments */
				GLsizei numArgs = 0;
				Expression ** args = NULL;
				
				if (!ParseArguments(compiler, &numArgs, &args)) {
					return GL_FALSE;
				}
				
				*expression = GlesCreateExprCall(compiler, symbol, numArgs, args);
				
				if (!*expression) {
					return GL_FALSE;
				}
			} else if (symbol->base.qualifier == QualifierConstant) {
				/* constant value */
				*expression = 
					GlesCreateExprConstant(compiler, symbol->base.type, symbol->variable.initializer);
			} else {
				/* variable access */
				*expression = 
					GlesCreateExprReference(compiler, symbol->base.type, symbol, NULL, 0);
				
				if (!*expression) {
					return GL_FALSE;
				}
			}
			
			return GL_TRUE;
		}
		
	case TokenTypeFloat:
	case TokenTypeInt:
	case TokenTypeBool:
	case TokenTypeFloatVec2:
	case TokenTypeFloatVec3:
	case TokenTypeFloatVec4:
	case TokenTypeIntVec2:
	case TokenTypeIntVec3:
	case TokenTypeIntVec4:
	case TokenTypeBoolVec2:
	case TokenTypeBoolVec3:
	case TokenTypeBoolVec4:
	case TokenTypeFloatMat2:
	case TokenTypeFloatMat3:
	case TokenTypeFloatMat4:
		typeValue = DecodeType(compiler);
		type = GlesBasicType(typeValue, PrecisionUndefined);
			
	constructor:
		{
			Expression ** args;
			GLsizei numArgs = 0;
			
			ADVANCE(compiler);
			
			if (!ParseArguments(compiler, &numArgs, &args)) {
				return GL_FALSE;
			}
			
			*expression = GlesCreateExprConstructor(compiler, type, numArgs, args);
			
			if (!expression) {
				return GL_FALSE;
			}
		}
			
		return GL_TRUE;		
		
	case TokenTypeTrue:
	case TokenTypeFalse:
		*expression = 
			GlesCreateExprBoolConstant(compiler, 
									   compiler->tokenizer->token.tokenType == TokenTypeTrue);
		ADVANCE(compiler);
		return GL_TRUE;
		
	case TokenTypeIntConstant:
		*expression =
			GlesCreateExprIntConstant(compiler,
									  compiler->tokenizer->token.value.i);
		ADVANCE(compiler);
		return GL_TRUE;
		
	case TokenTypeFloatConstant:
		*expression =
			GlesCreateExprFloatConstant(compiler,
									    compiler->tokenizer->token.value.f);
		ADVANCE(compiler);
		return GL_TRUE;
		
	case TokenTypeAsmRetval:
		if (!compiler->currentFunction ||
			compiler->currentFunction->base.type->func.returnType->base.kind == TypeVoid) {
			/* Function declared void but return statement has an argument */
			GlesCompileError(compiler, ErrS0039);
			return GL_FALSE;
		}
		
		*expression =
			GlesCreateExprReference(compiler, 
									compiler->currentFunction->base.type->func.returnType,
									compiler->currentFunction->result,
									NULL, 0);
			
		ADVANCE(compiler);
		return GL_TRUE;
	
	default:
		SyntaxError(compiler);
		return GL_FALSE;
	}
}

/**
 * Parse the optional tail of a postfix expression, i.e.
 * 
 * postfix_expression_tail ::=
 * 		{ "." <identifier> | "[" expression "]" | "++" | "--" }.
 * 
 * @param	compiler	reference to compiler object
 * @param	lhs			left hand side of postfix expression
 * @param	expression	out: resulting expression
 *
 * @return 	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParsePostfixExpressionTail(Compiler * compiler, Expression * lhs,
	Expression ** expression) {
	Expression * index;
	
	*expression = lhs;
			
	for (;;) {
		switch (compiler->tokenizer->token.tokenType) {
		case TokenTypeDot:
			ADVANCE(compiler);
			
			if (compiler->tokenizer->token.tokenType != TokenTypeIdentifier) {
				SyntaxError(compiler);
				return GL_FALSE;
			}
			
			*expression = 
				GlesCreateExprField(compiler, *expression, &compiler->tokenizer->token.s);
			
			ADVANCE(compiler);			
			break;
			
		case TokenTypeLeftBracket:
			ADVANCE(compiler);
			
			if (!ParseExpression(compiler, &index)) {
				return GL_FALSE;
			}
			
			*expression =
				GlesCreateExprIndex(compiler, *expression, index);
				
			MUSTBE(compiler, TokenTypeRightBracket);
			break;
			
		case TokenTypeIncOp:
		case TokenTypeDecOp:
			*expression =
				GlesCreateExprPostIncDec(compiler, *expression, 
										 compiler->tokenizer->token.tokenType == TokenTypeIncOp);
										 
			ADVANCE(compiler);
			break;
		
		default:
			return GL_TRUE;
		}

		if (!*expression) {
			return GL_FALSE;
		}		
	}
}

/**
 * Detect a unary expression, i.e.
 * 
 * unary_expression ::=
 * 		primary_expression postfix_expression_tail
 * 		| ( "++" | "--" | "+" | "-" | "!" ) unary_expression.
 * 
 * @param	compiler	reference to compiler object
 * @param	expression	out: resulting expression
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseUnaryExpression(Compiler * compiler, Expression ** expression) {
	
	Expression * result;
	GLboolean plus;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeIncOp:
	case TokenTypeDecOp:
		plus = (compiler->tokenizer->token.tokenType == TokenTypeIncOp);
		ADVANCE(compiler);
		
		if (!ParseUnaryExpression(compiler, &result)) {
			return GL_FALSE;
		}
		
		*expression =
			GlesCreateExprPreIncDec(compiler, result, plus);
			
		return (*expression != NULL);
		
	case TokenTypePlus:
	case TokenTypeDash:
		plus = (compiler->tokenizer->token.tokenType == TokenTypePlus);
		ADVANCE(compiler);
		
		if (!ParseUnaryExpression(compiler, &result)) {
			return GL_FALSE;
		}
		
		*expression =
			GlesCreateExprNegate(compiler, result, plus);
			
		return (*expression != NULL);
		
	case TokenTypeBang:
		ADVANCE(compiler);

		if (!ParseUnaryExpression(compiler, &result)) {
			return GL_FALSE;
		}
		
		*expression =
			GlesCreateExprNot(compiler, result);
			
		return (*expression != NULL);		
		
	default:
		if (!ParsePrimaryExpression(compiler, &result)) {
			return GL_FALSE;
		}
		
		return ParsePostfixExpressionTail(compiler, result, expression);
	}
}

/**
 * Parse a binary expression tail, i.e. a sequence of binary operators
 * and unary expression. This is implemented using simple operator
 * precedence parsing in order to reduce runtime stack requirements.
 * 
 * @param	compiler	reference to compiler object
 * @param	lhs			left-hand side expression
 * @param	expression	out: resulting expression
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */	
static GLboolean ParseBinaryExpressionTail(Compiler * compiler,
	Expression * lhs, Expression ** expression) {
		
	Expression * rhs;
	typedef Expression * (*CondFunctionPtr)(Compiler * compiler, Expression * condition);
	typedef Expression * (*FunctionPtr)(Compiler * compiler, Expression * left, Expression * right);
	
	CondFunctionPtr	condFunc;
	FunctionPtr 	func;
	GLsizei			prec;
	
	struct {
		FunctionPtr		func;
		GLsizei			prec;
		Expression *	expr;
	}	stack[15];
	
	GLsizei sp;
	
	stack[0].func = NULL;
	stack[0].prec = ~0;
	stack[0].expr = lhs;
	
	sp = 0;
	
	for (;;) {
		condFunc = NULL;
		
		switch (compiler->tokenizer->token.tokenType) {
		case TokenTypeStar:				func = GlesCreateExprMul; 		prec = 4; 	break;
		case TokenTypeSlash:			func = GlesCreateExprDiv; 		prec = 4; 	break;

		case TokenTypePlus:				func = GlesCreateExprAdd; 		prec = 5; 	break;
		case TokenTypeDash:				func = GlesCreateExprSub; 		prec = 5; 	break;

		case TokenTypeLeftAngle:		func = GlesCreateExprCompareLT; prec = 7; 	break;
		case TokenTypeRightAngle:		func = GlesCreateExprCompareGT; prec = 7; 	break;
		case TokenTypeLessEqualOp:		func = GlesCreateExprCompareLE; prec = 7; 	break;
		case TokenTypeGreaterEqualOp:	func = GlesCreateExprCompareGE; prec = 7; 	break;

		case TokenTypeEqualOp:			func = GlesCreateExprCompareEQ; prec = 8; 	break;
		case TokenTypeNotEqualOp:		func = GlesCreateExprCompareNE; prec = 8; 	break;

		case TokenTypeAndOp:			condFunc = GlesPrepareExprAnd;
										func = GlesCreateExprAnd; 		prec = 12; 	break;

		case TokenTypeXorOp:			func = GlesCreateExprXor; 		prec = 13; 	break;
		
		case TokenTypeOrOp:				condFunc = GlesPrepareExprOr;
										func = GlesCreateExprOr; 		prec = 14; 	break;

		default:
			goto done;
		}
		
		ADVANCE(compiler);
		
		if (!ParseUnaryExpression(compiler, &rhs)) {
			return GL_FALSE;
		}
		
		while (sp > 0 && stack[sp].prec >= prec) {
			--sp;
			stack[sp].expr =
				stack[sp + 1].func(compiler, stack[sp].expr, stack[sp + 1].expr);
				
			if (!stack[sp].expr) {
				return GL_FALSE;
			}
		}		
		
		if (condFunc) {
			Expression * prep = condFunc(compiler, stack[sp].expr);
			
			if (!prep) {
				return GL_FALSE;
			}
			
			stack[sp].expr = prep;
		}
		
		++sp;
		
		stack[sp].expr = rhs;
		stack[sp].func = func;
		stack[sp].prec = prec;
	}
	
done:
	while (sp > 0) {
		--sp;
		stack[sp].expr =
			stack[sp + 1].func(compiler, stack[sp].expr, stack[sp + 1].expr);
			
		if (!stack[sp].expr) {
			return GL_FALSE;
		}
	}		
	
	*expression = stack[0].expr;
	
	return GL_TRUE;
}

/**
 * Detect an arbitrary binary expression, i.e.
 * 
 * binary_expression ::= unary_expression binary_expression_tail.
 * 
 * @param	compiler	reference to compiler object
 * @param	expression	out: reference to resulting expression
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseBinaryExpression(Compiler * compiler, Expression ** expression) {
	Expression * lhs;
	
	return 
		ParseUnaryExpression(compiler, &lhs) &&
		ParseBinaryExpressionTail(compiler, lhs, expression);
}

/**
 * Detect and process a conditional expression tail. A conditional expression
 * is converted into an if/else sequence.
 * 
 * conditional_expression_tail ::= [ "?" expression ":" assignment_expression ].
 * 
 * @param	compiler	reference to compiler object
 * @param	lhs			left hand side of expression (condition)
 * @param	expression	out: reference to resulting expression
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseConditionalExpressionTail(Compiler * compiler, Expression * lhs, 
	Expression ** expression) {
	
	*expression = lhs;
		
	if (compiler->tokenizer->token.tokenType == TokenTypeQuestion) {
		Expression * trueExpression, * falseExpression;
		Symbol * temp;
		Expression *result;
	
		ADVANCE(compiler);
		
		if (lhs->base.type->base.kind != TypeBool) {
			/* Invalid type for conditional expression */
			GlesCompileError(compiler, ErrS0005);
			return GL_FALSE;
		}
		
		if (lhs->base.kind == ExpressionKindConstant) {
			if (!GlesCreateStmntIf(compiler, lhs, GL_FALSE) ||
				!ParseExpression(compiler, &trueExpression)) {
				return GL_FALSE;
			}

			MUSTBE(compiler, TokenTypeColon);
			
			if (!GlesCreateStmntElse(compiler) ||
				!ParseAssignmentExpression(compiler, &falseExpression)) {
				return GL_FALSE;
			}
			
			if (!GlesTypeMatches(trueExpression->base.type, falseExpression->base.type)) {
				/* Type mismatch for conditional expression */
				GlesCompileError(compiler, ErrS0006);
				return GL_FALSE;
			}
			
			if (!GlesCreateStmntEndif(compiler)) {
				return GL_FALSE;
			}

			if (lhs->constant.value[0].boolValue[0]) {
				*expression = trueExpression;
			} else {
				*expression = falseExpression;
			}
		} else {
			if (!GlesCreateStmntIf(compiler, lhs, GL_FALSE) ||
				!ParseExpression(compiler, &trueExpression)) {
				return GL_FALSE;
			}
			
			temp = GlesDeclareTempVariable(compiler, trueExpression->base.type);
			result = GlesCreateExprReference(compiler, trueExpression->base.type, temp, NULL, 0);
			
			if (trueExpression->base.type->base.kind == TypeArray ||
				!GlesCreateExprAssign(compiler, result, trueExpression)) {
				/* Invalid type for conditional expression */
				GlesCompileError(compiler, ErrS0005);
				return GL_FALSE;
			}
			
			MUSTBE(compiler, TokenTypeColon);
			
			if (!GlesCreateStmntElse(compiler) ||
				!ParseAssignmentExpression(compiler, &falseExpression)) {
				return GL_FALSE;
			}
			
			if (!GlesTypeMatches(trueExpression->base.type, falseExpression->base.type)) {
				/* Type mismatch for conditional expression */
				GlesCompileError(compiler, ErrS0006);
				return GL_FALSE;
			}
			
			if (!GlesCreateExprAssign(compiler, result, falseExpression)) {
				/* Type mismatch for operation */
				GlesCompileError(compiler, ErrS0001);
				return GL_FALSE;
			}
			
			if (!GlesCreateStmntEndif(compiler)) {
				return GL_FALSE;
			}
			
			*expression = result;
		}
	}

	return GL_TRUE;
}

/**
 * Parse a conditional expression, i.e.
 * 
 * conditional_expression ::= binary_expression conditional_expression_tail.
 * 
 * @param	compiler	reference to compiler object
 * @param	expression	out: reference to resulting expression
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseConditionalExpression(Compiler * compiler, Expression ** expression) {
	Expression * lhs;
	
	return
		ParseBinaryExpression(compiler, &lhs) &&
		ParseConditionalExpressionTail(compiler, lhs, expression);
}

/**
 * Parse an assignment expression, i.e.
 * 
 * assignment_expression ::=
 * 		  unary_expression binary_expression conditional_expression_tail
 * 		| unary_expression ( "*=" | "/=" | "+=" | "-=" ) assignment_expression.
 * 
 * @param	compiler	reference to compiler object
 * @param	expression	out: reference to resulting expression
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseAssignmentExpression(Compiler * compiler, Expression ** expression) {
	
	Expression * lhs, *rhs;
	Expression * (*func)(Compiler * compiler, Expression * left, Expression * right);
	
	if (!ParseUnaryExpression(compiler, &lhs)) {
		return GL_FALSE;
	}
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeEqual:		func = GlesCreateExprAssign;	break;
	case TokenTypeMulAssign:	func = GlesCreateExprAssignMul;	break;
	case TokenTypeDivAssign:	func = GlesCreateExprAssignDiv;	break;
	case TokenTypeAddAssign:	func = GlesCreateExprAssignAdd;	break;
	case TokenTypeSubAssign:	func = GlesCreateExprAssignSub;	break;
		 	
	default:
		return
			ParseBinaryExpressionTail(compiler, lhs, &rhs) &&
			ParseConditionalExpressionTail(compiler, rhs, expression);
	}

	ADVANCE(compiler);
	
	if (!ParseAssignmentExpression(compiler, &rhs)) {
		return GL_FALSE;
	}
	
	*expression = func(compiler, lhs, rhs);
	
	return (*expression != NULL);
}

/**
 * Parse a comma separated sequence of expressions, i.e.
 * 
 * expression ::= assignment_expression { "," assignment_expression }.
 * 
 * @param	compiler	reference to compiler object
 * @param	expression	out: reference to resulting expression object
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseExpression(Compiler * compiler, Expression ** expression) {
	if (!ParseAssignmentExpression(compiler, expression)) {
		return GL_FALSE;
	}
	
	while (compiler->tokenizer->token.tokenType == TokenTypeComma) {
		ADVANCE(compiler);
		
		if (!ParseAssignmentExpression(compiler, expression)) {
			return GL_FALSE;
		}
	}
	
	return GL_TRUE;
}

/**
 * Parse a constant expression, i.e a conditional expression that evaluates to a constant.
 * 
 * @param	compiler	reference to compiler object
 * @param	expression	out: reference to resulting value
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseConstantExpression(Compiler * compiler, Expression ** expression) {
	GLboolean oldConstExpression = compiler->constExpression;
	GLsizei oldCount = compiler->generator.instructionCount;
	BlockList * oldBlockList = compiler->generator.currentList;
	
	compiler->constExpression = GL_TRUE;
	compiler->generator.currentList = NULL;				/* suppress instruction generation	*/
	
	if (!ParseConditionalExpression(compiler, expression)) {
		return GL_FALSE;
	}
	
	if ((*expression)->base.kind != ExpressionKindConstant ||
		compiler->generator.instructionCount != oldCount) {
		/* S0012: expression is not constant */
		GlesCompileError(compiler, ErrS0012);
		return GL_FALSE;
	}
	
	compiler->constExpression = oldConstExpression;
	compiler->generator.currentList = oldBlockList;		/* resume instruction generation	*/
	
	return GL_TRUE;
}


/*
** --------------------------------------------------------------------------
** Statement parsing functions
** --------------------------------------------------------------------------
*/

/**
 * Parse a local declaration or an expression statement beginning with a constructor.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */ 
static GLboolean ParseConstructorOrDeclaration(Compiler * compiler) {
	Type * baseType = NULL, *type = NULL;
	Expression * lhs = NULL, *rhs, *expression;
	
	if (!ParseTypeSpecifierNoPrec(compiler, PrecisionUndefined, &baseType)) {
		return GL_FALSE;
	}
	
	if (compiler->tokenizer->token.tokenType == TokenTypeLeftParen) {
		Expression ** args;
		GLsizei numArgs;
		
		ADVANCE(compiler);
		
		if (!ParseArguments(compiler, &numArgs, &args) ||
			!GlesCreateExprConstructor(compiler, baseType, numArgs, args)) {
			return GL_FALSE;
		}

		return 
			ParsePostfixExpressionTail(compiler, lhs, &rhs) &&
			ParseBinaryExpressionTail(compiler, rhs, &lhs) 	&&
			ParseConditionalExpressionTail(compiler, lhs, &expression);
	} else {
		TokenString name = compiler->tokenizer->token.s;
		MUSTBE(compiler, TokenTypeIdentifier);
		
		Expression * initializer;
		
		if (!ParseOptArrayOrInitializer(compiler, baseType, &type, &initializer) ||
			!GlesDeclareVariable(compiler, 
								 name.first, name.length,
								 QualifierVariable, type, GL_FALSE,
								 initializer)) {
			return GL_FALSE;
		}
		
		if (!ParseInitDeclaratorListTail(compiler, baseType, QualifierVariable, GL_FALSE)) {
			return GL_FALSE;
		}
		
		return MustBe(compiler, TokenTypeSemicolon);
	}
}

/**
 * Parse a local declaration or an expression statement.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */ 
static GLboolean ParseExpressionOrDeclaration(Compiler * compiler) {
	Expression * expression;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeIdentifier:
		{
 			Symbol * symbol = 
 				GlesSymbolFindNested(compiler->currentScope,
 									 compiler->tokenizer->token.s.first,
 									 compiler->tokenizer->token.s.length,
 									 ~0);
 						
 			if (symbol && symbol->base.qualifier == QualifierTypeName) {
				return ParseConstructorOrDeclaration(compiler);
 			} else {
				return ParseExpression(compiler, &expression) &&
					MustBe(compiler, TokenTypeSemicolon);
 			}
		}
		
		break;
		
	case TokenTypeLeftParen:
	case TokenTypeIntConstant:
	case TokenTypeTrue:
	case TokenTypeFalse:
	case TokenTypeFloatConstant:
	case TokenTypeAsmRetval:
	case TokenTypeIncOp:
	case TokenTypeDecOp:
	case TokenTypePlus:
	case TokenTypeDash:
	case TokenTypeBang:
	case TokenTypeTilde:
		return ParseExpression(compiler, &expression) &&
			MustBe(compiler, TokenTypeSemicolon);
			
	case TokenTypeFloat:
	case TokenTypeInt:
	case TokenTypeBool:
	case TokenTypeFloatVec2:
	case TokenTypeFloatVec3:
	case TokenTypeFloatVec4:
	case TokenTypeIntVec2:
	case TokenTypeIntVec3:
	case TokenTypeIntVec4:
	case TokenTypeBoolVec2:
	case TokenTypeBoolVec3:
	case TokenTypeBoolVec4:
	case TokenTypeFloatMat2:
	case TokenTypeFloatMat3:
	case TokenTypeFloatMat4:
		return ParseConstructorOrDeclaration(compiler);
		      
    default:
    	return ParseLocalDeclaration(compiler); 
	}
}

/**
 * Parse an if statement, i.e.
 * 
 * if_statement ::= "if" "(" expression ")" statement [ "else" statement ].
 * 
 * @param	compiler		reference to compiler object
 * @param	continuation	out: reference to continuation object
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */ 
static GLboolean ParseIfStatement(Compiler * compiler, ForLoop ** continuation, GLuint * continuationMask) {
	Expression * condition;
	GLuint trueMask = 0, falseMask = 0;
	GLboolean returnValue;
	
	MUSTBE(compiler, TokenTypeIf);
	MUSTBE(compiler, TokenTypeLeftParen);
	
	if (!ParseExpression(compiler, &condition) ||
		!GlesCreateStmntIf(compiler, condition, GL_FALSE)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeRightParen);
	
	if (!ParseStatement(compiler, continuation, &trueMask)) {
		return GL_FALSE;
	}
	
	if (compiler->tokenizer->token.tokenType == TokenTypeElse) {
		ADVANCE(compiler);
		
		returnValue =
		 	GlesCreateStmntElse(compiler) &&
		 	ParseStatement(compiler, continuation, &falseMask) &&
		 	GlesCreateStmntEndif(compiler);
		
		*continuationMask = trueMask & falseMask;
		
		return returnValue;
	} else {
		*continuationMask = trueMask;
		
		return GlesCreateStmntEndif(compiler);
	}
}

/**
 * Parse an while statement, i.e.
 * 
 * while_statement ::= "while" "(" expression ")" statement.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */ 
static GLboolean ParseWhileStatement(Compiler * compiler, GLuint * continuationMask) {
	Expression * condition;
	GLboolean result;
	ForLoop * continuation = NULL;
	GLuint subMask = 0;
	
	MUSTBE(compiler, TokenTypeWhile);
	MUSTBE(compiler, TokenTypeLeftParen);
	
	if (!ParseExpression(compiler, &condition)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeRightParen);
	
	if (!GlesCreateStmntWhile(compiler, condition)) {
		return GL_FALSE;
	}
	
	result = ParseStatement(compiler, &continuation, &subMask);
	
	/* TODO: can update mask more precisely if condition is constant expression */
	*continuationMask = 0;
	
	return GlesCreateStmntEndWhile(compiler);
}

/**
 * Parse an do-while statement, i.e.
 * 
 * do_statement ::= "do" statement "while" "(" expression ")" ";".
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */ 
static GLboolean ParseDoStatement(Compiler * compiler, GLuint * continuationMask)  {
	Expression * condition;
	ForLoop * continuation = NULL;
	GLuint subMask = 0;
	
	MUSTBE(compiler, TokenTypeDo);
	
	if (GlesCreateStmntDo(compiler) ||
		!ParseStatement(compiler, &continuation, &subMask)) {
		return GL_FALSE;
	}
	
	*continuationMask = 
		subMask & (ContinuationMaskReturn | ContinuationMaskDiscard);
		
	MUSTBE(compiler, TokenTypeWhile);
	MUSTBE(compiler, TokenTypeLeftParen);
	
	if (!ParseExpression(compiler, &condition)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeRightParen);
	
	return 
		MustBe(compiler, TokenTypeSemicolon) &&
		GlesCreateStmntEndDo(compiler, condition);
}

/**
 * Parse the condition clause for a for statement. This clause is 
 * restricted according to Appendix A of the GLSL ES specification, i.e.
 * 
 * for_condition ::= identifier rel_op constant_expression
 * 
 * @param	compiler	reference to compiler object
 * @param	loopIndex	out: reference to loop index variable
 * @param	condition	out: reference to comparison operator
 * @param	boundary	out: iteration bound expression
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseForCondition(Compiler * compiler, Symbol ** loopIndex,
								   Cond * condition, Expression ** boundary) {
	if (compiler->tokenizer->token.tokenType != TokenTypeIdentifier) {
		SyntaxError(compiler);
		return GL_FALSE;
	}
	
	*loopIndex =
		GlesSymbolFindNested(compiler->currentScope,
							 compiler->tokenizer->token.s.first,
							 compiler->tokenizer->token.s.length,
							 ~0);
		
	if (!*loopIndex) {
		/* L0002: Undefined identifier */
		GlesCompileError(compiler, ErrL0002);
		return GL_FALSE;
	}
	
	ADVANCE(compiler);
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeEqualOp:			*condition = CondEQ; break;
	case TokenTypeNotEqualOp:		*condition = CondNE; break;
	case TokenTypeLeftAngle:		*condition = CondLT; break;
	case TokenTypeLessEqualOp:		*condition = CondLE; break;
	case TokenTypeRightAngle:		*condition = CondGT; break;
	case TokenTypeGreaterEqualOp:	*condition = CondGE; break;
		
	default:
		SyntaxError(compiler);
		return GL_FALSE;
	}
	
	ADVANCE(compiler);
	
	return ParseConstantExpression(compiler, boundary);
}

/**
 * Parse the index variable increment clause for a for-statement, i.e.
 * 
 * loop_increment ::=
 * 		  loop_index++ 
 * 		| loop_index--
 * 		| loop_index += constant_expression
 * 		| loop_index -= constant_expression. 
 * 
 * @param	compiler	reference to compiler object
 * @param	loopIndex	the loop index variable
 * @param	increment	out: constant loop increment value
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseLoopIncrement(Compiler * compiler, Symbol * loopIndex,
									Expression ** increment) {
	
	Expression * value;
	Expression * lhs = GlesCreateExprReference(compiler, loopIndex->base.type, loopIndex, NULL, 0);
	
	GLES_ASSERT(compiler->tokenizer->token.tokenType == TokenTypeIdentifier);
	GLES_ASSERT(loopIndex->base.length == compiler->tokenizer->token.s.length);
	GLES_ASSERT(!GlesMemcmp(compiler->tokenizer->token.s.first,
							loopIndex->base.name, loopIndex->base.length));
							
	MUSTBE(compiler, TokenTypeIdentifier);
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeIncOp:
		ADVANCE(compiler);

		if (loopIndex->base.type->base.kind == TypeInt) {
			*increment = GlesCreateExprIntConstant(compiler, 1);
		} else { 
			GLES_ASSERT(loopIndex->base.type->base.kind == TypeFloat);
			*increment = GlesCreateExprFloatConstant(compiler, 1.0f);
		}
		
		break;
		
	case TokenTypeDecOp:
		ADVANCE(compiler);

		if (loopIndex->base.type->base.kind == TypeInt) {
			*increment = GlesCreateExprIntConstant(compiler, -1);
		} else { 
			GLES_ASSERT(loopIndex->base.type->base.kind == TypeFloat);
			*increment = GlesCreateExprFloatConstant(compiler, -1.0f);
		}
		
		break;
		
	case TokenTypeAddAssign:
		ADVANCE(compiler);
		
		return ParseConstantExpression(compiler, increment);
		
	case TokenTypeSubAssign:
		ADVANCE(compiler);

		if (!ParseConstantExpression(compiler, &value)) {
			return GL_FALSE;
		}
		
		*increment = GlesCreateExprNegate(compiler, value, GL_FALSE);
		return (*increment != NULL);
		
	default:
		SyntaxError(compiler);
		break;
	}
	
	return GlesCreateExprAssignAdd(compiler, lhs, *increment) != NULL;	
}

/**
 * Parse the increment clause for a for-statement, i.e.
 * 
 * increments ::=
 * 		( loop_increment | assignment_expression ) 
 * 		{ "," ( loop_increment | assignment_expression ) }.   
 * 
 * @param	compiler	reference to compiler object
 * @param	loopIndex	the loop index variable
 * @param	increment	out: constant loop increment value
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseLoopIncrements(Compiler * compiler, Symbol * loopIndex,
									 Expression ** increment) {
	GLboolean seenIncrement = GL_FALSE;
	
	if (compiler->tokenizer->token.tokenType == TokenTypeIdentifier &&
		loopIndex->base.length == compiler->tokenizer->token.s.length &&
		!GlesMemcmp(compiler->tokenizer->token.s.first,
					loopIndex->base.name, loopIndex->base.length)) {
		if (!ParseLoopIncrement(compiler, loopIndex, increment)) {
			return GL_FALSE;
		} else {
			seenIncrement = GL_TRUE;
		}
	} else {
		Expression * ignore = NULL;
		
		if (!ParseAssignmentExpression(compiler, &ignore)) {
			return GL_FALSE;
		}
	}
	
	while (compiler->tokenizer->token.tokenType == TokenTypeComma) {
		if (compiler->tokenizer->token.tokenType == TokenTypeIdentifier &&
			loopIndex->base.length == compiler->tokenizer->token.s.length &&
			!GlesMemcmp(compiler->tokenizer->token.s.first,
						loopIndex->base.name, loopIndex->base.length)) {
			if (!ParseLoopIncrement(compiler, loopIndex, increment)) {
				return GL_FALSE;
			} else {
				if (seenIncrement) {
					/* Loop index can be incremented only once */
					GlesCompileError(compiler, ErrX0003);
					return GL_FALSE;
				} else {
					seenIncrement = GL_TRUE;
				}
			}
		} else {
			Expression * ignore = NULL;
			
			if (!ParseAssignmentExpression(compiler, &ignore)) {
				return GL_FALSE;
			}
		}
	}
	
	return GL_TRUE;
}
	
/**
 * Find the initial value for the given variable in the initializer list.
 * 
 * @param	initializer		list of initializations
 * @param	variable		variable to find in list
 * 
 * @return	constant initial value, or NULL if not found
 */ 
static Expression * ExtractInitialValue(InitializerList * initializer, Symbol * variable) {
	for (; initializer; initializer = initializer->next) {
		if (initializer->symbol == variable) {
			return initializer->value;
		}
	}
	
	return NULL;
}
								
/**
 * Parse a for statement, i.e.
 * 
 * for_statement ::= 
 * 		"for" "(" expression_or_declaration ";" [ for_condition ] ";" loop_increments ")" 
 * 		statement.
 * 
 * The for statement is really the only case where the lack of a parse tree makes the
 * processing a little bit more complicated. This would actually be much easier, if the
 * surface syntax chosen would that of Ada or Oberon.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if successful, GL_FALSE otherwise
 */ 
static GLboolean ParseForStatement(Compiler * compiler, GLuint * continuationMask) {
	Symbol * loopIndex = NULL;
	Cond loopCondition = CondF;
	Expression *initial = NULL, *increment = NULL, *boundary = NULL;
	ForLoop * continuation = NULL;
	GLuint subMask;

	GLboolean result;
	Scope *newScope =
		GlesScopeCreate(compiler->moduleMemory, compiler->currentScope);
	BlockList incrementBlocks, *oldList;
	
	incrementBlocks.head = incrementBlocks.tail = NULL;
	
	MUSTBE(compiler, TokenTypeFor);
	MUSTBE(compiler, TokenTypeLeftParen);
	
	compiler->currentScope = newScope;
	compiler->collectInitializers = GL_TRUE;
	compiler->initializers = NULL;
	
	/* all constant initializations of the for_init_part need to be captured */
	if (compiler->tokenizer->token.tokenType != TokenTypeSemicolon) {
		if (!ParseExpressionOrDeclaration(compiler)) {
			return GL_FALSE;
		}
	} else {
		ADVANCE(compiler);
	}
	
	compiler->collectInitializers = GL_FALSE;
	
	if (!ParseForCondition(compiler, &loopIndex, &loopCondition, &boundary)) {
		return GL_FALSE;
	}
	
	initial = ExtractInitialValue(compiler->initializers, loopIndex);
	
	MUSTBE(compiler, TokenTypeSemicolon);
	
	/* increments need to be generated into separate block list */
	oldList = compiler->generator.currentList;
	compiler->generator.currentList = &incrementBlocks;
	GlesCreateBlock(&compiler->generator);
	
	if (!ParseLoopIncrements(compiler, loopIndex, &increment)) {
		return GL_FALSE;
	}

	/* switch back to original instruction sequence */	
	compiler->generator.currentList = oldList;
	MUSTBE(compiler, TokenTypeRightParen);
	
	/* gen loop header with number of iterations */
	if (!GlesCreateStmntFor(compiler, loopIndex, initial, boundary, increment, loopCondition)) {
		return GL_FALSE;
	}
	
	result = ParseStatement(compiler, &continuation, &subMask);
	GLES_ASSERT(!continuation || continuation == compiler->currentLoop);
	
	/* merge increment instruction sequence */
	GlesInsertBlockList(&compiler->generator, &incrementBlocks);
	
	/* gen loop end */
	GlesCreateStmntEndFor(compiler);
	
	compiler->currentScope = compiler->currentScope->parent;	
	*continuationMask = 0;
	
	return result;
}

/**
 * Parse a break statement, i.e.
 * 
 * break_statement ::= "break" ";'.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return 	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseBreakStatement(Compiler * compiler, GLuint * continuationMask) {
	MUSTBE(compiler, TokenTypeBreak);
	
	*continuationMask = ContinuationMaskBreak;
	return GlesCreateStmntBreak(compiler) && MustBe(compiler, TokenTypeSemicolon);
}

/**
 * Parse a continue statement, i.e.
 * 
 * continue_statement ::= "continue" ";'.
 * 
 * @param	compiler		reference to compiler object
 * @param	continuation	out: reference to continuation object
 * 
 * @return 	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseContinueStatement(Compiler * compiler, ForLoop ** continuation, GLuint * continuationMask) {
	MUSTBE(compiler, TokenTypeContinue);

	*continuation = compiler->currentLoop;
	*continuationMask = ContinuationMaskContinue;
		
	return GlesCreateStmntContinue(compiler) && MustBe(compiler, TokenTypeSemicolon);
}

/**
 * Parse a return statement, i.e.
 * 
 * return_statement ::= "return" [ expression ] ";".
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseReturnStatement(Compiler * compiler, GLuint * continuationMask) {
	Expression * expression;
	
	MUSTBE(compiler, TokenTypeReturn);
	
	if (compiler->tokenizer->token.tokenType != TokenTypeSemicolon) {
		if (!ParseExpression(compiler, &expression) ||
			!GlesCreateStmntReturn(compiler, expression)) {
			return GL_FALSE;
		}
	}
	
	*continuationMask = ContinuationMaskReturn;
	return MustBe(compiler, TokenTypeSemicolon);
}

/**
 * Parse a discard statement, i.e.
 * 
 * discard_statement ::= "discard" ";'.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return 	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseDiscardStatement(Compiler * compiler, GLuint * continuationMask) {
	MUSTBE(compiler, TokenTypeDiscard);
	
	*continuationMask = ContinuationMaskDiscard;
	return GlesCreateStmntDiscard(compiler) && MustBe(compiler, TokenTypeSemicolon);
}

/**
 * Parse a unary assembly instruction
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return 	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseAsmUnary(Compiler * compiler, GLuint * continuationMask) {

	Opcode			opcode;
	Expression * 	result;
	Expression *	operand;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeAsmABS:		opcode = OpcodeABS;			break;
	case TokenTypeAsmABS_SAT:	opcode = OpcodeABS_SAT;		break;
	case TokenTypeAsmCOS:		opcode = OpcodeCOS;			break;
	case TokenTypeAsmCOS_SAT:	opcode = OpcodeCOS_SAT;		break;
	case TokenTypeAsmEX2:		opcode = OpcodeEX2;			break;
	case TokenTypeAsmEX2_SAT:	opcode = OpcodeEX2_SAT;		break;
	case TokenTypeAsmEXP:		opcode = OpcodeEXP;			break;
	case TokenTypeAsmEXP_SAT:	opcode = OpcodeEXP_SAT;		break;
	case TokenTypeAsmFLR:		opcode = OpcodeFLR;			break;
	case TokenTypeAsmFLR_SAT:	opcode = OpcodeFLR_SAT;		break;
	case TokenTypeAsmFRC:		opcode = OpcodeFRC;			break;
	case TokenTypeAsmFRC_SAT:	opcode = OpcodeFRC_SAT;		break;
	case TokenTypeAsmLG2:		opcode = OpcodeLG2;			break;
	case TokenTypeAsmLG2_SAT:	opcode = OpcodeLG2_SAT;		break;
	case TokenTypeAsmLOG:		opcode = OpcodeLOG;			break;
	case TokenTypeAsmLOG_SAT:	opcode = OpcodeLOG_SAT;		break;
	case TokenTypeAsmMOV:		opcode = OpcodeMOV;			break;
	case TokenTypeAsmMOV_SAT:	opcode = OpcodeMOV_SAT;		break;
	case TokenTypeAsmRCP:		opcode = OpcodeRCP;			break;
	case TokenTypeAsmRCP_SAT:	opcode = OpcodeRCP_SAT;		break;
	case TokenTypeAsmRSQ:		opcode = OpcodeRSQ;			break;
	case TokenTypeAsmRSQ_SAT:	opcode = OpcodeRSQ_SAT;		break;
	case TokenTypeAsmSCS:		opcode = OpcodeSCS;			break;
	case TokenTypeAsmSCS_SAT:	opcode = OpcodeSCS_SAT;		break;
	case TokenTypeAsmSIN: 		opcode = OpcodeSIN;			break;
	case TokenTypeAsmSIN_SAT:   opcode = OpcodeSIN_SAT;		break;
	case TokenTypeAsmSSG:		opcode = OpcodeSSG;			break;
	
	default:
		GLES_ASSERT(GL_FALSE);
		return GL_FALSE;
	}
	
	ADVANCE(compiler);
	
	if (!ParseAssignmentExpression(compiler, &result)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeComma);
	
	if (!ParseAssignmentExpression(compiler, &operand)) {
		return GL_FALSE;
	}
	
	if (compiler->tokenizer->token.tokenType == TokenTypeSemicolon) {
		ADVANCE(compiler);
	}
	
	*continuationMask = 0;
	
	return GlesAsmUnary(compiler, opcode, result, operand);
}

/**
 * Parse a binary assembly instruction
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return 	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseAsmBinary(Compiler * compiler, GLuint * continuationMask) {

	Opcode			opcode;
	
	Expression * 	result;
	Expression *	left;
	Expression *	right;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeAsmADD:		opcode = OpcodeADD;		break;	
	case TokenTypeAsmADD_SAT:	opcode = OpcodeADD_SAT;	break;	
	case TokenTypeAsmDP2:		opcode = OpcodeDP2;		break;
	case TokenTypeAsmDP2_SAT:	opcode = OpcodeDP2_SAT;	break;
	case TokenTypeAsmDP3:		opcode = OpcodeDP3;		break;
	case TokenTypeAsmDP3_SAT:	opcode = OpcodeDP3_SAT;	break;
	case TokenTypeAsmDP4:		opcode = OpcodeDP4;		break;
	case TokenTypeAsmDP4_SAT:	opcode = OpcodeDP4_SAT;	break;
	case TokenTypeAsmDPH:		opcode = OpcodeDPH;		break;
	case TokenTypeAsmDPH_SAT:	opcode = OpcodeDPH_SAT;	break;
	case TokenTypeAsmDST:		opcode = OpcodeDST;		break;
	case TokenTypeAsmDST_SAT:	opcode = OpcodeDST_SAT;	break;
	case TokenTypeAsmMAX:		opcode = OpcodeMAX;		break;
	case TokenTypeAsmMAX_SAT:	opcode = OpcodeMAX_SAT;	break;
	case TokenTypeAsmMIN:		opcode = OpcodeMIN;		break;
	case TokenTypeAsmMIN_SAT:	opcode = OpcodeMIN_SAT;	break;
	case TokenTypeAsmMUL:		opcode = OpcodeMUL;		break;
	case TokenTypeAsmMUL_SAT:	opcode = OpcodeMUL_SAT;	break;
	case TokenTypeAsmPOW:		opcode = OpcodePOW;		break;
	case TokenTypeAsmSEQ:   	opcode = OpcodeSEQ;		break;
	case TokenTypeAsmSFL:  		opcode = OpcodeSFL;		break;
	case TokenTypeAsmSGE:		opcode = OpcodeSGE;		break;
	case TokenTypeAsmSGT:		opcode = OpcodeSGT;		break;
	case TokenTypeAsmSLE:		opcode = OpcodeSLE;		break;
	case TokenTypeAsmSLT:   	opcode = OpcodeSLE;		break;
	case TokenTypeAsmSNE:   	opcode = OpcodeSNE;		break;
	case TokenTypeAsmSTR:		opcode = OpcodeSTR;		break;
	case TokenTypeAsmSUB:		opcode = OpcodeSUB;		break;
	case TokenTypeAsmSUB_SAT:	opcode = OpcodeSUB_SAT;	break;
	case TokenTypeAsmXPD:		opcode = OpcodeXPD;		break;
	case TokenTypeAsmXPD_SAT:	opcode = OpcodeXPD_SAT;	break;
		
	default:
		GLES_ASSERT(GL_FALSE);
		return GL_FALSE;
	}

	ADVANCE(compiler);
	
	if (!ParseAssignmentExpression(compiler, &result)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeComma);
	
	if (!ParseAssignmentExpression(compiler, &left)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeComma);
	
	if (!ParseAssignmentExpression(compiler, &right)) {
		return GL_FALSE;
	}
	
	if (compiler->tokenizer->token.tokenType == TokenTypeSemicolon) {
		ADVANCE(compiler);
	}
	
	*continuationMask = 0;

	return GlesAsmBinary(compiler, opcode, result, left, right);
}

/**
 * Parse a ternary assembly instruction
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return 	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseAsmTernary(Compiler * compiler, GLuint * continuationMask) {

	Opcode			opcode;
	
	Expression * 	result;
	Expression *	first;
	Expression *	second;
	Expression *	third;
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeAsmCMP:		opcode = OpcodeCMP;		break;
	case TokenTypeAsmCMP_SAT:	opcode = OpcodeCMP_SAT;	break;
	case TokenTypeAsmLRP:		opcode = OpcodeLRP;		break;
	case TokenTypeAsmLRP_SAT:	opcode = OpcodeLRP_SAT;	break;
	case TokenTypeAsmMAD:		opcode = OpcodeMAD;		break;
	case TokenTypeAsmMAD_SAT:	opcode = OpcodeMAD_SAT;	break;
				
	default:
		GLES_ASSERT(GL_FALSE);
		return GL_FALSE;
	}

	ADVANCE(compiler);
	
	if (!ParseAssignmentExpression(compiler, &result)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeComma);
	
	if (!ParseAssignmentExpression(compiler, &first)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeComma);
	
	if (!ParseAssignmentExpression(compiler, &second)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeComma);
	
	if (!ParseAssignmentExpression(compiler, &third)) {
		return GL_FALSE;
	}
	
	if (compiler->tokenizer->token.tokenType == TokenTypeSemicolon) {
		ADVANCE(compiler);
	}
	
	*continuationMask = 0;

	return GlesAsmTernary(compiler, opcode, result, first, second, third);
}

/**
 * Parse a texture assembly instruction
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return 	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseAsmTexture(Compiler * compiler, GLuint * continuationMask) {

	Opcode			opcode;
	
	Expression * 	result;
	Expression *	coords;
	Expression *	sampler;
	
	switch (compiler->tokenizer->token.tokenType) {		
	case TokenTypeAsmTEX:		opcode = OpcodeTEX;		break;
	case TokenTypeAsmTEX_SAT:	opcode = OpcodeTEX_SAT;	break;
	case TokenTypeAsmTXB:		opcode = OpcodeTXB;		break;
	case TokenTypeAsmTXB_SAT:	opcode = OpcodeTXB_SAT;	break;
	case TokenTypeAsmTXL:		opcode = OpcodeTXL;		break;
	case TokenTypeAsmTXL_SAT:	opcode = OpcodeTXL_SAT;	break;
	case TokenTypeAsmTXP:		opcode = OpcodeTXP;		break;
	case TokenTypeAsmTXP_SAT:	opcode = OpcodeTXP_SAT;	break;
	
	default:
		GLES_ASSERT(GL_FALSE);
		return GL_FALSE;
	}

	ADVANCE(compiler);
	
	if (!ParseAssignmentExpression(compiler, &result)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeComma);
	
	if (!ParseAssignmentExpression(compiler, &coords)) {
		return GL_FALSE;
	}
	
	MUSTBE(compiler, TokenTypeComma);
	
	if (!ParseAssignmentExpression(compiler, &sampler)) {
		return GL_FALSE;
	}
	
	if (compiler->tokenizer->token.tokenType == TokenTypeSemicolon) {
		ADVANCE(compiler);
	}
	
	*continuationMask = 0;

	return GlesAsmTexture(compiler, opcode, result, coords, sampler);
}

/**
 * Parse a general statement, i.e.
 * 
 * statement ::=
 *		  empty_statement
 * 		| compound_statement
 * 		| if_statement
 * 		| while_statement
 * 		| do_statement
 * 		| for_statement
 * 		| break_statement
 * 		| continue_statement
 * 		| discard_statement
 * 		| return_statement
 * 		| expression_or_declaration.
 * 
 * @param	compiler		reference to compiler object
 * @param	continuation	out: reference to continuation object
 * 
 * @return 	GL_TRUE if successful, GL_FALSE otherwise
 */
static GLboolean ParseStatement(Compiler * compiler, ForLoop ** continuation, GLuint * continuationMask) {
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeSemicolon:
		/* empty statement */
		ADVANCE(compiler);
		return GL_TRUE;
	
	case TokenTypeLeftBrace:
		return ParseCompoundStatement(compiler, NULL, continuation, continuationMask);
		
	case TokenTypeIf:
		return ParseIfStatement(compiler, continuation, continuationMask);
		
	case TokenTypeWhile:
		return ParseWhileStatement(compiler, continuationMask); 
		
	case TokenTypeDo:
		return ParseDoStatement(compiler, continuationMask); 
		
	case TokenTypeFor:
		return ParseForStatement(compiler, continuationMask); 
		
	case TokenTypeBreak:
		return ParseBreakStatement(compiler, continuationMask); 
		
	case TokenTypeContinue:
		return ParseContinueStatement(compiler, continuation, continuationMask); 
		
	case TokenTypeReturn:
		return ParseReturnStatement(compiler, continuationMask); 
		
	case TokenTypeDiscard:
		return ParseDiscardStatement(compiler, continuationMask);
		
	case TokenTypeAsmABS:
	case TokenTypeAsmABS_SAT:
	case TokenTypeAsmCOS:	
	case TokenTypeAsmCOS_SAT:	
	case TokenTypeAsmEX2:	
	case TokenTypeAsmEX2_SAT:	
	case TokenTypeAsmEXP:	
	case TokenTypeAsmEXP_SAT:	
	case TokenTypeAsmFLR:
	case TokenTypeAsmFLR_SAT:
	case TokenTypeAsmFRC:	
	case TokenTypeAsmFRC_SAT:	
	case TokenTypeAsmLG2:
	case TokenTypeAsmLG2_SAT:
	case TokenTypeAsmLOG:	
	case TokenTypeAsmLOG_SAT:	
	case TokenTypeAsmMOV:
	case TokenTypeAsmMOV_SAT:
	case TokenTypeAsmRCP:	
	case TokenTypeAsmRCP_SAT:	
	case TokenTypeAsmRSQ:	
	case TokenTypeAsmRSQ_SAT:	
	case TokenTypeAsmSCS:
	case TokenTypeAsmSCS_SAT:
	case TokenTypeAsmSIN:    
	case TokenTypeAsmSIN_SAT:    
	case TokenTypeAsmSSG:
		return ParseAsmUnary(compiler, continuationMask);
		
	case TokenTypeAsmADD:	
	case TokenTypeAsmADD_SAT:	
	case TokenTypeAsmDP2:	
	case TokenTypeAsmDP2_SAT:	
	case TokenTypeAsmDP3:
	case TokenTypeAsmDP3_SAT:
	case TokenTypeAsmDP4:	
	case TokenTypeAsmDP4_SAT:	
	case TokenTypeAsmDPH:	
	case TokenTypeAsmDPH_SAT:	
	case TokenTypeAsmDST:
	case TokenTypeAsmDST_SAT:
	case TokenTypeAsmMAX:	
	case TokenTypeAsmMAX_SAT:	
	case TokenTypeAsmMIN:	
	case TokenTypeAsmMIN_SAT:	
	case TokenTypeAsmMUL:	
	case TokenTypeAsmMUL_SAT:	
	case TokenTypeAsmPOW:
	case TokenTypeAsmSEQ:    
	case TokenTypeAsmSFL:  	
	case TokenTypeAsmSGE:
	case TokenTypeAsmSGT:	
	case TokenTypeAsmSLE:
	case TokenTypeAsmSLT:    
	case TokenTypeAsmSNE:    
	case TokenTypeAsmSTR:	
	case TokenTypeAsmSUB:	
	case TokenTypeAsmSUB_SAT:	
	case TokenTypeAsmXPD:
	case TokenTypeAsmXPD_SAT:
		return ParseAsmBinary(compiler, continuationMask);

	case TokenTypeAsmCMP:	
	case TokenTypeAsmCMP_SAT:	
	case TokenTypeAsmLRP:	
	case TokenTypeAsmLRP_SAT:	
	case TokenTypeAsmMAD:
	case TokenTypeAsmMAD_SAT:
		return ParseAsmTernary(compiler, continuationMask);
	
	case TokenTypeAsmTEX:	
	case TokenTypeAsmTEX_SAT:	
	case TokenTypeAsmTXB:	
	case TokenTypeAsmTXB_SAT:	
	case TokenTypeAsmTXL:
	case TokenTypeAsmTXL_SAT:
	case TokenTypeAsmTXP:
	case TokenTypeAsmTXP_SAT:
		return ParseAsmTexture(compiler, continuationMask);
	
	default:
		*continuationMask = 0;
	
		return ParseExpressionOrDeclaration(compiler); 
	}
}

/**
 * Parse a compound statement, i.e.
 * 
 * compound_statement ::= "{" { statement } "}".
 * 
 * @param	compiler		reference to compiler object
 * @param	newScope		a specific scope to use within this construction; if NULL,
 * 							an internal scope will be created
 * @param	continuation	out: reference to continuation object
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseCompoundStatement(Compiler * compiler, Scope * newScope, ForLoop ** continuation, GLuint * continuationMask) {
	MUSTBE(compiler, TokenTypeLeftBrace);
	
	ForLoop * innerContinuation = NULL, *prevContinuation = NULL;
	GLboolean createdIfBlock = GL_FALSE;
	GLuint accumulatedMask = 0, innerMask;
	GLboolean unreachable = GL_FALSE;
	
	if (!newScope) {
		newScope =
			GlesScopeCreate(compiler->moduleMemory, compiler->currentScope);
	}
			
	compiler->currentScope = newScope;
	
	while (compiler->tokenizer->token.tokenType != TokenTypeRightBrace) {
		if (!createdIfBlock && !prevContinuation && innerContinuation) {
			Expression * flagExpr = 
				GlesCreateExprReference(compiler, innerContinuation->continueFlag->base.type,
					innerContinuation->continueFlag, NULL, 0);
					
			GlesCreateStmntIf(compiler, flagExpr, GL_TRUE);
			createdIfBlock = GL_TRUE;
		}

		prevContinuation = innerContinuation;
		
		if (accumulatedMask && !unreachable) {
			/* unreachable code */
			/*GlesCompileWarning(compiler, ErrW0001);*/
			unreachable = GL_TRUE;
		}
		
		if (!ParseStatement(compiler, &innerContinuation, &innerMask)) {
			return GL_FALSE;
		}
		
		accumulatedMask |= innerMask;
	}
	
	if (createdIfBlock) {
		GlesCreateStmntEndif(compiler);
	}
	
	compiler->currentScope = compiler->currentScope->parent;
	
	if (innerContinuation) {
		*continuation = innerContinuation;
	}
		
	*continuationMask = accumulatedMask;
	
	return MustBe(compiler, TokenTypeRightBrace);
}

/*
** --------------------------------------------------------------------------
** Declaration parsing functions
** --------------------------------------------------------------------------
*/

/**
 * Parse a precision declaration, i.e.
 * 
 * compound_statement ::= "precision" precision_modifier 
 * 		( "int" | "float" | "sampler2D" | "sampler3D" | "samplerCube" ).
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParsePrecisionDeclaration(Compiler * compiler) {
	
	Precision precision;
	
	MUSTBE(compiler, TokenTypePrecision);
	precision = ParsePrecisionModifier(compiler);

	if (precision == PrecisionUndefined) {
		SyntaxError(compiler);
		return GL_FALSE;
	}

	/* deviation from reference grammer: only for limited set of types a precision can be declared */
		
	switch  (compiler->tokenizer->token.tokenType) {
	case TokenTypeInt:			
		compiler->currentScope->defaultIntPrec = precision; 
		break;
		
	case TokenTypeFloat:
		compiler->currentScope->defaultFltPrec = precision; 
		break;
		
	case TokenTypeSampler2D:
		compiler->currentScope->defaultS2DPrec = precision; 
		break;
		
	case TokenTypeSampler3D:
		compiler->currentScope->defaultS3DPrec = precision; 
		break;
		
	case TokenTypeSamplerCube:
		compiler->currentScope->defaultSCubPrec = precision; 
		break;
			
	default:
		SyntaxError(compiler);
		return GL_FALSE;
	}	
	
	ADVANCE(compiler);

	return MustBe(compiler, TokenTypeSemicolon);
}

/**
 * Parse a parameter declaration, i.e.
 * 
 * parameter ::=
 * 	[ "const" ] ["in" | "out" | "inout" ] 
 * 	type_specifier [ identifier ] [ "[" const_expression "]" ].
 * 
 * @param	compiler	reference to compiler object
 * @param	index		sequential index of the parameter in function header
 * @param	isAbstract	out: flag set to true if parameter declared without identifier
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
static GLboolean ParseParameter(Compiler * compiler, GLsizei index, GLboolean * isAbstract) {
	
	Type * type;
	Expression * size = NULL;
	Qualifier qualifier = QualifierParameterIn;
	GLboolean isConst = GL_FALSE;
	TokenString name;
	char buffer[10];
	
	if (compiler->tokenizer->token.tokenType == TokenTypeConst) {
		isConst = GL_TRUE;
		ADVANCE(compiler);
	} 
	
	switch (compiler->tokenizer->token.tokenType) {
	case TokenTypeIn:
		qualifier = QualifierParameterIn;
		ADVANCE(compiler);
		break;
		
	case TokenTypeOut:
		qualifier = QualifierParameterOut;
		ADVANCE(compiler);
		break;
		
	case TokenTypeInOut:
		qualifier = QualifierParameterInOut;
		ADVANCE(compiler);
		break;
		
	default:
		;
	}
	
	if (qualifier != QualifierParameterIn && isConst) {
		/* const only allowed in conjunction with in parameters */
		SyntaxError(compiler);
		return GL_FALSE;
	}
	
	if (!ParseTypeSpecifier(compiler, &type)) {
		return GL_FALSE;
	}
	
	if (compiler->tokenizer->token.tokenType == TokenTypeIdentifier) {
		name = compiler->tokenizer->token.s;
		ADVANCE(compiler);
	} else {
		/* fake a name for an abstract parameter */
		name.first = buffer;
		name.length = GlesSprintf(buffer, "$%d", index + 1);
		*isAbstract = GL_TRUE;
	}
	
	if (compiler->tokenizer->token.tokenType == TokenTypeLeftBracket) {
		ADVANCE(compiler);
		
		if (!ParseConstantExpression(compiler, &size)) {
			return GL_FALSE;
		}

		GLES_ASSERT(size->base.kind == ExpressionKindConstant);
		
		if (size->base.type->base.kind != TypeInt) {
			/* S0015: Expression must be an integral constant expression */
			
			GlesCompileError(compiler, ErrS0015);
			return GL_FALSE;
		}
		
		if (size->constant.value->intValue[0] < 0) {
			/* S0017: Array size must be greater than zero. */
			
			GlesCompileError(compiler, ErrS0017);
			return GL_FALSE;
		}
		
		type = 
			GlesTypeArrayCreate(compiler->moduleMemory, 
								type, 
								size->constant.value->intValue[0]);
		
		MUSTBE(compiler, TokenTypeRightBracket);
	}
	
	return GlesDeclareParameter(compiler, name.first, name.length, qualifier, 
		type, index, isConst);
}

/**
 * Parse a parameter list, i.e.
 * 
 * parameter_list ::= parameter { "," parameter }.
 * 
 * @param	compiler		reference to compiler object
 * @param	numParameters	out: number of parameters in pasred list
 * @param	isAbstract		out: list contains anonymous parameters
 * 
 * @return	GL_TRUE is parsed successfully, otherwise GL_FALSE.
 */
static GLboolean ParseParameterList(Compiler * compiler, GLsizei * numParameters, GLboolean * isAbstract) {
	GLsizei index = 0;
	
	if (!ParseParameter(compiler, index++, isAbstract)) {
		return GL_FALSE;
	}
	
	while (compiler->tokenizer->token.tokenType == TokenTypeComma) {
		ADVANCE(compiler);
	
		if (!ParseParameter(compiler, index++, isAbstract)) {
			return GL_FALSE;
		}
	}
	
	*numParameters = index;
	return GL_TRUE;
}

/**
 * Parse a function definition or a function prototype, i.e.
 * 
 * function_definition_or_prototype ::=
 * 		"(" [ "void" | parameter_list ] ")" ( ";" | compound_statement ).
 * 
 * @param	compiler	reference to compiler object
 * @param	returnType	reference to return type of function
 * @param	name		name of the function
 * 
 * @return	GL_TRUE if parse is successful, otherwise GL_FALSE
 */
static GLboolean ParseFunctionDefinitionOrPrototype(Compiler * compiler, Type * returnType,
	const TokenString * name) {

	Scope * newScope =
		GlesScopeCreate(compiler->moduleMemory, compiler->currentScope);
	GLsizei numParameters = 0;
	Symbol * symbol;
	GLboolean isAbstract = GL_FALSE;
	BlockList * oldList;
		
	compiler->currentScope = newScope;

	MUSTBE(compiler, TokenTypeLeftParen);
	
	if (compiler->tokenizer->token.tokenType == TokenTypeVoid) {
		ADVANCE(compiler);
	} else if (compiler->tokenizer->token.tokenType != TokenTypeRightParen) {
		if (!ParseParameterList(compiler, &numParameters, &isAbstract)) {
			return GL_FALSE;
		}
	}
	
	MUSTBE(compiler, TokenTypeRightParen);
	compiler->currentScope = compiler->currentScope->parent;

	symbol = 
		GlesDeclareFunction(compiler, name->first, name->length,
							returnType, numParameters, newScope);
							
	if (!symbol) {
		return GL_FALSE;
	}
		
	if (compiler->tokenizer->token.tokenType == TokenTypeSemicolon) {
		ADVANCE(compiler);
#if 0
	} else if (isAbstract) {
		SyntaxError(compiler);
		return GL_FALSE;
#endif
	} else {
		GLuint continuationMask = 0;
		ForLoop * continuation = NULL;
		compiler->currentFunction = &symbol->function;
		
		if (symbol->function.label->target) {
			/* redefinition of function */
			GlesCompileError(compiler, ErrS0023);
			return GL_FALSE;
		}

		/* create a new block for the function and keep it as a reference */
		oldList = compiler->generator.currentList;
		compiler->generator.currentList = &symbol->function.body; 
		symbol->function.label->target = GlesCreateBlock(&compiler->generator);
		
		if (!ParseCompoundStatement(compiler, newScope, &continuation, &continuationMask)) {
			return GL_FALSE;
		}

		if (!(continuationMask & (ContinuationMaskReturn | ContinuationMaskDiscard))) {
			/* last block does not necessarily contain a return statement */
			
			/*GlesCompileWarning(compiler, ErrW0002);*/
			GlesGenInstCond(&compiler->generator, OpcodeRET, CondT, 0, 0, 0, 0);	 
		}
		
		compiler->generator.currentList = oldList;		
		GLES_ASSERT(!continuation);
	}

	return GL_TRUE;
}

/**
 * Parse whatever comes after the identifier in a declaration, i.e.
 * 
 * opt_array_or_initializer ::=
 * 		[ "[" const_expression "]" | "=" assignment_expression ].
 * 
 * @param	compiler	reference to compiler object
 * @param	baseType	type on left hand side of declaration statement
 * @param	type		out: actual type as determined by declarator clause
 * @param	initializer	out: reference to initialization value
 * 
 * @return	GL_TRUE	if parse was successful, otherwise GL_FALSE.
 */
static GLboolean ParseOptArrayOrInitializer(Compiler * compiler, Type * baseType, Type ** type, 
											Expression ** initializer) {
	Expression * size = NULL;
	
	*initializer = NULL;
	*type = baseType;
	
	if (compiler->tokenizer->token.tokenType == TokenTypeLeftBracket) {
		ADVANCE(compiler);
		
		if (compiler->tokenizer->token.tokenType != TokenTypeRightBracket) {
			if (!ParseConstantExpression(compiler, &size)) {
				return GL_FALSE;
			}
			
			GLES_ASSERT(size->base.kind == ExpressionKindConstant);
			
			if (size->base.type->base.kind != TypeInt) {
				/* S0015: Expression must be an integral constant expression */
				
				GlesCompileError(compiler, ErrS0015);
				return GL_FALSE;
			}
			
			if (size->constant.value->intValue[0] < 0) {
				/* S0017: Array size must be greater than zero. */
				
				GlesCompileError(compiler, ErrS0017);
				return GL_FALSE;
			}
			
			*type = 
				GlesTypeArrayCreate(compiler->resultMemory, 
									baseType, 
									size->constant.value->intValue[0]);
		} else {
			*type = 
				GlesTypeArrayCreate(compiler->resultMemory, 
									baseType, 
									~0);
		}
		
		return MustBe(compiler, TokenTypeRightBracket);
	} else if (compiler->tokenizer->token.tokenType == TokenTypeEqual) {
		ADVANCE(compiler);
		return ParseAssignmentExpression(compiler, initializer);
	} else {
		return GL_TRUE;
	}
}

/**
 * Parse the remainder of an init declarator list declaration, i.e.
 * 
 * init_declarator_list_tail ::= { "," identifier opt_array_or_initializer }.
 * 
 * @param	compiler	reference to compiler object
 * @param	baseType	base type on left hand side of declaration statement
 * @param	qualifier	qualifier provided for identifiers to be declared
 * @param	invariant	GL_TRUE for variables to be declared as invariant
 * 
 * @return	GL_TRUE	if parse was successful, otherwise GL_FALSE.
 */
static GLboolean ParseInitDeclaratorListTail(Compiler * compiler, Type * baseType,
											 Qualifier qualifier, GLboolean invariant) {
	while (compiler->tokenizer->token.tokenType == TokenTypeComma) {
		Type * type;
		Expression * initializer = NULL;
		TokenString name;
		
		ADVANCE(compiler);
		name = compiler->tokenizer->token.s;
		MUSTBE(compiler, TokenTypeIdentifier);
		
		if (!ParseOptArrayOrInitializer(compiler, baseType, &type, &initializer) ||
			!GlesDeclareVariable(compiler, 
								 name.first, name.length,
								 qualifier, type, invariant,
								 initializer)) {
			return GL_FALSE;
		}
	}
	
	return GL_TRUE;
}

/**
 * Parse a local declaration, i.e.
 * 
 * local_declaration ::=
 * 		precision_declaration
 * 	  |	fully_specified_type identifier opt_array_or_initializer 
 * 		init_declarator_list_tail ";".
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE	if parse was successful, otherwise GL_FALSE.
 */
static GLboolean ParseLocalDeclaration(Compiler * compiler) {
	Type *baseType = NULL, *type = NULL;
	
	if (compiler->tokenizer->token.tokenType == TokenTypePrecision) {
		return ParsePrecisionDeclaration(compiler);
	} else {
		Qualifier qualifier = QualifierVariable;
		GLboolean invariant = GL_FALSE;
		Expression * initializer = NULL;
		TokenString name;
				
		if (!ParseFullySpecifiedType(compiler, &baseType, &qualifier, &invariant)) {
			return GL_FALSE;
		}
		
		name = compiler->tokenizer->token.s;
		MUSTBE(compiler, TokenTypeIdentifier);
		
		if (!ParseOptArrayOrInitializer(compiler, baseType, &type, &initializer) ||
			!GlesDeclareVariable(compiler, 
								 name.first, name.length,
								 qualifier, type, invariant,
								 initializer)) {
			return GL_FALSE;
		}

		if (!ParseInitDeclaratorListTail(compiler, baseType, qualifier, invariant)) {
			return GL_FALSE;
		}
		
		return MustBe(compiler, TokenTypeSemicolon);
	}
}

/**
 * Parse an external declaration.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE	if parse was successful, otherwise GL_FALSE.
 */
static GLboolean ParseExternalDeclaration(Compiler * compiler) {
	Type *baseType = NULL, *type = NULL;
	
	if (compiler->tokenizer->token.tokenType == TokenTypePrecision) {
		return ParsePrecisionDeclaration(compiler);
	} else if (compiler->tokenizer->token.tokenType == TokenTypeInvariant) {
		// invariant declaration or declaration of invariant varying
		ADVANCE(compiler);
		
		if (compiler->tokenizer->token.tokenType == TokenTypeIdentifier) {
			// invariant declaration of previously defined variables
			
			if (!GlesDeclareInvariant(compiler, 
								 	  compiler->tokenizer->token.s.first,
								 	  compiler->tokenizer->token.s.length)) {
				return GL_FALSE;
			}
								 	  
			ADVANCE(compiler);
			
			while (compiler->tokenizer->token.tokenType == TokenTypeComma) {
				ADVANCE(compiler);

				if (compiler->tokenizer->token.tokenType == TokenTypeIdentifier) {
					if (!GlesDeclareInvariant(compiler, 
										 	  compiler->tokenizer->token.s.first,
										 	  compiler->tokenizer->token.s.length)) {
						return GL_FALSE;
					}
					
					ADVANCE(compiler);
				} else {
					SyntaxError(compiler);
					return GL_FALSE;
				}
			}
			
			return MustBe(compiler, TokenTypeSemicolon);
		} else {
			// declaration of new variables
			Expression * initializer;
			TokenString name;
			
			MUSTBE(compiler, TokenTypeVarying);
			
			if (!ParseTypeSpecifier(compiler, &baseType)) {
				return GL_FALSE;
			}
			
			name = compiler->tokenizer->token.s;
			MUSTBE(compiler, TokenTypeIdentifier);

			if (!ParseOptArrayOrInitializer(compiler, baseType, &type, &initializer) ||
				!GlesDeclareVariable(compiler, 
									 name.first, name.length,
									 QualifierVarying, type, GL_TRUE,
									 initializer)) {
				return GL_FALSE;
			}
						
			if (!ParseInitDeclaratorListTail(compiler, baseType, QualifierVarying, GL_TRUE)) {
				return GL_FALSE;
			}
			
			return MustBe(compiler, TokenTypeSemicolon);
		}
	} else {
		Expression * initializer;
		Qualifier qualifier;
		GLboolean invariant;
		TokenString name;
		
		if (!ParseFullySpecifiedType(compiler, &type, &qualifier, &invariant)) {
			return GL_FALSE;
		}
		
		baseType = type;
		
		name = compiler->tokenizer->token.s;
		
		if (compiler->tokenizer->token.tokenType == TokenTypeSemicolon &&
			baseType->base.kind == TypeStruct) {
				ADVANCE(compiler);
				return GL_TRUE;
		}
		
		MUSTBE(compiler, TokenTypeIdentifier);

		if (compiler->tokenizer->token.tokenType == TokenTypeLeftParen) {
			/* function prototype or function declaration */
			
			if (qualifier != QualifierVariable) {
				/* no qualifier allowed for functions */
				
				SyntaxError(compiler);
				return GL_FALSE;
			}
			
			return ParseFunctionDefinitionOrPrototype(compiler, type, &name);
		} else {
			if (!ParseOptArrayOrInitializer(compiler, baseType, &type, &initializer) ||
				!GlesDeclareVariable(compiler, 
									 name.first, name.length,
									 qualifier, type, invariant,
									 initializer)) {
				return GL_FALSE;
			}
	
			if (!ParseInitDeclaratorListTail(compiler, baseType, qualifier, invariant)) {
				return GL_FALSE;
			}
			
			return MustBe(compiler, TokenTypeSemicolon);
		}
	}
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

/**
 * Parse a translation unit, i.e. 
 * 
 * translation_unit ::= { external_declaration }.
 * 
 * This is the top-level entry point into the parser.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE	if parse was successful, otherwise GL_FALSE.
 */
GLboolean GlesParseTranslationUnit(Compiler * compiler) {
	while (compiler->tokenizer->token.tokenType != TokenTypeEof) {
		if (!ParseExternalDeclaration(compiler)) {
			return GL_FALSE;
		}
	}
	
	return GL_TRUE;
}