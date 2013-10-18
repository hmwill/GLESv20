//
// Pass 2 vertex shader for deferred shading
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

uniform mat3 WorldToShadow;
uniform vec3 SphereOrigin;

uniform vec3 CameraPos;
uniform vec3 CameraDir;
uniform float DepthNear;
uniform float DepthFar;

varying vec2 DepthTexCoord;
varying vec3 ShadowNear;
varying vec3 ShadowDir;

void main()
{
    vec4 tmp1   = ftransform();
    gl_Position = tmp1;

    // Predivide out w to avoid perspective-correct interpolation.
    // The quantities being interpolated are screen-space texture 
    // coordinates and vectors to the near and far shadow plane,
    // all of which have to be bilinearly interpolated.
    // This could potentially be done by setting glHint,
    // but it wouldn't be guaranteed to work on all hardware.

    gl_Position.xyz /= gl_Position.w;
    gl_Position.w = 1.0;

    // Grab the transformed vertex's XY components as a texcoord
    // for sampling from the depth texture from pass 1.
    // Normalize them from [0,0] to [1,1]

    DepthTexCoord = gl_Position.xy * 0.5 + 0.5;

    // offset = vector to vertex from camera's position
    vec3 offset = (gl_Vertex.xyz / gl_Vertex.w) - CameraPos;

    // z = distance from vertex to camera plane
    float z = -dot(offset, CameraDir);

    vec3 shadowOffsetNear = offset * DepthNear / z;
    vec3 shadowOffsetFar  = offset * DepthFar / z;

    vec3 worldPositionNear = CameraPos + shadowOffsetNear;
    vec3 worldPositionFar  = CameraPos + shadowOffsetFar;

    vec3 shadowFar  = WorldToShadow * (worldPositionFar - SphereOrigin);
    ShadowNear = WorldToShadow * (worldPositionNear - SphereOrigin);
    ShadowDir  = shadowFar - ShadowNear;
}