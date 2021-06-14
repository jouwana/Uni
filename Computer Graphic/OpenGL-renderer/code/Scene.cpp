#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>
#include "InitShader.h"
#include "GL\freeglut.h"
#include <string>
#include "stb_image.h"



using namespace std;
void Scene::loadOBJModel(string fileName)
{
	MeshModel* model = new MeshModel(fileName);
	models.push_back(model);
	activeModel = models.size() - 1;
}


Scene::~Scene()
{
	for (int i = 0; i < models.size(); i++)
		delete (MeshModel*)models[i];
	for (int i = 0; i < cameras.size(); i++)
		delete cameras[i];
	//delete m_renderer;
	delete dialog;
}

vector<vec3> Scene::get_diffuse_lights_directions()
{
	vector<vec3> l;
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i]->type == ParallelLight && lights[i]->s == ON)
			l.push_back(lights[i]->direction);
	}
	return l;
}

vector<vec3> Scene::get_diffuse_lights_colors()
{
	vector<vec3> l;
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i]->type == ParallelLight && lights[i]->s == ON)
			l.push_back(lights[i]->luminance);
	}
	return l;
}

vector<vec4> Scene::get_specular_lights_positions()
{
	vector<vec4> l;
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i]->type == PointSource && lights[i]->s == ON)
			l.push_back(lights[i]->position);
	}
	return l;
}

vector<vec3> Scene::get_specular_lights_colors()
{
	vector<vec3> l;
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i]->type == PointSource && lights[i]->s == ON)
			l.push_back(lights[i]->luminance);
	}
	return l;
}

bool is_file_exist(string file_name)
{
	struct stat buffer;
	return (stat(file_name.c_str(), &buffer) == 0);
}

void Scene::animateColors(GLuint program)
{
	//uniform color for all the models -> can be the ambient color
	GLCall(GLuint col = glGetUniformLocation(program, "animateColor"));
	ASSERT(col != -1);

	//animate color by changing color each time function is called
	if (animateColor > 0.61f)
		incrementC = -0.005f;
	else if (animateColor < 0.01f)
		incrementC = 0.005f;
	animateColor += incrementC;
	if( colorAnimation == 0)
		GLCall(glUniform4f(col, animateColor, animateColor / 2, animateColor * 0.7f, 0.0f));
	else if (colorAnimation == 1)
	{
		animateColor += incrementC;
		GLCall(glUniform4f(col, animateColor, animateColor / 2, animateColor * 0.7f, 0.0f));
	}
	else if (colorAnimation == 2)
		GLCall(glUniform4f(col, 2 + animateColor, 0, 0, 0.0f));
}

void Scene::animateVertices(GLuint program)
{
	GLCall(GLuint col = glGetUniformLocation(program, "animateVertix"));
	ASSERT(col != -1);
	if (animateVertix > 0.21f)
		incrementV = -0.005f;
	else if (animateVertix < 0.01f)
		incrementV = 0.005f;
	animateVertix += incrementV;
	GLCall(glUniform1f(col, animateVertix));
}

void Scene::setShaderLights(GLuint program)
{
	int Gourand = (shading_alg == GOURAND) ? 1 : 0;
	GLCall(glUniform1i(glGetUniformLocation(program, "is_gourand"), Gourand));


	mat4 camera_transform;

	if (activeCamera != -1)
	{
		camera_transform = cameras[activeCamera]->getInvC();
	}
	else
	{
		camera_transform = mat4(1);
	}

	// Initialize shader lighting parameters
	vector<vec4> specular_positions_vec = get_specular_lights_positions();
	specular_positions = new vec4[specular_positions_vec.size()];
	for (int i = 0; i < specular_positions_vec.size(); i++)
		specular_positions[i] = camera_transform * specular_positions_vec[i];

	vector<vec3> diffuse_directions_vec = get_diffuse_lights_directions();
	diffuse_directions = new vec3[diffuse_directions_vec.size()];
	for (int i = 0; i < diffuse_directions_vec.size(); i++)
		diffuse_directions[i] = vec4ToVec3(camera_transform * (vec4(diffuse_directions_vec[i], 0)));

	vector<vec3> specular_colors_vec = get_specular_lights_colors();
	specular_colors = new vec3[specular_colors_vec.size()];
	for (int i = 0; i < specular_colors_vec.size(); i++)
		specular_colors[i] = specular_colors_vec[i];

	vector<vec3> diffuse_colors_vec = get_diffuse_lights_colors();
	diffuse_colors = new vec3[diffuse_colors_vec.size()];
	for (int i = 0; i < diffuse_colors_vec.size(); i++)
		diffuse_colors[i] = diffuse_colors_vec[i];


	//send vector sizes
	GLint d_count = diffuse_directions_vec.size();
	GLCall(GLuint loc_d_co = glGetUniformLocation(program, "parallal_light_count"));
	ASSERT(loc_d_co != -1);
	GLCall(glUniform1i(loc_d_co, d_count));

	GLint s_count = specular_positions_vec.size();
	GLCall(GLuint loc_s_co = glGetUniformLocation(program, "point_light_count"));
	ASSERT(loc_s_co != -1);
	GLCall(glUniform1i(loc_s_co, s_count));


	// set ambient light color
	vec3 light_ambient;
	if (lights[0]->s == ON)
		light_ambient = lights[0]->luminance;
	else
		light_ambient = vec3(0, 0, 0);
	GLCall(GLuint loc_al = glGetUniformLocation(program, "ambient_light_color"));
	ASSERT(loc_al != -1);
	GLCall(glUniform3f(loc_al, light_ambient.x, light_ambient.y, light_ambient.z));




	// set the parallal colors
	glUniform3fv(glGetUniformLocation(program, "parallal_colors_vec"), diffuse_colors_vec.size(), (const GLfloat*)(diffuse_colors));

	// set the point colors
	glUniform3fv(glGetUniformLocation(program, "point_colors_vec"), specular_colors_vec.size(), (const GLfloat*)(specular_colors));

	// set the parallal directions
	glUniform3fv(glGetUniformLocation(program, "parallal_directions_vec"), diffuse_directions_vec.size(), (const GLfloat*)(diffuse_directions));

	// set the point positions
	glUniform4fv(glGetUniformLocation(program, "point_positions_vec"), specular_positions_vec.size(), (const GLfloat*)(specular_positions));
}

