// -------------------------------------------------------------------------
// File:    rendererAPI.cpp
// Desc:    the API that we will use to access our rasterizer
//			should be very OpenGL ES-like (a subset)
//
// Author:  Tomas Akenine-Möller
// History: September, 2004 (started)
// -------------------------------------------------------------------------

#include "rendererAPI.h"
#include "misc.h"
#include "vertex.h"
#include "miscOGL.h"
#include "depthunit.h"
#include "colorunit.h"
#include "glstate.h"

#include "mmgr/mmgr.h"

cRendererAPI::cRendererAPI(void)
{
	mRasterizer=NULL;

	mLURActive=true;
}

void cRendererAPI::setRasterizer(cRasterizer *rasterizer)
{
	ASSERT(rasterizer);
	mRasterizer=rasterizer;
}

cRasterizer* cRendererAPI::getRasterizer(void)
{
	return mRasterizer;
}

void cRendererAPI::setResolution(int width, int height)
{
	mRasterizer->getColorUnit()->reshape(width,height);
	mRasterizer->getDepthUnit()->reshape(width,height);	
}
void cRendererAPI::activateLUR(void)
{
	mLURActive=true;			// make our software rasterizer (LUR) active by default
}

void cRendererAPI::activateHW_OGL(void)
{
	mLURActive=false;
}

int cRendererAPI::getWidth(void)
{
	return mRasterizer->getColorUnit()->getWidth();
}

int cRendererAPI::getHeight(void)
{
	return mRasterizer->getColorUnit()->getHeight();
}

const uint32 cRendererAPI::getColorWriteMask(void) const
{
	return (uint32) 0xff000000*(glState.mWriteEnableAlpha ? 1 : 0) +
					0x00ff0000*(glState.mWriteEnableBlue  ? 1 : 0) +
					0x0000ff00*(glState.mWriteEnableGreen ? 1 : 0) +
					0x000000ff*(glState.mWriteEnableRed   ? 1 : 0);
}

const uint32 cRendererAPI::getDepthFunc(void) const
{
	return glState.mDepthFunc;
}

void cRendererAPI::_glEnable(int what)
{
	if(mLURActive)
	{
		switch(what)
		{
		case GL_DEPTH_TEST:
			glState.mDepthTestEnabled=true;
			break;
		case GL_TEXTURE_2D:
			glState.mTextureUnits[ glState.mActiveTextureUnit ]->enable();
			break;
		}
	}
	else glEnable(what);
	
}

void cRendererAPI::_glDisable(int what)
{
	if(mLURActive)
	{
		switch(what)
		{
		case GL_DEPTH_TEST:
			glState.mDepthTestEnabled=false;
			break;
		case GL_TEXTURE_2D:
			glState.mTextureUnits[ glState.mActiveTextureUnit ]->disable();
			break;
		}
	}
	else glDisable(what);
}


////////////////////////////////////////////////
// 4.2 Whole frame buffer operations
////////////////////////////////////////////////

void cRendererAPI::_glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	if(mLURActive)
	{
		glState.mClearColor  = cropToByte(r);
		glState.mClearColor |= cropToByte(g)<<8;
		glState.mClearColor |= cropToByte(b)<<16;
		glState.mClearColor |= cropToByte(a)<<24;
	}
	else
	{
		glClearColor(r,g,b,a);
	}
}

void cRendererAPI::_glClear(GLbitfield clearBits)
{
	if(mLURActive)
	{
		if(clearBits|GL_COLOR_BUFFER_BIT) mRasterizer->getColorUnit()->clearBuffer(glState.mClearColor);
		if(clearBits|GL_DEPTH_BUFFER_BIT) mRasterizer->getDepthUnit()->clearBuffer(0x00ffffff);
	}
	else
	{
		glClear(clearBits);
	}
}

void cRendererAPI::_glColorMask(bool red, bool green, bool blue, bool alpha)
{
	if(mLURActive)
	{
		glState.mWriteEnableRed=red;
		glState.mWriteEnableGreen=green;
		glState.mWriteEnableBlue=blue;
		glState.mWriteEnableAlpha=alpha;
	}
	else
	{
		glColorMask(red,green,blue,alpha);
	}
}

////////////////////////////////////////////////
// 2.8 Vertex arrays
////////////////////////////////////////////////

void cRendererAPI::_glVertexPointer(int size,GLenum type, GLsizei stride,const void *ptr)
{
	if(mLURActive)
		_glVertexAttribPointer(ATTRIBUTE_POSITION,size,type,false,stride,ptr);
	else
		glVertexPointer(size,type,stride,ptr);
}

