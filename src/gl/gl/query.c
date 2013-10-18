/*
** ==========================================================================
**
** $Id: query.c 67 2007-09-25 05:51:44Z hmwill $
**
** State query functions
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-24 22:51:44 -0700 (Mon, 24 Sep 2007) $
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
#include "gl/state.h"



/*
** --------------------------------------------------------------------------
** State query variables
** --------------------------------------------------------------------------
*/

/**
 * This enum disciminates the different kinds of state variables that can
 * be queried.
 */
typedef enum VarKind {
	VarKindConstant,				/* the variable is a constant			*/
	VarKindState,					/* the variable is a member of State	*/
	VarKindSurface					/* the variable is a member of Surface	*/
} VarKind;

/**
 * This enum discriminates the different types of state variables, which
 * also determines the best and possible state function to retrieve their 
 * values.
 */
typedef enum VarType {
	VarTypeToggle,					/* enable/disable						*/
	VarTypeBoolean,					/* boolean value						*/
	VarTypeInteger,					/* integer value						*/
	VarTypeFloat					/* float value							*/
} VarType;

/**
 * Structure to describe implementation constants/limitations
 */
typedef struct Constants {
	GLfloat		pointSizeRange[2];
	GLfloat		lineWidthRange[2];
	GLuint		subPixelBits;
	GLuint		max3dTextureSize;
	GLuint		maxTextureSize;
	GLuint		maxCubeMapTextureSize;
	GLuint		maxViewportDims[2];
	GLuint		maxElementIndicies;
	GLuint		maxElementVertices;
	GLuint		sampleBuffers;
	GLuint		samples;
	GLuint		numCompressedTextureFormats;
	GLuint		maxVertexAttribs;
	GLuint		maxVertexUniformComponents;
	GLuint		maxVaryingFloats;
	GLuint		maxCombinedTextureImageUnits;
	GLuint		maxVertexTextureImageUnits;
	GLuint		maxFragmentUnifromComponents;
} Constants;

/**
 * Constant instance of table of implementations constants/limitations
 */
static const Constants ConstantTable = {
	{	1.0f, 1.0f },						/* point size range */
	{	1.0f, 1.0f },						/* line width range */
	GLES_SUBPIXEL_BITS,						/* sub pixel bits */
	GLES_MAX_TEXTURE_3D_SIZE,				/* max 3d texture size */
	GLES_MAX_TEXTURE_SIZE,					/* max texture size */
	GLES_MAX_CUBE_MAP_TEXTURE_SIZE,			/* max cube map texture size */
	{ 	GLES_MAX_VIEWPORT_WIDTH, 
		GLES_MAX_VIEWPORT_HEIGHT },			/* max viewport dims */
	GLES_MAX_ELEMENTS_INDICES,				/* max element indicies */
	GLES_MAX_ELEMENTS_VERTICES,				/* max element vertices */
	GL_SAMPLE_BUFFERS,						/* max sample buffers */
	GLES_SAMPLES,							/* max samples */
	0,										/* # compressed texture formats */
	GLES_MAX_VERTEX_ATTRIBS,				/* max vertex attribs */
	GLES_MAX_VERTEX_UNIFORM_COMPONENTS,		/* max vertex uniform components */
	GLES_MAX_VARYING_FLOATS,				/* max varying floats */
	GLES_MAX_TEXTURE_UNITS,					/* max combined texture image units */
	GLES_MAX_TEXTURE_UNITS,					/* max vertex texture image units */
	GLES_MAX_FRAGMENT_UNIFORM_COMPONENTS	/* max fragment uniform components */
};

/**
 * Structure used to describe a state variable that can be queried through
 * the API.
 */
typedef struct Variable {
	GLenum		name;				/* the variable name					*/
	VarKind		kind;				/* where to find the variable			*/
	VarType		type;				/* variable type (GLboolean, ...)		*/
	GLsizeiptr	offset;				/* offset of variable with structure	*/
	GLsizei		elements;			/* number of elements					*/
} Variable;

