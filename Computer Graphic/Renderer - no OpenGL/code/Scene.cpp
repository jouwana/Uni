#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>
//#include <math.h>   

using namespace std;

Scene::~Scene()
{
	for (int i = 0; i < models.size(); i++)
		delete (MeshModel*)models[i];
	for (int i = 0; i < cameras.size(); i++)
		delete cameras[i];
	//delete m_renderer;
	delete dialog;
}

void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	models.push_back(model);
	activeModel = models.size() - 1;	
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves
	//avgend 
	if (activeCamera != -1)
	{
		m_renderer->SetCameraTransform(cameras[activeCamera]->getInvC());
		m_renderer->SetProjection(cameras[activeCamera]->get_projection());
		m_renderer->Set_active_camera_postion(cameras[activeCamera]->position);
	}
	m_renderer->SetLights(lights);

	//draw models
	for (int i = 0; i< models.size() ; i++)
	{
		if (cameraTransforming)
			m_renderer->setColors(vec3(0, 1, 0.5));
		else if(worldTransforming)
			m_renderer->setColors(vec3(0, 0, 1));//green
		else if (i == activeModel)
			m_renderer->setColors(vec3(1, 0, 0));//blue
		else
			m_renderer->setColors(vec3(1, 1, 1));//white
		((MeshModel*)models[i])->draw(m_renderer);
	}


	// draw cameras
	if (drawCameras == true && cameras.size())
	{
		for (int i = 0; i < cameras.size(); i++)
		{
			if (i != activeCamera)
			{
				m_renderer->setColors(vec3(0, 0, 1));//green

				mat4 trans = cameras[i]->get_cTransform();
				
				vec4 new_position = cameras[activeCamera]->get_projection()*cameras[activeCamera]->getInvC()*trans * cameras[i]->position;
				m_renderer->draw_camera(new_position);
				m_renderer->drawLine(new_position.x, new_position.y, new_position.x+2, new_position.y-2);
				m_renderer->drawLine(new_position.x, new_position.y, new_position.x - 2, new_position.y - 2);
				/*vec4 new_direction = trans * cameras[i]->direction;

				vec4 v = new_direction - new_position;
				v = normalize(v);
				v *= 10;
				int a = new_position.x;
				int b = new_position.y;
				int c = v.x;
				int d = v.y;

				m_renderer->drawLine(a, b, a+c, b+d);*/
			}
		}
	}
	m_renderer->SwapBuffers();
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}

void Scene::transformWorld(mat4& transformation, mat4& inv)
{
	for (vector<Model*>::iterator it = models.begin(); it != models.end(); ++it)
	{
		((MeshModel*)(*it))->setWorldTransformation(transformation,inv);
	}
}

void Scene::transformCamera(mat4& transformation, mat4& inv)
{
	if (activeCamera == -1)
		return;
	cameras[activeCamera]->setInvC(inv);
	cameras[activeCamera]->setTransformation(transformation);
	
}

void Scene::transformActiveModel(mat4& transformation, mat4& inv)
{
	if (activeModel == -1)
		return;
	((MeshModel*)models.at(activeModel))->setModelTransformation(transformation,inv);
}


vec2 Scene::getMidOfActiveModel()
{
	if (activeModel == -1)
		return vec2(m_renderer->getWidth() / 2, m_renderer->getHeight() / 2);
	return ((MeshModel*)models[activeModel])->findMid();
}

// face normals
void Scene::drawNormals()
{
	for (int i = 0; i < models.size(); i++)
	{
		if (worldTransforming)
			m_renderer->setColors(vec3(1, 0, 1));
		else if (i == activeModel)
			m_renderer->setColors(vec3(1, 0, 1));//blue
		else
			m_renderer->setColors(vec3(1, 1, 1));//white
		((MeshModel*)models[i])->drawFaceNormals(m_renderer);
	}

	m_renderer->SwapBuffers();
}

void Scene::drawVertexNormals()
{
	if (activeCamera != -1)
		m_renderer->setNormals_cinv(cameras[activeCamera]->get_normal_ctranform());

	for (int i = 0; i < models.size(); i++)
	{
		if (worldTransforming)
			m_renderer->setColors(vec3(1, 0, 1));
		else if (i == activeModel)
			m_renderer->setColors(vec3(1, 0, 1));//blue
		else
			m_renderer->setColors(vec3(1, 1, 1));//white
		((MeshModel*)models[i])->drawVertexNormals(m_renderer);
	}

	m_renderer->SwapBuffers();
}

