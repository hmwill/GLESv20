//
// fragment shader for neighborhood averaging via convolution
//
// Author: Randi Rost
//
// Copyright (c) 2003-2005: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

// maximum size supported by this shader
const int MaxKernelSize = 25; 

// array of offsets for accessing the base image
uniform vec2 Offset[MaxKernelSize];

// size of kernel (width * height) for this execution
const int KernelSize = MaxKernelSize;

// final scaling value
uniform vec4 ScaleFactor;

// image to be convolved
uniform sampler2D BaseImage;

varying vec2 gl_TexCoord[1];

void main()
{
    int i;
    vec4 sum = vec4(0.0);

    for (i = 0; i < KernelSize; i++)
        sum += texture2D(BaseImage, gl_TexCoord[0].st + Offset[0/*i*/]);

    gl_FragColor = sum * ScaleFactor;
}