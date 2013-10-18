#ifndef GLES_STATE_STATE_H
#define GLES_STATE_STATE_H 1

/*
** ==========================================================================
**
** $Id: state.h 72 2007-10-08 00:44:04Z hmwill $			
** 
** GL Rendering State Declarations
**
** --------------------------------------------------------------------------
**
** $Author: hmwill $
** $Date: 2007-10-07 17:44:04 -0700 (Sun, 07 Oct 2007) $
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
** Constants
** --------------------------------------------------------------------------
*/

/** end of list value for arrays of index values */
#define NIL			(~0u)

/** value to used to mark bound values in object maps */
#define BOUND		(0u)


/*
** --------------------------------------------------------------------------
** Geometry Types
** --------------------------------------------------------------------------
*/

/**
 * 2-dimensional floating-point vector
 */
typedef union Vec2f {
	struct {
		GLfloat		x;
		GLfloat		y;
	};
	GLfloat			v[2];
} Vec2f;

/**
 * 2-dimensional integer vector
 */
typedef union Vec2i {
	struct {
		GLint		x;
		GLint		y;
	};
	GLint			v[2];
} Vec2i;

/**
 * 3-dimensional floating-point vector
 */
typedef union Vec3f {
	struct {
		GLfloat		x;
		GLfloat		y;
		GLfloat		z;
	};
	GLfloat			v[3];
} Vec3f;

/**
 * 3-dimensional integer vector
 */
typedef union Vec3i {
	struct {
		GLint		x;
		GLint		y;
		GLint		z;
	};
	GLint			v[3];
} Vec3i;

/**
 * 4-dimensional floating-point vector
 */
typedef union Vec4f {
	struct {
		GLfloat		x;
		GLfloat		y;
		GLfloat		z;
		GLfloat		w;
	};
	GLfloat			v[4];
} Vec4f;

/**
 * 4-dimensional integer vector
 */
typedef union Vec4i {
	struct {
		GLint		x;
		GLint		y;
		GLint		z;
		GLint		w;
	};
	GLint			v[4];
} Vec4i;

/**
 * RBGA color with floating point components
 */
typedef union Color {
	struct {
		GLclampf	red;				/**< red component 0..1				*/
		GLclampf	green;				/**< green component 0..1			*/
		GLclampf	blue;				/**< blue component 0..1			*/
		GLclampf	alpha;				/**< alpha component 0..1			*/
	};
	GLclampf		rgba[4];
} Color;

/**
 * RGBA color with unsigned byte components
 */
typedef union Colorub {
	struct {
		GLubyte		red;				/**< red component 0 .. 255			*/
		GLubyte		green;				/**< green component 0 .. 255		*/
		GLubyte		blue;				/**< blue component 0 .. 255		*/
		GLubyte		alpha;				/**< alpha component 0 .. 255		*/
	};
	GLubyte			rgba[4];
} Colorub;

/**
 * RGBA boolean mask
 */
typedef struct ColorMask {
	GLboolean		red;				/**< red component					*/
	GLboolean		green;				/**< green component				*/
	GLboolean		blue;				/**< blue component					*/
	GLboolean		alpha;				/**< alpha component				*/
} ColorMask;

/**
 * Rectangle structure with integer coordinates and sizes 
 */
typedef struct Rect {
	GLint			x;					/**< lower left corner x			*/
	GLint			y;					/**< lower left corner y			*/
	GLsizei			width;				/**< width in pixels				*/
	GLsizei			height;				/**< height in pixels				*/
} Rect;

/**
 * Size structure with integer values
 */
typedef struct Size {
	GLsizei			width;				/**< width in pixels				*/
	GLsizei			height;				/**< height in pixels				*/
} Size;

/**
 * Cube structure with integer coordinates and sizes 
 */
typedef struct Cube {
	GLint			x;					/**< lower left corner x			*/
	GLint			y;					/**< lower left corner y			*/
	GLint			z;					/**< lower left corner z			*/
	GLsizei			width;				/**< width in pixels				*/
	GLsizei			height;				/**< height in pixels				*/
	GLsizei			depth;				/**< depth in pixels				*/
} Cube;

/*
** --------------------------------------------------------------------------
** Shader Data Structures
** --------------------------------------------------------------------------
*/

/**
 * Post-vertex shader vertex geometry required for clipping and culling
 */