void Scene::draw_bounding_box()
{
	for (int i = 0; i < models.size(); i++)
	{
		if (worldTransforming)
			m_renderer->setColors(vec3(0, 0, 1));
		else if (i == activeModel)
			m_renderer->setColors(vec3(1, 0, 0));//blue
		else
			m_renderer->setColors(vec3(1, 1, 1));//white

		((MeshModel*)models[i])->draw_bounding_box(m_renderer);
	}

	m_renderer->SwapBuffers();
}

bool Scene::activateOnClick(int x, int y)
{
	//bool i = false; // is T identity
	//mat4 T = mat4(1);
	//if (activeCamera != -1)
	//{
	//	mat4 T = m_renderer->c_project * m_renderer->ctransfrom;
	//	i = true;
	//}
	for (int i = models.size()-1; i >= 0 ; i--)
	{
		if(((MeshModel*)models[i])->clickedOn(x,y, m_renderer->ctransfrom, m_renderer->c_project))
		{
			activeModel = i;
			return true;
		}	
	}
	activeModel = -1;
	return false;
}

void Scene::addNewCamera()
{
	/*vec4 pos = vec4(m_renderer->getWidth()/2, m_renderer->getHeight()/2, 1, 1);*/
	vec4 pos = vec4(m_renderer->getWidth()/2, m_renderer->getHeight() / 2, 0, 1);
	
	mat4 t = mat4(1);

	vec4 direction = vec4(0, 0, - 1,1);

	Camera* c = new Camera(pos, direction);
	c->setTransformation(t);
	cameras.push_back(c);
	activeCamera = cameras.size() - 1;

	//avg 
	mat4 FRUSTUM; // get the parameters from a dialog box
	bool projection_type = false; // false : Ortho, true: Prespective
	if (dialog.DoModal() == IDOK)// open the dialog to get the params
	{
		float edit_left_text = atof(dialog.edit_left_text);
		float edit_right_text = atof(dialog.edit_right_text);
		float edit_bottom_text = atof(dialog.edit_bottom_text);
		float edit_top_text = atof(dialog.edit_top_text);
		float edit_near_text = atof(dialog.edit_near_text);
		float edit_far_text = atof(dialog.edit_far_text);
		float edit_aspect_text = atof(dialog.edit_aspect_text);
		float edit_fovy_text = atof(dialog.edit_fovy_text);

		c->left = edit_left_text;
		c->right = edit_right_text;
		c->top = edit_top_text;
		c->bottom = edit_bottom_text;
		c->zNear = edit_near_text;
		c->zFar = edit_far_text;
		c->aspect = edit_aspect_text;
		c->fovy = edit_fovy_text;

		if (dialog.pres_checkbox == true && dialog.orthogonal_checkbox == false)
		{
			projection_type = true;
			c->projectionType = 2;
			if (edit_fovy_text ==0)
				c->projectionType = 1;
		}
		else if (dialog.pres_checkbox == false && dialog.orthogonal_checkbox == true)
			projection_type = 0; // same as default
		
		mat4* projection = new mat4(1); // default value
		if (projection_type == 0) // Ortho
			*projection = cameras[activeCamera]->Ortho(edit_left_text, edit_right_text, edit_bottom_text, edit_top_text, edit_near_text, edit_far_text);
		else
		{
			if(edit_fovy_text == 0 || edit_aspect_text == 0)
				*projection = cameras[activeCamera]->Frustum(edit_left_text, edit_right_text, edit_bottom_text, edit_top_text, edit_near_text, edit_far_text);
			else
				*projection = cameras[activeCamera]->Perspective(edit_fovy_text, edit_aspect_text, edit_near_text, edit_far_text);
		}

		cameras[activeCamera]->setProjection(*projection);
		delete projection;
	}

}

