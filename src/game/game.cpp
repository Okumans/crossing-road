#include "game.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "utility/model_manager.hpp"
#include "utility/shader_manager.hpp"
#include <memory>
#include <print>

Game::Game() : m_player(nullptr) {}

void Game::update(double delta_time) { ; }

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

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  if (!m_player) {
    if (ModelManager::exists("chicken")) {
      m_player = std::make_unique<Object>(ModelManager::getModel("chicken"));
    } else {
      return;
    }
  }

  glm::mat4 projection = camera.GetProjectionMatrix();
  glm::mat4 view = camera.GetViewMatrix();

  Shader &camera_shader = ShaderManager::getShader(ShaderType::CAMERA);
  camera_shader.use();

  camera_shader.setMat4("projection", projection);
  camera_shader.setMat4("view", view);

  float time = glfwGetTime();
  float translation = std::sin(time) * 0.5f;
  float rot_rads = 2 * glm::pi<float>() * (std::cos(time) * 0.5f + 0.5f);

  m_player->setPosition({0.0f, translation, 0.0f});
  m_player->setScale(0.8);
  m_player->setRotation({0.0f, rot_rads, 0.0f});

  m_player->draw(camera_shader);

  glDisable(GL_DEPTH_TEST);
}
