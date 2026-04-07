#pragma once

#include "graphics/idrawable.hpp"
#include "graphics/shader.hpp"
#include "scene/object.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>

enum class RowType { GRASS, ROAD, WATER };

class Row : public IDrawable {
public:
  Row(float z_pos, RowType type, float depth = 1.0f, float height = 0.0f)
      : m_zPos(z_pos), m_type(type), m_depth(depth), m_height(height) {};

  Row(RowType type, float depth = 1.0f, float height = 0.0f)
      : m_zPos(0.0f), m_type(type), m_depth(depth),
        m_height(height) {}; // For set later

  virtual ~Row() {}

  virtual void update(double delta_time) {
    for (auto &obj : m_objects) {
      obj->update(delta_time);
    }
  }
  virtual void draw(const RenderContext &ctx) = 0;
  virtual void drawSidePanel(const RenderContext &ctx, float nextHeight,
                             bool isForward) = 0;

  virtual void addObject(std::unique_ptr<Object> object) {
    m_objects.push_back(std::move(object));
  }

  virtual float getDepth() const { return m_depth; }
  virtual float getZPos() const { return m_zPos; }
  virtual float getHeight() const { return m_height; }
  virtual RowType getType() const { return m_type; }

  virtual void setDepth(float depth) { m_depth = depth; }
  virtual void setZPos(float z_pos) { m_zPos = z_pos; }
  virtual void setHeight(float height) { m_height = height; }
  virtual void setType(RowType type) { m_type = type; }

protected:
  float m_zPos;
  float m_depth;
  float m_height;
  RowType m_type;

  // glm::vec2 m_uvOffset;
  // Material m_material;
  // Material m_sideMaterial;
  // std::unique_ptr<Mesh> m_mesh;
  // std::unique_ptr<Mesh> m_sideMesh; // Reusable side quad mesh
  std::vector<std::unique_ptr<Object>> m_objects;
  //
  // void _setupMesh();
};
