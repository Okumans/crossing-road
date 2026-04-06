#pragma once

#include "game/row.hpp"
#include "graphics/material.hpp"
#include <optional>

class TextureRow : public Row {
protected:
  glm::vec2 m_uvOffset;
  Material m_material;
  Material m_sideMaterial;
  std::unique_ptr<Mesh> m_mesh;
  std::unique_ptr<Mesh> m_sideMesh;

public:
  TextureRow(float z_pos, RowType type, const Material &material,
             float depth = 1.0f, float height = 0.0f,
             std::optional<Material> sideMaterial = std::nullopt);

  TextureRow(
      RowType type, const Material &material, float depth = 1.0f,
      float height = 0.0f,
      std::optional<Material> sideMaterial = std::nullopt); // For set later

  virtual void draw(Shader &shader) override;
  virtual void drawSidePanel(Shader &shader, float nextHeight,
                             bool isForward) override;

private:
  void _setupMesh();
};
