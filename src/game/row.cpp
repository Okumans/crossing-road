#include "row.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>

Row::Row(float zPos, RowType type, const Material &material)
    : m_zPos(zPos), m_type(type) {
  _setupMesh(material);
}

void Row::_setupMesh(const Material &material) {
  float width = 20.0f;
  float depth = 0.5f;
  float halfWidth = width / 2.0f;
  float uMax = width / 2.0f;
  float vMax = depth / 2.0f;

  glm::vec3 n(0, 1, 0);
  glm::vec3 t(1, 0, 0);
  glm::vec3 b(0, 0, 1);

  std::vector<Vertex> vertices = {
      // Position             Normal  UV          Tangent Bitangent
      {{-halfWidth, 0, m_zPos}, n, {0.0f, 0.0f}, t, b},
      {{halfWidth, 0, m_zPos}, n, {uMax, 0.0f}, t, b},
      {{halfWidth, 0, m_zPos - depth}, n, {uMax, vMax}, t, b},
      {{-halfWidth, 0, m_zPos - depth}, n, {0.0f, vMax}, t, b}};

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

  m_mesh = std::make_unique<Mesh>(std::move(vertices), std::move(indices),
                                  material, glm::vec3(1.0f));
}

void Row::draw(Shader &shader) {
  // Identity model matrix since vertices are in world space
  shader.setMat4("u_Model", glm::mat4(1.0f));

  // Static factors for simple row materials
  shader.setFloat("u_MetallicFactor", 0.0f);
  shader.setFloat("u_RoughnessFactor", 1.0f);

  m_mesh->draw(shader);
}
