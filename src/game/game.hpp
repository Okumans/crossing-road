#pragma once

#include "graphics/camera.hpp"
#include "graphics/skybox.hpp"
#include "map_manager.hpp"
#include "scene/object.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>

#ifndef SHADER_PATH
#define SHADER_PATH ASSETS_PATH "/shaders"
#endif

#ifndef ICONS_PATH
#define ICONS_PATH ASSETS_PATH "/icons"
#endif

class Game {
private:
  std::unique_ptr<Object> m_player;
  MapManager m_map;
  std::unique_ptr<Skybox> m_skybox;

  // Shadow mapping
  GLuint m_shadowMapFBO;
  GLuint m_shadowMapTex;
  const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
  glm::mat4 m_lightSpaceMatrix;

public:
  Game();
  ~Game();

  void update(double delta_time);
  void render(double delta_time, Camera &camera);

  void setup();

  void moveForward();
  void moveLeft(double delta_time);
  void moveRight(double delta_time);
};

// TODO: integrated light source helpers
// TODO: integrated world position helpers
