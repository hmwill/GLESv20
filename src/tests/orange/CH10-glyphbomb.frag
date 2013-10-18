//
// Fragment shader for glyph bombing
//
// Author: Joshua Doss, Randi Rost
//
// Copyright (c) 2003-2006: 3Dlabs, Inc.
//
// See 3Dlabs-License.txt for license information
//

#define TWO_PI 6.28318

uniform vec4      ModelColor;

uniform sampler2D GlyphTex;
uniform sampler2D RandomTex;

uniform float     ColAdjust;
uniform float     ScaleFactor;
uniform float     Percentage;
uniform float     RO1;

const bool      RandomScale = true;
const bool      RandomRotate = true;
const float     SamplesPerCell = 4.0;

varying vec2      TexCoord;
varying float     LightIntensity;

void main()
{
    vec4 color  = ModelColor;
    vec2 cell   = floor(TexCoord);
    vec2 offset = TexCoord - cell;
   
    for (int i = -1; i <= int (RandomRotate); i++)
    {
        for (int j = -1; j <= int (RandomRotate); j++)
        {
            vec2 currentCell   = cell + vec2(float(i), float(j));
            vec2 currentOffset = offset - vec2(float(i), float(j));

            vec2 randomUV = currentCell * vec2(RO1);
         
            for (int k = 0; k < int (SamplesPerCell); k++)
            {
                vec4 random = texture2D(RandomTex, randomUV);
                randomUV   += random.ba;
               
                if (random.r < Percentage)
                {
                    vec2 glyphIndex;
                    mat2 rotator;
                    vec2 index;
                    float rotationAngle, cosRot, sinRot;
                                   
                    index.s = floor(random.b * 10.0);
                    index.t = floor(ColAdjust * 10.0);  
                                                    
                    if (RandomRotate)
                    {
                        rotationAngle = TWO_PI * random.g;                 
                        cosRot = cos(rotationAngle);
                        sinRot = sin(rotationAngle);
                        rotator[0] = vec2(cosRot, sinRot);
                        rotator[1] = vec2(-sinRot, cosRot);
                        glyphIndex = -rotator * 
                              (currentOffset - random.rg);                 
                    }
                    else
                    {
                        glyphIndex = currentOffset - random.rg;
                    }

                    if (RandomScale)
                        glyphIndex /= vec2(0.5 * random.r + 0.5);   

                    glyphIndex = 
                        (clamp(glyphIndex, 0.0, 1.0) + index) * 0.1;
            
                    vec4 image = texture2D(GlyphTex, glyphIndex);

                    if (image.r != 1.0)
                        color.rgb = mix(random.rgb * 0.7, color.rgb,
                                            image.r);             
                } 
            }        
        }
    }
   
    gl_FragColor   = color * LightIntensity;
   
}
