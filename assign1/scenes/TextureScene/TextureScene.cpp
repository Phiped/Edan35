// -------------------------------------------------------------------------
// File:    TextureScene.cpp
// Desc:    a very simple test scene: two triangles composing a quad with
//			a texture on it
//
// Author:  Tomas Akenine-Möller
// History: August, 2005
// -------------------------------------------------------------------------

#include "GL/glut.h"

#include "TextureScene.h"
#include "statistics.h"

#include "mmgr/mmgr.h"

static GLfloat vertices[] = {
	-1.0f, 0, -1.0f,
	 1.0f, 0, -1.0f,
	 1.0f, 0,  1.0f,
	-1.0f, 0, -1.0f,
	 1.0f, 0,  1.0f,
	-1.0f, 0,  1.0f,
};

static GLfloat texcoords[] = {
	0, 0,
	1, 0,
	1, 1,
	0, 0,
	1, 1,
	0, 1,
};

static GLubyte colors [] = {
	255,255,255,255,
	255,255,255,255,
	255,255,255,255,
	255,255,255,255,
	255,255,255,255,
	255,255,255,255
};

cTextureScene::cTextureScene(cRendererAPI *api):cScene(api)
{
	strcpy_s(mSceneName,"Texture");
}


bool cTextureScene::setup(void)
{
	printf("Executing setup code for <%s> scene\n",mSceneName);
	printf("==========================================================\n");
	setTimes(0.0f,2.0f,20);

	if(!loadTextureBuildMipmap("textures","test.bmp",mTexnum))
	{
		printf("Exiting...\n"); exit(0);
	}

	printf("==========================================================\n");
	return true;
}

void cTextureScene::cleanup(void)
{
}

void cTextureScene::firstFrameInit(bsCamera *camera)
{
	camera->setPosition(Vec3f(0,0.5,3));
	camera->setUpVector(Vec3f(0,1,0));
	camera->setLookAt(Vec3f(0,0,0));
	camera->setVFOV(50);
	camera->setNearFar(0.1,10000);
}

void cTextureScene::clearScreenAndInitFrame(bsCamera *camera)
{
	mAPI->_glClearColor(0.6,0.4,0.9,1);
	mAPI->_glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	mAPI->_glDepthFunc(GL_LESS);
	mAPI->_glEnable(GL_DEPTH_TEST);

	if(mAnimationOn)
	{
		float rad=0.4f*mFrameNo*2*3.14159/mNumFrames;
		camera->setPosition(Vec3f(3*sin(rad),0.5,3*cos(rad)));
		camera->setUpVector(Vec3f(0,1,0));
		camera->setLookAt(Vec3f(0,0,0));
		camera->setVFOV(50);
		camera->setNearFar(0.1,10000);
	}
}


bool cTextureScene::renderFrame(void)
{
	int winwidth=mAPI->getWidth();
	int winheight=mAPI->getHeight();

	bool end_anim=stepAnimation();

//	printf("frame: %d \n",mFrameNo);

	// set state
	mAPI->_glEnable(GL_TEXTURE_2D);
	mAPI->_glBindTexture(GL_TEXTURE_2D, mTexnum);

	// render simple scene
	mAPI->_glEnableClientState(GL_VERTEX_ARRAY);
	mAPI->_glEnableClientState(GL_COLOR_ARRAY);
	mAPI->_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	mAPI->_glVertexPointer(3, GL_FLOAT, 0, vertices);
	mAPI->_glColorPointer(4,GL_UNSIGNED_BYTE,0,colors);
	mAPI->_glTexCoordPointer(2,GL_FLOAT,0,texcoords);
	mAPI->_glDrawArrays(GL_TRIANGLES, 0, 6);	

	cScene::renderFrame();						// this allows for saving of frames...

	if(mAnimationOn==false) return false;
	else if(end_anim)
	{
		statistics::print();
		resetAnimation();
		return false;
	}
	else
	{
		glutPostRedisplay();
		return true;
	}
}





