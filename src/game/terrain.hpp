#pragma once

#include "game/row.hpp"
#include "game/row_queue.hpp"
#include "graphics/idrawable.hpp"
#include "scene/object.hpp"
#include "utility/utility.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <ranges>
#include <vector>

class Terrain;

struct PlacementRule {
  int attempts = 1;
  float probability = 1.0f;
  float minScale = 1.0f;
  float maxScale = 1.0f;
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
  std::map<RowType, std::vector<SpawnConfig>> m_rules;

public:
  TerrainPopulator() = default;
  TerrainPopulator(TerrainPopulator &&) = default;
  TerrainPopulator &operator=(TerrainPopulator &&) = default;

  TerrainPopulator &withRule(RowType type, std::unique_ptr<RowObject> &&object,
                             PlacementRule rule = {}) {
    assert(rule.probability >= 0.0f && rule.probability <= 1.0f);
    assert(rule.minX <= rule.maxX);

    m_rules[type].push_back({.object = std::move(object), .rule = rule});
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
  TerrainPopulator m_populator;
  uint32_t m_startRowIdx;
  float m_currRelativeZ;

public:
  Terrain(uint32_t start_row)
      : m_startRowIdx(start_row), m_currRelativeZ(0.0f) {}

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

  float generate() {
    float offset = _generateTerrain();
    m_populator.populate(*this);
    return offset;
  }

  void bind(TerrainPopulator &&populator) {
    m_populator = std::move(populator);
  }

  const auto &getRowsInfo() const { return m_rowsInfo; }

  auto getRows() {
    return m_rowsInfo | std::views::transform(
                            [](const RowInfo &ri) { return ri.row.get(); });
  }

protected:
  // virtual const Row *_getRowBeforeTerrain() {
  //   return m_rowBeforeTerrainGetter(m_currZ);
  // };

  virtual uint32_t _generateTerrain() = 0;
};

inline void TerrainPopulator::populate(Terrain &terrain) {
  for (auto &[row, idx, z] : terrain.m_rowsInfo) {
    RowType type = row->getType();
    if (!m_rules.contains(type))
      continue;

    for (const SpawnConfig &config : m_rules.at(type)) {
      const PlacementRule &rule = config.rule;

      for (size_t i = 0; i < rule.attempts; ++i) {
        if (Random::randChance(rule.probability)) {

          // Clone is much cheaper than creating a new once
          // (because Bounding Box calculation)
          std::unique_ptr<RowObject> obj =
              std::make_unique<RowObject>(*config.object);

          float x = Random::randFloat(rule.minX, rule.maxX);
          float scale = Random::randFloat(rule.minScale, rule.maxScale);
          float z = rule.zOffset;
          float y = row->getHeight() + rule.yOffset;

          obj->setPosition({x, y});
          obj->setZOffset(z);
          obj->setScale(obj->getScale() * scale);

          if (rule.randomRotation) {
            obj->setRotation(
                {0.0f, Random::randFloat(0.0f, glm::two_pi<float>()), 0.0f});
          }

          row->addObject(std::move(obj));
        }
      }
    }
  }
}
