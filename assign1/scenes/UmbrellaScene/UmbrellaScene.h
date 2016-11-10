// -------------------------------------------------------------------------
// File:    UmbrellaScene.h
// Desc:    test scene for detecting cracks between triangles
//
// Author:  Tomas Akenine-Möller
// History: September, 2005
// -------------------------------------------------------------------------

#ifndef C_UMBRELLA_SCENE_H
#define C_UMBRELLA_SCENE_H

#include "scene.h"
#include "misc.h"

class cUmbrellaScene : public cScene
{
public:
	cUmbrellaScene(cRendererAPI *api,int numtris);
	void		clearScreenAndInitFrame(bsCamera *camera);
	bool		renderFrame(void);
	void		firstFrameInit(bsCamera *camera);
	void		cleanup(void);			// called when exiting the program
	bool		setup(void);			// should be called once -- load textures etc here
protected:
	void		setVertex(int num,float x,float y,float z);
	void		setColor(int num,GLubyte r,GLubyte g,GLubyte b);
	GLfloat		*mVertices;
	GLubyte		*mColors;
	int			mNumTris;
};

#endif
