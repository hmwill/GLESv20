/*
** ==========================================================================
**
** $Id: log.c 60 2007-09-18 01:16:07Z hmwill $
**
** Compiler and Linker Log functions
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
** Internal functions
** --------------------------------------------------------------------------
*/

void GlesLogInit(Log * log) {
	GlesMemset(log, 0, sizeof(Log));
}

void GlesLogDeInit(Log * log) {
	LogRecord * current, * next;

	for (current = log->first; current; current = next) {
		next = current->next;
		GlesFree(current);
	}

	GlesMemset(log, 0, sizeof(Log));
}

void GlesLogClear(Log * log) {
	LogRecord * current, * next;

	/* truncate to a single page */

	if (log->first) {
		for (current = log->first->next; current; current = next) {
			next = current->next;
			GlesFree(current);
		}
		
		log->last = log->first;
		log->blockSize = 0;
		log->logSize = 0;
	} else {
		GLES_ASSERT(!log->last);
		GLES_ASSERT(!log->blockSize);
		GLES_ASSERT(!log->logSize);
	}
}

void GlesLogAppend(Log * log, const char * text, GLsizei length) {
	GLsizei remain;
	
	if (!log) {
		return;
	}
	
	remain = log->last ? GLES_LOG_BLOCK_SIZE - log->blockSize : 0;
	
	if (length < 0) {
		length = GlesStrlen(text);
	}
	
	if (remain) {
		if (length <= remain) {
			GlesMemcpy(log->last->text + log->blockSize, text, length);
			log->blockSize += length;
			log->logSize += length;
			return;
		} else {
			GlesMemcpy(log->last->text + log->blockSize, text, remain);
			log->blockSize = GLES_LOG_BLOCK_SIZE;
			length -= remain;
			log->logSize += remain;
			text += remain;
		}
	}
	
	/* we need to allocate more blocks */
	
	while (length >= 0) {
		LogRecord * newRecord = GlesMalloc(sizeof(LogRecord));
		
		if (!newRecord) {
			GlesRecordOutOfMemory(GLES_GET_STATE());
			return;
		}
		
		if (log->last) {
			log->last->next = newRecord;
			log->last = newRecord;
		} else {
			GLES_ASSERT(!log->first);
			log->first = log->last = newRecord;
		}
		
		if (length <= GLES_LOG_BLOCK_SIZE) {
			GlesMemcpy(newRecord->text, text, length);
			log->blockSize = length;
			log->logSize += length;
			return;
		} else {
			GlesMemcpy(newRecord->text, text, GLES_LOG_BLOCK_SIZE);
			log->logSize += GLES_LOG_BLOCK_SIZE;
			text += GLES_LOG_BLOCK_SIZE;
			length -= GLES_LOG_BLOCK_SIZE;
		}
	}
}

void GlesLogExtract(Log * log, GLsizei bufsize, char * text, GLsizei * length) {
	LogRecord * record;
	
	if (log->logSize + 1 < bufsize) {
		bufsize = log->logSize;
	}
	
	if (length) {
		*length = bufsize - 1;
	}

	for (record = log->first; bufsize > 0; record = record->next) {
		if (bufsize <= GLES_LOG_BLOCK_SIZE) {
			GlesMemcpy(text, record->text, bufsize - 1);
			text += bufsize - 1;
			*text = '\0';
			break;
		} else {
			GlesMemcpy(text, record->text, GLES_LOG_BLOCK_SIZE);
			text += GLES_LOG_BLOCK_SIZE;
			bufsize -= GLES_LOG_BLOCK_SIZE;
		}
	}
}

void GlesLogAppendLog(Log * log, const  Log * sourceLog) {
	LogRecord * record;
	GLsizeiptr remain = sourceLog->logSize;
	
	for (record = sourceLog->first; remain; record = record->next, 
		remain -= GLES_LOG_BLOCK_SIZE) {
		if (remain > GLES_LOG_BLOCK_SIZE) {
			GlesLogAppend(log, record->text, GLES_LOG_BLOCK_SIZE);			
		} else {
			GlesLogAppend(log, record->text, remain);
			return;
		}
	}
}
