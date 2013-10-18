/*
** ==========================================================================
**
** $Id: render.c 60 2007-09-18 01:16:07Z hmwill $
**
** Rendering functions
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
#include "raster/raster.h"
#include "frontend/linker.h"


/*
** --------------------------------------------------------------------------
** Module data
** --------------------------------------------------------------------------
*/

/**
 * Valid enumeration values for culling
 */
static const GLenum CullFaceValues[] = {
	GL_FRONT,
	GL_BACK,
	GL_FRONT_AND_BACK
};

/**
 * Valid enumeration values for face orientation
 */
static const GLenum FrontFaceValues[] = {
	GL_CCW,
	GL_CW
};

/*
** --------------------------------------------------------------------------
** Common Primitive Processing
** --------------------------------------------------------------------------
*/

static void DrawPoints(State * state, GLuint index);
static void DrawLines(State * state, GLuint index);
static void DrawLineStrip(State * state, GLuint index);
static void DrawLineLoop(State * state, GLuint index);
static void DrawTriangles(State * state, GLuint index);
static void DrawTriangleStrip(State * state, GLuint index);
static void DrawTriangleFan(State * state, GLuint index);

static void EndDrawLineLoop(State * state);

/**
 * Prepare the rendering pipeline for renderng of the given
 * primitive type.
 * 
 * In particular, this function will attempt to lock the 
 * rendering surface.
 * 
 * @param state
 * 		pointer to the GL state object
 * 
 * @param mode
 * 		the primitive type to render
 * 
 * @return
 * 		GL_TRUE if the preparation was successful, otherwise GL_FALSE
 */
static GLboolean Begin(State * state, GLenum mode) {
	GLsizei attr;
	
	switch (mode) {
	case GL_POINTS:
		state->drawFunction = &DrawPoints;
		state->endDrawFunction = NULL;
		break;
		
	case GL_LINES:
		state->drawFunction = &DrawLines;
		state->endDrawFunction = NULL;
		break;
		
	case GL_LINE_STRIP:
		state->drawFunction = &DrawLineStrip;
		state->endDrawFunction = NULL;
		break;
		
	case GL_LINE_LOOP:
		state->drawFunction = &DrawLineLoop;
		state->endDrawFunction = &EndDrawLineLoop;
		break;
		
	case GL_TRIANGLES:
		state->drawFunction = &DrawTriangles;
		state->endDrawFunction = NULL;
		break;
		
	case GL_TRIANGLE_STRIP:
		state->drawFunction = &DrawTriangleStrip;
		state->endDrawFunction = NULL;
		break;
		
	case GL_TRIANGLE_FAN:
		state->drawFunction = &DrawTriangleFan;
		state->endDrawFunction = NULL;
		break;
		
	default:
		GlesRecordInvalidEnum(state);
		return GL_FALSE;
	}
	
	if (!GlesPrepareProgram(state)) {
		state->drawFunction = NULL;
		state->endDrawFunction = NULL;
		return GL_FALSE;
	}
	
	state->primitiveState = 0;
	state->nextIndex = 0;
	
	/* prepare vertex attrib arrays */
	
	for (attr = 0; attr < GLES_MAX_VERTEX_ATTRIBS; ++attr) {
		GlesPrepareArray(state, &state->vertexAttribArray[attr]);
	}
	
	/* prepare current attrib values */

	for (attr = 0; attr < GLES_MAX_VERTEX_ATTRIBS; ++attr) {
		if (state->vertexAttribArray[attr].enabled) {
			state->currentAttrib[attr].x =
			state->currentAttrib[attr].y =
			state->currentAttrib[attr].z = 0.0f;
			state->currentAttrib[attr].w = 1.0f;
		} else {
			state->currentAttrib[attr] = state->vertexAttrib[attr];
		}
	}
		
	state->writeSurface->vtbl->lock(state->writeSurface);
	GlesInitRasterRect(state);
		
	return GL_TRUE;
}

