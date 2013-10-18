/*
** ==========================================================================
**
** $Id: raster.c 60 2007-09-18 01:16:07Z hmwill $
**
** Rasterization state functions
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
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glLineWidth (GLfloat width) {
	State * state = GLES_GET_STATE();

	if (width <= 0.0f) {
		GlesRecordInvalidValue(state);
	} else {
		state->lineWidth = width;
	}
}

GL_API void GL_APIENTRY glPointSize (GLfloat size) {
	State * state = GLES_GET_STATE();

	if (size <= 0.0f) {
		GlesRecordInvalidValue(state);
	} else {
		state->pointSize = size;
	}
}

GL_API void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) {
	State * state = GLES_GET_STATE();

	state->polygonOffsetFactor = factor;
	state->polygonOffsetUnits  = units;
}

