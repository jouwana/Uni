//#include "PngWrapper.h"
#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;
#define INDEX(width,x,y,c) (x+y*width)*3+c

struct FaceIdcs
{
	int v[4];
	int vn[4];
	int vt[4];

	FaceIdcs()
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream & aStream)
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for(int i = 0; i < 3; i++)
		{
			aStream >> std::ws >> v[i] >> std::ws;
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> std::ws;
			if (aStream.peek() == '/')
			{
				aStream >> c >> std::ws >> vn[i];
				continue;
			}
			else
				aStream >> vt[i];
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vn[i];
		}
	}
};

vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

void MeshModel::setMinAndMax()
{
	//give min and max starting values.
	min = vertex_positions[0].x;
	max = vertex_positions[0].x;

	//go over all vertices and check which on has the min/max values 
	for(int i = 0; i < face_counter*3;i++)
	{
		float x, y, z;
		x = vertex_positions[i].x;
		y = vertex_positions[i].y;
		z = vertex_positions[i].z;
		min = min(x, min);
		min = min(y, min);
		min = min(z, min);
		max = max(x, max);
		max = max(y, max);
		max = max(z, max);
	}
}


void MeshModel::findAspectRatio()
{
	//give starter values/
	xMin = vertex_positions[0].x;
	xMax = vertex_positions[0].x;
	yMin = vertex_positions[0].y;
	yMax = vertex_positions[0].y;
	zMin = zMax = vertex_positions[0].z;
	for (int i = 0; i < face_counter * 3; i++)
	{
		float x, y, z;
		x = vertex_positions[i].x;
		y = vertex_positions[i].y;
		z = vertex_positions[i].z;
		xMin = min(x, xMin);
		yMin = min(y, yMin);
		xMax = max(x, xMax);
		yMax = max(y, yMax);
		zMin = min(z, zMin);
		zMax = max(z, zMax);
	}
	//find the range the image uses in x axis and y axis
	ori_xRange = abs(xMin) + abs(xMax);
	ori_yRange = abs(yMin) + abs(yMax);
	

	//calculate aspect ratio.
	aspectRatio = ori_xRange / ori_yRange; // (width of image /height of image)

}


//new removed some parts which were there to help us resize window, but ruined other parts of the
//new code when i tried to resample, it still works, mostly, but we dont really need to resize window
void MeshModel::updatePositions(int width, int height)
{	//if this is the first time we render, initialize array
	if (firstDraw)
	{
		scaled_positions = new vec4[face_counter * 3];
		//set the minimum and maximum values
		setMinAndMax();
		firstDraw = false;
	}

	height -= 10;

	float percent;
	if (aspectRatio > 1)
	{
		if ((float)height < (float)width / aspectRatio)
		{
			percent = (float)height / ori_yRange;
			width = (float)height * aspectRatio;
			height = ori_xRange * percent;
		}
		else
		{
			percent = (float)width / ori_xRange;
			height = ori_yRange * percent * aspectRatio;
		}
	}
	else
	{
		if ((float)width < (float)height * aspectRatio)
		{
			percent = (float)width / ori_xRange;
			height = (float)width / aspectRatio;
			width = ori_yRange * percent;
		}
		else
		{
			percent = (float)height / ori_yRange;
			width = ori_xRange * percent / aspectRatio;
		}
	}
	percent = (float)height / ori_yRange;

	/*find the lowest y point position on screen, we will use this to make sure
	 *that the image always starts from the bottom of the screen.
	 *we wont need to remove this, as moving and rotating image will be done in renderer
	*/


	float adjustY = height / 2 + yMin / (abs(min) + abs(max)) * height;
	float adjustX = width / 2 + xMin / (abs(min) + abs(max)) * width;


	for (int i = 0; i < face_counter * 3; i++)
	{
		float x = vertex_positions[i].x;
		float y = vertex_positions[i].y;
		float z = vertex_positions[i].z;
		// subtract adjustX and adjustY IF you want to send full width & height.
		//x = width / 2 + x / (abs(min) + abs(max)) * width - adjustX;
		
		//y = height / 2 + y / (abs(min) + abs(max)) * height - adjustY + 10;
		/*y = height / 2 + y / (abs(min) + abs(max)) * height - adjustY;*/

		x = x / (abs(min) + abs(max));
		y = y / (abs(min) + abs(max));
		z = z / (abs(min) + abs(max)); // scale p using the same percentage.
		scaled_positions[i].x = x;
		scaled_positions[i].y = y;
		scaled_positions[i].z = z;
		scaled_positions[i].w = 1;

	}
	bounds.clear();
	bounds = compute_bounding_box();
	mat4 T = _world_transform * model_transform;

	for (int i = 0; i < face_counter * 3; i++)
	{
		scaled_positions[i] = T * scaled_positions[i];
	}

	for (int i = 0; i < bounds.size(); i++)
	{
		bounds[i] = T * bounds[i];
	}
}


