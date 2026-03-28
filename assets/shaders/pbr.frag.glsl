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
uniform sampler2D u_MetallicTex;
uniform sampler2D u_RoughnessTex;
uniform sampler2D u_AOTex;
uniform samplerCube u_Skybox;
uniform sampler2D u_ShadowMap;

// Fallbacks
uniform vec3 u_BaseColor;
uniform float u_MetallicFactor;
uniform float u_RoughnessFactor;
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
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0)
        return 0.0;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    return shadow;
}
// ----------------------------------------------------------------------------
void main()
{
  vec3 V = normalize(u_CameraPos - WorldPos);

  // Calculate tangent space view direction for parallax mapping
  mat3 invTBN = transpose(TBN);
  vec3 tangentViewDir = normalize(invTBN * V);

  vec2 texCoords = ParallaxMapping(TexCoords, tangentViewDir);

  // Sample textures and apply factors
  vec4 diffuseSample = texture(u_DiffuseTex, texCoords);
  if (diffuseSample.a < 0.1)
    discard;

  vec3 albedo = pow(diffuseSample.rgb, vec3(2.2)) * u_BaseColor;
  float metallic = texture(u_MetallicTex, texCoords).r * u_MetallicFactor;
  float roughness = texture(u_RoughnessTex, texCoords).r * u_RoughnessFactor;
  float ao = texture(u_AOTex, texCoords).r;

  vec3 N = getNormalFromMap(texCoords);

  // Calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 of 0.04
  // and if it's a metal, use the albedo color as F0 (metallic workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // Reflectance equation
  vec3 Lo = vec3(0.0);
  int numLights = min(u_NumLights, MAX_LIGHTS);
  for (int i = 0; i < numLights; ++i)
  {
    // Calculate per-light radiance
    vec3 L;
    vec3 radiance;
    if (u_Lights[i].type == 1) // Directional
    {
      L = normalize(-u_Lights[i].position); // position acts as direction
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

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // For energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // Multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // Scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    float shadow = 0.0;
    if (i == 0) // Assume first light is the sun which casts shadows
    {
        shadow = ShadowCalculation(FragPosLightSpace, N, L);
    }

    // Add to outgoing radiance Lo
    Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * NdotL;
  }

  // Ambient lighting
  vec3 ambient = vec3(0.03) * albedo * ao;

  vec3 color = ambient + Lo;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // Gamma correct
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, 1.0);
}
