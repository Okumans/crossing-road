#include "object.hpp"
#include "graphics/idrawable.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <memory>
#include <print>

RowObject::RowObject(std::shared_ptr<Model> model, glm::vec2 pos,
                     float z_offset, glm::vec3 scale, glm::vec3 rotation)
    : m_model(std::move(model)), m_position({pos, z_offset}),
      m_rotation(rotation), m_scale(scale),
      m_boundingBox(m_model ? _calculateAABB(*m_model) : AABB{}) {
  if (!m_model) {
    std::println(stderr, "Warning: Object created with null Model!");
  }
}
void RowObject::draw(const RenderContext &ctx, float z) {
  glm::mat4 model = glm::mat4(1.0f);

  // m_position.z is the relative offset within the row
  model = glm::translate(
      model, glm::vec3(m_position.x, m_position.y, z + m_position.z));

  model = glm::rotate(model, m_rotation.x, glm::vec3(1, 0, 0));
  model = glm::rotate(model, m_rotation.y, glm::vec3(0, 1, 0));
  model = glm::rotate(model, m_rotation.z, glm::vec3(0, 0, 1));

  model = glm::scale(model, m_scale);

  ctx.shader.setMat4("u_Model", model);
  m_model->draw(ctx);
}

bool RowObject::collided(const RowObject &other) { return false; }
bool RowObject::collided(AABB bounding_box) { return false; }

AABB RowObject::_calculateAABB(const Model &model) {
  // TODO: Implement actaul bounding box calculation.

  return AABB{.left = glm::vec3(0.0f),
              .right = glm::vec3(0.0f),
              .down = glm::vec3(0.0f)};
}