/**
 * Constant table listing all the state variables that can be queried by the
 * user using the generic IsEnabled/GetBoolean/GetInteger/GetFloat API
 * calls.
 */
static const Variable Variables[] = {
	/* enable/disable */
	{ GL_SCISSOR_TEST,				VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, scissorTestEnabled), 			1 },
	{ GL_SAMPLE_COVERAGE,			VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, sampleCoverageEnabled), 		1 },
	{ GL_SAMPLE_ALPHA_TO_COVERAGE,	VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, sampleAlphaToCoverageEnabled), 	1 },
	{ GL_STENCIL_TEST,				VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, stencilTestEnabled), 			1 },
	{ GL_DEPTH_TEST,				VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, depthTestEnabled), 				1 },
	{ GL_BLEND,						VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, blendEnabled), 					1 },
	{ GL_DITHER,					VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, ditherEnabled), 				1 },
	{ GL_VERTEX_PROGRAM_POINT_SIZE,	VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, vertexProgramPointSizeEnabled), 1 },
	{ GL_CULL_FACE,					VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, cullFaceEnabled), 				1 },
	{ GL_POLYGON_OFFSET_FILL,		VarKindState,	VarTypeToggle, GLES_OFFSETOF(State, polygonOffsetFillEnabled), 		1 },
		
	/* boolean */
	{ GL_SAMPLE_COVERAGE_INVERT,	VarKindState,	VarTypeBoolean, GLES_OFFSETOF(State, sampleCovInvert), 	1 },
	{ GL_COLOR_WRITEMASK,			VarKindState,	VarTypeBoolean, GLES_OFFSETOF(State, colorMask), 		4 },
	{ GL_DEPTH_WRITEMASK,			VarKindState,	VarTypeBoolean, GLES_OFFSETOF(State, depthMask), 		1 },

	/* float */
	{ GL_DEPTH_RANGE,			VarKindState,	VarTypeFloat, GLES_OFFSETOF(State, depthRange), 			2 },
	{ GL_POINT_SIZE,			VarKindState,	VarTypeFloat, GLES_OFFSETOF(State, pointSize), 				1 },
	{ GL_LINE_WIDTH,			VarKindState,	VarTypeFloat, GLES_OFFSETOF(State, lineWidth), 				1 },
	{ GL_POLYGON_OFFSET_FACTOR,	VarKindState,	VarTypeFloat, GLES_OFFSETOF(State, polygonOffsetFactor), 	1 },
	{ GL_POLYGON_OFFSET_UNITS,	VarKindState,	VarTypeFloat, GLES_OFFSETOF(State, polygonOffsetUnits), 	1 },
	{ GL_SAMPLE_COVERAGE_VALUE,	VarKindState,	VarTypeFloat, GLES_OFFSETOF(State, sampleCovValue), 		1 },
	{ GL_COLOR_CLEAR_VALUE,		VarKindState,	VarTypeFloat, GLES_OFFSETOF(State, clearColor.rgba),		4 },
	{ GL_BLEND_COLOR,			VarKindState,	VarTypeFloat, GLES_OFFSETOF(State, blendColor.rgba), 		4 },

	{ GL_ALIASED_POINT_SIZE_RANGE,	VarKindConstant, VarTypeFloat, GLES_OFFSETOF(Constants, pointSizeRange), 2 },
	{ GL_ALIASED_LINE_WIDTH_RANGE,	VarKindConstant, VarTypeFloat, GLES_OFFSETOF(Constants, lineWidthRange), 2 },
		
	/* integer/state */
	{ GL_CURRENT_PROGRAM,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, program), 						1 },
	{ GL_ARRAY_BUFFER_BINDING,			VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, arrayBuffer), 					1 },
	{ GL_ELEMENT_ARRAY_BUFFER_BINDING,	VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, elementArrayBuffer), 			1 },
	{ GL_VIEWPORT,						VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, viewport), 					4 },
	{ GL_CULL_FACE,						VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, cullMode), 					1 },
	{ GL_FRONT_FACE,					VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, frontFace), 					1 },
	{ GL_TEXTURE_BINDING_2D,			VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, texture2D), 					1 },
	{ GL_TEXTURE_BINDING_3D,			VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, texture3D), 					1 },
	{ GL_TEXTURE_BINDING_CUBE_MAP,		VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, textureCube), 					1 },
	{ GL_ACTIVE_TEXTURE,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, clientTextureUnit),	 		1 },
	{ GL_STENCIL_WRITEMASK,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilFront.writeMask), 		1 },
	{ GL_STENCIL_BACK_WRITEMASK,		VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilBack.writeMask), 		1 },
	{ GL_DEPTH_CLEAR_VALUE,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, clearDepth), 					1 },
	{ GL_STENCIL_CLEAR_VALUE,			VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, clearStencil), 				1 },
	{ GL_SCISSOR_BOX,					VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, scissorRect), 					4 },
	{ GL_STENCIL_FUNC,					VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilFront.func), 			1 },
	{ GL_STENCIL_VALUE_MASK,			VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilFront.mask), 			1 },
	{ GL_STENCIL_REF,					VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilFront.ref), 			1 },
	{ GL_STENCIL_FAIL,					VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilFront.fail), 			1 },
	{ GL_STENCIL_PASS_DEPTH_FAIL,		VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilFront.zfail), 			1 },
	{ GL_STENCIL_PASS_DEPTH_PASS,		VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilFront.zpass),	 		1 },
	{ GL_STENCIL_BACK_FUNC,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilBack.func), 			1 },
	{ GL_STENCIL_BACK_VALUE_MASK,		VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilBack.mask), 			1 },
	{ GL_STENCIL_BACK_REF,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilBack.ref), 				1 },
	{ GL_STENCIL_BACK_FAIL,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilBack.fail), 			1 },
	{ GL_STENCIL_BACK_PASS_DEPTH_FAIL,	VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilBack.zfail),	 		1 },
	{ GL_STENCIL_BACK_PASS_DEPTH_PASS,	VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, stencilBack.zpass), 			1 },
	{ GL_DEPTH_FUNC,					VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, depthFunc), 					1 },
	{ GL_BLEND_SRC_RGB,					VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, blendFuncSrcRGB), 				1 },
	{ GL_BLEND_SRC_ALPHA,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, blendFuncSrcAlpha),	 		1 },
	{ GL_BLEND_DST_RGB,					VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, blendFuncDstRGB), 				1 },
	{ GL_BLEND_DST_ALPHA,				VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, blendFuncDstAlpha), 			1 },
	{ GL_BLEND_EQUATION_RGB,			VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, blendEqnModeRBG), 				1 },
	{ GL_BLEND_EQUATION_ALPHA,			VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, blendEqnModeAlpha), 			1 },
	{ GL_GENERATE_MIPMAP_HINT,			VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, generateMipmapHint), 			1 },
	{ GL_FRAGMENT_SHADER_DERIVATIVE_HINT,VarKindState,	VarTypeInteger, GLES_OFFSETOF(State, fragmentShaderDerivativeHint),	1 },

	/* integer/surface */
	{ GL_RED_BITS,		VarKindSurface,	VarTypeInteger, GLES_OFFSETOF(Surface, redBits), 	1 },
	{ GL_GREEN_BITS,	VarKindSurface,	VarTypeInteger, GLES_OFFSETOF(Surface, greenBits), 	1 },
	{ GL_BLUE_BITS,		VarKindSurface,	VarTypeInteger, GLES_OFFSETOF(Surface, blueBits), 	1 },
	{ GL_ALPHA_BITS,	VarKindSurface,	VarTypeInteger, GLES_OFFSETOF(Surface, alphaBits), 	1 },
	{ GL_DEPTH_BITS,	VarKindSurface,	VarTypeInteger, GLES_OFFSETOF(Surface, depthBits), 	1 },
	{ GL_STENCIL_BITS,	VarKindSurface,	VarTypeInteger, GLES_OFFSETOF(Surface, stencilBits), 1 },
	
	{ GL_IMPLEMENTATION_COLOR_READ_TYPE_OES,	VarKindSurface,	VarTypeInteger, GLES_OFFSETOF(Surface, colorReadType), 		1 },
	{ GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES,	VarKindSurface,	VarTypeInteger, GLES_OFFSETOF(Surface, colorReadFormat),	1 },

	/* integer/constant */
	{ GL_SUBPIXEL_BITS,						VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, subPixelBits), 								1 },
	{ GL_MAX_3D_TEXTURE_SIZE,				VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, max3dTextureSize), 							1 },
	{ GL_MAX_TEXTURE_SIZE,					VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxTextureSize), 								1 },
	{ GL_MAX_CUBE_MAP_TEXTURE_SIZE,			VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxCubeMapTextureSize), 						1 },
	{ GL_MAX_VIEWPORT_DIMS,					VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxViewportDims), 							2 },
	{ GL_MAX_ELEMENTS_INDICES,				VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxElementIndicies), 							1 },
	{ GL_MAX_ELEMENTS_VERTICES,				VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxElementVertices), 							1 },
	{ GL_SAMPLE_BUFFERS,					VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, sampleBuffers), 								1 },
	{ GL_SAMPLES,							VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, samples), 									1 },
	{ GL_NUM_COMPRESSED_TEXTURE_FORMATS,	VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, numCompressedTextureFormats), 	1 },
	{ GL_COMPRESSED_TEXTURE_FORMATS,		VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, numCompressedTextureFormats), 	0 },
	{ GL_MAX_VERTEX_ATTRIBS,				VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxVertexAttribs), 							1 },
	{ GL_MAX_VERTEX_UNIFORM_COMPONENTS,		VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxVertexUniformComponents), 		1 },
	{ GL_MAX_VARYING_FLOATS,				VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxVaryingFloats), 							1 },
	{ GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,	VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxCombinedTextureImageUnits), 1 },
	{ GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,	VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxVertexTextureImageUnits), 		1 },
	{ GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,	VarKindConstant, VarTypeInteger, GLES_OFFSETOF(Constants, maxFragmentUnifromComponents),	1 },
};

