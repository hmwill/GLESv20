/*
** ==========================================================================
**
** $Id: expressions.c 69 2007-09-27 06:15:41Z hmwill $			
** 
** Shading Language Front-End: Expression Processing
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-26 23:15:41 -0700 (Wed, 26 Sep 2007) $
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
#include "frontend/expressions.h"
#include "frontend/declarations.h"
#include "frontend/statements.h"
#include "frontend/il.h"
#include "frontend/types.h"
#include "frontend/symbols.h"
#include "frontend/memory.h"
#include "frontend/tokenizer.h"

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static GLboolean IsLValue(Compiler * compiler, Expression * expr);

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

/**
 * Clone an expression node.
 * 
 * @param compiler	reference to compiler object
 * @param expr		expression node to clone
 * 
 * @return	a copy of the node passed in
 */
Expression * GlesCloneExpression(Compiler * compiler, Expression * expr) {
	Expression * clone;
	GLsizeiptr size = 0;
	
	switch (expr->base.kind) {
	case ExpressionKindConstant:			
		size = sizeof(ExpressionConstant);			
		break;
		
	case ExpressionKindVectorConstructor:	
		size = sizeof(ExpressionVectorConstructor);		
		break;
		
	case ExpressionKindStructConstructor:	
		size = sizeof(ExpressionStructConstructor);		
		break;
		
	case ExpressionKindFixedComponent:		
		size = sizeof(ExpressionFixedComponent);	
		break;
		
	case ExpressionKindRandomComponent:		
		size = sizeof(ExpressionRandomComponent);	
		break;
		
	case ExpressionKindReference:			
		size = sizeof(ExpressionReference);			
		break;
	}

	clone = GlesMemoryPoolAllocate(compiler->exprMemory, size);	
	GlesMemcpy(clone, expr, size);
	
	return clone;
}

/**
 * Create a void expression node.
 * 
 * @param compiler	reference to the compiler object
 * 
 * @return			newly created node or NULL in case of error
 */
Expression * GlesCreateExprVoid(Compiler * compiler) {
	Type * type = GlesBasicType(TypeVoid, PrecisionUndefined);
	GLboolean values[4] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE };
	Constant * constant = GlesCreateBoolConstant(compiler->exprMemory, values);
	
	if (!constant) {
		return NULL;
	}
	
	return GlesCreateExprConstant(compiler, type, constant);
}

/**
 * Create a constant expression node.
 * 
 * @param compiler	reference to the compiler object
 * @param type 		type of the constant to be created
 * @param values	reference to an array of constant data records
 * 
 * @return			newly created node or NULL in case of error
 */
Expression * GlesCreateExprConstant(Compiler * compiler, Type * type, Constant * values) {
	Expression * result = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(ExpressionConstant));
	
	result->base.kind = ExpressionKindConstant;
	result->base.type = type;
	result->constant.value = values;
	
	return result;
}

/**
 * Create a boolean constant expression node.
 * 
 * @param compiler	reference to the compiler object
 * @param value		boolean value to be turned into constant expression
 * 
 * @return			newly created node or NULL in case of error
 */
Expression * GlesCreateExprBoolConstant(Compiler * compiler, GLboolean value) {
	Type * type = GlesBasicType(TypeBool, PrecisionUndefined);
	GLboolean values[4] = { value, GL_FALSE, GL_FALSE, GL_FALSE };
	Constant * constant = GlesCreateBoolConstant(compiler->exprMemory, values);
	
	return GlesCreateExprConstant(compiler, type, constant);
}

/**
 * Create a integer constant expression node.
 * 
 * @param compiler	reference to the compiler object
 * @param value		integer value to be turned into constant expression
 * 
 * @return			newly created node or NULL in case of error
 */
Expression * GlesCreateExprIntConstant(Compiler * compiler, GLint value) {
	Type * type = GlesBasicType(TypeInt, PrecisionUndefined);
	GLint values[4] = { value, 0, 0, 0 };
	Constant * constant = GlesCreateIntConstant(compiler->exprMemory, values);
	
	return GlesCreateExprConstant(compiler, type, constant);
}

/**
 * Create a floating point constant expression node.
 * 
 * @param compiler	reference to the compiler object
 * @param value		float value to be turned into constant expression
 * 
 * @return			newly created node or NULL in case of error
 */
Expression * GlesCreateExprFloatConstant(Compiler * compiler, GLfloat value) {
	Type * type = GlesBasicType(TypeFloat, PrecisionUndefined);
	GLfloat values[4] = { value, 0.0f, 0.0f, 0.0f };
	Constant * constant = GlesCreateFloatConstant(compiler->exprMemory, values);
	
	return GlesCreateExprConstant(compiler, type, constant);
}

/**
 * Extract a component of an array, a matrix, a vector or a scalar. Will return NULL if
 * index is out of range.
 * 
 * For arrays: expr must be reference or constant; for references we create an updated
 * 	reference node, for constants we extract the component values into a new constant node.
 * 
 * For structures: operation will fail.
 * 
 * For matrices: expr must be reference or constant or constructor; for references we create reference
 * 	to column vector, for constants we extract column vector constant
 * 
 * For scalar: index has to be 0, if so, this will return expr
 * 
 * @param compiler		reference to compiler object
 * @param expr			expression to which indexing should be applied
 * @param index			constant index value to be applied
 * 
 * @return				newly created node or NULL in case of error
 */
Expression * GlesCreateExprComponent(Compiler * compiler, Expression * expr, GLsizei index) {
	
	switch (expr->base.type->base.kind) {
	case TypeArray:
		{
			Type * type = expr->base.type->array.elementType;
			GLsizei numElements = expr->base.type->array.elements;
			
			if (index > numElements) {
				GlesCompileError(compiler, ErrS0020);
				return NULL;
			} 

			switch (expr->base.kind) {
			case ExpressionKindConstant:
				/* create constant value for array element */
				return GlesCreateExprConstant(compiler, type, expr->constant.value + index * type->base.size);
				
			case ExpressionKindReference:
				/* create reference element type reference */
				{
					GLsizei factor = 1;
					Expression * indexExpr = (Expression *) expr->reference.index;	
				
					if (GlesTypeIsPrimitive(type)) {
						factor *= type->base.size;
					}
						
					if (indexExpr && factor * numElements != 1) {
						Expression * factorExpr = 
							GlesCreateExprIntConstant(compiler, factor * numElements);
	
						Expression * product =
							GlesCreateExprMul(compiler, indexExpr, factorExpr);
												
						indexExpr = GlesCreateExprComponent(compiler, product, 0);
					}
					
					return GlesCreateExprReference(compiler, type, expr->reference.ref, &indexExpr->fixedComponent,
												   (expr->reference.offset * numElements + index) * factor);
				}
															   
			default:
				GLES_ASSERT(GL_FALSE);
			}		
		}
		
		break;
		
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		{
			Type * type = GlesElementType(expr->base.type);
			GLsizei numElements = expr->base.type->base.size;
			
			if (index > numElements) {
				GlesCompileError(compiler, ErrS0020);
				return NULL;
			} 
			
			switch (expr->base.kind) {
			case ExpressionKindConstant:
				return GlesCreateExprConstant(compiler, type, expr->constant.value + index);
				
			case ExpressionKindReference:
				return GlesCreateExprReference(compiler, type, expr->reference.ref, expr->reference.index,
											   expr->reference.offset + index);

			case ExpressionKindStructConstructor:											   
				return (Expression *) expr->structConstructor.slots[index].base;

			default:
				GLES_ASSERT(GL_FALSE);
			}
		}
		
		break;
				
	case TypeBool:
	case TypeInt:
	case TypeFloat:
		{
			/* return self if index == 0, otherwise internal error */
			
			if (index > 1) {
				GlesCompileError(compiler, ErrS0020);
				return NULL;
			} 

			if (expr->base.kind == ExpressionKindVectorConstructor) {
				expr = (Expression *) expr->vectorConstructor.slots[0].base;
			}
			
			switch (expr->base.kind) {
			case ExpressionKindConstant:
			case ExpressionKindFixedComponent:
				return expr;
				
			case ExpressionKindRandomComponent:
				expr = GlesEvaluate(compiler, expr);
				GLES_ASSERT(expr->base.kind == ExpressionKindReference);
				/* fall through */
				
			case ExpressionKindReference:
				{
					Expression * result = 
						GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(ExpressionFixedComponent));
						
					result->base.kind = ExpressionKindFixedComponent;
					result->base.type = expr->base.type;
					result->fixedComponent.expr = &expr->reference;
					result->fixedComponent.index = index;
					
					return result;
				}

			default:
				GLES_ASSERT(GL_FALSE);
			}
			
			return NULL;
		}
		
		break;
			
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
		{
			Type * type = GlesElementType(expr->base.type);
			GLsizei numElements = expr->base.type->base.elements;
			
			if (index > numElements) {
				GlesCompileError(compiler, ErrS0020);
				return NULL;
			} 
			
			switch (expr->base.kind) {
			case ExpressionKindConstant:			
				return GlesCreateExprBoolConstant(compiler, expr->constant.value->boolValue[index]);
				
			case  ExpressionKindReference:
				{
					Expression * result = 
						GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(ExpressionFixedComponent));
						
					result->base.kind = ExpressionKindFixedComponent;
					result->base.type = type;
					result->fixedComponent.expr = &expr->reference;
					result->fixedComponent.index = index;
					
					return result;
				}
								
			case ExpressionKindVectorConstructor:
				GLES_ASSERT(expr->vectorConstructor.slots[index].base->kind == ExpressionKindConstant ||
							expr->vectorConstructor.slots[index].base->kind == ExpressionKindFixedComponent);
							
				return (Expression *) expr->vectorConstructor.slots[index].base;
				
			default:
				GLES_ASSERT(GL_FALSE);
			}
		}
		
		break;
	
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		{
			Type * type = GlesElementType(expr->base.type);
			GLsizei numElements = expr->base.type->base.elements;
			
			if (index > numElements) {
				GlesCompileError(compiler, ErrS0020);
				return NULL;
			} 
			
			switch (expr->base.kind) {
			case ExpressionKindConstant:			
				return GlesCreateExprIntConstant(compiler, expr->constant.value->intValue[index]);
				
			case  ExpressionKindReference:
				{
					Expression * result = 
						GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(ExpressionFixedComponent));
						
					result->base.kind = ExpressionKindFixedComponent;
					result->base.type = type;
					result->fixedComponent.expr = &expr->reference;
					result->fixedComponent.index = index;
					
					return result;
				}
								
			case ExpressionKindVectorConstructor:
				GLES_ASSERT(expr->vectorConstructor.slots[index].base->kind == ExpressionKindConstant ||
							expr->vectorConstructor.slots[index].base->kind == ExpressionKindFixedComponent);
							
				return (Expression *) expr->vectorConstructor.slots[index].base;
				
			default:
				GLES_ASSERT(GL_FALSE);
			}
		}
		
		break;
		
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		{
			Type * type = GlesElementType(expr->base.type);
			GLsizei numElements = expr->base.type->base.elements;
			
			if (index > numElements) {
				GlesCompileError(compiler, ErrS0020);
				return NULL;
			} 
			
			switch (expr->base.kind) {
			case ExpressionKindConstant:			
				return GlesCreateExprFloatConstant(compiler, expr->constant.value->floatValue[index]);
				
			case  ExpressionKindReference:
				{
					Expression * result = 
						GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(ExpressionFixedComponent));
						
					result->base.kind = ExpressionKindFixedComponent;
					result->base.type = type;
					result->fixedComponent.expr = &expr->reference;
					result->fixedComponent.index = index;
					
					return result;
				}
								
			case ExpressionKindVectorConstructor:
				GLES_ASSERT(expr->vectorConstructor.slots[index].base->kind == ExpressionKindConstant ||
							expr->vectorConstructor.slots[index].base->kind == ExpressionKindFixedComponent);
							
				return (Expression *) expr->vectorConstructor.slots[index].base;
				
			default:
				GLES_ASSERT(GL_FALSE);
			}
		}
		
		break;
	
	default:
		;
	}
	
	/* Internal compiler error */
	
	GlesCompileError(compiler, ErrI0000);
	GLES_ASSERT(GL_FALSE);
	return NULL;
}

/**
 * Converts an expression to a different type. expr has to be a scalar or vector of matching
 * dimensionality.
 */
Expression * GlesCreateExprTypeCast(Compiler * compiler, Type * type, Expression * expr) {

	Expression * result;
	
	GLES_ASSERT(type->base.elements == expr->base.type->base.elements);
	
	if (type->base.kind == expr->base.type->base.kind) {
		return expr;
	}

	if (expr->base.kind == ExpressionKindConstant) {
		Constant * convertedValue;
		
		convertedValue = GlesConvertConstant(compiler->exprMemory, expr->constant.value,
			GlesElementType(expr->base.type)->base.kind, GlesElementType(type)->base.kind);
			
		return GlesCreateExprConstant(compiler, type, convertedValue);
	} else {
		switch (expr->base.type->base.kind) {
		case TypeBool:
		case TypeInt:
		case TypeFloat:
		case TypeBoolVec2:
		case TypeBoolVec3:
		case TypeBoolVec4:
		case TypeIntVec2:
		case TypeIntVec3:
		case TypeIntVec4:
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
			result = GlesCloneExpression(compiler, expr);
			
			if (result) {
				result->base.type = type;
			}
			
			return result;
			
		default:
			;
		}
	}
	
	GlesCompileError(compiler, ErrI0000);
	GLES_ASSERT(GL_FALSE);
	return NULL;
}

/**
 * Create an l-value node. This is applicable to any variable type. If index is present, it needs
 * to be an integer scalar type; either a reference or a register symbol.
 */
Expression * GlesCreateExprReference(Compiler * compiler, Type * type, union Symbol * ref, ExpressionFixedComponent * index, GLsizeiptr offset) {

	Expression * result = NULL;
	
	switch (ref->base.qualifier) {
	case QualifierVariable:						/* general variable 			*/
	case QualifierParameterIn:					/* input parameter		 		*/
	case QualifierParameterOut:					/* output parameter		 		*/
	case QualifierParameterInOut:				/* input/output parameter		*/
	case QualifierConstant:						/* constant value				*/
	case QualifierAttrib:						/* vertex attrib				*/
	case QualifierUniform:						/* uniform value				*/
	case QualifierVarying:						/* varying value				*/
	
	case QualifierPosition:						/* gl_Position variable			*/
	case QualifierPointSize:					/* gl_PointSize					*/
	case QualifierFragCoord:					/* gl_FragCoord					*/
	case QualifierFrontFacing:					/* gl_FrontFacing				*/
	case QualifierFragColor:					/* gl_FragColor					*/
	case QualifierFragData:						/* gl_FragData					*/
	case QualifierPointCoord:					/* gl_PointCoord				*/
		break;
		
	case QualifierTypeName:						/* type name					*/
	case QualifierFunction:						/* user-defined function 		*/
		/* Syntax error (???) */
		GlesCompileError(compiler, ErrL0001);
		return NULL;
		
	case QualifierField:						/* struct member	 			*/
	default:
		/* internal compiler error */
		GLES_ASSERT(GL_FALSE);
		GlesCompileError(compiler, ErrI0000);
		return NULL;
	}
	
	/* TODO: handle indexed case properly */
	GLES_ASSERT(!index || index->base.type->base.kind == TypeInt);
	
	result = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(ExpressionReference));
	
	result->base.kind = ExpressionKindReference;
	result->base.type = type;
	result->reference.ref = ref;
	result->reference.offset = offset;
	result->reference.index = index;
	
	return result;
}

static GLsizei ExpandComponents(Compiler * compiler, Type * type, Expression * slot, GLsizei maxIndex, Expression ** result, GLsizei index) {

	GLsizei size = slot->base.type->base.size;
	GLsizei elements = slot->base.type->base.elements;
	GLsizei component;
	
	if (maxIndex < index + elements * size) {
		/* too many arguments for constructor */
		GlesCompileError(compiler, ErrS0007);
		return ~0;
	}
	
	switch (slot->base.type->base.kind) {
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		/* recursively reduce matrix case to vector case */
		
		for (component = 0; component < size; ++component) {
			index = ExpandComponents(compiler, type, GlesCreateExprComponent(compiler, slot, component), maxIndex, result, index);
			
			if (index == ~0) {
				return index;
			}
		}
		
		return index;
		
	case TypeFloat:		
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
	case TypeInt:		
	case TypeIntVec2:	
	case TypeIntVec3:	
	case TypeIntVec4:	
	case TypeBool:
	case TypeBoolVec2:	
	case TypeBoolVec3:	
	case TypeBoolVec4:	
		break;		 
	
	default:
		GlesCompileError(compiler, ErrS0007);
		return ~0;
	}

	if (GlesElementType(type) != GlesElementType(slot->base.type)) {
		/* need to cast to correct type */
		Type * dstType = GlesVectorType(GlesElementType(type)->base.kind, type->base.prec,elements);
		slot = GlesCreateExprTypeCast(compiler, dstType, slot);
	}
	
	for (component = 0; component < elements; ++component) {
		Expression * newExpression = GlesCreateExprComponent(compiler, slot, component);
		GLES_ASSERT(newExpression);
		result[index++] = newExpression;
	}
	
	return index;
}