GLuint Scene::setModelTexture(GLuint program, Model* mod, int pnum, GLuint loc_tc, int texture_type)
{
	MeshModel* curr = (MeshModel*)mod;

	string t_fileName;
	string path = "..\\obj_examples\\";
	switch (texture_type)
	{
	case (0): // given texture
		t_fileName = curr->texture_fileName;
		break;
	case(2): // plane
		t_fileName = path + "Stone.png";
		break;
	case(1): // sphere
		t_fileName = path + "Stone.png";
		break;
	}


	int has_texture = false;

	unsigned int texture;
	if (is_file_exist(t_fileName))
	{
		has_texture = true;
		glEnable(GL_TEXTURE_2D);

		GLCall(glGenTextures(1, &texture));
		GLCall(glBindTexture(GL_TEXTURE_2D, texture));
		// set the texture wrapping/filtering options (on the currently bound texture object)
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		// load and generate the texture
		int width, height, nrChannels;
		unsigned char* data = stbi_load(t_fileName.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			GLCall(glGenerateMipmap(GL_TEXTURE_2D));
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
		GLCall(glBindTexture(GL_TEXTURE_2D, texture));

		vec2* vTex = curr->get_vTexture();
		//bind texture of model

		GLuint textureBuffer;
		GLCall(glGenBuffers(1, &textureBuffer));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, textureBuffer));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * pnum, vTex, GL_STATIC_DRAW));

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, textureBuffer));
		GLCall(glEnableVertexAttribArray(loc_tc));
		GLCall(glVertexAttribPointer(loc_tc, 2, GL_FLOAT, GL_FALSE, 0, 0));

	}
	// has_texture
	GLCall(glUniform1i(glGetUniformLocation(program, "has_texture"), has_texture));
	//return textureBuffer;
	return (has_texture) ? texture : -1;
}

void Scene::setModelMaterial(GLuint program, Model* mod)
{
	// set the material fraction coefficients
	materialStruct m = ((MeshModel*)mod)->material;
	float material_shininess = m.shininess;
	float material_ambient = m.ambient;
	float material_diffuse = m.diffuse;
	float material_specular = m.specular;
	GLCall(glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess));
	GLCall(glUniform1f(glGetUniformLocation(program, "k_Ambient"), material_ambient));
	GLCall(glUniform1f(glGetUniformLocation(program, "k_Diffuse"), material_diffuse));
	GLCall(glUniform1f(glGetUniformLocation(program, "k_Specular"), material_specular));
}

void Scene::setShadingMode(Model* mod, vec3** vNormal, int pnum, mat3* normal_transform)
{
	MeshModel* curr = (MeshModel*)mod;

	// set the normal transform matrix for this model
	mat3 T = curr->get_normal_transform_matrix();

	mat3 normals_c_inv;
	if (activeCamera != -1)
		normals_c_inv = cameras[activeCamera]->get_normal_ctranform();
	else
		normals_c_inv = mat3(1);

	*normal_transform = transpose(T * normals_c_inv);

	if (shading_alg == GOURAND)
		(*vNormal) = curr->get_vNormals();

	else if (shading_alg == FLAT)
		(*vNormal) = curr->get_fNormals();

	else if (shading_alg == PHONG)
		(*vNormal) = curr->get_vNormals();

}

void Scene::setModelNormals(GLuint program, Model* mod, int pnum, vec3** vNormals, GLuint locN)
{
	mat3 normal_transform;
	setShadingMode(mod, vNormals, pnum, &normal_transform);

	// vertex normals used with vNormal attribute in shaders
	GLuint normalbuffer;
	GLCall(glGenBuffers(1, &normalbuffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, normalbuffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, pnum * sizeof(vec3), *vNormals, GL_STATIC_DRAW));

	// 3rd attribute buffer : normals

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, normalbuffer));
	GLCall(glEnableVertexAttribArray(locN));
	GLCall(glVertexAttribPointer(
		locN,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		0                          // array buffer offset
	));

	// bind normalMatrix transformatipn
	GLCall(GLuint locNM = glGetUniformLocation(program, "normalMatrix"));
	ASSERT(locNM != -1);
	GLCall(glUniformMatrix3fv(locNM, 1, GL_TRUE, normal_transform));

}

void Scene::setModelPositions(GLuint program, Model* mod, int pnum, GLuint loc)
{
	//save it for easier access
	vec3* positions = ((MeshModel*)mod)->vertex_positions;
	mat4 camera_transform;
	if (activeCamera != -1)
		camera_transform = cameras[activeCamera]->getInvC();
	else camera_transform = mat4(1);

	mat4 model_transform = camera_transform * ((MeshModel*)mod)->get_worldT() * ((MeshModel*)mod)->get_modelT();
	// bind normalMatrix transformatipn
	GLCall(GLuint locMM = glGetUniformLocation(program, "modelMatrix"));
	ASSERT(locMM != -1);
	GLCall(glUniformMatrix4fv(locMM, 1, GL_TRUE, model_transform));

	//bind vertices of model
	GLuint vertexBuffer;
	GLCall(glGenBuffers(1, &vertexBuffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * pnum, positions, GL_STATIC_DRAW));

	//vertices are used with vPosition attribute in shaders

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
	GLCall(glEnableVertexAttribArray(loc));
	GLCall(glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0));
}

