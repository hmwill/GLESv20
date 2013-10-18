/*
** ==========================================================================
**
** $Id: statements.c 62 2007-09-18 23:24:32Z hmwill $			
** 
** Shading Language Front-End: Statement Processing
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-18 16:24:32 -0700 (Tue, 18 Sep 2007) $
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
#include "frontend/declarations.h"
#include "frontend/statements.h"
#include "frontend/expressions.h"
#include "frontend/compiler.h"
#include "frontend/il.h"
#include "frontend/types.h"
#include "frontend/symbols.h"
#include "frontend/memory.h"

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

/**
 * Open an IF block in the generated instruction stream.
 * 
 * @param	compiler	reference to compiler object
 * @param	cond		reference to condition
 * @param	negate		if set to GL_TRUE, negate condition before evaluating
 * 
 * @return	GL_TRUE if generation of instruction was successful
 */
GLboolean GlesCreateStmntIf(Compiler * compiler, Expression * cond, GLboolean negate) {
	
	SrcReg	regCond;
	
	GlesGenFetch(compiler, &regCond, cond, NULL);
	
	GlesGenInstSrc(&compiler->generator, OpcodeSCC, &regCond);
	GlesGenInstCond(&compiler->generator, OpcodeIF, negate ? CondEQ : CondNE, 0, 0, 0, 0);
	GlesCreateBlock(&compiler->generator);
	
	return GL_TRUE;
}

/**
 * Generate an ELSE marker in the resulting instruction stream.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if generation of instruction was successful
 */
GLboolean GlesCreateStmntElse(Compiler * compiler) {
	GlesGenInstBase(&compiler->generator, OpcodeELSE);
	GlesCreateBlock(&compiler->generator);
	
	return GL_TRUE;
}

/**
 * Generate an ENDIF marker in the resulting instruction stream.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if generation of instruction was successful
 */
GLboolean GlesCreateStmntEndif(Compiler * compiler) {
	GlesGenInstBase(&compiler->generator, OpcodeENDIF);
	GlesCreateBlock(&compiler->generator);
	
	return GL_TRUE;
}

/**
 * Generate a KIL instruction in the resulting instruction stream. This
 * instruction is only valid in fragment shaders.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if generation of instruction was successful
 */
GLboolean GlesCreateStmntDiscard(Compiler * compiler) {
	if (compiler->shader->type != GL_FRAGMENT_SHADER) {
		/* "Syntax error" */
		GlesCompileError(compiler, ErrL0001);
		return GL_FALSE;
	}
	
	GlesGenInstCond(&compiler->generator, OpcodeKIL, CondT, 0, 0, 0, 0);
	return GL_TRUE;
}

/**
 * Generate a BRK instruction in the resulting instruction stream. 
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_TRUE if generation of instruction was successful
 */
GLboolean GlesCreateStmntBreak(Compiler * compiler) {
	if (!compiler->currentLoop) {
		/* "Syntax error" */
		GlesCompileError(compiler, ErrL0001);
		return GL_FALSE;
	}
	
	GlesGenInstCond(&compiler->generator, OpcodeBRK, CondT, 0, 0, 0, 0);
	return GL_TRUE;
}

/**
 * Continue is currently not supported.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_FALSE to indicate failure
 */
GLboolean GlesCreateStmntContinue(Compiler * compiler) {
	Expression * flagExpr;
	 
	if (!compiler->currentLoop) {
		/* "syntax error": continue used outside of loop */
		GlesCompileError(compiler, ErrL0001);
		return GL_FALSE;
	}
	
	flagExpr = 
		GlesCreateExprReference(compiler, compiler->currentLoop->continueFlag->base.type,
			compiler->currentLoop->continueFlag, NULL, 0);

	return 
		NULL !=
			GlesCreateExprAssign(compiler, flagExpr, GlesCreateExprBoolConstant(compiler, GL_TRUE));
}

/**
 * Generate a RET instruction in the resulting instruction stream. 
 * 
 * For functions with a return type of void, the return value has to be NULL.
 * For functions with a non-void return type, the type of the return value
 * has to match the return type.
 * 
 * @param	compiler	reference to compiler object
 * @param	expr		reference to result value; can be NULL
 * 
 * @return 	GL_TRUE if generation of return instruction sequence was
 * 			successful, otherwise GL_FALSE.
 */
