// -------------------------------------------------------------------------
// File:    LUR.cpp 
// Desc:    main file for LUR: Lund University Rasterizer
//
// Author:  Tomas Akenine-Möller
// History: September, 2004 (started)
// -------------------------------------------------------------------------


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GL/glut.h"

#include "rendererAPI.h"
#include "bsCamera.h"
#include "colorunit.h"
#include "depthunit.h"
#include "textureunit.h"
#include "tilerasterizer.h"
#include "statistics.h"
#include "timer.h"

#include "scenes/TexturedQuadScene/TexturedQuadScene.h"
#include "scenes/TextureScene/TextureScene.h"
#include "scenes/IsectTrisScene/IsectTrisScene.h"
#include "scenes/UmbrellaScene/UmbrellaScene.h"
#include "scenes/ProjectScene/ProjectScene.h"

#include "mmgr/mmgr.h"

void cleanExit(void);

static const float	gMaxFrameRate=5.0f;

static cRendererAPI gAPI;
static bool			gRenderUsingOpenGL=false;
static bool			gDisplayStats=false;
static bsCamera     gCamera;
static cRasterizer *gRasterizer;

static GLuint		tex;

static const int	gNumScenes=5;
static cScene		*gScenes[gNumScenes];
static cScene		*gCurrentScene;
static bool			gAnimationOn=false;

enum {MENU_EXIT,MENU_TOGGLE_RASTERIZER,MENU_TOGGLE_ANIMATION,MENU_TOGGLE_DISPLAY_STATS};

static void displayString(char *str,int startx,int starty)
{
	glColor3f(1,1,1);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();   
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, gAPI.getWidth(), 0, gAPI.getWidth());

	glRasterPos2f(startx, starty);

	int len=(int)strlen(str);
	for(int i=0; i<len; i++)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
	glPopMatrix();

	glPopMatrix();
	glPopAttrib();
}

static void displayStuff(void)
{
	glDrawBuffer(GL_FRONT);
	if(gRenderUsingOpenGL)
	{		
		displayString("Rendering: OpenGL",20,60);
	}
	else
	{
		char str[300];
		sprintf_s(str,"Rendering: LUR (%s)",gAPI.getRasterizer()->getClassName());
		displayString(str,20,200);
		if(gDisplayStats)
		{
			sprintf_s(str,"Visited pixels: %ld",statistics::get(STATS_VISITED_PIXELS));
			displayString(str,20,180);
			sprintf_s(str,"Processed pixels: %ld",statistics::get(STATS_PROCESSED_PIXELS));
			displayString(str,20,160);
			sprintf_s(str,"Shaded pixels: %ld",statistics::get(STATS_SHADED_PIXELS));
			displayString(str,20,140);
			// all BW numbers from statistics are in bits
			long long tmp;
			// print depth buffer BW stats
			tmp=(statistics::get(STATS_DEPTH_BUFFER_READ_BW)+statistics::get(STATS_DEPTH_BUFFER_WRITE_BW))>>3L;
	
			__int64 a = statistics::get(STATS_DEPTH_BUFFER_WRITE_BW)>>3L;
			sprintf_s(str,"Depth buffer BW: %I64d [R] + %I64d [W] = %I64d bytes = %5.3f Mbytes",statistics::get(STATS_DEPTH_BUFFER_READ_BW)>>3L,
				statistics::get(STATS_DEPTH_BUFFER_WRITE_BW)>>3L,tmp,float(double(tmp)/(1024.0*1024.0)));
			displayString(str,20,120);

			sprintf_s(str,"Depth clear BW: %I64d bytes = %5.3f Mbytes",statistics::get(STATS_DEPTH_CLEAR_BW)>>3L,
				float(double(statistics::get(STATS_DEPTH_CLEAR_BW)>>3L)/(1024.0*1024.0)));
			displayString(str,20,100);

			// print color buffer BW stats
			tmp=(statistics::get(STATS_COLOR_BUFFER_READ_BW)+statistics::get(STATS_COLOR_BUFFER_WRITE_BW))>>3L;
			sprintf_s(str,"Color buffer BW: %I64d [R] + %I64d [W] = %I64d bytes = %5.3f Mbytes",statistics::get(STATS_COLOR_BUFFER_READ_BW)>>3L,
				statistics::get(STATS_COLOR_BUFFER_WRITE_BW)>>3L,tmp,float(double(tmp)/(1024.0*1024.0)));
			displayString(str,20,80);

			sprintf_s(str,"Color clear BW: %I64d bytes = %5.3f Mbytes",statistics::get(STATS_COLOR_CLEAR_BW)>>3L,
				float(double(statistics::get(STATS_COLOR_CLEAR_BW)>>3L)/(1024.0*1024.0)));
			displayString(str,20,60);

			// print texture stats
			sprintf_s(str,"Texture reads: %I64d = %5.3f Mbytes",statistics::get(STATS_TEXTURE_READ_BW)>>3L,
				float(double(statistics::get(STATS_TEXTURE_READ_BW)>>3L)/(1024.0*1024.0)));
			displayString(str,20,40);

			int64 tot_r=(statistics::get(STATS_DEPTH_BUFFER_READ_BW) + statistics::get(STATS_COLOR_BUFFER_READ_BW) + statistics::get(STATS_TEXTURE_READ_BW)  )>>3L;
			int64 tot_w=(statistics::get(STATS_DEPTH_BUFFER_WRITE_BW) + statistics::get(STATS_COLOR_BUFFER_WRITE_BW) )>>3L;
			int64 tot_c=(statistics::get(STATS_DEPTH_CLEAR_BW) + statistics::get(STATS_COLOR_CLEAR_BW) )>>3L;

			sprintf_s(str,"Total bandwidth (BW): %5.3f [R] + %5.3f [W] + %5.3f [C] = %5.3f Mbytes\n",
				float(double(tot_r)/(1024.0*1024.0)),float(double(tot_w)/(1024.0*1024.0)),float(double(tot_c)/(1024.0*1024.0)),
				float(double(tot_r+tot_w+tot_c)/(1024.0*1024.0)));
			displayString(str,20,20);

		}
	}
	glDrawBuffer(GL_BACK);
	glFlush();
}

