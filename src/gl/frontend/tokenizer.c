/*
** ==========================================================================
**
** $Id: tokenizer.c 62 2007-09-18 23:24:32Z hmwill $
**
** Tokenizer for Shading Language Compiler
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-09-18 16:24:32 -0700 (Tue, 18 Sep 2007) $
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
#include "frontend/memory.h"
#include "frontend/tokenizer.h"
#include "frontend/compiler.h"
#include "frontend/symbols.h"

/*
 * --------------------------------------------------------------------------
 * Module-local data
 * --------------------------------------------------------------------------
 */

#define strncmp GlesStrncmp
#include "frontend/keywords.inc"
#undef strncmp

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

static void UpdateLineMacro(Tokenizer * tokenizer) {
	GLsizei length = 
		GlesSprintf(tokenizer->lineMacroText, "%d", tokenizer->lineno);
		
	GLES_ASSERT(tokenizer->lineMacro);
	GLES_ASSERT(tokenizer->lineMacro->text.first == tokenizer->lineMacroText);
	
	tokenizer->lineMacro->text.length = length;
}

static void UpdateFileMacro(Tokenizer * tokenizer) {
	GLsizei length = 
		GlesSprintf(tokenizer->fileMacroText, "%d", tokenizer->sourceno);
		
	GLES_ASSERT(tokenizer->fileMacro);
	GLES_ASSERT(tokenizer->fileMacro->text.first == tokenizer->fileMacroText);
	
	tokenizer->fileMacro->text.length = length;
}

static GLboolean FetchNextToken(Tokenizer * tokenizer) {
	GLfloat floatValue, fraction;
	GLint value, exponent, expSign;
	char * current = tokenizer->input.current;
	
again:
	tokenizer->token.s.first = current;
	tokenizer->token.s.length = 0;
	
	if (current == tokenizer->input.last) {
		if (tokenizer->input.freepointer) {
			GlesFree(tokenizer->input.freepointer);
			tokenizer->input.freepointer = NULL;
		}
		
		if (tokenizer->inComment) {
			tokenizer->token.tokenType = TokenTypeError;
						
			return GL_FALSE;
		} else if (tokenizer->input.sp >= 0) {
			GLsizei sp = tokenizer->input.sp--;

			current = 
			tokenizer->input.current = tokenizer->input.expansions[sp].current;
			tokenizer->input.last = tokenizer->input.expansions[sp].last;
			tokenizer->input.freepointer = tokenizer->input.expansions[sp].freepointer;
			
			if (tokenizer->input.expansions[sp].macro) {
				tokenizer->input.expansions[sp].macro->disabled = GL_FALSE;
			}
			
			goto again;
		} else {
			tokenizer->token.tokenType = TokenTypeEof;
			
			return GL_TRUE;
		}
	}
	
	if (tokenizer->inComment) { 
		switch (*current) {
		case '\r':
			++current;
			
			if (current != tokenizer->input.last && *current == '\n') {
				++current;
			}
			
			tokenizer->token.tokenType = TokenTypeEol;
			++tokenizer->lineno;
			UpdateLineMacro(tokenizer);
			break;
			
		case '\n':
			++current;
			tokenizer->token.tokenType = TokenTypeEol;
			++tokenizer->lineno;
			UpdateLineMacro(tokenizer);
			break;
			
		case '*':
			++current;
			
			if (current != tokenizer->input.last && *current == '/') {
				++current;
				tokenizer->token.tokenType = TokenTypeSpace;
				tokenizer->inComment = GL_FALSE;
				break;
			} else {			
				goto again;
			}
			
		default:
			++current;
			goto again;
		}
	} else {		
		switch (*current) {
		case ' ':
		case '\t':
			do {
				++current;
			} while (current != tokenizer->input.last && 
						(*current == ' ' || *current == '\t'));
			
			tokenizer->token.tokenType = TokenTypeSpace;
			break;
			
		case '\r':
			++current;
			
			if (current != tokenizer->input.last && *current == '\n') {
				++current;
			}
			
			tokenizer->token.tokenType = TokenTypeEol;
			++tokenizer->lineno;
			UpdateLineMacro(tokenizer);
			break;
			
		case '\n':
			++current;
			tokenizer->token.tokenType = TokenTypeEol;
			++tokenizer->lineno;
			UpdateLineMacro(tokenizer);			
			break;
			
		case 'A': case 'a':
		case 'B': case 'b':
		case 'C': case 'c':
		case 'D': case 'd':
		case 'E': case 'e':
		case 'F': case 'f':
		case 'G': case 'g':
		case 'H': case 'h':
		case 'I': case 'i':
		case 'J': case 'j':
		case 'K': case 'k':
		case 'L': case 'l':
		case 'M': case 'm':
		case 'N': case 'n':
		case 'O': case 'o':
		case 'P': case 'p':
		case 'Q': case 'q':
		case 'R': case 'r':
		case 'S': case 's':
		case 'T': case 't':
		case 'U': case 'u':
		case 'V': case 'v':
		case 'W': case 'w':
		case 'X': case 'x':
		case 'Y': case 'y':
		case 'Z': case 'z':
		case '_':
			do {
				++current;
			} while (current != tokenizer->input.last && 
					 	((*current >= 'a' && *current <= 'z') ||
					 	 (*current >= 'A' && *current <= 'Z') ||
					 	 (*current >= '0' && *current <= '9') ||
					 	 *current == '_'));
			
			tokenizer->token.tokenType = TokenTypeIdentifier;
			break;
			
		case '0': 
			++current;
			value = 0;
			
			if (current != tokenizer->input.last) { 
				if (*current == 'x' || *current == 'X') {
					/* hexadecimal constant */
					
					++current;
					
					if (current != tokenizer->input.last) {
						if (*current >= '0' && *current <= '9') {
							value = *current - '0'; 
						} else if (*current >= 'A' && *current <= 'F') {
							value = 10 + (*current - 'A');
						} else if (*current >= 'a' && *current <= 'f') {
							value = 10 + (*current - 'a');
						} else {
							goto error;
						}
					} else {
						goto error;
					}
					
					++current;
					
					while (current != tokenizer->input.last) {
						if (*current >= '0' && *current <= '9') {
							value = value * 16 + *current - '0'; 
						} else if (*current >= 'A' && *current <= 'F') {
							value = value * 16 + 10 + (*current - 'A');
						} else if (*current >= 'a' && *current <= 'f') {
							value = value * 16 + 10 + (*current - 'a');
						} else {
							break;
						}
						
						++current;
					}				
					
					tokenizer->token.tokenType = TokenTypeIntConstant;
					tokenizer->token.value.i = value;
					
					break;
					
				} else if (*current >= '0' && *current <= '7') {
					/* octal constant */
					
					GLuint value = *current - '0'; 
					
					++current;
					
					while (current != tokenizer->input.last) {
						if (*current >= '0' && *current <= '7') {
							value = value * 8 + *current - '0'; 
						} else {
							break;
						}
						
						++current;
					}				
					
					tokenizer->token.tokenType = TokenTypeIntConstant;
					tokenizer->token.value.i = value;
					
					break;
					
				} else if (*current == '.') {
					value = 0;
					goto fraction;
				}
			}
			
			tokenizer->token.tokenType = TokenTypeIntConstant;
			tokenizer->token.value.i = 0;
	
			break;
			
		case '1':
		case '2': case '3':
		case '4': case '5': 
		case '6': case '7':
		case '8': case '9':
			value = *current - '0';
			++current;
	
			while (current != tokenizer->input.last) {
				if (*current >= '0' && *current <= '9') {
					value = value * 10 + *current - '0'; 
				} else {
					break;
				}
				
				++current;
			}				
			
			if (current == tokenizer->input.last || *current != '.') {
				tokenizer->token.tokenType = TokenTypeIntConstant;
				tokenizer->token.value.i = value;
				break;
			}
			 
		fraction:
			++current;
			floatValue = value;
			fraction = 0.1f;
			exponent = 0;
			expSign = 1;
			
			while (current != tokenizer->input.last) {
				if (*current >= '0' && *current <= '9') {
					floatValue = floatValue + (*current - '0') * fraction;
					fraction *= 0.1f; 
				} else {
					break;
				}
				
				++current;
			}				
					
			if (current != tokenizer->input.last &&
				(*current == 'e' || *current == 'E')) {
				++current;
				
				if (current != tokenizer->input.last) {
					if (*current == '+') {
						++current;
					} else if (*current == '-') {
						++current;
						expSign = -1;
					}
				}
				
				if (current != tokenizer->input.last) {
					if (*current >= '0' && *current <= '9') {
						exponent = *current - '0'; 
					} else {
						goto error;
					}
				} else {
					goto error;
				}
				
				++current;
				
				while (current != tokenizer->input.last) {
					if (*current >= '0' && *current <= '9') {
						exponent = exponent * 10 + *current - '0'; 
					} else {
						break;
					}
					
					++current;
				}				
				
				exponent *= expSign;
				
				tokenizer->token.value.f = floatValue * GlesPowf(10.0f, exponent);			
			} else {
				tokenizer->token.value.f = floatValue;			
			}
			
			tokenizer->token.tokenType = TokenTypeFloatConstant;
			break;
			
		case '.':
			if (current + 1 != tokenizer->input.last &&
				current[1] >= '0' && current[1] <= '9') {
				value = 0;
				goto fraction;
			}
				
			++current;
			tokenizer->token.tokenType = TokenTypeDot;
			break;
			
		case '#':
			++current;
			tokenizer->token.tokenType = TokenTypeHash;
			break;
		 
		case '/':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '/') {
					++current;
					
					while (current != tokenizer->input.last && *current != '\r' && *current != '\n') {
						++current;
					} 
					
					goto again;
				} else if (*current == '*') {
					++current;					
					tokenizer->inComment = GL_TRUE;
					goto again;
				} else if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeDivAssign;
					break;
				} 
			}
	
			tokenizer->token.tokenType = TokenTypeSlash;		
			break;
			
		case '^':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '^') {
					++current;
					tokenizer->token.tokenType = TokenTypeXorOp;
					break;
				} else if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeXorAssign;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeCaret;
			break;
						 
		case '+':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '+') {
					++current;
					tokenizer->token.tokenType = TokenTypeIncOp;
					break;
				} else if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeAddAssign;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypePlus;
			break;
			
		case '-':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '-') {
					++current;
					tokenizer->token.tokenType = TokenTypeDecOp;
					break;
				} else if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeSubAssign;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeDash;
			break;
			
		case '*':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeMulAssign;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeStar;
			break;
			
		case '%':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeModAssign;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypePercent;
			break;
			
		case '=':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeEqualOp;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeEqual;
			break;
			
		case '<':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '<') {
					++current;
					
					if (current != tokenizer->input.last && *current == '=') {
						++current;
						tokenizer->token.tokenType = TokenTypeLeftAssign;
					} else {
						tokenizer->token.tokenType = TokenTypeLeftOp;
					}
					
					break;
				} else if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeLessEqualOp;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeLeftAngle;
			break;
			
		case '>':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '<') {
					++current;
					
					if (current != tokenizer->input.last && *current == '=') {
						++current;
						tokenizer->token.tokenType = TokenTypeRightAssign;
					} else {
						tokenizer->token.tokenType = TokenTypeRightOp;
					}
					
					break;
				} else if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeGreaterEqualOp;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeRightAngle;
			break;
			
		case '!':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeNotEqualOp;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeBang;
			break;
			
		case '~':
			++current;
			tokenizer->token.tokenType = TokenTypeTilde;
			break;
			
		case '&':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '&') {
					++current;
					tokenizer->token.tokenType = TokenTypeAndOp;
					break;
				} else if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeAndAssign;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeAmpersand;
			break;
			
		case '|':
			++current;
			
			if (current != tokenizer->input.last) {
				if (*current == '|') {
					++current;
					tokenizer->token.tokenType = TokenTypeOrOp;
					break;
				} else if (*current == '=') {
					++current;
					tokenizer->token.tokenType = TokenTypeOrAssign;
					break;
				}
			} 
			
			tokenizer->token.tokenType = TokenTypeVerticalBar;
			break;
				
		case '?':
			++current;
			tokenizer->token.tokenType = TokenTypeQuestion;
			break;
		 
		case ':':
			++current;
			tokenizer->token.tokenType = TokenTypeColon;
			break;
		 
		case ',':
			++current;
			tokenizer->token.tokenType = TokenTypeComma;
			break;
		 
		case ';':
			++current;
			tokenizer->token.tokenType = TokenTypeSemicolon;
			break;
		 
		case '(':
			++current;
			tokenizer->token.tokenType = TokenTypeLeftParen;
			break;
		 
		case ')':
			++current;
			tokenizer->token.tokenType = TokenTypeRightParen;
			break;
	
		case '[':
			++current;
			tokenizer->token.tokenType = TokenTypeLeftBracket;
			break;
		 
		case ']':
			++current;
			tokenizer->token.tokenType = TokenTypeRightBracket;
			break;
	
		case '{':
			++current;
			tokenizer->token.tokenType = TokenTypeLeftBrace;
			break;
		 
		case '}':
			++current;
			tokenizer->token.tokenType = TokenTypeRightBrace;
			break;
		 
		default:
		error:
			tokenizer->input.current = current;
			tokenizer->token.tokenType = TokenTypeError;
			return GL_FALSE;
		}
	}
	
	tokenizer->input.current = current;
	tokenizer->token.s.length = current - tokenizer->token.s.first;
	return GL_TRUE;
}