typedef struct VertexGeometry {
	Vec4f			position;				/**< gl_Position				*/
	GLfloat			pointSize;				/**< gl_PointSize				*/
	
	/** 
	 * Clipping flags.
	 * 
	 * The bits have the following interpretation:
	 * <ul>
	 * <li>bit 0: negative x
	 * <li>bit 1: positive x
	 * <li>bit 2: negative y
	 * <li>bit 3: positive y
	 * <li>bit 4: negative z
	 * <li>bit 5: positive z
	 * </ul>
	 */
	GLuint			cc;	
} VertexGeometry;

/**
 * Post-vertex shader vertex data structure 
 */
typedef struct Vertex {
	VertexGeometry	geometry;				/**< geometry info for vertex	*/
	
	/** Associated varying data */
	GLfloat			varying[GLES_MAX_VARYING_FLOATS];
} Vertex;

/**
 * Rasterizer input data struture for a single vertex
 */
typedef struct RasterVertex {
	Vec4f			screen;					/**< screen coordinates			*/
	
	/** 
	 * Associated varying data; usually points to data for corresponding 
	 * Vertex.
	 */
	const GLfloat *	varyingData;
} RasterVertex;

/*
** --------------------------------------------------------------------------
** Vertex Attribute Arrays
** --------------------------------------------------------------------------
*/

typedef struct Array Array;

/**
 * Prototype for functions used to retrieve vertex attribute data from
 * vertex arrays. Different functions will implement fetching of
 * different types and normalization.
 */
typedef void (*FetchFunc)(const Array * array, GLsizei index, Vec4f * result);

/**
 * Instances of Array represent memory areas that contain 
 * geometry information associated with an individual vertex attribute. Instances
 * of this type can be, but do not need to be, associated with an instance of
 * Buffer.
 */
struct Array {
	const void *	effectivePtr;		/**< pointer to array data at eval.	*/
	FetchFunc		fetchFunc;			/**< fetch function					*/
	const void *	ptr;				/**< pointer to array data			*/
	GLuint			boundBuffer;		/**< index of bound buffer object	*/
	GLint			size;				/**< number of array elements		*/
	GLsizei			stride;				/**< stride between cons. elements	*/
	GLenum			type;				/**< data type of elements			*/
	GLboolean		normalized;			/**< normalized data?				*/
	GLboolean		enabled;			/**< array enabled?					*/
};

/*
** --------------------------------------------------------------------------
** Vertex Buffers
** --------------------------------------------------------------------------
*/

/**
 * Instances of Buffer represent storage areas for geometry information
 * that are owned by the rendering library. If the graphics processor is
 * maintaining its own memory, then buffers are intended to reside in
 * this graphics processor controlled memory.
 */
typedef struct Buffer {
	void *			data;				/**< buffer data pointer			*/
	GLsizeiptr		size;				/**< size of buffer content			*/
	GLenum			bufferType;			/**< type of buffer					*/
	GLenum			usage;				/**< buffer usage mode				*/
	GLenum			access;				/**< buffer access type				*/
	GLboolean		mapped;				/**< buffer mapped to memory?		*/
	void *			mapPointer;			/**< current mapping address		*/
} Buffer;

/*
** --------------------------------------------------------------------------
** Textures
** --------------------------------------------------------------------------
*/

/**
 * Data structure to represent a 2-dimensional image
 */
typedef struct Image2D {
	void *					data;			/**< image data					*/
	GLsizei					width;			/**< width in pixels			*/
	GLsizei					height;			/**< height in pixels			*/
	GLenum					internalFormat;	/**< image format				*/
} Image2D;

/**
 * Data structure to represent a 3-dimensional image
 */
typedef struct Image3D {
	void *					data;			/**< image data					*/
	GLsizei					width;			/**< width in pixels			*/
	GLsizei					height;			/**< height in pixels			*/
	GLsizei					depth;			/**< depth in pixels			*/
	GLenum					internalFormat;	/**< image format				*/
} Image3D;

/**
 * Instances of TextureBase represent different types of texture data
 * that can be uploaded into the rendering library.
 */
typedef struct TextureBase {
	GLenum			textureType;		/**< type/target of the texture		*/
	GLenum			minFilter;			/**< minification filter mode		*/
	GLenum			magFilter;			/**< magnfication filter mode		*/
	GLenum			wrapS;				/**< s coordinate wrapping mode		*/
	GLenum			wrapT;				/**< t coordinate wrapping mode		*/
	GLenum			wrapR;				/**< r coordinate wrapping mode		*/
	GLuint			maxMipmapLevel;		/**< maximum mipmap level			*/
	GLboolean		isComplete;			/**< is the texture complete?		*/
} TextureBase;

