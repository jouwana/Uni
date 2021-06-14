// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>

#include "PrimMeshModel.h"
#include "StepSizeDlg.h"


#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

// blurring menu
#define BLURRING_NONE 0
#define BLURRING_FULL_SCREEN 1
#define BLURRING_BLOOM 2

// file menu
#define FILE_OPEN 1
#define PRIM_OPEN 2

// setStepSize menu
#define SCALE 1
#define ZOOM 2
#define TRANSLATE 3
#define ROTATE 4

// toggle menu
#define TOGGLE_BOUNDINGBOX 1
#define TOGGLE_NORM_FACE 2
#define TOGGLE_NORM_VERTEX 3
#define TOGGLE_CAMERA 4
#define TOGGLE_COLOR_ANIMATION_SET 5
#define TOGGLE_COLOR_ANIMATION_HUE 6
#define TOGGLE_VERTEX_ANIMATION 7
#define TOGGLE_NORMAL_MAPPING 8
#define TOGGLE_CANONICAL_MAPPING_1 9
#define TOGGLE_CANONICAL_MAPPING_2 10
#define TOGGLE_REGULAR_MAPPING 11
#define TOGGLE_ENVIRONMENT_MAPPING 12

// shading menu
#define SHADING_FLAT 1
#define SHADING_GOURAND 2
#define SHADING_PHONG 3
#define SHADING_TOON 4

// main menu
#define MAIN_LOOKAT 1
#define MAIN_ADD_CAMERA 2
#define MAIN_ADD_LIGHT 3
#define MAIN_DEMO 4
#define MAIN_ABOUT 5
#define MAIN_SET_MATERIAL 6
#define MAIN_SET_AMBIENT 7
#define MAIN_CHANGE_LIGHT_DETAILS 8
#define MAIN_SET_TURB 9

//decleration added before we start the code of anyother fucntion (near defines)
//function declerations:
void print_info();
void print_lights_info();

Scene *scene;
Renderer *renderer;

int last_x,last_y;
bool lb_down,rb_down,mb_down;



//my attributes
int R_width = 1280, R_height = 720; // initial values
bool clear = false;
bool worldTrans = false;
bool draw_face_normals = false;
bool draw_bounding_box = false;
bool draw_vertex_normals = false;
bool draw_cameras = false;
bool firstClick = true;
bool cameraTrans = false;
bool animateVertices = false;
int normalMapping = 0;
int animateColors = 0;
int texture_type =0 ;

//new attribute
float defaultStepScale = 1.05f;
float defaultStepZoom = 1.05f;
float defaultStepTranslate = 0.02f;
float defaultStepRotate = 0.5f;

//my defines
#define KEY_TAB 9
//new definition of theta
#define THETA (5*defaultStepRotate)*M_PI/180

//my helper function
void sendTransformation(mat4& trans, mat4& inv)
{

	if (cameraTrans)
		scene->transformCamera(trans, inv);
	else if (!cameraTrans && !worldTrans)
		scene->transformActiveModel(trans ,inv);
	else if (!cameraTrans && worldTrans) 
		scene->transformWorld(trans ,inv);

	//renderer->ClearColorBuffer();
	glutPostRedisplay();
}

vec2 screenMiddlePoint()
{
	return vec2(0, 0);
	
}

void animationTimer(int value)
{
	if(value)
		glutPostRedisplay();
}

//----------------------------------------------------------------------------
// Callbacks
bool fileIn = false;
void display(void)
{
	//if (!clear)
	//{
	//	renderer->ClearColorBuffer();
	//	renderer->ClearDepthBuffer();
	//	scene->draw();
	//	if (draw_normals)//face normals
	//		scene->drawNormals();

	//	if (draw_vertex_normals)
	//		scene->drawVertexNormals();
	//}
	if (fileIn)
	{
		GLCall(glClearColor(0.2, 0.2, 0.2, 0.0));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		
		scene->colorAnimation = animateColors;
		scene->vertexAnimation = animateVertices;
		scene->drawDemo(normalMapping, texture_type);
		
		if (draw_vertex_normals)
			scene->drawVertexNormals();
		if (draw_face_normals)
			scene->drawFaceNormals();
		if (draw_bounding_box)
			scene->draw_bounding_box();

		GLCall(glFlush());
		GLCall(glutSwapBuffers());
		glutTimerFunc(10, animationTimer, animateColors+animateVertices);
	}
}


