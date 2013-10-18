/*
** ==========================================================================
**
** $Id: texture.c 60 2007-09-18 01:16:07Z hmwill $
**
** Texture management functions
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-17 18:16:07 -0700 (Mon, 17 Sep 2007) $
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

#include <GLES/gl.h>
#include "config.h"
#include "platform/platform.h"
#include "gl/state.h"


/*
** --------------------------------------------------------------------------
** Local functions
** --------------------------------------------------------------------------
*/

static GLES_INLINE GLfloat WrapTexCoord(GLenum mode, GLfloat value) {
	GLfloat result;
	
	switch (mode) {
	case GL_CLAMP_TO_EDGE:
		result = GlesClampf(value);
		break;
				
	case GL_MIRRORED_REPEAT:
		{
			GLfloat f = GlesFloorf(value);
			
			if (((int) f) & 1) {
				/* odd */
				result = 1.0f - (value - f); 
			} else {
				/* even */
				result = value - f;
			}
		}
		
		break;
		
	case GL_REPEAT:
		result = value - GlesFloorf(value);
		break;
		
	default:
		GLES_ASSERT(GL_FALSE);
		result = 0.0f;
	}
	
	if (result == 1.0f) {
		/* prevent touching the border */
		result *= (65535.0f / 65536.0f);
	}
		
	return result; 
}

/**
 * Calculate an approxiamtion to the 2d hypotenuse sqrt(a^2 + b^2).
 * 
 * @param a
 * @param b
 */
static GLES_INLINE GLfloat Hypotenuse(GLfloat a, GLfloat b) {
	a = GlesFabsf(a);
	b = GlesFabsf(b);
	return GlesMaxf(a, b) + (11.0f / 32.0f) * GlesMaxf(a, b);
}

static Texture2D * GetCurrentTexture2D(State * state) {
	if (state->texture2D) {
		assert(state->textures[state->texture2D].base.textureType == GL_TEXTURE_2D);
		return &state->textures[state->texture2D].texture2D;
	} else {
		return &state->textureState.texture2D;
	}
}

static Texture3D * GetCurrentTexture3D(State * state) {
	if (state->texture3D) {
		assert(state->textures[state->texture3D].base.textureType == GL_TEXTURE_3D);
		return &state->textures[state->texture3D].texture3D;
	} else {
		return &state->textureState.texture3D;
	}
}

static TextureCube * GetCurrentTextureCube(State * state) {
	if (state->textureCube) {
		assert(state->textures[state->textureCube].base.textureType == GL_TEXTURE_CUBE_MAP);
		return &state->textures[state->textureCube].textureCube;
	} else {
		return &state->textureState.textureCube;
	}
}

static void AllocateImage2D(State * state, Image2D * image, GLenum internalFormat, 
							GLsizei width, GLsizei height,
							GLuint pixelElementSize) {
	GLsizeiptr size = pixelElementSize * width * height;

	GlesDeleteImage2D(state, image);

	image->internalFormat	= internalFormat;
	image->data				= GlesMalloc(size);

	if (image->data) {
		image->width		= width;
		image->height		= height;
	}
}

static void AllocateImage3D(State * state, Image3D * image, GLenum internalFormat, 
							GLsizei width, GLsizei height, GLsizei depth,
							GLuint pixelElementSize) {
	GLsizeiptr size = pixelElementSize * width * height * depth;

	GlesDeleteImage3D(state, image);

	image->internalFormat	= internalFormat;
	image->data				= GlesMalloc(size);

	if (image->data) {
		image->width		= width;
		image->height		= height;
		image->depth		= depth;
	}
}

/*
** --------------------------------------------------------------------------
** Format information
** --------------------------------------------------------------------------
*/

static GLenum GetBaseInternalFormat(GLenum internalFormat) {
	switch (internalFormat) {
		case GL_LUMINANCE:
		case GL_LUMINANCE_ALPHA:
		case GL_ALPHA:
			return internalFormat;

		case GL_RGB8:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_RGB565_OES:
			return GL_RGB;

		case GL_RGBA8:
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
			return GL_RGBA;

		default:
			GLES_ASSERT(0);
			return GL_NONE;
	}
}

static GLenum GetInternalFormat(State * state, GLenum internalformat, GLenum type) {
	switch (type) {
		case GL_UNSIGNED_BYTE:

			switch (internalformat) {
				case GL_LUMINANCE:
					return GL_LUMINANCE;

				case GL_LUMINANCE_ALPHA:
					return GL_LUMINANCE_ALPHA;

				case GL_ALPHA:
					return GL_ALPHA;

				case GL_RGB:
					return GL_RGB8;

				case GL_RGBA:
					return GL_RGBA8;

				default:
					GlesRecordInvalidEnum(state);
					return GL_NONE;
			}

		case GL_UNSIGNED_SHORT_4_4_4_4:
			if (internalformat != GL_RGBA) {
				GlesRecordInvalidEnum(state);
				return GL_NONE;
			}

			return type;

		case GL_UNSIGNED_SHORT_5_5_5_1:
			if (internalformat != GL_RGBA) {
				GlesRecordInvalidEnum(state);
				return GL_NONE;
			}

			return type;

		case GL_UNSIGNED_SHORT_5_6_5:
			if (internalformat != GL_RGB) {
				GlesRecordInvalidEnum(state);
				return GL_NONE;
			}

			return type;

		default:
			GlesRecordInvalidEnum(state);
			return GL_NONE;
	}
}

static GLsizei GetPixelSize(GLenum internalFormat) {
	switch (internalFormat) {
		case GL_LUMINANCE:
		case GL_ALPHA:
			return sizeof(GLubyte);

		case GL_LUMINANCE_ALPHA:
			return sizeof(GLubyte) * 2;

		case GL_RGB8:
			return sizeof(GLubyte) * 3;

		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
			return sizeof(GLushort);

		case GL_RGBA8:
			return sizeof(GLubyte) * 4;

		default:
			GLES_ASSERT(0);
			return 0;
	}
}

/**
 * Extract the mipmap filter from the minfilter texture parameter
 * 
 * @param minFilter
 * 		the minfilter texture parameter
 * @return GL_NONE, GL_NEAREST, or GL_LINEAR
 */
static GLenum GetMipmapFilter(GLenum minFilter) {
	switch (minFilter) {
	default:
		GLES_ASSERT(GL_FALSE);

	case GL_NEAREST:
	case GL_LINEAR:
		return GL_NONE;
		
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_NEAREST:
		return GL_NEAREST;
		
	case GL_NEAREST_MIPMAP_LINEAR:
	case GL_LINEAR_MIPMAP_LINEAR:
		return GL_LINEAR;
	}
}

/**
 * Extract the sample filter from the minfilter texture parameter
 * 
 * @param minFilter
 * 		the minfilter texture parameter
 * @return GL_NEAREST, or GL_LINEAR
 */
static GLenum GetSampleFilter(GLenum minFilter) {
	switch (minFilter) {
	default:
		GLES_ASSERT(GL_FALSE);

	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		return GL_NEAREST;
		
	case GL_LINEAR:
	case GL_LINEAR_MIPMAP_LINEAR:
	case GL_LINEAR_MIPMAP_NEAREST:
		return GL_LINEAR;
	}
}

static Image2D * GetImage2DForTargetAndLevel(State * state, GLenum target, GLint level) {
	if (level < 0 || level >= GLES_MAX_MIPMAP_LEVELS) {
		GlesRecordInvalidValue(state);
		return NULL;
	}

	switch (target) {
		case GL_TEXTURE_2D:
			return GetCurrentTexture2D(state)->image + level;			

		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			return GetCurrentTextureCube(state)->positiveX + level;			

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			return GetCurrentTextureCube(state)->negativeX + level;			

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			return GetCurrentTextureCube(state)->positiveY + level;			

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			return GetCurrentTextureCube(state)->negativeY + level;			

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			return GetCurrentTextureCube(state)->positiveZ + level;			

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			return GetCurrentTextureCube(state)->negativeZ + level;			

		default:
			GlesRecordInvalidEnum(state);
			return NULL;
	}
}

static Image3D * GetImage3DForTargetAndLevel(State * state, GLenum target, GLint level) {
	if (level < 0 || level >= GLES_MAX_MIPMAP_LEVELS) {
		GlesRecordInvalidValue(state);
		return NULL;
	}

	switch (target) {
		case GL_TEXTURE_3D:
			return GetCurrentTexture3D(state)->image + level;			

		default:
			GlesRecordInvalidEnum(state);
			return NULL;
	}
}

/*
** --------------------------------------------------------------------------
** Bitmap copy and conversion functions
** --------------------------------------------------------------------------
*/
typedef void (*CopyConversion)(GLubyte * dst, const GLubyte * src, GLsizei elements);

static void CopyRGB8fromRGB8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLsizeiptr size = elements * 3;

	do {
		*dst++ = *src++;
	} while (--size);
}

static void CopyRGB8fromRGB565(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	const GLushort * srcPtr = (const GLushort *) src;

	do {
		GLushort u565 = *srcPtr++;
		GLubyte b = (u565 & 0x001Fu) << 3;
		GLubyte g = (u565 & 0x07E0u) >> 3;
		GLubyte r = (u565 & 0xF800u) >> 8;

		r |= r >> 5;
		g |= g >> 6;
		b |= b >> 5;

		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
	} while (--elements);
}

static void CopyRGB565fromRGB8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLushort * dstPtr = (GLushort *) dst;

	do {
		GLushort r = *src++;
		GLushort g = *src++;
		GLushort b = *src++;

		*dstPtr++ = (b & 0xF8) >> 3 | (g & 0xFC) << 3 | (r & 0xF8) << 8;
	} while (--elements);
}

static void CopyRGBA8fromRGBA8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLsizeiptr size = elements * 4;

	do {
		*dst++ = *src++;
	} while (--size);
}

