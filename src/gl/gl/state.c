/*
** ==========================================================================
**
** $Id: state.c 60 2007-09-18 01:16:07Z hmwill $
**
** General GL state management functions
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

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "gl/state.h"


/*
** --------------------------------------------------------------------------
** Module local data
** --------------------------------------------------------------------------
*/

/**
 * Valid values for hints
 */
static GLenum Hints[] = {
	GL_DONT_CARE,
	GL_FASTEST,
	GL_NICEST
};

/**
 * Global handle for GL state until we have thread local storage.
 */
static State GlobalState;

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

extern State * GlesGetGlobalState() {
	return &GlobalState;
}

void GlesRecordError(State * state, GLenum error) {
	if (state->lastError == GL_NO_ERROR) {
		state->lastError = error;
	}
}

void GlesRecordInvalidEnum(State * state) {
	GlesRecordError(state, GL_INVALID_ENUM);
}

void GlesRecordInvalidValue(State * state) {
	GlesRecordError(state, GL_INVALID_VALUE);
}

void GlesRecordOutOfMemory(State * state) {
	GlesRecordError(state, GL_OUT_OF_MEMORY);
}

void GlesRecordInvalidOperation(State * state) {
	GlesRecordInvalidOperation(state);
}

GLboolean GlesValidateEnum(State * state, GLenum value, const GLenum * values, GLuint numValues) {
	while (numValues) {
		if (value == *values) {
			return GL_TRUE;
		}

		++values;
		--numValues;
	}

	GlesRecordInvalidEnum(state);
	return GL_FALSE;
}

void GlesInitState(State * state) {

	GLuint index;

	GlesMemset(state, 0, sizeof *state);

	/* array state */

	for (index = 0; index < GLES_MAX_VERTEX_ATTRIBS; ++index) {
		GlesInitArray(state->vertexAttribArray + index);
		state->vertexAttrib[index].x = 0.0f;
		state->vertexAttrib[index].y = 0.0f;
		state->vertexAttrib[index].z = 0.0f;
		state->vertexAttrib[index].w = 1.0f;
	}

	GlesInitArray(&state->elementIndexArray);

	/* buffer state */

	for (index = 0; index < GLES_MAX_BUFFERS; ++index) {
		GlesInitBuffer(state->buffers + index);
		state->bufferFreeList[index] = index + 1;
	}

	state->bufferFreeList[GLES_MAX_BUFFERS - 1] = NIL;
	state->arrayBuffer = 0;
	state->elementArrayBuffer = 0;

	/* texture state */

	GlesInitTexture2D(&state->textureState.texture2D);
	GlesInitTexture3D(&state->textureState.texture3D);
	GlesInitTextureCube(&state->textureState.textureCube);

	state->texture2D				= 0;
	state->texture3D				= 0;
	state->textureCube				= 0;

	for (index = 0; index < GLES_MAX_TEXTURES; ++index) {
		state->textures[index].base.textureType = GL_INVALID_ENUM;
		state->textureFreeList[index] = index + 1;
	}

	state->textureFreeList[GLES_MAX_TEXTURES - 1] = NIL;

	/* shader state */

	for (index = 0; index < GLES_MAX_SHADERS; ++index) {
		state->shaders[index].type = GL_INVALID_ENUM;
		state->shaderFreeList[index] = index + 1;
	}

	state->shaderFreeList[GLES_MAX_SHADERS - 1] = NIL;

	/* program state */

	for (index = 0; index < GLES_MAX_PROGRAMS; ++index) {
		GlesInitProgram(state->programs + index);
		state->programFreeList[index] = index + 1;
	}

	state->programFreeList[GLES_MAX_PROGRAMS - 1] = NIL;

	state->program = 0;

	/* rendering state */

	state->cullFaceEnabled			= GL_FALSE;
	state->cullMode					= GL_BACK;
	state->frontFace				= GL_CCW;

	/* rasterization state */

	state->lineWidth				= 1.0f;
	state->pointSize				= 1.0f;
	state->polygonOffsetFactor		= 0;
	state->polygonOffsetUnits		= 0;

	/* fragment processing state */

	state->blendEnabled				= GL_FALSE;
	state->blendColor.red			= 0.0f;
	state->blendColor.green			= 0.0f;
	state->blendColor.blue			= 0.0f;
	state->blendColor.alpha			= 0.0f;
	state->blendEqnModeRBG			= GL_FUNC_ADD;
	state->blendEqnModeAlpha		= GL_FUNC_ADD;
	state->blendFuncSrcRGB			= GL_ONE;
	state->blendFuncDstRGB			= GL_ZERO;
	state->blendFuncSrcAlpha		= GL_ONE;
	state->blendFuncDstAlpha		= GL_ZERO;

	state->colorMask.red			= GL_TRUE;
	state->colorMask.green			= GL_TRUE;
	state->colorMask.blue			= GL_TRUE;
	state->colorMask.alpha			= GL_TRUE;

	state->depthTestEnabled			= GL_FALSE;
	state->depthFunc				= GL_LESS;
	state->depthMask				= GL_TRUE;

	state->scissorTestEnabled		= GL_FALSE;
	state->scissorRect.x			= 0;
	state->scissorRect.y			= 0;
	state->scissorRect.width		= GLES_MAX_VIEWPORT_WIDTH;
	state->scissorRect.height		= GLES_MAX_VIEWPORT_HEIGHT;

	state->stencilTestEnabled		= GL_FALSE;
	state->stencilFront.func		= GL_ALWAYS;
	state->stencilFront.ref			= 0;
	state->stencilFront.mask		= (1 << GLES_MAX_STENCIL_BITS) - 1;
	state->stencilFront.writeMask	= (1 << GLES_MAX_STENCIL_BITS) - 1;
	state->stencilBack.func			= GL_ALWAYS;
	state->stencilBack.ref			= 0;
	state->stencilBack.mask			= (1 << GLES_MAX_STENCIL_BITS) - 1;
	state->stencilBack.writeMask	= (1 << GLES_MAX_STENCIL_BITS) - 1;
	state->stencilFront.fail		= GL_KEEP;
	state->stencilFront.zfail		= GL_KEEP;
	state->stencilFront.zpass		= GL_KEEP;
	state->stencilBack.fail			= GL_KEEP;
	state->stencilBack.zfail		= GL_KEEP;
	state->stencilBack.zpass		= GL_KEEP;

	/* multi-sampling */
	state->multiSampleEnabled			= GL_FALSE;
	state->sampleAlphaToCoverageEnabled	= GL_FALSE;
	state->sampleAlphaToOneEnabled		= GL_FALSE;
	state->sampleCoverageEnabled		= GL_FALSE;
	state->sampleCovValue				= 1.0f;
	state->sampleCovInvert				= GL_FALSE;

	/* clear values */

	state->clearColor.red			= 0.0f;
	state->clearColor.green			= 0.0f;
	state->clearColor.blue			= 0.0f;
	state->clearColor.alpha			= 0.0f;

	state->clearDepth				= 1.0f;
	state->clearStencil				= 0;

	/* viewport state */

	state->viewport.x				= 0;
	state->viewport.y				= 0;
	state->viewport.width			= GLES_MAX_VIEWPORT_WIDTH;
	state->viewport.height			= GLES_MAX_VIEWPORT_HEIGHT;

	state->depthRange[0]			= 0.0f;		/* near */
	state->depthRange[1]			= 1.0f;		/* far */

	/* general settings */

	state->packAlignment			= 4;
	state->unpackAlignment			= 4;

	/* hints */

	state->generateMipmapHint			= GL_DONT_CARE;
	state->fragmentShaderDerivativeHint	= GL_DONT_CARE;

	/* error state */

	state->lastError				= GL_NO_ERROR;
}

