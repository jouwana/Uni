#version 150
out vec4 fColor;

in vec3 TexCoords;
uniform samplerCube skybox;

void main()
{        
    fColor = texture(skybox, TexCoords);
}