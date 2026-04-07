#pragma once

#include "game/row.hpp"
#include "graphics/material.hpp"
#include "graphics/shader.hpp"

class WaterRow : public Row {
protected:
  glm::vec2 m_uvOffset;
  Material m_material;
  std::unique_ptr<Mesh> m_mesh;
  Shader &m_waterShader;

public:
  WaterRow(float z_pos, const Material &water_material, Shader &water_shader,
           float depth = 1.0f, float height = 0.0f);

  WaterRow(const Material &water_material, Shader &water_shader,
           float depth = 1.0f,
           float height = 0.0f); // For set later

  virtual void draw(const RenderContext &ctx) override;
  virtual void drawSidePanel(const RenderContext &ctx, float nextHeight,
                             bool isForward) override;

private:
  void _setupMesh();
};
