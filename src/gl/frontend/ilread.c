/*
** ==========================================================================
**
** $Id: ilread.c 75 2007-10-10 08:27:53Z hmwill $			
** 
** Intermediate Shader Language
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-10-10 01:27:53 -0700 (Wed, 10 Oct 2007) $
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
#include "frontend/il.h"
#include "frontend/types.h"
#include "frontend/symbols.h"
#include "frontend/memory.h"
#include "frontend/compiler.h"

/*
 * --------------------------------------------------------------------------
 * Module-local data
 * --------------------------------------------------------------------------
 */

#define strncmp GlesStrncmp
#include "frontend/opcodes.inc"
#undef strncmp

/**
 * Single entry in collision chain associated with the different hash values. The
 * look-up function maintains each collision chain using a move-to-front algorithm.
 */
typedef struct Bucket {
	struct Bucket *	next;		/**< next record in collision chain				*/
	const char *	name;		/**< pointer to symbol name 					*/
	GLsizeiptr		length;		/**< length of symbol name						*/
	void * 			data;		/**< reference to data associated with symbol	*/
} Bucket;

/**
 * Simple hash table structure for symbol look-up during parsing of the input text.
 * 
 * It is a simple array of pointers to collison chains. Currently, there is no 
 * rehashing implemented in this structure.
 */
typedef struct SymbolTable {
	MemoryPool *	pool;						/**< memory pool to use				*/
	Bucket *		buckets[GLES_SYMBOL_HASH];	/**< pointers to collision chains	*/
} SymbolTable;

/**
 * Data structure used while parsing a low level shading language program.
 */ 
typedef struct ShaderProgramParser {
	ShaderProgramGenerator generator;	/**< generator for instruction stream	*/
	MemoryPool *		temp;			/**< temporary memory				*/

	/** Hash table for words of constant data								*/ 
	SymbolTable			variables;		/**< symbol hash table	*/
	SymbolTable			addresses;		/**< symbol hash table	*/
	SymbolTable			labels;			/**< symbol hash table	*/
} ShaderProgramParser;

/**
 * Initialize a symbol table structure for use with the specified memory pool
 * 
 * @param	table	reference to the symbol table structure
 * @param	pool	memory pool to be used for all allocations done by the table
 */
static void InitSymbolTable(SymbolTable * table, MemoryPool * pool) {
	GlesMemset(table, 0, sizeof(SymbolTable));
	table->pool = pool;
}

/**
 * Add a new symbol definition to the symbol table
 * 
 * @param	table	reference to the symbol table structure
 * @param	name	pointer to first character of symbol name
 * @param	length	number of characters of symbol name
 * @param	data	reference to the data object to be associated with the symbol
 */
static void DefineSymbol(SymbolTable * table, const char * name, 
	GLsizeiptr length, void * data) {
	GLsizei hash = GlesSymbolHash(name, length);
	Bucket * bucket = GlesMemoryPoolAllocate(table->pool, sizeof(Bucket));

	GLES_ASSERT(data);
	
	bucket->name = name;
	bucket->length = length;
	bucket->data = data;
	bucket->next = table->buckets[hash];
	table->buckets[hash] = bucket;
}

/**
 * Search a previously defined symbol in a symbol table.
 * 
 * In addition top performing a look-up, the function will move the
 * symbol retrieved to the front of its corresponding collision chain.
 * 
 * @param	table	reference to the symbol table structure
 * @param	name	pointer to first character of symbol name
 * @param	length	number of characters of symbol name
 * 
 * @return	reference to object previously defined for the given symbol, or
 * 			NULL if no such object exists.
 */
static void * LookupSymbol(SymbolTable * table, const char * name, 
	GLsizeiptr length) {
	GLsizei hash = GlesSymbolHash(name, length);
	Bucket * bucket = table->buckets[hash], *prev = NULL;
	
	while (bucket) {
		if (bucket->length == length && !GlesMemcmp(name, bucket->name, length)) {
			
			if (prev) {
				/* move used entry to the front of the table */
				prev->next = bucket->next;
				bucket->next = table->buckets[hash];
				table->buckets[hash] = bucket;
			}
			
			return bucket->data;
		}
		
		prev = bucket;
		bucket = bucket->next;
	}
	 
	return NULL;
}

/*
** --------------------------------------------------------------------------
** Module-local functions
** --------------------------------------------------------------------------
*/

/**
 * Skip a sequence of white space characters
 * 
 * @param	begin	the current character to analyze
 * @param	end		the first character beyond the end of string
 * 
 * @return	the advanced character pointer to the first non-space character
 * 			in the input
 */
static const char * SkipSpace(const char * begin, const char * end) {
	if (!begin) {
		return NULL;
	}
	
	while (begin != end && isspace(*begin)) {
		++begin;
	}
	
	return begin;
}

static const char * MustBe(const char * text, const char * end, char ch) {
	if (!text) {
		return NULL;
	}
	
	text = SkipSpace(text, end);
	
	if (text == end || *text++ != ch) {
		return NULL;
	}
	
	return text;
}

/**
 * An improper identifier is any sequence of letters, digits, '$' or '0'.
 */
static const char * ImproperIdentifier(const char * text, const char * end) {
	if (!text || text == end || (!isalnum(*text) && *text != '$' && *text != '_')) {
		return NULL;
	}
	
	do {
		++text;
	} while (text != end && (isalnum(*text) || *text == '$' || *text == '_'));
	
	return text;
}

static const char * Identifier(const char * text, const char * end) {
	if (!text || text == end || (!isalpha(*text) && *text != '$' && *text != '_')) {
		return NULL;
	}
	
	do {
		++text;
	} while (text != end && (isalnum(*text) || *text == '$' || *text == '_'));
	
	return text;
}

static const char * Number(const char * text, const char * end, GLsizeiptr * pValue) {
	
	if (!text || text == end || !isdigit(*text)) {
		return NULL;
	}
	
	*pValue = *text++ - '0';
	
	while (text != end && isdigit(*text)) {
		*pValue = *pValue * 10 + (*text++ - '0');
	}
	
	return text;
}

