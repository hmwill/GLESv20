/*
** ==========================================================================
**
** $Id: linker.c 76 2007-10-20 04:34:44Z hmwill $			
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

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "frontend/linker.h"
#include "frontend/memory.h"
#include "frontend/types.h"
#include "frontend/il.h"

/*
** --------------------------------------------------------------------------
** Module-local data
** --------------------------------------------------------------------------
*/

static const char * ErrorMessages[] = {
	"I0000: Internal compiler error",
	"I0001: Out of memory error",
	"L0001: Globals must have the same type (including the same names for"
			" structure and field names) and precision.", 
	"L0004: Too many attribute values.", 
	"L0005: Too many uniform values.", 
	"L0006: Too many varyings.", 
	"L0007: Fragment shader uses a varying that has not been declared in the vertex shader.", 
	"L0008: Type mismatch between varyings: ", 
	"L0009: Missing main function for shader.", 
	"L0010: Type mismatch between uniform declared in vertex and fragment shader: ", 
};

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

/**
 * Prepare the linker object for linking of the given shader program.
 * 
 * @param linker	the linker object to initialize
 * @param program	the shader program to link
 * 
 * @return	GL_TRUE if preparation was successful
 */
static GLboolean PrepareLinker(Linker * linker, Program * program) {
	GLES_ASSERT(linker->program == NULL);
	
	linker->program = program;	
	
	linker->tempMemory = GlesMemoryPoolCreate(GLES_DEFAULT_PAGE_SIZE, &linker->allocationHandler);
	linker->workMemory = GlesMemoryPoolCreate(GLES_DEFAULT_PAGE_SIZE, &linker->allocationHandler);
	linker->resultMemory = GlesMemoryPoolCreate(GLES_DEFAULT_PAGE_SIZE, &linker->allocationHandler);
	
	linker->uniforms = NULL;
	
	return GL_TRUE;
}

/**
 * Load the shader IL representations needed for the linking process
 * 
 * @param	linker	reference to linker object
 * 
 * @return	GL_TRUE if the operation was successful
 */
static GLboolean LoadShaders(Linker * linker) {
	Shader * fragmentShader = 
		GlesGetShaderObject(linker->state, linker->program->fragmentShader);
		
	Shader * vertexShader = 
		GlesGetShaderObject(linker->state, linker->program->vertexShader);

	if (!fragmentShader || !vertexShader) {
		/* internal error (corrupted IL) */
		GlesLinkError(linker, LinkI0000);
		return GL_FALSE;
	}
	
	linker->fragment = 
		GlesParseShaderProgram(fragmentShader->il, fragmentShader->size, 
							   linker->workMemory, linker->tempMemory);

	linker->vertex = 
		GlesParseShaderProgram(vertexShader->il, vertexShader->size, 
							   linker->workMemory, linker->tempMemory);
							   
	return linker->fragment != NULL && linker->vertex != NULL;
}

typedef struct String {
	char *					name;			/* pointer to character array */
	GLsizeiptr				length;			/* length of text */
} String;

/**
 * Mark all special variables within the given list of variables.
 * 
 * @param list
 * 		pointer to linked list of variables
 * 
 * @param specials
 * 		base address of array defining the special variable names
 * 
 * @param numSpecials
 * 		number of elements in special variable names array
 */
static void MarkSpecialVariableList(ProgVar * list, 
									const String * specials, GLsizei numSpecials) {
	for (; list; list = list->base.next) {
		GLsizei index;
		
		for (index = 0; index < numSpecials; ++index) {
			if (list->named.length == specials[index].length &&
				!GlesMemcmp(list->named.name, specials[index].name, specials[index].length)) {
				list->base.special = GL_TRUE;
			}
		}
	}
}

/**
 * Mark special shader variables as such.
 * 
 * @param	linker	reference to linker object
 * 
 * @return	GL_TRUE if the operation was successful
 */