/**
 * Textures comprised of a mipmap stack of 2-dimensional images are represented
 * as Texture2D objects, which are derived from TextureBase.
 */
typedef struct Texture2D {
	TextureBase		base;				/**< common texture data			*/
	
	/** Stack of mipmap images */
	Image2D			image[GLES_MAX_MIPMAP_LEVELS];
} Texture2D;

/**
 * Textures comprised of a mipmap stack of 3-dimensional images are represented
 * as Texture3D objects, which are derived from TextureBase. Support for
 * this texture type is an optional feature of OpenGL ES.
 */
typedef struct Texture3D {
	TextureBase		base;				/**< common texture data			*/
	
	/** Stack of mipmap image cubes */
	Image3D			image[GLES_MAX_MIPMAP_LEVELS];
} Texture3D;

/**
 * Texture cubes comprised of a cube of 6 mipmap stacks of 2-dimensional images are
 * represented as TextureCube objects, which are derived from TextureBase.
 */
typedef struct TextureCube {
	TextureBase		base;				/**< common texture data			*/
	
	Image2D		positiveX[GLES_MAX_MIPMAP_LEVELS];	/**< positive x axis	*/
	Image2D		negativeX[GLES_MAX_MIPMAP_LEVELS];	/**< negative x axis	*/
	Image2D		positiveY[GLES_MAX_MIPMAP_LEVELS];	/**< positive y axis	*/
	Image2D		negativeY[GLES_MAX_MIPMAP_LEVELS];	/**< negative Y axis	*/
	Image2D		positiveZ[GLES_MAX_MIPMAP_LEVELS];	/**< positive z axis	*/
	Image2D		negativeZ[GLES_MAX_MIPMAP_LEVELS];	/**< negative z axis	*/
} TextureCube;

/**
 * Instances of TextureState aggregate the OpenGL ES state vector 
 * for texture information.
 */
typedef struct TextureState {
	Texture2D	texture2D;				/**< 2D texture state				*/
	Texture3D	texture3D;				/**< 3D texture state				*/
	TextureCube	textureCube;			/**< cube map texture state			*/
} TextureState;

/**
 * Texture is a polymorphic data type that can represent any texture
 * type.
 */
typedef union Texture {
	TextureBase	base;					/**< common object type info		*/
	Texture2D	texture2D;				/**< actual texture state			*/
	Texture3D	texture3D;				/**< actual texture state			*/
	TextureCube	textureCube;			/**< cube map texture state			*/
} Texture;

/**
 * Instances of TextureImageUnit represent the individual texture
 * sampling units that will access texture data during vertex and fragment
 * shader execution.
 */
typedef struct TextureImageUnit {
	Texture *	boundTexture;			/**< currently active texture		*/
} TextureImageUnit;

/**
 * Instances of StencilParams collect the set of parameters to define 
 * stencil operations. 
 *
 * Each instance of State contains two instances of this type, one
 * describing the parameters for front-facing primitives, the other one
 * describing the parameters for back-facing primitives. 
 */
typedef struct StencilParams {
	GLenum			func;				/**< stencil function 			*/
	GLint			ref;				/**< stencil ref. value			*/
	GLuint			mask;				/**< stencil value mask			*/
	GLuint			writeMask;			/**< stencil write mask			*/
	GLenum			fail;				/**< stencil op. fail 			*/
	GLenum			zfail;				/**< stencil op. zfail 			*/
	GLenum			zpass;				/**< stencil op. zpass 			*/
} StencilParams;

/*
** --------------------------------------------------------------------------
** Shaders
** --------------------------------------------------------------------------
*/

/**
 * A single record to capture log information during compiling and linking.
 * 
 * A record dows not represent a line of the log, but rather just a 
 * consecutive sequence of characters, with potentially embedded formatting
 * characters for line and form feed. 
 */
typedef struct LogRecord {
	struct LogRecord * 	next;				/**< next record				*/
	
	char		text[GLES_LOG_BLOCK_SIZE];	/**< log record text			*/
} LogRecord;

/**
 * A representation of the log created during compiling and linking.
 * 
 * The log is comprised of a sequence of fixed length log records, whose
 * concatenation represnts the overall log text. When appending to a log
 * any remaining space in the last log record is filled before adding
 * additional log records.
 */
typedef struct Log {
	LogRecord *			first;			/**< first log record				*/
	LogRecord * 		last;			/**< last log record				*/
	GLsizei				logSize;		/**< total number of characters		*/
	GLsizei				blockSize;		/**< characters used in last block	*/
} Log;

