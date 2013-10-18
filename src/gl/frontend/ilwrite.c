/*
** ==========================================================================
**
** $Id: ilwrite.c 75 2007-10-10 08:27:53Z hmwill $			
** 
** Intermediate Shader Language
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-10-10 01:27:53 -0700 (Wed, 10 Oct 2007) $
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

/*
 * --------------------------------------------------------------------------
 * Module-local data
 * --------------------------------------------------------------------------
 */

static const char *OpcodeNames[] = {
	"ARL",        /* s       a        address register load						*/
	"ABS",	      /* v       v        absolute value							*/
	"ABS_SAT",	  /* v       v        absolute value	(sat)					*/
	"ADD",	      /* v,v     v        add										*/
	"ADD_SAT",	  /* v,v     v        add	(sat)								*/
	"CAL",	      /* c       -        subroutine call							*/
	"CMP",	      /* v,v,v   v        compare									*/
	"CMP_SAT",	  /* v,v,v   v        compare	(sat)							*/
	"COS",	      /* s       ssss     cosine with reduction to [-PI,PI]			*/
	"COS_SAT",	  /* s       ssss     cosine with reduction to [-PI,PI]	(sat)	*/
	"DP2",	      /* v,v     ssss     2-component dot product					*/
	"DP2_SAT",	  /* v,v     ssss     2-component dot product	(sat)			*/
	"DP3",	      /* v,v     ssss     3-component dot product					*/
	"DP3_SAT",	  /* v,v     ssss     3-component dot product	(sat)			*/
	"DP4",	      /* v,v     ssss     4-component dot product					*/
	"DP4_SAT",	  /* v,v     ssss     4-component dot product	(sat)			*/
	"DPH",	      /* v,v     ssss     homogeneous dot product					*/
	"DPH_SAT",	  /* v,v     ssss     homogeneous dot product	(sat)			*/
	"DST",	      /* v,v     v        distance vector							*/
	"DST_SAT",	  /* v,v     v        distance vector	(sat)					*/
	"EX2",	      /* s       ssss     exponential base 2						*/
	"EX2_SAT",	  /* s       ssss     exponential base 2	(sat)				*/
	"EXP",        /* s       v        exponential base 2 (approximate)			*/
	"EXP_SAT",    /* s       v        exponential base 2 (approximate)	(sat)	*/
	"FLR",	      /* v       v        floor										*/
	"FLR_SAT",	  /* v       v        floor		(sat)							*/
	"FRC",	      /* v       v        fraction									*/
	"FRC_SAT",	  /* v       v        fraction	(sat)							*/
	"LG2",	      /* s       ssss     logarithm base 2							*/
	"LG2_SAT",	  /* s       ssss     logarithm base 2	(sat)					*/
	"LOG",        /* s       v        logarithm base 2 (approximate)			*/
	"LOG_SAT",    /* s       v        logarithm base 2 (approximate)	(sat)	*/
	"LRP",	      /* v,v,v   v        linear interpolation						*/
	"LRP_SAT",	  /* v,v,v   v        linear interpolation	(sat)				*/
	"MAD",	      /* v,v,v   v        multiply and add							*/
	"MAD_SAT",	  /* v,v,v   v        multiply and add	(sat)					*/
	"MAX",	      /* v,v     v        maximum									*/
	"MAX_SAT",	  /* v,v     v        maximum	(sat)							*/
	"MIN",	      /* v,v     v        minimum									*/
	"MIN_SAT",	  /* v,v     v        minimum	(sat)							*/
	"MOV",	      /* v       v        move										*/
	"MOV_SAT",	  /* v       v        move		(sat)							*/
	"MUL",	      /* v,v     v        multiply									*/
	"MUL_SAT",	  /* v,v     v        multiply	(sat)							*/
	"POW",	      /* s,s     ssss     exponentiate								*/
	"RCP",	      /* s       ssss     reciprocal								*/
	"RCP_SAT",	  /* s       ssss     reciprocal	(sat)						*/
	"RET",	      /* c       -        subroutine return							*/
	"RSQ",	      /* s       ssss     reciprocal square root					*/
	"RSQ_SAT",	  /* s       ssss     reciprocal square root	(sat)			*/
	"SCS",	      /* s       ss--     sine/cosine without reduction				*/
	"SCS_SAT",	  /* s       ss--     sine/cosine without reduction		(sat)	*/
	"SEQ",		  /* v,v     v        set on equal								*/
    "SFL",        /* v,v     v        set on false								*/
  	"SGE",        /* v,v     v        set on greater than or equal				*/
    "SGT",        /* v,v     v        set on greater than						*/
	"SIN",	      /* s       ssss     sine with reduction to [-PI,PI]			*/
	"SIN_SAT",	  /* s       ssss     sine with reduction to [-PI,PI]	(sat)	*/
    "SLE",        /* v,v     v        set on less than or equal					*/
    "SLT",        /* v,v     v        set on less than							*/
    "SNE",        /* v,v     v        set on not equal							*/
    "SSG",        /* v       v        set sign									*/
    "STR",        /* v,v(?)  v        set on true								*/
	"SUB",	      /* v,v     v        subtract									*/
	"SUB_SAT",	  /* v,v     v        subtract	(sat)							*/
	"SWZ",	      /* v       v        extended swizzle							*/
	"SWZ_SAT",	  /* v       v        extended swizzle	(sat)					*/
	"TEX",	      /* v       v        texture sample							*/
	"TEX_SAT",	  /* v       v        texture sample	(sat)					*/
	"TXB",	      /* v       v        texture sample with bias					*/
	"TXB_SAT",	  /* v       v        texture sample with bias		(sat)		*/
	"TXL",	      /* v       v        texture same w/explicit LOD				*/
	"TXL_SAT",	  /* v       v        texture same w/explicit LOD	(sat)		*/
	"TXP",	      /* v       v        texture sample with projection			*/
	"TXP_SAT",	  /* v       v        texture sample with projection	(sat)	*/
	"XPD",	      /* v,v     v        cross product								*/
	"XPD_SAT",	  /* v,v     v        cross product		(sat)					*/


	/* Structured control flow instructions ----------------------------------- */
		
	"BRK",	      /* c       -        break out of loop instruction				*/
	"ELSE",	  	  /* -       -        start if test else block					*/
	"ENDIF",	  /* -       -        end if test block							*/
	"ENDLOOP",	  /* -       -        end of loop block							*/
	"ENDREP",	  /* -       -        end of repeat block						*/
	"IF",	      /* c       -        start of if test block					*/
	"LOOP",	  	  /* v       -        start of loop block        				*/
	"REP",	      /* v       -        start of repeat block						*/

#if GLES_VERTEX_SHADER_BRANCH || GLES_FRAGMENT_SHADER_BRANCH
	/* Generic control flow instructions -------------------------------------- */
	
	"BRA",        /* c       -        branch									*/
#endif

	/* Fragment shader only instructions -------------------------------------- */
		
	"KIL",	      /* c  	 v        kill fragment								*/

	/* Vincent Additions over ARB/NVidia instruction set ---------------------- */
	
	"SCC",		  /* v       -   	  set condition code					    */
	"PHI",		  /* v,...   v		  phi instruction							*/
};

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static void WriteProgVar(Log * log, const ProgVarBase * var, GLsizeiptr offset) {
	
	const char * template = offset ? "$%d[%d]" : "$%d";
	char buffer[40];
				
	GlesLogAppend(log, buffer, GlesSprintf(buffer, template, var->id, offset)); 
}

