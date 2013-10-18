//
// Vertex shader for simplified uberlight model
//
// Author: Randi Rost
//         based on a RenderMan shader by Larry Gritz
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

uniform vec3 WCLightPos;      // Position of light in world coordinates
uniform vec4 ViewPosition;    // Position of camera in world space
uniform mat4 WCtoLC;          // World to light coordinate transform
uniform mat4 WCtoLCit;        // World to light inverse transpose
uniform mat4 MCtoWC;          // Model to world coordinate transform
uniform mat4 MCtoWCit;        // Model to world inverse transpose

varying vec3 LCpos;           // Vertex position in light coordinates
varying vec3 LCnorm;          // Normal in light coordinates
varying vec3 LCcamera;        // Camera position in light coordinates

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


void main()
{
    gl_Position = ftransform();
    
    // Compute world space position and normal
    vec4 wcPos  = MCtoWC * gl_Vertex;
    vec3 wcNorm = (MCtoWCit * vec4(gl_Normal, 0.0)).xyz;
    
    // Compute light coordinate system camera position,
    // vertex position and normal
    LCcamera = (WCtoLC * ViewPosition).xyz;
    LCpos    = (WCtoLC * wcPos).xyz;
    LCnorm   = (WCtoLCit * vec4(wcNorm, 0.0)).xyz;
}