static void CopyRGBA8fromRGBA51(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	const GLushort * srcPtr = (const GLushort *) src;

	do {
		GLushort u5551 = *srcPtr++;
		GLubyte b = (u5551 & 0x003Eu) << 2;
		GLubyte g = (u5551 & 0x07C0u) >> 3;
		GLubyte r = (u5551 & 0xF800u) >> 8;
		GLubyte a = (u5551 & 0x0001u) << 7;

		r |= r >> 5;
		g |= g >> 5;
		b |= b >> 5;
		if (a) a |= 0x7f;

		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
		*dst++ = a;
	} while (--elements);
}

static void CopyRGBA51fromRGBA8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLushort * dstPtr = (GLushort *) dst;

	do {
		GLushort r = *src++;
		GLushort g = *src++;
		GLushort b = *src++;
		GLushort a = *src++;

		*dstPtr++ = (b & 0xF8) >> 2 | (g & 0xF8) << 3 | (r & 0xF8) << 8 | (a & 0x80) >> 7;
	} while (--elements);
}

static void CopyRGBA8fromRGBA4(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	const GLushort * srcPtr = (const GLushort *) src;

	do {
		GLushort u4444 = *srcPtr++;
		GLubyte r = (u4444 & 0xF000u) >> 8;
		GLubyte g = (u4444 & 0x0F00u) >> 4;
		GLubyte b = (u4444 & 0x00F0u);
		GLubyte a = (u4444 & 0x000Fu) << 4;

		r |= r >> 4;
		g |= g >> 4;
		b |= b >> 4;
		a |= a >> 4;

		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
		*dst++ = a;
	} while (--elements);
}

static void CopyRGBA4fromRGBA8(GLubyte * dst, const GLubyte * src, GLsizei elements) {
	GLushort * dstPtr = (GLushort *) dst;

	do {
		GLushort r = *src++;
		GLushort g = *src++;
		GLushort b = *src++;
		GLushort a = *src++;

		*dstPtr++ = (r & 0xf0) << 8 | (g & 0xf0) << 4 | (b & 0xf0) | a >> 4;
	} while (--elements);
}

static GLES_INLINE GLsizeiptr Align(GLsizeiptr offset, GLuint alignment) {
	return (offset + alignment - 1) & ~(alignment - 1);
}

// -------------------------------------------------------------------------
// Given two bitmaps src and dst, where src has dimensions 
// (srcWidth * srcHeight) and dst has dimensions (dstWidth * dstHeight),
// copy the rectangle (srcX, srcY, copyWidth, copyHeight) into
// dst at location (dstX, dstY).
//
// It is assumed that the copy rectangle is non-empty and has been clipped
// against the src and target rectangles
// -------------------------------------------------------------------------
static void SimpleCopyPixels(GLsizei pixelSize, 
							 const GLubyte * src, 
							 GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth,
							 GLint srcX, GLint srcY, GLint srcZ, 
							 GLsizei copyWidth, GLsizei copyHeight, GLsizei copyDepth,
							 GLubyte * dst, 
							 GLsizei dstWidth, GLsizei dstHeight, GLsizei dstDepth, 
							 GLint dstX, GLint dstY, GLint dstZ,
							 GLuint srcAlignment, GLuint dstAlignment) {

	GLsizeiptr srcBytesWidth = Align(srcWidth * pixelSize, srcAlignment);
	GLsizeiptr dstBytesWidth = Align(dstWidth * pixelSize, dstAlignment);

	GLsizeiptr srcGap = srcBytesWidth - copyWidth * pixelSize;	// how many bytes to skip for next line
	GLsizeiptr dstGap = dstBytesWidth - copyWidth * pixelSize;	// how many bytes to skip for next line

	GLsizeiptr srcImageGap = srcBytesWidth * (srcHeight - copyHeight);
	GLsizeiptr dstImageGap = dstBytesWidth * (dstHeight - copyHeight);

	const GLubyte * srcPtr = src + srcX * pixelSize + (srcY + srcZ * srcHeight) * srcBytesWidth;
	GLubyte * dstPtr = dst + dstX * pixelSize + (dstY + dstZ * dstHeight) * dstBytesWidth;

	do {
		do {
			GLsizeiptr span = copyWidth * pixelSize;

			do {
				*dstPtr++ = *srcPtr++;
			} while (--span);

			srcPtr += srcGap;
			dstPtr += dstGap;
		} while (--copyHeight);

		srcPtr += srcImageGap;
		dstPtr += dstImageGap;
	} while (--copyDepth);
}

/*
** Given two bitmaps src and dst, where src has dimensions 
** (srcWidth * srcHeight) and dst has dimensions (dstWidth * dstHeight),
** copy the rectangle (srcX, srcY, copyWidth, copyHeight) into
** dst at location (dstX, dstY).
**
** It is assumed that the copy rectangle is non-empty and has been clipped
** against the src and target rectangles
*/
static void ConvertCopyPixels(GLsizei srcPixelSize, GLsizei dstPixelSize,
							  const GLubyte * src, 
							  GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth,
							  GLint srcX, GLint srcY, GLint srcZ, 
							  GLsizei copyWidth, GLsizei copyHeight, GLsizei copyDepth,
							  GLubyte * dst, 
							  GLsizei dstWidth, GLsizei dstHeight, GLsizei dstDepth, 
							  GLint dstX, GLint dstY, GLint dstZ,
							  CopyConversion srcConversion, CopyConversion dstConversion,
							  GLuint srcAlignment, GLuint dstAlignment) {

	GLsizeiptr srcBytesWidth = Align(srcWidth * srcPixelSize, srcAlignment);
	GLsizeiptr dstBytesWidth = Align(dstWidth * dstPixelSize, dstAlignment);

	GLsizeiptr srcGap = srcBytesWidth - copyWidth * srcPixelSize;	// how many bytes to skip for next line
	GLsizeiptr dstGap = dstBytesWidth - copyWidth * dstPixelSize;	// how many bytes to skip for next line

	GLsizeiptr srcImageGap = srcBytesWidth * (srcHeight - copyHeight);
	GLsizeiptr dstImageGap = dstBytesWidth * (dstHeight - copyHeight);

	const GLubyte * srcPtr = src + srcX * srcPixelSize + (srcY + srcZ * srcHeight) * srcBytesWidth;
	GLubyte * dstPtr = dst + dstX * dstPixelSize + (dstY + dstZ * dstHeight) * dstBytesWidth;

	do {
		do {
			GLsizeiptr span = copyWidth;
			GLubyte buffer[128 * 4];

			while (span > 128) {
				srcConversion(buffer, srcPtr, 128);
				dstConversion(dstPtr, buffer, 128);
				srcPtr += srcPixelSize * 128;
				dstPtr += dstPixelSize * 128;

				span -= 128;
			}

			if (span > 0) {
				srcConversion(buffer, srcPtr, span);
				dstConversion(dstPtr, buffer, span);
				srcPtr += srcPixelSize * span;
				dstPtr += dstPixelSize * span;
			}

			srcPtr += srcGap;
			dstPtr += dstGap;
		} while (--copyHeight);

		srcPtr += srcImageGap;
		dstPtr += dstImageGap;
	} while (--copyDepth);
}

