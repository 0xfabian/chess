#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uvs;
layout (location = 3) in vec3 in_tangent;

out vec3 wpos;
out vec2 uvs;
out vec4 lpos;
out vec3 normal;
out vec3 tangent;

uniform mat4 cam_mat;
uniform mat4 light_mat;
uniform mat4 model_mat;

uniform int white;
uniform int reflection;

void main()
{
    uvs = in_uvs;
    normal = in_normal;
    tangent = in_tangent;

    vec4 p = vec4(pos, 1.0f);

    if (white == 0)
    {
        p.x *= -1;
        normal.x *= -1;
        tangent.x *= -1;

        p.z *= -1;
        normal.z *= -1;
        tangent.z *= -1;        
    }

    wpos = vec3(model_mat * p);
    lpos = light_mat * model_mat * p;

    if (reflection == 1)
        wpos.y *= -1;

    gl_Position = cam_mat * vec4(wpos, 1.0);
}