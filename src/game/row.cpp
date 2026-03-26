#include "row.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

Row::Row(float zPos, RowType type) : m_zPos(zPos), m_type(type) {
  _setupMesh();
}

Row::~Row() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

void Row::_setupMesh() {
  float width = 20.0f;
  float depth = 0.5f;
  float halfWidth = width / 2.0f;
  float uMax = width / 2.0f;
  float vMax = depth / 2.0f;

  std::vector<float> vertices = {// Position (x, y, z)          // UV (u, v)
                                 -halfWidth, 0.0f, m_zPos,         0.0f, 0.0f,
                                 halfWidth,  0.0f, m_zPos,         uMax, 0.0f,
                                 halfWidth,  0.0f, m_zPos - depth, uMax, vMax,

                                 -halfWidth, 0.0f, m_zPos,         0.0f, 0.0f,
                                 halfWidth,  0.0f, m_zPos - depth, uMax, vMax,
                                 -halfWidth, 0.0f, m_zPos - depth, 0.0f, vMax};

  glCreateVertexArrays(1, &m_vao);
  glCreateBuffers(1, &m_vbo);

  glNamedBufferStorage(m_vbo, vertices.size() * sizeof(float), vertices.data(),
                       0);

  glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, 5 * sizeof(float));

  // Position
  glEnableVertexArrayAttrib(m_vao, 0);
  glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(m_vao, 0, 0); // Bind to Binding Point 0

  // TexCoords
  glEnableVertexArrayAttrib(m_vao, 2);
  glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
  glVertexArrayAttribBinding(m_vao, 2, 0); // Bind to Binding Point 0
}

void Row::draw(Shader &shader) {
  auto tex = _getTexture();
  if (tex) {
    glBindTextureUnit(0, tex->getTexID());
    shader.setInt("u_DiffuseTex0", 0);
    shader.setVec3("u_BaseColor0", glm::vec3(1.0f));
  }

  shader.setMat4("u_Model", glm::mat4(1.0f));

  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

std::shared_ptr<Texture> Row::_getTexture() {
  switch (m_type) {
  case RowType::GRASS:
    return TextureManager::getTexture(TextureName("grass"));
  case RowType::ROAD:
    return TextureManager::getTexture(TextureName("road"));
  case RowType::WATER:
    return TextureManager::getTexture(TextureName("water"));
  }
  return nullptr;
}
