//
// Pass 1 vertex shader for deferred shading
//
// Author: Hugh Malan
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

uniform vec3  CameraPos;
uniform vec3  CameraDir;
uniform float DepthNear;
uniform float DepthFar;

varying float CameraDepth;  // normalized camera depth
varying vec2  TexCoord;

void main()
{
    // offset = vector to vertex from camera's position
    vec3 offset = (gl_Vertex.xyz / gl_Vertex.w) - CameraPos;

    // z = distance from vertex to camera plane
    float z = -dot(offset, CameraDir);

    // Depth from vertex to camera, mapped to [0,1]
    CameraDepth = (z - DepthNear) / (DepthFar - DepthNear);

    // typical interpolated coordinate for texture lookup
    TexCoord = gl_MultiTexCoord0.xy;

    gl_Position = ftransform();
}