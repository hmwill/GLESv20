#ifndef GLES_FRONTEND_TOKENIZER_H
#define GLES_FRONTEND_TOKENIZER_H 1

/*
** ==========================================================================
**
** $Id: tokenizer.h 63 2007-09-19 05:16:07Z hmwill $			
** 
** Shading Language Front-End: Lexiacal Processing
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-18 22:16:07 -0700 (Tue, 18 Sep 2007) $
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

#include "gl/state.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

typedef enum TokenType {
	TokenTypeEof, TokenTypeEol, TokenTypeSpace,
	TokenTypeError,
	
	TokenTypeIdentifier, 
	TokenTypeFloatConstant, TokenTypeIntConstant, 
	TokenTypeLeftOp, TokenTypeRightOp,
	TokenTypeIncOp, TokenTypeDecOp, 
	TokenTypeLessEqualOp, TokenTypeGreaterEqualOp,
	TokenTypeEqualOp, TokenTypeNotEqualOp,
	TokenTypeAndOp, TokenTypeOrOp, TokenTypeXorOp,
	TokenTypeMulAssign, TokenTypeDivAssign, TokenTypeAddAssign,
	TokenTypeModAssign, TokenTypeLeftAssign, TokenTypeRightAssign,
	TokenTypeAndAssign, TokenTypeXorAssign, TokenTypeOrAssign,
	TokenTypeSubAssign,	
	TokenTypeLeftParen, TokenTypeRightParen, 
	TokenTypeLeftBracket, TokenTypeRightBracket,
	TokenTypeLeftBrace, TokenTypeRightBrace, TokenTypeDot,
	TokenTypeComma, TokenTypeColon, TokenTypeEqual, TokenTypeSemicolon,
	TokenTypeBang, TokenTypeDash, TokenTypeTilde, TokenTypePlus,
	TokenTypeStar, TokenTypeSlash, TokenTypePercent,
	TokenTypeLeftAngle, TokenTypeRightAngle, TokenTypeVerticalBar,
	TokenTypeCaret, TokenTypeAmpersand, TokenTypeQuestion,

	/* Pre-processor Tokens */
	TokenTypeHash,
	
	/* Shading Language Tokens */
	TokenTypeAttribute,
	TokenTypeConst,
	TokenTypeBool,
	TokenTypeFloat,
	TokenTypeInt,
	
	TokenTypeTrue,
	TokenTypeFalse,
	 
	TokenTypeBreak,
	TokenTypeContinue, 
	TokenTypeDo,
	TokenTypeElse,
	TokenTypeFor,
	TokenTypeIf,
	TokenTypeDiscard,
	TokenTypeReturn,
	 
	TokenTypeBoolVec2, TokenTypeBoolVec3, TokenTypeBoolVec4,
	TokenTypeIntVec2, TokenTypeIntVec3, TokenTypeIntVec4,
	TokenTypeFloatVec2, TokenTypeFloatVec3, TokenTypeFloatVec4,
	TokenTypeFloatMat2, TokenTypeFloatMat3, TokenTypeFloatMat4,
	
	TokenTypeIn,
	TokenTypeOut,
	TokenTypeInOut,
	
	TokenTypeUniform, TokenTypeVarying,
	TokenTypeSampler2D, TokenTypeSampler3D, TokenTypeSamplerCube,
	TokenTypeStruct, TokenTypeVoid, TokenTypeWhile,

	TokenTypeInvariant, TokenTypePrecision,
	TokenTypeHighPrecision, TokenTypeMediumPrecision, TokenTypeLowPrecision,
	
	/* Inline Assembly Language Tokens */	
	TokenTypeAsmRetval,
	
	TokenTypeAsmARL,  	
	TokenTypeAsmABS,		TokenTypeAsmABS_SAT,	
	TokenTypeAsmADD,		TokenTypeAsmADD_SAT,	
	TokenTypeAsmCAL,	
	TokenTypeAsmCMP,		TokenTypeAsmCMP_SAT,	
	TokenTypeAsmCOS,		TokenTypeAsmCOS_SAT,	
	TokenTypeAsmDP2,		TokenTypeAsmDP2_SAT,	
	TokenTypeAsmDP3,		TokenTypeAsmDP3_SAT,
	TokenTypeAsmDP4,		TokenTypeAsmDP4_SAT,	
	TokenTypeAsmDPH,		TokenTypeAsmDPH_SAT,	
	TokenTypeAsmDST,		TokenTypeAsmDST_SAT,
	TokenTypeAsmEX2,		TokenTypeAsmEX2_SAT,	
	TokenTypeAsmEXP,		TokenTypeAsmEXP_SAT,	
	TokenTypeAsmFLR,		TokenTypeAsmFLR_SAT,
	TokenTypeAsmFRC,		TokenTypeAsmFRC_SAT,	
	TokenTypeAsmLG2,		TokenTypeAsmLG2_SAT,
	TokenTypeAsmLOG,		TokenTypeAsmLOG_SAT,	
	TokenTypeAsmLRP,		TokenTypeAsmLRP_SAT,	
	TokenTypeAsmMAD,		TokenTypeAsmMAD_SAT,
	TokenTypeAsmMAX,		TokenTypeAsmMAX_SAT,	
	TokenTypeAsmMIN,		TokenTypeAsmMIN_SAT,	
	TokenTypeAsmMOV,		TokenTypeAsmMOV_SAT,
	TokenTypeAsmMUL,		TokenTypeAsmMUL_SAT,	
	TokenTypeAsmPOW,
	TokenTypeAsmRCP,		TokenTypeAsmRCP_SAT,	
	TokenTypeAsmRET,
	TokenTypeAsmRSQ,		TokenTypeAsmRSQ_SAT,		
	TokenTypeAsmSCS,		TokenTypeAsmSCS_SAT,
	TokenTypeAsmSEQ,    
	TokenTypeAsmSFL,  	
	TokenTypeAsmSGE,
    TokenTypeAsmSGT,	
	TokenTypeAsmSIN,    	TokenTypeAsmSIN_SAT,    
	TokenTypeAsmSLE,
    TokenTypeAsmSLT,    
	TokenTypeAsmSNE,    
	TokenTypeAsmSSG,
    TokenTypeAsmSTR,	
	TokenTypeAsmSUB,		TokenTypeAsmSUB_SAT,	
	TokenTypeAsmSWZ,		TokenTypeAsmSWZ_SAT,
	TokenTypeAsmTEX,		TokenTypeAsmTEX_SAT,	
	TokenTypeAsmTXB,		TokenTypeAsmTXB_SAT,	
	TokenTypeAsmTXL,		TokenTypeAsmTXL_SAT,
	TokenTypeAsmTXP,		TokenTypeAsmTXP_SAT,	
	TokenTypeAsmXPD,		TokenTypeAsmXPD_SAT,
	
} TokenType;

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

