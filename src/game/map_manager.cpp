#include "map_manager.hpp"
#include "game/row_queue.hpp"
#include "game/terrain.hpp"
#include "game/terrains/grass_terrain.hpp"
#include "game/terrains/hill_terrain.hpp"
#include "game/terrains/road_terrain.hpp"
#include "graphics/idrawable.hpp"

#include <cstdint>
#include <memory>

MapManager::MapManager() { RowQueue::init(19, 3.0f); }

void MapManager::addTerrain(TerrainType type) {
  std::unique_ptr<Terrain> terrain = nullptr;

  uint32_t curr_row_idx = RowQueue::get().getCurrRowIdx();

  switch (type) {
  case TerrainType::GRASSY:
    terrain = std::make_unique<GrassyTerrain>(curr_row_idx);
    break;
  case TerrainType::ROAD:
    terrain = std::make_unique<RoadTerrain>(curr_row_idx);
    break;
  case TerrainType::HILLY:
    terrain = std::make_unique<HillTerrain>(curr_row_idx);
    break;
  }

  assert(terrain);

  terrain->generate();
  m_terrains.push_back(std::move(terrain));
}

void MapManager::update(double delta_time) {
  for (auto &terrain : m_terrains) {
    terrain->update(delta_time);
  }
}

void MapManager::draw(const RenderContext &ctx) {
  Row *prevRow = nullptr;
  float prevRenderZ = 0.0f;

  for (auto [z, row] : RowQueue::get().getRowsWithZ()) {
    row->draw(ctx, z);

    float currentHeight = row->getHeight();
    float prevHeight = prevRow ? prevRow->getHeight() : 0.0f;

    // Side panel towards previous row
    if (currentHeight > prevHeight) {
      row->drawSidePanel(ctx, z, prevHeight, true);
    }

    // Previous row side panel towards current row
    if (prevRow && prevRow->getHeight() > currentHeight) {
      prevRow->drawSidePanel(ctx, prevRenderZ, currentHeight, false);
    }

    prevRow = row;
    prevRenderZ = z;
  }

  // Final row side panel to ground level
  if (prevRow && prevRow->getHeight() > 0.0f) {
    prevRow->drawSidePanel(ctx, prevRenderZ, 0.0f, false);
  }
}
