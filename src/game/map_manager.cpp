#include "map_manager.hpp"
#include "graphics/idrawable.hpp"

MapManager::MapManager() : m_nextZ(3.5f) {}

void MapManager::addRow(std::unique_ptr<Row> &&row) {
  row->setZPos(m_nextZ);
  m_nextZ -= row->getDepth();
  m_rows.push_back(std::move(row));
}

void MapManager::update(double delta_time) {
  for (auto &row : m_rows) {
    row->update(delta_time);
  }
}

void MapManager::draw(const RenderContext &ctx) {
  for (size_t i = 0; i < m_rows.size(); ++i) {
    m_rows[i]->draw(ctx);

    // Check neighbors for side panels
    float currentHeight = m_rows[i]->getHeight();

    // Check forward neighbor (i-1)
    if (i > 0) {
      float neighborHeight = m_rows[i - 1]->getHeight();
      if (currentHeight > neighborHeight) {
        m_rows[i]->drawSidePanel(ctx, neighborHeight, true);
      }
    } else {
      // First row, draw side panel to ground level if it's above 0
      if (currentHeight > 0.0f) {
        m_rows[i]->drawSidePanel(ctx, 0.0f, true);
      }
    }

    // Check backward neighbor (i+1)
    if (i < m_rows.size() - 1) {
      float neighborHeight = m_rows[i + 1]->getHeight();
      if (currentHeight > neighborHeight) {
        m_rows[i]->drawSidePanel(ctx, neighborHeight, false);
      }
    } else {
      // Last row, draw side panel to ground level if it's above 0
      if (currentHeight > 0.0f) {
        m_rows[i]->drawSidePanel(ctx, 0.0f, false);
      }
    }
  }
}
