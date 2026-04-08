#pragma once

#include "game/row.hpp"
#include "game/rows/texture_row.hpp"
#include "game/terrain.hpp"
#include "graphics/material.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "scene/car.hpp"
#include "utility/utility.hpp"

#include <memory>

class RoadTerrain : public Terrain {
private:
  inline static const char *ROAD_1_TEX_NAME = "road_1";
  inline static const char *ROAD_2_TEX_NAME = "road_2";

public:
  RoadTerrain(std::function<const Row *(float)> &before_row_getter,
              float curr_z)
      : Terrain(before_row_getter, curr_z) {}

  RoadTerrain(std::function<const Row *(float)> &&before_row_getter,
              float curr_z)
      : Terrain(std::move(before_row_getter), curr_z) {}

  virtual float _generateTerrain() override {
    assert(MaterialManager::exists(ROAD_1_TEX_NAME));
    assert(MaterialManager::exists(ROAD_2_TEX_NAME));

    const Material &road_mat_1 = MaterialManager::getMaterial(ROAD_1_TEX_NAME);
    const Material &road_mat_2 = MaterialManager::getMaterial(ROAD_2_TEX_NAME);

    size_t row_numbers = Random::randInt<size_t>(2, 5);
    const Material *start_mat = &road_mat_1;

    const Row *row_before = _getRowBeforeTerrain();
    if (row_before && row_before->getType() == RowType::ROAD) {
      if (const auto texture_row =
              dynamic_cast<const TextureRow *>(row_before)) {
        if (texture_row->getMaterial().getDiffuse()->getTexID() ==
            road_mat_1.getDiffuse()->getTexID()) {
          start_mat = &road_mat_2;
        }
      }
    }

    float curr_z = m_currZ;
    for (size_t i = 0; i < row_numbers; ++i) {
      const Material &current_mat =
          (i % 2 == 0) ? *start_mat
                       : (start_mat == &road_mat_1 ? road_mat_2 : road_mat_1);

      auto road_row = std::make_unique<TextureRow>(curr_z, RowType::ROAD,
                                                   current_mat, 1.0f, 0.05f);

      _populateLane(*road_row);

      curr_z -= road_row->getDepth();
      m_rows.push_back(std::move(road_row));
    }

    return curr_z;
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
      train->setPosition({x, row.getHeight() + 0.3f, row.getZPos() - 0.75f});

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
        car->setPosition({x_pos, row.getHeight(), row.getZPos() - 0.75f});

        if (direction < 0.0f)
          car->rotate({0, glm::radians(180.0f), 0});

        row.addObject(std::move(car));
      }
    }
  }
};
