#ifndef GLES_CONFIG_H
#define GLES_CONFIG_H 1

/*
** ==========================================================================
**
** $Id: config.h 65 2007-09-23 21:01:12Z hmwill $		
**
** Configuration settings and implementation limits for library build
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-23 14:01:12 -0700 (Sun, 23 Sep 2007) $
**
** --------------------------------------------------------------------------
**
** Vincent 3D Rendering Library, Programmable Pipeline Edition
** 
** Copyright (C) 2003-2007 Hans-Martin Will. 
**
** @CDDL_HEADER_START@
**
** The contents of this file are subject to the terms of the
** Common Development and Distribution License, Version 1.0 only
** (the "License").  You may not use this file except in compliance
** with the License.
**
** You can obtain a copy of the license at 
** http://www.vincent3d.com/software/ogles2/license/license.html
** See the License for the specific language governing permissions
** and limitations under the License.
**
** When distributing Covered Code, include this CDDL_HEADER in each
** file and include the License file named LICENSE.TXT in the root folder
** of your distribution.
** If applicable, add the following below this CDDL_HEADER, with the
** fields enclosed by brackets "[]" replaced with your own identifying
** information: Portions Copyright [yyyy] [name of copyright owner]
**
** @CDDL_HEADER_END@
**
** ==========================================================================
*/


#ifndef GLES_BUILD_NUMBER
#define GLES_BUILD_NUMBER "0.00 Debug"
#endif

#ifndef NDEBUG
#	define GLES_DEBUG			1		/* enable debugging code			*/
#else
#	define GLES_DEBUG			0		/* disable debugging code			*/
#endif

/*
** --------------------------------------------------------------------------
** Implementation Limits
** --------------------------------------------------------------------------
*/

#define GLES_SL_VERSION			100		/* shading language version			*/
#define GLES_MAX_MIPMAP_LEVELS	12		/* maximum number of mipmap levels	*/
#define GLES_MAX_TEXTURES		64		/* maximum number of textures		*/
#define GLES_MAX_TEXTURE_UNITS	 8		/* maximum number of texture units	*/
#define GLES_MAX_VERTEX_ATTRIBS	16		/* maximum number of vertex attr.	*/
#define GLES_MAX_SHADERS		64		/* maximum number of shaders		*/
#define GLES_MAX_PROGRAMS		32		/* maximum number of programs		*/
#define GLES_MAX_BUFFERS		64		/* maximum number of vertex buffers	*/

#define GLES_MAX_VERTEX_UNIFORM_COMPONENTS		512	/* storage for uniforms	*/
#define GLES_MAX_FRAGMENT_UNIFORM_COMPONENTS	GLES_MAX_VERTEX_UNIFORM_COMPONENTS
#define GLES_MAX_VARYING_FLOATS					32	/* storage for varying	*/
#define GLES_MAX_ADDRESS_BITS					12	/* shader address bits	*/

#define GLES_VERTEX_ATTRIB_COMPONENTS			4	/* # components			*/

#define GLES_MAX_VIEWPORT_WIDTH		2048	/* maximum viewport dimensions	*/
#define GLES_MAX_VIEWPORT_HEIGHT	2048

#define GLES_MAX_TEXTURE_SIZE		(1 << (GLES_MAX_MIPMAP_LEVELS - 1))	
#define GLES_MAX_TEXTURE_3D_SIZE		GLES_MAX_TEXTURE_SIZE
#define GLES_MAX_CUBE_MAP_TEXTURE_SIZE	GLES_MAX_TEXTURE_SIZE

#define GLES_MAX_STENCIL_BITS	8		/* maximum number of stencil bits	*/

#define GLES_SAMPLES			0 /*?*/	/* samples for multi sampling		*/
#define GLES_SAMPLE_BUFFERS		0 /*?*/	/* buffers multi sampling			*/
#define GLES_SUBPIXEL_BITS		4		/* sub-pixel accuracy				*/

