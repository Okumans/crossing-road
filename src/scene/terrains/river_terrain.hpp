#pragma once

#include "game/terrain.hpp"
#include "utility/not_initialized.hpp"

#include <cstdint>

class RiverTerrain : public Terrain {
private:
  inline static NotInitialized<TerrainPopulator, "s_terrainPopulator">
      s_terrainPopulator;

public:
  RiverTerrain(uint32_t start_z) : Terrain(TerrainType::RIVER, start_z) {
    _ensurePopulator();
  }

  virtual uint32_t generate() override {
    uint32_t latest_row_idx = _generateTerrain();
    s_terrainPopulator.ensureInitialized().populate(*this);
    return latest_row_idx;
  }

  virtual uint32_t generate(uint32_t row_numbers) override {
    uint32_t latest_row_idx = _generateTerrain(row_numbers);
    s_terrainPopulator.ensureInitialized().populate(*this);
    return latest_row_idx;
  }

  static void setup() { _ensurePopulator(); }

protected:
  virtual uint32_t _generateTerrain() override;
  virtual uint32_t _generateTerrain(uint32_t row_numbers) override;

private:
  static void _ensurePopulator();
};
