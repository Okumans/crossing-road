#include "row_object.hpp"
#include "graphics/idrawable.hpp"
#include "graphics/mesh.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <memory>
#include <print>

float RowObject::s_minClipY = 0.0f;
float RowObject::s_maxClipY = 0.0f;
bool RowObject::s_useClipY = false;

RowObject::RowObject(std::shared_ptr<Model> model, glm::vec2 pos,
                     float z_offset, glm::vec3 scale, glm::vec3 rotation,
                     bool defer_aabb_calculation)
    : m_model(std::move(model)), m_position({pos, z_offset}),
      m_rotation(rotation), m_scale(scale),
      m_baseAABB((!defer_aabb_calculation && m_model) ? _calculateAABB(*m_model)
                                                      : AABB{}) {
  if (!m_model) {
    std::println(stderr, "Warning: Object created with null Model!");
  }
  _updateLocalAABB(true);
}

void RowObject::_updateLocalAABB(bool deep_scan) {
  if (!m_model)
    return;

  glm::mat4 transform = glm::mat4(1.0f);
  transform = glm::rotate(transform, m_rotation.x, glm::vec3(1, 0, 0));
  transform = glm::rotate(transform, m_rotation.y, glm::vec3(0, 1, 0));
  transform = glm::rotate(transform, m_rotation.z, glm::vec3(0, 0, 1));
  transform = glm::scale(transform, m_scale);

  if (deep_scan) {
    if (s_useClipY) {
      m_localAABB = _calculateAABB(*m_model, transform, s_minClipY, s_maxClipY);
    } else {
      m_localAABB = _calculateAABB(*m_model, transform);
    }
  } else {
    m_localAABB = m_baseAABB;
    m_localAABB.transform(transform);
  }
}

void RowObject::setScale(glm::vec3 scale, bool recalculate_aabb) {
  m_scale = scale;
  _updateLocalAABB(recalculate_aabb);
  m_isDirty = true;
}

void RowObject::setScale(float scale, bool recalculate_aabb) {
  setScale(glm::vec3(scale), recalculate_aabb);
}

void RowObject::setRotation(glm::vec3 rads, bool recalculate_aabb) {
  m_rotation = rads;
  _updateLocalAABB(recalculate_aabb);
  m_isDirty = true;
}

void RowObject::rotate(glm::vec3 rads, bool recalculate_aabb) {
  m_rotation += rads;
  _updateLocalAABB(recalculate_aabb);
  m_isDirty = true;
}

glm::vec3 RowObject::getWorldAABBCenter() const {
  return m_localAABB.getCenter();
}

const AABB &RowObject::getWorldAABB(float z) const {
  if (m_isDirty || m_lastZ != z) {
    _updateGlobalAABB(z);
    m_isDirty = false;
    m_lastZ = z;
  }

  return m_worldAABB;
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

void RowObject::setHeightClip(float min_y, float max_y) {
  if (m_model) {
    m_localAABB = _calculateAABB(*m_model, glm::mat4(1.0f), min_y, max_y);
    m_isDirty = true;
  }
}

void RowObject::_updateGlobalAABB(float z) const {
  AABB updated_AABB = m_localAABB;
  updated_AABB.translate(
      glm::vec3(m_position.x, m_position.y, z + m_position.z));

  m_worldAABB = updated_AABB;
}

AABB RowObject::_calculateAABB(const Model &model, const glm::mat4 &transform,
                               float min_y, float max_y) {
  AABB aabb = AABB::empty();

  for (const Mesh &mesh : model.getMeshes()) {
    for (const Vertex &vertex : mesh.getVertices()) {
      glm::vec3 transformedPos(transform * glm::vec4(vertex.position, 1.0f));

      if (transformedPos.y >= min_y && transformedPos.y <= max_y)
        aabb.grow(transformedPos);
    }
  }

  return aabb;
}
