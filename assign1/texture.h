// -------------------------------------------------------------------------
// File:    cFrameBuffer.h
// Desc:    basic class for a cFrameBuffer
//
// Author:  Tomas Akenine-Möller
// History: May, 2004 (started)
// -------------------------------------------------------------------------

#ifndef C_TEXTURE_H
#define C_TEXTURE_H

#include <vector>
#include "GL/glut.h"

#include "cache.h"
#include "misc.h"
#include "vecmath.h"
#include "buffer.h"

class cTexture {
public:
	cTexture(GLuint textureId) {
		mTextureId = textureId;
		mMagFilter = GL_LINEAR; 
		mMinFilter = GL_LINEAR_MIPMAP_NEAREST;
		mWrapS = GL_REPEAT;
		mWrapT = GL_REPEAT;
	}
	virtual ~cTexture() {}

	/**	\brief Get the number of mip-map levels in this texture */
	virtual int		getNumMipmaps	() = 0;
	virtual void	setData			(int mipLevel, int width, int height, const void *data) = 0;
	virtual void	getSize			(int mipLevel, int &width, int &height) = 0;
	virtual Color4r	readTexel		(cTextureCache *cache, int mipLevel, int u, int v) = 0;
	
	GLuint	mTextureId;
	GLenum	mMagFilter, mMinFilter;
	GLenum  mWrapS, mWrapT;
protected:
};

class cRGBATexture : public cTexture {
public:
	cRGBATexture(GLuint textureId);
	~cRGBATexture();

	virtual int		getNumMipmaps	();
	virtual void	setData			(int mipLevel, int width, int height, const void *data);
	virtual void	getSize			(int mipLevel, int &width, int &height);
	virtual Color4r	readTexel		(cTextureCache *cache, int mipLevel, int u, int v);
private:
	std::vector<Buffer<uint32> *> mMipLevels;
};

#endif