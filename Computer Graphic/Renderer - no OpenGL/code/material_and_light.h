#include "vec.h"
#include "mat.h"
#include <string>
#pragma once
typedef struct materialStruct
{
	bool non_uniform = false;
	// reflectivity coefficient foreach color component
	bool self_lighting = false;
	vec3 emission; // emissive colour, models self-luminous sources
	GLfloat diffuse; // fraction of  diffuse light reflected from surface
	GLfloat specular; //fraction of  specular light reflected from surface
	GLfloat shininess; // alpha, for the specular component
	GLfloat ambient;

	// allow non-uniform material, different reflectivity properties for the back and back faces
	//vec4 back_diffuse;
	//vec4 back_specular;
	//GLfloat back_shininess; // alpha, for the specular component
} materialStruct;


enum LightType
{
	AmbientLight,
	PointSource,
	SpotLight,
	ParallelLight // parallel
	/*specular, diffuse, parralle*/
};

enum LightState
{
	OFF,ON
};


class Light
{
public:
	vec3 direction;
	//new original position that is the initial position without taking supersizng into consideration
	vec4 origPosition;
	vec4 position;

	// intensity of RGB
	/*vec3 light_ambient;
	vec3 light_diffuse;
	vec3 light_specular;*/

	vec3 luminance; // intensity of RGB
	LightType type;
	LightState s;

	Light(vec3& new_direction = vec3(0, 0, 0), vec4& new_position = vec4(0, 0, 500, 1), vec3& new_luminance = vec3(1, 1, 1), LightType new_type = AmbientLight,
		float superX = 1, float superY = 1, LightState state = ON)
	{
		direction = new_direction;
		position = new_position;
		luminance = new_luminance; // intensity of RGB
		type = new_type;
		s = state;
		/*	vec3 color = vec3(1, 1, 1);*/
	}

};