/*
** Given two bitmaps src and dst, where src has dimensions 
** (srcWidth * srcHeight) and dst has dimensions (dstWidth * dstHeight),
** copy the rectangle (srcX, srcY, copyWidth, copyHeight) into
** dst at location (dstX, dstY).
**
** The texture format and the type of the source format a given as
** parameters.
*/
static void CopyPixels(const void * src, 
					   GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth,
					   GLint srcX, GLint srcY, GLint srcZ, 
					   GLsizei copyWidth, GLsizei copyHeight, GLsizei copyDepth,
					   GLubyte * dst, 
					   GLsizei dstWidth, GLsizei dstHeight, GLsizei dstDepth, 
					   GLint dstX, GLint dstY, GLint dstZ,
					   GLenum baseInternalFormat, GLenum srcInternalFormat, GLenum dstInternalFormat,
					   GLuint srcAlignment, GLuint dstAlignment) {

	// ---------------------------------------------------------------------
	// clip lower left corner
	// ---------------------------------------------------------------------

	if (srcX >= srcWidth || srcY >= srcHeight || srcY >= srcDepth ||
		dstX >= dstWidth || dstY >= dstHeight || dstZ >= dstDepth ||
		copyWidth == 0 || copyHeight == 0 || copyDepth == 0) {
		return;
	}

	// ---------------------------------------------------------------------
	// clip the copy rectangle to the valid size
	// ---------------------------------------------------------------------

	if (copyWidth > dstWidth) {
		copyWidth = dstWidth;
	}

	if (copyWidth > srcWidth) {
		copyWidth = srcWidth;
	}

	if (copyHeight > dstHeight) {
		copyHeight = dstHeight;
	}

	if (copyHeight > srcHeight) {
		copyHeight = srcHeight;
	}

	if (copyDepth > dstDepth) {
		copyDepth = dstDepth;
	}

	if (copyDepth > srcDepth) {
		copyDepth = srcDepth;
	}

	// ---------------------------------------------------------------------
	// at this point we know that the copy rectangle is valid and non-empty
	// ---------------------------------------------------------------------

		
	switch (baseInternalFormat) {
		case GL_ALPHA:
		case GL_LUMINANCE:
			SimpleCopyPixels(sizeof(GLubyte), (const GLubyte *) src, 
							 srcWidth, srcHeight, srcDepth,
							 srcX, srcY, srcZ,
							 copyWidth, copyHeight, copyDepth,
							 (GLubyte *) dst, 
							 dstWidth, dstHeight, dstHeight,
							 dstX, dstY, dstZ,
							 srcAlignment, dstAlignment);
			break;

		case GL_LUMINANCE_ALPHA:
			SimpleCopyPixels(sizeof(GLubyte) * 2, (const GLubyte *) src, 
							 srcWidth, srcHeight, srcDepth, 
							 srcX, srcY, srcZ,
							 copyWidth, copyHeight,copyDepth,
							 (GLubyte *) dst, 
							 dstWidth, dstHeight, dstDepth,
							 dstX, dstY, dstZ,
							 srcAlignment, dstAlignment);
			break;

		case GL_RGB:
			switch (srcInternalFormat) {
				case GL_RGB:
					switch (dstInternalFormat) {
						case GL_RGB:
							SimpleCopyPixels(sizeof(GLubyte) * 3, (const GLubyte *) src, 
											 srcWidth, srcHeight, srcDepth,
											 srcX, srcY, srcZ,
											 copyWidth, copyHeight, copyDepth,
											 (GLubyte *) dst, 
											 dstWidth, dstHeight, dstDepth,
											 dstX, dstY, dstZ,
											 srcAlignment, dstAlignment);
							break;

						case GL_UNSIGNED_SHORT_5_6_5:
							ConvertCopyPixels(sizeof(GLubyte) * 3, sizeof(GLushort), (const GLubyte *) src, 
											  srcWidth, srcHeight, srcDepth,
											  srcX, srcY, srcZ,
											  copyWidth, copyHeight, copyDepth,
											  (GLubyte *) dst, 
											  dstWidth, dstHeight, dstDepth,
											  dstX, dstY, dstZ,
											  CopyRGB8fromRGB8, CopyRGB565fromRGB8, srcAlignment, dstAlignment);
							break;
					}
					break;

				case GL_UNSIGNED_SHORT_5_6_5:
					switch (dstInternalFormat) {
						case GL_RGB8:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLubyte) * 3, (const GLubyte *) src, 
											  srcWidth, srcHeight, srcDepth,
											  srcX, srcY, srcZ,
											  copyWidth, copyHeight, copyDepth,
											  (GLubyte *) dst, 
											  dstWidth, dstHeight, dstDepth, 
											  dstX, dstY, dstZ,
											  CopyRGB8fromRGB565, CopyRGB8fromRGB8, srcAlignment, dstAlignment);
							break;

						case GL_UNSIGNED_SHORT_5_6_5:
							SimpleCopyPixels(sizeof(GLushort), (const GLubyte *) src, 
											 srcWidth, srcHeight, srcDepth,
											 srcX, srcY, srcZ,
											 copyWidth, copyHeight, copyDepth,
											 (GLubyte *) dst, 
											 dstWidth, dstHeight, dstDepth,
											 dstX, dstY, dstZ,
											 srcAlignment, dstAlignment);
							break;
					}
					break;
			}

			break;

		case GL_RGBA:
			switch (srcInternalFormat) {
				case GL_RGBA8:
					switch (dstInternalFormat) {
						case GL_RGBA8:
							SimpleCopyPixels(sizeof(GLubyte) * 4, (const GLubyte *) src, 
											 srcWidth, srcHeight, srcDepth,
											 srcX, srcY, srcZ,
											 copyWidth, copyHeight, copyDepth,
											 (GLubyte *) dst, 
											 dstWidth, dstHeight, dstDepth,
											 dstX, dstY, dstZ,
											 srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_5_5_5_1:
							ConvertCopyPixels(sizeof(GLubyte) * 4, sizeof(GLushort), (const GLubyte *) src, 
											  srcWidth, srcHeight, srcDepth,
											  srcX, srcY, srcZ,
											  copyWidth, copyHeight, copyDepth,
											  (GLubyte *) dst, 
											  dstWidth, dstHeight, dstDepth,
											  dstX, dstY, dstZ,
											  CopyRGBA8fromRGBA8, CopyRGBA51fromRGBA8, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_4_4_4_4:
							ConvertCopyPixels(sizeof(GLubyte) * 4, sizeof(GLushort), (const GLubyte *) src, 
											  srcWidth, srcHeight, srcDepth,
											  srcX, srcY, srcZ,
											  copyWidth, copyHeight, copyDepth,
											  (GLubyte *) dst, 
											  dstWidth, dstHeight, dstDepth,
											  dstX, dstY, dstZ,
											  CopyRGBA8fromRGBA8, CopyRGBA4fromRGBA8, srcAlignment, dstAlignment);
							break;
					}
					break;

				case GL_UNSIGNED_SHORT_5_5_5_1:
					switch (dstInternalFormat) {
						case GL_RGBA8:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLubyte) * 4, (const GLubyte *) src, 
											  srcWidth, srcHeight, srcDepth,
											  srcX, srcY, srcZ,
											  copyWidth, copyHeight, copyDepth,
											  (GLubyte *) dst, 
											  dstWidth, dstHeight, dstDepth,
											  dstX, dstY, dstZ,
											  CopyRGBA8fromRGBA51, CopyRGBA8fromRGBA8, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_5_5_5_1:
							SimpleCopyPixels(sizeof(GLushort), (const GLubyte *) src, 
											 srcWidth, srcHeight, srcDepth,
											 srcX, srcY, srcZ,
											 copyWidth, copyHeight, copyDepth,
											 (GLubyte *) dst, 
											 dstWidth, dstHeight, dstDepth,
											 dstX, dstY, dstZ,
											 srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_4_4_4_4:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLushort), (const GLubyte *) src, 
											  srcWidth, srcHeight, srcDepth,
											  srcX, srcY, srcZ,
											  copyWidth, copyHeight, copyDepth,
											  (GLubyte *) dst, 
											  dstWidth, dstHeight, dstDepth,
											  dstX, dstY, dstDepth,
											  CopyRGBA8fromRGBA51, CopyRGBA4fromRGBA8, srcAlignment, dstAlignment);
							break;
					}
					break;

				case GL_UNSIGNED_SHORT_4_4_4_4:
					switch (dstInternalFormat) {
						case GL_RGBA8:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLubyte) * 4, (const GLubyte *) src, 
											  srcWidth, srcHeight, srcDepth,
											  srcX, srcY, srcZ,
											  copyWidth, copyHeight, copyDepth,
											  (GLubyte *) dst, 
											  dstWidth, dstHeight, dstDepth,
											  dstX, dstY, dstZ,
											  CopyRGBA8fromRGBA4, CopyRGBA8fromRGBA8, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_5_5_5_1:
							ConvertCopyPixels(sizeof(GLushort), sizeof(GLushort), (const GLubyte *) src, 
											  srcWidth, srcHeight, srcDepth,
											  srcX, srcY, srcZ,
											  copyWidth, copyHeight, copyDepth,
											  (GLubyte *) dst, 
											  dstWidth, dstHeight, dstDepth,
											  dstX, dstY, dstZ,
											  CopyRGBA8fromRGBA4, CopyRGBA51fromRGBA8, srcAlignment, dstAlignment);
							break;
						case GL_UNSIGNED_SHORT_4_4_4_4:
							SimpleCopyPixels(sizeof(GLushort), (const GLubyte *) src, 
											 srcWidth, srcHeight, srcDepth,
											 srcX, srcY, srcZ,
											 copyWidth, copyHeight, copyDepth,
											 (GLubyte *) dst, 
											 dstWidth, dstHeight, dstDepth,
											 dstX, dstY, dstZ,
											 srcAlignment, dstAlignment);
							break;
					}
					break;
			}

			break;

	}
}

/**
 * Fetch a pixel from memory using the specified format.
 * 
 * @param addr
 * 		memory address of the pixel data
 * @param internalFormat
 * 		pixel data format
 * @param result
 * 		where to store fetched results
 */
void GlesFetchPixel(const GLubyte * ptr, GLenum internalFormat, Vec4f * result) {
	switch (internalFormat) {
		case GL_LUMINANCE:
			result->x =
			result->y =
			result->z = ptr[0] * (1.0f / GLES_UBYTE_MAX);
			result->w = 1.0f;
			break;
			
		case GL_ALPHA:
			result->x =
			result->y =
			result->z = 0.0;
			result->w = ptr[0] * (1.0f / GLES_UBYTE_MAX);
			break;

		case GL_LUMINANCE_ALPHA:
			result->x =
			result->y =
			result->z = ptr[0] * (1.0f / GLES_UBYTE_MAX);
			result->w = ptr[1] * (1.0f / GLES_UBYTE_MAX);
			break;

		case GL_RGB8:
			result->x = ptr[0] * (1.0f / GLES_UBYTE_MAX);
			result->y = ptr[1] * (1.0f / GLES_UBYTE_MAX);
			result->z = ptr[2] * (1.0f / GLES_UBYTE_MAX);
			result->w = 1.0f;
			break;
			
		case GL_UNSIGNED_SHORT_5_6_5:
			{
				GLushort u565 = *(const GLushort *) ptr;
				GLubyte b = (u565 & 0x001Fu) << 3;
				GLubyte g = (u565 & 0x07E0u) >> 3;
				GLubyte r = (u565 & 0xF800u) >> 8;

				result->x = r * (1.0f / 32);
				result->y = g * (1.0f / 64);
				result->z = b * (1.0f / 32);
				result->w = 1.0f;
			}
			
			break;
			
		case GL_UNSIGNED_SHORT_4_4_4_4:
			{
				GLushort u4444 = *(const GLushort *) ptr;
				GLubyte r = (u4444 & 0xF000u) >> 8;
				GLubyte g = (u4444 & 0x0F00u) >> 4;
				GLubyte b = (u4444 & 0x00F0u);
				GLubyte a = (u4444 & 0x000Fu) << 4;

				result->x = r * (1.0f / 16);
				result->y = g * (1.0f / 16);
				result->z = b * (1.0f / 16);
				result->w = a * (1.0f / 16);
			}
			
			break;
			
		case GL_UNSIGNED_SHORT_5_5_5_1:
			{
				GLushort u5551 = *(const GLushort *) ptr;
				GLubyte b = (u5551 & 0x003Eu) << 2;
				GLubyte g = (u5551 & 0x07C0u) >> 3;
				GLubyte r = (u5551 & 0xF800u) >> 8;
				GLubyte a = (u5551 & 0x0001u) << 7;

				result->x = r * (1.0f / 32);
				result->y = g * (1.0f / 32);
				result->z = b * (1.0f / 32);
				result->w = a;
			}
			
			break;
			
		case GL_RGBA8:
			result->x = ptr[0] * (1.0f / GLES_UBYTE_MAX);
			result->y = ptr[1] * (1.0f / GLES_UBYTE_MAX);
			result->z = ptr[2] * (1.0f / GLES_UBYTE_MAX);
			result->w = ptr[3] * (1.0f / GLES_UBYTE_MAX);
			break;
			
		default:
			GLES_ASSERT(0);
	}
}

