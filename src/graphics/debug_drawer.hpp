#pragma once

#include "graphics/idrawable.hpp"
#include "scene/row_object.hpp"

class DebugDrawer {
public:
  static void drawAABB(const RenderContext &ctx, const AABB &aabb,
                       const glm::vec3 &color = {0.0f, 1.0f, 0.0f});

private:
  static void _init();
  static GLuint m_vao, m_vbo, m_ebo;
  static bool m_initialized;
};
