#include "model_manager.hpp"
#include "utility/enum_map.hpp"
#include "utility/utility.hpp"
#include <memory>

SettableNotInitialized<EnumMap<ModelName, std::shared_ptr<Model>>>
    ModelManager::s_models;

std::shared_ptr<Model> ModelManager::loadModel(ModelName name,
                                               const char *model_path,
                                               bool flip_vertical) {
  s_models.set(name, std::make_shared<Model>(model_path, flip_vertical));
  return ModelManager::s_models.getUnvalidated(name);
}

std::shared_ptr<Model> ModelManager::getModel(ModelName name) {
  return ModelManager::s_models.ensureInitialized()[name];
}

bool ModelManager::exists(ModelName name) {
  return ModelManager::s_models.ensureInitialized()[name] == nullptr;
}
