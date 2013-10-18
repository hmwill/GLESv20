#ifndef GLES_FRONTEND_IL_H
#define GLES_FRONTEND_IL_H 1

/*
** ==========================================================================
**
** $Id: il.h 76 2007-10-20 04:34:44Z hmwill $			
** 
** Intermediate Shader Language
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

#if GLES_VERTEX_SHADER_BRANCH || GLES_FRAGMENT_SHADER_BRANCH
#error General branching not support in this version
#endif

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

typedef enum InstKind {
	InstKindBase,
	InstKindUnary,
	InstKindBinary,
	InstKindTernary,
	InstKindArl,
	InstKindBranch,
	InstKindCond,
	InstKindSrc,
	InstKindSwizzle,
	InstKindTex,	
	/*InstKindMemory*/
	InstKindPhi,
} InstKind;

typedef enum Opcode {
	/* Instr.   		 Inputs  Output   Description								*/
	/* -------			 ------  ------   --------------------------------			*/
	
	OpcodeARL,        /* s       a        address register load						*/
	OpcodeABS,	      /* v       v        absolute value							*/
	OpcodeABS_SAT,    /* v       v        absolute value with saturate				*/
	OpcodeADD,	      /* v,v     v        add										*/
	OpcodeADD_SAT,	  /* v,v     v        add with saturate							*/
	OpcodeCAL,	      /* c       -        subroutine call							*/
	OpcodeCMP,	      /* v,v,v   v        compare									*/
	OpcodeCMP_SAT,	  /* v,v,v   v        compare with saturate						*/
	OpcodeCOS,	      /* s       ssss     cosine with reduction to [-PI,PI]			*/
	OpcodeCOS_SAT,    /* s       ssss     cosine with reduction to [-PI,PI] (sat)	*/
	OpcodeDP2,	      /* v,v     ssss     2-component dot product					*/
	OpcodeDP2_SAT,	  /* v,v     ssss     2-component dot product	(sat)			*/
	OpcodeDP3,	      /* v,v     ssss     3-component dot product					*/
	OpcodeDP3_SAT,	  /* v,v     ssss     3-component dot product	(sat)			*/
	OpcodeDP4,	      /* v,v     ssss     4-component dot product					*/
	OpcodeDP4_SAT,	  /* v,v     ssss     4-component dot product	(sat)			*/
	OpcodeDPH,	      /* v,v     ssss     homogeneous dot product					*/
	OpcodeDPH_SAT,	  /* v,v     ssss     homogeneous dot product	(sat)			*/
	OpcodeDST,	      /* v,v     v        distance vector							*/
	OpcodeDST_SAT,	  /* v,v     v        distance vector							*/
	OpcodeEX2,	      /* s       ssss     exponential base 2						*/
	OpcodeEX2_SAT,	  /* s       ssss     exponential base 2						*/
	OpcodeEXP,        /* s       v        exponential base 2 (approximate)			*/
	OpcodeEXP_SAT,    /* s       v        exponential base 2 (approximate)			*/
	OpcodeFLR,	      /* v       v        floor										*/
	OpcodeFLR_SAT,	  /* v       v        floor										*/
	OpcodeFRC,	      /* v       v        fraction									*/
	OpcodeFRC_SAT,	  /* v       v        fraction									*/
	OpcodeLG2,	      /* s       ssss     logarithm base 2							*/
	OpcodeLG2_SAT,	  /* s       ssss     logarithm base 2							*/
	OpcodeLOG,        /* s       v        logarithm base 2 (approximate)			*/
	OpcodeLOG_SAT,    /* s       v        logarithm base 2 (approximate)			*/
	OpcodeLRP,	      /* v,v,v   v        linear interpolation						*/
	OpcodeLRP_SAT,	  /* v,v,v   v        linear interpolation						*/
	OpcodeMAD,	      /* v,v,v   v        multiply and add							*/
	OpcodeMAD_SAT,	  /* v,v,v   v        multiply and add							*/
	OpcodeMAX,	      /* v,v     v        maximum									*/
	OpcodeMAX_SAT,	  /* v,v     v        maximum									*/
	OpcodeMIN,	      /* v,v     v        minimum									*/
	OpcodeMIN_SAT,	  /* v,v     v        minimum									*/
	OpcodeMOV,	      /* v       v        move										*/
	OpcodeMOV_SAT,	  /* v       v        move										*/
	OpcodeMUL,	      /* v,v     v        multiply									*/
	OpcodeMUL_SAT,	  /* v,v     v        multiply									*/
	OpcodePOW,	      /* s,s     ssss     exponentiate								*/
	OpcodeRCP,	      /* s       ssss     reciprocal								*/
	OpcodeRCP_SAT,	  /* s       ssss     reciprocal								*/
	OpcodeRET,	      /* c       -        subroutine return							*/
	OpcodeRSQ,	      /* s       ssss     reciprocal square root					*/
	OpcodeRSQ_SAT,	  /* s       ssss     reciprocal square root					*/
	OpcodeSCS,	      /* s       ss--     sine/cosine without reduction				*/
	OpcodeSCS_SAT,	  /* s       ss--     sine/cosine without reduction				*/
	OpcodeSEQ,		  /* v,v     v        set on equal								*/
    OpcodeSFL,        /* v,v     v        set on false								*/
  	OpcodeSGE,        /* v,v     v        set on greater than or equal				*/
    OpcodeSGT,        /* v,v     v        set on greater than						*/
	OpcodeSIN,	      /* s       ssss     sine with reduction to [-PI,PI]			*/
	OpcodeSIN_SAT,	  /* s       ssss     sine with reduction to [-PI,PI]			*/
    OpcodeSLE,        /* v,v     v        set on less than or equal					*/
    OpcodeSLT,        /* v,v     v        set on less than							*/
    OpcodeSNE,        /* v,v     v        set on not equal							*/
    OpcodeSSG,        /* v       v        set sign									*/
    OpcodeSTR,        /* v,v(?)  v        set on true								*/
	OpcodeSUB,	      /* v,v     v        subtract									*/
	OpcodeSUB_SAT,	  /* v,v     v        subtract									*/
	OpcodeSWZ,	      /* v       v        extended swizzle							*/
	OpcodeSWZ_SAT,	  /* v       v        extended swizzle							*/
	OpcodeTEX,	      /* v       v        texture sample							*/
	OpcodeTEX_SAT,	  /* v       v        texture sample							*/
	OpcodeTXB,	      /* v       v        texture sample with bias					*/
	OpcodeTXB_SAT,	  /* v       v        texture sample with bias					*/
	OpcodeTXL,	      /* v       v        texture same w/explicit LOD				*/
	OpcodeTXL_SAT,	  /* v       v        texture same w/explicit LOD				*/
	OpcodeTXP,	      /* v       v        texture sample with projection			*/
	OpcodeTXP_SAT,	  /* v       v        texture sample with projection			*/
	OpcodeXPD,	      /* v,v     v        cross product								*/
	OpcodeXPD_SAT,	  /* v,v     v        cross product								*/


	/* Structured control flow instructions --------------------------------------- */
		
	OpcodeBRK,	      /* c       -        break out of loop instruction				*/
	OpcodeELSE,	  	  /* -       -        start if test else block					*/
	OpcodeENDIF,	  /* -       -        end if test block							*/
	OpcodeENDLOOP,	  /* -       -        end of loop block							*/
	OpcodeENDREP,	  /* -       -        end of repeat block						*/
	OpcodeIF,	      /* c       -        start of if test block					*/
	OpcodeLOOP,	  	  /* v       -        start of loop block   	     			*/
	OpcodeREP,	      /* v       -        start of repeat block						*/

#if GLES_VERTEX_SHADER_BRANCH || GLES_FRAGMENT_SHADER_BRANCH
	/* Generic control flow instructions ------------------------------------------ */
	
	OpcodeBRA,        /* c       -        branch									*/
#endif

	/* Fragment shader only instructions ------------------------------------------ */
		
	OpcodeKIL,	      /* c  	 v        kill fragment								*/

	/* Vincent Additions over ARB/NVidia instruction set -------------------------- */
	
	OpcodeSCC,		  /* v       -   	  set condition code				    	*/
	OpcodePHI,		  /* v,...   v		  phi instruction							*/

	/* Pseudo-Instructions for assembly language ---------------------------------- */
	
	OpcodeINPUT = 1000,					/* input variable declaration				*/
	OpcodeOUTPUT,						/* output variable declaration				*/
	OpcodePARAM,						/* parameter or constant declaration		*/
	OpcodeTEMP,							/* temporary variable declaration			*/
	OpcodeADDRESS,						/* address variable declaration				*/
	
	/* Pseudo-Types for assembly language ----------------------------------------- */
	
	OpcodeLow = 2000,					/* low precision							*/
	OpcodeMedium,						/* medium precision							*/
	OpcodeHigh,							/* high precision							*/
	
	OpcodeBool,							/* bool type								*/
	OpcodeBoolVec2,						/* bvec2 type								*/
	OpcodeBoolVec3,						/* bvec3 type								*/
	OpcodeBoolVec4,						/* bvec4 type								*/
	OpcodeInt,							/* int type									*/
	OpcodeIntVec2,						/* ivec2 type								*/
	OpcodeIntVec3,						/* ivec3 type								*/
	OpcodeIntVec4,						/* ivec4 type								*/
	OpcodeFloat,						/* float type								*/
	OpcodeFloatVec2,					/* vec2 type 								*/
	OpcodeFloatVec3,					/* vec3 type 								*/
	OpcodeFloatVec4,					/* vec4 type 								*/
	OpcodeFloatMat2,					/* mat2 type								*/
	OpcodeFloatMat3,					/* mat3 type								*/
	OpcodeFloatMat4,					/* mat4 type								*/
	OpcodeSampler2D,					/* sampler2D type							*/
	OpcodeSampler3D,					/* sampler3D type							*/
	OpcodeSamplerCube,					/* samplerCube type							*/
} Opcode;		

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

