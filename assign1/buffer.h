// -------------------------------------------------------------------------
// File:    buffer.h
// Desc:    a buffer template
//
// Author:  Tomas Akenine-Möller
// History: 2002 (started)
// -------------------------------------------------------------------------

#ifndef BUFFER_H
#define BUFFER_H

#include <math.h>
#include <stdio.h>
#include "misc.h"

template<class BufferType> class Buffer
{
 public:
	Buffer();
	Buffer(int width,int height);
    ~Buffer();
	void			reshape(int width,int height);
	BufferType		get(int x,int y);
	BufferType*		getPointer(int x,int y);
	void			set(BufferType z,int x,int y);
	BufferType*		getBuffer(int &width,int &height);
	void			getSize(int &w,int &h);
	int				getWidth(void);
	int				getHeight(void);
protected:
	BufferType *mBuffer;
	int mWidth;
	int mHeight;
};

template<class BufferType>
inline Buffer<BufferType>::Buffer()
{
	mBuffer=0;
	mWidth=0;
	mHeight=0;
}

template<class BufferType>
inline Buffer<BufferType>::Buffer(int width,int height)
{
	mBuffer=0;
	mWidth=0;
	mHeight=0;
	reshape(width,height);
}

template<class BufferType>
inline Buffer<BufferType>::~Buffer()
{
	if(mBuffer) delete [] mBuffer;
}

template<class BufferType>
inline void Buffer<BufferType>::reshape(int width,int height)
{
	mWidth=width;
	mHeight=height;
	ASSERT(mWidth>0 && mHeight>0);
	if(!mBuffer) delete [] mBuffer;
	mBuffer=new BufferType[mWidth*mHeight];
	ASSERT(mBuffer);
}

template<class BufferType>
inline BufferType Buffer<BufferType>::get(int x,int y)
{
	ASSERT(x>=0 && x<mWidth);
	ASSERT(y>=0 && y<mHeight);
	ASSERT(mBuffer);
	return mBuffer[mWidth*y+x];
}

template<class BufferType>
inline BufferType* Buffer<BufferType>::getPointer(int x,int y)
{
	ASSERT(x>=0 && x<mWidth);
	ASSERT(y>=0 && y<mHeight);
	ASSERT(mBuffer);
	return &mBuffer[mWidth*y+x];
}

template<class BufferType>
inline void Buffer<BufferType>::set(BufferType z,int x,int y)
{
	ASSERT(x>=0 && x<mWidth);
	ASSERT(y>=0 && y<mHeight);
	ASSERT(mBuffer);
	mBuffer[mWidth*y+x]=z;
}

template<class BufferType>
inline BufferType *Buffer<BufferType>::getBuffer(int &width,int &height)
{
	ASSERT(mBuffer);
	width=mWidth;
	height=mHeight;
	return mBuffer;
}

template<class BufferType>
inline void Buffer<BufferType>::getSize(int &w,int &h)
{
	w=mWidth;
	h=mHeight;
}

template<class BufferType>
inline int Buffer<BufferType>::getWidth(void)
{
	return mWidth;
}

template<class BufferType>
inline int Buffer<BufferType>::getHeight(void)
{
	return mHeight;
}


#endif
