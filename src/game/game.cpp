#include "game.hpp"
#include "GLFW/glfw3.h"
#include "glad/gl.h"
#include "utility/shader_manager.hpp"
#include <cmath>

void Game::update(double delta_time) { ; }

void Game::render(double delta_time, Camera &camera) {
  glEnable(GL_DEPTH_TEST);

  glm::mat4 projection = camera.GetProjectionMatrix();
  glm::mat4 view = camera.GetViewMatrix();

  Shader &camera_shader = ShaderManager::getShader(ShaderType::CAMERA);
  camera_shader.use();

  camera_shader.setMat4("projection", projection);
  camera_shader.setMat4("view", view);

  float time = std::cos(glfwGetTime()) * 0.5;

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(
      model,
      glm::vec3(0.0f, time,
                -2.0f)); // translate it down so it's at the center of the scene
  model = glm::scale(
      model,
      glm::vec3(0.8f)); // it's a bit too big for our scene, so scale it down

  camera_shader.setMat4("model", model);

  m_model.draw(camera_shader);

  glDisable(GL_DEPTH_TEST);
}
