/*
** ==========================================================================
**
** $Id: anonymous_struct.vert 31 2007-08-19 07:49:45Z hmwill $			
** 
** Do not accept anonymous structure declarations
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-08-19 00:49:45 -0700 (Sun, 19 Aug 2007) $
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


struct S 
{ 
  int x; 
}; 

struct T 
{ 
  S; // Error: anonymous structures are disallowed. 
  int y; 
}; 

void main(void) {

}