#define GLES_MAX_ELEMENTS_INDICES	65536	/* no OES_element_index_uint	*/
#define GLES_MAX_ELEMENTS_VERTICES	65536	/* no OES_element_index_uint	*/

/*
** --------------------------------------------------------------------------
** Internal Limits
** --------------------------------------------------------------------------
*/

#define GLES_MAX_VERTEX_QUEUE	12		/* size of processed vertices queue	*/
										/* should be multiple of 2 and 3	*/

#define GLES_RASTER_BLOCK_SIZE	8		/* block size used in rasterizer	*/

#define GLES_LOG_BLOCK_SIZE		1024	/* number of characters per log blk	*/

#define GLES_MAX_PREPROC_MACROS 128		/* storage for 100 macros			*/

#define GLES_MAX_PREPROC_MACRO_DEPTH	10	/* nesting of macro expansions	*/
#define GLES_MAX_PREPROC_COND_DEPTH		10	/* nesting of if/else blocks	*/
#define GLES_PREPROC_SYMBOL_HASH 		123	/* preprocessor hash table size	*/
#define GLES_MAX_PREPROC_PARAMS			16	/* max. macro parameters		*/

#define GLES_SYMBOL_HASH 23				/* scope hash table size			*/
#define GLES_CONSTANT_HASH 23			/* const. table hash table size		*/

#define GLES_MAX_FUNCTION_DEPTH	16		/* nesting limit for function calls	*/
/*
** --------------------------------------------------------------------------
** Internal Precision Formats
** --------------------------------------------------------------------------
*/

#define GLES_VERTEX_HIGH_FLOAT_RANGE		62
#define GLES_VERTEX_HIGH_FLOAT_PRECISION	16
#define GLES_VERTEX_MEDIUM_FLOAT_RANGE		GLES_VERTEX_HIGH_FLOAT_RANGE
#define GLES_VERTEX_MEDIUM_FLOAT_PRECISION	GLES_VERTEX_HIGH_FLOAT_PRECISION
#define GLES_VERTEX_LOW_FLOAT_RANGE			GLES_VERTEX_HIGH_FLOAT_RANGE
#define GLES_VERTEX_LOW_FLOAT_PRECISION		GLES_VERTEX_HIGH_FLOAT_PRECISION

#define GLES_VERTEX_HIGH_INT_RANGE			62
#define GLES_VERTEX_HIGH_INT_PRECISION		1
#define GLES_VERTEX_MEDIUM_INT_RANGE		GLES_VERTEX_HIGH_INT_RANGE
#define GLES_VERTEX_MEDIUM_INT_PRECISION	GLES_VERTEX_HIGH_INT_PRECISION
#define GLES_VERTEX_LOW_INT_RANGE			GLES_VERTEX_HIGH_INT_RANGE
#define GLES_VERTEX_LOW_INT_PRECISION		GLES_VERTEX_HIGH_INT_PRECISION

/*
** --------------------------------------------------------------------------
** Branching Capabilities
** --------------------------------------------------------------------------
*/

#define GLES_VERTEX_SHADER_BRANCH			GL_FALSE	/* can do BRA?		*/
#define GLES_FRAGMENT_SHADER_BRANCH			GL_FALSE	/* can do BRA?		*/

/*
** --------------------------------------------------------------------------
** Version information
** --------------------------------------------------------------------------
*/

#define GLES_VENDOR				"Hans-Martin Will"
#define GLES_RENDERER			"Vincent 3D Rendering Library " GLES_BUILD_NUMBER
#define GLES_VERSION			"OpenGL ES 2.0"
#define GLES_SHADING_LANGUAGE	"OpenGL ES GLSL 1.00"
#define GLES_EXTENSIONS			"OES_fixed_point OES_single_precision " \
								"OES_read_format " \
								"OES_rgb8 OES_rgba8 OES_depth32 " \
								"OES_stencil1 OES_stencil4 OES_stencil8" \
								"OES_shader_source OES_mapbuffer "\
								"OES_texture_3D "\
								"VIN_shader_intermediate"

#endif /* ndef GLES_CONFIG_H */