//new look at active model, might not work without the added extra transformation that are written as comment
void Scene::lookAtActiveModel()
{
	if (activeModel == -1 || activeCamera == -1)
		return;

	vec4 mid = m_renderer->c_project * m_renderer->ctransfrom * (((MeshModel*)models[activeModel])->getmidwithz());
	vec3 tmp = transformVec4ToVec3(mid);

	vec2 mid_screen = vec2(m_renderer->getWidth() / 2, m_renderer->getHeight() / 2);


	mat4 t = Translate(mid_screen.x - tmp.x, mid_screen.y - tmp.y, 0);
	mat4 t_inv = Translate(-mid_screen.x + tmp.x, -mid_screen.y + tmp.y, 0);

	cameras[activeCamera]->setInvC(t);
	cameras[activeCamera]->setTransformation(t_inv);

	//Camera* curr = cameras[activeCamera];
	//
	//vec4 mid = curr->get_projection()*curr->getInvC()*((MeshModel*)models[activeModel])->getmidwithz();
	//
	//// will get the LookAt transformation and its inverse
	//mat4* camToWorld = new mat4(1); mat4* worldToCam = new mat4 (1);

	//cameras[activeCamera]->LookAt(camToWorld, worldToCam, curr->getPosition(), mid);
	//

	//////mat4 new_proj = curr->get_projection() * (*camToWorld);
	//////curr->setProjection(new_proj);

	//curr->clearTransfomrationAndSetNewPosition();
	//curr->setTransformation(*camToWorld);
	//curr->setInvC(*worldToCam);
	//
	//mat4 tmp = curr->get_cTransform();
	//
	//curr->setTransformation(curr->getInvC());
	//curr->setTransformation(*camToWorld);
}

void Scene::zoom_in_active_model(float defaultStepZoom)
{
	mat4 trans = Scale(defaultStepZoom, defaultStepZoom, defaultStepZoom);
	mat4 inv = Scale(1.0 / defaultStepZoom, 1.0 / defaultStepZoom, 1.0 / defaultStepZoom);

	cameras[activeCamera]->setInvC(trans);
	cameras[activeCamera]->setTransformation(inv);

	vec4 mid = m_renderer->c_project*m_renderer->ctransfrom*(((MeshModel*)models[activeModel])->getmidwithz());
	vec3 tmp = transformVec4ToVec3(mid);
	
	vec2 mid_screen = vec2(m_renderer->getWidth() / 2, m_renderer->getHeight() / 2);

	
	mat4 t = Translate(mid_screen.x - tmp.x, mid_screen.y - tmp.y, 0);
	mat4 t_inv = Translate(-mid_screen.x + tmp.x,- mid_screen.y + tmp.y, 0);

	cameras[activeCamera]->setInvC(t);
	cameras[activeCamera]->setTransformation(t_inv);
	
	/*lookAtActiveModel();*/
	//
	//MeshModel* curr = ((MeshModel*)models[activeModel]);
	//curr->setScaledMinAndMax();

	//Camera* currcam = cameras[activeCamera];
	//currcam->left = curr->sxMin;
	//currcam->right = curr->sxMax;
	//currcam->bottom = curr->syMin;
	//currcam->top = curr->syMax;

	//currcam->zFar = curr->szMax;
	//currcam->zNear = 0;

	////currcam->aspect = curr->sxMin;
	////currcam->fovy = curr->sxMin;


	///*float edit_left_text = curr->sxMin;
	//float edit_right_text = curr->sxMax;
	//float edit_bottom_text = curr->syMin;
	//float edit_top_text = curr->syMax;

	//float edit_near_text = curr->szMin;
	//float edit_far_text = curr->szMax;

	//float edit_aspect_text = curr->sxMin;
	//float edit_fovy_text = curr->sxMin;*/
	//mat4* projection = new mat4(1);

	//if (cameras[activeCamera]->projectionType == 0) // ortho
	//{
	//	*projection = cameras[activeCamera]->Ortho(currcam->left, currcam->right, currcam->bottom, currcam->top, currcam->zNear, currcam->zFar);
	//	cameras[activeCamera]->setProjection(*projection);
	//	m_renderer->SetProjection(currcam->get_projection());

	//	/*mat4 tmp = currcam->get_cTransform();

	//	currcam->setTransformation(currcam->getInvC());
	//	currcam->setTransformation(*camToWorld);

	//	currcam->setInvC(tmp);
	//	currcam->setInvC(*worldToCam);

	//	m_renderer->SetCameraTransform(currcam->getInvC());*/
	//}
	//if (cameras[activeCamera]->projectionType == 1)
	//{
	//	*projection = cameras[activeCamera]->Frustum(currcam->left, currcam->right, currcam->bottom, currcam->top, currcam->zNear, currcam->zFar);
	//	cameras[activeCamera]->setProjection(*projection);
	//	m_renderer->SetProjection(currcam->get_projection());
	//}
	//if (cameras[activeCamera]->projectionType == 2)
	//{
	//	*projection = cameras[activeCamera]->Perspective(currcam->fovy, currcam->aspect, currcam->zNear, currcam->zFar);
	//	cameras[activeCamera]->setProjection(*projection);
	//	m_renderer->SetProjection(currcam->get_projection());
	//}
}

