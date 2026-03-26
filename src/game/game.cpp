#include "game.hpp"
#include "glm/trigonometric.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"

#include "glad/gl.h"
#include "glm/fwd.hpp"
#include <memory>

Game::Game() : m_player(nullptr) {
  // Initial map setup
  m_map.addRow(RowType::GRASS);
  m_map.addRow(RowType::ROAD);
  m_map.addRow(RowType::ROAD);
  m_map.addRow(RowType::GRASS);
  m_map.addRow(RowType::WATER);
  m_map.addRow(RowType::WATER);
  m_map.addRow(RowType::GRASS);
}

void Game::update(double delta_time) { ; }

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  glm::mat4 projection = camera.getProjectionMatrix();
  glm::mat4 view = camera.getViewMatrix();

  Shader &camera_shader = ShaderManager::getShader(ShaderType::CAMERA);
  camera_shader.use();
  camera_shader.setMat4("u_Projection", projection);
  camera_shader.setMat4("u_View", view);

  // Draw Map
  m_map.draw(camera_shader);

  if (!m_player) {
    if (ModelManager::exists(ModelName::CHICKEN)) {
      m_player =
          std::make_unique<Object>(ModelManager::getModel(ModelName::CHICKEN));
      m_player->setRotation({0, glm::radians(-90.0f), 0});

      // Center of 0.5x0.5 cell at 0.25, 0.25
      m_player->setPosition({0.25f, 0.0f, 0.25f});
    } else {
      return;
    }
  }

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
