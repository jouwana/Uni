#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
//#include <math.h> 

#define INDEX(width,x,y,c) (x+y*width)*3+c

//zb 
//my function that calculates area of 2d triangle
/* A utility function to calculate area of triangle formed by (x1, y1),
   (x2, y2) and (x3, y3) */
inline float area(int x1, int y1, int x2, int y2, int x3, int y3)
{
	return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
}

//zb 
//my calculate the plane equation
/*
 * returns a vec4 containing A,B,C,D of the plane equation
 * A*x + B*y + C*z + D = 0
 */
inline vec4 CalcPlaneEq(vec3 p1, vec3 p2, vec3 p3)
{
	//find the plane equation
	float A = (p2.y - p1.y) * (p3.z - p1.z) - (p3.y - p1.y) * (p2.z - p1.z);
	float B = (p2.z - p1.z) * (p3.x - p1.x) - (p3.z - p1.z) * (p2.x - p1.x);
	float C = (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y);
	float D = -(A * p1.x + B * p1.y + C * p1.z);
	return vec4(A, B, C, D);
	//return (-A * p.x - B * p.y - D) / C;
}

Renderer::Renderer() :m_width(512), m_height(512)
{
	InitOpenGLRendering();
	CreateBuffers(512,512);
}

Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
	InitOpenGLRendering();
	CreateBuffers(width,height);
}

Renderer::~Renderer(void)
{
	if (m_outBuffer != nullptr)
		delete[] m_outBuffer;
	if (m_zbuffer != nullptr)
		delete[] m_zbuffer;
	if (m_superOutBuffer != nullptr)
		delete[] m_superOutBuffer;

	//bl 
	if (blur_outBuffer != nullptr)
		delete[] blur_outBuffer;
	
}

void Renderer::SetCameraTransform(const mat4& cTransform)
{
	this->ctransfrom = cTransform;
}

void Renderer::SetProjection(const mat4& projection)
{
	this->c_project = projection;
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
	//new we only need to allocate memory if we actually supersize
	if (superX != 1 && superY != 1)
		m_superOutBuffer = new float[3 * (superX * m_width) * (superY * m_height)];
	else m_superOutBuffer = nullptr;

	//avg 
	blur_outBuffer = new float[3 * m_width * m_height];

	m_zbuffer = new float[m_width * superX * m_height * superY];
	//set initial values to -infinity
	for (int i = 0; i < m_height * m_width; i++)
		m_zbuffer[i] = -std::numeric_limits<float>::infinity();
}

void Renderer::updateBuffer(int width, int height)
{
	if (m_outBuffer != nullptr)
		delete[] m_outBuffer;
	if (blur_outBuffer != nullptr)
		delete[] blur_outBuffer;
	if (m_superOutBuffer != nullptr)
		delete[] m_superOutBuffer;
	if (m_zbuffer != nullptr)
		delete[] m_zbuffer;
	CreateBuffers(width, height);
}

void Renderer::SetDemoBuffer()
{
	//vertical line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,256,i,0)]=1;	m_outBuffer[INDEX(m_width,256,i,1)]=0;	m_outBuffer[INDEX(m_width,256,i,2)]=0;

	}
	//horizontal line
	for(int i=0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width,i,256,0)]=1;	m_outBuffer[INDEX(m_width,i,256,1)]=0;	m_outBuffer[INDEX(m_width,i,256,2)]=1;

	}
}

void Renderer::draw(vec4* vertex_positions, const int size, materialStruct& material, vector<vec3>* normal_per_face)
{
	mat4 finalT = c_project*ctransfrom;

	vec3* new_vertex_position = new vec3 [size*3];

	// clipping algorithm - the vertex position are of specific polygon

	// normalization
	for(int i = 0; i < size*3 ; i++)
		new_vertex_position[i] = transformVec4ToVec3(finalT * vertex_positions[i]);
	
	DrawTriangles(vertex_positions, new_vertex_position, size, normal_per_face, v_normals, material);

	if(size != 0)
		delete[] new_vertex_position;
}

//avg 
void Renderer::compute_normal_per_face(const int size, vector<vec3> normal_per_face)
{
	mat3 normals_T = transpose(normals_c_inv);
	
	for (int i = 0; i < size; i++)
	{
		f_normals.push_back(normals_T * normal_per_face[i]);
	}
}