unsigned int Scene::loadNormalTexture(char const* path, GLuint NormalTextureID, int texture_type)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	if (texture_type == 1)
	{
		path = "..\\obj_examples\\Fabric_NormalMap.png";
	}

	else if (texture_type == 2)
	{
		path = "..\\obj_examples\\Stone_NormalMap.png";
	}

	if (texture_type == 0 || is_file_exist(path) == false)
	{
		path = "";
	}

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
		GLCall(glUniform1i(NormalTextureID, 1));

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT)); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		stbi_image_free(data);
	}
	else if (path != "")
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void Scene::setModelTangentsAndBitangents(GLuint program, Model* mod, int pnum, GLuint locT, GLuint locBT, vec3** atangent, vec3** abitangent)
{
	//save it for easier access	
	vec3* positions = ((MeshModel*)mod)->vertex_positions;
	vec2* vTex = ((MeshModel*)mod)->get_vTexture();

	int face_counter = pnum / 3;

	vec3* tangent = new vec3[pnum];
	vec3* bitangent = new vec3[pnum];

	(*atangent) = tangent;
	(*abitangent) = bitangent;

	for (int i = 0; i < face_counter; i++)
	{
		vec3 pos1 = positions[3 * i];
		vec3 pos2 = positions[3 * i + 1];
		vec3 pos3 = positions[3 * i + 2];

		vec2 uv1 = vTex[3 * i];
		vec2 uv2 = vTex[3 * i + 1];
		vec2 uv3 = vTex[3 * i + 2];

		vec3 edge1 = pos2 - pos1;
		vec3 edge2 = pos3 - pos1;
		vec2 deltaUV1 = uv2 - uv1;
		vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent[3 * i].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent[3 * i].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent[3 * i].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent[3 * i] = normalize(tangent[3 * i]);

		tangent[3 * i + 1].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent[3 * i + 1].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent[3 * i + 1].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent[3 * i + 1] = normalize(tangent[3 * i + 1]);

		tangent[3 * i + 2].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent[3 * i + 2].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent[3 * i + 2].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent[3 * i + 2] = normalize(tangent[3 * i + 2]);

		// bitangent
		bitangent[3 * i].x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent[3 * i].y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent[3 * i].z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent[3 * i] = normalize(bitangent[3 * i]);

		bitangent[3 * i + 1].x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent[3 * i + 1].y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent[3 * i + 1].z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent[3 * i + 1] = normalize(bitangent[3 * i + 1]);

		bitangent[3 * i + 2].x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent[3 * i + 2].y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent[3 * i + 2].z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent[3 * i + 2] = normalize(bitangent[3 * i + 2]);
	}

	//bind vertices of model
	GLuint tangentBuffer;
	GLCall(glGenBuffers(1, &tangentBuffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * pnum, tangent, GL_STATIC_DRAW));

	//vertices are used with vPosition attribute in shaders

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer));
	GLCall(glEnableVertexAttribArray(locT));
	GLCall(glVertexAttribPointer(locT, 3, GL_FLOAT, GL_FALSE, 0, 0));


	GLuint BitangentBuffer;
	GLCall(glGenBuffers(1, &BitangentBuffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, BitangentBuffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * pnum, bitangent, GL_STATIC_DRAW));

	//vertices are used with vPosition attribute in shaders

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, BitangentBuffer));
	GLCall(glEnableVertexAttribArray(locBT));
	GLCall(glVertexAttribPointer(locBT, 3, GL_FLOAT, GL_FALSE, 0, 0));
}

unsigned int Scene::loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}


void Scene::setModelPositions_forEM(GLuint program, Model* mod, int pnum, GLuint loc)
{
	//save it for easier access
	vec3* positions = ((MeshModel*)mod)->vertex_positions;
	mat4 camera_transform;
	if (activeCamera != -1)
		camera_transform = cameras[activeCamera]->getInvC();
	else camera_transform = mat4(1);

	// model transform
	mat4 model_matrix = ((MeshModel*)mod)->get_modelT();
	GLCall(GLuint locMM = glGetUniformLocation(program, "model"));
	ASSERT(locMM != -1);
	GLCall(glUniformMatrix4fv(locMM, 1, GL_TRUE, model_matrix));

	// view transform (world + camera)
	mat4 view_matrix = camera_transform * ((MeshModel*)mod)->get_worldT();
	GLCall(GLuint loc_vm = glGetUniformLocation(program, "view"));
	ASSERT(loc_vm != -1);
	GLCall(glUniformMatrix4fv(loc_vm, 1, GL_TRUE, view_matrix));

	//bind vertices of model
	GLuint vertexBuffer;
	GLCall(glGenBuffers(1, &vertexBuffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * pnum, positions, GL_STATIC_DRAW));

	//vertices are used with vPosition attribute in shaders
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
	GLCall(glEnableVertexAttribArray(loc));
	GLCall(glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0));
}

void Scene::setModelNormals_forEM(GLuint program, Model* mod, int pnum, vec3** vNormals, GLuint locN)
{
	// vertex normals used with vNormal attribute in shaders
	MeshModel* curr = (MeshModel*)mod;
	(*vNormals) = curr->get_vNormals();
	GLuint normalbuffer;
	GLCall(glGenBuffers(1, &normalbuffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, normalbuffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, pnum * sizeof(vec3), *vNormals, GL_STATIC_DRAW));

	// 3rd attribute buffer : normals
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, normalbuffer));
	GLCall(glEnableVertexAttribArray(locN));
	GLCall(glVertexAttribPointer(
		locN,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		0                          // array buffer offset
	));
}