/**
 * Instances of ShaderVariable represent variables that are used by instances
 * of Shader to retrieve input and capture output values. 
 * 
 * Uniform attributes remain constant during a single execution of the 
 * rendering pipeline.
 *  
 * Vertex attributes are used to retrieve input values from Array objects. 
 * 
 * Varying attributes are used for communication between the vertex and fragment 
 * shader, and are interpolated during the rasterization process. 
 */
typedef struct ShaderVariable {
	char *		name;					/**< null terminated attribute name	*/
	GLsizei		length;					/**< length of attribute name		*/
	GLsizei		location;				/**< location (index)				*/
	GLsizei		size;					/**< number of words				*/
	GLenum		type;					/**< type of attribute				*/
} ShaderVariable;

/**
 * An instance of Shader represents a user-definable, programmable piece
 * of the rendering pipeline.
 *
 * Currently, the library supports two types of shaders:
 * <ul>
 * 	<li>Vertex Shaders which are executed for each vertex processed
 * 		by the rendering pipeline.
 * 	<li>Fragment Shaders which are executed for each fragment processed
 * 		by the rendering pipeline.
 * </ul>
 */
typedef struct Shader {
	GLenum		type;					/**< shader type						*/
	
	/* shader source */
	char *		text;					/**< actual text lines of source	*/
	GLsizei		length;					/**< length of shader source		*/
	Log			log;					/**< shader compiler log			*/

	/* shader IL */
	char *		il;						/**< actual text lines of IL		*/
	GLsizei		size;					/**< length of shader intermediate	*/

	/** # of programs this shader is attached to */
	GLuint		attachmentCount;
	
	GLboolean	isCompiled;				/**< has this been compiled?		*/
	GLboolean	isDeleted;				/**< deletion requested?			*/
} Shader;

typedef struct FragContext FragContext;

/**
 * Signature of a fragment shader program function.
 */
typedef GLboolean (*FragmentProgram)(const FragContext * context);

/**
 * Signature of a vertex shader program function.
 */
typedef struct VertexContext VertexContext;

typedef GLboolean (*VertexProgram)(const VertexContext * context);

/*
** --------------------------------------------------------------------------
** Shader Programs
** --------------------------------------------------------------------------
*/

struct Executable;

/**
 * An instance of Program represents a combination of a vertex shader and a
 * fragment shader that are used together during the rendering process.
 */
typedef struct Program {
	GLuint			vertexShader;		/**< associated vertex shader		*/
	GLuint			fragmentShader;		/**< associated fragment shader		*/
	
	/* linking state */
	Log				log;				/**< linker log						*/
	
	/* execution state */
	Vec4f *			uniformData;		/**< uniform working data			*/
	GLuint *		uniformTypes;		/**< uniform type information		*/
	
	/* linkage state */
	GLboolean		isLinked;			/**< has this program been linked?	*/
	GLboolean		isValid;			/**< is this a valid program?		*/
	GLboolean		isDeleted;			/**< set to true if deletion requested*/

	/* generated executable */
	struct Executable *	
					executable;			/**< executable module				*/
} Program;

/*
** --------------------------------------------------------------------------
** Rendering Surface
** --------------------------------------------------------------------------
*/

struct Surface;

/**
 * Virtual function table for render surfaces
 */
typedef struct SurfaceVtbl {
	/** increase the reference count for this surface */
	void (*addref)(struct Surface * surface);	
	
	/** decrease the reference count for this surface */
	void (*release)(struct Surface * surface);	
	
	/** 
	 * lock the surface in memory, that is, pin the surface location to
	 * a referencable memory address.
	 * 
	 * This function will also ensure that anhy memory pointers in
	 * the surface structure will point to valid addresses.
	 */
	void (*lock)(struct Surface * surface);	
	
	/**
	 * unlock the surface, that is, unpin the surface location.
	 * 
	 * After this call, the mamory addresses in the surface structure
	 * can no longer assumed to be valid.
	 */
	void (*unlock)(struct Surface * surface);
} SurfaceVtbl;

/**
 * Instances of Surface represent targets for the rendering process.
 * They hold references to the individual memory areas that capture
 * color, depth and stencil information.
 */
