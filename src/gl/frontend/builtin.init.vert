/*
** ==========================================================================
**
** $Id: builtin.vert 38 2007-08-22 03:29:12Z hmwill $
** 
** Shading Language Front-End: Declarations for built-in functions and symbols 
** available in vertex shader
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-08-21 20:29:12 -0700 (Tue, 21 Aug 2007) $
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

/*
** --------------------------------------------------------------------------
** 4.5.3 Default Precision Qualifiers 
** --------------------------------------------------------------------------
*/

precision highp float; 
precision highp int; 
precision lowp sampler2D; 
precision lowp sampler3D; 
precision lowp samplerCube;

