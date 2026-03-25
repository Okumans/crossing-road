#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "glm/fwd.hpp"
#include "shader.h"
#include "texture_manager.hpp"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
};

class Mesh {
private:
  // mesh data
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
  std::vector<std::shared_ptr<Texture>> m_textures;

  glm::vec3 m_baseColor;

  // render data
  GLuint m_vao, m_vbo, m_ebo;

public:
  Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
       std::vector<std::shared_ptr<Texture>> textures, glm::vec3 color);

  Mesh(std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices,
       std::vector<std::shared_ptr<Texture>> &&textures, glm::vec3 color);

  ~Mesh();

  // Delete all kind copy constrcutor for now
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(Mesh &&other) noexcept;

  const std::vector<Vertex> &getVertices() const { return m_vertices; }
  const std::vector<uint32_t> &getIndices() const { return m_indices; }
  const std::vector<std::shared_ptr<Texture>> &getTextures() const {
    return m_textures;
  }
  const glm::vec3 &getBaseColor() const { return m_baseColor; }

  void draw(Shader &shader);

private:
  void _setupMesh();
};
