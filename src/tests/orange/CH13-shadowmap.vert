//
// Vertex shader for shadow mapping
//
// Author: Randi Rost, Philip Rideout
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
varying vec4 gl_FrontColor;

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

attribute float Accessibility;

uniform vec4 LightSourcePosition;
uniform mat4 TextureMatrix;

varying vec4  ShadowCoord;
  
// Ambient and diffuse scale factors.
const float As = 1.0 / 1.5;
const float Ds = 1.0 / 3.0;

void main()
{
    vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
    vec3 ecPosition3 = (vec3(ecPosition)) / ecPosition.w;
    vec3 VP = vec3(LightSourcePosition) - ecPosition3;
    VP = normalize(VP);
    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    float diffuse = max(0.0, dot(normal, VP));

    float scale = min(1.0, Accessibility * As + diffuse * Ds);

    vec4 texCoord = TextureMatrix * gl_Vertex;
    ShadowCoord   = texCoord / texCoord.w;

    gl_FrontColor  = vec4(scale * gl_Color.rgb, gl_Color.a);
    gl_Position    = ftransform();
}