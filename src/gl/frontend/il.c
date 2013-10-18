/*
** ==========================================================================
**
** $Id: il.c 71 2007-10-01 04:50:41Z hmwill $			
** 
** Intermediate Shader Language
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

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static GLES_INLINE void InitInstBase(InstBase * base, InstKind kind, Opcode op) {
	base->op = op;
	base->kind = kind;
}

static GLES_INLINE void InitInstAlu(InstAlu *		alu,
									InstKind		kind,
									Opcode			op,
									const DstReg *	dst,
									Precision		prec) {
	InitInstBase(&alu->base, kind, op);
	
	alu->dst	= *dst;
	alu->prec	= prec;
}
									
static GLES_INLINE void InitInstUnary(InstUnary *		unary,
									  Opcode 			op,
									  Precision			prec,
									  const DstReg *	dst,
									  const SrcReg *	src) {
	InitInstAlu(&unary->alu, InstKindUnary, op, dst, prec);
	
	unary->arg = *src; 
}									  	

static GLES_INLINE void InitInstSrc(InstSrc *		inst,
									Opcode			op,
									const SrcReg *	src) {
	InitInstBase(&inst->base, InstKindSrc, op);
	
	inst->arg	= *src;
}
																		  
static GLES_INLINE void InitInstBinary(InstBinary *		binary,
									   Opcode 			op,
									   Precision		prec,
									   const DstReg *	dst,
									   const SrcReg *	left,
									   const SrcReg *	right) {
	InitInstAlu(&binary->alu, InstKindBinary, op, dst, prec);
	
	binary->left = *left;
	binary->right = *right; 
}									  	
									  
static GLES_INLINE void InitInstTernary(InstTernary *	ternary,
									    Opcode 			op,
									    Precision		prec,
									    const DstReg *	dst,
									   	const SrcReg *	arg0,
									    const SrcReg *	arg1,
									    const SrcReg *	arg2) {
	InitInstAlu(&ternary->alu, InstKindTernary, op, dst, prec);
	
	ternary->arg0 = *arg0;
	ternary->arg1 = *arg1; 
	ternary->arg2 = *arg2; 
}									  	
									  
static GLES_INLINE void InitInstSwizzle(InstSwizzle *		swizzle,
									 	Opcode 				op,
									 	Precision			prec,
									 	const DstReg *		dst,
									 	SrcRef		 		src,
									 	ExtSwizzleOption	optionX,
									 	ExtSwizzleOption	optionY,
									 	ExtSwizzleOption	optionZ,
									 	ExtSwizzleOption	optionW) {
	InitInstAlu(&swizzle->alu, InstKindSwizzle, op, dst, prec);
	
	swizzle->arg 		= src; 
	swizzle->optionX	= optionX;
	swizzle->optionY	= optionY;
	swizzle->optionZ	= optionZ;
	swizzle->optionW	= optionW;
}									  	
									  
static GLES_INLINE void InitInstBranch(InstBranch * 	branch,
									   Opcode			op,
									   Label *			target,
									   Cond				cond,
									   GLubyte			selectX,
									   GLubyte			selectY,
									   GLubyte			selectZ,
									   GLubyte			selectW) {
	InitInstBase(&branch->base, InstKindBranch, op);
	
	branch->target 	= target;
	branch->cond	= cond;
	branch->selectX	= selectX;
	branch->selectY	= selectY;
	branch->selectZ	= selectZ;
	branch->selectW	= selectW;
}

static GLES_INLINE void InitInstCond(InstCond * 	iff,
								     Opcode			op,
								     Cond			cond,
								     GLubyte		selectX,
								     GLubyte		selectY,
								     GLubyte		selectZ,
								     GLubyte		selectW) {
	InitInstBase(&iff->base, InstKindCond, op);
	
	iff->cond		= cond;
	iff->selectX	= selectX;
	iff->selectY	= selectY;
	iff->selectZ	= selectZ;
	iff->selectW	= selectW;
}

static GLES_INLINE void InitInstTex(InstTex *		tex,
									Opcode			op,
								 	Precision		prec,
									TextureTarget	target,
									const DstReg *	dst,
									const SrcReg *	src,
									ProgVar *		sampler,
									GLsizeiptr 		offset) {
	InitInstAlu(&tex->alu, InstKindTex, op, dst, prec);
	
	tex->target		= target;
	tex->coords		= *src;
	tex->sampler	= sampler;
	tex->offset		= offset;
}
									
static GLES_INLINE void InitInstArl(InstArl *		arl,
									Opcode			op,
									ProgVarAddr *	dst,
									const SrcReg *	arg) {
	InitInstBase(&arl->base, InstKindArl, op);
	arl->dst	= dst;
	arl->arg 	= *arg;
}
									
#if 0
static GLES_INLINE void InitInstMemory(InstMemory *		mem,
									   Opcode			op,
									   const DstReg *	dst,
									   const SrcReg *	src,
									   ProgVarAddr *	index,
									   GLsizeiptr		offset) {
	InitInstBase(&mem->base, InstKindMemory, op);
	
	mem->dst	= *dst;
	mem->src	= *src;
	mem->offset	= offset;
	
	if (src) {
		mem->index = *src;
	} else {
		mem->index.reference = NULL;
	}
}
#endif

/**
 * Append an instruction to a basic block.
 * 
 * @param block		basic block to which instruction should be appended to
 * @param inst		instruction to append
 */