void cRendererAPI::_glColorPointer(int size,GLenum type, GLsizei stride,const void *ptr)
{
	if(mLURActive)
		_glVertexAttribPointer(ATTRIBUTE_PRIMARY_COLOR,size,type,true,stride,ptr);
	else
		glColorPointer(size,type,stride,ptr);
}

void cRendererAPI::_glTexCoordPointer(int size,GLenum type, GLsizei stride,const void *ptr)
{
	if(mLURActive)
		_glVertexAttribPointer(ATTRIBUTE_TEXTURE_COORD0,size,type,true,stride,ptr);
	else
		glTexCoordPointer(size,type,stride,ptr);
}

void cRendererAPI::_glVertexAttribPointer(GLuint index, 
										  GLint size, 
										  GLenum type, 
										  GLboolean normalized, 
										  GLsizei stride, 
										  const void *pointer) {
	if (mLURActive) {	
		glState.mVertexAttributes[index].mSize = size;
		glState.mVertexAttributes[index].mStride = stride;
		glState.mVertexAttributes[index].mPointer = pointer;
		glState.mVertexAttributes[index].mNormalize = normalized == GL_TRUE;
		glState.mVertexAttributes[index].mType = type;
	}
	else {
		// FIXME: Need glew or something for this
		//glVertexAttribPointerARB(index,size,type,normalized,stride,pointer;
	}
}
	
void cRendererAPI::_glEnableVertexAttribArray(GLuint index) {
	if (mLURActive) {
		glState.mVertexAttributes[index].mEnabled = true;
	}
	else {
		// FIXME: Need glew or something for this
		//glEnableVertexAttribArray(index);
	}
}

void cRendererAPI::_glDisableVertexAttribArray(GLuint index) {
	if (mLURActive) {
		glState.mVertexAttributes[index].mEnabled = false;
	}
	else {
		// FIXME: Need glew or something for this
		//glEnableVertexAttribArray(index);
	}
}

void cRendererAPI::_glDrawArrays(GLenum mode, int first, GLsizei vertex_count)
{
	if(mLURActive)
	{
		if(!glState.mVertexAttributes[ATTRIBUTE_POSITION].mEnabled) return;
		switch(mode)
		{
		case GL_TRIANGLES:
			processTriangles(first,vertex_count);
			break;
		default:
			printf("Error: glDrawArrays currently only supports GL_TRIANGLES.\n");
			return;
		}
	}
	else
	{
		glDrawArrays(mode,first,vertex_count);
	}
}

void cRendererAPI::_glEnableClientState(GLenum cap)
{
	if(mLURActive)
	{
		switch(cap)
		{
		case GL_VERTEX_ARRAY:
			_glEnableVertexAttribArray(ATTRIBUTE_POSITION);
			break;
		case GL_COLOR_ARRAY:
			_glEnableVertexAttribArray(ATTRIBUTE_PRIMARY_COLOR);
			break;
		case GL_TEXTURE_COORD_ARRAY:
			_glEnableVertexAttribArray(ATTRIBUTE_TEXTURE_COORD0);
			break;
		}
	}
	else glEnableClientState(cap);
}

void cRendererAPI::_glDisableClientState(GLenum cap)
{
	if(mLURActive)
	{
		switch(cap)
		{
		case GL_VERTEX_ARRAY:
			_glDisableVertexAttribArray(ATTRIBUTE_POSITION);
			break;
		case GL_COLOR_ARRAY:
			_glDisableVertexAttribArray(ATTRIBUTE_PRIMARY_COLOR);
			break;
		case GL_TEXTURE_COORD_ARRAY:
			_glDisableVertexAttribArray(ATTRIBUTE_TEXTURE_COORD0);
			break;
		}
	}
	else glDisableClientState(cap);
}

////////////////////////////////////////////////
// 2.11 Coordinate transformations
////////////////////////////////////////////////

void cRendererAPI::_glViewport(int x,int y, GLsizei w, GLsizei h)
{
	printf("glViewport: do not forget to take into account this one!\n");
	glViewport(x,y,w,h);
	
	mRasterizer->getColorUnit()->reshape(w,h);
	mRasterizer->getDepthUnit()->reshape(w,h);	//FIXME: Bad placement? Where should we place it?
}

void cRendererAPI::_glMatrixMode(GLenum mode)
{
	glMatrixMode(mode);
}