void reshape( int width, int height )
{
	//update values of global variables
	R_width = width;
	R_height = height;

	//update the renderer's buffers
	//renderer->updateBuffer(width, height);
}

void keyboard( unsigned char key, int x, int y )
{
	vec2 midPnt;
	if (worldTrans)
		midPnt = screenMiddlePoint();
	else
		midPnt = scene->getMidOfActiveModel();
	
	mat4 trans;
	mat4 inv;

	//new added degrees.
	float Xdeg = scene->getActiveCameraXRotation();
	float Ydeg = scene->getActiveCameraYRotation();
	float Zdeg = scene->getActiveCameraZRotation();

	
	switch ( key ) {
	case 033:
		exit( EXIT_SUCCESS );
		break;
		
	case 'f':
	//	renderer->enable_fog = !renderer->enable_fog;
		glutPostRedisplay();
		break;

	case 'i':// zoom in
		scene->zoom_in_active_model(defaultStepZoom);
		glutPostRedisplay();
		break;
	case 'o':// zoom out
		scene->zoom_out_active_model(defaultStepZoom);
		glutPostRedisplay();
		break;

	//get next model in scene
	case KEY_TAB:
		scene->nextActiveModel();
		glutPostRedisplay();
		break;

	//get next camera in scene
	case '`':
		scene->nextActiveCamera();
		glutPostRedisplay();
		break;
		
	//change between world and model transformation
	case 'm':
		worldTrans = !worldTrans;
		cameraTrans = false;
		scene->worldTransforming = !scene->worldTransforming;
		scene->cameraTransforming = false;
		glutPostRedisplay();
		break;

	//turn on/off camera transformation mode
	case 'e': //new wont go to camera mode if there are no active cameras
		if (scene->activeCamera != -1)
		{
			cameraTrans = !cameraTrans;
			worldTrans = false;
			scene->cameraTransforming = !scene->cameraTransforming;
			scene->worldTransforming = false;
		}
		glutPostRedisplay();
		break;

	case '1': // scale x axis
		if (cameraTrans)
		{}
		else // model transform
		{
			inv = Scale(defaultStepScale, 1, 1);
			trans = Scale(1.0 / defaultStepScale, 1.0 / 1, 1.0 / 1);
			sendTransformation(trans, inv);
		}
		break;

	case '2': // scale x axis
		if (cameraTrans)
		{
		}
		else // model transform
		{
			trans = Scale(defaultStepScale, 1, 1);
			inv = Scale(1.0 / defaultStepScale, 1.0 / 1, 1.0 / 1);
			sendTransformation(trans, inv);
		}
		break;
		
	case '4':// scale y axis
		if (cameraTrans)
		{
		}
		else // model transform
		{
			inv = Scale(1, defaultStepScale, 1);
			trans = Scale(1.0 / 1, 1.0 / defaultStepScale, 1.0 / 1);
			sendTransformation(trans, inv);
		}
		break;

	case '5': // scale y axis
		if (cameraTrans)
		{
		}
		else // model transform
		{
			trans = Scale(1, defaultStepScale, 1);
			inv = Scale(1.0 / 1, 1.0 / defaultStepScale, 1.0 / 1);
			sendTransformation(trans, inv);
		}
		break;

	case '7':// scale z axis
		if (cameraTrans)
		{
		}
		else // model transform
		{
			inv = Scale(1, 1, defaultStepScale);
			trans = Scale(1.0 / 1, 1.0 / 1, 1.0 / defaultStepScale);
			sendTransformation(trans, inv);
		}
		break;

	case '8':// scale z axis
		if (cameraTrans)
		{
		}
		else // model transform
		{
			trans = Scale(1, 1, defaultStepScale);
			inv = Scale(1.0 / 1, 1.0 / 1, 1.0 / defaultStepScale);
			sendTransformation(trans, inv);
		}
		break;


	//smaller size
	case '-':
		if (cameraTrans)
		{
			inv = Scale(defaultStepZoom, defaultStepZoom, defaultStepZoom);
			trans = Scale(1.0 / defaultStepZoom, 1.0 / defaultStepZoom, 1.0 / defaultStepZoom);
			sendTransformation(trans, inv);
		}
		else // model transform
		{
			inv = Scale(defaultStepScale, defaultStepScale, defaultStepScale);
			trans = Scale(1.0 / defaultStepScale, 1.0 / defaultStepScale, 1.0 / defaultStepScale);
			sendTransformation(trans, inv);
		}
		break;

	//bigger size
	case '+':
		if (cameraTrans)
		{
			trans = Scale(defaultStepZoom, defaultStepZoom, defaultStepZoom);
			inv = Scale(1.0 / defaultStepZoom, 1.0 / defaultStepZoom, 1.0 / defaultStepZoom);
			sendTransformation(trans, inv);
		}
		else // model transform
		{
			trans = Scale(defaultStepScale, defaultStepScale, defaultStepScale);
			inv = Scale(1.0 / defaultStepScale, 1.0 / defaultStepScale, 1.0 / defaultStepScale);
			sendTransformation(trans, inv);
		}
		
		break;
		
	//new move model based on degree of rotation done by active camera (dont all work)
	//transition around slowly
	case 'q': //forward
		trans = Translate(0, 0, -defaultStepTranslate);
		inv = Translate(0, 0, defaultStepTranslate);
		sendTransformation(trans, inv);

		break;
	case 'Q': //backward
		trans = Translate(0, 0, defaultStepTranslate);
		inv = Translate(0, 0, -defaultStepTranslate );
		sendTransformation(trans, inv);

		break;
	case 'w':	
		trans = Translate(sinf(Zdeg) * defaultStepTranslate,cosf(Xdeg) * cosf(Zdeg) * defaultStepTranslate,sinf(Xdeg) * defaultStepTranslate);
		inv = Translate(-sinf(Zdeg) * defaultStepTranslate, -cosf(Xdeg) * cosf(Zdeg) * defaultStepTranslate, -sinf(Xdeg) * defaultStepTranslate);
		sendTransformation(trans,inv);
		
		break;
	case 'a':
		trans = Translate(-cosf(Ydeg) * cosf(Zdeg) * defaultStepTranslate, -sinf(Zdeg) * defaultStepTranslate, -sinf(Ydeg) * defaultStepTranslate);
		inv = Translate(cosf(Ydeg) * cosf(Zdeg) * defaultStepTranslate, sinf(Zdeg) * defaultStepTranslate, sinf(Ydeg) * defaultStepTranslate);
		sendTransformation(trans,inv);
		break;
	case 's':
		trans = Translate(-sinf(Zdeg) * defaultStepTranslate, -cosf(Xdeg) * cosf(Zdeg) * defaultStepTranslate, -sinf(Xdeg) * defaultStepTranslate);
		inv = Translate(sinf(Zdeg) * defaultStepTranslate, cosf(Xdeg) * cosf(Zdeg) * defaultStepTranslate, sinf(Xdeg) * defaultStepTranslate);
		sendTransformation(trans, inv);
		break;
	case 'd':
		trans = Translate(cosf(Ydeg) * cosf(Zdeg) * defaultStepTranslate, sinf(Zdeg) * defaultStepTranslate, sinf(Ydeg) * defaultStepTranslate);
		inv = Translate(-cosf(Ydeg) * cosf(Zdeg) * defaultStepTranslate, -sinf(Zdeg) * defaultStepTranslate, -sinf(Ydeg) * defaultStepTranslate);
		sendTransformation(trans, inv);
		break;

	//transition around quickly
	case 'W':
		trans = Translate(sinf(Zdeg) * 8 * defaultStepTranslate, cosf(Xdeg) * cosf(Zdeg) * 8 * defaultStepTranslate, sinf(Xdeg) * 8 * defaultStepTranslate);
		inv = Translate(-sinf(Zdeg) * 8 * defaultStepTranslate, -cosf(Xdeg) * cosf(Zdeg) * 8 * defaultStepTranslate, -sinf(Xdeg) * 8 * defaultStepTranslate);
		sendTransformation(trans, inv);
		break;
	case 'A':
		trans = Translate(-cosf(Ydeg) * cosf(Zdeg) * 8 * defaultStepTranslate, -sinf(Zdeg) * 8 * defaultStepTranslate, -sinf(Ydeg) * 8 * defaultStepTranslate);
		inv = Translate(cosf(Ydeg) * cosf(Zdeg) * 8 * defaultStepTranslate, sinf(Zdeg) * 8 * defaultStepTranslate, sinf(Ydeg) * 8 * defaultStepTranslate);

		sendTransformation(trans, inv);
		break;
	case 'S':
		trans = Translate(-sinf(Zdeg) * 8 * defaultStepTranslate, -cosf(Xdeg) * cosf(Zdeg) * 8 * defaultStepTranslate, -sinf(Xdeg) * 8 * defaultStepTranslate);
		inv = Translate(sinf(Zdeg) * 8 * defaultStepTranslate, cosf(Xdeg) * cosf(Zdeg) * 8 * defaultStepTranslate, sinf(Xdeg) * 8 * defaultStepTranslate);
		sendTransformation(trans, inv);
		break;
	case 'D':
		trans = Translate(cosf(Ydeg) * cosf(Zdeg) * 8 * defaultStepTranslate, sinf(Zdeg) * 8 * defaultStepTranslate, sinf(Ydeg) * 8 * defaultStepTranslate);
		inv = Translate(-cosf(Ydeg) * cosf(Zdeg) * 8 * defaultStepTranslate, -sinf(Zdeg) * 8 * defaultStepTranslate, -sinf(Ydeg) * 8 * defaultStepTranslate);
		sendTransformation(trans, inv);
		break;

	//rotate on screen, move x,y, around middle of the screen, theta = 5 degrees
	case '9': //x,y rotation to the left
		//new set theta in camera
		if (cameraTrans)
			 scene->setActiveCameraZRotation(-THETA);

		trans = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		trans *= mat4(cosf(THETA), sinf(THETA), 0, 0, -sinf(THETA), cosf(THETA), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
		trans *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		inv = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		inv *= mat4(cosf(THETA), -sinf(THETA), 0, 0, sinf(THETA), cosf(THETA), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
		inv *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		sendTransformation(trans,inv);
		break;

	case '6': //x,y rotation to the right
			//new set theta in camera
		if (cameraTrans)
			scene->setActiveCameraZRotation(THETA);

		trans = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		trans *= mat4(cosf(THETA), -sinf(THETA), 0, 0, sinf(THETA), cosf(THETA), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
		trans *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		inv = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		inv *= mat4(cosf(THETA), sinf(THETA), 0, 0, -sinf(THETA), cosf(THETA), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
		inv *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		sendTransformation(trans,inv);
		break;
	}


}

//my special keys handler

void specialKeys(int key, int x, int y)
{
	vec2 midPnt;
	if (worldTrans)
		midPnt = screenMiddlePoint();
	else
		midPnt = scene->getMidOfActiveModel();

	mat4 trans;
	mat4 inv;


	switch (key)
	{
	case GLUT_KEY_F11:
		//draw the scene
		//renderer->ClearColorBuffer();
		clear = false;
		glutPostRedisplay();
		break;
	case GLUT_KEY_F3:
		//new supersample activate
		//renderer->setSuperSampleVars(2, 2);
		//scene->updateLightLocations();
		glutPostRedisplay();
		break;
	case GLUT_KEY_F4:
		//new supersample disable
		//renderer->setSuperSampleVars(1, 1);
		//scene->updateLightLocations();
		glutPostRedisplay();
		break;
	case GLUT_KEY_F1:
		//this button clears up the screen
		//renderer->ClearColorBuffer();
		clear = true;
		//renderer->SwapBuffers();
		break;
	case GLUT_KEY_F2:
		//this button clears up the scene.
		//delete renderer;
		delete scene;
		GLCall(glClearColor(0.2, 0.2, 0.2, 0.0));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		glutSwapBuffers();
		//renderer = new Renderer(R_width, R_height);
		scene = new Scene(renderer);
		//renderer->ClearColorBuffer();
		//renderer->SwapBuffers();
		//new reset the old values, these were the ones giving us the zoom in zoom out errors

		clear = false;
		worldTrans = false;
		draw_face_normals = false;
		draw_bounding_box = false;
		draw_vertex_normals = false;
		draw_cameras = false;
		firstClick = true;
		cameraTrans = false;
		

		//new the default step size of scale and zoom used to be incorrect in this part of the code (0.5f)
		defaultStepScale = 1.05f;
		defaultStepZoom = 1.05f;
		defaultStepTranslate = 0.02f;
		defaultStepRotate = 0.5f;


		//new mapping and textures
		texture_type = 0;
		animateVertices = false;
		animateColors = false;
		fileIn = false;
		normalMapping = 0;
		
	
		
		glutPostRedisplay();
		break;

		//rotation based on middle of the screen, theta = 5 degrees;
	case GLUT_KEY_UP: //rotation around x axis -> up

		if (cameraTrans)
			scene->setActiveCameraXRotation(THETA);

		trans = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		trans *= mat4(1, 0, 0, 0, 0, cosf(THETA), sinf(THETA), 0, 0, -sinf(THETA), cosf(THETA), 0, 0, 0, 0, 1);;
		trans *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		inv = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		inv *= mat4(1, 0, 0, 0, 0, cosf(THETA), -sinf(THETA), 0, 0, sinf(THETA), cosf(THETA), 0, 0, 0, 0, 1);;
		inv *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		sendTransformation(trans, inv);
		break;

	case GLUT_KEY_DOWN: //rotation around x axis -> down

		if (cameraTrans)
			scene->setActiveCameraXRotation(-THETA);

		trans = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		trans *= mat4(1, 0, 0, 0, 0, cosf(THETA), -sinf(THETA), 0, 0, sinf(THETA), cosf(THETA), 0, 0, 0, 0, 1);;
		trans *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		inv = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		inv *= mat4(1, 0, 0, 0, 0, cosf(THETA), sinf(THETA), 0, 0, -sinf(THETA), cosf(THETA), 0, 0, 0, 0, 1);;
		inv *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		sendTransformation(trans, inv);
		break;


	case GLUT_KEY_LEFT://rotate to the left (around y axis)

		if (cameraTrans)
			scene->setActiveCameraYRotation(-THETA);

		trans = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		trans *= mat4(cosf(THETA), 0, -sinf(THETA), 0, 0, 1, 0, 0, sinf(THETA), 0, cosf(THETA), 0, 0, 0, 0, 1);
		trans *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		inv = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		inv *= mat4(cosf(THETA), 0, sinf(THETA), 0, 0, 1, 0, 0, -sinf(THETA), 0, cosf(THETA), 0, 0, 0, 0, 1);
		inv *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		sendTransformation(trans, inv);
		break;

	case GLUT_KEY_RIGHT://rotate to the right (around y axis)

		if (cameraTrans)
			scene->setActiveCameraYRotation(THETA);

		trans = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		trans *= mat4(cosf(THETA), 0, sinf(THETA), 0, 0, 1, 0, 0, -sinf(THETA), 0, cosf(THETA), 0, 0, 0, 0, 1);
		trans *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		inv = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, midPnt.x, midPnt.y, 0, 1);
		inv *= mat4(cosf(THETA), 0, -sinf(THETA), 0, 0, 1, 0, 0, sinf(THETA), 0, cosf(THETA), 0, 0, 0, 0, 1);
		inv *= mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -midPnt.x, -midPnt.y, 0, 1);
		sendTransformation(trans, inv);
		break;

	case GLUT_KEY_F7:
		print_lights_info();
		break;
	case GLUT_KEY_F8:
		print_info();
		break;


	}

}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}

	//set down flags
	switch (button) {
	case GLUT_LEFT_BUTTON:
		lb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	case GLUT_RIGHT_BUTTON:
		rb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	case GLUT_MIDDLE_BUTTON:
		mb_down = (state == GLUT_UP) ? 0 : 1;
		break;
	}

	// add your code
	if (lb_down && button == GLUT_LEFT_BUTTON)
	{
		bool flag = scene->activateOnClick((x / (1280.0f / 2.0f)) - 1.0f, (-y / (720.0f / 2.0f)) + 1.0f);
		if (flag)
		{
			worldTrans = false;
			scene->worldTransforming = false;
			glutPostRedisplay();
		}
	}
	firstClick = true;
	/*cout << x << " " << y << endl;*/
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx = x - last_x;
	int dy = y - last_y;
	bool flag = scene->activateOnClick((x / (1280.0f / 2.0f)) - 1.0f, (-y / (720.0f / 2.0f)) + 1.0f);

	if (flag)
	{
		if (!firstClick)
		{
			worldTrans = false;
			scene->worldTransforming = false;
			float incX = (dx / (1280.0f/2.0f)) ;
			float incY = (-dy / (720.0f/2.0f)) ;
			scene->transformActiveModel(Translate(incX, incY, 0), Translate(-incX, -incY, 0));

		}
		else firstClick = false;
		glutPostRedisplay();
	}

	// update last x,y
	last_x = x;
	last_y = y;
}

