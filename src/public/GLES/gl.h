#ifndef __gl_h_
#define __gl_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.0 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
*/

#ifdef _WIN32
#   ifdef __GL_EXPORTS
#       define GL_API __declspec(dllexport)
#   else
#       define GL_API __declspec(dllimport)
#   endif
#else
#   ifdef __GL_EXPORTS
#       define GL_API
#   else
#       define GL_API extern
#   endif
#endif

#define GL_APIENTRY 

#ifndef GLAPI
#	define GLAPI GL_API
#endif


typedef unsigned int    GLenum;
typedef unsigned char   GLboolean;
typedef unsigned int    GLbitfield;
typedef signed char     GLbyte;
typedef short           GLshort;
typedef int             GLint;
typedef int             GLsizei;
typedef unsigned char   GLubyte;
typedef unsigned short  GLushort;
typedef unsigned int    GLuint;
typedef float           GLfloat;
typedef float           GLclampf;
typedef int             GLfixed;
typedef int             GLclampx;

typedef int             GLintptr;
typedef int             GLsizeiptr;


/* OpenGL ES core versions */
#define GL_OES_VERSION_2_0                1

/* ClearBufferMask */
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000

/* Boolean */
#define GL_FALSE                          0
#define GL_TRUE                           1

/* BeginMode */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006

/* AlphaFunction */
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

/* BlendingFactorDest */
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */

/* BlendEquationSeperate */
#define GL_FUNC_ADD                        0x8006
#define GL_BLEND_EQUATION                  0x8009
#define GL_BLEND_EQUATION_RGB              0x8009	/* same as BLEND_EQUATION */
#define GL_BLEND_EQUATION_ALPHA            0x883D

/* BlendSubtract */
#define GL_FUNC_SUBTRACT                   0x800A
#define GL_FUNC_REVERSE_SUBTRACT           0x800B

/* Separate Blend Functions i.e. GL_OES_blend_func_seperate */
#define GL_BLEND_DST_RGB                   0x80C8
#define GL_BLEND_SRC_RGB                   0x80C9
#define GL_BLEND_DST_ALPHA                 0x80CA
#define GL_BLEND_SRC_ALPHA                 0x80CB

#define GL_CONSTANT_COLOR                  0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR        0x8002
#define GL_CONSTANT_ALPHA                  0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA        0x8004
#define GL_BLEND_COLOR                     0x8005

/* Buffer Objects */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F

#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_STREAM_DRAW                    0x88E0
#define GL_WRITE_ONLY                     0x88B9

#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765
#define GL_BUFFER_ACCESS                  0x88BB

#define GL_CURRENT_VERTEX_ATTRIB          0x8626

/* CullFaceMode */
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408

/* DepthFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* EnableCap */
#define GL_TEXTURE_2D                     0x0DE1
#define GL_CULL_FACE                      0x0B44
#define GL_BLEND                          0x0BE2
#define GL_DITHER                         0x0BD0
#define GL_STENCIL_TEST                   0x0B90
#define GL_DEPTH_TEST                     0x0B71
#define GL_SCISSOR_TEST                   0x0C11
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_COVERAGE                0x80A0

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505

/* FrontFaceDirection */
#define GL_CW                             0x0900
#define GL_CCW                            0x0901

/* GetPName */
#define GL_POINT_SIZE                     0x0B11
#define GL_LINE_WIDTH                     0x0B21
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
#define GL_VIEWPORT                       0x0BA2
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB

/* GetTextureParameter */
/*      GL_TEXTURE_MAG_FILTER */
/*      GL_TEXTURE_MIN_FILTER */
/*      GL_TEXTURE_WRAP_S */
/*      GL_TEXTURE_WRAP_T */

#define GL_NUM_COMPRESSED_TEXTURE_FORMATS       0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS           0x86A3

/* HintMode */
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

/* HintTarget */
#define GL_GENERATE_MIPMAP_HINT            0x8192
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B

/* DataType */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_FIXED                          0x140C

/* PixelFormat */
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A

/* PixelStoreParameter */
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05

/* PixelType */
/*      GL_UNSIGNED_BYTE */
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_5_6_5           0x8363

