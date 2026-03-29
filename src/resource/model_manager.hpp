#pragma once

#include "graphics/model.hpp"

#include <memory>
#include <unordered_map>

// Define model names here
enum class ModelName { CHICKEN, TREE_1, TREE_2, BUSH_1, BUSH_2, ROCK_1, CAR_1 };

class ModelManager {
public:
  static std::unordered_map<ModelName, std::shared_ptr<Model>> models;

  static std::shared_ptr<Model>
  loadModel(ModelName name, const char *model_path, bool flip_vertical = false);

  static std::shared_ptr<Model> getModel(ModelName name);

  static bool exists(ModelName name);
};
