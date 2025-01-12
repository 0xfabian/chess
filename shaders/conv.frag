#version 460 core

in vec3 wpos;

out vec4 FragColor;

uniform samplerCube env;

const float PI = 3.14159265359;

void main()
{
    vec3 n = normalize(wpos);

    vec3 irradiance = vec3(0);

    vec3 right = cross(vec3(0, 1, 0), n);
    vec3 up = cross(n, right);

    float delta = 0.025;
    float samples = 0;

    for(float theta = 0; theta < 2 * PI; theta += delta)
    {
        for(float phi = 0; phi < 0.5 * PI; phi += delta)
        {
            vec3 tangent = vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
            vec3 sample_vec = tangent.x * right + tangent.y * up + tangent.z * n;

            irradiance += texture(env, sample_vec).rgb * cos(phi) * sin(phi);
            samples++;
        }
    }

    irradiance *= PI / samples;

    FragColor = vec4(irradiance, 1);
}