static GLboolean MarkSpecialVariables(Linker * linker) {
	/* Special input variables to the fragment shader that are not varyings */
	static const String specialFragmentInputs[] = {
		{ "gl_FragCoord", sizeof("gl_FragCoord") },
		{ "gl_FrontFacing", sizeof("gl_FrontFacing") },
		{ "gl_PointCoord", sizeof("gl_PointCoord") }
	};
	
	/* Special output variables of the fragment shader */
	static const String specialFragmentOutputs[] = {
		{ "gl_FragColor", sizeof("gl_FragColor") },
		{ "gl_FragData", sizeof("gl_FragData") },
	};
	
	/* Special output variables of the vertex shader that are not varyings */
	static const String specialVertexOutputs[] = {
		{ "gl_Position", sizeof("gl_Position") },
		{ "gl_PointSize", sizeof("gl_PointSize") },
	};
	
	MarkSpecialVariableList(linker->vertex->out, 
							specialVertexOutputs, 
							GLES_ELEMENTSOF(specialVertexOutputs));
	
	MarkSpecialVariableList(linker->fragment->in, 
							specialFragmentInputs, 
							GLES_ELEMENTSOF(specialFragmentInputs));
	
	MarkSpecialVariableList(linker->fragment->out, 
							specialFragmentOutputs, 
							GLES_ELEMENTSOF(specialFragmentOutputs));
	
	return GL_TRUE;
}

typedef struct VariableInfo {
	ProgVar *	variable[2];		/**< reference to variables */
	GLsizei		offset;				/**< offset in memory block */
	GLsizei		shift;				/**< component shift */
} VariableInfo;

/**
 * Sort the array of variables by number of elements and size in place.
 * 
 * @param variables		pointer to array of variables
 * @param numVariables	number of variables in array
 */
static void SortVariables(VariableInfo * variables, GLsizei numVariables) {
	GLsizei outer, inner;			/**< loop index; use simple bubble sort	*/
	
	for (outer = 0; outer < numVariables - 1; ++outer) {
		for (inner = outer + 1; inner < numVariables; ++inner) {
			if (variables[inner].variable[0]->base.type->base.elements >
				variables[outer].variable[0]->base.type->base.elements ||
				(variables[inner].variable[0]->base.type->base.elements ==
				 variables[outer].variable[0]->base.type->base.elements &&
				 variables[inner].variable[0]->base.type->base.size >
				 variables[outer].variable[0]->base.type->base.size)) {
				VariableInfo temp = variables[outer];
				variables[outer] = variables[inner];
				variables[inner] = temp;
			}
		}
	}
}

typedef struct AllocationMap {
	GLsizei rows;					/**< number of rows, columns := 4 */
	GLsizei	columnLength[4];		/**< for each column, number of rows used */
	GLsizei rowsUsed;				/**< number of rows tauched by allocation */
	GLsizei rowsUsed2;				/**< if 2-component variables spill into second column, count from bottom */
} AllocationMap;

static void InitAllocationMap(Linker * linker, AllocationMap * map, 
							  GLsizei components) {
	GLES_ASSERT(components % 4 == 0);
	GlesMemset(map, 0, sizeof(AllocationMap));
	
	map->rows = components / 4;
}