typedef struct Surface {
	const SurfaceVtbl * vtbl;			/**< virtual function table			*/
	
	void *	colorBuffer;				/**< color buffer address			*/
	void *	depthBuffer;				/**< depth buffer address			*/
	void *	stencilBuffer;				/**< stencil buffer address			*/

	Size		size;					/**< surface dimensions				*/
	Rect		viewport;				/**< current visible dimension		*/

	GLsizei		colorPitch;				/**< memory stepping for scanlines	*/
	GLsizei		depthPitch;				/**< memory stepping for scanlines	*/
	GLsizei		stencilPitch;			/**< memory stepping for scanlines	*/

	/**
	 * format of color buffer
	 * 
	 * One of: 
	 * <ul>
	 * 	<li>GL_RGB8
	 *	<li>GL_RGBA8
	 * 	<li>GL_RGBA4
	 * 	<li>GL_RGB5_A1
	 * 	<li>GL_RGB565_OES
	 * </ul>
	 */
	GLenum		colorFormat;			
										
	/**
	 * format of depth buffer
	 * 
	 * One of:
	 * <ul>
	 * 	<li>GL_DEPTH_COMPONENT16
	 * 	<li>GL_DEPTH_COMPONENT24
	 * 	<li>GL_DEPTH_COMPONENT32
	 * </ul>
	 */
	GLenum		depthFormat;

	/** 
	 * format of stencil buffer
	 * 
	 * One of:
	 * <ul>			
	 * 	<li>GL_STENCIL_INDEX1_OES
	 * 	<li>GL_STENCIL_INDEX4_OES
	 * 	<li>GL_STENCIL_INDEX8_OES
	 * </ul>
	 */
	GLenum		stencilFormat;
	
	GLenum		colorReadFormat;		/**< best format to read out data	*/
	GLenum		colorReadType;			/**< best type to read out data		*/
	
	GLuint		redBits;				/**< number of red bits				*/
	GLuint		greenBits;				/**< number of green bits			*/
	GLuint		blueBits;				/**< number of blue bits			*/
	GLuint		alphaBits;				/**< number of alpha bits			*/
	GLuint		depthBits;				/**< number of depth bits			*/
	GLuint		stencilBits;			/**< number of stencil bits			*/
} Surface;

/**
 * Rendering surface location; basically, a multi-dimensional pointer
 * within the actual render target.
 */
typedef struct SurfaceLoc {
	Surface *	surface;				/**< reference to actual surface	*/
	void *		color;					/**< color buffer base address		*/
	void *		depth;					/**< depth buffer address			*/
	void *		stencil;				/**< stencil buffer address			*/
	GLsizeiptr	offset;					/**< offset within scanline			*/
} SurfaceLoc;

/*
** --------------------------------------------------------------------------
** Shader Program Execution State
** --------------------------------------------------------------------------
*/

typedef struct State State;				/* forward declaration for GL state	*/

/**
 * Fragment program execution context.
 * 
 * This structure collects all the memory areas that a fragment shader needs
 * to access during execution.
 */
struct FragContext {
	State *			state;				/**< current GL state					*/
	TextureImageUnit *	
					textureImageUnit;	/**< texture image units to use			*/
	const Vec4f *	constant;			/**< base address of constant data (r/o)*/
	const Vec4f *	uniform;			/**< Base address of uniform data (r/o)	*/
	const GLfloat *	varying;			/**< Base address of varying data (r/o) */
	Vec4f *			result;				/**< Base address of results (w) 		*/
	Vec4f *			temp;				/**< Base address of temp. data (r/w)	*/
};

/**
 * Vertex program execution context.
 * 
 * This structure collects all the memory areas that a vertex shader needs
 * to access during execution.
 */
struct VertexContext {
	State *			state;				/**< current GL state					*/
	TextureImageUnit *	
					textureImageUnit;	/**< texture image units to use			*/
	const Vec4f * 	constant;			/**< base address of constant data (r/o)*/
	const Vec4f *	uniform;			/**< Base address of uniform data (r/o)	*/
	const Vec4f *	attrib;				/**< Base address of attrib data (r/o) 	*/
	VertexGeometry*	geometry;			/**< Base address of geometry data (w)	*/
	GLfloat *		varying;			/**< Base address of results (w) 		*/
	Vec4f *			temp;				/**< Base address of temp. data (r/w) 	*/
};

/*
** --------------------------------------------------------------------------
** Rendering Functions
** --------------------------------------------------------------------------
*/

/**
 * Signature of the vertex processing function that is called for each
 * vertex during processing of DrawArray and DrawElements.
 */
typedef void (*DrawFunction)(State * state, GLuint index);