struct Log;
union Symbol;

/**
 * Possible condition codes used for branching instructions.
 * 
 * The codes also serve as condition flag masks, with the bit assignments
 * 
 * 				less	equal	greater
 * 				bit 0	bit 1	bit 2
 */
typedef enum Cond {
	CondF,									/**< always false						*/
	CondLT,									/**< less than							*/
	CondEQ,									/**< equal								*/
	CondLE,									/**< less or equal						*/
	CondGT,									/**< greater than						*/
	CondNE,	 								/**< not equal							*/
	CondGE,									/**< greater or equal					*/
	CondT									/**< always true						*/
} Cond;

/**
 * Texture target for texture access operations.
 */
typedef enum TextureTarget {
	TextureTarget2D,
	TextureTarget3D,
	TextureTargetCube
} TextureTarget;

/**
 * Extended swizzle selector values for SWZ instruction
 */
typedef enum ExtSwizzleOption {
	ExtSwizzleSelect0,						/**< set to 0					*/
	ExtSwizzleSelect1,						/**< set to 1					*/
	ExtSwizzleSelectX,						/**< select x component			*/
	ExtSwizzleSelectY,						/**< select y component 		*/
	ExtSwizzleSelectZ,						/**< select z component			*/
	ExtSwizzleSelectW,						/**< select w component			*/
	ExtSwizzleSelectNeg = 8,				/**< unused by itself			*/
	ExtSwizzleSelectNeg1,					/**< set to -1					*/
	ExtSwizzleSelectNegX,					/**< select -x component		*/
	ExtSwizzleSelectNegY,					/**< select -y component		*/
	ExtSwizzleSelectNegZ,					/**< select -z component		*/
	ExtSwizzleSelectNegW					/**< select -w component		*/
} ExtSwizzleOption;