/**
 * Finish a rendering cycle.
 * 
 * In particular, this function will trigger any remaining rendering (e.g. to
 * close an open line loop) before releasing the rendering surface.

 * @param state
 * 		pointer to the GL state object
 */
static void End(State * state) {
	
	if (state->endDrawFunction) {
		state->endDrawFunction(state);
	}
	
	state->drawFunction = NULL;
	state->endDrawFunction = NULL;
	state->writeSurface->vtbl->unlock(state->writeSurface);	
}

/**
 * Determine the clipping flags for the given vertex.
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
 * 
 * @param vertex
 * 		the vertex to process
 */
static GLES_INLINE void CalcCC(Vertex * vertex) {
	vertex->geometry.cc =
		 (vertex->geometry.position.x < -vertex->geometry.position.w)       |
		((vertex->geometry.position.x >  vertex->geometry.position.w) << 1) |
		((vertex->geometry.position.y < -vertex->geometry.position.w) << 2) |
		((vertex->geometry.position.y >  vertex->geometry.position.w) << 3) |
		((vertex->geometry.position.z < -vertex->geometry.position.w) << 4) |
		((vertex->geometry.position.x >  vertex->geometry.position.w) << 5);
}

/**
 * Fetch the array data for the given vertex, and process it through the
 * vertex shader.
 * 
 * @param state
 * 		the current GL state
 * 
 * @param index
 * 		array index of the vertex to process
 * 
 * @param vertex
 * 		where to store the results of the operation
 */
static void SelectArrayElement(State * state, GLuint index, Vertex * vertex) {
	GLsizei attr;
	
	/* fetch the vertex attrib values */
	
	for (attr = 0; attr < GLES_MAX_VERTEX_ATTRIBS; ++attr) {
		if (state->vertexAttribArray[attr].enabled) {
			state->vertexAttribArray[attr].fetchFunc(&state->vertexAttribArray[attr], 
													 index, &state->currentAttrib[attr]);
		}
	}
	
	/* execute the vertex shader */
	
	state->vertexContext.geometry = &vertex->geometry;
	state->vertexContext.varying = vertex->varying;

	GlesVertexProgram(state->programs[state->program].executable)(&state->vertexContext);

#if 0
	/* pick projective half space; reportedly, this is a bug??? */
		
	if (vertex->geometry.position.w < 0) {
		vertex->geometry.position.x = -vertex->geometry.position.x; 
		vertex->geometry.position.y = -vertex->geometry.position.y; 
		vertex->geometry.position.z = -vertex->geometry.position.z; 
		vertex->geometry.position.w = -vertex->geometry.position.w; 
	}
#endif

	/* calculate the clipping flags */
	
	CalcCC(vertex);
}

static GLES_INLINE GLfloat InterpolateCoord(GLfloat x0f, GLfloat x1f, GLfloat coeff) {
	return x0f + (x1f - x0f) * coeff;
}

/**
 * Calculate the coordinates and associated varying values for a vertex defined
 * by clipping the segment from inside to outside at coeff.
 * 
 * @param newVertex
 * 		where to store the results
 * 
 * @param outside
 * 		the clipped-away end-point of the segment
 * 
 * @param inside
 * 		the end-point of the segment to remain
 * 
 * @param coeff
 * 		where along (inside, outside) to clip. 0 corresponds to inside, 1 to outside
 * 
 * @param numVarying
 * 		the number of varying attributes to process
 */
static void Interpolate(Vertex * newVertex, const Vertex * outside, const Vertex * inside, 
					    GLfloat coeff, GLsizei numVarying) {
	GLsizei index;
	
	for (index = 0; index < 4; ++index) {
		newVertex->geometry.position.v[index] =
			InterpolateCoord(inside->geometry.position.v[index], outside->geometry.position.v[index], coeff);
	}
			
	for (index = 0; index < numVarying; ++index) {
		newVertex->varying[index] =
			InterpolateCoord(inside->varying[index], outside->varying[index], coeff);
	}
}

