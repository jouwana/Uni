#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>
#include "material_and_light.h"
#include "SetMaterial_Dlg.h"

using namespace std;

class MeshModel : public Model
{
protected:
	MeshModel() {}

	
	//add more attributes
	mat4 _world_transform = mat4(1.0);
	mat4 _normal_transform = mat4(1.0);

	//my attributes:
	int face_counter = 0;
	int norm_count = 0;
	
	vec3* normal_positions = nullptr;
	

	vector<vec3> face_scaled_normal;
	vec3* vertex_scaled_normal;

	bool firstDraw = true;
	float aspectRatio = 1;
	float ori_xRange, ori_yRange;


	mat4 model_transform = mat4(1.0);
	mat3 model_inv_trans = mat3(1.0);
	mat3 world_inv_trans = mat3(1.0);
	

	//my helper functions:

	/*
	 * @info: finds and sets the min and max value of x,y,z in model
	 *	finds the absolute min and max, NOT one for each axis.
	 */
	void setMinAndMax();



	/*
	 * @info: finds the maximum and minimum values for Y and X,
	 *	calculates the range the image takes up on the y axis
	 *	and the x axis, and from the calculates the aspect ratio
	 *	of the image.
	 *	saves the value in the variable 'aspectRatio'.
	 */
	void findAspectRatio();

public:
	string file_name = "";
	string texture_fileName = "";
	string normal_texture_fileName = "";
	vec2* texture_positions = nullptr; // wont use it this hw.

	int get_normCount()
	{
		return norm_count;
	}
	vec3* vertex_positions = nullptr;
	vec4* scaled_positions = nullptr;
	int size;
	float min = 0, max = 0;
	float yMin, yMax, xMin, xMax, zMin, zMax;
	float syMin, syMax, sxMin, sxMax, szMin, szMax;

	//avg 
	materialStruct material; // model material (uniform)


		/*
	 * @param: width and height of screen, and vertex positions array
	 * @info: updates the positions of the vertices, and scales them
	 *	to fit the width and height of the screen.
	 *	saves the new values of vertices in scaled_positions.
	 *	DOES NOT change normal values!
	 */
	void updatePositions(int width, int height);
	
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);

	virtual void draw(Renderer* rend);
	vec2 findMid();
	vec3* get_vNormals();
	vec3* get_fNormals();
	vec3* get_interpolated_Normals();
	
	vec2* get_vTexture();
	mat3 get_normal_transform_matrix();
	mat4 get_modelT() { return model_transform; }
	mat4 get_worldT() { return _world_transform; }
	vector<vec4> bounds; // bounding box
	
	//avg 
	void set_material(); // set the model material. Allows different material for front and back faces.

	vector<vec3> compute_normal_per_face();
	
	vector<vec4> compute_bounding_box();

	/*
	 * @param: trans -> an object transformation matrix
	 * @info: update model_transformation matrix to be
	 *	trans*model_transformation
	 */
	void setModelTransformation(const mat4& trans, const mat4& inv)
	{
		model_transform = trans * model_transform;
		model_inv_trans = model_inv_trans * mat4ToMat3(inv);
	}

	/*
	 * @param: trans -> an object transformation matrix
	 * @info: update world_transformation matrix to be
	 *	trans*world_transformation
	 */
	void setWorldTransformation(const mat4& trans, const mat4& inv)
	{
		_world_transform = trans * _world_transform;
		world_inv_trans = world_inv_trans * mat4ToMat3(inv);
	}


	/*
	 * a  call to draw the face normals
	 */
	void drawFaceNormals(Renderer* rend);

	/*
	 * a  call to draw the vertex normals
	 */
	//avg 
	void drawVertexNormals(Renderer* rend);
	/*
	 * check if x,y are inside model
	 */

	bool clickedOn(float x, float y, mat4& trans, mat4& proj);

	/*
	 * set min and max after they were scaled
	 */
	void setScaledMinAndMax();

	void draw_bounding_box(Renderer* rend);

	//new extra func
	vec4 getmidwithz()
	{
		setScaledMinAndMax();
		return vec4((sxMax + sxMin) / 2, (syMax + syMin) / 2, (szMax+szMin)/2,1);
	}
};
