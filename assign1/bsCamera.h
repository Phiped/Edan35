// -------------------------------------------------------------------------
// File:    bsCamera
// Desc:    base (bs) class for a camera (position, view direction, 
//          upvec, FOV, viewport, etc)
//
// Author:  Tomas Akenine-Möller
// History: March,   2000 (started)
//          October, 2003 (cleaned code, added simple trackball)
//-------------------------------------------------------------------------

#ifndef BS_CAMERA_H
#define BS_CAMERA_H

#include "vecmath.h"
//#include "animation.h"

typedef struct 
{
	bool mShiftDown;
	bool mCtrlDown;
	bool mAltDown;
	int mMouseX,mMouseY,mPrevMouseX,mPrevMouseY;
	bool mMouseDown,mPrevMouseDown;
	int mMouseButton;
} bsMouseKeyInfo;

class bsCamera
{
public:
					bsCamera();
					~bsCamera();
	void			setPosition(const Vec3f);
	void			setUpVector(const Vec3r up);
	void			setLookAt(const Vec3r lookat);
	void			setVFOV(float vfov);
	void			setResolution(int width,int height);
	void			setNearFar(float n,float f);
	void			getNearFar(float &near_value,float &far_value) const;
	float			getNear(void) const;
	float			getFar(void) const;
	Vec3f			getPosition(void) const;
	Vec3f			getDirection(void) const;
	Vec3f			getUpVector(void) const;
	Vec3f			getLookAt(void) const;
	float			getVFOV(void) const;
	int				getWidth(void) const;
	int				getHeight(void) const;
	void			getTracerParams(Vec3r &campos, Vec3r &lower_left,Vec3r &deltax,Vec3r &deltay);
	void			setOGLMatrices(void) const;
	void			getModel2ScreenMatrix(Mtx4f &m) const;
	void			getModel2ClipMatrix(Mtx4f &m) const;
	void			getModel2EyeMatrix(Mtx4f &m) const;
	void			getScreen2ModelMatrix(Mtx4r &m) const;
	void			getViewportMatrix(Mtx4f &m) const;
	void			getCameraMatrix(Mtx4f &p) const;
	void			debugprint(void) const;
	void			enableTrackBall(void);
	void			disableTrackBall(void);
	void			recordMouse(int button, int state, int x, int y,bool shift,bool ctrl,bool alt);
	void			recordMotion(int x,int y);
	void			postRecordMotion(void);
	int				getDeltaMouseX(void);
	int				getDeltaMouseY(void);
//	void			setAnimation(Animation *a) {mAnimation=a;}
	void			setAnimationParams(float start,float end, int numframes);
	bool			nextFrame(void);
	float			getTime(void) {return mCurrentTime;};
	void			animate(void);
	virtual void	move(void);					// move with respect to mouse movements etc: override to implement own stuff
protected:
	void			precomputeTracerParams(void);
	Vec3f			mPosition;
	Vec3f			mDirection;  // view direction
	Vec3f			mUpVector;
	Vec3f			mLookAt;     // this one is the best to give
	float			mVFOV;       // vertical FOV (as in OpenGL)   
	int				mWidth;
	int				mHeight;
	bool			mPreComputed;  // true if the following parameters are valid:
	Vec3f			mLowerLeft;   // the center of the lower left pixel on the screen
	Vec3f			mDeltaX;      // delta vector for one pixel in x-dir
	Vec3f			mDeltaY;      // delta vector for one pixel in y-dir
	float			mNear,mFar;
	bool			mTrackBallEnabled;
	bsMouseKeyInfo  mMouseKeyInfo;
//	Animation		*mAnimation;
	float			mStartAnimTime;
	float			mEndAnimTime;
	float			mCurrentTime;
	float			mDeltaTime;
	int				mNumFrames;
	int				mCurrentFrame;
};

#endif  //BS_CAMERA_H
