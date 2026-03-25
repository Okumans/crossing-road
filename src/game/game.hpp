#pragma once

#include "camera.h"
#include "utility/model.hpp"
#include <memory>

#ifndef SHADER_PATH
#define SHADER_PATH ASSETS_PATH "/shaders"
#endif

#ifndef ICONS_PATH
#define ICONS_PATH ASSETS_PATH "/icons"
#endif

struct AABB {
  glm::vec3 left;
  glm::vec3 right;
  glm::vec3 down;

  bool contains(const AABB &other);
};

class Object {
private:
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

  void draw(Shader &shader);

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

class Game {
private:
  std::unique_ptr<Object> m_player;

public:
  Game();

  void update(double delta_time);
  void render(double delta_time, Camera &camera);
};

// TODO: integrated light source helpers
// TODO: integrated world position helpers
