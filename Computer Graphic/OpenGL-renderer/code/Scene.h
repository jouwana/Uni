#pragma once
#include "Change_light_Dlg.h"
#include "CameraDlg.h"
#include "AddLightDlg.h"
#include "SetAmbientDlg.h"
#include <vector>
#include <string>
#include "Renderer.h"
#include "afxcolorpickerctrl.h"
#include "InitShader.h"

#define SHADING_FLAT 1
#define SHADING_GOURAND 2
#define SHADING_PHONG 3

using namespace std;

class Model {
protected:
	virtual ~Model(){}
	//exp changed the virtual function to take a renderer.
	void virtual draw(Renderer* rend)=0;
};


class Camera {
	mat4 cTransform;
	mat4 projection;
	mat4 invCTransform = mat4(1.0);
	mat3 CTransform_normals = mat3(1.0);
	

	//new extra attributes
	float XcameraRotation = 0;
	float YcameraRotation = 0;
	float ZcameraRotation = 0;
	
public:
	vec4 position;
	vec4 direction;
	int projectionType = 0;// 0: ortho, 1: perspective, 2: perspective with fovy
	
	float left;
	float right;
	float bottom;
	float top;

	float zNear;
	float zFar;

	float aspect;
	float fovy;
	
	Camera(vec4 new_position, vec4 new_direction)
	{
		this->position = new_position;
		this->direction = new_direction;
	}

	mat4 getInvC()
	{
		return invCTransform;
	}
	void setInvC(const mat4& inv)
	{
		invCTransform = invCTransform * inv;
	}
	
	//avg 
	mat4 get_cTransform()
	{
		return cTransform;
	}

	mat3 get_normal_ctranform()
	{
		return CTransform_normals;
	}

	mat4 get_projection()
	{
		return projection;
	}


	//new some extra funcs
	vec4 getPosition()
	{
		return position;
	}

	void clearTransfomrationAndSetNewPosition()
	{
		//position = cTransform * position;
		cTransform = mat4(1);
		invCTransform = mat4(1);
	}

	
	//avgend 
	void setTransformation(const mat4& transform);
	void setProjection(const mat4& new_projection);
	//new added default here
	void LookAt(mat4* ctw, mat4* wtc, const vec4& eye, const vec4& at, const vec4& up =vec4(0, 1, 0, 0));
	mat4 Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	mat4 Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	mat4 Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);

	//new extra funcs
	float getXCameraRotation()
	{
		return XcameraRotation;
	}

	void setXCameraRotation(float deg)
	{
		XcameraRotation += deg;
	}
	float getYCameraRotation()
	{
		return YcameraRotation;
	}

	void setYCameraRotation(float deg)
	{
		YcameraRotation += deg;
	}
	float getZCameraRotation()
	{
		return ZcameraRotation;
	}

	void setZCameraRotation(float deg)
	{
		ZcameraRotation += deg;
	}

};

class Scene {

	
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer* m_renderer;

	CameraDlg dialog = new CameraDlg(); // camera parameters dialogs

	GLCall(GLuint program_2 = InitShader("normals_vshader.glsl", "normals_fshader.glsl"));
	GLCall(GLuint program = InitShader("minimal_vshader.glsl", "minimal_fshader.glsl"));
	GLCall(GLuint program_em = InitShader("envMap_vshader.glsl", "envMap_fshader.glsl"));
	GLCall(GLuint program_sc = InitShader("skycube_vshader.glsl", "skycube_fshader.glsl"));

public:
	ShadingAlgorithm shading_alg = PHONG; // defulat value
	void set_shading_alg(ShadingAlgorithm sa)
	{
		shading_alg = sa;
	}

	int width, height;
	vector<Model*> models;
	float animateColor = -1.0f, incrementC = 0.005f,incrementV = 0.005f, animateVertix=-1.0f;
	bool bounds = false, fnormals = false, vnormals = false,
			 vertexAnimation = false,toonShading = false , turbText = false;
	int colorAnimation = 0;

	//Scene() {};
	Scene(Renderer* renderer) : m_renderer(renderer) 
	{
		Light* ambient = new Light();
		ambient->type = AmbientLight;

		//sh 	

		//allow to user to choose intensity of each color component
		/*GLfloat Ir = 0.1;
		GLfloat Ig = 0.1;
		GLfloat Ib = 0.1;*/
		
		GLfloat Ir = 0.25;
		GLfloat Ig = 0.25;
		GLfloat Ib = 0.25;

		ambient->luminance = vec3(Ir, Ig, Ib);
		lights.push_back(ambient);

		
	}

	~Scene();
	vec4 get_illumination_according_to_algorithm(materialStruct& material, vec4& p, vec3& normal, int alg_id);
	vec4 compute_illumination_intensity(materialStruct& material, vec4& p, vec3& normal);
	void loadOBJModel(string fileName);
	void draw();
	vector<vec3> get_diffuse_lights_directions();
	vector<vec3> get_diffuse_lights_colors();
	vector<vec4> get_specular_lights_positions();
	vector<vec3> get_specular_lights_colors();

