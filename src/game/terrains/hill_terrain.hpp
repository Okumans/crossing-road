#pragma once

#include "game/terrain.hpp"
#include "utility/not_initialized.hpp"

#include <cstdint>

class HillTerrain : public Terrain {
private:
  inline static const char *GRASS_1_TEX_NAME = "grass_1";
  inline static const char *GRASS_2_TEX_NAME = "grass_2";
  inline static const char *GRASS_SIDE_TEX_NAME = "road_4";

  inline static NotInitialized<TerrainPopulator, "s_terrainPopulator">
      s_terrainPopulator;

public:
  HillTerrain(uint32_t start_z) : Terrain(TerrainType::HILLY, start_z) {
    _ensurePopulator();
  }

  virtual uint32_t generate() override {
    uint32_t latest_row_idx = _generateTerrain();
    s_terrainPopulator.ensureInitialized().populate(*this);
    return latest_row_idx;
  }

  static void setup() { _ensurePopulator(); }

private:
  virtual uint32_t _generateTerrain() override;
  static void _ensurePopulator();
};
