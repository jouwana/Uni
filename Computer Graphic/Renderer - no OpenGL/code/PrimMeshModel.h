#pragma once
#include "MeshModel.h"
#include "stdafx.h"

class PrimMeshModel : public MeshModel
{
protected:
	enum {CUBE, NONE } primType;
	
	vector<vec3> Cube_vertices;
	vec3* Cube_Positions;

	/*
	 * @info: initialize cube vertices
	 */
	void initCubeVertices();

	/*
	 * @info: initialize positions
	 */
	void initCubePositions();

	/*
	 * @params: rend -> a renderer
	 * @info: update cube positions
	 */
	void CubeUpdatePos(Renderer* rend);
	
public:
	/*
	 * @info: default initilization, initializes cube
	 *		can change it later to receive type.
	 */
	PrimMeshModel();
	~PrimMeshModel()
	{
		delete[] Cube_Positions;
	}

	/*
	 * @params: rend -> a renderer
	 * @info: draws a cube
	 */
	void drawCube(Renderer* rend);

	/*
	 * @params:rend -> a renderer
	 * @info: draws based on primType
	 */
	void draw(Renderer* rend) override
	{
		if (primType == CUBE)
			drawCube(rend);
	}

	virtual void drawVertexNormals(Renderer* rend);
};