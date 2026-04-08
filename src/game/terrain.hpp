#pragma once

#include "game/row.hpp"
#include "graphics/idrawable.hpp"
#include "scene/object.hpp"
#include "utility/utility.hpp"

#include <functional>
#include <map>
#include <memory>
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
  std::unique_ptr<Object> object;
  PlacementRule rule;
};

class TerrainPopulator {
private:
  std::map<RowType, std::vector<SpawnConfig>> m_rules;

public:
  TerrainPopulator() = default;
  TerrainPopulator(TerrainPopulator &&) = default;
  TerrainPopulator &operator=(TerrainPopulator &&) = default;

  TerrainPopulator &withRule(RowType type, std::unique_ptr<Object> &&object,
                             PlacementRule rule = {}) {
    assert(rule.probability >= 0.0f && rule.probability <= 1.0f);
    assert(rule.minX <= rule.maxX);

    m_rules[type].push_back({.object = std::move(object), .rule = rule});
    return *this;
  }

  void populate(Terrain &terrain);
};

class Terrain : public IDrawable {
  friend class TerrainPopulator;

protected:
  std::vector<std::unique_ptr<Row>> m_rows;
  float m_currZ;
  TerrainPopulator m_populator;
  std::function<const Row *(float)> m_rowBeforeTerrainGetter;

public:
  Terrain(std::function<const Row *(float)> before_row_getter, float curr_z)
      : m_currZ(curr_z),
        m_rowBeforeTerrainGetter(std::move(before_row_getter)) {}

  virtual ~Terrain() = default;

  virtual void update(float delta_time) {
    for (auto &row : m_rows)
      row->update(delta_time);
  }

  virtual void draw(const RenderContext &ctx) {
    for (auto &row : m_rows)
      row->draw(ctx);
  }

  float generate() {
    float offset = _generateTerrain();
    m_populator.populate(*this);
    return offset;
  }

  void bind(TerrainPopulator &&populator) {
    m_populator = std::move(populator);
  }

  void setZPos(float curr_z) { m_currZ = curr_z; }
  float getZPos() const { return m_currZ; }
  const std::vector<std::unique_ptr<Row>> &getRows() const { return m_rows; }

protected:
  virtual const Row *_getRowBeforeTerrain() {
    return m_rowBeforeTerrainGetter(m_currZ);
  };

  virtual float _generateTerrain() = 0;
};

inline void TerrainPopulator::populate(Terrain &terrain) {
  for (auto &rowPtr : terrain.m_rows) {
    RowType type = rowPtr->getType();
    if (!m_rules.contains(type))
      continue;

    for (const SpawnConfig &config : m_rules.at(type)) {
      const PlacementRule &rule = config.rule;

      for (size_t i = 0; i < rule.attempts; ++i) {
        if (Random::randChance(rule.probability)) {

          // Clone is much cheaper than creating a new once
          // (because Bounding Box calculation)
          std::unique_ptr<Object> obj =
              std::make_unique<Object>(*config.object);

          float x = Random::randFloat(rule.minX, rule.maxX);
          float scale = Random::randFloat(rule.minScale, rule.maxScale);
          float z =
              rowPtr->getZPos() - (rowPtr->getDepth() - 0.25f) + rule.zOffset;
          float y = rowPtr->getHeight() + rule.yOffset;

          obj->setPosition({x, y, z});
          obj->setScale(obj->getScale() * scale);

          if (rule.randomRotation) {
            obj->setRotation(
                {0.0f, Random::randFloat(0.0f, glm::two_pi<float>()), 0.0f});
          }

          rowPtr->addObject(std::move(obj));
        }
      }
    }
  }
}