/*
** --------------------------------------------------------------------------
** Internal functions
** --------------------------------------------------------------------------
*/

Tokenizer * GlesCreateTokenizer(Log * log, struct Compiler * compiler) {
	Tokenizer * tokenizer = GlesMalloc(sizeof(Tokenizer));
	
	tokenizer->log = log;
	tokenizer->compiler = compiler;
	
	return tokenizer;
}

void GlesDestroyTokenizer(Tokenizer * tokenizer) {
	GLES_ASSERT(tokenizer);
	
	GlesFree(tokenizer);
}

static GLboolean DefineMacro(Tokenizer * tokenizer, const TokenString * name,
	const TokenString * text, GLsizei numParameters,
	const TokenString * parameters);
	
static Macro * FindDefinition(Tokenizer * tokenizer, const TokenString * name);

static GLboolean PushString(Tokenizer * tokenizer, const char * string);

void GlesPrepareTokenizer(Tokenizer * tokenizer,
						  Shader * shader, struct MemoryPool * tempMemory,
						  GLsizei numStrings, const char *initStrings[]) {
	static const char header[] = 
		"------------------------------------------------------------\n"
		"Tokenizer/Preprocessor Log\n"
		"------------------------------------------------------------\n"
		"\n";
		
	static char LINE[] = "__LINE__";
	static char FILE[] = "__FILE__";
	
	TokenString	macroName, macroDef;
	
	GLES_ASSERT(tokenizer);
	GLES_ASSERT(tokenizer->shader == NULL);
	GLES_ASSERT(tokenizer->tempMemory == NULL);
	
	tokenizer->shader = shader;
	tokenizer->tempMemory = tempMemory;
	
	tokenizer->input.current = shader->text;
	tokenizer->input.last = shader->text + shader->length;
	tokenizer->input.freepointer = NULL;
	tokenizer->nest = -1;
	tokenizer->input.sp = -1;
	tokenizer->lineno = 1;
	tokenizer->sourceno = 1;	/* or 0 ??? */
	tokenizer->inComment = GL_FALSE;
	tokenizer->beginLine = GL_TRUE;
	tokenizer->hadResult = GL_FALSE;
	
	GlesMemset(tokenizer->macroHash, 0, sizeof tokenizer->macroHash);
	
	/* setup __LINE__ macro */
	macroName.first = LINE;
	macroName.length = 8;
	macroDef.first = tokenizer->lineMacroText;
	macroDef.length = sizeof(tokenizer->lineMacroText);

	DefineMacro(tokenizer, &macroName, &macroDef, 0, NULL);
	tokenizer->lineMacro = FindDefinition(tokenizer, &macroName);
	
	UpdateLineMacro(tokenizer);

	/* setup __FILE__ macro */
	macroName.first = FILE;
	macroName.length = 8;
	macroDef.first = tokenizer->fileMacroText;
	macroDef.length = sizeof(tokenizer->fileMacroText);

	DefineMacro(tokenizer, &macroName, &macroDef, 0, NULL);
	tokenizer->fileMacro = FindDefinition(tokenizer, &macroName);
	
	UpdateFileMacro(tokenizer);
	
#if GLES_DEBUG			
	if (tokenizer->log) {
		GlesLogAppend(tokenizer->log, header, ~0);
	}
#endif GLES_DEBUG
	
	GlesLogInit(&tokenizer->macroExpand);
	
	while (numStrings > 0) {
		PushString(tokenizer, initStrings[--numStrings]);
	}
	
	/* initialize to first token */
	GlesNextToken(tokenizer);	
}

