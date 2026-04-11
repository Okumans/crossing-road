#include "row_object.hpp"
#include "graphics/idrawable.hpp"
#include "graphics/mesh.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <memory>
#include <print>

RowObject::RowObject(std::shared_ptr<Model> model, glm::vec2 pos,
                     float z_offset, glm::vec3 scale, glm::vec3 rotation)
    : m_model(std::move(model)), m_position({pos, z_offset}),
      m_rotation(rotation), m_scale(scale),
      m_localAABB(m_model ? _calculateAABB(*m_model) : AABB{}) {
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

bool RowObject::collided(const RowObject &other) {
  return getWorldAABB().intersects(other.getWorldAABB());
}

bool RowObject::collided(AABB bounding_box) {
  return getWorldAABB().intersects(bounding_box);
}

void RowObject::_updateAABB(float z) const {
  glm::mat4 model = glm::mat4(1.0f);

  model = glm::translate(
      model, glm::vec3(m_position.x, m_position.y, z + m_position.z));

  model = glm::rotate(model, m_rotation.x, glm::vec3(1, 0, 0));
  model = glm::rotate(model, m_rotation.y, glm::vec3(0, 1, 0));
  model = glm::rotate(model, m_rotation.z, glm::vec3(0, 0, 1));

  model = glm::scale(model, m_scale);

  AABB updated_AABB = m_localAABB;
  updated_AABB.transform(model);

  m_worldAABB = updated_AABB;
}

AABB RowObject::_calculateAABB(const Model &model) {
  AABB aabb = AABB::empty();

  for (const Mesh &mesh : model.getMeshes()) {
    for (const Vertex vertex : mesh.getVertices()) {
      aabb.grow(vertex.position);
    }
  }

  return aabb;
}