static const char * Float(const char * text, const char * end, GLfloat * pValue) {
	GLfloat mantissa = 0.0f;
	GLboolean minus = GL_FALSE;
	
	if (!text || text == end) {
		return NULL;
	}
	
	if (*text == '-') {
		minus = GL_TRUE;
		++text;
	}
	
	if (text == end) {
		return NULL;
	}
	
	if (isdigit(*text)) {
		do {
			mantissa = mantissa * 10.0f + (*text++ - '0');
		} while (text != end && isdigit(*text));
	} else if (*text != '.') {
		return NULL;
	}
	
	if (text != end && *text == '.') {
		GLfloat factor = 0.1f;
		++text;
		
		while (text != end && isdigit(*text)) {
			mantissa += factor * (*text++ - '0');
			factor *= 0.1f;
		}
	}
	
	if (minus) {
		mantissa = -mantissa;
	}
	
	if (text != end && (*text == 'e' || *text == 'E')) {
		GLint exponent = 0;
		GLboolean negExponent = GL_FALSE;
		++text;
		
		if (text == end) {
			return NULL;
		}
		
		if (*text == '-') {
			negExponent = GL_TRUE;
			++text;
		} else if (*text == '+') {
			++text;
		}
		
		if (text == end || !isdigit(*text)) {
			return NULL;
		}
		
		do {
			exponent = exponent * 10 + (*text++ - '0');
		} while (text != end && isdigit(*text));
		
		if (negExponent) {
			exponent = -exponent;
		}
		
		*pValue = GlesPowf(mantissa, (GLfloat) exponent);
	} else {
		*pValue = mantissa;
	}
	
	return text;
}

static const char * SkipSpaceIdentifier(const char * text, const char * end, const char **pBegin) {
	text = SkipSpace(text, end);
	
	if (!text) {
		return NULL;
	}
	
	*pBegin = text;	
	return Identifier(text, end);
}

static const char * SkipSpaceImproperIdentifier(const char * text, const char * end, const char **pBegin) {
	text = SkipSpace(text, end);
	
	if (!text) {
		return NULL;
	}
	
	*pBegin = text;	
	return ImproperIdentifier(text, end);
}

/**
 * Parse a type descriptor into the given memory pool.
 * 
 * @param	pool	the memory pool in which to create the type object
 * @param	text	the first character of the type descriptor
 * @param	end		reference to first character beyond string
 * @param	pType	out: resulting type value
 * 
 * @return	reference to next character to parse
 */
const char * GlesParseType(MemoryPool * pool, const char * text, const char * end, Type ** pType) {
	
	TypeValue typeValue;
	Precision precision = PrecisionUndefined;
	const char * start;
	const struct Keyword * keyword;

	*pType = NULL;
		
	if (!(text = SkipSpaceIdentifier(text, end, &start))) {
		return NULL;
	}
	
	keyword = CheckOpcode(start, text - start);

	if (!keyword) {
		return NULL;
	}
		
	switch (keyword->opcode) {
	case OpcodeLow:
		precision = PrecisionLow;
		goto nextToken;
		
	case OpcodeMedium:
		precision = PrecisionMedium;
		goto nextToken;
		
	case OpcodeHigh:
		precision = PrecisionHigh;
		
	nextToken:
		if (!(text = SkipSpaceIdentifier(text, end, &start))) {
			return NULL;
		}
		
		keyword = CheckOpcode(start, text - start);
		
		if (!keyword) {
			return NULL;
		}
		
	default:
		break;
	}
	
	switch (keyword->opcode) {
	case OpcodeBool:		typeValue = TypeBool;			break;
	case OpcodeBoolVec2:	typeValue = TypeBoolVec2;		break;
	case OpcodeBoolVec3:	typeValue = TypeBoolVec3;		break;
	case OpcodeBoolVec4:	typeValue = TypeBoolVec4;		break;
	case OpcodeInt:			typeValue = TypeInt;			break;
	case OpcodeIntVec2:		typeValue = TypeIntVec2;		break;
	case OpcodeIntVec3:		typeValue = TypeIntVec3;		break;
	case OpcodeIntVec4:		typeValue = TypeIntVec4;		break;
	case OpcodeFloat:		typeValue = TypeFloat;			break;
	case OpcodeFloatVec2:	typeValue = TypeFloatVec2;		break;
	case OpcodeFloatVec3:	typeValue = TypeFloatVec3;		break;
	case OpcodeFloatVec4:	typeValue = TypeFloatVec4;		break;
	case OpcodeFloatMat2:	typeValue = TypeFloatMat2;		break;
	case OpcodeFloatMat3:	typeValue = TypeFloatMat3;		break;
	case OpcodeFloatMat4:	typeValue = TypeFloatMat4;		break;
	case OpcodeSampler2D:	typeValue = TypeSampler2D;		break;
	case OpcodeSampler3D:	typeValue = TypeSampler3D;		break;
	case OpcodeSamplerCube:	typeValue = TypeSamplerCube;	break;
	
	default:
		return NULL;
	}
	
	*pType = GlesBasicType(typeValue, precision);
	
	if (text != end) {
		text = SkipSpace(text, end);
		
		if (*text == '[') {
			GLsizeiptr elements = 0;
			
			text = Number(SkipSpace(text, end), end, &elements);
			
			if (!text) {
				return NULL;
			}
			
			text = MustBe(text, end, ']');
			
			if (!text) {
				return NULL;
			}
			
			*pType = GlesTypeArrayCreate(pool, *pType, elements);
		}
	}
	
	return text;
}

