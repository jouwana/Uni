#pragma once

#include "resource.h"
//#include "material_and_light.h"
//#include "Scene.h"

void display( void );
void reshape( int width, int height );
void keyboard( unsigned char key, int x, int y );
void mouse(int button, int state, int x, int y);
void fileMenu(int id);
void mainMenu(int id);
void initMenu();

//Scene* scene;
//Renderer* renderer;

// choose the shading algorithm t use in rendering
enum ShadingAlgorithm
{
	FLAT, GOURAND, PHONG
};

//Light* set_and_return_active_light(int id)
//{
//	return scene->scene_set_and_return_active_light(id);
//}