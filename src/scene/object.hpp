#pragma once

#include "graphics/idrawable.hpp"
#include "graphics/model.hpp"

#include <glm/glm.hpp>

#include <memory>

struct AABB {
  glm::vec3 left;
  glm::vec3 right;
  glm::vec3 down;

  bool contains(const AABB &other);
};

class Object : public IDrawable {
protected:
  std::shared_ptr<Model> m_model;
  glm::vec3 m_position;
  glm::vec3 m_rotation; // Added rotation in degrees
  glm::vec3 m_scale;
  AABB m_boundingBox;

public:
  Object(std::shared_ptr<Model> model, glm::vec3 pos = glm::vec3(0.0f),
         glm::vec3 scale = glm::vec3(1.0f),
         glm::vec3 rotation = glm::vec3(0.0f));

  Object(const Object &other) = default;
  Object(Object &&other) noexcept = default;
  Object &operator=(const Object &other) = default;
  Object &operator=(Object &&other) noexcept = default;
  virtual ~Object() = default;

  virtual void update(double delta_time) { (void)delta_time; }
  virtual void draw(const RenderContext &ctx);

  void setPosition(glm::vec3 pos) { m_position = pos; }
  void setScale(glm::vec3 scale) { m_scale = scale; };
  void setScale(float scale) { m_scale = glm::vec3(scale); };
  void setRotation(glm::vec3 rads) { m_rotation = rads; }
  void rotate(glm::vec3 rads) { m_rotation += rads; }

  glm::vec3 getPosition() const { return m_position; }
  glm::vec3 getScale() const { return m_scale; };
  glm::vec3 getRotation() const { return m_rotation; }

  bool collided(const Object &other);
  bool collided(AABB bounding_box);

private:
  static AABB _calculateAABB(const Model &model);
};
