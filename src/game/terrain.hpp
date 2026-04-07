#pragma once

#include "game/row.hpp"

#include <memory>
#include <vector>

class Terrain : public IDrawable {
protected:
  std::vector<std::unique_ptr<Row>> m_rows;

public:
  virtual void draw(Shader &shader) = 0;
  virtual void generate() = 0;
};
