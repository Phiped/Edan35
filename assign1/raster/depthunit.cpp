// -------------------------------------------------------------------------
// File:    cDepthUnit
// Desc:    the most basic depth unit you can have; nothing fancy at all.
//
// Author:  Tomas Akenine-Möller
// History: February, 2005 (started)
// -------------------------------------------------------------------------

#include "GL/glut.h"
#include "statistics.h"
#include "depthunit.h"
#include "glstate.h"

#include "mmgr/mmgr.h"

cSimpleDepthUnit::cSimpleDepthUnit()
{
	mDepthBuffer = new Buffer<uint32>();
}

cSimpleDepthUnit::~cSimpleDepthUnit()
{
}

bool cSimpleDepthUnit::cullTile(uint32 zmin, uint32 zmax, int pixel_x, int pixel_y) 
{
	return false;
}

bool cSimpleDepthUnit::depthTestAndUpdate(uint32 z,int pixel_x,int pixel_y)
{
	ASSERT(mDepthBuffer);
	if(!glState.mDepthTestEnabled) return true;

	statistics::add(STATS_DEPTH_BUFFER_READ_BW,24);		// assume depth=24 bits. TODO!
	uint32 zFromBuffer=mDepthBuffer->get(pixel_x,pixel_y);
	
	bool zpass = false;
	switch(glState.mDepthFunc)
	{
	case GL_NEVER:
		zpass=false;	
		break;
	case GL_LESS:
		zpass= z<zFromBuffer;	
		break;
	case GL_EQUAL:
		zpass= z==zFromBuffer;
		break;
	case GL_LEQUAL:
		zpass= z<=zFromBuffer;
		break;
	case GL_GREATER:
		zpass= z>zFromBuffer;
		break;
	case GL_NOTEQUAL:
		zpass= z!=zFromBuffer;
		break;
	case GL_GEQUAL:
		zpass= z>=zFromBuffer;
		break;
	case GL_ALWAYS:
		zpass=true;
		break;
	default:
		ASSERT(0);
		break;				// should never happen
	}
	
	if (zpass && glState.mWriteEnableDepth) {
		statistics::add(STATS_DEPTH_BUFFER_NUM_WRITES);
		statistics::add(STATS_DEPTH_BUFFER_WRITE_BW,24);		// assume depth=24 bits
		mDepthBuffer->set(z,pixel_x,pixel_y);
	}

	return zpass;
}

void cSimpleDepthUnit::clearBuffer(uint32 clear_color)
{
	ASSERT(mDepthBuffer);
	int w=mDepthBuffer->getWidth();
	int h=mDepthBuffer->getHeight();
	for(int y=0;y<h;y++)
	{
		for(int x=0;x<w;x++)
		{
			mDepthBuffer->set(clear_color,x,y);
		}
	}
	statistics::add(STATS_DEPTH_CLEAR_BW,w*h*24);		// assume 24 bits color buffer
}