/*
** ==========================================================================
**
** $Id: builtin.frag 38 2007-08-22 03:29:12Z hmwill $
** 
** Shading Language Front-End: Declarations for built-in functions and symbols 
** available in fragment shader
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-08-21 20:29:12 -0700 (Tue, 21 Aug 2007) $
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

/*
** --------------------------------------------------------------------------
** 7.2 Fragment Shader Special Variables 
** --------------------------------------------------------------------------
*/

mediump vec4  gl_FragCoord; 
        bool  gl_FrontFacing; 
mediump vec4  gl_FragColor; 
mediump vec4  gl_FragData[gl_MaxDrawBuffers]; 
mediump vec2  gl_PointCoord; 

/*
** --------------------------------------------------------------------------
** 8.7 Texture Lookup Functions
** --------------------------------------------------------------------------
*/

vec4 texture2D(sampler2D sampler, vec2 coord, float bias) {
	vec4	temp = vec4(coord, 0.0, bias);
	
	__txb	__retval, temp, sampler
}

vec4 texture2DProj(sampler2D sampler, vec3 coord, float bias) {
	vec4	temp, inv;
	
	__mov	temp.w, bias
	__rcp	inv, coord.z
	__mul	temp.xy, coord.xy, inv.xx

	__txb	__retval, temp, sampler
}

vec4 texture2DProj(sampler2D sampler, vec4 coord, float bias) {
	vec4	temp, inv;

	__mov	temp.w, bias
	__rcp	inv, coord.w
	__mul	temp.xy, coord.xy, inv.xx

	__txb	__retval, temp, sampler
}

vec4 texture3D(sampler3D sampler, vec3 coord, float bias)
{
   vec4		temp = vec4(coord.xyz, bias);

	__txb	__retval, temp, sampler
}

vec4 texture3DProj(sampler3D sampler, vec4 coord, float bias)
{
	vec4	temp, inv;

	__mov	temp.w, bias
	__rcp	inv, coord.w
	__mul	temp.xyz, coord.xyz, inv.xxx

	__txb	__retval, temp, sampler
}

vec4 textureCube (samplerCube sampler, vec3 coord, float bias) {
	vec4	temp = vec4(coord, bias);

	__txb	__retval, temp, sampler
}


