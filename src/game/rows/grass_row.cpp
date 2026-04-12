#include "grass_row.hpp"
#include "glm/ext/matrix_transform.hpp"

GrassRow::GrassRow(const Material &material, float depth, float height,
                   std::optional<Material> sideMaterial, float uv_scale_factor)
    : TextureRow(RowType::GRASS, material, depth, height, sideMaterial,
                 uv_scale_factor) {}

void GrassRow::drawSidePanel(const RenderContext &ctx, float z,
                             float next_height, bool is_forward) {
  if (m_height <= next_height)
    return;

  float panel_z = is_forward ? z : z - m_depth;
  float total_height = m_height - next_height;

  const float top_edge_height = 0.15f;
  const float side_detail_multiplier = 4.0f;

  // 1. Draw Top Edge Part (uses m_material)
  float draw_top_edge_height = std::min(total_height, top_edge_height);

  glm::mat4 model_top(1.0f);
  model_top = glm::translate(
      model_top, glm::vec3(0.0f, m_height - draw_top_edge_height, panel_z));

  if (!is_forward)
    model_top =
        glm::rotate(model_top, glm::radians(180.0f), glm::vec3(0, 1, 0));

  model_top =
      glm::scale(model_top, glm::vec3(1.0f, draw_top_edge_height, m_depth));

  ctx.shader.setMat4("u_Model", model_top);
  ctx.shader.setVec2("u_UVOffset", m_uvOffset);
  ctx.shader.setVec2(
      "u_UVScale", glm::vec2(1.0f, draw_top_edge_height / getUVScaleFactor()));

  m_topEdgeMesh->draw(ctx);

  // 2. Draw Main Side Part (uses m_sideMaterial)
  if (total_height > top_edge_height) {
    float main_side_height = total_height - top_edge_height;

    glm::mat4 model_side(1.0f);
    model_side =
        glm::translate(model_side, glm::vec3(0.0f, next_height, panel_z));
    if (!is_forward)
      model_side =
          glm::rotate(model_side, glm::radians(180.0f), glm::vec3(0, 1, 0));
    model_side =
        glm::scale(model_side, glm::vec3(1.0f, main_side_height, m_depth));

    ctx.shader.setMat4("u_Model", model_side);
    ctx.shader.setVec2("u_UVOffset", m_uvOffset);
    ctx.shader.setVec2("u_UVScale",
                       glm::vec2(side_detail_multiplier,
                                 main_side_height / getUVScaleFactor() *
                                     side_detail_multiplier));

    m_sideMesh->draw(ctx);
  }

  ctx.shader.setVec2("u_UVOffset", glm::vec2(0.0f));
  ctx.shader.setVec2("u_UVScale", glm::vec2(1.0f, 1.0f));
}
