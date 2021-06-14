#version 150
precision highp float;

uniform int is_skycube;
in vec3 aPos;
in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Position = vec3(view*model * vec4(aPos, 1.0));
    gl_Position = projection * vec4(Position, 1.0);
}