void Scene::zoom_out_active_model(float defaultStepZoom)
{
	mat4 inv = Scale(defaultStepZoom, defaultStepZoom, defaultStepZoom);
	mat4 trans = Scale(1.0 / defaultStepZoom, 1.0 / defaultStepZoom, 1.0 / defaultStepZoom);

	cameras[activeCamera]->setInvC(trans);
	cameras[activeCamera]->setTransformation(inv);

	vec4 mid = m_renderer->c_project * m_renderer->ctransfrom * (((MeshModel*)models[activeModel])->getmidwithz());
	vec3 tmp = transformVec4ToVec3(mid);

	vec2 mid_screen = vec2(m_renderer->getWidth() / 2, m_renderer->getHeight() / 2);


	mat4 t = Translate(mid_screen.x - tmp.x, mid_screen.y - tmp.y, 0);
	mat4 t_inv = Translate(-mid_screen.x + tmp.x, -mid_screen.y + tmp.y, 0);

	cameras[activeCamera]->setInvC(t);
	cameras[activeCamera]->setTransformation(t_inv);
}

void Scene::Set_material_for_active_model()
{
	if (activeModel != -1)
		((MeshModel*)models[activeModel])->set_material();
}

void Scene::change_light_details()
{
	Change_light_Dlg change_light_details = new Change_light_Dlg();

	if (change_light_details.DoModal() == IDOK)// open the dialog to get the params
	{
		float x_position = atof(change_light_details.x_pos);
		float y_position = atof(change_light_details.y_pos);
		float z_position = atof(change_light_details.z_pos);

		float x_direction = atof(change_light_details.x_dir);
		float y_direction = atof(change_light_details.y_dir);
		float z_direction = atof(change_light_details.z_dir);

		bool pnt = change_light_details.pnt_src;
		bool prl = change_light_details.prl_src;

		bool default_vals = change_light_details.use_default;
		bool on_or_off = change_light_details.on_or_off;
		int light_number = atoi(change_light_details.light_number);

		if (default_vals == true)
		{
			
			//vec3 new_direction = vec3(1, 1, 1);
			//vec4 new_position = vec4(m_renderer->getWidth() / 2, m_renderer->getHeight(), 600, 1);
			//*vec4 new_position = vec4(0, 0, 0, 1);*/
			//vec3 new_luminance = vec3(0.5, 0.5, 0.5); // intensity of RGB

		/*	LightType t;
			if (light_number == 0)
				t = AmbientLight;
			else if (pnt == true)
				t = PointSource;
			else
				t = ParallelLight;*/

			LightState state;
			if (on_or_off == true)
				state = ON;
			else
				state = OFF;
			
			//Light* newLight = new Light(new_direction, new_position, new_luminance, t, m_renderer->superX, m_renderer->superY, state);
			//lights[light_number] = newLight;
			/*lights.push_back(newLight);*/
			lights[light_number]->s = state;

			CColorDialog dialg;
			int st = dialg.DoModal();
			if(st == IDOK)
			{
				COLORREF a_c = dialg.GetColor();

				float r_c = GetRValue(a_c) / 255.0;
				float g_c = GetGValue(a_c) / 255.0;
				float b_c = GetBValue(a_c) / 255.0;
				vec3 light_color = vec3(r_c, g_c, b_c);
				lights[light_number]->luminance = light_color;
			}
			
			
		}
		else
		{
			CColorDialog dialg;
			if (dialg.DoModal() == IDOK)
			{
				COLORREF a_c = dialg.GetColor();

				float r_c = GetRValue(a_c) / 255.0;
				float g_c = GetGValue(a_c) / 255.0;
				float b_c = GetBValue(a_c) / 255.0;
				vec3 light_color = vec3(r_c, g_c, b_c);
				vec3 new_luminance = light_color; // intensity of RGB

				vec3 new_direction = vec3(x_direction, y_direction, z_direction);
				vec4 new_position = vec4(x_position, y_position, z_position, 1);

				LightType t;
				if (pnt == true)
					t = PointSource;
				else
					t = ParallelLight;

				LightState state;
				if (on_or_off == true)
					state = ON;
				else
					state = OFF;

				Light* newLight = new Light(new_direction, new_position, new_luminance,t,m_renderer->superX, m_renderer->superY, state);
				lights[light_number] = newLight;
				/*lights.push_back(newLight);*/
			}
		}

	}
	delete change_light_details;
}