static GLboolean AllocateVariables(Linker * linker, AllocationMap * map, 
								   VariableInfo * variable) {
								   	
	Type * type = variable->variable[0]->base.type;
	GLsizei index;
	
	switch (type->base.elements) {
	case 4:
	case 3:
		if (map->rowsUsed + type->base.size <= map->rows) {
			variable->offset = map->rowsUsed;
			variable->shift = 0;
			map->rowsUsed += type->base.size;
			
			for (index = 0; index < type->base.elements; ++index) {
				map->columnLength[index] = map->rowsUsed;
			}
			
			return GL_TRUE;
		} else {
			return GL_FALSE;
		}
		
	case 2:
		if (map->rowsUsed + type->base.size <= map->rows) {
			variable->offset = map->rowsUsed;
			variable->shift = 0;
			map->rowsUsed += type->base.size;
			
			for (index = 0; index < type->base.elements; ++index) {
				map->columnLength[index] = map->rowsUsed;
			}
			
			return GL_TRUE;
		} else if (map->columnLength[2] + type->base.size + map->rowsUsed2 <= map->rows) {
			map->rowsUsed2 += type->base.size;
			variable->offset = map->rows - map->rowsUsed2;
			variable->shift = 3;
			
			for (index = 0; index < type->base.elements; ++index) {
				map->columnLength[index + 2] = map->rowsUsed2;
			}
			
			return GL_TRUE;
		} else {			
			return GL_FALSE;
		}
		
	case 1:
		if (!map->rowsUsed2) {
			/* 
			 * case 1: all columns are still available and go all the way to the total
			 * number of rows.
			 */
			GLsizei columnIndex = -1;
			GLsizei leftOver = 0;//map->rows;
			
			/* pick column where variable fits with the least remaining space in column */
			for (index = 0; index < 4; ++index) {
				GLsizei spare = map->rows - map->columnLength[index] - type->base.size;
				
				if (spare >= 0 && spare > leftOver) {
					leftOver = spare;
					columnIndex = index;
				}
			}
			
			if (columnIndex == -1) {
				return GL_FALSE;
			} else {
				variable->shift = columnIndex;
				variable->offset = map->columnLength[columnIndex];
				map->columnLength[columnIndex] += type->base.size;
				return GL_TRUE;
			}			
		} else {
			/*
			 * case 2: we have started to allocate 2-component variables into z,w-columns.
			 * This means that only z and w column need to be considered.
			 */

			GLsizei columnIndex = -1;
			GLsizei leftOver = 0;//map->rows;
			
			/* pick column where variable fits with the least remaining space in column */
			for (index = 2; index < 4; ++index) {
				GLsizei spare = map->rows - map->rowsUsed2 - map->columnLength[index] - type->base.size;
				
				if (spare >= 0 && spare > leftOver) {
					leftOver = spare;
					columnIndex = index;
				}
			}
			
			if (columnIndex == -1) {
				return GL_FALSE;
			} else {
				variable->shift = columnIndex;
				variable->offset = map->columnLength[columnIndex];
				map->columnLength[columnIndex] += type->base.size;
				return GL_TRUE;
			}			
		}
		
	default:
		GLES_ASSERT(GL_FALSE);
		return GL_FALSE;
	}
}

/**
 * Map the varying variables of the vertex and fragment shaders.
 * 
 * Specifically, the functions performs the following steps:
 * <ol>
 * 	<li>Verify that varying variables used by fragment shader are defined by
 * 		vertex shader with correct type.
 * 	<li>All varying variables defined by vertex shader that are not used by
 * 		fragment shader can become temporaries.
 * 	<li>For all actual varyings, execute allocation algorithm, thereby rewriting
 * 		vertex and fragment shader code.
 * </ol>
 * 
 * @param	linker	reference to linker object
 * 
 * @return	GL_TRUE if the operation was successful
 */