static const char * ParseVariableDeclaration(ShaderProgramParser * parser,
										  	 const char * text, const char * end,
										  	 const char ** pName, GLsizeiptr * pLength,
										  	 Type ** pType, ProgVarSegment * pSegment,
										  	 GLuint * pLocation) {
										  	 	
	const char * name = NULL;
	GLsizeiptr length = 0;
	GLsizeiptr size = 1;
	Type * type = NULL;
	
	*pSegment = ProgVarSegNone;
	*pLocation = 0;
			  	
	text = SkipSpaceIdentifier(text, end, &name);
	
	if (!text) {
		return NULL;
	}
	
	length = text - name;
	text = SkipSpace(text, end);
	
	if (text != end && *text == '[') {
		++text;
		
		if (text == end || !isdigit(*text)) {
			return NULL;
		}
		
		size = *text++ - '0';
		
		while (text != end && isdigit(*text)) {
			size = size * 10 + (*text++ - '0');
		}
		
		if (!(text = MustBe(text, end, ']'))) {
			return NULL;
		}
	}
	
	if (!(text = MustBe(text, end, ':'))) {
		return NULL;
	}
	
	text = GlesParseType(parser->generator.result->memory, text, end, &type);
	
	if (!type || type->base.size != size) {
		return NULL;
	}
					
	text = SkipSpace(text, end);
	
	if (text != end && *text == '@') {
		const char * segment;
		GLsizeiptr segmentLength;
		GLuint location;
		
		text = SkipSpaceIdentifier(text, end, &segment);
		
		if (!text) {
			return NULL;
		}
		
		segmentLength = text - name;
		
		if (segmentLength == 5 && !GlesMemcmp(segment, "CONST", 5)) {
			*pSegment = ProgVarSegParam;
		} else if (segmentLength == 5 && !GlesMemcmp(segment, "PARAM", 5)) {
			*pSegment = ProgVarSegParam;
		} else if (segmentLength == 6 && !GlesMemcmp(segment, "ATTRIB", 6)) {
			*pSegment = ProgVarSegAttrib;
		} else if (segmentLength == 7 && !GlesMemcmp(segment, "VARYING", 7)) {
			*pSegment = ProgVarSegVarying;
		} else if (segmentLength == 5 && !GlesMemcmp(segment, "LOCAL", 5)) {
			*pSegment = ProgVarSegLocal;
		} else {
			return NULL;
		}
		
		text = SkipSpace(text, end);
		
		if (text != end && *text == '[') {
			++text;
			
			if (text == end || !isdigit(*text)) {
				return NULL;
			}
			
			location = *text++ - '0';
			
			while (text != end && isdigit(*text)) {
				location = location * 10 + (*text++ - '0');
			}
			
			if (!(text = MustBe(text, end, ']'))) {
				return NULL;
			}
			
			*pLocation = location;
		}
	}
	
	*pName = name;
	*pLength = length;
	*pType = type;
			  	 	
	return text;
}

/**
 * Parse an input variable declaration for the low-level shader language
 * 
 * @param	parser	program parser data structure
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseInputDeclaration(ShaderProgramParser * parser,
										  const char * text, const char * end) {
	
	const char * name, *externalName;
	GLsizeiptr length, externalLength;
	Type * type;
	ProgVar * var;
	ProgVarSegment segment;
	GLuint location;
	
	text = ParseVariableDeclaration(parser, text, end, &name, &length, &type, &segment, &location);
	
	if (!text) {
		return NULL;
	}
		
	text = SkipSpaceIdentifier(MustBe(text, end, '='), end, &externalName);
	
	if (!text) {
		return NULL;
	}
		
	externalLength = text - externalName;
	
	if (LookupSymbol(&parser->variables, name, length)) {
		return NULL;
	}
	
	var = GlesCreateProgVarIn(parser->generator.result, type, externalName, externalLength);
	DefineSymbol(&parser->variables, name, length, var);
	
	var->base.segment = segment;
	var->base.location = location;
	
	return MustBe(text, end, ';');
}
 
/**
 * Parse an ouput variable declaration for the low-level shader language
 * 
 * @param	program		the shader program being parsed
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseOutputDeclaration(ShaderProgramParser * parser,
										   const char * text, const char * end) {
	
	
	const char * name, *externalName;
	GLsizeiptr length, externalLength;
	Type * type;
	ProgVar * var;
	ProgVarSegment segment;
	GLuint location;
	
	text = ParseVariableDeclaration(parser, text, end, &name, &length, &type, &segment, &location);
	
	if (!text) {
		return NULL;
	}
		
	text = SkipSpaceIdentifier(MustBe(text, end, '='), end, &externalName);
	
	if (!text) {
		return NULL;
	}
			
	externalLength = text - externalName;
	
	if (LookupSymbol(&parser->variables, name, length)) {
		return NULL;
	}
	
	var = GlesCreateProgVarOut(parser->generator.result, type, externalName, externalLength);
	DefineSymbol(&parser->variables, name, length, var);
	
	var->base.segment = segment;
	var->base.location = location;
	
	return MustBe(text, end, ';');
}

static const char * ParseConstant(ShaderProgramParser * parser,
								   const char * text, const char * end,
								   GLfloat values[], GLsizei elements) {
	GLsizei count;
	
	text = Float(SkipSpace(MustBe(text, end, '{'), end), end, &values[0]);
	
	if (!text) {
		return NULL;
	}
	
	for (count = 1; count < elements; ++count) {
		text = Float(SkipSpace(MustBe(text, end, ','), end), end, &values[count]);

		if (!text) {
			return NULL;
		}		
	}
	
	while (count < 4) {
		values[count++] = 0.0f;
	}
	
	return MustBe(text, end, '}');
}

static const char * ParseConstants(ShaderProgramParser * parser,
								   const char * text, const char * end, 
								   Type * type, Constant * value) {
								   
	GLsizei index;
	GLfloat values[4];
		
	switch (type->base.kind) {
	case TypeBool:
	case TypeBoolVec2:
	case TypeBoolVec3:
	case TypeBoolVec4:
		text = ParseConstant(parser, text, end, values, type->base.elements);
		
		if (!text) {
			return NULL;
		}
		
		for (index = 0; index < 4; ++index) {
			value->boolValue[index] = (values[index] != 0.0f);
		}
		
		break;
	
	case TypeInt:
	case TypeIntVec2:
	case TypeIntVec3:
	case TypeIntVec4:
		text = ParseConstant(parser, text, end, values, type->base.elements);
		
		if (!text) {
			return NULL;
		}
		
		for (index = 0; index < 4; ++index) {
			value->intValue[index] = (GLint) values[index];
		}
		
		break;
	
	case TypeFloat:
	case TypeFloatVec2:
	case TypeFloatVec3:
	case TypeFloatVec4:
		text = ParseConstant(parser, text, end, value->floatValue, type->base.elements);
		
		if (!text) {
			return NULL;
		}

		break;
		
	case TypeFloatMat2:
	case TypeFloatMat3:
	case TypeFloatMat4:
		for (index = 0; index < type->base.size; ++index) {
			text = ParseConstant(parser, text, end, value[index].floatValue, type->base.elements);
		
			if (!text) {
				return NULL;
			}
		}
			
		break;
	
	case TypeArray:
		for (index = 0; index < type->array.elements; ++index) {
			text = ParseConstants(parser, text, end, type->array.elementType, 
								  value + index * type->array.elementType->base.size);
			
			if (!text) {
				return NULL;
			}
		}
		
		break;
		
	case TypeStruct:
		for (index = 0; index < type->structure.numFields; ++index) {
			text = ParseConstants(parser, text, end, type->structure.fields[index].type,
								  value + type->structure.fields[index].offset);
			
			if (!text) {
				return NULL;
			}
			
			if (index < type->structure.numFields - 1) {
				text = MustBe(text, end, ',');
				
				if (!text) {
					return NULL;
				}
			}
		}
		
		break;
	
	default:
		return NULL;
	}

	return text;	
}

/**
 * Parse a parameter variable or constant value declaration for the 
 * low-level shader language
 * 
 * @param	program		the shader program being parsed
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseParamDeclaration(ShaderProgramParser * parser,
										  const char * text, const char * end) {
	
	
	const char * name, *externalName;
	GLsizeiptr length, externalLength;
	Constant * value;
	Type * type;
	ProgVar * var;
	ProgVarSegment segment;
	GLuint location;
	
	text = ParseVariableDeclaration(parser, text, end, &name, &length, &type, &segment, &location);
	
	if (LookupSymbol(&parser->variables, name, length)) {
		return NULL;
	}
	
	text = SkipSpace(MustBe(text, end, '='), end);
	
	if (!text || text == end) {
		return NULL;
	}
			
	if (isalpha(*text) || *text == '$' || *text == '_') {
		/* uniform bound to external identifier */
		externalName = text;
		
		do {
			++text;
		} while (text != end && (isalnum(*text) || *text == '$' || *text == '_'));
		
		externalLength = text - externalName;
			
		var = GlesCreateProgVarParam(parser->generator.result, type, externalName, externalLength);
	} else {
		value = 
			GlesMemoryPoolAllocate(parser->generator.result->memory, 
								   sizeof(Constant) * type->base.size);
								   
		text = ParseConstants(parser, text, end, type, value);
		
		if (!text) {
			return NULL;
		}
				
		var = GlesCreateProgVarConst(parser->generator.result, value, type);
	}											  
	
	DefineSymbol(&parser->variables, name, length, var);
	
	var->base.segment = segment;
	var->base.location = location;
	
	return MustBe(text, end, ';');
}