static GLES_INLINE void AppendInst(Block * block, Inst * inst) {
	GLES_ASSERT(!inst->base.prev);
	GLES_ASSERT(!inst->base.next);
	
	if (!block->first) {
		GLES_ASSERT(!block->last);
		block->first = block->last = inst;
	} else {
		GLES_ASSERT(block->last);
		block->last->base.next = inst;
		inst->base.prev = block->last;
		block->last = inst;
	}
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

/**
 * Create a new label structure.
 * 
 * @param	gen		reference to code generator object
 * @param	sym		(optional) associated source program symbol
 * 
 * @return	newly allocated label structure attached to generated program
 */
Label * GlesCreateLabel(struct ShaderProgramGenerator *	gen, union Symbol * sym) {
	Label * label = GlesMemoryPoolAllocate(gen->result->memory, sizeof(Label));
	
	label->next = gen->result->labels;
	label->symbol = sym;
	gen->result->labels = label;
		
	return label;
}


/**
 * Generate a base instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 */
void GlesGenInstBase(ShaderProgramGenerator *	gen, 
					 Opcode 					op) {
	
	Inst * inst;
	
	GLES_ASSERT(gen);
	
	if (!gen->currentList) {
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstBase));	
	InitInstBase(&inst->base, InstKindBase, op);
	AppendInst(gen->currentList->tail, inst);
}
					  
/**
 * Generate an instruction that takes a vector argument but does not
 * have any result register.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	src		source operand
 */
void GlesGenInstSrc(struct ShaderProgramGenerator *	gen, 
					Opcode 							op,
					const SrcReg *					src) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(src);

	gen->instructionCount++;
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstSrc));
	InitInstSrc(&inst->src, op, src);
	AppendInst(gen->currentList->tail, inst);
}					  

/**
 * Generate a unary instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	prec	precision flags for instruction
 * @param	dst		destination 
 * @param	src		source operand
 */
void GlesGenInstUnary(ShaderProgramGenerator *		gen, 
					  Opcode 						op,
					  Precision						prec,
					  const DstReg *				dst,
					  const SrcReg *				src) {
	
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(dst);
	GLES_ASSERT(src);
	
	gen->instructionCount++;
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
		
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstUnary));
	InitInstUnary(&inst->unary, op, prec, dst, src);
	AppendInst(gen->currentList->tail, inst);
}
					  
/**
 * Generate a binary instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	prec	precision flags for instruction
 * @param	dst		destination 
 * @param	left	left source operand
 * @param	right	right source operand
 */
