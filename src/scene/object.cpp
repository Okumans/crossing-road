#include "object.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <memory>
#include <print>

Object::Object(std::shared_ptr<Model> model, glm::vec3 pos, glm::vec3 scale,
               glm::vec3 rotation)
    : m_model(std::move(model)), m_position(pos), m_rotation(rotation),
      m_scale(scale),
      m_boundingBox(m_model ? _calculateAABB(*m_model) : AABB{}) {
  if (!m_model) {
    std::println(stderr, "Warning: Object created with null Model!");
  }
}

void Object::draw(Shader &shader) {
  glm::mat4 model = glm::mat4(1.0f);

  model = glm::translate(model, m_position);

  model = glm::rotate(model, m_rotation.x, glm::vec3(1, 0, 0));
  model = glm::rotate(model, m_rotation.y, glm::vec3(0, 1, 0));
  model = glm::rotate(model, m_rotation.z, glm::vec3(0, 0, 1));

  model = glm::scale(model, m_scale);

  shader.setMat4("model", model);
  m_model->draw(shader);
}

bool Object::collided(const Object &other) { return false; }
bool Object::collided(AABB bounding_box) { return false; }

AABB Object::_calculateAABB(const Model &model) {
  // TODO: Implement actaul bounding box calculation.

  return AABB{.left = glm::vec3(0.0f),
              .right = glm::vec3(0.0f),
              .down = glm::vec3(0.0f)};
}
