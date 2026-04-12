#pragma once

#include "game/row.hpp"
#include "game/row_queue.hpp"
#include "game/rows/grass_row.hpp"
#include "game/terrain.hpp"
#include "graphics/material.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "utility/utility.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

class GrassyTerrain : public Terrain {
private:
  inline static const char *GRASS_1_TEX_NAME = "grass_1";
  inline static const char *GRASS_2_TEX_NAME = "grass_2";
  inline static const char *GRASS_SIDE_TEX_NAME = "road_4";

public:
  GrassyTerrain(uint32_t startRowidx) : Terrain(startRowidx) {
    _setupPopulator();
  }

private:
  virtual uint32_t _generateTerrain() override {
    enum class GrassMaterialType : uint8_t {
      GRASS_1 = 0,
      GRASS_2,
      Count // For getting element count
    };

    assert(MaterialManager::exists(GRASS_1_TEX_NAME));
    assert(MaterialManager::exists(GRASS_2_TEX_NAME));

    const Material &grass_mat_1 =
        MaterialManager::getMaterial(GRASS_1_TEX_NAME);
    const Material &grass_mat_2 =
        MaterialManager::getMaterial(GRASS_2_TEX_NAME);

    size_t row_numbers =
        Random::randWeighted<size_t>(1, 5, {20.0, 15.0, 5.0, 2.0, 1.0});
    GrassMaterialType start_grass_type =
        GrassMaterialType::GRASS_1; // default value

    const Row *row_before = RowQueue::get().getRow(m_startRowIdx - 1);

    // Maintain the pattern on grass textures
    if (!row_before || (row_before->getType() != RowType::GRASS)) {
      ; // Start with grass1, if its the first row
        // or last row isn't GRASS
    }

    else {
      // If last row is grass, which is TextureRow
      // if last row == grass1 -> switch to grass2
      // if last row != grass1 -> use grass1

      if (const auto texture_row =
              dynamic_cast<const TextureRow *>(row_before)) {
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
          static_cast<uint8_t>(GrassMaterialType::Count));

      assert(grass_mat_type != GrassMaterialType::Count);

      std::optional<Material> grass_mat;
      switch (grass_mat_type) {
      case GrassMaterialType::GRASS_1:
        grass_mat = grass_mat_1;
        break;
      case GrassMaterialType::GRASS_2:
        grass_mat = grass_mat_2;
        break;
      case GrassMaterialType::Count:
        std::unreachable();
      }

      assert(grass_mat.has_value());

      std::unique_ptr<GrassRow> grass_row = std::make_unique<GrassRow>(
          grass_mat.value(), 1.0f, 0.0f, grass_side_mat, 8.0f);

      last_row_idx = addRow(std::move(grass_row));
    }

    return last_row_idx;
  }

  void _setupPopulator() {
    TerrainPopulator greenery;

    PlacementRule base{
        .attempts = 8,
        .minX = -12.0f,
        .maxX = 12.0f,
        .zOffset = 0.25f,
    };

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::TREE_1)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.05f;
          r.minScale = 0.006f;
          r.maxScale = 0.006f;
        }));

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::BUSH_2)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.03f;
          r.minScale = 0.0025f;
          r.maxScale = 0.0025f;
          r.yOffset = 0.20f;
        }));

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::TREE_2)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.03f;
          r.minScale = 0.4f;
          r.maxScale = 0.4f;
          r.yOffset = -0.10f;
        }));

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::BUSH_1)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.02f;
          r.minScale = 0.002f;
          r.maxScale = 0.002f;
        }));

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::ROCK_1)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.02f;
          r.minScale = 0.005f;
          r.maxScale = 0.005f;
        }));

    bind(std::move(greenery));
  }
};