static void MarkUsedProgVar(ProgVarBase * var) {	
	var->used = GL_TRUE;
}

static void WriteProgVarAddr(Log * log, const ProgVarAddr * var) {
	
	char buffer[40];
				
	GlesLogAppend(log, buffer, GlesSprintf(buffer, "a%d", var->id)); 
}

static void MarkUsedProgVarAddr(ProgVarAddr * var) {
	var->used = GL_TRUE;
}

static void WriteSrcReg(Log * log, const SrcReg * reg) {
	char buffer[40];
	
	if (reg->negate) {
		GlesLogAppend(log, "-", 1);
	}
	
	if (reg->index) {
		if (reg->offset) {			
			GlesLogAppend(log, buffer, 
						  GlesSprintf(buffer, "$%d[a%d+%d]", 
						  			  reg->reference.base->id, reg->index->id, reg->offset));
		} else {
			GlesLogAppend(log, buffer, 
						  GlesSprintf(buffer, "$%d[a%d]", 
						  			  reg->reference.base->id, reg->index->id));
		}
	} else {
		if (reg->offset) {
			GlesLogAppend(log, buffer, 
						  GlesSprintf(buffer, "$%d[%d]", 
						  			  reg->reference.base->id, reg->offset));
		} else {
			GlesLogAppend(log, buffer, 
						  GlesSprintf(buffer, "$%d", 
						  			  reg->reference.base->id));
		}
	}
	
	if (reg->selectX != 0 || reg->selectY != 1 || reg->selectZ != 2 || reg->selectW != 3) {
		GlesLogAppend(log, ".", 1);
		
		GlesLogAppend(log, &"xyzw"[reg->selectX], 1);
		GlesLogAppend(log, &"xyzw"[reg->selectY], 1);
		GlesLogAppend(log, &"xyzw"[reg->selectZ], 1);
		GlesLogAppend(log, &"xyzw"[reg->selectW], 1);
	}
}

