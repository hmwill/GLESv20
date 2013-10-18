#ifndef GLES_RASTER_RASTER_H
#define GLES_RASTER_RASTER_H 1

/*
** ==========================================================================
**
** $Id: raster.h 60 2007-09-18 01:16:07Z hmwill $		
**
** Declarations for rasterizer 
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

void GlesRasterPointSprite(State * state, RasterVertex * center, GLfloat pointSize);

void GlesRasterLine(State * state, RasterVertex * a, RasterVertex * b);

void GlesRasterTriangle(State * state, RasterVertex * a, RasterVertex * b, RasterVertex * c,
						GLboolean backFacing);

/**
 * Convert a floating point value to fixed point with sub-pixel precision
 * as defined in GLES_SUBPIXEL_BITS.
 * 
 * @param value
 * 		the number to convert
 * 
 * @return
 * 		the converted fixed point value
 */
static GLES_INLINE GLint GlesRasterValue(GLfloat value) {
	return (GLint) (value * (GLfloat) (1 << GLES_SUBPIXEL_BITS) + 0.5f);
}

#endif /* ndef GLES_RASTER_RASTER_H */
