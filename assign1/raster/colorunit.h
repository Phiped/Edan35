// -------------------------------------------------------------------------
// File:    cColorUnit
// Desc:    the most basic color unit you can have; nothing fancy at all.
//			the purpose is to have a simple interface to reading and writing
//			color in a self-contained class.
//
// Author:  Tomas Akenine-Möller
// History: March, 2005 (started)
// -------------------------------------------------------------------------

#ifndef C_COLOR_UNIT_H
#define C_COLOR_UNIT_H

#include "buffer.h"
#include "cache.h"
#include <vector>

#define COLOR_TILE_SIZE 8

class cColorUnit 
{
public:
	virtual			~cColorUnit				() {}
	virtual uint32	readColor				(int pixel_x, int pixel_y) = 0;
	virtual void	writeColor				(uint32 color,int pixel_x,int pixel_y) = 0;
	virtual void	clearBuffer				(uint32 clear_color) = 0;
	virtual int		getWidth				(void) = 0;
	virtual int		getHeight				(void) = 0;
	virtual void	reshape					(int w,int h) = 0;
	virtual void	getBuffer				(uint32 *buffer) = 0;
};

class cSimpleColorUnit : public cColorUnit
{
public:
					cSimpleColorUnit		();
	virtual			~cSimpleColorUnit		();
	virtual uint32	readColor				(int pixel_x,int pixel_y);
	virtual void	writeColor				(uint32 color,int pixel_x,int pixel_y);
	virtual void	clearBuffer				(uint32 clear_color);
	virtual int		getWidth				(void) {ASSERT(mColorBuffer); return mColorBuffer->getWidth();}
	virtual int		getHeight				(void) {ASSERT(mColorBuffer); return mColorBuffer->getHeight();}
	virtual void	reshape					(int w,int h) {ASSERT(mColorBuffer); return mColorBuffer->reshape(w,h);}
	virtual void	getBuffer				(uint32 *buffer);
protected:
	Buffer<uint32> *mColorBuffer;
};

#endif