typedef union Inst Inst;
typedef struct Block Block;

/**
 * Blocks represent a basic linear sequence of instructions, such that if
 * any instruction of a block is executed, then all of them are.
 * 
 * Blocks hold instructions as a doubly-linked list, and form themselves
 * a doubly linked list.
 */
struct Block {
	GLuint			id;					/**< block identifier				*/
	Block *			prev;				/**< double-linked block list ptr.	*/
	Block *			next;				/**< double-linked block list ptr.	*/
	Inst *			first;				/**< double-linked inst. list ptr.	*/			
	Inst * 			last;				/**< double-linked inst. list ptr.	*/
};

/**
 * A reference to a block, even if the block has not been defined yet
 */
typedef struct Label {
	struct Label *	next;				/**< Singly-linked list ptr.		*/
	Block *			target;				/**< reference to target			*/
	union Symbol *	symbol;				/**< if applicable, source symbol	*/
} Label;

/**
 * Data segment identifiers
 */
typedef enum ProgVarSegment {
	ProgVarSegNone,		/**< no segment: not allocated or special register	*/
//	ProgVarSegConst,					/**< constant data segment			*/
	ProgVarSegParam,					/**< parameter data segment			*/
	ProgVarSegAttrib,					/**< attrib data segment			*/
	ProgVarSegVarying,					/**< varying data segment			*/
	ProgVarSegLocal,					/**< local/temp data segment		*/
} ProgVarSegment;

