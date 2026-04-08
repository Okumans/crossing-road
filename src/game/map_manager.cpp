#include "map_manager.hpp"
#include "game/terrain.hpp"
#include "game/terrains/grass_terrain.hpp"
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

  // for (const auto [index, row] : std::views::enumerate(getRows())) {
  //   m_terrains[i]->draw(ctx);

  // Check neighbors for side panels
  // float currentHeight = m_terrains[i]->getHeight();
  //
  // // Check forward neighbor (i-1)
  // if (i > 0) {
  //   float neighborHeight = m_terrains[i - 1]->getHeight();
  //   if (currentHeight > neighborHeight) {
  //     m_terrains[i]->drawSidePanel(ctx, neighborHeight, true);
  //   }
  // } else {
  //   // First row, draw side panel to ground level if it's above 0
  //   if (currentHeight > 0.0f) {
  //     m_terrains[i]->drawSidePanel(ctx, 0.0f, true);
  //   }
  // }
  //
  // // Check backward neighbor (i+1)
  // if (i < m_terrains.size() - 1) {
  //   float neighborHeight = m_terrains[i + 1]->getHeight();
  //   if (currentHeight > neighborHeight) {
  //     m_terrains[i]->drawSidePanel(ctx, neighborHeight, false);
  //   }
  // } else {
  //   // Last row, draw side panel to ground level if it's above 0
  //   if (currentHeight > 0.0f) {
  //     m_terrains[i]->drawSidePanel(ctx, 0.0f, false);
  //   }
  // }
  // }
}

const Row *const MapManager::getTerrainLastRowBefore(float curr_z) {
  auto it = std::ranges::lower_bound(m_terrains, curr_z, std::less<>(),
                                     &Terrain::getZPos);

  if (it != m_terrains.begin()) {
    return std::prev(it)->get()->getRows().back().get();
  }

  return nullptr;
}
