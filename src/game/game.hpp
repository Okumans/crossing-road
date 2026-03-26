#pragma once

#include "graphics/camera.hpp"
#include "map_manager.hpp"
#include "scene/object.hpp"

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

public:
  Game();

  void update(double delta_time);
  void render(double delta_time, Camera &camera);

  void moveForward();
  void moveLeft(double delta_time);
  void moveRight(double delta_time);
};

// TODO: integrated light source helpers
// TODO: integrated world position helpers