/**
 * Clip a convex primitive at the viewing frustrum. 
 * 
 * The primitive is defined as the convex hull of a list of vertices given as
 * array of pointers. The resulting list of pointers to vertices will be stored
 * in the input array. 
 * 
 * @param state
 * 		reference to the GL state object
 * 
 * @param input
 * 		array of pointers to the input vertices
 * 
 * @param temp
 * 		temporary array of pointers to vertices used during this function
 * 
 * @param numVertices
 * 		number of vertices of input primitive
 * 
 * @return
 * 		number of vertices of clipped primitive.
 */
static GLsizei Clip(State * state, Vertex ** input, Vertex ** temp, GLsizei numVertices) {
    size_t plane;

    Vertex ** vilist = input;
    Vertex ** volist = temp;

    Vertex * vprev, * vnext;
    GLsizei i, icnt = numVertices, ocnt = 0;
	const GLsizei numVarying = GLES_MAX_VARYING_FLOATS;
	Vertex * nextTemporary = state->tempVertices;

	GLuint cc = 0;

	for (i = 0; i < icnt; ++i) {
		cc |= vilist[i]->geometry.cc;
	}
	
	if (!cc) {
		return numVertices;
	}

    for (plane = 0; plane < 6; plane++) {
		GLuint c, p = 1 << plane;
		GLsizei coord = plane >> 1;
		GLboolean inside, prev_inside;

		if (!(cc & p)) continue;

		cc = 0;
		ocnt = 0;
		vnext = vilist[icnt - 1];

		inside = !(vnext->geometry.cc & p);

		for (c = 0; c < icnt; c++) {

			Vertex * voutside, *vinside;
			prev_inside = inside;
			vprev = vnext;
			vnext = vilist[c];
			inside = !(vnext->geometry.cc & p);

			if (inside ^ prev_inside) {
				if (inside) { 
					vinside = vnext; 
					voutside = vprev; 
				} else { 
					voutside = vnext; 
					vinside = vprev; 
				}

				GLfloat ci = vinside->geometry.position.v[coord];
				GLfloat wi = vinside->geometry.position.w;
				GLfloat co = voutside->geometry.position.v[coord];
				GLfloat wo = voutside->geometry.position.w;

				if (!(plane & 1)) {
					wi = -wi; wo = -wo;
				}

				GLfloat num = wi - ci;
				GLfloat denom = co - ci - wo + wi;

				Vertex * newVertex = nextTemporary++;

				GLfloat coeff = num / denom;

				Interpolate(newVertex, voutside, vinside, coeff, numVarying);
				CalcCC(newVertex);
				cc |= newVertex->geometry.cc;

				volist[ocnt++] = newVertex;
			}

			if (inside /*&& c != icnt-1*/) {
				volist[ocnt++] = vilist[c];
				cc |= vilist[c]->geometry.cc;
			}
		}

		{
			// swap input and output lists
			Vertex ** vtlist;
			vtlist = vilist; vilist = volist; volist = vtlist;
		}

		icnt = ocnt;

		if (!icnt)
			return 0;

    }

	return icnt;
}

/**
 * Perform projection and viewport transformation on a given vertex.
 * 
 * @param state
 * 		reference to the GL state object
 * 
 * @param vertex
 * 		the processed vertex to project
 * 
 * @param raster
 * 		where to store the results of the projection
 */
static void ProjectVertexToWindowCoords(State * state, Vertex * vertex, RasterVertex * raster) {

	Vec4f vec = vertex->geometry.position;
	
	raster->screen.w = vec.w ? 1.0f / vec.w : 0.0f;

	raster->screen.x = vec.x * raster->screen.w * state->viewportScale.x + state->viewportOrigin.x;
	raster->screen.y = vec.y * raster->screen.w * state->viewportScale.y + state->viewportOrigin.y;
	raster->screen.z = vec.z * raster->screen.w * state->depthScale + state->depthOrigin;

	raster->varyingData = vertex->varying;
}

/**
 * Calculate the determinant of the (x, y, z) coordinates of the three vertices
 * passed into this function.
 * 
 * @param a
 * 		the first vertex
 * @param b
 * 		the second vertex
 * @param c
 * 		the third vertex
 * 
 * @return
 * 		the determinant of the (x, y, z) coordinates of the vertices
 */
