#include "map_manager.hpp"
#include "external/magic_enum.hpp"
#include "game/row_queue.hpp"
#include "game/terrain.hpp"
#include "game/terrains/grass_terrain.hpp"
#include "game/terrains/hill_terrain.hpp"
#include "game/terrains/river_terrain.hpp"
#include "game/terrains/road_terrain.hpp"
#include "graphics/idrawable.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

MapManager::MapManager() : m_playerRowIdx(0) { RowQueue::init(19, 0.0f); }

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
  case TerrainType::RIVER:
    terrain = std::make_unique<RiverTerrain>(curr_row_idx);
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

  std::array weights = DEFAULT_TERRAIN_WEIGHT;

  static_assert(magic_enum::enum_count<TerrainType>() > 0,
                "At least 1 terrain need to exists");
  static_assert(weights.size() == magic_enum::enum_count<TerrainType>(),
                "Weights for all terrain type need to be specify");

  // Lower the changes same terrain generate next to each other
  weights[static_cast<size_t>(m_terrains.back()->getType())] = 0.1f;

  TerrainType choosed_type =
      static_cast<TerrainType>(Random::randWeighted<size_t>(
          0, magic_enum::enum_count<TerrainType>() - 1, weights));

  addTerrain(choosed_type);
}

void MapManager::draw(const RenderContext &ctx) {
  Row *prev_row = nullptr;
  float prev_render_z = 0.0f;

  for (const auto &[z, row] : RowQueue::get().getRowsWithZ()) {
    row->draw(ctx, z);

    float current_height = row->getHeight();
    float prev_height = prev_row ? prev_row->getHeight() : 0.0f;

    // Side panel towards previous row
    if (current_height > prev_height) {
      row->drawSidePanel(ctx, z, prev_height, true);
    }

    // Previous row side panel towards current row
    if (prev_row && prev_row->getHeight() > current_height) {
      prev_row->drawSidePanel(ctx, prev_render_z, current_height, false);
    }

    prev_row = row;
    prev_render_z = z;
  }

  // Final row side panel to ground level
  if (prev_row && prev_row->getHeight() > 0.0f) {
    prev_row->drawSidePanel(ctx, prev_render_z, 0.0f, false);
  }
}