void Renderer::draw_normal_per_face(vec4* vertex_positions, const int size)
{
	for (int i =0; i<size; i++)
	{
		vec3 v = f_normals[i];

		// vertex of the space
		vec3 v1 = transformVec4ToVec3(c_project*ctransfrom*vertex_positions[3*i]);
		vec3 v2 = transformVec4ToVec3(c_project*ctransfrom*vertex_positions[3*i+1]);
		vec3 v3 = transformVec4ToVec3(c_project*ctransfrom *vertex_positions[3*i+2]);

		// compute the center of the face
		//new need to divide the values by superX, as the vertices are supersized
		int midx = (v1.x + v2.x + v3.x) / 3;
		int midy = (v1.y + v2.y + v3.y) / 3;
		int midz = (v1.z + v2.z + v3.z) / 3;

		// normal vector
		vec3 n = vec3(v.x, v.y, v.z);
		n /= length(n);
		n *= 10;
		
		setColors(vec3(1,1,0));
		drawLine(midx, midy, midx + n.x, midy + n.y);

		// draw the points
		int x = midx;
		int y = midy;
		
		int i1 = INDEX(m_width, x, y, 0) - 1;
		int i2 = INDEX(m_width, x, y, 1) - 1;
		int i3 = INDEX(m_width, x, y, 2) - 1;

		if ((i1 < 0 || i1 >= m_width * m_height * 3) || (i3 >= m_width * m_height * 3) || x > m_width || x < 0)
			continue;

		setColors(vec3(1, 1, 0));
		m_outBuffer[i1] = color.x;
		m_outBuffer[i2] = color.y;
		m_outBuffer[i3] = color.z;

	}
}

//avg 
void Renderer::compute_normal_per_vertex(const int size, vec3* normal_per_vertex, mat3& T)
{
	mat3 normal_transform = transpose(T * normals_c_inv);
	v_normals = new vec3 [size*3];
	for (int i = 0; i < size * 3; i++)
	{
		vec3 npv = normal_per_vertex[i];
		vec3 n = normal_transform * (npv);
		v_normals[i] = normalize(n);
	}
}

void Renderer::draw_normal_per_vertex(vec4* vertex_positions, const int size)
{
	for (int i=0; i<size*3; i++)
	{
		vec3 vp = vec4ToVec3(vertex_positions[i]);
		vec3 p = transformVec4ToVec3(c_project*ctransfrom * vertex_positions[i]);
		
		vec3 n = v_normals[i];
		n *= 10;

		setColors(vec3(1,0,1));
		//new need to divide the values by superX, as the vertices are supersized
		drawLine(p.x, p.y, p.x + n.x, p.y + n.y);
	}
}


void Renderer::draw_bounding_box(vec4* vertex_positions, const int size, vector<vec4> bounds)
{
	mat4 finalT = this->c_project*ctransfrom;
	
	//avg  draw bounding box (should be changes to allow to user whether to draw it)
	for (int i = 0; i < bounds.size(); i = i + 4)
	{
		// transform to world frame (multiply by finalT)
		vec3 v1 = transformVec4ToVec3(finalT * bounds.at(i));
		vec3 v2 = transformVec4ToVec3(finalT * bounds.at(i + 1));
		vec3 v3 = transformVec4ToVec3(finalT * bounds.at(i + 2));
		vec3 v4 = transformVec4ToVec3(finalT * bounds.at(i + 3));
		drawLine(v1.x, v1.y, v2.x, v2.y);
		drawLine(v2.x, v2.y, v3.x, v3.y);
		drawLine(v3.x, v3.y, v4.x, v4.y);
		drawLine(v4.x, v4.y, v1.x, v1.y);

		for (int j = 0; j < 4; j++) // draw each vertex
		{
			vec4 temp(bounds.at(i + j));
			vec3 new_v = transformVec4ToVec3((finalT * temp));
			int x2 = new_v.x;
			int y2 = new_v.y;

			int i1 = INDEX(m_width, x2, y2, 0) - 1;
			int i2 = INDEX(m_width, x2, y2, 1) - 1;
			int i3 = INDEX(m_width, x2, y2, 2) - 1;

			if ((i1 < 0 || i1 >= m_width * m_height * 3) || (i3 >= m_width * m_height * 3) || x2 > m_width || x2 < 0)
				continue;

			m_outBuffer[i1] = color.x;
			m_outBuffer[i2] = color.y;
			m_outBuffer[i3] = color.z;
		}

	}
	
}

void Renderer::draw_camera(vec4 pos)
{
	vec3 n_pos = transformVec4ToVec3(pos);
	float i1 = INDEX(m_width, pos.x, pos.y, 0) - 1;
	int i2 = INDEX(m_width, pos.x, pos.y, 1) - 1;
	int i3 = INDEX(m_width, pos.x, pos.y, 2) - 1;

	/*ASSERT((int)i1 % 3 == 2);*/
	
	if (!((i1 < 0 || i1 >= m_width * m_height * 3) || (i3 >= m_width * m_height * 3) || pos.x > m_width || pos.x < 0))
	{
		setColors(vec3(0, 0, 1));//green
		m_outBuffer[(int)i1] = color.x;
		m_outBuffer[i2] = color.y;
		m_outBuffer[i3] = color.z;
	}
}

//my implemented clear depth buffer
void Renderer::ClearDepthBuffer()
{
	//set initial values to -infinity
	for (int i = 0; i < m_width * superX * m_height * superY; i++)
		m_zbuffer[i] = -std::numeric_limits<float>::infinity();
}


