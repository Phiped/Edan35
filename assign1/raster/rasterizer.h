// -------------------------------------------------------------------------
// File:    rasterizer.h
// Desc:    basic rasterizer class. Inherit from this one, and extend!
//			
// Author:  Tomas Akenine-Möller
// History: September, 2004 (started)
// -------------------------------------------------------------------------

#ifndef C_RASTERIZER_H
#define C_RASTERIZER_H

#include "edgefunc.h"
#include "vecmath.h"
#include "buffer.h"
#include "vertex.h"
#include "textureunit.h"

class cRendererAPI;
class cDepthUnit;
class cColorUnit;

class cRasterizer
{
public:
					cRasterizer					(cRendererAPI *api,cDepthUnit *du,cColorUnit *cu,cTextureUnit *tu);
	void			setResolution				(int width,int height);
	cColorUnit*		getColorUnit				(void)	{ASSERT(mColorUnit); return mColorUnit;}
	cDepthUnit*		getDepthUnit				(void)	{ASSERT(mDepthUnit); return mDepthUnit;}
	virtual void	rasterizeTriangle			(void);
	virtual bool	setup						(const cVertex &v0,const cVertex &v1,const cVertex &v2);
	virtual bool	perFragment					(int xi,int yi);

	Vec2f			computeBarycentricCoords	(void);
	Vec2f			computePerspectiveBaryCoords(void);
	void			computePerspectiveBaryDerivatives(int x,int y, Vec2f &ddx, Vec2f &ddy);	
	Vec4f			interpolateAttribute		(GLuint idx, Vec2f bary);
	Vec4f			attributeDerivative			(GLuint idx, Vec2f baryDerivative);

	uint32			interpolateZ				(Vec2f bary);


	virtual	char*   getClassName				(void) {return "cRasterizer";}
	virtual const uint16 getTileWidth			(void) const {return 1;}
	virtual const uint16 getTileHeight			(void) const {return 1;}
protected:
	bool			inside						(int x,int y);	
	float			mOneOverTriangleArea;
	cEdgeFunc		mEdgeFuncs[3];
	int32			mSignedDist[3];				// in [n,8] format. inside() computes these; later used by computePerspectiveBaryCoords and computeInterpolatedZ
	Vec2i			mBBox2DMin;
	Vec2i			mBBox2DMax;
	cVertex         mVertices[3];

	cRendererAPI	*mAPI;
	cDepthUnit		*mDepthUnit;
	cColorUnit		*mColorUnit;
	cTextureUnit    *mTextureUnit;

	Color4f			defaultFragmentProgram(const cFragment &frag);
};

#endif
