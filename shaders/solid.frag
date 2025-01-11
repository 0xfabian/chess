#version 460 core

in vec3 wpos;
in vec4 lpos;
in vec3 norm;

out vec4 FragColor;

uniform vec3 eye;
uniform vec3 light;
uniform sampler2D shadow_map;

uniform int white;
uniform int reflection;

float checkerboard(in vec2 p, in vec2 ddx, in vec2 ddy)
{
    // filter kernel
    vec2 w = max(abs(ddx), abs(ddy)) + 0.01;  
    // analytical integral (box filter)
    vec2 i = 2.0*(abs(fract((p-0.5*w)/2.0)-0.5)-abs(fract((p+0.5*w)/2.0)-0.5))/w;
    // xor pattern
    return 0.5 - 0.5*i.x*i.y;                  
}

float calc_shadow(vec4 light_space_pos)
{
    vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    // float closest = texture(shadow_map, proj_coords.xy).r;
    float current = proj_coords.z;

    float shadow = 0.0;

    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    int kernel = 5;
    int sz = kernel / 2;

    for(int x = -sz; x <= sz; ++x)
    {
        for(int y = -sz; y <= sz; ++y)
        {
            float pcfDepth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r; 
            shadow += current - 0.0005 > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= kernel * kernel;

    return 1 - shadow;

    // return (current - 0.0000 > closest) ? 0.5 : 1.0;  
}

void main()
{
    vec3 n = normalize(norm);

    vec3 to_light = normalize(light - wpos);
    vec3 view_dir = -normalize(eye);

    vec3 black_color = vec3(0, 0.5,0);
    vec3 white_color = vec3(1,0,0);
    vec3 check_black_color = vec3(0.02);
    vec3 check_white_color = vec3(1);
    vec3 ambient_color = vec3(0.7, 0.8, 0.9);

    vec3 color = black_color;

    if (white == 1)
        color = white_color;
    else if (white == 2)
    {
        if (wpos.x > -4 && wpos.x < 4 && wpos.z > -4 && wpos.z < 4)
        {
            float t = checkerboard(wpos.xz, vec2(0.01, 0), vec2(0, 0.01));
            color = mix(check_black_color, check_white_color, t);
        }
        else
        {
            FragColor = ambient_color;
            return;
        }
    }

    float att = 100 / pow(length(light - wpos), 2);

    vec3 ambient = 0.1 * color * ambient_color;
    vec3 diffuse = color * max(dot(to_light, n), 0.0) * att;
    vec3 spec = vec3(1) * pow(max(dot(reflect(to_light, n), view_dir), 0.0), 50.0) * att;
    float shadow = calc_shadow(lpos);

    vec3 final = ambient + shadow * (diffuse + spec);

    float alpha = 1.0;

    if (white == 2)
         alpha = 0.7;

    FragColor = vec4(pow(final, vec3(1 / 2.2)), alpha);
}