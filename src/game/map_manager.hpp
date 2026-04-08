#pragma once

#include "game/terrain.hpp"
#include "graphics/idrawable.hpp"
#include "row.hpp"

#include <generator>
#include <memory>
#include <vector>

enum class TerrainType {
  GRASSY,
  ROAD,
};

class MapManager : public IDrawable {
private:
  std::vector<std::unique_ptr<Terrain>> m_terrains;
  float m_currZ;

public:
  MapManager();

  void addTerrain(TerrainType type);
  void update(double delta_time);
  void draw(const RenderContext &ctx);

  const std::vector<std::unique_ptr<Terrain>> &getTerrain() const {
    return m_terrains;
  }

  std::generator<Row *> getRows() {
    for (const std::unique_ptr<Terrain> &terrain : m_terrains) {
      for (const std::unique_ptr<Row> &row : terrain->getRows()) {
        co_yield row.get();
      }
    }
  }

  std::generator<const Row *const> getRows() const {
    for (const std::unique_ptr<Terrain> &terrain : m_terrains) {
      for (const std::unique_ptr<Row> &row : terrain->getRows()) {
        co_yield row.get();
      }
    }
  }

  const Row *const getTerrainLastRowBefore(float curr_z);
};
