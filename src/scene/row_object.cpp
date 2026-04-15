#include "row_object.hpp"
#include "glm/fwd.hpp"
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
      m_rotation({0.0f, 0.0f, 0.0f}), m_rotationY(rotation.y), m_scale(scale) {

  if (!m_model) {
    std::println(stderr, "Warning: Object created with null Model!");
  }

  // Set rotation components and policy (Y is by default excluded for variety)
  m_rotation = {rotation.x, 0.0f, rotation.z};
  m_rotationY = rotation.y;

  if (defer_aabb_calculation) {
    m_baseAABB = AABB::empty();
    m_localAABB = AABB::empty();
  } else {
    recalculateAABB();
  }
}

RowObject RowObject::createWithDeferedState(std::shared_ptr<Model> model) {
  return RowObject(std::move(model), glm::vec2(0.0f), 0.0f, glm::vec3(0.0f),
                   glm::vec3(0.0f), true);
}

void RowObject::recalculateAABB() {
  if (!m_model)
    return;

  if (s_useClipY) {
    if (std::abs(m_scale.y) > 0.0001f) {
      float localMinY = s_minClipY / m_scale.y;
      float localMaxY = s_maxClipY / m_scale.y;

      if (m_scale.y < 0.0f)
        std::swap(localMinY, localMaxY);

      m_baseAABB =
          _calculateAABB(*m_model, glm::mat4(1.0f), localMinY, localMaxY);

    } else {
      m_baseAABB = AABB::empty();
    }
  } else {
    m_baseAABB = _calculateAABB(*m_model, glm::mat4(1.0f));
  }

  // 4. Finally, build your runtime AABB
  _updateLocalAABB();
}

void RowObject::_updateLocalAABB() {
  if (!m_model)
    return;

  glm::mat4 transform(1.0f);
  transform = glm::rotate(transform, m_rotation.x, glm::vec3(1, 0, 0));
  transform = glm::rotate(transform, m_rotation.z, glm::vec3(0, 0, 1));

  transform = glm::scale(transform, m_scale);

  m_localAABB = m_baseAABB;
  m_localAABB.transform(transform);

  // rotate around Y again (with m_localAABB center)
  if (m_includeYInAABB) {
    glm::vec3 local_aabb_center = m_localAABB.getCenter();
    glm::vec3 pivot(local_aabb_center.x, 0.0f, local_aabb_center.z);

    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, pivot);

    transform = glm::rotate(transform, m_rotationY, glm::vec3(0, 1, 0));

    transform = glm::translate(transform, -pivot);
    m_localAABB.transform(transform);
  }

  m_isDirty = true;
}

void RowObject::setScale(glm::vec3 scale) {
  m_scale = scale;
  recalculateAABB();
}

void RowObject::setScale(float scale) { setScale(glm::vec3(scale)); }

void RowObject::setRotationXZ(glm::vec2 rads, bool include_y_in_aabb) {
  m_rotation = {rads.x, 0.0f, rads.y};
  m_includeYInAABB = include_y_in_aabb;
  recalculateAABB();
}

void RowObject::setRotationY(float rads) {
  m_rotationY = rads;
  _updateLocalAABB();
}

void RowObject::rotate(glm::vec3 rads) {
  m_rotation.x += rads.x;
  m_rotation.z += rads.z;
  m_rotationY += rads.y;
  recalculateAABB();
}

glm::vec3 RowObject::getWorldAABBCenter(float z) const {
  if (m_isDirty || m_lastZ != z) {
    _updateGlobalAABB(z);
    m_isDirty = false;
    m_lastZ = z;
  }

  return m_worldAABB.getCenter();
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

  // use this to rotate around pivot (localAABB center) instead of normal
  // rotation
  glm::vec3 local_aabb_center = m_localAABB.getCenter();
  glm::vec3 pivot(local_aabb_center.x, 0.0f, local_aabb_center.z);

  // m_position.z is the relative offset within the row
  model = glm::translate(
      model, glm::vec3(m_position.x, m_position.y, z + m_position.z));

  model = glm::translate(model, pivot);

  model = glm::rotate(model, m_rotation.x, glm::vec3(1, 0, 0));
  model = glm::rotate(model, m_rotationY, glm::vec3(0, 1, 0));
  model = glm::rotate(model, m_rotation.z, glm::vec3(0, 0, 1));

  model = glm::translate(model, -pivot);

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
    // Re-scan the vertices with the new clip boundaries
    m_baseAABB = _calculateAABB(*m_model, glm::mat4(1.0f), min_y, max_y);

    // Apply the current rotation and scale to the newly clipped base
    _updateLocalAABB();
  }
}

void RowObject::_updateGlobalAABB(float z) const {
  AABB updated_aabb = m_localAABB;

  updated_aabb.translate(
      glm::vec3(m_position.x, m_position.y, z + m_position.z));

  const glm::vec3 scale_factor(AABB_COLLISION_SCALE_FACTOR, 1.0f,
                               AABB_COLLISION_SCALE_FACTOR);

  // Scale down by AABB_COLLISION_SCALE_FACTOR (for better UX)
  glm::vec3 center = (updated_aabb.min + updated_aabb.max) * 0.5f;
  glm::vec3 half_extents = (updated_aabb.max - updated_aabb.min) * 0.5f;

  glm::vec3 scaled_half_extents = half_extents * scale_factor;

  updated_aabb.min = center - scaled_half_extents;
  updated_aabb.max = center + scaled_half_extents;

  m_worldAABB = updated_aabb;
}

AABB RowObject::_calculateAABB(const Model &model, const glm::mat4 &transform,
                               float min_y, float max_y) {
  AABB aabb = AABB::empty();

  for (const Mesh &mesh : model.getMeshes()) {
    for (const Vertex &vertex : mesh.getVertices()) {
      glm::vec3 transformed_pos(transform * glm::vec4(vertex.position, 1.0f));

      if (transformed_pos.y >= min_y && transformed_pos.y <= max_y)
        aabb.grow(transformed_pos);
    }
  }

  return aabb;
}
