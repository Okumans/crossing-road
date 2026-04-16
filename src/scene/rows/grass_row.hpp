#pragma once

#include "texture_row.hpp"

class GrassRow : public TextureRow {
public:
  GrassRow(const Material &material, float depth = 1.0f, float height = 0.0f,
           std::optional<Material> side_material = std::nullopt,
           float uv_scale_factor = 4.0f);

  virtual void drawSidePanel(const RenderContext &ctx, float z,
                             float next_height, bool is_forward) override;
};
