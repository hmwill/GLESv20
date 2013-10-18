//
// PTM fragment shader by Brad Ritter, Hewlett-Packard
// and Randi Rost, 3Dlabs.
//
// © Copyright 2003 3Dlabs, Inc., and 
// Hewlett-Packard Development Company, L.P.,
// Reproduced with Permission
//
uniform sampler2D ABCred;          // = 0
uniform sampler2D DEFred;          // = 1
uniform sampler2D ABCgrn;          // = 2
uniform sampler2D DEFgrn;          // = 3
uniform sampler2D ABCblu;          // = 4
uniform sampler2D DEFblu;          // = 5
uniform sampler2D Lighttexture;    // = 6

uniform vec3 ABCscale, ABCbias;
uniform vec3 DEFscale, DEFbias;

varying float Du;        // passes the computed L*tPrime value
varying float Dv;        // passes the computed L*bPrime value
varying float LdotN;     // passes the computed L*Normal value
varying vec2  TexCoord;  // passes s, t, texture coords

void main()
{
    vec3    ABCcoef, DEFcoef;
    vec3    ptvec;

    // Read coefficient values for red and apply scale and bias factors
    ABCcoef = (texture2D(ABCred, TexCoord).rgb - ABCbias) * ABCscale;
    DEFcoef = (texture2D(DEFred, TexCoord).rgb - DEFbias) * DEFscale;

    // Compute red polynomial
    ptvec.r = ABCcoef[0] * Du * Du +
              ABCcoef[1] * Dv * Dv +
              ABCcoef[2] * Du * Dv +
              DEFcoef[0] * Du +
              DEFcoef[1] * Dv +
              DEFcoef[2];

    // Read coefficient values for green and apply scale and bias factors
    ABCcoef = (texture2D(ABCgrn, TexCoord).rgb - ABCbias) * ABCscale;
    DEFcoef = (texture2D(DEFgrn, TexCoord).rgb - DEFbias) * DEFscale;

    // Compute green polynomial
    ptvec.g = ABCcoef[0] * Du * Du +
              ABCcoef[1] * Dv * Dv +
              ABCcoef[2] * Du * Dv +
              DEFcoef[0] * Du +
              DEFcoef[1] * Dv +
              DEFcoef[2];

    // Read coefficient values for blue and apply scale and bias factors
    ABCcoef = (texture2D(ABCblu, TexCoord).rgb - ABCbias) * ABCscale;
    DEFcoef = (texture2D(DEFblu, TexCoord).rgb - DEFbias) * DEFscale;

    // Compute blue polynomial
    ptvec.b = ABCcoef[0] * Du * Du +
              ABCcoef[1] * Dv * Dv +
              ABCcoef[2] * Du * Dv +
              DEFcoef[0] * Du +
              DEFcoef[1] * Dv +
              DEFcoef[2];

    // Multiply result * light factor
    ptvec *= texture2D(Lighttexture, vec2(LdotN, 0.5)).rgb;
    
    // Assign result to gl_FragColor
    gl_FragColor = vec4(ptvec, 1.0);
}