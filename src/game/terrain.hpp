#pragma once

#include "game/row.hpp"
#include "game/row_queue.hpp"
#include "glm/fwd.hpp"
#include "graphics/idrawable.hpp"
#include "scene/row_object.hpp"
#include "utility/enum_map.hpp"
#include "utility/random.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <ranges>
#include <vector>

class Terrain;

enum class TerrainType {
  GRASSY,
  ROAD,
  HILLY,
};

struct PlacementRule {
  int attempts = 1;
  float probability = 1.0f;
  float scale = 1.0f;
  float minX = -Row::WIDTH / 2.0f;
  float maxX = Row::WIDTH / 2.0f;
  float yOffset = 0.0f;
  float zOffset = 0.0f;
  bool randomRotation = true;
};

struct SpawnConfig {
  std::unique_ptr<RowObject> object;
  PlacementRule rule;
};

struct PopulatorSettings {
  float minEdgeWeight = 0.4f;
  float maxEdgeWeight = 1.0f;
  float gapMarginStart = 0.2f; // Gap won't spawn in the first 20% of the road
  float gapMarginEnd = 0.8f;   // Gap won't spawn in the last 20% of the road
};

class TerrainPopulator {
private:
  EnumMap<RowType, std::optional<std::vector<SpawnConfig>>> m_rules;
  PopulatorSettings m_settings;

public:
  TerrainPopulator() = default;
  TerrainPopulator(PopulatorSettings settings) : m_settings(settings) {}
  TerrainPopulator(TerrainPopulator &&) = default;
  TerrainPopulator &operator=(TerrainPopulator &&) = default;

  /**
   * @brief Registers a spawning rule for a specific row type.
   *
   * This method adds an object template and its associated placement rules to
   * the populator. The provided object's scale is adjusted based on the rule,
   * and its AABB is recalculated.
   *
   * @note It is recommended that the passed object is in a deferred state (AABB
   * not yet calculated) as it will be recalculated here after scaling.
   *
   * @param type The row type to apply this rule to.
   * @param object The template object to spawn.
   * @param rule The placement rules (probability, scale, bounds, etc.).
   * @return TerrainPopulator& Reference to this for method chaining.
   */
  TerrainPopulator &withRule(RowType type, std::unique_ptr<RowObject> &&object,
                             PlacementRule rule = {}) {

    assert(rule.probability >= 0.0f && rule.probability <= 1.0f);
    assert(rule.minX <= rule.maxX);

    std::optional<std::vector<SpawnConfig>> &opt = m_rules[type];

    if (!opt)
      opt.emplace();

    object->setScale(object->getScale() * rule.scale);
    object->recalculateAABB();

    opt->push_back({.object = std::move(object), .rule = rule});

    return *this;
  }

  void populate(Terrain &terrain);
};

struct RowInfo {
  std::unique_ptr<Row> row;
  uint32_t rowIdx;
  float relativeZ;
};

class Terrain : public IZDrawable {
  friend class TerrainPopulator;

private:
  std::vector<RowInfo> m_rowsInfo;

protected:
  TerrainType m_type;
  uint32_t m_startRowIdx;
  float m_currRelativeZ;

public:
  Terrain(TerrainType type, uint32_t start_row)
      : m_type(type), m_startRowIdx(start_row) {}

  virtual ~Terrain() = default;

  virtual uint32_t addRow(std::unique_ptr<Row> &&row) {
    uint32_t row_idx = RowQueue::get().registerRow(row.get());
    float row_depth = row->getDepth();

    m_rowsInfo.push_back({.row = std::move(row), .rowIdx = row_idx});

    return row_idx;
  }

  virtual void update(float delta_time) {
    for (auto &[row, idx, z] : m_rowsInfo)
      row->update(delta_time);
  }

  virtual void draw(const RenderContext &ctx, float z) {
    for (auto &[row, idx, local_z] : m_rowsInfo)
      row->draw(ctx, local_z + z);
  }

  virtual uint32_t generate() {
    uint32_t lastest_row_idx = _generateTerrain();
    return lastest_row_idx;
  }

  TerrainType getType() const { return m_type; }
  const auto &getRowsInfo() const { return m_rowsInfo; }
  auto getRows() {
    return m_rowsInfo | std::views::transform(
                            [](const RowInfo &ri) { return ri.row.get(); });
  }

protected:
  virtual uint32_t _generateTerrain() = 0;
};

inline void TerrainPopulator::populate(Terrain &terrain) {
  for (auto &[row, idx, z] : terrain.m_rowsInfo) {
    RowType type = row->getType();

    const std::optional<std::vector<SpawnConfig>> &spawn_opt = m_rules[type];
    if (!spawn_opt.has_value())
      continue;

    const std::vector<SpawnConfig> &rules = m_rules[type].value();
    const uint32_t total_slots = row->SLOT_AMOUNT;

    assert(total_slots > 0);

    const float center_index = (total_slots - 1) / 2.0f;

    // Calculate gap range based on percentages in m_settings
    int gap_min = static_cast<int>(total_slots * m_settings.gapMarginStart);
    int gap_max = static_cast<int>(total_slots * m_settings.gapMarginEnd);
    int guaranteed_gap = Random::randInt(gap_min, gap_max);

    for (uint32_t i = 0; i < row->SLOT_AMOUNT; ++i) {
      if (static_cast<int>(i) == guaranteed_gap)
        continue;

      float dist_from_center = std::abs(static_cast<float>(i) - 12) / 12.0f;

      float weight_range = m_settings.maxEdgeWeight - m_settings.minEdgeWeight;
      float edge_weight =
          m_settings.minEdgeWeight + (weight_range * dist_from_center);

      for (const auto &config : rules) {
        if (Random::randChance(config.rule.probability * edge_weight)) {

          // Clone object (skip AABB recalculation)
          std::unique_ptr<RowObject> obj =
              std::make_unique<RowObject>(*config.object);

          // scale is already set, with recalculateAABB on withRule
          float x = static_cast<float>(i) - center_index;
          float y = row->getHeight() + config.rule.yOffset;

          obj->setPosition({x, y});
          obj->setZOffset(config.rule.zOffset);

          if (config.rule.randomRotation) {
            obj->setRotationY(Random::randFloat(0.0f, 360.0f));
          }

          if (!row->collided(*obj)) {
            row->addObject(std::move(obj));
            break;
          }
        }
      }
    }
  }
}