//sh 
void Scene::addLightSource()
{
	AddLightDlg add_light = new AddLightDlg();
	/*add_light.setAddLightDetailsMode();*/

	if (add_light.DoModal() == IDOK)// open the dialog to get the params
	{
		float x_position = atof(add_light.x_pos);
		float y_position = atof(add_light.y_pos);
		float z_position = atof(add_light.z_pos);

		float x_direction = atof(add_light.x_dir);
		float y_direction = atof(add_light.y_dir);
		float z_direction = atof(add_light.z_dir);

		bool pnt = add_light.pnt_src;
		bool prl = add_light.prl_src;

		bool default_vals = add_light.use_default;
		if (default_vals == true)
			{
				vec3 new_direction = vec3(1, 1, 1);
				vec4 new_position = vec4(m_renderer->getWidth() / 2, m_renderer->getHeight(), 600, 1);
				/*vec4 new_position = vec4(0, 0, 0, 1);*/
				vec3 new_luminance = vec3(1, 1, 1); // intensity of RGB

				LightType t;
				if (pnt == true)
					t = PointSource;
				else
					t = ParallelLight;

				Light* newLight = new Light(new_direction, new_position, new_luminance, t);
				lights.push_back(newLight);
			}
		else
		{
			CColorDialog dialg;
			if (dialg.DoModal() == IDOK)
			{
				COLORREF a_c = dialg.GetColor();

				float r_c = GetRValue(a_c) / 255.0;
				float g_c = GetGValue(a_c) / 255.0;
				float b_c = GetBValue(a_c) / 255.0;				
				vec3 light_color = vec3(r_c, g_c, b_c);
				vec3 new_luminance = light_color; // intensity of RGB

				vec3 new_direction = vec3(x_direction, y_direction, z_direction);
				vec4 new_position = vec4(x_position, y_position, z_position, 1);

				LightType t;
				if (pnt == true)
					t = PointSource;
				else
					t = ParallelLight;

				Light* newLight = new Light(new_direction, new_position, new_luminance, t);
				lights.push_back(newLight);
			}
		}

	}
	delete add_light;
}


void Scene::set_ambient_color()
{
	//sh 
	// Get the selected color from the CColorDialog. 
	CColorDialog dialg;
	if (dialg.DoModal() == IDOK)
	{
		COLORREF a_c = dialg.GetColor();

		float r_c = GetRValue(a_c)/255.0;
		float g_c = GetGValue(a_c)/255.0;
		float b_c = GetBValue(a_c)/255.0;
		
		vec3 light_color = vec3(r_c, g_c, b_c);
		lights[0]->luminance.x = light_color.x;
		lights[0]->luminance.y = light_color.y;
		lights[0]->luminance.z = light_color.z;
	}
}
//*****************************************************
// camera implementations
//*****************************************************

void Camera::setTransformation(const mat4& transform)
{
	cTransform = transform * cTransform;
	CTransform_normals = mat4ToMat3(transform) * CTransform_normals;
}

void Camera::setProjection(const mat4& new_projection)
{
	projection = new_projection;
}

/*
 * @arg eye: the position of the camera
 * @arg at: the position the camera look at
 * @arg up: the upside (y) direction of the camera
 */
 //avg
