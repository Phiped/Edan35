// -------------------------------------------------------------------------
// File:    cTextureUnit
// Desc:    the most basic texture fetch unit you can have; nothing fancy at all.
//			the purpose is to have a simple interface that can be extended
//
// Author:  Tomas Akenine-Möller
// History: May, 2005 (started)
// -------------------------------------------------------------------------

#include "textureunit.h"
#include "glstate.h"

#include "mmgr/mmgr.h"

cTextureUnit::cTextureUnit(cTextureCache *textureCache)
{
	mEnabled = false;
	mActiveTextureId = 0;
	mActiveTexture = NULL;
	mTextureCache = textureCache;
}

cTextureUnit::~cTextureUnit()
{
}

/**
 *	\brief Sets the texture currently bound to this texture unit.
 *	\param textureId	OpenGL style texture id 
 */
void cTextureUnit::setTexture(GLuint textureId) {
	mActiveTextureId = textureId;
	mActiveTexture = glState.mTextures[ mActiveTextureId ];
}

/**
 *	\brief Get the OpenGL-style texture id of the texture currently bound on this texture unit
 */
GLuint cTextureUnit::getTexture() {	return mActiveTextureId; }

/** \brief Enable the current texture unit */
void cTextureUnit::enable() { mEnabled = true; }

/** \brief Disable the current texture unit */
void cTextureUnit::disable() { mEnabled = false; }

/** \brief Determine if this textureunit is enabled or disabled */
bool cTextureUnit::isEnabled() { return mEnabled; }

/**
 *	\brief Perform a filtered texture lookup at the given position. 
 *	\param tc		Texture coordinate for the point to access
 *	\param ddx		Derivative in screen space x-direction (used for mip-map computation)
 *	\param ddy		Derivative in screen space y-direction (used for mip-map computation)
 */
Color4r cTextureUnit::readTexel(Vec2f tc, Vec2f ddx, Vec2f ddy)
{
	int w,h;
	bool magnify = true;

	// If texture isnt properly configured return white (like ogl does)
	if (!mEnabled || mActiveTextureId == 0 || mActiveTexture == NULL) 
		return Color4r(1.0,1.0,1.0,1.0);

	// Get the texture (base level) dimensions
	mActiveTexture->getSize(0,w,h);

	// The c constant is specified in glspec1.5, 3.8.9 and is slightly tweaked if LINEAR / NEAREST is mixed
	float c = mActiveTexture->mMagFilter == GL_LINEAR 
		&& (mActiveTexture->mMagFilter == GL_NEAREST_MIPMAP_NEAREST 
		|| mActiveTexture->mMagFilter == GL_NEAREST_MIPMAP_LINEAR ) ? 0.5f : 0.0f;

	float rho = MAX2( (ddx*w).length(), (ddy*h).length() );		// See glspec1.5, 3.8.8 Texture Minification
	float lambda = log(rho) / log(2.0f);						// See glspec1.5, 3.8.8 Texture Minification, skipped bias

	if (lambda < c) {
		// Texture magnification

		if (mActiveTexture->mMagFilter == GL_NEAREST)
			return nearest(0,tc);		
		else if (mActiveTexture->mMagFilter == GL_LINEAR) 
			return linear(0,tc);
	}
	else {
		// Texture minification
		if (mActiveTexture->mMinFilter == GL_NEAREST)
			return nearest(0,tc);		
		else if (mActiveTexture->mMinFilter == GL_LINEAR) 
			return linear(0,tc);
		else if (mActiveTexture->mMinFilter == GL_NEAREST_MIPMAP_NEAREST) {
			int mipLevel = MAX2(MIN2(((int)ceil( lambda + 0.5f )) - 1, mActiveTexture->getNumMipmaps() - 1), 0);		
			return nearest(mipLevel,tc);
		}
		else if (mActiveTexture->mMinFilter == GL_LINEAR_MIPMAP_NEAREST) {
			int mipLevel = MAX2(MIN2(((int)ceil( lambda + 0.5f )) - 1, mActiveTexture->getNumMipmaps() - 1), 0);
			return linear(mipLevel,tc);
		}
		else if (mActiveTexture->mMinFilter == GL_NEAREST_MIPMAP_LINEAR) {
			int mipLevel = MAX2(MIN2((int)lambda, mActiveTexture->getNumMipmaps() - 1), 0);

			if (mipLevel == mActiveTexture->getNumMipmaps() - 1)
				return nearest(mipLevel, tc);

			float t = lambda - (float)mipLevel;

			Color4r col1 = nearest(mipLevel, tc);
			Color4r col2 = nearest(mipLevel+1, tc);
			return col1.lerp(1.0f - t, col2);
		}
		else if (mActiveTexture->mMinFilter == GL_LINEAR_MIPMAP_LINEAR) {
			int mipLevel = MAX2(MIN2((int)lambda, mActiveTexture->getNumMipmaps() - 1), 0);

			if (mipLevel == mActiveTexture->getNumMipmaps() - 1)
				return linear(mipLevel, tc);

			float t = lambda - (float)mipLevel;

			Color4r col1 = linear(mipLevel, tc);
			Color4r col2 = linear(mipLevel+1, tc);
			return col1.lerp(1.0f - t, col2);
		}
	}

	ASSERT(false);
	return Color4r(1.0,1.0,1.0,1.0);
}

