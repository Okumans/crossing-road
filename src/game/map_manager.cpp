#include "map_manager.hpp"
#include "game/terrain.hpp"
#include "game/terrains/grass_terrain.hpp"
#include "game/terrains/hill_terrain.hpp"
#include "game/terrains/road_terrain.hpp"
#include "graphics/idrawable.hpp"
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>

MapManager::MapManager() : m_currZ(3.5f) {}

void MapManager::addTerrain(TerrainType type) {
  std::unique_ptr<Terrain> terrain = nullptr;

  switch (type) {
  case TerrainType::GRASSY:
    terrain = std::make_unique<GrassyTerrain>(
        [this](float before) { return getTerrainLastRowBefore(before); },
        m_currZ);
    break;
  case TerrainType::ROAD:
    terrain = std::make_unique<RoadTerrain>(
        [this](float before) { return getTerrainLastRowBefore(before); },
        m_currZ);
    break;
  case TerrainType::HILLY:
    terrain = std::make_unique<HillTerrain>(
        [this](float before) { return getTerrainLastRowBefore(before); },
        m_currZ);
    break;
  }

  assert(terrain);

  terrain->setZPos(m_currZ);
  m_currZ = terrain->generate();

  m_terrains.push_back(std::move(terrain));
}

void MapManager::update(double delta_time) {
  for (auto &terrain : m_terrains) {
    terrain->update(delta_time);
  }
}

void MapManager::draw(const RenderContext &ctx) {
  Row *prevRow = nullptr;

  for (Row *row : getRows()) {
    row->draw(ctx);

    float currentHeight = row->getHeight();
    float prevHeight = prevRow ? prevRow->getHeight() : 0.0f;

    // Side panel towards previous row
    if (currentHeight > prevHeight) {
      row->drawSidePanel(ctx, prevHeight, true);
    }

    // Previous row side panel towards current row
    if (prevRow && prevRow->getHeight() > currentHeight) {
      prevRow->drawSidePanel(ctx, currentHeight, false);
    }

    prevRow = row;
  }

  // Final row side panel to ground level
  if (prevRow && prevRow->getHeight() > 0.0f) {
    prevRow->drawSidePanel(ctx, 0.0f, false);
  }
}

const Row *const MapManager::getTerrainLastRowBefore(float curr_z) {
  auto it = std::ranges::lower_bound(m_terrains, curr_z, std::less<>(),
                                     &Terrain::getZPos);

  if (it != m_terrains.begin()) {
    return std::prev(it)->get()->getRows().back().get();
  }

  return nullptr;
}