//my function implementations
void Renderer::ClearColorBuffer()
{

	//make all pixels black;
	if (m_outBuffer != nullptr)
		for (int i = 0; i < 3 * m_width * m_height; i++)
			m_outBuffer[i] = 0;
	//we swapbuffers or draw after this function gets called.
	//new we need to clear superbuffer too
	//clear supersampler
	if (m_superOutBuffer != nullptr)
		for (int i = 0; i < 3 * (m_width * superX) * (m_height * superY); i++)
			m_superOutBuffer[i] = 0;


	if (blur_outBuffer != nullptr)
		for (int i = 0; i < 3 * m_width * m_height; i++)
			blur_outBuffer[i] = 0;
}


//zb 
void Renderer::DrawTriangles(const vec4* vertex_positions, const vec3* vertices, int size, vector<vec3>* f_normals, vec3* v_normals, materialStruct& material)
{
	for (int i = 0; i < size * 3;)
	{
		//save values of current triangle heads, before and after projection
		vec3 A = vertices[i];
		vec4 p1_4 = ctransfrom * vertex_positions[i];

		i++;
		vec3 B = vertices[i];
		vec4 p2_4 = ctransfrom * vertex_positions[i];

		i++;
		vec3 C = vertices[i];
		vec4 p3_4 = ctransfrom * vertex_positions[i];

		i++;


		//new updates coordinates of triangle based on supersampling
		A.x = A.x * superX;
		p1_4.x = p1_4.x * superX;
		A.y = A.y * superY;
		p1_4.y = p1_4.y * superY;
		B.x = B.x * superX;
		p2_4.x = p2_4.x * superX;
		B.y = B.y * superY;
		p2_4.y = p2_4.y * superY;
		C.x = C.x * superX;
		p3_4.x = p3_4.x * superX;
		C.y = C.y * superY;
		p3_4.y = p3_4.y * superY;

		//new set values of vec3s
		vec3 p3 = transformVec4ToVec3(p3_4);
		vec3 p2 = transformVec4ToVec3(p2_4);
		vec3 p1 = transformVec4ToVec3(p1_4);


		//calculate plane equation based on original heads
		vec4 planeEq = CalcPlaneEq(p1, p2, p3);

		//go through all point inside the bounding box of the triangle
		//and check if they are inside it or not
		float minX = min((int)A.x, min((int)B.x, (int)C.x));
		float maxX = max((int)A.x, max((int)B.x, (int)C.x));
		float minY = min((int)A.y, min((int)B.y, (int)C.y));
		float maxY = max((int)A.y, max((int)B.y, (int)C.y));

		vec2 maxValues = vec2(maxX, maxY);

		//set initial val of point
		vec2 p = vec2(minX, minY);

		/* Calculate area of triangle ABC */
		float ABC = area(A.x, A.y, B.x, B.y, C.x, C.y);

		vec3 TempColour;
		if (shadind_alg == FLAT)
		{
			TempColour = flat_shading_algorithm(p1_4, p2_4, p3_4, material, (*f_normals)[(i + 1) / 3 - 1], maxValues);

		}
		// color for polygon, calculate once since it's flat shading
		while (p.y <= maxY && p.y >= minY)
		{

			float inTri = 0;

			//if the point is one of the three heads
			if ((int)p.x == (int)A.x && (int)p.y == (int)A.y)
				inTri = 1;
			else if ((int)p.x == (int)B.x && (int)p.y == (int)B.y)
				inTri = 2;
			else if ((int)p.x == (int)C.x && (int)p.y == (int)C.y)
				inTri = 3;
			//check if the point is inside the triangle
			else {
				/* Calculate area of triangle PBC */
				float A1 = area(p.x, p.y, B.x, B.y, C.x, C.y);

				/* Calculate area of triangle PAC */
				float A2 = area(A.x, A.y, p.x, p.y, C.x, C.y);

				/* Calculate area of triangle PAB */
				float A3 = area(A.x, A.y, B.x, B.y, p.x, p.y);

				/* Check if sum of A1, A2 and A3 is same as A */
				if ((int)ABC == (int)(A1 + A2 + A3))
					inTri = 4;
			}

			//if point is inside triangle:
			if (inTri > 0)
			{

				float z;
				//if its a head
				if (inTri == 1)
					z = p1.z;
				else if (inTri == 2)
					z = p2.z;
				else if (inTri == 3)
					z = p3.z;
				else //calculate it based on plane equation
					z = (-(planeEq.x * p.x) - (planeEq.y * p.y) - planeEq.w) / planeEq.z;
				//new need to divide by the superY and superX so we can go back to the range of the original
				//new width and height
				int zBuffLoc = (int)(m_width * superX * (p.y) + p.x);

				if (!(zBuffLoc >= m_width * superX * m_height * superY || zBuffLoc < 0))
				{
					float depth = m_zbuffer[zBuffLoc];

					//if it should be drawn
					//my can be changed with "putColor" or simply apply light here
					if (z >= depth)
					{
						if (shadind_alg == GOURAND)
						{
							TempColour = gourand_shading_algorithm(p1_4, p2_4, p3_4, material, v_normals[i - 3], v_normals[i - 2], v_normals[i - 1], vec3(p.x, p.y, z), maxValues);
						}
						else if (shadind_alg == PHONG)
						{
							TempColour = phong_shading_algorithm(p1_4, p2_4, p3_4, material, v_normals[i - 3], v_normals[i - 2], v_normals[i - 1], vec3(p.x, p.y, z), maxValues);
						}
						m_zbuffer[zBuffLoc] = z;
						int x2 = p.x, y2 = p.y;

						//new the new temporary buffer's width is "m_width * superX" so we need to take that
						//new into consideration when we find the index
						int i1 = INDEX(m_width * superX, x2, y2, 0);
						int i2 = INDEX(m_width * superX, x2, y2, 1);
						int i3 = INDEX(m_width * superX, x2, y2, 2);

						if (!((i1 < 0 || i1 >= (m_width * superX) * (m_height * superY) * 3) || (i3 >= (superX * m_width) * (superY * m_height) * 3) || x2 > (superX * m_width) || x2 < 0 || y2 >= (superY*m_height) || y2<0))
						{
							//TODO: remember to add in a trigger for fog, possibly the letter 'f'
							color = TempColour;
							if (enable_fog)
								color = fog.mix(TempColour, z);
							//new if we are in the original size without supersampling, then just print directly on the screen
							if (superX == 1 && superY == 1)
							{
								m_outBuffer[i1] = color.x;
								m_outBuffer[i2] = color.y;
								m_outBuffer[i3] = color.z;
							}
							else
							{
								//new if we are supersampling, print into the new buffer
								m_superOutBuffer[i1] = color.x;
								m_superOutBuffer[i2] = color.y;
								m_superOutBuffer[i3] = color.z;
							}
						}
					}
				}
			}
			//if we exited the triangle bounding box in X then increment Y
			if (p.x > maxX) { p.x = minX; p.y++; }
			else p.x++;

		}

	}
	//new if we supersampled, we need to downsize and print
	if (superX != 1 || superY != 1)
		downSizeAndDraw();

	/*repeat_blur();*/
	switch (blurring_mode)
	{
	case(None):
		break;
	case (FULL_SCREEN):
		full_screen_blur();
		break;
	case (BLOOM):
		bloom();
		break;
	}
	
}


