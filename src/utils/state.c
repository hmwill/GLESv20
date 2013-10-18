/*
** ==========================================================================
**
** $Id: state.c 60 2007-09-18 01:16:07Z hmwill $			
** 
** Utility functions around GL state management for writing test cases
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


#include "utils.h"
#include <stdio.h>


GLboolean UtilResetState(void) {
	
	// Do we need to delete textures, shaders, and programs?
	
	#if 0
   GLint i, val;
	int windowWidth = atoi(flagWidthValue.cstring);
	int windowHeight = atoi(flagHeightValue.cstring);
   glGetIntegerv( GL_MAX_VERTEX_ATTRIBS, &val );

   for( i = 0; i < val; ++i )
   {
      glDisableVertexAttribArray( i );
      glVertexAttribPointer( i , 4, GL_FLOAT, GL_FALSE, 0, NULL );
   }
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glViewport( 0, 0, windowWidth, windowHeight );
   glDepthRangef( 0, 1 );
   glLineWidth( 1.f );
   glDisable( GL_CULL_FACE );
   glCullFace( GL_BACK );
   glFrontFace( GL_CCW );
   glPolygonOffset( 0, 0 );
   glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );
   glDisable( GL_SAMPLE_COVERAGE );
   glSampleCoverage( 1, GL_FALSE );
   glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &val );

   for ( i = 0; i < val; ++i ) {
      glActiveTexture( GL_TEXTURE0+i );
      glBindTexture( GL_TEXTURE_2D, 0 );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      //glBindTexture( GL_TEXTURE_3D, 0 ); // Need to check to see if extension is supported
      //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
      //glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      //glTexParameteri( GL_TEXTURE_3D, GL_WRAP_S, GL_REPEAT );
      //glTexParameteri( GL_TEXTURE_3D, GL_WRAP_T, GL_REPEAT );
      glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
      glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
      glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT );
   }

   glActiveTexture( GL_TEXTURE0 );
   glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
   glDepthMask( GL_TRUE );
   glStencilMaskSeparate( GL_FRONT_AND_BACK, 0xffffffff );
   glClearColor( 0, 0, 0, 0 );
   glClearDepthf( 1 );
   glClearStencil( 0 );
   glDisable( GL_SCISSOR_TEST );
   glScissor( 0, 0, windowWidth, windowHeight );
   glDisable( GL_STENCIL_TEST );
   glStencilFuncSeparate( GL_FRONT_AND_BACK, GL_ALWAYS, 0, 0xffffffff );
   glStencilOpSeparate( GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_KEEP );
   glDisable( GL_DEPTH_TEST );
   glDepthFunc( GL_LESS );
   glDisable( GL_BLEND );
   glBlendFuncSeparate( GL_ONE, GL_ZERO, GL_ONE, GL_ZERO );
   glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );
   glBlendColor( 0, 0, 0, 0 );
   glEnable( GL_DITHER );
   glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
   glPixelStorei( GL_PACK_ALIGNMENT, 4 );
   glUseProgram(0);
   glGetError();
	#endif
	return GL_TRUE;
}
