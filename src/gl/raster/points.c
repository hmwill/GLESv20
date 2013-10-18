/*
** ==========================================================================
**
** $Id: points.c 60 2007-09-18 01:16:07Z hmwill $		
**
** Rasterization of point sprites
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

/**
 * Rasterization of a point sprite centered at the given location
 * 
 * @param state
 * 		the GL state defining rasterization settings
 * 
 * @param center
 * 		the center of the point sprite
 * 
 * @param pointSize
 * 		the size of the sprite
 */
void GlesRasterPointSprite(State * state, RasterVertex * center, GLfloat pointSize) {
	
	GLint minX = GlesRasterValue(center->screen.x - pointSize / 2.0f);
	GLint minY = GlesRasterValue(center->screen.y - pointSize / 2.0f);
	GLint maxX = GlesRasterValue(center->screen.x + pointSize / 2.0f);
	GLint maxY = GlesRasterValue(center->screen.y + pointSize / 2.0f);
	GLint centerMinX = (minX + HALF_PIXEL) & ~SUBPIXEL_MASK;
	GLint centerMinY = (minY + HALF_PIXEL) & ~SUBPIXEL_MASK;
	GLint x, y;
	
	GLfloat px, py;
	GLfloat pDelta = 1.0f / pointSize;
	GLfloat pxStart = (minX - centerMinX) * pDelta * GlesLdexpf(1.0f, -GLES_SUBPIXEL_BITS);
	GLfloat pyStart = (minY - centerMinY) * pDelta * GlesLdexpf(1.0f, -GLES_SUBPIXEL_BITS);
		
	SurfaceLoc loc;
	 
	union { Vec4f vec4f; Color color; } result;

	state->fragContext.varying = center->varyingData;
	state->fragContext.result = &result.vec4f;

	GlesInitSurfaceLoc(state->writeSurface, &loc, 
					   centerMinX >> GLES_SUBPIXEL_BITS, centerMinY >> GLES_SUBPIXEL_BITS);
		
	for (y = centerMinY, py = pyStart; y < maxY; y += 1 << GLES_SUBPIXEL_BITS, py += pDelta) {
		for (x = centerMinX, px = pxStart; x < maxX; x += 1 << GLES_SUBPIXEL_BITS, px += pDelta) {

			// TODO: pixel onwership / scissor test
			if (GlesFragmentProgram(state->programs[state->program].executable)(&state->fragContext)) {            	
        		GlesWritePixel(state, &loc, &result.color, center->screen.z, GL_TRUE);
			}				
			
			GlesStepSurfaceLoc(&loc, 1, 0);
		}
		
		GlesStepSurfaceLoc(&loc, -(maxX - centerMinX) >> GLES_SUBPIXEL_BITS, 1);
	}
}