MeshModel::MeshModel(string fileName)
{
	loadFile(fileName);

	//TODO : dialog
	//sh 
	set_material();
}

MeshModel::~MeshModel(void)
{
	if (vertex_positions != nullptr)
		delete[] vertex_positions;
	if (texture_positions != nullptr)
		delete[] texture_positions;
	if (scaled_positions != nullptr)
		delete[] scaled_positions;
	if (normal_positions != nullptr)
		delete[] normal_positions;
}

void MeshModel::loadFile(string fileName)
{
	file_name = fileName;
	// set the file name for texture image (png);
	int index_s = fileName.find_last_of("\\");
	int index_d = fileName.find_last_of(".");
	texture_fileName = fileName.substr(0, index_d) + "_texture.png";
	normal_texture_fileName = fileName.substr(0, index_d) + "_NormalMap.png";
	cout << texture_fileName << endl;
	cout << normal_texture_fileName << endl;
	//texture_fileName = "C:\Users\avigailco\Documents\234325 Computer Graphics\obj_examples\textures\spot_texture.png";

	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	//my attributes;
	vector<vec3> normals;
	vector<vec2> textures;
	face_counter = 0;

	bool normals_exit = false;
	// while not end of file
	while (!ifile.eof())
	{
		// get line
		string curLine;
		getline(ifile, curLine);

		// read type of the line
		istringstream issLine(curLine);
		string lineType;

		issLine >> std::ws >> lineType;

		// based on the type parse data
		//exp : "v" at start of line is vertex.
		if (lineType == "v") /*BUG*/
			vertices.push_back(vec3fFromStream(issLine));
		//exp : "f" at start of line is face
		else if (lineType == "f") /*BUG // shouldnt be anymore*/
		{
			faces.push_back(issLine);
			face_counter++;
		}
		//exp : need to also get textures and normals
		else if (lineType == "vn") 
		{
			normals.push_back(vec3fFromStream(issLine));
			normals_exit = true;
		}
		else if (lineType == "vt")
		{
			textures.push_back(vec2fFromStream(issLine));
		}
		else if (lineType == "#" || lineType == "")
		{
			// comment / empty line
		}
		else
		{
			/*cout<< "Found unknown line Type \"" << lineType << "\"";*/
		}
	}
	//Vertex_positions is an array of vec3. Every three elements define a triangle in 3D.
	//If the face part of the obj is
	//f 1 2 3
	//f 1 3 4
	//Then vertex_positions should contain:
	//vertex_positions={v1,v2,v3,v1,v3,v4}

	//exp : each face has 3 vertices, max 3 normals & textures
	vertex_positions = new vec3[face_counter * 3];
	normal_positions = new vec3[face_counter * 3];
	texture_positions = new vec2[face_counter * 3];

	
	// iterate through all stored faces and create triangles
	int k=0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			/*exp  : it->v[i] has the number of the vertex, -1 becomes
			 *					the location of it in "vertices", the same goes for normals and textures;
			 */
			
			vertex_positions[k] = vertices[it->v[i]-1];
			
			//normals & textures could be empty, depends on file
			//we also assume file is correct and wont have negative indices
			if (!normals_exit)
				normal_positions[k] = vec3(it->v[1]-1,k,0); //number of vertex in vertices array, position of vertexNormal in normals. 
			else if (it->vn[i] != 0)
			{
				normal_positions[k] = normalize(normals[it->vn[i] - 1]);
				norm_count++;
			}
			if (it->vt[i] != 0)
				texture_positions[k] = textures[it->vt[i] - 1];

			
			k++;
		}
	}
	size = face_counter * 3;
	setMinAndMax();
	findAspectRatio();
	for(int i = 0; i <face_counter*3;i++)
	{
		vertex_positions[i].x /= (abs(min) + abs(max));
		vertex_positions[i].y /= (abs(min) + abs(max));
		vertex_positions[i].z /= (abs(min) + abs(max));
	}
	updatePositions(1280,720);


	if(!normals_exit)
	{
		vector<vec3> fnorm = compute_normal_per_face();
		vec4* vertNorms = new vec4[vertices.size()]; //vec 4 to also save number of normals
		for (int i = 0; i < vertices.size(); i++)
			vertNorms[i] = vec4(0,0,0,0);
		for (int i = 0; i<face_counter;i++)
		{
			for(int j = 0; j<3;j++)
			{
				int pos = normal_positions[3 * i + j].x;
				if(dot(vertNorms[pos], vec4(fnorm[i], 1)) >= 0)
					vertNorms[pos] += normalize(vec4(fnorm[i], 1));
				
			}
		}
		for (int i = 0; i < face_counter; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int pos = normal_positions[3 * i + j].x;
				int last = max(0, i - 2);
				//vec3 avgFaceNorms;
				//if (last < face_counter - 5)
				//	avgFaceNorms = (fnorm[last] + fnorm[last + 1] + fnorm[last + 2] + fnorm[last + 3] + fnorm[last + 4]) / 5;
				//else avgFaceNorms = fnorm[last];
				normal_positions[3 * i + j] = normalize((transformVec4ToVec3(vertNorms[pos]))); //+ avgFaceNorms)/2);
			}
		}
		fnorm.clear();
		delete[] vertNorms;
	}

	
	faces.clear();
	vertices.clear();
	normals.clear();
	textures.clear();
}


