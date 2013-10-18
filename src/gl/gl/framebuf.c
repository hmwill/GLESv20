/*
** ==========================================================================
**
** $Id: framebuf.c 60 2007-09-18 01:16:07Z hmwill $
**
** Framebuffer functions
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
** Local functions
** --------------------------------------------------------------------------
*/

static void IntersectRect(Rect * rect, const Rect * other) {
	
	GLint x = rect->x > other->x ? rect->x : other->x;
	GLint y = rect->y > other->y ? rect->y : other->y;
	GLsizei width = rect->x + rect->width < other->x + other->width ?
					(rect->x - x) + rect->width : (other->x - x) + other->width;
	GLsizei height = rect->y + rect->height < other->y + other->height ?
					(rect->y - y) + rect->height : (other->y - y) + other->height;

	rect->x = x;
	rect->y = y;
	rect->width = width;
	rect->height = height;
}

static GLES_INLINE GLubyte Min(GLubyte a, GLubyte b) {
	return a < b ? a : b;
}

static GLES_INLINE GLushort MulUbyte(GLubyte a, GLubyte b) {
	GLushort prod = a * b;
	return (((prod + (prod >> 6)) >> 7) + 1) >> 1;
}

static GLES_INLINE GLubyte ClampUbyte(GLushort value) {
	return (GLubyte) (value > GLES_UBYTE_MAX ? GLES_UBYTE_MAX : value);
}

static GLES_INLINE GLubyte ColorValue(GLfloat color, GLsizei bits) {
	return (GLubyte) (((1 << bits) - 1) * color);
}

static GLES_INLINE GLuint ColorWord(GLenum colorFormat, GLubyte r,
									GLubyte g, GLubyte b, GLubyte a) {
	switch (colorFormat) {
	case GL_RGBA4:
		return (r & 0xf0) << 8 | (g & 0xf0) << 4 | (b & 0xf0) | a >> 4;
		
	case GL_RGB565_OES:
		return (b & 0xF8) >> 3 | (g & 0xFC) << 3 | (r & 0xF8) << 8;
		
	case GL_RGB5_A1:
		return (b & 0xF8) >> 2 | (g & 0xF8) << 3 | (r & 0xF8) << 8 | (a & 0x80) >> 7;
		
	default:
		GLES_ASSERT(GL_FALSE);
		return 0;
	}
}

static GLES_INLINE GLuint ColorWriteMask(GLenum colorFormat, const ColorMask * mask) {
	return ColorWord(colorFormat, 
					 mask->red 	 ? GLES_UBYTE_MAX : 0,
					 mask->green ? GLES_UBYTE_MAX : 0,
					 mask->blue  ? GLES_UBYTE_MAX : 0,
					 mask->alpha ? GLES_UBYTE_MAX : 0);
}
									 									  									  	
/*
** --------------------------------------------------------------------------
** Public entry points
** --------------------------------------------------------------------------
*/

/** 
 * compute effective rasterization area as intersection of surface size,
 * surface viewport, and scissor rect
 * 
 * @param state
 * 		the current GL state
 */
void GlesInitRasterRect(State * state) {	 
	state->rasterRect.x = 0;
	state->rasterRect.y = 0;
	state->rasterRect.width  = state->writeSurface->size.width;
	state->rasterRect.height = state->writeSurface->size.height;
	
	IntersectRect(&state->rasterRect, &state->writeSurface->viewport);
	
	if (state->scissorTestEnabled) {
		IntersectRect(&state->rasterRect, &state->scissorRect);
	}
}

void GlesInitSurfaceLoc(Surface * surface, SurfaceLoc * loc, GLuint x, GLuint y) {
	loc->surface = surface;
	loc->offset = x;
	
	loc->color = (GLubyte *) surface->colorBuffer + surface->colorPitch * y;
	
	if (surface->depthBuffer) {
		loc->depth = (GLubyte *) surface->depthBuffer + surface->depthPitch * y;
	} else {
		loc->depth = NULL;
	}
	
	if (surface->stencilBuffer) {
		loc->stencil = (GLubyte *) surface->stencilBuffer + surface->stencilPitch * y;
	} else {
		loc->stencil = NULL;
	}
}
 
void GlesStepSurfaceLoc(SurfaceLoc * loc, GLint x, GLint y) {
	loc->offset += x;

	if (y) {
		Surface * surface = loc->surface;
		
		if (y == 1) {
			loc->color = (GLubyte *) loc->color + surface->colorPitch;
		
			if (surface->depthBuffer) {
				loc->depth = (GLubyte *) loc->depth + surface->depthPitch;
			}
			
			if (surface->stencilBuffer) {
				loc->stencil = (GLubyte *) loc->stencil + surface->stencilPitch;
			}
		} else if (y== -1) {
			loc->color = (GLubyte *) loc->color - surface->colorPitch;
		
			if (surface->depthBuffer) {
				loc->depth = (GLubyte *) loc->depth - surface->depthPitch;
			}
			
			if (surface->stencilBuffer) {
				loc->stencil = (GLubyte *) loc->stencil - surface->stencilPitch;
			}
		} else {	
			loc->color = (GLubyte *) loc->color + surface->colorPitch * y;
		
			if (surface->depthBuffer) {
				loc->depth = (GLubyte *) loc->depth + surface->depthPitch * y;
			}
			
			if (surface->stencilBuffer) {
				loc->stencil = (GLubyte *) loc->stencil + surface->stencilPitch * y;
			}
		}
	}
}

