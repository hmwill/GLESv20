//
// Vertex shader for rendering a particle system
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
attribute vec4 gl_Color;
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

uniform float Time;            // updated each frame by the application
uniform vec4  Background;      // constant color equal to background
 
attribute vec3  Velocity;      // initial velocity
attribute float StartTime;     // time at which particle is activated

varying vec4 Color;
 
void main()
{
    vec4  vert;
    float t = Time - StartTime;

    if (t >= 0.0)
    {
        vert    = gl_Vertex + vec4(Velocity * t, 0.0);
        vert.y -= 4.9 * t * t;
        Color   = gl_Color;
    }
    else
    {
        vert  = gl_Vertex;      // Initial position
        Color = Background;     // "pre-birth" color
    }
 
    gl_Position  = gl_ModelViewProjectionMatrix * vert;
}