static void MarkUsedSrcReg(SrcReg * reg) {
	if (reg->index) {
		reg->reference.base->used = GL_TRUE;
		reg->index->used = GL_TRUE;
	} else {
		reg->reference.base->used = GL_TRUE;
	}
}

static void WriteSwizzleSrcReg(Log * log, const InstSwizzle * swizzle) {
	char buffer[40];
	const char * options[] = {
		",0", ",1", ",x", ",y", ",z", ",w", ",-0", ",-1", ",-x", ",-y", ",-z", ",-w" 
	};
	
	GlesLogAppend(log, buffer, 
				  GlesSprintf(buffer, "$%d", 
				  			  swizzle->arg.base->id));
	
	GlesLogAppend(log, options[swizzle->optionX], ~0);
	GlesLogAppend(log, options[swizzle->optionX], ~0);
	GlesLogAppend(log, options[swizzle->optionX], ~0);
	GlesLogAppend(log, options[swizzle->optionX], ~0);
}

GLES_INLINE static void MarkUsedSwizzleSrcReg(InstSwizzle * swizzle) {
	swizzle->arg.base->used = GL_TRUE;
}

static void WriteOptionalMask(Log * log, const DstReg * reg) {
	if (!reg->maskW || !reg->maskX || !reg->maskY || !reg->maskZ) {	
		GlesLogAppend(log, ".", 1);
		
		if (reg->maskX)	GlesLogAppend(log, "x", 1);
		if (reg->maskY)	GlesLogAppend(log, "y", 1);
		if (reg->maskZ)	GlesLogAppend(log, "z", 1);
		if (reg->maskW)	GlesLogAppend(log, "w", 1);
	}
}

static void WriteDstReg(Log * log, const DstReg * reg) {
	WriteProgVar(log, reg->reference.base, reg->offset);
	WriteOptionalMask(log, reg);
}

static void MarkUsedDstReg(DstReg * reg) {
	MarkUsedProgVar(reg->reference.base);
}

static void WriteOpcode(Log * log, const InstBase * inst) {
	GlesLogAppend(log, OpcodeNames[inst->op], ~0);
	GlesLogAppend(log, " ", 1);
}

static void WriteAluOpcode(Log * log, const InstAlu * alu) {
	GlesLogAppend(log, OpcodeNames[alu->base.op], ~0);
	//WriteOpcode(log, &alu->base);
	
	switch (alu->prec) {
	case PrecisionUndefined:
		break;
		
	case PrecisionLow:
		GlesLogAppend(log, ".L", 2);
		break;
		
	case PrecisionMedium:
		GlesLogAppend(log, ".M", 2);
		break;
		
	case PrecisionHigh:
		GlesLogAppend(log, ".H", 2);
		break;		
	}

	GlesLogAppend(log, " ", 1);
}

