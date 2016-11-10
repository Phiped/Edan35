// -------------------------------------------------------------------------
// File:    bsCamera
// Desc:    base (bs) class for a camera (position, view direction, 
//          upvec, FOV, viewport, etc)
//			The idea is that this class could be used both in an OpenGL app,
//			but also in a ray tracer, for example.
//
// Author:  Tomas Akenine-Möller
// History: March,   2000 (started)
//          October, 2003 (cleaned code, added simple trackball)
//-------------------------------------------------------------------------

#include "GL/glut.h"
#include "misc.h"
#include "bsCamera.h"

#include "mmgr/mmgr.h"

bsCamera::bsCamera()
{
	mPosition.set(0,0,3.5);
	mDirection.set(0,0,-1);
	mLookAt.set(0,0,0);
	mUpVector.set(0,1,0);
	mVFOV=45.0;
	mWidth=512;
	mHeight=512;
	mNear=1.0f;
	mFar=5.0f;
	mPreComputed=false;
	mTrackBallEnabled=true;
//	mAnimation=NULL;
	mCurrentFrame=0;
	mNumFrames=0;					// implies no animation of camera
	mStartAnimTime=0.0f;
	mEndAnimTime=0.0f;
	mDeltaTime=0.0f;
	mCurrentTime=0.0f;
	precomputeTracerParams();
}

bsCamera::~bsCamera()
{
}

void bsCamera::setPosition(const Vec3f pos)
{
  mPosition=pos;
  mDirection=mLookAt-mPosition;		 // compute new direction
  mDirection.normalize();
  mPreComputed=false;
}


void bsCamera::setUpVector(const Vec3f up)
{
   mUpVector=up;
   mUpVector.normalize();
   ASSERT(mUpVector*mUpVector>fEPSILON);
   mPreComputed=false;
}

void bsCamera::setLookAt(const Vec3f lookat)
{
   mLookAt=lookat;
   mDirection=mLookAt-mPosition;  // compute new direction
   mDirection.normalize();
   mPreComputed=false;
}


void bsCamera::setVFOV(float vfov)
{
   mVFOV=vfov;
   mPreComputed=false;
}

void bsCamera::setResolution(int width,int height)
{
   mWidth=width;
   mHeight=height;
   mPreComputed=false;
}

Vec3f bsCamera::getPosition(void) const
{
   return mPosition;
}

Vec3f bsCamera::getDirection(void) const
{
   return mDirection;
}

Vec3f bsCamera::getUpVector(void) const
{
   return mUpVector;
}

Vec3f bsCamera::getLookAt(void) const
{
   return mLookAt;
}

float bsCamera::getVFOV(void) const
{
   return mVFOV;
}

int bsCamera::getWidth(void) const
{
   return mWidth;
}

int bsCamera::getHeight(void) const
{
   return mHeight;
}

void bsCamera::setNearFar(float near_value,float far_value)
{
   mNear=near_value;
   mFar=far_value;
}

void bsCamera::getNearFar(float &near_value,float &far_value) const
{
   near_value=mNear;
   far_value=mFar;
}

float bsCamera::getNear(void) const
{
   return mNear;
}

float bsCamera::getFar(void) const
{
   return mFar;
}

void bsCamera::precomputeTracerParams(void)
{
	float height,width;
	Vec3f xdir,ydir;

	xdir=mDirection%mUpVector;
	xdir.normalize();
	ydir=xdir%mDirection;
	ydir.normalize();

	height=2.0*tan(mVFOV*TORAD/2.0);
	ASSERT(mHeight>0);
	mDeltaY=ydir*(height/mHeight);

	width=2.0*(float)mWidth/(float)mHeight*tan(mVFOV*TORAD/2.0);
	ASSERT(mWidth>0);
	mDeltaX=xdir*(width/mWidth);

	mLowerLeft=mPosition+mDirection-
		mDeltaX*((mWidth-1.0)/2.0)-
		mDeltaY*((mHeight-1.0)/2.0);

	mPreComputed=true;
}

void bsCamera::getTracerParams(Vec3f &campos, Vec3f &lower_left,Vec3f &deltax,Vec3f &deltay)
{
   if(!mPreComputed)
   {
      precomputeTracerParams();
   }
   campos=mPosition;
   lower_left=mLowerLeft;
   deltax=mDeltaX;
   deltay=mDeltaY;
}