/**
 * Parse an temporary variable declaration for the low-level shader language
 * 
 * @param	program		the shader program being parsed
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseTempDeclaration(ShaderProgramParser * parser,
										 const char * text, const char * end) {
	
	const char * name;
	GLsizeiptr length;
	Type * type;
	ProgVar * var;
	ProgVarSegment segment;
	GLuint location;
	
	text = ParseVariableDeclaration(parser, text, end, &name, &length, &type, &segment, &location);
	
	if (!text) {
		return NULL;
	}
	 
	if (LookupSymbol(&parser->variables, name, length)) {
		return NULL;
	}
	
	var = GlesCreateProgVarTemp(parser->generator.result, type);
	DefineSymbol(&parser->variables, name, length, var);

	var->base.segment = segment;
	var->base.location = location;
	
	return MustBe(text, end, ';');
}
 
/**
 * Parse an address variable declaration for the low-level shader language
 * 
 * @param	program		the shader program being parsed
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseAddressDeclaration(ShaderProgramParser * parser,
											const char * text, const char * end) {
	const char * name = NULL;
	GLsizeiptr length = 0;
	ProgVarAddr * addr = NULL;
			  	
	text = SkipSpace(text, end);
	
	if (text == end || (!isalpha(*text) && *text != '$' && *text != '_')) {
		return NULL;
	}
			
	name = text;
	
	do {
		++text;
	} while (text != end && (isalnum(*text) || *text == '$' || *text == '_'));
	
	length = text - name;

	if (LookupSymbol(&parser->addresses, name, length)) {
		return NULL;
	}
	
	addr = GlesCreateProgVarAddr(parser->generator.result);
	DefineSymbol(&parser->addresses, name, length, addr);
	
	return MustBe(text, end, ';');
}

static const char * ParsePrecisionSuffix(ShaderProgramParser * parser, 
										 const char * text, const char * end,
										 Precision * pPrec) {
	if (!text) {
		return NULL;
	}
	
	if (text != end && *text == '.') {
		++text;
		
		if (text == end) {
			return NULL;
		}
		
		switch (toupper(*text++)) {
		case 'L':	*pPrec = PrecisionLow; 			break;
		case 'M':	*pPrec = PrecisionMedium;		break;
		case 'H':	*pPrec = PrecisionHigh;			break;
		case 'U':	*pPrec = PrecisionUndefined;	break;
		default:	return NULL;
		}
	} else {
		*pPrec = PrecisionUndefined;
	} 

	return text;
}

static const char * ParseProgVarAddr(ShaderProgramParser * parser, 
									 const char * text, const char * end,
									 ProgVarAddr ** pAddr) {
	const char * name;
	GLsizeiptr length;
	text = SkipSpaceIdentifier(text, end, &name);

	if (!text) {
		return NULL;
	}
	
	length = text - name;
	
	*pAddr = LookupSymbol(&parser->addresses, name, length);
	
	if (!*pAddr) {
		/* reference to undefined variable */
		return NULL;
	}
	
	return text;
}

static const char * ParseLabel(ShaderProgramParser * parser, 
							   const char * text, const char * end,
							   Label ** pLabel) {
	const char * begin;
	Label * label;
	text = SkipSpaceIdentifier(text, end, &begin);

	if (!text) {
		return NULL;
	}
	
	label = LookupSymbol(&parser->labels, begin, text - begin);
	
	if (!label) {
		label = GlesCreateLabel(&parser->generator, NULL);
		DefineSymbol(&parser->labels, begin, text - begin, label);
	}
	
	*pLabel = label;
		
	return text;
}

static const char * ParseProgVar(ShaderProgramParser * parser, 
								 const char * text, const char * end,
								 ProgVar ** pVar, GLsizeiptr * pOffset,
								 ProgVarAddr ** pAddr) {
	const char * name;
	
	*pVar = NULL;
	*pOffset = 0;
	*pAddr = NULL;
	
	text = SkipSpaceIdentifier(text, end, &name);

	if (!text) {
		return NULL;
	}
	
	*pVar = LookupSymbol(&parser->variables, name, text - name);
	
	if (!*pVar) {
		/* undefined symbol */
		return NULL;
	}
	
	text = SkipSpace(text, end);
	
	if (text != end && *text == '[') {
		++text;
		
		text = SkipSpace(text, end);
		
		if (!text || text == end) {
			return NULL;
		}
		
		if (isalpha(*text) || *text == '$' || *text == '_') {
			text = Identifier(text, end);
			
			if (!text) {
				return NULL;
			}
			
			*pAddr = LookupSymbol(&parser->addresses, name, text - name);
			
			if (!*pAddr) {
				/* undefined variable name */
				return NULL;
			}

			text = SkipSpace(text, end);
			
			if (text != end && *text == '+') {
				text = Number(SkipSpace(text + 1, end), end, pOffset);
			} 
		} else if (isdigit(*text)) {
			text = Number(SkipSpace(text, end), end, pOffset);
		} else {
			return NULL;
		}
		
		text = MustBe(text, end, ']');
	}
	
	return text;
}