static Expression * CreateExprVectorConstructor1(Compiler * compiler, Type * type, Expression * value) {
	Expression * expr = 
		GlesMemoryPoolAllocate(compiler->exprMemory, 
							   sizeof(ExpressionVectorConstructor) + type->base.elements * sizeof(ExpressionVectorElement));
	GLsizei index;
	
	expr->base.kind = ExpressionKindVectorConstructor;
	expr->base.type = type;

	for (index = 0; index < type->base.elements; ++index) {
		switch (value->base.kind) {
		case ExpressionKindConstant:
			expr->vectorConstructor.slots[index].constant = &value->constant;
			break;
			
		case ExpressionKindFixedComponent:
			expr->vectorConstructor.slots[index].fixedComponent = &value->fixedComponent;
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
	}				
	
	return expr;
}

static Expression * CreateExprVectorConstructor(Compiler * compiler, Type * type, Expression ** components) {
	Expression * expr = 
		GlesMemoryPoolAllocate(compiler->exprMemory, 
							   sizeof(ExpressionVectorConstructor) + type->base.elements * sizeof(ExpressionVectorElement));
	GLsizei index;
	
	expr->base.kind = ExpressionKindVectorConstructor;
	expr->base.type = type;

	for (index = 0; index < type->base.elements; ++index) {
		Expression * value = components[index];
		
		switch (value->base.kind) {
		case ExpressionKindConstant:
			expr->vectorConstructor.slots[index].constant = &value->constant;
			break;
			
		case ExpressionKindFixedComponent:
			expr->vectorConstructor.slots[index].fixedComponent = &value->fixedComponent;
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
	}				
	
	return expr;
}

GLES_INLINE static GLboolean IsVectorType(Type * type) {
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
		return GL_TRUE;
	default:
		return GL_FALSE;
	}
}

static Expression * GlesCreateExprScalarVectorConstructor(Compiler * compiler, Type * type, GLsizei numSlots, Expression ** slots) {
	
	GLsizei numComponents = type->base.elements * type->base.size;
	GLsizei index, numArgs = 0;
	Precision minPrecision = PrecisionUndefined;
	
	Expression ** components = 
		GlesMemoryPoolAllocate(compiler->exprMemory, 
							   sizeof(Expression *) * numComponents);
	
	/* 1. Expand expressions passed in into component expressions */
	
	if (numSlots == 1 && IsVectorType(type) && 
		type->base.elements < slots[0]->base.type->base.elements) {
			Expression * tempComponents[4];
			ExpandComponents(compiler, type, slots[0], 4, tempComponents, numArgs);
			
			for (index = 0; index < numComponents; ++index) {
				components[index] = tempComponents[index];
			}
			
			numArgs = numComponents;
		/* special case: for a single argument, we can have truncation */
	} else {
		for (index = 0; index < numSlots; ++index) {
			numArgs = ExpandComponents(compiler, type, slots[index], numComponents, components, numArgs);
		
			if (numArgs == ~0) {
				/* too many arguments */
				GlesCompileError(compiler, ErrS0008);
				return NULL;
			}
		}
	}
	
	if (numArgs != 1 && numArgs < numComponents) {
		/* too few arguments */
		GlesCompileError(compiler, ErrS0009);
		return NULL;
	}
	
	for (index = 0; index < numArgs; ++index) {
		if (minPrecision < components[index]->base.type->base.prec) {
			minPrecision = components[index]->base.type->base.prec;
		}
	}
	
	/* adjust precision of result type */
	type = GlesBasicType(type->base.kind, minPrecision);
	
	/* 2. Collect component expressions into new constructor; create matrix from vectors */
	
	switch (type->base.kind) {
	case TypeBool:
	case TypeInt:
	case TypeFloat:
		/* scalar case */
		return components[0];
		
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
		if (numArgs == 1) {
			if (components[0]->base.kind == ExpressionKindConstant) {
				/* constant vector */
				Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
				GLsizei index;
				
				for (index = 0; index < type->base.elements; ++index) {
					constant->boolValue[index] = components[0]->constant.value->boolValue[0];
				}				
				
				return GlesCreateExprConstant(compiler, type, constant);
			} else {
				/* general vector */
				return CreateExprVectorConstructor1(compiler, type, components[0]);
			}
		} else {
			GLboolean isConstant = (components[0]->base.kind == ExpressionKindConstant);
			
			for (index = 1; index < numComponents; ++index) {
				isConstant &= (components[index]->base.kind == ExpressionKindConstant);
			}
			
			if (isConstant) {
				/* constant vector */
				Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
				GLsizei index;
				
				for (index = 0; index < type->base.elements; ++index) {
					constant->boolValue[index] = components[index]->constant.value->boolValue[0];
				}				
				
				return GlesCreateExprConstant(compiler, type, constant);
			} else {
				/* general vector */
				return CreateExprVectorConstructor(compiler, type, components);
			}
		}
		
		break;

	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		if (numArgs == 1) {
			if (components[0]->base.kind == ExpressionKindConstant) {
				/* constant vector */
				Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
				GLsizei index;
				
				for (index = 0; index < type->base.elements; ++index) {
					constant->intValue[index] = components[0]->constant.value->intValue[0];
				}				
				
				return GlesCreateExprConstant(compiler, type, constant);
			} else {
				/* general vector */
				return CreateExprVectorConstructor1(compiler, type, components[0]);
			}
		} else {
			GLboolean isConstant = (components[0]->base.kind == ExpressionKindConstant);
			
			for (index = 1; index < numComponents; ++index) {
				isConstant &= (components[index]->base.kind == ExpressionKindConstant);
			}
			
			if (isConstant) {
				/* constant vector */
				Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
				GLsizei index;
				
				for (index = 0; index < type->base.elements; ++index) {
					constant->intValue[index] = components[index]->constant.value->intValue[0];
				}				
				
				return GlesCreateExprConstant(compiler, type, constant);
			} else {
				/* general vector */
				return CreateExprVectorConstructor(compiler, type, components);
			}
		}
		
		break;

	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		if (numArgs == 1) {
			if (components[0]->base.kind == ExpressionKindConstant) {
				/* constant vector */
				Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
				GLsizei index;
				
				for (index = 0; index < type->base.elements; ++index) {
					constant->floatValue[index] = components[0]->constant.value->floatValue[0];
				}				
				
				return GlesCreateExprConstant(compiler, type, constant);
			} else {
				/* general vector */
				return CreateExprVectorConstructor1(compiler, type, components[0]);
			}
		} else {
			GLboolean isConstant = (components[0]->base.kind == ExpressionKindConstant);
			
			for (index = 1; index < numComponents; ++index) {
				isConstant &= (components[index]->base.kind == ExpressionKindConstant);
			}
			
			if (isConstant) {
				/* constant vector */
				Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
				GLsizei index;
				
				for (index = 0; index < type->base.elements; ++index) {
					constant->floatValue[index] = components[index]->constant.value->floatValue[0];
				}				
				
				return GlesCreateExprConstant(compiler, type, constant);
			} else {
				/* general vector */
				return CreateExprVectorConstructor(compiler, type, components);
			}
		}
		
		break;

	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		/* matrix case */
		
		if (numArgs == 1) {
			if (components[0]->base.kind == ExpressionKindConstant) {
				/* constant matrix */
				Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant) * type->base.size);
				GLsizei index, outerIndex;
				
				for (outerIndex = 0; outerIndex < type->base.elements; ++outerIndex) {
					for (index = 0; index < type->base.elements; ++index) {
						constant[outerIndex].floatValue[index] = components[0]->constant.value->floatValue[0];
					}				
				}
								
				return GlesCreateExprConstant(compiler, type, constant);
			} else {
				/* general matrix */
				Expression * expr = 
					GlesMemoryPoolAllocate(compiler->exprMemory, 
										   sizeof(ExpressionStructConstructor) + type->base.size * sizeof(ExpressionStructElement));
										   
				Expression * vector = 
					CreateExprVectorConstructor1(compiler, GlesElementType(type), components[0]);
					
				GLsizei index;
				
				GLES_ASSERT(vector->base.kind == ExpressionKindVectorConstructor);
				
				expr->base.kind = ExpressionKindStructConstructor;
				expr->base.type = type;
				
				for (index = 0; index < type->base.size; ++index) {
					expr->structConstructor.slots[index].vectorConstructor = &vector->vectorConstructor;
				}
				
				return expr;
			}
		} else {
			GLboolean isConstant = (components[0]->base.kind == ExpressionKindConstant);
			
			for (index = 1; index < numComponents; ++index) {
				isConstant &= (components[index]->base.kind == ExpressionKindConstant);
			}
			
			if (isConstant) {
				/* constant matrix */
				Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
				GLsizei index, outerIndex, elementIndex;
				
				for (outerIndex = 0, elementIndex = 0; outerIndex < type->base.elements; ++outerIndex) {
					for (index = 0; index < type->base.elements; ++index) {
						constant[outerIndex].floatValue[index] = components[elementIndex++]->constant.value->floatValue[0];
					}				
				}
				
				return GlesCreateExprConstant(compiler, type, constant);
			} else {
				/* general matrix */
				Expression * expr = 
					GlesMemoryPoolAllocate(compiler->exprMemory, 
										   sizeof(ExpressionStructConstructor) + type->base.size * sizeof(ExpressionStructElement));
										   
				GLsizei index, slotIndex;
				
				expr->base.kind = ExpressionKindStructConstructor;
				expr->base.type = type;
				
				for (index = 0, slotIndex = 0; index < type->base.size; ++index) {
					Expression * vector = 
						CreateExprVectorConstructor(compiler, GlesElementType(type), components + slotIndex);
					
					if (!vector) {
						return NULL;
					}
					
					GLES_ASSERT(vector->base.kind == ExpressionKindVectorConstructor);
					
					expr->structConstructor.slots[index].vectorConstructor = &vector->vectorConstructor;					
					slotIndex += type->base.elements;
				}
				
				return expr;
			}
		}
		
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
	}
	
	return NULL;
}

static Expression * GlesCreateExprStructConstructor(Compiler * compiler, Type * type, 
													GLsizei numSlots, Expression ** slots) {
														
	GLsizei index;
	GLboolean isConstant = GL_TRUE;
	
	/* check that we have enough arguments and that they are of the right type */
	
	if (numSlots != type->structure.numFields) {
		GlesCompileError(compiler, ErrS0007);
		return NULL;
	}
	
	for (index = 0; index < numSlots; ++index) {
		if (!GlesTypeMatches(type->structure.fields[index].type, 
							 slots[index]->base.type)) {
			GlesCompileError(compiler, ErrS0011);
			return NULL;
		}
		
		isConstant &= (slots[index]->base.kind == ExpressionKindConstant);
	}
	
	if (isConstant) {
		/* if all the values are constants we can create a constant result */
		Constant * resultValues = GlesMemoryPoolAllocate(compiler->exprMemory, type->base.size);
		Constant * dst = resultValues;

		/* copy and concatenate the individual field members */
		
		for (index = 0; index < numSlots; ++index) {
			GLsizei count;
			Constant * src;
			
			GLES_ASSERT(slots[index]->base.kind == ExpressionKindConstant);
			count = slots[index]->base.type->base.size;
			src = slots[index]->constant.value;
			
			while (count--) {
				*dst++ = *src++;
			}
		}
		
		return GlesCreateExprConstant(compiler, type, resultValues);
	} else {
		/* otherwise we need to create a constructor node */
		Expression * result = 
			GlesMemoryPoolAllocate(compiler->exprMemory, 
								   sizeof(ExpressionStructConstructor) + sizeof(ExpressionStructElement) * numSlots);
		
		result->base.kind = ExpressionKindStructConstructor;
		result->base.type = type;
								   
		for (index = 0; index < numSlots; ++index) {
			Expression * slotValue = slots[index];
			
			switch (slotValue->base.kind) {
			case ExpressionKindConstant:
				result->structConstructor.slots[index].constant = &slotValue->constant;
				break;
				
			case ExpressionKindRandomComponent:
				slotValue = GlesEvaluate(compiler, slotValue);
				GLES_ASSERT(slotValue->base.kind = ExpressionKindReference);
				/* fall through */
				
			case ExpressionKindReference:
				result->structConstructor.slots[index].reference = &slotValue->reference;
				break;
				
			case ExpressionKindStructConstructor:
				result->structConstructor.slots[index].structConstructor = &slotValue->structConstructor;
				break;
				
			case ExpressionKindFixedComponent:
				slotValue = CreateExprVectorConstructor1(compiler, slotValue->base.type, slotValue);
				GLES_ASSERT(slotValue->base.kind = ExpressionKindVectorConstructor);
				/* fall through */
			
			case ExpressionKindVectorConstructor:
				result->structConstructor.slots[index].vectorConstructor = &slotValue->vectorConstructor;
				break;
				
			default:
				GLES_ASSERT(GL_FALSE);
			}			
		}
		
		return result;
	}
}

Expression * GlesCreateExprConstructor(Compiler * compiler, Type * type, GLsizei numSlots, Expression ** slots) {
	
	if (numSlots == 1 && GlesTypeMatches(type, slots[0]->base.type)) {
		/* identity */
		return slots[0];
	}
	
	switch (type->base.kind) {
	case TypeStruct:
		return GlesCreateExprStructConstructor(compiler, type, numSlots, slots);
		
	case TypeFloat:
	case TypeInt:
	case TypeBool:
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		return GlesCreateExprScalarVectorConstructor(compiler, type, numSlots, slots);

	default:
		GLES_ASSERT(GL_FALSE);
		return NULL;
	}
	
	return NULL;
}

/*
** --------------------------------------------------------------------------
** Functions - Derived expression building blocks
** --------------------------------------------------------------------------
*/

/**
 * Parse a swizzle pattern for a vector of the given number of elements.
 * 
 * @param	compiler	reference to compiler object
 * @param	pattern		the swizzle pattern to parse
 * @param	elements	number of vector elements from which to pick
 * @param	result		storage for resulting element indices
 * 
 * @return	the number of elements of the resulting vector, or 0 if the
 * 			pattern is invalid.
 */
static GLsizei ParseSwizzlePattern(Compiler * compiler, TokenString * pattern, 
								   GLsizei elements, GLsizei result[]) {
	static const char rgba[] = "rgba";
	static const char xyzw[] = "xyzw";
	static const char stpq[] = "stpq";
	
	GLsizei index;
	const char * fields;
	
	if (pattern->length > 4) {
		/* Invalid field selector */
		GlesCompileError(compiler, ErrS0026);
		return 0;
	}
	
	switch (pattern->first[0]) {
	case 'r': case 'g': case 'b': case 'a':	fields = rgba; break;
	case 'x': case 'y': case 'z': case 'w':	fields = xyzw; break;
	case 's': case 't': case 'p': case 'q':	fields = stpq; break;
	default:
		/* Invalid field selector */
		GlesCompileError(compiler, ErrS0026);
		return 0;
	}
	
	for (index = 0; index < pattern->length; ++index) {
		const char * p = GlesStrchr(fields, pattern->first[index]);		
		
		if (p) {
			GLsizei selector = p - fields;
			
			if (selector < elements) {
				result[index] = selector;
				continue;
			}
		}
		
		/* Invalid field selector */
		GlesCompileError(compiler, ErrS0026);
		return 0;
	}
	
	for (; index < 4; ++index) {
		result[index] = 0;
	}
	
	return pattern->length;
}

/**
 * Create an expression referencing a specific sub-field (struct member)
 * of a base expression with the given type at the given offset.
 * 
 * @param	compiler	Reference to compiler object
 * @param	expr		Left operand of '.'
 * @param	field		field data structure
 * 
 * @return	the component expression, or NULL in case of error
 */
static Expression * CreateExprField(Compiler * compiler, Expression * expr, 
									SymbolField * field) {
	switch (expr->base.kind) {
	case ExpressionKindConstant:
		return GlesCreateExprConstant(compiler, field->base.type, 
									  expr->constant.value + field->offset);
									  
	case ExpressionKindStructConstructor:
		return (Expression *) expr->structConstructor.slots[field->index].base;
		
	case ExpressionKindReference:
		{
			Symbol * fieldVar = 
				GlesSymbolFindExpansion(compiler, expr->reference.ref, field);
				
			Expression * index = (Expression *) expr->reference.index;	
			GLsizeiptr offset = expr->reference.offset;
						
			if (field->base.type->base.kind == TypeArray) {
				/* multiply offset and index by arrays size */
				GLsizei scale = field->base.type->array.elements;
				
				offset *= scale;
				
				if (index) {
					Expression * factor = 
						GlesCreateExprIntConstant(compiler, scale);

					Expression * product =
						GlesCreateExprMul(compiler, index, factor);
											
					index = GlesCreateExprComponent(compiler, product, 0);
				}
			} else if (GlesTypeIsPrimitive(field->base.type) &&
					   field->base.type->base.size != 1) {
			
				/* multiply offset and index by arrays size */
				GLsizei scale = field->base.type->base.size;
				
				offset *= scale;
				
				if (index) {
					Expression * factor = 
						GlesCreateExprIntConstant(compiler, scale);

					Expression * product =
						GlesCreateExprMul(compiler, index, factor);
											
					index = GlesCreateExprComponent(compiler, product, 0);
				}
			}
			
			GLES_ASSERT(!index || 
						index->base.kind == ExpressionKindFixedComponent);
			
			return GlesCreateExprReference(compiler, field->base.type, 
										   fieldVar, &index->fixedComponent, 
										   offset);
		}
											   
	default:
		GlesCompileError(compiler, ErrI0000);
		return NULL;
	}
}

/**
 * Evaluate an expression involving the dot operator.
 * 
 * @param	compiler	Reference to compiler object
 * @param	expr		Left operand of '.'
 * @param	field		Identifier on right hand side of '.'
 * 
 * @return	the component or vector swizzle identified by the combination of expr and
 * 			field, or NULL in case of an error
 */
Expression * GlesCreateExprField(Compiler * compiler, Expression * expr, struct TokenString * field) {
	Type * type = expr->base.type;
	
	/* expression must be vector or struct type */
	
	switch (type->base.kind) {
	case TypeStruct:
		{	
			Symbol * member = 
				GlesSymbolFind(type->structure.symbols, 
							   field->first, field->length, ~0);
							   
			if (!member) {
				GlesCompileError(compiler, ErrS0026);
				return NULL;
			}
					
			return CreateExprField(compiler, expr, &member->field);
		}
		
		break;
				
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
		{
			GLsizei	selector[4];
			GLsizei elements;
			TypeValue typeValue;
			Type *	resultType;
			
			elements = ParseSwizzlePattern(compiler, field, type->base.elements, selector);
			
			if (!elements) {
				return NULL;
			}
			
			typeValue = GlesElementType(type)->base.kind;
			resultType = GlesVectorType(typeValue, type->base.prec, elements);
			
			/* expr must be constant, reference, constructor */
			switch (expr->base.kind) {
			case ExpressionKindConstant:
				{
					Constant * swizzled = 
						GlesSwizzleConstant(compiler->exprMemory, expr->constant.value, 
											typeValue, selector);
											
					return GlesCreateExprConstant(compiler, resultType, swizzled);
				}
				
			case ExpressionKindVectorConstructor:
				{
					Expression *swizzled[4];
					GLsizei 	index;
					
					for (index = 0; index < elements; ++index) {
						swizzled[index] = (Expression *) expr->vectorConstructor.slots[selector[index]].base;
					}
					
					return GlesCreateExprConstructor(compiler, resultType, elements, swizzled);
				}
				
			case ExpressionKindReference:
				{
					Expression *swizzled[4], *components[4];
					GLsizei 	index, index2;
					Expression * result;
					GLboolean	assignable = GL_TRUE;
					
					ExpandComponents(compiler, type, expr, sizeof components, components, 0);

					for (index = 0; index < elements; ++index) {
						swizzled[index] = components[selector[index]];
						
						for (index2 = index + 1; index2 < elements; ++index2) {
							if (selector[index] == selector[index2]) {
								assignable = GL_FALSE;
							}
						}
					}
					
					result = CreateExprVectorConstructor(compiler, resultType, swizzled);
					GLES_ASSERT(result->base.kind == ExpressionKindVectorConstructor);
					result->vectorConstructor.assignable = assignable;
					return result;
				}
			
			default:
				GlesCompileError(compiler, ErrI0000);
			}
		}
			
		break;

	default:
		GlesCompileError(compiler, ErrS0026);
	}
	
	return NULL;
}

/**
 * Evaluate the index operator [].
 * 
 * @param	compiler	Reference to compiler object
 * @param	expr		Left operand of []
 * @param	index		Index operand of []
 * 
 * @return	The component value identified by expr and index.
 */
Expression * GlesCreateExprIndex(Compiler * compiler, Expression * expr, Expression * index) {

	if (index->base.type->base.kind != TypeInt) {
		GlesCompileError(compiler, ErrS0002);
		return NULL;
	}

	/* turn index into vector component */
		
	switch (index->base.kind) {
	case ExpressionKindConstant:
		/* if constant index, reduce to component extraction */
		return GlesCreateExprComponent(compiler, expr, index->constant.value->intValue[0]);
	
	case ExpressionKindVectorConstructor:
		index = (Expression *) index->vectorConstructor.slots[0].base;
		GLES_ASSERT(index->base.kind == ExpressionKindFixedComponent);
		break;
	
	case ExpressionKindRandomComponent:
		index = GlesEvaluate(compiler, index);
		GLES_ASSERT(index->base.kind == ExpressionKindReference);
		/* fall through */
		
	case ExpressionKindReference:
		index = GlesCreateExprComponent(compiler, index, 0);
		break;
	
	case ExpressionKindFixedComponent:
		break;
	
	default:
		GLES_ASSERT(GL_FALSE);
	}
	
	GLES_ASSERT(index->base.kind == ExpressionKindFixedComponent);
	
	/* cannot index arrays of samplers with arbitrary expressions */
		
	switch (expr->base.type->base.kind) {
	case TypeArray:
		GLES_ASSERT(expr->base.kind == ExpressionKindReference);

		{
			Expression * indexExpr = (Expression *) expr->reference.index;
			
			if (indexExpr) { 
				if (expr->base.type->array.elements != 1) {
					/* scale up the index expression by the number of array elements */
				
					Expression * factor = 
						GlesCreateExprIntConstant(compiler, 
												  expr->base.type->array.elements);
												  
					Expression * product = 
						GlesCreateExprReference(compiler, indexExpr->base.type, 
												GlesDeclareTempVariable(compiler, indexExpr->base.type), 
												NULL, 0);
					
					SrcReg factorReg, indexReg;
					DstReg productReg;
					Swizzle swizzle;
					
					GlesGenStore(compiler, &productReg, product, &swizzle);
					GlesGenFetch(compiler, &factorReg, factor, &swizzle);
					GlesGenFetch(compiler, &indexReg, indexExpr, &swizzle);
					
					GlesGenInstBinary(&compiler->generator, OpcodeMUL, index->base.type->base.prec, 
									  &productReg, &indexReg, &factorReg);
									  
					indexExpr = product;
				}
				
				/* add the new index value to the old one */

				{
					Expression * sum = 
						GlesCreateExprReference(compiler, indexExpr->base.type, 
												GlesDeclareTempVariable(compiler, indexExpr->base.type), 
												NULL, 0);
	
					SrcReg oldIndexReg, indexReg;
					DstReg sumReg;
					Swizzle swizzle;
					
					GlesGenStore(compiler, &sumReg, sum, &swizzle);
					GlesGenFetch(compiler, &indexReg, index, &swizzle);
					GlesGenFetch(compiler, &oldIndexReg, indexExpr, &swizzle);
					
					GlesGenInstBinary(&compiler->generator, OpcodeMUL, index->base.type->base.prec, 
									  &sumReg, &oldIndexReg, &indexReg);
									  
					index = sum;
				}
			}
		}
		
		if (GlesTypeIsPrimitive(expr->base.type->array.elementType) &&
			expr->base.type->array.elementType->base.size != 1) {
			/* scale up the overall index value */

			/* multiply index by element size */
			Expression * factor = 
				GlesCreateExprIntConstant(compiler, 
										  expr->base.type->array.elementType->base.size);
										  
			Expression * product = 
				GlesCreateExprReference(compiler, index->base.type, 
										GlesDeclareTempVariable(compiler, index->base.type), 
										NULL, 0);
			
			SrcReg factorReg, indexReg;
			DstReg productReg;
			Swizzle swizzle;
			
			GlesGenStore(compiler, &productReg, product, &swizzle);
			GlesGenFetch(compiler, &factorReg, factor, &swizzle);
			GlesGenFetch(compiler, &indexReg, index, &swizzle);
			
			GlesGenInstBinary(&compiler->generator, OpcodeMUL, index->base.type->base.prec, 
							  &productReg, &indexReg, &factorReg);
							  
			index = GlesCreateExprComponent(compiler, product, 0);
		} else if (index->base.kind != ExpressionKindFixedComponent) {
			index = GlesCreateExprComponent(compiler, index, 0);
		}
				
		/* fall through */
				
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
#if 1	
		/* dynamic indexing of vectors and matrices not implemented yet */
		GlesCompileError(compiler, ErrX0005);
		return NULL;
#else	
		switch (expr->base.kind) {
		case ExpressionKindStructConstructor:
			{
				/* create a temporary variable holding the constructed structure */
				
				Symbol * temp = 
					GlesDeclareTempVariable(compiler, expr->base.type);
					
				Expression * tempExpr =
					GlesCreateExprReference(compiler, expr->base.type, temp, NULL, 0);
					
				expr = GlesCreateExprAssign(compiler, tempExpr, expr); 
			}
			
			/* fall through */
					
		case ExpressionKindReference:
			if (expr->reference.index) {
				/* add the two indicies together */
				Expression * sum = 
					GlesCreateExprReference(compiler, index->base.type, 
											GlesDeclareTempVariable(compiler, index->base.type), 
											NULL, 0);
				
				SrcReg summandReg, indexReg;
				DstReg sumReg;
				Swizzle swizzle;
				
				GlesGenStore(compiler, &sumReg, sum, &swizzle);
				GlesGenFetch(compiler, &summandReg, (Expression *) expr->reference.index, &swizzle);
				GlesGenFetch(compiler, &indexReg, index, &swizzle);
				
				GlesGenInstBinary(&compiler->generator, OpcodeADD, index->base.type->base.prec,
								  &sumReg, &indexReg, &summandReg);
								  
				index = GlesCreateExprComponent(compiler, sum, 0);
			}

			GLES_ASSERT(index->base.kind == ExpressionKindFixedComponent);
					
			return GlesCreateExprReference(compiler, GlesElementType(expr->base.type), 
										   expr->reference.ref, &index->fixedComponent, 
										   expr->reference.offset);
		default:
			GLES_ASSERT(GL_FALSE);
		}
		
		break;
#endif

	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		switch (expr->base.kind) {
		case ExpressionKindVectorConstructor:
			expr = GlesEvaluate(compiler, expr);
			GLES_ASSERT(expr->base.kind == ExpressionKindReference);
			/* fall through */
			
		case ExpressionKindReference:
#if 1
			/* dynamic indexing of vectors not implemented yet */
			GlesCompileError(compiler, ErrX0005);
			return NULL;
#else
			{
				Expression * result = 
					GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(ExpressionRandomComponent));
					
				result->base.kind = ExpressionKindRandomComponent;
				result->base.type = GlesElementType(expr->base.type);
				result->randomComponent.expr = &expr->reference;
				result->randomComponent.index = &index->fixedComponent;
				
				return result;
			}
#endif
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
			
		break;
		
	default:
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		break;
	}
		
	return NULL;
}

