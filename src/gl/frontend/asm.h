#ifndef GLES_FRONTEND_ASM_H
#define GLES_FRONTEND_ASM_H 1

/*
** ==========================================================================
**
** $Id: il.h 76 2007-10-20 04:34:44Z hmwill $			
** 
** Inline Assembler
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-10-19 21:34:44 -0700 (Fri, 19 Oct 2007) $
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

#include "frontend/constants.h"
#include "frontend/types.h"
#include "frontend/memory.h"


GLboolean GlesAsmUnary(Compiler * compiler, Opcode opcode, 
	Expression * result, Expression * operand);
	
GLboolean GlesAsmBinary(Compiler * compiler, Opcode opcode, 
	Expression * result, Expression * left, Expression * right);

GLboolean GlesAsmTernary(Compiler * compiler, Opcode opcode, 
	Expression * result, Expression * first, Expression * second, 
	Expression * third);

GLboolean GlesAsmTexture(Compiler * compiler, Opcode opcode, 
	Expression * result, Expression * coords, Expression * sampler);

#endif /* ndef GLES_FRONTEND_ASM_H */