void GlesCleanupTokenizer(Tokenizer * tokenizer) {
	GLES_ASSERT(tokenizer);
	GLES_ASSERT(tokenizer->shader);
	GLES_ASSERT(tokenizer->tempMemory);
	
	GlesLogDeInit(&tokenizer->macroExpand);
	
	if (tokenizer->input.freepointer) {
		GlesFree(tokenizer->input.freepointer);
		tokenizer->input.freepointer = NULL;
	}
	
	tokenizer->shader = NULL;
	tokenizer->tempMemory = NULL;
}

#if GLES_DEBUG
static void WriteLog(Tokenizer * tokenizer) {
	
	GlesLogAppend(tokenizer->log, tokenizer->token.s.first, 
		tokenizer->token.s.length);
	GlesLogAppend(tokenizer->log, "\n", 1);
}
#endif /* GLES_DEBUG */

/*
 * --------------------------------------------------------------------------
 * Preprocessor code starts here
 * --------------------------------------------------------------------------
 */

static GLboolean PushString(Tokenizer * tokenizer, const char * string) {

	GLsizei sp;

	if (++tokenizer->input.sp >= GLES_MAX_PREPROC_MACRO_DEPTH) {
		GlesCompileError(tokenizer->compiler, ErrP0012);
		return GL_FALSE;
	}

	sp = tokenizer->input.sp;

	tokenizer->input. expansions[sp].freepointer = tokenizer->input.freepointer;
	tokenizer->input. expansions[sp].current = tokenizer->input.current;
	tokenizer->input. expansions[sp].last = tokenizer->input.last;
	tokenizer->input. expansions[sp].macro = NULL;

	tokenizer->input.current = string;
	tokenizer->input.last = string + GlesStrlen(string);
	tokenizer->input.freepointer = NULL;

	return GL_TRUE;
}

static GLboolean PushExpansion(Tokenizer * tokenizer, 
	const TokenString * expansion, Macro * macro, GLboolean cleanup) {
		
	GLsizei sp;
	
	if (++tokenizer->input.sp >= GLES_MAX_PREPROC_MACRO_DEPTH) {
		GlesCompileError(tokenizer->compiler, ErrP0012);
		return GL_FALSE;
	}
	
	sp = tokenizer->input.sp;
	
	tokenizer->input. expansions[sp].freepointer = tokenizer->input.freepointer;
	tokenizer->input. expansions[sp].current = tokenizer->input.current;
	tokenizer->input. expansions[sp].last = tokenizer->input.last;
	tokenizer->input. expansions[sp].macro = macro;
	
	tokenizer->input.current = expansion->first;
	tokenizer->input.last = expansion->first + expansion->length;
	tokenizer->input.freepointer = cleanup ? expansion->first : NULL;
		
	if (macro) {
		macro->disabled = GL_TRUE;
	}
		
	return GL_TRUE;
}

static Macro * FindDefinition(Tokenizer * tokenizer, const TokenString * name) {
	GLsizei hash = GlesSymbolHash(name->first, name->length);
	Macro * macro = tokenizer->macroHash[hash % GLES_PREPROC_SYMBOL_HASH];
	
	while (macro) {
		if (macro->name.length == name->length &&
			!GlesStrncmp(macro->name.first, name->first, name->length)) {
			if (!macro->disabled) {
				return macro;
			} else {
				return NULL;
			}
		}
		
		macro = macro->next;
	}
	
	return NULL;
}

static GLboolean ExpandString(Tokenizer * tokenizer, const TokenString * in,
	TokenString * out);
	
static GLsizei FindParameterIndex(const TokenString * in, const Macro * macro) {
	GLsizei index;
	
	for (index = 0; index < macro->numParameters; ++index) {
		if (macro->parameters[index].length == in->length &&
			!GlesStrncmp(macro->parameters[index].first, in->first, in->length)) {
			return index;
		}
	}
	
	return -1;
}

static GLboolean Substitute(Tokenizer * tokenizer, Macro * macro, 
	const TokenString * arguments, TokenString * out) {

	char * freeBuffer = NULL;
	GLsizei index;
	
	/* reset tokenizer input */
	
	InputState backupInput = tokenizer->input;

	tokenizer->input.current = macro->text.first;
	tokenizer->input.last = macro->text.first + macro->text.length;
	tokenizer->input.freepointer = NULL;
	tokenizer->input.sp = -1;
		
	do {
		GLboolean didExpand = GL_FALSE;
		
		/* process input string */
		while (FetchNextToken(tokenizer)) {
			if (tokenizer->token.tokenType == TokenTypeEof) {
				break;
			} else if (tokenizer->token.tokenType == TokenTypeError) {
				goto error;
			} else if (tokenizer->token.tokenType == TokenTypeSpace) {
				GlesLogAppend(&tokenizer->macroExpand, " ", 1);
			} else if (tokenizer->token.tokenType == TokenTypeIdentifier &&
				(index = FindParameterIndex(&tokenizer->token.s, macro)) >= 0) {
				/* this is a parameter substitution */
				GlesLogAppend(&tokenizer->macroExpand, 
					arguments[index].first, arguments[index].length);
			} else {
				GlesLogAppend(&tokenizer->macroExpand, 
					tokenizer->token.s.first, tokenizer->token.s.length);
			}
		}
	
		/* extract result into out value */
	
		out->length = tokenizer->macroExpand.logSize;
	
		if (out->length) {
			out->first = GlesMalloc(out->length + 1);
		
			if (!out->first) {
				goto error;
			}
		
			GlesLogExtract(&tokenizer->macroExpand, out->length + 1, out->first, 
				NULL);
			GlesLogClear(&tokenizer->macroExpand);
		} else {
			out->first = NULL;
		}

		if (freeBuffer) {
			GlesFree(freeBuffer);
		}
		
		if (!didExpand) {
			/* do until no more expansion possible */
			break;
		}
		
		freeBuffer = tokenizer->input.current = out->first;
		tokenizer->input.last = out->first + out->length;
		tokenizer->input.sp = -1;
	} while (GL_TRUE);
	
	/* restore tokenizer state */
		
	tokenizer->input = backupInput;
	return GL_TRUE;
	
error:
	if (freeBuffer) {
		GlesFree(freeBuffer);
	}

	out->first = NULL;
	out->length = 0;
	tokenizer->input = backupInput;

	return GL_FALSE;

}

static GLboolean Expand(Tokenizer * tokenizer, Macro * macro,
	const TokenString * arguments) {
	TokenString expandedArgs[GLES_MAX_PREPROC_PARAMS];
	TokenString expansion;
	GLsizei index;
	GLboolean result = GL_FALSE;
	
	GLES_ASSERT(!macro->disabled);
		
	if (macro->numParameters <= 0) {
		/* simple case, just push the replacement text */
		return PushExpansion(tokenizer, &macro->text, macro, GL_FALSE);
	} else {
		GlesMemset(expandedArgs, 0, sizeof expandedArgs);
		
		for (index = 0; index < macro->numParameters; ++index) {
			if (!ExpandString(tokenizer, &arguments[index],
				&expandedArgs[index])) {
				goto error;
			}
		}
		
		result = Substitute(tokenizer, macro, expandedArgs, &expansion) &&
			PushExpansion(tokenizer, &expansion, macro, GL_TRUE);
		
	error:
		for (index = 0; index < macro->numParameters; ++index) {
			if (expandedArgs[index].first) {
				GlesFree(expandedArgs[index].first);
			}
		}
		return result;
	}
}

