#include "model_manager.hpp"
#include <memory>
#include <unordered_map>

std::unordered_map<ModelName, std::shared_ptr<Model>> ModelManager::models;

std::shared_ptr<Model> ModelManager::loadModel(ModelName name,
                                               const char *model_path,
                                               bool flip_vertical) {
  models[name] = std::make_shared<Model>(model_path, flip_vertical);
  return ModelManager::models.at(name);
}

std::shared_ptr<Model> ModelManager::getModel(ModelName name) {
  return ModelManager::models.at(name);
}

bool ModelManager::exists(ModelName name) {
  return ModelManager::models.find(name) != ModelManager::models.end();
}
