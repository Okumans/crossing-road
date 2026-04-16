#include "texture_row.hpp"

#include "game/row.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "graphics/idrawable.hpp"
#include <memory>

std::unique_ptr<Mesh> TextureRow::s_topMesh = nullptr;
std::unique_ptr<Mesh> TextureRow::s_sideMesh = nullptr;

TextureRow::TextureRow(RowType type, const Material &material, float depth,
                       float height, std::optional<Material> side_material,
                       float uv_scale_factor)
    : Row(type, depth, height), m_material(material),
      m_sideMaterial(side_material.value_or(material)),
      m_uvScaleFactor(uv_scale_factor) {
  m_uvOffset = glm::vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
  _ensureStaticMeshes();
}

void TextureRow::draw(const RenderContext &ctx, float z) {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), {0.0f, m_height, z});
  model = glm::scale(model, glm::vec3(WIDTH, 1.0f, m_depth));

  ctx.shader.setMat4("u_Model", model);
  ctx.shader.setVec2("u_UVOffset", m_uvOffset);
  ctx.shader.setVec2("u_UVScale", glm::vec2(WIDTH / m_uvScaleFactor,
                                            m_depth / m_uvScaleFactor));

  s_topMesh->draw(ctx, m_material);

  ctx.shader.setVec2("u_UVOffset", glm::vec2(0.0f));
  ctx.shader.setVec2("u_UVScale", glm::vec2(1.0f, 1.0f));

  for (std::unique_ptr<RowObject> &obj : m_objects) {
    float center_z = -(m_depth / 2.0f);
    float obj_z = z + center_z - obj->getWorldAABBCenter().z;

    obj->draw(ctx, obj_z);
  }
}

void TextureRow::drawSidePanel(const RenderContext &ctx, float z,
                               float next_height, bool is_forward) {
  if (m_height <= next_height)
    return;

  float panel_z = is_forward ? z : z - m_depth;
  float panel_height = m_height - next_height;

  glm::mat4 model(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, next_height, panel_z));
  if (!is_forward)
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(WIDTH, panel_height, m_depth));

  ctx.shader.setMat4("u_Model", model);
  ctx.shader.setVec2("u_UVOffset", m_uvOffset);
  ctx.shader.setVec2("u_UVScale", glm::vec2(WIDTH / m_uvScaleFactor,
                                            panel_height / m_uvScaleFactor));

  s_sideMesh->draw(ctx, m_sideMaterial);

  ctx.shader.setVec2("u_UVOffset", glm::vec2(0.0f));
  ctx.shader.setVec2("u_UVScale", glm::vec2(1.0f, 1.0f));
}

void TextureRow::_ensureStaticMeshes() {
  if (s_topMesh)
    return;

  // 1. Top Mesh (Horizontal plane)
  // Unit size: X from -0.5 to 0.5, Z from 0 to -1
  glm::vec3 n(0, 1, 0);
  glm::vec3 t(1, 0, 0);
  glm::vec3 b(0, 0, 1);

  std::vector<Vertex> top_vertices = {
      {{-0.5f, 0.0f, 0.0f}, n, {0.0f, 0.0f}, t, b},
      {{0.5f, 0.0f, 0.0f}, n, {1.0f, 0.0f}, t, b},
      {{0.5f, 0.0f, -1.0f}, n, {1.0f, 1.0f}, t, b},
      {{-0.5f, 0.0f, -1.0f}, n, {0.0f, 1.0f}, t, b}};

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

  s_topMesh = std::make_unique<Mesh>(
      std::move(top_vertices), std::vector<uint32_t>(indices),
      Material::builder().create(), glm::vec3(1.0f));

  // 2. Side Mesh (Vertical plane)
  // Unit size: X from -0.5 to 0.5, Y from 0 to 1
  glm::vec3 side_n(0, 0, 1);
  glm::vec3 side_t(1, 0, 0);
  glm::vec3 side_b(0, 1, 0);

  std::vector<Vertex> side_vertices = {
      {{-0.5f, 0.0f, 0.0f}, side_n, {0.0f, 0.0f}, side_t, side_b},
      {{0.5f, 0.0f, 0.0f}, side_n, {1.0f, 0.0f}, side_t, side_b},
      {{0.5f, 1.0f, 0.0f}, side_n, {1.0f, 1.0f}, side_t, side_b},
      {{-0.5f, 1.0f, 0.0f}, side_n, {0.0f, 1.0f}, side_t, side_b}};

  s_sideMesh =
      std::make_unique<Mesh>(std::move(side_vertices), std::move(indices),
                             Material::builder().create(), glm::vec3(1.0f));
}
