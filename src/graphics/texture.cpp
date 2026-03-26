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
  int width, height, nrComponents;

  stbi_set_flip_vertically_on_load(flip_vertical);
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

  if (!data) {
    std::println(stderr, "ERROR::TEXTURE::LOAD_FAILED: {}", path);
    return 0;
  }

  // 1. Determine Formats
  GLenum internalFormat, dataFormat;
  if (nrComponents == 1) {
    internalFormat = GL_R8;
    dataFormat = GL_RED;
  } else if (nrComponents == 3) {
    internalFormat = GL_RGB8;
    dataFormat = GL_RGB;
  } else { // nrComponents == 4
    internalFormat = GL_RGBA8;
    dataFormat = GL_RGBA;
  }

  // 2. Create Texture Object (DSA)
  GLuint textureID;
  glCreateTextures(GL_TEXTURE_2D, 1, &textureID);

  // 3. Calculate Mipmap Levels
  int levels =
      static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;

  // 4. Allocate Immutable Storage (DSA)
  glTextureStorage2D(textureID, levels, internalFormat, width, height);

  // 5. Handle Alignment for non-power-of-two/odd dimensions (418px, 350px)
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // 6. Upload Data (DSA)
  glTextureSubImage2D(textureID, 0, 0, 0, width, height, dataFormat,
                      GL_UNSIGNED_BYTE, data);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // Reset to default

  // 7. Generate Mipmaps (DSA)
  glGenerateTextureMipmap(textureID);

  // 8. Set Parameters (DSA) - Using REPEAT for your scaling and NEAREST for the
  // Crossy vibe
  glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Use NEAREST for that blocky Crossy Road look, or LINEAR for smooth
  glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER,
                      GL_NEAREST_MIPMAP_LINEAR);
  glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  stbi_image_free(data);
  return textureID;
}
