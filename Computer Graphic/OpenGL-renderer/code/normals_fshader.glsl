
#version 150
uniform vec3 line_color;;

out vec4 fColor;

void main() 
{ 
   fColor = vec4(line_color.x,line_color.y,line_color.z,1);
   //fColor = vec4(1,0,0,1);
} 

