/*
** ==========================================================================
**
** $Id: vin.hv 1.3 2006/11/21 15:49:18 hmwill Exp $
**
** Use custom SDL bindings until EGL is a stable and meaningful target
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2006/11/21 15:49:18 $
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
#include <SDL.h>

typedef struct VinSurface * VinSurface;



GL_API GLboolean GL_APIENTRY vinInitialize (void);
GL_API GLboolean GL_APIENTRY vinTerminate (void);
GL_API void (* GL_APIENTRY vinGetProcAddress (const char *procname))() ;
/*GL_API VinSurface GL_APIENTRY vinCreateSurface (SDL_Surface * surface, GLenum depthFormat, GLenum stencilFormat);*/
GL_API GLboolean GL_APIENTRY vinDestroySurface (VinSurface surface);
GL_API GLboolean GL_APIENTRY vinMakeCurrent (VinSurface draw, VinSurface read);
GL_API VinSurface GL_APIENTRY vinGetReadSurface (void);
GL_API VinSurface GL_APIENTRY vinGetWriteSurface (void);

