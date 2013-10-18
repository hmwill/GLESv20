#ifndef GLES_FRONTEND_STATEMENTS_H
#define GLES_FRONTEND_STATEMENTS_H 1

/*
** ==========================================================================
**
** $Id: statements.h 64 2007-09-22 20:23:16Z hmwill $			
** 
** Shading Language Front-End: Statement Processing
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-22 13:23:16 -0700 (Sat, 22 Sep 2007) $
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

#include "frontend/expressions.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

typedef enum ContinuationMask {
	ContinuationMaskNone	= 0,
	ContinuationMaskReturn	= 1,
	ContinuationMaskBreak	= 2,
	ContinuationMaskContinue= 4,
	ContinuationMaskDiscard = 8
} ContinuationMask;

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

typedef struct ForLoop {
	struct ForLoop *		outer;			/**< enclosing loop				*/
	union Symbol *			continueFlag;	/**< variable for transformation of continue */
} ForLoop;

/*
** --------------------------------------------------------------------------
** Functions - These are really just dispatcher to underlying vertex and
** fragment language code generators.
**
** For vertex language, all these functions get mapped to conditional 
** branches. For fragment language, they get mapped to structured 
** control flow instructions.
** --------------------------------------------------------------------------
*/

GLboolean GlesCreateStmntIf(Compiler * compiler, Expression * cond, GLboolean negate);
GLboolean GlesCreateStmntElse(Compiler * compiler);
GLboolean GlesCreateStmntEndif(Compiler * compiler);

GLboolean GlesCreateStmntDiscard(Compiler * compiler);
GLboolean GlesCreateStmntBreak(Compiler * compiler);
GLboolean GlesCreateStmntContinue(Compiler * compiler);
GLboolean GlesCreateStmntReturn(Compiler * compiler, Expression * cond);

GLboolean GlesCreateStmntFor(Compiler * compiler, union Symbol * loopIndex, 
							 Expression * initial, Expression * boundary, Expression * increment,
							 Cond condition);
GLboolean GlesCreateStmntEndFor(Compiler * compiler);
GLboolean GlesCreateStmntWhile(Compiler * compiler, Expression * cond);
GLboolean GlesCreateStmntEndWhile(Compiler * compiler);
GLboolean GlesCreateStmntDo(Compiler * compiler);
GLboolean GlesCreateStmntEndDo(Compiler * compiler, Expression * cond);

#endif /* GLES_FRONTEND_STATEMENTS_H */