static GLboolean PreprocSkipAllSpace(Tokenizer * tokenizer) {
	do {
		GLboolean result = FetchNextToken(tokenizer);
		
		if (!result) {
			return GL_FALSE;
		}
	} while (tokenizer->token.tokenType == TokenTypeSpace ||
		tokenizer->token.tokenType == TokenTypeEol);
	
	return GL_TRUE;
}

static GLboolean CollectArgument(Tokenizer * tokenizer, TokenString * out) {

	GLint nest = 0;
	
	out->first = tokenizer->input.current;
	out->length = 0;
	
	while (FetchNextToken(tokenizer)) {
		if (tokenizer->token.tokenType == TokenTypeEof ||
			tokenizer->token.tokenType == TokenTypeError) {
			GlesCompileError(tokenizer->compiler, ErrP0001);
			return GL_FALSE;
		} else if (tokenizer->token.tokenType == TokenTypeLeftParen) {
			out->length += tokenizer->token.s.length;
			++nest;
		} else if (tokenizer->token.tokenType == TokenTypeRightParen) {
			if (!nest) {
				return GL_TRUE;
			} else {
				out->length += tokenizer->token.s.length;
				--nest;
			}
		} else if (tokenizer->token.tokenType == TokenTypeComma && !nest) {
			return GL_TRUE;
		} else {
			out->length += tokenizer->token.s.length;
		}
	}
	
	return GL_FALSE;
}

static GLboolean ProcessMacro(Tokenizer * tokenizer, Macro * macro) {
	GLsizei numArgs;
	TokenString arguments[GLES_MAX_PREPROC_PARAMS];
	
	if (macro->numParameters < 0) {
		if (!Expand(tokenizer, macro, NULL)) {
			return GL_FALSE;
		}
	} else {
		if (!PreprocSkipAllSpace(tokenizer)) {
			return GL_FALSE;
		}
		
		if (tokenizer->token.tokenType != TokenTypeLeftParen) {
			/* TODO: Should factor out this case for main driver */
			GlesLogAppend(&tokenizer->macroExpand, 
				macro->name.first, macro->name.length);
			GlesLogAppend(&tokenizer->macroExpand, " " , 1);
			GlesLogAppend(&tokenizer->macroExpand, 
				tokenizer->token.s.first, tokenizer->token.s.length);
		} else if (macro->numParameters == 0) {
			if (!PreprocSkipAllSpace(tokenizer)) {
				return GL_FALSE;
			}

			if (tokenizer->token.tokenType != TokenTypeRightParen ||
				!Expand(tokenizer, macro, NULL)) {
				return GL_FALSE;
			}
		} else {
			/* now collect arguments; keep track of nesting level */
			
			/* collect argument */
			if (!CollectArgument(tokenizer, arguments)) {
				return GL_FALSE;
			}
			
			numArgs = 1;

			/* while is comma and have more parameters */
			while (numArgs < macro->numParameters &&
				tokenizer->token.tokenType == TokenTypeComma) {
				/* collect argument */
				if (!CollectArgument(tokenizer, &arguments[numArgs++])) {
					return GL_FALSE;
				}
			}
			
			/* if not enough argument: error */
			/* if not right parenthesis: error */
			if (numArgs != macro->numParameters ||
				tokenizer->token.tokenType != TokenTypeRightParen) {
				GlesCompileError(tokenizer->compiler, ErrP0001);
				return GL_FALSE;
			}
									
			/* call into Expand */
			if (!Expand(tokenizer, macro, arguments)) {
				return GL_FALSE;
			}
		}
	}
	
	return GL_TRUE;
}

static GLboolean ExpandString(Tokenizer * tokenizer, const TokenString * in,
	TokenString * out) {

	char * freeBuffer = NULL;
		
	/* reset tokenizer input */
	
	InputState backupInput = tokenizer->input;

	tokenizer->input.current = in->first;
	tokenizer->input.last = in->first + in->length;
	tokenizer->input.freepointer = NULL;
	tokenizer->input.sp = -1;
		
	do {
		GLboolean didExpand = GL_FALSE;
		Macro *macro;
		
		/* process input string */
		while (FetchNextToken(tokenizer)) {
			if (tokenizer->token.tokenType == TokenTypeEof) {
				break;
			} else if (tokenizer->token.tokenType == TokenTypeError) {
				goto error;
			} else if (tokenizer->token.tokenType == TokenTypeSpace) {
				GlesLogAppend(&tokenizer->macroExpand, " ", 1);
			} else if (tokenizer->token.tokenType == TokenTypeIdentifier &&
				(macro = FindDefinition(tokenizer, &tokenizer->token.s)) != NULL) {
				/* this is a macro expansion */
				if (!ProcessMacro(tokenizer, macro)) {
					goto error;
				}
			} else {
				GlesLogAppend(&tokenizer->macroExpand, 
					tokenizer->token.s.first, tokenizer->token.s.length);
			}
		}
	
		/* extract result into out value */
	
		out->length = tokenizer->macroExpand.logSize;
	
		if (out->length) {
			out->first = GlesMalloc(out->length + 1);
		
			if (!out->first) {
				goto error;
			}
		
			GlesLogExtract(&tokenizer->macroExpand, out->length + 1, out->first, 
				NULL);
			GlesLogClear(&tokenizer->macroExpand);
		} else {
			out->first = NULL;
		}

		if (freeBuffer) {
			GlesFree(freeBuffer);
		}
		
		if (!didExpand) {
			/* do until no more expansion possible */
			break;
		}
		
		freeBuffer = tokenizer->input.current = out->first;
		tokenizer->input.last = out->first + out->length;
		tokenizer->input.sp = -1;
	} while (GL_TRUE);
	
	/* restore tokenizer state */
		
	tokenizer->input = backupInput;
	return GL_TRUE;
	
error:
	if (freeBuffer) {
		GlesFree(freeBuffer);
	}

	out->first = NULL;
	out->length = 0;
	tokenizer->input = backupInput;

	return GL_FALSE;

}


static GLboolean IsDefined(Tokenizer * tokenizer, const TokenString * name) {
	GLsizei hash = GlesSymbolHash(name->first, name->length);
	Macro * macro = tokenizer->macroHash[hash % GLES_PREPROC_SYMBOL_HASH];
	
	while (macro) {
		if (macro->name.length == name->length &&
			!GlesStrncmp(macro->name.first, name->first, name->length)) {
			return GL_TRUE;
		}
		
		macro = macro->next;
	}
	
	return GL_FALSE;
}

static GLboolean DefineMacro(Tokenizer * tokenizer, const TokenString * name,
	const TokenString * text, GLsizei numParameters,
	const TokenString * parameters) {

	GLsizei hash = GlesSymbolHash(name->first, name->length);
	GLsizei bucket = hash % GLES_PREPROC_SYMBOL_HASH;
	Macro * macro;
	GLsizei index;
	
	if (IsDefined(tokenizer, name)) {
		/* TODO: is actually OK if definitions are identical */
		/* duplicate macro definition */
		GlesCompileError(tokenizer->compiler, ErrP0009);
		return GL_FALSE;
	}
				
	GLsizei size = sizeof(Macro);
	
	if (numParameters > 0) {
		size += numParameters * sizeof(TokenString);
	}
	
	macro = GlesMemoryPoolAllocate(tokenizer->tempMemory, size);
	
	if (!macro) {
		GlesCompileError(tokenizer->compiler, ErrI0001);
		return GL_FALSE;
	}
	
	macro->next = tokenizer->macroHash[bucket];
	tokenizer->macroHash[bucket] = macro;
	macro->name = *name;
	macro->text = *text;
	macro->numParameters = numParameters;
	macro->disabled = GL_FALSE;
	
	for (index = 0; index < numParameters; ++index) {
		macro->parameters[index] = parameters[index];
	}
	
	return GL_TRUE;
}