/**
 * Fetch the pixel of a 2-dimensional image with coordinates x and y into result.
 * 
 * @param image
 * 		the image to fetch the pixel from
 * @param x, y
 * 		pixel coordinates
 * @param result
 * 		where to store the fetch results
 */
GLES_INLINE static void FetchPixel2D(const Image2D * image, GLuint x, GLuint y, Vec4f * result) {
	const GLubyte * ptr = ((const GLubyte *) image->data) +
		(y * image->width + x) * GetPixelSize(image->internalFormat);
	GlesFetchPixel(ptr, image->internalFormat, result);
}

/**
 * Fetch the pixel of a 3-dimensional image with coordinates x and y into result.
 * 
 * @param image
 * 		the image to fetch the pixel from
 * @param x, y, z
 * 		pixel coordinates
 * @param result
 * 		where to store the fetch results
 */
GLES_INLINE static void FetchPixel3D(const Image3D * image, GLuint x, GLuint y, GLuint z, Vec4f * result) {
	const GLubyte * ptr = ((const GLubyte *) image->data) +
		((z * image->height + y) * image->width + x) * GetPixelSize(image->internalFormat);
	GlesFetchPixel(ptr, image->internalFormat, result);
}

/**
 * Sample a two-dimensional image at the given location.
 * 
 * @param image
 * 		the image to sample
 * @param s, t
 * 		the texture coordinates in range (0..1)
 * @param wrapS, wrapT
 * 		wrapping modes for s and t
 * @param sampleFilter
 * 		the sample filter to use, this can be GL_LINEAR or GL_NEAREST
 * @param result
 * 		where to store the results of the sampling operation
 */
void ImageSample2D(const Image2D * image, 
				   GLfloat s, GLenum wrapS,
				   GLfloat t, GLenum wrapT,
				   GLenum sampleFilter, 
				   Vec4f *result) {
					   	
	if (sampleFilter == GL_NEAREST) {
		/* GL_NEAREST */
		GLuint x = WrapTexCoord(wrapS, s) * image->width;
		GLuint y = WrapTexCoord(wrapT, t) * image->height;
		
		FetchPixel2D(image, x, y, result);
	} else {
		/* GL_LINEAR */
		GLfloat xs = WrapTexCoord(wrapS, s) * image->width;
		GLfloat xf = GlesFracf(xs);
		GLuint xl  = (GLuint) xs;
		GLuint xu  = WrapTexCoord(wrapS, s + 1.0f/image->width) * image->width;
		
		GLfloat ys = WrapTexCoord(wrapT, t) * image->height;
		GLfloat yf = GlesFracf(ys);
		GLuint yl  = (GLuint) ys;
		GLuint yu  = WrapTexCoord(wrapT, t + 1.0f/image->height) * image->height;

		Vec4f ll, lu, ul, uu;
		
		FetchPixel2D(image, xl, yl, &ll);
		FetchPixel2D(image, xl, yu, &lu);
		FetchPixel2D(image, xu, yl, &ul);
		FetchPixel2D(image, xu, yu, &uu);
		
		result->x = 
			(ll.x * (1.0f - xf) + ul.x * xf) * (1.0f - yf) +
			(lu.x * (1.0f - xf) + uu.x * xf) * yf; 
		result->y = 
			(ll.y * (1.0f - xf) + ul.y * xf) * (1.0f - yf) +
			(lu.y * (1.0f - xf) + uu.y * xf) * yf; 
		result->z = 
			(ll.z * (1.0f - xf) + ul.z * xf) * (1.0f - yf) +
			(lu.z * (1.0f - xf) + uu.z * xf) * yf; 
		result->w = 
			(ll.w * (1.0f - xf) + ul.w * xf) * (1.0f - yf) +
			(lu.w * (1.0f - xf) + uu.w * xf) * yf; 
	}	
}

/**
 * Sample a three-dimensional image at the given location.
 * 
 * @param image
 * 		the image to sample
 * @param s, t, r
 * 		the texture coordinates in range (0..1)
 * @param wrapS, wrapT
 * 		wrapping modes for s, t and r
 * @param sampleFilter
 * 		the sample filter to use, this can be GL_LINEAR or GL_NEAREST
 * @param result
 * 		where to store the results of the sampling operation
 */
void ImageSample3D(const Image3D * image, 
				   GLfloat s, GLenum wrapS,
				   GLfloat t, GLenum wrapT,
				   GLfloat r, GLenum wrapR,
				   GLenum sampleFilter, 
				   Vec4f *result) {
					   	
	if (sampleFilter == GL_NEAREST) {
		/* GL_NEAREST */
		GLuint x = WrapTexCoord(wrapS, s) * image->width;
		GLuint y = WrapTexCoord(wrapT, t) * image->height;
		GLuint z = WrapTexCoord(wrapR, r) * image->depth;
		
		FetchPixel3D(image, x, y, z, result);
	} else {
		/* GL_LINEAR */
		GLfloat xs = WrapTexCoord(wrapS, s) * image->width;
		GLfloat xf = GlesFracf(xs);
		GLuint xl  = (GLuint) xs;
		GLuint xu  = WrapTexCoord(wrapS, s + 1.0f/image->width) * image->width;
		
		GLfloat ys = WrapTexCoord(wrapT, t) * image->height;
		GLfloat yf = GlesFracf(ys);
		GLuint yl  = (GLuint) ys;
		GLuint yu  = WrapTexCoord(wrapT, t + 1.0f/image->height) * image->height;

		GLfloat zs = WrapTexCoord(wrapR, r) * image->depth;
		GLfloat zf = GlesFracf(zs);
		GLuint zl  = (GLuint) zs;
		GLuint zu  = WrapTexCoord(wrapR, r + 1.0f/image->depth) * image->depth;

		Vec4f lll, lul, ull, uul, llu, luu, ulu, uuu;
		
		FetchPixel3D(image, xl, yl, zl, &lll);
		FetchPixel3D(image, xl, yu, zl, &lul);
		FetchPixel3D(image, xu, yl, zl, &ull);
		FetchPixel3D(image, xu, yu, zl, &uul);
		FetchPixel3D(image, xl, yl, zu, &llu);
		FetchPixel3D(image, xl, yu, zu, &luu);
		FetchPixel3D(image, xu, yl, zu, &ulu);
		FetchPixel3D(image, xu, yu, zu, &uuu);
		
		result->x =
			(1.0f - zf) * 
				((lll.x * (1.0f - xf) + ull.x * xf) * (1.0f - yf) +
				 (lul.x * (1.0f - xf) + uul.x * xf) * yf) +
			zf * 
				((llu.x * (1.0f - xf) + ulu.x * xf) * (1.0f - yf) +
				 (luu.x * (1.0f - xf) + uuu.x * xf) * yf); 
		result->y = 
			(1.0f - zf) * 
				((lll.y * (1.0f - xf) + ull.y * xf) * (1.0f - yf) +
				 (lul.y * (1.0f - xf) + uul.y * xf) * yf) +
			zf * 
				((llu.y * (1.0f - xf) + ulu.y * xf) * (1.0f - yf) +
				 (luu.y * (1.0f - xf) + uuu.y * xf) * yf); 
		result->z = 
			(1.0f - zf) * 
				((lll.z * (1.0f - xf) + ull.z * xf) * (1.0f - yf) +
				 (lul.z * (1.0f - xf) + uul.z * xf) * yf) +
			zf * 
				((llu.z * (1.0f - xf) + ulu.z * xf) * (1.0f - yf) +
				 (luu.z * (1.0f - xf) + uuu.z * xf) * yf); 
		result->w = 
			(1.0f - zf) * 
				((lll.w * (1.0f - xf) + ull.w * xf) * (1.0f - yf) +
				 (lul.w * (1.0f - xf) + uul.w * xf) * yf) +
			zf * 
				((llu.w * (1.0f - xf) + ulu.w * xf) * (1.0f - yf) +
				 (luu.w * (1.0f - xf) + uuu.w * xf) * yf); 
	}	
}


/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

void GlesInitImage2D(Image2D * image) {

	image->data				= NULL;
	image->internalFormat	= GL_LUMINANCE;
	image->width			= 0;
	image->height			= 0;
}

void GlesDeleteImage2D(State * state, Image2D * image) {

	if (image->data != NULL) {
		GlesFree(image->data);
		image->data = NULL;
	}

	image->width			= 0;
	image->height			= 0;
}

void GlesInitImage3D(Image3D * image) {

	image->data				= NULL;
	image->internalFormat	= GL_LUMINANCE;
	image->width			= 0;
	image->height			= 0;
	image->depth			= 0;
}

void GlesDeleteImage3D(State * state, Image3D * image) {

	if (image->data != NULL) {
		GlesFree(image->data);
		image->data = NULL;
	}

	image->width			= 0;
	image->height			= 0;
	image->depth			= 0;
}

void GlesInitTextureBase(TextureBase * texture, GLenum textureType) {
	texture->textureType	= textureType;
	texture->isComplete		= GL_FALSE;
	texture->minFilter		= GL_NEAREST_MIPMAP_LINEAR;
	texture->magFilter		= GL_LINEAR;
	texture->wrapR			= GL_REPEAT;
	texture->wrapS			= GL_REPEAT;
	texture->wrapT			= GL_REPEAT;
}

void GlesInitTexture2D(Texture2D * texture) {

	GLuint index;

	GlesInitTextureBase(&texture->base, GL_TEXTURE_2D);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesInitImage2D(texture->image + index);
	}
}

