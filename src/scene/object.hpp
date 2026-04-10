#pragma once

#include "graphics/idrawable.hpp"
#include "graphics/izdrawable.hpp"
#include "graphics/model.hpp"

#include <glm/glm.hpp>

#include <memory>

struct AABB {
  glm::vec3 left;
  glm::vec3 right;
  glm::vec3 down;

  bool contains(const AABB &other);
};

class RowObject : public IZDrawable {
protected:
  std::shared_ptr<Model> m_model;
  glm::vec3
      m_position; // (x, y) position, z is control on draw, stored z offset
  glm::vec3 m_rotation;
  glm::vec3 m_scale;
  AABB m_boundingBox;

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

  void setPosition(glm::vec2 pos) { m_position = {pos, 0.0f}; }
  void setZOffset(float offset) { m_position.z = offset; }

  void setScale(glm::vec3 scale) { m_scale = scale; };
  void setScale(float scale) { m_scale = glm::vec3(scale); };
  void setRotation(glm::vec3 rads) { m_rotation = rads; }
  void rotate(glm::vec3 rads) { m_rotation += rads; }

  glm::vec3 getPosition(float z) const {
    return {m_position.x, m_position.y, z + m_position.z};
  }
  glm::vec2 getPosition() const { return m_position; }
  float getZOffset() const { return m_position.z; }

  glm::vec3 getScale() const { return m_scale; };
  glm::vec3 getRotation() const { return m_rotation; }

  bool collided(const RowObject &other);
  bool collided(AABB bounding_box);

private:
  static AABB _calculateAABB(const Model &model);
};
