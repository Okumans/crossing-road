#pragma once

#include "camera.h"
#include "core/model.hpp"

#ifndef SHADER_PATH
#define SHADER_PATH ASSETS_PATH "/shaders"
#endif

#ifndef ICONS_PATH
#define ICONS_PATH ASSETS_PATH "/icons"
#endif

class Game {
private:
  Model m_model;

public:
  Game() : m_model(Model(ASSETS_PATH "/objects/backpack/backpack.obj", true)) {}

  void update(double delta_time);
  void render(double delta_time, Camera &camera);
};
