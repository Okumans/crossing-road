#include "starter_terrain.hpp"

#include "resource/material_manager.hpp"
#include "scene/rows/grass_row.hpp"
#include "scene/rows/texture_row.hpp"

#include <cassert>
#include <cstdint>
#include <memory>

uint32_t StarterTerrain::_generateTerrain() {
  const size_t row_numbers = 3;
  return _generateTerrain(row_numbers);
}

uint32_t StarterTerrain::_generateTerrain(uint32_t row_numbers) {
  assert(MaterialManager::exists(ROAD_1_TEX_NAME));

  const Material &road_mat_1 = MaterialManager::getMaterial(ROAD_1_TEX_NAME);

  uint32_t last_row_idx = 0;

  for (size_t i = 0; i < row_numbers; ++i) {
    std::unique_ptr<TextureRow> road_row =
        std::make_unique<GrassRow>(road_mat_1, 1.0f, 0.1f);

    last_row_idx = addRow(std::move(road_row));
  }

  return last_row_idx;
}