void Scene::skybox_rendering(unsigned int skyboxTexture)
{
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	//GLCall(glEnable(GL_DEPTH_TEST));

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	GLCall(glGenVertexArrays(1, &skyboxVAO));
	GLCall(glGenBuffers(1, &skyboxVBO));
	GLCall(glBindVertexArray(skyboxVAO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW));
	GLCall(glEnableVertexAttribArray(0));

	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
	//GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));

	// load textures
	unsigned int cubemapTexture = skyboxTexture;//loadCubemap(faces);

	// shader configuration 
	GLCall(glUseProgram(program_sc));

	//GLuint skyboxTextureID = glGetUniformLocation(program_sc, "skybox");
	//ASSERT(skyboxTextureID != -1);
	//GLCall(glUniform1i(skyboxTextureID, 0));

	GLCall(glDepthFunc(GL_LEQUAL));  // change depth function so depth test passes when values are equal to depth buffer's content
	GLCall(glUseProgram(program_sc));

	// view transform (world + camera)
	mat4 camera_transform;
	if (activeCamera != -1)
		camera_transform = cameras[activeCamera]->getInvC();
	else camera_transform = mat4(1);
	mat4 view_matrix = camera_transform;// *((MeshModel*)models[i])->get_worldT();
	// remove translation from the view matrix
	view_matrix[0][3] = 0;
	view_matrix[1][3] = 0;
	view_matrix[2][3] = 0;
	view_matrix[3][3] = 1;
	view_matrix[3][0] = 0;
	view_matrix[3][1] = 0;
	view_matrix[3][2] = 0;
	GLCall(GLuint loc_vm = glGetUniformLocation(program_sc, "view"));
	ASSERT(loc_vm != -1);
	GLCall(glUniformMatrix4fv(loc_vm, 1, GL_TRUE, view_matrix));

	// orojection matrix
	GLCall(GLuint locPM = glGetUniformLocation(program_sc, "projection"));
	mat4 projection;
	projection = mat4(1);
	if (activeCamera != -1)
	{
		projection = cameras[activeCamera]->get_projection();
		if (cameras[activeCamera]->projectionType == 0) // ORTHO
			projection[2][2] = 1; //RETURN Z-BUFFER
	}
	ASSERT(locPM != -1);
	GLCall(glUniformMatrix4fv(locPM, 1, GL_TRUE, projection));

	// skybox cube
	GLCall(glBindVertexArray(skyboxVAO));
	//GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture));
	GLCall(glDrawArrays(GL_TRIANGLES, 0, 36));
	//GLCall(glBindVertexArray(0));
	GLCall(glDepthFunc(GL_LESS));
}

void Scene::draw_With_Env_Map(int normalMapping)
{
	glEnable(GL_DEPTH_TEST);

	string path = "..\\obj_examples\\skybox\\";
	vector<std::string> faces;
	faces.push_back(path + "right.jpg");
	faces.push_back(path + "left.jpg");
	faces.push_back(path + "top.jpg");
	faces.push_back(path + "bottom.jpg");
	faces.push_back(path + "front.jpg");
	faces.push_back(path + "back.jpg");

	//skybox_rendering(faces);

	//GLuint skyboxTextureID = glGetUniformLocation(program_em, "skybox");
	unsigned int skyboxTexture = loadCubemap(faces);

	GLCall(glUseProgram(program_em));
	glEnable(GL_DEPTH_TEST);

	GLCall(GLuint loc_unm = glGetUniformLocation(program_em, "use_normalMapping"));
	//ASSERT(loc_unm != -1);
	GLCall(glUniform1i(loc_unm, normalMapping));

	GLCall(GLuint loc_cm = glGetUniformLocation(program_em, "cameraPos"));
	//ASSERT(loc_unm != -1);
	vec3 Camera_pos = vec3(0, 0, 0);
	GLCall(glUniform3fv(loc_cm, 0, Camera_pos));

	// projection matrix
	GLCall(GLuint locPM = glGetUniformLocation(program_em, "projection"));
	mat4 projection;
	projection = mat4(1);
	if (activeCamera != -1)
	{
		projection = cameras[activeCamera]->get_projection();
		if (cameras[activeCamera]->projectionType == 0) // ORTHO
			projection[2][2] = 1; //RETURN Z-BUFFER
	}
	ASSERT(locPM != -1);
	GLCall(glUniformMatrix4fv(locPM, 1, GL_TRUE, projection));

	//add all the models into the same out buffer
	for (int i = 0; i < models.size(); i++)
	{
		//save it as it is used multiple times
		const int pnum = ((MeshModel*)models[i])->size;

		//make sure positions are updated for drag/select functions
		((MeshModel*)models[i])->updatePositions(width, height);

		//set model attribute locations
		GLCall(GLuint locN = glGetAttribLocation(program_em, "aNormal"));
		ASSERT(locN != -1);
		GLCall(GLuint loc = glGetAttribLocation(program_em, "aPos"));
		ASSERT(loc != -1);


		//variable we will need to delete
		vec3** vNormals = new vec3*;


		//textureID = setModelTexture(program, models[i], pnum, loc_tc, texture_type);
		setModelPositions_forEM(program_em, models[i], pnum, loc);
		setModelNormals_forEM(program_em, models[i], pnum, vNormals, locN);



		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture));
		GLCall(glDrawArrays(GL_TRIANGLES, 0, pnum));



		delete vNormals; // vertex normals

	}

	skybox_rendering(skyboxTexture);
}