static GLboolean MapVaryings(Linker * linker) {
	
	ProgVar * in, * out;
	ProgVar ** pVar;
	AllocationMap allocationMap;
	GLboolean success = GL_TRUE;
	GLsizei numVarying = 0, index;
	VariableInfo * variables = NULL;
	
	/* 
	 * Count the number of varyings
	 */
	for (in = linker->fragment->in; in; in = in->base.next) {
		if (in->base.special) {
			continue;
		} else {
			++numVarying;
		}
	}
	
	variables = GlesMemoryPoolAllocate(linker->workMemory, sizeof(VariableInfo) * numVarying);
			
	/* 
	 * For input to fragment shader that is not one of the special values, make
	 * sure that it is defined by the vertex shader and of compatible type.
	 * 
	 * It is an error if not all used varyings are defined in this fashion.
	 */
	
	index = 0;
	
	for (in = linker->fragment->in; in; in = in->base.next) {
		if (in->base.special) {
			continue;
		}
		
		variables[index].variable[0] = in;
		
		for (out = linker->vertex->out; out; out = out->base.next) {
			if (in->named.length == out->named.length &&
				!GlesMemcmp(in->named.name, out->named.name, in->named.length)) {
				break;
			}
		}
		
		if (!out) {
			GlesLinkError(linker, LinkL0007);
			return GL_FALSE;
		} if (!GlesTypeMatches(in->base.type, out->base.type)) {
			GlesLinkErrorSymbol(linker, LinkL0008, in->named.name, in->named.length);			
			return GL_FALSE;
		} else {
			variables[index].variable[1] = out;
		}
		
		out->base.used = GL_TRUE;
		++index;
	}
	
	/* 
	 * Convert all vertex shader output variables that are neither special
	 * output values not used by the fragment shader into regular temporary 
	 * variables.
	 */
	
	for (pVar = &linker->vertex->out; *pVar; ) {
		if (!(*pVar)->base.special && !(*pVar)->base.used) {
			ProgVar * var = *pVar;
			*pVar = var->base.next;
			
			/* convert variable to regular temporary value */
			var->base.kind = ProgVarKindTemp;
			var->base.next = linker->vertex->temp;
			linker->vertex->temp = var;
		} else {
			pVar = &(*pVar)->base.next;
		}
	}
	
	/* 
	 * Execute the variable allocation algorithm on the set of varyings.
	 * 
	 * It is an error if the set of varyings exceeds the available resources. 
	 */
	 
	InitAllocationMap(linker, &allocationMap, GLES_MAX_VARYING_FLOATS);	
	SortVariables(variables, numVarying);
	
	for (success = GL_TRUE, index = 0; index < numVarying; ++index) {
		success = AllocateVariables(linker, &allocationMap, variables + index);
		
		if (!success) {
			break;
		}
	}
	
	if (success) {
		for (index = 0; index < numVarying; ++index) {
			variables[index].variable[0]->base.segment = 
			variables[index].variable[1]->base.segment = ProgVarSegVarying;
			variables[index].variable[0]->base.location = 
			variables[index].variable[1]->base.location = 
				variables[index].offset * 4 + variables[index].shift;
				
			/* Guarantee that no variable needs data access across vec4 boundaries */
			GLES_ASSERT(variables[index].shift + variables[index].variable[1]->base.type->base.elements <= 4);
		}
	}
	
	/* number of used rows will be used during shader execution */
	linker->numVarying = allocationMap.rowsUsed * 4;
	
	return success;
}

static GLboolean AllocateAttribs(Linker * linker) {
	/* 
		TODO: allocate attrib variables; take calls to glBindAttribLocation
		into consideration
	*/
	
	return GL_TRUE;
}

/**
 * Create an overall table of all uniform variables used by the vertex and
 * the fragment shader.
 *
 * @param	linker	the linker object 
 *
 * @return	GL_TRUE if the operation was successful
 */
static GLboolean CreateGlobalUniformTable(Linker * linker) {
	ShaderVariable * 	vertexVars = NULL;
	ShaderVariable *	allVars = NULL;
	ProgVar * 			var;
	GLsizeiptr			numVertexVars = 0;
	GLsizeiptr			numFragmentVars = 0;
	GLsizeiptr			index = 0;
	GLsizeiptr			location = 0;
	
	for (var = linker->vertex->param; var; var = var->base.next) {
		++numVertexVars;
	}
	
	/* create an intermediate array of variables */
	vertexVars = 
		GlesMemoryPoolAllocate(linker->workMemory, 
							   sizeof(ShaderVariable) * numVertexVars);

	/* copy the vertex variables into the symbol table */
	for (var = linker->vertex->param; var; var = var->base.next, ++index) {
		char * name = 
			GlesMemoryPoolAllocate(linker->resultMemory, 
							   	   var->named.length + 1);

		GlesMemcpy(name, var->named.name, var->named.length);
		name[var->named.length] = '\0';
		
		vertexVars[index].name = 		name;
		vertexVars[index].length =		var->named.length;
		vertexVars[index].location =	~0;		/* not allocated yet */
		vertexVars[index].size =		var->base.type->base.size;
		vertexVars[index].type =		var->base.type->base.kind;
	}

	GlesSortShaderVariables(vertexVars, numVertexVars);
	
	/* create the result array of all variables */

	for (var = linker->fragment->param; var; var = var->base.next) {
		ShaderVariable * shaderVar = 
			GlesFindShaderVariable(vertexVars, numVertexVars,
								   var->named.name, var->named.length);
			
		if (!shaderVar) {
			++numFragmentVars;
		} else {
			/* ensure that the types match */
			if (var->base.type->base.kind != shaderVar->type ||
				var->base.type->base.size != shaderVar->size) {
				GlesLinkErrorSymbol(linker, LinkL0010, 
									var->named.name, var->named.length);
				return GL_FALSE;
			}
		}
	}
	
	allVars = 
		GlesMemoryPoolAllocate(linker->resultMemory, 
							   sizeof(ShaderVariable) * 
								(numVertexVars + numFragmentVars));
							
	GlesMemcpy(allVars, vertexVars, sizeof(ShaderVariable) * numVertexVars);
	
	for (var = linker->fragment->param; var; var = var->base.next) {
		if (!GlesFindShaderVariable(vertexVars, numVertexVars,
									var->named.name, var->named.length)) {
			char * name = 
				GlesMemoryPoolAllocate(linker->resultMemory, 
								   	   var->named.length + 1);

			GlesMemcpy(name, var->named.name, var->named.length);
			name[var->named.length] = '\0';

			allVars[index].name = 		name;
			allVars[index].length =		var->named.length;
			allVars[index].location =	~0;		/* not allocated yet */
			allVars[index].size =		var->base.type->base.size;
			allVars[index].type =		var->base.type->base.kind;
			
			++index;
		}
	}

	GlesSortShaderVariables(allVars, numVertexVars + numFragmentVars);
	
	linker->numUniforms = numVertexVars + numFragmentVars;
	linker->uniforms = allVars;
	
	/* Allocate variables in alphabetic order */
	
	for (index = 0; index < linker->numUniforms; ++index) {
		linker->uniforms[index].location = location;
		location += linker->uniforms[index].size;
	}
	
	return GL_TRUE;
}