/**
 * Signature of the vertex processing function that is called after the
 * last vertex during processing of DrawArray and DrawElements.
 */
typedef void (*EndDrawFunction)(State * state);

/*
** --------------------------------------------------------------------------
** GL State
** --------------------------------------------------------------------------
*/

struct Compiler;
struct Linker;

/**
 * Instances of State aggregate all the GL state information that is
 * active at a given point in time. They are usually associated with the
 * current thread of execution.
 */
struct State {

	/*
	** ----------------------------------------------------------------------
	** EGL State
	** ----------------------------------------------------------------------
	*/

	/** the current source surface for reading operations */
	Surface *		readSurface;
	
	/** the current destination surface for write operations */
	Surface *		writeSurface;	

	/*
	** ----------------------------------------------------------------------
	** Exposed GL State
	** ----------------------------------------------------------------------
	*/

	/** the vertex attribute arrays */
	Array			vertexAttribArray[GLES_MAX_VERTEX_ATTRIBS];
	
	/** the element index array */
	Array			elementIndexArray;

	/** 
	 * current values for vertex attributes for which the corresponding
	 * array is not enabled.
	 */
	Vec4f			vertexAttrib[GLES_MAX_VERTEX_ATTRIBS];

	/** vertex buffer storage area */
	Buffer			buffers[GLES_MAX_BUFFERS];
	
	/** 
	 * the currently active array buffer that is modifed by subsequent
	 * API calls.
	 */
	GLuint			arrayBuffer;
	
	/** the currently active index array buffer */
	GLuint			elementArrayBuffer;		

	/** the free list for buffer objects; the first element is the list head */
	GLuint			bufferFreeList[GLES_MAX_BUFFERS];

	/* texture state */
	TextureState	textureState;				/**< default texture state		*/
	
	GLuint			texture2D;					/**< current 2D texture			*/
	GLuint			texture3D;					/**< current 3D texture			*/
	GLuint			textureCube;				/**< current cube map texture	*/

	Texture 	textures[GLES_MAX_TEXTURES];	/**< texture object storage		*/

	/** the free list for texture objects; the first element is the list head */
	GLuint		textureFreeList[GLES_MAX_TEXTURES];	
	
	/* texture image units */
	/**< index of texture image unit that is currently modified */
	GLuint			clientTextureUnit;
	
	/** texture image unit state */
	TextureImageUnit		textureUnits[GLES_MAX_TEXTURE_UNITS];

	/* shader state */
	Shader			shaders[GLES_MAX_SHADERS];	/**< shader object storage		*/

	/** the free list for shader objects; the first element is the list head */
	GLuint			shaderFreeList[GLES_MAX_SHADERS];

	/* program state */
	Program			programs[GLES_MAX_PROGRAMS];/**< program object storage		*/
	GLuint			program;					/**< current program			*/

	/** the free list for program objects; the first element is the list head */
	GLuint			programFreeList[GLES_MAX_SHADERS];
	
	/** flag for point sprite rendering; TBD clarify with final spec. */
	GLboolean		vertexProgramPointSizeEnabled;	

	/* rendering state */
	GLboolean		cullFaceEnabled;		/**< polygon culling enabled	*/
	GLenum			cullMode;				/**< culling mode				*/
	GLenum			frontFace;				/**< front face orientation		*/

	/* rasterization state */
	GLfloat			lineWidth;				/**< line rasterization width	*/
	GLfloat			pointSize;				/**< point rasterization size	*/
	GLfloat			polygonOffsetFactor;	/**< polygon offset factor		*/
	GLfloat			polygonOffsetUnits;		/**< polygon offset units		*/
	GLboolean		polygonOffsetFillEnabled;	/**< polygon offset enabled?*/

	/* fragment processing state */
	GLboolean		blendEnabled;			/**< is blending enabled		*/
	Color			blendColor;				/**< color for blending calc.	*/
	GLenum			blendEqnModeRBG;		/**< blending equation rgb		*/
	GLenum			blendEqnModeAlpha;		/**< blending equation alpha	*/
	GLenum			blendFuncSrcRGB;		/**< blending function src rgb	*/
	GLenum			blendFuncDstRGB;		/**< blending function dst rgb	*/
	GLenum			blendFuncSrcAlpha;		/**< blending function src alpha*/
	GLenum			blendFuncDstAlpha;		/**< blending function dst alpha*/
	ColorMask		colorMask;				/**< color write mask			*/

