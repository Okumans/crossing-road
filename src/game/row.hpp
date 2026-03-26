#pragma once

#include "graphics/mesh.hpp"
#include "graphics/shader.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <vector>

enum class RowType { GRASS, ROAD, WATER };

class Row {
public:
  Row(float zPos, RowType type,
      const std::vector<std::shared_ptr<Texture>> &textures);
  ~Row() = default;

  void draw(Shader &shader);
  float getZ() const { return m_zPos; }
  RowType getType() const { return m_type; }

private:
  float m_zPos;
  RowType m_type;
  std::unique_ptr<Mesh> m_mesh;

  void _setupMesh(const std::vector<std::shared_ptr<Texture>> &textures);
};
