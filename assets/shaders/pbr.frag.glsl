#version 450 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

// PBR Texture Samplers
uniform sampler2D u_DiffuseTex0;
uniform sampler2D u_NormalTex0;
uniform sampler2D u_MetallicTex0;
uniform sampler2D u_RoughnessTex0;
uniform sampler2D u_AOTex0;
uniform samplerCube u_Skybox;

// Fallbacks
uniform vec3  u_BaseColor0;
uniform float u_MetallicFactor;
uniform float u_RoughnessFactor;

// Lights
struct Light {
    vec3 position;
    vec3 color;
};
#define MAX_LIGHTS 4
uniform Light u_Lights[MAX_LIGHTS];
uniform int u_NumLights;

uniform vec3 u_CameraPos;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(u_NormalTex0, TexCoords).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3 albedo = texture(u_DiffuseTex0, TexCoords).rgb * u_BaseColor0;
    FragColor = vec4(albedo, 1.0);
}
