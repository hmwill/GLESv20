/*
** ==========================================================================
**
** $Id: il.c 71 2007-10-01 04:50:41Z hmwill $			
** 
** Inline Assembler
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-30 21:50:41 -0700 (Sun, 30 Sep 2007) $
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
#include "frontend/il.h"
#include "frontend/types.h"
#include "frontend/symbols.h"
#include "frontend/memory.h"
#include "frontend/compiler.h"
#include "frontend/expressions.h"
#include "frontend/asm.h"

GLboolean GlesAsmUnary(Compiler * compiler, Opcode opcode, 
	Expression * result, Expression * operand) {
	SrcReg operandReg;
	DstReg resultReg;
	
	// TODO: Validate arguments
	
	GlesGenFetch(compiler, &operandReg, operand, NULL);
		
	GlesGenStore(compiler, &resultReg, result, NULL);
	GlesGenInstUnary(&compiler->generator, opcode, result->base.type->base.prec, 
					 &resultReg, &operandReg);
					
	return GL_TRUE;
}
	
GLboolean GlesAsmBinary(Compiler * compiler, Opcode opcode, 
	Expression * result, Expression * left, Expression * right) {
	SrcReg leftReg, rightReg;
	DstReg resultReg;
	
	// TODO: Validate arguments

	GlesGenFetch(compiler, &leftReg, left, NULL);
	GlesGenFetch(compiler, &rightReg, right, NULL);
		
	GlesGenStore(compiler, &resultReg, result, NULL);
	GlesGenInstBinary(&compiler->generator, opcode, result->base.type->base.prec, 
					  &resultReg, &leftReg, &rightReg);
					
	return GL_TRUE;
}

GLboolean GlesAsmTernary(Compiler * compiler, Opcode opcode, 
	Expression * result, Expression * first, Expression * second, 
	Expression * third) {
	SrcReg firstReg, secondReg, thirdReg;
	DstReg resultReg;
	
	// TODO: Validate arguments

	GlesGenFetch(compiler, &firstReg, first, NULL);
	GlesGenFetch(compiler, &secondReg, second, NULL);
	GlesGenFetch(compiler, &thirdReg, third, NULL);
		
	GlesGenStore(compiler, &resultReg, result, NULL);
	GlesGenInstTernary(&compiler->generator, opcode, result->base.type->base.prec, 
					   &resultReg, &firstReg, &secondReg, &thirdReg);
					
	return GL_TRUE;
}

GLboolean GlesAsmTexture(Compiler * compiler, Opcode opcode, 
	Expression * result, Expression * coords, Expression * sampler) {
	
	SrcReg coordReg;
	DstReg resultReg;
	TextureTarget target;
	ProgVar * samplerVar;
	GLsizeiptr offset;

	/* Validate sampler */
	switch (sampler->base.type->base.kind) {
		case TypeSampler2D: 	target = TextureTarget2D; break;
		case TypeSampler3D: 	target = TextureTarget3D; break;
		case TypeSamplerCube:	target = TextureTargetCube; break;
		
		default: 
			GlesCompileError(compiler, ErrS0001);
			return GL_FALSE;
	}

	GLES_ASSERT(sampler->base.kind == ExpressionKindReference);
	samplerVar = sampler->reference.ref->variable.progVar;
	offset = sampler->reference.offset;
	
	// TODO: Validate arguments
	
	GlesGenFetch(compiler, &coordReg, coords, NULL);
	GlesGenStore(compiler, &resultReg, result, NULL);

	GlesGenInstTex(&compiler->generator, opcode, result->base.type->base.prec,
					target, &resultReg, &coordReg, samplerVar, offset);
		
	return GL_TRUE;
}