struct MemoryPool;
struct Compiler;

/**
 * Structure used to represent non-zero-terminated strings.
 */
typedef struct TokenString {
	char *					first;			/**< pointer to LA character */
	GLsizeiptr				length;			/**< number of characters */
} TokenString;

/**
 * Structure used to represent macro definitions.
 */
typedef struct Macro {
	struct Macro *			next;			/**< next definition in list */
	TokenString				name;			/**< macro name */
	TokenString 			text;			/**< replacement text */
	GLsizei					numParameters;	/**< ~0 for no arguments */
	GLboolean				disabled;		/**< macro is currently disabled */
	TokenString				parameters[0];	/**< parameter names */
} Macro;

typedef struct Conditional {
	GLboolean				elsePart;		/**< have reached else part		*/
	GLboolean				isTrue;			/**< current part is true		*/
	GLboolean				wasTrue;		/**< no further branch			*/
} Conditional;
	
/**
 * Structure used to represent look-ahead token values.
 */
typedef struct Token {
	TokenType		tokenType;				/**< token type					*/
	TokenString		s;						/**< string boundaries			*/
	
	union {									/**< associated value			*/
		GLfloat		f;
		GLboolean	b;
		GLint		i;
	} value;
} Token;

/**
 * Structure to represent an entry in the macro-expansion stack
 */	
typedef struct Expansion {
	char *					current;		/* pointer to LA character */
	char *					last;			/* pointer to last character */
	Macro *					macro;			/* the macro being expanded */
	char *					freepointer;	/* memory to deallocate after use */
} Expansion;
	
typedef struct InputState {
	char *					current;		/* pointer to LA character */
	char *					last;			/* pointer to last character */
	char *					freepointer;	/* memory to deallocate after use */
	GLsizei					sp;				/* stack pointer into expansion stack */
	Expansion				expansions[GLES_MAX_PREPROC_MACRO_DEPTH];	/* expansion stack */	
} InputState;
	
typedef struct Tokenizer {
	struct Compiler *		compiler;		/* reference to parent */
	Log *					log;			/* tokenizer and preprocessor log */
	Log 					macroExpand;	/* macro expansion buffer */
	Shader * 				shader;			/* the shader input source		*/
	struct MemoryPool * 	tempMemory;
	
	InputState				input;			/* input for tokenizer */
	
	GLsizei					lineno;			/* current line number */
	GLsizei					sourceno;		/* current source string number */
	GLboolean				inComment;		/* currently parsing a multi-line comment */
	GLboolean				beginLine;		/* currently at beginning of line */
	GLboolean				hadResult;		/* set to true if first token returned to parser */
							  
	Macro * 				lineMacro;		/* pointer to __LINE__ macro */
	Macro *					fileMacro;		/* pointer to __FILE__ macro */
	
	char					lineMacroText[16];	/* text for __LINE__ macro */
	char					fileMacroText[16];	/* text for __FILE__ macro */
	
	GLsizei					nest;			/* conditional stack		*/
	Conditional				conditionals[GLES_MAX_PREPROC_COND_DEPTH];
	
	Macro *					macroHash[GLES_PREPROC_SYMBOL_HASH];
	
	Token 					token;			/* look-ahead token			*/
} Tokenizer;

/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

Tokenizer * GlesCreateTokenizer(Log * log, struct Compiler * compiler);
void GlesDestroyTokenizer(Tokenizer * tokenizer);

void GlesPrepareTokenizer(Tokenizer * tokenizer,
						  Shader * shader, struct MemoryPool * tempMemory,
						  GLsizei numStrings, const char *initStrings[]);
void GlesCleanupTokenizer(Tokenizer * tokenizer);
GLboolean GlesNextToken(Tokenizer * tokenizer);
 
#endif /* GLES_FRONTEND_TOKENIZER_H */