void cRendererAPI::_glLoadMatrixf(float m[16])
{
	glLoadMatrixf(m);
}

void cRendererAPI::_glMultMatrixf(float m[16])
{
	glMultMatrixf(m);
}

void cRendererAPI::_glLoadIdentity(void)
{
	glLoadIdentity();
}

void cRendererAPI::_glRotatef(float angle, float x, float y, float z)
{
	glRotatef(angle,x,y,z);
}

void cRendererAPI::_glScalef(float x, float y, float z)
{
	glScalef(x,y,z);
}

void cRendererAPI::_glTranslatef(float x,float y, float z)
{
	glTranslatef(x,y,z);
}

void cRendererAPI::_glFrustum(double l, double r, double b, double t, double n, double f)
{
	glFrustum(l,r,b,t,n,f);
}

void cRendererAPI::_glOrtho(double l, double r, double b, double t, double n, double f)
{
	glOrtho(l,r,b,t,n,f);
}

void cRendererAPI::_glPushMatrix(void)
{
	glPushMatrix();
}

void cRendererAPI::_glPopMatrix(void)
{
	glPopMatrix();
}

////////////////////////////////////////////////////
// 4.1 Per-Fragment Operations and the Framebuffer
////////////////////////////////////////////////////
void cRendererAPI::_glDepthFunc(GLenum func)
{
	if(mLURActive)
	{
		glState.mDepthFunc=func;
	}
	else
	{
		glDepthFunc(func);
	}
}

void cRendererAPI::_glDepthMask(GLboolean write_depth_enable)
{
	if(mLURActive)
	{
		glState.mWriteEnableDepth= write_depth_enable ? true : false;
	}
	else
	{
		glDepthMask(write_depth_enable);
	}
}


void cRendererAPI::_glutSwapBuffers(void)
{
	if(mLURActive)
	{
		// write framebuffer's color to screen here
		int w = mRasterizer->getColorUnit()->getWidth(),h = mRasterizer->getColorUnit()->getHeight();
		uint32 *pImage = new uint32[ w*h ];
		mRasterizer->getColorUnit()->getBuffer(pImage);

		//pImage=(uint32*)mRasterizer->getColorUnit()->getBuffer(w,h);

		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();   
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, w, 0, h);
		glRasterPos2f(0, 0);
		glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
		glFlush();

		glPopMatrix();
		glPopAttrib();

		glutSwapBuffers();

		delete[] pImage;
	}
	else
	{
		glutSwapBuffers();
	}	
}

////////////////////////////////////////////////////
// 3.8 Texturing
////////////////////////////////////////////////////

void cRendererAPI::_glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,const void *pixels) {
	GLuint textureId = glState.mTextureUnits[glState.mActiveTextureUnit]->getTexture();

	ASSERT(textureId != 0 && glState.mTextures.find(textureId) != glState.mTextures.end());
	ASSERT(format == GL_RGBA);
	ASSERT(type == GL_UNSIGNED_BYTE);

	//FIXME: implement glPixelStorei UNPACK/PACK _ALIGNMENT

	if (glState.mTextures[textureId] == NULL)
		glState.mTextures[textureId] = new cRGBATexture(textureId);
	glState.mTextures[textureId]->setData(level,width,height,pixels);	
	
	glTexImage2D(target,level,internalFormat,width,height,border,format,type,pixels);
}

void cRendererAPI::_glGenTextures(GLsizei n, GLuint *textures) {
	for (int i = 0; i < n; i++) {
		ASSERT(glState.mTexId != 0);
		
		glGenTextures(1,&textures[i]);
		glState.mTextures[textures[i]] = NULL;
	}
}

void cRendererAPI::_glDeleteTextures(GLsizei n, const GLuint *textures) {
	for (int i = 0; i < n; i++) {
		ASSERT(glState.mTextures.find(textures[i]) != glState.mTextures.end());
		
		if (glState.mTextures[textures[i]] != NULL)
			delete glState.mTextures[textures[i]];
		glState.mTextures.erase(glState.mTextures.find(textures[i]));
	}
	glDeleteTextures(n,textures);
}

void cRendererAPI::_glActiveTexture(GLenum textureUnit) {
	ASSERT(textureUnit >= GL_TEXTURE0_ARB && textureUnit < GL_TEXTURE0_ARB + MAX_TEXTURE_UNITS);
	glState.mActiveTextureUnit = textureUnit - GL_TEXTURE0_ARB;
}

