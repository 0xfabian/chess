#version 460 core

layout (location = 0) in vec3 pos;

out vec3 wpos;

uniform mat4 cam_mat;

void main()
{
    wpos = pos;
    
    gl_Position = cam_mat * vec4(pos, 1);
}