void Scene::drawDemo(int normalMapping, int texture_type)
{
	//might be able to move this to scene constructor, as there is no need to do it more than once
	/// texture_type
	/// 0: normal texture
	///  1: canonical mapping 1 (plane)
	///  2: canonnical mapping 2 (sphere)
	///  3: environmnt mapping (reflection)
	if (texture_type == 3)// env mappnig
	{
		draw_With_Env_Map(normalMapping);
	}
	else
	{
		//set up the program to use these shaders
		GLCall(glUseProgram(program));

		//clear color (to black page) as well as clearing depth buffer
		//allow built-in Zbuffer Algorithm
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		//color animation
		if (colorAnimation == 0)
			animateColor = -1;
		else
		{
			if (animateColor < -0.5f)
				animateColor = 0;

		}
		animateColors(program);

		if (!vertexAnimation)
			animateVertix = -1;
		else
		{
			if (animateVertix < -0.5f)
				animateVertix = 0;
		}
		animateVertices(program);

		GLCall(GLuint loc_tt = glGetUniformLocation(program, "texture_type"));
		ASSERT(loc_tt != -1);
		GLCall(glUniform1i(loc_tt, texture_type));

		GLCall(GLuint loc_unm = glGetUniformLocation(program, "use_normalMapping"));
		ASSERT(loc_unm != -1);
		GLCall(glUniform1i(loc_unm, normalMapping));

		int turb = turbText;
		//** Turbulence Texture
		GLCall(GLuint text = glGetUniformLocation(program, "turbText"));
		ASSERT(text != -1);
		GLCall(glUniform1i(text, turb));

		// *************** camera ********************

		//BUG: ILLUSTARTION PROBLEM!! ESPEICALYY ORTHO (Z BUFFER WONT WORK THEN)

		GLCall(GLuint locPM = glGetUniformLocation(program, "Projection"));
		mat4 projection;
		projection = mat4(1);
		if (activeCamera != -1)
		{
			projection = cameras[activeCamera]->get_projection();
			if (cameras[activeCamera]->projectionType == 0) // ORTHO
				projection[2][2] = 1; //RETURN Z-BUFFER
		}
		ASSERT(locPM != -1);
		GLCall(glUniformMatrix4fv(locPM, 1, GL_TRUE, projection));

		// *************** camera end ********************

		setShaderLights(program);

		//add all the models into the same out buffer
		for (int i = 0; i < models.size(); i++)
		{
			//save it as it is used multiple times
			const int pnum = ((MeshModel*)models[i])->size;

			//make sure positions are updated for drag/select functions
			((MeshModel*)models[i])->updatePositions(width, height);


			//set model attribute locations
			GLCall(GLuint loc_tc = glGetAttribLocation(program, "vTexCoor"));
			ASSERT(loc_tc != -1);
			GLCall(GLuint locN = glGetAttribLocation(program, "vNormal"));
			ASSERT(locN != -1);
			GLCall(GLuint loc = glGetAttribLocation(program, "vPosition"));
			ASSERT(loc != -1);

			// normal mapping
			GLuint loc_t;
			GLuint loc_bt;

			//new 
			if (normalMapping == 1)
			{
				GLCall(loc_t = glGetAttribLocation(program, "aTangent"));
				//ASSERT(loc_t != -1);
				GLCall(loc_bt = glGetAttribLocation(program, "aBitangent"));
				//ASSERT(loc_bt != -1);
			}

			//variable we will need to delete
			vec3** vNormals = new vec3*;
			vec3** atangent = new vec3*;
			vec3** abitangent = new vec3*;

			//Tooning
			int Tooning = toonShading;
			unsigned int textureID;
			unsigned int normalMap;

			if (toonShading)
			{
				shading_alg = PHONG; //uses same light calculation
				GLCall(GLuint locT = glGetUniformLocation(program, "toonShading"));
				ASSERT(locT != -1);
				GLCall(glUniform1i(locT, 2));

				setModelMaterial(program, models[i]);
				textureID = setModelTexture(program, models[i], pnum, loc_tc, texture_type);
				setModelPositions(program, models[i], pnum, loc);
				setModelNormals(program, models[i], pnum, vNormals, locN);

				if (normalMapping)
				{
					setModelTangentsAndBitangents(program, models[i], pnum, loc_t, loc_bt, atangent, abitangent);
					string nt_fileName = ((MeshModel*)models[i])->normal_texture_fileName;
					GLuint NormalTextureID = glGetUniformLocation(program, "NormalTextureSampler");
					normalMap = loadNormalTexture(nt_fileName.c_str(), NormalTextureID, texture_type);
				}
				//draw silhouette 
				GLCall(glDrawArrays(GL_TRIANGLES, 0, pnum));
			}

			GLCall(GLuint locT = glGetUniformLocation(program, "toonShading"));
			ASSERT(locT != -1);
			GLCall(glUniform1i(locT, Tooning));


			setModelMaterial(program, models[i]);
			textureID = setModelTexture(program, models[i], pnum, loc_tc, texture_type);
			setModelPositions(program, models[i], pnum, loc);
			setModelNormals(program, models[i], pnum, vNormals, locN);

			//new 

			if (normalMapping)
			{
				setModelTangentsAndBitangents(program, models[i], pnum, loc_t, loc_bt, atangent, abitangent);
				string nt_fileName = ((MeshModel*)models[i])->normal_texture_fileName;
				GLuint NormalTextureID = glGetUniformLocation(program, "NormalTextureSampler");
				normalMap = loadNormalTexture(nt_fileName.c_str(), NormalTextureID, texture_type);
			}

			//draw triangles
			GLCall(glDrawArrays(GL_TRIANGLES, 0, pnum));
			GLCall(glDisable(GL_TEXTURE_2D));

			//disable attributes
			glDisableVertexAttribArray(loc);
			glDisableVertexAttribArray(locN);
			glDisableVertexAttribArray(loc_tc);


			glBindTexture(textureID, 0);
			//glDeleteTextures(1, &textureID);

			//new 
			if (normalMapping)
			{
				glDisableVertexAttribArray(loc_t);
				glDisableVertexAttribArray(loc_bt);
				//glBindTexture(normalMap, 0);
				//glDeleteTextures(1, &normalMap);
			}

			//delete vec4
			if (shading_alg == FLAT)
				delete[] * vNormals;

			delete vNormals;
			if (normalMapping)
			{
				delete atangent;
				delete abitangent;
			}
		}

		delete[] specular_positions;
		delete[] diffuse_directions;
		delete[] specular_colors;
		delete[] diffuse_colors;
	}
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
	for (int i = 0; i < models.size(); i++)
	{
		if (cameraTransforming)
			m_renderer->setColors(vec3(0, 1, 0.5));
		else if (worldTransforming)
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

				vec4 new_position = cameras[activeCamera]->get_projection() * cameras[activeCamera]->getInvC() * trans * cameras[i]->position;
				m_renderer->draw_camera(new_position);
				m_renderer->drawLine(new_position.x, new_position.y, new_position.x + 2, new_position.y - 2);
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

void Scene::transformWorld(mat4& transformation, mat4& inv)
{
	if (activeModel != -1)
		((MeshModel*)models.at(activeModel))->setWorldTransformation(transformation, inv);
	else
	{
		for (vector<Model*>::iterator it = models.begin(); it != models.end(); ++it)
		{
			((MeshModel*)(*it))->setWorldTransformation(transformation, inv);
		}
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
	((MeshModel*)models.at(activeModel))->setModelTransformation(transformation, inv);
}

vec2 Scene::getMidOfActiveModel()
{
	if (activeModel == -1)
		return vec2(0, 0);
	return ((MeshModel*)models[activeModel])->findMid();
}

// face normals
void Scene::drawFaceNormals()
{
	/*if (activeCamera != -1)
		scene->setNormals_cinv(cameras[activeCamera]->get_normal_ctranform());*/

	GLCall(glUseProgram(program_2));

	//clear color (to black page) as well as clearing depth buffer
	/*GLCall(glClearColor(0.0, 0.0, 0.0, 1.0));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));*/

	//allow built-in Zbuffer Algorithm
	glEnable(GL_DEPTH_TEST);
	for (int i = 0; i < models.size(); i++)
	{
		// set line color
		vec3 color;
		if (worldTransforming)
			color = vec3(1, 0, 1);
		else if (i == activeModel)
			color = vec3(1, 0, 1);//blue
		else
			color = vec3(1, 1, 1);//white

		GLCall(GLuint loc_nl = glGetUniformLocation(program_2, "line_color"));
		//ASSERT(loc_nl != -1);
		GLCall(glUniform3f(loc_nl, color.x, color.y, color.z));

		MeshModel* curr = ((MeshModel*)models[i]);
		//curr->updatePositions(width, height);

		// set the normal Trsnform matrix
		mat3 normal_transform;
		//mat3 T = curr->get_normal_transform_matrix();
		mat3 normals_c_inv;
		if (activeCamera != -1)
			normals_c_inv = cameras[activeCamera]->get_normal_ctranform();
		else
			normals_c_inv = mat3(1);
		normal_transform = transpose(normals_c_inv);

		// set the model Trsnform matrix
		mat4 camera_transform;
		if (activeCamera != -1)
			camera_transform = cameras[activeCamera]->getInvC();
		else camera_transform = mat4(1);

		mat4 model_transform = camera_transform * (curr->get_worldT()) * (curr->get_modelT());

		// calculate the vertices for drawing normals
		vec3* pos = curr->vertex_positions;
		const int pnum = ((MeshModel*)models[i])->size;

		vec3* v_n_tmp = curr->get_fNormals();
		vec4* v_n_p = new vec4[2 * pnum / 3];

		for (int i = 0; i < pnum / 3; i++)
		{
			vec3 v1 = transformVec4ToVec3(model_transform * vec4(pos[3 * i], 1));
			/*v1 = v1 / v1.w;*/
			vec3 v2 = transformVec4ToVec3(model_transform * vec4(pos[3 * i + 1], 1));
			/*v2 = v2 / v2.w;*/
			vec3 v3 = transformVec4ToVec3(model_transform * vec4(pos[3 * i + 2], 1));
			/*v3 = v3 / v3.w;*/
			float midx = (v1.x + v2.x + v3.x) / 3;
			float midy = (v1.y + v2.y + v3.y) / 3;
			float midz = (v1.z + v2.z + v3.z) / 3;
			vec4 face_center = vec4(midx, midy, midz, 1);

			v_n_p[2 * i] = face_center; // face center
			//v_n_p[2 * i + 1] = vec4(normalize(v_n_tmp[i]) * 0.2, 1) + face_center;
			vec3 tmp = 0.1 * normalize(normal_transform * v_n_tmp[3 * i]);
			v_n_p[2 * i + 1] = vec4(tmp.x, tmp.y, tmp.z, 0) + face_center;
		}

		// bind the normals				
		GLuint f_normalsBuffer;
		GLCall(glGenBuffers(1, &f_normalsBuffer));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, f_normalsBuffer));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 2 * pnum / 3, v_n_p, GL_STATIC_DRAW));

		//vertices are used with vPosition attribute in shaders
		GLCall(GLuint loc_fn = glGetAttribLocation(program_2, "normal_lines"));
		ASSERT(loc_fn != -1);
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, f_normalsBuffer));
		GLCall(glEnableVertexAttribArray(loc_fn));
		GLCall(glVertexAttribPointer(loc_fn, 4, GL_FLOAT, GL_FALSE, 0, 0));

		GLCall(glDrawArrays(GL_LINES, 0, 2 * pnum / 3));

		GLCall(glDisableVertexAttribArray(loc_fn));
		GLCall(glDisableVertexAttribArray(loc_nl));
		delete[] v_n_p;
	}
}