void cRendererAPI::_glBindTexture(GLenum target,GLuint texture) {
	ASSERT(target == GL_TEXTURE_2D);
	glState.mTextureUnits[glState.mActiveTextureUnit]->setTexture(texture);
	glBindTexture(GL_TEXTURE_2D,texture);
}

void cRendererAPI::_glTexParameteri(GLenum target, GLenum pname, GLenum param) {
	ASSERT(target == GL_TEXTURE_2D);
	GLuint texId = glState.mTextureUnits[glState.mActiveTextureUnit]->getTexture();
	if (mLURActive) {
		switch(pname) {
			case GL_TEXTURE_MAG_FILTER:
				glState.mTextures[texId]->mMagFilter = param;
				break;
			case GL_TEXTURE_MIN_FILTER:
				glState.mTextures[texId]->mMinFilter = param;
				break;
			case GL_TEXTURE_WRAP_S:
				glState.mTextures[texId]->mWrapS = param;
				break;
			case GL_TEXTURE_WRAP_T:
				glState.mTextures[texId]->mWrapT = param;
				break;
			default:
				ASSERT(false);
				break;
		}
	}
	glTexParameteri(target,pname,param);
}

////////////////////////////////////////////////////
// X.X Fragment and Vertex Programs
////////////////////////////////////////////////////
void cRendererAPI::_glSetVertexProgram( void (*vertexProgram)(cVertex &vertex,const cGLState *glState) ) {
	glState.vertexProgram = vertexProgram;
}
void cRendererAPI::_glSetFragmentProgram( Color4f (*fragmentProgram)(const cFragment &fragment,const cGLState *glState) ) {
	glState.fragmentProgram = fragmentProgram;
}


////////////////////////////////////////////////////
// Primitive processing
////////////////////////////////////////////////////

inline Vec4f getAttribute(cGLState::cPointerArray *ptrArray, GLsizei index, Vec4f defaultVal) {
	int ComponentSize;
	switch (ptrArray->mType) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			ComponentSize = 1;
			break;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			ComponentSize = 2;
			break;
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:
			ComponentSize = 4;
			break;
		case GL_DOUBLE:
			ComponentSize = 8;
			break;
	}
	
	int attribSize = ptrArray->mStride == 0 ? ptrArray->mSize*ComponentSize : ptrArray->mStride;
	void *elementPtr = ((uint8*)ptrArray->mPointer) + index*attribSize;
	Vec4r result = defaultVal;

	for(int i = 0; i < ptrArray->mSize; i++) {
		switch (ptrArray->mType) {
			case GL_BYTE:
				if (ptrArray->mNormalize)
					result[i] = (float)((int8 *)elementPtr)[i] / 128.0f;
				else
					result[i] = ((int8 *)elementPtr)[i];
				break;
			case GL_UNSIGNED_BYTE:
				if (ptrArray->mNormalize)
					result[i] = (float)((uint8 *)elementPtr)[i] / 255.0f;
				else
					result[i] = ((uint8 *)elementPtr)[i];
				break;
			case GL_SHORT:
				if (ptrArray->mNormalize)
					result[i] = (float)((int16 *)elementPtr)[i] / 32768.0f;
				else
					result[i] = ((int16 *)elementPtr)[i];
				break;
			case GL_UNSIGNED_SHORT:
				if (ptrArray->mNormalize)
					result[i] = (float)((uint16 *)elementPtr)[i] / 65535.0f;
				else
					result[i] = ((uint16 *)elementPtr)[i];
				break;
			case GL_INT:
				if (ptrArray->mNormalize)
					result[i] = (float)((int32 *)elementPtr)[i] / 2147483648.0f;
				else
					result[i] = ((int32 *)elementPtr)[i];
				break;
			case GL_UNSIGNED_INT:
				if (ptrArray->mNormalize)
					result[i] = (float)((uint32 *)elementPtr)[i] / 4294967295.0f;
				else
					result[i] = ((uint32 *)elementPtr)[i];
				break;
			case GL_FLOAT:
				result[i] = ((float *)elementPtr)[i];	// Normalized isn't defined for floats/doubles in the spec
				break;
			case GL_DOUBLE:
				result[i] = ((double *)elementPtr)[i];	// Normalized isn't defined for floats/doubles in the spec
				break;
		}
	}
	return result;
}

