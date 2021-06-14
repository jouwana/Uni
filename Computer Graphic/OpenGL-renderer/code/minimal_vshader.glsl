#version 150
precision highp float;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoor;
// normal mapping
in vec3 aTangent;
in vec3 aBitangent;

//canonical mapping
uniform int texture_type;

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;
uniform mat4 Projection;

uniform float animateVertix;

//******* lighs and stuff *******
#define MAX_LIGHTS 10
uniform int parallal_light_count;
uniform int point_light_count;
uniform vec3 parallal_directions_vec[MAX_LIGHTS];
uniform vec4 point_positions_vec[MAX_LIGHTS];


//******* things for gourand ********
uniform int is_gourand;
uniform vec3 parallal_colors_vec[MAX_LIGHTS];
uniform vec3 point_colors_vec[MAX_LIGHTS];
uniform vec3 ambient_light_color;

uniform float Shininess;
uniform float k_Ambient;
uniform float k_Diffuse;
uniform float k_Specular;


//**** toon shading?
uniform int toonShading;

// output values that will be interpolated per-fragment
out vec3 fN;
out vec3 fE;
out vec3 parallal_L_vec[MAX_LIGHTS];
out vec3 point_L_vec[MAX_LIGHTS];
out vec4 vertexColor;
out vec2 UV;
out vec3 posOut;
out mat3 TBN;
// ** HELPER FUNCTIONS DECLERATION **

void light_calculation(vec4 Vpos);

void Gourand_calc();


//** Main Fucntion **

void main()
{
	// normal mapping
	vec4 fN4 = normalize(Projection*vec4(normalMatrix*vNormal,0));
	fN = normalize(vec3(fN4.x,fN4.y,fN4.z));

	//vec3 N = fN;

	vec4 B4 = normalize(Projection*vec4(normalMatrix*aBitangent,0));
	vec3 B = vec3(B4.x, B4.y, B4.z);

	vec4 T4 = normalize(Projection*vec4(normalMatrix*aTangent,0));
	vec3 T = vec3(T4.x, T4.y, T4.z);

	vec3 N = normalize(cross(B,T));

    TBN = mat3(T, B, N);

	posOut = vPosition;

	//in projection mode, camera positions is always (0,0,0)
	vec4 eye_position = vec4(0,0,0,1);
	
	//temporary positions for settings up animation
	vec3 Vpos1 = vPosition;


	if(animateVertix > -0.5f)
	{
		Vpos1 += 0.2*animateVertix*vNormal;
	}

	//built in texture
	if (texture_type == 0)
	{
		UV = vec2(vTexCoor.x, -1 * vTexCoor.y);
	}

	//canonical plane
	else if (texture_type == 1)
	{
		UV = vec2(vPosition.x, -vPosition.y);
	}

	//canonical sphere
	else if (texture_type == 2)
	{
		float r = pow(vPosition.x,2) + pow(vPosition.y,2);
		float tan_teta = vPosition.y/vPosition.x;
		float teta = atan(tan_teta);
		UV = vec2(r, teta);
	}


	//final positions (modelMatrix inclues cTranfsorm)
	vec4 Vpos = Projection*modelMatrix*vec4(Vpos1,1);
	
	
	//if drawing silhouette for toon shading, then figure out what parts to draw
	if(toonShading == 2)
		if(dot(Vpos,vec4(fN4.x,fN4.y,fN4.z,1)) >= 0)
			Vpos += (0.01 * vec4(fN4.x,fN4.y,-8*fN4.z,0));


	//calculate light directions
	light_calculation(Vpos);

	
	vec3 eye_direction = vec3(Vpos - eye_position);
	fE = eye_direction;
	

	gl_Position = Vpos; 
	
	
	//*********** EXTRA CALCULATIONS FOR GOURAND *************

	if(is_gourand == 1)
	{
		Gourand_calc();
	}

	
}


//** HELPER FUNCTION DEFINITION **


// set the light directions for the fragment shader (pointligh and parallal lights)
void light_calculation(vec4 Vpos)
{
	for(int i=0; i < parallal_light_count; i++)
	{
		vec3 L = normalize (parallal_directions_vec[i]) ;
		parallal_L_vec[i] = L; 
	}

	for(int i=0; i < point_light_count; i++)
	{
		vec4 L = normalize((Projection*point_positions_vec[i] - Vpos)) ;
		point_L_vec[i] = vec3(L.x, L.y, L.z); 
	}

}



void Gourand_calc()
{
// Normalize the input lighting vectors
		vec3 N = normalize(fN);
		vec3 E = normalize(fE);

		vec4 ambient = vec4(ambient_light_color * k_Ambient, 1);
	
		vec4 diffuse = vec4(0.0,0.0,0.0,1.0);
		vec4 specular = vec4(0,0,0,1.0);

			// parallal light source
		for (int i = 0; i < parallal_light_count ; i++)
		{
			vec3 L = parallal_L_vec[i];
			vec3 H = normalize( L + E );

			vec4 lightColor = vec4(parallal_colors_vec[i],1);
		
			float diff = max(dot(N, L), 0.0);
			diffuse = diffuse + k_Diffuse * diff * lightColor;

			// discard the specular highlight if the light�s behind the vertex
			if( dot(L, N) < 0.0 )
			{
				specular += vec4(0.0, 0.0, 0.0, 0.0);
			}
			else
			{
				float spec = pow(max(dot(N, H), 0.0), Shininess);
				specular = specular + k_Specular * spec * lightColor;
			}
		}

		// point light source
		for (int i=0; i < point_light_count; i++)
		{
			vec3 L = point_L_vec[i];
			vec3 H = normalize( L + E );

			vec4 lightColor = vec4(point_colors_vec[i],1);
		
			float diff = max(dot(N, L), 0.0);
			diffuse = diffuse + k_Diffuse * diff * lightColor;

			// discard the specular highlight if the light�s behind the vertex
			if( dot(L, N) < 0.0 )
			{
				specular += vec4(0.0, 0.0, 0.0, 0.0);
			}
			else
			{
				float spec = pow(max(dot(N, H), 0.0), Shininess);
				specular =  specular + k_Specular * spec * lightColor;
			}
		}
		vertexColor = ambient + diffuse + specular;
	
}