//avg 
void MeshModel::set_material()
{
	// TODO: add a dialog box to choose parameters for material (change the different colors).
	SetMaterial_Dlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		vec3 emissive_color = vec3(0.0, 0.0, 0.0);
		float diffuse_color = atof(dlg.diffuse_value);
		float specular_color = atof(dlg.specular_val);
		bool use_def = dlg.use_default_check;

		//set non_uniform material
		material.non_uniform = dlg.non_uniform;

		GLfloat ambient_color = 1;
		GLfloat alpha = 0.4;// alpha, for the specular component

		if (dlg.self_lighting == true) // self lighting model
		{
			material.self_lighting = true;
			CColorDialog dialg;

			if (dialg.DoModal() == IDOK)
			{
				COLORREF a_c = dialg.GetColor();

				float r_c = GetRValue(a_c) / 255.0;
				float g_c = GetGValue(a_c) / 255.0;
				float b_c = GetBValue(a_c) / 255.0;

				emissive_color = vec3(r_c, g_c, b_c);
			}
		}
		else
		{
			material.self_lighting = false;
		}


		if (!use_def)
		{
			material.ambient = ambient_color;
			material.emission = emissive_color;
			material.diffuse = diffuse_color;
			material.specular = specular_color;
			material.shininess = alpha;
		}
		else
		{
			GLfloat diffuse_color = 0.5;// fraction of  diffuse light reflected from surface
			GLfloat specular_color = 0.5;//fraction of  specular light reflected from surface

			material.ambient = ambient_color;
			material.emission = emissive_color;
			material.diffuse = diffuse_color;
			material.specular = specular_color;
			material.shininess = alpha;

		}
	}
}

vector<vec3> MeshModel::compute_normal_per_face()
{
	vector<vec3> normals = vector<vec3>();

	for (int i=0; i < face_counter*3; i++)
	{
		//  3 point of triangle face
		vec3 vtx1 = vec3((scaled_positions[i]).x, (scaled_positions[i]).y, (scaled_positions[i]).z);
		i++;
		vec3 vtx2 = vec3(scaled_positions[i].x, scaled_positions[i].y, scaled_positions[i].z);
		i++;
		vec3 vtx3 = vec3(scaled_positions[i].x, scaled_positions[i].y, scaled_positions[i].z);


		// compute vectors
		vec3 u = vtx1 - vtx2;
		vec3 v = vtx3 - vtx2;

		// cross product of u X v
		vec3 normal = cross(v, u);
		normal = normalize(normal);
		normals.push_back(normal);
	}
	return normals;
}

void MeshModel::setScaledMinAndMax()
{
	try {
		sxMin = scaled_positions[0].x;
		sxMax = scaled_positions[0].x;
		syMin = scaled_positions[0].y;
		syMax = scaled_positions[0].y;
		szMin = scaled_positions[0].z;
		szMax = scaled_positions[0].z;

		// find the limits in each axis
		for (int i = 0; i < face_counter * 3; i++)
		{
			float x, y, z;
			x = scaled_positions[i].x;
			y = scaled_positions[i].y;
			z = scaled_positions[i].z;
			sxMin = min(x, sxMin);
			syMin = min(y, syMin);
			szMin = min(z, szMin);
			sxMax = max(x, sxMax);
			syMax = max(y, syMax);
			szMax = max(z, szMax);
		}
	}
	catch (...)
	{
		sxMin = -1;
		sxMax = -1;
		syMin = -1;
		syMax = -1;
		szMin = -1;
		szMax = -1;
	}
	
}

