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

  void translate(const glm::vec3 &offset) {
    min += offset;
    max += offset;
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

  bool isEmpty() const {
    return min.x > max.x || min.y > max.y || min.z > max.z;
  }
};

class RowObject : public IZDrawable {
public:
  static float s_minClipY;
  static float s_maxClipY;
  static bool s_useClipY;

protected:
  std::shared_ptr<Model> m_model;

  AABB m_baseAABB;          // Raw un-transformed AABB from model
  AABB m_localAABB;         // Rotated and Scaled AABB
  mutable AABB m_worldAABB; // Rotated, Scaled and Traslated AABB

private:
  glm::vec3
      m_position; // (x, y) position, z is control on draw, stored z offset
  glm::vec3 m_rotation;
  glm::vec3 m_scale;
  mutable bool m_isDirty = true;
  mutable float m_lastZ = 0.0f;

public:
  RowObject(std::shared_ptr<Model> model, glm::vec2 pos = glm::vec2(0.0f),
            float z_offset = 0.0f, glm::vec3 scale = glm::vec3(1.0f),
            glm::vec3 rotation = glm::vec3(0.0f),
            bool defer_aabb_calculation = false);

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

  void translate(glm::vec2 vec) {
    m_position += glm::vec3(vec, 0.0f);
    m_isDirty = true;
  }

  void setZOffset(float offset) {
    m_position.z = offset;
    m_isDirty = true;
  }

  void setScale(glm::vec3 scale, bool recalculate_aabb = true);
  void setScale(float scale, bool recalculate_aabb = true);
  void setRotation(glm::vec3 rads, bool recalculate_aabb = true);
  void rotate(glm::vec3 rads, bool recalculate_aabb = true);

  glm::vec3 getPosition(float z) const {
    return {m_position.x, m_position.y, z + m_position.z};
  }
  glm::vec2 getPosition() const { return m_position; }
  float getZOffset() const { return m_position.z; }

  glm::vec3 getScale() const { return m_scale; };
  glm::vec3 getRotation() const { return m_rotation; }

  const AABB &getLocalAABB() const { return m_localAABB; }
  glm::vec3 getWorldAABBCenter() const;
  const AABB &getWorldAABB(float z = 0.0f) const;

  bool collided(const RowObject &other);
  bool collided(AABB bounding_box);

  void setHeightClip(float min_y, float max_y);

private:
  void _updateLocalAABB(bool deep_scan);
  void _updateGlobalAABB(float z = 0.0f) const;
  static AABB _calculateAABB(const Model &model,
                             const glm::mat4 &transform = glm::mat4(1.0f),
                             float min_y = std::numeric_limits<float>::lowest(),
                             float max_y = std::numeric_limits<float>::max());
};
