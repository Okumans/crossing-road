#include "water_row.hpp"
#include "game/row.hpp"
#include "glad/gl.h"
#include "glm/ext/matrix_transform.hpp"
#include "graphics/shader.hpp"
#include "resource/shader_manager.hpp"

WaterRow::WaterRow(float z_pos, const Material &water_material,
                   Shader &water_shader, float depth, float height)
    : Row(z_pos, RowType::WATER, depth, height), m_material(water_material),
      m_waterShader(water_shader) {
  m_uvOffset = glm::vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
  _setupMesh();
}

WaterRow::WaterRow(const Material &water_material, Shader &water_shader,
                   float depth, float height)
    : WaterRow(0.0f, water_material, water_shader, depth, height) {}

void WaterRow::draw(Shader &shader) {
  if (shader.ID != ShaderManager::getShader(ShaderType::SHADOW).ID) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_waterShader.use();
    m_waterShader.setFloat("u_WaveStrength", 0.3f);
    m_waterShader.setVec2("u_WaveSpeed", glm::vec2(0.01f, 0.01f));
    m_waterShader.setVec3("u_BaseColor", glm::vec3(0.0, 0.4, 0.5));
    m_waterShader.setMat4(
        "u_Model", glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, m_zPos}));
    m_waterShader.setVec2("u_UVOffset", m_uvOffset);

    m_mesh->draw(m_waterShader);

    m_waterShader.setVec2("u_UVOffset", glm::vec2(0.0f));

    glDisable(GL_BLEND);

    shader.use();
  }

  for (auto &obj : m_objects) {
    obj->draw(shader);
  }
}

void WaterRow::drawSidePanel(Shader &shader, float nextHeight, bool isForward) {
  ;
}

void WaterRow::_setupMesh() {
  const float width = 30.0f;
  const float u_max = width / 6.0f;
  const float v_max = m_depth / 6.0f;

  glm::vec3 n(0, 1, 0);
  glm::vec3 t(1, 0, 0);
  glm::vec3 b(0, 0, 1);

  float half_width = width / 2.0f;

  std::vector<Vertex> vertices = {
      {{-half_width, m_height, 0.0f}, n, {0.0f, 0.0f}, t, b},
      {{half_width, m_height, 0.0f}, n, {u_max, 0.0f}, t, b},
      {{half_width, m_height, -m_depth}, n, {u_max, v_max}, t, b},
      {{-half_width, m_height, -m_depth}, n, {0.0f, v_max}, t, b}};

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

  m_mesh = std::make_unique<Mesh>(std::move(vertices), std::move(indices),
                                  m_material, glm::vec3(0.0, 0.4, 0.5), 0.8f);
}
