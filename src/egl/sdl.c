/*
** ==========================================================================
**
** $Id: sdl.c 60 2007-09-18 01:16:07Z hmwill $
**
** Use custom SDL bindings until EGL is a stable and meaningful target
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
#include <SDL/SDL.h>
#include "config.h"
#include "platform/platform.h"
#include "gl/state.h"

typedef struct VinSurface * VinSurface;

typedef struct SdlSurfaceWrapper {
	Surface			surface;
	SDL_Surface *	sdlSurface;
	GLint			refcount;
} SdlSurfaceWrapper;

static void AddrefSurface(struct Surface * surface) {
	SdlSurfaceWrapper * wrapper = (SdlSurfaceWrapper *) surface;
	
	++wrapper->refcount;
}
static void ReleaseSurface(struct Surface * surface) {
	SdlSurfaceWrapper * wrapper = (SdlSurfaceWrapper *) surface;
	
	if (--wrapper->refcount == 0) {
		if (wrapper->surface.depthBuffer) {
			GlesFree(wrapper->surface.depthBuffer);
		}
		
		if (wrapper->surface.stencilBuffer) {
			GlesFree(wrapper->surface.stencilBuffer);
		}
		
		GlesFree(wrapper);
	}
}

static void LockSurface(struct Surface * surface) {
	SdlSurfaceWrapper * wrapper = (SdlSurfaceWrapper *) surface;
	
	if (SDL_MUSTLOCK(wrapper->sdlSurface) && !wrapper->sdlSurface->locked) {
		SDL_LockSurface(wrapper->sdlSurface);
	}

	wrapper->surface.colorBuffer = 
		(GLubyte *) (wrapper->sdlSurface->pixels) + 
			wrapper->sdlSurface->pitch * (wrapper->sdlSurface->h - 1);	
	
	wrapper->surface.viewport.x = wrapper->sdlSurface->clip_rect.x;
	wrapper->surface.viewport.y = wrapper->sdlSurface->clip_rect.y;
	wrapper->surface.viewport.width = wrapper->sdlSurface->clip_rect.w;
	wrapper->surface.viewport.height = wrapper->sdlSurface->clip_rect.h;
}

static void UnlockSurface(struct Surface * surface) {
	SdlSurfaceWrapper * wrapper = (SdlSurfaceWrapper *) surface;

	if (SDL_MUSTLOCK(wrapper->sdlSurface) && wrapper->sdlSurface->locked) {
		SDL_UnlockSurface(wrapper->sdlSurface);
	}

	wrapper->surface.colorBuffer = NULL;	
}

static SurfaceVtbl Vtbl = {
	&AddrefSurface,
	&ReleaseSurface,
	&LockSurface,
	&UnlockSurface
};

GL_API GLboolean GL_APIENTRY vinInitialize (void) {
	GlesInitState(GlesGetGlobalState());
	
	return GL_TRUE;
}

GL_API GLboolean GL_APIENTRY vinTerminate (void) {
	GlesDeInitState(GlesGetGlobalState());
	
	return GL_TRUE;
}

GL_API void (* GL_APIENTRY vinGetProcAddress (const char *procname))() {
	return NULL;
}

GL_API VinSurface GL_APIENTRY vinCreateSurface (SDL_Surface * surface, GLenum depthFormat, GLenum stencilFormat) {
	SdlSurfaceWrapper * wrapper = NULL;
	GLuint depthBits;
	GLuint stencilBits;
	GLuint redBits, greenBits, blueBits, alphaBits;
	GLenum colorFormat = GL_INVALID_VALUE;
	GLenum colorReadFormat = GL_INVALID_VALUE;
	GLenum colorReadType = GL_INVALID_VALUE;		
	GLint depthPitch, stencilPitch;

	switch (depthFormat) {
	case GL_DEPTH_COMPONENT16:	depthBits = 16;	break;
	case GL_DEPTH_COMPONENT24:	/* depthBits = 24; break; */
	case GL_DEPTH_COMPONENT32:	depthBits = 32; break;
	default:
		return NULL;
	}

	depthPitch = depthBits * surface->w;
	
	switch (stencilFormat) {
	case GL_STENCIL_INDEX1_OES:	stencilBits = 1; break;
	case GL_STENCIL_INDEX4_OES:	stencilBits = 4; break;
	case GL_STENCIL_INDEX8_OES:	stencilBits = 8; break;
	default:
		return NULL;
	}
		
	stencilPitch = ((stencilBits * surface->w) + (GLES_BITS_PER_BYTE - 1)) / GLES_BITS_PER_BYTE;

	redBits 	= 8 - surface->format->Rloss;
	greenBits 	= 8 - surface->format->Gloss;
	blueBits 	= 8 - surface->format->Bloss;
	alphaBits 	= 8 - surface->format->Aloss;
	
	if (redBits == 8 && greenBits == 8 && blueBits == 8) {
		if (alphaBits == 0 && surface->format->BytesPerPixel == 3) {
			colorFormat = GL_RGB8;
			colorReadFormat = GL_RGB;
			colorReadType = GL_UNSIGNED_BYTE;
		} else if (alphaBits == 8 || surface->format->BytesPerPixel == 4) {
			colorFormat = GL_RGBA8;
			colorReadFormat = GL_RGBA;
			colorReadType = GL_UNSIGNED_BYTE;
		} else {
			return NULL;
		}
	} else if (redBits == 5 && greenBits == 6 && blueBits == 5 && alphaBits == 0) {
		colorFormat = GL_RGB565_OES;
		colorReadFormat = GL_RGB;
		colorReadType = GL_UNSIGNED_SHORT_5_6_5;
	} else if (redBits == 5 && greenBits == 5 && blueBits == 5 && alphaBits == 1) {
		colorFormat = GL_RGB5_A1;
		colorReadFormat = GL_RGBA;
		colorReadType = GL_UNSIGNED_SHORT_5_5_5_1;
	} else if (redBits == 4 && greenBits == 4 && blueBits == 4 && alphaBits == 4) {
		colorFormat = GL_RGBA4;
		colorReadFormat = GL_RGBA;
		colorReadType = GL_UNSIGNED_SHORT_4_4_4_4;
	} else {
		return NULL;
	}
	
	wrapper = GlesMalloc(sizeof(SdlSurfaceWrapper));

	if (wrapper) {
		wrapper->sdlSurface = surface;
		wrapper->surface.vtbl = &Vtbl;
		
		wrapper->surface.size.width = surface->w;
		wrapper->surface.size.height = surface->h;
		wrapper->surface.colorPitch = -surface->pitch;
		wrapper->surface.depthPitch = depthPitch;
		wrapper->surface.stencilPitch = stencilPitch;

		wrapper->surface.colorFormat = colorFormat;
		wrapper->surface.colorReadFormat = colorReadFormat;
		wrapper->surface.colorReadType = colorReadType;
		wrapper->surface.depthFormat = depthFormat;
		wrapper->surface.stencilFormat = stencilFormat;
		
		wrapper->surface.redBits 	= redBits;
		wrapper->surface.greenBits 	= greenBits;
		wrapper->surface.blueBits 	= blueBits;
		wrapper->surface.alphaBits 	= alphaBits;
		wrapper->surface.depthBits 	= depthBits;
		wrapper->surface.stencilBits = stencilBits;

		wrapper->surface.depthBuffer = GlesMalloc(surface->h * depthPitch);
		
		if (!wrapper->surface.depthBuffer) {
			GlesFree(wrapper);
			return NULL;
		}
		
		wrapper->surface.stencilBuffer = GlesMalloc(surface->h * stencilPitch);
		
		if (!wrapper->surface.stencilBuffer) {
			GlesFree(wrapper->surface.depthBuffer);
			GlesFree(wrapper);
			return NULL;
		}
		
		wrapper->refcount = 1;
		
		return (VinSurface) &wrapper->surface;
	} else {
		return NULL;
	}
}

