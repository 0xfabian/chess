#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

out vec3 wpos;
out vec4 lpos;
out vec3 norm;

uniform mat4 cam_mat;
uniform mat4 light_mat;
uniform mat4 model_mat;

uniform int white;
uniform int reflection;

void main()
{
    norm = normal;

    vec4 p = vec4(pos, 1.0f);

    if (white == 0)
    {
        p.z *= -1;
        norm.z *= -1;
    }
    
    wpos = vec3(model_mat * p);
    lpos = light_mat * model_mat * p;

    if (reflection == 1)
        wpos.y *= -1;

    gl_Position = cam_mat * vec4(wpos, 1.0);
}