void GlesReadColorub(const SurfaceLoc * loc, Colorub * result) {
	switch (loc->surface->colorFormat) {
	case GL_RGB8:
		{
			const GLubyte * ptr = (const GLubyte *) loc->color + loc->offset * 3;
			result->red 	= ptr[0];
			result->green 	= ptr[1];
			result->blue 	= ptr[2];
			result->alpha	= GLES_UBYTE_MAX;
		}
		
		break;
		
	case GL_RGBA8: 		
		{
			const GLubyte * ptr = (const GLubyte *) loc->color + loc->offset * 4;
			result->red 	= ptr[0];
			result->green 	= ptr[1];
			result->blue 	= ptr[2];
			result->alpha	= ptr[3];
		}
		
		break;
		
	case GL_RGBA4:
		{
			const GLushort u4444 = *((const GLushort *) loc->color + loc->offset);
			
			GLubyte r = (u4444 & 0xF000u) >> 8;
			GLubyte g = (u4444 & 0x0F00u) >> 4;
			GLubyte b = (u4444 & 0x00F0u);
			GLubyte a = (u4444 & 0x000Fu) << 4;
	
			result->red 	= r | r >> 4;
			result->green	= g | g >> 4;
			result->blue	= b | b >> 4;
			result->alpha	= a | a >> 4;
		}
		
		break;
		
	case GL_RGB5_A1:
		{
			const GLushort u5551 = *((const GLushort *) loc->color + loc->offset);
			
			GLubyte b = (u5551 & 0x003Eu) << 2;
			GLubyte g = (u5551 & 0x07C0u) >> 3;
			GLubyte r = (u5551 & 0xF800u) >> 8;
			GLubyte a = (u5551 & 0x0001u);
	
			result->red 	= r | r >> 5;
			result->green	= g | g >> 5;
			result->blue	= b | b >> 5;
			result->alpha	= a ? GLES_UBYTE_MAX : 0;
		}
		
		break;
		
	case GL_RGB565_OES:
		{
			const GLushort u565 = *((const GLushort *) loc->color + loc->offset);
			
			GLubyte b = (u565 & 0x001Fu) << 3;
			GLubyte g = (u565 & 0x07E0u) >> 3;
			GLubyte r = (u565 & 0xF800u) >> 8;
	
			result->red 	= r | r >> 5;
			result->green 	= g | g >> 6;
			result->blue 	= b | b >> 5;
			result->alpha	= GLES_UBYTE_MAX;
		}
		
		break;
		
	default: 
		GLES_ASSERT(GL_FALSE);
	}
}

void GlesWriteColorub(const SurfaceLoc * loc, const ColorMask * writeMask, const Colorub * color) {
	
	switch (loc->surface->colorFormat) {
	
	case GL_RGB8:
		{
			GLubyte * ptr = (GLubyte *) loc->color + loc->offset * 3;
			if (writeMask->red) 	ptr[0] = color->red;
			if (writeMask->green) 	ptr[1] = color->green;
			if (writeMask->blue) 	ptr[2] = color->blue;
		}
		
		break;
		
	case GL_RGBA4:
		{
			GLushort * ptr = (GLushort *) loc->color + loc->offset;
			GLushort mask = 
				(writeMask->red   ? 0xf000u : 0) | 
				(writeMask->green ? 0x0f00u : 0) | 
				(writeMask->blue  ? 0x00f0u : 0) | 
				(writeMask->alpha ? 0x000fu : 0);
				   
			GLushort value =
				(color->red   & 0xf0) << 8 | 
				(color->green & 0xf0) << 4 | 
				(color->blue  & 0xf0) | 
				(color->alpha) >> 4;
				
			value &= mask;
			*ptr = (*ptr & ~mask) | value;
		}
		
		break;
		
	case GL_RGB5_A1:
		{
			GLushort * ptr = (GLushort *) loc->color + loc->offset;
			GLushort mask =
				(writeMask->blue  ? 0x003eu : 0) |
				(writeMask->green ? 0x07c0u : 0) |
				(writeMask->red   ? 0xF800u : 0) |
				(writeMask->alpha ? 0x0001u : 0);
			
			GLushort value =
				(color->blue  & 0xF8) >> 2 | 
				(color->green & 0xF8) << 3 | 
				(color->red   & 0xF8) << 8 | 
				(color->alpha & 0x80) >> 7;
			
				
			value &= mask;
			*ptr = (*ptr & ~mask) | value;
		}

		break;
	
	case GL_RGBA8:
		{
			GLubyte * ptr = (GLubyte *) loc->color + loc->offset * 4;
			
			if (writeMask->red)		ptr[0] = color->red;
			if (writeMask->green)	ptr[1] = color->green;
			if (writeMask->blue) 	ptr[2] = color->blue;
			if (writeMask->alpha) 	ptr[3] = color->alpha;
		}
		
		break;
		
	case GL_RGB565_OES:
		{
			GLushort * ptr = (GLushort *) loc->color + loc->offset;

			GLushort mask =
				(writeMask->blue  ? 0x001fu : 0) |
				(writeMask->green ? 0x07e0u : 0) |
				(writeMask->red   ? 0xF800u : 0);
			
			GLushort value =
				(color->blue  & 0xF8) >> 3 | 
				(color->green & 0xFC) << 3 | 
				(color->red   & 0xF8) << 8;			
				
			value &= mask;
			*ptr = (*ptr & ~mask) | value;
		}

		break;
		
	default: 
		GLES_ASSERT(GL_FALSE);		
	}
}

void GlesReadColor(const SurfaceLoc * loc, Color * result) {
	Colorub colorub;
	
	GlesReadColorub(loc, &colorub);
	
	result->red		= colorub.red 	* (1.0f / GLES_UBYTE_MAX);
	result->green	= colorub.green	* (1.0f / GLES_UBYTE_MAX);
	result->blue	= colorub.blue 	* (1.0f / GLES_UBYTE_MAX);
	result->alpha	= colorub.alpha	* (1.0f / GLES_UBYTE_MAX);
}

