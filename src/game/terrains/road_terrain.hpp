#pragma once

#include "game/row.hpp"
#include "game/rows/road_row.hpp"
#include "game/rows/texture_row.hpp"
#include "game/terrain.hpp"
#include "graphics/material.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "utility/utility.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <utility>

class RoadTerrain : public Terrain {
private:
  inline static const char *ROAD_1_TEX_NAME = "road_1";
  inline static const char *ROAD_2_TEX_NAME = "road_2";

public:
  RoadTerrain(uint32_t start_z) : Terrain(start_z) {}

  virtual uint32_t _generateTerrain() override {
    enum class RoadMaterialType : uint8_t {
      ROAD_1 = 0,
      ROAD_2,
      Count // For getting element count
    };

    struct RoadMaterialConfig {
      const char *name;
      float uv_scale;
    };

    static const std::map<RoadMaterialType, RoadMaterialConfig>
        ROAD_MATERIAL_CONFIG = {
            {RoadMaterialType::ROAD_1, {ROAD_1_TEX_NAME, 4.0f}},
            {RoadMaterialType::ROAD_2, {ROAD_2_TEX_NAME, 1.0f}}};

    for (uint8_t i = 0; i < static_cast<uint8_t>(RoadMaterialType::Count);
         ++i) {
      assert(MaterialManager::exists(
          ROAD_MATERIAL_CONFIG.at(static_cast<RoadMaterialType>(i)).name));
    }

    size_t row_numbers =
        Random::randWeighted<size_t>(1, 5, {15.0, 10.0, 5.0, 2.0, 1.0});

    RoadMaterialType start_road_type = RoadMaterialType::ROAD_1;

    const Row *row_before = RowQueue::get().getRow(m_startRowIdx - 1);

    if (row_before && row_before->getType() == RowType::ROAD) {
      if (const auto texture_row =
              dynamic_cast<const TextureRow *>(row_before)) {
        if (texture_row->getMaterial().getDiffuse()->getTexID() ==
            MaterialManager::getMaterial(ROAD_1_TEX_NAME)
                .getDiffuse()
                ->getTexID()) {
          start_road_type = RoadMaterialType::ROAD_2;
        }
      }
    }

    uint32_t last_row_idx = 0;
    for (size_t i = 0; i < row_numbers; ++i) {
      RoadMaterialType road_mat_type = static_cast<RoadMaterialType>(
          (static_cast<uint8_t>(start_road_type) + i) %
          static_cast<uint8_t>(RoadMaterialType::Count));

      const auto &config = ROAD_MATERIAL_CONFIG.at(road_mat_type);
      const Material &material = MaterialManager::getMaterial(config.name);
      float uv_scale = config.uv_scale;

      float direction = Random::randChance(0.5f) ? 1.0f : -1.0f;
      float speed = Random::randFloat(3.0f, 6.0f);

      auto road_row = std::make_unique<RoadRow>(material, 1.0f, 0.05f, speed,
                                                direction, uv_scale);

      // Add car templates
      road_row->addCarTemplate(ModelManager::getModel(ModelName::CAR_1),
                               0.0030f);
      road_row->addCarTemplate(ModelManager::getModel(ModelName::CAR_2),
                               0.0022f);

      // Randomly pick a pattern
      if (Random::randChance(0.3f)) {
        road_row->setPattern(TrafficPattern::CLUSTER);
      } else {
        road_row->setPattern(TrafficPattern::CONSTANT);
      }

      // Add a chance for a train lane
      if (Random::randChance(0.15f)) {
        road_row = std::make_unique<RoadRow>(material, 1.0f, 0.05f,
                                             speed * 2.0f, direction, uv_scale);
        road_row->addCarTemplate(ModelManager::getModel(ModelName::TRAIN_1),
                                 0.3f);
        road_row->setPattern(TrafficPattern::TRAIN);
      }

      road_row->prePopulate();
      last_row_idx = addRow(std::move(road_row));
    }

    return last_row_idx;
  }
};
