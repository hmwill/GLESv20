#ifndef GLES_FRONTEND_LINKER_H
#define GLES_FRONTEND_LINKER_H 1

/*
** ==========================================================================
**
** $Id: linker.h 76 2007-10-20 04:34:44Z hmwill $			
** 
** Shading Language Linker
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

#include "gl/state.h"
#include "frontend/memory.h"

/*
** --------------------------------------------------------------------------
** Constants
** --------------------------------------------------------------------------
*/

/**
 * Error codes that can occur during linking. Very conveniently :-(, the
 * specifications uses error codes for compilers and linkers, so we cannot
 * just create a single, large table.
 */
typedef enum LinkError {
	
	LinkI0000,		/* Internal compiler error						*/
	LinkI0001,		/* Out of memory error							*/
	
	LinkL0001, 		/* Globals must have the same type 				*/
	LinkL0004, 		/* Too many attribute values.					*/ 
	LinkL0005, 		/* Too many uniform values.						*/ 
	LinkL0006, 		/* Too many varyings.							*/ 
	LinkL0007, 		/* Fragment shader uses a varying that has not 	*/
					/* been declared in the vertex shader. 			*/ 
	LinkL0008, 		/* Type mismatch between varyings.				*/ 
	LinkL0009, 		/* Missing main function for shader.			*/ 
	LinkL0010, 		/* Type mismatch between uniforms.				*/ 
} LinkError;

/*
** --------------------------------------------------------------------------
** Structures
** --------------------------------------------------------------------------
*/

struct MemoryPool;

/**
 * Representation of a memory segment; the memory area can be NULL
 */
typedef struct Segment {
	void *		base;					/**< address of data area			*/
	GLsizeiptr	size;					/**< segment size in vec4			*/
} Segment;
	
/**
 * Representation of the binary code to be executed as a shader.
 */
typedef struct ShaderBinary {
	Segment		code;					/**< shader code segment			*/
	Segment		data;					/**< shader data segment (uniform)	*/
	GLsizeiptr	bssSize;				/**< size of temporary segment		*/
} ShaderBinary;

typedef struct Executable {

	/* meta-data */	
	GLsizei			numUniforms;		/**< number of uniform entries		*/
	ShaderVariable *uniforms;			/**< uniforms meta-data				*/

	GLsizei			numVertexAttribs;	/**< number of vertex attributes	*/
	ShaderVariable *attribs;			/**< vertex attribute meta-data		*/

	/* working storage */
	GLsizei			numVarying;			/**< number of varying vectors		*/
	GLsizei			sizeUniforms;		/**< number of uniform vectors		*/

	/* actual shader code */
	ShaderBinary	vertex;				/**< vertex shader binary			*/
	ShaderBinary	fragment;			/**< fragment shader binary			*/
} Executable;

/**
 * Linker object.
 */
typedef struct Linker {
	State *					state;		/**< reference to GL state			*/
	Program *				program;	/**< currently processed program	*/
	
	struct ShaderProgram *	vertex;		/**< IL vertex program				*/
	struct ShaderProgram *	fragment;	/**< IL fragment program			*/
	
	JumpBuffer				allocationHandler;	/**< handler to use for pool allocation failures */

	GLsizeiptr				numVarying;	/**< number of varying vectors		*/
	GLsizeiptr				numUniforms;/**< number of all uniform vars		*/
	
	ShaderVariable *		uniforms;	/**< all uniform variables			*/
	
	Segment					vertexData;		/**< uniform storage for VS		*/
	Segment					fragmentData;	/**< uniform storage for FS		*/
	
	MemoryPool *			tempMemory;	/**< short term working storage		*/
	MemoryPool *			workMemory;	/**< working storage				*/
	MemoryPool *			resultMemory;		/**< result storage			*/
} Linker;

/*
** --------------------------------------------------------------------------
** Functions
** --------------------------------------------------------------------------
*/

Linker * GlesLinkerCreate(State * state);
void GlesLinkerDestroy(Linker * linker);

Executable * GlesLinkProgram(Linker * linker, Program * program);
void GlesLinkError(Linker * linker, LinkError error);
void GlesLinkErrorSymbol(Linker * linker, LinkError error,
	const char * name, GLsizeiptr length);
void GlesDeleteExecutable(State * state, Executable * executable);

GLES_INLINE static FragmentProgram GlesFragmentProgram(Executable * executable) {
	GLubyte * address = (GLubyte *) executable->fragment.code.base;
	return (FragmentProgram) address;
}

GLES_INLINE static VertexProgram GlesVertexProgram(Executable * executable) {
	GLubyte * address = (GLubyte *) executable->vertex.code.base;
	return (VertexProgram) address;
}



#endif /* GLES_FRONTEND_LINKER_H */