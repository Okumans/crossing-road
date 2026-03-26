#pragma once

#include "camera.h"
#include "utility/object.hpp"

#ifndef SHADER_PATH
#define SHADER_PATH ASSETS_PATH "/shaders"
#endif

#ifndef ICONS_PATH
#define ICONS_PATH ASSETS_PATH "/icons"
#endif

class Game {
private:
  std::unique_ptr<Object> m_player;

public:
  Game();

  void update(double delta_time);
  void render(double delta_time, Camera &camera);
};

// TODO: integrated light source helpers
// TODO: integrated world position helpers
