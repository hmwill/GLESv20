//
// Fragment shader for procedurally generated toy ball
//
// Author: Bill Licea-Kane
//
// Copyright (c) 2002-2003 ATI Research 
//
// See ATI-License.txt for license information
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

varying vec4 ECposition;   // surface position in eye coordinates
varying vec4 ECballCenter; // ball center in eye coordinates
uniform vec4 BallCenter;   // ball center in modelling coordinates

void main()
{ 
    ECposition   = gl_ModelViewMatrix * gl_Vertex;
    ECballCenter = gl_ModelViewMatrix * BallCenter;
    gl_Position  = ftransform();
}