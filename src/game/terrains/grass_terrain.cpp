#include "grass_terrain.hpp"

#include "external/magic_enum.hpp"
#include "game/row.hpp"
#include "game/row_queue.hpp"
#include "game/rows/grass_row.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "scene/row_object.hpp"
#include "utility/utility.hpp"

#include <cassert>
#include <cstdint>

uint32_t GrassyTerrain::_generateTerrain() {
  size_t row_numbers =
      Random::randWeighted<size_t>(1, 5, {20.0, 15.0, 5.0, 2.0, 1.0});

  return _generateTerrain(row_numbers);
}

uint32_t GrassyTerrain::_generateTerrain(uint32_t row_numbers) {
  enum class GrassMaterialType : uint8_t {
    GRASS_1 = 0,
    GRASS_2,
  };

  assert(MaterialManager::exists(GRASS_1_TEX_NAME));
  assert(MaterialManager::exists(GRASS_2_TEX_NAME));

  const Material &grass_mat_1 = MaterialManager::getMaterial(GRASS_1_TEX_NAME);
  const Material &grass_mat_2 = MaterialManager::getMaterial(GRASS_2_TEX_NAME);

  GrassMaterialType start_grass_type =
      GrassMaterialType::GRASS_1; // default value

  const Row *row_before = RowQueue::get().getRow(m_startRowIdx - 1);

  // Maintain the pattern on grass textures
  if (!row_before || (row_before->getType() != RowType::GRASS)) {
    ; // Start with grass1, if its the first row
      // or last row isn't GRASS
  } else {
    // If last row is grass, which is TextureRow
    // if last row == grass1 -> switch to grass2
    // if last row != grass1 -> use grass1

    if (const auto texture_row = dynamic_cast<const TextureRow *>(row_before)) {
      if (texture_row->getMaterial().getDiffuse()->getTexID() ==
          grass_mat_1.getDiffuse()->getTexID())
        start_grass_type = GrassMaterialType::GRASS_2;
      else
        ; // else use grass1
    }
  }

  const Material &grass_side_mat =
      MaterialManager::getMaterial(GRASS_SIDE_TEX_NAME);

  uint32_t last_row_idx = 0;

  for (size_t i = 0; i < row_numbers; ++i) {
    GrassMaterialType grass_mat_type = static_cast<GrassMaterialType>(
        (static_cast<uint8_t>(start_grass_type) + i) %
        static_cast<uint8_t>(magic_enum::enum_count<GrassMaterialType>()));

    std::optional<Material> grass_mat;
    switch (grass_mat_type) {
    case GrassMaterialType::GRASS_1:
      grass_mat = grass_mat_1;
      break;
    case GrassMaterialType::GRASS_2:
      grass_mat = grass_mat_2;
      break;
    }

    assert(grass_mat.has_value());

    std::unique_ptr<GrassRow> grass_row = std::make_unique<GrassRow>(
        grass_mat.value(), 1.0f, 0.0f, grass_side_mat, 8.0f);

    last_row_idx = addRow(std::move(grass_row));
  }

  return last_row_idx;
}

void GrassyTerrain::_ensurePopulator() {
  if (s_terrainPopulator.isInitialized())
    return;

  TerrainPopulator greenery;

  PlacementRule base{.attempts = 45,
                     .minX = GrassRow::SLOT_WIDTH / -2.0f,
                     .maxX = GrassRow::SLOT_WIDTH / 2.0f,
                     .zOffset = 0.25f};

  greenery.withRule(
      RowType::GRASS,
      std::make_unique<RowObject>(ModelManager::getModel(ModelName::TREE_1)),
      withBase(base, [](PlacementRule &r) {
        r.probability = 0.02f;
        r.scale = 0.006f;
      }));

  greenery.withRule(
      RowType::GRASS,
      std::make_unique<RowObject>(ModelManager::getModel(ModelName::BUSH_2)),
      withBase(base, [](PlacementRule &r) {
        r.probability = 0.2f;
        r.scale = 0.0025f;
        r.yOffset = 0.20f;
      }));

  greenery.withRule(
      RowType::GRASS,
      std::make_unique<RowObject>(ModelManager::getModel(ModelName::TREE_2)),
      withBase(base, [](PlacementRule &r) {
        r.probability = 0.05f;
        r.scale = 0.4f;
        r.yOffset = -0.10f;
      }));

  greenery.withRule(
      RowType::GRASS,
      std::make_unique<RowObject>(ModelManager::getModel(ModelName::BUSH_1)),
      withBase(base, [](PlacementRule &r) {
        r.probability = 0.3f;
        r.scale = 0.002f;
      }));

  greenery.withRule(
      RowType::GRASS,
      std::make_unique<RowObject>(ModelManager::getModel(ModelName::ROCK_1)),
      withBase(base, [](PlacementRule &r) {
        r.probability = 0.05f;
        r.scale = 0.005f;
      }));

  s_terrainPopulator.init(std::move(greenery));
}
