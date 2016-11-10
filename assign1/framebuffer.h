// -------------------------------------------------------------------------
// File:    cFrameBuffer.h
// Desc:    basic class for a cFrameBuffer
//
// Author:  Tomas Akenine-Möller
// History: May, 2004 (started)
// -------------------------------------------------------------------------

#ifndef C_FRAMEBUFFER_H
#define C_FRAMEBUFFER_H

#include <math.h>
#include <stdio.h>
#include "misc.h"
#include "buffer.h"

template<class ColorType,class ZType> class cFrameBuffer
{
 public:
	cFrameBuffer();
	cFrameBuffer(int width,int height);
    ~cFrameBuffer();
	void clear(ColorType color,ZType zvalue);
	void clearZBuffer(ZType zvalue);
	void clearColorBuffer(ColorType color);
	void reshape(int width,int height);
	ColorType getColor(int x,int y);
	void setColor(ColorType color,int x,int y);
	void setColor(ColorType color,uint32 color_mask,int x,int y);
	ZType getZ(int x,int y);
	void setZ(ZType color,int x,int y);
	Buffer<ColorType> *getColorBuffer(void);
	Buffer<ZType> *getZBuffer(void);
	void getSize(int &w,int &h);
	int getWidth(void);
	int getHeight(void);
 protected:
	Buffer<ColorType> *mColorBuffer;
	Buffer<ZType>     *mZBuffer;
	int mWidth;
	int mHeight;
};

template<class ColorType, class ZType>
inline cFrameBuffer<ColorType,ZType>::cFrameBuffer()
{
	mColorBuffer=0;
	mZBuffer=0;
	mWidth=0;
	mHeight=0;
}

template<class ColorType, class ZType>
inline cFrameBuffer<ColorType,ZType>::cFrameBuffer(int width,int height)
{
	mColorBuffer=0;
	mZBuffer=0;
	mWidth=0;
	mHeight=0;
	reshape(width,height);
}

template<class ColorType, class ZType>
inline cFrameBuffer<ColorType,ZType>::~cFrameBuffer()
{
	if(mColorBuffer) delete mColorBuffer;	
	if(mZBuffer) delete mZBuffer;	
}

template<class ColorType, class ZType>
inline void cFrameBuffer<ColorType,ZType>::clear(ColorType colorval,ZType zval)
{
	clearColorBuffer(colorval);
	clearZBuffer(zval);
}

template<class ColorType, class ZType>
inline void cFrameBuffer<ColorType,ZType>::clearZBuffer(ZType zval)
{
	ASSERT(mZBuffer);
	mZBuffer->clear(zval);
}

template<class ColorType, class ZType>
inline void cFrameBuffer<ColorType,ZType>::clearColorBuffer(ColorType color)
{
	ASSERT(mColorBuffer);
	mColorBuffer->clear(color);
}

template<class ColorType, class ZType>
inline void cFrameBuffer<ColorType,ZType>::reshape(int width,int height)
{
	if(!mColorBuffer) mColorBuffer=new Buffer<ColorType>(width,height);
	if(!mZBuffer) mZBuffer=new Buffer<ZType>(width,height);
	mWidth=width;
	mHeight=height;
	ASSERT(mWidth>0 && mHeight>0);
	mColorBuffer->reshape(mWidth,mHeight);
	mZBuffer->reshape(mWidth,mHeight);
}


template<class ColorType, class ZType>
inline ColorType cFrameBuffer<ColorType,ZType>::getColor(int x,int y)
{
	ASSERT(mColorBuffer);
	return mColorBuffer->get(x,y);
}

template<class ColorType, class ZType>
inline void cFrameBuffer<ColorType,ZType>::setColor(ColorType color,int x,int y)
{
	ASSERT(mColorBuffer);
	mColorBuffer->set(color,x,y);
}

template<class ColorType, class ZType>
inline void cFrameBuffer<ColorType,ZType>::setColor(ColorType color,uint32 colormask,int x,int y)
{
	// this function implements a color mask
	ASSERT(mColorBuffer);
	uint32 color_fb=(uint32)mColorBuffer->get(x,y);
	color_fb=color_fb & (~colormask);
	mColorBuffer->set(color_fb | (color & colormask),x,y);
}


template<class ColorType, class ZType>
inline ZType cFrameBuffer<ColorType,ZType>::getZ(int x,int y)
{
	ASSERT(mZBuffer);
	return mZBuffer->get(x,y);
}

template<class ColorType, class ZType>
inline void cFrameBuffer<ColorType,ZType>::setZ(ZType zval,int x,int y)
{
	ASSERT(mZBuffer);
	mZBuffer->set(zval,x,y);
}


template<class ColorType, class ZType>
inline Buffer<ColorType> *cFrameBuffer<ColorType,ZType>::getColorBuffer(void)
{
	ASSERT(mColorBuffer);	
	return mColorBuffer;
}

template<class ColorType, class ZType>
inline Buffer<ZType> *cFrameBuffer<ColorType,ZType>::getZBuffer(void)
{
	ASSERT(mZBuffer);	
	return mZBuffer;
}

template<class ColorType, class ZType>
inline void cFrameBuffer<ColorType,ZType>::getSize(int &w,int &h)
{
	w=mWidth;
	h=mHeight;
}

template<class ColorType, class ZType>
inline int cFrameBuffer<ColorType,ZType>::getWidth(void)
{
	return mWidth;
}

template<class ColorType, class ZType>
inline int cFrameBuffer<ColorType,ZType>::getHeight(void)
{
	return mHeight;
}

#endif
