#version 460 core

in vec3 wpos;
in vec4 lpos;
in vec3 norm;

out vec4 FragColor;

uniform vec3 eye;
uniform vec3 light;
uniform sampler2D shadow_map;
uniform sampler3D offset_texture;
uniform samplerCube irradiance_map;
uniform samplerCube env_map;

uniform int white;

float checkerboard(in vec2 p, in vec2 ddx, in vec2 ddy)
{
    vec2 w = max(abs(ddx), abs(ddy)) + 0.01;  
    vec2 i = 2.0 * (abs(fract((p - 0.5 * w) / 2.0) - 0.5) - abs(fract((p + 0.5 * w) / 2.0) - 0.5)) / w;

    return 0.5 - 0.5 * i.x * i.y;                  
}

float calc_shadow_rs(vec4 light_space_pos)
{
    vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    float current = proj_coords.z;

    ivec3 offset_coord;
    // 16 <- window_size
    vec2 f = mod(gl_FragCoord.xy, vec2(16));
    offset_coord.yz = ivec2(f);

    vec2 texel = 1.0 / textureSize(shadow_map, 0);

    float sum = 0;
    float depth = 0;
    float radius = 5;
    float bias = 0.0005;

    for (int i = 0; i < 4; i++)
    {
        offset_coord.x = i;
        vec4 offset = texelFetch(offset_texture, offset_coord, 0) * radius;

        depth = texture(shadow_map, proj_coords.xy + offset.rg * texel).r;
        sum += (depth + bias < current) ? 0.0 : 1.0;

        depth = texture(shadow_map, proj_coords.xy + offset.ba * texel).r;
        sum += (depth + bias < current) ? 0.0 : 1.0;
    }

    float shadow = sum / 8.0;

    if (shadow != 0.0 && shadow != 1.0)
    {
        // 32 <- samples / 2
        for (int i = 4; i < 32; i++)
        {
            offset_coord.x = i;
            vec4 offset = texelFetch(offset_texture, offset_coord, 0) * radius;

            depth = texture(shadow_map, proj_coords.xy + offset.rg * texel).r;
            sum += (depth + bias < current) ? 0.0 : 1.0;

            depth = texture(shadow_map, proj_coords.xy + offset.ba * texel).r;
            sum += (depth + bias < current) ? 0.0 : 1.0;
        }

        shadow = sum / 64;
        // 64 <- samples = filter_size ^ 2
    }

    return shadow;
}

float calc_shadow(vec4 light_space_pos)
{
    vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    float current = proj_coords.z;

    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    int kernel = 15;
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
}

float ndf_ggx(float n_dot_h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (n_dot_h * n_dot_h * (a2 - 1) + 1);

    return a2 / (3.14159 * denom * denom);
}

vec3 fresenl_schlick(float cos_theta, vec3 f0)
{
    return f0 + (1 - f0) * pow(1 - cos_theta, 5);
}

float geometry_smith(float n_dot_l, float n_dot_v, float roughness)
{
    float r = roughness + 1;
    float k = (r * r) / 8;

    float ggx1 = n_dot_l / (n_dot_l * (1 - k) + k);
    float ggx2 = n_dot_v / (n_dot_v * (1 - k) + k);

    return ggx1 * ggx2;
}

vec3 calc_pbr(vec3 n, vec3 v, vec3 l, vec3 albedo, float metalic, float roughness)
{
    vec3 h = normalize(v + l);

    float n_dot_l = max(dot(n, l), 0);
    float n_dot_v = max(dot(n, v), 0);
    float n_dot_h = max(dot(n, h), 0);
    float v_dot_h = max(dot(v, h), 0);

    float attenuation = 400 / pow(length(light - wpos), 2);

    vec3 f0 = mix(vec3(0.04), albedo, metalic);

    float d = ndf_ggx(n_dot_h, roughness);
    float g = geometry_smith(n_dot_l, n_dot_h, roughness);
    vec3 f = fresenl_schlick(n_dot_h, f0);

    vec3 cook_torrance = d * g * f / (4 * n_dot_l * n_dot_v + 0.001);

    vec3 kd = (vec3(1) - f) * (1 - metalic);
    vec3 diffuse = kd * albedo / 3.14159;
    vec3 brdf = diffuse + cook_torrance;

    return brdf * n_dot_l * attenuation;
}

void main()
{
    vec3 n = normalize(norm);
    vec3 v = normalize(eye - wpos);
    vec3 l = normalize(light - wpos);

    vec3 albedo = vec3(0);
    float metalic = 0;
    float roughness = 0.2;

    if (white == 0)
    {
        albedo = vec3(0);
        metalic = 0;
    }
    if (white == 1)
    {
        albedo = vec3(1);
        metalic = 0;
    }
    else if (white == 2)
    {
        float t = checkerboard(wpos.xz, vec2(0.01, 0), vec2(0, 0.01));
        albedo = mix(vec3(0), vec3(1), t);
    }

    vec3 direct = calc_pbr(n, v, l, albedo, metalic, roughness);
    float shadow = calc_shadow_rs(lpos);

    vec3 ks = fresenl_schlick(max(dot(n, v), 0), vec3(0.04));
    vec3 kd = (vec3(1) - ks) * (1 - metalic);
    vec3 ambient = 0.5 * kd * albedo * texture(irradiance_map, n).rgb;

    vec3 final = ambient + shadow * direct + ks * texture(env_map, reflect(-v, n)).rgb;

    final = final / (final + vec3(1));
    final = pow(final, vec3(1 / 2.2));

    FragColor = vec4(final, 1);
}