/**
 * Evaluate a function call expression.
 * 
 * @param	compiler	Reference to compiler object
 * @param	func		Symbol on left hand side of ()
 * @param	numArgs		Number of argument values provided
 * @param	args		Array of argument value pointers
 * 
 * @return	A node describing the function results or NULL in case of an error.
 * 			For functions without a result, a constant expression of type 
 * 			void is being returned.
 */
Expression * GlesCreateExprCall(Compiler * compiler, union Symbol * func, GLsizei numArgs, Expression ** args) {

	GLsizei index;
	Symbol * symbol;
	Type * type;
	
	if (func->base.qualifier != QualifierFunction) {
		/* S0001: Type mismatch in expression */
		GlesCompileError(compiler, ErrS0001);
		return NULL;
	}
	
	/* find correct overload */
	
	for (; func; func = func->function.overload) {
		type = func->base.type;

		GLES_ASSERT(type->base.kind == TypeFunction);
		
		if (type->func.numParams != numArgs) {
			/* different number of arguments, cannot match */
			continue;
		}
		
		for (index = 0; index < numArgs; ++index) {
			if (!GlesTypeMatches(args[index]->base.type, type->func.parameters[index].type)) {
				break;
			}
		}
		
		if (index == numArgs) {
			break;
		}
	}
	
	if (!func) {
		/* L0002: Undefined identifier */
		GlesCompileError(compiler, ErrL0002);
		return NULL;
	}
	
	/* TO DO: if all arguments are constant and function is built-in, evaluate at compile time */
	
	/* copy all in and inout argument values into parameter variables */
	
	for (index = 0; index < GLES_ELEMENTSOF(func->function.parameterScope->buckets); ++index) {
		for (symbol = func->function.parameterScope->buckets[index]; symbol; symbol = symbol->base.next) {
			if (symbol->base.qualifier == QualifierParameterIn ||
				symbol->base.qualifier == QualifierParameterInOut) {
				Expression * paramExpr =
					GlesCreateExprReference(compiler, symbol->base.type, symbol, NULL, 0);
					 
				if (!GlesCreateExprAssign(compiler, paramExpr, args[symbol->parameter.index])) {
					return NULL;
				}
			}
		}
	}
	
	/* emit function call */
	GlesGenInstBranch(&compiler->generator, OpcodeCAL, func->function.label, CondT, 0, 0, 0, 0);
	
	/* copy all inout and out values back into argument values */

	for (index = 0; index < GLES_ELEMENTSOF(func->function.parameterScope->buckets); ++index) {
		for (symbol = func->function.parameterScope->buckets[index]; symbol; symbol = symbol->base.next) {
			if (symbol->base.qualifier == QualifierParameterOut ||
				symbol->base.qualifier == QualifierParameterInOut) {
				Expression * paramExpr =
					GlesCreateExprReference(compiler, symbol->base.type, symbol, NULL, 0);
					 
				if (!GlesCreateExprAssign(compiler, args[symbol->parameter.index], paramExpr)) {
					return NULL;
				}
			}
		}
	}
	
	/* if function has return type, also copy return value */
	
	if (func->base.type->func.returnType &&
		func->base.type->func.returnType->base.kind != TypeVoid) {
		Expression * returnValue =
			GlesCreateExprReference(compiler, func->base.type->func.returnType, 
									func->function.result, NULL, 0);
			
		Expression * resultExpr =
			GlesCreateExprReference(compiler, func->base.type->func.returnType, 
									GlesDeclareTempVariable(compiler, func->base.type->func.returnType), 
									NULL, 0);
			 
		GlesCreateExprAssign(compiler, resultExpr, returnValue);
		
		return resultExpr;
	} else {
		return GlesCreateExprVoid(compiler);
	}
	
	return NULL;
}

/**
 * Evaluate a post-increment operator.
 * 
 * This function reduces a post-increment operation to an assignment to a temporary
 * variable and a pre-increment on the original value.
 * 
 * @param	compiler	reference to compiler object
 * @param	expr		the operand to be incremented
 * @param	inc			if GL_TRUE perform increment, otherwise decrement
 * 
 * @return 	reference to expression node holding original value, or NULL
 * 			if an error occurred.
 */
Expression * GlesCreateExprPostIncDec(Compiler * compiler, Expression * expr, GLboolean inc) {
	Type * type = expr->base.type;
	Symbol * tempVariable;
	Expression * tempExpr;
	
	/* copy current value into temporary variable */
	tempVariable = GlesDeclareTempVariable(compiler, type);
	tempExpr = GlesCreateExprReference(compiler, type, tempVariable, NULL, 0);

	/* Copy current value into temporary variable, then perform pre-increment */	
	if (!GlesCreateExprAssign(compiler, tempExpr, expr) ||
		!GlesCreateExprPreIncDec(compiler, expr, inc)) {
		return NULL;
	}
		
	return tempExpr;
}

/**
 * Evaluate a pre-increment operator.
 * 
 * This function reduces a pre-increment operation to an assign-add operation.
 * 
 * @param	compiler	reference to compiler object
 * @param	expr		the operand to be incremented
 * @param	inc			if GL_TRUE perform increment, otherwise decrement
 * 
 * @return 	reference to expression node holding the updated value, or NULL
 * 			if an error occurred.
 */
Expression * GlesCreateExprPreIncDec(Compiler * compiler, Expression * expr, GLboolean inc) {
	Type * type = expr->base.type, *rowType = type;
	GLsizei size = type->base.size;
	GLsizei index;
	Constant * incConstant;
	Expression * incValue; 
		
	/* type must be integer or floating point scalar, matrix or vector */
	switch (expr->base.type->base.kind) {
	case TypeInt:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:	
	case TypeFloat:
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		break;
		
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		rowType = GlesElementType(type);
		break;
		
	default:
		/* Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
	
	incConstant = GlesMemoryPoolAllocate(compiler->moduleMemory, sizeof(Constant));
	
	incConstant->floatValue[0] = 
	incConstant->floatValue[1] = 
	incConstant->floatValue[2] = 
	incConstant->floatValue[3] = 1.0f;
	 
	incValue = GlesCreateExprConstant(compiler, rowType, incConstant);
	
	if (size > 1) {
		for (index = 0; index < size; ++index) {
			Expression * row = GlesCreateExprComponent(compiler, expr, index);
			
			if (!(inc ? GlesCreateExprAssignAdd : GlesCreateExprAssignSub)(compiler, row, incValue)) {
				return NULL;
			}
		}
	} else {
		if (!(inc ? GlesCreateExprAssignAdd : GlesCreateExprAssignSub)(compiler, expr, incValue)) {
			return NULL;
		}	
	}
						
	return expr;
}

/**
 * Evalute a negation expression.
 * 
 * @param	compiler	reference to compiler object
 * @param	expr		expression to negate
 * @param	plus		if GL_TRUE, this operation is effectively a NOP.
 * 
 * @return	Expression node representing result of the negation operation, or
 * 			NULL in case of an error.
 */
Expression * GlesCreateExprNegate(Compiler * compiler, Expression * expr, GLboolean plus) {

	Type * type = expr->base.type, *rowType = type;
	GLsizei size = type->base.size;
	Symbol * result;
	Expression * resultExpr;
	
	switch (expr->base.type->base.kind) {
	case TypeInt:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:	
	case TypeFloat:
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		break;
		
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		rowType = GlesElementType(type);
		break;
		
	default:
		/* Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}

	if (plus) {
		return expr;
	}
	
	if (expr->base.kind == ExpressionKindConstant) {
		/* constant case 													*/
		
		GLsizei index, element;
		
		Constant * value = expr->constant.value;
		Constant * resultConst = 
			GlesMemoryPoolAllocate(compiler->exprMemory, 
								   type->base.size * sizeof(Constant));
								   
		switch (GlesElementType(rowType)->base.kind) {
		case TypeInt:
			for (index = 0; index < size; ++index) {
				for (element = 0; element < 4; ++element) {
					resultConst[index].intValue[element] =
						-value[index].intValue[element];
				}
			}
			
			break;
			
		case TypeFloat:
			for (index = 0; index < size; ++index) {
				for (element = 0; element < 4; ++element) {
					resultConst[index].floatValue[element] =
						-value[index].floatValue[element];
				}
			}

			break;
						
		default:
			GLES_ASSERT(GL_FALSE);
		}
		
		return GlesCreateExprConstant(compiler, type, resultConst);
	} else {
		/* non-constant case												*/
		
		result = GlesDeclareTempVariable(compiler, type);
		resultExpr = GlesCreateExprReference(compiler, type, result, NULL, 0);
		
		if (size > 1) {
			GLsizei index;
			
			for (index = 0; index < size; ++index) {
				Expression * row = GlesCreateExprComponent(compiler, expr, index);
				Expression * resultRow = GlesCreateExprComponent(compiler, resultExpr, index);
				SrcReg srcReg;
				DstReg dstReg;
				Swizzle swizzle;
	
				GlesGenStore(compiler, &dstReg, resultRow, &swizzle);
				GlesGenFetch(compiler, &srcReg, row, &swizzle);
				srcReg.negate = !srcReg.negate;
				GlesGenInstUnary(&compiler->generator, OpcodeMOV, rowType->base.prec, &dstReg, &srcReg);
			}
		} else {
			SrcReg srcReg;
			DstReg dstReg;
			Swizzle swizzle;
			
			GlesGenStore(compiler, &dstReg, resultExpr, &swizzle);
			GlesGenFetch(compiler, &srcReg, expr, &swizzle);
			srcReg.negate = !srcReg.negate;
			GlesGenInstUnary(&compiler->generator, OpcodeMOV, rowType->base.prec, &dstReg, &srcReg);
		}
		
		return resultExpr;
	}
}

/**
 * Evaluate a logical negation.
 * 
 * @param	compiler	reference to compiler object
 * @param	expr		expression to negate
 * 
 * @return	expression node representing the negated expression, or NULL if
 * 			an error occurred. Condition expression are mapped to condition
 * 			expressions.
 */ 
Expression * GlesCreateExprNot(Compiler * compiler, Expression * expr) {
	
	Type * type = expr->base.type;
	Expression * result;
	SrcReg srcReg, oneReg;
	DstReg dstReg;
	Swizzle swizzle;
		
	if (type->base.kind != TypeBool) {
		/* Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}

	switch (expr->base.kind) {
	case ExpressionKindConstant:
		return GlesCreateExprBoolConstant(compiler, !expr->constant.value->boolValue[0]);
		
	default:
		result = 
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
									
		GlesGenStore(compiler, &dstReg, result, &swizzle);
		GlesGenFetch(compiler, &srcReg, expr, &swizzle);
		GlesGenFetch(compiler, &oneReg, GlesCreateExprBoolConstant(compiler, GL_TRUE), &swizzle);
		GlesGenInstBinary(&compiler->generator, OpcodeSUB, type->base.prec, &dstReg, &oneReg, &srcReg);
		
		return result;
	}
}

/**
 * Evaluate the multiplication of two scalar values.
 * 
 * Element types and dimensions of the two vectors must match. Precision is promoted
 * to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeInt or TypeFloat
 * @param	right		right hand operand, must be of type TypeInt or TypeFloat
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprMulScalarScalar(Compiler * compiler, Expression * left, Expression * right) {
	Type * type;
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	Swizzle swizzle;
	Expression * result;
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);
	GLES_ASSERT(left->base.type->base.kind == TypeInt ||
				left->base.type->base.kind == TypeFloat);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (type->base.kind == TypeInt) {
			GLint leftValue 	= left->constant.value->intValue[0];
			GLint rightValue	= right->constant.value->intValue[0];
			
			constant->intValue[0] = leftValue * rightValue;
		} else {
			GLfloat leftValue 	= left->constant.value->floatValue[0];
			GLfloat rightValue	= right->constant.value->floatValue[0];
			
			constant->floatValue[0] = leftValue * rightValue;
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenStore(compiler, &resultReg, result, &swizzle);
		GlesGenFetch(compiler, &leftReg, left, &swizzle);
		GlesGenFetch(compiler, &rightReg, right, &swizzle);
		GlesGenInstBinary(&compiler->generator, OpcodeMUL, type->base.prec, 
						  &resultReg, &leftReg, &rightReg);
						  
		return result;
	}
}

/**
 * Evaluate the multiplication of two vector values.
 * 
 * Element types and dimensions of the two vectors must match. Precision is promoted
 * to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeIntVecx or TypeFloatVecx
 * @param	right		right hand operand, must be of type TypeIntVecx or TypeFloatVecx
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprMulVectorVector(Compiler * compiler, Expression * left, Expression * right) {
	
	Type * type;
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	Swizzle swizzle;
	Expression * result;
	GLsizei index;
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);
	GLES_ASSERT(left->base.type->base.kind == TypeIntVec2 	||
				left->base.type->base.kind == TypeIntVec3 	||
				left->base.type->base.kind == TypeIntVec4 	||
				left->base.type->base.kind == TypeFloatVec2	||
				left->base.type->base.kind == TypeFloatVec3	||
				left->base.type->base.kind == TypeFloatVec4);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (type->base.kind == TypeIntVec2 ||
			type->base.kind == TypeIntVec3 ||
			type->base.kind == TypeIntVec4) {
			for (index = 0; index < type->base.elements; ++index) {
				GLint leftValue 	= left->constant.value->intValue[index];
				GLint rightValue	= right->constant.value->intValue[index];
				
				constant->intValue[index] = leftValue * rightValue;
			}
		} else {
			for (index = 0; index < type->base.elements; ++index) {
				GLfloat leftValue 	= left->constant.value->floatValue[index];
				GLfloat rightValue	= right->constant.value->floatValue[index];
				
				constant->floatValue[index] = leftValue * rightValue; 
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenStore(compiler, &resultReg, result, &swizzle);
		GlesGenFetch(compiler, &leftReg, left, &swizzle);
		GlesGenFetch(compiler, &rightReg, right, &swizzle);
		GlesGenInstBinary(&compiler->generator, OpcodeMUL, type->base.prec, 
						  &resultReg, &leftReg, &rightReg);
						  
		return result;
	}
}

/**
 * Evaluate the multiplication of two matrix values value.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeFloatMatx
 * @param	right		right hand operand, must be of type TypeFloatVecx
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprMulMatrixMatrix(Compiler * compiler, Expression * left, Expression * right) {
	Type * type;
	SrcReg leftReg, rightReg, tempReg;
	DstReg resultReg;
	Swizzle swizzle;
	GLsizei index, row, column;

	GLES_ASSERT(left->base.type->base.kind == TypeFloatMat2 ||
				left->base.type->base.kind == TypeFloatMat3 ||
				left->base.type->base.kind == TypeFloatMat4);
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);
	
	type = GlesBasicType(right->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));

	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* constant case */
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant) * type->base.size);

		for (row = 0; row < type->base.elements; ++row) {
			for (column = 0; column < type->base.elements; ++column) {
				GLfloat sum = 0.0f;
				
				for (index = 0; index < type->base.elements; ++index) {
					GLfloat leftValue 	= left->constant.value[index].floatValue[row];
					GLfloat rightValue	= right->constant.value[column].floatValue[index];
					sum += leftValue * rightValue;	
				}
				
				constant[column].floatValue[row] = sum;
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
		
	} else {
		/* general case */

		Symbol * resultVar = GlesDeclareTempVariable(compiler, type);
		Expression * result = GlesCreateExprReference(compiler, type, resultVar, NULL, 0);

		for (column = 0; column < type->base.elements; ++column) {				
			Expression * rightColumn =  GlesCreateExprComponent(compiler, right, column);
			Expression * resultColumn = GlesCreateExprComponent(compiler, result, column);			
			Expression * leftColumn = GlesCreateExprComponent(compiler, left, 0);
				
			GlesGenStore(compiler, &resultReg, resultColumn, &swizzle);
			GlesGenFetch(compiler, &rightReg, rightColumn, &swizzle);
			GlesGenFetch(compiler, &leftReg, leftColumn, &swizzle);
			GlesGenFetch(compiler, &tempReg, resultColumn, NULL);
			
			GlesGenInstBinary(&compiler->generator, OpcodeMUL, type->base.prec, 
							  &resultReg, &leftReg, &rightReg);
			
			for (index = 1; index < type->base.elements; ++index) {				
				leftColumn = GlesCreateExprComponent(compiler, left, index);
				GlesGenFetch(compiler, &leftReg, leftColumn, &swizzle);
				GlesGenInstTernary(&compiler->generator, OpcodeMAD, type->base.prec, 
								   &resultReg, &leftReg, &rightReg, &tempReg);
			}
		}
		
		return result;
	}
	
	return NULL;
}

/**
 * Evaluate the multiplication of a matrix and a vector value.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeFloatMatx
 * @param	right		right hand operand, must be of type TypeFloatVecx
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprMulMatrixVector(Compiler * compiler, Expression * left, Expression * right) {
	Type * type;
	SrcReg leftReg, rightReg, tempReg;
	DstReg resultReg;
	Swizzle swizzle;
	GLsizei index, column;

	GLES_ASSERT(left->base.type->base.kind == TypeFloatMat2 ||
				left->base.type->base.kind == TypeFloatMat3 ||
				left->base.type->base.kind == TypeFloatMat4);
				
	GLES_ASSERT(right->base.type->base.kind == TypeFloatVec2 ||
				right->base.type->base.kind == TypeFloatVec3 ||
				right->base.type->base.kind == TypeFloatVec4);
				
	type = GlesBasicType(right->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));

	if (left->base.type->base.elements != right->base.type->base.elements) {
		/* Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
	
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* constant case */
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));

		for (index = 0; index < type->base.elements; ++index) {
			GLfloat sum = 0;
			
			for (column = 0; column < type->base.elements; ++column) {
				GLfloat leftValue 	= left->constant.value[column].floatValue[index];
				GLfloat rightValue	= right->constant.value->floatValue[column];
				sum += leftValue * rightValue;	
			}
			
			constant->floatValue[index] = sum; 
		}

		return GlesCreateExprConstant(compiler, type, constant);
		
	} else {
		/* general case */
		Symbol * resultVar = GlesDeclareTempVariable(compiler, type);
		Expression * resultExpr = GlesCreateExprReference(compiler, type, resultVar, NULL, 0);
		Expression * matrixColumn = GlesCreateExprComponent(compiler, left, 0);
		
		GlesGenStore(compiler, &resultReg, resultExpr, &swizzle);
		GlesGenFetch(compiler, &rightReg, right, &swizzle);
		GlesGenFetch(compiler, &leftReg, matrixColumn, &swizzle);
		GlesGenFetch(compiler, &tempReg, resultExpr, NULL);

		GlesGenInstBinary(&compiler->generator, OpcodeMUL, type->base.prec, 
						  &resultReg, &leftReg, &rightReg);

		for (index = 1; index < type->base.elements; ++index) {
			matrixColumn = GlesCreateExprComponent(compiler, left, index);
			GlesGenFetch(compiler, &leftReg,  matrixColumn, &swizzle);
			GlesGenInstTernary(&compiler->generator, OpcodeMAD, type->base.prec, 
							   &resultReg, &leftReg, &rightReg, &tempReg);
		}
		
		return resultExpr;
	}
	
	return NULL;
}

