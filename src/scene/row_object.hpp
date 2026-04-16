#pragma once

#include "glm/fwd.hpp"
#include "graphics/idrawable.hpp"
#include "graphics/izdrawable.hpp"
#include "graphics/model.hpp"
#include "utility/not_initialized.hpp"

#include <cassert>
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
  static inline NotInitialized<float, "s_minClipY"> s_minClipY;
  static inline NotInitialized<float, "s_maxClipY"> s_maxClipY;
  static inline NotInitialized<bool, "s_useClipY"> s_useClipY;

  inline static const glm::vec3 DEFAULT_AABB_COLLISION_SCALE_FACTOR =
      glm::vec3(0.8f, 1.0f, 0.7f);

protected:
  std::shared_ptr<Model> m_model;

  AABB m_baseAABB;          // Raw un-transformed AABB from model
  AABB m_localAABB;         // Rotated and Scaled AABB
  mutable AABB m_worldAABB; // Rotated, Scaled Clipped and Traslated AABB

  glm::vec3 m_aabbCollisionScaleFactor = DEFAULT_AABB_COLLISION_SCALE_FACTOR;
  bool m_isEnableAABBCollisionScaleFactor = true; // enable by default

private:
  glm::vec3
      m_position; // (x, y) position, z is control on draw, stored z offset
  glm::vec3 m_rotation; // X and Z rotation
  float m_rotationY = 0.0f;
  bool m_includeYInAABB = false;

  glm::vec3 m_scale;
  mutable bool m_isDirty = true;
  mutable float m_lastZ = 0.0f;

public:
  RowObject(std::shared_ptr<Model> model, glm::vec2 pos = glm::vec2(0.0f),
            float z_offset = 0.0f, glm::vec3 scale = glm::vec3(1.0f),
            glm::vec3 rotation = glm::vec3(0.0f),
            bool defer_aabb_calculation = false);

  static RowObject createWithDeferedState(std::shared_ptr<Model> model);

  RowObject(const RowObject &other) = default;
  RowObject(RowObject &&other) noexcept = default;
  RowObject &operator=(const RowObject &other) = default;
  RowObject &operator=(RowObject &&other) noexcept = default;
  virtual ~RowObject() = default;

  virtual void update(double delta_time) { (void)delta_time; }
  virtual void draw(const RenderContext &ctx, float z) override;

  void setAABBCollisionScaleFactor(glm::vec3 factor) {
    assert(factor.x >= 0 && factor.y >= 0 && factor.z >= 0);
    m_aabbCollisionScaleFactor = factor;
  }

  void setEnableAABBCollisionScaleFactor(bool is_enable) {
    m_isEnableAABBCollisionScaleFactor = is_enable;
  }

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

  void setScale(glm::vec3 scale);
  void setScale(float scale);

  /**
   * @brief Sets X and Z rotation and defines if Y rotation should be included
   * in AABB calculations.
   *
   * This method triggers a full AABB recalculation from the model.
   *
   * @param degrees X and Z rotation in degrees.
   * @param include_y_in_aabb If true, subsequent Y rotations will be included
   * in the axis-aligned bounding box (causing size changes). If false, the
   * AABB footprint remains consistent despite Y rotation.
   */
  void setRotationXZ(glm::vec2 degrees);

  /**
   * @brief Sets Y rotation.
   *
   * This method only updates the local AABB transform (no full recalculation).
   * It respects the policy set by the last call to setRotationXZ.
   *
   * @param degree in degrees
   */
  void setRotationY(float degree);

  void setIncludeYRotationInAABB(bool is_enable);

  void rotate(glm::vec3 degrees);

  glm::vec3 getPosition(float z) const {
    return {m_position.x, m_position.y, z + m_position.z};
  }
  glm::vec2 getPosition() const { return m_position; }
  float getZOffset() const { return m_position.z; }

  glm::vec3 getScale() const { return m_scale; };
  glm::vec3 getRotation() const {
    return {m_rotation.x, m_rotationY, m_rotation.z};
  }

  std::shared_ptr<Model> getModel() const { return m_model; }

  const AABB &getLocalAABB() const { return m_localAABB; }
  glm::vec3 getWorldAABBCenter(float z = 0.0f) const;
  const AABB &getWorldAABB(float z = 0.0f) const;

  bool collided(const RowObject &other);
  bool collided(AABB bounding_box);

  void setHeightClip(float min_y, float max_y);

  void recalculateAABB();

private:
  void _updateLocalAABB();
  void _updateGlobalAABB(float z = 0.0f) const;
  static AABB _calculateAABB(const Model &model,
                             const glm::mat4 &transform = glm::mat4(1.0f),
                             float min_y = std::numeric_limits<float>::lowest(),
                             float max_y = std::numeric_limits<float>::max());
};
