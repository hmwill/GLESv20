/*
** ==========================================================================
**
** $Id: frontend.c 69 2007-09-27 06:15:41Z hmwill $			
** 
** Compiler front end testing
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-26 23:15:41 -0700 (Wed, 26 Sep 2007) $
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
#include "frontend.h"
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

static void FailEmptyVertexShader() {
	UtilFailCompile("frontend/empty.shader", GL_VERTEX_SHADER, "S0029");
}

static void FailEmptyFragmentShader() {
	UtilFailCompile("frontend/empty.shader", GL_FRAGMENT_SHADER, "S0029");
}

static void SucceedMinimalVertexShader() {
	UtilSucceedCompile("frontend/minimal.vert", GL_VERTEX_SHADER);
}

static void SucceedMinimalFragmentShader() {
	UtilSucceedCompile("frontend/minimal.frag", GL_FRAGMENT_SHADER);
}

static void FailMainInvalidReturnType() {
	UtilFailCompile("frontend/main_invalid_return_type.vert", 
		GL_VERTEX_SHADER, "S0029");
}

static void FailMainInvalidParameters() {
	UtilFailCompile("frontend/main_invalid_parameters.vert", 
		GL_VERTEX_SHADER, "S0029");
}

static void SucceedBasicTypes() {
	UtilSucceedCompile("frontend/basic_types.vert", GL_VERTEX_SHADER);
}

static void SucceedSpecExample() {
	UtilSucceedCompile("frontend/spec_example.vert", GL_VERTEX_SHADER);
}

static void FailAnonymousStruct() {
	UtilFailCompile("frontend/anonymous_struct.vert", 
		GL_VERTEX_SHADER, "L0001");
}

static void FailEmbeddedStruct() {
	UtilFailCompile("frontend/embedded_struct.vert", 
		GL_VERTEX_SHADER, "L0001");
}

static void FailLeftHandComponent() {
	UtilFailCompile("frontend/left_hand_component.vert", 
		GL_VERTEX_SHADER, "S0037");
}

static void FailComponentMismatch() {
	UtilFailCompile("frontend/component_mismatch.vert", 
		GL_VERTEX_SHADER, "S0001");
}

static void SucceedLink() {
	UtilSucceedCompileLink("frontend/link_test.vert", "frontend/link_test.frag");
}

/**
 * Register all compiler front end tests
 */
GLboolean TestRegisterFrontend() {
	CU_pSuite pSuite = CU_add_suite("Compiler Frontend", SetupFixture, CleanupFixture);
	
	if (!pSuite) {
		return GL_FALSE;
	}
	
	if (!CU_add_test(pSuite, "Minimal Vertex Shader", 	SucceedMinimalVertexShader) 	||
		!CU_add_test(pSuite, "Minimal Fragment Shader", SucceedMinimalFragmentShader)	||
		!CU_add_test(pSuite, "Empty Vertex Shader", 	FailEmptyVertexShader) 			||
		!CU_add_test(pSuite, "Empty Fragment Shader", 	FailEmptyFragmentShader) 		||
		!CU_add_test(pSuite, "Main Return Type", 		FailMainInvalidReturnType) 		||
		!CU_add_test(pSuite, "Main Parameters", 		FailMainInvalidParameters) 		||
		!CU_add_test(pSuite, "Basic Types", 			SucceedBasicTypes) 				||
		!CU_add_test(pSuite, "Specification Examples", 	SucceedSpecExample) 			||
		!CU_add_test(pSuite, "Anonymous Struct", 		FailAnonymousStruct) 			||
		!CU_add_test(pSuite, "Embedded Struct", 		FailEmbeddedStruct) 			||
		!CU_add_test(pSuite, "Left Hand Component", 	FailLeftHandComponent) 			||
		!CU_add_test(pSuite, "Component Mismatch", 		FailComponentMismatch) 			||
		!CU_add_test(pSuite, "Link Test",		 		SucceedLink)) {
		return GL_FALSE;
	}
	
	return GL_TRUE;
}