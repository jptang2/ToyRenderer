#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(location = 0) in vec3 IN_COLOR;
layout(location = 1) in vec2 IN_TEXCOORD;
layout(location = 2) in vec3 IN_NORMAL;
layout(location = 3) in flat uint IN_ID;

layout (location = 0) out vec4 OUT_DIFFUSE_ROUGHNESS;
layout (location = 1) out vec4 OUT_NORMAL_METALLIC;
layout (location = 2) out vec4 OUT_EMISSION;

void main() 
{
    Material material   = FetchMaterial(IN_ID);
    vec4 color          = vec4(IN_COLOR, 1.0f);
    vec4 diffuse        = FetchDiffuse(material, IN_TEXCOORD);
    vec3 emission       = FetchEmission(material);
    float roughness     = FetchRoughness(material, IN_TEXCOORD);
    float metallic      = FetchMetallic(material, IN_TEXCOORD);

    OUT_DIFFUSE_ROUGHNESS = vec4(diffuse.xyz, roughness);
    OUT_NORMAL_METALLIC = vec4(normalize(IN_NORMAL), metallic);
    OUT_EMISSION = vec4(emission, 0.0f);
}