GL_API GLboolean GL_APIENTRY vinDestroySurface (VinSurface surface) {
	SdlSurfaceWrapper * wrapper = (SdlSurfaceWrapper *) surface;
	wrapper->surface.vtbl->release(&wrapper->surface);
	
	return GL_TRUE;
}

GL_API GLboolean GL_APIENTRY vinMakeCurrent (VinSurface draw, VinSurface read) {
	State * state = GlesGetGlobalState();
	Surface * readSurface = (Surface *) read;
	Surface * writeSurface = (Surface *) draw;
		
	if (readSurface) {
		readSurface->vtbl->addref(readSurface);
	}
	
	if (writeSurface) {
		writeSurface->vtbl->addref(writeSurface);
	}
	
	if (state->readSurface) {
		state->readSurface->vtbl->release(state->readSurface);
	}
	
	if (state->writeSurface) {
		state->writeSurface->vtbl->release(state->writeSurface);
	} else {
		glViewport(0, 0, writeSurface->size.width, writeSurface->size.height);
	}
	
	state->readSurface = readSurface;
	state->writeSurface = writeSurface;
	
	return GL_TRUE;
}

GL_API VinSurface GL_APIENTRY vinGetReadSurface (void) {
	State * state = GlesGetGlobalState();
	
	return (VinSurface) state->readSurface;
}

GL_API VinSurface GL_APIENTRY vinGetWriteSurface (void) {
	State * state = GlesGetGlobalState();
	
	return (VinSurface) state->writeSurface;
}


