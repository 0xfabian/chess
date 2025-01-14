#version 460 core

layout (location = 0) in vec3 pos;

uniform mat4 light_mat;
uniform mat4 model_mat;

void main()
{
    gl_Position = light_mat * model_mat * vec4(pos, 1.0f);
}