/* Shaders */
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
#define GL_MAX_VARYING_FLOATS             0x8B4B
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_SHADER_TYPE                    0x8B4F
#define GL_DELETE_STATUS                  0x8B80
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_CURRENT_PROGRAM                0x8B8D

/* StencilFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* StencilOp */
/*      GL_ZERO */
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_INVERT						  0x150A 
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508

/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

/* TextureMagFilter */
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

/* TextureMinFilter */
/*      GL_NEAREST */
/*      GL_LINEAR */
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

/* TextureParameterName */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

/* TextureTarget */
/*      GL_TEXTURE_2D */
/*      GL_TEXTURE_3D */
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C

/* TextureUnit */
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0

/* TextureWrapMode */
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_MIRRORED_REPEAT				  0x8370

/* Uniform Types */
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_CUBE                   0x8B60

/* Vertex Arrays */
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645


/*****************************************************************************************/
/*                                   OES Extensions                                      */
/*****************************************************************************************/

/* Mandatory Extensions */
#define OES_read_format				      1
#define OES_compressed_paletted_texture   1
#define OES_framebuffer_object            1
#define OES_stencil8                      1

/* Optional Extensions */
/*
#define OES_fbo_render_mipmap             1
#define OES_rgb8_rgba8                    1
#define OES_depth24                       1
#define OES_depth32                       1
#define OES_stencil1                      1
#define OES_stencil4                      1
#define OES_vertex_half_float             1
#define OES_texture_half_float            1
#define OES_texture_float                 1
#define OES_texture_half_float_linear     1
#define OES_texture_float_linear          1
#define OES_element_index_uint            1
#define OES_mapbuffer                     1
#define OES_texture_3D                    1
#define OES_texture_npot                  1
#define OES_fragment_precision_high       1
#define OES_compressed_ETC1_RGB8_texture  1
#define OES_shader_source                 1
#define OES_shader_binary                 1
 */

/* OES_read_format */
#define GL_IMPLEMENTATION_COLOR_READ_TYPE_OES   0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES 0x8B9B

/* OES_framebuffer_object */
#define GL_NONE                                              0

#define GL_FRAMEBUFFER_OES									 0x8D40

#define GL_RENDERBUFFER_OES									 0x8D41

#define GL_RGBA4                                             0x8056
#define GL_RGB5_A1                                           0x8057
#define GL_RGB565_OES										 0x8D62
#define GL_DEPTH_COMPONENT16								 0x81A5
#define GL_STENCIL_INDEX_OES                                 0x8D45

#define GL_RENDERBUFFER_WIDTH_OES							 0x8D42
#define GL_RENDERBUFFER_HEIGHT_OES							 0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_OES					 0x8D44

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES            0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES            0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES          0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_OES  0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES     0x8CD4

#define GL_COLOR_ATTACHMENT0_OES							 0x8CE0
#define GL_COLOR_ATTACHMENT1_OES							 0x8CE1
#define GL_COLOR_ATTACHMENT2_OES							 0x8CE2
#define GL_COLOR_ATTACHMENT3_OES							 0x8CE3
#define GL_COLOR_ATTACHMENT4_OES							 0x8CE4
#define GL_COLOR_ATTACHMENT5_OES							 0x8CE5
#define GL_COLOR_ATTACHMENT6_OES							 0x8CE6
#define GL_COLOR_ATTACHMENT7_OES							 0x8CE7
#define GL_COLOR_ATTACHMENT8_OES							 0x8CE8
#define GL_COLOR_ATTACHMENT9_OES							 0x8CE9
#define GL_COLOR_ATTACHMENT10_OES							 0x8CEA
#define GL_COLOR_ATTACHMENT11_OES							 0x8CEB
#define GL_COLOR_ATTACHMENT12_OES							 0x8CEC
#define GL_COLOR_ATTACHMENT13_OES							 0x8CED
#define GL_COLOR_ATTACHMENT14_OES							 0x8CEE
#define GL_COLOR_ATTACHMENT15_OES							 0x8CEF
#define GL_DEPTH_ATTACHMENT_OES								 0x8D00
#define GL_STENCIL_ATTACHMENT_OES							 0x8D20