void MeshModel::draw_bounding_box(Renderer* rend)
{
	rend->draw_bounding_box(scaled_positions, face_counter, bounds);
}

//avg 
vector<vec4> MeshModel::compute_bounding_box()
{
	vector<vec4> bounds_faces;

	setScaledMinAndMax();

	// compute the bounding box vertices
	vec4 v1 = vec4(sxMax, syMin, szMax, 1);
	vec4 v2 = vec4(sxMin, syMin, szMax, 1);
	vec4 v3 = vec4(sxMin, syMax, szMax,1);
	vec4 v4 = vec4(sxMax, syMax, szMax,1);

	vec4 v5 = vec4(sxMax, syMin, szMin, 1);
	vec4 v6 = vec4(sxMin, syMin, szMin,1);
	vec4 v7 = vec4(sxMin, syMax, szMin,1);
	vec4 v8 = vec4(sxMax, syMax, szMin,1);

	bounds_faces.push_back(v1); bounds_faces.push_back(v2);
	bounds_faces.push_back(v2); bounds_faces.push_back(v3);
	bounds_faces.push_back(v3); bounds_faces.push_back(v4); /// up
	bounds_faces.push_back(v4); bounds_faces.push_back(v1);

	bounds_faces.push_back(v5); bounds_faces.push_back(v6);
	bounds_faces.push_back(v6); bounds_faces.push_back(v7);
	bounds_faces.push_back(v7); bounds_faces.push_back(v8); /// down
	bounds_faces.push_back(v8); bounds_faces.push_back(v5);
	
	bounds_faces.push_back(v1); bounds_faces.push_back(v2);
	bounds_faces.push_back(v2); bounds_faces.push_back(v6);
	bounds_faces.push_back(v6); bounds_faces.push_back(v5); /// 1
	bounds_faces.push_back(v5); bounds_faces.push_back(v1);

	bounds_faces.push_back(v2); bounds_faces.push_back(v3);
	bounds_faces.push_back(v3); bounds_faces.push_back(v7);
	bounds_faces.push_back(v7); bounds_faces.push_back(v6); /// 2
	bounds_faces.push_back(v6); bounds_faces.push_back(v1);
	
	bounds_faces.push_back(v3); bounds_faces.push_back(v4);
	bounds_faces.push_back(v4); bounds_faces.push_back(v8);
	bounds_faces.push_back(v8); bounds_faces.push_back(v7); /// 3
	bounds_faces.push_back(v7); bounds_faces.push_back(v3);
	
	bounds_faces.push_back(v4); bounds_faces.push_back(v1);
	bounds_faces.push_back(v1); bounds_faces.push_back(v5);
	bounds_faces.push_back(v5); bounds_faces.push_back(v8); /// 4
	bounds_faces.push_back(v8); bounds_faces.push_back(v4);
	
	/*bounds_faces.push_back(v1); bounds_faces.push_back(v2); bounds_faces.push_back(v3); bounds_faces.push_back(v4); /// up
	bounds_faces.push_back(v5); bounds_faces.push_back(v6); bounds_faces.push_back(v7); bounds_faces.push_back(v8); /// down
	bounds_faces.push_back(v1); bounds_faces.push_back(v2); bounds_faces.push_back(v6); bounds_faces.push_back(v5); /// 1
	bounds_faces.push_back(v2); bounds_faces.push_back(v3); bounds_faces.push_back(v7); bounds_faces.push_back(v6); /// 2
	bounds_faces.push_back(v3); bounds_faces.push_back(v4); bounds_faces.push_back(v8); bounds_faces.push_back(v7); /// 3
	bounds_faces.push_back(v4); bounds_faces.push_back(v1); bounds_faces.push_back(v5); bounds_faces.push_back(v8); /// 4*/


	return bounds_faces;
}

void MeshModel::drawFaceNormals(Renderer* rend)
{
	//mat4 T = mat4(1);
	//
	//vector<vec3> scaled_normal = compute_normal_per_face();
	//for (int i = 0; i < face_counter; i++)
	//{
	//	scaled_normal[i] = normalize(transformVec4ToVec3(T * scaled_normal[i]));
	//	/*scaled_normal[i] = _normal_transform * normal_positions[i];*/
	//}
	//rend->compute_normal_per_face(face_counter, face_scaled_normal);
	rend->draw_normal_per_face(scaled_positions, face_counter);
}

