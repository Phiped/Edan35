// -------------------------------------------------------------------------
// File:    cTextureUnit
// Desc:    the most basic texture fetch unit you can have; nothing fancy at all.
//			the purpose is to have a simple interface that can be extended
//
// Author:  Tomas Akenine-Möller
// History: May, 2005 (started)
// -------------------------------------------------------------------------

#ifndef C_TEXTURE_UNIT_H
#define C_TEXTURE_UNIT_H

#include "GL/glut.h"

#include "cache.h"
#include "misc.h"
#include "vecmath.h"
#include "buffer.h"

class	cTexture;

class cTextureUnit
{
public:
						cTextureUnit		(cTextureCache *textureCache);
	virtual				~cTextureUnit		();
	
	virtual Color4r		readTexel			(Vec2f tc, Vec2f ddx, Vec2f ddy);
	
	virtual void		setTexture			(GLuint textureId);
	virtual GLuint		getTexture			();
	virtual void		enable				();
	virtual void		disable				();
	virtual bool		isEnabled			();
protected:
	cTextureCache		*mTextureCache;
	bool				mEnabled;
	GLuint				mActiveTextureId;
	cTexture			*mActiveTexture;

	// Some helper functions
	int					wrap				(GLenum mode, int dim, int tc);
	Color4r				nearest				(int mipLevel, Vec2r tc);
	Color4r				linear				(int mipLevel, Vec2r tc);
};

#endif