/**
 * Pack uniforms and constants for a vertex or fragment shader.
 *
 * @param	linker		reference to linker object
 * @param	shader		reference to shader object
 * @param	components	storage area size
 * @param	storageUsed	out: number of vec4 words needed
 *
 * @return 	GL_TRUE		if successful
 */
static GLboolean PackUniforms(Linker * linker, ShaderProgram * shader,
							  GLsizei components, GLsizei * storageUsed) {
	AllocationMap 		allocationMap;
	VariableInfo *		variables;
	GLsizei				numUniforms = 0;
	GLsizei				hash, index;
	ProgVar *			var;
	GLboolean			success;
						
	/* count the number of uniforms and constants */
	
	for (var = shader->param; var; var = var->base.next) {
		++numUniforms;
	}	
	
	for (hash = 0; hash < GLES_CONSTANT_HASH; ++hash) {
		for (var = shader->constants[hash]; var; var = var->base.next) {
			++numUniforms;
		}	
	}
	
	/* Create a map of all variables and constants */
	
	variables = 
		GlesMemoryPoolAllocate(linker->workMemory, 
							   sizeof(VariableInfo) * numUniforms);
	
	index = 0;

	for (var = shader->param; var; var = var->base.next, ++index) {
		variables[index].variable[0] = var;
		variables[index].variable[1] = var;
	}	

	for (hash = 0; hash < GLES_CONSTANT_HASH; ++hash) {
		for (var = shader->constants[hash]; var; var = var->base.next, ++index) {
			variables[index].variable[0] = var;
			variables[index].variable[1] = var;
		}	
	}

	InitAllocationMap(linker, &allocationMap, components);	
	SortVariables(variables, numUniforms);
	
	for (success = GL_TRUE, index = 0; index < numUniforms; ++index) {
		success = AllocateVariables(linker, &allocationMap, variables + index);
		
		if (!success) {
			break;
		}
	}
	
	if (success) {
		for (index = 0; index < numUniforms; ++index) {
			variables[index].variable[0]->base.segment = ProgVarSegParam;
			variables[index].variable[0]->base.location = 
				variables[index].offset * 4 + variables[index].shift;
				
			/* Guarantee that no variable needs data access across vec4 boundaries */
			GLES_ASSERT(variables[index].shift + variables[index].variable[0]->base.type->base.elements <= 4);
		}
	}
								
	*storageUsed = allocationMap.rowsUsed;
	
	return success;
}