/**
 * Evaluate the multiplication of a vector and a matrix value.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeFloatVecx
 * @param	right		right hand operand, must be of type TypeFloatMatx
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprMulVectorMatrix(Compiler * compiler, Expression * left, Expression * right) {
	
	Type * type;
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	GLsizei index, row;

	GLES_ASSERT(left->base.type->base.kind == TypeFloatVec2 ||
				left->base.type->base.kind == TypeFloatVec3 ||
				left->base.type->base.kind == TypeFloatVec4);
				
	GLES_ASSERT(right->base.type->base.kind == TypeFloatMat2 ||
				right->base.type->base.kind == TypeFloatMat3 ||
				right->base.type->base.kind == TypeFloatMat4);
				
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));

	if (left->base.type->base.elements != right->base.type->base.elements) {
		/* Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
	
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* constant case */
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));

		for (index = 0; index < type->base.elements; ++index) {
			GLfloat sum = 0;
			
			for (row = 0; row < type->base.elements; ++row) {
				GLfloat leftValue 	= left->constant.value->floatValue[row];
				GLfloat rightValue	= right->constant.value[index].floatValue[row];
				sum += leftValue * rightValue;	
			}
			
			constant->floatValue[index] = sum; 
		}

		return GlesCreateExprConstant(compiler, type, constant);
		
	} else {
		/* general case */
		Symbol * resultVar = GlesDeclareTempVariable(compiler, type);
		Expression * resultExpr = GlesCreateExprReference(compiler, type, resultVar, NULL, 0);
		Opcode op = type->base.elements == 4 ? OpcodeDP4 :
					type->base.elements == 3 ? OpcodeDP3 : OpcodeDP2;
		
		GlesGenFetch(compiler, &leftReg, left, NULL);
		
		for (index = 0; index < type->base.elements; ++index) {
			Expression * resultComponent = 
				GlesCreateExprComponent(compiler, resultExpr, index);
			Expression * matrixColumn = 
				GlesCreateExprComponent(compiler, right, index);
				
			GlesGenFetch(compiler, &rightReg,  matrixColumn, NULL);
			GlesGenStore(compiler, &resultReg, resultComponent, NULL);
			GlesGenInstBinary(&compiler->generator, op, type->base.prec, 
							  &resultReg, &leftReg, &rightReg);
		}
		
		return resultExpr;
	}
	
	return NULL;
}

/**
 * Evaluate the multiplication of two values.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand
 * @param	right		right hand operand
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
Expression * GlesCreateExprMul(Compiler * compiler, Expression * left, Expression * right) {

	Type * leftType = left->base.type;
	Type * rightType = right->base.type;
	Precision prec = (Precision) GlesMaxi(leftType->base.prec, rightType->base.prec);
	
	switch (leftType->base.kind) {
	case TypeInt:
		switch (rightType->base.kind) {
		case TypeInt:
			return CreateExprMulScalarScalar(compiler, left, right);
			
		case TypeIntVec2:
		case TypeIntVec3:
		case TypeIntVec4:
			left = GlesCreateExprScalarVectorConstructor(compiler, 
														 GlesBasicType(rightType->base.kind, prec), 
														 1, &left);
			return CreateExprMulVectorVector(compiler, 
											 left, 
											 right);
		default:
			;
		}
		
		break;
		
	case TypeFloat:
		switch (rightType->base.kind) {
		case TypeFloat:
			return CreateExprMulScalarScalar(compiler, left, right);
		
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
			left = GlesCreateExprScalarVectorConstructor(compiler, 
														 GlesBasicType(rightType->base.kind, prec), 
														 1, &left);
			return CreateExprMulVectorVector(compiler, 
											 left, 
											 right);
	
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			left = GlesCreateExprScalarVectorConstructor(compiler, 
														 GlesBasicType(rightType->base.kind, prec), 
														 1, &left);
			return CreateExprMulMatrixMatrix(compiler, 
											 left, 
											 right);

		default:
			;
		}
		
		break;
		
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		switch (rightType->base.kind) {
		case TypeFloat:
			right = GlesCreateExprScalarVectorConstructor(compiler, 
														  GlesBasicType(leftType->base.kind, prec), 
														  1, &right);
			return CreateExprMulVectorVector(compiler, 
											 left, 
											 right);
	
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
			return CreateExprMulVectorVector(compiler, left, right);
	
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			return CreateExprMulVectorMatrix(compiler, left, right);
			
		default:
			;
		}
	
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		switch (rightType->base.kind) {
		case TypeInt:
			right = GlesCreateExprScalarVectorConstructor(compiler, 
														  GlesBasicType(leftType->base.kind, prec), 
														  1, &right);
			return CreateExprMulVectorVector(compiler, 
											 left, 
											 right);
		
		case TypeIntVec2:
		case TypeIntVec3:
		case TypeIntVec4:
			return CreateExprMulVectorVector(compiler, left, right);
			
		default:
			;
		}
		
		break;		
	
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		switch (rightType->base.kind) {
		case TypeFloat:
			right = GlesCreateExprScalarVectorConstructor(compiler, 
														  GlesBasicType(leftType->base.kind, prec), 
														  1, &right);
			return CreateExprMulMatrixMatrix(compiler, 
											 left, 
											 right);
		
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
			return CreateExprMulMatrixVector(compiler, left, right);
	
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			return CreateExprMulMatrixMatrix(compiler, left, right);
			
		default:
			;
		}
	
	default:
		break;
	}
		
	/* Operator not supported for operand types */
	GlesCompileError(compiler, ErrS0004);
	return NULL;
}

/**
 * Evaluate the division of two scalar values.
 * 
 * Element types and dimensions of the two vectors must match. Precision is promoted
 * to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeInt or TypeFloat
 * @param	right		right hand operand, must be of type TypeInt or TypeFloat
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprDivScalarScalar(Compiler * compiler, Expression * left, Expression * right) {
	Type * type;
	SrcReg leftReg, rightReg, tempReg;
	DstReg resultReg;
	Swizzle swizzle;
	Expression * result;
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);
	GLES_ASSERT(left->base.type->base.kind == TypeInt ||
				left->base.type->base.kind == TypeFloat);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (type->base.kind == TypeInt) {
			GLint leftValue 	= left->constant.value->intValue[0];
			GLint rightValue	= right->constant.value->intValue[0];
			
			constant->intValue[0] = leftValue / rightValue;
		} else {
			GLfloat leftValue 	= left->constant.value->floatValue[0];
			GLfloat rightValue	= right->constant.value->floatValue[0];
			
			constant->floatValue[0] = leftValue / rightValue;
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenStore(compiler, &resultReg, result, &swizzle);
		GlesGenFetch(compiler, &leftReg, left, &swizzle);
		GlesGenFetch(compiler, &rightReg, right, &swizzle);
		GlesGenFetch(compiler, &tempReg, result, NULL);
		GlesGenInstUnary(&compiler->generator, OpcodeRCP, type->base.prec, 
						 &resultReg, &rightReg);
		GlesGenInstBinary(&compiler->generator, OpcodeMUL, type->base.prec, 
						  &resultReg, &leftReg, &tempReg);
						  
		if (type->base.kind == TypeInt) {
			/* for integer types insert a round down to int */
			GlesGenInstUnary(&compiler->generator, OpcodeFLR, type->base.prec,
							 &resultReg, &tempReg);
		}
		
		return result;
	}
}

/**
 * Evaluate the division of two vector values.
 * 
 * Element types and dimensions of the two vectors must match. Precision is promoted
 * to the greater of the two precisions. Vector division is reduced to element-wise
 * scalar division.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeIntVecx or TypeFloatVecx
 * @param	right		right hand operand, must be of type TypeIntVecx or TypeFloatVecx
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprDivVectorVector(Compiler * compiler, Expression * left, Expression * right) {
	
	Type * type;
	Expression * result;
	GLsizei index;
	
	GLES_ASSERT(GlesElementType(left->base.type)->base.kind ==
				GlesElementType(right->base.type)->base.kind);
				
	if (left->base.type->base.elements != right->base.type->base.elements) {
		/* Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
			
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						 
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (type->base.kind == TypeInt) {
			for (index = 0; index < type->base.elements; ++index) {
				GLint leftValue 	= left->constant.value->intValue[index];
				GLint rightValue	= right->constant.value->intValue[index];
				
				constant->intValue[index] = leftValue / rightValue;
			}
		} else {
			for (index = 0; index < type->base.elements; ++index) {
				GLfloat leftValue 	= left->constant.value->floatValue[index];
				GLfloat rightValue	= right->constant.value->floatValue[index];
				
				constant->floatValue[index] = leftValue / rightValue;
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general case */
		Expression * components[4];

		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
		
		for (index = 0; index < type->base.elements; ++index) {
			Expression * leftComponent =
				GlesCreateExprComponent(compiler, left, index);
			Expression * rightComponent =
				GlesCreateExprComponent(compiler, right, index);
				
			components[index] = CreateExprDivScalarScalar(compiler, leftComponent, rightComponent);
		}
		
		return GlesCreateExprScalarVectorConstructor(compiler, type, type->base.elements, components);
	}
}

/**
 * Evaluate the division of a vector by a scalar value.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeFloatVecx
 * @param	right		right hand operand, must be of type TypeFloat
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprDivVectorScalar(Compiler * compiler, Expression * left, Expression * right) {
	Type * type;
	SrcReg leftReg, rightReg, tempReg;
	DstReg resultReg;
	Expression * result;
	GLsizei index;
	
	GLES_ASSERT(left->base.type->base.kind == TypeFloatVec2 ||
				left->base.type->base.kind == TypeFloatVec3 ||
				left->base.type->base.kind == TypeFloatVec4);
	GLES_ASSERT(right->base.type->base.kind == TypeFloat);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (type->base.kind == TypeInt) {
			for (index = 0; index < type->base.elements; ++index) {
				GLint leftValue 	= left->constant.value->intValue[index];
				GLint rightValue	= right->constant.value->intValue[0];
				
				constant->intValue[index] = leftValue / rightValue;
			}
		} else {
			for (index = 0; index < type->base.elements; ++index) {
				GLfloat leftValue 	= left->constant.value->floatValue[index];
				GLfloat rightValue	= right->constant.value->floatValue[0];
				
				constant->floatValue[index] = leftValue / rightValue;
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenStore(compiler, &resultReg, result, NULL);
		GlesGenFetch(compiler, &leftReg, left, NULL);
		GlesGenFetch(compiler, &rightReg, right, NULL);
		GlesGenFetch(compiler, &tempReg, result, NULL);
		GlesGenInstUnary(&compiler->generator, OpcodeRCP, type->base.prec, 
						 &resultReg, &rightReg);
		GlesGenInstBinary(&compiler->generator, OpcodeMUL, type->base.prec, 
						  &resultReg, &leftReg, &tempReg);

		if (right->base.type->base.kind == TypeInt) {
			/* for integer types insert a round down to int */
			GlesGenInstUnary(&compiler->generator, OpcodeFLR, type->base.prec,
							 &resultReg, &tempReg);
		}
		
						  
		return result;
	}
}

/**
 * Evaluate the division of two matrix values.
 * 
 * Precision is promoted to the greater of the two precisions. Matrix division is 
 * reduced to column-wise vector division.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeFloatMatx
 * @param	right		right hand operand, must be of type TypeFloatMatx
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprDivMatrixMatrix(Compiler * compiler, Expression * left, Expression * right) {
	Type * type;
	Expression * result;
	GLsizei row, column;
	
	GLES_ASSERT(GlesElementType(left->base.type)->base.kind ==
				GlesElementType(right->base.type)->base.kind);
				
	if (left->base.type->base.elements != right->base.type->base.elements) {
		/* Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
			
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						 
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant) * type->base.size);
		
		if (type->base.kind == TypeInt) {
			for (row = 0; row < type->base.elements; ++row) {
				for (column = 0; column < type->base.elements; ++column) {
					GLint leftValue 	= left->constant.value[column].intValue[row];
					GLint rightValue	= right->constant.value[column].intValue[row];
					
					constant[column].intValue[row] = leftValue / rightValue;
				}
			}
		} else {
			for (row = 0; row < type->base.elements; ++row) {
				for (column = 0; column < type->base.elements; ++column) {
					GLfloat leftValue 	= left->constant.value[column].floatValue[row];
					GLfloat rightValue	= right->constant.value[column].floatValue[row];
					
					constant[column].floatValue[row] = leftValue / rightValue;
				}
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general case */
		Expression * components[4];

		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
		
		for (column = 0; column < type->base.elements; ++column) {
			Expression * leftColumn = GlesCreateExprComponent(compiler, left, column);
			Expression * rightColumn = GlesCreateExprComponent(compiler, right, column);
				
			components[column] = CreateExprDivVectorVector(compiler, leftColumn, rightColumn);
		}
		
		return GlesCreateExprScalarVectorConstructor(compiler, type, type->base.elements, components);
	}
}

/**
 * Evaluate the division of a matrix by a scalar value.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeFloatMatx
 * @param	right		right hand operand, must be of type TypeFloat
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprDivMatrixScalar(Compiler * compiler, Expression * left, Expression * right) {
	Type * type;
	SrcReg leftReg, rightReg, tempReg;
	DstReg resultReg;
	Expression * result;
	GLsizei index, column;
	
	GLES_ASSERT(left->base.type->base.kind == TypeFloatMat2 ||
				left->base.type->base.kind == TypeFloatMat3 ||
				left->base.type->base.kind == TypeFloatMat4);
	GLES_ASSERT(right->base.type->base.kind == TypeFloat);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant) * type->base.size);
		
		if (type->base.kind == TypeInt) {
			for (column = 0; column < type->base.size; ++column) {
				for (index = 0; index < type->base.elements; ++index) {
					GLint leftValue 	= left->constant.value[column].intValue[index];
					GLint rightValue	= right->constant.value->intValue[0];
					
					constant[column].intValue[index] = leftValue / rightValue;
				}
			}
		} else {
			for (column = 0; column < type->base.size; ++column) {
				for (index = 0; index < type->base.elements; ++index) {
					GLfloat leftValue 	= left->constant.value[column].floatValue[index];
					GLfloat rightValue	= right->constant.value->floatValue[0];
					
					constant[column].floatValue[index] = leftValue / rightValue;
				}
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenFetch(compiler, &rightReg, right, NULL);

		for (column = 0; column < type->base.elements; ++column) {
			Expression * sourceColumn = GlesCreateExprComponent(compiler, left, column);
			Expression * resultColumn = GlesCreateExprComponent(compiler, result, column);
			
			GlesGenFetch(compiler, &leftReg, sourceColumn, NULL);
			GlesGenFetch(compiler, &tempReg, resultColumn, NULL);
			GlesGenStore(compiler, &resultReg, resultColumn, NULL);
			GlesGenInstUnary(&compiler->generator, OpcodeRCP, type->base.prec, 
							 &resultReg, &rightReg);
			GlesGenInstBinary(&compiler->generator, OpcodeMUL, type->base.prec, 
							  &resultReg, &leftReg, &tempReg);

			if (right->base.type->base.kind == TypeInt) {
				/* for integer types insert a round down to int */
				GlesGenInstUnary(&compiler->generator, OpcodeFLR, type->base.prec,
								 &resultReg, &tempReg);
			}
			
		}
						  
		return result;
	}
}

