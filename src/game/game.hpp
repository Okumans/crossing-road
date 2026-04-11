#pragma once

#include "graphics/camera.hpp"
#include "graphics/skybox.hpp"
#include "map_manager.hpp"
#include "scene/row_object.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <cstdint>

#ifndef SHADER_PATH
#define SHADER_PATH ASSETS_PATH "/shaders"
#endif

#ifndef ICONS_PATH
#define ICONS_PATH ASSETS_PATH "/icons"
#endif

class Game {
private:
  std::unique_ptr<RowObject> m_player;
  MapManager m_map;
  std::unique_ptr<Skybox> m_skybox;

  // Shadow mapping
  GLuint m_shadowMapFBO;
  GLuint m_shadowMapTex;
  const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
  glm::mat4 m_lightSpaceMatrix;

  float m_currentTime = 0.0f;
  uint32_t m_playerRowIdx = 0;

public:
  Game();
  ~Game();

  void update(double delta_time);
  void render(double delta_time, Camera &camera);

  void setup();

  void moveForward();
  void moveLeft(double delta_time);
  void moveRight(double delta_time);

  glm::vec3 getPlayerPosition() const;
};

// TODO: integrated world position helpers
