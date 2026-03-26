#pragma once

#include "graphics/model.hpp"

#include <memory>
#include <string>
#include <unordered_map>

struct ModelName {
  const std::string name;

  constexpr ModelName(const char *name) : name(name) {}
  constexpr ModelName(std::string &&name) : name(name) {}

  bool operator==(const ModelName &) const = default;
};

class ModelManager {
public:
  static std::unordered_map<ModelName, std::shared_ptr<Model>> models;

  static std::shared_ptr<Model>
  loadModel(ModelName name, const char *model_path, bool flip_vertical = false);

  static std::shared_ptr<Model> getModel(ModelName name);

  static bool exists(ModelName name);
};
