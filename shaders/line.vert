#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 _color;

uniform mat4 view;

out vec3 color;

void main()
{
    gl_Position = view * vec4(pos, 1.0f);
    color = _color;
}