GLuint GlesReadDepth(const SurfaceLoc * loc) {
	GLuint dstDepth = 0;
	
	if (loc->depth) {
		switch (loc->surface->depthFormat) {
		case GL_DEPTH_COMPONENT16:
			dstDepth = *((const GLushort *) loc->depth + loc->offset);
			break;
			
		case GL_DEPTH_COMPONENT32:
			dstDepth = *((const GLuint *) loc->depth + loc->offset);
			break; 
			
		default:
			GLES_ASSERT(GL_FALSE);
			break;
		}
	}
	
	return dstDepth;
}

void GlesWriteDepth(const SurfaceLoc * loc, GLboolean writeMask, GLuint value) {
	if (writeMask && loc->depth) {
		switch (loc->surface->depthFormat) {
		case GL_DEPTH_COMPONENT16:
			*((GLushort *) loc->depth + loc->offset) = value;
			break;
			
		case GL_DEPTH_COMPONENT32:
			*((GLuint *) loc->depth + loc->offset) = value;
			break; 
			
		default:
			GLES_ASSERT(GL_FALSE);
			break;
		}
	}
}

GLuint GlesReadStencil(const SurfaceLoc * loc) {
	GLuint dstStencil = 0;

	if (loc->stencil) {			
		switch (loc->surface->stencilFormat) {
		case GL_STENCIL_INDEX1_OES:
			dstStencil = *((const GLubyte *) loc->stencil + (loc->offset >> 3));
			
			dstStencil >>= loc->offset & 7;
			dstStencil &= 1;
			
			break;
			
		case GL_STENCIL_INDEX4_OES:
			dstStencil = *((const GLubyte *) loc->stencil + (loc->offset >> 1));
			
			if (loc->offset & 1) {
				dstStencil >>= 4;
			}
			
			dstStencil &= 0xf;
			
			break;
			
		case GL_STENCIL_INDEX8_OES:
			dstStencil = *((const GLubyte *) loc->stencil + loc->offset);
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
	}
	
	return dstStencil;
}

void GlesWriteStencil(const SurfaceLoc * loc, GLuint writeMask, GLuint value) {
	if (loc->stencil) {			
		switch (loc->surface->stencilFormat) {
		case GL_STENCIL_INDEX1_OES:
			{
				GLubyte * ptr = ((GLubyte *) loc->stencil + (loc->offset >> 3));
				GLuint bitIndex = loc->offset & 7;
				
				writeMask &= 1;
				value &= writeMask;
				
				writeMask <<= bitIndex;
				value <<= bitIndex;
				
				*ptr = (*ptr & ~writeMask) | value;
			}
			
			break;
			
		case GL_STENCIL_INDEX4_OES:
			{
				GLubyte * ptr = ((GLubyte *) loc->stencil + (loc->offset >> 1)); 

				writeMask &= 0xf;
				value &= writeMask;
				
				if (loc->offset & 1) {
					writeMask <<= 4;
					value <<= 4;
				}
				
				*ptr = (*ptr & ~writeMask) | value;
			}
						
			break;
			
		case GL_STENCIL_INDEX8_OES:
			{
				GLubyte * ptr = ((GLubyte *) loc->stencil + loc->offset);
				
				writeMask &= GLES_UBYTE_MAX;
				value &= writeMask;
				
				*ptr = (*ptr & ~writeMask) | value;
			}
			
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
	}
}

/**
 * Write a pixel to the write surface, including depth test, stencil test,
 * and any necessary blending.
 * 
 * @param state
 * 		the current GL state
 * @param loc
 * 		pointers into color, depth and stencil buffers for current pixel
 * @param color
 * 		color value to write
 * @param depth
 * 		depth value to write, in range 0 .. 1.0f
 * @param front
 * 		GL_TRUE if front-facing
 */
void GlesWritePixel(State * state, const SurfaceLoc * loc, const Color * color, GLfloat depth, GLboolean front) {

	const StencilParams * stencilParams = front ? &state->stencilFront : &state->stencilBack;
	
	Colorub srcColor, dstColor;	
	GLuint 	srcDepth, dstDepth = 0;
	GLuint 	dstStencil = 0;

	GLboolean depthTestPassed = GL_TRUE;
		
	srcColor.red 	= ColorValue(color->red, 	GLES_BITS_PER_BYTE); 
	srcColor.green 	= ColorValue(color->green, 	GLES_BITS_PER_BYTE); 
	srcColor.blue 	= ColorValue(color->blue, 	GLES_BITS_PER_BYTE); 
	srcColor.alpha 	= ColorValue(color->alpha, 	GLES_BITS_PER_BYTE); 
	
	srcDepth = GlesClampf(depth) * ((1u << loc->surface->depthBits) - 1);
	
	GlesReadColorub(loc, &dstColor);
	dstDepth = GlesReadDepth(loc);
	dstStencil = GlesReadStencil(loc);
		
	if (state->depthTestEnabled) {		
		switch (state->depthFunc) {
		case GL_NEVER:		depthTestPassed = GL_FALSE;					break;
		case GL_LESS:		depthTestPassed = (srcDepth <  dstDepth);	break;
		case GL_EQUAL:		depthTestPassed = (srcDepth == dstDepth);	break;
		case GL_LEQUAL:		depthTestPassed = (srcDepth <= dstDepth);	break;
		case GL_GREATER:	depthTestPassed = (srcDepth >  dstDepth);	break;
		case GL_NOTEQUAL:	depthTestPassed = (srcDepth != dstDepth);	break;
		case GL_GEQUAL:		depthTestPassed = (srcDepth >= dstDepth);	break;
		case GL_ALWAYS:		depthTestPassed = GL_TRUE;					break;
		default:			GLES_ASSERT(GL_FALSE);						break;
		} 
	}

	if (state->stencilTestEnabled) {
		GLboolean stencilTestPassed = GL_FALSE;
		GLuint stencilRef = stencilParams->ref & stencilParams->mask;
		GLuint stencil = dstStencil & stencilParams->mask;
		GLuint stencilMax = (1 << state->writeSurface->stencilBits) - 1;

		switch (stencilParams->func) {
		case GL_NEVER:		stencilTestPassed = GL_FALSE;					break;
		case GL_LESS:		stencilTestPassed = (stencilRef <  stencil);	break;
		case GL_EQUAL:		stencilTestPassed = (stencilRef == stencil);	break;
		case GL_LEQUAL:		stencilTestPassed = (stencilRef <= stencil);	break;
		case GL_GREATER:	stencilTestPassed = (stencilRef >  stencil);	break;
		case GL_NOTEQUAL:	stencilTestPassed = (stencilRef != stencil);	break;
		case GL_GEQUAL:		stencilTestPassed = (stencilRef >= stencil);	break;
		case GL_ALWAYS:		stencilTestPassed = GL_TRUE;					break;
		default:			GLES_ASSERT(GL_FALSE);							break;
		}

		if (!stencilTestPassed) {
			switch (stencilParams->fail) {
			case GL_KEEP:													break;
			case GL_ZERO:		stencil = 0;								break;
			case GL_REPLACE:	stencil = stencilParams->ref;				break;
			case GL_INCR:		if (stencil != stencilMax) ++stencil;		break; 
			case GL_INCR_WRAP:	++stencil;									break;
			case GL_DECR:		if (stencil) --stencil;						break;
			case GL_DECR_WRAP:	--stencil;									break;
			case GL_INVERT:		stencil = ~stencil;							break;
			default:			GLES_ASSERT(GL_FALSE);						break;
			}
			
			GlesWriteStencil(loc, stencilParams->writeMask, stencil);			
			return;
		} else if (depthTestPassed) {
			switch (stencilParams->zpass) {
			case GL_KEEP:													break;
			case GL_ZERO:		stencil = 0;								break;
			case GL_REPLACE:	stencil = stencilParams->ref;				break;
			case GL_INCR:		if (stencil != stencilMax) ++stencil;		break; 
			case GL_INCR_WRAP:	++stencil;									break;
			case GL_DECR:		if (stencil) --stencil;						break;
			case GL_DECR_WRAP:	--stencil;									break;
			case GL_INVERT:		stencil = ~stencil;							break;
			default:			GLES_ASSERT(GL_FALSE);						break;
			}
			
			GlesWriteStencil(loc, stencilParams->writeMask, stencil);			
		} else {
			switch (stencilParams->zfail) {
			case GL_KEEP:													break;
			case GL_ZERO:		stencil = 0;								break;
			case GL_REPLACE:	stencil = stencilParams->ref;				break;
			case GL_INCR:		if (stencil != stencilMax) ++stencil;		break; 
			case GL_INCR_WRAP:	++stencil;									break;
			case GL_DECR:		if (stencil) --stencil;						break;
			case GL_DECR_WRAP:	--stencil;									break;
			case GL_INVERT:		stencil = ~stencil;							break;
			default:			GLES_ASSERT(GL_FALSE);						break;
			}
			
			GlesWriteStencil(loc, stencilParams->writeMask, stencil);			
			return;
		}
	} else if (!depthTestPassed) {
		return;
	}

	GlesWriteDepth(loc, state->depthMask, srcDepth);
		
	if (state->blendEnabled) {
		Colorub	srcFactor, dstFactor;
		
		switch (state->blendFuncSrcRGB) {
		case GL_ZERO:
			srcFactor.red 	= 0;
			srcFactor.green = 0;
			srcFactor.blue 	= 0;
			break;
			
		case GL_ONE:
			srcFactor.red 	= GLES_UBYTE_MAX;
			srcFactor.green = GLES_UBYTE_MAX;
			srcFactor.blue 	= GLES_UBYTE_MAX;
			break;
			
		case GL_SRC_COLOR:
			srcFactor.red 	= srcColor.red;
			srcFactor.green = srcColor.green;
			srcFactor.blue 	= srcColor.blue;
			break;
			
		case GL_ONE_MINUS_SRC_COLOR:
			srcFactor.red 	= GLES_UBYTE_MAX - srcColor.red;
			srcFactor.green = GLES_UBYTE_MAX - srcColor.green;
			srcFactor.blue 	= GLES_UBYTE_MAX - srcColor.blue;
			break;
			
		case GL_DST_COLOR:
			srcFactor.red 	= dstColor.red;
			srcFactor.green = dstColor.green;
			srcFactor.blue 	= dstColor.blue;
			break;
			
		case GL_ONE_MINUS_DST_COLOR:
			srcFactor.red 	= GLES_UBYTE_MAX - dstColor.red;
			srcFactor.green = GLES_UBYTE_MAX - dstColor.green;
			srcFactor.blue 	= GLES_UBYTE_MAX - dstColor.blue;
			break;
			
		case GL_SRC_ALPHA:
			srcFactor.red 	= 
			srcFactor.green = 
			srcFactor.blue 	= srcColor.alpha;
			break;
			
		case GL_ONE_MINUS_SRC_ALPHA:
			srcFactor.red 	= 
			srcFactor.green = 
			srcFactor.blue 	= GLES_UBYTE_MAX - srcColor.alpha;
			break;
			
		case GL_DST_ALPHA:
			srcFactor.red 	= 
			srcFactor.green = 
			srcFactor.blue 	= dstColor.alpha;
			break;
			
		case GL_ONE_MINUS_DST_ALPHA:
			srcFactor.red 	= 
			srcFactor.green = 
			srcFactor.blue 	= GLES_UBYTE_MAX - dstColor.alpha;
			break;
			
		case GL_CONSTANT_COLOR:
			srcFactor.red 	= ColorValue(state->blendColor.red, GLES_BITS_PER_BYTE);
			srcFactor.green = ColorValue(state->blendColor.green, GLES_BITS_PER_BYTE);
			srcFactor.blue 	= ColorValue(state->blendColor.blue, GLES_BITS_PER_BYTE);
			break;
			
		case GL_ONE_MINUS_CONSTANT_COLOR:
			srcFactor.red 	= GLES_UBYTE_MAX - ColorValue(state->blendColor.red, GLES_BITS_PER_BYTE);
			srcFactor.green = GLES_UBYTE_MAX - ColorValue(state->blendColor.green, GLES_BITS_PER_BYTE);
			srcFactor.blue 	= GLES_UBYTE_MAX - ColorValue(state->blendColor.blue, GLES_BITS_PER_BYTE);
			break;
			
		case GL_CONSTANT_ALPHA:
			srcFactor.red 	= 
			srcFactor.green = 
			srcFactor.blue 	= ColorValue(state->blendColor.alpha, GLES_BITS_PER_BYTE);
			break;
			
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			srcFactor.red 	=
			srcFactor.green =
			srcFactor.blue 	= GLES_UBYTE_MAX - ColorValue(state->blendColor.alpha, GLES_BITS_PER_BYTE);
			break;
			
		case GL_SRC_ALPHA_SATURATE:
			srcFactor.red 	= 
			srcFactor.green = 
			srcFactor.blue 	= Min(srcColor.alpha, GLES_UBYTE_MAX - dstColor.alpha);
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}

		switch (state->blendFuncSrcAlpha) {
		case GL_ZERO:
			srcFactor.alpha	= 0;
			break;
			
		case GL_ONE:
			srcFactor.alpha	= GLES_UBYTE_MAX;
			break;
			
		case GL_SRC_COLOR:
		case GL_SRC_ALPHA:
			srcFactor.alpha = srcColor.alpha;
			break;
			
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_ONE_MINUS_SRC_ALPHA:
			srcFactor.alpha	= GLES_UBYTE_MAX - srcColor.alpha;
			break;
						
		case GL_DST_COLOR:
		case GL_DST_ALPHA:
			srcFactor.alpha = dstColor.alpha;
			break;
			
		case GL_ONE_MINUS_DST_COLOR:
		case GL_ONE_MINUS_DST_ALPHA:
			srcFactor.alpha	= GLES_UBYTE_MAX - dstColor.alpha;
			break;
			
		case GL_CONSTANT_COLOR:
		case GL_CONSTANT_ALPHA:
			srcFactor.alpha	= ColorValue(state->blendColor.alpha, GLES_BITS_PER_BYTE);
			break;
			
		case GL_ONE_MINUS_CONSTANT_COLOR:
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			srcFactor.alpha = GLES_UBYTE_MAX - ColorValue(state->blendColor.alpha, GLES_BITS_PER_BYTE);
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
		
		switch (state->blendFuncDstRGB) {
		case GL_ZERO:
			dstFactor.red 	= 0;
			dstFactor.green = 0;
			dstFactor.blue 	= 0;
			break;
			
		case GL_ONE:
			dstFactor.red 	= GLES_UBYTE_MAX;
			dstFactor.green = GLES_UBYTE_MAX;
			dstFactor.blue 	= GLES_UBYTE_MAX;
			break;
			
		case GL_SRC_COLOR:
			dstFactor.red 	= srcColor.red;
			dstFactor.green = srcColor.green;
			dstFactor.blue 	= srcColor.blue;
			break;
			
		case GL_ONE_MINUS_SRC_COLOR:
			dstFactor.red 	= GLES_UBYTE_MAX - srcColor.red;
			dstFactor.green = GLES_UBYTE_MAX - srcColor.green;
			dstFactor.blue 	= GLES_UBYTE_MAX - srcColor.blue;
			break;
			
		case GL_DST_COLOR:
			dstFactor.red 	= dstColor.red;
			dstFactor.green = dstColor.green;
			dstFactor.blue 	= dstColor.blue;
			break;
			
		case GL_ONE_MINUS_DST_COLOR:
			dstFactor.red 	= GLES_UBYTE_MAX - dstColor.red;
			dstFactor.green = GLES_UBYTE_MAX - dstColor.green;
			dstFactor.blue 	= GLES_UBYTE_MAX - dstColor.blue;
			break;
			
		case GL_SRC_ALPHA:
			dstFactor.red 	= 
			dstFactor.green = 
			dstFactor.blue 	= srcColor.alpha;
			break;
			
		case GL_ONE_MINUS_SRC_ALPHA:
			dstFactor.red 	= 
			dstFactor.green = 
			dstFactor.blue 	= GLES_UBYTE_MAX - srcColor.alpha;
			break;
			
		case GL_DST_ALPHA:
			dstFactor.red 	= 
			dstFactor.green = 
			dstFactor.blue 	= dstColor.alpha;
			break;
			
		case GL_ONE_MINUS_DST_ALPHA:
			dstFactor.red 	= 
			dstFactor.green = 
			dstFactor.blue 	= GLES_UBYTE_MAX - dstColor.alpha;
			break;
			
		case GL_CONSTANT_COLOR:
			dstFactor.red 	= ColorValue(state->blendColor.red, GLES_BITS_PER_BYTE);
			dstFactor.green = ColorValue(state->blendColor.green, GLES_BITS_PER_BYTE);
			dstFactor.blue 	= ColorValue(state->blendColor.blue, GLES_BITS_PER_BYTE);
			break;
			
		case GL_ONE_MINUS_CONSTANT_COLOR:
			dstFactor.red 	= GLES_UBYTE_MAX - ColorValue(state->blendColor.red, GLES_BITS_PER_BYTE);
			dstFactor.green = GLES_UBYTE_MAX - ColorValue(state->blendColor.green, GLES_BITS_PER_BYTE);
			dstFactor.blue 	= GLES_UBYTE_MAX - ColorValue(state->blendColor.blue, GLES_BITS_PER_BYTE);
			break;
			
		case GL_CONSTANT_ALPHA:
			dstFactor.red 	= 
			dstFactor.green = 
			dstFactor.blue 	= ColorValue(state->blendColor.alpha, GLES_BITS_PER_BYTE);
			break;
			
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			dstFactor.red 	=
			dstFactor.green =
			dstFactor.blue 	= GLES_UBYTE_MAX - ColorValue(state->blendColor.alpha, GLES_BITS_PER_BYTE);
			break;
			
		case GL_SRC_ALPHA_SATURATE:
			dstFactor.red 	= 
			dstFactor.green = 
			dstFactor.blue 	= Min(srcColor.alpha, GLES_UBYTE_MAX - dstColor.alpha);
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}

		switch (state->blendFuncDstAlpha) {
		case GL_ZERO:
			dstFactor.alpha	= 0;
			break;
			
		case GL_ONE:
			dstFactor.alpha	= GLES_UBYTE_MAX;
			break;
			
		case GL_SRC_COLOR:
		case GL_SRC_ALPHA:
			dstFactor.alpha = srcColor.alpha;
			break;
			
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_ONE_MINUS_SRC_ALPHA:
			dstFactor.alpha	= GLES_UBYTE_MAX - srcColor.alpha;
			break;
						
		case GL_DST_COLOR:
		case GL_DST_ALPHA:
			dstFactor.alpha = dstColor.alpha;
			break;
			
		case GL_ONE_MINUS_DST_COLOR:
		case GL_ONE_MINUS_DST_ALPHA:
			dstFactor.alpha	= GLES_UBYTE_MAX - dstColor.alpha;
			break;
			
		case GL_CONSTANT_COLOR:
		case GL_CONSTANT_ALPHA:
			dstFactor.alpha	= ColorValue(state->blendColor.alpha, GLES_BITS_PER_BYTE);
			break;
			
		case GL_ONE_MINUS_CONSTANT_COLOR:
		case GL_ONE_MINUS_CONSTANT_ALPHA:
			dstFactor.alpha = GLES_UBYTE_MAX - ColorValue(state->blendColor.alpha, GLES_BITS_PER_BYTE);
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
		
		switch (state->blendEqnModeRBG) {
		case GL_FUNC_ADD:
			srcColor.red	= ClampUbyte(MulUbyte(srcFactor.red, srcColor.red) + 
										 MulUbyte(dstFactor.red, dstColor.red));
			srcColor.green	= ClampUbyte(MulUbyte(srcFactor.green, srcColor.green) + 
										 MulUbyte(dstFactor.green, dstColor.green));
			srcColor.blue	= ClampUbyte(MulUbyte(srcFactor.blue, srcColor.blue) + 
										 MulUbyte(dstFactor.blue, dstColor.blue));
			break;
			
		case GL_FUNC_SUBTRACT:
			srcColor.red	= ClampUbyte(MulUbyte(srcFactor.red, srcColor.red) -
										 MulUbyte(dstFactor.red, dstColor.red));
			srcColor.green	= ClampUbyte(MulUbyte(srcFactor.green, srcColor.green) - 
										 MulUbyte(dstFactor.green, dstColor.green));
			srcColor.blue	= ClampUbyte(MulUbyte(srcFactor.blue, srcColor.blue) - 
										 MulUbyte(dstFactor.blue, dstColor.blue));
			break;
			
		case GL_FUNC_REVERSE_SUBTRACT:
			srcColor.red	= ClampUbyte(MulUbyte(dstFactor.red, dstColor.red) -
										 MulUbyte(srcFactor.red, srcColor.red));
			srcColor.green	= ClampUbyte(MulUbyte(dstFactor.green, dstColor.green) -
										 MulUbyte(srcFactor.green, srcColor.green));
			srcColor.blue	= ClampUbyte(MulUbyte(dstFactor.blue, dstColor.blue) -
										 MulUbyte(srcFactor.blue, srcColor.blue));
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
		
		switch (state->blendEqnModeAlpha) {
		case GL_FUNC_ADD:
			srcColor.alpha	= ClampUbyte(MulUbyte(srcFactor.alpha, srcColor.alpha) + 
										 MulUbyte(dstFactor.alpha, dstColor.alpha));
			break;
			
		case GL_FUNC_SUBTRACT:
			srcColor.alpha	= ClampUbyte(MulUbyte(srcFactor.alpha, srcColor.alpha) -
										 MulUbyte(dstFactor.alpha, dstColor.alpha));
			break;
			
		case GL_FUNC_REVERSE_SUBTRACT:
			srcColor.alpha	= ClampUbyte(MulUbyte(dstFactor.alpha, dstColor.alpha) -
										 MulUbyte(srcFactor.alpha, srcColor.alpha));
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
	}

	GlesWriteColorub(loc, &state->colorMask, &srcColor);
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	State * state = GLES_GET_STATE();

	state->clearColor.red	= GlesClampf(red);
	state->clearColor.green	= GlesClampf(green);
	state->clearColor.blue	= GlesClampf(blue);
	state->clearColor.alpha = GlesClampf(alpha);
}

GL_API void GL_APIENTRY glClearColorx (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) {
	glClearColor(GlesFloatx(red), GlesFloatx(green), GlesFloatx(blue), GlesFloatx(alpha));
}

GL_API void GL_APIENTRY glClearDepthf (GLclampf depth) {
	State * state = GLES_GET_STATE();
	state->clearDepth = GlesClampf(depth) * ((1u << state->writeSurface->depthBits) - 1);
}

GL_API void GL_APIENTRY glClearDepthx (GLclampx depth) {
	glClearDepthf(GlesFloatx(depth));
}

GL_API void GL_APIENTRY glClearStencil (GLint s) {
	State * state = GLES_GET_STATE();
	state->clearStencil = s;
}

GL_API void GL_APIENTRY glClear (GLbitfield mask) {
	State * state = GLES_GET_STATE();
	Surface * surface = state->writeSurface;
	GLuint scanlines;

	surface->vtbl->lock(surface);
	GlesInitRasterRect(state);
	
	if (!state->rasterRect.width || !state->rasterRect.height) {
		/* empty rectangle? */
		surface->vtbl->unlock(surface);
		return;
	}
		
	scanlines = state->rasterRect.height;
	
	if (mask & GL_COLOR_BUFFER_BIT) {
		/* clear color buffer */
		
		GLubyte * start = ((GLubyte *) surface->colorBuffer) + 
			surface->colorPitch * state->rasterRect.y;
			
		GLubyte r = ColorValue(state->clearColor.red,   GLES_BITS_PER_BYTE);
		GLubyte g = ColorValue(state->clearColor.green, GLES_BITS_PER_BYTE);
		GLubyte b = ColorValue(state->clearColor.blue,  GLES_BITS_PER_BYTE);
		GLubyte a = ColorValue(state->clearColor.alpha, GLES_BITS_PER_BYTE);
		
		GLuint colorWord, colorMask;
		
		if (state->colorMask.red & state->colorMask.green & 
		 	state->colorMask.blue & state->colorMask.alpha) {
		 	/* simple case; just write new value */
		 	
		 	switch (surface->colorFormat) {
		 	case GL_RGB8:
		 		start += state->rasterRect.x * (sizeof(GLubyte) * 3);
		 		
		 		do {
		 			GLsizei pixels = state->rasterRect.width;
		 			GLubyte * ptr = start;
		 			
		 			do {
		 				/* BUGBUG: Big vs. little endian machines? */
		 				*ptr++ = r;
		 				*ptr++ = g;
		 				*ptr++ = b;
		 			} while (--pixels);
		 			
		 			start += surface->colorPitch;
		 		} while (--scanlines);
		 		
		 		break;
		 	
		 	case GL_RGBA8:
		 		start += state->rasterRect.x * (sizeof(GLubyte) * 4);
		 		
		 		do {
		 			GLsizei pixels = state->rasterRect.width;
		 			GLubyte * ptr = start;
		 			
		 			do {
		 				/* BUGBUG: Big vs. little endian machines? */
		 				*ptr++ = r;
		 				*ptr++ = g;
		 				*ptr++ = b;
		 				*ptr++ = a;
		 			} while (--pixels);
		 			
		 			start += surface->colorPitch;
		 		} while (--scanlines);
		 		
		 		break;
		 	
		 	case GL_RGB565_OES:
		 	case GL_RGB5_A1:
		 	case GL_RGBA4:
		 		start += state->rasterRect.x * sizeof(GLushort);
		 		colorWord = ColorWord(surface->colorFormat, r, g, b, a);
		 		
		 		do {
		 			GLsizei pixels = state->rasterRect.width;
		 			GLushort * ptr = (GLushort *) start;
		 			
		 			do {
		 				*ptr++ = colorWord;
		 			} while (--pixels);
		 			
		 			start += surface->colorPitch;
		 		} while (--scanlines);
		 		
		 		break;
		 		
		 	default:
		 		GLES_ASSERT(GL_FALSE);
		 	}
		} else if (state->colorMask.red | state->colorMask.green | 
		 	state->colorMask.blue | state->colorMask.alpha) {
		 	/* more complicated case; need to selectively merge new values into buffer */
		 	
		 	switch (surface->colorFormat) {
		 	case GL_RGB8:
		 		start += state->rasterRect.x * (sizeof(GLubyte) * 3);
		 		
		 		do {
		 			GLsizei pixels = state->rasterRect.width;
		 			GLubyte * ptr = start;
		 			
		 			do {
		 				/* BUGBUG: Big vs. little endian machines? */
		 				if (state->colorMask.red) 	*ptr++ = r; else ++ptr;
		 				if (state->colorMask.green) *ptr++ = g; else ++ptr;
		 				if (state->colorMask.blue) 	*ptr++ = b; else ++ptr;
		 			} while (--pixels);
		 			
		 			start += surface->colorPitch;
		 		} while (--scanlines);
		 				 		
		 		break;
		 		
		 	case GL_RGBA8:
		 		start += state->rasterRect.x * (sizeof(GLubyte) * 4);
		 		
		 		do {
		 			GLsizei pixels = state->rasterRect.width;
		 			GLubyte * ptr = start;
		 			
		 			do {
		 				/* BUGBUG: Big vs. little endian machines? */
		 				if (state->colorMask.red) 	*ptr++ = r; else ++ptr;
		 				if (state->colorMask.green) *ptr++ = g; else ++ptr;
		 				if (state->colorMask.blue) 	*ptr++ = b;	else ++ptr;
		 				if (state->colorMask.alpha) *ptr++ = a; else ++ptr;
		 			} while (--pixels);
		 			start += surface->colorPitch;
		 		} while (--scanlines);
		 		
		 		break;
		 		
		 	case GL_RGB565_OES:
		 	case GL_RGB5_A1:
		 	case GL_RGBA4:
		 		start += state->rasterRect.x * sizeof(GLushort);
		 		colorMask = ColorWriteMask(surface->colorFormat, &state->colorMask);
		 		colorWord = ColorWord(surface->colorFormat, r, g, b, a) & colorMask;
		 		colorMask = ~colorMask;
		 		
		 		do {
		 			GLsizei pixels = state->rasterRect.width;
		 			GLushort * ptr = (GLushort *) start;
		 			
		 			do {
		 				*ptr = (*ptr & colorMask) | colorWord;
		 				++ptr;
		 			} while (--pixels);
		 			
		 			start += surface->colorPitch;
		 		} while (--scanlines);
		 		
		 		
		 	default:
		 		GLES_ASSERT(GL_FALSE);
		 	}
		} /* else no operation */
	}
	
	if ((mask & GL_DEPTH_BUFFER_BIT) && surface->depthBuffer && state->depthMask) {
		/* clear depth buffer */

		GLubyte * start = ((GLubyte *) surface->depthBuffer) + 
			surface->depthPitch * state->rasterRect.y;
			
		switch (surface->depthFormat) {
		case GL_DEPTH_COMPONENT16:	
			start += state->rasterRect.x * sizeof(GLushort);
		 		
	 		do {
	 			GLsizei pixels = state->rasterRect.width;
	 			GLushort * ptr = (GLushort *) start;
	 			
	 			do {
	 				*ptr++ = state->clearDepth;
	 			} while (--pixels);
	 			
	 			start += surface->depthPitch;
	 		} while (--scanlines);
	 		
			break;
			
		case GL_DEPTH_COMPONENT32:
			start += state->rasterRect.x * sizeof(GLuint);
			
	 		do {
	 			GLsizei pixels = state->rasterRect.width;
	 			GLuint * ptr = (GLuint *) start;
	 			
	 			do {
	 				*ptr++ = state->clearDepth;
	 			} while (--pixels);
	 				 			
	 			start += surface->depthPitch;
	 		} while (--scanlines);
	 		
			break; 
			
		case GL_DEPTH_COMPONENT24:
		default:	
			GLES_ASSERT(GL_FALSE);
		}		
	}
	
	if ((mask & GL_STENCIL_BUFFER_BIT) && surface->stencilBuffer && state->stencilFront.writeMask) {
		/* clear stencil buffer */
		
		GLubyte * start = ((GLubyte *) surface->stencilBuffer) + 
			surface->stencilPitch * state->rasterRect.y;
			
		GLubyte subIndex;
		GLubyte stencil, stencilMask;
		
		switch (surface->stencilFormat) {
		case GL_STENCIL_INDEX1_OES:	
		 	start += (state->rasterRect.x / 8) * sizeof(GLubyte);
		 	stencil = (state->clearStencil & 1) ? GLES_UBYTE_MAX : 0;
		 	stencilMask = GLES_UBYTE_MAX;

			if (state->rasterRect.x / 8 == (state->rasterRect.x + state->rasterRect.width - 1) / 8) {
				/* just write a single byte per scanline */
				if (state->rasterRect.x & 7) {
					stencilMask &= ~((1 << ((state->rasterRect.x & 7) - 1)) - 1);
				}
				
				if ((state->rasterRect.x + state->rasterRect.width) & 7) {
					stencilMask &= (1 << (((state->rasterRect.x  + state->rasterRect.width) & 7) - 1)) - 1;
				}
				
				stencil &= stencilMask;
				stencilMask = ~stencilMask;
								
		 		do {
		 			*start = (*start & stencilMask) | stencil;
		 			start += surface->stencilPitch;
		 		} while (--scanlines);
			} else {
				GLubyte beginMask = 0, endMask = 0;
	 			GLsizei pixels = state->rasterRect.width;
				
				if (state->rasterRect.x & 7) {
					beginMask = ~((1 << ((state->rasterRect.x & 7) - 1)) - 1);
					--pixels;
				}
				
				if ((state->rasterRect.x + state->rasterRect.width) & 7) {
					endMask = (1 << (((state->rasterRect.x  + state->rasterRect.width) & 7) - 1)) - 1;
					--pixels;
				}
				
		 		do {
		 			GLubyte * ptr = (GLubyte *) start;

					if (beginMask) {
						*ptr = (*ptr & ~beginMask) | (stencil & beginMask);
						++ptr;
					}
					
					while (pixels--) {
						*ptr++ = stencil;
					}
					
					if (endMask) {
						*ptr = (*ptr & ~endMask) | (stencil & endMask);
					}		 			
		 			
		 			start += surface->stencilPitch;
		 		} while (--scanlines);
			}	 	
	 		
			break;
			
		case GL_STENCIL_INDEX4_OES:	
		 	start += (state->rasterRect.x / 2) * sizeof(GLubyte);
		 	stencil = (state->clearStencil & 0xf) | (state->clearStencil & 0xf) << 4;
					
	 		do {
	 			GLubyte * ptr = start;
	 			GLsizei pixels = state->rasterRect.width;
	 			subIndex = state->rasterRect.x & 1;
	 			
	 			if (subIndex) {
	 				/* partial byte first */
	 				*ptr = (*ptr & 0xf) | (0xf0 & stencil);
	 				++ptr;
	 				--subIndex;
	 			}
	 			
	 			while (pixels >> 1) {
	 				*ptr++ = stencil;
	 				pixels -= 2;
	 			}
	 			
	 			if (pixels) {
	 				/* partial byte at end */
	 				*ptr = (*ptr & 0xf0) | (0xf & stencil);
	 			}
	 			 
	 			start += surface->stencilPitch;
	 		} while (--scanlines);
	 		
			break;
			
		case GL_STENCIL_INDEX8_OES:	
		 	start += state->rasterRect.x * sizeof(GLubyte);
			
	 		do {
	 			GLubyte * ptr = start;
	 			GLsizei pixels = state->rasterRect.width;
	 			
	 			do {
	 				*ptr++ = state->clearStencil;
	 			} while (--pixels);
	 				 			
	 			start += surface->stencilPitch;
	 		} while (--scanlines);
	 		
			break;
			
		default:
			GLES_ASSERT(GL_FALSE);
		}
	}
	
	surface->vtbl->unlock(surface);
}
