#pragma once

#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/shader.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <optional>

enum class RowType { GRASS, ROAD, WATER };

class Row {
public:
  Row(float zPos, RowType type, const Material &material, float height = 0.0f,
      std::optional<Material> sideMaterial = std::nullopt);
  ~Row() = default;

  void draw(Shader &shader);
  void drawSidePanel(Shader &shader, float nextHeight, bool isForward);

  float getZ() const { return m_zPos; }
  float getHeight() const { return m_height; }
  RowType getType() const { return m_type; }

private:
  float m_zPos;
  float m_height;
  RowType m_type;
  Material m_material;
  Material m_sideMaterial;
  std::unique_ptr<Mesh> m_mesh;
  std::unique_ptr<Mesh> m_sideMesh; // Reusable side quad mesh

  void _setupMesh();
};