void Scene::drawVertexNormals()
{
	/*if (activeCamera != -1)
		scene->setNormals_cinv(cameras[activeCamera]->get_normal_ctranform());*/


	GLCall(glUseProgram(program_2));

	//clear color (to black page) as well as clearing depth buffer
	/*GLCall(glClearColor(0.0, 0.0, 0.0, 1.0));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));*/

	//allow built-in Zbuffer Algorithm
	glEnable(GL_DEPTH_TEST);
	for (int i = 0; i < models.size(); i++)
	{
		// set line color
		vec3 color;
		if (worldTransforming)
			color = vec3(1, 0, 1);
		else if (i == activeModel)
			color = vec3(1, 0, 1);//blue
		else
			color = vec3(1, 1, 1);//white

		GLCall(GLuint loc_nl = glGetUniformLocation(program_2, "line_color"));
		//ASSERT(loc_nl != -1);
		GLCall(glUniform3f(loc_nl, color.x, color.y, color.z));

		MeshModel* curr = ((MeshModel*)models[i]);

		// set the normal Trsnform matrix
		mat3 normal_transform;
		mat3 T = curr->get_normal_transform_matrix();
		mat3 normals_c_inv;
		if (activeCamera != -1)
			normals_c_inv = cameras[activeCamera]->get_normal_ctranform();
		else
			normals_c_inv = mat3(1);
		normal_transform = transpose(T * normals_c_inv);

		// set the model Trsnform matrix
		mat4 camera_transform;
		if (activeCamera != -1)
			camera_transform = cameras[activeCamera]->getInvC();
		else camera_transform = mat4(1);

		mat4 model_transform = camera_transform * (curr->get_worldT()) * (curr->get_modelT());

		// calculate the vertices for drawing normals
		vec3* pos = curr->vertex_positions;

		int vn_num = curr->get_normCount();
		vec3* v_n_tmp = curr->get_vNormals();
		vec4* v_n_p = new vec4[2 * vn_num];

		for (int i = 0; i < vn_num; i++)
		{
			vec4 t_p = model_transform * vec4(pos[i], 1);
			v_n_p[2 * i] = t_p;
			v_n_p[2 * i + 1] = vec4(normalize((normal_transform * v_n_tmp[i])) * 0.1, 1) + t_p;
			/*v_n_p[2 * i] = vec4(0.3, 0.5,0.5, 1);
			v_n_p[2 * i + 1] = vec4(0.9, 0.5, 0.5, 1);*/
		}

		// bind the normals				
		GLuint v_normalsBuffer;
		GLCall(glGenBuffers(1, &v_normalsBuffer));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, v_normalsBuffer));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * vn_num * 2, v_n_p, GL_STATIC_DRAW));

		//vertices are used with vPosition attribute in shaders
		GLCall(GLuint loc_vn = glGetAttribLocation(program_2, "normal_lines"));
		ASSERT(loc_vn != -1);
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, v_normalsBuffer));
		GLCall(glEnableVertexAttribArray(loc_vn));
		GLCall(glVertexAttribPointer(loc_vn, 4, GL_FLOAT, GL_FALSE, 0, 0));

		GLCall(glDrawArrays(GL_LINES, 0, vn_num * 2));

		GLCall(glDisableVertexAttribArray(loc_vn));
		GLCall(glDisableVertexAttribArray(loc_nl));
		delete[] v_n_p;
	}


}

