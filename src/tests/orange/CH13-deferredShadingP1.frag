//
// Pass 1 fragment shader for deferred shading
//
// Author: Hugh Malan
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

uniform sampler2D TextureMap;

varying float CameraDepth;
varying vec2  TexCoord;

void main()
{
    // draw the typical textured output to visual framebuffer
    gl_FragData[0] = texture2D(TextureMap, TexCoord);

    // write "normaliized vertex depth" to the depthmap's alpha.
    gl_FragData[1] = vec4(vec3(0.0), CameraDepth);
}