void GlesGenInstBinary(ShaderProgramGenerator *		gen, 
					   Opcode 						op,
					   Precision					prec,
					   const DstReg *				dst,
					   const SrcReg *				left,
					   const SrcReg *				right) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(dst);
	GLES_ASSERT(left);
	GLES_ASSERT(right);
	
	gen->instructionCount++;
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstBinary));
	InitInstBinary(&inst->binary, op, prec, dst, left, right);
	AppendInst(gen->currentList->tail, inst);
}

/**
 * Generate a binary instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	prec	precision flags for instruction
 * @param	dst		destination 
 * @param	arg0	source operand 0
 * @param	arg1	source operand 1
 * @param	arg2	source operand 2
 */
void GlesGenInstTernary(ShaderProgramGenerator *	gen, 
					    Opcode 						op,
					    Precision					prec,
					    const DstReg *				dst,
					   	const SrcReg *				arg0,
					    const SrcReg *				arg1,
					    const SrcReg *				arg2) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(dst);
	GLES_ASSERT(arg0);
	GLES_ASSERT(arg1);
	GLES_ASSERT(arg2);
	
	gen->instructionCount++;
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstTernary));
	InitInstTernary(&inst->ternary, op, prec, dst, arg0, arg1, arg2);
	AppendInst(gen->currentList->tail, inst);
}

/**
 * Generate a branch or subroutine call instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	target	branch target
 * @param	cond	branch condition
 * @param	selectX	selector for 1st condition flag
 * @param	selectY	selector for 2nd condition flag
 * @param	selectZ	selector for 3rd condition flag
 * @param	selectW	selector for 4th condition flag
 */
void GlesGenInstBranch(ShaderProgramGenerator *	gen,
					   Opcode					op,
					   Label *					target,
					   Cond						cond,
					   GLubyte					selectX,
					   GLubyte					selectY,
					   GLubyte					selectZ,
					   GLubyte					selectW) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(target);
	
	gen->instructionCount++;
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstBranch));
	InitInstBranch(&inst->branch, op, target, cond, selectX, selectY, selectZ, selectW);
	AppendInst(gen->currentList->tail, inst);
}
										   
/**
 * Generate an conditional instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	cond	branch condition
 * @param	selectX	selector for 1st condition flag
 * @param	selectY	selector for 2nd condition flag
 * @param	selectZ	selector for 3rd condition flag
 * @param	selectW	selector for 4th condition flag
 */
void GlesGenInstCond(struct ShaderProgramGenerator *	gen,
				 	 Opcode								op,
					 Cond								cond,
					 GLubyte							selectX,
					 GLubyte							selectY,
					 GLubyte							selectZ,
					 GLubyte							selectW) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	
	/* this is a special case, since an IF with a constant condition will be removed */
	if (cond != CondT && cond != CondF) {
		gen->instructionCount++;
	}
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstCond));
	InitInstCond(&inst->cond, op, cond, selectX, selectY, selectZ, selectW);
	AppendInst(gen->currentList->tail, inst);
}
				   	
/**
 * Generate a texture access instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	prec	precision modifier
 * @param	target	texture target to use
 * @param	dst		destination
 * @param	src		texture coordinate vector
 * @param	sampler	texture sampler variable to access texture memory
 * @param	offset	offset for sampler array
 */
void GlesGenInstTex(ShaderProgramGenerator *	gen,
					Opcode						op,
				 	Precision					prec,
					TextureTarget				target,
					const DstReg *				dst,
					const SrcReg *				src,
					ProgVar *					sampler,
					GLsizeiptr					offset) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(dst);
	GLES_ASSERT(src);
	GLES_ASSERT(sampler);
	
	gen->instructionCount++;
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstTex));
	InitInstTex(&inst->tex, op, prec, target, dst, src, sampler, offset);
	AppendInst(gen->currentList->tail, inst);
}