static void WriteConditionMask(Log * log, 
							  Cond cond,
							  GLubyte selectX, GLubyte selectY, GLubyte selectZ, GLubyte selectW) {
	static const char * conditions[] = {
		"F", "LT", "EQ", "LE", "GT", "NE", "GE", "T"
	};
	
	GlesLogAppend(log, conditions[cond], ~0);
	GlesLogAppend(log, ".", 1);
	GlesLogAppend(log, &"xyzw"[selectX], 1);
	GlesLogAppend(log, &"xyzw"[selectY], 1);
	GlesLogAppend(log, &"xyzw"[selectZ], 1);
	GlesLogAppend(log, &"xyzw"[selectW], 1);
}

static void WriteOptConditionMask(Log * log, 
							  	 Cond cond,
							  	 GLubyte selectX, GLubyte selectY, GLubyte selectZ, GLubyte selectW) {
	if (cond != CondT) {
		GlesLogAppend(log, " (", 1);
		WriteConditionMask(log, cond, selectX, selectY, selectZ, selectW);
		GlesLogAppend(log, ")", 1);
	}
}

static void WriteBlockId(Log * log, const Block * block) {
	char buffer[20];
	
	GlesLogAppend(log, buffer, GlesSprintf(buffer, "b%d", block->id));
}

static void WriteLabel(Log * log, Label * label) {
	if (label && label->target) {
		WriteBlockId(log, label->target);
	} else {
		GlesLogAppend(log, "<UNDEFINED>", ~0);
	}
	
}

static void WriteTexTarget(Log * log, TextureTarget target) {
	const char * targets[] = {
		"2D", "3D", "CUBE"
	};
	
	GlesLogAppend(log, targets[target], ~0);
}

static void WriteInst(Log * log, const Inst * inst) {
	
	GlesLogAppend(log, "\t", 1);
	
	switch (inst->base.kind) {
	case InstKindBase:
		WriteOpcode(log, &inst->base);
		break;
		
	case InstKindUnary:
		WriteAluOpcode(log, &inst->alu);
		WriteDstReg(log, &inst->alu.dst);
		GlesLogAppend(log, ",", 1);
		WriteSrcReg(log, &inst->unary.arg);
		break;
		
	case InstKindBinary:
		WriteAluOpcode(log, &inst->alu);
		WriteDstReg(log, &inst->alu.dst);
		GlesLogAppend(log, ",", 1);
		WriteSrcReg(log, &inst->binary.left);
		GlesLogAppend(log, ",", 1);
		WriteSrcReg(log, &inst->binary.right);
		break;
		
	case InstKindTernary:
		WriteAluOpcode(log, &inst->alu);
		WriteDstReg(log, &inst->alu.dst);
		GlesLogAppend(log, ",", 1);
		WriteSrcReg(log, &inst->ternary.arg0);
		GlesLogAppend(log, ",", 1);
		WriteSrcReg(log, &inst->ternary.arg1);
		GlesLogAppend(log, ",", 1);
		WriteSrcReg(log, &inst->ternary.arg2);
		break;
		
	case InstKindArl:
		WriteOpcode(log, &inst->base);
		WriteProgVarAddr(log, inst->arl.dst);
		GlesLogAppend(log, ",", 1);
		WriteSrcReg(log, &inst->arl.arg);
		break;
		
	case InstKindBranch:
		WriteOpcode(log, &inst->base);
		WriteLabel(log, inst->branch.target);
		WriteOptConditionMask(log, inst->branch.cond, inst->branch.selectX, 
							 inst->branch.selectY, inst->branch.selectZ, inst->branch.selectW);	
		break;
		
	case InstKindCond:
		WriteOpcode(log, &inst->base);
		WriteConditionMask(log, inst->cond.cond, inst->cond.selectX, 
						  inst->cond.selectY, inst->cond.selectZ, inst->cond.selectW);	
		break;
		
	case InstKindSrc:
		WriteOpcode(log, &inst->base);
		WriteSrcReg(log, &inst->src.arg);
		break;
		
	case InstKindSwizzle:
		WriteAluOpcode(log, &inst->alu);
		WriteDstReg(log, &inst->alu.dst);
		GlesLogAppend(log, ",", 1);
		WriteSwizzleSrcReg(log, &inst->swizzle);
		break;
		
	case InstKindTex:	
		WriteAluOpcode(log, &inst->alu);
		WriteDstReg(log, &inst->alu.dst);
		GlesLogAppend(log, ",", 1);
		WriteSrcReg(log, &inst->tex.coords);
		GlesLogAppend(log, ",", 1);
		WriteProgVar(log, &inst->tex.sampler->named.base, inst->tex.offset);		
		GlesLogAppend(log, ",", 1);
		WriteTexTarget(log, inst->tex.target);
		break;
		
	/*InstKindMemory*/
	case InstKindPhi:
		WriteOpcode(log, &inst->base);
		break;
		
	}

	GlesLogAppend(log, ";\n", 2);
}

