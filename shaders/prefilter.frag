#version 460 core

in vec3 wpos;

out vec4 FragColor;

uniform samplerCube env;
uniform float roughness;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}  

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}  

// float get_mipmap(vec3 n, vec3 v, vec3 h, float roughness, int SAMPLE_COUNT)
// {
//     float NdotH = max(dot(n, h), 0.0);
//     float HdotV = max(dot(h, v), 0.0);

//     float D   = DistributionGGX(NdotH, roughness);
//     float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001; 

//     float resolution = 512.0; // resolution of source cubemap (per face)
//     float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
//     float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

//     float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
// }

void main()
{
    vec3 n = normalize(wpos);
    vec3 v = n;

    const int sample_count = 4096;
    float total_weight = 0;
    vec3 prefilter = vec3(0);

    for (int i = 0; i < sample_count; i++)
    {
        vec2 Xi = Hammersley(i, sample_count);
        vec3 h = ImportanceSampleGGX(Xi, n, roughness);
        vec3 l = reflect(-v, h);

        float n_dot_l = max(dot(n, l), 0);

        if (n_dot_l > 0)
        {
            prefilter += texture(env, l).rgb * n_dot_l;
            total_weight += n_dot_l;
        }
    }

    prefilter /= total_weight;

    FragColor = vec4(prefilter, 1);
}