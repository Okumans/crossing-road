#pragma once

#include "game/terrain.hpp"
#include "scene/car.hpp"
#include "utility/enum_map.hpp"
#include "utility/utility.hpp"

#include <cstdint>
#include <memory>
#include <utility>

class RoadTerrain : public Terrain {
public:
  enum class CarType : uint8_t { CAR_1, CAR_2, TRAIN_1 };

private:
  inline static const constexpr char *ROAD_1_TEX_NAME = "road_1";
  inline static const constexpr char *ROAD_2_TEX_NAME = "road_2";
  inline static const constexpr char *ROAD_3_TEX_NAME = "road_3";

  enum class RoadMaterialType : uint8_t {
    ROAD_1 = 0,
    ROAD_2,
    ROAD_3,
  };

  struct RoadMaterialConfig {
    const char *name;
    float uvScale;
  };

  inline static SettableNotInitialized<EnumMap<CarType, std::unique_ptr<Car>>,
                                       "s_carTemplate">
      s_carTemplate;
  inline constexpr static EnumMap<RoadMaterialType, RoadMaterialConfig>
      ROAD_MATERIAL_CONFIG = {{{ROAD_1_TEX_NAME, 4.0f},
                               {ROAD_2_TEX_NAME, 4.0f},
                               {ROAD_3_TEX_NAME, 1.0f}}};

public:
  RoadTerrain(uint32_t start_z) : Terrain(TerrainType::ROAD, start_z) {}

  virtual uint32_t _generateTerrain() override;

  static void setCar(CarType type, std::unique_ptr<Car> &&car) {
    s_carTemplate.set(type, std::move(car));
  }

  static void setup();
};
