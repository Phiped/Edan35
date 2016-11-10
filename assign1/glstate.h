#ifndef GL_STATE_H
#define GL_STATE_H

#include <hash_map>

#include "GL/glut.h"
#include "glext.h"

#include "misc.h"
#include "texture.h"
#include "textureunit.h"
#include "vertex.h"

#define MAX_TEXTURE_UNITS 16

/**
 *	\brief The purpose of this class is to hold the current "OpenGL" state. These variables are global to the whole rendering API.
 */
class cGLState {
public:

	/**
	 *	\brief Internal struct that specifies the current state of a vertex array attribute. See glVertexPointer, glColorPointer, glTexCoordPointer, glVertexAttribPointer	 
	 */
	struct cPointerArray {
		bool			mEnabled;
		bool			mNormalize;
		GLenum			mType;
		GLsizei			mSize;
		GLsizei			mStride;
		const void		*mPointer;
	};	

	// ---- States for all vertex array attributes ----
	cPointerArray				mVertexAttributes[MAX_VERTEX_ATTRIBS];

	// ---- Color buffer state variables ----
	uint32						mClearColor;
	bool						mWriteEnableRed;
	bool						mWriteEnableGreen;
	bool						mWriteEnableBlue;
	bool						mWriteEnableAlpha;
	
	// ---- Depth buffer state varaibles ----
	bool						mDepthTestEnabled;
	uint32						mDepthFunc;
	bool						mWriteEnableDepth;

	// ---- Texture objects and texture unit states ----
	GLuint									mActiveTextureUnit;						// Currently active texture unit, see glActiveTexture
	cTextureUnit							*mTextureUnits[MAX_TEXTURE_UNITS];
	GLsizei									mTexId;
	stdext::hash_map<GLuint, cTexture *>	mTextures;
	cTextureCache							*mTextureCache;

	//X.X Fragment and Vertex Programs
	void (*vertexProgram)(cVertex &vertex,const cGLState *glState);
	Color4f (*fragmentProgram)(const cFragment &fragment,const cGLState *glState);

	cGLState();
};

extern cGLState &glState;
#endif