#pragma once

#include "graphics/idrawable.hpp"
#include "graphics/izdrawable.hpp"
#include "scene/row_object.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>

enum class RowType { GRASS, ROAD, WATER };

class Row : public IZDrawable {
public:
  inline static const float WIDTH = 22.0f;
  inline static const float SLOT_WIDTH = 1.0f;

protected:
  float m_depth;
  float m_height;
  RowType m_type;
  std::vector<std::unique_ptr<RowObject>> m_objects;

public:
  Row(RowType type, float depth = 1.0f, float height = 0.0f)
      : m_type(type), m_depth(depth), m_height(height) {}; // For set later

  virtual ~Row() {}

  virtual void update(double delta_time) {
    for (auto it = m_objects.begin(); it != m_objects.end();) {
      (*it)->update(delta_time);

      // Check if object should be removed (e.g. left the screen)
      // For now, we'll let the RoadRow handle its own objects,
      // but we need a way to mark them for deletion or just erase here.
      // Let's use a simple bounds check based on WIDTH.
      const AABB &worldAABB = (*it)->getWorldAABB();
      float halfWidth = WIDTH / 2.0f + 2.0f; // Padding

      if (worldAABB.min.x > halfWidth || worldAABB.max.x < -halfWidth) {
        it = m_objects.erase(it);
      } else {
        ++it;
      }
    }
  }

  virtual void draw(const RenderContext &ctx, float z) override = 0;

  virtual void drawSidePanel(const RenderContext &ctx, float z,
                             float next_height, bool is_forward) = 0;

  virtual void addObject(std::unique_ptr<RowObject> object) {
    m_objects.push_back(std::move(object));
  }

  virtual bool collided(const RowObject &target) const {
    for (const std::unique_ptr<RowObject> &object : m_objects) {
      if (object->collided(target))
        return true;
    }

    return false;
  }

  virtual float getDepth() const { return m_depth; }
  virtual float getHeight() const { return m_height; }
  virtual RowType getType() const { return m_type; }

  const std::vector<std::unique_ptr<RowObject>> &getObjects() const {
    return m_objects;
  }

  virtual void setDepth(float depth) { m_depth = depth; }
  virtual void setHeight(float height) { m_height = height; }
  virtual void setType(RowType type) { m_type = type; }
};
