#pragma once

#include "game/row.hpp"
#include "game/rows/texture_row.hpp"
#include "game/terrain.hpp"
#include "graphics/material.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "scene/car.hpp"
#include "utility/utility.hpp"

#include <cstdint>
#include <memory>

class RoadTerrain : public Terrain {
private:
  inline static const char *ROAD_1_TEX_NAME = "road_1";
  inline static const char *ROAD_2_TEX_NAME = "road_2";

public:
  RoadTerrain(uint32_t start_z) : Terrain(start_z) {}

  virtual uint32_t _generateTerrain() override {
    assert(MaterialManager::exists(ROAD_1_TEX_NAME));
    assert(MaterialManager::exists(ROAD_2_TEX_NAME));

    const Material &road_mat_1 = MaterialManager::getMaterial(ROAD_1_TEX_NAME);
    const Material &road_mat_2 = MaterialManager::getMaterial(ROAD_2_TEX_NAME);

    size_t row_numbers = Random::randInt<size_t>(2, 5);
    const Material *start_mat = &road_mat_1;

    // FIX: please replace nullptr with the proper row (after fix things)
    const Row *row_before = nullptr;

    if (row_before && row_before->getType() == RowType::ROAD) {
      if (const auto texture_row =
              dynamic_cast<const TextureRow *>(row_before)) {
        if (texture_row->getMaterial().getDiffuse()->getTexID() ==
            road_mat_1.getDiffuse()->getTexID()) {
          start_mat = &road_mat_2;
        }
      }
    }

    uint32_t last_row_idx = 0;
    for (size_t i = 0; i < row_numbers; ++i) {
      const Material &current_mat =
          (i % 2 == 0) ? *start_mat
                       : (start_mat == &road_mat_1 ? road_mat_2 : road_mat_1);

      auto road_row =
          std::make_unique<TextureRow>(RowType::ROAD, current_mat, 1.0f, 0.05f);

      _populateLane(*road_row);

      last_row_idx = addRow(std::move(road_row));
    }

    return last_row_idx;
  }

private:
  void _populateLane(Row &row) {
    float direction = Random::randChance(0.5f) ? 1.0f : -1.0f;

    // 20% Train lane
    if (Random::randChance(0.2f)) {
      float speed = Random::randFloat(6.0f, 10.0f) * direction;
      float x = direction > 0.0f ? -15.0f : 15.0f;

      std::unique_ptr<Car> train = std::make_unique<Car>(
          ModelManager::getModel(ModelName::TRAIN_1), speed);
      train->setScale(0.3f);
      train->setRotation(
          {0.0f, glm::radians(direction > 0.0f ? 90.0f : -90.0f), 0.0f});
      train->setPosition({x, row.getHeight() + 0.3f});

      row.addObject(std::move(train));
    }

    // 80% Car lane
    else {
      float speed = Random::randFloat(1.5f, 4.5f) * direction;
      int numCars = Random::randInt(1, 2);

      for (int i = 0; i < numCars; ++i) {
        float x_pos = Random::randFloat(-10.0f, 10.0f);

        ModelName modelName =
            Random::randChance(0.5f) ? ModelName::CAR_1 : ModelName::CAR_2;
        float scale = (modelName == ModelName::CAR_1) ? 0.0030f : 0.0022f;

        auto car =
            std::make_unique<Car>(ModelManager::getModel(modelName), speed);
        car->setScale(scale);
        car->setRotation({0, glm::radians(90.0f), 0});
        car->setPosition({x_pos, row.getHeight()});

        if (direction < 0.0f)
          car->rotate({0, glm::radians(180.0f), 0});

        row.addObject(std::move(car));
      }
    }
  }
};
