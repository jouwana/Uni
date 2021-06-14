#version 150
precision highp float;

in vec4 normal_lines;

void main()
{
    gl_Position = normal_lines;
    //gl_Position.z=0;
    gl_Position.w=1;
    //frColor = line_color;
}