/**
 * Flavors of shader program variables
 */
typedef enum ProgVarKind {
	ProgVarKindParam,					/**< shader parameter (uniform)		*/
	ProgVarKindTemp,					/**< local data						*/
	ProgVarKindIn,						/**< input value (attrib, varying)	*/
	ProgVarKindOut,						/**< output value (frag., varying)	*/
	ProgVarKindConst,					/**< constant data					*/
} ProgVarKind;

typedef union ProgVar ProgVar;

typedef struct ProgVarBase {
	/* peristent fields */
	ProgVar *		next;				/**< next entry in list				*/
	ProgVarKind		kind;				/**< variable kind					*/
	GLuint			id;					/**< identifier						*/
	Type *			type;				/**< variable type					*/
	
	ProgVarSegment	segment: 4;			/**< data segment for variable		*/
	GLuint			location: GLES_MAX_ADDRESS_BITS;	
	GLuint			shift: 2;			/**< index with vec4				*/		
										/**< offset within segment in words	*/
	 
	/* transient data fields */
	GLboolean		special: 1;			/**< special built-in value			*/
	GLboolean		used: 1;			/**< for usage analysis				*/

} ProgVarBase;

typedef struct ProgVarConst {
	ProgVarBase 	base;
	Constant *		values;				/**< initializer values				*/
} ProgVarConst;

typedef struct ProgVarTemp {
	ProgVarBase		base;
} ProgVarTemp;

typedef struct ProgVarNamed {
	ProgVarBase		base;
	char *			name;				/**< variable name					*/
	GLsizeiptr		length;				/**< variable name length			*/
} ProgVarNamed;

typedef struct ProgVarParam {
	ProgVarNamed	named;
} ProgVarParam;

typedef struct ProgVarIn {
	ProgVarNamed	named;
} ProgVarIn;

typedef struct ProgVarOut {
	ProgVarNamed	named;
} ProgVarOut;

union ProgVar {
	ProgVarBase			base;
	ProgVarConst		constant;
	ProgVarParam		param;
	ProgVarNamed		named;
	ProgVarIn			in;
	ProgVarOut			out;
	ProgVarTemp			temp;
};

typedef struct ProgVarAddr {
	struct ProgVarAddr *	next;
	GLuint					id;			/**< variable identifier			*/
	GLboolean				used: 1;
} ProgVarAddr;

/**
 * Data structure to represent a shader program in the low level (intermediate)
 * shading language used by Vincent.
 */
typedef struct ShaderProgram {
	MemoryPool *		memory;			/**< memory for all associated data	*/
	BlockList			blocks;			/**< List of basic blocks			*/
	Label *				labels;			/**< List of labels					*/
	Label *				main;			/**< entry point to shader program	*/
	GLuint				numVars;		/**< number of variables			*/
	GLuint				numAddrVars;	/**< number of address variables	*/
	GLuint				numBlocks;		/**< number of generated blocks		*/
	
	ProgVar *			param;			/**< List of all parameters			*/
	ProgVar *			temp;			/**< List of temporaries			*/
	ProgVar *			out;			/**< output values					*/
	ProgVar *			in;				/**< input values					*/
	ProgVarAddr *		addr;			/**< address registers				*/
	
	/** Hash table for words of constant data								*/ 
	ProgVar *			constants[GLES_CONSTANT_HASH];
} ShaderProgram;

/**
 * Data structure used while constructing a low level shading language program.
 */
typedef struct ShaderProgramGenerator {
	ShaderProgram *		result;			/**< resulting shader program		*/
	BlockList *			currentList;	/**< currently active block list	*/
	GLsizei				instructionCount;	/**< number of generated instructions */
} ShaderProgramGenerator;

typedef union SrcRef {
	ProgVarBase	*		base;
	ProgVarConst *		constant;
	ProgVarParam *		param;
	ProgVarIn *			in;
	ProgVarOut *		out;
	ProgVarTemp *		temp;
} SrcRef;