void GlesDeInitState(State * state) {
	/* TODO */
}

void GlesGenObjects(State * state, GLuint * freeList, GLuint maxElements, GLsizei n, GLuint *objs) {
	GLuint * base = objs;

	if (n < 0 || objs == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	while (n--) {
		GLuint nextObj = GlesBindObject(freeList, maxElements);

		if (nextObj == NIL) {
			GlesRecordError(state, GL_OUT_OF_MEMORY);

			while (base != objs) {
				GlesUnbindObject(freeList, maxElements, *base++);
			}

			return;
		}

		*objs++ = nextObj;
	}
}

GLuint GlesBindObject(GLuint * freeList, GLuint maxElements) {
	if (freeList[0] != NIL) {
		GLuint result = freeList[0];
		freeList[0] = freeList[freeList[0]];
		freeList[result] = BOUND;
		return result;
	} else {
		return NIL;
	}
}

void GlesUnbindObject(GLuint * freeList, GLuint maxElements, GLuint obj) {
	GLES_ASSERT(GlesIsBoundObject(freeList, maxElements, obj));

	freeList[obj] = freeList[0];
	freeList[0] = obj;
}

GLboolean GlesIsBoundObject(GLuint * freeList, GLuint maxElements, GLuint obj) {
	if (obj < maxElements) {
		return freeList[obj] == BOUND;
	} else {
		return GL_FALSE;
	}
}

static void Toggle(GLenum cap, GLboolean value) {
	State * state = GLES_GET_STATE();
	
	switch (cap) {
	case GL_SCISSOR_TEST:
		state->scissorTestEnabled = value;
		break;
		
	case GL_SAMPLE_COVERAGE:
		state->sampleCoverageEnabled = value;
		break;
		
	case GL_SAMPLE_ALPHA_TO_COVERAGE:
		state->sampleAlphaToCoverageEnabled = value;
		break;
		
	case GL_STENCIL_TEST:
		state->stencilTestEnabled = value;
		break;
		
	case GL_DEPTH_TEST:
		state->depthTestEnabled = value;
		break;
		
	case GL_BLEND:
		state->blendEnabled = value;
		break;
		
	case GL_DITHER:
		state->ditherEnabled = value;
		break;
		
	case GL_VERTEX_PROGRAM_POINT_SIZE:
		state->vertexProgramPointSizeEnabled = value;
		break;
		
	case GL_CULL_FACE:
		state->cullFaceEnabled = value;
		break;
		
	case GL_POLYGON_OFFSET_FILL:
		state->polygonOffsetFillEnabled = value;
		break;
		
	default:
		GlesRecordInvalidEnum(state);
	}
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glDisable (GLenum cap) {
	Toggle(cap, GL_FALSE);
}

GL_API void GL_APIENTRY glEnable (GLenum cap) {
	Toggle(cap, GL_TRUE);
}

GL_API void GL_APIENTRY glFinish (void) {
	/* State * state = GLES_GET_STATE(); */
}

GL_API void GL_APIENTRY glFlush (void) {
	/* State * state = GLES_GET_STATE(); */
}

GL_API void GL_APIENTRY glHint (GLenum target, GLenum mode) {
	State * state = GLES_GET_STATE();
	
	if (!GlesValidateEnum(state, mode, Hints, GLES_ELEMENTSOF(Hints))) {
		return;
	}
	
	switch (target) {
	case GL_GENERATE_MIPMAP_HINT:
		state->generateMipmapHint = mode;
		break;
		
	case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
		state->fragmentShaderDerivativeHint = mode;
		break;
		
	default:
		GlesRecordInvalidEnum(state);
		return;
	}
}
