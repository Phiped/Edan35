// -------------------------------------------------------------------------
// File:    scene.h
// Desc:    a class that is supposed to hold a scene to be rendered
//			inherit from this one, and add stuff for your scene
//
// Author:  Tomas Akenine-Möller
// History: September, 2002		Started
//			August, 2005		Adapated to LUR, fixed small things	
// -------------------------------------------------------------------------

#ifndef C_SCENE_H
#define C_SCENE_H

#include "bsCamera.h"
#include "rendererAPI.h"

class cScene
{
public:
					cScene(cRendererAPI *api);
	void			setTimes(float start,float stop,int num_frames);
	void			toggleAnimationOnOff(void);
	void			setSaveFrames(bool save);
	bool			loadTextureBuildMipmap(char *path,char *texturename,GLuint &texnum);
	bool			getSaveFrames(void);
	virtual void	startAnimation(void) ;
	virtual void	stopAnimation(void);
	virtual void	resetAnimation(void);

	virtual void	clearScreenAndInitFrame(bsCamera *camera);
	virtual bool	renderFrame(void);
	virtual void	firstFrameInit(bsCamera *camera);	// called when we change to this scene...
	virtual	bool	setup(void);						// should be called once -- load textures etc here
	virtual void	cleanup(void);						// when exiting progam -- freeing mem etc
protected:
	virtual bool	stepAnimation(void);
	bool			mAnimationOn;
	bool			mSaveFrames;
	float			mTime;
	float			mDeltaTime;
	float			mStartTime;
	float			mStopTime;
	int				mNumFrames;
	int				mFrameNo;
	cRendererAPI	*mAPI;
	char			mSceneName[100];
};

#endif
