// -------------------------------------------------------------------------
// File:    UmbrellaScene.h
// Desc:    test scene for detecting cracks between triangles
//
// Author:  Tomas Akenine-Möller
// History: September, 2005
// -------------------------------------------------------------------------

#include "GL/glut.h"

#include "UmbrellaScene.h"
#include "statistics.h"

#include "mmgr/mmgr.h"

cUmbrellaScene::cUmbrellaScene(cRendererAPI *api,int numtris):cScene(api)
{
	strcpy_s(mSceneName,"Umbrella");
	mNumTris=numtris;
	mVertices=new GLfloat[3*3*mNumTris];
	mColors=new GLubyte[3*3*mNumTris];
}

void cUmbrellaScene::setVertex(int num,float x,float y,float z)
{
	mVertices[3*num+0]=x;
	mVertices[3*num+1]=y;
	mVertices[3*num+2]=z;
}

void cUmbrellaScene::setColor(int num,GLubyte r,GLubyte g,GLubyte b)
{
	mColors[3*num+0]=r;
	mColors[3*num+1]=g;
	mColors[3*num+2]=b;
}

bool cUmbrellaScene::setup(void)
{
	printf("Executing setup code for <%s> scene\n",mSceneName);
	printf("==========================================================\n");
	setTimes(0.0f,10.0f,100);
	const float radius=1.0f;
	float angle;
	for(int q=0;q<mNumTris;q++)
	{
		setVertex(3*q+0,0.0f,0.0f,0.0f);
		angle=q*2.0f*M_PI/mNumTris;	
		setVertex(3*q+1,radius*cos(angle),radius*sin(angle),0.0f);
		angle=(q+1)*2.0f*M_PI/mNumTris;	
		setVertex(3*q+2,radius*cos(angle),radius*sin(angle),0.0f);

		GLubyte r=255-rand()%128;
		GLubyte g=255-rand()%128;
		GLubyte b=255-rand()%128;
		setColor(3*q+0,r,g,b);
		setColor(3*q+1,r,g,b);
		setColor(3*q+2,r,g,b);
	}
	printf("==========================================================\n");
	return true;
}

void cUmbrellaScene::cleanup(void)
{
	delete [] mColors;
	delete [] mVertices;
}

void cUmbrellaScene::firstFrameInit(bsCamera *camera)
{
	camera->setPosition(Vec3f(0,0,3));
	camera->setUpVector(Vec3f(0,1,0));
	camera->setLookAt(Vec3f(0,0,0));
	camera->setVFOV(50);
	camera->setNearFar(0.1,10000);
}

void cUmbrellaScene::clearScreenAndInitFrame(bsCamera *camera)
{
	mAPI->_glClearColor(0.0,0.0,0.0,1);
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


bool cUmbrellaScene::renderFrame(void)
{
	int winwidth=mAPI->getWidth();
	int winheight=mAPI->getHeight();

	bool end_anim=stepAnimation();

	glPushMatrix();
	if(mAnimationOn)
	{
		glTranslatef(0.2f*mFrameNo/mNumFrames,0,0);
//		glRotatef(5.0f*mFrameNo/mNumFrames,0,0,1);
	}

	// set state
	mAPI->_glDisable(GL_TEXTURE_2D);
	//mAPI->_glBindTexture(GL_TEXTURE_2D, mTexnum);

	// render simple scene
	mAPI->_glEnableClientState(GL_VERTEX_ARRAY);
	mAPI->_glEnableClientState(GL_COLOR_ARRAY);
	//mAPI->_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	mAPI->_glVertexPointer(3, GL_FLOAT, 0, mVertices);
	mAPI->_glColorPointer(3,GL_UNSIGNED_BYTE,0,mColors);
	//mAPI->_glTexCoordPointer(2,GL_FLOAT,0,texcoords);
	mAPI->_glDrawArrays(GL_TRIANGLES, 0, 3*mNumTris);	

	glPopMatrix();

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