/**
 * Generate an extended swizzle (SWZ) instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	prec	precision flags for instruction
 * @param	dst		destination 
 * @param	src		source operand 
 * @param	optionX	swizzle selector for x coordinate
 * @param	optionY	swizzle selector for y coordinate
 * @param	optionZ	swizzle selector for z coordinate
 * @param	optionW	swizzle selector for w coordinate
 */
void GlesGenInstSwizzle(ShaderProgramGenerator *	gen,
					 	Opcode 						op,
					 	Precision					prec,
					 	const DstReg *				dst,
					 	SrcRef						src,
					 	ExtSwizzleOption			optionX,
					 	ExtSwizzleOption			optionY,
					 	ExtSwizzleOption			optionZ,
					 	ExtSwizzleOption			optionW) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(dst);
	GLES_ASSERT(src.base);
	
	gen->instructionCount++;
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstSwizzle));
	InitInstSwizzle(&inst->swizzle, op, prec, dst, src, 
						optionX, optionY, optionZ, optionW);
	AppendInst(gen->currentList->tail, inst);
}

/**
 * Generate an address load (ARL) instruction.
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	dst		destination to load, must be address register variable
 * @param	arg		source operand to load; swizzle part is ignored
 */
void GlesGenInstArl(ShaderProgramGenerator *	gen,
					Opcode						op,
					ProgVarAddr *				dst,
					const SrcReg *				arg) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(dst);
	GLES_ASSERT(arg);
	
	gen->instructionCount++;
		
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstArl));
	InitInstArl(&inst->arl, op, dst, arg);
	AppendInst(gen->currentList->tail, inst);
}

#if 0
/**
 * Generate memory access instruction (only used in low level IL).
 * 
 * @param	gen		reference to code generator object
 * @param	op		instruction opcode
 * @param	dst		destination
 * @param	src		source operand
 * @param	index	index to use, can be NULL
 * @param	offset	constant offset to apply
 */
void GlesGenInstMemory(ShaderProgramGenerator *	gen,
					   Opcode					op,
					   const DstReg *			dst,
					   const SrcReg *			src,
					   const SrcReg *			index,
					   GLsizeiptr				offset) {
	Inst * inst;
	
	GLES_ASSERT(gen);
	GLES_ASSERT(dst);
	
	if (!gen->currentList) {
		/* 
		 * Code generation is currently suppressed; keep track of violation of
		 * constant expressions.
		 */
		gen->constViolation |= gen->constExpression;
		return;
	}
	
	inst = GlesMemoryPoolAllocate(gen->result->memory, sizeof(InstMemory));
	InitInstMemory(&inst->memory, op, dst, src, index, offset);
	AppendInst(gen->currentList->tail, inst);
}
#endif

/**
 * Create a new instruction sequence basic block.
 * 
 * @param	gen		reference to compiler object
 * 
 * @return	a newly generated block appended to the current list of blocks
 */
Block * GlesCreateBlock(ShaderProgramGenerator * gen) {
	Block * result;
	
	if (!gen->currentList) {
		return NULL;
	}
	
	result = GlesMemoryPoolAllocate(gen->result->memory, sizeof(Block));	
	result->id = gen->result->numBlocks++;
	result->prev = gen->currentList->tail;
	
	if (gen->currentList->tail) {
		GLES_ASSERT(gen->currentList->head);		
		gen->currentList->tail->next = result; 	
	} else {
		GLES_ASSERT(!gen->currentList->head);
		gen->currentList->head = result;
	}
	
	gen->currentList->tail = result;
	
	return result;
}

/**
 * Insert the contents of the given list of instructions blocks at the
 * current point in the overall instruction sequence.
 * 
 * @param	gen		reference to compiler object
 * @param	list	reference to list of blocks to insert
 */
void GlesInsertBlockList(struct ShaderProgramGenerator * gen, BlockList * list) {
	GLES_ASSERT(gen->currentList != list);
	
	while (list->head) {
		Block * block = list->head;
		list->head = block->next;
		
		block->next = NULL;
		block->prev = gen->currentList->tail;
		
		if (gen->currentList->tail) {
			gen->currentList->tail->next = block;
		} else {
			gen->currentList->head = block;
		}
		
		gen->currentList->tail = block;
	}
	
	list->tail = NULL;
}