void fileMenu(int id)
{
	switch (id)
	{

		case PRIM_OPEN:
		{
			PrimMeshModel* p = new PrimMeshModel();
			scene->loadPrimModel(p);
			glutPostRedisplay();
		}
		break;

		case FILE_OPEN:
			CFileDialog dlg(TRUE,_T(".obj"),NULL,NULL,_T("*.obj|*.*"));
			if(dlg.DoModal()==IDOK)
			{
				std::string s((LPCTSTR)dlg.GetPathName());
				scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
				clear = false;
				glutPostRedisplay();
				fileIn = true;
			}
			break;

	}
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_DEMO:
		scene->drawDemo(normalMapping, texture_type);
		break;
	case MAIN_ABOUT:
		AfxMessageBox(_T("Computer Graphics"));
		break;

	case MAIN_LOOKAT:
		//scene->lookAtActiveModel();
		glutPostRedisplay();
		break;
	case MAIN_ADD_CAMERA:// add a new camera
		scene->addNewCamera();
		glutPostRedisplay();
		break;
	case MAIN_ADD_LIGHT: // add a new camera
		scene->addLightSource();
		glutPostRedisplay();
		break;
	case MAIN_SET_MATERIAL:
		scene ->Set_material_for_active_model();
		glutPostRedisplay();
		break;
	case MAIN_SET_AMBIENT:
		scene->set_ambient_color();
		glutPostRedisplay();
		break;	
	case MAIN_CHANGE_LIGHT_DETAILS: // add a new camera
		print_lights_info();
		scene->change_light_details();
		glutPostRedisplay();
		print_info();
		break;
	case MAIN_SET_TURB:
		scene->turbText = !scene->turbText;
		glutPostRedisplay();
		break;
	}
}