static GLES_INLINE GLfloat Det(Vertex * a, Vertex * b, Vertex * c) {
	return
		+ a->geometry.position.x * (b->geometry.position.y * c->geometry.position.z - 
									b->geometry.position.z * c->geometry.position.y)
		- a->geometry.position.y * (b->geometry.position.x * c->geometry.position.z - 
									b->geometry.position.z * c->geometry.position.x)
		+ a->geometry.position.z * (b->geometry.position.x * c->geometry.position.y -
									b->geometry.position.y * c->geometry.position.x);
}

/*
** --------------------------------------------------------------------------
** Rendering of Points
** --------------------------------------------------------------------------
*/

static void DrawPoint(State * state, Vertex * a) {
	RasterVertex ra;
	
	if (a->geometry.cc) {
		/* 
		 * need clarification if clipping behavior is changed to only clip
		 * against front and back plane.
		 */
		return;
	}
	
	ProjectVertexToWindowCoords(state, a, &ra);
	
	/* TODO: calrify how shader works with point size manipulation */
	GlesRasterPointSprite(state, &ra, a->geometry.pointSize);
}

static void DrawPoints(State * state, GLuint index) {
	SelectArrayElement(state, index, &state->vertexQueue[0]);
	DrawPoint(state, &state->vertexQueue[0]);
}

/*
** --------------------------------------------------------------------------
** Rendering of Lines
** --------------------------------------------------------------------------
*/

static void DrawLine(State * state, Vertex * a, Vertex * b) {
	Vertex * vertices[4], * temp[4];
	
	if (a->geometry.cc & b->geometry.cc) {
		/* outside of frustrum */
		return;
	}
	
	vertices[0] = a;
	vertices[1] = b;
	
	if (Clip(state, vertices, temp, 2) >= 2) {
		RasterVertex ra, rb;
		
		ProjectVertexToWindowCoords(state, vertices[0], &ra);
		ProjectVertexToWindowCoords(state, vertices[1], &rb);
	}
}

static void DrawLines(State * state, GLuint index) {
	SelectArrayElement(state, index, &state->vertexQueue[state->nextIndex++]);

	if (state->nextIndex == 2) {
		DrawLine(state, &state->vertexQueue[0], &state->vertexQueue[1]);
		state->nextIndex = 0;
	}
}

static void DrawLineStrip(State * state, GLuint index) {
	SelectArrayElement(state, index, &state->vertexQueue[state->nextIndex++]);

	if (state->primitiveState != 0) {
		// determine line orienation based on parity
		if (state->nextIndex & 1) {
			DrawLine(state, &state->vertexQueue[1], &state->vertexQueue[0]);
		} else {
			DrawLine(state, &state->vertexQueue[0], &state->vertexQueue[1]);
		}
	} else {
		// remember that we have seen the first vertex
		state->primitiveState = 1;
	}

	if (state->nextIndex == 2) {
		state->nextIndex = 0;
	}
}

static void DrawLineLoop(State * state, GLuint index) {
	SelectArrayElement(state, index, &state->vertexQueue[state->nextIndex++]);

	if (state->primitiveState == 2) {
		// we have seen at least 3 vertices
		if (state->nextIndex & 1) {
			DrawLine(state, &state->vertexQueue[1], &state->vertexQueue[2]);
		} else {
			DrawLine(state, &state->vertexQueue[2], &state->vertexQueue[1]);
		}
	} else if (state->primitiveState == 1) {
		// determine line orienation based on parity
		DrawLine(state, &state->vertexQueue[0], &state->vertexQueue[1]);

		// we have seen at least 2 vertices
		state->primitiveState = 2;
	} else {
		// remember that we have seen the first vertex
		state->primitiveState = 1;
	}

	if (state->nextIndex == 3) {
		state->nextIndex = 1;
	}
}

