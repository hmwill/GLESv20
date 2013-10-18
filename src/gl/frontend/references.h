#ifndef GLES_FRONTEND_REFERENCES_H
#define GLES_FRONTEND_REFERENCES_H 1

/*
** ==========================================================================
**
** $Id: references.h 60 2007-09-18 01:16:07Z hmwill $			
** 
** Shading Language Operand References
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

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

typedef enum RefType {
	RefTypeSymbol,						/* reference to a symbol			*/
	RefTypeField,						/* reference to a structure field	*/
	RefTypeIndex						/* indexing an array by a value		*/
	RefTypeConstIndex					/* indexing an array by a const		*/
	RefTypeConstant						/* reference to a constant			*/
} RefType;

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

typedef union Reference Reference;
typedef union Symbol Symbol;
typedef union Type type;
typedef struct Field Field;
typedef union Constant Constant;

typedef struct ReferenceBase {
	RefType			refType;			/* type of the reference node		*/
} ReferenceBase;

typedef struct ReferenceSymbol {
	ReferenceBase	base;
	Symbol *		symbol;
} ReferenceSymbol;

typedef struct ReferenceField {
	ReferenceBase	base;
	Reference *		reference;
	Field *			field;
} ReferenceField;

typedef struct ReferenceIndex {
	ReferenceBase	base;
	Reference *		reference;
	Symbol *		index;
} ReferenceIndex;

typedef struct ReferenceConstIndex {
	ReferenceBase	base;
	Reference *		reference;
	GLsizei			index;
} ReferenceIndex;

typedef struct ReferenceConstant {
	ReferenceBase	base;
	Type *			type;
	Constant *		constant;
} ReferenceConstant;

union Reference {
	ReferenceBase		base;
	ReferenceSymbol		symbol;
	ReferenceField		field;
	ReferenceIndex		index;
	ReferenceConstIndex	constIndex;
	ReferenceConstant	constant;
};
 
/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

#endif /* GLES_FRONTEND_REFERENCES_H */