/**
 * Find the entry describing the nature of a given state variable.
 * 
 * @param name
 * 		the name of the variable to find in the meta-data table
 * 
 * @return
 * 		either the pointer to the Variable record describing the requested
 * 		variable, or NULL if no such variable exists.
 */
static const Variable * FindVariable(GLenum name) {
	GLsizei index;
	
	for (index = 0; index < GLES_ELEMENTSOF(Variables); ++index) {
		if (Variables[index].name == name) {
			return Variables + index;
		}
	}
	
	return NULL;
}

/**
 * For the given GL state and variable, determine the location of the
 * variable value in memory.
 * 
 * @param state
 * 		the current GL state structure
 * @param var
 * 		the descriptor of the state variable to retrieve
 * 
 * @return
 * 		the address of the value for the state variable in memory
 */
static const void * GetVariableAddress(State * state, const Variable * var) {
	const void * base = NULL;
	
	switch (var->kind) {
	case VarKindConstant:	base = &ConstantTable; 		break;
	case VarKindState:		base = state; 				break;
	case VarKindSurface:	base = state->writeSurface; break;
	default:				GLES_ASSERT(GL_FALSE);
	}
	
	return ((const GLubyte *) base) + var->offset;
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API GLboolean GL_APIENTRY glIsEnabled (GLenum cap) {
	State * state = GLES_GET_STATE();
	const Variable * var = FindVariable(cap);
	
	if (var == NULL || var->type != VarTypeToggle) {
		GlesRecordInvalidEnum(state);
		return GL_FALSE;
	}
	
	GLES_ASSERT(var->type == VarTypeBoolean && var->elements == 1);
	
	return *(const GLboolean *) GetVariableAddress(state, var);
}

GL_API void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean *params) {
	State * state = GLES_GET_STATE();
	const Variable * var = FindVariable(pname);
	const void * addr;
	GLuint count;
	
	if (!var) {
		GlesRecordInvalidEnum(state);
	}
	
	addr = GetVariableAddress(state, var);
	count = var->elements;
	
	switch (var->type) {
	case VarTypeToggle:
		*params = *(const GLboolean *) addr;
		break;
	
	case VarTypeBoolean:
		{
			const GLboolean * ref = (const GLboolean *) addr;
			
			while (count--) {
				*params++ = *ref++;
			}
		}
		
		break;
	
	case VarTypeInteger:
		{
			const GLuint * ref = (const GLuint *) addr;
			
			while (count--) {
				*params++ = (*ref++ != 0);
			}
		}
		
		break;
	
	case VarTypeFloat:
		{
			const GLfloat * ref = (const GLfloat *) addr;
			
			while (count--) {
				*params++ = (*ref++ != 0.0f);
			}
		}
		
		break;	
		
	default:
		GLES_ASSERT(GL_FALSE);
		break;	
	}
}

