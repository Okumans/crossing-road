#pragma once

#include "graphics/material.hpp"
#include "row.hpp"

#include <memory>
#include <optional>
#include <vector>

class MapManager {
public:
  MapManager();

  void addRow(RowType type, const Material &material, float height = 0.0f,
              std::optional<Material> sideMaterial = std::nullopt);
  void update(double delta_time);
  void draw(Shader &shader);

  const std::vector<std::unique_ptr<Row>> &getRows() const { return m_rows; }

private:
  std::vector<std::unique_ptr<Row>> m_rows;
  float m_nextZ;
};