#define GL_FRAMEBUFFER_COMPLETE_OES                          0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES             0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES     0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_OES   0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES             0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_OES                0x8CDA
#define GL_FRAMEBUFFER_UNSUPPORTED_OES                       0x8CDD
#define GL_FRAMEBUFFER_STATUS_ERROR_OES                      0x8CDE

#define GL_FRAMEBUFFER_BINDING_OES							 0x8CA6
#define GL_RENDERBUFFER_BINDING_OES							 0x8CA7
#define GL_MAX_COLOR_ATTACHMENTS_OES						 0x8CDF
#define GL_MAX_RENDERBUFFER_SIZE_OES						 0x84E8

#define GL_INVALID_FRAMEBUFFER_OPERATION_OES				 0x0506

/* OES_rgb8_rgba8 */
#define GL_RGB8                           0x8051
#define GL_RGBA8                          0x8058   

/* OES_depth_24 */
#define GL_DEPTH_COMPONENT24              0x81A6

/* OES_depth_32 */
#define GL_DEPTH_COMPONENT32              0x81A7

/* OES_stencil1 */
#define GL_STENCIL_INDEX1_OES             0x8D46

/* OES_stencil4 */
#define GL_STENCIL_INDEX4_OES             0x8D47

/* OES_stencil8 */
#define GL_STENCIL_INDEX8_OES             0x8D48

/* OES_vertex_half_float */
#define GL_HALF_FLOAT_OES                 0x8D61

/* OES_compressed_ETC1_RGB8_texture */
#define GL_ETC1_RGB8_OES                  0x8D64

/* OES_mapbuffer */
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD

/* OES_shader_source */
#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_SHADER_SOURCE_LENGTH           0x8B88

/* OES_shader_binary */
#define GL_PLATFORM_BINARY_OES            0x8D63

/* OES_texture_3D */
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_TEXTURE_3D                     0x806F
#define GL_TEXTURE_BINDING_3D             0x806A
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_SAMPLER_3D                     0x8B5F