void setStepSizeMenu(int id)
{
	StepSizeDlg step_dlg;
	float step_size;
	switch (id)
	{
	case SCALE:
		if (step_dlg.DoModal() == IDOK)
		{
			step_size = atof(step_dlg.edit_stepSize_text);
		}
		if (step_size == 0)
			defaultStepScale = 0.5f;
		else defaultStepScale = step_size;
		glutPostRedisplay();
		break;

	case ZOOM:
		if (step_dlg.DoModal() == IDOK)
		{
			step_size = atof(step_dlg.edit_stepSize_text);
		}
		if (step_size == 0)
			defaultStepZoom = 0.5f;
		else defaultStepZoom = step_size;
		glutPostRedisplay();
		break;

	case TRANSLATE:
		if (step_dlg.DoModal() == IDOK)
		{
			step_size = atof(step_dlg.edit_stepSize_text);
		}
		if (step_size == 0)
			defaultStepTranslate = 0.02f;
		else defaultStepTranslate = step_size;
		glutPostRedisplay();
		break;


	case ROTATE:
		if (step_dlg.DoModal() == IDOK)
		{
			step_size = atof(step_dlg.edit_stepSize_text);
		}
		if (step_size == 0)
			defaultStepRotate = 0.5f;
		else defaultStepRotate = step_size;
		glutPostRedisplay();
		break;
	}

}

