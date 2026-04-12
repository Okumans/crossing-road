#pragma once

#include "texture_row.hpp"
#include <optional>
#include <vector>

enum class TrafficPattern {
  CONSTANT, // Evenly spaced cars
  CLUSTER,  // Groups of cars
  TRAIN     // Special long vehicle behavior
};

class RoadRow : public TextureRow {
private:
  float m_laneSpeed;
  float m_direction; // 1.0 or -1.0
  TrafficPattern m_pattern;

  double m_spawnTimer = 0.0;
  double m_nextSpawnDelay = 0.0;

  // Pattern specific state
  int m_clusterRemaining = 0;
  bool m_isWaitingForTrain = false;

  // Car templates for this lane
  struct CarTemplate {
    std::shared_ptr<Model> model;
    float scale;
  };
  std::vector<CarTemplate> m_carTemplates;

public:
  RoadRow(const Material &material, float depth, float height, float speed,
          float direction, float uv_scale_factor = 4.0f);

  void addCarTemplate(std::shared_ptr<Model> model, float scale);
  void setPattern(TrafficPattern pattern) { m_pattern = pattern; }
  void prePopulate();

  void update(double delta_time) override;

private:
  void _spawnCar(std::optional<float> override_x = std::nullopt);
  float _calculateNextDelay();
};