void Camera::LookAt(mat4* ctw, mat4* wtc, const vec4& eye, const vec4& at, const vec4& up)
{
	mat4 camToWorld;

	/*vec4 n = normalize(eye - at);
	vec4 u = cross(normalize(up), n);
	vec4 v = cross(n, u);*/

	vec4 n = normalize(eye - at);
	vec4 u = normalize(cross(up, n));
	vec4 v = normalize(cross(n, u));

	vec4 t = vec4(0.0, 0.0, 0.0, 0.1);
	mat4 worldToCam = mat4(u, v, n, t);

	camToWorld[0][0] = u.x;
	camToWorld[0][1] = u.y;
	camToWorld[0][2] = u.z;
	camToWorld[1][0] = v.x;
	camToWorld[1][1] = v.y;
	camToWorld[1][2] = v.z;
	camToWorld[2][0] = n.x;
	camToWorld[2][1] = n.y;
	camToWorld[2][2] = n.z;

	camToWorld[3][0] = eye.x;
	camToWorld[3][1] = eye.y;
	camToWorld[3][2] = eye.z;
	camToWorld[3][3] = 1;


	*ctw = camToWorld;
	*wtc =  (worldToCam*Translate(-eye));
}


//avg 
mat4 Camera::Ortho(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{
	// values are in camera coordinates
	// we should normalize the viewing volume

	// create the orthogonal projection matrix
	mat4 M = mat4(1);
	M[2][2] = 0;

	// create the translate and scale matrices
	mat4 ST = Scale(2.0f / (right - left), 2.0f / (top - bottom), -2.0f / (zFar - zNear));
	ST[0][3] = -(left + right) / (right - left);
	ST[1][3] = -(top + bottom) / (top - bottom);
	ST[2][3] = -(zFar + zNear) / (zFar - zNear);

	return M * ST;
}

/*
 * return@ a matrix which transform an arbitrary frustum to the canonical view volume
 */

mat4 Camera::Frustum(const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar)
{
	//// shear to symmetrize the frustum (zFar and zNear do not change)
	//mat4 H = mat4(1);
	//H[0][2] = (left + right) / (2 * zNear);
	//H[1][2] = (top + bottom) / (2 * zNear);

	//// scale the frustum such that the sides intersect the projection plane at a 45-degree angle (Z far and near still do not change)
	//mat4 S = Scale((2 * zNear) / (right - left), (2 * zNear) / (top - bottom), 1);

	////transform to canonical view volume (z near and far should br transformed to the range [-1, 1]
	//// x, y coordinates should be projected to the projeaction plane
	//float alpha = (zNear + zFar) / (zNear - zFar);
	//float beta = (2 * zNear * zFar) / (zNear - zFar);
	//mat4 N = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 0.1, 0.0, 0.0), vec4(0.0, 0.0, alpha, -1.0), vec4(0.0, 0.0, beta, 0.0));

	//return N * S * H;
	mat4 m;
	int offset = 0;
	float r_width = 1.0f / (right - left);
	float r_height = 1.0f / (top - bottom);
	float r_depth = 1.0f / (zFar - zNear);
	float x = 2.0f * (r_width);
	float y = 2.0f * (r_height);
	float z = 2.0f * (r_depth);
	float A = (right + left) * r_width;
	float B = (top + bottom) * r_height;
	float C = (zFar + zNear) * r_depth;
	m[0][0] = x;
	m[0][3] = -A;
	m[1][2] = y;
	m[1][3] = -B;
	m[2][2] = -z;
	m[2][3] = -C;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[1][0] = 0.0f;
	m[1][2] = 0.0f;
	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;

	return m;
}


/*
 * @arg aspect  =w/h
 */
mat4 Camera::Perspective(const float fovy, const float aspect,
	const float mzNear, const float mzFar)
{
	float top, bottom, left, right;

	const double DEG2RAD = 3.14159265 / 180;
	double tangent = tan(fovy / 2 * DEG2RAD); // tangent of half fovY
	double height = mzNear * tangent; // half height of near plane
	double width = height * aspect; // half width of near plane
	
	left = -width;
	right = width;
	top = height;
	bottom = -height;
	zNear = mzNear;
	zFar = mzFar;	
									
	// params: left, right, bottom, top, near, far
	return Frustum(-width, width, -height, height, mzNear, mzFar);
}