void GlesDeleteTexture2D(State * state, Texture2D * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesDeleteImage2D(state, texture->image + index);
	}
}

void GlesInitTexture3D(Texture3D * texture) {

	GLuint index;

	GlesInitTextureBase(&texture->base, GL_TEXTURE_3D);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesInitImage3D(texture->image + index);
	}
}

void GlesDeleteTexture3D(State * state, Texture3D * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesDeleteImage3D(state, texture->image + index);
	}
}

void GlesInitTextureCube(TextureCube * texture) {

	GLuint index;

	GlesInitTextureBase(&texture->base, GL_TEXTURE_CUBE_MAP);

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {
		GlesInitImage2D(texture->negativeX + index);
		GlesInitImage2D(texture->positiveX + index);
		GlesInitImage2D(texture->negativeY + index);
		GlesInitImage2D(texture->positiveY + index);
		GlesInitImage2D(texture->negativeZ + index);
		GlesInitImage2D(texture->positiveZ + index);
	}
}

void GlesDeleteTextureCube(State * state, TextureCube * texture) {

	GLuint index;

	for (index = 0; index < GLES_MAX_MIPMAP_LEVELS; ++index) {

		GlesDeleteImage2D(state, texture->negativeX + index);
		GlesDeleteImage2D(state, texture->positiveX + index);
		GlesDeleteImage2D(state, texture->negativeY + index);
		GlesDeleteImage2D(state, texture->positiveY + index);
		GlesDeleteImage2D(state, texture->negativeZ + index);
		GlesDeleteImage2D(state, texture->positiveZ + index);
	}
}


/*
** --------------------------------------------------------------------------
** Public API entry points - Texture allocation and binding
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glActiveTexture (GLenum texture) {
	State * state = GLES_GET_STATE();
	
	if (texture < GL_TEXTURE0 || texture >= GL_TEXTURE0 + GLES_MAX_TEXTURE_UNITS) {
		GlesRecordInvalidEnum(state);
	}
	
	state->clientTextureUnit = texture - GL_TEXTURE0;
}

GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture) {

	State * state = GLES_GET_STATE();
	GLuint * textureRef = NULL;

	/************************************************************************/
	/* Validate parameters													*/
	/************************************************************************/

	if (target != GL_TEXTURE_2D && target != GL_TEXTURE_3D && target != GL_TEXTURE_CUBE_MAP) {
		GlesRecordInvalidEnum(state);
		return;
	}

	if (texture >= GLES_MAX_TEXTURES) {
		GlesRecordInvalidValue(state);
		return;
	}

	switch (target) {
		case GL_TEXTURE_2D:			
			textureRef = &state->texture2D;	
			break;

		case GL_TEXTURE_3D:			
			textureRef = &state->texture3D;	
			break;

		case GL_TEXTURE_CUBE_MAP:	
			textureRef = &state->textureCube;	
			break;
	}

	if (texture == 0) {
		/********************************************************************/
		/* 1. case: Reset to default texture state							*/
		/********************************************************************/

		*textureRef = 0;

	} else if (state->textures[texture].base.textureType != GL_INVALID_ENUM) {
		/********************************************************************/
		/* 2. case: Re-use a prviously allocated texture					*/
		/********************************************************************/

		if (state->textures[texture].base.textureType != target) {

			GlesRecordInvalidOperation(state);
			return;
		}

		*textureRef = texture;

	} else {
		/********************************************************************/
		/* 3. case: create a new texture object								*/
		/********************************************************************/

		switch (target) {

			case GL_TEXTURE_2D:			
				GlesInitTexture2D(&state->textures[texture].texture2D);	
				break;

			case GL_TEXTURE_3D:			
				GlesInitTexture3D(&state->textures[texture].texture3D);	
				break;

			case GL_TEXTURE_CUBE_MAP:	
				GlesInitTextureCube(&state->textures[texture].textureCube);	
				break;
		}

		*textureRef = texture;
	}
	
	if (texture) {
		state->textureUnits[state->clientTextureUnit].boundTexture = 
			&state->textures[texture];
	}
}

GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures) {

	State * state = GLES_GET_STATE();

	/************************************************************************/
	/* Validate parameters													*/
	/************************************************************************/

	if (n < 0 || textures == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Delete the individual textures										*/
	/************************************************************************/

	while (n--) {
		if (GlesIsBoundObject(state->textureFreeList, GLES_MAX_TEXTURES, *textures)) {

			GLuint * textureRef = NULL;
			GLuint unit;

			switch (state->textures[*textures].base.textureType) {

				case GL_TEXTURE_2D:			
					textureRef = &state->texture2D;	
					GlesDeleteTexture2D(state, &state->textures[*textures].texture2D);
					break;

				case GL_TEXTURE_3D:			
					textureRef = &state->texture3D;	
					GlesDeleteTexture3D(state, &state->textures[*textures].texture3D);
					break;

				case GL_TEXTURE_CUBE_MAP:	
					textureRef = &state->textureCube;	
					GlesDeleteTextureCube(state, &state->textures[*textures].textureCube);
					break;
			}

			if (textureRef != NULL && *textureRef == *textures) {
				/************************************************************************/
				/* Unbind the texture if it is currently in use and bound				*/
				/************************************************************************/

				*textureRef = 0;
			}
			
			for (unit = 0; unit < GLES_MAX_TEXTURE_UNITS; ++unit) {
				if (state->textureUnits[unit].boundTexture == &state->textures[*textures]) {
					state->textureUnits[unit].boundTexture = NULL;
				}
			}
		}

		++textures;
	}
}

GL_API void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures) {

	State * state = GLES_GET_STATE();
	GlesGenObjects(state, state->textureFreeList, GLES_MAX_TEXTURES, n, textures);
}

GL_API GLboolean GL_APIENTRY glIsTexture (GLuint texture) {

	State * state = GLES_GET_STATE();
	return GlesIsBoundObject(state->textureFreeList, GLES_MAX_TEXTURES, texture) &&
		state->textures[texture].base.textureType != GL_INVALID_ENUM;
}

/*
** --------------------------------------------------------------------------
** Public API entry points - Texture parameters
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY glPixelStorei (GLenum pname, GLint param) {

	State * state = GLES_GET_STATE();

	switch (pname) {
		case GL_UNPACK_ALIGNMENT:
			switch (param) {
			case 1:
			case 2:
			case 4:
			case 8:
				state->packAlignment = param;
				break;

			default:
				GlesRecordInvalidValue(state);
				break;
			}

			break;

		case GL_PACK_ALIGNMENT:
			switch (param) {
			case 1:
			case 2:
			case 4:
			case 8:
				state->unpackAlignment = param;
				break;

			default:
				GlesRecordInvalidValue(state);
				break;
			}

			break;

		default:
			GlesRecordInvalidEnum(state);
			break;
	}
}

GL_API void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) {

	State * state = GLES_GET_STATE();
	GLint result;

	if (params == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	glGetTexParameteriv(target, pname, &result);
	*params = (GLfloat) result;
}

GL_API void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) {

	State * state = GLES_GET_STATE();
	Texture * texture;


	if (params == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	switch (target) {
		case GL_TEXTURE_2D:
			if (state->texture2D) {
				texture = &state->textures[state->texture2D];
			} else {
				texture = (Texture *) &state->textureState.texture2D;
			}

			assert(texture->base.textureType == GL_TEXTURE_2D);
			break;

		case GL_TEXTURE_3D:
			if (state->texture3D) {
				texture = &state->textures[state->texture3D];
			} else {
				texture = (Texture *) &state->textureState.texture3D;
			}

			assert(texture->base.textureType == GL_TEXTURE_3D);
			break;

		case GL_TEXTURE_CUBE_MAP:
			if (state->textureCube) {
				texture = &state->textures[state->textureCube];
			} else {
				texture = (Texture *) &state->textureState.textureCube;
			}

			assert(texture->base.textureType == GL_TEXTURE_CUBE_MAP);
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}

	switch (pname) {
		case GL_TEXTURE_MIN_FILTER:
			*params = texture->base.minFilter;
			break;

		case GL_TEXTURE_MAG_FILTER:
			*params = texture->base.magFilter;
			break;

		case GL_TEXTURE_WRAP_S:
			*params = texture->base.wrapS;
			break;

		case GL_TEXTURE_WRAP_T:
			*params = texture->base.wrapT;
			break;

		case GL_TEXTURE_WRAP_R:

			if (target != GL_TEXTURE_3D) {
				GlesRecordInvalidEnum(state);
				return;
			}

			*params = texture->base.wrapR;
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}
}

GL_API void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
	glTexParameteri(target, pname, (GLint) param);
}

GL_API void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) {

	State * state = GLES_GET_STATE();

	if (params == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	glTexParameteri(target, pname, (GLint) *params);
}

GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) {

	State * state = GLES_GET_STATE();
	Texture * texture;

	switch (target) {
		case GL_TEXTURE_2D:
			if (state->texture2D) {
				texture = &state->textures[state->texture2D];
			} else {
				texture = (Texture *) &state->textureState.texture2D;
			}

			assert(texture->base.textureType == GL_TEXTURE_2D);
			break;

		case GL_TEXTURE_3D:
			if (state->texture3D) {
				texture = &state->textures[state->texture3D];
			} else {
				texture = (Texture *) &state->textureState.texture3D;
			}

			assert(texture->base.textureType == GL_TEXTURE_3D);
			break;

		case GL_TEXTURE_CUBE_MAP:
			if (state->textureCube) {
				texture = &state->textures[state->textureCube];
			} else {
				texture = (Texture *) &state->textureState.textureCube;
			}

			assert(texture->base.textureType == GL_TEXTURE_CUBE_MAP);
			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}

	switch (pname) {
		case GL_TEXTURE_MIN_FILTER:
			switch (param) {
				case GL_LINEAR:
				case GL_NEAREST:
				case GL_LINEAR_MIPMAP_LINEAR:
				case GL_LINEAR_MIPMAP_NEAREST: 
				case GL_NEAREST_MIPMAP_LINEAR:
				case GL_NEAREST_MIPMAP_NEAREST:
					texture->base.minFilter = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_MAG_FILTER:
			switch (param) {
				case GL_LINEAR:
				case GL_NEAREST:
					texture->base.magFilter = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_WRAP_S:
			switch (param) {
				case GL_REPEAT:
				case GL_CLAMP_TO_EDGE:
				case GL_MIRRORED_REPEAT:
					texture->base.wrapS = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_WRAP_T:
			switch (param) {
				case GL_REPEAT:
				case GL_CLAMP_TO_EDGE:
				case GL_MIRRORED_REPEAT:
					texture->base.wrapT = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		case GL_TEXTURE_WRAP_R:

			if (target != GL_TEXTURE_3D) {
				GlesRecordInvalidEnum(state);
				return;
			}

			switch (param) {
				case GL_REPEAT:
				case GL_CLAMP_TO_EDGE:
				case GL_MIRRORED_REPEAT:
					texture->base.wrapR = param;
					break;

				default:
					GlesRecordInvalidValue(state);
					return;
			}

			break;

		default:
			GlesRecordInvalidEnum(state);
			return;
	}
}

GL_API void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params) {

	State * state = GLES_GET_STATE();

	if (params == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	glTexParameteri(target, pname, *params);
}

/*
** --------------------------------------------------------------------------
** Public API entry points - Texture image functions
** --------------------------------------------------------------------------
*/
GL_API void GL_APIENTRY 
glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, 
						GLsizei width, GLsizei height, GLint border, 
						GLsizei imageSize, const void *data) {
	State * state = GLES_GET_STATE();
	GlesRecordInvalidEnum(state);
}