static void MarkUsed(Inst * inst) {
	
	switch (inst->base.kind) {
	case InstKindUnary:
		MarkUsedDstReg(&inst->alu.dst);
		MarkUsedSrcReg(&inst->unary.arg);
		break;
		
	case InstKindBinary:
		MarkUsedDstReg(&inst->alu.dst);
		MarkUsedSrcReg(&inst->binary.left);
		MarkUsedSrcReg(&inst->binary.right);
		break;
		
	case InstKindTernary:
		MarkUsedDstReg(&inst->alu.dst);
		MarkUsedSrcReg(&inst->ternary.arg0);
		MarkUsedSrcReg(&inst->ternary.arg1);
		MarkUsedSrcReg(&inst->ternary.arg2);
		break;
		
	case InstKindArl:
		MarkUsedProgVarAddr(inst->arl.dst);
		MarkUsedSrcReg(&inst->arl.arg);
		break;
		
	case InstKindSrc:
		MarkUsedSrcReg(&inst->src.arg);
		break;
		
	case InstKindSwizzle:
		MarkUsedDstReg(&inst->alu.dst);
		MarkUsedSwizzleSrcReg(&inst->swizzle);
		break;
		
	case InstKindTex:	
		MarkUsedDstReg(&inst->alu.dst);
		MarkUsedSrcReg(&inst->tex.coords);
		MarkUsedProgVar(&inst->tex.sampler->named.base);		
		break;
		
	/*InstKindMemory*/
	case InstKindPhi:
	default:
		break;
	}
}

static void WriteBlock(Log * log, const Block * block) {
	Inst * inst;
	
	WriteBlockId(log, block);
	GlesLogAppend(log, ":\n", 2);
	
	for (inst = block->first; inst; inst = inst->base.next) {
		WriteInst(log, inst);
	}
	
	GlesLogAppend(log, "\n", 1);
}

static void WriteOptSize(Log * log, GLsizeiptr size) {
	char buffer[20];
	
	if (size != 1) {
		GlesLogAppend(log, buffer, GlesSprintf(buffer, "[%d]", size));
	}
}

static void WriteOptLocation(Log * log, const ProgVarBase * base) {
	char buffer[40];
	
	if (base->kind == ProgVarKindConst) {
		GlesLogAppend(log, buffer, GlesSprintf(buffer, "@CONST[%d]", base->location));		
	} else if (base->segment != ProgVarSegNone) {
		switch (base->segment) {
		case ProgVarSegParam:
			GlesLogAppend(log, buffer, GlesSprintf(buffer, "@PARAM[%d]", base->location));
			break;
			
		case ProgVarSegAttrib:
			GlesLogAppend(log, buffer, GlesSprintf(buffer, "@ATTRIB[%d]", base->location));
			break;
			
		case ProgVarSegVarying:
			GlesLogAppend(log, buffer, GlesSprintf(buffer, "@VARYING[%d]", base->location));
			break;
			
		case ProgVarSegLocal:
			GlesLogAppend(log, buffer, GlesSprintf(buffer, "@LOCAL[%d]", base->location));
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
			return;
		}
	}
}