void toggleMenu(int id)
{
	switch(id)
	{
		case TOGGLE_BOUNDINGBOX:
			draw_bounding_box = !draw_bounding_box;
			glutPostRedisplay();
			break;
		case TOGGLE_NORM_FACE:
			draw_face_normals = !draw_face_normals;
			glutPostRedisplay();
			break;
		case TOGGLE_NORM_VERTEX:
			draw_vertex_normals = !draw_vertex_normals;
			glutPostRedisplay();
			break;
		case TOGGLE_CAMERA:
			draw_cameras = !draw_cameras;
			scene->set_drawCameras(draw_cameras);
			glutPostRedisplay();
			break;
		case TOGGLE_COLOR_ANIMATION_SET:
			animateColors = (animateColors == 1) ? 0 : 1;
			glutPostRedisplay();
			break;
		case TOGGLE_COLOR_ANIMATION_HUE:
			animateColors = (animateColors == 2) ? 0 : 2;
			glutPostRedisplay();
			break;
		case TOGGLE_VERTEX_ANIMATION:
			animateVertices = !animateVertices;
			glutPostRedisplay();
			break;
		case TOGGLE_NORMAL_MAPPING:
			if (normalMapping == 0)
				normalMapping = 1;
			else
				normalMapping = 0;
			glutPostRedisplay();
			break;
		case TOGGLE_REGULAR_MAPPING:
			texture_type = 0;
			glutPostRedisplay();
			break;
		case TOGGLE_CANONICAL_MAPPING_1:
			texture_type = (texture_type == 1)?0:1;
			glutPostRedisplay();
			break;
		case TOGGLE_CANONICAL_MAPPING_2:
			texture_type = (texture_type == 2) ? 0 : 2;
			glutPostRedisplay();
			break;
		case TOGGLE_ENVIRONMENT_MAPPING:
			texture_type = (texture_type == 3) ? 0 : 3;
			glutPostRedisplay();
			break;
	}
}