GL_API void GL_APIENTRY 
glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, 
						GLsizei width, GLsizei height, GLsizei depth, GLint border, 
						GLsizei imageSize, const void *data) {
	State * state = GLES_GET_STATE();
	GlesRecordInvalidEnum(state);
}

GL_API void GL_APIENTRY 
glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, 
						   GLsizei width, GLsizei height, GLenum format, 
						   GLsizei imageSize, const void *data) {
	State * state = GLES_GET_STATE();
	GlesRecordInvalidEnum(state);
}

GL_API void GL_APIENTRY 
glCompressedTexSubImage3D (GLenum target, GLint level, 
						   GLint xoffset, GLint yoffset, GLint zoffset, 
						   GLsizei width, GLsizei height, GLsizei depth, 
						   GLenum format, GLsizei imageSize, const void *data) {
	State * state = GLES_GET_STATE();
	GlesRecordInvalidEnum(state);
}

GL_API void GL_APIENTRY 
glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, 
				  GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	State * state = GLES_GET_STATE();

	GLsizei pixelSize;
	GLenum textureFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image2D * image = GetImage2DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	if (state->readSurface == NULL) {
		GlesRecordInvalidOperation(state);
		return;
	}

	textureFormat = state->readSurface->colorFormat;

	if (internalformat != GetBaseInternalFormat(textureFormat)) {
		GlesRecordInvalidValue(state);
		return;
	}

	pixelSize = GetPixelSize(textureFormat);

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_SIZE || height > GLES_MAX_TEXTURE_SIZE) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width == 0 || height == 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width + x > state->readSurface->size.width || height + y > state->readSurface->size.height) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (border != 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	AllocateImage2D(state, image, textureFormat, width, height, pixelSize);

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	state->readSurface->vtbl->lock(state->readSurface);

	CopyPixels(state->readSurface->colorBuffer, 
			   state->readSurface->size.width, state->readSurface->size.height, 1,
			   x, y, 0, width, height, 1,
			   image->data, width, height, 1, 0, 0, 0, internalformat, textureFormat, textureFormat, 1, 1);

	state->readSurface->vtbl->unlock(state->readSurface);
}

GL_API void GL_APIENTRY 
glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, 
					 GLint x, GLint y, GLsizei width, GLsizei height) {
	State * state = GLES_GET_STATE();

	GLenum textureFormat, baseInternalFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image2D * image = GetImage2DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	baseInternalFormat = GetBaseInternalFormat(image->internalFormat);

	if (baseInternalFormat != 
		GetBaseInternalFormat(state->readSurface->colorFormat)) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	textureFormat = state->readSurface->colorFormat;

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_SIZE || height > GLES_MAX_TEXTURE_SIZE) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width == 0 || height == 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width + x > state->readSurface->size.width || height + y > state->readSurface->size.height) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (xoffset < 0 || yoffset < 0 || 
		xoffset + width > image->width || 
		yoffset + height > image->height) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	state->readSurface->vtbl->lock(state->readSurface);

	CopyPixels(state->readSurface->colorBuffer, 
			   state->readSurface->size.width, state->readSurface->size.height, 1,
			   x, y, 0, width, height, 1,
			   image->data, image->width, image->height, 1, xoffset, yoffset, 0, baseInternalFormat, 
			   textureFormat, image->internalFormat, 1, 1);

	state->readSurface->vtbl->unlock(state->readSurface);
}

GL_API void GL_APIENTRY 
glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, 
					 GLint x, GLint y, GLsizei width, GLsizei height) {
	State * state = GLES_GET_STATE();

	GLenum textureFormat, baseInternalFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image3D * image = GetImage3DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	baseInternalFormat = GetBaseInternalFormat(image->internalFormat);

	if (baseInternalFormat != 
		GetBaseInternalFormat(state->readSurface->colorFormat)) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	textureFormat = state->readSurface->colorFormat;

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_SIZE || height > GLES_MAX_TEXTURE_SIZE) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width == 0 || height == 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width + x > state->readSurface->size.width || 
		height + y > state->readSurface->size.height) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (xoffset < 0 || yoffset < 0 || zoffset < 0 ||
		xoffset + width > image->width || 
		yoffset + height > image->height ||
		zoffset + 1 > image->depth) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	state->readSurface->vtbl->lock(state->readSurface);

	CopyPixels(state->readSurface->colorBuffer, 
			   state->readSurface->size.width, state->readSurface->size.height, 1, 
			   x, y, 0, width, height, 1,
			   image->data, image->width, image->height, image->depth, 
			   xoffset, yoffset, zoffset, baseInternalFormat, 
			   textureFormat, image->internalFormat, 1, 1);

	state->readSurface->vtbl->unlock(state->readSurface);
}

GL_API void GL_APIENTRY 
glTexImage2D (GLenum target, GLint level, GLenum internalformat, 
			  GLsizei width, GLsizei height, GLint border, 
			  GLenum format, GLenum type, const void *pixels) {

	State * state = GLES_GET_STATE();
	GLsizei pixelSize;
	GLenum textureFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image2D * image = GetImage2DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	textureFormat = GetInternalFormat(state, internalformat, type);

	if (textureFormat == GL_NONE) {
		return;
	}

	pixelSize = GetPixelSize(textureFormat);

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_SIZE || height > GLES_MAX_TEXTURE_SIZE) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (border != 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if ((width == 0 || height == 0) && pixels != NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width != 0 && height != 0 && pixels == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	AllocateImage2D(state, image, textureFormat, width, height, pixelSize);

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	CopyPixels(pixels, 
			   width, height, 1, 
			   0, 0, 0, width, height, 1, 
			   image->data, width, height, 1, 0, 0, 0,
			   internalformat, textureFormat, textureFormat, state->packAlignment, 1);
}

GL_API void GL_APIENTRY 
glTexImage3D (GLenum target, GLint level, GLenum internalformat, 
			  GLsizei width, GLsizei height, GLsizei depth, GLint border, 
			  GLenum format, GLenum type, const void *pixels) {
	State * state = GLES_GET_STATE();
	GLsizei pixelSize;
	GLenum textureFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image3D * image = GetImage3DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	textureFormat = GetInternalFormat(state, internalformat, type);

	if (textureFormat == GL_NONE) {
		return;
	}

	pixelSize = GetPixelSize(textureFormat);

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_3D_SIZE || 
		height > GLES_MAX_TEXTURE_3D_SIZE ||
		depth > GLES_MAX_TEXTURE_3D_SIZE) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (border != 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if ((width == 0 || height == 0 || depth == 0) && pixels != NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width != 0 && height != 0 && depth != 0 && pixels == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	AllocateImage3D(state, image, textureFormat, width, height, depth, pixelSize);

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	CopyPixels(pixels, 
			   width, height, depth,
			   0, 0, 0, width, height, depth,
			   image->data, width, height, depth, 0, 0, 0,
			   internalformat, textureFormat, textureFormat, state->packAlignment, 1);
}

GL_API void GL_APIENTRY 
glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, 
				 GLsizei width, GLsizei height, 
				 GLenum format, GLenum type, const void *pixels) {
	State * state = GLES_GET_STATE();
	GLsizei pixelSize;
	GLenum textureFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image2D * image = GetImage2DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	if (GetBaseInternalFormat(image->internalFormat) != format) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	textureFormat = GetInternalFormat(state, format, type);

	if (textureFormat == GL_NONE) {
		return;
	}

	pixelSize = GetPixelSize(textureFormat);

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_SIZE || height > GLES_MAX_TEXTURE_SIZE) {
		GlesRecordInvalidValue(state);
		return;
	}

	if ((width == 0 || height == 0) && pixels != NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width != 0 && height != 0 && pixels == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (xoffset < 0 || yoffset < 0 || 
		xoffset + width > image->width || 
		yoffset + height > image->height) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	CopyPixels(pixels, 
			   width, height, 1,
			   0, 0, 0, width, height, 1,
			   image->data, image->width, image->height, 1, xoffset, yoffset, 0,
			   format, textureFormat, image->internalFormat, state->packAlignment, 1);
}

GL_API void GL_APIENTRY 
glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, 
				 GLsizei width, GLsizei height, GLsizei depth, 
				 GLenum format, GLenum type, const void *pixels) {
	State * state = GLES_GET_STATE();
	GLsizei pixelSize;
	GLenum textureFormat;

	/************************************************************************/
	/* Determine texture image to load										*/
	/************************************************************************/

	Image3D * image = GetImage3DForTargetAndLevel(state, target, level);

	if (image == NULL) {
		return;
	}

	if (GetBaseInternalFormat(image->internalFormat) != format) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Determine texture format												*/
	/************************************************************************/

	textureFormat = GetInternalFormat(state, format, type);

	if (textureFormat == GL_NONE) {
		return;
	}

	pixelSize = GetPixelSize(textureFormat);

	/************************************************************************/
	/* Verify image dimensions												*/
	/************************************************************************/

	if (width > GLES_MAX_TEXTURE_3D_SIZE || 
		height > GLES_MAX_TEXTURE_3D_SIZE ||
		depth > GLES_MAX_TEXTURE_3D_SIZE) {
		GlesRecordInvalidValue(state);
		return;
	}

	if ((width == 0 || height == 0 || depth == 0) && pixels != NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width != 0 && height != 0 && depth != 0 && pixels == NULL) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (xoffset < 0 || yoffset < 0 || zoffset < 0 ||
		xoffset + width > image->width || 
		yoffset + height > image->height ||
		zoffset + depth > image->depth) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	if (!image->data) {
		GlesRecordOutOfMemory(state);
		return;
	}

	CopyPixels(pixels, 
			   width, height, depth,
			   0, 0, 0, width, height, depth,
			   image->data, 
			   image->width, image->height, image->depth, 
			   xoffset, yoffset, zoffset,
			   format, textureFormat, image->internalFormat, state->packAlignment, 1);
}