static GLboolean UndefineMacro(Tokenizer * tokenizer, const TokenString * name) {
	GLsizei hash = GlesSymbolHash(name->first, name->length);
	GLsizei bucket = hash % GLES_PREPROC_SYMBOL_HASH;
	Macro * macro = tokenizer->macroHash[bucket], *prevMacro = NULL;
	
	while (macro) {
		if (macro->name.length == name->length &&
			!GlesStrncmp(macro->name.first, name->first, name->length)) {
			if (prevMacro == NULL) {
				tokenizer->macroHash[bucket] = macro->next;
			} else {
				prevMacro->next = macro->next;
			}
			
			return GL_TRUE;
		}
		
		prevMacro = macro;
		macro = macro->next;
	}
	
	return GL_TRUE;
}

static GLboolean PreprocSkipToEol(Tokenizer * tokenizer) {
	while (tokenizer->token.tokenType != TokenTypeEol &&
		tokenizer->token.tokenType != TokenTypeEof &&
		tokenizer->token.tokenType != TokenTypeError) {
		GLboolean result = FetchNextToken(tokenizer);
		
		if (!result) {
			return GL_FALSE;
		}
	}
	
	return (tokenizer->token.tokenType != TokenTypeError);
}

static GLboolean PreprocSkipSpace1(Tokenizer * tokenizer) {
	do {
		GLboolean result = FetchNextToken(tokenizer);
		
		if (!result) {
			return GL_FALSE;
		}
	} while (tokenizer->token.tokenType == TokenTypeSpace);
	
	return GL_TRUE;
}

static GLboolean PreprocSkipSpace0(Tokenizer * tokenizer) {
	while (tokenizer->token.tokenType == TokenTypeSpace) {
		GLboolean result = FetchNextToken(tokenizer);
		
		if (!result) {
			return GL_FALSE;
		}
	}
	
	return GL_TRUE;
}