	GLboolean		depthTestEnabled;		/**< is depth test enabled		*/
	GLenum			depthFunc;				/**< depth test function		*/
	GLboolean		depthMask;				/**< depth write mask			*/
	
	GLboolean		ditherEnabled;			/**< dithering enabled?			*/

	/* no multi-sampling support at this point */
	GLboolean		multiSampleEnabled;		/**< multi-sampling?			*/
	GLboolean		sampleAlphaToCoverageEnabled;
	GLboolean		sampleAlphaToOneEnabled;
	GLboolean		sampleCoverageEnabled;
	GLclampf		sampleCovValue;			/**< sample coverage value		*/
	GLboolean		sampleCovInvert;		/**< invert flag				*/

	GLboolean		scissorTestEnabled;		/**< is the scissor test enabled*/
	Rect			scissorRect;			/**< scissor rectangle			*/

	GLboolean		stencilTestEnabled;		/**< is the stencil test enabled*/
	StencilParams	stencilFront;			/**< front facing parameters	*/
	StencilParams	stencilBack;			/**< back facing parameters		*/

	/* clear values */
	Color			clearColor;				/**< clear color				*/
	GLuint			clearDepth;				/**< clear depth value			*/
	GLint			clearStencil;			/**< clear stencil value		*/

	/* viewport state */
	Rect			viewport;				/**< viewport area				*/
	GLclampf		depthRange[2];			/**< [0] = near, [1] = far		*/

	Vec2f			viewportOrigin;			/* derived state				*/
	Vec2f			viewportScale;			/* derived state				*/
	GLfloat			depthOrigin;			/* derived state				*/
	GLfloat			depthScale;				/* derived state				*/

	/* general settings */
	GLuint			packAlignment;			/**< image pixel packing		*/
	GLuint			unpackAlignment;		/**< image pixel unpacking		*/

	/** 
	 * Does the fragment shader need to support derivatives?
	 */
	GLenum			fragmentShaderDerivativeHint;
	
	/**
	 * Do we want to generate mipmaps whenever the base texture is updated?
	 */
	GLenum			generateMipmapHint;

	GLenum			lastError;				/**< last error that occurred	*/

	/*
	** ----------------------------------------------------------------------
	** Internal Execution State
	** ----------------------------------------------------------------------
	*/

	/** indices of <i>enabled</i> attribute arrays */
	GLsizei			enabledAttribArrays[GLES_MAX_VERTEX_ATTRIBS];
	
	/** number of enabled attribute arrays */
	GLsizei			numEnabledAttribArrays;

	/** storage area for vertex attributes as input to vertex processing */
	Vec4f			tempVertexAttrib[GLES_MAX_VERTEX_ATTRIBS];

	/** storage area for results of vertex processing */
	Vertex			vertexQueue[GLES_MAX_VERTEX_QUEUE];
	
	/** temporary vertex storage */
	Vertex			tempVertices[GLES_MAX_VERTEX_QUEUE];

	/** function to call for each vertex within Begin/End loop */
	DrawFunction	drawFunction;
	
	/** function to call from within End to terminate rendering */
	EndDrawFunction	endDrawFunction;
	
	GLuint			primitiveState;		/**< state variable during Begin/End*/
	GLuint			nextIndex;			/**< index in vertex queue			*/
	
	/** current attribute values as input to vertex shader */
	Vec4f			currentAttrib[GLES_MAX_VERTEX_ATTRIBS];
	
	VertexContext	vertexContext;		/**< vertex shader execution context*/
	FragContext		fragContext;		/**< fragment shader execution context*/
	
	Rect			rasterRect;			/**< effective rasterization area	*/
	
	struct Compiler *	compiler;		/**< shader compiler reference */
	struct Linker *		linker;			/**< program linker reference */
};

/*
** For simplicity, let's start out with a single global state
*/
extern State * GlesGetGlobalState();

#define GLES_GET_STATE() (GlesGetGlobalState())


/*
** --------------------------------------------------------------------------
** State Management Functions
** --------------------------------------------------------------------------
*/

void GlesRecordError(State * state, GLenum error);

void GlesRecordInvalidEnum(State * state);
void GlesRecordInvalidValue(State * state);
void GlesRecordOutOfMemory(State * state);
void GlesRecordInvalidOperation(State * state);

GLboolean GlesValidateEnum(State * state, GLenum value, const GLenum * values, GLuint numValues);

void GlesInitState(State * state);
void GlesDeInitState(State * state);

/*
 * --------------------------------------------------------------------------
 * Array Functions
 * --------------------------------------------------------------------------
 */

