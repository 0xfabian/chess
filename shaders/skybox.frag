#version 460 core

in vec3 wpos;

out vec4 FragColor;

uniform samplerCube skybox;

void main()
{
    vec3 color = texture(skybox, wpos).rgb;

    color = color / (color + vec3(1));
    color = pow(color, vec3(1 / 2.2));

    FragColor = vec4(color, 1);
}