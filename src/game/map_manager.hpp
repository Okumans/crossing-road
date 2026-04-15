#pragma once

#include "game/terrain.hpp"
#include "graphics/idrawable.hpp"
#include "row.hpp"

#include <generator>
#include <memory>
#include <vector>

class MapManager : public IDrawable {
private:
  inline static const std::array DEFAULT_TERRAIN_WEIGHT = {5.0f, 8.0f, 2.0f,
                                                           111.5f};

  std::vector<std::unique_ptr<Terrain>> m_terrains;
  uint32_t m_playerRowIdx;

public:
  MapManager();

  void addTerrain(TerrainType type);
  void update(double delta_time);
  void draw(const RenderContext &ctx);

  void updatePlayerRowIdx(uint32_t idx) { m_playerRowIdx = idx; }

  const std::vector<std::unique_ptr<Terrain>> &getTerrain() const {
    return m_terrains;
  }

  std::generator<Row *> getRows() {
    for (const std::unique_ptr<Terrain> &terrain : m_terrains) {
      for (Row *row : terrain->getRows()) {
        co_yield row;
      }
    }
  }

  std::generator<const Row *const> getRows() const {
    for (const std::unique_ptr<Terrain> &terrain : m_terrains) {
      for (const Row *row : terrain->getRows()) {
        co_yield row;
      }
    }
  }

private:
  void _generateNext();
};
