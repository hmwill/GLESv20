//
// Fragment shader for shadow mapping
//
// Author: Philip Rideout
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

uniform sampler2DShadow ShadowMap;
uniform float Epsilon;

varying vec4 ShadowCoord;

float lookup(float x, float y)
{
    float depth = shadow2DProj(ShadowMap,
                      ShadowCoord + vec3(x, y, 0) * Epsilon).x;
    return depth != 1.0 ? 0.75 : 1.0;
}
 
void main()
{
    float shadeFactor = lookup(0.0, 0.0);
    gl_FragColor = vec4(shadeFactor * gl_Color.rgb, gl_Color.a);
}