/**
 * Evaluate the division of two values.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand
 * @param	right		right hand operand
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
Expression * GlesCreateExprDiv(Compiler * compiler, Expression * left, Expression * right) {
	
	Precision prec = (Precision) GlesMaxi(left->base.type->base.prec, right->base.type->base.prec);

	switch (left->base.type->base.kind) {
	case TypeInt:
		switch (right->base.type->base.kind) {
		case TypeInt:
			return CreateExprDivScalarScalar(compiler, left, right);
			
		case TypeIntVec2:
		case TypeIntVec3:
		case TypeIntVec4:
			left = GlesCreateExprScalarVectorConstructor(compiler, 
														 GlesBasicType(right->base.type->base.kind, prec), 
														 1, &left);
			return CreateExprDivVectorVector(compiler, left, right);
			
		default:
			break;
		}
		
		break;
		
	case TypeFloat:
		switch (right->base.type->base.kind) {
		case TypeFloat:
			return CreateExprDivScalarScalar(compiler, left, right);
			
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
			left = GlesCreateExprScalarVectorConstructor(compiler, 
														 GlesBasicType(right->base.type->base.kind, prec), 
														 1, &left);
			return CreateExprDivVectorVector(compiler, left, right);
			
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			left = GlesCreateExprScalarVectorConstructor(compiler, 
														 GlesBasicType(right->base.type->base.kind, prec), 
														 1, &left);
			return CreateExprDivMatrixMatrix(compiler, left, right);
			
		default:
			break;
		}
		
		break;
	
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		switch (right->base.type->base.kind) {
		case TypeInt:
			return CreateExprDivVectorScalar(compiler, left, right);
			
		case TypeIntVec2:
		case TypeIntVec3:
		case TypeIntVec4:
			return CreateExprDivVectorVector(compiler, left, right);
			
		default:
			break;
		}
		
		break;
	
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		switch (right->base.type->base.kind) {
		case TypeFloat:
			return CreateExprDivVectorScalar(compiler, left, right);
			
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
			return CreateExprDivVectorVector(compiler, left, right);
			
		default:
			break;
		}
		
		break;
	
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		switch (right->base.type->base.kind) {
		case TypeFloat:
			return CreateExprDivMatrixScalar(compiler, left, right);
			
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			return CreateExprDivMatrixMatrix(compiler, left, right);
			
		default:
			break;
		}
		
		break;
	
	default:
		break;
	}
	
	GLES_ASSERT(GL_FALSE);
	return NULL;
}

/**
 * Evaluate the addition or subtraction of two scalar values.
 * 
 * Element types and dimensions of the two vectors must match. Precision is promoted
 * to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeInt or TypeFloat
 * @param	right		right hand operand, must be of type TypeInt or TypeFloat
 * @param	negLeft		use -left instead of left
 * @param	negRight	use -right instead of right
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprAddScalarScalar(Compiler * compiler, Expression * left, Expression * right, GLboolean negLeft, GLboolean negRight) {
	
	Type * type;
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	Swizzle swizzle;
	Expression * result;
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);
	GLES_ASSERT(left->base.type->base.kind == TypeInt ||
				left->base.type->base.kind == TypeFloat);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (type->base.kind == TypeInt) {
			GLint leftValue 	= left->constant.value->intValue[0];
			GLint rightValue	= right->constant.value->intValue[0];
			
			constant->intValue[0] = 
				(negLeft  ? -leftValue  : leftValue ) +
				(negRight ? -rightValue : rightValue);
		} else {
			GLfloat leftValue 	= left->constant.value->floatValue[0];
			GLfloat rightValue	= right->constant.value->floatValue[0];
			
			constant->floatValue[0] = 
				(negLeft  ? -leftValue  : leftValue ) +
				(negRight ? -rightValue : rightValue);
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenStore(compiler, &resultReg, result, &swizzle);
		GlesGenFetch(compiler, &leftReg, left, &swizzle);
		leftReg.negate ^= negLeft;
		
		GlesGenFetch(compiler, &rightReg, right, &swizzle);
		rightReg.negate ^= negRight;
		
		GlesGenInstBinary(&compiler->generator, OpcodeADD, type->base.prec, 
						  &resultReg, &leftReg, &rightReg);
						  
		return result;
	}
}

/**
 * Evaluate the addition or subtraction of a vector and a scalar value.
 * 
 * Element types and dimensions of the two vectors must match. Precision is promoted
 * to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeVecIntx or TypeFloatVecx
 * @param	right		right hand operand, must be of type TypeInt or TypeFloat
 * @param	negLeft		use -left instead of left
 * @param	negRight	use -right instead of right
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprAddVectorScalar(Compiler * compiler, Expression * left, Expression * right, GLboolean negLeft, GLboolean negRight) {
	Type * type;
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	Expression * result;
	GLsizei index;
	
	GLES_ASSERT(left->base.type->base.elements > 1 && left->base.type->base.size == 1);
	GLES_ASSERT(GlesElementType(left->base.type)->base.kind == right->base.type->base.kind);
	GLES_ASSERT(right->base.type->base.kind == TypeInt ||
				right->base.type->base.kind == TypeFloat);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (right->base.kind == TypeInt) {
			for (index = 0; index < type->base.elements; ++index) {
				GLint leftValue 	= left->constant.value->intValue[index];
				GLint rightValue	= right->constant.value->intValue[0];
				
				constant->intValue[index] = 
					(negLeft  ? -leftValue  : leftValue ) +
					(negRight ? -rightValue : rightValue);
			}
		} else {
			for (index = 0; index < type->base.elements; ++index) {
				GLfloat leftValue 	= left->constant.value->floatValue[index];
				GLfloat rightValue	= right->constant.value->floatValue[0];
				
				constant->floatValue[index] = 
					(negLeft  ? -leftValue  : leftValue ) +
					(negRight ? -rightValue : rightValue);
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenFetch(compiler, &leftReg, left, NULL);
		leftReg.negate ^= negLeft;
		
		GlesGenFetch(compiler, &rightReg, right, NULL);
		rightReg.negate ^= negRight;
		rightReg.selectY = rightReg.selectZ = rightReg.selectW = rightReg.selectX;
		
		GlesGenStore(compiler, &resultReg, result, NULL);
		GlesGenInstBinary(&compiler->generator, OpcodeADD, type->base.prec, 
						  &resultReg, &leftReg, &rightReg);
						  
		return result;
	}
}

/**
 * Evaluate the addition or subtraction of two vector values.
 * 
 * Element types and dimensions of the two vectors must match. Precision is promoted
 * to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeIntVecx or TypeFloatVecx
 * @param	right		right hand operand, must be of type TypeIntVecx or TypeFloatVecx
 * @param	negLeft		use -left instead of left
 * @param	negRight	use -right instead of right
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprAddVectorVector(Compiler * compiler, Expression * left, Expression * right, GLboolean negLeft, GLboolean negRight) {
	
	Type * type;
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	Expression * result;
	GLsizei index;
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);
	GLES_ASSERT(left->base.type->base.kind == TypeIntVec2 	||
				left->base.type->base.kind == TypeIntVec3 	||
				left->base.type->base.kind == TypeIntVec4 	||
				left->base.type->base.kind == TypeFloatVec2	||
				left->base.type->base.kind == TypeFloatVec3	||
				left->base.type->base.kind == TypeFloatVec4);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (type->base.kind == TypeIntVec2 ||
			type->base.kind == TypeIntVec3 ||
			type->base.kind == TypeIntVec4) {
			for (index = 0; index < type->base.elements; ++index) {
				GLint leftValue 	= left->constant.value->intValue[index];
				GLint rightValue	= right->constant.value->intValue[index];
				
				constant->intValue[index] = 
					(negLeft  ? -leftValue  : leftValue ) +
					(negRight ? -rightValue : rightValue);
			}
		} else {
			for (index = 0; index < type->base.elements; ++index) {
				GLfloat leftValue 	= left->constant.value->floatValue[index];
				GLfloat rightValue	= right->constant.value->floatValue[index];
				
				constant->floatValue[index] = 
					(negLeft  ? -leftValue  : leftValue ) +
					(negRight ? -rightValue : rightValue);
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenFetch(compiler, &leftReg, left, NULL);
		leftReg.negate ^= negLeft;
		
		GlesGenFetch(compiler, &rightReg, right, NULL);
		rightReg.negate ^= negRight;
		
		GlesGenStore(compiler, &resultReg, result, NULL);
		GlesGenInstBinary(&compiler->generator, OpcodeADD, type->base.prec, 
						  &resultReg, &leftReg, &rightReg);
						  
		return result;
	}
}

/**
 * Evaluate the addition or subtraction of a matrix and a scalar value.
 * 
 * Element types and dimensions of the two vectors must match. Precision is promoted
 * to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeFloatMatx
 * @param	right		right hand operand, must be of type TypeFloat
 * @param	negLeft		use -left instead of left
 * @param	negRight	use -right instead of right
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprAddMatrixScalar(Compiler * compiler, Expression * left, Expression * right, GLboolean negLeft, GLboolean negRight) {
	
	Type * type;
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	Expression * result;
	GLsizei row, index;
	
	GLES_ASSERT(left->base.type->base.elements > 1 && left->base.type->base.size > 1);
	GLES_ASSERT(left->base.type->base.kind == TypeFloatMat2 ||
				left->base.type->base.kind == TypeFloatMat3 ||
				left->base.type->base.kind == TypeFloatMat4);
	GLES_ASSERT(right->base.type->base.kind == TypeFloat);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant) * type->base.size);
		
		for (row = 0; row < type->base.size; ++row) {
			for (index = 0; index < type->base.elements; ++index) {
				GLfloat leftValue 	= left->constant.value[row].floatValue[index];
				GLfloat rightValue	= right->constant.value->floatValue[0];
				
				constant[row].floatValue[index] = 
					(negLeft  ? -leftValue  : leftValue ) +
					(negRight ? -rightValue : rightValue);
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		GlesGenFetch(compiler, &rightReg, right, NULL);
		rightReg.negate ^= negRight;
		rightReg.selectY = rightReg.selectZ = rightReg.selectW = rightReg.selectX;

		/* create an ADD instruction for each row of the matrix */
		for (row = 0; row < type->base.size; ++row) {		
			GlesGenFetch(compiler, &leftReg, GlesCreateExprComponent(compiler, left, row), NULL);
			leftReg.negate ^= negLeft;
			
			GlesGenStore(compiler, &resultReg, result, NULL);
			GlesGenInstBinary(&compiler->generator, OpcodeADD, type->base.prec, 
							  &resultReg, &leftReg, &rightReg);
		}
								  
		return result;
	}
}

/**
 * Evaluate the addition or subtraction of two matrix values.
 * 
 * Element types and dimensions of the two matrices must match. Precision is promoted
 * to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand, must be of type TypeFloatMatx
 * @param	right		right hand operand, must be of type TypeFloatMatx
 * @param	negLeft		use -left instead of left
 * @param	negRight	use -right instead of right
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
static Expression * CreateExprAddMatrixMatrix(Compiler * compiler, Expression * left, Expression * right, GLboolean negLeft, GLboolean negRight) {
	
	Type * type;
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	Expression * result;
	GLsizei index, row;
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);
	GLES_ASSERT(left->base.type->base.kind == TypeFloatMat2	||
				left->base.type->base.kind == TypeFloatMat3	||
				left->base.type->base.kind == TypeFloatMat4);
	
	type = GlesBasicType(left->base.type->base.kind, 
						 GlesMaxi(left->base.type->base.prec, right->base.type->base.prec));
						  
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* evaluate constant expression */
		
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant) * type->base.size);

		for (row = 0; row < type->base.size; ++row) {		
			for (index = 0; index < type->base.elements; ++index) {
				GLfloat leftValue 	= left->constant.value[row].floatValue[index];
				GLfloat rightValue	= right->constant.value[row].floatValue[index];
				
				constant[row].floatValue[index] = 
					(negLeft  ? -leftValue  : leftValue ) +
					(negRight ? -rightValue : rightValue);
			}
		}

		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		/* general non-constant case */
		
		result = 	
			GlesCreateExprReference(compiler, type, 
									GlesDeclareTempVariable(compiler, type), 
									NULL, 0);
	
		/* create an ADD instruction for each row of the matrix */
		for (row = 0; row < type->base.size; ++row) {		
			GlesGenFetch(compiler, &leftReg, GlesCreateExprComponent(compiler, left, row), NULL);
			leftReg.negate ^= negLeft;
			GlesGenFetch(compiler, &rightReg, GlesCreateExprComponent(compiler, right, row), NULL);
			rightReg.negate ^= negRight;
			
			GlesGenStore(compiler, &resultReg, result, NULL);
			GlesGenInstBinary(&compiler->generator, OpcodeADD, type->base.prec, 
							  &resultReg, &leftReg, &rightReg);
		}
								  
		return result;
	}
}

/**
 * Evaluate the addition or subtraction of two values.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand
 * @param	right		right hand operand
 * @param	add			calculate left + right if GL_TRUE, left - right otherwise
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
Expression * GlesCreateExprAddSub(Compiler * compiler, Expression * left, Expression * right, GLboolean add) {

	Type * typeLeft = left->base.type;
	Type * typeRight = right->base.type;
	
	switch (typeLeft->base.kind) {
	case TypeInt:
		switch (typeRight->base.kind) {
		case TypeInt:
			return CreateExprAddScalarScalar(compiler, left, right, GL_FALSE, !add);
			
		case TypeIntVec2:
		case TypeIntVec3:
		case TypeIntVec4:
			return CreateExprAddVectorScalar(compiler, right, left, !add, GL_FALSE);
			
		default:
			;
		}
		
		break;
		
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		switch (typeRight->base.kind) {
		case TypeInt:
			return CreateExprAddVectorScalar(compiler, left, right, GL_FALSE, !add);
			
		case TypeIntVec2:
		case TypeIntVec3:
		case TypeIntVec4:
			return CreateExprAddVectorVector(compiler, left, right, GL_FALSE, !add);
			
		default:
			;
		}
		
		break;
		
	case TypeFloat:
		switch (typeRight->base.kind) {
		case TypeFloat:
			return CreateExprAddScalarScalar(compiler, left, right, GL_FALSE, !add);
			
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
			return CreateExprAddVectorScalar(compiler, right, left, !add, GL_FALSE);
			
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			return CreateExprAddMatrixScalar(compiler, right, left, !add, GL_FALSE);
			
		default:
			break;
		}
		
		break;
		
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		switch (typeRight->base.kind) {
		case TypeFloat:
			return CreateExprAddVectorScalar(compiler, left, right, GL_FALSE, !add);
			
		case TypeFloatVec2:
		case TypeFloatVec3:
		case TypeFloatVec4:
			return CreateExprAddVectorVector(compiler, left, right, GL_FALSE, !add);
			
		default:
			break;
		}
		
		break;
			
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		switch (typeRight->base.kind) {
		case TypeFloat:
			return CreateExprAddMatrixScalar(compiler, left, right, GL_FALSE, !add);
			
		case TypeFloatMat2:
		case TypeFloatMat3:
		case TypeFloatMat4:
			return CreateExprAddMatrixMatrix(compiler, left, right, GL_FALSE, !add);
			
		default:
			break;
		}
		
		break;
	
	default:
		;
	}
	
	/* Operator not supported for operand types */
	GlesCompileError(compiler, ErrS0004);
	return NULL;
}

/**
 * Evaluate the addition of two values.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand
 * @param	right		right hand operand
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
Expression * GlesCreateExprAdd(Compiler * compiler, Expression * left, Expression * right) {
	return GlesCreateExprAddSub(compiler, left, right, GL_TRUE);
}

/**
 * Evaluate the difference of two values.
 * 
 * Precision is promoted to the greater of the two precisions.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand operand
 * @param	right		right hand operand
 * 
 * @return	reference to expression node identifying result, or NULL in
 * 			case of an error.
 */
Expression * GlesCreateExprSub(Compiler * compiler, Expression * left, Expression * right) {
	return GlesCreateExprAddSub(compiler, left, right, GL_FALSE);
}

/**
 * Evaluate a comparison operator.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand, must be of type TypeInt or TypeFloat
 * @param	right		right operand, must be of type TypeInt or TypeFloat
 * 
 * @return	Expression node representing result of type TypeBool, or NULL in
 * 			case of an error
 */
static Expression * CreateExprCompare(Compiler * compiler, Expression * left, Expression * right, Opcode op) {

	Type * type = GlesBasicType(TypeBool, PrecisionUndefined);
	Precision prec;
	
	if ((left->base.type->base.kind != TypeInt &&
		 left->base.type->base.kind != TypeFloat) ||
		(right->base.type->base.kind != TypeInt &&
		 right->base.type->base.kind != TypeFloat) ||
		left->base.type->base.kind != right->base.type->base.kind) {
		/* Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}

	prec = (Precision) GlesMaxi(left->base.type->base.prec, right->base.type->base.prec);

	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
			
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (left->base.type->base.kind == TypeInt) {
			GLint leftValue	 = left->constant.value[0].intValue[0];
			GLint rightValue = right->constant.value[0].intValue[0];
			GLboolean result;
			
			switch (op) {
			case OpcodeSLT:	result = (leftValue <  rightValue);	break;
			case OpcodeSLE:	result = (leftValue <= rightValue);	break;
			case OpcodeSGT:	result = (leftValue >  rightValue);	break;
			case OpcodeSGE:	result = (leftValue >= rightValue);	break;
			default:
				GLES_ASSERT(GL_FALSE);
			}
			
			constant[0].boolValue[0] = result;
		} else {
			GLfloat leftValue	= left->constant.value[0].floatValue[0];
			GLfloat rightValue 	= right->constant.value[0].floatValue[0];
			GLboolean result;
			
			GLES_ASSERT(left->base.type->base.kind == TypeFloat);
			
			switch (op) {
			case OpcodeSLT:	result = (leftValue <  rightValue);	break;
			case OpcodeSLE:	result = (leftValue <= rightValue);	break;
			case OpcodeSGT:	result = (leftValue >  rightValue);	break;
			case OpcodeSGE:	result = (leftValue >= rightValue);	break;
			default:
				GLES_ASSERT(GL_FALSE);
			}
			
			constant[0].boolValue[0] = result;
		}
		
		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		SrcReg	regLeft, regRight;
		DstReg	regResult;
		
		Symbol * resultVar = GlesDeclareTempVariable(compiler, type);
		Expression * result = GlesCreateExprReference(compiler, type, resultVar, NULL, 0);

		GlesGenStore(compiler, &regResult, result, NULL);
		GlesGenFetch(compiler, &regLeft, left, NULL);
		GlesGenFetch(compiler, &regRight, right, NULL);
		
		GlesGenInstBinary(&compiler->generator, op, prec,
						  &regResult, &regLeft, &regRight);
						  		
		return result;
	}
}

/**
 * Evaluate < operator.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand, must be of type TypeInt or TypeFloat
 * @param	right		right operand, must be of type TypeInt or TypeFloat
 * 
 * @return	Expression node representing result of type TypeBool, or NULL in
 * 			case of an error
 */
Expression * GlesCreateExprCompareLT(Compiler * compiler, Expression * left, Expression * right) {
	return CreateExprCompare(compiler, left, right, OpcodeSLT);
}

/**
 * Evaluate <= operator.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand, must be of type TypeInt or TypeFloat
 * @param	right		right operand, must be of type TypeInt or TypeFloat
 * 
 * @return	Expression node representing result of type TypeBool, or NULL in
 * 			case of an error
 */
Expression * GlesCreateExprCompareLE(Compiler * compiler, Expression * left, Expression * right) {
	return CreateExprCompare(compiler, left, right, OpcodeSLE);
}

/**
 * Evaluate > operator.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand, must be of type TypeInt or TypeFloat
 * @param	right		right operand, must be of type TypeInt or TypeFloat
 * 
 * @return	Expression node representing result of type TypeBool, or NULL in
 * 			case of an error
 */
Expression * GlesCreateExprCompareGT(Compiler * compiler, Expression * left, Expression * right) {
	return CreateExprCompare(compiler, left, right, OpcodeSGT);
}

/**
 * Evaluate >= operator.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand, must be of type TypeInt or TypeFloat
 * @param	right		right operand, must be of type TypeInt or TypeFloat
 * 
 * @return	Expression node representing result of type TypeBool, or NULL in
 * 			case of an error
 */
Expression * GlesCreateExprCompareGE(Compiler * compiler, Expression * left, Expression * right) {
	return CreateExprCompare(compiler, left, right, OpcodeSGE);
}

static Expression * CreateExprEqual(Compiler * compiler, Expression * left, Expression * right, Opcode op);

/**
 * Generate the equality comparison for two struct values. 
 * 
 * @param	compiler	reference to compiler object
 * @param	left		reference to left operand, must be of type TypeStruct
 * @param	right		reference to right operand, must be of type TypeStruct
 * @param	op			Comparison opcode to use
 * 
 * @return	reference to result value of type TypeBool, or NULL if an error
 * 			occurred
 */
static Expression * CreateExprEqualStruct(Compiler * compiler, Expression * left, Expression * right, Opcode op) {

	GLES_ASSERT(GlesTypeMatches(left->base.type, right->base.type));
	GLES_ASSERT(left->base.type->base.kind == TypeStruct);
	
	GLsizei numFields = left->base.type->structure.numFields, index;
	Field * fields = left->base.type->structure.fields;
	
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* constant case */
		
		GLboolean result;
		
		if (op == OpcodeSEQ) {
			result = GL_TRUE;
			
			for (index = 0; index < numFields; ++index) {
				Expression * compare =
					CreateExprEqual(compiler,
									CreateExprField(compiler, left, fields[index].symbol),
									CreateExprField(compiler, right, fields[index].symbol),
									op);
										
				GLES_ASSERT(compare->base.type->base.kind == TypeBool);
				GLES_ASSERT(compare->base.kind == ExpressionKindConstant);
				
				result &= compare->constant.value[0].boolValue[0];
			}			
		} else {
			result = GL_FALSE;
			
			for (index = 0; index < numFields; ++index) {
				Expression * compare =
					CreateExprEqual(compiler,
									CreateExprField(compiler, left, fields[index].symbol),
									CreateExprField(compiler, right, fields[index].symbol),
									op);
										
				GLES_ASSERT(compare->base.type->base.kind == TypeBool);
				GLES_ASSERT(compare->base.kind == ExpressionKindConstant);
				
				result |= compare->constant.value[0].boolValue[0];
			}
		}
		
		return GlesCreateExprBoolConstant(compiler, result);
	} else {
		Expression * result = 
			CreateExprEqual(compiler,
							CreateExprField(compiler, left, fields[0].symbol),
							CreateExprField(compiler, right, fields[0].symbol),
							op);
								
		for (index = 1; index < numFields; ++index) {
			Expression * result0 = result;
			Expression * resultIndex = 
				CreateExprEqual(compiler,
								CreateExprField(compiler, left, fields[index].symbol),
								CreateExprField(compiler, right, fields[index].symbol),
								op);
								
			Symbol * resultSymbol = GlesDeclareTempVariable(compiler, result->base.type);
			result = GlesCreateExprReference(compiler, result->base.type, resultSymbol, NULL, 0);
			
			SrcReg	regResult0, regResultIndex;
			DstReg	regResult;
			
			GlesGenStore(compiler, &regResult, result, NULL);
			GlesGenFetch(compiler, &regResult0, result0, NULL);
			GlesGenFetch(compiler, &regResultIndex, resultIndex, NULL);

			GlesGenInstBinary(&compiler->generator, (op == OpcodeSEQ) ? OpcodeMIN : OpcodeMAX, PrecisionLow,  
							  &regResult, &regResult0, &regResultIndex);						
		}
		
		return result;
	} 
}

/**
 * Generate the equality comparison for two array values. This is one of those funny
 * aspects of the specification: Even though comparisons of arrays is explicitly
 * not supported, it needs to be provided since struct types can contain arrays, and
 * there is no exception made to the comparability of structs even if they contain
 * array members.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		reference to left operand, must be of type TypeArray
 * @param	right		reference to right operand, must be of type TypeArray
 * @param	op			Comparison opcode to use
 * 
 * @return	reference to result value of type TypeBool, or NULL if an error
 * 			occurred
 */
static Expression * CreateExprEqualArray(Compiler * compiler, Expression * left, Expression * right, Opcode op) {
	
	GLsizei elements = left->base.type->array.elements, index;
	
	GLES_ASSERT(GlesTypeMatches(left->base.type, right->base.type));
	GLES_ASSERT(left->base.type->base.kind = TypeArray);
	
	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* constant case */
		
		GLboolean result;
		
		if (op == OpcodeSEQ) {
			result = GL_TRUE;
			
			for (index = 0; index < elements; ++index) {
				Expression * compare =
					CreateExprEqual(compiler,
									GlesCreateExprComponent(compiler, left, index),
									GlesCreateExprComponent(compiler, right, index),
									op);
										
				GLES_ASSERT(compare->base.type->base.kind == TypeBool);
				GLES_ASSERT(compare->base.kind == ExpressionKindConstant);
				
				result &= compare->constant.value[0].boolValue[0];
			}			
		} else {
			result = GL_FALSE;
			
			for (index = 0; index < elements; ++index) {
				Expression * compare =
					CreateExprEqual(compiler,
									GlesCreateExprComponent(compiler, left, index),
									GlesCreateExprComponent(compiler, right, index),
									op);
										
				GLES_ASSERT(compare->base.type->base.kind == TypeBool);
				GLES_ASSERT(compare->base.kind == ExpressionKindConstant);
				
				result |= compare->constant.value[0].boolValue[0];
			}
		}
		
		return GlesCreateExprBoolConstant(compiler, result);
	} else {
		Expression * result = 
			CreateExprEqual(compiler,
							GlesCreateExprComponent(compiler, left, 0),
							GlesCreateExprComponent(compiler, right, 0),
							op);
								
		for (index = 1; index < elements; ++index) {
			Expression * result0 = result;
			Expression * resultIndex = 
				CreateExprEqual(compiler,
								GlesCreateExprComponent(compiler, left, index),
								GlesCreateExprComponent(compiler, right, index),
								op);
								
			Symbol * resultSymbol = GlesDeclareTempVariable(compiler, result->base.type);
			result = GlesCreateExprReference(compiler, result->base.type, resultSymbol, NULL, 0);
			
			SrcReg	regResult0, regResultIndex;
			DstReg	regResult;
			
			GlesGenStore(compiler, &regResult, result, NULL);
			GlesGenFetch(compiler, &regResult0, result0, NULL);
			GlesGenFetch(compiler, &regResultIndex, resultIndex, NULL);

			GlesGenInstBinary(&compiler->generator, (op == OpcodeSEQ) ? OpcodeMIN : OpcodeMAX, PrecisionLow, 
							  &regResult, &regResult0, &regResultIndex);						
		}
		
		return result;
	} 
}

