#pragma once

#include "graphics/material.hpp"
#include "graphics/mesh.hpp"
#include "graphics/shader.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

enum class RowType { GRASS, ROAD, WATER };

class Row {
public:
  Row(float zPos, RowType type, const Material &material);
  ~Row() = default;

  void draw(Shader &shader);
  float getZ() const { return m_zPos; }
  RowType getType() const { return m_type; }

private:
  float m_zPos;
  RowType m_type;
  std::unique_ptr<Mesh> m_mesh;

  void _setupMesh(const Material &material);
};