GLboolean GlesCreateStmntReturn(Compiler * compiler, Expression * expr) {
	
	GLES_ASSERT(compiler->currentFunction);
	GLES_ASSERT(compiler->currentFunction->base.type->base.kind == TypeFunction);
	
	if (compiler->currentFunction->base.type->func.returnType->base.kind != TypeVoid) {
		/* need a result */
		Expression * resultExpr;
		
		if (!expr) {
			/* Function declared with a return value but return statement has no argument. */
			GlesCompileError(compiler, ErrS0038);
			return GL_FALSE;
		}
		
		if (!GlesTypeMatches(expr->base.type, compiler->currentFunction->base.type->func.returnType)) {
			/* Return type of function definition must match return type of function declaration. */
			GlesCompileError(compiler, ErrS0042);
			return GL_FALSE;
		}
		
		resultExpr = 
			GlesCreateExprReference(compiler, 
									compiler->currentFunction->base.type->func.returnType,
									compiler->currentFunction->result,
									NULL, 0);
									
		/* assign result value to result variable */
		GlesCreateExprAssign(compiler, resultExpr, expr);				
	} else if (expr) {
		/* Function declared void but return statement has an argument */
		GlesCompileError(compiler, ErrS0039);
		return GL_FALSE;
	}

	GlesGenInstCond(&compiler->generator, OpcodeRET, CondT, 0, 0, 0, 0);	 
	return GL_TRUE;
}

/**
 * Calculate the number of iterations, integer case.
 * 
 * @param	initial			reference to the initial value of the loop index variable
 * @param	boundary		reference to the boundary value for the loop index
 * @param	increment		reference to the increment value for the loop index
 * @param	condition		the condition operation used in the loop condition clause
 * 
 * @return	the number of iterations, or ~0 if the loop is not valid.
 */
static GLint CalcNumIterationsInt(GLint initial, GLint boundary, GLint increment,
							   	  Cond condition) {

	GLint diff;
	
	/* reduce > and >= to < and <= */ 
	if (condition == CondGT) {
		condition = CondLT;
	} else if (condition == CondGE) {
		initial = -initial;
		boundary = -boundary;
		increment = -increment;
		condition = CondLE;
	} 
	
	switch (condition) {
	default:
		return ~0;
		
	case CondGT:
		initial = -initial;
		boundary = -boundary;
		increment = -increment;
		/* fall through */
		
	case CondLT:
		if (initial >= boundary) {
			return 0;
		} else if (increment <= 0) {
			return ~0;
		}
			
		diff = boundary - initial;
		return diff / increment;
		break;
		
	case CondGE:
		initial = -initial;
		boundary = -boundary;
		increment = -increment;
		/* fall through */
		
	case CondLE:
		if (initial > boundary) {
			return 0;
		} else if (increment <= 0) {
			return ~0;
		}
		
		diff = boundary - initial;
		return (diff + 1) / increment;
		
	case CondEQ:
		if (initial == boundary) {
			if (increment != 0) {
				return 1;
			} else {
				return ~0;
			}
		} else {
			return 0;
		}
		
	case CondNE:
		if (initial == boundary) {
			return 0;
		}
		
		if (initial > boundary) {
			initial = -initial;
			boundary = -boundary;
			increment = -increment;
		}
		
		if (increment <= 0) {
			return ~0;
		}

		diff = boundary - initial;
		
		if (diff % increment) {
			return ~0;
		}
		
		return diff / increment;		
	}
}

/**
 * Calculate the number of iterations, floating point case.
 * 
 * @param	initial			reference to the initial value of the loop index variable
 * @param	boundary		reference to the boundary value for the loop index
 * @param	increment		reference to the increment value for the loop index
 * @param	condition		the condition operation used in the loop condition clause
 * 
 * @return	the number of iterations, or ~0 if the loop is not valid.
 */
static GLint CalcNumIterationsFloat(GLfloat initial, GLfloat boundary, GLfloat increment,
							   	  	Cond condition) {
	GLfloat diff;
	
	/* reduce > and >= to < and <= */ 
	if (condition == CondGT) {
		condition = CondLT;
	} else if (condition == CondGE) {
		initial = -initial;
		boundary = -boundary;
		increment = -increment;
		condition = CondLE;
	} 
	
	switch (condition) {
	default:
		return ~0;
		
	case CondGT:
		initial = -initial;
		boundary = -boundary;
		increment = -increment;
		/* fall through */
		
	case CondLT:
		if (initial >= boundary) {
			return 0;
		} else if (increment <= 0) {
			return ~0;
		}
			
		diff = boundary - initial;
		return diff / increment;
		break;
		
	case CondGE:
		initial = -initial;
		boundary = -boundary;
		increment = -increment;
		/* fall through */
		
	case CondLE:
		if (initial > boundary) {
			return 0;
		} else if (increment <= 0) {
			return ~0;
		}
		
		diff = boundary - initial;
		return (diff + 1) / increment;		
	}
}

/**
 * Calculate the number of iterations.
 * 
 * @param	initial			reference to the initial value of the loop index variable
 * @param	boundary		reference to the boundary value for the loop index
 * @param	increment		reference to the increment value for the loop index
 * @param	condition		the condition operation used in the loop condition clause
 * 
 * @return	the number of iterations, or ~0 if the loop is not valid.
 */
