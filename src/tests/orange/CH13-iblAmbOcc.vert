//
// Vertex shader for image-based lighting with ambient occlusion
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

uniform vec3  BaseColor;
uniform float SpecularPercent;
uniform float DiffusePercent;

uniform samplerCube SpecularEnvMap;
uniform samplerCube DiffuseEnvMap;

varying vec3  ReflectDir;
varying vec3  Normal;
varying float Accessibility;

void main()
{
    // Look up environment map values in cube maps

    vec3 diffuseColor  = 
        vec3(textureCube(DiffuseEnvMap,  normalize(Normal)));

    vec3 specularColor = 
        vec3(textureCube(SpecularEnvMap, normalize(ReflectDir)));

    // Add lighting to base color and mix

    vec3 color = mix(BaseColor, diffuseColor*BaseColor, DiffusePercent);
    color     *= Accessibility;
    color      = mix(color, specularColor + color, SpecularPercent);

	// What is going on here?
    //gl_FragColor = vec4(envColor, 1.0);
}