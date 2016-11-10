// 121031 - Fix cache findEntry and addEntry to use entire cache, not just one set. Fix thanks to Ola Olsson. MCD

#ifndef C_CACHE_H
#define C_CACHE_H

#include "GL/glut.h"
#include <stdlib.h>
#include "misc.h"

#define CACHE_ENTRY_SIZE 64		// number of bytes... do not change this! requires changes in cRGBATexture::readTexel()...

class cCacheEntry {
	bool modified,dirty;
public:
	cCacheEntry()				{modified = false; dirty = true;}
	~cCacheEntry()				{modified = false; dirty = true;}
    
	bool isModified()			{return modified && !dirty;}
	void setModified(bool m)	{modified = m;}
	bool isDirty()				{return dirty;}
	void setDirty(bool d)		{dirty = d;}

	// This is the general interface any cache entry should implement:
	// uint32 getKey() const;								- return the cache key
	// bool operator==(const cTextureEntry &other) const;	- compare two cache entries
	// void writeBack();									- called when a modified entry is removed from the cache
};

class cTextureEntry : public cCacheEntry {
	GLuint				mTexture;
	GLuint				mMipLevel;
	GLuint				mPosKey;

public:

	cTextureEntry() : cCacheEntry() {
		mTexture = mMipLevel = mPosKey = 0;
	}
	cTextureEntry(GLuint texture, GLuint mipLevel, GLuint posKey) : cCacheEntry() {
		mTexture = texture; mMipLevel = mipLevel; mPosKey = posKey;
	}
	~cTextureEntry() {
		mTexture = mMipLevel = mPosKey = 0;
	}

	uint32 getKey() const { return mPosKey; }
	bool operator==(const cTextureEntry &other) const {
		return mTexture == other.mTexture && mMipLevel == other.mMipLevel && mPosKey == other.mPosKey;
	}	
	void writeBack() {}
};

template<class CacheEntry>
class cCache {
public:
	cCache(int nSets, int nEntries) {
		mNumSetEntries = nEntries;
		mNumSets = nSets;
		mEntries = new CacheEntry[ mNumSetEntries*mNumSets ];
	}
	~cCache() {
		ASSERT(mEntries != NULL);

		delete[] mEntries;
		mEntries = NULL;
		mNumSets = 0;
		mNumSetEntries = 0;
	}


	void printCacheSize(void)
	{ 
		printf("Size of cache is: %d bytes\n",mNumSetEntries*mNumSets*CACHE_ENTRY_SIZE);
	}


	CacheEntry *findEntry(const CacheEntry &Entry) {
		uint32 key = (Entry.getKey() % mNumSetEntries);
		uint32 index;
		for (int i = 0; i < mNumSets; i++) {
			index = key * mNumSets + i;
			ASSERT(index < mNumSetEntries * mNumSets);
			if (!mEntries[index].isDirty() && mEntries[index] == Entry)
				return &mEntries[index];
		}
		return NULL;
	}

	CacheEntry *addEntry(const CacheEntry &Entry) {
		ASSERT(findEntry(Entry) == NULL);
		
		GLuint key = (Entry.getKey() % mNumSetEntries) * mNumSets + (int)((float)rand()/(RAND_MAX+1) * mNumSets);
		if (mEntries[key].isModified()) {
			mEntries[key].writeBack();
			mEntries[key].setModified(false);
		}
		
		mEntries[key] = Entry;		
		mEntries[key].setDirty(false);
		return &mEntries[key];
	}

	void invalidate() {
		for (int i = 0; i < mNumSets*mNumSetEntries; i++)
			mEntries[i].setDirty(true);
	}

protected:
	int					mNumSets;
	int					mNumSetEntries;
	CacheEntry			*mEntries;
};

typedef cCache<cTextureEntry>	cTextureCache;

#endif