/**
 * Generate the equality comparison for two matrix values.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		reference to left operand, must be of type TypeFloatMatx
 * @param	right		reference to right operand, must be of type TypeFloatMatx
 * @param	op			Comparison opcode to use
 * 
 * @return	reference to result value of type TypeBool, or NULL if an error
 * 			occurred
 */
static Expression * CreateExprEqualMatrix(Compiler * compiler, Expression * left, Expression * right, Opcode op) {
	Precision prec = (Precision) GlesMaxi(left->base.type->base.prec, right->base.type->base.prec);
	GLsizei row, column;
	GLsizei elements = left->base.type->base.elements;

	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);

	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* constant case */
		GLboolean result;
		
		if (op == OpcodeSEQ) {
			result = GL_TRUE;
			
			for (column = 0; column < elements; ++column) {
				for (row = 0; row < elements; ++row) {
					result &=
						left->constant.value[column].floatValue[row] ==
						right->constant.value[column].floatValue[row];
				}					
			}
		} else {
			result = GL_FALSE;
			
			for (column = 0; column < elements; ++column) {
				for (row = 0; row < elements; ++row) {
					result |=
						left->constant.value[column].floatValue[row] !=
						right->constant.value[column].floatValue[row];
				}					
			}
		}
		
		return GlesCreateExprBoolConstant(compiler, result);
	} else {
		Type * compareType = GlesVectorType(TypeBool, PrecisionUndefined, elements);
		Symbol * compareResult = GlesDeclareTempVariable(compiler, compareType);
		Expression * compareExpr = GlesCreateExprReference(compiler, compareType, compareResult, NULL, 0);
		Symbol * compareResult2 = GlesDeclareTempVariable(compiler, compareType);
		Expression * compareExpr2 = GlesCreateExprReference(compiler, compareType, compareResult2, NULL, 0);
		
		Type * countType = GlesBasicType(TypeFloat, PrecisionLow);
		Symbol * countResult = GlesDeclareTempVariable(compiler, countType);
		Expression * countExpr = GlesCreateExprReference(compiler, countType, countResult, NULL, 0);
		
		Type * equalityType = GlesBasicType(TypeBool, PrecisionUndefined);
		Symbol * equalityResult = GlesDeclareTempVariable(compiler, equalityType);
		Expression * equalityExpr = GlesCreateExprReference(compiler, equalityType, equalityResult, NULL, 0);
		
		Opcode countOp = elements == 2 ? OpcodeDP2 : elements == 3 ? OpcodeDP3 : OpcodeDP4;
		
		DstReg regCompareResult, regCompareResult2, regCountResult, regEqualityResult;
		SrcReg regLeft, regRight, regCompareResultSrc, regCompareResultSrc2, regCountResultSrc, regEqualityOperand;
		
		GlesGenStore(compiler, &regCompareResult, compareExpr, NULL);
		GlesGenFetch(compiler, &regLeft, GlesCreateExprComponent(compiler, left, 0), NULL);
		GlesGenFetch(compiler, &regRight, GlesCreateExprComponent(compiler, right, 0), NULL);		
		
		GlesGenInstBinary(&compiler->generator, op, prec, 
						  &regCompareResult, &regLeft, &regRight);
						  
		GlesGenFetch(compiler, &regCompareResultSrc, compareExpr, NULL);
		
		for (column = 1; column < elements; ++column) {
			GlesGenStore(compiler, &regCompareResult2, compareExpr, NULL);
			GlesGenFetch(compiler, &regLeft, GlesCreateExprComponent(compiler, left, column), NULL);
			GlesGenFetch(compiler, &regRight, GlesCreateExprComponent(compiler, right, column), NULL);		
			
			GlesGenInstBinary(&compiler->generator, op, prec, 
							  &regCompareResult, &regLeft, &regRight);
							  
			GlesGenFetch(compiler, &regCompareResultSrc2, compareExpr2, NULL);
			
			GlesGenInstBinary(&compiler->generator, (op == OpcodeSEQ) ? OpcodeMIN : OpcodeMAX, PrecisionLow, 
							  &regCompareResult, &regCompareResultSrc, &regCompareResultSrc2);
		}		
		
		GlesGenStore(compiler, &regCountResult, countExpr, NULL);
		
		GlesGenInstBinary(&compiler->generator, countOp, PrecisionLow,
						  &regCountResult, &regCompareResultSrc, &regCompareResultSrc);
						  
		GlesGenStore(compiler, &regEqualityResult, equalityExpr, NULL);
		GlesGenFetch(compiler, &regCountResultSrc, countExpr, NULL);
		
		if (op == OpcodeSEQ) {
			GlesGenFetch(compiler, &regEqualityOperand, GlesCreateExprIntConstant(compiler, elements), NULL);
			GlesGenInstBinary(&compiler->generator, OpcodeSEQ, PrecisionLow,
							  &regEqualityResult, &regCountResultSrc, &regEqualityOperand);
		} else {
			GlesGenFetch(compiler, &regEqualityOperand, GlesCreateExprIntConstant(compiler, 0), NULL);
			GlesGenInstBinary(&compiler->generator, OpcodeSGT, PrecisionLow,
							  &regEqualityResult, &regCountResultSrc, &regEqualityOperand);
		}
		
		return equalityExpr;
	}
		
	return NULL;
}

/**
 * Generate the equality comparison for two vector values.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		reference to left operand, must be of type TypeBoolVecx,
 * 						TypeIntVecx, or TypeFloatVecx
 * @param	right		reference to right operand, must be of type TypeBoolVecx,
 * 						TypeIntVecx, or TypeFloatVecx
 * @param	op			Comparison opcode to use
 * 
 * @return	reference to result value of type TypeBool, or NULL if an error
 * 			occurred
 */
static Expression * CreateExprEqualVector(Compiler * compiler, Expression * left, Expression * right, Opcode op) {
	Type * type = GlesBasicType(TypeBool, PrecisionUndefined);
	Precision prec = (Precision) GlesMaxi(left->base.type->base.prec, right->base.type->base.prec);
	GLsizei index;
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);

	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* constant case */
					
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (left->base.type->base.kind == TypeBoolVec2 ||
			left->base.type->base.kind == TypeBoolVec3 ||
			left->base.type->base.kind == TypeBoolVec4) {
			GLboolean result;
			
			if (op == OpcodeSEQ) {
				result =
					left->constant.value[0].boolValue[0] ==
					right->constant.value[0].boolValue[0];
					
				for (index = 1; index < left->base.type->base.elements; ++index) {
					result &=
						left->constant.value[0].boolValue[index] ==
						right->constant.value[0].boolValue[index];
				}
			} else {
				GLES_ASSERT(op == OpcodeSNE);
				
				result =
					left->constant.value[0].boolValue[0] !=
					right->constant.value[0].boolValue[0];
					
				for (index = 1; index < left->base.type->base.elements; ++index) {
					result |=
						left->constant.value[0].boolValue[index] !=
						right->constant.value[0].boolValue[index];
				}
			}
			
			constant[0].boolValue[0] = result;
		} else if (left->base.type->base.kind == TypeIntVec2 ||
				   left->base.type->base.kind == TypeIntVec3 ||
				   left->base.type->base.kind == TypeIntVec4) {
			GLboolean result;
			
			if (op == OpcodeSEQ) {
				result =
					left->constant.value[0].intValue[0] ==
					right->constant.value[0].intValue[0];
					
				for (index = 1; index < left->base.type->base.elements; ++index) {
					result &=
						left->constant.value[0].intValue[index] ==
						right->constant.value[0].intValue[index];
				}
			} else {
				GLES_ASSERT(op == OpcodeSNE);
				
				result =
					left->constant.value[0].intValue[0] !=
					right->constant.value[0].intValue[0];
					
				for (index = 1; index < left->base.type->base.elements; ++index) {
					result |=
						left->constant.value[0].intValue[index] !=
						right->constant.value[0].intValue[index];
				}
			}
			
			constant[0].boolValue[0] = result;
		} else {
			GLES_ASSERT(left->base.type->base.kind == TypeFloatVec2 ||
						left->base.type->base.kind == TypeFloatVec3 ||
						left->base.type->base.kind == TypeFloatVec4);
						
			GLboolean result;
			
			if (op == OpcodeSEQ) {
				result =
					left->constant.value[0].floatValue[0] ==
					right->constant.value[0].floatValue[0];
					
				for (index = 1; index < left->base.type->base.elements; ++index) {
					result &=
						left->constant.value[0].floatValue[index] ==
						right->constant.value[0].floatValue[index];
				}
			} else {
				GLES_ASSERT(op == OpcodeSNE);
				
				result =
					left->constant.value[0].floatValue[0] !=
					right->constant.value[0].floatValue[0];
					
				for (index = 1; index < left->base.type->base.elements; ++index) {
					result |=
						left->constant.value[0].floatValue[index] !=
						right->constant.value[0].floatValue[index];
				}
			}
			
			constant[0].boolValue[0] = result;
		} 
				
		return GlesCreateExprConstant(compiler, type, constant);
		
	} else {
		/* general case */
		SrcReg	regLeft, regRight;
		
		GLsizei elements = left->base.type->base.elements;

		Type * compareType = GlesVectorType(TypeBool, PrecisionUndefined, elements);
		Symbol * compareResult = GlesDeclareTempVariable(compiler, compareType);
		Expression * compareExpr = GlesCreateExprReference(compiler, compareType, compareResult, NULL, 0);
		
		Type * countType = GlesBasicType(TypeFloat, PrecisionLow);
		Symbol * countResult = GlesDeclareTempVariable(compiler, countType);
		Expression * countExpr = GlesCreateExprReference(compiler, countType, countResult, NULL, 0);
		
		Type * equalityType = GlesBasicType(TypeBool, PrecisionUndefined);
		Symbol * equalityResult = GlesDeclareTempVariable(compiler, equalityType);
		Expression * equalityExpr = GlesCreateExprReference(compiler, equalityType, equalityResult, NULL, 0);
		
		Opcode countOp = elements == 2 ? OpcodeDP2 : elements == 3 ? OpcodeDP3 : OpcodeDP4;
		
		DstReg regCompareResult, regCountResult, regEqualityResult;
		SrcReg regCompareResultSrc, regCountResultSrc, regEqualityOperand;
		
		GlesGenFetch(compiler, &regLeft, left, NULL);
		GlesGenFetch(compiler, &regRight, right, NULL);
		GlesGenStore(compiler, &regCompareResult, compareExpr, NULL);
		
		GlesGenInstBinary(&compiler->generator, op, prec, 
						  &regCompareResult, &regLeft, &regRight);
						  
		GlesGenFetch(compiler, &regCompareResultSrc, compareExpr, NULL);
		GlesGenStore(compiler, &regCountResult, countExpr, NULL);
		
		GlesGenInstBinary(&compiler->generator, countOp, PrecisionLow,
						  &regCountResult, &regCompareResultSrc, &regCompareResultSrc);
						  
		GlesGenStore(compiler, &regEqualityResult, equalityExpr, NULL);
		GlesGenFetch(compiler, &regCountResultSrc, countExpr, NULL);
		
		if (op == OpcodeSEQ) {
			GlesGenFetch(compiler, &regEqualityOperand, GlesCreateExprIntConstant(compiler, elements), NULL);
			GlesGenInstBinary(&compiler->generator, OpcodeSEQ, PrecisionLow,
							  &regEqualityResult, &regCountResultSrc, &regEqualityOperand);
		} else {
			GlesGenFetch(compiler, &regEqualityOperand, GlesCreateExprIntConstant(compiler, 0), NULL);
			GlesGenInstBinary(&compiler->generator, OpcodeSGT, PrecisionLow,
							  &regEqualityResult, &regCountResultSrc, &regEqualityOperand);
		}
		
		return equalityExpr;
	}
}

/**
 * Generate the equality comparison for two scalar values.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		reference to left operand, must be of type TypeBool,
 * 						TypeInt, TypeFloat, TypeSampler2D, TypeSampler3D or TypeSamplerCube
 * @param	right		reference to right operand, must be of type TypeBool,
 * 						TypeInt, TypeFloat, TypeSampler2D, TypeSampler3D or TypeSamplerCube
 * @param	op			Comparison opcode to use
 * 
 * @return	reference to result value of type TypeBool, or NULL if an error
 * 			occurred
 */
static Expression * CreateExprEqualScalar(Compiler * compiler, Expression * left, Expression * right, Opcode op) {
	
	GLES_ASSERT(left->base.type->base.kind == right->base.type->base.kind);
	Type * type = GlesBasicType(TypeBool, PrecisionUndefined);
	Precision prec = (Precision) GlesMaxi(left->base.type->base.prec, right->base.type->base.prec);

	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
			
		Constant * constant = GlesMemoryPoolAllocate(compiler->exprMemory, sizeof(Constant));
		
		if (left->base.type->base.kind == TypeBool) {
			GLboolean leftValue	 = left->constant.value[0].boolValue[0];
			GLboolean rightValue = right->constant.value[0].boolValue[0];
			GLboolean result;
			
			switch (op) {
			case OpcodeSEQ:	result = (leftValue == rightValue);	break;
			case OpcodeSNE:	result = (leftValue != rightValue);	break;
			default:
				GLES_ASSERT(GL_FALSE);
			}
			
			constant[0].boolValue[0] = result;
		} else if (left->base.type->base.kind == TypeFloat) {
			GLfloat leftValue	= left->constant.value[0].floatValue[0];
			GLfloat rightValue 	= right->constant.value[0].floatValue[0];
			GLboolean result;
			
			switch (op) {
			case OpcodeSEQ:	result = (leftValue == rightValue);	break;
			case OpcodeSNE:	result = (leftValue != rightValue);	break;
			default:
				GLES_ASSERT(GL_FALSE);
			}
			
			constant[0].boolValue[0] = result;
		} else {
			GLES_ASSERT(left->base.type->base.kind == TypeInt 		||
						left->base.type->base.kind == TypeSampler2D	||
						left->base.type->base.kind == TypeSampler3D	||
						left->base.type->base.kind == TypeSamplerCube);
						
			GLint leftValue	 = left->constant.value[0].intValue[0];
			GLint rightValue = right->constant.value[0].intValue[0];
			GLboolean result;
			
			switch (op) {
			case OpcodeSEQ:	result = (leftValue == rightValue);	break;
			case OpcodeSNE:	result = (leftValue != rightValue);	break;
			default:
				GLES_ASSERT(GL_FALSE);
			}
			
			constant[0].boolValue[0] = result;
		}
		
		return GlesCreateExprConstant(compiler, type, constant);
	} else {
		SrcReg	regLeft, regRight;
		DstReg	regResult;
		
		Symbol * resultVar = GlesDeclareTempVariable(compiler, type);
		Expression * result = GlesCreateExprReference(compiler, type, resultVar, NULL, 0);

		GlesGenStore(compiler, &regResult, result, NULL);
		GlesGenFetch(compiler, &regLeft, left, NULL);
		GlesGenFetch(compiler, &regRight, right, NULL);
		
		GlesGenInstBinary(&compiler->generator, op, prec,
						  &regResult, &regLeft, &regRight);
						  		
		return result;
	}
}

static Expression * CreateExprEqual(Compiler * compiler, Expression * left, Expression * right, Opcode op) {

	if (!GlesTypeMatches(left->base.type, right->base.type)) {
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;	
	}
	
	switch (left->base.type->base.kind) {
	case TypeStruct:
		return CreateExprEqualStruct(compiler, left, right, op);
		
	case TypeArray:
		return CreateExprEqualArray(compiler, left, right, op);
		
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		return CreateExprEqualMatrix(compiler, left, right, op);
		
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
		
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		return CreateExprEqualVector(compiler, left, right, op);		
		
	case TypeBool:
	case TypeInt:
	case TypeFloat:

	case TypeSampler2D:	
	case TypeSampler3D:	
	case TypeSamplerCube:
		return CreateExprEqualScalar(compiler, left, right, op);
		
	default:
		;
	}
	
	/* S0004: Operator not supported for operand types */
	GlesCompileError(compiler, ErrS0004);
	return NULL;	
}

Expression * GlesCreateExprCompareEQ(Compiler * compiler, Expression * left, Expression * right) {
	return CreateExprEqual(compiler, left, right, OpcodeSEQ);
}

Expression * GlesCreateExprCompareNE(Compiler * compiler, Expression * left, Expression * right) {
	return CreateExprEqual(compiler, left, right, OpcodeSNE);
}

/**
 * This function is called when we see an && operator. Effectively, it
 * opens an if block.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand side of && operator
 * 
 * @return	if left is a constant, this constant value is returned. Otherwise,
 * 			a new temporary variable is created and the value of left is
 * 			assigned to it. NULL in case of an error.
 */
