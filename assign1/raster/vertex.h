// -------------------------------------------------------------------------
// File:    cVertex
// Desc:    a vertex that is used as input to the rasterizer
//			a vertex can contain position, color, textur coords, etc
//
// Author:  Tomas Akenine-Möller
// History: February, 2005 (started)
// -------------------------------------------------------------------------

#ifndef C_VERTEX_H
#define C_VERTEX_H

#include "vecmath.h"

// Mappings taken from the opengl ARB_vertex_program spec
#define ATTRIBUTE_POSITION			0
#define ATTRIBUTE_WEIGHTS			1
#define ATTRIBUTE_NORMAL			2
#define ATTRIBUTE_PRIMARY_COLOR		3
#define ATTRIBUTE_SECONDARY_COLOR	4
#define ATTRIBUTE_FOG_COLOR			5
#define ATTRIBUTE_TEXTURE_COORD0	8
#define ATTRIBUTE_TEXTURE_COORD1	9
#define ATTRIBUTE_TEXTURE_COORD2	10
#define ATTRIBUTE_TEXTURE_COORD3	11
#define ATTRIBUTE_TEXTURE_COORD4	12
#define ATTRIBUTE_TEXTURE_COORD5	13
#define ATTRIBUTE_TEXTURE_COORD6	14
#define ATTRIBUTE_TEXTURE_COORD7	15
#define MAX_VERTEX_ATTRIBS			16

class cVertex
{
public:
	Vec4f		mAttributes[MAX_VERTEX_ATTRIBS];
};

class cFragment {
public:
	Vec4f		mAttributes[MAX_VERTEX_ATTRIBS];
	Vec4f		mDdx[MAX_VERTEX_ATTRIBS], mDdy[MAX_VERTEX_ATTRIBS];
};

#endif