static GLint CalcNumIterations(Expression * initial, Expression * boundary, Expression * increment,
							   Cond condition) {
	if (initial->base.type->base.kind == TypeInt) {
		return CalcNumIterationsInt(initial->constant.value[0].intValue[0],	
									boundary->constant.value[0].intValue[0],
									increment->constant.value[0].intValue[0],
									condition);
	} else {
		GLES_ASSERT(initial->base.type->base.kind == TypeFloat);
		
		return CalcNumIterationsFloat(initial->constant.value[0].floatValue[0],	
								 	  boundary->constant.value[0].floatValue[0],
									  increment->constant.value[0].floatValue[0],
									  condition);
	}
}

/**
 * Begin a for loop with a constant number of iterations
 * 
 * @param	compiler		reference to compiler object
 * @param	loopIndex		reference to the loop index variable
 * @param	initial			reference to the initial value of the loop index variable
 * @param	boundary		reference to the boundary value for the loop index
 * @param	increment		reference to the increment value for the loop index
 * @param	condition		the condition operation used in the loop condition clause
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
GLboolean GlesCreateStmntFor(Compiler * compiler, union Symbol * loopIndex, 
							 Expression * initial, Expression * boundary, Expression * increment,
							 Cond condition) {
	GLint numIterations; 
	SrcReg regSrc;
	ForLoop * forLoop;

	if (loopIndex == NULL ||
		loopIndex->base.type->base.kind != TypeInt &&
		loopIndex->base.type->base.kind != TypeFloat) {
		/* loop index must be of type int or float	*/
		GlesCompileError(compiler, ErrX0004);
		return GL_FALSE;
	}
	
	if (initial == NULL ||
		initial->base.type->base.kind != loopIndex->base.type->base.kind ||
		initial->base.kind != ExpressionKindConstant) {
		/* loop index variable must be initialized... */
		GlesCompileError(compiler, ErrX0006);
		return GL_FALSE;
	}
	
	if (boundary == NULL													||
		boundary->base.type->base.kind != loopIndex->base.type->base.kind 	||
		boundary->base.kind != ExpressionKindConstant					   	||
		increment == NULL												   	||
		increment->base.type->base.kind != loopIndex->base.type->base.kind 	||
		increment->base.kind != ExpressionKindConstant) {
		/* loop index variable must be incremented... */
		GlesCompileError(compiler, ErrX0007);
		return GL_FALSE;
	}

	numIterations = CalcNumIterations(initial, boundary, increment, condition);
	
	if (numIterations == ~0 || numIterations == 0) {
		/* loop must be properly bounded... */
		GlesCompileError(compiler, ErrX0008);
		return GL_FALSE;
	}
	
	GlesGenFetch(compiler, &regSrc, GlesCreateExprIntConstant(compiler, numIterations), NULL);
	GlesGenInstSrc(&compiler->generator, OpcodeREP, &regSrc);
	
	forLoop = GlesMemoryPoolAllocate(compiler->moduleMemory, sizeof(ForLoop));
	forLoop->outer = compiler->currentLoop;
	compiler->currentLoop = forLoop;
	
	forLoop->continueFlag = GlesDeclareTempVariable(compiler, GlesBasicType(TypeBool, PrecisionUndefined));
	
	/* initialize the continue flag for this loop with GL_FALSE at the beginning of the body */
	GlesCreateExprAssign(compiler,
						 GlesCreateExprReference(compiler, forLoop->continueFlag->base.type,
						 						 forLoop->continueFlag, NULL, 0),
						 GlesCreateExprBoolConstant(compiler, GL_FALSE));
	
	return GL_TRUE;
}

/**
 * Terminate a for loop
 * 
 * @param	compiler		reference to compiler object
 * 
 * @return	GL_TRUE if successful, otherwise GL_FALSE
 */
GLboolean GlesCreateStmntEndFor(Compiler * compiler) {
	GLES_ASSERT(compiler->currentLoop);
	
	compiler->currentLoop = compiler->currentLoop->outer;
	GlesGenInstBase(&compiler->generator, OpcodeENDREP);
	
	return GL_TRUE;
}

/**
 * while is currently not supported.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_FALSE to indicate failure
 */
GLboolean GlesCreateStmntWhile(Compiler * compiler, Expression * cond) {
	/* while and do-while loops not supported in this version */
	GlesCompileError(compiler, ErrX0001);
	return GL_FALSE;
}

/**
 * while is currently not supported.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_FALSE to indicate failure
 */
GLboolean GlesCreateStmntEndWhile(Compiler * compiler) {
	GLES_ASSERT(GL_FALSE);
	return GL_FALSE;
}

/**
 * do-while is currently not supported.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_FALSE to indicate failure
 */
GLboolean GlesCreateStmntDo(Compiler * compiler) {
	/* while and do-while loops not supported in this version */
	GlesCompileError(compiler, ErrX0001);
	return GL_FALSE;
}

/**
 * do-while is currently not supported.
 * 
 * @param	compiler	reference to compiler object
 * 
 * @return	GL_FALSE to indicate failure
 */
GLboolean GlesCreateStmntEndDo(Compiler * compiler, Expression * cond) {
	GLES_ASSERT(GL_FALSE);
	return GL_FALSE;
}

