// -------------------------------------------------------------------------
// File:    tilerasterizer.h
// Desc:    basic tile rasterizer class: visits all pixels inside an NxM
//			block (tile) of pixels before moving on to the next tile ==> improves
//			coherency (good for texture caching etc)
//			TODO: uses conservative rasterization algo by Akenine-Möller and Aila
//			for finding tiles overlapped by triangle
// Author:  Tomas Akenine-Möller
// History: March, 2005 (started)
// -------------------------------------------------------------------------

#ifndef C_TILE_RASTERIZER_H
#define C_TILE_RASTERIZER_H

#include "rasterizer.h"

class cTileRasterizer:public cRasterizer
{
public:
					cTileRasterizer				(cRendererAPI *api,cDepthUnit *du,cColorUnit *cu,cTextureUnit *tu):cRasterizer(api,du,cu,tu)
												{mTileWidth=8; mTileHeight=8;}
	virtual void    setTileSize					(int tile_width,int tile_height);
	const uint16	getTileWidth				(void) const;
	const uint16	getTileHeight				(void) const;
	virtual void	rasterizeTriangle			(void);
	virtual bool    preTile						(int tile_x,int tile_y);
	virtual void    postTile					(int tile_x,int tile_y);
	virtual bool	setup						(const cVertex &v0,const cVertex &v1,const cVertex &v2);
	virtual	char*   getClassName				(void) {return "cTileRasterizer";}
protected:
	uint16			mTileWidth;
	uint16			mTileHeight;
	Vec2i			mTileCoordsMin;
	Vec2i			mTileCoordsMax;

	uint32			mZMin, mZMax;
};

#endif
