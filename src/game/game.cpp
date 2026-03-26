#include "game.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"

#include "GLFW/glfw3.h"
#include "glad/gl.h"
#include "glm/ext/scalar_constants.hpp"
#include "glm/fwd.hpp"
#include <memory>

Game::Game() : m_player(nullptr) {}

void Game::update(double delta_time) { ; }

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  if (!m_player) {
    if (ModelManager::exists("chicken")) {
      m_player = std::make_unique<Object>(ModelManager::getModel("chicken"));
    } else {
      return;
    }
  }

  glm::mat4 projection = camera.getProjectionMatrix();
  glm::mat4 view = camera.getViewMatrix();

  Shader &camera_shader = ShaderManager::getShader(ShaderType::CAMERA);
  camera_shader.use();

  camera_shader.setMat4("projection", projection);
  camera_shader.setMat4("view", view);

  float time = glfwGetTime();
  float translation = std::sin(time) * 0.5f;
  float rot_rads = 2 * glm::pi<float>() * (std::cos(time) * 0.5f + 0.5f);
  float scale = (std::cos(2 * time) * 0.5f + 1.0f);

  m_player->setPosition({0.0f, translation, 0.0f});
  m_player->setScale(scale);
  m_player->setRotation({0.0f, rot_rads, 0.0f});

  m_player->draw(camera_shader);

  glDisable(GL_DEPTH_TEST);
}
