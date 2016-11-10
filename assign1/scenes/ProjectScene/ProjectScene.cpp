// -------------------------------------------------------------------------
// File:    ProjectScene.h
// Desc:    scene that will be used in project. it is empty now, will include
//			more stuff later on.
//
// Author:  Tomas Akenine-Möller
// History: September, 2005
// -------------------------------------------------------------------------

#include "GL/glut.h"

#include "ProjectScene.h"
#include "statistics.h"

#include "mmgr/mmgr.h"

cProjectScene::cProjectScene(cRendererAPI *api):cScene(api)
{
	strcpy_s(mSceneName,"Project");
}


bool cProjectScene::setup(void)
{
	printf("Executing setup code for <%s> scene\n",mSceneName);
	printf("==========================================================\n");
	setTimes(0.0f,10.0f,100);
	printf("==========================================================\n");
	return true;
}

void cProjectScene::cleanup(void)
{
}

void cProjectScene::firstFrameInit(bsCamera *camera)
{
	camera->setPosition(Vec3f(0,0,3));
	camera->setUpVector(Vec3f(0,1,0));
	camera->setLookAt(Vec3f(0,0,0));
	camera->setVFOV(50);
	camera->setNearFar(0.1,10000);
}

void cProjectScene::clearScreenAndInitFrame(bsCamera *camera)
{
	mAPI->_glClearColor(0.0,1.0,0.0,1);
	mAPI->_glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	mAPI->_glDepthFunc(GL_LESS);
	mAPI->_glEnable(GL_DEPTH_TEST);

	if(mAnimationOn)
	{
		camera->setPosition(Vec3f(0,0,3));
		camera->setUpVector(Vec3f(0,1,0));
		camera->setLookAt(Vec3f(0,0,0));
		camera->setVFOV(50);
		camera->setNearFar(1,5);
	}
}


bool cProjectScene::renderFrame(void)
{
	int winwidth=mAPI->getWidth();
	int winheight=mAPI->getHeight();

	bool end_anim=stepAnimation();

	if(mAnimationOn)
	{
	}


	cScene::renderFrame();

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





