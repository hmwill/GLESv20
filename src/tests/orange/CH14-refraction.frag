//
// Fragment shader for refraction
//
// Author: Randi Rost
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

varying vec3  Reflect;
varying vec3  Refract;
varying float Ratio;

uniform samplerCube Cubemap;

void main()
{
    vec3 refractColor = vec3(textureCube(Cubemap, Refract));
    vec3 reflectColor = vec3(textureCube(Cubemap, Reflect));

    vec3 color   = mix(refractColor, reflectColor, Ratio);

    gl_FragColor = vec4(color, 1.0);
}