void MeshModel::drawVertexNormals(Renderer* rend)
{
	
	/*mat4 T = _world_transform * model_transform;*/
	/*vertex_scaled_normal = normal_positions;*/

	/*for (int i = 0; i < face_counter*3; i++)
	{
		scaled_normal[i] = transformVec4ToVec3(T * scaled_normal[i]);
	}*/

	rend->draw_normal_per_vertex(scaled_positions, face_counter);

}

bool MeshModel::clickedOn(float x, float y, mat4& trans, mat4& proj)
{
	if (firstDraw)
		return false;
	setScaledMinAndMax();
	mat4 T = proj * trans;
	vec4 after_projMax = T * vec4(sxMax, syMax, szMax, 1);
	vec4 after_projMin = T * vec4(sxMin, syMin, szMin, 1);
	return (x < after_projMax.x&& x > after_projMin.x && y < after_projMax.y&& y > after_projMin.y);
}

vec2 MeshModel::findMid()
{
	setScaledMinAndMax();
	return vec2((sxMax + sxMin) / 2, (syMax + syMin) / 2);
}

//new 
vec3* MeshModel::get_vNormals()
{
	
	return normal_positions;
}

vec3* MeshModel::get_fNormals()
{
	vec3* f_normals = new vec3[3 * face_counter];
	vector<vec3> f = compute_normal_per_face();
	for (int i = 0; i < face_counter; i++)
	{
		f_normals[3 * i] = f_normals[3 * i + 1] = f_normals[3 * i + 2] = f[i];
	}

	return f_normals;
}

vec3* MeshModel::get_interpolated_Normals()
{
	//vec3* inter_normals = get_vNormals();
	//// remove effect of supersampling
	//
	//// linearly interpolate lighting intensities at the vertices over interior pixels  of the polygonn, in the image plane
	//float alpha_1, alpha_2, alpha_3;
	//vec3 p = pixel; // the pixel which we want to calculate its illumination

	//// calcuate the alpha using barycentric coordinates 
	//float area_1, area_2, area_3;

	//// Interpolating intensities along the polygon edges.
	//// first, get the 3D-coordinates for the pixel using interpolation?
	//area_1 = 0.5 * length(cross((transformVec4ToVec3(vtx1) - p), (transformVec4ToVec3(vtx2) - p)));
	//area_2 = 0.5 * length(cross((transformVec4ToVec3(vtx2) - p), (transformVec4ToVec3(vtx3) - p)));
	//area_3 = 0.5 * length(cross((transformVec4ToVec3(vtx3) - p), (transformVec4ToVec3(vtx1) - p)));

	//float area_sum = area_1 + area_2 + area_3;
	//alpha_1 = area_1 / area_sum;
	//alpha_2 = area_2 / area_sum;
	//alpha_3 = area_3 / area_sum;

	//vec3 normal = alpha_1 * normal_1 + alpha_2 * normal_2 + alpha_3 * normal_3;

	//return compute_illumination_intensity(material, vec4(p, 1), normal, maxValues);
	return nullptr;
}

vec2* MeshModel::get_vTexture()
{
	return texture_positions;
}

//new 
mat3 MeshModel::get_normal_transform_matrix()
{
	// set the vertrex normal for this model in renderer
	mat3 T = model_inv_trans * world_inv_trans;

	/*mat3 normal_transform = transpose(T * normals_c_inv);*/
	
	/*return normal_transform;*/
	return T;
}


void MeshModel::draw(Renderer* rend)
{
	int width = rend->getWidth();
	int height = rend->getHeight();
	//update positions based on screen size.
	updatePositions(width, height);

	// set the vertrex normal for this model in renderer
	mat3 T = model_inv_trans * world_inv_trans;
	vertex_scaled_normal = normal_positions;
	rend->compute_normal_per_vertex(face_counter, vertex_scaled_normal, T);
		
	// face normals
	face_scaled_normal = compute_normal_per_face();
	for (int i = 0; i < face_counter; i++)
	{
		face_scaled_normal[i] = normalize(face_scaled_normal[i]);
		/*scaled_normal[i] = _normal_transform * normal_positions[i];*/
	}
	rend->compute_normal_per_face(face_counter, face_scaled_normal);

	//call renderer so it can do the drawing.
	rend->draw(scaled_positions, face_counter, material, &face_scaled_normal);
}