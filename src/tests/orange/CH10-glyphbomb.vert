//
// Vertex shader for glyph bombing
//
// Author: Joshua Doss, Randi Rost
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

uniform float  SpecularContribution;
uniform vec3   LightPosition;
uniform float  ScaleFactor;

varying float  LightIntensity;
varying vec2   TexCoord;

void main()
{
    vec3  ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex);
    vec3  tnorm      = normalize(gl_NormalMatrix * gl_Normal);
    vec3  lightVec   = normalize(LightPosition - ecPosition);
    vec3  reflectVec = reflect(-lightVec, tnorm);
    vec3  viewVec    = normalize(-ecPosition);
    float diffuse    = max(dot(lightVec, tnorm), 0.0);
    float spec       = 0.0;

    if(diffuse > 0.0)
       {
          spec = max(dot(reflectVec, viewVec), 0.0);
          spec = pow(spec, 16.0);
       }
    
    float diffusecontribution  = 1.0 - SpecularContribution;
    LightIntensity = diffusecontribution * diffuse * 2.0 +
                         SpecularContribution * spec;
    
    TexCoord = gl_MultiTexCoord0.st * ScaleFactor;
    
    gl_Position = ftransform();
}