void shadeMenu(int id)
{
	scene->toonShading = false;
	switch (id)
	{
	case SHADING_FLAT:
		scene->set_shading_alg(FLAT);
		glutPostRedisplay();
		break;
	case SHADING_GOURAND:
		scene->set_shading_alg(GOURAND);
		glutPostRedisplay();
		break;
	case SHADING_PHONG:
		scene->set_shading_alg(PHONG);
		glutPostRedisplay();
		break;
	case SHADING_TOON:
		scene->toonShading = true;
		glutPostRedisplay();
		break;

	}
}

void blurMenu(int id)
{
	switch (id)
	{
	case BLURRING_NONE:
		renderer->blurring_mode = None;
		glutPostRedisplay();
		break;
	case BLURRING_FULL_SCREEN:
		renderer->blurring_mode = FULL_SCREEN;
		glutPostRedisplay();
		break;
	case BLURRING_BLOOM:
		renderer->blurring_mode = BLOOM;
		glutPostRedisplay();
		break;
	}
}

void initMenu()
{

	int menuFile = glutCreateMenu(fileMenu);	
	glutAddMenuEntry("Open..", FILE_OPEN);
	/*glutAddMenuEntry("Primitive: Box", PRIM_OPEN);*/

	int menuSetStepSize = glutCreateMenu(setStepSizeMenu);
	glutAddMenuEntry("Scale", SCALE);
	glutAddMenuEntry("Zoom", ZOOM);
	glutAddMenuEntry("Translate", TRANSLATE);
	glutAddMenuEntry("Rotate", ROTATE);

	int menuToggle = glutCreateMenu(toggleMenu);
	glutAddMenuEntry("Bounding Box", TOGGLE_BOUNDINGBOX);
	glutAddMenuEntry("Normals Per Face", TOGGLE_NORM_FACE);
	glutAddMenuEntry("Normals Per Vertex", TOGGLE_NORM_VERTEX);
	glutAddMenuEntry("Camera", TOGGLE_CAMERA);
	glutAddMenuEntry("Color Animation 1", TOGGLE_COLOR_ANIMATION_SET);
	glutAddMenuEntry("Color Animation 2", TOGGLE_COLOR_ANIMATION_HUE);
	glutAddMenuEntry("Vertex Animation", TOGGLE_VERTEX_ANIMATION);
	glutAddMenuEntry("Normal Mapping", TOGGLE_NORMAL_MAPPING);
	glutAddMenuEntry("Canonical Mapping 1", TOGGLE_CANONICAL_MAPPING_1);
	glutAddMenuEntry("Canonical Mapping 2", TOGGLE_CANONICAL_MAPPING_2);
	glutAddMenuEntry("Regular Mapping", TOGGLE_REGULAR_MAPPING);
	glutAddMenuEntry("Environment Mapping", TOGGLE_ENVIRONMENT_MAPPING);
	

	//sh 
	int shadingMenu = glutCreateMenu(shadeMenu);
	glutAddMenuEntry("FLAT", SHADING_FLAT);
	glutAddMenuEntry("GOURAND", SHADING_GOURAND);
	glutAddMenuEntry("PHONG", SHADING_PHONG);
	glutAddMenuEntry("TOON", SHADING_TOON);
	
	int blurringMenu = glutCreateMenu(blurMenu);
	glutAddMenuEntry("None", BLURRING_NONE);
	glutAddMenuEntry("Full Screen Blurring", BLURRING_FULL_SCREEN);
	glutAddMenuEntry("Blooming", BLURRING_BLOOM);


	glutCreateMenu(mainMenu);
	glutAddSubMenu("Add Model", menuFile);
	glutAddMenuEntry("Set Ambient Color", MAIN_SET_AMBIENT);
	glutAddMenuEntry("Set Model Material", MAIN_SET_MATERIAL);
	//glutAddSubMenu("Choose Blur Mode", blurringMenu);
	glutAddSubMenu("Set Step Size", menuSetStepSize);
	//glutAddMenuEntry("Look At Model", MAIN_LOOKAT);
	glutAddSubMenu("Toggle", menuToggle);
	glutAddMenuEntry("Turbulence Texture", MAIN_SET_TURB);
	glutAddMenuEntry("Add Camera", MAIN_ADD_CAMERA);
	glutAddMenuEntry("Add Light", MAIN_ADD_LIGHT);
	glutAddMenuEntry("Change Light Details", MAIN_CHANGE_LIGHT_DETAILS);
	glutAddSubMenu("Choose shading", shadingMenu);
	glutAddMenuEntry("Demo",MAIN_DEMO);
	glutAddMenuEntry("About",MAIN_ABOUT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------
//the printing funcs
void print_lights_info()
{
	//clear screen before printing
	cout << flush;
	system("CLS");
	cout << "Light information:" << endl;
	scene->print_lights_info();

}

void print_info()
{
	//clear screen before printing
	cout << flush;
	system("CLS");
	cout << "Program User Instructions:" << endl;
	cout << "	right click: open menu" << endl;

	cout << "	i: zoom in" << endl;
	cout << "	o: zoom out" << endl;
	cout << "	W, A, S, D : quick UP, LEFT, DOWN, RIGHT" << endl;
	cout << "	w, a, s, d : slow UP, LEFT, DOWN, RIGHT" << endl;
	cout << "	q, Q : FORWARD, BACKWARD" << endl;
	
	cout << "	m: transform all models" << endl;
	cout << "	TAB: switch active model" << endl;
	cout << "	`: switch active camera" << endl;
	cout << "	e: camera transformation" << endl;

	cout << "	+, -: aspect ratio scaling" << endl;
	cout << "	1, 2: x scaling" << endl;
	cout << "	4, 5: y scaling" << endl;
	cout << "	7, 8: z scaling" << endl;

	cout << "	6, 9: z axis rotation" << endl;
	cout << "	arrow keys: y and x axis rotation" << endl;

	cout << "	left mouse clicking: choose model" << endl;
	cout << "	mouse motion: drag model" << endl;

	cout << "	f: toggle Fog Effect" << endl;

	cout << "	F1: clear screen" << endl;
	cout << "	F2: delete scene" << endl;
	cout << "	F3: enable SuperSampling (2x2)" << endl;
	cout << "	F4: disable SuperSampling" << endl;
	cout << "	F11: redraw scene" << endl;
	cout << "	F12: draw demo" << endl;
}

int my_main( int argc, char **argv )
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA| GLUT_DOUBLE);
	glutInitWindowSize( R_width, R_height );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow( "CG" );
	glewExperimental = GL_TRUE;
	glewInit();
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		/*		...*/
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));


	GLuint vao; // VertexArrayID
	GLCall(glGenVertexArrays(1, &vao));
	GLCall(glBindVertexArray(vao));
	
	//renderer = new Renderer(R_width,R_height);
	scene = new Scene(renderer);
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	initMenu();
	
	//my special keys function bind
	glutSpecialFunc(specialKeys);
	
	print_info();//prints info
	
	GLCall(glClearColor(0.2, 0.2, 0.2, 0.0));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	glutSwapBuffers();
	
	glutMainLoop();
	delete scene;
	//delete renderer;
	return 0;
}

CWinApp theApp;

using namespace std;

int main( int argc, char **argv )
{
	int nRetCode = 0;
	
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv );
	}
	
	return nRetCode;
}