/**
 * Map all constants into the data segment based on the variable locations
 * determined by PackUniforms.
 *
 * @param	linker	reference to linker object
 * @param	shader	reference to shader object
 * @param	segment	reference to data segment to capture result
 * @param	storage	size of segment in vec4
 */
static void MapConstants(Linker * linker, ShaderProgram * shader, 
						 Segment * segment, GLsizei storage) {

	Vec4f * 	base;
	GLsizei		hash, index;
	ProgVar *	var;
	
	segment->size = storage;
	segment->base = GlesMemoryPoolAllocate(linker->workMemory, storage * sizeof(Vec4f));
	
	base = (Vec4f *) segment->base;
	
	for (hash = 0; hash < GLES_CONSTANT_HASH; ++hash) {
		for (var = shader->constants[hash]; var; var = var->base.next, ++index) {
			GLES_ASSERT(var->base.segment == ProgVarSegParam);
			GLfloat * start = base[var->base.location].v + var->base.shift;
			GLsizei elements = var->base.type->base.elements;
			GLsizei words = var->base.type->base.size;
			GLsizei index;
			Constant * constant = var->constant.values;
			
			/* need to discriminate based on type */
			while (words--) {
				switch (var->base.type->base.kind) {
				case TypeBool:
				case TypeBoolVec2:
				case TypeBoolVec3:
				case TypeBoolVec4:
					for (index = 0; index < elements; ++index) {
						start[index] = constant->boolValue[index];
					}
				
					break;
					
				case TypeInt:	
				case TypeIntVec2:
				case TypeIntVec3:
				case TypeIntVec4:
					for (index = 0; index < elements; ++index) {
						start[index] = constant->intValue[index];
					}
			
					break;
				
				
				case TypeFloat:	
				case TypeFloatVec2:
				case TypeFloatVec3:
				case TypeFloatVec4:
				case TypeFloatMat2:
				case TypeFloatMat3:
				case TypeFloatMat4:
					for (index = 0; index < elements; ++index) {
						start[index] = constant->floatValue[index];
					}
			
					break;
				
				
				case TypeSampler2D:
				case TypeSampler3D:
				case TypeSamplerCube:
					/* at this point, we do not support constant samplers */
					
				default:
					GLES_ASSERT(GL_FALSE);
				}
								
				start += 4;
				constant += 1;
			}
		}	
	}	
}

static GLboolean MergeUniforms(Linker * linker) {
	
	GLsizei	vertexUniformWords = 0;
	GLsizei	fragmentUniformWords = 0;
	
	/*
	1.  Collect union of uniform variables. For variables present in both
		vertex and fragment shader verify that the types and precisions(?)
		coincide.

	2.	Create a variable/memory location map for the union of the
		PARAM variables. This map is based on vec4 memory model.
	*/
	
	if (!CreateGlobalUniformTable(linker)) {
		return GL_FALSE;
	}
	
	/*		
	3.	Pack the variables and constants for the vertex shader
	*/
	
	if (!PackUniforms(linker, linker->vertex, 
					  GLES_MAX_VERTEX_UNIFORM_COMPONENTS,
					  &vertexUniformWords)) {
		return GL_FALSE;
	}
	
	/*	
	4.	Pack the variables and constants for the fragment shader
	*/
	
	if (!PackUniforms(linker, linker->fragment, 
					  GLES_MAX_FRAGMENT_UNIFORM_COMPONENTS,
					  &fragmentUniformWords)) {
		return GL_FALSE;
	}
	
	/*
	5.	Create an constant memory area for the vertex shader.
		Fill this memory area based on the constant data used.
	*/
	MapConstants(linker, linker->vertex, &linker->vertexData,
				 vertexUniformWords);

	/*	
	6.	Create a map that determines how data in the constant memory area
		for the vertex shader is initialized based on variable settings
		exposed through the API.
	*/
	
	/*	
	7.	Create an constant memory area for the fragment shader.
		Fill this memory area based on the constant data used.
	*/
	MapConstants(linker, linker->fragment, &linker->fragmentData,
				 fragmentUniformWords);
	
	/*
	8.	Create a map that determines how data in the constant memory area
		for the fragment shader is initialized based on variable settings
		exposed through the API.
	*/

	/*
	9.	Rewrite vertex program to reflect new variable allocations
	
	10.	Rewrite fragment program to reflect new variable allocations
	*/

	return GL_TRUE;
}

