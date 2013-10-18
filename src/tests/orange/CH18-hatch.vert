//
// Vertex shader for procedurally generated hatching or "woodcut" appearance.
//
// This is an OpenGL 2.0 implementation of Scott F. Johnston's "Mock Media"
// (from "Advanced RenderMan: Beyond the Companion" SIGGRAPH 98 Course Notes)
//
// Author: Bert Freudenberg <bert@isg.cs.uni-magdeburg.de>
//
// Copyright (c) 2002-2003 3Dlabs, Inc. 
//
// See 3Dlabs-License.txt for license information
//

// --------------------------------------------------------------------------
// OpenGL Shading Language Compatibility
// --------------------------------------------------------------------------

attribute vec4 gl_Vertex;
attribute vec3 gl_Normal;
attribute vec4 gl_MultiTexCoord0;
attribute vec4 gl_MultiTexCoord1;
attribute vec4 gl_MultiTexCoord2;
attribute vec4 gl_MultiTexCoord3;
attribute vec4 gl_MultiTexCoord4;
attribute vec4 gl_MultiTexCoord5;
attribute vec4 gl_MultiTexCoord6;
attribute vec4 gl_MultiTexCoord7;
	
varying vec4 gl_TexCoord[2];
uniform mat4 gl_ModelViewProjectionMatrix;
uniform mat4 gl_ModelViewMatrix;
uniform mat3 gl_NormalMatrix;

vec4 ftransform()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

// --------------------------------------------------------------------------
// End of OpenGL Shading Language Compatibility
// --------------------------------------------------------------------------

uniform vec3  LightPosition;
uniform float Time;

varying vec3  ObjPos;
varying float V;
varying float LightIntensity;
 
void main()
{
    ObjPos          = (vec3(gl_Vertex) + vec3(0.0, 0.0, Time)) * 0.2;
 
    vec3 pos        = vec3(gl_ModelViewMatrix * gl_Vertex);
    vec3 tnorm      = normalize(gl_NormalMatrix * gl_Normal);
    vec3 lightVec   = normalize(LightPosition - pos);

    LightIntensity  = max(dot(lightVec, tnorm), 0.0);

    V = gl_MultiTexCoord0.t;  // try .s for vertical stripes

    gl_Position = ftransform();
}