//avg 
void Renderer::plotLineLow(int x0, int y0, int x1, int y1)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int yi = 1;
	if (dy < 0)
	{
		yi = -1;
		dy = -dy;
	}
	int D = (2 * dy) - dx;
	int y = y0;

	for (int x = x0; x <= x1; x++)
	{

		int i1 = INDEX(m_width, x, y, 0) - 1;
		int i2 = INDEX(m_width, x, y, 1) - 1;
		int i3 = INDEX(m_width, x, y, 2) - 1;

		/*ASSERT(i1 % 3 == 2);*/
		
		if ((i1 < 0 || i1 >= m_width * m_height * 3) || (i3 >= m_width * m_height * 3) || x > m_width || x < 0)
			continue;
		
		m_outBuffer[i1] = color.x;
		m_outBuffer[i2] = color.y;
		m_outBuffer[i3] = color.z;

		if (D > 0)
		{
			y = y + yi;
			D = D + (2 * (dy - dx));
		}
		else
			D = D + 2 * dy;
	}
}


//avg 
void Renderer::plotLineHigh(int x0, int y0, int x1, int y1)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	int xi = 1;
	if (dx < 0)
	{
		xi = -1;
		dx = -dx;
	}
	int D = (2 * dx) - dy;
	int x = x0;

	for (int y = y0; y <= y1; y++)
	{

		int i1 = INDEX(m_width, x, y, 0) - 1;
		int i2 = INDEX(m_width, x, y, 1) - 1;
		int i3 = INDEX(m_width, x, y, 2) - 1;

		if ((i1 < 0 || i1 >= m_width * m_height * 3) || (i3 >= m_width * m_height * 3) || x > m_width || x < 0)
			continue;

		/*ASSERT(i1 % 3 == 2);*/
		
		m_outBuffer[i1] = color.x;
		m_outBuffer[i2] = color.y;
		m_outBuffer[i3] = color.z;
		if (D > 0)
		{
			x = x + xi;
			D = D + (2 * (dx - dy));
		}
		else
			D = D + 2 * dx;
	}
}


