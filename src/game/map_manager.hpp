#pragma once

#include "row.hpp"

#include <memory>
#include <vector>

class MapManager {
public:
    MapManager();

    void addRow(RowType type, const std::vector<std::shared_ptr<Texture>>& textures);
    void draw(Shader& shader);


  const std::vector<std::unique_ptr<Row>> &getRows() const { return m_rows; }

private:
  std::vector<std::unique_ptr<Row>> m_rows;
  float m_nextZ;
};
