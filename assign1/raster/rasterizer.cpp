// -------------------------------------------------------------------------
// File:    rasterizer
// Desc:    basic rasterizer class. Inherit from this one, and extend!
//			
// Author:  Tomas Akenine-Möller
// History: September, 2004 (started)
// -------------------------------------------------------------------------

#include "rasterizer.h"
#include "vecmath.h"
#include "misc.h"
#include "rendererAPI.h"
#include "depthunit.h"
#include "colorunit.h"
#include "statistics.h"
#include "miscogl.h"

#include "mmgr/mmgr.h"

cRasterizer::cRasterizer(cRendererAPI *api,cDepthUnit *du,cColorUnit *cu,cTextureUnit *tu)
{	
	ASSERT(api);
	ASSERT(du);
	ASSERT(cu);
	ASSERT(tu);
	mAPI=api;
	mDepthUnit=du;
	mColorUnit=cu;
	mTextureUnit=tu;
}


bool cRasterizer::setup(const cVertex &v0,const cVertex &v1,const cVertex &v2)
{
	mVertices[0]=v0;
	mVertices[1]=v1;
	mVertices[2]=v2;

	// Perform final "viewport transformation" (see glspec 2.11.1) and round the coordinates to fixed point values (keep clip-space w for interpolation)
	Mtx4f vp;
	fGetViewportMatrix(vp);

	for (int i = 0; i < 3; i++) {
		Vec4f device = mVertices[i].mAttributes[ATTRIBUTE_POSITION] * (1.0f / mVertices[i].mAttributes[ATTRIBUTE_POSITION].w);	//device coords
		Vec4f view = vp*device;
		mVertices[i].mAttributes[ATTRIBUTE_POSITION].x = view.x;
		mVertices[i].mAttributes[ATTRIBUTE_POSITION].y = view.y;
		mVertices[i].mAttributes[ATTRIBUTE_POSITION].z = view.z;
		// the mVertices[i].mAttributes[ATTRIBUTE_POSITION].w is kept for later use...
	}

	int x[3],y[3];
	int signed_area;

	// fetch position coordinates (only x & y) for the three vertices of the triangle
	x[0]=floatToFixed(2, mVertices[0].mAttributes[ATTRIBUTE_POSITION].x);		// make them be on [n,0] fixedpoint format
	x[1]=floatToFixed(2, mVertices[1].mAttributes[ATTRIBUTE_POSITION].x);		
	x[2]=floatToFixed(2, mVertices[2].mAttributes[ATTRIBUTE_POSITION].x);		
	y[0]=floatToFixed(2, mVertices[0].mAttributes[ATTRIBUTE_POSITION].y);
	y[1]=floatToFixed(2, mVertices[1].mAttributes[ATTRIBUTE_POSITION].y);
	y[2]=floatToFixed(2, mVertices[2].mAttributes[ATTRIBUTE_POSITION].y);

	//---- compute integer bounding box of triangle
	// min corner
	mBBox2DMin.x=MIN3(x[0],x[1],x[2]);
	mBBox2DMin.y=MIN3(y[0],y[1],y[2]);
	mBBox2DMin.x=MAX2(mBBox2DMin.x,0);							// prevent negative coords
	mBBox2DMin.y=MAX2(mBBox2DMin.y,0);
	// max corner
	mBBox2DMax.x=MAX3(x[0],x[1],x[2]);
	mBBox2DMax.y=MAX3(y[0],y[1],y[2]);
	mBBox2DMax.x=MIN2(mBBox2DMax.x+1,(mColorUnit->getWidth()-1)<<2);	// prevent too large coords
	mBBox2DMax.y=MIN2(mBBox2DMax.y+1,(mColorUnit->getHeight()-1)<<2);
	//---- end of BBox computation

	signed_area = (x[1]-x[0]) * (y[2]-y[0]) - (x[2]-x[0]) * (y[1]-y[0]);
	//signed_area = floatToFixed(2, signed_area);
	if(signed_area==0) return false;									 // zero-area triangle? if so, skip it.

	//---- setup edge functions: note the order; an edge function with number n is derived from vertices not including n
	mEdgeFuncs[2].setup(signed_area,x[0],y[0],x[1],y[1]);
	mEdgeFuncs[0].setup(signed_area,x[1],y[1],x[2],y[2]);
	mEdgeFuncs[1].setup(signed_area,x[2],y[2],x[0],y[0]);
	
	mOneOverTriangleArea=fabs(1.0f/signed_area);		// needs to be a positive number...

	return true;
}


bool cRasterizer::inside(int x,int y)
{	
	statistics::add(STATS_VISITED_PIXELS);

	// all edge functions must return true in order for cRasterizer::inside() to return true
	bool ef[3];
	for (int i = 0; i < 3; i++) 
		ef[i] = mEdgeFuncs[i].inside(x,y,mSignedDist[i]);

	return ef[0] && ef[1] && ef[2];
}
 
Vec2f cRasterizer::computeBarycentricCoords(void)		// linear barycentric coordinates
{
	float u_bary = mSignedDist[1]*mOneOverTriangleArea;
	float v_bary = mSignedDist[2]*mOneOverTriangleArea;
	return Vec2f(u_bary,v_bary);
}

Vec2f cRasterizer::computePerspectiveBaryCoords(void)
{
	return computeBarycentricCoords();
}

