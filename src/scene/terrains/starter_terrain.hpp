#pragma once

#include "game/terrain.hpp"

#include <cstdint>

class StarterTerrain : public Terrain {
private:
  inline static const char *ROAD_1_TEX_NAME = "road_5";

public:
  StarterTerrain(uint32_t startRowidx)
      : Terrain(TerrainType::STARTER, startRowidx) {}

  virtual uint32_t generate() override {
    uint32_t latest_row_idx = _generateTerrain();
    return latest_row_idx;
  }

private:
  virtual uint32_t _generateTerrain() override;
  virtual uint32_t _generateTerrain(uint32_t row_numbers) override;
};