/**
 * Sample a 2D texture at the given location.
 * 
 * NOTE: This function will actually change to a signature that will
 * process a 2 by 2 block of pixels in order to perform proper mipmapping.
 * 
 * @param base
 * 		texture base information
 * @param image
 * 		mipmap array of texture images
 * @param coord
 * 		texture coordinates at which to sample, the w coordinate contains 
 * 		the lod bias to apply, or the lod if no derivatives are given
 * @param dx
 * 		the derivative of the coord vector with regard to x
 * @param dy
 * 		the derivative of the coord vector with regard to y
 * @param result
 * 		where to store fetched result values
 */
void TextureSample2D(const TextureBase * base, const Image2D * image, 
					 const Vec4f * coords,
				  	 const Vec4f * dx, const Vec4f * dy,
                     Vec4f * result) {
	GLenum mipmapFilter = GetMipmapFilter(base->minFilter);
	GLenum sampleFilter = GetSampleFilter(base->minFilter);
	
	if (mipmapFilter != GL_NONE && !base->isComplete) {
		/* 
		 * texture samples as 0 vector if mipmapping is requested, but texture
		 * is not complete.
		 */
		result->x = result->y = result->z = result->w = 0.0f;
		return;
	}
		
	// determine mipmap level
	GLfloat lambda_ = GlesClampf(coords->w);
	GLint lambda = 0;
	
	// compute lambda'
	if (mipmapFilter != GL_NONE) {
		if (dx && dy) {
			GLsizei width  = image[0].width;
			GLsizei height = image[0].height;
	
			lambda_ +=
				GlesLog2f(GlesMaxf(width * Hypotenuse(GlesFabsf(dx->x), GlesFabsf(dx->y)),
						   		   height * Hypotenuse(GlesFabsf(dy->x), GlesFabsf(dy->y))));
		}

		lambda = (GLint) lambda_;
		
		if (lambda_ < (mipmapFilter != GL_NONE && 
					   sampleFilter == GL_NEAREST &&
					   base->magFilter == GL_LINEAR ? 0.5f : 0.0f)) {
			/* magnification; use base mipmap level */
			mipmapFilter = GL_NEAREST;
			sampleFilter = base->magFilter;
			lambda = 0;
		} else if (lambda_ >= base->maxMipmapLevel) {
			/* clip at max level */
			lambda = base->maxMipmapLevel;
			mipmapFilter = GL_NEAREST;
		}
	}
		
	// fetch actual pixel data
	if (mipmapFilter == GL_NONE || mipmapFilter == GL_NEAREST) {
		ImageSample2D(&image[lambda], 
					  coords->x, base->wrapS,
					  coords->y, base->wrapT,
					  sampleFilter, result);
	} else {
		GLfloat mipmapBlend = GlesFracf(lambda_);
		Vec4f lower, higher;
		ImageSample2D(&image[lambda], 
					  coords->x, base->wrapS,
					  coords->y, base->wrapT, 
					  sampleFilter, &lower);
		ImageSample2D(&image[lambda], 
					  coords->x, base->wrapS,
					  coords->y, base->wrapT, 
					  sampleFilter, &higher);
		
		result->x = lower.x * (1.0f - mipmapBlend) + higher.x * mipmapBlend;
		result->y = lower.y * (1.0f - mipmapBlend) + higher.y * mipmapBlend;
		result->z = lower.z * (1.0f - mipmapBlend) + higher.z * mipmapBlend;
		result->w = lower.w * (1.0f - mipmapBlend) + higher.w * mipmapBlend;
	}
}
                         
/**
 * Sample a 2D texture at the given location.
 * 
 * NOTE: This function will actually change to a signature that will
 * process a 2 by 2 block of pixels in order to perform proper mipmapping.
 * 
 * @param unit
 * 		texture image unit
 * @param coord
 * 		texture coordinates at which to sample, the w coordinate contains 
 * 		the lod bias to apply, or the lod if no derivatives are given
 * @param dx
 * 		the derivative of the coord vector with regard to x
 * @param dy
 * 		the derivative of the coord vector with regard to y
 * @param result
 * 		where to store fetched result values
 */
void GlesTextureSample2D(const TextureImageUnit * unit, const Vec4f * coords,
						 const Vec4f * dx, const Vec4f * dy,
                         Vec4f * result) {
	GLES_ASSERT(unit && unit->boundTexture &&
				unit->boundTexture->base.textureType == GL_TEXTURE_2D);

	Texture2D * texture = &unit->boundTexture->texture2D;
	TextureSample2D(&texture->base, texture->image, coords, dx, dy, result);
}

/**
 * Sample a 3D texture at the given location.
 * 
 * NOTE: This function will actually change to a signature that will
 * process a 2 by 2 block of pixels in order to perform proper mipmapping.
 * 
 * @param unit
 * 		texture image unit
 * @param coord
 * 		texture coordinates at which to sample, the w coordinate contains 
 * 		the lod bias to apply, or the lod if no derivatives are given
 * @param dx
 * 		the derivative of the coord vector with regard to x
 * @param dy
 * 		the derivative of the coord vector with regard to y
 * @param result
 * 		where to store fetched result values
 */
                         
void GlesTextureSample3D(const TextureImageUnit * unit, const Vec4f * coords,
						 const Vec4f * dx, const Vec4f * dy,
                         Vec4f * result) {
	GLES_ASSERT(unit && unit->boundTexture &&
				unit->boundTexture->base.textureType == GL_TEXTURE_3D);

	Texture3D * texture = &unit->boundTexture->texture3D;
	
	GLenum mipmapFilter = GetMipmapFilter(texture->base.minFilter);
	GLenum sampleFilter = GetSampleFilter(texture->base.minFilter);
	
	if (mipmapFilter != GL_NONE && !texture->base.isComplete) {
		/* 
		 * texture samples as 0 vector if mipmapping is requested, but texture
		 * is not complete.
		 */
		result->x = result->y = result->z = result->w = 0.0f;
		return;
	}
		
	// determine mipmap level
	GLfloat lambda_ = GlesClampf(coords->w);
	GLint lambda = 0;
	
	// compute lambda'
	if (mipmapFilter != GL_NONE) {
		if (dx && dy) {
			GLsizei width = texture->image[0].width;
			GLsizei height = texture->image[0].height;
			GLsizei depth = texture->image[0].depth;
	
			lambda_ +=
				GlesLog2f(
					GlesMaxf(
						GlesMaxf(width * Hypotenuse(GlesFabsf(dx->x), GlesFabsf(dy->x)),
						   		 height * Hypotenuse(GlesFabsf(dx->y), GlesFabsf(dy->y))),
						depth * Hypotenuse(GlesFabsf(dx->z), GlesFabsf(dy->z))));
		}
							
		lambda = (GLint) lambda_;

		if (lambda_ < (mipmapFilter != GL_NONE && 
					   sampleFilter == GL_NEAREST &&
					   texture->base.magFilter == GL_LINEAR ? 0.5f : 0.0f)) {
			/* magnification; use base mipmap level */
			mipmapFilter = GL_NEAREST;
			sampleFilter = texture->base.magFilter;
			lambda = 0;
		} else if (lambda_ >= texture->base.maxMipmapLevel) {
			/* clip at max level */
			lambda = texture->base.maxMipmapLevel;
			mipmapFilter = GL_NEAREST;
		}
	}
		
	// fetch actual pixel data
	if (mipmapFilter == GL_NONE || mipmapFilter == GL_NEAREST) {
		ImageSample3D(&texture->image[lambda], 
					  coords->x, texture->base.wrapS,
					  coords->y, texture->base.wrapT,
					  coords->z, texture->base.wrapR,
					  sampleFilter, result);
	} else {
		GLfloat mipmapBlend = GlesFracf(lambda_);
		Vec4f lower, higher;
		ImageSample3D(&texture->image[lambda], 
					  coords->x, texture->base.wrapS,
					  coords->y, texture->base.wrapT, 
					  coords->z, texture->base.wrapR,
					  sampleFilter, &lower);
		ImageSample3D(&texture->image[lambda], 
					  coords->x, texture->base.wrapS,
					  coords->y, texture->base.wrapT, 
					  coords->z, texture->base.wrapR,
					  sampleFilter, &higher);
		
		result->x = lower.x * (1.0f - mipmapBlend) + higher.x * mipmapBlend;
		result->y = lower.y * (1.0f - mipmapBlend) + higher.y * mipmapBlend;
		result->z = lower.z * (1.0f - mipmapBlend) + higher.z * mipmapBlend;
		result->w = lower.w * (1.0f - mipmapBlend) + higher.w * mipmapBlend;
	}
}                         