typedef union DstRef {
	ProgVarBase	*		base;
	ProgVarOut *		out;
	ProgVarTemp *		temp;
} DstRef;

/**
 * Representation of the swizzle mask for an operation
 */
typedef struct Swizzle {
	GLubyte				selectX	: 2;	/**< selection for x-component		*/
	GLubyte				selectY	: 2;	/**< selection for y-component		*/
	GLubyte				selectZ	: 2;	/**< selection for z-component		*/
	GLubyte				selectW	: 2;	/**< selection for w-component		*/
} Swizzle;

/**
 * Representation of a regular source operand, which can be either of scalar
 * or of vector type
 */
typedef struct SrcReg {
	SrcRef				reference;		/**< register or variable			*/
	ProgVarAddr *		index;			/**< optional index register		*/
	GLsizeiptr			offset;			/**< integer offset					*/
	GLubyte				selectX	: 2;	/**< selection for x-component		*/
	GLubyte				selectY	: 2;	/**< selection for y-component		*/
	GLubyte				selectZ	: 2;	/**< selection for z-component		*/
	GLubyte				selectW	: 2;	/**< selection for w-component		*/
	GLboolean			negate	: 1;	/**< negate value before use		*/
} SrcReg;

/**
 * Constructor function for the SrcReg structure.
 * 
 * @param	reg			the structure to initialize
 * @param	reference	register or variable being referenced
 * @param	offset		for array and structure access
 * @param 	selectX		selection for x-component
 * @param	selectY		selection for y-component
 * @param	selectZ		selection for z-component
 * @param	selectW		selection for w-component
 * @param	negate		negate value before use
 */
static GLES_INLINE void GlesInitSrcReg(SrcReg *		reg,
									   SrcRef		reference,
									   GLsizeiptr	offset,
									   GLubyte		selectX,
									   GLubyte		selectY,
									   GLubyte		selectZ,
									   GLubyte		selectW,
									   GLboolean	negate) {
	reg->reference 	= reference;
	reg->offset		= offset;
	reg->selectX 	= selectX;
	reg->selectY 	= selectY;
	reg->selectZ 	= selectZ;
	reg->selectW 	= selectW;
	reg->negate		= negate;
}

/**
 * Constructor function for the SrcReg structure.
 * 
 * All non-specified fields are filled with default values, i.e.
 * no swizzling of components and no negation of argument value.
 * 
 * @param	reg			the structure to initialize
 * @param	reference	register or variable being referenced
 */
static GLES_INLINE void GlesInitSrcReg0(SrcReg * reg, SrcRef reference) {
	GlesInitSrcReg(reg, reference, 0, 0, 1, 2, 3, GL_FALSE);
}

typedef struct DstReg {
	DstRef				reference;
	GLsizeiptr			offset;			/**< offset from base location		*/
	GLboolean			maskX : 1;		/**< write mask for x-component		*/
	GLboolean			maskY : 1;		/**< write mask for y-component		*/
	GLboolean			maskZ : 1;		/**< write mask for z-component		*/
	GLboolean			maskW : 1;		/**< write mask for w-component		*/
} DstReg;

/**
 * Constructor for DstReg structure.
 * 
 * @param	reg			the structure to initialize
 * @param	reference	register or variable being referenced
 * @param	offset		offset from base location
 * @param	maskX		write mask for x-component
 * @param	maskY		write mask for y-component
 * @param	maskZ		write mask for z-component
 * @param	maskW		write mask for w-component
 */
static GLES_INLINE void GlesInitDstReg(DstReg *		reg,
									   DstRef		reference,
									   GLsizeiptr	offset,
									   GLboolean	maskX,
									   GLboolean	maskY,
									   GLboolean	maskZ,
									   GLboolean	maskW) {
	reg->reference 	= reference;
	reg->offset		= offset;
	reg->maskX 		= maskX;
	reg->maskY 		= maskY;
	reg->maskZ 		= maskZ;
	reg->maskW 		= maskW;
}
										   
/**
 * Constructor for DstReg structure.
 * 
 * All unspecified fields are set to default values, i.e. all components
 * are written.
 * 
 * @param	reg			the structure to initialize
 * @param	reference	register or variable being referenced
 */