static void WriteType(Log * log, Type * type) {
	char buffer[20];
	static const char * precisions[] = {
		"", "low ", "medium ", "high "
	};
	
	GLsizei arrayElements = ~0;
	const char * typeName = "";
		
	GlesLogAppend(log, ":", 1);

	if (type->base.kind == TypeArray) {
		arrayElements = type->array.elements;
		type = type->array.elementType;
	}
	
	GlesLogAppend(log, precisions[type->base.prec], ~0);

	switch (type->base.kind) {
	case TypeBool:			typeName = "bool";			break;		
	case TypeBoolVec2:		typeName = "bvec2";			break;		
	case TypeBoolVec3:		typeName = "bvec3";			break;
	case TypeBoolVec4:		typeName = "bvec4";			break;
	case TypeInt:			typeName = "int";			break;
	case TypeIntVec2:		typeName = "ivec2";			break;
	case TypeIntVec3:		typeName = "ivec3";			break;
	case TypeIntVec4:		typeName = "ivec4";			break;
	case TypeFloat:			typeName = "float";			break;
	case TypeFloatVec2:		typeName = "vec2";			break;
	case TypeFloatVec3:		typeName = "vec3";			break;
	case TypeFloatVec4:		typeName = "vec4";			break;
	case TypeFloatMat2:		typeName = "mat2";			break;
	case TypeFloatMat3:		typeName = "mat3";			break;
	case TypeFloatMat4:		typeName = "mat4";			break;
	case TypeSampler2D:		typeName = "sampler2D";		break;
	case TypeSampler3D:		typeName = "sampler3D";		break;
	case TypeSamplerCube:	typeName = "samplerCube";	break;
		
	default:
		GLES_ASSERT(GL_FALSE);
	}
	
	GlesLogAppend(log, typeName, ~0);
	

	if (arrayElements != ~0) {
		GlesLogAppend(log, buffer, GlesSprintf(buffer, "[%d]", arrayElements));
	}
}

static void WriteInputs(Log * log, const ShaderProgram * program) {
	ProgVar * var;
	
	for (var = program->in; var; var = var->base.next) {
		if (var->base.used) {
			GlesLogAppend(log, "INPUT ", ~0);
			WriteProgVar(log, &var->base, 0);
			WriteOptSize(log, var->base.type->base.size);
			WriteType(log, var->base.type);
			WriteOptLocation(log, &var->base);
			GlesLogAppend(log, "=", 1);
			GlesLogAppend(log, var->in.named.name, var->in.named.length);
			GlesLogAppend(log, ";\n", 2);
		}
	}
}

static void WriteOutputs(Log * log, const ShaderProgram * program) {
	ProgVar * var;
	
	for (var = program->out; var; var = var->base.next) {
		if (var->base.used) {
			GlesLogAppend(log, "OUTPUT ", ~0);
			WriteProgVar(log, &var->base, 0);
			WriteOptSize(log, var->base.type->base.size);
			WriteType(log, var->base.type);
			WriteOptLocation(log, &var->base);
			GlesLogAppend(log, "=", 1);
			GlesLogAppend(log, var->out.named.name, var->out.named.length);
			GlesLogAppend(log, ";\n", 2);
		}
	}
}

static void WriteParams(Log * log, const ShaderProgram * program) {
	ProgVar * var;
	
	for (var = program->param; var; var = var->base.next) {
		if (var->base.used) {
			GlesLogAppend(log, "PARAM ", ~0);
			WriteProgVar(log, &var->base, 0);
			WriteOptSize(log, var->base.type->base.size);
			WriteType(log, var->base.type);
			WriteOptLocation(log, &var->base);
			GlesLogAppend(log, "=", 1);
			GlesLogAppend(log, var->param.named.name, var->param.named.length);
			GlesLogAppend(log, ";\n", 2);
		}
	}
}

