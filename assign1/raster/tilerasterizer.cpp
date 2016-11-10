// -------------------------------------------------------------------------
// File:    tilerasterizer
// Desc:    basic tile rasterizer class: visits all pixels inside an NxM
//			block (tile) of pixels before moving on to the next tile ==> improves
//			coherency (good for texture caching etc)
//			TODO: uses conservative rasterization algo by Akenine-Möller and Aila
//			for finding tiles overlapped by triangle
// Author:  Tomas Akenine-Möller
// History: March, 2005 (started)
// -------------------------------------------------------------------------

#include "tilerasterizer.h"
#include "rendererAPI.h"
#include "depthunit.h"
#include "colorunit.h"

#include "mmgr/mmgr.h"

void cTileRasterizer::setTileSize(int tile_width,int tile_height)
{
	// tile_width and tile_height must be powers of two!
	if(!isPowerOfTwo(tile_width))
	{
		// find most significant bit that is set
		int q;
		for(q=15;q>=0;q--) if((1<<q) & tile_width) break;
		tile_width=1<<q;		// make it power of two, i.e., round down towards the closest power of two
	}
	if(!isPowerOfTwo(tile_height))
	{
		// find most significant bit that is set
		int q=0;
		for(int q=15;q>=0;q--) if((1<<q) & tile_height) break;
		tile_height=1<<q;		// make it power of two, i.e., round down towards the closest power of two
	}

	mTileWidth=tile_width;
	mTileHeight=tile_height;
	printf("tile size: %d x %d\n",mTileWidth,mTileHeight);
}

const uint16 cTileRasterizer::getTileWidth(void) const
{
	return mTileWidth;
}

const uint16 cTileRasterizer::getTileHeight(void) const
{
	return mTileHeight;
}

bool cTileRasterizer::setup(const cVertex &v0,const cVertex &v1,const cVertex &v2)
{
	bool setup_ok=cRasterizer::setup(v0,v1,v2);
	
	// add cTileRasterizer-specific setup code here

	mZMin = mZMax = (uint32)iClamp( int(0xffffff*mVertices[0].mAttributes[ATTRIBUTE_POSITION].z), 0, 0xffffff );

	for (int i = 1; i < 3; i++) {
		// convert float z to 24 bit fixed z
		uint32 z = (uint32)iClamp( int(0xffffff*mVertices[i].mAttributes[ATTRIBUTE_POSITION].z), 0, 0xffffff );
		mZMin = MIN2(mZMin,z);
		mZMax = MAX2(mZMax,z);
	}


	// compute tile coordinates
	mTileCoordsMin.x=(mBBox2DMin.x/mTileWidth);							// floor 
	mTileCoordsMin.y=(mBBox2DMin.y/mTileHeight);						// floor
	mTileCoordsMax.x=((mBBox2DMax.x+mTileWidth-1)/mTileWidth);			// ceil 
	mTileCoordsMax.y=((mBBox2DMax.y+mTileHeight-1)/mTileHeight);		// ceil

	return setup_ok;
}

bool cTileRasterizer::preTile(int tile_x,int tile_y)
{
	Vec2f bary;
	uint32 zMin = mZMax,zMax = mZMin,z;

	// Fixme: Add Tomas' conservative tile test


	int xadd[4] = {0,mTileWidth-1,0,mTileWidth-1};
	int yadd[4] = {0,0,mTileHeight-1,mTileHeight-1};

	// add specific tile code here (to be executed just before the pixels are being traversed)
	for (int i = 0; i < 4; i++) {
		inside(tile_x*mTileWidth + xadd[i],tile_y*mTileHeight + yadd[i]);
		bary = computeBarycentricCoords();
		z = interpolateZ(bary);	
		zMin = MIN2(zMin,z);
		zMax = MAX2(zMax,z);
	}
	zMin = MAX2(mZMin,zMin);	
	zMax = MIN2(mZMax,zMax);	

	return !mDepthUnit->cullTile(zMin,zMax,tile_x*mTileWidth,tile_y*mTileHeight);
}

void cTileRasterizer::postTile(int tile_x,int tile_y)
{
	// add specific tile code here (to be executed just after the pixels have been traversed)

}


void cTileRasterizer::rasterizeTriangle(void)
{	
	ASSERT(mAPI);
	//ASSERT(mFrameBuffer);
	ASSERT(mDepthUnit);

#if 0
	mBBox2DMin.debugprint();
	mBBox2DMax.debugprint();
	mTileCoordsMin.debugprint();
	mTileCoordsMax.debugprint();
	printf("\n");
#endif

	// start with brute force traversal of tile inside bbox of triangle: visit every pixel in there
	// loop over all tiles
	for(int byi=mTileCoordsMin.y; byi<mTileCoordsMax.y; byi++)
	{
		for(int bxi=mTileCoordsMin.x; bxi<mTileCoordsMax.x; bxi++)
		{
			// loop over all pixels inside current tiles
			preTile(bxi,byi);
			for(int yi=byi*mTileHeight; yi<(byi+1)*mTileHeight ;yi++)
			{
				for(int xi=bxi*mTileWidth; xi<(bxi+1)*mTileWidth ;xi++)
				{
					if(inside(xi,yi))								// use edge functions for inclusion testing
					{
						perFragment(xi,yi);
					}
				}
			}
			postTile(bxi,byi);
		}
	}
}