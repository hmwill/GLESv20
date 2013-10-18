//
// Vertex shader for Ward BRDF reflection
//
// Author: Randi Rost
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
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

attribute vec3 Tangent;
attribute vec3 Binormal;

uniform vec3 LightDir;  // Light direction in eye coordinates
uniform vec4 ViewPosition;

varying vec3 N, L, H, R, T, B;

void main() 
{
    vec3 V, eyeDir;
    vec4 pos;
       
    pos    = gl_ModelViewMatrix * gl_Vertex;
    eyeDir = pos.xyz;
    
    N = normalize(gl_NormalMatrix * gl_Normal);
    L = normalize(LightDir);
    V = normalize((gl_ModelViewMatrix * ViewPosition).xyz - pos.xyz);
    H = normalize(L + V);
    R = normalize(reflect(eyeDir, N));
    T = normalize(gl_NormalMatrix * Tangent);
    B = normalize(gl_NormalMatrix * Binormal);
    
    gl_Position = ftransform();
}