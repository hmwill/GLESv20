/*
** ==========================================================================
**
** $Id: triangles.c 60 2007-09-18 01:16:07Z hmwill $		
**
** Rasterization of triangles
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

#define SUBPIXEL_MASK ((1 << GLES_SUBPIXEL_BITS) - 1)
#define HALF_PIXEL (1 << (GLES_SUBPIXEL_BITS - 1))

static GLES_INLINE GLint Min(GLint a, GLint b, GLint c) {
	return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

static GLES_INLINE GLint Max(GLint a, GLint b, GLint c) {
	return a > b ? (a > c ? a : c) : (b > c ? b : c);
}

typedef struct Interpolation {
	GLfloat	dx, dy, value;
} Interpolation;

/**
 * Rasterization of a triangle defined by the 3 given vertices
 * 
 * @param state
 * 		the GL state defining rasterization settings
 * 
 * @param a
 * 		first vertex
 * @param b 
 * 		second vertex
 * @param c
 * 		third vertex
 * @param backFacing
 * 		if true, this is a back facing triangle
 */
void GlesRasterTriangle(State * state, RasterVertex * a, RasterVertex * b, RasterVertex * c,
						GLboolean backFacing) {
					
	// 28.4 fixed-point coordinates
    GLint y1 = GlesRasterValue(a->screen.y);
    GLint y2 = GlesRasterValue(b->screen.y);
    GLint y3 = GlesRasterValue(c->screen.y);

    GLint x1 = GlesRasterValue(a->screen.x);
    GLint x2 = GlesRasterValue(b->screen.x);
    GLint x3 = GlesRasterValue(c->screen.x);

    // Deltas
    GLint dx12 = x1 - x2, dx23 = x2 - x3, dx31 = x3 - x1;
    GLint dy12 = y1 - y2, dy23 = y2 - y3, dy31 = y3 - y1;

	GLfloat subpixelScale = GlesLdexpf(1.0f, -GLES_SUBPIXEL_BITS);
	GLfloat fdx12 = dx12 * subpixelScale;
	GLfloat fdx31 = dx31 * subpixelScale;
	GLfloat fdy12 = dy12 * subpixelScale;
	GLfloat fdy31 = dy31 * subpixelScale;
	
	// compute the triangle area; denominator for interpolants
	GLfloat area = fdx31 * fdy12 - fdx12 * fdy31;
	
	if (area <= 0.0f) {
		return;
	}
	
	GLfloat invArea = 1.0f / area;
	
    // Bounding rectangle
    GLint minx = (Min(x1, x2, x3) + SUBPIXEL_MASK) >> GLES_SUBPIXEL_BITS;
    GLint maxx = (Max(x1, x2, x3) + SUBPIXEL_MASK) >> GLES_SUBPIXEL_BITS;
    GLint miny = (Min(y1, y2, y3) + SUBPIXEL_MASK) >> GLES_SUBPIXEL_BITS;
    GLint maxy = (Max(y1, y2, y3) + SUBPIXEL_MASK) >> GLES_SUBPIXEL_BITS;
    
    GLint span = maxx - minx;

	// x and y coordinate of pixel center of min/min-corner of rectangle
	GLfloat xStart = minx + 0.5f;
	GLfloat yStart = miny + 0.5f;
	
	// vector from a to (xmin, xmax) pixel center
	GLfloat deltaX = xStart - a->screen.x;
	GLfloat deltaY = yStart - a->screen.y;
	
	// interpolation of 1/w
	GLfloat dw12 = a->screen.w - b->screen.w, dw31 = c->screen.w - a->screen.w;
	Interpolation invW;	
	invW.dx = (fdy12 * dw31 - fdy31 * dw12) * invArea; 	
	invW.dy = (fdx31 * dw12 - fdx12 * dw31) * invArea; 	
	invW.value = a->screen.w + deltaX * invW.dx + deltaY * invW.dy;
	
	// interpolation of depth value
	GLfloat dd12 = a->screen.z - b->screen.z, dd31 = c->screen.z - a->screen.z;
	Interpolation depth;	
	depth.dx = (fdy12 * dd31 - fdy31 * dd12) * invArea; 	
	depth.dy = (fdx31 * dd12 - fdx12 * dd31) * invArea; 	
	
	GLfloat depthSlope=  GlesMaxf(GlesFabsf(depth.dx), GlesFabsf(depth.dy));
	GLfloat factor    =  depthSlope * state->polygonOffsetFactor;

	depth.value = a->screen.z + deltaX * depth.dx + deltaY * depth.dy + factor +
						state->polygonOffsetUnits * GlesLdexpf(1.0f, -state->writeSurface->depthBits);
		
	// interpolation of varyings
	Interpolation varying[GLES_MAX_VARYING_FLOATS];
	GLfloat vars[GLES_MAX_VARYING_FLOATS];
	
	union { Vec4f vec4f; Color color; } result;

	state->fragContext.varying = vars;
	state->fragContext.result = &result.vec4f;
	
	GLsizei index;
	
	// initialize gradients for varying data
	for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
		GLfloat aInvW = a->varyingData[index] * a->screen.w;
		GLfloat bInvW = b->varyingData[index] * b->screen.w;
		GLfloat cInvW = c->varyingData[index] * c->screen.w;

		GLfloat dv12 = aInvW - bInvW, dv31 = cInvW - aInvW;
		
		varying[index].dx = (fdy12 * dv31 - fdy31 * dv12) * invArea; 	
		varying[index].dy = (fdx31 * dv12 - fdx12 * dv31) * invArea; 	
		
		varying[index].value = 
			aInvW + deltaX * varying[index].dx + deltaY * varying[index].dy; 
	}
	 
    // Half-edge function values at origin
    GLint c1 = y2 * x1 - x2 * y1;
    GLint c2 = y3 * x2 - x3 * y2;
    GLint c3 = y1 * x3 - x1 * y3;
    
    // Correct for fill convention
    if (dy12 < 0 || (dy12 == 0 && dx12 > 0)) c1++;
    if (dy23 < 0 || (dy23 == 0 && dx23 > 0)) c2++;
    if (dy31 < 0 || (dy31 == 0 && dx31 > 0)) c3++;

	// determine values for pixel center of (minx, miny)
    GLint cy1 = c1 - dx12 * ((miny << GLES_SUBPIXEL_BITS) + HALF_PIXEL)
    			   + dy12 * ((minx << GLES_SUBPIXEL_BITS) + HALF_PIXEL);
    GLint cy2 = c2 - dx23 * ((miny << GLES_SUBPIXEL_BITS) + HALF_PIXEL) 
    			   + dy23 * ((minx << GLES_SUBPIXEL_BITS) + HALF_PIXEL);
    GLint cy3 = c3 - dx31 * ((miny << GLES_SUBPIXEL_BITS) + HALF_PIXEL) 
    			   + dy31 * ((minx << GLES_SUBPIXEL_BITS) + HALF_PIXEL);

	GLint x, y;
	SurfaceLoc loc;
	
	/* init surface location */
	GlesInitSurfaceLoc(state->writeSurface, &loc, minx, miny);
	
    for (y = miny; y < maxy; y++)
    {
        GLint cx1 = cy1, cx2 = cy2, cx3 = cy3;

        for (x = minx; x < maxx; x++)
        {
            if (cx1 > 0 && cx2 > 0 && cx3 > 0)
            {
            	/* TODO: pixel ownership & scissor test */
            	
            	GLfloat w = 1.0f / invW.value;
            	
            	for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
            		vars[index] = varying[index].value * w;
            	}
            	 
				if (GlesFragmentProgram(state->programs[state->program].executable)(&state->fragContext)) {            	
            		GlesWritePixel(state, &loc, &result.color, depth.value, !backFacing);
				}				
            }

            cx1 += dy12 << GLES_SUBPIXEL_BITS, 
            cx2 += dy23 << GLES_SUBPIXEL_BITS, 
            cx3 += dy31 << GLES_SUBPIXEL_BITS;
            
            invW.value += invW.dx;
            depth.value += depth.dx;
            
            for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
            	varying[index].value += varying[index].dx;
            }
            
            GlesStepSurfaceLoc(&loc, 1, 0);
        }

        cy1 -= dx12 << GLES_SUBPIXEL_BITS, 
        cy2 -= dx23 << GLES_SUBPIXEL_BITS, 
        cy3 -= dx31 << GLES_SUBPIXEL_BITS;

        invW.value += invW.dy - invW.dx * span;
        depth.value += depth.dy - depth.dx * span;
        
        for (index = 0; index < GLES_MAX_VARYING_FLOATS; ++index) {
        	varying[index].value += varying[index].dy - varying[index].dx * span;
        }

        GlesStepSurfaceLoc(&loc, -span, 1);
    }
}
