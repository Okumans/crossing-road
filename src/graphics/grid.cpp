#include "grid.hpp"
#include <vector>

Grid::Grid() { _setupGrid(); }

Grid::~Grid() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

void Grid::_setupGrid() {
  // Large quad vertices on y = 0
  float size = 500.0f;
  std::vector<float> vertices = {
      -size, 0.0f, -size, size, 0.0f, -size, size,  0.0f, size,

      -size, 0.0f, -size, size, 0.0f, size,  -size, 0.0f, size};

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  glBindVertexArray(0);
}

void Grid::draw(const Camera &camera, Shader &shader) {
  shader.use();
  shader.setMat4("view", camera.getViewMatrix());
  shader.setMat4("projection", camera.getProjectionMatrix());
  shader.setVec3("cameraPos", camera.Position);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glDisable(GL_BLEND);
}