static GLboolean PreprocDefine(Tokenizer * tokenizer) {
	TokenString name, text;
	char * start, *end;
	GLsizei numParameters = ~0;
	TokenString parameters[GLES_MAX_PREPROC_PARAMS];
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	/* identifier */
	if (tokenizer->token.tokenType != TokenTypeIdentifier) {
		/* preprocessor syntax error */
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	name = tokenizer->token.s;
	start = tokenizer->input.current;
	
	if (!FetchNextToken(tokenizer)) {
		return GL_FALSE;
	}
	
	/* optional parenthesis */
	if (tokenizer->token.tokenType == TokenTypeLeftParen) {
		/* argument list */
		if (!PreprocSkipSpace1(tokenizer)) {
			return GL_FALSE;
		}
		
		numParameters = 0;

		if (tokenizer->token.tokenType == TokenTypeIdentifier) {
			/* parameter list */

			/* first parameter */
			parameters[numParameters++] = tokenizer->token.s;
			
			if (!PreprocSkipSpace1(tokenizer)) {
				return GL_FALSE;
			}
			
			while (tokenizer->token.tokenType == TokenTypeComma) {
				if (!PreprocSkipSpace1(tokenizer)) {
					return GL_FALSE;
				}

				if (tokenizer->token.tokenType != TokenTypeIdentifier) {
					/* preprocessor syntax error */
					GlesCompileError(tokenizer->compiler, ErrP0001);
					return GL_FALSE;					
				}

				if (numParameters >= GLES_MAX_PREPROC_PARAMS) {
					GlesCompileError(tokenizer->compiler, ErrP0001);
					return GL_FALSE;										
				}
				
				parameters[numParameters++] = tokenizer->token.s;
				
				if (!PreprocSkipSpace1(tokenizer)) {
					return GL_FALSE;
				}
			}
			
			if (tokenizer->token.tokenType != TokenTypeRightParen) {
				/* preprocessor syntax error */
				GlesCompileError(tokenizer->compiler, ErrP0001);
				return GL_FALSE;
			} 
			
		} else if (tokenizer->token.tokenType != TokenTypeRightParen) {
			/* preprocessor syntax error */
			GlesCompileError(tokenizer->compiler, ErrP0001);
			return GL_FALSE;			
		}
		
		GLES_ASSERT(tokenizer->token.tokenType == TokenTypeRightParen);
		
		start = tokenizer->input.current;

		if (!FetchNextToken(tokenizer)) {
			return GL_FALSE;
		}
	}

	/* replacement text until end of line */
	end = start;
	
	while (tokenizer->token.tokenType != TokenTypeEol &&
		tokenizer->token.tokenType != TokenTypeEof) {
		end = tokenizer->input.current;

		if (!FetchNextToken(tokenizer)) {
			return GL_FALSE;
		}
	}

	/* define the new macro */
	text.first = start;
	text.length = end - start;
	
	return DefineMacro(tokenizer, &name, &text, numParameters, parameters);
}

static GLboolean PreprocUndef(Tokenizer * tokenizer) {
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	if (tokenizer->token.tokenType != TokenTypeIdentifier) {
		/* preprocessor syntax error */
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	if (!UndefineMacro(tokenizer, &tokenizer->token.s)) {
		return GL_FALSE;
	}
	
	return PreprocSkipToEol(tokenizer);
}

static GLboolean EvaluateExpression(Tokenizer * tokenizer, GLint *result);

static GLboolean EvaluatePrimaryExpression(Tokenizer * tokenizer, GLint *result) {

	Macro * macro;
	
	while (GL_TRUE) {
		GLES_ASSERT(tokenizer->token.tokenType != TokenTypeSpace);

		switch (tokenizer->token.tokenType) {
		case TokenTypeIdentifier:
			if (tokenizer->token.s.length == 7 &&
				!GlesStrncmp(tokenizer->token.s.first, "defined", 7)) {
				if (!PreprocSkipSpace1(tokenizer)) {
					return GL_FALSE;
				}
				
				if (tokenizer->token.tokenType == TokenTypeIdentifier) {
					*result = IsDefined(tokenizer, &tokenizer->token.s);
				} else if (tokenizer->token.tokenType == TokenTypeLeftParen) {
					if (!PreprocSkipSpace1(tokenizer)) {
						return GL_FALSE;
					}
					
					if (tokenizer->token.tokenType != TokenTypeIdentifier) {
						/* preprocessor syntax error */
						GlesCompileError(tokenizer->compiler, ErrP0001);
						return GL_FALSE;
					}
					
					*result = IsDefined(tokenizer, &tokenizer->token.s);
					
					if (!PreprocSkipSpace1(tokenizer)) {
						return GL_FALSE;
					}
					
					if (tokenizer->token.tokenType != TokenTypeRightParen) {
						/* preprocessor syntax error */
						GlesCompileError(tokenizer->compiler, ErrP0001);
						return GL_FALSE;
					}
				} else {
					*result = 0;
					return GL_TRUE;
				}
				
				return PreprocSkipSpace1(tokenizer);

			} else if ((macro = FindDefinition(tokenizer, &tokenizer->token.s)) != NULL) {
				if (!ProcessMacro(tokenizer, macro) ||
					!PreprocSkipSpace0(tokenizer)) {
					return GL_FALSE;
				}

				/* start over */
				break;
			} else {
				*result = 0;
				return PreprocSkipSpace1(tokenizer);
			}
		
		case TokenTypeLeftParen:
			if (!PreprocSkipSpace1(tokenizer) ||
				!EvaluateExpression(tokenizer, result))
				return GL_FALSE;
			
			if (tokenizer->token.tokenType != TokenTypeRightParen) {
				/* preprocessor syntax error */
				GlesCompileError(tokenizer->compiler, ErrP0001);
				return GL_FALSE;
			}
		
			return PreprocSkipSpace1(tokenizer);
		
		case TokenTypeIntConstant:
			*result = tokenizer->token.value.i;
			return PreprocSkipSpace1(tokenizer);
		
		case TokenTypeFloatConstant:
			/* no floating point in preprocessor */
			GlesCompileError(tokenizer->compiler, ErrP0013);
			return GL_FALSE;
		
		default:
			/* preprocessor syntax error */
			GlesCompileError(tokenizer->compiler, ErrP0001);
			return GL_FALSE;
		}
	}
}

static GLboolean EvaluateUnaryExpression(Tokenizer * tokenizer, GLint *result) {
	
	GLint value;
	
	GLES_ASSERT(tokenizer->token.tokenType != TokenTypeSpace);
	
	switch (tokenizer->token.tokenType) {
	case TokenTypeDash:
		if (!PreprocSkipSpace1(tokenizer) ||
			!EvaluateUnaryExpression(tokenizer, &value)) {
			return GL_FALSE;
		}
		
		*result = -value;
		return GL_TRUE;
		
	case TokenTypeTilde:
		if (!PreprocSkipSpace1(tokenizer) ||
			!EvaluateUnaryExpression(tokenizer, &value)) {
			return GL_FALSE;
		}
	
		*result = ~value;		
		return GL_TRUE;
		
	case TokenTypeBang:
		if (!PreprocSkipSpace1(tokenizer) ||
			!EvaluateUnaryExpression(tokenizer, &value)) {
			return GL_FALSE;
		}
		
		*result = !value;		
		return GL_TRUE;
		
	default:
		return EvaluatePrimaryExpression(tokenizer, result);
	}
}

static GLint EvaluateLessEqual(GLint left, GLint right) {
	return left <= right;
}

static GLint EvaluateGreaterEqual(GLint left, GLint right) {
	return left >= right;
}

static GLint EvaluateEqual(GLint left, GLint right) {
	return left == right;
}

static GLint EvaluateNotEqual(GLint left, GLint right) {
	return left != right;
}

static GLint EvaluateAnd(GLint left, GLint right) {
	return left && right;
}

static GLint EvaluateOr(GLint left, GLint right) {
	return left || right;
}

static GLint EvaluatePlus(GLint left, GLint right) {
	return left + right;
}

static GLint EvaluateDash(GLint left, GLint right) {
	return left - right;
}

static GLint EvaluateStar(GLint left, GLint right) {
	return left * right;
}

static GLint EvaluateSlash(GLint left, GLint right) {
	return left / right;
}

static GLint EvaluatePercent(GLint left, GLint right) {
	return left % right;
}

static GLint EvaluateLeftAngle(GLint left, GLint right) {
	return left < right;
}

static GLint EvaluateRightAngle(GLint left, GLint right) {
	return left > right;
}

static GLint EvaluateVerticalBar(GLint left, GLint right) {
	return left | right;
}

static GLint EvaluateCaret(GLint left, GLint right) {
	return left ^ right;
}

static GLint EvaluateAmpersand(GLint left, GLint right) {
	return left & right;
}

static GLint EvaluateLeft(GLint left, GLint right) {
	return left << right;
}

static GLint EvaluateRight(GLint left, GLint right) {
	return left >> right;
}

static GLboolean EvaluateBinaryExpressionTail(Tokenizer * tokenizer, GLint left, GLint *result) {
	
	typedef GLint (*functionPtr)(GLint left, GLint right);
	struct {
		functionPtr		func;
		GLsizei			prec;
		GLint			expr;
	} stack[14];
		
	GLint value;
	GLsizei sp, prec;
	
	stack[0].func = NULL;
	stack[0].prec = ~0;
	stack[0].expr = left;
	
	sp = 0;
	
	GLES_ASSERT(tokenizer->token.tokenType != TokenTypeSpace);
	
	while (GL_TRUE) {
		functionPtr func;
		
		switch (tokenizer->token.tokenType) {
		case TokenTypeLessEqualOp:		func = EvaluateLessEqual;		prec = 6; break;
		case TokenTypeGreaterEqualOp:	func = EvaluateGreaterEqual;	prec = 6; break;
		case TokenTypeEqualOp:			func = EvaluateEqual;			prec = 7; break;
		case TokenTypeNotEqualOp:		func = EvaluateNotEqual;		prec = 7; break;
		case TokenTypeAndOp:			func = EvaluateAnd;				prec = 11; break;
		case TokenTypeOrOp:				func = EvaluateOr;				prec = 12; break;
		case TokenTypePlus:				func = EvaluatePlus;			prec = 4; break;
		case TokenTypeDash:				func = EvaluateDash;			prec = 4; break;
		case TokenTypeStar: 			func = EvaluateStar;			prec = 3; break;
		case TokenTypeSlash: 			func = EvaluateSlash;			prec = 3; break;
		case TokenTypePercent:			func = EvaluatePercent;			prec = 3; break;
		case TokenTypeLeftAngle: 		func = EvaluateLeftAngle;		prec = 6; break;
		case TokenTypeRightAngle: 		func = EvaluateRightAngle;		prec = 6; break;
		case TokenTypeVerticalBar:	 	func = EvaluateVerticalBar;		prec = 10; break;
		case TokenTypeCaret:  			func = EvaluateCaret;			prec = 9; break;
		case TokenTypeAmpersand: 		func = EvaluateAmpersand;		prec = 8; break;
		case TokenTypeLeftOp:		 	func = EvaluateLeft;			prec = 5; break;
		case TokenTypeRightOp: 			func = EvaluateRight;			prec = 5; break;

		default:
			goto done;
		}
		
		if (!PreprocSkipSpace1(tokenizer) ||
			!EvaluateUnaryExpression(tokenizer, &value)) {
			return GL_FALSE;
		}		
		
		while (sp > 0 && stack[sp].prec >= prec) {
			--sp;
			stack[sp].expr = stack[sp + 1].func(stack[sp].expr, stack[sp + 1].expr);
		}		
		
		++sp;
		
		stack[sp].expr = value;
		stack[sp].func = func;
		stack[sp].prec = prec;
	}
	
done:
	while (sp > 0) {
		--sp;
		stack[sp].expr = stack[sp + 1].func(stack[sp].expr, stack[sp + 1].expr);
	}		

	*result = stack[0].expr;

	return GL_TRUE;
}

static GLboolean EvaluateBinaryExpression (Tokenizer * tokenizer, GLint *result) {
	GLint value;
	
	GLES_ASSERT(tokenizer->token.tokenType != TokenTypeSpace);
	
	if (!EvaluateUnaryExpression(tokenizer, &value)) {
		return GL_FALSE;
	}
	
	GLES_ASSERT(tokenizer->token.tokenType != TokenTypeSpace);
	
	switch (tokenizer->token.tokenType) {
	case TokenTypeLessEqualOp:
	case TokenTypeGreaterEqualOp:
	case TokenTypeEqualOp:
	case TokenTypeNotEqualOp:
	case TokenTypeAndOp:
	case TokenTypeOrOp:
	case TokenTypePlus:
	case TokenTypeDash:
	case TokenTypeStar: 
	case TokenTypeSlash: 
	case TokenTypePercent:
	case TokenTypeLeftAngle: 
	case TokenTypeRightAngle: 
	case TokenTypeVerticalBar:
	case TokenTypeCaret:
	case TokenTypeAmpersand:
	case TokenTypeLeftOp:
	case TokenTypeRightOp:
		return EvaluateBinaryExpressionTail(tokenizer, value, result);
	
	default:
		*result = value;
		return GL_TRUE;
	}
}

static GLboolean EvaluateConditionalExpressionTail(Tokenizer * tokenizer, GLint left, GLint *result) {
	
	GLint trueValue, falseValue;
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeQuestion);
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	GLES_ASSERT(tokenizer->token.tokenType != TokenTypeSpace);
	
	if (!EvaluateExpression(tokenizer, &trueValue)) {
		return GL_FALSE;
	}
	
	if (tokenizer->token.tokenType != TokenTypeColon) {
		/* preprocessor syntax error */
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}

	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	if (!EvaluateExpression(tokenizer, &falseValue)) {
		return GL_FALSE;
	}
	
	*result = left ? trueValue : falseValue;
	return GL_TRUE;
}

static GLboolean EvaluateExpression(Tokenizer * tokenizer, GLint *result) {
	GLint value;
	
	GLES_ASSERT(tokenizer->token.tokenType != TokenTypeSpace);
	
	if (!EvaluateBinaryExpression(tokenizer, &value)) {
		return GL_FALSE;
	}
	
	if (tokenizer->token.tokenType == TokenTypeQuestion) {
		return EvaluateConditionalExpressionTail(tokenizer, value, result);
	} else {
		*result = value;
		return GL_TRUE;
	}
}

static GLboolean Evaluate(Tokenizer * tokenizer, GLboolean * result) {
	GLint value;
	
	if (!EvaluateExpression(tokenizer, &value)) {
		return GL_FALSE;
	}
	
	if (tokenizer->token.tokenType != TokenTypeEol &&
		tokenizer->token.tokenType != TokenTypeEof) {
		/* preprocessor syntax error */
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	*result = value != 0;
	
	return GL_TRUE;
}

static GLboolean PreprocIf(Tokenizer * tokenizer) {
	GLboolean condition;
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (!PreprocSkipSpace1(tokenizer) ||
		!Evaluate(tokenizer, &condition)) {
		return GL_FALSE;
	}
		
	if (++tokenizer->nest >= GLES_MAX_PREPROC_COND_DEPTH) {
		/* maximum nesting depth exceeded */
		GlesCompileError(tokenizer->compiler, ErrP0011);
		return GL_FALSE;
	}
	
	tokenizer->conditionals[tokenizer->nest].elsePart = GL_FALSE;
	tokenizer->conditionals[tokenizer->nest].wasTrue = 
	tokenizer->conditionals[tokenizer->nest].isTrue = 
		condition &&
		(tokenizer->nest == 0 ||
		 tokenizer->conditionals[tokenizer->nest-1].isTrue);

	return PreprocSkipToEol(tokenizer);
}

static GLboolean PreprocElif(Tokenizer * tokenizer) {
	GLboolean condition;
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (!Evaluate(tokenizer, &condition)) {
		return GL_FALSE;
	}
		
	if (tokenizer->nest < 0 ||
		tokenizer->conditionals[tokenizer->nest].elsePart) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	tokenizer->conditionals[tokenizer->nest].elsePart = GL_FALSE;
	tokenizer->conditionals[tokenizer->nest].wasTrue = 
	tokenizer->conditionals[tokenizer->nest].isTrue = 
		condition &&
		(tokenizer->nest == 0 ||
		 tokenizer->conditionals[tokenizer->nest-1].isTrue);

	return PreprocSkipToEol(tokenizer);
}

static GLboolean PreprocIfdef(Tokenizer * tokenizer, GLboolean negate) {
	GLboolean condition;
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}

	if (tokenizer->token.tokenType != TokenTypeIdentifier) {
		/* preprocessor syntax error */
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;					
	}
	
	if (++tokenizer->nest >= GLES_MAX_PREPROC_COND_DEPTH) {
		/* maximum nesting depth exceeded */
		GlesCompileError(tokenizer->compiler, ErrP0011);
		return GL_FALSE;
	}
	
	condition = IsDefined(tokenizer, &tokenizer->token.s);
	if (negate) condition = !condition;
		
	tokenizer->conditionals[tokenizer->nest].elsePart = GL_FALSE;
	tokenizer->conditionals[tokenizer->nest].wasTrue = 
	tokenizer->conditionals[tokenizer->nest].isTrue = 
		condition &&
		(tokenizer->nest == 0 ||
		 tokenizer->conditionals[tokenizer->nest-1].isTrue);
	
	return PreprocSkipToEol(tokenizer);
}

static GLboolean PreprocElse(Tokenizer * tokenizer) {
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (tokenizer->nest < 0 ||
		tokenizer->conditionals[tokenizer->nest].elsePart) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	tokenizer->conditionals[tokenizer->nest].elsePart = GL_TRUE;
	tokenizer->conditionals[tokenizer->nest].isTrue = 
		!tokenizer->conditionals[tokenizer->nest].wasTrue &&
		(tokenizer->nest == 0 ||
		 tokenizer->conditionals[tokenizer->nest-1].isTrue);
	tokenizer->conditionals[tokenizer->nest].wasTrue = GL_TRUE;
	
	return PreprocSkipToEol(tokenizer);
}

static GLboolean PreprocEndif(Tokenizer * tokenizer) {
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (tokenizer->nest < 0) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	--tokenizer->nest;
	
	return PreprocSkipToEol(tokenizer);
}

static GLboolean PreprocError(Tokenizer * tokenizer) {
	char * current = tokenizer->input.current;
	char * last = tokenizer->input.current;

	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	while (tokenizer->token.tokenType != TokenTypeEol &&
		tokenizer->token.tokenType != TokenTypeEof) {
		last = tokenizer->input.current;
		GLboolean result = FetchNextToken(tokenizer);
		
		if (!result) {
			break;
		}
	}
	
	while (current != last && (*current == ' ' || *current == '\t')) {
		++current;
	}
	
	GlesCompileErrorMessage(tokenizer->compiler, current, last - current);
	
	return GL_FALSE;
}

static GLboolean GetEnable(Tokenizer * tokenizer, GLboolean * enable) {
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	if (tokenizer->token.tokenType != TokenTypeLeftParen) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}

	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}

	if (tokenizer->token.tokenType != TokenTypeIdentifier) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}

	if (tokenizer->token.s.length == 2 &&
		!GlesStrncmp(tokenizer->token.s.first, "on", 2)) {
		*enable = GL_TRUE;
	} else if (tokenizer->token.s.length == 3 &&
		!GlesStrncmp(tokenizer->token.s.first, "off", 3)) {
		*enable = GL_FALSE;
	} else {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}

	if (tokenizer->token.tokenType != TokenTypeRightParen) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	return GL_TRUE;
}