//avg 
void Renderer::drawLine(int x0, int y0, int x1, int y1)
{
	if (abs(y1 - y0) < abs(x1 - x0))
	{
		if (x0 > x1)
			plotLineLow(x1, y1, x0, y0);
		else
			plotLineLow(x0, y0, x1, y1);
	}
	else
	{
		if (y0 > y1)
			plotLineHigh(x1, y1, x0, y0);
		else
			plotLineHigh(x0, y0, x1, y1);
	}
}

//new function
//my downsize and draw on screen
void Renderer::downSizeAndDraw()
{
	// we downsize by X (width) first then we downsize by Y
	// make a new array that will have the new image with is only downsized on X
// meaning it has the original width but still has the super height
	float* downSizedX = new float[3 * m_width * (m_height * superY)];

	//if we did supersample on X, then we need to downsize
	if (superX > 1)
	{
		//go over every row and sum up the pixels into 1
		//ever superX amount of pixels in a row should be downsized into only one pixel
		for (int i = 0; i < m_height * superY; i++)
		{
			for (int j = 0; j < m_width; j++)
			{
				vec3 pixelColor = vec3(0, 0, 0);

				for (int k = 0; k < superX; k++)
				{
					//find the current X we are on
					int currj = j * superX;
					//find the current index we are on, we are running on a supersized buffer, so we send the super width
					int currIndex = INDEX((m_width * superX), currj, i, 0);
					pixelColor += vec3(m_superOutBuffer[currIndex], m_superOutBuffer[currIndex + 1], m_superOutBuffer[currIndex + 2]);
				}
				//put in the average color
				downSizedX[INDEX(m_width, j, i, 0)] = pixelColor.x / superX;
				downSizedX[INDEX(m_width, j, i, 1)] = pixelColor.y / superX;
				downSizedX[INDEX(m_width, j, i, 2)] = pixelColor.z / superX;
			}
		}
	}
	else //if we didnt supersize X, its the same as the original super buffer
		for (int i = 0; i < m_width * m_height * superY * 3; i++)
			downSizedX[i] = m_superOutBuffer[i];

	//downsize on Y
	if (superY > 1)
	{
		//go over every column, and downsize every "superY" amount of pixels into one pixel
		for (int i = 0; i < m_width; i++)
		{
			for (int j = 0; j < m_height; j++)
			{
				vec3 pixelColor = vec3(0, 0, 0);
				for (int k = 0; k < superY; k++)
				{
					//the current Y we are in
					int currentY = j * superY + k;
					//the current index we are in, downsizedX has the width of the priginal, so we dont multiple by superX
					int currIndex = INDEX(m_width, i, currentY, 0);
					pixelColor += vec3(downSizedX[currIndex], downSizedX[currIndex + 1], downSizedX[currIndex + 2]);
				}
				//put in the average color
				m_outBuffer[INDEX(m_width, i, j, 0)] = pixelColor.x / superY;
				m_outBuffer[INDEX(m_width, i, j, 1)] = pixelColor.y / superY;
				m_outBuffer[INDEX(m_width, i, j, 2)] = pixelColor.z / superY;
			}
		}
	}
	else // if we didnt size up Y, its the same as downsizedX
		for (int i = 0; i < m_width * m_height * 3; i++)
			m_outBuffer[i] = downSizedX[i];

	//we no longer need the buffer, so we delete it
	delete[] downSizedX;


}


/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.
void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[]={
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[]={
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc)+sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	GLint  vPosition = glGetAttribLocation( program, "vPosition" );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0 );

	GLint  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc) );
	glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );
	a = glGetError();
}
void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}
void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}


// sh 
vec3 Renderer::phong_shading_algorithm(vec4& vtx1, vec4& vtx2, vec4& vtx3, materialStruct material, vec3& normal_1, vec3& normal_2, vec3& normal_3, vec3 pixel, vec2& maxValues)
{
	if (material.self_lighting) // self lighting model
	{
		vec3 emissiveColor = vec3(0, 0, 0);
		if (material.non_uniform)
		{
			emissiveColor = vec3((pixel.y / maxValues.y) - ((int)pixel.y % 30) / 100.0f, (pixel.x / maxValues.x) - ((int)pixel.x % 30) / 100.0f,
				(pixel.x / maxValues.x) - ((int)pixel.x % 25) / 100.0f);
		}
		return (emissiveColor + material.emission);
	}



	// remove effect of supersampling
	vtx1 = vec4(vtx1.x / superX, vtx1.y / superY, vtx1.z, vtx1.w);
	vtx2 = vec4(vtx2.x / superX, vtx2.y / superY, vtx2.z, vtx2.w);
	vtx3 = vec4(vtx3.x / superX, vtx3.y / superY, vtx3.z, vtx3.w);
	pixel = vec3(pixel.x / superX, pixel.y / superY, pixel.z);

	// linearly interpolate lighting intensities at the vertices over interior pixels  of the polygonn, in the image plane
	float alpha_1, alpha_2, alpha_3;
	vec3 p = pixel; // the pixel which we want to calculate its illumination

	// calcuate the alpha using barycentric coordinates 
	float area_1, area_2, area_3;

	// Interpolating intensities along the polygon edges.
	// first, get the 3D-coordinates for the pixel using interpolation?
	area_1 = 0.5 * length(cross((transformVec4ToVec3(vtx1) - p), (transformVec4ToVec3(vtx2) - p)));
	area_2 = 0.5 * length(cross((transformVec4ToVec3(vtx2) - p), (transformVec4ToVec3(vtx3) - p)));
	area_3 = 0.5 * length(cross((transformVec4ToVec3(vtx3) - p), (transformVec4ToVec3(vtx1) - p)));

	float area_sum = area_1 + area_2 + area_3;
	alpha_1 = area_1 / area_sum;
	alpha_2 = area_2 / area_sum;
	alpha_3 = area_3 / area_sum;

	vec3 normal = alpha_1 * normal_1 + alpha_2 * normal_2 + alpha_3 * normal_3;
	
	return compute_illumination_intensity(material, vec4(p, 1), normal, maxValues);
}

