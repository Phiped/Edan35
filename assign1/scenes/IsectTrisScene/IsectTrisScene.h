// -------------------------------------------------------------------------
// File:    IsectTrisScene.h
// Desc:    a very simple test scene: two intersecting triangles
//
// Author:  Tomas Akenine-Möller
// History: August, 2005
// -------------------------------------------------------------------------

#ifndef C_ISECT_TRIS_SCENE_H
#define C_ISECT_TRIS_SCENE_H

#include "scene.h"

class cIsectTrisScene: public cScene
{
public:
	cIsectTrisScene(cRendererAPI *api);
	void		clearScreenAndInitFrame(bsCamera *camera);
	bool		renderFrame(void);
	void		firstFrameInit(bsCamera *camera);
	void		cleanup(void);			// called when exiting the program
	bool		setup(void);			// should be called once -- load textures etc here
protected:
	GLuint		mTexnum;
};

#endif
