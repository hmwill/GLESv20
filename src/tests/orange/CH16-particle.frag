//
// Fragment shader for rendering a particle system
//
// Author: Randi Rost
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

varying vec4 Color;

void main (void)
{
    gl_FragColor = Color;
}