void GlesInitArray(Array * array);
void GlesPrepareArray(State * state, Array * array);

/*
 * --------------------------------------------------------------------------
 * Vertex Buffer Functions
 * --------------------------------------------------------------------------
 */

void GlesInitBuffer(Buffer * buffer);
void GlesDeleteBuffer(Buffer * buffer);

/*
 * --------------------------------------------------------------------------
 * Shader Functions
 * --------------------------------------------------------------------------
 */

Shader * GlesGetShaderObject(State * state, GLuint shader);
void GlesInitShader(Shader * shader, GLenum shaderType);
void GlesDeleteShader(State * state, Shader * shader);
void GlesUnrefShader(State * state, GLuint shader);

/*
 * --------------------------------------------------------------------------
 * Program Functions
 * --------------------------------------------------------------------------
 */

void GlesInitProgram(Program * program);
void GlesDeleteProgram(State * state, Program * program);
Program * GlesGetProgramObject(State * state, GLuint program);
GLboolean GlesValidateProgram(State * state, Program * program, Log * log);
GLboolean GlesPrepareProgram(State * state);

/*
 * --------------------------------------------------------------------------
 * Object Management
 * --------------------------------------------------------------------------
 */

void GlesGenObjects(State * state, GLuint * freeList, GLuint maxElements, GLsizei n, GLuint *objs);
GLuint GlesBindObject(GLuint * freeList, GLuint maxElements);
void GlesUnbindObject(GLuint * freeList, GLuint maxElements, GLuint obj);
GLboolean GlesIsBoundObject(GLuint * freeList, GLuint maxElements, GLuint obj);

/*
 * --------------------------------------------------------------------------
 * Log Functions
 * --------------------------------------------------------------------------
 */

void GlesLogInit(Log * log);
void GlesLogDeInit(Log * log);
void GlesLogClear(Log * log);
void GlesLogAppend(Log * log, const char * text, GLsizei length);
void GlesLogAppendLog(Log * log, const  Log * sourceLog);
void GlesLogExtract(Log * log, GLsizei bufsize, char * text, GLsizei * length);

/*
 * --------------------------------------------------------------------------
 * Texture Functions
 * --------------------------------------------------------------------------
 */

void GlesInitImage2D(Image2D * image);
void GlesDeleteImage2D(State * state, Image2D * image);

void GlesInitImage3D(Image3D * image);
void GlesDeleteImage3D(State * state, Image3D * image);

void GlesInitTexture2D(Texture2D * texture);
void GlesDeleteTexture2D(State * state, Texture2D * texture);

void GlesInitTexture3D(Texture3D * texture);
void GlesDeleteTexture3D(State * state, Texture3D * texture);

void GlesInitTextureCube(TextureCube * texture);
void GlesDeleteTextureCube(State * state, TextureCube * texture);


void GlesTextureSample2D(const TextureImageUnit * unit, const Vec4f * coords,
						 const Vec4f * dx, const Vec4f * dy,
                         Vec4f * result);
                         
void GlesTextureSample3D(const TextureImageUnit * unit, const Vec4f * coords,
						 const Vec4f * dx, const Vec4f * dy,
                         Vec4f * result);
                         
void GlesTextureSampleCube(const TextureImageUnit * unit, const Vec4f * coords,
						   const Vec4f * dx, const Vec4f * dy,
                           Vec4f * result);

/*
 * --------------------------------------------------------------------------
 * Framebuffer Functions
 * --------------------------------------------------------------------------
 */

void GlesInitSurfaceLoc(Surface * surface, SurfaceLoc * loc, GLuint x, GLuint y);

void GlesStepSurfaceLoc(SurfaceLoc * loc, GLint x, GLint y);

void GlesWritePixel(State * state, const SurfaceLoc * loc, const Color * color, 
					GLfloat depth, GLboolean front);   
					                      
/*
 * --------------------------------------------------------------------------
 * Helper Functions
 * --------------------------------------------------------------------------
 */
void GlesInitRasterRect(State * state);

/*
 * --------------------------------------------------------------------------
 * ShaderVariable Helper Functions
 * --------------------------------------------------------------------------
 */

void GlesSortShaderVariables(ShaderVariable * first, GLsizeiptr count);
ShaderVariable * GlesFindShaderVariable(ShaderVariable * first, GLsizeiptr count,
	const char * name, GLsizeiptr length);

#endif /* ndef GLES_STATE_STATE_H */
