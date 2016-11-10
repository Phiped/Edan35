// -------------------------------------------------------------------------
// File:    TexturedQuadScene.h
// Desc:    a very simple test scene: two triangles composing a quad with
//			a texture on it
//
// Author:  Tomas Akenine-Möller
// History: August, 2005
// -------------------------------------------------------------------------

#ifndef C_TEXTURED_QUAD_SCENE_H
#define C_TEXTURED_QUAD_SCENE_H

#include "scene.h"

class cTexturedQuadScene: public cScene
{
public:
//	cTexturedQuadScene(cRendererAPI *api):cScene(api);
	cTexturedQuadScene(cRendererAPI *api);
	void		clearScreenAndInitFrame(bsCamera *camera);
	bool		renderFrame(void);
	void		firstFrameInit(bsCamera *camera);
	void		cleanup(void);			// called when exiting the program
	bool		setup(void);			// should be called once -- load textures etc here
protected:
	GLuint		mTexnum;
};

#endif