GL_API GLenum GL_APIENTRY glGetError (void) {
	State * state = GLES_GET_STATE();
	GLenum result = state->lastError;
	state->lastError = GL_NO_ERROR;
	return result;
}

GL_API void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *params) {
	State * state = GLES_GET_STATE();
	const Variable * var = FindVariable(pname);
	const void * addr;
	GLuint count;
	
	if (!var) {
		GlesRecordInvalidEnum(state);
	}
	
	addr = GetVariableAddress(state, var);
	count = var->elements;
	
	switch (var->type) {
	case VarTypeToggle:
		*params = *(const GLboolean *) addr;
		break;
	
	case VarTypeBoolean:
		{
			const GLboolean * ref = (const GLboolean *) addr;
			
			while (count--) {
				*params++ = *ref++;
			}
		}
		
		break;
	
	case VarTypeInteger:
		{
			const GLuint * ref = (const GLuint *) addr;
			
			while (count--) {
				*params++ = *ref++;
			}
		}
		
		break;
	
	case VarTypeFloat:
		{
			const GLfloat * ref = (const GLfloat *) addr;
			
			while (count--) {
				*params++ = *ref++;
			}
		}
		
		break;	
		
	default:
		GLES_ASSERT(GL_FALSE);
		break;	
	}
}

