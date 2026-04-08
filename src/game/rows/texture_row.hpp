#pragma once

#include "game/row.hpp"
#include "graphics/idrawable.hpp"
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

  virtual void draw(const RenderContext &ctx) override;
  virtual void drawSidePanel(const RenderContext &ctx, float nextHeight,
                             bool isForward) override;

  const Material getMaterial() const { return m_material; }
  const Material getSideMaterial() const { return m_sideMaterial; }

private:
  void _setupMesh();
};
