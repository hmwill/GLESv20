//
// Fragment shader for chromatic aberration effect
//
// Author: Randi Rost
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

varying vec3  Reflect;
varying vec3  RefractR;
varying vec3  RefractG;
varying vec3  RefractB;
varying float Ratio;

uniform samplerCube Cubemap;

void main()
{
    vec3 refractColor, reflectColor;

    refractColor.r = vec3(textureCube(Cubemap, RefractR)).r;
    refractColor.g = vec3(textureCube(Cubemap, RefractG)).g;
    refractColor.b = vec3(textureCube(Cubemap, RefractB)).b;

    reflectColor   = vec3(textureCube(Cubemap, Reflect));

    vec3 color     = mix(refractColor, reflectColor, Ratio);

    gl_FragColor   = vec4(color, 1.0);
}