void Scene::draw_bounding_box()
{

	GLCall(glUseProgram(program_2));

	//clear color (to black page) as well as clearing depth buffer
	/*GLCall(glClearColor(0.0, 0.0, 0.0, 1.0));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));*/

	//allow built-in Zbuffer Algorithm
	glEnable(GL_DEPTH_TEST);
	for (int i = 0; i < models.size(); i++)
	{
		// set line color
		vec3 color;
		if (worldTransforming)
			color = vec3(1, 0, 1);
		else if (i == activeModel)
			color = vec3(1, 0, 1);//blue
		else
			color = vec3(1, 1, 1);//white

		GLCall(GLuint loc_nl = glGetUniformLocation(program_2, "line_color"));
		//ASSERT(loc_nl != -1);
		GLCall(glUniform3f(loc_nl, color.x, color.y, color.z));

		MeshModel* curr = ((MeshModel*)models[i]);
		//curr->updatePositions(width, height);

		// set the model Trsnform matrix
		mat4 camera_transform;
		if (activeCamera != -1)
			camera_transform = cameras[activeCamera]->getInvC();
		else camera_transform = mat4(1);

		mat4 model_transform = camera_transform;// *(curr->get_worldT())* (curr->get_modelT());

		// calculate the vertices for drawing normals
		vector<vec4> bounds = curr->bounds;//compute_bounding_box();
		const int pnum = bounds.size();
		vec4* b = new vec4[pnum];

		for (int i = 0; i < pnum; i++)
		{
			b[i] = model_transform * bounds[i];
		}

		// bind the normals				
		GLuint v_boundsBuffer;
		GLCall(glGenBuffers(1, &v_boundsBuffer));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, v_boundsBuffer));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * pnum, b, GL_STATIC_DRAW));

		//vertices are used with vPosition attribute in shaders
		GLCall(GLuint loc_vn = glGetAttribLocation(program_2, "normal_lines"));
		ASSERT(loc_vn != -1);
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, v_boundsBuffer));
		GLCall(glEnableVertexAttribArray(loc_vn));
		GLCall(glVertexAttribPointer(loc_vn, 4, GL_FLOAT, GL_FALSE, 0, 0));

		GLCall(glDrawArrays(GL_LINES, 0, pnum));

		GLCall(glDisableVertexAttribArray(loc_vn));
		GLCall(glDisableVertexAttribArray(loc_nl));
		delete[] b;
	}
}

