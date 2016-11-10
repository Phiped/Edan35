// -------------------------------------------------------------------------
// File:    TextureScene.h
// Desc:    a very simple test scene: two triangles composing a quad with
//			a texture on it
//
// Author:  Tomas Akenine-Möller
// History: August, 2005
// -------------------------------------------------------------------------

#ifndef C_TEXTURE_SCENE_H
#define C_TEXTURE_SCENE_H

#include "scene.h"

class cTextureScene: public cScene
{
public:
//	cTextureScene(cRendererAPI *api):cScene(api);
	cTextureScene(cRendererAPI *api);
	void		clearScreenAndInitFrame(bsCamera *camera);
	bool		renderFrame(void);
	void		firstFrameInit(bsCamera *camera);
	void		cleanup(void);			// called when exiting the program
	bool		setup(void);			// should be called once -- load textures etc here
protected:
	GLuint		mTexnum;
};

#endif