// the display callback function for GLUT
static void displayCB(void)
{
	cTimer timer;
	timer.reset();
	timer.start();
	if(gRenderUsingOpenGL)  gAPI.activateHW_OGL();
	else					gAPI.activateLUR();

	gCurrentScene->clearScreenAndInitFrame(&gCamera);
	gCamera.setOGLMatrices();

	gCurrentScene->renderFrame();
	
	// delay so that rendering (with real graphics hardware is not too fast)
	double t;
	do 
	{
		timer.stop();
		t=timer.getTime();
		timer.start();		
	} while(t < 1.0f/gMaxFrameRate);
	gAPI._glutSwapBuffers();
	displayStuff();
}

void reshapeCB(int width, int height)
{
	// make the window size divide:able with tile size
	width=width - (width  % gRasterizer->getTileWidth());
	height=height-(height % gRasterizer->getTileHeight());
	width=MAX2(gRasterizer->getTileWidth(),width);
	height=MAX2(gRasterizer->getTileHeight(),height);
	glutReshapeWindow(width,height);
	
	gCamera.setResolution(width,height);
}

// the keyboard callback function for GLUT
static void keyboardCB(unsigned char key, int x, int y)
{
	int scene_num=int(key-'1');
	switch(key)
	{
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		if(gCurrentScene!=gScenes[scene_num])
		{
			gCurrentScene=gScenes[scene_num];
			gCurrentScene->firstFrameInit(&gCamera);
		}
		break;
	case 'r':
	case 'R':
		gRenderUsingOpenGL=!gRenderUsingOpenGL;
		break;
	case 's':
	case 'S':
		gDisplayStats=!gDisplayStats;
		break;
	case 'c':
	case 'C':
		statistics::clear();
		break;
	case 'p':
	case 'P':
		statistics::print();
		break;
	case ' ':
		gAnimationOn=!gAnimationOn;
		gCurrentScene->toggleAnimationOnOff();
		break;
	case 27:		// ESCAPE  == EXIT
		cleanExit();
		break;
	}
	glutPostRedisplay();
}

