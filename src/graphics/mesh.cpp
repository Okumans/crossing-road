#include "mesh.hpp"
#include "graphics/material.hpp"

#include <glad/gl.h>

#include <cstdint>
#include <utility>

Mesh::Mesh(std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices,
           Material material, glm::vec3 color, float opacity)
    : m_vertices(std::move(vertices)), m_indices(std::move(indices)),
      m_material(material), m_baseColor(color), m_opacity(opacity) {
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
      m_indices(std::move(other.m_indices)),
      m_material(std::move(other.m_material)), m_baseColor(other.m_baseColor),
      m_opacity(other.m_opacity), m_vao(other.m_vao), m_vbo(other.m_vbo),
      m_ebo(other.m_ebo) {
  other.m_vertices.clear();
  other.m_indices.clear();
  other.m_baseColor = glm::vec3(0.0f);
  other.m_opacity = 1.0f;
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

  // Tangent vec3
  glEnableVertexArrayAttrib(m_vao, 3);
  glVertexArrayAttribFormat(m_vao, 3, 3, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, tangent));
  glVertexArrayAttribBinding(m_vao, 3, 0);

  // Bitangent vec3
  glEnableVertexArrayAttrib(m_vao, 4);
  glVertexArrayAttribFormat(m_vao, 4, 3, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, bitangent));
  glVertexArrayAttribBinding(m_vao, 4, 0);
}

// void Mesh::draw(Shader &shader) {
//   uint32_t diffuse_n = 0;
//   uint32_t specular_n = 0;
//   uint32_t normal_n = 0;
//   uint32_t height_n = 0;
//   uint32_t metallic_n = 0;
//   uint32_t roughness_n = 0;
//   uint32_t ao_n = 0;
//
//   char name_buffer[34]; // 2 (fixed) + 11 (type_name) + 20 (max i
//                         // digits) + 1 (\0)
//
//   for (size_t i = 0; i < m_textures.size(); ++i) {
//     const char *type_name;
//     uint32_t *counter;
//
//     TextureType type = m_textures[i]->getType();
//     if (type == TextureType::DIFFUSE) {
//       type_name = "Diffuse";
//       counter = &diffuse_n;
//     } else if (type == TextureType::SPECULAR) {
//       type_name = "Specular";
//       counter = &specular_n;
//     } else if (type == TextureType::NORMAL) {
//       type_name = "Normal";
//       counter = &normal_n;
//     } else if (type == TextureType::HEIGHT) {
//       type_name = "Height";
//       counter = &height_n;
//     } else if (type == TextureType::METALLIC) {
//       type_name = "Metallic";
//       counter = &metallic_n;
//     } else if (type == TextureType::ROUGHNESS) {
//       type_name = "Roughness";
//       counter = &roughness_n;
//     } else if (type == TextureType::AO) {
//       type_name = "AO";
//       counter = &ao_n;
//     } else {
//       std::unreachable();
//     }
//
//     std::snprintf(name_buffer, sizeof(name_buffer), "u_%sTex%u", type_name,
//                   (*counter)++);
//     shader.setInt(name_buffer, i);
//
//     std::snprintf(name_buffer, sizeof(name_buffer), "u_BaseColor%lu", i);
//     shader.setVec3(name_buffer, m_baseColor);
//
//     glBindTextureUnit(i, m_textures[i]->getTexID());
//   }
//
//   char color_buffer[30];
//
//   glBindVertexArray(m_vao);
//   glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
// }

void Mesh::draw(Shader &shader) {
  int counter = 0;

  // 1. Diffuse/Albedo
  shader.setInt("u_DiffuseTex", counter);
  if (m_material.getDiffuse()) {
    glBindTextureUnit(counter, m_material.getDiffuse()->getTexID());
  } else {
    glBindTextureUnit(counter, TextureManager::getTexture(STATIC_WHITE_TEXTURE)->getTexID());
  }
  counter++;

  // 2. Normal
  shader.setInt("u_NormalTex", counter);
  if (m_material.getNormal()) {
    glBindTextureUnit(counter, m_material.getNormal()->getTexID());
  } else {
    glBindTextureUnit(counter, TextureManager::getTexture(STATIC_NORMAL_TEXTURE)->getTexID());
  }
  counter++;

  // 3. Height/Parallax
  shader.setInt("u_HeightTex", counter);
  if (m_material.getHeight()) {
    glBindTextureUnit(counter, m_material.getHeight()->getTexID());
  } else {
    glBindTextureUnit(counter, TextureManager::getTexture(STATIC_BLACK_TEXTURE)->getTexID());
  }
  counter++;

  // 4. MetallicRoughness (Packed)
  shader.setInt("u_MetallicRoughnessTex", counter);
  if (m_material.getMetallic()) {
    glBindTextureUnit(counter, m_material.getMetallic()->getTexID());
  } else {
    // Default PBR: Roughness=1.0, Metallic=0.0
    glBindTextureUnit(counter, TextureManager::getTexture(STATIC_PBR_DEFAULT_TEXTURE)->getTexID());
  }
  counter++;

  // 5. AO
  shader.setInt("u_AOTex", counter);
  if (m_material.getAO()) {
    glBindTextureUnit(counter, m_material.getAO()->getTexID());
  } else {
    glBindTextureUnit(counter, TextureManager::getTexture(STATIC_WHITE_TEXTURE)->getTexID());
  }
  counter++;

  // Factors
  shader.setVec3("u_BaseColor", m_baseColor);
  shader.setFloat("u_Opacity", m_opacity);
  shader.setFloat("u_MetallicFactor", m_material.getMetallicFactor());
  shader.setFloat("u_RoughnessFactor", m_material.getRoughnessFactor());
  shader.setFloat("u_AOFactor", m_material.getAOFactor());

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
}