Expression * GlesPrepareExprAnd(Compiler * compiler, Expression * left) {
	
	if (left->base.type->base.kind != TypeBool) {
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
	
	if (left->base.kind == ExpressionKindConstant) {
		/* constant case; pass on as result */
		
		if (!GlesCreateStmntIf(compiler, left, GL_TRUE) ||
			!GlesCreateStmntElse(compiler)) {
			return NULL;
		}
		
		return left;
	} else {
		Type * type = GlesBasicType(TypeBool, PrecisionUndefined);
		Symbol * resultValue = GlesDeclareTempVariable(compiler, type);
		Expression * resultExpr = GlesCreateExprReference(compiler, type, resultValue, NULL, 0);
		SrcReg regSrc;
		DstReg regDst;
				
		if (!GlesCreateStmntIf(compiler, left, GL_FALSE)) {
			return NULL;
		}
		
		GlesGenStore(compiler, &regDst, resultExpr, NULL);
		GlesGenFetch(compiler, &regSrc, left, NULL);
		
		GlesGenInstUnary(&compiler->generator, OpcodeMOV, PrecisionUndefined, 
						 &regDst, &regSrc);
		
		if (!GlesCreateStmntElse(compiler)) {
			return NULL;
		} else {
			return resultExpr;
		}
	}
}

/**
 * Finish handling of an && operator.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left hand side as created by GlesPrepareExprAnd
 * @param	right		right hand side operand
 * 
 * @return	resulting expression value node, or NULL in case of an error
 */
Expression * GlesCreateExprAnd(Compiler * compiler, Expression * left, Expression * right) {

	GLES_ASSERT(left->base.type->base.kind == TypeBool);
	
	if (left->base.type->base.kind != TypeBool) {
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
	
	if (left->base.kind == ExpressionKindConstant) {
		/* constant case; pass on as result */
		
		GlesCreateStmntEndif(compiler);

		if (!left->constant.value[0].boolValue[0]) {
			return left;
		} else {
			return right;
		}
	} else {
		SrcReg regSrc;
		DstReg regDst;
		Swizzle swizzle;
				
		GLES_ASSERT(left->base.kind == ExpressionKindReference);
				
		GlesGenStore(compiler, &regDst, left, &swizzle);
		GlesGenFetch(compiler, &regSrc, right, &swizzle);
		
		GlesGenInstUnary(&compiler->generator, OpcodeMOV, PrecisionUndefined,
						 &regDst, &regSrc);

		GlesCreateStmntEndif(compiler);
		
		return left;
	}	
}

Expression * GlesPrepareExprOr(Compiler * compiler, Expression * left) {
	if (left->base.type->base.kind != TypeBool) {
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
	
	if (left->base.kind == ExpressionKindConstant) {
		/* constant case; pass on as result */
		
		if (!GlesCreateStmntIf(compiler, left, GL_TRUE) ||
			!GlesCreateStmntElse(compiler)) {
			return NULL;
		}
		
		return left;
	} else {
		Type * type = GlesBasicType(TypeBool, PrecisionUndefined);
		Symbol * resultValue = GlesDeclareTempVariable(compiler, type);
		Expression * resultExpr = GlesCreateExprReference(compiler, type, resultValue, NULL, 0);
		SrcReg regSrc;
		DstReg regDst;
				
		if (!GlesCreateStmntIf(compiler, left, GL_TRUE)) {
			return NULL;
		}
		
		GlesGenStore(compiler, &regDst, resultExpr, NULL);
		GlesGenFetch(compiler, &regSrc, left, NULL);
		
		GlesGenInstUnary(&compiler->generator, OpcodeMOV, PrecisionUndefined, 
						 &regDst, &regSrc);
		
		if (!GlesCreateStmntElse(compiler)) {
			return NULL;
		} else {
			return resultExpr;
		}
	}
}

Expression * GlesCreateExprOr(Compiler * compiler, Expression * left, Expression * right) {
	GLES_ASSERT(left->base.type->base.kind == TypeBool);
	
	if (left->base.type->base.kind != TypeBool) {
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
	
	if (left->base.kind == ExpressionKindConstant) {
		/* constant case; pass on as result */
		
		GlesCreateStmntEndif(compiler);

		if (left->constant.value[0].boolValue[0]) {
			return left;
		} else {
			return right;
		}
	} else {
		SrcReg regSrc;
		DstReg regDst;
				
		GLES_ASSERT(left->base.kind == ExpressionKindReference);
				
		GlesGenStore(compiler, &regDst, left, NULL);
		GlesGenFetch(compiler, &regSrc, right, NULL);
		
		GlesGenInstUnary(&compiler->generator, OpcodeMOV, PrecisionUndefined, 
						 &regDst, &regSrc);

		GlesCreateStmntEndif(compiler);
		
		return left;
	}	
}

/**
 * Evaluate an exclusive or operation.
 * 
 * @param	compiler	reference to compiler object
 * @param 	left		reference to left operand, must be of type TypeBool
 * @param 	right		reference to right operand, must be of type TypeBool
 * 
 * @return	Reference to resulting expression node of type TypeBool, or
 * 			NULL in case of an error.
 */
Expression * GlesCreateExprXor(Compiler * compiler, Expression * left, Expression * right) {

	Type * type = GlesBasicType(TypeBool, PrecisionUndefined);
	
	if (left->base.type->base.kind != TypeBool ||
		right->base.type->base.kind != TypeBool) {
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}

	if (left->base.kind == ExpressionKindConstant &&
		right->base.kind == ExpressionKindConstant) {
		/* constant case */
		
		return GlesCreateExprBoolConstant(compiler,
										  left->constant.value[0].boolValue[0] ^
										  right->constant.value[0].boolValue[0]);
	} else {
		SrcReg	regLeft, regRight;
		DstReg	regDiff, regResult;
		
		Symbol * resultSymbol = GlesDeclareTempVariable(compiler, type);
		Expression * resultExpr = GlesCreateExprReference(compiler, type, resultSymbol, NULL, 0);
		
		GlesGenStore(compiler, &regResult, resultExpr, NULL);
		GlesGenFetch(compiler, &regLeft, left, NULL);
		GlesGenFetch(compiler, &regRight, right, NULL);
		
		GlesGenInstBinary(&compiler->generator, OpcodeSNE, PrecisionLow, 
						  &regDiff, &regLeft, &regRight);
						  
		return resultExpr;
	}
}

/**
 * Evaluate an assignment operation.
 * 
 * @param	left	left-hand side of assignment operator, must be assignable l-value
 * @param	right	right-hand side of assignment operator
 * 
 * @return 	left hand-side expression, or NULL in case of an error
 */
Expression * GlesCreateExprAssign(Compiler * compiler, Expression * left, Expression * right) {

	Type * type = left->base.type;
	SrcReg srcReg;
	DstReg dstReg;
	Swizzle swizzle;
	GLsizei index;

	if (left->base.kind == ExpressionKindVectorConstructor &&
		!left->vectorConstructor.assignable) {
		/* S00037: L-value contains duplicate components */
		GlesCompileError(compiler, ErrS0037);
		return NULL;
	}
	
	if (!IsLValue(compiler, left)) {
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;
	}
	
	if (!GlesTypeMatches(left->base.type, right->base.type)) {
		/* S0001: Type mismatch in expression */
		GlesCompileError(compiler, ErrS0001);
		return NULL;
	}
	
	switch (left->base.type->base.kind) {
	case TypeStruct:
		/* member-wise assignment */
		
		GLES_ASSERT(left->base.kind == ExpressionKindReference);
		
		for (index = 0; index < type->structure.numFields; ++index) {
			if (!GlesCreateExprAssign(compiler, 
									  CreateExprField(compiler, left, type->structure.fields[index].symbol), 
									  CreateExprField(compiler, right, type->structure.fields[index].symbol))) {
				return NULL;
			}
		}
		
		break;
		
	case TypeArray:
		/* element-wise assignment */
		for (index = 0; index < type->array.elements; ++index) {
			if (!GlesCreateExprAssign(compiler, 
									  GlesCreateExprComponent(compiler, left, index), 
									  GlesCreateExprComponent(compiler, right, index))) {
				return NULL;
			}
		}
		
		break;
		
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		/* column-wise assignment */
		for (index = 0; index < type->array.elements; ++index) {
			if (!GlesCreateExprAssign(compiler, 
									  GlesCreateExprComponent(compiler, left, index), 
									  GlesCreateExprComponent(compiler, right, index))) {
				return NULL;
			}
		}
		
		break;
		
	case TypeBool:
	case TypeInt:
	case TypeFloat:
	
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:

	case TypeSampler2D:	
	case TypeSampler3D:	
	case TypeSamplerCube:
		/* direct transfer of values */
		GlesGenStore(compiler, &dstReg, left, &swizzle);
		GlesGenFetch(compiler, &srcReg, right, &swizzle);
		GlesGenInstUnary(&compiler->generator, OpcodeMOV, type->base.prec, &dstReg, &srcReg);
		
		break;
		
	default:
		/* S0004: Operator not supported for operand types */
		GlesCompileError(compiler, ErrS0004);
		return NULL;	
	}
	
	/* If we need to keep track of constant initializers, do it here */
	
	if (compiler->collectInitializers 				&&
		left->base.kind == ExpressionKindReference 	&&
		left->reference.index == NULL 				&& 
		left->reference.offset == 0) {
		InitializerList * entry = GlesMemoryPoolAllocate(compiler->moduleMemory, sizeof(InitializerList));
		
		entry->next = compiler->initializers;
		entry->symbol = left->reference.ref;
		entry->value = right;
		compiler->initializers = entry;
	}
	
	return right;
}

/**
 * Evaluate a multiply-assignment operation.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand
 * @param	right		right operand
 * 
 * @return	reference to resulting expression value, or NULL in case of error.
 */
Expression * GlesCreateExprAssignMul(Compiler * compiler, Expression * left, Expression * right) {
	Expression * result = GlesCreateExprMul(compiler, left, right);
	
	if (result) {
		if (!GlesCreateExprAssign(compiler, left, result)) {
			return NULL;
		}
	} 
	
	return result;
}

/**
 * Evaluate a divide-assignment operation.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand
 * @param	right		right operand
 * 
 * @return	reference to resulting expression value, or NULL in case of error.
 */
Expression * GlesCreateExprAssignDiv(Compiler * compiler, Expression * left, Expression * right) {
	Expression * result = GlesCreateExprDiv(compiler, left, right);
	
	if (result) {
		if (!GlesCreateExprAssign(compiler, left, result)) {
			return NULL;
		}
	} 
	
	return result;
}

/**
 * Evaluate a add-assignment operation.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand
 * @param	right		right operand
 * 
 * @return	reference to resulting expression value, or NULL in case of error.
 */
Expression * GlesCreateExprAssignAdd(Compiler * compiler, Expression * left, Expression * right) {
	Expression * result = GlesCreateExprAdd(compiler, left, right);
	
	if (result) {
		if (!GlesCreateExprAssign(compiler, left, result)) {
			return NULL;
		}
	} 
	
	return result;
}

/**
 * Evaluate a subtract-assignment operation.
 * 
 * @param	compiler	reference to compiler object
 * @param	left		left operand
 * @param	right		right operand
 * 
 * @return	reference to resulting expression value, or NULL in case of error.
 */
Expression * GlesCreateExprAssignSub(Compiler * compiler, Expression * left, Expression * right) {
	Expression * result = GlesCreateExprSub(compiler, left, right);
	
	if (result) {
		if (!GlesCreateExprAssign(compiler, left, result)) {
			return NULL;
		}
	} 
	
	return result;
}

/*
** --------------------------------------------------------------------------
** Functions - Derived expression building blocks to set a condition code
** register.
** --------------------------------------------------------------------------
*/

/**
 * Evaluate an expression into a temporary variable.
 * 
 * @param	compiler	reference to compiler object
 * @param	expr		the expression to evaluate
 * 
 * @return	a new expression node that is guaranteed to be a reference node.
 */
Expression /* Reference */ * GlesEvaluate(Compiler * compiler, Expression * expr) {
	Symbol * resultSymbol = GlesDeclareTempVariable(compiler, expr->base.type);
	Expression * resultExpr = GlesCreateExprReference(compiler, expr->base.type, resultSymbol, NULL, 0);
	
	GlesCreateExprAssign(compiler, resultExpr, expr);
	
	return resultExpr;	
}

/**
 * Try to extract a single reference node out of all vector components. If this is the
 * case, this common reference node is returned. Otherwise the result is NULL.
 * 
 * @param	constructor	the constructor node to examine
 * 
 * @return	a single reference node common to all vector components, or NULL if
 * 			no such node exists.
 */
static ExpressionReference * TrySimpleSwizzle(ExpressionVectorConstructor * constructor) {
	GLsizei elements = constructor->base.type->base.elements;
	ExpressionReference * reference, *reference2;
	
	if (constructor->slots[0].base->kind != ExpressionKindFixedComponent) {
		return NULL;
	}
	
	reference = constructor->slots[0].fixedComponent->expr;
	
	while (elements > 1) {
		--elements;
		
		if (constructor->slots[elements].base->kind != ExpressionKindFixedComponent) {
			return NULL;
		}
		
		reference2 = constructor->slots[elements].fixedComponent->expr;
		
		if (reference->ref != reference2->ref ||
			reference->offset != reference2->offset ||
			reference->index != reference2->index) {
			return NULL;
		}
	}
	
	return reference;
}

/**
 * Try to extract a single reference node and an extended swizzle pattern
 * out of all vector components. If this is the case, this common reference node 
 * is returned. Otherwise the result is NULL.
 * 
 * @param	constructor	the constructor node to examine
 * @param	out: reference to an array of 4 extended swizzle components, which will
 * 			be filled with the correct components values upon success.
 * 
 * @return	a single reference node common to all vector components, or NULL if
 * 			no such node exists.
 */
static ExpressionReference * TryExtendedSwizzle(ExpressionVectorConstructor * constructor,
												ExtSwizzleOption swizzle[4]) {
	Type * type = constructor->base.type;
	GLsizei elements = type->base.elements;
	ExpressionReference * reference = NULL, *reference2;
	
	while (elements > 0) {
		--elements;
		
		if (constructor->slots[elements].base->kind == ExpressionKindConstant) {
			Constant * constant = constructor->slots[elements].constant->value;
			
			switch (constructor->slots[elements].constant->base.type->base.kind) {
			case TypeBool:
				/* boolean is 0/1 by default */
				if (constant->boolValue[0]) {
					swizzle[elements] = ExtSwizzleSelect1;
				} else {
					swizzle[elements] = ExtSwizzleSelect0;
				}
				
				continue;
				
			case TypeInt:
				if (constant->intValue[0] == 0) {
					swizzle[elements] = ExtSwizzleSelect0;
				} else if(constant->intValue[0] == 1) {
					swizzle[elements] = ExtSwizzleSelect1;
				} else if(constant->intValue[0] == -1) {
					swizzle[elements] = ExtSwizzleSelectNeg1;
				} else {
					return NULL;
				}

				continue;
				
			case TypeFloat:
				if (constant->floatValue[0] == 0.0f) {
					swizzle[elements] = ExtSwizzleSelect0;
				} else if (constant->floatValue[0] == 1.0f) {
					swizzle[elements] = ExtSwizzleSelect1;
				} else if (constant->floatValue[0] == -1.0f) {
					swizzle[elements] = ExtSwizzleSelectNeg1;
				} else {
					return NULL;
				}

				continue;
				
			default:
				GLES_ASSERT(GL_FALSE);
				return NULL;
			}
			
			continue;
		} else if (constructor->slots[elements].base->kind != ExpressionKindFixedComponent) {
			return NULL;
		}

		swizzle[elements] = constructor->slots[elements].fixedComponent->index;
				
		if (!reference) {
			reference = constructor->slots[elements].fixedComponent->expr;
		} else {
			reference2 = constructor->slots[elements].fixedComponent->expr;
			
			if (reference->ref != reference2->ref ||
				reference->offset != reference2->offset ||
				reference->index != reference2->index) {
				return NULL;
			}
		}
	}
	
	return reference;
}

static void GenVectorFetch(Compiler * compiler, SrcReg * reg, ExpressionVectorConstructor * constructor, 
						   const Swizzle * swizzle) {
	/* case 3: general case: split up 0 and 1 components; split up components by reference */
	/* handle this recursively: remove 1 reference, load */
	/* of constant part is only 0 and 1 let's start with an extended swizzle. Otherwise
	 * load a regular constant */
	GLsizei elements = constructor->base.type->base.elements, index;
	GLboolean hasConstant = GL_FALSE, isZeroOne = GL_TRUE;
	ExpressionVectorElement slots[4], origSlots[4];
	Symbol * resultSymbol = GlesDeclareTempVariable(compiler, constructor->base.type);
	Expression * resultExpr = GlesCreateExprReference(compiler, constructor->base.type, resultSymbol, NULL, 0);
	
	SrcReg	regSrc;
	DstReg	regDst;
	
	GlesGenStore(compiler, &regDst, resultExpr, NULL);
	
	for (index = 0; index < elements; ++index) {
		slots[index] = origSlots[index] = constructor->slots[index];
		
		if (slots[index].base->kind == ExpressionKindConstant) {
			Constant * constant = slots[index].constant->value;
			hasConstant = GL_TRUE;
		
			switch (slots[index].constant->base.type->base.kind) {
			case TypeBool:
				/* boolean is 0/1 by default */
				break;
				
			case TypeInt:
					if (constant->intValue[0] !=  0 &&
						constant->intValue[0] !=  1 &&
						constant->intValue[0] != -1) {
					isZeroOne = GL_FALSE;
				}

				break;
				
			case TypeFloat:
					if (constant->floatValue[0] !=  0.0f &&
						constant->floatValue[0] !=  1.0f &&
						constant->floatValue[0] != -1.0f) {
					isZeroOne = GL_FALSE;
				}

				break;
				
			default:
				GLES_ASSERT(GL_FALSE);
			}
		}
	}

	if (hasConstant) {
		if (isZeroOne) {
			/* generate an extended swizzle */
			ExpressionReference * ref = NULL;
			ExpressionFixedComponent * filler = NULL;
			GLboolean writeMask[4] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE };
			
			for (index = 0; index < elements; ++index) {
				if (slots[index].base->kind == ExpressionKindConstant) {
					constructor->slots[index].constant = slots[index].constant;
					slots[index].base = NULL;
					writeMask[index] = GL_TRUE;
				} else {
					GLES_ASSERT(slots[index].base->kind == ExpressionKindFixedComponent);
					
					if (ref == NULL) {
						filler = slots[index].fixedComponent;
						ref = slots[index].fixedComponent->expr;
						constructor->slots[index].fixedComponent = slots[index].fixedComponent;
						slots[index].fixedComponent = NULL;
						writeMask[index] = GL_TRUE;
					} else if (slots[index].fixedComponent->expr == ref) {
						constructor->slots[index].fixedComponent = slots[index].fixedComponent;
						slots[index].fixedComponent = NULL;
						writeMask[index] = GL_TRUE;
					} else {
						constructor->slots[index].fixedComponent = filler;						
					}
				}				
			}
			
			regDst.maskX = writeMask[0];
			regDst.maskY = writeMask[1];
			regDst.maskZ = writeMask[2];
			regDst.maskW = writeMask[3];
			
			GlesGenFetch(compiler, &regSrc, (Expression *) constructor, NULL);
		} else {
			GLboolean writeMask[4] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE };
			Constant value;
			
			GlesMemset(&value, 0, sizeof(Constant));
			
			for (index = 0; index < elements; ++index) {
				if (slots[index].base->kind == ExpressionKindConstant) {
					writeMask[index] = GL_TRUE;
					
					switch (constructor->base.type->base.kind) {
					case TypeBool:
					case TypeBoolVec2:
					case TypeBoolVec3:
					case TypeBoolVec4:
						value.boolValue[index] = slots[index].constant->value->boolValue[0];
						break;
						
					case TypeInt:
					case TypeIntVec2:
					case TypeIntVec3:
					case TypeIntVec4:
						value.intValue[index] = slots[index].constant->value->intValue[0];
						break;
						
					case TypeFloat:
					case TypeFloatVec2:
					case TypeFloatVec3:
					case TypeFloatVec4:
						value.floatValue[index] = slots[index].constant->value->floatValue[0];
						break;
											
					default:
						GLES_ASSERT(GL_FALSE);
					}
					
					slots[index].base = NULL;
				}
			}
						
			/* generate a fetch for a constant value */
			GlesGenFetch(compiler, &regSrc, 
						 GlesCreateExprConstant(compiler, constructor->base.type, &value), NULL);
		}
	} else {
		ExpressionReference * ref = NULL;
		ExpressionFixedComponent * filler = NULL;
		GLboolean writeMask[4] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE };
		
		for (index = 0; index < elements; ++index) {
			GLES_ASSERT(slots[index].base->kind == ExpressionKindFixedComponent);
			
			if (ref == NULL) {
				filler = slots[index].fixedComponent;
				ref = slots[index].fixedComponent->expr;
				constructor->slots[index].fixedComponent = slots[index].fixedComponent;
				slots[index].fixedComponent = NULL;
				writeMask[index] = GL_TRUE;
			} else if (slots[index].fixedComponent->expr == ref) {
				constructor->slots[index].fixedComponent = slots[index].fixedComponent;
				slots[index].fixedComponent = NULL;
				writeMask[index] = GL_TRUE;
			} else {
				constructor->slots[index].fixedComponent = filler;						
			}
		}
		
		regDst.maskX = writeMask[0];
		regDst.maskY = writeMask[1];
		regDst.maskZ = writeMask[2];
		regDst.maskW = writeMask[3];
		
		GlesGenFetch(compiler, &regSrc, (Expression *) constructor, NULL);
	}
	
	GlesGenInstUnary(&compiler->generator, OpcodeMOV, constructor->base.type->base.prec, 
					 &regDst, &regSrc);
					 
	while (GL_TRUE) {
		ExpressionReference * ref = NULL;
		ExpressionFixedComponent * filler = NULL;
		GLboolean writeMask[4] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE };
		
		/* search for next unprocessed slot */
		
		for (index = 0; index < elements; ++index) {
			if (slots[index].base) {
				break;
			}
		}
		
		if (index == elements) {
			break;
		}
		
		for (index = 0; index < elements; ++index) {
			if (!slots[index].base) {
				constructor->slots[index].fixedComponent = NULL;						
				continue;
			}
				
			GLES_ASSERT(slots[index].base->kind == ExpressionKindFixedComponent);
			
			if (ref == NULL) {
				filler = slots[index].fixedComponent;
				ref = slots[index].fixedComponent->expr;
				constructor->slots[index].fixedComponent = slots[index].fixedComponent;
				slots[index].fixedComponent = NULL;
				writeMask[index] = GL_TRUE;
			} else if (slots[index].fixedComponent->expr == ref) {
				constructor->slots[index].fixedComponent = slots[index].fixedComponent;
				slots[index].fixedComponent = NULL;
				writeMask[index] = GL_TRUE;
			} else {
				constructor->slots[index].fixedComponent = filler;						
			}
		}

		for (index = 0; index < elements; ++index) {
			if (!constructor->slots[index].fixedComponent) {
				constructor->slots[index].fixedComponent = filler;
			}
		}
		
		regDst.maskX = writeMask[0];
		regDst.maskY = writeMask[1];
		regDst.maskZ = writeMask[2];
		regDst.maskW = writeMask[3];
		
		GlesGenFetch(compiler, &regSrc, (Expression *) constructor, NULL);
		GlesGenInstUnary(&compiler->generator, OpcodeMOV, constructor->base.type->base.prec, 
						 &regDst, &regSrc);					 
	}
	
	GlesGenFetch(compiler, reg, resultExpr, swizzle);
	
	/* restore original node */
	
	for (index = 0; index < elements; ++index) {
		constructor->slots[index] = origSlots[index];
	}
}

