#pragma once

#include "object.hpp"
#include <print>

class Car : public Object {
private:
  float m_speed;

public:
  Car(std::shared_ptr<Model> model, float speed,
      glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f),
      glm::vec3 rotation = glm::vec3(0.0f))
      : Object(model, pos, scale, rotation), m_speed(speed) {}

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