static GLboolean PreprocPragma(Tokenizer * tokenizer) {
	GLboolean enable;
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}

	if (tokenizer->token.tokenType != TokenTypeIdentifier) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}

	if (tokenizer->token.s.length == 5 &&
		!GlesStrncmp(tokenizer->token.s.first, "debug", 5)) {
		if (!GetEnable(tokenizer, &enable)) {
			return GL_FALSE;
		}
			
		GlesPragmaDebug(tokenizer->compiler, enable);
	} else if (tokenizer->token.s.length == 8 &&
		!GlesStrncmp(tokenizer->token.s.first, "optimize", 8)) {
		if (!GetEnable(tokenizer, &enable)) {
			return GL_FALSE;
		}

		GlesPragmaOptimize(tokenizer->compiler, enable);			
	}
	
	/* all unknown pragmas are ignored */
	
	return PreprocSkipToEol(tokenizer);
}

static GLboolean PreprocExtension(Tokenizer * tokenizer) {
	TokenString extension;
	GLboolean all;
	GLboolean unknown;
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (tokenizer->hadResult) {
		/* #extension must be first directive or statement */
		GlesCompileError(tokenizer->compiler, ErrP0008);
		return GL_FALSE;
	}
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	/* extension_name (or all) */
	if (tokenizer->token.tokenType != TokenTypeIdentifier) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	extension = tokenizer->token.s;
	
	all = extension.length == 3 &&
		!GlesStrncmp(extension.first, "all", 3);
	
	/* if not all, it's an unknown extension; at least for now */
	unknown = !all;
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	/* : */
	if (tokenizer->token.tokenType != TokenTypeColon) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}

	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}

	/* enable, disable, require, warn */
	if (tokenizer->token.tokenType != TokenTypeIdentifier) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}

	switch (tokenizer->token.s.length) {
	case 4:
		if (!GlesStrncmp(tokenizer->token.s.first, "warn", 4)) {
			if (unknown) {
				GlesCompileError(tokenizer->compiler, ErrP0003);
				return GL_FALSE;
			}			
		} else {
			goto invalid;
		}
		
		break;
		
	case 6:
		if (!GlesStrncmp(tokenizer->token.s.first, "enable", 6)) {
			if (all || unknown) {
				GlesCompileError(tokenizer->compiler, ErrP0003);
				return GL_FALSE;
			}		
		} else {
			goto invalid;
		}
	
		break;
	
	case 7:
		if (!GlesStrncmp(tokenizer->token.s.first, "disable", 7)) {
			/* can always disable */
		} else if (!GlesStrncmp(tokenizer->token.s.first, "require", 7)) {
			if (all || unknown) {
				GlesCompileError(tokenizer->compiler, ErrP0003);
				return GL_FALSE;
			}
		} else {
			goto invalid;
		}
	
		break;
	
	default:
	invalid:
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	return PreprocSkipToEol(tokenizer);
}

