/*
** ==========================================================================
**
** $Id: orange.c 37 2007-08-22 02:56:46Z hmwill $			
** 
** Example shaders from orange book
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-08-21 19:56:46 -0700 (Tue, 21 Aug 2007) $
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
#include <vin.h>
#include <stdlib.h>
#include "CUnit/Basic.h"
#include "orange.h"
#include "utils.h"


static int SetupFixture() {
	if (!vinInitialize()) {
		return CUE_SINIT_FAILED;
	} else {
		return CUE_SUCCESS;
	}
}

static int CleanupFixture() {
	if (!vinTerminate()) {
		return CUE_SCLEAN_FAILED;
	} else {
		return CUE_SUCCESS;
	}
}

#define VERTEX_SHADER(name, file) \
static void name() { UtilSucceedCompile("orange/" file, GL_VERTEX_SHADER); }

#include "orange_vertex.inc"

#undef VERTEX_SHADER

#define VERTEX_SHADER(name, file) \
if (!CU_add_test(pSuite, file, name)) return GL_FALSE;

static GLboolean AddVertexShaderTests(CU_pSuite pSuite) {
#	include "orange_vertex.inc"	
	return GL_TRUE;
}

#undef VERTEX_SHADER

#define FRAGMENT_SHADER(name, file) \
static void name() { UtilSucceedCompile("orange/" file, GL_FRAGMENT_SHADER); }

#include "orange_fragment.inc"

#undef FRAGMENT_SHADER

#define FRAGMENT_SHADER(name, file) \
if (!CU_add_test(pSuite, file, name)) return GL_FALSE;

static GLboolean AddFragmentShaderTests(CU_pSuite pSuite) {
#	include "orange_fragment.inc"	
	return GL_TRUE;
}

#undef FRAGMENT_SHADER


/**
 * Register all compiler front end tests
 */
GLboolean TestRegisterOrange() {
	CU_pSuite pSuite = CU_add_suite("Orange Book Tests", SetupFixture, CleanupFixture);
	
	if (!pSuite) {
		return GL_FALSE;
	}
	
	if (!AddVertexShaderTests(pSuite) ||
		!AddFragmentShaderTests(pSuite)) {
		return GL_FALSE;
	}
	
	return GL_TRUE;
}