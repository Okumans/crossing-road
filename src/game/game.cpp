#include "game.hpp"
#include "glm/trigonometric.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"

#include "glad/gl.h"
#include "glm/fwd.hpp"
#include <memory>

Game::Game() : m_player(nullptr), m_grid(std::make_unique<Grid>()) {}

void Game::update(double delta_time) { ; }

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  glm::mat4 projection = camera.getProjectionMatrix();
  glm::mat4 view = camera.getViewMatrix();

  // Draw grid
  Shader &grid_shader = ShaderManager::getShader(ShaderType::GRID);
  m_grid->draw(camera, grid_shader);

  if (!m_player) {
    if (ModelManager::exists(ModelName::CHICKEN)) {
      m_player =
          std::make_unique<Object>(ModelManager::getModel(ModelName::CHICKEN));
      m_player->setPosition({0, 0, 0.25f});
      m_player->setRotation({0, glm::radians(-90.0f), 0});
    } else {
      return;
    }
  }

  Shader &camera_shader = ShaderManager::getShader(ShaderType::CAMERA);
  camera_shader.use();

  camera_shader.setMat4("projection", projection);
  camera_shader.setMat4("view", view);

  m_player->draw(camera_shader);

  glDisable(GL_DEPTH_TEST);
}

void Game::moveForward() {
  if (m_player) {
    glm::vec3 pos = m_player->getPosition();
    pos.z -= 0.5f;
    m_player->setPosition(pos);
  }
}

void Game::moveLeft(double delta_time) {
  if (m_player) {
    glm::vec3 pos = m_player->getPosition();
    pos.x -= 2.0f * (float)delta_time;
    m_player->setPosition(pos);
  }
}

void Game::moveRight(double delta_time) {
  if (m_player) {
    glm::vec3 pos = m_player->getPosition();
    pos.x += 2.0f * (float)delta_time;
    m_player->setPosition(pos);
  }
}
