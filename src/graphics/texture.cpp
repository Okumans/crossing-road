#include "texture.hpp"
#include <print>

#define STB_IMAGE_IMPLEMENTATION

#include "external/stb_image.h"

Texture::Texture(const char *path, TextureType type, bool flip_vertical)
    : m_ownTex(true), m_type(type) {
  m_texID = _loadTexture(path, flip_vertical);
}

Texture::Texture(GLuint tex_id, TextureType type, bool own)
    : m_ownTex(own), m_type(type), m_texID(tex_id) {}

Texture::Texture(Texture &&other) noexcept
    : m_ownTex(other.m_ownTex), m_type(other.m_type), m_texID(other.m_texID) {
  other.m_type = static_cast<TextureType>(0);
  other.m_ownTex = false;
  other.m_texID = 0;
}

Texture::~Texture() {
  if (m_ownTex && m_texID)
    glDeleteTextures(1, &m_texID);
}

GLuint Texture::_loadTexture(const char *path, bool flip_vertical) {
  GLuint textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;

  stbi_set_flip_vertically_on_load(flip_vertical);

  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::println("Texture failed to load at path: {}", path);
    stbi_image_free(data);
  }

  return textureID;
}
