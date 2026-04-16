#include "river_terrain.hpp"

#include "game/rows/water_row.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "resource/shader_manager.hpp"
#include "scene/row_object.hpp"
#include "utility/random.hpp"
#include "utility/utility.hpp"
#include <cstdint>
#include <memory>

uint32_t RiverTerrain::_generateTerrain() {
  size_t row_numbers = Random::randInt(2, 4);

  return _generateTerrain(row_numbers);
}

uint32_t RiverTerrain::_generateTerrain(uint32_t row_numbers) {
  float row_height = Random::randFloat(-0.3f, -0.02f);

  const Material &water_mat = MaterialManager::getMaterial("water_1");
  Shader &water_shader = ShaderManager::getShader(ShaderType::WATER);

  uint32_t last_row_idx = 0;
  for (size_t i = 0; i < row_numbers; ++i) {
    auto water_row = std::make_unique<WaterRow>(water_mat, water_shader);
    water_row->setHeight(row_height);
    last_row_idx = addRow(std::move(water_row));
  }

  return last_row_idx;
}

void RiverTerrain::_ensurePopulator() {
  if (s_terrainPopulator.isInitialized())
    return;

  TerrainPopulator river_populator;

  PlacementRule base{.attempts = 40,
                     .minX = Row::SLOT_WIDTH / -2.0f,
                     .maxX = Row::SLOT_WIDTH / 2.0f,
                     .zOffset = 0.5f};

  std::unique_ptr<RowObject> lilypad_model =
      std::make_unique<RowObject>(ModelManager::getModel(ModelName::LILYPAD_1));
  lilypad_model->setEnableAABBCollisionScaleFactor(false);

  river_populator.withRule(RowType::WATER, std::move(lilypad_model),
                           withBase(base, [](PlacementRule &r) {
                             r.probability = 0.2f;
                             r.scale = 0.8f;
                             r.yOffset = 0.05f;
                           }));

  s_terrainPopulator.init(std::move(river_populator));
}
