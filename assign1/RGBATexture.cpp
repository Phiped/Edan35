#include "texture.h"
#include "statistics.h"

#include "mmgr/mmgr.h"

cRGBATexture::cRGBATexture(GLuint textureId) : cTexture(textureId) {
}

cRGBATexture::~cRGBATexture() {
	for (size_t i = 0; i < mMipLevels.size(); i++)
		if (mMipLevels[i] != NULL)
			delete mMipLevels[i];
	mMipLevels.clear();
}

void cRGBATexture::setData(int mipLevel, int width, int height, const void *data) {
	if ((int)mMipLevels.size() <= mipLevel) {
		size_t prevSize = mMipLevels.size();
		mMipLevels.resize(mipLevel+1);
		for (size_t i = prevSize; i < mMipLevels.size(); i++) 
			mMipLevels[i] = NULL;
	}
	
	mMipLevels[mipLevel] = new Buffer<uint32>(width,height);

	uint32 *idata = (uint32*) data;
	for (int y = 0; y < height; y++) 
		for (int x = 0; x < width; x++) 
			mMipLevels[mipLevel]->set(idata[x+y*width],x,y);	   
}

int cRGBATexture::getNumMipmaps() {
	return (int)mMipLevels.size();
}

void cRGBATexture::getSize(int mipLevel, int &width, int &height) {
	mMipLevels[mipLevel]->getSize(width,height);
}


Color4r cRGBATexture::readTexel(cTextureCache *textureCache, int mipLevel, int u, int v) {
	ASSERT(mipLevel < (int)mMipLevels.size());

	uint32 col = mMipLevels[mipLevel]->get( u, v );
	if (textureCache != NULL) {
		int w,h;
		mMipLevels[mipLevel]->getSize(w,h);
//		int tu = u/4, tv = v/4;
//		cTextureEntry Entry(mTextureId,mipLevel, tu + tv*(w/4));

#define TILE_SIZE 4
		int tu = (u/TILE_SIZE) % TILE_SIZE, tv = (v/TILE_SIZE) % TILE_SIZE;
		int stu = u/(TILE_SIZE*TILE_SIZE), stv = v/(TILE_SIZE*TILE_SIZE);

		uint32 tileAddr = (tu%2 + (tv%2)*2) + 4*(((tu - tu%2)/2) + ((tv - tv%2)/2)*(TILE_SIZE/2));
		uint32 superTileAddr = (stu + stv*(w/(TILE_SIZE*TILE_SIZE)))*TILE_SIZE*TILE_SIZE;

		cTextureEntry Entry(mTextureId,mipLevel, superTileAddr*(2*TILE_SIZE*TILE_SIZE) + (mipLevel%2)*(TILE_SIZE*TILE_SIZE) + tileAddr);
		
		if (textureCache->findEntry(Entry) == NULL) {
			
			statistics::add(STATS_TEXTURE_READ_BW, (CACHE_ENTRY_SIZE*8));
			statistics::add(STATS_TEXTURE_NUM_READS);

			ASSERT(CACHE_ENTRY_SIZE == 64);
			textureCache->addEntry(Entry);
		}
	}
	else {
		statistics::add(STATS_TEXTURE_READ_BW, 32);	
		statistics::add(STATS_TEXTURE_NUM_READS);
	}

	return Color4f(col);
}