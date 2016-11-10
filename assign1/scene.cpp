// -------------------------------------------------------------------------
// File:    scene.h
// Desc:    a class that is supposed to hold a scene to be rendered
//			inherit from this one, and add stuff for your scene
//
// Author:  Tomas Akenine-Möller
// History: September, 2002		Started
//			August, 2005		Adapated to LUR, fixed small things	
// -------------------------------------------------------------------------

#include "GL/glut.h"
#include <stdio.h>
#include <stdlib.h>
#include "il/il.h"
#include "il/ilut.h"

#include "scene.h"
#include "misc.h"
#include "statistics.h"

#include "mmgr/mmgr.h"

cScene::cScene(cRendererAPI *api)
{
	// some default values...
	mStartTime=mTime=0.0f;
	mStopTime=1000.0f;
	mDeltaTime=0.1f;
	mAnimationOn=false;
	mFrameNo=0;
	mSaveFrames=false;
	mAPI=api;
	strcpy_s(mSceneName,"cScene");
}

void cScene::setTimes(float start,float stop,int num_frames)
{
	mStartTime=start;
	mStopTime=stop;
	ASSERT(mStopTime>mStartTime);
	ASSERT(num_frames>=1),
	mNumFrames=num_frames;
	if(mNumFrames==1) mDeltaTime=mStopTime-mStartTime;
	else			  mDeltaTime=(mStopTime-mStopTime)/mNumFrames;
}

void cScene::startAnimation(void)
{
	mAnimationOn=true;
}

void cScene::stopAnimation(void)
{
	mAnimationOn=false;
}

void cScene::toggleAnimationOnOff(void)
{
	mAnimationOn=!mAnimationOn;
}

void cScene::resetAnimation(void)
{
	mTime=mStartTime;
	mAnimationOn=false;
	mFrameNo=0;
	mSaveFrames=false;
}

void cScene::setSaveFrames(bool save)
{
	mSaveFrames=save;
}

bool cScene::getSaveFrames(void)
{
	return mSaveFrames;
}

bool cScene::stepAnimation(void)
{
	if(mAnimationOn)
	{
		if(mFrameNo==0) statistics::clear();
		mTime+=mDeltaTime;
		mFrameNo++;
		if(mFrameNo>=mNumFrames)
		{
			mSaveFrames=false;
			return true;
		}
		else return false;
	}
	return false;
}

bool cScene::loadTextureBuildMipmap(char *path,char *texturename,GLuint &texnum)
{
	ilInit();

	GLuint tnum = 0;
	mAPI->_glGenTextures(1,&tnum);
	texnum = tnum;
	printf("texnum=%d\n",texnum);

	ILuint image;
	ilGenImages( 1, &image );
	ilBindImage( image );
	
	printf("<%s> Loading texture <%s>...",mSceneName,texturename); fflush(stdout);
	char name[200];
	sprintf_s(name,"%s/%s",path,texturename);
	if (!ilLoadImage(name)) {
		printf("Failed to load texture <%s>.\n",name);
		return false;
	}

	// Build mipmaps and upload them
	iluBuildMipmaps();
	int nMipmaps = ilGetInteger(IL_NUM_MIPMAPS);
	for (int i = 0; i <= nMipmaps; i++) {
		ilBindImage( image );	// HAXY, does il use opengl for this???
		ilActiveMipmap(i);
		int w = ilGetInteger(IL_IMAGE_HEIGHT);
		int h = ilGetInteger(IL_IMAGE_HEIGHT);

		void *data = malloc(w*h*sizeof(unsigned char)*4);
		ilCopyPixels(0,0,0,w,h,1,GL_RGBA,GL_UNSIGNED_BYTE,data);

		mAPI->_glBindTexture(GL_TEXTURE_2D,texnum);
		mAPI->_glTexImage2D(GL_TEXTURE_2D,i,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
//		mAPI->_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		mAPI->_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		free(data);
	}

	ilDeleteImages(1, &image);

	printf("OK\n");
	return true;
}


// should clear screen and possibly init some state before starting on a new frame
void cScene::clearScreenAndInitFrame(bsCamera *camera)
{
	mAPI->_glClearColor(0.0,0.0,0.0,1);
	mAPI->_glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	mAPI->_glDepthFunc(GL_LESS);
	mAPI->_glEnable(GL_DEPTH_TEST);
}

void cScene::firstFrameInit(bsCamera *camera)
{
}

bool cScene::renderFrame(void)
{
	if(mSaveFrames)
	{
		int width,height;
		width=mAPI->getWidth();
		height=mAPI->getHeight();

		char filename[100],command[500];
		sprintf_s(filename,"%05d.ppm",mFrameNo);
		unsigned char* buffer = new unsigned char[4*width*height];

		printf("Saving <%s>...",filename); fflush(stdout); 
		// read back frame buffer and save as a ppm image file

		glReadBuffer(GL_FRONT);
		glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,buffer);

//		fWritePPMfromRGBA(filename, width, height, buffer, true);

		// then convert to another format (JPG,BMP), and delete old file
		sprintf_s(command,"convert %s  %05d.bmp",filename,mFrameNo);
		system(command);	
		sprintf_s(command,"del %s",filename);
		system(command);
		printf("OK\n");		
		delete [] buffer;
	}
	return false;
}

void cScene::cleanup(void)
{

}

bool cScene::setup(void)
{
	return true;
}