bool Scene::activateOnClick(float x, float y)
{
	//bool i = false; // is T identity
	//mat4 T = mat4(1);
	//if (activeCamera != -1)
	//{
	//	mat4 T = m_renderer->c_project * m_renderer->ctransfrom;
	//	i = true;
	//}
	for (int i = models.size() - 1; i >= 0; i--)
	{
		if (((MeshModel*)models[i])->clickedOn(x, y, mat4(1), mat4(1)))
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
	/*vec4 pos = vec4(1280()/2, 720()/2, 1, 1);*/
	vec4 pos = vec4(1280 / 2, 720 / 2, 0, 1);

	mat4 t = mat4(1);

	vec4 direction = vec4(0, 0, -1, 1);

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
			if (edit_fovy_text == 0)
				c->projectionType = 1;
		}
		else if (dialog.pres_checkbox == false && dialog.orthogonal_checkbox == true)
			projection_type = 0; // same as default

		mat4 projection = mat4(1); // default value
		if (projection_type == 0) // Ortho
		{
			//*projection = Ortho(edit_left_text, edit_right_text, edit_bottom_text, edit_top_text, edit_near_text, edit_far_text);
			projection = cameras[activeCamera]->Ortho(edit_left_text, edit_right_text, edit_bottom_text, edit_top_text, edit_near_text, edit_far_text);
		}

		else
			projection = cameras[activeCamera]->Perspective(edit_fovy_text, edit_aspect_text, edit_near_text, edit_far_text);
		//{
		//	if (edit_fovy_text == 0 || edit_aspect_text == 0)
		//		glFrustum(edit_left_text, edit_right_text, edit_bottom_text, edit_top_text, edit_near_text, edit_far_text);
		//	else
		//		//*projection = glm::perspective(edit_fovy_text, edit_aspect_text, edit_near_text, edit_far_text);
		//		/*gluPerspective(edit_fovy_text, edit_aspect_text, edit_near_text, edit_far_text);				*/
		//}

		cameras[activeCamera]->setProjection(projection);

	}

}

//new look at active model, might not work without the added extra transformation that are written as comment
void Scene::lookAtActiveModel()
{
	if (activeModel == -1 || activeCamera == -1)
		return;

	vec4 mid = m_renderer->c_project * m_renderer->ctransfrom * (((MeshModel*)models[activeModel])->getmidwithz());
	vec3 tmp = transformVec4ToVec3(mid);

	vec2 mid_screen = vec2(1280 / 2, 720 / 2);


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

	vec4 mid = m_renderer->c_project * m_renderer->ctransfrom * (((MeshModel*)models[activeModel])->getmidwithz());
	vec3 tmp = transformVec4ToVec3(mid);

	vec2 mid_screen = vec2(1280 / 2, 720 / 2);


	mat4 t = Translate(mid_screen.x - tmp.x, mid_screen.y - tmp.y, 0);
	mat4 t_inv = Translate(-mid_screen.x + tmp.x, -mid_screen.y + tmp.y, 0);

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

	vec2 mid_screen = vec2(1280 / 2, 720 / 2);


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
			//vec4 new_position = vec4(1280() / 2, 720(), 600, 1);
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
			if (st == IDOK)
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

				Light* newLight = new Light(new_direction, new_position, new_luminance, t, m_renderer->superX, m_renderer->superY, state);
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
			/*vec4 new_position = vec4(1280 / 2, 720, 600, 1);*/
			vec4 new_position = vec4(0, 0, 0, 1);
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

		float r_c = GetRValue(a_c) / 255.0;
		float g_c = GetGValue(a_c) / 255.0;
		float b_c = GetBValue(a_c) / 255.0;

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
	*wtc = (worldToCam * Translate(-eye));
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
	double width = height / aspect; // half width of near plane

	left = -width;
	right = width;
	top = height;
	bottom = -height;
	zNear = mzNear;
	zFar = mzFar;




	// params: left, right, bottom, top, near, far
	return Frustum(-width, width, -height, height, mzNear, mzFar);


}



/***************************--OLD CODE--**************************/

//vec4 Scene::get_illumination_according_to_algorithm(materialStruct& material, vec4& p, vec3& normal, int alg_id)
//{
//	if (alg_id == SHADING_FLAT)
//	{
//
//	}
//	if (alg_id == SHADING_PHONG)
//	{
//
//	}
//	if (alg_id == SHADING_GOURAND)
//	{
//		return 
//	}
//}

//vec4 Scene::compute_illumination_intensity(materialStruct& material, vec4& p, vec3& normal)
//{
//	vec3 final_light = vec3(0, 0, 0);
//	if (lights.size() != 0)
//	{
//		if (material.non_uniform)
//		{
//			final_light = vec3((p.y / maxValues.y) - ((int)p.y % 30) / 100.0f, (p.x / maxValues.x) - ((int)p.x % 30) / 100.0f,
//				(p.x / maxValues.x) - ((int)p.x % 25) / 100.0f);
//			final_light /= 5.0f;
//
//		}
//		if (lights[0]->s == ON)
//			final_light += material.ambient * vec3(lights[0]->luminance.x, lights[0]->luminance.y, lights[0]->luminance.z);
//	}
//
//	vec4 l; // direction to light source
//	vec4 r; // direction of reflectd ray
//	vec4 v; // direction to COP
//	vec4 half;
//
//	v = active_camera_position - p;
//	v = normalize(v);
//
//	for (int i = 1; i < lights.size(); i++)
//	{
//		Light light = *(lights[i]);
//		if (light.s == OFF) // the light is off
//			continue;
//
//		if (light.type == PointSource) // the light direction is the vector from the pointSource to the vertex
//		{
//			l = normalize(ctransfrom * (light.position) - p); // light_direction
//			half = normalize(l + normalize(-p));
//		}
//		else if (light.type == ParallelLight) // the light direction is the same for all of the scene
//		{
//			l = normalize(ctransfrom * (vec4(light.direction, 0)));
//			half = normalize(l + normalize(-p));
//		}
//
//		//r = normalize(2 * (max(dot(l, normal), 0)) * normal - l);
//
//		GLfloat diffuse, specular;
//		// calculate the diffuse and specular reflection component foreach color (RGB)
//		float d = dot(l, normal); // maybe dot(normal, postitino/dirctino)??
//		if (d > 0.0)
//			diffuse = material.diffuse * d;
//		else
//			diffuse = 0.0;
//
//
//		float s = dot(vec4(normal, 1), half);
//		if (s > 0.0)
//			specular = material.specular * pow(s, material.shininess);
//		else
//			specular = 0;
//
//		final_light += (diffuse + specular) * light.luminance;
//	}
//
//	/*if (final_light.x > 1)
//		final_light.x = 1;
//	if (final_light.y > 1)
//		final_light.y = 1;
//	if (final_light.z > 1)
//		final_light.z = 1;*/
//	return final_light;
//}
//new 