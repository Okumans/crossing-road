#include "road_terrain.hpp"

#include "external/magic_enum.hpp"
#include "game/row.hpp"
#include "game/rows/road_row.hpp"
#include "graphics/material.hpp"
#include "resource/material_manager.hpp"
#include "resource/model_manager.hpp"
#include "scene/car.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

uint32_t RoadTerrain::_generateTerrain() {
  for (const auto &[type, car] : s_carTemplate.ensureInitialized().pairs()) {
    if (!car)
      throw std::runtime_error(
          std::format("Car with type=\"{}\" need to be set first",
                      magic_enum::enum_name(type)));
  }

  size_t row_numbers =
      Random::randWeighted<size_t>(1, 5, {15.0, 10.0, 5.0, 2.0, 1.0});

  RoadMaterialType start_road_type = RoadMaterialType::ROAD_1;

  const Row *row_before = RowQueue::get().getRow(m_startRowIdx - 1);

  if (row_before && row_before->getType() == RowType::ROAD) {
    if (const auto texture_row = dynamic_cast<const TextureRow *>(row_before)) {
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
        static_cast<uint8_t>(RoadMaterialType::ROAD_3));

    const auto &config = ROAD_MATERIAL_CONFIG.at(road_mat_type);
    const Material &material = MaterialManager::getMaterial(config.name);
    float uv_scale = config.uvScale;

    float direction = Random::randChance(0.5f) ? 1.0f : -1.0f;
    float speed = Random::randFloat(3.0f, 6.0f);

    auto road_row = std::make_unique<RoadRow>(material, 1.0f, 0.05f, speed,
                                              direction, uv_scale);

    // Add car templates
    road_row->addCarTemplate(std::make_unique<Car>(
        *s_carTemplate.ensureInitialized()[CarType::CAR_1]));
    road_row->addCarTemplate(std::make_unique<Car>(
        *s_carTemplate.ensureInitialized()[CarType::CAR_2]));

    // Randomly pick a pattern
    if (Random::randChance(0.3f)) {
      road_row->setPattern(TrafficPattern::CLUSTER);
    } else {
      road_row->setPattern(TrafficPattern::CONSTANT);
    }

    // Add a chance for a train lane
    if (Random::randChance(0.15f)) {
      const auto &config = ROAD_MATERIAL_CONFIG.at(RoadMaterialType::ROAD_3);
      const Material &material = MaterialManager::getMaterial(config.name);
      float uv_scale = config.uvScale;

      road_row = std::make_unique<RoadRow>(material, 1.0f, 0.05f, speed * 5.0f,
                                           direction, uv_scale);
      road_row->addCarTemplate(std::make_unique<Car>(
          *s_carTemplate.ensureInitialized()[CarType::TRAIN_1]));
      road_row->setPattern(TrafficPattern::TRAIN);
    }

    road_row->prePopulate();
    last_row_idx = addRow(std::move(road_row));
  }

  return last_row_idx;
}

void RoadTerrain::setup() {
  setCar(CarType::CAR_1,
         std::make_unique<Car>(ModelManager::getModel(ModelName::CAR_1), 0.0f,
                               glm::vec2(0.0f), 0.0f, glm::vec3(0.0030f)));

  setCar(CarType::CAR_2,
         std::make_unique<Car>(ModelManager::getModel(ModelName::CAR_2), 0.0f,
                               glm::vec2(0.0f), 0.0f, glm::vec3(0.0022f)));

  setCar(CarType::TRAIN_1,
         std::make_unique<Car>(ModelManager::getModel(ModelName::TRAIN_1), 0.0f,
                               glm::vec2(0.0f), 0.0f, glm::vec3(0.4f)));
}
