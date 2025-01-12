#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

out vec3 wpos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    wpos = pos;
    
    gl_Position = projection * view * vec4(pos, 1);
}