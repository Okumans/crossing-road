#include "lighting_manager.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <format>

std::vector<Light> LightingManager::m_lights = {};

void LightingManager::addLight(const Light &light) {
  if (m_lights.size() < MAX_LIGHTS) {
    m_lights.push_back(light);
  }
}

void LightingManager::setLight(size_t index, const Light &light) {
  if (index < m_lights.size()) {
    m_lights[index] = light;
  }
}

void LightingManager::clearLights() { m_lights.clear(); }

void LightingManager::apply(Shader &shader) {
  shader.setInt("u_NumLights", (int)m_lights.size());

  for (size_t i = 0; i < m_lights.size(); ++i) {
    std::string base = std::format("u_Lights[{}]", i);
    shader.setVec3(base + ".position", m_lights[i].position);
    shader.setVec3(base + ".color", m_lights[i].color);
    shader.setInt(base + ".type", (int)m_lights[i].type);
  }
}

glm::mat4
LightingManager::calculateLightSpaceMatrix(const glm::vec3 &targetPos) {
  Light shadowCaster = getShadowCaster();
  glm::vec3 lightDir = glm::normalize(shadowCaster.position);

  glm::mat4 lightProjection =
      glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 20.0f);
  glm::mat4 lightView = glm::lookAt(targetPos - lightDir * 10.0f, targetPos,
                                    glm::vec3(0.0f, 1.0f, 0.0f));

  return lightProjection * lightView;
}

bool LightingManager::hasShadowCaster() {
  for (const auto &light : m_lights) {
    if (light.castsShadows)
      return true;
  }

  return false;
}

Light LightingManager::getShadowCaster() {
  for (const auto &light : m_lights) {
    if (light.castsShadows)
      return light;
  }

  // Fallback if none (should check hasShadowCaster first)
  if (!m_lights.empty())
    return m_lights[0];

  return Light();
}
