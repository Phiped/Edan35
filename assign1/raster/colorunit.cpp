// -------------------------------------------------------------------------
// File:    cColorUnit
// Desc:    the most basic color unit you can have; nothing fancy at all.
//			the purpose is to have a simple interface to reading and writing
//			color in a self-contained class.
//
// Author:  Tomas Akenine-Möller
// History: March, 2005 (started)
// -------------------------------------------------------------------------

#include "statistics.h"
#include "colorunit.h"
#include "glstate.h"
#include <memory.h>

#include "mmgr/mmgr.h"

cSimpleColorUnit::cSimpleColorUnit()
{
	mColorBuffer= new Buffer<uint32>();
}

cSimpleColorUnit::~cSimpleColorUnit()
{
}

void cSimpleColorUnit::writeColor(uint32 color,int pixel_x,int pixel_y)
{
	ASSERT(mColorBuffer);
	statistics::add(STATS_COLOR_BUFFER_NUM_WRITES);
	statistics::add(STATS_COLOR_BUFFER_WRITE_BW,32);		// assume 32 bits color buffer

	uint32 colorMask =	0xff000000*(glState.mWriteEnableAlpha ? 1 : 0) +
						0x00ff0000*(glState.mWriteEnableBlue  ? 1 : 0) +
						0x0000ff00*(glState.mWriteEnableGreen ? 1 : 0) +
						0x000000ff*(glState.mWriteEnableRed   ? 1 : 0);

	if(colorMask==0xffffffff) mColorBuffer->set(color,pixel_x,pixel_y);
	else
	{
		// use color mask...
		uint32 color_fb=(uint32)mColorBuffer->get(pixel_x,pixel_y);
		color_fb=color_fb & (~colorMask);
		mColorBuffer->set(color_fb | (color & colorMask),pixel_x,pixel_y);
	}
}

uint32 cSimpleColorUnit::readColor(int pixel_x,int pixel_y)
{
	ASSERT(mColorBuffer);
	statistics::add(STATS_COLOR_BUFFER_NUM_READS);
	statistics::add(STATS_COLOR_BUFFER_READ_BW,32);		// assume 32 bits color buffer
	return mColorBuffer->get(pixel_x,pixel_y);
}

void cSimpleColorUnit::clearBuffer(uint32 clear_color)
{
	ASSERT(mColorBuffer);
	int w=mColorBuffer->getWidth();
	int h=mColorBuffer->getHeight();
	for(int y=0;y<h;y++)
	{
		for(int x=0;x<w;x++)
		{
			mColorBuffer->set(clear_color,x,y);
		}
	}
	statistics::add(STATS_COLOR_CLEAR_BW,w*h*32);		// assume 32 bits color buffer
}

void cSimpleColorUnit::getBuffer(uint32 *buffer) {
	int w,h;
	memcpy(buffer, mColorBuffer->getBuffer(w,h), mColorBuffer->getWidth()*mColorBuffer->getHeight()*sizeof(uint32));
}