static const char * ParseDstReg(ShaderProgramParser * parser, 
								const char * text, const char * end,
								DstReg * pDstReg) {
									
	ProgVarAddr * addr = NULL;

	if (!(text = ParseProgVar(parser, text, end, 
							  (ProgVar **) &pDstReg->reference.base, &pDstReg->offset, &addr))) {
		return NULL;
	}
	
	if (addr || 
		(pDstReg->reference.base->kind != ProgVarKindOut &&
		 pDstReg->reference.base->kind != ProgVarKindTemp)) {
		return NULL;
	}
	
	if (text != end && *text == '.') {
		pDstReg->maskX =
		pDstReg->maskY =
		pDstReg->maskZ =
		pDstReg->maskW = GL_FALSE;
		++text;
		
		if (text != end && *text == 'x') {
			++text;
			pDstReg->maskX = GL_TRUE;
		}
		
		if (text != end && *text == 'y') {
			++text;
			pDstReg->maskY = GL_TRUE;
		}
		
		if (text != end && *text == 'z') {
			++text;
			pDstReg->maskZ = GL_TRUE;
		}
		
		if (text != end && *text == 'w') {
			++text;
			pDstReg->maskW = GL_TRUE;
		}
	} else {
		pDstReg->maskX =
		pDstReg->maskY =
		pDstReg->maskZ =
		pDstReg->maskW = GL_TRUE;
	}
	 
	return text;
}

static const char * ParseSrcReg(ShaderProgramParser * parser, 
								const char * text, const char * end,
								SrcReg * pSrcReg) {

	if (!(text = SkipSpace(text, end))) {
		return NULL;
	}
	
	if (text != end && *text == '-') {
		++text;
		pSrcReg->negate = GL_TRUE;
	} else {
		pSrcReg->negate = GL_FALSE;
	}
										
	if (!(text = ParseProgVar(parser, text, end, 
							  (ProgVar **) &pSrcReg->reference.base, &pSrcReg->offset, 
							  &pSrcReg->index))) {
		return NULL;
	}
	
	if (pSrcReg->index &&
		pSrcReg->reference.base->kind != ProgVarKindParam) {
		return NULL;
	}
	
	pSrcReg->selectX = 0;
	pSrcReg->selectY = 1;
	pSrcReg->selectZ = 2;
	pSrcReg->selectW = 3;

	if (text != end && *text == '.') {
		++text;
		
		if (text != end && 
			(*text == 'x' || *text == 'y' || *text == 'z' || *text == 'w')) {
			switch (*text++) {
			case 'x':	pSrcReg->selectX = 0; break;
			case 'y':	pSrcReg->selectX = 1; break;
			case 'z':	pSrcReg->selectX = 2; break;
			case 'w':	pSrcReg->selectX = 3; break;
			default: return NULL;
			}
		}   

		if (text != end && 
			(*text == 'x' || *text == 'y' || *text == 'z' || *text == 'w')) {
			switch (*text++) {
			case 'x':	pSrcReg->selectY = 0; break;
			case 'y':	pSrcReg->selectY = 1; break;
			case 'z':	pSrcReg->selectY = 2; break;
			case 'w':	pSrcReg->selectY = 3; break;
			default: return NULL;
			}
		}   

		if (text != end && 
			(*text == 'x' || *text == 'y' || *text == 'z' || *text == 'w')) {
			switch (*text++) {
			case 'x':	pSrcReg->selectZ = 0; break;
			case 'y':	pSrcReg->selectZ = 1; break;
			case 'z':	pSrcReg->selectZ = 2; break;
			case 'w':	pSrcReg->selectZ = 3; break;
			default: return NULL;
			}
		}   

		if (text != end && 
			(*text == 'x' || *text == 'y' || *text == 'z' || *text == 'w')) {
			switch (*text++) {
			case 'x':	pSrcReg->selectW = 0; break;
			case 'y':	pSrcReg->selectW = 1; break;
			case 'z':	pSrcReg->selectW = 2; break;
			case 'w':	pSrcReg->selectW = 3; break;
			default: return NULL;
			}
		}   
	}
	
	return text;
}

static const char * ParseSwizzleSrcReg(ShaderProgramParser * parser, 
									   const char * text, const char * end,
									   SrcRef *					pSrc,
								 	   ExtSwizzleOption *		pOptionX,
								 	   ExtSwizzleOption *		pOptionY,
								 	   ExtSwizzleOption	*		pOptionZ,
								 	   ExtSwizzleOption	*		pOptionW) {
								 	   	
	GLsizeiptr offset;
	ProgVarAddr * addr;
	ExtSwizzleOption options[4];
	GLsizei optionCount = 0;
	
	if (!(text = SkipSpace(text, end))) {
		return NULL;
	}
	
	if (!(text = ParseProgVar(parser, text, end, 
							  (ProgVar **) &pSrc->base, &offset, &addr))) {
		return NULL;
	}
	
	if (offset || addr) {
		return NULL;
	}

	options[0] = options[1] = options[2] = options[3] = ExtSwizzleSelect0;	

	do {
		GLboolean neg = GL_FALSE;
		
		text = SkipSpace(text, end);
		
		if (!text || text == end || *text != ',') {
			break;
		}
		
		text = SkipSpace(text + 1, end);
		
		if (*text == '-') {
			text = SkipSpace(text + 1, end);
			neg = GL_TRUE;
		}
		
		if (text == end) {
			return NULL;
		}
		
		switch (*text++) {
		case 'x': options[optionCount] = ExtSwizzleSelectX; break;
		case 'y': options[optionCount] = ExtSwizzleSelectY; break;
		case 'z': options[optionCount] = ExtSwizzleSelectZ; break;
		case 'w': options[optionCount] = ExtSwizzleSelectW; break;
		case '0': options[optionCount] = ExtSwizzleSelect0; break;
		case '1': options[optionCount] = ExtSwizzleSelect1; break;
		default:  return NULL;
		}
		
		if (neg) {
			options[optionCount] += ExtSwizzleSelectNeg;			
		}
		
		++optionCount;
	} while (optionCount < 4);
	
	*pOptionX = options[0];
	*pOptionY = options[1];
	*pOptionZ = options[2];
	*pOptionW = options[3];

	return text;
}

