#pragma once

#include "game/row.hpp"
#include "game/row_queue.hpp"
#include "glm/fwd.hpp"
#include "graphics/idrawable.hpp"
#include "scene/row_object.hpp"
#include "utility/enum_map.hpp"
#include "utility/utility.hpp"

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
  Count,
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

class TerrainPopulator {
private:
  EnumMap<RowType, std::optional<std::vector<SpawnConfig>>> m_rules;

public:
  TerrainPopulator() = default;
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
      : m_type(type), m_startRowIdx(start_row), m_currRelativeZ(0.0f) {}

  virtual ~Terrain() = default;

  virtual uint32_t addRow(std::unique_ptr<Row> &&row) {
    uint32_t row_idx = RowQueue::get().registerRow(row.get());
    float row_depth = row->getDepth();

    m_rowsInfo.push_back({.row = std::move(row),
                          .rowIdx = row_idx,
                          .relativeZ = m_currRelativeZ});

    m_currRelativeZ -= row_depth;

    return row_idx;
  }

  virtual void update(float delta_time) {
    for (auto &[row, idx, z] : m_rowsInfo)
      row->update(delta_time);
  }

  virtual void draw(const RenderContext &ctx, float z) {
    for (auto &[row, idx, z] : m_rowsInfo)
      row->draw(ctx, z);
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
    if (!m_rules[type])
      continue;

    const auto &rules = m_rules[type].value();

    // Grid-based spawning (25 slots)
    // Randomly pick one slot in the playable area to definitely be a gap
    int guaranteed_gap = Random::randInt(5, 19);

    for (int i = 0; i < 25; ++i) {
      if (i == guaranteed_gap)
        continue;

      // Weight probability to be higher at edges (0.0 to 12.0 distance)
      float dist_from_center = std::abs(i - 12) / 12.0f;
      float edge_weight =
          0.4f + 0.6f * dist_from_center; // 0.4 center, 1.0 edges

      for (const auto &config : rules) {
        if (Random::randChance(config.rule.probability * edge_weight)) {

          // Clone object (skip AABB recalculation)
          std::unique_ptr<RowObject> obj =
              std::make_unique<RowObject>(*config.object);

          // scale is already set, with recalculateAABB on withRule
          float x = -12.0f + (float)i;
          float z = config.rule.zOffset;
          float y = row->getHeight() + config.rule.yOffset;

          obj->setPosition({x, y});
          obj->setZOffset(z);

          if (config.rule.randomRotation) {
            obj->setRotationY(Random::randFloat(0.0f, glm::two_pi<float>()));
          }

          if (!row->collided(*obj)) {
            row->addObject(std::move(obj));
            break; // One object per slot
          }
        }
      }
    }
  }
}
