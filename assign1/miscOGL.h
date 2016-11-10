// -------------------------------------------------------------------------
// File:    miscOGL
// Desc:    misc OpenGL-related calls
//
// Author:  Tomas Akenine-Möller
// History: February, 2005 (started)
// -------------------------------------------------------------------------

#ifndef MISC_OGL_H
#define MISC_OGL_H

#include "vecmath.h"

inline void fGetModel2ScreenMatrix(Mtx4f &m)
{
	Mtx4f mv,pr,vp;  // modelview, projection, viewport

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);	

	// for now, we assume that the viewport occupies the entire window
	float whalf=viewport[2]*0.5f;	// get width from viewport
	float hhalf=viewport[3]*0.5f;	// get height from viewport

	vp.set(whalf, 0,     0,   whalf,
		0,     hhalf, 0,   hhalf,
		0,     0,     0.5, 0.5,
		0,     0,     0,   1);

	glGetFloatv(GL_MODELVIEW_MATRIX,mv.array);
	glGetFloatv(GL_PROJECTION_MATRIX,pr.array);
	m=vp*(pr*mv);
}

inline void fGetViewportMatrix(Mtx4f &vp) {
	GLint viewport[4];
	GLfloat dr[2];

	glGetIntegerv(GL_VIEWPORT, viewport);	
	glGetFloatv(GL_DEPTH_RANGE, dr);

	// for now, we assume that the viewport occupies the entire window
	float whalf=viewport[2]*0.5f;	// get width from viewport
	float hhalf=viewport[3]*0.5f;	// get height from viewport
	float zhalf=(dr[1]-dr[0])*0.5f;	// get half z interval from depth range
	float zo=(dr[1]+dr[0])*0.5f;	// get z origin from depth range

	vp.set(whalf, 0,     0,   whalf,
		0,     hhalf, 0,   hhalf,
		0,     0,     zhalf, zo,
		0,     0,     0,   1);

}

inline void fGetModelviewProjection(Mtx4f &m)
{
	Mtx4f mv,pr;  // modelview, projection, viewport
	glGetFloatv(GL_MODELVIEW_MATRIX,mv.array);
	glGetFloatv(GL_PROJECTION_MATRIX,pr.array);
	m = pr*mv;
}

inline void fGetTextureMatrix(Mtx4f &m)
{
	glGetFloatv(GL_TEXTURE_MATRIX,m.array);
}

#endif