static Executable * GenerateExecutable(Linker * linker) {
	return NULL;
}

static Executable * Link(Linker * linker) {

	GLES_ASSERT(linker);
	
	if (LoadShaders(linker) 			&&
		MarkSpecialVariables(linker)	&&
		MapVaryings(linker)				&&
		AllocateAttribs(linker)			&&
		MergeUniforms(linker)) {
		return GenerateExecutable(linker);
	} else {
		return NULL;
	}
}

static void CleanupLinker(Linker * linker) {
	linker->program = NULL;
	
	if (linker->tempMemory) {
		GlesMemoryPoolDestroy(linker->tempMemory);
	}
	
	if (linker->workMemory) {
		GlesMemoryPoolDestroy(linker->workMemory);
	}
	
	if (linker->resultMemory) {
		GlesMemoryPoolDestroy(linker->resultMemory);
	}
} 

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

Linker * GlesLinkerCreate(State * state) {
	Linker * linker = GlesMalloc(sizeof (Linker));
	
	if (linker) {
		linker->state = state;
	}
	
	return linker;
}

void GlesLinkerDestroy(Linker * linker) {
	GLES_ASSERT(linker);
	
	GlesFree(linker);
}

/**
 * Append an error message to the linker log.
 * 
 * @param	linker	reference to linker object
 * @param	error	error message code
 */
void GlesLinkError(Linker * linker, LinkError error) {
	GLES_ASSERT(linker);
	GLES_ASSERT(linker->program);
	
	GlesLogAppend(&linker->program->log, ErrorMessages[error], GlesStrlen(ErrorMessages[error]));
}

/**
 * Append an error message to the linker log.
 * 
 * @param	linker	reference to linker object
 * @param	error	error message code
 * @param	name		name of offending symbol
 * @param	length		string length of symbol name
 */
void GlesLinkErrorSymbol(Linker * linker, LinkError error, 
	const char * name, GLsizeiptr length) {
	GLES_ASSERT(linker);
	GLES_ASSERT(linker->program);

	GlesLogAppend(&linker->program->log, ErrorMessages[error], GlesStrlen(ErrorMessages[error]));
	GlesLogAppend(&linker->program->log, name, length);
}

/**
 * Link the vertex shader with the fragment shader for the given program,
 * and create a binary executable that can be executed later-on.
 * 
 * @param	linker	reference to linker object
 * @param	program	reference to the program to link
 * 
 * @return	reference to a newly created executable for the program, or NULL
 * 			in case an error occurred.
 */
Executable * GlesLinkProgram(Linker * linker, Program * program) {

	Executable * result = NULL;
		
	GLES_ASSERT(program);
	GLES_ASSERT(linker->state);
	
	if (!GlesSetjmp(linker->allocationHandler)) {
		if (PrepareLinker(linker, program)) {
			result = Link(linker);
		}
	} else {
		GlesLinkError(linker, LinkI0001);
	}
		
	CleanupLinker(linker);
	
	return result;
}

/**
 * Destructor for shader binary object.
 *
 * @param	binary	the binary to destroy
 */
static void DestroyShaderBinary(ShaderBinary * binary) {
	
	GLES_ASSERT(binary);
	
	if (binary->code.base) {
		GlesFree(binary->code.base);
	}
	
	if (binary->data.base) {
		GlesFree(binary->data.base);
	}
}

/**
 * Destroy a previously created binary executable. This will reclaim any
 * resources used by the executable.
 * 
 * @param	state		reference to GL state
 * @param	executable	the binary executable to be destroyed
 */
void GlesDeleteExecutable(State * state, Executable * executable) {
	GLES_ASSERT(executable);

	if (executable->uniforms) {
		GlesFree(executable->uniforms);
	}
	
	if (executable->attribs) {
		GlesFree(executable->attribs);
	}
	
	DestroyShaderBinary(&executable->vertex);
	DestroyShaderBinary(&executable->fragment);
	
	GlesFree(executable);
}
