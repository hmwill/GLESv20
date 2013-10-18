//
// Pass 2 fragment shader for deferred shading
//
// Author: Hugh Malan
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

uniform sampler2D DepthTexture;
uniform sampler2D ShadowTexture;

varying vec2 DepthTexCoord;
varying vec3 ShadowNear;
varying vec3 ShadowDir;

const vec3 shadowColor = vec3(0.0);

void main()
{
    // read from DepthTexture
    // (depth is stored in texture's alpha component)
    float cameraDepth = texture2D(DepthTexture, DepthTexCoord).a;

    vec3 shadowPos = (cameraDepth * ShadowDir) + ShadowNear;
    float l = dot(shadowPos.yz, shadowPos.yz);
    float d = shadowPos.x;

    // k = shadow density: 0=opaque, 1=transparent
    // (use texture's red component as the density)
    float k = texture2D(ShadowTexture, vec2(l, d)).r;

    gl_FragColor = vec4(shadowColor, k);
}