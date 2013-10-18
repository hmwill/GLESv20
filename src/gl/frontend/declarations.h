#ifndef GLES_FRONTEND_DECLARATIONS_H
#define GLES_FRONTEND_DECLARATIONS_H 1

/*
** ==========================================================================
**
** $Id: declarations.h 60 2007-09-18 01:16:07Z hmwill $			
** 
** Shading Language Front-End: Declaration Processing
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
#include "frontend/symbols.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/


/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

union Expression;

/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

GLboolean GlesDeclareStructField(Compiler * compiler, Type * structure, 
						  		 const char * name, GLsizei length,
					  	  		 Type * type);

GLboolean GlesDeclareStruct(Compiler * compiler, 
						  	const char * name, GLsizei length, Type * type);
						  	
GLboolean GlesDeclareVariable(Compiler * compiler, 
						  	  const char * name, GLsizei length, 
						  	  Qualifier qualifier, Type * type, GLboolean invariant,
						  	  union Expression * initializer);

GLboolean GlesDeclareParameter(Compiler * compiler, 
						  	  const char * name, GLsizei length, 
						  	  Qualifier qualifier, Type * type, GLsizei index,
							  GLboolean constant);
						  	  
Symbol * GlesDeclareTempVariable(Compiler * compiler, Type * type);
						  	   
GLboolean GlesDeclareInvariant(Compiler * compiler, 
							   const char * name, GLsizei length);
							   						  	   
Symbol * GlesDeclareFunction(Compiler * compiler,
							 const char * name, GLsizei length, 
							 Type * returnType, 
							 GLsizei numParameters, Scope * parameters);
							  
GLboolean GlesFinishStructDeclaration(Compiler * compiler, Type * type);

Symbol * GlesSymbolFindExpansion(Compiler * compiler, 
								 Symbol * base, SymbolField * field);

#endif /* GLES_FRONTEND_DECLARATIONS_H */