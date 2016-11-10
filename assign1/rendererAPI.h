// -------------------------------------------------------------------------
// File:    rendererAPI.h
// Desc:    the API that we will use to access our rasterizer
//			should be very OpenGL ES-like
//
// Authors: Tomas Akenine-Möller and Jon Hasselgren
// History: September, 2004 (started)
// -------------------------------------------------------------------------

#ifndef C_RENDERER_API_H
#define C_RENDERER_API_H

#include "GL/glut.h"
#include "glext.h"

#include "buffer.h"
#include "rasterizer.h"
#include "glstate.h"

class cRendererAPI
{
public:	
					cRendererAPI				(void);
	void			activateLUR					(void);
	void			activateHW_OGL				(void);
	void			setResolution				(int width, int height);
	int				getWidth					(void);
	int				getHeight					(void);
	void			setRasterizer				(cRasterizer *rasterizer);
	cRasterizer*	getRasterizer				(void);

	bool			getDepthTestEnabled			(void) {return glState.mDepthTestEnabled;}
	bool			getDepthWriteMask			(void) {return glState.mWriteEnableDepth;}
	const uint32	getColorWriteMask			(void) const;
	const uint32	getDepthFunc				(void) const;
	
	/////////////////////////////////////////////////////
	////////////// OpenGL ES functions //////////////////
	/////////////////////////////////////////////////////

	// misc
	void			_glEnable(int what);
	void			_glDisable(int what);

	// 2.8 Vertex arrays
	void			_glVertexPointer			(int size,GLenum type, GLsizei stride,const void *ptr);
	void			_glColorPointer				(int size,GLenum type, GLsizei stride,const void *ptr);
	void			_glTexCoordPointer			(int size,GLenum type, GLsizei stride,const void *ptr);
	void			_glDrawArrays				(GLenum mode, int first, GLsizei count);
	void			_glEnableClientState		(GLenum cap);
	void			_glDisableClientState		(GLenum cap);
	
	void			_glVertexAttribPointer		(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
	void			_glEnableVertexAttribArray	(GLuint index);
	void			_glDisableVertexAttribArray	(GLuint index);
	// end Vertex arrays

	// 2.11 Coordinate transformations (OGL ES spec)
	void			_glViewport					(int x, int y, GLsizei w, GLsizei h);
	void			_glMatrixMode				(GLenum mode);
	void			_glLoadMatrixf				(float m[16]);
	void			_glMultMatrixf				(float m[16]);
	void			_glLoadIdentity				(void);
	void			_glRotatef					(float angle, float x, float y, float z);
	void			_glScalef					(float x, float y, float z);
	void			_glTranslatef				(float x,float y, float z);
	void			_glFrustum					(double l, double r, double b, double t, double n, double f);
	void			_glOrtho					(double l, double r, double b, double t, double n, double f);
	void			_glPushMatrix				(void);
	void			_glPopMatrix				(void);
	// end Coordinate transformations

	// 4.1 Per-Fragment Operations and the Framebuffer
	void			_glDepthFunc				(GLenum func);
	void			_glDepthMask				(GLboolean write_depth_enable);

	// 4.2 Whole frame buffer operations
	void			_glClearColor				(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void			_glClear					(GLbitfield clearBits);
	void			_glColorMask				(bool red, bool green, bool blue, bool alpha);

	//3.8 Texturing
	void			_glTexImage2D				(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
	void			_glTexParameteri			(GLenum target, GLenum pname, GLenum param);
	void			_glBindTexture				(GLenum target, GLuint texture);
	void			_glDeleteTextures			(GLsizei n, const GLuint *textures);
	void			_glGenTextures				(GLsizei n, GLuint *textures);
	void			_glActiveTexture			(GLenum texture);

	void			_glutSwapBuffers			(void);

	//X.X Fragment and Vertex Programs
	void			_glSetVertexProgram( void (*VertexProgram)(cVertex &vertex,const cGLState *glState) );
	void			_glSetFragmentProgram( Color4f (*FragmentProgram)(const cFragment &fragment,const cGLState *glState) );

protected:
	bool			mLURActive;					// if false, then use standard OpenGL for everything!			
	cRasterizer		*mRasterizer;

	const static int MAX_CLIP_VERTS = 16;
	int				mNumGenClipVerts;
	cVertex			mGenClipVerts[MAX_CLIP_VERTS];

	void			defaultVertexProgram(cVertex &vertex);
	void			clipReset();
	void			clipByPlane(int &nVerts,cVertex **Verts, const Vec4f &pn, float pd);
	void			processTriangles(GLsizei first, GLsizei vertex_count);
};

#endif
