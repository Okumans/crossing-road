#include "map_manager.hpp"

MapManager::MapManager() : m_nextZ(0.5f) {}

void MapManager::addRow(RowType type) {
  m_rows.push_back(std::make_unique<Row>(m_nextZ, type));
  m_nextZ -= 0.5f;
}

void MapManager::draw(Shader &shader) {
  for (auto &row : m_rows) {
    row->draw(shader);
  }
}