/**
 *	\brief Looks up a texture value from the given mip-level with no interpolation
 *	\param mipLevel	Integer specifying the mip-map level
 *	\param tc		Vec2r holding normalized texture coordinates
 *	\return			The filtered texture color
 */
Color4r cTextureUnit::nearest(int mipLevel, Vec2r tc) 
{
	int w,h;
	mActiveTexture->getSize(mipLevel,w,h);    
	return mActiveTexture->readTexel(mTextureCache, mipLevel, wrap(mActiveTexture->mWrapS, w, (int)(tc.x*w)), wrap(mActiveTexture->mWrapS, h, (int)(tc.y*h)) );
}

/**
 *	\brief Looks up a texture value from the given mip-level with bi-linear interpolation
 *	\param mipLevel	Integer specifying the mip-map level
 *	\param tc		Vec2r holding normalized texture coordinates
 *	\return			The filtered texture color
 */
Color4r cTextureUnit::linear(int mipLevel, Vec2r tc) 
{
	int w,h;
	mActiveTexture->getSize(mipLevel,w,h);    
	float texture_s = tc.x*w - 0.5;
	float texture_t = tc.y*h - 0.5;			// Ogl specifies the texel center in (0.5, 0.5)
	int int_s = (int)floor(texture_s);
	int int_t = (int)floor(texture_t);
	float frac_s = texture_s - (float)int_s;
	float frac_t = texture_t - (float)int_t;

	// Perform 4 texture lookups
	Color4r col_11 = mActiveTexture->readTexel(mTextureCache, mipLevel, wrap(mActiveTexture->mWrapS, w, int_s), wrap(mActiveTexture->mWrapT, h, int_t));
	Color4r col_21 = mActiveTexture->readTexel(mTextureCache, mipLevel, wrap(mActiveTexture->mWrapS, w, int_s+1), wrap(mActiveTexture->mWrapT, h, int_t));
	Color4r col_12 = mActiveTexture->readTexel(mTextureCache, mipLevel, wrap(mActiveTexture->mWrapS, w, int_s), wrap(mActiveTexture->mWrapT, h, int_t+1));
	Color4r col_22 = mActiveTexture->readTexel(mTextureCache, mipLevel, wrap(mActiveTexture->mWrapS, w, int_s+1), wrap(mActiveTexture->mWrapT, h, int_t+1));

	// Bi-linearly blend the color values
	Color4r col_y1 = col_11.lerp(1.0f - frac_s, col_21);
	Color4r col_y2 = col_12.lerp(1.0f - frac_s, col_22);
	return col_y1.lerp(1.0f - frac_t, col_y2);
}
/**
 *	\brief Wraps an integer texture coordinate using the specified wrapping mode (GL_CLAMP or GL_REPEAT)
 *	\param mode		Wrapping mode
 *	\param dim		Dimension of the texture (width or height)
 *	\param tc		Integer texture coordinate to be wrapped
 */
int cTextureUnit::wrap(GLenum mode,int dim,int tc) {
	if (mode == GL_CLAMP)
		return MAX2(MIN2(tc,dim-1),0);
	
	ASSERT(mode == GL_REPEAT);
	return tc >= 0 ? tc % dim : (dim + (tc%dim))%dim;
}