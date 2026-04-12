#include "map_manager.hpp"
#include "game/row_queue.hpp"
#include "game/terrain.hpp"
#include "game/terrains/grass_terrain.hpp"
#include "game/terrains/hill_terrain.hpp"
#include "game/terrains/road_terrain.hpp"
#include "graphics/idrawable.hpp"
#include "utility/utility.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

MapManager::MapManager() : m_playerRowIdx(0) { RowQueue::init(19, 3.0f); }

void MapManager::addTerrain(TerrainType type) {
  std::unique_ptr<Terrain> terrain = nullptr;

  uint32_t curr_row_idx = RowQueue::get().getCurrRowIdx();

  assert(type != TerrainType::Count);

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
  case TerrainType::Count:
    std::unreachable();
  }

  assert(terrain);

  terrain->generate();
  m_terrains.push_back(std::move(terrain));
}

void MapManager::update(double delta_time) {
  for (auto &terrain : m_terrains) {
    terrain->update(delta_time);
  }

  const RowQueue &row_queue = RowQueue::get();

  assert(row_queue.getCurrRowIdx() == 0 ||
         m_playerRowIdx < row_queue.getCurrRowIdx());

  while (row_queue.getCurrRowIdx() == 0 ||
         (row_queue.getCurrRowIdx() - m_playerRowIdx) <
             row_queue.getCapacity()) {
    _generateNext();
  }

  const uint32_t minimum_buffer = 4;
  if (m_playerRowIdx - row_queue.getFirstStoredRowIdx() > minimum_buffer) {
    RowQueue::get().step((m_playerRowIdx - row_queue.getFirstStoredRowIdx()) -
                         minimum_buffer);
  }

  std::erase_if(m_terrains, [&row_queue](const auto &terrain) {
    const auto &rowsInfo = terrain->getRowsInfo();
    if (rowsInfo.empty())
      return true;
    return rowsInfo.back().rowIdx < row_queue.getFirstStoredRowIdx();
  });
}

void MapManager::_generateNext() {
  if (m_terrains.empty()) {
    addTerrain(TerrainType::GRASSY);
    addTerrain(TerrainType::GRASSY);
    return;
  }

  std::array weights = {5.0f, 8.0f, 2.0f};

  static_assert(static_cast<size_t>(TerrainType::Count) > 0,
                "At least 1 terrain need to exists");
  static_assert(weights.size() == static_cast<size_t>(TerrainType::Count),
                "Weights for all terrain type need to be specify");

  // Lower the changes same terrain generate next to each other
  weights[static_cast<size_t>(m_terrains.back()->getType())] = 0.1f;

  TerrainType choosed_type =
      static_cast<TerrainType>(Random::randWeighted<size_t>(
          0, static_cast<size_t>(TerrainType::Count) - 1, weights));

  addTerrain(choosed_type);
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