static const char * ParseCondMask(ShaderProgramParser * parser, 
								  const char * text, const char * end,
								  Cond * pCond,
								  GLubyte * pSelectX, GLubyte * pSelectY, 
								  GLubyte * pSelectZ, GLubyte * pSelectW) {
									
	GLsizei maskCount = 0;
	GLubyte mask[4];
	const char * begin;
	
	static struct {
		const char *	text;
		GLsizeiptr		length;
		Cond			cond;
	} conditions[] = {
		{ "T",	1,	CondT	},
		{ "F", 	1,	CondF	},
		{ "EQ",	2,	CondEQ	},
		{ "NE",	2,	CondNE	},
		{ "LT",	2,	CondLT	},
		{ "LE",	2,	CondLE	},
		{ "GT",	2,	CondGT	},
		{ "GE",	2,	CondGE	}
	};
	
	GLsizei index;
	
	text = SkipSpaceIdentifier(text, end, &begin);
	
	if (!text) {
		return NULL;
	}
	
	for (index = 0; index < GLES_ELEMENTSOF(conditions); ++index) {
		if (text - begin == conditions[index].length && 
			!GlesMemcmp(begin, conditions[index].text, conditions[index].length)) {
			*pCond = conditions[index].cond;
			break;
		} 
	}
	
	if (index == GLES_ELEMENTSOF(conditions)) {
		return NULL;
	}

	text = MustBe(text, end, '.');
	
	if (!text) {
		return NULL;
	}
	
	text = SkipSpaceIdentifier(text, end, &begin);
		
	if (!text || text - begin < 1 || text - begin > 4) {
		/* too few or too many characters in mask */ 
		return NULL;
	}
	
	while (begin != text) {
		switch (*begin) {
		case 'x': mask[maskCount++] = 0; break;
		case 'y': mask[maskCount++] = 1; break;
		case 'z': mask[maskCount++] = 2; break;
		case 'w': mask[maskCount++] = 3; break;
		default: return NULL;
		}
		
		++begin;
	}
	
	while (maskCount < 4) {
		mask[maskCount++] = mask[0];
	}
	
	*pSelectX = mask[0];
	*pSelectY = mask[1];
	*pSelectZ = mask[2];
	*pSelectW = mask[3];
		
	return text;
}

static const char * ParseOptCondMask(ShaderProgramParser * parser, 
									 const char * text, const char * end,
									 Cond * pCond,
									 GLubyte * pSelectX, GLubyte * pSelectY, 
									 GLubyte * pSelectZ, GLubyte * pSelectW) {

	if (text != end && *text == '(') {
		++text;
		
		text = ParseCondMask(parser, text, end, pCond, pSelectX, pSelectY, pSelectZ, pSelectW);
		
		return MustBe(text, end, ')');
	} else {
		*pCond = CondT;
		*pSelectX = 0;
		*pSelectY = 0;
		*pSelectZ = 0;
		*pSelectW = 0;
	}

	return text;
}

static const char * ParseTexTarget(ShaderProgramParser * parser, 
								   const char * text, const char * end,
								   TextureTarget * pTarget) {
	const char * begin = NULL;
	
	text = SkipSpaceImproperIdentifier(text, end, &begin);
	
	if (!text) {
		return NULL;
	}
	
	if (text - begin == 2 && !GlesMemcmp(begin, "2D", 2)) {
		*pTarget = TextureTarget2D; 
	} else if (text - begin == 2 && !GlesMemcmp(begin, "3D", 2)) {
		*pTarget = TextureTarget3D;
	} else if (text - begin == 4 && !GlesMemcmp(begin, "CUBE", 4)) {
		*pTarget = TextureTargetCube;
	} else {
		return NULL;
	}

	return text;
}

/**
 * Parse a unary instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseUnaryInstruction(ShaderProgramParser * parser, Opcode op,
										  const char * text, const char * end) {

	Precision prec;
	DstReg dst;
	SrcReg src;
											  	
	if (!(text = ParsePrecisionSuffix(parser, text, end, &prec))) {
		return NULL;
	}
	
	/* parse destination */
	if (!(text = ParseDstReg(parser, text, end, &dst))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSrcReg(parser, MustBe(text, end, ','), end, &src))) {
		return NULL;
	}
	
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}
	
	GlesGenInstUnary(&parser->generator, op, prec, &dst, &src);
	 	
	return text;
}
 
/**
 * Parse a binary instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseBinaryInstruction(ShaderProgramParser * parser, Opcode op,
										   const char * text, const char * end) {

	Precision prec;
	DstReg dst;
	SrcReg left, right;
											  	
	if (!(text = ParsePrecisionSuffix(parser, text, end, &prec))) {
		return NULL;
	}
	
	/* parse destination */
	if (!(text = ParseDstReg(parser, text, end, &dst))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSrcReg(parser, MustBe(text, end, ','), end, &left))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSrcReg(parser, MustBe(text, end, ','), end, &right))) {
		return NULL;
	}
	
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}
	
	GlesGenInstBinary(&parser->generator, op, prec, &dst, &left, &right);
	 	
	return text;
}

/**
 * Parse a ternary instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseTernaryInstruction(ShaderProgramParser * parser, Opcode op,
										  	const char * text, const char * end) {

	Precision prec;
	DstReg dst;
	SrcReg arg0, arg1, arg2;
											  	
	if (!(text = ParsePrecisionSuffix(parser, text, end, &prec))) {
		return NULL;
	}
	
	/* parse destination */
	if (!(text = ParseDstReg(parser, text, end, &dst))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSrcReg(parser, MustBe(text, end, ','), end, &arg0))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSrcReg(parser, MustBe(text, end, ','), end, &arg1))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSrcReg(parser, MustBe(text, end, ','), end, &arg2))) {
		return NULL;
	}
	
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}
	
	GlesGenInstTernary(&parser->generator, op, prec, &dst, &arg0, &arg1, &arg2);
	 	
	return text;
}

/**
 * Parse an extended swizzle instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseSwizzleInstruction(ShaderProgramParser * parser, Opcode op,
										  	const char * text, const char * end) {
	Precision prec;
	DstReg dst;
	SrcRef src;
 	ExtSwizzleOption optionX, optionY, optionZ, optionW;
 												  	
	if (!(text = ParsePrecisionSuffix(parser, text, end, &prec))) {
		return NULL;
	}
	
	/* parse destination */
	if (!(text = ParseDstReg(parser, text, end, &dst))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSwizzleSrcReg(parser, MustBe(text, end, ','), end, &src, 
									&optionX, &optionY, &optionZ, &optionW))) {
		return NULL;
	}
	
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}
	
	GlesGenInstSwizzle(&parser->generator, op, prec, &dst, src, optionX, optionY, optionZ, optionW);
	 	
	return text;
}