/**
 * This is a small helper function to calculate the derivative after the
 * coordinate transformation in GlesTextureSampleCube.
 * 
 * For now we will assume that the compiler will factor out the computation
 * of the divisions after inlining.
 * 
 * @param f
 * 		the transformed coordinate
 * @param dfdx
 * 		the derivative of the transformed coordinate
 * @param g
 * 		the coordinate used to transform f
 * @param dgdx
 * 		the derivative of the coordinate used to transform f
 * @return
 * 		the derivative for the transformed coordinate
 */
static GLES_INLINE GLfloat Derivative(GLfloat f, GLfloat dfdx, GLfloat g, GLfloat dgdx) {
	GLfloat invG = (1.0f / g), invG2 = invG * invG;
	return 0.5f * (g * dgdx * -invG2 + dfdx * invG);  
}

/**
 * Sample a cube texture at the given location.
 * 
 * NOTE: This function will actually change to a signature that will
 * process a 2 by 2 block of pixels in order to perform proper mipmapping.
 * 
 * @param unit
 * 		texture image unit
 * @param coord
 * 		texture coordinates at which to sample, the w coordinate contains 
 * 		the lod bias to apply, or the lod if no derivatives are given
 * @param dx
 * 		the derivative of the coord vector with regard to x
 * @param dy
 * 		the derivative of the coord vector with regard to y
 * @param result
 * 		where to store fetched result values
 */
                         
void GlesTextureSampleCube(const TextureImageUnit * unit, const Vec4f * coords,
						   const Vec4f * dx, const Vec4f * dy,
                           Vec4f * result) {
	GLES_ASSERT(unit && unit->boundTexture &&
				unit->boundTexture->base.textureType == GL_TEXTURE_CUBE_MAP);
	
	Vec4f scaledCoords, scaledDx, scaledDy;
	Vec3f absCoords;
	
	const TextureCube * texture = &unit->boundTexture->textureCube;
	const Image2D * image = NULL;
	
	absCoords.x = GlesFabsf(coords->x);
	absCoords.y = GlesFabsf(coords->y);
	absCoords.z = GlesFabsf(coords->z);
		
	if (absCoords.x >= absCoords.y && absCoords.x >= absCoords.z) {
		if (coords->x >= 0.0f) {
			/* GL_TEXTURE_CUBE_MAP_POSITIVE_X */
			GLfloat maxa = coords->x;
			GLfloat invMaxa = 1.0f / maxa;

			image = texture->positiveX;
			scaledCoords.x = 0.5f * (-coords->z * invMaxa + 1.0f);
			scaledCoords.y = 0.5f * (-coords->y * invMaxa + 1.0f);
			
			if (dx) {
				scaledDx.x = Derivative(-coords->z, -dx->z, coords->x, dx->x);
				scaledDx.y = Derivative(-coords->y, -dx->y, coords->x, dx->x);
			}
			
			if (dy) {
				scaledDx.x = Derivative(-coords->z, -dy->z, coords->x, dy->x);
				scaledDx.y = Derivative(-coords->y, -dy->y, coords->x, dy->x);
			}			
		} else {
			/* GL_TEXTURE_CUBE_MAP_NEGATIVE_X */
			GLfloat maxa = -coords->x;
			GLfloat invMaxa = 1.0f / maxa;

			image = texture->negativeX;
			scaledCoords.x = 0.5f * ( coords->z * invMaxa + 1.0f);
			scaledCoords.y = 0.5f * (-coords->y * invMaxa + 1.0f);

			if (dx) {
				scaledDx.x = Derivative( coords->z,  dx->z, -coords->x, -dx->x);
				scaledDx.y = Derivative(-coords->y, -dx->y, -coords->x, -dx->x);
			}
			
			if (dy) {
				scaledDx.x = Derivative( coords->z,  dy->z, -coords->x, -dy->x);
				scaledDx.y = Derivative(-coords->y, -dy->y, -coords->x, -dy->x);
			}
		}
	} else if (absCoords.y > absCoords.x && absCoords.y >= absCoords.z) {
		if (coords->y >= 0.0f) {
			/* GL_TEXTURE_CUBE_MAP_POSITIVE_Y */
			GLfloat maxa = coords->y;
			GLfloat invMaxa = 1.0f / maxa;

			image = texture->positiveY;
			scaledCoords.x = 0.5f * (coords->x * invMaxa + 1.0f);
			scaledCoords.y = 0.5f * (coords->z * invMaxa + 1.0f);

			if (dx) {
				scaledDx.x = Derivative(coords->x, dx->x, coords->y, dx->y);
				scaledDx.y = Derivative(coords->z, dx->z, coords->y, dx->y);
			}
			
			if (dy) {
				scaledDx.x = Derivative(coords->x, dy->x, coords->y, dy->y);
				scaledDx.y = Derivative(coords->z, dy->z, coords->y, dy->y);
			}
		} else {
			/* GL_TEXTURE_CUBE_MAP_NEGATIVE_Y */
			GLfloat maxa = -coords->y;
			GLfloat invMaxa = 1.0f / maxa;
		
			image = texture->negativeY;
			scaledCoords.x = 0.5f * ( coords->x * invMaxa + 1.0f);
			scaledCoords.y = 0.5f * (-coords->z * invMaxa + 1.0f);

			if (dx) {
				scaledDx.x = Derivative( coords->x,  dx->x, -coords->y, -dx->y);
				scaledDx.y = Derivative(-coords->z, -dx->z, -coords->y, -dx->y);
			}
			
			if (dy) {
				scaledDx.x = Derivative( coords->x,  dy->x, -coords->y, -dy->y);
				scaledDx.y = Derivative(-coords->z, -dy->z, -coords->y, -dy->y);
			}
		}
	} else {
		if (coords->z >= 0.0f) {
			/* GL_TEXTURE_CUBE_MAP_POSITIVE_Z */
			GLfloat maxa = coords->z;
			GLfloat invMaxa = 1.0f / maxa;
		
			image = texture->positiveZ;
			scaledCoords.x = 0.5f * ( coords->x * invMaxa + 1.0f);
			scaledCoords.y = 0.5f * (-coords->y * invMaxa + 1.0f);

			if (dx) {
				scaledDx.x = Derivative( coords->x,  dx->x, coords->z, dx->z);
				scaledDx.y = Derivative(-coords->y, -dx->y, coords->z, dx->z);
			}
			
			if (dy) {
				scaledDx.x = Derivative( coords->x,  dy->x, coords->z, dy->z);
				scaledDx.y = Derivative(-coords->y, -dy->y, coords->z, dy->z);
			}
		} else {
			/* GL_TEXTURE_CUBE_MAP_NEGATIVE_Z */
			GLfloat maxa = -coords->z;
			GLfloat invMaxa = 1.0f / maxa;
		
			image = texture->negativeZ;
			scaledCoords.x = 0.5f * (-coords->x * invMaxa + 1.0f);
			scaledCoords.y = 0.5f * (-coords->y * invMaxa + 1.0f);

			if (dx) {
				scaledDx.x = Derivative(-coords->x, -dx->x, -coords->z, -dx->z);
				scaledDx.y = Derivative(-coords->y, -dx->y,  coords->z, -dx->z);
			}
			
			if (dy) {
				scaledDx.x = Derivative(-coords->x, -dy->x, -coords->z, -dy->z);
				scaledDx.y = Derivative(-coords->y, -dy->y, -coords->z, -dy->z);
			}
		}
	}
	
	scaledCoords.z = 0.0f;
	scaledCoords.w = coords->w;
	
	TextureSample2D(&texture->base, image, &scaledCoords, 
					dx ? &scaledDx : NULL, 
					dy ? &scaledDy : NULL, result);

}

/*
** --------------------------------------------------------------------------
** Framebuffer-related API
**
** The specification keeps this API call in the section about framebuffer
** manipulation, but it is really almost the same as fetching a texture image
** from the current rendering surface.
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {
	State * state = GLES_GET_STATE();

	GLsizei pixelSize;
	GLenum surfaceFormat;
	GLenum internalFormat;
	GLenum baseInternalFormat;

	/************************************************************************/
	/* Determine surface format and destination								*/
	/************************************************************************/

	if (state->readSurface == NULL) {
		GlesRecordInvalidOperation(state);
		return;
	}

	internalFormat = GetInternalFormat(state, format, type);
	
	if (internalFormat == GL_NONE) {
		return;
	}
	
	baseInternalFormat = GetBaseInternalFormat(internalFormat);
	surfaceFormat = state->readSurface->colorFormat;

	if (baseInternalFormat != GetBaseInternalFormat(surfaceFormat)) {
		GlesRecordInvalidValue(state);
		return;
	}

	pixelSize = GetPixelSize(surfaceFormat);

	/************************************************************************/
	/* Verify surface dimensions											*/
	/************************************************************************/

	if (width == 0 || height == 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (width + x > state->readSurface->size.width || height + y > state->readSurface->size.height) {
		GlesRecordInvalidValue(state);
		return;
	}

	/************************************************************************/
	/* Copy the actual image data											*/
	/************************************************************************/

	state->readSurface->vtbl->lock(state->readSurface);
	CopyPixels(state->readSurface->colorBuffer, 
			   state->readSurface->size.width, state->readSurface->size.height, 1,
			   x, y, 0, width, height, 1,
			   pixels, width, height, 1, 0, 0, 0, 
			   baseInternalFormat, surfaceFormat, internalFormat, 1, 
			   state->unpackAlignment);
	state->readSurface->vtbl->unlock(state->readSurface);
}

