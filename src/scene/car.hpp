#pragma once

#include "object.hpp"

class Car : public RowObject {
private:
  float m_speed;

public:
  Car(std::shared_ptr<Model> model, float speed,
      glm::vec2 pos = glm::vec2(0.0f), float z_offset = 0.0f,
      glm::vec3 scale = glm::vec3(1.0f), glm::vec3 rotation = glm::vec3(0.0f))
      : RowObject(model, pos, z_offset, scale, rotation), m_speed(speed) {}

  void update(double delta_time) override {
    m_position.x += m_speed * (float)delta_time;

    // Wrap around
    if (m_speed > 0.0f && m_position.x > 15.0f) {
      m_position.x = -15.0f;
    } else if (m_speed < 0.0f && m_position.x < -15.0f) {
      m_position.x = 15.0f;
    }
  }
};