/**
 * Parse a branch instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseBranchInstruction(ShaderProgramParser * parser, Opcode op,
										   const char * text, const char * end) {

	Label * label = NULL;										   	
	Cond cond;
	GLubyte selectX, selectY, selectZ, selectW;
	
	if (!(text = ParseLabel(parser, text, end, &label))) {
		return NULL;
	}
	
	if (!(text = ParseOptCondMask(parser, text, end, &cond, &selectX, &selectY, &selectZ, &selectW))) {
		return NULL;
	}
											   	
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}

	GlesGenInstBranch(&parser->generator, op, label, cond, selectX, selectY, selectZ, selectW);
	GlesCreateBlock(&parser->generator);
	
	return text;
}

/**
 * Parse a conditional instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseCondInstruction(ShaderProgramParser * parser, Opcode op,
										 const char * text, const char * end) {
	Cond cond;
	GLubyte selectX, selectY, selectZ, selectW;
	
	if (!(text = ParseCondMask(parser, text, end, &cond, &selectX, &selectY, &selectZ, &selectW))) {
		return NULL;
	}
											   	
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}

	GlesGenInstCond(&parser->generator, op, cond, selectX, selectY, selectZ, selectW);
	GlesCreateBlock(&parser->generator);
	
	return text;
}

/**
 * Parse a no-operand instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseBaseInstruction(ShaderProgramParser * parser, Opcode op,
										 const char * text, const char * end) {
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}
	
	GlesGenInstBase(&parser->generator, op);
	
	switch (op) {
	case OpcodeELSE:
	case OpcodeENDIF:
	case OpcodeENDLOOP:
	case OpcodeENDREP:
		GlesCreateBlock(&parser->generator);
		break;
		
	default:
		;
	}
	 	
	return text;
}

/**
 * Parse a texture instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseTextureInstruction(ShaderProgramParser * parser, Opcode op,
										    const char * text, const char * end) {
	Precision prec;
	DstReg dst;
	SrcReg src;
	TextureTarget target;
	ProgVar * sampler = NULL;
	ProgVarAddr * addr = NULL;
	GLsizeiptr offset;
	
											  	
	if (!(text = ParsePrecisionSuffix(parser, text, end, &prec))) {
		return NULL;
	}
	
	/* parse destination */
	if (!(text = ParseDstReg(parser, text, end, &dst))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSrcReg(parser, MustBe(text, end, ','), end, &src))) {
		return NULL;
	}
	
	if (!(text = ParseProgVar(parser, MustBe(text, end, ','), end, &sampler, &offset, &addr))) {
		return NULL;
	}
	
	if (addr || (sampler->base.kind != ProgVarKindParam && sampler->base.kind != ProgVarKindTemp)) {
		return NULL;
	}
	
	if (!(text = ParseTexTarget(parser, MustBe(text, end, ','), end, &target))) {
		return NULL;
	}
		 
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}
	
	GlesGenInstTex(&parser->generator, op, prec, target, &dst, &src, sampler, offset);
	 	
	return text;
}

/**
 * Parse load address instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseAddressInstruction(ShaderProgramParser * parser, Opcode op,
											const char * text, const char * end) {
	ProgVarAddr *addr = NULL;
	SrcReg src;
											  	
	/* parse destination */
	if (!(text = ParseProgVarAddr(parser, text, end, &addr))) {
		return NULL;
	}
	
	/* parse source */
	if (!(text = ParseSrcReg(parser, MustBe(text, end, ','), end, &src))) {
		return NULL;
	}
	
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}
	
	GlesGenInstArl(&parser->generator, op, addr, &src);
	 	
	return text;
}
/**
 * Parse a source-operand-only instruction for the low-level shader language
 * 
 * @param	parser	reference to program parser structure
 * @param	op			instruction opcode
 * @param	text		pointer to current position in program text
 * @param	end			pointer beyond last character of input text
 * 
 * @return	pointer to next unparsed character
 */
static const char * ParseSourceInstruction(ShaderProgramParser * parser, Opcode op,
										   const char * text, const char * end) {
	SrcReg src;
											  	
	/* parse source */
	if (!(text = ParseSrcReg(parser, text, end, &src))) {
		return NULL;
	}
	
	if (!(text = MustBe(text, end, ';'))) {
		return NULL;
	}
	
	GlesGenInstSrc(&parser->generator, op, &src);

	switch (op) {
	case OpcodeLOOP:
	case OpcodeREP:
		GlesCreateBlock(&parser->generator);
		break;
		
	default:
		;
	}
	 	
	return text;
}

/**
 * Parse a shader program text describing an intermediate instruction stream.
 * 
 * @param	text	pointer to first character of input text
 * @param	length	number of characters in input text; if ~0, use '\0' terminator for
 * 					end of string
 * @param	pool	memory pool to use for result
 * @param	temp	memory pool to as working storage during parsing operation
 * 
 * @return	a newly parsed shader program instance, or NULL in case of an error.
 */
