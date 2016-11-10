#include "GL/glut.h"
#include "glstate.h"

#include "mmgr/mmgr.h"

cGLState _glState;
cGLState &glState = _glState;

cGLState::cGLState() {
	mClearColor=0;

	mWriteEnableRed=true;
	mWriteEnableGreen=true;
	mWriteEnableBlue=true;
	mWriteEnableAlpha=true;
	mDepthTestEnabled=true;

	mDepthFunc=GL_LESS;
	mWriteEnableDepth=true;

	mTexId = 1;
	mActiveTextureUnit = 0;

	mTextureCache=NULL;
// uncomment the following two lines if you want to use a texture cache
//	mTextureCache = new cTextureCache(16,2);
//	mTextureCache->printCacheSize();

	for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++)
		mVertexAttributes[i].mEnabled = false;

	for (int i = 0; i < MAX_TEXTURE_UNITS; i++)
		mTextureUnits[i] = new cTextureUnit(mTextureCache);

	vertexProgram = NULL;
	fragmentProgram = NULL;
}