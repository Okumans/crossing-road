#include "map_manager.hpp"
#include "game/terrain.hpp"
#include "game/terrains/grass_terrain.hpp"
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
  for (const std::unique_ptr<Terrain> &terrain : m_terrains) {
    terrain->draw(ctx);
  }

  // make sure side pandel is draw
  // TODO: embbeded this logic into terrain draw method instead
  float prev_height = 0.0f;
  for (const auto [index, row] : std::views::enumerate(getRows())) {
    float curr_height = row->getHeight();

    if (index > 0) {
      if (curr_height > prev_height) {
        row->drawSidePanel(ctx, prev_height, true);
      }
    } else {
      // First row, draw side panel to ground level if it's above 0
      if (curr_height > 0.0f) {
        row->drawSidePanel(ctx, 0.0f, true);
      }
    }
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
