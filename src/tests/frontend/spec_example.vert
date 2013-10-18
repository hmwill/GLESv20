/*
** ==========================================================================
**
** $Id: spec_examples.vert 31 2007-08-19 07:49:45Z hmwill $			
** 
** All vertex shader language examples from the specification document
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-08-19 00:49:45 -0700 (Sun, 19 Aug 2007) $
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

// 4.1.7
struct light { 
    float intensity; 
    vec3 position; 
} lightVar; 

light lightVar1 = light(3.0, vec3(1.0, 2.0, 3.0)); 

// 4.1.9
float frequencies[3]; 
//uniform vec4 lightPosition[4]; 
const int numLights = 2; 
light lights[numLights];

void section_4_1_x() {
	// 4.1.2
	bool success;      // declare “success” to be a Boolean 
	bool done = false; // declare and initialize “done”

	// 4.1.3
	int i, j = 42; 

	// 4.1.4
	float a, b = 1.5; 

	// 4.1.5
	vec2 texcoord1, texcoord2; 
	vec3 position; 
	vec4 myRGBA; 
	ivec2 textureLookup; 
	bvec3 lessThan;

	// 4.1.6
	mat2 mat2D; 
	mat3 optMatrix; 
	mat4 view, projection;

	// 4.1.7
	light lightVar2;
}


// 4.3.2
const vec3 zAxis = vec3 (0.0, 0.0, 1.0); 

// 4.3.3
attribute vec4 position; 
attribute vec3 normal; 
attribute vec2 texCoord; 

// 4.3.4
uniform vec4 lightPosition; 

// 4.3.5
varying vec3 normal0;

// 5.5
void section_5_5() {
	
	// 5.5
	vec4 pos0 = vec4(1.0, 2.0, 3.0, 4.0); 
	vec4 swiz= pos0.wzyx; // swiz = (4.0, 3.0, 2.0, 1.0) 
	vec4 dup = pos0.xxyy; // dup = (1.0, 1.0, 2.0, 2.0)
	
	vec4 pos1 = vec4(1.0, 2.0, 3.0, 4.0); 
	pos1.xw = vec2(5.0, 6.0); // pos1 = (5.0, 2.0, 3.0, 6.0) 
	pos1.wx = vec2(7.0, 8.0); // pos1 = (8.0, 2.0, 3.0, 7.0)
}

void main(void) {


	
}