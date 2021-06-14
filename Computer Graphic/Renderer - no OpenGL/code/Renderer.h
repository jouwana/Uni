#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"
#include "material_and_light.h"


using namespace std;
#define INDEX(width,x,y,c) (x+y*width)*3+c


class Fog
{
	float maxDistance, minDistance;
	vec3 fogColour = vec3(0.0, 0.0, 0.0);
public:
	Fog(float maxD, float minD) : maxDistance(maxD), minDistance(minD) {}
	vec3 mix(vec3 colour, float z)
	{
		float fogFactor = (maxDistance - z) / (maxDistance - minDistance);
		fogFactor = (fogFactor < 0.0) ? 0.0 : (fogFactor > 1.0) ? 1.0 : fogFactor;
		vec3 newColor = fogColour * (fogFactor)+colour * (1 - fogFactor);
		return (newColor > fogColour) ? fogColour : newColor;
	}
};

enum blur
{
	None, FULL_SCREEN, BLOOM
};

class Renderer
{
	vector<Light*> lights;

	float *m_zbuffer = nullptr; // width*height
	int m_width, m_height;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	//my added attribute
	vec3 color = vec3(1, 1, 1); //color white.

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex; 
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	float* m_outBuffer = nullptr; // 3*width*height
	
	//bl 
	float* blur_outBuffer = nullptr; // 3*width*height
	void bloom();
	void blurring();
	void full_screen_blur();
	blur blurring_mode = None;


	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();
	
	//zb 
	Fog fog = Fog(100.0, 0.0);
	vec3* v_normals;
	vector<vec3> f_normals;
	void Renderer::DrawTriangles(const vec4* vertex_positions, const vec3* vertices, int size, vector<vec3>* f_normals, vec3* v_normals, materialStruct& material);
	
	//avg 
	void plotLineLow(int x0, int y0, int x1, int y1);
	void plotLineHigh(int x0, int y0, int x1, int y1);
	void drawLine(int x0, int y0, int x1, int y1);

	
	void SetCameraTransform(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();

	

	//my attributes
	mat4 ctransfrom = mat4(1);
	mat4 c_project = mat4(1);
	mat3 normals_c_inv = mat3(1);
	vec4 active_camera_position;

	void Set_active_camera_postion(vec4& p)
	{
		active_camera_position = p;
	}
	
	//my helper func
	/*
	 * @info: returns width of renderer
	 */
	int getWidth() { return m_width; }
	/*
	* @info: returns height of renderer
	*/
	int getHeight() { return m_height; }

	/*
	 * @params: C -> vec3 of 1s and 0s
	 * @info: sets color vector of renderer based on C.
	 */
	void setColors(vec3 C)
	{
		color = vec3(C);
	}

	// set the camera inverse transfornmation matrix
	void setNormals_cinv(mat3& T)
	{
		normals_c_inv = T;
	}
	
	/*
	 * @param:  width and height of the new screen
	 * @info:  deletes the old buffer and creates a new
	 *	 one based on the parameters sent.
	 */
	void updateBuffer(int width, int height);

	/*
	 * @params: a vertex array, and its size/3
	 * @info: draws the vertices on the buffer.
	 *	every 3 vertices in a row make a triangle. 
	 * NEED TO switch buffers after this function
	 */
	void draw(vec4* vertex_positions, const int size, materialStruct& material, vector<vec3>* normal_per_face);
	
	void compute_normal_per_face(const int size, vector<vec3> normal_per_face);
	void draw_normal_per_face(vec4* vertex_positions, const int size);
	
	void draw_bounding_box(vec4* vertex_positions, const int size, vector<vec4> bounds);

	void compute_normal_per_vertex(const int size, vec3* normal_per_vertex, mat3& T);
	void draw_normal_per_vertex(vec4* vertex_positions, const int size);

	void draw_camera(vec4 pos);

	//sh 
	ShadingAlgorithm shadind_alg = FLAT; // defulat value
	void set_shading_alg(ShadingAlgorithm sa)
	{
		shadind_alg = sa;
	}

	// ilumination according to phong reflection model
	vec3 compute_illumination_intensity(materialStruct& material, vec4& p, vec3& normal, vec2& maxValues);

	//// shading algorithms
	// calculate the final color of a polygon using the face normal, consider all light's sources
	vec3 flat_shading_algorithm(vec4& vtx1, vec4& vtx2, vec4& vtx3, materialStruct material, vec3& normal, vec2& maxValues);

	// calculate the final color of a polygon using the vertex colors, and interpolation intensity over polygon interior, consider all light's sources
	vec3 gourand_shading_algorithm(vec4& vtx1, vec4& vtx2, vec4& vtx3, materialStruct material, vec3& normal_1, vec3& normal_2, vec3& normal_3, vec3 pixel, vec2& maxValues);

	// calculate the final color of a polygon, consider all light's sources
	// interpolate (at the vertices in the image space) normal vectors instead of ilumination intensitites
	// apply the ilumination eqation foreach interior pixel with its own (interpolated) noraml.
	vec3 phong_shading_algorithm(vec4& vtx1, vec4& vtx2, vec4& vtx3, materialStruct material, vec3& normal_1, vec3& normal_2, vec3& normal_3, vec3 pixel, vec2& maxValues);

	// get lights from scene
	void SetLights(vector<Light*> s_lights)
	{
		lights = s_lights;
	}


	// implement full screen blur
	// implement light bloom (gausian filter on the bright pixels and combine it with the original image

	GLfloat superX = 1, superY = 1;
	GLfloat* m_superOutBuffer = nullptr;
	void downSizeAndDraw();
	
	// super sampling info
	void setSuperSampleVars(float x, float y)
	{
		superX = x;
		superY = y;
		updateBuffer(m_width, m_height);
	}
	bool enable_fog = false;
};

