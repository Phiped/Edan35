// -------------------------------------------------------------------------
// File:    ProjectScene.h
// Desc:    scene that will be used in project. it is empty now, will include
//			more stuff later on.
//
// Author:  Tomas Akenine-Möller
// History: September, 2005
// -------------------------------------------------------------------------

#ifndef C_PROJECT_SCENE_H
#define C_PROJECT_SCENE_H

#include "scene.h"
#include "misc.h"

class cProjectScene : public cScene
{
public:
	cProjectScene(cRendererAPI *api);
	void		clearScreenAndInitFrame(bsCamera *camera);
	bool		renderFrame(void);
	void		firstFrameInit(bsCamera *camera);
	void		cleanup(void);			// called when exiting the program
	bool		setup(void);			// should be called once -- load textures etc here
protected:
};

#endif
