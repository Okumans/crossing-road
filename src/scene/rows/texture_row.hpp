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

  static std::unique_ptr<Mesh> s_topMesh;
  static std::unique_ptr<Mesh> s_sideMesh;

private:
  float m_uvScaleFactor;

public:
  TextureRow(RowType type, const Material &material, float depth = 1.0f,
             float height = 0.0f,
             std::optional<Material> side_material = std::nullopt,
             float uv_scale_factor = 4.0f);

  virtual void draw(const RenderContext &ctx, float z) override;
  virtual void drawSidePanel(const RenderContext &ctx, float z,
                             float next_height, bool is_forward) override;

  const Material getMaterial() const { return m_material; }
  const Material getSideMaterial() const { return m_sideMaterial; }
  void setUVScaleFactor(float uv_scale_factor) {
    m_uvScaleFactor = uv_scale_factor;
  };
  float getUVScaleFactor() const { return m_uvScaleFactor; }

protected:
  static void _ensureStaticMeshes();
};