	vec4* specular_positions;
	vec3* diffuse_directions;
	vec3* specular_colors;
	vec3* diffuse_colors;

	
	void animateColors(GLuint program);
	void animateVertices(GLuint program);
	void setShaderLights(GLuint program);
	GLuint setModelTexture(GLuint program, Model* mod, int pnum, GLuint loc_tc, int texture_typ);
	void setModelMaterial(GLuint program, Model* mod);
	void setShadingMode(Model* mod, vec3** vNormal, int pnum, mat3* normal_transform);
	void setModelNormals(GLuint program, Model* mod, int pnum, vec3** vNormals, GLuint locN);
	void setModelPositions(GLuint program, Model* mod, int pnum, GLuint loc);
	unsigned int loadNormalTexture(char const* path, GLuint NormalTextureID, int texture_type);
	void setModelTangentsAndBitangents(GLuint program, Model* mod, int pnum, GLuint locT, GLuint locBT, vec3** atangent, vec3** abitangent);
	unsigned int loadCubemap(vector<std::string> faces);
	void setModelPositions_forEM(GLuint program, Model* mod, int pnum, GLuint loc);
	void setModelNormals_forEM(GLuint program, Model* mod, int pnum, vec3** vNormals, GLuint locN);
	void draw_With_Env_Map(int normalMapping);
	void skybox_rendering(unsigned int skyboxTexture);
	void drawDemo(int normalMapping, int texture_type);

	int activeModel = -1;
	int activeLight = -1;
	int activeCamera = -1;

	//my attributes
	bool worldTransforming = false;
	bool cameraTransforming = false;
	bool drawCameras = false;

	//my helper functions

	// scene initializtion, addition of an ambient light to the scene
	Scene();
	void set_ambient_color();

	void addLightSource();

	void set_drawCameras(bool c_draw)
	{
		drawCameras = c_draw;
	}

	/*
	 * @param: mod: a model (prim)
	 * @info: adds mod to the models vector
	 */
	void loadPrimModel(Model* mod)
	{
		models.push_back(mod);
		activeModel = models.size() - 1;
	}


	/*
	 * @info send transformations to the camera
	 */
	void transformCamera(mat4& transformation, mat4& inv);

	/*
	 *@info: send the transformation matrix to the active model
	 */
	void transformActiveModel(mat4& transformation, mat4& inv);

	/*
	 *@info: send the transformation matrix to the all models
	 */
	void transformWorld(mat4& transformation, mat4& inv);

	/*
	* @info: changes active model to the next one
	*/
	void nextActiveModel()
	{
		if (models.size() == 0)
			return;
		activeModel++;
		if (activeModel == models.size())
			activeModel = 0;
	}

	/*
	 * @info: changed active camera to the next one
	 */
	void nextActiveCamera()
	{
		if (cameras.size() == 0)
			return;
		activeCamera++;
		if (activeCamera == cameras.size())
			activeCamera = 0;
	}

	vec2 getMidOfActiveModel();

	void drawFaceNormals();

	void drawVertexNormals();

	void draw_bounding_box();

	bool activateOnClick(float x, float y);

	void addNewCamera();


	//new extra funcs

	float getActiveCameraXRotation()
	{
		if (activeCamera == -1)
			return 0;
		return cameras[activeCamera]->getXCameraRotation();
	}

	void setActiveCameraXRotation(float deg)
	{
		if (activeCamera == -1)
			return;
		cameras[activeCamera]->setXCameraRotation(deg);
	}
	float getActiveCameraYRotation()
	{
		if (activeCamera == -1)
			return 0;
		return cameras[activeCamera]->getYCameraRotation();
	}

	void setActiveCameraYRotation(float deg)
	{
		if (activeCamera == -1)
			return;
		cameras[activeCamera]->setYCameraRotation(deg);
	}
	float getActiveCameraZRotation()
	{
		if (activeCamera == -1)
			return 0;
		return cameras[activeCamera]->getZCameraRotation();
	}

	void setActiveCameraZRotation(float deg)
	{
		if (activeCamera == -1)
			return;
		cameras[activeCamera]->setZCameraRotation(deg);
	}

	void lookAtActiveModel();
	void zoom_in_active_model(float defaultStepZoom);
	void zoom_out_active_model(float defaultStepZoom);


	////new helping function that updates light position when we supersize
	//void updateLightLocations()
	//{
	//	for (int i = 0; i < lights.size(); i++)
	//		if (lights[i]->type == PointSource) //new only affects pointsource, as paralel is only a direction
	//			lights[i]->position = vec4(lights[i]->origPosition.x * m_renderer->superX, lights[i]->origPosition.y * m_renderer->superY,
	//				lights[i]->origPosition.z * ((m_renderer->superX)), lights[i]->origPosition.w);
	//	//new the value for Z was mostly found through trial and error, if fits great on 2x2 resampling, and good enough for 3x3
	//	//new though some problems remain, and some items end up in extra shade (with 3x3) -> 4x4 gives me purple color, but its
	//	//new too slow to keep anyway, so we wont use it
	//}

	void Set_material_for_active_model();
	void change_light_details();
	/*Light* scene_set_and_return_active_light(int id)
	{
		activeLight = id;
		return lights[activeLight];
	}*/

	//added light info printer
	void print_lights_info()
	{
		cout.setf(ios::fixed, ios::floatfield);
		cout.precision(3);
		for (int i = 0; i < lights.size(); i++)
		{
			cout << "	Light ID: '" << i << "'." << endl;
			cout << "		Light Type: ";
			switch (lights[i]->type)
			{
			case AmbientLight:
				cout << "Ambient";
				break;
			case PointSource:
				cout << "Point Source" << endl;
				cout << "		Position (X,Y,Z,W): " << lights[i]->position;
				break;
			case SpotLight:
				cout << "Spotlight";
				break;
			case ParallelLight:
				cout << "Parallel light" << endl;
				cout << "		Direction (X,Y,Z): " << lights[i]->direction;
				break;
			}
			cout << endl;
			
			cout << "		Color (R,G,B): " << lights[i]->luminance << endl;
			cout << "		On/Off (0/1): " << lights[i]->s << endl;
			cout << " - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;

		}
	}
};