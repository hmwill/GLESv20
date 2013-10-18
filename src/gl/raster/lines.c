/*
** ==========================================================================
**
** $Id: lines.c 60 2007-09-18 01:16:07Z hmwill $		
**
** Rasterization of lines
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
#include "raster/raster.h"
#include "frontend/linker.h"


#define HALF_PIXEL (1 << (GLES_SUBPIXEL_BITS - 1))
#define SUBPIXEL_MASK ((1 << GLES_SUBPIXEL_BITS) - 1)

typedef struct Interpolation {
	GLfloat	dx, dy, value;
} Interpolation;

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

/**
 * Compute the absolute value
 */
static GLES_INLINE GLint Abs(GLint value) {
	return value >= 0 ? value : -value;
}

/**
 * Compute the signature function
 */
static GLES_INLINE GLint Sign(GLint value) {
	return value > 0 ? 1 : value < 0 ? -1 : 0;
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

/**
 * Rasterization of an aliased line defined by its end-vertices
 * 
 * @param state
 * 		the GL state defining rasterization settings
 * 
 * @param a
 * 		first vertex
 * 
 * @param b 
 * 		second vertex
 * 
 * This function implements a sub-pixel corrected version of the classical
 * Bresenham algorithm. For further discussion, please refer to
 * 
 * Lathrop, O., Kirk, D., and Voorhies, D. (1990) "Accurate Rendering by Subpixel Addressing",
 * IEEE Computer Graphics and Applications, Vol. 10(5), pp. 45-53
 * 
 * Rogers, D. F. (1998) "Procedural Elements for Computer Graphics", 2nd ed., McGraw-Hill
 */
void GlesRasterLine(State * state, RasterVertex * a, RasterVertex * b) {
	
	GLint x = GlesRasterValue(a->screen.x);
	GLint y = GlesRasterValue(a->screen.y);
	GLint x2 = GlesRasterValue(b->screen.x);
	GLint y2 = GlesRasterValue(b->screen.y);
	
	GLint deltaX = Abs(x2 - x);
	GLint deltaY = Abs(y2 - y);
	GLint signX = Sign(x2 - x);
	GLint signY = Sign(y2 - y);
	
	GLfloat denominator = deltaX * deltaX + deltaY * deltaY;
	GLsizei index;
	
	Interpolation invW, depth, varying[GLES_MAX_VARYING_FLOATS];
	GLfloat vars[GLES_MAX_VARYING_FLOATS];
	
	SurfaceLoc loc;
	 
	union { Vec4f vec4f; Color color; } result;

	state->fragContext.varying = vars;
	state->fragContext.result = &result.vec4f;

	if (deltaX >= deltaY) {
		GLint prestep = (x & SUBPIXEL_MASK) - HALF_PIXEL;
		GLint error = deltaY * 2 - deltaX;	/* the formula assumes pixel centers */
		GLint count = (deltaX - prestep) >> GLES_SUBPIXEL_BITS;
		GLfloat fractionX = (GLfloat) deltaX / denominator;
		GLfloat fractionY = 1.0f - fractionX;

		/* set up interpolation */
		
		invW.value = a->screen.w;
		invW.dx = (b->screen.w - a->screen.w) * fractionX;
		invW.dy = (b->screen.w - a->screen.w) * fractionY;
		
		depth.value = a->screen.z;
		depth.dx = (b->screen.z - a->screen.z) * fractionX;
		depth.dy = (b->screen.z - a->screen.z) * fractionY;
		
		for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
			varying[index].value = a->varyingData[index];
			varying[index].dx = (b->varyingData[index] * b->screen.w - a->varyingData[index] * a->screen.w) * fractionX;
			varying[index].dy = (b->varyingData[index] * b->screen.w - a->varyingData[index] * a->screen.w) * fractionY;
		}
		
		/* adjust the error for pre-step */
		x += prestep;
		error += (prestep * deltaY * ((1 << (GLES_SUBPIXEL_BITS + 1)) + 1)) >> GLES_SUBPIXEL_BITS;
		
		/* how to handle pre-stepping concerning interpolation values? */
		
		if (error > 0) {
			y += (signY << GLES_SUBPIXEL_BITS);				
			error -= deltaX * 2;
				
			invW.value += invW.dy;
			depth.value += depth.dy;
			
			for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
				varying[index].value += varying[index].dy;
			}
		}
		
		GlesInitSurfaceLoc(state->writeSurface, &loc, 
						   x >> GLES_SUBPIXEL_BITS, y >> GLES_SUBPIXEL_BITS);

		while (--count) {
        	GLfloat w = 1.0f / invW.value;
        	
        	for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
        		vars[index] = varying[index].value * w;
        	}

			// TODO: pixel onwership / scissor test
			if (GlesFragmentProgram(state->programs[state->program].executable)(&state->fragContext)) {            	
        		GlesWritePixel(state, &loc, &result.color, depth.value, GL_TRUE);
			}				
			
			if (error > 0) {
				y += (signY << GLES_SUBPIXEL_BITS);				
				error -= deltaX * 2;
				
				invW.value += invW.dy;
				depth.value += depth.dy;
				
				for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
					varying[index].value += varying[index].dy;
				}
				
				GlesStepSurfaceLoc(&loc, 0, signY);
			}
			
			x += (signX << GLES_SUBPIXEL_BITS);
			error += deltaY * 2;

			invW.value += invW.dx;
			depth.value += depth.dx;
			
			for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
				varying[index].value += varying[index].dx;
			}

			GlesStepSurfaceLoc(&loc, signX, 0);
		}
	} else {
		GLint prestep = (y & SUBPIXEL_MASK) - HALF_PIXEL;
		GLint error = deltaX * 2 - deltaY;	
		GLint count = (deltaY - prestep) >> GLES_SUBPIXEL_BITS;
		GLfloat fractionY = (GLfloat) deltaY / denominator;
		GLfloat fractionX = 1.0f - fractionY;

		/* set up interpolation */
		
		invW.value = a->screen.w;
		invW.dx = (b->screen.w - a->screen.w) * fractionX;
		invW.dy = (b->screen.w - a->screen.w) * fractionY;
		
		depth.value = a->screen.z;
		depth.dx = (b->screen.z - a->screen.z) * fractionX;
		depth.dy = (b->screen.z - a->screen.z) * fractionY;
		
		for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
			varying[index].value = a->varyingData[index];
			varying[index].dx = (b->varyingData[index] * b->screen.w - a->varyingData[index] * a->screen.w) * fractionX;
			varying[index].dy = (b->varyingData[index] * b->screen.w - a->varyingData[index] * a->screen.w) * fractionY;
		}
		
		/* adjust the error for pre-step */
		y += prestep;
		error += (prestep * deltaX * ((1 << (GLES_SUBPIXEL_BITS + 1)) + 1)) >> GLES_SUBPIXEL_BITS;
		
		/* how to handle pre-stepping concerning interpolation values? */
		
		if (error > 0) {
			x += (signX << GLES_SUBPIXEL_BITS);
			error -= deltaY * 2;

			invW.value += invW.dx;
			depth.value += depth.dx;
			
			for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
				varying[index].value += varying[index].dx;
			}
		}
		
		GlesInitSurfaceLoc(state->writeSurface, &loc, 
						   x >> GLES_SUBPIXEL_BITS, y >> GLES_SUBPIXEL_BITS);

		while (--count) {
        	GLfloat w = 1.0f / invW.value;
        	
        	for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
        		vars[index] = varying[index].value * w;
        	}
        	
			// TODO: pixel onwership / scissor test
			if (GlesFragmentProgram(state->programs[state->program].executable)(&state->fragContext)) {            	
        		GlesWritePixel(state, &loc, &result.color, depth.value, GL_TRUE);
			}				
			
			if (error > 0) {
				x += (signX << GLES_SUBPIXEL_BITS);
				error -= deltaY * 2;

				invW.value += invW.dx;
				depth.value += depth.dx;
				
				for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
					varying[index].value += varying[index].dx;
				}
				
				GlesStepSurfaceLoc(&loc, signX, 0);
			}
			
			y += (signY << GLES_SUBPIXEL_BITS);
			error += deltaX * 2;

			invW.value += invW.dy;
			depth.value += depth.dy;
			
			for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
				varying[index].value += varying[index].dy;
			}

			GlesStepSurfaceLoc(&loc, 0, signY);
		}
	}
}
