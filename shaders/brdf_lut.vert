#version 460 core

layout (location = 0) in vec3 pos;

out vec2 uv;

void main()
{
    uv = pos.xy * 0.5 + vec2(0.5);
    
    gl_Position = vec4(pos, 1);
}