#version 460 core

in vec3 wpos;

out vec4 FragColor;

uniform sampler2D env;

vec2 sample_spherical(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183);
    uv += 0.5;

    return uv;
}

void main()
{
    vec2 uv = sample_spherical(normalize(wpos));
    
    FragColor = texture(env, uv);
}