/**
 * Initialize the register structure to reference the given expression. The
 * order of the components is determined by the swizzle argment.
 * 
 * @param	reg		reference to source register structure to initialize
 * @param	expr	reference to expression node to load
 * @param	swizzle	an additional permutation to apply to the components
 * 					of the register
 */
void GlesGenFetch(Compiler * compiler, SrcReg * reg, Expression * expr, const Swizzle * swizzle) {
	
	Type * type = expr->base.type;
	ProgVar * var;
	ExpressionReference * ref;
	ExtSwizzleOption extSwizzle[4];
	GLsizei elements;
	
	GLES_ASSERT(type->base.size == 1);
	
	reg->reference.base = NULL;
	reg->negate = GL_FALSE;
	reg->offset = 0;
	reg->index = NULL;
	reg->selectX = 0;
	reg->selectY = 1;
	reg->selectZ = 2;
	reg->selectW = 3;
	
	switch (expr->base.kind) {
	case ExpressionKindConstant:
		/* really the same as a reference */
		var = GlesCreateProgVarConst(compiler->generator.result, expr->constant.value, expr->base.type);
		reg->reference.constant = &var->constant; 		
		break;
		
	case ExpressionKindVectorConstructor:
		GLES_ASSERT(expr->base.type->base.size == 1);
		elements = expr->base.type->base.elements;
		
		ref = TrySimpleSwizzle(&expr->vectorConstructor);
		
		if (ref) {
			/* case 1: simple vector load with swizzle */
			GlesGenFetch(compiler, reg, (Expression *) ref, NULL);
			
			switch (elements) {
			case 4:	reg->selectW = expr->vectorConstructor.slots[3].fixedComponent->index;
			case 3: reg->selectZ = expr->vectorConstructor.slots[2].fixedComponent->index;
			case 2:	reg->selectY = expr->vectorConstructor.slots[1].fixedComponent->index;
			case 1:	reg->selectX = expr->vectorConstructor.slots[0].fixedComponent->index;
			default:
				;
			}
			
			break;
		} 

		ref = TryExtendedSwizzle(&expr->vectorConstructor, extSwizzle);
		
		if (ref) {
			/* case 2: all components are based on the same reference or are 0 or 1 */

			ExtSwizzleOption selectX, selectY, selectZ, selectW;
			Symbol * resultSymbol = GlesDeclareTempVariable(compiler, expr->base.type);
			Expression * resultExpr = GlesCreateExprReference(compiler, expr->base.type, resultSymbol, NULL, 0);
			DstReg tempReg;
						
			GlesGenFetch(compiler, reg, (Expression *) ref, NULL);
			
			if (swizzle) {
				selectX = extSwizzle[swizzle->selectX];
				selectY = extSwizzle[swizzle->selectY];
				selectZ = extSwizzle[swizzle->selectZ];
				selectW = extSwizzle[swizzle->selectW];				
			} else {
				selectX = extSwizzle[0];
				selectY = extSwizzle[1];
				selectZ = extSwizzle[2];
				selectW = extSwizzle[3];				
			}
			
			GlesGenStore(compiler, &tempReg, resultExpr, NULL);
			GlesGenInstSwizzle(&compiler->generator, OpcodeSWZ, expr->base.type->base.prec, 
							   &tempReg, reg->reference, selectX, selectY, selectZ, selectW);
							   
			GlesGenFetch(compiler, reg, resultExpr, NULL);			
			return;
		}

		GenVectorFetch(compiler, reg, &expr->vectorConstructor, swizzle);		
		break;
		
	case ExpressionKindFixedComponent:	/* fixed vector component	*/
		GlesGenFetch(compiler, reg, (Expression *) expr->fixedComponent.expr, NULL);
		reg->selectX = expr->fixedComponent.index;
		reg->selectY = expr->fixedComponent.index;
		reg->selectZ = expr->fixedComponent.index;
		reg->selectW = expr->fixedComponent.index;
		break; 
	
	case ExpressionKindRandomComponent:	/* indexed vector component	*/
		/* not implemented yet */
		GLES_ASSERT(GL_FALSE);
		break;
		
	case ExpressionKindReference:		/* reference of a variable 	*/
		if (expr->reference.index) {
			SrcReg regSrc;
			ProgVarAddr * addr = GlesCreateProgVarAddr(compiler->generator.result);
			
			GlesGenFetch(compiler, &regSrc, (Expression *) expr->reference.index, NULL);			
			GlesGenInstArl(&compiler->generator, OpcodeARL, addr, &regSrc);
			reg->index = addr;
		}

		switch (expr->reference.ref->base.qualifier) {
		case QualifierAttrib:
			GLES_ASSERT(compiler->shader->type == GL_VERTEX_SHADER);

			if (!expr->reference.ref->variable.progVar) {
				expr->reference.ref->variable.progVar =
					GlesCreateProgVarIn(compiler->generator.result, expr->base.type,
										expr->reference.ref->base.name,
										expr->reference.ref->base.length);
			}
			
			reg->reference.in = &expr->reference.ref->variable.progVar->in;
			break;
			
		case QualifierVarying:
			if (compiler->shader->type == GL_VERTEX_SHADER) {
				if (!expr->reference.ref->variable.progVar) {
					expr->reference.ref->variable.progVar =
						GlesCreateProgVarOut(compiler->generator.result, expr->base.type,
											 expr->reference.ref->base.name,
											 expr->reference.ref->base.length);
				}
				
				reg->reference.out = &expr->reference.ref->variable.progVar->out;
			} else if (compiler->shader->type == GL_FRAGMENT_SHADER) {
				if (!expr->reference.ref->variable.progVar) {
					expr->reference.ref->variable.progVar =
						GlesCreateProgVarIn(compiler->generator.result, expr->base.type,
											expr->reference.ref->base.name,
											expr->reference.ref->base.length);
				}
				
				reg->reference.in = &expr->reference.ref->variable.progVar->in;
			} else {
				GLES_ASSERT(GL_FALSE);
			}
			
			break;
			
		case QualifierVariable:		
		case QualifierParameterIn:
		case QualifierParameterOut:
		case QualifierParameterInOut:
			if (!expr->reference.ref->variable.progVar) {
				expr->reference.ref->variable.progVar =
					GlesCreateProgVarTemp(compiler->generator.result, expr->base.type);
			}
			
			reg->reference.temp = &expr->reference.ref->variable.progVar->temp;
			break;
			
		case QualifierUniform:
			if (!expr->reference.ref->variable.progVar) {
				expr->reference.ref->variable.progVar =
					GlesCreateProgVarParam(compiler->generator.result, 
										   expr->reference.ref->base.type,
										   expr->reference.ref->base.name,
										   expr->reference.ref->base.length);
			}
			
			reg->reference.param = &expr->reference.ref->variable.progVar->param;
			break;
		
		/*
		 * 7.1 Special objects in vertex shaders
		 */

		case QualifierPosition:						/* gl_Position variable			*/
		case QualifierPointSize:					/* gl_PointSize					*/
			GLES_ASSERT(compiler->shader->type == GL_VERTEX_SHADER);
		
			if (!expr->reference.ref->variable.progVar) {
				expr->reference.ref->variable.progVar =
					GlesCreateProgVarOut(compiler->generator.result, expr->base.type,
										 expr->reference.ref->base.name,
										 expr->reference.ref->base.length);
			}
			
			reg->reference.out = &expr->reference.ref->variable.progVar->out;
			break;
			
		/*
		 * 7.2 Special objects in fragment shaders
		 */
		 
		case QualifierFragCoord:					/* gl_FragCoord					*/
		case QualifierFrontFacing:					/* gl_FrontFacing				*/
		case QualifierPointCoord:					/* gl_PointCoord				*/
			GLES_ASSERT(compiler->shader->type == GL_FRAGMENT_SHADER);

			if (!expr->reference.ref->variable.progVar) {
				expr->reference.ref->variable.progVar =
					GlesCreateProgVarIn(compiler->generator.result, expr->base.type,
										expr->reference.ref->base.name,
										expr->reference.ref->base.length);
			}
			
			reg->reference.in = &expr->reference.ref->variable.progVar->in;
			break;
			
		case QualifierFragColor:					/* gl_FragColor					*/
		case QualifierFragData:						/* gl_FragData					*/
			GLES_ASSERT(compiler->shader->type == GL_FRAGMENT_SHADER);

			if (!expr->reference.ref->variable.progVar) {
				expr->reference.ref->variable.progVar =
					GlesCreateProgVarOut(compiler->generator.result, expr->base.type,
										 expr->reference.ref->base.name,
										 expr->reference.ref->base.length);
			}
			
			reg->reference.out = &expr->reference.ref->variable.progVar->out;
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
				
		reg->offset = expr->reference.offset;
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
	}
	
	if (swizzle) {
		/* apply the additional swizzle if provided */
		
		GLubyte select[4];
		
		select[0] = reg->selectX;
		select[1] = reg->selectY;
		select[2] = reg->selectZ;
		select[3] = reg->selectW;
		
		reg->selectX = select[swizzle->selectX];
		reg->selectY = select[swizzle->selectY];
		reg->selectZ = select[swizzle->selectZ];
		reg->selectW = select[swizzle->selectW];
	}
}

/**
 * Initialize the register structure to reference the given expression.
 * Any necessary permutation of the components is stored into the swizzle
 * argument to be used in calls to GlesGenFetch().
 * 
 * @param	reg		reference to register structure to initialize
 * @param	expr	reference to expression node to store into
 * @param	swizzle	out: reference to swizzle structure to receive permutation
 * 					to apply to source arguments used in instruction
 */
void GlesGenStore(Compiler * compiler, DstReg * reg, Expression * expr, Swizzle * swizzle) {
	Type * type = expr->base.type;
	GLsizei elements = type->base.elements, index;
	GLboolean mask[4];
	GLuint permutation[4];
	
	reg->reference.base = NULL;
	reg->offset = 0;
	reg->maskX = GL_FALSE;
	reg->maskY = GL_FALSE;
	reg->maskZ = GL_FALSE;
	reg->maskW = GL_FALSE;
	
	if (swizzle) {
		swizzle->selectX = 0;
		swizzle->selectY = 1;
		swizzle->selectZ = 2;
		swizzle->selectW = 3;
	}
		
	GLES_ASSERT(type->base.size == 1);
	
	switch (expr->base.kind) {
	case ExpressionKindRandomComponent:	/* indexed vector component	*/
		/* not implemented yet */
		GLES_ASSERT(GL_FALSE);
		break;
	
	case ExpressionKindFixedComponent:	/* fixed vector component	*/
		/* initialize swizzle and mask */
		GLES_ASSERT(swizzle);

		switch(expr->fixedComponent.index) {
		case 0:	reg->maskX = GL_TRUE; swizzle->selectX = 0; break;
		case 1: reg->maskY = GL_TRUE; swizzle->selectY = 0; break;
		case 2: reg->maskZ = GL_TRUE; swizzle->selectZ = 0; break;
		case 3: reg->maskW = GL_TRUE; swizzle->selectW = 0; break;
		default:
			GLES_ASSERT(GL_FALSE);
		}
				
		expr = (Expression *) expr->fixedComponent.expr;
		break;
	
	case ExpressionKindReference:		/* reference of a variable 	*/
		/* initialize swizzle and mask */
		
		switch (elements) {
		default: GLES_ASSERT(GL_FALSE);
			
		case 4: reg->maskW = GL_TRUE; /* fall through */
		case 3: reg->maskZ = GL_TRUE; /* fall through */
		case 2: reg->maskY = GL_TRUE; /* fall through */
		case 1: reg->maskX = GL_TRUE;
		}						
		
		break;
	
	case ExpressionKindVectorConstructor:	/* swizzled write		*/
		/* initialize swizzle and mask */

		for (index = 0; index < GLES_ELEMENTSOF(mask); ++index) {
			mask[index] = GL_FALSE;
			permutation[index] = 0;
		}
						
		for (index = 0; index < elements; ++index) {
			GLuint componentIndex = expr->vectorConstructor.slots[index].fixedComponent->index;
			
			GLES_ASSERT(!mask[componentIndex]);
			mask[componentIndex] = GL_TRUE;
			permutation[componentIndex] = index;
		}
		
		reg->maskX = mask[0];
		reg->maskY = mask[1];
		reg->maskZ = mask[2];
		reg->maskW = mask[3];
		
		swizzle->selectX = permutation[0];
		swizzle->selectY = permutation[1];
		swizzle->selectZ = permutation[2];
		swizzle->selectW = permutation[3];
		
		expr = (Expression *) expr->vectorConstructor.slots[0].fixedComponent->expr;
		break;
	
	default:
		GLES_ASSERT(GL_FALSE);
	}
	
	GLES_ASSERT(expr->base.kind == ExpressionKindReference);
	
	/* no dynamic indexing for non-uniforms, i.e. no write access */
	GLES_ASSERT(!expr->reference.index);

	switch (expr->reference.ref->base.qualifier) {
	case QualifierVarying:
		if (compiler->shader->type == GL_VERTEX_SHADER) {
			if (!expr->reference.ref->variable.progVar) {
				expr->reference.ref->variable.progVar =
					GlesCreateProgVarOut(compiler->generator.result, expr->base.type,
										 expr->reference.ref->base.name,
										 expr->reference.ref->base.length);
			}
			
			reg->reference.out = &expr->reference.ref->variable.progVar->out;
		} else {
			GLES_ASSERT(GL_FALSE);
		}
		
		break;
		
	case QualifierVariable:		
	case QualifierParameterIn:
	case QualifierParameterOut:
	case QualifierParameterInOut:
		if (!expr->reference.ref->variable.progVar) {
			expr->reference.ref->variable.progVar =
				GlesCreateProgVarTemp(compiler->generator.result, expr->base.type);
		}
		
		reg->reference.temp = &expr->reference.ref->variable.progVar->temp;
		break;
			
	/*
	 * 7.1 Special objects in vertex shaders
	 */

	case QualifierPosition:						/* gl_Position variable			*/
	case QualifierPointSize:					/* gl_PointSize					*/
		GLES_ASSERT(compiler->shader->type == GL_VERTEX_SHADER);
	
		if (!expr->reference.ref->variable.progVar) {
			expr->reference.ref->variable.progVar =
				GlesCreateProgVarOut(compiler->generator.result, expr->base.type,
								 	 expr->reference.ref->base.name,
									 expr->reference.ref->base.length);
		}
		
		reg->reference.out = &expr->reference.ref->variable.progVar->out;
		break;
		
	/*
	 * 7.2 Special objects in fragment shaders
	 */
	 
	case QualifierFragColor:					/* gl_FragColor					*/
	case QualifierFragData:						/* gl_FragData					*/
		GLES_ASSERT(compiler->shader->type == GL_FRAGMENT_SHADER);

		if (!expr->reference.ref->variable.progVar) {
			expr->reference.ref->variable.progVar =
				GlesCreateProgVarOut(compiler->generator.result, expr->base.type,
									 expr->reference.ref->base.name,
									 expr->reference.ref->base.length);
		}
		
		reg->reference.out = &expr->reference.ref->variable.progVar->out;
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
	}
			
	reg->offset = expr->reference.offset;
	
}

/**
 * Check if the expression node can be used as the target of an assignment.
 * 
 * @param	compiler	reference to compiler object
 * @param	expr		the expression node to test
 * 
 * @return	GL_TRUE if the expr is a valid left-hand operand of an assignment 
 * 			operator.
 */
static GLboolean IsLValue(Compiler * compiler, Expression * expr) {
	switch (expr->base.kind) {
	case ExpressionKindFixedComponent:
		return IsLValue(compiler, (Expression *) expr->fixedComponent.expr);
		
	case ExpressionKindVectorConstructor:
		return expr->vectorConstructor.assignable &&
			IsLValue(compiler, (Expression *) expr->vectorConstructor.slots[0].base);
			
	case ExpressionKindReference:
		switch (expr->reference.ref->base.qualifier) {
		case QualifierVarying:
			return (compiler->shader->type == GL_VERTEX_SHADER);
			
		case QualifierVariable:		
			return GL_TRUE;
			
		case QualifierParameterIn:
		case QualifierParameterOut:
		case QualifierParameterInOut:
			return !expr->reference.ref->parameter.constant;
				
		/*
		 * 7.1 Special objects in vertex shaders
		 */
	
		case QualifierPosition:						/* gl_Position variable			*/
		case QualifierPointSize:					/* gl_PointSize					*/
			return (compiler->shader->type == GL_VERTEX_SHADER);
			
		/*
		 * 7.2 Special objects in fragment shaders
		 */
		 
		case QualifierFragColor:					/* gl_FragColor					*/
		case QualifierFragData:						/* gl_FragData					*/
			return (compiler->shader->type == GL_FRAGMENT_SHADER);
			
		default:
			;
		}
		
	default:
		;
	}
	
	return GL_FALSE;
}

#if GLES_DEBUG

#include <stdio.h>

static void PrintIndent(GLuint indentLevel) {
	const char tabs[] = "\t\t\t\t\t\t\t\t";
	
	while (indentLevel > 8) {
		printf(tabs);
		indentLevel -= 8;
	}
	
	printf(tabs + 8 - indentLevel);
}

static void PrintName(const char * name, GLsizeiptr length) {
	while (length--) {
		putchar(*name++);
	}
}

static void PrintExpr(Expression * expr, GLuint indentLevel);
static void PrintFixedComponent(const ExpressionFixedComponent * fixed, GLuint indentLevel);
static void PrintReference(const ExpressionReference *reference, GLuint indentLevel);

static void PrintConstant(const ExpressionConstant * constant, GLuint indentLevel) {
	PrintIndent(indentLevel);
	printf("constant");
}

static void PrintVectorConstructor(const ExpressionVectorConstructor * vector, GLuint indentLevel) {
	Type * type = vector->base.type;
	GLsizei index;
	
	PrintIndent(indentLevel);
	printf("<\n");

	for (index = 0; index < type->base.elements; ++index) {
		PrintIndent(indentLevel + 1);
		printf("[%d]=\n", index);
		PrintExpr((Expression *) vector->slots[index].base, indentLevel + 2);
		putchar('\n');
	}
	
	PrintIndent(indentLevel);
	printf(">\n");
}

static void PrintStructConstructor(const ExpressionStructConstructor * structure, GLuint indentLevel) {
	Type * type = structure->base.type;
	GLsizei index;
	
	PrintIndent(indentLevel);
	printf("{\n");

	for (index = 0; index < type->structure.numFields; ++index) {
		PrintIndent(indentLevel + 1);
		PrintName(type->structure.fields[index].symbol->base.name,
			type->structure.fields[index].symbol->base.length);
		printf("=\n");
		PrintExpr((Expression *) structure->slots[index].base, indentLevel + 2);
	}
	
	PrintIndent(indentLevel);
	printf("}\n");
}

static void PrintFixedComponent(const ExpressionFixedComponent * fixed, GLuint indentLevel) {
	PrintReference(fixed->expr, indentLevel);
	printf("[%d]", fixed->index);
}

static void PrintRandomComponent(const ExpressionRandomComponent * random, GLuint indentLevel) {
	PrintReference(random->expr, indentLevel);
	putchar('[');
	PrintFixedComponent(random->index, 0);
	putchar(']');
}

static void PrintReference(const ExpressionReference *reference, GLuint indentLevel) {
	PrintIndent(indentLevel);
	PrintName(reference->ref->base.name, reference->ref->base.length);
	putchar(':');
	GlesTypePrint(reference->base.type);
}
	
static void PrintExpr(Expression * expr, GLuint indentLevel) {
	
	switch (expr->base.kind) {
	case ExpressionKindConstant:
		PrintConstant(&expr->constant, indentLevel);
		break;
	
	case ExpressionKindVectorConstructor:
		PrintVectorConstructor(&expr->vectorConstructor, indentLevel);
		break;

	case ExpressionKindStructConstructor:
		PrintStructConstructor(&expr->structConstructor, indentLevel);
		break;

	case ExpressionKindFixedComponent:	/* fixed vector component	*/
		PrintFixedComponent(&expr->fixedComponent, indentLevel);
		break;

	case ExpressionKindRandomComponent:	/* indexed vector component	*/
		PrintRandomComponent(&expr->randomComponent, indentLevel);
		break;

	case ExpressionKindReference:		/* reference of a variable 	*/
		PrintReference(&expr->reference, indentLevel);
		break;

	default:
		PrintIndent(indentLevel);
		printf("!%p!:", expr);
		GlesTypePrint(expr->base.type);
	}
}

void GlesPrintExpr(Expression * expr) {
	PrintExpr(expr, 0);
	putchar('\n');
}
#endif