GL_API void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *params) {
	State * state = GLES_GET_STATE();
	const Variable * var = FindVariable(pname);
	const void * addr;
	GLuint count;
	
	if (!var) {
		GlesRecordInvalidEnum(state);
	}
	
	addr = GetVariableAddress(state, var);
	count = var->elements;
	
	switch (var->type) {
	case VarTypeToggle:
		*params = *(const GLboolean *) addr;
		break;
	
	case VarTypeBoolean:
		{
			const GLboolean * ref = (const GLboolean *) addr;
			
			while (count--) {
				*params++ = *ref++;
			}
		}
		
		break;
	
	case VarTypeInteger:
		{
			const GLuint * ref = (const GLuint *) addr;
			
			while (count--) {
				*params++ = *ref++;
			}
		}
		
		break;
	
	case VarTypeFloat:
		{
			const GLfloat * ref = (const GLfloat *) addr;
			
			while (count--) {
				*params++ = *ref++;
			}
		}
		
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
		break;	
	}
}

GL_API void GL_APIENTRY glGetPointerv (GLenum pname, void **params) {
	State * state = GLES_GET_STATE();
	
	switch (pname) {
		
	default:
		GlesRecordInvalidEnum(state);
		return;
	}
}

GL_API const GLubyte * GL_APIENTRY glGetString (GLenum name) {
	State * state = GLES_GET_STATE();
	
	switch (name) {
	case GL_VENDOR:
		return (const GLubyte *) GLES_VENDOR;
		
	case GL_RENDERER:
		return (const GLubyte *) GLES_RENDERER;
		
	case GL_VERSION:
		return (const GLubyte *) GLES_VERSION;
		
	case GL_SHADING_LANGUAGE_VERSION:
		return (const GLubyte *) GLES_SHADING_LANGUAGE;
		
	case GL_EXTENSIONS:
		return (const GLubyte *) GLES_EXTENSIONS;
		
	default:
		GlesRecordInvalidEnum(state);
		return NULL;
	}
}
