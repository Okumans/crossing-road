#include "mesh.hpp"
#include "glad/gl.h"
#include "utility/texture_manager.hpp"
#include <cstdint>
#include <memory>
#include <utility>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
           std::vector<std::shared_ptr<Texture>> textures)
    : m_vertices(vertices), m_indices(indices), m_textures(textures) {
  _setupMesh();
}

Mesh::Mesh(std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices,
           std::vector<std::shared_ptr<Texture>> &&textures)
    : m_vertices(vertices), m_indices(indices), m_textures(textures) {
  _setupMesh();
}

Mesh::~Mesh() {
  if (m_vao)
    glDeleteVertexArrays(1, &m_vao);

  if (m_vbo)
    glDeleteBuffers(1, &m_vbo);

  if (m_ebo)
    glDeleteBuffers(1, &m_ebo);
}

Mesh::Mesh(Mesh &&other) noexcept
    : m_vertices(std::move(other.m_vertices)),
      m_indices(std::move(other.m_indices)), m_textures(other.m_textures),
      m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo) {
  other.m_vertices.clear();
  other.m_indices.clear();
  other.m_vao = 0;
  other.m_vbo = 0;
  other.m_ebo = 0;
}

void Mesh::_setupMesh() {
  glCreateVertexArrays(1, &m_vao);
  glCreateBuffers(1, &m_vbo);
  glCreateBuffers(1, &m_ebo);

  glNamedBufferStorage(m_vbo, m_vertices.size() * sizeof(Vertex),
                       m_vertices.data(), 0);
  glNamedBufferStorage(m_ebo, m_indices.size() * sizeof(uint32_t),
                       m_indices.data(), 0);

  glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));
  glVertexArrayElementBuffer(m_vao, m_ebo);

  // Position vec3
  glEnableVertexArrayAttrib(m_vao, 0);
  glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, position));
  glVertexArrayAttribBinding(m_vao, 0, 0);

  // Normal vec3
  glEnableVertexArrayAttrib(m_vao, 1);
  glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, normal));
  glVertexArrayAttribBinding(m_vao, 1, 0);

  // TexCoords vec2
  glEnableVertexArrayAttrib(m_vao, 2);
  glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, texCoords));
  glVertexArrayAttribBinding(m_vao, 2, 0);
}

void Mesh::draw(Shader &shader) {
  uint32_t diffuse_n = 1;
  uint32_t specular_n = 1;

  char name_buffer[47]; // 17 (fixed) + 8 (type_name) + 1 (_) + 20 (max i
                        // digits) + 1 (\0)

  for (size_t i = 0; i < m_textures.size(); ++i) {

    const char *type_name;
    uint32_t *counter;

    if (TextureType type = m_textures[i]->getType();
        type == TextureType::DIFFUSE) {
      type_name = "diffuse";
      counter = &diffuse_n;
    }

    else if (type == TextureType::SPECULAR) {
      type_name = "specular";
      counter = &specular_n;
    }

    else {
      // Unreachable
      std::unreachable();
    }

    std::snprintf(name_buffer, sizeof(name_buffer), "texture_%s_%u", type_name,
                  (*counter)++);

    shader.setInt(name_buffer, i);
    glBindTextureUnit(i, m_textures[i]->getTexID());
  }

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
}