static void WriteTemps(Log * log, const ShaderProgram * program) {
	ProgVar * var;
	
	for (var = program->temp; var; var = var->base.next) {
		if (var->base.used) {
			GlesLogAppend(log, "TEMP ", ~0);
			WriteProgVar(log, &var->base, 0);
			WriteOptSize(log, var->base.type->base.size);
			WriteType(log, var->base.type);
			WriteOptLocation(log, &var->base);
			GlesLogAppend(log, ";\n", 2);
		}
	}
}

static void ClearUsed(ProgVar * var) {
	for (; var; var = var->base.next) {
		var->base.used = GL_FALSE;
	}	
}

static void WriteAddrs(Log * log, const ShaderProgram * program) {
	ProgVarAddr * var;
	
	for (var = program->addr; var; var = var->next) {
		if (var->used) {
			GlesLogAppend(log, "ADDRESS ", ~0);
			WriteProgVarAddr(log, var);
			GlesLogAppend(log, ";\n", 2);
		}
	}
}

static void ClearUsedAddrs(const ShaderProgram * program) {
	ProgVarAddr * var;
	
	for (var = program->addr; var; var = var->next) {
		var->used = GL_FALSE;
	}
}

static void WriteConstantScalar(Log * log, GLfloat x) {
	char buffer[100];
	
	GlesLogAppend(log, buffer, GlesSprintf(buffer, "{ %f }", x));
}

static void WriteConstantVec2(Log * log, GLfloat x, GLfloat y) {
	char buffer[100];
	
	GlesLogAppend(log, buffer, GlesSprintf(buffer, "{ %f, %f }", x, y));
}

static void WriteConstantVec3(Log * log, GLfloat x, GLfloat y, GLfloat z) {
	char buffer[100];
	
	GlesLogAppend(log, buffer, GlesSprintf(buffer, "{ %f, %f, %f }", x, y, z));
}

static void WriteConstantVec4(Log * log, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	char buffer[100];
	
	GlesLogAppend(log, buffer, GlesSprintf(buffer, "{ %f, %f, %f, %f }", x, y, z, w));
}

static void WriteConstant(Log * log, const Constant * value, const Type * type, const char * separator) {
	GLsizei index;
		
	switch (type->base.kind) {
	case TypeBool:
		GlesLogAppend(log, separator, ~0);
		WriteConstantScalar(log, (GLfloat) value->boolValue[0]);
		break;
		
	case TypeBoolVec2:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec2(log, (GLfloat) value->boolValue[0], (GLfloat) value->boolValue[1]);
		break;
		
	case TypeBoolVec3:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec3(log, (GLfloat) value->boolValue[0], (GLfloat) value->boolValue[1],
						 (GLfloat) value->boolValue[2]);
		break;
		
	case TypeBoolVec4:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec4(log, (GLfloat) value->boolValue[0], (GLfloat) value->boolValue[1],
						 (GLfloat) value->boolValue[2], (GLfloat) value->boolValue[3]);
		break;
		
	case TypeInt:
		GlesLogAppend(log, separator, ~0);
		WriteConstantScalar(log, (GLfloat) value->intValue[0]);
		break;
		
	case TypeIntVec2:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec2(log, (GLfloat) value->intValue[0], (GLfloat) value->intValue[1]);
		break;
		
	case TypeIntVec3:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec3(log, (GLfloat) value->intValue[0], (GLfloat) value->intValue[1],
						 (GLfloat) value->intValue[2]);
		break;
		
	case TypeIntVec4:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec4(log, (GLfloat) value->intValue[0], (GLfloat) value->intValue[1],
						 (GLfloat) value->intValue[2], (GLfloat) value->intValue[3]);
		break;

	case TypeFloat:
		GlesLogAppend(log, separator, ~0);
		WriteConstantScalar(log, value->floatValue[0]);
		break;
		
	case TypeFloatVec2:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec2(log, value->floatValue[0], value->floatValue[1]);
		break;
		
	case TypeFloatVec3:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec3(log, value->floatValue[0], value->floatValue[1],
						 value->floatValue[2]);
		break;
		
	case TypeFloatVec4:
		GlesLogAppend(log, separator, ~0);
		WriteConstantVec4(log, value->floatValue[0], value->floatValue[1],
						 value->floatValue[2], value->floatValue[3]);
		break;
		
	case TypeFloatMat2:
		for (index = 0; index < 2; ++index) {
			GlesLogAppend(log, separator, ~0);
			WriteConstantVec2(log, value[index].floatValue[0], value[index].floatValue[1]);
			separator = ",";
		}
		
		break;
		
	case TypeFloatMat3:
		for (index = 0; index < 3; ++index) {
			GlesLogAppend(log, separator, ~0);
			WriteConstantVec3(log, value[index].floatValue[0], value[index].floatValue[1],
							 value[index].floatValue[2]);
			separator = ",";
		}
		
		break;
		
	case TypeFloatMat4:
		for (index = 0; index < 4; ++index) {
			GlesLogAppend(log, separator, ~0);
			WriteConstantVec4(log, value[index].floatValue[0], value[index].floatValue[1],
							 value[index].floatValue[2], value[index].floatValue[3]);
			separator = ",";
		}
		
		break;

	case TypeArray:
		for (index = 0; index < type->array.elements; ++index) {
			WriteConstant(log, value + index * type->array.elementType->base.size, 
						 type->array.elementType, separator);
			separator = ",";
		}
		
		break;
		
	case TypeStruct:
		for (index = 0; index < type->structure.numFields; ++index) {
			WriteConstant(log, value + type->structure.fields[index].offset, 
						 type->structure.fields[index].type, separator);
			separator = ",";
		}
		
		break;
	
	default:
		GLES_ASSERT(GL_FALSE);
	}
}