static void EndDrawLineLoop(State * state) {
	if (state->primitiveState == 2) {
		// index of last vertex written
		GLuint prevIndex = 3 - state->nextIndex;

		DrawLine(state, &state->vertexQueue[prevIndex], &state->vertexQueue[0]);
	}
}

/*
** --------------------------------------------------------------------------
** Rendering of Triangles
** --------------------------------------------------------------------------
*/

static void DrawTriangle(State * state, Vertex * a, Vertex * b, Vertex * c) {
	Vertex * vertices[8], * temp[8];
	GLsizei numVertices, index;
	GLboolean cw, backFace;
	
	if (a->geometry.cc & b->geometry.cc & c->geometry.cc) {
		/* outside of frustrum */
		return;
	}
		
	cw = Det(a, b, c) < 0.0f; 
	backFace = cw ^ (state->frontFace == GL_CW);

	if (state->cullFaceEnabled) {
		if ((state->cullMode == GL_FRONT_FACE) ^ backFace) {
			return;
		}
	}
		
	if (cw) {
		vertices[0] = b;
		vertices[1] = a;
	} else {
		vertices[0] = a;
		vertices[1] = b;
	}
	
	vertices[2] = c;
	
	numVertices = Clip(state, vertices, temp, 3);
	
	if (numVertices >= 3) {
		RasterVertex ra, rb;
		
		ProjectVertexToWindowCoords(state, vertices[0], &ra);
		ProjectVertexToWindowCoords(state, vertices[1], &rb);
		
		for (index = 2; index < numVertices; ++index) {
			RasterVertex rc;
			ProjectVertexToWindowCoords(state, vertices[index], &rc);
			GlesRasterTriangle(state, &ra, &rb, &rc, backFace);
		}
	}
}

static void DrawTriangles(State * state, GLuint index) {
	SelectArrayElement(state, index, state->vertexQueue + state->nextIndex++);

	if (state->nextIndex == 3) {
		DrawTriangle(state, state->vertexQueue + 0, state->vertexQueue + 1,
		state->vertexQueue + 2);
		state->nextIndex = 0;
	}
}

static void DrawTriangleStrip(State * state, GLuint index) {
	SelectArrayElement(state, index, state->vertexQueue + state->nextIndex);

	if (state->primitiveState == 3) {
		// even triangle
		GLuint prevIndex = (state->nextIndex - 1) > state->nextIndex ? state->nextIndex + 2 : state->nextIndex - 1;
		GLuint prevIndex2 = (state->nextIndex - 2) > state->nextIndex ? state->nextIndex + 1 : state->nextIndex - 2;
		DrawTriangle(state, state->vertexQueue + prevIndex, state->vertexQueue + prevIndex2, 
					 state->vertexQueue + state->nextIndex);
		state->primitiveState = 2;
	} else if (state->primitiveState == 2) {
		// odd triangle
		GLuint prevIndex = (state->nextIndex - 1) > state->nextIndex ? state->nextIndex + 2 : state->nextIndex - 1;
		GLuint prevIndex2 = (state->nextIndex - 2) > state->nextIndex ? state->nextIndex + 1 : state->nextIndex - 2;
		DrawTriangle(state, state->vertexQueue + prevIndex2, state->vertexQueue + prevIndex, 
					 state->vertexQueue + state->nextIndex);
		state->primitiveState = 3;
	} else {
		// remember seen a vertex
		++state->primitiveState;
	}

	if (++state->nextIndex == 3) {
		state->nextIndex = 0;
	}
}