void bsCamera::setOGLMatrices(void) const  // set up OpenGL matrices
{
	glViewport(0, 0, mWidth, mHeight);		// might want to add support for other viewport sizes later on
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective((real)mVFOV,
                  (real)mWidth/mHeight,		// aspect ratio: I believe this is the right way to do it.
                  (real)mNear,(real)mFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(mPosition[0],mPosition[1],mPosition[2],
			  mLookAt[0],mLookAt[1],mLookAt[2],
              mUpVector[0],mUpVector[1],mUpVector[2]);
}

void bsCamera::getViewportMatrix(Mtx4f &m) const		// assumes entire window is used
{
	float whalf=mWidth*0.5f;
	float hhalf=mHeight*0.5f;
	m.set(whalf, 0,     0,   whalf,
		0,     hhalf, 0,   hhalf,
		0,     0,     0.5, 0.5,
		0,     0,     0,   1);
}

void bsCamera::getModel2ScreenMatrix(Mtx4f &m) const
{   
	Mtx4f mv,pr,vp;  // modelview, projection, viewport
	
	// set matrices
//	setOGLMatrices();

	// get matrices		
	glGetFloatv(GL_MODELVIEW_MATRIX,mv.array);
	glGetFloatv(GL_PROJECTION_MATRIX,pr.array);
	getViewportMatrix(vp);     
	m=vp*(pr*mv);
}

void bsCamera::getModel2ClipMatrix(Mtx4f &m) const
{   
	Mtx4f mv,pr;  // modelview, projection
	
	// get matrices		
	glGetFloatv(GL_MODELVIEW_MATRIX,mv.array);
	glGetFloatv(GL_PROJECTION_MATRIX,pr.array);
	m=(pr*mv);
}

void bsCamera::getModel2EyeMatrix(Mtx4f &m) const
{
	glGetFloatv(GL_MODELVIEW_MATRIX,m.array);
}

void bsCamera::getCameraMatrix(Mtx4f &p) const
{
	glGetFloatv(GL_PROJECTION_MATRIX,p.array);
}

void bsCamera::getScreen2ModelMatrix(Mtx4r &m) const
{
	getModel2ScreenMatrix(m);
	bool result=m.invert();
	if(result==false)
	{
		printf("Err: bsCamera -- could not invert Screen2Model matrix\n");
	}
}

void bsCamera::enableTrackBall(void)
{
	mTrackBallEnabled=true;
}

void bsCamera::disableTrackBall(void)
{
	mTrackBallEnabled=false;
}

void bsCamera::recordMouse(int button, int state, int x, int y,bool shift,bool ctrl,bool alt)
{
	mMouseKeyInfo.mPrevMouseDown=mMouseKeyInfo.mMouseDown;	
	mMouseKeyInfo.mMouseDown=state==GLUT_UP ? false : true;
	mMouseKeyInfo.mMouseX=x;
	mMouseKeyInfo.mMouseY=mHeight-y-1;
	mMouseKeyInfo.mPrevMouseX=mMouseKeyInfo.mMouseX;
	mMouseKeyInfo.mPrevMouseY=mMouseKeyInfo.mMouseY;
	mMouseKeyInfo.mMouseButton=button;					// GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON
	mMouseKeyInfo.mShiftDown=shift;
	mMouseKeyInfo.mCtrlDown=ctrl;
	mMouseKeyInfo.mAltDown=alt;
	//	printf("Mouse coords: %d %d\n",x,mWinHeight-y-1);
}

void bsCamera::recordMotion(int x,int y)
{
	Vec3f campos,lookat,up;
	mMouseKeyInfo.mPrevMouseDown=mMouseKeyInfo.mMouseDown;
	mMouseKeyInfo.mMouseX=x;
	mMouseKeyInfo.mMouseY=mHeight-y-1;	
}

void bsCamera::postRecordMotion(void)
{
	mMouseKeyInfo.mPrevMouseX=mMouseKeyInfo.mMouseX;
	mMouseKeyInfo.mPrevMouseY=mMouseKeyInfo.mMouseY;	
}

int bsCamera::getDeltaMouseX(void)
{
	return mMouseKeyInfo.mMouseX-mMouseKeyInfo.mPrevMouseX;
}

int bsCamera::getDeltaMouseY(void)
{
	return mMouseKeyInfo.mMouseY-mMouseKeyInfo.mPrevMouseY;
}

void bsCamera::move(void)
{
	if(!mTrackBallEnabled) return;
	if(mMouseKeyInfo.mMouseButton==GLUT_LEFT_BUTTON)
	{
		Mtx3f mtx,mtx2;
		Vec3f tmpvec,tmpright,tmpup;
		Vec3f camdir,camright;
		camdir=mLookAt-mPosition;
		camdir.normalize();
		if(mMouseKeyInfo.mShiftDown)
		{
			float delta;
			delta=(mMouseKeyInfo.mMouseY-mMouseKeyInfo.mPrevMouseY)*0.04;
			delta*=delta;
			if(mMouseKeyInfo.mMouseY-mMouseKeyInfo.mPrevMouseY<0) delta=-delta;
			mPosition+=camdir*delta;
		}
		else if(mMouseKeyInfo.mCtrlDown)
		{
			camright=camdir%mUpVector;
			camright.normalize();      
			float delta;
			delta=(mMouseKeyInfo.mMouseY-mMouseKeyInfo.mPrevMouseY)*0.04;
			delta*=delta;
			if(mMouseKeyInfo.mMouseY-mMouseKeyInfo.mPrevMouseY<0) delta=-delta;
			mPosition+=mUpVector*delta;
			mLookAt+=mUpVector*delta;
			delta=(mMouseKeyInfo.mMouseX-mMouseKeyInfo.mPrevMouseX)*0.04;
			delta*=delta;
			if(mMouseKeyInfo.mMouseX-mMouseKeyInfo.mPrevMouseX<0) delta=-delta;
			mPosition+=camright*delta;
			mLookAt+=camright*delta;
		}
		else
		{
			// rotate real camera (always looks at (0,0,0))
			// first rotate around up-axis
			// a really simple navigator: at some point this should be improved upon

			real rad=(mMouseKeyInfo.mPrevMouseX-mMouseKeyInfo.mMouseX)*0.004;
			camright=camdir%mUpVector;
			camright.normalize();

			mtx.rotAxis(mUpVector,rad);
			tmpvec=mtx*mPosition;
			tmpright=mtx*camright;

			rad=-(mMouseKeyInfo.mPrevMouseY-mMouseKeyInfo.mMouseY)*0.004;
			mtx2.rotAxis(tmpright,rad);
			mPosition=mtx2*tmpvec;
			camright=tmpright;
			tmpup=mtx2*mUpVector;
			mUpVector=tmpup;
		}
		mDirection=mLookAt-mPosition;
		mDirection.normalize();
		mPreComputed=false;
		glutPostRedisplay();
	}
}

void bsCamera::setAnimationParams(float start,float end, int numframes)
{
	mStartAnimTime=start;
	mEndAnimTime=end;
	mNumFrames=numframes;
	mCurrentFrame=0;
	mCurrentTime=mStartAnimTime;
	if(numframes<1) mDeltaTime=mEndAnimTime-mStartAnimTime;
	else			mDeltaTime=(mEndAnimTime-mStartAnimTime)/(mNumFrames-1);
}

void bsCamera::animate(void)
{
#if 0
	if(mNumFrames>0 && mAnimation)
	{
		double pos[3],dir[3],up[3];
		int gotpos, gotdir;
		GetCamera(mAnimation,mCurrentTime,&gotpos,pos,&gotdir,dir,up);
		if(gotpos) setPosition(Vec3f(pos[0],pos[1],pos[2]));
		if(gotdir)
		{
			setLookAt(Vec3f(pos[0]+dir[0],pos[1]+dir[1],pos[2]+dir[2]));
			setUpVector(Vec3f(up[0],up[1],up[2]));
		}
	}
#endif
}

bool bsCamera::nextFrame(void)
{
	if(mNumFrames==0) return true;		// there is no animation of the camera, just return true
	mCurrentFrame++;
	if(mCurrentFrame>=mNumFrames)		// end of animation...
	{
		return false;
	}
	mCurrentTime=mStartAnimTime + mCurrentFrame*mDeltaTime;
	printf("Current time: %2.2f\n",mCurrentTime);
	return true;
}

void bsCamera::debugprint(void) const
{
   printf("Camera:\n");
   printf(" from: "); mPosition.debugprint();
   printf("   at: "); mLookAt.debugprint();
   printf("  dir: "); mDirection.debugprint();
   printf("   up: "); mUpVector.debugprint();

   printf(" near: %f\n",mNear);
   printf("  far: %f\n",mFar);
   printf(" VFOV: %f\n",mVFOV);
   printf("  res: %d x %d\n",mWidth,mHeight);
}

