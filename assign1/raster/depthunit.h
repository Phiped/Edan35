// -------------------------------------------------------------------------
// File:    cDepthUnit
// Desc:    the most basic depth unit you can have; nothing fancy at all.
//
// Author:  Tomas Akenine-Möller
// History: February, 2005 (started)
// -------------------------------------------------------------------------

#ifndef C_DEPTH_UNIT_H
#define C_DEPTH_UNIT_H

#include "buffer.h"
#include "cache.h"
#include <vector>

#define DEPTH_TILE_SIZE 8

class cDepthUnit
{
public:
	virtual ~cDepthUnit() {}

	virtual bool	depthTestAndUpdate		(uint32 z,int pixel_x,int pixel_y) = 0;
	virtual bool	cullTile				(uint32 zmin, uint32 zmax, int pixel_x,int pixel_y) = 0;
	virtual void	clearBuffer				(uint32 clead_value) = 0;
	virtual int		getWidth				(void) = 0;
	virtual int		getHeight				(void) = 0;
	virtual void	reshape					(int w,int h) = 0;
};

class cSimpleDepthUnit : public cDepthUnit
{
public:
					cSimpleDepthUnit		();
	virtual			~cSimpleDepthUnit		();
	virtual bool	depthTestAndUpdate		(uint32 z,int pixel_x,int pixel_y);
	virtual bool	cullTile				(uint32 zmin, uint32 zmax, int pixel_x,int pixel_y);
	virtual void	clearBuffer				(uint32 clead_value);
	
	virtual int		getWidth				(void) {ASSERT(mDepthBuffer); return mDepthBuffer->getWidth();}
	virtual int		getHeight				(void) {ASSERT(mDepthBuffer); return mDepthBuffer->getHeight();}
	virtual void	reshape					(int w,int h) { ASSERT(mDepthBuffer); mDepthBuffer->reshape(w,h); }
	uint32*			getBuffer				(int &w,int &h) {ASSERT(mDepthBuffer); return mDepthBuffer->getBuffer(w,h);}
protected:
	Buffer<uint32> *mDepthBuffer;
};

#endif