static GLboolean PreprocVersion(Tokenizer * tokenizer) {
	GLsizei version;
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (tokenizer->hadResult) {
		/* #version must be first directive or statement */
		GlesCompileError(tokenizer->compiler, ErrP0005);
		return GL_FALSE;
	}
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	/* line number */
	if (tokenizer->token.tokenType != TokenTypeIntConstant) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	version = tokenizer->token.value.i;
	
	if (version != GLES_SL_VERSION) {
		GlesCompileError(tokenizer->compiler, ErrP0007);
		return GL_FALSE;
	}
	
	return PreprocSkipToEol(tokenizer);
}

static GLboolean PreprocLine(Tokenizer * tokenizer) {
	GLsizei lineno = tokenizer->lineno + 1;
	GLsizei sourceno = tokenizer->sourceno;
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeIdentifier);
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	/* line number */
	if (tokenizer->token.tokenType != TokenTypeIntConstant) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	lineno = tokenizer->token.value.i;
	UpdateLineMacro(tokenizer);
	
	if (!PreprocSkipSpace1(tokenizer)) {
		return GL_FALSE;
	}
	
	/* optional source string number */
	if (tokenizer->token.tokenType == TokenTypeIntConstant) {
		sourceno = tokenizer->token.value.i;
	} else if (tokenizer->token.tokenType != TokenTypeEol &&
		tokenizer->token.tokenType != TokenTypeEof) {
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	tokenizer->lineno = lineno - 1;
	tokenizer->sourceno = sourceno;
	
	UpdateLineMacro(tokenizer);
	UpdateFileMacro(tokenizer);
	
	return PreprocSkipToEol(tokenizer);
}

/**
 * Parse a pre-processing statement using the current tokenizer state.
 *
 * @param	tokenizer	current tokenizer state
 *
 * @return	GL_TRUE if parse was successful. Current token will be
 *			TokenTypeEol or TokenTypeEof.
 */
static GLboolean PreprocStatement(Tokenizer * tokenizer) {
	GLboolean result;
	
	GLES_ASSERT(tokenizer->token.tokenType == TokenTypeHash);
	
	do {
		result = FetchNextToken(tokenizer);
		
		if (!result) {
			return GL_FALSE;
		}
	} while (tokenizer->token.tokenType == TokenTypeSpace);
	
	if (tokenizer->token.tokenType == TokenTypeEol ||
		tokenizer->token.tokenType == TokenTypeEof) {
		/* just a stray '#' at the beginning of a line */
		return GL_TRUE;	
	} else if (tokenizer->token.tokenType != TokenTypeIdentifier) {
		/* must be identifier token */
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
	
	switch (tokenizer->token.s.length) {
	case 2:
		if (!GlesStrncmp(tokenizer->token.s.first, "if", 2)) {
			return PreprocIf(tokenizer);
		} else {
			goto invalid;
		}
		
	case 4:
		if (!GlesStrncmp(tokenizer->token.s.first, "else", 4)) {
			return PreprocElse(tokenizer);
		} else 	if (!GlesStrncmp(tokenizer->token.s.first, "elif", 4)) {
			return PreprocElif(tokenizer);
		} else 	if (!GlesStrncmp(tokenizer->token.s.first, "line", 4)) {
			return PreprocLine(tokenizer);
		} else {
			goto invalid;
		}

	case 5:
		if (!GlesStrncmp(tokenizer->token.s.first, "undef", 5)) {
			return PreprocUndef(tokenizer);
		} else 	if (!GlesStrncmp(tokenizer->token.s.first, "ifdef", 5)) {
			return PreprocIfdef(tokenizer, GL_FALSE);
		} else 	if (!GlesStrncmp(tokenizer->token.s.first, "endif", 5)) {
			return PreprocEndif(tokenizer);
		} else 	if (!GlesStrncmp(tokenizer->token.s.first, "error", 5)) {
			return PreprocError(tokenizer);
		} else {
			goto invalid;
		}

	case 6:
		if (!GlesStrncmp(tokenizer->token.s.first, "define", 6)) {
			return PreprocDefine(tokenizer);
		} else 	if (!GlesStrncmp(tokenizer->token.s.first, "ifndef", 6)) {
			return PreprocIfdef(tokenizer, GL_TRUE);
		} else 	if (!GlesStrncmp(tokenizer->token.s.first, "pragma", 6)) {
			return PreprocPragma(tokenizer);
		} else {
			goto invalid;
		}

	case 7:
		if (!GlesStrncmp(tokenizer->token.s.first, "version", 7)) {
			return PreprocVersion(tokenizer);
		} else {
			goto invalid;
		}

	case 8:
		if (!GlesStrncmp(tokenizer->token.s.first, "extension", 8)) {
			return PreprocExtension(tokenizer);
		}
		
		/* fall through */
		
	default:
	invalid:
		/* must be identifier token */
		GlesCompileError(tokenizer->compiler, ErrP0001);
		return GL_FALSE;
	}
}

/*
 * --------------------------------------------------------------------------
 * Main driver function for tokenizer; invokes both tokenization and
 * preprocessing.
 * --------------------------------------------------------------------------
 */

GLboolean GlesNextToken(Tokenizer * tokenizer) {
	
	for (;;) {
		GLboolean result = FetchNextToken(tokenizer);
		
		if (!result) {
			return GL_FALSE;
		} else {
			if (tokenizer->token.tokenType == TokenTypeHash &&
				tokenizer->beginLine) {
				// parse a pre-processor statement
				if (!PreprocStatement(tokenizer)) {
					return GL_FALSE;
				}
				
				GLES_ASSERT(tokenizer->token.tokenType == TokenTypeEol ||
							tokenizer->token.tokenType == TokenTypeEof);
							
				continue;
			} else if (tokenizer->token.tokenType == TokenTypeIdentifier) {
				Macro * macro;
				const struct Keyword * keyword;
				
				if ((macro = FindDefinition(tokenizer, &tokenizer->token.s)) != NULL) {
					if (!ProcessMacro(tokenizer, macro) ||
						!PreprocSkipSpace0(tokenizer)) {
						GlesCompileError(tokenizer->compiler, ErrI0001);
						return GL_FALSE;
					}
					
					continue;
				}
			
				keyword = 
					CheckKeyword(tokenizer->token.s.first,
								 tokenizer->token.s.length);
								 
				if (keyword) {
					tokenizer->token.tokenType = keyword->token;
					
					if (keyword->token == TokenTypeError) {
						/* use of reserved keyword */
						GlesCompileError(tokenizer->compiler, ErrL0003);
						return GL_FALSE;
					}
				}
			}
			
#if GLES_DEBUG			
			if (tokenizer->log) {
				WriteLog(tokenizer);
			}
#endif

			if (tokenizer->token.tokenType == TokenTypeEol) {
				tokenizer->beginLine = GL_TRUE;
			} else {
				if (tokenizer->token.tokenType != TokenTypeSpace) {
					tokenizer->beginLine = GL_FALSE;
					
					if (tokenizer->nest < 0 ||
						tokenizer->conditionals[tokenizer->nest].isTrue) {
						if (!tokenizer->hadResult && tokenizer->input.sp >= 0) {
							tokenizer->hadResult = GL_TRUE;
						}
						
						return GL_TRUE;		
					}
				}

			}
		}
	}
}