static void DrawTriangleFan(State * state, GLuint index) {
	SelectArrayElement(state, index, state->vertexQueue + state->nextIndex);

	if (state->primitiveState == 3) {
		// even triangle
		GLuint prevIndex = (state->nextIndex - 1) > state->nextIndex ? state->nextIndex + 2 : state->nextIndex - 1;
		GLuint prevIndex2 = (state->nextIndex - 2) > state->nextIndex ? state->nextIndex + 1 : state->nextIndex - 2;
		DrawTriangle(state, state->vertexQueue + prevIndex, state->vertexQueue + prevIndex2, 
				     state->vertexQueue + state->nextIndex);
		state->primitiveState = 2;
	} else if (state->primitiveState == 2) {
		// odd triangle
		GLuint prevIndex = (state->nextIndex - 1) > state->nextIndex ? state->nextIndex + 2 : state->nextIndex - 1;
		GLuint prevIndex2 = (state->nextIndex - 2) > state->nextIndex ? state->nextIndex + 1 : state->nextIndex - 2;
		DrawTriangle(state, state->vertexQueue + prevIndex2, state->vertexQueue + prevIndex, 
					 state->vertexQueue + state->nextIndex);
		state->primitiveState = 3;
	} else if (state->primitiveState == 1) {
		// remember seen second vertex
		state->primitiveState = 2;
	} else if (state->primitiveState == 0) {
		// remember seen first vertex
		state->primitiveState = 1;
	}

	if (++state->nextIndex == 3) {
		state->nextIndex = 1;
	}
}

/*
** --------------------------------------------------------------------------
** Public API entry points
** --------------------------------------------------------------------------
*/

GL_API void GL_APIENTRY glCullFace (GLenum mode) {
	State * state = GLES_GET_STATE();

	if (GlesValidateEnum(state, mode, CullFaceValues, GLES_ELEMENTSOF(CullFaceValues))) {
		state->cullMode = mode;
	}
}

GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) {

	State * state = GLES_GET_STATE();

	if (count < 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (Begin(state, mode)) {
		while (count-- > 0) {
			(state->drawFunction)(state, first++);
		}
		
		End(state);		
	}
}

GL_API void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices) {

	State * state = GLES_GET_STATE();

	if (count < 0) {
		GlesRecordInvalidValue(state);
		return;
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT) {
		GlesRecordInvalidEnum(state);
		return;
	}

	if (state->elementArrayBuffer) {
		GLubyte * bufferBase =
			(GLubyte *) state->buffers[state->elementArrayBuffer].data;

		if (!bufferBase) {
			GlesRecordInvalidOperation(state);
			return;
		}

		indices = bufferBase + ((const GLubyte *) indices - (const GLubyte *) NULL);
	}


	if (!indices) {
		return;
	}

	if (type == GL_UNSIGNED_BYTE) {
		const GLubyte * ptr = (const GLubyte *) indices;

		if (Begin(state, mode)) {
			while (count-- > 0) {
				state->drawFunction(state, *ptr++);
			}

			End(state);
		}
	} else if (type == GL_UNSIGNED_SHORT) {
		const GLushort * ptr = (const GLushort *) indices;

		if (Begin(state, mode)) {
			while (count-- > 0) {
				state->drawFunction(state, *ptr++);
			}

			End(state);
		}
	} else {
		GlesRecordInvalidEnum(state);
	}
}

GL_API void GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar) {
	State * state = GLES_GET_STATE();

	state->depthRange[0] = zNear = GlesClampf(zNear);
	state->depthRange[1] = zFar = GlesClampf(zFar);

	state->depthOrigin = (zNear + zFar) / 2.0f;
	state->depthScale = ((zFar - zNear) / 2.0f) * (1.0f - FLT_EPSILON);
}

GL_API void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height) {
	State * state = GLES_GET_STATE();

	if (width < 0 || height < 0) {
		GlesRecordInvalidValue(state);
	} else {
		state->viewport.x		= x;
		state->viewport.y		= y;
		state->viewport.width	= width;
		state->viewport.height	= height;

		state->viewportOrigin.x	= x + width / 2.0f;
		state->viewportOrigin.y = y + height / 2.0f;
		state->viewportScale.x = width / 2.0f;
		state->viewportScale.y = height / 2.0f;
	}
}

GL_API void GL_APIENTRY glFrontFace (GLenum mode) {
	State * state = GLES_GET_STATE();

	if (GlesValidateEnum(state, mode, FrontFaceValues, GLES_ELEMENTSOF(FrontFaceValues))) {
		state->frontFace = mode;
	}
}

