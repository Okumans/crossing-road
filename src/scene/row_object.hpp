#pragma once

#include "graphics/idrawable.hpp"
#include "graphics/izdrawable.hpp"
#include "graphics/model.hpp"

#include <glm/glm.hpp>

#include <memory>

struct AABB {
  glm::vec3 min{0.0f};
  glm::vec3 max{0.0f};

  glm::vec3 getCenter() const { return (min + max) * 0.5f; }

  glm::vec3 getSize() const { return max - min; }

  bool intersects(const AABB &other) const {
    return glm::all(glm::lessThanEqual(min, other.max)) &&
           glm::all(glm::greaterThanEqual(max, other.min));
  }

  bool contains(const glm::vec3 &point) const {
    return glm::all(glm::greaterThanEqual(point, min)) &&
           glm::all(glm::lessThanEqual(point, max));
  }

  void grow(const glm::vec3 &point) {
    min = glm::min(min, point);
    max = glm::max(max, point);
  }

  void transform(const glm::mat4 &matrix) {
    glm::vec3 corners[8] = {{min.x, min.y, min.z}, {max.x, min.y, min.z},
                            {min.x, max.y, min.z}, {max.x, max.y, min.z},
                            {min.x, min.y, max.z}, {max.x, min.y, max.z},
                            {min.x, max.y, max.z}, {max.x, max.y, max.z}};

    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(std::numeric_limits<float>::lowest());

    for (int i = 0; i < 8; ++i) {
      glm::vec3 transformed = glm::vec3(matrix * glm::vec4(corners[i], 1.0f));
      grow(transformed);
    }
  }

  static AABB empty() {
    return {glm::vec3(std::numeric_limits<float>::max()),
            glm::vec3(std::numeric_limits<float>::lowest())};
  }
};

class RowObject : public IZDrawable {
protected:
  std::shared_ptr<Model> m_model;
  glm::vec3
      m_position; // (x, y) position, z is control on draw, stored z offset
  glm::vec3 m_rotation;
  glm::vec3 m_scale;

  AABB m_localAABB;
  mutable AABB m_worldAABB;

  mutable bool m_isDirty = true;

public:
  RowObject(std::shared_ptr<Model> model, glm::vec2 pos = glm::vec2(0.0f),
            float z_offset = 0.0f, glm::vec3 scale = glm::vec3(1.0f),
            glm::vec3 rotation = glm::vec3(0.0f));

  RowObject(const RowObject &other) = default;
  RowObject(RowObject &&other) noexcept = default;
  RowObject &operator=(const RowObject &other) = default;
  RowObject &operator=(RowObject &&other) noexcept = default;
  virtual ~RowObject() = default;

  virtual void update(double delta_time) { (void)delta_time; }
  virtual void draw(const RenderContext &ctx, float z) override;

  void setPosition(glm::vec2 pos) {
    m_position = {pos, 0.0f};
    m_isDirty = true;
  }

  void setZOffset(float offset) {
    m_position.z = offset;
    m_isDirty = true;
  }

  void setScale(glm::vec3 scale) {
    m_scale = scale;
    m_isDirty = true;
  };

  void setScale(float scale) {
    m_scale = glm::vec3(scale);
    m_isDirty = true;
  };

  void setRotation(glm::vec3 rads) {
    m_rotation = rads;
    m_isDirty = true;
  }

  void rotate(glm::vec3 rads) {
    m_rotation += rads;
    m_isDirty = true;
  }

  glm::vec3 getPosition(float z) const {
    return {m_position.x, m_position.y, z + m_position.z};
  }
  glm::vec2 getPosition() const { return m_position; }
  float getZOffset() const { return m_position.z; }

  glm::vec3 getScale() const { return m_scale; };
  glm::vec3 getRotation() const { return m_rotation; }

  const AABB &getLocalAABB() const { return m_localAABB; }
  const AABB &getWorldAABB(float z = 0.0f) const {
    if (m_isDirty) {
      _updateAABB(z);
      m_isDirty = true;
    }

    return m_worldAABB;
  }

  bool collided(const RowObject &other);
  bool collided(AABB bounding_box);

private:
  void _updateAABB(float z = 0.0f) const;
  static AABB _calculateAABB(const Model &model);
};
