#pragma once

#include "game/row.hpp"
#include "row_object.hpp"

class Car : public RowObject {
private:
  float m_speed;

public:
  Car(std::shared_ptr<Model> model, float speed,
      glm::vec2 pos = glm::vec2(0.0f), float z_offset = 0.0f,
      glm::vec3 scale = glm::vec3(1.0f), glm::vec3 rotation = glm::vec3(0.0f))
      : RowObject(model, pos, z_offset, scale, rotation), m_speed(speed) {}

  void update(double delta_time) override {
    glm::vec2 pos = getPosition();
    pos.x += m_speed * (float)delta_time;
    setPosition(pos); // Temporarily set position to get accurate world AABB

    float halfWidth = Row::WIDTH / 2.0f;
    const AABB &worldAABB = getWorldAABB(0.0f);
    const AABB &localAABB = getLocalAABB();

    // Wrap around logic
    if (m_speed > 0.0f && worldAABB.min.x > halfWidth) {
      // Moving right, fully exited right side. Wrap to left.
      // New pos.x + localAABB.max.x = -halfWidth
      pos.x = -halfWidth - localAABB.max.x;
      setPosition(pos);
    } else if (m_speed < 0.0f && worldAABB.max.x < -halfWidth) {
      // Moving left, fully exited left side. Wrap to right.
      // New pos.x + localAABB.min.x = halfWidth
      pos.x = halfWidth - localAABB.min.x;
      setPosition(pos);
    }
  }
};