void cRasterizer::computePerspectiveBaryDerivatives(int x,int y, Vec2f &ddx, Vec2f &ddy)
{
	int signedDist[3];
	
	// Save mSignedDist values
	signedDist[0] = mSignedDist[0];
	signedDist[1] = mSignedDist[1];
	signedDist[2] = mSignedDist[2];

	Vec2f bary[3];
	int xx[3] = {x, x+1, x};
	int yy[3] = {y, y, y+1};

	// Evaluate barycentric coords for three pixels 
	for (int i = 0; i < 3; i++) {
		inside(xx[i],yy[i]);
		bary[i] = computePerspectiveBaryCoords();
	}

	// Compute derivatives using finite differences
	ddx = bary[1] - bary[0];
	ddy = bary[2] - bary[0];

	// Restore mSignedDist values
	mSignedDist[0] = signedDist[0];
	mSignedDist[1] = signedDist[1];
	mSignedDist[2] = signedDist[2];
}



Vec4f cRasterizer::interpolateAttribute(GLuint idx, Vec2f bary) {
	return mVertices[0].mAttributes[idx] 
		+ (mVertices[1].mAttributes[idx]-mVertices[0].mAttributes[idx])*bary.x 
		+ (mVertices[2].mAttributes[idx]-mVertices[0].mAttributes[idx])*bary.y;	
}

Vec4f cRasterizer::attributeDerivative(GLuint idx, Vec2f baryDerivative) {
	return (mVertices[1].mAttributes[idx]-mVertices[0].mAttributes[idx])*baryDerivative.x 
		+ (mVertices[2].mAttributes[idx]-mVertices[0].mAttributes[idx])*baryDerivative.y;	
}


uint32 cRasterizer::interpolateZ(Vec2f bary)
{
	float z = mVertices[0].mAttributes[ATTRIBUTE_POSITION].z 
		+ (mVertices[1].mAttributes[ATTRIBUTE_POSITION].z - mVertices[0].mAttributes[ATTRIBUTE_POSITION].z)*bary.x 
		+ (mVertices[2].mAttributes[ATTRIBUTE_POSITION].z - mVertices[0].mAttributes[ATTRIBUTE_POSITION].z)*bary.y;
	// convert float z to 24 bit fixed z
	int iz=int(0xffffff*z);
	return (uint32)iClamp(iz,0,0xffffff);
}

bool cRasterizer::perFragment(int xi,int yi)
{
	xi >>= 2;
	yi >>= 2;
	//---------------------------------------------------------------------
	//---- FLOATING-POINT PER-PIXEL COMPUTATIONS --------------------------
	//---------------------------------------------------------------------
	
	statistics::add(STATS_PROCESSED_PIXELS);

	//------- depth test
	// since depth (z/w) is interpolated linearly, we can use Barycentric coords (no perspective)
	Vec2f bary=computeBarycentricCoords();		
	uint32 z=interpolateZ(bary);
	
	if (!mDepthUnit->depthTestAndUpdate(z,xi,yi))
		return false;	
	//------- end depth testing

	statistics::add(STATS_SHADED_PIXELS);

	// _p means "in perspective"
	Vec2f bddx_p,bddy_p;

	Vec2f bary_p = computePerspectiveBaryCoords();
	computePerspectiveBaryDerivatives(xi,yi,bddx_p,bddy_p);

	// Interpolate with perspective all enabled attributes
	cFragment frag;	
	for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++) {
		if (glState.mVertexAttributes[i].mEnabled) {
			// Interpolate attribute and compute derivatives
			frag.mAttributes[i] = interpolateAttribute(i, bary_p);
			frag.mDdx[i] = attributeDerivative(i, bddx_p);
			frag.mDdy[i] = attributeDerivative(i, bddy_p);
		}
	}

	uint32 col = 0xFFFFFFF;
	// Call fragment program (if any) or do fixed pipeline fragment operations
	if (glState.fragmentProgram != NULL) 
		col = glState.fragmentProgram(frag,&glState).toRGBA();
	else 
		col = defaultFragmentProgram(frag).toRGBA();	

	//----- write result to frame buffer
	mColorUnit->writeColor(col,xi,yi);

	return true;
}

void cRasterizer::rasterizeTriangle(void)
{	
	ASSERT(mAPI);
	ASSERT(mColorUnit);
	ASSERT(mDepthUnit);

	// brute force traversal of triangle: visit every pixel in the screen-space bounding-box of the triangle
	for(int yi= mBBox2DMin.y; yi<= mBBox2DMax.y; yi++)
	{
		for(int xi=mBBox2DMin.x; xi < mBBox2DMax.x; xi++)
		{		
			if(inside(xi,yi))								// use edge functions for inclusion testing
			{
				perFragment(xi,yi);							// do per-pixel computations, reads, and writes
			}
		}
	}
}

Color4f cRasterizer::defaultFragmentProgram(const cFragment &frag) {
	Color4f col(1,1,1,1);

	if (glState.mVertexAttributes[ATTRIBUTE_PRIMARY_COLOR].mEnabled)
		col = col ^ frag.mAttributes[ATTRIBUTE_PRIMARY_COLOR];

	if (glState.mTextureUnits[0]->isEnabled()) {
		Vec2f tc = Vec2f(frag.mAttributes[ATTRIBUTE_TEXTURE_COORD0].x,frag.mAttributes[ATTRIBUTE_TEXTURE_COORD0].y);
		Vec2f tdx = Vec2f(frag.mDdx[ATTRIBUTE_TEXTURE_COORD0].x,frag.mDdx[ATTRIBUTE_TEXTURE_COORD0].y);
		Vec2f tdy = Vec2f(frag.mDdy[ATTRIBUTE_TEXTURE_COORD0].x,frag.mDdy[ATTRIBUTE_TEXTURE_COORD0].y);		
	
		// blending
		col = col ^ glState.mTextureUnits[0]->readTexel(tc, tdx, tdy);
	}
	return col;
}