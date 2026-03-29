#version 450 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;
in vec4 FragPosLightSpace;

// PBR Texture Samplers
uniform sampler2D u_DiffuseTex;
uniform sampler2D u_NormalTex;
uniform sampler2D u_HeightTex;
uniform sampler2D u_MetallicRoughnessTex; // G = Roughness, B = Metallic
uniform sampler2D u_AOTex;
uniform samplerCube u_Skybox;
uniform sampler2D u_ShadowMap;

// Fallbacks & Factors
uniform vec3 u_BaseColor;
uniform float u_Opacity;
uniform float u_MetallicFactor;
uniform float u_RoughnessFactor;
uniform float u_AOFactor;
uniform float u_HeightScale;

// Lights
struct Light {
  vec3 position; // For point light: position; For directional light: direction
  vec3 color;
  int type; // 0: Point, 1: Directional
};
#define MAX_LIGHTS 4
uniform Light u_Lights[MAX_LIGHTS];
uniform int u_NumLights;

uniform vec3 u_CameraPos;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
vec3 getNormalFromMap(vec2 texCoords)
{
  vec3 tangentNormal = texture(u_NormalTex, texCoords).xyz * 2.0 - 1.0;
  return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
  float height = texture(u_HeightTex, texCoords).r;
  vec2 p = viewDir.xy / max(viewDir.z, 0.01) * (height * u_HeightScale);
  return texCoords - p;
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float nom = NdotV;
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
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;

  if (projCoords.z > 1.0)
    return 0.0;

  float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;

  float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.001);

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
  for (int x = -1; x <= 1; ++x)
  {
    for (int y = -1; y <= 1; ++y)
    {
      float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  return shadow;
}
// ----------------------------------------------------------------------------
void main()
{
  vec3 V = normalize(u_CameraPos - WorldPos);
  mat3 invTBN = transpose(TBN);
  vec3 tangentViewDir = normalize(invTBN * V);

  vec2 texCoords = ParallaxMapping(TexCoords, tangentViewDir);

  vec4 diffuseSample = texture(u_DiffuseTex, texCoords);
  float finalAlpha = diffuseSample.a * u_Opacity;

  // Very aggressive discard for "Cutout" transparency (standard for this style)
  if (finalAlpha < 0.7)
    discard;

  vec3 albedo = pow(diffuseSample.rgb, vec3(2.2)) * u_BaseColor;

  vec3 mrSample = texture(u_MetallicRoughnessTex, texCoords).rgb;
  // Add a small bias to roughness to prevent absolute mirror surfaces on foliage
  float roughness = clamp(mrSample.g * u_RoughnessFactor, 0.05, 1.0);
  float metallic = mrSample.b * u_MetallicFactor;

  float ao = texture(u_AOTex, texCoords).r * u_AOFactor;

  vec3 N = getNormalFromMap(texCoords);
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  vec3 Lo = vec3(0.0);
  int numLights = min(u_NumLights, MAX_LIGHTS);
  for (int i = 0; i < numLights; ++i)
  {
    vec3 L;
    vec3 radiance;
    if (u_Lights[i].type == 1) // Directional
    {
      L = normalize(-u_Lights[i].position);
      radiance = u_Lights[i].color;
    }
    else // Point
    {
      L = normalize(u_Lights[i].position - WorldPos);
      float distance = length(u_Lights[i].position - WorldPos);
      float attenuation = 1.0 / (distance * distance);
      radiance = u_Lights[i].color * attenuation;
    }

    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    float shadow = 0.0;
    if (i == 0) shadow = ShadowCalculation(FragPosLightSpace, N, L);

    Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * NdotL;
  }

  // IBL / Ambient
  vec3 kS_ambient = fresnelSchlick(max(dot(N, V), 0.0), F0);
  vec3 kD_ambient = 1.0 - kS_ambient;
  kD_ambient *= 1.0 - metallic;

  // Ambient Diffuse (increased slightly for better visibility in shadows)
  vec3 ambient_diffuse = kD_ambient * albedo * 0.3 * ao;

  vec3 R = reflect(-V, N);
  // Specular reflection from skybox - linearize the sample
  // We use a lower mip for rough surfaces (increased range for more blur)
  vec3 reflection = textureLod(u_Skybox, R, roughness * 7.0).rgb;
  reflection = pow(reflection, vec3(2.2));

  // Specular Ambient (IBL Specular)
  // For non-metals, we blend with albedo to prevent pure blue "plastic" look on grass
  vec3 specular_ambient = reflection * kS_ambient * ao;
  if (metallic < 0.1) {
    specular_ambient *= albedo;
  }

  vec3 color = ambient_diffuse + specular_ambient + Lo; // HDR & Gamma
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, finalAlpha);
}
