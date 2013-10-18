/*
** ==========================================================================
**
** $Id: file.c 60 2007-09-18 01:16:07Z hmwill $			
** 
** Utility functions around files for writing test cases
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


#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define PATH_SEPARATOR 	":"
#define FILE_SEPARATOR 	"/"


/**
 * Try to open the file using fopen in any of the directories listed in the
 * given search path.
 * 
 * @param	filename
 * 			the name of the file to open
 * @param	mode
 * 			the mode used as second argument to fopen
 * @param	path
 * 			a string of path entries specifying a list of directories to be
 * 			scanned for the given file.
 * 
 * @return	an open FILE * or NULL if the file could not be opened
 */
FILE * UtilFopenPath(const char * filename, const char * mode, const char * path) {
	
	size_t pathLength = strlen(path);
	char * copyPath = malloc(pathLength + 1), *component;
	FILE * file = NULL;
	
	if (!copyPath) {
		fprintf(stderr, "Fatal error: Out of memory in %s (%d)\n", __FILE__, __LINE__);
		return NULL;
	}
	
	strcpy(copyPath, path);
	
	while ((component = strsep(&copyPath, PATH_SEPARATOR)) != NULL) {
		if (component[0]) {
			char buffer[FILENAME_MAX];
			
			strcpy(buffer, component);
			strcat(buffer, FILE_SEPARATOR);
			strcat(buffer, filename);
			file = fopen(buffer, mode);
		} else {
			file = fopen(filename, mode);
		}
		
		if (file) {
			break;
		}
	}
	
	free(copyPath);
	
	return file;
}
