#include "texture_row.hpp"
#include "game/row.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "graphics/idrawable.hpp"
#include <memory>

TextureRow::TextureRow(RowType type, const Material &material, float depth,
                       float height, std::optional<Material> sideMaterial)
    : Row(type, depth, height), m_material(material),
      m_sideMaterial(sideMaterial.value_or(material)) {
  m_uvOffset = glm::vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
  _setupMesh();
}

void TextureRow::draw(const RenderContext &ctx, float z) {
  ctx.shader.setMat4("u_Model",
                     glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, z}));
  ctx.shader.setVec2("u_UVOffset", m_uvOffset);
  m_mesh->draw(ctx);

  ctx.shader.setVec2("u_UVOffset", glm::vec2(0.0f));

  for (std::unique_ptr<RowObject> &obj : m_objects) {
    float center_z = -(m_depth / 2.0f);
    obj->draw(ctx, z + center_z - obj->getWorldAABBCenter().z);
  }
}

void TextureRow::drawSidePanel(const RenderContext &ctx, float z,
                               float nextHeight, bool isForward) {
  if (m_height <= nextHeight)
    return;

  float panelZ = isForward ? z : z - m_depth;

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, nextHeight, panelZ));
  if (!isForward) {
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
  }
  model = glm::scale(model, glm::vec3(1.0f, m_height - nextHeight, m_depth));

  ctx.shader.setMat4("u_Model", model);
  ctx.shader.setVec2("u_UVOffset", m_uvOffset);

  m_sideMesh->draw(ctx);
  ctx.shader.setVec2("u_UVOffset", glm::vec2(0.0f));
}

void TextureRow::_setupMesh() {
  const float width = WIDTH;
  const float u_max = width / 4.0f;
  const float v_max = m_depth / 4.0f;

  glm::vec3 n(0, 1, 0);
  glm::vec3 t(1, 0, 0);
  glm::vec3 b(0, 0, 1);

  float half_width = width / 2.0f;

  std::vector<Vertex> vertices = {
      {{-half_width, m_height, 0.0f}, n, {0.0f, 0.0f}, t, b},
      {{half_width, m_height, 0.0f}, n, {u_max, 0.0f}, t, b},
      {{half_width, m_height, -m_depth}, n, {u_max, v_max}, t, b},
      {{-half_width, m_height, -m_depth}, n, {0.0f, v_max}, t, b}};

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

  m_mesh = std::make_unique<Mesh>(std::move(vertices), std::move(indices),
                                  m_material, glm::vec3(1.0f));

  glm::vec3 side_n(0, 0, 1);
  glm::vec3 side_t(1, 0, 0);
  glm::vec3 side_b(0, 1, 0);

  std::vector<Vertex> side_vertices = {
      {{-half_width, 0.0f, 0.0f}, side_n, {0.0f, 0.0f}, side_t, side_b},
      {{half_width, 0.0f, 0.0f}, side_n, {u_max, 0.0f}, side_t, side_b},
      {{half_width, 1.0f, 0.0f}, side_n, {u_max, 1.0f}, side_t, side_b},
      {{-half_width, 1.0f, 0.0f}, side_n, {0.0f, 1.0f}, side_t, side_b}};

  std::vector<uint32_t> side_indices = {0, 1, 2, 0, 2, 3};

  m_sideMesh =
      std::make_unique<Mesh>(std::move(side_vertices), std::move(side_indices),
                             m_sideMaterial, glm::vec3(1.0f));
}