// the menus callback function for GLUT
static void menusCB(int value)
{
	switch(value)
	{
	case MENU_TOGGLE_RASTERIZER:
		gRenderUsingOpenGL=!gRenderUsingOpenGL;
		break;
	case MENU_TOGGLE_ANIMATION:
		gAnimationOn=!gAnimationOn;
		gCurrentScene->toggleAnimationOnOff();
		break;
	case MENU_TOGGLE_DISPLAY_STATS:
		gDisplayStats=!gDisplayStats;
		break;
	case MENU_EXIT:
		cleanExit();
		break;
	}
	glutPostRedisplay();
}

static void recordMouseCB(int button, int state, int x, int y)
{
	bool shiftDown=glutGetModifiers() & GLUT_ACTIVE_SHIFT ? true : false;
	bool ctrlDown =glutGetModifiers() & GLUT_ACTIVE_CTRL ? true : false;
	bool altDown  =glutGetModifiers() & GLUT_ACTIVE_ALT ? true : false;
	gCamera.recordMouse(button,state,x,y,shiftDown,ctrlDown,altDown); 
	if(button==GLUT_LEFT_BUTTON)
	{
		printf("%d %d\n",x,gAPI.getHeight()-y-1);
	}
}

void recordMotionCB(int x,int y)
{
	gCamera.recordMotion(x,y);
	gCamera.move();
	gCamera.postRecordMotion();
	gCamera.setOGLMatrices();
}


// call this function when you exit the program.
// is supposed to clean up everything, free memory etc.
void cleanExit(void)
{
	// free memory etc here...

	exit(0);
}

void initRasterizer(void)
{
	// init basic rasterizer
	cSimpleDepthUnit	*du=new cSimpleDepthUnit();
	cColorUnit			*cu=new cSimpleColorUnit();
	cTextureUnit		*tu=new cTextureUnit(NULL);

	cRasterizer *rasterizer=new cRasterizer(&gAPI,du,cu,tu);
// uncomment these two lines if you want a tile-based rasterizer
//	cTileRasterizer *rasterizer=new cTileRasterizer(&gAPI,du,cu,tu);
//	rasterizer->setTileSize(2,2);
	gAPI.setRasterizer(rasterizer);

	gRasterizer=rasterizer;		// save the rasterizer for reshapeCB...
}

void initScenes(void)
{
	gScenes[0]=new cUmbrellaScene(&gAPI,64);
	gScenes[1]=new cUmbrellaScene(&gAPI,10);
	gScenes[2]=new cTexturedQuadScene(&gAPI);
	gScenes[3]=new cTextureScene(&gAPI);
	gScenes[4]=new cProjectScene(&gAPI);

	// main setup code is executed for each scene (loading of textures etc)
	for(int q=0;q<gNumScenes;q++)
	{
		gScenes[q]->setup();
	}

	gCurrentScene=gScenes[0];
	gCurrentScene->firstFrameInit(&gCamera);
}

int main(int argc,char *argv[])
{
	initRasterizer();

	gAPI.setResolution(640,480);
//	gAPI.setResolution(320,240);
	gRenderUsingOpenGL=false;

	glutInit(&argc, argv);
    glutInitWindowPosition(500,100);
	glutInitWindowSize(gAPI.getWidth(),gAPI.getHeight());
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
	glutCreateWindow("LUR: Lund University Rasterizer");

	initScenes();

	glutDisplayFunc(displayCB);
	glutKeyboardFunc(keyboardCB);
	glutMouseFunc(recordMouseCB);
	glutMotionFunc(recordMotionCB);
	glutReshapeFunc(reshapeCB);


	// main menu
	glutCreateMenu(menusCB);
	glutAddMenuEntry("[R] toggle Rasterizer", MENU_TOGGLE_RASTERIZER);
	glutAddMenuEntry("[S] toggle display Statistics", MENU_TOGGLE_DISPLAY_STATS);
	glutAddMenuEntry("[SPACE] toggle animation on/off",MENU_TOGGLE_ANIMATION);
	glutAddMenuEntry("[ESC] Exit", MENU_EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
	return 0;
}