ShaderProgram * GlesParseShaderProgram(const char * text, GLsizeiptr length, 
									   MemoryPool * pool, MemoryPool * temp) {
									   	
	ShaderProgram * program = GlesCreateShaderProgram(pool);
	ShaderProgramParser parser;
	
	const char * end, *start;
	const struct Keyword * keyword;
	Label * label;
	
	GlesMemset(&parser, 0, sizeof parser);
	parser.generator.result = program;
	parser.generator.instructionCount = 0;
	parser.generator.currentList = &program->blocks;
	parser.temp = temp;
	InitSymbolTable(&parser.variables, temp);
	InitSymbolTable(&parser.addresses, temp);
	InitSymbolTable(&parser.labels, temp);
	GlesCreateBlock(&parser.generator);
	
	if (length == ~0) {
		length = GlesStrlen(text);
	}
	
	end = text + length;
	
	if (!program) {
		return NULL;
	}

	do {	
		/* parse next line of program */
		text = SkipSpace(text, end);

		
		if (text == end || *text == '\0') {
			break;
		}

		if (*text == '#') {
			/* comment */
			
			do {
				++text;
			} while (text != end && *text != '\n');
			
			continue; 
		} else {		
			/* cases: comment, declaration, label, or instruction */
			start = text;
			
			while (text != end && (isalnum(*text) || *text == '$' || *text == '_')) {
				++text;
			}
				
			if (text == start) {
				/* no identifier where expected */
				return NULL;
			}
			
			/* check for opcode/assembly lanuage keyword */
			keyword = CheckOpcode(start, text - start);
							 
			if (keyword) {
				/* opcode */
				switch (keyword->opcode) {
				case OpcodeARL:
					/* arl instruction */
					text = ParseAddressInstruction(&parser, OpcodeARL, text, end);
					break;
					
				case OpcodeABS:
				case OpcodeABS_SAT:
				case OpcodeCOS:
				case OpcodeCOS_SAT:
				case OpcodeEX2:
				case OpcodeEX2_SAT:
				case OpcodeEXP:
				case OpcodeEXP_SAT:
				case OpcodeFLR:
				case OpcodeFLR_SAT:
				case OpcodeFRC:
				case OpcodeFRC_SAT:
				case OpcodeLG2:
				case OpcodeLG2_SAT:
				case OpcodeLOG:
				case OpcodeLOG_SAT:
				case OpcodeMOV:
				case OpcodeMOV_SAT:
				case OpcodeRCP:
				case OpcodeRCP_SAT:
				case OpcodeRSQ:
				case OpcodeRSQ_SAT:
				case OpcodeSCS:
				case OpcodeSCS_SAT:
				case OpcodeSIN:
				case OpcodeSIN_SAT:
			    case OpcodeSSG:
					/* unary instruction */
					text = ParseUnaryInstruction(&parser, keyword->opcode, text, end);
					break;
					
				case OpcodeADD:
				case OpcodeADD_SAT:
				case OpcodeDP2:
				case OpcodeDP2_SAT:
				case OpcodeDP3:
				case OpcodeDP3_SAT:
				case OpcodeDP4:
				case OpcodeDP4_SAT:
				case OpcodeDPH:
				case OpcodeDPH_SAT:
				case OpcodeDST:
				case OpcodeDST_SAT:
				case OpcodeMAX:
				case OpcodeMAX_SAT:
				case OpcodeMIN:
				case OpcodeMIN_SAT:
				case OpcodeMUL:
				case OpcodeMUL_SAT:
				case OpcodePOW:
				case OpcodeSEQ:
			    case OpcodeSFL:
			  	case OpcodeSGE:
			    case OpcodeSGT:
			    case OpcodeSLE:
			    case OpcodeSLT:
			    case OpcodeSNE:
			    case OpcodeSTR:
				case OpcodeSUB:
				case OpcodeSUB_SAT:
				case OpcodeXPD:
				case OpcodeXPD_SAT:
					/* binary instructon */
					text = ParseBinaryInstruction(&parser, keyword->opcode, text, end);
					break;
					
				case OpcodeCAL:	
			#if GLES_VERTEX_SHADER_BRANCH || GLES_FRAGMENT_SHADER_BRANCH
				case OpcodeBRA: 
			#endif
					/* branch instruction */
					text = ParseBranchInstruction(&parser, keyword->opcode, text, end);
					break;
					
				case OpcodeIF:  
				case OpcodeRET:	
				case OpcodeBRK:	
				case OpcodeKIL:	
					/* conditional instruction */
					text = ParseCondInstruction(&parser, keyword->opcode, text, end);
					break;
					
				case OpcodeCMP:	
				case OpcodeCMP_SAT:	
				case OpcodeLRP:	
				case OpcodeLRP_SAT:	
				case OpcodeMAD:	
				case OpcodeMAD_SAT:	
					/* ternary instruction */
					text = ParseTernaryInstruction(&parser, keyword->opcode, text, end);
					break;
					
				case OpcodeSWZ:	
				case OpcodeSWZ_SAT:	
					/* swizzle instruction */
					text = ParseSwizzleInstruction(&parser, keyword->opcode, text, end);
					break;
					
				case OpcodeTEX:	
				case OpcodeTEX_SAT:	
				case OpcodeTXB:	
				case OpcodeTXB_SAT:	
				case OpcodeTXL:	
				case OpcodeTXL_SAT:	
				case OpcodeTXP:	
				case OpcodeTXP_SAT:	
					/* texture instruction */
					text = ParseTextureInstruction(&parser, keyword->opcode, text, end);
					break;
					
				case OpcodeELSE:
				case OpcodeENDIF:
				case OpcodeENDLOOP:
				case OpcodeENDREP:
					/* base instruction */
					text = ParseBaseInstruction(&parser, keyword->opcode, text, end);
					break;
					
				case OpcodeLOOP:
				case OpcodeREP:
				case OpcodeSCC:
					/* source instruction */
					text = ParseSourceInstruction(&parser, keyword->opcode, text, end);
					break;	

				case OpcodePHI:
					/* phi instruction */
					GLES_ASSERT(GL_FALSE);
					break;
			
				case OpcodeINPUT:	
					/* input variable declaration */
					text = ParseInputDeclaration(&parser, text, end);
					break;
					
				case OpcodeOUTPUT:						
					/* output variable declaration */
					text = ParseOutputDeclaration(&parser, text, end);
					break;
					
				case OpcodePARAM:						
					/* parameter or constant declaration */
					text = ParseParamDeclaration(&parser, text, end);
					break;
					
				case OpcodeTEMP:							
					/* temporary variable declaration */
					text = ParseTempDeclaration(&parser, text, end);
					break;
					
				case OpcodeADDRESS:						
					/* address variable declaration	*/
					text = ParseAddressDeclaration(&parser, text, end);
					break;
					
				default:
					return NULL;
				}
			} else if (text != end && *text == ':') {
				/* label */
				Label * label;
				
				++text;
				label = LookupSymbol(&parser.labels, start, text - start - 1);
				
				if (!label) {
					label = GlesCreateLabel(&parser.generator, NULL);
					DefineSymbol(&parser.labels, start, text - start - 1, label);
				}
				
				/* create a new block if there is no current block or it is not empty */
				if (!parser.generator.currentList->tail ||
					parser.generator.currentList->tail->first) {
					GlesCreateBlock(&parser.generator);
				}
				
				label->target = parser.generator.currentList->tail;
			} else {
				/* error */
				return NULL;
			}
		}
		
		if (!text) {
			return NULL;
		}
	} while (text != end);
	
	/* Verify that all labels have been defined */
	for (label = program->labels; label; label = label->next) {
		if (!label->target) {
			return NULL;
		}
	}
	
	return program;
}