GL_API void GL_APIENTRY glActiveTexture (GLenum texture);
GL_API void GL_APIENTRY glAttachShader (GLuint program, GLuint shader);
GL_API void GL_APIENTRY glBindAttribLocation (GLuint program, GLuint index, const char *name);
GL_API void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer);
GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture);
GL_API void GL_APIENTRY glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GL_API void GL_APIENTRY glBlendEquation( GLenum mode );
GL_API void	GL_APIENTRY glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor);
GL_API void GL_APIENTRY glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
GL_API void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GL_API void GL_APIENTRY glClear (GLbitfield mask);
GL_API void GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GL_API void GL_APIENTRY glClearDepthf (GLclampf depth);
GL_API void GL_APIENTRY glClearStencil (GLint s);
GL_API void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GL_API void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
GL_API void GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
GL_API void GL_APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GL_API void GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_API GLuint GL_APIENTRY glCreateProgram (void);
GL_API GLuint GL_APIENTRY glCreateShader (GLenum type);
GL_API void GL_APIENTRY glCullFace (GLenum mode);
GL_API void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers);
GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures);
GL_API void GL_APIENTRY glDeleteProgram (GLuint program);
GL_API void GL_APIENTRY glDeleteShader (GLuint shader);
GL_API void GL_APIENTRY glDetachShader (GLuint program, GLuint shader);
GL_API void GL_APIENTRY glDepthFunc (GLenum func);
GL_API void GL_APIENTRY glDepthMask (GLboolean flag);
GL_API void GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar);
GL_API void GL_APIENTRY glDisable (GLenum cap);
GL_API void GL_APIENTRY glDisableVertexAttribArray (GLuint index);
GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count);
GL_API void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices);
GL_API void GL_APIENTRY glEnable (GLenum cap);
GL_API void GL_APIENTRY glEnableVertexAttribArray (GLuint index);
GL_API void GL_APIENTRY glFinish (void);
GL_API void GL_APIENTRY glFlush (void);
GL_API void GL_APIENTRY glFrontFace (GLenum mode);
GL_API void GL_APIENTRY glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name);
GL_API void GL_APIENTRY glGetActiveUniform (GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name);
GL_API void GL_APIENTRY glGetAttachedShaders (GLuint program, GLsizei maxcount, GLsizei *count, GLuint *shaders);
GL_API int  GL_APIENTRY glGetAttribLocation (GLuint program, const char *name);
GL_API void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean *params);
GL_API void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers);
GL_API void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures);
GL_API GLenum GL_APIENTRY glGetError (void);
GL_API void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *params);
GL_API void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGetPointerv (GLenum pname, void **params);
GL_API void GL_APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufsize, GLsizei *length, char *infolog);
GL_API const GLubyte * GL_APIENTRY glGetString (GLenum name);
GL_API void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
GL_API void GL_APIENTRY glGetUniformfv (GLuint program, GLint location, GLfloat *params);
GL_API void GL_APIENTRY glGetUniformiv (GLuint program, GLint location, GLint *params);
GL_API int  GL_APIENTRY glGetUniformLocation (GLuint program, const char *name);
GL_API void GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params);
GL_API void GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, void **pointer);
GL_API void GL_APIENTRY glHint (GLenum target, GLenum mode);
GL_API GLboolean GL_APIENTRY glIsBuffer (GLuint buffer);
GL_API GLboolean GL_APIENTRY glIsEnabled (GLenum cap);
GL_API GLboolean GL_APIENTRY glIsProgram (GLuint program);
GL_API GLboolean GL_APIENTRY glIsShader (GLuint shader);
GL_API GLboolean GL_APIENTRY glIsTexture (GLuint texture);
GL_API void GL_APIENTRY glLineWidth (GLfloat width);
GL_API void GL_APIENTRY glLinkProgram (GLuint program);
GL_API void GL_APIENTRY glPixelStorei (GLenum pname, GLint param);
GL_API void GL_APIENTRY glPointSize (GLfloat size);
GL_API void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units);
GL_API void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
GL_API void GL_APIENTRY glSampleCoverage (GLclampf value, GLboolean invert);
GL_API void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
GL_API void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask);
GL_API void GL_APIENTRY glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask);
GL_API void GL_APIENTRY glStencilMask (GLuint mask);
GL_API void GL_APIENTRY glStencilMaskSeparate (GLenum face, GLuint mask);
GL_API void GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass);
GL_API void GL_APIENTRY glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
GL_API void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param);
GL_API void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param);
GL_API void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params);
GL_API void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
GL_API void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
GL_API void GL_APIENTRY glUniform1i (GLint location, GLint x);
GL_API void GL_APIENTRY glUniform2i (GLint location, GLint x, GLint y);
GL_API void GL_APIENTRY glUniform3i (GLint location, GLint x, GLint y, GLint z);
GL_API void GL_APIENTRY glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w);
GL_API void GL_APIENTRY glUniform1f (GLint location, GLfloat x);
GL_API void GL_APIENTRY glUniform2f (GLint location, GLfloat x, GLfloat y);
GL_API void GL_APIENTRY glUniform3f (GLint location, GLfloat x, GLfloat y, GLfloat z);
GL_API void GL_APIENTRY glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_API void GL_APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint *v);
GL_API void GL_APIENTRY glUniform2iv (GLint location, GLsizei count, const GLint *v);
GL_API void GL_APIENTRY glUniform3iv (GLint location, GLsizei count, const GLint *v);
GL_API void GL_APIENTRY glUniform4iv (GLint location, GLsizei count, const GLint *v);
GL_API void GL_APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat *v);
GL_API void GL_APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat *v);
GL_API void GL_APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat *v);
GL_API void GL_APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat *v);
GL_API void GL_APIENTRY glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_API void GL_APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_API void GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_API void GL_APIENTRY glUseProgram (GLuint program);
GL_API void GL_APIENTRY glValidateProgram (GLuint program);
GL_API void GL_APIENTRY glVertexAttrib1f (GLuint indx, GLfloat x);
GL_API void GL_APIENTRY glVertexAttrib2f (GLuint indx, GLfloat x, GLfloat y);
GL_API void GL_APIENTRY glVertexAttrib3f (GLuint indx, GLfloat x, GLfloat y, GLfloat z);
GL_API void GL_APIENTRY glVertexAttrib4f (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_API void GL_APIENTRY glVertexAttrib1fv (GLuint indx, const GLfloat *values);
GL_API void GL_APIENTRY glVertexAttrib2fv (GLuint indx, const GLfloat *values);
GL_API void GL_APIENTRY glVertexAttrib3fv (GLuint indx, const GLfloat *values);
GL_API void GL_APIENTRY glVertexAttrib4fv (GLuint indx, const GLfloat *values);
GL_API void GL_APIENTRY glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *ptr);
GL_API void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

