#pragma once

#include "game/row.hpp"
#include "game/rows/texture_row.hpp"
#include "game/terrain.hpp"
#include "graphics/material.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "utility/utility.hpp"

#include <cmath>
#include <cstdint>
#include <memory>
#include <numbers>

class HillTerrain : public Terrain {
private:
  inline static const char *GRASS_1_TEX_NAME = "grass_1";
  inline static const char *GRASS_2_TEX_NAME = "grass_2";

public:
  HillTerrain(uint32_t start_z) : Terrain(start_z) { _setupPopulator(); }

  virtual uint32_t _generateTerrain() override {
    assert(MaterialManager::exists(GRASS_1_TEX_NAME));
    assert(MaterialManager::exists(GRASS_2_TEX_NAME));

    const Material &grass_mat_1 =
        MaterialManager::getMaterial(GRASS_1_TEX_NAME);
    const Material &grass_mat_2 =
        MaterialManager::getMaterial(GRASS_2_TEX_NAME);

    size_t row_numbers = Random::randInt<size_t>(5, 9);
    float peak_height = Random::randFloat(0.4f, 1.0f);

    const Row *row_before = RowQueue::get().getRow(m_startRowIdx - 1);
    const Material *start_mat = &grass_mat_1;

    if (row_before && row_before->getType() == RowType::GRASS) {
      if (const auto texture_row =
              dynamic_cast<const TextureRow *>(row_before)) {
        if (texture_row->getMaterial().getDiffuse()->getTexID() ==
            grass_mat_1.getDiffuse()->getTexID()) {
          start_mat = &grass_mat_2;
        }
      }
    }

    uint32_t last_row_idx = 0;

    for (size_t i = 0; i < row_numbers; ++i) {
      const Material &current_mat =
          (i % 2 == 0)
              ? *start_mat
              : (start_mat == &grass_mat_1 ? grass_mat_2 : grass_mat_1);

      // Hill shape: height = peak * sin(pi * (i + 1) / (row_numbers + 1))
      float h =
          peak_height * std::sin(std::numbers::pi_v<float> * (float)(i + 1) /
                                 (float)(row_numbers + 1));

      auto grass_row =
          std::make_unique<TextureRow>(RowType::GRASS, current_mat, 1.0f, h);

      last_row_idx = addRow(std::move(grass_row));
    }

    return last_row_idx;
  }

private:
  void _setupPopulator() {
    TerrainPopulator greenery;

    PlacementRule base{
        .attempts = 45, .minX = -13.0f, .maxX = 13.0f, .zOffset = 0.25f};

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::TREE_1)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.2f;
          r.minScale = 0.006f;
          r.maxScale = 0.006f;
        }));

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::BUSH_2)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.12f;
          r.minScale = 0.0025f;
          r.maxScale = 0.0025f;
          r.yOffset = 0.20f;
        }));

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::TREE_2)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.12f;
          r.minScale = 0.4f;
          r.maxScale = 0.4f;
          r.yOffset = -0.10f;
        }));

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::BUSH_1)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.12f;
          r.minScale = 0.002f;
          r.maxScale = 0.002f;
        }));

    greenery.withRule(
        RowType::GRASS,
        std::make_unique<RowObject>(ModelManager::getModel(ModelName::ROCK_1)),
        withBase(base, [](PlacementRule &r) {
          r.probability = 0.15f; // Hills have more rocks
          r.minScale = 0.005f;
          r.maxScale = 0.005f;
        }));

    bind(std::move(greenery));
  }
};
