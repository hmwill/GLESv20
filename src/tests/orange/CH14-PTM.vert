//
// PTM vertex shader by Brad Ritter, Hewlett-Packard
// and Randi Rost, 3Dlabs.
//
// © Copyright 2003 3Dlabs, Inc., and 
// Hewlett-Packard Development Company, L.P.,
// Reproduced with Permission
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

uniform vec3 LightPos; 
uniform vec3 EyeDir;

attribute vec3 Tangent;
attribute vec3 Binormal;

varying float Du; 
varying float Dv; 
varying float LdotN;
varying vec2  TexCoord;

void main()
{
    vec3 lightTemp;
    vec3 halfAngleTemp;
    vec3 tPrime;
    vec3 bPrime;

    // Transform vertex
    gl_Position = ftransform();
    lightTemp = normalize(LightPos - gl_Vertex.xyz);

    // Calculate the Half Angle vector
    halfAngleTemp = normalize(EyeDir + lightTemp);

    // Calculate T' and B'
    //    T' = |T - (T.H)H|
    tPrime = Tangent - (halfAngleTemp * dot(Tangent, halfAngleTemp));
    tPrime = normalize(tPrime);

    //    B' = H x T'
    bPrime = cross(halfAngleTemp, tPrime);

    Du = dot(lightTemp, tPrime);
    Dv = dot(lightTemp, bPrime);

    // Multiply the Half Angle vector by NOISE_FACTOR
    // to avoid noisy BRDF data
    halfAngleTemp = halfAngleTemp * 0.9;

    // Hu = Dot(HalfAngle, T)
    // Hv = Dot(HalfAngle, B)
    // Remap [-1.0..1.0] to [0.0..1.0]
    TexCoord.s = dot(Tangent,  halfAngleTemp) * 0.5 + 0.5;
    TexCoord.t = dot(Binormal, halfAngleTemp) * 0.5 + 0.5;

    // "S" Text Coord3: Dot(Light, Normal);
    LdotN = dot(lightTemp, gl_Normal) * 0.5 + 0.5;
}