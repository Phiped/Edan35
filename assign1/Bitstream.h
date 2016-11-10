#include "misc.h"

class cBitStreamWriter {
public:
	cBitStreamWriter(void *data) { mData = data; mPos = 0; }
	~cBitStreamWriter() { mData = NULL; mPos = 0; }

	void writeBits(int nBits, uint32 data) {
		ASSERT(mData);
		for (int i = 0; i < nBits; i++) {
			if ((data >> i) & 1)
				((uint8*)mData)[mPos/8] |= (1 << (mPos % 8));
			else 
				((uint8*)mData)[mPos/8] &= ~(1 << (mPos % 8));
			mPos++;
		}
	}

	void setPos(int pos) {mPos = pos;}
	uint32 getPos() {return mPos;}

protected:
	void *mData;
	uint32 mPos;
};

class cBitStreamReader {
public:
	cBitStreamReader(const void *data) { mData = data; mPos = 0; }
	~cBitStreamReader() { mData = NULL; mPos = 0; }

	uint32 readBits(int nBits) {
		ASSERT(mData);
		uint32 data = 0;
		for (int i = 0; i < nBits; i++) {
			data |= ((uint8*)mData)[mPos/8] & (1 << (mPos % 8)) ?  1 << i : 0;
			mPos++;
		}
		return data;
	}

	int readInt(int nBits) {
		ASSERT(mData);
		uint32 data = 0;
		for (int i = 0; i < nBits; i++) {
			data |= ((uint8*)mData)[mPos/8] & (1 << (mPos % 8)) ?  1 << i : 0;
			mPos++;
		}
		if (data & (1<<(nBits-1)))
			return (0xffffffff & ~((1<<nBits)-1)) | data;
		return data;
	}

	void setPos(int pos) {mPos = pos;}
	uint32 getPos() {return mPos;}

protected:
	const void *mData;
	uint32 mPos;
};