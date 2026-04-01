#include "row.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <optional>
#include <print>
#include <vector>

Row::Row(float zPos, RowType type, const Material &material, float height,
         std::optional<Material> sideMaterial)
    : m_zPos(zPos), m_height(height), m_type(type), m_material(material),
      m_sideMaterial(sideMaterial.value_or(material)) {
  m_uvOffset = glm::vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
  _setupMesh();
}

void Row::_setupMesh() {
  float width = 30.0f;
  float depth = 0.5f;
  float halfWidth = width / 2.0f;
  float uMax = width / 6.0f;
  float vMax = depth / 6.0f;

  glm::vec3 n(0, 1, 0);
  glm::vec3 t(1, 0, 0);
  glm::vec3 b(0, 0, 1);

  std::vector<Vertex> vertices = {
      // Position                               Normal  UV          Tangent
      // Bitangent
      {{-halfWidth, m_height, m_zPos}, n, {0.0f, 0.0f}, t, b},
      {{halfWidth, m_height, m_zPos}, n, {uMax, 0.0f}, t, b},
      {{halfWidth, m_height, m_zPos - depth}, n, {uMax, vMax}, t, b},
      {{-halfWidth, m_height, m_zPos - depth}, n, {0.0f, vMax}, t, b}};

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

  m_mesh = std::make_unique<Mesh>(std::move(vertices), std::move(indices),
                                  m_material, glm::vec3(1.0f));

  // Setup side mesh (a vertical quad facing front or back)
  // We'll use a unit quad and scale it in drawSidePanel
  std::vector<Vertex> sideVertices = {
      {{-halfWidth, 0.0f, 0.0f}, {0, 0, 1}, {0.0f, 0.0f}, t, {0, 1, 0}},
      {{halfWidth, 0.0f, 0.0f}, {0, 0, 1}, {uMax, 0.0f}, t, {0, 1, 0}},
      {{halfWidth, 1.0f, 0.0f}, {0, 0, 1}, {uMax, vMax}, t, {0, 1, 0}},
      {{-halfWidth, 1.0f, 0.0f}, {0, 0, 1}, {0.0f, vMax}, t, {0, 1, 0}}};
  std::vector<uint32_t> sideIndices = {0, 1, 2, 0, 2, 3};
  m_sideMesh =
      std::make_unique<Mesh>(std::move(sideVertices), std::move(sideIndices),
                             m_sideMaterial, glm::vec3(1.0f));
}

void Row::update(double delta_time) {
  for (auto &obj : m_objects) {
    obj->update(delta_time);
  }
}

void Row::draw(Shader &shader) {
  shader.setMat4("u_Model", glm::mat4(1.0f));
  shader.setVec2("u_UVOffset", m_uvOffset);
  m_mesh->draw(shader);

  shader.setVec2("u_UVOffset", glm::vec2(0.0f));
  for (auto &obj : m_objects) {
    obj->draw(shader);
  }
}

void Row::drawSidePanel(Shader &shader, float nextHeight, bool isForward) {
  if (m_height <= nextHeight)
    return;

  float depth = 0.5f;
  float z = isForward ? m_zPos : m_zPos - depth;

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, nextHeight, z));
  model = glm::scale(model, glm::vec3(1.0f, m_height - nextHeight, 1.0f));

  shader.setMat4("u_Model", model);
  shader.setVec2("u_UVOffset", m_uvOffset);

  m_sideMesh->draw(shader);
  shader.setVec2("u_UVOffset", glm::vec2(0.0f));
}

void Row::addObject(std::unique_ptr<Object> object) {
  m_objects.push_back(std::move(object));
}