static GLES_INLINE void GlesInitDstReg0(DstReg * reg,DstRef reference) {
	return GlesInitDstReg(reg, reference, 0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

/**
 * Constructor for DstReg structure.
 * 
 * All unspecified fields are set to default values, i.e. all components
 * are written unconditionally.
 * 
 * @param	reg			the structure to initialize
 * @param	reference	register or variable being referenced
 * @param	maskX		write mask for x-component
 * @param	maskY		write mask for y-component
 * @param	maskZ		write mask for z-component
 * @param	maskW		write mask for w-component
 */
static GLES_INLINE void GlesInitDstReg1(DstReg *	reg,
									    DstRef		reference,
									    GLboolean	maskX,
									    GLboolean	maskY,
									    GLboolean	maskZ,
									    GLboolean	maskW) {
	return GlesInitDstReg(reg, reference, 0, maskX, maskY, maskZ, maskW);
}
									   
typedef struct InstBase {
	Inst *		prev;
	Inst *		next;
	InstKind	kind;
	Opcode 		op;
} InstBase;

typedef struct InstAlu {
	InstBase		base;
	DstReg			dst;				/**< destination register 				*/
	Precision		prec	: 2;		/**< precision flags for result 		*/
} InstAlu;

typedef struct InstSrc {
	InstBase		base;
	SrcReg			arg;				/**< argument value for operation		*/
} InstSrc;

typedef struct InstUnary {
	InstAlu			alu;
	SrcReg			arg;				/**< argument value for operation		*/
} InstUnary;

typedef struct InstBinary {
	InstAlu			alu;
	SrcReg			left;				/**< left operand						*/
	SrcReg			right;				/**< right operand						*/
} InstBinary;

typedef struct InstTernary {
	InstAlu			alu;
	SrcReg			arg0;				/**< first operand						*/
	SrcReg			arg1;				/**< second operand						*/
	SrcReg			arg2;				/**< third operand						*/
} InstTernary;

typedef struct InstSwizzle {
	InstAlu				alu;
	SrcRef				arg;			/**< operand value						*/
	ExtSwizzleOption	optionX	: 4;	/**< selection for x-component			*/
	ExtSwizzleOption	optionY : 4;	/**< selection for y-component			*/
	ExtSwizzleOption	optionZ : 4;	/**< selection for z-component			*/
	ExtSwizzleOption	optionW : 4;	/**< selection for w-component			*/
} InstSwizzle;

typedef struct InstBranch {
	InstBase		base;
	Label *			target;				/**< branch target 						*/
	Cond			cond 	: 4;		/**< condition to test for 				*/
	GLubyte			selectX	: 2;		/**< 1st CC-component to test			*/
	GLubyte			selectY	: 2;		/**< 2nd CC-component to test			*/
	GLubyte			selectZ	: 2;		/**< 3rd CC-component to test			*/
	GLubyte			selectW	: 2;		/**< 4th CC-component to test			*/
} InstBranch;

typedef struct InstCond {
	InstBase		base;
	Cond			cond 	: 4;		/**< condition to test for 				*/
	GLubyte			selectX	: 2;		/**< 1st CC-component to test			*/
	GLubyte			selectY	: 2;		/**< 2nd CC-component to test			*/
	GLubyte			selectZ	: 2;		/**< 3rd CC-component to test			*/
	GLubyte			selectW	: 2;		/**< 4th CC-component to test			*/
} InstCond;

typedef struct InstTex {
	InstAlu				alu;
	SrcReg				coords;			/**< texture coordinates				*/
	ProgVar *			sampler;		/**< variable bound to sampler 			*/
	GLsizeiptr			offset;			/**< constant index for sampler array	*/
	TextureTarget		target 	: 2;	/**< texture target 					*/
} InstTex;

typedef struct InstArl {
	InstBase			base;
	ProgVarAddr *		dst;			/**< bound to address register symbol 	*/
	SrcReg				arg;			/**< index value to load				*/
} InstArl;

typedef struct InstPhi {
	InstBase			base;
	ProgVarTemp *		dst;			/**< phi instruction target 			*/
	GLsizei				numSrc;			/**< number of variables 				*/
	ProgVar *			src[0];			/**< source value list 					*/
} InstPhi;

/**
 * Representation of extended ARB instruction set.
 */ 
union Inst {
	InstBase		base;
	InstAlu			alu;
	InstUnary		unary;
	InstBinary		binary;
	InstTernary		ternary;
	InstArl			arl;
	InstBranch		branch;
	InstCond		cond;
	InstSrc			src;
	InstSwizzle		swizzle;
	InstTex			tex;
	
	InstPhi			phi;
};

/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

Label * GlesCreateLabel(struct ShaderProgramGenerator *	gen, union Symbol * sym);

void GlesGenInstBase(struct ShaderProgramGenerator *	gen, 
					  Opcode 			op);
					  
void GlesGenInstSrc(struct ShaderProgramGenerator *	gen, 
					Opcode 				op,
					const SrcReg *		src);
					  
void GlesGenInstUnary(struct ShaderProgramGenerator *	gen, 
					  Opcode 			op,
					  Precision			prec,
					  const DstReg *	dst,
					  const SrcReg *	src);
					  
void GlesGenInstBinary(struct ShaderProgramGenerator *	gen, 
					   Opcode 				op,
					   Precision			prec,
					   const DstReg *		dst,
					   const SrcReg *		left,
					   const SrcReg *		right);

void GlesGenInstTernary(struct ShaderProgramGenerator *	gen, 
					    Opcode 				op,
					    Precision			prec,
					    const DstReg *		dst,
					   	const SrcReg *		arg0,
					    const SrcReg *		arg1,
					    const SrcReg *		arg2);

void GlesGenInstBranch(struct ShaderProgramGenerator *	gen,
					   Opcode				op,
					   Label *				target,
					   Cond					cond,
					   GLubyte				selectX,
					   GLubyte				selectY,
					   GLubyte				selectZ,
					   GLubyte				selectW);
										   
void GlesGenInstCond(struct ShaderProgramGenerator *	gen,
				     Opcode				op,
				     Cond				cond,
				     GLubyte			selectX,
				     GLubyte			selectY,
				     GLubyte			selectZ,
				     GLubyte			selectW);
				   

void GlesGenInstTex(struct ShaderProgramGenerator *	gen,
					Opcode				op,
				 	Precision			prec,
					TextureTarget		target,
					const DstReg *		dst,
					const SrcReg *		src,
					ProgVar *			sampler,
					GLsizeiptr			offset);

void GlesGenInstSwizzle(struct ShaderProgramGenerator *	gen,
					 	Opcode 				op,
					 	Precision			prec,
					 	const DstReg *		dst,
					 	SrcRef				src,
					 	ExtSwizzleOption	optionX,
					 	ExtSwizzleOption	optionY,
					 	ExtSwizzleOption	optionZ,
					 	ExtSwizzleOption	optionW);

void GlesGenInstArl(struct ShaderProgramGenerator *	gen,
					Opcode				op,
					ProgVarAddr *		dst,
					const SrcReg *		arg);

/*
void GlesGenInstMemory(struct ShaderProgramGenerator *gen,
					   Opcode			op,
					   const DstReg *	dst,
					   const SrcReg *	src,
					   const SrcReg *	index,
					   GLsizeiptr		offset);
*/

Block * GlesCreateBlock(struct ShaderProgramGenerator * gen);
void GlesInsertBlockList(struct ShaderProgramGenerator * gen, BlockList * list);

ShaderProgram *	GlesCreateShaderProgram(MemoryPool * pool);

ProgVar * 	GlesCreateProgVarConst(ShaderProgram * program, Constant * constant, Type * type);
ProgVar * 	GlesCreateProgVarTemp(ShaderProgram * program, Type * type);
ProgVar * 	GlesCreateProgVarIn(ShaderProgram * program, Type * type, const char * name, GLsizei length);
ProgVar * 	GlesCreateProgVarOut(ShaderProgram * program, Type * type, const char * name, GLsizei length);
ProgVar * 	GlesCreateProgVarParam(ShaderProgram * program, Type * type, const char * name, GLsizei length);

ProgVarAddr * 	GlesCreateProgVarAddr(ShaderProgram * program);

void GlesWriteShaderProgram(struct Log * log, const ShaderProgram * program);
ShaderProgram * GlesParseShaderProgram(const char * text, GLsizeiptr length, MemoryPool * pool, MemoryPool * temp);

#endif /* GLES_FRONTEND_IL_H */