/*
** --------------------------------------------------------------------------
** Functions to create higher-level objects in IL programs
** --------------------------------------------------------------------------
*/

ShaderProgram *	GlesCreateShaderProgram(MemoryPool * pool) {
	ShaderProgram * result = GlesMemoryPoolAllocate(pool, sizeof(ShaderProgram));
	
	result->memory = pool;
	
	return result;
}

ProgVar * GlesCreateProgVarConst(ShaderProgram * program, Constant * constant, Type * type) {
	GLsizei hash = GlesHashConstant(constant, type) % GLES_CONSTANT_HASH;
	ProgVar * var;
	
	for (var = program->constants[hash]; var; var = var->base.next) {
		if (GlesCompareConstant(var->constant.values, constant, type)) {
			return var;
		}
	}
	
	var = GlesMemoryPoolAllocate(program->memory, sizeof(ProgVarConst));
	
	var->base.kind = ProgVarKindConst;
	var->base.type = type;
	var->base.id = program->numVars++;
	
	var->constant.values = GlesMemoryPoolAllocate(program->memory, sizeof(Constant) * type->base.size);
	
	GlesMemcpy(var->constant.values, constant, sizeof(Constant) * type->base.size);
	var->base.next = program->constants[hash];
	program->constants[hash] = var;
	
	return var;
}

ProgVar * GlesCreateProgVarTemp(ShaderProgram * program, Type * type) {
	ProgVar * var = GlesMemoryPoolAllocate(program->memory, sizeof(ProgVarTemp));
	
	var->base.kind = ProgVarKindTemp;
	var->base.type = type;
	var->base.next = program->temp;
	var->base.id = program->numVars++;
	program->temp = var;
	
	return var;
}

ProgVar * GlesCreateProgVarIn(ShaderProgram * program, Type * type, const char * name, GLsizei length) {
	ProgVar * var = GlesMemoryPoolAllocate(program->memory, sizeof(ProgVarIn));
	
	var->base.kind = ProgVarKindIn;
	var->base.type = type;
	var->base.id = program->numVars++;
	var->in.named.name = GlesMemoryPoolAllocate(program->memory, length);
	
	GlesMemcpy(var->in.named.name, name, length);
	var->in.named.length = length;
	var->base.next = program->in;
	program->in = var;
	
	return var;
}

ProgVar * GlesCreateProgVarOut(ShaderProgram * program, Type * type, const char * name, GLsizei length) {
	ProgVar * var = GlesMemoryPoolAllocate(program->memory, sizeof(ProgVarOut));
	
	var->base.kind = ProgVarKindOut;
	var->base.type = type;
	var->base.id = program->numVars++;
	var->out.named.name = GlesMemoryPoolAllocate(program->memory, length);
	
	GlesMemcpy(var->out.named.name, name, length);
	var->out.named.length = length;
	var->out.named.base.next = program->out;
	program->out = var;
	
	return var;
}

ProgVar * GlesCreateProgVarParam(ShaderProgram * program, Type * type, const char * name, GLsizei length) {
	ProgVar * var = GlesMemoryPoolAllocate(program->memory, sizeof(ProgVarParam));
	
	var->base.kind = ProgVarKindParam;
	var->base.id = program->numVars++;
	var->param.named.base.type = type;
	var->param.named.name = GlesMemoryPoolAllocate(program->memory, length);
	
	GlesMemcpy(var->param.named.name, name, length);
	var->param.named.length = length;
	var->param.named.base.next = program->param;
	program->param = var;
	
	return var;
}

ProgVarAddr * GlesCreateProgVarAddr(ShaderProgram * program) {
	ProgVarAddr * var = GlesMemoryPoolAllocate(program->memory, sizeof(ProgVarAddr));
	
	var->next = program->addr;
	var->id = program->numAddrVars++;
	program->addr = var;
	
	return var;
}


					   