/*****************************************************************************************/
/*                                 OES extension functions                               */
/*****************************************************************************************/

/* OES_framebuffer_object */
GL_API GLboolean GL_APIENTRY glIsRenderbufferOES(GLuint renderbuffer);
GL_API void GL_APIENTRY glBindRenderbufferOES(GLenum target, GLuint renderbuffer);
GL_API void GL_APIENTRY glDeleteRenderbuffersOES(GLsizei n, const GLuint *renderbuffers);
GL_API void GL_APIENTRY glGenRenderbuffersOES(GLsizei n, GLuint *renderbuffers);
GL_API void GL_APIENTRY glRenderbufferStorageOES(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GL_API void GL_APIENTRY glGetRenderbufferParameterivOES(GLenum target, GLenum pname, GLint* params);
GL_API void GL_APIENTRY glGetRenderbufferStorageFormatsivOES(GLenum target, int n, GLint *listofformats, GLint *numsupportedformats);
GL_API GLboolean GL_APIENTRY glIsFramebufferOES(GLuint framebuffer);
GL_API void GL_APIENTRY glBindFramebufferOES(GLenum target, GLuint framebuffer);
GL_API void GL_APIENTRY glDeleteFramebuffersOES(GLsizei n, const GLuint *framebuffers);
GL_API void GL_APIENTRY glGenFramebuffersOES(GLsizei n, GLuint *framebuffers);
GL_API GLenum GL_APIENTRY glCheckFramebufferStatusOES(GLenum target);
GL_API void GL_APIENTRY glFramebufferTexture2DOES(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_API void GL_APIENTRY glFramebufferTexture3DOES(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
GL_API void GL_APIENTRY glFramebufferRenderbufferOES(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GL_API void GL_APIENTRY glGetFramebufferAttachmentParameterivOES(GLenum target, GLenum attachment, GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGenerateMipmapOES(GLenum target);

/* OES_mapbuffer */
GL_API void GL_APIENTRY *glMapBuffer (GLenum target, GLenum access);
GL_API GLboolean GL_APIENTRY glUnmapBuffer (GLenum target);

/* OES_texture_3D */
GL_API void GL_APIENTRY glTexImage3D (GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
GL_API void GL_APIENTRY glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
GL_API void GL_APIENTRY glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_API void GL_APIENTRY glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
GL_API void GL_APIENTRY glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);

/* OES_shader_source */
GL_API void GL_APIENTRY glCompileShader (GLuint shader);
GL_API void GL_APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufsize, GLsizei *length, char *infolog);
GL_API void GL_APIENTRY glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei *length, char *source);
GL_API void GL_APIENTRY glReleaseShaderCompilerOES(void);
GL_API void GL_APIENTRY glShaderSource(GLuint shader, GLsizei count, const char **string, const GLint *length);

/* OES_shader_binary */
GL_API void GL_APIENTRY glShaderBinaryOES(GLint n, GLuint *shaders, GLenum binaryformat, const void *binary, GLint length);

/* OES_shader_source + OES_shader_binary */
GL_API void GL_APIENTRY glGetShaderPrecisionFormatOES(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);

/* VIN_shader_intermediate */
#define GL_SHADER_INTERMEDIATE_LENGTH_VIN		0x8EC0

GL_API void GL_APIENTRY glGetShaderIntermediateVIN (GLuint shader, GLsizei bufsize, GLsizei *length, char *intermediate);

#ifdef __cplusplus
}
#endif

#endif /* __gl_h_ */

