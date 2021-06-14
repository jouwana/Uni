#version 150


//******* lights and stuff *******
#define MAX_LIGHTS 10
uniform int parallal_light_count;
uniform int point_light_count;
uniform vec3 parallal_colors_vec[MAX_LIGHTS];
uniform vec3 point_colors_vec[MAX_LIGHTS];
uniform vec3 ambient_light_color;

uniform float Shininess;
uniform float k_Ambient;
uniform float k_Diffuse;
uniform float k_Specular;

//**** animation color ***
uniform vec4 animateColor;

//***is gourand
uniform int is_gourand;

//**** toon shading?
uniform int toonShading;

//**** turbulence texture;
uniform int turbText;

///*** texture
uniform sampler2D ourTexture;
uniform int has_texture;
uniform sampler2D NormalTextureSampler;  


//****per-fragment interpolated values coming infrom the vertex shader **
in vec3 fN;
in vec3 fE;
in vec3 parallal_L_vec[MAX_LIGHTS];
in vec3 point_L_vec[MAX_LIGHTS];
in vec4 vertexColor;
in vec2 UV;
in vec3 posOut;
in mat3 TBN;
uniform int use_normalMapping;

out vec4 fColor;

//** HELPER FUNCTION DECLERATION 

vec4 HSVtoRGB();
vec4 ColorOfLights();


// 2D Random
float random (in vec2 st);

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st);

// Expects -1<x<1
vec3 marble_color (float x);

float turbulence (vec3 P, int numFreq);

vec3 marble_function(vec3 pos);

void main() 
{ 

	vec4 ourColor;


	if(is_gourand==1)
			ourColor = vertexColor;
	

	//****** if not gourand ***********
	else
		ourColor += ColorOfLights();
	
	//set alpha value
	ourColor.a = 1.0;

	

	//** animations values
	if(animateColor.x >= 1.9f)
	{
		//color animation type 2
		ourColor += HSVtoRGB();
	}
	
	else if(animateColor.x > -0.05f)
		ourColor += animateColor;

	//** if we are toon shading, they we need to change values to only 4 hues.
	if(toonShading == 1)
	{
		//clamp colors
		ourColor.x = (ourColor.x>1.0f)?1.0f:ourColor.x;
		ourColor.y = (ourColor.y>1.0f)?1.0f:ourColor.y;
		ourColor.z = (ourColor.z>1.0f)?1.0f:ourColor.z;

		//limit number of hues
		ourColor = 4*ourColor;
		ourColor = vec4(round(ourColor.x),round(ourColor.y),round(ourColor.z),4);
		ourColor = ourColor/4;
	}


	//if drawing silhouette for toon shading, color it black
	if(toonShading == 2)
		fColor = vec4(0,0,0,0.8);
	
	else
	{//if using turbulence texture
		
		if(turbText == 1)
		{
			ourColor *= vec4(marble_function(posOut),0);
		}

		if (has_texture == 1)
			fColor = texture(ourTexture, UV) * ourColor;  
		
		else
			fColor = ourColor;
	}

} 


//** HELPER FUNCTION DEFINITION

vec4 HSVtoRGB()
{
	//used HSV variables
	float H = 0+(animateColor.x-2)*180;
	float S = 35;
	float V = 35;

	//turn RGB to HSV
	float s = S/100;
	float v = V/100;
	float C = s*v;
	float X = C*(1-abs((H/60.0 - (round(H/60 / 2)*2))-1));
	float m = v-C;
	float r,g,b;
	if(H >= 0 && H < 60){
		r = C,g = X,b = 0;
	}
	else if(H >= 60 && H < 120){
		r = X,g = C,b = 0;
	}
	else if(H >= 120 && H < 180){
		r = 0,g = C,b = X;
	}
	else if(H >= 180 && H < 240){
		r = 0,g = X,b = C;
	}
	else if(H >= 240 && H < 300){
		r = X,g = 0,b = C;
	}
	else{
		r = C,g = 0,b = X;
	}
	float R = (r+m);
	float G = (g+m);
	float B = (b+m);

	//add to ourColor
	return vec4(R,G,B,1);
}

vec4 ColorOfLights()
{

	// Normalize the input lighting vectors
	vec3 N;
	vec3 normal;
	if (use_normalMapping == 1)
	{
		normal = texture(NormalTextureSampler, UV).rgb;
		normal = normal * 2.0 - 1.0;   
		normal = normalize(transpose(TBN) * normal); 
		N = normal;	
	}
	else if (use_normalMapping == 0)
	{
		N = normalize(fN);
	}
	
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

		// discard the specular highlight if the light’s behind the vertex
		if( dot(L, N) <= 0.0 )
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

		// discard the specular highlight if the light’s behind the vertex
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
	//if (use_normalMapping == 1)
	//{
	//		return (ambient + diffuse + specular) + 2*vec4(normal,1);
	//}
	//else 
		return (ambient + diffuse + specular) ;

}




// Expects -1<x<1
  vec3 marble_color (float x)
  {
  //very marble like
	vec3 col;
	x = 0.5*(x+1.);          // transform -1<x<1 to 0<x<1
	x=sqrt(x);
	x=sqrt(x);
	x=sqrt(x);
	//x=sqrt(x);
    col = vec3(.2 + .75*x);  // scale x from 0<x<1 to 0.2<x<0.95
    col.b*=0.95;             // slightly reduce blue component (make color "warmer"):
    return col;
  }



  float turbulence (vec3 P, int numFreq)
   {
      float val = 0.0;
      float freq = 1.0;
      for (int i=0; i<numFreq; i++) {
         val += abs (noise (P.xy *freq) / freq);
         freq *= 2.07;
      }
      return val;
   }
  
  
  vec3 marble_function(vec3 pos)
   {
      float amplitude = 10.0;
      const int roughness = 10;     // noisiness of veins (#octaves in turbulence)

      float t = 6.28 * pos.x / 1.1 ;

	  vec3 turbulence_vec = vec3(0.3*pos.y+0.5*pos.x +0.2*pos.z,0.4*pos.y+0.3*pos.x+0.3*pos.z,pos.z);
      t += amplitude * turbulence (normalize(turbulence_vec), roughness);

      //to stop it from being identical, use sin function
      t = sin(t);

	  //multiple by 0.4 so as to stop it from being too bright
      return marble_color(t);
   }





//**HORRIBLE FUNCTIONS NO ONE WANTS TO SEE T^T

// 2D Random
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return (mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y);
	
}