void cRendererAPI::defaultVertexProgram(cVertex &vertex) {
	Mtx4f M,T;
	fGetModelviewProjection(M);//fGetModel2ScreenMatrix(M);
	fGetTextureMatrix(T);

	vertex.mAttributes[ATTRIBUTE_POSITION] = M*vertex.mAttributes[ATTRIBUTE_POSITION];
	vertex.mAttributes[ATTRIBUTE_TEXTURE_COORD0] = T*vertex.mAttributes[ATTRIBUTE_TEXTURE_COORD0];
}

void cRendererAPI::clipReset() {
	mNumGenClipVerts = 0;
}

void cRendererAPI::clipByPlane(int &nVerts, cVertex **vertPointers, const Vec4f &pn, float pd) {	

	int		outVerts = 0;
	cVertex *resultPointers[MAX_CLIP_VERTS];

	float	thisDp = pn*vertPointers[0]->mAttributes[ATTRIBUTE_POSITION];
	bool	thisFront = thisDp > -pd;

	for (int i = 0; i < nVerts; i++) {
		ASSERT(outVerts < MAX_CLIP_VERTS);

		int next = (i+1)%nVerts;
		if (thisFront) 
			resultPointers[outVerts++] = vertPointers[i];		

		float nextDp = pn*vertPointers[next]->mAttributes[ATTRIBUTE_POSITION];
		bool nextFront = nextDp > -pd;
		if (thisFront != nextFront) {
			ASSERT(mNumGenClipVerts < MAX_CLIP_VERTS);
			float t = (pd + thisDp) / (thisDp - nextDp);
			for (int u = 0; u < MAX_VERTEX_ATTRIBS; u++) 
				mGenClipVerts[mNumGenClipVerts].mAttributes[u] = vertPointers[i]->mAttributes[u]*(1-t) + vertPointers[next]->mAttributes[u]*t;
			
			resultPointers[outVerts++] = &mGenClipVerts[mNumGenClipVerts++];
		}

		thisDp = nextDp;
		thisFront = nextFront;
	}

	nVerts = outVerts;
	memcpy(vertPointers, resultPointers, sizeof(cVertex*)*nVerts);
}

void cRendererAPI::processTriangles(GLsizei first, GLsizei vertex_count) {
	ASSERT(vertex_count % 3 == 0);

	const int num_tris=vertex_count/3;

	for(GLsizei i = 0; i < num_tris; i++)
	{
		cVertex v[3];
		for (int j = 0; j < 3; j++) {
			for (int a = 0; a < MAX_VERTEX_ATTRIBS; a++) {
				v[j].mAttributes[a] = Vec4f(0,0,0,1);
				if (glState.mVertexAttributes[a].mEnabled)
					v[j].mAttributes[a] = getAttribute(&glState.mVertexAttributes[a], first + i*3 + j, Vec4f(0,0,0,1));
			}

			//Run the vertex program, or the default (fixed function) vertex program
			if (glState.vertexProgram != NULL)
				glState.vertexProgram(v[j], &glState);
			else // No vertex program, do fixed function vertex processing
				defaultVertexProgram(v[j]);
		}

		// Clip triangle by standard OpenGL clipping planes
		int nVerts = 3;
		cVertex *clipResult[MAX_CLIP_VERTS] = {&v[0],&v[1],&v[2]};
		
		clipReset();
		clipByPlane(nVerts,clipResult, Vec4f(1,0,0,1).normalize(), 0);	// -wc <= xc <==> 0 <= xc + wc
		clipByPlane(nVerts,clipResult, Vec4f(-1,0,0,1).normalize(), 0);	// xc <= wc <==> 0 <= wc - xc
		clipByPlane(nVerts,clipResult, Vec4f(0,1,0,1).normalize(), 0);	// -wc <= yc <==> 0 <= yc + wc
		clipByPlane(nVerts,clipResult, Vec4f(0,-1,0,1).normalize(), 0);	// yc <= wc <==> 0 <= wc - yc
		clipByPlane(nVerts,clipResult, Vec4f(0,0,1,1).normalize(), 0);	// -wc <= zc <==> 0 <= zc + wc
		clipByPlane(nVerts,clipResult, Vec4f(0,0,-1,1).normalize(), 0);	// zc <= wc <==> 0 <= wc - zc

		// Rasterize the clipped result (triangle fan)
		for (int j = 1; j < nVerts-1; j++) {
			if(!mRasterizer->setup(*clipResult[0],*clipResult[j],*clipResult[j+1])) break;
			mRasterizer->rasterizeTriangle();			
		}
	}
}