// calculate the final color of a polygon using the vertex colors, and interpolation intensity over polygon interior, consider all light's sources
vec3 Renderer::gourand_shading_algorithm(vec4& vtx1, vec4& vtx2, vec4& vtx3, materialStruct material, vec3& normal_1, vec3& normal_2, vec3& normal_3, vec3 pixel, vec2& maxValues)
{
	if (material.self_lighting)
	{
		vec3 emissiveColor = vec3(0, 0, 0);
		if (material.non_uniform)
		{
			emissiveColor = vec3((pixel.y / maxValues.y) - ((int)pixel.y % 30) / 100.0f, (pixel.x / maxValues.x) - ((int)pixel.x % 30) / 100.0f,
				(pixel.x / maxValues.x) - ((int)pixel.x % 25) / 100.0f);
		}// self lighting model
		return (emissiveColor + material.emission);
	}


	vec3 final_I;
	// linearly interpolate lighting intensities at the vertices over interior pixels  of the polygonn, in the image plane
	vec3 illumination_v1 = compute_illumination_intensity(material, vtx1, normal_1, maxValues);
	vec3 illumination_v2 = compute_illumination_intensity(material, vtx2, normal_2, maxValues);
	vec3 illumination_v3 = compute_illumination_intensity(material, vtx3, normal_3, maxValues);

	float alpha_1, alpha_2, alpha_3;
	vec3 p = pixel; // the pixel which we want to calculate its illumination

	// calcuate the alpha using barycentric coordinates 
	float area_1, area_2, area_3;

	// Interpolating intensities along the polygon edges.
	// first, get the 3D-coordinates for the pixel using interpolation?
	area_1 = 0.5 * length(cross((transformVec4ToVec3(vtx1) - p), (transformVec4ToVec3(vtx2) - p)));
	area_2 = 0.5 * length(cross((transformVec4ToVec3(vtx2) - p), (transformVec4ToVec3(vtx3) - p)));
	area_3 = 0.5 * length(cross((transformVec4ToVec3(vtx3) - p), (transformVec4ToVec3(vtx1) - p)));

	float area_sum = area_1 + area_2 + area_3;
	alpha_1 = area_1 / area_sum;
	alpha_2 = area_2 / area_sum;
	alpha_3 = area_3 / area_sum;

	float illumination_at_p_R = alpha_1 * illumination_v1.x + alpha_2 * illumination_v2.x + alpha_3 * illumination_v3.x;
	float illumination_at_p_G = alpha_1 * illumination_v1.y + alpha_2 * illumination_v2.y + alpha_3 * illumination_v3.y;
	float illumination_at_p_B = alpha_1 * illumination_v1.z + alpha_2 * illumination_v2.z + alpha_3 * illumination_v3.z;

	final_I = vec3(illumination_at_p_R, illumination_at_p_G, illumination_at_p_B);
	return final_I;
}

//sh 
vec3 Renderer::flat_shading_algorithm(vec4& vtx1, vec4& vtx2, vec4& vtx3, materialStruct material, vec3& normal, vec2& maxValues)
{
	float new_x = (vtx1.x + vtx2.x + vtx3.x) / superX / 3;
	float new_y = (vtx1.y + vtx2.y + vtx3.y) / superY / 3;
	float new_z = (vtx1.z + vtx2.z + vtx3.z) / 3;
	float new_w = (vtx1.w + vtx2.w + vtx3.w) / 3;

	vec4 face_center(new_x, new_y, new_z, new_w);

	if (material.self_lighting) // self lighting model
	{
		vec3 emissiveColor = vec3(0, 0, 0);
		if (material.non_uniform)
		{
			emissiveColor = vec3((face_center.y / maxValues.y) - ((int)face_center.y % 30) / 100.0f, (face_center.x / maxValues.x) - ((int)face_center.x % 30) / 100.0f,
				(face_center.x / maxValues.x) - ((int)face_center.x % 25) / 100.0f);
		}// self lighting model
		return emissiveColor + material.emission;
	}

	// ambient light contribution. light[0] is the ambient light
	vec3 illumination = compute_illumination_intensity(material, face_center, normal, maxValues);

	return illumination;
}