static void WriteConstants(Log * log, const ShaderProgram * program) {
	ProgVar * var;
	GLsizei index;
	
	for (index = 0; index < GLES_CONSTANT_HASH; ++index) {
		for (var = program->constants[index]; var; var = var->base.next) {
			if (var->base.used) {
				GlesLogAppend(log, "PARAM ", ~0);
				WriteProgVar(log, &var->base, 0);
				WriteType(log, var->base.type);
				WriteConstant(log, var->constant.values, var->constant.base.type, "="); 
				GlesLogAppend(log, ";\n", 2);
			}
		}
	}
}

static void ClearUsedConstants(const ShaderProgram * program) {
	ProgVar * var;
	GLsizei index;
	
	for (index = 0; index < GLES_CONSTANT_HASH; ++index) {
		for (var = program->constants[index]; var; var = var->base.next) {
			var->base.used = GL_FALSE;
		}
	}
}

static void MarkUsedVariables(const ShaderProgram * program) {
	Block * block;
	Inst * inst;
	
	ClearUsedConstants(program);
	ClearUsedAddrs(program);
	ClearUsed(program->param);
	ClearUsed(program->temp);
	ClearUsed(program->in);
	ClearUsed(program->out);
	
	for (block = program->blocks.head; block; block = block->next) {
		for (inst = block->first; inst; inst = inst->base.next) {
			MarkUsed(inst);
		}
	}
}

/**
 * Write the shader program to the log stream in human readable form.
 * 
 * @param	log		the log stream to use
 * @param	program	the shader program to dump
 */
void GlesWriteShaderProgram(Log * log, const ShaderProgram * program) {
	Block * block;
	
	static const char header[] = 
		"# ------------------------------------------------------------\n"
		"# IL Output\n"
		"# ------------------------------------------------------------\n"
		"\n";
		
	GlesLogAppend(log, header, ~0);
	
	MarkUsedVariables(program);
	
	WriteConstants(log, program);
	WriteParams(log, program);
	WriteInputs(log, program);
	WriteOutputs(log, program);
	WriteTemps(log, program);
	WriteAddrs(log, program);
	
	for (block = program->blocks.head; block; block = block->next) {
		WriteBlock(log, block);
	}
}

