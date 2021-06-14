#include "stdafx.h"
#include "PrimMeshModel.h"

PrimMeshModel::PrimMeshModel()
{
	initCubeVertices();
	initCubePositions();
	vertex_positions = Cube_Positions;
	primType = CUBE;
}


void PrimMeshModel::initCubeVertices()
{
	Cube_vertices.push_back(vec3(1, -1, -1));
	Cube_vertices.push_back(vec3(1, -1, 1));
	Cube_vertices.push_back(vec3(-1, -1, 1));
	Cube_vertices.push_back(vec3(-1, -1, -1));
	Cube_vertices.push_back(vec3(1, 1, -0.999999));
	Cube_vertices.push_back(vec3(0.999999, 1, 1.000001));
	Cube_vertices.push_back(vec3(-1, 1, 1));
	Cube_vertices.push_back(vec3(+1, 1, -1));
}

void PrimMeshModel::initCubePositions()
{
	Cube_Positions = new vec3[12 * 3];
	Cube_Positions[0] = Cube_vertices[1];
	Cube_Positions[1] = Cube_vertices[2];
	Cube_Positions[2] = Cube_vertices[3];
	Cube_Positions[3] = Cube_vertices[7];
	Cube_Positions[4] = Cube_vertices[6];
	Cube_Positions[5] = Cube_vertices[1];
	Cube_Positions[6] = Cube_vertices[4];
	Cube_Positions[7] = Cube_vertices[5];
	Cube_Positions[8] = Cube_vertices[1];
	Cube_Positions[9] = Cube_vertices[5];
	Cube_Positions[10] = Cube_vertices[6];
	Cube_Positions[11] = Cube_vertices[2];
	Cube_Positions[12] = Cube_vertices[2];
	Cube_Positions[13] = Cube_vertices[6];
	Cube_Positions[14] = Cube_vertices[7];
	Cube_Positions[15] = Cube_vertices[0];
	Cube_Positions[16] = Cube_vertices[3];
	Cube_Positions[17] = Cube_vertices[7];
	Cube_Positions[18] = Cube_vertices[0];
	Cube_Positions[19] = Cube_vertices[1];
	Cube_Positions[20] = Cube_vertices[3];
	Cube_Positions[21] = Cube_vertices[4];
	Cube_Positions[22] = Cube_vertices[7];
	Cube_Positions[23] = Cube_vertices[5];
	Cube_Positions[24] = Cube_vertices[0];
	Cube_Positions[25] = Cube_vertices[4];
	Cube_Positions[26] = Cube_vertices[1];
	Cube_Positions[27] = Cube_vertices[1];
	Cube_Positions[28] = Cube_vertices[5];
	Cube_Positions[29] = Cube_vertices[2];
	Cube_Positions[30] = Cube_vertices[3];
	Cube_Positions[31] = Cube_vertices[2];
	Cube_Positions[32] = Cube_vertices[7];
	Cube_Positions[33] = Cube_vertices[4];
	Cube_Positions[34] = Cube_vertices[0];
	Cube_Positions[35] = Cube_vertices[7];
}

void PrimMeshModel::CubeUpdatePos(Renderer* rend)
{
	face_counter = 12;
	vertex_positions = Cube_Positions;
	findAspectRatio();
	updatePositions(rend->getWidth() / 2, rend->getHeight() / 2);
	/*for (int i = 0; i < 36; i++)
	{
		cout << Cube_Positions[i] << "   ,    " << scaled_positions[i] << endl;
	}*/
}

void PrimMeshModel::drawCube(Renderer* rend)
{
	CubeUpdatePos(rend);

	int width = rend->getWidth();
	int height = rend->getHeight();
	//update positions based on screen size.
	updatePositions(width, height);


	// face normals
	face_scaled_normal = compute_normal_per_face();
	for (int i = 0; i < face_counter; i++)
	{
		face_scaled_normal[i] = normalize(face_scaled_normal[i]);
		/*scaled_normal[i] = _normal_transform * normal_positions[i];*/
	}

	// set the vertrex normal for this model in renderer
	mat3 T = model_inv_trans * world_inv_trans;
	rend->compute_normal_per_vertex(face_counter, vertex_scaled_normal, T);

	//call renderer so it can do the drawing.
	rend->draw(scaled_positions, 12, material, &face_scaled_normal);
}

void PrimMeshModel::drawVertexNormals(Renderer* rend)
{

}