//sh 
/// <summary>
/// Using Phong reflectivity model: 
/// Computs the illumination intensity at a given point p that have a given material
/// </summary>
/// <param name="material"> material of vertex p </param>
/// <param name="p"> point coordinates </param>
/// /// <param name="normal"> normal to the point (consider face or vertex) </param>
/// <returns> the illumination intensity at p </returns>
vec3 Renderer::compute_illumination_intensity(materialStruct& material, vec4& p, vec3& normal, vec2& maxValues)
{
	vec3 final_light = vec3(0, 0, 0); 
	if (lights.size() != 0)
	{
		if (material.non_uniform)
		{
			final_light = vec3((p.y / maxValues.y) - ((int)p.y % 30) / 100.0f, (p.x / maxValues.x) - ((int)p.x % 30) / 100.0f,
				(p.x / maxValues.x) - ((int)p.x % 25) / 100.0f);
			final_light /= 5.0f;

		}
		if (lights[0]->s == ON)
			final_light += material.ambient * vec3(lights[0]->luminance.x, lights[0]->luminance.y, lights[0]->luminance.z);
	}

	vec4 l; // direction to light source
	vec4 r; // direction of reflectd ray
	vec4 v; // direction to COP
	vec4 half;

	v = active_camera_position - p;
	v = normalize(v);

	for (int i = 1; i < lights.size(); i++)
	{
		Light light = *(lights[i]);
		if (light.s == OFF) // the light is off
			continue;
		
		if (light.type == PointSource) // the light direction is the vector from the pointSource to the vertex
		{
			l = normalize(ctransfrom*(light.position) - p); // light_direction
			half = normalize(l + normalize(- p));
		}
		else if (light.type == ParallelLight) // the light direction is the same for all of the scene
		{
			l = normalize(ctransfrom*(vec4(light.direction, 0)));
			half = normalize(l + normalize(-p));
		}

		//r = normalize(2 * (max(dot(l, normal), 0)) * normal - l);

		GLfloat diffuse, specular;
		// calculate the diffuse and specular reflection component foreach color (RGB)
		float d = dot(l, normal); // maybe dot(normal, postitino/dirctino)??
		if (d > 0.0)
			diffuse = material.diffuse * d ;
		else
			diffuse = 0.0;


		float s = dot(vec4(normal, 1), half);
		if (s > 0.0)
			specular = material.specular * pow(s, material.shininess);
		else
			specular = 0;

		final_light += (diffuse + specular) * light.luminance;
	}

	/*if (final_light.x > 1)
		final_light.x = 1;
	if (final_light.y > 1)
		final_light.y = 1;
	if (final_light.z > 1)
		final_light.z = 1;*/
	return final_light;
}

//bl 
// repeat the blurring 10 times (5 horizontal and 5 vertical)
void Renderer::full_screen_blur()
{
	// create the blooming buffer
	for (int x = 0; x < m_width; x++)
		for (int y = 0; y < m_height; y++)
		{
			int i1 = INDEX(m_width, x, y, 0);
			int i2 = INDEX(m_width, x, y, 1);
			int i3 = INDEX(m_width, x, y, 2);

			vec3 current_color(m_outBuffer[i1], m_outBuffer[i2], m_outBuffer[i3]);

			blur_outBuffer[i1] = min(current_color.x, 1);
			blur_outBuffer[i2] = min(current_color.y, 1);
			blur_outBuffer[i3] = min(current_color.z, 1);
		}

	// execute vertical and horizontal blurring
	for (int i = 0; i < 1; i++)
		blurring();

	for (int i = 0; i < m_width * m_height * 3; i++)
	{
		m_outBuffer[i] = blur_outBuffer[i];
	}
}

void Renderer::bloom()
{
	// create the blooming buffer
	for(int x=0; x<m_width;x++)
		for (int y=0;y<m_height;y++)
		{
			int i1 = INDEX(m_width, x, y, 0);
			int i2 = INDEX(m_width, x, y, 1);
			int i3 = INDEX(m_width, x, y, 2);

			vec3 current_color(m_outBuffer[i1], m_outBuffer[i2], m_outBuffer[i3]);
			float brightness = dot(current_color, vec3(0.2126, 0.7152, 0.0722));
			
			vec3 bright_color;
			if (brightness >= 1.0)
				bright_color = current_color;
			else
				bright_color = vec3(0.0, 0.0, 0.0);

			blur_outBuffer[i1] = min(bright_color.x, 1);
			blur_outBuffer[i2] = min(bright_color.y, 1);
			blur_outBuffer[i3] = min(bright_color.z, 1);
		}

	// execute vertical and horizontal blurring
	for (int i = 0; i < 1; i++)
		blurring();

	//// blending both
	///*float gamma = 2.2;*/
	for (int i = 0; i < m_width* m_height*3; i++)
	{
		m_outBuffer[i] += blur_outBuffer[i];
	}
}

