/*
** ==========================================================================
**
** $Id: testmain.c 69 2007-09-27 06:15:41Z hmwill $			
** 
** Main module for testing framework
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
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "frontend/frontend.h"
#include "orange/orange.h"
#include "utils.h"

GLboolean interactive = GL_FALSE;

UtilOption options[] = {
	{ "i", UtilOptionBoolean, &interactive },
	{ NULL, UtilOptionNone, NULL }
};

int main(int argc, char * argv[]) {

	CU_pSuite pSuite = NULL;
	CU_pTest pTest = NULL;
	GLint nextArg = UtilGetOpts(argc, argv, options);
	
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "Could not initialize SDL video sub-system. Exiting tests...\n");
		return EXIT_FAILURE;
	}
	
	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry()) {
		goto cleanup_sdl;
	}

	/* add test suites to the registry */
	if (!TestRegisterFrontend() /*||
		!TestRegisterOrange()*/) {
		goto cleanup;
	}

	if (interactive) {
		CU_console_run_tests();
	} else {
		/* Run all tests using the CUnit Basic interface */
		CU_basic_set_mode(CU_BRM_VERBOSE);
	
		if (nextArg <= argc && argv[nextArg]) {
			pSuite = CU_get_suite_by_name(argv[nextArg], CU_get_registry());
		
			if (!pSuite) {
				fprintf(stderr, "Could not find test suite %s\n", argv[nextArg]);
				goto cleanup;
			}
			
			++nextArg;
		}
	
		if (nextArg <= argc && argv[nextArg]) {
			pTest = CU_get_test_by_name(argv[nextArg], pSuite);
		
			if (!pTest) {
				fprintf(stderr, "Could not find test case %s\n", argv[nextArg]);
				goto cleanup;
			}
			
			++nextArg;
		}
	
		if (pTest) {
			CU_basic_run_test(pSuite, pTest);
		} else if (pSuite) {
			CU_basic_run_suite(pSuite);
		} else {
			CU_basic_run_tests();
		}
	
		CU_basic_show_failures(CU_get_failure_list());
	}
	
cleanup:
	CU_cleanup_registry();
	
cleanup_sdl:
	SDL_Quit();
	
	return CU_get_error();
}