#include "road_row.hpp"
#include "scene/car.hpp"
#include "utility/utility.hpp"

#include <glm/gtc/constants.hpp>
#include <optional>

RoadRow::RoadRow(const Material &material, float depth, float height,
                 float speed, float direction, float uv_scale_factor)
    : TextureRow(RowType::ROAD, material, depth, height, std::nullopt,
                 uv_scale_factor),
      m_laneSpeed(speed), m_direction(direction),
      m_pattern(TrafficPattern::CONSTANT) {
  m_nextSpawnDelay = _calculateNextDelay();
  m_uvOffset = glm::vec2(0.0f);
}

void RoadRow::addCarTemplate(std::shared_ptr<Model> model, float scale) {
  m_carTemplates.push_back({model, scale});
}

void RoadRow::prePopulate() {
  if (m_carTemplates.empty())
    return;

  float half_width = WIDTH / 2.0f;
  float current_x = -half_width + 2.0f;

  while (current_x < half_width - 2.0f) {
    if (Random::randChance(0.4f)) {
      _spawnCar(current_x);
      current_x += 6.0f; // Minimum gap for pre-population
    } else {
      current_x += 2.0f;
    }
  }
}

void RoadRow::update(double delta_time) {
  // 1. Let base update handle movement and cleanup of existing cars
  TextureRow::update(delta_time);

  // 2. Handle spawning of new cars
  m_spawnTimer += delta_time;
  if (m_spawnTimer >= m_nextSpawnDelay) {
    _spawnCar();
    m_spawnTimer = 0.0;
    m_nextSpawnDelay = _calculateNextDelay();
  }
}

void RoadRow::_spawnCar(std::optional<float> override_x) {
  if (m_carTemplates.empty())
    return;

  // Pick a random template
  const auto &t =
      m_carTemplates[Random::randInt<size_t>(0, m_carTemplates.size() - 1)];

  // Create the car
  auto car = std::make_unique<Car>(t.model, m_laneSpeed * m_direction);
  car->setScale(t.scale);

  // Orient car based on direction
  if (m_direction > 0.0f) {
    car->setRotation({0, glm::radians(90.0f), 0});
  } else {
    car->setRotation({0, glm::radians(-90.0f), 0});
  }

  float x;
  if (override_x.has_value()) {
    x = override_x.value();
  } else {
    // Initial position just off-screen
    float half_width = WIDTH / 2.0f;
    const AABB &local_aabb = car->getLocalAABB();

    if (m_direction > 0.0f) {
      x = -half_width - local_aabb.max.x - 0.5f;
    } else {
      x = half_width - local_aabb.min.x + 0.5f;
    }
  }

  car->setPosition({x, m_height});
  addObject(std::move(car));
}

float RoadRow::_calculateNextDelay() {
  switch (m_pattern) {
  case TrafficPattern::CLUSTER:
    if (m_clusterRemaining > 0) {
      m_clusterRemaining--;
      return Random::randFloat(0.8f, 1.5f); // Tight grouping
    } else {
      m_clusterRemaining = Random::randInt(1, 3);
      return Random::randFloat(4.0f, 7.0f); // Large gap between clusters
    }
  case TrafficPattern::TRAIN:
    return Random::randFloat(8.0f, 15.0f); // Trains are rare
  case TrafficPattern::CONSTANT:
  default:
    return Random::randFloat(3.0f, 5.0f);
  }
}