void Renderer::blurring()
{
	// change the m_outBuffer. Extract two images and then

	float weight[10] = { 0.5, 0.423110, 0.27546789, 0.19494, 0.130224, 0.098755, 0.07988851, 0.053151, 0.02122, 0.0098 };
	/*float weight[5] = { 2 * 0.227027, 2 * 0.1945946, 2 * 0.1216216, 2 * 0.054054, 2 * 0.016216 };*/
	/*float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};*/
	float* blur_temp = new float[3 * m_width * m_height]; // 3*width*height
	// go throughgt all of the pixels and apply the gaussian filter in two steps: horizontal and vertical
	// vertical 
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			int i1 = INDEX(m_width, x, y, 0);
			int i2 = INDEX(m_width, x, y, 1);
			int i3 = INDEX(m_width, x, y, 2);

			vec3 result = vec3(blur_outBuffer[i1], blur_outBuffer[i2], blur_outBuffer[i3]) * weight[0];
			for (int i = 1; i < 10; i++)
			{
				if (!((i1 < 0 || i1 >= m_width * m_height * 3) || (i3 >= m_width * m_height * 3) || x >= m_width || x < 0 || y<0 || y>= m_height))
				{
					int i1_ = INDEX(m_width, x, y + i, 0);
					int i2_ = INDEX(m_width, x, y + i, 1);
					int i3_ = INDEX(m_width, x, y + i, 2);
				
					if (!((i1_ < 0 || i1_ >= m_width * m_height * 3) || (i3_ >= m_width * m_height * 3) || x >= m_width || x < 0 || (y+i) < 0 || (y+i) >= m_height))
					{
						vec3 tmp = vec3(blur_outBuffer[i1_], blur_outBuffer[i2_], blur_outBuffer[i3_]) * weight[i];
						result += tmp;

					}
					i1_ = INDEX(m_width, x, y - i, 0);
					i2_ = INDEX(m_width, x, y - i, 1);
					i3_ = INDEX(m_width, x, y - i, 2);
					// inside the image
					if (!((i1_ < 0 || i1_ >= m_width * m_height * 3) || (i3_ >= m_width * m_height * 3) || x>=m_width || x < 0 || (y-i) < 0 || (y-i) >= m_height))
					{
						result += vec3(blur_outBuffer[i1_], blur_outBuffer[i2_], blur_outBuffer[i3_]) * weight[i];
					}
					blur_temp[i1] = min(result.x,1);
					blur_temp[i2] = min(result.y,1);
					blur_temp[i3] = min(result.z,1);
				}
			}
			
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////
	// horizontal, excatly the same
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			int i1 = INDEX(m_width, x, y, 0);
			int i2 = INDEX(m_width, x, y, 1);
			int i3 = INDEX(m_width, x, y, 2);

			vec3 result = vec3(blur_temp[i1], blur_temp[i2], blur_temp[i3])*weight[0];
			for (int i = 1; i < 10; i++)
			{
				if (!((i1 < 0 || i1 >= m_width * m_height * 3) || (i3 >= m_width * m_height * 3) || x>=m_width || x < 0 || y < 0 || y >= m_height))
				{
					int i1_ = INDEX(m_width, x + i, y, 0);
					int i2_ = INDEX(m_width, x + i, y, 1);
					int i3_ = INDEX(m_width, x + i, y, 2);
					// inside the image
				
					if (!((i1_ < 0 || i1_ >= m_width * m_height * 3) || (i3_ >= m_width * m_height * 3) || (x + i) >= m_width || (x + i) < 0 || y < 0 || y >= m_height))
					{
						result += vec3(blur_temp[i1_], blur_temp[i2_], blur_temp[i3_]) * weight[i];
					}
					i1_ = INDEX(m_width, x - i, y, 0);
					i2_ = INDEX(m_width, x - i, y, 1);
					i3_ = INDEX(m_width, x - i, y, 2);
					// inside the image
					if (!((i1_ < 0 || i1_ >= m_width * m_height * 3) || (i3_ >= m_width * m_height * 3) || (x - i) >= m_width || (x - i) < 0 || y < 0 || y >= m_height))
					{
						result += vec3(blur_temp[i1_], blur_temp[i2_], blur_temp[i3_]) * weight[i];
					}
					blur_outBuffer[i1] = min(result.x,1);
					blur_outBuffer[i2] = min(result.y,1);
					blur_outBuffer[i3] = min(result.z,1);

				}
				
			}
		}
	}

	delete[] blur_temp;
}

